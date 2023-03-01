#include <PalmOS.h>

#include "sys.h"
#include "thread.h"
#include "pwindow.h"
#include "vfs.h"
#include "pumpkin.h"
#include "dia.h"
#include "debug.h"
#include "xalloc.h"

#define TW 12
#define TH 14

#define CODE_CAP    157
#define CODE_SHIFT  158

#define PALMOS_MODULE "Keyboard"

typedef struct {
  KeyboardType kbd;
  Boolean upper;
  Boolean cap;
  Int16 sel;
  UInt16 pos;
  UInt16 bitmapBase;
  RectangleType bounds[256];
  UInt16 graffitiState;
} kbd_module_t;

extern thread_key_t *kbd_key;

static UInt8 alpha_lower[] = {
  'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p',
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'',
  'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.',
  '-', '/'
};

static UInt8 alpha_upper[] = {
  'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P',
  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"',
  'Z', 'X', 'C', 'V', 'B', 'N', 'M', '!', '*',
  '_', '?'
};

static UInt8 accent_lower[] = {
  225, 224, 228, 226, 229, 227, 230, 231, 241, 248,
  233, 232, 235, 234, 237, 236, 239, 238, 223, 253, 255,
  243, 242, 246, 244, 245, 250, 249, 252, 251,
  191, 171
};

static UInt8 accent_upper[] = {
  193, 192, 196, 194, 197, 195, 198, 199, 209, 216,
  201, 200, 203, 202, 205, 204, 207, 206, 223, 221, 159,
  211, 210, 214, 212, 213, 218, 217, 220, 219,
  161, 187
};

int KeyboardInitModule(void) {
  kbd_module_t *module;

  if ((module = xcalloc(1, sizeof(kbd_module_t))) == NULL) {
    return -1;
  }

  module->kbd = kbdAlpha;
  module->upper = false;
  module->bitmapBase = 32000;
  thread_set(kbd_key, module);

  return 0;
}

int KeyboardFinishModule(void) {
  kbd_module_t *module = (kbd_module_t *)thread_get(kbd_key);

  if (module) {
    xfree(module);
  }

  return 0;
}

static void DrawKeyFrame(Coord x1, Coord y1, Coord x2, Coord y2, Boolean last) {
  UInt16 prev;

  prev = WinSetCoordinateSystem(kCoordinatesDouble);
  x1 = WinScaleCoord(x1, false);
  y1 = WinScaleCoord(y1, false);
  x2 = WinScaleCoord(x2, true);
  y2 = WinScaleCoord(y2, true);
  WinDrawLine(x1, y1, x1, y2);
  if (last) WinDrawLine(x2, y1, x2, y2);
  WinDrawLine(x1, y1, x2, y1);
  WinDrawLine(x1, y2, x2, y2);
  WinSetCoordinateSystem(prev);
}

static UInt16 DrawChar(Char *label, UInt8 code, UInt16 bitmapID, UInt16 tw, UInt16 th, Coord x, Coord y, Boolean first, Boolean last) {
  kbd_module_t *module = (kbd_module_t *)thread_get(kbd_key);
  RectangleType rect;
  MemHandle h;
  BitmapType *bmp;
  Coord bw, bh;
  UInt16 len, cw, ch;
  Char buf[16];
  Boolean bitmapDrawn = false;

  RctSetRectangle(&rect, x, y, tw, th);
  WinEraseRectangle(&rect, 0);

  if (code) {
    RctSetRectangle(&rect, x+1, y+1, tw - (last ? 2 : 1), th-2);
    MemMove(&module->bounds[code], &rect, sizeof(RectangleType));

    if (bitmapID) {
      if ((h = DmGetResource(bitmapRsc, bitmapID)) != NULL) {
        if ((bmp = MemHandleLock(h)) != NULL) {
          BmpGetDimensions(bmp, &bw, &bh, NULL);
          WinPaintBitmap(bmp, x + (tw - bw) / 2, y + (th - bh) / 2);
          MemHandleUnlock(h);
          bitmapDrawn = true;
        }
        DmReleaseResource(h);
      }
    }

    if (!bitmapDrawn) {
      if (label) {
        len = StrLen(label);
        cw = FntCharsWidth(label, len);
        StrCopy(buf, label);
      } else {
        len = 1;
        cw = FntCharWidth(code);
        buf[0] = code;
        buf[1] = 0;
      }
      ch = FntCharHeight();
      WinDrawChars(buf, len, x + (tw - cw) / 2 + 1, y + (th - ch) / 2);
    }

  }

  DrawKeyFrame(x, y, x + tw - 1, y + th - 1, last);

  return x + tw;
}

