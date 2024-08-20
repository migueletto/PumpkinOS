#include <PalmOS.h>

#include "gps.h"
#include "app.h"
#include "main.h"
#include "map.h"
#include "mapdecl.h"
#include "misc.h"
#include "list.h"
#include "compass.h"
#include "symbol.h"
#include "log.h"
#include "gui.h"
#include "ddb.h"
#include "file.h"
#include "format.h"
#include "garmin.h"
#include "gpc.h"
#include "trig.h"
#include "fill.h"
#include "label.h"
#include "object.h"
#include "point.h"
#include "route.h"
#include "error.h"
#include "MathLib.h"
#include "ndebug.h"

#include "debug.h"

#define WAYPOINTS_LAYER	0
#define ROUTES_LAYER	1
#define TRACKS_LAYER	2

#define MAX_MEASURES	32

static RectangleType window, subwindow;
static DmOpenRef ptRef, dataRef;
static FileHand currentLog, storedLog;
static WinHandle whBuf, whFont;
static Boolean hd;

typedef struct {
  UInt16 obj;
  double distance;
} MapFilter;

static Int32 first_lat, last_lat, first_lon, last_lon;
static Int32 center_lat, center_lon, dlat0, dlat, dlon0, dlon;
static Int32 target_lat, target_lon;
static Int32 pos_lat, pos_lon;
static double pos_rlat, pos_rlon;	// in radians
static double pos_speed, pos_course;
static Boolean locked;
static Boolean target;
static Boolean invalidMap;
static Int16 positionColor, currentColor, storedColor, selectedColor, targetColor;
static Int16 zoomLevel, zoomOffset;
static FontID font;

static Int16 measureType, measureCount;
static Int16 measureX0, measureY0, measureX1, measureY1;
static Int16 measureX[MAX_MEASURES], measureY[MAX_MEASURES];

static UInt16 findLayer, findCat, selectedObj, tableTop, tableRow, resettable;
static UInt16 layerNumber[MAX_LAYERS_PALM+3];
static char *layerName[MAX_LAYERS_PALM+3];
static UInt16 numfilter;
static MapFilter *filter;

static MapType *map;
static MapColorType *paleterec;
static MapPlineType *datarec[MAX_LAYERS_PALM][2];
static UInt8 *symbolrec[MAX_LAYERS_PALM][2];
static MapICoordType *pointrec[MAX_LAYERS_PALM][32];
static char *textrec[MAX_LAYERS_PALM][16];
static UInt32 *indexrec[MAX_LAYERS_PALM][4];
static UInt8 *colorrec[MAX_LAYERS_PALM];
static UInt16 *regionrec[MAX_LAYERS_PALM];
static MapPOIIndexType *poirec[MAX_LAYERS_PALM][8];

static DmOpenRef tmpRef;
static UInt16 *tmpFilter;

#define MAX_WPT_CATEGORIES 12

/*
static char *WptCategories[MAX_WPT_CATEGORIES] = {
  "All",		// 0
  "Outdor",		// 1
  "Transportation",	// 2
  "Business",		// 3
  "Marker",		// 4
  "Civil",		// 5
  "Marine",		// 6
  "Aviation",		// 7
  "Navigation Aid",	// 8
  "Contacts",		// 9
  "Letter",		// 10
  "Number"		// 11
};
*/

#define MAX_POI_CATEGORIES 12

static char *POICategories[MAX_POI_CATEGORIES] = {
  "All",		// 0
  "Food & Drink",	// 1
  "Lodging",		// 2
  "Attractions",	// 3
  "Entertainment",	// 4
  "Shopping",		// 5
  "Services",		// 6
  "Transportation",	// 7
  "Emergency & Gov.",	// 8
  "Manmade Places",	// 9
  "Water Feature",	// 10
  "Land Feature"	// 11
};

#define MAX_LINE_CATEGORIES 7

static char *LineCategories[MAX_LINE_CATEGORIES] = {
  "All",		// 0
  "Road",		// 1
  "Rail",		// 2
  "Ferry",		// 3
  "Land",		// 4
  "Water",		// 5
  "Boundary"		// 6
};

#define MAX_AREA_CATEGORIES 5

static char *AreaCategories[MAX_AREA_CATEGORIES] = {
  "All",		// 0
  "Urban",		// 1
  "Park",		// 2
  "Land",		// 3
  "Water"		// 4
};

static void MapRedraw(void);
static void MapRecalc(void);
static void MapGrad2Coord(double lat, double lon, Int32 *clat, Int32 *clon);
static void MapCoord2Grad(Int32 clat, Int32 clon, double *lat, double *lon);
static void MapCoord2Rad(Int32 clat, Int32 clon, double *lat, double *lon);
static Boolean MapCoord2Screen(Int32 lat, Int32 lon, Int32 *x, Int32 *y,
                               Boolean offscreen);
static Boolean MapScreen2Coord(Int32 *lat, Int32 *lon, Int16 x, Int16 y);
static void MapDrawDot(Int16 x, Int16 y);
static void MapDrawLine(Int16 x1, Int16 y1, Int16 x2, Int16 y2, Int16 width,
                        Int16 c1, Int16 c2);
static void MapDrawLog(FileHand f, Int16 color, UInt16 max, Boolean line);
static void MapDrawRoute(Int16 color);
static MapPOIType *getpdatarec(UInt16 layer, UInt16 c);
static MapPlineType *getdatarec(UInt16 layer, UInt16 c);
static MapICoordType *getpointrec(UInt16 layer, UInt16 c);
static MapPOIIndexType *getpoirec(UInt16 layer, UInt16 c);
static char *gettextrec(UInt16 layer, UInt16 c);
static char *getitextrec(UInt16 layer, UInt32 i);
static void getpline(UInt16 symbol, Int16 zoom, Int16 *c, Int16 *w);
static void getregion(UInt16 symbol, Int16 zoom, Int16 *c, Int16 *w);
static Boolean clipline(Int32 *x0, Int32 *y0, Int32 *x1, Int32 *y1,
                        Int32 mindim, Int32 maxdim);

static Boolean MapPointDistance(UInt16 layer, UInt16 i, double *d);
static Int16 MapCompareProximity(void *e1, void *e2, Int32 other);
static void MapFillProximity(UInt16 i, UInt16 cat);
static void MapFillCategory(UInt16 i, UInt16 cat);
static void MapFindRefresh(void);
static void MapShowDetail(Int16 layer, UInt16 i);
static void MapSavePoint(Int16 layer, UInt16 i);
static double MapCalcLength(Int16 layer, UInt16 obj);
static double MapCalcArea(Int16 layer, UInt16 obj);
static UInt16 GetLineCat(UInt16 symbol);
static UInt16 GetAreaCat(UInt16 symbol);
static UInt16 quadrant(double a, Int32 x, Int32 y);

Err MapInit(Boolean _hd, RectangleType *rect, DmOpenRef _ptRef, WinHandle _whBuf)
{
  UInt16 index;
  Err err;

  hd = _hd;
  MemMove(&window, rect, sizeof(window));
  RctSetRectangle(&subwindow,
                  window.topLeft.x + window.extent.x/3,
                  window.topLeft.y + window.extent.y/3,
                  window.extent.x/3, window.extent.y/3);

  ptRef = _ptRef;
  whBuf = _whBuf;
  target = false;
  currentLog = NULL;
  storedLog = NULL;
  dataRef = 0;
  map = NULL;
  zoomLevel = DEFAULT_ZOOM;
  zoomOffset = 0;
  findLayer = WAYPOINTS_LAYER;
  findCat = 0;
  tableTop = 0;
  tableRow = 0;
  selectedObj = 0;
  layerName[0] = "Waypoints";
  layerName[1] = "Routes";
  layerName[2] = "Tracks";
  font = stdFont;
  invalidMap = true;
  pos_lat = pos_lon = 0;
  pos_rlat = pos_rlon = 0;
  filter = NULL;
  resettable = 1;

  if (hd) WinSetCoordinateSystem(kCoordinatesDouble);
  whFont = WinCreateOffscreenWindow(window.extent.x, FntCharHeight(), screenFormat, &err);
  if (hd) WinSetCoordinateSystem(kCoordinatesStandard);

  tmpRef = 0;
  tmpFilter = NULL;

  if ((tmpRef = DbOpenCreateByName(TempName, TempType, AppID, dmModeReadWrite, &err)) == 0) {
    return -1;
  }

  if ((tmpFilter = (UInt16 *)DbOpenRec(tmpRef, 0, &err)) == NULL) {
    if (DbCreateRec(tmpRef, &index, MAX_RECSIZE, 0) != 0) {
      DbClose(tmpRef);
      return -2;
    }
    if ((tmpFilter = (UInt16 *)DbOpenRec(tmpRef, index, &err)) == NULL) {
      DbClose(tmpRef);
      return -3;
    }
  }

  return 0;
}

void MapSetCurrentLog(FileHand _currentLog)
{
  currentLog = _currentLog;
}

void MapSetStoredLog(FileHand _storedLog)
{
  storedLog = _storedLog;
}

Err MapSetData(DmOpenRef _dataRef)
{
  UInt16 i, j;
  Err err;

  if (_dataRef) {
    dataRef = _dataRef;
    if ((map = (MapType *)DbOpenRec(dataRef, 0, &err)) == NULL) {
      InfoDialog(ERROR, "openrec map: %d", err);
      dataRef = 0;
      return -1;
    }

    if (map->version != MAP_VERSION) {
      DbCloseRec(dataRef, 0, (char *)map);
      InfoDialog(ERROR, "Invalid map version");
      dataRef = 0;
      return -1;
    }


    if (map->paleterec &&
       (paleterec = (MapColorType *)DbOpenRec(dataRef,
          map->paleterec, &err)) == NULL)
      InfoDialog(ERROR, "openrec palete %d: %d", map->paleterec, err);

    for (i = 0; i < map->nlayers; i++) {
      for (j = 0; j < 2; j++)
        datarec[i][j] =  NULL;
      for (j = 0; j < 2; j++)
        symbolrec[i][j] =  NULL;
      for (j = 0; j < 32; j++)
        pointrec[i][j] = NULL;
      for (j = 0; j < 16; j++)
        textrec[i][j] = NULL;
      for (j = 0; j < 4; j++)
        indexrec[i][j] = NULL;
      for (j = 0; j < 8; j++)
        poirec[i][j] = NULL;
      colorrec[i] =  NULL;
      regionrec[i] = NULL;

      if (map->layer[i].datarec)
        for (j = 0; j < map->layer[i].ndatarecs; j++) {
          datarec[i][j] = (MapPlineType *)DbOpenRec(dataRef,
             map->layer[i].datarec+j, &err);
          if (datarec[i][j] == NULL) {
            InfoDialog(ERROR, "openrec %d data %d: %d", i,
              map->layer[i].datarec+j, err);
            dataRef = 0;
            return -1;
          }
        }

      if (map->layer[i].symbolrec)
        for (j = 0; j < map->layer[i].nsymbolrecs; j++) {
          symbolrec[i][j] = (UInt8 *)DbOpenRec(dataRef,
             map->layer[i].symbolrec+j, &err);
          if (symbolrec[i][j] == NULL) {
            InfoDialog(ERROR, "openrec %d symbol %d (%d): %d", i,
              map->layer[i].symbolrec+j, map->layer[i].nsymbolrecs, err);
            dataRef = 0;
            return -1;
          }
        }

      if (map->layer[i].pointrec)
        for (j = 0; j < map->layer[i].npointrecs; j++) {
          pointrec[i][j] = (MapICoordType *)DbOpenRec(dataRef,
             map->layer[i].pointrec+j, &err);
          if (pointrec[i][j] == NULL) {
            InfoDialog(ERROR, "openrec %d point %d: %d", i,
              map->layer[i].pointrec+j, err);
            dataRef = 0;
            return -1;
          }
        }

      if (map->layer[i].textrec)
        for (j = 0; j < map->layer[i].ntextrecs; j++) {
          textrec[i][j] = (char *)DbOpenRec(dataRef,
             map->layer[i].textrec+j, &err);
          if (textrec[i][j] == NULL) {
            InfoDialog(ERROR, "openrec %d text %d: %d", i,
              map->layer[i].textrec+j, err);
            dataRef = 0;
            return -1;
          }
        }

      if (map->layer[i].indexrec)
        for (j = 0; j < map->layer[i].nindexrecs; j++) {
          indexrec[i][j] = (UInt32 *)DbOpenRec(dataRef,
             map->layer[i].indexrec+j, &err);
          if (indexrec[i][j] == NULL) {
            InfoDialog(ERROR, "openrec %d index %d: %d", i,
              map->layer[i].indexrec+j, err);
            dataRef = 0;
            return -1;
          }
        }

      if (map->layer[i].colorrec &&
         (colorrec[i] = (UInt8 *)DbOpenRec(dataRef,
            map->layer[i].colorrec, &err)) == NULL)
        InfoDialog(ERROR, "openrec %d color %d: %d", i,
          map->layer[i].colorrec, err);

      if (map->layer[i].poirec)
        for (j = 0; j < map->layer[i].npoirecs; j++) {
          poirec[i][j] = (MapPOIIndexType *)DbOpenRec(dataRef,
             map->layer[i].poirec+j, &err);
          if (poirec[i][j] == NULL) {
            InfoDialog(ERROR, "openrec %d poi %d: %d", i,
              map->layer[i].poirec+j, err);
            dataRef = 0;
            return -1;
          }
        }

      if (map->layer[i].regionrec[zoomLevel] &&
         (regionrec[i] = (UInt16 *)DbOpenRec(dataRef,
            map->layer[i].regionrec[zoomLevel], &err)) == NULL) {
        InfoDialog(ERROR, "openrec %d region %d: %d", i,
          map->layer[i].regionrec[zoomLevel], err);
        dataRef = 0;
        return -1;
      }
    }
  } else {
    if (map) {
      if (map->paleterec)
        DbCloseRec(dataRef, map->paleterec, (char *)paleterec);
      paleterec = NULL;

      for (i = 0; i < map->nlayers; i++) {
        if (map->layer[i].datarec)
          for (j = 0; j < map->layer[i].ndatarecs; j++) {
            DbCloseRec(dataRef, map->layer[i].datarec+j, (char *)datarec[i][j]);
            datarec[i][j] = NULL;
          }

        if (map->layer[i].symbolrec)
          for (j = 0; j < map->layer[i].nsymbolrecs; j++) {
            DbCloseRec(dataRef, map->layer[i].symbolrec+j, (char *)symbolrec[i][j]);
            symbolrec[i][j] = NULL;
          }

        if (map->layer[i].pointrec)
          for (j = 0; j < map->layer[i].npointrecs; j++) {
            DbCloseRec(dataRef, map->layer[i].pointrec+j, (char *)pointrec[i][j]);
            pointrec[i][j] = NULL;
          }

        if (map->layer[i].textrec)
          for (j = 0; j < map->layer[i].ntextrecs; j++) {
            DbCloseRec(dataRef, map->layer[i].textrec+j, (char *)textrec[i][j]);
            textrec[i][j] = NULL;
          }

        if (map->layer[i].indexrec)
          for (j = 0; j < map->layer[i].nindexrecs; j++) {
            DbCloseRec(dataRef, map->layer[i].indexrec+j, (char *)indexrec[i][j]);
            indexrec[i][j] = NULL;
          }

        if (map->layer[i].poirec)
          for (j = 0; j < map->layer[i].npoirecs; j++) {
            DbCloseRec(dataRef, map->layer[i].poirec+j, (char *)poirec[i][j]);
            poirec[i][j] = NULL;
          }

        if (map->layer[i].colorrec)
          DbCloseRec(dataRef, map->layer[i].colorrec, (char *)colorrec[i]);
        colorrec[i] = NULL;

        if (map->layer[i].regionrec[zoomLevel])
          DbCloseRec(dataRef, map->layer[i].regionrec[zoomLevel], (char *)regionrec[i]);
        regionrec[i] = NULL;
      }
      DbCloseRec(dataRef, 0, (char *)map);
    }
    dataRef = _dataRef;
    map = NULL;
  }

  return 0;
}

