/******************************************************************************
 *
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: SamplePrefix.h
 *
 *****************************************************************************/


#include <BuildDefines.h>


#ifdef ERROR_CHECK_LEVEL_OK_TO_REDEFINE
  #undef ERROR_CHECK_LEVEL_OK_TO_REDEFINE
  #undef ERROR_CHECK_LEVEL
#endif

#ifndef ERROR_CHECK_LEVEL
  #define ERROR_CHECK_LEVEL ERROR_CHECK_FULL
#endif
