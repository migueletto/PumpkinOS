/******************************************************************************
 *
 * Copyright (c) 1996-2003 PalmSource, Inc. All rights reserved.
 *
 * File: ScrollBar.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *	  This file defines scroll bar structures and routines.
 *
 *****************************************************************************/

#ifndef __SCROLLBAR_H__
#define __SCROLLBAR_H__

#include <PalmTypes.h>
#include <CoreTraps.h>

#include <Rect.h>
#include <Event.h>

typedef enum { sclUpArrow, sclDownArrow, sclUpPage, sclDownPage, sclCar } 
	ScrollBarRegionType;


typedef struct ScrollBarAttrTag
#ifdef ALLOW_ACCESS_TO_INTERNALS_OF_SCROLLBARS	// These fields will not be available in the next OS release!
{
	UInt16 usable			:1;		// Set if part of ui 
	UInt16 visible			:1;		// Set if drawn, used internally
	UInt16 hilighted		:1;		// Set if region is hilighted
	UInt16 shown			:1;		// Set if drawn and maxValue > minValue
	UInt16 activeRegion	:4;		// ScrollBarRegionType
	UInt16 reserved		:8;		// Reserved for future use
}
#endif
ScrollBarAttrType;


typedef struct ScrollBarType
#ifdef ALLOW_ACCESS_TO_INTERNALS_OF_SCROLLBARS	// These fields will not be available in the next OS release!
{
	uint16_t x, y, w, h; // for 68K programs accessing fields directly

	RectangleType		bounds;
	UInt16				id;
	ScrollBarAttrType	attr;
	Int16					value;
	Int16					minValue;
	Int16					maxValue;
	Int16					pageSize;
	Int16					penPosInCar;
	Int16					savePos;
}
#endif
ScrollBarType;

typedef ScrollBarType *ScrollBarPtr;

#ifdef __cplusplus
extern "C" {
#endif

extern void 	SclGetScrollBar (const ScrollBarType *bar, Int16 *valueP, 
	Int16 *minP, Int16 *maxP, Int16 *pageSizeP)
							SYS_TRAP(sysTrapSclGetScrollBar);

extern void		SclSetScrollBar (ScrollBarType *bar, Int16 value, 
						Int16 min, Int16 max, Int16 pageSize)
							SYS_TRAP(sysTrapSclSetScrollBar);

extern void		SclDrawScrollBar (ScrollBarType *bar)
							SYS_TRAP(sysTrapSclDrawScrollBar);

extern Boolean	SclHandleEvent (ScrollBarType *bar, const EventType *event)
							SYS_TRAP(sysTrapSclHandleEvent);

#ifdef __cplusplus 
}
#endif


#endif //__SCROLLBAR_H__
