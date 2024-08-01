#include <PalmOS.h>

#include "debug.h"

#define ROD_MARGIN 2

void SclGetScrollBar(const ScrollBarType *bar, Int16 *valueP, Int16 *minP, Int16 *maxP, Int16 *pageSizeP) {
  if (bar) {
    if (valueP) *valueP = bar->value;
    if (minP) *minP = bar->minValue;
    if (maxP) *maxP = bar->maxValue;
    if (pageSizeP) *pageSizeP = bar->pageSize;
    debug(DEBUG_TRACE, "Scroll", "SclGetScrollBar value=%d, min=%d, max=%d, page=%d", bar->value, bar->minValue, bar->maxValue, bar->pageSize);
  }
}

void SclSetScrollBar(ScrollBarType *bar, Int16 value, Int16 min, Int16 max, Int16 pageSize) {
  Boolean redraw;

  if (bar && value >= 0 && min >= 0 && max >= min && pageSize >= 0) {
    debug(DEBUG_TRACE, "Scroll", "SclSetScrollBar value=%d, min=%d, max=%d, page=%d", value, min, max, pageSize);

    if (bar->value != value) {
      bar->value = value;
      redraw = true;
    }
    if (bar->minValue != min) {
      bar->minValue = min;
      redraw = true;
    }
    if (bar->maxValue != max) {
      bar->maxValue = max;
      redraw = true;
    }
    if (bar->pageSize != pageSize) {
      bar->pageSize = pageSize;
      redraw = true;
    }

    if (redraw) {
      if (bar->attr.visible) {
        SclDrawScrollBar(bar);
      }
    }
  }
}

static void line(int x1, int y1, int x2, int y2) {
  if (x1 <= x2) {
    WinDrawLine(x1, y1, x2, y2);
  }
}

void SclDrawScrollBar(ScrollBarType *bar) {
  IndexedColorType formFill, scrollFore, oldb, oldf;
  RectangleType rect;
  RGBColorType rgb;
  UInt16 numPages, h, y, ah;

  if (bar) {
    debug(DEBUG_TRACE, "Scroll", "SclDrawScrollBar min=%d value=%d max=%d (%d,%d,%d,%d)", bar->minValue, bar->value, bar->maxValue, bar->bounds.topLeft.x, bar->bounds.topLeft.y, bar->bounds.extent.x, bar->bounds.extent.y);
    formFill = UIColorGetTableEntryIndex(UIFormFill);
    oldb = WinSetBackColor(formFill);
    WinEraseRectangle(&bar->bounds, 0);
    bar->attr.visible = false;

    if (bar->minValue < bar->maxValue && bar->pageSize > 0) {
      ah = bar->bounds.extent.x >= 6 ? 4 : 2;

      // rod
      rgb.r = rgb.g = rgb.b = 192;
      WinSetBackColorRGB(&rgb, NULL);
      rect.topLeft.y = bar->bounds.topLeft.y + ah + 1;
      rect.extent.y = bar->bounds.extent.y - 2*ah - 2;
      rect.topLeft.x = bar->bounds.topLeft.x + ROD_MARGIN;
      rect.extent.x = bar->bounds.extent.x - 2*ROD_MARGIN;
      WinEraseRectangle(&rect, 0);

      rgb.r = rgb.g = rgb.b = 0;
      scrollFore = WinRGBToIndex(&rgb);
      oldf = WinSetForeColor(scrollFore);

      // top arrow
      y = bar->bounds.topLeft.y+ah;
      line(bar->bounds.topLeft.x+3, y-4, bar->bounds.topLeft.x+bar->bounds.extent.x-4, y-4);
      line(bar->bounds.topLeft.x+2, y-3, bar->bounds.topLeft.x+bar->bounds.extent.x-3, y-3);
      line(bar->bounds.topLeft.x+1, y-2, bar->bounds.topLeft.x+bar->bounds.extent.x-2, y-2);
      line(bar->bounds.topLeft.x+0, y-1, bar->bounds.topLeft.x+bar->bounds.extent.x-1, y-1);

      // bottom arrow
      y = bar->bounds.topLeft.y+bar->bounds.extent.y;
      line(bar->bounds.topLeft.x+0, y-4, bar->bounds.topLeft.x+bar->bounds.extent.x-1, y-4);
      line(bar->bounds.topLeft.x+1, y-3, bar->bounds.topLeft.x+bar->bounds.extent.x-2, y-3);
      line(bar->bounds.topLeft.x+2, y-2, bar->bounds.topLeft.x+bar->bounds.extent.x-3, y-2);
      line(bar->bounds.topLeft.x+3, y-1, bar->bounds.topLeft.x+bar->bounds.extent.x-4, y-1);

      // car
      numPages = (bar->maxValue - bar->minValue + 1 + bar->pageSize) / bar->pageSize;
      h = (bar->bounds.extent.y - 2*ah - 2) / numPages;
      if (h < 1) h = 1;
      y = ((bar->value - bar->minValue) * (bar->bounds.extent.y - 2*ah - 2 - h)) / (bar->maxValue - bar->minValue);
      rect.topLeft.y = bar->bounds.topLeft.y + ah + 1 + y;
      rect.extent.y = h;
      rect.topLeft.x = bar->bounds.topLeft.x + ROD_MARGIN;
      rect.extent.x = bar->bounds.extent.x - 2*ROD_MARGIN;
      WinSetBackColor(scrollFore);
      WinEraseRectangle(&rect, 0);
      bar->attr.visible = true;

      WinSetForeColor(oldf);
    }
    WinSetBackColor(oldb);
    debug(DEBUG_TRACE, "Scroll", "SclDrawScrollBar end");
  }
}

