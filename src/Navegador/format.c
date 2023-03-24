#include <PalmOS.h>
 
#include "sys.h"
#include "format.h"
#include "gps.h"
#include "app.h"
#include "main.h"

#include "debug.h"

typedef union {   
  double d;
  UInt32 ul[2];
  UInt16 us[4];
  UInt8  ub[8];
} MyFlpCompDouble;

#define NUM_DIGITS      15
#define MIN_FLOAT       6
#define ROUND_FACTOR    1.0000000000000005

void FormatDateTime(char *buf, UInt8 day, UInt8 month, UInt16 year,
                    UInt8 hour, UInt8 minute, UInt8 second)
{
  Int16 i;
  FormatDate(buf, day, month, year, true);
  i = StrLen(buf);
  buf[i++] = ' ';
  buf[i++] = ' ';
  FormatTime(&buf[i], hour, minute, second, true);
}

void FormatDate(char *buf, UInt8 day, UInt8 month, UInt16 year, Boolean sep)
{
  Int16 i = 0;
  buf[i++] = '0'+year/1000;
  year = year % 1000;
  buf[i++] = '0'+year/100;
  year = year % 100;
  buf[i++] = '0'+year/10;
  buf[i++] = '0'+year%10;
  if (sep) buf[i++] = '-';
  buf[i++] = '0'+month/10;
  buf[i++] = '0'+month%10;
  if (sep) buf[i++] = '-';
  buf[i++] = '0'+day/10;
  buf[i++] = '0'+day%10;
  buf[i++] = '\0';
}

void FormatTime(char *buf, UInt8 hour, UInt8 minute, UInt8 second, Boolean sep)
{
  Int16 i = 0;
  buf[i++] = '0'+hour/10;
  buf[i++] = '0'+hour%10;
  if (sep) buf[i++] = ':';
  buf[i++] = '0'+minute/10;
  buf[i++] = '0'+minute%10;
  if (sep) buf[i++] = ':';
  buf[i++] = '0'+second/10;
  buf[i++] = '0'+second%10;
  buf[i++] = '\0';
}

void FormatDistance(char *buf, double distance)
{
  AppPrefs *prefs;
  Int16 n1, n2;
  char *unit;

  // distance: in meters

  prefs = GetPrefs();
  if (prefs->unit_system == UNIT_METRIC) {
    if (distance < 1000.0) {
      n1 = 1, n2 = 0;
      unit = " m";
    } else if (distance < 10000.0) {
      distance *= 0.001;
      n1 = 1, n2 = 2;
      unit = " km";
    } else {
      distance *= 0.001;
      n1 = 1, n2 = 1;
      unit = " km";
    }
  } else {
    distance *= M_TO_FEET;
    if (distance < MILE_TO_FEET) {
      n1 = 1, n2 = 0;
      unit = " ft";
    } else if (distance < 10.0*MILE_TO_FEET) {
      distance /= MILE_TO_FEET;
      n1 = 1, n2 = 2;
      unit = " mi";
    } else {
      distance /= MILE_TO_FEET;
      n1 = 1, n2 = 1;
      unit = " mi";
    }
  }
      
  buf[0] = 0;
  fStrPrintF(buf, distance, n1, n2);
  StrCat(buf, unit);
}

void FormatArea(char *buf, double side)
{
  AppPrefs *prefs;
  Int16 n1, n2;
  char *unit;

  // side: in meters

  prefs = GetPrefs();
  if (prefs->unit_system == UNIT_METRIC) {
/*
    if (side < 1000.0) {
      n1 = 1, n2 = 0;
      unit = " m\262";
    } else if (side < 100000.0) {
      side *= 0.001;
      n1 = 1, n2 = 2;
      unit = " km\262";
    } else {
      side *= 0.001;
      n1 = 1, n2 = 1;
      unit = " km\262";
    }
*/
    if (side < 100.0) {
      n1 = 1, n2 = 0;
      unit = " m\262";
    } else {
      side *= 0.01;
      n1 = 1, n2 = 2;
      unit = " ha";
    }
  } else {
    side *= M_TO_FEET;
    if (side < 100.0) {
      n1 = 1, n2 = 0;
      unit = " ft\262";
    } else if (side < MILE_TO_FEET) {
      n1 = 1, n2 = 4;
      unit = " mi\262";
      side /= MILE_TO_FEET;
    } else {
      n1 = 1, n2 = 2;
      unit = " mi\262";
      side /= MILE_TO_FEET;
    }
  }

  fStrPrintF(buf, side * side, n1, n2);
  StrCat(buf, unit);
}

