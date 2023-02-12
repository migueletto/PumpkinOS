/******************************************************************************
 *
 * Copyright (c) 1999-2003 PalmSource, Inc. All rights reserved.
 *
 * File: PalmSpecifyOptLevel.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 *****************************************************************************/

// Include this file (perhaps as a prefix) if the default optimization level doesn't work
// or needs to be adjusted in some way.
//
// Example usages:
//
// #ifdef SPECIFY_OPT_LEVEL
// #pramga optimization_level 1
// #endif
//
// #ifdef SPECIFY_OPT_LEVEL
// #pragma global_optimization off
// #endif

#define SPECIFY_OPT_LEVEL
