#include <PalmOS.h>

#include "gps.h"
#include "garmin.h"
#include "ndebug.h"

#define GARMIN_PRODUCT_ID 154 // eTrex Venture
#define GARMIN_SOFT_VERSION 234 // 2.34
#define GARMIN_UNIT_ID  'Host'

#define MODEL_EARTHMATE 1
#define MODEL_TRIPMATE 2

#define MAX_BUF  256
#define MAX_ARGS 64
#define PI  3.14159265358979323846
#define KNOT_TO_MS      (1852.0/3600.0)
#define M_TO_FEET 3.2808269

static uint16_t buf[MAX_BUF];
static uint8_t sendbuf[MAX_BUF], string[32], bbuf[MAX_BUF];
static uint8_t *arg[MAX_ARGS];
static int protocol = PROTOCOL_ZODIAC;
static int s = -1;
static int model = MODEL_EARTHMATE;
static int sendid = 1;
static int first_msg = 1;

static int32_t dlat, dlon, dheight;
static double lat0, lon0;
static uint16_t seq = 0;
static int32_t msgs, errors;
static uint16_t datum;
static ChannelSummary chs[MAX_CHANNELS];
static VisibleSatellite sat[MAX_CHANNELS];
static char index_sat[99];

#define PACKET_RCV 0
#define PACKET_SND 1

static char *direction[] = {"RCV", "SND"};

#define EXPORT_ONE_POINT 1
#define EXPORT_ALL_POINTS 2
#define EXPORT_ONE_ROUTE 3
#define EXPORT_ALL_ROUTES 4
#define EXPORT_ONE_TRACK 5
#define EXPORT_CURR_TRACK 6
#define EXPORT_ALL_TRACKS 7
#define IMPORT_ALL_POINTS 8
#define IMPORT_ALL_ROUTES 9
#define IMPORT_ALL_TRACKS 10

static int link_protocol, command_protocol;
static int app_protocol[8], data_protocol[8][3];
static int transfer;

#define GARMIN_ACTIVE_LOG "ACTIVE LOG"
static WaypointType point;
static RouteType rte;
static TracklogType log;
static D10X_Wpt_Type D10X;
static D20X_Rte_Hdr_Type D20X;
//static D700_Position_Type D700;

static int zodiac_pos(int32_t, int32_t, int32_t);
static int zodiac_msg(uint16_t cmd, uint16_t flags, uint16_t *buf, int n);
static int zodiac_send(uint16_t *, int);
static uint16_t zodiac_checksum(uint16_t *buf, int n);
static uint8_t hex_digit(uint8_t c);

static int garmin_msg(uint16_t id, uint8_t *buf, int n);
static int garmin_send(uint8_t id, uint8_t *data, uint8_t size);
static int garmin_ack(uint8_t id);
static int garmin_nack(uint8_t id);
static int garmin_send_unknown(uint16_t cmd);
static int garmin_log_d30x(int data_protocol, TracklogType *log,
                           int new_segment, uint8_t *data);
static int garmin_d30x_log(int data_protocol, TracklogType *log,
                           uint8_t *data, int n);
static int garmin_point_d10x(int data_protocol, WaypointType *point,
                             uint8_t *data);
static int garmin_d10x_point(int data_protocol, WaypointType *point,
                             uint8_t *data, int n);
static int garmin_rte_d20x(int data_protocol, RouteType *rte,
                           uint8_t *data);
static int garmin_d20x_rte(int data_protocol, RouteType *rte,
                           uint8_t *data, int n);
//static int garmin_pos_d700(double lat, double lon, uint8_t *data);
static int garmin_d80x_pvt(int data_protocol, uint8_t *data, int n);
static uint16_t garmin_map_symbol(int d, uint16_t s);
static uint16_t garmin_unmap_symbol(int d, uint16_t s);
static int garmin_assign_protocols(int id, int version);
static int garmin_wpt_data(WaypointType *point, uint8_t *data);
static int garmin_rte_hdr(RouteType *rte, uint8_t *data);
static int garmin_rte_data(WaypointType *point, uint8_t *data);
static int garmin_link_data(RouteType *rte, uint8_t *data);
static int garmin_trk_hdr(char *name);
static int garmin_trk_data(TracklogType *log, uint8_t *data, uint32_t t0);
//static int garmin_pos_data(double lat, double lon, uint8_t *data);
static int garmin_cmplt(int p);
static int garmin_protocol_array(uint8_t *data);

static int pack_word(uint16_t w, uint8_t *buf, int i);
static int pack_dword(uint32_t l, uint8_t *buf, int i);
static int pack_string(char *s, uint8_t *buf, int i);
static uint16_t unpack_word(uint8_t *buf);
static uint32_t unpack_dword(uint8_t *buf);
static float invert_float(float f);
static double invert_double(double d);
static void debug_command(int d, uint16_t cmd);
static void debug_ack(int d, int ack, uint16_t id);
static void debug_packet(int d, uint16_t id, uint8_t *buf, int n);

typedef struct {
  char *cmd;
  int (*handler)(uint8_t *arg[], int n);
} nmea_handler;

static int nmea_gpgga(uint8_t *arg[], int n);
static int nmea_gpgsa(uint8_t *arg[], int n);
static int nmea_gpgsv(uint8_t *arg[], int n);
static int nmea_gprmc(uint8_t *arg[], int n);
static int nmea_pgrme(uint8_t *arg[], int n);
static int nmea_pmgnst(uint8_t *arg[], int n);

static nmea_handler handler_tab[] = {
  {"GPGGA",  nmea_gpgga},
  {"GPGSA",  nmea_gpgsa},
  {"GPGSV",  nmea_gpgsv},
  {"GPRMC",  nmea_gprmc},
  {"PGRME",  nmea_pgrme},
  {"PMGNST", nmea_pmgnst},
  {0, 0}
};

typedef struct {
  int id;
  int version;
  int link;
  int cmnd;
  int wpt, wpt_d0;
  int rte, rte_d0, rte_d1;
  int trk, trk_d0;
  int prx, prx_d0;
  int alm, alm_d0;
} GarminProtocol;


static GarminProtocol garmin_protocol[] = {
{  7,   0, 1, 10, 100, 100, 200, 200, 100,  -1,  -1,  -1,  -1, 500, 500},
{ 13,   0, 1, 10, 100, 100, 200, 200, 100, 300, 300, 400, 400, 500, 500},
{ 14,   0, 1, 10, 100, 100, 200, 200, 100,  -1,  -1, 400, 400, 500, 500},
{ 15,   0, 1, 10, 100, 151, 200, 200, 151,  -1,  -1, 400, 151, 500, 500},
{ 18,   0, 1, 10, 100, 100, 200, 200, 100, 300, 300, 400, 400, 500, 500},
{ 20,   0, 2, 11, 100, 150, 200, 201, 150,  -1,  -1, 400, 450, 500, 550},
{ 22,   0, 1, 10, 100, 152, 200, 200, 152, 300, 300, 400, 152, 500, 500},
{ 23,   0, 1, 10, 100, 100, 200, 200, 100, 300, 300, 400, 400, 500, 500},
{ 24,   0, 1, 10, 100, 100, 200, 200, 100, 300, 300, 400, 400, 500, 500},
{ 25,   0, 1, 10, 100, 100, 200, 200, 100, 300, 300, 400, 400, 500, 500},
{ 29, 400, 1, 10, 100, 102, 200, 201, 102, 300, 300, 400, 102, 500, 500},
{ 29,   0, 1, 10, 100, 101, 200, 201, 101, 300, 300, 400, 101, 500, 500},
{ 31,   0, 1, 10, 100, 100, 200, 201, 100, 300, 300,  -1,  -1, 500, 500},
{ 33,   0, 2, 11, 100, 150, 200, 201, 150,   -1, -1, 400, 450, 500, 550},
{ 34,   0, 2, 11, 100, 150, 200, 201, 150,   -1, -1, 400, 450, 500, 550},
{ 35,   0, 1, 10, 100, 100, 200, 200, 100, 300, 300, 400, 400, 500, 500},
{ 36, 300, 1, 10, 100, 152, 200, 200, 152, 300, 300,   -1, -1, 500, 500},
{ 36,   0, 1, 10, 100, 152, 200, 200, 152, 300, 300, 400, 152, 500, 500},
{ 39,   0, 1, 10, 100, 151, 200, 201, 151, 300, 300,  -1,  -1, 500, 500},
{ 41,   0, 1, 10, 100, 100, 200, 201, 100, 300, 300,  -1,  -1, 500, 500},
{ 42,   0, 1, 10, 100, 100, 200, 200, 100, 300, 300, 400, 400, 500, 500},
{ 44,   0, 1, 10, 100, 101, 200, 201, 101, 300, 300, 400, 101, 500, 500},
{ 45,   0, 1, 10, 100, 152, 200, 201, 152, 300, 300,  -1,  -1, 500, 500},
{ 47,   0, 1, 10, 100, 100, 200, 201, 100, 300, 300,  -1,  -1, 500, 500},
{ 48,   0, 1, 10, 100, 154, 200, 201, 154, 300, 300,  -1,  -1, 500, 501},
{ 49,   0, 1, 10, 100, 102, 200, 201, 102, 300, 300, 400, 102, 500, 501},
{ 50,   0, 1, 10, 100, 152, 200, 201, 152, 300, 300,  -1,  -1, 500, 501},
{ 52,   0, 2, 11, 100, 150, 200, 201, 150,  -1,  -1, 400, 450, 500, 550},
{ 53,   0, 1, 10, 100, 152, 200, 201, 152, 300, 300,  -1,  -1, 500, 501},
{ 55,   0, 1, 10, 100, 100, 200, 201, 100, 300, 300,  -1,  -1, 500, 500},
{ 56,   0, 1, 10, 100, 100, 200, 201, 100, 300, 300,  -1,  -1, 500, 500},
{ 59,   0, 1, 10, 100, 100, 200, 201, 100, 300, 300,  -1,  -1, 500, 500},
{ 61,   0, 1, 10, 100, 100, 200, 201, 100, 300, 300,  -1,  -1, 500, 500},
{ 62,   0, 1, 10, 100, 100, 200, 201, 100, 300, 300,  -1,  -1, 500, 500},
{ 64,   0, 2, 11, 100, 150, 200, 201, 150,  -1,  -1, 400, 450, 500, 551},
{ 71,   0, 1, 10, 100, 155, 200, 201, 155, 300, 300,  -1,  -1, 500, 501},
{ 72,   0, 1, 10, 100, 104, 200, 201, 104, 300, 300,  -1,  -1, 500, 501},
{ 73,   0, 1, 10, 100, 103, 200, 201, 103, 300, 300,  -1,  -1, 500, 501},
{ 74,   0, 1, 10, 100, 100, 200, 201, 100, 300, 300,  -1,  -1, 500, 500},
{ 76,   0, 1, 10, 100, 102, 200, 201, 102, 300, 300, 400, 102, 500, 501},
{ 77, 361, 1, 10, 100, 103, 200, 201, 103, 300, 300, 400, 403, 500, 501},
{ 77, 350, 1, 10, 100, 103, 200, 201, 103, 300, 300,  -1,  -1, 500, 501},
{ 77, 301, 1, 10, 100, 103, 200, 201, 103, 300, 300, 400, 403, 500, 501},
{ 77,   0, 1, 10, 100, 100, 200, 201, 100, 300, 300, 400, 400, 500, 501},
{ 87,   0, 1, 10, 100, 103, 200, 201, 103, 300, 300, 400, 403, 500, 501},
{ 88,   0, 1, 10, 100, 102, 200, 201, 102, 300, 300, 400, 102, 500, 501},
{ 95,   0, 1, 10, 100, 103, 200, 201, 103, 300, 300, 400, 403, 500, 501},
{ 96,   0, 1, 10, 100, 103, 200, 201, 103, 300, 300, 400, 403, 500, 501},
{ 97,   0, 1, 10, 100, 103, 200, 201, 103, 300, 300,  -1,  -1, 500, 501},
{ 98,   0, 2, 11, 100, 150, 200, 201, 150,  -1,  -1, 400, 450, 500, 551},
{100,   0, 1, 10, 100, 103, 200, 201, 103, 300, 300, 400, 403, 500, 501},
{105,   0, 1, 10, 100, 103, 200, 201, 103, 300, 300, 400, 403, 500, 501},
{106,   0, 1, 10, 100, 103, 200, 201, 103, 300, 300, 400, 403, 500, 501},
{112,   0, 1, 10, 100, 152, 200, 201, 152, 300, 300,  -1,  -1, 500, 501},
{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}
};

