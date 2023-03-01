#include <PalmOS.h>

#include "sys.h"
#include "thread.h"
#include "pwindow.h"
#include "vfs.h"
#include "pumpkin.h"
#include "debug.h"
#include "xalloc.h"

/*
Tables have an edit mode, which is activated as soon as a tblEnterEvent occurs within an editable text
field in the table, or when your application calls the TblGrabFocus function to deliberately select a specific
text cell for editing. You can check whether a table is in edit mode by calling the TblEditing function,
which returns true if a text field is currently active for editing. It also is possible to retrieve a
pointer to the current field, if any, by calling TblGetCurrentField.
The table manager calls the save data callback function any time the current field loses the focus. This
can occur when the user changes the focus by tapping in a different table cell, when the user taps a note
indicator in a textWithNoteTableItem cell, or if your code explicitly calls TblReleaseFocus. Calling
TblReleaseFocus manually is necessary any time your code redraws the table in such a way that the
current field may no longer appear in the table, thus causing it to lose the focus. For example, Librarian’s
EditFormScroll function calls TblReleaseFocus before attempting to scroll the table.
*/

static Boolean TblSaveData(TableType *tableP, UInt16 row, UInt16 col);

static void TblDrawTableRow(TableType *tableP, UInt16 row) {
  UInt16 column, len, th, tw, w, year, month, day, i;
  UInt32 seconds;
  DateTimeType dt;
  IndexedColorType fieldBack, fieldText, fieldLine, objFore, oldb, oldf, oldt;
  WinDrawOperation prev;
  Int16 dataOffset, dataSize;
  MemHandle dataH;
  RectangleType rect;
  TableColumnAttrType *cattr;
  TableItemType *item;
  ListType *list;
  FontID old;
  char *s, buf[16];

  debug(DEBUG_TRACE, "Table", "TblDrawTableRow id %d row %d", tableP->id, row);
  seconds = TimGetSeconds();
  TimSecondsToDateTime(seconds, &dt);

  fieldBack = UIColorGetTableEntryIndex(UIFieldBackground);
  fieldText = UIColorGetTableEntryIndex(UIFieldText);
  fieldLine = UIColorGetTableEntryIndex(UIFieldTextLines);

  objFore = UIColorGetTableEntryIndex(UIObjectForeground);
  oldb = WinSetBackColor(fieldBack);
  oldf = WinSetForeColor(objFore);
  oldt = WinSetTextColor(fieldText);
  i = row * tableP->numColumns;

  for (column = 0; column < tableP->numColumns; column++, i++) {
    if (!tableP->columnAttrs[column].usable) continue;
    debug(DEBUG_TRACE, "Table", "TblDrawTableRow id %d row %d col %d", tableP->id, row, column);
    cattr = &tableP->columnAttrs[column];
    item = &tableP->items[i];
    old = FntSetFont(item->fontID);
    TblGetItemBounds(tableP, row, column, &rect);
    WinSetClip(&rect);
    WinSetBackColor(fieldBack);
    WinSetForeColor(objFore);
    WinSetTextColor(fieldText);
    WinEraseRectangle(&rect, 0);

    switch (item->itemType) {
      case checkboxTableItem:
        FntSetFont(checkboxFont);
        WinDrawChar(item->intValue ? 1 : 0, rect.topLeft.x, rect.topLeft.y);
        break;
      case customTableItem:
         // Application-defined cell. The height of the item is fixed at 11 pixels.
        rect.extent.y = 11;
        if (cattr->m68k_drawfunc) {
          CallTableDrawItem(cattr->m68k_drawfunc, tableP, row, column, &rect);
        } else if (cattr->drawCallback) {
          cattr->drawCallback(tableP, row, column, &rect);
        }
        break;
      case dateTableItem:
        // Non-editable date in the form month/day, or a dash if the date value is -1.
        // The date is followed by an exclamation point if it has past.
        // The intValue field should be a value that can be cast to DateType.
        // DateType is currently defined as a 16-bit number: yyyyyyymmmmddddd
        // The first 7 bits are the year given as the offset since 1904, the next 4 bits are the month,
        // and the next 5 bits are the day.
        // Dates are always drawn in the current font.
        if (item->intValue == -1) {
          StrCopy(buf, "-");
        } else {
          year = (((UInt16)item->intValue) >> 9) & 0x0F;
          month = (((UInt16)item->intValue) >> 5) & 0x0F;
          day = ((UInt16)item->intValue) & 0x1F;
          StrPrintF(buf, "%02d/%02d", month, day);
          if (year < dt.year || month < dt.month || day < dt.day) {
            StrCat(buf, "!");
          }
        }
        FntSetFont(old);
        WinDrawChars(buf, StrLen(buf), rect.topLeft.x, rect.topLeft.y);
        break;
      case labelTableItem:
        if (item->ptr) {
          len = StrLen(item->ptr);
          tw = FntCharsWidth(item->ptr, len);
          w = FntCharWidth(':');
          WinDrawChars(item->ptr, len, rect.topLeft.x + rect.extent.x - tw - w, rect.topLeft.y);
          WinDrawChar(':', rect.topLeft.x + rect.extent.x - w, rect.topLeft.y);
        }
        break;
      case numericTableItem:
        FntSetFont(boldFont);
        StrPrintF(buf, "%d", item->intValue);
        WinDrawChars(buf, StrLen(buf), rect.topLeft.x+1, rect.topLeft.y);
        debug(DEBUG_TRACE, "Table", "TblDrawTable numericTableItem (%d,%d) = %d", row, column, item->intValue);
        break;
      case popupTriggerTableItem:
        // intValue is the index of the list item that should be displayed.
        // ptr is a pointer to the list. Lists are displayed in the system’s default font.
        list = (ListType *)item->ptr;
        if (list && (s = LstGetSelectionText(list, item->intValue)) != NULL) {
          debug(DEBUG_TRACE, "Table", "TblDrawTable popupTriggerTableItem row %d col %d \"%s\"", row, column, s);
          len = StrLen(s);
          tw = FntCharsWidth(s, len);
          w = FntCharWidth(':');
          WinDrawChars(s, len, rect.topLeft.x + rect.extent.x - tw - w, rect.topLeft.y);
          WinDrawChar(':', rect.topLeft.x + rect.extent.x - w, rect.topLeft.y);
          FntSetFont(symbol7Font);
          w += FntCharWidth(2);
          WinDrawChar(2, rect.topLeft.x + rect.extent.x - tw - w - 2, rect.topLeft.y+2);
        }
        break;
      case textTableItem:
      case narrowTextTableItem:
      case textWithNoteTableItem:
        dataH = NULL;
        dataOffset = 0;
        dataSize = 0;

        if (cattr->m68k_loadfunc) {
          CallTableLoadData(cattr->m68k_loadfunc, tableP, row, column, false, &dataH, &dataOffset, &dataSize, &tableP->currentField);
        } else if (cattr->loadDataCallback) {
          cattr->loadDataCallback(tableP, row, column, false, &dataH, &dataOffset, &dataSize, &tableP->currentField);
        }

        if (cattr->m68k_drawfunc) {
          CallTableDrawItem(cattr->m68k_drawfunc, tableP, row, column, &rect);
        } else if (cattr->drawCallback) {
          cattr->drawCallback(tableP, row, column, &rect);
        } else {
          th = FntCharHeight();
          WinSetForeColor(fieldLine);
          WinDrawLine(rect.topLeft.x, rect.topLeft.y + th - 1, rect.topLeft.x + rect.extent.x - 1, rect.topLeft.y + th - 1);

          prev = WinSetDrawMode(winOverlay);
          if (dataH) {
            if ((s = MemHandleLock(dataH)) != NULL) {
              len = StrLen(&s[dataOffset]);
              if (len > dataSize) len = dataSize;
              if (len) {
                WinPaintChars(&s[dataOffset], len, rect.topLeft.x+1, rect.topLeft.y);
              }
              MemHandleUnlock(dataH);
            }
          } else if (item->ptr && item->ptr[0]) {
            len = StrLen(item->ptr);
            WinPaintChars(item->ptr, len, rect.topLeft.x+1, rect.topLeft.y);
          }
          WinSetDrawMode(prev);
        }

        if (item->itemType == textWithNoteTableItem) {
          FntSetFont(symbolFont);
          rect.extent.x -= FntCharWidth(0x0B);
          WinDrawChar(0x0B, rect.topLeft.x + rect.extent.x, rect.topLeft.y);
        }
        break;
      case timeTableItem:
        // not implemented in PalmOS
        debug(DEBUG_ERROR, "Table", "TblDrawTable timeTableItem not implemented");
        break;
      case tallCustomTableItem:
        if (cattr->m68k_drawfunc) {
          CallTableDrawItem(cattr->m68k_drawfunc, tableP, row, column, &rect);
        } else if (cattr->drawCallback) {
          cattr->drawCallback(tableP, row, column, &rect);
        }
        break;
      default:
        debug(DEBUG_ERROR, "Table", "TblDrawTable row %d col %d invalid item type %d", row, column, item->itemType);
        break;
    }
    FntSetFont(old);
  }

  WinResetClip();
  WinSetBackColor(oldb);
  WinSetForeColor(oldf);
  WinSetTextColor(oldt);
}

