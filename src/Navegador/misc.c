#include <PalmOS.h>
#include <BmpGlue.h>

#include "sys.h"
#include "gps.h"
#include "app.h"
#include "main.h"
#include "format.h"
#include "list.h"
#include "log.h"
#include "map.h"
#include "mapdecl.h"
#include "datum.h"
#include "gui.h"
#include "ddb.h"
#include "error.h"
#include "MathLib.h"
#include "misc.h"

#include "debug.h"

void GetUTC(unsigned char *day, unsigned char *month, unsigned short *year,  
            unsigned char *hour, unsigned char *minute, unsigned char *second)
{
  Int32 seconds, timezone, daylight_saving;
  DateTimeType datetime;
  
  seconds = TimGetSeconds();
  timezone = ((Int32)PrefGetPreference(prefTimeZone)) * 60; 
  daylight_saving = ((Int32)PrefGetPreference(prefDaylightSavingAdjustment)) * 60;

  seconds -= timezone;
  seconds -= daylight_saving;
  TimSecondsToDateTime(seconds, &datetime);
            
  if (day) *day = datetime.day;
  if (month) *month = datetime.month;
  if (year) *year = datetime.year;
  if (hour) *hour = datetime.hour;
  if (minute) *minute = datetime.minute;
  if (second) *second = datetime.second;
}

void GetCoord(FormPtr frm, CoordType *coord)
{   
  char buf[32];
  double lat, lon, height;
  AppPrefs *prefs = GetPrefs();
    
  lat = coord->latitude;
  lon = coord->longitude;
  height = coord->height;
  
  DatumConvert(&lon, &lat, &height);

  gStrPrintF(buf, lat);
  FldInsertStr(frm, slatFld, buf);
  gStrPrintF(buf, lon);
  FldInsertStr(frm, slongFld, buf);
    
  if (prefs->unit_system == UNIT_METRIC)
    height *= 0.001;
  else
    height *= (0.001 * KM_TO_MILE);
  fStrPrintF(buf, height, 2, 4);
  FldInsertStr(frm, heightFld, buf);
}

void DrawBmp(UInt16 id, Int16 x, Int16 y, Boolean centered)
{
  MemHandle h;   
  BitmapPtr bmp; 
  UInt16 bwidth, bheight, rowBytes;

  if ((h = DmGetResource(bitmapRsc, id)) == NULL)
    return;

  if ((bmp = (BitmapPtr)MemHandleLock(h)) == NULL) {
    DmReleaseResource(h);
    return;
  }

  if (centered) {
    BmpGlueGetDimensions(bmp, (Coord *)&bwidth, (Coord *)&bheight, &rowBytes);
    WinDrawBitmap(bmp, x-bwidth/2, y-bheight/2);
  } else {
    WinDrawBitmap(bmp, x, y);
  }

  MemHandleUnlock(h);
  DmReleaseResource(h);
}

Int16 SetField(FormPtr frm, UInt16 id, char *buf)
{
  FieldPtr fld;
  
  fld = (FieldPtr)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, id));
  FldSetTextPtr(fld, buf);
  FldDrawField(fld);
  
  return 0;
}

void FldInsertStr(FormPtr frm, UInt16 id, char *str)
{
  FieldPtr fld;
  UInt16 index, len;
  FieldAttrType attr;
  Boolean old;

  index = FrmGetObjectIndex(frm, id);
  fld = (FieldPtr)FrmGetObjectPtr(frm, index);
  if (fld == NULL) return;

  FldGetAttributes(fld, &attr);
  old = attr.editable;
  attr.editable = true;
  FldSetAttributes(fld, &attr);

  len = FldGetTextLength(fld);
  if (len) {
    FldDelete(fld, 0, len);
  }
  if (str && str[0]) {
    FldInsert(fld, str, StrLen(str));
  } else {
    FldInsert(fld, "", 0);
  }

  attr.editable = old;
  FldSetAttributes(fld, &attr);
}

void DrawFld(FormPtr frm, UInt16 id)
{
  FieldPtr fld;
  UInt16 index;

  index = FrmGetObjectIndex(frm, id);
  fld = (FieldPtr)FrmGetObjectPtr(frm, index);
  if (fld != NULL)
    FldDrawField(fld);
}