static void DrawAlphaKeyboard(UInt8 *keys, UInt16 tw, UInt16 th, RectangleType *rect) {
  kbd_module_t *module = (kbd_module_t *)thread_get(kbd_key);
  UInt16 i;
  Coord x, y;

  x = rect->topLeft.x;
  y = rect->topLeft.y + 6;
  i = 0;

  x = DrawChar("esc", 27, 0, tw+6, th, x, y, false, false);
  x = DrawChar(NULL, keys[i++],    0, tw, th, x, y, false, false);
  x = DrawChar(NULL, keys[i++],    0, tw, th, x, y, false, false);
  x = DrawChar(NULL, keys[i++],    0, tw, th, x, y, false, false);
  x = DrawChar(NULL, keys[i++],    0, tw, th, x, y, false, false);
  x = DrawChar(NULL, keys[i++],    0, tw, th, x, y, false, false);
  x = DrawChar(NULL, keys[i++],    0, tw, th, x, y, false, false);
  x = DrawChar(NULL, keys[i++],    0, tw, th, x, y, false, false);
  x = DrawChar(NULL, keys[i++],    0, tw, th, x, y, false, false);
  x = DrawChar(NULL, keys[i++],    0, tw, th, x, y, false, false);
  x = DrawChar(NULL, keys[i++],    0, tw, th, x, y, false, false);
  x = DrawChar("back", '\b', module->bitmapBase, rect->extent.x - 11*tw - 6, th, x, y, false, true);
  x = rect->topLeft.x;
  y += th;
  x = DrawChar("tab", '\t',  module->bitmapBase+1, rect->extent.x - 11*tw, th, x, y, true, false);
  x = DrawChar(NULL, keys[i++],    0, tw, th, x, y, false, false);
  x = DrawChar(NULL, keys[i++],    0, tw, th, x, y, false, false);
  x = DrawChar(NULL, keys[i++],    0, tw, th, x, y, false, false);
  x = DrawChar(NULL, keys[i++],    0, tw, th, x, y, false, false);
  x = DrawChar(NULL, keys[i++],    0, tw, th, x, y, false, false);
  x = DrawChar(NULL, keys[i++],    0, tw, th, x, y, false, false);
  x = DrawChar(NULL, keys[i++],    0, tw, th, x, y, false, false);
  x = DrawChar(NULL, keys[i++],    0, tw, th, x, y, false, false);
  x = DrawChar(NULL, keys[i++],    0, tw, th, x, y, false, false);
  x = DrawChar(NULL, keys[i++],    0, tw, th, x, y, false, false);
  x = DrawChar(NULL, keys[i++],    0, tw, th, x, y, false, true);
  x = rect->topLeft.x;
  y += th;
  x = DrawChar("cap", CODE_CAP, 0, tw+8, th, x, y, true, false);
  x = DrawChar(NULL, keys[i++],     0, tw, th, x, y, false, false);
  x = DrawChar(NULL, keys[i++],     0, tw, th, x, y, false, false);
  x = DrawChar(NULL, keys[i++],     0, tw, th, x, y, false, false);
  x = DrawChar(NULL, keys[i++],     0, tw, th, x, y, false, false);
  x = DrawChar(NULL, keys[i++],     0, tw, th, x, y, false, false);
  x = DrawChar(NULL, keys[i++],     0, tw, th, x, y, false, false);
  x = DrawChar(NULL, keys[i++],     0, tw, th, x, y, false, false);
  x = DrawChar(NULL, keys[i++],     0, tw, th, x, y, false, false);
  x = DrawChar(NULL, keys[i++],     0, tw, th, x, y, false, false);
  x = DrawChar("enter", '\n', module->bitmapBase+2, rect->extent.x - 10*tw - 8, th, x, y, false, true);
  x = rect->topLeft.x;
  y += th;
  x = DrawChar("shf", CODE_SHIFT, 0, tw+16, th, x, y, true, false);
  x = DrawChar("space", ' ', 0, rect->extent.x - 3*tw - 16, th, x, y, false, false);
  x = DrawChar(NULL, keys[i++],     0, tw, th, x, y, false, false);
  x = DrawChar(NULL, keys[i++],     0, tw, th, x, y, false, true);
}

