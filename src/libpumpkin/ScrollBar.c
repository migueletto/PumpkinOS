#include <PalmOS.h>

#include "FormAccessor.h"
#include "debug.h"

#define ROD_MARGIN 2

void SclGetScrollBar(const ScrollBarType *bar, Int16 *valueP, Int16 *minP, Int16 *maxP, Int16 *pageSizeP) {
  if (bar) {
    //if (valueP) *valueP = bar->value;
    //if (minP) *minP = bar->minValue;
    //if (maxP) *maxP = bar->maxValue;
    //if (pageSizeP) *pageSizeP = bar->pageSize;
    //debug(DEBUG_TRACE, "Scroll", "SclGetScrollBar value=%d, min=%d, max=%d, page=%d", bar->value, bar->minValue, bar->maxValue, bar->pageSize);
    if (valueP) *valueP = FrmObjectGetField((void *)bar, frmScrollBarObj, FormScrollBarFieldValue);
    if (minP) *minP = FrmObjectGetField((void *)bar, frmScrollBarObj, FormScrollBarFieldMinValue);
    if (maxP) *maxP = FrmObjectGetField((void *)bar, frmScrollBarObj, FormScrollBarFieldMaxValue);
    if (pageSizeP) *pageSizeP = FrmObjectGetField((void *)bar, frmScrollBarObj, FormScrollBarFieldPageSize);
  }
}