static int gps_strcmp(char *s1, char *s2);
static int gps_strncpy(char *dst, char *src, int n, int fill, int convert);
static int gps_strlen(char *s);
static void gps_memcpy(char *dst, char *src, int n);
static void gps_memset(char *s, char c, int n);

static uint8_t dle = 0x10;
static uint8_t eot = 0x03;

int gps_buf(uint8_t *rbuf, int n)
{
  int k;
  uint8_t c;
  uint16_t word;
  static int i, inm;
  static uint16_t cmd, len, flags, sum;
  static uint8_t id, bsum;

  DebugMsg("RCV SERIAL %d", n);
  DebugBytes(rbuf, n);

  for (k = 0; k < n; k++) {
    c = rbuf[k];
    switch (s) {
      case -1:
        if (c == 'E' && protocol == PROTOCOL_ZODIAC) s = -2;
        else if (c == 'A' && protocol == PROTOCOL_ZODIAC) s = -10;
        else if (c == dle && (protocol == PROTOCOL_GARMIN ||
                 protocol == PROTOCOL_GARMIN_HOST)) s = 200;
        else if (c == '$' && protocol == PROTOCOL_NMEA) s = 100, inm = 0;
        break;
      case -2:
        if (c == 'A') s = -3; else s = -1;
        break;
      case -3:
        if (c == 'R') s = -4; else s = -1;
        break;
      case -4:
        if (c == 'T') s = -5; else s = -1;
        break;
      case -5:
        if (c == 'H') s = -6; else s = -1;
        break;
      case -6:
        if (c == 'A') {
          if (protocol != PROTOCOL_ZODIAC) {
            s = -1;
            SetErrors(10);
          } else {
            s = 0;
            Send((unsigned char *)"EARTHA\r\n", 8);
            model = MODEL_EARTHMATE;
          }
        } else
          s = -1;
        break;
      case -10:
        if (c == 'S') s = -11; else s = -1;
        break;
      case -11:
        if (c == 'T') s = -12; else s = -1;
        break;
      case -12:
        if (c == 'R') s = -13; else s = -1;
        break;
      case -13:
        if (c == 'A') s = -14; else s = -1;
        break;
      case -14:
        if (c == 'L') {
          if (protocol != PROTOCOL_ZODIAC) {
            s = -1;
            SetErrors(10);
          } else {
            s = 0;
            Send((unsigned char *)"ASTRAL\r\n", 8);
            model = MODEL_TRIPMATE;
          }
        } else
          s = -1;
        break;
      case 0:
        if (c == 0xFF)
          s = 1;
        break;
      case 1:
        if (c == 0x81)
          s = 2, cmd = len = flags = sum = 0;
        else
          s = 0;
        break;
      case 2:
        s = 3;
        cmd = c;
        break;
      case 3:
        s = 4;
        cmd |= (((uint16_t)c)<<8);
        break;
      case 4:
        s = 5;
        len = c;
        break;
      case 5:
        s = 6;
        len |= (((uint16_t)c)<<8);
        break;
      case 6:
        s = 7;
        flags = c;
        break;
      case 7:
        s = 8;
        flags |= (((uint16_t)c)<<8);
        break;
      case 8:
        s = 9;
        sum = c;
        break;
      case 9:
        sum |= (((uint16_t)c)<<8);
        if (!len) {
          s = 0;
          zodiac_msg(cmd, flags, buf, 0);
          SetMessages(++msgs);
        } else
          s = 11, len *= 2, i = 0;
        break;
      case 11:
        if (i % 2)
          buf[i/2] |= (((uint16_t)c)<<8);
        else
          buf[i/2] = c;
        i++;
        len--;
        if (len == 0)
          s = 12;
        break;
      case 12:
        s = 13;
        sum = c;
        break;
      case 13:
        s = 0;
        sum |= (((uint16_t)c)<<8);
        word = zodiac_checksum(buf, i/2);
        if (sum == word) {
          zodiac_msg(cmd, flags, buf, i/2);
          SetMessages(++msgs);
        } else
          SetErrors(++errors);
        break;
      case 100: // NMEA
        if (c == 13)
          s = 101;
        else if (inm < MAX_BUF)
          bbuf[inm++] = c;
        break;
      case 101:
        if (protocol != PROTOCOL_NMEA)
          SetErrors(10);
        else if (c == 10)
          nmea_sentence(bbuf, inm);
        s = -1;
        break;
      case 200: // Garmin binary protocol
        bsum = c;
        id = c;
        s = 201;
        break;
      case 201:
        bsum += c;
        len = c;
        i = 0;
        if (len == 0)
          s = 203;
        else
          s = (c == dle) ? 1202 : 202;
        break;
      case 202:
        bsum += c;
        bbuf[i++] = c;
        len--;
        if (len == 0)
          s = (c == dle) ? 1203 : 203;
        else
          s = (c == dle) ? 1202 : 202;
        break;
      case 203:
        bsum = (bsum ^ 0xff)+1;
        if (c == bsum) {
          if (protocol == PROTOCOL_GARMIN || protocol == PROTOCOL_GARMIN_HOST)
            garmin_msg(id, bbuf, i);
        } else {
          SetErrors(++errors);
          garmin_nack(id);
        }
        s = (c == dle) ? 1204 : 204;
        break;
      case 204:
        s = 205;
        break;
      case 205:
        if (protocol != PROTOCOL_GARMIN && protocol != PROTOCOL_GARMIN_HOST)
          SetErrors(10);
        s = -1;
        break;
      case 1202:
         s -= 1000;
         break;
      case 1203:
         s -= 1000;
         break;
      case 1204:
         s -= 1000;
         break;
    }
  }

  return 0;
}

static int zodiac_msg(uint16_t cmd, uint16_t flags, uint16_t *buf, int n)
{
  int i, k;
  uint32_t udi;
  uint16_t failure, *p;
  int32_t di;
  double fv, fv2;

  SetAlive();

  switch (cmd) {
    case 1000:
      if (first_msg) {
        zodiac_msgrequest(1011);
        zodiac_setdatum(0); // zodiac always uses WGS84
        zodiac_msgenable(1000, 1);
        zodiac_msgenable(1002, 1);
        zodiac_msgenable(1003, 1);
        zodiac_pos(dlat, dlon, dheight);
        first_msg = 0;
        return 0;
      }

      SetValidity(!buf[4], buf[6]);
      SetUTC(buf[13], buf[14], buf[15], buf[16], buf[17], buf[18]);

      udi = buf[22];
      udi = (udi<<16) | buf[21];
      fv = (double)((int32_t)udi) / 10000.0;
      fv = fv / 10000.0;
      fv = fv * 180.0 / PI;

      udi = buf[24];
      udi = (udi<<16) | buf[23];
      fv2 = (double)((int32_t)udi) / 10000.0;
      fv2 = fv2 / 10000.0;
      fv2 = fv2 * 180.0 / PI;

      SetPosition(fv, fv2);

      udi = buf[26];
      udi = (udi<<16) | buf[25];
      fv = (double)((int32_t)udi) / 100.0;
      SetHeight(fv);

      di = (int16_t)buf[27];
      fv = (double)(di) / 100.0;
      SetSep(fv);

      udi = buf[29];
      udi = (udi<<16) | buf[28];
      fv = (double)(udi) / 100.0;
      SetSpeed(fv);

      udi = buf[30];
      fv = (double)(udi) / 1000.0;
      fv = fv * 180.0 / PI;
      if (fv > 180.0)
        fv -= 360.0;
      SetCourse(fv);

      di = (int16_t)buf[31];
      fv = (double)(di) / 10000.0;
      fv = fv * 180.0 / PI;
      if (fv > 180.0)
        fv -= 360.0;
      SetVar(fv);

      di = (int16_t)buf[32];
      fv = (double)(di) / 100.0;
      SetClimb(fv);

      SetDatum(buf[33]);

      di = buf[35];
      di = (di<<16) | buf[34];
      fv = (double)(di) / 100.0;
      di = buf[37];
      di = (di<<16) | buf[36];
      fv2 = (double)(di) / 100.0;
      SetEPE(fv, fv2);

      if (!buf[4])
        SaveData();
      break;

    case 1002:
      for (i = 0; i < MAX_CHANNELS; i++) {
        chs[i].flags = buf[9+3*i];
        chs[i].prn = buf[10+3*i];
        chs[i].cno = buf[11+3*i]; // dBHZ (0-60)
        if (chs[i].cno > 60)
          chs[i].cno = 60;
      }
      SetChannelSummary(MAX_CHANNELS, chs);
      break;

    case 1003:
      for (i = 0; i < buf[8]; i++) {
        sat[i].prn = buf[9+3*i];

        di = buf[10+3*i];
        fv = (double)(di) / 10000.0;
        fv = fv * 180.0 / PI;
        if (fv > 180.0)
          fv -= 360.0;
        sat[i].azimuth = fv;

        di = buf[11+3*i];
        fv = (double)(di) / 10000.0;
        fv = fv * 180.0 / PI;
        if (fv > 180.0)
          fv -= 360.0;
        sat[i].elevation = fv;
      }
      SetDOP((double)(buf[3]) / 100.0,
             (double)(buf[4]) / 100.0,
             (double)(buf[5]) / 100.0,
             (double)(buf[6]) / 100.0,
             (double)(buf[7]) / 100.0);
      SetVisible(buf[8], sat);
      break;

    case 1011:
      p = &buf[13];
      for (i = 0, k = 0; i < 10; i++) {
        string[k++] = (uint8_t)(p[i] & 0x00ff); 
        string[k++] = (uint8_t)(p[i] >> 8); 
      }
      string[k] = 0;
      SetDevice(STR_DELORME,
                model == MODEL_EARTHMATE ? STR_EARTHMATE : STR_TRIPMATE,
                (char *)string);
      break;

    case 1100: // built-in test result
      failure = 0;
      if (buf[3])
        failure |= FAILURE_ROM;
      if (buf[4])
        failure |= FAILURE_RAM;
      if (buf[5])
        failure |= FAILURE_EEPROM;
      if (buf[6])
        failure |= FAILURE_DUAL_PORT_RAM;
      if (buf[7])
        failure |= FAILURE_DSP;
      if (buf[8])
        failure |= FAILURE_RTC;

      SetFailure(failure);
      break;

    default:
      break;
  }

  return 0;
}

int gps_protocol(int p)
{
  protocol = p;
  return 0;
}

int gps_init(double lat, double lon, double height, uint16_t _datum)
{
  int i;

  sendid = 1;
  s = -1;
  SetMessages(msgs = 0);
  SetErrors(errors = 0);
  first_msg = 1;

  lat = lat * PI / 180.0;
  lat0 = lat;
  lat = lat * 10000.0;
  dlat = (int32_t)(lat * 10000.0);

  lon = lon * PI / 180.0;
  lon0 = lon;
  lon = lon * 10000.0;
  dlon = (int32_t)(lon * 10000.0);

  dheight = (int32_t)(height);
  datum = _datum;

  for (i = 0; i < 3; i++) {
    data_protocol[POINT_PROTOCOL][i] = -1;
    data_protocol[ROUTE_PROTOCOL][i] = -1;
    data_protocol[TRACK_PROTOCOL][i] = -1;
    data_protocol[PVT_PROTOCOL][i] = -1;
  }

  if (protocol == PROTOCOL_GARMIN) {
    link_protocol = -1;
    command_protocol = -1;
    app_protocol[POINT_PROTOCOL] = -1;
    app_protocol[ROUTE_PROTOCOL] = -1;
    app_protocol[TRACK_PROTOCOL] = -1;
    app_protocol[PVT_PROTOCOL] = -1;

    garmin_product_rqst();

  } else if (protocol == PROTOCOL_GARMIN_HOST) {
    link_protocol = 1;
    command_protocol = 10;

    app_protocol[POINT_PROTOCOL] = 100;
    data_protocol[POINT_PROTOCOL][0] = 108;

    app_protocol[ROUTE_PROTOCOL] = 200;
    data_protocol[ROUTE_PROTOCOL][0] = 202;
    data_protocol[ROUTE_PROTOCOL][1] = 108;

    app_protocol[TRACK_PROTOCOL] = 301;
    data_protocol[TRACK_PROTOCOL][0] = 310;
    data_protocol[TRACK_PROTOCOL][1] = 301;

    app_protocol[PVT_PROTOCOL] = -1;

    SetProtocol(LINK_PROTOCOL, link_protocol, 0);
    SetProtocol(POINT_PROTOCOL,
      app_protocol[POINT_PROTOCOL], data_protocol[POINT_PROTOCOL]);
    SetProtocol(ROUTE_PROTOCOL,
      app_protocol[ROUTE_PROTOCOL], data_protocol[ROUTE_PROTOCOL]);
    SetProtocol(TRACK_PROTOCOL,
      app_protocol[TRACK_PROTOCOL], data_protocol[TRACK_PROTOCOL]);
  }

  SetHandshake(0);
  return 0;
}

