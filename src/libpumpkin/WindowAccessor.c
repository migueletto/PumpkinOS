#define ALLOW_ACCESS_TO_INTERNALS_OF_WINDOWS

#include <PalmOS.h>

#include "bytes.h"
#include "pumpkin.h"
#include "debug.h"
#include "WindowAccessor.h"

#define GET_SET(f,n)      case f: if (set) wh->n = value; else value = wh->n; break
				#define GET_SET_FLAG(f,n) case f: if (set) wh->windowFlags.n = value; else value = wh->windowFlags.n; break

UIntPtr WinGetSetField(WinHandle wh, WindowSelector selector, WindowFlagSelector flagSelector, UIntPtr value, Boolean set) {
  UInt16 v16;
  UInt32 v32;
  uint8_t *ram;
  int m68k;

  if (wh) {
    m68k = pumpkin_is_m68k();

    if (m68k) {
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
          ram = pumpkin_heap_base();
          if (set) {
            put4b(value - (UIntPtr)ram, (UInt8 *)wh, selector);
          } else {
            get4b(&v32, (UInt8 *)wh, selector);
            value = (UIntPtr)ram + v32;
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
    } else {
      switch (selector) {
        GET_SET(WindowFieldDisplayWidthV20, displayWidthV20);
        GET_SET(WindowFieldDisplayHeightV20, displayHeightV20);
        GET_SET(WindowFieldWindowBoundsX, windowBounds.topLeft.x);
        GET_SET(WindowFieldWindowBoundsY, windowBounds.topLeft.y);
        GET_SET(WindowFieldWindowBoundsW, windowBounds.extent.x);
        GET_SET(WindowFieldWindowBoundsH, windowBounds.extent.y);
        GET_SET(WindowFieldClippingBoundsX1, clippingBounds.left);
        GET_SET(WindowFieldClippingBoundsY1, clippingBounds.top);
        GET_SET(WindowFieldClippingBoundsX2, clippingBounds.right);
        GET_SET(WindowFieldClippingBoundsY2, clippingBounds.bottom);
        GET_SET(WindowFieldFrameType, frameType.word);
        case WindowFieldWindowFlags:
          switch (flagSelector) {
            GET_SET_FLAG(WindowFlagFormat, format);
            GET_SET_FLAG(WindowFlagOffscreen, offscreen);
            GET_SET_FLAG(WindowFlagModal, modal);
            GET_SET_FLAG(WindowFlagFocusable, focusable);
            GET_SET_FLAG(WindowFlagEnabled, enabled);
            GET_SET_FLAG(WindowFlagVisible, visible);
            GET_SET_FLAG(WindowFlagDialog, dialog);
            GET_SET_FLAG(WindowFlagFreeBitmap, freeBitmap);
          }
          break;
        default:
          debug(DEBUG_ERROR, "Bitmap", "invalid window selector %u", selector);
          break;
      }
    }
  }

  return value;
}
