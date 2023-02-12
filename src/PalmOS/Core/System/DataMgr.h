/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: DataMgr.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *		Header for the Data Manager
 *
 *****************************************************************************/

#ifndef __DATAMGR_H__
#define __DATAMGR_H__


// Include elementary types
#include <PalmTypes.h>					// Basic types
#include <CoreTraps.h>					// Trap Numbers.
#include <ErrorBase.h>					// Error numbers

// Other headers we depend on
#include <MemoryMgr.h>				 


typedef UInt32	DmResType;
typedef UInt16	DmResID;

/************************************************************
 * Category equates
 *************************************************************/
#define	dmRecAttrCategoryMask	0x0F	// mask for category #
#define	dmRecNumCategories		16		// number of categories
#define	dmCategoryLength			16		// 15 chars + 1 null terminator

#define  dmAllCategories			0xff
#define  dmUnfiledCategory  		0

#define	dmMaxRecordIndex			0xffff



// Record Attributes
//
// *** IMPORTANT:
// ***
// *** Any changes to record attributes must be reflected in dmAllRecAttrs and dmSysOnlyRecAttrs ***
// ***
// *** Only one nibble is available for record attributes
//
// *** ANY CHANGES MADE TO THESE ATTRIBUTES MUST BE REFLECTED IN DESKTOP LINK
// *** SERVER CODE (DLCommon.h, DLServer.c)
#define	dmRecAttrDelete			0x80	// delete this record next sync
#define	dmRecAttrDirty				0x40	// archive this record next sync
#define	dmRecAttrBusy				0x20	// record currently in use
#define	dmRecAttrSecret			0x10	// "secret" record - password protected


// All record atributes (for error-checking)
#define	dmAllRecAttrs				( dmRecAttrDelete |			\
												dmRecAttrDirty |			\
												dmRecAttrBusy |			\
												dmRecAttrSecret )

// Record attributes which only the system is allowed to change (for error-checking)
#define	dmSysOnlyRecAttrs			( dmRecAttrBusy )


/************************************************************
 * Database Header equates
 *************************************************************/
#define	dmDBNameLength				32			// 31 chars + 1 null terminator

// Attributes of a Database
//
// *** IMPORTANT:
// ***
// *** Any changes to database attributes must be reflected in dmAllHdrAttrs and dmSysOnlyHdrAttrs ***
// ***
#define	dmHdrAttrResDB					0x0001	// Resource database
#define 	dmHdrAttrReadOnly				0x0002	// Read Only database
#define	dmHdrAttrAppInfoDirty		0x0004	// Set if Application Info block is dirty
															// Optionally supported by an App's conduit
#define	dmHdrAttrBackup				0x0008	//	Set if database should be backed up to PC if
															//	no app-specific synchronization conduit has
															//	been supplied.
#define	dmHdrAttrOKToInstallNewer 	0x0010	// This tells the backup conduit that it's OK
															//  for it to install a newer version of this database
															//  with a different name if the current database is
															//  open. This mechanism is used to update the 
															//  Graffiti Shortcuts database, for example. 
#define	dmHdrAttrResetAfterInstall	0x0020 	// Device requires a reset after this database is 
															// installed.
#define	dmHdrAttrCopyPrevention		0x0040	// This database should not be copied to 

#define	dmHdrAttrStream				0x0080	// This database is used for file stream implementation.
#define	dmHdrAttrHidden				0x0100	// This database should generally be hidden from view
															//  used to hide some apps from the main view of the
															//  launcher for example.
															// For data (non-resource) databases, this hides the record
															//	 count within the launcher info screen.
#define	dmHdrAttrLaunchableData		0x0200	// This data database (not applicable for executables)
															//  can be "launched" by passing it's name to it's owner
															//  app ('appl' database with same creator) using
															//  the sysAppLaunchCmdOpenNamedDB action code. 

#define	dmHdrAttrRecyclable			0x0400	// This database (resource or record) is recyclable:
															//  it will be deleted Real Soon Now, generally the next
															//  time the database is closed. 

#define	dmHdrAttrBundle				0x0800	// This database (resource or record) is associated with
															// the application with the same creator. It will be beamed
															// and copied along with the application. 

#define	dmHdrAttrOpen					0x8000	// Database not closed properly


// All database atributes (for error-checking)
#define	dmAllHdrAttrs					(	dmHdrAttrResDB |						\
													dmHdrAttrReadOnly |					\
													dmHdrAttrAppInfoDirty |				\
													dmHdrAttrBackup |						\
													dmHdrAttrOKToInstallNewer |		\
													dmHdrAttrResetAfterInstall |		\
													dmHdrAttrCopyPrevention |			\
													dmHdrAttrStream |						\
													dmHdrAttrHidden |						\
													dmHdrAttrLaunchableData |			\
													dmHdrAttrRecyclable |				\
													dmHdrAttrBundle |						\
													dmHdrAttrOpen	)
													
