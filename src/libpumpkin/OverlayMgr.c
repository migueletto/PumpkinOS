#include <PalmOS.h>

#include "debug.h"

static LmLocaleType locale;

Err OmOverlayDBNameToLocale(const Char *overlayDBName, LmLocaleType *overlayLocale) {
  debug(DEBUG_ERROR, PUMPKINOS, "OmOverlayDBNameToLocale not implemented");
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
        StrNCopy(overlayDBName, baseDBName, len);
        language[0] = TxtLowerChar(language[0]);
        language[1] = TxtLowerChar(language[1]);
	StrCat(overlayDBName, "_");
	StrCat(overlayDBName, language);
	StrCat(overlayDBName, country);
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

// must be called only by PalmOS thread on startup
Err OmSetSystemLocale(const LmLocaleType *systemLocale) {
  if (systemLocale) {
    locale.language = systemLocale->language;
    locale.country = systemLocale->country;
  }

  return errNone;
}

void *OmGetRoutineAddress(OmSelector inSelector) {
  debug(DEBUG_ERROR, PUMPKINOS, "OmGetRoutineAddress not implemented");
  return NULL;
}

Err OmGetNextSystemLocale(Boolean iNewSearch, OmSearchStateType *ioStateInfoP, LmLocaleType *oLocaleP) {
  debug(DEBUG_ERROR, PUMPKINOS, "OmGetNextSystemLocale not implemented");
  return omErrNoNextSystemLocale;
}
