#include <PalmOS.h>
#include <time.h>
#include <sys/time.h>

#include "resource.h"
#include "thread.h"
#include "mutex.h"
#include "AppRegistry.h"
#include "storage.h"
#include "pumpkin.h"
#include "debug.h"
#include "xalloc.h"

#define AppName "Preferences"
#define AppID   'Pref'

typedef struct {
  UInt16 index;
  char name[kMaxCountryNameLen+1];
} prefs_country_t;

typedef struct {
  char dateBuf[32];
  char longDateBuf[32];
  char timeBuf[32];
  char timeZoneBuf[64];
  UInt16 numLocales;
  prefs_country_t *countries;
  char **countryNames;
} prefs_data_t;

static const char *choices[] = {
  "Date & Time",
  "Formats"
};

static void formSetup(FormType *frm) {
  UInt16 index;
  ListType *lst;
  ControlType *ctl;

  index = FrmGetObjectIndex(frm, panelList);
  lst = (ListType *)FrmGetObjectPtr(frm, index);
  LstSetListChoices(lst, (char **)choices, 2);

  index = FrmGetObjectIndex(frm, panelTrigger);
  ctl = (ControlType *)FrmGetObjectPtr(frm, index);

  switch (FrmGetActiveFormID()) {
    case datetimeForm:
      LstSetSelection(lst, 0);
      CtlSetLabel(ctl, choices[0]);
      break;
    case formatsForm:
      LstSetSelection(lst, 1);
      CtlSetLabel(ctl, choices[1]);
      break;
  }
}

static void formSelect(UInt16 index) {
  switch (index) {
    case 0:
      FrmGotoForm(datetimeForm);
      break;
    case 1:
      FrmGotoForm(formatsForm);
      break;
  }
}

static Boolean DatetimeFormHandleEvent(EventPtr event) {
  prefs_data_t *data = pumpkin_get_data();
  SystemPreferencesType prefs;
  UInt32 seconds;
  DateTimeType dateTime;
  FormType *frm;
  MemHandle h;
  ControlType *ctl;
  ListType *lst;
  Int16 year, month, day, hour, minute;
  Int16 timeZone;
  LmLocaleType locale;
  UInt16 i, index, *tz;
  Boolean handled = false;

  switch (event->eType) {
    case frmOpenEvent:
      frm = FrmGetActiveForm();
      formSetup(frm);

      PrefGetPreferences(&prefs);
      seconds = TimGetSeconds();
      TimSecondsToDateTime(seconds, &dateTime);

      DateToAscii(dateTime.month, dateTime.day, dateTime.year, prefs.dateFormat, data->dateBuf);
      index = FrmGetObjectIndex(frm, setDateTrigger);
      ctl = (ControlType *)FrmGetObjectPtr(frm, index);
      CtlSetLabel(ctl, data->dateBuf);

      TimeToAscii(dateTime.hour, dateTime.minute, prefs.timeFormat, data->timeBuf);
      index = FrmGetObjectIndex(frm, setTimeTrigger);
      ctl = (ControlType *)FrmGetObjectPtr(frm, index);
      CtlSetLabel(ctl, data->timeBuf);

      if ((h = DmGetResource(wrdListRscType, 13400)) != NULL) {
        if ((tz = MemHandleLock(h)) != NULL) {
          for (i = 0; i < tz[0]; i++) {
            if (prefs.timeZone == (Int16)tz[i+1]) {
              if (SysStringByIndex(13400, i, data->timeZoneBuf, 64) != NULL) {
                index = FrmGetObjectIndex(frm, setTimeZoneTrigger);
                ctl = (ControlType *)FrmGetObjectPtr(frm, index);
                CtlSetLabel(ctl, data->timeZoneBuf);
              }
              break;
            }
          }
          MemHandleUnlock(h);
        }
        DmReleaseResource(h);
      }

      index = FrmGetObjectIndex(frm, daylightTrigger);
      ctl = (ControlType *)FrmGetObjectPtr(frm, index);
      CtlSetLabel(ctl, prefs.daylightSavings ? "On" : "Off");

      index = FrmGetObjectIndex(frm, daylightList);
      lst = (ListType *)FrmGetObjectPtr(frm, index);
      LstSetSelection(lst, prefs.daylightSavings ? 1 : 0);

      FrmDrawForm(frm);
      handled = true;
      break;

    case ctlSelectEvent:
      switch (event->data.ctlSelect.controlID) {
        case setDateTrigger:
          seconds = TimGetSeconds();
          TimSecondsToDateTime(seconds, &dateTime);
          month = dateTime.month;
          day = dateTime.day;
          year = dateTime.year;
          if (SelectDay(selectDayByDay, &month, &day, &year, "Set Date")) {
            // XXX set system date
          }
          handled = true;
          break;
        case setTimeTrigger:
          seconds = TimGetSeconds();
          TimSecondsToDateTime(seconds, &dateTime);
          hour = dateTime.hour;
          minute = dateTime.minute;
          if (SelectOneTime(&hour, &minute, "Set Time")) {
            // XXX set system time
          }
          handled = true;
          break;
        case setTimeZoneTrigger:
          PrefGetPreferences(&prefs);
          timeZone = prefs.timeZone;
          locale.language = lmAnyLanguage;
          locale.country = prefs.country;
          if (SelectTimeZone(&timeZone, &locale, "Set Time Zone", true, false)) {
            PrefSetPreference(prefTimeZone, timeZone);
          }
          handled = true;
          break;
        default:
          break;
      }
      break;

    case popSelectEvent:
      switch (event->data.popSelect.listID) {
        case panelList:
          formSelect(event->data.popSelect.selection);
          handled = true;
          break;
        case daylightList:
          PrefSetPreference(prefDaylightSavings, event->data.popSelect.selection);
          handled = true;
          break;
      }
      break;

    case menuEvent:
      if (event->data.menu.itemID == aboutCmd) {
        AbtShowAboutPumpkin(AppID);
      }
      handled = true;
      break;

    default:
      break;
  }

  return handled;
}

