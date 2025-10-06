/*
	HtmlLib68K.h
	Copyright(c) 1996-2002 ACCESS CO., LTD.
	All rights are reserved by ACCESS CO., LTD., whether the whole or
	part of the source code including any modifications.
*/
#ifndef HTMLLIb68K_H__
#define HTMLLIb68K_H__

#include <PalmTypes.h>
#include <LibTraps.h>

#include <HtmlLibTrap.h>

#include <ErrorBase.h>

#define htmlErrorClass	(appErrorClass)

/************************************************************
 * HtmlLib error codes
 *************************************************************/
#define	htmlErrInUse			(htmlErrorClass | 1)
#define	htmlErrOutOfMemory		(htmlErrorClass | 2)
#define	htmlErrInvalidParam		(htmlErrorClass | 3)		
#define	htmlErrNotFound			(htmlErrorClass | 4)
#define htmlErrNotInitialized	(htmlErrorClass | 5)
#define htmlErrCantOpen			(htmlErrorClass | 6)

#define htmlLibProgressEvent	(0x7000)

/************************************************************
 * HtmlLib l
 *************************************************************/

#define HtmlLibID  		'AsHL'
#define HtmlLibDBType 	'libr'
#define HtmlLibName     "HtmlLibrary"


/* 
 * Typedef
 */

/* HtmlDisplayMode */
#define	htmlHandheldDisplayMode	0x01
#define htmlFaithfulDisplayMode 0x02

typedef UInt8 HtmlDisplayMode;

/* HtmlProgressStatus */
#define htmlProgressDone		0x0001
#define	htmlProgressBusy		0x0002
#define htmlProgressOutOfMemory	0x0003
#define htmlProgressError		0x0004

typedef UInt16 HtmlProgressStatus;

/* HtmlSubmitFormMethod */
#define htmlHttpMethodGet	0x00000001
#define htmlHttpMethodPost	0x00000002

typedef UInt32 HtmlSubmitFormMethod;

/* HtmlDirections */
#define htmlDirLeft			0x01
#define htmlDirRight		0x02
#define htmlDirUp			0x03
#define htmlDirDown			0x04

typedef Int8 HtmlScrollDirection;

/* HtmlScrollMode */
#define htmlNormalScrollbarMode	0x01
#define htmlNoPageScrollbarMode	0x02

typedef Int8 HtmlScrollbarMode;


typedef void HtmlLibLinkSelCallback(MemHandle htmlLibH, Char *url, void *cbData);
typedef Boolean HtmlLibScanPhoneNumberCallback(MemHandle htmlLibH, Char *buffer, Int32 length, Int32 *patternStart, Int32 *patternEnd);
typedef void HtmlLibInclusionCallback(MemHandle htmlLibH, Char *url, void *cbData);
typedef void HtmlLibSubmitFormCallback(MemHandle htmlLibH, Char *url, HtmlSubmitFormMethod method, Char *query, void *cbData);

typedef Boolean NewControlProc(UInt16 id, 
 	UInt16 style, const Char *textP, 
	Coord x, Coord y, Coord width, Coord height, 
	UInt16 font, UInt8 group, Boolean leftAnchor);

typedef Err NewListProc(UInt16 id, 
					Coord x, Coord y, Coord width, Coord height, 
					UInt16 font, Int16 visibleItems, Int16 triggerId);

typedef Boolean NewFieldProc(UInt16 id, 
	Coord x, Coord y, Coord width, Coord height, 
	UInt16 font, UInt32 maxChars, UInt16 editable, UInt16 underlined, 
	UInt16 singleLine, UInt16 dynamicSize, UInt16 justification, 
	UInt16 autoShift, UInt16 hasScrollBar, UInt16 numeric);

typedef void DeleteObjectProc(UInt16 index);

typedef struct {
	NewControlProc *newControlProc;
	NewListProc *newListProc;
	NewFieldProc *newFieldProc;
	DeleteObjectProc *deleteObjectProc;
} HtmlLibFormWidgetProc;


/* 
 * API
 */
Err HtmlLibOpen(UInt16 refnum)
		SYS_TRAP(kHtmlLibTrapOpen);
Err HtmlLibClose(UInt16 refnum)
		SYS_TRAP(kHtmlLibTrapClose);
Err HtmlLibSleep(UInt16 refnum)
		SYS_TRAP(kHtmlLibTrapSleep);
Err HtmlLibWake(UInt16 refnum)
		SYS_TRAP(kHtmlLibTrapWake);

MemHandle HtmlLibInitialize(UInt16 refnum, RectangleType bounds, Err *errP)
		SYS_TRAP(kHtmlLibTrapInitialize);
void HtmlLibFinalize(UInt16 refnum, MemHandle htmllibH)
		SYS_TRAP(kHtmlLibTrapFinalize);

void HtmlLibResizeScreen(UInt16 refnum, MemHandle htmlLibH, RectangleType newBounds)
		SYS_TRAP(kHtmlLibTrapResizeScreen);
