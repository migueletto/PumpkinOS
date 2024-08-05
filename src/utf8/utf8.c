#include <PalmOS.h>

#include "pumpkin.h"
#include "language.h"
#include "debug.h"

// https://www.cogsci.ed.ac.uk/~richard/utf-8.cgi?input=C2+A0&mode=bytes

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
  if (w > 0xFF) {
    switch (w) {
      case 0x2010:
      case 0x2011: w = '-'; break;
      case 0x2012:
      case 0x2013:
      case 0x2014: w = '_'; break;
      case 0x2015: w = 151; break;
      case 0x2018: w = 145; break;
      case 0x2019: w = 146; break;
      case 0x201C: w = 147; break;
      case 0x201D: w = 148; break;
      case 0x2020: w = 134; break;
      case 0x2021: w = 135; break;
      case 0x2022: w = 149; break;
      case 0x2030: w = 137; break;
      case 0x20AC: w = 128; break;
      case 0x2122: w = 153; break;
      default: w = ' '; break;
    }
    *f = FntGetFontPtr();
  }

  return w;
}

static void finish(void *data) {
}

Err LanguageSet(language_t *lang) {
  lang->nextChar = nextChar;
  lang->mapChar = mapChar;
  lang->finish = finish;

  return errNone;
}
