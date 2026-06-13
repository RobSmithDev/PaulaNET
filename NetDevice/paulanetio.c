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

#include <proto/exec.h>
#include <proto/dos.h>
#include <exec/types.h>
#include <devices/trackdisk.h>
#include "paulanetio.h"
#include "paulanetstruct.h"
#include <string.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <stdio.h>
#include <stdlib.h>

#include "debug.h"

typedef UBYTE DataMode;
#define dmAccessPoints          1
#define dmConfig                2
#define dmEthernet              3

#define PAULA_INDEX_PADDING      64         
#define PAULA_WRITE_PADDING      (4+2)    // (write padding in bytes, 4 for the two sync)
#define CHIPBUFFER_SIZE          (MAX_TRANSFER_SIZE + (PAULA_INDEX_PADDING<<2))

#define DATA_MAGIC_ID            0x5450
#define SYNCWORD                 0x44894489
#define TRACK_ACCESSPOINT        75
#define TRACK_CONFIG             76
#define TRACK_ETHERNET           77
#define TRACK_READ               1          // Upper side RECEIVE from PICO
#define TRACK_WRITE              0          // Lower side TRANSMIT TO PICO

#define COMPRESSION_WORSTCASE    (((ETH_MTU+127)/127)+2)

#define LZ4_MIN_MATCH       4

#define TRACK_OFFSET(track, side) ((track<<1) + side)
#define TRACK_SEEK_OFFSET(track, side) (((track<<1) + side) * NUMSECS * TD_SECTOR)

// I/O Data Header
struct DataHeader {    
    UWORD       magic;          // Should read Pt
    UWORD       checksum;       // Simple checksum
    UWORD       dataSize;       // Full data size
    UWORD       compressedSize; // Compressed (transmitted) size
    DataMode    dataMode;       // What this data is
    UBYTE       compressed;     // Is it compressed
    UBYTE       amigaCounter;   // For TX/RX Packet Management
    UBYTE       picoCounter;
};


// PaulaNET data
struct PaulaNET {
    struct ExecBase*            pnSysBase;
    struct DosLibrary*          pnDOSBase;
    struct MsgPort*             port;
    struct IOExtTD*             device;
    struct PaulaNETSettings     settings;
    struct LiveStatus           status;
    UWORD*                      buffer;
    UWORD*                      packetBackup;
    UWORD*                      compressionSpace;
    UWORD*                      writePosition;   
    UWORD                       spaceRemaining;
    BOOL                        deviceOpened;
    BOOL                        compressionMode;    
    BOOL                        scanActivated;
    UBYTE                       lastAmigaCounter;
    UBYTE                       lastPicoCounter;
    UBYTE                       packetCounter;
    BOOL                        ackLastPacketData;
};

#define SysBase dev->pnSysBase
#define DOSBase dev->pnDOSBase


struct PacketsHeader {
    UBYTE numPackets;    
    UBYTE flags; 
};

struct CompressionHeader {
    UWORD compressedSize;     
    UWORD originalSize;    
};

static const UWORD stuffChar[] = {0x16c0, 0x4e75};	
#ifdef DEBUG    
void ssprintf(struct PaulaNET* dev, const char *messageFormat, char* buf, ...) {
    va_list args;
    va_start(args, buf);  /* FIX: last named param before ... is buf, not messageFormat */
    RawDoFmt((STRPTR)messageFormat, (APTR)args, (void (*)(void))stuffChar, buf);
    va_end(args);
}

void hexDump(struct PaulaNET* out, const void* data, ULONG length) {
    const UBYTE* p = (const UBYTE*)data;
    char buffer[160];  /* slightly larger to be safe */
    char tmp[32];
    ULONG i, j;
    for (i = 0; i < length; i += 16) {
        buffer[0] = '\0';
        ssprintf(out, "%04lx: ", tmp, i);
        strcat(buffer, tmp);
        for (j = 0; j < 16; j++) {
            if (i + j < length) {
                ssprintf(out, "%02lx ", tmp, (ULONG)p[i+j]);
                strcat(buffer, tmp);
            } else strcat(buffer, "   ");             
        }

        strcat(buffer, " ");
        for (j = 0; j < 16 && i + j < length; j++) {
            UBYTE c = p[i+j];
            tmp[0] = (c >= 32 && c < 127) ? (char)c : '.';
            tmp[1] = '\0';
            strcat(buffer, tmp);
        }

        strcat(buffer, "\n");
        D((buffer));
    }
}
#endif



// Formatted logging to console window

static void logMessagef(struct PaulaNET* dev, const char *messageFormat, ...) {
#ifdef DEBUG
	char buf[100];
    va_list args;
    va_start(args, messageFormat);	 
    RawDoFmt((STRPTR)messageFormat, (APTR)args, (void (*)(void))stuffChar, buf);
	va_end(args);
	D((buf));
#endif
}


// Calculates the checksum on some data and returns it
static UWORD calcChecksum(UWORD* data, int bytes) {
    WORD checksum = 0;
    while (bytes>0) {
        checksum ^= *data++;
        bytes-=2;
    }
    return checksum;
}

// Reverse pass through the buffer to calculate run lengths at each position.
// On exit, runs[i] contains the number of consecutive identical bytes starting at position i, capped at MAX_RUN.
static void buildRunTable(const UBYTE* src, const UWORD len, UBYTE* runs) {
    const UBYTE* s = src + len - 1;
    UBYTE* r = runs + len - 1;
    *r = 1;
    while (s > src) {
        --s; --r;
        if (s[0] == s[1]) r[0] = (r[1] < 127) ? r[1] + 1 : 127;
        else r[0] = 1;
    }
}

