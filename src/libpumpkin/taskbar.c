#include <PalmOS.h>

#include "pumpkin.h"
#include "pwindow.h"
#include "surface.h"
#include "taskbar.h"
#include "debug.h"

#define MAX_TASKS      32
#define MAX_WIDGETS     8

#define WIDGET_WIDTH   16
#define WIDGET_HEIGHT  16

#define BACK_COLOR    223
#define FORE_COLOR    255
#define TEXT_COLOR    255
#define BORDER_COLOR  221
#define CLOCK_COLOR   222

#define TASK_WIDTH(i)     (1 + tb->tasks[i].bmpWidth + 1 + tb->tasks[i].nameWidth + 1)
#define TASKS_TOTAL_WIDTH (tb->width - tb->clockWidth)
#define TASK_EQUAL_WIDTH  ((TASKS_TOTAL_WIDTH + tb->num_tasks - 1) / tb->num_tasks)

typedef struct {
  Int32 taskId;
  LocalID dbID;
  UInt32 creator;
  char *name;
  BitmapType *bmp;
  UInt32 bmpWidth, bmpHeight;
  UInt32 nameWidth, width;
  Boolean truncated;
} taskbar_task_t;

typedef struct {
  Boolean (*callback)(void *data);
  void *data;
} taskbar_widget_callback_t;

typedef struct {
  Int32 taskId;
  UInt32 id;
  BitmapType *bmp;
} taskbar_widget_t;

struct taskbar_t {
  window_provider_t *wp;
  window_t *w;
  surface_t *surface;
  texture_t *texture;
  WinHandle wh;
  BitmapType *bmp;
  UInt16 density;
  UInt32 width, height, usedWidth, baseClockWidth, clockWidth;
  UInt32 x, y;
  taskbar_task_t tasks[MAX_TASKS];
  taskbar_widget_t widgets[MAX_WIDGETS];
  UInt32 num_tasks, num_widgets;
};

static UInt32 calc_task_width(taskbar_t *tb, UInt32 i) {
  UInt16 prevCoordSys, density;
  BitmapType *bmp;
  Coord width, height;

  prevCoordSys = WinSetCoordinateSystem(tb->density == kDensityDouble ? kCoordinatesDouble : kCoordinatesStandard);
  tb->tasks[i].bmpWidth = 0;
  tb->tasks[i].bmpHeight = 0;

  tb->tasks[i].width = 1; // empty column at the beginning

  if (tb->tasks[i].bmp) {
    if ((bmp = BmpGetBestBitmapEx(tb->tasks[i].bmp, tb->density, BmpGetBitDepth(tb->tasks[i].bmp), false)) != NULL) {
      BmpGetDimensions(bmp, &width, &height, NULL);
      density = BmpGetDensity(bmp);
      if (tb->density > density) {
        width <<= 1;
        height <<= 1;
      } else if (tb->density < density) {
        width >>= 1;
        height >>= 1;
      }
      tb->tasks[i].bmpWidth = width;
      tb->tasks[i].bmpHeight = height;
      tb->tasks[i].width += width; // bitmap width
      tb->tasks[i].width += 1; // empty column between bitmap and name
    }
  }

  tb->tasks[i].nameWidth = FntCharsWidth(tb->tasks[i].name, StrLen(tb->tasks[i].name));
  tb->tasks[i].width += tb->tasks[i].nameWidth; // name width
  tb->tasks[i].width += 1; // empty column at the end
  tb->tasks[i].truncated = false;
  WinSetCoordinateSystem(prevCoordSys);

  return tb->tasks[i].width;
}

static void draw_separator(taskbar_t *tb, Coord x) {
  WinSetForeColor(BORDER_COLOR);
  WinPaintLine(x, 0, x, tb->height - 1);
}

