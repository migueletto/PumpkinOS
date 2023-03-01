#include <PalmOS.h>

#include "bytes.h"
#include "debug.h"

UInt16 LmGetNumLocales(void) {
  MemHandle h;
  UInt8 *p;
  UInt16 i, num = 0;

  if ((h = DmGetResource('locs', 10000)) != NULL) {
    if ((p = MemHandleLock(h)) != NULL) {
      i  = get2b(&num, p, 0); // version ???
      i += get2b(&num, p, i); // number of locales
      MemHandleUnlock(h);
    }
    DmReleaseResource(h);
  }

  return num;
}

Err LmTimeZoneToIndex(Int16 timeZone, UInt16 *ioLocaleIndex) {
  MemHandle h;
  UInt8 *p;
  UInt16 i, num;
  Int16 tz;
  Err err = errNone;

  if (ioLocaleIndex == NULL) {
    return lmErrSettingDataOverflow;
  }

  if ((h = DmGetResource('locs', 10000)) != NULL) {
    if ((p = MemHandleLock(h)) != NULL) {
      i  = get2b(&num, p, 0); // version ???
      i += get2b(&num, p, i); // number of locales
      p = &p[i];

      for (i = *ioLocaleIndex; i < num; i++) {
        get2b((uint16_t *)&tz, &p[26], 0);
        if (tz == timeZone) {
          *ioLocaleIndex = i;
          break;
        }
        p += 64;
      }
      if (i == num) {
        err = lmErrBadLocaleIndex;
      }
      MemHandleUnlock(h);
    } else {
      err = lmErrBadLocaleIndex;
    }
    DmReleaseResource(h);
  } else {
    err = lmErrBadLocaleIndex;
  }

  return err;
}

Err LmLocaleToIndex(const LmLocaleType *iLocale, UInt16 *oLocaleIndex) {
  MemHandle h;
  UInt8 *p;
  UInt16 i, num;
  Err err = errNone;

  if (iLocale == NULL || oLocaleIndex == NULL) {
    return lmErrSettingDataOverflow;
  }

  if ((h = DmGetResource('locs', 10000)) != NULL) {
    if ((p = MemHandleLock(h)) != NULL) {
      i  = get2b(&num, p, 0); // version ???
      i += get2b(&num, p, i); // number of locales
      p = &p[i];

      for (i = 0; i < num; i++) {
        if ((iLocale->language == lmAnyLanguage || iLocale->language == p[0]) &&
            (iLocale->country == lmAnyCountry || iLocale->country == p[1])) {
          *oLocaleIndex = i;
          break;
        }
        p += 64;
      }
      if (i == num) {
        err = lmErrBadLocaleIndex;
      }
      MemHandleUnlock(h);
    } else {
      err = lmErrBadLocaleIndex;
    }
    DmReleaseResource(h);
  } else {
    err = lmErrBadLocaleIndex;
  }

  return err;
}

/*
0000: 00 01 00 21 00 00 41 75 73 74 72 61 6c 69 61 00   ...!..Australia.
0010: 00 00 00 00 00 00 00 00 00 00 01 08 01 00 02 58   ...............X
0020: 00 00 41 75 73 74 72 61 6c 69 61 6e 20 44 6f 6c   ..Australian Dol
0030: 6c 61 72 00 00 00 24 00 00 00 00 00 41 55 24 00   lar...$.....AU$.
0040: 00 00 02 01

00b0: 00 00 00 00 00 00 42 46 00 00 00 00 42 46 00 00   ......BF....BF..
00c0: 00 00 02 01 5a 03 42 72 61 7a 69 6c 00 00 00 00   ....Z.Brazil....
00d0: 00 00 00 00 00 00 00 00 00 00 01 08 02 00 ff 4c   ...............L
00e0: 01 00 43 72 75 7a 65 69 72 6f 00 00 00 00 00 00   ..Cruzeiro......
00f0: 00 00 00 00 00 00 52 24 00 00 00 00 52 24 00 00   ......R$....R$..
0100: 00 00 02 01 00 04 43 61 6e 61 64 61 00 00 00 00   ......Canada....
*/

