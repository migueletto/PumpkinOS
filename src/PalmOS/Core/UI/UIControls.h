/******************************************************************************
 *
 * Copyright (c) 1998-2003 PalmSource, Inc. All rights reserved.
 *
 * File: UIControls.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *             	Contrast & brightness control for devices with
 *						software contrast.
 *
 *****************************************************************************/

#ifndef	__UICONTROLS_H__
#define	__UICONTROLS_H__

#include <CoreTraps.h>
#include <Window.h>

// for UIPickColor
#define UIPickColorStartPalette	0
#define UIPickColorStartRGB		1

typedef UInt16 UIPickColorStartType;



#ifdef __cplusplus
extern "C" {
#endif

extern void UIContrastAdjust()
		SYS_TRAP(sysTrapUIContrastAdjust);

extern void UIBrightnessAdjust()
		SYS_TRAP(sysTrapUIBrightnessAdjust);

Boolean UIPickColor(IndexedColorType *indexP, RGBColorType *rgbP,
					     UIPickColorStartType start, const Char *titleP,
					     const Char *tipP)
		SYS_TRAP(sysTrapUIPickColor);


#ifdef __cplusplus 
}
#endif

#endif	// __UICONTROLS_H__
