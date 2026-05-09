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

#include <proto/exec.h>
#define NO_INLINE_STDARG
#include <proto/intuition.h>
#undef NO_INLINE_STDARG
#include <proto/gadtools.h>
#include <proto/graphics.h>
#include <proto/dos.h>
#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <libraries/gadtools.h>
#include "maindlg.h"
#include <stdio.h>
#include <string.h>
#include "paulanetio.h"

#include "paulanetstruct.h"
#include "scanforaccesspoints.h"
#include "scanresults.h"

// Gadget Indexes
#define GID_STATUS           0
#define GID_SSID             1
#define GID_PASSWORD         2
#define GID_SCAN             3
#define GID_HOSTNAME         4
#define GID_MAXRX            5
#define GID_MAXTX            6
#define GID_TX_COMPRESS      7
#define GID_RX_COMPRESS      8
#define GID_SAVE             9
#define GID_CANCEL           10
#define NUM_GADGETS          11

#define WIN_WIDTH   440
#define WIN_HEIGHT  180

#define LBL_X       10
#define GAD_X       160
#define GAD_W       260
#define ROW_H       18
#define ROW_START   2      
#define ROW_STEP    20     



extern struct ExecBase *SysBase;
extern struct IntuitionBase *IntuitionBase;
extern struct DosLibrary *DOSBase;
extern struct UtilityBase *UtilityBase;
extern struct GfxBase *GfxBase;
extern struct Library *GadToolsBase;

static struct Window   *window       = NULL;
static struct Gadget   *gadgetList   = NULL;
static struct Gadget   *gadgets[NUM_GADGETS];
static APTR             visualInfo   = NULL;
static struct Screen   *screen       = NULL;

static char  statusText[128]    = "Refreshing";
static char  titleText[128]    = "";

// Enable and disable all controls 
static void setMainWindowGadgetsEnabled(BOOL enabled) {
    for (int i = 0; i < NUM_GADGETS; i++) 
        if (gadgets[i]) GT_SetGadgetAttrs(gadgets[i], window, NULL, GA_Disabled, (ULONG)(!enabled), TAG_END);
}

// Handle Access Point scanning
static BOOL handleScanAPs(APTR device, struct ScanResult* selected) {
    if (!selected) return FALSE;
    BOOL ret = FALSE;
    struct Screen *scr = LockPubScreen(NULL);
	setMainWindowGadgetsEnabled(FALSE);   
    selected->channel=0;
    selected->rssi=0;
    selected->ssid[0]=0;

    if (!paulaNetBeginNetworkScan(device)) {
        struct EasyStruct es = { sizeof(struct EasyStruct), 0, "PaulaNET","Error starting network scan","Cancel"};
		EasyRequest(NULL, &es, NULL);		
        return FALSE;
    }

    struct ScanResults* results = (struct ScanResults*)AllocMem(sizeof(struct ScanResults), MEMF_CLEAR);
    if (results) {
        BOOL scanned = scanForAccessPoints(window->LeftEdge + (window->Width >> 1), window->TopEdge + (window->Height >> 1), results, device);
        if (scanned) {
            if (results->count<1) {
                struct EasyStruct es = { sizeof(struct EasyStruct), 0, "PaulaNET","No access points found","OK"};
		        EasyRequest(NULL, &es, NULL);
            } else {
                int index = selectAccessPoint(window->LeftEdge + (window->Width >> 1), window->TopEdge + (window->Height >> 1), results);
                if (index>=0) {
                    // Save selection
                    *selected = results->ScanResults[index];
                    ret = TRUE;
                }
            }
        }
        FreeMem(results, sizeof(struct ScanResults));
    }
	setMainWindowGadgetsEnabled(TRUE);   
    if (scr) UnlockPubScreen(NULL, scr);
    return ret;
}