void HtmlLibSetDisplayMode(UInt16 refnum, MemHandle htmlLibH, HtmlDisplayMode mode)
		SYS_TRAP(kHtmlLibTrapSetDisplayMode);

void HtmlLibSetLinkSelectionCallback(UInt16 refnum, MemHandle htmlLibH, HtmlLibLinkSelCallback *callback, void *cbData)
		SYS_TRAP(kHtmlLibTrapSetLinkSelectionCallback);
void HtmlLibSetScanPhoneNumberCallback(UInt16 refnum, MemHandle htmlLibH, HtmlLibScanPhoneNumberCallback *callback)
		SYS_TRAP(kHtmlLibTrapSetScanPhoneNumberCallback);
void HtmlLibSetInclusionCallback(UInt16 refnum, MemHandle htmlLibH, HtmlLibInclusionCallback *callback, void *cbData)
		SYS_TRAP(kHtmlLibTrapSetInclusionCallback);
void HtmlLibSetSubmitFormCallback(UInt16 refnum, MemHandle htmlLibH, HtmlLibSubmitFormCallback *callback, void *cbData)
		SYS_TRAP(kHtmlLibTrapSetSubmitFormCallback);

UInt16 HtmlLibProgress(UInt16 refnum, MemHandle htmlLibH, Int32 stayTime)
		SYS_TRAP(kHtmlLibTrapProgress);
Boolean HtmlLibNotifyUIEvent(UInt16 refnum, MemHandle htmlLibH, EventType *eventP)
		SYS_TRAP(kHtmlLibTrapNotifyUIEvent);
Err HtmlLibFindText(UInt16 refnum, MemHandle htmlLibH, Char *searchString, Boolean wrapSearch)
		SYS_TRAP(kHtmlLibTrapFindText);
Boolean HtmlLibGetTextSelection(UInt16 refnum, MemHandle htmlLibH, Char **selectedText)
		SYS_TRAP(kHtmlLibTrapGetTextSelection);
void HtmlLibClearScreen(UInt16 refnum, MemHandle htmlLibH)
		SYS_TRAP(kHtmlLibTrapClearScreen);
void HtmlLibAbortRenderData(UInt16 refnum, MemHandle htmlLibH)
		SYS_TRAP(kHtmlLibTrapAbortRenderData);

MemHandle HtmlLibCreateContentObject(UInt16 refnum, MemHandle htmlLibH)
		SYS_TRAP(kHtmlLibTrapCreateContentObject);
void HtmlLibDestroyContentObject(UInt16 refnum, MemHandle contentH)
		SYS_TRAP(kHtmlLibTrapDestroyContentObject);
Err HtmlLibAddTextData(UInt16 refnum, MemHandle contentH, Char *url, Char *mimeType, Char *charset, void *data, Int32 dataLen)
		SYS_TRAP(kHtmlLibTrapAddTextData);
Err HtmlLibAddImageData(UInt16 refnum, MemHandle contentH, Char *url, Char *mimeType, void *data, Int32 dataLen)
		SYS_TRAP(kHtmlLibTrapAddImageData);
void HtmlLibRenderData(UInt16 refnum, MemHandle contentH)
		SYS_TRAP(kHtmlLibTrapRenderData);
void HtmlLibRedrawScreen(UInt16 refnum, MemHandle htmlLibH)
		SYS_TRAP(kHtmlLibTrapRedrawScreen);
void HtmlLibSetScrollbars(UInt16 refnum, MemHandle htmlLibH, UInt16 *scrollbars, UInt16 size)
		SYS_TRAP(kHtmlLibTrapSetScrollbars);

Err HtmlLibGetVScrollPosition(UInt16 refnum, MemHandle htmlLibH, Int16 *valueP)
		SYS_TRAP(kHtmlLibTrapGetVScrollPosition);
Err HtmlLibSetVScrollPosition(UInt16 refnum, MemHandle htmlLibH, Int16 newPosition)
		SYS_TRAP(kHtmlLibTrapSetVScrollPosition);

Boolean HtmlLibCanScroll(UInt16 refnum, MemHandle htmlLibH, HtmlScrollDirection direction)
		SYS_TRAP(kHtmlLibTrapCanScroll);
Err HtmlLibSetScrollbarMode(UInt16 refnum, MemHandle htmlLibH, HtmlScrollbarMode mode)
		SYS_TRAP(kHtmlLibTrapSetScrollbarMode);
Err HtmlLibSetBiggerFont(UInt16 refnum, MemHandle htmlLibH, Boolean value)
		SYS_TRAP(kHtmlLibTrapSetBiggerFont);

/* 68K only */
void HtmlLibSetFormWidgetProc(UInt16 refnum, MemHandle htmlLibH, HtmlLibFormWidgetProc *proc)
		SYS_TRAP(kHtmlLibTrapSetFormWidgetProc);

#endif /* HTMLLIb68K_H__ */
