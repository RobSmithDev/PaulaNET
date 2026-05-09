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
#ifndef SCAN_FOR_ACCESS_POINTS
#define SCAN_FOR_ACCESS_POINTS

#include "paulanetstruct.h"

// Runs a Scanning Busy window. Returns FALSE if cancelled or failed
BOOL scanForAccessPoints(int parentCenterX, int parentCenterY, struct ScanResults* results, APTR device);


#endif