static uint16_t zodiac_checksum(uint16_t *buf, int n)
{
  int i;
  int16_t sum;

  for (i = 0, sum = 0; i < n; i++)
    sum += buf[i];

  if (sum != -32768)
    sum = -sum;

  return sum;
}

static int zodiac_pos(int32_t dlat, int32_t dlon, int32_t dheight)
{
  uint16_t word[27];
  unsigned char day, month, hour, minute, second;
  uint16_t year;

  if (seq > 32767)
    seq = 0;

  GetUTC(&day, &month, &year, &hour, &minute, &second);

  word[0] = 0x81FF;
  word[1] = 1200;
  word[2] = 21;
  word[3] = 0x0000;
  word[4] = zodiac_checksum(word, 4);
  word[5] = seq++;
  word[6] = 0x001C;

  word[7] = 0;
  word[8] = 0;
  word[9] = 0;

  word[10] = day;
  word[11] = month;
  word[12] = year;
  word[13] = hour;
  word[14] = minute;
  word[15] = second;

  word[16] = dlat & 0xFFFF;
  word[17] = dlat >> 16;
  word[18] = dlon & 0xFFFF;
  word[19] = dlon >> 16;
  word[20] = dheight & 0xFFFF;
  word[21] = dheight >> 16;

  word[20] = 0;
  word[21] = 0;
  word[22] = 0;
  word[23] = 0;
  word[24] = 0;
  word[25] = 0;

  word[26] = zodiac_checksum(&word[5], 21);

  return zodiac_send(word, 27);
}

int zodiac_test(void)
{
  uint16_t word[8];

  if (seq > 32767)
    seq = 0;

  word[0] = 0x81FF;
  word[1] = 1300;
  word[2] = 2;
  word[3] = 0x0000;
  word[4] = zodiac_checksum(word, 4);
  word[5] = seq++;
  word[6] = 0; // reserved
  word[7] = zodiac_checksum(&word[5], 2);

  return zodiac_send(word, 8);
}

int zodiac_msgenable(uint16_t id, int on)
{
  uint16_t word[5];

  word[0] = 0x81FF;
  word[1] = id;
  word[2] = 0;
  word[3] = on ? 0x4000 : 0x8000;
  word[4] = zodiac_checksum(word, 4);

  return zodiac_send(word, 5);
}

int zodiac_msgrequest(uint16_t id)
{
  uint16_t word[5];
  
  word[0] = 0x81FF;
  word[1] = id;
  word[2] = 0;
  word[3] = 0x4800;
  word[4] = zodiac_checksum(word, 4);

  return zodiac_send(word, 5);
}

int zodiac_setdatum(uint16_t datum)
{
  uint16_t word[8];

  if (seq > 32767)
    seq = 0;

  word[0] = 0x81FF;
  word[1] = 1211;
  word[2] = 2;
  word[3] = 0x0000;
  word[4] = zodiac_checksum(word, 4);
  word[5] = seq++;
  word[6] = datum;
  word[7] = zodiac_checksum(&word[5], 2);

  return zodiac_send(word, 8);
}

int zodiac_restart(uint16_t flags)
{
  uint16_t word[8];

  if (seq > 32767)
    seq = 0;

  word[0] = 0x81FF;
  word[1] = 1303;
  word[2] = 2;
  word[3] = 0x0000;
  word[4] = zodiac_checksum(word, 4);
  word[5] = seq++;
  word[6] = flags;
  word[7] = zodiac_checksum(&word[5], 2);

  return zodiac_send(word, 8);
}

static int zodiac_send(uint16_t *word, int n)
{
  int i, k;

  for (i = 0, k = 0; i < n; i++) {
    sendbuf[k++] = word[i] & 0xFF;
    sendbuf[k++] = word[i] >> 8;
  }

  Send(sendbuf, k);
  return 0;
}

static uint8_t hex_digit(uint8_t c)
{
  uint8_t digit = 0;

  if (c >= '0' && c <= '9') digit = c-'0';
  else if (c >= 'A' && c <= 'F') digit = c-'A'+10;
  else if (c >= 'a' && c <= 'f') digit = c-'a'+10;

  return digit;
}

int nmea_sentence(uint8_t *rbuf, int n)
{
  int i, k;
  uint8_t *cmd, sum1, sum2;

  cmd = &rbuf[0];
  sum1 = 0;

  for (i = 0, k = 0; i < n && k < MAX_ARGS; i++) {
    if (rbuf[i] == '*') {
      rbuf[i] = 0;
      sum2 = hex_digit(rbuf[i+1])*16 + hex_digit(rbuf[i+2]);
      if (sum1 != sum2) {
        SetErrors(++errors);
        return -1;
      }
      break;
    }
    sum1 ^= rbuf[i];
    if (rbuf[i] == ',') {
      rbuf[i] = 0;
      arg[k++] = &rbuf[i+1];
    }
  }

  SetAlive();

  if (k > 0)
    for (i = 0; handler_tab[i].cmd; i++)
      if (!gps_strcmp((char *)cmd, handler_tab[i].cmd))
        return handler_tab[i].handler(arg, k);

  return 0;
}

static int nmea_gpgga(uint8_t *arg[], int n)
{
  //        0      1        2 3         4 5 6  7   8     9 0    1
  // $GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47

  double height;

  if (n < 9)
    return -1;

  if (gps_strlen((char *)arg[8]) >= 2) {
    height = sys_atof((char *)arg[8]);
    if (arg[9][0] == 'f' || arg[9][0] == 'F')
      height /= M_TO_FEET;
    SetHeight(height);
  }

  return 0;
}

static int nmea_gpgsa(uint8_t *arg[], int n)
{
  // $GPGSA,A,3,04,05,,09,12,,,24,,,,,2.5,1.3,2.1*39

  int i, nsats;

  if (n < 17)
    return -1;

  nsats = 0;

  for (i = 0; i < 99; i++) {
    index_sat[i] = 0;
  }

  for (i = 0; i < 12; i++) {
    if (arg[2+i][0]) {
      index_sat[StrAToI((char *)arg[2+i])] = 1;
      nsats++;
    }
  }

  SetValidity(arg[1][0] != '1' ? 1 : 0, nsats);

  if (gps_strlen((char *)arg[14]) >= 2 && gps_strlen((char *)arg[15]) >= 2 &&
      gps_strlen((char *)arg[16]) >= 2) {
    SetDOP(0.0, sys_atof((char *)arg[14]), sys_atof((char *)arg[15]), sys_atof((char *)arg[16]), 0.0);
  }

  return 0;
}

static int nmea_gpgsv(uint8_t *arg[], int n)
{
  // $GPGSV,2,1,08,01,40,083,46,02,17,308,41,12,07,344,39,14,22,228,45*75

  int i, b, t, ngsv, igsv, nview;

  if (n < 7)
    return -1;

  ngsv = StrAToI((char *)arg[0]);
  igsv = StrAToI((char *)arg[1])-1;
  nview = StrAToI((char *)arg[2]);

  b = 4*igsv;
  t = nview - b;
  if (t > 4) t = 4;

  if (n < (3 + 4*t))
    return -1;

  for (i = 0; i < t && (b+i) < MAX_CHANNELS; i++) {
    sat[b+i].prn = StrAToI((char *)arg[3 + i*4]);
    sat[b+i].elevation = sys_atof((char *)arg[4 + i*4]);
    sat[b+i].azimuth = sys_atof((char *)arg[5 + i*4]);

    chs[b+i].prn = sat[b+i].prn;
    chs[b+i].cno = StrAToI((char *)arg[6 + i*4]); // dBHZ (0-50) -> (0-60)
    if (chs[i].cno > 50)
      chs[i].cno = 50;
    chs[b+i].cno = (chs[b+i].cno * 60) / 50;
    chs[b+i].flags = 0;
    if (chs[b+i].cno > 0)
      chs[b+i].flags |= CHANNEL_VALID;
    if (index_sat[chs[b+i].prn])
      chs[b+i].flags |= CHANNEL_USED;
  }

  if ((igsv+1) == ngsv) {
    SetChannelSummary(MAX_CHANNELS, chs);
    SetVisible(nview, sat);
  }

  return 0;
}

static int nmea_gprmc(uint8_t *arg[], int n)
{
  // $GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A

  double lat, lon, var;
  char buf[8], day, month, hour, minute, second;
  int16_t year;

  if (n < 11)
    return -1;

  if (sendid) {
    sendid = 0;
    SetDevice(STR_GENERIC, STR_NMEA, "");
  }

  if (gps_strlen((char *)arg[0]) == 6 && gps_strlen((char *)arg[8]) == 6) {
    hour = 10*(arg[0][0]-'0') + arg[0][1]-'0';
    minute = 10*(arg[0][2]-'0') + arg[0][3]-'0';
    second = 10*(arg[0][4]-'0') + arg[0][5]-'0';

    day = 10*(arg[8][0]-'0') + arg[8][1]-'0';
    month = 10*(arg[8][2]-'0') + arg[8][3]-'0';
    year = 10*(arg[8][4]-'0') + arg[8][5]-'0';
    year += year < 80 ? 2000: 1900;

    SetUTC(day, month, year, hour, minute, second);
  }

  if (gps_strlen((char *)arg[2]) >= 3 && gps_strlen((char *)arg[2]) >= 4) {
    buf[0] = arg[2][0];
    buf[1] = arg[2][1];
    buf[2] = 0;
    lat = sys_atof(buf);
    lat += sys_atof((char *)&arg[2][2]) / 60.0;
    if (arg[3][0] == 'S')
      lat = -lat;

    buf[0] = arg[4][0];
    buf[1] = arg[4][1];
    buf[2] = arg[4][2];
    buf[3] = 0;
    lon = sys_atof(buf);
    lon += sys_atof((char *)&arg[4][3]) / 60.0;
    if (arg[5][0] == 'W')
      lon = -lon;
        
    SetPosition(lat, lon);
  }

  if (gps_strlen((char *)arg[6]) >= 2)
    SetSpeed(sys_atof((char *)arg[6]) * KNOT_TO_MS);

  if (gps_strlen((char *)arg[9]) >= 2) {
    var = sys_atof((char *)arg[9]);
    if (arg[10][0] == 'W')
      var = -var;
    SetVar(var);
  }

  if (gps_strlen((char *)arg[7]) >= 2)
    SetCourse(sys_atof((char *)arg[7]));

  SaveData();

  return 0;
}

static int nmea_pgrme(uint8_t *arg[], int n)
{
  // $PGRME,8.6,M,9.6,M,12.9,M*15

  if (gps_strlen((char *)arg[0]) >= 2 && gps_strlen((char *)arg[2]))
    SetEPE(sys_atof((char *)arg[0]), sys_atof((char *)arg[2]));

  return 0;
}

static int nmea_pmgnst(uint8_t *arg[], int n)
{
  // $PMGNST,02.12,3,T,534,05.0,+03327,00*40

  return 0;
}

int garmin_product_rqst(void)
{
  return garmin_send(L000_Product_Rqst, 0, 0);
}

int garmin_command(uint16_t cmd)
{
  uint8_t buf[2];

  if (command_protocol != 10)
    return -1;

  debug_command(PACKET_SND, cmd);
  pack_word(cmd, buf, 0);

  switch (link_protocol) {
    case 1:
      return garmin_send(L001_Command_Data, buf, 2);
    case 2:
      return garmin_send(L002_Command_Data, buf, 2);
  }

  return -1;
}

int garmin_start_pvt(void)
{
  return garmin_command(Cmnd_Start_Pvt_Data);
}

int garmin_stop_pvt(void)
{
  return garmin_command(Cmnd_Stop_Pvt_Data);
}

int garmin_transfer_disp(void)
{
  return garmin_command(Cmnd_Transfer_Disp);
}

int garmin_transfer_wpt(void)
{
  switch (link_protocol) {
    case 1:
      return garmin_command(Cmnd_Transfer_Wpt);
      break;
    case 2:
      return garmin_command(Cmnd2_Transfer_Wpt);
  }
  return -1;
}

