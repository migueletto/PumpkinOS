#include <PalmOS.h>

#include "sys.h"
#include "resource.h"
#include "resedit.h"
#include "pumpkin.h"
#include "xalloc.h"
#include "debug.h"

#define MAX_TITLE 64

typedef struct {
  FormType *frm;
  MemHandle handle;
  FormType *formP;
  UInt32 size;
  void *rsrc;
  PointType topLeft;
  RectangleType *bounds;
  char **itemText;
  Int16 selected;
  char *rtitle;
  char title[MAX_TITLE];
  Boolean changed, stop;
} form_edit_t;

static const RGBColorType white  = { 0, 0xff, 0xff, 0xff };
static const RGBColorType gray   = { 0, 0xe0, 0xe0, 0xe0 };
static const RGBColorType red    = { 0, 0xff, 0x80, 0x80 };

static const dynamic_form_item_t formPropertiesItems[] = {
  { "Position:", numPairItem, 0 },
  { "Size:",     numPairItem, 0 },
  { NULL,        alphaItem,   0 }
};

static const dynamic_form_item_t titlePropertiesItems[] = {
  { "Title:", alphaItem, 32 },
  { NULL,     alphaItem,  0 }
};

static const dynamic_form_item_t labelPropertiesItems[] = {
  { "Label:",    alphaItem,   32 },
  { "Position:", numPairItem,  0 },
  { "Font:",     numericItem,  4 },
  { "Usable",    checkboxItem, 0 },
  { NULL,        alphaItem,    0 }
};

static const dynamic_form_item_t fieldPropertiesItems[] = {
  { "Max chars:",   numericItem,  5 },
  { "Position:",    numPairItem,  0 },
  { "Size:",        numPairItem,  0 },
  { "Font:",        numericItem,  4 },
  { "Editable",     checkboxItem, 0 },
  { "Underlined",   checkboxItem, 0 },
  { "Single line",  checkboxItem, 0 },
  { NULL,           alphaItem,    0 }
};

static const dynamic_form_item_t controlPropertiesItems[] = {
  { "Label:",    alphaItem,   32 },
  { "Style:",    numericItem,  2 },
  { "Position:", numPairItem,  0 },
  { "Size:",     numPairItem,  0 },
  { "Font:",     numericItem,  4 },
  { "Usable",    checkboxItem, 0 },
  { NULL,        alphaItem,    0 }
};

static const dynamic_form_item_t graphicControlPropertiesItems[] = {
  { "Bitmap ID:", numericItem,  6 },
  { "Style:",     numericItem,  2 },
  { "Position:",  numPairItem,  0 },
  { "Size:",      numPairItem,  0 },
  { "Usable",     checkboxItem, 0 },
  { NULL,         alphaItem,    0 }
};

static const dynamic_form_item_t sliderControlPropertiesItems[] = {
  { "Thumb ID:",  numericItem,  6 },
  { "Bckgd ID:",  numericItem,  6 },
  { "Style:",     numericItem,  2 },
  { "Position:",  numPairItem,  0 },
  { "Size:",      numPairItem,  0 },
  { "Min value:", numericItem,  5 },
  { "Max value:", numericItem,  5 },
  { "Page size:", numericItem,  5 },
  { "Usable",     checkboxItem, 0 },
  { NULL,         alphaItem,    0 }
};

static const dynamic_form_item_t listPropertiesItems[] = {
  { "Position:",  numPairItem,  0 },
  { "Size:",      numPairItem,  0 },
  { "Font:",      numericItem,  4 },
  { "Num items:", numericItem,  5 },
  { "Usable",     checkboxItem, 0 },
  { NULL,         alphaItem,    0 }
};

static const dynamic_form_item_t tablePropertiesItems[] = {
  { "Position:", numPairItem,  0 },
  { "Size:",     numPairItem,  0 },
  { "Num cols:", numericItem,  5 },
  { "Num rows:", numericItem,  5 },
  { "Usable",    checkboxItem, 0 },
  { NULL,        alphaItem,    0 }
};

static const dynamic_form_item_t bitmapPropertiesItems[] = {
  { "Bitmap ID:", numericItem,  6 },
  { "Position:",  numPairItem,  0 },
  { "Usable",     checkboxItem, 0 },
  { NULL,         alphaItem,    0 }
};

static const dynamic_form_item_t scrollPropertiesItems[] = {
  { "Position:",  numPairItem,  0 },
  { "Size:",      numPairItem,  0 },
  { "Min value:", numericItem,  5 },
  { "Max value:", numericItem,  5 },
  { "Page size:", numericItem,  5 },
  { "Usable",     checkboxItem, 0 },
  { NULL,         alphaItem,    0 }
};

static const dynamic_form_item_t gadgetPropertiesItems[] = {
  { "Position:", numPairItem,  0 },
  { "Size:",     numPairItem,  0 },
  { "Extended",  checkboxItem, 0 },
  { "Usable",    checkboxItem, 0 },
  { NULL,        alphaItem,    0 }
};

static const dynamic_form_item_t grfPropertiesItems[] = {
  { "Position:", numPairItem, 0 },
  { NULL,        alphaItem,   0 }
};

