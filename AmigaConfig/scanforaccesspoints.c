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
#include "scanforaccesspoints.h"
#include <proto/exec.h>
#define NO_INLINE_STDARG
#include <proto/intuition.h>
#undef NO_INLINE_STDARG
#include <proto/gadtools.h>
#include <proto/graphics.h>
#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <libraries/gadtools.h>
#include <proto/dos.h>
#include <clib/alib_protos.h>
#include <stdio.h>
#include <string.h>
#include "paulanetio.h"
#include "paulanetstruct.h"
#include "scanresults.h"

extern struct ExecBase *SysBase;
extern struct Library  *GadToolsBase;
extern struct IntuitionBase *IntuitionBase;
extern struct GfxBase       *GfxBase;
extern struct DosLibrary *DOSBase;


#define SCAN_WIN_W  230
#define SCAN_WIN_H   70

// Runs a Scanning Busy window. Returns FALSE if cancelled or failed
BOOL scanForAccessPoints(int parentCenterX, int parentCenterY, struct ScanResults* results, APTR device) {
    if (!results) return FALSE;

    struct Window *scanWin;
    struct Gadget *glist = NULL;    
    struct TextAttr  ta  = { (STRPTR)"topaz.font", 8, FS_NORMAL, FPF_ROMFONT };
    results->count = 0;
   
    BOOL result = TRUE;

    struct Screen *scr = LockPubScreen(NULL);
    if (!scr) return FALSE;

    APTR vi = GetVisualInfo(scr, TAG_END);
    if (!vi) { 
        UnlockPubScreen(NULL, scr); 
        return FALSE; 
    }

    struct Gadget *gad = CreateContext(&glist);

    // Create cancel button
    struct NewGadget ng;
    ng.ng_LeftEdge   = (SCAN_WIN_W - 80) / 2;       ng.ng_TopEdge    = 30;
    ng.ng_Width      = 80;                          ng.ng_Height     = 18;
    ng.ng_GadgetText = (UBYTE*)"Cancel";
    ng.ng_GadgetID   = 1;
    ng.ng_Flags      = PLACETEXT_IN;
    ng.ng_TextAttr   = &ta;
    ng.ng_VisualInfo = vi;
    gad = CreateGadget(BUTTON_KIND, gad, &ng, TAG_END);

    // Open window
    scanWin = OpenWindowTags(NULL,
        WA_Left,        parentCenterX - (SCAN_WIN_W >> 1),
        WA_Top,         parentCenterY - (SCAN_WIN_H >> 1),
        WA_Width,       SCAN_WIN_W,
        WA_Height,      SCAN_WIN_H,
        WA_Title,       (ULONG)"PaulaNET",
        WA_Gadgets,     (ULONG)glist,
        WA_Flags,       WFLG_DRAGBAR | WFLG_ACTIVATE |
                        WFLG_GIMMEZEROZERO | WFLG_RMBTRAP,
        WA_IDCMP,       IDCMP_CLOSEWINDOW | IDCMP_GADGETUP | IDCMP_REFRESHWINDOW,
        WA_PubScreen,   (ULONG)scr,
        TAG_END);

    UnlockPubScreen(NULL, scr);

    if (!scanWin) { 
        FreeVisualInfo(vi); 
        FreeGadgets(glist); 
        return FALSE; 
    }

    GT_RefreshWindow(scanWin, NULL);

    struct RastPort *rp = scanWin->RPort;
    struct TextFont *font = OpenFont(&ta);
    SetFont(rp, font);
    SetAPen(rp, 1);
    Move(rp, 15, 15);   Text(rp, "Scanning, Please Wait...", 24);

    BOOL done = FALSE;

    struct MsgPort* timerPort = CreateMsgPort();
    struct timerequest* timerReq = (struct timerequest*)CreateIORequest(timerPort, sizeof(struct timerequest));
    OpenDevice(TIMERNAME, UNIT_VBLANK, (struct IORequest*)timerReq, 0);
     // Send first timer request
    timerReq->tr_node.io_Command = TR_ADDREQUEST;
    timerReq->tr_time.tv_secs = 0;
    timerReq->tr_time.tv_micro = 100000;
    SendIO((struct IORequest*)timerReq);

    struct IntuiMessage *msg;

    // In your loop, wait on both ports
    ULONG windowSig = 1L << scanWin->UserPort->mp_SigBit;
    ULONG timerSig  = 1L << timerPort->mp_SigBit;
    int counter = 0;

    while (!done) {
        ULONG signals = Wait(windowSig | timerSig);

        if (signals & timerSig) {
            GetMsg(timerPort);  // clear the message            
            if (!paulaNetIsAccessPointScanActive(device)) {
                // Fetch access points                
                done = TRUE;
                result = paulaNetGetAccessPoints(device, results);
            }
            
            // Requeue timer
            timerReq->tr_node.io_Command = TR_ADDREQUEST;
            timerReq->tr_time.tv_secs = 0;
            timerReq->tr_time.tv_micro = 100000;
            SendIO((struct IORequest*)timerReq);

            drawRSSIBarsDirect(rp,  30, SCAN_WIN_H - 35 , counter, 2, 1, FALSE); 
            drawRSSIBarsDirect(rp,  SCAN_WIN_W-50, SCAN_WIN_H - 35 , counter, 2, 1, TRUE);         
            counter++;
            if (counter>=6) counter=0;
        }

        if (signals & windowSig && !done) {
            while ((msg = GT_GetIMsg(scanWin->UserPort)) != NULL) {
                ULONG cls  = msg->Class;
                struct Gadget *g = (struct Gadget *)msg->IAddress;
                GT_ReplyIMsg(msg);

                switch (cls) {
                    case IDCMP_CLOSEWINDOW:
                        done = TRUE; 
                        result = FALSE;
                        break;
                    case IDCMP_GADGETUP:
                        if (g->GadgetID == 1) {
                            done = TRUE; 
                            result = FALSE; 
                        }
                        break;
                    case IDCMP_REFRESHWINDOW:
                        GT_BeginRefresh(scanWin);
                        GT_EndRefresh(scanWin, TRUE);
                        Move(rp, 15, 15);   Text(rp, "Scanning, Please Wait...", 24);
                        drawRSSIBarsDirect(rp,  30, SCAN_WIN_H - 35 , counter, 2, 1, FALSE); 
                        drawRSSIBarsDirect(rp,  SCAN_WIN_W-50, SCAN_WIN_H - 35 , counter, 2, 1, TRUE);         
                        break;
                }
            }
        }       
    }

    AbortIO((struct IORequest*)timerReq);
    WaitIO((struct IORequest*)timerReq);
    CloseDevice((struct IORequest*)timerReq);
    DeleteIORequest((struct IORequest*)timerReq);
    DeleteMsgPort(timerPort);

    CloseFont(font);
    CloseWindow(scanWin);
    FreeVisualInfo(vi);
    FreeGadgets(glist);
    return result;
}


