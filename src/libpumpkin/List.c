#include <PalmOS.h>

#include "sys.h"
#include "pwindow.h"
#include "vfs.h"
#include "bytes.h"
#include "logtrap.h"
#include "emupalmosinc.h"
#include "pumpkin.h"
#include "debug.h"
#include "xalloc.h"

static void LstDrawItem(ListType *listP, Int16 i, Boolean selected) {
  WinHandle oldw, oldd;
  RectangleType rect;
  IndexedColorType objFrame, objFore, objFill, objSelFore, objSelFill, oldf, oldb, oldt;
  FontID old;
  Int16 j, y, dy;
  uint32_t w, *aux32;
  char *s;

  oldw = WinGetActiveWindow();
  WinSetActiveWindow(listP->popupWin);
  oldd = WinSetDrawWindow(listP->popupWin);

  objFrame = UIColorGetTableEntryIndex(UIObjectFrame);
  objFore = UIColorGetTableEntryIndex(UIObjectForeground);
  objFill = UIColorGetTableEntryIndex(UIFormFill);
  objSelFore = UIColorGetTableEntryIndex(UIObjectSelectedForeground);
  objSelFill = UIColorGetTableEntryIndex(UIObjectSelectedFill);

  old = FntSetFont(listP->font);
  dy = FntCharHeight();

  j = i + listP->topItem;
  y = i * dy + 1;

  if (selected) {
    debug(DEBUG_TRACE, "List", "LstDrawItem list %d item %d on", listP->id, j);
    oldb = WinSetBackColor(objSelFill);
    oldt = WinSetTextColor(objSelFore);
  } else {
    debug(DEBUG_TRACE, "List", "LstDrawItem list %d item %d off", listP->id, j);
    oldb = WinSetBackColor(objFill);
    oldt = WinSetTextColor(objFore);
  }

  oldf = WinSetForeColor(objFrame);
  RctSetRectangle(&rect, 1, y, listP->bounds.extent.x-2, dy);
  WinEraseRectangle(&rect, 0);

  if (j < listP->numItems) {
    if (listP->m68k_drawfunc) {
      RctSetRectangle(&rect, 1, y, listP->bounds.extent.x-2, dy);
      CallListDrawItem(listP->m68k_drawfunc, j, &rect, listP->itemsText);
    } else if (listP->drawItemsCallback) {
      RctSetRectangle(&rect, 1, y, listP->bounds.extent.x-2, dy);
      listP->drawItemsCallback(j, &rect, listP->itemsText);
    } else {
      if (listP->itemsText) {
        if (pumpkin_is_m68k()) {
          aux32 = (uint32_t *)listP->itemsText;
          if (aux32[j]) {
            get4b(&w, (uint8_t *)(&aux32[j]), 0);
            s = (char *)(emupalmos_ram() + w);
            WinDrawChars(s, StrLen(s), 2, y);
          }
        } else {
          if (listP->itemsText[j]) {
            WinDrawChars(listP->itemsText[j], StrLen(listP->itemsText[j]), 2, y);
          }
        }
      }
    }
  }

  if (i == 0 && listP->topItem > 0) {
    // draw arrow up at upper right
    FntSetFont(symbolFont);
    WinDrawChar(8, listP->bounds.extent.x-FntCharWidth(8)-1, 1);
    FntSetFont(listP->font);
  } else if (i == listP->visibleItems-1 && (listP->numItems - listP->topItem) > listP->visibleItems) {
    // draw arrow down at bottom right
    FntSetFont(symbolFont);
    WinDrawChar(7, listP->bounds.extent.x-FntCharWidth(7)-1, y+1);
    FntSetFont(listP->font);
  }

  FntSetFont(old);
  WinSetForeColor(oldf);
  WinSetBackColor(oldb);
  WinSetTextColor(oldt);
  WinSetActiveWindow(oldw);
  WinSetDrawWindow(oldd);
}

