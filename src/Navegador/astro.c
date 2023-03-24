#include <PalmOS.h>

#include "gui.h"
#include "gps.h"
#include "app.h"
#include "main.h"
#include "format.h"
#include "compass.h"
#include "mapdecl.h"
#include "misc.h"
#include "MathLib.h"
#include "astro.h"

/*
  N = longitude of the ascending node
  i = inclination to the ecliptic (plane of the Earth's orbit)
  w = argument of perihelion
  a = semi-major axis, or mean distance from Sun
  e = eccentricity (0=circle, 0-1=ellipse, 1=parabola)
  M = mean anomaly (0 at perihelion; increases uniformly with time)

  ecl = the obliquity of the ecliptic
*/

// Sun's upper limb touches the horizon (including atmospheric refraction)
#define SUN_H	-0.833
#define MOON_H	0.0

#define SUN_I	1
#define MOON_I	1

typedef struct {
  double N, N2;
  double i, i2;
  double w, w2;
  double a, a2;
  double e, e2;
  double M, M2;
} OrbitalElements;

static OrbitalElements sunElements = 
  {0.0, 0.0,
   0.0, 0.0,
   282.9404, 4.70935E-5,
   1.0, 0.0,
   0.016709, -1.151E-9,
   356.0470, 0.9856002585};

static OrbitalElements moonElements = 
  {125.1228, -0.0529538083,
   5.1454, 0.0,
   318.0634, 0.1643573223,
   60.2666, 0.0,
   0.054900, 0.0,
   115.3654, 13.0649929509};

static Int16 sx, sy, xsky, ysky, rsky, tsky;
static double lat, lon;
static UInt32 last_update;
static Boolean hd;

static void AstroCalcSunPos(OrbitalElements *sun, double dh,
                            double *ra, double *dec,double *lonsun);
static Boolean AstroCalcRiseSet(OrbitalElements *sun, double dh, double height,
                                double lat, double lon,
                                double ra, double dec,
                                double *rise, double *set);
static void AstroCalcMoonPos(OrbitalElements *sun, OrbitalElements *moon,
                             double dh, double lonsun, double *ra, double *dec,
                             double *d, double *ph);
static void AstroGetSun(double d, double h,
                        double lat, double lon,
                        double *az, double *alt, double *lonsun,
                        double *drise, double *rise,
                        double *dset, double *set);
static void AstroGetMoon(double d, double h,
                         double lat, double lon,
                         double *az, double *alt, double lonsun, double *ph,
                         double *drise, double *rise,
                         double *dset, double *set);
static void AstroAdjustDate(double *d, double *h);
static void AstroAdjustTime(double *t);
static void AstroDrawSky(double sun_az, double sun_alt,
                         double moon_az, double moon_alt,
                         double moon_phase);
static void AstroFillTime(Boolean ok, double now, double d, double h,
                          FormPtr frm, UInt16 id);
static void AstroDrawForm(FormPtr frm);

static void radec2azalt(double lat, double lst,
                        double ra, double dec,
                        double *az, double *alt);

void AstroInit(Boolean highDensity, Int16 _sx, Int16 _sy,
               Int16 _xsky, Int16 _ysky, Int16 _rsky, Int16 _tsky)
{
  hd = highDensity;
  sx = _sx;
  sy = _sy;
  xsky = _xsky;
  ysky = _ysky;
  rsky = _rsky;
  tsky = _tsky;
  last_update = 0;
}

void AstroPosition(double _lat, double _lon)
{
  lat = _lat;
  lon = _lon;
}

