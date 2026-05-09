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
#ifndef SCAN_RESULTS
#define SCAN_RESULTS

#include "paulanetstruct.h"


// Draws RSSI bars!
void drawRSSIBars(struct RastPort *rp, UWORD x, UWORD y, WORD rssi, ULONG penFilled, ULONG penEmpty, BOOL flip);

// Same as above but uses an index rather than an rssi value
void drawRSSIBarsDirect(struct RastPort *rp, UWORD x, UWORD y, WORD bars, ULONG penFilled, ULONG penEmpty, BOOL flip);

// Show results
int selectAccessPoint(int parentCenterX, int parentCenterY, struct ScanResults* results);

#endif