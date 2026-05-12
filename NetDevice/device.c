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

#define DEVICE_MAIN

#include <proto/exec.h>
#include <proto/utility.h>
#include <proto/dos.h>
#include <proto/expansion.h>
#include <clib/exec_protos.h>
#include <clib/alib_protos.h>
#include <devices/timer.h>
#include <proto/timer.h>
#include <exec/types.h>
#include <dos/dostags.h>
#include <exec/ports.h>
#include <utility/tagitem.h>
#include <exec/lists.h>
#include <exec/errors.h>
#include <exec/tasks.h>
#include <proto/utility.h>
#include <exec/execbase.h>
#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include "sana2.h"
#include "paulanetstruct.h"
#include "paulanetio.h"


#ifdef HAVE_VERSION_H
#include "version.h"
#endif

#include "newstyle.h"
#include "device.h"
#include "macros.h"


const UWORD dev_supportedcmds[] = { NSCMD_DEVICEQUERY, CMD_READ, CMD_WRITE, /*S2_SANA2HOOK, */S2_GETGLOBALSTATS, S2_BROADCAST, CMD_WRITE, S2_ONEVENT, S2_READORPHAN, S2_ONLINE, S2_OFFLINE, S2_GETSTATIONADDRESS, S2_DEVICEQUERY, S2_GETSPECIALSTATS, 0 };


#include <proto/exec.h>
#include <proto/dos.h>
#include <stdio.h>
#include <stdlib.h>

//#define _DEBUG

__saveds void frame_proc();
char *frame_proc_name = "PaulaNETPacketScheduler";

static UBYTE HW_MAC[] = {0x00,0x00,0x00,0x00,0x00,0x00};

struct ProcInit {
   struct Message msg;
   struct devbase *db;
   BOOL  error;
   UBYTE pad[2];
};

// Free's anything left after init
void freeInit(DEVBASEP) {
#ifdef _DEBUG	
	if (db->db_debugConsole) Close(db->db_debugConsole);
	db->db_debugConsole = 0;
#endif	
	if (DOSBase) CloseLibrary(DOSBase); 
	DOSBase = NULL;
	if (UtilityBase) CloseLibrary(UtilityBase); 
	UtilityBase = NULL;
}

void DevTermIO( DEVBASEP, struct IORequest *ioreq );

// Simple logging to console window
void logMessage(struct devbase* db, const char *message) {
#ifdef _DEBUG			
	if (!db->db_debugConsole) db->db_debugConsole = Open("CON:10/10/620/100/PaulaNET Debug Output (RobSmithDev)/AUTO/CLOSE/WAIT", MODE_NEWFILE);
    if (db->db_debugConsole) {
		FPuts(db->db_debugConsole, message);
		FPuts(db->db_debugConsole, "\n");
	}
#endif
}

// Formatted logging to console window
static const UWORD stuffChar[] = {0x16c0, 0x4e75};
void logMessagef(struct devbase* db, const char *messageFormat, ...) {
#ifdef _DEBUG	
	char buf[100];
    va_list args;
    va_start(args, messageFormat);	 
    RawDoFmt((STRPTR)messageFormat, (APTR)args, (void (*)(void))stuffChar, buf);
	va_end(args);
	logMessage(db, buf);
#endif	
}

// Simple device init that saves all the real errors until later
__saveds struct Device *DevInit( ASMR(d0) DEVBASEP ASMREG(d0), ASMR(a0) BPTR seglist ASMREG(a0), ASMR(a6) struct Library *_SysBase  ASMREG(a6) ) {	
	db->db_SysBase = _SysBase;
	db->db_SegList = seglist;
	db->db_DOSBase = NULL;
	db->db_UtilityBase = NULL;
	db->db_online = 0;
	db->db_decrementCountOnFail = 0;
	db->db_quit = FALSE;
	db->db_debugConsole = 0;
	D(("PaulaNET: Opening\n"));

	DOSBase = OpenLibrary("dos.library", 36);
	if (!DOSBase) {
		D(("PaulaNET: Failed to open dos.library (36)\n"));
		return 0;
	}	

	UtilityBase = OpenLibrary("utility.library", 37);
	if (!UtilityBase) {		
		D(("PaulaNET: Failed to open utility.library (37)\n"));
		freeInit(db);
		return 0;
	}

	D(("PaulaNET: Opened\n"));
	return (struct Device*)db;
}

// Return an error and clean up
LONG returnError(struct devbase* db, struct IOSana2Req *ioreq, LONG errorCode) {
	ioreq->ios2_Req.io_Error = errorCode; 
	ioreq->ios2_Req.io_Unit = (struct Unit *)0;   
	ioreq->ios2_Req.io_Device = (struct Device *)0;	
	if (db->db_decrementCountOnFail) {
		db->db_decrementCountOnFail = 0;
		db->db_Lib.lib_OpenCnt--;
	}
	return errorCode;
}

