#include <PalmOS.h>
#include <GPSLib68K.h>

#include "gui.h"
#include "gps.h"
#include "app.h"
#include "main.h"
#include "garmin.h"
#include "gpslib.h"
#include "protocol.h"
#include "log.h"
#include "serial.h"
#include "misc.h"
#include "error.h"
#include "list.h"
#include "ddb.h"
#include "trig.h"
#include "compass.h"
#include "map.h"
#include "mapdecl.h"
#include "format.h"
#include "sound.h"
#include "sat.h"
#include "astro.h"
#include "trip.h"
#include "file.h"
#include "symbol.h"
#include "font.h"
#include "datum.h"
#include "thin.h"
#include "network.h"
#include "object.h"
#include "point.h"
#include "route.h"
#include "tracks.h"
#include "display.h"
#include "ndebug.h"
#include "MathLib.h"
#include "pumpkin.h"

#include "debug.h"

#define RX_TIMEOUT 8
#define MSG_TIMEOUT 16
#define PVT_TIMEOUT 8
#define MAX_ERRORS 8

#define X_DIR  80
#define Y_DIR  71
#define R_DIR  28
#define H_DIR  5 // arrow head

#define X_SKY  80
#define Y_SKY  71
#define R_SKY  45

static Err GpsOnline(void);
static void GpsOffline(void);
static Int16 GpsReceive(void);

static void FatalError(void);

static void SetRecords(void);
static void Center(void);
static void ZoomIn(void);
static void ZoomOut(void);
static void ZoomOffset(Int16 offset);
static void FillNearest(void);
static void InsertNearest(UInt16 i, double d);
static Int16 CompareNearest(void *e1, void *e2, Int32 other);
static void DrawGotoPoint(char *name, FormPtr frm);
static void SetLocalTime(void);

typedef struct {
  UInt16 index;
  double distance;
} NearType;

static UInt16 pointForm;
static Boolean highDensity = false;
static Boolean gpsLib = false;

static char vendor[16], model[32], version[16];
static char gpsid[64];
static FileHand currentLog, storedLog, importedLog, newLog;
static DmOpenRef ptRef, rtRef, dataRef;
static UInt16 gotoActive;
static WaypointType point, savepoint;
static UInt16 firstSymbol;
static UInt16 currentIndex, currentSeq;
static char *progressTitle;
static Int32 progressIndex, progressTotal;
static UInt32 garmin_t0;
static Boolean getting_current;
static char **datum_names;
static Int16 num_datums;
static UInt16 nearnum, nearmax, drawn[3];
static NearType nearest[3];
static char findbuf[16];
static Boolean setTime;
static UInt16 lastRoute;

static Int16 measure;
static CoordType target;
static TimestampType ts;
static UInt32 last_time = 0;
static double last_ehpe = 0.0, last_evpe = 0.0;
static double last_hdop = 0.0, last_vdop = 0.0;
static double last_sep = 0.0, last_speed = 0.0;
static double last_course = 0.0, last_var = 0.0, last_climb = 0.0;
static double desired_course = 0.0;
static int16_t last_visible = 0, last_validity = 0, last_solution = 0;
static ChannelSummary last_chs[MAX_CHANNELS];
static VisibleSatellite last_sat[MAX_CHANNELS];
static UInt16 x_dir, y_dir, r_dir, h_dir;
static UInt16 errors = 0;
static int32_t last_messages = 0;
static Int16 handshake;

static UInt16 gpsRefnum;
static UInt16 GpsLibRef;
static UInt32 deviceCreators[MAX_DEVICES];
static char *deviceNames[MAX_DEVICES];
static UInt16 numDevices;

static UInt8 inbuf[TAM_BUF];
static Boolean gpsOnline;
static UInt32 last_response = 0;
static UInt32 last_msg = 0;
static UInt32 last_pvt = 0;

static RectangleType mapWindow, hmapWindow;
static UInt32 screenx, screeny, hscreenx, hscreeny;
static FileType tracks;

static Int16 current_zoom = DEFAULT_ZOOM;
static double zoom_factor[NUM_ZOOM] = {
  4 *   0.002, // 0
  4 *   0.005, // 1
  4 *   0.010, // 2
  4 *   0.020, // 3
  4 *   0.050, // 4
  4 *   0.100, // 5
  4 *   0.200, // 6
  4 *   0.500, // 7
  4 *   1.000, // 8
  4 *   2.000, // 9
  4 *   5.000, // 10
  4 *  10.000, // 11
  4 *  20.000, // 12
  4 *  50.000, // 13
  4 * 100.000, // 14
  4 * 200.000 // 15
};

static MemHandle font;
static WinHandle whBuf;
static Int16 mapfont[3];

Boolean ActionEvent(UInt16 i, EventPtr event)
{
  static UInt32 last_ticks = 0;
  Boolean handled;
  AppPrefs *prefs;
  EventType param;
  UInt32 ticks, min;
  UInt16 act;
  UInt16 form = FrmGetActiveFormID();

  handled = false;
  prefs = GetPrefs();
  ticks = TimGetTicks();
  min = 3*SysTicksPerSecond()/4;
  if (prefs->action[i] != ACTION_NONE && (ticks - last_ticks) < min)
    return true;
  last_ticks = ticks;

  switch (act = prefs->action[i]) {
    case ACTION_CONNECTION:
      param.eType = frmGadgetEnterEvent;
      StatusGadgetCallback(NULL, formGadgetHandleEventCmd, &param);
      handled = true;
      break;
    case ACTION_POINT:
      switch (FrmGetActiveFormID()) {
        case MainForm:
        case SatForm:
        case MapForm:
        case CompassForm:
        case SeekPointForm:
        case FollowRouteForm:
        case WaypointsForm:
        case RoutesForm:
        case TracksForm:
        case TripForm:
        case AstroForm:
          FrmPopupForm(AutoPointForm);
          handled = true;
          break;
        case EditPointForm:
        case NewPointForm:
          handled = true;
          break;
        case AutoPointForm:
          // the same buttons opens and closes the form
          event->data.keyDown.modifiers = 0;
          event->data.keyDown.chr = escapeChr;
          handled = false;
      }
      break;
    case ACTION_NEXT:
      switch (FrmGetActiveFormID()) {
        case MainForm:
          MenuEvent(SatForm);
          break;
        case SatForm:
          MenuEvent(MapForm);
          break;
        case MapForm:
          switch (gotoActive) {
            case 0:
              MenuEvent(CompassForm);
              break;
            case 1:
              MenuEvent(SeekPointForm);
              break;
            case 2:
              MenuEvent(FollowRouteForm);
          }
          break;
        case CompassForm:
        case SeekPointForm:
        case FollowRouteForm:
          MenuEvent(WaypointsForm);
          break;
        case WaypointsForm:
          MenuEvent(RoutesForm);
          break;
        case RoutesForm:
          MenuEvent(TracksForm);
          break;
        case TracksForm:
          MenuEvent(TripForm);
          break;
        case TripForm:
          MenuEvent(AstroForm);
          break;
        case AstroForm:
          MenuEvent(MainForm);
      }
      handled = true;
      break;
    case ACTION_ZOOMIN:
      if (form == MapForm)
        ZoomIn();
      handled = true;
      break;
    case ACTION_ZOOMOUT:
      if (form == MapForm)
        ZoomOut();
      handled = true;
      break;
    case ACTION_CAPTURE:
      SaveScreen(hscreenx, hscreeny);
      InfoDialog(INFO, "Done");
      handled = true;
  }

  return handled;
}

Boolean AlignUpperGadgets(FormPtr frm) {
  Int16 d;

  d = FrmObjectRightAlign(frm, FrmGetObjectIndex(frm, statusCtl), -1);
  FrmObjectRightAlign(frm, FrmGetObjectIndex(frm, logCtl), d);

  return true;
}

void resizeForm(FormPtr frm) {
  RectangleType rect;
  Coord width, height;
  WinHandle wh;

  WinGetDisplayExtent(&width, &height);
  RctSetRectangle(&rect, 0, 0, width, height);
  wh = FrmGetWindowHandle(frm);
  WinSetBounds(wh, &rect);
}

Boolean MainFormHandleEvent(EventPtr event)
{
  FormPtr frm;
  AppPrefs *prefs;
  Boolean handled;

  handled = false;

  prefs = GetPrefs();
  switch (event->eType) {
    case frmOpenEvent:
      frm = FrmGetActiveForm();
      resizeForm(frm);

      FrmSetGadgetHandler(frm, FrmGetObjectIndex(frm, logCtl),
          LogGadgetCallback);
      FrmSetGadgetHandler(frm, FrmGetObjectIndex(frm, statusCtl),
          StatusGadgetCallback);
      AlignUpperGadgets(frm);
      FrmDrawForm(frm);

      SetDevice(vendor, model, version);
      SetValidity(last_validity, last_solution);
      SetVisible(last_visible, last_sat);
      SetPosition(prefs->coord.latitude, prefs->coord.longitude);
      SetLocalTime();
      SetHeight(prefs->coord.height);
      SetSpeedUI(last_speed);
      SetCourse(last_course);
      SetVar(last_var);
      SetEPE(last_ehpe, last_evpe);
      SetDOP(0.0, 0.0, last_hdop, last_vdop, 0.0);
      SetChannelSummary(MAX_CHANNELS, last_chs);

      handled = true;
      break;

    case frmUpdateEvent:
      frm = FrmGetActiveForm();
      FrmDrawForm(frm);
      handled = true;
      break;

    case nilEvent:
      idle();
      handled = true;
      break;

    case menuEvent:
      MenuEvent(event->data.menu.itemID);
      handled = true;
      break;

    default:
      break;
  }

  return handled;
}

Boolean SatFormHandleEvent(EventPtr event)
{
  FormPtr frm;
  Boolean handled;

  handled = false;

  switch (event->eType) {
    case frmOpenEvent:   
      frm = FrmGetActiveForm();
      resizeForm(frm);

      FrmSetGadgetHandler(frm, FrmGetObjectIndex(frm, logCtl),
          LogGadgetCallback);
      FrmSetGadgetHandler(frm, FrmGetObjectIndex(frm, statusCtl),
          StatusGadgetCallback);
      AlignUpperGadgets(frm);
      FrmDrawForm(frm);

      SetVisible(last_visible, last_sat);
      SetChannelSummary(MAX_CHANNELS, last_chs);
      MapInvalid();

      handled = true;
      break;

    case frmUpdateEvent:
      frm = FrmGetActiveForm();
      FrmDrawForm(frm);
      SetVisible(last_visible, last_sat);
      SetChannelSummary(MAX_CHANNELS, last_chs);
      handled = true;
      break;

    case nilEvent:
      idle();
      handled = true;
      break;

    case menuEvent:
      MenuEvent(event->data.menu.itemID);
      handled = true;

    default:
      break;
  }

  return handled;
}

static void AlignMapGadgets(FormPtr frm) {
  Int16 d;

  d = FrmObjectRightAlign(frm, FrmGetObjectIndex(frm, lockCtl), -1);
  FrmObjectRightAlign(frm, FrmGetObjectIndex(frm, zoomoutCtl), d);
  FrmObjectRightAlign(frm, FrmGetObjectIndex(frm, zoominCtl), d);
  FrmObjectRightAlign(frm, FrmGetObjectIndex(frm, centerCtl), d);

  d = FrmObjectBottomAlign(frm, FrmGetObjectIndex(frm, lockCtl), -1);
  FrmObjectBottomAlign(frm, FrmGetObjectIndex(frm, zoomoutCtl), d);
  FrmObjectBottomAlign(frm, FrmGetObjectIndex(frm, zoominCtl), d);
  FrmObjectBottomAlign(frm, FrmGetObjectIndex(frm, centerCtl), d);
}

Boolean MapFormHandleEvent(EventPtr event)
{
  FormPtr frm;
  AppPrefs *prefs;
  Boolean handled;
  static Int16 xpen, ypen;
  static Boolean penDown;

  handled = false;

  switch (event->eType) {
    case frmOpenEvent:
      frm = FrmGetActiveForm();
      resizeForm(frm);

      AlignMapGadgets(frm);
      AlignUpperGadgets(frm);
      FrmSetGadgetHandler(frm, FrmGetObjectIndex(frm, centerCtl),  CenterGadgetCallback);
      FrmSetGadgetHandler(frm, FrmGetObjectIndex(frm, zoominCtl),  ZoominGadgetCallback);
      FrmSetGadgetHandler(frm, FrmGetObjectIndex(frm, zoomoutCtl), ZoomoutGadgetCallback);
      FrmSetGadgetHandler(frm, FrmGetObjectIndex(frm, lockCtl),    LockGadgetCallback);
      FrmSetGadgetHandler(frm, FrmGetObjectIndex(frm, logCtl),     LogGadgetCallback);
      FrmSetGadgetHandler(frm, FrmGetObjectIndex(frm, statusCtl),  StatusGadgetCallback);
      FrmDrawForm(frm);

      if (GetRecNum(1) > 0 && GetRecSelection(1) != lastRoute) {
        lastRoute = GetRecSelection(1);
        MapInvalid();
      }

      MapDrawScale(zoom_factor[current_zoom], hscreenx, hscreeny);
      MapDraw();

      penDown = false;
      measure = 0;
      handled = true;
      break;

    case frmUpdateEvent:
      frm = FrmGetActiveForm();
      FrmDrawForm(frm);
      MapDrawScale(zoom_factor[current_zoom], hscreenx, hscreeny);
      MapDraw();
      handled = true;
      break;

    case nilEvent:
      if (!penDown)
        idle();
      handled = true;
      break;
  
    case menuEvent:
      MenuEvent(event->data.menu.itemID);
      handled = true;
      break;

    case penDownEvent:
      if (RctPtInRectangle(event->screenX, event->screenY, &mapWindow)) {
        prefs = GetPrefs();
        if (prefs->locked && !measure)
          SndPlaySystemSound(sndError);
        else {
          penDown = true;
          xpen = event->screenX;
          ypen = event->screenY;
        }
        handled = true;
      } else if (measure == 2) {
        measure = 0;
        MapStopMeasure();
        MapDraw();
      }
      break;
    case penMoveEvent:
      if (RctPtInRectangle(event->screenX, event->screenY, &mapWindow)) {
        if (penDown) {
          if (measure == 1) {
            Int16 dx = hscreenx / screenx;
            Int16 dy = hscreeny / screeny;
            MapMeasurePoint(xpen*dx, ypen*dy, event->screenX*dx, event->screenY*dy);
          } else if (measure == 0) {
            Int16 dx = event->screenX - xpen;
            Int16 dy = event->screenY - ypen;
            dx *= hscreenx / screenx;
            dy *= hscreeny / screeny;
            MapDrawRel(whBuf, WinGetActiveWindow(), dx, dy);
            MapInvalid();
          }
        }
        handled = true;
      } else if (penDown && measure) {
        penDown = false;
        measure = 0;
        MapStopMeasure();
      }
      break;
    case penUpEvent:
      if (RctPtInRectangle(event->screenX, event->screenY, &mapWindow)) {
        if (penDown) {
          if (measure == 1) {
            // distance
            measure = 0;
            MapStopMeasure();
          } else if (measure == 2) {
            // area
            Int16 dx = hscreenx / screenx;
            Int16 dy = hscreeny / screeny;
            MapDraw();
            MapMeasureAddPoint(event->screenX*dx, event->screenY*dy);
          } else {
            Int16 dx = event->screenX - xpen;
            Int16 dy = event->screenY - ypen;
            if (highDensity) {
              dx *= hscreenx / screenx;
              dy *= hscreeny / screeny;
            }
            MapMoveXY(dx, dy);
            MapDraw();
          }
          penDown = false;
        }
        handled = true;
      } else if (penDown && measure) {
        penDown = false;
        measure = 0;
        MapStopMeasure();
      }
      break;

    default:
      break;

  }

  return handled;
}

