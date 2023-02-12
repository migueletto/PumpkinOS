#include <PalmOS.h>

#include <time.h>
#include <sys/time.h>

#include "pwindow.h"
#include "sys.h"
#include "vfs.h"
#include "mem.h"
#include "pumpkin.h"
#include "timeutc.h"
#include "debug.h"
#include "xalloc.h"

#define YEAR0    1904
#define S_P_MIN  60
#define S_P_HOUR (60*S_P_MIN)
#define S_P_DAY  (24*S_P_HOUR)

#define PALMOS_MODULE "DateTime"

static const char *monthNames[] = {
  "Jan", "Feb", "Mar", "Apr",
  "May", "Jun", "Jul", "Aug",
  "Sep", "Oct", "Nov", "Dec"
};

static const char *fullMonthNames[] = {
  "January", "February", "March", "April",
  "May", "June", "July", "August",
  "September", "October", "November", "December"
};

static const char *dayNames[] = {
  "Sunday", "Monday", "Tuesday", "Wednesday",
  "Thursday", "Friday", "Saturday"
};

static void TimGmTime(UInt32 seconds, struct tm *tm) {
  UInt32 year, month, day, hour, min;
  UInt32 days, d, s, count;

  d = 0;

  for (year = YEAR0, count = 0;; year++) {
    days = daysinmonth(year, 2) == 28 ? 365 : 366;
    s = days * S_P_DAY;
    if (count + s > seconds) break;
    count += s;
    d += days;
  }
  tm->tm_year = year - 1900;

  for (month = 1; month <= 12; month++) {
    days = daysinmonth(year, month);
    s = days * S_P_DAY;
    if (count + s > seconds) break;
    count += s;
    d += days;
  }
  tm->tm_mon = month - 1;

  for (day = 1; day <= days; day++) {
    if (count + S_P_DAY > seconds) break;
    count += S_P_DAY;
    d++;
  }
  tm->tm_mday = day;
  tm->tm_wday = (5 + d) % 7; // 1904-01-01 was Friday

  for (hour = 0; hour < 24; hour++) {
    if (count + S_P_HOUR > seconds) break;
    count += S_P_HOUR;
  }
  tm->tm_hour = hour;

  for (min = 0; min < 60; min++) {
    if (count + S_P_MIN > seconds) break;
    count += S_P_MIN;
  }
  tm->tm_min = min;
  tm->tm_sec = seconds - count;
}

static UInt32 TimTimeGm(struct tm *tm) {
  UInt32 year, month, count;

  for (year = YEAR0, count = 0; year < tm->tm_year + 1900; year++) {
    for (month = 1; month <= 12; month++) {
      count += daysinmonth(year, month);
    }
  }

  for (month = 1; month < tm->tm_mon + 1; month++) {
    count += daysinmonth(year, month);
  }

  count += tm->tm_mday - 1;
  count *= S_P_DAY;
  count += tm->tm_hour * S_P_HOUR;
  count += tm->tm_min  * S_P_MIN;
  count += tm->tm_sec;

  return count;
}

static void tim_t2tm(UInt32 seconds, struct tm *tm) {
  Int32 zone, dls;

  zone = PrefGetPreference(prefTimeZone)*60;
  dls = PrefGetPreference(prefDaylightSavingAdjustment)*60;
  if (zone >= 0 || seconds >= -zone) {
    seconds += zone;
  }
  if (dls >= 0 || seconds >= -dls) {
    seconds += dls;
  }
  TimGmTime(seconds, tm);
}

static time_t tim_tm2t(struct tm *tm) {
  Int32 zone, dls;
  UInt32 seconds;

  zone = PrefGetPreference(prefTimeZone)*60;
  dls = PrefGetPreference(prefDaylightSavingAdjustment)*60;
  seconds = TimTimeGm(tm);
  if (zone <= 0 || seconds >= zone) {
    seconds -= zone;
  }
  if (dls <= 0 || seconds >= dls) {
    seconds -= dls;
  }

  return seconds;
}

void TimSecondsToDateTime(UInt32 seconds, DateTimeType *dateTimeP) {
  struct tm tm;

  tim_t2tm(seconds, &tm);
  dateTimeP->year = tm.tm_year + 1900;
  dateTimeP->month = tm.tm_mon + 1;
  dateTimeP->day = tm.tm_mday;
  dateTimeP->hour = tm.tm_hour;
  dateTimeP->minute = tm.tm_min;
  dateTimeP->second = tm.tm_sec;
  debug(DEBUG_TRACE, PALMOS_MODULE, "TimSecondsToDateTime %u = %04d-%02d-%02d  %02d:%02d:%02d", seconds,
    dateTimeP->year, dateTimeP->month, dateTimeP->day, dateTimeP->hour, dateTimeP->minute, dateTimeP->second);
}