int garmin_send_wpt(void)
{
  uint8_t buf[2];
  uint16_t id;

  transfer = EXPORT_ONE_POINT;

  switch (link_protocol) {
    case 1: id = L001_Records; break;
    case 2: id = L002_Records; break;
    default: return -1;
  }

  if (GetNumPoints() > 0) {
    pack_word(1, buf, 0);
    BeginExport(1);
  } else {
    pack_word(0, buf, 0);
    BeginExport(0);
  }

  return garmin_send(id, buf, 2);
}

int garmin_send_wpts(void)
{
  uint8_t buf[2];
  uint16_t id;
  int n;

  transfer = EXPORT_ALL_POINTS;

  switch (link_protocol) {
    case 1: id = L001_Records; break;
    case 2: id = L002_Records; break;
    default: return -1;
  }

  if ((n = GetNumPoints()) > 0) {
    pack_word(n, buf, 0);
    BeginExport(n);
  } else {
    pack_word(0, buf, 0);
    BeginExport(0);
  }

  return garmin_send(id, buf, 2);
}

int garmin_transfer_rte(void)
{
  switch (link_protocol) {
    case 1:
      return garmin_command(Cmnd_Transfer_Rte);
      break;
    case 2:
      return garmin_command(Cmnd2_Transfer_Rte);
  }
  return -1;
}

int garmin_send_rte(void)
{
  uint8_t buf[2];
  uint16_t id;
  int m, n;

  switch (link_protocol) {
    case 1: id = L001_Records; break;
    case 2: id = L002_Records; break;
    default: return -1;
  }

  if (GetNumRoutes() == 0)
    return 0;

  if ((m = GetSelectedRoute(0)) == 0)
    return 0;

  n = 1 + m;
  if (app_protocol[ROUTE_PROTOCOL] == 201 && m > 1)
    n += m - 1;

  transfer = EXPORT_ONE_ROUTE;

  pack_word(n, buf, 0);
  BeginExport(n);

  return garmin_send(id, buf, 2);
}

int garmin_send_rtes(void)
{
  uint8_t buf[2];
  uint16_t id;
  int i, r, m, n;

  switch (link_protocol) {
    case 1: id = L001_Records; break;
    case 2: id = L002_Records; break;
    default: return -1;
  }
  
  if ((r = GetNumRoutes()) == 0)
    return 0;
  
  for (i = 0, n = 0; i < r; i++) {
    m = GetRoute(i, 0);
    n += 1 + m;
    if (app_protocol[ROUTE_PROTOCOL] == 201 && m > 1)
      n += m - 1;
  }

  transfer = EXPORT_ALL_ROUTES;
  
  pack_word(n, buf, 0);
  BeginExport(n);
  
  return garmin_send(id, buf, 2);
}

int garmin_transfer_trk(void)
{
  switch (link_protocol) {
    case 1:
      return garmin_command(Cmnd_Transfer_Trk);
  }

  return -1;
}

int garmin_send_trk(void)
{
  uint8_t buf[2];
  uint16_t id;
  int n;

  switch (link_protocol) {
    case 1: id = L001_Records; break;
    case 2: id = L002_Records; break;
    default: return -1;
  }

  if (GetNumTracks() <= 1)
    return 0;

  if ((n = GetSelectedTrack(0)) == 0)
    return 0;

  transfer = EXPORT_ONE_TRACK;

  if (app_protocol[TRACK_PROTOCOL] == 301 ||
      app_protocol[TRACK_PROTOCOL] == 302)
    n++; // header + n points

  pack_word(n, buf, 0);
  BeginExport(n);

  return garmin_send(id, buf, 2);
}

int garmin_send_curr_trk(void)
{
  uint8_t buf[2];
  uint16_t id;
  int n;

  switch (link_protocol) {
    case 1: id = L001_Records; break;
    case 2: id = L002_Records; break;
    default: return -1;
  }

  if ((n = GetTrack(0, 0)) == 0)
    return 0;

  transfer = EXPORT_CURR_TRACK;

  if (app_protocol[TRACK_PROTOCOL] == 301 ||
      app_protocol[TRACK_PROTOCOL] == 302)
    n++; // header + n points

  pack_word(n, buf, 0);
  BeginExport(n);

  return garmin_send(id, buf, 2);
}

int garmin_send_trks(void)
{
  uint8_t buf[2]; 
  uint16_t id;
  int i, n, m, t;
          
  switch (link_protocol) {
    case 1: id = L001_Records; break;
    case 2: id = L002_Records; break;
    default: return -1;
  }

  transfer = EXPORT_ALL_TRACKS;
  n = 0;

  if ((t = GetNumTracks()) > 0) {
    for (i = 0; i < t; i++) {
      m = GetTrack(i, 0);
      n += m;
      if (app_protocol[TRACK_PROTOCOL] == 301 ||
          app_protocol[TRACK_PROTOCOL] == 302)
        n++;
    }
  }

  if (!n)
    return 0;

  pack_word(n, buf, 0);
  BeginExport(n);

  return garmin_send(id, buf, 2);
}

int garmin_abort_transfer(void)
{
  switch (link_protocol) {
    case 1:
      return garmin_command(Cmnd_Abort_Transfer);
      break;
    case 2:
      return garmin_command(Cmnd2_Abort_Transfer);
  }

  return -1;
}

