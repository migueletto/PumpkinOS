#include <PalmOS.h>

#include "sys.h"
#include "thread.h"
#include "pwindow.h"
#include "vfs.h"
#include "rgb.h"
#include "bytes.h"
#include "pumpkin.h"
#include "dbg.h"
#include "sys.h"
#include "debug.h"
#include "xalloc.h"

#define MAX_PAL  256
#include "palette.h"

#define FORM_FILL_ALPHA 0xFF

typedef struct {
  RGBColorType foreColorRGB;
  RGBColorType backColorRGB;
  RGBColorType textColorRGB;
  IndexedColorType foreColor;
  IndexedColorType backColor;
  IndexedColorType textColor;
  UInt16 foreColor565;
  UInt16 backColor565;
  UInt16 textColor565;
  UInt8 foreAlpha, backAlpha;
  WinDrawOperation transferMode;
  PatternType pattern;
  UInt8 patternData[8];
  UnderlineModeType underlineMode; // XXX it is not being used. Change WinDrawChars() to use this attribute.
  UInt16 density, width, height, depth, depth0, coordSys;
  WinHandle displayWindow;
  WinHandle activeWindow;
  WinHandle drawWindow;
  DrawStateType state[DrawStateStackSize];
  FullColorTableType fcolorTable;
  ColorTableType *colorTable;
  int numPush;
} win_module_t;

typedef struct {
  WinHandle wh;
  RectangleType rect;
  UInt16 coordSys;
} win_surface_t;

extern thread_key_t *win_key;

/*
static void setTableEntry(UIColorTableEntries i, UInt8 r, UInt8 g, UInt8 b) {
  RGBColorType rgb;

  rgb.r = r;
  rgb.g = g;
  rgb.b = b;
  UIColorSetTableEntry(i, &rgb);
}
*/

static void directAccessHack(WinHandle wh, uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
  uint8_t *bits = wh->bitmapP ? BmpGetBits(wh->bitmapP) : NULL;
  uint32_t addr = bits ? bits - (uint8_t *)pumpkin_heap_base() : 0;
  put2b(width,  (uint8_t *)wh,  0);
  put2b(height, (uint8_t *)wh,  2);
  put4b(addr,   (uint8_t *)wh,  4);
  put2b(x,      (uint8_t *)wh, 10);
  put2b(y,      (uint8_t *)wh, 12);
  put2b(width,  (uint8_t *)wh, 14);
  put2b(height, (uint8_t *)wh, 16);
}

int WinInitModule(UInt16 density, UInt16 width, UInt16 height, UInt16 depth, WinHandle displayWindow) {
  win_module_t *module;
  Err err;

  if ((module = xcalloc(1, sizeof(win_module_t))) == NULL) {
    return -1;
  }

  thread_set(win_key, module);

  module->density = density;
  module->width = width;
  module->height = height;
  module->depth = depth;
  module->depth0 = depth;

  module->pattern = whitePattern;
  module->coordSys = kCoordinatesStandard;

  module->foreColor565 = 0x0000; // black
  module->backColor565 = 0xffff; // white
  module->textColor565 = 0x0000; // black

  module->foreColor = 0xff; // black
  module->backColor = 0x00; // white
  module->textColor = 0xff; // black

  module->colorTable = (ColorTableType *)&module->fcolorTable;
  MemMove(module->colorTable->entry, defaultPalette, MAX_PAL * sizeof(RGBColorType));
  module->colorTable->numEntries = MAX_PAL;

  module->foreColorRGB = module->colorTable->entry[module->foreColor];
  module->backColorRGB = module->colorTable->entry[module->backColor];
  module->textColorRGB = module->colorTable->entry[module->textColor];

  if (displayWindow) {
    module->displayWindow = displayWindow;
  } else {
    module->displayWindow = pumpkin_heap_alloc(sizeof(WindowType), "Window");
    module->displayWindow->windowBounds.extent.x = width/2;
    module->displayWindow->windowBounds.extent.y = height/2;
    module->displayWindow->windowFlags.freeBitmap = true;
    module->displayWindow->bitmapP = BmpCreate3(width, height, module->density, module->depth, false, 0, NULL, &err);
    module->displayWindow->density = module->density;
    directAccessHack(module->displayWindow, 0, 0, width/2, height/2);
    dbg_add(0, module->displayWindow->bitmapP);
  }

  module->activeWindow = module->displayWindow;
  module->drawWindow = module->displayWindow;
  WinEraseWindow();
//debug(1, "XXX", "WinInitModule displayWindow=%p", module->displayWindow);

/*
  setTableEntry(UIFormFill,           0x30, 0x30, 0x30);
  setTableEntry(UIFormFrame,          0x20, 0x10, 0x10);
  setTableEntry(UIFieldBackground,    0x30, 0x30, 0x30);
  setTableEntry(UIFieldTextLines,     0x60, 0x60, 0x60);
  setTableEntry(UIFieldText,          0xFF, 0xFF, 0xFF);
  setTableEntry(UIFieldTextHighlightBackground, 0xFF, 0x80, 0x00);
  setTableEntry(UIFormTitle,          0xFF, 0xFF, 0xFF);
  setTableEntry(UIMenuFill,           0x30, 0x30, 0x30);
  setTableEntry(UIMenuForeground,     0xFF, 0xFF, 0xFF);
  setTableEntry(UIObjectFrame,        0xB0, 0x70, 0x00);
  setTableEntry(UIObjectForeground,   0xFF, 0xFF, 0xFF
  setTableEntry(UIObjectFill,         0x80, 0x40, 0x00);
  setTableEntry(UIObjectSelectedForeground, 0xFF, 0x60, 0x00);
  setTableEntry(UIObjectSelectedFill, 0x20, 0x00, 0x00);
*/

  module->foreAlpha = 0xFF;
  module->backAlpha = 0xFF;

  return 0;
}

void *WinReinitModule(void *module) {
  win_module_t *old = NULL;

  if (module) {
    WinFinishModule(false);
    thread_set(win_key, module);
  } else {
    old = (win_module_t *)thread_get(win_key);
    WinInitModule(old->density, old->width, old->height, old->depth0, old->displayWindow);
  }

  return old;
}

int WinFinishModule(Boolean deleteDisplay) {
  win_module_t *module = (win_module_t *)thread_get(win_key);

  if (module) {
    if (deleteDisplay) {
      dbg_delete(module->displayWindow->bitmapP);
      if (module->displayWindow->bitmapP) BmpDelete(module->displayWindow->bitmapP);
      pumpkin_heap_free(module->displayWindow, "Window");
    }
    xfree(module);
  }

  return 0;
}

static void pointTo(UInt16 density, Coord *x, Coord *y) {
  win_module_t *module = (win_module_t *)thread_get(win_key);

  switch (density) {
    case kDensityLow:
      switch (module->coordSys) {
        case kCoordinatesDouble:
          if (x) *x = *x / 2;
          if (y) *y = *y / 2;
          break;
      }
      break;
    case kDensityDouble:
      switch (module->coordSys) {
        case kCoordinatesStandard:
          if (x) *x = *x * 2;
          if (y) *y = *y * 2;
          break;
      }
      break;
  }
}

static void pointFrom(UInt16 density, Coord *x, Coord *y) {
  win_module_t *module = (win_module_t *)thread_get(win_key);

  switch (density) {
    case kDensityLow:
      switch (module->coordSys) {
        case kCoordinatesDouble:
          if (x) *x = *x * 2;
          if (y) *y = *y * 2;
          break;
      }
      break;
    case kDensityDouble:
      switch (module->coordSys) {
        case kCoordinatesStandard:
          if (x) *x = *x / 2;
          if (y) *y = *y / 2;
          break;
      }
      break;
  }
}

ColorTableType *pumpkin_defaultcolorTable(void) {
  win_module_t *module = (win_module_t *)thread_get(win_key);
  return module->colorTable;
}

Boolean WinValidateHandle(WinHandle winHandle) {
  return winHandle != NULL;
}

WinHandle WinCreateWindow(const RectangleType *bounds, FrameType frame, Boolean modal, Boolean focusable, UInt16 *error) {
  WinHandle wh;

  // XXX The docs say "uses the bitmap and drawing state of the current draw window", but it is not happening here

  if ((wh = WinCreateOffscreenWindow(bounds->extent.x, bounds->extent.y, nativeFormat, error)) != NULL) {
    wh->windowBounds.topLeft.x = bounds->topLeft.x;
    wh->windowBounds.topLeft.y = bounds->topLeft.y;
    wh->frameType.word = frame;
    wh->windowFlags.modal = modal;
    wh->windowFlags.focusable = focusable;
  }

  return wh;
}

WinHandle WinCreateBitmapWindow(BitmapType *bitmapP, UInt16 *error) {
  WinHandle wh = NULL;
  Coord width, height;
  Err err = sysErrNoFreeResource;

  if (bitmapP) {
    width = bitmapP->width;
    height = bitmapP->height;

    if ((wh = pumpkin_heap_alloc(sizeof(WindowType), "Window")) != NULL) {
      wh->bitmapP = bitmapP;
      wh->windowFlags.freeBitmap = false;
      RctSetRectangle(&wh->windowBounds, 0, 0, width, height);
      //WinSetClipingBounds(wh, &wh->windowBounds);

      directAccessHack(wh, 0, 0, width, height);
      wh->density = BmpGetDensity(bitmapP);
      err = errNone;
    }
//debug(1, "XXX", "WinCreateBitmapWindow %dx%d bitmap %p: %p", width, height, bitmapP, wh);
  }

  if (error) *error = err;

  return wh;
}

void WinDeleteWindow(WinHandle winHandle, Boolean eraseIt) {
  if (winHandle) {
    if (winHandle->bitmapP && winHandle->windowFlags.freeBitmap) {
      debug(DEBUG_TRACE, "Window", "WinDeleteWindow BmpDelete %p", winHandle->bitmapP);
      BmpDelete(winHandle->bitmapP);
    }
    pumpkin_heap_free(winHandle, "Window");
  }
}

void WinInitializeWindow(WinHandle winHandle) {
  // system use only
  debug(DEBUG_ERROR, "Window", "WinInitializeWindow not implemented");
}

void WinAddWindow(WinHandle winHandle) {
  // system use only
  debug(DEBUG_ERROR, "Window", "WinAddWindow not implemented");
}

void WinRemoveWindow(WinHandle winHandle) {
  // system use only
  debug(DEBUG_ERROR, "Window", "WinRemoveWindow not implemented");
}

void WinMoveWindowAddr(WindowType *oldLocationP, WindowType *newLocationP) {
  // system use only
  debug(DEBUG_ERROR, "Window", "WinMoveWindowAddr not implemented");
}

void WinSetActiveWindow(WinHandle winHandle) {
  win_module_t *module = (win_module_t *)thread_get(win_key);
  module->activeWindow = winHandle;
//debug(1, "XXX", "WinSetActiveWindow %p", module->activeWindow);
}

WinHandle WinSetDrawWindow(WinHandle winHandle) {
  win_module_t *module = (win_module_t *)thread_get(win_key);
  WinHandle prev = module->drawWindow;
  module->drawWindow = winHandle;
  debug(DEBUG_TRACE, "Window", "WinSetDrawWindow %p", module->drawWindow);
  return prev;
}

WinHandle WinGetDrawWindow(void) {
  win_module_t *module = (win_module_t *)thread_get(win_key);
  return module->drawWindow;
}

WinHandle WinGetActiveWindow(void) {
  win_module_t *module = (win_module_t *)thread_get(win_key);
  return module->activeWindow;
}

WinHandle WinGetDisplayWindow(void) {
  win_module_t *module = (win_module_t *)thread_get(win_key);
  return module->displayWindow;
}

WinHandle WinGetFirstWindow(void) {
  debug(DEBUG_ERROR, "Window", "WinGetFirstWindow not implemented");
  return NULL;
}

void WinEnableWindow(WinHandle winHandle) {
  // system use only
  debug(DEBUG_ERROR, "Window", "WinEnableWindow not implemented");
}

void WinDisableWindow(WinHandle winHandle) {
  // system use only
  debug(DEBUG_ERROR, "Window", "WinDisableWindow not implemented");
}

Int16 WinGetBorderRect(WinHandle wh, RectangleType *rect) {
  Int16 xmargin = 0, ymargin = 0;

  MemMove(rect, &wh->windowBounds, sizeof(RectangleType));
  if (wh->windowFlags.modal) {
    if (wh->windowBounds.topLeft.x >= 2) {
      xmargin = 2;
    } else {
      xmargin = wh->windowBounds.topLeft.x;
    }
    if (wh->windowBounds.topLeft.y >= 2) {
      ymargin = 2;
    } else {
      ymargin = wh->windowBounds.topLeft.y;
    }

    rect->topLeft.x -= xmargin;
    rect->topLeft.y -= xmargin;
    rect->extent.x += 2*ymargin;
    rect->extent.y += 2*ymargin;
  }

  return xmargin < ymargin ? xmargin : ymargin;
}

// Return a rectangle, in display-relative coordinates, that defines the
// size and location of a window and its frame.

void WinGetWindowFrameRect(WinHandle winHandle, RectangleType *r) {
  WinGetBorderRect(winHandle, r);
}

void WinDrawWindowFrame(void) {
  debug(DEBUG_ERROR, "Window", "WinDrawWindowFrame not implemented");
}

// Create an offscreen window and copy the specified region from the draw window to the offscreen window.
// The offscreen window is the same size as the region to copy.
// This function tries to copy the window’s bitmap using compressed
// format if possible. It may display a fatal error message if an error
// occurs when it tries to shrink the pointer for the compressed bits.

WinHandle WinSaveBits(const RectangleType *source, UInt16 *error) {
  WinHandle wh = NULL;

  if (source) {
    debug(DEBUG_TRACE, "Window", "WinSaveBits %d,%d,%d,%d", source->topLeft.x, source->topLeft.y, source->extent.x, source->extent.y);
    wh = WinCreateOffscreenWindow(source->extent.x, source->extent.y, nativeFormat, error);
    if (wh) {
      WinCopyRectangle(WinGetDrawWindow(), wh, source, 0, 0, winPaint);
    }
  }

  return wh;
}

// Copy the contents of the specified window to the draw window and delete the passed window.
// This routine is generally used to restore a region of the display that was saved with WinSaveBits.
// destX: x coordinate in the draw window to copy to.
// destY: y coordinate in the draw window to copy to.

void WinRestoreBits(WinHandle winHandle, Coord destX, Coord destY) {
  RectangleType rect;

  if (winHandle) {
    debug(DEBUG_TRACE, "Window", "WinRestoreBits %d,%d", destX, destY);
    RctSetRectangle(&rect, 0, 0, winHandle->windowBounds.extent.x, winHandle->windowBounds.extent.y);
    WinCopyRectangle(winHandle, WinGetDrawWindow(), &rect, destX, destY, winPaint);
    WinDeleteWindow(winHandle, false);
  }
}

void WinSetDisplayExtent(Coord extentX, Coord extentY) {
  win_module_t *module = (win_module_t *)thread_get(win_key);
  Err err;

  pointFrom(module->density, &extentX, &extentY);
  module->width = extentX;
  module->height = extentY;

  module->displayWindow->windowBounds.extent.x = module->width/2;
  module->displayWindow->windowBounds.extent.y = module->height/2;
  if (module->displayWindow->bitmapP) {
    debug(DEBUG_TRACE, "Window", "WinSetDisplayExtent BmpDelete %p", module->displayWindow->bitmapP);
    BmpDelete(module->displayWindow->bitmapP);
  }
  module->displayWindow->bitmapP = BmpCreate3(module->width, module->height, module->density, module->depth, false, 0, NULL, &err);
  directAccessHack(module->displayWindow, 0, 0, module->width/2, module->height/2);
}

void WinGetDisplayExtent(Coord *extentX, Coord *extentY) {
  win_module_t *module = (win_module_t *)thread_get(win_key);

  if (extentX) *extentX = module->width;
  if (extentY) *extentY = module->height;
  pointFrom(module->density, extentX, extentY);
}

void WinGetDrawWindowBounds(RectangleType *rP) {
  WinGetBounds(WinGetDrawWindow(), rP);
}

