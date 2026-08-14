/******************************************************************************
 *                                                                            *
 *            (C) Copyright 2000-2002, Sony Corporation                       *
 *                                                                            *
 *----------------------------------------------------------------------------*
 *                                                                            *
 *    <IDENTIFICATION>                                                        *
 *       file name    : $Workfile: SonyHRLib.h $
 *                                                                            *
 *    <PROFILE>                                                               *
 *       High Resolution APIs                                                 *
 *                                                                            *
 *    <HISTORY>                                                               *
 *       Started on   : 01/01/17                                              *
 *       Last Modified: $Date: 03/06/17 17:21 $
 *       Ver 0.10 : 2000/12/17                                                *
 *                                                                            *
 ******************************************************************************/
/* this file is best viewed by setting TAB-stop as 3 */

#ifndef __SONYHIGHRESLIB_H__
#define __SONYHIGHRESLIB_H__

#include <SonySystemResources.h>
#include <SonyErrorBase.h>

// BUILDING_APPLICATION
#if BUILDING_APP_OR_LIB_HIGH_RES || CPU_TYPE != CPU_68K
	// direct link to library code
	#define HR_TRAP(trapNum)
#else
	// else someone else is including this public header file; use traps
	#define HR_TRAP(trapNum)	SYS_TRAP(trapNum)
#endif

/********************************************************************
 * define SCREEN SIZE
 ********************************************************************/
 
#define hrWidth			320
#define hrHeight			320
#define stdWidth			160
#define stdHeight			160

/********************************************************************
 * define hrErrorClass
 ********************************************************************/
 
#define hrErrNone			 0
#define hrErrorClass		(sonyHRErrorClass)
#define hrErrParam		(hrErrorClass | 1)
#define hrErrNotOpen		(hrErrorClass | 2)
#define hrErrStillOpen	(hrErrorClass | 3)
#define hrErrNoGlobals	(hrErrorClass | 4)
#define hrErrMemory		(hrErrorClass | 5)
#define hrErrNoFeature	(hrErrorClass | 6)

/********************************************************************
 * define VERSION Info
 ********************************************************************/
#define HR_VERSION_SUPPORT_FNTSIZE	(0x200)

/********************************************************************
 * define High Resolution Font
 ********************************************************************/

enum hrFontID {
	hrTinyFont = 0x00,
	hrTinyBoldFont,
	hrSmallFont,
	hrSmallSymbolFont,
	hrSmallSymbol11Font,
	hrSmallSymbol7Font,
	hrSmallLedFont,
	hrSmallBoldFont,
	hrStdFont,
	hrBoldFont,
	hrLargeFont,
	hrSymbolFont,
	hrSymbol11Font,
	hrSymbol7Font,
	hrLedFont,
	hrLargeBoldFont,
	hrFntAppFontCustomBase = 0x80
};

typedef enum hrFontID HRFontID;

#if CPU_TYPE == CPU_68K
/********************************************************************
 * define HRTrapNumEnum
 ********************************************************************/