Boolean TripFormHandleEvent(EventPtr event)
{
  FormPtr frm;
  Boolean handled;
  AppPrefs *prefs;
      
  handled = false;

  switch (event->eType) {
    case frmOpenEvent:
      frm = FrmGetActiveForm();
      resizeForm(frm);

      FrmSetGadgetHandler(frm, FrmGetObjectIndex(frm, logCtl),
          LogGadgetCallback);
      FrmSetGadgetHandler(frm, FrmGetObjectIndex(frm, statusCtl),
          StatusGadgetCallback);
      AlignUpperGadgets(frm);
      FrmDrawForm(frm);

      SetLocalTime();
      SetSpeedUI(last_speed);
      SetTrip();

      handled = true;
      break;

    case frmUpdateEvent:
      frm = FrmGetActiveForm();
      FrmDrawForm(frm);
      handled = true;
      break;

    case nilEvent:
      idle();
      break;

    case menuEvent:  
      MenuEvent(event->data.menu.itemID);
      handled = true;
      break;

    case ctlSelectEvent:
      if (event->data.ctlSelect.controlID == resetBtn) {
        TripReset(TimGetSeconds(), 0, 0.0);
        prefs = GetPrefs();
        prefs->avg_speed = 0;
        prefs->max_speed = 0;
        SetSpeedUI(last_speed);
        SetTrip();
        handled = true;
      }
      break;

    default:
      break;
  }
      
  return handled;
}

static void FatalError(void)
{
  EndTransfer();
  SndPlaySystemSound(sndError);
  GpsOffline();
  StatusGadgetCallback(NULL, formGadgetDrawCmd, NULL);
}

void idle(void)
{
  Int16 r;
  Int32 now;
  UInt16 n;
  AppPrefs *prefs;
  GPSTimeDataType gpsTime;

  prefs = GetPrefs();

  if (newLog) {
    if ((n = ThinLog(storedLog, newLog, zoom_factor[current_zoom]/4, 16)) > 0) {
      progressIndex += n;
      ProgressGadgetCallback(NULL, formGadgetDrawCmd, NULL);
    } else {
      progressIndex += StopThinLog(storedLog, newLog);
      ProgressGadgetCallback(NULL, formGadgetDrawCmd, NULL);
      EndTransfer();
    }
    return;
  }

  SetLocalTime();
  SetTrip();

  if (gpsOnline) {
    if (prefs->protocol == PROTOCOL_GPSLIB) {
      if (gpsLib && GPSGetTime(GpsLibRef, &gpsTime) == gpsErrNone) {
        DateTimeType dt;
        UInt8 hour, minute;
        TimSecondsToDateTime(TimGetSeconds(), &dt);
        hour = gpsTime.seconds / 3600;
        gpsTime.seconds = gpsTime.seconds % 3600;
        minute = gpsTime.seconds / 60;
        gpsTime.seconds = gpsTime.seconds % 60;
        SetUTC(dt.day, dt.month, dt.year, hour, minute, gpsTime.seconds);
      }
    } else {
      r = GpsReceive();
      now = TimGetSeconds();

      switch (r) {
        case -1:
          FatalError();
          InfoDialog(ERROR, "Communication error, disconnecting");
          break;
        case  0:
          if (prefs->protocol == PROTOCOL_ZODIAC ||
              prefs->protocol == PROTOCOL_NMEA) {
            if ((now - last_response) > RX_TIMEOUT) {
              FatalError();
              InfoDialog(ERROR, "No response from GPS, disconnecting");
            }
          } else if (prefs->protocol == PROTOCOL_GARMIN) {
            if ((now - last_pvt) > PVT_TIMEOUT) {
              SetVisible(0, 0);
              SetValidity(0, 0);
              last_pvt = now;
              //garmin_start_pvt();
            }
          }
          break;
        default:
          last_response = now;
          if (prefs->protocol == PROTOCOL_ZODIAC ||
              prefs->protocol == PROTOCOL_NMEA) {
            if ((now - last_msg) > MSG_TIMEOUT) {
              FatalError();
              InfoDialog(ERROR,
                 "Unknown data received from GPS, disconnecting");
            }
          }
      }
      if (errors > MAX_ERRORS) {
        FatalError();
        InfoDialog(ERROR, "Too many errors, disconnecting");
      }
    }
  }
}

void SaveData(void)
{
  static char buf1[32], buf2[32];
  double old_course, cosd, d = 0, rcx = 0, rcy = 0, rtx = 0, rty = 0;
  AppPrefs *prefs = GetPrefs();
  FormPtr frm = FrmGetActiveForm();
  UInt16 form = FrmGetActiveFormID();

  TripRecord(&ts, &prefs->coord, last_speed);
  TripLog(&ts, &prefs->coord, last_speed, currentLog);

  if (form == TracksForm)
    SetRecords();

  if (gotoActive && currentSeq >= 0)
    RouteGetNext(&currentSeq, &currentIndex, &target);

  if (form == CompassForm || form == SeekPointForm ||
      form == FollowRouteForm || form == MapForm) {

    if (form == MapForm || form == SeekPointForm || form == FollowRouteForm) {
      rcx = TORAD(prefs->coord.longitude);
      rcy = TORAD(prefs->coord.latitude);
      rtx = TORAD(target.longitude);
      rty = TORAD(target.latitude);

      cosd = sin(rcy) * sin(rty) + cos(rcy) * cos(rty) *
               cos(TORAD(fabs(prefs->coord.longitude-target.longitude)));
      d = acos(cosd) * EARTH_RADIUS * 1000.0;

      if (gotoActive && d < MAX_DISTANCE)
        MapSetTarget(target.latitude, target.longitude);
      else
        MapResetTarget();
    }

    if (form == SeekPointForm || form == FollowRouteForm) {
      buf1[0] = 0;
      if (last_speed > 0.0)
        hStrPrintFd(buf1, (d / last_speed) / 3600.0);
      SetField(frm, timeFld, buf1);

      FormatDistance(buf2, d);
      SetField(frm, distFld, buf2);

      old_course = desired_course;
      desired_course = atan2(sin(rcx-rtx)*cos(rty),
                             cos(rcy)*sin(rty)-sin(rcy)*cos(rty)*cos(rcx-rtx));
      desired_course = -TODEG(desired_course);

      if (FrmGetWindowHandle(frm) == WinGetActiveWindow()) {
        if (highDensity) WinSetCoordinateSystem(kCoordinatesDouble);
        DrawArrow(x_dir, y_dir, r_dir, h_dir, old_course, true, false);
        DrawArrow(x_dir, y_dir, r_dir, h_dir, desired_course, true, true);
        if (highDensity) WinSetCoordinateSystem(kCoordinatesStandard);
      }
    }

    if (form == CompassForm || form == SeekPointForm ||
        form == FollowRouteForm) {
      if (FrmGetWindowHandle(frm) == WinGetActiveWindow()) {
        FillNearest();
        NearestGadgetCallback(NULL, formGadgetDrawCmd, NULL);
      }
    }
  }

  SetTrip();
}

static void FillNearest(void)
{
  static UInt32 last_alarm = 0;
  Err err;
  Int16 i, n;
  UInt16 index, rindex;
  UInt32 t;
  double rcy, rty, cosd, d;
  WaypointType *p;
  RoutePointType *rp;
  RouteType *rte;
  DmOpenRef dbRef;
  AppPrefs *prefs = GetPrefs();
  UInt16 form = FrmGetActiveFormID();

  rcy = TORAD(prefs->coord.latitude);
  nearnum = 0;
  nearmax = 0;

  if (form == CompassForm || form == SeekPointForm) {
    n = DmNumRecords(ptRef);
    for (i = 0; i < n; i++) {
      if ((p = (WaypointType *)DbOpenRec(ptRef, i, &err)) == NULL)
        continue;
      
      if (fabs(p->coord.latitude - prefs->coord.latitude) < 2.0 &&
          fabs(p->coord.longitude - prefs->coord.longitude) < 2.0) {
    
        rty = TORAD(p->coord.latitude);
        cosd = sin(rcy) * sin(rty) + cos(rcy) * cos(rty) *
               cos(TORAD(fabs(prefs->coord.longitude - p->coord.longitude)));
        d = acos(cosd) * EARTH_RADIUS * 1000.0;
        InsertNearest(i, d);
      }
      DbCloseRec(ptRef, i, (char *)p);
    }
  } else if (currentSeq >= 0) {
    index = GetRecIndex(1, GetRecSelection(1));
    rte = (RouteType *)DbOpenRec(rtRef, index, &err);

    if (rte && !err) {
      dbRef = DbOpenByName(rte->ident, dmModeReadOnly, &err);

      if (dbRef && !err) {
        n = GetRecNum(2);

        for (i = 0; i < n && nearnum < 3; i++) {
          rindex = GetRecIndex(2, i);

          if ((rp = (RoutePointType *)DbOpenRec(dbRef, rindex, &err)) == NULL)
            continue;

          if (rp->seq >= currentSeq) {
            rty = TORAD(rp->coord.latitude);
            cosd = sin(rcy) * sin(rty) + cos(rcy) * cos(rty) *
                   cos(TORAD(fabs(prefs->coord.longitude-rp->coord.longitude)));
            d = acos(cosd) * EARTH_RADIUS * 1000.0;

            nearest[nearnum].index = rindex;
            nearest[nearnum].distance = d;
            nearnum++;
          }
          DbCloseRec(dbRef, rindex, (char *)rp);
        }
        DbClose(dbRef);
      }
      DbCloseRec(rtRef, index, (char *)rte);
    }
  }

  if (prefs->proximity_alarm) {
    for (i = 0; i < nearnum; i++) {
      if (nearest[i].distance < prefs->proximity_limit) {
        t = TimGetSeconds();
        if ((t - last_alarm) > 15) {
          PlaySound(SOUND_ALARM);   
          last_alarm = t;
        }
      }
    }
  }
}

static Int16 CompareNearest(void *e1, void *e2, Int32 other)
{
  NearType *p1 = (NearType *)e1;
  NearType *p2 = (NearType *)e2;
  if (p1->distance < p2->distance) return -1;
  if (p1->distance > p2->distance) return 1;
  return 0;
}

static void InsertNearest(UInt16 i, double d)
{
  UInt16 i1, i2;

  if (nearnum < 3) {
    nearest[nearnum].index = i;
    nearest[nearnum].distance = d;
    if (d > nearest[nearmax].distance)
      nearmax = nearnum;
    nearnum++;
    SysQSort(nearest, nearnum, sizeof(NearType), CompareNearest, 0);

  } else {
    if (d < nearest[nearmax].distance) {
      nearest[nearmax].index = i;
      nearest[nearmax].distance = d;
      i1 = (nearmax+1) % 3;
      i2 = (nearmax+2) % 3;
      if (nearest[i1].distance > nearest[nearmax].distance) nearmax = i1;
      if (nearest[i2].distance > nearest[nearmax].distance) nearmax = i2;
      SysQSort(nearest, nearnum, sizeof(NearType), CompareNearest, 0);
    }
  }
}

void SetUTC(char day, char month, int16_t year, char hour, char minute, char second)
{
  UInt32 t;
  DateTimeType dt;

  ts.year = year;
  ts.month = month;
  ts.day = day;
  ts.hour = hour;
  ts.minute = minute;
  ts.second = second;

  if (setTime) {
    // synchronize time

    dt.second = second;
    dt.minute = minute;
    dt.hour = hour;
    dt.day = day;
    dt.month = month;
    dt.year = year;
    t = TimDateTimeToSeconds(&dt);

    // takes into account timezone and daylight saving
    if (GetRomVersionNumber() >= 40) {
      t += ((Int32)PrefGetPreference(prefTimeZone)) * 60;
      t += ((Int32)PrefGetPreference(prefDaylightSavingAdjustment)) * 60;
    } else {
      t += (((Int32)PrefGetPreference(prefMinutesWestOfGMT)) - 720) * 60;
      if (PrefGetPreference(prefDaylightSavings) != dsNone)
        t += 3600;
    }

    TimSetSeconds(t);
    last_response = t;
    setTime = false;
  }

  SetLocalTime();
}

static void SetLocalTime(void)
{
  static char buf[32];
  DateTimeType dt;
  FormPtr frm = FrmGetActiveForm();
  UInt16 form = FrmGetActiveFormID();

  if (FrmGetWindowHandle(frm) != WinGetActiveWindow())
    return;

  TimSecondsToDateTime(TimGetSeconds(), &dt);

  if (form == MainForm) {
    FormatDateTime(buf, dt.day, dt.month, dt.year,
                   dt.hour, dt.minute, dt.second);
    SetField(frm, utcFld, buf);
  } else if (form == CompassForm || form == SeekPointForm ||
             form == FollowRouteForm || form == TripForm) {
    FormatTime(buf, dt.hour, dt.minute, dt.second, true);
    SetField(frm, utcFld, buf);
  } else if (form == AstroForm)
    AstroUpdate(&dt);
}