// Return the bounds of a window in display-relative coordinates.
void WinGetBounds(WinHandle winH, RectangleType *rP) {
  if (winH && rP) {
    MemMove(rP, &winH->windowBounds, sizeof(RectangleType));
    WinScaleRectangle(rP);
  }
}

// Set the bounds of the window to display-relative coordinates.
// A visible window cannot have its bounds modified.
void WinSetBounds(WinHandle winHandle, const RectangleType *rP) {
  win_module_t *module = (win_module_t *)thread_get(win_key);
  BitmapType *bmp, *old;
  WinHandle prev;
  UInt32 density, depth;
  Coord width, height;
  UInt16 prevCoordSys;
  Err err;

  if (winHandle && rP && (rP->extent.x  != winHandle->windowBounds.extent.x  || rP->extent.y  != winHandle->windowBounds.extent.y ||
                          rP->topLeft.x != winHandle->windowBounds.topLeft.x || rP->topLeft.y != winHandle->windowBounds.topLeft.y)) {
    MemMove(&winHandle->windowBounds, rP, sizeof(RectangleType));
    WinUnscaleRectangle(&winHandle->windowBounds);
    //put2b(winHandle->windowBounds.topLeft.x, (uint8_t *)winHandle, 10);
    //put2b(winHandle->windowBounds.topLeft.y, (uint8_t *)winHandle, 12);
    //put2b(winHandle->windowBounds.extent.x,  (uint8_t *)winHandle, 14);
    //put2b(winHandle->windowBounds.extent.y,  (uint8_t *)winHandle, 16);

    width = winHandle->windowBounds.extent.x;
    height = winHandle->windowBounds.extent.y;
    prevCoordSys = WinSetCoordinateSystem(module->density == kDensityDouble ? kCoordinatesDouble : kCoordinatesStandard);
    width = WinScaleCoord(width, false);
    height = WinScaleCoord(height, false);
    WinSetCoordinateSystem(prevCoordSys);
    WinScreenGetAttribute(winScreenDensity, &density);
    WinScreenMode(winScreenModeGetDefaults, NULL, NULL, &depth, NULL);
    bmp = BmpCreate3(width, height, density, depth, false, 0, NULL, &err);
    if (bmp) {
      old = winHandle->bitmapP;
      winHandle->bitmapP = bmp;
      prev = WinSetDrawWindow(winHandle);
      WinPaintBitmap(old, 0, 0);
      WinSetDrawWindow(prev);
      debug(DEBUG_TRACE, "Window", "WinSetBounds BmpDelete %p", old);
      BmpDelete(old);
    }
    directAccessHack(winHandle, winHandle->windowBounds.topLeft.x, winHandle->windowBounds.topLeft.y, winHandle->windowBounds.extent.x, winHandle->windowBounds.extent.y);
  }
}

void WinGetWindowExtent(Coord *extentX, Coord *extentY) {
  win_module_t *module = (win_module_t *)thread_get(win_key);

  if (module->drawWindow) {
    if (extentX) *extentX = module->drawWindow->windowBounds.extent.x;
    if (extentY) *extentY = module->drawWindow->windowBounds.extent.y;
  }
}

// Convert a display-relative coordinate to a window-relative coordinate. The coordinate returned is relative to the display window.
void WinDisplayToWindowPt(Coord *extentX, Coord *extentY) {
}

// Convert a window-relative coordinate to a display-relative coordinate. The coordinate passed is assumed to be relative to the draw window.
void WinWindowToDisplayPt(Coord *extentX, Coord *extentY) {
  win_module_t *module = (win_module_t *)thread_get(win_key);

  if (module->drawWindow) {
    if (extentX) *extentX += module->drawWindow->windowBounds.topLeft.x;
    if (extentY) *extentY += module->drawWindow->windowBounds.topLeft.y;
  }
}

BitmapType *WinGetBitmap(WinHandle winHandle) {
  return winHandle ? winHandle->bitmapP : NULL;
}

// Note that the bounds and clippingBounds fields in the WindowType data structure are always stored using native coordinates.
// The various functions that access these fields convert the native coordinates to the coordinate system being used by the window.

void WinSetClipingBounds(WinHandle wh, const RectangleType *rP) {
  win_module_t *module = (win_module_t *)thread_get(win_key);
  Coord x1, y1, x2, y2;

  if (wh && rP) {
    x1 = rP->topLeft.x;
    y1 = rP->topLeft.y;
    x2 = rP->extent.x > 0 ? x1 + rP->extent.x - 1 : x1;
    y2 = rP->extent.y > 0 ? y1 + rP->extent.y - 1 : y1;

    if (module->density == kDensityDouble && WinGetCoordinateSystem() == kCoordinatesStandard) {
      x1 = x1 << 1;
      y1 = y1 << 1;
      x2 = x2 << 1;
      y2 = y2 << 1;
      if (x2 > x1) x2++;
      if (y2 > y1) y2++;
    }
//debug(1, "XXX", "WinSetClipingBounds %p (%d,%d,%d,%d)", wh, x1, y1, x2, y2);

    debug(DEBUG_TRACE, "Window", "WinSetClipingBounds (%d,%d,%d,%d) -> (%d,%d,%d,%d)",
      rP->topLeft.x, rP->topLeft.y, rP->extent.x, rP->extent.y, x1, y1, x2, y2);
    wh->clippingBounds.left = x1;
    wh->clippingBounds.right = x2;
    wh->clippingBounds.top = y1;
    wh->clippingBounds.bottom = y2;
  }
}

void WinSetClip(const RectangleType *rP) {
  win_module_t *module = (win_module_t *)thread_get(win_key);

  if (module->drawWindow) {
    WinSetClipingBounds(module->drawWindow, rP);
  }
}

void WinResetClip(void) {
  win_module_t *module = (win_module_t *)thread_get(win_key);

  if (module->drawWindow) {
//debug(1, "XXX", "WinResetClip %p", module->drawWindow);
    module->drawWindow->clippingBounds.left = 0;
    module->drawWindow->clippingBounds.right = 0;
    module->drawWindow->clippingBounds.top = 0;
    module->drawWindow->clippingBounds.bottom = 0;
  }
}

void WinGetClip(RectangleType *rP) {
  win_module_t *module = (win_module_t *)thread_get(win_key);
  Coord x1, y1, x2, y2;

  if (module->drawWindow && rP) {
    x1 = module->drawWindow->clippingBounds.left;
    x2 = module->drawWindow->clippingBounds.right;
    y1 = module->drawWindow->clippingBounds.top;
    y2 = module->drawWindow->clippingBounds.bottom;

    if (module->density == kDensityDouble && WinGetCoordinateSystem() == kCoordinatesStandard) {
      x1 = x1 >> 1;
      y1 = y1 >> 1;
      x2 = x2 >> 1;
      y2 = y2 >> 1;
    }

    if (x1 < x2 && y1 < y2) {
      RctSetRectangle(rP, x1, y1, x2 - x1 + 1, y2 - y1 + 1);
    } else {
      RctSetRectangle(rP, 0, 0, 0, 0);
    }
  }
}

void WinClipRectangle(RectangleType *rP) {
  win_module_t *module = (win_module_t *)thread_get(win_key);
  Coord x1, y1, x2, y2;

  if (module->drawWindow) {
    if (rP && !(module->drawWindow->clippingBounds.left == 0 && module->drawWindow->clippingBounds.right == 0)) {
      x1 = rP->topLeft.x;
      y1 = rP->topLeft.y;
      x2 = x1 + rP->extent.x - 1;
      y2 = y1 + rP->extent.y - 1;

      if (module->density == kDensityDouble && WinGetCoordinateSystem() == kCoordinatesStandard) {
        x1 = x1 << 1;
        y1 = y1 << 1;
        x2 = (x2 << 1) + 1;
        y2 = (y2 << 1) + 1;
      }

      if (x1 <= module->drawWindow->clippingBounds.right  && x2 >= module->drawWindow->clippingBounds.left &&
          y1 <= module->drawWindow->clippingBounds.bottom && y2 >= module->drawWindow->clippingBounds.top) {

        if (x1 < module->drawWindow->clippingBounds.left) {
          x1 = module->drawWindow->clippingBounds.left;
        }
        if (x2 > module->drawWindow->clippingBounds.right) {
          x2 = module->drawWindow->clippingBounds.right;
        }
        if (y1 < module->drawWindow->clippingBounds.top) {
          y1 = module->drawWindow->clippingBounds.top;
        }
        if (y2 > module->drawWindow->clippingBounds.bottom) {
          y2 = module->drawWindow->clippingBounds.bottom;
        }

        if (module->density == kDensityDouble && WinGetCoordinateSystem() == kCoordinatesStandard) {
          x1 = x1 >> 1;
          y1 = y1 >> 1;
          x2 = x2 >> 1;
          y2 = y2 >> 1;
        }

        RctSetRectangle(rP, x1, x2, x2 - x1 + 1, y2 - y1 + 1);

      } else {
        // no intersection
        RctSetRectangle(rP, 0, 0, 0, 0);
      }
    }
  }
}

Boolean WinModal(WinHandle winHandle) {
  return winHandle ? winHandle->windowFlags.modal : false;
}

static IndexedColorType getBit(WinHandle wh, Coord x, Coord y, RGBColorType *rgb) {
  IndexedColorType p = 0;

  switch (BmpGetDensity(wh->bitmapP)) {
    case kDensityLow:
      switch (WinGetCoordinateSystem()) {
        case kCoordinatesStandard:
          if (rgb) BmpGetPixelRGB(wh->bitmapP, x, y, rgb);
          else p = BmpGetPixel(wh->bitmapP, x, y);
          break;
        case kCoordinatesDouble:
          if (rgb) BmpGetPixelRGB(wh->bitmapP, x/2, y/2, rgb);
          else p = BmpGetPixel(wh->bitmapP, x/2, y/2);
          break;
      }
      break;
    case kDensityDouble:
      switch (WinGetCoordinateSystem()) {
        case kCoordinatesStandard:
          if (rgb) BmpGetPixelRGB(wh->bitmapP, x*2, y*2, rgb);
          else p = BmpGetPixel(wh->bitmapP, x*2, y*2);
          break;
        case kCoordinatesDouble:
          if (rgb) BmpGetPixelRGB(wh->bitmapP, x, y, rgb);
          else p = BmpGetPixel(wh->bitmapP, x, y);
          break;
      }
      break;
  }

  return p;
}

// Return the color value of a pixel in the current draw window
IndexedColorType WinGetPixel(Coord x, Coord y) {
  win_module_t *module = (win_module_t *)thread_get(win_key);
  IndexedColorType p = 0;

  if (module->drawWindow) {
    p = getBit(module->drawWindow, x, y, NULL);
  }

  return p;
}

// Return the RGB color values of a pixel in the current draw window
Err WinGetPixelRGB(Coord x, Coord y, RGBColorType *rgbP) {
  win_module_t *module = (win_module_t *)thread_get(win_key);
  Err err = sysErrParamErr;

  if (module->drawWindow && rgbP) {
    getBit(module->drawWindow, x, y, rgbP);
    err = errNone;
  }

  return err;
}

void WinAdjustCoords(Coord *x, Coord *y) {
  win_module_t *module = (win_module_t *)thread_get(win_key);
  pointTo(module->density, x, y);
}

void WinAdjustCoordsInv(Coord *x, Coord *y) {
  win_module_t *module = (win_module_t *)thread_get(win_key);
  pointFrom(module->density, x, y);
}

static void WinAdjustCoordEnd(Coord *c, UInt16 cs) {
  win_module_t *module = (win_module_t *)thread_get(win_key);

  switch (module->density) {
    case kDensityLow:
      switch (cs) {
        case kCoordinatesDouble:    *c /= 2; break;
      }
      break;
    case kDensityDouble:
      switch (cs) {
        case kCoordinatesStandard:  *c = *c * 2 + 1; break;
      }
      break;
  }
}

static void dirty_region(Coord x1, Coord y1, Coord x2, Coord y2) {
  win_module_t *module = (win_module_t *)thread_get(win_key);
  Coord xx1, yy1, xx2, yy2, aux;

//debug(1, "XXX", "dirty_region (%d,%d,%d,%d) ...", x1, y1, x2, y2);
  if (x1 > x2) {
    aux = x1;
    x1 = x2;
    x2 = aux;
  }
  if (y1 > y2) {
    aux = y1;
    y1 = y2;
    y2 = aux;
  }

  xx1 = x1;
  yy1 = y1;
  pointTo(module->density, &xx1, &yy1);
  xx2 = x2;
  yy2 = y2;
  WinAdjustCoordEnd(&xx2, module->coordSys);
  WinAdjustCoordEnd(&yy2, module->coordSys);
//debug(1, "XXX", "dirty_region adjusted (%d,%d,%d,%d)", xx1, yy1, xx2, yy2);

  pumpkin_screen_dirty(module->activeWindow, xx1, yy1, xx2-xx1+1, yy2-yy1+1);
//debug(1, "XXX", "dirty_region done");
}

//#define CLIP_OK(left,right,top,bottom,x,y) 1
#define CLIP_OK(left,right,top,bottom,x,y) (left == 0 && right == 0) || ((x) >= left && (x) <= right && (y) >= top && (y) <= bottom)

#define CLIPW_OK(wh,x,y) CLIP_OK(wh->clippingBounds.left,wh->clippingBounds.right,wh->clippingBounds.top,wh->clippingBounds.bottom,x,y)

static void WinPutBit(WinHandle wh, Coord x, Coord y, UInt32 b, WinDrawOperation mode) {
  //pointTo(&x, &y);
  pointTo(wh->density, &x, &y);
  if (CLIPW_OK(wh, x, y)) {
    //Boolean dbl = BmpGetDensity(wh->bitmapP) == kDensityDouble && WinGetCoordinateSystem() == kCoordinatesStandard;
    Boolean dbl = wh->density == kDensityDouble && WinGetCoordinateSystem() == kCoordinatesStandard;
    BmpPutBit(b, false, wh->bitmapP, x, y, mode, dbl);
  }
}

static void WinPutBitDisplay(WinHandle wh, Coord x, Coord y, UInt32 windowColor, UInt32 displayColor, WinDrawOperation mode) {
  win_module_t *module = (win_module_t *)thread_get(win_key);
  Coord x0, y0;

  if (wh) {
    WinPutBit(wh, x, y, windowColor, mode);

    if (wh == module->activeWindow && wh != module->displayWindow) {
      x0 = wh->windowBounds.topLeft.x;
      y0 = wh->windowBounds.topLeft.y;
      if (module->coordSys == kCoordinatesDouble) {
        x0 <<= 1;
        y0 <<= 1;
      }
      WinPutBit(module->displayWindow, x0 + x, y0 + y, displayColor, mode);
    }
  } else {
    debug(DEBUG_ERROR, "Window", "WinPutBitDisplay null wh");
  }
}

static UInt32 getColor(win_module_t *module, UInt16 depth, Boolean back) {
  UInt32 c = 0;

  switch (depth) {
    case 1:
    case 2:
    case 4:
    case 8:
      c = back ? module->backColor : module->foreColor;
      break;
    case 16:
      c = back ? module->backColor565 : module->foreColor565;
      break;
    case 24:
      if (back) {
        c = rgb24(module->backColorRGB.r, module->backColorRGB.g, module->backColorRGB.b);
      } else {
        c = rgb24(module->foreColorRGB.r, module->foreColorRGB.g, module->foreColorRGB.b);
      }
      break;
    case 32:
      if (back) {
        c = rgb32(module->backColorRGB.r, module->backColorRGB.g, module->backColorRGB.b);
      } else {
        c = rgb32(module->foreColorRGB.r, module->foreColorRGB.g, module->foreColorRGB.b);
      }
      break;
  }

  return c;
}

