#include <PalmOS.h>

#include <time.h>
#include <sys/time.h>

#include "thread.h"
#include "pwindow.h"
#include "sys.h"
#include "vfs.h"
#include "mem.h"
#include "bytes.h"
#include "pumpkin.h"
#include "debug.h"
#include "xalloc.h"

typedef struct {
  char *name;
  UInt16 country;
  Int16 tz;
} tz_data_t;

static void SelectTimeZoneDrawItem(Int16 itemNum, RectangleType *bounds, Char **itemsText) {
  tz_data_t *item;
  UInt16 tz, n;
  char s, buf[16];

  if (bounds && itemsText) {
    item = (tz_data_t *)itemsText[itemNum];
    if (item && item->name) {
      if (item->tz < 0) {
        tz = -item->tz;
        s = '-';
      } else if (item->tz > 0) {
        tz = item->tz;
        s = '+';
      } else {
        tz = item->tz;
        s = ' ';
      }
      snprintf(buf, sizeof(buf)-1, "%c%d:%02d", s, tz/60, tz%60);
      n = StrLen(buf);
      WinDrawChars(buf, n, bounds->topLeft.x + bounds->extent.x - FntCharsWidth(buf, n) - 10, bounds->topLeft.y);
      WinDrawChars(item->name, StrLen(item->name), bounds->topLeft.x+1, bounds->topLeft.y);
    }
  }
}

