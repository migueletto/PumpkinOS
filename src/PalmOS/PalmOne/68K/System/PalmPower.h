/******************************************************************************
 * Copyright (c) 2005 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/** 
 *@ingroup System
 *
 */
 
/**
 * @file 	PalmPower.h
 * @version 1.0
 * @date 	08/05/2003
 *
 * @brief  Battery notification.
 *
 * 	   This contains the Battery state notification
 *   	   Any application can register for the notification to be notified
 * 	   when the of the various battery states
 * 
 *
 * <hr>
 */ 


#ifndef __PalmPower_H__
#define __PalmPower_H__



/** 
 * @name Battery state notification
 *
 */
/*@{*/
#define kPwrBatteryStateBroadcaster		'bsbc'		/**< Battery notification broadcaster. */
#define kPwrBatteryStateNotifyEvent		'bsnt'		/**< Battery notification notifyType */
/*@}*/

#endif // __PalmPower_H__
