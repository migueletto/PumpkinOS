#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>

#include <PalmOS.h>
#include <VFSMgr.h>

#ifdef ARMEMU
#include "armemu.h"
#endif
#include "pumpkin.h"
#include "m68k.h"
#include "m68kcpu.h"
#include "emupalmos.h"
#include "debug.h"

void palmos_intltrap(uint32_t sp, uint16_t idx, uint32_t sel) {
  char buf[256];

  switch (sel) {
    case intlTxtCharAttr: {
      // UInt16 TxtCharAttr(WChar inChar)
      uint16_t inChar = ARG32;
      UInt16 res = TxtCharAttr(inChar);
      debug(DEBUG_TRACE, "EmuPalmOS", "TxtCharAttr(%d): %d", inChar, res);
      m68k_set_reg(M68K_REG_D0, res);
    }
      break;
    case intlTxtCharSize: {
      // UInt16 TxtCharSize(WChar inChar)
      uint16_t inChar = ARG32;
      UInt16 res = TxtCharSize(inChar);
      debug(DEBUG_TRACE, "EmuPalmOS", "TxtCharSize(%d): %d", inChar, res);
      m68k_set_reg(M68K_REG_D0, res);
    }
      break;
    case intlTxtGetPreviousChar: {
      // UInt16 TxtGetPreviousChar(const Char *inText, UInt32 inOffset, WChar *outChar)
      uint32_t inTextP = ARG32;
      uint16_t inOffset = ARG32;
      uint16_t outCharP = ARG32;
      char *inText = emupalmos_trap_sel_in(inTextP, sysTrapIntlDispatch, sel, 0);
      WChar outChar;
      UInt16 res = TxtGetPreviousChar(inText, inOffset, &outChar);
      debug(DEBUG_TRACE, "EmuPalmOS", "TxtGetPreviousChar(0x%08X \"%s\", %d, 0x%08X): %d", inTextP, inText, inOffset, outCharP, res);
      m68k_set_reg(M68K_REG_D0, res);
    }
      break;
    case intlTxtGetNextChar: {
      // UInt16 TxtGetNextChar(const Char *inText, UInt32 inOffset, WChar *outChar)
      uint32_t inTextP = ARG32;
      uint16_t inOffset = ARG32;
      uint16_t outCharP = ARG32;
      char *inText = emupalmos_trap_sel_in(inTextP, sysTrapIntlDispatch, sel, 0);
      emupalmos_trap_sel_in(outCharP, sysTrapIntlDispatch, sel, 2);
      WChar outChar;
      UInt16 res = TxtGetNextChar(inText, inOffset, &outChar);
      if (outCharP) m68k_write_memory_16(outCharP, outChar);
      debug(DEBUG_TRACE, "EmuPalmOS", "TxtGetNextChar(0x%08X \"%s\", %d, 0x%08X): %d", inTextP, inText, inOffset, outCharP, res);
      m68k_set_reg(M68K_REG_D0, res);
    }
      break;
    case intlTxtParamString: {
      // Char *TxtParamString(const Char *inTemplate, const Char *param0, const Char *param1, const Char *param2, const Char *param3)
      uint32_t inTemplateP = ARG32;
      uint32_t param0P = ARG32;
      uint32_t param1P = ARG32;
      uint32_t param2P = ARG32;
      uint32_t param3P = ARG32;
      char *inTemplate = emupalmos_trap_sel_in(inTemplateP, sysTrapIntlDispatch, sel, 0);
      char *param0 = emupalmos_trap_sel_in(param0P, sysTrapIntlDispatch, sel, 1);
      char *param1 = emupalmos_trap_sel_in(param1P, sysTrapIntlDispatch, sel, 2);
      char *param2 = emupalmos_trap_sel_in(param2P, sysTrapIntlDispatch, sel, 3);
      char *param3 = emupalmos_trap_sel_in(param3P, sysTrapIntlDispatch, sel, 4);
      char *res = TxtParamString(inTemplate, param0, param1, param2, param3);
      uint32_t r = emupalmos_trap_out(res);
      debug(DEBUG_TRACE, "EmuPalmOS", "TxtParamString(0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X): 0x%08X", inTemplateP, param0P, param1P, param2P, param3P, r);
      m68k_set_reg(M68K_REG_A0, r);
    }
      break;
    case intlTxtCharIsValid: {
      // Boolean TxtCharIsValid(WChar inChar)
      uint16_t inChar = ARG16;
      Boolean res = TxtCharIsValid(inChar);
      debug(DEBUG_TRACE, "EmuPalmOS", "TxtCharIsValid(%d): %d", inChar, res);
      m68k_set_reg(M68K_REG_D0, res);
      }
      break;
    //case intlIntlInit:
    //case intlTxtByteAttr:
    //case intlTxtCharXAttr:
    //case intlTxtGetChar:
    //case intlTxtSetNextChar:
    //case intlTxtCharBounds:
    //case intlTxtPrepFindString:
    //case intlTxtFindString:
    //case intlTxtReplaceStr:
    //case intlTxtWordBounds:
    //case intlTxtCharEncoding:
    //case intlTxtStrEncoding:
    //case intlTxtEncodingName:
    //case intlTxtMaxEncoding:
    //case intlTxtTransliterate:
    //case intlTxtCompare:
    //case intlTxtCaselessCompare:
    //case intlTxtCharWidth:
    //case intlTxtGetTruncationOffset:
    //case intlIntlGetRoutineAddress:
    //case intlIntlHandleEvent:
    //case intlTxtConvertEncodingV35:
    //case intlTxtConvertEncoding:
    //case intlIntlSetRoutineAddress:
    //case intlTxtGetWordWrapOffset:
    //case intlTxtNameToEncoding:
    //case intlIntlStrictChecks:
    default:
      snprintf(buf, sizeof(buf)-1, "IntlDispatch selector %d not mapped", sel);
      emupalmos_panic(buf, EMUPALMOS_INVALID_TRAP);
      break;
  }
}