static void drawForm(FormType *current, form_edit_t *data) {
  RectangleType rect;
  RGBColorType oldc, rgb;
  IndexedColorType formFill;
  WinHandle olda, oldd;
  UInt16 objIndex, index, state, i, j;

  if (data->formP) {
    oldd = WinSetDrawWindow(&data->formP->window);
    olda = WinGetActiveWindow();
    WinSetActiveWindow(&data->formP->window);

    FrmGetFormBounds(data->formP, &rect);
    rect.topLeft.x = 0;
    rect.topLeft.y = 0;
    formFill = UIColorGetTableEntryIndex(UIFormFill);
    WinSetBackColor(formFill);
    WinEraseRectangle(&rect, 0);

    for (i = 0; i <= rect.extent.y; i += 4) {
      for (j = 0; j <= rect.extent.x; j += 4) {
        WinPaintPixel(j, i);
      }
    }

    UIColorGetTableEntryRGB(UIFormFrame, &oldc);
    rgb.r = 0x80;
    rgb.g = 0x00;
    rgb.b = 0x80;
    UIColorSetTableEntry(UIFormFrame, &rgb);

    state = KbdGrfGetState();
    KbdGrfSetState(GRAFFITI_SHIFT);
    data->formP->attr.visible = 1;
    for (objIndex = 0; objIndex < data->formP->numObjects; objIndex++) {
      FrmDrawObject(data->formP, objIndex, false);

      if (FrmGetObjectType(data->formP, objIndex) == frmPopupObj) {
        for (index = 0; index < data->formP->numObjects; index++) {
          if (FrmGetObjectType(data->formP, index) == frmListObj) {
            if (data->formP->objects[index].object.list->id == data->formP->objects[objIndex].object.popup->listID) {
              FrmGetObjectBounds(data->formP, index, &data->bounds[objIndex]);
              break;
            }
          }
        }
      } else {
        FrmGetObjectBounds(data->formP, objIndex, &data->bounds[objIndex]);
      }
      RctInsetRectangle(&data->bounds[objIndex], 1);
    }
    KbdGrfSetState(state);

    if (data->selected >= 0 && data->selected < data->formP->numObjects) {
      WinSetForeColorRGB(&red, NULL);
      WinPaintRectangleFrame(1, &data->bounds[data->selected]);
    }

    UIColorSetTableEntry(UIFormFrame, &oldc);

    WinSetActiveWindow(olda);
    WinSetDrawWindow(oldd);
  }
}

static Boolean formGadgetCallback(FormGadgetTypeInCallback *gad, UInt16 cmd, void *param) {
  FormType *frm;
  EventType *event;
  RectangleType rect;
  RGBColorType oldb, oldt, oldf;
  PatternType oldp;
  form_edit_t *data;
  int index;

  if (cmd == formGadgetDeleteCmd) {
    return true;
  }

  frm = FrmGetActiveForm();
  index = FrmGetObjectIndex(frm, formGad);
  FrmGetObjectBounds(frm, index, &rect);
  data = (form_edit_t *)FrmGetGadgetData(frm, index);
  WinSetBackColorRGB(&white, &oldb);
  WinSetTextColorRGB(NULL, &oldt);
  oldp = WinGetPatternType();
  WinSetPatternType(blackPattern);

  switch (cmd) {
    case formGadgetDrawCmd:
      WinSetForeColorRGB(&gray, &oldf);
      drawForm(frm, data);
      WinSetForeColorRGB(&oldf, NULL);
      break;

    case formGadgetEraseCmd:
      WinSetForeColorRGB(&white, &oldf);
      WinPaintRectangle(&rect, 0);
      WinSetForeColorRGB(&oldf, NULL);
      break;

    case formGadgetHandleEventCmd:
      event = (EventType *)param;
      if (event->eType == frmGadgetEnterEvent) {
      }
      break;
  }

  WinSetPatternType(oldp);
  WinSetBackColorRGB(&oldb, NULL);
  WinSetTextColorRGB(&oldt, NULL);

  return true;
}

