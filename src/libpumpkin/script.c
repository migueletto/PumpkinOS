#include <PalmOS.h>

#include "pumpkin.h"
#include "graphic.h"
#include "fill.h"
#include "color.h"
#include "ptr.h"
#include "xalloc.h"
#include "debug.h"

#define TAG_DB   "cmd_db"
#define TAG_RSRC "cmd_rsrc"

typedef struct {
  char *tag;
  DmOpenRef dbRef;
} app_db_t;

typedef struct {
  char *tag;
  UInt32 type;
  UInt16 id;
  MemHandle h;
} app_resource_t;

static int app_script_event(int pe) {
  script_int_t wait;
  EventType event;
  Err err;

  event.eType = nilEvent;

  if (script_get_integer(pe, 0, &wait) == 0) {
    EvtGetEvent(&event, wait);
    if (!SysHandleEvent(&event)) {
      if (!MenuHandleEvent(NULL, &event, &err)) {
        FrmDispatchEvent(&event);
      }
    }
  }

  return script_push_integer(pe, event.eType);
}

static int app_script_screen_rgb(int pe) {
  script_int_t red, green, blue;
  RGBColorType rgb;
  UInt32 c;
  int r = -1;

  if (script_get_integer(pe, 0, &red) == 0 &&
      script_get_integer(pe, 1, &green) == 0 &&
      script_get_integer(pe, 2, &blue) == 0) {

    rgb.r = red;
    rgb.g = green;
    rgb.b = blue;
    c = RGBToLong(&rgb);
    r = script_push_integer(pe, c);
  }

  return r;
}

static int app_script_screen_width(int pe) {
  UInt32 width;

  WinScreenMode(winScreenModeGet, &width, NULL, NULL, NULL);

  return script_push_integer(pe, width);
}

static int app_script_screen_height(int pe) {
  UInt32 height;

  WinScreenMode(winScreenModeGet, NULL, &height, NULL, NULL);

  return script_push_integer(pe, height);
}

static int app_script_setpixel(int pe) {
  script_int_t x, y, c;
  RGBColorType rgb, old;
  int r = -1;

  if (script_get_integer(pe, 0, &x) == 0 &&
      script_get_integer(pe, 1, &y) == 0 &&
      script_get_integer(pe, 2, &c) == 0) {

    LongToRGB(c, &rgb);
    WinSetForeColorRGB(&rgb, &old);
    WinPaintPixel(x, y);
    WinSetForeColorRGB(&old, NULL);
    r = 0;
  }

  return r;
}

static int app_script_clear(int pe) {
  script_int_t c;
  RGBColorType rgb, old;
  int r = -1;

  if (script_get_integer(pe, 0, &c) == 0) {
    LongToRGB(c, &rgb);
    WinSetBackColorRGB(&rgb, &old);
    WinEraseWindow();
    WinSetBackColorRGB(&old, NULL);
    r =  0;
  }

  return r;
}

static int app_script_draw_text(int pe) {
  script_int_t x, y, fg, bg;
  char *s = NULL;
  RGBColorType rgb, oldt, oldb;
  int r = -1;

  if (script_get_string(pe, 0, &s) == 0 &&
      script_get_integer(pe, 1, &x) == 0 &&
      script_get_integer(pe, 2, &y) == 0 &&
      script_get_integer(pe, 3, &fg) == 0 &&
      script_get_integer(pe, 4, &bg) == 0) {

    LongToRGB(fg, &rgb);
    WinSetTextColorRGB(&rgb, &oldt);
    LongToRGB(bg, &rgb);
    WinSetBackColorRGB(&rgb, &oldb);
    WinPaintChars(s, StrLen(s), x, y);
    WinSetBackColorRGB(&oldb, NULL);
    WinSetTextColorRGB(&oldt, NULL);

    r = 0;
  }

  if (s) xfree(s);

  return r;
}

static int app_script_draw_line(int pe) {
  script_int_t x1, y1, x2, y2, c;
  RGBColorType rgb, old;
  int r = -1;

  if (script_get_integer(pe, 0, &x1) == 0 &&
      script_get_integer(pe, 1, &y1) == 0 &&
      script_get_integer(pe, 2, &x2) == 0 &&
      script_get_integer(pe, 3, &y2) == 0 &&
      script_get_integer(pe, 4, &c) == 0) {

    LongToRGB(c, &rgb);
    WinSetForeColorRGB(&rgb, &old);
    WinPaintLine(x1, y1, x2, y2);
    WinSetForeColorRGB(&old, NULL);
    r = 0;
  }

  return r;
}

