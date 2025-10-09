#include <PalmOS.h>

#include "pumpkin.h"
#include "bytes.h"
#include "debug.h"

// each word is 2 bytes, 10 words per locale
#define numCols 10
#define numLocales(h) ((MemHandleSize(h) - 2) / (numCols * 2))

UInt16 LmGetNumLocales(void) {
  MemHandle h;
  UInt16 num = 0;

  if ((h = DmGetResource(wrdListRscType, 22000)) != NULL) {
    num = numLocales(h);
    DmReleaseResource(h);
  }

  return num;
}

Err LmTimeZoneToIndex(Int16 timeZone, UInt16 *ioLocaleIndex) {
  MemHandle h;
  UInt16 i, num, *p;
  Int16 tz;
  Err err = errNone;

  if (ioLocaleIndex == NULL) {
    return lmErrSettingDataOverflow;
  }

  if ((h = DmGetResourceDecoded(wrdListRscType, 22000)) != NULL) {
    if ((p = MemHandleLock(h)) != NULL) {
      num = numLocales(h);
      p++;
      for (i = *ioLocaleIndex; i < num; i++) {
        tz = (Int16)p[i * numCols + 6];
        if (tz == timeZone) {
          *ioLocaleIndex = i;
          break;
        }
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
  UInt16 i, num, *p;
  Err err = errNone;

  if (iLocale == NULL || oLocaleIndex == NULL) {
    return lmErrSettingDataOverflow;
  }

  if ((h = DmGetResourceDecoded(wrdListRscType, 22000)) != NULL) {
    if ((p = MemHandleLock(h)) != NULL) {
      num = numLocales(h);
      p++;
      for (i = 0; i < num; i++) {
        if ((iLocale->language == lmAnyLanguage || iLocale->language == p[i * numCols + 1]) &&
            (iLocale->country == lmAnyCountry || iLocale->country == p[i * numCols + 0])) {
          *oLocaleIndex = i;
          break;
        }
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
  UInt8 *value8;
  UInt16 *value16, num, *p;
  Err err = errNone;

  if (oValue == NULL) {
    return lmErrSettingDataOverflow;
  }

  if ((h = DmGetResourceDecoded(wrdListRscType, 22000)) != NULL) {
    if ((p = MemHandleLock(h)) != NULL) {
      num = numLocales(h);
      p++;
      if (iLocaleIndex < num) {
        switch (iChoice) {
          case lmChoiceLocale:
            if (iValueSize >= sizeof(LmLocaleType)) {
              locale = (LmLocaleType *)oValue;
              locale->country  = p[iLocaleIndex * numCols + 0];
              locale->language = p[iLocaleIndex * numCols + 1];
            } else {
              err = lmErrSettingDataOverflow;
            }
            break;
          case lmChoiceCountryName:
            if (iValueSize >= kMaxCountryNameLen+1) {
              MemMove(oValue, PrefCountryName(iLocaleIndex), kMaxCountryNameLen+1);
            } else {
              err = lmErrSettingDataOverflow;
            }
            break;
          case lmChoiceDateFormat:
            if (iValueSize >= sizeof(DateFormatType)) {
              df = (DateFormatType *)oValue;
              *df = p[iLocaleIndex * numCols + 2];
            } else {
              err = lmErrSettingDataOverflow;
            }
            break;
          case lmChoiceLongDateFormat:
            if (iValueSize >= sizeof(DateFormatType)) {
              df = (DateFormatType *)oValue;
              *df = p[iLocaleIndex * numCols + 3];
            } else {
              err = lmErrSettingDataOverflow;
            }
            break;
          case lmChoiceTimeFormat:
            if (iValueSize >= sizeof(TimeFormatType)) {
              tf = (TimeFormatType *)oValue;
              *tf = p[iLocaleIndex * numCols + 4];
            } else {
              err = lmErrSettingDataOverflow;
            }
            break;
          case lmChoiceWeekStartDay:
            if (iValueSize >= 1) {
              value8 = (UInt8 *)oValue;
              *value8 = p[iLocaleIndex * numCols + 5];
            } else {
              err = lmErrSettingDataOverflow;
            }
            break;
          case lmChoiceTimeZone:
            if (iValueSize >= 2) {
              value16 = (UInt16 *)oValue;
              *value16 = p[iLocaleIndex * numCols + 6];
            } else {
              err = lmErrSettingDataOverflow;
            }
            break;
          case lmChoiceNumberFormat:
            if (iValueSize >= sizeof(NumberFormatType)) {
              nf = (NumberFormatType *)oValue;
              *nf = p[iLocaleIndex * numCols + 7];
            } else {
              err = lmErrSettingDataOverflow;
            }
            break;
          case lmChoiceCurrencyName:
            if (iValueSize >= kMaxCurrencyNameLen+1) {
              SysStringByIndex(21000, iLocaleIndex, (char *)oValue, kMaxCurrencyNameLen+1);
            } else {
              err = lmErrSettingDataOverflow;
            }
            break;
          case lmChoiceCurrencySymbol:
            if (iValueSize >= kMaxCurrencySymbolLen+1) {
              SysStringByIndex(19000, iLocaleIndex, (char *)oValue, kMaxCurrencySymbolLen+1);
            } else {
              err = lmErrSettingDataOverflow;
            }
            break;
          case lmChoiceUniqueCurrencySymbol:
            if (iValueSize >= kMaxCurrencySymbolLen+1) {
              SysStringByIndex(20000, iLocaleIndex, (char *)oValue, kMaxCurrencySymbolLen+1);
            } else {
              err = lmErrSettingDataOverflow;
            }
            break;
          case lmChoiceCurrencyDecimalPlaces:
            if (iValueSize >= 1) {
              value8 = (UInt8 *)oValue;
              *value8 = p[iLocaleIndex * numCols + 8];
            } else {
              err = lmErrSettingDataOverflow;
            }
            break;
          case lmChoiceMeasurementSystem:
            if (iValueSize >= sizeof(MeasurementSystemType)) {
              ms = (MeasurementSystemType *)oValue;
              *ms = p[iLocaleIndex * numCols + 9];
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