static void DrawNumberKeyboard(UInt16 tw, UInt16 th, RectangleType *rect) {
  kbd_module_t *module = (kbd_module_t *)thread_get(kbd_key);
  Coord x, y, x0, y0;

  x0 = rect->topLeft.x + 6;
  y0 = rect->topLeft.y + 6;

  x = x0;
  y = y0;
  x = DrawChar(NULL, '$', 0, tw, th, x, y, false, false);
  x = DrawChar(NULL, 128, 0, tw, th, x, y, false, false); // Euro symbol
  x = DrawChar(NULL, 163, 0, tw, th, x, y, false, false); // Pound symbol
  x = DrawChar(NULL, 165, 0, tw, th, x, y, false, true);  // Yen symbol
  x =  x0;
  y += th;
  x = DrawChar(NULL, '[', 0, tw, th, x, y, false, false);
  x = DrawChar(NULL, ']', 0, tw, th, x, y, false, false);
  x = DrawChar(NULL, '{', 0, tw, th, x, y, false, false);
  x = DrawChar(NULL, '}', 0, tw, th, x, y, false, true);
  x = x0;
  y += th;
  x = DrawChar(NULL, '<', 0, tw, th, x, y, false, false);
  x = DrawChar(NULL, '>', 0, tw, th, x, y, false, false);
  x = DrawChar(NULL, '\\', 0, tw, th, x, y, false, false);
  x = DrawChar(NULL, '=', 0, tw, th, x, y, false, true);
  x = x0;
  y += th;
  x = DrawChar(NULL, '@', 0, tw, th, x, y, false, false);
  x = DrawChar(NULL, '~', 0, tw, th, x, y, false, false);
  x = DrawChar(NULL, '&', 0, tw, th, x, y, false, false);
  x = DrawChar(NULL, '#', 0, tw, th, x, y, false, true);

  x += 8;
  x0 = x;
  y = y0;
  x = DrawChar(NULL, '1', 0, tw, th, x, y, false, false);
  x = DrawChar(NULL, '2', 0, tw, th, x, y, false, false);
  x = DrawChar(NULL, '3', 0, tw, th, x, y, false, true);
  x = x0;
  y += th;
  x = DrawChar(NULL, '4', 0, tw, th, x, y, false, false);
  x = DrawChar(NULL, '5', 0, tw, th, x, y, false, false);
  x = DrawChar(NULL, '6', 0, tw, th, x, y, false, true);
  x = x0;
  y += th;
  x = DrawChar(NULL, '7', 0, tw, th, x, y, false, false);
  x = DrawChar(NULL, '8', 0, tw, th, x, y, false, false);
  x = DrawChar(NULL, '9', 0, tw, th, x, y, false, true);
  x = x0;
  y += th;
  x = DrawChar(NULL, '(', 0, tw, th, x, y, false, false);
  x = DrawChar(NULL, '0', 0, tw, th, x, y, false, false);
  x = DrawChar(NULL, ')', 0, tw, th, x, y, false, true);

  x += 8;
  x0 = x;
  y = y0;
  x = DrawChar(NULL, '-', 0, tw, th, x, y, false, false);
  x = DrawChar(NULL, '+', 0, tw, th, x, y, false, false);
  x = DrawChar("back", '\b', module->bitmapBase, tw*2, th, x, y, false, true);
  x = x0;
  y += th;
  x = DrawChar(NULL, '/', 0, tw, th, x, y, false, false);
  x = DrawChar(NULL, '*', 0, tw, th, x, y, false, false);
  x = DrawChar(NULL, ':', 0, tw, th, x, y, false, false);
  x = DrawChar("tab", '\t',  module->bitmapBase+1, tw, th, x, y, false, true);
  x = x0;
  y += th;
  x = DrawChar(NULL, '.', 0, tw, th, x, y, false, false);
  x = DrawChar(NULL, ',', 0, tw, th, x, y, false, false);
  x = DrawChar("enter", '\n', module->bitmapBase+2, tw*2, th, x, y, false, true);
  x = x0;
  y += th;
  x = DrawChar("space", ' ', 0, tw*3, th, x, y, false, false);
  x = DrawChar(NULL, '%', 0, tw, th, x, y, false, true);
}

