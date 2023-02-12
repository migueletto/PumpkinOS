/******************************************************************************
 *
 * Copyright (c) 1999-2003 PalmSource, Inc. All rights reserved.
 *
 * File: TsmGlue.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *		Header file for TsmGlueXXX library routines.
 *
 *****************************************************************************/

#ifndef __TSMGLUE_H__
#define __TSMGLUE_H__

#include <TextServicesMgr.h>

// Library versions of all the Text Services Mgr routines:

#ifdef __cplusplus
	extern "C" {
#endif

TsmFepModeType
TsmGlueGetFepMode(void* nullParam);

TsmFepModeType
TsmGlueSetFepMode(void* nullParam, TsmFepModeType inNewMode);

#ifdef __cplusplus
	}
#endif

#endif	// __TSMGLUE_H__