static void fillObjectNames(form_edit_t *data) {
  FormObjectType obj;
  UInt16 index;
  char buf[256];

  data->itemText = xcalloc(data->formP->numObjects, sizeof(char *));

  for (index = 0; index < data->formP->numObjects; index++) {
    obj = data->formP->objects[index].object;

    switch (FrmGetObjectType(data->formP, index)) {
      case frmFieldObj:
        sys_snprintf(buf, sizeof(buf)-1, "Field %d", obj.field->id);
        break;
      case frmControlObj:
        sys_snprintf(buf, sizeof(buf)-1, "Control %d", obj.control->id);
        break;
      case frmListObj:
        sys_snprintf(buf, sizeof(buf)-1, "List %d", obj.list->id);
        break;
      case frmTitleObj:
        sys_snprintf(buf, sizeof(buf)-1, "Title");
        break;
      case frmTableObj:
        sys_snprintf(buf, sizeof(buf)-1, "Table %d", obj.table->id);
        break;
      case frmBitmapObj:
        sys_snprintf(buf, sizeof(buf)-1, "Bitmap %d", obj.bitmap->rscID);
        break;
      case frmLineObj:
        sys_snprintf(buf, sizeof(buf)-1, "Line");
        break;
      case frmFrameObj:
        sys_snprintf(buf, sizeof(buf)-1, "Frame");
        break;
      case frmRectangleObj:
        sys_snprintf(buf, sizeof(buf)-1, "Rect");
        break;
      case frmLabelObj:
        sys_snprintf(buf, sizeof(buf)-1, "Label %d", obj.label->id);
        break;
      case frmPopupObj:
        sys_snprintf(buf, sizeof(buf)-1, "Popup %d %d", obj.popup->controlID, obj.popup->listID);
        break;
      case frmGraffitiStateObj:
        sys_snprintf(buf, sizeof(buf)-1, "Graffiti");
        break;
      case frmGadgetObj:
        sys_snprintf(buf, sizeof(buf)-1, "Gadget %d", obj.gadget->id);
        break;
      case frmScrollBarObj:
        sys_snprintf(buf, sizeof(buf)-1, "Scroll %d", obj.scrollBar->id);
        break;
    }
    data->itemText[index] = xstrdup(buf);
  }
}

static void freeObjectNames(form_edit_t *data) {
  UInt16 index;

  if (data->itemText) {
    for (index = 0; index < data->formP->numObjects; index++) {
      if (data->itemText[index]) xfree(data->itemText[index]);
    }
    xfree(data->itemText);
  }
}

static void drawObjectList(Int16 itemNum, RectangleType *bounds, Char **itemsText) {
  WinPaintChars(itemsText[itemNum], StrLen(itemsText[itemNum]), bounds->topLeft.x, bounds->topLeft.y);
}

static void formPropertiesCallback(FormType *frm, dynamic_form_phase_t phase, void *_data) {
  form_edit_t *data = (form_edit_t *)_data;
  RectangleType rect;
  UInt16 density, depth;
  Err err;

  switch (phase) {
    case setProperties:
      FrmGetFormBounds(data->formP, &rect);
      setFieldNum(frm, 1000, data->topLeft.x, true);
      setFieldNum(frm, 1001, data->topLeft.y, false);
      setFieldNum(frm, 1002, rect.extent.x, false);
      setFieldNum(frm, 1003, rect.extent.y, false);
      break;
    case getProperties:
      rect.topLeft.x = getFieldNum(frm, 1000);
      rect.topLeft.y = getFieldNum(frm, 1001);
      rect.extent.x = getFieldNum(frm, 1002);
      rect.extent.y = getFieldNum(frm, 1003);
      data->topLeft.x = rect.topLeft.x;
      data->topLeft.y = rect.topLeft.y;
      data->formP->window.windowBounds.extent.x = rect.extent.x;
      data->formP->window.windowBounds.extent.y = rect.extent.y;
      density = BmpGetDensity(data->formP->window.bitmapP);
      depth = BmpGetBitDepth(data->formP->window.bitmapP);
      BmpDelete(data->formP->window.bitmapP);
      data->formP->window.bitmapP = BmpCreate3(rect.extent.x, rect.extent.y, density, depth, false, 0, NULL, &err);
      data->changed = true;
      break;
    case finishForm:
      formGadgetCallback(NULL, formGadgetEraseCmd, NULL);
      formGadgetCallback(NULL, formGadgetDrawCmd, NULL);
      break;
  }
}

static void titlePropertiesCallback(FormType *frm, dynamic_form_phase_t phase, void *_data) {
  form_edit_t *data = (form_edit_t *)_data;
  FormTitleType *title;
  char *old, buf[64];
  int len;

  title = FrmGetObjectPtr(data->formP, data->selected);

  switch (phase) {
    case setProperties:
      setField(frm, 1000, title->text, true);
      break;
    case getProperties:
      getField(frm, 1000, buf, sizeof(buf));
      len = StrLen(buf);
      old = title->text;
      title->text = MemPtrNew(len + 1);
      MemMove(title->text, buf, len);
      if (old) MemPtrFree(old);
      data->changed = true;
      break;
    case finishForm:
      formGadgetCallback(NULL, formGadgetDrawCmd, NULL);
      break;
  }
}

static void labelPropertiesCallback(FormType *frm, dynamic_form_phase_t phase, void *_data) {
  form_edit_t *data = (form_edit_t *)_data;
  FormLabelType *label;
  int newLen;
  char buf[64];

  label = FrmGetObjectPtr(data->formP, data->selected);

  switch (phase) {
    case setProperties:
      setField(frm, 1000, label->text, true);
      setFieldNum(frm, 1001, label->pos.x, false);
      setFieldNum(frm, 1002, label->pos.y, false);
      setFieldNum(frm, 1003, label->fontID, false);
      setControlValue(frm, 1004, label->attr.usable);
      break;
    case getProperties:
      getField(frm, 1000, buf, sizeof(buf));
      newLen = StrLen(buf);
      label = pumpkin_heap_realloc(label, sizeof(FormLabelType) + newLen + 1, "Label");
      label->text = label->buf;
      label->len = newLen;
      MemMove(label->text, buf, newLen);
      label->text[newLen] = 0;
      label->pos.x = getFieldNum(frm, 1001);
      label->pos.y = getFieldNum(frm, 1002);
      label->fontID = getFieldNum(frm, 1003);
      label->attr.usable = getControlValue(frm, 1004);
      data->changed = true;
      break;
    case finishForm:
      formGadgetCallback(NULL, formGadgetDrawCmd, NULL);
      break;
  }
}

