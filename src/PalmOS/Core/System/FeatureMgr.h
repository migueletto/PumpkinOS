/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: FeatureMgr.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *		Header for the Feature Manager
 *
 *****************************************************************************/

#ifndef __FEATUREMGR_H__
#define __FEATUREMGR_H__


#include <PalmTypes.h>
#include <CoreTraps.h>
#include <ErrorBase.h>



/************************************************************
 * Feature manager error codes
 * the constant ftrErrorClass is defined in ErrorBase.h
 *************************************************************/
#define	ftrErrInvalidParam				(ftrErrorClass | 1)
#define	ftrErrNoSuchFeature				(ftrErrorClass | 2)
#define	ftrErrAlreadyExists				(ftrErrorClass | 3)
#define	ftrErrROMBased						(ftrErrorClass | 4)
#define	ftrErrInternalErr					(ftrErrorClass | 5)


/************************************************************
 * Feature Manager procedures
 *************************************************************/
#ifdef __cplusplus
extern "C" {
#endif


// Init the feature Manager
Err		FtrInit(void)
							SYS_TRAP(sysTrapFtrInit);


// Get a feature
Err		FtrGet(UInt32 creator, UInt16 featureNum, UInt32 *valueP)
							SYS_TRAP(sysTrapFtrGet);

// Set/Create a feature.
Err		FtrSet(UInt32 creator, UInt16 featureNum, UInt32 newValue)
							SYS_TRAP(sysTrapFtrSet);

// Unregister a feature
Err		FtrUnregister(UInt32 creator, UInt16 featureNum)
							SYS_TRAP(sysTrapFtrUnregister);

// Get a feature by index
Err		FtrGetByIndex(UInt16 index, Boolean romTable, 
					UInt32 *creatorP, UInt16 *numP, UInt32 *valueP)
							SYS_TRAP(sysTrapFtrGetByIndex);

// Get temporary space from storage heap
Err		FtrPtrNew(UInt32 creator, UInt16 featureNum, UInt32 size,
					void **newPtrP)
							SYS_TRAP(sysTrapFtrPtrNew);

// Release temporary space to storage heap
Err		FtrPtrFree(UInt32 creator, UInt16 featureNum)
							SYS_TRAP(sysTrapFtrPtrFree);


// Resize block of temporary storage
Err		FtrPtrResize(UInt32 creator, UInt16 featureNum, UInt32 newSize,
					void **newPtrP)
							SYS_TRAP(sysTrapFtrPtrResize);


#ifdef __cplusplus
}
#endif

#endif // __FEATUREMGR_H__