Boolean SelectTimeZone(Int16 *ioTimeZoneP, LmLocaleType *ioLocaleInTimeZoneP, const Char *titleP, Boolean showTimes, Boolean anyLocale) {
  FormType *frm, *previous;
  ListType *list;
  FieldType *fld;
  MemHandle h;
  UInt16 i, j, max, maxc, maxs, index;
  UInt16 *tz, *country;
  UInt8 *p;
  tz_data_t **choices;
  char *prefix, *str;
  char buf[256];
  char currentTime[16], newTime[16];
  Boolean r = false;

  if (ioTimeZoneP == NULL || (ioLocaleInTimeZoneP == NULL && !anyLocale)) return false;

  debug(DEBUG_INFO, "DateTime", "SelectTimeZone timeZone=%d", *ioTimeZoneP);
  frm = FrmInitForm(13400);
  if (titleP) FrmSetTitle(frm, (char *)titleP);

  index = FrmGetObjectIndex(frm, 13403);
  if ((list = (ListType *)FrmGetObjectPtr(frm, index)) != NULL) {
    // DmGetResource('tSTR', 13419)  0x346B  string 0x5e 0x31 0x72 0x00    XXX what is this ?

    // DmGetResource('wrdl', 13400)  0x3458  71 words, timezone in minutes (can be negative)
    // 00 47 01 0e 02 58 02 3a 01 e0 00 3c 01 68 00 3c
    // ff 4c ff 10 fe 98 fe d4 fe 5c fe 20 ff 10 01 e0
    // 00 3c 00 3c 00 78 fd a8 00 3c 00 3c 00 78 01 e0
    // 00 3c 01 4a 00 d2 00 00 00 78 00 3c 02 1c 02 1c
    // 00 b4 fe 98 fe 20 ff 88 01 59 00 3c 02 94 02 d0
    // 00 3c 01 2c 01 e0 00 3c 00 00 ff c4 00 b4 fd 6c
    // 00 b4 01 e0 00 78 00 3c 00 3c 00 3c 01 e0 01 a4
    // 00 78 00 f0 00 00 fd e4 fe 98 fe d4 fd a8 fe 5c
    // fe 20 00 00 00 3c 01 a4 02 1c 01 e0 01 e0 fe 5c

    choices = NULL;
    max = 0;

    if ((h = DmGetResource(wrdListRscType, 13400)) != NULL_HANDLE) {
      if ((tz = MemHandleLock(h)) != NULL) {
        max = tz[0];
        if (max > 0) {
          if ((choices = xcalloc(max, sizeof(tz_data_t *))) != NULL) {
            for (j = 0; j < max; j++) {
               if ((choices[j] = xcalloc(1, sizeof(tz_data_t))) != NULL) {
                 choices[j]->tz = tz[j+1];
               }
            }
          }
        }
        MemHandleUnlock(h);
      }
      DmReleaseResource(h);
    }

    // DmGetResource('wrdl', 13401)  0x3459  71 words, country code
    // 00 47 00 23 00 00 00 00 00 00 00 01 00 31 00 02
    // 00 03 00 04 00 04 00 04 00 04 00 04 00 46 00 1c
    // 00 4e 00 05 00 06 00 ad 00 07 00 08 00 69 00 09
    // 00 73 00 18 00 77 00 0b 00 74 00 0c 00 0d 00 1a
    // 00 91 00 0f 00 0f 00 4b 00 a7 00 10 00 a2 00 11
    // 00 12 00 af 00 1d 00 b0 00 b4 00 b4 00 ba 00 e8
    // 00 bc 00 1e 00 ec 00 13 00 14 00 15 00 20 00 1f
    // 00 db 00 22 00 16 00 17 00 17 00 17 00 17 00 17
    // 00 17 00 0a 00 0e 00 19 00 19 00 19 00 1b 00 0f

    if ((h = DmGetResource(wrdListRscType, 13401)) != NULL_HANDLE) {
      if ((country = MemHandleLock(h)) != NULL) {
        maxc = tz[0];
        if (maxc == max) {
          for (j = 0; j < max; j++) {
            if (choices && choices[j]) {
              choices[j]->country = country[j+1];
            }
          }
        }
        MemHandleUnlock(h);
      }
      DmReleaseResource(h);
    }

    // DmGetResource('tSTL', 13400)  0x3458  71 strings with timezone names, first = "Afghanistan", last = "Mexico (Mountain)"

    if ((h = DmGetResource(strListRscType, 13400)) != NULL_HANDLE) {
      if ((p = MemHandleLock(h)) != NULL) {
        i = 0;
        i += pumpkin_getstr(&prefix, p, 0);
        i += get2b(&maxs, p, i);
        if (maxs == max) {
          for (j = 0; j < max; j++) {
            i += pumpkin_getstr(&str, p, i);
            if (choices && choices[j]) {
              snprintf(buf, sizeof(buf)-1, "%s%s", prefix, str);
              choices[j]->name = xstrdup(buf);
            }
          }
        }
        MemHandleUnlock(h);
      }
      DmReleaseResource(h);
    }
  }

  if (list && choices) {
    LstSetDrawFunction(list, SelectTimeZoneDrawItem);
    LstSetListChoices(list, (char **)choices, max);
    for (i = 0; i < max; i++) {
      if (choices[i]->tz == *ioTimeZoneP) {
        if (anyLocale || ioLocaleInTimeZoneP == NULL || ioLocaleInTimeZoneP->country == choices[i]->country) {
          LstSetSelection(list, i);
          LstMakeItemVisible(list, i);
          break;
        }
      }
    }
  }

  // 13401: field
  // 13402: field
  // 13405: label "New time"
  // 13407: label "Current time"

  if (showTimes) {
     StrCopy(currentTime, "XXX"); // XXX fill currentTime
     StrCopy(newTime, "XXX"); // XXX fill newTime
    index = FrmGetObjectIndex(frm, 13401);
    fld = (FieldType *)FrmGetObjectPtr(frm, index);
    FldSetTextPtr(fld, currentTime);
    index = FrmGetObjectIndex(frm, 13402);
    fld = (FieldType *)FrmGetObjectPtr(frm, index);
    FldSetTextPtr(fld, newTime);

  } else {
    index = FrmGetObjectIndex(frm, 13401);
    FrmSetUsable(frm, index, false);
    index = FrmGetObjectIndex(frm, 13402);
    FrmSetUsable(frm, index, false);
    index = FrmGetObjectIndex(frm, 13405);
    FrmSetUsable(frm, index, false);
    index = FrmGetObjectIndex(frm, 13407);
    FrmSetUsable(frm, index, false);

    // XXX grow list to use space of hidden controls
  }

  previous = FrmGetActiveForm();
  if (FrmDoDialog(frm) == 13412) { // "OK" button
    if (list && choices) {
      i = LstGetSelection(list);
      *ioTimeZoneP = choices[i]->tz;
      if (!anyLocale && ioLocaleInTimeZoneP) {
        ioLocaleInTimeZoneP->language = 0; // XXX
        ioLocaleInTimeZoneP->country = choices[i]->country;
      }
    }
    r = true;
  }

  if (list) {
    LstSetListChoices(list, NULL, 0);
  }

  FrmDeleteForm(frm);
  FrmSetActiveForm(previous);

  if (choices) {
    for (i = 0; i < max; i++) {
      if (choices[i]) {
        if (choices[i]->name) xfree(choices[i]->name);
        xfree(choices[i]);
      }
    }
    xfree(choices);
  }

  return r;

} 