void taskbar_update(taskbar_t *tb) {
  WinHandle oldDrawWindow;
  IndexedColorType oldBackColor, oldForeColor, oldTextColor;
  PatternType oldPattern;
  RectangleType rect;
  FontID oldFont;
  DateTimeType dt;
  BitmapType *bmp;
  UInt16 prevCoordSys, len, truncLen, maxWidth, i;
  UInt32 t, tf, nameHeight;
  Coord x0, x, y;
  char buf[dmDBNameLength];

  prevCoordSys = WinSetCoordinateSystem(tb->density == kDensityDouble ? kCoordinatesDouble : kCoordinatesStandard);
  oldDrawWindow = WinSetDrawWindow(tb->wh);
  oldPattern = WinGetPatternType();
  WinSetPatternType(blackPattern);
  oldFont = FntSetFont(stdFont);
  oldBackColor = WinSetBackColor(BACK_COLOR);
  oldForeColor = WinSetForeColor(FORE_COLOR);
  oldTextColor = WinSetTextColor(TEXT_COLOR);
  WinEraseWindow();

  x = 0;
  for (i = 0; i < tb->num_tasks; i++) {
    RctSetRectangle(&rect, x, 0, tb->tasks[i].width, tb->height);
    WinSetClip(&rect);
    x0 = x;
    draw_separator(tb, x);
    x += 1;
    if (tb->tasks[i].bmp) {
      if ((bmp = BmpGetBestBitmapEx(tb->tasks[i].bmp, tb->density, BmpGetBitDepth(tb->tasks[i].bmp), false)) != NULL) {
        y = tb->height > tb->tasks[i].bmpHeight ? (tb->height - tb->tasks[i].bmpHeight) / 2 : 0;
        WinPaintBitmapEx(bmp, x, y, false);
        x += 1;
        x += tb->tasks[i].bmpWidth;
      }
    }
    nameHeight = FntCharHeight();
    y = tb->height > nameHeight ? (tb->height - nameHeight) / 2 : 0;
    WinSetForeColor(TEXT_COLOR);
    len = StrLen(tb->tasks[i].name);
    if (tb->tasks[i].truncated) {
      // name does not fit into slot, truncate and add "..."
      maxWidth = tb->tasks[i].width - (1 + tb->tasks[i].bmpWidth + 1 + 1 + 6 + FntCharWidth(chrEllipsis));
      truncLen = FntWidthToOffset(tb->tasks[i].name, len, maxWidth, NULL, NULL);
      if (truncLen > 0) {
        WinPaintChars(tb->tasks[i].name, truncLen, x, y);
        WinPaintChar(chrEllipsis, x + FntCharsWidth(tb->tasks[i].name, truncLen), y);
      }
    } else {
      WinPaintChars(tb->tasks[i].name, len, x, y);
    }
    draw_separator(tb, x0 + tb->tasks[i].width - 1);
    x = x0 + tb->tasks[i].width;
    WinResetClip();
  }

  WinSetBackColor(CLOCK_COLOR);
  RctSetRectangle(&rect, TASKS_TOTAL_WIDTH, 0, tb->clockWidth, tb->height);
  WinEraseRectangle(&rect, 0);

  t = TimGetSeconds();
  TimSecondsToDateTime(t, &dt);
  tf = PrefGetPreference(prefTimeFormat);
  TimeToAscii(dt.hour, dt.minute, tf, buf);
  len = StrLen(buf);
  x = FntCharsWidth(buf, len);
  WinSetForeColor(TEXT_COLOR);
  WinPaintChars(buf, len, tb->width - (tb->baseClockWidth + x) / 2, 0);

  x = tb->width - tb->baseClockWidth;
  for (i = 0; i < tb->num_widgets; i++) {
    if ((bmp = BmpGetBestBitmapEx(tb->widgets[i].bmp, tb->density, BmpGetBitDepth(tb->widgets[i].bmp), false)) != NULL) {
      x -= WIDGET_WIDTH;
      y = (tb->height - WIDGET_HEIGHT) / 2;
      WinPaintBitmapEx(bmp, x, y, false);
    }
  }

  FntSetFont(oldFont);
  WinSetPatternType(oldPattern);
  WinSetBackColor(oldBackColor);
  WinSetForeColor(oldForeColor);
  WinSetTextColor(oldTextColor);
  WinSetDrawWindow(oldDrawWindow);
  WinSetCoordinateSystem(prevCoordSys);

  BmpDrawSurface(tb->bmp, 0, 0, tb->width, tb->height, tb->surface, 0, 0, true, false);
}

taskbar_t *taskbar_create(window_provider_t *wp, window_t *w, uint16_t density, uint32_t x, uint32_t y, uint32_t width, uint32_t height, int encoding) {
  taskbar_t *tb;
  UInt16 prevCoordSys;
  char buf[32];
  Err err;

  if ((tb = sys_calloc(1, sizeof(taskbar_t))) != NULL) {
    if ((tb->surface = surface_create(width, height, encoding)) != NULL) {
      prevCoordSys = WinSetCoordinateSystem(density == kDensityDouble ? kCoordinatesDouble : kCoordinatesStandard);
      if ((tb->wh = WinCreateOffscreenWindow(width, height, nativeFormat, &err)) != NULL) {
        tb->wp = wp;
        tb->w = w;
        tb->density = density;
        tb->bmp = WinGetBitmap(tb->wh);
        tb->x = x;
        tb->y = y;
        tb->width = width;
        tb->height = height;
        StrCopy(buf, "99:99 am");
        tb->baseClockWidth = FntCharsWidth(buf, StrLen(buf)) + 2;
        tb->clockWidth = tb->baseClockWidth;
        taskbar_update(tb);
      } else {
        tb->wp->destroy_texture(tb->w, tb->texture);
        surface_destroy(tb->surface);
        sys_free(tb);
        tb = NULL;
      }
      WinSetCoordinateSystem(prevCoordSys);
    } else {
      sys_free(tb);
      tb = NULL;
    }
  }

  return tb;
}