void SetMessages(int32_t messages)
{
  last_messages = messages;
}

void SetErrors(int32_t n)
{
  errors = n;
}

void SetVisible(int16_t visible, VisibleSatellite *sat)
{
  static char buf[8];
  Int16 i;
  FormPtr frm = FrmGetActiveForm();
  UInt16 form = FrmGetActiveFormID();

  last_visible = visible;

  if (sat) {
    for (i = 0; i < visible; i++)
      last_sat[i] = sat[i];
  }

  if (FrmGetWindowHandle(frm) != WinGetActiveWindow())
    return;

  if (form == SatForm) {
    if (sat) {
      if (highDensity) WinSetCoordinateSystem(kCoordinatesDouble);
      SatDrawSky(visible, sat, last_chs);
      if (highDensity) WinSetCoordinateSystem(kCoordinatesStandard);
    }
  } else if (form == MainForm) {
    StrPrintF(buf, "%d", visible);
    SetField(frm, visibleFld, buf);
  }
}

void SetEPE(double h, double v)
{
  static char buf1[32], buf2[32];
  FormPtr frm = FrmGetActiveForm();
  UInt16 form = FrmGetActiveFormID();

  last_ehpe = h;
  last_evpe = v;

  if (FrmGetWindowHandle(frm) != WinGetActiveWindow())
    return;

  if (form == MainForm || form == CompassForm || form == SeekPointForm ||
      form == FollowRouteForm) {
    FormatDistance(buf1, h);
    SetField(frm, ehpeFld, buf1);
  }
  if (form == MainForm) {
    FormatDistance(buf2, v);
    SetField(frm, evpeFld, buf2);
  }
}

void SetDOP(double gdop, double pdop, double hdop, double vdop, double tdop)
{
  static char buf1[32], buf2[32];
  FormPtr frm = FrmGetActiveForm();
  UInt16 form = FrmGetActiveFormID();

  last_hdop = hdop;
  last_vdop = vdop;

  if (form == MainForm) {
    fStrPrintF(buf1, hdop, 2, 2);
    SetField(frm, hdopFld, buf1);
    fStrPrintF(buf2, vdop, 2, 2);
    SetField(frm, vdopFld, buf2);
  }
}

void SetHandshake(int16_t h)
{
  handshake = h;
  StatusGadgetCallback(NULL, formGadgetDrawCmd, NULL);
}

void SetAlive(void)
{
  last_msg = TimGetSeconds();
}

void SetValidity(int16_t validity, int16_t solution)
{
  static char buf[8];
  FormPtr frm = FrmGetActiveForm();
  UInt16 form = FrmGetActiveFormID();

  last_validity = validity;
  last_solution = solution;
 
  if (FrmGetWindowHandle(frm) != WinGetActiveWindow())
    return;

  StatusGadgetCallback(NULL, formGadgetDrawCmd, NULL);

  if (form == MainForm) {
    StrPrintF(buf, "%d", validity ? solution : 0);
    SetField(frm, usedFld, buf);
  }
}

void SetPosition(double lat, double lon)
{
  static char buf1[32], buf2[32];
  AppPrefs *prefs;
  FormPtr frm;
  UInt16 form;

  prefs = GetPrefs();
  prefs->coord.latitude = lat;
  prefs->coord.longitude = lon;

  AstroPosition(lat, lon);
  if (!measure)
    MapPosition(lat, lon, last_speed, last_course);
  ObjectPositionValid(last_validity);

  frm = FrmGetActiveForm();

  if (FrmGetWindowHandle(frm) != WinGetActiveWindow())
    return;

  form = FrmGetActiveFormID();

  if (form == MainForm) {
    gStrPrintF(buf1, lat);
    SetField(frm, latFld, buf1);
    gStrPrintF(buf2, lon);
    SetField(frm, longFld, buf2);
  } else if (form == MapForm) {
    if (!measure)
      MapDraw();
  }
}

void SetHeight(double height)
{
  static char buf[32];
  AppPrefs *prefs = GetPrefs();
  FormPtr frm = FrmGetActiveForm();
  UInt16 form = FrmGetActiveFormID();

  prefs->coord.height = height;

  if (FrmGetWindowHandle(frm) != WinGetActiveWindow())
    return;

  if (form == MainForm || form == CompassForm || form == SeekPointForm ||
      form == FollowRouteForm) {
    FormatDistance(buf, height);
    SetField(frm, heightFld, buf);
  }
}

void SetDatum(uint16_t datum)
{
}

void SetChannelSummary(int16_t n, ChannelSummary *chs)
{
  Int16 i;
  AppPrefs *prefs;
  UInt16 form = FrmGetActiveFormID();

  for (i = 0; i < n; i++)
    last_chs[i] = chs[i];

  if (form == SatForm) {
    if (highDensity) WinSetCoordinateSystem(kCoordinatesDouble);

    SatDrawChannels(n, chs);
    prefs = GetPrefs();
    if (prefs->protocol == PROTOCOL_ZODIAC)
      SatDrawSky(last_visible, last_sat, chs);

    if (highDensity) WinSetCoordinateSystem(kCoordinatesStandard);
  }
}

void SetSep(double sep)
{
  last_sep = sep;
}

void SetSpeed(double speed)
{
  AppPrefs *prefs = GetPrefs();

  last_speed = speed;

  prefs->avg_speed = (prefs->avg_speed + speed) / 2;
  if (speed >= prefs->max_speed)
    prefs->max_speed = speed;

  SetSpeedUI(speed);
}

void SetSpeedUI(double speed)
{
  static char buf1[32], buf2[32], buf3[32];
  AppPrefs *prefs;
  FormPtr frm = FrmGetActiveForm();
  UInt16 form = FrmGetActiveFormID();

  if (FrmGetWindowHandle(frm) != WinGetActiveWindow())
    return;

  if (form == MainForm || form == CompassForm || form == SeekPointForm ||
      form == FollowRouteForm || form == TripForm) {
    FormatSpeed(buf1, speed);
    SetField(frm, speedFld, buf1);

    if (form == TripForm) {
      prefs = GetPrefs();

      FormatSpeed(buf2, prefs->max_speed);
      SetField(frm, maxspeedFld, buf2);

      FormatSpeed(buf3, prefs->avg_speed);
      SetField(frm, aspeedFld, buf3);
    }
  }
}

void SetXYSpeed(double x, double y)
{
  double s = sqrt(x*x + y*y);

  if (s < 0.5)
    s = 0.0;

  SetSpeed(s);
  if (s > 0.0)
    SetCourse(TODEG(atan2(y, x)));

  last_pvt = TimGetSeconds();
}

void SetClimb(double climb)
{
  last_climb = climb;
}

void SetCourse(double course)
{
  static char buf[32];
  char deg[2];
  double c;
  Int16 turn, len, dx, dy;
  RectangleType rect;
  FormPtr frm = FrmGetActiveForm();
  UInt16 form = FrmGetActiveFormID();

  c = course;
  if (c < 0.0) c += 360.0;
  else if (c > 360.0) c -= 360.0;

  fStrPrintF(buf, c, 3, 1);
  deg[0] = 0260;
  deg[1] = 0;
  StrCat(buf, deg);

  if (form == MainForm) {
    if (FrmGetWindowHandle(frm) == WinGetActiveWindow())
      SetField(frm, courseFld, buf);
  }

  if (form == CompassForm || form == SeekPointForm || form == FollowRouteForm) {
    if (FrmGetWindowHandle(frm) == WinGetActiveWindow()) {
      if (highDensity) WinSetCoordinateSystem(kCoordinatesDouble);
      DrawArrow(x_dir, y_dir, r_dir, h_dir, last_course, false, false);
      DrawArrow(x_dir, y_dir, r_dir, h_dir, course, false, true);
      if (form == CompassForm) {
        dx = FntCharWidth('0') * 7;
        dy = FntCharHeight();
        RctSetRectangle(&rect, x_dir - dx/2, y_dir - dy/2, dx, dy);
        WinEraseRectangle(&rect, 0);
        len = StrLen(buf);
        dx = FntCharsWidth(buf, len);
        WinDrawChars(buf, len, x_dir - dx/2, y_dir - dy/2);
      }
      if (highDensity) WinSetCoordinateSystem(kCoordinatesStandard);

      if (form == SeekPointForm || form == FollowRouteForm) {
        buf[0] = 0;
        turn = (Int16)(desired_course - course);

        if (turn < -180) turn += 360;
        else if (turn > 180) turn -= 360;

        if (turn == 0)
          FrmCopyLabel(frm, turnLbl, "\005");
        else if (turn < 0) {
          StrPrintF(buf, "%d%c", -turn, 0260);
          FrmCopyLabel(frm, turnLbl, "\003");
        } else {
          StrPrintF(buf, "%d%c", turn, 0260);
          FrmCopyLabel(frm, turnLbl, "\004");
        }

        SetField(frm, turnFld, buf);
      }
    }
  }

  last_course = course;
}

void SetVar(double var)
{
  last_var = var;
}

void SetFailure(uint16_t failure)
{
  if (failure) {
    FatalError();
    InfoDialog(ERROR, "GPS built-in test failed (%04x)", failure);
  } else
    InfoDialog(INFO, "GPS built-in test OK");
}

void SetDevice(char *vd, char *md, char *vr)
{
  char *s;
  FormPtr frm = FrmGetActiveForm();
  UInt16 form = FrmGetActiveFormID();

  MemSet(gpsid, sizeof(gpsid), 0);

  StrNCopy(vendor, vd, sizeof(vendor)-1);
  StrNCopy(model, md, sizeof(model)-1);
  if ((s = StrStr(model, "Software Version")) != NULL)
    s[0] = 0;
  StrNCopy(version, vr, sizeof(version)-1);
  StrPrintF(gpsid, "%s %s %s", vendor, model, version);

  if (FrmGetWindowHandle(frm) != WinGetActiveWindow())
    return;

  if (form == MainForm)
    SetField(frm, gpsFld, gpsid);
}

int GetNumPoints(void)
{
  return GetRecNum(0);
}

int GetPoint(int index, WaypointType *point)
{
  WaypointType *p;
  Err err;

  progressIndex++;
  ProgressGadgetCallback(NULL, formGadgetDrawCmd, NULL);

  if ((p = (WaypointType *)DbOpenRec(ptRef, index, &err)) == NULL)
    return -1;

  MemMove(point, p, sizeof(WaypointType));
  DbCloseRec(ptRef, index, (char *)p);

  return sizeof(WaypointType);
}

int GetSelectedPoint(WaypointType *point)
{
  return GetPoint(GetRecIndex(0, GetRecSelection(0)), point);
}

int GetNumRoutes(void)
{
  return GetRecNum(1);
}

int GetRoute(int index, RouteType *rte)
{
  RouteType *r;
  DmOpenRef dbRef;
  UInt16 n;
  Err err;

  if ((r = (RouteType *)DbOpenRec(rtRef, index, &err)) == NULL)
    return 0;

  if ((dbRef = DbOpenByName(r->ident, dmModeReadOnly, &err)) == 0) {
    DbCloseRec(rtRef, index, (char *)r);
    return 0;
  }

  ObjectDefine(2, dbRef, 0, 0, 0, GetRoutePointName, NULL,
    GetRoutePointSymbol, GetRoutePointSeq,
    NULL, CompareSeq);

  n = DmNumRecords(dbRef);
  DbClose(dbRef);

  if (rte)
    MemMove(rte, r, sizeof(RouteType));

  DbCloseRec(rtRef, index, (char *)r);

  return n;
}

int GetSelectedRoute(RouteType *rte)
{
  if (GetRecNum(1) == 0)
    return 0;
  return GetRoute(GetRecIndex(1, GetRecSelection(1)), rte);
}

int GetRoutePoint(int index, RouteType *rte, WaypointType *point)
{
  DmOpenRef dbRef;
  RoutePointType *rpoint;
  UInt16 rindex;
  Err err;

  if (index >= GetRecNum(2))
    return 0;

  if ((dbRef = DbOpenByName(rte->ident, dmModeReadOnly, &err)) == 0)
    return 0;

  rindex = GetRecIndex(2, index);

  if ((rpoint = (RoutePointType *)DbOpenRec(dbRef, rindex, &err)) == NULL) {
    DbClose(dbRef);
    return 0;
  }

  progressIndex++;
  ProgressGadgetCallback(NULL, formGadgetDrawCmd, NULL);

  MemSet(point, sizeof(WaypointType), 0);
  StrCopy(point->name, rpoint->name);
  point->symbol = rpoint->symbol;
  point->coord = rpoint->coord;

  DbCloseRec(dbRef, rindex, (char *)rpoint);
  DbClose(dbRef);

  return sizeof(WaypointType);
}

int GetNumTracks(void)
{
  return tracks.n + 1; // +1: currentLog
}

int GetTrack(int index, char *name)
{
  Int16 n;
  AppPrefs *prefs;

  if (index == 0) { // currentLog
    getting_current = true;

    if (name)
      StrCopy(name, "ACTIVE LOG");

    if (currentLog) {
      n = (Int16)(LogSize(currentLog) / sizeof(TracklogType));
      SeekLog(currentLog, -(n * sizeof(TracklogType)));
      return n;
    }
    return 0;
  }

  getting_current = false;

  if (tracks.n == 0)
    return 0;

  index--; // -1: currentLog

  if (storedLog) {
    prefs = GetPrefs();
    CloseLog(storedLog);
    storedLog = OpenLog(tracks.fname[index], AppID,
                        LogType, fileModeReadOnly);
    MapSetStoredLog(prefs->showtrack ? storedLog : NULL);
    if (!storedLog)
      return 0;
  }

  if (name) {
    n = StrLen(tracks.fname[index]);
    StrCopy(name, tracks.fname[index]);
    name[n] = 0;
  }
  n = (UInt16)(tracks.rec[index].size / sizeof(TracklogType));
  SeekLog(storedLog, -(n * sizeof(TracklogType)));
  
  return n;
}

int GetSelectedTrack(char *name)
{
  AppPrefs *prefs;

  prefs = GetPrefs();
  return GetTrack(prefs->track+1, name); // +1 : currentLog
}

int GetTrackLog(TracklogType *log)
{
  FileHand f;

  progressIndex++;
  ProgressGadgetCallback(NULL, formGadgetDrawCmd, NULL);

  f = getting_current ? currentLog : storedLog;

  if (!f)
    return 0;

  if (ReadLog(f, log, sizeof(TracklogType)) == 0) {
    log->t -= garmin_t0;
    return sizeof(TracklogType);
  }

  return 0;
}