static void TblEraseTableRow(TableType *tableP, UInt16 row) {
  IndexedColorType formFill, oldb;
  RectangleType rect;
  UInt16 i;

  formFill = UIColorGetTableEntryIndex(UIFormFill);
  oldb = WinSetBackColor(formFill);

  rect.topLeft.x = tableP->bounds.topLeft.x;
  rect.extent.x = 0;
  for (i = 0; i < tableP->numColumns; i++) {
    rect.extent.x += tableP->columnAttrs[i].width;
    if (i < tableP->numColumns-1) {
      rect.extent.x += tableP->columnAttrs[i].spacing;
    }
  }

  rect.topLeft.y = tableP->bounds.topLeft.y;
  for (i = 0; i < row; i++) {
    rect.topLeft.y += tableP->rowAttrs[i].height;
  }
  rect.extent.y = tableP->rowAttrs[row].height;

  WinEraseRectangle(&rect, 0);
  WinSetBackColor(oldb);
}

static void TblDrawTableIfInvalid(TableType *tableP, Boolean ifInvalid) {
  UInt16 row;

  if (tableP) {
    for (row = 0; row < tableP->numRows; row++) {
      if (tableP->rowAttrs[row].usable) {
        if (!(ifInvalid && !tableP->rowAttrs[row].invalid)) {
          TblDrawTableRow(tableP, row);
          tableP->rowAttrs[row].invalid = false;
        }
      }
    }
  }
}