UInt32 TimDateTimeToSeconds(const DateTimeType *dateTimeP) {
  struct tm tm;
  UInt32 seconds;

  xmemset(&tm, 0, sizeof(struct tm));
  tm.tm_year = dateTimeP->year - 1900;
  tm.tm_mon = dateTimeP->month - 1;
  tm.tm_mday = dateTimeP->day;
  tm.tm_hour = dateTimeP->hour;
  tm.tm_min = dateTimeP->minute;
  tm.tm_sec = dateTimeP->second;
  seconds = tim_tm2t(&tm);

  debug(DEBUG_TRACE, PALMOS_MODULE, "TimDateTimeToSeconds %04d-%02d-%02d  %02d:%02d:%02d = %u",
    dateTimeP->year, dateTimeP->month, dateTimeP->day, dateTimeP->hour, dateTimeP->minute, dateTimeP->second, seconds);

  return seconds;
}

void TimAdjust(DateTimeType *dateTimeP, Int32 adjustment) {
  UInt32 seconds;

  if (dateTimeP) {
    debug(DEBUG_TRACE, PALMOS_MODULE, "TimAdjust %04d-%02d-%02d  %02d:%02d:%02d (%d)", dateTimeP->year, dateTimeP->month, dateTimeP->day, dateTimeP->hour, dateTimeP->minute, dateTimeP->second, adjustment);
    seconds = TimDateTimeToSeconds(dateTimeP);
    seconds += adjustment;
    TimSecondsToDateTime(seconds, dateTimeP);
    debug(DEBUG_TRACE, PALMOS_MODULE, "TimAdjust %04d-%02d-%02d  %02d:%02d:%02d", dateTimeP->year, dateTimeP->month, dateTimeP->day, dateTimeP->hour, dateTimeP->minute, dateTimeP->second);
  }
}

// This function returns a descriptive string for the specified time zone.
// This string identifies the time zone first by its country, such as “USA
// (Mountain)” or “Canada (Eastern).” If the function cannot find a
// time zone that matches the specified GMT offset and country, it
// returns a string containing the time zone as a numeric offset from
// the GMT (for example, “GMT+9:00”).
// timeZone: the time zone, given as minutes east of Greenwich Mean Time (GMT).

void TimeZoneToAscii(Int16 timeZone, const LmLocaleType *localeP, Char *string) {
  char s;

  if (string) {
    if (timeZone < 0) {
      timeZone = -timeZone;
      s = '-';
    } else {
      s = '+';
    }
    StrPrintF(string, "GMT%c%d:%02d", s, timeZone / 60, timeZone % 60);
  }
}

Int16 DaysInMonth(Int16 month, Int16 year) {
  return daysinmonth(year, month);
}

Int16 DayOfWeek(Int16 month, Int16 day, Int16 year) {
  struct tm tm;
  time_t t;

  xmemset(&tm, 0, sizeof(struct tm));
  tm.tm_year = year - 1900;
  tm.tm_mon = month - 1;
  tm.tm_mday = day;
  tm.tm_hour = 12;
  tm.tm_min = 0;
  tm.tm_sec = 0;
  t = tim_tm2t(&tm);

  tim_t2tm(t, &tm);

  return tm.tm_wday;
}

/*
typedef enum {
  dom1stSun,  dom1stMon,  dom1stTue,  dom1stWen,  dom1stThu,  dom1stFri,  dom1stSat,
  dom2ndSun,  dom2ndMon,  dom2ndTue,  dom2ndWen,  dom2ndThu,  dom2ndFri,  dom2ndSat,
  dom3rdSun,  dom3rdMon,  dom3rdTue,  dom3rdWen,  dom3rdThu,  dom3rdFri,  dom3rdSat,
  dom4thSun,  dom4thMon,  dom4thTue,  dom4thWen,  dom4thThu,  dom4thFri,  dom4thSat,
  domLastSun, domLastMon, domLastTue, domLastWen, domLastThu, domLastFri, domLastSat
} DayOfMonthType;
*/