// Compress data using our RLE schema. for each byte:
//      if MSB is set, then the remaining 7-bits state how many of the following bytes should be copied from the stream
//      if MSB is clear, then the remaining 7-bits state how many times to repeat the next byte
int rleCompress(struct PaulaNET* dev, const UBYTE* src, UWORD srcLen, UBYTE* dst, UBYTE* runsBuffer) {
    UWORD srcPos = 0;
    UWORD dstPos = 0;
    UBYTE run;
    UWORD rawStart;
    UBYTE  rawLen;
    UBYTE* rawHdr;
    UBYTE absorb;

    buildRunTable(src, srcLen, runsBuffer);
    while (srcPos < srcLen) {
        // See how many repeats there are at this position
        run = runsBuffer[srcPos];
        // >=3 is the threshold when RLE becomes more efficient than storing the raw data
        if (run >= 3) {
            dst[dstPos++] = run;
            dst[dstPos++] = src[srcPos];
            srcPos += run;
        }
        else {
            rawStart = srcPos;
            rawLen = 0;
            rawHdr = &dst[dstPos++];

            // Work out how much raw data to store
            while (srcPos < srcLen && rawLen < 127) {
                run = runsBuffer[srcPos];
                if (run >= 3) break;  // better as RLE
                absorb = run;
                if (rawLen + absorb > 127) absorb = 127 - rawLen;
                srcPos += absorb;
                rawLen += absorb;
            }
            if (rawLen == 0) dstPos--; else {
                *rawHdr = 0x80 | rawLen;
                CopyMem(&src[rawStart], &dst[dstPos], rawLen);
                dstPos += rawLen;
            }
        }
    }

    // Must be even
    if (dstPos&1) dst[dstPos++] = 0;

    return dstPos;
}

// Data decompressor based on the above schema. 
UWORD rleDecompress(struct PaulaNET* dev, const UBYTE* src, UWORD srclen, UBYTE* dst, UWORD dstmax) {
    UWORD srcPos = 0;
    UWORD dstPos = 0;
    UBYTE hdr, count;
    UBYTE* tgt;
    while (srcPos < srclen) {
        hdr = src[srcPos++];
        count = hdr & 0x7F;
        // Shouldn't happen, but good for preventing damaged data causing an issue
        if (dstPos + count > dstmax) break;
        if (hdr & 0x80) {
            // Copy raw bytes from stream
            if (srcPos + count > srclen) break;            
            CopyMem((const APTR)&src[srcPos], &dst[dstPos], count);
            srcPos += count;
        }
        else 
        if (count) {
            // Repeat byte a specific number of times
            if (srcPos >= srclen) break;
            int count2 = count;
            tgt = &dst[dstPos];
            while (count2--) *tgt++ = src[srcPos];
            srcPos++;                        
        }
        dstPos += count;
    }
    return dstPos;
}

// Decompress LZ4 block format data.
UWORD lz4Decompress(struct PaulaNET* dev, const UBYTE* src, UWORD srcLen, UBYTE* dst, UWORD dstMax) {
    UWORD srcPos = 0;
    UWORD dstPos = 0;

    while (srcPos < srcLen) {
        // --- Token ---
        const UBYTE token    = src[srcPos++];
        UWORD      litLen   = token >> 4;
        UWORD      matchLen = token & 0x0F;

        // --- Extra literal length ---
        if (litLen == 15) {
            UBYTE extra;
            do { extra = src[srcPos++]; litLen += extra; } while (extra == 255);
        }

        // --- Literals ---
        if (dstPos + litLen > dstMax) break;
        if (srcPos + litLen > srcLen) break;
        memcpy(&dst[dstPos], &src[srcPos], litLen);
        srcPos += litLen;
        dstPos += litLen;

        // --- End of stream: final sequence has no match ---
        if (srcPos >= srcLen) break;

        // --- Match offset ---
        if (srcPos + 2 > srcLen) break;
        const UWORD offset = ((UWORD)src[srcPos] << 8) | ((UWORD)src[srcPos + 1]);
        srcPos += 2;
        if (offset == 0 || offset > dstPos) break;  // corrupt

        // --- Extra match length ---
        matchLen += LZ4_MIN_MATCH;
        if ((token & 0x0F) == 15) {
            UBYTE extra;
            do { extra = src[srcPos++]; matchLen += extra; } while (extra == 255);
        }

        // --- Match copy (may overlap, so byte-by-byte) ---
        if (dstPos + matchLen > dstMax) break;
        const UBYTE* matchSrc = &dst[dstPos - offset];
        for (UWORD i = 0; i < matchLen; i++) dst[dstPos++] = matchSrc[i];
    }

    return dstPos;
}

// This generate a valid MFM output for the data input, it's not how it should be done (bits being spread into bytes, its an odd/even approach, which is faster)
int mfmEncode(const UBYTE* src, UWORD srcLen, UWORD* dst, UWORD* lastBit) {
    UWORD* out = dst;
    for (UWORD i = 0; i < srcLen; i++) {
        UWORD word = ((UWORD)(src[i] & 0xAA) << 7) | (src[i] & 0x55);

        // Compute clock bits (clock bit values are inverted)
        UWORD lShifted = (word << 1);
        UWORD rShifted = (word >> 1) | (*lastBit << 15);
        UWORD cBitsInv = lShifted | rShifted;

        // Reverse the computed clock bits
        UWORD cBits = cBitsInv ^ 0xAAAA;
        word |= cBits;
        *out++ = word;
        *lastBit = word;
    }
    return ((UBYTE*)out) - ((UBYTE*)dst);
}

// MFM decoder that matches the encoder on PaulaNET device
int mfmDecode(const UWORD* src, UWORD srcLenBytes, UBYTE* dst, UWORD dstmax) {
    if ((srcLenBytes>>1) > dstmax) srcLenBytes = dstmax<<1;
    for (UWORD i = 0; i < srcLenBytes; i += 2) {
        UWORD noclock = *src & 0x5555;
        *dst++ = (noclock >> 7) | noclock;
        src++;
    }
    return srcLenBytes >> 1;
}

