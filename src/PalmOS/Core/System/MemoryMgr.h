/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: MemoryMgr.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *		Include file for Memory Manager
 *
 *****************************************************************************/

#ifndef __MEMORYMGR_H__
#define __MEMORYMGR_H__


// Include elementary types
#include <PalmTypes.h>					// Basic types
#include <CoreTraps.h>					// Trap Numbers.
#include <ErrorBase.h>


/************************************************************
 * Memory Manager Types
 *************************************************************/
typedef enum {	memIDPtr, memIDHandle } LocalIDKind;


/************************************************************
 * Flags accepted as parameter for MemNewChunk.
 *************************************************************/
#define memNewChunkFlagPreLock				0x0100
#define memNewChunkFlagNonMovable			0x0200
#define memNewChunkFlagAtStart				0x0400	// force allocation at front of heap
#define memNewChunkFlagAtEnd					0x0800	// force allocation at end of heap


/************************************************************
 * Memory Manager Debug settings for the MemSetDebugMode function
 *************************************************************/
#define		memDebugModeCheckOnChange			0x0001
#define		memDebugModeCheckOnAll				0x0002
#define		memDebugModeScrambleOnChange		0x0004
#define		memDebugModeScrambleOnAll			0x0008
#define		memDebugModeFillFree					0x0010
#define		memDebugModeAllHeaps					0x0020
#define		memDebugModeRecordMinDynHeapFree	0x0040




/************************************************************
 * Memory Manager result codes
 *************************************************************/
#define	memErrChunkLocked			(memErrorClass | 1)
#define	memErrNotEnoughSpace		(memErrorClass | 2)
#define	memErrInvalidParam		(memErrorClass | 3)		/* invalid param or requested size is too big */
#define	memErrChunkNotLocked		(memErrorClass | 4)
#define	memErrCardNotPresent		(memErrorClass | 5)
#define	memErrNoCardHeader		(memErrorClass | 6)
#define	memErrInvalidStoreHeader (memErrorClass | 7)
#define	memErrRAMOnlyCard			(memErrorClass | 8)
#define	memErrWriteProtect		(memErrorClass | 9)
#define	memErrNoRAMOnCard			(memErrorClass | 10)
#define	memErrNoStore				(memErrorClass | 11)
#define	memErrROMOnlyCard			(memErrorClass | 12)


/********************************************************************
 * Memory Manager Routines
 * These are define as external calls only under emulation mode or
 *  under native mode from the module that actually installs the trap
 *  vectors
 ********************************************************************/
