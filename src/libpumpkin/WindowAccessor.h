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

typedef enum {
  DrawStateFlagUnscaledBitmaps,
  DrawStateFlagUnscaledText,
  DrawStateFlagUnpaddedText,
  DrawStateFlagUseFloor,
  DrawStateFlagReserved
} DrawStateFlagSelector;

typedef enum {
  DrawStateTransferMode = 0,
  DrawStatePattern = 1,
  DrawStateUnderlineMode = 2,
  DrawStateFontId = 3,
  DrawStateFont = 4,
  DrawStatePatternData = 8,
  DrawStateForeColor = 16,
  DrawStateBackColor = 17,
  DrawStateTextColor = 18,
  DrawStateReserved = 19,
  DrawStateForeColorRGB = 20,
  DrawStateBackColorRGB = 24,
  DrawStateTextColorRGB = 28,
  DrawStateCoordinateSystem = 32,
  DrawStateFlags = 34,
  DrawStateScale = 36,
  DrawStateNtvToActiveScale = 40,
  DrawStateStdToActiveScale = 44,
  DrawStateActiveToStdScale = 48,
  DrawStateSize = 52
} DrawStateSelector;

UIntPtr WinGetSetField(WinHandle wh, WindowSelector selector, WindowFlagSelector flagSelector, UIntPtr value, Boolean set);
UIntPtr DrawStateGetSetField(WinHandle wh, DrawStateSelector selector, DrawStateFlagSelector flagSelector, UIntPtr value, Boolean set);

#define WinGetField(wh, selector) WinGetSetField((wh), (selector), 0, 0, false)
#define WinSetField(wh, selector, value) WinGetSetField((wh), (selector), 0, (value), true)

#define WinGetFlag(wh, flagSelector) (UInt16)WinGetSetField((wh), WindowFieldWindowFlags, (flagSelector), 0, false)
#define WinSetFlag(wh, flagSelector, value) WinGetSetField((wh), WindowFieldWindowFlags, (flagSelector), (value), true)

#define RctSetRectFromWin(r, wh) do { \
  RctSetRectangle((r), \
    WinGetField((wh), WindowFieldWindowBoundsX), \
    WinGetField((wh), WindowFieldWindowBoundsY), \
    WinGetField((wh), WindowFieldWindowBoundsW), \
    WinGetField((wh), WindowFieldWindowBoundsH)); \
} while(0)

#define RctSetWinFromRect(r, wh) do { \
  WinSetField((wh), WindowFieldWindowBoundsX, r->topLeft.x); \
  WinSetField((wh), WindowFieldWindowBoundsY, r->topLeft.y); \
  WinSetField((wh), WindowFieldWindowBoundsW, r->extent.x); \
  WinSetField((wh), WindowFieldWindowBoundsH, r->extent.y); \
} while(0)

#define RctSetWinFromValues(wh, x, y, w, h) do { \
  WinSetField((wh), WindowFieldWindowBoundsX, x); \
  WinSetField((wh), WindowFieldWindowBoundsY, y); \
  WinSetField((wh), WindowFieldWindowBoundsW, w); \
  WinSetField((wh), WindowFieldWindowBoundsH, h); \
} while(0)

void encode_drawState(uint8_t *buf, DrawStateType *drawState);