// Looks at the incoming data stream, and works out how many bits (including bytes) need to be shifted to re-align - data MUST be longword aligned
static int findDataOffset(void* data, int dataLength) {
    ULONG* dataPos = (ULONG*)data;
    
    if (dataLength < PAULA_INDEX_PADDING) return -1;  
    ULONG current = *dataPos;
    // Unlikely
    if (current == SYNCWORD) return 0;

    for (int longword=0; longword<PAULA_INDEX_PADDING>>1; longword++) {
        dataPos++;
        ULONG next = *dataPos;
        for (int bits=1; bits<=32; bits++) {
            current = (current << 1) | (next >> 31);
            next <<= 1;
            if (current == SYNCWORD) return (longword*32) + bits;
        }
    }
    return -1;
}

// Align the data to the bit shifted position - data must be longword aligned
static void shiftBuffer(struct PaulaNET* dev, void* data, int length, int bitOffset) {
    ULONG* dataPos = (ULONG*)data + (bitOffset >> 5);
    ULONG* output  = (ULONG*)data;
    int bitShift   = bitOffset & 31;
    if (bitShift == 0) {
        if (bitOffset) CopyMem(dataPos, output, length);
        return;
    }
    int longLength = (length + 3) >> 2;

    ULONG current = *dataPos++;
    current <<= bitShift;

    while (longLength--) {
        ULONG next = *dataPos++;
        *output++ = current | (next >> (32 - bitShift));
        current = next << bitShift;
    }   
}

// Takes the input data and shifts it so it's aligned, then returns the start of the COMPRESSED data
// The header is copied into the DataHeader supplied 
// If not found, or the checksum fails, returns NULL. Output buffer is from "compression space"
// If skipChecksum is set then checkum is not checked, and data length isnt validated. Data is also not decompressed
static APTR alignData(struct PaulaNET* device, void* data, int dataLength, struct DataHeader* header, BOOL headerOnly) {
    int offset = findDataOffset(data, dataLength);
    if (offset < 0) {
#ifdef DEBUGGING
        FPrintf(Output(),"SYNC NOT FOUND\n");
#endif        
        D(("No Alignment"));
        return NULL; // not found    
    }
    offset += 32; // skip past sync word (4 bytes)
    UBYTE* dataStart;
    UWORD receivedDataLength;

    // The DataHeader is MFM encoded straight after this. This is to make sure its always received correctly        
    if ((offset & 15) == 0) {
        dataStart = ((UBYTE*)data) + (offset>>3);
        mfmDecode((WORD*)dataStart, sizeof(struct DataHeader)*2, (UBYTE*)header, sizeof(struct DataHeader));        
        // Validate header
        if (header->magic != DATA_MAGIC_ID) return NULL;
        if (headerOnly) return (APTR)header;
        receivedDataLength = header->compressed ? header->compressedSize : header->dataSize;
        if (receivedDataLength > dataLength - ((offset/32) + (sizeof(struct DataHeader)*2))) {
            D(("Insufficient Data Received [1]"));
            return NULL; 
        }
        dataStart += sizeof(struct DataHeader) * 2; 
    } else {
        // Data needs aligning first
        shiftBuffer(device, data, sizeof(struct DataHeader)*2, offset);
        mfmDecode((WORD*)data, sizeof(struct DataHeader)*2, (UBYTE*)header, sizeof(struct DataHeader));
        offset += sizeof(struct DataHeader) * 16;        // 8 bits + mfm encoding so double  
        // Validate header
        if (header->magic != DATA_MAGIC_ID) return NULL;
        if (headerOnly) return (APTR)header;
        receivedDataLength = header->compressed ? header->compressedSize : header->dataSize;
        // The below can't happen, so check early
        if (receivedDataLength> dataLength - ((offset/32) + (sizeof(struct DataHeader)*2))) {
            D(("Insufficient Data Received [1]"));
            return NULL; 
        }
        shiftBuffer(device, data, receivedDataLength, offset);        
        dataStart = (UBYTE*)data;
    }

    // Once here, the header is valid, we know what type of data this is, and how big it should be.
    // The next step is validating the data    
    if (calcChecksum((UWORD*)dataStart, header->compressed ? header->compressedSize : header->dataSize) != header->checksum) {
        D(("Checksum Failed"));
        return NULL;
    }
    
#ifdef DEBUGGING
    FPrintf(Output(), "Data Size: %ld\n", (LONG)header->dataSize);
    FPrintf(Output(), "Compressed Size: %ld\n", (LONG)header->compressedSize);
#endif

    // Produce standard output
    switch (header->compressed & 3) {
        // RAW data
        case 0: return dataStart; 

        // Compressed Data
        case 1: if (rleDecompress(device, dataStart, header->compressedSize, (UBYTE*)device->compressionSpace, CHIPBUFFER_SIZE) != header->dataSize) return NULL; 
                return device->compressionSpace;

        // MFM encoded
        case 2: if (mfmDecode((UWORD*)dataStart, header->compressedSize, (UBYTE*)device->compressionSpace, CHIPBUFFER_SIZE) != header->dataSize) return NULL; 
                return device->compressionSpace;

        default: 
            D(("Invalid Compression Mode"));
            return NULL;
    }    
}