Err LmGetLocaleSetting(UInt16 iLocaleIndex, LmLocaleSettingChoice iChoice, void *oValue, UInt16 iValueSize) {
  LmLocaleType *locale;
  MemHandle h;
  DateFormatType *df;
  TimeFormatType *tf;
  NumberFormatType *nf;
  MeasurementSystemType *ms;
  UInt8 *p, *value8;
  UInt16 i, num;
  Err err = errNone;

  if (oValue == NULL) {
    return lmErrSettingDataOverflow;
  }

  if ((h = DmGetResource('locs', 10000)) != NULL) {
    if ((p = MemHandleLock(h)) != NULL) {
      i  = get2b(&num, p, 0); // version ???
      i += get2b(&num, p, i); // number of locales
      p = &p[i];

      if (iLocaleIndex < num) {
        p = &p[iLocaleIndex * 64];

        switch (iChoice) {
          case lmChoiceLocale:
            if (iValueSize >= sizeof(LmLocaleType)) {
              locale = (LmLocaleType *)oValue;
              locale->language = p[0];
              locale->country = p[1];
            } else {
              err = lmErrSettingDataOverflow;
            }
            break;
          case lmChoiceCountryName:
            if (iValueSize >= kMaxCountryNameLen+1) {
              MemMove(oValue, &p[2], kMaxCountryNameLen+1);
            } else {
              err = lmErrSettingDataOverflow;
            }
            break;
          case lmChoiceDateFormat:
            if (iValueSize >= sizeof(DateFormatType)) {
              df = (DateFormatType *)oValue;
              *df = p[22];
            } else {
              err = lmErrSettingDataOverflow;
            }
            break;
          case lmChoiceLongDateFormat:
            if (iValueSize >= sizeof(DateFormatType)) {
              df = (DateFormatType *)oValue;
              *df = p[23];
            } else {
              err = lmErrSettingDataOverflow;
            }
            break;
          case lmChoiceTimeFormat:
            if (iValueSize >= sizeof(TimeFormatType)) {
              tf = (TimeFormatType *)oValue;
              *tf = p[24];
            } else {
              err = lmErrSettingDataOverflow;
            }
            break;
          case lmChoiceWeekStartDay:
            if (iValueSize >= 1) {
              value8 = (UInt8 *)oValue;
              *value8 = p[25];
            } else {
              err = lmErrSettingDataOverflow;
            }
            break;
          case lmChoiceTimeZone:
            if (iValueSize >= 2) {
              get2b(oValue, &p[26], 0);
            } else {
              err = lmErrSettingDataOverflow;
            }
            break;
          case lmChoiceNumberFormat:
            if (iValueSize >= sizeof(NumberFormatType)) {
              nf = (NumberFormatType *)oValue;
              *nf = p[28];
            } else {
              err = lmErrSettingDataOverflow;
            }
            break;
          case lmChoiceCurrencyName:
            if (iValueSize >= kMaxCurrencyNameLen+1) {
              MemMove(oValue, &p[30], kMaxCurrencyNameLen+1);
            } else {
              err = lmErrSettingDataOverflow;
            }
            break;
          case lmChoiceCurrencySymbol:
            if (iValueSize >= kMaxCurrencySymbolLen+1) {
              MemMove(oValue, &p[50], kMaxCurrencySymbolLen+1);
            } else {
              err = lmErrSettingDataOverflow;
            }
            break;
          case lmChoiceUniqueCurrencySymbol:
            if (iValueSize >= kMaxCurrencySymbolLen+1) {
              MemMove(oValue, &p[56], kMaxCurrencySymbolLen+1);
            } else {
              err = lmErrSettingDataOverflow;
            }
            break;
          case lmChoiceCurrencyDecimalPlaces:
            if (iValueSize >= 1) {
              value8 = (UInt8 *)oValue;
              *value8 = p[62];
            } else {
              err = lmErrSettingDataOverflow;
            }
            break;
          case lmChoiceMeasurementSystem:
            if (iValueSize >= sizeof(MeasurementSystemType)) {
              ms = (MeasurementSystemType *)oValue;
              *ms = p[63];
            } else {
              err = lmErrSettingDataOverflow;
            }
            break;
          case lmChoiceSupportsLunarCalendar:
          case lmChoicePrimarySMSEncoding:
          case lmChoiceSecondarySMSEncoding:
          case lmChoicePrimaryEmailEncoding:
          case lmChoiceSecondaryEmailEncoding:
          case lmChoiceOutboundVObjectEncoding:
          case lmChoiceInboundDefaultVObjectEncoding:
          default:
            err = lmErrBadLocaleSettingChoice;
            break;
        }
      } else {
        err = lmErrBadLocaleIndex;
      }
      MemHandleUnlock(h);
    } else {
      err = lmErrBadLocaleIndex;
    }
    DmReleaseResource(h);
  } else {
    err = lmErrBadLocaleIndex;
  }

  return err;
}