static Boolean FormatsFormHandleEvent(EventPtr event) {
  prefs_data_t *data = pumpkin_get_data();
  SystemPreferencesType prefs;
  UInt32 seconds;
  DateTimeType dateTime;
  FormType *frm;
  ControlType *ctl;
  ListType *lst;
  RectangleType timeRect, dateRect, longDateRect;
  UInt16 i, index, localeIndex, listIndex, dx;
  FontID timeFont, dateFont, longDateFont, old;
  DateFormatType dateFormat;
  TimeFormatType timeFormat;
  NumberFormatType numberFormat;
  LmLocaleType locale;
  UInt8 weekStartDay;
  char *text;
  Boolean handled = false;

  switch (event->eType) {
    case frmOpenEvent:
    case frmUpdateEvent:
      frm = FrmGetActiveForm();
      formSetup(frm);

      PrefGetPreferences(&prefs);
      for (i = 0; i < data->numLocales; i++) {
        if (prefs.country == data->countries[i].index) {
          break;
        }
      }
      if (i == data->numLocales) {
        i = 0;
      }

      index = FrmGetObjectIndex(frm, presetTrigger);
      ctl = (ControlType *)FrmGetObjectPtr(frm, index);
      CtlSetLabel(ctl, data->countryNames[i]);

      index = FrmGetObjectIndex(frm, presetList);
      lst = (ListType *)FrmGetObjectPtr(frm, index);
      LstSetListChoices(lst, data->countryNames, data->numLocales);
      LstSetSelection(lst, i);

      switch (prefs.timeFormat) {
        case tfColon:
        case tfColonAMPM:  // 1:00 pm
        case tfHoursAMPM:  // XXX 1 pm 
          listIndex = 0;
          break;
        case tfColon24h:   // 13:00
        case tfHours24h:   // XXX 13
          listIndex = 1;
          break;
        case tfDot:
        case tfDotAMPM:    // 1.00 pm
          listIndex = 2;
          break;
        case tfDot24h:     // 13.00
          listIndex = 3;
          break;
        case tfComma24h:   // 13,00
          listIndex = 4;
          break;
      }

      index = FrmGetObjectIndex(frm, timeFormatList);
      lst = (ListType *)FrmGetObjectPtr(frm, index);
      LstSetSelection(lst, listIndex);
      text = LstGetSelectionText(lst, listIndex);

      index = FrmGetObjectIndex(frm, timeFormatTrigger);
      FrmGetObjectBounds(frm, index, &timeRect);
      ctl = (ControlType *)FrmGetObjectPtr(frm, index);
      CtlSetLabel(ctl, text);
      timeFont = ctl->font;

      switch (prefs.dateFormat) {
        case dfMDYWithSlashes:   // 12/31/95
          listIndex = 0;
          break;
        case dfDMYWithSlashes:   // 31/12/95
          listIndex = 1;
          break;
        case dfDMYWithDots:      // 31.12.95
          listIndex = 2;
          break;
        case dfDMYWithDashes:    // 31-12-95
          listIndex = 3;
          break;
        case dfYMDWithSlashes:   // 95/12/31
          listIndex = 4;
          break;
        case dfYMDWithDots:      // 95.12.31
          listIndex = 5;
          break;
        case dfYMDWithDashes:    // 95-12-31
          listIndex = 6;
          break;
        case dfMDYWithDashes:    // 12-31-95
          listIndex = 7;
          break;
        default:
          listIndex = 0;
          break;
      }

      index = FrmGetObjectIndex(frm, dateFormatList);
      lst = (ListType *)FrmGetObjectPtr(frm, index);
      LstSetSelection(lst, listIndex);
      text = LstGetSelectionText(lst, listIndex);

      index = FrmGetObjectIndex(frm, dateFormatTrigger);
      FrmGetObjectBounds(frm, index, &dateRect);
      ctl = (ControlType *)FrmGetObjectPtr(frm, index);
      CtlSetLabel(ctl, text);
      dateFont = ctl->font;

      switch (prefs.longDateFormat) {
        case dfMDYLongWithComma: // Dec 31, 1995
          listIndex = 0;
          break;
        case dfDMYLong:          // 31 Dec 1995
          listIndex = 1;
          break;
        case dfDMYLongWithComma: // 31 Dec, 1995
          listIndex = 2;
          break;
        case dfDMYLongWithDot:   // 31. Dec 1995
          listIndex = 3;
          break;
        case dfYMDLongWithSpace: // 1995 Dec 31
          listIndex = 4;
          break;
        case dfYMDLongWithDot:   // 1995.12.31
          listIndex = 5;
          break;
        case dfDMYLongNoDay:     // Dec 1995
          listIndex = 6;
          break;
        case dfMYMed:            // Dec '95
          listIndex = 7;
          break;
        case dfMYMedNoPost:      // Dec 95
          listIndex = 8;
          break;
        default:
          listIndex = 0;
          break;
      }

      index = FrmGetObjectIndex(frm, ldateFormatList);
      lst = (ListType *)FrmGetObjectPtr(frm, index);
      LstSetSelection(lst, listIndex);
      text = LstGetSelectionText(lst, listIndex);

      index = FrmGetObjectIndex(frm, ldateFormatTrigger);
      FrmGetObjectBounds(frm, index, &longDateRect);
      ctl = (ControlType *)FrmGetObjectPtr(frm, index);
      CtlSetLabel(ctl, text);
      longDateFont = ctl->font;

      index = FrmGetObjectIndex(frm, weekStartList);
      lst = (ListType *)FrmGetObjectPtr(frm, index);
      listIndex = prefs.weekStartDay == sunday ? 0 : 1;
      LstSetSelection(lst, listIndex);
      text = LstGetSelectionText(lst, listIndex);

      index = FrmGetObjectIndex(frm, weekStartTrigger);
      ctl = (ControlType *)FrmGetObjectPtr(frm, index);
      CtlSetLabel(ctl, text);

      index = FrmGetObjectIndex(frm, numbersList);
      lst = (ListType *)FrmGetObjectPtr(frm, index);
      listIndex = prefs.numberFormat;
      if (listIndex >= 5) listIndex = 0;
      LstSetSelection(lst, listIndex);
      text = LstGetSelectionText(lst, listIndex);

      index = FrmGetObjectIndex(frm, numbersTrigger);
      ctl = (ControlType *)FrmGetObjectPtr(frm, index);
      CtlSetLabel(ctl, text);

      FrmDrawForm(frm);

      WinDrawLine(0, timeRect.topLeft.y - 5, 159, timeRect.topLeft.y - 5);

      seconds = TimGetSeconds();
      TimSecondsToDateTime(seconds, &dateTime);
      TimeToAscii(dateTime.hour, dateTime.minute, prefs.timeFormat, data->timeBuf);
      DateToAscii(dateTime.month, dateTime.day, dateTime.year, prefs.dateFormat, data->dateBuf);
      DateToAscii(dateTime.month, dateTime.day, dateTime.year, prefs.longDateFormat, data->longDateBuf);

      old = FntSetFont(symbol7Font);
      dx = FntCharWidth(2);
      FntSetFont(old);

      timeRect.topLeft.x += dx;
      dateRect.topLeft.x += dx;
      longDateRect.topLeft.x += dx;

      old = FntSetFont(timeFont);
      WinPaintChars(data->timeBuf, StrLen(data->timeBuf), timeRect.topLeft.x, timeRect.topLeft.y + FntCharHeight());
      FntSetFont(old);

      old = FntSetFont(dateFont);
      WinPaintChars(data->dateBuf, StrLen(data->dateBuf), dateRect.topLeft.x, dateRect.topLeft.y + FntCharHeight());
      FntSetFont(old);

      old = FntSetFont(longDateFont);
      WinPaintChars(data->longDateBuf, StrLen(data->longDateBuf), longDateRect.topLeft.x, longDateRect.topLeft.y + FntCharHeight());
      FntSetFont(old);

      handled = true;
      break;

    case popSelectEvent:
      switch (event->data.popSelect.listID) {
        case panelList:
          formSelect(event->data.popSelect.selection);
          handled = true;
          break;
        case presetList:
          if (event->data.popSelect.selection < data->numLocales) {
            localeIndex = data->countries[event->data.popSelect.selection].index;
            PrefSetPreference(prefCountry, localeIndex);

            if (LmGetLocaleSetting(localeIndex, lmChoiceLocale, &locale, sizeof(LmLocaleType)) == errNone) {
              PrefSetPreference(prefLanguage, locale.language);
            }
            if (LmGetLocaleSetting(localeIndex, lmChoiceTimeFormat, &timeFormat, sizeof(TimeFormatType)) == errNone) {
              PrefSetPreference(prefTimeFormat, timeFormat);
            }
            if (LmGetLocaleSetting(localeIndex, lmChoiceDateFormat, &dateFormat, sizeof(DateFormatType)) == errNone) {
              PrefSetPreference(prefDateFormat, dateFormat);
            }
            if (LmGetLocaleSetting(localeIndex, lmChoiceLongDateFormat, &dateFormat, sizeof(DateFormatType)) == errNone) {
              PrefSetPreference(prefLongDateFormat, dateFormat);
            }
            if (LmGetLocaleSetting(localeIndex, lmChoiceWeekStartDay, &weekStartDay, sizeof(UInt8)) == errNone) {
              PrefSetPreference(prefWeekStartDay, weekStartDay);
            }
            if (LmGetLocaleSetting(localeIndex, lmChoiceNumberFormat, &numberFormat, sizeof(NumberFormatType)) == errNone) {
              PrefSetPreference(prefNumberFormat, numberFormat);
            }

            FrmUpdateForm(FrmGetActiveFormID(), 0);
          }
          handled = true;
          break;
        case timeFormatList:
          switch (event->data.popSelect.selection) {
            case 0: timeFormat = tfColonAMPM; break;
            case 1: timeFormat = tfColon24h; break;
            case 2: timeFormat = tfDotAMPM; break;
            case 3: timeFormat = tfDot24h; break;
            case 4: timeFormat = tfComma24h; break;
            default: timeFormat = tfColonAMPM; break;
          }
          PrefSetPreference(prefTimeFormat, timeFormat);
          handled = true;
          break;
        case dateFormatList:
          switch (event->data.popSelect.selection) {
            case 0: dateFormat = dfMDYWithSlashes; break;
            case 1: dateFormat = dfDMYWithSlashes; break;
            case 2: dateFormat = dfDMYWithDots; break;
            case 3: dateFormat = dfDMYWithDashes; break;
            case 4: dateFormat = dfYMDWithSlashes; break;
            case 5: dateFormat = dfYMDWithDots; break;
            case 6: dateFormat = dfYMDWithDashes; break;
            case 7: dateFormat = dfMDYWithDashes; break;
            default: dateFormat = dfMDYWithSlashes; break;
          }
          PrefSetPreference(prefDateFormat, dateFormat);
          handled = true;
          break;
        case ldateFormatList:
          switch (event->data.popSelect.selection) {
            case 0: dateFormat = dfMDYLongWithComma; break;
            case 1: dateFormat = dfDMYLong; break;
            case 2: dateFormat = dfDMYLongWithComma; break;
            case 3: dateFormat = dfDMYLongWithDot; break;
            case 4: dateFormat = dfYMDLongWithSpace; break;
            case 5: dateFormat = dfYMDLongWithDot; break;
            case 6: dateFormat = dfDMYLongNoDay; break;
            case 7: dateFormat = dfMYMed; break;
            case 8: dateFormat = dfMYMedNoPost; break;
            default: dateFormat = dfMDYLongWithComma; break;
          }
          PrefSetPreference(prefLongDateFormat, dateFormat);
          handled = true;
          break;
        case weekStartList:
          PrefSetPreference(prefWeekStartDay, event->data.popSelect.selection);
          handled = true;
          break;
        case numbersList:
          PrefSetPreference(prefNumberFormat, event->data.popSelect.selection);
          handled = true;
          break;
      }
      break;

    default:
      break;
  }

  return handled;
}

