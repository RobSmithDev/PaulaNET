/*

_____             _       _   _ ___________ 
|  __ \          | |     | \ | |  ___|_   _|
| |__) |___ _   _| | __ _|  \| | |__   | |  
|  ___/ _` | | | | |/ _` | . ` |  __|  | |  
| |  | (_| | |_| | | (_| | |\  | |___  | |  
|_|   \__,_|\__,_|_|\__,_|_| \_|_____| |_|  

  PaulaNET - Amiga WiFi via the Floppy Port
  Using Paula's DMA for what she never intended
  Copyright (C) 2026 RobSmithDev

*/

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "hardware/watchdog.h"
#include "hardware/dma.h"
#include "hardware/pio.h"
#include "hardware/timer.h"
#include "hardware/clocks.h"
#include "pico/cyw43_arch.h"
#include "hardware/flash.h"
#include "hardware/uart.h"
#include "pico/multicore.h"
#include "build/floppytx.pio.h"
#include "rleCompression.h"
#include "trackBuilder.h"
#include "pico/flash.h"

#define DEBUGGING 1


/*
PaulaNET Protocol is:

Track 0..79: Normal Amiga Disk
Track 80:    Access Points
Track 81:    Config
Track 82:    Ethernet

Head Selection:
    Lower Side:     Receive  data from Amiga  (Core1)      Amiga -> Pico
    Upper Side:     Transmit data TO Amiga    (Core0)      Pico -> Amiga
                    Write Protect is used as a signalling method for when data is available
                    When Write Protect is LOW (Write Protected) Data is not available
                    When Write Protect is HIGH (Write Enabled) Data is available, backwards but makes sense
*/

#define MAJOR_VERSION               1
#define MINOR_VERSION               0


#define PIN_LED_PAULAIO           17      // Data from/to paula
#define PIN_LED_WIFI              16      // Wifi status, off, connecting, connected
#define PIN_LED_WIFIIO            18      // Data To/From WIFI

#define PIN_READDATA              15   // From the Amigas point of view, we write to this
#define PIN_WRITEDATA             14   // From The Amigas point of view, we read from this
#define PIN_WRITEGATE             13
#define PIN_TRACK0                12
#define PIN_READY                 11
#define PIN_WRITEPROTECT          10
#define PIN_MOTOR                 9
#define PIN_SELECT                8
#define PIN_DIR                   7
#define PIN_STEP                  6
#define PIN_SIDE                  5
#define PIN_RESET                 4
#define PIN_DISKCHANGE            3
#define PIN_INDEX                 2

#define TRACK_ACCESSPOINT        75
#define TRACK_CONFIG             76
#define TRACK_ETHERNET           77

// MAX number of bytes we'll receive/send at a time from DMA
#define MAX_TRANSFER_SIZE           16384
#define MIN_TRANSFER_SIZE           1536

#define PAULA_INDEX_PADDING         120         // Padding to add at the start of any data sent to the Amiga + PLL lock
#define PAULA_MAX_BYTES             (MAX_TRANSFER_SIZE + (PAULA_INDEX_PADDING * 8))
#define PAULA_MAX_WORDS             (PAULA_MAX_BYTES>>1)
#define MAX_TX_SIZE                 (PAULA_MAX_BYTES - PAULA_INDEX_PADDING - PAULA_INDEX_PADDING)
#define MAX_WIFI_SCANS              5           // Perform wifi scan this many times to try to catch all the networks

#define SETTINGS_MAGIC_ID           0x504E4554
#define MAX_SCAN_RESULTS            64

// Place settings at the last sector of flash
#define FLASH_TARGET_OFFSET (PICO_FLASH_SIZE_BYTES - (FLASH_BLOCK_SIZE*2))

enum DataMode: uint8_t { dmAccessPoints = 1, dmConfig = 2, dmEthernet = 3};
enum PaulaMode {pmDisk, pmAccessPoint, pmConfig, pmEthernet};

#define DEVICESTATUS_CANRECEIVE             1
#define DEVICESTATUS_CANSEND                2

#define SWAP16(x) __builtin_bswap16(x)
#define SWAP32(x) ((((x) & 0xFF) << 24) | (((x) & 0xFF00) << 8) | (((x) >> 8) & 0xFF00) | (((x) >> 24) & 0xFF))

#define FLAGS_AMIGA_COMPRESSION         (1<<1)          // Amiga compresses data when sending to the Pico
#define FLAGS_PICO_COMPRESSION          (1<<0)          // Pico compresses data when sending to the Amiga


struct PaulaNETSettings {
    uint16_t        length;         // Size of this structure to allow for future expansion
    char            ssid[34];       // 32 + null + 1 for padding
    char            pwd[66];        // 64 + null + 1 for padding
    char            hostname[34];   // 32 + null + 1 for padding
    uint16_t        flags;          // Settings
    uint16_t        maxRxSize;      // Max size of data we'll receive from the Amiga in one go
    uint16_t        maxTxSize;      // Max size of data we'll transfer to the Amiga in one go
};

struct FlashSettings {
    uint32_t                    magic;          // Magic number to detect valid/uninitialised flash
    struct PaulaNETSettings     settings;
};

struct LiveStatus {
    uint16_t                             length;         // size of this
    uint8_t                              versionh, versionl;   // firmware version
    uint8_t                              macAddress[6];  // MAC address
    volatile int16_t                     rssi;
    volatile uint8_t                     linkStatus;
    uint8_t                              reserved;
};

// Header on the front of all data sent or received
struct DataHeader {
    uint32_t    magic;          // Should read Pnet
    uint16_t    checksum;       // Simple checksum
    uint16_t    dataSize;       // Full data size
    uint16_t    compressedSize; // Compressed data size
    DataMode    dataMode;       // What this data is
    uint8_t     compressed;     // Was compression used?
};

struct PaulaNETSettings settings;
struct LiveStatus liveStatus;

// Wifi access point scan
uint32_t scanResultCount;
struct ScanResult {
    char        ssid[34];       // 32 + null + padding
    int16_t     rssi;           // signal strength
    uint8_t     channel;
    uint8_t     padding;
};
ScanResult scanResults[MAX_SCAN_RESULTS]; 

#define PACKET_QUEUE_SIZE   20
#define ETH_MTU             1518  // 1500 + ethernet header + some margin

// A single packet, as it's sent or received
struct Packet {
    uint8_t data[ETH_MTU];
    uint16_t length;
};

// To store packets
struct PacketQueue {
    Packet  packets[PACKET_QUEUE_SIZE];
    volatile uint32_t readIndex;
    volatile uint32_t writeIndex;
};


struct PacketsHeader {
    uint8_t numPackets;     
    uint8_t flags; 
    uint8_t amigaCounter; 
    uint8_t picoCounter;
};

struct CompressionHeader {
    uint16_t compressedSize;     
    uint16_t originalSize;    
};


#define ETH_MTU_MFM_COMPRESSED   ((ETH_MTU<<1) + 4 + (sizeof(CompressionHeader)<<1))

// Data for each packet to be sent, as it's pre-compressed
struct CompressedPacket {
    uint8_t data[ETH_MTU_MFM_COMPRESSED];
    uint16_t length;
};

// All packets to be compressed
struct CompressedQueue {
    CompressedPacket packets[PACKET_QUEUE_SIZE];
    volatile uint32_t readIndex;
    volatile uint32_t writeIndex;
};

struct PacketQueue txQueue;
struct PacketQueue rxQueue;
struct CompressedQueue compressedRxQueue;

// To make claiming them easy for us
struct PioClaim {
    PIO pio;
    int sm;
};

PioClaim pioFloppyTX, pioFloppyRx, pioDriveSelect, pioDriveStep;


// Wifi led status
absolute_time_t wifiIOLedOff = {0};


// For reading/writing
static uint8_t IoBuffer[PAULA_MAX_BYTES];
// To write data to before being compressed.
static uint8_t CompressBuffer[PAULA_MAX_BYTES];
// Backup of packets to be sent
static uint8_t PacketBackup[PAULA_MAX_BYTES];
static uint16_t PacketBackupSize = 0;
PacketsHeader packetHeaderSend;