// Database attributes which only the system is allowed to change (for error-checking)
#define	dmSysOnlyHdrAttrs				(	dmHdrAttrResDB |		\
													dmHdrAttrOpen	)


/************************************************************
 * Unique ID equates
 *************************************************************/
#define	dmRecordIDReservedRange		1			// The range of upper bits in the database's
															// uniqueIDSeed from 0 to this number are
															// reserved and not randomly picked when a
															// database is created.
#define	dmDefaultRecordsID			0			// Records in a default database are copied
															// with their uniqueIDSeeds set in this range.
#define	dmUnusedRecordID				0			// Record ID not allowed on the device


/************************************************************
 * Mode flags passed to DmOpenDatabase
 *************************************************************/
#define	dmModeReadOnly				0x0001		// read  access
#define	dmModeWrite					0x0002		// write access
#define	dmModeReadWrite			0x0003		// read & write access
#define	dmModeLeaveOpen			0x0004		// leave open when app quits
#define	dmModeExclusive			0x0008		// don't let anyone else open it
#define	dmModeShowSecret			0x0010		// force show of secret records

// Generic type used to represent an open Database
typedef	void *						DmOpenRef;


/************************************************************
 * Structure passed to DmGetNextDatabaseByTypeCreator and used
 *  to cache search information between multiple searches.
 *************************************************************/
typedef struct {
	UInt32		info[8];
	void *p;
	} DmSearchStateType;
typedef DmSearchStateType*	DmSearchStatePtr;	



/************************************************************
 * Structures used by the sorting routines
 *************************************************************/
typedef struct {
	UInt8			attributes;							// record attributes;
	UInt8			uniqueID[3];						// unique ID of record
	} SortRecordInfoType;

typedef SortRecordInfoType *SortRecordInfoPtr;

typedef Int16 DmComparF (void *, void *, Int16 other, SortRecordInfoPtr, 
								SortRecordInfoPtr, MemHandle appInfoH);



/************************************************************
 * Database manager error codes
 * the constant dmErrorClass is defined in ErrorBase.h
 *************************************************************/
#define	dmErrMemError					(dmErrorClass | 1)
#define	dmErrIndexOutOfRange			(dmErrorClass | 2)
#define	dmErrInvalidParam				(dmErrorClass | 3)
#define	dmErrReadOnly					(dmErrorClass | 4)
#define	dmErrDatabaseOpen				(dmErrorClass | 5)
#define	dmErrCantOpen					(dmErrorClass | 6)
#define	dmErrCantFind					(dmErrorClass | 7)
#define	dmErrRecordInWrongCard		(dmErrorClass | 8)
#define	dmErrCorruptDatabase			(dmErrorClass | 9)
#define	dmErrRecordDeleted			(dmErrorClass | 10)
#define	dmErrRecordArchived			(dmErrorClass | 11)
#define	dmErrNotRecordDB				(dmErrorClass | 12)
#define	dmErrNotResourceDB			(dmErrorClass | 13)
#define	dmErrROMBased					(dmErrorClass | 14)
#define	dmErrRecordBusy				(dmErrorClass | 15)
#define	dmErrResourceNotFound		(dmErrorClass | 16)
#define	dmErrNoOpenDatabase			(dmErrorClass | 17)
#define	dmErrInvalidCategory			(dmErrorClass | 18)
#define	dmErrNotValidRecord			(dmErrorClass | 19)
#define	dmErrWriteOutOfBounds		(dmErrorClass | 20)
#define	dmErrSeekFailed				(dmErrorClass | 21)
#define	dmErrAlreadyOpenForWrites	(dmErrorClass | 22)
#define	dmErrOpenedByAnotherTask	(dmErrorClass | 23)
#define  dmErrUniqueIDNotFound		(dmErrorClass | 24)
#define  dmErrAlreadyExists			(dmErrorClass | 25)
#define	dmErrInvalidDatabaseName	(dmErrorClass | 26)
#define	dmErrDatabaseProtected		(dmErrorClass | 27)
#define	dmErrDatabaseNotProtected	(dmErrorClass | 28)

/************************************************************
 * Values for the direction parameter of DmSeekRecordInCategory
 *************************************************************/
#define dmSeekForward				 1
#define dmSeekBackward				-1


/************************************************************
 * Data Manager procedures
 *************************************************************/
