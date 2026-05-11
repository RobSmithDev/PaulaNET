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

#ifndef _INC_DEVICE_H
#define _INC_DEVICE_H

/* includes */
#include "compiler.h"
#include <dos/dos.h>
#include <exec/lists.h>
#include <exec/libraries.h>
#include <exec/devices.h>
#include <exec/semaphores.h>
#include "debug.h"
#include "sana2.h"
#include "paulanetstruct.h"

/* reassign Library bases from global definitions to own struct */
#define SysBase       db->db_SysBase
#define DOSBase       db->db_DOSBase
#define UtilityBase   db->db_UtilityBase

struct devbase {
	struct Library db_Lib;
	BPTR db_SegList;            /* from Device Init */

	struct Library *db_SysBase; /* Exec Base */
	struct Library *db_DOSBase;
	struct Library *db_UtilityBase;
	struct Sana2DeviceStats db_DevStats;

	BPTR db_debugConsole; 
	
	BOOL db_decrementCountOnFail;
	BOOL db_quit;
		
	volatile USHORT db_online;
	
	struct List db_ReadList;
	struct SignalSemaphore db_ReadListSem;
	struct List db_WriteList;
	struct SignalSemaphore db_WriteListSem;
	struct List db_EventList;
	struct SignalSemaphore db_EventListSem;   
	struct List db_ReadOrphanList;
	struct SignalSemaphore db_ReadOrphanListSem;
	struct Process* db_Proc;
	struct SignalSemaphore db_ProcSem;
};

#ifndef DEVBASETYPE
#define DEVBASETYPE struct devbase
#endif
#ifndef DEVBASEP
#define DEVBASEP DEVBASETYPE *db
#endif

/* PROTOS */

ASM LONG LibNull( void );

ASM SAVEDS struct Device *DevInit(ASMR(d0) DEVBASEP                  ASMREG(d0), 
                                  ASMR(a0) BPTR seglist              ASMREG(a0), 
				  ASMR(a6) struct Library *_SysBase  ASMREG(a6) );

ASM SAVEDS LONG DevOpen( ASMR(a1) struct IOSana2Req *ios2            ASMREG(a1), 
                         ASMR(d0) ULONG unit                         ASMREG(d0), 
                         ASMR(d1) ULONG flags                        ASMREG(d1),
                         ASMR(a6) DEVBASEP                           ASMREG(a6) );

ASM SAVEDS BPTR DevClose(   ASMR(a1) struct IORequest *ios2         ASMREG(a1),
                            ASMR(a6) DEVBASEP                        ASMREG(a6) );

ASM SAVEDS BPTR DevExpunge( ASMR(a6) DEVBASEP                        ASMREG(a6) );

ASM SAVEDS VOID DevBeginIO( ASMR(a1) struct IOSana2Req *ios2         ASMREG(a1),
                            ASMR(a6) DEVBASEP                        ASMREG(a6) );

ASM SAVEDS LONG DevAbortIO( ASMR(a1) struct IORequest *ios2         ASMREG(a1),
                            ASMR(a6) DEVBASEP                        ASMREG(a6) );

void DevTermIO( DEVBASETYPE*, struct IORequest * );

/* private functions */
#ifdef DEVICE_MAIN

#endif /* DEVICE_MAIN */

#define HW_ADDRFIELDSIZE          6
#define HW_ETH_HDR_SIZE          14       /* ethernet header: dst, src, type */

typedef BOOL (*BMFunc)(__reg("a0") void* a, __reg("a1") void* b, __reg("d0") long c);

typedef struct BufferManagement
{
  struct MinNode   bm_Node;
  BMFunc           bm_CopyFromBuffer;
  BMFunc           bm_CopyToBuffer;
} BufferManagement;

#endif /* _INC_DEVICE_H */
