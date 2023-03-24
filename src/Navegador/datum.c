#include <PalmOS.h>

#include "datum.h"
#include "MathLib.h"

#define WGS84_INDEX	0

static Ellipsoid ellipsoids[] = {
{"NULL",			0000000.000000,  000.000000},	// 0
{"Airy",			6377563.396000,  299.324965},	// 1
{"Modified Airy",		6377340.189000,  299.324965},	// 2
{"Australian National",		6378160.000000,  298.250000},	// 3
{"Bessel 1841",			6377397.155000,  299.152813},	// 4
{"Clarke 1866",			6378206.400000,  294.978698},	// 5
{"Clarke 1880",			6378249.145000,  293.465000},	// 6
{"Everest 1830",		6377276.345000,  300.801700},	// 7
{"Everest 1948",		6377304.063000,  300.801700},	// 8
{"Fischer 1960",		6378166.000000,  298.300000},	// 9
{"Modified Fischer 1960",	6378155.000000,  298.300000},	// 10
{"Fischer 1968",		6378150.000000,  298.300000},	// 11
{"GRS 1980",			6378137.000000,  298.257222},	// 12
{"Helmert 1906",		6378200.000000,  298.300000},	// 13
{"Hough",			6378270.000000,  297.000000},	// 14
{"International",		6378388.000000,  297.000000},	// 15
{"Krassovsky",			6378245.000000,  298.300000},	// 16
{"South American 1969",		6378160.000000,  298.250000},	// 17
{"WGS 60",			6378165.000000,  298.300000},	// 18
{"WGS 66",			6378145.000000,  298.250000},	// 19
{"WGS 72",			6378135.000000,  298.260000},	// 20
{"WGS 84",			6378137.000000,  298.257224},	// 21
{"Bessel 1841 (Namibia)",	6377483.865000,  299.152813},	// 22
{"Everest 1956",		6377301.243000,  300.801700},	// 23
{"Everest 1969",		6377295.664000,  300.801700},	// 24
{"Everest (Sabah & Sarawak)",	6377298.556000,  300.801700},	// 25
{"SGS 85",			6378136.000000,  298.257000}	// 26
};

static Datum datums[] = {
{21,    0.0,   0.0,    0.0},	// Index   0: WGS84
{20,    0.0,   0.0,    0.0},	// Index   1: WGS72
{17,  -60.0,  -2.0,  -41.0}	// Index   2: SAD69 Brazil
};

static char *datum_names[] = {
"WGS84",			// Index   0
"WGS72",			// Index   1
"SAD69 Brazil"			// Index   2
};

static int datum_ids[] = {
0,				// Index   0
1,				// Index   1
2,				// Index   2
-1
};

static int index;
static double dx, dy, dz;
static double from_a, from_e2;
static double to_a, to_b, to_e2, to_el2;
  
int DatumGetNum(void)
{
  return sizeof(datum_names) / sizeof(char *);
}

char **DatumGetList(void)
{
  return datum_names;
}

int DatumGetIndex(int id)
{
  int index;

  for (index = 0; datum_ids[index] != -1; index++)
    if (datum_ids[index] == id)
      return index;

  return -1;
}

int DatumGetID(int index)
{
  return datum_ids[index];
}

char *DatumGetName(int index)
{
  return datum_names[index];
}

void DatumFromWGS84(int _index)
{
  double f;
 
  index = _index;

  if (index == WGS84_INDEX)
    return;

  dx = -datums[index].dx;
  dy = -datums[index].dy;
  dz = -datums[index].dz;
 
  f = 1.0 / ellipsoids[datums[WGS84_INDEX].ellipsoid].i_f;
  from_a = ellipsoids[datums[WGS84_INDEX].ellipsoid].a;
  from_e2 = 2.0 * f - f*f;
 
  f = 1.0 / ellipsoids[datums[index].ellipsoid].i_f;
  to_a = ellipsoids[datums[index].ellipsoid].a;
  to_b = to_a * (1.0 - f);
  to_e2 = 2.0 * f - f*f; 
  to_el2 = (to_a*to_a - to_b*to_b) / (to_b*to_b);
}

void DatumToWGS84(int _index)
{
  double f;
 
  index = _index;

  if (index == WGS84_INDEX)
    return;

  dx = datums[index].dx;
  dy = datums[index].dy;
  dz = datums[index].dz;
 
  f = 1.0 / ellipsoids[datums[index].ellipsoid].i_f;
  from_a = ellipsoids[datums[index].ellipsoid].a;
  from_e2 = 2.0 * f - f*f;

  f = 1.0 / ellipsoids[datums[WGS84_INDEX].ellipsoid].i_f;
  to_a = ellipsoids[datums[WGS84_INDEX].ellipsoid].a;
  to_b = to_a * (1.0 - f);
  to_e2 = 2.0 * f - f*f;
  to_el2 = (to_a*to_a - to_b*to_b) / (to_b*to_b);
}

void DatumConvert(double *lon0, double *lat0, double *h0)
{
  double pi, lat, lon, h;
  double x, y, z, sinl, cosl, N;
  double p, t, sint, cost, sin3t, cos3t;
  
  if (index == WGS84_INDEX)
    return;

  pi = sys_pi();
  lat = pi * (*lat0) / 180.0;
  lon = pi * (*lon0) / 180.0;
  h = *h0;

  sinl = sin(lat);
  cosl = cos(lat);
  N = from_a / sqrt(1.0 - from_e2 * sinl * sinl);

  x = (N + h) * cosl * cos(lon);
  y = (N + h) * cosl * sin(lon);
  z = (N * (1.0 - from_e2) + h) * sinl;

  x += dx;
  y += dy;
  z += dz;

  p = sqrt(x*x + y*y);
  t = atan((z*to_a) / (p*to_b));

  sint = sin(t);
  cost = cos(t);
  sin3t = sint * sint * sint;
  cos3t = cost * cost * cost;

  lat = atan((z + to_el2 * to_b * sin3t) / (p - to_e2 * to_a * cos3t));
  lon = atan2(y, x);

  sinl = sin(lat);
  N = to_a / sqrt(1.0 - to_e2 * sinl * sinl);

  h = p / cos(lat) - N;

  lat = lat * 180.0 / pi;
  lon = lon * 180.0 / pi;

  *lat0 = lat;
  *lon0 = lon;
  *h0 = h;
}