typedef enum tagHRTrapNumEnum
{
	HRTrapGetAPIVersion = sysLibTrapCustom,
	HRTrapWinClipRectangle,
	HRTrapWinCopyRectangle,
	HRTrapWinCreateBitmapWindow,
	HRTrapWinCreateOffscreenWindow,
	HRTrapWinCreateWindow,
	HRTrapWinDisplayToWindowPt,
	HRTrapWinDrawBitmap,
	HRTrapWinDrawChar,
	HRTrapWinDrawChars,
	HRTrapWinDrawGrayLine,
	HRTrapWinDrawGrayRectangleFrame,
	HRTrapWinDrawInvertedChars,
	HRTrapWinDrawLine,
	HRTrapWinDrawPixel,
	HRTrapWinDrawRectangle,
	HRTrapWinDrawRectangleFrame,
	HRTrapWinDrawTruncChars,
	HRTrapWinEraseChars,
	HRTrapWinEraseLine,
	HRTrapWinErasePixel,
	HRTrapWinEraseRectangle,
	HRTrapWinEraseRectangleFrame,
	HRTrapWinFillLine,
	HRTrapWinFillRectangle,
	HRTrapWinGetClip,
	HRTrapWinGetDisplayExtent,
	HRTrapWinGetFramesRectangle,
	HRTrapWinGetPixel,
	HRTrapWinGetWindowBounds,
	HRTrapWinGetWindowExtent,
	HRTrapWinGetWindowFrameRect,
	HRTrapWinInvertChars,
	HRTrapWinInvertLine,
	HRTrapWinInvertPixel,
	HRTrapWinInvertRectangle,
	HRTrapWinInvertRectangleFrame,
	HRTrapWinPaintBitmap,
	HRTrapWinPaintChar,
	HRTrapWinPaintChars,
	HRTrapWinPaintLine,
	HRTrapWinPaintLines,
	HRTrapWinPaintPixel,
	HRTrapWinPaintPixels,
	HRTrapWinPaintRectangle,
	HRTrapWinPaintRectangleFrame,
	HRTrapWinRestoreBits,
	HRTrapWinSaveBits,
	HRTrapWinScreenMode,
	HRTrapWinScrollRectangle,
	HRTrapWinSetClip,
	HRTrapWinSetWindowBounds,
	HRTrapWinWindowToDisplayPt,
	HRTrapBmpBitsSize,
	HRTrapBmpSize,
	HRTrapBmpCreate,
	HRTrapFntGetFont,
	HRTrapFntSetFont,
	HRTrapFontSelect,
	HRTrapSystem,
	HRTrapWinGetPixelRGB,
	HRTrapGetInfo,
	HRTrapFntBaseLine,
	HRTrapFntCharHeight,
	HRTrapFntLineHeight,
	HRTrapFntAverageCharWidth,
	HRTrapFntCharWidth,
	HRTrapFntWCharWidth,
	HRTrapFntCharsWidth,
	HRTrapFntWidthToOffset,
	HRTrapFntCharsInWidth,
	HRTrapFntDescenderHeight,
	HRTrapFntLineWidth,
	HRTrapFntWordWrap,
	HRTrapFntWordWrapReverseNLines,
	HRTrapFntGetScrollValues

} HRTrapNumEnum;
#endif


/********************************************************************
 * API Prototypes
 ********************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/********************************************************************
 * Standard library open, close, sleep and wake functions
 ********************************************************************/

extern Err	HROpen(UInt16 refNum)		HR_TRAP(sysLibTrapOpen);

extern Err	HRClose(UInt16 refNum)		HR_TRAP(sysLibTrapClose);

extern Err	HRSleep(UInt16 refNum)		HR_TRAP(sysLibTrapSleep);

extern Err	HRWake(UInt16 refNum)		HR_TRAP(sysLibTrapWake);


/********************************************************************
 * Custom library API functions
 ********************************************************************/

extern Err	HRGetAPIVersion(UInt16 refNum, UInt16 *versionP)
	HR_TRAP(HRTrapGetAPIVersion);

extern void HRWinClipRectangle(UInt16 refNum, RectangleType *rP)
	HR_TRAP(HRTrapWinClipRectangle);

extern void HRWinCopyRectangle(UInt16 refNum, WinHandle srcWin, WinHandle dstWin, RectangleType *srcRect, Coord destX, Coord destY, WinDrawOperation mode)
	HR_TRAP(HRTrapWinCopyRectangle);

extern WinHandle HRWinCreateBitmapWindow(UInt16 refNum, BitmapType *bitmapP, UInt16 *error)
	HR_TRAP(HRTrapWinCreateBitmapWindow);

extern WinHandle HRWinCreateOffscreenWindow(UInt16 refNum, Coord width, Coord height, WindowFormatType format, UInt16 *error)
	HR_TRAP(HRTrapWinCreateOffscreenWindow);

extern WinHandle HRWinCreateWindow(UInt16 refNum, RectangleType *bounds, FrameType frame, Boolean modal, Boolean focusable, UInt16 *error)
	HR_TRAP(HRTrapWinCreateWindow);