// Push the settings to the dialog
void populateSettings(const struct PaulaNETSettings *s) {
    if (!window || !s) return;    
    GT_SetGadgetAttrs(gadgets[GID_SSID], window, NULL, GTST_String, (ULONG)s->ssid, TAG_END);
    GT_SetGadgetAttrs(gadgets[GID_PASSWORD], window, NULL, GTST_String, (ULONG)s->pwd, TAG_END);
    GT_SetGadgetAttrs(gadgets[GID_HOSTNAME], window, NULL, GTST_String, (ULONG)s->hostname, TAG_END);
    GT_SetGadgetAttrs(gadgets[GID_MAXRX], window, NULL, GTIN_Number, (ULONG)s->maxRxSize, TAG_END);
    GT_SetGadgetAttrs(gadgets[GID_MAXTX], window, NULL, GTIN_Number, (ULONG)s->maxTxSize, TAG_END);
    GT_SetGadgetAttrs(gadgets[GID_TX_COMPRESS], window, NULL, GTCB_Checked, (ULONG)((s->flags & FLAGS_AMIGA_COMPRESSION) ? TRUE : FALSE), TAG_END);
    GT_SetGadgetAttrs(gadgets[GID_RX_COMPRESS], window, NULL, GTCB_Checked, (ULONG)((s->flags & FLAGS_PICO_COMPRESSION) ? TRUE : FALSE), TAG_END);
}

// Read back the settings from the dialog
void readSettings(struct PaulaNETSettings *s) {
    struct StringInfo *si;
    if (!s) return;
    s->length = sizeof(struct PaulaNETSettings);
    si = (struct StringInfo *)gadgets[GID_SSID]->SpecialInfo;                   strncpy(s->ssid, si->Buffer, 32); s->ssid[32] = '\0';
    si = (struct StringInfo *)gadgets[GID_PASSWORD]->SpecialInfo;               strncpy(s->pwd, si->Buffer, 32); s->pwd[64] = '\0';
    si = (struct StringInfo *)gadgets[GID_HOSTNAME]->SpecialInfo;               strncpy(s->hostname, si->Buffer, 32); s->hostname[32] = '\0';
    struct StringInfo *sri = (struct StringInfo *)gadgets[GID_MAXRX]->SpecialInfo;      s->maxRxSize = (UWORD)sri->LongInt;
    sri = (struct StringInfo *)gadgets[GID_MAXTX]->SpecialInfo;                         s->maxTxSize = (UWORD)sri->LongInt;
    s->flags = 0;
    if (gadgets[GID_TX_COMPRESS]->Flags & GFLG_SELECTED) s->flags |= FLAGS_AMIGA_COMPRESSION;
    if (gadgets[GID_RX_COMPRESS]->Flags & GFLG_SELECTED) s->flags |= FLAGS_PICO_COMPRESSION;
}

// Change the status message
void setStatusText(const char *text) {
    if (!text) return;
    strncpy(statusText, text, 127);
    statusText[127] = '\0';
    if (window && gadgets[GID_STATUS]) GT_SetGadgetAttrs(gadgets[GID_STATUS], window, NULL, GTTX_Text, (ULONG)statusText, TAG_END);
}