// Transmit some data to PaulaNET
BOOL transmitToPaulaNET(struct PaulaNET* dev, int track, DataMode mode, int dataSize, APTR data) {
    // Step 1: Prepare
    dev->buffer[0] = 0x0001;
    dev->buffer[1] = 0x0001;
    struct DataHeader* header = (struct DataHeader*)&dev->buffer[2];
    header->magic = DATA_MAGIC_ID;
    header->dataSize = dataSize;
    header->dataMode = mode;
    header->compressed = (dev->settings.flags & FLAGS_AMIGA_COMPRESSION) ? 1 : 0;    
#ifdef DRIVERMODE    
    header->compressed |= FLAGS_FROM_DRIVER;
#endif    
    header->amigaCounter = dev->lastAmigaCounter;
    header->picoCounter = dev->lastPicoCounter;

    if (header->compressed) {
        header->compressedSize = rleCompress(dev, (UBYTE*)data, dataSize, (UBYTE*)&header[1], (UBYTE*)dev->compressionSpace);
        header->checksum = calcChecksum((UWORD*)&header[1], header->compressedSize);
    } else {
        UWORD lastBit = 0;
        mfmEncode((UBYTE*)data, dataSize, (UWORD*)&header[1], &lastBit);
        header->compressedSize = dataSize<<1;
        header->checksum = calcChecksum((UWORD*)&header[1], header->compressedSize);
    }    
    UBYTE* dataEnd = ((UBYTE*)&header[1]) + header->compressedSize;
  
#ifdef DEBUGGING
    hexDump(Output(), (const void*)&dev->buffer[2], 128);
#endif    

    for (int i=0; i<16; i++) *dataEnd++ = 0xAA;
    // Word align
    if ((dataEnd - ((UBYTE*)dev->buffer)) & 1)  *dataEnd++ = 0xAA;        

    // Step 2: Transmit
    dev->device->iotd_Req.io_Command = ETD_RAWWRITE;
    dev->device->iotd_Req.io_Flags   = 0;
    dev->device->iotd_Count          = 0xFFFFFFFF;
    dev->device->iotd_Req.io_Data    = (APTR)dev->buffer;
    dev->device->iotd_Req.io_Length  = dataEnd - ((UBYTE*)dev->buffer);
    dev->device->iotd_Req.io_Offset  = TRACK_OFFSET(track, TRACK_WRITE);

    return DoIO((struct IORequest *)dev->device) == 0;
}

// Searches the system for the PaulaNET device, if found it returns an open handle to it
APTR paulaNetFindAndOpen(struct ExecBase *Sysbase, struct DosLibrary *Dosbase) {
    APTR device = paulaNetOpen(Sysbase, Dosbase, 1); // most likely on device 1
    if (!device) device = paulaNetOpen(Sysbase, Dosbase, 3);  // very unlikely
    if (!device) device = paulaNetOpen(Sysbase, Dosbase, 2);  // also unlikely
    if (!device) device = paulaNetOpen(Sysbase, Dosbase, 0);  // I'd be suprised
    return device;
}

static BOOL paulaNetInternalGetConfig(APTR device);

// Close the PaulaNET device
void paulaNetClose(APTR device) {
    if (!device) return;

    struct PaulaNET* dev = (struct PaulaNET*)device;
    
    if (dev->device) {
        // Switch off motor
        if (dev->deviceOpened) {
            dev->device->iotd_Req.io_Command = TD_MOTOR;
            dev->device->iotd_Req.io_Flags   = 0;
            dev->device->iotd_Req.io_Length  = 0;
            DoIO((struct IORequest *)dev->device);
            CloseDevice((struct IORequest *)dev->device);
        }
        DeleteIORequest((struct IORequest *)dev->device);
    }
    if (dev->port) DeleteMsgPort((struct MsgPort *)dev->port);
    if (dev->buffer) FreeMem(dev->buffer, CHIPBUFFER_SIZE);
    if (dev->compressionSpace) FreeMem(dev->compressionSpace, CHIPBUFFER_SIZE);

    FreeMem(device, sizeof(struct PaulaNET));
}

#ifdef DEBUGGING
void printPaulaNetScanResults(struct ScanResults* results) {
    BPTR out = Output();
    FPrintf(out, "=== WiFi Scan Results (%lu networks) ===\n", (ULONG)results->count);
    
    for (UWORD i = 0; i < results->count; i++) {
        struct ScanResult* sr = &results->ScanResults[i];        
        FPrintf(out, "\n[%lu] SSID:    %s\n",    (ULONG)(i+1), (ULONG)sr->ssid);
        FPrintf(out, "     RSSI:    %ld dBm\n",  (LONG)sr->rssi);
        FPrintf(out, "     Channel: %lu\n",       (ULONG)sr->channel);
    }
}
void printPaulaNetInfo(struct PaulaNET* dev) {
    BPTR out = Output();
    
    // Settings
    FPrintf(out, "=== PaulaNET Settings ===\n");
    FPrintf(out, "Length:     %lu\n",   (ULONG)dev->settings.length);
    FPrintf(out, "SSID:       %s\n",    (ULONG)dev->settings.ssid);
    FPrintf(out, "Password:   %s\n",    (ULONG)dev->settings.pwd);
    FPrintf(out, "Hostname:   %s\n",    (ULONG)dev->settings.hostname);
    FPrintf(out, "MaxTxSize:  %lu\n",   (ULONG)dev->settings.maxTxSize);
    FPrintf(out, "MaxRxSize:  %lu\n",   (ULONG)dev->settings.maxRxSize);
    FPrintf(out, "Flags:      0x%02lx\n", (ULONG)dev->settings.flags);
        
    // Status
    FPrintf(out, "\n=== PaulaNET Status ===\n");
    FPrintf(out, "Length:     %lu\n",   (ULONG)dev->status.length);
    FPrintf(out, "Version:    %lu.%lu\n", (ULONG)dev->status.versionh, (ULONG)dev->status.versionl);
    FPrintf(out, "MAC:        %02lx:%02lx:%02lx:%02lx:%02lx:%02lx\n",
        (ULONG)dev->status.macAddress[0],
        (ULONG)dev->status.macAddress[1],
        (ULONG)dev->status.macAddress[2],
        (ULONG)dev->status.macAddress[3],
        (ULONG)dev->status.macAddress[4],
        (ULONG)dev->status.macAddress[5]);
    FPrintf(out, "RSSI:       %ld dBm\n", (LONG)dev->status.rssi);
    FPrintf(out, "Link:       %s\n",    dev->status.linkStatus ? (ULONG)"Connected" : (ULONG)"Disconnected");
}
#endif

