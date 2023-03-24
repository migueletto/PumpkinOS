#include <PalmOS.h>

#include "gps.h"
#include "app.h"
#include "main.h"
#include "map.h"
#include "mapdecl.h"
#include "ddb.h"
#include "symbol.h"
#include "garmin.h"
#include "misc.h"
#include "error.h"
#include "point.h"

Err NewPoint(DmOpenRef dbRef, WaypointType *point)
{
  Err err;
  UInt16 index;

  if ((err = DbCreateRec(dbRef, &index, sizeof(WaypointType), 0)) != 0) {
    InfoDialog(ERROR, "Error creating point (%d)", err);
    return -1;
  }

  return UpdatePoint(dbRef, index, point);
}
    
Err UpdatePoint(DmOpenRef dbRef, UInt16 index, WaypointType *point)
{
  Err err;
  WaypointType *p;

  p = (WaypointType *)DbOpenRec(dbRef, index, &err);
  if (err) {
    InfoDialog(ERROR, "Error opening point (%d)", err);
    return -1;
  }

  if (GetSymbolIndex(point->symbol) == 0xFFFF)
    point->symbol = sym_wpt_dot;

  if ((err = DmCheckAndWrite(p, 0, point, sizeof(WaypointType))) != 0)
    InfoDialog(ERROR, "Error writing point (%d)", err);

  DbUpdateCloseRec(dbRef, index, (char *)p, true);

  return err ? -1 : 0;
}

Int16 getpointname(void *rec, char *buf)
{
  WaypointType *p = (WaypointType *)rec;
  StrCopy(buf, p->name);
  return StrLen(buf)+1;
} 

Int16 GetPointComment(void *rec, char *buf)
{
  WaypointType *p = (WaypointType *)rec;
  StrCopy(buf, p->comment);
  return StrLen(buf)+1;
}

UInt32 getpointdata(void *rec)
{
  WaypointType *p = (WaypointType *)rec;
  return p->symbol; 
} 

void getpointcenter(void *rec, double *lat, double *lon)
{
  WaypointType *p = (WaypointType *)rec;
  *lat = p->coord.latitude;
  *lon = p->coord.longitude;
} 

UInt32 getpointdistance(void *rec)
{
  UInt32 d;
  WaypointType *p = (WaypointType *)rec;
  MapWaypointDistance(p, (float *)(&d));
  return d;
}