extern void HRWinDisplayToWindowPt(UInt16 refNum, Coord *extentX, Coord *extentY)
	HR_TRAP(HRTrapWinDisplayToWindowPt);

extern void HRWinDrawBitmap(UInt16 refNum, BitmapPtr bitmapP, Coord x, Coord Y)
	HR_TRAP(HRTrapWinDrawBitmap);

extern void HRWinDrawChar(UInt16 refNum, WChar theChar, Coord x, Coord Y)
	HR_TRAP(HRTrapWinDrawChar);

extern void HRWinDrawChars(UInt16 refNum, const Char *chars, Int16 len, Coord x, Coord y)
	HR_TRAP(HRTrapWinDrawChars);

extern void HRWinDrawGrayLine(UInt16 refNum, Coord x1, Coord y1, Coord x2, Coord y2)
	HR_TRAP(HRTrapWinDrawGrayLine);

extern void HRWinDrawGrayRectangleFrame(UInt16 refNum, FrameType frame, RectangleType *rP)
	HR_TRAP(HRTrapWinDrawGrayRectangleFrame);

extern void HRWinDrawInvertedChars(UInt16 refNum, const Char *chars, Int16 len, Coord x, Coord y)
	HR_TRAP(HRTrapWinDrawInvertedChars);

extern void HRWinDrawLine(UInt16 refNum, Coord x1, Coord y1, Coord x2, Coord y2)
	HR_TRAP(HRTrapWinDrawLine);

extern void HRWinDrawPixel(UInt16 refNum, Coord x, Coord y)
	HR_TRAP(HRTrapWinDrawPixel);

extern void HRWinDrawRectangle(UInt16 refNum, RectangleType *rP, UInt16 cornerDiam)
	HR_TRAP(HRTrapWinDrawRectangle);

extern void HRWinDrawRectangleFrame(UInt16 refNum, FrameType frame, RectangleType *rP)
	HR_TRAP(HRTrapWinDrawRectangleFrame);

extern void HRWinDrawTruncChars(UInt16 refNum, const Char *chars, Int16 len, Coord x, Coord y, Coord maxWidth)
	HR_TRAP(HRTrapWinDrawTruncChars);

extern void HRWinEraseChars(UInt16 refNum, const Char *chars, Int16 len, Coord x, Coord y)
	HR_TRAP(HRTrapWinEraseChars);

extern void HRWinEraseLine(UInt16 refNum, Coord x1, Coord y1, Coord x2, Coord y2)
	HR_TRAP(HRTrapWinEraseLine);

extern void HRWinErasePixel(UInt16 refNum, Coord x, Coord y)
	HR_TRAP(HRTrapWinErasePixel);

extern void HRWinEraseRectangle(UInt16 refNum, RectangleType *rP, UInt16 cornerDiam)
	HR_TRAP(HRTrapWinEraseRectangle);

extern void HRWinEraseRectangleFrame(UInt16 refNum, FrameType frame, RectangleType *rP)
	HR_TRAP(HRTrapWinEraseRectangleFrame);

extern void HRWinFillLine(UInt16 refNum, Coord x1, Coord y1, Coord x2, Coord y2)
	HR_TRAP(HRTrapWinFillLine);

extern void HRWinFillRectangle(UInt16 refNum, RectangleType *rP, UInt16 cornerDiam)
	HR_TRAP(HRTrapWinFillRectangle);

extern void HRWinGetClip(UInt16 refNum, RectangleType *rP)
	HR_TRAP(HRTrapWinGetClip);

extern void HRWinGetDisplayExtent(UInt16 refNum, Coord *extentX, Coord *extentY)
	HR_TRAP(HRTrapWinGetDisplayExtent);

extern void HRWinGetFramesRectangle(UInt16 refNum, FrameType frame, RectangleType *rP, RectangleType *obscuredRectP)
	HR_TRAP(HRTrapWinGetFramesRectangle);

extern IndexedColorType HRWinGetPixel(UInt16 refNum, Coord x, Coord y)
	HR_TRAP(HRTrapWinGetPixel);

extern void HRWinGetWindowBounds(UInt16 refNum, RectangleType *rP)
	HR_TRAP(HRTrapWinGetWindowBounds);