static void controlPropertiesCallback(FormType *frm, dynamic_form_phase_t phase, void *_data) {
  form_edit_t *data = (form_edit_t *)_data;
  RectangleType rect;
  ControlType *ctl;
  int newLen;
  char buf[64];

  ctl = FrmGetObjectPtr(data->formP, data->selected);

  switch (phase) {
    case setProperties:
      FrmGetObjectBounds(data->formP, data->selected, &rect);
      setField(frm, 1000, ctl->text, true);
      setFieldNum(frm, 1001, ctl->style, false);
      setFieldNum(frm, 1002, rect.topLeft.x, false);
      setFieldNum(frm, 1003, rect.topLeft.y, false);
      setFieldNum(frm, 1004, rect.extent.x, false);
      setFieldNum(frm, 1005, rect.extent.y, false);
      setFieldNum(frm, 1006, ctl->font, false);
      setControlValue(frm, 1007, ctl->attr.usable);
      break;
    case getProperties:
      getField(frm, 1000, buf, sizeof(buf));
      newLen = StrLen(buf);
      ctl = pumpkin_heap_realloc(ctl, sizeof(ControlType) + newLen + 1, "Control");
      ctl->text = ctl->buf;
      ctl->len = newLen;
      MemMove(ctl->text, buf, newLen);
      ctl->text[newLen] = 0;
      ctl->style = getFieldNum(frm, 1001);
      rect.topLeft.x = getFieldNum(frm, 1002);
      rect.topLeft.y = getFieldNum(frm, 1003);
      rect.extent.x = getFieldNum(frm, 1004);
      rect.extent.y = getFieldNum(frm, 1005);
      FrmSetObjectBounds(data->formP, data->selected, &rect);
      ctl->font = getFieldNum(frm, 1006);
      ctl->attr.usable = getControlValue(frm, 1007);
      data->changed = true;
      break;
    case finishForm:
      formGadgetCallback(NULL, formGadgetDrawCmd, NULL);
      break;
  }
}

static void graphicControlPropertiesCallback(FormType *frm, dynamic_form_phase_t phase, void *_data) {
  form_edit_t *data = (form_edit_t *)_data;
  RectangleType rect;
  GraphicControlType *ctl;

  ctl = FrmGetObjectPtr(data->formP, data->selected);

  switch (phase) {
    case setProperties:
      FrmGetObjectBounds(data->formP, data->selected, &rect);
      setFieldNum(frm, 1000, ctl->bitmapID, true);
      setFieldNum(frm, 1001, ctl->style, false);
      setFieldNum(frm, 1002, rect.topLeft.x, false);
      setFieldNum(frm, 1003, rect.topLeft.y, false);
      setFieldNum(frm, 1004, rect.extent.x, false);
      setFieldNum(frm, 1005, rect.extent.y, false);
      setControlValue(frm, 1006, ctl->attr.usable);
      break;
    case getProperties:
      ctl->bitmapID = getFieldNum(frm, 1000);
      ctl->style = getFieldNum(frm, 1001);
      rect.topLeft.x = getFieldNum(frm, 1002);
      rect.topLeft.y = getFieldNum(frm, 1003);
      rect.extent.x = getFieldNum(frm, 1004);
      rect.extent.y = getFieldNum(frm, 1005);
      FrmSetObjectBounds(data->formP, data->selected, &rect);
      ctl->attr.usable = getControlValue(frm, 1006);
      data->changed = true;
      break;
    case finishForm:
      formGadgetCallback(NULL, formGadgetDrawCmd, NULL);
      break;
  }
}

static void sliderControlPropertiesCallback(FormType *frm, dynamic_form_phase_t phase, void *_data) {
  form_edit_t *data = (form_edit_t *)_data;
  RectangleType rect;
  SliderControlType *ctl;

  ctl = FrmGetObjectPtr(data->formP, data->selected);

  switch (phase) {
    case setProperties:
      FrmGetObjectBounds(data->formP, data->selected, &rect);
      setFieldNum(frm, 1000, ctl->thumbID, true);
      setFieldNum(frm, 1001, ctl->backgroundID, false);
      setFieldNum(frm, 1002, ctl->style, false);
      setFieldNum(frm, 1003, rect.topLeft.x, false);
      setFieldNum(frm, 1004, rect.topLeft.y, false);
      setFieldNum(frm, 1005, rect.extent.x, false);
      setFieldNum(frm, 1006, rect.extent.y, false);
      setFieldNum(frm, 1007, ctl->minValue, false);
      setFieldNum(frm, 1008, ctl->maxValue, false);
      setFieldNum(frm, 1009, ctl->pageSize, false);
      setControlValue(frm, 1010, ctl->attr.usable);
      break;
    case getProperties:
      ctl->thumbID = getFieldNum(frm, 1000);
      ctl->backgroundID = getFieldNum(frm, 1001);
      ctl->style = getFieldNum(frm, 1002);
      rect.topLeft.x = getFieldNum(frm, 1003);
      rect.topLeft.y = getFieldNum(frm, 1004);
      rect.extent.x = getFieldNum(frm, 1005);
      rect.extent.y = getFieldNum(frm, 1006);
      FrmSetObjectBounds(data->formP, data->selected, &rect);
      ctl->minValue = getFieldNum(frm, 1007);
      ctl->maxValue = getFieldNum(frm, 1008);
      ctl->pageSize = getFieldNum(frm, 1009);
      ctl->attr.usable = getControlValue(frm, 1010);
      data->changed = true;
      break;
    case finishForm:
      formGadgetCallback(NULL, formGadgetDrawCmd, NULL);
      break;
  }
}