void LstDrawList(ListType *listP) {
  WinHandle oldw, oldd;
  IndexedColorType objFrame, oldf;
  int i, w, h;

  if (listP->bitsBehind && !listP->attr.poppedUp) return;

  oldw = WinGetActiveWindow();
  WinSetActiveWindow(listP->popupWin);
  oldd = WinSetDrawWindow(listP->popupWin);

  // save the area behing the list
  if (!listP->attr.visible && listP->bitsBehind) {
    debug(DEBUG_TRACE, "List", "LstDrawList WinSaveRectangle");
    WinSaveRectangle(listP->bitsBehind, &listP->popupWin->windowBounds);
  }

  // draw items
  for (i = 0; i < listP->visibleItems; i++) {
    LstDrawItem(listP, i, (i + listP->topItem == listP->currentItem));
  }

  // draw border around list
  w = listP->bounds.extent.x;
  h = listP->bounds.extent.y;
  debug(DEBUG_TRACE, "List", "LstDrawList list %d w %d h %d", listP->id, w, h);
  objFrame = UIColorGetTableEntryIndex(UIObjectFrame);
  oldf = WinSetForeColor(objFrame);

  WinDrawLine(0,   0,   w-1, 0);
  WinDrawLine(0,   h-1, w-1, h-1);
  WinDrawLine(0,   0,   0,   h-1);
  WinDrawLine(w-1, 0,   w-1, h-1);
  WinSetForeColor(oldf);
  WinSetActiveWindow(oldw);
  WinSetDrawWindow(oldd);

  listP->attr.visible = 1;
}

void LstEraseList(ListType *listP) {
  if (listP->bitsBehind && !listP->attr.poppedUp) return;

  if (listP->attr.visible) {
    debug(DEBUG_TRACE, "List", "LstEraseList list %d", listP->id);
    if (listP->bitsBehind) {
      debug(DEBUG_TRACE, "List", "LstEraseList WinRestoreRectangle");
      WinRestoreRectangle(listP->bitsBehind, &listP->popupWin->windowBounds);
    }
    listP->attr.visible = 0;
  }
}

Int16 LstGetSelection(const ListType *listP) {
  return listP ? listP->currentItem : noListSelection;
}

Char *LstGetSelectionText(const ListType *listP, Int16 itemNum) {
  uint32_t w, *aux32;
  Char *s = NULL;

  if (listP && itemNum >= 0 && itemNum < listP->numItems && listP->itemsText) {
    if (pumpkin_is_m68k()) {
      aux32 = (uint32_t *)listP->itemsText;
      get4b(&w, (uint8_t *)(&aux32[itemNum]), 0);
      if (w) s = (char *)(emupalmos_ram() + w);
    } else {
      s = listP->itemsText[itemNum];
    }
  }

  return s;
}

