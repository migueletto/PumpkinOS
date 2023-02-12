#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>

#include "script.h"
#include "pit_io.h"
#include "gps.h"
#include "sock.h"
#include "debug.h"
#include "xalloc.h"

static int gps_callback(int pe, int mode, time_t ts, int year, int mon, int day, int hour, int min, int sec, script_real_t lon, script_real_t lat, script_real_t alt, script_real_t speed, script_real_t course, script_ref_t ref) {
  script_arg_t ret;

  return script_call(pe, ref, &ret, "IIIIIIIIIDDDDD", mkint(1), mkint(mode), mkint(ts), mkint(year), mkint(mon), mkint(day), mkint(hour), mkint(min), mkint(sec), lon, lat, alt, speed, course);
}

static int sat_callback(int pe, int total, int index, int prn, int elevation, int azimuth, int snr, int used, script_ref_t ref) {
  script_arg_t ret;

  return script_call(pe, ref, &ret, "IIIIIIIB", mkint(2), mkint(total), mkint(index), mkint(prn), mkint(elevation), mkint(azimuth), mkint(snr), used);
}

static int gps_newpos(gps_t *gps) {
  int newpos;

  newpos = gps->newpos;
  gps->newpos = 0;
  if (newpos) {
    return gps_callback(gps->pe, gps->mode, gps->ts, gps->year, gps->month, gps->day, gps->hour, gps->min, gps->sec, gps->lon, gps->lat, gps->height, gps->speed, gps->course, gps->ref);
  }

  return 0;
}

static int gps_newsat(gps_t *gps) {
  int newsat, i;

  newsat = gps->newsat;
  gps->newsat = 0;

  if (newsat) {
    for (i = 0; i < gps->nsats; i++) {
      if (sat_callback(gps->pe, gps->nsats, i+1, gps->sat[i].prn, gps->sat[i].elevation, gps->sat[i].azimuth, gps->sat[i].snr, gps->sat[i].used, gps->ref) == -1) return -1;
    }
  }

  return 0;
}

//static int gps_sock_callback(int event, io_addr_t *addr, unsigned char *buf, int len, int fd, int handle, void **data) {
static int gps_sock_callback(io_arg_t *arg) {
  gps_t *gps;

  gps = (gps_t *)arg->data;

  switch (arg->event) {
    case IO_CONNECT:
      if (gps_callback(gps->pe, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, gps->ref) == -1) return -1;
      break;
    case IO_DATA:
      if (arg->len && gps->parse_data && gps->parse_data(arg->buf, arg->len, gps, arg->fd) == 0) {
        if (gps_newpos(gps) == -1) return -1;
        if (gps_newsat(gps) == -1) return -1;
      }
      break;
    case IO_LINE:
      if (arg->buf[0] && gps->parse_line && gps->parse_line((char *)arg->buf, gps, arg->fd) == 0) {
        if (gps_newpos(gps) == -1) return -1;
        if (gps_newsat(gps) == -1) return -1;
      }
      break;
    case IO_CMD:
      if (gps->cmd) {
        return gps->cmd(arg->buf, arg->len, gps, arg->fd);
      }
      break;
    case IO_TIMEOUT:
      // force disconnect
      return 1;
    case IO_DISCONNECT:
      gps_callback(gps->pe, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, gps->ref);
      script_remove_ref(gps->pe, gps->ref);
      if (gps->priv) xfree(gps->priv);
      xfree(gps);
  }

  return 0;
}

int gps_client(gps_t *gps) {
  bt_provider_t *bt;

  bt = script_get_pointer(gps->pe, BT_PROVIDER);
  return sock_client(gps->tag, gps->pe, gps_sock_callback, 1, gps, bt);
}
