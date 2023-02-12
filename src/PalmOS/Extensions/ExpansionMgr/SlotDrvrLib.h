/******************************************************************************
 *
 * Copyright (c) 1998-2003 PalmSource, Inc. All rights reserved.
 *
 * File: SlotDrvrLib.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *             	Sample Slot Driver library implementation.
 *
 *****************************************************************************/

#ifndef __SlotDrvr_LIB_H__
#define __SlotDrvr_LIB_H__

#include "ExpansionMgr.h"

// BUILDING_APPLICATION
#if BUILDING_APP_OR_LIB
	// direct link to library code
	#define SlotDrvr_LIB_TRAP(trapNum)
#else
	// else someone else is including this public header file; use traps
	#define SlotDrvr_LIB_TRAP(trapNum)	SYS_TRAP(trapNum)
#endif

#define slotDrvrAPIVersion		0x00000002

// The number of bytes per sector is fixed
#define slotSectorSize			512


/********************************************************************
 * Card Metrics
 * These structures contains all of the information about the physical
 * structure of the card that may be needed by a filesystem in order
 * to format volumes on the card.
 ********************************************************************/
#define slotDrvrPartitionTypeFAT12				(0x01)
#define slotDrvrPartitionTypeFAT16Under32MB		(0x04)
#define slotDrvrPartitionTypeFAT16Over32MB		(0x06)
#define slotDrvrBootablePartition				(0x80)
#define slotDrvrNonBootablePartition			(0x00)

typedef struct CardMetricsTag {
	UInt32	totalSectors;			// The total number of sectors accessable via SlotCardSector[Read/Write]
									//  	(some media may contain extra sectors in case one goes bad,
									//   	 or for storing configuration information, but they are handled
									//  	 internally to the slot driver, and not accessable) 
	UInt16	bytesPerSector;			// The number of bytes in one sector.
									//  	currently for Palm, this must be the standard 512
	UInt16	sectorsPerHead;			// The number of Sectors per Head
									//  	as given by guidelines in the specification for this media type
									//  	even though all of our disks accesses are LBA, 
									//  	this is for compatibility when filling out MBRs and PBRs
									// 		if the media guidelines don't care, this value is set to 0
	UInt16	headsPerCylinder;		// The number of Heads per Cylinder
									//  	as given by guidelines in the specification for this media type
									//  	even though all of our disks accesses are LBA, 
									//  	this is for compatibility when filling out MBRs and PBRs
									// 		if the media guidelines don't care, this value is set to 0
	UInt16	reserved1;				// Reserved
	UInt8	sectorsPerBlock;		// A suggested number of Sectors per Block (Cluster)
									//  	as given by guidelines in the specification for this media type
									//		if the media guidelines don't care, this value will be set to 0
	UInt8	partitionType;			// The suggested partition type (System ID) of the first partition
									//  	as given by guidelines in the specification for this media type
									//		if the media guidelines don't care, this value will be set to 0
	UInt8	bootIndicator;			// The suggested bootability of the first partition
									//  	as given by guidelines in the specification for this media type
									//  	(generally, 0x80=bootable, default boot partition 0x00=not-bootable)
									//		if the media guidelines don't care, this value will be set to 0xFF
	UInt8	reserved2;				// Reserved
	UInt32	partitionStart;			// The suggested starting sector of the first partition
									//  	as given by guidelines in the specification for this media type
									//		if this value is set to zero, and the partitionSize value is non-zero
									//		 the media guidelines suggest to not use an MBR, and only use a PBR at sector 0
									//  	if the media guidelines don't care, the partitionSize value will be set to 0
	UInt32	partitionSize;			// The suggested size of the first partition
									//  	as given by guidelines in the specification for this media type
									// 		if the media guidelines don't care, this value will be set to 0, and 
									//  	 the partitionStart parameter is also ignored
} CardMetricsType, *CardMetricsPtr;


/********************************************************************
 * SlotDrvr library function trap ID's. Each library call gets a trap number:
 *   SlotDrvrLibTrapXXXX which serves as an index into the library's dispatch table.
 *   The constant sysLibTrapCustom is the first available trap number after
 *   the system predefined library traps Open,Close,Sleep & Wake.
 *
 * WARNING!!! The order of these traps MUST match the order of the dispatch
 *  table in SlotDrvrLibDispatch.c!!!
 ********************************************************************/

#define SlotTrapLibAPIVersion		(sysLibTrapCustom)
#define SlotTrapCustomControl		(sysLibTrapCustom+1)
#define SlotTrapCardPresent			(sysLibTrapCustom+2)
#define SlotTrapCardInfo			(sysLibTrapCustom+3)
#define SlotTrapCardMediaType		(sysLibTrapCustom+4)
#define SlotTrapCardIsFilesystemSupported	(sysLibTrapCustom+5)
#define SlotTrapCardMetrics			(sysLibTrapCustom+6)
#define SlotTrapCardLowLevelFormat	(sysLibTrapCustom+7)
#define SlotTrapCardSectorRead		(sysLibTrapCustom+8)
#define SlotTrapCardSectorWrite		(sysLibTrapCustom+9)
#define SlotTrapPowerCheck			(sysLibTrapCustom+10)
#define SlotTrapMediaType			(sysLibTrapCustom+11)
#define SlotTrapCardReserve			(sysLibTrapCustom+12)
#define SlotTrapCardRelease			(sysLibTrapCustom+13)
#define SlotTrapCardGetSerialPort	(sysLibTrapCustom+14)