// Returns TRUE on success
BOOL openSettingsWindow(void) {
    struct NewGadget ng;
    struct Gadget   *gad;
    UWORD            topaz8[] = { 8, 0 };   /* TextAttr for topaz 8 */
    struct TextAttr  textAttr = { (STRPTR)"topaz.font", 8, 0, 0 };

     // Open the default public screen 
    screen = LockPubScreen(NULL);
    if (!screen) return FALSE;

    visualInfo = GetVisualInfo(screen, TAG_END);
    if (!visualInfo) { 
        UnlockPubScreen(NULL, screen); 
        return FALSE; 
    }

    /* --- Build gadget list --- */
    gad = CreateContext(&gadgetList);
    if (!gad) goto fail;

    ng.ng_VisualInfo = visualInfo; 
    ng.ng_Flags      = PLACETEXT_LEFT;
    ng.ng_TextAttr   = &textAttr; 

    int y = ROW_START;   

    ng.ng_LeftEdge   = GAD_X;   ng.ng_TopEdge    = y;
    ng.ng_Width      = GAD_W;   ng.ng_Height     = ROW_H; 
    ng.ng_GadgetText = (UBYTE *)"Status:";  ng.ng_GadgetID   = GID_STATUS;
    gad = CreateGadget(TEXT_KIND, gad, &ng, GTTX_Text,   (ULONG)statusText, GTTX_Border, TRUE, TAG_END);
    gadgets[GID_STATUS] = gad;
    y+=ROW_STEP;

    ng.ng_LeftEdge   = GAD_X;   ng.ng_TopEdge    = y;
    ng.ng_Width      = GAD_W;   ng.ng_Height     = ROW_H; 
    ng.ng_GadgetText = (UBYTE *)"SSID:";  ng.ng_GadgetID   = GID_SSID;
    gad = CreateGadget(STRING_KIND, gad, &ng, GTST_MaxChars, 32, TAG_END);
    gadgets[GID_SSID] = gad;
    y+=ROW_STEP;

    ng.ng_LeftEdge   = GAD_X;   ng.ng_TopEdge    = y;
    ng.ng_Width      = GAD_W;   ng.ng_Height     = ROW_H; 
    ng.ng_GadgetText = (UBYTE *)"Password:";     ng.ng_GadgetID   = GID_PASSWORD;
    gad = CreateGadget(STRING_KIND, gad, &ng, GTST_MaxChars, 64, GTST_String,   (ULONG)"", TAG_END);
    gadgets[GID_PASSWORD] = gad;
    y+=ROW_STEP;

    ng.ng_TopEdge    = y+1;
    ng.ng_Height     = ROW_H-2; 
    ng.ng_LeftEdge   = GAD_X + GAD_W/2+10;
    ng.ng_Width      = GAD_W / 2 - 10;
    ng.ng_GadgetText = (UBYTE *)"Scan...";
    ng.ng_GadgetID   = GID_SCAN;
	ng.ng_Flags      = PLACETEXT_IN;
    gad = CreateGadget(BUTTON_KIND, gad, &ng, TAG_END);
    gadgets[GID_SCAN] = gad;
    y+=ROW_STEP;

    ng.ng_Flags      = PLACETEXT_LEFT;
    ng.ng_LeftEdge   = GAD_X;   ng.ng_TopEdge    = y;
    ng.ng_Width      = GAD_W;   ng.ng_Height     = ROW_H; 
    ng.ng_GadgetText = (UBYTE *)"Host Name:";   ng.ng_GadgetID   = GID_HOSTNAME;
    gad = CreateGadget(STRING_KIND, gad, &ng, GTST_MaxChars, 32, TAG_END);
    gadgets[GID_HOSTNAME] = gad;    
    y+=ROW_STEP;

    ng.ng_LeftEdge   = GAD_X;   ng.ng_TopEdge    = y;
    ng.ng_Width      = GAD_W/3;   ng.ng_Height     = ROW_H; 
    ng.ng_GadgetText = (UBYTE *)"Max RX:"; ng.ng_GadgetID   = GID_MAXRX;
    gad = CreateGadget(INTEGER_KIND, gad, &ng, GTIN_Number, 1518, TAG_END);
    gadgets[GID_MAXRX] = gad;

    ng.ng_LeftEdge   = GAD_X+8+GAD_W/2;   ng.ng_TopEdge    = y;
    ng.ng_Width      =  GAD_W/3;   ng.ng_Height     = ROW_H; 
    ng.ng_GadgetText = (UBYTE *)"TX:";  ng.ng_GadgetID   = GID_MAXTX;
    gad = CreateGadget(INTEGER_KIND, gad, &ng, GTIN_Number, 1518, TAG_END);
    gadgets[GID_MAXTX] = gad;
    y+=ROW_STEP;

    ng.ng_LeftEdge   = GAD_X;   ng.ng_TopEdge    = y;
    ng.ng_Height     = ROW_H; 
    ng.ng_Width      = ROW_H;   
    ng.ng_GadgetText = (UBYTE *)"Compression: RX:";  ng.ng_GadgetID   = GID_RX_COMPRESS;
    ng.ng_Flags      = PLACETEXT_LEFT;
    gad = CreateGadget(CHECKBOX_KIND, gad, &ng, GTCB_Checked, FALSE, TAG_END);
    gadgets[GID_RX_COMPRESS] = gad;


    ng.ng_LeftEdge   = GAD_X + 138;   ng.ng_TopEdge    = y;
    ng.ng_Height     = ROW_H; 
    ng.ng_Width      = ROW_H;   
    ng.ng_GadgetText = (UBYTE *)"TX:";  ng.ng_GadgetID   = GID_TX_COMPRESS;
    ng.ng_Flags      = PLACETEXT_LEFT;
    gad = CreateGadget(CHECKBOX_KIND, gad, &ng, GTCB_Checked, FALSE, TAG_END);
    gadgets[GID_TX_COMPRESS] = gad;

    y+=ROW_STEP;
    
    ng.ng_LeftEdge   = LBL_X;
    ng.ng_TopEdge    = y;
    ng.ng_Width      = (GAD_W + GAD_X - LBL_X - 8) / 2;
    ng.ng_Height     = ROW_H + 2;
    ng.ng_GadgetText = (UBYTE *)"Save and Apply";
    ng.ng_GadgetID   = GID_SAVE;
    ng.ng_Flags      = PLACETEXT_IN;
    gad = CreateGadget(BUTTON_KIND, gad, &ng, TAG_END);
    gadgets[GID_SAVE] = gad;

    ng.ng_LeftEdge   = LBL_X + ng.ng_Width + 8;
    ng.ng_GadgetText = (UBYTE *)"Close";
    ng.ng_GadgetID   = GID_CANCEL;
    gad = CreateGadget(BUTTON_KIND, gad, &ng, TAG_END);
    gadgets[GID_CANCEL] = gad;

    if (!gad) goto fail;

    /* --- Open the window --- */
    window = OpenWindowTags(NULL,
		WA_Left,        (screen->Width - WIN_WIDTH) / 2,
        WA_Top,         (screen->Height - WIN_HEIGHT) / 2,
        WA_Width,       WIN_WIDTH,
        WA_Height,      WIN_HEIGHT,
        WA_Title,       (ULONG)titleText,
        WA_Gadgets,     (ULONG)gadgetList,
        WA_Flags,       WFLG_CLOSEGADGET | WFLG_DRAGBAR | WFLG_DEPTHGADGET | WFLG_ACTIVATE | WFLG_GIMMEZEROZERO,
        WA_IDCMP,       IDCMP_CLOSEWINDOW  | IDCMP_GADGETUP     | IDCMP_GADGETDOWN   | IDCMP_REFRESHWINDOW,
        WA_PubScreen,   (ULONG)screen,
        TAG_END);

    if (!window) goto fail;

    GT_RefreshWindow(window, NULL);
    UnlockPubScreen(NULL, screen);
    screen = NULL;
    return TRUE;

fail:
    if (visualInfo) { FreeVisualInfo(visualInfo); visualInfo = NULL; }
    if (gadgetList) { FreeGadgets(gadgetList);    gadgetList = NULL; }
    if (screen)     { UnlockPubScreen(NULL, screen); screen = NULL; }
    return FALSE;
}