static void DrawKeyboard(KeyboardType kbd, Boolean upper, UInt16 tw, UInt16 th, RectangleType *rect) {
  WinEraseRectangle(rect, 0);

  switch (kbd) {
    case kbdAlpha:
      debug(DEBUG_INFO, PALMOS_MODULE, "SysKeyboardGadgetCallback draw alpha keyboard");
      DrawAlphaKeyboard(upper ? alpha_upper : alpha_lower, tw, th, rect);
      break;
    case kbdNumbersAndPunc:
      debug(DEBUG_INFO, PALMOS_MODULE, "SysKeyboardGadgetCallback draw number keyboard");
      DrawNumberKeyboard(tw, th, rect);
      break;
    case kbdAccent:
      debug(DEBUG_INFO, PALMOS_MODULE, "SysKeyboardGadgetCallback draw accent keyboard");
      DrawAlphaKeyboard(upper ? accent_upper : accent_lower, tw, th, rect);
      break;
    default:
      break;
  }
}

static Boolean SysKeyboardGadgetCallback(FormGadgetTypeInCallback *gad, UInt16 cmd, void *param) {
  kbd_module_t *module = (kbd_module_t *)thread_get(kbd_key);
  RectangleType rect;
  FormType *formP;
  UInt16 kbdGadgetIndex;

  if (cmd == formGadgetDeleteCmd) return true;

  formP = FrmGetActiveForm();
  kbdGadgetIndex = FrmGetObjectIndex(formP, 32010);
  FrmGetObjectBounds(formP, kbdGadgetIndex, &rect);

  switch (cmd) {
    case formGadgetDrawCmd:
      MemSet(&module->bounds[0], 256 * sizeof(RectangleType), 0);
      DrawKeyboard(module->kbd, module->upper, TW, TH, &rect);
      break;

    case formGadgetEraseCmd:
      WinEraseRectangle(&rect, 0);
      break;
  }

  return true;
}

static void KbdSetType(KeyboardType kbd) {
  kbd_module_t *module = (kbd_module_t *)thread_get(kbd_key);
  FormType *formP;
  RectangleType rect;
  UInt16 objIndex;

  module->kbd = kbd;

  if ((formP = FrmGetActiveForm()) != NULL) {
    if (FrmGetFormId(formP) == 32000) {
      objIndex = FrmGetObjectIndex(formP, 32010);
      FrmGetObjectBounds(formP, objIndex, &rect);
      MemSet(&module->bounds[0], 256 * sizeof(RectangleType), 0);
      DrawKeyboard(module->kbd, module->upper, TW, TH, &rect);
    }
  }
}

