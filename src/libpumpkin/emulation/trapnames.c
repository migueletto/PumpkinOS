#include <PalmOS.h>
#include <VFSMgr.h>
#include <HsNavCommon.h>

#include "sys.h"
#include "bytes.h"
#include "pumpkin.h"
#ifdef ARMEMU
#include "armemu.h"
#endif
#include "logtrap.h"
#include "m68k/m68k.h"
#include "m68k/m68kcpu.h"
#include "emupalmosinc.h"
#include "emupalmos.h"
#include "trapnames.h"
#include "endianness.h"
#include "debug.h"

#include "syscalls.c"

static trap_t allTraps[0x10000];

static char *intl_traps[] = {
  "Init",
  "TxtByteAttr",
  "TxtCharAttr",
  "TxtCharXAttr",
  "TxtCharSize",
  "TxtGetPreviousChar",
  "TxtGetNextChar",
  "TxtGetChar",
  "TxtSetNextChar",
  "TxtCharBounds",
  "TxtPrepFindString",
  "TxtFindString",
  "TxtReplaceStr",
  "TxtWordBounds",
  "TxtCharEncoding",
  "TxtStrEncoding",
  "TxtEncodingName",
  "TxtMaxEncoding",
  "TxtTransliterate",
  "TxtCharIsValid",
  "TxtCompare",
  "TxtCaselessCompare",
  "TxtCharWidth",
  "TxtGetTruncationOffset",
  "IntlGetRoutineAddress",
  "IntlHandleEvent",
  "TxtParamString",
  "TxtConvertEncodingV35",
  "TxtConvertEncoding",
  "IntlSetRoutineAddress",
  "TxtGetWordWrapOffset",
  "TxtNameToEncoding",
  "IntlStrictChecks"
};

static char *om_traps[] = {
  "Init",
  "OpenOverlayDatabase",
  "LocaleToOverlayDBName",
  "OverlayDBNameToLocale",
  "GetCurrentLocale",
  "GetIndexedLocale",
  "GetSystemLocale",
  "SetSystemLocale",
  "GetRoutineAddress",
  "GetNextSystemLocale"
};

static char *hd_traps[] = {
  "BmpGetNextBitmapAnyDensity",
  "BmpGetVersion",
  "BmpGetCompressionType",
  "BmpGetDensity",
  "BmpSetDensity",
  "BmpGetTransparentValue",
  "BmpSetTransparentValue",
  "BmpCreateBitmapV3",
  "WinSetCoordinateSystem",
  "WinGetCoordinateSystem",
  "WinScalePoint",
  "WinUnscalePoint",
  "WinScaleRectangle",
  "WinUnscaleRectangle",
  "WinScreenGetAttribute",
  "WinPaintTiledBitmap",
  "WinGetSupportedDensity",
  "EvtGetPenNative",
  "WinScaleCoord",
  "WinUnscaleCoord",
  "WinPaintRoundedRectangleFrame",
  "WinSetScalingMode",
  "WinGetScalingMode"
};

static char *file_traps[] = {
  "Init",
  "CustomControl",
  "FileCreate",
  "FileOpen",
  "FileClose",
  "FileReadData",
  "FileRead",
  "FileWrite",
  "FileDelete",
  "FileRename",
  "FileSeek",
  "FileEOF",
  "FileTell",
  "FileResize",
  "FileGetAttributes",
  "FileSetAttributes",
  "FileGetDate",
  "FileSetDate",
  "FileSize",
  "DirCreate",
  "DirEntryEnumerate",
  "GetDefaultDirectory",
  "RegisterDefaultDirectory",
  "UnregisterDefaultDirectory",
  "VolumeFormat",
  "VolumeMount",
  "VolumeUnmount",
  "VolumeEnumerate",
  "VolumeInfo",
  "VolumeGetLabel",
  "VolumeSetLabel",
  "VolumeSize",
  "InstallFSLib",
  "RemoveFSLib",
  "ImportDatabaseFromFile",
  "ExportDatabaseToFile",
  "FileDBGetResource",
  "FileDBInfo",
  "FileDBGetRecord",
  "ImportDatabaseFromFileCustom",
  "ExportDatabaseToFileCustom",
  "Private1"
};