void FldSetStr(FormPtr frm, UInt16 id, char *s)
{
  FieldPtr fld;
  UInt16 index;
  Int16 size, max;
  MemHandle h;
  MemPtr p;
  
  index = FrmGetObjectIndex(frm, id);
  fld = (FieldPtr)FrmGetObjectPtr(frm, index);
  if (!fld || ! s)
    return;

  max = FldGetMaxChars(fld);
  size = StrLen(s)+1;
  if (size > max) size = max;
  h = MemHandleNew(size);
  if (!h)
    return;
  p = MemHandleLock(h);
  if (!p) {
    MemHandleFree(h);
    return;
  }
  MemMove((char *)p, s, size);
  ((char *)p)[size-1] = 0;
  MemHandleUnlock(h);
  FldSetTextHandle(fld, h);
  FldSetInsertionPoint(fld, 0);
  FldSetInsPtPosition(fld, 0);
}

Err DmCheckAndWrite(MemPtr dst, UInt32 offset, MemPtr src, UInt32 bytes)
{
  Err err;

  if ((err = DmWriteCheck(dst, offset, bytes)) != 0)
    return err;
  return DmWrite(dst, offset, src, bytes);
}

Boolean GetFloat(FieldPtr fld, double *value, double min, double max)
{
  double tmp;
  char *s, *i;
            
  s = FldGetTextPtr(fld);
  if (!s || !s[0])
    return false;
            
  i = StrChr(s, ':');
  if (i == NULL)
    tmp = sys_atof(s);
  else {
    char buf[16];
    double d;
    Int16 len = i-s;
    if (len)
      StrNCopy(buf, s, len);
    buf[len] = 0;
    d = sys_atof(&i[1])/60.0;
    tmp = sys_atof(buf);
    tmp += tmp < 0.0 ? -d : d;
  }

  if (tmp < min || tmp > max)
    return false;

  *value = tmp;
  return true;
}

Boolean GetUInt(FormPtr frm, UInt16 id, UInt16 *value, UInt16 min, UInt16 max)
{
  FieldPtr fld;
  UInt16 tmp;
  char *s;

  fld = (FieldPtr)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, id));
  if (!fld)
    return false;
  s = FldGetTextPtr(fld);
  if (!s || !s[0])
    return false;

  tmp = StrAToI(s);
  if (tmp < min || tmp > max)
    return false;

  *value = tmp;
  return true;
}

void ListDrawData(Int16 itemNum, RectangleType *bounds, char **itemsText)
{  
  RectangleType rect;
  Int16 x, y, len, namedx;
  char *s;
 
  namedx = TAM_NAME*FntCharWidth('a');

  MemMove(&rect, bounds, sizeof(RectangleType));
  rect.extent.x = namedx-1;
  WinSetClip(&rect);
  x = rect.topLeft.x;
  y = rect.topLeft.y;
  s = itemsText[itemNum];
  len = StrLen(s);
  WinDrawChars(s, len, x, y);
  if (s[len+1]) {
    s = &s[len+1];
    len = StrLen(s);
    rect.extent.x = bounds->extent.x-10;
    WinSetClip(&rect);
    WinDrawChars(s, len, x+namedx+1, y);
  }
  WinResetClip();
}

char *GetString(char *title, char *value)
{
  FormType *frm;
  FieldType *fld;
  UInt16 index;
  char *s;
  static char buf[64];

  frm = FrmInitForm(InputForm);
  FrmSetTitle(frm, title);
  index = FrmGetObjectIndex(frm, inputFld);

  if (value && value[0]) {
    FldInsertStr(frm, inputFld, value);
    fld = (FieldPtr)FrmGetObjectPtr(frm, index);
    FldSetSelection(fld, 0, StrLen(value));
  }

  FrmSetFocus(frm, index);
               
  if (FrmDoDialog(frm) == okBtn) {
    fld = (FieldPtr)FrmGetObjectPtr(frm, index);
    buf[0] = 0;
    if ((s = FldGetTextPtr(fld)) != NULL)
      StrNCopy(buf, s, sizeof(buf)-1);
    FrmDeleteForm(frm);
    return buf[0] ? buf : NULL;
  }

  FrmDeleteForm(frm);
  return NULL;
}

