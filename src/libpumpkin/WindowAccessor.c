#include <PalmOS.h>

#include "bytes.h"
#include "pumpkin.h"
#include "debug.h"
#include "WindowAccessor.h"

UIntPtr WinGetSetField(WinHandle wh, WindowSelector selector, WindowFlagSelector flagSelector, UIntPtr value, Boolean set) {
  UInt16 v16;
  UInt32 v32;
  uint8_t *ram;

  if (wh) {
    switch (selector) {
      case WindowFieldDisplayWidthV20:
      case WindowFieldDisplayHeightV20:
      case WindowFieldWindowBoundsX:
      case WindowFieldWindowBoundsY:
      case WindowFieldWindowBoundsW:
      case WindowFieldWindowBoundsH:
      case WindowFieldClippingBoundsX1:
      case WindowFieldClippingBoundsY1:
      case WindowFieldClippingBoundsX2:
      case WindowFieldClippingBoundsY2:
      case WindowFieldFrameType:
        if (set) {
          put2b(value, (UInt8 *)wh, selector);
        } else {
          get2b(&v16, (UInt8 *)wh, selector);
          value = v16;
        }
        break;
      case WindowFieldBitmapP:
      case WindowFieldDrawStateP:
        ram = pumpkin_heap_base();
        if (set) {
          put4b(value ? (value - (UIntPtr)ram) : 0, (UInt8 *)wh, selector);
        } else {
          get4b(&v32, (UInt8 *)wh, selector);
          value = v32 ? ((UIntPtr)ram + v32) : 0;
        }
        break;
      case WindowFieldWindowFlags:
        get2b(&v16, (UInt8 *)wh, selector);
        if (set) {
          switch (flagSelector) {
            case WindowFlagFormat:     v16 &= 0x7FFF; v16 |= value ? 0x8000 : 0x0000; break;
            case WindowFlagOffscreen:  v16 &= 0xBFFF; v16 |= value ? 0x4000 : 0x0000; break;
            case WindowFlagModal:      v16 &= 0xDFFF; v16 |= value ? 0x2000 : 0x0000; break;
            case WindowFlagFocusable:  v16 &= 0xEFFF; v16 |= value ? 0x1000 : 0x0000; break;
            case WindowFlagEnabled:    v16 &= 0xF7FF; v16 |= value ? 0x0800 : 0x0000; break;
            case WindowFlagVisible:    v16 &= 0xFBFF; v16 |= value ? 0x0400 : 0x0000; break;
            case WindowFlagDialog:     v16 &= 0xFDFF; v16 |= value ? 0x0200 : 0x0000; break;
            case WindowFlagFreeBitmap: v16 &= 0xFEFF; v16 |= value ? 0x0100 : 0x0000; break;
          }
          put2b(v16, (UInt8 *)wh, selector);
        } else {
          switch (flagSelector) {
            case WindowFlagFormat:     value = (v16 & 0x8000) ? 1 : 0; break;
            case WindowFlagOffscreen:  value = (v16 & 0x4000) ? 1 : 0; break;
            case WindowFlagModal:      value = (v16 & 0x2000) ? 1 : 0; break;
            case WindowFlagFocusable:  value = (v16 & 0x1000) ? 1 : 0; break;
            case WindowFlagEnabled:    value = (v16 & 0x0800) ? 1 : 0; break;
            case WindowFlagVisible:    value = (v16 & 0x0400) ? 1 : 0; break;
            case WindowFlagDialog:     value = (v16 & 0x0200) ? 1 : 0; break;
            case WindowFlagFreeBitmap: value = (v16 & 0x0100) ? 1 : 0; break;
          }
        }
        break;
      default:
        debug(DEBUG_ERROR, "Bitmap", "invalid window selector %u", selector);
        break;
    }
  }

  return value;
}

UIntPtr DrawStateGetSetField(WinHandle wh, DrawStateSelector selector, DrawStateFlagSelector flagSelector, UIntPtr value, Boolean set) {
  UInt8 v8;
  UInt32 v32;
  uint8_t *ram, *p;

  if (wh) {
    p = (UInt8 *)wh + WindowFieldDrawStateP;

    switch (selector) {
      case DrawStateTransferMode:
      case DrawStatePattern:
      case DrawStateUnderlineMode:
      case DrawStateFontId:
        if (set) {
          put1(value, p, selector);
        } else {
          get1(&v8, p, selector);
          value = v8;
        }
        break;
      case DrawStateFont:
        ram = pumpkin_heap_base();
        if (set) {
          put4b(value - (UIntPtr)ram, p, selector);
        } else {
          get4b(&v32, p, selector);
          value = (UIntPtr)ram + v32;
        }
        break;
      case DrawStatePatternData:
      case DrawStateForeColor:
      case DrawStateBackColor:
      case DrawStateTextColor:
      case DrawStateReserved:
      case DrawStateForeColorRGB:
      case DrawStateBackColorRGB:
      case DrawStateTextColorRGB:
      case DrawStateCoordinateSystem:
      case DrawStateFlags:
      case DrawStateScale:
      case DrawStateNtvToActiveScale:
      case DrawStateStdToActiveScale:
      case DrawStateActiveToStdScale:
        break;
      default:
        debug(DEBUG_ERROR, "Bitmap", "invalid window selector %u", selector);
        break;
    }
  }

  return value;
}

void encode_drawState(uint8_t *buf, DrawStateType *drawState) {
  uint32_t addr, i;
  uint8_t *ram;

  ram = pumpkin_heap_base();
  put1(drawState->transferMode, buf, DrawStateTransferMode);
  put1(drawState->pattern, buf, DrawStatePattern);
  put1(drawState->underlineMode, buf, DrawStateUnderlineMode);
  put1(drawState->fontId, buf, DrawStateFontId);
  addr = (uint8_t *)drawState->font - ram;
  put4b(addr, buf, DrawStateFont);
  for (i = 0; i < 8; i++) {
    put1(drawState->patternData[i], buf, DrawStatePatternData + i);
  }
}
