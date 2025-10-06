/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/**
 * @ingroup SystemDef
 */

/**
 * @file  PmLibErrors.h
 * @brief Errors common to all our portable Pm Libraries
 *
 */

/* (these libraries share the same error class since we are so limited in the
 * number of OEM error classes we can define)
 *
 * \author Debbie Chyi
 *
 * $Id: //device/Handheld/Dev/SDK/2.0/Incs/Common/Libraries/PmLibErrors.h
 *
 *****************************************************************************/

#ifndef __PM_LIB_ERRORS_H__
#define __PM_LIB_ERRORS_H__

#include <Common/System/HsErrorClasses.h>


#define pmErrNotSupported					  (pmLibErrorClass | 0x01)


#endif // __PM_LIB_ERRORS_H__