static char *tsm_traps[] = {
  "GetFepMode",
  "SetFepMode",
  "HandleEvent",
  "Init",
  "DrawMode",
  "GetSystemFep",
  "SetSystemFep",
  "GetCurrentFep",
  "SetCurrentFep",
  "GetSystemFepCreator",
  "SetSystemFepCreator",
  "GetCurrentFepCreator",
  "SetCurrentFepCreator",
  "FepHandleEvent",
  "FepMapEvent",
  "FepTerminate",
  "FepReset",
  "FepCommitAction",
  "FepOptionsList"
};

static char *nav_traps[] = {
  "FrmCountObjectsInNavOrder",
  "FrmGetNavOrder",
  "FrmSetNavOrder",
  "FrmGetNavEntry",
  "FrmSetNavEntry",
  "FrmGetNavState",
  "FrmSetNavState",
  "FrmNavDrawFocusRing",
  "FrmNavRemoveFocusRing",
  "FrmNavGetFocusRingInfo",
  "FrmNavObjectTakeFocus"
};

void allTrapsInit(void) {
  int i, j;

  for (i = 0; i < 0x10000; i++) {
    allTraps[i].trap = 0;
    allTraps[i].name = NULL;
    allTraps[i].rType = NULL;
    allTraps[i].nArgs = 0;
  }

  for (i = 0; traps[i].name; i++) {
    allTraps[traps[i].trap].trap = traps[i].trap;
    allTraps[traps[i].trap].name = traps[i].name;
    allTraps[traps[i].trap].rType = traps[i].rType;
    allTraps[traps[i].trap].nArgs = traps[i].nArgs;
    for (j = 0; j < 16; j++) {
      allTraps[traps[i].trap].arg[j-1] = traps[i].arg[j-1];
    }
  }
}