void MapFinish(void)
{
  if (filter) MemPtrFree(filter);
  if (whFont) WinDeleteWindow(whFont, false);

  if (tmpRef && tmpFilter) {
    DbCloseRec(tmpRef, 0, (char *)tmpFilter);
    DbClose(tmpRef);
  }
  tmpRef = 0;
  tmpFilter = NULL;
}

void MapColor(Int16 _positionColor, Int16 _currentColor, Int16 _storedColor,
              Int16 _selectedColor, Int16 _targetColor)
{
  positionColor = _positionColor;
  currentColor = _currentColor;
  storedColor = _storedColor;
  selectedColor = _selectedColor;
  targetColor = _targetColor;
}

void MapFont(FontID _font)
{
  font = _font;
}

void MapDraw(void)
{
  if (invalidMap) {
    MapRedraw();
    invalidMap = false;
  }
  if (hd) WinSetCoordinateSystem(kCoordinatesDouble);
  WinSetClip(&window);
  WinCopyRectangle(whBuf, WinGetActiveWindow(), &window,
     window.topLeft.x, window.topLeft.y, winPaint);
  WinResetClip();
  if (hd) WinSetCoordinateSystem(kCoordinatesStandard);
}

void MapDrawRel(WinHandle src, WinHandle dst, Int16 dx, Int16 dy)
{
  if (hd) WinSetCoordinateSystem(kCoordinatesDouble);
  WinSetClip(&window);

  if (dx || dy) {
    RGBColorType rgb;
    WinHandle oldw;
    UInt16 oldc, c;

    if (dataRef && map) {
      rgb.r = map->bgcolor.r;
      rgb.g = map->bgcolor.g;
      rgb.b = map->bgcolor.b;
    } else {
      rgb.r = 255;
      rgb.g = 255;
      rgb.b = 255;
    }
    c = WinRGBToIndex(&rgb);

    oldw = WinSetDrawWindow(dst);
    oldc = WinSetBackColor(c);
    WinFillRectangle(&window, 0);
    WinSetBackColor(oldc);
    WinSetDrawWindow(oldw);
  }

  WinCopyRectangle(src, dst, &window,
     window.topLeft.x + dx, window.topLeft.y + dy, winPaint);

  WinResetClip();
  if (hd) WinSetCoordinateSystem(kCoordinatesStandard);
}

void MapCoord(double _lat, double _lon, double _dlat, double _dlon)
{
  MapGrad2Coord(_lat, _lon, &center_lat, &center_lon);
  MapGrad2Coord(_dlat, _dlon, &dlat, &dlon);
  if (dlat < 0) dlat = -dlat;
  dlat0 = dlat;
  dlon0 = dlon;
  MapRecalc();
  MapInvalid();
}

void MapGetCenter(double *_lat, double *_lon)
{
  MapCoord2Grad(center_lat, center_lon, _lat, _lon);
}

void MapCenter(double _lat, double _lon)
{
  MapGrad2Coord(_lat, _lon, &center_lat, &center_lon);
  MapRecalc();
  MapInvalid();
}

void MapCenterXY(UInt16 x, UInt16 y)
{
  MapScreen2Coord(&center_lat, &center_lon, x, y);
  MapRecalc();
  MapInvalid();
}

void MapMoveXY(Int16 dx, Int16 dy)
{
  Int32 ddx, ddy;
  ddx = (dlon * dx) / window.extent.x;
  ddy = (dlon * dy) / window.extent.y;
  center_lat -= ddy;
  center_lon -= ddx;
  MapRecalc();
  MapInvalid();
}

void MapZoom(double _dlat, double _dlon)
{
  MapGrad2Coord(_dlat, _dlon, &dlat, &dlon);
  if (dlat < 0) dlat = -dlat;
  MapRecalc();
  MapInvalid();
}

void MapZoomOffset(Int16 offset)
{
  zoomOffset = offset;
}

void MapZoomLevel(Int16 level)
{
  Int16 i;
  Err err;

  level += zoomOffset;

  if (level < 0)
    level = 0;
  else if (level >= NUM_ZOOM)
    level = NUM_ZOOM-1;

  if (zoomLevel != level && map) {
    for (i = 0; i < map->nlayers; i++) {
      if (map->layer[i].regionrec[zoomLevel])
        DbCloseRec(dataRef, map->layer[i].regionrec[zoomLevel],
           (char *)regionrec[i]);
      regionrec[i] = NULL;

      if (map->layer[i].regionrec[level] &&
          (regionrec[i] = (UInt16 *)DbOpenRec(dataRef,
             map->layer[i].regionrec[level], &err)) == NULL)
        InfoDialog(ERROR, "x openrec %d region %d: %d",
          i, map->layer[i].regionrec[level], err);
    }
  }

  zoomLevel = level;
}

void MapPosition(double lat, double lon, double speed, double course)
{
  Int32 tmp_lat, tmp_lon;

  MapGrad2Coord(lat, lon, &tmp_lat, &tmp_lon);

  if (pos_lat != tmp_lat || pos_lon != tmp_lon) {
    MapInvalid();

    if (findLayer == WAYPOINTS_LAYER)
      resettable = 1;
    else if (findLayer > TRACKS_LAYER)
      if (map->layer[layerNumber[findLayer]].type == MAP_POINT)
        resettable = 1;
  }

  pos_rlat = TORAD(lat);
  pos_rlon = TORAD(lon);
  pos_lat = tmp_lat;
  pos_lon = tmp_lon;
  pos_speed = speed;
  pos_course = course;
}

Boolean MapWaypointDistance(WaypointType *p, float *d)
{
  double rcy, rty, cosd;

  rcy = pos_rlat;
  rty = TORAD(p->coord.latitude);

  cosd = sin(rcy) * sin(rty) + cos(rcy) * cos(rty) *
           cos(pos_rlon - TORAD(p->coord.longitude));
  *d = acos(cosd) * EARTH_RADIUS_M;

  return *d < MAX_DISTANCE;
}

static Boolean MapPointDistance(UInt16 layer, UInt16 i, double *d)
{
  MapICoordType *icoord;
  double lat, lon, rcy, rty, cosd;

  icoord = getpointrec(layer, i);
  MapCoord2Rad(icoord->y, icoord->x, &lat, &lon);

  rcy = pos_rlat;
  rty = lat;

  cosd = sin(rcy) * sin(rty) + cos(rcy) * cos(rty) * cos(pos_rlon - lon);
  *d = acos(cosd);

  if (*d > MAX_DISTANCE_R)
    return false;

  *d *= EARTH_RADIUS_M;
  return true;
}

static void MapDrawLog(FileHand f, Int16 color, UInt16 max, Boolean line)
{
  TracklogType tlog;
  UInt32 size;
  UInt16 i, n, oldc;
  Boolean visible, visible0;
  Int32 x, y, x0, y0, lat0, lon0, loglat, loglon;

  size = LogSize(f);
  if (size > 0) {
    n = size / sizeof(TracklogType);
    if (n > max)
      n = max;
    SeekLog(f, -(n*sizeof(TracklogType)));
    oldc = WinSetForeColor(color);

    ReadLog(f, &tlog, sizeof(TracklogType));
    MapGrad2Coord(tlog.latitude, tlog.longitude, &loglat, &loglon);
    visible0 = MapCoord2Screen(loglat, loglon, &x0, &y0, false);
    lat0 = loglat;
    lon0 = loglon;

    if (visible0 && !line)
      MapDrawDot(x0, y0);

    for (i = 1; i < n; i++) {
      ReadLog(f, &tlog, sizeof(TracklogType));
      MapGrad2Coord(tlog.latitude, tlog.longitude, &loglat, &loglon);
      visible = MapCoord2Screen(loglat, loglon, &x, &y, false);

      if (line) {
        if (visible0 && !visible) {
          MapCoord2Screen(loglat, loglon, &x, &y, true);
          if (x0 < -1000 || x0 > 1000 || y0 < -1000 || y0 > 1000)
            x0 = x, y0 = y;
          MapDrawLine(x0, y0, x, y, 3, color, color);
        } else if (!visible0 && visible) {
          MapCoord2Screen(lat0, lon0, &x0, &y0, true);
          if (x0 < -1000 || x0 > 1000 || y0 < -1000 || y0 > 1000)
            x0 = x, y0 = y;
          MapDrawLine(x0, y0, x, y, 3, color, color);
        } else if (visible0 && visible) {
          MapDrawLine(x0, y0, x, y, 3, color, color);
        } else {
          Int32 x1 = lon0, y1 = lat0, x2 = loglon, y2 = loglat;
          if (clipline(&x1, &y1, &x2, &y2, first_lon, last_lon) &&
              clipline(&y1, &x1, &y2, &x2, first_lat, last_lat)) {
            MapCoord2Screen(lat0, lon0, &x0, &y0, true);
            MapCoord2Screen(loglat, loglon, &x, &y, true);
            MapDrawLine(x0, y0, x, y, 3, color, color);
          }
        }

        x0 = x;
        y0 = y;
        lat0 = loglat;
        lon0 = loglon;
        visible0 = visible;
      } else if (visible)
        MapDrawDot(x, y);
    }

    WinSetForeColor(oldc);
    SeekLog(f, 0);
  }
}