int BeginImport(int n)
{
  progressTitle = "Receiving";
  progressIndex = 0;
  progressTotal = n;
  SetWait(25);
  MenuEvent(ProgressForm);
  return 0;
}

int IncCounter(void)
{
  progressIndex++;
  ProgressGadgetCallback(NULL, formGadgetDrawCmd, NULL);
  return 0;
}

int AddPoint(WaypointType *point)
{
  Int16 i;

  progressIndex++;
  ProgressGadgetCallback(NULL, formGadgetDrawCmd, NULL);

  for (i = TAM_NAME-1; i >= 0 && point->name[i] == ' '; i--)
    point->name[i] = 0;

  for (i = TAM_COMMENT-1; i >= 0 && point->comment[i] == ' '; i--)
    point->comment[i] = 0;

  i = FindRec(0, point->name, 0, false, 0);

  if (i >= 0)
    return UpdatePoint(ptRef, GetRecIndex(0, i), point);

  return NewPoint(ptRef, point);
}

int AddRoute(RouteType *rte)
{
  Int16 i;

  progressIndex++;
  ProgressGadgetCallback(NULL, formGadgetDrawCmd, NULL);

  for (i = TAM_ROUTENAME-1; i >= 0 && rte->ident[i] == ' '; i--)
    rte->ident[i] = 0;

  if (rte->ident[0] == 0)
    StrPrintF(rte->ident, "ROUTE-%03u", rte->id);

  DbDelete(rte->ident);

  i = FindRec(1, rte->ident, 0, false, 0);

  if (i >= 0)
    return UpdateRoute(GetRecIndex(1, i), rte);

  return NewRoute(rte);
}

int AddRoutePoint(RouteType *rte, WaypointType *point)
{
  Int16 i, r;
  DmOpenRef dbRef;
  RoutePointType rpoint;
  Err err;

  progressIndex++;
  ProgressGadgetCallback(NULL, formGadgetDrawCmd, NULL);

  for (i = TAM_NAME-1; i >= 0 && point->name[i] == ' '; i--)
    point->name[i] = 0;

  for (i = TAM_COMMENT-1; i >= 0 && point->comment[i] == ' '; i--)
    point->comment[i] = 0;

  if (point->name[0] == 0 && point->symbol == 0)
    return 0;

  if ((dbRef = DbOpenByName(rte->ident, dmModeReadWrite, &err)) == 0)
    return -1;

  MemSet(&rpoint, sizeof(rpoint), 0);
  StrCopy(rpoint.name, point->name);
  rpoint.symbol = point->symbol;
  rpoint.coord = point->coord;

  r = AppendRoutePoint(dbRef, &rpoint);
  DbClose(dbRef);

  return r;
}

int AddTrack(char *name)
{
  progressIndex++;
  ProgressGadgetCallback(NULL, formGadgetDrawCmd, NULL);

  if (storedLog) {
    CloseLog(storedLog);
    storedLog = NULL;
    MapSetStoredLog(storedLog);
  }

  if (importedLog) {
    CloseLog(importedLog);
    importedLog = NULL;
  }

  importedLog = OpenLog(name, AppID, LogType, fileModeReadWrite);

  return 0;
}

int AddTrackLog(TracklogType *log)
{
  progressIndex++;
  ProgressGadgetCallback(NULL, formGadgetDrawCmd, NULL);

  if (importedLog) {
    log->t += garmin_t0;
    WriteLog(importedLog, log, sizeof(TracklogType));
  }

  return 0;
}

int SetCurrentTrack(void)
{
  progressIndex++;
  ProgressGadgetCallback(NULL, formGadgetDrawCmd, NULL);

  if (currentLog) {
    CloseLog(currentLog);
    currentLog = NULL;
  }
  currentLog = OpenLog(LogName, AppID, LogType, fileModeReadWrite);
  MapSetCurrentLog(currentLog);

  return 0;
}

int AddCurrentTrackLog(TracklogType *log)
{
  progressIndex++;
  ProgressGadgetCallback(NULL, formGadgetDrawCmd, NULL);
    
  if (currentLog) {
    log->t += garmin_t0;
    WriteLog(currentLog, log, sizeof(TracklogType));
  }
    
  return 0;
}

int EndTransfer(void)
{
  AppPrefs *prefs;
  UInt16 form;

  SetWait(1);
  prefs = GetPrefs();

  if (FrmGetActiveFormID() == ProgressForm)
    PopForm();

  form = FrmGetActiveFormID();
  progressIndex = 0;
  progressTotal = 0;

  if (newLog) {
    CloseLog(newLog);
    newLog = NULL;
  }

  if (importedLog) {
    CloseLog(importedLog);
    importedLog = NULL;
  }

  BuildRecList(0, ptRef, getpointname, GetPointComment, getpointdata, NULL, comparename);
  if (GetRecNum(0) == 1) SetRecSelection(0, 0);

  BuildRecList(1, rtRef, GetRouteName, NULL, NULL, NULL, comparename);
  if (GetRecNum(1) == 1) SetRecSelection(1, 0);

  TrackFillList(&tracks);

  if (form == WaypointsForm || form == RoutesForm)
    ObjectRefresh(FrmGetActiveForm());
  else if (form == TracksForm)
    TrackRefresh(FrmGetActiveForm());

  if (currentLog)
    SeekLog(currentLog, 0);

  if (storedLog) {
    CloseLog(storedLog);
    storedLog = NULL;
  }

  if (tracks.n)
    storedLog = OpenLog(tracks.fname[prefs->track], AppID,
                        LogType, fileModeReadOnly);

  MapSetStoredLog(prefs->showtrack ? storedLog : NULL);
  MapInvalid();

  if (form == TracksForm)
    SetRecords();

  return 0;
}

int BeginExport(int n)
{
  progressTitle = "Sending";
  progressIndex = 0;
  progressTotal = n;
  SetWait(25);
  MenuEvent(ProgressForm);
  return 0;
}

void SavePoint(char *name, UInt16 symbol, double lat, double lon)
{
  AppPrefs *prefs;

  prefs = GetPrefs();
  MemSet(&savepoint, sizeof(point), 0);

  StrPrintF(savepoint.name, "P%05d", prefs->autopoint);
  StrNCopy(savepoint.comment, name, TAM_COMMENT-1);
  savepoint.symbol = symbol;
  savepoint.coord.latitude = lat;
  savepoint.coord.longitude = lon;

  FrmPopupForm(SavePointForm);
}

Boolean GpsFormHandleEvent(EventPtr event)
{
  FormPtr frm;
  ListPtr lst;
  Boolean handled;
  UInt16 index, numProtocols;
  static AppPrefs *prefs;
  static char *protocolNames[5] = {STR_ZODIAC, STR_NMEA, STR_GARMIN, STR_GARMINHOST, STR_GPSLIB};
  handled = false;

  switch (event->eType) {
    case frmOpenEvent:
      prefs = LoadPrefs();
      frm = FrmGetActiveForm();

      index = prefs->serial_port;
      lst = (ListPtr)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, deviceList));
      LstSetHeight(lst, numDevices);
      LstSetListChoices(lst, deviceNames, numDevices);
      LstSetSelection(lst, index);
      CtlSetLabel((ControlPtr)FrmGetObjectPtr(frm,
         FrmGetObjectIndex(frm, deviceCtl)), LstGetSelectionText(lst, index));

      index = prefs->serial_baud;
      lst = (ListPtr)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, baudList));
      LstSetSelection(lst, index);
      CtlSetLabel((ControlPtr)FrmGetObjectPtr(frm,
         FrmGetObjectIndex(frm, baudCtl)), LstGetSelectionText(lst, index));

      index = prefs->protocol;
      lst = (ListPtr)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, protocolList));
      numProtocols = gpsLib ? 5 : 4;
      LstSetHeight(lst, numProtocols);
      LstSetListChoices(lst, protocolNames, numProtocols);
      LstSetSelection(lst, index);
      CtlSetLabel((ControlPtr)FrmGetObjectPtr(frm,
         FrmGetObjectIndex(frm, protocolCtl)), LstGetSelectionText(lst, index));

      FrmDrawForm(frm);
      handled = true;
      break;

    case nilEvent:
      idle();
      handled = true;
      break;

    case ctlSelectEvent:
      switch (event->data.ctlSelect.controlID) {
        case okBtn:
          SavePrefs();
          // fall-trough

        case cancelBtn:
          PopForm();
          handled = true;
      }
      break;

    case popSelectEvent:
      switch (event->data.popSelect.listID) {
        case deviceList:
          prefs->serial_port = event->data.popSelect.selection;
          break;
        case baudList:
          prefs->serial_baud = event->data.popSelect.selection;
          break;
        case protocolList:
          prefs->protocol = event->data.popSelect.selection;
      }
      handled = false;
      break;

    default:
      break;
  }

  return handled;
}

Boolean UnitsFormHandleEvent(EventPtr event)
{
  FormPtr frm;
  ListPtr lst;
  FieldPtr fld;
  Boolean handled;
  char buf[32];
  UInt16 index;
  double speed;
  static AppPrefs *prefs;

  handled = false;

  switch (event->eType) {
    case frmOpenEvent:
      prefs = LoadPrefs();
      frm = FrmGetActiveForm();
      if (prefs->unit_system == UNIT_METRIC)
        FrmSetControlValue(frm, FrmGetObjectIndex(frm, metricSel), 1);
      else
        FrmSetControlValue(frm, FrmGetObjectIndex(frm, statuteSel), 1);

      FrmSetControlValue(frm, FrmGetObjectIndex(frm, speedCtl),
        prefs->speed_warning);
      speed = prefs->speed_limit;
      if (prefs->unit_system == UNIT_METRIC)
        speed *= 3.6;
      else
        speed *= (3.6 * KM_TO_MILE);
      fStrPrintF(buf, speed, 4, 1);
      FldInsertStr(frm, speedFld, buf);
      FrmCopyLabel(frm, unitLbl,
         prefs->unit_system == UNIT_METRIC ? "km/h" : "mi/h");

      //FrmSetControlValue(frm, FrmGetObjectIndex(frm, distanceCtl),
        //prefs->proximity_alarm);
      //distance = prefs->proximity_limit;
      //if (prefs->unit_system == UNIT_METRIC)
        //distance *= 0.001;
      //else
        //distance *= (0.001 * KM_TO_MILE);
      //fStrPrintF(buf, distance, 4, 3);
      //FldInsertStr(frm, distFld, buf);
      //FrmCopyLabel(frm, distanceLbl,
         //prefs->unit_system == UNIT_METRIC ? "km" : "mi");

      FrmSetControlValue(frm, FrmGetObjectIndex(frm, beepCtl), prefs->log_beep);
      FrmSetControlValue(frm,
        FrmGetObjectIndex(frm, densityCtl+prefs->density), true);

      index = DatumGetIndex(prefs->datum);
      lst = (ListPtr)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, datumList));
      LstSetHeight(lst, num_datums > 10 ? 10 : num_datums);
      LstSetListChoices(lst, datum_names, num_datums);
      LstSetSelection(lst, index);
      CtlSetLabel((ControlPtr)FrmGetObjectPtr(frm,
         FrmGetObjectIndex(frm, datumCtl)), LstGetSelectionText(lst, index));

      FrmDrawForm(frm);
      handled = true;
      break;

    case nilEvent:
      idle();
      handled = true;
      break;

    case ctlSelectEvent:
      switch (event->data.ctlSelect.controlID) {
        case okBtn:
          handled = true;

          frm = FrmGetActiveForm();
          fld = (FieldPtr)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm,speedFld));
          if (!GetFloat(fld, &prefs->speed_limit, 0.0, 500.0)) {
            InfoDialog(INFO, "Invalid speed");
            break;
          }
          if (prefs->unit_system == UNIT_METRIC)
            prefs->speed_limit /= 3.6;
          else
            prefs->speed_limit /= (3.6 * KM_TO_MILE);

          //fld = (FieldPtr)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, distFld));
          //if (!GetFloat(fld, &prefs->proximity_limit, 0.0, 50.0)) {
            //InfoDialog(INFO, "Invalid distance");
            //break;
          //}
          //if (prefs->unit_system == UNIT_METRIC)
            //prefs->proximity_limit /= 0.001;
          //else
            //prefs->proximity_limit /= (0.001 * KM_TO_MILE);

          if (storedLog) {
            CloseLog(storedLog);
            storedLog = NULL;
          }   
          if (tracks.n)  
            storedLog = OpenLog(tracks.fname[prefs->track], AppID,
                                LogType, fileModeReadOnly);
          MapSetStoredLog(prefs->showtrack ? storedLog : NULL);
          MapInvalid();
          SavePrefs();
          PopForm();
          if (FrmGetActiveFormID() == MapForm) {
            MapDrawScale(zoom_factor[current_zoom], hscreenx, hscreeny);
            MapDraw();
          }
          handled = true;
          break;

        case cancelBtn:
          PopForm();
          handled = true;
          break;

        case metricSel:
          prefs->unit_system = UNIT_METRIC;
          frm = FrmGetActiveForm();
          fStrPrintF(buf, prefs->speed_limit * 3.6, 4, 1);
          FldInsertStr(frm, speedFld, buf);
          FrmCopyLabel(frm, unitLbl, "km/h");
          //fStrPrintF(buf, prefs->proximity_limit * 0.001, 4, 3);
          //FldInsertStr(frm, distFld, buf);
          //FrmCopyLabel(frm, distanceLbl, "km");
          handled = true;
          break;

        case statuteSel:
          prefs->unit_system = UNIT_STATUTE;
          frm = FrmGetActiveForm();
          fStrPrintF(buf, prefs->speed_limit * (3.6 * KM_TO_MILE), 4, 1);
          FldInsertStr(frm, speedFld, buf);
          FrmCopyLabel(frm, unitLbl, "mi/h");
          //fStrPrintF(buf, prefs->proximity_limit * (0.001 * KM_TO_MILE), 4, 3);
          //FldInsertStr(frm, distFld, buf);
          //FrmCopyLabel(frm, distanceLbl, "miles");
          handled = true;
          break;

        case speedCtl: 
          prefs->speed_warning = !prefs->speed_warning;
          handled = true;
          break;

        case distanceCtl: 
          prefs->proximity_alarm = !prefs->proximity_alarm;
          handled = true;
          break;   

        case beepCtl:
          prefs->log_beep = !prefs->log_beep;
          handled = true;
          break;   
          
        case densityCtl:
          prefs->density = 0;
          handled = true;
          break;
          
        case density1Ctl:
          prefs->density = 1;
          handled = true;
          break;   
  
        case density2Ctl:
          prefs->density = 2;
          handled = true;
      }
      break;

    case popSelectEvent:
      switch (event->data.popSelect.listID) {
        case datumList:
          prefs->datum = DatumGetID(event->data.popSelect.selection);
      }
      handled = false;
      break;

    default:
      break;
  }

  return handled;
}