void FormatSpeed(char *buf, double speed)
{
  AppPrefs *prefs;
  char *unit;

  // speed: in meters/second

  prefs = GetPrefs();

  if (prefs->unit_system == UNIT_METRIC) {
    speed *= 3.6;
    unit = " km/h";
  } else {
    speed *= (3.6 * KM_TO_MILE);
    unit = " mi/h";
  }

  fStrPrintF(buf, speed, 4, 1);
  StrCat(buf, unit);
}

void gStrPrintF(char *buf, double f)
{
  Int32 g, m, d, pg, i;
  double pf, tmp;
    
  pf = f < 0.0 ? -f : f;
  g = (Int32)(f);
  pg = g < 0 ? -g : g;
  tmp = (pf-(double)(pg))*60.0;
  m = (Int32)(tmp+0.005);
  tmp = (tmp-(double)(m))*100000.0;
  d = (Int32)(tmp);
  if (d < 0) d = 0;
  i = 0;
  if (g < 0)
    buf[i++] = '-';
  if (pg > 99) {
    buf[i++] = '0'+pg/100;
    pg = pg % 100;
    buf[i++] = '0'+pg/10;
  } else if (pg > 9)
    buf[i++] = '0'+pg/10;
  buf[i++] = '0'+pg%10;
  buf[i++] = ':';
  buf[i++] = '0'+m/10;
  buf[i++] = '0'+m%10;
  buf[i++] = '.';
  buf[i++] = '0'+d/10000;
  d = d % 10000;
  buf[i++] = '0'+d/1000;
  d = d % 1000;
  buf[i++] = '0'+d/100;
  d = d % 100;
  buf[i++] = '0'+d/10;
  buf[i++] = '0'+d%10;
  buf[i++] = '\0';
}

void gnStrPrintF(char *buf, double f, Int16 ci)
{
  Int32 g, m, d, pg, i;
  double pf, tmp;

  pf = f < 0.0 ? -f : f;
  g = (Int32)(f);
  pg = g < 0 ? -g : g;
  tmp = (pf-(double)(pg))*60.0;
  m = (Int32)(tmp+0.005);
  tmp = (tmp-(double)(m))*10000.0;
  d = (Int32)(tmp);
  if (d < 0) d = 0;
  i = 0;
  if (pg > 99) {
    if (ci == 3)
      buf[i++] = '0'+pg/100;
    pg = pg % 100;
  }
  buf[i++] = '0'+pg/10;
  buf[i++] = '0'+pg%10;
  buf[i++] = '0'+m/10;
  buf[i++] = '0'+m%10;
  buf[i++] = '.';
  buf[i++] = '0'+d/1000;
  d = d % 1000;
  buf[i++] = '0'+d/100;
  d = d % 100;
  buf[i++] = '0'+d/10;
  buf[i++] = '0'+d%10;
  buf[i++] = '\0';
}

void hStrPrintFd(char *buf, double d)
{
  Int32 h, m, s;
  double tmp;

  h = (Int32)(d);
  tmp = (d-(double)(h))*60.0;
  m = (Int32)(tmp+0.005);
  tmp = (tmp-(double)(m))*60.0;
  s = (Int32)(tmp+0.005);

  hStrPrintF(buf, h, m, s);
}

void hStrPrintFi(char *buf, UInt32 i)
{
  DateTimeType dt;
  TimSecondsToDateTime(i, &dt);
  hStrPrintF(buf, dt.hour, dt.minute, dt.second);
}

void hStrPrintF(char *buf, Int16 h, Int16 m, Int16 s)
{
  Int16 i, d;
  
  i = 0;

  if (h >= 24) {
    d = h / 24;
    h = h % 24;
    if (d >= 10)
      buf[i++] = '0'+d/10;
    buf[i++] = '0'+d%10;
    buf[i++] = 'd';
    buf[i++] = ' ';
  } else
    d = 0;

  buf[i++] = '0'+h/10;
  buf[i++] = '0'+h%10;
  buf[i++] = ':';
  buf[i++] = '0'+m/10;
  buf[i++] = '0'+m%10;
  if (d == 0) {
    buf[i++] = ':';
    buf[i++] = '0'+s/10;
    buf[i++] = '0'+s%10;
  }
  buf[i++] = '\0';
}

void fStrPrintF(char *buf, double f, Int16 ci, Int16 cd) {
  char fmt[16];
  sys_sprintf(fmt, "%%%d.%df", ci+cd, cd);
  sys_sprintf(buf, fmt, f);
}
