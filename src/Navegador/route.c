#include <PalmOS.h>

#include "gps.h"
#include "app.h"
#include "main.h"
#include "garmin.h"
#include "gui.h"
#include "ddb.h"
#include "list.h"
#include "format.h"
#include "map.h"
#include "mapdecl.h"
#include "misc.h"
#include "symbol.h"
#include "error.h"
#include "object.h"
#include "point.h"
#include "route.h"

#define TMP_ROUTE "tmpRoute"

static DmOpenRef ptRef, rtRef, rtptRef;
static UInt16 previous;

static Boolean distanceTop;
static Int16 distanceCount;
static float currentDistance;

void RouteInit(DmOpenRef _ptRef, DmOpenRef _rtRef)
{
  ptRef = _ptRef;
  rtRef = _rtRef;
}

Err NewRoute(RouteType *rte)
{
  Err err;
  UInt16 index;
 
  if ((err = DbCreateRec(rtRef, &index, sizeof(RouteType), 0)) != 0) {
    InfoDialog(ERROR, "Error creating route (%d)", err);
    return -1;
  }
  
  return UpdateRoute(index, rte);
}

Err UpdateRoute(UInt16 index, RouteType *rte)
{
  Err err;
  RouteType *p;

  p = (RouteType *)DbOpenRec(rtRef, index, &err);
  if (err) {
    InfoDialog(ERROR, "Error opening route (%d)", err);
    return -1;
  }

  if ((err = DmCheckAndWrite(p, 0, rte, sizeof(RouteType))) != 0)
    InfoDialog(ERROR, "Error writing route (%d)", err);

  DbCloseRec(rtRef, index, (char *)p);

  if (err == 0)
    DbCreate(rte->ident, RtePointsFType, AppID);

  return err ? -1 : 0;
}

Err AppendRoutePoint(DmOpenRef dbRef, RoutePointType *point)
{
  Err err;
  UInt16 index;
  RoutePointType *p;

  if ((err = DbCreateRec(dbRef, &index, sizeof(RoutePointType), 0)) != 0) {
    InfoDialog(ERROR, "Error creating point (%d)", err);
    return -1;
  }

  p = (RoutePointType *)DbOpenRec(dbRef, index, &err);
  if (err) {
    InfoDialog(ERROR, "Error opening point (%d)", err);
    return -1;
  }

  if (GetSymbolIndex(point->symbol) == 0xFFFF)
    point->symbol = sym_wpt_dot;

  point->seq = index;

  if ((err = DmCheckAndWrite(p, 0, point, sizeof(RoutePointType))) != 0)
    InfoDialog(ERROR, "Error writing point (%d)", err);

  DbCloseRec(dbRef, index, (char *)p);

  return err ? -1 : 0;
}

Err InsertRoutePoint(DmOpenRef dbRef, RoutePointType *point)
{
  Err err;
  UInt16 index, n, seq;
  RoutePointType *p;
  
  n = DmNumRecords(dbRef);

  for (index = 0; index < n; index++) {
    p = (RoutePointType *)DbOpenRec(dbRef, index, &err);
    if (!p || err) {
      InfoDialog(ERROR, "Error creating point (%d)", err);
      return -1;
    }
    if (p->seq >= point->seq) {
      seq = p->seq+1;
      if ((err = DmCheckAndWrite(p,
            TAM_NAME + sizeof(UInt16) + sizeof(CoordType),
            &seq, sizeof(UInt16))) != 0) {
        InfoDialog(ERROR, "Error writing point (%d)", err);
        DbCloseRec(dbRef, index, (char *)p);
        return -1;
      }
    }
    DbCloseRec(dbRef, index, (char *)p);
  }

  if ((err = DbCreateRec(dbRef, &index, sizeof(RoutePointType), 0)) != 0) {
    InfoDialog(ERROR, "Error creating point (%d)", err);
    return -1;
  } 
  
  p = (RoutePointType *)DbOpenRec(dbRef, index, &err);
  if (err) {
    InfoDialog(ERROR, "Error opening point (%d)", err);
    return -1;
  } 

  if (GetSymbolIndex(point->symbol) == 0xFFFF)
    point->symbol = sym_wpt_dot;

  if ((err = DmCheckAndWrite(p, 0, point, sizeof(RoutePointType))) != 0)
    InfoDialog(ERROR, "Error writing point (%d)", err);
  
  DbCloseRec(dbRef, index, (char *)p);
    
  return err ? -1 : 0;
}

