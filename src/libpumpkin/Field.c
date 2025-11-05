#include <PalmOS.h>

static void nop(Err err);
#define PALMOS_MODULE "Field"
#define PALMOS_MODERR nop

#include "sys.h"
#include "thread.h"
#include "pwindow.h"
#include "vfs.h"
#include "pumpkin.h"
#include "debug.h"

/*
  UIFieldBackground:
  UIFieldText:
  UIFieldTextLines:
  UIFieldCaret:
  UIFieldTextHighlightBackground:
  UIFieldTextHighlightForeground:

  UInt16 numeric        :1; // Set if numeric, digits and secimal separator only
  UInt16 hasScrollBar   :1; // Set if the field has a scroll bar
  UInt16 autoShift      :1; // Set if auto case shift
  UInt16 justification  :2; // text alignment
  UInt16 underlined     :2; // text underlined mode
  UInt16 dirty          :1; // Set if user modified
  UInt16 insPtVisible   :1; // Set if the ins pt is scolled into view
  UInt16 dynamicSize    :1; // Set if height expands as text is entered
  UInt16 hasFocus       :1; // Set if the field has the focus
  UInt16 singleLine     :1; // Set if only a single line is displayed
  UInt16 editable       :1; // Set if editable
  UInt16 visible        :1; // Set if drawn, used internally
  UInt16 usable         :1; // Set if part of ui

  typedef struct LineInfoTag {
    UInt16  start;      // position in text string of first char.
    UInt16  length;     // number of character in the line
  } LineInfoType;

  UInt16        id;
  RectangleType rect;
  FieldAttrType attr;
  Char          *text;          // pointer to the start of text string
  MemHandle     textHandle;       // block the contains the text string
  LineInfoPtr   lines;
  UInt16        textLen;
  UInt16        textBlockSize;
  UInt16        maxChars;
  UInt16        selFirstPos;
  UInt16        selLastPos;
  UInt16        insPtXPos;
  UInt16        insPtYPos;
  FontID        fontID;
  UInt8         maxVisibleLines;    // added in 4.0 to support FldSetMaxVisibleLines
*/

typedef struct {
  FieldType *activeField;
  UInt16 penDownOffset;
  UInt16 penDownX, penDownY;
  FieldType auxField;
} fld_module_t;

static void nop(Err err) {
}

int FldInitModule(void) {
  fld_module_t *module;

  if ((module = sys_calloc(1, sizeof(fld_module_t))) == NULL) {
    return -1;
  }

  module->auxField.id = 1;
  module->auxField.attr.usable = true;

  pumpkin_set_local_storage(fld_key, module);

  return 0;
}

void *FldReinitModule(void *module) {
  fld_module_t *old = NULL;

  if (module) {
    FldFinishModule();
    pumpkin_set_local_storage(fld_key, module);
  } else {
    old = (fld_module_t *)pumpkin_get_local_storage(fld_key);
    FldInitModule();
  }

  return old;
}

int FldFinishModule(void) {
  fld_module_t *module = (fld_module_t *)pumpkin_get_local_storage(fld_key);

  if (module) {
    FldFreeMemory(&module->auxField);
    sys_free(module);
  }

  return 0;
}

FieldType *FldGetActiveField(void) {
  fld_module_t *module = (fld_module_t *)pumpkin_get_local_storage(fld_key);

  return module->activeField;
}

void FldSetActiveField(FieldType *fldP) {
  fld_module_t *module = (fld_module_t *)pumpkin_get_local_storage(fld_key);

  InsPtEnable(false);

  if (fldP) {
    debug(DEBUG_TRACE, PALMOS_MODULE, "FldSetActiveField field %d", fldP->id);
  } else {
    debug(DEBUG_TRACE, PALMOS_MODULE, "FldSetActiveField field NULL");
  }

  if (module->activeField) {
    debug(DEBUG_TRACE, PALMOS_MODULE, "FldSetActiveField release current focus");
    FldReleaseFocus(module->activeField);
  }

  module->activeField = fldP;

  if (module->activeField) {
    debug(DEBUG_TRACE, PALMOS_MODULE, "FldSetActiveField grab new focus");
    FldGrabFocus(module->activeField);
  }
}

static UInt16 FldNextWord(FieldType *fldP, UInt16 i, UInt16 *width, char *word, UInt16 max) {
  FontType *f;
  UInt32 wch;
  UInt16 j, n, total;
  UInt8 c;

  *width = 0;
  total = 0;

  for (j = 0; j < max; j++) {
    n = pumpkin_next_char((UInt8 *)fldP->text, i, fldP->textLen, &wch);
    c = pumpkin_map_char(wch, &f);
    word[j] = (char)c;
    if (c <= ' ') {
      if (j == 0) {
        *width += FntFontCharWidth(f, c);
        total += n;
        j++;
      }
      break;
    }
    *width += FntFontCharWidth(f, c);
    total += n;
    i += n;
  }
  word[j] = 0;

  return total;
}

