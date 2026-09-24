#include <PalmOS.h>

#include "FormAccessor.h"
#include "debug.h"

#define ROD_MARGIN 2

void SclGetScrollBar(const ScrollBarType *bar, Int16 *valueP, Int16 *minP, Int16 *maxP, Int16 *pageSizeP) {
  if (bar) {
    if (valueP) *valueP = FrmObjectGetField((void *)bar, frmScrollBarObj, FormScrollBarFieldValue);
    if (minP) *minP = FrmObjectGetField((void *)bar, frmScrollBarObj, FormScrollBarFieldMinValue);
    if (maxP) *maxP = FrmObjectGetField((void *)bar, frmScrollBarObj, FormScrollBarFieldMaxValue);
    if (pageSizeP) *pageSizeP = FrmObjectGetField((void *)bar, frmScrollBarObj, FormScrollBarFieldPageSize);
    //debug(DEBUG_TRACE, "Scroll", "SclGetScrollBar value=%d, min=%d, max=%d, page=%d", bar->value, bar->minValue, bar->maxValue, bar->pageSize);
  }
}

void SclSetScrollBar(ScrollBarType *bar, Int16 value, Int16 min, Int16 max, Int16 pageSize) {
  Boolean redraw;

  if (bar && value >= 0 && min >= 0 && max >= min && pageSize >= 0) {
    debug(DEBUG_TRACE, "Scroll", "SclSetScrollBar value=%d, min=%d, max=%d, page=%d", value, min, max, pageSize);

    if (FrmObjectGetField(bar, frmScrollBarObj, FormScrollBarFieldValue) != value) {
      FrmObjectSetField(bar, frmScrollBarObj, FormScrollBarFieldValue, value);
      redraw = true;
    }
    if (FrmObjectGetField(bar, frmScrollBarObj, FormScrollBarFieldMinValue) != min) {
      FrmObjectSetField(bar, frmScrollBarObj, FormScrollBarFieldMinValue, min);
      redraw = true;
    }
    if (FrmObjectGetField(bar, frmScrollBarObj, FormScrollBarFieldMaxValue) != max) {
      FrmObjectSetField(bar, frmScrollBarObj, FormScrollBarFieldMaxValue, max);
      redraw = true;
    }
    if (FrmObjectGetField(bar, frmScrollBarObj, FormScrollBarFieldPageSize) != pageSize) {
      FrmObjectSetField(bar, frmScrollBarObj, FormScrollBarFieldPageSize, pageSize);
      redraw = true;
    }
    if (FrmObjectGetField(bar, frmScrollBarObj, FormScrollBarFieldMinValue) == 0 &&
        FrmObjectGetField(bar, frmScrollBarObj, FormScrollBarFieldMaxValue) == 0) {
      redraw = true;
    }

    if (redraw) {
      FrmObjectSetFlag(bar, frmScrollBarObj, FormScrollBarFieldAttr, ScrollBarFlagUsable, 1);
      debug(DEBUG_TRACE, "Scroll", "SclSetScrollBar must redraw");
      if (FrmObjectGetFlag(bar, frmScrollBarObj, FormScrollBarFieldAttr, ScrollBarFlagVisible)) {
        SclDrawScrollBar(bar);
      }
    }
  }
}

static void hline(int x1, int x2, int y) {
  if (x1 <= x2) {
    WinDrawLine(x1, y, x2, y);
  }
}