// Open the PaulaNET device
APTR paulaNetOpen(struct ExecBase *Sysbase, struct DosLibrary *Dosbase, int deviceIndex) {
    // Prep funcs
    struct PaulaNET t;
    t.pnSysBase = Sysbase;
    t.pnDOSBase = Dosbase;
    struct PaulaNET* dev = &t;

    struct PaulaNET* tmp = (struct PaulaNET*)AllocMem(sizeof(struct PaulaNET), MEMF_CLEAR);
    if (!tmp) return NULL;
    tmp->pnSysBase = Sysbase;
    tmp->pnDOSBase = Dosbase;

    tmp->port = CreateMsgPort();
    if (!tmp->port) {
        paulaNetClose(tmp);
        return NULL;
    }
   
    tmp->device = (struct IOExtTD *)CreateIORequest(tmp->port, sizeof(struct IOExtTD));
    if (!tmp->device) {
        paulaNetClose(tmp);        
        return NULL;
    }

    tmp->buffer = (UWORD*)AllocMem(CHIPBUFFER_SIZE, MEMF_CHIP | MEMF_PUBLIC | MEMF_CLEAR);
    if (!tmp->buffer) {
        paulaNetClose(tmp);        
        return NULL;
    }

    // This can be any type of memory
    tmp->compressionSpace = (UWORD*)AllocMem(CHIPBUFFER_SIZE, MEMF_CLEAR);
    if (!tmp->compressionSpace) {
        paulaNetClose(tmp);        
        return NULL;
    }

    // One extra block to backup pacxkets to be sent
    tmp->packetBackup = (UWORD*)AllocMem(CHIPBUFFER_SIZE, MEMF_CHIP | MEMF_PUBLIC | MEMF_CLEAR);
     if (!tmp->packetBackup) {
        paulaNetClose(tmp);        
        return NULL;
    }

    if (OpenDevice(TD_NAME, deviceIndex, (struct IORequest *)tmp->device, 36)) {
        paulaNetClose(tmp);  
        return NULL;
    }

    // See if there's a disk present
    tmp->deviceOpened = TRUE;
    tmp->device->iotd_Req.io_Command = TD_CHANGESTATE;
    tmp->device->iotd_Req.io_Flags   = 0;
    tmp->device->iotd_Req.io_Length  = 0;
    tmp->device->iotd_Req.io_Data    = NULL;
    tmp->lastAmigaCounter = 0xFF;
    tmp->lastPicoCounter = 0xFF;
    tmp->packetBackup[0] = 0x0001;
    tmp->packetBackup[1] = 0x0001;
    tmp->packetCounter = 0;
    tmp->ackLastPacketData = FALSE;

    DoIO((struct IORequest*)tmp->device);
    if (tmp->device->iotd_Req.io_Actual) {
        paulaNetClose(tmp);  
        return NULL;
    }

    // Validate this is PaulaNET
    if (!paulaNetInternalGetConfig(tmp)) {
        Delay(50);
        // It might fail the first time
        if (!paulaNetInternalGetConfig(tmp)) {
            // This seek causes the device to simulate a disk eject, resetting Workbench's track index
            // Something weird causes a track mis-alignment. I confirmed on Gotek same issue after reboot.
            // Dont know if this is a Pistorm or Kickstart 3.2 issue or both 
            Delay(10);
            tmp->device->iotd_Req.io_Flags   = 0;
            tmp->device->iotd_Count          = 0xFFFFFFFF;
            tmp->device->iotd_Req.io_Command = TD_SEEK;
            tmp->device->iotd_Req.io_Offset  = TRACK_SEEK_OFFSET(79,0);
            DoIO((struct IORequest *)tmp->device);
            Delay(10);
            tmp->device->iotd_Req.io_Command = TD_SEEK;
            tmp->device->iotd_Req.io_Offset  = TRACK_SEEK_OFFSET(0,0);
            DoIO((struct IORequest *)tmp->device);
            Delay(10);
            for (int i=0; i<15; i++) {                
                Delay(10);  
                if (!paulaNetInternalGetConfig(tmp)) {
                    paulaNetClose(tmp);  
                    return NULL;
                } else break;
            }
        }
    }

#ifdef DEBUGGING    
    printPaulaNetInfo(tmp);
#endif    
    return (APTR)tmp;
}

// Starts a scan for access points
BOOL paulaNetBeginNetworkScan(APTR device) {
    struct PaulaNET* dev = (struct PaulaNET*)device;
    if (!dev) return FALSE;
    if (dev->scanActivated) {
        paulaNetIsAccessPointScanActive(device);
        if (dev->scanActivated) return FALSE; // not allowed while busy
    }

    // Transmit something back to PaulaNET, doesn't matter what
    if (!transmitToPaulaNET(dev, TRACK_ACCESSPOINT, dmAccessPoints, 1024, dev->buffer)) return FALSE;
    dev->scanActivated = TRUE;

    Delay(1);

      // Check actual status by seeking to the track
    dev->device->iotd_Req.io_Command = TD_SEEK;
    dev->device->iotd_Count          = 0xFFFFFFFF;
    dev->device->iotd_Req.io_Flags   = 0;
    dev->device->iotd_Req.io_Offset  = TRACK_SEEK_OFFSET(TRACK_ACCESSPOINT, TRACK_WRITE);
    DoIO((struct IORequest *)dev->device);

    Delay(5);

    return TRUE;
}

// Returns TRUE if an access point scan is active
BOOL paulaNetIsAccessPointScanActive(APTR device) {
    struct PaulaNET* dev = (struct PaulaNET*)device;
    if (!dev) return FALSE;
    if (!dev->scanActivated) return FALSE;
    
    // Check actual status by seeking to the track
    dev->device->iotd_Req.io_Command = TD_SEEK;
    dev->device->iotd_Count          = 0xFFFFFFFF;
    dev->device->iotd_Req.io_Flags   = 0;
    dev->device->iotd_Req.io_Offset  = TRACK_SEEK_OFFSET(TRACK_ACCESSPOINT, TRACK_WRITE);
    if (DoIO((struct IORequest *)dev->device)) return TRUE;

    Delay(1);

    // and checking the write protect status, weird I know
    dev->device->iotd_Req.io_Flags = 0;
    dev->device->iotd_Req.io_Command = TD_PROTSTATUS;    
    if (DoIO((struct IORequest *)dev->device)) return TRUE;

    // Shouldn't happen
    if (dev->device->iotd_Req.io_Error != 0) return TRUE;
    
    // Check result
    if (dev->device->iotd_Req.io_Actual) {
        // Write protected
        return TRUE; // scan still active
    } else {
        // Not write protected - scan complete
        dev->scanActivated = FALSE;
        return FALSE;
    }
}

