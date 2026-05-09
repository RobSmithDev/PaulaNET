
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
#include <stdint.h>   /* uint8_t, uint16_t etc */
#include "trackBuilder.h"
#include "DiskImage.h"

#define SECTORS_PER_TRACK       11
#define DATA_SIZE               1024

#define B2W(a,b) (((((uint16_t)(a))<<8) & 0xFF00) | ((uint16_t)(b) & 0x00FF))


// Add clock bits to data (borrowed from my minimig MiSTer FLoppy update)
uint16_t AddClockBits(uint16_t value, uint16_t* previous) {
	// Clear all previously set clock bits
	value &= 0x5555;

	// Compute clock bits (clock bit values are inverted)
	uint16_t lShifted = (value << 1);
	uint16_t rShifted = (value >> 1) | (*previous << 15);
	uint16_t cBitsInv = lShifted | rShifted;

	// Reverse the computed clock bits
	uint16_t cBits = cBitsInv ^ 0xAAAA;

	// Return original value with the clock bits added
	value |= cBits;
	*previous = value;
	
	return value;
}

// This generate a valid MFM output for the data input, it's not how it should be done (bits being spread into bytes, its an odd/even approach, which is faster)
int mfmEncode(const uint8_t* src, uint16_t srcLen, uint16_t* dst, uint16_t* lastBit) {
    uint16_t* out = dst;
    for (uint16_t i = 0; i < srcLen; i++) {
        uint16_t word = ((uint16_t)(src[i] & 0xAA) << 7) | (src[i] & 0x55);

        // Compute clock bits (clock bit values are inverted)
        uint16_t lShifted = (word << 1);
        uint16_t rShifted = (word >> 1) | (*lastBit << 15);
        uint16_t cBitsInv = lShifted | rShifted;

        // Reverse the computed clock bits
        uint16_t cBits = cBitsInv ^ 0xAAAA;
        word |= cBits;
        *out++ = word;
        *lastBit = word;
    }
    return ((uint8_t*)out) - ((uint8_t*)dst);
}

// MFM decoder that matches the encoder on PaulaNET device
int mfmDecode(const uint16_t* src, uint16_t srcLenBytes, uint8_t* dst) {
    for (uint16_t i = 0; i < srcLenBytes; i += 2) {
        uint16_t noclock = __builtin_bswap16(*src & 0x5555);
        *dst++ = (noclock >> 7) | noclock;
        src++;
    }
    return srcLenBytes >> 1;
}

// Borrowed and modified from SendSector in minimig core. Returns number of bytes written to output
static int createSector(uint8_t* sectorData, const uint8_t sector, unsigned char trackIncHead, uint16_t* lastBit, uint16_t* output) {
	uint8_t checksum[4];
	uint16_t i;
	uint8_t x, y;
	uint8_t* p;

    uint16_t* out = output;

    // Preamble
    *out++ = AddClockBits(0, lastBit);  // 0xAAAA
    *out++ = AddClockBits(0, lastBit);  // 0xAAAA

    // MFM Sync
    *out++ = B2W(0x44,0x89);
    *out++ = B2W(0x44,0x89);
    *lastBit = B2W(0x44,0x89); // maintain clock bits properly

	// odd bits of header
	x = 0x55;                   checksum[0] = x;
	y = trackIncHead >> 1;      checksum[1] = y;
	*out++ = AddClockBits(B2W(x, y), lastBit);

	x = sector >> 1;            checksum[2] = x;
	y = (11 - sector) >> 1;     checksum[3] = y;
	*out++ = AddClockBits(B2W(x, y), lastBit);

	// even bits of header
	x = 0x55;                   checksum[0] ^= x;
	y = trackIncHead;           checksum[1] ^= y;
	*out++ = AddClockBits(B2W(x, y), lastBit);

	x = sector;                 checksum[2] ^= x;
	y = 11 - sector;            checksum[3] ^= y;
	*out++ = AddClockBits(B2W(x, y), lastBit);

	// sector label and reserved area (changes nothing to checksum)
	i = 0x10;
	while (i--) *out++=(AddClockBits(0, lastBit));

	// send header checksum
	*out++ = AddClockBits(0, lastBit);      
    *out++ = AddClockBits(0, lastBit);
	*out++ = AddClockBits(B2W(checksum[0], checksum[1]), lastBit);
	*out++ = AddClockBits(B2W(checksum[2], checksum[3]), lastBit);

	// calculate data checksum
	checksum[0] = 0; checksum[1] = 0; checksum[2] = 0; checksum[3] = 0;
	p = sectorData;
	i = DATA_SIZE / 2 / 4;
	while (i--) {
		x = *p++; checksum[0] ^= x ^ (x >> 1);
		x = *p++; checksum[1] ^= x ^ (x >> 1);
		x = *p++; checksum[2] ^= x ^ (x >> 1);
		x = *p++; checksum[3] ^= x ^ (x >> 1);
	}
	// send data checksum
	*out++ = AddClockBits(0, lastBit);
	*out++ = AddClockBits(0, lastBit);
	*out++ = AddClockBits(B2W(checksum[0], checksum[1]), lastBit);
	*out++ = AddClockBits(B2W(checksum[2], checksum[3]), lastBit);
	// odd bits of data field
	i = DATA_SIZE / 4;
	p = sectorData;
	while (i--) {
		x = *p++ >> 1;
		y = *p++ >> 1;
		*out++ = AddClockBits(B2W(x, y), lastBit);
	}
	// even bits of data field
	i = DATA_SIZE / 4;
	p = sectorData;
	while (i--) {
		x = *p++;
		y = *p++;
		*out++ = AddClockBits(B2W(x, y), lastBit);
	}
    return ((uint8_t*)out) - ((uint8_t*)output);
}

// Generates a track in MFM format suitable for streaming out for a specific track and side.
// The track data is generated from the DiskImage.h and DiskImage.cpp files
int generateMFMTrackData(int track, bool lowerSide, void* data) {
    static uint8_t sectorData[512];

    const int fullTrack = (track * 2) + (lowerSide ? 0 : 1);
    const int startSector = fullTrack * SECTORS_PER_TRACK;

    uint16_t* outData = (uint16_t*)data;
    uint16_t lastBit = 0;

    // Preamble
    for (int a=0; a<30; a++) *outData++ = AddClockBits(0, &lastBit);

    for (int sector = 0; sector<SECTORS_PER_TRACK; sector++) {
        getSectorData(startSector + sector, sectorData);
        outData+= createSector(sectorData, sector, fullTrack, &lastBit, outData)/2;
    }

    // Add sector gap
    for (int i=0; i<320; i++) *outData++ = AddClockBits(0, &lastBit);  // 0xAAAA

    // Return how many bytes written
    return ((uint8_t*)outData) - ((uint8_t*)data);
}