static void FldRenderField(FieldType *fldP, Boolean setPos, Boolean draw, UInt16 offset, UInt16 *offsetRow, UInt16 *topOffset) {
  fld_module_t *module = (fld_module_t *)pumpkin_get_local_storage(fld_key);
  IndexedColorType fieldBack, fieldLine, fieldText, fieldBackHigh, oldb, oldf, oldt, oldBack, back;
  WinDrawOperation prev;
  RectangleType clip, aux;
  UInt16 th, tw, row, col, bottom, x, y, n, i;
  Boolean posFound, insPtEnable, valueSet;
  FontID old;
  char word[64];

  IN;
  if (fldP) {
    old = FntSetFont(fldP->fontID);
    th = FntCharHeight();
    bottom = fldP->top + fldP->rect.extent.y / th;
    debug(DEBUG_TRACE, PALMOS_MODULE, "field draw=%d setPos=%d top=%d bottom=%d", draw, setPos, fldP->top, bottom);
    fldP->numUsedLines = 0;
    row = col = 0;
    x = 1;
    y = 0;
    insPtEnable = false;
    valueSet = false;
    oldb = oldf = oldt = 0;
    fieldBackHigh = fieldBack = 0;

    if (draw) {
      fieldBack = UIColorGetTableEntryIndex(UIFieldBackground);
      fieldLine = UIColorGetTableEntryIndex(UIFieldTextLines);
      fieldText = UIColorGetTableEntryIndex(UIFieldText);
      fieldBackHigh = UIColorGetTableEntryIndex(UIFieldTextHighlightBackground);
      oldb = WinSetBackColor(fieldBack);
      oldf = WinSetForeColor(fieldLine);
      oldt = WinSetTextColor(fieldText);
    }

    posFound = false;

    WinSetAsciiText(true);
    debug(DEBUG_TRACE, PALMOS_MODULE, "begin text %d chars", fldP->textLen);
    for (i = 0; i < fldP->textLen; i += n) {
      n = FldNextWord(fldP, i, &tw, word, sizeof(word)-1);

      if (offsetRow && !valueSet && i >= offset) {
        *offsetRow = row;
        valueSet = true;
      }

      if (topOffset && !valueSet && row >= fldP->top) {
        *topOffset = i;
        valueSet = true;
      }

      if (setPos && module->penDownY >= y && module->penDownY < y + th && module->penDownX >= x && module->penDownX < x + tw) {
        InsPtSetHeight(th - 2);
        InsPtSetLocation(fldP->rect.topLeft.x + x - 1, fldP->rect.topLeft.y + y);
        insPtEnable = fldP->attr.editable && fldP->attr.hasFocus && row >= fldP->top && row < bottom;
        debug(DEBUG_TRACE, PALMOS_MODULE, "insPtEnable %d (a)", insPtEnable);
        fldP->pos = i;
        posFound = true;
      }

      if (!setPos && draw && i == fldP->pos) {
        InsPtSetHeight(th - 2);
        InsPtSetLocation(fldP->rect.topLeft.x + x - 1, fldP->rect.topLeft.y + y);
        insPtEnable = fldP->attr.editable && fldP->attr.hasFocus && row >= fldP->top && row < bottom;
        debug(DEBUG_TRACE, PALMOS_MODULE, "insPtEnable %d (b)", insPtEnable);
        posFound = true;
      }

      debug(DEBUG_TRACE, PALMOS_MODULE, "word=\"%s\" col=%d row=%d", word, col, row);

      if (!StrCompare(word, "\n")) {
        debug(DEBUG_TRACE, PALMOS_MODULE, "linefeed");
        if (!posFound && setPos && module->penDownY >= y && module->penDownY < y + th && module->penDownX >= x && module->penDownX < fldP->rect.topLeft.x + fldP->rect.extent.x) {
          InsPtSetHeight(th - 2);
          InsPtSetLocation(fldP->rect.topLeft.x + x - 1, fldP->rect.topLeft.y + y);
          insPtEnable = fldP->attr.editable && fldP->attr.hasFocus && row >= fldP->top && row < bottom;
          debug(DEBUG_TRACE, PALMOS_MODULE, "insPtEnable %d (c)", insPtEnable);
          fldP->pos = i;
          posFound = true;
        }

        if (draw && row >= fldP->top && row < bottom) {
          RctSetRectangle(&aux, fldP->rect.topLeft.x + x, fldP->rect.topLeft.y + y, fldP->rect.extent.x - x, th);
          if (fldP->attr.editable && fldP->attr.underlined) aux.extent.y--;
          WinEraseRectangle(&aux, 0);
        }
        x = 1;
        col = 0;
        debug(DEBUG_TRACE, PALMOS_MODULE, "col = 0, row %d -> %d", row, row+1);
        row++;
        if (row >= fldP->top && row < bottom) {
          fldP->numUsedLines++;
          if (draw && row-1 >= fldP->top && row-1 < bottom) {
            debug(DEBUG_TRACE, PALMOS_MODULE, "y %d -> %d (linefeed)", y, y + th);
            y += th;
          }
        }
      } else {
        if (x + tw > fldP->rect.extent.x) {
          debug(DEBUG_TRACE, PALMOS_MODULE, "overflow x: %d + %d >= %d", x, tw, fldP->rect.extent.x);
          x = 1;
          col = 0;
          debug(DEBUG_TRACE, PALMOS_MODULE, "col = 0, row %d -> %d", row, row+1);
          row++;
          if (row >= fldP->top && row < bottom) {
            fldP->numUsedLines++;
            if (draw && row-1 >= fldP->top && row-1 < bottom) {
              debug(DEBUG_TRACE, PALMOS_MODULE, "y %d -> %d (overflow)", y, y + th);
              y += th;
            }
          }
        }
        if (draw && row >= fldP->top && row < bottom) {
          if (fldP->attr.editable && fldP->attr.underlined) {
            WinDrawLine(fldP->rect.topLeft.x + x, fldP->rect.topLeft.y + y + th - 1,
                        fldP->rect.topLeft.x + fldP->rect.extent.x - 1, fldP->rect.topLeft.y + y + th - 1);
          }
          back = (fldP->selFirstPos <= i && i < fldP->selLastPos) ? fieldBackHigh : fieldBack;
          oldBack = WinSetBackColor(back);
          //debug(DEBUG_TRACE, PALMOS_MODULE, "draw '%c' col=%d row=%d x=%d y=%d", c, col, row, x, y);
          debug(DEBUG_TRACE, PALMOS_MODULE, "draw \"%s\" col=%d row=%d x=%d y=%d", word, col, row, x, y);
          if (fldP->attr.editable && fldP->attr.underlined) {
            RctSetRectangle(&clip, fldP->rect.topLeft.x + x, fldP->rect.topLeft.y + y, tw, th - 1);
            WinSetClip(&clip);
            //WinPaintChar(c, fldP->rect.topLeft.x + x, fldP->rect.topLeft.y + y);
            WinPaintChars(word, StrLen(word), fldP->rect.topLeft.x + x, fldP->rect.topLeft.y + y);
            WinSetBackColor(oldBack);
            RctSetRectangle(&clip, fldP->rect.topLeft.x + x, fldP->rect.topLeft.y + y + th - 1, tw, 1);
            WinSetClip(&clip);
            prev = WinSetDrawMode(winOverlay);
            //WinPaintChar(c, fldP->rect.topLeft.x + x, fldP->rect.topLeft.y + y);
            WinPaintChars(word, StrLen(word), fldP->rect.topLeft.x + x, fldP->rect.topLeft.y + y);
            WinSetDrawMode(prev);
            WinResetClip();
          } else {
            //WinPaintChar(c, fldP->rect.topLeft.x + x, fldP->rect.topLeft.y + y);
            WinPaintChars(word, StrLen(word), fldP->rect.topLeft.x + x, fldP->rect.topLeft.y + y);
          }
        }
        //debug(DEBUG_TRACE, PALMOS_MODULE, "col %d -> %d", col, col+1);
        //debug(DEBUG_TRACE, PALMOS_MODULE, "x %d -> %d", x, x+tw);
        col++;
        x += tw;
      }
    }
    if (x > 1) fldP->numUsedLines++;
    fldP->totalLines = row + 1;
    debug(DEBUG_TRACE, PALMOS_MODULE, "end text used=%d total=%d", fldP->numUsedLines, fldP->totalLines);

    if (!posFound) {
      if (setPos) {
        fldP->pos = fldP->textLen;
      }

      if (draw) {
        InsPtSetHeight(th - 2);
        InsPtSetLocation(fldP->rect.topLeft.x + x - 1, fldP->rect.topLeft.y + y);
        insPtEnable = fldP->attr.editable && fldP->attr.hasFocus && (fldP->textLen == 0 || (row >= fldP->top && row < bottom));
        debug(DEBUG_TRACE, PALMOS_MODULE, "insPtEnable %d (d)", insPtEnable);
      }
    }

    if (draw) {
      if (row >= fldP->top && row < bottom) {
        RctSetRectangle(&aux, fldP->rect.topLeft.x + x, fldP->rect.topLeft.y + y, fldP->rect.extent.x - x, th);
        if (fldP->attr.editable && fldP->attr.underlined) aux.extent.y--;
        WinEraseRectangle(&aux, 0);
        if (fldP->attr.editable && fldP->attr.underlined) {
          debug(DEBUG_TRACE, PALMOS_MODULE, "underline row=%d x=%d y=%d", row, x, y);
          WinDrawLine(fldP->rect.topLeft.x + x, fldP->rect.topLeft.y + y + th - 1,
                      fldP->rect.topLeft.x + fldP->rect.extent.x - 1, fldP->rect.topLeft.y + y + th - 1);
        }
        y += th;
        debug(DEBUG_TRACE, PALMOS_MODULE, "y %d -> %d", y, y + th);
        row++;
        for (; row < bottom; row++) {
          RctSetRectangle(&aux, fldP->rect.topLeft.x, fldP->rect.topLeft.y + y, fldP->rect.extent.x, th);
          if (fldP->attr.editable && fldP->attr.underlined) aux.extent.y--;
          WinEraseRectangle(&aux, 0);
          if (fldP->attr.editable && fldP->attr.underlined) {
            debug(DEBUG_TRACE, PALMOS_MODULE, "underline row=%d y=%d", row, y);
            WinDrawLine(fldP->rect.topLeft.x, fldP->rect.topLeft.y + y + th - 1,
                        fldP->rect.topLeft.x + fldP->rect.extent.x - 1, fldP->rect.topLeft.y + y + th - 1);
          }
          y += th;
          debug(DEBUG_TRACE, PALMOS_MODULE, "y %d -> %d", y, y + th);
        }
      }

      WinSetBackColor(oldb);
      WinSetForeColor(oldf);
      WinSetTextColor(oldt);
      fldP->attr.visible = 1;
    }

    FntSetFont(old);

    debug(DEBUG_TRACE, PALMOS_MODULE, "InsPtEnable(%d)", insPtEnable);
    InsPtEnable(insPtEnable);
  }
  WinSetAsciiText(false);
  OUTV;
}

