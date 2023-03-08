#include <PalmOS.h>

#include "pumpkin.h"
#include "graphic.h"
#include "fill.h"
#include "color.h"
#include "ptr.h"
#include "xalloc.h"
#include "debug.h"

#define TAG_DB   "script_db"
#define TAG_RSRC "script_rsrc"
#define TAG_FORM "script_form"

#define MAX_TITLE 32

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

typedef struct {
  char *tag;
  FormType *frm;
} app_form_t;

typedef struct {
  UInt16 form;
  int pe, obj;
} script_data_t;

static Boolean ScriptFormHandleEvent(EventPtr event) {
  script_data_t *data = pumpkin_get_data();
  Boolean handled = false;

  switch (event->eType) {
    case ctlSelectEvent:
      data->obj = pumpkin_script_create_obj(data->pe, NULL);
      pumpkin_script_obj_iconst(data->pe, data->obj, "control", event->data.ctlSelect.controlID);
      handled = true;
      break;
    case menuEvent:
      data->obj = pumpkin_script_create_obj(data->pe, NULL);
      pumpkin_script_obj_iconst(data->pe, data->obj, "item", event->data.menu.itemID);
      handled = true;
      break;
    case appStopEvent:
      data->obj = pumpkin_script_create_obj(data->pe, NULL);
      break;
    default:
      break;
  }

  return handled;
}

static int app_script_ui_event(int pe) {
  script_data_t *data = pumpkin_get_data();
  script_int_t wait;
  EventType event;
  Err err;
  int obj, r = -1;

  if (script_get_integer(pe, 0, &wait) == 0) {
    EvtGetEvent(&event, wait);

    if (data->form) {
      if (!SysHandleEvent(&event)) {
        if (!MenuHandleEvent(NULL, &event, &err)) {
          data->obj = 0;
          FrmDispatchEvent(&event);
          if (data->obj > 0) {
            pumpkin_script_obj_iconst(pe, data->obj, "type", event.eType);
            r = script_push_object(pe, data->obj);
          }
        }
      }
    } else {
      SysHandleEvent(&event);

      obj = pumpkin_script_create_obj(pe, NULL);
      pumpkin_script_obj_iconst(pe, obj, "type", event.eType);

      switch (event.eType) {
        case keyDownEvent:
          pumpkin_script_obj_iconst(pe, obj, "key", event.data.keyDown.chr);
          pumpkin_script_obj_boolean(pe, obj, "cmd", (event.data.keyDown.modifiers & commandKeyMask) ? 1 : 0);
          break;
        case penDownEvent:
        case penMoveEvent:
          pumpkin_script_obj_iconst(pe, obj, "x", event.screenX);
          pumpkin_script_obj_iconst(pe, obj, "y", event.screenY);
          break;
        default:
          break;
      }

      r = script_push_object(pe, obj);
    }
  }

  return r;
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
  int r = 1;

  if (app_script_bitmap_dimensions(pe, &width, &height) == 0) {
    r = script_push_integer(pe, width);
  }

  return r;
}

static int app_script_bitmap_height(int pe) {
  Coord width, height;
  int r = -1;

  if (app_script_bitmap_dimensions(pe, &width, &height) == 0) {
    r = script_push_integer(pe, height);
  }

  return r;
}

static void form_destructor(void *p) {
  app_form_t *form;

  if (p) {
    form = (app_form_t *)p;
    if (form->frm) FrmDeleteForm(form->frm);
    xfree(form);
  }
}

static int app_script_ui_form(int pe) {
  script_int_t id, x, y, width, height;
  FormType *frm;
  app_form_t *form;
  char *title = NULL;
  int ptr, r = -1;

  if (script_get_integer(pe, 0, &id) == 0 &&
      script_get_string(pe, 1, &title) == 0 &&
      script_get_integer(pe, 2, &x) == 0 &&
      script_get_integer(pe, 3, &y) == 0 &&
      script_get_integer(pe, 4, &width) == 0 &&
      script_get_integer(pe, 5, &height) == 0) {

    if ((form = xcalloc(1, sizeof(app_form_t))) != NULL) {
      if ((frm = FrmNewForm(id, title, x, y, width, height, false, 0, 0, 0)) != NULL) {
        form->tag = TAG_FORM;
        form->frm = frm;
        ptr = ptr_new(form, form_destructor);
        r = script_push_integer(pe, ptr);
      } else {
        xfree(form);
      }
    }
  }

  if (title) xfree(title);

  return r;
}

static int app_script_ui_show(int pe) {
  script_data_t *data = pumpkin_get_data();
  app_form_t *form;
  int ptr, r = -1;

  if (script_get_integer(pe, 0, &ptr) == 0) {
    if ((form = ptr_lock(ptr, TAG_FORM)) != NULL) {
      data->form = form->frm->formId;
      FrmSetActiveForm(form->frm);
      FrmSetEventHandler(form->frm, ScriptFormHandleEvent);
      FrmDrawForm(form->frm);
      ptr_unlock(ptr, TAG_FORM);
      r = script_push_boolean(pe, 1);
    }
  }

  return r;
}