#ifdef __cplusplus
extern "C" {
#endif


//-------------------------------------------------------------------
// Initialization
//-------------------------------------------------------------------
Err			MemInit(void)
							SYS_TRAP(sysTrapMemInit);
							
Err			MemKernelInit(void)
							SYS_TRAP(sysTrapMemKernelInit);
	
Err			MemInitHeapTable(UInt16 cardNo)
							SYS_TRAP(sysTrapMemInitHeapTable);
	
//-------------------------------------------------------------------
// Card formatting and Info
//-------------------------------------------------------------------
UInt16		MemNumCards(void)
							SYS_TRAP(sysTrapMemNumCards);
					
Err			MemCardFormat(UInt16 cardNo, const Char *cardNameP, 
					const Char *manufNameP, const Char *ramStoreNameP)
							SYS_TRAP(sysTrapMemCardFormat);
					
Err			MemCardInfo(UInt16 cardNo, 
					Char *cardNameP, Char *manufNameP,
					UInt16 *versionP, UInt32 *crDateP,
					UInt32 *romSizeP, UInt32 *ramSizeP,
					UInt32 *freeBytesP)
							SYS_TRAP(sysTrapMemCardInfo);


//-------------------------------------------------------------------
// Store Info
//-------------------------------------------------------------------
Err			MemStoreInfo(UInt16 cardNo, UInt16 storeNumber,  
					UInt16 *versionP, UInt16 *flagsP, Char *nameP,
					UInt32 *	crDateP, UInt32 *bckUpDateP,
					UInt32 *	heapListOffsetP, UInt32 *initCodeOffset1P,
					UInt32 *initCodeOffset2P, LocalID*	databaseDirIDP)
							SYS_TRAP(sysTrapMemStoreInfo);

Err			MemStoreSetInfo(UInt16 cardNo, UInt16 storeNumber,
					UInt16 *versionP, UInt16 *flagsP,  Char *nameP, 
					UInt32 *crDateP, UInt32 *bckUpDateP, 
					UInt32 *heapListOffsetP, UInt32 *initCodeOffset1P,
					UInt32 *initCodeOffset2P, LocalID*	databaseDirIDP)
							SYS_TRAP(sysTrapMemStoreSetInfo);


//-------------------------------------------------------------------
// Heap Info & Utilities
//-------------------------------------------------------------------
UInt16			MemNumHeaps(UInt16 cardNo)
							SYS_TRAP(sysTrapMemNumHeaps);
	
UInt16			MemNumRAMHeaps(UInt16 cardNo)
							SYS_TRAP(sysTrapMemNumRAMHeaps);
	
UInt16			MemHeapID(UInt16 cardNo, UInt16 heapIndex)
							SYS_TRAP(sysTrapMemHeapID);
	
Boolean			MemHeapDynamic(UInt16 heapID)
							SYS_TRAP(sysTrapMemHeapDynamic);
	
Err				MemHeapFreeBytes(UInt16 heapID, UInt32 *freeP, UInt32 *maxP)
							SYS_TRAP(sysTrapMemHeapFreeBytes);
							
UInt32	  		MemHeapSize(UInt16 heapID)
							SYS_TRAP(sysTrapMemHeapSize);
							
UInt16			MemHeapFlags(UInt16 heapID)
							SYS_TRAP(sysTrapMemHeapFlags);


// Heap utilities
Err				MemHeapCompact(UInt16 heapID)
							SYS_TRAP(sysTrapMemHeapCompact);
							
Err				MemHeapInit(UInt16 heapID, Int16 numHandles, Boolean initContents)
							SYS_TRAP(sysTrapMemHeapInit);
							
Err				MemHeapFreeByOwnerID(UInt16 heapID, UInt16 ownerID)
							SYS_TRAP(sysTrapMemHeapFreeByOwnerID);


//-------------------------------------------------------------------
// Low Level Allocation
//-------------------------------------------------------------------
MemPtr			MemChunkNew(UInt16 heapID, UInt32 size, UInt16 attr)
							SYS_TRAP(sysTrapMemChunkNew);
							
Err				MemChunkFree(MemPtr chunkDataP)
							SYS_TRAP(sysTrapMemChunkFree);



//-------------------------------------------------------------------
// Pointer (Non-Movable) based Chunk Routines
//-------------------------------------------------------------------
MemPtr			MemPtrNew(UInt32 size) 
							SYS_TRAP(sysTrapMemPtrNew);
							
#define			MemPtrFree(	p) \
						MemChunkFree(p)

// Getting Attributes
MemHandle			MemPtrRecoverHandle(MemPtr p)
							SYS_TRAP(sysTrapMemPtrRecoverHandle);

UInt16			MemPtrFlags(MemPtr p)
							SYS_TRAP(sysTrapMemPtrFlags);

UInt32			MemPtrSize(MemPtr p)
							SYS_TRAP(sysTrapMemPtrSize);
							
UInt16			MemPtrOwner(MemPtr p)
							SYS_TRAP(sysTrapMemPtrOwner);
							
UInt16			MemPtrHeapID(MemPtr p)
							SYS_TRAP(sysTrapMemPtrHeapID);
							
Boolean			MemPtrDataStorage(MemPtr p)
							SYS_TRAP(sysTrapMemPtrDataStorage);
							
UInt16			MemPtrCardNo(MemPtr p)
							SYS_TRAP(sysTrapMemPtrCardNo);
							
LocalID			MemPtrToLocalID(MemPtr p)
							SYS_TRAP(sysTrapMemPtrToLocalID);

// Setting Attributes
Err				MemPtrSetOwner(MemPtr p, UInt16 owner)
							SYS_TRAP(sysTrapMemPtrSetOwner);
							
Err				MemPtrResize(MemPtr p, UInt32 newSize)
							SYS_TRAP(sysTrapMemPtrResize);
							
Err				MemPtrResetLock(MemPtr p)
							SYS_TRAP(sysTrapMemPtrResetLock);

Err				MemPtrUnlock(MemPtr p)
							SYS_TRAP(sysTrapMemPtrUnlock);


//-------------------------------------------------------------------
// Handle (Movable) based Chunk Routines
//-------------------------------------------------------------------
MemHandle			MemHandleNew(UInt32 size)
							SYS_TRAP(sysTrapMemHandleNew);
							
Err				MemHandleFree(MemHandle h)
							SYS_TRAP(sysTrapMemHandleFree);

// Getting Attributes
UInt16			MemHandleFlags(MemHandle h) 
							SYS_TRAP(sysTrapMemHandleFlags);				

UInt32			MemHandleSize(MemHandle h) 
							SYS_TRAP(sysTrapMemHandleSize);

UInt16			MemHandleOwner(MemHandle h) 
							SYS_TRAP(sysTrapMemHandleOwner);

UInt16			MemHandleLockCount(MemHandle h)
							SYS_TRAP(sysTrapMemHandleLockCount);
							
UInt16			MemHandleHeapID(MemHandle h) 
							SYS_TRAP(sysTrapMemHandleHeapID);

Boolean			MemHandleDataStorage(MemHandle h) 
							SYS_TRAP(sysTrapMemHandleDataStorage);

UInt16			MemHandleCardNo(MemHandle h) 
							SYS_TRAP(sysTrapMemHandleCardNo);

LocalID			MemHandleToLocalID(MemHandle h)
							SYS_TRAP(sysTrapMemHandleToLocalID);


// Setting Attributes
Err				MemHandleSetOwner( MemHandle h,  UInt16 owner) 
							SYS_TRAP(sysTrapMemHandleSetOwner);
						
Err				MemHandleResize(MemHandle h,  UInt32 newSize) 
							SYS_TRAP(sysTrapMemHandleResize);
						
MemPtr			MemHandleLock(MemHandle h)
							SYS_TRAP(sysTrapMemHandleLock);
							
Err				MemHandleUnlock(MemHandle h)
							SYS_TRAP(sysTrapMemHandleUnlock);
							
Err				MemHandleResetLock(MemHandle h) 
							SYS_TRAP(sysTrapMemHandleResetLock);
							
							


//-------------------------------------------------------------------
// Local ID based routines
//-------------------------------------------------------------------
MemPtr			MemLocalIDToGlobal(LocalID local, UInt16 cardNo)
							SYS_TRAP(sysTrapMemLocalIDToGlobal);
							
LocalIDKind		MemLocalIDKind(LocalID local)
							SYS_TRAP(sysTrapMemLocalIDKind);

MemPtr			MemLocalIDToPtr(LocalID local, UInt16 cardNo)
							SYS_TRAP(sysTrapMemLocalIDToPtr);

MemPtr			MemLocalIDToLockedPtr(LocalID local, UInt16 cardNo)
							SYS_TRAP(sysTrapMemLocalIDToLockedPtr);


//-------------------------------------------------------------------
// Utilities
//-------------------------------------------------------------------
Err				MemMove(void *dstP, const void *sP, Int32 numBytes)
							SYS_TRAP(sysTrapMemMove);
							
Err				MemSet(void *dstP, Int32 numBytes, UInt8 value)
							SYS_TRAP(sysTrapMemSet);

Int16				MemCmp (const void *s1, const void *s2, Int32 numBytes)
							SYS_TRAP(sysTrapMemCmp);

Err				MemSemaphoreReserve(Boolean writeAccess)
							SYS_TRAP(sysTrapMemSemaphoreReserve);
							
Err				MemSemaphoreRelease(Boolean writeAccess)
							SYS_TRAP(sysTrapMemSemaphoreRelease);

//-------------------------------------------------------------------
// Debugging Support
//-------------------------------------------------------------------
UInt16			MemDebugMode(void)
							SYS_TRAP(sysTrapMemDebugMode);

Err				MemSetDebugMode(UInt16 flags)
							SYS_TRAP(sysTrapMemSetDebugMode);

Err				MemHeapScramble(UInt16 heapID)
							SYS_TRAP(sysTrapMemHeapScramble);
							
Err				MemHeapCheck(UInt16 heapID)
							SYS_TRAP(sysTrapMemHeapCheck);


#ifdef __cplusplus 
}
#endif





#endif  // __MEMORYMGR_H__
