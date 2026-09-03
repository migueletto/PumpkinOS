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

void palmos_CtlSysTrap(uint32_t sp, uint16_t idx, uint32_t trap) {

  switch (trap) {
    case sysTrapCtlNewControl: {
      // ControlType *CtlNewControl(void **formPP, UInt16 ID, ControlStyleType style, const Char *textP, Coord x, Coord y, Coord width, Coord height, FontID font, UInt8 group, Boolean leftAnchor)
      uint32_t formPP = ARG32;
      uint16_t id = ARG16;
      uint8_t style = ARG8;
      uint32_t textP = ARG32;
      int16_t x = ARG16;
      int16_t y = ARG16;
      int16_t width = ARG16;
      int16_t height = ARG16;
      uint8_t font = ARG8;
      uint8_t group = ARG8;
      uint8_t leftAnchor = ARG8;
      uint32_t formP = formPP ? m68k_read_memory_32(formPP) : 0;
      void *form = emupalmos_trap_in(formP, trap, 0);
      char *text = (char *)emupalmos_trap_in(textP, trap, 3);
      ControlType *ctl = CtlNewControl(&form, id, style, text, x, y, width, height, font, group, leftAnchor);
      uint32_t a = emupalmos_trap_out(ctl);
      debug(DEBUG_TRACE, "EmuPalmOS", "CtlNewControl(0x%08X, %u, %d, 0x%08X [%s], %d, %d, %d, %d, %d, %d, %d): 0x%08X", formPP, id, style, textP, text, x, y, width, height, font, group, leftAnchor, a);
      m68k_set_reg(M68K_REG_A0, a);
    }
    break;
    case sysTrapCtlGetStyle68K: {
      // ControlStyleType CtlGetStyle(ControlType *controlP)
      // custom trap created for use in 68K code
      uint32_t controlP = ARG32;
      ControlType *control = (ControlType *)emupalmos_trap_in(controlP, trap, 0);
      ControlStyleType style = control ? control->style : 0;
      debug(DEBUG_TRACE, "EmuPalmOS", "CtlGetStyle(0x%08X): %d", controlP, style);
      m68k_set_reg(M68K_REG_D0, style);
    }
    break;
    case sysTrapCtlGetLabel: {
      // const Char *CtlGetLabel(ControlType *controlP)
      uint32_t controlP = ARG32;
      ControlType *s_controlP = (ControlType *)emupalmos_trap_in(controlP, trap, 0);
      Char *res = (Char *)CtlGetLabel(s_controlP);
      uint32_t r_res = emupalmos_trap_out(res);
      m68k_set_reg(M68K_REG_A0, r_res);
      debug(DEBUG_TRACE, "EmuPalmOS", "CtlGetLabel(controlP=0x%08X): 0x%08X", controlP, r_res);
    }
    break;
    case sysTrapCtlDrawControl: {
      // void CtlDrawControl(in ControlType *controlP)
      uint32_t controlP = ARG32;
      ControlType *s_controlP = emupalmos_trap_in(controlP, trap, 0);
      CtlDrawControl(controlP ? s_controlP : NULL);
      debug(DEBUG_TRACE, "EmuPalmOS", "CtlDrawControl(controlP=0x%08X)", controlP);
    }
    break;
    case sysTrapCtlEraseControl: {
      // void CtlEraseControl(in ControlType *controlP)
      uint32_t controlP = ARG32;
      ControlType *s_controlP = emupalmos_trap_in(controlP, trap, 0);
      CtlEraseControl(controlP ? s_controlP : NULL);
      debug(DEBUG_TRACE, "EmuPalmOS", "CtlEraseControl(controlP=0x%08X)", controlP);
    }
    break;
    case sysTrapCtlHideControl: {
      // void CtlHideControl(in ControlType *controlP)
      uint32_t controlP = ARG32;
      ControlType *s_controlP = emupalmos_trap_in(controlP, trap, 0);
      CtlHideControl(controlP ? s_controlP : NULL);
      debug(DEBUG_TRACE, "EmuPalmOS", "CtlHideControl(controlP=0x%08X)", controlP);
    }
    break;
    case sysTrapCtlShowControl: {
      // void CtlShowControl(in ControlType *controlP)
      uint32_t controlP = ARG32;
      ControlType *s_controlP = emupalmos_trap_in(controlP, trap, 0);
      CtlShowControl(controlP ? s_controlP : NULL);
      debug(DEBUG_TRACE, "EmuPalmOS", "CtlShowControl(controlP=0x%08X)", controlP);
    }
    break;
    case sysTrapCtlEnabled: {
      // Boolean CtlEnabled(in ControlType *controlP)
      uint32_t controlP = ARG32;
      ControlType *s_controlP = emupalmos_trap_in(controlP, trap, 0);
      Boolean res = CtlEnabled(controlP ? s_controlP : NULL);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "CtlEnabled(controlP=0x%08X): %d", controlP, res);
    }
    break;
    case sysTrapCtlSetEnabled: {
      // void CtlSetEnabled(in ControlType *controlP, Boolean usable)
      uint32_t controlP = ARG32;
      ControlType *s_controlP = emupalmos_trap_in(controlP, trap, 0);
      uint8_t usable = ARG8;
      CtlSetEnabled(controlP ? s_controlP : NULL, usable);
      debug(DEBUG_TRACE, "EmuPalmOS", "CtlSetEnabled(controlP=0x%08X, usable=%d)", controlP, usable);
    }
    break;
    case sysTrapCtlSetUsable: {
      // void CtlSetUsable(in ControlType *controlP, Boolean usable)
      uint32_t controlP = ARG32;
      ControlType *s_controlP = emupalmos_trap_in(controlP, trap, 0);
      uint8_t usable = ARG8;
      CtlSetUsable(controlP ? s_controlP : NULL, usable);
      debug(DEBUG_TRACE, "EmuPalmOS", "CtlSetUsable(controlP=0x%08X, usable=%d)", controlP, usable);
    }
    break;
    case sysTrapCtlGetValue: {
      // Int16 CtlGetValue(in ControlType *controlP)
      uint32_t controlP = ARG32;
      ControlType *s_controlP = emupalmos_trap_in(controlP, trap, 0);
      Int16 res = CtlGetValue(controlP ? s_controlP : NULL);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "CtlGetValue(controlP=0x%08X): %d", controlP, res);
    }
    break;
    case sysTrapCtlSetValue: {
      // void CtlSetValue(in ControlType *controlP, Int16 newValue)
      uint32_t controlP = ARG32;
      ControlType *s_controlP = emupalmos_trap_in(controlP, trap, 0);
      int16_t newValue = ARG16;
      CtlSetValue(controlP ? s_controlP : NULL, newValue);
      debug(DEBUG_TRACE, "EmuPalmOS", "CtlSetValue(controlP=0x%08X, newValue=%d)", controlP, newValue);
    }
    break;
    case sysTrapCtlSetLabel: {
      // void CtlSetLabel(in ControlType *controlP, in Char *newLabel)
      uint32_t controlP = ARG32;
      ControlType *s_controlP = emupalmos_trap_in(controlP, trap, 0);
      uint32_t newLabel = ARG32;
      char *s_newLabel = emupalmos_trap_in(newLabel, trap, 1);
      CtlSetLabel(controlP ? s_controlP : NULL, newLabel ? s_newLabel : NULL);
      debug(DEBUG_TRACE, "EmuPalmOS", "CtlSetLabel(controlP=0x%08X, newLabel=0x%08X [%s])", controlP, newLabel, s_newLabel);
    }
    break;
    case sysTrapCtlSetGraphics: {
      // void CtlSetGraphics(in ControlType *ctlP, DmResID newBitmapID, DmResID newSelectedBitmapID)
      uint32_t ctlP = ARG32;
      uint8_t *ram = pumpkin_heap_base();
      ControlType *s_ctlP = ctlP ? (ControlType *)(ram + ctlP) : NULL;
      uint16_t newBitmapID = ARG16;
      uint16_t newSelectedBitmapID = ARG16;
      CtlSetGraphics(ctlP ? s_ctlP : NULL, newBitmapID, newSelectedBitmapID);
      debug(DEBUG_TRACE, "EmuPalmOS", "CtlSetGraphics(ctlP=0x%08X, newBitmapID=%d, newSelectedBitmapID=%d)", ctlP, newBitmapID, newSelectedBitmapID);
    }
    break;
    case sysTrapCtlSetSliderValues: {
      // void CtlSetSliderValues(in ControlType *ctlP, in UInt16 *minValueP, in UInt16 *maxValueP, in UInt16 *pageSizeP, in UInt16 *valueP)
      uint32_t ctlP = ARG32;
      uint8_t *ram = pumpkin_heap_base();
      ControlType *s_ctlP = ctlP ? (ControlType *)(ram + ctlP) : NULL;
      uint32_t minValueP = ARG32;
      UInt16 l_minValueP = 0;
      if (minValueP) l_minValueP = m68k_read_memory_16(minValueP);
      uint32_t maxValueP = ARG32;
      UInt16 l_maxValueP = 0;
      if (maxValueP) l_maxValueP = m68k_read_memory_16(maxValueP);
      uint32_t pageSizeP = ARG32;
      UInt16 l_pageSizeP = 0;
      if (pageSizeP) l_pageSizeP = m68k_read_memory_16(pageSizeP);
      uint32_t valueP = ARG32;
      UInt16 l_valueP = 0;
      if (valueP) l_valueP = m68k_read_memory_16(valueP);
      CtlSetSliderValues(ctlP ? s_ctlP : NULL, minValueP ? &l_minValueP : NULL, maxValueP ? &l_maxValueP : NULL, pageSizeP ? &l_pageSizeP : NULL, valueP ? &l_valueP : NULL);
      if (minValueP) m68k_write_memory_16(minValueP, l_minValueP);
      if (maxValueP) m68k_write_memory_16(maxValueP, l_maxValueP);
      if (pageSizeP) m68k_write_memory_16(pageSizeP, l_pageSizeP);
      if (valueP) m68k_write_memory_16(valueP, l_valueP);
      debug(DEBUG_TRACE, "EmuPalmOS", "CtlSetSliderValues(ctlP=0x%08X, minValueP=0x%08X [%d], maxValueP=0x%08X [%d], pageSizeP=0x%08X [%d], valueP=0x%08X [%d])", ctlP, minValueP, l_minValueP, maxValueP, l_maxValueP, pageSizeP, l_pageSizeP, valueP, l_valueP);
    }
    break;
    case sysTrapCtlGetSliderValues: {
      // void CtlGetSliderValues(in ControlType *ctlP, out UInt16 *minValueP, out UInt16 *maxValueP, out UInt16 *pageSizeP, out UInt16 *valueP)
      uint32_t ctlP = ARG32;
      uint8_t *ram = pumpkin_heap_base();
      ControlType *s_ctlP = ctlP ? (ControlType *)(ram + ctlP) : NULL;
      uint32_t minValueP = ARG32;
      UInt16 l_minValueP = 0;
      uint32_t maxValueP = ARG32;
      UInt16 l_maxValueP = 0;
      uint32_t pageSizeP = ARG32;
      UInt16 l_pageSizeP = 0;
      uint32_t valueP = ARG32;
      UInt16 l_valueP = 0;
      CtlGetSliderValues(ctlP ? s_ctlP : NULL, minValueP ? &l_minValueP : NULL, maxValueP ? &l_maxValueP : NULL, pageSizeP ? &l_pageSizeP : NULL, valueP ? &l_valueP : NULL);
      if (minValueP) m68k_write_memory_16(minValueP, l_minValueP);
      if (maxValueP) m68k_write_memory_16(maxValueP, l_maxValueP);
      if (pageSizeP) m68k_write_memory_16(pageSizeP, l_pageSizeP);
      if (valueP) m68k_write_memory_16(valueP, l_valueP);
      debug(DEBUG_TRACE, "EmuPalmOS", "CtlGetSliderValues(ctlP=0x%08X, minValueP=0x%08X [%d], maxValueP=0x%08X [%d], pageSizeP=0x%08X [%d], valueP=0x%08X [%d])", ctlP, minValueP, l_minValueP, maxValueP, l_maxValueP, pageSizeP, l_pageSizeP, valueP, l_valueP);
    }
    break;
    case sysTrapCtlHitControl: {
      // void CtlHitControl(in ControlType *controlP)
      uint32_t controlP = ARG32;
      ControlType *s_controlP = emupalmos_trap_in(controlP, trap, 0);
      CtlHitControl(controlP ? s_controlP : NULL);
      debug(DEBUG_TRACE, "EmuPalmOS", "CtlHitControl(controlP=0x%08X)", controlP);
    }
    break;
    case sysTrapCtlHandleEvent: {
      // Boolean CtlHandleEvent(in ControlType *controlP, in EventType *pEvent)
      uint32_t controlP = ARG32;
      ControlType *s_controlP = emupalmos_trap_in(controlP, trap, 0);
      uint32_t pEvent = ARG32;
      EventType l_pEvent;
      decode_event(pEvent, &l_pEvent);
      Boolean res = CtlHandleEvent(controlP ? s_controlP : NULL, pEvent ? &l_pEvent : NULL);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "CtlHandleEvent(controlP=0x%08X, pEvent=0x%08X): %d", controlP, pEvent, res);
    }
    break;
    case sysTrapCtlValidatePointer: {
      // Boolean CtlValidatePointer(in ControlType *controlP)
      uint32_t controlP = ARG32;
      ControlType *s_controlP = emupalmos_trap_in(controlP, trap, 0);
      Boolean res = CtlValidatePointer(controlP ? s_controlP : NULL);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "CtlValidatePointer(controlP=0x%08X): %d", controlP, res);
    }
    break;
  }
}