static void AstroDrawSky(double sun_az, double sun_alt,
                         double moon_az, double moon_alt,
                         double moon_phase)
{
  Int16 x, y, r, dy, rs, rx, ry;
  UInt16 bmpId;
  double g, a;
  
  x = xsky;
  y = ysky;
  r = rsky;
  
  dy = FntCharHeight();
  WinDrawChar('N', x-FntCharWidth('N')/2, y-r-dy+1);
  WinDrawChar('W', x-r-FntCharWidth('W')-1, y-dy/2);
  WinDrawChar('E', x+r+3, y-dy/2);
  WinDrawChar('S', x-FntCharWidth('S')/2, y+r+1);
  
  dy = FntCharHeight();
  
  WinDrawPixel(x, y);
  DrawCircle(x, y, r);
  DrawCircle(x, y, r/2);

  if (sun_alt >= 0.0 && sun_alt <= 90.0) {
    g = -sun_az + 90.0;
    a = TORAD(g);
    rs = (90.0 - sun_alt) * (r-2) / 90.0;
    ry = sin(a) * rs;
    rx = cos(a) * rs;
    DrawBmp(sunBmp, x+rx, y-ry, true);
  }

  if (moon_alt >= 0.0 && moon_alt <= 90.0) {
    g = -moon_az + 90.0;
    a = TORAD(g);
    rs = (90.0 - moon_alt) * (r-2) / 90.0;
    ry = sin(a) * rs;
    rx = cos(a) * rs;

    if (moon_phase < 22.5)
      bmpId = moon0Bmp;
    else if (moon_phase < 67.5)
      bmpId = moon1Bmp;
    else if (moon_phase < 112.5)
      bmpId = moon2Bmp;
    else if (moon_phase < 157.5)
      bmpId = moon3Bmp;
    else
      bmpId = moon4Bmp;

    DrawBmp(bmpId, x+rx, y-ry, true);
  }
}

static void AstroCalcSunPos(OrbitalElements *sun, double dh,
                            double *ra, double *dec, double *lonsun)
{
  double ecl, M, e, w, E, xv, yv, v, r, xs, ys, xe, ye, ze;

  ecl = TORAD(23.4393 - 3.563E-7 * dh);

  M = TORAD(sun->M + sun->M2 * dh);
  w = TORAD(sun->w + sun->w2 * dh);
  e = sun->e + sun->e2 * dh;

  E = M + e * sin(M) * (1.0 + e * cos(M));
  xv = cos(E) - e;
  yv = sqrt(1.0 - e*e) * sin(E);
  v = atan2(yv, xv);
  r = sqrt(xv*xv + yv*yv);
  *lonsun = v + w;
  xs = r * cos(*lonsun);
  ys = r * sin(*lonsun);
  xe = xs;
  ye = ys * cos(ecl);
  ze = ys * sin(ecl);

  *dec = TODEG(atan2(ze, sqrt(xe*xe + ye*ye)));
  *ra  = TODEG(atan2(ye, xe));
  if (*ra < 0.0) *ra += 360.0;
  *ra /= 15.0;
}

static Boolean AstroCalcRiseSet(OrbitalElements *sun, double dh, double height,
                                double lat, double lon,
                                double ra, double dec,
                                double *rise, double *set)
{
  double coslha, lha, M, w, L, GMST0, UT;

  height = TORAD(height);
  lat = TORAD(lat);
  dec = TORAD(dec);

  coslha = (sin(height) - sin(lat) * sin(dec)) / (cos(lat) * cos(dec));

  *rise = 0.0;
  *set = 0.0;

  if (coslha < -1.0 || coslha > 1.0)
    return false;

  lha = acos(coslha);
  lha = TODEG(lha) / 15.04107;
  //lha = TODEG(lha) / 15.0;

  M = sun->M + sun->M2 * dh;
  w = sun->w + sun->w2 * dh;
  L = M + w;
  GMST0 = L + 180.0;
  for (; GMST0 < 0.0; GMST0 += 360.0);
  for (; GMST0 > 360.0; GMST0 -= 360.0);
  UT = ra - (GMST0 / 15.0) - (lon / 15.0);

  *rise = UT - lha;
  *set = UT + lha;

  return true;
}