static Boolean ApplicationHandleEvent(EventPtr event) {
  FormPtr frm;
  UInt16 formID;
  Boolean handled;

  handled = false;

  switch (event->eType) {
    case frmLoadEvent:
      formID = event->data.frmLoad.formID;
      frm = FrmInitForm(formID);
      FrmSetActiveForm(frm);
      switch (formID) {
        case formatsForm:
          FrmSetEventHandler(frm, FormatsFormHandleEvent);
          break;
        case datetimeForm:
          FrmSetEventHandler(frm, DatetimeFormHandleEvent);
          break;
      }
      handled = true;
      break;
    case frmCloseEvent:
      break;
    default:
      break;
  }

  return handled;
}

static void EventLoop(void) {
  EventType event;
  Err err;

  do {
    EvtGetEvent(&event, evtWaitForever);
    if (SysHandleEvent(&event)) continue;
    if (MenuHandleEvent(NULL, &event, &err)) continue;
    if (ApplicationHandleEvent(&event)) continue;
    FrmDispatchEvent(&event);
  } while (event.eType != appStopEvent);
}

static Int32 compareCountry(void *e1, void *e2, void *otherP) {
  prefs_country_t *c1 = (prefs_country_t *)e1;
  prefs_country_t *c2 = (prefs_country_t *)e2;

  return StrCompare(c1->name, c2->name);
}

