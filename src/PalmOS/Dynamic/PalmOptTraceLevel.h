/******************************************************************************
 *
 * Copyright (c) 1999-2003 PalmSource, Inc. All rights reserved.
 *
 * File: PalmOptTraceLevel.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 *****************************************************************************/

#ifndef __PALMOPTTRACELEVEL_H__
#define __PALMOPTTRACELEVEL_H__

#include <BuildDefines.h>

	#ifdef TRACE_OUTPUT_OK_TO_REDEFINE
		#undef TRACE_OUTPUT_OK_TO_REDEFINE
		#undef TRACE_OUTPUT
	#endif
	
	#ifndef TRACE_OUTPUT
		#define TRACE_OUTPUT TRACE_OUTPUT_OFF
	#endif
	
#endif