static void MapDrawRoute(Int16 color)
{
  DmOpenRef dbRef;
  RoutePointType *rp;
  UInt16 i, n, index, oldf;
  Boolean visible, visible0;
  Int32 x, y, x0, y0, lat0, lon0, loglat, loglon;
  Err err;

  if (GetRecNum(1) == 0 || (n = GetRecNum(2)) == 0)
    return;

  if ((dbRef = DbOpenByName(GetRecName(1, GetRecSelection(1)), dmModeReadOnly, &err)) == 0)
    return;

  index = GetRecIndex(2, 0);
  rp = (RoutePointType *)DbOpenRec(dbRef, index, &err);
  if (rp == NULL) {
    DbClose(dbRef);
    return;
  }

  MapGrad2Coord(rp->coord.latitude, rp->coord.longitude, &loglat, &loglon);
  visible0 = MapCoord2Screen(loglat, loglon, &x0, &y0, false);
  lat0 = loglat;
  lon0 = loglon;

  oldf = WinSetForeColor(color);

  if (visible0) {
    MapDrawDot(x0, y0); // in case the symbol is not drawn
    DrawSymbol(hsymbolId + GetSymbolIndex(rp->symbol),
      x0, y0, true, rp->name, font, color, false);
  }

  DbCloseRec(dbRef, index, (char *)rp);

  for (i = 1; i < n; i++) {
    index = GetRecIndex(2, i);
    rp = (RoutePointType *)DbOpenRec(dbRef, index, &err);
    if (rp == NULL)
      continue;

    MapGrad2Coord(rp->coord.latitude, rp->coord.longitude, &loglat, &loglon);
    visible = MapCoord2Screen(loglat, loglon, &x, &y, false);

    if (visible0 && !visible) {
      MapCoord2Screen(loglat, loglon, &x, &y, true);
      if (x0 < -1000 || x0 > 1000 || y0 < -1000 || y0 > 1000)
        x0 = x, y0 = y;
      MapDrawLine(x0, y0, x, y, 3, color, color);
    } else if (!visible0 && visible) {
      MapCoord2Screen(lat0, lon0, &x0, &y0, true);
      if (x0 < -1000 || x0 > 1000 || y0 < -1000 || y0 > 1000)
        x0 = x, y0 = y;
      MapDrawLine(x0, y0, x, y, 3, color, color);
    } else if (visible0 && visible) {
      MapDrawLine(x0, y0, x, y, 3, color, color);
    } else {
      Int32 x1 = lon0, y1 = lat0, x2 = loglon, y2 = loglat;
      if (clipline(&x1, &y1, &x2, &y2, first_lon, last_lon) &&
          clipline(&y1, &x1, &y2, &x2, first_lat, last_lat)) {
        MapCoord2Screen(lat0, lon0, &x0, &y0, true);
        MapCoord2Screen(loglat, loglon, &x, &y, true);
        MapDrawLine(x0, y0, x, y, 3, color, color);
      }
    }

    if (visible) {
      MapDrawDot(x, y); // in case the symbol is not drawn
      DrawSymbol(hsymbolId + GetSymbolIndex(rp->symbol),
        x, y, true, rp->name, font, color, false);
    }

    DbCloseRec(dbRef, index, (char *)rp);

    x0 = x;
    y0 = y;
    lat0 = loglat;
    lon0 = loglon;
    visible0 = visible;
  }

  WinSetForeColor(oldf);
}

void MapLock(Boolean _locked)
{
  locked = _locked;
}

static void MapRecalc(void)
{
  first_lat = center_lat - dlat/2;
  last_lat = center_lat + dlat/2;
  first_lon = center_lon - dlon/2;
  last_lon = center_lon + dlon/2;
}

static void MapGrad2Coord(double lat, double lon, Int32 *clat, Int32 *clon)
{
  *clat = (Int32)(-lat * (3600.0 * 32.0));
  *clon = (Int32)(lon * (3600.0 * 32.0));
}

static void MapCoord2Grad(Int32 clat, Int32 clon, double *lat, double *lon)
{
  *lat = -(double)(clat) / (3600.0 * 32.0);
  *lon = (double)(clon) / (3600.0 * 32.0);
}

static void MapCoord2Rad(Int32 clat, Int32 clon, double *lat, double *lon)
{
  // (a * PI) / (3600 * 32 * 180)
  *lat = -((double)(clat) * sys_pi()) / 20736000.0;
  *lon = ((double)(clon) * sys_pi()) / 20736000.0;
}

static Boolean MapCoord2Screen(Int32 lat, Int32 lon, Int32 *x, Int32 *y,
                               Boolean offscreen)
{
  if (offscreen ||
      (lat >= first_lat && lat <= last_lat &&
       lon >= first_lon && lon <= last_lon)) {
    *x = window.topLeft.x + ((lon - first_lon) * window.extent.x) / dlon;
    *y = window.topLeft.y + ((lat - first_lat) * window.extent.y) / dlat;
    return true;
  }
  return false;
}

static Boolean MapScreen2Coord(Int32 *lat, Int32 *lon, Int16 x, Int16 y)
{
  *lat = first_lat + (dlat * (y - window.topLeft.y)) / window.extent.y;
  *lon = first_lon + (dlon * (x - window.topLeft.x)) / window.extent.x;
  return true;
}

static void MapDrawDot(Int16 x, Int16 y)
{
  if (hd) {
    WinDrawLine(x-1, y-4, x+1, y-4);
    WinDrawLine(x-3, y-3, x+3, y-3);
    WinDrawLine(x-3, y-2, x+3, y-2);
    WinDrawLine(x-4, y-1, x+4, y-1);
    WinDrawLine(x-4, y+0, x+4, y+0);
    WinDrawLine(x-4, y+1, x+4, y+1);
    WinDrawLine(x-3, y+2, x+3, y+2);
    WinDrawLine(x-3, y+3, x+3, y+3);
    WinDrawLine(x-1, y+4, x+1, y+4);
  } else {
    WinDrawPixel(x, y-2);
    WinDrawLine(x-1, y-1, x+1, y-1);
    WinDrawLine(x-2, y, x+2, y);
    WinDrawLine(x-1, y+1, x+1, y+1);
    WinDrawPixel(x, y+2);
  }
}

static void MapDrawLine(Int16 x1, Int16 y1, Int16 x2, Int16 y2, Int16 width,
                        Int16 c1, Int16 c2)
{
  Int16 tx, ty;

  switch (width) {
    case 1:
      WinSetForeColor(c1);
      WinDrawLine(x1, y1, x2, y2);
      break;
    case 2:
      WinSetForeColor(c1);
      WinDrawLine(x1, y1, x2, y2);
      tx = x2 > x1 ? x2-x1 : x1-x2;
      ty = y2 > y1 ? y2-y1 : y1-y2;
      if (tx < ty)
        WinDrawLine(x1-1, y1, x2-1, y2);
      else
        WinDrawLine(x1, y1-1, x2, y2-1);
      break;
    case 3:
      WinSetForeColor(c2);
      WinDrawLine(x1, y1, x2, y2);
      WinSetForeColor(c1);
      tx = x2 > x1 ? x2-x1 : x1-x2;
      ty = y2 > y1 ? y2-y1 : y1-y2;
      if (tx < ty) {
        WinDrawLine(x1-1, y1, x2-1, y2);
        WinDrawLine(x1+1, y1, x2+1, y2);
      } else {
        WinDrawLine(x1, y1-1, x2, y2-1);
        WinDrawLine(x1, y1+1, x2, y2+1);
      }
      break;
    case 4:
      WinSetForeColor(c1);
      WinDrawLine(x1, y1, x2, y2);
      tx = x2 > x1 ? x2-x1 : x1-x2;
      ty = y2 > y1 ? y2-y1 : y1-y2;
      if (tx < ty) {
        WinDrawLine(x1-1, y1, x2-1, y2);
        WinDrawLine(x1+1, y1, x2+1, y2);
        WinDrawLine(x1+2, y1, x2+2, y2);
      } else {
        WinDrawLine(x1, y1-1, x2, y2-1);
        WinDrawLine(x1, y1+1, x2, y2+1);
        WinDrawLine(x1, y1+2, x2, y2+2);
      }
      break;
    case 5:
      WinSetForeColor(c1);
      WinDrawLine(x1, y1, x2, y2);
      tx = x2 > x1 ? x2-x1 : x1-x2;
      ty = y2 > y1 ? y2-y1 : y1-y2;
      if (tx < ty) {
        WinDrawLine(x1-2, y1, x2-2, y2);
        WinDrawLine(x1-1, y1, x2-1, y2);
        WinDrawLine(x1+1, y1, x2+1, y2);
        WinDrawLine(x1+2, y1, x2+2, y2);
      } else {
        WinDrawLine(x1, y1-2, x2, y2-2);
        WinDrawLine(x1, y1-1, x2, y2-1);
        WinDrawLine(x1, y1+1, x2, y2+1);
        WinDrawLine(x1, y1+2, x2, y2+2);
      }
  }
}

