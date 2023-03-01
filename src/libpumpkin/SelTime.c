#include <PalmOS.h>

#include "sys.h"
#include "thread.h"
#include "pwindow.h"
#include "vfs.h"
#include "pumpkin.h"
#include "debug.h"
#include "xalloc.h"

typedef struct {
  TimeType startTime, endTime;
  char startBuf[16], endBuf[16];
  Int16 endOfDay;
  Boolean startSelected;
} seltime_module_t;

extern thread_key_t *seltime_key;

int SelTimeInitModule(void) {
  seltime_module_t *module;

  if ((module = xcalloc(1, sizeof(seltime_module_t))) == NULL) {
    return -1;
  }

  thread_set(seltime_key, module);

  return 0;
}

int SelTimeFinishModule(void) {
  seltime_module_t *module = (seltime_module_t *)thread_get(seltime_key);

  if (module) {
    xfree(module);
  }

  return 0;
}

static ControlType *checkControl(FormType *frm, UInt16 controlID) {
  ControlType *ctl;
  UInt16 index;

  index = FrmGetObjectIndex(frm, controlID);
  ctl = FrmGetObjectPtr(frm, index);

  return CtlGetValue(ctl) == 0 ? NULL : ctl;
}

static ControlType *selectedControl(FormType *frm) {
  ControlType *ctl;

  if ((ctl = checkControl(frm, 12105)) != NULL) return ctl;
  if ((ctl = checkControl(frm, 12106)) != NULL) return ctl;
  if ((ctl = checkControl(frm, 12107)) != NULL) return ctl;

  return NULL;
}

static Boolean format24H(void) {
  return PrefGetPreference(prefTimeFormat) == tfColon24h;
}

static Boolean SelectOneTimeHandleEvent(EventType *eventP) {
  FormType *frm;
  ControlType *ctl;
  Int16 n;
  char *s;
  Boolean is24h, up, handled = false;

  is24h = format24H();

  switch (eventP->eType) {
    case frmOpenEvent:
      frm = FrmGetActiveForm();
      FrmDrawForm(frm);
      handled = true;
      break;
    case ctlSelectEvent:
      frm = FrmGetActiveForm();
      up = eventP->data.ctlSelect.controlID == 12108;
      switch (eventP->data.ctlSelect.controlID) {
        case 12108: // up
        case 12109: // down
          if ((ctl = selectedControl(frm)) != NULL) {
            switch (ctl->id) {
              case 12105: // hour
                if ((s = (char *)CtlGetLabel(ctl)) != NULL) {
                  n = sys_atoi(s);
                  if (up) {
                    n = (n == (is24h ? 23 : 12)) ? 0 : n+1;
                  } else {
                    n = (n == 0) ? (is24h ? 23 : 12) : n-1;
                  }
                  StrPrintF(s, "%02d", n);
                  CtlSetLabel(ctl, s);
                }
                break;
              case 12106: // minute (high digit)
                if ((s = (char *)CtlGetLabel(ctl)) != NULL) {
                  n = sys_atoi(s);
                  if (up) {
                    n = (n == 5) ? 0 : n+1;
                  } else {
                    n = (n == 0) ? 5 : n-1;
                  }
                  StrPrintF(s, "%d", n);
                  CtlSetLabel(ctl, s);
                }
                break;
              case 12107: // minute (low digit)
                if ((s = (char *)CtlGetLabel(ctl)) != NULL) {
                  n = sys_atoi(s);
                  if (up) {
                    n = (n == 9) ? 0 : n+1;
                  } else {
                    n = (n == 0) ? 9 : n-1;
                  }
                  StrPrintF(s, "%d", n);
                  CtlSetLabel(ctl, s);
                }
                break;
            }
          }
          handled = true;
          break;
        case 12105:
        case 12106:
        case 12107:
        case 12110:
        case 12111:
          handled = true;
          break;
      }
      break;
    default:
      break;
  }

  return handled;
}

