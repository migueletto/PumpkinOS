/******************************************************************************
 *
 * Copyright (c) 2000-2003 PalmSource, Inc. All rights reserved.
 *
 * File: ExpansionMgr.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *		Header file for Expansion Manager.
 *
 *****************************************************************************/

#ifndef __EXPANSIONMGR_H__
#define __EXPANSIONMGR_H__

#include <PalmTypes.h>
#include <CoreTraps.h>
#include <SystemMgr.h>

// When building the PalmOS 3.5 version of ExpansionMgr, 
// since these constants were not in the 3.5 headers...
#ifdef BUILDING_AGAINST_PALMOS35
	#define sysTrapExpansionMgr	sysTrapSysReserved2

	#define expErrorClass			0x2900			// Post 3.5 this is defined in ErrorBase.h
	
	// Post 3.5 these are defined in NotifyMgr.h.
	#define sysNotifyCardInsertedEvent	'crdi'	// Broadcast when an ExpansionMgr card is 
																// inserted into a slot, and the slot driver 
																// calls ExpCardInserted.  Always broadcast
																// from UI task.
																// ExpansionMgr will play a sound & attempt to
																// mount a volume unless 'handled' is set
																// to true by a notification handler.
																// PARAMETER: slot number cast as void*
																
	#define sysNotifyCardRemovedEvent	'crdo'	// Broadcast when an ExpansionMgr card is 
																// removed from a slot, and the slot driver 
																// calls ExpCardRemoved.  Always broadcast
																// from UI task.
																// ExpansionMgr will play a sound & attempt to
																// unmount a volume unless 'handled' is set
																// to true by a notification handler.
																// PARAMETER: slot number cast as void*

	#define sysNotifyVolumeMountedEvent	'volm'	// Broadcast when a VFSMgr volume is 
																// mounted, Always broadcast
																// from UI task.
																// PARAMETER: VFSAnyMountParamPtr cast as void*

	#define sysNotifyVolumeUnmountedEvent 'volu'	// Broadcast when a VFSMgr volume is 
																// unmounted, Always broadcast
																// from UI task.
																// PARAMETER: volume refNum cast as void*
	
	
	#define sysFileCExpansionMgr			'expn'		// Type of Expansion Manager extension database

	#define		sysFileTSlotDriver		'libs'			// file type for slot driver libraries

#else
	#define sysTrapExpansionMgr	sysTrapExpansionDispatch
#endif


#ifndef USE_EXPMGR_TRAPS
	#if EMULATION_LEVEL == EMULATION_NONE
		#define	USE_EXPMGR_TRAPS	1
	#else
		#define	USE_EXPMGR_TRAPS	0
	#endif
#endif


#ifdef BUILDING_EXPMGR_DISPATCH
	#define EXPMGR_TRAP(expMgrSelectorNum)
#else
	#define EXPMGR_TRAP(sel) \
		_SYSTEM_API(_CALL_WITH_SELECTOR)(_SYSTEM_TABLE, sysTrapExpansionMgr, sel)
#endif





#define expFtrIDVersion				0	// ID of feature containing version of ExpansionMgr.
												// Check existence of this feature to see if ExpMgr is installed.

#define expMgrVersionNum			((UInt16)200)	// version of the ExpansionMgr, obtained from the feature

#define expInvalidSlotRefNum		0

typedef Err (*ExpPollingProcPtr)(UInt16 slotLibRefNum, 
			void *slotPollRefConP);


/************************************************************
 * Capabilities of the hardware device for ExpCardInfoType.capabilityFlags
 *************************************************************/
#define	expCapabilityHasStorage		0x00000001	// card supports reading (& maybe writing) sectors
#define	expCapabilityReadOnly		0x00000002	// card is read only
#define expCapabilitySerial         0x00000004  // card supports dumb serial interface

#define expCardInfoStringMaxLen		31

typedef struct ExpCardInfoTag
{
	UInt32	capabilityFlags;	// bits for different stuff the card supports
	Char		manufacturerStr[expCardInfoStringMaxLen+1];	// Manufacturer, e.g., "Palm", "Motorola", etc...
	Char		productStr[expCardInfoStringMaxLen+1];			// Name of product, e.g., "SafeBackup 32MB"
	Char		deviceClassStr[expCardInfoStringMaxLen+1];	// Type of product, e.g., "Backup", "Ethernet", etc.
	Char		deviceUniqueIDStr[expCardInfoStringMaxLen+1];// Unique identifier for product, e.g., a serial number.  Set to "" if no such identifier exists.
}	ExpCardInfoType, *ExpCardInfoPtr;


/************************************************************
 * Iterator start and stop constants.
 * Used by ExpSlotEnumerate
 *************************************************************/
#define expIteratorStart              0L
#define expIteratorStop               0xffffffffL


/************************************************************
 * Bits in the 'handled' field used in Card Inserted and Removed notifications
 *************************************************************/