static int app_script_draw_rect(int pe) {
  script_int_t x, y, w, h, c;
  RectangleType rect;
  RGBColorType rgb, old;
  int filled, r = -1;

  if (script_get_integer(pe, 0, &x) == 0 &&
      script_get_integer(pe, 1, &y) == 0 &&
      script_get_integer(pe, 2, &w) == 0 &&
      script_get_integer(pe, 3, &h) == 0 &&
      script_get_boolean(pe, 4, &filled) == 0 &&
      script_get_integer(pe, 5, &c) == 0) {

    LongToRGB(c, &rgb);
    WinSetForeColorRGB(&rgb, &old);
    if (filled) {
      RctSetRectangle(&rect, x, y, w, h);
      WinPaintRectangle(&rect, 0);
    } else {
      WinPaintLine(x, y, x+w-1, y);
      WinPaintLine(x+w-1, y, x+w-1, y+h-1);
      WinPaintLine(x+w-1, y+h-1, x, y+h-1);
      WinPaintLine(x, y+h-1, x, y);
    }
    WinSetForeColorRGB(&old, NULL);
    r = 0;
  }

  return r;
}

static void app_graphic_setpixel(void *data, int x, int y, uint32_t color) {
  RGBColorType rgb, old;

  LongToRGB(color, &rgb);
  WinSetForeColorRGB(&rgb, &old);
  WinPaintPixel(x, y);
  WinSetForeColorRGB(&old, NULL);
}

static int app_script_draw_circle(int pe) {
  script_int_t x, y, rx, ry, c;
  int filled, r = -1;

  if (script_get_integer(pe, 0, &x) == 0 &&
      script_get_integer(pe, 1, &y) == 0 &&
      script_get_integer(pe, 2, &rx) == 0 &&
      script_get_integer(pe, 3, &ry) == 0 &&
      script_get_boolean(pe, 4, &filled) == 0 &&
      script_get_integer(pe, 5, &c) == 0) {

    graphic_ellipse(NULL, x, y, rx, ry, filled, c, app_graphic_setpixel, NULL);
    r = 0;
  }

  return r;
}

static int app_script_fill(int pe) {
  script_int_t x, y, c;
  RectangleType rect;
  RGBColorType rgb, old;
  int r = -1;

  if (script_get_integer(pe, 0, &x) == 0 &&
      script_get_integer(pe, 1, &y) == 0 &&
      script_get_integer(pe, 2, &c) == 0) {

    LongToRGB(c, &rgb);
    WinSetForeColorRGB(&rgb, &old);
    RctSetRectangle(&rect, 0, 0, 0, 0);
    AppSeedFill(x, y, &rect, &rgb);
    WinSetForeColorRGB(NULL, &old);
    r = 0;
  }

  return r;
}

static void db_destructor(void *p) {
  app_db_t *db;

  if (p) {
    db = (app_db_t *)p;
    if (db->dbRef) DmCloseDatabase(db->dbRef);
    xfree(db);
  }
}

static int app_script_open_db(int pe) {
  app_db_t *db;
  LocalID dbID;
  char *dbname = NULL;
  int ptr, r = -1;

  if (script_get_string(pe, 0, &dbname) == 0) {
    if ((db = xcalloc(1, sizeof(app_db_t))) != NULL) {
      if ((dbID = DmFindDatabase(0, dbname)) != 0) {
        if ((db->dbRef = DmOpenDatabase(0, dbID, dmModeReadOnly)) != NULL) {
          db->tag = TAG_DB;
          ptr = ptr_new(db, db_destructor);
          r = script_push_integer(pe, ptr);
        } else {
          xfree(db);
        }
      } else {
        xfree(db);
      }
    }
  }

  if (dbname) xfree(dbname);

  return r;
}

static int app_script_close_db(int pe) {
  script_int_t ptr;
  int r = -1;

  if (script_get_integer(pe, 0, &ptr) == 0) {
    r = ptr_free(ptr, TAG_DB);
  }

  return r;
}

static void rsrc_destructor(void *p) {
  app_resource_t *rsrc;

  if (p) {
    rsrc = (app_resource_t *)p;
    if (rsrc->h) DmReleaseResource(rsrc->h);
    xfree(rsrc);
  }
}

static int app_script_get_rsrc(int pe, Boolean one) {
  UInt32 ptype;
  script_int_t id;
  app_resource_t *rsrc;
  char *type = NULL;
  int ptr, r = -1;

  if (script_get_string(pe, 0, &type) == 0 &&
      script_get_integer(pe, 1, &id) == 0) {

    if ((rsrc = xcalloc(1, sizeof(app_resource_t))) != NULL) {
      pumpkin_s2id(&ptype, type);
      rsrc->h = one ? DmGet1Resource(ptype, id) : DmGetResource(ptype, id);
      if (rsrc->h != NULL) {
        rsrc->tag = TAG_RSRC;
        rsrc->type = ptype;
        rsrc->id = id;
        ptr = ptr_new(rsrc, rsrc_destructor);
        r = script_push_integer(pe, ptr);
      } else {
        xfree(rsrc);
      }
    }
  }

  if (type) xfree(type);

  return r;
}

static int app_script_get_resource(int pe) {
  return app_script_get_rsrc(pe, false);
}

static int app_script_get1_resource(int pe) {
  return app_script_get_rsrc(pe, true);
}

static int app_script_release_resource(int pe) {
  script_int_t ptr;
  int r = -1;

  if (script_get_integer(pe, 0, &ptr) == 0) {
    r = ptr_free(ptr, TAG_RSRC);
  }

  return r;
}

