/******************************************************************************
 * Copyright (c) 2004 PalmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
 
/**
 * @file 	HsErrorClasses.h
 * @version 1.0
 * @date 	06/17/1999
 * @author  Vitaly Kruglikov
 *
 *	Public header file for the error classes used by Handspring
 *	viewer modules.
 *
 * <hr>
 */

#ifndef	  __HS_ERROR_CLASSES_H__
#define	  __HS_ERROR_CLASSES_H__


#include <SystemMgr.h>	  // for appErrorClass


/************************************************************
 * Error Classes for each manager
 *************************************************************/

// IMPORTANT!!!: DON'T DEFINE ERROR CODES BASED ON appErrorClass BELOW
// SINCE THEY COLLIDE WITH APPLICATION-DEFINED ERROR CODE NUMBER SPACE.
// IN PARTICULAR, SINCE OUR SYSTEM CODE RUNS IN THE SAME CONTEXT
// AS APPLICATION CODE, THESE COLLISIONS CAN RESULT IN THE WRONG ERROR
// STRING BEING REPORTED TO THE USER WHEN OUR ERROR STRING TABLE ID
// COLLIDES WITH THE APP'S.  Base your system error code on OEM Error
// Classes instead (see below).

#define hsAppErrorClass	    appErrorClass				// For top-level applications
#define	hsFfsErrorClass	    (appErrorClass | 0x0100)	// Flash file system manager
#define	hsFlmErrorClass	    (appErrorClass | 0x0200)	// Flash manager
#define	hsFlashErrorClass   (appErrorClass | 0x0300)    // Flash driver
#define	hsFttErrorClass	    (appErrorClass | 0x0400)    // File Transfer Transport driver
													    //  (used by the File Installer app)
#define	hsFtcErrorClass	    (appErrorClass | 0x0500)    // File Transfer Command driver
													    //  (used by the File Installer app)

#define hsTasLibErrorClass  (appErrorClass | 0x0600)    // Task manager.

#define hsHwmErrorClass		(appErrorClass | 0x0800)	// Phone HAL

#define hsSrmExtErrorClass	(appErrorClass | 0x0A00)	// Srm Extensions
#define hsSTEErrorClass		(appErrorClass | 0x0C00)	// Smart Text Engine

// IMPORTANT: SEE IMPORTANT COMMENT ABOVE!!!


/************************************************************
 * OEM Error Classes 
 *************************************************************/

// IMPORTANT: THERE ARE *VERY FEW* OEM ERROR CLASSES RESERVED BY THE SYSTEM,
// SO USE THEM VERY CAREFULLY. IF YOUR ERROR CODES WILL NOT BE SEEN
// BY THE SYSTEM OUTSIDE YOUR CONTROL, CONSIDER USEING SOME OTHER ERROR
// BASE.  THE RESERVED RANGE IS 0x7000 - 0x7F00 -- ONLY 16 ERROR CLASSES
// ARE RESERVED. CONSIDER SHARING AN ERROR CLASS AMONG RELATED MODULES.

#define hsOEMErrorClass				oemErrorClass

#define hsOEMRadioErrorClass		(hsOEMErrorClass)	// Radio Errors

// NetMaster library error class
#define hsNetMasterErrorClass		(hsOEMErrorClass + 0x0100)	// 0x7100

// NetPref library error class
#define hsNetPrefLibErrorClass		(hsOEMErrorClass + 0x0200)	// 0x7200

// Handspring Phone Library error class #2.  We ran out of error codes
// in error class #1 (hsPhoneLibErrorClass_1), so we are reserving a
// second error class for the Phone Library.
#define hsPhoneLibErrorClass_2		(hsOEMErrorClass + 0x0300)	// 0x7300

// Used by the ComChannel Provder. Which is the compoennt that controls the CDMA
// and GSM radios
#define hsCcpErrorClass				(hsOEMErrorClass + 0x0400)	// 0x7400

// Handspring Sound Extensions Library class
#define hsSoundLibErrorClass		(hsOEMErrorClass + 0x0500)	// 0x7500


// Used by Handspring Extensions error codes defined in HsExtCommon.h.
#define hsExtErrorClass				(hsOEMErrorClass + 0x0600)	// 0x7600

// Message Store
#define hsMSLErrorClass				(hsOEMErrorClass + 0x0700)	// 0x7700

// Errors in portable Pm libraries (shared between PmKeyLib, PmSysGadgetLib,
//	PmUIUtilLib, and PmSystemLib)
#define pmLibErrorClass				(hsOEMErrorClass + 0x0800)	// 0x7800


/************************************************************
 * Error Class Exceptions 
 *************************************************************/

// Handspring Phone Library error class #1.  See allso hsPhoneLibErrorClass_2
// for Phone Library error class #2.
// A long time ago, Handspring accidentally used 0x4000 as the error base
// for its Phone Library.  PalmSource reserved this error class for
// Handspring per message from Bob Petersen dated
// Friday, June 06, 2003 11:51 AM (RE: Question following up on LICINFO00574).
//
#define hsPhoneLibErrorClass_1		(0x4000)


// IMPORTANT: THE ERROR CLASSES ABOVE MUST BE LESS THAN 0x7F00!!!


#endif	  // __HS_ERROR_CLASSES_H__ -- include once




