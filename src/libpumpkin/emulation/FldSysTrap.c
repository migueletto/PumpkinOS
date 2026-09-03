#include <PalmOS.h>
#include <VFSMgr.h>
#include <DLServer.h>
#include <Helper.h>
#include <CharAttr.h>
#include <HsNavCommon.h>
#include <INetMgr.h>

#include "sys.h"
#ifdef ARMEMU
#include "armemu.h"
#include "armp.h"
#endif
#include "pumpkin.h"
#include "mutex.h"
#include "storage.h"
#include "logtrap.h"
#include "m68k/m68k.h"
#include "m68k/m68kcpu.h"
#include "emupalmos.h"
#include "debug.h"

void palmos_FldSysTrap(uint32_t sp, uint16_t idx, uint32_t trap) {

  switch (trap) {
    case sysTrapFldCopy: {
      // void FldCopy(in FieldType *fldP)
      uint32_t fldP = ARG32;
      FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
      FldCopy(fldP ? s_fldP : NULL);
      debug(DEBUG_TRACE, "EmuPalmOS", "FldCopy(fldP=0x%08X)", fldP);
    }
    break;
    case sysTrapFldCut: {
      // void FldCut(in FieldType *fldP)
      uint32_t fldP = ARG32;
      FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
      FldCut(fldP ? s_fldP : NULL);
      debug(DEBUG_TRACE, "EmuPalmOS", "FldCut(fldP=0x%08X)", fldP);
    }
    break;
    case sysTrapFldDrawField: {
      // void FldDrawField(in FieldType *fldP)
      uint32_t fldP = ARG32;
      FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
      FldDrawField(fldP ? s_fldP : NULL);
      debug(DEBUG_TRACE, "EmuPalmOS", "FldDrawField(fldP=0x%08X)", fldP);
    }
    break;
    case sysTrapFldEraseField: {
      // void FldEraseField(in FieldType *fldP)
      uint32_t fldP = ARG32;
      FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
      FldEraseField(fldP ? s_fldP : NULL);
      debug(DEBUG_TRACE, "EmuPalmOS", "FldEraseField(fldP=0x%08X)", fldP);
    }
    break;
    case sysTrapFldFreeMemory: {
      // void FldFreeMemory(in FieldType *fldP)
      uint32_t fldP = ARG32;
      FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
      FldFreeMemory(fldP ? s_fldP : NULL);
      debug(DEBUG_TRACE, "EmuPalmOS", "FldFreeMemory(fldP=0x%08X)", fldP);
    }
    break;
    case sysTrapFldGetBounds: {
      // void FldGetBounds(in FieldType *fldP, out RectangleType *rect)
      uint32_t fldP = ARG32;
      FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
      uint32_t rect = ARG32;
      RectangleType l_rect;
      FldGetBounds(fldP ? s_fldP : NULL, rect ? &l_rect : NULL);
      encode_rectangle(rect, &l_rect);
      debug(DEBUG_TRACE, "EmuPalmOS", "FldGetBounds(fldP=0x%08X, rect=0x%08X [%d,%d,%d,%d])", fldP, rect, l_rect.topLeft.x, l_rect.topLeft.y, l_rect.extent.x, l_rect.extent.y);
    }
    break;
    case sysTrapFldGetFont: {
      // FontID FldGetFont(in FieldType *fldP)
      uint32_t fldP = ARG32;
      FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
      FontID res = FldGetFont(fldP ? s_fldP : NULL);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "FldGetFont(fldP=0x%08X): %d", fldP, res);
    }
    break;
    case sysTrapFldGetSelection: {
      // void FldGetSelection(in FieldType *fldP, out UInt16 *startPosition, out UInt16 *endPosition)
      uint32_t fldP = ARG32;
      FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
      uint32_t startPosition = ARG32;
      UInt16 l_startPosition = 0;
      uint32_t endPosition = ARG32;
      UInt16 l_endPosition = 0;
      FldGetSelection(fldP ? s_fldP : NULL, startPosition ? &l_startPosition : NULL, endPosition ? &l_endPosition : NULL);
      if (startPosition) m68k_write_memory_16(startPosition, l_startPosition);
      if (endPosition) m68k_write_memory_16(endPosition, l_endPosition);
      debug(DEBUG_TRACE, "EmuPalmOS", "FldGetSelection(fldP=0x%08X, startPosition=0x%08X [%d], endPosition=0x%08X [%d])", fldP, startPosition, l_startPosition, endPosition, l_endPosition);
    }
    break;
    case sysTrapFldGetTextHandle: {
      // MemHandle FldGetTextHandle(in FieldType *fldP)
      uint32_t fldP = ARG32;
      FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
      MemHandle res = FldGetTextHandle(fldP ? s_fldP : NULL);
      uint32_t r_res = emupalmos_trap_out(res);
      m68k_set_reg(M68K_REG_A0, r_res);
      debug(DEBUG_TRACE, "EmuPalmOS", "FldGetTextHandle(fldP=0x%08X): 0x%08X", fldP, r_res);
    }
    break;
    case sysTrapFldGetTextPtr: {
      // Char *FldGetTextPtr(in FieldType *fldP)
      uint32_t fldP = ARG32;
      FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
      Char *res = FldGetTextPtr(fldP ? s_fldP : NULL);
      uint32_t r_res = emupalmos_trap_out(res);
      m68k_set_reg(M68K_REG_A0, r_res);
      debug(DEBUG_TRACE, "EmuPalmOS", "FldGetTextPtr(fldP=0x%08X): 0x%08X", fldP, r_res);
    }
    break;
    case sysTrapFldHandleEvent: {
      // Boolean FldHandleEvent(in FieldType *fldP, in EventType *eventP)
      uint32_t fldP = ARG32;
      FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
      uint32_t eventP = ARG32;
      EventType l_eventP;
      decode_event(eventP, &l_eventP);
      Boolean res = FldHandleEvent(fldP ? s_fldP : NULL, eventP ? &l_eventP : NULL);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "FldHandleEvent(fldP=0x%08X, eventP=0x%08X): %d", fldP, eventP, res);
    }
    break;
    case sysTrapFldPaste: {
      // void FldPaste(in FieldType *fldP)
      uint32_t fldP = ARG32;
      FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
      FldPaste(fldP ? s_fldP : NULL);
      debug(DEBUG_TRACE, "EmuPalmOS", "FldPaste(fldP=0x%08X)", fldP);
    }
    break;
    case sysTrapFldRecalculateField: {
      // void FldRecalculateField(in FieldType *fldP, Boolean redraw)
      uint32_t fldP = ARG32;
      FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
      uint8_t redraw = ARG8;
      FldRecalculateField(fldP ? s_fldP : NULL, redraw);
      debug(DEBUG_TRACE, "EmuPalmOS", "FldRecalculateField(fldP=0x%08X, redraw=%d)", fldP, redraw);
    }
    break;
    case sysTrapFldSetBounds: {
      // void FldSetBounds(in FieldType *fldP, in RectangleType *rP)
      uint32_t fldP = ARG32;
      FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
      uint32_t rP = ARG32;
      RectangleType l_rP;
      decode_rectangle(rP, &l_rP);
      FldSetBounds(fldP ? s_fldP : NULL, rP ? &l_rP : NULL);
      debug(DEBUG_TRACE, "EmuPalmOS", "FldSetBounds(fldP=0x%08X, rP=0x%08X [%d,%d,%d,%d])", fldP, rP, l_rP.topLeft.x, l_rP.topLeft.y, l_rP.extent.x, l_rP.extent.y);
    }
    break;
    case sysTrapFldSetFont: {
      // void FldSetFont(in FieldType *fldP, FontID fontID)
      uint32_t fldP = ARG32;
      FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
      uint8_t fontID = ARG8;
      FldSetFont(fldP ? s_fldP : NULL, fontID);
      debug(DEBUG_TRACE, "EmuPalmOS", "FldSetFont(fldP=0x%08X, fontID=%d)", fldP, fontID);
    }
    break;
    case sysTrapFldSetText: {
      // void FldSetText(in FieldType *fldP, MemHandle textHandle, UInt16 offset, UInt16 size)
      uint32_t fldP = ARG32;
      FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
      uint32_t textHandle = ARG32;
      uint8_t *ram = pumpkin_heap_base();
      MemHandle l_textHandle = textHandle ? ram + textHandle : NULL;
      uint16_t offset = ARG16;
      uint16_t size = ARG16;
      FldSetText(fldP ? s_fldP : NULL, textHandle ? l_textHandle : 0, offset, size);
      debug(DEBUG_TRACE, "EmuPalmOS", "FldSetText(fldP=0x%08X, textHandle=0x%08X, offset=%d, size=%d)", fldP, textHandle, offset, size);
    }
    break;
    case sysTrapFldSetTextHandle: {
      // void FldSetTextHandle(in FieldType *fldP, MemHandle textHandle)
      uint32_t fldP = ARG32;
      FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
      uint32_t textHandle = ARG32;
      uint8_t *ram = pumpkin_heap_base();
      MemHandle l_textHandle = textHandle ? ram + textHandle : NULL;
      FldSetTextHandle(fldP ? s_fldP : NULL, textHandle ? l_textHandle : 0);
      debug(DEBUG_TRACE, "EmuPalmOS", "FldSetTextHandle(fldP=0x%08X, textHandle=0x%08X)", fldP, textHandle);
    }
    break;
    case sysTrapFldSetTextPtr: {
      // void FldSetTextPtr(in FieldType *fldP, in Char *textP)
      uint32_t fldP = ARG32;
      FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
      uint32_t textP = ARG32;
      uint8_t *ram = pumpkin_heap_base();
      char *s_textP = textP ? (char *)(ram + textP) : NULL;
      FldSetTextPtr(fldP ? s_fldP : NULL, textP ? s_textP : NULL);
      debug(DEBUG_TRACE, "EmuPalmOS", "FldSetTextPtr(fldP=0x%08X, textP=0x%08X [%s])", fldP, textP, s_textP);
    }
    break;
    case sysTrapFldSetUsable: {
      // void FldSetUsable(in FieldType *fldP, Boolean usable)
      uint32_t fldP = ARG32;
      FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
      uint8_t usable = ARG8;
      FldSetUsable(fldP ? s_fldP : NULL, usable);
      debug(DEBUG_TRACE, "EmuPalmOS", "FldSetUsable(fldP=0x%08X, usable=%d)", fldP, usable);
    }
    break;
    case sysTrapFldSetSelection: {
      // void FldSetSelection(in FieldType *fldP, UInt16 startPosition, UInt16 endPosition)
      uint32_t fldP = ARG32;
      FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
      uint16_t startPosition = ARG16;
      uint16_t endPosition = ARG16;
      FldSetSelection(fldP ? s_fldP : NULL, startPosition, endPosition);
      debug(DEBUG_TRACE, "EmuPalmOS", "FldSetSelection(fldP=0x%08X, startPosition=%d, endPosition=%d)", fldP, startPosition, endPosition);
    }
    break;
    case sysTrapFldGrabFocus: {
      // void FldGrabFocus(in FieldType *fldP)
      uint32_t fldP = ARG32;
      FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
      FldGrabFocus(fldP ? s_fldP : NULL);
      debug(DEBUG_TRACE, "EmuPalmOS", "FldGrabFocus(fldP=0x%08X)", fldP);
    }
    break;
    case sysTrapFldReleaseFocus: {
      // void FldReleaseFocus(in FieldType *fldP)
      uint32_t fldP = ARG32;
      FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
      FldReleaseFocus(fldP ? s_fldP : NULL);
      debug(DEBUG_TRACE, "EmuPalmOS", "FldReleaseFocus(fldP=0x%08X)", fldP);
    }
    break;
    case sysTrapFldGetInsPtPosition: {
      // UInt16 FldGetInsPtPosition(in FieldType *fldP)
      uint32_t fldP = ARG32;
      FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
      UInt16 res = FldGetInsPtPosition(fldP ? s_fldP : NULL);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "FldGetInsPtPosition(fldP=0x%08X): %d", fldP, res);
    }
    break;
    case sysTrapFldSetInsPtPosition: {
      // void FldSetInsPtPosition(in FieldType *fldP, UInt16 pos)
      uint32_t fldP = ARG32;
      FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
      uint16_t pos = ARG16;
      FldSetInsPtPosition(fldP ? s_fldP : NULL, pos);
      debug(DEBUG_TRACE, "EmuPalmOS", "FldSetInsPtPosition(fldP=0x%08X, pos=%d)", fldP, pos);
    }
    break;
    case sysTrapFldSetInsertionPoint: {
      // void FldSetInsertionPoint(in FieldType *fldP, UInt16 pos)
      uint32_t fldP = ARG32;
      FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
      uint16_t pos = ARG16;
      FldSetInsertionPoint(fldP ? s_fldP : NULL, pos);
      debug(DEBUG_TRACE, "EmuPalmOS", "FldSetInsertionPoint(fldP=0x%08X, pos=%d)", fldP, pos);
    }
    break;
    case sysTrapFldGetScrollPosition: {
      // UInt16 FldGetScrollPosition(in FieldType *fldP)
      uint32_t fldP = ARG32;
      FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
      UInt16 res = FldGetScrollPosition(fldP ? s_fldP : NULL);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "FldGetScrollPosition(fldP=0x%08X): %d", fldP, res);
    }
    break;
    case sysTrapFldSetScrollPosition: {
      // void FldSetScrollPosition(in FieldType *fldP, UInt16 pos)
      uint32_t fldP = ARG32;
      FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
      uint16_t pos = ARG16;
      FldSetScrollPosition(fldP ? s_fldP : NULL, pos);
      debug(DEBUG_TRACE, "EmuPalmOS", "FldSetScrollPosition(fldP=0x%08X, pos=%d)", fldP, pos);
    }
    break;
    case sysTrapFldGetScrollValues: {
      // void FldGetScrollValues(in FieldType *fldP, out UInt16 *scrollPosP, out UInt16 *textHeightP, out UInt16 *fieldHeightP)
      uint32_t fldP = ARG32;
      FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
      uint32_t scrollPosP = ARG32;
      UInt16 l_scrollPosP = 0;
      uint32_t textHeightP = ARG32;
      UInt16 l_textHeightP = 0;
      uint32_t fieldHeightP = ARG32;
      UInt16 l_fieldHeightP = 0;
      FldGetScrollValues(fldP ? s_fldP : NULL, scrollPosP ? &l_scrollPosP : NULL, textHeightP ? &l_textHeightP : NULL, fieldHeightP ? &l_fieldHeightP : NULL);
      if (scrollPosP) m68k_write_memory_16(scrollPosP, l_scrollPosP);
      if (textHeightP) m68k_write_memory_16(textHeightP, l_textHeightP);
      if (fieldHeightP) m68k_write_memory_16(fieldHeightP, l_fieldHeightP);
      debug(DEBUG_TRACE, "EmuPalmOS", "FldGetScrollValues(fldP=0x%08X, scrollPosP=0x%08X [%d], textHeightP=0x%08X [%d], fieldHeightP=0x%08X [%d])", fldP, scrollPosP, l_scrollPosP, textHeightP, l_textHeightP, fieldHeightP, l_fieldHeightP);
    }
    break;
    case sysTrapFldGetTextLength: {
      // UInt16 FldGetTextLength(in FieldType *fldP)
      uint32_t fldP = ARG32;
      FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
      UInt16 res = FldGetTextLength(fldP ? s_fldP : NULL);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "FldGetTextLength(fldP=0x%08X): %d", fldP, res);
    }
    break;
    case sysTrapFldScrollField: {
      // void FldScrollField(in FieldType *fldP, UInt16 linesToScroll, WinDirectionType direction)
      uint32_t fldP = ARG32;
      FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
      uint16_t linesToScroll = ARG16;
      uint8_t direction = ARG8;
      FldScrollField(fldP ? s_fldP : NULL, linesToScroll, direction);
      debug(DEBUG_TRACE, "EmuPalmOS", "FldScrollField(fldP=0x%08X, linesToScroll=%d, direction=%d)", fldP, linesToScroll, direction);
    }
    break;
    case sysTrapFldScrollable: {
      // Boolean FldScrollable(in FieldType *fldP, WinDirectionType direction)
      uint32_t fldP = ARG32;
      FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
      uint8_t direction = ARG8;
      Boolean res = FldScrollable(fldP ? s_fldP : NULL, direction);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "FldScrollable(fldP=0x%08X, direction=%d): %d", fldP, direction, res);
    }
    break;
    case sysTrapFldGetVisibleLines: {
      // UInt16 FldGetVisibleLines(in FieldType *fldP)
      uint32_t fldP = ARG32;
      FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
      UInt16 res = FldGetVisibleLines(fldP ? s_fldP : NULL);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "FldGetVisibleLines(fldP=0x%08X): %d", fldP, res);
    }
    break;
    case sysTrapFldGetTextHeight: {
      // UInt16 FldGetTextHeight(in FieldType *fldP)
      uint32_t fldP = ARG32;
      FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
      UInt16 res = FldGetTextHeight(fldP ? s_fldP : NULL);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "FldGetTextHeight(fldP=0x%08X): %d", fldP, res);
    }
    break;
    case sysTrapFldCalcFieldHeight: {
      // UInt16 FldCalcFieldHeight(in Char *chars, UInt16 maxWidth)
      uint32_t chars = ARG32;
      uint8_t *ram = pumpkin_heap_base();
      char *s_chars = chars ? (char *)(ram + chars) : NULL;
      uint16_t maxWidth = ARG16;
      UInt16 res = FldCalcFieldHeight(chars ? s_chars : NULL, maxWidth);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "FldCalcFieldHeight(chars=0x%08X [%s], maxWidth=%d): %d", chars, s_chars, maxWidth, res);
    }
    break;
    case sysTrapFldWordWrap: {
      // UInt16 FldWordWrap(in Char *chars, Int16 maxWidth)
      uint32_t chars = ARG32;
      uint8_t *ram = pumpkin_heap_base();
      char *s_chars = chars ? (char *)(ram + chars) : NULL;
      int16_t maxWidth = ARG16;
      UInt16 res = FldWordWrap(chars ? s_chars : NULL, maxWidth);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "FldWordWrap(chars=0x%08X [%s], maxWidth=%d): %d", chars, s_chars, maxWidth, res);
    }
    break;
    case sysTrapFldCompactText: {
      // void FldCompactText(in FieldType *fldP)
      uint32_t fldP = ARG32;
      FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
      FldCompactText(fldP ? s_fldP : NULL);
      debug(DEBUG_TRACE, "EmuPalmOS", "FldCompactText(fldP=0x%08X)", fldP);
    }
    break;
    case sysTrapFldDirty: {
      // Boolean FldDirty(in FieldType *fldP)
      uint32_t fldP = ARG32;
      FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
      Boolean res = FldDirty(fldP ? s_fldP : NULL);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "FldDirty(fldP=0x%08X): %d", fldP, res);
    }
    break;
    case sysTrapFldSetDirty: {
      // void FldSetDirty(in FieldType *fldP, Boolean dirty)
      uint32_t fldP = ARG32;
      FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
      uint8_t dirty = ARG8;
      FldSetDirty(fldP ? s_fldP : NULL, dirty);
      debug(DEBUG_TRACE, "EmuPalmOS", "FldSetDirty(fldP=0x%08X, dirty=%d)", fldP, dirty);
    }
    break;
    case sysTrapFldGetMaxChars: {
      // UInt16 FldGetMaxChars(in FieldType *fldP)
      uint32_t fldP = ARG32;
      FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
      UInt16 res = FldGetMaxChars(fldP ? s_fldP : NULL);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "FldGetMaxChars(fldP=0x%08X): %d", fldP, res);
    }
    break;
    case sysTrapFldSetMaxChars: {
      // void FldSetMaxChars(in FieldType *fldP, UInt16 maxChars)
      uint32_t fldP = ARG32;
      FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
      uint16_t maxChars = ARG16;
      FldSetMaxChars(fldP ? s_fldP : NULL, maxChars);
      debug(DEBUG_TRACE, "EmuPalmOS", "FldSetMaxChars(fldP=0x%08X, maxChars=%d)", fldP, maxChars);
    }
    break;
    case sysTrapFldInsert: {
      // Boolean FldInsert(in FieldType *fldP, in Char *insertChars, UInt16 insertLen)
      uint32_t fldP = ARG32;
      FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
      uint32_t insertChars = ARG32;
      uint8_t *ram = pumpkin_heap_base();
      char *s_insertChars = insertChars ? (char *)(ram + insertChars) : NULL;
      uint16_t insertLen = ARG16;
      Boolean res = FldInsert(fldP ? s_fldP : NULL, insertChars ? s_insertChars : NULL, insertLen);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "FldInsert(fldP=0x%08X, insertChars=0x%08X [%s], insertLen=%d): %d", fldP, insertChars, s_insertChars, insertLen, res);
    }
    break;
    case sysTrapFldDelete: {
      // void FldDelete(in FieldType *fldP, UInt16 start, UInt16 end)
      uint32_t fldP = ARG32;
      FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
      uint16_t start = ARG16;
      uint16_t end = ARG16;
      FldDelete(fldP ? s_fldP : NULL, start, end);
      debug(DEBUG_TRACE, "EmuPalmOS", "FldDelete(fldP=0x%08X, start=%d, end=%d)", fldP, start, end);
    }
    break;
    case sysTrapFldUndo: {
      // void FldUndo(in FieldType *fldP)
      uint32_t fldP = ARG32;
      FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
      FldUndo(fldP ? s_fldP : NULL);
      debug(DEBUG_TRACE, "EmuPalmOS", "FldUndo(fldP=0x%08X)", fldP);
    }
    break;
    case sysTrapFldGetTextAllocatedSize: {
      // UInt16 FldGetTextAllocatedSize(in FieldType *fldP)
      uint32_t fldP = ARG32;
      FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
      UInt16 res = FldGetTextAllocatedSize(fldP ? s_fldP : NULL);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "FldGetTextAllocatedSize(fldP=0x%08X): %d", fldP, res);
    }
    break;
    case sysTrapFldSetTextAllocatedSize: {
      // void FldSetTextAllocatedSize(in FieldType *fldP, UInt16 allocatedSize)
      uint32_t fldP = ARG32;
      FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
      uint16_t allocatedSize = ARG16;
      FldSetTextAllocatedSize(fldP ? s_fldP : NULL, allocatedSize);
      debug(DEBUG_TRACE, "EmuPalmOS", "FldSetTextAllocatedSize(fldP=0x%08X, allocatedSize=%d)", fldP, allocatedSize);
    }
    break;
    case sysTrapFldGetAttributes: {
      // void FldGetAttributes(in FieldType *fldP, in FieldAttrType *attrP)
      uint32_t fldP = ARG32;
      FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
      uint32_t attrP = ARG32;
      UInt16 attrBits;
      FieldAttrType attrFields;
      FldGetAttributes(fldP ? s_fldP : NULL, attrP ? &attrFields : NULL);
      if (attrP) {
        attrBits = 0;
        if (attrFields.usable)       attrBits |= 0x8000;
        if (attrFields.visible)      attrBits |= 0x4000;
        if (attrFields.editable)     attrBits |= 0x2000;
        if (attrFields.singleLine)   attrBits |= 0x1000;
        if (attrFields.hasFocus)     attrBits |= 0x0800;
        if (attrFields.dynamicSize)  attrBits |= 0x0400;
        if (attrFields.insPtVisible) attrBits |= 0x0200;
        if (attrFields.dirty)        attrBits |= 0x0100;
        attrBits |= attrFields.underlined    << 6;
        attrBits |= attrFields.justification << 4;
        if (attrFields.autoShift)    attrBits |= 0x0008;
        if (attrFields.hasScrollBar) attrBits |= 0x0004;
        if (attrFields.numeric)      attrBits |= 0x0002;
        if (attrFields.reserved)     attrBits |= 0x0001;
        m68k_write_memory_16(attrP, attrBits);
      }
      debug(DEBUG_TRACE, "EmuPalmOS", "FldGetAttributes(fldP=0x%08X, attrP=0x%08X)", fldP, attrP);
    }
    break;
    case sysTrapFldSetAttributes: {
      // void FldSetAttributes(in FieldType *fldP, in FieldAttrType *attrP)
      uint32_t fldP = ARG32;
      FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
      uint32_t attrP = ARG32;
      UInt16 attrBits;
      FieldAttrType attrFields;
      attrBits = attrP ? m68k_read_memory_16(attrP) : 0;
      if (attrP) {
        attrFields.usable        = (attrBits & 0x8000) ? 1 : 0;
        attrFields.visible       = (attrBits & 0x4000) ? 1 : 0;
        attrFields.editable      = (attrBits & 0x2000) ? 1 : 0;
        attrFields.singleLine    = (attrBits & 0x1000) ? 1 : 0;
        attrFields.hasFocus      = (attrBits & 0x0800) ? 1 : 0;
        attrFields.dynamicSize   = (attrBits & 0x0400) ? 1 : 0;
        attrFields.insPtVisible  = (attrBits & 0x0200) ? 1 : 0;
        attrFields.dirty         = (attrBits & 0x0100) ? 1 : 0;
        attrFields.underlined    = (attrBits & 0x00c0) >> 6;
        attrFields.justification = (attrBits & 0x0030) >> 4;
        attrFields.autoShift     = (attrBits & 0x0008) ? 1 : 0;
        attrFields.hasScrollBar  = (attrBits & 0x0004) ? 1 : 0;
        attrFields.numeric       = (attrBits & 0x0002) ? 1 : 0;
        attrFields.reserved      = (attrBits & 0x0001) ? 1 : 0;
      }
      FldSetAttributes(fldP ? s_fldP : NULL, attrP ? &attrFields : NULL);
      debug(DEBUG_TRACE, "EmuPalmOS", "FldSetAttributes(fldP=0x%08X, attrP=0x%08X)", fldP, attrP);
    }
    break;
    case sysTrapFldSendChangeNotification: {
      // void FldSendChangeNotification(in FieldType *fldP)
      uint32_t fldP = ARG32;
      FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
      FldSendChangeNotification(fldP ? s_fldP : NULL);
      debug(DEBUG_TRACE, "EmuPalmOS", "FldSendChangeNotification(fldP=0x%08X)", fldP);
    }
    break;
    case sysTrapFldSendHeightChangeNotification: {
      // void FldSendHeightChangeNotification(in FieldType *fldP, UInt16 pos, Int16 numLines)
      uint32_t fldP = ARG32;
      FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
      uint16_t pos = ARG16;
      int16_t numLines = ARG16;
      FldSendHeightChangeNotification(fldP ? s_fldP : NULL, pos, numLines);
      debug(DEBUG_TRACE, "EmuPalmOS", "FldSendHeightChangeNotification(fldP=0x%08X, pos=%d, numLines=%d)", fldP, pos, numLines);
    }
    break;
    case sysTrapFldMakeFullyVisible: {
      // Boolean FldMakeFullyVisible(in FieldType *fldP)
      uint32_t fldP = ARG32;
      FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
      Boolean res = FldMakeFullyVisible(fldP ? s_fldP : NULL);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "FldMakeFullyVisible(fldP=0x%08X): %d", fldP, res);
    }
    break;
    case sysTrapFldGetNumberOfBlankLines: {
      // UInt16 FldGetNumberOfBlankLines(in FieldType *fldP)
      uint32_t fldP = ARG32;
      FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
      UInt16 res = FldGetNumberOfBlankLines(fldP ? s_fldP : NULL);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "FldGetNumberOfBlankLines(fldP=0x%08X): %d", fldP, res);
    }
    break;
    case sysTrapFldSetMaxVisibleLines: {
      // void FldSetMaxVisibleLines(in FieldType *fldP, UInt8 maxLines)
      uint32_t fldP = ARG32;
      FieldType *s_fldP = emupalmos_trap_in(fldP, trap, 0);
      uint8_t maxLines = ARG8;
      FldSetMaxVisibleLines(fldP ? s_fldP : NULL, maxLines);
      debug(DEBUG_TRACE, "EmuPalmOS", "FldSetMaxVisibleLines(fldP=0x%08X, maxLines=%d)", fldP, maxLines);
    }
    break;
    case sysTrapFldNewField: {
      // FieldType *FldNewField(void **formPP, UInt16 id,
      //   Coord x, Coord y, Coord width, Coord height,
      //   FontID font, UInt32 maxChars, Boolean editable, Boolean underlined,
      //   Boolean singleLine, Boolean dynamicSize, JustificationType justification,
      //   Boolean autoShift, Boolean hasScrollBar, Boolean numeric)
      uint32_t formPP = ARG32;
      uint16_t id = ARG16;
      int16_t x = ARG16;
      int16_t y = ARG16;
      int16_t width = ARG16;
      int16_t height = ARG16;
      uint8_t font = ARG8;
      uint32_t maxChars = ARG32;
      uint8_t editable = ARG8;
      uint8_t underlined = ARG8;
      uint8_t singleLine = ARG8;
      uint8_t dynamicSize = ARG8;
      uint8_t justification = ARG8;
      uint8_t autoShift = ARG8;
      uint8_t hasScrollBar = ARG8;
      uint8_t numeric = ARG8;
      uint32_t formP = formPP ? m68k_read_memory_32(formPP) : 0;
      void *form = emupalmos_trap_in(formP, trap, 0);
      FieldType *fld = FldNewField(&form, id, x, y, width, height,
        font, maxChars, editable, underlined,
        singleLine, dynamicSize, justification,
        autoShift, hasScrollBar, numeric);
      uint32_t a = emupalmos_trap_out(fld);
      debug(DEBUG_TRACE, "EmuPalmOS", "FldNewField(0x%08X, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d): 0x%08X",
        formPP, id, x, y, width, height, font, maxChars,
        editable, underlined, singleLine, dynamicSize, justification, autoShift, hasScrollBar, numeric, a);
      m68k_set_reg(M68K_REG_A0, a);
    }
    break;
  }
}