static int garmin_msg(uint16_t id, uint8_t *buf, int n)
{
  int i, k, m, p, d;
  int bpp, width, height;
  uint8_t data[128];
  uint16_t w, ack_id;
  static uint32_t t0 = 0;
  static int rec = 0, nrecs = 0, nlogs = 0, nrpoints = 0;
  static int current_track = -1;

  SetAlive();
  p = d = 0;

  debug_packet(PACKET_RCV, id, buf, n);

  if (link_protocol == 2 && id == L002_Records)
    id = L002_Records_1000;

  switch (id) {
    case L000_Ack_Byte:
    case L000_Nak_Byte:
      ack_id = buf[0];
      debug_ack(PACKET_RCV, id == L000_Ack_Byte ? 1 : 0, ack_id);

      if (link_protocol == 2 && ack_id == L002_Records)
        ack_id = L002_Records_1000;

      switch (ack_id) {
        case L001_Records:
        case L002_Records_1000:
          switch (transfer) {
            case EXPORT_ONE_POINT:
              rec = nrecs = 0;
              if (GetNumPoints() > 0 && GetSelectedPoint(&point) > 0) {
                nrecs = 1;
                garmin_wpt_data(&point, data);
              } else
                garmin_cmplt(POINT_PROTOCOL);
              break;
            case EXPORT_ALL_POINTS:
              rec = nrecs = 0;
              if ((nrecs = GetNumPoints()) > 0 && GetPoint(0, &point) > 0)
                garmin_wpt_data(&point, data);
              else {
                nrecs = 0;
                garmin_cmplt(POINT_PROTOCOL);
              }
              break;
            case EXPORT_ONE_ROUTE:
              switch (app_protocol[ROUTE_PROTOCOL]) {
                case 200:
                case 201:
                  rec = 0;
                  nrecs = 1;
                  nrpoints = 0;
                  GetSelectedRoute(&rte);
                  garmin_rte_hdr(&rte, data);
                  break;
                default:
                  rec = nrecs = 0;
                  garmin_cmplt(ROUTE_PROTOCOL);
              }
              break;
            case EXPORT_ALL_ROUTES:
              switch (app_protocol[ROUTE_PROTOCOL]) {
                case 200:
                case 201:
                  rec = 0;
                  nrecs = GetNumRoutes();
                  nrpoints = 0;
                  GetRoute(0, &rte);
                  IncCounter();
                  garmin_rte_hdr(&rte, data);
                  break;
                default:
                  rec = nrecs = 0;
                  garmin_cmplt(ROUTE_PROTOCOL);
              }
              break;
            case EXPORT_ONE_TRACK:
              switch (app_protocol[TRACK_PROTOCOL]) {
                case 300:
                  if (GetTrackLog(&log)) {
                    rec = 0, nrecs = 1, t0 = 0, nlogs = 0;
                    garmin_trk_data(&log, data, t0);
                  } else {
                    rec = nrecs = 0;
                    garmin_cmplt(TRACK_PROTOCOL);
                  }
                  break;
                case 301:
                case 302:
                  rec = 0, nrecs = 1, t0 = 0, nlogs = 0;
                  k = GetSelectedTrack((char *)data);
                  garmin_trk_hdr((char *)data);
                  break;
                default:
                  rec = nrecs = 0;
                  garmin_cmplt(TRACK_PROTOCOL);
              }
              break;
            case EXPORT_ALL_TRACKS:
            case EXPORT_CURR_TRACK:
              nrecs = (transfer == EXPORT_ALL_TRACKS) ? GetNumTracks() : 1;
              for (rec = 0; rec < nrecs; rec++) {
                if (GetTrack(rec, (char *)data) > 0)
                  break;
              }
              if (rec < nrecs) {
                switch (app_protocol[TRACK_PROTOCOL]) {
                  case 300:
                    if (GetTrackLog(&log)) {
                      t0 = 0, nlogs = 0;
                      garmin_trk_data(&log, data, t0);
                    } else {
                      rec = nrecs = 0;
                      garmin_cmplt(TRACK_PROTOCOL);
                    }
                    break;
                  case 301:
                  case 302:
                    t0 = 0, nlogs = 0;
                    garmin_trk_hdr((char *)data);
                    break;
                  default:
                    rec = nrecs = 0;
                    garmin_cmplt(TRACK_PROTOCOL);
                }
              } else {
                rec = nrecs = 0;
                garmin_cmplt(TRACK_PROTOCOL);
              }
          }
          break;
        case L001_Wpt_Data:
        case L002_Wpt_Data:
          rec++;
          if (rec < nrecs && GetPoint(rec, &point) > 0)
            garmin_wpt_data(&point, data);
          else
            garmin_cmplt(POINT_PROTOCOL);
          break;
        case L001_Rte_Hdr:
        case L002_Rte_Hdr:
        case L001_Rte_Link_Data:
          if (GetRoutePoint(nrpoints++, &rte, &point))
            garmin_rte_data(&point, data);
          else {
            rec++;
            if (rec == nrecs)
              garmin_cmplt(ROUTE_PROTOCOL);
            else {
              nrpoints = 0;
              GetRoute(rec, &rte);
              IncCounter();
              garmin_rte_hdr(&rte, data);
            }
          }
          break;
        case L001_Rte_Wpt_Data:
        case L002_Rte_Wpt_Data:
          if (app_protocol[ROUTE_PROTOCOL] == 201) {
            IncCounter();

            // just to check if there is one more points,
            // because Link is sent only between points

            if (GetRoutePoint(nrpoints, &rte, &point))
              garmin_link_data(&rte, data);
            else {
              rec++;
              if (rec == nrecs)
                garmin_cmplt(ROUTE_PROTOCOL);
              else {
                nrpoints = 0;
                GetRoute(rec, &rte);
                IncCounter();
                garmin_rte_hdr(&rte, data);
              }
            }
          } else {
            if (GetRoutePoint(nrpoints++, &rte, &point))
              garmin_rte_data(&point, data);
            else {
              rec++;
              if (rec == nrecs)
                garmin_cmplt(ROUTE_PROTOCOL);
              else {
                nrpoints = 0;
                GetRoute(rec, &rte);
                IncCounter();
                garmin_rte_hdr(&rte, data);
              }
            }
          }
          break;
        case L001_Trk_Hdr:
        case L001_Trk_Data:
          if (GetTrackLog(&log)) {
            garmin_trk_data(&log, data, t0);
            t0 = log.t;
            nlogs++;
          } else {
            for (rec++; rec < nrecs; rec++)
              if (GetTrack(rec, (char *)data) > 0)
                break;
            t0 = 0;
            nlogs = 0;
            if (rec < nrecs) {
              switch (app_protocol[TRACK_PROTOCOL]) {
                case 300:
                  if (GetTrackLog(&log))
                    garmin_trk_data(&log, data, t0);
                  else {
                    rec = nrecs = 0;
                    garmin_cmplt(TRACK_PROTOCOL);
                  }
                  break;
                case 301:
                case 302:
                  garmin_trk_hdr((char *)data);
              }
            } else {
              rec = nrecs = 0;
              garmin_cmplt(TRACK_PROTOCOL);
            }
          }
          break;
        case L001_Position_Data:
        case L002_Position_Data:
          break;
        case L001_Xfer_Cmplt:
           //L002_Xfer_Cmplt:
        case Cmnd_Abort_Transfer:
          rec = nrecs = 0;
          EndTransfer();
      }
      break;
    case L000_Unit_ID:
      garmin_ack(L000_Unit_ID);
      break;
    case L000_36:
      garmin_ack(L000_36);
      break;
    case L000_48:
      garmin_ack(L000_48);
      i = pack_word(9600, data, 0);
      garmin_send(L000_49, data, i);
      break;
    case L000_45:
      garmin_ack(L000_45);
      i = pack_word(7, data, 0);
      garmin_send(L000_74, data, i);
      break;
    case L000_75:
      garmin_ack(L000_75);
      i = pack_word(0, data, 0);
      garmin_send(L000_74, data, i);
      break;
    case L000_252:
      garmin_ack(L000_252);
      break;
    case L000_Product_Rqst:
      if (protocol == PROTOCOL_GARMIN_HOST) {
        garmin_ack(L000_Product_Rqst);
        i = pack_word(GARMIN_PRODUCT_ID, data, 0);
        i = pack_word(GARMIN_SOFT_VERSION, data, i);
        i = pack_string(STR_HOST, data, i);
        garmin_send(L000_Product_Data, data, i);
        garmin_protocol_array(data);
      }
      break;
    case L000_Product_Data:
      garmin_ack(L000_Product_Data);
      w = unpack_word(&buf[2]);
      garmin_assign_protocols(unpack_word(&buf[0]), w);
      k = 0;
      if (w > 9999)
        w = w % 10000;
      if (w > 999) {
        data[k++] = '0' + w / 1000;
        w = w % 1000;
      }
      data[k++] = '0' + w / 100;
      w = w % 100;
      data[k++] = '.';
      data[k++] = '0' + w / 10;
      data[k++] = '0' + w % 10;
      data[k++] = 0;
      SetDevice(STR_GARMIN, (char *)&buf[4], (char *)data);
      break;
    case L000_Protocol_Array:
      garmin_ack(L000_Protocol_Array);
      m = n/3;
      for (k = 0; k < m; k++) {
        w = unpack_word(&buf[k*3+1]);
        switch (buf[k*3]) {
          case 'L':
            link_protocol = w;
            break;
          case 'A':
            p = 0;
            d = 0;
            if (w >= 10 && w <= 19)
              command_protocol = w;
            else if (w >= 100 && w <= 199)
              app_protocol[p = POINT_PROTOCOL] = w;
            else if (w >= 200 && w <= 299)
              app_protocol[p = ROUTE_PROTOCOL] = w;
            else if (w >= 300 && w <= 399)
              app_protocol[p = TRACK_PROTOCOL] = w;
            else if (w >= 800 && w <= 899)
              app_protocol[p = PVT_PROTOCOL] = w;
            break;
          case 'D':
            if (d < 3) switch (p) {
              case POINT_PROTOCOL:
              case ROUTE_PROTOCOL:
              case TRACK_PROTOCOL:
              case PVT_PROTOCOL:
                data_protocol[p][d++] = w;
            }
        }
      }
      SetHandshake(1);
      SetProtocol(LINK_PROTOCOL, link_protocol, 0);
      SetProtocol(POINT_PROTOCOL,
        app_protocol[POINT_PROTOCOL], data_protocol[POINT_PROTOCOL]);
      SetProtocol(ROUTE_PROTOCOL,
        app_protocol[ROUTE_PROTOCOL], data_protocol[ROUTE_PROTOCOL]);
      SetProtocol(TRACK_PROTOCOL,
        app_protocol[TRACK_PROTOCOL], data_protocol[TRACK_PROTOCOL]);

      //if (command_protocol == 10 && app_protocol[PVT_PROTOCOL] == 800)
        //garmin_start_pvt();
      break;
    case L001_Records:
      garmin_ack(L001_Records);
      w = unpack_word(buf);
      BeginImport(w);
      break;
    case L002_Records_1000:
      garmin_ack(L002_Records);
      w = unpack_word(buf);
      BeginImport(w);
      break;
    case L001_Pvt_Data:
      garmin_ack(id);
      garmin_d80x_pvt(app_protocol[PVT_PROTOCOL], buf, n);
      break;
    case L001_Wpt_Data:
    case L002_Wpt_Data:
      garmin_ack(id);
      if (!garmin_d10x_point(data_protocol[POINT_PROTOCOL][0], &point, buf, n))
        AddPoint(&point);
      break;
    case L001_Rte_Hdr:
    case L002_Rte_Hdr:
      garmin_ack(id);
      if (!garmin_d20x_rte(data_protocol[ROUTE_PROTOCOL][0], &rte, buf, n))
        AddRoute(&rte);
      break;
    case L001_Rte_Wpt_Data:
    case L002_Rte_Wpt_Data:
      garmin_ack(id);
      if (!garmin_d10x_point(data_protocol[ROUTE_PROTOCOL][1], &point, buf, n))
        AddRoutePoint(&rte, &point);
      break;
    case L001_Rte_Link_Data:
      garmin_ack(L001_Rte_Link_Data);
      IncCounter();
      break;
    case L001_Trk_Hdr:
      garmin_ack(L001_Trk_Hdr);
      if (data_protocol[TRACK_PROTOCOL][0] == 310) {
        gps_memset((char *)&D10X, 0, sizeof(D10X));
        gps_memcpy((char *)&D10X, (char *)buf, n);

        i = gps_strncpy((char *)data, &D10X.D310.trk_ident[0], TAM_TRACKNAME-1, 0, 0);
        data[i] = 0;

        current_track = !gps_strcmp((char *)data, GARMIN_ACTIVE_LOG);
        if (current_track)
          SetCurrentTrack();
        else
          AddTrack((char *)data);
      }
      break;
    case L001_Trk_Data:
      garmin_ack(L001_Trk_Data);
      switch (app_protocol[TRACK_PROTOCOL]) {
        case 300:
          i = garmin_d30x_log(data_protocol[TRACK_PROTOCOL][0], &log, buf, n);
          break;
        case 301:
        case 302:
          i = garmin_d30x_log(data_protocol[TRACK_PROTOCOL][1], &log, buf, n);
          break;
        default:
          i = -1;
      }

      if (i == 0) {
        if (current_track == -1) {
          current_track = 1;
          SetCurrentTrack();
        }
        if (current_track)
          AddCurrentTrackLog(&log);
        else
          AddTrackLog(&log);
      }
      break;
    case L001_Xfer_Cmplt:
       //L002_Xfer_Cmplt:
      garmin_ack(L001_Xfer_Cmplt);
      EndTransfer();
      break;
    case L001_Command_Data:
      debug_command(PACKET_RCV, unpack_word(buf));

      if (protocol == PROTOCOL_GARMIN_HOST) {
        garmin_ack(L001_Command_Data);
        switch (unpack_word(buf)) {
          case Cmnd_Transfer_Wpt:
            garmin_send_wpts();
            break;
          case Cmnd_Transfer_Rte:
            garmin_send_rtes();
            break;
          case Cmnd_Transfer_Trk:
            garmin_send_trks();
            break;
          case Cmnd_Abort_Transfer:
            rec = nrecs = 0;
            EndTransfer();
            break;
          case Cmnd_Transfer_UID:
            pack_dword(GARMIN_UNIT_ID, data, 0);
            garmin_send(L000_Unit_ID, data, 4);
            break;
          case Cmnd_57:
            garmin_send_unknown(Cmnd_57);
            break;
          case Cmnd_58:
            break;
          case Cmnd_63:
            garmin_send_unknown(Cmnd_63);
        }
      }
      break;
    case L002_Command_Data:
      if (protocol == PROTOCOL_GARMIN_HOST) {
        garmin_ack(L002_Command_Data);
        switch (unpack_word(buf)) {
          case Cmnd2_Transfer_Wpt:
            garmin_send_wpts();
            break;
          case Cmnd2_Transfer_Rte:
            garmin_send_rtes();
            break;
          case Cmnd2_Abort_Transfer:
            rec = nrecs = 0;
            EndTransfer();
        }
      }
      break;
    case L001_Date_Time_Data:
    case L002_Date_Time_Data:
      garmin_ack(id);
      break;
    case L001_Position_Data:
    case L002_Position_Data:
      garmin_ack(id);
      break;
    case L001_Disp_Data:
      garmin_ack(L001_Disp_Data);
      if (n == 40 && buf[0] == 0) {
        bpp = unpack_word(&buf[0x0c]);
        width = unpack_word(&buf[0x10]);
        height = unpack_word(&buf[0x14]);
        StartDisplay(width, height, bpp);
      } else if (n > 8 && buf[0] == 1)
        SendDisplayRow(&buf[8], n-8);
      break;
    default:
      garmin_ack(id);
  }

  return 0;
}

static int garmin_protocol_array(uint8_t *data)
{
  int i;

  i = 0;
  data[i++] = 'P';
  i = pack_word(0, data, i);
              
  data[i++] = 'L';
  i = pack_word(link_protocol, data, i);
                          
  data[i++] = 'A';
  i = pack_word(command_protocol, data, i);
                  
  data[i++] = 'A';
  i = pack_word(app_protocol[POINT_PROTOCOL], data, i);
  data[i++] = 'D';
  i = pack_word(data_protocol[POINT_PROTOCOL][0], data, i);
                
  data[i++] = 'A';
  i = pack_word(app_protocol[ROUTE_PROTOCOL], data, i);
  data[i++] = 'D';
  i = pack_word(data_protocol[ROUTE_PROTOCOL][0], data, i);
  data[i++] = 'D';
  i = pack_word(data_protocol[ROUTE_PROTOCOL][1], data, i);

  data[i++] = 'A';
  i = pack_word(app_protocol[TRACK_PROTOCOL], data, i);
  data[i++] = 'D';
  i = pack_word(data_protocol[TRACK_PROTOCOL][0], data, i);
  data[i++] = 'D';
  i = pack_word(data_protocol[TRACK_PROTOCOL][1], data, i);
                    
  data[i++] = 'A';
  i = pack_word(600, data, i);
  data[i++] = 'D';
  i = pack_word(600, data, i);

  data[i++] = 'A';
  i = pack_word(700, data, i);
  data[i++] = 'D';
  i = pack_word(700, data, i);

  return garmin_send(L000_Protocol_Array, data, i);
}

static int garmin_wpt_data(WaypointType *point, uint8_t *data)
{
  int i;
  i = garmin_point_d10x(data_protocol[POINT_PROTOCOL][0], point, data);
  switch (link_protocol) {
    case 1: return garmin_send(L001_Wpt_Data, data, i > 0 ? i : 0);
    case 2: return garmin_send(L002_Wpt_Data, data, i > 0 ? i : 0);
  }
  return -1;
}

static int garmin_rte_hdr(RouteType *rte, uint8_t *data)
{
  int i;
  i = garmin_rte_d20x(data_protocol[ROUTE_PROTOCOL][0], rte, data);
  switch (link_protocol) {
    case 1: return garmin_send(L001_Rte_Hdr, data, i > 0 ? i : 0);
    case 2: return garmin_send(L002_Rte_Hdr, data, i > 0 ? i : 0);
  }
  return -1;
}

static int garmin_rte_data(WaypointType *point, uint8_t *data)
{
  int i;
  i = garmin_point_d10x(data_protocol[ROUTE_PROTOCOL][1], point, data);
  switch (link_protocol) {
    case 1: return garmin_send(L001_Rte_Wpt_Data, data, i > 0 ? i : 0);
    case 2: return garmin_send(L002_Rte_Wpt_Data, data, i > 0 ? i : 0);
  }
  return -1;
}

static int garmin_link_data(RouteType *rte, uint8_t *data)
{
  int i;
  i = garmin_rte_d20x(data_protocol[ROUTE_PROTOCOL][2], rte, data);
  return garmin_send(L001_Rte_Link_Data, data, i);
}

static int garmin_trk_hdr(char *name)
{
  int i;
  gps_memset((char *)&D10X, 0, sizeof(D10X));
  D10X.D310.dspl = 0;
  D10X.D310.color = 0xff;
  i = gps_strncpy(&D10X.D310.trk_ident[0], name, TAM_TRACKNAME, 0, 1);
  D10X.D310.trk_ident[i] = 0;
  return garmin_send(L001_Trk_Hdr, (uint8_t *)&D10X.D310, i+3);
}