void FldDrawField(FieldType *fldP) {
  FldRenderField(fldP, false, true, 0, NULL, NULL);
}

void FldEraseField(FieldType *fldP) {
  IndexedColorType formFill, oldb;

  IN;
  if (fldP) {
    debug(DEBUG_TRACE, PALMOS_MODULE, "FldEraseField field %d", fldP->id);
    if (fldP->attr.visible) {
      debug(DEBUG_TRACE, PALMOS_MODULE, "FldEraseField field %d is visible", fldP->id);
      formFill = UIColorGetTableEntryIndex(UIFormFill);
      oldb = WinSetBackColor(formFill);
      WinEraseRectangle(&fldP->rect, 0);
      WinSetBackColor(oldb);
      fldP->attr.visible = 0;
    }
  }
  OUTV;
}

/*
Release the handle-based memory allocated to the field’s text and
the associated word-wrapping information.
*/
void FldFreeMemory(FieldType *fldP) {
  IN;
  if (fldP) {
    debug(DEBUG_TRACE, PALMOS_MODULE, "FldFreeMemory field %d", fldP->id);
    if (fldP->textHandle) {
      debug(DEBUG_TRACE, PALMOS_MODULE, "FldFreeMemory free text handle");
      MemHandleFree(fldP->textHandle);
      fldP->textHandle = NULL;
    }
    if (fldP->textBuf) {
      debug(DEBUG_TRACE, PALMOS_MODULE, "FldFreeMemory free text buf");
      pumpkin_heap_free(fldP->textBuf, "FieldTextBuf");
      fldP->textBuf = NULL;
    }
    fldP->text = NULL;
    fldP->textLen = 0;
    fldP->size = 0;
    fldP->textBlockSize = 0;
    fldP->numUsedLines = 0;
    fldP->totalLines = 0;
    fldP->pos = 0;
  }
  OUTV;
}

void FldGetBounds(const FieldType *fldP, RectanglePtr rect) {
  IN;
  if (fldP) {
    MemMove(rect, &fldP->rect, sizeof(RectangleType));
  }
  OUTV;
}

FontID FldGetFont(const FieldType *fldP) {
  FontID id;
  IN;
  id = fldP ? fldP->fontID : 0;
  OUTV;
  return id;
}

void FldGetSelection(const FieldType *fldP, UInt16 *startPosition, UInt16 *endPosition) {
  IN;
  if (fldP) {
    if (startPosition) *startPosition = fldP->selFirstPos;
    if (endPosition) *endPosition = fldP->selLastPos;
  }
  OUTV;
}

static void FldUpdateHandle(FieldType *fldP) {
  UInt16 oldSize, textLen, restSize;
  UInt8 *p, *buf;

  if (fldP) {
    if (fldP->textHandle == NULL) {
      debug(DEBUG_TRACE, PALMOS_MODULE, "FldUpdateHandle textHandle is null");
      if ((fldP->textHandle = MemHandleNew(fldP->textLen + 1)) != NULL) {
        fldP->textBlockSize = fldP->textLen + 1;
        fldP->size = fldP->textBlockSize;
        if ((p = MemHandleLock(fldP->textHandle)) != NULL) {
          DmWrite(p, 0, fldP->text, fldP->size);
          MemHandleUnlock(fldP->textHandle);
        }
        debug(DEBUG_TRACE, PALMOS_MODULE, "FldUpdateHandle alloc textHandle %p blockSize %d", fldP->textHandle, fldP->textBlockSize);
      }
    } else {
      oldSize = fldP->size;
      restSize = fldP->textBlockSize - (fldP->offset + oldSize);
      textLen = fldP->textLen + 1;
      debug(DEBUG_TRACE, PALMOS_MODULE, "FldUpdateHandle textHandle %p blockSize %u offset %u size %u (%u) restSize %u",
        fldP->textHandle, fldP->textBlockSize, fldP->offset, fldP->size, textLen, restSize);
      if (textLen > fldP->size) fldP->size = textLen;

      if ((buf = pumpkin_heap_alloc(fldP->offset + fldP->size + restSize, "FieldUpdateBuf")) != NULL) {
        if ((p = MemHandleLock(fldP->textHandle)) != NULL) {
          if (fldP->offset) MemMove(buf, p, fldP->offset);
          MemMove(buf + fldP->offset, fldP->text, textLen);
          if (restSize) MemMove(buf + fldP->offset + fldP->size, p + fldP->offset + oldSize, restSize);
          MemHandleUnlock(fldP->textHandle);
          if (MemHandleResize(fldP->textHandle, fldP->offset + fldP->size + restSize) == errNone) {
            fldP->textBlockSize = fldP->offset + fldP->size + restSize;
            debug(DEBUG_TRACE, PALMOS_MODULE, "FldUpdateHandle resize textHandle %p blockSize %u offset %u size %u (%u) restSize %u",
              fldP->textHandle, fldP->textBlockSize, fldP->offset, fldP->size, textLen, restSize);
            if ((p = MemHandleLock(fldP->textHandle)) != NULL) {
              DmWrite(p, 0, buf, fldP->textBlockSize);
              MemHandleUnlock(fldP->textHandle);
            }
          }
          pumpkin_heap_free(buf, "FieldUpdateBuf");
        }
      }
    }
  }
}

