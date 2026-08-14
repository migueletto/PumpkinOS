/******************************************************************************
 *                                                                            *
 *            (C) Copyright 2000-2002, Sony Corporation                       *
 *                                                                            *
 *----------------------------------------------------------------------------*
 *                                                                            *
 *    <IDENTIFICATION>                                                        *
 *       file name    : $Workfile: SonyNotify.h $
 *                                                                            *
 *    <PROFILE>                                                               *
 *       Type/DataStructure of Notifications for Sony System                  *
 *                                                                            *
 *    <HISTORY>                                                               *
 *       Started on   : 00/12/04                                              *
 *       Last Modified: $Date: 03/06/17 17:21 $
 *                                                                            *
 ******************************************************************************/
/* this file is best viewed by setting TAB-stop as 3 */

#ifndef __SONYNOTIFY_H__
#define __SONYNOTIFY_H__

/******************************************************************************
 *    Includes                                                                *
 ******************************************************************************/
#include <SonyTypes.h>
#include <NotifyMgr.h>
#include <SonySystemResources.h>
#include <SonyPnP.h>

/******************************************************************************
 *    Constants                                                               *
 ******************************************************************************/
/*** param for sysExternalConnectorAttach/DetachEvent ***/
	/* cast as (void *) */
#define sonySysCntIntelligentUart	(0x0000)
#define sonySysCntTunerCradle			(0x0001)
#define sonySysCntSpeakerCradle		(0x0002)
#define sonySysCntUartCradle			(0x0003)
#define sonySysCntBatteryCradle		(0x0004)
#define sonySysCntModem					(0x0005)
#define sonySysCntSuperCradle			(0x0006)
#define sonySysCntReserve1				(0x0007)
#define sonySysCntUsbCradle			(0x0008)
#define sonySysCntReserve2				(0x0009)
#define sonySysCntUndocked				(0x000A)

/******************************************************************************
 *    Type/DataStructure of Notifications                                     *
 ******************************************************************************/
#define sonySysNotifyBroadcasterCode      sonySysFileCSony

/*** PnP ***/
#define sonySysNotifyPnPDeviceAttachedEvent	'SnPa'
#define sysNotifyPnPDeviceAttachedEvent		sonySysNotifyPnPDeviceAttachedEvent
				/* Notified when PnP device is attached.
				   Broadcast from Interrupt */
	/* (void *)notifyDetailsP is pointer to SysNotifyPnPDeviceType */
	/* those constants related to SysNotifyPnPDeviceType are defined in SonyPnP.h */

#define sonySysNotifyPnPDeviceDetachedEvent	'SnPd'
#define sysNotifyPnPDeviceDetachedEvent		sonySysNotifyPnPDeviceDetachedEvent
				/* Notified when PnP device is detached.
				   Broadcast from Interrupt */
	/* (void *)notifyDetailsP is pointer to SysNotifyPnPDeviceType */
	
#define sonySysNotifyPnPDevice2DriverEvent	'SnPh'
#define sysNotifyPnPDevice2DriverEvent		sonySysNotifyPnPDevice2DriverEvent
				/* Notified when PnPMgr wants to get device information.
				   Broadcast from Interrupt */
	/* (void *)notifyDetailsP is pointer to SysNotifyPnPDevice2DriverType */
	
#define sonySysNotifyPnPDriverEnumerateEvent	'SnPv'
#define sysNotifyPnPDriverEnumerateEvent		sonySysNotifyPnPDriverEnumerateEvent
				/* Notified when PnPMgr wants to enumerate drivers.
				   Broadcast from Interrupt */
	/* (void *)notifyDetailsP is pointer to SysNotifyPnPDriverInfoListType */

#define sonySysNotifyPnPDeviceEnumerateEvent	'SnPe'
#define sysNotifyPnPDeviceEnumerateEvent		sonySysNotifyPnPDeviceEnumerateEvent
				/* Notified when PnPMgr wants to enumerate devices.
				   Broadcast from Interrupt */
	/* (void *)notifyDetailsP is pointer to SysNotifyPnPDeviceInfoListType */
	
