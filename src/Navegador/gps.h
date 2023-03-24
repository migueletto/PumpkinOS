#define FAILURE_ROM		0x01
#define FAILURE_RAM		0x02
#define FAILURE_EEPROM		0x04
#define FAILURE_DUAL_PORT_RAM	0x08
#define FAILURE_DSP		0x10
#define FAILURE_RTC		0x20

#define MAX_CHANNELS		12

#define CHANNEL_USED		0x1
#define CHANNEL_EPHEMERIDS	0x2
#define CHANNEL_VALID		0x4
#define CHANNEL_DGPS		0x8

#define PROTOCOL_ZODIAC		0
#define PROTOCOL_NMEA		1
#define PROTOCOL_GARMIN		2
#define PROTOCOL_GARMIN_HOST	3
#define PROTOCOL_GPSLIB		4

#define STR_UNKNOWN		"Unknown"
#define STR_DELORME		"Delorme"
#define STR_TRIPMATE		"Tripmate"
#define STR_EARTHMATE		"Earthmate"
#define STR_GARMIN		"Garmin"
#define STR_GENERIC		"Generic"
#define STR_ZODIAC		"Zodiac"
#define STR_NMEA		"NMEA"
#define STR_GARMINHOST		"Garmin Host"
#define STR_GPSLIB		"GPSLib"
#define STR_HOST		"Host"

#define TAM_NAME		16
#define TAM_ROUTENAME		24
#define TAM_TRACKNAME		32
#define TAM_COMMENT		48	

#define LINK_PROTOCOL		0
#define POINT_PROTOCOL		1
#define ROUTE_PROTOCOL		2
#define TRACK_PROTOCOL		3
#define PVT_PROTOCOL		4

typedef struct {
  uint16_t flags;
  uint16_t prn;
  uint16_t cno;
} ChannelSummary;

typedef struct {
  uint16_t prn;
  double azimuth;
  double elevation;
} VisibleSatellite;

typedef struct {
  uint16_t year;
  uint8_t month, day, hour, minute, second;
} TimestampType;

typedef struct {
  double longitude, latitude, height;
} CoordType;

typedef struct {
  char name[TAM_NAME];
  char comment[TAM_COMMENT];
  uint16_t datum;
  TimestampType date;
  uint16_t symbol;
  CoordType coord;
} WaypointType;

typedef struct {
  char ident[TAM_ROUTENAME];
  uint8_t id;
} RouteType;

typedef struct {
  char name[TAM_NAME];
  uint16_t symbol;
  CoordType coord;
  uint16_t seq;
} RoutePointType;

typedef struct {
  uint32_t t;
  float longitude, latitude, height;
} TracklogType;

int gps_protocol(int p);
int gps_init(double lat, double lon, double height, uint16_t datum);
int gps_buf(uint8_t *rbuf, int n);

int zodiac_setdatum(uint16_t);
int zodiac_test(void);
int zodiac_msgenable(uint16_t id, int);
int zodiac_msgrequest(uint16_t id);
int zodiac_restart(uint16_t);

int nmea_sentence(uint8_t *rbuf, int n);

int garmin_product_rqst(void);
int garmin_command(uint16_t);
int garmin_abort_transfer(void);
int32_t garmin_to_coord(double d);
double garmin_from_coord(int32_t l);

int garmin_transfer_wpt(void);
int garmin_send_wpt(void);
int garmin_send_wpts(void);

int garmin_transfer_rte(void);
int garmin_send_rte(void);
int garmin_send_rtes(void);

int garmin_transfer_trk(void);
int garmin_send_trk(void);
int garmin_send_curr_trk(void);
int garmin_send_trks(void);

int garmin_start_pvt(void);
int garmin_stop_pvt(void);

int garmin_transfer_disp(void);

void GetUTC(unsigned char *, unsigned char *, uint16_t *, unsigned char *, unsigned char *, unsigned char *);

int IncCounter(void);

int GetNumPoints(void);
int GetPoint(int index, WaypointType *point);
int GetSelectedPoint(WaypointType *point);
int AddPoint(WaypointType *point);

int GetNumRoutes(void);
int GetRoute(int index, RouteType *rte);
int GetSelectedRoute(RouteType *rte);
int GetRoutePoint(int index, RouteType *rte, WaypointType *point);
int AddRoute(RouteType *rte);
int AddRoutePoint(RouteType *rte, WaypointType *point);

int GetNumTracks(void);
int GetTrack(int index, char *name);
int GetSelectedTrack(char *name);
int GetTrackLog(TracklogType *log);

int AddTrack(char *name);
int AddTrackLog(TracklogType *log);

int SetCurrentTrack(void);
int AddCurrentTrackLog(TracklogType *log);

int BeginImport(int n);
int BeginExport(int n);
int EndTransfer(void);

void SetHandshake(int16_t);
void SetProtocol(int, int, int *);
void SetAlive(void);
void SetValidity(int16_t v, int16_t n);
void SetUTC(char, char, int16_t, char, char, char);
void SetPosition(double lat, double lon);
void SetHeight(double n);
void SetSep(double n);
void SetSpeed(double n);
void SetXYSpeed(double x, double y);
void SetClimb(double n);
void SetCourse(double n);
void SetVar(double n);
void SetMessages(int32_t n);
void SetErrors(int32_t n);
void SetEPE(double h, double v);
void SetDOP(double gdop, double pdop, double hdop, double vdop, double tdop);
void SetFailure(uint16_t);
void SetDatum(uint16_t);
void SetDevice(char *vendor, char *model, char *version);
void SetChannelSummary(int16_t n, ChannelSummary *chs);
void SetVisible(int16_t n, VisibleSatellite *sat);

void StartDisplay(int width, int height, int bpp);
void SendDisplayRow(uint8_t *buf, int n);

void Send(uint8_t *buf, int n);
void SaveData(void);
void ExpandText(WaypointType *p, char *mask, char *result, int max);
