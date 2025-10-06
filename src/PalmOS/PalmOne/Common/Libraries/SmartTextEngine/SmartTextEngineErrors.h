/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/** @ingroup STE
 *
 */

/**
 *
 * @file 	SmartTextEngineErrors.h
 * @brief   Error codes for Smart Text Engine shared library
 *
 */


#ifndef _SMARTTEXTENGINEERRORS_H_
#define _SMARTTEXTENGINEERRORS_H_

typedef UInt32 STEErr;	/**< Common define for Smart Text Engine Error Code return value */


// Maybe one day I will implement all of these different errors
#define steErrBase                  hsSTEErrorClass		/**< Error Code starting base */
#define steErrUnimplemented         (steErrBase + 1)    /**< Functionality not currently implemented*/
#define steErrInternal              (steErrBase + 2)    /**< Unspecified internal error */
#define steErrDynamicMemory         (steErrBase + 3)    /**< Dynamic memory allocation failed */
#define steErrStorageMemory         (steErrBase + 4)    /**< Storage memory allocation failed */
#define steErrOperationNotAllowed   (steErrBase + 5)    /**< Operation cannot be performed at this time */
#define steErrInvalidArguments      (steErrBase + 6)    /**< Bad arguments */
#define steErrSTEInit			    (steErrBase + 7)    /**< STE initialization failed */

#endif // _SMARTTEXTENGINEERRORS_H_