static int app_script_bitmap_draw(int pe) {
  BitmapType *bmp;
  app_resource_t *rsrc;
  script_int_t ptr, x, y;
  int r = -1;

  if (script_get_integer(pe, 0, &ptr) == 0 &&
      script_get_integer(pe, 1, &x) == 0 &&
      script_get_integer(pe, 2, &y) == 0) {

    if ((rsrc = ptr_lock(ptr, TAG_RSRC)) != NULL) {
      if (rsrc->type == bitmapRsc || rsrc->type == iconType) {
        if (rsrc->h && (bmp = MemHandleLock(rsrc->h)) != NULL) {
          WinPaintBitmap(bmp, x, y);
          MemHandleUnlock(rsrc->h);
          r = 0;
        }
      }
      ptr_unlock(ptr, TAG_RSRC);
    }
  }

  return r;
}

static int app_script_bitmap_dimensions(int pe, Coord *width, Coord *height) {
  BitmapType *bmp;
  app_resource_t *rsrc;
  script_int_t ptr;
  int r = -1;

  if (script_get_integer(pe, 0, &ptr) == 0) {
    if ((rsrc = ptr_lock(ptr, TAG_RSRC)) != NULL) {
      if (rsrc->type == bitmapRsc || rsrc->type == iconType) {
        if (rsrc->h && (bmp = MemHandleLock(rsrc->h)) != NULL) {
          BmpGetDimensions(bmp, width, height, NULL);
          MemHandleUnlock(rsrc->h);
          r = 0;
        }
      }
      ptr_unlock(ptr, TAG_RSRC);
    }
  }

  return r;
}

static int app_script_bitmap_width(int pe) {
  Coord width, height;
  int r;

  if ((r = app_script_bitmap_dimensions(pe, &width, &height)) == 0) {
    r = script_push_integer(pe, width);
  }

  return r;
}

static int app_script_bitmap_height(int pe) {
  Coord width, height;
  int r;

  if ((r = app_script_bitmap_dimensions(pe, &width, &height)) == 0) {
    r = script_push_integer(pe, height);
  }

  return r;
}

int pumpkin_script_appenv(int pe) {
  int obj;

  pumpkin_script_global_function(pe, "event", app_script_event);
  pumpkin_script_global_iconst(pe, "keyDown", keyDownEvent);
  pumpkin_script_global_iconst(pe, "appStop", appStopEvent);

  if ((obj = pumpkin_script_create_obj(pe, "screen")) != -1) {
    pumpkin_script_obj_function(pe, obj, "rgb",      app_script_screen_rgb);
    pumpkin_script_obj_function(pe, obj, "width",    app_script_screen_width);
    pumpkin_script_obj_function(pe, obj, "height",   app_script_screen_height);
    pumpkin_script_obj_function(pe, obj, "setpixel", app_script_setpixel);
    pumpkin_script_obj_function(pe, obj, "clear",    app_script_clear);
    pumpkin_script_obj_function(pe, obj, "draw",     app_script_draw_text);
    pumpkin_script_obj_function(pe, obj, "line",     app_script_draw_line);
    pumpkin_script_obj_function(pe, obj, "rect",     app_script_draw_rect);
    pumpkin_script_obj_function(pe, obj, "circle",   app_script_draw_circle);
    pumpkin_script_obj_function(pe, obj, "fill",     app_script_fill);
  }

  if ((obj = pumpkin_script_create_obj(pe, "db")) != -1) {
    pumpkin_script_obj_function(pe, obj, "open",     app_script_open_db);
    pumpkin_script_obj_function(pe, obj, "close",    app_script_close_db);
  }

  if ((obj = pumpkin_script_create_obj(pe, "rsrc")) != -1) {
    pumpkin_script_obj_function(pe, obj, "get",      app_script_get_resource);
    pumpkin_script_obj_function(pe, obj, "get1",     app_script_get1_resource);
    pumpkin_script_obj_function(pe, obj, "release",  app_script_release_resource);
  }

  if ((obj = pumpkin_script_create_obj(pe, "bitmap")) != -1) {
    pumpkin_script_obj_function(pe, obj, "draw",     app_script_bitmap_draw);
    pumpkin_script_obj_function(pe, obj, "width",    app_script_bitmap_width);
    pumpkin_script_obj_function(pe, obj, "height",   app_script_bitmap_height);
  }

  return 0;
}

uint32_t pumpkin_script_main(uint16_t code, void *param, uint16_t flags) {
  MemHandle h;
  int32_t r;
  int pe;
  char *buf;

  if ((pe = pumpkin_script_create()) > 0) {
    if ((h = DmGet1Resource(sysRsrcTypeScript, 1)) != NULL) {
      if ((buf = MemHandleLock(h)) != NULL) {
        debug(DEBUG_INFO, PUMPKINOS, "running app script");
        pumpkin_script_appenv(pe);
        pumpkin_script_run(pe, buf, &r);
        debug(DEBUG_INFO, PUMPKINOS, "app script returned %d", r);
        MemHandleUnlock(h);
      }
      DmReleaseResource(h);
    }

    pumpkin_script_destroy(pe);
  }

  return 0;
}
