#include <PalmOS.h>
#include <GPSLib68K.h>

#include <time.h>
#include <sys/time.h>
#include <math.h>

#include "thread.h"
#include "script.h"
#include "pwindow.h"
#include "pfont.h"
#include "graphic.h"
#include "image.h"
#include "sys.h"
#include "pit_io.h"
#include "ptr.h"
#include "vfs.h"
#include "mem.h"
#include "pumpkin.h"
#include "gps.h"
#include "debug.h"
#include "xalloc.h"

#define NUM_SATS     12

#define TAG_GPSLIB   gpsLibName
#define TAG_GPSPRIV  "gpspriv"

typedef struct {
  char *tag;
  GPSStatusDataType status;
  GPSPositionDataType position;
  GPSVelocityDataType velocity;
  GPSTimeDataType time;
} gps_priv_t;

typedef struct {
  int ptr;
} priv_ptr_t;

typedef struct {
  bt_provider_t *bt;
  int handle;
  int ptr;
  GPSSatDataType sat[NUM_SATS];
  UInt8 numSats;
  gps_parse_line_f parse_line;
} gps_module_t;

extern thread_key_t *gps_key;

int GPSInitModule(gps_parse_line_f parse_line, bt_provider_t *bt) {
  gps_module_t *module;

  if ((module = xcalloc(1, sizeof(gps_module_t))) == NULL) {
    return -1;
  }

  module->bt = bt;
  module->parse_line = parse_line;
  module->numSats = NUM_SATS;
  module->ptr = -1;
  module->handle = -1;
  thread_set(gps_key, module);

  return 0;
}

int GPSFinishModule(void) {
  gps_module_t *module = (gps_module_t *)thread_get(gps_key);

  if (module) {
    xfree(module);
  }

  return 0;
}

UInt16 GPSGetLibAPIVersion(const UInt16 refNum) {
  return gpsAPIVersion;
}

static int gpslib_callback(io_arg_t *arg) {
  priv_ptr_t *priv_ptr;
  gps_priv_t *priv;
  gps_t *gps;
  SysNotifyParamType notify;
  UInt16 detail;
  int newpos;

  gps = (gps_t *)arg->data;
  priv_ptr = (priv_ptr_t *)(gps->priv);

  switch (arg->event) {
    case IO_CONNECT:
    case IO_CONNECT_ERROR:
      if ((priv = ptr_lock(priv_ptr->ptr, TAG_GPSPRIV)) != NULL) {
        priv->status.mode = gpsModeOff;
        priv->status.fix = gpsFixInvalid;
        ptr_unlock(priv_ptr->ptr, TAG_GPSPRIV);
      }
      break;
    case IO_LINE:
      debug(DEBUG_INFO, "GPSLIB", "received line");
      if (arg->len && gps->parse_line && gps->parse_line((char *)arg->buf, gps, arg->fd) == 0) {
        newpos = gps->newpos;
        gps->newpos = 0;
        if (newpos) {
          debug(DEBUG_INFO, "GPSLIB", "new position mode %d", gps->mode);
          debug(DEBUG_INFO, "GPSLIB", "orig lon=%.6f, lat=%.6f, height=%.1f, speed=%.1f, course=%.1f", gps->lon, gps->lat, gps->height, gps->speed, gps->course);
          if ((priv = ptr_lock(priv_ptr->ptr, TAG_GPSPRIV)) != NULL) {
            if (gps->mode <= 0) {
              priv->status.mode = gpsModeOff;
              priv->status.fix = gpsFixInvalid;
            } else if (gps->mode == 2) {
              priv->status.mode = gpsModeNormal;
              priv->status.fix = gpsFix2D;
            } else if (gps->mode == 3) {
              priv->status.mode = gpsModeNormal;
              priv->status.fix = gpsFix3D;
            }

            detail = gpsStatusChange;
            MemSet(&notify, sizeof(SysNotifyParamType), 0);
            notify.notifyType = sysNotifyGPSDataEvent;
            notify.broadcaster = gpsLibCreator;
            notify.notifyDetailsP = &detail;
            debug(DEBUG_INFO, "GPSLIB", "broadcast gpsStatusChange");
            SysNotifyBroadcast(&notify);

            // degrees = semicircles * ( 180 / 2^31 )
            // semicircles = degrees * ( 2^31 / 180 )
            priv->position.lon = (Int32)(gps->lon * (2147483648.0 / 180.0));
            priv->position.lat = (Int32)(gps->lat * (2147483648.0 / 180.0));
            priv->position.altMSL = (float)gps->height;  // meters
            //priv->position.altWGS84 = (float)gps->height;  // meters
            priv->velocity.track = (float)(gps->course * M_PI / 180.0);  // in radians
            priv->velocity.speed = (float)gps->speed;  // m/s
            priv->time.seconds = gps->hour*24*60*60 + gps->min*60 + gps->sec;  // seconds since midnight (UTC)
            debug(DEBUG_INFO, "GPSLIB", "conv lon=%d, lat=%d, alt=%.1f, speed=%.1f, track=%.1f",
              priv->position.lon, priv->position.lat, priv->position.altMSL, priv->velocity.speed, priv->velocity.track);

            detail = gpsLocationChange;
            MemSet(&notify, sizeof(SysNotifyParamType), 0);
            notify.notifyType = sysNotifyGPSDataEvent;
            notify.broadcaster = gpsLibCreator;
            notify.notifyDetailsP = &detail;
            debug(DEBUG_INFO, "GPSLIB", "broadcast gpsLocationChange");
            SysNotifyBroadcast(&notify);

            ptr_unlock(priv_ptr->ptr, TAG_GPSPRIV);
          }
        }
      }
      break;
    case IO_TIMEOUT:
      // force disconnect
      return 1;
    case IO_DISCONNECT:
      debug(DEBUG_INFO, "GPSLIB", "disconnect");
      ptr_free(priv_ptr->ptr, TAG_GPSPRIV);
      xfree(priv_ptr);
      xfree(gps);
      break;
  }

  return 0;
}

