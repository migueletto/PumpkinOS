#include <PalmOS.h>
#include <VFSMgr.h>
#include <DLServer.h>
#include <Helper.h>
#include <CharAttr.h>
#include <HsNavCommon.h>

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

void palmos_FntSysTrap(uint32_t sp, uint16_t idx, uint32_t trap) {

  switch (trap) {
    case sysTrapFntDefineFont: {
      // Err FntDefineFont(FontID font, FontPtr fontP)
      uint8_t font = ARG8;
      uint32_t fontP = ARG32;
      FontPtr fontp = (FontPtr)emupalmos_trap_in(fontP, trap, 1);
      Err err = FntDefineFont(font, fontp);
      debug(DEBUG_TRACE, "EmuPalmOS", "FntDefineFont(%d, 0x%08X): %d", font, fontP, err);
      m68k_set_reg(M68K_REG_D0, err);
    }
    break;
    case sysTrapFntGetFont: {
      // FontID FntGetFont(void)
      FontID res = FntGetFont();
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "FntGetFont(): %d", res);
    }
    break;
    case sysTrapFntSetFont: {
      // FontID FntSetFont(FontID font)
      uint8_t font = ARG8;
      FontID res = FntSetFont(font);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "FntSetFont(font=%d): %d", font, res);
    }
    break;
    case sysTrapFntGetFontPtr: {
      // FontType *FntGetFontPtr(void)
      FontType *res = FntGetFontPtr();
      uint32_t r_res = emupalmos_trap_out(res);
      m68k_set_reg(M68K_REG_A0, r_res);
      debug(DEBUG_TRACE, "EmuPalmOS", "FntGetFontPtr(): 0x%08X", r_res);
    }
    break;
    case sysTrapFntBaseLine: {
      // Int16 FntBaseLine(void)
      Int16 res = FntBaseLine();
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "FntBaseLine(): %d", res);
    }
    break;
    case sysTrapFntCharHeight: {
      // Int16 FntCharHeight(void)
      Int16 res = FntCharHeight();
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "FntCharHeight(): %d", res);
    }
    break;
    case sysTrapFntLineHeight: {
      // Int16 FntLineHeight(void)
      Int16 res = FntLineHeight();
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "FntLineHeight(): %d", res);
    }
    break;
    case sysTrapFntAverageCharWidth: {
      // Int16 FntAverageCharWidth(void)
      Int16 res = FntAverageCharWidth();
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "FntAverageCharWidth(): %d", res);
    }
    break;
    case sysTrapFntCharWidth: {
      // Int16 FntCharWidth(Char ch)
      int8_t ch = ARG8;
      Int16 res = FntCharWidth(ch);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "FntCharWidth(ch=%d): %d", ch, res);
    }
    break;
    case sysTrapFntWCharWidth: {
      // Int16 FntWCharWidth(WChar iChar)
      uint16_t iChar = ARG16;
      Int16 res = FntWCharWidth(iChar);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "FntWCharWidth(iChar=%d): %d", iChar, res);
    }
    break;
    case sysTrapFntCharsWidth: {
      // Int16 FntCharsWidth(in Char *chars, Int16 len)
      uint32_t chars = ARG32;
      char *s_chars = emupalmos_trap_in(chars, trap, 0);
      int16_t len = ARG16;
      Int16 res = s_chars ? FntCharsWidth(s_chars, len) : 0;
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "FntCharsWidth(chars=0x%08X [%.*s], len=%d): %d", chars, len, s_chars, len, res);
    }
    break;
    case sysTrapFntWidthToOffset: {
      // Int16 FntWidthToOffset(in Char *pChars, UInt16 length, Int16 pixelWidth, out Boolean *leadingEdge, out Int16 *truncWidth)
      uint32_t pChars = ARG32;
      char *s_pChars = emupalmos_trap_in(pChars, trap, 0);
      uint16_t length = ARG16;
      int16_t pixelWidth = ARG16;
      uint32_t leadingEdge = ARG32;
      Boolean l_leadingEdge = false;
      uint32_t truncWidth = ARG32;
      Int16 l_truncWidth = 0;
      Int16 res = s_pChars ? FntWidthToOffset(s_pChars, length, pixelWidth, leadingEdge ? &l_leadingEdge : NULL, truncWidth ? &l_truncWidth : NULL) : 0;
      if (leadingEdge) m68k_write_memory_8(leadingEdge, l_leadingEdge);
      if (truncWidth) m68k_write_memory_16(truncWidth, l_truncWidth);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "FntWidthToOffset(pChars=0x%08X [%s], length=%d, pixelWidth=%d, leadingEdge=0x%08X, truncWidth=0x%08X [%d]): %d", pChars, s_pChars, length, pixelWidth, leadingEdge, truncWidth, l_truncWidth, res);
    }
    break;
    case sysTrapFntCharsInWidth: {
      // void FntCharsInWidth(in Char *string, inout Int16 *stringWidthP, inout Int16 *stringLengthP, out Boolean *fitWithinWidth)
      uint32_t string = ARG32;
      char *s_string = emupalmos_trap_in(string, trap, 0);
      uint32_t stringWidthP = ARG32;
      Int16 l_stringWidthP = 0;
      if (stringWidthP) l_stringWidthP = m68k_read_memory_16(stringWidthP);
      uint32_t stringLengthP = ARG32;
      Int16 l_stringLengthP = 0;
      if (stringLengthP) l_stringLengthP = m68k_read_memory_16(stringLengthP);
      uint32_t fitWithinWidth = ARG32;
      Boolean l_fitWithinWidth = 0;
      if (s_string) FntCharsInWidth(s_string, stringWidthP ? &l_stringWidthP : NULL, stringLengthP ? &l_stringLengthP : NULL, fitWithinWidth ? &l_fitWithinWidth : NULL);
      if (stringWidthP) m68k_write_memory_16(stringWidthP, l_stringWidthP);
      if (stringLengthP) m68k_write_memory_16(stringLengthP, l_stringLengthP);
      if (fitWithinWidth) m68k_write_memory_8(fitWithinWidth, l_fitWithinWidth);
      debug(DEBUG_TRACE, "EmuPalmOS", "FntCharsInWidth(string=0x%08X [%s], stringWidthP=0x%08X [%d], stringLengthP=0x%08X [%d], fitWithinWidth=0x%08X [%d])", string, s_string, stringWidthP, l_stringWidthP, stringLengthP, l_stringLengthP, fitWithinWidth, l_fitWithinWidth);
    }
    break;
    case sysTrapFntDescenderHeight: {
      // Int16 FntDescenderHeight(void)
      Int16 res = FntDescenderHeight();
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "FntDescenderHeight(): %d", res);
    }
    break;
    case sysTrapFntLineWidth: {
      // Int16 FntLineWidth(in Char *pChars, UInt16 length)
      uint32_t pChars = ARG32;
      char *s_pChars = emupalmos_trap_in(pChars, trap, 0);
      uint16_t length = ARG16;
      Int16 res = s_pChars ? FntLineWidth(s_pChars, length) : 0;
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "FntLineWidth(pChars=0x%08X [%s], length=%d): %d", pChars, s_pChars, length, res);
    }
    break;
    case sysTrapFntWordWrap: {
      // UInt16 FntWordWrap(in Char *chars, UInt16 maxWidth)
      uint32_t chars = ARG32;
      char *s_chars = emupalmos_trap_in(chars, trap, 0);
      uint16_t maxWidth = ARG16;
      UInt16 res = s_chars ? FntWordWrap(s_chars, maxWidth) : 0;
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "FntWordWrap(chars=0x%08X [%s], maxWidth=%d): %d", chars, s_chars, maxWidth, res);
    }
    break;
    case sysTrapFntWordWrapReverseNLines: {
      // void FntWordWrapReverseNLines(in Char *chars, UInt16 maxWidth, inout UInt16 *linesToScrollP, inout UInt16 *scrollPosP)
      uint32_t chars = ARG32;
      char *s_chars = emupalmos_trap_in(chars, trap, 0);
      uint16_t maxWidth = ARG16;
      uint32_t linesToScrollP = ARG32;
      UInt16 l_linesToScrollP = 0;
      if (linesToScrollP) l_linesToScrollP = m68k_read_memory_16(linesToScrollP);
      uint32_t scrollPosP = ARG32;
      UInt16 l_scrollPosP = 0;
      if (scrollPosP) l_scrollPosP = m68k_read_memory_16(scrollPosP);
      if (s_chars) FntWordWrapReverseNLines(s_chars, maxWidth, linesToScrollP ? &l_linesToScrollP : NULL, scrollPosP ? &l_scrollPosP : NULL);
      if (linesToScrollP) m68k_write_memory_16(linesToScrollP, l_linesToScrollP);
      if (scrollPosP) m68k_write_memory_16(scrollPosP, l_scrollPosP);
      debug(DEBUG_TRACE, "EmuPalmOS", "FntWordWrapReverseNLines(chars=0x%08X [%s], maxWidth=%d, linesToScrollP=0x%08X [%d], scrollPosP=0x%08X [%d])", chars, s_chars, maxWidth, linesToScrollP, l_linesToScrollP, scrollPosP, l_scrollPosP);
    }
    break;
    case sysTrapFntGetScrollValues: {
      // void FntGetScrollValues(in Char *chars, UInt16 width, UInt16 scrollPos, out UInt16 *linesP, out UInt16 *topLine)
      uint32_t chars = ARG32;
      char *s_chars = emupalmos_trap_in(chars, trap, 0);
      uint16_t width = ARG16;
      uint16_t scrollPos = ARG16;
      uint32_t linesP = ARG32;
      UInt16 l_linesP = 0;
      uint32_t topLine = ARG32;
      UInt16 l_topLine = 0;
      if (s_chars) FntGetScrollValues(s_chars, width, scrollPos, linesP ? &l_linesP : NULL, topLine ? &l_topLine : NULL);
      if (linesP) m68k_write_memory_16(linesP, l_linesP);
      if (topLine) m68k_write_memory_16(topLine, l_topLine);
      debug(DEBUG_TRACE, "EmuPalmOS", "FntGetScrollValues(chars=0x%08X [%s], width=%d, scrollPos=%d, linesP=0x%08X [%d], topLine=0x%08X [%d])", chars, s_chars, width, scrollPos, linesP, l_linesP, topLine, l_topLine);
    }
    break;
  }
}