static void MapRedraw(void)
{
  WinHandle oldWh;
  WaypointType *p;
  RGBColorType rgb;
  MapLayerType *layer;
  MapPlineType *pline;
  MapColorType *color;
  MapIBoundingType *bounds;
  MapICoordType *coord, *polygon;
  MapLabelType sel;
  UInt16 i, j, k, c, m, n, firstobj, nobjs;
  Int32 x, y, x0, y0, pos_x, pos_y, tar_x, tar_y;
  Int32 mx, my, x1, x2, y1, y2;
  Int16 xr, yr, nrx, nry, xr0, xr1, yr0, yr1, ir, incr;
  Int16 oldt, oldb, oldc, bc, fc, tc, lc, lc2, width;
  UInt8 *visited, *bsymbol;
  Boolean named, onscreen;
  UInt16 lastsymbol, *wsymbol;
  Int32 dx, dy, mind, maxd, dseg;
  Int32 wlat, wlon;
  FontID oldf;
  float angle;
  char *name;
  double a;
  Err err;

  if (hd) WinSetCoordinateSystem(kCoordinatesDouble);

  oldWh = WinSetDrawWindow(whBuf);
  WinEraseWindow();
  WinSetClip(&window);
  oldf = FntSetFont(font);
  oldt = WinSetTextColor(0);
  oldc = WinSetForeColor(0);
  oldb = WinSetBackColor(0);

  onscreen = MapCoord2Screen(pos_lat, pos_lon, &pos_x, &pos_y, false);

  if (locked && (!onscreen || !RctPtInRectangle(pos_x, pos_y, &subwindow))) {
    center_lat = pos_lat;
    center_lon = pos_lon;
    MapRecalc();
    onscreen = MapCoord2Screen(pos_lat, pos_lon, &pos_x, &pos_y, false);
  }

  LabelInit();
  sel.type = -1;

  if (dataRef && map) {
    rgb.r = map->bgcolor.r;
    rgb.g = map->bgcolor.g;
    rgb.b = map->bgcolor.b;
    bc = WinRGBToIndex(&rgb);

    WinSetBackColor(bc);
    WinFillRectangle(&window, 0);

    for (i = 0; i < map->nlayers; i++) {
      layer = &map->layer[i];

      if (!(layer->flags & MAP_LAYER_ACTIVE) ||
           (layer->flags & MAP_LAYER_LARGE) ||
          !regionrec[i] || !pointrec[i][0])
        continue;

      named = layer->flags & MAP_LAYER_NAMED;
      rgb.r = layer->textcolor.r;
      rgb.g = layer->textcolor.g;
      rgb.b = layer->textcolor.b;
      tc = WinRGBToIndex(&rgb);

      rgb.r = layer->color.r;
      rgb.g = layer->color.g;
      rgb.b = layer->color.b;
      fc = WinRGBToIndex(&rgb);
      WinSetForeColor(fc);

      nrx = layer->xregions;
      nry = layer->yregions;
      bounds = (MapIBoundingType *)(&layer->bounds);
      dx = bounds->x1 - bounds->x0;
      dy = bounds->y1 - bounds->y0;

      xr0 = ((first_lon - bounds->x0) * nrx) / dx;
      xr1 = ((last_lon - bounds->x0) * nrx) / dx;
      yr0 = ((first_lat - bounds->y0) * nry) / dy;
      yr1 = ((last_lat - bounds->y0) * nry) / dy;

      if (xr0 < 0) xr0 = 0;
      if (xr1 >= nrx) xr1 = nrx-1;
      if (yr0 < 0) yr0 = 0;
      if (yr1 >= nry) yr1 = nry-1;

      incr = 2*(nrx-1-xr1+xr0);

      switch (layer->type) {
        case MAP_POINT:
          lastsymbol = 0;

          for (ir = 2*(yr0*nrx+xr0), yr = yr0; yr <= yr1; yr++, ir += incr) {
            for (xr = xr0; xr <= xr1; xr++) {
               firstobj = regionrec[i][ir++];
               nobjs = regionrec[i][ir++];

               for (j = 0; j < nobjs; j++) {
                 c = regionrec[i][firstobj+j];
                 coord = getpointrec(i, c);
                 if (layer->flags & MAP_LAYER_SYMBOL)
                   wsymbol = c < 32720 ?
                     (UInt16 *)symbolrec[i][0] : (UInt16 *)symbolrec[i][1];
                 else
                   wsymbol = NULL;

                 if (MapCoord2Screen(coord->y, coord->x, &x, &y, false)) {
                   lastsymbol = wsymbol ? wsymbol[c % 32720] : 0xFFFF;
                   name = named ? gettextrec(i, c) : NULL;

                   if (findLayer > TRACKS_LAYER &&
                       layerNumber[findLayer] == i && selectedObj == c) {
                     sel.type = MAP_POINT;
                     sel.x = x;
                     sel.y = y;
                     sel.font = font;
                     sel.color = tc;
                     sel.label = name;
                     if (lastsymbol == 0xFFFE)	// 0xFFFE = empty
                       sel.layer = 0;
                     else
                       sel.layer = hsymbolId + GetSymbolIndex(lastsymbol);
                   } else {
                     if (lastsymbol == 0xFFFE)	// 0xFFFE = empty
                       DrawSymbol(0, x, y, true, name, font, tc, false);
                     else
                       DrawSymbol(hsymbolId + GetSymbolIndex(lastsymbol),
                         x, y, true, name, font, tc, false);
                   }
                 }
               }
            }
          }
          break;
        case MAP_PLINE:
        case MAP_REGION:
          lastsymbol = 0;
          bsymbol = layer->flags & MAP_LAYER_SYMBOL ? symbolrec[i][0] : NULL;

          visited = MemPtrNew(layer->nobjs);
          MemSet(visited, layer->nobjs, 0);
          a = (double)window.extent.y / (double)window.extent.x;

          for (ir = 2*(yr0*nrx+xr0), yr = yr0; yr <= yr1; yr++, ir += incr) {
            for (xr = xr0; xr <= xr1; xr++) {
              firstobj = regionrec[i][ir++];
              nobjs = regionrec[i][ir++];

              for (j = 0; j < nobjs; j++) {
                k = regionrec[i][firstobj+j];

                if (visited[k])
                  continue;
                visited[k] = 1;

                pline = getdatarec(i, k);
                if (pline->npoints == 0)
                  continue;

                width = 1;
                lc = fc;
                lc2 = bc;

                if (findLayer > TRACKS_LAYER &&
                    layerNumber[findLayer] == i && selectedObj == k) {
                  lc = lc2 = selectedColor;
                  width = 3;
                } else {
                  if (layer->type == MAP_REGION) {
                    if (paleterec && colorrec[i]) {
                      color = &paleterec[colorrec[i][k]];
                      rgb.r = color->r;
                      rgb.g = color->g;
                      rgb.b = color->b;
                      lc = WinRGBToIndex(&rgb);
                      width = 1;
                    } else if (bsymbol)
                      getregion(bsymbol[k], zoomLevel, &lc, &width);
                  } else {
                    if (bsymbol) {
                      getpline(bsymbol[k], zoomLevel, &lc, &width);
                      if (paleterec && colorrec[i]) {
                        color = &paleterec[colorrec[i][k]];
                        rgb.r = color->r;
                        rgb.g = color->g;
                        rgb.b = color->b;
                        lc = WinRGBToIndex(&rgb);
                      }
                    } else { 
                      if (paleterec && colorrec[i]) {
                        color = &paleterec[colorrec[i][k]];
                        rgb.r = color->r;
                        rgb.g = color->g;
                        rgb.b = color->b;
                        lc = WinRGBToIndex(&rgb);
                        width = 1;
                      }
                    }
                  }
                }

                if (width == 0)
                  continue;

                WinSetForeColor(lc);

                c = pline->firstpoint;
                n = c+pline->npoints;

                coord = getpointrec(i, c);
                MapCoord2Screen(coord->y, coord->x, &x0, &y0, true);

                if (layer->type == MAP_REGION) {
                  polygon = MemPtrNew(2*pline->npoints * sizeof(MapICoordType));

                  for (m = 0, c++; c <= n; c++) {
                    coord = c == n ? getpointrec(i, pline->firstpoint) :
                                     getpointrec(i, c);
                    MapCoord2Screen(coord->y, coord->x, &x, &y, true);

                    x1 = x0;
                    y1 = y0;
                    x2 = x;
                    y2 = y;

                    if (clipline(&x1, &y1, &x2, &y2, window.topLeft.x,
                          window.topLeft.x + window.extent.x - 1) &&
                        clipline(&y1, &x1, &y2, &x2, window.topLeft.y,
                          window.topLeft.y + window.extent.y - 1)) {

                      if (m == 0 ||
                          polygon[m-1].x != x1 || polygon[m-1].y != y1) {
                        polygon[m].x = x1;
                        polygon[m].y = y1;
                        m++;
                      }
                      polygon[m].x = x2;
                      polygon[m].y = y2;
                      m++;
                    } else {
                      UInt16 q1 = quadrant(a, x0, y0);
                      UInt16 q2 = quadrant(a, x, y);

                      if (q1 != q2) {
                        if ((q1 == 0 && q2 == 1) || (q1 == 1 && q2 == 0)) {
                          polygon[m].x = window.topLeft.x + window.extent.x-1;
                          polygon[m].y = window.topLeft.y;
                          m++;
                        } else if ((q1 == 1 && q2 == 2) ||
                                   (q1 == 2 && q2 == 1)) {
                          polygon[m].x = window.topLeft.x + window.extent.x-1;
                          polygon[m].y = window.topLeft.y + window.extent.y-1;
                          m++;
                        } else if ((q1 == 2 && q2 == 3) ||
                                   (q1 == 3 && q2 == 2)) {
                          polygon[m].x = window.topLeft.x;
                          polygon[m].y = window.topLeft.y + window.extent.y-1;
                          m++;
                        } else if ((q1 == 3 && q2 == 0) ||
                                   (q1 == 0 && q2 == 3)) {
                          polygon[m].x = window.topLeft.x;
                          polygon[m].y = window.topLeft.y;
                          m++;
                        }
                      }
                    }

                    x0 = x;
                    y0 = y;
                  }

                  MapFillPolygon(m, polygon);
                  MemPtrFree(polygon);

                } else {
                  mind = window.extent.x / 50;
                  mind *= mind;
                  maxd = 0;
                  mx = my = -1;
                  angle = 0;

                  for (c++; c < n; c++) {
                    coord = getpointrec(i, c);
                    MapCoord2Screen(coord->y, coord->x, &x, &y, true);

                    x1 = x0;
                    y1 = y0;
                    x2 = x;
                    y2 = y;

                    if (clipline(&x1, &y1, &x2, &y2, window.topLeft.x,
                          window.topLeft.x + window.extent.x - 1) &&
                        clipline(&y1, &x1, &y2, &x2, window.topLeft.y,
                          window.topLeft.y + window.extent.y - 1)) {
                      MapDrawLine(x1, y1, x2, y2, width, lc, lc2);

                      if (named && hd) {
                        dseg = (x2-x1)*(x2-x1) + (y2-y1)*(y2-y1);

                        if (dseg > maxd && dseg > mind) {
                          maxd = dseg;
                          angle = atan2(y0 - y, x - x0);
                          if (angle > sys_pi()/2)
                            angle -= sys_pi();
                          else if (angle < -sys_pi()/2)
                            angle += sys_pi();
                          mx = (x1 + x2) / 2;
                          my = (y1 + y2) / 2;
                        }
                      }
                    }

                    x0 = x;
                    y0 = y;
                  }

                  if (named && hd && mx != -1) {
                    name = gettextrec(i, k);
                    LabelAdd(MAP_PLINE, mx, my, font, tc, angle, name);
                  }
                }
              }
            }
          }
          MemPtrFree(visited);
      }
    }
  }

  if ((n = DmNumRecords(ptRef)) > 0) {
    if (map) {
      rgb.r = map->textcolor.r;
      rgb.g = map->textcolor.g;
      rgb.b = map->textcolor.b;
    } else {
      rgb.r = 0;
      rgb.g = 0;
      rgb.b = 0;
    }
    tc = WinRGBToIndex(&rgb);

    for (i = 0; i < n; i++) {
      if ((p = (WaypointType *)DbOpenRec(ptRef, i, &err)) != NULL) {
        MapGrad2Coord(p->coord.latitude, p->coord.longitude, &wlat, &wlon);
        if (MapCoord2Screen(wlat, wlon, &x, &y, false)) {
          if (findLayer == WAYPOINTS_LAYER && selectedObj == i) {
            sel.type = MAP_POINT;
            sel.x = x;
            sel.y = y;
            sel.font = font;
            sel.color = tc;
            sel.label = p->name;
            sel.layer = hsymbolId + GetSymbolIndex(p->symbol);
          } else
            DrawSymbol(hsymbolId + GetSymbolIndex(p->symbol), x, y, true,
              p->name, font, tc, false);
        }
        DbCloseRec(ptRef, i, (char *)p);
      }
    }
  }

  MapDrawRoute(storedColor);

  if (storedLog)
    MapDrawLog(storedLog, storedColor, 2048, true);

  if (currentLog)
    MapDrawLog(currentLog, currentColor, 2048, false);

  if (sel.type == MAP_POINT)
    DrawSymbol(sel.layer, sel.x, sel.y, true, sel.label, sel.font,
     sel.color, false);

  LabelFinish(hd);

  if (onscreen) {
    if (target) {
      MapCoord2Screen(target_lat, target_lon, &tar_x, &tar_y, true);

      x1 = pos_x;
      y1 = pos_y;
      x2 = tar_x;
      y2 = tar_y;

      if (clipline(&x1, &y1, &x2, &y2, window.topLeft.x,
            window.topLeft.x + window.extent.x - 1) &&
          clipline(&y1, &x1, &y2, &x2, window.topLeft.y,
            window.topLeft.y + window.extent.y - 1))
        MapDrawLine(x1, y1, x2, y2, 3, targetColor, targetColor);
    }

    WinSetForeColor(positionColor);
    if (pos_speed > 0.0)
      DrawArrow(pos_x, pos_y, hd ? 18 : 14, hd ? 8 : 6,
        pos_course, false, true);
    MapDrawDot(pos_x, pos_y);
  }

  WinSetTextColor(oldt);
  WinSetBackColor(oldb);
  WinSetForeColor(oldc);
  FntSetFont(oldf);
  WinResetClip();
  WinSetDrawWindow(oldWh);

  if (hd) WinSetCoordinateSystem(kCoordinatesStandard);
}

static UInt16 quadrant(double a, Int32 x, Int32 y)
{
  Int32 x0, y0, dx, dy, xc, yc, yt;
  Int16 q_0_3, q_0_1;

  x0 = window.topLeft.x;
  y0 = window.topLeft.y;
  dx = window.extent.x;
  dy = window.extent.y;
  xc = x0 + dx/2;
  yc = y0 + dy/2;

  yt = yc + (-a) * (x - xc);
  q_0_3 = y < yt;

  yt = yc + a * (x - xc);
  q_0_1 = y < yt;

  if (q_0_3) {
    if (q_0_1)
      return 0;
    return 3;
  }
  if (q_0_1)
    return 1;
  return 2;
}

static MapPOIType *getpdatarec(UInt16 layer, UInt16 c)
{
  UInt16 j = c / 16360;	// MAX_RECSIZE / 4
  MapPOIType *pdatarec = (MapPOIType *)datarec[layer][j];
  return &pdatarec[c % 16360];
}

static MapPlineType *getdatarec(UInt16 layer, UInt16 c)
{
  UInt16 j = c / 16360;	// MAX_RECSIZE / 4
  return &datarec[layer][j][c % 16360];
}

static MapICoordType *getpointrec(UInt16 layer, UInt16 c)
{
  UInt16 j = c / 8180;	// MAX_RECSIZE / 8
  return &pointrec[layer][j][c % 8180];
}

static MapPOIIndexType *getpoirec(UInt16 layer, UInt16 c)
{
  UInt16 j = c / 3272;	// MAX_RECSIZE / 20
  return &poirec[layer][j][c % 3272];
}

static char *gettextrec(UInt16 layer, UInt16 obj)
{
  UInt32 i = indexrec[layer][obj / 16360][obj % 16360];
  char *s = getitextrec(layer, i);
  return s[0] >0 && s[0] < 32 ? s+1 : s;
}

static char *getitextrec(UInt16 layer, UInt32 i)
{
  static char text[MAP_OBJNAME];
  char *s;
  UInt32 j, record, offset;

  record = i / MAX_RECSIZE;
  offset = i % MAX_RECSIZE;
  s = &textrec[layer][record][offset];

  if ((offset+MAP_OBJNAME) <= MAX_RECSIZE)
    return s;

  for (i = 0, j = 0; s[j]; i++, j++) {
    if ((offset+j) == MAX_RECSIZE) {
      s = &textrec[layer][record+1][0];
      offset = 0;
      j = 0;
    }
    text[i] = s[j];
  }
  text[i] = 0;

  return text;
}

