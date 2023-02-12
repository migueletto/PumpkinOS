#include <PalmOS.h>

#include "resource.h"
#include "resedit.h"

Boolean editString(FormType *frm, char *title, MemHandle h) {
  FieldType *fld;
  UInt16 index;

  index = FrmGetObjectIndex(frm, stringFld);
  if ((fld = (FieldType *)FrmGetObjectPtr(frm, index)) != NULL) {
    FldSetTextHandle(fld, h);
    fld->attr.hasFocus = true;
    FrmSetTitle(frm, title);
    FrmDoDialog(frm);
    FldSetTextHandle(fld, NULL);
  }

  return true;
}
