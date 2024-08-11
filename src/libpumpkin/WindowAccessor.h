typedef enum {
  WindowFlagFormat,
  WindowFlagOffscreen,
  WindowFlagModal,
  WindowFlagFocusable,
  WindowFlagEnabled,
  WindowFlagVisible,
  WindowFlagDialog,
  WindowFlagFreeBitmap
} WindowFlagSelector;

typedef enum {
  WindowFieldDisplayWidthV20 = 0,
  WindowFieldDisplayHeightV20 = 2,
  WindowFieldDisplayAddrV20 = 4,
  WindowFieldWindowFlags = 8,
  WindowFieldWindowBoundsX = 10,
  WindowFieldWindowBoundsY = 12,
  WindowFieldWindowBoundsW = 14,
  WindowFieldWindowBoundsH = 16,
  WindowFieldClippingBoundsX1 = 18,
  WindowFieldClippingBoundsY1 = 20,
  WindowFieldClippingBoundsX2 = 22,
  WindowFieldClippingBoundsY2 = 24,
  WindowFieldBitmapP = 26,
  WindowFieldFrameType = 30,
  WindowFieldDrawStateP = 32,
  WindowFieldNextWindow = 36
} WindowSelector;

UIntPtr WinGetSetField(WinHandle wh, WindowSelector selector, WindowFlagSelector flagSelector, UIntPtr value, Boolean set);

#define WinGetField(wh, selector) WinGetSetField(bmp, selector, 0, 0, false)
#define WinSetField(wh, selector, value) WinGetSetField(bmp, selector, 0, value, true)