static int app_script_ui_title(int pe) {
  char *title = NULL;
  app_form_t *form;
  char *s;
  int ptr, len, r = -1;

  if (script_get_integer(pe, 0, &ptr) == 0 &&
      script_get_string(pe, 1, &title) == 0) {

    if (title[0]) {
      if ((form = ptr_lock(ptr, TAG_FORM)) != NULL) {
        len = StrLen(title);
        if ((s = MemPtrNew(len + 1)) != NULL) {
          StrNCopy(s, title, len);
          FrmSetTitle(form->frm, s);
          r = script_push_boolean(pe, 1);
        }
        ptr_unlock(ptr, TAG_FORM);
      }
    }
  }

  if (title) xfree(title);

  return r;
}

static int app_script_ui_control(int pe, ControlStyleType style, Coord dw, Coord dh) {
  script_int_t id, x, y;
  Coord width, height;
  FontID old, font;
  app_form_t *form;
  char *text = NULL;
  void *f;
  int ptr, r = -1;

  if (script_get_integer(pe, 0, &ptr) == 0 &&
      script_get_integer(pe, 1, &id) == 0 &&
      script_get_string(pe, 2, &text) == 0 &&
      script_get_integer(pe, 3, &x) == 0 &&
      script_get_integer(pe, 4, &y) == 0) {

    if (text[0]) {
      if ((form = ptr_lock(ptr, TAG_FORM)) != NULL) {
        font = stdFont;
        old = FntSetFont(font);
        width = FntCharsWidth(text, StrLen(text)) + dw;
        height = FntCharHeight() + dh;
        FntSetFont(old);

        f = form->frm;
        if (CtlNewControl(&f, id, style, text, x, y, width, height, font, 0, true) != NULL) {
          r = script_push_boolean(pe, 1);
        }
        ptr_unlock(ptr, TAG_FORM);
      }
    }
  }

  if (text) xfree(text);

  return r;
}

static int app_script_ui_button(int pe) {
  return app_script_ui_control(pe, buttonCtl, 4, 2);
}

static int app_script_ui_alert(int pe) {
  script_int_t id;
  char *msg = NULL;
  int r = -1;

  if (script_get_integer(pe, 0, &id) == 0 &&
      script_get_string(pe, 1, &msg) == 0) {

    if (msg[0]) {
       FrmCustomAlert(id, msg, NULL, NULL);
       r = script_push_boolean(pe, 1);
    }
  }

  if (msg) xfree(msg);

  return r;
}

int pumpkin_script_appenv(int pe) {
  int obj;

  pumpkin_script_global_iconst(pe, "nilEvent",  nilEvent);
  pumpkin_script_global_iconst(pe, "keyDown",   keyDownEvent);
  pumpkin_script_global_iconst(pe, "penDown",   penDownEvent);
  pumpkin_script_global_iconst(pe, "penMove",   penMoveEvent);
  pumpkin_script_global_iconst(pe, "menuEvent", menuEvent);
  pumpkin_script_global_iconst(pe, "ctlSelect", ctlSelectEvent);
  pumpkin_script_global_iconst(pe, "appStop",   appStopEvent);

  pumpkin_script_global_iconst(pe, "info",   10024);
  pumpkin_script_global_iconst(pe, "warn",   10031);
  pumpkin_script_global_iconst(pe, "error",  10021);

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

  if ((obj = pumpkin_script_create_obj(pe, "ui")) != -1) {
    pumpkin_script_obj_function(pe, obj, "event",    app_script_ui_event);
    pumpkin_script_obj_function(pe, obj, "form",     app_script_ui_form);
    pumpkin_script_obj_function(pe, obj, "show",     app_script_ui_show);
    pumpkin_script_obj_function(pe, obj, "title",    app_script_ui_title);
    pumpkin_script_obj_function(pe, obj, "button",   app_script_ui_button);
    pumpkin_script_obj_function(pe, obj, "alert",    app_script_ui_alert);
  }

  return 0;
}

uint32_t pumpkin_script_main(uint16_t code, void *param, uint16_t flags) {
  script_data_t data;
  MemHandle h;
  UInt16 id;
  char msg[256];
  int pe;

  if ((pe = pumpkin_script_create()) > 0) {
    pumpkin_script_appenv(pe);
    MemSet(&data, sizeof(data), 0);
    data.pe = pe;
    pumpkin_set_data(&data);

    for (id = 1; id < 32768; id++) {
      if ((h = DmGet1Resource(sysRsrcTypeScript, id)) == NULL) break;
      DmReleaseResource(h);
      if (pumpkin_script_init(pe, sysRsrcTypeScript, id) != 0) {
        if (pumpkin_script_get_last_error(pe, msg, sizeof(msg)) == 0) {
          SysFatalAlert(msg);
        } else {
          SysFatalAlert("Script error.");
        }
        break;
      }
    }

    pumpkin_script_destroy(pe);
  }

  return 0;
}