static void getp(uint32_t addr, char *a, char *buf) {
  int8_t b;
  uint8_t B;
  int16_t w;
  uint16_t W;
  int32_t l;
  uint32_t L;
  char st[8];

  if (addr) {
    if (!sys_strcmp(a, "Wp")) {
      W = m68k_read_memory_16(addr);
      sys_sprintf(buf, "W[%u]", W);
    } else if (!sys_strcmp(a, "wp")) {
      w = m68k_read_memory_16(addr);
      sys_sprintf(buf, "w[%d]", w);
    } else if (!sys_strcmp(a, "Bp")) {
      B = m68k_read_memory_8(addr);
      sys_sprintf(buf, "B[%u]", B);
    } else if (!sys_strcmp(a, "bp")) {
      b = m68k_read_memory_8(addr);
      sys_sprintf(buf, "b[%d]", b);
    } else if (!sys_strcmp(a, "Lp")) {
      L = m68k_read_memory_32(addr);
      sys_sprintf(buf, "L[%u]", L);
    } else if (!sys_strcmp(a, "lp")) {
      l = m68k_read_memory_32(addr);
      sys_sprintf(buf, "l[%d]", l);
    } else if (!sys_strcmp(a, "4p")) {
      L = m68k_read_memory_32(addr);
      pumpkin_id2s(L, st);
      sys_sprintf(buf, "L['%s']", st);
    } else if (!sys_strcmp(a, "frmp")) {
      W = m68k_read_memory_16(addr+104);
      W = sys_htobe16(W);
      sys_sprintf(buf, "Form[%u]", W);
    } else if (!sys_strcmp(a, "fobjp")) {
      sys_sprintf(buf, "FormObj");
    } else if (!sys_strcmp(a, "ctlp")) {
      W = m68k_read_memory_16(addr);
      W = sys_htobe16(W);
      sys_sprintf(buf, "Control[%u]", W);
    } else if (!sys_strcmp(a, "lstp")) {
      W = m68k_read_memory_16(addr);
      W = sys_htobe16(W);
      sys_sprintf(buf, "List[%u]", W);
    } else if (!sys_strcmp(a, "fldp")) {
      W = m68k_read_memory_16(addr);
      W = sys_htobe16(W);
      sys_sprintf(buf, "Field[%u]", W);
    } else if (!sys_strcmp(a, "tblp")) {
      W = m68k_read_memory_16(addr);
      W = sys_htobe16(W);
      sys_sprintf(buf, "Table[%u]", W);
    } else if (!sys_strcmp(a, "sclp")) {
      W = m68k_read_memory_16(addr+8);
      W = sys_htobe16(W);
      sys_sprintf(buf, "ScrollBar[%u]", W);
    } else if (!sys_strcmp(a, "gadp")) {
      W = m68k_read_memory_16(addr);
      W = sys_htobe16(W);
      sys_sprintf(buf, "Gadget[%u]", W);
    } else if (!sys_strcmp(a, "evtp")) {
      char *name;
      W = m68k_read_memory_16(addr);
      name = EvtGetEventName(W);
      if (name) {
        sys_sprintf(buf, "Event[%s]", name);
      } else {
        sys_sprintf(buf, "Event[%u]", W);
      }
    } else if (!sys_strcmp(a, "rctp")) {
      int16_t x, y, width, height;
      x = m68k_read_memory_16(addr);
      y = m68k_read_memory_16(addr+2);
      width = m68k_read_memory_16(addr+4);
      height = m68k_read_memory_16(addr+6);
      sys_sprintf(buf, "Rect[%d,%d,%d,%d]", x, y, width, height);
    } else if (!sys_strcmp(a, "dttp")) {
      int16_t second, minute, hour, day, month, year;
      second = m68k_read_memory_16(addr);
      minute = m68k_read_memory_16(addr+2);
      hour = m68k_read_memory_16(addr+4);
      day = m68k_read_memory_16(addr+6);
      month = m68k_read_memory_16(addr+8);
      year = m68k_read_memory_16(addr+10);
      sys_sprintf(buf, "DateTime[%04d-%02d-%02d_%02d:%02d:%02d]", year, month, day, hour, minute, second);
    } else if (!sys_strcmp(a, "bmpp")) {
      uint16_t width, height, version;
      width = m68k_read_memory_16(addr);
      height = m68k_read_memory_16(addr+2);
      version = m68k_read_memory_16(addr+8) & 0xFF;
      sys_sprintf(buf, "Bitmap[V%d_%dx%d]", version, width, height);
    } else if (!sys_strcmp(a, "winp")) {
      int16_t width, height;
      width = m68k_read_memory_16(addr);
      height = m68k_read_memory_16(addr+2);
      sys_sprintf(buf, "Window[%dx%d]", width, height);
    } else if (!sys_strcmp(a, "lid")) {
      sys_sprintf(buf, "LocalID");
    } else if (!sys_strcmp(a, "lidp")) {
      sys_sprintf(buf, "LocalID[]");
    } else {
      sys_sprintf(buf, "0x%08X", addr);
    }
  } else {
    sys_strcpy(buf, "NULL");
  }
}

static uint32_t getarg(char *a, uint32_t sp, uint16_t idx, char *buf, int max, int returning) {
  int8_t b;
  uint8_t B;
  int16_t w;
  uint16_t W;
  int32_t l;
  uint32_t L;
  char st[8];
  int input_param;

  if (a[0] == 'o') {
    input_param = 0;
    a++;
  } else {
    input_param = 1;
  }

  if (a[1] == 0) switch (a[0]) {
    case 'c':
    case 'b':
      b = ARG8;
      sys_sprintf(buf, "%d", b);
      break;
    case 'B':
      B = ARG8;
      sys_sprintf(buf, "%u", B);
      break;
    case 'w':
      w = ARG16;
      sys_sprintf(buf, "%d", w);
      break;
    case 'W':
      W = ARG16;
      sys_sprintf(buf, "%u", W);
      break;
    case 'l':
      l = ARG32;
      sys_sprintf(buf, "%d", l);
      break;
    case 'L':
      L = ARG32;
      sys_sprintf(buf, "%u", L);
      break;
    case 'p':
      L = ARG32;
      sys_sprintf(buf, "0x%08X", L);
      break;
    case '4':
      L = ARG32;
      pumpkin_id2s(L, st);
      sys_sprintf(buf, "'%s'", st);
      break;
    case 'S': {
      int i = 0, j = 0;
      L = ARG32;
      if (L) {
        buf[i++] = '"';
        for (;;) {
          b = m68k_read_memory_8(L+j);
          j++;
          if (b == 0) break;
          buf[i++] = b;
          if (i == max - 2) break;
        }
        buf[i++] = '"';
        buf[i] = 0;
      } else {
        sys_strcpy(buf, "NULL");
      }
      }
      break;
  } else {
    L = ARG32;
    if (returning || input_param) {
      getp(L, a, buf);
    } else {
      sys_strcpy(buf, L ? "addr" : "NULL");
    }
  }

  return idx;
}

