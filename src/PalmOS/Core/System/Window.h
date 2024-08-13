/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: Window.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *        This file defines window structures and routines that support color.
 *
 *****************************************************************************/

#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <PalmTypes.h>
#include <CoreTraps.h>
#ifdef PALMOS
#include <FixedMath.h>
#endif
#include <Font.h>
#include <Rect.h>
#include <Bitmap.h>

#include <PalmOptErrorCheckLevel.h>	// #define ERROR_CHECK_LEVEL

#define 	kWinVersion		5

// enum for WinScrollRectangle
typedef enum WinDirectionTag 
{ 
	winUp = 0, 
	winDown, 
	winLeft, 
	winRight 
} WinDirectionType;


// enum for WinCreateOffscreenWindow
typedef enum WindowFormatTag 
{ 
	screenFormat = 0, 
	genericFormat, 
	nativeFormat 
} WindowFormatType;


// enum for WinLockScreen
typedef enum WinLockInitTag 
{
	winLockCopy, 
	winLockErase, 
	winLockDontCare
} WinLockInitType;


// operations for the WinScreenMode function
typedef enum WinScreenModeOperationTag 
{
	winScreenModeGetDefaults,
	winScreenModeGet,
	winScreenModeSetToDefaults,
	winScreenModeSet,
	winScreenModeGetSupportedDepths,
	winScreenModeGetSupportsColor
} WinScreenModeOperation;


// Operations for the WinPalette function
#define winPaletteGet 				0
#define winPaletteSet 				1
#define winPaletteSetToDefault		2
#define winPaletteInit				3		// for internal use only


// transfer modes for color drawing
typedef enum WinDrawOperationTag 
{
	winPaint, 
	winErase, 
	winMask, 
	winInvert, 
	winOverlay, 
	winPaintInverse, 
	winSwap
} WinDrawOperation;


typedef enum PatternTag 
{ 
	blackPattern, 
	whitePattern, 
	grayPattern, 
	customPattern,
	lightGrayPattern,
	darkGrayPattern
} PatternType;

#define noPattern				blackPattern
#define grayHLinePattern		0xAA
#define grayHLinePatternOdd		0x55


// grayUnderline means dotted current foreground color
// solidUnderline means solid current foreground color
// colorUnderline redundant, use solidUnderline instead
typedef enum UnderlineModeTag 
{ 
	noUnderline, 
	grayUnderline, 
	solidUnderline, 
	colorUnderline 
} UnderlineModeType;

typedef UInt8 IndexedColorType;			// 1, 2, 4, or 8 bit index

typedef UInt8 CustomPatternType[8];		// 8x8 1-bit deep pattern


// for WinPalette startIndex value, respect indexes in passed table
#define WinUseTableIndexes -1


// constants used by WinSetCoordinateSystem
#define kCoordinatesNative			0
#define kCoordinatesStandard		72
#define kCoordinatesOneAndAHalf		108
#define kCoordinatesDouble			144
#define kCoordinatesTriple			216
#define kCoordinatesQuadruple		288


// selectors for WinScreenGetAttribute
typedef enum WinScreenAttrTag 
{
	winScreenWidth,
	winScreenHeight,
	winScreenRowBytes,
	winScreenDepth,
	winScreenAllDepths,
	winScreenDensity,
	winScreenPixelFormat,
	winScreenResolutionX,
	winScreenResolutionY
} WinScreenAttrType;


// flags for WinSetScalingMode
#define kBitmapScalingOff		1
#define kTextScalingOff			2
#define kTextPaddingOff			4


typedef struct DrawStateFlagsType
#ifdef ALLOW_ACCESS_TO_INTERNALS_OF_WINDOWS	// These fields will not be available in the next OS release!
{
	UInt16	unscaledBitmaps:1;			// if set, do not scale bitmaps
	UInt16	unscaledText:1;				// if set, do not scale text
	UInt16	unpaddedText:1;				// if set, do not insert pixels between glyphs
	UInt16	useFloor:1;						// used internally by OS
	UInt16 	reserved:12;
}
#endif
DrawStateFlagsType;


//-----------------------------------------------
// Draw state structure
//-----------------------------------------------

