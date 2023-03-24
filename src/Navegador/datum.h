typedef struct {
  char *name;
  double a;
  double i_f;
} Ellipsoid;

typedef struct {
  int ellipsoid;
  double dx, dy, dz;
} Datum;

int DatumGetNum(void);
char **DatumGetList(void);
int DatumGetIndex(int id);
int DatumGetID(int index);
char *DatumGetName(int index);
void DatumFromWGS84(int index);
void DatumToWGS84(int index);
void DatumConvert(double *lon, double *lat, double *h);