static Boolean SysKeyboardHandleEvent(EventType *eventP) {
  kbd_module_t *module = (kbd_module_t *)thread_get(kbd_key);
  FormType *formP;
  FieldType *kbdFldP;
  UInt16 kbdFieldIndex, i, pos;
  char c;
  Boolean handled = false;

  formP = FrmGetActiveForm();
  kbdFieldIndex = FrmGetObjectIndex(formP, 32003);
  kbdFldP = FrmGetObjectPtr(formP, kbdFieldIndex);

  switch (eventP->eType) {
    case frmOpenEvent:
      FrmDrawForm(formP);
      FldSetInsPtPosition(kbdFldP, module->pos);
      FldSetActiveField(kbdFldP);
      handled = true;
      break;
    case penDownEvent:
      module->sel = -1;
      for (i = 0; i < 256; i++) {
        if (module->bounds[i].extent.x > 0) {
          if (RctPtInRectangle(eventP->screenX, eventP->screenY, &module->bounds[i])) {
            module->sel = i;
            WinInvertRect(&module->bounds[i], 0);
            handled = true;
            break;
          }
        }
      }
      break;
    case penUpEvent:
      for (i = 0; i < 256 && !handled; i++) {
        if (module->bounds[i].extent.x > 0) {
          if (RctPtInRectangle(eventP->screenX, eventP->screenY, &module->bounds[i])) {
            if (i == module->sel) {
              WinInvertRect(&module->bounds[i], 0);
              switch (i) {
                case CODE_SHIFT:
                  module->upper = !module->upper;
                  module->cap = false;
                  KbdGrfSetState(module->upper ? GRAFFITI_SHIFT : 0);
                  break;
                case CODE_CAP:
                  module->cap = !module->cap;
                  module->upper = module->cap;
                  KbdGrfSetState(module->cap ? GRAFFITI_CAPS : 0);
                  break;
                case '\b':
                  pos = FldGetInsPtPosition(kbdFldP);
                  if (pos > 0) FldDelete(kbdFldP, pos-1, pos);
                  break;
                default:
                  c = i;
                  FldInsert(kbdFldP, &c, 1);
                  if (!module->cap && module->upper && module->kbd != kbdNumbersAndPunc) {
                    module->upper = false;
                    KbdGrfSetState(0);
                  }
                  break;
              }
            } else {
              module->sel = -1;
            }
            handled = true;
          }
        }
      }
      break;
    case ctlSelectEvent:
      switch (eventP->data.ctlSelect.controlID) {
        case 32005:
          KbdSetType(kbdAlpha);
          break;
        case 32006:
          KbdSetType(kbdNumbersAndPunc);
          break;
        case 32007:
          KbdSetType(kbdAccent);
          break;
        case 32008: // up
        case 32009: // down
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }

  return handled;
}

void SysKeyboardDialog(KeyboardType kbd) {
  kbd_module_t *module = (kbd_module_t *)thread_get(kbd_key);
  FormType *formP, *kbdFormP, *previous;
  FieldType *fldP, *kbdFldP;
  ControlType *typeControlP;
  MemHandle textHandle;
  UInt16 objIndex, kbdFieldIndex, typeControlID, typeControlIndex, kbdGadgetIndex;

  debug(DEBUG_INFO, PALMOS_MODULE, "SysKeyboardDialog begin");

  if ((formP = FrmGetActiveForm()) != NULL) {
    debug(DEBUG_INFO, PALMOS_MODULE, "SysKeyboardDialog active form ok");

    if ((objIndex = FrmGetFocus(formP)) != noFocus) {
      debug(DEBUG_INFO, PALMOS_MODULE, "SysKeyboardDialog get focus ok");

      if (FrmGetObjectType(formP, objIndex) == frmFieldObj) {
        debug(DEBUG_INFO, PALMOS_MODULE, "SysKeyboardDialog object type ok");

        if ((fldP = FrmGetObjectPtr(formP, objIndex)) != NULL) {
          debug(DEBUG_INFO, PALMOS_MODULE, "SysKeyboardDialog object ptr ok");
          textHandle = FldGetTextHandle(fldP);

          if (textHandle != NULL) {
            debug(DEBUG_INFO, PALMOS_MODULE, "SysKeyboardDialog text handle ok");

            if ((kbdFormP = FrmInitForm(32000)) != NULL) {
              debug(DEBUG_INFO, PALMOS_MODULE, "SysKeyboardDialog init form ok");
              FrmSetEventHandler(kbdFormP, SysKeyboardHandleEvent);

              switch (kbd) {
                case kbdAlpha:          typeControlID = 32005; break;
                case kbdNumbersAndPunc: typeControlID = 32006; break;
                case kbdAccent:         typeControlID = 32007; break;
                case kbdDefault:        typeControlID = 32005; kbd = kbdAlpha; break;
                default:                typeControlID = 32005; break;
                  break;
              }
              typeControlIndex = FrmGetObjectIndex(kbdFormP, typeControlID);
              typeControlP = FrmGetObjectPtr(kbdFormP, typeControlIndex);
              module->kbd = kbd;
              module->upper = false;
              module->cap = false;

              kbdFieldIndex = FrmGetObjectIndex(kbdFormP, 32003);
              kbdFldP = FrmGetObjectPtr(kbdFormP, kbdFieldIndex);

              kbdGadgetIndex = FrmGetObjectIndex(kbdFormP, 32010);
              FrmSetGadgetHandler(kbdFormP, kbdGadgetIndex, SysKeyboardGadgetCallback);
              previous = FrmGetActiveForm();

              if (typeControlP != NULL && kbdFldP != NULL) {
                CtlSetValue(typeControlP, 1);
                FldSetMaxChars(kbdFldP, FldGetMaxChars(fldP));
                FldSetTextHandle(kbdFldP, textHandle);
                module->pos = FldGetInsPtPosition(fldP);

                if (FrmDoDialog(kbdFormP) == 32002) { // "Done" button
                  debug(DEBUG_INFO, PALMOS_MODULE, "SysKeyboardDialog do dialog done button");
                  module->pos = FldGetInsPtPosition(kbdFldP);
                  textHandle = FldGetTextHandle(kbdFldP);
                  FldSetTextHandle(kbdFldP, NULL);
                  FldSetTextHandle(fldP, textHandle);
                } else {
                  debug(DEBUG_INFO, PALMOS_MODULE, "SysKeyboardDialog do dialog other button");
                  module->pos = FldGetInsPtPosition(kbdFldP);
                  FldSetTextHandle(kbdFldP, NULL);
                }
                FldSetInsPtPosition(fldP, module->pos);
                FldSetActiveField(fldP);
              }
              FrmDeleteForm(kbdFormP);
              FrmSetActiveForm(previous);
            }
          }
        }
      }
    }
  }
}

void SysKeyboardDialogV10(void) {
  SysKeyboardDialog(kbdDefault);
}

KeyboardStatus *KeyboardStatusNew(UInt16 keyboardID) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "KeyboardStatusNew not implemented");
  return NULL;
}