static void getpline(UInt16 symbol, Int16 zoom, Int16 *c, Int16 *w)
{
  UInt8 r, g, b;
  RGBColorType rgb;

  switch (symbol & 0x7F) {	// Tracksource has streets with 0x82
    case 0x01: r = 0; g = 0; b = 255; *w = 3; break;       // Blue Major Hwy
    case 0x02: r = 255; g = 0; b = 0; *w = 3; break;       // Red Major Hwy
    case 0x03: r = 255; g = 0; b = 0; *w = 2; break;       // Principal road
    case 0x04: r = 0; g = 0; b = 0; *w = 2; break;         // Principal street
    case 0x05: r = 0; g = 0; b = 0; *w = 1; break;         // Paved street
    case 0x06: r = 128; g = 128; b = 128; *w = 1; break;   // Road
    case 0x07: r = 50; g = 50; b = 50; *w = 1; break;      // Alley
    case 0x08: r = 128; g = 128; b = 128; *w = 3; break;   // Ramp
    case 0x09: r = 128; g = 128; b = 128; *w = 3; break;   // Ramp
    case 0x0A: r = 128; g = 128; b = 128; *w = 1; break;   // Unpaved road
    case 0x0B: r = 0; g = 0; b = 0; *w = 3; break;         // Major Hwy conn.
    case 0x0C: r = 25; g = 25; b = 25; *w = 1; break;      // Roundabout
    case 0x14: r = 0; g = 0; b = 0; *w = 1; break;         // Railway
    case 0x15: r = 15; g = 15; b = 15; *w = 1; break;      // Shoreline
    case 0x16: r = 30; g = 30; b = 30; *w = 1; break;      // Trail
    case 0x18: r = 81; g = 168; b = 255; *w = 1; break;    // Stream
    case 0x19: r = 192; g = 192; b = 192; *w = 1; break;   // Time zone
    case 0x1A: r = 0; g = 0; b = 0; *w = 1; break;         // Ferry line
    case 0x1B: r = 0; g = 0; b = 0; *w = 1; break;         // Ferry line
    case 0x1C: r = 128; g = 255; b = 255; *w = 1; break;   // Interstate bound.
    case 0x1D: r = 128; g = 128; b = 192; *w = 1; break;   // County boundary
    case 0x1E: r = 255; g = 255; b = 128; *w = 1; break;   // Int. boundary
    case 0x1F: r = 0; g = 200; b = 255; *w = 1; break;     // River
    case 0x20: r = 144; g = 112; b = 64; *w = 1; break;    // Contour (main)
    case 0x21: r = 144; g = 112; b = 64; *w = 1; break;    // Contour (second.)
    case 0x22: r = 144; g = 112; b = 64; *w = 1; break;    // Contour (detail)
    case 0x23: r = 0; g = 248; b = 255; *w = 1; break;     // Depth (main)
    case 0x24: r = 0; g = 248; b = 255; *w = 1; break;     // Depth (secondary)
    case 0x25: r = 0; g = 248; b = 255; *w = 1; break;     // Depth (detail)
    case 0x26: r = 0; g = 200; b = 245; *w = 1; break;     // Intermitent river
    case 0x27: r = 60; g = 60; b = 60; *w = 1; break;      // Airport runaway
    case 0x28: r = 70; g = 70; b = 70; *w = 1; break;      // Pipeline
    case 0x29: r = 80; g = 80; b = 80; *w = 1; break;      // Powerline
    case 0x2A: r = 10; g = 10; b = 10; *w = 1; break;      // Marine boundary
    case 0x2B: r = 255; g = 100; b = 100; *w = 1; break;   // Marine hazard
    default  : *w = 0; return;
  }

  rgb.r = r;
  rgb.g = g;
  rgb.b = b;

  *c = WinRGBToIndex(&rgb);
}

static void getregion(UInt16 symbol, Int16 zoom, Int16 *c, Int16 *w)
{
  UInt8 r, g, b;
  RGBColorType rgb;

  switch (symbol & 0x7F) {
    case 0x01:
    case 0x02:
    case 0x03: r = 192; g = 192; b = 192; break; // Metropolitan area
    case 0x04: r = 64; g = 128; b = 128; break;	 // Military
    case 0x05: r = 243; g = 243; b = 243; break; // Parking lot
    case 0x06: r = 241; g = 241; b = 241; break; // Parking garage
    case 0x07: r = 239; g = 239; b = 239; break; // Airport area
    case 0x08: r = 237; g = 237; b = 237; break; // Shopping center
    case 0x09: r = 235; g = 235; b = 235; break; // Marina
    case 0x0A: r = 233; g = 233; b = 233; break; // University
    case 0x0B: r = 231; g = 231; b = 231; break; // Hospital
    case 0x0C: r = 229; g = 229; b = 229; break; // Industrial area
    case 0x0D: r = 255; g = 150; b = 100; break; // Reservation
    case 0x0E: r = 128; g = 128; b = 128; break; // Airport runaway
    case 0x14:
    case 0x15:
    case 0x16: r = 160; g = 255; b = 160; break; // National park
    case 0x17: r = 0; g = 183; b = 0; break;     // City park
    case 0x18: r = 0; g = 193; b = 0; break;     // Golf
    case 0x19: r = 0; g = 198; b = 0; break;     // Sport
    case 0x1A: r = 255; g = 194; b = 166; break; // Cemetery area
    case 0x1E:
    case 0x1F:
    case 0x20: r = 0; g = 203; b = 0; break;     // State park
    case 0x28: r = 0; g = 128; b = 255; break;   // Ocean
    case 0x29: r = 0; g = 200; b = 247; break;   // Blue (?)
    case 0x32: r = 0; g = 200; b = 249; break;   // Sea
    case 0x3B: r = 0; g = 200; b = 247; break;   // Blue (?)
    case 0x3C:
    case 0x3D:
    case 0x3E:
    case 0x3F:
    case 0x40:
    case 0x41:
    case 0x42:
    case 0x43:
    case 0x44: r = 0; g = 200; b = 253; break;   // Lake
    case 0x45: r = 0; g = 200; b = 247; break;   // Blue (?)
    case 0x46:
    case 0x47:
    case 0x48:
    case 0x49: r = 0; g = 200; b = 255; break;   // River
    case 0x4C: r = 0; g = 200; b = 245; break;   // Intermitent river
    case 0x4D: r = 202; g = 199; b = 233; break; // Glacier
    //case 0x4E: Orchard
    //case 0x4F: Scrub
    case 0x50: r = 151; g = 130; b = 102; break; // Woods
    case 0x51: r = 0; g = 248; b = 255; break;   // Wetland
    //case 0x52: Tundra
    default  : *w = 0; return;
  }

  *w = 1;
  rgb.r = r;
  rgb.g = g;
  rgb.b = b;
  *c = WinRGBToIndex(&rgb);
}

static Boolean clipline(Int32 *x0, Int32 *y0, Int32 *x1, Int32 *y1,
                        Int32 mindim, Int32 maxdim)
{
  double m; // gradient of line

  if (*x0 < mindim) { // start of line is left of window
    if (*x1 < mindim) // as is the end, so the line never cuts the window
      return false;

    m = (*y1 - *y0) / (double)(*x1 - *x0);

    *y0 -= m * (*x0 - mindim);
    *x0 = mindim;

    if (*x1 > maxdim) {
      *y1 += m * (maxdim - *x1);
      *x1 = maxdim;
    }
    return true;
  }

  if (*x0 > maxdim) { // start of line is right of window complement of above
    if (*x1 > maxdim) // as is the end, so the line misses the window
      return false;

    m = (*y1 - *y0) / (double)(*x1 - *x0);

    *y0 += m * (maxdim - *x0);
    *x0 = maxdim;

    if (*x1 < mindim) {
      *y1 -= m * (*x1 - mindim);
      *x1 = mindim;
    }
    return true;
  }

  // the final case - the start of the line is inside the window
  if (*x1 > maxdim) { // other end is outside to the right
    m = (*y1 - *y0) / (double)(*x1 - *x0);

    *y1 += m * (maxdim - *x1);
    *x1 = maxdim;
    return true;
  }

  if (*x1 < mindim) { // other end is outside to the left
    m = (*y1 - *y0) / (double)(*x1 - *x0);

    *y1 -= m * (*x1 - mindim);
    *x1 = mindim;
    return true;
  }

  // only get here if both points are inside the window
  return true;
}

void MapSetTarget(double lat, double lon)
{
  MapGrad2Coord(lat, lon, &target_lat, &target_lon);
  target = true;
}

void MapResetTarget(void)
{
  target = false;
}

void MapDrawScale(double d, Int16 sx, Int16 sy)
{
  UInt16 x, y, dx, dy, ds, n1, n2;
  FontID old;
  char buf[32], *unit;
  RectangleType rect;
  AppPrefs *prefs = GetPrefs();

  // d is screen width in KM

  d /= 4.0; // scale is 1/4 of screen width

  if (prefs->unit_system == UNIT_METRIC) {
    if (d < 1.0) {
      d *= 1000.0;
      n1 = 1, n2 = 0;
      unit = "m";
    } else if (d < 10.0) {
      n1 = 1, n2 = 0;
      unit = "km";
    } else {
      n1 = 1, n2 = 0;
      unit = "km";
    }
  } else {
    d *= KM_TO_MILE;
    if (d < 1.0) {
      d *= 1000.0;
      n1 = 1, n2 = 0;
      unit = "ft";
    } else if (d < 10.0) {
      n1 = 1, n2 = 1;
      unit = "mi";
    } else {
      n1 = 1, n2 = 1;
      unit = "mi";
    }
  }

  if (hd) WinSetCoordinateSystem(kCoordinatesDouble);

  x = window.topLeft.x;
  y = window.topLeft.y + window.extent.y;

  old = FntSetFont(stdFont);
  fStrPrintF(buf, d, n1, n2);
  dy = FntCharHeight();
  ds = FntCharsWidth(buf, StrLen(buf));
  dx = sx/4;
  dy = sy-y;

  RctSetRectangle(&rect, x, y, 3*dx/2, dy);
  WinSetClip(&rect);
  WinEraseRectangle(&rect, 0);

  WinDrawChars(buf,  StrLen(buf),  x+(dx-ds)/2, y-1);
  WinDrawChars(unit, StrLen(unit), x+dx+4,      y-1);
  WinDrawLine(x,      y+dy/2, x,      y+dy-1);
  WinDrawLine(x+dx-1, y+dy/2, x+dx-1, y+dy-1);
  WinDrawLine(x,      y+dy-1, x+dx-1, y+dy-1);

  FntSetFont(old);
  WinResetClip();
  if (hd) WinSetCoordinateSystem(kCoordinatesStandard);
}

void MapInvalid(void)
{
  invalidMap = true;
}

static Int16 MapCompareProximity(void *e1, void *e2, Int32 other)
{
  MapFilter *p1 = (MapFilter *)e1;
  MapFilter *p2 = (MapFilter *)e2;
  if (p1->distance < p2->distance) return -1;
  if (p1->distance > p2->distance) return 1;
  return 0;
}

static void MapFillProximity(UInt16 i, UInt16 cat)
{
  MapLayerType *layer;
  MapPOIType *poi;
  MapIBoundingType *bounds;
  Int16 xr, yr, nrx, nry, xr0, xr1, yr0, yr1, ir, incr;
  UInt16 j, obj, firstobj, nobjs;
  Int32 n, r, lat0, lat1, lon0, lon1, dx, dy;
  double d;

  layer = &map->layer[i];
  numfilter = 0;

  nrx = layer->xregions;
  nry = layer->yregions;
  bounds = (MapIBoundingType *)(&layer->bounds);
  dx = bounds->x1 - bounds->x0;
  dy = bounds->y1 - bounds->y0;

  r = (Int32)MAX_DISTANCE;	// small error: 30 -> 32
  lat0 = pos_lat - r/2;
  lat1 = pos_lat + r/2;
  lon0 = pos_lon - r/2;
  lon1 = pos_lon + r/2;

  xr0 = ((lon0 - bounds->x0) * nrx) / dx;
  xr1 = ((lon1 - bounds->x0) * nrx) / dx;
  yr0 = ((lat0 - bounds->y0) * nry) / dy;
  yr1 = ((lat1 - bounds->y0) * nry) / dy;

  if (xr0 < 0) xr0 = 0;
  if (xr1 >= nrx) xr1 = nrx-1;
  if (yr0 < 0) yr0 = 0;
  if (yr1 >= nry) yr1 = nry-1;

  incr = 2*(nrx-1-xr1+xr0);

  // count how many objects are there inside radius r

  for (n = 0, ir = 2*(yr0*nrx+xr0), yr = yr0; yr <= yr1; yr++, ir += incr)
    for (xr = xr0; xr <= xr1; xr++)
      n += regionrec[i][ir++];

  if (n > MAX_RECSIZE/sizeof(MapFilter))
    n = MAX_RECSIZE/sizeof(MapFilter);

  if (n > 2048)	// XXX
    n = 2048;

  if (filter)
    MemPtrFree(filter);

  if ((filter = MemPtrNew(n * sizeof(MapFilter))) == NULL)
    return;

  MemSet(filter, sizeof(n * sizeof(MapFilter)), 0);

  for (ir = 2*(yr0*nrx+xr0), yr = yr0; yr <= yr1 && numfilter < n;
       yr++, ir += incr) {
    for (xr = xr0; xr <= xr1 && numfilter < n; xr++) {
      firstobj = regionrec[i][ir++];
      nobjs = regionrec[i][ir++];

      for (j = 0; j < nobjs && numfilter < n; j++) {
        obj = regionrec[i][firstobj+j];
        poi = getpdatarec(i, obj);
        if ((cat == 0 || poi == NULL || ((poi->type >> 8) == cat)) &&
             MapPointDistance(i, obj, &d)) {
          filter[numfilter].obj = obj;
          filter[numfilter].distance = d;
          numfilter++;
        }
      }
    }
  }

  SysQSort(filter, numfilter, sizeof(MapFilter), MapCompareProximity, 0);
}