static void AstroCalcMoonPos(OrbitalElements *sun, OrbitalElements *moon,
                             double dh, double lonsun, double *ra, double *dec,
                             double *d, double *ph)
{
  double ecl, M, e, a, N, i, w, E, E0, xv, yv, v, r, elong;
  double xh, yh, zh, xg, yg, zg, xe, ye, ze;
  double lonecl, latecl, cos_latecl, cos_ecl, sin_ecl, aux;
  double Ms, Mm, Nm, ws, wm, Ls, Lm, D, F;
  double cos_N, sin_N, cos_i, sin_i, cos_vw, sin_vw;

  ecl = TORAD(23.4393 - 3.563E-7 * dh);

  M = TORAD(moon->M + moon->M2 * dh);
  N = TORAD(moon->N + moon->N2 * dh);
  i = TORAD(moon->i + moon->i2 * dh);
  w = TORAD(moon->w + moon->w2 * dh);
  e = moon->e + moon->e2 * dh;
  a = moon->a + moon->a2 * dh;
  
  E = M + e * sin(M) * (1.0 + e * cos(M));

  if (e > 0.05) do {
    E0 = E;
    E = E0 - (E0 - e * sin(E0) - M) / (1.0 - e * cos(E0));
  } while (fabs(E-E0) > 0.001);

  xv = a * (cos(E) - e);
  yv = a * (sqrt(1.0 - e*e) * sin(E));
  v = atan2(yv, xv);
  r = sqrt(xv*xv + yv*yv);

  cos_N = cos(N);
  sin_N = sin(N);
  cos_i = cos(i);
  sin_i = sin(i);
  cos_vw = cos(v+w);
  sin_vw = sin(v+w);
  aux = sin_vw * cos_i;

  xh = r * (cos_N * cos_vw - sin_N * aux);
  yh = r * (sin_N * cos_vw + cos_N * aux);
  zh = r * (sin_vw * sin_i);

  lonecl = atan2(yh, xh);
  latecl = atan2(zh, sqrt(xh*xh + yh*yh));
  // Perturbations of the Moon
  lonecl = TODEG(lonecl);
  latecl = TODEG(latecl);

  Ms = TORAD(sun->M + sun->M2 * dh); // Mean Anomaly of the Sun
  ws = TORAD(sun->w + sun->w2 * dh); // Argument of perihelion for the Sun
  Mm = M;             // Mean Anomaly of the Moon
  wm = w;             // Argument of perihelion for the Moon
  Nm = N;             // Longitude of the Moon's node

  Ls = Ms + ws;       // Mean Lodgitude of the Spn (Ns=0 
  Lm = Mm + wm + Nm;  // Mean Longitude of the Moon
  D = Lm - Ls;        // Mean elongation of the Moon
  F = Lm - Nm;        // Argument of latitude for the Moon

  // Add these terms to the Moon's longitude (degrees):

  lonecl += -1.274 * sin(Mm - 2*D);      // (the Evection)
  lonecl += +0.658 * sin(2*D);           // (the Variation)
  lonecl += -0.186 * sin(Ms);            // (the Yearly Equation)
  lonecl += -0.059 * sin(2*Mm - 2*D);
  lonecl += -0.057 * sin(Mm - 2*D + Ms);
  lonecl += +0.053 * sin(Mm + 2*D);
  lonecl += +0.046 * sin(2*D - Ms);
  lonecl += +0.041 * sin(Mm - Ms);
  lonecl += -0.035 * sin(D);             // (the Parallactic Equation)
  lonecl += -0.031 * sin(Mm + Ms);
  lonecl += -0.015 * sin(2*F - 2*D);
  lonecl += +0.011 * sin(Mm - 4*D);

  // Add these terms to the Moon's latitude (degrees):

  latecl += -0.173 * sin(F - 2*D);
  latecl += -0.055 * sin(Mm - F - 2*D);
  latecl += -0.046 * sin(Mm + F - 2*D);
  latecl += +0.033 * sin(F + 2*D);
  latecl += +0.017 * sin(2*Mm + F);

  lonecl = TORAD(lonecl);
  latecl = TORAD(latecl);

  cos_latecl = cos(latecl);

  xh = r * cos(lonecl) * cos_latecl;
  yh = r * sin(lonecl) * cos_latecl;
  zh = r * sin(latecl);

  xg = xh;
  yg = yh;
  zg = zh;

  cos_ecl = cos(ecl);
  sin_ecl = sin(ecl);

  xe = xg;
  ye = yg * cos_ecl - zg * sin_ecl;
  ze = yg * sin_ecl + zg * cos_ecl;

  *dec = TODEG(atan2(ze, sqrt(xe*xe + ye*ye)));
  *ra  = TODEG(atan2(ye, xe));
  if (*ra < 0.0) *ra += 360.0;
  *ra /= 15.0;

  *d = sqrt(xe*xe + ye*ye + ze*ze);

  elong = TODEG(acos(cos(lonsun - lonecl) * cos_latecl));
  *ph = 180.0 - elong;

  // ph =   0 : full moon
  // ph =  90 : half moon
  // ph = 180 : new moon
}