Boolean LstHandleEvent(ListType *listP, const EventType *eventP) {
  EventType event;
  RectangleType rect, upArrow, downArrow;
  FormType *frm;
  FontID old;
  UInt16 h, dy, index;
  Boolean insideUpArrow, insideDownArrow;
  Boolean handled = false;

  rect.extent.x = listP->bounds.extent.x;
  rect.extent.y = listP->bounds.extent.y;

  if (listP->bitsBehind) {
    // list is a popup, pen events are relative to the list window
    rect.topLeft.x = 0;
    rect.topLeft.y = 0;
  } else {
    // list is not a popup, pen events are relative to the form window
    rect.topLeft.x = listP->bounds.topLeft.x;
    rect.topLeft.y = listP->bounds.topLeft.y;
  }

  old = FntSetFont(listP->font);
  dy = FntCharHeight();
  FntSetFont(old);
  RctSetRectangle(&upArrow, rect.topLeft.x + rect.extent.x - 10, rect.topLeft.y + 1, 10, dy);
  RctSetRectangle(&downArrow, rect.topLeft.x + rect.extent.x - 10, rect.topLeft.y + 1 + (listP->visibleItems - 1) * dy, 10, dy);
  insideUpArrow = listP->topItem > 0 && RctPtInRectangle(eventP->screenX, eventP->screenY, &upArrow);
  insideDownArrow = (listP->topItem + listP->visibleItems) < listP->numItems && RctPtInRectangle(eventP->screenX, eventP->screenY, &downArrow);

  switch (eventP->eType) {
    case penDownEvent:
      if (listP->attr.visible) {
        debug(DEBUG_TRACE, "List", "LstHandleEvent penDown list is visible");
        if (RctPtInRectangle(eventP->screenX, eventP->screenY, &rect)) {
          debug(DEBUG_TRACE, "List", "LstHandleEvent penDown inside list %d", listP->id);
          MemSet(&event, sizeof(EventType), 0);
          event.eType = lstEnterEvent;
          event.data.lstEnter.listID = listP->id;
          event.data.lstEnter.pList = listP;
          EvtAddEventToQueue(&event);
          handled = true;

          if (!insideUpArrow && !insideDownArrow) {
            // has to be done here otherwise it would not be visible on time
            if (listP->currentItem != noListSelection) {
              LstDrawItem(listP, listP->currentItem - listP->topItem, false);
            }
            h = rect.extent.y / listP->visibleItems;
            LstDrawItem(listP, (eventP->screenY - rect.topLeft.y) / h, true);
          }

        } else {
          if (listP->bitsBehind) {
            debug(DEBUG_TRACE, "List", "LstHandleEvent penDown outside list %d", listP->id);
            MemSet(&event, sizeof(EventType), 0);
            event.eType = lstExitEvent;
            event.data.lstExit.listID = listP->id;
            event.data.lstExit.pList = listP;
            EvtAddEventToQueue(&event);
            handled = true;
          }
        }
      }
      break;
    case penUpEvent:
      if (listP->attr.visible) {
        debug(DEBUG_TRACE, "List", "LstHandleEvent penUp list is visible");

        if (insideUpArrow) {
          debug(DEBUG_TRACE, "List", "LstHandleEvent list %d up arrow clicked", listP->id);
          listP->topItem -= listP->visibleItems;
          if (listP->topItem < 0) {
            listP->topItem = 0;
          }
          LstDrawList(listP);
          handled = true;

        } else if (insideDownArrow) {
          debug(DEBUG_TRACE, "List", "LstHandleEvent list %d down arrow clicked", listP->id);
          listP->topItem += listP->visibleItems;
          if ((listP->topItem + listP->visibleItems) >= listP->numItems) {
            listP->topItem = listP->numItems - listP->visibleItems;
          }
          LstDrawList(listP);
          handled = true;
        }

        if (!handled) {
          if (RctPtInRectangle(eventP->screenX, eventP->screenY, &rect)) {
            h = rect.extent.y / listP->visibleItems;

            if (listP->controlID) {
              frm = (FormType *)listP->formP;
              index = FrmGetObjectIndex(frm, listP->controlID);
              MemSet(&event, sizeof(EventType), 0);
              event.eType = popSelectEvent;
              event.data.popSelect.listID = listP->id;
              event.data.popSelect.listP = listP;
              event.data.popSelect.selection = listP->topItem + (eventP->screenY - rect.topLeft.y) / h;
              event.data.popSelect.priorSelection = listP->currentItem;
              event.data.popSelect.controlID = listP->controlID;
              event.data.popSelect.controlP = FrmGetObjectPtr(frm, index);
              EvtAddEventToQueue(&event);
              debug(DEBUG_TRACE, "List", "LstHandleEvent penUp popSelect %d", event.data.popSelect.selection);
              listP->currentItem = event.data.popSelect.selection;
              handled = true;

            } else {
              MemSet(&event, sizeof(EventType), 0);
              event.eType = lstSelectEvent;
              event.data.lstSelect.listID = listP->id;
              event.data.lstSelect.pList = listP;
              event.data.lstSelect.selection = listP->topItem + (eventP->screenY - rect.topLeft.y) / h;
              EvtAddEventToQueue(&event);
              listP->currentItem = event.data.lstSelect.selection;
              if (listP->currentItem >= listP->numItems) {
                listP->currentItem = listP->numItems ? listP->numItems-1 : 0;
              }
              debug(DEBUG_TRACE, "List", "LstHandleEvent penUp listSelect %d", listP->currentItem);
              handled = true;
            }

          } else {
            if (listP->bitsBehind) {
              debug(DEBUG_TRACE, "List", "LstHandleEvent penUp outside list %d", listP->id);
              MemSet(&event, sizeof(EventType), 0);
              event.eType = lstExitEvent;
              event.data.lstExit.listID = listP->id;
              event.data.lstExit.pList = listP;
              EvtAddEventToQueue(&event);
              handled = true;
            }
          }
        }
      }

      break;
    case lstEnterEvent:
      debug(DEBUG_TRACE, "List", "LstHandleEvent lstEnter");
      if (eventP->data.lstEnter.listID == listP->id) {
        debug(DEBUG_TRACE, "List", "LstHandleEvent lstEnter list id %d", listP->id);
      }
      break;
    case lstSelectEvent:
      debug(DEBUG_TRACE, "List", "LstHandleEvent lstSelect");
      if (eventP->data.lstSelect.listID == listP->id) {
        debug(DEBUG_TRACE, "List", "LstHandleEvent lstSelect list id %d", listP->id);
      }
      break;
    case popSelectEvent:
      debug(DEBUG_TRACE, "List", "LstHandleEvent popSelect");
      if (eventP->data.popSelect.listID == listP->id) {
        debug(DEBUG_TRACE, "List", "LstHandleEvent popSelect list id %d", listP->id);
      }
      break;
    case lstExitEvent:
      debug(DEBUG_TRACE, "List", "LstHandleEvent lstExit");
      if (eventP->data.lstExit.listID == listP->id) {
        debug(DEBUG_TRACE, "List", "LstHandleEvent lstExit list id %d", listP->id);
      }
      break;
    case keyDownEvent:
      if (listP->attr.visible && listP->bitsBehind && listP->attr.search) {
        // XXX uses up to 5 chars to do incremental search to navigate the list
      }
      break;
    default:
      break;
  }

  return handled;
}