#define doarg(n) \
  if (allTraps[trap].nArgs > n-1) { \
    idx = getarg(allTraps[trap].arg[n-1], sp, idx, buf, sizeof(buf)-1, returning); \
    if (sys_strlen(line) + 1 + sys_strlen(buf) + 1 < sizeof(line)) { \
      sys_strcat(line, " "); \
      sys_strcat(line, buf); \
    } \
  }

#define doargs() \
  line[0] = 0; \
  doarg(1); \
  doarg(2); \
  doarg(3); \
  doarg(4); \
  doarg(5); \
  doarg(6); \
  doarg(7); \
  doarg(8); \
  doarg(9); \
  doarg(10); \
  doarg(11); \
  doarg(12); \
  doarg(13); \
  doarg(14); \
  doarg(15); \
  doarg(16);

static int getret(char *rType, char *buf) {
  int8_t b;
  int16_t w;
  int32_t l;
  uint32_t d;
  int ret = 1;

  switch (rType[0]) {
    case '?':
      sys_strcpy(buf, "unknown");
      break;
    case 'v':
      ret = 0;
      break;
    case 'p':
      d = m68k_get_reg(NULL, M68K_REG_A0);
      sys_sprintf(buf, "0x%08X", d);
      break;
    case 'c':
    case 'b':
      d = m68k_get_reg(NULL, M68K_REG_D0) & 0xFF;
      b = d;
      sys_sprintf(buf, "%d", b);
      break;
    case 'w':
      d = m68k_get_reg(NULL, M68K_REG_D0) & 0xFFFF;
      w = d;
      sys_sprintf(buf, "%d", w);
      break;
    case 'l':
      d = m68k_get_reg(NULL, M68K_REG_D0);
      l = d;
      sys_sprintf(buf, "%d", l);
      break;
    case 'B':
      d = m68k_get_reg(NULL, M68K_REG_D0) & 0xFF;
      sys_sprintf(buf, "%u", d);
      break;
    case 'W':
      d = m68k_get_reg(NULL, M68K_REG_D0) & 0xFFFF;
      sys_sprintf(buf, "%u", d);
      break;
    case 'L':
      d = m68k_get_reg(NULL, M68K_REG_D0);
      sys_sprintf(buf, "%u", d);
      break;
  }

  return ret;
}

static char *spaces(uint32_t n) {
  static char buf[256];
  uint32_t i;

  for (i = 0; i < n && i < 256; i++) {
    buf[i] = ' ';
  }
  buf[i] = 0;

  return buf;
}