static void InitData(void) {
  prefs_data_t *data;
  UInt16 i;

  data = xcalloc(1, sizeof(prefs_data_t));
  data->numLocales = LmGetNumLocales();
  data->countryNames = xcalloc(data->numLocales, sizeof(char *));
  data->countries = xcalloc(data->numLocales, sizeof(prefs_country_t));

  for (i = 0; i < data->numLocales; i++) {
    data->countries[i].index = i;
    LmGetLocaleSetting(i, lmChoiceCountryName, data->countries[i].name, kMaxCountryNameLen+1);
  }

  SysQSortP(data->countries, data->numLocales, sizeof(prefs_country_t), compareCountry, NULL);

  for (i = 0; i < data->numLocales; i++) {
    data->countryNames[i] = data->countries[i].name;
  }

  pumpkin_set_data(data);
}

static void FinishData(void) {
  prefs_data_t *data = pumpkin_get_data();

  xfree(data->countryNames);
  xfree(data->countries);

  xfree(data);
}

UInt32 PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags) {
  switch (cmd) {
    case sysAppLaunchCmdNormalLaunch:
      break;
    default:
      debug(DEBUG_INFO, "Prefs", "launch code %d ignored", cmd);
      return 0;
  }

  InitData();
  FrmGotoForm(datetimeForm);
  EventLoop();
  FrmCloseAllForms();
  FinishData();

  return 0;
}