static void fieldPropertiesCallback(FormType *frm, dynamic_form_phase_t phase, void *_data) {
  form_edit_t *data = (form_edit_t *)_data;
  RectangleType rect;
  FieldType *fld;

  fld = FrmGetObjectPtr(data->formP, data->selected);

  switch (phase) {
    case setProperties:
      FrmGetObjectBounds(data->formP, data->selected, &rect);
      setFieldNum(frm, 1000, fld->maxChars, true);
      setFieldNum(frm, 1001, rect.topLeft.x, false);
      setFieldNum(frm, 1002, rect.topLeft.y, false);
      setFieldNum(frm, 1003, rect.extent.x, false);
      setFieldNum(frm, 1004, rect.extent.y, false);
      setFieldNum(frm, 1005, fld->fontID, false);
      setControlValue(frm, 1006, fld->attr.editable);
      setControlValue(frm, 1007, fld->attr.underlined);
      setControlValue(frm, 1008, fld->attr.singleLine);
      break;
    case getProperties:
      fld->maxChars = getFieldNum(frm, 1000);
      rect.topLeft.x = getFieldNum(frm, 1001);
      rect.topLeft.y = getFieldNum(frm, 1002);
      rect.extent.x = getFieldNum(frm, 1003);
      rect.extent.y = getFieldNum(frm, 1004);
      FrmSetObjectBounds(data->formP, data->selected, &rect);
      fld->fontID = getFieldNum(frm, 1005);
      fld->attr.editable = getControlValue(frm, 1006);
      fld->attr.underlined = getControlValue(frm, 1007);
      fld->attr.singleLine = getControlValue(frm, 1008);
      data->changed = true;
      break;
    case finishForm:
      formGadgetCallback(NULL, formGadgetDrawCmd, NULL);
      break;
  }
}

static void listPropertiesCallback(FormType *frm, dynamic_form_phase_t phase, void *_data) {
  form_edit_t *data = (form_edit_t *)_data;
  RectangleType rect;
  ListType *lst;

  lst = FrmGetObjectPtr(data->formP, data->selected);

  switch (phase) {
    case setProperties:
      FrmGetObjectBounds(data->formP, data->selected, &rect);
      setFieldNum(frm, 1000, rect.topLeft.x, true);
      setFieldNum(frm, 1001, rect.topLeft.y, false);
      setFieldNum(frm, 1002, rect.extent.x, false);
      setFieldNum(frm, 1003, rect.extent.y, false);
      setFieldNum(frm, 1004, lst->font, false);
      setFieldNum(frm, 1005, lst->numItems, false);
      setControlValue(frm, 1006, lst->attr.usable);
      break;
    case getProperties:
      rect.topLeft.x = getFieldNum(frm, 1000);
      rect.topLeft.y = getFieldNum(frm, 1001);
      rect.extent.x = getFieldNum(frm, 1002);
      rect.extent.y = getFieldNum(frm, 1003);
      FrmSetObjectBounds(data->formP, data->selected, &rect);
      lst->font = getFieldNum(frm, 1004);
      lst->numItems = getFieldNum(frm, 1005);
      lst->attr.usable = getControlValue(frm, 1006);
      data->changed = true;
      break;
    case finishForm:
      formGadgetCallback(NULL, formGadgetDrawCmd, NULL);
      break;
  }
}

static void tablePropertiesCallback(FormType *frm, dynamic_form_phase_t phase, void *_data) {
  form_edit_t *data = (form_edit_t *)_data;
  RectangleType rect;
  TableType *tbl;

  tbl = FrmGetObjectPtr(data->formP, data->selected);

  switch (phase) {
    case setProperties:
      FrmGetObjectBounds(data->formP, data->selected, &rect);
      setFieldNum(frm, 1000, rect.topLeft.x, true);
      setFieldNum(frm, 1001, rect.topLeft.y, false);
      setFieldNum(frm, 1002, rect.extent.x, false);
      setFieldNum(frm, 1003, rect.extent.y, false);
      setFieldNum(frm, 1004, tbl->numColumns, false);
      setFieldNum(frm, 1005, tbl->numRows, false);
      setControlValue(frm, 1006, tbl->attr.usable);
      break;
    case getProperties:
      rect.topLeft.x = getFieldNum(frm, 1000);
      rect.topLeft.y = getFieldNum(frm, 1001);
      rect.extent.x = getFieldNum(frm, 1002);
      rect.extent.y = getFieldNum(frm, 1003);
      FrmSetObjectBounds(data->formP, data->selected, &rect);
      tbl->numColumns = getFieldNum(frm, 1004);
      tbl->numRows = getFieldNum(frm, 1005);
      tbl->attr.usable = getControlValue(frm, 1006);
      data->changed = true;
      break;
    case finishForm:
      formGadgetCallback(NULL, formGadgetDrawCmd, NULL);
      break;
  }
}