static void MapFillCategory(UInt16 i, UInt16 cat)
{
  MapLayerType *layer;
  UInt16 j, nobjs, offset;
  MapPOIType *poi;
  UInt8 *bsymbol;

  layer = &map->layer[i];
  nobjs = layer->nobjs;
  numfilter = 0;
  offset = 0;

  switch (layer->type) {
    case MAP_POINT:
      for (j = 0; j < nobjs; j++) {
        poi = getpdatarec(i, j);
        if (cat == 0 || poi == NULL || (poi->type >> 8) == cat) {
          DmWrite(tmpFilter, offset, &j, sizeof(UInt16));
          numfilter++;
          offset += sizeof(UInt16);
        }
      }
      break;
    case MAP_PLINE:
      bsymbol = symbolrec[i][0];
      for (j = 0; j < nobjs; j++) {
        if (cat == 0 || bsymbol == NULL || GetLineCat(bsymbol[j]) == cat) {
          DmWrite(tmpFilter, offset, &j, sizeof(UInt16));
          numfilter++;
          offset += sizeof(UInt16);
        }
      }
      break;
    case MAP_REGION:
      bsymbol = symbolrec[i][0];
      for (j = 0; j < nobjs; j++) {
        if (cat == 0 || bsymbol == NULL || GetAreaCat(bsymbol[j]) == cat) {
          DmWrite(tmpFilter, offset, &j, sizeof(UInt16));
          numfilter++;
          offset += sizeof(UInt16);
        }
      }
  }
}

static UInt16 GetLineCat(UInt16 symbol)
{
  switch (symbol & 0x7F) {
    case 0x01: return 1; // Blue Major Hwy
    case 0x02: return 1; // Red Major Hwy
    case 0x03: return 1; // Principal road
    case 0x04: return 1; // Principal street
    case 0x05: return 1; // Paved street
    case 0x06: return 1; // Road
    case 0x07: return 1; // Alley
    case 0x08: return 1; // Ramp
    case 0x09: return 1; // Ramp
    case 0x0A: return 1; // Unpaved road
    case 0x0B: return 1; // Major Hwy conn.
    case 0x0C: return 1; // Roundabout
    case 0x14: return 2; // Railway
    case 0x15: return 5; // Shoreline
    case 0x16: return 4; // Trail
    case 0x18: return 5; // Stream
    case 0x19: return 6; // Time zone
    case 0x1A: return 3; // Ferry line
    case 0x1B: return 3; // Ferry line
    case 0x1C: return 6; // Interstate bound.
    case 0x1D: return 6; // County boundary
    case 0x1E: return 6; // Int. boundary
    case 0x1F: return 5; // River
    case 0x20: return 4; // Contour (main)
    case 0x21: return 4; // Contour (second.)
    case 0x22: return 4; // Contour (detail)
    case 0x23: return 5; // Depth (main)
    case 0x24: return 5; // Depth (secondary)
    case 0x25: return 5; // Depth (detail)
    case 0x26: return 5; // Intermitent river
    case 0x27: return 1; // Airport runaway
    case 0x28: return 4; // Pipeline
    case 0x29: return 4; // Powerline
    case 0x2A: return 5; // Marine boundary
    case 0x2B: return 5; // Marine hazard
  }
  return 0;
}

static UInt16 GetAreaCat(UInt16 symbol)
{
  switch (symbol & 0x7F) {
    case 0x01:
    case 0x02:
    case 0x03: return 1; // Metropolitan area
    case 0x04: return 1; // Military
    case 0x05: return 1; // Parking lot
    case 0x06: return 1; // Parking garage
    case 0x07: return 1; // Airport area
    case 0x08: return 1; // Shopping center
    case 0x09: return 4; // Marina
    case 0x0A: return 1; // University
    case 0x0B: return 1; // Hospital
    case 0x0C: return 1; // Industrial area
    case 0x0D: return 3; // Reservation
    case 0x0E: return 1; // Airport runaway
    case 0x14:
    case 0x15:
    case 0x16: return 2; // National park
    case 0x17: return 2; // City park
    case 0x18: return 2; // Golf
    case 0x19: return 2; // Sport
    case 0x1A: return 1; // Cemetery area
    case 0x1E:
    case 0x1F:
    case 0x20: return 2; // State park
    case 0x28: return 4; // Ocean
    case 0x29: return 4; // Blue (?)
    case 0x32: return 4; // Sea
    case 0x3B: return 4; // Blue (?)
    case 0x3C:
    case 0x3D:
    case 0x3E:
    case 0x3F:
    case 0x40:
    case 0x41:
    case 0x42:
    case 0x43:
    case 0x44: return 4; // Lake
    case 0x45: return 4; // Blue (?)
    case 0x46:
    case 0x47:
    case 0x48:
    case 0x49: return 4; // River
    case 0x4C: return 4; // Intermitent river
    case 0x4D: return 3; // Glacier
    case 0x4E: return 3; // Orchard
    case 0x4F: return 3; // Scrub
    case 0x50: return 3; // Woods
    case 0x51: return 4; // Wetland
    case 0x52: return 3; // Tundra
  }
  return 0;
}

static void FeatureDrawCell(void *t, Int16 row, Int16 col, RectangleType *rect)
{
  TableType *tbl;
  UInt16 layer, objrow, obj, symbol, *wsymbol, len, dx;
  UInt16 tc = 0, fg = 0, bg = 0;
  UInt32 dynamic;
  Boolean hassymbol, draw;
  char *s, buf[32];
  float *d;
  double d1 = 0;
  AppPrefs *prefs;

  tbl = t;
  objrow = TblGetRowData(tbl, row);

  s = NULL;
  d1 = 0;
  symbol = 0;
  hassymbol = false;
  buf[0] = 0;
  prefs = GetPrefs();

  if (findLayer == WAYPOINTS_LAYER) {
    obj = objrow;

    if (obj < GetRecNum(0)) {
      s = GetRecName(0, obj);
      symbol = hsymbolId + GetSymbolIndex(GetRecData(0, obj));
      hassymbol = true;

      if (prefs->proximity_sort) {
        dynamic = GetRecDynamic(0, obj);
        d = (float *)(&dynamic);
        FormatDistance(buf, *d);
      }
    }
  } else if (findLayer == ROUTES_LAYER) {
    obj = objrow;

    if (obj < GetRecNum(1))
      s = GetRecName(1, obj);

  } else if (findLayer == TRACKS_LAYER) {
    obj = objrow;

    if (obj < GetTrackNum())
      s = GetTrackName(obj);
  } else {
    layer = layerNumber[findLayer];

    if (map->layer[layer].type == MAP_POINT) {
      draw = objrow < numfilter;
      if (draw) {
        if (prefs->proximity_sort) {
          obj = filter[objrow].obj;
          d1 = filter[objrow].distance;
        } else
          obj = tmpFilter[objrow];
      }
    } else {
      draw = objrow < numfilter;
      if (draw)
        obj = tmpFilter[objrow];
    }

    if (draw) {
      s = gettextrec(layer, obj);

      if (map->layer[layer].type == MAP_POINT) {
        if (map->layer[layer].flags & MAP_LAYER_SYMBOL) {
          wsymbol = obj < 32720 ?
            (UInt16 *)symbolrec[layer][0] : (UInt16 *)symbolrec[layer][1];
          symbol = hsymbolId + GetSymbolIndex(wsymbol[obj % 32720]);
          hassymbol = true;
        }
        if (prefs->proximity_sort)
          FormatDistance(buf, d1);
      }
    }
  }

  if (s) {
    WinSetClip(rect);

    if (objrow == tableRow) {
      tc = WinSetTextColor(UIColorGetTableEntryIndex(UIObjectSelectedForeground));
      fg = WinSetForeColor(UIColorGetTableEntryIndex(UIObjectSelectedFill));
      bg = WinSetBackColor(UIColorGetTableEntryIndex(UIObjectSelectedFill));
      WinFillRectangle(rect, 0);
    }

    if (buf[0]) {
      len = StrLen(buf);
      dx = FntCharsWidth(buf, len);
      WinPaintChars(buf, len,
        rect->topLeft.x + rect->extent.x - dx, rect->topLeft.y);
      rect->extent.x -= dx + FntCharWidth('W');
      WinSetClip(rect);
    }

    if (hd && hassymbol) {
      DrawBmp(symbol, rect->topLeft.x+9, rect->topLeft.y+9, true);
      WinPaintChars(s, StrLen(s), rect->topLeft.x+12, rect->topLeft.y);
    } else
      WinPaintChars(s, StrLen(s), rect->topLeft.x, rect->topLeft.y);

    if (objrow == tableRow) {
      WinSetTextColor(tc);
      WinSetForeColor(fg);
      WinSetBackColor(bg);
    }

    WinResetClip();
  }
}

static void MapFindRefresh(void)
{
  FormPtr frm;
  ListType *lst;
  TableType *tbl;
  ScrollBarType *scl;
  UInt16 i, size, ncats;
  AppPrefs *prefs;

  if (resettable) {
    tableTop = 0;
    tableRow = 0;
    selectedObj = 0;
    resettable = 0;
  }

  prefs = GetPrefs();
  frm = FrmGetActiveForm();
  lst = (ListPtr)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, catList));

  if (findLayer == WAYPOINTS_LAYER) {
    if (prefs->proximity_sort)
      BuildRecList(0, ptRef, getpointname, NULL, getpointdata,
        getpointdistance, comparedistance);
    else
      BuildRecList(0, ptRef, getpointname, NULL, getpointdata,
        NULL, comparename);

    size = GetRecNum(0);
    LstSetHeight(lst, 1);
    LstSetListChoices(lst, POICategories, 1);
    findCat = 0;

    FrmHideObject(frm, FrmGetObjectIndex(frm, saveBtn));

  } else if (findLayer == ROUTES_LAYER) {
    size = GetRecNum(1);
    LstSetHeight(lst, 1);
    LstSetListChoices(lst, POICategories, 1);
    findCat = 0;

    FrmHideObject(frm, FrmGetObjectIndex(frm, saveBtn));

  } else if (findLayer == TRACKS_LAYER) {
    size = GetTrackNum();
    LstSetHeight(lst, 1);
    LstSetListChoices(lst, POICategories, 1);
    findCat = 0;

    FrmHideObject(frm, FrmGetObjectIndex(frm, saveBtn));

  } else {
    size = 0;

    switch (map->layer[layerNumber[findLayer]].type) {
      case MAP_POINT:
        if (getpdatarec(layerNumber[findLayer], 0))
          ncats = MAX_POI_CATEGORIES;
        else {
          ncats = 1;
          findCat = 0;
        }
        if (prefs->proximity_sort) {
          MapFillProximity(layerNumber[findLayer], findCat);
          selectedObj = filter[tableRow].obj;
        } else {
          MapFillCategory(layerNumber[findLayer], findCat);
          selectedObj = tmpFilter[tableRow];
        }
        size = numfilter;

        LstSetHeight(lst, ncats);
        LstSetListChoices(lst, POICategories, ncats);
        FrmShowObject(frm, FrmGetObjectIndex(frm, saveBtn));
        break;

      case MAP_PLINE:
        if (symbolrec[layerNumber[findLayer]])
          ncats = MAX_LINE_CATEGORIES;
        else {
          ncats = 1;
          findCat = 0;
        }
        MapFillCategory(layerNumber[findLayer], findCat);
        selectedObj = tmpFilter[tableRow];
        size = numfilter;

        LstSetHeight(lst, ncats);
        LstSetListChoices(lst, LineCategories, ncats);
        FrmHideObject(frm, FrmGetObjectIndex(frm, saveBtn));
        break;

      case MAP_REGION:
        if (symbolrec[layerNumber[findLayer]])
          ncats = MAX_AREA_CATEGORIES;
        else {
          ncats = 1;
          findCat = 0;
        }
        MapFillCategory(layerNumber[findLayer], findCat);
        selectedObj = tmpFilter[tableRow];
        size = numfilter;

        LstSetHeight(lst, ncats);
        LstSetListChoices(lst, AreaCategories, ncats);
        FrmHideObject(frm, FrmGetObjectIndex(frm, saveBtn));
    }
  }

  LstSetSelection(lst, findCat);
  CtlSetLabel((ControlPtr)FrmGetObjectPtr(frm,
    FrmGetObjectIndex(frm, catCtl)), LstGetSelectionText(lst, findCat));

  tbl = (TableType *)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, featureTbl));
  scl = (ScrollBarType *)FrmGetObjectPtr(frm,FrmGetObjectIndex(frm,featureScl));

  if (size > mapFindRows) {
    SclSetScrollBar(scl, tableTop, 0, size-1, mapFindRows);
    TblHasScrollBar(tbl, true);
  } else {
    SclSetScrollBar(scl, 0, 0, 0, 0);
    TblHasScrollBar(tbl, false);
  }

  for (i = 0; i < mapFindRows; i++) {
    TblSetRowUsable(tbl, i, true);
    TblSetItemStyle(tbl, i, 0, customTableItem);
    TblSetRowData(tbl, i, tableTop+i);
  }

  TblSetColumnUsable(tbl, 0, true);
  TblSetCustomDrawProcedure(tbl, 0, FeatureDrawCell);

  TblMarkTableInvalid(tbl);
  TblRedrawTable(tbl);
}

void MapFindReset(void)
{
  findLayer = WAYPOINTS_LAYER;
  findCat = 0;
  tableRow = 0;
  tableTop = 0;
}