typedef struct DrawStateType
#ifdef ALLOW_ACCESS_TO_INTERNALS_OF_WINDOWS	// These fields will not be available in the next OS release!
{
	WinDrawOperation	transferMode;
	PatternType			pattern;
	UnderlineModeType	underlineMode;
	FontID				fontId;
	FontPtr				font;
	CustomPatternType	patternData;
	
	// These are only valid for indexed color bitmaps
	IndexedColorType	foreColor;
	IndexedColorType	backColor;
	IndexedColorType	textColor;
	UInt8				reserved;	

	// These are only valid for direct color bitmaps
	RGBColorType		foreColorRGB;
	RGBColorType		backColorRGB;
	RGBColorType		textColorRGB;
	
	UInt16				coordinateSystem;		// active coordinate system
	DrawStateFlagsType 	flags;
#ifdef PALMOS
	FixedType			scale;					// active to native scaling factor
	FixedType			ntvToActiveScale;		// native to active scaling factor, inverse of scale
	FixedType			stdToActiveScale;		// standard to active scaling factor
	FixedType			activeToStdScale;		// active to standard scaling factor, inverse of stdToActive
#endif
}
#endif
DrawStateType;


#define DrawStateStackSize 7		// enough to draw things immediately if we're in an update




//-----------------------------------------------
// The Window Structures.
//-----------------------------------------------

typedef union FrameBitsType 
{
	struct 
	{
		UInt16 cornerDiam		: 8;				// corner diameter, max 38
		UInt16 reserved_3		: 3; 
		UInt16 threeD			: 1;				// Draw 3D button    
		UInt16 shadowWidth		: 2;				// Width of shadow
		UInt16 width			: 2;				// Width frame
	} bits;
	UInt16 word;									// IMPORTANT: INITIALIZE word to zero before setting bits!
} FrameBitsType;

typedef UInt16 FrameType;

//  Standard Frame Types
#define noFrame         0
#define simpleFrame     1
#define rectangleFrame  1
#define simple3DFrame   0x0012			 // 3d, frame = 2
#define roundFrame      0x0401          // corner = 7, frame = 1
#define boldRoundFrame  0x0702          // corner = 7, frame = 2
#define popupFrame      0x0205          // corner = 2,  frame = 1, shadow = 1
#define dialogFrame     0x0302          // corner = 3,  frame = 2
#define menuFrame       popupFrame


#define winDefaultDepthFlag		0xFF

typedef struct WindowFlagsType
#ifdef ALLOW_ACCESS_TO_INTERNALS_OF_WINDOWS	// These fields will not be available in the next OS release!
{
	UInt16 format:1;      // window format:  0=screen mode; 1=generic mode
	UInt16 offscreen:1;   // offscreen flag: 0=onscreen ; 1=offscreen
	UInt16 modal:1;       // modal flag:     0=modeless window; 1=modal window
	UInt16 focusable:1;   // focusable flag: 0=non-focusable; 1=focusable
	UInt16 enabled:1;     // enabled flag:   0=disabled; 1=enabled
	UInt16 visible:1;     // visible flag:   0-invisible; 1=visible
	UInt16 dialog:1;      // dialog flag:    0=non-dialog; 1=dialog
	UInt16 freeBitmap:1;  // free bitmap w/window: 0=don't free, 1=free
	UInt16 reserved:8;
}
#endif
WindowFlagsType;

typedef struct WindowType
#ifdef ALLOW_ACCESS_TO_INTERNALS_OF_WINDOWS	// These fields will not be available in the next OS release!
{
  // hack to allow 68K programs to access some fields inside WindowType.
  // pad accounts for displayWidthV20, displayHeightV20, displayAddrV20 and windowFlags.
  // x, y, w, h allow programs to access windowBounds (SubHunt does this)

  //uint16_t pad[5];
  uint16_t w_v20, h_v20;
  uint32_t addr_v20;
  uint16_t flags;
  int16_t x, y, w, h;
  int16_t cx, cy, cw, ch;
  uint32_t bitmap;

  Coord					displayWidthV20;		// use WinGetDisplayExtent instead
  Coord					displayHeightV20;		// use WinGetDisplayExtent instead
  void					*displayAddrV20;		// use the drawing functions instead
  WindowFlagsType			windowFlags;
  RectangleType			windowBounds;
  AbsRectType				clippingBounds;
  BitmapPtr				bitmapP;
  FrameBitsType   		frameType;
  DrawStateType		*drawStateP;				// was GraphicStatePtr
  struct WindowType	*nextWindow;

  Coord minH, prefH, maxH;
  Coord minW, prefW, maxW;

  // density is stored inside BitmapV3,
  // it is replicated here for performance reasons
  UInt16 density;
}
#endif
WindowType;