// Device open!
__saveds LONG DevOpen( ASMR(a1) struct IOSana2Req *ioreq ASMREG(a1), ASMR(d0) ULONG unit ASMREG(d0), ASMR(d1) ULONG flags ASMREG(d1), ASMR(a6) DEVBASEP ASMREG(a6) ) {		
	db->db_decrementCountOnFail = 0;	
	D(("PaulaNET: DevOpen\n"));
			
    // promiscuous mode not supported
	if ((flags & SANA2OPF_PROM) && (unit)) {
		logMessagef(db, "PaulaNET: SANA2OPF_PROM not supported on unit id %ld", unit);
		return returnError(db, ioreq, IOERR_OPENFAIL);
	}	
		
	db->db_Lib.lib_OpenCnt++; /* avoid Expunge, see below for separate "unit" open count */			
	db->db_decrementCountOnFail = 1;
	if (unit==0 && db->db_Lib.lib_OpenCnt==1) {
		APTR device = paulaNetFindAndOpen((struct ExecBase *)db->db_SysBase, (struct DosLibrary *)db->db_DOSBase);
		if (!device) {
			logMessagef(db, "PaulaNET: PaulaNET device not detected"); 
			return returnError(db, ioreq, IOERR_OPENFAIL);
		} 
		
		logMessagef(db, "PaulaNET: PaulaNET device detected and opened"); 
		struct PaulaNETSettings settings;
		paulaNetGetConfig(device, &settings);

		struct LiveStatus status;
		if (!paulaNetGetStatus(device, &status)) {
			paulaNetClose(device);
			logMessagef(db, "DevOpen: PaulaNET failed to fetch status"); 
			return returnError(db, ioreq, IOERR_OPENFAIL);
		}
		// Take a copy of the MAC Address
		memcpy(HW_MAC, status.macAddress, 6);
		logMessagef(db, "DevOpen: MAC Address %02lx:%02lx:%02lx:%02lx:%02lx:%02lx",HW_MAC[0],HW_MAC[1],HW_MAC[2],HW_MAC[3],HW_MAC[4],HW_MAC[5]); 
		db->db_online = (status.linkStatus == LINK_STATUS_NONET) || (status.linkStatus == LINK_STATUS_UP) || (status.linkStatus == LINK_STATUS_NOIP) || (status.linkStatus == LINK_STATUS_JOIN);
		paulaNetClose(device);		
		
		NewList(&db->db_ReadList);			InitSemaphore(&db->db_ReadListSem);
		NewList(&db->db_WriteList);			InitSemaphore(&db->db_WriteListSem);
		NewList(&db->db_EventList);			InitSemaphore(&db->db_EventListSem);
		NewList(&db->db_ReadOrphanList); 	InitSemaphore(&db->db_ReadOrphanListSem);

		InitSemaphore(&db->db_ProcSem);

		struct ProcInit init;
		struct MsgPort *port;

		if (port = CreateMsgPort()) {
			D(("DevOpen: Starting Server"));
			db->db_quit = FALSE;
			if (db->db_Proc = CreateNewProcTags(NP_Entry, frame_proc, NP_Name, frame_proc_name, NP_Priority, 0, TAG_DONE)) {
				init.error = 1;
				init.db = db;
				init.msg.mn_Length = sizeof(init);
				init.msg.mn_ReplyPort = port;

				D(("DevOpen: handover db: %lx",init.db));
				PutMsg(&db->db_Proc->pr_MsgPort, (struct Message*)&init);
				WaitPort(port);

				if (init.error) {
					logMessagef(db,"DevOpen: Process startup error"); 
					return returnError(db, ioreq, IOERR_OPENFAIL);
				}
			} else {
				logMessagef(db,"DevOpen: Couldn't create process"); 
				return returnError(db, ioreq, IOERR_OPENFAIL);
			}
			DeleteMsgPort(port);
		} else {
			logMessagef(db,"DevOpen: Failed to create message port"); 
			return returnError(db, ioreq, IOERR_OPENFAIL);
		}
	}
	
	struct BufferManagement *bm;
	if ((bm = (struct BufferManagement*)AllocVec(sizeof(struct BufferManagement), MEMF_CLEAR|MEMF_PUBLIC))) {
		if (!(bm->bm_CopyToBuffer = (BMFunc)GetTagData(S2_CopyToBuff16, 0, (struct TagItem *)ioreq->ios2_BufferManagement))){
			bm->bm_CopyToBuffer = (BMFunc)GetTagData(S2_CopyToBuff, 0, (struct TagItem *)ioreq->ios2_BufferManagement);
		}

		if (!(bm->bm_CopyFromBuffer = (BMFunc)GetTagData(S2_CopyFromBuff16, 0, (struct TagItem *)ioreq->ios2_BufferManagement))){
			bm->bm_CopyFromBuffer = (BMFunc)GetTagData(S2_CopyFromBuff, 0, (struct TagItem *)ioreq->ios2_BufferManagement);
		}
		
		ioreq->ios2_BufferManagement = (VOID *)bm;
		ioreq->ios2_Req.io_Error = 0;
		ioreq->ios2_Req.io_Unit = (struct Unit *)unit; // not a real pointer, but id integer
		ioreq->ios2_Req.io_Device = (struct Device *)db;
	}else{
		return returnError(db, ioreq, IOERR_OPENFAIL);
	}
		
	ioreq->ios2_Req.io_Message.mn_Node.ln_Type = NT_REPLYMSG;
	db->db_Lib.lib_Flags &= ~LIBF_DELEXP;
	
	logMessage(db, "DevOpen: Ready"); 
	return 0;
}

