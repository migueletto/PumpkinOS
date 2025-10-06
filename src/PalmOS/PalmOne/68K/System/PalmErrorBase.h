/******************************************************************************
 * Copyright (c) 2005 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/** 
 *@ingroup System
 *
 */
 
/**
 * @file 	PalmErrorBase.h
 * @version 1.0
 * @date 	03/20/2003
 *
 * @brief Include file for Error Management 
 * 
 *
 * <hr>
 */



#ifndef __PALMERRORBASE_H__
#define __PALMERRORBASE_H__

#include "ErrorBase.h"

#define	oemErrorClass				0x7000	/**< OEM/Licensee errors (0x7000-0x7EFF shared among ALL partners) */


/**
 * @name Error Classes for OEM components
 *
 */
/*@{*/
#define	softGraffitErrorClass	(oemErrorClass | 200)		/**<		*/
#define	statusBarErrorClass		(oemErrorClass | 300)	/**<		*/
/*@}*/
#endif // __PALMERRORBASE_H__