void TblDrawTable(TableType *tableP) {
  TblDrawTableIfInvalid(tableP, false);
  tableP->attr.visible = true;
}

void TblRedrawTable(TableType *tableP) {
  TblDrawTableIfInvalid(tableP, true);
  tableP->attr.visible = true;
}

void TblEraseTable(TableType *tableP) {
  IndexedColorType formFill, oldb;

  if (tableP) {
    tableP->attr.visible = false;
//debug(1, "XXX", "TblEraseTable selected=false");
    tableP->attr.selected = false;

    formFill = UIColorGetTableEntryIndex(UIFormFill);
    oldb = WinSetBackColor(formFill);
    WinEraseRectangle(&tableP->bounds, 0);
    WinSetBackColor(oldb);
  }
}

static UInt16 TblScreenToRow(TableType *tableP, UInt16 screenY) {
  UInt16 row, selected = 0, amount = 0;

  for (row = 0; row < tableP->numRows; row++) {
    if (screenY >= tableP->bounds.topLeft.y + amount) {
      selected = row;
    }
    amount += tableP->rowAttrs[row].height;
  }

  return selected;
}

static UInt16 TblScreenToColumn(TableType *tableP, UInt16 screenX) {
  UInt16 column, selected = 0, amount = 0;

  for (column = 0; column < tableP->numColumns; column++) {
    if (screenX >= tableP->bounds.topLeft.x + amount) {
      selected = column;
    }
    amount += tableP->columnAttrs[column].width + tableP->columnAttrs[column].spacing;
  }

  return selected;
}

// De acordo com log do Mu:
// penDownEvent:  SaveDataProc do antigo
// tblEnterEvent: LoadDataProc do novo

