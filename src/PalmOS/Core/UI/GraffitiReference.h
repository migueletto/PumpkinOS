/******************************************************************************
 *
 * Copyright (c) 1996-2003 PalmSource, Inc. All rights reserved.
 *
 * File: GraffitiReference.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *	  This file defines the Graffiti Reference routines.
 *
 *****************************************************************************/

#ifndef __GRAFFITIREFERENCE_H__
#define __GRAFFITIREFERENCE_H__

#include <PalmTypes.h>
#include <CoreTraps.h>

typedef enum 
	{
	referenceDefault = 0xff		// based on graffiti mode
	} ReferenceType;

/************************************************************
 * Graffiti Reference procedures
 *************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

#ifdef BUILDING_GRAFFITI_EXT
#define GRF_TRAP(x)
#else
#define GRF_TRAP  SYS_TRAP
#endif


extern void SysGraffitiReferenceDialog (ReferenceType referenceType)
							GRF_TRAP(sysTrapSysGraffitiReferenceDialog);


#ifdef __cplusplus
}
#endif

#endif // __GRAFFITIREFERENCE_H__