extern void HRWinGetWindowExtent(UInt16 refNum, Coord *extentX, Coord *extentY)
	HR_TRAP(HRTrapWinGetWindowExtent);

extern void HRWinGetWindowFrameRect(UInt16 refNum, WinHandle winHandle, RectangleType *rP)
	HR_TRAP(HRTrapWinGetWindowFrameRect);

extern void HRWinInvertChars(UInt16 refNum, const Char *chars, Int16 len, Coord x, Coord y)
	HR_TRAP(HRTrapWinInvertChars);

extern void HRWinInvertLine(UInt16 refNum, Coord x1, Coord y1, Coord x2, Coord y2)
	HR_TRAP(HRTrapWinInvertLine);

extern void HRWinInvertPixel(UInt16 refNum, Coord x, Coord y)
	HR_TRAP(HRTrapWinInvertPixel);

extern void HRWinInvertRectangle(UInt16 refNum, RectangleType *rP, UInt16 cornerDiam)
	HR_TRAP(HRTrapWinInvertRectangle);

extern void HRWinInvertRectangleFrame(UInt16 refNum, FrameType frame, RectangleType *rP)
	HR_TRAP(HRTrapWinInvertRectangleFrame);

extern void HRWinPaintBitmap(UInt16 refNum, BitmapType *bitmapP, Coord x, Coord y)
	HR_TRAP(HRTrapWinPaintBitmap);

extern void HRWinPaintChar(UInt16 refNum, WChar theChar, Coord x, Coord y)
	HR_TRAP(HRTrapWinPaintChar);

extern void HRWinPaintChars(UInt16 refNum, const Char *chars, Int16 len, Coord x, Coord y)
	HR_TRAP(HRTrapWinPaintChars);

extern void HRWinPaintLine(UInt16 refNum, Coord x1, Coord y1, Coord x2, Coord y2)
	HR_TRAP(HRTrapWinPaintLine);

extern void HRWinPaintLines(UInt16 refNum, UInt16 numLines, WinLineType lines[])
	HR_TRAP(HRTrapWinPaintLines);

extern void HRWinPaintPixel(UInt16 refNum, Coord x, Coord y)
	HR_TRAP(HRTrapWinPaintPixel);

extern void HRWinPaintPixels(UInt16 refNum, UInt16 numPoints, PointType pts[])
	HR_TRAP(HRTrapWinPaintPixels);

extern void HRWinPaintRectangle(UInt16 refNum, RectangleType *rP, UInt16 cornerDiam)
	HR_TRAP(HRTrapWinPaintRectangle);

extern void HRWinPaintRectangleFrame(UInt16 refNum, FrameType frame, RectangleType *rP)
	HR_TRAP(HRTrapWinPaintRectangleFrame);

extern void HRWinRestoreBits(UInt16 refNum, WinHandle winHandle, Coord destX, Coord destY)
	HR_TRAP(HRTrapWinRestoreBits);

extern WinHandle HRWinSaveBits(UInt16 refNum, RectangleType *sourceP, UInt16 *error)
	HR_TRAP(HRTrapWinSaveBits);

extern Err HRWinScreenMode(UInt16 refNum, WinScreenModeOperation operation, UInt32 *widthP, UInt32 *heightP, UInt32 *depthP, Boolean *enableColorP)
	HR_TRAP(HRTrapWinScreenMode);

extern void HRWinScrollRectangle(UInt16 refNum, RectangleType *rP, WinDirectionType direction, Coord distance, RectangleType *vacatedP)
	HR_TRAP(HRTrapWinScrollRectangle);

extern void HRWinSetClip(UInt16 refNum, RectangleType *rP)
	HR_TRAP(HRTrapWinSetClip);

extern void HRWinSetWindowBounds(UInt16 refNum, WinHandle winHandle, RectangleType *rP)
	HR_TRAP(HRTrapWinSetWindowBounds);

extern void HRWinWindowToDisplayPt(UInt16 refNum, Coord *extentX, Coord *extentY)
	HR_TRAP(HRTrapWinWindowToDisplayPt);