__saveds BPTR DevClose( ASMR(a1) struct IORequest *ioreq ASMREG(a1), ASMR(a6) DEVBASEP ASMREG(a6) ) {
	BPTR  ret = (0);

	if (!ioreq) return ret;
	db->db_Lib.lib_OpenCnt--;

	if (db->db_Lib.lib_OpenCnt == 0) {
		if (db->db_Proc) {
			Signal((struct Task*)db->db_Proc, SIGBREAKF_CTRL_C);
			db->db_quit = TRUE; // Alternative closure control if signals are not processing 

			// Wait for shutdown
			ObtainSemaphore(&db->db_ProcSem);
			ReleaseSemaphore(&db->db_ProcSem);
			db->db_Proc = 0;
		}   
	}
	
	ioreq->io_Device = (0);
	ioreq->io_Unit   = (struct Unit *)(-1);
	if (db->db_Lib.lib_Flags & LIBF_DELEXP) ret = DevExpunge(db);
	return ret;
}

__saveds BPTR DevExpunge( ASMR(a6) DEVBASEP ASMREG(a6) ) {
	BPTR seglist = db->db_SegList;

	if( db->db_Lib.lib_OpenCnt ) {
		db->db_Lib.lib_Flags |= LIBF_DELEXP;
		return 0;
	}

	Remove((struct Node*)db);

	freeInit(db);
	FreeMem( ((BYTE*)db)-db->db_Lib.lib_NegSize,(ULONG)(db->db_Lib.lib_PosSize + db->db_Lib.lib_NegSize));

	return seglist;
}