static UInt32 getPattern(WinHandle wh, Coord x, Coord y, PatternType pattern) {
  win_module_t *module = (win_module_t *)thread_get(win_key);
  Coord rx, ry;
  UInt8 b, depth;
  UInt32 c1, c2, c = 0;

  if (wh) {
    depth = BmpGetBitDepth(wh->bitmapP);

    switch (pattern) {
      case blackPattern:
        //c = depth == 16 ? module->foreColor565 : module->foreColor;
        c = getColor(module, depth, false);
        break;
      case whitePattern:
        //c = depth == 16 ? module->backColor565 : module->backColor;
        c = getColor(module, depth, true);
        break;
      case grayPattern:
      case lightGrayPattern:
      case darkGrayPattern:
        //c1 = depth == 16 ? module->foreColor565 : module->foreColor;
        //c2 = depth == 16 ? module->backColor565 : module->backColor;
        c1 = getColor(module, depth, false);
        c2 = getColor(module, depth, true);
        rx = x % 2;
        ry = y % 2;
        c = (rx == ry) ? c1 : c2;
        break;
      case customPattern:
        //c1 = depth == 16 ? module->foreColor565 : module->foreColor;
        //c2 = depth == 16 ? module->backColor565 : module->backColor;
        c1 = getColor(module, depth, false);
        c2 = getColor(module, depth, true);
        rx = x % 8;
        ry = y % 8;
        b = module->patternData[ry] << rx;
        c = (b & 0x80) ? c1 : c2;
        break;
    }
  }

  return c;
}

static void draw_hline(Coord x1, Coord x2, Coord y, PatternType pattern) {
  win_module_t *module = (win_module_t *)thread_get(win_key);
  Coord x, aux;
  UInt32 c, d;

  if (x1 > x2) {
    aux = x1;
    x1 = x2;
    x2 = aux;
  }

  for (x = x1; x <= x2; x++) {
    c = getPattern(module->drawWindow, x, y, pattern);
    d = getPattern(module->displayWindow, x, y, pattern);
    WinPutBitDisplay(module->drawWindow, x, y, c, d, module->transferMode);
  }
}

static void draw_vline(Coord x, Coord y1, Coord y2, PatternType pattern) {
  win_module_t *module = (win_module_t *)thread_get(win_key);
  Coord y, aux;
  UInt32 c, d;

  if (y1 > y2) {
    aux = y1;
    y1 = y2;
    y2 = aux;
  }

  for (y = y1; y <= y2; y++) {
    c = getPattern(module->drawWindow, x, y, pattern);
    d = getPattern(module->displayWindow, x, y, pattern);
    WinPutBitDisplay(module->drawWindow, x, y, c, d, module->transferMode);
  }
}

static void draw_gline(int x1, int y1, int x2, int y2, PatternType pattern) {
  win_module_t *module = (win_module_t *)thread_get(win_key);
  int dx, dy, sx, sy, err, e2;
  UInt32 c, d;

  dx = x2 - x1;
  if (dx < 0) dx = -dx;
  sx = x1 < x2 ? 1 : -1;
  dy = y2 - y1;
  if (dy < 0) dy = -dy;
  sy = y1 < y2 ? 1 : -1;
  err = (dx > dy ? dx : -dy)/2;

  for (;;) {
    c = getPattern(module->drawWindow, x1, y1, pattern);
    d = getPattern(module->displayWindow, x1, y1, pattern);
    WinPutBitDisplay(module->drawWindow, x1, y1, c, d, module->transferMode);
    if (x1 == x2 && y1 == y2) break;
    e2 = err;
    if (e2 > -dx) { err -= dy; x1 += sx; }
    if (e2 <  dy) { err += dx; y1 += sy; }
  }
}

UInt8 WinGetBackAlpha(void) {
  win_module_t *module = (win_module_t *)thread_get(win_key);
  return module->backAlpha;
}

UInt8 WinSetBackAlpha(UInt8 alpha) {
  win_module_t *module = (win_module_t *)thread_get(win_key);
  UInt8 old = module->backAlpha;
  module->backAlpha = alpha;
  return old;
}

void WinEraseWindow(void) {
  win_module_t *module = (win_module_t *)thread_get(win_key);
  RGBColorType back, fore;
  RectangleType rect;
  Coord y;

  if (module->drawWindow) {
    WinGetBounds(module->drawWindow, &rect);
    WinSetBackColorRGB(NULL, &back);
    WinSetForeColorRGB(&back, &fore);
    for (y = 0; y < rect.extent.y; y++) {
      //WinEraseLine(rect.topLeft.x, y, rect.topLeft.x + rect.extent.x - 1, y);
      draw_hline(rect.topLeft.x, rect.topLeft.x + rect.extent.x - 1, y, blackPattern);
    }
    WinSetForeColorRGB(&fore, NULL);
    if (module->drawWindow == module->activeWindow) dirty_region(rect.topLeft.x, rect.topLeft.y, rect.topLeft.x + rect.extent.x - 1, rect.topLeft.y + rect.extent.y - 1);
  }
}

void WinPaintPixel(Coord x, Coord y) {
  win_module_t *module = (win_module_t *)thread_get(win_key);
  UInt32 c, d;

  if (module->drawWindow) {
    c = BmpGetBitDepth(module->drawWindow->bitmapP) == 16 ? module->foreColor565 : module->foreColor;
    d = BmpGetBitDepth(module->displayWindow->bitmapP) == 16 ? module->foreColor565 : module->foreColor;
    WinPutBitDisplay(module->drawWindow, x, y, c, d, module->transferMode);
    if (module->drawWindow == module->activeWindow) dirty_region(x, y, x, y);
  }
}

void WinDrawPixel(Coord x, Coord y) {
  WinDrawOperation prev = WinSetDrawMode(winPaint);
  PatternType oldp = WinGetPatternType();
  WinSetPatternType(blackPattern);
  WinPaintPixel(x, y);
  WinSetPatternType(oldp);
  WinSetDrawMode(prev);
}

void WinErasePixel(Coord x, Coord y) {
  RGBColorType back, fore;
  WinDrawOperation prev = WinSetDrawMode(winPaint);
  PatternType oldp = WinGetPatternType();
  WinSetPatternType(blackPattern);
  WinSetBackColorRGB(NULL, &back);
  WinSetForeColorRGB(&back, &fore);
  WinPaintPixel(x, y);
  WinSetPatternType(oldp);
  WinSetForeColorRGB(&fore, NULL);
  WinSetDrawMode(prev);
}

    //UInt16 prevCoordSys = 0; WinSetCoordinateSystem(module->density == kDensityDouble ? kCoordinatesDouble : kCoordinatesStandard);

#define invertPrefix() \
    WinDrawOperation prevMode = WinSetDrawMode(winInvert); \
    UInt16 prevCoordSys = WinSetCoordinateSystem(module->density == kDensityDouble ? kCoordinatesDouble : kCoordinatesStandard); \
    Boolean isDouble = prevCoordSys == kCoordinatesDouble; \
    RGBColorType back, white; \
    white.r = white.g = white.b = 0xFF; \
    WinSetBackColorRGB(&white, &back); \
    PatternType oldPattern = WinGetPatternType(); \
    WinSetPatternType(whitePattern);

#define invertSuffix() \
    WinSetPatternType(oldPattern); \
    WinSetBackColorRGB(&back, NULL); \
    WinSetCoordinateSystem(prevCoordSys); \
    WinSetDrawMode(prevMode);

void WinInvertPixel(Coord x, Coord y) {
  win_module_t *module = (win_module_t *)thread_get(win_key);
  invertPrefix();
  if (!isDouble) x = WinScaleCoord(x, false);
  if (!isDouble) y = WinScaleCoord(y, false);
  WinPaintPixel(x, y);
  invertSuffix();
}

void WinPaintPixels(UInt16 numPoints, PointType pts[]) {
  UInt16 i;

  if (pts) {
    for (i = 0; i < numPoints; i++) {
      WinPaintPixel(pts[i].x, pts[i].y);
    }
  }
}

void WinPaintLine(Coord x1, Coord y1, Coord x2, Coord y2) {
  win_module_t *module = (win_module_t *)thread_get(win_key);

  if (y1 == y2) {
    draw_hline(x1, x2, y2, module->pattern);
  } else if (x1 == x2) {
    draw_vline(x1, y1, y2, module->pattern);
  } else {
    draw_gline(x1, y1, x2, y2, module->pattern);
  }

  if (module->drawWindow == module->activeWindow) dirty_region(x1, y1, x2, y2);
}

void WinPaintLines(UInt16 numLines, WinLineType lines[]) {
  UInt16 i;

  if (lines) {
    for (i = 0; i < numLines; i++) {
      WinPaintLine(lines[i].x1, lines[i].y1, lines[i].x2, lines[i].y2);
    }
  }
}

void WinDrawLine(Coord x1, Coord y1, Coord x2, Coord y2) {
  WinDrawOperation prev = WinSetDrawMode(winPaint);
  PatternType oldp = WinGetPatternType();
  WinSetPatternType(blackPattern);
  WinPaintLine(x1, y1, x2, y2);
  WinSetPatternType(oldp);
  WinSetDrawMode(prev);
}

void WinEraseLine(Coord x1, Coord y1, Coord x2, Coord y2) {
  RGBColorType back, fore;
  WinDrawOperation prev = WinSetDrawMode(winPaint);
  PatternType oldp = WinGetPatternType();
  WinSetPatternType(blackPattern);
  WinSetBackColorRGB(NULL, &back);
  WinSetForeColorRGB(&back, &fore);
  WinPaintLine(x1, y1, x2, y2);
  WinSetPatternType(oldp);
  WinSetForeColorRGB(&fore, NULL);
  WinSetDrawMode(prev);
}

void WinDrawGrayLine(Coord x1, Coord y1, Coord x2, Coord y2) {
  PatternType oldp = WinGetPatternType();
  WinSetPatternType(grayPattern);
  WinFillLine(x1, y1, x2, y2);
  WinSetPatternType(oldp);
}

void WinInvertLine(Coord x1, Coord y1, Coord x2, Coord y2) {
  win_module_t *module = (win_module_t *)thread_get(win_key);
  invertPrefix();
  if (!isDouble) x1 = WinScaleCoord(x1, false);
  if (!isDouble) y1 = WinScaleCoord(y1, false);
  if (!isDouble) x2 = WinScaleCoord(x2, true);
  if (!isDouble) y2 = WinScaleCoord(y2, true);
  WinPaintLine(x1, y1, x2, y2);
  invertSuffix();
}

// fill=true : draw filled retangle inside rP (including limits)
// fill=false: draw retangle border around rP (ouside limits)
static void WinPaintRectangleF(const RectangleType *rP, Int16 width, Int16 cornerDiam, Boolean fill, Boolean gray) {
  //win_module_t *module = (win_module_t *)thread_get(win_key);
  Coord x1, y1, x2, y2, y, d, aux;
  Int16 i;

  cornerDiam = (cornerDiam + 1) / 2;

  if (rP && cornerDiam >= 0 && rP->extent.x > 0 && rP->extent.y > 0) {
    x1 = rP->topLeft.x;
    y1 = rP->topLeft.y;
    x2 = x1 + rP->extent.x - 1;
    y2 = y1 + rP->extent.y - 1;

    if (y1 > y2) {
      aux = y1;
      y1 = y2;
      y2 = aux;
    }

    if (cornerDiam == 0) {
      if (fill) {
        for (y = y1; y <= y2; y++) {
          //draw_hline(x1, x2, y, module->pattern);
          WinPaintLine(x1, y, x2, y);
        }
      } else {
        for (i = 0; i < width; i++) {
          if (gray) {
            WinDrawGrayLine(x1-1-i, y1-1-i, x2+1+i, y1-1-i);
            WinDrawGrayLine(x1-1-i, y2+1+i, x2+1+i, y2+1+i);
            WinDrawGrayLine(x1-1-i, y1-1-i, x1-1-i, y2+1+i);
            WinDrawGrayLine(x2+1+i, y1-1-i, x2+1+i, y2+1+i);
            //draw_hline(x1-1-i, x2+1+i, y1-1-i, grayPattern);
            //draw_hline(x1-1-i, x2+1+i, y2+1+i, grayPattern);
            //draw_vline(x1-1-i, y1-1-i, y2+1+i, grayPattern);
            //draw_vline(x2+1+i, y1-1-i, y2+1+i, grayPattern);
          } else {
            WinPaintLine(x1-1-i, y1-1-i, x2+1+i, y1-1-i);
            WinPaintLine(x1-1-i, y2+1+i, x2+1+i, y2+1+i);
            WinPaintLine(x1-1-i, y1-1-i, x1-1-i, y2+1+i);
            WinPaintLine(x2+1+i, y1-1-i, x2+1+i, y2+1+i);
            //draw_hline(x1-1-i, x2+1+i, y1-1-i, module->pattern);
            //draw_hline(x1-1-i, x2+1+i, y2+1+i, module->pattern);
            //draw_vline(x1-1-i, y1-1-i, y2+1+i, module->pattern);
            //draw_vline(x2+1+i, y1-1-i, y2+1+i, module->pattern);
          }
        }
      }
      //if (module->drawWindow == module->activeWindow) dirty_region(x1-1, y1-1, x2+1, y2+1);
      return;
    }

    if (cornerDiam > rP->extent.y/2) cornerDiam = rP->extent.y/2;

    d = cornerDiam;
    if (!fill) {
      for (i = 0; i < width; i++) {
        if (gray) {
          WinDrawGrayLine(x1+d, y1-1-i, x2-d, y1-1-i);
          WinDrawGrayLine(x1+d, y2+1+i, x2-d, y2+1+i);
        } else {
          WinPaintLine(x1+d, y1-1-i, x2-d, y1-1-i);
          WinPaintLine(x1+d, y2+1+i, x2-d, y2+1+i);
        }
      }
    }

    for (y = y1; y <= y1+cornerDiam; y++) {
      if (fill) {
        WinPaintLine(x1+d, y, x2-d, y);
        //draw_hline(x1+d, x2-d, y, module->pattern);
      } else {
        if (!gray || (y % 2) == 0) {
          for (i = 0; i < width; i++) {
            WinPaintPixel(x1+d-1-i, y);
            WinPaintPixel(x2-d+1+i, y);
          }
        }
      }
      d--;
    }
    for (; y < y2-cornerDiam; y++) {
      if (fill) {
        WinPaintLine(x1, y, x2, y);
        //draw_hline(x1, x2, y, module->pattern);
      } else {
        if (!gray || (y % 2) == 0) {
          for (i = 0; i < width; i++) {
            WinPaintPixel(x1-1-i, y);
            WinPaintPixel(x2+1+i, y);
          }
        }
      }
    }
    d = 0;
    for (; y <= y2; y++) {
      if (fill) {
        WinPaintLine(x1+d, y, x2-d, y);
        //draw_hline(x1+d, x2-d, y, module->pattern);
      } else {
        if (!gray || (y % 2) == 0) {
          for (i = 0; i < width; i++) {
            WinPaintPixel(x1+d-1-i, y);
            WinPaintPixel(x2-d+1+i, y);
          }
        }
      }
      d++;
    }
    //if (fill && module->drawWindow == module->activeWindow) dirty_region(x1-1, y1-1, x2+1, y2+1);
  }
}

void WinPaintRectangle(const RectangleType *rP, UInt16 cornerDiam) {
  win_module_t *module = (win_module_t *)thread_get(win_key);
  PatternType oldp = module->pattern;
  module->pattern = blackPattern; // XXX is this correct ? if it is not blackPattern, ChemTable does not paint the whole cell
  WinPaintRectangleF(rP, 1, cornerDiam, true, false);
  module->pattern = oldp;
}

void WinDrawRectangle(const RectangleType *rP, UInt16 cornerDiam) {
  WinDrawOperation prev = WinSetDrawMode(winPaint);
  PatternType oldp = WinGetPatternType();
  WinSetPatternType(blackPattern);
  WinPaintRectangle(rP, cornerDiam);
  WinSetPatternType(oldp);
  WinSetDrawMode(prev);
}