void trapHook(uint32_t pc, emu_state_t *state) {
  uint16_t trap, idx, instruction, selector;
  uint32_t sp, d;
  char line[512];
  char buf[128];
  char *name;
  int returning, ret;

  if (state->ldef && state->lt) state->ldef->rethook(state->lt, pc);

  if (state->stackp && pc == state->stack[state->stackp-1]) {
    state->stackp--;
    trap = state->stackt[state->stackp];

    if (allTraps[trap].name) {
      name = trapName(trap, &selector, 1);
      sp = m68k_get_reg(NULL, M68K_REG_SP);
      idx = 0;
      returning = 1;
      sys_memset(line, 0, sizeof(line));
      doargs();
      if (allTraps[trap].rType[1]) {
        d = m68k_get_reg(NULL, M68K_REG_A0);
        getp(d, allTraps[trap].rType, buf);
        ret = 1;
      } else {
        ret = getret(allTraps[trap].rType, buf);
      }
      if (ret) {
        debug(DEBUG_TRACE, "Trap", "[%d] %sreturn %s%s : %s", state->stackp, spaces(state->stackp), name, line, buf);
      } else {
        debug(DEBUG_TRACE, "Trap", "[%d] %sreturn %s%s", state->stackp, spaces(state->stackp), name, line);
      }
    } else {
      debug(DEBUG_TRACE, "Trap", "[%d] %sreturn unknown 0x%04X", state->stackp, spaces(state->stackp), trap);
    }
  }

  instruction = m68k_read_memory_16(pc);

  if (instruction == 0x4E4F) {
    trap = m68k_read_memory_16(pc + 2);
    if (state->ldef && state->lt) state->ldef->hook(state->lt, pc, trap);

    if (debug_getsyslevel("Trap") == DEBUG_TRACE /*&&
        (state->stackp == 0 ||
         state->stackt[state->stackp-1] == sysTrapSysAppLaunch ||
         state->stackt[state->stackp-1] == sysTrapFrmDispatchEvent ||
         state->stackt[state->stackp-1] == sysTrapFrmHandleEvent)*/) {

      sp = m68k_get_reg(NULL, M68K_REG_SP);
      idx = 0;
      state->stackt[state->stackp] = trap;
      state->stack[state->stackp++] = pc+4;

      if (allTraps[trap].name) {
        name = trapName(trap, &selector, 1);
        returning = 0;
        doargs();
        debug(DEBUG_TRACE, "Trap", "[%d] %s%s%s", state->stackp-1, spaces(state->stackp-1), name, line);
      } else {
        debug(DEBUG_TRACE, "Trap", "[%d] %sunknown 0x%04X", state->stackp-1, spaces(state->stackp-1), trap);
      }
    }
  }
}

char *trapName(uint16_t trap, uint16_t *selector, int follow) {
  char *name = "unknown";
  uint32_t d2, sp;
  uint16_t aux;

  *selector = 0xFFFF;

  switch (trap) {
    case sysTrapIntlDispatch:
      d2 = m68k_get_reg(NULL, M68K_REG_D2);
      if (d2 < intlMaxSelector) {
        name = follow ? intl_traps[d2] : allTraps[trap].name;
        *selector = d2;
      }
      break;
    case sysTrapOmDispatch:
      d2 = m68k_get_reg(NULL, M68K_REG_D2);
      if (d2 < omMaxSelector) {
        name = follow ? om_traps[d2] : allTraps[trap].name;
        *selector = d2;
      }
      break;
    case sysTrapHighDensityDispatch:
      d2 = m68k_get_reg(NULL, M68K_REG_D2);
      if (d2 < HDSelectorInvalid) {
        name = follow ? hd_traps[d2] : allTraps[trap].name;
        *selector = d2;
      }
      break;
    case sysTrapFileSystemDispatch:
      d2 = m68k_get_reg(NULL, M68K_REG_D2);
      if (d2 < vfsMaxSelector) {
        name = follow ? file_traps[d2] : allTraps[trap].name;
        *selector = d2;
      }
      break;
    case sysTrapTsmDispatch:
      d2 = m68k_get_reg(NULL, M68K_REG_D2);
      if (d2 < tsmMaxSelector) {
        name = follow ? tsm_traps[d2] : allTraps[trap].name;
        *selector = d2;
      }
      break;
    case sysTrapNavSelector:
      sp = m68k_get_reg(NULL, M68K_REG_SP);
      aux = m68k_read_memory_16(sp);
      if (aux <= NavSelectorFrmNavObjectTakeFocus) {
        name = follow ? nav_traps[aux] : allTraps[trap].name;
        *selector = aux;
      }
      break;
    default:
      if (allTraps[trap].name) name = allTraps[trap].name;
      break;
  }

  return name;
}