__saveds VOID DevBeginIO( ASMR(a1) struct IOSana2Req *ioreq ASMREG(a1), ASMR(a6) DEVBASEP ASMREG(a6) ) {    
	ioreq->ios2_Req.io_Message.mn_Node.ln_Type = NT_MESSAGE;
	ioreq->ios2_Req.io_Error = S2ERR_NO_ERROR;
	if (ioreq->ios2_Req.io_Command != S2_ONEVENT){
		// Wire error used by onevent but otherwise can be set to generic error
		ioreq->ios2_WireError = S2WERR_GENERIC_ERROR;
	}

	switch( ioreq->ios2_Req.io_Command ) {
	case NSCMD_DEVICEQUERY: 
		{
			D(("NSCMD_DEVICEQUERY"));
			struct IOStdReq *ioreq2 = (struct IOStdReq *)ioreq;
			ULONG size = sizeof(struct NSDeviceQueryResult);
			if((ioreq2->io_Data == NULL) || (ioreq2->io_Length < size)) {
				ioreq2->io_Error = IOERR_BADLENGTH;
			}
			else {
				struct NSDeviceQueryResult *info = ioreq2->io_Data;
				ioreq2->io_Error = 0;
				ioreq2->io_Actual = size;
				info->SizeAvailable = size;
				info->DevQueryFormat = 0;
				info->DeviceType = NSDEVTYPE_SANA2;
				info->DeviceSubType = 0;
				info->SupportedCommands = (APTR)dev_supportedcmds;
			}

			if (!(ioreq2->io_Flags & IOF_QUICK)) {
				ioreq2->io_Message.mn_Node.ln_Type = NT_MESSAGE;
				ReplyMsg((struct Message *)ioreq2);
			}
			else {
				/* otherwise just mark it as done */
				ioreq2->io_Message.mn_Node.ln_Type = NT_REPLYMSG;
			}		
			return;
		}
		break;
		
	case CMD_READ:
		if (ioreq->ios2_BufferManagement == NULL) {
			ioreq->ios2_Req.io_Error = S2ERR_BAD_ARGUMENT;
			ioreq->ios2_WireError = S2WERR_BUFF_ERROR;
		} else if (!db->db_online) {
			ioreq->ios2_Req.io_Error = S2ERR_OUTOFSERVICE;
			ioreq->ios2_WireError = S2WERR_UNIT_OFFLINE;
		} else {
			ioreq->ios2_Req.io_Flags &= ~SANA2IOF_QUICK;
			ObtainSemaphore(&db->db_ReadListSem);
			AddTail((struct List*)&db->db_ReadList, (struct Node*)ioreq);
			ReleaseSemaphore(&db->db_ReadListSem);
			ioreq = NULL;
		}
		break;

	case S2_GETGLOBALSTATS:
		D(("GETGLOBALSTATS"));
		memcpy(ioreq->ios2_StatData, &db->db_DevStats, sizeof(struct Sana2DeviceStats));
		break;

	case S2_BROADCAST:   
		if (ioreq->ios2_DstAddr) {
			memset(ioreq->ios2_DstAddr, 0xFF, HW_ADDRFIELDSIZE);
		} else {
			D(("bcast: invalid dst addr\n"));
			ioreq->ios2_Req.io_Error = S2ERR_BAD_ADDRESS;
			ioreq->ios2_WireError = S2WERR_BUFF_ERROR;
		}
		// fall through!	
	case CMD_WRITE: 
		if (ioreq->ios2_BufferManagement == NULL) {
			ioreq->ios2_Req.io_Error = S2ERR_BAD_ARGUMENT;
			ioreq->ios2_WireError = S2WERR_BUFF_ERROR;
		} else if (!db->db_online) {
			ioreq->ios2_Req.io_Error = S2ERR_OUTOFSERVICE;
			ioreq->ios2_WireError = S2WERR_UNIT_OFFLINE;
		} else {	
			ioreq->ios2_Req.io_Flags &= ~SANA2IOF_QUICK;
			ioreq->ios2_Req.io_Error = 0;
			ObtainSemaphore(&db->db_WriteListSem);
			// The sending process reads from the head of the list,
			// so add to the tail here, otherwise packets could go out in swapped order
			AddTail((struct List*)&db->db_WriteList, (struct Node*)ioreq);
			ReleaseSemaphore(&db->db_WriteListSem);
			Signal((struct Task*)db->db_Proc, SIGBREAKF_CTRL_F);
			ioreq = NULL;
		}
		break;
  
    case S2_ONEVENT:
		D(("S2_ONEVENT"));
      if (((ioreq->ios2_WireError & S2EVENT_ONLINE) && (db->db_online)) ||
         ((ioreq->ios2_WireError & S2EVENT_OFFLINE) && (!db->db_online))) {
           ioreq->ios2_Req.io_Error = 0;
           ioreq->ios2_WireError &= (S2EVENT_ONLINE|S2EVENT_OFFLINE);
		   ioreq->ios2_Req.io_Flags |= SANA2IOF_QUICK;
		   D(("S2_ONEVENT: reply quick, wireerror 0x%04X", ioreq->ios2_WireError));
           DevTermIO(db, (struct IORequest*)ioreq);
           ioreq = NULL;
      } else{
		  if ((ioreq->ios2_WireError & (S2EVENT_ONLINE|S2EVENT_OFFLINE|S2EVENT_ERROR|S2EVENT_TX|S2EVENT_RX|S2EVENT_BUFF|S2EVENT_HARDWARE|S2EVENT_SOFTWARE)) != ioreq->ios2_WireError) {
			// we cannot handle such events 
			ioreq->ios2_Req.io_Error = S2ERR_NOT_SUPPORTED;
			ioreq->ios2_WireError = S2WERR_BAD_EVENT;
			D(("S2_ONEVENT: error, not supported event"));
		  }
		  else {
			  D(("S2_ONEVENT: queued"));
			// Queue anything else 
			ioreq->ios2_Req.io_Flags &= ~SANA2IOF_QUICK;
			ObtainSemaphore(&db->db_EventListSem);
			AddTail((struct List*)&db->db_EventList, (struct Node*)ioreq);
			ReleaseSemaphore(&db->db_EventListSem);
			ioreq = NULL;
		  }
	  }
      break;  

	case S2_READORPHAN:
		if (ioreq->ios2_BufferManagement == NULL) {
			ioreq->ios2_Req.io_Error = S2ERR_BAD_ARGUMENT;
			ioreq->ios2_WireError = S2WERR_BUFF_ERROR;
		} else if (!db->db_online) {
			ioreq->ios2_Req.io_Error = S2ERR_OUTOFSERVICE;
			ioreq->ios2_WireError = S2WERR_UNIT_OFFLINE;
		} else {                      
			ioreq->ios2_Req.io_Flags &= ~SANA2IOF_QUICK;
			ObtainSemaphore(&db->db_ReadOrphanListSem);
			AddTail((struct List*)&db->db_ReadOrphanList, (struct Node*)ioreq);
			ReleaseSemaphore(&db->db_ReadOrphanListSem);
			ioreq = NULL;
		}
		break;      

	case S2_ONLINE:
		db->db_online = 1;
		break;

	case S2_OFFLINE:
		db->db_online = 0;
		break;

	case S2_CONFIGINTERFACE:   
		break;

	case S2_GETSTATIONADDRESS:
		memcpy(ioreq->ios2_SrcAddr, HW_MAC, HW_ADDRFIELDSIZE); /* current */
		memcpy(ioreq->ios2_DstAddr, HW_MAC, HW_ADDRFIELDSIZE); /* default */
		break;
		
	case S2_DEVICEQUERY: {
			struct Sana2DeviceQuery *devquery;
			devquery = ioreq->ios2_StatData;
			devquery->DevQueryFormat = 0;        // this is format 0
			devquery->DeviceLevel = 0;           // this spec defines level 0
			if (devquery->SizeAvailable >= 18) devquery->AddrFieldSize = HW_ADDRFIELDSIZE * 8; // in bits
			if (devquery->SizeAvailable >= 22) devquery->MTU           = ETH_MTU-HW_ETH_HDR_SIZE; // max size
			if (devquery->SizeAvailable >= 26) devquery->BPS           = 1000*1000*100;   // unlikely
			if (devquery->SizeAvailable >= 30) devquery->HardwareType  = S2WireType_Ethernet;
			if (devquery->SizeAvailable >= 34) devquery->RawMTU        = ETH_MTU;
			devquery->SizeSupplied = (devquery->SizeAvailable<34?devquery->SizeAvailable:34);
		}
		break;
	case S2_GETSPECIALSTATS:
		{
		  struct Sana2SpecialStatHeader *s2ssh = (struct Sana2SpecialStatHeader *)ioreq->ios2_StatData;
		  s2ssh->RecordCountSupplied = 0;
		}
		break;			
	default:
		{
			ioreq->ios2_Req.io_Error = S2ERR_NOT_SUPPORTED;
			ioreq->ios2_WireError = S2WERR_GENERIC_ERROR;
			break;
		}
	}

	if (ioreq) DevTermIO(db, (struct IORequest*)ioreq);
}