void WinEraseRectangle(const RectangleType *rP, UInt16 cornerDiam) {
  win_module_t *module = (win_module_t *)thread_get(win_key);
  RGBColorType back, fore;
  UInt8 alpha;
  Boolean f = false;

  WinDrawOperation prev = WinSetDrawMode(winPaint);
  WinSetBackColorRGB(NULL, &back);
  WinSetForeColorRGB(&back, &fore);

  if (module->foreColor == UIColorGetTableEntryIndex(UIFormFill)) {
    alpha = module->foreAlpha;
    module->foreAlpha = FORM_FILL_ALPHA;
    f = true;
  }

  WinPaintRectangle(rP, cornerDiam);

  if (f) {
    module->foreAlpha = alpha;
  }

  WinSetForeColorRGB(&fore, NULL);
  WinSetDrawMode(prev);
}

void WinInvertRectangle(const RectangleType *rP, UInt16 cornerDiam) {
  win_module_t *module = (win_module_t *)thread_get(win_key);
  if (rP) {
    invertPrefix();
    RectangleType rect;
    MemMove(&rect, rP, sizeof(RectangleType));
    if (!isDouble) WinScaleRectangle(&rect);
    if (!isDouble) cornerDiam = WinScaleCoord(cornerDiam, false);
    WinPaintRectangleF(&rect, 1, cornerDiam, true, false);
    invertSuffix();
  }
}

void WinFillLine(Coord x1, Coord y1, Coord x2, Coord y2) {
  win_module_t *module = (win_module_t *)thread_get(win_key);

  if (y1 == y2) {
    draw_hline(x1, x2, y2, module->pattern);
  } else if (x1 == x2) {
    draw_vline(x1, y1, y2, module->pattern);
  } else {
    draw_gline(x1, y1, x2, y2, module->pattern);
  }
  if (module->drawWindow == module->activeWindow) dirty_region(x1, y1, x2, y2);
}

void WinFillRectangle(const RectangleType *rP, UInt16 cornerDiam) {
  win_module_t *module = (win_module_t *)thread_get(win_key);
  Coord x1, y1, x2, y2, y, d, aux;

  if (rP) {
    x1 = rP->topLeft.x;
    y1 = rP->topLeft.y;
    x2 = x1 + rP->extent.x - 1;
    y2 = y1 + rP->extent.y - 1;

    if (y1 > y2) {
      aux = y1;
      y1 = y2;
      y2 = aux;
    }

    if (cornerDiam > rP->extent.y/2) cornerDiam = rP->extent.y/2;

    d = cornerDiam;
    for (y = y1; y < y1+cornerDiam; y++) {
      draw_hline(x1+d, x2-d, y, module->pattern);
      d--;
    }
    for (; y < y2-cornerDiam; y++) {
      draw_hline(x1, x2, y, module->pattern);
    }
    d = 0;
    for (; y <= y2; y++) {
      draw_hline(x1+d, x2-d, y, module->pattern);
      d++;
    }
    if (module->drawWindow == module->activeWindow) dirty_region(x1, y1, x2, y2);
  }
}

/*
FrameType is a bitfield:
UInt16 cornerDiam  : 8;
UInt16 reserved_3  : 3;
UInt16 threeD      : 1;
UInt16 shadowWidth : 2;  // only meaninful for cornerDiam=0
UInt16 width       : 2;
*/
void WinPaintRectangleFrame(FrameType frame, const RectangleType *rP) {
  win_module_t *module = (win_module_t *)thread_get(win_key);
  UInt16 cornerDiam, width;

  if (module->drawWindow && rP) {
    cornerDiam = frame >> 8;
    width = frame & 0x03;
    if (frame & 0x10) {
      debug(DEBUG_ERROR, "Window", "WinPaintRectangleFrame 3D not supported");
    }
    if (width > 0) {
      if (cornerDiam > 2) {
        //debug(DEBUG_ERROR, "Window", "WinPaintRectangleFrame cornerDiam > 2 not supported");
        //cornerDiam = 2; // XXX looks better until proper round frame is implemented
      }
      WinPaintRectangleF(rP, width, cornerDiam, false, false);
    }
  }
}

void WinDrawGrayRectangleFrame(FrameType frame, const RectangleType *rP) {
  win_module_t *module = (win_module_t *)thread_get(win_key);
  UInt16 cornerDiam, width;

  if (module->drawWindow && rP) {
    cornerDiam = frame >> 8;
    width = frame & 0x03;
    if (frame & 0x10) {
      debug(DEBUG_ERROR, "Window", "WinDrawGrayRectangleFrame 3D not supported");
    }
    if (width > 0) {
      if (cornerDiam > 2) {
        //debug(DEBUG_ERROR, "Window", "WinDrawGrayRectangleFrame cornerDiam > 2 not supported");
        //cornerDiam = 2; // XXX looks better until proper round frame is implemented
      }
      WinPaintRectangleF(rP, width, cornerDiam, false, true);
    }
  }
}

void WinDrawRectangleFrame(FrameType frame, const RectangleType *rP) {
  WinDrawOperation prev = WinSetDrawMode(winPaint);
  PatternType oldp = WinGetPatternType();
  WinSetPatternType(blackPattern);
  WinPaintRectangleFrame(frame, rP);
  WinSetPatternType(oldp);
  WinSetDrawMode(prev);
}

void WinEraseRectangleFrame(FrameType frame, const RectangleType *rP) {
  RGBColorType back, fore;
  WinDrawOperation prev = WinSetDrawMode(winPaint);
  WinSetBackColorRGB(NULL, &back);
  WinSetForeColorRGB(&back, &fore);
  WinDrawRectangleFrame(frame, rP);
  WinSetForeColorRGB(&fore, NULL);
  WinSetDrawMode(prev);
}

void WinInvertRectangleFrame(FrameType frame, const RectangleType *rP) {
  win_module_t *module = (win_module_t *)thread_get(win_key);
  if (rP) {
    invertPrefix();
    RectangleType rect;
    MemMove(&rect, rP, sizeof(RectangleType));
    if (!isDouble) WinScaleRectangle(&rect);
    WinPaintRectangleFrame(frame, &rect);
    invertSuffix();
  }
}

void WinCopyBitmap(BitmapType *srcBmp, WinHandle dst, RectangleType *srcRect, Coord dstX, Coord dstY) {
  win_module_t *module = (win_module_t *)thread_get(win_key);
  BitmapType *dstBmp;
  UInt32 srcSize, dstSize, pixelSize, srcLineSize, dstLineSize, srcOffset, dstOffset, len;
  RectangleType dstRect, aux, clip, intersection, *dirtyRect;
  UInt16 depth;
  Boolean clipping;
  Coord srcWidth, srcHeight, dstWidth, dstHeight, dx, dy, y;
  UInt8 *srcBits, *dstBits;

  dstBmp = WinGetBitmap(dst);
  depth = BmpGetBitDepth(srcBmp);
  dirtyRect = NULL;

  switch (depth) {
    case  8:
    case 16:
    case 24:
    case 32:
      pixelSize = depth >> 3;
      break;
    default:
      // not supported
      debug(DEBUG_ERROR, "Window", "WinCopyBitmap invalid depth %u", depth);
      return;
  }

  if (BmpGetDensity(srcBmp) == BmpGetDensity(dstBmp) && depth == BmpGetBitDepth(dstBmp)) {
    BmpGetSizes(srcBmp, &srcSize, NULL);
    BmpGetSizes(dstBmp, &dstSize, NULL);
    srcBits = BmpGetBits(srcBmp);
    dstBits = BmpGetBits(dstBmp);
    BmpGetDimensions(srcBmp, &srcWidth, &srcHeight, NULL);
    BmpGetDimensions(dstBmp, &dstWidth, &dstHeight, NULL);
    clipping = (dst->clippingBounds.right > dst->clippingBounds.left) && (dst->clippingBounds.bottom > dst->clippingBounds.top);

    if (srcRect == NULL && dstX == 0 && dstY == 0 && srcSize == dstSize && !clipping) {
      // copy the whole window (best case)
      MemMove(dstBits, srcBits, dstSize);
      RctSetRectangle(&dstRect, 0, 0, dstWidth, dstHeight);
      dirtyRect = &dstRect;

    } else {
      if (srcRect == NULL) {
        // if srcRect is null, use the whole src window
        RctSetRectangle(&aux, 0, 0, srcWidth, srcHeight);
        srcRect = &aux;
      }

      // check limits on srcRect
      if (srcRect->topLeft.x + srcRect->extent.x > srcWidth) {
        srcRect->extent.x = srcWidth - srcRect->extent.x;
      }
      if (srcRect->topLeft.y + srcRect->extent.y > srcHeight) {
        srcRect->extent.y = srcHeight - srcRect->extent.y;
      }

      // set dstRect with same width and height from srcRect
      RctSetRectangle(&dstRect, dstX, dstY, srcRect->extent.x, srcRect->extent.y);

      // check limits on dstRect
      if (dstRect.topLeft.x + dstRect.extent.x > dstWidth) {
        dstRect.extent.x = dstWidth - dstRect.extent.x;
      }
      if (dstRect.topLeft.y + dstRect.extent.y > dstHeight) {
        dstRect.extent.y = dstHeight - dstRect.extent.y;
      }

      // srcRect and dstRect must have same width and height
      dstRect.extent.x = minValue(dstRect.extent.x, srcRect->extent.x);
      dstRect.extent.y = minValue(dstRect.extent.y, srcRect->extent.y);
      srcRect->extent.x = dstRect.extent.x;
      srcRect->extent.y = dstRect.extent.y;

      if (clipping) {
        // destination window has an active clipping region, compute intersection
        RctAbsToRect(&dst->clippingBounds, &clip);
        RctGetIntersection(&dstRect, &clip, &intersection);
        // adjust srcRect
        dx = intersection.topLeft.x - dstRect.topLeft.x;
        dy = intersection.topLeft.y - dstRect.topLeft.y;
        srcRect->topLeft.x += dx;
        srcRect->topLeft.y += dy;
        RctCopyRectangle(&intersection, &dstRect);
        srcRect->extent.x = dstRect.extent.x;
        srcRect->extent.y = dstRect.extent.y;
      }

      if (dstRect.extent.x > 0 && dstRect.extent.y > 0) {
        dirtyRect = &dstRect;

        if (srcWidth == dstWidth &&
            srcRect->topLeft.x == 0 && srcRect->extent.x == srcWidth &&
            dstRect.topLeft.x == 0 && dstRect.extent.x == dstWidth) {

          // copy a full width rectangle (2nd best case)
          srcLineSize = srcWidth * pixelSize;
          srcOffset = srcRect->topLeft.y * srcLineSize;
          dstOffset = dstRect.topLeft.y * srcLineSize;
          srcBits += srcOffset;
          dstBits += dstOffset;
          len = srcRect->extent.y * srcLineSize;
          MemMove(dstBits, srcBits, len);

        } else {
          // copy an arbitraty rectangle (generic case)
          srcLineSize = srcWidth * pixelSize;
          dstLineSize = dstWidth * pixelSize;
          srcOffset = srcRect->topLeft.y * srcLineSize + srcRect->topLeft.x * pixelSize;
          dstOffset = dstRect.topLeft.y * dstLineSize + dstRect.topLeft.x * pixelSize;
          srcBits += srcOffset;
          dstBits += dstOffset;
          len = srcRect->extent.x * pixelSize;
          for (y = 0; y < srcRect->extent.y; y++) {
            MemMove(dstBits, srcBits, len);
            srcBits += srcLineSize;
            dstBits += dstLineSize;
          }
        }
      }
    }
  } else {
    debug(DEBUG_ERROR, "Window", "WinCopyBitmap density or depth does not match");
  }

  if (dirtyRect && (dst == module->activeWindow || dst == module->displayWindow)) {
    pumpkin_screen_dirty(dst, dirtyRect->topLeft.x, dirtyRect->topLeft.y, dirtyRect->extent.x, dirtyRect->extent.y);
  }
}

void WinCopyWindow(WinHandle src, WinHandle dst, RectangleType *srcRect, Coord dstX, Coord dstY) {
  if (src && dst) {
    WinCopyBitmap(WinGetBitmap(src), dst, srcRect, dstX, dstY);
  }
}

void WinBlitBitmap(BitmapType *bitmapP, WinHandle wh, const RectangleType *rect, Coord x, Coord y, WinDrawOperation mode, Boolean text) {
  win_module_t *module = (win_module_t *)thread_get(win_key);
  BitmapType *windowBitmap, *displayBitmap, *best;
  RectangleType srcRect;
  UInt16 windowDensity, bitmapDensity, bitmapDepth, coordSys, displayDepth, windowDepth, tc, bc, tcd, bcd;
  UInt32 transparentValue;
  Coord i, j, srcX, srcY, id, jd, dstX, dstY, w, h, ax, ay;
  Coord srcX0, srcY0, dstX0, dstY0, srcIncX, dstIncX, srcIncY, dstIncY;
  Coord x1, y1, x2, y2, x3, y3, x4, y4;
  Boolean bitmapTransp, dbl, hlf;

  if (bitmapP && wh && rect) {
    windowBitmap = WinGetBitmap(wh);
    windowDensity = BmpGetDensity(windowBitmap);
    windowDepth = BmpGetBitDepth(windowBitmap);

    if ((best = BmpGetBestBitmapEx(bitmapP, windowDensity, windowDepth, !text)) != NULL) {
      bitmapDensity = BmpGetDensity(best);
      bitmapDepth = BmpGetBitDepth(best);
      bitmapTransp = BmpGetTransparentValue(best, &transparentValue);

      if (bitmapDensity == windowDensity && bitmapDepth == windowDepth && bitmapDepth >= 8 && !bitmapTransp && mode == winPaint && !text) {
        // it is possible to use fast copy
        RctCopyRectangle(rect, &srcRect);
        coordSys = WinGetCoordinateSystem();
        if (bitmapDensity == kDensityDouble && coordSys == kCoordinatesStandard) {
          WinSetCoordinateSystem(kCoordinatesDouble);
          x = WinScaleCoord(x, false);
          y = WinScaleCoord(y, false);
          WinScaleRectangle(&srcRect);
        } else if (bitmapDensity == kDensityLow && coordSys == kCoordinatesDouble) {
          WinSetCoordinateSystem(kCoordinatesStandard);
          x = WinUnscaleCoord(x, false);
          y = WinUnscaleCoord(y, false);
          WinUnscaleRectangle(&srcRect);
        }
        WinCopyBitmap(best, wh, &srcRect, x, y);
        WinSetCoordinateSystem(coordSys);
        return;
      }

      dbl = bitmapDensity == kDensityLow && windowDensity == kDensityDouble;
      hlf = bitmapDensity == kDensityDouble && windowDensity == kDensityLow;

      ax = wh->windowBounds.topLeft.x;
      ay = wh->windowBounds.topLeft.y;
      srcX = rect->topLeft.x;
      srcY = rect->topLeft.y;
      w = rect->extent.x;
      h = rect->extent.y;
      dstX = x;
      dstY = y;
      coordSys = WinGetCoordinateSystem();

      if (bitmapDensity == kDensityLow && coordSys == kCoordinatesDouble) {
        srcX >>= 1;
        srcY >>= 1;
        w >>= 1;
        h >>= 1;
      } else if (bitmapDensity == kDensityDouble && coordSys == kCoordinatesStandard) {
        srcX <<= 1;
        srcY <<= 1;
        w <<= 1;
        h <<= 1;
      }

      srcY0 = (srcY < dstY) ? h-1 : 0;
      srcX0 = (srcX < dstX) ? w-1 : 0;

      if (hlf) {
        srcIncX = (srcX < dstX) ? -2 : 2;
        srcIncY = (srcY < dstY) ? -2 : 2;
      } else {
        srcIncX = (srcX < dstX) ? -1 : 1;
        srcIncY = (srcY < dstY) ? -1 : 1;
      }

      if (windowDensity == kDensityDouble) {
        ax <<= 1;
        ay <<= 1;
      }

      if (windowDensity == kDensityLow && coordSys == kCoordinatesDouble) {
        dstX >>= 1;
        dstY >>= 1;
      } else if (windowDensity == kDensityDouble && coordSys == kCoordinatesStandard) {
        dstX <<= 1;
        dstY <<= 1;
      }

      if (dbl) {
        dstY0 = srcY0*2;
        dstX0 = srcX0*2;
        dstIncX = srcIncX*2;
        dstIncY = srcIncY*2;
      } else if (hlf) {
        dstY0 = srcY0/2;
        dstX0 = srcX0/2;
        dstIncX = srcIncX/2;
        dstIncY = srcIncY/2;
      } else {
        dstY0 = srcY0;
        dstX0 = srcX0;
        dstIncX = srcIncX;
        dstIncY = srcIncY;
      }

      displayBitmap = WinGetBitmap(module->displayWindow);
      debug(DEBUG_TRACE, "Window", "WinBlitBitmap (%d,%d,%d,%d) density %d to (%d,%d) density %d, coordsys %d, dbl %d, hlf %d",
        srcX, srcY, w, h, bitmapDensity, dstX, dstY, windowDensity, coordSys, dbl, hlf);

      x1 = wh->clippingBounds.left;
      x2 = wh->clippingBounds.right;
      y1 = wh->clippingBounds.top;
      y2 = wh->clippingBounds.bottom;
      if (!(x1 == 0 && x2 == 0) && windowDensity == kDensityLow) {
        x1 = x1 >> 1;
        y1 = y1 >> 1;
        x2 = x2 >> 1;
        y2 = y2 >> 1;
      }
//debug(1, "XXX", "window %p clipping %d %d %d %d", wh, x1, x2, y1, y2);

      x3 = module->displayWindow->clippingBounds.left;
      x4 = module->displayWindow->clippingBounds.right;
      y3 = module->displayWindow->clippingBounds.top;
      y4 = module->displayWindow->clippingBounds.bottom;
//debug(1, "XXX", "display clipping %d %d %d %d", x3, x4, y3, y4);

      tc = windowDepth == 16 ? module->textColor565 : module->textColor;
      bc = windowDepth == 16 ? module->backColor565 : module->backColor;

      displayDepth = BmpGetBitDepth(displayBitmap);
      tcd = displayDepth == 16 ? module->textColor565 : module->textColor;
      bcd = displayDepth == 16 ? module->backColor565 : module->backColor;

      for (i = srcY0, id = dstY0; i >= 0 && i < h; i += srcIncY, id += dstIncY) {
        for (j = srcX0, jd = dstX0; j >= 0 && j < w; j += srcIncX, jd += dstIncX) {
          if (CLIP_OK(x1, x2, y1, y2, dstX + jd, dstY + id)) {
            BmpCopyBit(best, srcX + j, srcY + i, windowBitmap, dstX + jd, dstY + id, mode, dbl, text, tc, bc);
          }

          if (wh == module->activeWindow && wh != module->displayWindow) {
            if (CLIP_OK(x3, x4, y3, y4, ax + dstX + jd, ay + dstY + id)) {
              BmpCopyBit(best, srcX + j, srcY + i, displayBitmap, ax + dstX + jd, ay + dstY + id, mode, dbl, text, tcd, bcd);
            }
          }
        }
      }

      if (wh == module->activeWindow || wh == module->displayWindow) {
        if (dbl) {
          w <<= 1;
          h <<= 1;
        }
//debug(1, "XXX", "dirty %p %d,%d,%d,%d", wh, dstX, dstY, w, h);
        pumpkin_screen_dirty(wh, dstX, dstY, w, h);
      }
    }
  }
}

