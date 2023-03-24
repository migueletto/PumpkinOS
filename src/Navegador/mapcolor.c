#include <PalmOS.h>

#include "gps.h"
#include "app.h"
#include "main.h"
#include "map.h"
#include "mapdecl.h"
#include "misc.h"
#include "gui.h"

static Boolean PositionCallback(FormGadgetTypeInCallback *gad, UInt16 cmd, void *param);
static Boolean CurrentCallback(FormGadgetTypeInCallback *gad, UInt16 cmd, void *param);
static Boolean StoredCallback(FormGadgetTypeInCallback *gad, UInt16 cmd, void *param);
static Boolean SelectedCallback(FormGadgetTypeInCallback *gad, UInt16 cmd, void *param);
static Boolean TargetCallback(FormGadgetTypeInCallback *gad, UInt16 cmd, void *param);
static Boolean MapColorGadgetCallback(UInt16 id, UInt16 cmd, void *param);
static void DrawColor(IndexedColorType c, RectangleType *rect);

static AppPrefs *prefs;

Boolean MapColorFormHandleEvent(EventPtr event)
{
  FormPtr frm;
  Boolean handled;

  handled = false;

  switch (event->eType) {
    case frmOpenEvent:
      prefs = LoadPrefs();
      frm = FrmGetActiveForm();
      FrmSetGadgetHandler(frm, FrmGetObjectIndex(frm, poscolorCtl),
          PositionCallback);
      FrmSetGadgetHandler(frm, FrmGetObjectIndex(frm, curcolorCtl),
          CurrentCallback);
      FrmSetGadgetHandler(frm, FrmGetObjectIndex(frm, stocolorCtl),
          StoredCallback);
      FrmSetGadgetHandler(frm, FrmGetObjectIndex(frm, selcolorCtl),
          SelectedCallback);
      FrmSetGadgetHandler(frm, FrmGetObjectIndex(frm, tarcolorCtl),
          TargetCallback);
      FrmDrawForm(frm);
      handled = true;
      break;

    case nilEvent:
      idle();
      handled = true;
      break;

    case ctlSelectEvent:
      switch (event->data.ctlSelect.controlID) {
        case okBtn:
          SavePrefs();
          // fall-trough

        case cancelBtn: 
          PopForm();
          handled = true;
      }
      break;

    default:
      break;
  }

  return handled;
}

static Boolean PositionCallback(FormGadgetTypeInCallback *gad, UInt16 cmd, void *param)
{
  return MapColorGadgetCallback(poscolorCtl, cmd, param);
}

static Boolean CurrentCallback(FormGadgetTypeInCallback *gad, UInt16 cmd, void *param)
{
  return MapColorGadgetCallback(curcolorCtl, cmd, param);
}

static Boolean StoredCallback(FormGadgetTypeInCallback *gad, UInt16 cmd, void *param)
{
  return MapColorGadgetCallback(stocolorCtl, cmd, param);
}

static Boolean SelectedCallback(FormGadgetTypeInCallback *gad, UInt16 cmd, void *param)
{
  return MapColorGadgetCallback(selcolorCtl, cmd, param);
}

static Boolean TargetCallback(FormGadgetTypeInCallback *gad, UInt16 cmd, void *param)
{
  return MapColorGadgetCallback(tarcolorCtl, cmd, param);
}

static Boolean MapColorGadgetCallback(UInt16 id, UInt16 cmd, void *param)
{
  FormPtr frm;
  RGBColorType rgb;
  RectangleType rect;
  EventType *event;
  IndexedColorType c;
  Boolean ok;
  UInt16 index;

  if (cmd == formGadgetDeleteCmd)
    return true;

  frm = FrmGetActiveForm();

  index = FrmGetObjectIndex(frm, id);
  FrmGetObjectBounds(frm, index, &rect);
  
  switch (cmd) {
    case formGadgetDrawCmd:
      switch (id) {
        case poscolorCtl: DrawColor(prefs->positionColor, &rect); break;
        case curcolorCtl: DrawColor(prefs->currentColor, &rect); break;
        case stocolorCtl: DrawColor(prefs->storedColor, &rect); break;
        case selcolorCtl: DrawColor(prefs->selectedColor, &rect); break;
        case tarcolorCtl: DrawColor(prefs->targetColor, &rect); break;
        default: return true;
      }
      break;
    case formGadgetEraseCmd:
      WinEraseRectangle(&rect, 0);
      break;
    case formGadgetHandleEventCmd:
      event = (EventType *)param;
      if (event->eType == frmGadgetEnterEvent) {
        switch (id) {
          case poscolorCtl: c = prefs->positionColor; break;
          case curcolorCtl: c = prefs->currentColor; break;
          case stocolorCtl: c = prefs->storedColor; break;
          case selcolorCtl: c = prefs->selectedColor; break;
          case tarcolorCtl: c = prefs->targetColor; break;
          default: return true;
        }
        ok = false;
        //if (UIPickColor(&c, NULL, 0, NULL, NULL)) {
        WinIndexToRGB(c, &rgb);
        if (UIPickColor(&c, &rgb, UIPickColorStartPalette, NULL, NULL)) {
          DrawColor(c, &rect);
          MapInvalid();
          ok = true;
        }
        if (ok) {
          switch (id) {
            case poscolorCtl: prefs->positionColor = c; break;
            case curcolorCtl: prefs->currentColor = c; break;
            case stocolorCtl: prefs->storedColor = c; break;
            case selcolorCtl: prefs->selectedColor = c; break;
            case tarcolorCtl: prefs->targetColor = c; break;
          }
          MapColor(prefs->positionColor, prefs->currentColor,
            prefs->storedColor, prefs->selectedColor, prefs->targetColor);
        }
      }
  }

  return true;
}

static void DrawColor(IndexedColorType c, RectangleType *rect)
{
  RGBColorType rgb;
  IndexedColorType old;
  UInt16 x, y, dx, dy;

  x = rect->topLeft.x;
  y = rect->topLeft.y;
  dx = rect->extent.x;
  dy = rect->extent.y;

  old = WinSetBackColor(c);
  WinEraseRectangle(rect, 0);
  WinSetBackColor(old);

  rgb.r = 0;
  rgb.g = 0;
  rgb.b = 0;
  old = WinSetForeColor(WinRGBToIndex(&rgb));
  WinDrawLine(x, y, x+dx-1, y);
  WinDrawLine(x+dx-1, y, x+dx-1, y+dy-1);
  WinDrawLine(x+dx-1, y+dy-1, x, y+dy-1);
  WinDrawLine(x, y+dy-1, x, y);
  WinSetForeColor(old);
}