Boolean EditRouteFormHandleEvent(EventPtr event)
{
  FormPtr frm;
  UInt16 form, index;
  FieldPtr fld;
  char *s;
  Err err;
  RouteType rte, *p;
  UInt32 type, creator;
  Boolean handled;
  static char origName[TAM_ROUTENAME];

  handled = false;

  frm = FrmGetActiveForm();
  form = FrmGetActiveFormID();
  previous = form;

  switch (event->eType) {
    case frmOpenEvent:
      MemSet(&rte, sizeof(rte), 0);
      MemSet(&origName, sizeof(origName), 0);

      if (form == EditRouteForm) {
        index = GetRecIndex(1, GetRecSelection(1));
        p = (RouteType *)DbOpenRec(rtRef, index, &err);
        if (p) {
          FldInsertStr(frm, nameFld, p->ident);
          MemMove(&rte, p, sizeof(rte));
          DbCloseRec(rtRef, index, (char *)p);
          StrNCopy(origName, rte.ident, TAM_ROUTENAME-1);
        }
      } else {
        DbDelete(TMP_ROUTE);
        DbCreate(TMP_ROUTE, RtePointsFType, AppID);
        StrNCopy(origName, TMP_ROUTE, TAM_ROUTENAME-1);
      }

      FrmSetFocus(frm, FrmGetObjectIndex(frm, nameFld));
      FrmDrawForm(frm);

      rtptRef = DbOpenByName(origName, dmModeReadWrite, &err);

      if (rtptRef) {
        ObjectDefine(2, rtptRef, SelectPointForm, 0, 0,
          GetRoutePointName, NULL, GetRoutePointSymbol, GetRoutePointSeq,
          NULL, CompareSeq);
        ObjectSelect(2);
        SetRecSelection(2, 0);
        ObjectRefresh(frm);
      }

      handled = true;
      break;

    case nilEvent:
      handled = ObjectListHandleEvent(event);
      break;

    case keyDownEvent:
      if (rtptRef && event->data.keyDown.modifiers & commandKeyMask)
        handled = ObjectListHandleEvent(event);
      break;

    case sclRepeatEvent: 
    case sclExitEvent: 
    case tblEnterEvent:
      handled = rtptRef ? ObjectListHandleEvent(event) : true;
      break;

    case ctlSelectEvent:
      switch (event->data.ctlSelect.controlID) {
        case newBtn: // opens SelectPointForm
          if (GetRecNum(0)) { // only if there are waypoints
            ObjectSelect(2);
            handled = rtptRef ? ObjectListHandleEvent(event) : true;
          } else
            handled = true;
          break;
        case deleteBtn: // delete route point
          if (GetRecNum(2)) { // only if the routh has points
            ObjectSelect(2);
            handled = rtptRef ? ObjectListHandleEvent(event) : true;
          } else
            handled = true;
          break;

        case okBtn: // closes EditRouteForm
          handled = true;

          fld = (FieldPtr)FrmGetObjectPtr(frm,
          FrmGetObjectIndex(frm, nameFld));
          s = FldGetTextPtr(fld);

          if (s && s[0]) {
            if (form == NewRouteForm) {
              if (FindRec(1, s, 0, false, 0) != -1) {
                InfoDialog(INFO, "Duplicate route name");
                break;
              }
              if (DmFindDatabase(0, s)) {
                DbGetTypeCreator(s, &type, &creator);

                if (type != RtePointsFType || creator != AppID) {
                  InfoDialog(INFO, "Duplicate database name");
                  break;
                }
                DbDelete(s);
              }
              StrNCopy(rte.ident, s, TAM_ROUTENAME-1);
              rte.ident[StrLen(s)] = 0;
              DbDelete(rte.ident);
              DbRename(TMP_ROUTE, rte.ident);

              NewRoute(&rte);

            } else {
              if (StrCompare(s, origName)) {
                if (FindRec(1, s, 0, false, 0) != -1) {
                  InfoDialog(INFO, "Duplicate route name");
                  break;
                }
                if (DmFindDatabase(0, s)) {
                  InfoDialog(INFO, "Duplicate database name");
                  break;
                }
                DbRename(origName, s);
              }
              StrNCopy(rte.ident, s, TAM_ROUTENAME-1);
              rte.ident[StrLen(s)] = 0;

              index = GetRecIndex(1, GetRecSelection(1));
              UpdateRoute(index, &rte);
            }
            MapInvalid();

          } else if (GetRecNum(2) > 0) {
            InfoDialog(INFO, "Invalid name");
            break;
          }

          if (rtptRef) DbClose(rtptRef);

          ObjectSelect(1);
          PopForm();
          BuildRecList(1, rtRef, GetRouteName, NULL, NULL, NULL, comparename);
          if (GetRecNum(1) == 1)
            SetRecSelection(1, 0);
          ObjectRefresh(FrmGetActiveForm());
          GetSelectedRoute(NULL);

          handled = true;
      }
      break;

    default:
      break;
  }

  return handled;
}