static void AlignTracksControls(FormPtr frm) {
  FrmObjectBottomAlign(frm, FrmGetObjectIndex(frm, deleteBtn), -1);
  FrmObjectBottomAlign(frm, FrmGetObjectIndex(frm, renameBtn), -1);
  FrmObjectBottomAlign(frm, FrmGetObjectIndex(frm, thinBtn), -1);
}

Boolean TracksListHandleEvent(EventPtr event)
{
  FormPtr frm;
  TableType *tbl;
  char *name;
  LocalID dbID;
  Int32 size;
  UInt16 id, n;
  AppPrefs *prefs;
  Boolean handled;

  handled = false;

  switch (event->eType) {
    case frmOpenEvent:
      prefs = GetPrefs();
      frm = FrmGetActiveForm();
      resizeForm(frm);
  
      AlignTracksControls(frm);
      AlignUpperGadgets(frm);
      FrmSetGadgetHandler(frm, FrmGetObjectIndex(frm, logCtl),
          LogGadgetCallback);
      FrmSetGadgetHandler(frm, FrmGetObjectIndex(frm, statusCtl),
          StatusGadgetCallback);

      TrackFillList(&tracks);

      if (storedLog) {
        CloseLog(storedLog);
        storedLog = NULL;
      }
      if (tracks.n)
        storedLog = OpenLog(tracks.fname[prefs->track], AppID,
                            LogType, fileModeReadOnly);

      MapSetStoredLog(prefs->showtrack ? storedLog : NULL);

      FrmDrawForm(frm);
      TrackRefresh(frm);
      SetRecords();

      handled = true;
      break;

    case frmUpdateEvent:
      frm = FrmGetActiveForm();
      FrmDrawForm(frm);
      TrackRefresh(frm);
      SetRecords();
      handled = true;
      break;

    case nilEvent:
      idle();
      handled = true;
      break;

    case menuEvent:
      MenuEvent(event->data.menu.itemID);
      handled = true;
      break;

    case ctlSelectEvent:
      name = NULL;
      switch (id = event->data.ctlSelect.controlID) {
        case saveresetBtn:
          frm = FrmGetActiveForm();

          size = LogSize(currentLog);
          if (size > 0)
            MapInvalid();

          CloseLog(currentLog);

          if (size > 0) {
            if ((dbID = DmFindDatabase(0, LogName)) != 0) {
              if ((name = GetString("New track name", NULL)) != NULL) {
                if (DmFindDatabase(0, name) != 0)
                  InfoDialog(INFO, "Duplicate database name");
                else {
                  DmSetDatabaseInfo(0, dbID, name, NULL, NULL, NULL, NULL, NULL,
                                    NULL, NULL, NULL, NULL, NULL);
                  TrackFillList(&tracks);
                  TrackRefresh(frm);
                }
              }
            }
          } else
            DeleteLog(LogName);

          currentLog = OpenLog(LogName, AppID, LogType, fileModeUpdate);
          MapSetCurrentLog(currentLog);
          SetRecords();
          DrawFld(frm, recFld);

          if (tracks.n) { 
            prefs = GetPrefs();
            if (storedLog) {
              CloseLog(storedLog);
              storedLog = NULL;
            }
            storedLog = OpenLog(tracks.fname[prefs->track], AppID,
                                LogType, fileModeReadOnly);
            MapSetStoredLog(prefs->showtrack ? storedLog : NULL);
          }

          handled = true;
          break;

        case resetBtn:
          frm = FrmGetActiveForm();

          if (LogSize(currentLog) > 0) {
            MapInvalid();
          }

          CloseLog(currentLog);
          DeleteLog(LogName);

          currentLog = OpenLog(LogName, AppID, LogType, fileModeUpdate);
          MapSetCurrentLog(currentLog);
          SetRecords();
          DrawFld(frm, recFld);

          handled = true;
          break;

        case renameBtn:
            if (tracks.n == 0) {
              handled = true;
              break;
            }
            prefs = GetPrefs();
            if ((name = GetString("New track name",
                           tracks.fname[prefs->track])) == NULL) {
              handled = true;
              break;
            }
            if (DmFindDatabase(0, name) != 0) {
              InfoDialog(INFO, "Duplicate database name");
              handled = true;
              break;
            }
            // fall-through

        case deleteBtn:
          if (id == deleteBtn && FrmAlert(DeleteAlert) != 0) {
            handled = true;
            break;
          }

          if (tracks.n > 0) {
            if (storedLog) {
              CloseLog(storedLog);
              storedLog = NULL;
              MapSetStoredLog(storedLog);
            }

            frm = FrmGetActiveForm();
            prefs = GetPrefs();

            if (prefs->track < tracks.n) {
              if (id == deleteBtn)
                DeleteLog(tracks.fname[prefs->track]);
              else
                RenameLog(tracks.fname[prefs->track], name);
            }

            TrackFillList(&tracks);

            if (tracks.n) {
              storedLog = OpenLog(tracks.fname[prefs->track], AppID,
                                  LogType, fileModeReadOnly);
              MapSetStoredLog(prefs->showtrack ? storedLog : NULL);
            }

            TrackRefresh(frm);
            MapInvalid();
          }
          handled = true;
          break;

        case thinBtn:
          if (tracks.n > 0 && storedLog) {
            prefs = GetPrefs();
            if ((name = GetString("New track name",
                           tracks.fname[prefs->track])) != NULL) {
              if (DmFindDatabase(0, name) != 0)
                InfoDialog(INFO, "Duplicate database name");
              else if ((newLog = OpenLog(name, AppID, LogType,
                          fileModeUpdate)) != NULL) {
                if ((n = StartThinLog(storedLog, newLog)) > 0) {
                  progressTitle = "Reducing track";
                  progressIndex = 0;
                  progressTotal = n;
                  SetWait(25);
                  MenuEvent(ProgressForm);
                } else {
                  CloseLog(newLog);
                  newLog = NULL;
                }
              }
            }
          }
          handled = true;
      }
      break;

    case sclRepeatEvent:
    case sclExitEvent: 
      TrackSetTop(event->data.sclRepeat.newValue);
      TrackRefresh(FrmGetActiveForm());
      break;

    case tblEnterEvent:
      tbl = event->data.tblSelect.pTable;

      if (TblGetRowData(tbl, event->data.tblSelect.row) < tracks.n) {
        UInt16 tmp = TblGetRowData(tbl, event->data.tblSelect.row);

        prefs = GetPrefs();

        if (tmp != prefs->track) {
          UInt16 top = TrackGetTop();

          TblMarkRowInvalid(tbl, event->data.tblSelect.row);
          if (prefs->track >= top && prefs->track < top+trackRows)
            TblMarkRowInvalid(tbl, prefs->track-top);

          prefs->track = tmp;
          if (storedLog) {
            CloseLog(storedLog);
            storedLog = NULL;
          }
          if (tracks.n)
            storedLog = OpenLog(tracks.fname[prefs->track], AppID,
                              LogType, fileModeReadOnly);
          MapSetStoredLog(prefs->showtrack ? storedLog : NULL);
          MapInvalid();

          TblRedrawTable(tbl);
        }
      }
      handled = true; 
      break;

    default:
      break;
  }

  return handled;
}

UInt16 GetTrackNum(void)
{
  return tracks.n;
}

char *GetTrackName(UInt16 i)
{
  return tracks.fname[i];
}

UInt32 GetTrackSize(UInt16 i)
{
  return tracks.rec[i].size;
}

void SelectTrack(UInt16 i)
{
  AppPrefs *prefs;

  prefs = GetPrefs();
  prefs->track = i;
  prefs->showtrack = 1;

  if (storedLog) {
    CloseLog(storedLog);
    storedLog = NULL;
  }
  storedLog = OpenLog(tracks.fname[prefs->track], AppID,
                      LogType, fileModeReadOnly);
  MapSetStoredLog(storedLog);
  MapInvalid();
}

static void SetRecords(void)
{
  Int32 size;
  UInt16 n;
  char buf[16];
  FormPtr frm = FrmGetActiveForm();
  
  if (currentLog && FrmGetWindowHandle(frm) == WinGetActiveWindow()) {
    size = LogSize(currentLog);
    n = size / sizeof(TracklogType);
    StrPrintF(buf, "%u", n);
    FldInsertStr(frm, recFld, buf);
  }
}

Boolean EditPointFormHandleEvent(EventPtr event)
{
  static CoordType coord0;
  FormPtr frm;
  UInt16 form, index;
  FieldPtr fld;
  GraphicControlType *ctl;
  char buf[32], *s;
  Err err;
  WaypointType *p;
  Boolean handled, dirty;
  AppPrefs *prefs;

  handled = false;

  frm = FrmGetActiveForm();
  form = FrmGetActiveFormID();

  prefs = GetPrefs();
  DatumFromWGS84(DatumGetIndex(prefs->datum));

  switch (event->eType) {
    case frmOpenEvent:
      MemSet(&point, sizeof(point), 0);
      FrmCopyLabel(frm, unitLbl, prefs->unit_system == UNIT_METRIC ? "km" : "miles");

      if (form == NewPointForm || form == AutoPointForm) {
        if (form == AutoPointForm) {
          StrPrintF(buf, "P%05d", prefs->autopoint);
          FldInsertStr(frm, nameFld, buf);
          fld = (FieldPtr)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, nameFld));
          FldSetSelection(fld, 0, 6);
        }
        if (gpsOnline && last_validity) {
          coord0 = prefs->coord;
          GetCoord(frm, &prefs->coord);
          point.date = ts;
        } else {
          GetUTC(&point.date.day, &point.date.month, &point.date.year, &point.date.hour, &point.date.minute, &point.date.second);
        }
        point.symbol = sym_wpt_dot;
        FormatDateTime(buf, point.date.day, point.date.month, point.date.year, point.date.hour, point.date.minute, point.date.second);
        FldInsertStr(frm, utcFld, buf);

      } else if (form == SavePointForm) {
        MemMove(&point, &savepoint, sizeof(point));

        FldInsertStr(frm, nameFld, point.name);
        fld = (FieldPtr)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, nameFld));
        FldSetSelection(fld, 0, 6);

        FldInsertStr(frm, commentFld, point.comment);
        fld = (FieldPtr)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm,commentFld));
        FldSetInsertionPoint(fld, 0);
        FldSetInsPtPosition(fld, 0);

        coord0 = point.coord;
        GetCoord(frm, &point.coord);

        GetUTC(&point.date.day, &point.date.month, &point.date.year, &point.date.hour, &point.date.minute, &point.date.second);
        FormatDateTime(buf, point.date.day, point.date.month, point.date.year, point.date.hour, point.date.minute, point.date.second);
        FldInsertStr(frm, utcFld, buf);

      } else {
        index = GetRecIndex(0, GetRecSelection(0));
        p = (WaypointType *)DbOpenRec(ptRef, index, &err);
        if (p && !err) {
          FldInsertStr(frm, nameFld, p->name);
          FldInsertStr(frm, commentFld, p->comment);
          fld = (FieldPtr)FrmGetObjectPtr(frm,
              FrmGetObjectIndex(frm, commentFld));
          FldSetInsertionPoint(fld, 0);
          FldSetInsPtPosition(fld, 0);
          coord0 = p->coord;
          GetCoord(frm, &p->coord);
          FormatDateTime(buf, p->date.day, p->date.month, p->date.year,
                         p->date.hour, p->date.minute, p->date.second);
          FldInsertStr(frm, utcFld, buf);
          MemMove(&point, p, sizeof(point));
          DbCloseRec(ptRef, index, (char *)p);
        }
      }
      FrmSetFocus(frm, FrmGetObjectIndex(frm, nameFld));

      ctl = (GraphicControlType *)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, symbolCtl));
      CtlSetGraphics((ControlType *)ctl, symbolId + GetSymbolIndex(point.symbol), 0);
      FldInsertStr(frm, datumFld, DatumGetName(DatumGetIndex(prefs->datum)));

      FrmDrawForm(frm);
      handled = true;
      break;

    case nilEvent:
      idle();
      handled = true;
      break;

    case keyDownEvent:
      if (event->data.keyDown.chr != escapeChr)
        break;
      event->data.ctlSelect.controlID = okBtn;
      // fall-through

    case ctlSelectEvent:
      switch (event->data.ctlSelect.controlID) {
        case getBtn:
          if (gpsOnline && last_validity) {
            coord0 = prefs->coord;
            GetCoord(frm, &prefs->coord);
            point.date = ts;
            FormatDateTime(buf,
                           point.date.day, point.date.month, point.date.year,
                           point.date.hour, point.date.minute,
                           point.date.second);
            FldInsertStr(frm, utcFld, buf);
          } else
            InfoDialog(INFO, "There is no fix");
          break;

        case symbolCtl:
          MenuEvent(SymbolForm);
          handled = true;
          break;

        case okBtn:
          handled = true;

          fld = (FieldPtr)FrmGetObjectPtr(frm,
            FrmGetObjectIndex(frm, nameFld));
          s = FldGetTextPtr(fld);
          if (!s || !s[0]) {
            InfoDialog(INFO, "Invalid name");
            break;
          }
          if ((form == NewPointForm || form == AutoPointForm ||
               form == SavePointForm) && FindRec(0, s, 0, false, 0) == 0) {
            InfoDialog(INFO, "Duplicate name");
            break;
          }
          StrNCopy(point.name, s, TAM_NAME-1);
          point.name[StrLen(s)] = 0;

          fld = (FieldPtr)FrmGetObjectPtr(frm,
            FrmGetObjectIndex(frm, commentFld));
          s = FldGetTextPtr(fld);
          if (s && s[0]) {
            StrNCopy(point.comment, s, TAM_COMMENT-1);
            point.comment[StrLen(s)] = 0;
          } else
            point.comment[0] = 0;

          dirty = false;

          fld = (FieldPtr)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, slatFld));
          if (!GetFloat(fld, &point.coord.latitude, -90.0, 90.0)) {
            InfoDialog(INFO, "Invalid latitude");
            break;
          }
          if (FldDirty(fld))
            dirty = true;

          fld = (FieldPtr)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm,slongFld));
          if (!GetFloat(fld, &point.coord.longitude, -180.0,180.0)) {
            InfoDialog(INFO, "Invalid longitude");
            break;
          }
          if (FldDirty(fld))
            dirty = true;

          fld = (FieldPtr)FrmGetObjectPtr(frm,FrmGetObjectIndex(frm,heightFld));
          if (!GetFloat(fld, &point.coord.height, -9999.0,9999.0)) {
            InfoDialog(INFO, "Invalid elevation");
            break;
          }
          if (prefs->unit_system == UNIT_METRIC)
            point.coord.height *= 1000.0;
          else
            point.coord.height = (point.coord.height / KM_TO_MILE) * 1000.0;
          if (FldDirty(fld))
            dirty = true;

          if (dirty) {
            DatumToWGS84(DatumGetIndex(prefs->datum));
            DatumConvert(&point.coord.longitude, &point.coord.latitude,
                         &point.coord.height);
          } else {
            point.coord.latitude = coord0.latitude;
            point.coord.longitude = coord0.longitude;
            point.coord.height = coord0.height;
          }

          if (form == NewPointForm || form == AutoPointForm ||
              form == SavePointForm)
            NewPoint(ptRef, &point);
          else {
            index = GetRecIndex(0, GetRecSelection(0));
            UpdatePoint(ptRef, index, &point);
          }

          MapInvalid();

          if (form == AutoPointForm || form == SavePointForm)
            prefs->autopoint++;

          PopForm();
          BuildRecList(0, ptRef, getpointname, GetPointComment, getpointdata, NULL, comparename);

          if (FrmGetActiveFormID() == WaypointsForm)
            ObjectRefresh(FrmGetActiveForm());

          handled = true;
          break;

        case cancelBtn:
          PopForm();
          handled = true;
      }
      break;

    default:
      break;
  }

  return handled;
}