static void gpslib_destructor(void *p) {
  gps_priv_t *priv;

  priv = (gps_priv_t *)p;

  if (priv) {
    xfree(priv);
  }
}

Err GPSOpen(const UInt16 refNum) {
  gps_module_t *module = (gps_module_t *)thread_get(gps_key);
  io_addr_t addr;
  priv_ptr_t *priv_ptr;
  gps_priv_t *priv;
  gps_t *gps;
  char host[256];
  uint32_t id, port;

  if (module->ptr != -1) {
    return gpsErrNone;
  }

  if (pumpkin_get_serial_by_creator(&id, NULL, gpsLibCreator) == -1) {
    debug(DEBUG_ERROR, "GPSLIB", "serial not found");
    return gpsErrNotOpen;
  }

  if (pumpkin_info_serial(id, host, sizeof(host), &port) == -1) {
    debug(DEBUG_ERROR, "GPSLIB", "serial %u info not found", id);
    return gpsErrNotOpen;
  }

  if (io_fill_addr(host, port, &addr) == -1) {
    return gpsErrNotOpen;
  }

  if ((priv_ptr = xcalloc(1, sizeof(priv_ptr_t))) == NULL) {
    return gpsErrNotOpen;
  }

  if ((gps = xcalloc(1, sizeof(gps_t))) == NULL) {
    xfree(priv_ptr);
    return gpsErrNotOpen;
  }

  if ((priv = xcalloc(1, sizeof(gps_priv_t))) == NULL) {
    xfree(gps);
    xfree(priv_ptr);
    return gpsErrNotOpen;
  }

  priv->tag = TAG_GPSPRIV;
  priv->status.mode = gpsModeOff;
  priv->status.fix = gpsFixInvalid;

  if ((module->ptr = ptr_new(priv, gpslib_destructor)) == -1) {
    xfree(priv);
    xfree(gps);
    xfree(priv_ptr);
    return gpsErrNotOpen;
  }

  priv_ptr->ptr = module->ptr;

  gps->parse_line = module->parse_line;
  gps->priv = priv_ptr;

  if (gps->parse_line == NULL) {
    debug(DEBUG_ERROR, "GPSLIB", "gps parse provider not available");
  }

  if ((module->handle = io_stream_client(TAG_GPSLIB, &addr, gpslib_callback, gps, 0, module->bt)) == -1) {
    ptr_free(module->ptr, TAG_GPSPRIV);
    module->ptr = -1;
    xfree(gps);
    xfree(priv_ptr);
  }

  return gpsErrNone;
}

Err GPSClose(const UInt16 refNum) {
  gps_module_t *module = (gps_module_t *)thread_get(gps_key);

  if (module->ptr != -1) {
    ptr_free(module->ptr, TAG_GPSPRIV);
    module->ptr = -1;
    thread_end(TAG_GPSLIB, module->handle);
    module->handle = -1;
  }
  
  return gpsErrNone;
}

UInt8 GPSGetMaxSatellites(const UInt16 refNum) {
  gps_module_t *module = (gps_module_t *)thread_get(gps_key);

  return module->numSats;
}