typedef WindowType *WinPtr;
typedef WindowType *WinHandle;


//-----------------------------------------------
//  More graphics shapes
//-----------------------------------------------
typedef struct WinLineType 
{
	Coord x1;
	Coord y1;
	Coord x2;
	Coord y2;
} WinLineType;

// Rectangles, Points defined in Rect.h


//-----------------------------------------------
//  Low Memory Globals
//-----------------------------------------------

// This is the structure of a low memory global reserved for the Window Manager
// In GRAPHIC_VERSION_2, it held a single drawing state.  In this version, it
// holds stack information for structures that are allocated from the dynamic heap
typedef struct GraphicStateType
#ifdef ALLOW_ACCESS_TO_INTERNALS_OF_WINDOWS	// These fields will not be available in the next OS release!
{
	DrawStateType*		drawStateP;
	DrawStateType*		drawStateStackP;
	Int16				drawStateIndex;
	UInt16				unused;				// was screenLockCount
}
#endif
GraphicStateType;

// ----------------------
// Window manager errors
// ----------------------
#define	winErrPalette							(winErrorClass | 1)



//-----------------------------------------------
//  Macros
//-----------------------------------------------

// For now, the window handle is a pointer to a window structure,
// this however may change, so use the following macros. 

#define WinGetWindowPointer(winHandle) 	((WindowType*) winHandle)
#define WinGetWindowHandle(winPtr) 			((WinHandle) winPtr)

#if ERROR_CHECK_LEVEL == ERROR_CHECK_FULL
#define ECWinValidateHandle(winHandle) 	WinValidateHandle(winHandle)
#else
#define ECWinValidateHandle(winHandle) 
#endif


// Selectors for the PIN trap dispatcher
#define	pinPINSetInputAreaState			  0
#define pinPINGetInputAreaState			  1
#define pinPINSetInputTriggerState		  2
#define pinPINGetInputTriggerState	 	  3
#define pinPINAltInputSystemEnabled		  4
#define pinPINGetCurrentPinletName		  5
#define pinPINSwitchToPinlet			  6
#define pinPINCountPinlets				  7
#define pinPINGetPinletInfo				  8
#define pinPINSetInputMode				  9
#define pinPINGetInputMode				  10
#define pinPINClearPinletState			  11
#define pinPINShowReferenceDialog		  12
#define pinWinSetConstraintsSize		  13
#define pinFrmSetDIAPolicyAttr			  14
#define pinFrmGetDIAPolicyAttr			  15
#define pinStatHide						  16
#define pinStatShow						  17
#define pinStatGetAttribute				  18



#define pinSysGetOrientation              19
#define pinSysSetOrientation              20
#define pinSysGetOrientationTriggerState  21
#define pinSysSetOrientationTriggerState  22



// If nobody has explicitly specified whether the high density trap dispatcher 
// should be used, set it based on the emulation level.
#ifndef USE_PINSDISPATCH_TRAPS
	#if EMULATION_LEVEL == EMULATION_NONE
		#define	USE_PINSDISPATCH_TRAPS	1
	#else
		#define	USE_PINSDISPATCH_TRAPS	0
	#endif
#endif

#if USE_PINSDISPATCH_TRAPS
	#define PINS_TRAP(selector)	\
		_SYSTEM_API(_CALL_WITH_SELECTOR)(_SYSTEM_TABLE, sysTrapPinsDispatch, selector)
#else
	#define PINS_TRAP(selector)