Int16 DayOfMonth(Int16 month, Int16 day, Int16 year) {
  Int16 i, n, r = 0;

  for (i = day, n = 0; i > 0 ; i -= 7) {
    n++;
  }

  switch (DayOfWeek(month, day, year)) {
    case 0: r = dom1stSun + n*7; break;
    case 1: r = dom1stMon + n*7; break;
    case 2: r = dom1stTue + n*7; break;
    case 3: r = dom1stWen + n*7; break;
    case 4: r = dom1stThu + n*7; break;
    case 5: r = dom1stFri + n*7; break;
    case 6: r = dom1stSat + n*7; break;
  }

  return r;
}

void DateSecondsToDate(UInt32 seconds, DateType *dateP) {
  DateTimeType dt;

  if (dateP) {
    TimSecondsToDateTime(seconds, &dt);
    dateP->year = dt.year - YEAR0;
    dateP->month = dt.month;
    dateP->day = dt.day;
  }
}

void DateDaysToDate(UInt32 days, DateType *dateP) {
  UInt32 year, month, count, d;

  if (dateP) {
    dateP->year = 0;
    dateP->month = 0;
    dateP->day = 0;

    for (year = 0, count = 0; year < 128; year++) {
      for (month = 1; month <= 12; month++) {
        d = daysinmonth(year, month);
        if (count + d >= days) {
          dateP->year = year;
          dateP->month = month;
          dateP->day = days - count + 1;
          debug(DEBUG_TRACE, PALMOS_MODULE, "DateDaysToDate %u = %04d-%02d-%02d ", days, dateP->year+YEAR0, dateP->month, dateP->day);
          return;
        }
        count += d;
      }
    }
  }
}

UInt32 DateToDays(DateType date) {
  UInt32 year, month, days;

  for (year = 0, days = 0; year < date.year; year++) {
    for (month = 1; month <= 12; month++) {
      days += daysinmonth(year, month);
    }
  }

  for (month = 1; month < date.month; month++) {
    days += daysinmonth(year, month);
  }

  days += date.day - 1;
  debug(DEBUG_TRACE, PALMOS_MODULE, "DateToDays %04d-%02d-%02d = %u", date.year+YEAR0, date.month, date.day, days);

  return days;
}

void DateAdjust(DateType *dateP, Int32 adjustment) {
  DateTimeType dateTime;
  UInt32 seconds;

  if (dateP) {
    debug(DEBUG_TRACE, PALMOS_MODULE, "DateAdjust %04d-%02d-%02d (%d)", dateP->year+YEAR0, dateP->month, dateP->day, adjustment);
    dateTime.year = dateP->year + YEAR0;
    dateTime.month = dateP->month;
    dateTime.day = dateP->day;
    dateTime.hour = 12;
    dateTime.minute = 0;
    dateTime.second = 0;
    seconds = TimDateTimeToSeconds(&dateTime);
    seconds += adjustment * S_P_DAY;
    DateSecondsToDate(seconds, dateP);
    debug(DEBUG_TRACE, PALMOS_MODULE, "DateAdjust %04d-%02d-%02d", dateP->year+YEAR0, dateP->month, dateP->day);
  }
}

