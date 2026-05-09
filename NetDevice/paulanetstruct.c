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
#include "paulanetstruct.h"

#include <exec/types.h>

// A conversion between the RSSI value and a suitable number of BARS. Returns 0 to 5
int rssiToBars(WORD rssi) {
    if (rssi >= -50) return 5;
    if (rssi >= -60) return 4;
    if (rssi >= -70) return 3;
    if (rssi >= -80) return 2;
    if (rssi >= -90) return 1;
    return 0;
}

