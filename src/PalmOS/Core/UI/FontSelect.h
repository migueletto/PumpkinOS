/******************************************************************************
 *
 * Copyright (c) 1997-2003 PalmSource, Inc. All rights reserved.
 *
 * File: FontSelect.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *	  This file defines the font selector routine.
 *
 *****************************************************************************/

#ifndef	__FONTSELECT_H__
#define	__FONTSELECT_H__

#include <PalmTypes.h>
#include <CoreTraps.h>

#include <Font.h>

#ifdef __cplusplus
extern "C" {
#endif

extern FontID FontSelect (FontID fontID)
						SYS_TRAP(sysTrapFontSelect);


#ifdef __cplusplus 
}
#endif

#endif // __FONTSELECT_H__
