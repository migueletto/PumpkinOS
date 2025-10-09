#include <PalmOS.h>

#include "mutex.h"
#include "bytes.h"
#include "storage.h"
#include "debug.h"

static LmLocaleType locale;

Err OmOverlayDBNameToLocale(const Char *overlayDBName, LmLocaleType *overlayLocale) {
  debug(DEBUG_ERROR, "Overlay", "OmOverlayDBNameToLocale not implemented");
  return omErrUnknownLocale;
}

Err OmLocaleToOverlayDBName(const Char *baseDBName, const LmLocaleType *targetLocale, Char *overlayDBName) {
  // Return the overlay database’s name given the base database name and the locale.
  // Returns errNone upon success, or omErrUnknownLocale if the targetLocale parameter is invalid.
  // The appropriate overlay database name is currently: baseDBName_llCC
  //   baseDBName: the name of the base database as you passed it in.
  //   ll: two-character code identifying the language.
  //   CC: two-character code identifying the country.
  // The base database name is truncated if necessary to allow for this suffix.

  UInt16 localeIndex, len;
  char *language, *country;
  Err err = omErrUnknownLocale;

  if (baseDBName && targetLocale && overlayDBName) {
    if (LmLocaleToIndex(targetLocale, &localeIndex) == errNone) {
      language = PrefLanguageCode(targetLocale->language);
      country = PrefCountryCode(targetLocale->country);

      if (language && country) {
        len = StrLen(baseDBName);
        if (len + 5 > dmDBNameLength-1) {
          len = dmDBNameLength - 1 - 5;
        }
        MemMove(overlayDBName, baseDBName, len);
        overlayDBName[len] = '_';
        overlayDBName[len + 1] = TxtLowerChar(language[0]);
        overlayDBName[len + 2] = TxtLowerChar(language[1]);
        overlayDBName[len + 3] = country[0];
        overlayDBName[len + 4] = country[1];
        overlayDBName[len + 5] = 0;
        err = errNone;
      }
    }
  }

  return err;
}

void OmGetCurrentLocale(LmLocaleType *currentLocale) {
  OmGetSystemLocale(currentLocale);
}

Err OmGetIndexedLocale(UInt16 localeIndex, LmLocaleType *theLocale) {
  return LmGetLocaleSetting(localeIndex, lmChoiceLocale, theLocale, sizeof(LmLocaleType)) == errNone ? errNone : omErrInvalidLocaleIndex;
}

void OmGetSystemLocale(LmLocaleType *systemLocale) {
  if (systemLocale) {
    systemLocale->language = locale.language;
    systemLocale->country = locale.country;
  }
}

// must be called only by main thread on startup
Err OmSetSystemLocale(const LmLocaleType *systemLocale) {
  if (systemLocale) {
    locale.language = systemLocale->language;
    locale.country = systemLocale->country;
  }

  return errNone;
}

void *OmGetRoutineAddress(OmSelector inSelector) {
  debug(DEBUG_ERROR, "Overlay", "OmGetRoutineAddress not implemented");
  return NULL;
}

Err OmGetNextSystemLocale(Boolean iNewSearch, OmSearchStateType *ioStateInfoP, LmLocaleType *oLocaleP) {
  debug(DEBUG_ERROR, "Overlay", "OmGetNextSystemLocale not implemented");
  return omErrNoNextSystemLocale;
}