/*
The handle returned by this function is not necessarily the handle to
the start of the string. If you’ve used FldSetText() to set the
field’s text to a string that is part of a database record, the text
handle points to the start of that record. You’ll need to compute the
offset from the start of the record to the start of the string. You can
either store the offset that you passed to FldSetText or you can
compute the offset by performing pointer arithmetic on the pointer
you get by locking this handle and the pointer returned by
FldGetTextPtr().
If you are obtaining the text handle so that you can edit the field’s
text, you must remove the handle from the field before you do so. If
you change the text while it is being used by a field, the field’s
internal structures specifying the text length, allocated size, and
word wrapping information can become out of sync. To avoid this
problem, remove the text handle from the field, change the text, and
then set the field’s text handle again.
*/
MemHandle FldGetTextHandle(const FieldType *_fldP) {
  FieldType *fldP;
  MemHandle h = NULL;

  IN;
  fldP = (FieldType *)_fldP;
  if (fldP) {
    h = fldP->textHandle;
  }
  OUTV;

  return h;
}

Char *FldGetTextPtr(const FieldType *fldP) {
  Char *p;
  IN;
  p = fldP ? fldP->text : NULL;
  OUTV;
  return p;
}

static Boolean deleteSelection(FieldType *fldP) {
  Boolean r = false;

  if (fldP->selFirstPos < fldP->selLastPos) {
    FldDelete(fldP, fldP->selFirstPos, fldP->selLastPos);
    FldSetInsertionPoint(fldP, fldP->selFirstPos);
    FldSetSelection(fldP, 0, 0);
    r = true;
  }

  return r;
}

static void FldGrabFocusEx(FieldType *fldP, Boolean setPos) {
  if (fldP) {
    fldP->attr.hasFocus = true;
    FldRenderField(fldP, setPos, fldP->attr.visible, 0, NULL, NULL);
    InsPtEnable(true);
  }
}

Boolean FldHandleEvent(FieldType *fldP, EventType *eventP) {
  fld_module_t *module = (fld_module_t *)pumpkin_get_local_storage(fld_key);
  EventType event;
  Char c;
  Boolean handled = false;

  IN;
  if (fldP && eventP) {
    switch (eventP->eType) {
      case keyDownEvent:
        if (fldP->attr.usable && fldP->attr.editable && (fldP->attr.hasFocus || fldP->selFirstPos < fldP->selLastPos) &&
            !(eventP->data.keyDown.modifiers & commandKeyMask)) {
          switch (eventP->data.keyDown.chr) {
            case '\b':
              if (!deleteSelection(fldP)) {
                FldDelete(fldP, fldP->pos - 1, fldP->pos);
              }
              break;
            case '\r':
              debug(DEBUG_TRACE, PALMOS_MODULE, "FldHandleEvent CR");
              break;
            case '\n':
              debug(DEBUG_TRACE, PALMOS_MODULE, "FldHandleEvent NL");
              if (!fldP->attr.singleLine) {
                deleteSelection(fldP);
                c = eventP->data.keyDown.chr;
                FldInsert(fldP, &c, 1);
              }
              break;
            default:
              if (eventP->data.keyDown.chr >= 32) {
                deleteSelection(fldP);
                c = eventP->data.keyDown.chr;
                debug(DEBUG_TRACE, PALMOS_MODULE, "FldHandleEvent key %d", c);
                FldInsert(fldP, &c, 1);
              }
              break;
          }
          handled = true;
        }
        break;

      case penDownEvent:
        if (fldP->attr.usable && fldP->attr.editable && RctPtInRectangle(eventP->screenX, eventP->screenY, &fldP->rect)) {
          debug(DEBUG_TRACE, PALMOS_MODULE, "FldHandleEvent penDown inside field %d", fldP->id);
          if (fldP->selFirstPos < fldP->selLastPos) {
            FldSetSelection(fldP, 0, 0);
          }

          module->penDownX = eventP->screenX - fldP->rect.topLeft.x;
          module->penDownY = eventP->screenY - fldP->rect.topLeft.y;
          FldGrabFocusEx(fldP, true);
          module->penDownOffset = fldP->pos;

          MemSet(&event, sizeof(EventType), 0);
          event.eType = fldEnterEvent;
          event.screenX = eventP->screenX;
          event.screenY = eventP->screenY;
          event.data.fldEnter.fieldID = fldP->id;
          event.data.fldEnter.pField = fldP;
          EvtAddEventToQueue(&event);
          handled = true;
        }
        break;

      case penMoveEvent:
        if (fldP->attr.usable && fldP->attr.editable && eventP->penDown && RctPtInRectangle(eventP->screenX, eventP->screenY, &fldP->rect)) {
          module->penDownX = eventP->screenX - fldP->rect.topLeft.x;
          module->penDownY = eventP->screenY - fldP->rect.topLeft.y;
          FldRenderField(fldP, true, false, 0, NULL, NULL);

          if (fldP->pos != module->penDownOffset) {
            if (fldP->pos < module->penDownOffset) {
              FldSetSelection(fldP, fldP->pos, module->penDownOffset);
              InsPtEnable(false);
            } else {
              FldSetSelection(fldP, module->penDownOffset, fldP->pos);
              InsPtEnable(false);
            }
            debug(DEBUG_INFO, PALMOS_MODULE, "FldHandleEvent penMove selection \"%.*s\"", fldP->selLastPos - fldP->selFirstPos, &fldP->text[fldP->selFirstPos]);
          }
          handled = true;
        }
        break;

      case penUpEvent:
        if (RctPtInRectangle(eventP->screenX, eventP->screenY, &fldP->rect)) {
          debug(DEBUG_TRACE, PALMOS_MODULE, "FldHandleEvent penUp inside field %d", fldP->id);
        }
        break;

      case fldEnterEvent:
        debug(DEBUG_TRACE, PALMOS_MODULE, "FldHandleEvent fldEnter field %d", fldP->id);
        FldSetActiveField(NULL);
        FldSetActiveField(fldP);
        module->penDownX = eventP->screenX - fldP->rect.topLeft.x;
        module->penDownY = eventP->screenY - fldP->rect.topLeft.y;
        FldGrabFocusEx(fldP, true);
        break;

      default:
        break;
    }
  }
  OUTV;

  return handled;
}

