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
      default:
        break;
    }
  }

  return value;
}