#endif
	
	
#ifdef __cplusplus
extern "C" {
#endif

//-----------------------------------------------
// Routines relating to windows management       
//-----------------------------------------------

Boolean WinValidateHandle (WinHandle winHandle)
							SYS_TRAP(sysTrapWinValidateHandle);
							
WinHandle WinCreateWindow (const RectangleType *bounds, FrameType frame, 
	Boolean modal, Boolean focusable, UInt16 *error)
							SYS_TRAP(sysTrapWinCreateWindow);

WinHandle WinCreateOffscreenWindow (Coord width, Coord height, 
	WindowFormatType format, UInt16 *error)
							SYS_TRAP(sysTrapWinCreateOffscreenWindow);

WinHandle WinCreateBitmapWindow (BitmapType *bitmapP, UInt16 *error)
							SYS_TRAP(sysTrapWinCreateBitmapWindow);

void WinDeleteWindow (WinHandle winHandle, Boolean eraseIt)
							SYS_TRAP(sysTrapWinDeleteWindow);

void WinInitializeWindow (WinHandle winHandle)
							SYS_TRAP(sysTrapWinInitializeWindow);

void WinAddWindow (WinHandle winHandle)
							SYS_TRAP(sysTrapWinAddWindow);

void WinRemoveWindow (WinHandle winHandle)
							SYS_TRAP(sysTrapWinRemoveWindow);

void WinMoveWindowAddr (WindowType *oldLocationP, WindowType *newLocationP)
							SYS_TRAP(sysTrapWinMoveWindowAddr);

void WinSetActiveWindow (WinHandle winHandle)
							SYS_TRAP(sysTrapWinSetActiveWindow);

WinHandle WinSetDrawWindow (WinHandle winHandle)
							SYS_TRAP(sysTrapWinSetDrawWindow);

WinHandle WinGetDrawWindow (void)
							SYS_TRAP(sysTrapWinGetDrawWindow);

WinHandle WinGetActiveWindow (void)
							SYS_TRAP(sysTrapWinGetActiveWindow);

WinHandle WinGetDisplayWindow (void)
							SYS_TRAP(sysTrapWinGetDisplayWindow);

WinHandle WinGetFirstWindow (void)
							SYS_TRAP(sysTrapWinGetFirstWindow);

void WinEnableWindow (WinHandle winHandle)
							SYS_TRAP(sysTrapWinEnableWindow);

void WinDisableWindow (WinHandle winHandle)
							SYS_TRAP(sysTrapWinDisableWindow);

void WinGetWindowFrameRect (WinHandle winHandle, RectangleType *r)
							SYS_TRAP(sysTrapWinGetWindowFrameRect);

void WinDrawWindowFrame (void)
							SYS_TRAP(sysTrapWinDrawWindowFrame);

void WinEraseWindow (void)
							SYS_TRAP(sysTrapWinEraseWindow);

WinHandle WinSaveBits (const RectangleType *source, UInt16 *error)
							SYS_TRAP(sysTrapWinSaveBits);

void WinRestoreBits (WinHandle winHandle, Coord destX, Coord destY)
							SYS_TRAP(sysTrapWinRestoreBits);

void WinCopyRectangle (WinHandle srcWin, WinHandle dstWin, 
	const RectangleType *srcRect, Coord destX, Coord destY, WinDrawOperation mode)
							SYS_TRAP(sysTrapWinCopyRectangle);

void WinScrollRectangle (const RectangleType *rP, WinDirectionType direction,
	Coord distance, RectangleType *vacatedP)
							SYS_TRAP(sysTrapWinScrollRectangle);

void WinGetDisplayExtent (Coord *extentX, Coord *extentY)
							SYS_TRAP(sysTrapWinGetDisplayExtent);

void WinGetDrawWindowBounds (RectangleType *rP)
							SYS_TRAP(sysTrapWinGetDrawWindowBounds);

void WinGetBounds (WinHandle winH, RectangleType *rP)
							SYS_TRAP(sysTrapWinGetBounds);

void WinSetBounds (WinHandle winHandle, const RectangleType *rP)
							SYS_TRAP(sysTrapWinSetBounds);
							
#ifdef ALLOW_OLD_API_NAMES

#define WinGetWindowBounds(rP)				(WinGetDrawWindowBounds((rP)))
#define WinSetWindowBounds(winH, rP)		(WinSetBounds((winH), (rP)))

#endif

void WinGetWindowExtent (Coord *extentX, Coord *extentY)
							SYS_TRAP(sysTrapWinGetWindowExtent);

void WinDisplayToWindowPt (Coord *extentX, Coord *extentY)
							SYS_TRAP(sysTrapWinDisplayToWindowPt);

void WinWindowToDisplayPt (Coord *extentX, Coord *extentY)
							SYS_TRAP(sysTrapWinWindowToDisplayPt);

BitmapType *WinGetBitmap (WinHandle winHandle)
							SYS_TRAP(sysTrapWinGetBitmap);

void WinGetClip (RectangleType *rP)
							SYS_TRAP(sysTrapWinGetClip);

void WinSetClip (const RectangleType *rP)
							SYS_TRAP(sysTrapWinSetClip);

void WinResetClip (void)
							SYS_TRAP(sysTrapWinResetClip);

void WinClipRectangle (RectangleType *rP)
							SYS_TRAP(sysTrapWinClipRectangle);

Boolean WinModal (WinHandle winHandle)
							SYS_TRAP(sysTrapWinModal);

//-----------------------------------------------
// Routines to draw shapes or frames shapes      
//-----------------------------------------------

// Pixel(s)
IndexedColorType WinGetPixel (Coord x, Coord y)
							SYS_TRAP(sysTrapWinGetPixel);

Err WinGetPixelRGB (Coord x, Coord y, RGBColorType* rgbP) 	// Direct color version
                  SYS_TRAP (sysTrapWinGetPixelRGB);

void WinPaintPixel (Coord x, Coord y)								// uses drawing mode
							SYS_TRAP(sysTrapWinPaintPixel);

void WinDrawPixel (Coord x, Coord y)
							SYS_TRAP(sysTrapWinDrawPixel);

void WinErasePixel (Coord x, Coord y)
							SYS_TRAP(sysTrapWinErasePixel);

void WinInvertPixel (Coord x, Coord y)
							SYS_TRAP(sysTrapWinInvertPixel);

void WinPaintPixels (UInt16 numPoints, PointType pts[])
							SYS_TRAP(sysTrapWinPaintPixels);

// Line(s)
void WinPaintLines (UInt16 numLines, WinLineType lines[])
							SYS_TRAP(sysTrapWinPaintLines);

void WinPaintLine (Coord x1, Coord y1, Coord x2, Coord y2)
							SYS_TRAP(sysTrapWinPaintLine);

void WinDrawLine (Coord x1, Coord y1, Coord x2, Coord y2)
							SYS_TRAP(sysTrapWinDrawLine);

void WinDrawGrayLine (Coord x1, Coord y1, Coord x2, Coord y2)
							SYS_TRAP(sysTrapWinDrawGrayLine);

void WinEraseLine (Coord x1, Coord y1, Coord x2, Coord y2)
							SYS_TRAP(sysTrapWinEraseLine);

void WinInvertLine (Coord x1, Coord y1, Coord x2, Coord y2)
							SYS_TRAP(sysTrapWinInvertLine);

void WinFillLine (Coord x1, Coord y1, Coord x2, Coord y2)
							SYS_TRAP(sysTrapWinFillLine);


// Rectangle
void WinPaintRectangle (const RectangleType *rP, UInt16 cornerDiam)
							SYS_TRAP(sysTrapWinPaintRectangle);

void WinDrawRectangle (const RectangleType *rP, UInt16 cornerDiam)
							SYS_TRAP(sysTrapWinDrawRectangle);

void WinEraseRectangle (const RectangleType *rP, UInt16 cornerDiam)
							SYS_TRAP(sysTrapWinEraseRectangle);

void WinInvertRectangle (const RectangleType *rP, UInt16 cornerDiam)
							SYS_TRAP(sysTrapWinInvertRectangle);

void WinFillRectangle (const RectangleType *rP, UInt16 cornerDiam)
							SYS_TRAP(sysTrapWinFillRectangle);

// Rectangle frames
void WinPaintRectangleFrame (FrameType frame, const RectangleType *rP)
							SYS_TRAP(sysTrapWinPaintRectangleFrame);

void WinDrawRectangleFrame (FrameType frame, const RectangleType *rP)
							SYS_TRAP(sysTrapWinDrawRectangleFrame);

void WinDrawGrayRectangleFrame (FrameType frame, const RectangleType *rP)
							SYS_TRAP(sysTrapWinDrawGrayRectangleFrame);

void WinEraseRectangleFrame (FrameType frame, const RectangleType *rP)
							SYS_TRAP(sysTrapWinEraseRectangleFrame);

void WinInvertRectangleFrame (FrameType frame, const RectangleType *rP)
							SYS_TRAP(sysTrapWinInvertRectangleFrame);

void WinGetFramesRectangle (FrameType  frame, const RectangleType *rP, 
	RectangleType *obscuredRect)
							SYS_TRAP(sysTrapWinGetFramesRectangle);


// Bitmap            
void WinDrawBitmap (BitmapPtr bitmapP, Coord x, Coord y)
							SYS_TRAP(sysTrapWinDrawBitmap);

void WinPaintBitmap (BitmapType *bitmapP, Coord x, Coord y)
							SYS_TRAP(sysTrapWinPaintBitmap);


// Characters 
void WinDrawChar (WChar theChar, Coord x, Coord y)
							SYS_TRAP(sysTrapWinDrawChar);

void WinDrawChars (const Char *chars, Int16 len, Coord x, Coord y)
							SYS_TRAP(sysTrapWinDrawChars);

void WinPaintChar (WChar theChar, Coord x, Coord y)
							SYS_TRAP(sysTrapWinPaintChar);

void WinPaintChars (const Char *chars, Int16 len, Coord x, Coord y)
							SYS_TRAP(sysTrapWinPaintChars);

void WinDrawInvertedChars (const Char *chars, Int16 len, Coord x, Coord y)
							SYS_TRAP(sysTrapWinDrawInvertedChars);

void WinDrawTruncChars(const Char *chars, Int16 len, Coord x, Coord y, Coord maxWidth)
							SYS_TRAP(sysTrapWinDrawTruncChars);

void WinEraseChars (const Char *chars, Int16 len, Coord x, Coord y)
							SYS_TRAP(sysTrapWinEraseChars);

void WinInvertChars (const Char *chars, Int16 len, Coord x, Coord y)
							SYS_TRAP(sysTrapWinInvertChars);

UnderlineModeType WinSetUnderlineMode (UnderlineModeType mode)
							SYS_TRAP(sysTrapWinSetUnderlineMode);



//-----------------------------------------------
// Routines for patterns and colors                 
//-----------------------------------------------

void WinPushDrawState (void)	// "save" fore, back, text color, pattern, underline mode, font
							SYS_TRAP(sysTrapWinPushDrawState);

void WinPopDrawState (void)		// "restore" saved drawing variables
							SYS_TRAP(sysTrapWinPopDrawState);


WinDrawOperation WinSetDrawMode (WinDrawOperation newMode)
							SYS_TRAP(sysTrapWinSetDrawMode);


IndexedColorType WinSetForeColor (IndexedColorType foreColor)
							SYS_TRAP(sysTrapWinSetForeColor);

IndexedColorType WinSetBackColor (IndexedColorType backColor)
							SYS_TRAP(sysTrapWinSetBackColor);

IndexedColorType WinSetTextColor (IndexedColorType textColor)
							SYS_TRAP(sysTrapWinSetTextColor);

// Direct color versions
void	 WinSetForeColorRGB (const RGBColorType* newRgbP, RGBColorType* prevRgbP)
			  				SYS_TRAP (sysTrapWinSetForeColorRGB);

void	 WinSetBackColorRGB (const RGBColorType* newRgbP, RGBColorType* prevRgbP)
			  				SYS_TRAP (sysTrapWinSetBackColorRGB);

void	 WinSetTextColorRGB (const RGBColorType* newRgbP, RGBColorType* prevRgbP)
			 				SYS_TRAP (sysTrapWinSetTextColorRGB);

void WinGetPattern (CustomPatternType *patternP)
							SYS_TRAP(sysTrapWinGetPattern);

PatternType WinGetPatternType (void)
							SYS_TRAP(sysTrapWinGetPatternType);

void WinSetPattern (const CustomPatternType *patternP)
							SYS_TRAP(sysTrapWinSetPattern);

void WinSetPatternType (PatternType newPattern)
							SYS_TRAP(sysTrapWinSetPatternType);

Err WinPalette(UInt8 operation, Int16 startIndex, 
			 	  			 UInt16 paletteEntries, RGBColorType *tableP)
							SYS_TRAP(sysTrapWinPalette);

IndexedColorType WinRGBToIndex(const RGBColorType *rgbP)
							SYS_TRAP(sysTrapWinRGBToIndex);

void WinIndexToRGB(IndexedColorType i, RGBColorType *rgbP)
							SYS_TRAP(sysTrapWinIndexToRGB);

// "obsolete" color call, supported for backwards compatibility
void WinSetColors(const RGBColorType *newForeColorP, RGBColorType *oldForeColorP,
									const RGBColorType *newBackColorP, RGBColorType *oldBackColorP)
							SYS_TRAP(sysTrapWinSetColors);


//-----------------------------------------------
// WinScreen functions            
//-----------------------------------------------

void WinScreenInit(void) 
							SYS_TRAP(sysTrapWinScreenInit);

Err WinScreenMode(WinScreenModeOperation operation, 
						UInt32 *widthP,
						UInt32 *heightP, 
						UInt32 *depthP, 
						Boolean *enableColorP)
							SYS_TRAP(sysTrapWinScreenMode);


//-----------------------------------------------
// Screen tracking (double buffering) support            
//-----------------------------------------------
UInt8 *WinScreenLock(WinLockInitType initMode)
							SYS_TRAP(sysTrapWinScreenLock);
							
void WinScreenUnlock(void)
							SYS_TRAP(sysTrapWinScreenUnlock);


//-----------------------------------------------
// High Density support functions
//  (HIGH_DENSITY_TRAP is defined in Bitmap.h)
//-----------------------------------------------
UInt16 	WinSetCoordinateSystem(UInt16 coordSys)
							HIGH_DENSITY_TRAP(HDSelectorWinSetCoordinateSystem);
							
UInt16	WinGetCoordinateSystem(void)
							HIGH_DENSITY_TRAP(HDSelectorWinGetCoordinateSystem);
							
Coord		WinScaleCoord(Coord coord, Boolean ceiling)
							HIGH_DENSITY_TRAP(HDSelectorWinScaleCoord);

Coord		WinUnscaleCoord(Coord coord, Boolean ceiling)
							HIGH_DENSITY_TRAP(HDSelectorWinUnscaleCoord);

void		WinScalePoint(PointType* pointP, Boolean ceiling)
							HIGH_DENSITY_TRAP(HDSelectorWinScalePoint);

void		WinUnscalePoint(PointType* pointP, Boolean ceiling)
							HIGH_DENSITY_TRAP(HDSelectorWinUnscalePoint);

void		WinScaleRectangle(RectangleType* rectP)
							HIGH_DENSITY_TRAP(HDSelectorWinScaleRectangle);

void		WinUnscaleRectangle(RectangleType* rectP)
							HIGH_DENSITY_TRAP(HDSelectorWinUnscaleRectangle);

Err		WinScreenGetAttribute(WinScreenAttrType selector, UInt32* attrP)
							HIGH_DENSITY_TRAP(HDSelectorWinScreenGetAttribute);

void		WinPaintTiledBitmap(BitmapType* bitmapP, RectangleType* rectP)
							HIGH_DENSITY_TRAP(HDSelectorWinPaintTiledBitmap);
							
Err		WinGetSupportedDensity(UInt16* densityP)
							HIGH_DENSITY_TRAP(HDSelectorWinGetSupportedDensity);
							
void 		EvtGetPenNative(WinHandle winH, Int16* pScreenX, Int16* pScreenY, Boolean* pPenDown)
							HIGH_DENSITY_TRAP(HDSelectorEvtGetPenNative);

void 		WinPaintRoundedRectangleFrame(const RectangleType *rP, Coord width, Coord cornerRadius, Coord shadowWidth)
							HIGH_DENSITY_TRAP(HDSelectorWinPaintRoundedRectangleFrame);
							
UInt32	WinSetScalingMode(UInt32 mode)
							HIGH_DENSITY_TRAP(HDSelectorWinSetScalingMode);

UInt32	WinGetScalingMode(void)
							HIGH_DENSITY_TRAP(HDSelectorWinGetScalingMode);
							
						
//-----------------------------------------------
// Pen Input Manager support function
//-----------------------------------------------	
Err		WinSetConstraintsSize(WinHandle winH, Coord minH, Coord prefH, Coord maxH,
							Coord minW, Coord prefW, Coord maxW)
							PINS_TRAP(pinWinSetConstraintsSize);

#ifdef __cplusplus 
}
#endif


#endif //__WINDOW_H__