void taskbar_destroy(taskbar_t *tb) {
  UInt16 i;

  if (tb) {
    for (i = 0; i < tb->num_tasks; i++) {
      if (tb->tasks[i].name) sys_free(tb->tasks[i].name);
      if (tb->tasks[i].bmp) sys_free(tb->tasks[i].bmp);
    }
    for (i = 0; i < tb->num_widgets; i++) {
      if (tb->widgets[i].bmp) sys_free(tb->widgets[i].bmp);
    }
    if (tb->wh) WinDeleteWindow(tb->wh, false);
    if (tb->surface) surface_destroy(tb->surface);
    sys_free(tb);
  }
}

static void recalc_space(taskbar_t *tb) {
  UInt32 d, i;

  if (tb->usedWidth > TASKS_TOTAL_WIDTH) {
    // taskbar can not fit all tasks, divide space equally between them
    tb->usedWidth = 0;
    d = TASK_EQUAL_WIDTH;
    for (i = 0; i < tb->num_tasks; i++) {
      tb->tasks[i].truncated = TASK_WIDTH(i) > d;
      tb->tasks[i].width = d;
      tb->usedWidth += d;
    }
  }
}

void taskbar_add(taskbar_t *tb, Int32 taskId, LocalID dbID, UInt32 creator, char *name) {
  DmOpenRef dbRef;
  MemHandle h;
  BitmapType *bmp;
  UInt32 size;

  if (tb->num_tasks < MAX_TASKS) {
    tb->tasks[tb->num_tasks].taskId = taskId;
    tb->tasks[tb->num_tasks].dbID = dbID;
    tb->tasks[tb->num_tasks].creator = creator;
    tb->tasks[tb->num_tasks].name = sys_strdup(name);
    tb->tasks[tb->num_tasks].truncated = false;

    if ((dbRef = DmOpenDatabase(0, dbID, dmModeReadOnly)) != NULL) {
      h = DmGet1Resource(iconType, 1001);
      if (h == NULL) h = DmGetResource(iconType, 10001);
      if (h != NULL) {
        if ((bmp = MemHandleLock(h)) != NULL) {
          size = MemHandleSize(h);
          if ((tb->tasks[tb->num_tasks].bmp = sys_calloc(1, size)) != NULL) {
            sys_memcpy(tb->tasks[tb->num_tasks].bmp, bmp, size);
          }
          MemHandleUnlock(h);
        }
        DmReleaseResource(h);
      }
      DmCloseDatabase(dbRef);
    }

    tb->usedWidth += calc_task_width(tb, tb->num_tasks);
    tb->num_tasks++;
    recalc_space(tb);
    taskbar_update(tb);
  }
}

void taskbar_remove(taskbar_t *tb, LocalID dbID) {
  UInt32 i, d, usedWidth;

  for (i = 0; i < tb->num_tasks; i++) {
    if (tb->tasks[i].dbID == dbID) {
      if (tb->tasks[i].name) sys_free(tb->tasks[i].name);
      if (tb->tasks[i].bmp) sys_free(tb->tasks[i].bmp);
      tb->tasks[i].name = NULL;
      tb->tasks[i].bmp = NULL;
      break;
    }
  }

  if (i == tb->num_tasks) return;

  tb->usedWidth -= tb->tasks[i].width;

  for (; i < tb->num_tasks - 1; i++) {
    tb->tasks[i] = tb->tasks[i+1];
  }
  tb->num_tasks--;

  if (tb->usedWidth > TASKS_TOTAL_WIDTH) {
    // remaining tasks still do not fit in available space
    tb->usedWidth = 0;
    d = TASK_EQUAL_WIDTH;
    for (i = 0; i < tb->num_tasks; i++) {
      tb->tasks[i].truncated = TASK_WIDTH(i) > d;
      tb->tasks[i].width = d;
      tb->usedWidth += d;
    }
  } else {
    // remaining tasks fit in available space, check if truncation is still needed
    usedWidth = 0;
    for (i = 0; i < tb->num_tasks; i++) {
      usedWidth += TASK_WIDTH(i);
    }
    if (usedWidth > TASKS_TOTAL_WIDTH) {
      // truncation is still needed
      tb->usedWidth = 0;
      d = TASK_EQUAL_WIDTH;
      for (i = 0; i < tb->num_tasks; i++) {
        tb->tasks[i].truncated = TASK_WIDTH(i) > d;
        tb->tasks[i].width = d;
        tb->usedWidth += d;
      }
    } else {
      // truncation is not needed anymore
      for (i = 0; i < tb->num_tasks; i++) {
        tb->tasks[i].truncated = false;
        tb->tasks[i].width = TASK_WIDTH(i);
      }
      tb->usedWidth = usedWidth;
    }
  }

  taskbar_update(tb);
}

