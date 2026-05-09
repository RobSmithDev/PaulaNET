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

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#define MAX_RUN     127                 // Maximum RLE length
#define MAX_RAW     127                 // Maximum RAW length


// Returns a guess as to the size of buffer needed to hold compressed data in a worse case situation
int rleCompressionBufferRequired(uint16_t srclen) {
    return srclen + (srclen / 127) + 1;
}

// Reverse pass through the buffer to calculate run lengths at each position.
// On exit, runs[i] contains the number of consecutive identical bytes starting at position i, capped at MAX_RUN.
static void buildRunTable(const uint8_t* src, const uint16_t len, uint8_t* runs) {
    const uint8_t* s = src + len - 1;
    uint8_t* r = runs + len - 1;

    *r = 1;
    while (s > src) {
        --s; --r;
        if (s[0] == s[1]) r[0] = (r[1] < MAX_RUN) ? r[1] + 1 : MAX_RUN;
        else r[0] = 1;
    }
}

// Compress data using our RLE schema. for each byte:
//      if MSB is set, then the remaining 7-bits state how many of the following bytes should be copied from the stream
//      if MSB is clear, then the remaining 7-bits state how many times to repeat the next byte
size_t rleCompress(const uint8_t* src, uint16_t srcLen, uint8_t* dst, bool wordSwap, uint8_t* runBuffer) {
    buildRunTable(src, srcLen, runBuffer);

    uint16_t srcPos = 0;
    uint16_t dstPos = 0;

    while (srcPos < srcLen) {
        // See how many repeats there are at this position
        uint8_t run = runBuffer[srcPos];
        // >=3 is the threshold when RLE becomes more efficient than storing the raw data
        if (run >= 3) {
            dst[dstPos++] = run;
            dst[dstPos++] = src[srcPos];
            srcPos += run;
        }
        else {
            uint16_t rawStart = srcPos;
            uint8_t  rawLen = 0;
            uint8_t* rawHdr = &dst[dstPos++];

            // Work out how much raw data to store
            while (srcPos < srcLen && rawLen < MAX_RAW) {
                run = runBuffer[srcPos];
                if (run >= 3) break;  // better as RLE
                uint8_t absorb = run;
                if (rawLen + absorb > MAX_RAW) absorb = MAX_RAW - rawLen;
                srcPos += absorb;
                rawLen += absorb;
            }
            if (rawLen == 0) dstPos--; else {
                *rawHdr = 0x80 | rawLen;
                memcpy(&dst[dstPos], &src[rawStart], rawLen);
                dstPos += rawLen;
            }
        }
    }

    // Must be even
    if (dstPos&1) dst[dstPos++] = 0;

    if (wordSwap) {
        uint16_t* dst1 = (uint16_t*)dst;
        for (int i=0; i<dstPos; i+=2) *dst1++ = (*dst1>>8) | (*dst1 << 8);
    }
    return dstPos;
}

// Data decompressor based on the above schema. 
uint16_t rleDecompress(const uint8_t* src, uint16_t srclen, uint8_t* dst, uint16_t dstmax) {
    uint16_t srcPos = 0;
    uint16_t dstPos = 0;
    while (srcPos < srclen) {
        const uint8_t  hdr = src[srcPos++];
        const uint8_t  count = hdr & 0x7F;
        // Shouldn't happen, but good for preventing damaged data causing an issue
        if (dstPos + count > dstmax) break;
        if (hdr & 0x80) {
            // Copy raw bytes from stream
            if (srcPos + count > srclen) break;
            memcpy(&dst[dstPos], &src[srcPos], count);
            srcPos += count;
        }
        else {
            // Repeat byte a specific number of times
            if (srcPos >= srclen) break;
            memset(&dst[dstPos], src[srcPos++], count);
        }
        dstPos += count;
    }
    
    return dstPos;
}

// Works out the space needed to decompress a buffer
uint16_t rleCalcDecompressSize(const uint8_t* src, uint16_t srclen) {
    uint16_t srcPos = 0;
    uint16_t dstSize = 0;
    while (srcPos < srclen) {
        uint8_t hdr = src[srcPos++];
        uint8_t count = hdr & 0x7F;
        if (hdr & 0x80) srcPos += count; else srcPos++;         
        dstSize += count;
    }
    return dstSize;
}