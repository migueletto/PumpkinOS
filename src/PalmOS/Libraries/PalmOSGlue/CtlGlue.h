/******************************************************************************
 *
 * Copyright (c) 2001-2003 PalmSource, Inc. All rights reserved.
 *
 * File: CtlGlue.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *		Glue providing compatibility for applications that wish
 *		to make calls to the Control Manager, but which might be running
 *		on a system which does not support newer calls.
 *
 *****************************************************************************/

#ifndef __CTLGLUE_H__
#define __CTLGLUE_H__

#include <Control.h>

#ifdef __cplusplus
	extern "C" {
#endif

extern ControlStyleType CtlGlueGetControlStyle(const ControlType *ctlP);

extern FontID CtlGlueGetFont (const ControlType *ctlP);
extern void CtlGlueSetFont (ControlType *ctlP, FontID fontID);
extern void CtlGlueGetGraphics (const ControlType *ctlP, DmResID *bitmapID, DmResID *selectedBitmapID);
extern SliderControlType* CtlGlueNewSliderControl (void **formPP, UInt16 ID, 
	ControlStyleType style, DmResID thumbID, DmResID backgroundID, 
	Coord x, Coord y, Coord width, Coord height, UInt16 minValue, UInt16 maxValue,
	UInt16 pageSize, UInt16 value);
extern void CtlGlueSetLeftAnchor (ControlType *ctlP, Boolean leftAnchor);

extern Boolean CtlGlueIsGraphical(ControlType* controlP);
extern void CtlGlueSetFrameStyle(ControlType* controlP, ButtonFrameType frameStyle);

#ifdef __cplusplus
	}
#endif

#endif	// __CTLGLUE_H__