// Compression buffers
static uint8_t Core0RunBuffer[RUN_BUFFER_SIZE];
static uint8_t Core1RunBuffer[RUN_BUFFER_SIZE];

// live!
volatile int track = 0;                 // Current track number.  
volatile bool isSelected = false;       // TRUE if this device is active
volatile bool isMotorActive = false;    // True if the motor is latched
volatile bool wifiRequestScan = false;  // request a wifi scan
volatile bool wifiScanActive = false;   // Is a wifi scan currently active?
volatile bool wifiIsUp = false;         // Is wifi actually working
volatile bool wifiReconnect = false;    // Set to TRUE if the wifi should be reconnected
volatile bool diskChange = false;       // Signal a disk change
volatile bool rebootImminent = false;   // A reboot is about to happen
volatile uint32_t lastStepTime;
volatile int wifiScanCounter = 0;       // How many scans have been completed before we say its actually complete
volatile bool isWifiConnecting = false; // Is wifi connecting?
volatile PaulaMode paulaMode;
uint8_t lastAmigaCounter = 0xFF;
uint8_t lastPicoCounter = 0x7F;


#include "pico/sync.h"
spin_lock_t* printLock;
void safePrintf(const char* fmt, ...) {    
#ifdef DEBUGGING    
    uint32_t save = spin_lock_blocking(printLock);
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    spin_unlock(printLock, save);
#endif    
}
void hexDump(const void* data, int length) {
#ifdef DEBUGGING        
    uint32_t save = spin_lock_blocking(printLock);

    const uint8_t* p = (const uint8_t*)data;
    uint32_t i, j;
    for (i = 0; i < length; i += 16) {
        printf("%04lx: ", i);
        for (j = 0; j < 16; j++) {
            if (i + j < length)
                printf("%02lx ", (uint32_t)p[i+j]);
            else
                printf("   ");
        }
        printf(" ");
        for (j = 0; j < 16 && i + j < length; j++) {
            uint8_t c = p[i+j];
            printf("%lc", (uint32_t)(c >= 32 && c < 127 ? c : '.'));
        }
        printf( "\n");
        sleep_ms(1);
    }
    spin_unlock(printLock, save);
#endif    
}


