#include <PalmOS.h>
#include <GPSLib68K.h>

#include "gps.h"
#include "app.h"
#include "gpslib.h"
#include "format.h"

#include "debug.h"

static ChannelSummary chs[MAX_CHANNELS];
static VisibleSatellite sat[MAX_CHANNELS];

Err GpsLibEventCallback(SysNotifyParamType *notifyParams) {
  UInt16 GpsLibRef, notifInfo;
  UInt16 *notifInfoP;
  GPSPositionDataType gpsPos;
  GPSVelocityDataType gpsSpeed;
  GPSStatusDataType gpsStatus;
  UInt16 numSats, used, i;
  static GPSSatDataType *gpsSat = NULL;

  if (notifyParams->notifyType == sysNotifyGPSDataEvent) {
    notifInfoP = (UInt16 *)notifyParams->notifyDetailsP;
    notifInfo = *notifInfoP;
    GpsLibRef = *(UInt16 *)notifyParams->userDataP;
    debug(1, "Navg", "GpsLibEventCallback %d", notifInfo);

    switch (notifInfo) {
      case gpsLocationChange:
        debug(1, "Navg", "GpsLibEventCallback gpsLocationChange");
        if (GPSGetPosition(GpsLibRef, &gpsPos) == gpsErrNone) {
          debug(1, "Navg", "GpsLibEventCallback GPSGetPosition alt=%.1f", gpsPos.altMSL);
          SetPosition(garmin_from_coord(gpsPos.lat), garmin_from_coord(gpsPos.lon));
          SetHeight(gpsPos.altMSL);
        }
        if (GPSGetVelocity(GpsLibRef, &gpsSpeed) == gpsErrNone) {
          SetSpeedUI(gpsSpeed.speed);
          SetClimb(gpsSpeed.up);
        }
        break;
      case gpsLostFix:
        SetValidity(0, 0);
        break;
      case gpsStatusChange:
      case gpsSatDataChange:
        numSats = GPSGetMaxSatellites(GpsLibRef);

        if (gpsSat == NULL) {
          if ((gpsSat = MemPtrNew(numSats * sizeof(GPSSatDataType))) == NULL) return 0;
        }

        used = 0;

        if (GPSGetSatellites(GpsLibRef, gpsSat) == gpsErrNone) {
          for (i = 0; i < numSats && i < MAX_CHANNELS; i++) {
            sat[i].prn = gpsSat[i].svid;
            sat[i].elevation = (gpsSat[i].elevation * 180.0) / sys_pi();
            sat[i].azimuth = (gpsSat[i].azimuth * 180.0) / sys_pi();

            chs[i].prn = gpsSat[i].svid;
            chs[i].cno = gpsSat[i].snr;
            chs[i].flags = 0;

            if (chs[i].cno > 0)
              chs[i].flags |= CHANNEL_VALID;
            if (gpsSat[i].status & gpsSatEphMask)
              chs[i].flags |= CHANNEL_EPHEMERIDS;
            if (gpsSat[i].status & gpsSatDifMask)
              chs[i].flags |= CHANNEL_DGPS;
            if (gpsSat[i].status & gpsSatUsedMask) {
              chs[i].flags |= CHANNEL_USED;
              used++;
            }
          }
          SetChannelSummary(numSats, chs);
          SetVisible(numSats, sat);
        }

        if (GPSGetStatus(GpsLibRef, &gpsStatus) == gpsErrNone) {
          switch (gpsStatus.fix) {
            case gpsFixUnusable:
            case gpsFixInvalid:
              SetValidity(0, 0);
              break;
            case gpsFix2D:
            case gpsFix2DDiff:
              SetValidity(1, used);
              break;
            case gpsFix3D:
            case gpsFix3DDiff:
              SetValidity(1, used);
          }
          SetEPE(gpsStatus.eph, gpsStatus.epv);
        }
        break;
      case gpsModeChange:
      case gpsNeedInitialized:
      case gpsDisplayLegalese:
        break;
    }
  }

  return 0;
}