static void LstAdjust(ListType *listP) {
  FormType *formP;
  Int16 th, fh;
  Coord w, h;
  FontID old;
  Err err;

  if (listP) {
    LstEraseList(listP);

    formP = (FormType *)listP->formP;
    old = FntSetFont(listP->font);
    th = FntCharHeight();
    FntSetFont(old);

    w = listP->bounds.extent.x;
    if (w > formP->window.windowBounds.extent.x) w = formP->window.windowBounds.extent.x;

    if (listP->bitsBehind) {
      h = listP->visibleItems * th + 2;
      debug(DEBUG_TRACE, "List", "LstAdjust %d visible=%d h=%d", listP->id, listP->visibleItems, h);
      //fh = 160;
      fh = formP->window.windowBounds.extent.y;
      if (h > fh) {
        listP->visibleItems = (fh - 2) / th;
        h = listP->visibleItems * th + 2;
        debug(DEBUG_TRACE, "List", "LstAdjust %d visible=%d th=%d h=%d", listP->id, listP->visibleItems, th, h);
      }
    } else {
      h = listP->bounds.extent.y;
      debug(DEBUG_TRACE, "List", "LstAdjust %d visible=%d h=%d (static)", listP->id, listP->visibleItems, h);
    }
    if (h > formP->window.windowBounds.extent.y) h = formP->window.windowBounds.extent.y;
    if (h == 0) h = th;
    debug(DEBUG_TRACE, "List", "LstAdjust %d visible=%d h=%d extent=%d", listP->id, listP->visibleItems, h, formP->window.windowBounds.extent.y);

    if (listP->bounds.topLeft.x + w > formP->window.windowBounds.extent.x) {
      listP->bounds.topLeft.x = formP->window.windowBounds.extent.x - w;
    }
    debug(DEBUG_TRACE, "List", "LstAdjust %d visible=%d y=%d h=%d fh=%d (before)", listP->id, listP->visibleItems, listP->bounds.topLeft.y, h, formP->window.windowBounds.extent.y);
    if (listP->bounds.topLeft.y + h > formP->window.windowBounds.extent.y) {
      listP->bounds.topLeft.y = formP->window.windowBounds.extent.y - h;
      debug(DEBUG_TRACE, "List", "LstAdjust %d visible=%d y=%d h=%d fh=%d (after)", listP->id, listP->visibleItems, listP->bounds.topLeft.y, h, formP->window.windowBounds.extent.y);
    }

    listP->bounds.extent.x = w;
    listP->bounds.extent.y = h;
    debug(DEBUG_TRACE, "List", "LstAdjust %d visible=%d extent=%d", listP->id, listP->visibleItems, listP->bounds.extent.y);

    if (listP->popupWin) {
      WinDeleteWindow(listP->popupWin, false);
    }
    listP->popupWin = WinCreateOffscreenWindow(w, h, nativeFormat, &err);
    listP->popupWin->windowFlags.modal = 1;
    listP->popupWin->windowBounds.topLeft.x = formP->window.windowBounds.topLeft.x + listP->bounds.topLeft.x;
    listP->popupWin->windowBounds.topLeft.y = formP->window.windowBounds.topLeft.y + listP->bounds.topLeft.y;
    debug(DEBUG_TRACE, "List", "LstAdjust %d visible=%d py=%d", listP->id, listP->visibleItems, listP->popupWin->windowBounds.topLeft.y);

    if (listP->bitsBehind) {
      WinDeleteWindow(listP->bitsBehind, false);
      listP->bitsBehind = WinCreateOffscreenWindow(w, h, nativeFormat, &err);
    }
  }
}