void WinCopyRectangle(WinHandle srcWin, WinHandle dstWin, const RectangleType *srcRect, Coord dstX, Coord dstY, WinDrawOperation mode) {
  win_module_t *module = (win_module_t *)thread_get(win_key);

  if (srcWin == NULL) {
    debug(DEBUG_ERROR, "Window", "WinCopyRectangle srcWin is NULL");
    return;
  }

  if (dstWin == NULL) {
    dstWin = module->drawWindow;
    if (dstWin == NULL) {
      debug(DEBUG_ERROR, "Window", "WinCopyRectangle drawWindow is NULL");
      return;
    }
  }

  debug(DEBUG_TRACE, "Window", "WinCopyRectangle srcWin=%p srcBmp=%p %d,%d,%d,%d -> dstWin=%p dstBmp=%p %d,%d (mode %d)", srcWin, srcWin->bitmapP, srcRect->topLeft.x, srcRect->topLeft.y, srcRect->extent.x, srcRect->extent.y, dstWin, dstWin->bitmapP, dstX, dstY, mode);
  WinBlitBitmap(srcWin->bitmapP, dstWin, srcRect, dstX, dstY, mode, false);
}

void WinPaintBitmap(BitmapPtr bitmapP, Coord x, Coord y) {
  win_module_t *module = (win_module_t *)thread_get(win_key);
  BitmapType *windowBitmap, *best;
  RectangleType rect;
  MemHandle handle;
  DmResType resType;
  DmResID resID;
  UInt32 size;
  uint32_t magic;
  uint8_t *bmp, *base, *end;
  char stype[8];
  Coord w, h;
  Boolean destroy = false;

  if (bitmapP && module->drawWindow) {
    // This a hack to make apps like Kyle's Quest work.
    // KQ calls WinDrawBitmap passing a memory block containing a bitmap,
    // but this bitmap was MemMove'd into the block instead of being decoded from a resource.
    // Therefore it is not a valid PumpkinOS BitmapType, and must be created/decoded from the memory block.

    bmp = (uint8_t *)bitmapP;
    base = (uint8_t *)pumpkin_heap_base();
    end = base + pumpkin_heap_size();
    if (bmp < base || bmp >= end) {
      debug(DEBUG_ERROR, "Window", "WinPaintBitmap invalid address %p", bitmapP);
      return;
    }

    get4b(&magic, (uint8_t *)bitmapP, BITMAP_SPACE-4);

    if (magic != BITMAP_MAGIC) {
      if ((handle = MemPtrRecoverHandle(bitmapP)) == NULL) {
        debug(DEBUG_INFO, "Window", "WinPaintBitmap local buffer 0x%08X %p", bmp - base, bitmapP);
        bitmapP = pumpkin_create_bitmap(NULL, (uint8_t *)bitmapP, BITMAP_SPACE, bitmapRsc, 0, NULL);
        destroy = true;
      } else {
        size = MemHandleSize(handle);
        if (DmResourceType(handle, &resType, &resID) != errNone) {
          debug(DEBUG_INFO, "Window", "WinPaintBitmap handle 0x%08X is not a resource (%d bytes)", (uint8_t *)handle - base, size);
          bitmapP = pumpkin_create_bitmap(NULL, (uint8_t *)bitmapP, size, bitmapRsc, 0, NULL);
          destroy = true;
        } else if (resType != bitmapRsc && resType != iconType) {
          pumpkin_id2s(resType, stype);
          debug(DEBUG_INFO, "Window", "WinPaintBitmap handle 0x%08X of type '%s' is not a bitmap resource (%d bytes)", (uint8_t *)handle - base, stype, size);
          bitmapP = pumpkin_create_bitmap(NULL, (uint8_t *)bitmapP, size, bitmapRsc, 0, NULL);
          destroy = true;
        }
      }
    }

    debug(DEBUG_TRACE, "Window", "WinPaintBitmap %p %d,%d at %d,%d", bitmapP, bitmapP->width, bitmapP->height, x, y);
    windowBitmap = WinGetBitmap(module->drawWindow);

    if ((best = BmpGetBestBitmap(bitmapP, BmpGetDensity(windowBitmap), BmpGetBitDepth(windowBitmap))) != NULL) {
      BmpGetDimensions(best, &w, &h, NULL);
      debug(DEBUG_TRACE, "Window", "WinPaintBitmap best %p %d,%d at %d,%d", best, w, h, x, y);
      RctSetRectangle(&rect, 0, 0, w, h);
      WinScaleRectangle(&rect);
      WinBlitBitmap(best, module->drawWindow, &rect, x, y, module->transferMode, false);
    }

    if (destroy) {
      pumpkin_destroy_bitmap(bitmapP);
    }
  }
}

void WinDrawBitmap(BitmapType *bitmapP, Coord x, Coord y) {
  WinDrawOperation prev = WinSetDrawMode(winPaint);
  WinPaintBitmap(bitmapP, x, y);
  WinSetDrawMode(prev);
}

static void WinDrawCharsC(uint8_t *chars, Int16 len, Coord x, Coord y, int max) {
  win_module_t *module = (win_module_t *)thread_get(win_key);
  FontType *f;
  FontTypeV2 *f2;
  FontID font;
  UInt16 density;
  Coord x0, y0, w, h;
  UInt16 prev, wch;
  Boolean v10;
  RectangleType rect;
  uint16_t ch;
  uint32_t i, col, index, mult;

  if (module->drawWindow && chars && len > 0) {
    x0 = x;
    y0 = y;
    f = FntGetFontPtr();
    font = FntGetFont();
    // As of PalmOS 3.1 the Euro sign is 0x80, and numeric space was moved from 0x80 to 0x19
    v10 = (font == stdFont || font == boldFont) && pumpkin_is_v10();

    if (f == NULL) {
      debug(DEBUG_ERROR, "Window", "WinDrawCharsC: FntGetFontPtr returned NULL !");
      return;
    }

    if (f->v == 1) {
      density = BmpGetDensity(WinGetBitmap(module->drawWindow));

      switch (density) {
        case kDensityLow:
          if (module->coordSys == kCoordinatesDouble) {
            x >>= 1;
            y >>= 1;
          }
          break;
        case kDensityDouble:
          if (module->coordSys == kCoordinatesStandard) {
            x <<= 1;
            y <<= 1;
          }
          break;
      }
//debug(1, "XXX", "WinDrawCharsC(\"%.*s\", %d, %d) %p v1 density %d", len, chars, x, y, WinGetDrawWindow(), density);

      for (i = 0; i < len;) {
        i += pumpkin_next_char(chars, i, &wch);
        ch = pumpkin_map_char(wch, &f);
        if (v10 && ch == 0x80) ch = 0x19; // numeric space
        if (ch >= f->firstChar && ch <= f->lastChar) {
          col = f->column[ch - f->firstChar];
          RctSetRectangle(&rect, col, 0, f->width[ch - f->firstChar], f->fRectHeight);
          WinBlitBitmap(f->bmp, module->drawWindow, &rect, x, y, module->transferMode, true);
          x += f->width[ch - f->firstChar];
        } else {
          debug(DEBUG_ERROR, "Window", "missing symbol 0x%04X", wch);
          x += MISSING_SYMBOL_WIDTH;
        }
      }
    } else {
      f2 = (FontTypeV2 *)f;
      if (f2->densityCount >= 2 && f2->densities[0].density == kDensityLow && f2->densities[1].density == kDensityDouble) {
        density = BmpGetDensity(WinGetBitmap(module->drawWindow));
        prev = WinGetCoordinateSystem();

        switch (density) {
          case kDensityLow:
            index = 0; // low density font
            mult = 1;
            if (module->coordSys == kCoordinatesDouble) {
              x >>= 1;
              y >>= 1;
            }
            WinSetCoordinateSystem(kCoordinatesStandard);
            break;
          case kDensityDouble:
            index = 1; // double density font
            mult = 2;
            if (module->coordSys == kCoordinatesStandard) {
              x <<= 1;
              y <<= 1;
            }
            WinSetCoordinateSystem(kCoordinatesDouble);
            break;
        }
//debug(1, "XXX", "WinDrawCharsC(\"%.*s\", %d, %d) %p v2 density %d mode %d", len, chars, x, y, WinGetDrawWindow(), density, module->transferMode);
//debug_bytes(1, "XXX", chars, len);

        for (i = 0; i < len;) {
          i += pumpkin_next_char(chars, i, &wch);
          ch = pumpkin_map_char(wch, &f);
          f2 = (FontTypeV2 *)f;
          if (v10 && ch == 0x80) ch = 0x19; // numeric space
          if (ch >= f2->firstChar && ch <= f2->lastChar) {
            col = f2->column[ch - f2->firstChar]*mult;
            RctSetRectangle(&rect, col, 0, f2->width[ch - f2->firstChar]*mult, f2->fRectHeight*mult);
            WinBlitBitmap(f2->bmp[index], module->drawWindow, &rect, x, y, module->transferMode, true);
            x += f2->width[ch - f2->firstChar]*mult;
          } else {
            debug(DEBUG_ERROR, "Window", "missing symbol 0x%04X", wch);
            x += MISSING_SYMBOL_WIDTH;
          }
        }
        WinSetCoordinateSystem(prev);
      }
    }

    if (module->drawWindow == module->activeWindow) {
      w = FntCharsWidth((char *)chars, len);
      h = FntCharHeight();
      if (w > 0) {
        dirty_region(x0, y0, x0+w-1, y0+h-1);
      }
    }
  }
}

void WinDrawChar(WChar theChar, Coord x, Coord y) {
  char c[2];

  if (theChar > 0xFF) {
    c[0] = theChar >> 8;
    c[1] = theChar & 0xFF;
    WinDrawChars(c, 2, x, y);
  } else {
    c[0] = theChar & 0xFF;
    WinDrawChars(c, 1, x, y);
  }
}

void WinPaintChar(WChar theChar, Coord x, Coord y) {
  char c[2];

  if (theChar > 0xFF) {
    c[0] = theChar >> 8;
    c[1] = theChar & 0xFF;
    WinPaintChars(c, 2, x, y);
  } else {
    c[0] = theChar & 0xFF;
    WinPaintChars(c, 1, x, y);
  }
}

void WinDrawChars(const Char *chars, Int16 len, Coord x, Coord y) {
  debug(DEBUG_TRACE, "Window", "WinDrawChars(\"%.*s\", %d, %d)", len, chars, x, y);
  WinDrawOperation prev = WinSetDrawMode(winPaint);
  WinDrawCharsC((uint8_t *)chars, len, x, y, 0);
  WinSetDrawMode(prev);
}

void WinPaintChars(const Char *chars, Int16 len, Coord x, Coord y) {
  debug(DEBUG_TRACE, "Window", "WinPaintChars(\"%.*s\", %d, %d)", len, chars, x, y);
  WinDrawCharsC((uint8_t *)chars, len, x, y, 0);
}

// This routine draws the on bits and any underline in the background
// color and the off bits in the text color. (Black and white uses copy
// NOT mode.) This is the standard function for drawing inverted text.

void WinDrawInvertedChars(const Char *chars, Int16 len, Coord x, Coord y) {
  win_module_t *module = (win_module_t *)thread_get(win_key);
  debug(DEBUG_TRACE, "Window", "WinDrawInvertedChars(\"%.*s\", %d, %d)", len, chars, x, y);
  WinDrawOperation prev = WinSetDrawMode(winPaint);
  IndexedColorType oldt = module->textColor;
  IndexedColorType oldb = module->backColor;
  WinSetTextColor(oldb);
  WinSetBackColor(oldt);
  WinDrawCharsC((uint8_t *)chars, len, x, y, 0);
  WinSetTextColor(oldt);
  WinSetBackColor(oldb);
  WinSetDrawMode(prev);
}

void WinDrawTruncChars(const Char *chars, Int16 len, Coord x, Coord y, Coord maxWidth) {
  debug(DEBUG_TRACE, "Window", "WinDrawTruncChars(\"%.*s\", %d, %d)", len, chars, x, y);
  WinDrawOperation prev = WinSetDrawMode(winPaint);
  WinDrawCharsC((uint8_t *)chars, len, x, y, maxWidth);
  WinSetDrawMode(prev);
}

void WinGlueDrawTruncChars(const Char* pChars, UInt16 length, Int16 x, Int16 y, Int16 maxWidth) {
  WinDrawTruncChars(pChars, length, x, y, maxWidth);
}

// The winMask transfer mode is used to erase the characters. See
// WinDrawOperation for more information. This has the effect of
// erasing only the on bits for the characters rather than the entire text
// rectangle. This function only works if the foreground color is black
// and the background color is white.

void WinEraseChars(const Char *chars, Int16 len, Coord x, Coord y) {
  debug(DEBUG_TRACE, "Window", "WinEraseChars(\"%.*s\", %d, %d)", len, chars, x, y);
  WinDrawOperation prev = WinSetDrawMode(winMask);
  WinDrawCharsC((uint8_t *)chars, len, x, y, 0);
  WinSetDrawMode(prev);
}

