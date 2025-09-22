#include <PalmOS.h>

#include "pumpkin.h"
#include "pwindow.h"
#include "surface.h"
#include "taskbar.h"
#include "debug.h"

#define MAX_TASKS 32

#define BACK_COLOR   223
#define FORE_COLOR   255
#define TEXT_COLOR   255
#define BORDER_COLOR 221
#define CLOCK_COLOR  222

typedef struct {
  UInt32 taskId;
  LocalID dbID;
  UInt32 creator;
  char *name;
  BitmapType *bmp;
  UInt32 bmp_size;
  UInt32 width;
} taskbar_task_t;

struct taskbar_t {
  window_provider_t *wp;
  window_t *w;
  surface_t *surface;
  texture_t *texture;
  WinHandle wh;
  BitmapType *bmp;
  UInt16 density;
  UInt32 width, height, usedWidth, clockWidth;
  UInt32 x, y;
  taskbar_task_t tasks[MAX_TASKS];
  UInt32 num_tasks;
};

static void tasks_width(taskbar_t *tb) {
  UInt32 i;
  UInt16 prevCoordSys;
  BitmapType *bmp;
  Coord width;

  prevCoordSys = WinSetCoordinateSystem(tb->density == kDensityDouble ? kCoordinatesDouble : kCoordinatesStandard);
  tb->usedWidth = 0;
  for (i = 0; i < tb->num_tasks; i++) {
    if (tb->tasks[i].width == 0) {
      if (tb->tasks[i].bmp) {
        if ((bmp = BmpGetBestBitmapEx(tb->tasks[i].bmp, tb->density, BmpGetBitDepth(tb->tasks[i].bmp), false)) != NULL) {
          BmpGetDimensions(bmp, &width, NULL, NULL);
          tb->tasks[i].width += width + 2;
        }
      }
      tb->tasks[i].width += FntCharsWidth(tb->tasks[i].name, StrLen(tb->tasks[i].name)) + 6;
    }
    tb->usedWidth += tb->tasks[i].width;
  }
  WinSetCoordinateSystem(prevCoordSys);
}

static void draw_separator(taskbar_t *tb, Coord x) {
  WinSetForeColor(BORDER_COLOR);
  WinPaintLine(x, 2, x, tb->height - 3);
  WinPaintLine(x+1, 2, x+1, tb->height - 3);
}

void taskbar_update(taskbar_t *tb) {
  WinHandle oldDrawWindow;
  IndexedColorType oldBackColor, oldForeColor, oldTextColor;
  PatternType oldPattern;
  RectangleType rect;
  FontID oldFont;
  DateTimeType dt;
  BitmapType *bmp;
  UInt16 prevCoordSys, density, len, i;
  UInt32 t, tf;
  Coord x0, x, y, width, height;
  char clock[32];

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
    x0 = x;
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
        y = tb->height > height ? (tb->height - height) / 2 : 0;
        WinPaintBitmapEx(bmp, x, y, false);
        x += width + 2;
      }
    }
    height = FntCharHeight();
    len = StrLen(tb->tasks[i].name);
    y = tb->height > height ? (tb->height - height) / 2 : 0;
    WinSetForeColor(TEXT_COLOR);
    WinPaintChars(tb->tasks[i].name, len, x, y);
    x += FntCharsWidth(tb->tasks[i].name, len) + 2;
    draw_separator(tb, x);
    x += 4;
    x = x0 + tb->tasks[i].width;
  }

  WinSetBackColor(CLOCK_COLOR);
  RctSetRectangle(&rect, tb->width - tb->clockWidth, 0, tb->clockWidth, tb->height);
  WinEraseRectangle(&rect, 0);

  t = TimGetSeconds();
  TimSecondsToDateTime(t, &dt);
  tf = PrefGetPreference(prefTimeFormat);
  TimeToAscii(dt.hour, dt.minute, tf, clock);
  len = StrLen(clock);
  x = FntCharsWidth(clock, len);
  WinSetForeColor(TEXT_COLOR);
  WinPaintChars(clock, len, tb->width - (tb->clockWidth + x) / 2, 0);

  FntSetFont(oldFont);
  WinSetPatternType(oldPattern);
  WinSetBackColor(oldBackColor);
  WinSetForeColor(oldForeColor);
  WinSetTextColor(oldTextColor);
  WinSetDrawWindow(oldDrawWindow);
  WinSetCoordinateSystem(prevCoordSys);

  BmpDrawSurface(tb->bmp, 0, 0, tb->width, tb->height, tb->surface, 0, 0, true);
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
        tb->clockWidth = FntCharsWidth(buf, StrLen(buf)) + 2;
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
    if (tb->wh) WinDeleteWindow(tb->wh, false);
    if (tb->surface) surface_destroy(tb->surface);
    sys_free(tb);
  }
}

void taskbar_add(taskbar_t *tb, UInt32 taskId, LocalID dbID, UInt32 creator, char *name) {
  DmOpenRef dbRef;
  MemHandle h;
  BitmapType *bmp;
  UInt32 i, d, size, oldWidth;

  if (tb->num_tasks < MAX_TASKS) {
    tb->tasks[tb->num_tasks].taskId = taskId;
    tb->tasks[tb->num_tasks].dbID = dbID;
    tb->tasks[tb->num_tasks].creator = creator;
    tb->tasks[tb->num_tasks].name = sys_strdup(name);

    if ((dbRef = DmOpenDatabase(0, dbID, dmModeReadOnly)) != NULL) {
      if ((h = DmGetResource(iconType, 1001)) != NULL) {
        if ((bmp = MemHandleLock(h)) != NULL) {
          size = MemHandleSize(h);
          if ((tb->tasks[tb->num_tasks].bmp = sys_calloc(1, size)) != NULL) {
            sys_memcpy(tb->tasks[tb->num_tasks].bmp, bmp, size);
            tb->tasks[tb->num_tasks].bmp_size = size;
          }
          MemHandleUnlock(h);
        }
        DmReleaseResource(h);
      }
      DmCloseDatabase(dbRef);
    }

    oldWidth = tb->usedWidth;
    tb->num_tasks++;
    tasks_width(tb);
    debug(DEBUG_TRACE, "taskbar", "add index=%d taskId=%d dbID=0x%08X name=[%s] width=%d pos=%d", tb->num_tasks-1, taskId, dbID, name, tb->tasks[tb->num_tasks-1].width, oldWidth);
    if (tb->usedWidth > (tb->width - tb->clockWidth)) {
      d = (tb->usedWidth - (tb->width - tb->clockWidth)) / tb->num_tasks;
      for (i = 0; i < tb->num_tasks; i++) {
        tb->tasks[i].width -= d;
      }
      tasks_width(tb);
    }

    taskbar_update(tb);
  }
}

void taskbar_remove(taskbar_t *tb, LocalID dbID) {
  UInt16 i;

  for (i = 0; i < tb->num_tasks; i++) {
    if (tb->tasks[i].dbID == dbID) {
      if (tb->tasks[i].name) sys_free(tb->tasks[i].name);
      if (tb->tasks[i].bmp) sys_free(tb->tasks[i].bmp);
      tb->tasks[i].name = NULL;
      tb->tasks[i].bmp = NULL;
      break;
    }
  }

  if (i < tb->num_tasks) {
    for (; i < tb->num_tasks - 1; i++) {
      tb->tasks[i] = tb->tasks[i+1];
    }
    tb->num_tasks--;
  }

  tasks_width(tb);
  if (tb->usedWidth < (tb->width - tb->clockWidth)) {
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

UInt32 taskbar_clicked(taskbar_t *tb, int cx, int down) {
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

  return 0;
}