Boolean SelectPointFormHandleEvent(EventPtr event)
{
  FormPtr frm;
  WaypointType *p;
  RoutePointType rtepoint;
  UInt16 index;
  Err err;
  Boolean handled;

  handled = false;

  switch (event->eType) {
    case frmOpenEvent:
      frm = FrmGetActiveForm();
      ObjectDefine(3, ptRef, 0, 0, 0, getpointname, NULL, getpointdata,
        NULL, NULL, comparename);
      ObjectSelect(3);
      FrmDrawForm(frm);
      ObjectRefresh(frm);
      handled = true;
      break;

    case nilEvent:
    case sclRepeatEvent: 
    case sclExitEvent: 
    case keyDownEvent:
    case tblEnterEvent:
      handled = ObjectListHandleEvent(event);
      break;

    case ctlSelectEvent:
      switch (event->data.ctlSelect.controlID) {
        case okBtn:
          if (rtptRef) {
            index = GetRecIndex(3, GetRecSelection(3));
            p = (WaypointType *)DbOpenRec(ptRef, index, &err);
            if (p) {
              MemSet(&rtepoint, sizeof(rtepoint), 0);
              StrCopy(rtepoint.name, p->name);
              rtepoint.symbol = p->symbol;
              rtepoint.coord = p->coord;
              DbCloseRec(ptRef, index, (char *)p);

              if (GetRecNum(2) == 0)
                AppendRoutePoint(rtptRef, &rtepoint);
              else {
                rtepoint.seq = GetRecDynamic(2, GetRecSelection(2))+1;
                InsertRoutePoint(rtptRef, &rtepoint);
              }
            }
          }
          ObjectSelect(2);
          BuildRecList(2, rtptRef, GetRoutePointName, NULL,
            GetRoutePointSymbol, GetRoutePointSeq, CompareSeq);
          if (GetRecNum(2) == 1)
            SetRecSelection(2, 0);

          FrmReturnToForm(previous);
          ObjectRefresh(FrmGetActiveForm());

          handled = true;
      }
      break;

    default:
      break;
  }

  return handled;
}

Int16 GetRouteName(void *rec, char *buf)
{
  RouteType *p = (RouteType *)rec;
  StrCopy(buf, p->ident);
  return StrLen(buf)+1;
}

void GetRouteCenter(void *rec, double *lat, double *lon)
{
  RouteType *rte;
  RoutePointType rpoint;
  DmOpenRef dbRef;
  UInt16 i, n;
  Err err;

  rte = (RouteType *)rec;
  dbRef = DbOpenByName(rte->ident, dmModeReadOnly, &err);

  if (dbRef) {
    n = DmNumRecords(dbRef);
    *lat = *lon = 0;

    for (i = 0; i < n; i++) {
      if (RouteGetPoint(dbRef, i, &rpoint) != 0)
        break;
      *lat += rpoint.coord.latitude;
      *lon += rpoint.coord.longitude;
    }

    if (i) {
      *lat /= i;
      *lon /= i;
    }

    DbClose(dbRef);
  }
}

Int16 GetRoutePointName(void *rec, char *buf)
{
  RoutePointType *p = (RoutePointType *)rec;
  StrCopy(buf, p->name);
  return StrLen(buf)+1;
}

UInt32 GetRoutePointSymbol(void *rec)
{
  RoutePointType *p = (RoutePointType *)rec;
  return p->symbol;
}

UInt32 GetRoutePointSeq(void *rec)
{
  RoutePointType *p = (RoutePointType *)rec;
  return p->seq;
}

UInt32 GetRoutePointDistance(void *rec)
{
  RoutePointType *p;
  WaypointType point;
  UInt32 d;

  p = (RoutePointType *)rec;
  point.coord = p->coord;
  MapWaypointDistance(&point, (float *)(&d));

  return d;
}

Int16 CompareSeq(void *e1, void *e2, Int32 other)
{
  RecordType *r1 = (RecordType *)e1;
  RecordType *r2 = (RecordType *)e2;
  return r1->dynamic - r2->dynamic;
}