#ifdef __cplusplus
extern "C" {
#endif


// Initialization
Err		DmInit(void)
							SYS_TRAP(sysTrapDmInit);


// Directory Lists
Err		DmCreateDatabase(UInt16 cardNo, const Char *nameP, 
					UInt32 creator, UInt32 type, Boolean resDB)
							SYS_TRAP(sysTrapDmCreateDatabase);

Err		DmCreateDatabaseFromImage(MemPtr bufferP)
							SYS_TRAP(sysTrapDmCreateDatabaseFromImage);


Err		DmDeleteDatabase(UInt16 cardNo, LocalID dbID)
							SYS_TRAP(sysTrapDmDeleteDatabase);

UInt16	DmNumDatabases(UInt16 cardNo)
							SYS_TRAP(sysTrapDmNumDatabases);
							
LocalID	DmGetDatabase(UInt16 cardNo, UInt16 index)
							SYS_TRAP(sysTrapDmGetDatabase);

LocalID	DmFindDatabase(UInt16 cardNo, const Char *nameP)
							SYS_TRAP(sysTrapDmFindDatabase);

Err		DmGetNextDatabaseByTypeCreator(Boolean newSearch, DmSearchStatePtr stateInfoP,
			 		UInt32	type, UInt32 creator, Boolean onlyLatestVers, 
			 		UInt16 *cardNoP, LocalID *dbIDP)
							SYS_TRAP(sysTrapDmGetNextDatabaseByTypeCreator);


// Database info
Err		DmDatabaseInfo(UInt16 cardNo, LocalID	dbID, Char *nameP,
					UInt16 *attributesP, UInt16 *versionP, UInt32 *crDateP,
					UInt32 *	modDateP, UInt32 *bckUpDateP,
					UInt32 *	modNumP, LocalID *appInfoIDP,
					LocalID *sortInfoIDP, UInt32 *typeP,
					UInt32 *creatorP)
							SYS_TRAP(sysTrapDmDatabaseInfo);

Err		DmSetDatabaseInfo(UInt16 cardNo, LocalID	dbID, const Char *nameP,
					UInt16 *attributesP, UInt16 *versionP, UInt32 *crDateP,
					UInt32 *	modDateP, UInt32 *bckUpDateP,
					UInt32 *	modNumP, LocalID *appInfoIDP,
					LocalID *sortInfoIDP, UInt32 *typeP,
					UInt32 *creatorP)
							SYS_TRAP(sysTrapDmSetDatabaseInfo);

Err		DmDatabaseSize(UInt16 cardNo, LocalID dbID, UInt32 *numRecordsP,
					UInt32 *	totalBytesP, UInt32 *dataBytesP)
							SYS_TRAP(sysTrapDmDatabaseSize);
							
							
// This routine can be used to prevent a database from being deleted (by passing
//  true for 'protect'). It will increment the protect count if 'protect' is true
//  and decrement it if 'protect' is false. This is used by code that wants to
//  keep a particular record or resource in a database locked down but doesn't
//  want to keep the database open. This information is keep in the dynamic heap so
//  all databases are "unprotected" at system reset. 
Err		DmDatabaseProtect(UInt16 cardNo, LocalID dbID, Boolean protect)
							SYS_TRAP(sysTrapDmDatabaseProtect);


// Open/close Databases
DmOpenRef	DmOpenDatabase(UInt16 cardNo, LocalID dbID, UInt16 mode)
							SYS_TRAP(sysTrapDmOpenDatabase);

DmOpenRef	DmOpenDatabaseByTypeCreator(UInt32 type, UInt32 creator, UInt16 mode)
							SYS_TRAP(sysTrapDmOpenDatabaseByTypeCreator);
							
DmOpenRef	DmOpenDBNoOverlay(UInt16 cardNo, LocalID dbID, UInt16 mode)
							SYS_TRAP(sysTrapDmOpenDBNoOverlay);

Err			DmCloseDatabase(DmOpenRef dbP)
							SYS_TRAP(sysTrapDmCloseDatabase);


// Info on open databases
DmOpenRef	DmNextOpenDatabase(DmOpenRef currentP)
							SYS_TRAP(sysTrapDmNextOpenDatabase);
							
Err			DmOpenDatabaseInfo(DmOpenRef dbP, LocalID *dbIDP, 
					UInt16 *openCountP, UInt16 *modeP, UInt16 *cardNoP,
					Boolean *resDBP)
							SYS_TRAP(sysTrapDmOpenDatabaseInfo);

LocalID		DmGetAppInfoID (DmOpenRef dbP)
							SYS_TRAP(sysTrapDmGetAppInfoID);

void DmGetDatabaseLockState(DmOpenRef dbR, UInt8 *highest, UInt32 *count, UInt32 *busy)
							SYS_TRAP(sysTrapDmGetDatabaseLockState);

// Utility to unlock all records and clear busy bits
Err			DmResetRecordStates(DmOpenRef dbP)
							SYS_TRAP(sysTrapDmResetRecordStates);


// Error Query
Err			DmGetLastErr(void)
							SYS_TRAP(sysTrapDmGetLastErr);


//------------------------------------------------------------
// Record based access routines
//------------------------------------------------------------

// Record Info
UInt16	DmNumRecords(DmOpenRef dbP)
							SYS_TRAP(sysTrapDmNumRecords);

UInt16	DmNumRecordsInCategory(DmOpenRef dbP, UInt16 category)
							SYS_TRAP(sysTrapDmNumRecordsInCategory);

Err		DmRecordInfo(DmOpenRef dbP, UInt16 index,
					UInt16 *attrP, UInt32 *uniqueIDP, LocalID *chunkIDP)
							SYS_TRAP(sysTrapDmRecordInfo);

Err		DmSetRecordInfo(DmOpenRef dbP, UInt16 index,
					UInt16 *attrP, UInt32 *uniqueIDP)
							SYS_TRAP(sysTrapDmSetRecordInfo);
							


// Record attaching and detaching
Err		DmAttachRecord(DmOpenRef dbP, UInt16 *atP,
					MemHandle	newH, MemHandle *oldHP)
							SYS_TRAP(sysTrapDmAttachRecord);

Err		DmDetachRecord(DmOpenRef dbP, UInt16 index,
					MemHandle *oldHP)
							SYS_TRAP(sysTrapDmDetachRecord);
					
Err		DmMoveRecord(DmOpenRef dbP, UInt16 from, UInt16 to)
							SYS_TRAP(sysTrapDmMoveRecord);



// Record creation and deletion
MemHandle	DmNewRecord(DmOpenRef dbP, UInt16 *atP, UInt32 size)
							SYS_TRAP(sysTrapDmNewRecord);

Err		DmRemoveRecord(DmOpenRef dbP, UInt16 index)
							SYS_TRAP(sysTrapDmRemoveRecord);

Err		DmDeleteRecord(DmOpenRef dbP, UInt16 index)
							SYS_TRAP(sysTrapDmDeleteRecord);

Err		DmArchiveRecord(DmOpenRef dbP, UInt16 index)
							SYS_TRAP(sysTrapDmArchiveRecord);

MemHandle	DmNewHandle(DmOpenRef dbP, UInt32 size)
							SYS_TRAP(sysTrapDmNewHandle);

Err		DmRemoveSecretRecords(DmOpenRef dbP)
							SYS_TRAP(sysTrapDmRemoveSecretRecords);


// Record viewing manipulation
Err		DmFindRecordByID(DmOpenRef dbP, UInt32 uniqueID, UInt16 *indexP)
							SYS_TRAP(sysTrapDmFindRecordByID);

MemHandle	DmQueryRecord(DmOpenRef dbP, UInt16 index)
							SYS_TRAP(sysTrapDmQueryRecord);

MemHandle	DmGetRecord(DmOpenRef dbP, UInt16 index)
							SYS_TRAP(sysTrapDmGetRecord);
							
MemHandle	DmQueryNextInCategory(DmOpenRef dbP, UInt16 *indexP, UInt16 category)
							SYS_TRAP(sysTrapDmQueryNextInCategory);
							
UInt16	DmPositionInCategory (DmOpenRef dbP, UInt16 index, UInt16 category)
							SYS_TRAP(sysTrapDmPositionInCategory);
							
Err		DmSeekRecordInCategory (DmOpenRef dbP, UInt16 *indexP, UInt16 offset,
					Int16 direction, UInt16 category)
							SYS_TRAP(sysTrapDmSeekRecordInCategory);


MemHandle	DmResizeRecord(DmOpenRef dbP, UInt16 index, UInt32 newSize)
							SYS_TRAP(sysTrapDmResizeRecord);

Err		DmReleaseRecord(DmOpenRef dbP, UInt16 index, Boolean dirty)
							SYS_TRAP(sysTrapDmReleaseRecord);

UInt16	DmSearchRecord(MemHandle recH, DmOpenRef *dbPP)
							SYS_TRAP(sysTrapDmSearchRecord);


// Category manipulation
Err		DmMoveCategory (DmOpenRef dbP, UInt16 toCategory, UInt16 fromCategory, Boolean dirty)
							SYS_TRAP(sysTrapDmMoveCategory);

Err		DmDeleteCategory (DmOpenRef dbR, UInt16 categoryNum)
							SYS_TRAP(sysTrapDmDeleteCategory);
							
							
// Validation for writing
Err		DmWriteCheck(void *recordP, UInt32 offset, UInt32 bytes)
							SYS_TRAP(sysTrapDmWriteCheck);
							
// Writing
Err		DmWrite(void *recordP, UInt32 offset, const void *srcP, UInt32 bytes)
							SYS_TRAP(sysTrapDmWrite);
							
Err		DmStrCopy(void *recordP, UInt32 offset, const Char *srcP)
							SYS_TRAP(sysTrapDmStrCopy);

Err		DmSet(void *recordP, UInt32 offset, UInt32 bytes, UInt8 value)
							SYS_TRAP(sysTrapDmSet);
							

							

//------------------------------------------------------------
// Resource based access routines
//------------------------------------------------------------

// High level access routines
MemHandle	DmGetResource(DmResType type, DmResID resID)
							SYS_TRAP(sysTrapDmGetResource);

MemHandle	DmGet1Resource(DmResType type, DmResID resID)
							SYS_TRAP(sysTrapDmGet1Resource);

Err			DmReleaseResource(MemHandle resourceH)
							SYS_TRAP(sysTrapDmReleaseResource);

MemHandle		DmResizeResource(MemHandle resourceH, UInt32 newSize)
							SYS_TRAP(sysTrapDmResizeResource);


// Searching resource databases  
DmOpenRef	DmNextOpenResDatabase(DmOpenRef dbP)
							SYS_TRAP(sysTrapDmNextOpenResDatabase);

UInt16		DmFindResourceType(DmOpenRef dbP, DmResType resType, UInt16 typeIndex)
							SYS_TRAP(sysTrapDmFindResourceType);

UInt16		DmFindResource(DmOpenRef dbP, DmResType resType, DmResID resID, 
					MemHandle resH)
							SYS_TRAP(sysTrapDmFindResource);

UInt16		DmSearchResource(DmResType resType, DmResID resID,
					MemHandle resH, DmOpenRef *dbPP)
							SYS_TRAP(sysTrapDmSearchResource);


// Resource Info
UInt16		DmNumResources(DmOpenRef dbP)
							SYS_TRAP(sysTrapDmNumResources);

Err			DmResourceInfo(DmOpenRef dbP, UInt16 index,
					DmResType *resTypeP, DmResID *resIDP,  
					LocalID *chunkLocalIDP)
							SYS_TRAP(sysTrapDmResourceInfo);

Err			DmSetResourceInfo(DmOpenRef dbP, UInt16 index,
					DmResType *resTypeP, DmResID *resIDP)
							SYS_TRAP(sysTrapDmSetResourceInfo);



// Resource attaching and detaching
Err			DmAttachResource(DmOpenRef dbP, MemHandle	newH, 
					DmResType resType, DmResID resID)
							SYS_TRAP(sysTrapDmAttachResource);

Err			DmDetachResource(DmOpenRef dbP, UInt16 index,
					MemHandle *oldHP)
							SYS_TRAP(sysTrapDmDetachResource);



// Resource creation and deletion
MemHandle		DmNewResource(DmOpenRef dbP, DmResType resType, DmResID resID,
					UInt32 size)
							SYS_TRAP(sysTrapDmNewResource);

Err			DmRemoveResource(DmOpenRef dbP, UInt16 index)
							SYS_TRAP(sysTrapDmRemoveResource);



// Resource manipulation
MemHandle		DmGetResourceIndex(DmOpenRef dbP, UInt16 index)
							SYS_TRAP(sysTrapDmGetResourceIndex);



// Record sorting
Err 			DmQuickSort(DmOpenRef dbP, DmComparF *compar, Int16 other)
							SYS_TRAP(sysTrapDmQuickSort);

Err			DmInsertionSort (DmOpenRef dbR, DmComparF *compar, Int16 other)
							SYS_TRAP(sysTrapDmInsertionSort);

UInt16		DmFindSortPosition(DmOpenRef dbP, void *newRecord,
					SortRecordInfoPtr newRecordInfo, DmComparF *compar, Int16 other)
							SYS_TRAP(sysTrapDmFindSortPosition);

UInt16		DmFindSortPositionV10(DmOpenRef dbP, void *newRecord,
					DmComparF *compar, Int16 other)
							SYS_TRAP(sysTrapDmFindSortPositionV10);

#ifdef __cplusplus
}
#endif

#endif // __DATAMGR_H__