Boolean TblHandleEvent(TableType *tableP, EventType *eventP) {
  EventType event;
  TableItemType *item;
  UInt16 i, row, column;
  Int16 currentRow, currentCol;
  Boolean handled = false;

  switch (eventP->eType) {
    case penDownEvent:
//debug(1, "XXX", "table %d penDown (%d,%d)", tableP->id, eventP->screenX, eventP->screenY);
      if (tableP->attr.usable && tableP->attr.visible) {
        if (RctPtInRectangle(eventP->screenX, eventP->screenY, &tableP->bounds)) {
//debug(1, "XXX", "penDownEvent in table");
          row = TblScreenToRow(tableP, eventP->screenY);
          column = TblScreenToColumn(tableP, eventP->screenX);
          if (tableP->rowAttrs[row].usable && tableP->columnAttrs[column].usable) {

            if (TblGetSelection(tableP, &currentRow, &currentCol)) {
//debug(1, "XXX", "penDownEvent saveData");
              TblSaveData(tableP, currentRow, currentCol);
            }

//debug(1, "XXX", "TblHandleEvent penDownEvent selected=true");
            tableP->attr.selected = true;
            MemSet(&event, sizeof(EventType), 0);
            event.eType = tblEnterEvent;
            event.screenX = eventP->screenX;
            event.screenY = eventP->screenY;
            event.data.tblEnter.tableID = tableP->id;
            event.data.tblEnter.pTable = tableP;
            event.data.tblEnter.row = row;
            event.data.tblEnter.column = column;
//debug(1, "XXX", "penDownEvent row=%d col=%d send tblEnterEvent", row, column);
            EvtAddEventToQueue(&event);
            handled = true;
          } else {
//debug(1, "XXX", "table %d penDown row %d usable %d col %d usable %d", tableP->id, row, tableP->rowAttrs[row].usable, column, tableP->columnAttrs[column].usable);
          }
        } else {
//debug(1, "XXX", "table %d penDown not in bounds", tableP->id);
        }
      } else {
//debug(1, "XXX", "table %d penDown usable=%d visible=%d", tableP->id, tableP->attr.usable, tableP->attr.visible);
      }
      break;

    case penUpEvent:
//debug(1, "XXX", "table %d penUp", tableP->id);
      MemSet(&event, sizeof(EventType), 0);
      if (RctPtInRectangle(eventP->screenX, eventP->screenY, &tableP->bounds)) {
        row = TblScreenToRow(tableP, eventP->screenY);
        column = TblScreenToColumn(tableP, eventP->screenX);
//debug(1, "XXX", "ZZZ penUpEvent in table row=%d col=%d", row, column);
        if (tableP->rowAttrs[row].usable && tableP->columnAttrs[column].usable) {
          event.eType = tblSelectEvent;
          event.screenX = eventP->screenX;
          event.screenY = eventP->screenY;
          event.data.tblSelect.tableID = tableP->id;
          event.data.tblSelect.pTable = tableP;
          event.data.tblSelect.row = row;
          event.data.tblSelect.column = column;
//debug(1, "XXX", "ZZZ penUpEvent in table row=%d col=%d send tblSelectEvent", row, column);
          EvtAddEventToQueue(&event);
          debug(DEBUG_TRACE, "Table", "table %d select %d,%d", tableP->id, event.data.tblSelect.column, event.data.tblSelect.row);

          i = row * tableP->numColumns + column;
          item = &tableP->items[i];
          switch (item->itemType) {
            case checkboxTableItem:
              item->intValue = !item->intValue;
              TblMarkRowInvalid(tableP, row);
              TblRedrawTable(tableP);
              FldSetActiveField(NULL);
              tableP->attr.editing = false;
//debug(1, "XXX", "table %d TblHandleEvent penUp checkbox editing=false", tableP->id);
              break;
            case textTableItem:
            case textWithNoteTableItem:
            case narrowTextTableItem:
//debug(1, "XXX", "ZZZ penUpEvent in table row=%d col=%d send fldEnterEvent", row, column);
              MemSet(&event, sizeof(EventType), 0);
              event.eType = fldEnterEvent;
              event.screenX = eventP->screenX;
              event.screenY = eventP->screenY;
              event.data.fldEnter.fieldID = tableP->currentField.id;
              event.data.fldEnter.pField = &tableP->currentField;
              FldHandleEvent(&tableP->currentField, &event);
              break;
            default:
              FldSetActiveField(NULL);
              tableP->attr.editing = false;
//debug(1, "XXX", "table %d TblHandleEvent penUp other editing=false", tableP->id);
              break;
          }

        }
      } else {
        event.data.tblExit.tableID = tableP->id;
        event.data.tblExit.pTable = tableP;
        event.data.tblExit.row = 0;
        event.data.tblExit.column = 0;
        EvtAddEventToQueue(&event);
        debug(DEBUG_TRACE, "Table", "table %d exit", tableP->id);
      }
      handled = true;
      break;

    case tblEnterEvent:
//debug(1, "XXX", "tblEnterEvent");
      if (eventP->data.tblEnter.tableID == tableP->id) {
//debug(1, "XXX", "tblEnterEvent in table");
        TblSetSelection(tableP, eventP->data.tblEnter.row, eventP->data.tblEnter.column);
        TblGrabFocus(tableP, eventP->data.tblEnter.row, eventP->data.tblEnter.column);
//debug(1, "XXX", "tblEnterEvent in table select row=%d col=%d", eventP->data.tblEnter.row, eventP->data.tblEnter.column);
        handled = true;
      }
      break;
    case tblSelectEvent:
//debug(1, "XXX", "tblSelectEvent event id=%d table id=%d", eventP->data.tblSelect.tableID, tableP->id);
      if (eventP->data.tblSelect.tableID == tableP->id) {
        row = eventP->data.tblSelect.row;
        column = eventP->data.tblSelect.column;
//debug(1, "XXX", "tblSelectEvent in table select row=%d col=%d", eventP->data.tblSelect.row, eventP->data.tblSelect.column);
        i = row * eventP->data.tblSelect.pTable->numColumns + column;
        item = &eventP->data.tblSelect.pTable->items[i];

        switch (item->itemType) {
          case textTableItem:
          case textWithNoteTableItem:
          case narrowTextTableItem:
            // XXX if set true, librarian fields do not get focus when 1st clicked
//debug(1, "XXX", "table %d TblHandleEvent select text editing=true", tableP->id);
//debug(1, "XXX", "tblSelectEvent in table select row=%d col=%d editing text", eventP->data.tblSelect.row, eventP->data.tblSelect.column);
            tableP->attr.editing = true;
            break;
          default:
            break;
        }
      }
      break;
    case tblExitEvent:
//debug(1, "XXX", "tblExitEvent");
      debug(DEBUG_TRACE, "Table", "exit table %d", tableP->id);
      handled = true;
      break;
    default:
      break;
  }

  return handled;
}

void TblGetItemBounds(const TableType *tableP, Int16 row, Int16 column, RectangleType *rP) {
  UInt16 i;

  if (tableP && row >= 0 && row < tableP->numRows && column >= 0 && column < tableP->numColumns && rP) {
    rP->extent.x = tableP->columnAttrs[column].width;
    rP->extent.y = tableP->rowAttrs[row].height;
    rP->topLeft.x = tableP->bounds.topLeft.x;
    for (i = 0; i < column; i++) {
      rP->topLeft.x += tableP->columnAttrs[i].width + tableP->columnAttrs[i].spacing;
    }
    rP->topLeft.y = tableP->bounds.topLeft.y;
    for (i = 0; i < row; i++) {
      rP->topLeft.y += tableP->rowAttrs[i].height;
    }
  }
}

// Select (highlight) the specified item. If there is already a selected item, it is unhighlighted.
// If row contains a masked private database record, then the item remains unselected.
// This function cannot highlight an entire row; it can only highlight
// one cell in a row, and it always unhighlights the previously selected
// table cell. If you want to select an entire row, either create a table
// that has a single column, or write your own selection code.
// If the selected item is a multi-line text field or a text field with a
// nonstandard height, this function only highlights the top eleven
// pixels. If you want a larger area highlighted, you must write your own selection code.