// Quick checksum calculator
uint16_t calculateChecksum(const uint16_t* data, int32_t lengthInBytes) {
    uint16_t checksum = 0;
    while (lengthInBytes>0) {
        checksum ^= *data++;
        lengthInBytes-=2;
    }
    return checksum;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Setup and Reset


// Signal a disk change
void signalDiskChange() {
    gpio_put(PIN_DISKCHANGE, 0);
    diskChange = true;
    __dmb();
}

// Save settings to flash
void __not_in_flash_func(saveSettings)() {
    gpio_put(PIN_LED_PAULAIO, 0);    gpio_put(PIN_LED_WIFI, 0);    gpio_put(PIN_LED_WIFIIO, 0);
    FlashSettings set;
    set.magic = SETTINGS_MAGIC_ID;
    set.settings = settings;
    set.settings.length = sizeof(set.settings);
    uint8_t buffer[FLASH_PAGE_SIZE];
    memset(buffer, 0xFF, FLASH_PAGE_SIZE);   
    memcpy(buffer, &set, sizeof(set));

    uint32_t interrupts = save_and_disable_interrupts();
    flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(FLASH_TARGET_OFFSET, buffer, FLASH_PAGE_SIZE);
    flash_flush_cache();
    restore_interrupts(interrupts);
    busy_wait_ms(10);
}

// Load settings from flash
void loadSettings() {
    // Actual memory mapped address to read from
    const FlashSettings* flashSettings = (const FlashSettings*)(XIP_BASE + FLASH_TARGET_OFFSET);
    if (flashSettings->magic != SETTINGS_MAGIC_ID) {
        // defaults
        memset(&settings, 0, sizeof(settings));
        settings.maxRxSize = 8192;
        settings.maxTxSize = 8192;
        settings.flags = FLAGS_AMIGA_COMPRESSION | FLAGS_PICO_COMPRESSION;
        strcpy(settings.hostname, "PaulaNET");
    } else {
        settings = flashSettings->settings;
        // Handle if the data was smaller
        if (sizeof(settings) > flashSettings->settings.length) {
            // In future this might be changed with special initialization values if the size changes
            memset(((uint8_t*)&settings)+flashSettings->settings.length, 0, sizeof(settings) - flashSettings->settings.length);
        }
    }    
}

// Claim a pio and SM combination. Returns FALSE on fail.
bool requestAndClaimPIO(PioClaim* claim) {
    claim->sm = pio_claim_unused_sm(pio0, false);  // false = don't panic if none free
    if (claim->sm >= 0) {
        claim->pio = pio0;
        return true;
    } else {
        // Try pio1
        claim->sm = pio_claim_unused_sm(pio1, false);
        if (claim->sm >= 0) {
            claim->pio = pio1;
            return true;
        } else {
            return false;
        }
    }
}

// As I keep forgetting which way round this is, a nice inline for it
inline void setWriteProtected(const bool writeProtected) {
    gpio_put(PIN_WRITEPROTECT, !writeProtected);
}

// Reverse of the above
inline bool isWriteProtected() {
    return !gpio_get(PIN_WRITEPROTECT);
}

// Return true if the LOWER side is currently selected
inline bool isLowerSide() {
    return gpio_get(PIN_SIDE);
}

// Reset everything
void reset() {
    paulaMode = PaulaMode::pmDisk;
    track = 5;
    gpio_put(PIN_TRACK0, 1);       
    gpio_put(PIN_DISKCHANGE, 0);   // no disk to start with
    gpio_put(PIN_LED_PAULAIO, 1);   
    gpio_put(PIN_LED_WIFI, 1);   
    gpio_put(PIN_LED_WIFIIO, 1);   
    gpio_put(PIN_READDATA, 1);
    gpio_put(PIN_INDEX, 1);

    memset(&txQueue, 0, sizeof(txQueue));
    memset(&rxQueue, 0, sizeof(rxQueue));
    memset(&compressedRxQueue, 0, sizeof(compressedRxQueue));
    memset(&PacketBackup, 0, sizeof(PacketBackup));
    memset(PacketBackup, 0xAA, PAULA_INDEX_PADDING);
    uint16_t* b = (uint16_t*)&PacketBackup[PAULA_INDEX_PADDING];
    b[0] = 0x4489;
    b[1] = 0x4489;
}

// ISR for monitoring some of the pins in a timely fashion - This is ran on CORE0
void __not_in_flash_func(pinMonitor)(uint gpio, uint32_t events)  {
    switch (gpio) {       
        case PIN_SELECT:
            isSelected = (events & GPIO_IRQ_EDGE_FALL) != 0;
            // Motor state is latched on select
            if (isSelected) isMotorActive = gpio_get(PIN_MOTOR) == 0;            
            gpio_set_dir(PIN_READDATA, isSelected ? GPIO_OUT : GPIO_IN);
            gpio_set_dir(PIN_TRACK0, isSelected ? GPIO_OUT : GPIO_IN);
            gpio_set_dir(PIN_WRITEPROTECT, isSelected ? GPIO_OUT : GPIO_IN);
            gpio_set_dir(PIN_DISKCHANGE, isSelected ? GPIO_OUT : GPIO_IN);
            gpio_set_dir(PIN_INDEX, isSelected ? GPIO_OUT : GPIO_IN);
            __dmb();
            break;

    }
}

// Handle the STEPPING of a drive by responding to PIO. The normal ISR was causing too much latency when other things were happennig
void __not_in_flash_func(stepPioHandler)() {
    while (!pio_sm_is_rx_fifo_empty(pioDriveStep.pio, pioDriveStep.sm)) {
        uint32_t val = pio_sm_get(pioDriveStep.pio, pioDriveStep.sm);
        const bool dir = (val >> 1) & 1;        
        if (dir) {
            if (track) track--;
        } else {
            if (track < 79) track++;
        }        
        lastStepTime = time_us_32();
        gpio_put(PIN_TRACK0, track != 0);
        if (diskChange) {
            gpio_put(PIN_DISKCHANGE, 1);
            diskChange = false;
        }        
        switch (track) {
            case TRACK_ACCESSPOINT: paulaMode = PaulaMode::pmAccessPoint; break;
            case TRACK_CONFIG:      paulaMode = PaulaMode::pmConfig;      break;
            case TRACK_ETHERNET:    paulaMode = PaulaMode::pmEthernet;    break;
            default:                paulaMode = PaulaMode::pmDisk;        break;
        }
        __dmb();
    }
}

// Use PIO for STEP 
void setupStepPIO() {    
    uint offset = pio_add_program(pioDriveStep.pio, &step_capture_program);

    pio_sm_config c = step_capture_program_get_default_config(offset);

    // Pin base is PIN_STEP (6), covers STEP(6), DIR(7), SELECT(8)
    sm_config_set_in_pins(&c, PIN_STEP);
    sm_config_set_jmp_pin(&c, PIN_SELECT);

    // Configure pins as inputs
    pio_sm_set_consecutive_pindirs(pioDriveStep.pio, pioDriveStep.sm, PIN_STEP, 3, false);
    pio_gpio_init(pioDriveStep.pio, PIN_STEP);
    pio_gpio_init(pioDriveStep.pio, PIN_DIR);
    pio_gpio_init(pioDriveStep.pio, PIN_SELECT);

    // Shift left, autopush off (we use explicit push)
    sm_config_set_in_shift(&c, false, false, 32);

    pio_sm_init(pioDriveStep.pio, pioDriveStep.sm, offset, &c);

    // Set up IRQ
    pio_set_irq0_source_enabled(pioDriveStep.pio,  (pio_interrupt_source)(pis_sm0_rx_fifo_not_empty + pioDriveStep.sm), true);
    int irqNum = pioDriveStep.pio == pio0 ? PIO0_IRQ_0 : PIO1_IRQ_0;
    irq_set_exclusive_handler(irqNum, stepPioHandler);
    irq_set_enabled(irqNum, true);

    pio_sm_set_enabled(pioDriveStep.pio, pioDriveStep.sm, true);
}

// Handles making the drive select fast enough so DriveID works. I could have just wired this to select lol
void setupDriveSelectPIO(uint selectPin, uint readyPin) {
    uint offset = pio_add_program(pioDriveSelect.pio, &drive_select_program);
    pio_sm_config c = drive_select_program_get_default_config(offset);
    
    sm_config_set_in_pins(&c, selectPin);
    sm_config_set_set_pins(&c, readyPin, 1);
    
    // Full speed - we want minimum latency
    sm_config_set_clkdiv(&c, 1.0f);
    
    pio_gpio_init(pioDriveSelect.pio, readyPin);
    pio_sm_set_consecutive_pindirs(pioDriveSelect.pio, pioDriveSelect.sm, readyPin, 1, true);  // ready = output
    // selectPin stays as input, no pindirs change needed
    
    pio_sm_init(pioDriveSelect.pio, pioDriveSelect.sm, offset, &c);
    pio_sm_exec(pioDriveSelect.pio, pioDriveSelect.sm, pio_encode_set(pio_pins, 1));
    pio_sm_set_enabled(pioDriveSelect.pio, pioDriveSelect.sm, true);
}

// Setup the GPIO pins
void preparePins() {
    liveStatus.versionh = MAJOR_VERSION;
    liveStatus.versionl = MINOR_VERSION;
    liveStatus.reserved = 0;

    // OUTPUT Pins
    gpio_init(PIN_LED_PAULAIO);    gpio_set_dir(PIN_LED_PAULAIO, GPIO_OUT);
    gpio_init(PIN_LED_WIFI);       gpio_set_dir(PIN_LED_WIFI, GPIO_OUT);
    gpio_init(PIN_LED_WIFIIO);     gpio_set_dir(PIN_LED_WIFIIO, GPIO_OUT);

    // Drive related output - They're currently set as inputs as they're only set as output when selected
    gpio_init(PIN_READDATA);        gpio_set_dir(PIN_READDATA, GPIO_IN);
    gpio_init(PIN_TRACK0);          gpio_set_dir(PIN_TRACK0, GPIO_IN);
    gpio_init(PIN_READY);           gpio_put(PIN_READY, 1); gpio_set_dir(PIN_READY, GPIO_IN); 
    gpio_init(PIN_WRITEPROTECT);    gpio_set_dir(PIN_WRITEPROTECT, GPIO_IN);
    gpio_init(PIN_DISKCHANGE);      gpio_set_dir(PIN_DISKCHANGE, GPIO_IN);
    gpio_init(PIN_INDEX);           gpio_set_dir(PIN_INDEX, GPIO_IN);

    // INPUT Pins - dont really need the pullups as they're on the logic converters, but for completeness
    gpio_init(PIN_WRITEDATA);       gpio_set_dir(PIN_WRITEDATA, GPIO_IN);       gpio_pull_up(PIN_WRITEDATA);
    gpio_init(PIN_WRITEGATE);       gpio_set_dir(PIN_WRITEGATE, GPIO_IN);       gpio_pull_up(PIN_WRITEGATE);
    gpio_init(PIN_MOTOR);           gpio_set_dir(PIN_MOTOR, GPIO_IN);           gpio_pull_up(PIN_MOTOR);    
    gpio_init(PIN_SELECT);          gpio_set_dir(PIN_SELECT, GPIO_IN);          gpio_pull_up(PIN_SELECT);   
    gpio_init(PIN_DIR);             gpio_set_dir(PIN_DIR, GPIO_IN);             gpio_pull_up(PIN_DIR);      
    gpio_init(PIN_STEP);            gpio_set_dir(PIN_STEP, GPIO_IN);            gpio_pull_up(PIN_STEP);     
    gpio_init(PIN_SIDE);            gpio_set_dir(PIN_SIDE, GPIO_IN);            gpio_pull_up(PIN_SIDE);     
    gpio_init(PIN_RESET);           gpio_set_dir(PIN_RESET, GPIO_IN);           gpio_pull_up(PIN_RESET);    

    // Claim required PIO's
    requestAndClaimPIO(&pioDriveStep);
    requestAndClaimPIO(&pioDriveSelect);
    requestAndClaimPIO(&pioFloppyRx);
    requestAndClaimPIO(&pioFloppyTX);

    // Monitor STEP from a PIO
    setupStepPIO();
    setupDriveSelectPIO(PIN_SELECT, PIN_READY);  // Make sure DriveID works
    gpio_set_dir(PIN_READY, GPIO_OUT); 

    // Monitor SELECT from the ISR
    gpio_set_irq_enabled_with_callback(PIN_SELECT, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &pinMonitor);
}

// Flash the WIFI led
inline void flashWifiIO() {
    gpio_put(PIN_LED_WIFIIO, 1);
    wifiIOLedOff = make_timeout_time_ms(25);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CORE 1: Reading and writing data between the pico and the Amiga
//         This core needs good timing so does not use any interrupts etc


// Request settings be saved on core0
void __not_in_flash_func(requestSettingsSave)(bool reconnect) {
    multicore_fifo_push_blocking(reconnect);
    busy_wait_ms(10);
    multicore_fifo_pop_blocking();    
    busy_wait_ms(10);
}

// Handle and process the data we received from the Amiga into IoBuffer
bool handleDataReceived(const PaulaMode mode, const uint32_t bytesReceived) {
    if (bytesReceived < sizeof(DataHeader)) {
        safePrintf("Not enough data received\n");
        return false;
    }
    // Check header is intact
    DataHeader* header = (DataHeader*)IoBuffer;

    header->dataSize = SWAP16(header->dataSize);
    header->compressedSize = SWAP16(header->compressedSize);
    header->magic = SWAP32(header->magic);

//   safePrintf("Compress Size: %i\n", header->compressedSize);
//   safePrintf("Data Size: %i\n", header->dataSize);
//   safePrintf("Checksum: %04lx\n", header->checksum);
//   safePrintf("Data Mode: %i\n", header->dataMode);
//   safePrintf("Compressed: %i\n", header->compressed);
//   safePrintf("magic %x     %x\n", header->magic, SETTINGS_MAGIC_ID);

    if (header->magic !=SETTINGS_MAGIC_ID) {
        safePrintf("Receive header mismatch\n");
        return false;
    }

    // Simple validation
    if ((sizeof(DataHeader) + header->compressedSize >= bytesReceived) || (header->dataSize >= PAULA_MAX_BYTES) || (header->compressedSize >= PAULA_MAX_BYTES)) {
        safePrintf("Receive data size overflow\n");
        return false;
    }
    // Validate checksum
    uint16_t checksum = calculateChecksum((uint16_t*)&header[1], header->compressedSize);
    if (checksum != header->checksum) {
        safePrintf("Receive checksum failed %04lx\n", checksum);
        return false;
    }

    uint8_t* dataStart = (uint8_t*)&header[1];
    if (header->dataMode != DataMode::dmEthernet)   // ethernet is compressed differently       
        if (header->compressed) {
            if (rleDecompress((uint8_t*)&header[1], header->compressedSize, CompressBuffer, PAULA_MAX_BYTES) != header->dataSize) {
                safePrintf("Decompressed data received size mismatch\n");
                return false;
            }       
            dataStart = CompressBuffer;
        } else {
            // It's MFM encoded
            mfmDecode((uint16_t*)&header[1], header->compressedSize, CompressBuffer);
            dataStart = CompressBuffer;
        }

    switch (mode) {
        case PaulaMode::pmAccessPoint:
            if (header->dataMode != DataMode::dmAccessPoints) {
                safePrintf("case PaulaMode::pmAccessPoint but %i received\n",header->dataMode);
                return false;
            }
            // A write in Access Point mode means start scanning for Wifi. We dont care about the data contents
            if (!wifiScanActive) {
                safePrintf("Requesting wifi scan\n");                
                wifiRequestScan = true;
                __dmb();
            }
            return true;
            break;

        case PaulaMode::pmConfig: 
            if (header->dataMode != DataMode::dmConfig) {
                safePrintf("case PaulaMode::pmConfig but %i received\n",header->dataMode);
                return false;
            }
            {
                PaulaNETSettings* inSettings = (PaulaNETSettings*)dataStart;
                uint16_t settingsLen = SWAP16(inSettings->length);
                if ((settingsLen >= 142) && (settingsLen<1023) && (settingsLen <= header->dataSize)) {
                    inSettings->maxRxSize = SWAP16(inSettings->maxRxSize);
                    inSettings->maxTxSize = SWAP16(inSettings->maxTxSize);   
                    // Clamp a few things
                    if (inSettings->maxRxSize<MIN_TRANSFER_SIZE) inSettings->maxRxSize=MIN_TRANSFER_SIZE;
                    if (inSettings->maxTxSize<MIN_TRANSFER_SIZE) inSettings->maxTxSize=MIN_TRANSFER_SIZE;
                    if (inSettings->maxRxSize>MAX_TRANSFER_SIZE) inSettings->maxRxSize=MAX_TRANSFER_SIZE;
                    if (inSettings->maxTxSize>MAX_TRANSFER_SIZE) inSettings->maxTxSize=MAX_TRANSFER_SIZE;

                    // Password coming back will be all "*" if it wasnt changed
                    bool newPassword = strlen(inSettings->pwd) != strlen(settings.pwd);
                    if (!newPassword) {
                        for (int i=0; i<strlen(inSettings->pwd); i++)
                            if (inSettings->pwd[i]!='*') {
                                newPassword = true;
                                break;
                            }
                    }
                    if (!newPassword) strcpy(inSettings->pwd, settings.pwd);
                    inSettings->flags = SWAP16(inSettings->flags);

                    // Check if wifi needs to reconnect
                    bool reconnect = (strcmp(settings.hostname, inSettings->hostname)) || (strcmp(settings.pwd, inSettings->pwd)) || (strcmp(settings.ssid, inSettings->ssid));
                    if (settingsLen > sizeof(settings)) settingsLen = sizeof(settings);
                    memcpy(&settings, inSettings, settingsLen);                   
                    __dmb();
                    requestSettingsSave(reconnect);     
                    return true;               
                } else {
                    safePrintf("Config save error\n");
                    return false;
                }
            }
            break;
        case PaulaMode::pmEthernet: 
            if (header->dataMode  != DataMode::dmEthernet) {
                safePrintf("case PaulaMode::pmEthernet but %i received\n",header->dataMode);
                return false;
            }
            {
                // Decode the header
                PacketsHeader hdr;
                mfmDecode((uint16_t*)dataStart, sizeof(hdr)*2, (uint8_t*)&hdr);
                dataStart += sizeof(hdr)*2;

                // Is the Amiga ACK our current packets buffer? Yes it might be
                if (hdr.picoCounter == lastPicoCounter) PacketBackupSize = 0;
                // Have we seen this one before?
                if (hdr.amigaCounter != lastAmigaCounter) {
                    lastAmigaCounter = hdr.amigaCounter;
                    // How many packets?
                    int len = bytesReceived - (sizeof(DataHeader) + (sizeof(PacketsHeader)<<1));
                    if (len > header->dataSize) len = header->dataSize;
                    
                    // Extract each packet
                    CompressionHeader chead;                
                    while (hdr.numPackets) {
                        // Enough room left?
                        if (header->dataSize < sizeof(chead)<<1) {
                            safePrintf("Ran out of data when reading packets. %i remaining\n", hdr.numPackets);
                            return false;
                            break;
                        }

                        // Extract compression header
                        mfmDecode((uint16_t*)dataStart, sizeof(chead)<<1, (uint8_t*)&chead);
                        dataStart += sizeof(chead)<<1;
                        len -= sizeof(chead)<<1;
                        chead.originalSize = SWAP16(chead.originalSize);
                        chead.compressedSize = SWAP16(chead.compressedSize);

                        // Now extract each ethernet packet
                        uint16_t compressedSize = chead.compressedSize & 0x7FFF;  // MSB explains compression mode
                        if (compressedSize > len) {
                            safePrintf("Ran out of data reading packet data. Required %i, remaining %i\n", compressedSize, len);
                            return false;
                        }

                        // Is there any space? Ideally we dont want to wait here, but 1ms should be enough to make some space
                        if ((txQueue.writeIndex + 1) % PACKET_QUEUE_SIZE == txQueue.readIndex) sleep_ms(1);

                        // Space now?
                        if ((txQueue.writeIndex + 1) % PACKET_QUEUE_SIZE != txQueue.readIndex) {
                            if (chead.compressedSize & 0x8000) {
                                // Packet is compressed
                                if (rleDecompress((uint8_t*)dataStart, compressedSize, txQueue.packets[txQueue.writeIndex].data, ETH_MTU) != chead.originalSize) {
                                    safePrintf("Packet received was decompressed to incorrect size");
                                    return false;
                                } 
                            } else {
                                // Packet is MFM encoded
                                mfmDecode((uint16_t*)dataStart, compressedSize, txQueue.packets[txQueue.writeIndex].data);
                            }
                            // Save it
                            txQueue.packets[txQueue.writeIndex].length = chead.originalSize;

                            __dmb();
                            txQueue.writeIndex = (txQueue.writeIndex + 1) % PACKET_QUEUE_SIZE;
                        } else {
                            safePrintf("Packet queue full, dropping packet");
                            return false;
                        }
                        dataStart += compressedSize;
                        len -= compressedSize;

                        // Must be on an even boundary
                        if (compressedSize&1) {
                            dataStart++;
                            len --;
                        }                
                        hdr.numPackets--;
                    }
                }
			}
            return true;
            break;            
    }
    return false;
}
 
// Prepares PacketBackup buffer for packets to transmit. When finished this buffer is RAW mfm ready to be streamed.
void preparePacketsToTransmit() {
    if (PacketBackupSize) return; // already set? it's already ready
    lastPicoCounter++;

    // Pointer to whre data starts
    uint8_t* headerStart = (uint8_t*)&PacketBackup[ PAULA_INDEX_PADDING + 4];
    uint8_t* dataStart = headerStart +  (sizeof(DataHeader)*2);
    uint8_t* dataPos = dataStart;
    dataPos += sizeof(packetHeaderSend)<<1; // skip past the packet header, we'll do that later

    // Populate the start of the packet header
    packetHeaderSend.numPackets = 0;
    packetHeaderSend.picoCounter = lastPicoCounter;

    // NOTE: cycleCounterRX will need filling at the time of send incase it changes
    int bufferUsed = sizeof(packetHeaderSend)<<1;

    while ((compressedRxQueue.readIndex != compressedRxQueue.writeIndex) && ((bufferUsed + compressedRxQueue.packets[compressedRxQueue.readIndex].length) < settings.maxTxSize) && (packetHeaderSend.numPackets < PACKET_QUEUE_SIZE)) {
        // Copy the packet                
        memcpy(dataPos, compressedRxQueue.packets[compressedRxQueue.readIndex].data, compressedRxQueue.packets[compressedRxQueue.readIndex].length);
        bufferUsed += compressedRxQueue.packets[compressedRxQueue.readIndex].length;
        dataPos += compressedRxQueue.packets[compressedRxQueue.readIndex].length;
        packetHeaderSend.numPackets++;
        compressedRxQueue.readIndex = (compressedRxQueue.readIndex + 1) % PACKET_QUEUE_SIZE;
    }
    __dmb();

    // Total number of bytes in the payload
    PacketBackupSize = dataPos - dataStart;
}

// Populate IoBuffer and return the total number of bytes to transmit
uint32_t populateTransmitBufferForPackets() {   
    uint8_t* headerStart = (uint8_t*)&PacketBackup[ PAULA_INDEX_PADDING + 4];
    uint8_t* dataStart = headerStart +  (sizeof(DataHeader)*2);    

    preparePacketsToTransmit();
    packetHeaderSend.flags = ((txQueue.writeIndex == txQueue.readIndex) ? 0 : DEVICESTATUS_CANSEND) | ((compressedRxQueue.writeIndex == compressedRxQueue.readIndex) ? 0 : DEVICESTATUS_CANRECEIVE);
    packetHeaderSend.amigaCounter = lastAmigaCounter;  // send what the amiga last sent us

    uint16_t lastBit = 0;
    mfmEncode((uint8_t*)&packetHeaderSend, sizeof(packetHeaderSend), (uint16_t*)dataStart, &lastBit);

    DataHeader header;
    header.dataMode = DataMode::dmEthernet;  
    header.magic = SWAP32(SETTINGS_MAGIC_ID);
    header.dataSize = PacketBackupSize;        
    header.compressed = 0;          // Raw
    int compressSize = header.dataSize;
    header.compressedSize = SWAP16(header.dataSize);   
    header.checksum = SWAP16(calculateChecksum((uint16_t*)dataStart, header.dataSize));    
    header.dataSize = SWAP16(header.dataSize);

    lastBit = 0;
    mfmEncode((const uint8_t*)&header, sizeof(header), (uint16_t*)headerStart, &lastBit);

    dataStart += compressSize;     
    
    int dataLength = (dataStart - PacketBackup) + PAULA_INDEX_PADDING;
    if (dataLength > PAULA_MAX_BYTES) dataLength = PAULA_MAX_BYTES;
    dataLength -= (dataStart - PacketBackup);
    if (dataLength) memset(dataStart, 0xAA, dataLength);
    dataStart += dataLength;
       
    return dataStart - PacketBackup;
}

// Populate IoBuffer and return number of bytes saved to it
uint32_t populateTransmitBuffer(const PaulaMode mode) {
    // Some simple noise for paula to latch onto
    memset(IoBuffer, 0xAA, PAULA_INDEX_PADDING);

    uint16_t* syncPos = (uint16_t*)&IoBuffer[PAULA_INDEX_PADDING];
    *syncPos++ = 0x4489;
    *syncPos++ = 0x4489;

    // The header is always sent out MFM encoded. The rest of the data isn't
    DataHeader header;
    header.checksum = 0;
    header.magic = SWAP32(SETTINGS_MAGIC_ID);
    header.dataSize = 0; // to be filled    
    
    uint8_t* dataPos = CompressBuffer;

    switch (mode) {
        case PaulaMode::pmAccessPoint:   
            header.dataMode = DataMode::dmAccessPoints;             
            *((uint16_t*)dataPos) = SWAP16(scanResultCount); 
            dataPos += 2;
            if (scanResultCount) memcpy(dataPos, scanResults, sizeof(ScanResult) * scanResultCount);
            dataPos += sizeof(ScanResult) * scanResultCount;
            header.dataSize = dataPos - CompressBuffer;
            //safePrintf("Prepared buffer for %i access points\n", scanResultCount);
            break;

        case PaulaMode::pmConfig: 
            header.dataMode = DataMode::dmConfig; 
            {
                PaulaNETSettings* amigaSettings = (PaulaNETSettings*)dataPos;
                *amigaSettings = settings;
                amigaSettings->length = SWAP16(sizeof(settings));
                amigaSettings->flags = SWAP16(amigaSettings->flags);
                amigaSettings->maxRxSize = SWAP16(amigaSettings->maxRxSize);
                amigaSettings->maxTxSize = SWAP16(amigaSettings->maxTxSize);
                // Fill ALL characters with * so no data can leak, even from previous passwords
                memset(dataPos + offsetof(PaulaNETSettings, pwd), '*', sizeof(settings.pwd));
                // Null terminator
                *(dataPos + offsetof(PaulaNETSettings, pwd) + strlen(settings.pwd)) = 0;
                dataPos += sizeof(settings);

                // After settings we transmit some basic live status data
                LiveStatus* status = (LiveStatus*)dataPos;
                *status = liveStatus;
                status->rssi = SWAP16(status->rssi);
                status->length = SWAP16(sizeof(LiveStatus));            
                dataPos += sizeof(liveStatus);
                header.dataSize = dataPos - CompressBuffer;
                //safePrintf("Prepared buffer for config and status\n");
            }
            break;       
    }

    // Odd?
    if (header.dataSize&1) {
        *dataPos++ = 0;
        header.dataSize++;
    }

    // Compress the buffer directly into the output
    uint16_t lastBit = 0;
    int compressSize;
    if (settings.flags & FLAGS_PICO_COMPRESSION) {
        header.compressed = 1;  // Compressed
        compressSize = header.dataSize ? rleCompress(CompressBuffer, header.dataSize, ((uint8_t*)syncPos) + (sizeof(header)*2), true, Core1RunBuffer) : 0;
    } else {
        header.compressed = 2;  // MFM
        compressSize = header.dataSize ? mfmEncode(CompressBuffer, header.dataSize, (uint16_t*)(((uint8_t*)syncPos) + (sizeof(header)*2)), &lastBit) : 0;
    }
    // Calculate the checksum on the compressed buffer
    header.checksum = SWAP16(calculateChecksum((uint16_t*)(((uint8_t*)syncPos) + (sizeof(header)*2)), compressSize));
    header.compressedSize = SWAP16(compressSize);
    
    header.dataSize = SWAP16(header.dataSize);

    lastBit = 0;
    mfmEncode((const uint8_t*)&header, sizeof(header), (uint16_t*)syncPos, &lastBit);

    int len = compressSize + PAULA_INDEX_PADDING + 4 + (sizeof(header) * 2);
    dataPos = &IoBuffer[len];
    
    if (len < settings.maxTxSize) {
        memset(dataPos, 0xAA, settings.maxTxSize - len);
        dataPos += settings.maxTxSize - len;
    }
    memset(dataPos, 0xAA, PAULA_INDEX_PADDING+PAULA_INDEX_PADDING);
    dataPos += PAULA_INDEX_PADDING+PAULA_INDEX_PADDING;   
    return dataPos - IoBuffer;
}

// Setup and prepare the Pio using PIO 0
void setupReadingPIO(uint pin) {    
    uint offset = pio_add_program(pioFloppyRx.pio, &floppy_rx_program);
    pio_sm_config c = floppy_rx_program_get_default_config(offset);
    
    sm_config_set_jmp_pin(&c, pin);
    sm_config_set_in_pins(&c, pin);
    sm_config_set_clkdiv(&c, 1.0f);
    sm_config_set_in_shift(&c, false, false, 32);
    
    pio_gpio_init(pioFloppyRx.pio, pin);
    gpio_set_dir(pin, GPIO_IN);
    
    pio_sm_init(pioFloppyRx.pio, pioFloppyRx.sm, offset, &c);
    
    // Initialise X to 0xFFFFFFFF
    pio_sm_exec(pioFloppyRx.pio, pioFloppyRx.sm, pio_encode_set(pio_x, 0));
    pio_sm_exec(pioFloppyRx.pio, pioFloppyRx.sm, pio_encode_mov_not(pio_x, pio_x));
    pio_sm_set_enabled(pioFloppyRx.pio, pioFloppyRx.sm, true);    
}

// Capture data and return number of bytes read
uint32_t readFromPaula() {   
    while (!pio_sm_is_rx_fifo_empty(pioFloppyRx.pio, pioFloppyRx.sm)) pio_sm_get(pioFloppyRx.pio, pioFloppyRx.sm);  // flush anything else

    // The Sync bits are the LSB of each block as they arrive first
    // Read initial SYNC - NOTE the value from pio_sm_get_blocking COUNTS DOWN 
    uint32_t lastTime = pio_sm_get_blocking(pioFloppyRx.pio, pioFloppyRx.sm); 
    uint32_t nextTime = pio_sm_get_blocking(pioFloppyRx.pio, pioFloppyRx.sm); 

    // Time between bits 
    uint32_t bitPeriod = (lastTime-nextTime) / 16; 
    if ((bitPeriod < 100) || (bitPeriod > 250)) return 0;

    lastTime = nextTime;

    uint32_t data = 0;
    const int32_t bias = bitPeriod / 2;
    uint32_t totalTime = 0;    
    uint32_t bytesReceived = 0;

    while (!gpio_get(PIN_WRITEGATE)) {
        // Bit detected
        if (!pio_sm_is_rx_fifo_empty(pioFloppyRx.pio, pioFloppyRx.sm)) {
            uint32_t curr = pio_sm_get(pioFloppyRx.pio, pioFloppyRx.sm);

            // Work out what bit this should be
            uint32_t diff = lastTime - curr;
            totalTime += diff;
            lastTime = curr;

            uint32_t bit = (totalTime - bias) / bitPeriod;

            if (bit >= 8) {
                if (bytesReceived < PAULA_MAX_BYTES-2) IoBuffer[bytesReceived++] = data; else return bytesReceived;
                totalTime -= bitPeriod * 8;
                bit = (totalTime - bias) / bitPeriod;

                while (bit >= 8) {
                    if (bytesReceived < PAULA_MAX_BYTES-2) IoBuffer[bytesReceived++] = 0; else return bytesReceived;
                    totalTime -= bitPeriod * 8;
                    bit = (totalTime - bias) / bitPeriod;
                }
                data = 1 << (7 - bit);  // always set after loop
                
            } else {
                data |= 1 << (7 - bit);
            }
            // Resync
            totalTime = (bit + 1) * bitPeriod; 
        }
    }

    if (bytesReceived < PAULA_MAX_BYTES-2) IoBuffer[bytesReceived++] = data;
    if (bytesReceived&1) IoBuffer[bytesReceived++] = 0;
    return bytesReceived;
}

// Setup the transmit to amiga PIO
void setupWritingPIO(uint pin) {
#ifdef PICO_RP2350
    uint offset = pio_add_program(pioFloppyTX.pio, &floppy_tx_pico2_program);
#else    
    uint offset = pio_add_program(pioFloppyTX.pio, &floppy_tx_program);
#endif
    pio_sm_config c = floppy_tx_program_get_default_config(offset);
    sm_config_set_out_shift(&c, false, true, 16);  // shift left, autopull, 16 bits
    sm_config_set_set_pins(&c, pin, 1);
    sm_config_set_clkdiv(&c, 8);                   // 125MHz/8 = 15.625MHz

    pio_sm_set_consecutive_pindirs(pioFloppyTX.pio, pioFloppyTX.sm, pin, 1, true);
    pio_gpio_init(pioFloppyTX.pio, pin);
    pio_sm_init(pioFloppyTX.pio, pioFloppyTX.sm, offset, &c);
    pio_sm_set_enabled(pioFloppyTX.pio, pioFloppyTX.sm, true);
}

// Core 1 MAIN
void core1Main() {
    setupReadingPIO(PIN_WRITEDATA);  // incomming
    setupWritingPIO(PIN_READDATA);   // outgoing

    // DMA channel or writing at 500kbps
    int dma_chan = dma_claim_unused_channel(true);
    dma_channel_config cfg = dma_channel_get_default_config(dma_chan);
    channel_config_set_transfer_data_size(&cfg, DMA_SIZE_16);
    channel_config_set_read_increment(&cfg, true);   // advance through buffer
    channel_config_set_write_increment(&cfg, false);  // always write to PIO TX FIFO
    channel_config_set_dreq(&cfg, pio_get_dreq(pioFloppyTX.pio, pioFloppyTX.sm, true));  // pace to PIO TX FIFO

    while (1) {
        
        // Only allow reading from the Amiga when selected and on LOWER side
        if (isSelected) {
            switch (paulaMode) {
                case PaulaMode::pmAccessPoint: setWriteProtected(wifiScanActive); break;  // Write protected while scan is active
                case PaulaMode::pmConfig: setWriteProtected(false); break;   // Config can always be saved
                case PaulaMode::pmEthernet: setWriteProtected(false); break;  
                case PaulaMode::pmDisk: setWriteProtected(true);  break; // disk is always read-only  
            }

            // Allow 5ms of settle time between seeks. Min is 3ms, and max would be 15ms
            if (time_us_32() - lastStepTime > 5000) {
                // Special Disk Mode
                if (paulaMode == PaulaMode::pmDisk) {
                    if (isMotorActive) {
                        int currentTrack = track;                        
                        bool currentSide = isLowerSide();
                        gpio_put(PIN_LED_PAULAIO, 1);
                        const uint32_t bufferUsed = generateMFMTrackData(currentTrack, currentSide, (void*)IoBuffer)/2;                        
                        // Now we keep re-transmitting the same buffer over and over until MOTOR goes off or something else changes.
                        while ((currentTrack == track) && (currentSide == isLowerSide()) && (isSelected) && (isMotorActive) && gpio_get(PIN_RESET)) {
                            // Start transmission
                            gpio_put(PIN_INDEX, 0);   // index is used to help sync the data stream
                            dma_channel_configure(dma_chan, &cfg, &pioFloppyTX.pio->txf[pioFloppyTX.sm],IoBuffer,bufferUsed,true);
                            sleep_us(3000);   // wait 3ms, this allows the index to work properly.
                            gpio_put(PIN_INDEX, 1);
                            // Wait for one of these states to change
                            while ((currentTrack == track) && (currentSide == isLowerSide() && (isSelected) && (isMotorActive) && gpio_get(PIN_RESET) && dma_channel_is_busy(dma_chan)));
                            dma_channel_abort(dma_chan);
                        }
                        gpio_put(PIN_LED_PAULAIO, 0);                                                
                    }
                } else
                if (isLowerSide()) {
                    ///////////////////////////////////////////////
                    // The Amiga is sending us data - RECEIVE MODE
                    // We dont care about the status of MOTOR during this, we'll just assume its good
                    if (!gpio_get(PIN_WRITEGATE)) {                
                        gpio_put(PIN_LED_PAULAIO, 1);
                        const PaulaMode mode = paulaMode; // capture the mode at the start

                        uint32_t bytesReceived = readFromPaula();
                        if (bytesReceived) handleDataReceived(mode, bytesReceived);

                        // Wait until all data has finished being received
                        while (!gpio_get(PIN_WRITEGATE));
                        gpio_put(PIN_LED_PAULAIO, 0);
                    }            
                } else {
                    if (isMotorActive) {
                        //////////////////////////////////////////////////
                        // We're sending data TO the Amiga - TRANSMIT mode
                        // Create a buffer that we can send to the Amiga 
                        uint32_t bufferUsed;
                        int currentTrack = track;

                        if (paulaMode == PaulaMode::pmEthernet) {
                            bufferUsed = populateTransmitBufferForPackets() / 2;   // length is number of 16-bit words use
                        } else {
                            bufferUsed = populateTransmitBuffer(paulaMode) / 2;   // length is number of 16-bit words used
                        }
                        sleep_ms(1);
                        gpio_put(PIN_LED_PAULAIO, 1);         
                                                
                        // Now we keep re-transmitting the same buffer over and over until MOTOR goes off or something else changes.
                        while ((!isLowerSide()) && (isSelected) && gpio_get(PIN_RESET) && isMotorActive && (currentTrack == track)) {
                            // Start transmission
                            gpio_put(PIN_INDEX, 0);   // index is used to help sync the data stream
                            dma_channel_configure(dma_chan, &cfg, &pioFloppyTX.pio->txf[pioFloppyTX.sm],  (paulaMode == PaulaMode::pmEthernet) ? PacketBackup : IoBuffer,bufferUsed,true);
                            sleep_us(1000);   // wait 3ms, this allows the index to work properly.
                            gpio_put(PIN_INDEX, 1);

                            // Wait for one of these states to change
                            while ((!isLowerSide()) && (isSelected) && dma_channel_is_busy(dma_chan) && gpio_get(PIN_RESET) && isMotorActive && (currentTrack == track));
                            dma_channel_abort(dma_chan);    
                        }                        
                        gpio_put(PIN_LED_PAULAIO, 0);
                    } 
                }            
            }
        } 

        // Stay out of trouble during a reset
        while (!gpio_get(PIN_RESET))  tight_loop_contents();

        tight_loop_contents();
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CORE 0: WIFI I/O

// Wifi Access Point Scan Result
int wifiScanCallback(void *env, const cyw43_ev_scan_result_t *result) {
    if (rebootImminent) return 0;
    if (result->ssid_len < 1) return 0; // skip networks that don't broadcast their ssid
    if (result) {
        ScanResult sr;
        uint8_t len = result->ssid_len < sizeof(sr.ssid) - 1 ? result->ssid_len : sizeof(sr.ssid) - 1;
        memcpy(sr.ssid, result->ssid, len);
        sr.ssid[len] = '\0';   
        if (strlen(sr.ssid)) {
            // Block duplicate 
            for (int i = 0; i < scanResultCount; i++) 
                if (strcmp(scanResults[i].ssid, sr.ssid) == 0) {
                    if (result->rssi > SWAP16(scanResults[i].rssi)) scanResults[i].rssi = SWAP16(result->rssi);
                    return 0;
                }

            // New result
            if (scanResultCount < MAX_SCAN_RESULTS) {            
                sr.rssi = SWAP16(result->rssi);
                sr.channel = result->channel;                
                scanResults[scanResultCount++] = sr;
                //safePrintf("%i : Found Network %s\n", scanResultCount, sr.ssid);
                __dmb();
            }
        }
    }
    return 0;
}

// Begin a wifi connection 
void startWifiConnection() {
    static bool wasStaEnabled = false;
    if (wasStaEnabled) cyw43_arch_disable_sta_mode();
    wifiIsUp = false;
    cyw43_arch_enable_sta_mode();
    wasStaEnabled = true;
    if (strlen(settings.ssid)) {        
        if (strlen(settings.hostname)) netif_set_hostname(netif_default, settings.hostname); else netif_set_hostname(netif_default, "PaulaNET"); 
        safePrintf("Connecting to WIFI ssid %s\n", settings.ssid);
        safePrintf("RSSI: %d\n", liveStatus.rssi);
        isWifiConnecting = cyw43_arch_wifi_connect_async(settings.ssid, strlen(settings.pwd) ? settings.pwd : NULL, strlen(settings.pwd) ? CYW43_AUTH_WPA2_MIXED_PSK : CYW43_AUTH_OPEN ) == 0;
    } 
}


static netif_input_fn original_input = NULL;

// Capture the packets as they come in
static err_t rawEthernetReceive(struct pbuf *p, struct netif *netif) {    
    if (rebootImminent) {
        pbuf_free(p);
        return ERR_OK;
    }
    uint16_t len = p->tot_len;
    //if (rebootImminent) return original_input(p, netif);;
    flashWifiIO();

    if ((rxQueue.writeIndex + 1) % PACKET_QUEUE_SIZE != rxQueue.readIndex){
        pbuf_copy_partial(p, rxQueue.packets[rxQueue.writeIndex].data, len, 0);    
        rxQueue.packets[rxQueue.writeIndex].length = len;
        __dmb();
        rxQueue.writeIndex = (rxQueue.writeIndex + 1) % PACKET_QUEUE_SIZE;
    }

    //return original_input(p, netif);
    pbuf_free(p);
    return ERR_OK;
}

// Runs all of the WIFI stuff - was originally a repeating timer but causes too many issues
void handleWIFI() {

    // WIFI
    cyw43_arch_poll();

    // This only needs to run 5 times a second
    static uint32_t lastRun = time_us_32();
    if (time_us_32() - lastRun < 200000) return;
    lastRun = time_us_32();
    
    if (wifiScanActive) {
        static int counter = 0;
        counter = (counter+1) & 3;
        gpio_put(PIN_LED_WIFI, counter&1);       
        return; 
    } else
    if (isWifiConnecting) {
        gpio_put(PIN_LED_WIFI, !gpio_get(PIN_LED_WIFI)); 
        wifiIsUp = false;
    }

    int32_t rssi;
    if (cyw43_wifi_get_rssi(&cyw43_state, &rssi)) liveStatus.rssi = 0; else liveStatus.rssi = rssi;         
    int status = cyw43_wifi_link_status(&cyw43_state, CYW43_ITF_STA);

    //safePrintf("Wifi Status: %i\n", status);
    switch (status) {
        case CYW43_LINK_JOIN:
        case CYW43_LINK_NOIP: 
        case CYW43_LINK_UP:  gpio_put(PIN_LED_WIFI, 1); wifiIsUp = true; isWifiConnecting = false; break;
        default:             gpio_put(PIN_LED_WIFI, 0); wifiIsUp = false;break;
    }       
    switch (status) {
        case CYW43_LINK_DOWN         : liveStatus.linkStatus = 0; break;
        case CYW43_LINK_JOIN         : liveStatus.linkStatus = 1; break;
        case CYW43_LINK_NOIP         : liveStatus.linkStatus = 2; break;
        case CYW43_LINK_UP           : liveStatus.linkStatus = 3; break;
        case CYW43_LINK_FAIL         : liveStatus.linkStatus = 4; break;
        case CYW43_LINK_NONET        : liveStatus.linkStatus = 5; break;
        case CYW43_LINK_BADAUTH      : liveStatus.linkStatus = 6; break;        
    }    
    __dmb();    

#ifdef DEBUGGING        
    // Bootloader access
    char c = getchar_timeout_us(0);
    if (c == 'B') {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        sleep_ms(100);
        reset_usb_boot(0, 0);
    }
#endif
}


// Core0 main
int main() {   
    printLock = spin_lock_init(spin_lock_claim_unused(true));        
    stdio_init_all();   
    stdio_set_translate_crlf(&stdio_uart, false);   
    setvbuf(stdout, NULL, _IONBF, 0);
    gpio_put(PIN_LED_PAULAIO, 0);    gpio_put(PIN_LED_WIFI, 0);    gpio_put(PIN_LED_WIFIIO, 0);    
    preparePins();
    reset();     
    isSelected = false;
    isMotorActive = false;
    loadSettings();
/*
    for (;;) {
         // Bootloader access
        char c = getchar_timeout_us(0);
        if (c == 'B') {
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
            sleep_ms(100);
            reset_usb_boot(0, 0);
        }
        gpio_set_dir(PIN_DISKCHANGE, GPIO_OUT);
        gpio_put(PIN_LED_PAULAIO, 1);    gpio_put(PIN_LED_WIFI, 1);    gpio_put(PIN_LED_WIFIIO, 1); gpio_put(PIN_DISKCHANGE, 1);
        sleep_ms(5000);
        gpio_put(PIN_LED_PAULAIO, 0);    gpio_put(PIN_LED_WIFI, 0);    gpio_put(PIN_LED_WIFIIO, 0); gpio_put(PIN_DISKCHANGE, 0);
        sleep_ms(5000);
        safePrintf("Test\n");
    }
            
*/
   
    // Initialise the Wi-Fi chip, if that fails theres no real point in running
    if (cyw43_arch_init()) {
        safePrintf("Wi-Fi init failed\n");
        // Failure shown by ALL LEDS mostly OFF, but flashing briefly on once per second
        for (;;) {
            gpio_put(PIN_LED_PAULAIO, 0);    gpio_put(PIN_LED_WIFI, 0);    gpio_put(PIN_LED_WIFIIO, 0);
            sleep_ms(900);
            gpio_put(PIN_LED_PAULAIO, 1);    gpio_put(PIN_LED_WIFI, 1);    gpio_put(PIN_LED_WIFIIO, 1);
            sleep_ms(100);
        }
        return -1;
    }
 
    startWifiConnection();   
 
    // In main, after cyw43_arch_init:
    original_input = netif_default->input;
    netif_default->input = rawEthernetReceive;


    cyw43_wifi_get_mac(&cyw43_state, CYW43_ITF_STA, liveStatus.macAddress);
    gpio_put(PIN_LED_PAULAIO, 0);    gpio_put(PIN_LED_WIFI, 0);    gpio_put(PIN_LED_WIFIIO, 0);    
    multicore_launch_core1(core1Main); 
    // insert disk
    signalDiskChange();
    // Signal we're up and running
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);   
          
    while (true) {        
        handleWIFI();
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, !isMotorActive);   // LED onboard goes off when motor is active        
#ifdef DEBUGGING
        static int lastTrack = -1;
        if (lastTrack != track) {
           lastTrack = track;
           switch (paulaMode) {
            case dmAccessPoints: safePrintf("Track Changed to: %i  AP\n", lastTrack );break;
            case dmConfig:safePrintf("Track Changed to: %i  CONFIG\n", lastTrack );break;
            case dmEthernet:safePrintf("Track Changed to: %i  ETH\n", lastTrack );break;
            case pmDisk:safePrintf("Track Changed to: %i  DISK\n", lastTrack );break;
            default: safePrintf("Track Changed to: %i  ??\n", track );break;
           }
           
        }
#endif        

        // Wifi scan
        if (wifiRequestScan && !wifiScanActive) {
            wifiScanActive = true;
            wifiRequestScan = false;
            __dmb();
            scanResultCount = 0;
            wifiScanCounter = 0;
            cyw43_wifi_scan_options_t scanOptions = {0};
            if (cyw43_wifi_scan(&cyw43_state, &scanOptions, NULL, wifiScanCallback) == 0) {
                safePrintf("Begin wifi scan\n");
                //wifiScanActive = true;
                sleep_ms(100);
            } else {
                safePrintf("Wifi scan request failed\n");
                wifiScanActive = false;
            }
            __dmb();
        } else if ((wifiScanActive) && (!cyw43_wifi_scan_active(&cyw43_state))) {
            wifiScanCounter++;
            if (wifiScanCounter <= MAX_WIFI_SCANS) {
                cyw43_wifi_scan_options_t scanOptions = {0};
                if (cyw43_wifi_scan(&cyw43_state, &scanOptions, NULL, wifiScanCallback) == 0) {
                    //safePrintf("Repeat wifi scan\n");                    
                } else {
                    //safePrintf("Repeat wifi scan request failed\n");
                    wifiScanActive = false;
                    __dmb();
                }
            } else {
                // Sort by signal strength
                qsort(scanResults, scanResultCount, sizeof(ScanResult), [](const void* a, const void* b) {
                     return SWAP16(((ScanResult*)b)->rssi) - SWAP16(((ScanResult*)a)->rssi);
                });
                safePrintf("wifi scan complete\n");
                wifiScanActive = false;
                wifiRequestScan = false;
                __dmb();
            }
        }

        // Check for packets to send from core 1
        if (txQueue.readIndex != txQueue.writeIndex) {
            flashWifiIO();
            cyw43_send_ethernet(&cyw43_state, CYW43_ITF_STA, txQueue.packets[txQueue.readIndex].length, txQueue.packets[txQueue.readIndex].data, false); 
            txQueue.readIndex = (txQueue.readIndex + 1) % PACKET_QUEUE_SIZE;
        }

        // Monitor fot settings changes 
        if (multicore_fifo_rvalid()) {
            wifiReconnect = multicore_fifo_pop_blocking();
            safePrintf("Save settings requested %i\n",(int)wifiReconnect);
            saveSettings();
            multicore_fifo_push_blocking(0);
            safePrintf("Settings saved\n");
        }

        // Reconnect (sent from core1)
        if (wifiReconnect) {
            wifiReconnect = false;            
            startWifiConnection();   
        }

        // LED flashing
        if (!is_nil_time(wifiIOLedOff) && absolute_time_diff_us(get_absolute_time(), wifiIOLedOff) <= 0) {
            gpio_put(PIN_LED_WIFIIO, 0);
            wifiIOLedOff = nil_time;
        }

        // Compress one pending packet per loop iteration
        if (rxQueue.readIndex != rxQueue.writeIndex) {
            if ((compressedRxQueue.writeIndex + 1) % PACKET_QUEUE_SIZE != compressedRxQueue.readIndex) {              
                CompressionHeader hdr;
                hdr.originalSize = SWAP16(rxQueue.packets[rxQueue.readIndex].length);
                uint8_t* outputPosition = &compressedRxQueue.packets[compressedRxQueue.writeIndex].data[sizeof(CompressionHeader) * 2];
                // Prep compress and encode each packet for transmission
                if (settings.flags & FLAGS_PICO_COMPRESSION) {
                    compressedRxQueue.packets[compressedRxQueue.writeIndex].length = rleCompress(rxQueue.packets[rxQueue.readIndex].data, rxQueue.packets[rxQueue.readIndex].length, outputPosition, true, Core0RunBuffer);
                    hdr.compressedSize = SWAP16(compressedRxQueue.packets[compressedRxQueue.writeIndex].length);                    
                    // WORD padding
                    if (compressedRxQueue.packets[compressedRxQueue.writeIndex].length&1) {
                        *(outputPosition+compressedRxQueue.packets[compressedRxQueue.writeIndex].length) = 0;
                        compressedRxQueue.packets[compressedRxQueue.writeIndex].length++;
                    }
                    uint16_t lastBit = 0;
                    mfmEncode((uint8_t*)&hdr, sizeof(hdr), (uint16_t*)compressedRxQueue.packets[compressedRxQueue.writeIndex].data, &lastBit);
                } else {
                    compressedRxQueue.packets[compressedRxQueue.writeIndex].length = rxQueue.packets[rxQueue.readIndex].length << 1; 
                    hdr.compressedSize = SWAP16(compressedRxQueue.packets[compressedRxQueue.writeIndex].length | 0x8000);  // MFM
                    // Encode the header
                    uint16_t lastBit = 0;
                    mfmEncode((uint8_t*)&hdr, sizeof(hdr), (uint16_t*)compressedRxQueue.packets[compressedRxQueue.writeIndex].data, &lastBit);
                    mfmEncode(rxQueue.packets[rxQueue.readIndex].data, rxQueue.packets[rxQueue.readIndex].length, (uint16_t*)outputPosition, &lastBit);                     
                }
                compressedRxQueue.packets[compressedRxQueue.writeIndex].length += sizeof(CompressionHeader)*2;
                __dmb();
                compressedRxQueue.writeIndex = (compressedRxQueue.writeIndex + 1) % PACKET_QUEUE_SIZE;
                rxQueue.readIndex = (rxQueue.readIndex + 1) % PACKET_QUEUE_SIZE;
            }
        }
       
        // Handle system reset
        if (!gpio_get(PIN_RESET)) {
            rebootImminent = true;
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
            gpio_set_dir(PIN_READY, GPIO_IN); 
            gpio_put(PIN_LED_PAULAIO, 1);    
            gpio_put(PIN_LED_WIFI, 1);    
            gpio_put(PIN_LED_WIFIIO, 1);
            isSelected = false;
            isMotorActive = false;
            while (!gpio_get(PIN_RESET));
            // Now reboot the Pico!
            watchdog_reboot(0, 0, 0);
            for(;;);
        }               

        tight_loop_contents();
    }
}