void DateToAscii(UInt8 months, UInt8 days, UInt16 years, DateFormatType dateFormat, Char *pString) {
  if (pString) {
    StrCopy(pString, "FIXME");

    switch (dateFormat) {
      case dfMDYWithSlashes:   // 12/31/95
        StrPrintF(pString, "%02d/%02d/%02d", months, days, years%100);
        break;
      case dfDMYWithSlashes:   // 31/12/95
        StrPrintF(pString, "%02d/%02d/%02d", days, months, years%100);
        break;
      case dfDMYWithDots:      // 31.12.95
        StrPrintF(pString, "%02d.%02d.%02d", days, months, years%100);
        break;
      case dfDMYWithDashes:    // 31-12-95
        StrPrintF(pString, "%02d-%02d-%02d", days, months, years%100);
        break;
      case dfYMDWithSlashes:   // 95/12/31
        StrPrintF(pString, "%02d/%02d/%02d", years%100, months, days);
        break;
      case dfYMDWithDots:      // 95.12.31
        StrPrintF(pString, "%02d.%02d.%02d", years%100, months, days);
        break;
      case dfYMDWithDashes:    // 95-12-31
        StrPrintF(pString, "%02d-%02d-%02d", years%100, months, days);
        break;
      case dfMDYLongWithComma: // Dec 31, 1995
        if (months >= 1 && months <= 12) {
          StrPrintF(pString, "%s %d, %04d", monthNames[months-1], days, years);
        }
        break;
      case dfDMYLong:          // 31 Dec 1995
        if (months >= 1 && months <= 12) {
          StrPrintF(pString, "%d %s %04d", days, monthNames[months-1], years);
        }
        break;
      case dfDMYLongWithDot:   // 31. Dec 1995
        if (months >= 1 && months <= 12) {
          StrPrintF(pString, "%d. %s %04d", days, monthNames[months-1], years);
        }
        break;
      case dfDMYLongNoDay:     // Dec 1995
        if (months >= 1 && months <= 12) {
          StrPrintF(pString, "%s %04d", monthNames[months-1], years);
        }
        break;
      case dfDMYLongWithComma: // 31 Dec: 1995
        if (months >= 1 && months <= 12) {
          StrPrintF(pString, "%d %s: %04d", days, monthNames[months-1], years);
        }
        break;
      case dfYMDLongWithDot:   // 1995.12.31
        StrPrintF(pString, "%04d.%02d.%02d", years, months, days);
        break;
      case dfYMDLongWithSpace: // 1995 Dec 31
        if (months >= 1 && months <= 12) {
          StrPrintF(pString, "%04d %s %d", years, monthNames[months-1], days);
        }
        break;
      case dfMYMed:            // Dec '95
        if (months >= 1 && months <= 12) {
          StrPrintF(pString, "%s '%02d", monthNames[months-1], years%100);
        }
        break;
      case dfMYMedNoPost:      // Dec 95    (added for French 2.0 ROM)
        if (months >= 1 && months <= 12) {
          StrPrintF(pString, "%s %02d", monthNames[months-1], years%100);
        }
        break;
      case dfMDYWithDashes:    // 12-31-95  (added for 4.0 ROM)
        StrPrintF(pString, "%02d-%02d-%02d", months, days, years%100);
        break;
      default:
        debug(DEBUG_ERROR, "PALMOS_MODULE", "invalid dateFormat %d", dateFormat);
        break;
    }
  }
}

void TimeToAscii(UInt8 hours, UInt8 minutes, TimeFormatType timeFormat, Char *pString) {
  char sep, ampm, buf[4];
  int h24, i;

  if (pString) {
    StrCopy(pString, "FIXME");

    switch (timeFormat) {
      case tfColon:     h24 = 1; sep = ':'; break;
      case tfColonAMPM: h24 = 0; sep = ':'; break;
      case tfColon24h:  h24 = 1; sep = ':'; break;
      case tfDot:       h24 = 1; sep = '.'; break;
      case tfDotAMPM:   h24 = 0; sep = '.'; break;
      case tfDot24h:    h24 = 1; sep = '.'; break;
      case tfHoursAMPM: h24 = 0; sep = 0;   break;
      case tfHours24h:  h24 = 1; sep = 0;   break;
      case tfComma24h:  h24 = 1; sep = ','; break;
      default:          h24 = 1; sep = ':'; break;
    }

    i = 0;
    ampm = 0;

    if (h24) {
      StrPrintF(buf, "%d", hours);
      pString[i++] = buf[0];
      if (buf[1]) pString[i++] = buf[1];
    } else {
      switch (hours) {
        case 0 :
          hours = 12;
          ampm = 'a';
          break;
        case 12:
          ampm = 'p';
          break;
        default:
          if (hours < 12) {
            ampm = 'a';
          } else {
            hours -= 12;
            ampm = 'p';
          }
      }
      StrPrintF(buf, "%d", hours);
      pString[i++] = buf[0];
      if (buf[1]) pString[i++] = buf[1];
    }

    if (sep) {
      pString[i++] = sep;
      StrPrintF(buf, "%02d", minutes);
      pString[i++] = buf[0];
      pString[i++] = buf[1];
    }

    if (ampm) {
      pString[i++] = ' ';
      pString[i++] = ampm;
      pString[i++] = 'm';
    }
    pString[i++] = 0;
  }
}

/*
The following code converts the current local time to UTC:
Int16 timeZone = PrefGetPreference(prefTimeZone);
Int16 daylightSavingAdjustment = PrefGetPreference(prefDaylightSavingAdjustment);
UInt32 utcTime = TimTimeZoneToUTC(TimGetSeconds(), timeZone, daylightSavingAdjustment);
*/

UInt32 TimTimeZoneToUTC(UInt32 seconds, Int16 timeZone, Int16 daylightSavingAdjustment) {
  return seconds - timeZone * 60 + daylightSavingAdjustment * 60;
}

UInt32 TimUTCToTimeZone(UInt32 secondsUTC, Int16 timeZone, Int16 daylightSavingAdjustment) {
  return secondsUTC + timeZone * 60 - daylightSavingAdjustment * 60;
}