void RouteGetFirst(UInt16 *currentSeq, UInt16 *currentIndex)
{
  UInt16 index, tmp;
  RouteType *rte;
  RoutePointType rpoint;
  DmOpenRef dbRef;
  UInt32 di;
  float *d;
  Err err;

  index = GetRecIndex(1, GetRecSelection(1));
  rte = (RouteType *)DbOpenRec(rtRef, index, &err);

  if (rte) {
    dbRef = DbOpenByName(rte->ident, dmModeReadOnly, &err);

    if (dbRef) {
      ObjectDefine(2, dbRef, 0, 0, 0, GetRoutePointName, NULL,
        GetRoutePointSymbol, GetRoutePointDistance,
        NULL, comparedistance);

      tmp = GetRecIndex(2, 0);
      di = GetRecDynamic(2, 0);

      ObjectDefine(2, dbRef, 0, 0, 0, GetRoutePointName, NULL,
        GetRoutePointSymbol, GetRoutePointSeq, NULL, CompareSeq);

      if (RouteGetPoint(dbRef, tmp, &rpoint) == 0) {
        d = (float *)(&di);
        currentDistance = *d;
        distanceCount = 0;
        distanceTop = false;

        *currentIndex = tmp;
        *currentSeq = rpoint.seq;
      }
      DbClose(dbRef);
    }
    DbCloseRec(rtRef, index, (char *)rte);
  }
}

void RouteGetNext(UInt16 *currentSeq, UInt16 *currentIndex, CoordType *target)
{
  UInt16 index, nextIndex, i, n;
  WaypointType point;
  RouteType *rte;
  RoutePointType rpoint, nrpoint;
  DmOpenRef dbRef;
  Boolean next, last;
  float d, nd;
  Err err;

  index = GetRecIndex(1, GetRecSelection(1));
  rte = (RouteType *)DbOpenRec(rtRef, index, &err);

  if (rte) {
    dbRef = DbOpenByName(rte->ident, dmModeReadOnly, &err);

    if (dbRef) {
      next = false;
      nextIndex = 0;
      n = GetRecNum(2);
      d = nd = 0;

      for (i = 0; i < n; i++) {
        if (GetRecDynamic(2, i) <= *currentSeq)
          continue;

        nextIndex = GetRecIndex(2, i);
        break;
      }
      last = i == n;

      if (RouteGetPoint(dbRef, *currentIndex, &rpoint) == 0 &&
          (last || RouteGetPoint(dbRef, nextIndex, &nrpoint) == 0)) {

        point.coord = rpoint.coord;
        MapWaypointDistance(&point, &d);

        if (!last) {
          point.coord = nrpoint.coord;
          MapWaypointDistance(&point, &nd);
        }

        if (d <= currentDistance)  {
          if (distanceCount < 5)
            distanceCount++;
          else
            distanceTop = true;
        } else {
          if (distanceCount > 0)
            distanceCount--;
          else if (!last) {
            if (distanceTop || nd < d)
              next = true;
          }
        }
        currentDistance = d;
      }

      if (next) {
        currentDistance = nd;
        distanceCount = 0;
        distanceTop = false;

        *currentIndex = nextIndex;
        *currentSeq = nrpoint.seq;
        target->latitude = nrpoint.coord.latitude;
        target->longitude = nrpoint.coord.longitude;
      }
      DbClose(dbRef);
    }
    DbCloseRec(rtRef, index, (char *)rte);
  }
}

Err RouteGetPoint(DmOpenRef dbRef, UInt16 index, RoutePointType *rpoint)
{
  RoutePointType *rp;
  Err err;

  if ((rp = (RoutePointType *)DbOpenRec(dbRef, index, &err)) == NULL)
    return -1;

  MemMove(rpoint, rp, sizeof(RoutePointType));
  DbCloseRec(dbRef, index, (char *)rp);

  return 0;
}

void RouteInvert(char *name)
{
  DmOpenRef dbRef;
  RoutePointType rpoint, *rp;
  UInt16 i, n, index;
  Err err;

  dbRef = DbOpenByName(name, dmModeReadWrite, &err);

  if (dbRef) {
    ObjectDefine(2, dbRef, 0, 0, 0,
      GetRoutePointName, NULL, NULL, GetRoutePointSeq, NULL, CompareSeq);

    n = GetRecNum(2);
    for (i = 0; i < n; i++) {
      index = GetRecIndex(2, i);

      if ((rp = (RoutePointType *)DbOpenRec(dbRef, index, &err)) == NULL)
        break;

      MemMove(&rpoint, rp, sizeof(RoutePointType));
      rpoint.seq = n - 1 - i;

      DmCheckAndWrite(rp, 0, &rpoint, sizeof(RoutePointType));
      DbCloseRec(dbRef, index, (char *)rp);
    }

    DbClose(dbRef);
  }
}