static void vline(int y1, int y2, int x) {
  if (y1 <= y2) {
    WinDrawLine(x, y1, x, y2);
  }
}
void SclDrawScrollBar(ScrollBarType *bar) {
  IndexedColorType formFill, scrollFore, oldb, oldf;
  RectangleType bounds, rect;
  RGBColorType rgb;
  Int16 value, minValue, maxValue, pageSize;
  UInt16 numPages, w, h, x, y, d;
  Boolean vertical;

  if (bar) {
    RctSetRectFromAddr(&bounds, bar, FormScrollBarFieldRectX);
    value = FrmObjectGetField(bar, frmScrollBarObj, FormScrollBarFieldValue);
    minValue = FrmObjectGetField(bar, frmScrollBarObj, FormScrollBarFieldMinValue);
    maxValue = FrmObjectGetField(bar, frmScrollBarObj, FormScrollBarFieldMaxValue);
    pageSize = FrmObjectGetField(bar, frmScrollBarObj, FormScrollBarFieldPageSize);

    vertical = bounds.extent.y > bounds.extent.x;

    debug(DEBUG_TRACE, "Scroll", "SclDrawScrollBar min=%d value=%d max=%d bounds=(%d,%d,%d,%d)",
      minValue, value, maxValue, bounds.topLeft.x, bounds.topLeft.y, bounds.extent.x, bounds.extent.y);
    formFill = UIColorGetTableEntryIndex(UIFormFill);
    oldb = WinSetBackColor(formFill);
    FrmObjectSetFlag(bar, frmScrollBarObj, FormScrollBarFieldAttr, ScrollBarFlagVisible, 0);

    if (minValue == 0 && maxValue == 0) {
      WinEraseRectangle(&bounds, 0);

    } else if (minValue < maxValue && pageSize > 0) {
      WinEraseRectangle(&bounds, 0);

      if (vertical) {
        d = bounds.extent.x >= 6 ? 4 : 2;

        // rod
        rgb.r = rgb.g = rgb.b = 192;
        WinSetBackColorRGB(&rgb, NULL);
        rect.topLeft.y = bounds.topLeft.y + d + 1;
        rect.extent.y  = bounds.extent.y - 2*d - 2;
        rect.topLeft.x = bounds.topLeft.x + ROD_MARGIN;
        rect.extent.x  = bounds.extent.x - 2*ROD_MARGIN;
        WinEraseRectangle(&rect, 0);

        rgb.r = rgb.g = rgb.b = 0;
        scrollFore = WinRGBToIndex(&rgb);
        oldf = WinSetForeColor(scrollFore);

        // top arrow
        y = bounds.topLeft.y+d;
        hline(bounds.topLeft.x+3, bounds.topLeft.x+bounds.extent.x-4, y-4);
        hline(bounds.topLeft.x+2, bounds.topLeft.x+bounds.extent.x-3, y-3);
        hline(bounds.topLeft.x+1, bounds.topLeft.x+bounds.extent.x-2, y-2);
        hline(bounds.topLeft.x+0, bounds.topLeft.x+bounds.extent.x-1, y-1);

        // bottom arrow
        y = bounds.topLeft.y+bounds.extent.y;
        hline(bounds.topLeft.x+0, bounds.topLeft.x+bounds.extent.x-1, y-4);
        hline(bounds.topLeft.x+1, bounds.topLeft.x+bounds.extent.x-2, y-3);
        hline(bounds.topLeft.x+2, bounds.topLeft.x+bounds.extent.x-3, y-2);
        hline(bounds.topLeft.x+3, bounds.topLeft.x+bounds.extent.x-4, y-1);

        // car
        numPages = (maxValue - minValue + 1 + pageSize - 1) / pageSize;
        h = (bounds.extent.y - 2*d - 2) / numPages;
        if (h < 1) h = 1;
        y = ((value - minValue) * (bounds.extent.y - 2*d - 2 - h)) / (maxValue - minValue);
        rect.topLeft.y = bounds.topLeft.y + d + 1 + y;
        rect.extent.y = h;
        rect.topLeft.x = bounds.topLeft.x + ROD_MARGIN;
        rect.extent.x = bounds.extent.x - 2*ROD_MARGIN;
        WinSetBackColor(scrollFore);
        WinEraseRectangle(&rect, 0);

      } else {
        d = bounds.extent.y >= 6 ? 4 : 2;

        // rod
        rgb.r = rgb.g = rgb.b = 192;
        WinSetBackColorRGB(&rgb, NULL);
        rect.topLeft.x = bounds.topLeft.x + d + 1;
        rect.extent.x  = bounds.extent.x - 2*d - 2;
        rect.topLeft.y = bounds.topLeft.y + ROD_MARGIN;
        rect.extent.y  = bounds.extent.y - 2*ROD_MARGIN;
        WinEraseRectangle(&rect, 0);
  
        rgb.r = rgb.g = rgb.b = 0;
        scrollFore = WinRGBToIndex(&rgb);
        oldf = WinSetForeColor(scrollFore);
  
        // left arrow
        x = bounds.topLeft.x+d; 
        vline(bounds.topLeft.y+3, bounds.topLeft.y+bounds.extent.y-4, x-4);
        vline(bounds.topLeft.y+2, bounds.topLeft.y+bounds.extent.y-3, x-3);
        vline(bounds.topLeft.y+1, bounds.topLeft.y+bounds.extent.y-2, x-2);
        vline(bounds.topLeft.y+0, bounds.topLeft.y+bounds.extent.y-1, x-1);

        // right arrow
        x = bounds.topLeft.x+bounds.extent.x;
        vline(bounds.topLeft.y+0, bounds.topLeft.y+bounds.extent.y-1, x-4);
        vline(bounds.topLeft.y+1, bounds.topLeft.y+bounds.extent.y-2, x-3);
        vline(bounds.topLeft.y+2, bounds.topLeft.y+bounds.extent.y-3, x-2);
        vline(bounds.topLeft.y+3, bounds.topLeft.y+bounds.extent.y-4, x-1);

        // car
        numPages = (maxValue - minValue + 1 + pageSize - 1) / pageSize;
        w = (bounds.extent.x - 2*d - 2) / numPages;
        if (w < 1) w = 1;
        x = ((value - minValue) * (bounds.extent.x - 2*d - 2 - w)) / (maxValue - minValue);
        rect.topLeft.x = bounds.topLeft.x + d + 1 + x;
        rect.extent.x = w;
        rect.topLeft.y = bounds.topLeft.y + ROD_MARGIN;
        rect.extent.y = bounds.extent.y - 2*ROD_MARGIN;
        WinSetBackColor(scrollFore);
        WinEraseRectangle(&rect, 0);
      }

      FrmObjectSetFlag(bar, frmScrollBarObj, FormScrollBarFieldAttr, ScrollBarFlagVisible, 1);
      debug(DEBUG_TRACE, "Scroll", "SclDrawScrollBar car %d,%d,%d,%d", rect.topLeft.x, rect.topLeft.y, rect.extent.x, rect.extent.y);

      WinSetForeColor(oldf);
    }
    WinSetBackColor(oldb);
    debug(DEBUG_TRACE, "Scroll", "SclDrawScrollBar end");
  }
}