void LstSetHeight(ListType *listP, Int16 visibleItems) {
  if (listP) {
    debug(DEBUG_TRACE, "List", "LstSetHeight new=%d, current=%d", visibleItems, listP->visibleItems);
    listP->visibleItems = visibleItems;
    LstAdjust(listP);
  }
}

void LstSetPosition(ListType *listP, Coord x, Coord y) {
  FormType *formP;

  if (listP && x >= 0 && y >= 0) {
    formP = (FormType *)listP->formP;

    if (x + listP->bounds.extent.x > formP->window.windowBounds.extent.x) {
      x = formP->window.windowBounds.extent.x - listP->bounds.extent.x;
    }
    if (y + listP->bounds.extent.y > formP->window.windowBounds.extent.y) {
      y = formP->window.windowBounds.extent.y - listP->bounds.extent.y;
    }

    listP->bounds.topLeft.x = x;
    listP->bounds.topLeft.y = y;
    listP->popupWin->windowBounds.topLeft.x = formP->window.windowBounds.topLeft.x + listP->bounds.topLeft.x;
    listP->popupWin->windowBounds.topLeft.y = formP->window.windowBounds.topLeft.y + listP->bounds.topLeft.y;
  }
}

void LstSetSelection(ListType *listP, Int16 itemNum) {
  if (listP) {
    if ((itemNum >= 0 && itemNum < listP->numItems) || itemNum == noListSelection) {
      debug(DEBUG_TRACE, "List", "LstSetSelection itemNum %d", itemNum);
      listP->currentItem = itemNum;
    } else if (itemNum == 0 && listP->numItems == 0) {
      debug(DEBUG_TRACE, "List", "LstSetSelection itemNum %d", itemNum);
      listP->currentItem = itemNum;
    }
  }
}

// This function does not copy the strings in the
// itemsText array, which means that you need to ensure that the
// array is not moved or deallocated until after you are done with the list.
// If you use a callback routine to draw the items in your list, the
// itemsText pointer is simply passed to that callback routine and is
// not otherwise used by the List Manager code.

void LstSetListChoices(ListType *listP, Char **itemsText, Int16 numItems) {
  if (listP) {
    debug(DEBUG_TRACE, "List", "LstSetListChoices itemsText=%p numItems=%d", itemsText, numItems);
    listP->itemsText = itemsText;
    listP->numItems = numItems;
    if (listP->bitsBehind) {
      if (numItems) listP->visibleItems = numItems;
    }
    LstAdjust(listP);
  }
}

// custom function to free the items
void LstFreeListChoices(ListType *listP) {
  if (listP) {
    debug(DEBUG_TRACE, "List", "LstFreeListChoices itemsText=%p numItems=%d", listP->itemsText, listP->numItems);
    if (listP->itemsText) {
      MemPtrFree(listP->itemsText);
      listP->itemsText = NULL;
    }
    listP->numItems = 0;
  }
}

void LstSetDrawFunction(ListType *listP, ListDrawDataFuncPtr func) {
  if (listP) {
    listP->drawItemsCallback = func;
  }
}

void LstSetTopItem(ListType *listP, Int16 itemNum) {
  if (listP && itemNum >= 0 && itemNum < listP->numItems) {
    if (itemNum < listP->visibleItems || listP->numItems < listP->visibleItems) {
      listP->topItem = 0; // first page
    } else if (itemNum >= listP->numItems - listP->visibleItems) {
      listP->topItem = listP->numItems - listP->visibleItems; // last page
    } else {
      listP->topItem = itemNum;
    }
  }
}

void LstMakeItemVisible(ListType *listP, Int16 itemNum) {
  if (listP && itemNum >= 0 && itemNum < listP->numItems) {
    if (itemNum < listP->topItem || itemNum >= listP->topItem + listP->visibleItems) {
      LstSetTopItem(listP, itemNum);
    }
  }
}