void TblSelectItem(TableType *tableP, Int16 row, Int16 column) {
  Int16 currentRow, currentCol;
  RectangleType rect;

  if (tableP && row >= 0 && row < tableP->numRows && column >= 0 && column < tableP->numColumns) {
    if (TblGetSelection(tableP, &currentRow, &currentCol)) {
      // unhighligh previous item
      TblGetItemBounds(tableP, currentRow, currentCol, &rect);
      WinInvertRect(&rect, 0);
    }

    TblSetSelection(tableP, row, column);
    // highligh new item
    TblGetItemBounds(tableP, row, column, &rect);
    WinInvertRect(&rect, 0);
  }
}

Int16 TblGetItemInt(const TableType *tableP, Int16 row, Int16 column) {
  Int16 value = 0;

  if (tableP && row >= 0 && row < tableP->numRows && column >= 0 && column < tableP->numColumns) {
    value = tableP->items[row*tableP->numColumns + column].intValue;
  }

  return value;
}

void TblSetItemInt(TableType *tableP, Int16 row, Int16 column, Int16 value) {
  if (tableP && row >= 0 && row < tableP->numRows && column >= 0 && column < tableP->numColumns) {
    tableP->items[row*tableP->numColumns + column].intValue = value;
    debug(DEBUG_TRACE, "Table", "TblSetItemInt (%d,%d) = %d", row, column, value);
  }
}

void TblSetItemPtr(TableType *tableP, Int16 row, Int16 column, void *value) {
  if (tableP && row >= 0 && row < tableP->numRows && column >= 0 && column < tableP->numColumns) {
    tableP->items[row*tableP->numColumns + column].ptr = value;
  }
}

void TblSetItemStyle(TableType *tableP, Int16 row, Int16 column, TableItemStyleType type) {
  if (tableP && row >= 0 && row < tableP->numRows && column >= 0 && column < tableP->numColumns) {
    tableP->items[row*tableP->numColumns + column].itemType = type;
  }
}

void TblUnhighlightSelection(TableType *tableP) {
  Int16 currentRow, currentCol;
  RectangleType rect;

  if (tableP) {
    if (TblGetSelection(tableP, &currentRow, &currentCol)) {
      TblGetItemBounds(tableP, currentRow, currentCol, &rect);
      WinInvertRect(&rect, 0);
    }
//debug(1, "XXX", "TblUnhighlightSelection selected=false");
    tableP->attr.selected = false;
  }
}

Boolean TblRowUsable(const TableType *tableP, Int16 row) {
  Boolean usable = false;

  if (tableP && row >= 0 && row < tableP->numRows) {
    usable = tableP->rowAttrs[row].usable;
  }

  return usable;
}

void TblSetRowUsable(TableType *tableP, Int16 row, Boolean usable) {
  if (tableP && row >= 0 && row < tableP->numRows) {
    if (tableP->attr.visible) {
      if (usable && !tableP->rowAttrs[row].usable) {
        TblDrawTableRow(tableP, row);
      } else if (!usable && tableP->rowAttrs[row].usable) {
        TblEraseTableRow(tableP, row);
      }
    }
    tableP->rowAttrs[row].usable = usable;
  }
}

Int16 TblGetLastUsableRow(const TableType *tableP) {
  Int16 row;

  for (row = tableP->numRows-1; row >= 0; row--) {
    if (tableP->rowAttrs[row].usable) {
      return row;
    }
  }

  return tblUnusableRow;
}

void TblSetColumnUsable(TableType *tableP, Int16 column, Boolean usable) {
  if (tableP && column >= 0 && column < tableP->numColumns) {
    tableP->columnAttrs[column].usable = usable;
  }
}

void TblSetRowSelectable(TableType *tableP, Int16 row, Boolean selectable) {
  if (tableP && row >= 0 && row < tableP->numRows) {
    tableP->rowAttrs[row].selectable = selectable;
  }
}

Boolean TblRowSelectable(const TableType *tableP, Int16 row) {
  Boolean selectable = false;

  if (tableP && row >= 0 && row < tableP->numRows) {
    selectable = tableP->rowAttrs[row].selectable;
  }

  return selectable;

}

Int16 TblGetNumberOfRows(const TableType *tableP) {
  return tableP ? tableP->numRows : 0;
}

void TblGetBounds(const TableType *tableP, RectangleType *rP) {
  if (tableP && rP) {
    MemMove(rP, &tableP->bounds, sizeof(RectangleType));
  }
}

void TblSetBounds(TableType *tableP, const RectangleType *rP) {
  if (tableP && rP) {
    MemMove(&tableP->bounds, rP, sizeof(RectangleType));
  }
}

Coord TblGetRowHeight(const TableType *tableP, Int16 row) {
  UInt16 height = 0;

  if (tableP && row >= 0 && row < tableP->numRows) {
    height = tableP->rowAttrs[row].height;
  }

  return height;
}

void TblSetRowHeight(TableType *tableP, Int16 row, Coord height) {
  IndexedColorType formFill, oldb;
  RectangleType rect;
  UInt16 y;

  if (tableP && row >= 0 && row < tableP->numRows) {
    tableP->rowAttrs[row].height = height;
    for (row = 0, y = 0; row < tableP->numRows; row++) {
      if (tableP->rowAttrs[row].usable) {
        y += tableP->rowAttrs[row].height;
      }
    }

    if (tableP->attr.visible) {
      if (y < tableP->bounds.extent.y) {
        // table has shrinked, erase bottom
        RctSetRectangle(&rect, tableP->bounds.topLeft.x, tableP->bounds.topLeft.y + y, tableP->bounds.extent.x, tableP->bounds.extent.y - y);
        formFill = UIColorGetTableEntryIndex(UIFormFill);
        oldb = WinSetBackColor(formFill);
        WinEraseRectangle(&rect, 0);
        WinSetBackColor(oldb);
      }
    }
    // XXX comentado: TblSetRowHeight nao altera altura da tabela (se alterar, da problema no MemoPad ao mudar categoria para Business (por ex.) e depois de volta para All)
    //tableP->bounds.extent.y = y;
  }
}

