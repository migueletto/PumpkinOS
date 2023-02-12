#ifndef PIT_GPS_H
#define PIT_GPS_H

#ifdef __cplusplus
extern "C" {
#endif

#define GPS_PARSE_LINE_PROVIDER  "gps_parse_line"

#define MAX_SATS 32

struct gps_t;

typedef int (*gps_parse_data_f)(unsigned char *buf, int len, struct gps_t *gps, int fd);

typedef int (*gps_parse_line_f)(char *buf, struct gps_t *gps, int fd);

typedef int (*gps_cmd_f)(unsigned char *buf, int len, struct gps_t *gps, int fd);

typedef struct {
  int prn, elevation, azimuth, snr, used;
} gps_sat_t;

typedef struct gps_t {
  char *tag;
  int newpos;
  int mode;
  int day, month, year;
  int hour, min, sec;
  script_real_t lat, lon, height, speed, course;
  time_t ts;
  int pe;
  script_ref_t ref;
  void *priv;

  int nused;
  int used[MAX_SATS];
  int nsats, newsat;
  gps_sat_t sat[MAX_SATS];

  gps_parse_data_f parse_data;
  gps_parse_line_f parse_line;
  gps_cmd_f cmd;
} gps_t;

int gps_client(gps_t *gps);

#ifdef __cplusplus
}
#endif

#endif
