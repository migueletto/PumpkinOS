#include <PalmOS.h>
#include <VFSMgr.h>

#ifdef ARMEMU
#include "armemu.h"
#endif
#include "logtrap.h"
#include "m68k/m68k.h"
#include "m68k/m68kcpu.h"
#include "emupalmosinc.h"
#include "emupalmos.h"
#include "debug.h"

ProgressPtr	PrgStartDialogV31(const Char *title, PrgCallbackFunc textCallback) {
  return PrgStartDialog(title, textCallback, NULL);
}

ProgressPtr	PrgStartDialog(const Char *title, PrgCallbackFunc textCallback, void *userDataP) {
  ProgressPtr	prgP = NULL;
  UInt16 index;
  
  if (title && textCallback && (prgP = MemPtrNew(sizeof(ProgressType))) != NULL) {
    StrNCopy(prgP->title, title, progressMaxTitle);
    prgP->textCallback = textCallback;
    prgP->userDataP = userDataP;
    prgP->oldFrmP = FrmGetActiveForm();
    prgP->frmP = FrmNewForm(1000, title, 2, 76, 156, 82, true, 0, 0, 0);
    FrmNewBitmap(&prgP->frmP, 1000, 10004, 0, 15);
    index = FrmGetObjectIndex(prgP->frmP, 1000);
    FrmHideObject(prgP->frmP, index);
    FldNewField((void **)&prgP->frmP, 2000, 0, 40, 76, FntCharHeight(), stdFont, progressMaxMessage, false, false,
      true, false, leftAlign, false, false, false);
    FrmSetActiveForm(prgP->frmP);
    FrmDrawForm(prgP->frmP);
  }

  return prgP;
}

void PrgStopDialog(ProgressPtr prgP, Boolean force) {
  if (prgP) {
    FrmEraseForm(prgP->frmP);
    FrmSetActiveForm(prgP->oldFrmP);
    FrmDeleteForm(prgP->frmP);
    MemPtrFree(prgP);
  }
}

Boolean	PrgHandleEvent(ProgressPtr prgP, EventType *eventP) {
  PrgCallbackData data;
  char text[progressMaxMessage];
  uint32_t addr;
  uint16_t flags;
  uint8_t *ram, *d;
  UInt16 index;
  FieldAttrType attr;
  FieldType *fld;
  FormBitmapType *bmp;
  Boolean handled = false;

  if (prgP) {
    if (eventP) {
      handled = SysHandleEvent(eventP);
    }

    if (prgP->needUpdate) {
      MemSet(&data, sizeof(PrgCallbackData), 0);
      debug(DEBUG_INFO, "Progress", "PrgHandleEvent stage %d error %d message [%s] ...", prgP->stage, prgP->error, prgP->message);

      if (pumpkin_is_m68k()) {
        if ((d = MemPtrNew(sizeof(PrgCallbackData) + progressMaxMessage)) != NULL) {
          ram = pumpkin_heap_base();
          addr = d - ram;
          m68k_write_memory_16(addr +  0, prgP->stage);
          m68k_write_memory_32(addr +  2, addr + sizeof(PrgCallbackData));
          m68k_write_memory_16(addr +  6, progressMaxMessage);
          m68k_write_memory_32(addr +  8, (uint8_t *)prgP->message - ram);
          m68k_write_memory_16(addr + 12, prgP->error);
          m68k_write_memory_16(addr + 16, 0x2000); // textChanged = 1
          m68k_write_memory_32(addr + 38, (uint8_t *)prgP->userDataP - ram);
          CallPrgCallback((uint8_t *)prgP->textCallback - ram, addr);
          data.bitmapId = m68k_read_memory_16(addr + 14);
          flags = m68k_read_memory_16(addr + 16);
          data.textChanged = (flags & 0x2000) ? 1 : 0;
          data.textP = text;
          MemMove(data.textP, (char *)d + sizeof(PrgCallbackData), progressMaxMessage);
          MemPtrFree(d);
        }
      } else {
        data.stage = prgP->stage;
        data.textP = text;
        data.textLen = progressMaxMessage;
        data.message = prgP->message;
        data.error = prgP->error;
        data.userDataP = prgP->userDataP;
        prgP->textCallback(&data);
      }

      debug(DEBUG_INFO, "Progress", "PrgHandleEvent text [%s] bitmapId %u textChanged %d", data.textP, data.bitmapId, data.textChanged);

      // update bitmap
      if (data.bitmapId != prgP->lastBitmapID) {
        index = FrmGetObjectIndex(prgP->frmP, 1000);
        bmp = (FormBitmapType *)FrmGetObjectPtr(prgP->frmP, index);
        if (prgP->lastBitmapID == 0) {
          bmp->rscID = data.bitmapId;
          FrmShowObject(prgP->frmP, index);
        } else if (data.bitmapId == 0) {
          FrmHideObject(prgP->frmP, index);
        }
        prgP->lastBitmapID = data.bitmapId;
      }

      // update text
      index = FrmGetObjectIndex(prgP->frmP, 2000);
      fld = (FieldType *)FrmGetObjectPtr(prgP->frmP, index);
      FldGetAttributes(fld, &attr);
      attr.editable = 1;
      FldSetAttributes(fld, &attr);
      FldDelete(fld, 0, progressMaxMessage);
      FldInsert(fld, data.textP, StrLen(data.textP));
      attr.editable = 0;
      FldSetAttributes(fld, &attr);

      prgP->needUpdate = 0;
    }
  }

  return handled;
}

void PrgUpdateDialog(ProgressPtr prgP, UInt16 err, UInt16 stage, const Char *messageP, Boolean updateNow) {
  if (prgP) {
    prgP->error = err;
    prgP->stage = stage;
    if (messageP) StrNCopy(prgP->message, messageP, progressMaxMessage);
    prgP->needUpdate = 1;

    if (updateNow) {
      PrgHandleEvent(prgP, NULL);
    }
  }
}

/*
   0 UInt16 stage;         // <= current stage
   2 Char *textP;          // => buffer to hold text to display
   6 UInt16 textLen;       // <= length of text buffer
   8 Char  *message;       // <= additional text for display
  12 Err error;            // <= current error
  14 UInt16 bitmapId;        // => resource ID of bitmap to display
  16 UInt16 canceled:1;      // <= true if user has pressed the cancel button      
     UInt16 showDetails:1;   // <= true if user pressed down arrow for more details
     UInt16 textChanged:1;   // => if true then update text (defaults to true)
     UInt16 timedOut:1;      // <= true if update caused by a timeout
  18 UInt32 timeout;       // <> timeout in ticks to force next update (for animation)
  22 UInt32 barMaxValue;     // the maximum value for the progress bar, if = 0 then the bar is
  26 UInt32 barCurValue;     // the current value of the progress bar, the bar will be drawn 
  30 Char   *barMessage;     // additional text for display below the progress bar.
  34 UInt16  barFlags;     // reserved for future use.
  36 UInt16  delay:1;        // => if true delay 1 second after updating form icon/msg
  38 void *  userDataP;      // <= context pointer that caller passed to PrgStartDialog
*/