Coord TblGetColumnWidth(const TableType *tableP, Int16 column) {
  Coord width = 0;

  if (tableP && column >= 0 && column < tableP->numColumns) {
    width = tableP->columnAttrs[column].width;
  }

  return width;
}

void TblSetColumnWidth(TableType *tableP, Int16 column, Coord width) {
  if (tableP && column >= 0 && column < tableP->numColumns) {
    tableP->columnAttrs[column].width = width;
  }
}

Coord TblGetColumnSpacing(const TableType *tableP, Int16 column) {
  Coord spacing = 0;

  if (tableP && column >= 0 && column < tableP->numColumns) {
    spacing = tableP->columnAttrs[column].spacing;
  }

  return spacing;
}

void TblSetColumnSpacing(TableType *tableP, Int16 column, Coord spacing) {
  if (tableP && column >= 0 && column < tableP->numColumns) {
    tableP->columnAttrs[column].spacing = spacing;
  }
}

Boolean TblFindRowID(const TableType *tableP, UInt16 id, Int16 *rowP) {
  Boolean found = false;
  UInt16 row;

  if (tableP) {
    for (row = 0; row < tableP->numRows && !found; row++) {
      if (tableP->rowAttrs[row].id == id) {
        if (rowP) *rowP = row;
        found = true;
      }
    }
  }

  return found;
}

Boolean TblFindRowData(const TableType *tableP, UIntPtr data, Int16 *rowP) {
  Boolean found = false;
  UInt16 row;

  if (tableP) {
    for (row = 0; row < tableP->numRows && !found; row++) {
      if (tableP->rowAttrs[row].data == data) {
        if (rowP) *rowP = row;
        found = true;
      }
    }
  }

  return found;
}

UInt16 TblGetRowID(const TableType *tableP, Int16 row) {
  UInt16 id = 0;

  if (tableP && row >= 0 && row < tableP->numRows) {
    id = tableP->rowAttrs[row].id;
  }

  return id;
}

void TblSetRowID(TableType *tableP, Int16 row, UInt16 id) {
  if (tableP && row >= 0 && row < tableP->numRows) {
    tableP->rowAttrs[row].id = id;
  }
}

UIntPtr TblGetRowData(const TableType *tableP, Int16 row) {
  UIntPtr data = 0;

  if (tableP && row >= 0 && row < tableP->numRows) {
    data = tableP->rowAttrs[row].data;
  }

  return data;
}

void TblSetRowData(TableType *tableP, Int16 row, UIntPtr data) {
  if (tableP && row >= 0 && row < tableP->numRows) {
    tableP->rowAttrs[row].data = data;
  }
}

Boolean TblRowInvalid(const TableType *tableP, Int16 row) {
  Boolean invalid = false;

  if (tableP && row >= 0 && row < tableP->numRows) {
    invalid = tableP->rowAttrs[row].invalid;
  }

  return invalid;
}

void TblMarkRowInvalid(TableType *tableP, Int16 row) {
  if (tableP && row >= 0 && row < tableP->numRows) {
    tableP->rowAttrs[row].invalid = true;
  }
}

void TblMarkTableInvalid(TableType *tableP) {
  UInt16 row;

  if (tableP) {
    for (row = 0; row < tableP->numRows; row++) {
      tableP->rowAttrs[row].invalid = true;
    }
  }
}

Boolean TblGetSelection(const TableType *tableP, Int16 *rowP, Int16 *columnP) {
  Boolean selected = false;

  if (tableP) {
    selected = tableP->attr.selected;
    if (rowP) *rowP = tableP->currentRow;
    if (columnP) *columnP = tableP->currentColumn;
  }

  return selected;
}

// Insert a row into the table before the specified row.
// The number of rows in a table is the maximum number of rows
// displayed on the screen. Unlike a multi-line text field, there is no
// notion of a table that is bigger than the available screen. For this
// reason, this function does not increase the number of table rows.
// Instead of keeping track of a total number of rows in the table and a
// number of rows displayed on the screen, tables mark any row that
// isn’t currently displayed with a usable value of false. (See TableRowAttrType.)
// The newly inserted row is marked as invalid, unusable, and not masked.
void TblInsertRow(TableType *tableP, Int16 row) {
  int i, j;

//debug(1, "XXX", "TblInsertRow row %d numRows %d", row, tableP->numRows);
  if (tableP && row >= 0 && row < tableP->numRows) {
    if (row < tableP->numRows-1) {
      for (i = tableP->numRows-1; i > row; i--) {
//debug(1, "XXX", "TblInsertRow move attrs row %d to %d", row + i - 1, row + i);
        MemMove(&tableP->rowAttrs[row + i], &tableP->rowAttrs[row + i - 1], sizeof(TableRowAttrType));
        for (j = 0; j < tableP->numColumns; j++) {
          MemMove(&tableP->items[(row + i) * tableP->numColumns + j], &tableP->items[(row + i - 1) * tableP->numColumns + j], sizeof(TableItemType));
        }
      }
      //MemSet(&tableP->rowAttrs[row], sizeof(TableRowAttrType), 0);
      //MemSet(&tableP->items[row * tableP->numColumns], tableP->numColumns * sizeof(TableRowAttrType), 0);
    }

//debug(1, "XXX", "TblInsertRow invalidate row %d", row);
    tableP->rowAttrs[row].invalid = true;
    tableP->rowAttrs[row].usable = false;
    tableP->rowAttrs[row].masked = false;
  }
}

