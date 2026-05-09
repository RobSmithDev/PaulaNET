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
#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <libraries/gadtools.h>
#include <clib/alib_protos.h>
#include <stdio.h>
#include <string.h> 

#include "scanresults.h"
#include "scanforaccesspoints.h"
#include "paulanetstruct.h"

extern struct ExecBase *SysBase;
extern struct Library  *GadToolsBase;
extern struct IntuitionBase *IntuitionBase;
extern struct GfxBase       *GfxBase;

#define ICON_BAR_W   4
#define ICON_BAR_GAP 2
#define ICON_HEIGHT  11

#define RES_WIN_W   450
#define RES_WIN_H   150
#define RES_LV_H    80   

#define RES_GID_LIST    1
#define RES_GID_SELECT  2
#define RES_GID_CANCEL  3

// To hold the results
#define LV_LINE_LEN 64
static struct List      listViewList;
static char             listViewLines[MAX_SCAN_RESULTS][LV_LINE_LEN];
static struct Node      listViewNodes[MAX_SCAN_RESULTS];

// Draws RSSI bars!
void drawRSSIBarsDirect(struct RastPort *rp, UWORD x, UWORD y, WORD bars, ULONG penFilled, ULONG penEmpty, BOOL flip) {    
    for (UWORD i = 0; i < 5; i++) {
        unsigned short offset = i * (ICON_BAR_W + ICON_BAR_GAP);
        unsigned short bx     = flip ? (x + (ICON_BAR_W + ICON_BAR_GAP * 5) - offset) : (x + offset);
        unsigned short bh     = 3 + (i << 1);
        unsigned short by     = y + (ICON_HEIGHT - bh);
        unsigned long pen     = (i < bars) ? penFilled : penEmpty;
        SetAPen(rp, pen);
        RectFill(rp, bx, by, bx + ICON_BAR_W - 1, y + ICON_HEIGHT - 1);
    }
}

// Draws RSSI bars!
void drawRSSIBars(struct RastPort *rp, UWORD x, UWORD y, WORD rssi, ULONG penFilled, ULONG penEmpty, BOOL flip) {    
    drawRSSIBarsDirect(rp, x, y, rssiToBars(rssi), penFilled, penEmpty, flip);
}

// Return a pointer to a single scan result
static void buildLVList(struct ScanResults* results) {
    NewList(&listViewList);

    for (int i = 0; i < results->count; i++) {
        struct ScanResult* result = &results->ScanResults[i];
        snprintf(listViewLines[i], LV_LINE_LEN, "%-22s     Ch:%-2d %ddBm", result->ssid, result->channel, (int)result->rssi);
        listViewNodes[i].ln_Name = listViewLines[i];
        listViewNodes[i].ln_Type = NT_USER;
        listViewNodes[i].ln_Pri  = 0;
        AddTail(&listViewList, &listViewNodes[i]);
    }
}

// Draw info for the selection    
static void drawSelection(struct RastPort *rp, struct ScanResult* result) {         
    SetAPen(rp, 0); 
    RectFill(rp, 10, RES_LV_H+9, RES_WIN_W-24, RES_LV_H+9+13); 
    if (!result) return;
        
    char detBuf[64]; 
    snprintf(detBuf, 64, "Ch: %d   RSSI: %ddBm", result->channel,  (int)result->rssi); 
    SetAPen(rp, 1); 
    Move(rp, 14, RES_LV_H + 18); 
    Text(rp, detBuf, strlen(detBuf)); 
    drawRSSIBars(rp,  RES_WIN_W - 20 - (5*(ICON_BAR_W+ICON_BAR_GAP)-ICON_BAR_GAP) - 4, RES_LV_H + 10, result->rssi, 2, 1, FALSE); 
}


