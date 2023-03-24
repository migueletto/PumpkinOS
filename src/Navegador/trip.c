#include <PalmOS.h>

#include "gps.h"
#include "app.h"
#include "gui.h"
#include "main.h"
#include "trip.h"
#include "log.h"
#include "map.h"
#include "sound.h"
#include "format.h"
#include "misc.h"
#include "MathLib.h"
#include "error.h"

static UInt32 start_time = 0;
static UInt32 moving_time = 0;
static UInt32 moving_start_time = 0;
static double distance = 0.0;
static Boolean moving = false;

static double trip_lat0, trip_lon0;
static Boolean trip_firstpoint;

static double log_lat0, log_lon0;
static Boolean log_firstpoint;

static void DoLog(TimestampType *ts, CoordType *coord, double speed, FileHand h);

void TripStop(void)
{
  if (moving) {
    moving_time += TimGetSeconds() - moving_start_time;
    moving = false;
  }
}

void TripReset(UInt32 stime, UInt32 mtime, double dist)
{
  start_time = stime;
  moving_time = mtime;
  moving_start_time = stime;
  distance = dist;
  trip_firstpoint = true;
  log_firstpoint = true;
}

void TripRecord(TimestampType *ts, CoordType *coord, double speed)
{
  UInt32 t;
  double d, cosd, ry0, ry;

  if (trip_firstpoint) {
    trip_lat0 = coord->latitude;
    trip_lon0 = coord->longitude;
    trip_firstpoint = false;

  } else {
    t = TimGetSeconds();
    ry0 = TORAD(trip_lat0);
    ry = TORAD(coord->latitude);

    cosd = sin(ry) * sin(ry0) +
           cos(ry) * cos(ry0) * cos(TORAD(fabs(coord->longitude-trip_lon0)));
    d = (acos(cosd) * EARTH_RADIUS) * 1000.0;	// diatance in meters

    if (d > 5.0 && speed > 0.1) {
      if (!moving) {
        moving_start_time = t;
        moving = true;
      }

      distance += d;
      trip_lat0 = coord->latitude;
      trip_lon0 = coord->longitude;

    } else {
      if (moving) {
        moving_time += t - moving_start_time;
        moving = false;
      }
    }
  }
}

void TripLog(TimestampType *ts, CoordType *coord, double speed, FileHand h)
{
  double d, cosd, mind, ry0, ry;
  AppPrefs *prefs;
  UInt32 t;
  static UInt32 last_warning = 0;
    
  prefs = GetPrefs();

  if (prefs->speed_warning && speed > prefs->speed_limit) {
    t = TimGetSeconds();
    if ((t - last_warning) > 15) {
      PlaySound(SOUND_WARNING);
      last_warning = t;
    }
  }

  if (log_firstpoint) {
    log_lat0 = coord->latitude;
    log_lon0 = coord->longitude;
    if (prefs->log_enabled) {
      DoLog(ts, coord, speed, h);
      if (prefs->log_beep)
        PlaySound(SOUND_BEEP);
    }
    log_firstpoint = false;

  } else {
    ry0 = TORAD(log_lat0);
    ry = TORAD(coord->latitude);

    cosd = sin(ry) * sin(ry0) +
           cos(ry) * cos(ry0) * cos(TORAD(fabs(coord->longitude-log_lon0)));
    d = (acos(cosd) * EARTH_RADIUS) * 1000.0;	// distance in KM

    // high density

    if (speed < 5.0)
      mind = 10.0;
    else if (speed < 20.0)
      mind = 25.0;
    else
      mind = 50.0;

    switch (prefs->density) {
      case 0:	// low
        mind *= 4.0;
        break;
      case 1:	// medium
        mind *= 2.0;
    }

    if (d > mind) {
      log_lat0 = coord->latitude;
      log_lon0 = coord->longitude;
      if (prefs->log_enabled) {
        DoLog(ts, coord, speed, h);
        if (prefs->log_beep)
          PlaySound(SOUND_BEEP);
      }
    }
  }
}

static void DoLog(TimestampType *ts, CoordType *coord, double speed, FileHand h)
{
  TracklogType log;
  DateTimeType dt;

  dt.year = ts->year;
  dt.month = ts->month;
  dt.day = ts->day;
  dt.hour = ts->hour;
  dt.minute = ts->minute;
  dt.second = ts->second;
  log.t = TimDateTimeToSeconds(&dt);
  log.latitude = coord->latitude;
  log.longitude = coord->longitude;
  log.height = coord->height;
  WriteLog(h, &log, sizeof(TracklogType));
}

UInt32 TripGetStartTime(void)
{
  return start_time;
}

UInt32 TripGetStopedTime(void)
{
  return TripGetTotalTime() - TripGetMovingTime();
}

UInt32 TripGetMovingTime(void)
{
  return moving_time + (moving ? (TimGetSeconds() - moving_start_time) : 0);
}

UInt32 TripGetTotalTime(void)
{
  return TimGetSeconds() - TripGetStartTime();
}

double TripGetDistance(void)
{
  return distance;
}

void SetTrip(void)
{
  static char buf1[32], buf2[32], buf3[32], buf4[32];
  FormPtr frm = FrmGetActiveForm();
  UInt16 form = FrmGetActiveFormID();

  if (form != TripForm || FrmGetWindowHandle(frm) != WinGetActiveWindow())
    return;

  hStrPrintFd(buf1, TripGetStopedTime() / 3600.0);
  SetField(frm, stimeFld, buf1);

  hStrPrintFd(buf2, TripGetMovingTime() / 3600.0);
  SetField(frm, mtimeFld, buf2);

  hStrPrintFd(buf3, TripGetTotalTime() / 3600.0);
  SetField(frm, timeFld, buf3);

  FormatDistance(buf4, TripGetDistance());
  SetField(frm, distFld, buf4);
}