// SANA-2 Event management
void DoEvent(DEVBASEP, long event) {
	struct IOSana2Req *ior, *ior2;
	ObtainSemaphore(&db->db_EventListSem );

	for(ior = (struct IOSana2Req *) db->db_EventList.lh_Head; (ior2 = (struct IOSana2Req *) ior->ios2_Req.io_Message.mn_Node.ln_Succ) != NULL; ior = ior2 ) {
		if (ior->ios2_WireError & event) {
			Remove((struct Node*)ior);
			DevTermIO(db, (struct IORequest *)ior);
		}
	}
	ReleaseSemaphore(&db->db_EventListSem );
}

__saveds LONG DevAbortIO( ASMR(a1) struct IORequest *ioreq ASMREG(a1), ASMR(a6) DEVBASEP ASMREG(a6) ) {
	LONG   ret = 0;
	struct IOSana2Req* ios2 = (struct IOSana2Req*)ioreq;

	D(("scsidayna: AbortIO on %lx\n",(ULONG)ioreq));

	Remove((struct Node*)ioreq);

	ioreq->io_Error = IOERR_ABORTED;
	ios2->ios2_WireError = 0;

	ReplyMsg((struct Message*)ioreq);
	return ret;
}

void DevTermIO( DEVBASEP, struct IORequest *ioreq ) {
	struct IOSana2Req* ios2 = (struct IOSana2Req*)ioreq;

	if (!(ios2->ios2_Req.io_Flags & SANA2IOF_QUICK)) {
		ReplyMsg((struct Message *)ioreq);
	} else {
		ioreq->io_Message.mn_Node.ln_Type = NT_REPLYMSG;
	}
}

void rejectAllPackets(DEVBASEP) {
  struct IOSana2Req *ior;

   ObtainSemaphore(&db->db_WriteListSem);
   for (ior = (struct IOSana2Req *)db->db_WriteList.lh_Head; ior->ios2_Req.io_Message.mn_Node.ln_Succ; ior = (struct IOSana2Req *)ior->ios2_Req.io_Message.mn_Node.ln_Succ) {      
      ior->ios2_Req.io_Error = S2ERR_OUTOFSERVICE;
      ior->ios2_WireError = S2WERR_UNIT_OFFLINE;
      Remove((struct Node*)ior);
      DevTermIO(db, (struct IORequest*)ior);
   }
   ReleaseSemaphore(&db->db_WriteListSem);

   ObtainSemaphore(&db->db_ReadListSem);
   for (ior = (struct IOSana2Req *)db->db_ReadList.lh_Head; ior->ios2_Req.io_Message.mn_Node.ln_Succ; ior = (struct IOSana2Req *)ior->ios2_Req.io_Message.mn_Node.ln_Succ) {
      ior->ios2_Req.io_Error = S2ERR_OUTOFSERVICE;
      ior->ios2_WireError = S2WERR_UNIT_OFFLINE;
      Remove((struct Node*)ior);
      DevTermIO(db, (struct IORequest*)ior);
   }
   ReleaseSemaphore(&db->db_ReadListSem);

   ObtainSemaphore(&db->db_ReadOrphanListSem);
   for (ior = (struct IOSana2Req *)db->db_ReadOrphanList.lh_Head; ior->ios2_Req.io_Message.mn_Node.ln_Succ; ior = (struct IOSana2Req *)ior->ios2_Req.io_Message.mn_Node.ln_Succ) {
      ior->ios2_Req.io_Error = S2ERR_OUTOFSERVICE;
      ior->ios2_WireError = S2WERR_UNIT_OFFLINE;
      Remove((struct Node*)ior);
      DevTermIO(db, (struct IORequest*)ior);
   }
   ReleaseSemaphore(&db->db_ReadOrphanListSem);   
}