Boolean MapFindFormHandleEvent(EventPtr event)
{
  FormType *frm;
  ListType *lst;
  TableType *tbl;
  ScrollBarType *scl;
  UInt16 i, j, k, n, row, size, tmp, chr, layer;
  char *s;
  double clat, clon;
  MapPlineType *pline;
  MapICoordType *coord;
  WaypointType *p;
  TracklogType tlog;
  AppPrefs *prefs;
  FileHand f;
  Boolean handled, changed;
  Err err;

  handled = false;

  switch (event->eType) {
    case frmOpenEvent:
      if (dataRef && map) {
        for (i = 0, j = 3; i < map->nlayers; i++) {
          if ((map->layer[i].flags & MAP_LAYER_ACTIVE) &&
              (map->layer[i].flags & MAP_LAYER_NAMED) &&
              !(map->layer[i].flags & MAP_LAYER_LARGE)) {
            layerNumber[j] = i;
            layerName[j++] = map->layer[i].name;
          }
        }
        n = map->nlayers + 3;
      } else
        n = 3;

      prefs = GetPrefs();
      frm = FrmGetActiveForm();

      lst = (ListPtr)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, layerList));
      LstSetHeight(lst, n);
      LstSetListChoices(lst, layerName, n);
      LstSetSelection(lst, findLayer);
      CtlSetLabel((ControlPtr)FrmGetObjectPtr(frm,
        FrmGetObjectIndex(frm, layerCtl)), LstGetSelectionText(lst, findLayer));

      FrmSetControlValue(frm, FrmGetObjectIndex(frm, proxCtl),
        prefs->proximity_sort);

      MapFindRefresh();

      FrmDrawForm(frm);
      handled = true;
      break;

    case nilEvent:
      idle();
      handled = true;
      break;

    case sclRepeatEvent: 
    case sclExitEvent: 
      tableTop = event->data.sclRepeat.newValue;

      frm = FrmGetActiveForm();
      tbl = (TableType *)FrmGetObjectPtr(frm,
        FrmGetObjectIndex(frm, featureTbl));

      for (row = 0; row < mapFindRows; row++)
	TblSetRowData(tbl, row, tableTop+row);

      TblMarkTableInvalid(tbl);
      TblRedrawTable(tbl);
      break;

    case keyDownEvent: 
      frm = FrmGetActiveForm();
      scl = (ScrollBarType *)FrmGetObjectPtr(frm,
        FrmGetObjectIndex(frm, featureScl));
      tbl = (TableType *)FrmGetObjectPtr(frm,
        FrmGetObjectIndex(frm, featureTbl));

      if (findLayer == WAYPOINTS_LAYER)
        size = GetRecNum(0);
      else if (findLayer == ROUTES_LAYER)
        size = GetRecNum(1);
      else if (findLayer == TRACKS_LAYER)
        size = GetTrackNum();
      else
        size = numfilter;

      prefs = GetPrefs();
      chr = event->data.keyDown.chr;

      if (event->data.keyDown.modifiers & commandKeyMask) {
        changed = false;

        switch (chr) {
          case pageDownChr:
            if (size > mapFindRows) {
              tableTop += mapFindRows;
              if (tableTop >= size)
                tableTop = size-1;
              changed = true;
            }
            handled = true;
            break;
          case pageUpChr:
            if (size > mapFindRows) {
              if (tableTop >= mapFindRows)
                tableTop -= mapFindRows;
              else
                tableTop = 0;
              changed = true;
            }
            handled = true;
        }

        if (changed) {
          for (row = 0; row < mapFindRows; row++)
	    TblSetRowData(tbl, row, tableTop+row);

          SclSetScrollBar(scl, tableTop, 0, size-1, mapFindRows);
          TblMarkTableInvalid(tbl);
          TblRedrawTable(tbl);
        }
      } else if (size && !prefs->proximity_sort && TxtCharIsAlNum(chr)) {
        handled = true;

        if (chr >= 'a' && chr <= 'z')
          chr &= 0xdf;

        layer = layerNumber[findLayer];

        for (i = 0; i < size; i++) {
          s = NULL;

          switch (findLayer) {
            case WAYPOINTS_LAYER:
              s = GetRecName(0, i);
              break;
            case ROUTES_LAYER:
              s = GetRecName(1, i);
              break;
            case TRACKS_LAYER:
              s = GetTrackName(i);
              break;
            default:
              s = gettextrec(layer, tmpFilter[i]);
          }

          if (s && (s[0] & 0xdf) == chr) {
            if (tableRow >= tableTop && tableRow < tableTop+mapFindRows)
              TblMarkRowInvalid(tbl, tableRow-tableTop);

            if (i >= tableTop && i < tableTop+mapFindRows)
              TblMarkRowInvalid(tbl, i-tableTop);
            else {
              if (i < mapFindRows)
                tableTop = 0;
              else
                tableTop = i - mapFindRows/2;

              for (row = 0; row < mapFindRows; row++)
	        TblSetRowData(tbl, row, tableTop+row);

              SclSetScrollBar(scl, tableTop, 0, size-1, mapFindRows);
              TblMarkTableInvalid(tbl);
            }

            tableRow = i;
            selectedObj = i;
            TblRedrawTable(tbl);
            break;
          }
        }
      }
      break;

    case tblEnterEvent: 
      tbl = event->data.tblSelect.pTable;
      tmp = TblGetRowData(tbl, event->data.tblSelect.row);

      if (findLayer == WAYPOINTS_LAYER)
        size = GetRecNum(0);
      else if (findLayer == ROUTES_LAYER)
        size = GetRecNum(1);
      else if (findLayer == TRACKS_LAYER)
        size = GetTrackNum();
      else
        size = numfilter;

      if (tmp < size) {
        TblMarkRowInvalid(tbl, event->data.tblSelect.row);
        if (tableRow >= tableTop && tableRow < tableTop+mapFindRows)
          TblMarkRowInvalid(tbl, tableRow-tableTop);

        tableRow = tableTop+event->data.tblSelect.row;

        if (findLayer == WAYPOINTS_LAYER)
          selectedObj = tmp;
        else if (findLayer == ROUTES_LAYER)
          selectedObj = tmp;
        else if (findLayer == TRACKS_LAYER)
          selectedObj = tmp;
        else {
          if (map->layer[layerNumber[findLayer]].type == MAP_POINT) {
            prefs = GetPrefs();
            if (prefs->proximity_sort)
              selectedObj = filter[tmp].obj;
            else
              selectedObj = tmpFilter[tmp];
          } else
            selectedObj = tmpFilter[tmp];
        }
        TblRedrawTable(tbl);
      }

      handled = true;
      break;

    case popSelectEvent:
      switch (event->data.popSelect.listID) {
        case layerList:
          if (findLayer != event->data.popSelect.selection) {
            findLayer = event->data.popSelect.selection;
            findCat = 0;
            resettable = 1;
            MapFindRefresh();
          }
          break;
        case catList:
          if (findCat != event->data.popSelect.selection) {
            findCat = event->data.popSelect.selection;
            resettable = 1;
            MapFindRefresh();
          }
      }
      handled = false;
      break;

    case ctlSelectEvent:
      switch (event->data.ctlSelect.controlID) {
        case centerBtn:
          size = 0;

          if (findLayer == WAYPOINTS_LAYER) {
            size = GetRecNum(0);

            if (size > 0) {
              i = GetRecIndex(0, selectedObj);
              if ((p = (WaypointType *)DbOpenRec(ptRef, i, &err)) != NULL) {
                MapGrad2Coord(p->coord.latitude, p->coord.longitude,
                  &center_lat, &center_lon);
                DbCloseRec(ptRef, i, (char *)p);
              }
            }
          } else if (findLayer == ROUTES_LAYER) {
            size = GetRecNum(1);

            if (size > 0) {
              RouteType rte;
              StrCopy(rte.ident, GetRecName(1, selectedObj));
              GetRouteCenter(&rte, &clat, &clon);
              MapGrad2Coord(clat, clon, &center_lat, &center_lon);
            }

          } else if (findLayer == TRACKS_LAYER) {
            size = GetTrackNum();

            if (size > 0) {
              f = OpenLog(GetTrackName(selectedObj), AppID, LogType,
                    fileModeReadOnly);
              if (f) {
                clat = clon = 0;

                FileSeek(f, 0, fileOriginBeginning);
                for (j = 0;; j++) {
                  if (ReadLog(f, &tlog, sizeof(TracklogType)) != 0)
                    break;
                  clat += tlog.latitude;
                  clon += tlog.longitude;
                }
                CloseLog(f);

                if (j) {
                  clat /= j;
                  clon /= j;
                  MapGrad2Coord(clat, clon, &center_lat, &center_lon);
                  SelectTrack(selectedObj);
                }
              }
            }
          } else {
            switch (map->layer[layerNumber[findLayer]].type) {
              case MAP_POINT:
                size = numfilter;
                if (size > 0) {
                  coord = getpointrec(layerNumber[findLayer], selectedObj);
                  center_lat = coord->y;
                  center_lon = coord->x;
                }
                break;
              case MAP_PLINE:
              case MAP_REGION:
                size = numfilter;
                if (size > 0) {
                  pline = getdatarec(layerNumber[findLayer], selectedObj);
                  if (pline->npoints > 0) {
                    center_lat = 0;
                    center_lon = 0;
                    k = pline->firstpoint;
                    for (j = 0; j < pline->npoints; j++) {
                      coord = getpointrec(layerNumber[findLayer], k++);
                      center_lat += coord->y;
                      center_lon += coord->x;
                    }
                    center_lat /= pline->npoints;
                    center_lon /= pline->npoints;
                  }
                }
            }
          }

          if (size > 0) {
            locked = false;
            prefs = GetPrefs();
            prefs->locked = false;
            MapRecalc();
            MapInvalid();
          }
          PopForm();
          handled = true;
          break;
        case editBtn:
          switch (findLayer) {
            case WAYPOINTS_LAYER:
              if (GetRecNum(0))
                MapShowDetail(-1, selectedObj);
              break;
            case ROUTES_LAYER:
              if (GetRecNum(1))
                MapShowDetail(-2, selectedObj);
              break;
            case TRACKS_LAYER:
              if (GetTrackNum())
                MapShowDetail(-3, selectedObj);
              break;
            default:
              MapShowDetail(layerNumber[findLayer], selectedObj);
          }
          handled = true;
          break;
        case saveBtn:
          MapSavePoint(layerNumber[findLayer], selectedObj);
          handled = true;
          break;
        case proxCtl:
          prefs = GetPrefs();
          prefs->proximity_sort = !prefs->proximity_sort;

          if (findLayer == WAYPOINTS_LAYER)
            resettable = 1;
          else if (findLayer > TRACKS_LAYER)
            if (map->layer[layerNumber[findLayer]].type == MAP_POINT)
              resettable = 1;

          MapFindRefresh();
          handled = true;
          break;   
        case cancelBtn:
          MapInvalid();
          PopForm();
          handled = true;
      }
      break;

    default:
      break;
  }

  return handled;
}

static void MapSavePoint(Int16 layer, UInt16 i)
{
  char *name;
  UInt16 *wsymbol, symbol;
  MapICoordType *coord;
  double lat, lon;

  name = gettextrec(layer, i);
  wsymbol = i < 32720 ? (UInt16 *)symbolrec[layer][0] :
                        (UInt16 *)symbolrec[layer][1];
  symbol = wsymbol[i % 32720];
  coord = getpointrec(layer, i);
  MapCoord2Grad(coord->y, coord->x, &lat, &lon);

  SavePoint(name, symbol, lat, lon);
}