static void bitmapPropertiesCallback(FormType *frm, dynamic_form_phase_t phase, void *_data) {
  form_edit_t *data = (form_edit_t *)_data;
  FormBitmapType *bitmap;

  bitmap = FrmGetObjectPtr(data->formP, data->selected);

  switch (phase) {
    case setProperties:
      setFieldNum(frm, 1000, bitmap->rscID, true);
      setFieldNum(frm, 1001, bitmap->pos.x, false);
      setFieldNum(frm, 1002, bitmap->pos.y, false);
      setControlValue(frm, 1003, bitmap->attr.usable);
      break;
    case getProperties:
      bitmap->rscID = getFieldNum(frm, 1000);
      bitmap->pos.x = getFieldNum(frm, 1001);
      bitmap->pos.y = getFieldNum(frm, 1002);
      bitmap->attr.usable = getControlValue(frm, 1003);
      data->changed = true;
      break;
    case finishForm:
      formGadgetCallback(NULL, formGadgetDrawCmd, NULL);
      break;
  }
}

static void scrollPropertiesCallback(FormType *frm, dynamic_form_phase_t phase, void *_data) {
  form_edit_t *data = (form_edit_t *)_data;
  RectangleType rect;
  ScrollBarType *scl;

  scl = FrmGetObjectPtr(data->formP, data->selected);

  switch (phase) {
    case setProperties:
      FrmGetObjectBounds(data->formP, data->selected, &rect);
      setFieldNum(frm, 1000, rect.topLeft.x, true);
      setFieldNum(frm, 1001, rect.topLeft.y, false);
      setFieldNum(frm, 1002, rect.extent.x, false);
      setFieldNum(frm, 1003, rect.extent.y, false);
      setFieldNum(frm, 1004, scl->minValue, false);
      setFieldNum(frm, 1005, scl->maxValue, false);
      setFieldNum(frm, 1006, scl->pageSize, false);
      setControlValue(frm, 1007, scl->attr.usable);
      break;
    case getProperties:
      rect.topLeft.x = getFieldNum(frm, 1000);
      rect.topLeft.y = getFieldNum(frm, 1001);
      rect.extent.x = getFieldNum(frm, 1002);
      rect.extent.y = getFieldNum(frm, 1003);
      FrmSetObjectBounds(data->formP, data->selected, &rect);
      scl->minValue = getFieldNum(frm, 1004);
      scl->maxValue = getFieldNum(frm, 1005);
      scl->pageSize = getFieldNum(frm, 1006);
      scl->attr.usable = getControlValue(frm, 1007);
      data->changed = true;
      break;
    case finishForm:
      formGadgetCallback(NULL, formGadgetDrawCmd, NULL);
      break;
  }
}

static void gadgetPropertiesCallback(FormType *frm, dynamic_form_phase_t phase, void *_data) {
  form_edit_t *data = (form_edit_t *)_data;
  RectangleType rect;
  FormGadgetType *gad;

  gad = FrmGetObjectPtr(data->formP, data->selected);

  switch (phase) {
    case setProperties:
      FrmGetObjectBounds(data->formP, data->selected, &rect);
      setFieldNum(frm, 1000, rect.topLeft.x, false);
      setFieldNum(frm, 1001, rect.topLeft.y, false);
      setFieldNum(frm, 1002, rect.extent.x, false);
      setFieldNum(frm, 1003, rect.extent.y, false);
      setControlValue(frm, 1004, gad->attr.extended);
      setControlValue(frm, 1005, gad->attr.usable);
      break;
    case getProperties:
      rect.topLeft.x = getFieldNum(frm, 1000);
      rect.topLeft.y = getFieldNum(frm, 1001);
      rect.extent.x = getFieldNum(frm, 1002);
      rect.extent.y = getFieldNum(frm, 1003);
      FrmSetObjectBounds(data->formP, data->selected, &rect);
      gad->attr.extended = getControlValue(frm, 1004);
      gad->attr.usable = getControlValue(frm, 1005);
      data->changed = true;
      break;
    case finishForm:
      formGadgetCallback(NULL, formGadgetDrawCmd, NULL);
      break;
  }
}

