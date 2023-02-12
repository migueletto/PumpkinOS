#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>

#include "timeutc.h"
#include "sys.h"

#define YEAR0 1970

static const int ndays[] = {
  0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365
};

static int leap_year(int year) {
  return (year % 4) == 0 && ((year % 100) != 0 || (year % 400) == 0);
}

static int leap_days(int year) {
  return (year/4 - (YEAR0 - 1)/4) - (year/100 - (YEAR0 - 1)/100) + (year/400 - (YEAR0 - 1)/400);
}

time_t timeutc(struct tm *tm) {
  int year;
  time_t t;

  year = tm->tm_year + 1900;

  t = 365 * (year - YEAR0) + ndays[tm->tm_mon] + tm->tm_mday - 1 + leap_days(year - 1);
  if (tm->tm_mon > 1 && leap_year(year)) t++;

  t = t * 24 + tm->tm_hour;
  t = t * 60 + tm->tm_min;
  t = t * 60 + tm->tm_sec;

  return t;
}

struct tm *utctime(const time_t *t, struct tm *tm) {
  sys_gmtime(t, tm);
  return tm;
}

int timeofday(struct timeval *tv) {
  return gettimeofday(tv, NULL);
}

int daysinmonth(int year, int month) {
  int n = 0;

  if (month >= 1 && month <= 12) {
    n = ndays[month] - ndays[month-1];
    if (month == 2 && leap_year(year)) {
      n++;
    }
  }

  return n;
}
