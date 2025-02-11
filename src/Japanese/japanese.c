#include <PalmOS.h>

#include "language.h"

#define SYSTEM_FONTS  8
#define NUM_FONTS    44

// To activate Japanese support, compile this and start PumpkinOS with (on Linux):
// LANG=ja_jp ./pumpkin.sh

typedef struct {
  MemHandle kh[SYSTEM_FONTS];
  FontTypeV2 *kf[SYSTEM_FONTS];
  MemHandle h[NUM_FONTS];
  FontTypeV2 *f[NUM_FONTS];
} japanese_t;

static const int systemIds[SYSTEM_FONTS] = {
  10000, 10001, 10002, 10100, 10101, 10102, 10103, 10003
};

static const int customIds[NUM_FONTS] = {
  10004, 10005, 10006, 10007,
  10009, 10010, 10011, 10012, 10013, 10014, 10015,
  10016, 10017, 10018, 10019, 10020, 10021, 10022,
  10023, 10024, 10025, 10026, 10027, 10028, 10029,
  10030, 10031, 10032, 10033, 10034, 10035, 10036,
  10037, 10038, 10039, 10040, 10041, 10042, 10043,
  10044, 10045, 10046, 10047, 10048
};

static int getIndex(UInt16 c) {
  int index = -1;

  switch (c) {
    // first byte of JIS X 0208 Kanji character
    case 0x81: index = 0; break;
    case 0x82: index = 1; break;
    case 0x83: index = 2; break;
    case 0x84: index = 3; break;
    case 0x88: index = 4; break;
    case 0x89: index = 5; break;
    case 0x8A: index = 6; break;
    case 0x8B: index = 7; break;
    case 0x8C: index = 8; break;
    case 0x8D: index = 9; break;
    case 0x8E: index = 10; break;
    case 0x8F: index = 11; break;
    case 0x90: index = 12; break;
    case 0x91: index = 13; break;
    case 0x92: index = 14; break;
    case 0x93: index = 15; break;
    case 0x94: index = 16; break;
    case 0x95: index = 17; break;
    case 0x96: index = 18; break;
    case 0x97: index = 19; break;
    case 0x98: index = 20; break;
    case 0x99: index = 21; break;
    case 0x9A: index = 22; break;
    case 0x9B: index = 23; break;
    case 0x9C: index = 24; break;
    case 0x9D: index = 25; break;
    case 0x9E: index = 26; break;
    case 0x9F: index = 27; break;

    // first byte of JIS X 0208 Kanji character
    case 0xE0: index = 28; break;
    case 0xE1: index = 29; break;
    case 0xE2: index = 30; break;
    case 0xE3: index = 31; break;
    case 0xE4: index = 32; break;
    case 0xE5: index = 33; break;
    case 0xE6: index = 34; break;
    case 0xE7: index = 35; break;
    case 0xE8: index = 36; break;
    case 0xE9: index = 37; break;
    case 0xEA: index = 38; break;
    case 0xEB: index = 39; break;
    case 0xEC: index = 40; break;
    case 0xED: index = 41; break;
    case 0xEE: index = 42; break;
    case 0xEF: index = 43; break;
  }

  return index;
}

static UInt32 nextChar(UInt8 *s, UInt32 i, UInt32 len, UInt32 *w, void *data) {
  UInt32 c, n;

  c = s[i];

  switch (FntGetFont()) {
    case stdFont:
    case boldFont:
    case largeFont:
    case largeBoldFont:
      if (getIndex(c) >= 0) {
        *w = (c << 8) | s[i+1];
        n = 2;
      } else {
        *w = c;
        n = 1;
      }
      break;
    default:
      *w = c;
      n = 1;
      break;
  }

  return n;
}

static UInt8 mapChar(UInt32 w, FontType **f, void *data) {
  japanese_t *japanese = (japanese_t *)data;
  int index = -1;

  switch (FntGetFont()) {
    case stdFont:
    case boldFont:
    case largeFont:
    case largeBoldFont:
      if (w & 0x8000) {
        index = getIndex(w >> 8);
        if (index >= 0) {
          // kanji
          *f = (FontType *)japanese->f[index];
        } else {
          // missing kangi
          *f = FntGetFontPtr();
          w = ' ';
        }
      } else {
        // ASCII + katakana
        *f = FntGetFontPtr();
      }
      break;
    default:
      *f = FntGetFontPtr();
      break;
  }

  return w & 0xFF;
}

static void finish(void *data) {
  japanese_t *japanese = (japanese_t *)data;
  int i;

  if (japanese) {
    for (i = 0; i < SYSTEM_FONTS; i++) {
      if (japanese->kh[i]) {
        MemHandleUnlock(japanese->kh[i]);
        DmReleaseResource(japanese->kh[i]);
      }
    }

    for (i = 0; i < NUM_FONTS; i++) {
      if (japanese->h[i]) {
        MemHandleUnlock(japanese->h[i]);
        DmReleaseResource(japanese->h[i]);
      }
    }
    sys_free(japanese);
  }
}

Err LanguageSet(language_t *lang) {
  japanese_t *japanese;
  int i;
  Err err = sysErrParamErr;

  if ((japanese = sys_calloc(1, sizeof(japanese_t))) != NULL) {
    for (i = 0; i < SYSTEM_FONTS; i++) {
      if ((japanese->kh[i] = DmGetResource(fontExtRscType, systemIds[i])) != NULL) {
        japanese->kf[i] = MemHandleLock(japanese->kh[i]);
      }
    }

    for (i = 0; i < NUM_FONTS; i++) {
      if ((japanese->h[i] = DmGetResource(fontExtRscType, customIds[i])) != NULL) {
        japanese->f[i] = MemHandleLock(japanese->h[i]);
      }
    }

    lang->nextChar = nextChar;
    lang->mapChar = mapChar;
    lang->finish = finish;
    lang->data = japanese;
    err = errNone;
  }

  return err;
}
