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

static int app_script_ui_menu(int pe) {
  script_int_t pos, id, cmd;
  char *text = NULL;
  int r = -1;

  if (script_get_integer(pe, 0, &pos) == 0 &&
      script_get_integer(pe, 1, &id) == 0 &&
      script_get_integer(pe, 2, &cmd) == 0 &&
      script_get_string(pe, 3, &text) == 0) {

    if (MenuAddItem(pos, id, cmd, text) == errNone) {
      r = script_push_boolean(pe, 1);
    }
  }

  if (text) xfree(text);

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
      if ((frm = FrmNewForm(id, title, x, y, width, height, false, 0, 0, 1001)) != NULL) {
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
  script_int_t ptr;
  app_form_t *form;
  int r = -1;

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
  script_int_t ptr;
  char *title = NULL;
  app_form_t *form;
  char *s;
  int len, r = -1;

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

static int app_script_ui_getlabel(int pe) {
  script_int_t ptr, id;
  app_form_t *form;
  ControlType *ctl;
  UInt16 index;
  char *s;
  int r = -1;

  if (script_get_integer(pe, 0, &ptr) == 0 &&
      script_get_integer(pe, 1, &id) == 0) {

    if ((form = ptr_lock(ptr, TAG_FORM)) != NULL) {
      if ((index = FrmGetObjectIndex(form->frm, id)) != 0xffff) {
        if (FrmGetObjectType(form->frm, index) == frmControlObj) {
          if ((ctl = (ControlType *)FrmGetObjectPtr(form->frm, index)) != NULL) {
            if ((s = (char *)CtlGetLabel(ctl)) != NULL) {
              r = script_push_string(pe, s);
            }
          }
        }
      }
      ptr_unlock(ptr, TAG_FORM);
    }
  }

  return r;
}

static int app_script_ui_getvalue(int pe) {
  script_int_t ptr, id;
  app_form_t *form;
  ControlType *ctl;
  UInt16 index;
  int r = -1;

  if (script_get_integer(pe, 0, &ptr) == 0 &&
      script_get_integer(pe, 1, &id) == 0) {

    if ((form = ptr_lock(ptr, TAG_FORM)) != NULL) {
      if ((index = FrmGetObjectIndex(form->frm, id)) != 0xffff) {
        if (FrmGetObjectType(form->frm, index) == frmControlObj) {
          if ((ctl = (ControlType *)FrmGetObjectPtr(form->frm, index)) != NULL) {
            r = script_push_integer(pe, CtlGetValue(ctl));
          }
        }
      }
      ptr_unlock(ptr, TAG_FORM);
    }
  }

  return r;
}

static int app_script_ui_bounds(int pe) {
  script_int_t ptr, id;
  script_ref_t obj;
  app_form_t *form;
  RectangleType rect;
  FontID old;
  UInt16 index;
  char *s;
  int r = -1;

  if (script_get_integer(pe, 0, &ptr) == 0 &&
      script_get_integer(pe, 1, &id) == 0 &&
      script_get_object(pe, 2, &obj) == 0) {

    if ((form = ptr_lock(ptr, TAG_FORM)) != NULL) {
      if ((index = FrmGetObjectIndex(form->frm, id)) != 0xffff) {
        rect.extent.x = 0;
        rect.extent.y = 0;
        if (FrmGetObjectType(form->frm, index) == frmLabelObj) {
          // labels only have their size computed when the form is drawn,
          // so we have to compute it here
          if ((s = (char *)FrmGetLabel(form->frm, id)) != NULL) {
            old = FntSetFont(boldFont);
            rect.extent.x = FntCharsWidth(s, StrLen(s));
            rect.extent.y = FntCharHeight();
            FntSetFont(old);
          }
        } else {
          FrmGetObjectBounds(form->frm, index, &rect);
        }
        pumpkin_script_obj_iconst(pe, obj, "width", rect.extent.x);
        pumpkin_script_obj_iconst(pe, obj, "height", rect.extent.y);
        r = script_push_boolean(pe, 1);
      }
      ptr_unlock(ptr, TAG_FORM);
    }
  }

  return r;
}

static int app_script_ui_label(int pe) {
  script_int_t ptr, id, x, y, font;
  app_form_t *form;
  char *text = NULL;
  int r = -1;

  if (script_get_integer(pe, 0, &ptr) == 0 &&
      script_get_integer(pe, 1, &id) == 0 &&
      script_get_string(pe,  2, &text) == 0 &&
      script_get_integer(pe, 3, &x) == 0 &&
      script_get_integer(pe, 4, &y) == 0 &&
      script_get_integer(pe, 5, &font) == 0) {

    if (text[0]) {
      if ((form = ptr_lock(ptr, TAG_FORM)) != NULL) {
        if (FrmNewLabel(&form->frm, id, text, x, y, font) != NULL) {
          r = script_push_boolean(pe, 1);
        }
        ptr_unlock(ptr, TAG_FORM);
      }
    }
  }

  if (text) xfree(text);

  return r;
}

static int app_script_ui_control(int pe, ControlStyleType style, Coord dw, Coord dh) {
  script_int_t ptr, id, x, y, font, group, min, max, page;
  Coord width, height;
  UInt16 minValue, maxValue, pageSize;
  FontID old;
  ControlType *ctl;
  app_form_t *form;
  char *text = NULL;
  void *f;
  int selected, r = -1;

  if (script_get_integer(pe, 0, &ptr) == 0 &&
      script_get_integer(pe, 1, &id) == 0 &&
      script_get_string(pe,  2, &text) == 0 &&
      script_get_integer(pe, 3, &x) == 0 &&
      script_get_integer(pe, 4, &y) == 0 &&
      script_get_integer(pe, 5, &font) == 0) {

    script_opt_integer(pe, 6, &group);
    script_opt_boolean(pe, 7, &selected);

    if (style == sliderCtl || style == feedbackSliderCtl) {
      script_opt_integer(pe, 8, &min);
      script_opt_integer(pe, 9, &max);
      script_opt_integer(pe, 10, &page);
    }

    if (text[0]) {
      if ((form = ptr_lock(ptr, TAG_FORM)) != NULL) {
        old = FntSetFont(font);
        width = FntCharsWidth(text, StrLen(text)) + dw;
        height = FntCharHeight() + dh;
        FntSetFont(old);

        f = form->frm;
        if ((ctl = CtlNewControl(&f, id, style, text, x, y, width, height, font, group, true)) != NULL) {
          if (style == pushButtonCtl || style == checkboxCtl) {
            CtlSetValue(ctl, selected ? 1 : 0);
          } else if (style == sliderCtl || style == feedbackSliderCtl) {
            minValue = min;
            maxValue = max;
            pageSize = page;
            CtlSetSliderValues(ctl, &minValue, &maxValue, &pageSize, NULL);
          }
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
  return app_script_ui_control(pe, buttonCtl, 6, 2);
}

static int app_script_ui_rbutton(int pe) {
  return app_script_ui_control(pe, repeatingButtonCtl, 6, 2);
}

static int app_script_ui_pushbutton(int pe) {
  return app_script_ui_control(pe, pushButtonCtl, 6, 2);
}

static int app_script_ui_checkbox(int pe) {
  return app_script_ui_control(pe, checkboxCtl, 10, 0);
}

static int app_script_ui_slider(int pe) {
  return app_script_ui_control(pe, sliderCtl, 2, 0);
}

static int app_script_ui_fslider(int pe) {
  return app_script_ui_control(pe, feedbackSliderCtl, 2, 0);
}

static int app_script_ui_selector(int pe) {
  return app_script_ui_control(pe, selectorTriggerCtl, 6, 2);
}

static int app_script_ui_field(int pe) {
  script_int_t ptr, id, x, y, cols, rows, max, font;
  Coord width, height;
  FontID old;
  app_form_t *form;
  void *f;
  int r = -1;

  if (script_get_integer(pe, 0, &ptr) == 0 &&
      script_get_integer(pe, 1, &id) == 0 &&
      script_get_integer(pe, 2, &x) == 0 &&
      script_get_integer(pe, 3, &y) == 0 &&
      script_get_integer(pe, 4, &cols) == 0 &&
      script_get_integer(pe, 5, &rows) == 0 &&
      script_get_integer(pe, 6, &max) == 0 &&
      script_get_integer(pe, 7, &font) == 0) {

    if ((form = ptr_lock(ptr, TAG_FORM)) != NULL) {
      old = FntSetFont(font);
      width = cols * FntCharWidth('A');
      height = rows * FntCharHeight();
      FntSetFont(old);

      f = form->frm;
      if (FldNewField(&f, id, x, y, width, height, font, max, true, true,
            rows == 1, false, leftAlign, true, true, false) != NULL) {
        r = script_push_boolean(pe, 1);
      }
      ptr_unlock(ptr, TAG_FORM);
    }
  }

  return r;
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

static int app_script_ui_about(int pe) {
  char *descr = NULL;
  int r = -1;

  if (script_get_string(pe, 0, &descr) == 0) {
    AbtShowAboutEx(pumpkin_get_app_creator(), aboutDialog+2, descr);
  }

  if (descr) xfree(descr);

  return r;
}

int pumpkin_script_appenv(int pe) {
  int obj;

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
    pumpkin_script_obj_function(pe, obj, "menu",     app_script_ui_menu);
    pumpkin_script_obj_function(pe, obj, "form",     app_script_ui_form);
    pumpkin_script_obj_function(pe, obj, "show",     app_script_ui_show);
    pumpkin_script_obj_function(pe, obj, "title",    app_script_ui_title);
    pumpkin_script_obj_function(pe, obj, "getlabel", app_script_ui_getlabel);
    pumpkin_script_obj_function(pe, obj, "getvalue", app_script_ui_getvalue);
    pumpkin_script_obj_function(pe, obj, "bounds",   app_script_ui_bounds);
    pumpkin_script_obj_function(pe, obj, "label",    app_script_ui_label);
    pumpkin_script_obj_function(pe, obj, "button",   app_script_ui_button);
    pumpkin_script_obj_function(pe, obj, "rbutton",  app_script_ui_rbutton);
    pumpkin_script_obj_function(pe, obj, "pushbutton", app_script_ui_pushbutton);
    pumpkin_script_obj_function(pe, obj, "checkbox", app_script_ui_checkbox);
    pumpkin_script_obj_function(pe, obj, "slider",   app_script_ui_slider);
    pumpkin_script_obj_function(pe, obj, "fslider",  app_script_ui_fslider);
    pumpkin_script_obj_function(pe, obj, "selector", app_script_ui_selector);
    pumpkin_script_obj_function(pe, obj, "field",    app_script_ui_field);
    pumpkin_script_obj_function(pe, obj, "alert",    app_script_ui_alert);
    pumpkin_script_obj_function(pe, obj, "about",    app_script_ui_about);
  }

  if ((obj = pumpkin_script_create_obj(pe, "event")) != -1) {
    pumpkin_script_obj_iconst(pe, obj, "nilEvent",  nilEvent);
    pumpkin_script_obj_iconst(pe, obj, "keyDown",   keyDownEvent);
    pumpkin_script_obj_iconst(pe, obj, "penDown",   penDownEvent);
    pumpkin_script_obj_iconst(pe, obj, "penMove",   penMoveEvent);
    pumpkin_script_obj_iconst(pe, obj, "menuEvent", menuEvent);
    pumpkin_script_obj_iconst(pe, obj, "ctlSelect", ctlSelectEvent);
    pumpkin_script_obj_iconst(pe, obj, "appStop",   appStopEvent);
  }

  if ((obj = pumpkin_script_create_obj(pe, "alert")) != -1) {
    pumpkin_script_obj_iconst(pe, obj, "info",   10024);
    pumpkin_script_obj_iconst(pe, obj, "warn",   10031);
    pumpkin_script_obj_iconst(pe, obj, "error",  10021);
  }

  if ((obj = pumpkin_script_create_obj(pe, "font")) != -1) {
    pumpkin_script_obj_iconst(pe, obj, "std",       stdFont);
    pumpkin_script_obj_iconst(pe, obj, "bold",      boldFont);
    pumpkin_script_obj_iconst(pe, obj, "large",     largeFont);
    pumpkin_script_obj_iconst(pe, obj, "symbol",    symbolFont);
    pumpkin_script_obj_iconst(pe, obj, "symbol",    symbolFont);
    pumpkin_script_obj_iconst(pe, obj, "symbol11",  symbol11Font);
    pumpkin_script_obj_iconst(pe, obj, "symbol7",   symbol7Font);
    pumpkin_script_obj_iconst(pe, obj, "led",       ledFont);
    pumpkin_script_obj_iconst(pe, obj, "largeBold", largeBoldFont);
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