// Receive data from PaulaNET with retry
APTR receiveFromPaulaNet(struct PaulaNET* dev, int track, int dataLen, DataMode mode, struct DataHeader* header, BOOL headerOnly) {
    APTR data = NULL;

    // Allow retries - this is only usually needed if somehow the track number gets out of sync
    for (int retry=0; retry<3; retry++) {
        if (retry == 2) {
            Delay(1);
            dev->device->iotd_Req.io_Flags   = 0;
            dev->device->iotd_Count          = 0xFFFFFFFF;
            dev->device->iotd_Req.io_Command = TD_SEEK;
            dev->device->iotd_Req.io_Offset  = TRACK_SEEK_OFFSET(79,TRACK_READ);
            DoIO((struct IORequest *)dev->device);
            Delay(1);
        }

        // Read
        dev->device->iotd_Req.io_Command = ETD_RAWREAD;
        dev->device->iotd_Req.io_Flags   = IOTDF_INDEXSYNC;
        dev->device->iotd_Count          = 0xFFFFFFFF;
        dev->device->iotd_Req.io_Data    = (APTR)dev->buffer;
        dev->device->iotd_Req.io_Length  = ((dataLen+1)&0xFFFFFFFE) + (sizeof(struct DataHeader)*2) + (PAULA_INDEX_PADDING * 2);
        if (dev->device->iotd_Req.io_Length > CHIPBUFFER_SIZE) dev->device->iotd_Req.io_Length = CHIPBUFFER_SIZE;
        if (dev->device->iotd_Req.io_Length <1024) dev->device->iotd_Req.io_Length = 1024;
        dev->device->iotd_Req.io_Offset  = TRACK_OFFSET(track, TRACK_READ);
        BYTE t = DoIO((struct IORequest *)dev->device);
    
        if (t) {
            D(("IO Error"));
            continue;
        } else {    
            data = alignData(dev, dev->buffer, dev->device->iotd_Req.io_Length, header, headerOnly);
            if (!data) {
                D(("Align Failed"));
                continue;
            }
        }
        if (header->dataMode != mode) {
            D(("Wrong Mode"));
            continue;
        }
#ifdef DRIVERMODE
        // If this matches then the Pico has received our last batch of packets
        if (header->amigaCounter == dev->lastAmigaCounter) dev->writePosition = NULL;
#endif        
        return data;
    }
    D(("No Data"));
    return NULL;
}

// Populates the results from the scan. Returns the number found.
BOOL paulaNetGetAccessPoints(APTR device, struct ScanResults* results) {
    if (!results) return FALSE;
    results->count = 0;

    struct PaulaNET* dev = (struct PaulaNET*)device;
    if (!dev) return FALSE;
     if (dev->scanActivated) {
        paulaNetIsAccessPointScanActive(device);
        if (dev->scanActivated) return FALSE; // not allowed while busy
    }

    struct DataHeader header;
    APTR data = receiveFromPaulaNet(dev, TRACK_ACCESSPOINT, sizeof(struct ScanResults), dmAccessPoints, &header, FALSE);
    if (!data) return FALSE;

    results->count = *((UWORD*)data);
    if (results->count > 127) return FALSE; // never allowed this many anyway
    if (results->count<1) return TRUE;        
    if (results->count > MAX_SCAN_RESULTS) results->count = MAX_SCAN_RESULTS;
    if (results->count* sizeof(struct ScanResult) > header.dataSize-2) return FALSE; // shouldn't happen
    CopyMem(((UBYTE*)data)+2, results->ScanResults, sizeof(struct ScanResult) * results->count);
    return TRUE;
}

// Fetch the config from the device 
static BOOL paulaNetInternalGetConfig(APTR device) {
    struct PaulaNET* dev = (struct PaulaNET*)device;
    if (!dev) return FALSE;

     struct DataHeader header;
    APTR data = receiveFromPaulaNet(dev, TRACK_CONFIG, sizeof(struct PaulaNETSettings)  + sizeof(struct LiveStatus), dmConfig, &header, FALSE);
    if (!data) return FALSE;  
    struct PaulaNETSettings* recv = (struct PaulaNETSettings*)data;
    {
        int len = sizeof(struct PaulaNETSettings);
        if (recv->length < len) {
            memset(&dev->settings, 0, len);
            len = recv->length;
        }
        CopyMem(recv, &dev->settings, len);    
    }

    // Now handle the live status
    {
        struct LiveStatus* recvs = (struct LiveStatus*)(((UBYTE*)data) + recv->length);
        int len = sizeof(struct LiveStatus);
        if (recvs->length < len) {
            memset(&dev->status, 0, len);
            len = recvs->length;
        }
        CopyMem(recvs, &dev->status, len);    
    }

    return TRUE;
}

// Public access to settings
void paulaNetGetConfig(APTR device, struct PaulaNETSettings* settings) {
    struct PaulaNET* dev = (struct PaulaNET*)device;
    if (!dev) return;

    CopyMem(&dev->settings, settings, sizeof(struct PaulaNETSettings));    
}

// Fetch the config from the device - BLOCKING
BOOL paulaNetPutConfig(APTR device, struct PaulaNETSettings* settings) {
    struct PaulaNET* dev = (struct PaulaNET*)device;
    if (!dev) return FALSE;
    if (!settings) return FALSE;

    // Transmit something back to PaulaNET
    if (!transmitToPaulaNET(dev, TRACK_CONFIG, dmConfig, sizeof(struct PaulaNETSettings), settings)) return FALSE;
    // Take a copy if they were saved
    dev->settings = *settings;    
    return TRUE;
}