void KeyboardStatusFree(KeyboardStatus *ks) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "KeyboardStatusFree not implemented");
}

void KbdSetLayout(KeyboardStatus *ks, UInt16 layout) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "KbdSetLayout not implemented");
}

UInt16 KbdGetLayout(const KeyboardStatus *ks) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "KbdGetLayout not implemented");
  return 0;
}

void KbdSetPosition(KeyboardStatus *ks, const PointType *p) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "KbdSetPosition not implemented");
}

void KbdGetPosition(const KeyboardStatus *ks, PointType *p) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "KbdGetPosition not implemented");
}

void KbdSetShiftState(KeyboardStatus *ks, UInt16 shiftState) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "KbdSetShiftState not implemented");
}

UInt16 KbdGetShiftState(const KeyboardStatus *ks) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "KbdGetShiftState not implemented");
  return 0;
}

void KbdDraw(KeyboardStatus *ks, Boolean keyTopsOnly, Boolean ignoreModifiers) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "KbdDraw not implemented");
}

void KbdErase(KeyboardStatus *ks) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "KbdErase not implemented");
}

Boolean KbdHandleEvent(KeyboardStatus *ks, EventType * pEvent) {
  debug(DEBUG_ERROR, PALMOS_MODULE, "KbdHandleEvent not implemented");
  return false;
}