void FldCut(FieldType *fldP) {
  IN;
  if (fldP) {
    if (fldP->attr.editable) {
      FldCopy(fldP);
      deleteSelection(fldP);
      FldDrawField(fldP);
    }
  }
  OUTV;
}

void FldCopy(const FieldType *fldP) {
  Int16 len;

  IN;
  if (fldP) {
    if (fldP->text && fldP->textLen) {
      len = fldP->selLastPos - fldP->selFirstPos;
      if (len > 0) {
        ClipboardAddItem(clipboardText, &fldP->text[fldP->selFirstPos], len);
      }
    }
  }
  OUTV;
}

void FldPaste(FieldType *fldP) {
  UInt16 length;
  MemHandle h;
  char *s;

  IN;
  if (fldP) {
    if ((h = ClipboardGetItem(clipboardText, &length)) != NULL) {
      if (length > 0 && (s = MemHandleLock(h)) != NULL) {
        deleteSelection(fldP);
        FldInsert(fldP, s, length);
        MemHandleUnlock(h);
      }
    }
  }
  OUTV;
}

void FldRecalculateField(FieldType *fldP, Boolean redraw) {
  IN;
  FldRenderField(fldP, false, fldP->attr.visible && redraw, 0, NULL, NULL);
  OUTV;
}

void FldSetBounds(FieldType *fldP, const RectangleType *rP) {
  IN;
  if (fldP && rP) {
    FldEraseField(fldP);
    MemMove(&fldP->rect, rP, sizeof(RectangleType));
    FldRecalculateField(fldP, true);
  }
  OUTV;
}

void FldSetFont(FieldType *fldP, FontID fontID) {
  IN;
  if (fldP) {
    if (fldP->attr.visible) {
      FldEraseField(fldP);
    }
    fldP->fontID = fontID;
    FldRecalculateField(fldP, true);
  }
  OUTV;
}

/*
The handle that you pass to this function is assumed to contain a
null-terminated string starting at offset bytes in the memory
chunk. The string should be between 0 and size - 1 bytes in length.
The field does not make a copy of the memory chunk or the string
data; instead, it stores the handle to the record in its structure.
FldSetText updates the word-wrapping information and places
the insertion point after the last visible character.
Because FldSetText (and FldSetTextHandle) may be used to
edit database records, they do not free the memory associated with
the previous text handle.
*/
void FldSetText(FieldType *fldP, MemHandle textHandle, UInt16 offset, UInt16 size) {
  Char *s;
  UInt16 len;
  UInt32 handleSize;

  IN;
  if (fldP) {
    debug(DEBUG_TRACE, "Field", "FldSetText fld %p handle %p offset %d size %d", fldP, textHandle, offset, size);
    fldP->textHandle = NULL;
    fldP->offset = 0;
    fldP->size = 0;
    fldP->textBlockSize = 0;
    handleSize = textHandle ? MemHandleSize(textHandle) : 0;
    if (handleSize > fldP->maxChars) {
      fldP->maxChars = handleSize;
      if (fldP->textBuf) {
        fldP->textBuf = pumpkin_heap_realloc(fldP->textBuf, fldP->maxChars, "FieldTextBuf");
        debug(DEBUG_TRACE, "Field", "FldSetText realloc textBuf=%p size=%d", fldP->textBuf, fldP->maxChars);
        fldP->text = fldP->textBuf;
        fldP->textLen = 0;
      }
    }

    if (fldP->textBuf == NULL) {
      fldP->textBuf = pumpkin_heap_alloc(fldP->maxChars, "FieldTextBuf");
      debug(DEBUG_TRACE, "Field", "FldSetText alloc textBuf=%p size=%d", fldP->textBuf, fldP->maxChars);
      fldP->text = fldP->textBuf;
      fldP->textLen = 0;
    }

    if (textHandle) {
      debug(DEBUG_TRACE, "Field", "FldSetText textHandle not NULL");
      fldP->textHandle = textHandle;
      fldP->textBlockSize = handleSize;
      if (size > fldP->textBlockSize) {
        debug(DEBUG_TRACE, "Field", "FldSetText size %d > textBlockSize %d", size, fldP->textBlockSize);
        size = fldP->textBlockSize;
      }
      if (offset > fldP->textBlockSize) {
        debug(DEBUG_TRACE, "Field", "FldSetText offset %d > textBlockSize %d", offset, fldP->textBlockSize);
        offset = 0;
        size = 0;
      }
      if ((offset + size) > fldP->textBlockSize) {
        debug(DEBUG_TRACE, "Field", "FldSetText offset+size %d > textBlockSize %d", offset + size, fldP->textBlockSize);
        size = fldP->textBlockSize - offset;
      }
      fldP->offset = offset;
      fldP->size = size;
      debug(DEBUG_TRACE, "Field", "FldSetText offset=%d size=%d", fldP->offset, fldP->size);
      if ((s = MemHandleLock(textHandle)) != NULL) {
        if (fldP->text) {
          len = fldP->size;
          debug(DEBUG_TRACE, "Field", "FldSetText len=%d", len);
          if (len > fldP->maxChars) {
            len = fldP->maxChars;
            debug(DEBUG_TRACE, "Field", "FldSetText len %d > maxChars %d, len=%d", len, fldP->maxChars, len);
          }
          MemMove(fldP->text, &s[offset], len);
          fldP->textLen = StrLen(&s[offset]);
          debug(DEBUG_TRACE, "Field", "FldSetText MemMove(%p, %p, %d)", fldP->text, &s[offset], len);
          debug(DEBUG_TRACE, "Field", "FldSetText texLen=%d", fldP->textLen);
        }
        MemHandleUnlock(textHandle);
      }
    }
    FldRecalculateField(fldP, false);
  }
  OUTV;
}

void FldSetTextHandle(FieldType *fldP, MemHandle textHandle) {
  UInt16 size;

  IN;
  debug(DEBUG_TRACE, "Field", "FldSetTextHandle handle %p", textHandle);
  size = textHandle ? MemHandleSize(textHandle) : 0;
  FldSetText(fldP, textHandle, 0, size);
  OUTV;
}

/*
The field never frees the string that you pass to this function, even
when the field itself is freed. You must free the string yourself.
Before you free the string, make sure the field is not still displaying
it. Set the field’s string pointer to some other string or call
FldSetTextPtr(fldP, NULL) before freeing a string you have
passed using this function.
*/
void FldSetTextPtr(FieldType *fldP, Char *textP) {
  IN;
  if (fldP) {
    debug(DEBUG_TRACE, "Field", "FldSetTextPtr %p", textP);
    if (!fldP->attr.editable) {
      if (textP) {
        fldP->text = textP;
        fldP->textLen = StrLen(textP);
      } else {
        fldP->text = NULL;
        fldP->textLen = 0;
      }
      FldRecalculateField(fldP, false);
    }
  }
  OUTV;
}