UInt16 GetSelectedSymbol(void)
{
  return point.symbol;
}

void SetSelectedSymbol(UInt16 type)
{
  point.symbol = type;
}

Boolean CompassFormHandleEvent(EventPtr event)
{
  FormPtr frm;
  UInt16 form, index;
  UInt32 hr;
  WaypointType *p;
  RouteType *rte;
  RoutePointType *rp;
  DmOpenRef dbRef;
  AppPrefs *prefs;
  Boolean handled;
  Err err;
  static char name[32];

  handled = false;

  prefs = GetPrefs();
  switch (event->eType) {
    case frmOpenEvent:
      frm = FrmGetActiveForm();
      resizeForm(frm);
      form = FrmGetActiveFormID();

      MemSet(name, sizeof(name), 0);

      if (form == SeekPointForm) {
        index = GetRecIndex(0, GetRecSelection(0));
        p = (WaypointType *)DbOpenRec(ptRef, index, &err);
        if (p && !err) {
          StrNCopy(name, p->name, TAM_NAME-1);
          target = p->coord;
          DbCloseRec(ptRef, index, (char *)p);
        }
        FrmCopyLabel(frm, turnLbl, "\005");

      } else if (form == FollowRouteForm) {
        if (currentSeq == (UInt16)-1)
          RouteGetFirst(&currentSeq, &currentIndex);

        index = GetRecIndex(1, GetRecSelection(1));
        rte = (RouteType *)DbOpenRec(rtRef, index, &err);

        if (rte) {
          dbRef = DbOpenByName(rte->ident, dmModeReadOnly, &err);

          if (dbRef) {
            rp = (RoutePointType *)DbOpenRec(dbRef, currentIndex, &err);
            if (rp) {
              StrNCopy(name, rp->name, TAM_NAME-1);
              target = rp->coord;
              DbCloseRec(dbRef, currentIndex, (char *)rp);
            }
            DbClose(dbRef);
          }
          DbCloseRec(rtRef, index, (char *)rte);
        }
        FrmCopyLabel(frm, turnLbl, "\005");
      }

      FrmSetGadgetHandler(frm, FrmGetObjectIndex(frm, logCtl),
          LogGadgetCallback);
      FrmSetGadgetHandler(frm, FrmGetObjectIndex(frm, statusCtl),
          StatusGadgetCallback);
      FrmSetGadgetHandler(frm, FrmGetObjectIndex(frm, nearestCtl),
          NearestGadgetCallback);
      drawn[0] = drawn[1] = drawn[2] = 0xFFFF;
      AlignUpperGadgets(frm);
      FrmDrawForm(frm);

      DrawGotoPoint(name, frm);

      SetLocalTime();
      SetSpeedUI(last_speed);
      SetEPE(last_ehpe, last_evpe);
      SetHeight(prefs->coord.height);

      hr = (hscreenx < hscreeny) ? hscreenx : hscreeny;
      h_dir = hr * H_DIR / 160;

      if (form == CompassForm) {
        x_dir = hscreenx * (X_DIR+20) / 160;
        y_dir = hscreeny * (Y_DIR-4)  / 160;
        r_dir = hr * (R_DIR+10) / 160;
      } else {
        x_dir = hscreenx * X_DIR / 160;
        y_dir = hscreeny * Y_DIR / 160;
        r_dir = hr * R_DIR / 160;
      }

      if (highDensity) WinSetCoordinateSystem(kCoordinatesDouble);
      DrawCompass(x_dir, y_dir, r_dir, LABEL_N|LABEL_S|LABEL_E|LABEL_W);
      if (highDensity) WinSetCoordinateSystem(kCoordinatesStandard);

      if (form == SeekPointForm)
        gotoActive = 1;
      else if (form == FollowRouteForm)
        gotoActive = 2;
      handled = true;
      break;

    case frmUpdateEvent:
      frm = FrmGetActiveForm();
      FrmDrawForm(frm);
      DrawGotoPoint(name, frm);
      if (highDensity) WinSetCoordinateSystem(kCoordinatesDouble);
      DrawCompass(x_dir, y_dir, r_dir, LABEL_N|LABEL_S|LABEL_E|LABEL_W);
      if (highDensity) WinSetCoordinateSystem(kCoordinatesStandard);
      handled = true;
      break;

    case nilEvent:
      idle();
      handled = true;
      break;

    case menuEvent:
      MenuEvent(event->data.menu.itemID);
      handled = true;
      break;

    default:
      break;
  }

  return handled;
}

static void DrawGotoPoint(char *name, FormPtr frm)
{
  UInt16 index, old, dx;
  RectangleType rect;

  if (name && name[0]) {
    old = FntSetFont(stdFont);
    dx = FntCharsWidth(name, StrLen(name));
    FntSetFont(old);

    index = FrmGetObjectIndex(frm, targetFld);
    FrmGetObjectBounds(frm, index, &rect);

    if (dx < rect.extent.x) {
      rect.topLeft.x = screenx - dx;
      rect.extent.x = dx;
      FrmSetObjectBounds(frm, index, &rect);
    }
    FldInsertStr(frm, targetFld, name);
  }
}

Boolean FindFormHandleEvent(EventPtr event)
{
  FormPtr frm;
  FieldPtr fld;
  UInt16 id, index;
  Int16 i;
  char *s;
  AppPrefs *prefs;
  Boolean handled;

  handled = false;

  prefs = GetPrefs();
  switch (event->eType) {
    case frmOpenEvent:
      frm = FrmGetActiveForm();
      FrmSetControlValue(frm, FrmGetObjectIndex(frm, matchCtl),
         prefs->exact);
      FrmSetControlValue(frm, FrmGetObjectIndex(frm, caseCtl),
         !prefs->caseless);
      index = FrmGetObjectIndex(frm, nameFld);
      FrmSetFocus(frm, index);
      FrmDrawForm(frm);
      FldInsertStr(frm, nameFld, findbuf);
      fld = FrmGetObjectPtr(frm, index);
      FldSetSelection(fld, 0, StrLen(findbuf));
      handled = true;
      break;

    case nilEvent:
      idle();
      handled = true;
      break;   
  
    case ctlSelectEvent:
      switch (id = event->data.ctlSelect.controlID) {
        case findBtn:
        case findnextBtn:
          frm = FrmGetActiveForm();
          index = FrmGetObjectIndex(frm, nameFld);
          fld = FrmGetObjectPtr(frm, index);
          if ((s = FldGetTextPtr(fld)) != NULL && s[0]) {
            StrNCopy(findbuf, s, sizeof(findbuf)-1);
            i = FindRec(0, s, (id == findBtn) ? 0 : GetRecSelection(0)+1,
                        prefs->caseless, prefs->exact ? 0 : StrLen(s));
          } else
            i = -1;
          PopForm();
          if (i >= 0) {
            if (FrmGetActiveFormID() == WaypointsForm) {
              SetRecSelection(0, i);
              if (i < objectRows)
                ObjectSetTop(0);
              else
                ObjectSetTop(i-objectRows/2);
              ObjectRefresh(FrmGetActiveForm());
            }
          }
          handled = true;
          break;
        case caseCtl:
          prefs->caseless = !prefs->caseless;
          handled = true;
          break;
        case matchCtl:
          prefs->exact = !prefs->exact;
          handled = true;
      }
      break;

    default:
      break;
  }

  return handled;
}

Boolean GadgetCallback(UInt16 id, UInt16 cmd, void *param)
{
  EventType *event;
  FormPtr frm;
  RectangleType rect;
  UInt16 form, index, symbol, bmpId, tc;
  UInt16 x, y, width, i, len, dy, dx1, dx2;
  Int32 tmp;
  WaypointType *p;
  RouteType *rte;
  RoutePointType *rp;
  DmOpenRef dbRef;
  AppPrefs *prefs;
  char buf[32], name[TAM_NAME];
  Err err;

  if (cmd == formGadgetDeleteCmd)
    return true;

  frm = FrmGetActiveForm();
  form = FrmGetActiveFormID();

  index = FrmGetObjectIndex(frm, id);
  FrmGetObjectBounds(frm, index, &rect);
  x = rect.topLeft.x;
  y = rect.topLeft.y;
  width = rect.extent.x;
  
  switch (cmd) {
    case formGadgetDrawCmd:
      switch (id) {
        case statusCtl:
          WinEraseRectangle(&rect, 0);
          tc = WinSetTextColor(UIColorGetTableEntryIndex(UIDialogFrame));
          if (!gpsOnline)
            DrawBmp(offlineBmp, x, y, false);
          else if (!last_validity) {
            if (handshake)
              WinDrawChars("ok", 2, x, y);
            else
              DrawBmp(onlineBmp, x, y, false);
          } else switch (last_solution) {
              case 0:
              case 1:
              case 2:  
                DrawBmp(onlineBmp, x, y, false);
                break;
              case 3:
                WinDrawChars("2D", 2, x, y);
                break;
              default:
                WinDrawChars("3D", 2, x, y);
          }
          WinSetTextColor(tc);
          break;
        case logCtl:
          WinEraseRectangle(&rect, 0);
          prefs = GetPrefs();
          DrawBmp(prefs->log_enabled ? logBmp : notlogBmp, x, y, false);
          break;
        case centerCtl:
          DrawBmp(centerBmp, x, y, false);
          break;
        case zoominCtl:
          DrawBmp(zoominBmp, x, y, false);
          break;
        case zoomoutCtl:
          DrawBmp(zoomoutBmp, x, y, false);
          break;
        case lockCtl:
          WinEraseRectangle(&rect, 0);
          prefs = GetPrefs();
          DrawBmp(prefs->locked ? lockedBmp : unlockedBmp, x, y, false);
          break;
        case symbolTbl:
          DrawSymbolTable(&rect, firstSymbol);
          break;
        case nearestCtl:
          dbRef = 0;

          if (form == FollowRouteForm) {
            index = GetRecIndex(1, GetRecSelection(1));
            rte = (RouteType *)DbOpenRec(rtRef, index, &err);
            if (rte) {
              dbRef = DbOpenByName(rte->ident, dmModeReadOnly, &err);
              DbCloseRec(rtRef, index, (char *)rte);
            }
          }

          for (i = 0; i < nearnum; i++) {
            if (dbRef) {
              rp = (RoutePointType *)DbOpenRec(dbRef, nearest[i].index, &err);
              if (rp == NULL)
                continue;
              symbol = rp->symbol;
              StrCopy(name, rp->name);
              DbCloseRec(dbRef, nearest[i].index, (char *)rp);
            } else {
              p = (WaypointType *)DbOpenRec(ptRef, nearest[i].index, &err);
              if (p == NULL)
                continue;
              StrCopy(name, p->name);
              symbol = p->symbol;
              DbCloseRec(ptRef, nearest[i].index, (char *)p);
            }

            if (nearest[i].index != drawn[i]) {
              RctSetRectangle(&rect, x, y+i*16, 16, 16);
              WinEraseRectangle(&rect, 0);
              DrawSymbol(symbolId + GetSymbolIndex(symbol), x+8, y+i*16+8, true, NULL, 0, 0, true);
            }

            dy = FntCharHeight();
            len = StrLen(name);
            dx1 = FntCharsWidth(name, len);
            WinDrawChars(name, len, x+20, y+i*16+8-dy/2);

            FormatDistance(buf, nearest[i].distance);
            len = StrLen(buf);
            dx2 = FntCharsWidth(buf, len);
            WinDrawChars(buf, len, x+width-dx2, y+i*16+8-dy/2);

            RctSetRectangle(&rect, x+20+dx1, y+i*16+8-dy/2,
              width-dx2-20-dx1, 16);
            WinEraseRectangle(&rect, 0);
            drawn[i] = nearest[i].index;
          }

          if (dbRef)
            DbClose(dbRef);

          for (; i < 3; i++) {
            RctSetRectangle(&rect, x, y+i*16, width, 16);
            WinEraseRectangle(&rect, 0);
          }
          break;
        case progressCtl:
          WinDrawRectangleFrame(simpleFrame, &rect);
          if (progressIndex) {
            tmp = (progressIndex <= progressTotal) ?
                  progressIndex : progressTotal;
            rect.extent.x = (progressTotal == 0) ? 0 :
                            (tmp * rect.extent.x) / progressTotal;
            WinDrawRectangle(&rect, 0);
          }
      }
      break;
    case formGadgetEraseCmd:
      break;
    case formGadgetHandleEventCmd:
      event = (EventType *)param;
      if (event->eType == frmGadgetEnterEvent) switch (id) {
        case statusCtl:
          SndPlaySystemSound(sndClick);
          WinEraseRectangle(&rect, 0);
          if (gpsOnline) {
            GpsOffline();
            DrawBmp(offlineBmp, x, y, false);
          } else {
            bmpId = GpsOnline() == 0 ? onlineBmp : offlineBmp;
            DrawBmp(bmpId, x, y, false);
          }
          break;
        case logCtl:
          SndPlaySystemSound(sndClick);
          prefs = GetPrefs();
          prefs->log_enabled = !prefs->log_enabled;
          GadgetCallback(logCtl, formGadgetDrawCmd, NULL);
          break;
        case centerCtl:
          Center();
          break;
        case zoominCtl:
          ZoomIn();
          break;
        case zoomoutCtl:
          ZoomOut();
          break;
        case lockCtl:
          prefs = GetPrefs();
          prefs->locked = !prefs->locked;
          SndPlaySystemSound(sndClick);
          GadgetCallback(lockCtl, formGadgetDrawCmd, NULL);
          MapLock(prefs->locked);
          break;
      }
  }

  return true;
}

