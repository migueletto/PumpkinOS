#include <PalmOS.h>

#include "gps.h"
#include "app.h"
#include "main.h"
#include "ddb.h"
#include "log.h"
#include "error.h"
#include "display.h"

#define MAX_WIDTH	640
#define TEMP_BITMAP	"NavTmpBmp"
#define LINESIZE(w)	((((w) * 8 + 31) / 32) * 4)

static char name[32];
static UInt8 bits[MAX_WIDTH];
static UInt16 total, count, bpp, size;
static Int32 base;
static Boolean valid = false;

typedef struct {
  UInt16 type;
  UInt32 fileSize;
  UInt32 reserved;
  UInt32 dataOffset;
  UInt32 headerSize;
  UInt32 width;
  UInt32 height;
  UInt16 planes;
  UInt16 bpp;
  UInt32 compression;
  UInt32 dataSize;
  UInt32 hRes;
  UInt32 vRes;
  UInt32 colors;
  UInt32 iColors;
} WindowsBitmap;

typedef struct {
  UInt8 blue;
  UInt8 green;
  UInt8 red;
  UInt8 pad;
} WindowsBitmapColor;

static RGBColorType pal[4] = {
  {0, 255, 255, 255},
  {0, 128, 128, 128},
  {0, 64, 64, 64},
  {0, 0, 0, 0}
};

static void SaveHeader(FileHand f, UInt16 width, UInt16 height,
                       RGBColorType *pal, UInt16 colors);
static void InvertBitmap(char *name, UInt16 width, UInt16 height);

static void SaveHeader(FileHand f, UInt16 width, UInt16 height,
                       RGBColorType *pal, UInt16 colors)
{
  UInt16 i;
  UInt32 colorsize, linesize;
  RGBColorType rgb;
  WindowsBitmap wbmp;
  WindowsBitmapColor wrgb;

  colorsize = colors * sizeof(WindowsBitmapColor);
  linesize = LINESIZE(width);

  wbmp.type = 0x424d;
  wbmp.reserved = 0;
  wbmp.headerSize = 0x28;
  wbmp.width = width;
  wbmp.height = height;
  wbmp.planes = 1;
  wbmp.bpp = 8;
  wbmp.compression = 0;
  wbmp.hRes = 0x0EC4;
  wbmp.vRes = 0x0EC4;
  wbmp.colors = colors;
  wbmp.iColors = 0;

  wbmp.dataOffset = sizeof(wbmp) + colorsize;
  wbmp.dataSize = height * linesize;
  wbmp.fileSize = wbmp.dataOffset + wbmp.dataSize;

  wbmp.dataOffset = wbmp.dataOffset;
  wbmp.dataSize = wbmp.dataSize;
  wbmp.fileSize = wbmp.fileSize;

  WriteLog(f, &wbmp, sizeof(wbmp));

  for (i = 0; i < colors; i++) {
    if (pal) {
      wrgb.red = pal[i].r;
      wrgb.green = pal[i].g;
      wrgb.blue = pal[i].b;
    } else {
      WinPalette(winPaletteGet, i, 1, &rgb);
      wrgb.red = rgb.r;
      wrgb.green = rgb.g;
      wrgb.blue = rgb.b;
    }
    wrgb.pad = 0;
    WriteLog(f, &wrgb, sizeof(wrgb));
  }
}

static void InvertBitmap(char *name, UInt16 width, UInt16 height)
{
  FileHand f0, f1;
  WindowsBitmap wbmp;
  WindowsBitmapColor wrgb;
  Int32 offset;
  UInt16 i;
  Err err;

  DbDelete(TEMP_BITMAP);

  if ((err = DbRename(name, TEMP_BITMAP)) != 0) {
    InfoDialog(INFO, "Rename: %d", err);
    return;
  }

  if ((f0 = OpenLog(TEMP_BITMAP, AppID, ScreenType,fileModeReadOnly)) == NULL) {
    InfoDialog(INFO, "Open 0");
    return;
  }

  if ((f1 = OpenLog(name, AppID, ScreenType, fileModeReadWrite)) == NULL) {
    CloseLog(f0);
    InfoDialog(INFO, "Open 1");
    return;
  }

  FileSeek(f0, 0, fileOriginBeginning);
  ReadLog(f0, &wbmp, sizeof(wbmp));
  WriteLog(f1, &wbmp, sizeof(wbmp));

  for (i = 0; i < 256; i++) {
    ReadLog(f0, &wrgb, sizeof(wrgb));
    WriteLog(f1, &wrgb, sizeof(wrgb));
  }

  for (i = 0, offset = width; i < height; i++, offset += width) {
    FileSeek(f0, -offset, fileOriginEnd);
    ReadLog(f0, bits, width);
    WriteLog(f1, bits, width);
  }

  CloseLog(f1);
  CloseLog(f0);
  DbDelete(TEMP_BITMAP);
}

void SaveScreen(UInt16 width, UInt16 height)
{ 
  AppPrefs *prefs;
  FileHand f;
  BitmapType *bmp; 
  UInt32 n; 
  void *p;
  
  prefs = GetPrefs();
  StrPrintF(name, "%s%d", ScreenName, prefs->capture++);
  DbDelete(name);
  f = OpenLog(name, AppID, ScreenType, fileModeReadWrite);
  
  SaveHeader(f, width, height, NULL, 256);

  bmp = WinGetBitmap(WinGetActiveWindow());
  p = BmpGetBits(bmp);
  BmpGetSizes(bmp, &n, NULL);
  WriteLog(f, p, n);
  CloseLog(f);

  InvertBitmap(name, LINESIZE(width), height);
}

void StartDisplay(int width, int height, int _bpp)
{
  AppPrefs *prefs;
  FileHand f;

  valid = false;

  if (width <= 0 || width > MAX_WIDTH ||
      height <= 0 || height > MAX_WIDTH || _bpp != 2)
    return;

  valid = true;
  bpp = _bpp;
  size = LINESIZE(width);

  prefs = GetPrefs();
  StrPrintF(name, "%s%d", ScreenName, prefs->capture++);
  DbDelete(name);
  f = OpenLog(name, AppID, ScreenType, fileModeReadWrite);

  SaveHeader(f, width, height, pal, 4);
  base = FileTell(f, NULL, NULL);
  CloseLog(f);

  count = 0;
  total = height;
  BeginImport(total);
}

void SendDisplayRow(uint8_t *buf, int n)
{
  FileHand f;
  UInt16 i, j, k, mask;

  if (!valid)
    return;

  IncCounter();
  count++;

  f = OpenLog(name, AppID, ScreenType, fileModeUpdate);

  mask = (1 << bpp) - 1;

  MemSet(bits, sizeof(bits), 0);
  for (i = 0, k = 0; i < n; i++)
    for (j = 0; j < 8 && k < MAX_WIDTH; j += bpp)
      bits[k++] = (buf[i] >> j) & mask;

  FileSeek(f, base + (total-count) * size, fileOriginBeginning);
  WriteLog(f, bits, size);
  CloseLog(f);

  if (count >= total) {
    EndTransfer();
    valid = false;
  }
}
