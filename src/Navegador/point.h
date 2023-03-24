Err NewPoint(DmOpenRef dbRef, WaypointType *point);
Err UpdatePoint(DmOpenRef dbRef, UInt16 index, WaypointType *point);

Int16 getpointname(void *rec, char *buf);
Int16 GetPointComment(void *rec, char *buf);
UInt32 getpointdata(void *rec);
void getpointcenter(void *rec, double *lat, double *lon);
UInt32 getpointdistance(void *rec);
