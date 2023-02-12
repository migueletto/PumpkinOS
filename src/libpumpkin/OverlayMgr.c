#include <PalmOS.h>

#include "debug.h"

static LmLocaleType locale;

Err OmOverlayDBNameToLocale(const Char *overlayDBName, LmLocaleType *overlayLocale) {
  debug(DEBUG_ERROR, "PALMOS", "OmOverlayDBNameToLocale not implemented");
  return omErrUnknownLocale;
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

// must be called only by PalmOS thread on startup
Err OmSetSystemLocale(const LmLocaleType *systemLocale) {
  if (systemLocale) {
    locale.language = systemLocale->language;
    locale.country = systemLocale->country;
  }

  return errNone;
}

void *OmGetRoutineAddress(OmSelector inSelector) {
  debug(DEBUG_ERROR, "PALMOS", "OmGetRoutineAddress not implemented");
  return NULL;
}

Err OmGetNextSystemLocale(Boolean iNewSearch, OmSearchStateType *ioStateInfoP, LmLocaleType *oLocaleP) {
  debug(DEBUG_ERROR, "PALMOS", "OmGetNextSystemLocale not implemented");
  return omErrNoNextSystemLocale;
}
