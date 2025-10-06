/******************************************************************************
 * Copyright (c) 2005 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/** 
 *@ingroup System
 *
 */
 
/**
 * @file 	PalmLaunchCodes.h
 * @version 1.0
 * @date 	03/20/2003
 *
 * @brief Device specific launch codes.
 * 
 *
 * <hr>
 */


#ifndef __PALMLAUNCHCODES_H__
#define __PALMLAUNCHCODES_H__

#if defined(__arm__)
#include "CmnLaunchCodes.h"
#else
#include "SystemMgr.h"
#endif
 
/**
 * @name Launch commands
 *
 */
/*@{*/
#define devAppLaunchCmdCustomBase		(sysAppLaunchCmdCustomBase + 1)		/**< Custom action code base */

#define devAppLaunchCmdGenericCall		(devAppLaunchCmdCustomBase	+ 1)	/**< Used when an application wants to launch another application, then to be re-launched when the service is finished */
#define devAppLaunchCmdIncomingCall		(devAppLaunchCmdCustomBase	+ 2)	/**<		*/
#define devAppLaunchCmdCallerId			(devAppLaunchCmdCustomBase	+ 3)	/**<		*/
#define devAppLaunchCmdTelPowerOn		(devAppLaunchCmdCustomBase	+ 4)	/**<		*/
#define devAppLaunchMyFavorite			(devAppLaunchCmdCustomBase	+ 4)	/**<		*/
#define devAppLaunchCmdIncomingCallTest		(devAppLaunchCmdCustomBase	+ 5)	/**<		*/
#define devAppLaunchCmdSliderOpened		(devAppLaunchCmdCustomBase	+ 6)	/**<		*/
#define devAppLaunchCmdSliderClosed		(devAppLaunchCmdCustomBase	+ 7)	/**<		*/
#define devAppLaunchLauncherDUBegin		(devAppLaunchCmdCustomBase	+ 8)	/**<		*/
#define devAppLaunchLauncherDUEnd		(devAppLaunchCmdCustomBase	+ 9)	/**<		*/
#define devAppLaunchLPCM_DUBegin		(devAppLaunchCmdCustomBase	+ 10)	/**<		*/
#define devAppLaunchLPCM_DUEnd			(devAppLaunchCmdCustomBase	+ 11)	/**<		*/
#define devAppLaunchWithCodeCheck		(devAppLaunchCmdCustomBase	+ 12)	/**<		*/
#define sysAppLaunchCmdFromSetupApp		(devAppLaunchCmdCustomBase	+ 13)	/**< Used to lock a Panel (panel cannot stop except with Done) */
/*@}*/

/**
 * Parameter blocks for action codes
 **/

/**
 * @brief For devAppLaunchCmdGenericCall
 *
 **/
typedef struct {
	UInt32	senderCreatorId;		/**< IN: the creator ID of the calling code resource */
	void*		userDataP;		/**< IN: pointer to custom data (optional) */
	UInt8		operation;		/**< IN: one of the sysAppLaunchGenericCallOp<operation> codes */
	UInt8		response;		/**< IN: set to 0; OUT: one of the sysAppLaunchGenericCallRet<response> codes, or 0 if not handled */
} DevAppLaunchCmdGenericCallType;

/**
 * @name Applicable operation code values:
 *
 */
/*@{*/
#define devAppLaunchGenericCallOpSubcall	((UInt8)1)	/**< the sender wishes to launch the handler */
#define devAppLaunchGenericCallOpExit		((UInt8)2) 	/**< the launched code resource replies that it won't launch the caller back */
#define devAppLaunchGenericCallOpReturn		((UInt8)3) 	/**< the sub-call is finished, the launched code resource is ready to launch the caller back */
#define devAppLaunchGenericCallOpCustomBase	((UInt8)0x80)	/**< custom operation code base (custom codes begin at this value) */
/*@}*/

/**
 * @name Applicable response code values:
 *
 */
/*@{*/
#define devAppLaunchGenericCallRetAccepted	((UInt8)1)	/**< request accepted by the handler */
#define devAppLaunchGenericCallRetRejected	((UInt8)2)	/**< request rejected by the handler */
/*@}*/


#endif // __PALMLAUNCHCODES_H__
