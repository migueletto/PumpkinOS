/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: Control.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *	  This file defines check box structures and routines.
 *
 *****************************************************************************/

#ifndef __CONTROL_H__
#define __CONTROL_H__

#include <PalmTypes.h>
#include <CoreTraps.h>
#include <DataMgr.h>
#include <Event.h>

typedef struct ControlAttrTag
#ifdef ALLOW_ACCESS_TO_INTERNALS_OF_CONTROLS	// These fields will not be available in the next OS release!
{
	UInt8 usable				:1;	// set if part of ui 
	UInt8 enabled				:1;	// set if interactable (not grayed out)
	UInt8 visible				:1;	// set if drawn (set internally)
	UInt8 on						:1;	// set if on (checked)
	UInt8 leftAnchor			:1;	// set if bounds expand to the right
	                        		// clear if bounds expand to the left
   UInt8 frame					:3;
	UInt8 drawnAsSelected	:1;	// support for old-style graphic controls
											// where control overlaps a bitmap
	UInt8 graphical			:1;	// set if images are used instead of text
	UInt8 vertical				:1;	// true for vertical sliders
	UInt8 reserved				:5;
}
#endif
ControlAttrType;


enum controlStyles {buttonCtl, pushButtonCtl, checkboxCtl, popupTriggerCtl,
						  selectorTriggerCtl, repeatingButtonCtl, sliderCtl,
						  feedbackSliderCtl, colorTriggerCtl };
typedef enum controlStyles ControlStyleType;

enum buttonFrames {noButtonFrame, standardButtonFrame, boldButtonFrame,
						 rectangleButtonFrame};
typedef enum buttonFrames ButtonFrameType;


typedef struct ControlType
#ifdef ALLOW_ACCESS_TO_INTERNALS_OF_CONTROLS	// These fields will not be available in the next OS release!
{
  UInt8 pad[26];

  Boolean selected;
  UInt16 objIndex;
  void *formP;
  Char *text;	

	UInt16			id;
	RectangleType		bounds;
	DmResID			bitmapID;			// overlays text in ControlType
	DmResID			selectedBitmapID;	// overlays text in ControlType
	ControlAttrType		attr;
	ControlStyleType	style;
	FontID			font;
	UInt8			group;
	UInt8 			reserved;

	UInt16 listID;
  UInt16 len;
  char buf[0];
}
#endif
ControlType;

typedef ControlType *ControlPtr;				// deprecated, use ControlType *

typedef struct ControlType GraphicControlType;

// GraphicControlType *'s can be cast to ControlType *'s and passed to all
// Control API functions (as long as the 'graphical' bit in the attrs is set)

/*
typedef struct GraphicControlType
#ifdef ALLOW_ACCESS_TO_INTERNALS_OF_CONTROLS	// These fields will not be available in the next OS release!
{
	UInt16					id;
	RectangleType			bounds;
	DmResID					bitmapID;			// overlays text in ControlType
	DmResID					selectedBitmapID;	// overlays text in ControlType
	ControlAttrType		attr;
	ControlStyleType		style;
	FontID					unused;
	UInt8						group;
	UInt8 					reserved;

  Boolean selected;
  UInt16 objIndex;
  void *formP;
}
#endif
GraphicControlType;
*/


// SliderControlType *'s can be cast to ControlType *'s and passed to all
// Control API functions (as long as the control style is a slider)

typedef struct SliderControlType
#ifdef ALLOW_ACCESS_TO_INTERNALS_OF_CONTROLS	// These fields will not be available in the next OS release!
{
  UInt16 mId, x, y, w, h;
  UInt32 mText;
  UInt16 mAttr;
  UInt16 pad1, pad2;

  Boolean selected;
  UInt16 objIndex;
  void *formP;
  Char *text;	

	UInt16			id;
	RectangleType		bounds;
	DmResID			thumbID;			// overlays text in ControlType
	DmResID			backgroundID;	// overlays text in ControlType
	ControlAttrType		attr;				// graphical *is* set
	ControlStyleType	style;			// must be sliderCtl or repeatingSliderCtl
	UInt8			reserved;		
	Int16			minValue;
	Int16			maxValue;
	Int16			pageSize;
	Int16			value;
	MemPtr			activeSliderP;
}
#endif
SliderControlType;