static void grfPropertiesCallback(FormType *frm, dynamic_form_phase_t phase, void *_data) {
  form_edit_t *data = (form_edit_t *)_data;
  RectangleType rect;

  switch (phase) {
    case setProperties:
      FrmGetObjectBounds(data->formP, data->selected, &rect);
      setFieldNum(frm, 1000, rect.topLeft.x, false);
      setFieldNum(frm, 1001, rect.topLeft.y, false);
      break;
    case getProperties:
      rect.topLeft.x = getFieldNum(frm, 1000);
      rect.topLeft.y = getFieldNum(frm, 1001);
      FrmSetObjectBounds(data->formP, data->selected, &rect);
      data->changed = true;
      break;
    case finishForm:
      formGadgetCallback(NULL, formGadgetDrawCmd, NULL);
      break;
  }
}

static Boolean eventHandler(EventType *event) {
  form_edit_t *data;
  RectangleType rect;
  FormType *frm;
  ListType *lst;
  ControlType *ctl;
  UInt16 index;
  char *s;
  Boolean handled = false;

  frm = FrmGetActiveForm();
  index = FrmGetObjectIndex(frm, formGad);
  data = (form_edit_t *)FrmGetGadgetData(frm, index);

  switch (event->eType) {
    case frmOpenEvent:
      index = FrmGetObjectIndex(frm, frmpropCtl);
      ctl = (ControlType *)FrmGetObjectPtr(frm, index);
      ctl->attr.frame = noButtonFrame;

      index = FrmGetObjectIndex(frm, objpropCtl);
      ctl = (ControlType *)FrmGetObjectPtr(frm, index);
      ctl->attr.frame = noButtonFrame;

      if (data->formP->numObjects > 0) {
        index = FrmGetObjectIndex(frm, objectList);
        lst = (ListType *)FrmGetObjectPtr(frm, index);
        LstSetDrawFunction(lst, drawObjectList);
        LstSetListChoices(lst, data->itemText, data->formP->numObjects);
        LstSetHeight(lst, data->formP->numObjects);
        LstSetSelection(lst, 0);
        s = LstGetSelectionText(lst, 0);
        index = FrmGetObjectIndex(frm, objectCtl);
        ctl = (ControlType *)FrmGetObjectPtr(frm, index);
        CtlSetLabel(ctl, s);
        sys_snprintf(data->title, MAX_TITLE-1, "%s: %s", data->rtitle, s);
      } else {
        index = FrmGetObjectIndex(frm, objectCtl);
        FrmHideObject(frm, index);
        index = FrmGetObjectIndex(frm, objpropCtl);
        FrmHideObject(frm, index);
        StrNCopy(data->title, data->rtitle, MAX_TITLE-1);
      }
      FrmSetTitle(frm, data->title);

      index = FrmGetObjectIndex(frm, formGad);
      FrmGetObjectBounds(frm, index, &rect);

      data->formP->window.windowBounds.topLeft.x = rect.topLeft.x + (rect.extent.x - data->formP->window.windowBounds.extent.x) / 2;
      data->formP->window.windowBounds.topLeft.y = rect.topLeft.y + (rect.extent.y - data->formP->window.windowBounds.extent.y) / 2;
      data->formP->window.windowBounds.topLeft.x += frm->window.windowBounds.topLeft.x;
      data->formP->window.windowBounds.topLeft.y += frm->window.windowBounds.topLeft.y;

      for (index = 0; index < data->formP->numObjects; index++) {
        if (FrmGetObjectType(data->formP, index) == frmListObj) {
          lst = (ListType *)FrmGetObjectPtr(data->formP, index);
          lst->popupWin->windowBounds.topLeft.x = data->formP->window.windowBounds.topLeft.x + lst->bounds.topLeft.x;
          lst->popupWin->windowBounds.topLeft.y = data->formP->window.windowBounds.topLeft.y + lst->bounds.topLeft.y;
        }
      }

      FrmDrawForm(frm);
      handled = true;
      break;
    case popSelectEvent:
      if (event->data.popSelect.listID == objectList) {
        data->selected = event->data.popSelect.selection;
        formGadgetCallback(NULL, formGadgetEraseCmd, NULL);
        formGadgetCallback(NULL, formGadgetDrawCmd, NULL);
        index = FrmGetObjectIndex(frm, objectList);
        lst = (ListType *)FrmGetObjectPtr(frm, index);
        s = LstGetSelectionText(lst, data->selected);
        sys_snprintf(data->title, MAX_TITLE-1, "%s: %s", data->rtitle, s);
        FrmSetTitle(frm, data->title);
      }
      break;
    case ctlExitEvent:
      switch (event->data.ctlSelect.controlID) {
        case objpropCtl:
          index = FrmGetObjectIndex(frm, objpropCtl);
          ctl = (ControlType *)FrmGetObjectPtr(frm, index);
          CtlSetValue(ctl, 0);
          handled = true;
          break;
      }
      break;
    case ctlSelectEvent:
      switch (event->data.ctlSelect.controlID) {
        case frmpropCtl:
          CtlSetValue(event->data.ctlSelect.pControl, 0);
          showDynamicForm(formPropertiesItems, "Form", formPropertiesCallback, data);
          handled = true;
          break;
        case objpropCtl:
          CtlSetValue(event->data.ctlSelect.pControl, 0);

          if (data->selected != -1) {
            switch (FrmGetObjectType(data->formP, data->selected)) {
              case frmTitleObj:
                showDynamicForm(titlePropertiesItems, "Title", titlePropertiesCallback, data);
                break;
              case frmLabelObj:
                showDynamicForm(labelPropertiesItems, "Label", labelPropertiesCallback, data);
                break;
              case frmFieldObj:
                showDynamicForm(fieldPropertiesItems, "Field", fieldPropertiesCallback, data);
                break;
              case frmControlObj:
                ctl = (ControlType *)FrmGetObjectPtr(data->formP, data->selected);
                if (ctl->style == sliderCtl || ctl->style == feedbackSliderCtl) {
                  showDynamicForm(sliderControlPropertiesItems, "Control", sliderControlPropertiesCallback, data);
                } else if (ctl->attr.graphical) {
                  showDynamicForm(graphicControlPropertiesItems, "Control", graphicControlPropertiesCallback, data);
                } else {
                  showDynamicForm(controlPropertiesItems, "Control", controlPropertiesCallback, data);
                }
                break;
              case frmListObj:
                showDynamicForm(listPropertiesItems, "List", listPropertiesCallback, data);
                break;
              case frmBitmapObj:
                showDynamicForm(bitmapPropertiesItems, "Bitmap", bitmapPropertiesCallback, data);
                break;
              case frmGadgetObj:
                showDynamicForm(gadgetPropertiesItems, "Gadget", gadgetPropertiesCallback, data);
                break;
              case frmGraffitiStateObj:
                showDynamicForm(grfPropertiesItems, "Graffiti State", grfPropertiesCallback, data);
                break;
              case frmScrollBarObj:
                showDynamicForm(scrollPropertiesItems, "Scroll Bar", scrollPropertiesCallback, data);
                break;
              case frmTableObj:
                showDynamicForm(tablePropertiesItems, "Table", tablePropertiesCallback, data);
                break;
              default:
                break;
            }
          }
          handled = true;
          break;
        case okBtn:
          data->stop = true;
          handled = true;
          break;
        case cancelBtn:
          if (!data->changed || FrmCustomAlert(QuestionAlert, "Discard changes ?", "", "") == 0) {
            data->changed = false;
            data->stop = true;
          }
          handled = true;
          break;
      }
      break;
    default:
      break;
  }

  return handled;
}

