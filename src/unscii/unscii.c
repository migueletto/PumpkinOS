#include <PalmOS.h>

#include "language.h"
#include "debug.h"

#define BASE8  0x3000
#define BASE16 0x4000

static const uint16_t pages[] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x15,
  0x16, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x25,
  0x26, 0x27, 0x28, 0x29, 0x2b, 0x2e, 0x4d, 0xe0,
  0xe1, 0xe8, 0xec, 0xfe, 0xff, 0xffff
};

typedef struct {
  MemHandle h8[256];
  MemHandle h16[256];
  FontTypeV2 *f8[256];
  FontTypeV2 *f16[256];
} unscii_t;

static UInt32 nextChar(UInt8 *s, UInt32 i, UInt32 len, UInt32 *w, void *data) {
  UInt32 n, c1, c2, c3, c4;

  c1 = s[i];
  c2 = len > 1 && i < len-1 ? s[i+1] : 0;
  c3 = len > 2 && i < len-2 ? s[i+2] : 0;
  c4 = len > 3 && i < len-3 ? s[i+3] : 0;

  if (!(c1 & 0x80)) {
    // 1 byte 0xxxxxxx
    *w = s[i];
    n = 1;
    debug(DEBUG_TRACE, "Lang", "1 byte %2X -> 0x%02X '%c'", c1, *w, *w);
  } else if ((c1 & 0xE0) == 0xC0) {
    // 2 bytes 110xxxxx 10xxxxxx
    *w = ((c1 & 0x1F) << 6) | (c2 & 0x3F);
    n = 2;
    debug(DEBUG_INFO, "Lang", "2 bytes sequence %2X %2X -> 0x%04X", c1, c2, *w);
  } else if ((c1 & 0xF0) == 0xE0) {
    // 3 bytes 1110xxxx 10xxxxxx 10xxxxxx
    *w = ((c1 & 0x0F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F);
    n = 3;
    debug(DEBUG_INFO, "Lang", "3 bytes sequence %2X %2X %2X -> 0x%04X", c1, c2, c3, *w);
  } else if ((c1 & 0xF8) == 0xF0) {
    // 4 bytes 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
    *w = ((c1 & 0x07) << 18) | ((c2 & 0x3F) << 12) | ((c3 & 0x3F) << 6) | (c4 & 0x3F);
    n = 4;
    debug(DEBUG_INFO, "Lang", "4 bytes sequence %2X %2X %2X %2X -> 0x%08X", c1, c2, c3, c4, *w);
  } else {
    debug(DEBUG_ERROR, "Lang", "invalid sequence %2X %2X %2X %2X", c1, c2, c3, c4);
    *w = s[i];
    n = 1;
  }

  return n;
}

static UInt8 mapChar(UInt32 w, FontType **f, void *data) {
  unscii_t *unscii = (unscii_t *)data;
  UInt32 page;

  if (w < 0x10000) {
    page = w >> 8;
    if (unscii->f8[page]) {
      *f = (FontType *)unscii->f8[page];
    } else {
      *f = FntGetFontPtr();
    }
  } else {
    *f = FntGetFontPtr();
  }

  return w;
}

static void finish(void *data) {
  unscii_t *unscii = (unscii_t *)data;
  int i;

  if (unscii) {
    for (i = 0; i < 256; i++) {
      if (unscii->h8[i]) {
        MemHandleUnlock(unscii->h8[i]);
        DmReleaseResource(unscii->h8[i]);
      }
      if (unscii->h16[i]) {
        MemHandleUnlock(unscii->h16[i]);
        DmReleaseResource(unscii->h16[i]);
      }
    }
    sys_free(unscii);
  }
}

Err LanguageSet(language_t *lang) {
  unscii_t *unscii;
  int i;
  Err err = sysErrParamErr;
  
  if ((unscii = sys_calloc(1, sizeof(unscii_t))) != NULL) {
    for (i = 0; pages[i] != 0xffff; i++) {
      if ((unscii->h8[i] = DmGetResource(fontExtRscType, BASE8 + pages[i])) != NULL) {
        unscii->f8[i] = MemHandleLock(unscii->h8[i]);
      }
      if ((unscii->h16[i] = DmGetResource(fontExtRscType, BASE16 + pages[i])) != NULL) {
        unscii->f16[i] = MemHandleLock(unscii->h16[i]);
      }
    }

    lang->nextChar = nextChar;
    lang->mapChar = mapChar;
    lang->finish = finish;
    lang->data = unscii;
    err = errNone;
  }

  return err;
}
