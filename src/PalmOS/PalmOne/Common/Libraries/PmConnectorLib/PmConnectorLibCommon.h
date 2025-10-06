/*******************************************************************************
 * Copyright (c) 2004-2005 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 ******************************************************************************/
/**
 * @defgroup	PmConnectorLib Multi Connector Library
 * @brief		This library provides support for multi-connector related functionalities.
 *
 * Applications can register itself to be notified when a peripheral is attached
 * or detached to the multi-connector port. Applications can also use this library
 * to enable/disable the power pin and to re-route audio output.
 *
 * @{
 * @}
 */
/**
 * @ingroup PmConnectorLib
 */

/**
 * @file  PmConnectorLibCommon.h
 * @brief Public 68k common header file for Multi-Connector Library for palmOne
 *        devices that use multi-connector.
 *       
 * This file contains common structures, types and defines for palmOne library
 * that exports multi-connector related APIs.
 * <hr>
 */

#ifndef __PMConnectorLIBCOMMON_H__
#define __PMConnectorLIBCOMMON_H__

#include <NotifyMgr.h>

/********************************************************************
 * Constants
 ********************************************************************/
#define kPmConnectorLibType		  		sysFileTLibrary
#define kPmConnectorLibCreator	      	'PmAt'
#define kPmConnectorLibName		  		"PmConnector"
#define kPmConnectorNotifyType				'PmCo'
#define kPmConnectorNotifySysAttach         sysExternalConnectorAttachEvent
#define kPmConnectorNotifySysDetach         sysExternalConnectorDetachEvent

// Bits used in notifyDetailsP for kPmConnectorNotifySysAttach and kPmConnectorNotifySysDetach
#define kPmDockStatusCharging           0x08
#define kPmDockStatusUSBCradle          0x10
#define kPmDockStatusSerialPeripheral   0x40

/********************************************************************
 * Traps
 ********************************************************************/
/**
 * @name Function Traps
 */
/*@{*/
#define kPmConnectorLibTrapOpen			sysLibTrapOpen
#define kPmConnectorLibTrapClose		sysLibTrapClose
#define kPmConnectorLibTrapSleep		sysLibTrapSleep
#define kPmConnectorLibTrapWake			sysLibTrapWake
#define kPmConnectorLibTrapGetVersion		(sysLibTrapCustom + 0)
#define kPmConnectorLibTrapControl      	(sysLibTrapCustom + 1)
#define kPmConnectorLibTrapSetAudioOutput	(sysLibTrapCustom + 2)
#define kPmConnectorLibTrapSetAudioInput	(sysLibTrapCustom + 3)
/*@}*/

/***********************************************************************
 * Library version
 ***********************************************************************/
#define		kPmConnectorLibVersion	sysMakeROMVersion(1, 3, 0, sysROMStageRelease, 0)


/***********************************************************************
 * PmConnectorLib result codes
 * (appErrorClass is reserved for 3rd party apps/libraries.
 * It is defined in SystemMgr.h)
 ***********************************************************************/
/**
 * @name Error Codes
 */
/*@{*/
#define kPmConnectorLibErrorClass			(oemErrorClass + 0x100)
/* Invalid parameter */
#define kPmConnectorLibErrBadParam			(kPmConnectorLibErrorClass | 0x01)
/* Memory error */
#define kPmConnectorLibErrNoMemory			(kPmConnectorLibErrorClass | 0x02)
/* Library is not open */
#define kPmConnectorLibErrNotOpen			(kPmConnectorLibErrorClass | 0x03)
/* Returned from PmConnectorLibClose() if the library is still open */
#define kPmConnectorLibErrStillOpen			(kPmConnectorLibErrorClass | 0x04)
/* Internal erro */
#define kPmConnectorLibErrInternal			(kPmConnectorLibErrorClass | 0x05)
/* Unsupported function */
#define kPmConnectorLibErrNotSupported		(kPmConnectorLibErrorClass | 0x06)
/* Bad version */
#define kPmConnectorLibErrNotCompatible	(kPmConnectorLibErrorClass | 0x07)
/*@}*/

/***********************************************************************
 * Constants - control commands
 ***********************************************************************/

/**
 * @name Control commands for Connector
 */
/*@{*/
#define kPmConnectorLibCtrlPowerOn				0x01
#define kPmConnectorLibCtrlPowerOff				0x02
#define kPmConnectorLibCtrlGetDeviceClass		0x03
/*@}*/

/***********************************************************************
 * Notification constants 
 ***********************************************************************/

/**
 * @name Notification Constants
 */
/*@{*/
#define kPmConnectorClassAudioWHeadset			0x05 // was 0x00 in versions <= 1.2.0
#define kPmConnectorClassAudioNoHeadset			0x00 // was 0x01 in versions <= 1.2.0
#define kPmConnectorClassCarkit					0x01 // was 0x02 in versions <= 1.2.0
#define kPmConnectorClassGenericSerial			0x03
#define kPmConnectorClassSmartSerial			0x07
#define kPmConnectorClassSerial					0x10
#define kPmConnectorClassUSB					0xf0
#define kPmConnectorClassUnknown				0x00ff
#define kPmConnectorClassNoDevice				0x0fff
#define kPmConnectorClassMask					0x0fff
/*@}*/


/** @brief State of the connector */
typedef enum {
	kPmConnectorDeviceAttach = 0x8000,	/**< is currently attached */
	kPmConnectorDeviceDetach = 0x4000	/**< is currently detached */
} ePmConnectorDeviceState;

typedef UInt16 PmConnectorStateType;

/** @brief Container for the notification details */
typedef struct {
	UInt32					serialPortId;
	UInt32					deviceClass;
	UInt32					athenaDeviceCreatorCode;
	UInt32					reserved1;
	UInt32					reserved2;
} PmConnectorNotifyDataType;

/**
 * @brief Audio Output Routings
 */
typedef enum tagAudioOutputSetting 
{
    kAudioOutputSettingDefault = 0,  //device should do something reaasonable based on its current capabilities
    kAudioOutputSettingHeadphone,
    kAudioOutputSettingAthena,
    kAudioOutputSettingSpeaker
} ePmConnectorAudioOutputSettingsType;

/** Audio Output setting type */
typedef UInt16 PmConnectorAudioOutputSettingType;

typedef enum tagAudioInputSetting 
{
    kAudioInputDefault = 0,  //device should do something reaasonable based on its current capabilities
    kAudioInputExternalMic,
    kAudioInputInternalMic
} ePmConnectorAudioInputSettingsType;

typedef UInt16 PmConnectorAudioInputSettingType;


/** Library control type */
typedef UInt16 PmConnectorLibControlType;

#endif // __PMConnectorLIBCOMMON_H__
