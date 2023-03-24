#include <PalmOS.h>

#include "gps.h"
#include "app.h"
#include "main.h"
#include "scroll.h"

void UpdateScrollBar(FieldPtr fld, FormPtr frm, UInt16 scrollID)
{
  FieldAttrType attr;

  FldGetAttributes(fld, &attr);

  if (!attr.singleLine && scrollID) {
    UInt16 scrollPos, textHeight, fieldHeight, maxValue, index;
    ScrollBarPtr bar;

    FldGetScrollValues(fld, &scrollPos, &textHeight, &fieldHeight);
    if ((index = FrmGetObjectIndex(frm, scrollID)) == 0xFFFF ||
        FrmGetObjectType(frm, index) != frmScrollBarObj)
      return;
    bar = (ScrollBarPtr)FrmGetObjectPtr(frm, index);

    if (bar) {
      IndexedColorType bg;

      if (textHeight > fieldHeight)
        maxValue = (textHeight - fieldHeight) + FldGetNumberOfBlankLines(fld);
      else if (scrollPos)
        maxValue = scrollPos;
      else
        maxValue = 0;

      bg = WinSetBackColor(0);
      SclSetScrollBar(bar, scrollPos, 0, maxValue, fieldHeight-1);
      WinSetBackColor(bg);
    }
  }
}

void ScrollField(FormPtr frm, UInt16 fieldID, UInt16 scrollID,
                        Int16 linesToScroll, Boolean updateScrollbar)
{
  FieldPtr fld;
  UInt16 blankLines, index;
  WinDirectionType direction;

  index = FrmGetObjectIndex(frm, fieldID);
  fld = (FieldPtr)FrmGetObjectPtr(frm, index);
  direction = linesToScroll < 0 ? winUp : winDown;

  if (fld && FldScrollable(fld, direction)) {
    blankLines = FldGetNumberOfBlankLines(fld);

    if (linesToScroll < 0)
      FldScrollField(fld, -linesToScroll, winUp);
    else if (linesToScroll > 0)
      FldScrollField(fld, linesToScroll, winDown);
    else
      FldScrollField(fld, 0, winDown);

    if (blankLines || updateScrollbar)
      UpdateScrollBar(fld, frm, scrollID);
  }
}
