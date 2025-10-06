/*
	HtmlLib68K.h
	Copyright(c) 1996-2002 ACCESS CO., LTD.
	All rights are reserved by ACCESS CO., LTD., whether the whole or
	part of the source code including any modifications.
*/
#ifndef HTMLLIbTRAP_H__
#define HTMLLIbTRAP_H__

#ifdef HERCULES_NATIVE
#include <LibTraps68K.h>
#else
#include <LibTraps.h>
#endif

/* const */
#define kHtmlLibTrapOpen sysLibTrapOpen
#define kHtmlLibTrapClose sysLibTrapClose
#define kHtmlLibTrapSleep sysLibTrapSleep
#define kHtmlLibTrapWake sysLibTrapWake
#define kHtmlLibTrapInitialize		(sysLibTrapCustom + 0)
#define kHtmlLibTrapFinalize		(sysLibTrapCustom + 1)
#define kHtmlLibTrapResizeScreen	(sysLibTrapCustom + 2)
#define kHtmlLibTrapSetDisplayMode  (sysLibTrapCustom + 3)
#define kHtmlLibTrapSetLinkSelectionCallback	(sysLibTrapCustom + 4)
#define kHtmlLibTrapSetScanPhoneNumberCallback	(sysLibTrapCustom + 5)
#define kHtmlLibTrapSetInclusionCallback		(sysLibTrapCustom + 6)
#define kHtmlLibTrapSetSubmitFormCallback		(sysLibTrapCustom + 7)
#define kHtmlLibTrapProgress			(sysLibTrapCustom + 8)
#define kHtmlLibTrapNotifyUIEvent		(sysLibTrapCustom + 9)
#define kHtmlLibTrapFindText			(sysLibTrapCustom + 10)
#define kHtmlLibTrapGetTextSelection	(sysLibTrapCustom + 11)
#define kHtmlLibTrapClearScreen			(sysLibTrapCustom + 12)
#define kHtmlLibTrapAbortRenderData		(sysLibTrapCustom + 13)
#define kHtmlLibTrapCreateContentObject		(sysLibTrapCustom + 14)
#define kHtmlLibTrapDestroyContentObject	(sysLibTrapCustom + 15)
#define kHtmlLibTrapAddTextData				(sysLibTrapCustom + 16)
#define kHtmlLibTrapRenderData				(sysLibTrapCustom + 17)
#define kHtmlLibTrapRedrawScreen			(sysLibTrapCustom + 18)
#define kHtmlLibTrapSetScrollbars			(sysLibTrapCustom + 19)
#define kHtmlLibTrapGetVScrollPosition		(sysLibTrapCustom + 20)
#define kHtmlLibTrapSetVScrollPosition		(sysLibTrapCustom + 21)
#define kHtmlLibTrapCanScroll				(sysLibTrapCustom + 22)
#define kHtmlLibTrapSetScrollbarMode		(sysLibTrapCustom + 23)
#define kHtmlLibTrapAddImageData			(sysLibTrapCustom + 24)
#define kHtmlLibTrapSetBiggerFont			(sysLibTrapCustom + 25)
#define kHtmlLibTrapSetFormWidgetProc		(sysLibTrapCustom + 26)


#endif /* HTMLLIbTRAP_H__ */