/*
Structure of ovly.1000 resources:

typedef struct {
  UInt16 version;
  UInt32 flags;
  UInt32 baseChecksum;
  OmLocaleType targetLocale;
  UInt32 baseDBType;
  UInt32 baseDBCreator;
  UInt32 baseDBCreateDate;
  UInt32 baseDBModDate;
  UInt16 numOverlays;
  OmOverlayRscType overlays[0];
} OmOverlaySpecType;

// Flags for OmOverlaySpecType.flags field
#define omSpecAttrForBase  1   // 'ovly' (in base) describes base itself
#define omSpecAttrStripped 2   // Localized resources stripped (base only)
#define omSpecAttr???      4   // ???

typedef struct {
  OmOverlayKind overlayType;
  UInt32 rscType;
  UInt16 rscID;
  UInt32 rscLength;
  UInt32 rscChecksum;
} OmOverlayRscType;

// Values for OmOverlayKind
#define omOverlayKindHide    0  // Hide base resource (not supported in version <= 3)
#define omOverlayKindAdd     1  // Add new resource (not support in version <= 2)
#define omOverlayKindReplace 2  // Replace base resource
#define omOverlayKindBase    3  // Description of base resource itself (not supported in version <= 2)

Resource 'ovly' 1000 from FileBrowser-PFil.prc:
00000000  00 04 00 00 00 07 ff ff  e2 ef 00 00 00 17 61 70  |..............ap|
00000010  70 6c 50 46 69 6c 00 00  00 00 00 00 00 00 00 4a  |plPFil.........J|
00000020  00 03 4d 42 41 52 17 70  00 00 01 52 00 00 5e cc  |..MBAR.p...R..^.|
...
version: 4 (0x0004)
flags: 0x00000007
baseChecksum: 0xffffe2ef
targetLocale.language: 0 (0x0000)
targetLocale.country: 23 (0x0017)
baseDBType: appl (0x6170706c)
baseDBCreator: PFil (0x5046696c)
baseDBCreateDate: 0
baseDBModDate: 0
numOverlays: 74 (0x004A)

overlayType: 3 (0x0003) = omOverlayKindBase
rscType: MBAR
rscID: 6000 (0x1770)
rscLength: 338 (0x00000152)
rscChecksum: 0x00005ecc

Resource 'ovly' 1000 from FileBrowser-PFil_enUS.prc:
00000000  00 04 00 00 00 04 ff ff  e2 ef 00 00 00 17 61 70  |..............ap|
00000010  70 6c 50 46 69 6c be 92  a8 3e be 92 a8 3e 00 4a  |plPFil...>...>.J|
00000020  00 02 4d 42 41 52 17 70  00 00 01 52 00 00 5e cc  |..MBAR.p...R..^.|
...

*/

OmOverlaySpecType *pumpkin_create_overlay(void *h, uint8_t *p, uint32_t size, uint32_t *dsize) {
  OmOverlaySpecType *ov = NULL;
  UInt16 numOverlays, j;
  char st[8];
  int i;

  get2b(&numOverlays, p, 30);

  if ((ov = StoNewDecodedResource(h, sizeof(OmOverlaySpecType) + numOverlays * sizeof(OmOverlayRscType), 0, 0)) != NULL) {
    i = 0;
    i += get2b(&ov->version, p, i);
    i += get4b(&ov->flags, p, i);
    i += get4b(&ov->baseChecksum, p, i);
    i += get2b(&ov->targetLocale.language, p, i);
    i += get2b(&ov->targetLocale.country, p, i);
    i += get4b(&ov->baseDBType, p, i);
    i += get4b(&ov->baseDBCreator, p, i);
    i += get4b(&ov->baseDBCreateDate, p, i);
    i += get4b(&ov->baseDBModDate, p, i);
    i += get2b(&ov->numOverlays, p, i);
    debug(DEBUG_TRACE, "Overlay", "overlay version=%u, flags=0x%02X, language=%u, country=%u, entries=%d",
      ov->version, ov->flags, ov->targetLocale.language, ov->targetLocale.country, ov->numOverlays);

    for (j = 0; j < numOverlays; j++) {
      i += get2b(&ov->overlays[j].overlayType, p, i);
      i += get4b(&ov->overlays[j].rscType, p, i);
      i += get2b(&ov->overlays[j].rscID, p, i);
      i += get4b(&ov->overlays[j].rscLength, p, i);
      i += get4b(&ov->overlays[j].rscChecksum, p, i);
      pumpkin_id2s(ov->overlays[j].rscType, st);
      debug(DEBUG_TRACE, "Overlay", "resource %3u: kind %u, type '%s', id %5u, size %5u",
        j, ov->overlays[j].overlayType, st, ov->overlays[j].rscID, ov->overlays[j].rscLength);
    }
  }

  return ov;
}

void pumpkin_destroy_overlay(void *p) {
  if (p) {
    MemChunkFree(p);
  }
}