extern Err HRWinGetPixelRGB(UInt16 refNum, Coord x, Coord y, RGBColorType *rgbP)
	HR_TRAP(HRTrapWinGetPixelRGB);



extern UInt32 HRBmpBitsSize(UInt16 refNum, BitmapType *bitmapP)
	HR_TRAP(HRTrapBmpBitsSize);

extern UInt32 HRBmpSize(UInt16 refNum, BitmapType *bitmapP)
	HR_TRAP(HRTrapBmpSize);

extern BitmapType *HRBmpCreate(UInt16 refNum, Coord width, Coord height, UInt8 depth, ColorTableType *colortableP, UInt16 *error)
	HR_TRAP( HRTrapBmpCreate);



extern HRFontID HRFntGetFont(UInt16 refNum)
	HR_TRAP(HRTrapFntGetFont);

extern HRFontID HRFntSetFont(UInt16 refNum, HRFontID font)
	HR_TRAP(HRTrapFntSetFont);

extern HRFontID HRFontSelect(UInt16 refNum, HRFontID font)
	HR_TRAP(HRTrapFontSelect);

extern Int16 HRFntBaseLine (UInt16 refNum)
	HR_TRAP(HRTrapFntBaseLine);

extern Int16 HRFntCharHeight (UInt16 refNum)
	HR_TRAP(HRTrapFntCharHeight);

extern Int16 HRFntLineHeight (UInt16 refNum)
	HR_TRAP(HRTrapFntLineHeight);

extern Int16 HRFntAverageCharWidth (UInt16 refNum)
	HR_TRAP(HRTrapFntAverageCharWidth);

extern Int16 HRFntCharWidth (UInt16 refNum, Char ch)
	HR_TRAP(HRTrapFntCharWidth);

extern Int16 HRFntWCharWidth (UInt16 refNum, WChar iChar)
	HR_TRAP(HRTrapFntWCharWidth);

extern Int16 HRFntCharsWidth (UInt16 refNum, Char const *chars, Int16 len)
	HR_TRAP(HRTrapFntCharsWidth);

extern Int16 HRFntWidthToOffset (UInt16 refNum, Char const *pChars, UInt16 length,
		Int16 pixelWidth, Boolean *leadingEdge, Int16 *truncWidth)
	HR_TRAP(HRTrapFntWidthToOffset);

extern void HRFntCharsInWidth (UInt16 refNum, Char const *string, 
		Int16 *stringWidthP, Int16 *stringLengthP, 
		Boolean *fitWithinWidth)
	HR_TRAP(HRTrapFntCharsInWidth);

extern Int16 HRFntDescenderHeight (UInt16 refNum)
	HR_TRAP(HRTrapFntDescenderHeight);

extern Int16 HRFntLineWidth (UInt16 refNum, Char const *pChars, UInt16 length)
	HR_TRAP(HRTrapFntLineWidth);

extern UInt16 HRFntWordWrap (UInt16 refNum, Char const *chars, UInt16 maxWidth)
	HR_TRAP(HRTrapFntWordWrap);

extern void HRFntWordWrapReverseNLines (UInt16 refNum, Char const *const chars, 
		UInt16 maxWidth, UInt16 *linesToScrollP, UInt16 *scrollPosP)
	HR_TRAP(HRTrapFntWordWrapReverseNLines);

extern void HRFntGetScrollValues (UInt16 refNum, Char const *chars, UInt16 width, 
		UInt16 scrollPos, UInt16 *linesP, UInt16 *topLine)
	HR_TRAP(HRTrapFntGetScrollValues);


/* System Use Only */
extern Err HRSystem(UInt16 refNum, UInt16 operation, UInt32 *param1, UInt32 *param2, UInt32 *param3)
	HR_TRAP(HRTrapSystem);

extern Err HRGetInfo(UInt16 refNum, UInt16 *ptr1, UInt16 *ptr2)
	HR_TRAP(HRTrapGetInfo);
	
/** Number of API : 80 **/

#ifdef __cplusplus
}
#endif

#endif /* __SONYHIGHRESLINB_H__ */