static void radec2azalt(double lat, double lst,
                        double ra, double dec,
                        double *az, double *alt)
{
  double ha, s1, s2, c1;

  ra *= 15.0;

  ha = lst - ra;
  if (ha < 0.0) ha += 360.0;

  dec = TORAD(dec);
  lat = TORAD(lat);
  ha = TORAD(ha);

  s1 = sin(dec);
  c1 = cos(lat);
  s2 = sin(lat);

  *alt = asin(s1*s2 + cos(dec)*c1*cos(ha));
  *az = acos((s1 - sin(*alt)*s2) / (cos(*alt)*c1));

  *alt = TODEG(*alt);
  *az = TODEG(*az);

  if (ha >= 0.0 && ha < sys_pi())
    *az = 360.0 - *az;
}

static void AstroGetSun(double d, double h,
                        double lat, double lon,
                        double *az, double *alt, double *lonsun,
                        double *drise, double *rise,
                        double *dset, double *set)
{
  Int16 i;
  double dh, lst, ra, dec, r, s, rise_tmp, set_tmp, d_tmp, tmp;

  d_tmp = d;

  dh = d + h / 24.0;
  AstroCalcSunPos(&sunElements, dh, &ra, &dec, lonsun);

  lst = 100.46 + 0.985647 * (dh - 1.5) + lon + 15.0 * h;
  for (; lst < 0.0; lst += 360.0);
  for (; lst > 360.0; lst -= 360.0);

  radec2azalt(lat, lst, ra, dec, az, alt);

  *drise = *rise = *dset = *set = 0.0;

  if (!AstroCalcRiseSet(&sunElements, dh, SUN_H, lat, lon, ra, dec,
          &rise_tmp, &set_tmp))
   return;

  d = d_tmp;
  h = rise_tmp;

  for (i = 0; i < SUN_I; i++) {
    dh = d + h / 24.0;
    AstroCalcSunPos(&sunElements, dh, &ra, &dec, &tmp);
    AstroCalcRiseSet(&sunElements, dh, SUN_H, lat, lon, ra, dec, &r, &s);
    h = r;
  }
  *drise = d;
  *rise = h;

  d = d_tmp;
  h = set_tmp;

  for (i = 0; i < SUN_I; i++) {
    dh = d + h / 24.0;
    AstroCalcSunPos(&sunElements, dh, &ra, &dec, &tmp);
    AstroCalcRiseSet(&sunElements, dh, SUN_H, lat, lon, ra, dec, &r, &s);
    h = s;
  }
  *dset = d;
  *set = h;
}

static void AstroGetMoon(double d, double h,
                         double lat, double lon,
                         double *az, double *alt, double lonsun, double *ph,
                         double *drise, double *rise,
                         double *dset, double *set)
{
  Int16 i;
  double dh, lst, ra, dec, dist, mpar, r, s, rise_tmp, set_tmp, d_tmp;

  d_tmp = d;

  dh = d + h / 24.0;
  AstroCalcMoonPos(&sunElements, &moonElements, dh, lonsun, &ra, &dec,
    &dist, ph);

  lst = 100.46 + 0.985647 * (dh - 1.5) + lon + 15.0 * h;
  for (; lst < 0.0; lst += 360.0);
  for (; lst > 360.0; lst -= 360.0);
    
  radec2azalt(lat, lst, ra, dec, az, alt); 
  mpar = TODEG(asin(1.0 / dist));

  // Moon's topocentric correction
  *alt -= mpar * cos(TORAD(*alt));

  if (!AstroCalcRiseSet(&sunElements, dh, MOON_H, lat, lon, ra, dec,
          &rise_tmp, &set_tmp))
    return;

  d = d_tmp;
  h = rise_tmp;

  for (i = 0; i < MOON_I; i++) {
    dh = d + h / 24.0;
    AstroCalcMoonPos(&sunElements, &moonElements, lonsun, dh, &ra, &dec,
      &dist, ph);
    AstroCalcRiseSet(&sunElements, dh, MOON_H, lat, lon, ra, dec, &r, &s);
    h = r;
  }
  *drise = d;
  *rise = h;

  d = d_tmp;
  h = set_tmp;

  for (i = 0; i < MOON_I; i++) {
    dh = d + h / 24.0;
    AstroCalcMoonPos(&sunElements, &moonElements, dh, lonsun, &ra, &dec,
      &dist, ph);
    AstroCalcRiseSet(&sunElements, dh, MOON_H, lat, lon, ra, dec, &r, &s);
    h = s;
  }
  *dset = d;
  *set = h;
}