Boolean SelectOneTime(Int16 *hour, Int16 *minute, const Char *titleP) {
  FormType *frm, *previous;
  ControlType *hctl, *m1ctl, *m2ctl, *amctl, *pmctl;
  UInt16 index;
  Boolean am, is24h;
  char *s, hbuf[8], mbuf1[8], mbuf2[8];
  Boolean r = false;

  if (hour == NULL || minute == NULL) return false;

  is24h = format24H();

  if (*hour < 0 || *hour > 23) {
    *hour = 0;
    am = true;
  } else if (*hour < 12) {
    am = true;
  } else {
    if (!is24h) *hour -= 12;
    am = false;
  }

  frm = FrmInitForm(12100);
  if (titleP) FrmSetTitle(frm, (char *)titleP);

  index = FrmGetObjectIndex(frm, 12105); // hour
  hctl = FrmGetObjectPtr(frm, index);
  hctl->attr.on = 1;
  StrPrintF(hbuf, "%02d", *hour);
  CtlSetLabel(hctl, hbuf);

  index = FrmGetObjectIndex(frm, 12106); // minute (high digit)
  m1ctl = FrmGetObjectPtr(frm, index);
  StrPrintF(mbuf1, "%d", *minute / 10);
  CtlSetLabel(m1ctl, mbuf1);

  index = FrmGetObjectIndex(frm, 12107); // minute (low digit)
  m2ctl = FrmGetObjectPtr(frm, index);
  StrPrintF(mbuf2, "%d", *minute % 10);
  CtlSetLabel(m2ctl, mbuf2);

  index = FrmGetObjectIndex(frm, 12110); // AM
  amctl = FrmGetObjectPtr(frm, index);
  if (is24h) {
    FrmHideObject(frm, index);
  } else if (am) {
    CtlSetValue(amctl, 1);
  }

  index = FrmGetObjectIndex(frm, 12111); // PM
  pmctl = FrmGetObjectPtr(frm, index);
  if (is24h) {
    FrmHideObject(frm, index);
  } else if (!am) {
    CtlSetValue(pmctl, 1);
  }

  FrmSetEventHandler(frm, SelectOneTimeHandleEvent);
  previous = FrmGetActiveForm();

  if (FrmDoDialog(frm) == 12112) { // "OK" button
    if ((s = (char *)CtlGetLabel(hctl)) != NULL) {
      *hour = sys_atoi(s);
      am = CtlGetValue(amctl);
      if (is24h && !am) *hour += 12;
    }
    if ((s = (char *)CtlGetLabel(m1ctl)) != NULL) {
      *minute = sys_atoi(s) * 10;
    }
    if ((s = (char *)CtlGetLabel(m2ctl)) != NULL) {
      *minute += sys_atoi(s);
    }
    r = true;
  }

  FrmDeleteForm(frm);
  FrmSetActiveForm(previous);

  return r;
}

static void drawLists(FormType *frm, TimeType *timeP) {
  UInt16 index;
  ListType *list;

  index = FrmGetObjectIndex(frm, 10206);
  list = FrmGetObjectPtr(frm, index);
  if (timeP->hours != 0xFF) {
    LstSetSelection(list, timeP->hours);
    LstMakeItemVisible(list, timeP->hours);
  } else {
    LstSetSelection(list, noListSelection);
  }
  LstDrawList(list);

  index = FrmGetObjectIndex(frm, 10207);
  list = FrmGetObjectPtr(frm, index);
  if (timeP->minutes != 0xFF) {
    LstSetSelection(list, timeP->minutes / 5);
    LstMakeItemVisible(list, timeP->minutes / 5);
  } else {
    LstSetSelection(list, noListSelection);
  }
  LstDrawList(list);
}