//----------------------------------------------------------
//	Control Functions
//----------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

extern void CtlDrawControl (ControlType *controlP)
							SYS_TRAP(sysTrapCtlDrawControl);

extern void CtlEraseControl (ControlType *controlP)
							SYS_TRAP(sysTrapCtlEraseControl);

extern void CtlHideControl (ControlType *controlP)
							SYS_TRAP(sysTrapCtlHideControl);

extern void CtlShowControl (ControlType *controlP)
							SYS_TRAP(sysTrapCtlShowControl);

extern Boolean CtlEnabled (const ControlType *controlP)
							SYS_TRAP(sysTrapCtlEnabled);

extern void CtlSetEnabled (ControlType *controlP, Boolean usable)
							SYS_TRAP(sysTrapCtlSetEnabled);

extern void CtlSetUsable (ControlType *controlP, Boolean usable)
							SYS_TRAP(sysTrapCtlSetUsable);

extern Int16 CtlGetValue (const ControlType *controlP)
							SYS_TRAP(sysTrapCtlGetValue);

extern void CtlSetValue (ControlType *controlP, Int16 newValue)
							SYS_TRAP(sysTrapCtlSetValue);

extern const Char *CtlGetLabel (const ControlType *controlP)
							SYS_TRAP(sysTrapCtlGetLabel);

extern void CtlSetLabel (ControlType *controlP, const Char *newLabel)
							SYS_TRAP(sysTrapCtlSetLabel);

extern void CtlSetGraphics (ControlType *ctlP,
	DmResID newBitmapID, DmResID newSelectedBitmapID)
							SYS_TRAP(sysTrapCtlSetGraphics);

extern void CtlSetSliderValues(ControlType *ctlP, const UInt16 *minValueP, const UInt16 *maxValueP,
					const UInt16 *pageSizeP, const UInt16 *valueP)
							SYS_TRAP(sysTrapCtlSetSliderValues);

extern void CtlGetSliderValues(const ControlType *ctlP, UInt16 *minValueP, UInt16 *maxValueP,
					UInt16 *pageSizeP, UInt16 *valueP)
							SYS_TRAP(sysTrapCtlGetSliderValues);

extern void CtlHitControl (const ControlType *controlP)
							SYS_TRAP(sysTrapCtlHitControl);

extern Boolean CtlHandleEvent (ControlType *controlP, EventType *pEvent)
							SYS_TRAP(sysTrapCtlHandleEvent);

extern Boolean CtlValidatePointer (const ControlType *controlP)
							SYS_TRAP(sysTrapCtlValidatePointer);

extern ControlType *CtlNewControl (void **formPP, UInt16 ID, 
 	ControlStyleType style, const Char *textP, 
	Coord x, Coord y, Coord width, Coord height, 
	FontID font, UInt8 group, Boolean leftAnchor)
							SYS_TRAP(sysTrapCtlNewControl);

extern GraphicControlType *CtlNewGraphicControl (void **formPP, UInt16 ID, 
   ControlStyleType style, DmResID bitmapID, DmResID selectedBitmapID, 
   Coord x, Coord y, Coord width, Coord height, 
   UInt8 group, Boolean leftAnchor)
							SYS_TRAP(sysTrapCtlNewGraphicControl);

extern SliderControlType *CtlNewSliderControl (void **formPP, UInt16 ID, 
   ControlStyleType style, DmResID thumbID, DmResID backgroundID, 
   Coord x, Coord y, Coord width, Coord height, UInt16 minValue, UInt16 maxValue,
   UInt16 pageSize, UInt16 value)
							SYS_TRAP(sysTrapCtlNewSliderControl);

#ifdef __cplusplus 
}
#endif


#endif //__CONTROL_H__