// Receive a packet
ULONG receivePacket(DEVBASEP, UBYTE* packet, USHORT packetSize, struct IOSana2Req *req) {	
	ULONG datasize;
	BYTE *frame_ptr;
	BOOL broadcast;
	ULONG res = 0;
	req->ios2_PacketType = ((USHORT)packet[12]<<8)|((USHORT)packet[13]);

	if (req->ios2_Req.io_Flags & SANA2IOF_RAW) {
		frame_ptr = packet;
		datasize = packetSize;
		req->ios2_Req.io_Flags = SANA2IOF_RAW;
	} else {
		frame_ptr = packet+HW_ETH_HDR_SIZE;
		datasize = packetSize-HW_ETH_HDR_SIZE;
		req->ios2_Req.io_Flags = 0;
	}
	req->ios2_DataLength = datasize;
	
	// copy frame to device user (probably tcp/ip system)
	struct BufferManagement *bm = (struct BufferManagement *)req->ios2_BufferManagement;
	if (!(*bm->bm_CopyToBuffer)(req->ios2_Data, frame_ptr, datasize)) {
		req->ios2_Req.io_Error = S2ERR_SOFTWARE;
		req->ios2_WireError = S2WERR_BUFF_ERROR;
		DoEvent(db, S2EVENT_ERROR | S2EVENT_BUFF | S2EVENT_SOFTWARE);
		return 0;
	}
  
	req->ios2_Req.io_Error = req->ios2_WireError = 0;

	memcpy(req->ios2_SrcAddr, packet+6, HW_ADDRFIELDSIZE);
	memcpy(req->ios2_DstAddr, packet, HW_ADDRFIELDSIZE);  

	broadcast = TRUE;
	for (int i=0; i<HW_ADDRFIELDSIZE; i++) {
		if (packet[i] != 0xff) {
			broadcast = FALSE;
			break;
		}
	}
	
	if (broadcast) req->ios2_Req.io_Flags |= SANA2IOF_BCAST;
	return 1;
}

// Handle a received packet
void onPacketReceived(struct devbase* db, UBYTE* packetData, int packetLength) {
	ObtainSemaphore(&db->db_ReadListSem);	

	USHORT packetType = ((USHORT)packetData[12]<<8)|((USHORT)packetData[13]);   

	struct IOSana2Req *ior = NULL;						
	for (ior = (struct IOSana2Req *)db->db_ReadList.lh_Head; ior->ios2_Req.io_Message.mn_Node.ln_Succ; ior = (struct IOSana2Req *)ior->ios2_Req.io_Message.mn_Node.ln_Succ) {
		if (ior->ios2_PacketType == packetType) {
			db->db_DevStats.PacketsReceived++;
			Remove((struct Node*)ior);
			receivePacket(db, packetData, packetLength, ior);
			DevTermIO(db, (struct IORequest *)ior);
			ior = NULL;
			break;
		}
	}
	ReleaseSemaphore(&db->db_ReadListSem);
							
	// Nothing wanted it?
	if (ior) {			
		db->db_DevStats.UnknownTypesReceived++;
		ObtainSemaphore(&db->db_ReadOrphanListSem);
		ior = (struct IOSana2Req *)RemHead((struct List*)&db->db_ReadOrphanList);
		ReleaseSemaphore(&db->db_ReadOrphanListSem);

		if (!ior) {
			// No orphan buffer - signal problem
			DoEvent(db, S2EVENT_BUFF | S2EVENT_RX);  // Signal BEFORE dropping        
			// Very brief delay (1-2 ticks) to let RoadShow post a buffer
			Delay(1);
			// Try ONE more time
			ObtainSemaphore(&db->db_ReadOrphanListSem);
			ior = (struct IOSana2Req *)RemHead((struct List*)&db->db_ReadOrphanList);
			ReleaseSemaphore(&db->db_ReadOrphanListSem);        
			if (!ior) {
				logMessagef(db,"PacketServer: Warn - Orphaned packet not picked up of type %lx", packetType);
				db->db_DevStats.Overruns++;
				DoEvent(db, S2EVENT_ERROR | S2EVENT_BUFF | S2EVENT_SOFTWARE | S2EVENT_RX);
			} else {
				receivePacket(db, packetData, packetLength, ior);
				DevTermIO(db, (struct IORequest *)ior);
			}
		} else {
			receivePacket(db, packetData, packetLength, ior);
			DevTermIO(db, (struct IORequest *)ior);  									
		} 								
	}
}