void FldSetUsable(FieldType *fldP, Boolean usable) {
  IN;
  if (fldP) {
    fldP->attr.usable = usable;
  }
  OUTV;
}

void FldSetSelection(FieldType *fldP, UInt16 startPosition, UInt16 endPosition) {
  fld_module_t *module = (fld_module_t *)pumpkin_get_local_storage(fld_key);

  IN;
  if (fldP) {
    if (startPosition < fldP->textLen && endPosition <= fldP->textLen && startPosition <= endPosition) {
      fldP->selFirstPos = startPosition;
      fldP->selLastPos = endPosition;
      FldRecalculateField(fldP, true);
      module->activeField = fldP;
    }
  }
  OUTV;
}

// Turn the insertion point on (if the specified field is visible) and position the blinking insertion point in the field.
// You rarely need to call this function directly. Instead, use FrmSetFocus, which calls FldGrabFocus for you.
// One instance where you need to call FldGrabFocus directly is to programmatically set the focus in a field that is contained in a table cell.
// This function sets the field attribute hasFocus to true.

void FldGrabFocus(FieldType *fldP) {
  FldGrabFocusEx(fldP, false);
}

// Turn the blinking insertion point off if the field is visible and has the current focus, reset the Graffiti state, and reset the undo state.
// This function sets the field attribute hasFocus to false. (See FieldAttrType.)
// Usually, you don’t need to call this function. If the field is in a form or in a table that doesn’t use custom drawing functions, the field
// code releases the focus for you when the focus changes to some other control. If your field is in any other type of object, such as a
// table that uses custom drawing functions or a gadget, you must call FldReleaseFocus when the focus moves away from the field.

void FldReleaseFocus(FieldType *fldP) {
  IN;
  if (fldP) {
    debug(DEBUG_TRACE, PALMOS_MODULE, "FldReleaseFocus field %d (visible %d, editable %d)", fldP->id, fldP->attr.visible, fldP->attr.editable);
    if (fldP->attr.visible && fldP->attr.editable) {
      debug(DEBUG_TRACE, PALMOS_MODULE, "FldReleaseFocus insPtEnable 0");
      InsPtEnable(false);
    }
    fldP->attr.hasFocus = false;
  }
  OUTV;
}

UInt16 FldGetInsPtPosition(const FieldType *fldP) {
  UInt16 offset = 0;

  if (fldP) {
    offset = fldP->pos;
  }

  return offset;
}

void FldSetInsPtPosition(FieldType *fldP, UInt16 pos) {
  // Set the location of the insertion point for a given string position.
  // If the position is beyond the visible text, the field is scrolled until the position is visible.

  FldSetInsertionPoint(fldP, pos);
  FldSetScrollPosition(fldP, pos);
}

void FldSetInsertionPoint(FieldType *fldP, UInt16 pos) {
  // Set the location of the insertion point based on a specified string position.
  // This routine differs from FldSetInsPtPosition in that it doesn’t make the character position visible.
  // FldSetInsertionPoint also doesn’t make the field the current focus of input if it was not already.
  // If pos indicates a position beyond the end of the text in the field, the insertion point is set to the end of the field’s text.

  if (fldP) {
    fldP->pos = pos;
    FldRenderField(fldP, false, false, 0, NULL, NULL);
  }
}

// Return the offset of the first character in the first visible line of a field
UInt16 FldGetScrollPosition(const FieldType *fldP) {
  UInt16 pos = 0;

  IN;
  if (fldP) {
    FldRenderField((FieldType *)fldP, false, false, 0, NULL, &pos);
    debug(DEBUG_INFO, PALMOS_MODULE, "FldGetScrollPosition top %d, offset %d", fldP->top, pos);
  }
  OUTV;

  return pos;
}

void FldSetScrollPosition(FieldType *fldP, UInt16 pos) {
  UInt16 row;

  IN;
  if (fldP && fldP->totalLines > 0) {
    fldP->top = 0;
    row = 0;
    FldRenderField(fldP, false, false, pos, &row, NULL);
    debug(DEBUG_INFO, PALMOS_MODULE, "FldSetScrollPosition offset %d, top %d -> %d", pos, fldP->top, row);
    fldP->top = row;
  }
  OUTV;
}

UInt16 FldGetTextLength(const FieldType *fldP) {
  UInt16 len;
  IN;
  len = fldP ? fldP->textLen : 0;
  OUTV;
  return len;
}

void FldScrollField(FieldType *fldP, UInt16 linesToScroll, WinDirectionType direction) {
  UInt16 top, visibleLines;

  IN;
  if (fldP) {
    switch (direction) {
      case winUp:
        top = (fldP->top >= linesToScroll) ? fldP->top - linesToScroll : 0;
        break;
      case winDown:
        if (fldP->top + linesToScroll > fldP->totalLines) {
          linesToScroll = fldP->totalLines - fldP->top;
        }
        top = fldP->top + linesToScroll;
        visibleLines = FldGetVisibleLines(fldP);
        if (fldP->totalLines - top < visibleLines) {
          top = fldP->totalLines - visibleLines;
        }
        break;
      default:
        top = fldP->top;
        break;
    }

    if (fldP->top != top) {
      fldP->top = top;
      FldDrawField(fldP);
    }
  }
  OUTV;
}

Boolean FldScrollable(const FieldType *fldP, WinDirectionType direction) {
  Boolean scrollable = false;

  IN;
  if (fldP) {
    switch (direction) {
      case winUp:
        scrollable = fldP->top > 0;
        break;
      case winDown:
        scrollable = (fldP->top + fldP->numUsedLines) < fldP->totalLines;
        break;
      default:
        break;
    }
  }
  OUTV;

  return scrollable;
}

// Return the number of lines that can be displayed within the visible
// bounds of the field, regardless of what text is stored in the field.
UInt16 FldGetVisibleLines(const FieldType *fldP) {
  RectangleType rect;
  FontID old;
  UInt16 th, num = 0;

  if (fldP) {
    FldGetBounds(fldP, &rect);
    old = FntSetFont(fldP->fontID);
    th = FntCharHeight();
    FntSetFont(old);
    num = th > 0 ? rect.extent.y / th : 0;
  }

  return num;
}

UInt16 FldGetTextHeight(const FieldType *fldP) {
  UInt16 height, th;
  FontID old;

  IN;
  height = 0;

  if (fldP && fldP->numUsedLines) {
    old = FntSetFont(fldP->fontID);
    th = FntCharHeight();
    FntSetFont(old);
    height = th * fldP->numUsedLines;
  }
  OUTV;

  return height;
}