// Shows the results from the access point scan. Returns the index number selected, or -1 if cancelled
int selectAccessPoint(int parentCenterX, int parentCenterY, struct ScanResults* results) {
    if (!results) return -1;
    struct Window  *resWin  = NULL;
    struct Gadget  *glist   = NULL, *gad;
    struct Gadget  *lvGad   = NULL;
    struct Gadget*  selectButton = NULL;
    struct NewGadget ng;
    struct TextAttr  ta = { (STRPTR)"topaz.font", 8, 0, 0 };
      
    int selected = -1; 
    struct Screen *scr = LockPubScreen(NULL);
    if (!scr) return -1;

    APTR vi = GetVisualInfo(scr, TAG_END);
    if (!vi) { 
        UnlockPubScreen(NULL, scr); 
        return -1; 
    }

    buildLVList(results); 

    gad = CreateContext(&glist);

    // List view for the access points
    ng.ng_LeftEdge   = 8;                   ng.ng_TopEdge    = 8;
    ng.ng_Width      = RES_WIN_W - 24;      ng.ng_Height     = RES_LV_H;
    ng.ng_GadgetText = NULL;
    ng.ng_GadgetID   = RES_GID_LIST;
    ng.ng_Flags      = 0;
    ng.ng_TextAttr   = &ta;
    ng.ng_VisualInfo = vi;
    gad = CreateGadget(LISTVIEW_KIND, gad, &ng, GTLV_Labels, (ULONG)&listViewList,  GTLV_ShowSelected, NULL,  GTLV_ScrollWidth,  16,  TAG_END);
    lvGad = gad;

    // Select
    ng.ng_LeftEdge   = RES_WIN_W - 8 - 80*2 - 16;
    ng.ng_TopEdge    = RES_LV_H + ICON_HEIGHT + 20;
    ng.ng_Width      = 80;  ng.ng_Height     = 18;
    ng.ng_GadgetText = (UBYTE *)"Select";
    ng.ng_GadgetID   = RES_GID_SELECT;
    ng.ng_Flags      = PLACETEXT_IN;
    gad = CreateGadget(BUTTON_KIND, gad, &ng, GA_Disabled, TRUE, TAG_END);
    selectButton = gad;

    // Cancel
    ng.ng_LeftEdge  += 88;
    ng.ng_GadgetText = (UBYTE *)"Cancel";
    ng.ng_GadgetID   = RES_GID_CANCEL;
    gad = CreateGadget(BUTTON_KIND, gad, &ng, TAG_END);

    if (!gad || !selectButton || !lvGad) { 
        UnlockPubScreen(NULL, scr);
        FreeVisualInfo(vi); 
        FreeGadgets(glist); 
        return -1; 
    }

    resWin = OpenWindowTags(NULL,
        WA_Left,        parentCenterX - (RES_WIN_W>>1),
        WA_Top,         parentCenterY - (RES_WIN_H>>1),
        WA_Width,       RES_WIN_W,
        WA_Height,      RES_WIN_H,
        WA_Title,       (ULONG)"Access Points Found",
        WA_Gadgets,     (ULONG)glist,
        WA_Flags,       WFLG_CLOSEGADGET | WFLG_DRAGBAR | WFLG_DEPTHGADGET | WFLG_ACTIVATE | WFLG_GIMMEZEROZERO,
        WA_IDCMP,       IDCMP_CLOSEWINDOW | IDCMP_GADGETUP | IDCMP_REFRESHWINDOW | LISTVIEWIDCMP,
        WA_PubScreen,   (ULONG)LockPubScreen(NULL),
        TAG_END);

    UnlockPubScreen(NULL, scr);

    if (!resWin) { 
        FreeVisualInfo(vi); 
        FreeGadgets(glist); 
        return -1; 
    }

    GT_RefreshWindow(resWin, NULL);


    // Draw a header row above the detail area
    struct RastPort *rp = resWin->RPort;
    struct TextFont *font = OpenFont(&ta);
    SetFont(rp, font);
    DrawBevelBox(rp, 8, RES_LV_H + 7, RES_WIN_W-24, 17,  GT_VisualInfo, (ULONG)vi, GTBB_Recessed, TRUE, TAG_DONE);


    BOOL done = FALSE;
    while (!done) {
        WaitPort(resWin->UserPort);

        struct IntuiMessage *msg;
        while ((msg = GT_GetIMsg(resWin->UserPort)) != NULL) {
            ULONG  cls  = msg->Class;
            UWORD  code = msg->Code;
            struct Gadget *g = (struct Gadget *)msg->IAddress;
            GT_ReplyIMsg(msg);

            switch (cls) {
            case IDCMP_CLOSEWINDOW:
                done = TRUE;
                selected = -1;
                break;

            case IDCMP_REFRESHWINDOW:
                GT_BeginRefresh(resWin);
                GT_EndRefresh(resWin, TRUE);
                DrawBevelBox(rp, 8, RES_LV_H + 7, RES_WIN_W-24, 17,  GT_VisualInfo, (ULONG)vi, GTBB_Recessed, TRUE, TAG_DONE);
                drawSelection(rp, &results->ScanResults[selected]);
                break;

            case IDCMP_GADGETUP:
                switch (g->GadgetID) {
                case RES_GID_LIST:
                    selected = (int)code;
                    drawSelection(rp, &results->ScanResults[code]);          
                    GT_SetGadgetAttrs(selectButton, resWin, NULL, GA_Disabled, selected <0 , TAG_END);
                    break;

                case RES_GID_SELECT:
                    if ((selected >= 0) && (selected < results->count)) done = TRUE;                        
                    break;

                case RES_GID_CANCEL:
                    done = TRUE;
                    selected = -1;
                    break;
                }
                break;
            }
        }
    }

    CloseFont(font);
    CloseWindow(resWin);
    FreeVisualInfo(vi);
    FreeGadgets(glist);
    return selected;
}