// The number of rows in the table is not decreased; instead, this row is
// moved from its current spot to the end of the table and is marked
// unusable so that it won’t be displayed when the table is redrawn.
// This function does not visually update the display. To update the
// display, call TblRedrawTable.
void TblRemoveRow(TableType *tableP, Int16 row) {
  TableRowAttrType aux;
  int i, j, n;

//debug(1, "XXX", "TblRemoveRow row %d numRows %d", row, tableP->numRows);
  if (tableP && row >= 0 && row < tableP->numRows) {
    if (row < tableP->numRows-1) {
      MemMove(&aux, &tableP->rowAttrs[row], sizeof(TableRowAttrType));
      for (i = row; i < tableP->numRows-1; i++) {
        MemMove(&tableP->rowAttrs[row], &tableP->rowAttrs[row+1], sizeof(TableRowAttrType));
      }
      MemMove(&tableP->rowAttrs[i], &aux, sizeof(TableRowAttrType));

      j = row * tableP->numColumns;
      n = (tableP->numRows - row - 1) * tableP->numColumns;
      for (i = 0; i < n; i++) {
        MemMove(&tableP->items[j + i], &tableP->items[j + i + tableP->numColumns], sizeof(TableItemType));
      }
    }

    tableP->rowAttrs[row].usable = false;
  }
}

void TblGrabFocus(TableType *tableP, Int16 row, Int16 column) {
  TableColumnAttrType *cattr;
  TableItemType *item;
  RectangleType rect;
  MemHandle dataH;
  FontID old;
  Int16 i, dataOffset, dataSize;
  Err err;

  if (tableP && row >= 0 && row < tableP->numRows && column >= 0 && column < tableP->numColumns) {
    cattr = &tableP->columnAttrs[column];
    i = row * tableP->numColumns + column;
    item = &tableP->items[i];

    switch (item->itemType) {
      case textTableItem:
      case textWithNoteTableItem:
      case narrowTextTableItem:
        MemSet(&tableP->currentField, sizeof(FieldType), 0);
        tableP->currentField.id = 0; // XXX
        TblGetItemBounds(tableP, row, column, &rect);
        if (item->itemType == narrowTextTableItem) {
          rect.extent.x -= item->intValue;
        } else
        if (item->itemType == textWithNoteTableItem) {
          old = FntSetFont(symbolFont);
          rect.extent.x -= FntCharWidth(0x0B);
          FntSetFont(old);
        }
        FldSetBounds(&tableP->currentField, &rect);
        FldSetMaxChars(&tableP->currentField, 32); // XXX
        FldSetFont(&tableP->currentField, item->fontID);
        tableP->currentField.attr.underlined = 1;
        tableP->currentField.attr.editable = 1;
        tableP->currentField.attr.usable = 1;
        tableP->currentField.attr.singleLine = (rect.extent.y / FntCharHeight()) <= 1;
        tableP->currentField.attr.hasFocus = 1;

        if (cattr->m68k_loadfunc || cattr->loadDataCallback) {
          if (cattr->m68k_loadfunc) {
            err = CallTableLoadData(cattr->m68k_loadfunc, tableP, row, column, true, &dataH, &dataOffset, &dataSize, &tableP->currentField);
          } else {
            err = cattr->loadDataCallback(tableP, row, column, true, &dataH, &dataOffset, &dataSize, &tableP->currentField);
          }
          if (err == errNone) {
            if (dataH) {
//debug(1, "XXX", "table grabFocus FldSetText offset %d size %d", dataOffset, dataSize);
              FldSetText(&tableP->currentField, dataH, dataOffset, dataSize);
            }
          }
        }

        FldDrawField(&tableP->currentField);
//debug(1, "XXX", "table %d TblGrabFocus editing=true", tableP->id);
        tableP->attr.editing = true;
      default:
        break;
    }
  }
}

static Boolean TblSaveData(TableType *tableP, UInt16 row, UInt16 col) {
  TableColumnAttrType *cattr;
  TableItemType *item;
  UInt16 i;
  Boolean r = false;

  if (tableP && col >= 0 && col < tableP->numColumns && row >= 0 && row < tableP->numRows) {
    cattr = &tableP->columnAttrs[col];
    if (cattr->m68k_savefunc || cattr->saveDataCallback) {
      i = row * tableP->numColumns + col;
      item = &tableP->items[i];

      switch (item->itemType) {
        case textTableItem:
        case textWithNoteTableItem:
        case narrowTextTableItem:
          if (cattr->m68k_savefunc) {
            r = CallTableSaveData(cattr->m68k_savefunc, tableP, row, col);
          } else if (cattr->saveDataCallback) {
            r = cattr->saveDataCallback(tableP, row, col);
          }
          break;
        default:
          break;
      }
    }
  }

  return r;
}