Int16 LstGetNumberOfItems(const ListType *listP) {
  return listP ? listP->numItems : 0;
}

// Display a modal window that contains the items in the list.
// Saves the previously active window. Creates and deletes the new popup window.
// Returns the list item selected, or -1 if no item was selected.

Int16 LstPopupList(ListType *listP) {
  FormType *frm;
  EventType event;
  WinHandle prevActive, prevDraw;
  Boolean stop, entered, handled;
  Err err;
  UInt16 index;
  Int16 r = -1;

  debug(DEBUG_TRACE, "List", "LstPopupList begin");
  if (listP) {
    listP->attr.poppedUp = 1;
    LstDrawList(listP);
    prevActive = WinGetActiveWindow();
    WinSetActiveWindow(listP->popupWin);
    prevDraw = WinSetDrawWindow(listP->popupWin);

    frm = (FormType *)listP->formP;
    if (frm) {
      index = FrmGetObjectIndex(frm, listP->id);
      if (index != frmInvalidObjectId) {
        frm->activeList = index;
      }
    }

    debug(DEBUG_TRACE, "List", "LstPopupList loop begin");
    entered = false;

    for (stop = false; !stop;) {
      EvtGetEvent(&event, evtWaitForever);
      if (SysHandleEvent(&event)) continue;
      if (MenuHandleEvent(NULL, &event, &err)) continue;

      handled = false;

      switch (event.eType) {
        case lstSelectEvent:
          debug(DEBUG_TRACE, "List", "LstPopupList lstSelect");
          if (event.data.lstSelect.listID == listP->id) {
            r = event.data.lstSelect.selection;
            debug(DEBUG_TRACE, "List", "LstPopupList lstSelect list id %d selection %d", listP->id, r);
            stop = true;
          }
          break;
        case popSelectEvent:
          debug(DEBUG_TRACE, "List", "LstPopupList popSelect");
          if (event.data.popSelect.listID == listP->id) {
            r = event.data.popSelect.selection;
            debug(DEBUG_TRACE, "List", "LstPopupList popSelect list id %d selection %d", listP->id, r);
            stop = true;
          }
          break;
        case lstExitEvent:
          debug(DEBUG_TRACE, "List", "LstPopupList lstExit");
          if (event.data.lstExit.listID == listP->id) {
            debug(DEBUG_TRACE, "List", "LstPopupList lstExit list id %d", listP->id);
            stop = true;
          }
          break;
        case appStopEvent:
          debug(DEBUG_TRACE, "List", "LstPopupList appStop");
          stop = true;
          break;
        case lstEnterEvent:
          debug(DEBUG_TRACE, "List", "LstPopupList lstEnter");
          entered = true;
          break;
        case penDownEvent:
          debug(DEBUG_TRACE, "List", "LstPopupList penDown");
          handled = LstHandleEvent(listP, &event);
          break;
        case penUpEvent:
          debug(DEBUG_TRACE, "List", "LstPopupList penUp");
          if (entered) {
            handled = LstHandleEvent(listP, &event);
          } else {
            handled = true;
          }
          break;
        default:
          debug(DEBUG_TRACE, "List", "LstPopupList event %d", event.eType);
          break;
      }

      if (stop) {
        if (listP->bitsBehind) {
          debug(DEBUG_TRACE, "List", "LstPopupList popSelect list id %d erase", listP->id);
          LstEraseList(listP);
        }
        listP->attr.poppedUp = 0;
        frm->activeList = -1;
        WinSetActiveWindow(prevActive);
        WinSetDrawWindow(prevDraw);
      }

      if (!handled) {
        FrmDispatchEvent(&event);
      }
    }
    debug(DEBUG_TRACE, "List", "LstPopupList loop end");
  }

  debug(DEBUG_TRACE, "List", "LstPopupList end r=%d", r);
  return r;
}

