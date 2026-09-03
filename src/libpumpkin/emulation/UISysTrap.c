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

void palmos_UISysTrap(uint32_t sp, uint16_t idx, uint32_t trap) {

  switch (trap) {
    case sysTrapUIColorPushTable: {
      // Err UIColorPushTable(void)
      Err err = UIColorPushTable();
      debug(DEBUG_TRACE, "EmuPalmOS", "UIColorPushTable(): %d", err);
      m68k_set_reg(M68K_REG_D0, err);
    }
    break;
    case sysTrapUIColorPopTable: {
      // Err UIColorPopTable(void)
      Err err = UIColorPopTable();
      debug(DEBUG_TRACE, "EmuPalmOS", "UIColorPopTable(): %d", err);
      m68k_set_reg(M68K_REG_D0, err);
    }
    break;
    case sysTrapUIColorSetTableEntry: {
      // Err UIColorSetTableEntry(UIColorTableEntries which, const RGBColorType *rgbP)
      uint8_t which = ARG8;
      uint32_t rgbP = ARG32;
      emupalmos_trap_in(rgbP, trap, 1);
      RGBColorType rgb;
      decode_rgb(rgbP, &rgb);
      Err err = UIColorSetTableEntry(which, rgbP ? &rgb : NULL);
      debug(DEBUG_TRACE, "EmuPalmOS", "UIColorSetTableEntry(%d, 0x%08X [%d,%d,%d,%d]): %d", which, rgbP, rgb.index, rgb.r, rgb.g, rgb.b, err);
      m68k_set_reg(M68K_REG_D0, err);
    }
    break;
    case sysTrapUIColorGetTableEntryRGB: {
      // void UIColorGetTableEntryRGB(UIColorTableEntries which, RGBColorType *rgbP)
      uint8_t which = ARG8;
      uint32_t rgbP = ARG32;
      emupalmos_trap_in(rgbP, trap, 1);
      RGBColorType rgb;
      UIColorGetTableEntryRGB(which, rgbP ? &rgb : NULL);
      encode_rgb(rgbP, &rgb);
      debug(DEBUG_TRACE, "EmuPalmOS", "UIColorGetTableEntryRGB(%d, 0x%08X [%d,%d,%d,%d])", which, rgbP, rgb.index, rgb.r, rgb.g, rgb.b);
    }
    break;
    case sysTrapUIColorGetTableEntryIndex: {
      // IndexedColorType UIColorGetTableEntryIndex(UIColorTableEntries which)
      uint8_t which = ARG8;
      IndexedColorType c = UIColorGetTableEntryIndex(which);
      debug(DEBUG_TRACE, "EmuPalmOS", "UIColorGetTableEntryIndex(%d): %d", which, c);
      m68k_set_reg(M68K_REG_D0, c);
    }
    break;
    case sysTrapUIPickColor: {
      // Boolean UIPickColor(IndexedColorType *indexP, RGBColorType *rgbP, UIPickColorStartType start, const Char *titleP, const Char *tipP)
      uint32_t indexP = ARG32;
      uint32_t rgbP = ARG32;
      uint16_t start = ARG16;
      uint32_t titleP = ARG32;
      uint32_t tipP = ARG32;
      emupalmos_trap_in(indexP, trap, 0);
      emupalmos_trap_in(rgbP, trap, 1);
      IndexedColorType index;
      RGBColorType rgb;
      if (indexP) index = m68k_read_memory_8(indexP);
      decode_rgb(rgbP, &rgb);
      char *title = (char *)emupalmos_trap_in(titleP, trap, 3);
      char *tip = (char *)emupalmos_trap_in(tipP, trap, 4);
      Boolean res = UIPickColor(indexP ? &index : NULL, rgbP ? &rgb : NULL, start, title, tip);
      if (indexP) m68k_write_memory_8(indexP, index);
      encode_rgb(rgbP, &rgb);
      debug(DEBUG_TRACE, "EmuPalmOS", "UIPickColor(indexP=0x%08X, rgbP=0x%08X, start=%d, title=%s, tip=%s)", indexP, rgbP, start, title ? title : "", tip ? tip : "");
      m68k_set_reg(M68K_REG_D0, res);
    }
    break;
    case sysTrapUIBrightnessAdjust:
      // void UIBrightnessAdjust(void)
      UIBrightnessAdjust();
      debug(DEBUG_TRACE, "EmuPalmOS", "UIBrightnessAdjust()");
    break;
    case sysTrapUIContrastAdjust:
      // void UIContrastAdjust(void)
      UIContrastAdjust();
      debug(DEBUG_TRACE, "EmuPalmOS", "UIContrastAdjust()");
    break;
  }
}
