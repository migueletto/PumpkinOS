#include <PalmOS.h>

#include "bytes.h"
#include "pumpkin.h"
#include "debug.h"
#include "FormAccessor.h"

UIntPtr FrmObjectGetSetField(void *obj, FormObjectKind kind, UInt16 selector, UInt16 flagSelector, UIntPtr value, Boolean set) {
  UInt8 *ram;
  UInt16 v16;
  UInt32 v32;

  if (obj) {
    switch (kind) {
      case frmTitleObj:
        switch (selector) {
          case FormTitleFieldRectX:
          case FormTitleFieldRectY:
          case FormTitleFieldRectW:
          case FormTitleFieldRectH:
            if (set) {
              put2b(value, (UInt8 *)obj, selector);
            } else {
              get2b(&v16, (UInt8 *)obj, selector);
              value = v16;
            }
            break;
          case FormTitleFieldText:
            ram = pumpkin_heap_base();
            if (set) {
              put4b(value ? (value - (UIntPtr)ram) : 0, (UInt8 *)obj, selector);
            } else {
              get4b(&v32, (UInt8 *)obj, selector);
              value = v32 ? ((UIntPtr)ram + v32) : 0;
            }
            break;
          default:
            break;
        }
        break;
      case frmScrollBarObj:
        switch (selector) {
          case FormScrollBarFieldRectX:
          case FormScrollBarFieldRectY:
          case FormScrollBarFieldRectW:
          case FormScrollBarFieldRectH:
          case FormScrollBarFieldId:
          case FormScrollBarFieldValue:
          case FormScrollBarFieldMinValue:
          case FormScrollBarFieldMaxValue:
          case FormScrollBarFieldPageSize:
          case FormScrollBarFieldPenPos:
          case FormScrollBarFieldSavePos:
            if (set) {
              put2b(value, (UInt8 *)obj, selector);
            } else {
              get2b(&v16, (UInt8 *)obj, selector);
              value = v16;
            }
            break;
          case FormScrollBarFieldAttr:
            get2b(&v16, (UInt8 *)obj, selector);
            if (set) {
              switch (flagSelector) {
                case ScrollBarFlagUsable:       v16 &= 0x7FFF; v16 |= value ? 0x8000 : 0x0000; break;
                case ScrollBarFlagVisible:      v16 &= 0xBFFF; v16 |= value ? 0x4000 : 0x0000; break;
                case ScrollBarFlagHilighted:    v16 &= 0xDFFF; v16 |= value ? 0x2000 : 0x0000; break;
                case ScrollBarFlagShown:        v16 &= 0xEFFF; v16 |= value ? 0x1000 : 0x0000; break;
                case ScrollBarFlagActiveRegion: v16 &= 0xF0FF; v16 |= ((value & 0x0F) << 8); break;
              }
              put2b(v16, (UInt8 *)obj, selector);
            } else {
              switch (flagSelector) {
                case ScrollBarFlagUsable:       value = (v16 & 0x8000) ? 1 : 0; break;
                case ScrollBarFlagVisible:      value = (v16 & 0x4000) ? 1 : 0; break;
                case ScrollBarFlagHilighted:    value = (v16 & 0x2000) ? 1 : 0; break;
                case ScrollBarFlagShown:        value = (v16 & 0x1000) ? 1 : 0; break;
                case ScrollBarFlagActiveRegion: value = (v16 & 0x0F00) >> 8; break;
              }
            }
            break;
          default:
            break;
        }
        break;
      default:
        break;
    }
  }

  return value;
}