UInt16 FldCalcFieldHeight(const Char *chars, UInt16 maxWidth) {
  fld_module_t *module = (fld_module_t *)pumpkin_get_local_storage(fld_key);
  UInt16 totalLines = 0;

  IN;
  if (chars) {
    if (chars[0]) {
      module->auxField.rect.extent.x = maxWidth;
      module->auxField.rect.extent.y = 32767;
      module->auxField.maxChars = StrLen(chars) + 1;
      module->auxField.fontID = FntGetFont();
      FldSetTextPtr(&module->auxField, (char *)chars);
      totalLines = module->auxField.totalLines;
      FldSetTextPtr(&module->auxField, NULL);
    } else {
      totalLines = 1;
    }
  } else {
    totalLines = 1;
  }
  OUTV;

  return totalLines;
}

UInt16 FldWordWrap(const Char *chars, Int16 maxWidth) {
  Int16 n = 0;

  IN;
  if (chars) {
    n = FntWidthToOffset(chars, StrLen(chars), maxWidth, NULL, NULL);
  }
  OUTV;

  return n;
}

void FldCompactText(FieldType *fldP) {
  IN;
  OUTV;
}

// Return true if the field has been modified since the text value was set.
Boolean FldDirty(const FieldType *fldP) {
  Boolean dirty;
  IN;
  dirty = fldP ? fldP->attr.dirty : false;
  OUTV;
  return dirty;
}

void FldSetDirty(FieldType *fldP, Boolean dirty) {
  IN;
  if (fldP) {
    fldP->attr.dirty = dirty;
  }
  OUTV;
}

UInt16 FldGetMaxChars(const FieldType *fldP) {
  UInt16 max;
  IN;
  max = fldP ? fldP->maxChars : 0;
  OUTV;
  return max;
}

void FldSetMaxChars(FieldType *fldP, UInt16 maxChars) {
  UInt16 old;
  char *aux;

  IN;
  if (fldP) {
    if (maxChars > maxFieldTextLen) maxChars = maxFieldTextLen;
    old = fldP->maxChars;
    fldP->maxChars = maxChars;
    if (fldP->textBuf) {
      aux = pumpkin_heap_alloc(fldP->maxChars, "FieldTextBuf");
      sys_memcpy(aux, fldP->textBuf, old);
      pumpkin_heap_free(fldP->textBuf, "FieldTextBuf");
      fldP->textBuf = aux;
      fldP->text = fldP->textBuf;
    }
  }
  OUTV;
}

Boolean FldInsert(FieldType *fldP, const Char *insertChars, UInt16 insertLen) {
  UInt16 deleteLen, availableLen;
  Boolean r = false;

  IN;
  if (fldP && fldP->attr.editable && insertChars && insertLen) {
    debug(DEBUG_TRACE, PALMOS_MODULE, "FldInsert \"%.*s\" (%d chars)", insertLen, insertChars, insertLen);

    if (fldP->textBuf == NULL) {
      debug(DEBUG_TRACE, PALMOS_MODULE, "FldInsert allocating %d bytes for textBuf", fldP->maxChars);
      fldP->textBuf = pumpkin_heap_alloc(fldP->maxChars, "FieldTextBuf");
      fldP->text = fldP->textBuf;
      fldP->textLen = 0;
    }

    if (fldP->selFirstPos < fldP->selLastPos) {
      // there is a text selection on the field

      // check if new text fits into the field
      deleteLen = fldP->selLastPos - fldP->selFirstPos;
      availableLen = fldP->maxChars - fldP->textLen + deleteLen;
      if (insertLen > availableLen) {
        insertLen = availableLen;
      }
      debug(DEBUG_TRACE, PALMOS_MODULE, "FldInsert %d chars", insertLen);

      fldP->pos = fldP->selFirstPos;

      if (insertLen == deleteLen) {
        // inserted text is the same length as current selection
        // just copy new text over current selection
        MemMove(&fldP->text[fldP->pos], insertChars, insertLen);
      } else if (insertLen < deleteLen) {
        // inserted text is smaller than current selection
        // copy new text over current selection
        MemMove(&fldP->text[fldP->pos], insertChars, insertLen);
        // shift the remainder of current text to the left
        MemMove(&fldP->text[fldP->pos + insertLen], &fldP->text[fldP->selLastPos], fldP->textLen - fldP->selLastPos);
      } else {
        // inserted text is bigger than current selection
        // open space for new text
        MemMove(&fldP->text[fldP->pos + insertLen], &fldP->text[fldP->pos + deleteLen], fldP->textLen - (fldP->pos + deleteLen));
        // insert the new text
        MemMove(&fldP->text[fldP->pos], insertChars, insertLen);
      }
      fldP->selFirstPos = fldP->selLastPos = 0;
      fldP->pos += insertLen;
      fldP->textLen += insertLen;

    } else {
      // there is no current text selection

      // check if new text fits into the field
      availableLen = fldP->maxChars - fldP->textLen;
      if (insertLen > availableLen) {
        insertLen = availableLen;
      }
      debug(DEBUG_TRACE, PALMOS_MODULE, "FldInsert %d chars", insertLen);

      // open space for new text
      MemMove(&fldP->text[fldP->pos + insertLen], &fldP->text[fldP->pos], fldP->textLen - fldP->pos);
      // insert the new text
      MemMove(&fldP->text[fldP->pos], insertChars, insertLen);
      fldP->pos += insertLen;
      fldP->textLen += insertLen;
    }

    // zero the remainder of the text buffer (not really necessary)
    MemSet(&fldP->text[fldP->textLen], fldP->maxChars - fldP->textLen, 0);

    FldUpdateHandle(fldP);
    FldSetDirty(fldP, true);
    FldRenderField(fldP, false, fldP->attr.visible, 0, NULL, NULL);
  }
  OUTV;

  return r;
}

/*
This function deletes all characters from the starting offset up to the
ending offset and sets the field’s dirty attribute. It does not delete
the character at the ending offset.
end param: if you pass a value that is greater than the number of bytes in
the field, all characters in the field are deleted.
*/
void FldDelete(FieldType *fldP, UInt16 start, UInt16 end) {
  UInt16 deleteLen;

  IN;
  if (fldP && fldP->attr.editable && fldP->textLen > 0 && start < end && start < fldP->textLen) {
    debug(DEBUG_TRACE, PALMOS_MODULE, "FldDelete %d to %d", start, end);

    if (start == 0 && end >= fldP->textLen) {
      debug(DEBUG_TRACE, PALMOS_MODULE, "FldDelete all");
      if (end > fldP->textLen) end = fldP->textLen;
      MemSet(fldP->text, end, 0);
      fldP->selFirstPos = fldP->selLastPos = 0;
      fldP->textLen = 0;
      fldP->pos = 0;
      fldP->attr.hasFocus = 1;

    } else {
      if (end > fldP->textLen) end = fldP->textLen;
      deleteLen = end - start;
      debug(DEBUG_TRACE, PALMOS_MODULE, "FldDelete %d chars", deleteLen);

      if (end < fldP->textLen) {
        MemMove(&fldP->text[start], &fldP->text[end], fldP->textLen - end);
      }

      fldP->selFirstPos = fldP->selLastPos = 0;
      fldP->textLen -= deleteLen;
      fldP->pos = start;
    }

    // zero the remainder of the text buffer (not really necessary)
    MemSet(&fldP->text[fldP->textLen], fldP->maxChars - fldP->textLen, 0);

    FldUpdateHandle(fldP);
    FldSetDirty(fldP, true);
    FldRenderField(fldP, false, fldP->attr.visible, 0, NULL, NULL);
  }
  OUTV;
}