void InitPrefs(AppPrefs *prefs)
{
  RGBColorType rgb;

  prefs->coord.latitude = -19.9;
  prefs->coord.longitude = -43.9;
  prefs->coord.height = 0.0;
  prefs->datum = 0;
  prefs->log_enabled = 0;
  prefs->log_beep = 0;
  prefs->unit_system = UNIT_METRIC;
  prefs->action[0] = ACTION_NEXT;
  prefs->action[1] = ACTION_NONE;
  prefs->action[2] = ACTION_NONE;
  prefs->action[3] = ACTION_NONE;
  prefs->speed_limit = 80.0/3.6;	// 80 km/h
  prefs->speed_warning = 0;
  prefs->avg_speed = 0.0;
  prefs->max_speed = 0.0;
  prefs->proximity_limit = 100.0;	// 100 m
  prefs->proximity_alarm = 0;
  prefs->distance = 0.0;
  prefs->moving_time = 0;
  prefs->start_time = TimGetSeconds();
  prefs->serial_port = 0;
  prefs->serial_baud = 0;
  prefs->protocol = PROTOCOL_NMEA;
  prefs->autopoint = 1;
  prefs->caseless = 0;
  prefs->exact = 1;
  prefs->density = 0;
  prefs->track = 0;
  prefs->showtrack = 0;
  prefs->current_zoom = DEFAULT_ZOOM;
  prefs->locked = 0;
  prefs->center_lat = prefs->coord.latitude;
  prefs->center_lon = prefs->coord.longitude;
  prefs->display = MainForm;
  prefs->capture = 0;
  prefs->mapfont = 0;
  prefs->proximity_sort = 0;
  prefs->mapname[0] = 0;
  prefs->nethost[0] = 0;
  prefs->neturl[0] = 0;
  prefs->netport = 0;
  prefs->mapdetail = 2;
  prefs->route = 0;

  rgb.r = 128; rgb.g = 0; rgb.b = 128;
  prefs->positionColor = WinRGBToIndex(&rgb);

  rgb.r = 0; rgb.g = 0; rgb.b = 128;
  prefs->currentColor = WinRGBToIndex(&rgb);

  rgb.r = 0; rgb.g = 128; rgb.b = 0;
  prefs->storedColor = WinRGBToIndex(&rgb);

  rgb.r = 0; rgb.g = 255; rgb.b = 255;
  prefs->selectedColor = WinRGBToIndex(&rgb);

  rgb.r = 255; rgb.g = 255; rgb.b = 0;
  prefs->targetColor = WinRGBToIndex(&rgb);
}

void ExpandText(WaypointType *p, char *mask, char *result, int max)
{
  Int16 i, j, n;
  char buf[32];

  for (i = 0, j = 0; mask[i] && j < max; i++) {
    if (mask[i] == '%') {
      i++;
      buf[0] = 0;

      switch (mask[i]) {
        case '%':
          buf[0] = '%';
          buf[1] = 0;
          break;
        case 'a':
          fStrPrintF(buf, p->coord.latitude, 1, 6);
          break;
        case 'o':
          fStrPrintF(buf, p->coord.longitude, 1, 6);
          break;
        case 'e':
          fStrPrintF(buf, p->coord.height, 1, 0);
          break;
        case 'd':
          FormatDate(buf, p->date.day, p->date.month, p->date.year, false);
          break;
        case 't':
          FormatTime(buf, p->date.hour, p->date.minute, p->date.second, false);
          break;
        case 's':
          StrPrintF(buf, "%d", p->symbol);
          break;
        case 'n':
          StrCopy(buf, p->name);
      }

      n = StrLen(buf);
      if ((j + n) >= max)
        n = max - j - 1;
      if (n > 0) {
        StrNCopy(&result[j], buf, n);
        j += n;
      }
    } else
      result[j++] = mask[i];
  }

  result[j] = 0;
}

Int16 comparename(void *e1, void *e2, Int32 other)
{
  return StrCompare(((RecordType *)e1)->name, ((RecordType *)e2)->name);
}

Int16 comparedistance(void *e1, void *e2, Int32 other)
{
  RecordType *r1 = (RecordType *)e1;
  RecordType *r2 = (RecordType *)e2;
  float d1 = (float)r1->dynamic;
  float d2 = (float)r2->dynamic;
  if (d1 < d2) return -1;
  if (d1 > d2) return 1;
  return 0;
}
