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

#ifndef RLE_COMPRESSION
#define RLE_COMPRESSION

#define RUN_BUFFER_SIZE   2048                // Max size of data that can be compressed and size of run buffer needed or LZ4 compressor


// Returns a guess as to the size of buffer needed to hold compressed data in a worse case situation
int rleCompressionBufferRequired(uint16_t srclen);

// Compress data using our RLE schema. for each byte:
//      if MSB is set, then the remaining 7-bits state how many of the following bytes should be copied from the stream
//      if MSB is clear, then the remaining 7-bits state how many times to repeat the next byte
//      srcLen should not be bigger than the MTU size. Compression output is clipped at 1540 bytes!
size_t rleCompress(const uint8_t* src, uint16_t srcLen, uint8_t* dst, bool wordSwap, uint8_t* runBuffer);

// Data decompressor based on the above schema. 
uint16_t rleDecompress(const uint8_t* src, uint16_t srclen, uint8_t* dst, uint16_t dstmax);

// Works out the space needed to decompress a buffer
uint16_t rleCalcDecompressSize(const uint8_t* src, uint16_t srclen);

// Experimental LZ4 compressor
size_t lz4Compress(const uint8_t* src, uint16_t srcLen, uint8_t* dst, bool wordSwap, uint16_t* hashBuffer);

#endif