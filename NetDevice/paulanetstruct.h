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
#ifndef PAULANET_STRUCT
#define PAULANET_STRUCT

#include <proto/exec.h>
#include <exec/types.h>

// Maximum number of the ScanResult you will ever receive
#define MAX_SCAN_RESULTS            64

// Results from a WIFI Scan
struct ScanResult {
    BYTE                ssid[34];       // 32 + null + padding
    WORD                rssi;           // signal strength
    UBYTE               channel;
    UBYTE               padding;
};

// All the results from a scan
struct ScanResults {
    struct ScanResult ScanResults[MAX_SCAN_RESULTS];
    UWORD count;
};

#define MAX_TRANSFER_SIZE           16384
#define MIN_TRANSFER_SIZE           1536
#define ETH_MTU                     1518    // 1500 + ethernet header + some margin
#define MAX_PACKETS_PER_CALL        20      // Max packets to send or receive in one go

#define LINK_STATUS_DOWN            0       // Network down
#define LINK_STATUS_JOIN            1       // Joining
#define LINK_STATUS_NOIP            2       // No IP (normal for UP)
#define LINK_STATUS_UP              3       // Up with IP (shouldnt happen)
#define LINK_STATUS_FAIL            4       // Connect fail
#define LINK_STATUS_NONET           5       // SSID Network not found
#define LINK_STATUS_BADAUTH         6       // Auth failed

// Flags used in PaulaNETSettings
#define FLAGS_FROM_DRIVER        (1<<6)         // Bit set to signal that this request came from the driver not the config tool 
#define FLAGS_AMIGA_COMPRESSION  (1<<1)         // Set to make the Amiga apply RLE compression on data is transmits to the Pico, else MFM
#define FLAGS_PICO_COMPRESSION   (1<<0)         // Set to make the Pico apply RLE compression on data is transmits to the Pico, else MFM

// Settings saved on the PaulaNET device
struct PaulaNETSettings {
    UWORD               length;         // size of this structure - internally managed
    char                ssid[34];       // 32 + null + padding, WIFI to connect to
    char                pwd[66];        // 64 + null + padding, Password
    char                hostname[34];   // 32 + null + padding, Hostname that PaulaNET will appear on your router as
    UWORD               flags;          // Various settings (see above)
    UWORD               maxTxSize;      // Maximum size of data transmitted to the Pico in one chunk 
    UWORD               maxRxSize;      // Maximum size of data received from the Pico in one chunk
};

// Live status 
struct LiveStatus {
    UWORD                                length;                // size of this - internally managed
    UBYTE                                versionh, versionl;    // firmware version
    UBYTE                                macAddress[6];         // MAC address
    WORD                                 rssi;                  // Current signal strength
    UBYTE                                linkStatus;            // Link status. If connected then it will be to the ssid in the settings
    UBYTE                                reserved;              // rfu/padding
};

// A conversion between the RSSI value and a suitable number of BARS. Returns 0 to 5
int rssiToBars(SHORT rssi);

#define DEVICESTATUS_CANRECEIVE             1
#define DEVICESTATUS_CANSEND                2
#define DEVICESTATUS_ERROR                  128

// Callback
typedef void (*PacketReceivedCallback)(APTR userContext, UBYTE* packetData, int packetLength);



#endif