// Fetches the LIVE status of the device
BOOL paulaNetGetStatus(APTR device, struct LiveStatus* status) {
    if (!paulaNetInternalGetConfig(device)) return FALSE;
    struct PaulaNET* dev = (struct PaulaNET*)device;
    *status = dev->status;
    return TRUE;
}

// Reads raw data from the device. Packets are valid until the next call
BOOL paulaNetReceivePackets(APTR device, APTR userContext, PacketReceivedCallback callback) {
    struct PaulaNET* dev = (struct PaulaNET*)device;
    if (!dev) return FALSE;
    if (!callback) return FALSE;

    // header
    struct DataHeader header;

    // Try to query how much we need to read
    APTR data = receiveFromPaulaNet(dev, TRACK_ETHERNET, 0, dmEthernet, &header, TRUE); 
    if (!data) return FALSE;

    // ABORT and RESET
    dev->device->iotd_Req.io_Flags   = 0;
    dev->device->iotd_Count          = 0xFFFFFFFF;
    dev->device->iotd_Req.io_Command = TD_SEEK;
    dev->device->iotd_Req.io_Offset  = TRACK_SEEK_OFFSET(TRACK_ETHERNET,TRACK_WRITE);
    DoIO((struct IORequest *)dev->device);

    // No data waiting
    if (header.compressedSize == 0) {
        dev->ackLastPacketData = FALSE;
        return TRUE;
    }

    // Now collect the required number of bytes - *should* be quicker
    data = receiveFromPaulaNet(dev, TRACK_ETHERNET, header.compressedSize, dmEthernet, &header, FALSE);    

    if (!data) {
        D(("No Packet Data"));
        return FALSE;    
    }

    dev->ackLastPacketData = TRUE;

#ifdef DEBUGGING
    FPrintf(Output(), "Compressed Size: %ld\n", (LONG)header.compressedSize);
    FPrintf(Output(), "Data Size: %ld\n", (LONG)header.dataSize);
    FPrintf(Output(), "Data Mode: %ld\n", (LONG)header.dataMode);
    FPrintf(Output(), "Compressed: %ld\n", (LONG)header.compressed);
#endif    
    UWORD* pkt = (UWORD*)data;

    // Decode header
    struct PacketsHeader pkHeader;
    mfmDecode(pkt, sizeof(struct PacketsHeader)*2, (UBYTE*)&pkHeader, sizeof(struct PacketsHeader));
    pkt += sizeof(struct PacketsHeader);
    //UWORD* pktEnd = (UWORD*)(((UBYTE*)pkt) + ((header.dataSize - 4)<<1));
    UWORD* pktEnd = (UWORD*)(((UBYTE*)pkt) + header.dataSize - (sizeof(struct PacketsHeader)<<1));

    // Skip if we've already seen this data
    if (dev->lastPicoCounter == header.picoCounter) {
        D(("Already seen."));
        return TRUE;
    }

    // Keep a copy of the last packet counter so we can tell the Pico we received it
    dev->lastPicoCounter = header.picoCounter;
    
    struct CompressionHeader cHeader;    
    for (int i=0; i<pkHeader.numPackets; i++) {
        if (pkt > pktEnd) {
#ifdef DEBUGGING
            FPrintf(Output(), "Packet header past end of buffer\n");
#endif            
            return TRUE;
        }
        // Decode header
        mfmDecode(pkt, sizeof(struct CompressionHeader)*2, (UBYTE*)&cHeader,sizeof(struct CompressionHeader));                
        pkt += sizeof(struct CompressionHeader);        
        if (pkt > pktEnd) {
#ifdef DEBUGGING
            FPrintf(Output(), "Packet data past end of buffer\n");
#endif
            return TRUE;
        }
        
        int dataLength = cHeader.compressedSize & 0x7FFF;
        int realDataLength = dataLength;
        if (realDataLength&1) realDataLength++; // has to be even
        if (((UBYTE*)pkt) + realDataLength > (UBYTE*)pktEnd) {
            return TRUE;
        }

        // Decode buffer        
        if (cHeader.compressedSize & 0x8000) {
            // MFM encoded
            mfmDecode(pkt, dataLength, (UBYTE*)dev->compressionSpace, CHIPBUFFER_SIZE);            
            callback(userContext, (UBYTE*)dev->compressionSpace, cHeader.originalSize);
        } else {
            // Compressed
            //int res = lz4Decompress(dev, (UBYTE*)pkt, dataLength, (UBYTE*)dev->compressionSpace, CHIPBUFFER_SIZE);
            //if (res == cHeader.originalSize) {
            if (rleDecompress(dev, (UBYTE*)pkt, dataLength, (UBYTE*)dev->compressionSpace, CHIPBUFFER_SIZE) == cHeader.originalSize) {
                callback(userContext, (UBYTE*)dev->compressionSpace, cHeader.originalSize);
            } else {
            }
        }

        pkt += realDataLength>>1;
    }

    return TRUE;
}


// Resets and prepares PaulaNET to receive some ethernet packets 
// returns FALSE if the previous buffer needs to be re-sent.
BOOL paulaNetBeginTransmitPackets(APTR device) {
    struct PaulaNET* dev = (struct PaulaNET*)device;
    if (!dev) return FALSE;    
    if (dev->writePosition) return FALSE;
    dev->lastAmigaCounter++;
    dev->compressionMode = (dev->settings.flags & FLAGS_AMIGA_COMPRESSION)!=0;
    dev->writePosition = (UWORD*)(((UBYTE*)&dev->packetBackup[2]) + sizeof(struct DataHeader) + (sizeof(struct PacketsHeader)<<1));
    dev->packetCounter = 0; // temp storage for packet counter
    dev->spaceRemaining = dev->settings.maxTxSize - (sizeof(struct CompressionHeader)<<1) - (((UBYTE*)dev->writePosition) - ((UBYTE*)dev->packetBackup));
    if (dev->compressionMode) dev->spaceRemaining-=COMPRESSION_WORSTCASE; else dev->spaceRemaining>>=1;
    return TRUE;
}

