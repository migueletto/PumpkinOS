#define AppID   'Navg'
#define AppName "Navegador"

#define WaypointsFName	"NavPoints"
#define WaypointsFType	'Data'

#define RoutesFName	"NavRoutes"
#define RoutesFType	'Data'
#define RtePointsFType	'NavR'

#define SymbolsFName	"NavSymbols"
#define SymbolsFType	'Data'

#define TracksFType	'Trck'

#define LogName		"NavLog"
#define LogType		'NavL'

#define DebugName	"NavDebug"
#define DebugType	'NavD'

#define ScreenName	"NavScreen"
#define ScreenType	'NavB'

#define MapFType	'MAPA'

#define TempName	"NavTemp"
#define TempType	'Temp'

#define PREFS_VERSION	1

#define MAX_DEVICES	16

#define TAM_BUF		2048
#define TAM_HOST        64
#define TAM_PORT        8
#define TAM_URL         256

#define UNIT_METRIC	0
#define UNIT_STATUTE	1

#define ACTION_NONE		0
#define ACTION_CONNECTION	1
#define ACTION_POINT		2
#define ACTION_NEXT		3
#define ACTION_ZOOMIN		4
#define ACTION_ZOOMOUT		5
#define ACTION_CAPTURE		6

typedef struct {
  CoordType coord;
  double speed_limit, avg_speed, max_speed, proximity_limit, distance;
  double center_lat, center_lon;
  UInt32 start_time, moving_time;
  UInt16 datum;
  UInt16 autopoint;
  UInt16 track;
  UInt16 display;
  UInt8 log_enabled;
  UInt8 log_beep;
  UInt8 unit_system;
  UInt8 speed_warning;
  UInt8 proximity_alarm;
  UInt8 serial_port;
  UInt8 serial_baud;
  UInt8 protocol;
  UInt8 caseless;
  UInt8 exact;
  UInt8 density;
  UInt8 showtrack;
  UInt8 current_zoom;
  UInt8 locked;
  UInt8 mapfont;
  UInt8 proximity_sort;
  UInt8 positionColor;
  UInt8 currentColor;
  UInt8 storedColor;
  UInt8 selectedColor;
  UInt8 targetColor;
  UInt8 action[4];
  UInt16 capture;
  char mapname[32];
  char nethost[TAM_HOST];
  char neturl[TAM_URL];
  UInt16 netport;
  UInt8 mapdetail;
  UInt8 pad;
  UInt16 route;
} AppPrefs;

Err AppInit(void *);
void AppFinish(void);
void SetEventHandler(FormPtr, Int16);
Boolean InterceptEvent(EventPtr);
Boolean ActionEvent(UInt16, EventPtr);
void MenuEvent(UInt16);
void PopForm(void);
void idle(void);

void resizeForm(FormPtr frm);

Boolean MainFormHandleEvent(EventPtr);
Boolean SatFormHandleEvent(EventPtr);
Boolean MapFormHandleEvent(EventPtr);
Boolean CompassFormHandleEvent(EventPtr);
Boolean TripFormHandleEvent(EventPtr);
Boolean GpsFormHandleEvent(EventPtr);
Boolean UnitsFormHandleEvent(EventPtr);
Boolean ButtonsFormHandleEvent(EventPtr);
Boolean FindFormHandleEvent(EventPtr);
Boolean ProgressFormHandleEvent(EventPtr);
Boolean AboutFormHandleEvent(EventPtr);
Boolean ObjectListHandleEvent(EventPtr event);
Boolean TracksListHandleEvent(EventPtr event);
Boolean EditPointFormHandleEvent(EventPtr event);
Boolean NetworkFormHandleEvent(EventPtr event);
Boolean SymbolFormHandleEvent(EventPtr event);
Boolean LoadMapFormHandleEvent(EventPtr event);
Boolean MapFindFormHandleEvent(EventPtr event);
Boolean EditRouteFormHandleEvent(EventPtr event);
Boolean SelectPointFormHandleEvent(EventPtr event);
Boolean FollowRouteFormHandleEvent(EventPtr event);
Boolean MapColorFormHandleEvent(EventPtr event);
Boolean AstroFormHandleEvent(EventPtr);

Boolean AlignUpperGadgets(FormPtr frm);
Boolean GadgetCallback(UInt16 id, UInt16 cmd, void *param);
Boolean ProgressGadgetCallback(FormGadgetTypeInCallback *gad, UInt16 cmd, void *param);
Boolean LogGadgetCallback(FormGadgetTypeInCallback *gad, UInt16 cmd, void *param);
Boolean StatusGadgetCallback(FormGadgetTypeInCallback *gad, UInt16 cmd, void *param);
Boolean CenterGadgetCallback(FormGadgetTypeInCallback *gad, UInt16 cmd, void *param);
Boolean ZoominGadgetCallback(FormGadgetTypeInCallback *gad, UInt16 cmd, void *param);
Boolean ZoomoutGadgetCallback(FormGadgetTypeInCallback *gad, UInt16 cmd, void *param);
Boolean LockGadgetCallback(FormGadgetTypeInCallback *gad, UInt16 cmd, void *param);
Boolean SymbolGadgetCallback(FormGadgetTypeInCallback *gad, UInt16 cmd, void *param);
Boolean NearestGadgetCallback(FormGadgetTypeInCallback *gad, UInt16 cmd, void *param);

UInt16 GetTrackNum(void);
char *GetTrackName(UInt16 i);
UInt32 GetTrackSize(UInt16 i);
void SelectTrack(UInt16 i);

void SetSpeedUI(double speed);
void SavePoint(char *name, UInt16 symbol, double lat, double lon);