#define sonySysNotifyPnPPifEnumerateEvent	'SnPp'
#define sysNotifyPnPPifEnumerateEvent		sonySysNotifyPnPPifEnumerateEvent
				/* Notified when PnPMgr wants to enumerate Pifs.
				   Broadcast from Interrupt */
	/* (void *)notifyDetailsP is pointer to SysNotifyPnPDeviceInfoListType */
	
#define sonySysNotifyPnPUpdateInfoEvent	'SnPu'
#define sysNotifyPnPUpdateInfoEvent		sonySysNotifyPnPUpdateInfoEvent
				/* Notified when any PnP informations are updated.
				   Broadcast from Interrupt */
	/* (void *)notifyDetailsP is pointer to SysNotifyPnPDeviceInfoListType */

/*** SilkMgr ***/
#define sonySysNotifyPalmSilkChangeEvent		'SnPs'
#define sysNotifyPalmSilkChangeEvent			sonySysNotifyPalmSilkChangeEvent
				/* Notified when some state changes happen on Palm-compatible 
				     Silkscreen, like Soft-Graffiti turns into Soft-Keyboard or
				     vice-versa. */
				/* This is for some Graffiti utilities such as FEPs, which assume
				     Graffiti is always there */
	/* (void *)notifyDetailsP is UInt32 as follows */
#define sysNotifyPalmSilkAppeared			(0x00000001L)
#define sysNotifyPalmSilkDisappeared		(0x00000000L)

#define sonySysNotifySilkStateChangeEvent		'SnSs'
#define sysNotifySilkStateChangeEvent			sonySysNotifySilkStateChangeEvent
				/* Notified when some state changes happen on SilkMgr, like
				     enabled/disabled resizability */
				/* Normally this is for StatusBar to change its Resize-icon */
				/* Not notified when actual resize happes, in which case 
				     sysNotifyDisplayChangeEvent is broadcasted */
	/* (void *)notifyDetailsP is pointer to SysNotifySilkStateChangeType */
typedef struct {
	UInt16 type;	/* as defined in SonySilkLib.h */
	UInt16 state;	/* as defined in SonySilkLib.h */
	UInt32 rsv;
} SysNotifySilkStateChangeType;
typedef SysNotifySilkStateChangeType *SysNotifySilkStateChangePtr;
typedef SysNotifySilkStateChangeType SonySysNotifySilkStateChangeType;
typedef SysNotifySilkStateChangeType *SonySysNotifySilkStateChangePtr;


/*** HoldExtn ***/
	/*** CAUTION: This notification is depricated ***/
#define sonySysNotifyHoldStatusChangeEvent	'SnHd' /* Hold status change */
				/* This notification is broadcasted by Deferred */
				/* Caution: Hold-notificaiton is broadcasted only when device is
				     powered On, so every On/Off notificaiton may not be paird with
				     each other. */ 

typedef struct {
	Boolean holdOn;	/* true when On, false when Off */
	UInt32 lock;		/* bitfields, valid when holdOn is true */
	UInt32 rsv;
} SonySysNotifyHoldStatusChangeDetailsType;
typedef SonySysNotifyHoldStatusChangeDetailsType
					*SonySysNotifyHoldStatusChangeDetailsPtr;
typedef SonySysNotifyHoldStatusChangeDetailsPtr
					SonySysNotifyHoldStatusChangeDetailsP;
typedef SonySysNotifyHoldStatusChangeDetailsType SonySysNotifyHoldStatusChangeType;
typedef SonySysNotifyHoldStatusChangeDetailsPtr SonySysNotifyHoldStatusChangePtr;

#define sonySysNotifyHoldLockKey				(0x000001)
#define sonySysNotifyHoldLockPen				(0x000002)
#define sonySysNotifyHoldLockScreen			(0x000004)

/*** DemoLoaderExtn ***/
#define sonySysNotifyDemoExtractEvent	'SnDm' /* sample data extract event */

#ifndef  _BUILD_FOR_PALMOS_5_
/*** MsaLib ***/
	/*** CAUTION: This notification is not used on PalmOS 5 devices ***/
#define sonySysNotifyMsaStatusChangeEvent 'SnSc' /* Msa-Lib status change     */
				/* This notification is broadcasted by Deferred */
#define sonySysNotifyMsaEnforceOpenEvent 'SnEo'
#endif

#endif	// __SONYNOTIFY_H__