// Close and free
static void closeSettingsWindow(void) {
    if (window)     { CloseWindow(window);          window     = NULL; }
    if (visualInfo) { FreeVisualInfo(visualInfo);   visualInfo = NULL; }
    if (gadgetList) { FreeGadgets(gadgetList);      gadgetList = NULL; }
}

void runSettingsWindow(APTR device) {
    struct IntuiMessage *msg;
    BOOL done = FALSE;


    struct LiveStatus status;
    struct PaulaNETSettings settings;
    paulaNetGetConfig(device, &settings);    
    if (paulaNetGetStatus(device, &status)) 
        sprintf(titleText, "PaulaNET Settings (Firmware V%ld.%ld)", status.versionh, status.versionl);
    else sprintf(titleText, "PaulaNET Settings");

    if (!openSettingsWindow()) return;

    populateSettings(&settings);


    struct MsgPort* timerPort = CreateMsgPort();
    struct timerequest* timerReq = (struct timerequest*)CreateIORequest(timerPort, sizeof(struct timerequest));
    OpenDevice(TIMERNAME, UNIT_VBLANK, (struct IORequest*)timerReq, 0);

    // Send first timer request
    timerReq->tr_node.io_Command = TR_ADDREQUEST;
    timerReq->tr_time.tv_secs = 0;
    timerReq->tr_time.tv_micro = 250000;
    SendIO((struct IORequest*)timerReq);

    // In your loop, wait on both ports
    ULONG windowSig = 1L << window->UserPort->mp_SigBit;
    ULONG timerSig  = 1L << timerPort->mp_SigBit;

    while (!done) {
        ULONG signals = Wait(windowSig | timerSig);

        if (signals & timerSig) {
            GetMsg(timerPort);  // clear the message
            
            if (paulaNetGetStatus(device, &status)) {

               BYTE out[128];
               switch (status.linkStatus) {
                    case LINK_STATUS_DOWN: sprintf(out, "Not connected"); break;
                    case LINK_STATUS_JOIN: 
                    case LINK_STATUS_UP:
                    case LINK_STATUS_NOIP: sprintf(out, "Connected (%ld dBm)", (LONG)status.rssi); break;
                    case LINK_STATUS_FAIL: sprintf(out, "Connection failed"); break;
                    case LINK_STATUS_NONET: sprintf(out, "Network not found"); break;
                    case LINK_STATUS_BADAUTH: sprintf(out, "Authentication failed"); break;
                    default: sprintf(out, "Unknown status (%ld)", (LONG)status.linkStatus); break;
                }
                setStatusText((const char*)out);
            }
            
            // Requeue timer
            timerReq->tr_node.io_Command = TR_ADDREQUEST;
            timerReq->tr_time.tv_secs = 1;
            timerReq->tr_time.tv_micro = 0;
            SendIO((struct IORequest*)timerReq);
        }

        if (signals & windowSig) {
            while ((msg = GT_GetIMsg(window->UserPort)) != NULL) {
                ULONG  iclass  = msg->Class;
                UWORD  icode   = msg->Code;
                struct Gadget *igad = (struct Gadget *)msg->IAddress;

                GT_ReplyIMsg(msg);   /* reply BEFORE accessing gadget data */

                switch (iclass) {

                case IDCMP_CLOSEWINDOW:
                    done   = TRUE;
                    break;

                case IDCMP_REFRESHWINDOW:
                    GT_BeginRefresh(window);
                    GT_EndRefresh(window, TRUE);
                    break;

                case IDCMP_GADGETUP:
                    switch (igad->GadgetID) {

                    case GID_MAXRX:
                    case GID_MAXTX:
                        {
                            struct StringInfo *si = (struct StringInfo *)igad->SpecialInfo;
                            LONG val = si->LongInt;
                            if (val < MIN_TRANSFER_SIZE)  val = MIN_TRANSFER_SIZE;
                            if (val > MAX_TRANSFER_SIZE) val = MAX_TRANSFER_SIZE;
                            GT_SetGadgetAttrs(igad, window, NULL, GTIN_Number, (ULONG)val, TAG_END);
                        }
                        break;

                    case GID_SCAN: {
                            struct ScanResult selected;
                            if (handleScanAPs(device, &selected)) {
                                strncpy(settings.ssid,selected.ssid, 32);
                                GT_SetGadgetAttrs(gadgets[GID_SSID], window, NULL, GTST_String, (ULONG)settings.ssid, TAG_END);
                            }
                        }
                        break;

                    case GID_SAVE:
                        readSettings(&settings);
                        if (paulaNetPutConfig(device, &settings)) {
                            struct EasyStruct es = { sizeof(struct EasyStruct), 0, "PaulaNET","Settings saved to the PaulaNET Device","OK"};
		                    EasyRequest(NULL, &es, NULL);		
                        } else {
                            struct EasyStruct es = { sizeof(struct EasyStruct), 0, "PaulaNET","Failed saving settings to the PaulaNET device","Cancel"};
		                    EasyRequest(NULL, &es, NULL);		
                        }                                                
                        break;

                    case GID_CANCEL:
                        done   = TRUE;
                        break;
                    }
                    break;

                case IDCMP_GADGETDOWN:
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

    closeSettingsWindow();
    return;
}