static void SclAddRepeatEvent(ScrollBarType *bar, UInt16 value) {
  EventType event;

  debug(DEBUG_TRACE, "Scroll", "SclAddRepeatEvent value %d (%d)", value, bar->value);
  MemSet(&event, sizeof(EventType), 0);
  event.eType = sclRepeatEvent;
  event.data.sclRepeat.scrollBarID = bar->id;
  event.data.sclRepeat.pScrollBar = bar;
  event.data.sclRepeat.value = bar->value;
  event.data.sclRepeat.newValue = value;
  event.data.sclRepeat.time = TimGetTicks();
  EvtAddEventToQueue(&event);
}

Boolean	SclHandleEvent(ScrollBarType *bar, const EventType *eventP) {
  EventType event;
  UInt16 screenY, y, ah;
  Int32 value;
  Boolean handled = false;

  if (bar->attr.visible) {
    screenY = eventP->screenY;
    ah = bar->bounds.extent.x >= 6 ? 4 : 2;

    switch (eventP->eType) {
      case penDownEvent:
        if (bar->attr.usable && bar->attr.visible) {
          if (RctPtInRectangle(eventP->screenX, eventP->screenY, &bar->bounds)) {
            MemSet(&event, sizeof(EventType), 0);
            event.eType = sclEnterEvent;
            event.screenX = eventP->screenX;
            event.screenY = eventP->screenY;
            event.data.sclEnter.scrollBarID = bar->id;
            event.data.sclEnter.pScrollBar = bar;
            EvtAddEventToQueue(&event);
            debug(DEBUG_TRACE, "Scroll", "SclHandleEvent scrollBar %d enter", bar->id);

            // not sure hilighted is used for anything else, but it is necessary to
            // register if a penDown occurred inside the ScrollBar so that penUp
            // is handled correctly.
            bar->attr.hilighted = true;

            handled = true;
          }
        }
        break;

      case penUpEvent:
        if (bar->attr.hilighted) {
          if (screenY < bar->bounds.topLeft.y) {
            screenY = bar->bounds.topLeft.y;
          } else if (screenY >= bar->bounds.topLeft.y + bar->bounds.extent.y) {
            screenY = bar->bounds.topLeft.y + bar->bounds.extent.y - 1;
          }
          y = screenY - bar->bounds.topLeft.y;

          if (y >= ah && y < bar->bounds.extent.y - ah) {
            value = ((y - ah) * (bar->maxValue - bar->minValue + 1)) / (bar->bounds.extent.y - 2*ah);
            if (value > bar->maxValue) value = bar->maxValue;
            if (value < bar->minValue) value = bar->minValue;
          } else {
            value = bar->value;
          }

          MemSet(&event, sizeof(EventType), 0);
          event.eType = sclExitEvent;
          event.screenX = eventP->screenX;
          event.screenY = screenY;
          event.data.sclExit.scrollBarID = bar->id;
          event.data.sclExit.pScrollBar = bar;
          event.data.sclExit.value = value;
          event.data.sclExit.newValue = value;
          EvtAddEventToQueue(&event);
          handled = true;
        }
        bar->attr.hilighted = false;
        break;

      case penMoveEvent:
        if (screenY < bar->bounds.topLeft.y) {
          screenY = bar->bounds.topLeft.y;
        } else if (screenY >= bar->bounds.topLeft.y + bar->bounds.extent.y) {
          screenY = bar->bounds.topLeft.y + bar->bounds.extent.y - 1;
        }
        // fall through

      case sclEnterEvent:
        debug(DEBUG_TRACE, "Scroll", "SclHandleEvent sclEnterEvent");
        y = screenY - bar->bounds.topLeft.y;
        MemSet(&event, sizeof(EventType), 0);
        event.screenX = eventP->screenX;
        event.screenY = screenY;

        if (y < ah) {
          // top arrow
          debug(DEBUG_TRACE, "Scroll", "SclHandleEvent scrollBar %d top arrow", bar->id);
          if (bar->value > bar->minValue) {
            SclAddRepeatEvent(bar, bar->value-1);
          }
        } else if (y >= bar->bounds.extent.y - ah) {
          // bottom arrow
          debug(DEBUG_TRACE, "Scroll", "SclHandleEvent scrollBar %d bottom arrow", bar->id);
          if (bar->value < bar->maxValue) {
            SclAddRepeatEvent(bar, bar->value+1);
          }
        } else {
          // rod
          value = ((y - ah) * (bar->maxValue - bar->minValue + 1)) / (bar->bounds.extent.y - 2*ah);
          if (value > bar->maxValue) value = bar->maxValue;
          if (value < bar->minValue) value = bar->minValue;
          debug(DEBUG_TRACE, "Scroll", "SclHandleEvent scrollBar %d rod value %d (%d)", bar->id, value, bar->value);
          if (value != bar->value) {
            SclAddRepeatEvent(bar, value);
          }
        }
        handled = true;
        break;

      case sclRepeatEvent:
        debug(DEBUG_TRACE, "Scroll", "SclHandleEvent sclRepeatEvent scrollBar %d value %d (%d)", bar->id, eventP->data.sclRepeat.newValue, bar->value);
        if (eventP->data.sclRepeat.scrollBarID == bar->id) {
          bar->value = eventP->data.sclRepeat.newValue;
          debug(DEBUG_TRACE, "Scroll", "SclHandleEvent sclRepeatEvent draw %d", bar->value);
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