// Undo the last change made to the field object, if any.
// Changes include typing, backspaces, delete, paste, and cut.

void FldUndo(FieldType *fldP) {
  NOTIMPLV;
}

UInt16 FldGetTextAllocatedSize(const FieldType *fldP) {
  UInt16 size = 0;

  IN;
  if (fldP) {
    size = fldP->textBlockSize;
  }
  OUTV;

  return size;
}

void FldSetTextAllocatedSize(FieldType *fldP, UInt16 allocatedSize) {
  IN;
  if (fldP) {
    fldP->textBlockSize = allocatedSize;
  }
  OUTV;
}

void FldGetAttributes(const FieldType *fldP, FieldAttrPtr attrP) {
  IN;
  if (fldP) {
    *attrP = fldP->attr;
  }
  OUTV;
}

void FldSetAttributes(FieldType *fldP, const FieldAttrType *attrP) {
  Boolean redraw;

  IN;
  if (fldP && attrP) {
    redraw =  fldP->attr.editable != attrP->editable;
    fldP->attr = *attrP;
    if (redraw && fldP->attr.visible) {
      FldDrawField(fldP);
    }
  }
  OUTV;
}

void FldSendChangeNotification(const FieldType *fldP) {
/*
  EventType event;

  IN;
  if (fldP) {
    xmemset(&event, 0, sizeof(EventType));
    event.eType = fldChangedEvent;
    event.screenX = fldP->rect.topLeft.x;
    event.screenY = fldP->rect.topLeft.y;
    event.data.fldChanged.fieldID = fldP->id;
    event.data.fldChanged.pField = (FieldType *)fldP;
    EvtAddEventToQueue(&event);
  }
  OUTV;
*/
}

// Send a fldHeightChangedEvent to the event queue.
// This function is used internally by the field code. You normally never call it in application code.

void FldSendHeightChangeNotification(const FieldType *fldP, UInt16 pos, Int16 numLines) {
  NOTIMPLV;
}

// Generates an event to cause a dynamically resizable field to expand its height to make its text fully visible.

Boolean FldMakeFullyVisible(FieldType *fldP) {
  NOTIMPLI;
}

UInt16 FldGetNumberOfBlankLines(const FieldType *fldP) {
  UInt16 n = 0;
  FontID old;

  IN;
  if (fldP) {
    old = FntSetFont(fldP->fontID);
    n = fldP->rect.extent.y / FntCharHeight();
    FntSetFont(old);

    if (fldP->numUsedLines) {
      n -= fldP->numUsedLines;
    }
  }
  OUTV;

  return n;
}

void FldSetMaxVisibleLines(FieldType *fldP, UInt8 maxLines) {
  IN;
  if (fldP) {
    fldP->maxVisibleLines = maxLines;
  }
  OUTV;
}

void FldGetScrollValues(const FieldType *fldP, UInt16 *scrollPosP, UInt16 *textHeightP, UInt16 *fieldHeightP) {
  UInt16 scrollPos = 0, textHeight = 0, fieldHeight = 0, th;
  FontID old;

  if (fldP) {
    old = FntSetFont(fldP->fontID);
    th = FntCharHeight();
    FntSetFont(old);

    // the line of text that is the topmost visible line
    scrollPos = fldP->top;

    // the number of lines needed to display the field’s text, given the width of the field
    textHeight = fldP->totalLines;

    // the number of visible lines in the field
    fieldHeight = fldP->rect.extent.y / th;

    debug(DEBUG_TRACE, "Field", "FldGetScrollValues pos=%d th=%d textHeight=%d fieldHeight=%d", scrollPos, th, textHeight, fieldHeight);
  }

  if (scrollPosP) *scrollPosP = scrollPos;
  if (textHeightP) *textHeightP = textHeight;
  if (fieldHeightP) *fieldHeightP = fieldHeight;
}

FieldType *FldNewField(void **formPP, UInt16 id,
  Coord x, Coord y, Coord width, Coord height,
  FontID font, UInt32 maxChars, Boolean editable, Boolean underlined,
  Boolean singleLine, Boolean dynamicSize, JustificationType justification,
  Boolean autoShift, Boolean hasScrollBar, Boolean numeric) {

  FieldType *fldP;
  FormType *formP;

  if ((fldP = pumpkin_heap_alloc(sizeof(FieldType), "Field")) != NULL) {
    fldP->id = id;
    fldP->rect.topLeft.x = x;
    fldP->rect.topLeft.y = y;
    fldP->rect.extent.x = width;
    fldP->rect.extent.y = height;
    fldP->attr.usable = true;
    fldP->attr.editable = editable;
    fldP->attr.underlined = underlined;
    fldP->attr.singleLine = singleLine;
    fldP->attr.dynamicSize = dynamicSize;
    fldP->attr.justification = justification;
    fldP->attr.autoShift = autoShift;
    fldP->attr.hasScrollBar = hasScrollBar;
    fldP->attr.numeric = numeric;
    fldP->maxChars = maxChars;
    fldP->fontID = font;

    if (formPP) {
      formP = *formPP;
      if (formP) {
        fldP->objIndex = formP->numObjects++;
        formP->objects = sys_realloc(formP->objects, formP->numObjects * sizeof(FormObjListType));
        if (formP->objects) {
          formP->objects[fldP->objIndex].objectType = frmFieldObj;
          formP->objects[fldP->objIndex].id = id;
          formP->objects[fldP->objIndex].object.field = fldP;
          formP->objects[fldP->objIndex].object.field->formP = formP;
        }
      }
    }
  }

  return fldP;
}

void FldReplaceText(FieldType *fldP, char *s, Boolean focus) {
  UInt16 len;
  FieldAttrType attr;
  Boolean old;

  IN;
  if (fldP) {
    FldGetAttributes(fldP, &attr);
    old = attr.editable;
    attr.editable = true;
    FldSetAttributes(fldP, &attr);

    len = FldGetTextLength(fldP);
    if (len) FldDelete(fldP, 0, len);
    FldInsert(fldP, s, StrLen(s));

    attr.editable = old;
    FldSetAttributes(fldP, &attr);

    if (focus) fldP->attr.hasFocus = true;
  }
  OUTV;
}