UInt16 KbdGrfGetState(void) {
  kbd_module_t *module = (kbd_module_t *)thread_get(kbd_key);
  return module->graffitiState;
}

void KbdGrfSetState(UInt16 state) {
  kbd_module_t *module = (kbd_module_t *)thread_get(kbd_key);
  FormType *formP;
  UInt16 objIndex, numObjects;

  pumpkin_dia_set_graffiti_state(state);
  module->graffitiState = state;

  switch (state) {
    case GRAFFITI_SHIFT:
      module->cap = false;
      module->upper = true;
      break;
    case GRAFFITI_CAPS:
      module->cap = true;
      module->upper = true;
      break;
    default:
      module->cap = false;
      module->upper = false;
      break;
  }

  if ((formP = FrmGetActiveForm()) != NULL) {
    numObjects = FrmGetNumberOfObjects(formP);
    for (objIndex = 0; objIndex < numObjects; objIndex++) {
      if (FrmGetObjectType(formP, objIndex) == frmGraffitiStateObj) {
        FrmDrawObject(formP, objIndex, false);
        break;
      }
    }
  }
}

Err KbdDrawKeyboard(KeyboardType kbd, Boolean upper, WinHandle wh, RectangleType *bounds) {
  kbd_module_t *module = (kbd_module_t *)thread_get(kbd_key);
  WinHandle prev;
  BitmapType *bmp;
  RectangleType rect;
  RGBColorType fg, bg;
  Coord bw, bh;
  UInt16 i;
  Err err = dmErrInvalidParam;

  if (wh && bounds) {
    MemSet(&module->bounds[0], 256 * sizeof(RectangleType), 0);

    prev = WinSetDrawWindow(wh);
    bmp = WinGetBitmap(wh);
    BmpGetDimensions(bmp, &bw, &bh, NULL);
    RctSetRectangle(&rect, 0, 0, bw, bh);
    if (BmpGetDensity(bmp) == kDensityDouble) {
      RctSetRectangle(&rect, 0, 0, bw/2, bh/2);
    }

    WinPushDrawState();
    dia_color(&fg, &bg);
    WinSetTextColorRGB(&fg, NULL);
    WinSetForeColorRGB(&fg, NULL);
    WinSetBackColorRGB(&bg, NULL);
    WinEraseRectangle(&rect, 0);
    module->bitmapBase = 32100;
    DrawKeyboard(kbd, upper, TW, TH, &rect);
    module->bitmapBase = 32000;
    WinPopDrawState();

    WinSetDrawWindow(prev);

    if (BmpGetDensity(bmp) == kDensityDouble) {
      MemSet(bounds, 256 * sizeof(RectangleType), 0);
      for (i = 0; i < 256; i++) {
        if (module->bounds[i].extent.x) {
          bounds[i].topLeft.x = module->bounds[i].topLeft.x * 2 - 1;
          bounds[i].topLeft.y = module->bounds[i].topLeft.y * 2 - 1;
          bounds[i].extent.x = module->bounds[i].extent.x * 2 + 1;
          bounds[i].extent.y = module->bounds[i].extent.y * 2 + 1;
        }
      }
    } else {
      MemMove(bounds, &module->bounds[0], 256 * sizeof(RectangleType));
    }
    err = errNone;
  }

  return err;
}