static void setTime(FormType *frm, UInt16 id, TimeType *timeP, Boolean is24h, char *buf, Boolean set) {
  UInt16 index;
  ControlType *ctl;

  index = FrmGetObjectIndex(frm, id);
  ctl = FrmGetObjectPtr(frm, index);
  if (timeP->hours != 0xFF && timeP->minutes != 0xFF) {
    TimeToAscii(timeP->hours, timeP->minutes, is24h ? tfColon24h : tfColonAMPM, buf);
  } else {
    StrCopy(buf, "");
  }
  CtlSetLabel(ctl, buf);
  CtlSetValue(ctl, set ? 1 : 0);
}

static Boolean SelectTimeHandleEvent(EventType *eventP) {
  seltime_module_t *module = (seltime_module_t *)thread_get(seltime_key);
  FormType *frm;
  ControlType *ctl;
  UInt16 index;
  Boolean is24h, handled = false;

  is24h = format24H();
  frm = FrmGetActiveForm();

  switch (eventP->eType) {
    case frmOpenEvent:
      FrmDrawForm(frm);
      index = FrmGetObjectIndex(frm, 10212);
      FrmShowObject(frm, index);
      handled = true;
      break;

    case lstSelectEvent:
      switch (eventP->data.lstSelect.listID) {
        case 10206:
          if (module->startSelected) {
            // start time is selected
            module->startTime.hours = eventP->data.lstSelect.selection;
            setTime(frm, 10204, &module->startTime, is24h, module->startBuf, true);
          } else {
            // end time is selected
            module->endTime.hours = eventP->data.lstSelect.selection;
            setTime(frm, 10205, &module->endTime, is24h, module->endBuf, true);
          }
          break;
        case 10207:
          if (module->startSelected) {
            // start time is selected
            module->startTime.minutes = eventP->data.lstSelect.selection * 5;
            setTime(frm, 10204, &module->startTime, is24h, module->startBuf, true);
          } else {
            // end time is selected
            module->endTime.minutes = eventP->data.lstSelect.selection * 5;
            setTime(frm, 10205, &module->endTime, is24h, module->endBuf, true);
          }
          break;
      }
      break;

    case ctlSelectEvent:
      switch (eventP->data.ctlSelect.controlID) {
        case 10204:
          index = FrmGetObjectIndex(frm, 10204);
          ctl = FrmGetObjectPtr(frm, index);
          if (CtlGetValue(ctl)) {
            index = FrmGetObjectIndex(frm, 10205);
            ctl = FrmGetObjectPtr(frm, index);
            CtlSetValue(ctl, 0);
            drawLists(frm, &module->startTime);
          }
          module->startSelected = true;
          //handled = true;
          break;
        case 10205:
          index = FrmGetObjectIndex(frm, 10205);
          ctl = FrmGetObjectPtr(frm, index);
          if (CtlGetValue(ctl)) {
            index = FrmGetObjectIndex(frm, 10204);
            ctl = FrmGetObjectPtr(frm, index);
            CtlSetValue(ctl, 0);
            drawLists(frm, &module->endTime);
          }
          module->startSelected = false;
          //handled = true;
          break;
        case 10210: // No Time
          module->startTime.hours = 0xFF;
          module->startTime.minutes = 0xFF;
          module->endTime.hours = 0xFF;
          module->endTime.minutes = 0xFF;
          break;
        case 10212: // All day
          module->startTime.hours = 8;
          module->startTime.minutes = 0;
          module->endTime.hours = module->endOfDay;
          module->endTime.minutes = 0;
          break;
      }
      break;
    default:
      break;
  }

  return handled;
}