void SclSetScrollBar(ScrollBarType *bar, Int16 value, Int16 min, Int16 max, Int16 pageSize) {
  Boolean redraw;

  if (bar && value >= 0 && min >= 0 && max >= min && pageSize >= 0) {
    debug(DEBUG_TRACE, "Scroll", "SclSetScrollBar value=%d, min=%d, max=%d, page=%d", value, min, max, pageSize);

/*
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
    if (bar->minValue == 0 && bar->maxValue == 0) {
      redraw = true;
    }
*/
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
      //bar->attr.usable = true;
      FrmObjectSetFlag(bar, frmScrollBarObj, FormScrollBarFieldAttr, ScrollBarFlagUsable, 1);
      debug(DEBUG_TRACE, "Scroll", "SclSetScrollBar must redraw");
      //if (bar->attr.visible)
      if (FrmObjectGetFlag(bar, frmScrollBarObj, FormScrollBarFieldAttr, ScrollBarFlagVisible)) {
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
  RectangleType bounds, rect;
  RGBColorType rgb;
  Int16 value, minValue, maxValue, pageSize;
  UInt16 numPages, h, y, ah;

  if (bar) {
    RctSetRectFromAddr(&bounds, bar, FormScrollBarFieldRectX);
    value = FrmObjectGetField(bar, frmScrollBarObj, FormScrollBarFieldValue);
    minValue = FrmObjectGetField(bar, frmScrollBarObj, FormScrollBarFieldMinValue);
    maxValue = FrmObjectGetField(bar, frmScrollBarObj, FormScrollBarFieldMaxValue);
    pageSize = FrmObjectGetField(bar, frmScrollBarObj, FormScrollBarFieldPageSize);

    //debug(DEBUG_TRACE, "Scroll", "SclDrawScrollBar min=%d value=%d max=%d bounds=(%d,%d,%d,%d)", bar->minValue, bar->value, bar->maxValue, bar->bounds.topLeft.x, bar->bounds.topLeft.y, bar->bounds.extent.x, bar->bounds.extent.y);
    formFill = UIColorGetTableEntryIndex(UIFormFill);
    oldb = WinSetBackColor(formFill);
    //bar->attr.visible = false;
    FrmObjectSetFlag(bar, frmScrollBarObj, FormScrollBarFieldAttr, ScrollBarFlagVisible, 0);

    //if (bar->minValue == 0 && bar->maxValue == 0)
    if (minValue == 0 && maxValue == 0) {
      //WinEraseRectangle(&bar->bounds, 0);
      WinEraseRectangle(&bounds, 0);

    //} else if (bar->minValue < bar->maxValue && bar->pageSize > 0)
    } else if (minValue < maxValue && pageSize > 0) {
      //WinEraseRectangle(&bar->bounds, 0);
      WinEraseRectangle(&bounds, 0);
      //ah = bar->bounds.extent.x >= 6 ? 4 : 2;
      ah = bounds.extent.x >= 6 ? 4 : 2;

      // rod
      rgb.r = rgb.g = rgb.b = 192;
      WinSetBackColorRGB(&rgb, NULL);
      //rect.topLeft.y = bar->bounds.topLeft.y + ah + 1;
      //rect.extent.y = bar->bounds.extent.y - 2*ah - 2;
      //rect.topLeft.x = bar->bounds.topLeft.x + ROD_MARGIN;
      //rect.extent.x = bar->bounds.extent.x - 2*ROD_MARGIN;
      rect.topLeft.y = bounds.topLeft.y + ah + 1;
      rect.extent.y = bounds.extent.y - 2*ah - 2;
      rect.topLeft.x = bounds.topLeft.x + ROD_MARGIN;
      rect.extent.x = bounds.extent.x - 2*ROD_MARGIN;
      WinEraseRectangle(&rect, 0);

      rgb.r = rgb.g = rgb.b = 0;
      scrollFore = WinRGBToIndex(&rgb);
      oldf = WinSetForeColor(scrollFore);

      // top arrow
      //y = bar->bounds.topLeft.y+ah;
      y = bounds.topLeft.y+ah;
      //line(bar->bounds.topLeft.x+3, y-4, bar->bounds.topLeft.x+bar->bounds.extent.x-4, y-4);
      //line(bar->bounds.topLeft.x+2, y-3, bar->bounds.topLeft.x+bar->bounds.extent.x-3, y-3);
      //line(bar->bounds.topLeft.x+1, y-2, bar->bounds.topLeft.x+bar->bounds.extent.x-2, y-2);
      //line(bar->bounds.topLeft.x+0, y-1, bar->bounds.topLeft.x+bar->bounds.extent.x-1, y-1);
      line(bounds.topLeft.x+3, y-4, bounds.topLeft.x+bounds.extent.x-4, y-4);
      line(bounds.topLeft.x+2, y-3, bounds.topLeft.x+bounds.extent.x-3, y-3);
      line(bounds.topLeft.x+1, y-2, bounds.topLeft.x+bounds.extent.x-2, y-2);
      line(bounds.topLeft.x+0, y-1, bounds.topLeft.x+bounds.extent.x-1, y-1);

      // bottom arrow
      //y = bar->bounds.topLeft.y+bar->bounds.extent.y;
      y = bounds.topLeft.y+bounds.extent.y;
      //line(bar->bounds.topLeft.x+0, y-4, bar->bounds.topLeft.x+bar->bounds.extent.x-1, y-4);
      //line(bar->bounds.topLeft.x+1, y-3, bar->bounds.topLeft.x+bar->bounds.extent.x-2, y-3);
      //line(bar->bounds.topLeft.x+2, y-2, bar->bounds.topLeft.x+bar->bounds.extent.x-3, y-2);
      //line(bar->bounds.topLeft.x+3, y-1, bar->bounds.topLeft.x+bar->bounds.extent.x-4, y-1);
      line(bounds.topLeft.x+0, y-4, bounds.topLeft.x+bounds.extent.x-1, y-4);
      line(bounds.topLeft.x+1, y-3, bounds.topLeft.x+bounds.extent.x-2, y-3);
      line(bounds.topLeft.x+2, y-2, bounds.topLeft.x+bounds.extent.x-3, y-2);
      line(bounds.topLeft.x+3, y-1, bounds.topLeft.x+bounds.extent.x-4, y-1);

      // car
      //numPages = (bar->maxValue - bar->minValue + 1 + bar->pageSize - 1) / bar->pageSize;
      numPages = (maxValue - minValue + 1 + pageSize - 1) / pageSize;
      //h = (bar->bounds.extent.y - 2*ah - 2) / numPages;
      h = (bounds.extent.y - 2*ah - 2) / numPages;
      if (h < 1) h = 1;
      //y = ((bar->value - bar->minValue) * (bar->bounds.extent.y - 2*ah - 2 - h)) / (bar->maxValue - bar->minValue);
      y = ((value - minValue) * (bounds.extent.y - 2*ah - 2 - h)) / (maxValue - minValue);
      //rect.topLeft.y = bar->bounds.topLeft.y + ah + 1 + y;
      rect.topLeft.y = bounds.topLeft.y + ah + 1 + y;
      rect.extent.y = h;
      //rect.topLeft.x = bar->bounds.topLeft.x + ROD_MARGIN;
      //rect.extent.x = bar->bounds.extent.x - 2*ROD_MARGIN;
      rect.topLeft.x = bounds.topLeft.x + ROD_MARGIN;
      rect.extent.x = bounds.extent.x - 2*ROD_MARGIN;
      WinSetBackColor(scrollFore);
      WinEraseRectangle(&rect, 0);
      //bar->attr.visible = true;
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
  //event.data.sclRepeat.scrollBarID = bar->id;
  event.data.sclRepeat.scrollBarID = id;
  event.data.sclRepeat.pScrollBar = bar;
  //event.data.sclRepeat.value = value;
  event.data.sclRepeat.value = currentValue;
  event.data.sclRepeat.newValue = value;
  event.data.sclRepeat.time = TimGetTicks();
  EvtAddEventToQueue(&event);
}

Boolean	SclHandleEvent(ScrollBarType *bar, const EventType *eventP) {
  EventType event;
  RectangleType bounds;
  UInt16 id, screenY, y, ah;
  Int16 currentValue, minValue, maxValue;
  Int32 value;
  Boolean handled = false;

  //if (bar->attr.visible)
  if (FrmObjectGetFlag(bar, frmScrollBarObj, FormScrollBarFieldAttr, ScrollBarFlagVisible)) {
    id = FrmObjectGetField(bar, frmScrollBarObj, FormScrollBarFieldId);
    minValue = FrmObjectGetField(bar, frmScrollBarObj, FormScrollBarFieldMinValue);
    maxValue = FrmObjectGetField(bar, frmScrollBarObj, FormScrollBarFieldMaxValue);
    currentValue = FrmObjectGetField(bar, frmScrollBarObj, FormScrollBarFieldValue);
    RctSetRectFromAddr(&bounds, bar, FormScrollBarFieldRectX);
    screenY = eventP->screenY;
    //ah = bar->bounds.extent.x >= 6 ? 4 : 2;
    ah = bounds.extent.x >= 6 ? 4 : 2;

    switch (eventP->eType) {
      case penDownEvent:
        //if (bar->attr.usable)
        if (FrmObjectGetFlag(bar, frmScrollBarObj, FormScrollBarFieldAttr, ScrollBarFlagUsable)) {
          //if (RctPtInRectangle(eventP->screenX, eventP->screenY, &bar->bounds))
          if (RctPtInRectangle(eventP->screenX, eventP->screenY, &bounds)) {
            MemSet(&event, sizeof(EventType), 0);
            event.eType = sclEnterEvent;
            event.screenX = eventP->screenX;
            event.screenY = eventP->screenY;
            //event.data.sclEnter.scrollBarID = bar->id;
            event.data.sclEnter.scrollBarID = id;
            event.data.sclEnter.pScrollBar = bar;
            EvtAddEventToQueue(&event);
            //debug(DEBUG_TRACE, "Scroll", "SclHandleEvent scrollBar %d enter", bar->id);
            debug(DEBUG_TRACE, "Scroll", "SclHandleEvent scrollBar %d enter", id);

            // not sure hilighted is used for anything else, but it is necessary to
            // register if a penDown occurred inside the ScrollBar so that penUp
            // is handled correctly.
            //bar->attr.hilighted = true;
            FrmObjectSetFlag(bar, frmScrollBarObj, FormScrollBarFieldAttr, ScrollBarFlagHilighted, 1);

            handled = true;
          }
        }
        break;

      case penUpEvent:
        //if (bar->attr.hilighted)
        if (FrmObjectGetFlag(bar, frmScrollBarObj, FormScrollBarFieldAttr, ScrollBarFlagHilighted)) {
          //if (screenY < bar->bounds.topLeft.y)
          if (screenY < bounds.topLeft.y) {
            //screenY = bar->bounds.topLeft.y;
            screenY = bounds.topLeft.y;
          //} else if (screenY >= bar->bounds.topLeft.y + bar->bounds.extent.y)
          } else if (screenY >= bounds.topLeft.y + bounds.extent.y) {
            //screenY = bar->bounds.topLeft.y + bar->bounds.extent.y - 1;
            screenY = bounds.topLeft.y + bounds.extent.y - 1;
          }
          //y = screenY - bar->bounds.topLeft.y;
          y = screenY - bounds.topLeft.y;

          //if (y >= ah && y < bar->bounds.extent.y - ah)
          if (y >= ah && y < bounds.extent.y - ah) {
            //value = ((y - ah) * (bar->maxValue - bar->minValue + 1)) / (bar->bounds.extent.y - 2*ah);
            value = ((y - ah) * (maxValue - minValue + 1)) / (bounds.extent.y - 2*ah);
            //if (value > bar->maxValue) value = bar->maxValue;
            if (value > maxValue) value = maxValue;
            //if (value < bar->minValue) value = bar->minValue;
            if (value < minValue) value = minValue;
          } else {
            //value = bar->value;
            value = currentValue;
          }

          MemSet(&event, sizeof(EventType), 0);
          event.eType = sclExitEvent;
          event.screenX = eventP->screenX;
          event.screenY = screenY;
          //event.data.sclExit.scrollBarID = bar->id;
          event.data.sclExit.scrollBarID = id;
          event.data.sclExit.pScrollBar = bar;
          event.data.sclExit.value = value;
          event.data.sclExit.newValue = value;
          EvtAddEventToQueue(&event);
          handled = true;
        }
        //bar->attr.hilighted = false;
        FrmObjectSetFlag(bar, frmScrollBarObj, FormScrollBarFieldAttr, ScrollBarFlagHilighted, 0);
        break;

      case penMoveEvent:
        //if (screenY < bar->bounds.topLeft.y)
        if (screenY < bounds.topLeft.y) {
          //screenY = bar->bounds.topLeft.y;
          screenY = bounds.topLeft.y;
        //} else if (screenY >= bar->bounds.topLeft.y + bar->bounds.extent.y)
        } else if (screenY >= bounds.topLeft.y + bounds.extent.y) {
          //screenY = bar->bounds.topLeft.y + bar->bounds.extent.y - 1;
          screenY = bounds.topLeft.y + bounds.extent.y - 1;
        }
        // fall through

      case sclEnterEvent:
        debug(DEBUG_TRACE, "Scroll", "SclHandleEvent sclEnterEvent");
        //y = screenY - bar->bounds.topLeft.y;
        y = screenY - bounds.topLeft.y;
        MemSet(&event, sizeof(EventType), 0);
        event.screenX = eventP->screenX;
        event.screenY = screenY;

        if (y < ah) {
          // top arrow
          debug(DEBUG_TRACE, "Scroll", "SclHandleEvent scrollBar %d top arrow", id);
          //if (bar->value > bar->minValue)
          if (currentValue > minValue) {
            //SclAddRepeatEvent(bar, bar->value-1);
            SclAddRepeatEvent(bar, currentValue-1);
          }
        //} else if (y >= bar->bounds.extent.y - ah)
        } else if (y >= bounds.extent.y - ah) {
          // bottom arrow
          debug(DEBUG_TRACE, "Scroll", "SclHandleEvent scrollBar %d bottom arrow", id);
          //if (bar->value < bar->maxValue)
          if (currentValue < maxValue) {
            //SclAddRepeatEvent(bar, bar->value+1);
            SclAddRepeatEvent(bar, currentValue+1);
          }
        } else {
          // rod
          //value = bar->minValue + ((y - ah) * (bar->maxValue - bar->minValue + 1)) / (bar->bounds.extent.y - 2*ah);
          value = minValue + ((y - ah) * (maxValue - minValue + 1)) / (bounds.extent.y - 2*ah);
          //if (value > bar->maxValue) value = bar->maxValue;
          //if (value < bar->minValue) value = bar->minValue;
          if (value > maxValue) value = maxValue;
          if (value < minValue) value = minValue;
          //debug(DEBUG_TRACE, "Scroll", "SclHandleEvent scrollBar %d rod value %d (%d)", bar->id, value, bar->value);
          debug(DEBUG_TRACE, "Scroll", "SclHandleEvent scrollBar %d rod value %d (%d)", id, value, currentValue);
          //if (value != bar->value)
          if (value != currentValue) {
            SclAddRepeatEvent(bar, value);
          }
        }
        handled = true;
        break;

      case sclRepeatEvent:
        debug(DEBUG_TRACE, "Scroll", "SclHandleEvent sclRepeatEvent scrollBar %d value %d (%d)", id, eventP->data.sclRepeat.newValue, currentValue);
        //if (eventP->data.sclRepeat.scrollBarID == bar->id)
        if (eventP->data.sclRepeat.scrollBarID == id) {
          //bar->value = eventP->data.sclRepeat.newValue;
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
