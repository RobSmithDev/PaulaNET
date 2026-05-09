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
#ifndef TRACK_BUILDER
#define TRACK_BUILDER


// Generates a track in MFM format suitable for streaming out for a specific track and side.
// The track data is generated from the DiskImage.h and DiskImage.cpp files
int generateMFMTrackData(int track, bool lowerSide, void* data);

// Small simple MFM encoder
int mfmEncode(const uint8_t* src, uint16_t srcLen, uint16_t* dst, uint16_t* lastBit);
// MFM decoder 
int mfmDecode(const uint16_t* src, uint16_t srcLenBytes, uint8_t* dst);

#endif