Err GPSGetStatus(const UInt16 refNum, GPSStatusDataType *status) {
  gps_module_t *module = (gps_module_t *)thread_get(gps_key);
  gps_priv_t *priv;
  Err err = gpsErrNotOpen;

  if (status) {
    if (module->ptr != -1 && (priv = ptr_lock(module->ptr, TAG_GPSPRIV)) != NULL) {
      MemMove(status, &priv->status, sizeof(GPSStatusDataType));
      ptr_unlock(module->ptr, TAG_GPSPRIV);
      err = gpsErrNone;
    } else {
      GPSClose(refNum);
    }
  }

  return err;
}

Err GPSGetPosition(const UInt16 refNum, GPSPositionDataType *position) {
  gps_module_t *module = (gps_module_t *)thread_get(gps_key);
  gps_priv_t *priv;
  Err err = gpsErrNotOpen;

  if (position) {
    if (module->ptr != -1 && (priv = ptr_lock(module->ptr, TAG_GPSPRIV)) != NULL) {
      MemMove(position, &priv->position, sizeof(GPSPositionDataType));
      ptr_unlock(module->ptr, TAG_GPSPRIV);
      err = gpsErrNone;
    } else {
      GPSClose(refNum);
    }
    debug(DEBUG_INFO, "GPSLIB", "GPSGetPosition alt=%.1f", position->altMSL);
  }

  return err;
}

Err GPSGetTime(const UInt16 refNum, GPSTimeDataType *time) {
  gps_module_t *module = (gps_module_t *)thread_get(gps_key);
  gps_priv_t *priv;
  Err err = gpsErrNotOpen;

  if (time) {
    if (module->ptr != -1 && (priv = ptr_lock(module->ptr, TAG_GPSPRIV)) != NULL) {
      MemMove(time, &priv->time, sizeof(GPSTimeDataType));
      ptr_unlock(module->ptr, TAG_GPSPRIV);
      err = gpsErrNone;
    } else {
      GPSClose(refNum);
    }
  }

  return err;
}

Err GPSGetVelocity(const UInt16 refNum, GPSVelocityDataType *velocity) {
  gps_module_t *module = (gps_module_t *)thread_get(gps_key);
  gps_priv_t *priv;
  Err err = gpsErrNotOpen;

  if (velocity) {
    if (module->ptr != -1 && (priv = ptr_lock(module->ptr, TAG_GPSPRIV)) != NULL) {
      MemMove(velocity, &priv->velocity, sizeof(GPSVelocityDataType));
      ptr_unlock(module->ptr, TAG_GPSPRIV);
      err = gpsErrNone;
    } else {
      GPSClose(refNum);
    }
  }

  return err;
}

Err GPSGetPVT(const UInt16 refNum, GPSPVTDataType *pvt) {
  gps_module_t *module = (gps_module_t *)thread_get(gps_key);
  gps_priv_t *priv;
  Err err = gpsErrNotOpen;

  if (pvt) {
    if (module->ptr != -1 && (priv = ptr_lock(module->ptr, TAG_GPSPRIV)) != NULL) {
      MemMove(&pvt->status, &priv->status, sizeof(GPSStatusDataType));
      MemMove(&pvt->position, &priv->position, sizeof(GPSPositionDataType));
      MemMove(&pvt->velocity, &priv->velocity, sizeof(GPSVelocityDataType));
      MemMove(&pvt->time, &priv->time, sizeof(GPSTimeDataType));
      ptr_unlock(module->ptr, TAG_GPSPRIV);
      err = gpsErrNone;
    } else {
      GPSClose(refNum);
    }
  }

  return err;
}

/*
  UInt8       svid;       // space vehicle identifier
  UInt8       status;     // status bitfield
  Int16       snr;        // signal to noise ratio * 100 (dB Hz)
  float       azimuth;    // azimuth (radians)
  float       elevation;  // elevation (radians)
*/
Err GPSGetSatellites(const UInt16 refNum, GPSSatDataType *sat) {
  gps_module_t *module = (gps_module_t *)thread_get(gps_key);
  int i;

  if (sat) {
    MemSet(sat, module->numSats * sizeof(GPSSatDataType), 0);
    for (i = 0; i < module->numSats; i++) {
      sat[i].svid = i+1;
      sat[i].status = gpsSatEphMask;
      if (i >= 4 && i <= 7) sat[i].status |= gpsSatUsedMask;
      sat[i].snr = 30 + i*2;
      sat[i].azimuth = (i*20) * M_PI / 180.0;
      sat[i].elevation = (30 + i*5) * M_PI / 180.0;
      debug(DEBUG_INFO, "GPSLIB", "GPSGetSatellites %d %.1f %.1f", i, sat[i].azimuth, sat[i].elevation);
    }
  }

  return gpsErrNone;
}