Boolean editForm(FormType *frm, char *title, MemHandle h) {
  form_edit_t data;
  FormType *previous;
  EventType event;
  Coord width, height;
  UInt32 density, depth;
  UInt16 index;
  void *p;
  Err err;
  Boolean r = false;

  MemSet(&data, sizeof(form_edit_t), 0);
  data.frm = frm;
  data.handle = h;
  data.rtitle = title;

  if ((p = MemHandleLock(h)) != NULL) {
    data.size = MemHandleSize(h);
    data.rsrc = pumpkin_heap_alloc(data.size, "form_rsrc");
    MemMove(data.rsrc, p, data.size);
    MemHandleUnlock(h);
  }

  if (data.rsrc) {
    if ((data.formP = pumpkin_create_form(data.rsrc, data.size)) != NULL) {
      WinScreenGetAttribute(winScreenDensity, &density);
      WinScreenMode(winScreenModeGetDefaults, NULL, NULL, &depth, NULL);
      width = data.formP->window.windowBounds.extent.x;
      height = data.formP->window.windowBounds.extent.y;
      WinAdjustCoords(&width, &height);
      data.formP->window.bitmapP = BmpCreate3(width, height, density, depth, false, 0, NULL, &err);
      data.topLeft.x = data.formP->window.windowBounds.topLeft.x;
      data.topLeft.y = data.formP->window.windowBounds.topLeft.y;
      data.bounds = xcalloc(data.formP->numObjects, sizeof(RectangleType));
      fillObjectNames(&data);

      index = FrmGetObjectIndex(frm, formGad);
      FrmSetGadgetHandler(frm, index, formGadgetCallback);
      FrmSetGadgetData(frm, index, &data);

      FrmSetEventHandler(frm, eventHandler);
      previous = FrmGetActiveForm();
      FrmSetActiveForm(frm);
      MemSet(&event, sizeof(EventType), 0);
      event.eType = frmOpenEvent;
      event.data.frmOpen.formID = frm->formId;
      FrmDispatchEvent(&event);

      do {
        EvtGetEvent(&event, 500);
        if (SysHandleEvent(&event)) continue;
        if (MenuHandleEvent(NULL, &event, &err)) continue;
        if (eventHandler(&event)) continue;
        FrmDispatchEvent(&event);
      } while (event.eType != appStopEvent && !data.stop);

      if (data.formP->window.bitmapP) {
        BmpDelete(data.formP->window.bitmapP);
      }
      if (data.formP->bitsBehindForm) {
        WinDeleteWindow(data.formP->bitsBehindForm, false);
      }
      pumpkin_destroy_form(data.formP);
      freeObjectNames(&data);
      xfree(data.bounds);

      FrmEraseForm(frm);
      FrmSetActiveForm(previous);

      if (data.changed) {
        r = true;
      }
    }
  }

  return r;
}