static int garmin_trk_data(TracklogType *log, uint8_t *data, uint32_t t0)
{
  int i;
  switch (app_protocol[TRACK_PROTOCOL]) {
    case 300:
      i = garmin_log_d30x(data_protocol[TRACK_PROTOCOL][0], log,
                          (log->t - t0) > 900 ? 1 : 0, data);
      break;
    case 301:
    case 302:
      i = garmin_log_d30x(data_protocol[TRACK_PROTOCOL][1], log,
                          (log->t - t0) > 900 ? 1 : 0, data);
      break;
    default:
      return -1;
  }
  return garmin_send(L001_Trk_Data, data, i);
}

/*
// not used for now
static int garmin_pos_data(double lat, double lon, uint8_t *data)
{
  int i;
  i = garmin_pos_d700(lat, lon, data);
  switch (link_protocol) {
    case 1: return garmin_send(L001_Position_Data, data, i);
    case 2: return garmin_send(L002_Position_Data, data, i);
  }
  return -1;
}
*/

static int garmin_cmplt(int p)
{
  uint8_t buf[2];
  uint16_t cmd;

  switch (link_protocol) {
    case 1:
      switch (p) {
        case POINT_PROTOCOL: cmd = Cmnd_Transfer_Wpt; break;
        case ROUTE_PROTOCOL: cmd = Cmnd_Transfer_Rte; break;
        case TRACK_PROTOCOL: cmd = Cmnd_Transfer_Trk; break;
        default: return -1;
      }
      pack_word(cmd, buf, 0);
      return garmin_send(L001_Xfer_Cmplt, buf, 2);
      break;
    case 2:
      switch (p) {
        case POINT_PROTOCOL: cmd = Cmnd2_Transfer_Wpt; break;
        case ROUTE_PROTOCOL: cmd = Cmnd2_Transfer_Rte; break;
        default: return -1;
      }
      pack_word(cmd, buf, 0);
      return garmin_send(L002_Xfer_Cmplt, buf, 2);
  }

  return -1;
}

static int garmin_ack(uint8_t id)
{
  uint8_t buf[2];
  debug_ack(PACKET_SND, 1, id);
  buf[0] = id;
  buf[1] = 0;
  return garmin_send(L000_Ack_Byte, buf, 2);
}

static int garmin_nack(uint8_t id)
{
  uint8_t buf[2];
  debug_ack(PACKET_SND, 0, id);
  buf[0] = id;
  buf[1] = 0;
  return garmin_send(L000_Nak_Byte, buf, 2);
}

static int garmin_send_unknown(uint16_t cmd)
{
  static uint8_t rsp_57[] = {0x04, 0xbc, 0x01, 0x00, 0x02, 0xde, 0x00, 0x00,
                           0x01, 0x94, 0x00, 0x00, 0x01, 0x6f, 0x00, 0x00,
                           0x00, 0x4a, 0x00, 0x00, 0x80, 0x37, 0x00, 0x00,
                           0x00, 0x25, 0x00, 0x00};
  static uint8_t rsp_63[] = {0x0a, 0x00, 0x58, 0x02, 0x00, 0x00, 0x10, 0x00,
                           0x00, 0x01, 0x00, 0x00};

  switch (cmd) {
    case Cmnd_57:
      return garmin_send(L000_252, rsp_57, 28);
    case Cmnd_63:
      return garmin_send(L000_95, rsp_63, 12);
  }

  return 0;
}

static int garmin_point_d10x(int data_protocol, WaypointType *point,
                             uint8_t *data)
{
  int size, i;

  gps_memset((char *)&D10X, 0, sizeof(D10X));
  size = 0;

  switch (data_protocol) {
    case 100:
      gps_strncpy(D10X.D100.ident, point->name, 6, 1, 2);
      pack_dword(garmin_to_coord(point->coord.latitude),
                 (uint8_t *)&D10X.D100.posn.lat, 0);
      pack_dword(garmin_to_coord(point->coord.longitude),
                 (uint8_t *)&D10X.D100.posn.lon, 0);
      gps_strncpy(D10X.D100.cmnt, point->comment, 40, 1, 1);
      size = sizeof(D100_Wpt_Type);
      break;
    case 101:
      gps_strncpy(D10X.D101.ident, point->name, 6, 1, 2);
      pack_dword(garmin_to_coord(point->coord.latitude),
                 (uint8_t *)&D10X.D101.posn.lat, 0);
      pack_dword(garmin_to_coord(point->coord.longitude),
                 (uint8_t *)&D10X.D101.posn.lon, 0);
      gps_strncpy(D10X.D101.cmnt, point->comment, 40, 1, 1);
      D10X.D101.dst = invert_float(0.0);
      D10X.D101.smbl = (uint8_t)point->symbol;
      size = sizeof(D101_Wpt_Type);
      break;
    case 102:
      gps_strncpy(D10X.D102.ident, point->name, 6, 1, 2);
      pack_dword(garmin_to_coord(point->coord.latitude),
                 (uint8_t *)&D10X.D102.posn.lat, 0);
      pack_dword(garmin_to_coord(point->coord.longitude),
                 (uint8_t *)&D10X.D102.posn.lon, 0);
      gps_strncpy(D10X.D102.cmnt, point->comment, 40, 1, 1);
      D10X.D102.dst = invert_float(0.0);
      D10X.D102.smbl = garmin_map_symbol(data_protocol, point->symbol);
      size = sizeof(D102_Wpt_Type);
      break;
    case 103:
      gps_strncpy(D10X.D103.ident, point->name, 6, 1, 2);
      pack_dword(garmin_to_coord(point->coord.latitude),
                 (uint8_t *)&D10X.D103.posn.lat, 0);
      pack_dword(garmin_to_coord(point->coord.longitude),
                 (uint8_t *)&D10X.D103.posn.lon, 0);
      gps_strncpy(D10X.D103.cmnt, point->comment, 40, 1, 1);
      D10X.D103.smbl = (uint8_t)garmin_map_symbol(data_protocol, point->symbol);
      D10X.D103.dspl = 0;
      size = sizeof(D103_Wpt_Type);
      break;
    case 104:
      gps_strncpy(D10X.D104.ident, point->name, 6, 1, 2);
      pack_dword(garmin_to_coord(point->coord.latitude),
                 (uint8_t *)&D10X.D104.posn.lat, 0);
      pack_dword(garmin_to_coord(point->coord.longitude),
                 (uint8_t *)&D10X.D104.posn.lon, 0);
      gps_strncpy(D10X.D104.cmnt, point->comment, 40, 1, 1);
      D10X.D104.dst = invert_float(0.0);
      D10X.D104.smbl = garmin_map_symbol(data_protocol, point->symbol);
      D10X.D104.dspl = 0;
      size = sizeof(D104_Wpt_Type);
      break;
    case 105:
      pack_dword(garmin_to_coord(point->coord.latitude),
                 (uint8_t *)&D10X.D105.posn.lat, 0);
      pack_dword(garmin_to_coord(point->coord.longitude),
                 (uint8_t *)&D10X.D105.posn.lon, 0);
      D10X.D105.smbl = garmin_map_symbol(data_protocol, point->symbol);
      i = gps_strncpy(&D10X.D105.s[0], point->name, TAM_NAME, 0, 2);
      size = sizeof(D105_Wpt_Type) + i + 1;
      break;
    case 106:
      D10X.D106.wpt_class = 0;
      for (i = 0; i < 13; i++)
        D10X.D106.subclass[i] = 0;
      pack_dword(garmin_to_coord(point->coord.latitude),
                 (uint8_t *)&D10X.D106.posn.lat, 0);
      pack_dword(garmin_to_coord(point->coord.longitude),
                 (uint8_t *)&D10X.D106.posn.lon, 0);
      D10X.D106.smbl = garmin_map_symbol(data_protocol, point->symbol);
      i = gps_strncpy(D10X.D106.s, point->name, TAM_NAME, 0, 2);
      size = sizeof(D106_Wpt_Type) + i + 2; // lnk_ident
      break;
    case 107:
      gps_strncpy(D10X.D107.ident, point->name, 6, 1, 2);
      pack_dword(garmin_to_coord(point->coord.latitude),
                 (uint8_t *)&D10X.D107.posn.lat, 0);
      pack_dword(garmin_to_coord(point->coord.longitude),
                 (uint8_t *)&D10X.D107.posn.lon, 0);
      D10X.D107.unused = 0;
      gps_strncpy(D10X.D107.cmnt, point->comment, 40, 1, 1);
      D10X.D107.smbl = (uint8_t)garmin_map_symbol(data_protocol, point->symbol);
      D10X.D107.dspl = 0;
      D10X.D107.dst = invert_float(0.0);
      D10X.D107.color = 0; // default color
      size = sizeof(D107_Wpt_Type);
      break;
    case 108:
      D10X.D108.wpt_class = 0; // user waypoint
      D10X.D108.color = 0xff; // default color
      D10X.D108.dspl = 0; // display symbol with waypoint name
      D10X.D108.attr = 0x60; // ???
      D10X.D108.smbl = garmin_map_symbol(data_protocol, point->symbol);
      for (i = 0; i < 6; i++)
        D10X.D108.subclass[i] = 0x00;
      for (; i < 18; i++)
        D10X.D108.subclass[i] = 0xff;
      pack_dword(garmin_to_coord(point->coord.latitude),
                 (uint8_t *)&D10X.D108.posn.lat, 0);
      pack_dword(garmin_to_coord(point->coord.longitude),
                 (uint8_t *)&D10X.D108.posn.lon, 0);
      D10X.D108.alt = invert_float(point->coord.height);
      D10X.D108.dpth = invert_float(0.0);
      D10X.D108.dist = invert_float(0.0);
      D10X.D108.state[0] = ' ';
      D10X.D108.state[1] = ' ';
      D10X.D108.cc[0] = ' ';
      D10X.D108.cc[1] = ' ';
      i = gps_strncpy(&D10X.D108.s[0], point->name, TAM_NAME, 0, 1);
      i += gps_strncpy(&D10X.D108.s[i+1], point->comment, TAM_COMMENT, 0, 1);
      size = sizeof(D108_Wpt_Type) + i + 6;
      break;
    case 109:
      D10X.D109.dtyp = 1;  // packet type
      D10X.D109.wpt_class = 0;  // user waypoint
      D10X.D109.dspl_color = 0x1f;   // bit field
      D10X.D109.attr = 0x70;    // ???
      D10X.D109.smbl = garmin_map_symbol(data_protocol, point->symbol);
      for (i = 0; i < 6; i++)
        D10X.D109.subclass[i] = 0x00;
      for (; i < 18; i++)
        D10X.D109.subclass[i] = 0xff;
      pack_dword(garmin_to_coord(point->coord.latitude),
                 (uint8_t *)&D10X.D109.posn.lat, 0);
      pack_dword(garmin_to_coord(point->coord.longitude),
                 (uint8_t *)&D10X.D109.posn.lon, 0);
      D10X.D109.alt = invert_float(point->coord.height);
      D10X.D109.dpth = invert_float(0.0);
      D10X.D109.dist = invert_float(0.0);
      D10X.D109.state[0] = ' ';
      D10X.D109.state[1] = ' ';
      D10X.D109.cc[0] = ' ';
      D10X.D109.cc[1] = ' ';
      D10X.D109.ete = 0xffffffff;
      i = gps_strncpy(&D10X.D109.s[0], point->name, TAM_NAME, 0, 1);
      i += gps_strncpy(&D10X.D109.s[i+1], point->comment, TAM_COMMENT, 0, 1);
      size = sizeof(D109_Wpt_Type) + i + 6;
      break;
    case 150:
      gps_strncpy(D10X.D150.ident, point->name, 6, 1, 2);
      D10X.D150.cc[0] = ' ';
      D10X.D150.cc[1] = ' ';
      D10X.D150.wpt_class = 4; // usr_wpt_class
      pack_dword(garmin_to_coord(point->coord.latitude),
                 (uint8_t *)&D10X.D150.posn.lat, 0);
      pack_dword(garmin_to_coord(point->coord.longitude),
                 (uint8_t *)&D10X.D150.posn.lon, 0);
      pack_word((uint16_t)(int)(point->coord.height), (uint8_t *)&D10X.D150.alt, 0);
      gps_memset((char *)&D10X.D150.city, ' ', 24);
      D10X.D150.state[0] = ' ';
      D10X.D150.state[1] = ' ';
      gps_memset((char *)&D10X.D150.name, ' ', 30);
      gps_strncpy(D10X.D150.cmnt, point->comment, 40, 1, 1);
      size = sizeof(D150_Wpt_Type);
      break;
    case 151:
    case 152:
    case 154:
    case 155:
      gps_strncpy(D10X.D151.ident, point->name, 6, 1, 2);
      pack_dword(garmin_to_coord(point->coord.latitude),
                 (uint8_t *)&D10X.D151.posn.lat, 0);
      pack_dword(garmin_to_coord(point->coord.longitude),
                 (uint8_t *)&D10X.D151.posn.lon, 0);
      D10X.D151.unused = 0;
      gps_strncpy(D10X.D151.cmnt, point->comment, 40, 1, 1);
      D10X.D151.dst = invert_float(0.0);
      gps_memset((char *)&D10X.D151.name, ' ', 30);
      gps_memset((char *)&D10X.D151.city, ' ', 24);
      D10X.D151.state[0] = ' ';
      D10X.D151.state[1] = ' ';
      pack_word((uint16_t)(int)(point->coord.height), (uint8_t *)&D10X.D151.alt, 0);
      D10X.D151.cc[0] = ' ';
      D10X.D151.cc[1] = ' ';
      D10X.D151.unused2 = 0;
      D10X.D151.wpt_class = (data_protocol == 151) ? 2 : 4;  // usr_wpt_class
      if (data_protocol == 154) {
        D10X.D154.smbl = garmin_map_symbol(data_protocol, point->symbol);
        size = sizeof(D154_Wpt_Type);
      } else if (data_protocol == 155) {
        D10X.D155.smbl = garmin_map_symbol(data_protocol, point->symbol);
        D10X.D155.dspl = 3; // symbol + name
        size = sizeof(D155_Wpt_Type);
      } else
        size = sizeof(D151_Wpt_Type);
      break;
    default:
      return -1;
  }

  gps_memcpy((char *)data, (char *)&D10X, size);
  return size;
}

