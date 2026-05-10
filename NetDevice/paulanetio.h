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

#ifndef PAULANET_IO
#define PAULANET_IO

#include <exec/types.h>
#include "paulanetstruct.h"

// Searches the system for the PaulaNET device, if found it returns an open handle to it
APTR paulaNetFindAndOpen(struct ExecBase *Sysbase, struct DosLibrary *Dosbase);

// Close the PaulaNET device
void paulaNetClose(APTR device);

// Open the PaulaNET device
APTR paulaNetOpen(struct ExecBase *Sysbase, struct DosLibrary *Dosbase, int deviceIndex);

// Starts a scan for access points
BOOL paulaNetBeginNetworkScan(APTR device);

// Returns TRUE if an access point scan is active
BOOL paulaNetIsAccessPointScanActive(APTR device);

// Populates the results from the scan.
BOOL paulaNetGetAccessPoints(APTR device, struct ScanResults* results);

// Public access to settings
void paulaNetGetConfig(APTR device, struct PaulaNETSettings* settings);

// Fetch the config from the device - BLOCKING
BOOL paulaNetPutConfig(APTR device, struct PaulaNETSettings* settings);

// Fetches the LIVE status of the device, also refreshes settings
BOOL paulaNetGetStatus(APTR device, struct LiveStatus* status);
 
// Reads raw packets from the device. Retrns a bitmask of whats possible with the device - see DEVICESTATUS_ flags
BOOL paulaNetReceivePackets(APTR device, APTR userContext, PacketReceivedCallback callback);

// Resets and prepares PaulaNET to receive some ethernet packets 
// returns FALSE if the previous buffer needs to be re-sent.
BOOL paulaNetBeginTransmitPackets(APTR device);

  // Add a packet to be sent to PaulaNET and returns how much space is left. packetLength cannot be larger than ETH_MTU
// Returns if theres space for another. If false or you have no more you should call paulaNetTransmitPackets
BOOL paulaNetAddPacket(APTR device, void* data, UWORD packetLength);

  // Sends the packets you have queued using paulaNetAddPacket - Assumes you checked if can first
BOOL paulaNetTransmitPackets(APTR device);



#endif