static void Center(void)
{
  AppPrefs *prefs = GetPrefs();

  if (prefs->locked)
    SndPlaySystemSound(sndError);
  else {
    SndPlaySystemSound(sndClick);
    if (last_validity) {
      MapCenter(prefs->coord.latitude, prefs->coord.longitude);
      MapDraw();
    }   
  }
}

static void ZoomIn(void)
{
  SndPlaySystemSound(sndClick);
  if (current_zoom > 0) {
    current_zoom--;
    MapDrawScale(zoom_factor[current_zoom], hscreenx, hscreeny);
    MapZoomLevel(current_zoom);
    MapZoom(zoom_factor[current_zoom] / DEG_TO_KM,
            zoom_factor[current_zoom] / DEG_TO_KM);
    MapDraw();
  }
}

static void ZoomOut(void)
{
  SndPlaySystemSound(sndClick);
  if (current_zoom < (NUM_ZOOM-1)) {
    current_zoom++;
    MapDrawScale(zoom_factor[current_zoom], hscreenx, hscreeny);
    MapZoomLevel(current_zoom);
    MapZoom(zoom_factor[current_zoom] / DEG_TO_KM,
            zoom_factor[current_zoom] / DEG_TO_KM);
    MapDraw();
  }
}

static void ZoomOffset(Int16 offset)
{
  MapZoomOffset(offset);
  MapZoomLevel(current_zoom);
  MapZoom(zoom_factor[current_zoom] / DEG_TO_KM,
          zoom_factor[current_zoom] / DEG_TO_KM);
}

Boolean ProgressFormHandleEvent(EventPtr event)
{
  FormPtr frm;
  Boolean handled;
  static Int16 count = 0;
  static char buf[64];

  handled = false;
      
  switch (event->eType) { 
    case frmOpenEvent:
      frm = FrmGetActiveForm();
      FrmSetGadgetHandler(frm, FrmGetObjectIndex(frm, progressCtl),
          ProgressGadgetCallback);
      if (newLog)
        StrCopy(buf, progressTitle);
      else
        StrPrintF(buf, "%s %u %s", progressTitle, (UInt16)progressTotal,
          progressTotal <= 1 ? "record" : "records");
      FrmSetTitle(frm, buf);
      FrmDrawForm(frm);
      count = 0;
      handled = true;
      break;

    case nilEvent:
      idle();
      handled = true;
      break;

    case ctlSelectEvent:
      switch (event->data.ctlSelect.controlID) {
        case cancelBtn:
          if (newLog)
            EndTransfer();
          else {
            count++;
            if (count == 1)
              garmin_abort_transfer();
            else
              EndTransfer();
          }
          handled = true;
      }
      break;

    default:
      break;
  }

  return handled;
}

void PopForm(void)
{
  FormPtr frm;
  AppPrefs *prefs;
  GraphicControlType *ctl;
  Err err;

  prefs = GetPrefs();

  switch (FrmGetActiveFormID()) {
    case SymbolForm:
      FrmReturnToForm(pointForm);
      frm = FrmGetActiveForm();
      ctl = (GraphicControlType *)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, symbolCtl));
      CtlSetGraphics((ControlType *)ctl, symbolId + GetSymbolIndex(point.symbol), 0);
      break;

    case MapFindForm:
      // because of Save and Prox ordering
      BuildRecList(0, ptRef, getpointname, GetPointComment, getpointdata, NULL, comparename);
      FrmReturnToForm(prefs->display);
      MapDraw();
      GadgetCallback(statusCtl, formGadgetDrawCmd, NULL);
      GadgetCallback(lockCtl, formGadgetDrawCmd, NULL);
      break;

    case LoadMapForm:
      if (dataRef) {
        MapSetData(0);
        DbClose(dataRef);
        dataRef = 0;
      }

      if (prefs->mapname[0]) {
        dataRef = DbOpenByName(prefs->mapname, dmModeReadOnly, &err);
        if (dataRef == 0)
          prefs->mapname[0] = 0;
        else if (MapSetData(dataRef) != 0) {
          prefs->mapname[0] = 0;
          DbClose(dataRef);
          dataRef = 0;
        }
      }
      MapInvalid();

      FontSet(mapfont[prefs->mapfont]);
      MapFont(mapfont[prefs->mapfont]);
      MapSetStoredLog(prefs->showtrack ? storedLog : NULL);

      // detail to offset: 0 a 4 -> -2 a 2
      ZoomOffset((Int16)prefs->mapdetail - 2);

      FrmReturnToForm(prefs->display);
      GadgetCallback(statusCtl, formGadgetDrawCmd, NULL);

      if (prefs->display == MapForm)
        MapDraw();
      break;

    case MapColorForm:
      FrmReturnToForm(prefs->display);
      if (prefs->display == MapForm)
        MapDraw();
      break;

    case SavePointForm:
      FrmReturnToForm(MapFindForm);
      break;

    case ProgressForm:
      FrmReturnToForm(prefs->display);
      FrmUpdateForm(prefs->display, frmRedrawUpdateCode);
      break;

    default:
      FrmReturnToForm(prefs->display);
      GadgetCallback(statusCtl, formGadgetDrawCmd, NULL);
  }
}

static Err GpsOnline(void)
{
  UInt32 baud;
  UInt16 cardNo;
  LocalID dbID;
  char buf[8];
  AppPrefs *prefs = GetPrefs();

  // Zodiac: 9600 baud, 8 bits, no parity, 1 stop bit
  // NMEA:   4800 baud, 8 bits, no parity, 1 stop bit

  switch (prefs->serial_baud) {
    case 0: baud = 4800; break;
    case 1: baud = 9600; break;
    case 2: baud = 19200; break;
    case 3: baud = 38400; break;
    case 4: baud = 57600; break;
    case 5: baud = 115200; break;
    default: return -1;
  }

  if (prefs->protocol == PROTOCOL_GPSLIB) {
    if (!gpsLib)
      return -1;
    if (SysCurAppDatabase(&cardNo, &dbID) != 0)
      return -1;
    if (GPSOpen(GpsLibRef) != 0)
      return -1;
    if (SysNotifyRegister(cardNo, dbID, sysNotifyGPSDataEvent, GpsLibEventCallback, sysNotifyNormalPriority, &GpsLibRef) != 0) {
      GPSClose(GpsLibRef);
      return -1;
    }
    StrPrintF(buf, "%04x", GPSGetLibAPIVersion(GpsLibRef));
    SetDevice(STR_GARMIN, STR_GPSLIB, buf);
  } else if (prefs->serial_port < numDevices) {
    if (SerialOnline(&gpsRefnum, baud, 8, 0, 1, 0, 0, 0, deviceCreators[prefs->serial_port]) != 0)
      return -1;
  } else {
    InfoDialog(ERROR, "No serial port");
    return -1;
  }

  SetWait(1);
  gpsOnline = true;

  gps_protocol(prefs->protocol);
  SysTaskDelay(SysTicksPerSecond()/5);

  gps_init(prefs->coord.latitude, prefs->coord.longitude,
    prefs->coord.height, 0);
  ts.day = ts.month = ts.year = ts.hour = ts.minute = ts.second = 0;

  last_speed = 0.0;
  last_validity = 0;
  last_time = 0;
  errors = 0;

  last_pvt = last_msg = last_response = TimGetSeconds();
  SatResetChannels(last_chs, last_sat);
  nearnum = 0;

  if (prefs->protocol == PROTOCOL_GARMIN_HOST)
    SetDevice("", "", "");
  else
    SetDevice(STR_UNKNOWN, "", "");

  return 0;
}

static void GpsOffline(void)
{
  AppPrefs *prefs = GetPrefs();
  UInt16 form = FrmGetActiveFormID();

  TripStop();

  if (prefs->protocol == PROTOCOL_GPSLIB) {
    if (gpsLib)
      GPSClose(GpsLibRef);
  } else
    SerialOffline(gpsRefnum);

  SetWait(1);
  gpsOnline = false;

  nearnum = 0;
  last_speed = 0.0;
  last_validity = 0;
  MapPosition(prefs->coord.latitude, prefs->coord.longitude, 0, 0);
  ObjectPositionValid(last_validity);

  if (gotoActive) {
    gotoActive = 0;
    currentIndex = 0;
    currentSeq = -1;
    MapResetTarget();
    MapInvalid();
  }

  if (form == MapForm)
    MapDraw();

  last_visible = 0;
  SatResetChannels(last_chs, last_sat);

  if (form == SatForm) {
    if (highDensity) WinSetCoordinateSystem(kCoordinatesDouble);
    SatDrawSky(last_visible, last_sat, last_chs);
    SatDrawChannels(MAX_CHANNELS, last_chs);
    if (highDensity) WinSetCoordinateSystem(kCoordinatesStandard);
  }

  SetDevice(STR_UNKNOWN, "", "");
  InitProtocol(0);
  handshake = 0;
}

static Int16 GpsReceive(void)
{
  Err err;
  Int16 n;
  UInt16 num;

  EvtResetAutoOffTimer();
  if ((n = SerialReceive(gpsRefnum, inbuf, TAM_BUF, &err)) == -1) {
    if (err == serErrTimeOut)
      return 0;
    num = err == serErrLineErr ? SerialGetStatus(gpsRefnum) : 0;
    InfoDialog(ERROR, "Error reading from GPS (%d,%04x)", err, num);
    return -1;
  }

  gps_buf(inbuf, n);

  return n;
}

void Send(uint8_t *buf, int n)
{
  //Int16 i;
  Err err;
      
  //SysTaskDelay(SysTicksPerSecond()/10);
  //for (i = 0; i < n; i++)
    //SerialSend(gpsRefnum, &buf[i], 1, &err);
  SerialSend(gpsRefnum, buf, n, &err);
}

