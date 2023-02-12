/******************************************************************************
 *
 * Copyright (c) 2000-2003 PalmSource, Inc. All rights reserved.
 *
 * File: ResGlue.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *		Header file for ResGlue library routines.
 *
 *		Glue providing resource utility routines.
 *
 *****************************************************************************/

#ifndef __RESGLUE_H__
#define __RESGLUE_H__

#include <PalmTypes.h>

#ifdef __cplusplus
	extern "C" {
#endif

UInt32 ResGlueLoadConstant(UInt16 rscID);

#ifdef __cplusplus
	}
#endif

#endif	// __RESGLUE_H__