// This function applies the winInvert operation of
// WinDrawOperation to the characters in the draw window.
// To perform color inverting, use WinSetTextColor and
// WinSetBackColor to choose the desired colors, and call WinPaintChar.

void WinInvertChars(const Char *chars, Int16 len, Coord x, Coord y) {
  debug(DEBUG_TRACE, "Window", "WinInvertChars(\"%.*s\", %d, %d)", len, chars, x, y);
  WinDrawOperation prev = WinSetDrawMode(winInvert);
  WinDrawCharsC((uint8_t *)chars, len, x, y, 0);
  WinSetDrawMode(prev);
}

UnderlineModeType WinSetUnderlineMode(UnderlineModeType mode) {
  win_module_t *module = (win_module_t *)thread_get(win_key);
  UnderlineModeType prev = module->underlineMode;
  module->underlineMode = mode;
  return prev;
}

WinDrawOperation WinSetDrawMode(WinDrawOperation newMode) {
  win_module_t *module = (win_module_t *)thread_get(win_key);
  debug(DEBUG_TRACE, "Window", "WinSetDrawMode %d", newMode);
  WinDrawOperation prev = module->transferMode;
  module->transferMode = newMode;
  return prev;
}

IndexedColorType WinGetForeColor(void) {
  win_module_t *module = (win_module_t *)thread_get(win_key);
  return module->foreColor;
}

IndexedColorType WinSetForeColor(IndexedColorType foreColor) {
  win_module_t *module = (win_module_t *)thread_get(win_key);
  ColorTableType *colorTable;

  colorTable = module->drawWindow ? BmpGetColortable(module->drawWindow->bitmapP) : NULL;
  if (colorTable == NULL) colorTable = module->colorTable;

  IndexedColorType prev = module->foreColor;

  if (foreColor >= 0 && foreColor < colorTable->numEntries) {
    module->foreColor = foreColor;
    module->foreColorRGB = colorTable->entry[foreColor];
    module->foreColor565 = rgb565(module->foreColorRGB.r, module->foreColorRGB.g, module->foreColorRGB.b);
    debug(DEBUG_TRACE, "Window", "WinSetForeColor index=%d r=0x%02X g=0x%02X b=0x%02X rgb565=0x%04X", foreColor, module->foreColorRGB.r, module->foreColorRGB.g, module->foreColorRGB.b, module->foreColor565);
  }

  return prev;
}

IndexedColorType WinGetBackColor(void) {
  win_module_t *module = (win_module_t *)thread_get(win_key);
  return module->backColor;
}

IndexedColorType WinSetBackColor(IndexedColorType backColor) {
  win_module_t *module = (win_module_t *)thread_get(win_key);
  ColorTableType *colorTable;

  colorTable = module->drawWindow ? BmpGetColortable(module->drawWindow->bitmapP) : NULL;
  if (colorTable == NULL) colorTable = module->colorTable;

  IndexedColorType prev = module->backColor;

  if (backColor >= 0 && backColor < colorTable->numEntries) {
    module->backColor = backColor;
    module->backColorRGB = colorTable->entry[backColor];
    module->backColor565 = rgb565(module->backColorRGB.r, module->backColorRGB.g, module->backColorRGB.b);
    debug(DEBUG_TRACE, "Window", "WinSetBackColor index=%d r=0x%02X g=0x%02X b=0x%02X rgb565=0x%04X", backColor, module->backColorRGB.r, module->backColorRGB.g, module->backColorRGB.b, module->backColor565);
  }

  return prev;
}

IndexedColorType WinSetTextColor(IndexedColorType textColor) {
  win_module_t *module = (win_module_t *)thread_get(win_key);
  ColorTableType *colorTable;

  colorTable = module->drawWindow ? BmpGetColortable(module->drawWindow->bitmapP) : NULL;
  if (colorTable == NULL) colorTable = module->colorTable;

  IndexedColorType prev = module->textColor;

  if (textColor >= 0 && textColor < colorTable->numEntries) {
    module->textColor = textColor;
    module->textColorRGB = colorTable->entry[textColor];
    module->textColor565 = rgb565(module->textColorRGB.r, module->textColorRGB.g, module->textColorRGB.b);
  }

  return prev;
}

void WinSetForeColorRGB(const RGBColorType* newRgbP, RGBColorType* prevRgbP) {
  win_module_t *module = (win_module_t *)thread_get(win_key);

  if (prevRgbP) {
    prevRgbP->index = module->foreColorRGB.index;
    prevRgbP->r = module->foreColorRGB.r;
    prevRgbP->g = module->foreColorRGB.g;
    prevRgbP->b = module->foreColorRGB.b;
  }

  if (newRgbP) {
    module->foreColorRGB.index = newRgbP->index;
    module->foreColorRGB.r = newRgbP->r;
    module->foreColorRGB.g = newRgbP->g;
    module->foreColorRGB.b = newRgbP->b;
    module->foreColor = WinRGBToIndex(&module->foreColorRGB);
    module->foreColor565 = rgb565(module->foreColorRGB.r, module->foreColorRGB.g, module->foreColorRGB.b);
  }
}

void WinSetBackColorRGB(const RGBColorType* newRgbP, RGBColorType* prevRgbP) {
  win_module_t *module = (win_module_t *)thread_get(win_key);

  if (prevRgbP) {
    prevRgbP->index = module->backColorRGB.index;
    prevRgbP->r = module->backColorRGB.r;
    prevRgbP->g = module->backColorRGB.g;
    prevRgbP->b = module->backColorRGB.b;
  }

  if (newRgbP) {
    module->backColorRGB.index = newRgbP->index;
    module->backColorRGB.r = newRgbP->r;
    module->backColorRGB.g = newRgbP->g;
    module->backColorRGB.b = newRgbP->b;
    module->backColor = WinRGBToIndex(&module->backColorRGB);
    module->backColor565 = rgb565(module->backColorRGB.r, module->backColorRGB.g, module->backColorRGB.b);
  }
}

void WinSetTextColorRGB(const RGBColorType* newRgbP, RGBColorType* prevRgbP) {
  win_module_t *module = (win_module_t *)thread_get(win_key);

  if (prevRgbP) {
    prevRgbP->index = module->textColorRGB.index;
    prevRgbP->r = module->textColorRGB.r;
    prevRgbP->g = module->textColorRGB.g;
    prevRgbP->b = module->textColorRGB.b;
  }

  if (newRgbP) {
    module->textColorRGB.index = newRgbP->index;
    module->textColorRGB.r = newRgbP->r;
    module->textColorRGB.g = newRgbP->g;
    module->textColorRGB.b = newRgbP->b;
    module->textColor = WinRGBToIndex(&module->textColorRGB);
    module->textColor565 = rgb565(module->textColorRGB.r, module->textColorRGB.g, module->textColorRGB.b);
  }
}

void WinGetPattern(CustomPatternType *patternP) {
  win_module_t *module = (win_module_t *)thread_get(win_key);
  if (patternP) {
    MemMove(patternP, module->patternData, sizeof(CustomPatternType));
  }
}

void WinSetPattern(const CustomPatternType *patternP) {
  win_module_t *module = (win_module_t *)thread_get(win_key);

  if (patternP) {
    MemMove(module->patternData, patternP, sizeof(CustomPatternType));
    module->pattern = customPattern;
    debug(DEBUG_TRACE, "Window", "WinSetPattern [0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X]",
      module->patternData[0], module->patternData[1], module->patternData[2], module->patternData[3],
      module->patternData[4], module->patternData[5], module->patternData[6], module->patternData[7]);
  }
}

PatternType WinGetPatternType(void) {
  win_module_t *module = (win_module_t *)thread_get(win_key);
  return module->pattern;
}

void WinSetPatternType(PatternType newPattern) {
  win_module_t *module = (win_module_t *)thread_get(win_key);

  switch (newPattern) {
    case blackPattern:
    case whitePattern:
    case grayPattern:
    case customPattern:
      module->pattern = newPattern;
      break;
    case lightGrayPattern:
      debug(DEBUG_ERROR, "Window", "WinSetPatternType lightGrayPattern not supported, using grayPattern");
      module->pattern = grayPattern;
      break;
    case darkGrayPattern:
      debug(DEBUG_ERROR, "Window", "WinSetPatternType darkGrayPattern not supported, using grayPattern");
      module->pattern = grayPattern;
      break;
  }
}

// WinRGBToIndex uses the draw window’s color table to return the
// appropriate color table index. If the draw window does not have a
// color table, the default color table of the current screen is used.

IndexedColorType WinRGBToIndex(const RGBColorType *rgbP) {
  win_module_t *module = (win_module_t *)thread_get(win_key);
  ColorTableType *colorTable;
  uint32_t i, d, dmin, imin;
  int32_t dr, dg, db;

  colorTable = module->drawWindow ? BmpGetColortable(module->drawWindow->bitmapP) : NULL;
  if (colorTable == NULL) colorTable = module->colorTable;

  dmin = 0xffffffff;
  imin = 0;

  for (i = 0; i < colorTable->numEntries; i++) {
    if (rgbP->r == colorTable->entry[i].r && rgbP->g == colorTable->entry[i].g && rgbP->b == colorTable->entry[i].b) {
      return i;
    }
    dr = (int32_t)rgbP->r - (int32_t)colorTable->entry[i].r;
    dr = dr * dr;
    dg = (int32_t)rgbP->g - (int32_t)colorTable->entry[i].g;
    dg = dg * dg;
    db = (int32_t)rgbP->b - (int32_t)colorTable->entry[i].b;
    db = db * db;
    d = dr + dg + db;
    if (d < dmin) {
      dmin = d;
      imin = i;
    }
  }
  debug(DEBUG_TRACE, "Window", "WinRGBToIndex %d,%d,%d -> %d,%d,%d", rgbP->r, rgbP->g, rgbP->b, colorTable->entry[imin].r, colorTable->entry[imin].g, colorTable->entry[imin].b);

  return imin;
}

void WinIndexToRGB(IndexedColorType i, RGBColorType *rgbP) {
  win_module_t *module = (win_module_t *)thread_get(win_key);
  ColorTableType *colorTable;

  if (rgbP) {
    colorTable = module->drawWindow ? BmpGetColortable(module->drawWindow->bitmapP) : NULL;
    if (colorTable == NULL) colorTable = module->colorTable;

    rgbP->index = i;
    rgbP->r = colorTable->entry[i].r;
    rgbP->g = colorTable->entry[i].g;
    rgbP->b = colorTable->entry[i].b;
  }
}

// "obsolete" color call, supported for backwards compatibility
void WinSetColors(const RGBColorType *newForeColorP, RGBColorType *oldForeColorP, const RGBColorType *newBackColorP, RGBColorType *oldBackColorP) {
  WinSetForeColorRGB(newForeColorP, oldForeColorP);
  WinSetTextColorRGB(newForeColorP, NULL); // apparently, text color is also set to foreColor (Vexed uses this call before WinDrawChars)
  WinSetBackColorRGB(newBackColorP, oldBackColorP);
}

void WinScreenInit(void) {
  // system use only
  debug(DEBUG_ERROR, "Window", "WinScreenInit not implemented");
}

UInt8 *WinScreenLock(WinLockInitType initMode) {
  // XXX does nothing yet

  switch (initMode) {
    case winLockCopy:
      break;
    case winLockErase:
      break;
    case winLockDontCare:
      break;
  }

  return NULL;
}

void WinScreenUnlock(void) {
  // XXX does nothing yet
}

UInt16 WinSetCoordinateSystem(UInt16 coordSys) {
  win_module_t *module = (win_module_t *)thread_get(win_key);
  UInt16 prev;

  debug(DEBUG_TRACE, "Window", "WinSetCoordinateSystem %d", coordSys);
  prev = module->coordSys;

  switch (coordSys) {
     case kCoordinatesNative:
       switch (module->density) {
         case kDensityLow:       module->coordSys = kCoordinatesStandard;  break;
         case kDensityDouble:    module->coordSys = kCoordinatesDouble;    break;
       }
       break;
     case kCoordinatesStandard:
       module->coordSys = coordSys;
       break;
     case kCoordinatesDouble:
       if (module->density == kDensityDouble) {
         module->coordSys = coordSys;
       }
       break;
     default:
       debug(DEBUG_ERROR, "Window", "WinSetCoordinateSystem %d unsupported", coordSys);
       break;
  }
//debug(1, "XXX", "WinSetCoordinateSystem new=%d (%d), prev=%d", module->coordSys, coordSys, prev);

  return prev;
}

UInt16 WinGetCoordinateSystem(void) {
  win_module_t *module = (win_module_t *)thread_get(win_key);
  return module->coordSys;
}

Coord WinScaleCoord(Coord coord, Boolean ceiling) {
  win_module_t *module = (win_module_t *)thread_get(win_key);

  if (module->coordSys == kCoordinatesDouble) {
    coord *= 2;
    if (ceiling) coord++;
  }

  return coord;
}

Coord WinUnscaleCoord(Coord coord, Boolean ceiling) {
  win_module_t *module = (win_module_t *)thread_get(win_key);

  if (module->coordSys == kCoordinatesDouble) {
    coord /= 2;
  }

  return coord;
}

void WinScalePoint(PointType *pointP, Boolean ceiling) {
  if (pointP) {
    pointP->x = WinScaleCoord(pointP->x, ceiling);
    pointP->y = WinScaleCoord(pointP->y, ceiling);
  }
}

void WinUnscalePoint(PointType *pointP, Boolean ceiling) {
  if (pointP) {
    pointP->x = WinUnscaleCoord(pointP->x, ceiling);
    pointP->y = WinUnscaleCoord(pointP->y, ceiling);
  }
}

// (2,3) (7,8) -> x:2-8, y:3-10 -> x:4-17, y:6-21 -> (4,6) (14,16)
void WinScaleRectangle(RectangleType *rectP) {
  if (rectP) {
    WinScalePoint(&rectP->topLeft, false);
    WinScalePoint(&rectP->extent, false);
  }
}

void WinUnscaleRectangle(RectangleType *rectP) {
  Coord x2, y2;

  if (rectP) {
    x2 = rectP->topLeft.x + rectP->extent.x - 1;
    y2 = rectP->topLeft.y + rectP->extent.y - 1;
    WinUnscalePoint(&rectP->topLeft, false);
    x2 = WinUnscaleCoord(x2, true);
    y2 = WinUnscaleCoord(y2, true);
    rectP->extent.x = x2 - rectP->topLeft.x + 1;
    rectP->extent.y = y2 - rectP->topLeft.y + 1;
  }
}

void WinScaleAbsRect(AbsRectType *arP, Boolean ceiling) {
  if (arP) {
    arP->left = WinScaleCoord(arP->left, ceiling);
    arP->right = WinScaleCoord(arP->right, ceiling);
    arP->top = WinScaleCoord(arP->top, ceiling);
    arP->bottom = WinScaleCoord(arP->bottom, ceiling);
  }
}

void WinUnscaleAbsRect(AbsRectType *arP, Boolean ceiling) {
  if (arP) {
    arP->left = WinUnscaleCoord(arP->left, ceiling);
    arP->right = WinUnscaleCoord(arP->right, ceiling);
    arP->top = WinUnscaleCoord(arP->top, ceiling);
    arP->bottom = WinUnscaleCoord(arP->bottom, ceiling);
  }
}