Boolean SelectTime(TimeType *startTimeP, TimeType *endTimeP, Boolean untimed, const Char *titleP, Int16 startOfDay, Int16 endOfDay, Int16 startOfDisplay) {
  seltime_module_t *module = (seltime_module_t *)thread_get(seltime_key);
  FormType *frm, *previous;
  UInt16 index, i;
  ListType *list;
  char hours[24][8], *choices[24];
  Boolean is24h, r = false;

  // XXX startOfDisplay is not being used

  frm = FrmInitForm(10200);
  FrmSetTitle(frm, (char *)titleP);

  // control id 10204 font 1 style 1 attr 0xC100 text "12:00 pm  " at (10,30,50,13)
  // control id 10205 font 1 style 1 attr 0xC100 text "12:00 pm"   at (10,61,50,13)
  // list id 10206 numItems  0 visibleItems 12 usable 1
  // list id 10207 numItems 12 visibleItems 12 usable 1
  // control id 10208 font 0 style 0 attr 0xC900 text "OK"      at (5,138,37,13)
  // control id 10209 font 0 style 0 attr 0xC900 text "Cancel"  at (48,138,37,13)
  // control id 10210 font 1 style 0 attr 0xC900 text "No Time" at (10,115,50,13)
  // control id 10212 font 1 style 0 attr 0x4900 text "All Day" at (10,95,50,13)

  is24h = format24H();

  for (i = 0; i < 24; i++) {
    if (is24h) {
      StrPrintF(hours[i], "%2d", i);
    } else {
      switch (i) {
        case  0:
          StrCopy(hours[i], "12A");
          break;
        case 12:
          StrCopy(hours[i], "12P");
          break;
        default:
          StrPrintF(hours[i], "%2d", i < 12 ? i : i - 12);
          break;
      }
    }
    choices[i] = hours[i];
  }

  module->startSelected = true;

  if (untimed) {
    module->startTime.hours = 0xFF;
    module->startTime.minutes = 0xFF;

    module->endTime.hours = 0xFF;
    module->endTime.minutes = 0xFF;
  } else {
    module->startTime.hours = startTimeP->hours;
    module->startTime.minutes = (startTimeP->minutes / 5) * 5;
    setTime(frm, 10204, &module->startTime, is24h, module->startBuf, module->startSelected);

    module->endTime.hours = endTimeP->hours;
    module->endTime.minutes = (endTimeP->minutes / 5) * 5;
    setTime(frm, 10205, &module->endTime, is24h, module->endBuf, !module->startSelected);
  }

  if (startOfDay < 0) startOfDay = 0;
  else if (startOfDay > 12) startOfDay = 12;

  if (endOfDay < 0) endOfDay = 0;
  else if (endOfDay > 23) endOfDay = 23;
  module->endOfDay = endOfDay;

  index = FrmGetObjectIndex(frm, 10206);
  list = FrmGetObjectPtr(frm, index);
  LstSetListChoices(list, choices, 24);
  LstSetTopItem(list, startOfDay);
  if (untimed) {
    LstSetSelection(list, noListSelection);
  } else {
    LstSetSelection(list, module->startTime.hours);
    LstMakeItemVisible(list, module->startTime.hours);
  }

  if (untimed) {
    LstSetSelection(list, noListSelection);
  } else {
    index = FrmGetObjectIndex(frm, 10207);
    list = FrmGetObjectPtr(frm, index);
    LstSetSelection(list, module->startTime.minutes / 5);
    LstMakeItemVisible(list, module->startTime.minutes / 5);
  }

  FrmSetEventHandler(frm, SelectTimeHandleEvent);
  previous = FrmGetActiveForm();

  switch (FrmDoDialog(frm)) {
    case 10208: // "OK" button
    case 10210: // "No time" button
    case 10212: // "All day" button
      startTimeP->hours = module->startTime.hours;
      startTimeP->minutes = module->startTime.minutes;
      endTimeP->hours = module->endTime.hours;
      endTimeP->minutes = module->endTime.minutes;
      r = true;
  }

  FrmDeleteForm(frm);
  FrmSetActiveForm(previous);

  return r;
}

Boolean SelectTimeV33(TimeType *startTimeP, TimeType *endTimeP, Boolean untimed, const Char *titleP, Int16 startOfDay) {
  return SelectTime(startTimeP, endTimeP, untimed, titleP, startOfDay, 23, 0);
}