#define expHandledVolume		0x01	// any volumes associated with the card have been dealt with... the ExpansionMgr will not mount or unmount as appropriate.
#define expHandledSound			0x02	// Any pleasing sounds have already been played... the ExpansionMgr will not play a pleasing sound on this insertion/removal.


/************************************************************
 * Error codes
 *************************************************************/
#define expErrUnsupportedOperation			(expErrorClass | 1)		// unsupported or undefined opcode and/or creator
#define expErrNotEnoughPower					(expErrorClass | 2)		// the required power is not available

#define expErrCardNotPresent					(expErrorClass | 3)		// no card is present
#define expErrInvalidSlotRefNum				(expErrorClass | 4)		// slot reference number is bad
#define expErrSlotDeallocated					(expErrorClass | 5)		// slot reference number is within valid range, but has been deallocated.
#define expErrCardNoSectorReadWrite			(expErrorClass | 6)		// the card does not support the 
																						// SlotDriver block read/write API
#define expErrCardReadOnly						(expErrorClass | 7)		// the card does support R/W API
																						// but the card is read only
#define expErrCardBadSector					(expErrorClass | 8)		// the card does support R/W API
																						// but the sector is bad
#define expErrCardProtectedSector			(expErrorClass | 9)		// The card does support R/W API
																						// but the sector is protected
#define expErrNotOpen							(expErrorClass | 10)		// slot driver library has not been opened
#define expErrStillOpen							(expErrorClass | 11)		// slot driver library is still open - maybe it was opened > once
#define expErrUnimplemented					(expErrorClass | 12)		// Call is unimplemented
#define expErrEnumerationEmpty				(expErrorClass | 13)		// No values remaining to enumerate
#define expErrIncompatibleAPIVer				(expErrorClass | 14)		// The API version of this slot driver is not supported by this version of ExpansionMgr.


/************************************************************
 * Common media types.  Used by SlotCardMediaType and SlotMediaType.
 *************************************************************/
#define expMediaType_Any				'wild'	// matches all media types when looking up a default directory
#define expMediaType_MemoryStick		'mstk'
#define expMediaType_CompactFlash	'cfsh'
#define expMediaType_SecureDigital	'sdig'
#define expMediaType_MultiMediaCard	'mmcd'
#define expMediaType_SmartMedia		'smed'
#define expMediaType_RAMDisk			'ramd'	// a RAM disk based media
#define expMediaType_PoserHost		'pose'	// Host filesystem emulated by Poser
#define expMediaType_MacSim			'PSim'	// Host filesystem emulated by Poser


/************************************************************
 * Selectors for routines found in the Expansion manager. The order
 * of these selectors MUST match the jump table in ExpansionMgr.c.
 *************************************************************/
#define expInit					0
#define expSlotDriverInstall	1
#define expSlotDriverRemove	2
#define expSlotLibFind			3
#define expSlotRegister			4
#define expSlotUnregister		5
#define expCardInserted			6
#define expCardRemoved			7
#define expCardPresent			8
#define expCardInfo				9
#define expSlotEnumerate		10
#define expCardGetSerialPort	11

#define expMaxSelector			expCardGetSerialPort


Err ExpInit(void)
		EXPMGR_TRAP(expInit);

Err ExpSlotDriverInstall(UInt32 dbCreator, UInt16 *slotLibRefNumP)
		EXPMGR_TRAP(expSlotDriverInstall);

Err ExpSlotDriverRemove(UInt16 slotLibRefNum)
		EXPMGR_TRAP(expSlotDriverRemove);

Err ExpSlotLibFind(UInt16 slotRefNum, UInt16 *slotLibRefNum)
		EXPMGR_TRAP(expSlotLibFind);

Err ExpSlotRegister(UInt16 slotLibRefNum, UInt16 *slotRefNum)
		EXPMGR_TRAP(expSlotRegister);

Err ExpSlotUnregister(UInt16 slotRefNum)
		EXPMGR_TRAP(expSlotUnregister);

Err ExpCardInserted(UInt16 slotRefNum)
		EXPMGR_TRAP(expCardInserted);

Err ExpCardRemoved(UInt16 slotRefNum)
		EXPMGR_TRAP(expCardRemoved);

Err ExpCardPresent(UInt16 slotRefNum)
		EXPMGR_TRAP(expCardPresent);

Err ExpCardInfo(UInt16 slotRefNum, ExpCardInfoType* infoP)
		EXPMGR_TRAP(expCardInfo);

Err ExpSlotEnumerate(UInt16 *slotRefNumP, UInt32 *slotIteratorP)
		EXPMGR_TRAP(expSlotEnumerate);
		
Err ExpCardGetSerialPort(UInt16 slotRefNum, UInt32* portP)
		EXPMGR_TRAP(expCardGetSerialPort);



#endif	// __EXPANSIONMGR_H__