// unlike WinScreenMode, this function always returns the true screen dimensions
Err WinScreenGetAttribute(WinScreenAttrType selector, UInt32 *attrP) {
  win_module_t *module = (win_module_t *)thread_get(win_key);
  Err err = sysErrParamErr;

  switch (selector) {
    case winScreenWidth:
      if (attrP) *attrP = module->width;
      err = errNone;
      break;
    case winScreenHeight:
      if (attrP) *attrP = module->height;
      err = errNone;
      break;
    case winScreenRowBytes:
      switch (module->depth) {
        case 1:
          if (attrP) *attrP = (module->width + 7) / 8;
          err = errNone;
          break;
        case 2:
          if (attrP) *attrP = (module->width + 3) / 4;
          err = errNone;
          break;
        case 4:
          err = errNone;
          if (attrP) *attrP = (module->width + 1) / 2;
          break;
        case 8:
          if (attrP) *attrP = module->width;
          err = errNone;
          break;
        case 16:
          if (attrP) *attrP = module->width * 2;
          err = errNone;
          break;
      }
      break;
    case winScreenDepth:
      if (attrP) *attrP = module->depth;
      err = errNone;
      break;
    case winScreenAllDepths:
      // XXX 1, 2 and 4 ?
      if (attrP) {
        *attrP = 1 << (8 - 1);
        *attrP |= 1 << (16 - 1);
      }
      err = errNone;
      break;
    case winScreenDensity:
      if (attrP) *attrP = module->density;
      err = errNone;
      break;
    case winScreenPixelFormat:
      switch (module->depth) {
        case 1:
        case 2:
        case 4:
        case 8:
          if (attrP) *attrP = pixelFormatIndexed;
          err = errNone;
          break;
        case 16:
          if (attrP) *attrP = pixelFormat565;
          err = errNone;
          break;
      }
      break;
    case winScreenResolutionX:
      // The number of pixels per inch along the screen’s x axis
      // XXX density or 320 ?
      if (attrP) *attrP = module->density;
      err = errNone;
      break;
    case winScreenResolutionY:
      // The number of pixels per inch along the screen’s y axis
      // XXX density or 320 ?
      if (attrP) *attrP = module->density;
      err = errNone;
      break;
    default:
      debug(DEBUG_ERROR, "Window", "WinScreenGetAttribute %d not implemented", selector);
      if (attrP) *attrP = 0;
      break;
  }

  return err;
}

void WinPaintTiledBitmap(BitmapType* bitmapP, RectangleType* rectP) {
  debug(DEBUG_ERROR, "Window", "WinPaintTiledBitmap not implemented");
}

Err WinGetSupportedDensity(UInt16 *densityP) {
  win_module_t *module = (win_module_t *)thread_get(win_key);
  Err err = sysErrParamErr;

  if (densityP) {
    switch (*densityP) {
      case 0:
        *densityP = kDensityLow;
        err = errNone;
        break;
      case kDensityLow:
        if (module->density > kDensityLow) {
          *densityP = kDensityDouble;
          err = errNone;
        }
        break;
    }
  }

  return err;
}

void WinPaintRoundedRectangleFrame(const RectangleType *rP, Coord width, Coord cornerRadius, Coord shadowWidth) {
  WinPaintRectangleFrame((cornerRadius << 8) | (width & 0x03), rP);  // XXX it is not the same thing, corner and shadow are ignored
}

UInt32 WinSetScalingMode(UInt32 mode) {
  debug(DEBUG_ERROR, "Window", "WinSetScalingMode not implemented");
  return 0;
}

UInt32 WinGetScalingMode(void) {
  debug(DEBUG_ERROR, "Window", "WinGetScalingMode not implemented");
  return 0;
}

void WinSaveRectangle(WinHandle dstWin, const RectangleType *srcRect) {
  win_module_t *module = (win_module_t *)thread_get(win_key);
  RectangleType rect;
  UInt32 width, height;
  Coord srcX, srcY, srcW, srcH, w, h;
  UInt16 prevCoordSys;

  if (dstWin && srcRect) {
    srcX = srcRect->topLeft.x;
    srcY = srcRect->topLeft.y;
    srcW = srcRect->extent.x;
    srcH = srcRect->extent.y;
    WinScreenMode(winScreenModeGet, &width, &height, NULL, NULL);
    debug(DEBUG_TRACE, "Window", "WinSaveRectangle %d,%d,%d,%d", srcX, srcY, srcW, srcH);

    pointTo(module->density, &srcX, &srcY);
    pointTo(module->density, &srcW, &srcH);
    w = width;
    h = height;
    pointTo(module->density, &w, &h);
    width = w;
    height = h;

    if (srcX < 0) srcX = 0;
    if (srcX >= width) srcX = width-1;
    if ((srcX + srcW) > width) {
      srcW = width - srcX;
    }
    if (srcY < 0) srcY = 0;
    if (srcY >= height) srcY = height-1;
    if ((srcY + srcH) > height) {
      srcH = width - srcY;
    }

    debug(DEBUG_TRACE, "Window", "WinSaveRectangle adj %d,%d,%d,%d", srcX, srcY, srcW, srcH);

    RctSetRectangle(&rect, srcX, srcY, srcW, srcH);
    prevCoordSys = WinSetCoordinateSystem(module->density == kDensityDouble ? kCoordinatesDouble : kCoordinatesStandard);
    WinCopyRectangle(WinGetDisplayWindow(), dstWin, &rect, 0, 0, winPaint);
    WinSetCoordinateSystem(prevCoordSys);
  }
}

void WinRestoreRectangle(WinHandle srcWin, const RectangleType *dstRect) {
  win_module_t *module = (win_module_t *)thread_get(win_key);
  RectangleType rect;
  UInt32 width, height;
  Coord dstX, dstY, dstW, dstH, w, h;
  UInt16 prevCoordSys;

  if (srcWin && dstRect) {
    dstX = dstRect->topLeft.x;
    dstY = dstRect->topLeft.y;
    dstW = dstRect->extent.x;
    dstH = dstRect->extent.y;
    WinScreenMode(winScreenModeGet, &width, &height, NULL, NULL);
    debug(DEBUG_TRACE, "Window", "WinRestoreRectangle %d,%d,%d,%d", dstX, dstY, dstW, dstH);

    pointTo(module->density, &dstX, &dstY);
    pointTo(module->density, &dstW, &dstH);
    w = width;
    h = height;
    pointTo(module->density, &w, &h);
    width = w;
    height = h;

    if (dstX < 0) dstX = 0;
    if (dstX >= width) dstX = width-1;
    if ((dstX + dstW) > width) {
      dstW = width - dstX;
    }
    if (dstY < 0) dstY = 0;
    if (dstY >= height) dstY = height-1;
    if ((dstY + dstH) > height) {
      dstH = width - dstY;
    }

    debug(DEBUG_TRACE, "Window", "WinRestoreRectangle adj %d,%d,%d,%d", dstX, dstY, dstW, dstH);

    WinHandle old = WinGetActiveWindow();
    WinHandle wh = WinGetDisplayWindow();
    WinSetActiveWindow(wh);

    RctSetRectangle(&rect, 0, 0, dstW, dstH);
    prevCoordSys = WinSetCoordinateSystem(module->density == kDensityDouble ? kCoordinatesDouble : kCoordinatesStandard);
    WinCopyRectangle(srcWin, wh, &rect, dstX, dstY, winPaint);
    WinSetCoordinateSystem(prevCoordSys);

    WinSetActiveWindow(old);
  }
}

// Scroll a rectangle in the draw window.
void WinScrollRectangle(const RectangleType *rP, WinDirectionType direction, Coord distance, RectangleType *vacatedP) {
  win_module_t *module = (win_module_t *)thread_get(win_key);
  RectangleType rect;

  if (module->drawWindow && rP && vacatedP && distance > 0) {
      switch (direction) {
        case winUp:
//debug(1, "XXX", "WinScrollRectangle %p (%d,%d,%d,%d) %d", module->drawWindow, rP->topLeft.x, rP->topLeft.y, rP->extent.x, rP->extent.y, distance);
          RctSetRectangle(&rect, rP->topLeft.x, rP->topLeft.y + distance, rP->extent.x, rP->extent.y - distance);
          WinCopyRectangle(module->drawWindow, module->drawWindow, &rect, rP->topLeft.x, rP->topLeft.y, winPaint);
          RctSetRectangle(vacatedP, rP->topLeft.x, rP->topLeft.y + rP->extent.y - distance, rP->extent.x, distance);
          break;
        case winDown:
          RctSetRectangle(&rect, rP->topLeft.x, rP->topLeft.y, rP->extent.x, rP->extent.y - distance);
          WinCopyRectangle(module->drawWindow, module->drawWindow, &rect, rP->topLeft.x, rP->topLeft.y + distance, winPaint);
          RctSetRectangle(vacatedP, rP->topLeft.x, rP->topLeft.y, rP->extent.x, distance);
          break;
        case winLeft:
          RctSetRectangle(&rect, rP->topLeft.x + distance, rP->topLeft.y, rP->extent.x - distance, rP->extent.y);
          WinCopyRectangle(module->drawWindow, module->drawWindow, &rect, rP->topLeft.x, rP->topLeft.y, winPaint);
          RctSetRectangle(vacatedP, rP->topLeft.x + rP->extent.x - distance, rP->topLeft.y, distance, rP->extent.y);
          break;
        case winRight:
          RctSetRectangle(&rect, rP->topLeft.y, rP->topLeft.x, rP->extent.x - distance, rP->extent.y);
          WinCopyRectangle(module->drawWindow, module->drawWindow, &rect, rP->topLeft.x + distance, rP->topLeft.y, winPaint);
          RctSetRectangle(vacatedP, rP->topLeft.x, rP->topLeft.y, distance, rP->extent.y);
          break;
      }
  }
}

WinHandle WinCreateOffscreenWindow(Coord width, Coord height, WindowFormatType format, UInt16 *error) {
  win_module_t *module = (win_module_t *)thread_get(win_key);
  UInt16 density, depth;
  Coord w0, h0;
  WinHandle wh = NULL;
  Err err = sysErrNoFreeResource;

  if ((wh = pumpkin_heap_alloc(sizeof(WindowType), "Window")) != NULL) {
    RctSetRectangle(&wh->windowBounds, 0, 0, width, height);
    //WinSetClipingBounds(wh, &wh->windowBounds);
    w0 = width;
    h0 = height;

    switch (format) {
      case screenFormat:
        // The window’s bitmap is allocated using the hardware screen’s depth, but for backward compatibility the bitmap associated with the
        // offscreen window is always low density, and the window always uses a coordinate system that directly maps offscreen pixels to coordinates.
        density = kDensityLow;
        depth = module->depth;
        debug(DEBUG_INFO, "Window", "WinCreateOffscreenWindow creating low density window (screenFormat)");
        break;
      case genericFormat:
        // Like screenFormat, except that genericFormat offscreen windows do not accept pen input.
        wh->windowFlags.format = true;
        density = kDensityLow;
        depth = module->depth;
        debug(DEBUG_INFO, "Window", "WinCreateOffscreenWindow creating low density window (genericFormat)");
        break;
      case nativeFormat:
        // Reflects the actual hardware screen format in all ways, including screen depth, density, and pixel format.
        // When using this format, the width and height arguments must be specified using the active coordinate system.
        density = module->density;
        depth = module->depth; // XXX BikeOrDie does not work when using "depth = DEPTH" here
        if (density == kDensityDouble && module->coordSys == kCoordinatesStandard) {
          width *= 2;
          height *= 2;
        }
        break;
    }
    debug(DEBUG_TRACE, "Window", "WinCreateOffscreenWindow %d,%d format %d depth %d density %d", width, height, format, depth, density);

    wh->bitmapP = BmpCreate3(width, height, density, depth, false, 0, NULL, &err);
    if (wh->bitmapP) {
      wh->windowFlags.offscreen = true;
      wh->windowFlags.freeBitmap = true;
      err = errNone;

      // fill window bitmap with white color
      IndexedColorType old = WinSetForeColor(0x00);
      WinHandle p = WinSetDrawWindow(wh);
      WinDrawRectangle(&wh->windowBounds, 0);
      WinSetDrawWindow(p);
      WinSetForeColor(old);

      directAccessHack(wh, 0, 0, w0, h0);
      wh->density = density;
    } else {
      pumpkin_heap_free(wh, "Window");
      wh = NULL;
    }
//debug(1, "XXX", "WinCreateOffscreenWindow %dx%d depth %d, density %d, bitmap %p: %p", width, height, depth, density, wh->bitmapP, wh);
  }

  if (error) *error = err;

  return wh;
}

/*
static void broadcastDisplayChange(UInt32 oldDepth, UInt32 newDepth) {
  SysNotifyDisplayChangeDetailsType data;
  SysNotifyParamType notify;

  data.oldDepth = oldDepth;
  data.newDepth = newDepth;

  MemSet(&notify, sizeof(SysNotifyParamType), 0);
  notify.notifyType = sysNotifyDisplayChangeEvent;
  notify.broadcaster = 0; // XXX
  notify.notifyDetailsP = (void *)&data;

  SysNotifyBroadcast(&notify);
}
*/

// Set or retrieve the palette for the draw window
Err WinPalette(UInt8 operation, Int16 startIndex, UInt16 paletteEntries, RGBColorType *tableP) {
  win_module_t *module = (win_module_t *)thread_get(win_key);
  WinHandle wh;
  ColorTableType *colorTable;
  UInt16 i, index;
  Err err = sysErrParamErr;

  wh = module->drawWindow;

  if (wh) {
    colorTable = BmpGetColortable(wh->bitmapP);
    if (colorTable == NULL) {
      colorTable = module->colorTable;
      debug(DEBUG_INFO, "Window", "WinPalette drawWindow colorTable is NULL, using system colorTable");
    }

    if (colorTable) {
      switch (operation) {
        case winPaletteGet:
          if (tableP) {
            if (startIndex == WinUseTableIndexes) {
              for (i = 0; i < paletteEntries; i++) {
                index = tableP[i].index;
                if (index < colorTable->numEntries) {
                  tableP[i] = colorTable->entry[index];
                }
              }
            } else {
              for (i = 0; i < paletteEntries; i++) {
                if ((startIndex + i) < colorTable->numEntries) {
                  tableP[i] = colorTable->entry[startIndex + i];
                }
              }
            }
            err = errNone;
          }
          break;

        // During a set operation, this function broadcasts the
        // sysNotifyDisplayChangeEvent to notify any interested
        // observer that the color palette has changed.

        case winPaletteSet:
          if (tableP) {
            if (startIndex == WinUseTableIndexes) {
              for (i = 0; i < paletteEntries; i++) {
                index = tableP[i].index;
                if (index < colorTable->numEntries) {
                  colorTable->entry[index] = tableP[i];
                }
              }
            } else {
              for (i = 0; i < paletteEntries; i++) {
                if ((startIndex + i) < colorTable->numEntries) {
                  colorTable->entry[startIndex + i] = tableP[i];
                }
              }
            }
            //broadcastDisplayChange(module->depth, module->depth);
            err = errNone;
          }
          break;

        case winPaletteSetToDefault:
          for (i = 0; i < colorTable->numEntries; i++) {
            colorTable->entry[i] = defaultPalette[i];
          }
          //broadcastDisplayChange(module->depth, module->depth);
          err = errNone;
          break;
      }
    } else {
      debug(DEBUG_ERROR, "Window", "WinPalette drawWidoww colorTable is NULL");
      err = winErrPalette;
    }
  }

  return err;
}

void WinPushDrawState(void) {
  win_module_t *module = (win_module_t *)thread_get(win_key);

  debug(DEBUG_TRACE, "Window", "WinPushDrawState");
  if (module->numPush < DrawStateStackSize) {
    module->state[module->numPush].pattern = module->pattern;
    module->state[module->numPush].underlineMode = module->underlineMode;
    module->state[module->numPush].transferMode = module->transferMode;
    module->state[module->numPush].foreColor = module->foreColor;
    module->state[module->numPush].backColor = module->backColor;
    module->state[module->numPush].textColor = module->textColor;
    module->state[module->numPush].foreColorRGB = module->foreColorRGB;
    module->state[module->numPush].backColorRGB = module->backColorRGB;
    module->state[module->numPush].textColorRGB = module->textColorRGB;
    module->state[module->numPush].coordinateSystem = module->coordSys;
    module->state[module->numPush].fontId = FntGetFont();
    WinGetPattern(&module->state[module->numPush].patternData);
    module->numPush++;

  } else {
    debug(DEBUG_ERROR, "Window", "WinPushDrawState over push");
  }
}

