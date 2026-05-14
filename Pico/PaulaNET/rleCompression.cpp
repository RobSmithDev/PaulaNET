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

/*

// Experimental LZ4 compression Pico->Amiga direction only

// LZ4 constants
#define LZ4_HASH_BITS       10
#define LZ4_HASH_SIZE       (1 << LZ4_HASH_BITS)   // 4096 entries
#define LZ4_HASH_MASK       (LZ4_HASH_SIZE - 1)
#define LZ4_MIN_MATCH       4
#define LZ4_MAX_OFFSET      65535
#define LZ4_LAST_LITERALS   5                       // LZ4 spec: last 5 bytes must be literals

// Hash 4 bytes to a table index
static inline uint16_t lz4Hash(uint32_t v) {
    return (uint16_t)(((v * 2654435761UL) >> 20) & LZ4_HASH_MASK);
}

// Read 4 bytes as uint32 (unaligned safe)
static inline uint32_t read32(const uint8_t* p) {
    uint32_t v;
    memcpy(&v, p, 4);
    return v;
}


// Emit a literal+match sequence into dst, returns new dstPos
// literals    - pointer to start of literal bytes in src
// litLen      - number of literal bytes
// matchOffset - backwards offset (1-based), 0 means end-of-stream (no match)
// matchLen    - match length (actual bytes, >= LZ4_MIN_MATCH), 0 if no match
static uint16_t emitSequence(uint8_t* dst, uint16_t dstPos, const uint8_t* literals, uint16_t litLen, uint16_t matchOffset, uint16_t matchLen) {
    // --- Token byte ---
    uint8_t litNibble   = (litLen   >= 15) ? 15 : (uint8_t)litLen;
    uint8_t matchNibble = (matchLen == 0)  ? 0  : ((matchLen - LZ4_MIN_MATCH) >= 15) ? 15 : (uint8_t)(matchLen - LZ4_MIN_MATCH);
    dst[dstPos++] = (litNibble << 4) | matchNibble;

    // --- Extra literal length bytes ---
    if (litLen >= 15) {
        uint16_t extra = litLen - 15;
        while (extra >= 255) { dst[dstPos++] = 255; extra -= 255; }
        dst[dstPos++] = (uint8_t)extra;
    }

    // --- Literal bytes ---
    memcpy(&dst[dstPos], literals, litLen);
    dstPos += litLen;

    // --- Match offset and extra length (omitted on final sequence) ---
    if (matchLen > 0) {
        dst[dstPos++] = (uint8_t)(matchOffset >> 8);
        dst[dstPos++] = (uint8_t)(matchOffset & 0xFF);

        if (matchLen - LZ4_MIN_MATCH >= 15) {
            uint16_t extra = matchLen - LZ4_MIN_MATCH - 15;
            while (extra >= 255) { dst[dstPos++] = 255; extra -= 255; }
            dst[dstPos++] = (uint8_t)extra;
        }
    }

    return dstPos;
}

// Compress src into dst using LZ4 block format.
// hashBuffer must be LZ4_HASH_SIZE * sizeof(uint16_t) bytes (2KB).
// Returns compressed size, or 0 on failure (dst too small).
// wordSwap byte-swaps output words, matching your RLE option.
size_t lz4Compress(const uint8_t* src, uint16_t srcLen, uint8_t* dst, bool wordSwap, uint16_t* hashBuffer) {
    // Clear hash table (positions default to 0 = unset, we use 0xFFFF as invalid)
    memset(hashBuffer, 0xFF, LZ4_HASH_SIZE * sizeof(uint16_t));

    uint16_t srcPos  = 0;           // current input position
    uint16_t litStart = 0;          // start of current literal run
    uint16_t dstPos  = 0;

    // We must leave LZ4_LAST_LITERALS bytes at the end as literals
    const uint16_t matchLimit = (srcLen > LZ4_LAST_LITERALS) ? srcLen - LZ4_LAST_LITERALS : 0;

    while (srcPos < matchLimit) {
        if (srcPos + LZ4_MIN_MATCH > srcLen) break;

        uint32_t    v    = read32(&src[srcPos]);
        uint16_t    h    = lz4Hash(v);
        uint16_t    ref  = hashBuffer[h];
        hashBuffer[h]    = srcPos;

        // Check for a valid match
        bool matched = false;
        if (ref != 0xFFFF && srcPos - ref <= LZ4_MAX_OFFSET && ref < srcPos) {
            if (read32(&src[ref]) == v) {
                // Extend match forward
                uint16_t matchLen = LZ4_MIN_MATCH;
                uint16_t maxMatch = srcLen - srcPos;
                if (maxMatch > LZ4_LAST_LITERALS) maxMatch -= LZ4_LAST_LITERALS;
                while (matchLen < maxMatch && src[srcPos + matchLen] == src[ref + matchLen])
                    matchLen++;

                uint16_t litLen = srcPos - litStart;
                uint16_t offset = srcPos - ref;

                dstPos = emitSequence(dst, dstPos, &src[litStart], litLen, offset, matchLen);

                srcPos   += matchLen;
                litStart  = srcPos;
                matched   = true;
            }
        }

        if (!matched) srcPos++;
    }

    // Emit final literal-only sequence (no match, ML nibble ignored by spec)
    uint16_t finalLitLen = srcLen - litStart;
    dstPos = emitSequence(dst, dstPos, &src[litStart], finalLitLen, 0, 0);

    // Must be even
    bool dec = false;
    if (dstPos & 1) {
        uint8_t i = dst[dstPos];
        dst[dstPos++] = i;
        dec = true;
    }

    if (wordSwap) {
        uint16_t* p = (uint16_t*)dst;
        for (int i = 0; i < dstPos; i += 2) { *p = (*p >> 8) | (*p << 8); p++; }
    }

    if (dec) dstPos--;

    return dstPos;
}

*/