// This runs as a separate task!
__saveds void frame_proc() {
	struct ProcInit* init; 
	{
		struct { void *db_SysBase; } *db = (void*)0x4;
		struct Process* proc;
		proc = (struct Process*)FindTask(NULL);
		WaitPort(&proc->pr_MsgPort);
		init = (struct ProcInit*)GetMsg(&proc->pr_MsgPort);
	}

	struct devbase* db = init->db;
	// This semaphore must be obtained by this process before it replies its init message, and then
	// hold it for its entire lifetime, otherwise the process exit won't be arbitrated properly.
	ObtainSemaphore(&db->db_ProcSem);
  
	// Temporary packet store	
	UBYTE* packetData = (UBYTE*)AllocVec(ETH_MTU+16, MEMF_PUBLIC);	

	struct MsgPort *timerPort = NULL;
	struct timerequest *timerReq = NULL;
	struct Library *TimerBase = NULL;

	timerPort = CreateMsgPort();
	if (timerPort) timerReq = (struct timerequest *)CreateIORequest(timerPort, sizeof(struct timerequest));
	if (timerReq) 
		if (OpenDevice(TIMERNAME, UNIT_MICROHZ, (struct IORequest *)timerReq, 0) == 0) TimerBase = (struct Library *)timerReq->tr_node.io_Device;
	
	logMessagef(db,"PacketServer: Starting Wifi Device"); 
	struct PaulaNETSettings settings;
	APTR device = paulaNetFindAndOpen((struct ExecBase *)db->db_SysBase, (struct DosLibrary *)db->db_DOSBase);	

	if ((!device) || (!packetData) || (!timerPort) || (!timerReq) || (!TimerBase)) {
		init->error = 1;
		DoEvent(db, S2EVENT_OFFLINE);
		db->db_online = 0;
		
		if (!device) {
			logMessagef(db,"PacketServer: Unknown error occured opening PaulaNET device");
		} else {
			paulaNetClose(device);
			device = NULL;
		}
				
		if (!packetData) {
			logMessage(db,"PacketServer: Out of memory [1]");
			D(("PacketServer: Out of memory [1]\n")); 
		} else FreeVec((void*)packetData);

		if (!timerReq) {
			logMessage(db,"PacketServer: Out of memory [2]");
			D(("PacketServer: Out of memory [2]\n")); 
		} else {
			CloseDevice((struct IORequest *)timerReq);
			DeleteIORequest((struct IORequest *)timerReq);
		}

		if (!timerPort) {
			logMessage(db,"PacketServer: Out of memory [3]");
			D(("PacketServer: Out of memory [3]\n")); 
		} else {
			DeleteMsgPort(timerPort);
		}
		
		ReplyMsg((struct Message*)init);
		Forbid();
		ReleaseSemaphore(&db->db_ProcSem);
		return;
	}

	paulaNetGetConfig(device, &settings);

	// Reply started
	init->error = 0;
	ReplyMsg((struct Message*)init);


	ULONG recv = 0;
		
	// Raise priority slightly
	SetTaskPri((struct Task*)db->db_Proc,1);      
	struct timeval timeLastWifiCheck = {0UL,0UL};
	struct timeval timeWifiCheck = {0UL,0UL};

	int lastFailTxCount = 0;

	while (!(recv & SIGBREAKF_CTRL_C) && !db->db_quit) {
		struct IOSana2Req *nextwrite;

		GetSysTime(&timeWifiCheck);
		// Every 5 seconds check WIFI status and fetch settings
		if (abs(timeWifiCheck.tv_secs-timeLastWifiCheck.tv_secs)>=5) {			
			struct LiveStatus status;
			if (paulaNetGetStatus(device, &status)) {
				paulaNetGetConfig(device, &settings);

				BOOL wasOnline = db->db_online;
				db->db_online = (status.linkStatus == LINK_STATUS_NONET) || (status.linkStatus == LINK_STATUS_UP) || (status.linkStatus == LINK_STATUS_NOIP) || (status.linkStatus == LINK_STATUS_JOIN);
				if (db->db_online) {
					logMessagef(db,"PacketServer: Wifi Connected, Signal Strength: %ld dB\n", status.rssi);
				} else {
					logMessage(db,"PacketServer: Wifi not connected\n");
				}
				if (wasOnline != db->db_online) {
					if (wasOnline) {
						logMessage(db,"PacketServer: Reject Packets\n");
						rejectAllPackets(db);
						DoEvent(db, S2EVENT_OFFLINE);
					} else {
						GetSysTime(&db->db_DevStats.LastStart);
						logMessage(db,"PacketServer: Online\n");
						DoEvent(db, S2EVENT_ONLINE);
					}
				}
			}
			timeLastWifiCheck.tv_secs = timeWifiCheck.tv_secs;
		}
		
		if (db->db_online) {			
			USHORT counter = 0; 
			
			recv = SetSignal(0L, 0L);				
			if (!(recv & SIGBREAKF_CTRL_C)) {
				paulaNetReceivePackets(device, db, (PacketReceivedCallback)onPacketReceived);				
			}
			
			// Transmit
			if (!(recv & SIGBREAKF_CTRL_C)) {

				// This will only work if the previous lot of packets was received properly by the pico
				if (paulaNetBeginTransmitPackets(device)) {
					struct IOSana2Req *nextwrite;

					// Collect packets until not enough data space or too many
					ObtainSemaphore(&db->db_WriteListSem);
					struct IOSana2Req *ior = (struct IOSana2Req *)db->db_WriteList.lh_Head;
					while ((nextwrite = (struct IOSana2Req *)ior->ios2_Req.io_Message.mn_Node.ln_Succ) != NULL) {
						USHORT sz = ior->ios2_DataLength;
						UBYTE* dataOut = packetData;

						// Add packet ethernet header if needed
						if (!(ior->ios2_Req.io_Flags & SANA2IOF_RAW)) {
							*((USHORT*)(dataOut+12)) = (USHORT)ior->ios2_PacketType;
							// Add ethernet header
							memcpy(dataOut, ior->ios2_DstAddr, HW_ADDRFIELDSIZE);
							memcpy(dataOut+6, HW_MAC, HW_ADDRFIELDSIZE);
							sz += HW_ETH_HDR_SIZE;
							dataOut += HW_ETH_HDR_SIZE;
						}

						if (sz < ETH_MTU) {
							// Add the data
							struct BufferManagement *bm = (struct BufferManagement *)ior->ios2_BufferManagement;				   
							if (!(*bm->bm_CopyFromBuffer)(dataOut, ior->ios2_Data, ior->ios2_DataLength)) {
								ior->ios2_Req.io_Error = S2ERR_SOFTWARE;
								ior->ios2_WireError = S2WERR_BUFF_ERROR;
								DoEvent(db, S2EVENT_ERROR | S2EVENT_BUFF | S2EVENT_SOFTWARE);
								D(("bm_CopyFromBuffer FAIL"));		
								Remove((struct Node*)ior);
								DevTermIO(db, (struct IORequest *)ior);
							} else {						
								ior->ios2_Req.io_Error = ior->ios2_WireError = 0;
								db->db_DevStats.PacketsSent++;
								db->db_DevStats.PacketsSent++;
								Remove((struct Node*)ior);
								DevTermIO(db, (struct IORequest *)ior);
								if (!paulaNetAddPacket(device, packetData, sz)) break;
							}
						} else {
							ior->ios2_Req.io_Error  = S2ERR_MTU_EXCEEDED;
	  						ior->ios2_WireError = S2WERR_BUFF_ERROR;
	  						DoEvent(db, S2EVENT_ERROR | S2EVENT_BUFF | S2EVENT_SOFTWARE);
	  						Remove((struct Node*)ior);
							DevTermIO(db, (struct IORequest *)ior);
						}
						ior = nextwrite;
					}
					ReleaseSemaphore(&db->db_WriteListSem);
				}

				// Send any packets found - Must ALWAYS call this to clear the receive buffer on the pico, this will re-send the previous set if it wasn't ACK'd
				if (!paulaNetTransmitPackets(device)) {
					// This doesnt matter that much if it fails as the code will just keep re-sending this forever!
					logMessagef(db, "Failed to send packets. will retry");
				}
			}			
			if (recv & SIGBREAKF_CTRL_C) D(("Terminate Requested"));
		} else {
			Delay(5);
		}
	}
	
	logMessage(db,"PacketServer: Shutting down [1]");
		
	DoEvent(db, S2EVENT_OFFLINE);
	db->db_online = 0;
	paulaNetClose(device);
	rejectAllPackets(db);
	
	FreeVec((void*)packetData);

	logMessage(db,"PacketServer: Shutting down [2]");
	CloseDevice((struct IORequest *)timerReq);
	DeleteIORequest((struct IORequest *)timerReq);
	DeleteMsgPort(timerPort);
				
	logMessage(db,"PacketServer: Closed");
	
	Forbid();
	ReleaseSemaphore(&db->db_ProcSem);
}
