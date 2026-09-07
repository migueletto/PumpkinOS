typedef enum {
  FormTitleFieldRectX = 0,
  FormTitleFieldRectY = 2,
  FormTitleFieldRectW = 4,
  FormTitleFieldRectH = 6,
  FormTitleFieldText  = 8
} FormTitleSelector;

typedef enum {
  FormScrollBarFieldRectX    = 0,
  FormScrollBarFieldRectY    = 2,
  FormScrollBarFieldRectW    = 4,
  FormScrollBarFieldRectH    = 6,
  FormScrollBarFieldId       = 8,
  FormScrollBarFieldAttr     = 10,
  FormScrollBarFieldValue    = 12,
  FormScrollBarFieldMinValue = 14,
  FormScrollBarFieldMaxValue = 16,
  FormScrollBarFieldPageSize = 18,
  FormScrollBarFieldPenPos   = 20,
  FormScrollBarFieldSavePos  = 22
} FormScrollBarSelector;

typedef enum {
  ScrollBarFlagUsable,
  ScrollBarFlagVisible,
  ScrollBarFlagHilighted,
  ScrollBarFlagShown,
  ScrollBarFlagActiveRegion
} ScrollBarFlagSelector;

UIntPtr FrmObjectGetSetField(void *obj, FormObjectKind kind, UInt16 selector, UInt16 flagSelector, UIntPtr value, Boolean set);

#define FrmObjectGetField(obj, kind, selector) FrmObjectGetSetField((obj), (kind), (selector), 0, 0, false)
#define FrmObjectSetField(obj, kind, selector, value) FrmObjectGetSetField((obj), (kind), (selector), 0, (value), true)

#define FrmObjectGetFlag(obj, kind, selector, flagSelector) FrmObjectGetSetField((obj), (kind), (selector), (flagSelector), 0, false)
#define FrmObjectSetFlag(obj, kind, selector, flagSelector, value) FrmObjectGetSetField((obj), (kind), (selector), (flagSelector), (value), true)

#define RctSetRectFromAddr(r, p, offset) do { \
  RctSetRectangle((r), \
    FrmObjectGetField((UInt8 *)(p) + (offset), frmTitleObj, FormTitleFieldRectX), \
    FrmObjectGetField((UInt8 *)(p) + (offset), frmTitleObj, FormTitleFieldRectY), \
    FrmObjectGetField((UInt8 *)(p) + (offset), frmTitleObj, FormTitleFieldRectW), \
    FrmObjectGetField((UInt8 *)(p) + (offset), frmTitleObj, FormTitleFieldRectH)); \
} while(0)

#define RctSetAddrFromRect(r, p, offset) do { \
  FrmObjectSetField((UInt8 *)(p) + (offset), frmTitleObj, FormTitleFieldRectX, (r)->topLeft.x); \
  FrmObjectSetField((UInt8 *)(p) + (offset), frmTitleObj, FormTitleFieldRectY, (r)->topLeft.y); \
  FrmObjectSetField((UInt8 *)(p) + (offset), frmTitleObj, FormTitleFieldRectW, (r)->extent.x); \
  FrmObjectSetField((UInt8 *)(p) + (offset), frmTitleObj, FormTitleFieldRectH, (r)->extent.y); \
} while(0)