// Add a packet to be sent to PaulaNET and returns how much space is left. packetLength cannot be larger than ETH_MTU
// Returns if theres space for another. If false or you have no more you should call paulaNetTransmitPackets
BOOL paulaNetAddPacket(APTR device, void* data, UWORD packetLength) {
    struct PaulaNET* dev = (struct PaulaNET*)device;
    if (!dev) return FALSE;
    if (packetLength>ETH_MTU) packetLength=ETH_MTU;
    if (packetLength > dev->spaceRemaining) return FALSE;
    if (dev->packetBackup[0] >= MAX_PACKETS_PER_CALL) return FALSE;

    struct CompressionHeader hdr;
    hdr.originalSize = packetLength;
    if (dev->compressionMode) {
        hdr.compressedSize = rleCompress(dev, (UBYTE*)data, packetLength, (UBYTE*)&dev->writePosition[sizeof(struct CompressionHeader)], (UBYTE*)dev->compressionSpace) | 0x8000;
        UWORD lastBit=0;
        mfmEncode((UBYTE*)&hdr, sizeof(hdr), (UWORD*)dev->writePosition, &lastBit);
        hdr.compressedSize &= 0x7FFF;
    } else {        
        UWORD lastBit=0;
        hdr.compressedSize = packetLength<<1;
        mfmEncode((UBYTE*)&hdr, sizeof(hdr), (UWORD*)dev->writePosition, &lastBit);
        mfmEncode((UBYTE*)data, packetLength, (UWORD*)&dev->writePosition[sizeof(struct CompressionHeader)], &lastBit);
    }
    if (hdr.compressedSize&1) {
        *((UBYTE*)(dev->writePosition)+hdr.compressedSize)=0;
        dev->writePosition++;    
        hdr.compressedSize++;
    }
    dev->writePosition += sizeof(struct CompressionHeader) + (hdr.compressedSize>>1);
    dev->packetCounter++;
    // work out remaining space
    dev->spaceRemaining = dev->settings.maxTxSize - (sizeof(struct CompressionHeader)<<1) - (((UBYTE*)dev->writePosition) - ((UBYTE*)dev->packetBackup));
    if (dev->compressionMode) dev->spaceRemaining-=COMPRESSION_WORSTCASE; else dev->spaceRemaining>>=1;
    return (dev->spaceRemaining>=ETH_MTU) && (dev->packetBackup[0]<MAX_PACKETS_PER_CALL);
}

// Sends the packets you have queued using paulaNetAddPacket - Assumes you checked if there was space if possible
BOOL paulaNetTransmitPackets(APTR device) {
    struct PaulaNET* dev = (struct PaulaNET*)device;
    if (!dev) return FALSE;
    if (!dev->writePosition) return TRUE;  // nothing to send
    UWORD lastBit = 0;

    // Smart if no data is needed ot be transmitted and the pico already know's we're in sync, dont waste time in a send
    if ((!dev->ackLastPacketData) && (!dev->packetCounter)) {
        D(("SKIPPING Transmit, nothing to send"));
        dev->writePosition = NULL;
        return TRUE;
    }

    // Prepare
    struct DataHeader* header = (struct DataHeader*)&dev->packetBackup[2];

    // Write the packet header, this is actually after the main header
    struct PacketsHeader hdr;
    hdr.numPackets = dev->packetCounter;

    hdr.flags = 0; // rfu
    mfmEncode((UBYTE*)&hdr, sizeof(hdr), (UWORD*)&header[1], &lastBit);

     // Step 1: Prepare header
    header->magic = DATA_MAGIC_ID;
    header->dataSize = ((UBYTE*)dev->writePosition) - ((UBYTE*)&header[1]);
    header->compressedSize = header->dataSize;
    header->dataMode = dmEthernet;
    header->compressed = 2;
#ifdef DRIVERMODE    
    header->compressed |= FLAGS_FROM_DRIVER;
#endif     
    header->checksum = calcChecksum((UWORD*)&header[1], header->compressedSize);
    header->amigaCounter = dev->lastAmigaCounter;
    header->picoCounter = dev->lastPicoCounter;
    UBYTE* dataEnd = ((UBYTE*)&header[1]) + header->compressedSize;
    if ((dataEnd - ((UBYTE*)dev->packetBackup)) & 1)  *dataEnd++ = 0xAA;        

    // Trail
    *dataEnd++ = 0xAA;
    *dataEnd++ = 0xAA;

    // Word align

    // Step 2: Transmit
    dev->device->iotd_Req.io_Command = ETD_RAWWRITE;
    dev->device->iotd_Req.io_Flags   = 0;
    dev->device->iotd_Count          = 0xFFFFFFFF;
    dev->device->iotd_Req.io_Data    = (APTR)dev->packetBackup;
    dev->device->iotd_Req.io_Length  = dataEnd - ((UBYTE*)dev->packetBackup);
    dev->device->iotd_Req.io_Offset  = TRACK_OFFSET(TRACK_ETHERNET, TRACK_WRITE);

    if (DoIO((struct IORequest *)dev->device)) {
        // If this fails its usually the write protect bit hasnpt propergated. 
        dev->device->iotd_Req.io_Command = ETD_RAWWRITE;
        dev->device->iotd_Req.io_Flags   = 0;
        dev->device->iotd_Count          = 0xFFFFFFFF;
        dev->device->iotd_Req.io_Data    = (APTR)dev->packetBackup;
        dev->device->iotd_Req.io_Length  = dataEnd - ((UBYTE*)dev->packetBackup);
        dev->device->iotd_Req.io_Offset  = TRACK_OFFSET(TRACK_ETHERNET, TRACK_WRITE);
        return DoIO((struct IORequest *)dev->device) == 0;
    } else return TRUE;
}

