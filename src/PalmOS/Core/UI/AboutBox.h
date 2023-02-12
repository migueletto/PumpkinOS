/******************************************************************************
 *
 * Copyright (c) 1995-2003 PalmSource, Inc. All rights reserved.
 *
 * File: AboutBox.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *	  This file defines About Box routines
 *
 *****************************************************************************/

#ifndef __ABOUTBOX_H__
#define __ABOUTBOX_H__

#include <PalmTypes.h>
#include <CoreTraps.h>

#ifdef __cplusplus
extern "C" {
#endif

// WARNING!!! This routine is for the private use of Palm applications.
// It is released with the public headers so that the sample apps
// released with the SDK can be compiled by developers.
void AbtShowAbout (UInt32 creator)
		SYS_TRAP(sysTrapAbtShowAbout);
		
#ifdef __cplusplus 
}
#endif

#endif // __ABOUTBOX_H__
