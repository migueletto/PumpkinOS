/******************************************************************************
 *
 * Copyright (c) 2001-2003 PalmSource, Inc. All rights reserved.
 *
 * File: FldGlue.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *		Glue providing compatibility for applications that wish
 *		to make calls to the Field Manager, but which might be running
 *		on a system which does not support newer calls.
 *
 *****************************************************************************/

#ifndef __FLDGLUE_H__
#define __FLDGLUE_H__

#include <Field.h>

#ifdef __cplusplus
	extern "C" {
#endif

Boolean FldGlueGetLineInfo(const FieldType *fldP, UInt16 lineNum, UInt16* startP, UInt16* lengthP);

#ifdef __cplusplus
	}
#endif

#endif	// __FLDGLUE_H__
