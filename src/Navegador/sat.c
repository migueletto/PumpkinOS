#include <PalmOS.h>

#include "gui.h"
#include "gps.h"
#include "sat.h"
#include "app.h"
#include "format.h"
#include "compass.h"
#include "MathLib.h"

static Int16 usedColor, validColor, invalidColor, whiteColor, blackColor;
static Int16 sx, sy, xsky, ysky, rsky, tsky;
static WinHandle whBuf;
static Boolean hd;

void SatInit(Boolean highDensity, Int16 _sx, Int16 _sy,
             Int16 _xsky, Int16 _ysky, Int16 _rsky, Int16 _tsky,
             WinHandle _whBuf)
{
  hd = highDensity;
  sx = _sx;
  sy = _sy;
  xsky = _xsky;
  ysky = _ysky;
  rsky = _rsky;
  tsky = _tsky;
  whBuf = _whBuf;
}

void SatColor(Int16 _whiteColor, Int16 _blackColor,
             Int16 _usedColor, Int16 _validColor, Int16 _invalidColor)
{
  usedColor = _usedColor;
  validColor = _validColor;
  invalidColor = _invalidColor;
  whiteColor = _whiteColor;
  blackColor = _blackColor;
}

void SatDrawChannels(int16_t n, ChannelSummary *chs)
{
  UInt16 i, x, y, dx, dy, tx, t, bg, oldbg;
  RectangleType rect;
  FontID old;
  UInt8 flags;
  char buf[8];

  old = FntSetFont(stdFont);
  dx = FntCharWidth('0');
  if (hd)
    dx += 2; // to avoid numbers being too close
  dy = FntCharHeight();
  x = (sx / 2) - (12 * (1 + 2 * dx) + 1) / 2;
  y = sy - 3 * dy;

  for (i = 0; i < n; i++, x += 2 * dx + 1) {
    flags = chs[i].flags & 0xff;

    if (flags & CHANNEL_USED)
      bg = usedColor;
    else if (flags & CHANNEL_VALID)
      bg = validColor;
    else
      bg = invalidColor;

    t = (chs[i].cno * 2*dy) / 60;	// dBHZ (0-60)

    RctSetRectangle(&rect, x, y, 2*dx+2, 2 * dy);
    WinSetForeColor(blackColor);
    WinDrawLine(rect.topLeft.x, rect.topLeft.y,
      rect.topLeft.x+rect.extent.x-1, rect.topLeft.y);
    WinDrawLine(rect.topLeft.x+rect.extent.x-1, rect.topLeft.y,
      rect.topLeft.x+rect.extent.x-1, rect.topLeft.y+rect.extent.y-1);
    WinDrawLine(rect.topLeft.x+rect.extent.x-1, rect.topLeft.y+rect.extent.y-1,
      rect.topLeft.x, rect.topLeft.y+rect.extent.y-1);
    WinDrawLine(rect.topLeft.x, rect.topLeft.y+rect.extent.y-1,
      rect.topLeft.x, rect.topLeft.y);

    RctSetRectangle(&rect, x+1, y+1, 2*dx, 2*dy-2-t);
    WinEraseRectangle(&rect, 0);

    if (t) {
      oldbg = WinSetBackColor(bg);
      RctSetRectangle(&rect, x+1, y+2*dy-2-t+1, 2*dx, t);
      WinFillRectangle(&rect, 0);
      WinSetBackColor(oldbg);
    }

    if (chs[i].prn > 0) {
      StrPrintF(buf, "%02d", chs[i].prn);
      tx = FntCharsWidth(buf, 2);
      WinDrawChars(buf, 2, x+1+dx-tx/2, y+2*dy+1);
    } else {
      RctSetRectangle(&rect, x+1, y+2*dy+1, 2*dx, dy);
      WinEraseRectangle(&rect, 0);
    }
  }
  FntSetFont(old);
}