void WinPopDrawState(void) {
  win_module_t *module = (win_module_t *)thread_get(win_key);

  debug(DEBUG_TRACE, "Window", "WinPopDrawState");
  if (module->numPush) {
    module->numPush--;
    WinSetPattern(&module->state[module->numPush].patternData);
    module->pattern = module->state[module->numPush].pattern;
    module->underlineMode = module->state[module->numPush].underlineMode;
    module->transferMode = module->state[module->numPush].transferMode;
    module->foreColor = module->state[module->numPush].foreColor;
    module->backColor = module->state[module->numPush].backColor;
    module->textColor = module->state[module->numPush].textColor;
    module->foreColorRGB = module->state[module->numPush].foreColorRGB;
    module->backColorRGB = module->state[module->numPush].backColorRGB;
    module->textColorRGB = module->state[module->numPush].textColorRGB;
    module->foreColor565 = rgb565(module->foreColorRGB.r, module->foreColorRGB.g, module->foreColorRGB.b);
    module->backColor565 = rgb565(module->backColorRGB.r, module->backColorRGB.g, module->backColorRGB.b);
    module->textColor565 = rgb565(module->textColorRGB.r, module->textColorRGB.g, module->textColorRGB.b);
    module->coordSys = module->state[module->numPush].coordinateSystem;
    FntSetFont(module->state[module->numPush].fontId);

  } else {
    debug(DEBUG_ERROR, "Window", "WinPushDrawState under pop");
  }
}

Err WinScreenMode(WinScreenModeOperation operation, UInt32 *widthP, UInt32 *heightP, UInt32 *depthP, Boolean *enableColorP) {
  win_module_t *module = (win_module_t *)thread_get(win_key);
  Coord width, height;
  BitmapTypeV3 *bmp;
  Err err = sysErrParamErr;

  switch (operation) {
    case winScreenModeGetDefaults:
    case winScreenModeGet:
       // For backward compatibility, when operation is winScreenModeGet or winScreenModeGetDefaults,
       // a single-density width or height is returned, even if the handheld has a double-density display.
      width = module->width;
      height = module->height;
      pointFrom(module->density, &width, &height);
      if (depthP) *depthP = operation == winScreenModeGet ? module->depth : module->depth0;
      if (widthP) *widthP = width;
      if (heightP) *heightP = height;
      if (enableColorP) *enableColorP = true;
      err = errNone;
      break;

    case winScreenModeSetToDefaults:
      break;

    case winScreenModeSet:
      bmp = (BitmapTypeV3 *)module->displayWindow->bitmapP;

// XXX PalmVNC calls this with 320x320, which would mean 640x640, which is wrong
/*
      if (widthP && heightP) {
        width = *widthP;
        height = *heightP;
        WinAdjustCoords(&width, &height);
        bmp->width = width;
        bmp->height = height;
        err = errNone;
      }
*/

      if (depthP) {
        switch (*depthP) {
          case 1:
            bmp->pixelSize = 1;
            bmp->rowBytes = (bmp->width + 7) / 8;
            bmp->bitsSize = bmp->height * bmp->rowBytes;
            module->depth = *depthP;
            err = errNone;
            break;
          case 2:
            bmp->pixelSize = 2;
            bmp->rowBytes = (bmp->width + 3) / 4;
            bmp->bitsSize = bmp->height * bmp->rowBytes;
            module->depth = *depthP;
            err = errNone;
            break;
          case 4:
            bmp->pixelSize = 4;
            bmp->rowBytes = (bmp->width + 1) / 2;
            bmp->bitsSize = bmp->height * bmp->rowBytes;
            module->depth = *depthP;
            err = errNone;
            break;
          case 8:
            bmp->pixelSize = 8;
            bmp->rowBytes = bmp->width;
            bmp->bitsSize = bmp->height * bmp->rowBytes;
            module->depth = *depthP;
            err = errNone;
            break;
          case 16:
            //broadcastDisplayChange(module->depth, *depthP);
            bmp->pixelSize = 16;
            bmp->rowBytes = bmp->width * 2;
            bmp->bitsSize = bmp->height * bmp->rowBytes;
            module->depth = *depthP;
            err = errNone;
            break;
          default:
            debug(DEBUG_ERROR, "Window", "WinScreenMode winScreenModeSet depth %d not supported", *depthP);
            break;
        }
      }
      BmpFillData((BitmapType *)bmp);
      break;

    case winScreenModeGetSupportedDepths:
      // The position representing a particular bit depth corresponds to the value 2^(bitDepth-1)
      if (depthP) {
        // XXX 1, 2 and 4 ?
        *depthP  = 1 << ( 8 - 1);
        *depthP |= 1 << (16 - 1);
        err = 0;
      }
      break;

    case winScreenModeGetSupportsColor:
      if (enableColorP) *enableColorP = true;
      err = 0;
      break;

    default:
      debug(DEBUG_ERROR, "Window", "WinScreenMode invalid operation %d", operation);
      break;
  }

  return err;
}

Err WinSetConstraintsSize(WinHandle winH, Coord minH, Coord prefH, Coord maxH, Coord minW, Coord prefW, Coord maxW) {
  Err err = sysErrParamErr;

  if (winH) {
    winH->minH = minH;
    winH->prefH = prefH;
    winH->maxH = maxH;
    winH->minW = minW;
    winH->prefW = prefW;
    winH->maxW = maxW;
    err = errNone;
  }

  return err;
}

static Boolean isSpace(Char c) {
  return c == ' ' || c == '\t';
}

static Boolean isLineBreak(Char c) {
  return c == '\r' || c == '\n';
}

void WinDrawCharBox(Char *text, UInt16 len, FontID font, RectangleType *bounds, Boolean draw, UInt16 *drawnLines, UInt16 *totalLines, UInt16 *maxWidth, LineInfoType *lineInfo) {
  UInt16 drawn, total, start, span;
  UInt16 i, tw, th, x, y;
  UInt16 lastSpaceOffset, width;
  Boolean hasSpace;
  FontID old;
  Char c;

  old = FntSetFont(font);
  th = FntCharHeight();
  drawn = 0;
  total = 0;
  start = 0;
  x = 0;
  y = 0;
  lastSpaceOffset = 0;
  hasSpace = false;
  if (maxWidth) *maxWidth = 0;

  if (text && len && bounds) {
    debug(DEBUG_TRACE, "Window", "WinDrawCharBox: text \"%.*s\"", len, text);

    for (i = 0; i < len;) {
      c = text[i];
      debug(DEBUG_TRACE, "Window", "WinDrawCharBox: char %d \'%c\'", i, c);
      tw = FntCharWidth(c);
      if (isSpace(c)) {
        lastSpaceOffset = i;
        hasSpace = true;
      }
      i++;
      if (isLineBreak(c) || x + tw >= bounds->extent.x) {
        debug(DEBUG_TRACE, "Window", "WinDrawCharBox: line break x=%d tw=%d bounds=%d", x, tw, bounds->extent.x);
        if (!isLineBreak(c) && hasSpace) {
          span = lastSpaceOffset - start;
        } else {
          // XXX fica uma caracter a mais no fim da linha. com -1 resolve, mas da crash com ENTER
          span = i /*- 1*/ - start;
        }
        if ((y + th) <= bounds->extent.y) {
          if (draw) WinDrawChars(&text[start], span, bounds->topLeft.x, bounds->topLeft.y + y);
          width = FntCharsWidth(&text[start], span);
          if (maxWidth && width > *maxWidth) *maxWidth = width;
          drawn++;
        }
        if (lineInfo) {
          lineInfo[total].start = start;
          lineInfo[total].length = span;
        }
        for (start += span; isSpace(text[start]) && start < len; start++);
        i = start;
        debug(DEBUG_TRACE, "Window", "WinDrawCharBox: i=start=%d", i);
        x = 0;
        y += th;
        lastSpaceOffset = 0;
        hasSpace = false;
        total++;
      }
      if (!isLineBreak(c)) x += tw;
    }
    debug(DEBUG_TRACE, "Window", "WinDrawCharBox: end loop len=%d i=%d start=%d", len, i, start);
    if (i > start) {
      if ((y + th) <= bounds->extent.y) {
        if (draw) WinDrawChars(&text[start], i - start, bounds->topLeft.x, bounds->topLeft.y + y);
        width = FntCharsWidth(&text[start], i - start);
        if (maxWidth && width > *maxWidth) *maxWidth = width;
        drawn++;
      }
      if (lineInfo) {
        lineInfo[total].start = start;
        lineInfo[total].length = i - start;
      }
      total++;
    }
  }

  FntSetFont(old);

  if (text && lineInfo) {
    if (bounds) {
      debug(DEBUG_TRACE, "Window", "WinDrawCharBox: bounds (%d,%d,%d,%d)", bounds->topLeft.x, bounds->topLeft.y, bounds->extent.x, bounds->extent.y);
    }
    for (i = 0; i < total; i++) {
      debug(DEBUG_TRACE, "Window", "WinDrawCharBox: line %d start %d len %d \"%.*s\"", i, lineInfo[i].start, lineInfo[i].length, lineInfo[i].length, &text[lineInfo[i].start]);
    }
  }

  if (drawnLines) *drawnLines = drawn;
  if (totalLines) *totalLines = total;
  debug(DEBUG_TRACE, "Window", "WinDrawCharBox: drawn %d, total %d", drawn, total);
}

void EvtGetPenNative(WinHandle winH, Int16* pScreenX, Int16* pScreenY, Boolean* pPenDown) {
  WinHandle old;

  if (winH) {
    old = WinGetActiveWindow();
    WinSetActiveWindow(winH);
    EvtGetPen(pScreenX, pScreenY, pPenDown);
    WinSetActiveWindow(old);
  }
}

void WinInvertRect(RectangleType *rect, UInt16 corner) {
  IndexedColorType objFore, objFill, objSelFill, objSelFore, oldb, oldf;
  RectangleType aux;
  WinDrawOperation prev;
  UInt16 coordSys;

  objFill = UIColorGetTableEntryIndex(UIObjectFill);
  objFore = UIColorGetTableEntryIndex(UIObjectForeground);
  objSelFill = UIColorGetTableEntryIndex(UIObjectSelectedFill);
  objSelFore = UIColorGetTableEntryIndex(UIFormTitle);
  prev = WinSetDrawMode(winSwap);

  // using double coordinates to preserve font shape
  MemMove(&aux, rect, sizeof(RectangleType));
  coordSys = WinSetCoordinateSystem(kCoordinatesDouble);
  if (coordSys == kCoordinatesStandard) WinScaleRectangle(&aux);
  corner = WinScaleCoord(corner, false);

  oldb = WinSetBackColor(objFill);
  oldf = WinSetForeColor(objSelFill);
  WinPaintRectangle(&aux, corner);

  WinSetBackColor(objFore);
  WinSetForeColor(objSelFore);
  WinPaintRectangle(&aux, corner);

  WinSetCoordinateSystem(coordSys);
  WinSetBackColor(oldb);
  WinSetForeColor(oldf);
  WinSetDrawMode(prev);
}

void WinSendWindowEvents(WinHandle wh) {
  win_module_t *module = (win_module_t *)thread_get(win_key);
  EventType event;

  if (module->activeWindow) {
    MemSet(&event, sizeof(event), 0);
    event.eType = winExitEvent;
    event.data.winExit.exitWindow = module->activeWindow;
    event.data.winExit.enterWindow = wh;
    EvtAddEventToQueue(&event);
  }

  MemSet(&event, sizeof(event), 0);
  event.eType = winEnterEvent;
  event.data.winEnter.exitWindow = module->activeWindow;
  event.data.winEnter.enterWindow = wh;
  EvtAddEventToQueue(&event);
}

void WinLegacyGetAddr(UInt32 *startAddr, UInt32 *endAddr) {
  win_module_t *module = (win_module_t *)thread_get(win_key);
  BitmapType *bmp = WinGetBitmap(module->displayWindow);
  uint8_t *bits = BmpGetBits(bmp);
  uint32_t len = bmp->width * bmp->height;

  switch (BmpGetBitDepth(bmp)) {
    case 1: len /= 8; break;
    case 2: len /= 4; break;
    case 4: len /= 2; break;
  }

  *startAddr = bits - (uint8_t *)pumpkin_heap_base();
  *endAddr = *startAddr + len;
}

static void WinLegacyWrite(UInt32 offset, UInt32 value, UInt16 n) {
  win_module_t *module = (win_module_t *)thread_get(win_key);
  BitmapType *bmp = WinGetBitmap(module->displayWindow);
  RectangleType rect;
  WinHandle old;
  Coord cols, x, y;

  switch (BmpGetBitDepth(bmp)) {
    case 1:
      cols = bmp->width / 8;
      x = (offset % cols) * 8;
      y = offset / cols;
      RctSetRectangle(&rect, x, y, 8*n, 1);
      break;
    case 2:
      cols = bmp->width / 4;
      x = (offset % cols) * 4;
      y = offset / cols;
      RctSetRectangle(&rect, x, y, 4*n, 1);
      break;
    case 4:
      cols = bmp->width / 2;
      x = (offset % cols) * 2;
      y = offset / cols;
      RctSetRectangle(&rect, x, y, 2*n, 1);
      break;
  }

  old = module->activeWindow;
  module->activeWindow = module->displayWindow;
  dirty_region(x, y, x + rect.extent.x, y + rect.extent.y);
  module->activeWindow = old;
}

void WinLegacyWriteByte(UInt32 offset, UInt8 value) {
  WinLegacyWrite(offset, value, 1);
}

void WinLegacyWriteWord(UInt32 offset, UInt16 value) {
  WinLegacyWrite(offset, value, 2);
}

void WinLegacyWriteLong(UInt32 offset, UInt32 value) {
  WinLegacyWrite(offset, value, 4);
}

static void WinSurfaceSetPixel(void *data, int x, int y, uint32_t color) {
  win_module_t *module = (win_module_t *)thread_get(win_key);
  win_surface_t *wsurf = (win_surface_t *)data;
  WinHandle old;
  UInt16 prev;
  uint32_t oldc;

  old = WinSetDrawWindow(wsurf->wh);
  oldc = module->foreColor565;
  module->foreColor565 = color;
  prev = WinSetCoordinateSystem(wsurf->coordSys);
  WinPaintPixel(wsurf->rect.topLeft.x + x, wsurf->rect.topLeft.y + y);
  WinSetCoordinateSystem(prev);
  module->foreColor565 = oldc;
  WinSetDrawWindow(old);
}

static uint32_t WinSurfaceColorRGB(void *data, int red, int green, int blue, int alpha) {
  return rgb565(red, green, blue);
}

static void WinSurfaceDestroy(void *data) {
  win_surface_t *wsurf = (win_surface_t *)data;

  if (wsurf) {
    xfree(wsurf);
  }
}

surface_t *WinCreateSurface(WinHandle wh, RectangleType *rect) {
  surface_t *surface = NULL;
  win_surface_t *wsurf;
  BitmapType *bitmapP;
  UInt16 prev, density;

  bitmapP = WinGetBitmap(wh);

  if (BmpGetBitDepth(bitmapP) != 16) {
    debug(DEBUG_ERROR, "Bitmap", "WinCreateSurface supports only 16 bits");
    return NULL;
  }

  if ((wsurf = xcalloc(1, sizeof(win_surface_t))) != NULL) {
    if ((surface = xcalloc(1, sizeof(surface_t))) != NULL) {
      wsurf->wh = wh;
      xmemcpy(&wsurf->rect, rect, sizeof(RectangleType));
      density = BmpGetDensity(bitmapP);
      wsurf->coordSys = density == kDensityDouble ? kCoordinatesDouble : kCoordinatesStandard;
      prev = WinSetCoordinateSystem(wsurf->coordSys);
      if (prev != wsurf->coordSys) {
        switch (prev) {
          case kCoordinatesStandard:
            WinScaleRectangle(&wsurf->rect);
            break;
          case kCoordinatesDouble:
            WinUnscaleRectangle(&wsurf->rect);
            break;
        }
        WinSetCoordinateSystem(prev);
      }

      surface->tag = TAG_SURFACE;
      surface->encoding = SURFACE_ENCODING_RGB565;
      surface->width = wsurf->rect.extent.x;
      surface->height = wsurf->rect.extent.y;
      surface->setpixel = WinSurfaceSetPixel;
      surface->color_rgb = WinSurfaceColorRGB;
      surface->destroy = WinSurfaceDestroy;
      surface->data = wsurf;
    } else {
      xfree(wsurf);
    }
  }

  return surface;
}
