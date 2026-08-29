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

void palmos_StrSysTrap(uint32_t sp, uint16_t idx, uint32_t trap) {

  switch (trap) {
    case sysTrapStrVPrintF:
    case sysTrapStrPrintF: {
      // Int16 StrVPrintF(Char *s, const Char *formatStr, _Palm_va_list arg)
      // Int16 StrPrintF(Char *s, const Char *formatStr, ...)
      uint32_t str = ARG32;
      uint32_t formatStr = ARG32;
      char *s = emupalmos_trap_in(str, trap, 0);
      char *f = emupalmos_trap_in(formatStr, trap, 1);
      int vararg = trap == sysTrapStrVPrintF;
      Int16 res = 0;
      if (s && f) {
        int i, j = 0, k = 1, t = 0, sz = 0, arglen = 0;
        uint32_t arg, v_arg;
        char *p, *q, fmt[16];
        if (vararg) {
          v_arg = ARG32;
        } else {
          v_arg = 0;
        }
        for (i = 0, p = s; f[i]; i++) {
          switch (t) {
            case 0:
              if (f[i] == '%') {
                j = 0;
                fmt[j++] = f[i];
                arglen = -1;
                sz = 2;
                t = 1;
              } else {
                *p++ = f[i];
              }
              break;
            case 1:
              switch (f[i]) {
                case 'h':
                case 'H':
                  fmt[j++] = f[i];
                  sz = 2;
                  break;
                case 'l':
                case 'L':
                  fmt[j++] = f[i];
                  sz = 4;
                  break;
                case 'd':
                case 'i':
                case 'u':
                case 'x':
                case 'X':
                  if (vararg) {
                    switch (sz) {
                      case 1:  arg = m68k_read_memory_16(v_arg) & 0xff; v_arg += 2; break;
                      case 2:  arg = m68k_read_memory_16(v_arg); v_arg += 2; break;
                      case 4:  arg = m68k_read_memory_32(v_arg); v_arg += 4; break;
                      default: arg = m68k_read_memory_16(v_arg); v_arg += 2; break;
                    }
                  } else {
                    switch (sz) {
                      case 1:  arg = ARG8;  break;
                      case 2:  arg = ARG16; break;
                      case 4:  arg = ARG32; break;
                      default: arg = ARG16; break;
                    }
                  }
                  k++;
                  fmt[j++] = f[i];
                  fmt[j] = 0;
                  sys_sprintf(p, fmt, arg);
                  p += sys_strlen(p);
                  t = 0;
                  break;
                case 'c':
                case 'C':
                  if (vararg) {
                    arg = m68k_read_memory_16(v_arg) & 0xff;
                    v_arg += 2;
                  } else {
                    arg = ARG8;
                  }
                  k++;
                  fmt[j++] = f[i];
                  fmt[j] = 0;
                  sys_sprintf(p, fmt, arg);
                  p += sys_strlen(p);
                  t = 0;
                  break;
                case 's':
                  if (vararg) {
                    arg = m68k_read_memory_32(v_arg);
                    v_arg += 4;
                  } else {
                    arg = ARG32;
                  }
                  k++;
                  q = emupalmos_trap_in(arg, trap, k);
                  fmt[j++] = f[i];
                  fmt[j] = 0;
                  if (arglen < 0) {
                    sys_sprintf(p, fmt, q);
                  } else {
                    sys_sprintf(p, fmt, arglen, q);
                  }
                  p += sys_strlen(p);
                  t = 0;
                  break;
                case '*':
                  if (vararg) {
                    arglen = m68k_read_memory_16(v_arg);
                    v_arg += 2;
                  } else {
                    arglen = ARG16;
                  }
                  k++;
                  break;
                case '%':
                  *p++ = f[i];
                  t = 0;
                  break;
                default:
                  fmt[j++] = f[i];
                  break;
              }
              break;
          }
        }
        *p = 0;
        res = sys_strlen(s);
      }
      debug(DEBUG_TRACE, "EmuPalmOS", "StrPrintF(0x%08X \"%s\", 0x%08X \"%s\", ...): %d", str, s ? s : "", formatStr, f ? f : "", res);
      m68k_set_reg(M68K_REG_D0, res);
    }
    break;
    case sysTrapStrCopy: {
      // Char *StrCopy(out Char *dst, in Char *src)
      uint32_t dst = ARG32;
      char *s_dst = emupalmos_trap_in(dst, trap, 0);
      uint32_t src = ARG32;
      char *s_src = emupalmos_trap_in(src, trap, 1);
      Char *res = NULL;
      if (s_dst && s_src) {
        uint32_t len = sys_strlen(s_src)+1;
        if (emupalmos_check_address(dst, len, 0)) {
          debug(DEBUG_TRACE, "logmem", "write %u %u", dst, len);
          res = StrCopy(s_dst, s_src);
        }
      }
      uint32_t r_res = emupalmos_trap_out(res);
      m68k_set_reg(M68K_REG_A0, r_res);
      debug(DEBUG_TRACE, "EmuPalmOS", "StrCopy(dst=0x%08X [%s], src=0x%08X [%s]): 0x%08X", dst, s_dst, src, s_src, r_res);
    }
    break;
    case sysTrapStrNCopy: {
      // Char *StrNCopy(out Char *dst, in Char *src, Int16 n)
      uint32_t dst = ARG32;
      char *s_dst = emupalmos_trap_in(dst, trap, 0);
      uint32_t src = ARG32;
      char *s_src = emupalmos_trap_in(src, trap, 1);
      int16_t n = ARG16;
      Char *res = NULL;
      if (s_dst && s_src) {
        if (emupalmos_check_address(dst, n, 0)) {
          debug(DEBUG_TRACE, "logmem", "write %u %u", dst, n);
          res = StrNCopy(s_dst, s_src, n);
        }
      }
      uint32_t r_res = emupalmos_trap_out(res);
      m68k_set_reg(M68K_REG_A0, r_res);
      debug(DEBUG_TRACE, "EmuPalmOS", "StrNCopy(dst=0x%08X [%s], src=0x%08X [%s], n=%d): 0x%08X", dst, s_dst, src, s_src, n, r_res);
    }
    break;
    case sysTrapStrCat: {
      // Char *StrCat(out Char *dst, in Char *src)
      uint32_t dst = ARG32;
      char *s_dst = emupalmos_trap_in(dst, trap, 0);
      uint32_t src = ARG32;
      char *s_src = emupalmos_trap_in(src, trap, 1);
      Char *res = NULL;
      if (s_dst && s_src) {
        uint32_t len = sys_strlen(s_src)+1;
        if (emupalmos_check_address(dst + sys_strlen(s_dst), len, 0)) {
          debug(DEBUG_TRACE, "logmem", "write %u %u", dst, len);
          res = StrCat(s_dst, s_src);
        }
      }
      uint32_t r_res = emupalmos_trap_out(res);
      m68k_set_reg(M68K_REG_A0, r_res);
      debug(DEBUG_TRACE, "EmuPalmOS", "StrCat(dst=0x%08X [%s], src=0x%08X [%s]): 0x%08X", dst, s_dst, src, s_src, r_res);
    }
    break;
    case sysTrapStrNCat: {
      // Char *StrNCat(out Char *dst, in Char *src, Int16 n)
      uint32_t dst = ARG32;
      char *s_dst = emupalmos_trap_in(dst, trap, 0);
      uint32_t src = ARG32;
      char *s_src = emupalmos_trap_in(src, trap, 1);
      int16_t n = ARG16;
      Char *res = NULL;
      if (s_dst && s_src) {
        if (emupalmos_check_address(dst, n, 0)) {
          debug(DEBUG_TRACE, "logmem", "write %u %u", dst, n);
          res = StrNCat(s_dst, s_src, n);
        }
      }
      uint32_t r_res = emupalmos_trap_out(res);
      m68k_set_reg(M68K_REG_A0, r_res);
      debug(DEBUG_TRACE, "EmuPalmOS", "StrNCat(dst=0x%08X [%s], src=0x%08X [%s], n=%d): 0x%08X", dst, s_dst, src, s_src, n, r_res);
    }
    break;
    case sysTrapStrLen: {
      // UInt16 StrLen(in Char *src)
      uint32_t src = ARG32;
      char *s_src = emupalmos_trap_in(src, trap, 0);
      UInt16 res = s_src ? StrLen(s_src) : 0;
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "StrLen(src=0x%08X [%s]): %d", src, s_src, res);
    }
    break;
    case sysTrapStrCompareAscii: {
      // Int16 StrCompareAscii(in Char *s1, in Char *s2)
      uint32_t s1 = ARG32;
      char *s_s1 = emupalmos_trap_in(s1, trap, 0);
      uint32_t s2 = ARG32;
      char *s_s2 = emupalmos_trap_in(s2, trap, 1);
      Int16 res = s_s1 && s_s2 ? StrCompareAscii(s_s1, s_s2) : 0;
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "StrCompareAscii(s1=0x%08X [%s], s2=0x%08X [%s]): %d", s1, s_s1, s2, s_s2, res);
    }
    break;
    case sysTrapStrCompare: {
      // Int16 StrCompare(in Char *s1, in Char *s2)
      uint32_t s1 = ARG32;
      char *s_s1 = emupalmos_trap_in(s1, trap, 0);
      uint32_t s2 = ARG32;
      char *s_s2 = emupalmos_trap_in(s2, trap, 1);
      Int16 res = s_s1 && s_s2 ? StrCompare(s_s1, s_s2) : 0;
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "StrCompare(s1=0x%08X [%s], s2=0x%08X [%s]): %d", s1, s_s1, s2, s_s2, res);
    }
    break;
    case sysTrapStrNCompareAscii: {
      // Int16 StrNCompareAscii(in Char *s1, in Char *s2, Int32 n)
      uint32_t s1 = ARG32;
      char *s_s1 = emupalmos_trap_in(s1, trap, 0);
      uint32_t s2 = ARG32;
      char *s_s2 = emupalmos_trap_in(s2, trap, 1);
      int32_t n = ARG32;
      Int16 res = s_s1 && s_s2 ? StrNCompareAscii(s_s1, s_s2, n) : 0;
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "StrNCompareAscii(s1=0x%08X [%s], s2=0x%08X [%s], n=%d): %d", s1, s_s1, s2, s_s2, n, res);
    }
    break;
    case sysTrapStrNCompare: {
      // Int16 StrNCompare(in Char *s1, in Char *s2, Int32 n)
      uint32_t s1 = ARG32;
      char *s_s1 = emupalmos_trap_in(s1, trap, 0);
      uint32_t s2 = ARG32;
      char *s_s2 = emupalmos_trap_in(s2, trap, 1);
      int32_t n = ARG32;
      Int16 res = s_s1 && s_s2 ? StrNCompare(s_s1, s_s2, n) : 0;
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "StrNCompare(s1=0x%08X [%s], s2=0x%08X [%s], n=%d): %d", s1, s_s1, s2, s_s2, n, res);
    }
    break;
    case sysTrapStrCaselessCompare: {
      // Int16 StrCaselessCompare(in Char *s1, in Char *s2)
      uint32_t s1 = ARG32;
      char *s_s1 = emupalmos_trap_in(s1, trap, 0);
      uint32_t s2 = ARG32;
      char *s_s2 = emupalmos_trap_in(s2, trap, 1);
      Int16 res = s_s1 && s_s2 ? StrCaselessCompare(s_s1, s_s2) : 0;
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "StrCaselessCompare(s1=0x%08X [%s], s2=0x%08X [%s]): %d", s1, s_s1, s2, s_s2, res);
    }
    break;
    case sysTrapStrNCaselessCompare: {
      // Int16 StrNCaselessCompare(in Char *s1, in Char *s2, Int32 n)
      uint32_t s1 = ARG32;
      char *s_s1 = emupalmos_trap_in(s1, trap, 0);
      uint32_t s2 = ARG32;
      char *s_s2 = emupalmos_trap_in(s2, trap, 1);
      int32_t n = ARG32;
      Int16 res = s_s1 && s_s2 ?StrNCaselessCompare(s_s1, s_s2, n) : 0;
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "StrNCaselessCompare(s1=0x%08X [%s], s2=0x%08X [%s], n=%d): %d", s1, s_s1, s2, s_s2, n, res);
    }
    break;
    case sysTrapStrToLower: {
      // Char *StrToLower(out Char *dst, in Char *src)
      uint32_t dst = ARG32;
      char *s_dst = emupalmos_trap_in(dst, trap, 0);
      uint32_t src = ARG32;
      char *s_src = emupalmos_trap_in(src, trap, 1);
      Char *res = s_dst && s_src ? StrToLower(s_dst, s_src) : NULL;
      uint32_t r_res = emupalmos_trap_out(res);
      m68k_set_reg(M68K_REG_A0, r_res);
      debug(DEBUG_TRACE, "EmuPalmOS", "StrToLower(dst=0x%08X [%s], src=0x%08X [%s]): 0x%08X", dst, s_dst, src, s_src, r_res);
    }
    break;
    case sysTrapStrIToA: {
      // Char *StrIToA(out Char *s, Int32 i)
      uint32_t s = ARG32;
      char *s_s = emupalmos_trap_in(s, trap, 0);
      int32_t i = ARG32;
      Char *res = s_s ? StrIToA(s_s, i) : NULL;
      uint32_t r_res = emupalmos_trap_out(res);
      m68k_set_reg(M68K_REG_A0, r_res);
      debug(DEBUG_TRACE, "EmuPalmOS", "StrIToA(s=0x%08X [%s], i=%d): 0x%08X", s, s_s, i, r_res);
    }
    break;
    case sysTrapStrIToH: {
      // Char *StrIToH(out Char *s, UInt32 i)
      uint32_t s = ARG32;
      char *s_s = emupalmos_trap_in(s, trap, 0);
      uint32_t i = ARG32;
      Char *res = s_s ? StrIToH(s_s, i) : NULL;
      uint32_t r_res = emupalmos_trap_out(res);
      m68k_set_reg(M68K_REG_A0, r_res);
      debug(DEBUG_TRACE, "EmuPalmOS", "StrIToH(s=0x%08X [%s], i=%d): 0x%08X", s, s_s, i, r_res);
    }
    break;
    case sysTrapStrLocalizeNumber: {
      // Char *StrLocalizeNumber(out Char *s, Char thousandSeparator, Char decimalSeparator)
      uint32_t s = ARG32;
      char *s_s = emupalmos_trap_in(s, trap, 0);
      int8_t thousandSeparator = ARG8;
      int8_t decimalSeparator = ARG8;
      Char *res = s_s ? StrLocalizeNumber(s_s, thousandSeparator, decimalSeparator) : NULL;
      uint32_t r_res = emupalmos_trap_out(res);
      m68k_set_reg(M68K_REG_A0, r_res);
      debug(DEBUG_TRACE, "EmuPalmOS", "StrLocalizeNumber(s=0x%08X [%s], thousandSeparator=%d, decimalSeparator=%d): 0x%08X", s, s_s, thousandSeparator, decimalSeparator, r_res);
    }
    break;
    case sysTrapStrDelocalizeNumber: {
      // Char *StrDelocalizeNumber(out Char *s, Char thousandSeparator, Char decimalSeparator)
      uint32_t s = ARG32;
      char *s_s = emupalmos_trap_in(s, trap, 0);
      int8_t thousandSeparator = ARG8;
      int8_t decimalSeparator = ARG8;
      Char *res = s_s ? StrDelocalizeNumber(s_s, thousandSeparator, decimalSeparator) : NULL;
      uint32_t r_res = emupalmos_trap_out(res);
      m68k_set_reg(M68K_REG_A0, r_res);
      debug(DEBUG_TRACE, "EmuPalmOS", "StrDelocalizeNumber(s=0x%08X [%s], thousandSeparator=%d, decimalSeparator=%d): 0x%08X", s, s_s, thousandSeparator, decimalSeparator, r_res);
    }
    break;
    case sysTrapStrChr: {
      // Char *StrChr(in Char *str, WChar chr)
      uint32_t str = ARG32;
      char *s_str = emupalmos_trap_in(str, trap, 0);
      uint16_t chr = ARG16;
      Char *res = s_str ? StrChr(s_str, chr) : NULL;
      uint32_t r_res = emupalmos_trap_out(res);
      m68k_set_reg(M68K_REG_A0, r_res);
      debug(DEBUG_TRACE, "EmuPalmOS", "StrChr(str=0x%08X [%s], chr=%d): 0x%08X", str, s_str, chr, r_res);
    }
    break;
    case sysTrapStrStr: {
      // Char *StrStr(in Char *str, in Char *token)
      uint32_t str = ARG32;
      char *s_str = emupalmos_trap_in(str, trap, 0);
      uint32_t token = ARG32;
      char *s_token = emupalmos_trap_in(token, trap, 1);
      Char *res = s_str && s_token ? StrStr(s_str, s_token) : NULL;
      uint32_t r_res = emupalmos_trap_out(res);
      m68k_set_reg(M68K_REG_A0, r_res);
      debug(DEBUG_TRACE, "EmuPalmOS", "StrStr(str=0x%08X [%s], token=0x%08X [%s]): 0x%08X", str, s_str, token, s_token, r_res);
    }
    break;
    case sysTrapStrAToI: {
      // Int32 StrAToI(in Char *str)
      uint32_t str = ARG32;
      char *s_str = emupalmos_trap_in(str, trap, 0);
      Int32 res = s_str ? StrAToI(s_str) : 0;
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "StrAToI(str=0x%08X [%s]): %d", str, s_str, res);
    }
    break;
  }
}