void DateToDOWDMFormat(UInt8 months, UInt8 days, UInt16 years, DateFormatType dateFormat, Char *pString) {
  UInt16 i, dow;

  if (pString) {
    dow = DayOfWeek(months, days, years);
    for (i = 0; i < 3; i++) {
      pString[i] = dayNames[dow][i];
    }
    pString[i++] = ' ';
    DateToAscii(months, days, years, dateFormat, &pString[i]);
  }
}

UInt16 DateTemplateToAscii(const Char *templateP, UInt8 months, UInt8 days, UInt16 years, Char *stringP, Int16 stringLen) {
  UInt16 i, j, s, dow, type, len = 0;
  Char c;

  if (templateP && months >= 1 && months <= 12 && days >= 1 && days <= 31) {
    // Ex.: ^3z/^0z
    debug(DEBUG_TRACE, PALMOS_MODULE, "DateTemplateToAscii (\"%s\", %d, %d, %d)", templateP, years, months, days);

    dow = DayOfWeek(months, days, years);

    for (i = 0, s = 0; templateP[i]; i++) {
      c = templateP[i];

      switch (s) {
        case 0:
          if (c == '^') {
            s = 1;
          } else {
            if (stringP && len < stringLen) stringP[len] = c;
            len++;
          }
          break;
        case 1:
          type = c;
          s = 2;
          break;
        case 2:
          switch (type) {
            case '0': // day number
              switch (c) {
                case 's':
                case 'r':
                case 'l':
                  if (days > 9) {
                    if (stringP && len < stringLen) stringP[len] = '0' + (days / 10);
                    len++;
                  }
                  if (stringP && len < stringLen) stringP[len] = '0' + (days % 10);
                  len++;
                  break;
                case 'z':
                  if (stringP && len < stringLen) stringP[len] = '0' + (days / 10);
                  len++;
                  if (stringP && len < stringLen) stringP[len] = '0' + (days % 10);
                  len++;
                  break;
              }
              break;
            case '1': // day name
              switch (c) {
                case 's':
                  if (stringP && len < stringLen) stringP[len] = dayNames[dow][0];
                  len++;
                  break;
                case 'r':
                  for (j = 0; j < 3; j++) {
                    if (stringP && len < stringLen) stringP[len] = dayNames[dow][j];
                    len++;
                  }
                  break;
                case 'l':
                  for (j = 0; dayNames[dow][j]; j++) {
                    if (stringP && len < stringLen) stringP[len] = dayNames[dow][j];
                    len++;
                  }
                  break;
              }
              break;
            case '2': // month name
              switch (c) {
                case 's':
                  if (stringP && len < stringLen) stringP[len] = fullMonthNames[months-1][0];
                  len++;
                  break;
                case 'r':
                  for (j = 0; j < 3; j++) {
                    if (stringP && len < stringLen) stringP[len] = fullMonthNames[months-1][j];
                    len++;
                  }
                  break;
                case 'l':
                  for (j = 0; fullMonthNames[months-1][j]; j++) {
                    if (stringP && len < stringLen) stringP[len] = fullMonthNames[months-1][j];
                    len++;
                  }
                  break;
              }
              break;
            case '3': // month number
              switch (c) {
                case 's':
                case 'r':
                case 'l':
                  if (months > 9) {
                    if (stringP && len < stringLen) stringP[len] = '0' + (months / 10);
                    len++;
                  }
                  if (stringP && len < stringLen) stringP[len] = '0' + (months % 10);
                  len++;
                  break;
                case 'z':
                  if (stringP && len < stringLen) stringP[len] = '0' + (months / 10);
                  len++;
                  if (stringP && len < stringLen) stringP[len] = '0' + (months % 10);
                  len++;
                  break;
              }
              break;
            case '4': // year number
              switch (c) {
                case 's':
                  if (stringP && len < stringLen) stringP[len] = '0' + ((years % 100) / 10);
                  len++;
                  if (stringP && len < stringLen) stringP[len] = '0' + (years % 10);
                  len++;
                  break;
                case 'r':
                case 'l':
                  if (stringP && len < stringLen) stringP[len] = '0' + (years / 1000);
                  len++;
                  if (stringP && len < stringLen) stringP[len] = '0' + ((years / 100) % 10);
                  len++;
                  if (stringP && len < stringLen) stringP[len] = '0' + ((years % 100) / 10);
                  len++;
                  if (stringP && len < stringLen) stringP[len] = '0' + (years % 10);
                  len++;
                  break;
              }
              break;
          }
          s = 0;
          break;
      }
    }
    if (stringP && len < stringLen) stringP[len] = 0;
  }

  return len;
}