static void AstroAdjustDate(double *d, double *h)
{
  while ((*h) < 0.0) {
    (*d) -= 1.0;
    (*h) += 24.0;
  }

  while ((*h) >= 24.0) {
    (*d) += 1.0;
    (*h) -= 24.0;
  }
}

static void AstroAdjustTime(double *t)
{
  double timezone, daylight_saving;

  timezone = ((double)((Int32)PrefGetPreference(prefTimeZone))) / 60.0;
  daylight_saving =
     ((double)((Int32)PrefGetPreference(prefDaylightSavingAdjustment))) / 60.0;

  *t += timezone + daylight_saving;
}

static void AstroFillTime(Boolean ok, double now, double d, double h, FormPtr frm, UInt16 id)
{
  char buf[16];

  if (ok) {
    hStrPrintFd(buf, h);
    buf[5] = (now > (d + h / 24.0)) ? '-' : '+';
    buf[6] = 0;
  } else
    StrCopy(buf, "--");

  FldInsertStr(frm, id, buf);
}

static void AstroDrawForm(FormPtr frm)
{
  uint16_t year;
  uint8_t month, day, hour, min, sec;
  double now, d, h, moon_phase;
  double sun_az, sun_alt, dsun_rise, sun_rise, dsun_set, sun_set, lonsun;
  double moon_az, moon_alt, dmoon_rise, moon_rise, dmoon_set, moon_set;
  Boolean sun_ok, moon_ok;

  FrmDrawForm(frm);

  GetUTC(&day, &month, &year, &hour, &min, &sec);

  // number of days since 2000-01-01 00:00
  d = (double)(367L*year - (7L*(year+(month+9L)/12L))/4L + (275L*month)/9L+day-730530L);
  h = (double)hour + ((double)min)/60.0 + ((double)sec)/3600.0;

  AstroGetSun(d, h, lat, lon, &sun_az, &sun_alt, &lonsun, &dsun_rise, &sun_rise, &dsun_set, &sun_set);
  AstroGetMoon(d, h, lat, lon, &moon_az, &moon_alt, lonsun, &moon_phase, &dmoon_rise, &moon_rise, &dmoon_set, &moon_set);

  if (hd) WinSetCoordinateSystem(kCoordinatesDouble);
  AstroDrawSky(sun_az, sun_alt, moon_az, moon_alt, moon_phase);
  if (hd) WinSetCoordinateSystem(kCoordinatesStandard);

  sun_ok = sun_rise != 0.0 || sun_set != 0.0;
  moon_ok = moon_rise != 0.0 || moon_set != 0.0;

  AstroAdjustTime(&h);
  AstroAdjustDate(&d, &h);
  now = d + h / 24.0;

  AstroAdjustTime(&sun_rise);
  AstroAdjustDate(&dsun_rise, &sun_rise);
  AstroAdjustTime(&sun_set);
  AstroAdjustDate(&dsun_set, &sun_set);

  AstroFillTime(sun_ok, now, dsun_rise, sun_rise, frm, sriseFld);
  AstroFillTime(sun_ok, now, dsun_set, sun_set, frm, ssetFld);

  AstroAdjustTime(&moon_rise);
  AstroAdjustDate(&dmoon_rise, &moon_rise);
  AstroAdjustTime(&moon_set);
  AstroAdjustDate(&dmoon_set, &moon_set);

  AstroFillTime(moon_ok, now, dmoon_rise, moon_rise, frm, mriseFld);
  AstroFillTime(moon_ok, now, dmoon_set, moon_set, frm, msetFld);
}

void AstroUpdate(DateTimeType *datetime)
{
  UInt32 t;
  FormPtr frm;
  static char buf[16];

  t = TimGetSeconds();
  frm = FrmGetActiveForm();

  if ((t - last_update) >= 60) {
    AstroDrawForm(frm);
    last_update = t;
  }

  FormatTime(buf, datetime->hour, datetime->minute, datetime->second, true);
  SetField(frm, utcFld, buf);
}

Boolean AstroFormHandleEvent(EventPtr event)
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
      AstroDrawForm(frm);
      last_update = TimGetSeconds();
      handled = true;
      break;

    case frmUpdateEvent:
      frm = FrmGetActiveForm();
      AstroDrawForm(frm);
      last_update = TimGetSeconds();
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