void TblReleaseFocus(TableType *tableP) {
  Int16 currentRow, currentCol;

  if (tableP) {
    if (TblGetSelection(tableP, &currentRow, &currentCol)) {
//debug(1, "XXX", "TblReleaseFocus TblSaveData %d %d,%d", tableP->id, currentCol, currentRow);
      TblSaveData(tableP, currentRow, currentCol);
    }
//debug(1, "XXX", "table %d TblReleaseFocus editing=false", tableP->id);
    tableP->attr.editing = false;
    tableP->attr.selected = false;
  }
}

Boolean TblEditing(const TableType *tableP) {
  return tableP ? tableP->attr.editing : false;
}

FieldPtr TblGetCurrentField(const TableType *tableP) {
  return tableP && tableP->attr.editing ? &((TableType *)tableP)->currentField : NULL;
}

void TblSetColumnEditIndicator(TableType *tableP, Int16 column, Boolean editIndicator) {
  if (tableP && column >= 0 && column < tableP->numColumns) {
    tableP->columnAttrs[column].editIndicator = editIndicator;
  }
}

void TblSetRowStaticHeight(TableType *tableP, Int16 row, Boolean staticHeight) {
  if (tableP && row >= 0 && row < tableP->numRows) {
    tableP->rowAttrs[row].staticHeight = staticHeight;
  }
}

void TblHasScrollBar(TableType *tableP, Boolean hasScrollBar) {
  if (tableP) {
    tableP->attr.hasScrollBar = hasScrollBar;
  }
}

FontID TblGetItemFont(const TableType *tableP, Int16 row, Int16 column) {
  FontID fontID = 0;

  if (tableP && row >= 0 && row < tableP->numRows && column >= 0 && column < tableP->numColumns) {
    fontID = tableP->items[row*tableP->numColumns + column].fontID;
  }

  return fontID;
}

void TblSetItemFont(TableType *tableP, Int16 row, Int16 column, FontID fontID) {
  if (tableP && row >= 0 && row < tableP->numRows && column >= 0 && column < tableP->numColumns) {
    tableP->items[row*tableP->numColumns + column].fontID = fontID;
  }
}

void *TblGetItemPtr(const TableType *tableP, Int16 row, Int16 column) {
  void *ptr = NULL;

  if (tableP && row >= 0 && row < tableP->numRows && column >= 0 && column < tableP->numColumns) {
    ptr = tableP->items[row*tableP->numColumns + column].ptr;
  }

  return ptr;
}

Boolean TblRowMasked(const TableType *tableP, Int16 row) {
  Boolean masked = false;

  if (tableP && row >= 0 && row < tableP->numRows) {
    masked = tableP->rowAttrs[row].masked;
  }

  return masked;
}

void TblSetRowMasked(TableType *tableP, Int16 row, Boolean masked) {
  if (tableP && row >= 0 && row < tableP->numRows) {
    tableP->rowAttrs[row].masked = masked;
  }
}

// If true and the item’s row also has a masked attribute of true, the table cell is drawn on the screen but is shaded to obscure the information that it contains.
void TblSetColumnMasked(TableType *tableP, Int16 column, Boolean masked) {
  if (tableP && column >= 0 && column < tableP->numColumns) {
    tableP->columnAttrs[column].masked = masked;
  }
}

Int16 TblGetNumberOfColumns(const TableType *tableP) {
  return tableP ? tableP->numColumns : 0;
}

Int16 TblGetTopRow(const TableType *tableP) {
  return tableP ? tableP->topRow : 0;
}

void TblSetSelection(TableType *tableP, Int16 row, Int16 column) {
  if (tableP && row >= 0 && row < tableP->numRows && column >= 0 && column < tableP->numColumns) {
    tableP->currentRow = row;
    tableP->currentColumn = column;
//debug(1, "XXX", "TblSetSelection selected=true");
    tableP->attr.selected = true;
  }
}

void TblSetCustomDrawProcedure(TableType *tableP, Int16 column, TableDrawItemFuncPtr drawCallback) {
  if (tableP && column >= 0 && column < tableP->numColumns) {
    tableP->columnAttrs[column].drawCallback = drawCallback;
  }
}

// loadDataCallback is called during TblDrawTable and TblRedrawTable
void TblSetLoadDataProcedure(TableType *tableP, Int16 column, TableLoadDataFuncPtr loadDataCallback) {
  if (tableP && column >= 0 && column < tableP->numColumns) {
    tableP->columnAttrs[column].loadDataCallback = loadDataCallback;
  }
}

// saveDataCallback is called when the focus moves from one table cell to another and when the table loses focus entirely
void TblSetSaveDataProcedure(TableType *tableP, Int16 column, TableSaveDataFuncPtr saveDataCallback) {
  if (tableP && column >= 0 && column < tableP->numColumns) {
    tableP->columnAttrs[column].saveDataCallback = saveDataCallback;
  }
}
