#define PI		3.14159265358979323846
#define TORAD(a)	(((a)*PI)/180.0)
#define TODEG(a)	(((a)*180.0)/PI)

#define M_TO_FEET	3.2808269
#define MILE_TO_FEET	5279.9791
#define KM_TO_MILE	0.6213712
#define EARTH_RADIUS	6378.1  
#define EARTH_RADIUS_M	6378100.0
#define DEG_TO_KM	(TORAD(1.0)*EARTH_RADIUS)

#define MIN_DISTANCE	50.0
#define MAX_DISTANCE	50000.0
#define MAX_DISTANCE_R	0.00783932
// MAX_DISTANCE_R = MAX_DISTANCE / EARTH_RADIUS_M

void FormatDateTime(char *buf, UInt8 day, UInt8 month, UInt16 year,
                    UInt8 hour, UInt8 minute, UInt8 second);
void FormatDate(char *buf, UInt8 day, UInt8 month, UInt16 year, Boolean sep);
void FormatTime(char *buf, UInt8 hour, UInt8 minute, UInt8 second, Boolean sep);
void FormatDistance(char *buf, double distance);
void FormatArea(char *buf, double side);
void FormatSpeed(char *buf, double speed);
void gStrPrintF(char *buf, double f);
void gnStrPrintF(char *buf, double f, Int16 ci);
void fStrPrintF(char *buf, double f, Int16 i, Int16 d);
void hStrPrintFd(char *buf, double d);
void hStrPrintFi(char *buf, UInt32 i);
void hStrPrintF(char *buf, Int16 h, Int16 m, Int16 s);
