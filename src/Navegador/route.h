void RouteInit(DmOpenRef ptRef, DmOpenRef rtRef);
Err NewRoute(RouteType *rte);
Err UpdateRoute(UInt16 index, RouteType *rte);
Err AppendRoutePoint(DmOpenRef dbRef, RoutePointType *point);
Err InsertRoutePoint(DmOpenRef dbRef, RoutePointType *point);

Int16 GetRouteName(void *rec, char *buf);
void GetRouteCenter(void *rec, double *lat, double *lon);
Int16 GetRoutePointName(void *rec, char *buf);
UInt32 GetRoutePointSymbol(void *rec);
UInt32 GetRoutePointSeq(void *rec);
UInt32 GetRoutePointDistance(void *rec);
Int16 CompareSeq(void *e1, void *e2, Int32 other);

Err RouteGetPoint(DmOpenRef dbRef, UInt16 index, RoutePointType *rpoint);
void RouteGetFirst(UInt16 *currentSeq, UInt16 *currentIndex);
void RouteGetNext(UInt16 *currentSeq, UInt16 *currentIndex, CoordType *target);

void RouteInvert(char *name);