Err AppInit(void *_param)
{
  UInt32 depth, version, attr, hr;
  RGBColorType rgb;
  DateTimeType dt;
  UInt32 y0, dy;
  FontID old;
  AppPrefs *prefs;
  EventType event;
  UInt16 x_sky, y_sky, r_sky, top_sky;
  Int16 c1, c2, c3, c4, c5;
  Err err;

  Debug(0);

  while (EvtEventAvail()) 
    EvtGetEvent(&event, 0);

  highDensity = false;

  if (FtrGet(sysFtrCreator, sysFtrNumWinVersion, &version) == 0 &&
      version >= 4) {
    WinScreenGetAttribute(winScreenDensity, &attr);
    if (attr >= kDensityDouble)
      highDensity = true;
  }

  old = FntSetFont(stdFont);
  if (highDensity) {
    font = DmGetResource(fontExtRscType, hsmallFontId);
    FntDefineFont(hsmallFont, MemHandleLock(font));
  } else {
    font = DmGetResource(fontRscType, smallFontId);
    FntDefineFont(smallFont, MemHandleLock(font));
  }
  FntSetFont(old);

  prefs = GetPrefs();
  current_zoom = prefs->current_zoom;

  WinScreenMode(winScreenModeGetSupportedDepths, NULL, NULL, &depth, NULL);
  if (depth & 0x8000)
    depth = 8;
  else if (depth & 0x0080)
    depth = 8;
  else if (depth & 0x0008)
    depth = 4;
  else if (depth & 0x0002)
    depth = 2;
  else
    depth = 1;

  rgb.r = 255; rgb.g = 255; rgb.b = 255;
  c1 = WinRGBToIndex(&rgb);
  rgb.r = 0; rgb.g = 0; rgb.b = 0;
  c2 = WinRGBToIndex(&rgb);
  rgb.r = 0; rgb.g = 0; rgb.b = 255;
  c3 = WinRGBToIndex(&rgb);
  rgb.r = 136; rgb.g = 136; rgb.b = 136;
  c4 = WinRGBToIndex(&rgb);
  rgb.r = 221; rgb.g = 221; rgb.b = 221;
  c5 = WinRGBToIndex(&rgb);
  SatColor(c1, c2, c3, c4, c5);

  MapColor(prefs->positionColor, prefs->currentColor, prefs->storedColor,
    prefs->selectedColor, prefs->targetColor);

  WinScreenMode(winScreenModeSet, NULL, NULL, &depth, NULL);
  WinScreenMode(winScreenModeGet, &screenx, &screeny, NULL, NULL);

  gpsLib = false;

  err = SysLibFind(gpsLibName, &GpsLibRef);
  if (err) {
    err = SysLibLoad(gpsLibType, gpsLibCreator, &GpsLibRef);
  }
  if (!err) {
    gpsLib = true;
  }

  if ((ptRef = DbOpenCreateByName(WaypointsFName, WaypointsFType, AppID, dmModeReadWrite, &err)) == 0) {
    InfoDialog(ERROR, "Error opening points database (%d)", err);
    return err;
  }

  if ((rtRef = DbOpenCreateByName(RoutesFName, RoutesFType, AppID, dmModeReadWrite, &err)) == 0) {
    DbClose(ptRef);
    ptRef = 0;
    InfoDialog(ERROR, "Error opening routes database (%d)", err);
    return err;
  }
  
  if (highDensity) {
    WinSetCoordinateSystem(kCoordinatesDouble);
    WinScreenMode(winScreenModeGet, &hscreenx, &hscreeny, NULL, NULL);
    //if (hscreeny > hscreenx) hscreeny = hscreenx;
  } else {
    hscreenx = screenx;
    hscreeny = screenx;
  }
  whBuf = WinCreateOffscreenWindow(hscreenx, hscreeny, nativeFormat, &err);

  if (whBuf == NULL || err != 0) {
    InfoDialog(ERROR, "Out of memory (%d)", err);
    return err;
  }

  hr = (hscreenx < hscreeny) ? hscreenx : hscreeny;
  x_sky = hscreenx * X_SKY / 160;
  y_sky = hscreeny * Y_SKY / 160;
  r_sky = hr * R_SKY / 160;
  top_sky = hscreeny * 16 / 160;

  y0 = 16;
  dy = screeny - 10 - y0;
  RctSetRectangle(&mapWindow, 0, y0, screenx, dy);

  y0 = y0 * (hscreeny / screeny);
  dy = dy * (hscreeny / screeny);
  RctSetRectangle(&hmapWindow, 0, y0, hscreenx, dy);

  if (SymbolInit(highDensity) != 0) {
    DbClose(rtRef);
    DbClose(ptRef);
    ptRef = 0;
    rtRef = 0;
    InfoDialog(ERROR, "Symbol init failed");
    return -1;
  }

  if ((err = MapInit(highDensity, &hmapWindow, ptRef, whBuf)) != 0) {
    SymbolFinish();
    DbClose(rtRef);
    DbClose(ptRef);
    ptRef = 0;
    rtRef = 0;
    InfoDialog(ERROR, "Map init failed (%d)", err);
    return err;
  }
  MapZoomOffset((Int16)prefs->mapdetail - 2);
  MapZoomLevel(current_zoom);

  FontInit(highDensity);

  mapfont[0] = highDensity ? hsmallFont : smallFont;
  mapfont[1] = stdFont;
  mapfont[2] =  boldFont;
  MapFont(mapfont[prefs->mapfont]);
  FontSet(mapfont[prefs->mapfont]);

  MapSetData(0);
  dataRef = 0;

  if (prefs->mapname[0]) {
    dataRef = DbOpenByName(prefs->mapname, dmModeReadOnly, &err);
    if (dataRef == 0)
      prefs->mapname[0] = 0;
    else if (MapSetData(dataRef) != 0) {
      prefs->mapname[0] = 0;
      DbClose(dataRef);
      dataRef = 0;
    }
  }

  ListInit();
  ObjectDefine(0, ptRef, NewPointForm, EditPointForm, SeekPointForm,
    getpointname, GetPointComment, getpointdata, NULL, getpointcenter,
    comparename);
  ObjectDefine(1, rtRef, NewRouteForm, EditRouteForm, FollowRouteForm,
    GetRouteName, NULL, NULL, NULL, GetRouteCenter, comparename);
  GetSelectedRoute(NULL);

  if (prefs->display == WaypointsForm)
    ObjectSelect(0);
  else if (prefs->display == RoutesForm)
    ObjectSelect(1);
  else if (prefs->display == SeekPointForm || prefs->display == FollowRouteForm)
    prefs->display = CompassForm;

  newLog = NULL;
  importedLog = NULL;
  currentLog = OpenLog(LogName, AppID, LogType, fileModeUpdate);
  CreateFileList(AppID, LogType, &tracks, LogName);

  if (tracks.n)
    storedLog = OpenLog(tracks.fname[prefs->track], AppID,
                        LogType, fileModeReadOnly);
  else
    storedLog = NULL;

  MapSetCurrentLog(currentLog);
  MapSetStoredLog(prefs->showtrack ? storedLog : NULL);

  MapCoord(prefs->center_lat, prefs->center_lon,
           zoom_factor[current_zoom] / DEG_TO_KM,
           zoom_factor[current_zoom] / DEG_TO_KM);
  MapPosition(prefs->coord.latitude, prefs->coord.longitude, 0, 0);
  MapLock(prefs->locked);

  SetWait(1);
  gpsOnline = false;
  last_validity = 0;

  ts.day = ts.month = ts.year = ts.hour = ts.minute = ts.second = 0;

  SatInit(highDensity, hscreenx, hscreeny, x_sky, y_sky, r_sky, top_sky, whBuf);
  SatResetChannels(last_chs, last_sat);

  AstroInit(highDensity, hscreenx, hscreeny, x_sky, y_sky, r_sky,top_sky);
  AstroPosition(prefs->coord.latitude, prefs->coord.longitude);

  TripReset(prefs->start_time, prefs->moving_time, prefs->distance);

  SerialInit();
  numDevices = MAX_DEVICES;
  if (SerialList(deviceNames, deviceCreators, &numDevices) != 0)
    return -1;

  SetDevice(STR_UNKNOWN, "", "");
  findbuf[0] = 0;

  dt.year = 1989;
  dt.month = 12;
  dt.day = 31;
  dt.hour = 0;
  dt.minute = 0;
  dt.second = 0;
  garmin_t0 = TimDateTimeToSeconds(&dt);

  datum_names = DatumGetList();
  num_datums = DatumGetNum();

  progressIndex = 0;
  progressTotal = 0;

  TrigInit();
  RouteInit(ptRef, rtRef);
  ObjectInit(highDensity);
  TrackSetTop(0);

  gotoActive = 0;
  currentIndex = 0;
  currentSeq = -1;

  InitProtocol(0);
  handshake = 0;

  setTime = false;

  lastRoute = prefs->route;
  if (lastRoute > GetRecNum(1))
    lastRoute = GetRecNum(1);

  return 0;
}

void AppFinish(void)
{
  UInt16 i;
  AppPrefs *prefs;

  prefs = GetPrefs();
  prefs->start_time = TripGetStartTime();
  prefs->moving_time = TripGetMovingTime();
  prefs->distance = TripGetDistance();
  prefs->current_zoom = current_zoom;
  MapGetCenter(&prefs->center_lat, &prefs->center_lon);

  if (gpsOnline) GpsOffline();
 
  FreeRecList(0);
  FreeRecList(1);
  FreeRecList(2);
  FreeRecList(3);

  if (dataRef) {
    MapSetData(0);
    DbClose(dataRef);
    dataRef = 0;
  }

  if (ptRef) DbClose(ptRef);
  if (rtRef) DbClose(rtRef);

  ptRef = rtRef = 0;

  if (gpsLib)
    SysLibRemove(GpsLibRef);

  MapFinish();
  FontFinish();

  FntSetFont(stdFont);
  MemHandleUnlock(font);
  DmReleaseResource(font);

  WinDeleteWindow(whBuf, true);

  if (currentLog)
    CloseLog(currentLog);
  if (storedLog)
    CloseLog(storedLog);

  DestroyFileList(&tracks);

  SymbolFinish();
  TrigFinish();

  for (i = 0; i < numDevices; i++) {
    if (deviceNames[i]) {
      MemPtrFree(deviceNames[i]);
      deviceNames[i] = NULL;
    }
  }

  prefs->route = lastRoute;
}

Boolean InterceptEvent(EventPtr event)
{
  UInt16 form = FrmGetActiveFormID();
  Boolean handled = false;

  switch (event->eType) {
    case keyDownEvent:
      if (!(event->data.keyDown.modifiers & commandKeyMask))
        break;

      switch (event->data.keyDown.chr) {
        case hard1Chr:
          handled = ActionEvent(0, event);
          break;
        case hard2Chr:
          handled = ActionEvent(1, event);
          break;
        case hard3Chr:
          handled = ActionEvent(2, event);
          break;
        case hard4Chr:
          handled = ActionEvent(3, event);
          break;
        case findChr:
          if (form == WaypointsForm && GetRecNum(0) > 0)
            FrmPopupForm(FindForm);
          else if (form == MapForm)
            FrmPopupForm(MapFindForm);
          handled = true;
      }
      break;

    default:
      break;
  }

  return handled;
}

void MenuEvent(UInt16 id)
{
  UInt16 formId, index;
  AppPrefs *prefs;
  double lat, lon;
  WaypointType *p, pos;
  Err err;

  switch (id) {
    case GpsForm:
    case UnitsForm:
    case ButtonsForm:
    case ProgressForm:
    case LoadMapForm:
    case MapColorForm:
    case NetworkForm:
      FrmPopupForm(id);
      break;

    case AboutForm:
      AbtShowAboutPumpkin(AppID);
      break;

    case SymbolForm:
      pointForm = FrmGetActiveFormID();
      FrmPopupForm(id);
      break;

    case MainForm:
    case SatForm:
    case MapForm:
    case CompassForm:
    case SeekPointForm:
    case FollowRouteForm:
    case WaypointsForm:
    case RoutesForm:
    case TracksForm:
    case TripForm:
    case AstroForm:
      formId = FrmGetActiveFormID();

      if (id != formId) {
        prefs = GetPrefs();
        prefs->display = id;
        
        if (id == WaypointsForm)
          ObjectSelect(0);
        else if (id == RoutesForm)
          ObjectSelect(1);

        if (gotoActive == 1 && id == CompassForm)
          FrmGotoForm(SeekPointForm);
        else if (gotoActive == 2 && id == CompassForm)
          FrmGotoForm(FollowRouteForm);
        else
          FrmGotoForm(id);
      }
      break;

    case HelpCmd:
      FrmHelp(FrmGetActiveFormID());
      break;

    case SendPointCmd:
      formId = FrmGetActiveFormID();
      if (formId == MainForm) {
        if (gpsOnline && last_validity) {
          prefs = GetPrefs();
          StrCopy(pos.name, "Position");
          pos.comment[0] = 0;
          pos.datum = 0;
          pos.date = ts;
          pos.symbol = sym_wpt_dot;
          pos.coord = prefs->coord;
          NetSendPoint(&pos, false);
        } else
          InfoDialog(INFO, "There is no fix");
      } else {
        if (GetRecNum(0) <= 0)
          break;
        index = GetRecIndex(0, GetRecSelection(0));
        if ((p = (WaypointType *)DbOpenRec(ptRef, index, &err)) != NULL) {
          NetSendPoint(p, false);
          DbCloseRec(ptRef, index, (char *)p);
        }
      }
      break;

    case StopNavCmd:
      if (gotoActive) {
        gotoActive = 0;
        currentIndex = 0;
        currentSeq = -1;
        MapResetTarget();
        MapInvalid();

        if (FrmGetActiveFormID() == SeekPointForm)
          ObjectSelect(0);
        else
          ObjectSelect(1);

        prefs = GetPrefs();
        prefs->display = CompassForm;
        FrmGotoForm(CompassForm);
      }
      break;

    case RepositionCmd:
      if (!gpsOnline) {
        MapGetCenter(&lat, &lon);
        prefs = GetPrefs();
        prefs->coord.latitude = lat;
        prefs->coord.longitude = lon;
        prefs->coord.height = 0.0;
        AstroPosition(prefs->coord.latitude, prefs->coord.longitude);
        MapPosition(prefs->coord.latitude, prefs->coord.longitude, 0, 0);
        MapInvalid();
        MapDraw();
      } else
        InfoDialog(INFO, "Repositioning works only when offline");
      break;

    case MeasureDistanceCmd:
      measure = 1;
      MapStartMeasure(measure);
      break;

    case MeasureAreaCmd:
      measure = 2;
      MapStartMeasure(measure);
      break;

    case InvertRouteCmd:
      if (GetRecNum(1) <= 0)
        break;
      RouteInvert(GetRecName(1, GetRecSelection(1)));
      break;

    case ShowProtocolCmd:
      prefs = GetPrefs();
      if (!gpsOnline)
        InfoDialog(INFO, "Not online");
      else if (prefs->protocol != PROTOCOL_GARMIN &&
               prefs->protocol != PROTOCOL_GARMIN_HOST)
        InfoDialog(INFO, "This option is available only for Garmin devices");
      else if (prefs->protocol == PROTOCOL_GARMIN && !handshake)
        InfoDialog(INFO, "Protocol handshake was not completed");
      else
        ShowProtocol();
      break;

    case StartDebugCmd:
      Debug(1);
      DebugMsg("debug begin");
      break;
    case StopDebugCmd:
      DebugMsg("debug end");
      Debug(0);
      break;
    case DeleteDebugCmd:
      Debug(-1);
      break;

    case ImportPointsCmd:
    case ExportPointCmd:
    case ExportPointsCmd:
    case ImportRoutesCmd:
    case ExportRouteCmd:
    case ExportRoutesCmd:
    case ImportTracksCmd:
    case ExportTrackCmd:
    case ExportCurrTrackCmd:
    case ExportTracksCmd:
    case GetDisplayCmd:

      prefs = GetPrefs();
      if (prefs->protocol == PROTOCOL_GARMIN_HOST)
        InfoDialog(INFO, "Garmin Host can not initiate a transfer");
      else if (!gpsOnline)
        InfoDialog(INFO, "Not online");
      else if (prefs->protocol != PROTOCOL_GARMIN)
        InfoDialog(INFO, "This option is available only for Garmin devices");
      else if (!handshake)
        InfoDialog(INFO, "Protocol handshake was not completed");
      else switch (id) {
        case ImportPointsCmd:
          if (garmin_transfer_wpt() != 0)
            InfoDialog(INFO, "Failed");
          break;
        case ExportPointCmd:
          if (GetRecNum(0) > 0) garmin_send_wpt();
          break;
        case ExportPointsCmd:
          if (GetRecNum(0) > 0) garmin_send_wpts();
          break;
        case ImportTracksCmd:
          garmin_transfer_trk();
          break;
        case ExportTrackCmd:
          garmin_send_trk();
          break;
        case ExportCurrTrackCmd:
          garmin_send_curr_trk();
          break;
        case ExportTracksCmd:
          garmin_send_trks();
          break;
        case ImportRoutesCmd:
          garmin_transfer_rte();
          break;
        case ExportRouteCmd:
          if (GetRecNum(1) > 0) garmin_send_rte();
          break;
        case ExportRoutesCmd:
          if (GetRecNum(1) > 0) garmin_send_rtes();
          break;
        case GetDisplayCmd:
          garmin_transfer_disp();
      }
      break;
    case SetTimeCmd:
      if (gpsOnline && last_validity)
        setTime = true;
      else
        InfoDialog(INFO, "There is no fix");
      break;
  }
}
