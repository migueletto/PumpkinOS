#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "ts.h"
#include "timeutc.h"

time_t time2ts(int day, int month, int year, int hour, int min, int sec) {
  struct tm tm;

  memset(&tm, 0, sizeof(tm));
  tm.tm_year = year - 1900;
  tm.tm_mon = month - 1;
  tm.tm_mday = day;
  tm.tm_hour = hour;
  tm.tm_min = min;
  tm.tm_sec = sec;

  return timeutc(&tm);
}

void ts2time(time_t ts, int *day, int *month, int *year, int *wday, int *hour, int *min, int *sec) {
  struct tm tm;

  utctime(&ts, &tm);

  if (year)  *year  = tm.tm_year + 1900;
  if (month) *month = tm.tm_mon + 1;
  if (day)   *day   = tm.tm_mday;
  if (wday)  *wday  = tm.tm_wday;
  if (hour)  *hour  = tm.tm_hour;
  if (min)   *min   = tm.tm_min;
  if (sec)   *sec   = tm.tm_sec;
}
