#include <PalmOS.h>

#include "thin.h"
#include "gps.h"
#include "log.h"
#include "format.h"
#include "MathLib.h"
#include "error.h"

static TracklogType tlog, tlog0;

UInt16 StartThinLog(FileHand src, FileHand dst) {
  UInt16 n;

  if ((n =  LogSize(src) / sizeof(TracklogType)) == 0)
    return 0;

  // position at the begining
  SeekLog(src, -(n*sizeof(TracklogType)));

  if (ReadLog(src, &tlog0, sizeof(TracklogType)) != 0) {
    SeekLog(src, 0);
    return 0;
  }

  WriteLog(dst, &tlog0, sizeof(TracklogType));
    
  return n;
}

UInt16 ThinLog(FileHand src, FileHand dst, double e, UInt16 n) {
  UInt16 i;
  double dx, y1, y2, cosd, d;

  for (i = 0; i < n; i++) {
    if (ReadLog(src, &tlog, sizeof(TracklogType)) != 0) {
      tlog = tlog0;
      break;
    }

    y1 = TORAD(tlog0.latitude);
    y2 = TORAD(tlog.latitude);
    dx = TORAD(fabs(tlog0.longitude-tlog.longitude));
  
    cosd = sin(y1) * sin(y2) + cos(y1) * cos(y2) * cos(dx);
    d = acos(cosd) * EARTH_RADIUS;	// distance in KM

    if (d < e)
      continue;

    WriteLog(dst, &tlog, sizeof(TracklogType));
    tlog0 = tlog;
  }

  return i;
}

UInt16 StopThinLog(FileHand src, FileHand dst) {
  // position at end
  SeekLog(src, 0);

  if (tlog.latitude != tlog0.latitude || tlog.longitude != tlog0.longitude) {
    WriteLog(dst, &tlog, sizeof(TracklogType));
    return 1;
  }

  return 0;
}