/********************************************************************
 * API Prototypes
 ********************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/********************************************************************
 * Standard library open, close, sleep and wake functions
 ********************************************************************/

extern Err SlotOpen(UInt16 slotLibRefNum)
				SlotDrvr_LIB_TRAP(sysLibTrapOpen);
	
extern Err SlotClose(UInt16 slotLibRefNum)
				SlotDrvr_LIB_TRAP(sysLibTrapClose);
	
extern Err SlotSleep(UInt16 slotLibRefNum)
				SlotDrvr_LIB_TRAP(sysLibTrapSleep);
	
extern Err SlotWake(UInt16 slotLibRefNum)
				SlotDrvr_LIB_TRAP(sysLibTrapWake);
	
/********************************************************************
 * Custom library API functions
 ********************************************************************/

extern UInt32 SlotLibAPIVersion(UInt16 slotLibRefNum)
				SlotDrvr_LIB_TRAP(SlotTrapLibAPIVersion);

extern Err SlotCustomControl(UInt16 slotLibRefNum, UInt32 apiCreator, UInt16 apiSelector, 
									void *valueP, UInt16 *valueLenP)
				SlotDrvr_LIB_TRAP(SlotTrapCustomControl);

extern Err SlotCardPresent(UInt16 slotLibRefNum, UInt16 slotRefNum)
				SlotDrvr_LIB_TRAP(SlotTrapCardPresent);

extern Err SlotCardInfo(UInt16 slotLibRefNum, UInt16 slotRefNum, ExpCardInfoType *infoP)
				SlotDrvr_LIB_TRAP(SlotTrapCardInfo);

extern Err SlotCardMediaType(UInt16 slotLibRefNum, UInt16 slotRefNum, UInt32 *mediaTypeP)
				SlotDrvr_LIB_TRAP(SlotTrapCardMediaType);

extern Err SlotMediaType(UInt16 slotLibRefNum, UInt16 slotRefNum, UInt32 *mediaTypeP)
				SlotDrvr_LIB_TRAP(SlotTrapMediaType);

extern Err SlotCardReserve(UInt16 slotLibRefNum, UInt16 slotRefNum)
				SlotDrvr_LIB_TRAP(SlotTrapCardReserve);

extern Err SlotCardRelease(UInt16 slotLibRefNum, UInt16 slotRefNum)
				SlotDrvr_LIB_TRAP(SlotTrapCardRelease);

extern Err SlotCardGetSerialPort(UInt16 slotLibRefNum, UInt16 slotRefNum, UInt32* portP)
				SlotDrvr_LIB_TRAP(SlotTrapCardGetSerialPort);



/********************************************************************
 * SlotDriver Formatting APIs:
 ********************************************************************/

extern Boolean SlotCardIsFilesystemSupported(UInt16 slotLibRefNum, UInt16 slotRefNum, UInt32 filesystemType)
				SlotDrvr_LIB_TRAP(SlotTrapCardIsFilesystemSupported);

extern Err SlotCardMetrics(UInt16 slotLibRefNum, UInt16 slotRefNum, CardMetricsPtr cardMetricsP)
				SlotDrvr_LIB_TRAP(SlotTrapCardMetrics);

extern Err SlotCardLowLevelFormat(UInt16 slotLibRefNum, UInt16 slotRefNum)
					SlotDrvr_LIB_TRAP(SlotTrapCardLowLevelFormat);


/********************************************************************
 * SlotDriver Logical Block Read/Write APIs:
 ********************************************************************/

extern Err SlotCardSectorRead(UInt16 slotLibRefNum, UInt16 slotRefNum, UInt32 sectorNumber,
	UInt8 *bufferP, UInt32 *numSectorsP)
				SlotDrvr_LIB_TRAP(SlotTrapCardSectorRead);

extern Err SlotCardSectorWrite(UInt16 slotLibRefNum, UInt16 slotRefNum, UInt32 sectorNumber, 
	UInt8 *bufferP, UInt32 *numSectorsP)
				SlotDrvr_LIB_TRAP(SlotTrapCardSectorWrite);


/********************************************************************
 * Power Mgmt APIs:
 ********************************************************************/

#define slotLibPowerFlag_WakeUp			0x0001	// Add the power required to bring the slot hardware out of low-power mode
#define slotLibPowerFlag_FormatMedia	0x0002	// Add the power required to perform a low-level format of the card media

extern Err SlotPowerCheck(UInt16 slotLibRefNum, UInt16 slotRefNum, UInt16 operationFlags,
					UInt16 readBlocks, UInt16 writeBlocks)
				SlotDrvr_LIB_TRAP(SlotTrapPowerCheck);

#ifdef __cplusplus 
}
#endif


#endif	// __SlotDrvr_LIB_H__