static void SclAddRepeatEvent(ScrollBarType *bar, UInt16 value) {
  UInt16 id;
  Int16 currentValue;
  EventType event;

  id = FrmObjectGetField(bar, frmScrollBarObj, FormScrollBarFieldId);
  currentValue = FrmObjectGetField(bar, frmScrollBarObj, FormScrollBarFieldValue);
  debug(DEBUG_TRACE, "Scroll", "SclAddRepeatEvent value %d (%d)", value, currentValue);
  MemSet(&event, sizeof(EventType), 0);
  event.eType = sclRepeatEvent;
  event.data.sclRepeat.scrollBarID = id;
  event.data.sclRepeat.pScrollBar = bar;
  event.data.sclRepeat.value = currentValue; // XXX or value ?
  event.data.sclRepeat.newValue = value;
  event.data.sclRepeat.time = TimGetTicks();
  EvtAddEventToQueue(&event);
}

Boolean	SclHandleEvent(ScrollBarType *bar, const EventType *eventP) {
  EventType event;
  RectangleType bounds;
  UInt16 id, screenX, screenY, x, y, d;
  Int16 currentValue, minValue, maxValue;
  Int32 value;
  Boolean vertical, handled = false;

  if (FrmObjectGetFlag(bar, frmScrollBarObj, FormScrollBarFieldAttr, ScrollBarFlagVisible)) {
    id = FrmObjectGetField(bar, frmScrollBarObj, FormScrollBarFieldId);
    minValue = FrmObjectGetField(bar, frmScrollBarObj, FormScrollBarFieldMinValue);
    maxValue = FrmObjectGetField(bar, frmScrollBarObj, FormScrollBarFieldMaxValue);
    currentValue = FrmObjectGetField(bar, frmScrollBarObj, FormScrollBarFieldValue);
    RctSetRectFromAddr(&bounds, bar, FormScrollBarFieldRectX);
    screenX = eventP->screenX;
    screenY = eventP->screenY;

    vertical = bounds.extent.y > bounds.extent.x;
    if (vertical) {
      d = bounds.extent.x >= 6 ? 4 : 2;
    } else {
      d = bounds.extent.y >= 6 ? 4 : 2;
    }

    switch (eventP->eType) {
      case penDownEvent:
        if (FrmObjectGetFlag(bar, frmScrollBarObj, FormScrollBarFieldAttr, ScrollBarFlagUsable)) {
          if (RctPtInRectangle(eventP->screenX, eventP->screenY, &bounds)) {
            MemSet(&event, sizeof(EventType), 0);
            event.eType = sclEnterEvent;
            event.screenX = eventP->screenX;
            event.screenY = eventP->screenY;
            event.data.sclEnter.scrollBarID = id;
            event.data.sclEnter.pScrollBar = bar;
            EvtAddEventToQueue(&event);
            debug(DEBUG_TRACE, "Scroll", "SclHandleEvent scrollBar %d enter", id);

            // not sure hilighted is used for anything else, but it is necessary to
            // register if a penDown occurred inside the ScrollBar so that penUp
            // is handled correctly.
            FrmObjectSetFlag(bar, frmScrollBarObj, FormScrollBarFieldAttr, ScrollBarFlagHilighted, 1);

            handled = true;
          }
        }
        break;

      case penUpEvent:
        if (FrmObjectGetFlag(bar, frmScrollBarObj, FormScrollBarFieldAttr, ScrollBarFlagHilighted)) {
          if (vertical) {
            if (screenY < bounds.topLeft.y) {
              screenY = bounds.topLeft.y;
            } else if (screenY >= bounds.topLeft.y + bounds.extent.y) {
              screenY = bounds.topLeft.y + bounds.extent.y - 1;
            }
            y = screenY - bounds.topLeft.y;

            if (y >= d && y < bounds.extent.y - d) {
              value = ((y - d) * (maxValue - minValue + 1)) / (bounds.extent.y - 2*d);
              if (value > maxValue) value = maxValue;
              if (value < minValue) value = minValue;
            } else {
              value = currentValue;
            }
          } else {
            if (screenX < bounds.topLeft.x) {
              screenX = bounds.topLeft.x;
            } else if (screenX >= bounds.topLeft.x + bounds.extent.x) {
              screenX = bounds.topLeft.x + bounds.extent.x - 1;
            }
            x = screenX - bounds.topLeft.x;

            if (x >= d && x < bounds.extent.x - d) {
              value = ((x - d) * (maxValue - minValue + 1)) / (bounds.extent.x - 2*d);
              if (value > maxValue) value = maxValue;
              if (value < minValue) value = minValue;
            } else {
              value = currentValue;
            }
          }

          MemSet(&event, sizeof(EventType), 0);
          event.eType = sclExitEvent;
          event.screenX = screenX;
          event.screenY = screenY;
          event.data.sclExit.scrollBarID = id;
          event.data.sclExit.pScrollBar = bar;
          event.data.sclExit.value = value;
          event.data.sclExit.newValue = value;
          EvtAddEventToQueue(&event);
          handled = true;
        }
        FrmObjectSetFlag(bar, frmScrollBarObj, FormScrollBarFieldAttr, ScrollBarFlagHilighted, 0);
        break;

      case penMoveEvent:
        if (vertical) {
          if (screenY < bounds.topLeft.y) {
            screenY = bounds.topLeft.y;
          } else if (screenY >= bounds.topLeft.y + bounds.extent.y) {
            screenY = bounds.topLeft.y + bounds.extent.y - 1;
          }
        } else {
          if (screenX < bounds.topLeft.x) {
            screenX = bounds.topLeft.x;
          } else if (screenX >= bounds.topLeft.x + bounds.extent.x) {
            screenX = bounds.topLeft.x + bounds.extent.x - 1;
          }
        }
        // fall through

      case sclEnterEvent:
        debug(DEBUG_TRACE, "Scroll", "SclHandleEvent sclEnterEvent");
        MemSet(&event, sizeof(EventType), 0);
        event.screenX = screenX;
        event.screenY = screenY;

        if (vertical) {
          y = screenY - bounds.topLeft.y;
          if (y < d) {
            // top arrow
            debug(DEBUG_TRACE, "Scroll", "SclHandleEvent scrollBar %d top arrow", id);
            if (currentValue > minValue) {
              SclAddRepeatEvent(bar, currentValue-1);
            }
          } else if (y >= bounds.extent.y - d) {
            // bottom arrow
            debug(DEBUG_TRACE, "Scroll", "SclHandleEvent scrollBar %d bottom arrow", id);
            if (currentValue < maxValue) {
              SclAddRepeatEvent(bar, currentValue+1);
            }
          } else {
            // rod
            value = minValue + ((y - d) * (maxValue - minValue + 1)) / (bounds.extent.y - 2*d);
            if (value > maxValue) value = maxValue;
            if (value < minValue) value = minValue;
            debug(DEBUG_TRACE, "Scroll", "SclHandleEvent scrollBar %d rod value %d (%d)", id, value, currentValue);
            if (value != currentValue) {
              SclAddRepeatEvent(bar, value);
            }
          }
        } else {
          x = screenX - bounds.topLeft.x;
          if (x < d) {
            // left arrow
            debug(DEBUG_TRACE, "Scroll", "SclHandleEvent scrollBar %d left arrow", id);
            if (currentValue > minValue) {
              SclAddRepeatEvent(bar, currentValue-1);
            }
          } else if (x >= bounds.extent.x - d) {
            // right arrow
            debug(DEBUG_TRACE, "Scroll", "SclHandleEvent scrollBar %d right arrow", id);
            if (currentValue < maxValue) {
              SclAddRepeatEvent(bar, currentValue+1);
            }
          } else {
            // rod
            value = minValue + ((x - d) * (maxValue - minValue + 1)) / (bounds.extent.x - 2*d);
            if (value > maxValue) value = maxValue;
            if (value < minValue) value = minValue;
            debug(DEBUG_TRACE, "Scroll", "SclHandleEvent scrollBar %d rod value %d (%d)", id, value, currentValue);
            if (value != currentValue) {
              SclAddRepeatEvent(bar, value);
            }
          }
        }
        handled = true;
        break;

      case sclRepeatEvent:
        debug(DEBUG_TRACE, "Scroll", "SclHandleEvent sclRepeatEvent scrollBar %d value %d (%d)", id, eventP->data.sclRepeat.newValue, currentValue);
        if (eventP->data.sclRepeat.scrollBarID == id) {
          FrmObjectSetField(bar, frmScrollBarObj, FormScrollBarFieldValue, eventP->data.sclRepeat.newValue);
          debug(DEBUG_TRACE, "Scroll", "SclHandleEvent sclRepeatEvent draw %d", currentValue);
          SclDrawScrollBar(bar);
          handled = true;
        }
        break;

      default:
        break;
    }
  }

  return handled;
}