static int garmin_d80x_pvt(int data_protocol, uint8_t *data, int n)
{
  int i;

  gps_memset((char *)&D10X, 0, sizeof(D10X));
  gps_memcpy((char *)&D10X, (char *)data, n);

  switch (data_protocol) {
    case 800:
      SetHeight(invert_float(D10X.D800.alt) + invert_float(D10X.D800.msl_hght));
      SetEPE(invert_float(D10X.D800.eph), invert_float(D10X.D800.epv));
      i = unpack_word((uint8_t *)&D10X.D800.fix);
      switch (i) {
        case 0:
        case 1:
          SetVisible(0, 0);
          SetValidity(0, 0);
          break;
        case 2:
        case 4:
          SetVisible(3, 0);
          SetValidity(1, 3);    // 2D
          break;
        case 3:
        case 5:
          SetVisible(4, 0);
          SetValidity(1, 4);    // 3D
      }
      SetPosition((invert_double(D10X.D800.posn.lat) / PI) * 180.0,
                  (invert_double(D10X.D800.posn.lon) / PI) * 180.0);
      SetXYSpeed(invert_float(D10X.D800.east), invert_float(D10X.D800.north));
      SetClimb(invert_float(D10X.D800.up));
      SaveData();
      break;
    default:
      return -1;
  }

  return 0;
}

static int garmin_d10x_point(int data_protocol, WaypointType *point,
                             uint8_t *data, int n)
{
  int i;

  gps_memset((char *)point, 0, sizeof(WaypointType));
  gps_memset((char *)&D10X, 0, sizeof(D10X));
  gps_memcpy((char *)&D10X, (char *)data, n);

  switch (data_protocol) {
    case 100:
      point->symbol = sym_wpt_dot;
      point->coord.latitude =
         garmin_from_coord(unpack_dword((uint8_t *)&D10X.D100.posn.lat));
      point->coord.longitude =
         garmin_from_coord(unpack_dword((uint8_t *)&D10X.D100.posn.lon));
      point->coord.height = 0.0;
      i = gps_strncpy(point->name, D10X.D100.ident, 6, 0, 0);
      point->name[i] = 0;
      i = gps_strncpy(point->comment, D10X.D100.cmnt, 40, 0, 0);
      point->comment[i] = 0;
      break;
    case 101:
      point->symbol = D10X.D101.smbl;
      point->coord.latitude =
         garmin_from_coord(unpack_dword((uint8_t *)&D10X.D101.posn.lat));
      point->coord.longitude =
         garmin_from_coord(unpack_dword((uint8_t *)&D10X.D101.posn.lon));
      point->coord.height = 0.0;
      i = gps_strncpy(point->name, D10X.D101.ident, 6, 0, 0);
      point->name[i] = 0;
      i = gps_strncpy(point->comment, D10X.D101.cmnt, 40, 0, 0);
      point->comment[i] = 0;
      break;
    case 102:
      point->symbol = garmin_unmap_symbol(data_protocol, D10X.D102.smbl);
      point->coord.latitude =
         garmin_from_coord(unpack_dword((uint8_t *)&D10X.D102.posn.lat));
      point->coord.longitude =
         garmin_from_coord(unpack_dword((uint8_t *)&D10X.D102.posn.lon));
      point->coord.height = 0.0;
      i = gps_strncpy(point->name, D10X.D102.ident, 6, 0, 0);
      point->name[i] = 0;
      i = gps_strncpy(point->comment, D10X.D102.cmnt, 40, 0, 0);
      point->comment[i] = 0;
      break;
    case 103:
      point->symbol = garmin_unmap_symbol(data_protocol, D10X.D103.smbl);
      point->coord.latitude =
         garmin_from_coord(unpack_dword((uint8_t *)&D10X.D103.posn.lat));
      point->coord.longitude =
         garmin_from_coord(unpack_dword((uint8_t *)&D10X.D103.posn.lon));
      point->coord.height = 0.0;
      i = gps_strncpy(point->name, D10X.D103.ident, 6, 0, 0);
      point->name[i] = 0;
      i = gps_strncpy(point->comment, D10X.D103.cmnt, 40, 0, 0);
      point->comment[i] = 0;
      break;
    case 104:
      point->symbol = garmin_unmap_symbol(data_protocol, D10X.D104.smbl);
      point->coord.latitude =
         garmin_from_coord(unpack_dword((uint8_t *)&D10X.D104.posn.lat));
      point->coord.longitude =
         garmin_from_coord(unpack_dword((uint8_t *)&D10X.D104.posn.lon));
      point->coord.height = 0.0;
      i = gps_strncpy(point->name, D10X.D104.ident, 6, 0, 0);
      point->name[i] = 0;
      i = gps_strncpy(point->comment, D10X.D104.cmnt, 40, 0, 0);
      point->comment[i] = 0;
      break;
    case 105:
      point->symbol = garmin_unmap_symbol(data_protocol, D10X.D105.smbl);
      point->coord.latitude =
         garmin_from_coord(unpack_dword((uint8_t *)&D10X.D105.posn.lat));
      point->coord.longitude =
         garmin_from_coord(unpack_dword((uint8_t *)&D10X.D105.posn.lon));
      point->coord.height = 0.0;
      i = gps_strncpy(point->name, &D10X.D105.s[0], TAM_NAME-1, 0, 0);
      point->name[i] = 0;
      point->comment[0] = 0;
      break;
    case 106:
      point->symbol = garmin_unmap_symbol(data_protocol, D10X.D106.smbl);
      point->coord.latitude =
         garmin_from_coord(unpack_dword((uint8_t *)&D10X.D106.posn.lat));
      point->coord.longitude =
         garmin_from_coord(unpack_dword((uint8_t *)&D10X.D106.posn.lon));
      point->coord.height = 0.0;
      i = gps_strncpy(point->name, &D10X.D106.s[0], TAM_NAME-1, 0, 0);
      point->name[i] = 0;
      point->comment[0] = 0;
      break;
    case 107:
      point->symbol = garmin_unmap_symbol(data_protocol, D10X.D107.smbl);
      point->coord.latitude =
         garmin_from_coord(unpack_dword((uint8_t *)&D10X.D107.posn.lat));
      point->coord.longitude =
         garmin_from_coord(unpack_dword((uint8_t *)&D10X.D107.posn.lon));
      point->coord.height = 0.0;
      i = gps_strncpy(point->name, D10X.D107.ident, TAM_NAME-1, 0, 0);
      point->name[i] = 0;
      i = gps_strncpy(point->comment, D10X.D107.cmnt, TAM_COMMENT-1, 0, 0);
      point->comment[i] = 0;
      break;
    case 108:
      point->symbol = garmin_unmap_symbol(data_protocol, D10X.D108.smbl);
      point->coord.latitude =
         garmin_from_coord(unpack_dword((uint8_t *)&D10X.D108.posn.lat));
      point->coord.longitude =
         garmin_from_coord(unpack_dword((uint8_t *)&D10X.D108.posn.lon));
      point->coord.height = invert_float(D10X.D108.alt);
      if (point->coord.height == 1.0e25)
        point->coord.height = 0;
      i = gps_strncpy(point->name, &D10X.D108.s[0], TAM_NAME-1, 0, 0);
      point->name[i] = 0;
      i = gps_strncpy(point->comment, &D10X.D108.s[i+1], TAM_COMMENT-1, 0, 0);
      point->comment[i] = 0;
      break;
    case 109:
      point->symbol = garmin_unmap_symbol(data_protocol, D10X.D109.smbl);
      point->coord.latitude =
         garmin_from_coord(unpack_dword((uint8_t *)&D10X.D109.posn.lat));
      point->coord.longitude =
         garmin_from_coord(unpack_dword((uint8_t *)&D10X.D109.posn.lon));
      point->coord.height = invert_float(D10X.D109.alt);
      if (point->coord.height == 1.0e25)
        point->coord.height = 0;
      i = gps_strncpy(point->name, &D10X.D109.s[0], TAM_NAME-1, 0, 0);
      point->name[i] = 0;
      i = gps_strncpy(point->comment, &D10X.D109.s[i+1], TAM_COMMENT-1, 0, 0);
      point->comment[i] = 0;
      break;
    default:
      return -1;
  }

  GetUTC(&point->date.day, &point->date.month, &point->date.year,
         &point->date.hour, &point->date.minute, &point->date.second);
  point->datum = datum;

  return 0;
}

static int garmin_rte_d20x(int data_protocol, RouteType *rte, uint8_t *data)
{
  int size, i;

  gps_memset((char *)&D20X, 0, sizeof(D20X));
  size = 0;

  switch (data_protocol) {
    case 200:
      D20X.D200.nmbr = rte->id;
      size = sizeof(D200_Rte_Hdr_Type);
      break;
    case 201:
      D20X.D201.nmbr = rte->id;
      gps_strncpy(D20X.D201.cmnt, rte->ident, 20, 1, 1);
      size = sizeof(D201_Rte_Hdr_Type);
      break;
    case 202:
      i = gps_strncpy(&D20X.D202.rte_ident[0], rte->ident, 40, 0, 1);
      size = i + 1;
      break;
    case 210:
      size = sizeof(D210_Rte_Link_Type) + 1;
      break;
    default:
      return -1;
  }

  gps_memcpy((char *)data, (char *)&D20X, size);
  return size;
}

static int garmin_d20x_rte(int data_protocol, RouteType *rte,
                           uint8_t *data, int n)
{
  int i;
      
  gps_memset((char *)rte, 0, sizeof(RouteType));
  gps_memset((char *)&D20X, 0, sizeof(D20X));
  gps_memcpy((char *)&D20X, (char *)data, n);
      
  switch (data_protocol) {
    case 200:
      rte->ident[0] = 0;
      rte->id = D20X.D200.nmbr;
      break;
    case 201:
      i = gps_strncpy(rte->ident, D20X.D201.cmnt, 20, 0, 0);
      rte->ident[i] = 0;
      rte->id = D20X.D201.nmbr;
      break;
    case 202:
      i = gps_strncpy(rte->ident, D20X.D202.rte_ident, 20, 0, 0);
      rte->ident[i] = 0;
      rte->id = 0xff;
      break;
    default:
      return -1;
  }

  return 0;
}