void taskbar_draw(taskbar_t *tb) {
  uint8_t *raw;
  int len;

  if (tb->texture == NULL) {
    tb->texture = tb->wp->create_texture(tb->w, tb->width, tb->height);
  }

  raw = (uint8_t *)tb->surface->getbuffer(tb->surface->data, &len);
  tb->wp->update_texture(tb->w, tb->texture, raw);
  tb->wp->draw_texture(tb->w, tb->texture, tb->x, tb->y);
}

Int32 taskbar_clicked(taskbar_t *tb, int cx) {
  UInt16 i;
  Coord x;

  x = 0;
  for (i = 0; i < tb->num_tasks; i++) {
    if (cx >= x && cx < x + tb->tasks[i].width) {
      debug(DEBUG_TRACE, "taskbar", "clicked index=%d x=%d taskId=%d dbID=0x%08X name=[%s] width=%d", i, cx, tb->tasks[i].taskId, tb->tasks[i].dbID, tb->tasks[i].name, tb->tasks[i].width);
      return tb->tasks[i].taskId;
    }
    x += tb->tasks[i].width;
  }

  return -1;
}

Boolean taskbar_add_widget(taskbar_t *tb, Int32 taskId, UInt32 id, LocalID dbID, UInt16 bmpID) {
  DmOpenRef dbRef = NULL;
  MemHandle h;
  BitmapType *bmp;
  UInt32 size;
  Boolean r = false;

  if (tb->num_widgets < MAX_WIDGETS) {
    if (dbID == 0 || (dbRef = DmOpenDatabase(0, dbID, dmModeReadOnly)) != NULL) {
      if ((h = DmGetResource(bitmapRsc, bmpID)) != NULL) {
        if ((bmp = MemHandleLock(h)) != NULL) {
          size = MemHandleSize(h);
          if ((tb->widgets[tb->num_widgets].bmp = sys_calloc(1, size)) != NULL) {
            sys_memcpy(tb->widgets[tb->num_widgets].bmp, bmp, size);
            tb->widgets[tb->num_widgets].taskId = taskId;
            tb->widgets[tb->num_widgets].id = id;
            tb->num_widgets++;
            r = true;
          }
          MemHandleUnlock(h);
        }
        DmReleaseResource(h);
      }
      if (dbRef) DmCloseDatabase(dbRef);
    }
  }

  if (r) {
    tb->clockWidth += WIDGET_WIDTH;
    recalc_space(tb);
    taskbar_update(tb);
  }

  return r;
}

Boolean taskbar_remove_widget(taskbar_t *tb, UInt32 id) {
  UInt32 i;

  for (i = 0; i < tb->num_widgets; i++) {
    if (tb->widgets[i].id == id) {
      if (tb->widgets[i].bmp) sys_free(tb->widgets[i].bmp);
      tb->widgets[i].bmp = NULL;
      break;
    }
  }

  if (i == tb->num_widgets) return false;

  for (; i < tb->num_widgets - 1; i++) {
    tb->widgets[i] = tb->widgets[i+1];
  }

  tb->clockWidth -= WIDGET_WIDTH;
  tb->num_widgets--;
  taskbar_update(tb);

  return true;
}

Int32 taskbar_widget_clicked(taskbar_t *tb, int cx, UInt32 *id) {
  UInt16 i;
  Coord x;

  x = tb->width - tb->baseClockWidth - WIDGET_WIDTH;
  for (i = 0; i < tb->num_widgets; i++) {
    if (cx >= x && cx < x + WIDGET_WIDTH) {
      debug(DEBUG_INFO, "taskbar", "clicked widget index=%d x=%d id=%d", i, cx, tb->widgets[i].id);
      *id = tb->widgets[i].id;
      return tb->widgets[i].taskId;
    }
    x -= WIDGET_WIDTH;
  }

  return -1;
}