Boolean LstScrollList(ListType *listP, WinDirectionType direction, Int16 itemCount) {
  if (listP && listP->numItems > 0 && itemCount > 0 && listP->currentItem != noListSelection) {
    switch (direction) {
      case winUp:
        listP->currentItem -= itemCount;
        if (listP->currentItem < 0) listP->currentItem = 0;
        debug(DEBUG_TRACE, "List", "LstScrollList winUp itemCount %d currentItem %d", itemCount, listP->currentItem);
        break;
      case winDown:
        listP->currentItem += itemCount;
        if (listP->currentItem >= listP->numItems) listP->currentItem = listP->numItems-1;
        debug(DEBUG_TRACE, "List", "LstScrollList winDown itemCount %d currentItem %d", itemCount, listP->currentItem);
        break;
      default:
        break;
    }
  }

  return true;
}

Int16 LstGetVisibleItems(const ListType *listP) {
  return listP ? listP->visibleItems : 0;
}

Int16 LstGetTopItem(const ListType *listP) {
  return listP ? listP->topItem : 0;
}

ListType *LstNewListEx(void **formPP, UInt16 id, Coord x, Coord y, Coord width, Coord height, FontID font, Int16 visibleItems, Int16 triggerId, Boolean usable) {
  // XXX triggerId not used
  ListType *lstP = NULL;
  FormType **fpp, *formP;
  Err err;

  if (formPP) {
    fpp = (FormType **)formPP;
    formP = *fpp;

    if (formP) {
      if ((lstP = pumpkin_heap_alloc(sizeof(ListType), "List")) != NULL) {
        if (width > formP->window.windowBounds.extent.x) {
          width = formP->window.windowBounds.extent.x;
        }
        if (height > formP->window.windowBounds.extent.y) {
          height = formP->window.windowBounds.extent.y;
        }
        if (x + width > formP->window.windowBounds.extent.x) {
          x = formP->window.windowBounds.extent.x - width;
        }
        if (y + height > formP->window.windowBounds.extent.y) {
          y = formP->window.windowBounds.extent.y - height;
        }

        lstP->id = id;
        lstP->bounds.topLeft.x = x; // form relative coordinate
        lstP->bounds.topLeft.y = y; // form relative coordinate
        lstP->bounds.extent.x = width;
        lstP->bounds.extent.y = height;
        lstP->attr.usable       = usable;
        lstP->attr.enabled      = 0;
        lstP->attr.visible      = 0;
        lstP->attr.poppedUp     = 0;
        lstP->attr.hasScrollBar = 0;
        lstP->attr.search       = 0;
        lstP->attr.reserved     = 0;

        lstP->numItems = 0;
        lstP->font = font;
        lstP->visibleItems = visibleItems;
        lstP->formP = formP;
        lstP->marker = 'List';

        lstP->popupWin = WinCreateOffscreenWindow(width, height, nativeFormat, &err);
        if (err == errNone) {
          lstP->popupWin->windowFlags.modal = 1;
          lstP->popupWin->windowBounds.topLeft.x = formP->window.windowBounds.topLeft.x + lstP->bounds.topLeft.x; // absolute screen coordinate
          lstP->popupWin->windowBounds.topLeft.y = formP->window.windowBounds.topLeft.y + lstP->bounds.topLeft.y; // absolute screen coordinate

          if (!lstP->attr.usable) {
            lstP->bitsBehind = WinCreateOffscreenWindow(width, height, nativeFormat, &err);
          }

          if (err == errNone) {
            if (formP->numObjects == 0) {
              formP->objects = xcalloc(1, sizeof(FormObjListType));
            } else {
              formP->objects = xrealloc(formP->objects, (formP->numObjects + 1) * sizeof(FormObjListType));
            }
            formP->objects[formP->numObjects].objectType = frmListObj;
            formP->objects[formP->numObjects].object.list = lstP;
            formP->objects[formP->numObjects].id = formP->objects[formP->numObjects].object.control->id;
            formP->numObjects++;
          } else {
            WinDeleteWindow(lstP->popupWin, false);
            pumpkin_heap_free(lstP, "List");
          }
        } else {
          pumpkin_heap_free(lstP, "List");
        }
      }
    }
  }

  return lstP;
}

Err LstNewList(void **formPP, UInt16 id, Coord x, Coord y, Coord width, Coord height, FontID font, Int16 visibleItems, Int16 triggerId) {
  Err err = sysErrParamErr;

  if (LstNewListEx(formPP, id, x, y, width, height, font, visibleItems, triggerId, true) != NULL) {
    err = errNone;
  }

  return err;
}