static int garmin_log_d30x(int data_protocol, TracklogType *log,
                           int new_segment, uint8_t *data)
{
  int size;
  
  gps_memset((char *)&D10X, 0, sizeof(D10X));
  size = 0;

  switch (data_protocol) {
    case 300:
      pack_dword(log->t, (uint8_t *)&D10X.D300.time, 0);
      pack_dword(garmin_to_coord(log->latitude),
                 (uint8_t *)&D10X.D300.posn.lat, 0);
      pack_dword(garmin_to_coord(log->longitude),
                 (uint8_t *)&D10X.D300.posn.lon, 0);
      D10X.D300.new_trk = new_segment ? 1 : 0;
      size = sizeof(D300_Trk_Point_Type);
      break;
    case 301:
      pack_dword(log->t, (uint8_t *)&D10X.D301.time, 0);
      pack_dword(garmin_to_coord(log->latitude),
                 (uint8_t *)&D10X.D301.posn.lat, 0);
      pack_dword(garmin_to_coord(log->longitude),
                 (uint8_t *)&D10X.D301.posn.lon, 0);
      D10X.D301.alt = invert_float(log->height);
      D10X.D301.dpth = invert_float(0.0);
      D10X.D301.new_trk = new_segment ? 1 : 0;
      size = sizeof(D301_Trk_Point_Type);
      break;
    case 302:
      pack_dword(log->t, (uint8_t *)&D10X.D302.time, 0);
      pack_dword(garmin_to_coord(log->latitude),
                 (uint8_t *)&D10X.D302.posn.lat, 0);
      pack_dword(garmin_to_coord(log->longitude),
                 (uint8_t *)&D10X.D302.posn.lon, 0);
      D10X.D302.alt = invert_float(log->height);
      D10X.D302.dpth = invert_float(0.0);
      D10X.D302.temp = invert_float(1.0e25);
      D10X.D302.new_trk = new_segment ? 1 : 0;
      size = sizeof(D302_Trk_Point_Type);
      break;
    default:
      return -1;
  }

  gps_memcpy((char *)data, (char *)&D10X, size);
  return size;
}

static int garmin_d30x_log(int data_protocol, TracklogType *log,
                           uint8_t *data, int n)
{
  gps_memset((char *)log, 0, sizeof(TracklogType));
  gps_memset((char *)&D10X, 0, sizeof(D10X));
  gps_memcpy((char *)&D10X, (char *)data, n);

  switch (data_protocol) {
    case 300:
      log->t = unpack_dword((uint8_t *)&D10X.D300.time);
      log->latitude =
        garmin_from_coord(unpack_dword((uint8_t *)&D10X.D300.posn.lat));
      log->longitude =
        garmin_from_coord(unpack_dword((uint8_t *)&D10X.D300.posn.lon));
      log->height = 0.0;
      break;
    case 301:
      log->t = unpack_dword((uint8_t *)&D10X.D301.time);
      log->latitude =
        garmin_from_coord(unpack_dword((uint8_t *)&D10X.D301.posn.lat));
      log->longitude =
        garmin_from_coord(unpack_dword((uint8_t *)&D10X.D301.posn.lon));
      log->height = invert_float(D10X.D301.alt);
      break;
    case 302:
      log->t = unpack_dword((uint8_t *)&D10X.D302.time);
      log->latitude =
        garmin_from_coord(unpack_dword((uint8_t *)&D10X.D302.posn.lat));
      log->longitude =
        garmin_from_coord(unpack_dword((uint8_t *)&D10X.D302.posn.lon));
      log->height = invert_float(D10X.D302.alt);
      break;
    default:
      return -1;
  }
  return 0;
}

/*
static int garmin_pos_d700(double lat, double lon, uint8_t *data)
{
  int size;

  gps_memset((char *)&D700, 0, sizeof(D700));
  size = sizeof(D700_Position_Type);

  D700.lat = invert_double(lat);
  D700.lon = invert_double(lon);
  
  gps_memcpy((char *)data, (char *)&D700, size);
  return size;
}
*/

static const double f_garmin_to_coord = 11930464.71111111111111111; // (2^31 / 180)

int32_t garmin_to_coord(double d)
{
  return (int32_t)(d * f_garmin_to_coord);
}

double garmin_from_coord(int32_t l)
{
  static double f = 2147483648.0;  // 2^31
  return ((double)(l) / f) * 180.0;
}

static uint16_t garmin_map_symbol(int protocol, uint16_t s)
{
  uint16_t r;
  uint8_t *p;

  switch (protocol) {
    case 103:
    case 107:
      switch (s) {
        case sym_house:  r = smbl_house; break;
        case sym_fuel:  r = smbl_gas; break;
        case sym_car:  r = smbl_car; break;
        case sym_fish:  r = smbl_fish; break;
        case sym_anchor: r = smbl_anchor; break;
        case sym_wreck:  r = smbl_wreck; break;
        case sym_skull:  r = smbl_skull; break;
        case sym_flag:  r = smbl_flag; break;
        case sym_camp:  r = smbl_camp; break;
        case sym_deer:  r = smbl_deer; break;
        case sym_1st_aid: r = smbl_1st_aid; break;
        default:  r = smbl_dot;
      }
      break;
    default:
      p = (uint8_t *)&r;
      p[0] = s & 0x00ff;
      p[1] = s >> 8;
  }

  return r;
}

static uint16_t garmin_unmap_symbol(int protocol, uint16_t s)
{
  uint16_t r;
  uint8_t *p;
        
  switch (protocol) {
    case 103:
    case 107:
      switch (s) {
        case smbl_house:        r = sym_house; break;
        case smbl_gas:          r = sym_fuel; break;
        case smbl_car:          r = sym_car; break;
        case smbl_fish:         r = sym_fish; break;
        case smbl_anchor:       r = sym_anchor; break;
        case smbl_wreck:        r = sym_wreck; break;
        case smbl_skull:        r = sym_skull; break;
        case smbl_flag:         r = sym_flag; break;
        case smbl_camp:         r = sym_camp; break;
        case smbl_deer:         r = sym_deer; break;
        case smbl_1st_aid:      r = sym_1st_aid; break;
        default:                r = sym_wpt_dot;
      }
      break;
    default:
      p = (uint8_t *)&r;
      p[0] = s & 0x00ff;
      p[1] = s >> 8;
  }

  return r;
}

static int garmin_send(uint8_t id, uint8_t *data, uint8_t size)
{
  int i, k;
  uint8_t bsum;

  debug_packet(PACKET_SND, id, data, size);

  i = 0;
  sendbuf[i++] = dle;
  sendbuf[i++] = id;
  bsum = id;
  sendbuf[i++] = size;
  bsum += size;
  if (size == dle)
    sendbuf[i++] = dle;

  for (k = 0 ; k < size; k++) {
    sendbuf[i++] = data[k];
    bsum += data[k];
    if (data[k] == dle)
      sendbuf[i++] = dle;
  }
  bsum = (bsum ^ 0xff)+1;
  sendbuf[i++] = bsum;
  if (bsum == dle)
    sendbuf[i++] = dle;
  sendbuf[i++] = dle;
  sendbuf[i++] = eot;

  DebugMsg("SND SERIAL %d", i);
  DebugBytes(sendbuf, i);

  Send(sendbuf, i);
  return 0;
}

static int garmin_assign_protocols(int id, int version)
{
  int i;

  link_protocol = -1;
  command_protocol = -1;
  app_protocol[POINT_PROTOCOL] = -1;
  app_protocol[ROUTE_PROTOCOL] = -1;
  app_protocol[TRACK_PROTOCOL] = -1;
  app_protocol[PVT_PROTOCOL] = -1;

  for (i = 0; i < 3; i++) {
    data_protocol[POINT_PROTOCOL][i] = -1;
    data_protocol[ROUTE_PROTOCOL][i] = -1;
    data_protocol[TRACK_PROTOCOL][i] = -1;
    data_protocol[PVT_PROTOCOL][i] = -1;
  }

  for (i = 0; garmin_protocol[i].id != -1; i++) {
    if (id == garmin_protocol[i].id && version >= garmin_protocol[i].version) {
      link_protocol = garmin_protocol[i].link;
      command_protocol = garmin_protocol[i].cmnd;

      app_protocol[POINT_PROTOCOL] = garmin_protocol[i].wpt;
      data_protocol[POINT_PROTOCOL][0] = garmin_protocol[i].wpt_d0;

      app_protocol[ROUTE_PROTOCOL] = garmin_protocol[i].rte;
      data_protocol[ROUTE_PROTOCOL][0] = garmin_protocol[i].rte_d0;
      data_protocol[ROUTE_PROTOCOL][1] = garmin_protocol[i].rte_d1;

      app_protocol[TRACK_PROTOCOL] = garmin_protocol[i].trk;
      data_protocol[TRACK_PROTOCOL][0] = garmin_protocol[i].trk_d0;

      app_protocol[PVT_PROTOCOL] = -1;
      return i;
    }
  }

  return -1;
}

static int pack_word(uint16_t w, uint8_t *buf, int i)
{
  buf[i++] = w & 0x00ff;
  buf[i++] = w >> 8;
  return i; 
}

static int pack_dword(uint32_t l, uint8_t *buf, int i)
{
  buf[i++] = (uint8_t)(l & 0xff);
  buf[i++] = (uint8_t)((l >> 8) & 0xff);
  buf[i++] = (uint8_t)((l >> 16) & 0xff);
  buf[i++] = (uint8_t)((l >> 24) & 0xff);
  return i; 
}

static int pack_string(char *s, uint8_t *buf, int i)
{
  int k;
  for (k = 0; s[k]; k++)
    buf[i++] = s[k];
  buf[i++] = 0;
  return i; 
}

static uint16_t unpack_word(uint8_t *buf)
{
  uint16_t w;
  w = ((uint16_t)buf[0]) | (((uint16_t)buf[1]) << 8);
  return w;
}

static uint32_t unpack_dword(uint8_t *buf)
{
  uint32_t l;
  l = ((uint32_t)buf[0]) | (((uint32_t)buf[1]) << 8) |
      (((uint32_t)buf[2]) << 16) | (((uint32_t)buf[3]) << 24);
  return l;
}

static float invert_float(float f)
{
  float g;
  uint8_t *p, *q;

  p = (uint8_t *)&f;
  q = (uint8_t *)&g;

  q[0] = p[3];
  q[1] = p[2];
  q[2] = p[1];
  q[3] = p[0];

  return g;
}

static double invert_double(double d)
{
  double e;
  uint8_t *p, *q;
 
  p = (uint8_t *)&d;
  q = (uint8_t *)&e;
      
  q[0] = p[7];
  q[1] = p[6];
  q[2] = p[5];
  q[3] = p[4];
  q[4] = p[3];
  q[5] = p[2];
  q[6] = p[1];
  q[7] = p[0];

  return e;
}

static int gps_strcmp(char *s1, char *s2)
{
  int i, d;

  for (i = 0; s1[i] && s2[i]; i++) {
    d = s1[i] - s2[i];
    if (d < 0) return -1;
    if (d > 0) return 1;
  }
  d = s1[i] - s2[i];
  if (d < 0) return -1;
  if (d > 0) return 1;

  return 0;
}

static int gps_strncpy(char *dst, char *src, int n, int fill, int convert)
{
  int i, j, k;

  // convert == 0 : any character
  // convert == 1 : only uppercase, digits, space and dash
  // convert == 2 : only uppercase and digits

  for (i = 0, j = 0; i < n && src[i]; i++)
    if (convert) {
      if (src[i] >= 'a' && src[i] <= 'z')
        dst[j++] = src[i] & 0xdf;
      else if (src[i] >= 'A' && src[i] <= 'Z')
        dst[j++] = src[i];
      else if (src[i] >= '0' && src[i] <= '9')
        dst[j++] = src[i];
      else if (convert == 1 && (src[i] == ' ' || src[i] == '-'))
        dst[j++] = src[i];
    } else
      dst[j++] = src[i];

  if (!fill) {
    dst[j] = 0;
    return j;
  }

  for (k = j; j < n; j++)
    dst[j] = ' ';

  return k;
}

static int gps_strlen(char *s)
{
  int i;

  for (i = 0; s[i]; i++);

  return i;
}

static void gps_memcpy(char *dst, char *src, int n)
{
  int i;

  for (i = 0; i < n; i++)
    dst[i] = src[i];
}

static void gps_memset(char *s, char c, int n)
{
  int i;

  for (i = 0; i < n; i++)
    s[i] = c;
}

static void debug_packet(int d, uint16_t id, uint8_t *packet, int n)
{
  switch (id) {
    case L001_Command_Data:
    case L000_Ack_Byte:
    case L000_Nak_Byte:
      break;
    default:
      DebugMsg("%s GRMN ID %u DATA %d", direction[d], id, n);
      DebugBytes(packet, n);
  }
}

static void debug_command(int d, uint16_t cmd)
{
  DebugMsg("%s GRMN CMD %u", direction[d], cmd);
}

static void debug_ack(int d, int ack, uint16_t id)
{
  DebugMsg("%s GRMN %s %u", direction[d], ack ? "ACK" : "NAK", id);
}