void SatDrawSky(int16_t n, VisibleSatellite *sat, ChannelSummary *chs)
{
  Int16 k, x, y, r, dx, dy, rs, rx, ry, l, fg, bg, j, i[MAX_CHANNELS];
  UInt8 flags;
  FontID oldFont;
  WinHandle whForm;
  RectangleType rect;
  double g, a;
  char buf[8];
  ChannelSummary tmp_chs[MAX_CHANNELS];

  whForm = WinSetDrawWindow(whBuf);
  WinEraseWindow();

  x = xsky;
  y = ysky;
  r = rsky;

  dy = FntCharHeight();
  WinDrawChar('N', x-FntCharWidth('N')/2, y-r-dy+1);
  WinDrawChar('W', x-r-FntCharWidth('W')-1, y-dy/2);
  WinDrawChar('E', x+r+3, y-dy/2);
  WinDrawChar('S', x-FntCharWidth('S')/2, y+r+1);

  oldFont = FntSetFont(hd ? hsmallFont : smallFont);
  dy = FntCharHeight();

  WinDrawPixel(x, y);
  DrawCircle(x, y, r);
  DrawCircle(x, y, r/2);

  WinPushDrawState();

  for (k = 0; k < n; k++) {
    tmp_chs[k].cno = 0;
    tmp_chs[k].prn = sat[k].prn;
    tmp_chs[k].flags = 0;
    for (j = 0; j < MAX_CHANNELS; j++)
      if (sat[k].prn == chs[j].prn) {
        tmp_chs[k] = chs[j];
        break;
      }
  }

  for (k = 0, j = 0; k < n; k++) {
    if (tmp_chs[k].cno == 0)
      i[j++] = k;
  }

  for (k = 0; k < n && j < n; k++) {
    flags = tmp_chs[k].flags & 0xff;
    if (tmp_chs[k].cno > 0 && !(flags & CHANNEL_VALID))
      i[j++] = k;
  }

  for (k = 0; k < n && j < n; k++) {
    flags = tmp_chs[k].flags & 0xff;
    if (tmp_chs[k].cno > 0 && (flags & CHANNEL_VALID) &&
                              !(flags & CHANNEL_USED))
      i[j++] = k;
  }

  for (k = 0; k < n && j < n; k++) {
    flags = tmp_chs[k].flags & 0xff;
    if (tmp_chs[k].cno > 0 && (flags & CHANNEL_VALID) &&
                               (flags & CHANNEL_USED))
      i[j++] = k;
  }
  
  for (k = 0; k < n; k++) {
    if (sat[i[k]].prn > 0 &&
        sat[i[k]].elevation >= 0.0 && sat[i[k]].elevation <= 90.0) {

      g = -sat[i[k]].azimuth + 90.0;
      a = TORAD(g);
      rs = (90.0 - sat[i[k]].elevation) * (r-2) / 90.0;
      ry = sin(a) * rs;
      rx = cos(a) * rs;
      StrPrintF(buf, "%02d", sat[i[k]].prn);
      l = StrLen(buf);
      dx = FntCharsWidth(buf, l);

      flags = tmp_chs[i[k]].flags & 0xff;
      if (flags & CHANNEL_USED)
        fg = whiteColor, bg = usedColor;
      else if (flags & CHANNEL_VALID)
        fg = whiteColor, bg = validColor;
      else if (tmp_chs[i[k]].cno > 0)
        fg = blackColor, bg = invalidColor;
      else
        fg = blackColor, bg = whiteColor;

      rect.topLeft.x = x+rx-dx/2-1;
      rect.topLeft.y = y-ry-dy/2-1;
      rect.extent.x = dx+1;
      rect.extent.y = dy+1;

      WinSetTextColor(fg);
      WinSetBackColor(bg);
      WinFillRectangle(&rect, 0);
      WinDrawChars(buf, l, rect.topLeft.x+1, rect.topLeft.y+1);

      WinSetForeColor(blackColor);
      WinDrawLine(rect.topLeft.x-1, rect.topLeft.y-1,
        rect.topLeft.x+rect.extent.x, rect.topLeft.y-1);
      WinDrawLine(rect.topLeft.x+rect.extent.x, rect.topLeft.y-1,
        rect.topLeft.x+rect.extent.x, rect.topLeft.y+rect.extent.y);
      WinDrawLine(rect.topLeft.x+rect.extent.x, rect.topLeft.y+rect.extent.y,
        rect.topLeft.x-1, rect.topLeft.y+rect.extent.y);
      WinDrawLine(rect.topLeft.x-1, rect.topLeft.y+rect.extent.y,
        rect.topLeft.x-1, rect.topLeft.y-1);
    }
  }
  WinPopDrawState();
  FntSetFont(oldFont);

  y = tsky;
  dy = FntCharHeight();
  dy = sy-3*dy-y-1;
  RctSetRectangle(&rect, 0, y, sx, dy);
  WinCopyRectangle(whBuf, whForm, &rect, 0, y, winPaint);
  WinSetDrawWindow(whForm);
}

void SatResetChannels(ChannelSummary *chs, VisibleSatellite *sat)
{
  Int16 i;

  for (i = 0; i < MAX_CHANNELS; i++) {
    chs[i].cno = 0;
    chs[i].prn = 0;
    chs[i].flags = 0;

    sat[i].prn = 0;
    sat[i].azimuth = 0.0;
    sat[i].elevation = 0.0;
  }
}