static void MapShowDetail(Int16 layer, UInt16 i)
{
  FormType *frm;
  Int16 type;
  UInt16 index, j, n, len;
  UInt8 cat, *bsymbol;
  char aux[32], *s;
  double lat, lon, side, length;
  MapPlineType *pline;
  MapPOIType *poi;
  MapPOIIndexType *poi_index;
  MapICoordType *coord;
  WaypointType *p;
  Err err;
  static char buf[256];

  type = layer >= 0 ? map->layer[layer].type : layer;
  buf[0] = 0;

  switch (type) {
    case -1:	// waypoints
      index = GetRecIndex(0, i);

      if ((p = (WaypointType *)DbOpenRec(ptRef, index, &err)) != NULL) {
        StrCopy(buf, p->name);

        if (p->comment[0]) {
          StrCat(buf, "\n");
          StrCat(buf, p->comment);
        }

        gStrPrintF(aux, p->coord.latitude);
        StrCat(buf, "\nLatitude: ");
        StrCat(buf, aux);

        gStrPrintF(aux, p->coord.longitude);
        StrCat(buf, "\nLongitude: ");
        StrCat(buf, aux);

        DbCloseRec(ptRef, index, (char *)p);
      }
      break;

    case -2:	// routes
      StrCopy(buf, GetRecName(1, i));
      GetRoute(GetRecIndex(1, i), NULL);
      if ((n = GetRecNum(2)) > 1) {
        StrPrintF(aux, "\nPoints: %d", n);
        StrCat(buf, aux);

        StrCat(buf, "\nFrom: ");
        StrCat(buf, GetRecName(2, 0));
        StrCat(buf, "\nTo: ");
        StrCat(buf, GetRecName(2, n-1));

        length = MapCalcLength(-2, i);
        FormatDistance(aux, length);
        StrCat(buf, "\nLength: ");
        StrCat(buf, aux);
      }
      break;

    case -3:	// tracks
      StrCopy(buf, GetTrackName(i));

      StrPrintF(aux, "\nPoints: %d",
        (UInt16)(GetTrackSize(i) / sizeof(TracklogType)));
      StrCat(buf, aux);

      length = MapCalcLength(-3, i);
      FormatDistance(aux, length);
      StrCat(buf, "\nLength: ");
      StrCat(buf, aux);
      break;

    case MAP_POINT:
      StrCopy(buf, gettextrec(layer, i));
      poi = getpdatarec(layer, i);

      if (poi) {
        cat = poi->type >> 8;
        //subcat = poi->type & 0xFF;

        if (cat > 0 && cat < MAX_POI_CATEGORIES) {
          StrCat(buf, "\n");
          StrCat(buf, POICategories[cat]);
        }
      }

      coord = getpointrec(layer, i);
      MapCoord2Grad(coord->y, coord->x, &lat, &lon);

      gStrPrintF(aux, lat);
      StrCat(buf, "\nLatitude: ");
      StrCat(buf, aux);

      gStrPrintF(aux, lon);
      StrCat(buf, "\nLongitude: ");
      StrCat(buf, aux);

      if (poi && poi->poiindex != 0xFFFF) {
        poi_index = getpoirec(layer, poi->poiindex);
        StrCat(buf, "\n");
        StrCat(buf, "\n");

        if (poi_index->street) {
          StrCat(buf, getitextrec(layer, poi_index->street));

          if (poi_index->number) {
            // special number formats:
            // 12- 3 : 12 # 3
            // 12-03 : 12 # 3
            // 12-13 : 12 APT 3
            // 12-23 : 12 BLDG 3
            // 12-33 : 12 DEPT 3
            // 12-43 : 12 FL 3
            // 12-53 : 12 RM 3
            // 12-63 : 12 STE 3
            // 12-73 : 12 UNIT 3

            StrCat(buf, ", ");
            s = getitextrec(layer, poi_index->number);
            len = StrLen(s);
            for (j = 0; s[j]; j++)
              if (s[j] == '-')
                break;

            if (j && s[j] == '-' && len >= 5 && len < 24 &&
                (s[j+1] == ' ' || (s[j+1] >= '0' && s[j+1] <= '7'))) {

              StrNCopy(aux, s, j);
              aux[j] = 0;
              StrCat(buf, aux);

              switch (s[j+1]) {
                case ' ':
                case '0': StrPrintF(aux, " # %s", &s[j+2]); break;
                case '1': StrPrintF(aux, " APT %s", &s[j+2]); break;
                case '2': StrPrintF(aux, " BLDG %s", &s[j+2]); break;
                case '3': StrPrintF(aux, " DEPT %s", &s[j+2]); break;
                case '4': StrPrintF(aux, " FL %s", &s[j+2]); break;
                case '5': StrPrintF(aux, " RM %s", &s[j+2]); break;
                case '6': StrPrintF(aux, " STE %s", &s[j+2]); break;
                case '7': StrPrintF(aux, " UNIT %s", &s[j+2]); break;
                default: aux[0] = 0;
              }
              StrCat(buf, aux);
            } else
              StrCat(buf, s);
          }
          StrCat(buf, "\n");
        }

        if (poi_index->city)
          StrCat(buf, getitextrec(layer, poi_index->city));

        if (poi_index->zip) {
          if (poi_index->city)
            StrCat(buf, " ");
          StrCat(buf, getitextrec(layer, poi_index->zip));
        }

        if (poi_index->city || poi_index->zip)
          StrCat(buf, "\n");

        if (poi_index->phone)
          StrCat(buf, getitextrec(layer, poi_index->phone));
      }
      break;

    case MAP_PLINE:
      StrCopy(buf, gettextrec(layer, i));
      bsymbol = symbolrec[layer][0];

      if (bsymbol) {
        cat = GetLineCat(bsymbol[i]);

        if (cat > 0 && cat < MAX_LINE_CATEGORIES) {
          StrCat(buf, "\n");
          StrCat(buf, LineCategories[cat]);
        }
      }

      pline = getdatarec(layer, i);
      StrPrintF(aux, "\nPoints: %d", pline->npoints);
      StrCat(buf, aux);

      length = MapCalcLength(layer, i);
      FormatDistance(aux, length);
      StrCat(buf, "\nLength: ");
      StrCat(buf, aux);
      break;

    case MAP_REGION:
      StrCopy(buf, gettextrec(layer, i));
      bsymbol = symbolrec[layer][0];

      if (bsymbol) {
        cat = GetAreaCat(bsymbol[i]);

        if (cat > 0 && cat < MAX_AREA_CATEGORIES) {
          StrCat(buf, "\n");
          StrCat(buf, AreaCategories[cat]);
        }
      }

      pline = getdatarec(layer, i);
      StrPrintF(aux, "\nPoints: %d", pline->npoints);
      StrCat(buf, aux);

      length = MapCalcLength(layer, i);
      FormatDistance(aux, length);
      StrCat(buf, "\nPerimeter: ");
      StrCat(buf, aux);

      side = MapCalcArea(layer, i);
      FormatArea(aux, side);
      StrCat(buf, "\nArea: ");
      StrCat(buf, aux);
  }

  frm = FrmInitForm(MapDetailForm);
  FldInsertStr(frm, detailFld, buf);
  FrmDoDialog(frm);
  FrmDeleteForm(frm);
}

static double MapCalcLength(Int16 layer, UInt16 obj)
{
  MapPlineType *pline;
  MapICoordType *coord;
  double x, y, x0 = 0, y0 = 0, rcx, rcy, rtx, rty, cosd, length;
  UInt16 i, first, n;
  TracklogType tlog;
  RoutePointType rpoint;
  DmOpenRef dbRef;
  FileHand f;
  Err err;

  if (layer == -2) {
    // routes
    first = 0;
    n = GetRecNum(2);
    dbRef = DbOpenByName(GetRecName(1, obj), dmModeReadOnly, &err);
    f = NULL;

  } else if (layer == -3) {
    // tracks
    first = 0;
    n = (UInt16)(GetTrackSize(obj) / sizeof(TracklogType));
    f = OpenLog(GetTrackName(obj), AppID, LogType, fileModeReadOnly);
    dbRef = 0;
    if (f)
      FileSeek(f, 0, fileOriginBeginning);

  } else {
    pline = getdatarec(layer, obj);
    first = pline->firstpoint;
    n = pline->npoints;
    f = NULL;
    dbRef = 0;
  }

  for (i = 0, length = 0; i < n; i++) {
    if (layer == -2) {
      if (RouteGetPoint(dbRef, GetRecIndex(2, i), &rpoint) != 0)
        break;
      x = rpoint.coord.longitude;
      y = rpoint.coord.latitude;
    } else if (layer == -3) {
      if (ReadLog(f, &tlog, sizeof(TracklogType)) != 0)
        break;
      x = tlog.longitude;
      y = tlog.latitude;
    } else {
      coord = getpointrec(layer, first+i);
      MapCoord2Grad(coord->y, coord->x, &y, &x);
    }

    if (i == 0) {
      x0 = x;
      y0 = y;
      continue;
    }

    rcx = TORAD(x);
    rcy = TORAD(y);
    rtx = TORAD(x0);
    rty = TORAD(y0);

    cosd = sin(rcy) * sin(rty) + cos(rcy) * cos(rty) * cos(rcx-rtx);
    if (cosd >= -1.0 && cosd <= 1.0)
      length += acos(cosd);

    x0 = x;
    y0 = y;
  }

  if (f)
    CloseLog(f);
  if (dbRef)
    DbClose(dbRef);

  return length * EARTH_RADIUS * 1000.0;
}

static double MapCalcArea(Int16 layer, UInt16 obj)
{   
  MapPlineType *pline;
  MapICoordType *coord;
  double area, side, xi, yi, xj, yj;
  UInt16 i, j;
  
  pline = getdatarec(layer, obj);

  for (i = 0, area = 0; i < pline->npoints; i++) {
    j = (i + 1) % pline->npoints;

    coord = getpointrec(layer, pline->firstpoint+i);
    MapCoord2Grad(coord->y, coord->x, &yi, &xi);

    coord = getpointrec(layer, pline->firstpoint+j);
    MapCoord2Grad(coord->y, coord->x, &yj, &xj);

    area += xi * yj;
    area -= yi * xj;
  }

  area /= 2;
  if (area < 0)
    area = -area;

  side = sqrt(area);
  side *= DEG_TO_KM * 1000.0;

  // returns the side (in meters), not the area

  return side;
}

void MapStartMeasure(Int16 mtype)
{
  measureType = mtype;
  measureCount = 0;
  measureX0 = measureY0 = measureX1 = measureY1 = -1;
}

void MapStopMeasure(void)
{
  RectangleType rect;

  if (hd) WinSetCoordinateSystem(kCoordinatesDouble);
  WinSetClip(&window);

  if (measureX0 != -1) {
    RctSetRectangle(&rect, measureX0-2, measureY0-2, measureX1-measureX0+4, measureY1-measureY0+4);
    WinCopyRectangle(whBuf, WinGetActiveWindow(), &rect, measureX0-2, measureY0-2, winPaint);
  }
  RctSetRectangle(&rect, window.extent.x/4, 0, window.extent.x/2, FntCharHeight());
  WinSetClip(&rect);
  WinEraseRectangle(&rect, 0);

  WinResetClip();
  if (hd) WinSetCoordinateSystem(kCoordinatesStandard);
}

void MapMeasurePoint(UInt16 x0, UInt16 y0, UInt16 x1, UInt16 y1)
{
  Int32 lat0, lon0, lat1, lon1;
  double rlat0, rlon0, rlat1, rlon1;
  double rcy, rty, cosd, d;
  RectangleType rect;
  char buf[32];

  MapScreen2Coord(&lat0, &lon0, x0, y0);
  MapCoord2Rad(lat0, lon0, &rlat0, &rlon0);

  MapScreen2Coord(&lat1, &lon1, x1, y1);
  MapCoord2Rad(lat1, lon1, &rlat1, &rlon1);

  rcy = rlat0;
  rty = rlat1;

  cosd = sin(rcy) * sin(rty) + cos(rcy) * cos(rty) * cos(rlon0 - rlon1);
  d = acos(cosd) * EARTH_RADIUS_M;
  FormatDistance(buf, d);

  if (hd) WinSetCoordinateSystem(kCoordinatesDouble);
  WinSetClip(&window);

  if (measureX0 != -1) {
    RctSetRectangle(&rect, measureX0-2, measureY0-2, measureX1-measureX0+4, measureY1-measureY0+4);
    WinCopyRectangle(whBuf, WinGetActiveWindow(), &rect, measureX0-2, measureY0-2, winPaint);
  }

  MapDrawLine(x0, y0, x1, y1, 2, positionColor, 0);

  RctSetRectangle(&rect, window.extent.x/4, 0, window.extent.x/2, FntCharHeight());
  WinSetClip(&rect);
  WinEraseRectangle(&rect, 0);
  WinDrawChars(buf, StrLen(buf), rect.topLeft.x, rect.topLeft.y);

  WinResetClip();
  if (hd) WinSetCoordinateSystem(kCoordinatesStandard);

  if (x0 < x1) {
    measureX0 = x0;
    measureX1 = x1;
  } else {
    measureX0 = x1;
    measureX1 = x0;
  }

  if (y0 < y1) {
    measureY0 = y0;
    measureY1 = y1;
  } else {
    measureY0 = y1;
    measureY1 = y0;
  }
}

void MapMeasureAddPoint(UInt16 x, UInt16 y)
{
  UInt16 i, j;
  Int32 lat, lon;
  double area, side, xi, yi, xj, yj;
  RectangleType rect;
  char buf[32];

  if (measureCount < MAX_MEASURES) {
    measureX[measureCount] = x;
    measureY[measureCount] = y;
    measureCount++;
  } else
    SndPlaySystemSound(sndError);

  if (hd) WinSetCoordinateSystem(kCoordinatesDouble);
  WinSetClip(&window);

  for (i = 0, area = 0; i < measureCount; i++) {
    j = (i + 1) % measureCount;

    if (measureCount > 2) {
      MapScreen2Coord(&lat, &lon, measureX[i], measureY[i]);
      MapCoord2Grad(lat, lon, &xi, &yi);

      MapScreen2Coord(&lat, &lon, measureX[j], measureY[j]);
      MapCoord2Grad(lat, lon, &xj, &yj);

      area += xi * yj;
      area -= yi * xj;
    }
    MapDrawLine(measureX[i], measureY[i], measureX[j], measureY[j], 2, positionColor, 0);
  }

  if (measureCount > 2) {
    area /= 2;
    if (area < 0)
      area = -area;

    side = sqrt(area);
    side *= DEG_TO_KM * 1000.0;
    FormatArea(buf, side);

    RctSetRectangle(&rect, window.extent.x/4, 0, window.extent.x/2, FntCharHeight());
    WinSetClip(&rect);
    WinEraseRectangle(&rect, 0);
    WinDrawChars(buf, StrLen(buf), rect.topLeft.x, rect.topLeft.y);
  }

  WinResetClip();
  if (hd) WinSetCoordinateSystem(kCoordinatesStandard);
}
