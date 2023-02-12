/******************************************************************************
 *
 * Copyright (c) 1995-2003 PalmSource, Inc. All rights reserved.
 *
 * File: DLCommon.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *		Desktop Link Protocol(DLP) function id's, parameters, and frame
 *		structures.
 *
 *****************************************************************************/

#ifndef __DLCOMMON_H__
#define __DLCOMMON_H__

#include <PalmTypes.h>

#include "VFSMgr.h"

/************************************************************
 * DesktopLink function codes
 *************************************************************/

// DlpFuncID -- function id's used in request blocks sent to
// the DesktopLink application on Pilot.  The width of the function
// id base is 7 bits.  The high order bit(the 8th bit) is used to
// distinguish requests from their responses --  it is cleared in request
// blocks and is set in response blocks (i.e., the response to a particular
// command will have the same function code base as the command and the
// high order bit set).  See dlpFuncRespFlag defined below.
//
typedef enum DlpFuncID {

	dlpReservedFunc = 0x0F,			// range reserved for internal use

	// DLP 1.0 FUNCTIONS START HERE (PalmOS v1.0)
	dlpReadUserInfo,
	
	dlpWriteUserInfo,
	
	dlpReadSysInfo,
	
	dlpGetSysDateTime,
	
	dlpSetSysDateTime,
	
	dlpReadStorageInfo,
	
	dlpReadDBList,
	
	dlpOpenDB,
	
	dlpCreateDB,
	
	dlpCloseDB,
	
	dlpDeleteDB,
	
	dlpReadAppBlock,
	
	dlpWriteAppBlock,

	dlpReadSortBlock,

	dlpWriteSortBlock,

	dlpReadNextModifiedRec,

	dlpReadRecord,

	dlpWriteRecord,

	dlpDeleteRecord,

	dlpReadResource,

	dlpWriteResource,

	dlpDeleteResource,

	dlpCleanUpDatabase,

	dlpResetSyncFlags,

	dlpCallApplication,

	dlpResetSystem,
	
	dlpAddSyncLogEntry,
	
	dlpReadOpenDBInfo,
	
	dlpMoveCategory,
	
	dlpProcessRPC,					// remote procedure calls interface
	
	dlpOpenConduit,				// this command is sent before each conduit is opened
	
	dlpEndOfSync,					// ends the sync session
	
	dlpResetRecordIndex,			// resets "modified record" index
	
	dlpReadRecordIDList,			// LAST 1.0 FUNCTION
	
	
	// DLP 1.1 FUNCTIONS ADDED HERE (PalmOS v2.0 Personal, and Professional)
	dlpReadNextRecInCategory,	// iterate through all records in category
	
	dlpReadNextModifiedRecInCategory,	// iterate through modified records in category
	
	dlpReadAppPreference,		// read application preference
	
	dlpWriteAppPreference,		// write application preference
	
	dlpReadNetSyncInfo,			// read Network HotSync settings
	
	dlpWriteNetSyncInfo,			// write Network HotSync settings

	dlpReadFeature,				// read a feature from Feature Manager
	
		// DLP 1.2 FUNCTIONS ADDED HERE (PalmOS v3.0)
	dlpFindDB,						// find a database given creator and type, or name, or
										// get info on currently-open db
	dlpSetDBInfo,					// change database information (name, attributes, version,
										// creation, modification, backup dates, type and creator

	
	// DLP 1.3 FUNCTIONS ADDED HERE (PalmOS v4.0)
	dlpLoopBackTest,				// Perform a loopback test with the DLServer.  The desktop
										// will send data down to the device and the device will 
										// immediately echo it back.
	
	dlpExpSlotEnumerate,				// Get the number of slots on the device from the expansion manager
	
	dlpExpCardPresent,			// Query the expansion manager to see if the card is present.
	
	dlpExpCardInfo,      
	
	dlpVFSCustomControl,			// Make a custom control call to the VFS manager
	
	dlpVFSGetDefaultDirectory,	// Get the defualt directory from the VFS manager
	
	// ADH Does this one really need to be part of the sync API?
	dlpVFSImportDatabaseFromFile, // Import a database into the storage heap from a file
	
	dlpVFSExportDatabaseToFile, /// Export a database from the storage heap to a file.
	
	dlpVFSFileCreate,					// Create a file using the VFS manager
	
	dlpVFSFileOpen,					// Open a file using the VFS manager
	
	dlpVFSFileClose,					// Close a file using the VFS manager
	
	dlpVFSFileWrite,					// Write a file using the VFS manager
	
	dlpVFSFileRead,					// Read from a file using the VFS manager
	
	dlpVFSFileDelete,					// Delete a file using the VFS manager
	
	dlpVFSFileRename,					// Rename a file using the VFS manager
	
	dlpVFSFileEOF,						// VFS End of File?
	
	dlpVFSFileTell,					// ???
	
//	dlpVFSFileTruncate,
	
	dlpVFSFileGetAttributes,
	
	dlpVFSFileSetAttributes,
	
	dlpVFSFileGetDates,
	
	dlpVFSFileSetDates,
	
//	dlpVFSFileGetSize,
	
	dlpVFSDirCreate,
	
	dlpVFSDirEntryEnumerate,
	
	dlpVFSGetFile,						// HotSync only
	
	dlpVFSPutFile,						// HotSync only
	
	dlpVFSVolumeFormat,	
	
	dlpVFSVolumeEnumerate,
	
	dlpVFSVolumeInfo,
	
	dlpVFSVolumeGetLabel,
	
	dlpVFSVolumeSetLabel,
	
	dlpVFSVolumeSize,
	
	dlpVFSFileSeek,
	
	dlpVFSFileResize,
	
	dlpVFSFileSize,
	
	dlpExpSlotMediaType,
	
	dlpWriteResourceStream,
	
	dlpWriteRecordStream,
	
	dlpReadResourceStream,
	
	dlpReadRecordStream,
	
	dlpLastFunc						// ***ALWAYS KEEP LAST***

	} DlpFuncID;

#define	dlpLastPilotV10FuncID	dlpReadRecordIDList


// Desktop Link function error codes returned in the response errorCode
// field.
typedef enum DlpRespErrorCode {
	dlpRespErrNone = 0,					// reserve 0 for no error
	dlpRespErrSystem,						// general Pilot system error
	dlpRespErrIllegalReq,				// unknown function ID
	dlpRespErrMemory,						// insufficient dynamic heap memory
	dlpRespErrParam,						// invalid parameter
	dlpRespErrNotFound,					// database, record, file, or resource not found
												// VFS File Not Found Error
	dlpRespErrNoneOpen,					// there are no open databases
	dlpRespErrDatabaseOpen,				// database is open by someone else
	dlpRespErrTooManyOpenDatabases,	// there are too many open databases
	dlpRespErrAlreadyExists,			// DB or File already exists
												// VFS File Already Exists
	dlpRespErrCantOpen,					// couldn't open DB
	dlpRespErrRecordDeleted,			// record is deleted
	dlpRespErrRecordBusy,				// record is in use by someone else
	dlpRespErrNotSupported,				// the requested operation is not supported
												// on the given database type(record or resource)
	dlpRespErrUnused1,					// was dlpRespErrROMBased
	dlpRespErrReadOnly,					// caller does not have write access(or DB is in ROM)
	dlpRespErrNotEnoughSpace,			// not enough space in data store for record/resource/etc.
	dlpRespErrLimitExceeded,			// size limit exceeded
	dlpRespErrCancelSync,				// cancel the sync
	
	dlpRespErrBadWrapper,				// bad arg wrapper(for debugging)
	dlpRespErrArgMissing,				// required arg not found(for debugging)
	dlpRespErrArgSize,					// invalid argument size
	
	dlpRespErrLastReserved = 127,		// End of DLP Error codes
												
	dlpRespErrExpansionRange = expErrorClass, // Expansion Manager range
	
	dlpRespErrVFSRange = vfsErrorClass		// VFS Manager range									
												
	} DlpRespErrorCode;


// Database flags
// NOTE: THESE *MUST* MATCH THE TOUCHDOWN DB ATTRIBUTES(AT LEAST IN THE FIRST VERSION).
// ANY CHANGES MUST BE REFLECTED IN "READ DB LIST" AND 
#define	dlpDBFlagResDB				0x0001	// resource DB if set; record DB if cleared

#define	dlpDBFlagReadOnly			0x0002	// DB is read only if set; read/write if cleared

#define	dlpDBFlagAppInfoDirty	0x0004	// Set if Application Info block is dirty
														// Optionally supported by an App's conduit

#define	dlpDBFlagBackup			0x0008	//	Set if database should be backed up to PC if
														//	no app-specific synchronization conduit has
														//	been supplied.

#define	dlpDBFlagOKToInstallNewer 	0x0010	// This tells the backup conduit that it's OK
															//  for it to install a newer version of this database
															//  with a different name if the current database is
															//  open. This mechanism is used to update the 
															//  Graffiti Shortcuts database, for example. 

#define	dlpDBFlagResetAfterInstall	0x0020 	// Device requires a reset after this database is 
															// installed.

#define	dlpDBFlagCopyPrevention		0x0040	// This database should not be copied to 
															//  another deviced.

#define	dlpDBFlagOpen					0x8000	// DB is open




// Database record attributes
#define	dlpRecAttrDeleted			0x80	// delete this record next sync
#define	dlpRecAttrDirty			0x40	// archive this record next sync
#define	dlpRecAttrBusy				0x20	// record currently in use
#define	dlpRecAttrSecret			0x10	// "secret" record - password protected
#define	dlpRecAttrArchived		0x08	// archived record


// Date/time will be described in the following format
//			yr(2 bytes), mo(1 byte, 1-12), dy(1 byte, 1-31),
//			hr(1 byte, 0-23), min(1 byte, 0-59), sec(1 byte, 0-59),
//			unused(1 byte).

typedef struct DlpDateTimeType {	// OFFSET
	UInt16	year;						// 0;		year (high, low)
	UInt8		month;					// 2;		month: 1-12
	UInt8		day;						// 3;		day: 1-31
	UInt8		hour;						// 4;		hour: 0-23
	UInt8		minute;					// 5;		minute: 0-59
	UInt8		second;					// 6;		second: 0-59
	UInt8		unused;					// 7;		unused -- set to null!
	} DlpDateTimeType;				// TOTAL: 8 bytes


// Version structure
typedef struct DlpVersionType {
												// OFFSET
	UInt16				wMajor;			// 0;		major version number (0 = ignore)
	UInt16				wMinor;			// 2;		minor version number
												// TOTAL: 4 bytes										
	} DlpVersionType;

 
/************************************************************
 * Request and Response headers.
 *
 * Each DLP request and response data block begins with the
 * corresponding header structure which identifies the function
 * id, argument count, and error code(responses only).
 *************************************************************/

// Request header:
//
typedef struct DlpReqHeaderType {
												// OFFSET
	UInt8					id;				// 0;		request function ID
	UInt8					argc;				// 2;		count of args that follow this header
												// TOTAL: 2 bytes
	} DlpReqHeaderType;

typedef DlpReqHeaderType*		DlpReqHeaderPtr;

typedef struct DlpReqType {
												// OFFSET
	DlpReqHeaderType	header;			// 0;		request header
												// FIXED SIZE: 2 bytes
	UInt8					args[2];			// 2;		request arguments -- var size
	} DlpReqType;

typedef DlpReqType*		DlpReqPtr;
 
// Response header:
//
typedef struct DlpRespHeaderType {
												// OFFSET
	UInt8					id;				// 0;		response function ID
	UInt8					argc;				// 1;		count of arguments that follow this header
	UInt16				errorCode;		// 2;		error code
												// TOTAL: 4 bytes
	} DlpRespHeaderType;

typedef DlpRespHeaderType*		DlpRespHeaderPtr;

typedef struct DlpRespType {
												// OFFSET
	DlpRespHeaderType	header;			// 0;		response header
												// FIXED SIZE: 4 bytes
	UInt8					args[2];			// 4;		response arguments -- var size
	} DlpRespType;
	
typedef DlpRespType*		DlpRespPtr;


// Generic request/response body type(for utility routines)
//
typedef union DlpGenericBodyType {
	UInt8					id;				// request/response id
	DlpReqType			req;				// request body
	DlpRespType			resp;				// response body
	} DlpGenericBodyType;
	
typedef DlpGenericBodyType*		DlpGenericBodyPtr;


// dlpFuncRespFlag is used to form a function response ID from a
// function ID by or'ing it with the function ID.  For example: if
// dlpFuncDeleteResource is the request ID, the correct response ID
// must be (dlpFuncDeleteResource | dlpFuncRespFlag).
//
#define dlpFuncRespFlag				0x80

// dlpFuncIDMask is used to mask out the function ID value
#define dlpFuncIDMask				0x7f

// dlpFirstArgID is the value of the first argument ID to be defined for
// functions.  Values below it are reserved.
//
#define dlpFirstArgID				0x20


/************************************************************
 *
 * Argument headers used to "wrap" request and response arguments
 *
 * IMPORTANT:	ARGUMENT WRAPPERS IN REQUESTS AND RESPONSES MUST
 *					ALWAYS START ON AN EVEN-BYTE BOUNDARY.  The server
 *					implementation expects this to be the case.
 *
 *************************************************************/

// dlpSmallArgFlag is used to identify "small" argument wrappers by
// or'ing it with the argument id in argument header.
//
// ADH Change
//#define dlpSmallArgFlag			0x080
#define dlpSmallArgFlag			0x80

// dlpShortArgIDMask is used to mask out the argument id value
//
#define dlpShortArgIDMask	0x7F


// dlpLongArgFlag is used to identify "long" argument wrappers by
// or'ing it with the argument id in argument header.
//
// ADH Change
//#define dlpLongArgFlag			0xC000
#define dlpLongArgFlag			0x40

// dlpLongArgIDMask is used to mask out the argument id value
//
// ADH Change
//#define dlpLongArgIDMask		0x3FFF
#define dlpLongArgIDMask		0xBF


//-------------------------------------------------------------------------
// Int16 argument wrappers (v1.0-compatible)
//-------------------------------------------------------------------------

// Maximum Int16 argument size which can be "wrapped"
#define dlpMaxTinyArgSize				0x000000FFL
#define dlpMaxSmallArgSize				0x0000FFFFL
#define dlpMaxShortArgSize				dlpMaxSmallArgSize



// Tiny argument header for data sizes up to 255 bytes(optimization)
//
typedef struct DlpTinyArgWrapperType {
												// OFFSET
	UInt8					bID;				// 0;		argument ID
	UInt8					bSize;			// 1;		argument size (does NOT include this arg header)
												// TOTAL: 2 bytes
	} DlpTinyArgWrapperType;
	
typedef struct DlpTinyArgType {
	DlpTinyArgWrapperType	wrapper;	// 0;		argument header
	UInt8							data[2];	// 2;		argument data -- var size
	} DlpTinyArgType;
	

// Small argument header for data sizes above 255 bytes(*may also be used for
// smaller arguments when convenient*)
//
typedef struct DlpSmallArgWrapperType {
												// OFFSET
	UInt8					bID;				// 0;		argument ID
	UInt8					unused;			// 1;		unused(for alignment) -- set to null!
	UInt16					wSize;			// 2;		argument size (does NOT include this arg header)
												// TOTAL: 4 bytes
	} DlpSmallArgWrapperType;

typedef struct DlpSmallArgType {
	DlpSmallArgWrapperType	wrapper;	// 0;		argument header
	UInt8							data[2];	// 4;		argument data -- var size
	} DlpSmallArgType;

// Unions of Int16 argument types
typedef union DlpShortArgWrapperType {
	UInt8							bID;		// arg id
	DlpTinyArgWrapperType	tiny;		// "tiny" arg wrapper
	DlpSmallArgWrapperType	small;	// "tiny" arg wrapper
	} DlpShortArgWrapperType;
typedef DlpShortArgWrapperType*	DlpShortArgWrapperPtr;
	
typedef union DlpShortArgType {
	UInt8							bID;		// arg id
	DlpTinyArgType				tiny;		// "tiny" arg
	DlpSmallArgType			small;	// "small" arg
	} DlpShortArgType;

typedef DlpShortArgType*			DlpShortArgPtr;



//-------------------------------------------------------------------------
// Int32 argument wrapper (v2.0 extension)
//-------------------------------------------------------------------------
// NOTE: Pilot v2.0 will implement the capability to parse long arguments
// but will not originate them.  This will assure backwards compatibility with
// the 1.0 desktop as well as compatibility with the future version of the
// desktop software which may originate the "long" argument wrappers.
//
// Int32 argument wrappers are identified by the dlpLongArgFlag bits set
// in the argument id field.

// Maximum long argument size which can be "wrapped"
#define dlpMaxLongArgSize				0xFFFFFFFFL

typedef struct DlpLongArgWrapperType {
												// OFFSET
	UInt8					bID;				// 0;		argument ID
	UInt8					unused;			// 1:		unused set to null
	UInt32				dwSize;			// 2;		argument size (does NOT include this arg header)
												// TOTAL: 6 bytes
	} DlpLongArgWrapperType;
typedef DlpLongArgWrapperType*		DlpLongArgWrapperPtr;

typedef struct DlpLongArgType {
												// OFFSET
	DlpLongArgWrapperType	wrapper;	// 0;		argument header
	UInt8							data[2];	// 6;		argument data -- var size
	} DlpLongArgType;
typedef DlpLongArgType*	DlpLongArgPtr;


//-------------------------------------------------------------------------
// Unions of all argument and wrapper types
//-------------------------------------------------------------------------

// Union of all argument wrapper types
typedef union DlpGenericArgWrapperType {
	DlpShortArgWrapperType	shortWrap;	// "Int16" arg wrapper(tiny and small)
	DlpLongArgWrapperType	longWrap;	// "long" arg wrapper
	} DlpGenericArgWrapperType;
typedef DlpGenericArgWrapperType*		DlpGenericArgWrapperPtr;


// Union of all argument types
typedef union DlpGenericArgType {
	DlpShortArgType			shortArg;	// "Int16" arg(tiny and small)
	DlpLongArgType				longArg;		// "long" arg
	} DlpGenericArgType;
typedef DlpGenericArgType*			DlpGenericArgPtr;


/********************************************************************
 * Desktop Link Protocol Parameters
 ********************************************************************/

// dlpCmdTimeoutSec -- this is the number of seconds to wait for a command
// to begin coming in before timing out
//
#define dlpCmdTimeoutSec				30



/************************************************************
 * DLP function argument structures
 *************************************************************/
#pragma mark *** DLP Command Arguments ****

//////////////////////////////////////////////////////////////////////////
// dlpReadUserInfo
//////////////////////////////////////////////////////////////////////////
//		Request arguments: none
//
//		Response arguments:
//
//		Possible error codes
//			dlpRespErrSystem,
//			dlpRespErrMemory

// Request argument structure:
//
//		none.
	
// Response argument structure:
//
#define dlpReadUserInfoRespArgID		dlpFirstArgID

typedef struct DlpReadUserInfoRespHdrType {
												// OFFSET
	UInt32				userID;			//	0;		user ID number (0 if none)
	UInt32				viewerID;		// 4;		id assigned to viewer by the desktop
	UInt32				lastSyncPC;		// 8;		last sync PC id (0 if none)
	DlpDateTimeType	succSyncDate;	//	12;	last successful sync (year = 0 if none)
	DlpDateTimeType	lastSyncDate;	//	20;	last sync date(year = 0 if none)
	UInt8					userNameLen;	//	28;	length of user name field,
												//			including null (0 = no user name)
	UInt8					passwordLen;	//	29;	length of encrypted password
												//			(0 = no password set)
												// TOTAL: 30 bytes										
	} DlpReadUserInfoRespHdrType;

typedef struct DlpReadUserInfoRespType {
												// OFFSET
	DlpReadUserInfoRespHdrType			//
							header;			//	0;		fixed-size header
												// FIXED SIZE: 30 bytes										
	UInt8					nameAndPassword[2];	// 30;	user name -- var size
	// User name begins at the nameAndPassword field and is null-terminated.
	// The encrypted password follows the user name and is NOT null-terminated.
	// The encrypted password may contain any byte values(0-255).
	} DlpReadUserInfoRespType;


	
//////////////////////////////////////////////////////////////////////////
//	dlpWriteUserInfo
//////////////////////////////////////////////////////////////////////////
//		Request arguments:
//			user id (4 bytes)
//			last sync PC id(4 bytes)
//			user name
//
//		Response arguments: none
//
//		Possible error codes
//			dlpRespErrSystem,
//			dlpRespErrNotEnoughSpace,
//			dlpRespErrParam

// Request argument structure:
//
#define dlpWriteUserInfoReqArgID		dlpFirstArgID

#define dlpMaxUserNameSize		41		// max user name size, including null byte

typedef struct DlpWriteUserInfoReqHdrType {
												// OFFSET
	UInt32				userID;			//	0;		user ID number
	UInt32				viewerID;		// 4;		id assigned to viewer by the desktop
	UInt32				lastSyncPC;		// 8;		last sync PC id
	DlpDateTimeType	lastSyncDate;	//	12;	last sync date(year = 0 if none)
	UInt8					modFlags;		// 20;	flags indicating which values are being
												//			modified; see the dlpUserInfoMod...
												//			flags defined below
	UInt8					userNameLen;	//	21;	user name length, including null
												// TOTAL: 22 bytes										
	} DlpWriteUserInfoReqHdrType;

// Flags indicating which values are being changed by the dlpWriteUserInfo
// request.  These flags are used in the modFlags field of DlpWriteUserInfoReqHdrType.
// These flags are additive.
//
#define dlpUserInfoModUserID			0x80	// changing the user id
#define dlpUserInfoModSyncPC			0x40	// changing the last sync PC id
#define dlpUserInfoModSyncDate		0x20	// changing sync date
#define dlpUserInfoModName				0x10	// changing user name
#define dlpUserInfoModViewerID		0x08	// changing the viewer id

typedef struct DlpWriteUserInfoReqType {
												// OFFSET
	DlpWriteUserInfoReqHdrType
							header;			//	0;		fixed-size header
												// FIXED SIZE: 22 bytes										
	UInt8					userName[2];	// 22;	user name -- var size
	// User name begins at the userName field and is null-terminated.
	} DlpWriteUserInfoReqType;

	
// Response argument structure:
//
//		none.


	
//////////////////////////////////////////////////////////////////////////
//	dlpReadSysInfo
//////////////////////////////////////////////////////////////////////////
//
//		Possible error codes
//			dlpRespErrSystem
//

// Request argument structure:
//
//		none. DLP v1.0 and v1.1

enum {
	// NEW FOR DLP v1.2:
	dlpReadSysInfoReqArgID		= dlpFirstArgID
	};


// dlpReadSysInfoReqArgID request arg structure
//
typedef struct DlpReadSysInfoReqType {
												// OFFSET
	DlpVersionType		dlpVer;			// 0;		DLP version of the caller
												// TOTAL: 4 bytes										
	} DlpReadSysInfoReqType;
	
	
// Response argument structure:
//
// Both response arguments are returned in one reply
enum {
	dlpReadSysInfoRespArgID		= dlpFirstArgID,
	
	// NEW FOR DLP v1.2:
	dlpReadSysInfoVerRespArgID
	};

// dlpReadSysInfoRespArgID response arg structure:
//
typedef struct DlpReadSysInfoRespType {
												// OFFSET
	UInt32				romSWVersion;	// 0;		ROM-based sys software version
	UInt32				localizationID;// 4;		localization ID
	UInt8					unused;			//	8;		unused(for alignment) -- set to null!
	UInt8					prodIDSize;		// 9;		size of productID/model field
	UInt32				prodID;			// 10;	product id (was variable size)
												// TOTAL: 14 bytes
	} DlpReadSysInfoRespType;
	

// dlpReadSysInfoVerRespArgID response arg structure:
//
typedef struct DlpReadSysInfoVerRespType {
												// OFFSET
	DlpVersionType		dlpVer;			// 0;		DLP version of the device
	DlpVersionType		compVer;			// 4;		product compatibility version of the device
	UInt32				dwMaxRecSize;	// 8;		maximum record/resource size that may be allocated on
												//			the device given that sufficient free memory exists
												//			(0xFFFFFFFF = up to available memory)
												// TOTAL: 12 bytes
	} DlpReadSysInfoVerRespType;



//////////////////////////////////////////////////////////////////////////
//	dlpGetSysDateTime
//////////////////////////////////////////////////////////////////////////
//
//		Possible error codes: none

// Request argument structure:
//
//		none.
	
// Response argument structure:
//
#define dlpGetSysDateTimeRespArgID		dlpFirstArgID

typedef struct DlpGetSysDateTimeRespType {
												// OFFSET
	DlpDateTimeType	dateTime;		// 0;		system date/time
												// TOTAL: 8 bytes
	} DlpGetSysDateTimeRespType;



//////////////////////////////////////////////////////////////////////////
//	dlpSetSysDateTime
//////////////////////////////////////////////////////////////////////////
//		Request arguments:
//			new system date/time
//
//		Response arguments: none
//
//		Possible error codes
//			dlpRespErrParam

// Request argument structure:
//
#define dlpSetSysDateTimeReqArgID		dlpFirstArgID

typedef struct DlpSetSysDateTimeReqType {
												// OFFSET
	DlpDateTimeType	dateTime;		// 0;		new system date/time
												// TOTAL: 8 bytes
	} DlpSetSysDateTimeReqType;

typedef DlpSetSysDateTimeReqType*	DlpSetSysDateTimeReqPtr;
	
// Response argument structure:
//
//		none.


	
//////////////////////////////////////////////////////////////////////////
//	dlpReadStorageInfo
//////////////////////////////////////////////////////////////////////////
//
//		Possible error codes
//			dlpRespErrSystem,
//			dlpRespErrMemory,
//			dlpRespErrNotFound

// Request argument structure:
//
enum {
	dlpReadStorageInfoReqArgID		= dlpFirstArgID
	};

typedef struct DlpReadStorInfoReqType {
												// OFFSET
	UInt8					startCardNo;	// 0;		card number to start at
												//			(0 = first)
	UInt8					unused;			//	1;		unused -- set to null!
												// TOTAL: 2 bytes
	} DlpReadStorInfoReqType;


// Response argument structure:
//
enum {
	dlpReadStorageInfoRespArgID		= dlpFirstArgID,
	dlpReadStorageInfoExRespArgID		// v1.1 extension
	};

//
// dlpReadStorageInfoRespArgID:
//

// Card info structure of variable size
typedef struct DlpCardInfoHdrType {
												// OFFSET
	UInt8					totalSize;		// 0;		total size of this card info
												//			*ROUNDED UP TO EVEN SIZE*
	UInt8					cardNo;			// 1;		card number
	UInt16				cardVersion;	// 2;		card version
	DlpDateTimeType	crDate;			// 4;		creation date/time
	UInt32				romSize;			// 12;	ROM size
	UInt32				ramSize;			// 16;	RAM size
	UInt32				freeRam;			// 20;	total free data store RAM - Fixed in DLP v1.2 to exclude
												//			dynamic heap RAM
	UInt8					cardNameSize;	// 24;	size of card name string
	UInt8					manufNameSize;	// 25;	size of manuf. name string
												// TOTAL: 26 bytes;
	} DlpCardInfoHdrType;

typedef struct DlpCardInfoType {
												// OFFSET
	DlpCardInfoHdrType					//
							header;			// 0;		fixed-size header
												// FIXED SIZE: 26 bytes;
	UInt8					cardNameAndManuf[2];
												// 26;	card name and manuf. text -- var size
	// Card name is the cardNameSize bytes of text at cardNameAndManuf,
	// followed immediately by manufNameSize bytes of manufacturer name.
	} DlpCardInfoType;

	
typedef struct DlpReadStorInfoRespHdrType {
												// OFFSET
	UInt8					lastCardNo;		// 0;		card number of last card retrieved
	UInt8					more;				//	1;		non-zero if there are more cards
	UInt8					unused;			//	2;		unused -- set to null!
	UInt8					actCount;		// 3;		actual count of structures returned
												// TOTAL: 4 bytes
	} DlpReadStorInfoRespHdrType;

typedef struct DlpReadStorInfoRespType {
												// OFFSET
	DlpReadStorInfoRespHdrType			//
							header;			// 0;		fixed-size header
												// FIXED SIZE: 4 bytes
	DlpCardInfoType	cardInfo[1];	// 4;		actCount of card info structures -- var size
	} DlpReadStorInfoRespType;


//
// EXTENDED ARGUMENTS(DL v1.1): dlpReadStorageInfoExRespArgID
//
typedef struct DlpReadStorInfoExRespType {
												// OFFSET
	UInt16				romDBCount;		// 0;		ROM database count
	UInt16				ramDBCount;		// 2;		RAM database count
	UInt32				dwReserved1;	// 4;		RESERVED -- SET TO NULL!
	UInt32				dwReserved2;	// 8;		RESERVED -- SET TO NULL!
	UInt32				dwReserved3;	// 12;	RESERVED -- SET TO NULL!
	UInt32				dwReserved4;	// 16;	RESERVED -- SET TO NULL!
												// TOTAL: 20 bytes
	} DlpReadStorInfoExRespType;
	
	
	
	
//////////////////////////////////////////////////////////////////////////
//	dlpReadDBList
//////////////////////////////////////////////////////////////////////////
//
//		Possible error codes
//			dlpRespErrSystem,
//			dlpRespErrMemory,
//			dlpRespErrNotFound

// Request argument structure:
//
#define dlpReadDBListReqArgID		dlpFirstArgID

typedef struct DlpReadDBListReqType {
												// OFFSET
	UInt8					srchFlags;		// 0;		search flags
	UInt8					cardNo;			// 1;		card number -- 0-based
	UInt16				startIndex;		// 2;		DB index to start at
												//			(0 = from beginning)
												// TOTAL: 4 bytes
	} DlpReadDBListReqType;

#define dlpReadDBListFlagRAM			0x80		// Search for RAM-based
#define dlpReadDBListFlagROM			0x40		// Search for ROM-based
#define dlpReadDBListFlagMultiple	0x20		// OK to return multiple entries (DEFINED FOR DLP v1.2)


// Response argument structure:
//
#define dlpReadDBListRespArgID		dlpFirstArgID

// Database info structure of variable size
typedef struct DlpDBInfoHdrType {
												// OFFSET
	UInt8					totalSize;		// 0;		total size of the DB info (DlpDBInfoHdrType + name)
												//			*ROUNDED UP TO EVEN SIZE*
	UInt8					miscFlags;		//	1;		dlpDbInfoMiscFlag... flags(v1.1) -- set all unused bits to null!
	UInt16				dbFlags;			// 2;		DB flags: dlpDBFlagReadOnly,
												//			dlpDBFlagResDB, dlpDBFlagAppInfoDirty, dlpDBFlagOpen,
												//			dlpDBFlagBackup, etc;
	UInt32				type;				// 4;		database type
	UInt32				creator;			// 8;		database creator
	UInt16				version;			// 12;	database version
	UInt32				modNum;			// 14;	modification number
	DlpDateTimeType	crDate;			// 18;	creation date
	DlpDateTimeType	modDate;			// 26;	latest modification date
	DlpDateTimeType	backupDate;		// 34;	latest backup date
	UInt16				dbIndex;			//	42;	DB index (or dlpDbInfoUnknownDbIndex for dlpFindDB)
												// TOTAL: 44 bytes;
	} DlpDBInfoHdrType;

// Flags for the miscFlags field of DlpDBInfoHdrType
#define dlpDbInfoMiscFlagExcludeFromSync	0x80		// DEFINED FOR DLP v1.1
#define dlpDbInfoMiscFlagRamBased			0x40		// DEFINED FOR DLP v1.2

// Unknown index value for the dbIndex field of DlpDBInfoHdrType
#define dlpDbInfoUnknownDbIndex		0xFFFF


typedef struct DlpDBInfoType {
												// OFFSET
	DlpDBInfoHdrType	header;			// 0;		fixed-size header
												// FIXED SIZE: 44 bytes;
	UInt8					name[2];			// 44;	databse name text -- var size and
												//			null-terminated
	} DlpDBInfoType;

	
typedef struct DlpReadDBListRespHdrType {
												// OFFSET
	UInt16				lastIndex;		// 0;		DB index of last entry retrieved
	UInt8					flags;			// 2;		flags: dlpReadDBListRespFlagMore
	UInt8					actCount;		// 3;		actual count of structures returned
												// TOTAL: 4 bytes
	} DlpReadDBListRespHdrType;

// dlpReadDBListRespFlagMore flag: if set, indicates that there are more
// databases to list -- this enables the server to send the listing
// incrementally, reducing server memory requirements if necessary
#define dlpReadDBListRespFlagMore	0x80
	
typedef struct DlpReadDBListRespType {
												// OFFSET
	DlpReadDBListRespHdrType			//
							header;			//	0;		fixed-size header
												// FIXED SIZE: 4 bytes
	DlpDBInfoType		dbInfo[1];		// 4;		actCount of DB info structures -- var size
	} DlpReadDBListRespType;
	

//////////////////////////////////////////////////////////////////////////
//	dlpOpenDB
//////////////////////////////////////////////////////////////////////////
//
//		Possible error codes
//			dlpRespErrSystem,
//			dlpRespErrMemory,
//			dlpRespErrParam,
//			dlpRespErrNotFound
//			dlpRespErrTooManyOpenDatabases
//			dlpRespErrCantOpen

// Request argument structure:
//
#define dlpOpenDBReqArgID		dlpFirstArgID

typedef struct DlpOpenDBReqHdrType {
												// OFFSET
	UInt8					cardNo;			// 0;		memory module number
	UInt8					mode;				// 1;		open mode
												// TOTAL: 2 bytes;
	} DlpOpenDBReqHdrType;

#define dlpOpenDBModeRead			0x80
#define dlpOpenDBModeWrite			0x40
#define dlpOpenDBModeExclusive	0x20
#define dlpOpenDBModeShowSecret	0x10


typedef struct DlpOpenDBReqType {
												// OFFSET
	DlpOpenDBReqHdrType					//
							header;			//	0;		fixed-size header
												// FIXED SIZE: 2 bytes;
	UInt8					name[2];			// 2;		databse name text -- var size
												//			zero-terminated
	} DlpOpenDBReqType;

typedef DlpOpenDBReqType*		DlpOpenDBReqPtr;

	
// Response argument structure:
//
#define dlpOpenDBRespArgID		dlpFirstArgID
//
// The response argument is the 1-byte database ID to be passed in
// subsequent read/write requests.

	
	
//////////////////////////////////////////////////////////////////////////
//	dlpCreateDB
//////////////////////////////////////////////////////////////////////////
//
//		Possible error codes
//			dlpRespErrSystem,
//			dlpRespErrMemory,
//			dlpRespErrParam,
//			dlpRespErrAlreadyExists,
//			dlpRespErrCantOpen,
//			dlpRespErrNotEnoughSpace,
//			dlpRespErrTooManyOpenDatabases

// Request argument structure:
//
#define dlpCreateDBReqArgID		dlpFirstArgID

typedef struct DlpCreateDBReqHdrType {
												// OFFSET
	UInt32				creator;			// 0;		DB creator
	UInt32				type;				// 4;		DB type
	UInt8					cardNo;			// 8;		memory module number
	UInt8					unused;			//	9;		unused -- set to null
	UInt16				dbFlags;			// 10;	allowed flags: dlpDBFlagResDB,
												//			dlpDBFlagBackup, dlpDBFlagOKToInstallNewer,
												//			dlpDBFlagResetAfterInstall
	UInt16				version;			// 12;	DB version #
												// TOTAL: 14 bytes;
	} DlpCreateDBReqHdrType;

typedef struct DlpCreateDBReqType {
												// OFFSET
	DlpCreateDBReqHdrType				//
							header;			//	0;		fixed-size header
												// FIXED SIZE: 12 bytes;
	UInt8					name[2];			// 14;	DB name text -- var size
												//			zero-terminated
	} DlpCreateDBReqType;

typedef DlpCreateDBReqType*		DlpCreateDBReqPtr;

	
// Response argument structure:
//
#define dlpCreateDBRespArgID		dlpFirstArgID

// The response argument is the 1-byte database ID to be passed in
// subsequent read/write requests.


	
//////////////////////////////////////////////////////////////////////////
//	dlpCloseDB
//////////////////////////////////////////////////////////////////////////
//
//		Possible error codes
//			dlpRespErrParam,
//			dlpRespErrSystem,
//			dlpRespErrMemory,
//			dlpRespErrNoneOpen

// Request argument structure:
//
typedef enum {
	dlpCloseDBReqArgID = dlpFirstArgID,				// close a specific database
	dlpCloseDBAllReqArgID,								// close all databases
	dlpCloseDBExReqArgID									// close a specific db and update backup
																// and/or modification dates (PalmOS v3.0)
	} DlpCloseDBReqArgID;

// Argument structure to close a specific database(dlpCloseDBDBIDReqArgID):
//
// The request argument is the 1-byte database ID returned in open/create
// DB responses.
typedef UInt8 DlpCloseDBReqType;

// Argument structure to close all databases(dlpCloseDBReqAllArgID):
//
// This request argument contains no data


// Request type for dlpCloseDBExReqArgID (PalmOS v3.0):
typedef struct DlpCloseDBExReqType {
												// OFFSET
	UInt8					dbID;				//	0;		database id to close
	UInt8					bOptFlags;		// 1;		bitfield of dlpCloseDBExOptFlag... flags
												// TOTAL: 2 bytes										
	} DlpCloseDBExReqType;

// Option flags
#define dlpCloseDBExOptFlagUpdateBackupDate	0x80	// Update the backup date after closing
#define dlpCloseDBExOptFlagUpdateModDate		0x40	// Update the modification date after closing


#define dlpCloseDBExOptAllFlags	(dlpCloseDBExOptFlagUpdateBackupDate | dlpCloseDBExOptFlagUpdateModDate)


// Response argument structure:
//
//		none.


	
//////////////////////////////////////////////////////////////////////////
//	dlpDeleteDB
//////////////////////////////////////////////////////////////////////////
//
//		Possible error codes
//			dlpRespErrSystem,
//			dlpRespErrParam,
//			dlpRespErrNotFound,
//			dlpRespErrCantOpen,
//			dlpRespErrDatabaseOpen

// Request argument structure:
//
#define dlpDeleteDBReqArgID		dlpFirstArgID

typedef struct DlpDeleteDBReqHdrType {
												// OFFSET
	UInt8					cardNo;			// 0;		memory module number
	UInt8					unused;			// 1;		unused -- set to null!
												// TOTAL: 2 bytes;
	} DlpDeleteDBReqHdrType;


typedef struct DlpDeleteDBReqType {
												// OFFSET
	DlpDeleteDBReqHdrType				//
							header;			//	0;		fixed-size header
												// FIXED SIZE: 2 bytes;
	UInt8					name[2];			// 2;		databse name text -- var size
												//			zero-terminated
	} DlpDeleteDBReqType;

typedef DlpDeleteDBReqType*		DlpDeleteDBReqPtr;


// Response argument structure:
//
//		none.


//////////////////////////////////////////////////////////////////////////
//	dlpReadOpenDBInfo
//////////////////////////////////////////////////////////////////////////
//		Get information on an open database
//
//		Possible error codes
//			dlpRespErrNoneOpen
//			dlpRespErrParam

// Request argument structure:
//
#define dlpReadOpenDBInfoArgID			dlpFirstArgID

// The request argument is the 1-byte database ID returned in open/create
// DB responses.
	

// Response argument structure:
//
#define dlpReadOpenDBInfoRespArgID		dlpFirstArgID


typedef struct DlpReadOpenDBInfoRespType {
												// OFFSET
	UInt16				numRec;			//	0;		number of records or resources
												// TOTAL: 2 bytes
	} DlpReadOpenDBInfoRespType;

	
//////////////////////////////////////////////////////////////////////////
//	dlpMoveCategory
//////////////////////////////////////////////////////////////////////////
//
//		Possible error codes
//			dlpRespErrNoneOpen
//			dlpRespErrParam
//			dlpRespErrNotSupported
//			dlpRespErrReadOnly

// Request argument structure:
//
#define dlpMoveCategoryReqArgID			dlpFirstArgID

typedef struct DlpMoveCategoryReqType {
												// OFFSET
	UInt8					dbID;				//	0;		database id
	UInt8					fromCategory;	//	1;		id of the "from" category
	UInt8					toCategory;		//	2;		id of the "to" category
	UInt8					unused;			//	3;		unused -- set to null!
												// TOTAL: 4 bytes;
	} DlpMoveCategoryReqType;
	

// Response argument structure:
//
//		none.

	
//////////////////////////////////////////////////////////////////////////
//	dlpReadAppBlock
//////////////////////////////////////////////////////////////////////////
//
//		Possible error codes
//			dlpRespErrSystem,
//			dlpRespErrMemory,
//			dlpRespErrNotFound
//			dlpRespErrNoneOpen
//			dlpRespErrParam

// Request argument structure:
//
#define dlpReadBlockReqArgID			dlpFirstArgID

typedef struct DlpReadBlockReqType {
												// OFFSET
	UInt8					dbID;				//	0;		database id
	UInt8					unused;			//	1;		unused -- set to null!
	UInt16				offset;			//	2;		offset into the block
	UInt16				numBytes;		//	4;		number of bytes to read starting
												//			at offset(-1 = to the end)
												// TOTAL: 6 bytes;
	} DlpReadBlockReqType;
	

// Response argument structure:
//
#define dlpReadBlockRespArgID			dlpFirstArgID

typedef struct DlpReadBlockRespHdrType {
												// OFFSET
	UInt16				blockSize;		//	0;		actual block size -- may be greater
												//			than the amount of data returned
												//	TOTAL: 2 bytes
	} DlpReadBlockRespHdrType;

typedef struct DlpReadBlockRespType {
												// OFFSET
	DlpReadBlockRespHdrType				//
							header;			//	0;		fixed size header
												//	FIXED SIZE: 2 bytes
	UInt8					data[2];			// 2;		block data -- var size
	} DlpReadBlockRespType;

	
//////////////////////////////////////////////////////////////////////////
//	dlpWriteAppBlock
//////////////////////////////////////////////////////////////////////////
//
//		Possible error codes
//			dlpRespErrSystem,
//			dlpRespErrParam,
//			dlpRespErrReadOnly
//			dlpRespErrNotEnoughSpace
//			dlpRespErrNoneOpen

// Request argument structure:
//
#define dlpWriteBlockReqArgID			dlpFirstArgID

typedef struct DlpWriteBlockReqHdrType {
												// OFFSET
	UInt8					dbID;				//	0;		database id
	UInt8					unused;			//	1;		unused -- set to null!
	UInt16				blockSize;		//	2;		total block size(0 = free existing block)
												// TOTAL: 4 bytes;
	} DlpWriteBlockReqHdrType;


typedef struct DlpWriteBlockReqType {
												// OFFSET
	DlpWriteBlockReqHdrType				//
							header;			//	0;		fixed-size header
												// FIXED SIZE: 4 bytes;
	UInt8					data[2];			// 4;		block data -- var size
	} DlpWriteBlockReqType;


// Response argument structure:
//
//		none.



//////////////////////////////////////////////////////////////////////////
//	dlpReadSortBlock
//////////////////////////////////////////////////////////////////////////
//
//		Possible error codes
//			dlpRespErrSystem,
//			dlpRespErrMemory
//			dlpRespErrNotFound
//			dlpRespErrNoneOpen

// Request argument structure:
//
// see dlpReadAppBlock


// Response argument structure:
//
// see dlpReadAppBlock


//////////////////////////////////////////////////////////////////////////
//	dlpWriteSortBlock
//////////////////////////////////////////////////////////////////////////
//
//		Possible error codes
//			dlpRespErrSystem,
//			dlpRespErrMemory,
//			dlpRespErrParam,
//			dlpRespErrNoneOpen

// Request argument structure:
//
// see dlpWriteAppBlock


// Response argument structure:
//
//		none.



//////////////////////////////////////////////////////////////////////////
//	dlpReadNextModifiedRec
//////////////////////////////////////////////////////////////////////////
//
//		Possible error codes
//			see dlpReadRecord
//

// Request argument structure:
//
#define dlpReadNextModRecReqArgID		dlpFirstArgID
// The request argument is the 1-byte database ID returned in open/create
// DB responses.


// Response argument structure:
//
// Response argument id = dlpReadRecordRespArgID

//	Response argument structure = DlpReadRecordRespType



//////////////////////////////////////////////////////////////////////////
//	dlpResetRecordIndex
//////////////////////////////////////////////////////////////////////////
//	Resets the "next modified record" index to the beginning
//
//		Possible error codes
//			dlpRespErrParam
//			dlpRespErrNoneOpen

// Request argument structure:
//
#define dlpResetRecordIndexReqArgID		dlpFirstArgID

// The request argument is the 1-byte database ID returned in open/create
// DB responses.

// Response argument structure:
//
//		none.



//////////////////////////////////////////////////////////////////////////
//	dlpReadRecord
//////////////////////////////////////////////////////////////////////////
//
//		Possible error codes
//			dlpRespErrNotSupported,
//			dlpRespErrSystem,
//			dlpRespErrMemory,
//			dlpRespErrParam,
//			dlpRespErrNotFound,
//			dlpRespErrRecordBusy,
//			dlpRespErrNoneOpen

typedef enum {
	dlpReadRecordIdArgID = dlpFirstArgID,
	dlpReadRecordIndexArgID
	} DlpReadRecordReqArgID;

// dlpReadRecordIdArgID request argument structure:
//
typedef struct DlpReadRecordByIDReqType {
												// OFFSET
	UInt8					dbID;				//	0;		database id
	UInt8					unused;			//	1;		unused -- set to null!
	UInt32				recordID;		// 2;		unique record id
	UInt16				offset;			//	6;		offset into the record
	UInt16				numBytes;		//	8;		number of bytes to read starting
												//			at the offset(-1 = "to the end")
												// TOTAL: 10 bytes;
	} DlpReadRecordByIDReqType;

// dlpReadRecordIndexArgID request argument structure:
//
typedef struct DlpReadRecordByIndexReqType {
												// OFFSET
	UInt8					dbID;				//	0;		database id
	UInt8					unused;			//	1;		unused -- set to null!
	UInt16				index;			// 2;		record index
	UInt16				offset;			//	4;		offset into the record
	UInt16				numBytes;		//	6;		number of bytes to read starting
												//			at the offset(-1 = "to the end")
												// TOTAL: 8 bytes;
	} DlpReadRecordByIndexReqType;


// Response argument structure:
//
#define dlpReadRecordRespArgID		dlpFirstArgID

typedef struct DlpReadRecordRespHdrType {
												// OFFSET
	UInt32				recordID;		// 0;		unique record id
	UInt16				index;			//	4;		record index
	UInt16				recSize;			//	6;		total record size in bytes
	UInt8					attributes;		// 8;		record attributes
	UInt8					category;		// 9;		record category index
												// TOTAL: 10 bytes;
	} DlpReadRecordRespHdrType;

typedef struct DlpReadRecordRespType {
												// OFFSET
	DlpReadRecordRespHdrType			//
							header;			//	0;		fixed-size header
												// FIXED SIZE: 8 bytes;
	UInt8					data[2];			// 8;		record data -- var size
	} DlpReadRecordRespType;



//////////////////////////////////////////////////////////////////////////
//	dlpReadRecordIDList
//////////////////////////////////////////////////////////////////////////
//	Returns a list of unique record id's.  May need to call more than once
// to get the entire list.  dlpRespErrNotFound is returned when "start"
// is out of bounds
//
//		Possible error codes
//			dlpRespErrNotSupported,
//			dlpRespErrParam,
//			dlpRespErrNotFound,
//			dlpRespErrNoneOpen

#define dlpReadRecordIDListReqArgID		dlpFirstArgID

// dlpReadRecordIDListReqArgID request argument structure:
//
typedef struct DlpReadRecordIDListReqType {
												// OFFSET
	UInt8					dbID;				//	0;		database id
	UInt8					flags;			//	1;		request flags
	UInt16				start;			// 2;		starting record index (0-based)
	UInt16				maxEntries;		//	4;		maximum number of entries, or
												//			0xFFFF to return as many as possible
												// TOTAL: 6 bytes;
	} DlpReadRecordIDListReqType;

// dlpReadRecordIDListFlagSortDB: if set, DL Server will call the creator
// application to resort the database before returning the list.
#define dlpReadRecordIDListFlagSortDB	0x80


// Response argument structure:
//
#define dlpReadRecordIDListRespArgID		dlpFirstArgID

typedef struct DlpReadRecordIDListRespHdrType {
												// OFFSET
	UInt16				numEntries;		// 0;		number of entries returned
												// TOTAL: 2 bytes;
	} DlpReadRecordIDListRespHdrType;

typedef struct DlpReadRecordIDListRespType {
												// OFFSET
	DlpReadRecordIDListRespHdrType	//
							header;			//	0;		fixed-size header
												// FIXED SIZE: 2 bytes;
	UInt32				recID[1];		// 8;		list of record id's -- var size
	} DlpReadRecordIDListRespType;

	

//////////////////////////////////////////////////////////////////////////
//	dlpWriteRecord
//////////////////////////////////////////////////////////////////////////
//
//		Possible error codes
//			dlpRespErrSystem,
//			dlpRespErrParam,
//			dlpRespErrNotFound
//			dlpRespErrNotEnoughSpace
//			dlpRespErrNotSupported
//			dlpRespErrReadOnly
//			dlpRespErrNoneOpen

// Request argument structure:
//
#define dlpWriteRecordReqArgID		dlpFirstArgID

// dlpWriteRecordReqArgID -- required
typedef struct DlpWriteRecordReqHdrType {
												// OFFSET
	UInt8					dbID;				//	0;		database id
	UInt8					flags;			//	1;		set unused bits to null!
	UInt32				recordID;		// 2;		unique record id or null
	UInt8					attributes;		// 6;		record attributes -- only
												//			dlpRecAttrSecret is allowed here
												//			v1.1 extension:
												//			dlpRecAttrDeleted, dlpRecAttrArchived and
												//			dlpRecAttrDirty are also allowed.
	UInt8					category;		// 7;		record category
												// TOTAL: 8 bytes;
	} DlpWriteRecordReqHdrType;

#define dlpWriteRecordReqFlagDataIncluded	0x80	// original implementer of destop software always
																// set this bit.  Define it here for compatibility

typedef struct DlpWriteRecordReqType {
												// OFFSET
	DlpWriteRecordReqHdrType			//
							header;			//	0;		fixed-size header
												// FIXED SIZE: 8 bytes;
	UInt8					data[2];			// 8;		record data -- var size
	} DlpWriteRecordReqType;


// Response argument structure:
//
#define dlpWriteRecordRespArgID		dlpFirstArgID

typedef struct DlpWriteRecordRespType {
												// OFFSET
	UInt32				recordID;		// 0;		record ID
												// TOTAL: 4 bytes
	} DlpWriteRecordRespType;



//////////////////////////////////////////////////////////////////////////
//	dlpDeleteRecord
//////////////////////////////////////////////////////////////////////////
//
//		Possible error codes
//			dlpRespErrSystem,
//			dlpRespErrParam,
//			dlpRespErrNotFound
//			dlpRespErrNotSupported
//			dlpRespErrReadOnly
//			dlpRespErrNoneOpen
//

// Request argument structure:
//
#define dlpDeleteRecordReqArgID	dlpFirstArgID

// Argument structure to delete by record ID(dlpDeleteRecordReqIDArgID):
typedef struct DlpDeleteRecordReqType {
												// OFFSET
	UInt8					dbID;				//	0;		database id
	UInt8					flags;			//	1;		flags (dlpDeleteRecFlagDeleteAll)
	UInt32				recordID;		// 2;		unique record id (see flags)
												// TOTAL: 6 bytes;
	} DlpDeleteRecordReqType;

// dlpDeleteRecFlagDeleteAll: if this flag is set, the reocordID field
// is ignored and all database records will be deleted
#define dlpDeleteRecFlagDeleteAll		0x80

// dlpDeleteRecFlagByCategory: if this flag is set, the least significant byte
// of the reocordID field contains the category id of records to be deleted (PalmOS 2.0)
#define dlpDeleteRecFlagByCategory		0x40

// Response argument structure:
//
//		none.



//////////////////////////////////////////////////////////////////////////
//	dlpReadResource
//////////////////////////////////////////////////////////////////////////
//
//		Possible error codes
//			dlpRespErrSystem,
//			dlpRespErrParam,
//			dlpRespErrNotFound
//			dlpRespErrNoneOpen

typedef enum {
	dlpReadResourceIndexArgID = dlpFirstArgID,
	dlpReadResourceTypeArgID
	} DlpReadResourceReqArgID;

// dlpReadResourceIndexArgID request argument structure:
//
typedef struct DlpReadResourceByIndexReqType {
												// OFFSET
	UInt8					dbID;				//	0;		database id
	UInt8					unused;			//	1;		unused -- set to null!
	UInt16				index;			// 2;		resource index
	UInt16				offset;			//	4;		offset into the resource
	UInt16				numBytes;		//	6;		number of bytes to read starting
												//			at the offset(-1 = "to the end")
												// TOTAL: 8 bytes;
	} DlpReadResourceByIndexReqType;

// dlpReadResourceTypeArgID request argument structure:
//
typedef struct DlpReadResourceByTypeReqType {
												// OFFSET
	UInt8					dbID;				//	0;		database id
	UInt8					unused;			//	1;		unused -- set to null!
	UInt32				type;				// 2;		resource type
	UInt16				id;				// 6;		resource id
	UInt16				offset;			//	8;		offset into the resource
	UInt16				numBytes;		//	10;	number of bytes to read starting
												//			at the offset(-1 = "to the end")
												// TOTAL: 12 bytes;
	} DlpReadResourceByTypeReqType;
	

// Response argument structure:
//
#define dlpReadResourceRespArgID		dlpFirstArgID

typedef struct DlpReadResourceRespHdrType {
												// OFFSET
	UInt32				type;				// 0;		resource type
	UInt16				id;				// 4;		resource id
	UInt16				index;			//	6;		resource index
	UInt16				resSize;			//	8;		total resource size in bytes
												// TOTAL: 10 bytes;
	} DlpReadResourceRespHdrType;

typedef struct DlpReadResourceRespType {
												// OFFSET
	DlpReadResourceRespHdrType			//
							header;			//	0;		fixed-size header
												// FIXED SIZE: 6 bytes;
	UInt8					resData[2];		// 6;		resource data -- var size
	} DlpReadResourceRespType;



//////////////////////////////////////////////////////////////////////////
//	dlpWriteResource
//////////////////////////////////////////////////////////////////////////
//
//		Possible error codes
//			dlpRespErrSystem,
//			dlpRespErrNotEnoughSpace,
//			dlpRespErrParam,
//			dlpRespErrNoneOpen

// Request argument structure:
//
#define dlpWriteResourceReqArgID		dlpFirstArgID

typedef struct DlpWriteResourceReqHdrType {
												// OFFSET
	UInt8					dbID;				//	0;		database id
	UInt8					unused;			//	1;		inused -- set to null!
	UInt32				type;				// 2;		resource type
	UInt16				id;				// 6;		resource id
	UInt16				resSize;			//	8;		total resource size
												// TOTAL: 10 bytes;
	} DlpWriteResourceReqHdrType;

typedef struct DlpWriteResourceReqType {
												// OFFSET
	DlpWriteResourceReqHdrType			//
							header;			//	0;		fixed-size header
												// FIXED SIZE: 10 bytes;
	UInt8					data[2];			// 10;		resource data -- var size
	} DlpWriteResourceReqType;

// Response argument structure:
//
//		none.


//////////////////////////////////////////////////////////////////////////
//	dlpWriteResourceStream
//////////////////////////////////////////////////////////////////////////
//
//		Possible error codes
//			dlpRespErrSystem,
//			dlpRespErrNotEnoughSpace,
//			dlpRespErrParam,
//			dlpRespErrNoneOpen
#pragma mark Dm Write Resource Stream

//
// Callback parameters
//
// Write calback parameters
typedef struct DlpWriteResourceCallbackParamType{
										// OFFSET
	MemPtr  dmBufP;				// 0: Locked dm heap handle
	UInt32  numBytes;				// 4:	Expected File Size
	UInt32  offset;
										// TOTAL: 8 bytes
	} DlpWriteResourceCallbackParamType;

typedef DlpWriteResourceCallbackParamType* DlpWriteResourceCallbackParamPtr;

// Request argument structure:
//
#define dlpWriteResourceStreamReqArgID		dlpFirstArgID

typedef struct DlpWriteResourceStreamReqType {
												// OFFSET
	UInt8					dbID;				//	0;		database id
	UInt8					unused;			//	1;		inused -- set to null!
	UInt32				type;				// 2;		resource type
	UInt16				id;				// 6;		resource id
	UInt16				resSize;			//	8;		total resource size
												// TOTAL: 10 bytes;
	} DlpWriteResourceStreamReqType;
	
typedef DlpWriteResourceStreamReqType* DlpWriteResourceStreamReqPtr;

// Response argument structure:
//
//		none.


//////////////////////////////////////////////////////////////////////////
//	dlpWriteRecordStream
//////////////////////////////////////////////////////////////////////////
//
//		Possible error codes
//			dlpRespErrSystem,
//			dlpRespErrParam,
//			dlpRespErrNotFound
//			dlpRespErrNotEnoughSpace
//			dlpRespErrNotSupported
//			dlpRespErrReadOnly
//			dlpRespErrNoneOpen
#pragma mark Dm Write Record Stream

//
// Callback parameters
//
// Write calback parameters
typedef struct DlpWriteRecordCallbackParamType{
										// OFFSET
	MemPtr  dmBufP;				// 0: Locked dm heap handle
	UInt32  numBytes;				// 4:	Expected File Size
	UInt32  offset;
										// TOTAL: 8 bytes
	} DlpWriteRecordCallbackParamType;

typedef DlpWriteRecordCallbackParamType* DlpWriteRecordCallbackParamPtr;

// Request argument structure:
//
#define dlpWriteRecordStreamReqArgID		dlpFirstArgID

// dlpWriteRecordReqArgID -- required
typedef struct DlpWriteRecordStreamReqType {
												// OFFSET
	UInt8					dbID;				//	0;		database id
	UInt8					flags;			//	1;		set unused bits to null!
	UInt32				recordID;		// 2;		unique record id or null
	UInt8					attributes;		// 6;		record attributes -- only
												//			dlpRecAttrSecret is allowed here
												//			v1.1 extension:
												//			dlpRecAttrDeleted, dlpRecAttrArchived and
												//			dlpRecAttrDirty are also allowed.
	UInt8					category;		// 7;		record category
	UInt32				recordSize;
												// TOTAL: 8 bytes;
	} DlpWriteRecordStreamReqType;

typedef DlpWriteRecordStreamReqType* DlpWriteRecordStreamReqPtr;

// Response argument structure:
//
#define dlpWriteRecordStreamRespArgID		dlpFirstArgID

typedef struct DlpWriteRecordStreamRespType {
												// OFFSET
	UInt32				recordID;		// 0;		record ID
												// TOTAL: 4 bytes
	} DlpWriteRecordStreamRespType;





//////////////////////////////////////////////////////////////////////////
//	dlpDeleteResource
//////////////////////////////////////////////////////////////////////////
//
//		Possible error codes
//			dlpRespErrSystem,
//			dlpRespErrParam,
//			dlpRespErrNotFound
//			dlpRespErrNotSupported
//			dlpRespErrReadOnly
//			dlpRespErrNoneOpen

// Request argument structure:
//
#define dlpDeleteResourceReqArgID		dlpFirstArgID

typedef struct DlpDeleteResourceReqType {
												// OFFSET
	UInt8					dbID;				//	0;		database id
	UInt8					flags;			//	1;		flags (dlpDeleteResFlagDeleteAll)
	UInt32				type;				// 2;		resource type
	UInt16				id;				// 6;		resource id
												// TOTAL: 8 bytes;
	} DlpDeleteResourceReqType;

// dlpDeleteResFlagDeleteAll: if set, all resources in the db will be deleted
#define dlpDeleteResFlagDeleteAll	0x80


// Response argument structure:
//
//		none.



//////////////////////////////////////////////////////////////////////////
//	dlpCleanUpDatabase
//////////////////////////////////////////////////////////////////////////
//		Deletes all records which are marked as archived or deleted in the
//		record database
//
//		Possible error codes
//			dlpRespErrSystem,
//			dlpRespErrParam,
//			dlpRespErrReadOnly,
//			dlpRespErrNotSupported
//			dlpRespErrNoneOpen

// Request argument structure:
//
#define dlpCleanUpDatabaseReqArgID		dlpFirstArgID

// The request argument is the 1-byte database ID returned in open/create
// DB responses.

// Response argument structure:
//
//		none.



//////////////////////////////////////////////////////////////////////////
//	dlpResetSyncFlags
//////////////////////////////////////////////////////////////////////////
//		For record databases, reset all dirty flags.
//		For both record and resource databases, set the last sync time to NOW
//
//		Possible error codes
//			dlpRespErrSystem,
//			dlpRespErrParam
//			dlpRespErrReadOnly,
//			dlpRespErrNoneOpen

// Request argument structure:
//
#define dlpResetSyncFlagsReqArgID		dlpFirstArgID

// The request argument is the 1-byte database ID returned in open/create
// DB responses.


// Response argument structure:
//
//		none.



//////////////////////////////////////////////////////////////////////////
//	dlpCallApplication
//////////////////////////////////////////////////////////////////////////
//		Call an application entry point via an action code
//
//		Possible error codes
//			dlpRespErrSystem,
//			dlpRespErrMemory,
//			dlpRespErrParam,
//			dlpRespErrNotFound

// Request argument structure:
//
enum	{
	dlpCallApplicationReqArgIDV10 = dlpFirstArgID,	// req id for Pilot v1.0
	dlpCallAppReqArgID										// req id for Pilot v2.0 and later
	};

// dlpCallApplicationReqArgIDV10:
typedef struct DlpCallApplicationReqHdrTypeV10 {
												// OFFSET
	UInt32				creator;			// 0;		app DB creator id
	UInt16				action;			// 4;		action code
	UInt16				paramSize;		// 6;	custom param size
												// TOTAL: 8 bytes
	} DlpCallApplicationReqHdrTypeV10;

typedef struct DlpCallApplicationReqTypeV10 {
												// OFFSET
	DlpCallApplicationReqHdrTypeV10	//
							header;			//	0;		fixed-size header
												// FIXED SIZE: 8 bytes
	UInt8					paramData[2];	// 8;	custom param data -- var size
	} DlpCallApplicationReqTypeV10;


// dlpCallAppReqArgID (Pilot v2.0):
typedef struct DlpCallAppReqHdrType {
												// OFFSET
	UInt32				creator;			// 0;		DB creator id of target executable
	UInt32				type;				// 4;		DB type id of target executable
	UInt16				action;			// 8;		action code
	UInt32				dwParamSize;	// 10;	custom param size in number of bytes
	UInt32				dwReserved1;	// 14;	RESERVED -- set to NULL!!!
	UInt32				dwReserved2;	// 18;	RESERVED -- set to NULL!!!
												// TOTAL: 22 bytes
	} DlpCallAppReqHdrType;

typedef struct DlpCallAppReqType {
												// OFFSET
	DlpCallAppReqHdrType					//
							hdr;				//	0;		fixed-size header
												// FIXED SIZE: 22 bytes
	UInt8					paramData[2];	// 22;	custom param data -- var size
	} DlpCallAppReqType;



// Response argument structure:
//
enum	{
	dlpCallApplicationRespArgIDV10 = dlpFirstArgID,		// resp id for Pilot v1.0
	dlpCallAppRespArgID											// resp id for Pilot v2.0 and later
	};

// dlpCallApplicationRespArgIDV10:
typedef struct DlpCallApplicationRespHdrTypeV10 {
												// OFFSET
	UInt16				action;			// 0;		action code which was called
	UInt16				resultCode;		// 2;		result error code returned by action
	UInt16				resultSize;		// 4;		custom result data size
												// TOTAL: 6 bytes
	} DlpCallApplicationRespHdrTypeV10;

typedef struct DlpCallApplicationRespTypeV10 {
												// OFFSET
	DlpCallApplicationRespHdrTypeV10	//
							header;			//	0;		fixed-size header
												// FIXED SIZE: 6 bytes
	UInt8					resultData[2];	// 6;		custom result data -- var size
	} DlpCallApplicationRespTypeV10;

// dlpCallAppRespArgID:
typedef struct DlpCallAppRespHdrType {
												// OFFSET
	UInt32					dwResultCode;	// 0;		result error code returned by handler
	UInt32					dwResultSize;	// 4;		custom result data size
	UInt32					dwReserved1;	// 8;		RESERVED -- SET TO NULL!!!
	UInt32					dwReserved2;	// 12;	RESERVED -- SET TO NULL!!!
												// TOTAL: 16 bytes
	} DlpCallAppRespHdrType;

typedef struct DlpCallAppRespType {
												// OFFSET
	DlpCallAppRespHdrType				//
							hdr;				//	0;		fixed-size header
												// FIXED SIZE: 16 bytes
	UInt8					resultData[2];	// 16;		custom result data -- var size
	} DlpCallAppRespType;



//////////////////////////////////////////////////////////////////////////
//	dlpResetSystem
//////////////////////////////////////////////////////////////////////////
//
//		Possible error codes
//			dlpRespErrSystem

// Request argument structure:
//
//		none.

// Response argument structure:
//
//		none.



//////////////////////////////////////////////////////////////////////////
// dlpAddSyncLogEntry
//////////////////////////////////////////////////////////////////////////
//
//		Possible error codes
//			dlpRespErrSystem,
//			dlpRespErrNotEnoughSpace,
//			dlpRespErrLimitExceeded,
//			dlpRespErrParam

// Request argument structure:
//
#define dlpAddSyncLogEntryReqArgID		dlpFirstArgID

typedef struct DlpAddSyncLogEntryReqType {
												// OFFSET
	UInt8					text[2];			// 0;		entry text -- var size and
												//			null-terminated
	} DlpAddSyncLogEntryReqType;

// Response argument structure:
//
//		none.



//////////////////////////////////////////////////////////////////////////
// dlpProcessRPC
//////////////////////////////////////////////////////////////////////////
//		Remote Procedure Call interface
//
//		Request arguments:
//			RPC command block
//
//		Response arguments:
//			RPC reply block
//
//		Possible error codes
//			0 on success; otherwise error code from the TouchDown
//			RPC executor
//
// NOTE: this is a low-level system command which does not use arg wrappers.

// Request argument structure:
//
// Block of RPC command data (no arg wrapper)

// Response argument structure:
//
// Block of RPC reply data of same length as command block(no arg wrapper)

	

//////////////////////////////////////////////////////////////////////////
// dlpOpenConduit
//////////////////////////////////////////////////////////////////////////
//		This command is sent before each conduit is opened by the desktop.
//		If the viewer has a cancel pending, it will return dlpRespErrCancelSync
//		in the response header's errorCode field.
//
//		Request arguments:	none.
//
//		Response arguments:	none.
//
//		Possible error codes
//			dlpRespErrCancelSync

// Request argument structure:
//
//		none.

// Response argument structure:
//
//		none.


	
//////////////////////////////////////////////////////////////////////////
// dlpEndOfSync
//////////////////////////////////////////////////////////////////////////
//		This command is sent by the desktop to end the sync.
//
//		Request arguments:	termination code: 0 = normal termination;
//									otherwise the client is aborting the sync
//
//		Possible error codes
//			0

// Request argument structure:
//
#define dlpEndOfSyncReqArgID		dlpFirstArgID

typedef enum DlpSyncTermCode {
	dlpTermCodeNormal = 0,				// normal termination
	dlpTermCodeOutOfMemory,				// termination due to low memory on TD
	dlpTermCodeUserCan,					// user cancelled from desktop
	dlpTermCodeOther,						// catch-all abnormal termination code
	dlpTermCodeIncompatibleProducts	// incompatibility between desktop and handheld hotsync products
	} DlpSyncTermCode;
	
	
typedef struct DlpEndOfSyncReqType {
												// OFFSET
	UInt16				termCode;		// 0;		termination code
												// TOTAL: 2 bytes
	} DlpEndOfSyncReqType;


// Response argument structure:
//
//		none.



//////////////////////////////////////////////////////////////////////////
//	dlpReadNextRecInCategory
//////////////////////////////////////////////////////////////////////////
//
//		Possible error codes
//			see dlpReadRecord

#define dlpReadNextRecInCategoryReqArgID	dlpFirstArgID

// dlpReadNextRecInCategoryReqArgID request argument structure:
//
typedef struct DlpReadNextRecInCategoryReqType {
												// OFFSET
	UInt8					dbID;				//	0;		database id
	UInt8					category;		//	1;		category id
												// TOTAL: 2 bytes;
	} DlpReadNextRecInCategoryReqType;


// Response argument structure:
//
// Response argument id = dlpReadRecordRespArgID

//	Response argument structure = DlpReadRecordRespType



//////////////////////////////////////////////////////////////////////////
//	dlpReadNextModifiedRecInCategory
//////////////////////////////////////////////////////////////////////////
//
//		Possible error codes
//			see dlpReadNextRecInCategory

// Request argument structure:
//
// same as dlpReadNextRecInCategory

// Response argument structure:
//
// same as dlpReadNextRecInCategory



//////////////////////////////////////////////////////////////////////////
//	dlpReadAppPreference
//////////////////////////////////////////////////////////////////////////
//
//		Possible error codes
//			dlpRespErrSystem,
//			dlpRespErrMemory,
//			dlpRespErrParam,
//			dlpRespErrNotFound,

#define dlpReadAppPreferenceReqArgID	dlpFirstArgID

// dlpReadAppPreferenceReqArgID request argument structure:
//
typedef struct DlpReadAppPreferenceReqType {
												// OFFSET
	UInt32				creator;			//	0;		application creator type
	UInt16				id;				// 4;		preference id
	UInt16				reqBytes;		//	6;		max. number of preference bytes requested;
												//			pass 0xFFFF for actual size
	UInt8					flags;			// 8;		command flags: dlpAppPrefReqFlagBackedUp - if set, use backed-up pref db
	UInt8					unused;			// 9;		reserved/padding -- set to NUL!
												// TOTAL: 10 bytes;
	} DlpReadAppPreferenceReqType;

#define dlpReadAppPrefActualSize		0xFFFF
#define dlpAppPrefReqFlagBackedUp	0x80

// Response argument structure:
//
#define dlpReadAppPreferenceRespArgID		dlpFirstArgID

typedef struct DlpReadAppPreferenceRespHdrType {
												// OFFSET
	UInt16				version;			// 0;		version number of the application
	UInt16				actualSize;		// 2;		actual preference data size
	UInt16				retBytes;		// 4;		number of preference bytes returned
												// TOTAL: 6 bytes
	} DlpReadAppPreferenceRespHdrType;

typedef struct DlpReadAppPreferenceRespType {
												// OFFSET
	DlpReadAppPreferenceRespHdrType	//
							header;			//	0;		fixed-size header
												// FIXED SIZE: 6 bytes
	UInt8					data[2];			// 6;		custom result data -- var size
	} DlpReadAppPreferenceRespType;

	
//////////////////////////////////////////////////////////////////////////
//	dlpWriteAppPreference
//////////////////////////////////////////////////////////////////////////
//
//		Possible error codes
//			dlpRespErrSystem,
//			dlpRespErrParam,
//			dlpRespErrNotEnoughSpace

// Request argument structure:
//
#define dlpWriteAppPreferenceReqArgID		dlpFirstArgID

typedef struct DlpWriteAppPreferenceReqHdrType {
												// OFFSET
	UInt32				creator;			//	0;		application creator type
	UInt16				id;				// 4;		preference id
	UInt16				version;			// 6;		version number of the application
	UInt16				prefSize;		// 8;		preference size(in number of bytes)
	UInt8					flags;			// 10;	command flags: dlpAppPrefReqFlagBackedUp - if set, use backed-up pref db
	UInt8					unused;			// 11;	reserved/padding -- set to NUL!
												// TOTAL: 12 bytes;
	} DlpWriteAppPreferenceReqHdrType;

typedef struct DlpWriteAppPreferenceReqType {
												// OFFSET
	DlpWriteAppPreferenceReqHdrType	//
							header;			//	0;		fixed-size header
												// FIXED SIZE: 12 bytes;
	UInt8					data[2];			// 12;	record data -- var size
	} DlpWriteAppPreferenceReqType;


// Response argument structure:
//
// none.



//////////////////////////////////////////////////////////////////////////
// dlpReadNetSyncInfo
//////////////////////////////////////////////////////////////////////////
//		Request arguments: none
//
//		Response arguments:
//
//		Possible error codes
//			dlpRespErrSystem,
//			dlpRespErrMemory

// Request argument structure:
//
//		none.
	
// Response argument structure:
//
#define dlpReadNetSyncInfoRespArgID		dlpFirstArgID

typedef struct DlpReadNetSyncInfoRespHdrType {
												// OFFSET
	UInt8					lanSyncOn;		//	0;		non-zero if Lan Sync is enabled
	UInt8					bReserved1;		// 1;		reserved -- SET TO NULL!
	UInt32				dwReserved1;	// 2;		reserved -- SET TO NULL!
	UInt32				dwReserved2;	// 6;		reserved -- SET TO NULL!
	UInt32				dwReserved3;	// 10;	reserved -- SET TO NULL!
	UInt32				dwReserved4;	// 14;	reserved -- SET TO NULL!
	UInt16				syncPCNameSize;//	18;	length of sync PC host name,
												//			including null (0 = no host name)
	UInt16				syncPCAddrSize;//	20;	length of sync PC address,
												//			including null (0 = no address)
	UInt16				syncPCMaskSize;//	22;	length of sync PC subnet mask,
												//			including null (0 = no mask)
												// TOTAL: 24 bytes										
	} DlpReadNetSyncInfoRespHdrType;

typedef struct DlpReadNetSyncInfoRespType {
												// OFFSET
	DlpReadNetSyncInfoRespHdrType		//
							hdr;				//	0;		fixed-size header
												// FIXED SIZE: 24 bytes										
	UInt8					syncAddr[2];	// 24;	sync IP address/host name -- var size,
												//			null-terminated
	} DlpReadNetSyncInfoRespType;


	
//////////////////////////////////////////////////////////////////////////
//	dlpWriteNetSyncInfo
//////////////////////////////////////////////////////////////////////////
//		Request arguments:
//
//		Response arguments: none
//
//		Possible error codes
//			dlpRespErrSystem,
//			dlpRespErrNotEnoughSpace,
//			dlpRespErrParam

// Request argument structure:
//
#define dlpWriteNetSyncInfoReqArgID		dlpFirstArgID

#define dlpMaxHostAddrLength	255	// maximum PC host name length, not including NULL

typedef struct DlpWriteNetSyncInfoReqHdrType {
												// OFFSET
	UInt8					modFlags;		// 0;		flags indicating which values are being
												//			modified; see the dlpNetSyncInfoMod...
												//			flags defined below
	UInt8					lanSyncOn;		//	1;		non-zero if Lan Sync is enabled
	UInt32				dwReserved1;	// 2;		reserved -- SET TO NULL!
	UInt32				dwReserved2;	// 6;		reserved -- SET TO NULL!
	UInt32				dwReserved3;	// 10;	reserved -- SET TO NULL!
	UInt32				dwReserved4;	// 14;	reserved -- SET TO NULL!
	UInt16				syncPCNameSize;//	18;	length of sync PC host name,
												//			including null (0 = no address/host name)
	UInt16				syncPCAddrSize;//	20;	length of sync PC address,
												//			including null (0 = no address)
	UInt16				syncPCMaskSize;//	22;	length of sync PC subnet mask,
												//			including null (0 = no mask)
												// TOTAL: 24 bytes										
	} DlpWriteNetSyncInfoReqHdrType;

// Flags indicating which values are being changed by the dlpWriteNetSyncInfo
// request.  These flags are used in the modFlags field of DlpWriteNetSyncInfoReqHdrType.
// These flags are additive.
//
#define dlpNetSyncInfoModLanSyncOn	0x80	// changing the "lan sync on" setting
#define dlpNetSyncInfoModSyncPCName	0x40	// changing the sync PC host name
#define dlpNetSyncInfoModSyncPCAddr	0x20	// changing the sync PC address
#define dlpNetSyncInfoModSyncPCMask	0x10	// changing the sync PC subnet mask

typedef struct DlpWriteNetSyncInfoReqType {
												// OFFSET
	DlpWriteNetSyncInfoReqHdrType
							hdr;				//	0;		fixed-size header
												// FIXED SIZE: 24 bytes										
	UInt8					syncAddr[2];	// 24;	sync IP address/host name -- var size,
												//			null-terminated
	} DlpWriteNetSyncInfoReqType;

	
// Response argument structure:
//
//		none.



//////////////////////////////////////////////////////////////////////////
// dlpReadFeature
//////////////////////////////////////////////////////////////////////////
//		Request arguments:
//
//		Response arguments:
//
//		Possible error codes
//			dlpRespErrSystem,
//			dlpRespErrNotFound,
//			dlpRespErrParam

// Request argument structure:
//
#define dlpReadFeatureReqArgID		dlpFirstArgID

typedef struct DlpReadFeatureReqType {
												// OFFSET
	UInt32				dwFtrCreator;	// 0;		feature creator
	UInt16				wFtrNum;			// 4;		feature number
												// TOTAL: 6 bytes										
	} DlpReadFeatureReqType;
	
	
// Response argument structure:
//
#define dlpReadFeatureRespArgID		dlpFirstArgID

typedef struct DlpReadFeatureRespType {
												// OFFSET
	UInt32				dwFeature;		//	0;		feature value
												// TOTAL: 4 bytes
	} DlpReadFeatureRespType;



//////////////////////////////////////////////////////////////////////////
// dlpFindDB
//////////////////////////////////////////////////////////////////////////
//		Request arguments:
//
//		Response arguments:
//
//		Possible error codes
//			dlpRespErrSystem,
//			dlpRespErrNotFound,
//			dlpRespErrParam

// Request argument structure:
//

// dlpFindDBByNameReqArgID, dlpFindDBByTypeCreatorReqArgID and dlpFindDBByOpenHandleReqArgID
// are mutually exclusive
enum	{
	dlpFindDBByNameReqArgID = dlpFirstArgID,		// req id for card + name based search
	dlpFindDBByOpenHandleReqArgID,					// req id for search given an open db handle 
	dlpFindDBByTypeCreatorReqArgID					// req id for type + creator based iterative search
	};

// Option flags
#define dlpFindDBOptFlagGetAttributes	0x80		// get database attributes -- this is
																// an option to allow find operations to skip
																// returning this data as a performance optimization

#define dlpFindDBOptFlagGetSize			0x40		// get record count and data size also -- this is
																// an option because the operation can take a long
																// time, which we would rather avoid if it is not needed
																
#define dlpFindDBOptFlagGetMaxRecSize	0x20		// get max rec/resource size -- this is
																// an option because the operation can take a long
																// time, which we would rather avoid if it is not needed
																// (dlpFindDBOptFlagGetMaxRecSize is only supported for
																// dlpFindDBByOpenHandleReqArgID)

// Request type for dlpdlpFindDBByNameReqArgID:
typedef struct DlpFindDBByNameReqHdrType {
												// OFFSET
	UInt8					bOptFlags;		// 0;		bitfield of dlpFindDBOptFlag... flags
	UInt8					bCardNo;			// 2;		card number to search
												// TOTAL: 4 bytes										
	} DlpFindDBByNameReqHdrType;

typedef struct DlpFindDBByNameReqType {
												// OFFSET
	DlpFindDBByNameReqHdrType			//
							header;			// 0;		fixed size header
												// FIXED SIZE: 4 bytes
	UInt8					name[2];			// variable size -- zero-terminated database name string
	} DlpFindDBByNameReqType;


// Request type for dlpFindDBByOpenHandleReqArgID:
typedef struct DlpFindDBByOpenHandleReqType {
												// OFFSET
	UInt8					bOptFlags;		// 0;		bitfield of dlpFindDBOptFlag... flags
	UInt8					bDbID;			// 1;		database id returned by dlpOpenDB or dlpCreateDB
												// TOTAL: 2 bytes										
	} DlpFindDBByOpenHandleReqType;


// Request type for dlpFindDBByTypeCreatorReqArgID:
typedef struct DlpFindDBByTypeCreatorReqType {
												// OFFSET
	UInt8					bOptFlags;		// 0;		bitfield of dlpFindDBOptFlag... flags
	UInt8					bSrchFlags;		// 1;		bitfield of dlpFindDBSrchFlag... flags
	UInt32				dwType;			// 2;		db type id (zero = wildcard)
	UInt32				dwCreator;		// 6;		db creator id (zero = wildcard)
												// TOTAL: 10 bytes
	} DlpFindDBByTypeCreatorReqType;

#define dlpFindDBSrchFlagNewSearch		0x80			// set to beging a new search
#define dlpFindDBSrchFlagOnlyLatest		0x40			// set to search for the latest version

	
	
// Response argument structures for dlpFindDBByNameReqArgID, dlpFindDBByOpenHandleReqArgID and
// dlpFindDBByTypeCreatorReqArgID (if found):
//
enum	{
	dlpFindDBBasicRespArgID = dlpFirstArgID,		// resp arg id for basic info
																// (if dlpFindDBOptFlagGetAttributes is set)
	dlpFindDBSizeRespArgID								// resp arg id for size info
																// (if dlpFindDBOptFlagGetSize or dlpFindDBOptFlagGetMaxRecSize
																// are set)
	};


// dlpFindDBBasicRespArgID (returned only if dlpFindDBOptFlagGetAttributes is set):

typedef struct DlpFindDBBasicRespHdrType {
												// OFFSET
	UInt8					bCardNo;			// 0;		card number of database
	UInt8					bReserved;		// 1;		RESERVED -- SET TO NULL
	UInt32				dwLocalID;		// 2;		local id of the database (for internal use)
	UInt32				dwOpenRef;		// 6;		db open ref of the database if it is currently opened
												//			by the caller; zero otherwise (for internal use) can
												//			change after read record list
	DlpDBInfoHdrType	info;				//10;		database info (creator, type, flags, etc.) MUST BE LAST FIELD
												// TOTAL: 54 bytes
	} DlpFindDBBasicRespHdrType;

typedef struct DlpFindDBBasicRespType {
												// OFFSET
	DlpFindDBBasicRespHdrType			//
							header;			// 0;		fixed-size header
												// FIXED SIZE: 54 bytes
	UInt8					name[2];			// variable size -- zero-terminated database name string
	} DlpFindDBBasicRespType;



// dlpFindDBSizeRespArgID (returned only if dlpFindDBOptFlagGetSize or dlpFindDBOptFlagGetMaxRecSize is set):

typedef struct DlpFindDBSizeRespType {
												// OFFSET
	
	// Returned if dlpFindDBOptFlagGetSize is set for all queries:
	// (otherwise, fields are set to zero)
	UInt32				dwNumRecords;	// 0;		record/resource count
	UInt32				dwTotalBytes;	// 4;		total bytes used by db
	UInt32				dwDataBytes;	// 8;		bytes used for data
	
	// Returned if dlpFindDBOptFlagGetSize is set for dlpFindDBByOpenHandleReqArgID only:
	// (otherwise, fields are set to zero)
	UInt32				dwAppBlkSize;	//12;		size of app info block size (for
												//			dlpFindDBByOpenHandleReqArgID only)
	UInt32				dwSortBlkSize;	//16;		size of sort info block size(for
												//			dlpFindDBByOpenHandleReqArgID only)
												//
	// Returned if dlpFindDBOptFlagGetMaxRecSize is set for dlpFindDBByOpenHandleReqArgID only:
	// (otherwise, field is set to zero)
	UInt32				dwMaxRecSize;	//20;		size of largest record or resource in the database (for
												//			dlpFindDBByOpenHandleReqArgID + dlpFindDBOptFlagGetMaxRecSize only)
												//	TOTAL: 24 bytes
	} DlpFindDBSizeRespType;




//////////////////////////////////////////////////////////////////////////
// dlpSetDBInfo
//////////////////////////////////////////////////////////////////////////
//		Request arguments:
//
//		Response arguments:
//
//		Possible error codes
//			dlpRespErrSystem,
//			dlpRespErrParam,
//			dlpRespErrNotFound
//			dlpRespErrNotEnoughSpace
//			dlpRespErrNotSupported
//			dlpRespErrReadOnly
//			dlpRespErrNoneOpen

// Request argument structure:
//

enum	{
	dlpSetDBInfoReqArgID = dlpFirstArgID
	};


typedef struct DlpSetDBInfoReqHdrType {
												// OFFSET
	UInt8					dbID;				// 0;		database id returned by dlpOpenDB or dlpCreateDB
	UInt8					bReserved;		// 1;		RESERVED -- SET TO NULL
	UInt16				wClrDbFlags;	// 2;		flags to clear; allowed DB flags: dlpDBFlagAppInfoDirty,
												//			dlpDBFlagBackup, dlpDBFlagOKToInstallNewer,
												//			dlpDBFlagResetAfterInstall, dlpDBFlagCopyPrevention;
												//			0 = don't change
	UInt16				wSetDbFlags;	// 4;		flags to set; allowed DB flags: dlpDBFlagAppInfoDirty,
												//			dlpDBFlagBackup, dlpDBFlagOKToInstallNewer,
												//			dlpDBFlagResetAfterInstall, dlpDBFlagCopyPrevention;
												//			0 = don't change
	UInt16				wDbVersion;		// 6;		database version; dlpSetDBInfoNoVerChange = don't change
	DlpDateTimeType	crDate;			// 8;		creation date; zero year = don't change
	DlpDateTimeType	modDate;			//16;		modification date; zero year = don't change
	DlpDateTimeType	bckUpDate;		//24;		backup date; zero year = don't change
	UInt32				dwType;			//32;		database type id; zero = don't change
	UInt32				dwCreator;		//36;		database creator id; zero = don't change
												// TOTAL: 40 bytes										
	} DlpSetDBInfoReqHdrType;

#define dlpSetDBInfoNoVerChange		0xFFFF

typedef struct DlpSetDBInfoReqType {
												// OFFSET
	DlpSetDBInfoReqHdrType				//
							header;			// 0;		fixed size header
												// FIXED SIZE: 40 bytes
	UInt8					name[2];			// variable size -- zero-terminated database name string
	} DlpSetDBInfoReqType;


// Response argument structure:
//
//		none.


//////////////////////////////////////////////////////////////////////////
//	dlpExpSlotEnumerate
//////////////////////////////////////////////////////////////////////////
//		Get the used and total size of a volume
//
//		Possible error codes
//			
//			
#pragma mark Exp Slot Enumerate
// Response argument structure:
//
#define dlpExpSlotsEnumerateRespArgID		dlpFirstArgID

typedef struct DlpExpSlotsEnumerateRespHdrType {
												// OFFSET
	UInt16				numSlots;		// 0: Number of slots
										
	} DlpExpSlotsEnumerateRespHdrType;
							
typedef struct DlpExpSlotsEnumerateRespType {
												// OFFSET
	DlpExpSlotsEnumerateRespHdrType
								header;		// 2:    Fixed Header											
												
	UInt16					slots[2]; 	// 0;		Variable size number of volumes
											
	} DlpExpSlotsEnumerateRespType;

typedef DlpExpSlotsEnumerateRespType*		DlpExpSlotsEnumerateRespPtr;


//////////////////////////////////////////////////////////////////////////
//	dlpExpCardPresent
//////////////////////////////////////////////////////////////////////////
//		Get the used and total size of a volume
//
//		Possible error codes
//			
//			
#pragma mark Exp Card Present
// Request argument structure:
//
#define dlpExpCardPresentReqArgID			dlpFirstArgID

typedef struct DlpExpCardPresentReqType {
												// OFFSET
	UInt16				slotRef;		// 0: Slot Reference
										
	} DlpExpCardPresentReqType;

typedef DlpExpCardPresentReqType*	DlpExpCardPresentReqPtr;							


//////////////////////////////////////////////////////////////////////////
//	dlpExpCardInfo
//////////////////////////////////////////////////////////////////////////
//
//		Possible error codes
//		Fill in later.
#pragma mark Exp Card Info

// Request argument structure:
//
#define dlpExpCardInfoReqArgID		dlpFirstArgID

typedef struct DlpExpCardInfoReqType {
												// OFFSET
	UInt16				slotRefNum;		// 0;	Slot Reference Number
												
	} DlpExpCardInfoReqType;

typedef DlpExpCardInfoReqType*	DlpExpCardInfoReqPtr;

// Response argument structure:
//
#define dlpExpCardInfoTypeRespArgID		dlpFirstArgID

typedef struct DlpExpCardInfoRespHdrType {
												   // OFFSET
	UInt32				capabilityFlags;	// 0: Capabilities of the Slot
	UInt16            numStrings;
										
	} DlpExpCardInfoRespHdrType;
							
typedef struct DlpExpCardInfoRespType {
												// OFFSET
	DlpExpCardInfoRespHdrType
								header;		// 2:    Fixed Header											
												
	UInt16					buffer[2]; // 0;		Packed Strings buffer
											
	} DlpExpCardInfoRespType;

typedef DlpExpCardInfoRespType*		DlpExpCardInfoRespPtr;



//////////////////////////////////////////////////////////////////////////
//	dlpVFSFileCustomControl
//////////////////////////////////////////////////////////////////////////
//
//		Possible error codes
//		Fill in later.
#pragma mark VFS File Custom Control

// Request argument structure:
//
#define dlpVFSFileCustomControlReqArgID		dlpFirstArgID

typedef struct DlpVFSFileCustomControlReqHdrType {
												// OFFSET
	UInt32	fsCreator;					// 0;
	UInt32	apiCreator;					// 4;
	UInt16	apiSelector;				// 8;	
	UInt16	bufLen;						// 10;
												// TOTAL: 12 bytes
	} DlpVFSFileCustomControlReqHdrType;

typedef struct DlpVFSFileCustomControlReqType {
												// OFFSET
	DlpVFSFileCustomControlReqHdrType//
							header;			//	0;		fixed-size header
												// FIXED SIZE: 12 bytes;
	UInt16				buffer[2];		// 14;	The path name and the
												// new name are packed together
												// both are zero terminated.
	} DlpVFSFileCustomControlReqType;

typedef DlpVFSFileCustomControlReqType*	DlpVFSFileCustomControlReqPtr;

// Response argument structure:
//
#define dlpVFSFileCustomControlRespArgID		dlpFirstArgID

typedef struct DlpVFSFileCustomControlHdrType {
												// OFFSET
	UInt16				bufLen;			// 0: Buffer length
										
	} DlpVFSFileCustomControlHdrType;
							
typedef struct DlpVFSFileCustomControlRespType {
												// OFFSET
	DlpVFSFileCustomControlHdrType
								header;		// 2:    Fixed Header											
												
	UInt16					buffer[2]; // 0;		Variable size number of volumes
											
	} DlpVFSFileCustomControlRespType;

typedef DlpVFSFileCustomControlRespType*		DlpVFSFileCustomControlRespPtr;


//////////////////////////////////////////////////////////////////////////
//	dlpVFSGetDefaultDirectory
//////////////////////////////////////////////////////////////////////////
//
//		Possible error codes

#pragma mark VFS Get Default Directory

// Request argument structure:
//
#define dlpVFSGetDefaultDirReqArgID		dlpFirstArgID

typedef struct DlpVFSGetDefaultDirReqHdrType {
												// OFFSET
	UInt16				volRefNum;		// 0: Volume reference		
	} DlpVFSGetDefaultDirReqHdrType;

typedef struct DlpVFSGetDefaultDirReqType {
												// OFFSET
	DlpVFSGetDefaultDirReqHdrType
							header;			//	0;		fixed-size header
												// FIXED SIZE: 2 bytes;
	UInt8					fileTypeStr[2];// 2;		File type string -- var size
												//			zero-terminated
	} DlpVFSGetDefaultDirReqType;

typedef DlpVFSGetDefaultDirReqType*		DlpVFSGetDefaultDirReqPtr;

// Response argument structure:
//
#define dlpVFSGetDefaultDirectoryRespArgID		dlpFirstArgID

typedef struct DlpVFSGetDefaultDirRespHdrType {
												// OFFSET
	UInt16				bufSize;		// 0: Buffer Size	
	} DlpVFSGetDefaultDirRespHdrType;

typedef struct DlpVFSGetDefaultDirRespType {
												// OFFSET
	DlpVFSGetDefaultDirRespHdrType	header;											
								
	Char					dirPath[2];		// 0;		directory path -- var size
												//			zero-terminated
	} DlpVFSGetDefaultDirRespType;

typedef DlpVFSGetDefaultDirRespType*		DlpVFSGetDefaultDirRespPtr;


//////////////////////////////////////////////////////////////////////////
//	dlpVFSImportDatabaseFromFile
//////////////////////////////////////////////////////////////////////////
//
//		Possible error codes

#pragma mark VFS Import DB From File

// Request argument structure:
//
#define dlpVFSImportDBFromFileReqArgID		dlpFirstArgID

typedef struct DlpVFSImportDBFromFileReqHdrType {
												// OFFSET
	UInt16				volRefNum;		// 0: Volume reference
										
	} DlpVFSImportDBFromFileReqHdrType;

typedef struct DlpVFSImportDBFromFileReqType {
												// OFFSET
	DlpVFSImportDBFromFileReqHdrType
							header;			//	0;		fixed-size header
												// FIXED SIZE: 2 bytes;
	UInt8					pathName[2];	// 2;		path name -- var size
												//			zero-terminated
	} DlpVFSImportDBFromFileReqType;

typedef DlpVFSImportDBFromFileReqType*		DlpVFSImportDBFromFileReqPtr;

// Response argument structure:
//
#define dlpVFSImportDBFromFileRespArgID		dlpFirstArgID

typedef struct DlpVFSImportDBFromFileRespType {
												// OFFSET
	UInt16	cardNo;				// 0: Card Number
	LocalID	dbID;					// 2: Database ID
	
	} DlpVFSImportDBFromFileRespType;		// TOTAL: 4 bytes


//////////////////////////////////////////////////////////////////////////
//	dlpVFSExportDatabaseToFile
//////////////////////////////////////////////////////////////////////////
//
//		Possible error codes

#pragma mark VFS Export DB To File

// Request argument structure:
//
#define dlpVFSExportDBToFileReqArgID		dlpFirstArgID

typedef struct DlpVFSExportDBToFileReqHdrType {
												// OFFSET
	UInt16				volRefNum;		// 0: Volume reference
	UInt16				cardNo;			// 2: Card Number
	LocalID				dbID;				// 4: Database ID
										
	} DlpVFSExportDBToFileReqHdrType;

typedef struct DlpVFSExportDBToFileFileReqType {
												// OFFSET
	DlpVFSExportDBToFileReqHdrType
							header;			//	0;		fixed-size header
												// FIXED SIZE: 2 bytes;
	UInt8					pathName[2];	// 2;		path name -- var size
												//			zero-terminated
	} DlpVFSExportDBToFileReqType;

typedef DlpVFSExportDBToFileReqType*		DlpVFSExportDBToFileReqPtr;


//////////////////////////////////////////////////////////////////////////
//	dlpVFSFileCreate
//////////////////////////////////////////////////////////////////////////
//
//		Possible error codes
//		Fill in later.
#pragma mark VFS File Create

// Request argument structure:
//
#define dlpVFSFileCreateReqArgID		dlpFirstArgID

typedef struct DlpVFSFileCreateReqHdrType {
												// OFFSET
	UInt16				volRefNum;		// 0;		
												// TOTAL: 2 bytes
	} DlpVFSFileCreateReqHdrType;

typedef struct DlpVFSFileCreateReqType {
												// OFFSET
	DlpVFSFileCreateReqHdrType				//
							header;			//	0;		fixed-size header
												// FIXED SIZE: 12 bytes;
	Char					path[2];			// 14;	abs file path -- var size
												//			zero-terminated
	} DlpVFSFileCreateReqType;
	
typedef DlpVFSFileCreateReqType*	DlpVFSFileCreateReqPtr;		
	
	
//////////////////////////////////////////////////////////////////////////
// VFS File Open
//////////////////////////////////////////////////////////////////////////
//		Request arguments:
//
//		Response arguments:
//
//		Possible error codes
//
//
#pragma mark VFS File Open

// Request argument structure:
//

#define dlpVFSFileOpenReqArgID		dlpFirstArgID

// Open modes
// don't let anyone else open it
#define	dlpOpenFileModeExclusive			(0x0001UL)	
// open for read access	
#define	dlpOpenFileModeRead					(0x0002UL)	
// open for write access, implies exclusive	
#define	dlpOpenFileModeWrite				(0x0004UL | dlpOpenFileModeExclusive)		
// open for read/write access
#define	dlpOpenFileModeReadWrite			(dlpOpenFileModeRead | dlpOpenFileModeWrite)		


typedef struct DlpVFSFileOpenReqHdrType {
												// OFFSET
	UInt16				volRefNum;		// 0:	Volume Reference Number		
	UInt16				openMode;		// 2: Open Mode
												// TOTAL: 4 bytes
	} DlpVFSFileOpenReqHdrType;

typedef struct DlpVFSFileOpenReqType {
												// OFFSET
	DlpVFSFileOpenReqHdrType				//
							header;			//	0;		fixed-size header
												// FIXED SIZE: 12 bytes;
	Char					path[2];			// 14;	abs file path -- var size
												//			zero-terminated
	} DlpVFSFileOpenReqType;
	
typedef DlpVFSFileOpenReqType*	DlpVFSFileOpenReqPtr;	

// Response argument structure:
//
#define dlpVFSFileOpenRespArgID		dlpFirstArgID

typedef struct DlpVFSFileOpenRespType {
												// OFFSET
	FileRef fileRef;						//	0;	File reference	
		
	} DlpVFSFileOpenRespType;		// TOTAL: 4 bytes

	

//////////////////////////////////////////////////////////////////////////
//	dlpLoopBackTest
//////////////////////////////////////////////////////////////////////////
//
//		Possible error codes
//			dlpRespErrSystem,
//			dlpRespErrNotEnoughSpace,
//			dlpRespErrParam,
//			dlpRespErrNoneOpen
#pragma mark Loopback Test

// Request argument structure:
//
#define dlpLoopBackTestReqArgID		dlpFirstArgID

typedef struct DlpLoopBackTestReqHdrType {
												// OFFSET
	UInt32				dataSize;		//	0;		total resource size
												// TOTAL: 4 bytes;
	} DlpLoopBackTestReqHdrType;

typedef struct DlpLoopBackTestReqType {
												// OFFSET
	DlpLoopBackTestReqHdrType			//
							header;			//	0;		fixed-size header
												// FIXED SIZE: 10 bytes;
	UInt8					data[2];			// 10;		data -- var size
	} DlpLoopBackTestReqType;

// Response argument structure:
//
//		
#define dlpLoopBackTestRespArgID		dlpFirstArgID

typedef struct DlpLoopBackTestRespHdrType {
												// OFFSET
	UInt32				dataSize;		//	0;		total resource size in bytes
												// TOTAL: 4 bytes;
	} DlpLoopBackTestRespHdrType;

typedef struct DlpLoopBackTestRespType {
												// OFFSET
	DlpLoopBackTestRespHdrType			//
							header;			//	0;		fixed-size header
												// FIXED SIZE: 6 bytes;
	UInt8					data[2];		// 6;		data -- var size
	} DlpLoopBackTestRespType;


//////////////////////////////////////////////////////////////////////////
//	dlpVFSFileClose
//////////////////////////////////////////////////////////////////////////
//		Get the used and total size of a volume
//
//		Possible error codes
//			
//			
#pragma mark VFS File Close
// Request argument structure:
//
#define dlpVFSFileCloseReqArgID			dlpFirstArgID

typedef struct DlpVFSFileCloseReqType {
										// OFFSET
	FileRef fileRef;				// 0:	File Reference
										// TOTAL: 4 bytes
	} DlpVFSFileCloseReqType;

typedef DlpVFSFileCloseReqType*	DlpVFSFileCloseReqPtr;


//////////////////////////////////////////////////////////////////////////
//	dlpVFSFileWrite
//////////////////////////////////////////////////////////////////////////
//		Set the used and total size of a volume
//
//		Possible error codes
//			
//			
#pragma mark VFS File Write

// Write calback parameters
typedef struct DlpVFSFileWriteCallbackParamType{
										// OFFSET
	FileRef fileRef;				// 0:	File Reference
	UInt32  numBytes;				// 4:	Expected File Size
										// TOTAL: 8 bytes
	} DlpVFSFileWriteCallbackParamType;


// Request argument structure:
//
#define dlpVFSFileWriteReqArgID		dlpFirstArgID

typedef struct DlpVFSFileWriteReqType {
										// OFFSET
	FileRef fileRef;				// 0:	File Reference
	UInt32  numBytes;				// 4:	File Attributes
										// TOTAL: 8 bytes
	} DlpVFSFileWriteReqType;

typedef DlpVFSFileWriteReqType*	DlpVFSFileWriteReqPtr;

// Response argument structure:
//
#define dlpVFSFileWriteRespArgID		dlpFirstArgID

typedef struct DlpVFSFileWriteRespType {
												// OFFSET
	UInt32 numBytesWritten;				//	0;	File attributes	
	} DlpVFSFileWriteRespType;	// TOTAL: 4 bytes


//////////////////////////////////////////////////////////////////////////
//	dlpVFSFileRead
//////////////////////////////////////////////////////////////////////////
//		Set the used and total size of a volume
//
//		Possible error codes
//			
//			
#pragma mark VFS File Read

// Write calback parameters
typedef struct DlpVFSFileReadCallbackParamType{
										// OFFSET
	FileRef fileRef;				// 0:	File Reference
	UInt32  numBytes;				// 4:	Expected File Size
										// TOTAL: 8 bytes
	} DlpVFSFileReadCallbackParamType;

typedef DlpVFSFileReadCallbackParamType* DlpVFSFileReadCallbackParamPtr;

// Request argument structure:
//
#define dlpVFSFileReadReqArgID		dlpFirstArgID

typedef struct DlpVFSFileReadReqType {
										// OFFSET
	FileRef fileRef;				// 0:	File Reference
	UInt32  numBytes;				// 4:	File Attributes
										// TOTAL: 8 bytes
	} DlpVFSFileReadReqType;

typedef DlpVFSFileReadReqType*	DlpVFSFileReadReqPtr;

// Response argument structure:
//
#define dlpVFSFileReadRespArgID		dlpFirstArgID

typedef struct DlpVFSFileReadRespType {
												// OFFSET
	UInt32 numBytes;				//	0;		
	} DlpVFSFileReadRespType;	// TOTAL: 4 bytes


//////////////////////////////////////////////////////////////////////////
//	dlpVFSFileDelete
//////////////////////////////////////////////////////////////////////////
//
//		Possible error codes

#pragma mark VFS File Delete


// Request argument structure:
//
#define dlpVFSFileDeleteReqArgID		dlpFirstArgID

typedef struct DlpVFSFileDeleteReqHdrType {
												// OFFSET
	UInt16				volRefNum;		// 0: Volume reference
										
	} DlpVFSFileDeleteReqHdrType;

typedef struct DlpVFSFileDeleteReqType {
												// OFFSET
	DlpVFSFileDeleteReqHdrType
							header;			//	0;		fixed-size header
												// FIXED SIZE: 2 bytes;
	UInt8					pathNameP[2];		// 2;		Directory Name -- var size
												//			zero-terminated
	} DlpVFSFileDeleteReqType;

typedef DlpVFSFileDeleteReqType*		DlpVFSFileDeleteReqPtr;


//////////////////////////////////////////////////////////////////////////
//	dlpVFSFileRename
//////////////////////////////////////////////////////////////////////////
//
//		Possible error codes
//		Fill in later.
#pragma mark VFS File Rename

// Request argument structure:
//
#define dlpVFSFileRenameReqArgID		dlpFirstArgID

typedef struct DlpVFSFileRenameReqHdrType {
												// OFFSET
	UInt16	volRefNum;					// 0;
	UInt16	numStrings;				   // 2;
												// TOTAL: 4 bytes
	} DlpVFSFileRenameReqHdrType;

typedef struct DlpVFSFileRenameReqType {
												// OFFSET
	DlpVFSFileRenameReqHdrType				//
							header;			//	0;		fixed-size header
												// FIXED SIZE: 12 bytes;
	Char					packedNames[2];// 14;	The path name and the
												// new name are packed together
												// both are zero terminated.
	} DlpVFSFileRenameReqType;

typedef DlpVFSFileRenameReqType*		DlpVFSFileRenameReqPtr;
	

//////////////////////////////////////////////////////////////////////////
//	dlpVFSFileEOF
//////////////////////////////////////////////////////////////////////////
//		Get the used and total size of a volume
//
//		Possible error codes
//			
//			
#pragma mark VFS File EOF
// Request argument structure:
//
#define dlpVFSFileEOFReqArgID			dlpFirstArgID

typedef struct DlpVFSFileEOFReqType {
										// OFFSET
	FileRef fileRef;				// 0: File Reference
										// TOTAL: 4 bytes
	} DlpVFSFileEOFReqType;

typedef DlpVFSFileEOFReqType*	DlpVFSFileEOFReqPtr;

#define dlpVFSFileEOFRespArgID			dlpFirstArgID


//////////////////////////////////////////////////////////////////////////
//	dlpVFSFileTell
//////////////////////////////////////////////////////////////////////////
//		Get the used and total size of a volume
//
//		Possible error codes
//			
//			
#pragma mark VFS File Tell
// Request argument structure:
//
#define dlpVFSFileTellReqArgID			dlpFirstArgID

typedef struct DlpVFSFileTellReqType {
										// OFFSET
	FileRef fileRef;				// 0:	File reference number
										// TOTAL: 2 bytes
	} DlpVFSFileTellReqType;

typedef DlpVFSFileTellReqType*	DlpVFSFileTellReqPtr;

// Response argument structure:
//
#define dlpVFSFileTellRespArgID		dlpFirstArgID

typedef struct DlpVFSFileTellRespType {
												// OFFSET
	UInt32 filePosition;					//	0;	Position in the file	
	} DlpVFSFileTellRespType;		// TOTAL: 4 bytes


//////////////////////////////////////////////////////////////////////////
//	dlpVFSFileResize
//////////////////////////////////////////////////////////////////////////
//		Set the used and total size of a volume
//
//		Possible error codes
//			
//			
#pragma mark VFS File Resize
// Request argument structure:
//
#define dlpVFSFileResizeReqArgID		dlpFirstArgID

typedef struct DlpVFSFileResizeReqType {
										// OFFSET
	FileRef fileRef;				// 0:	File Reference
	UInt32  newSize;				// 4: NewSize
										// TOTAL: 8 bytes
	} DlpVFSFileResizeReqType;

typedef DlpVFSFileResizeReqType*	DlpVFSFileResizeReqPtr;


//////////////////////////////////////////////////////////////////////////
//	dlpVFSFileGetAttributes
//////////////////////////////////////////////////////////////////////////
//		Get the used and total size of a volume
//
//		Possible error codes
//			
//			
#pragma mark VFS File Get Attributes
// Request argument structure:
//
#define dlpVFSGetAttributesReqArgID		dlpFirstArgID

typedef struct DlpVFSGetAttributesReqType {
										// OFFSET
	FileRef fileRef;				// 0:	Volume reference number
										// TOTAL: 4 bytes
	} DlpVFSGetAttributesReqType;

typedef DlpVFSGetAttributesReqType*	DlpVFSGetAttributesReqPtr;

// Response argument structure:
//
#define dlpVFSGetAttributesRespArgID		dlpFirstArgID

typedef struct DlpVFSGetAttributesRespType {
												// OFFSET
	UInt32 attributes;					//	0;	File attributes	
	} DlpVFSGetAttributesRespType;			// TOTAL: 4 bytes


//////////////////////////////////////////////////////////////////////////
//	dlpVFSFileSetAttributes
//////////////////////////////////////////////////////////////////////////
//		Set the used and total size of a volume
//
//		Possible error codes
//			
//			
#pragma mark VFS File Set Attributes
// Request argument structure:
//
#define dlpVFSSetAttributesReqArgID		dlpFirstArgID

typedef struct DlpVFSSetAttributesReqType {
										// OFFSET
	FileRef fileRef;				// 0:	File Reference
	UInt32 attributes;			// 4:	File Attributes
										// TOTAL: 8 bytes
	} DlpVFSSetAttributesReqType;

typedef DlpVFSSetAttributesReqType*	DlpVFSSetAttributesReqPtr;


//////////////////////////////////////////////////////////////////////////
//	dlpVFSFileGetDate
//////////////////////////////////////////////////////////////////////////
//		Get the used and total size of a volume
//
//		Possible error codes
//			
//			
#pragma mark VFS File Get Dates
// Request argument structure:
//
#define dlpVFSGetDatesReqArgID		dlpFirstArgID

typedef struct DlpVFSGetDatesReqType {
										// OFFSET
	FileRef fileRef;				// 0:	Volume reference number
	UInt16 whichDate;				// 4: Which date parameter
										// TOTAL: 6 bytes
	} DlpVFSGetDatesReqType;

typedef DlpVFSGetDatesReqType*	DlpVFSGetDatesReqPtr;

// Response argument structure:
//
#define dlpVFSGetDatesRespArgID		dlpFirstArgID

typedef struct DlpVFSGetDatesRespType {
												// OFFSET
	UInt32 date;							//	0;	The date requested	
	} DlpVFSGetDatesRespType;			// TOTAL: 4 bytes


//////////////////////////////////////////////////////////////////////////
//	dlpVFSFileSetDate
//////////////////////////////////////////////////////////////////////////
//		Set the used and total size of a volume
//
//		Possible error codes
//			
//			
#pragma mark VFS File Set Dates
// Request argument structure:
//
#define dlpVFSSetDatesReqArgID		dlpFirstArgID

typedef struct DlpVFSSetDatesReqType {
										// OFFSET
	FileRef fileRef;				// 0:	File Reference
	UInt16 whichDate;				// 4: Which date parameter
	UInt32 date;					// 6: The date
										// TOTAL: 10 bytes
	} DlpVFSSetDatesReqType;

typedef DlpVFSSetDatesReqType*	DlpVFSSetDatesReqPtr;


//////////////////////////////////////////////////////////////////////////
//	dlpVFSFileGetSize
//////////////////////////////////////////////////////////////////////////
//		Get the used and total size of a volume
//
//		Possible error codes
//			
//			
#pragma mark VFS File Size
// Request argument structure:
//
#define dlpVFSGetFileSizeReqArgID		dlpFirstArgID

typedef struct DlpVFSGetFileSizeReqType {
										// OFFSET
	FileRef fileRef;				// 0:	File Reference
										// TOTAL: 4 bytes
	} DlpVFSGetFileSizeReqType;

typedef DlpVFSGetFileSizeReqType*	DlpVFSGetFileSizeReqPtr;

// Response argument structure:
//
#define dlpVFSGetFileSizeRespArgID		dlpFirstArgID

typedef struct DlpVFSGetFileSizeRespType {
												// OFFSET
	UInt32 fileSize;						//	0;	Amount of the volume already used.	
	} DlpVFSGetFileSizeRespType;		// TOTAL: 4 bytes


//////////////////////////////////////////////////////////////////////////
//	dlpVFSDirCreate
//////////////////////////////////////////////////////////////////////////
//
//		Possible error codes

#pragma mark VFS Directory Create


// Request argument structure:
//
#define dlpVFSDirCreateReqArgID		dlpFirstArgID

typedef struct DlpVFSDirCreateReqHdrType {
												// OFFSET
	UInt16				volRefNum;		// 0: Volume reference
										
	} DlpVFSDirCreateReqHdrType;

typedef struct DlpVFSDirCreateReqType {
												// OFFSET
	DlpVFSDirCreateReqHdrType
							header;			//	0;		fixed-size header
												// FIXED SIZE: 2 bytes;
	UInt8					dirNameP[2];		// 2;		Directory Name -- var size
												//			zero-terminated
	} DlpVFSDirCreateReqType;

typedef DlpVFSDirCreateReqType*		DlpVFSDirCreateReqPtr;


//////////////////////////////////////////////////////////////////////////
//	dlpVFSDirEntryEnumerate
//////////////////////////////////////////////////////////////////////////
//
//		Possible error codes

// ADH, This needs to be reconciled with the desktop version.

// NEED To have some state information

#pragma mark VFS Directory Enumerate

// Request argument structure:
//
#define dlpVFSDirEntryEnumrerateReqArgID			dlpFirstArgID

typedef struct DlpVFSDirEntryEnumrerateReqType {
										// OFFSET
	FileRef dirRefNum;			// 0: Directory Reference number
	UInt32  dirEntryIterator;  // 4: Directory Iterator
	UInt32	bufferSize;			// 8: max return buffer
										// TOTAL: 4 bytes
	} DlpVFSDirEntryEnumrerateReqType;

typedef DlpVFSDirEntryEnumrerateReqType*	DlpVFSDirEntryEnumrerateReqPtr;


// Response argument structure:
//
#define dlpVFSDirEntryEnumerateRespArgID		dlpFirstArgID

typedef struct DlpVFSDirEntryEnumerateHdrType {
												// OFFSET
	UInt32            dirEntryIterator; // 0: Directory iterator
	UInt32				numEntries;		   // 4: Number of directory entries
										
	} DlpVFSDirEntryEnumerateHdrType;
							
typedef struct DlpVFSDirEntryType {
	UInt32	attributes;             // Directory attributes
	Char		name[2];						// Zero terminated string. Strings that have an even length
	                                 // will be null terminated and have a pad byte.
	} DlpVFSDirEntryType;							
							
typedef struct DlpVFSDirEntryEnumerateRespType {
												// OFFSET
	DlpVFSDirEntryEnumerateHdrType
								header;		// 2:    Fixed Header											
												
	DlpVFSDirEntryType	entries[2]; // 0;		Variable size number of volumes
											
	} DlpVFSDirEntryEnumerateRespType;

typedef DlpVFSDirEntryEnumerateRespType*		DlpVFSDirEntryEnumerateRespPtr;



//////////////////////////////////////////////////////////////////////////
//	dlpVFSVolumeFormat
//////////////////////////////////////////////////////////////////////////
//		Get the used and total size of a volume
//
//		Possible error codes
//			
//			
#pragma mark VFS Volume Format
// Request argument structure:
//
#define dlpVFSVolumeFormatReqArgID			dlpFirstArgID

typedef struct DlpVFSVolumeFormatHdrType {
												// OFFSET
	UInt16				fsLibRefNum; // 0: File System Library Ref Num				
	UInt16				vfsMountParamLen;
	UInt8				flags;
	UInt8				unused;
   										
	} DlpVFSVolumeFormatHdrType;
							
typedef struct DlpVFSVolumeFormatReqType {
												// OFFSET
	DlpVFSVolumeFormatHdrType
								header;		// 2:    Fixed Header											
												
	UInt8			mountParameters[2];  // 0;		Mount parameters
											
	} DlpVFSVolumeFormatReqType;

typedef DlpVFSVolumeFormatReqType*		DlpVFSVolumeFormatReqPtr;

//////////////////////////////////////////////////////////////////////////
//	dlpVFSVolumeEnumerate
//////////////////////////////////////////////////////////////////////////
//
//		Possible error codes

#pragma mark VFS Volume Enumerate

// Response argument structure:
//
#define dlpVFSVolumeEnumerateRespArgID		dlpFirstArgID

typedef struct DlpVFSVolumeEnumerateHdrType {
												// OFFSET
	UInt16				numVolumes;		// 0: Number of volumes
										
	} DlpVFSVolumeEnumerateHdrType;
							
typedef struct DlpVFSVolumeEnumerateRespType {
												// OFFSET
	DlpVFSVolumeEnumerateHdrType
								header;		// 2:    Fixed Header											
												
	UInt16					volumes[2]; // 0;		Variable size number of volumes
											
	} DlpVFSVolumeEnumerateRespType;

typedef DlpVFSVolumeEnumerateRespType*		DlpVFSVolumeEnumerateRespPtr;



//////////////////////////////////////////////////////////////////////////
//	dlpVFSVolumeInfo
//////////////////////////////////////////////////////////////////////////
//		Get the used and total size of a volume
//
//		Possible error codes
//			
//			
#pragma mark VFS Volume Info
// Request argument structure:
//
#define dlpVFSVolumeInfoReqArgID			dlpFirstArgID

typedef struct DlpVFSVolumeInfoReqType {
										// OFFSET
	UInt16 volRefNum;				// 0:	Volume reference number
										// TOTAL: 2 bytes
	} DlpVFSVolumeInfoReqType;

typedef DlpVFSVolumeInfoReqType*	DlpVFSVolumeInfoReqPtr;

// Response argument structure:
//
#define dlpVFSVolumeInfoRespArgID		dlpFirstArgID

typedef struct DlpVFSVolumeInfoRespType {
												// OFFSET
	UInt32	attributes;			// 0:  read-only etc.
	UInt32	fsType;				// 4: Filesystem type for this volume (defined below)
	UInt32	fsCreator;			// 8: Creator code of filesystem driver for this volume.
										//    For use with VFSCustomControl().
	UInt32	mountClass;			// 12: mount class that mounted this volume
	// For slot based filesystems: (mountClass = VFSMountClass_SlotDriver)
	
	UInt16	slotLibRefNum;		// 16: Library on which the volume is mounted
	UInt16	slotRefNum;			// 18: ExpMgr slot number of card containing volume
	UInt32	mediaType;			// 20: Type of card media (mediaMemoryStick, mediaCompactFlash, etc...)
	UInt32	reserved;			// 24: reserved for future use (other mountclasses may need more space)	
	} DlpVFSVolumeInfoRespType;		// TOTAL: 28 bytes


//////////////////////////////////////////////////////////////////////////
//	dlpVFSVolumeGetLabel
//////////////////////////////////////////////////////////////////////////
//
//		Possible error codes

#pragma mark VFS Volume Get Label


// Request argument structure:
//
#define dlpVFSVolumeGetLabelReqArgID		dlpFirstArgID

typedef struct DlpVFSVolumeGetLabelReqType {
												// OFFSET
	UInt16				volRefNum;		// 0: Volume reference									
	} DlpVFSVolumeGetLabelReqType;	// TOTAL: 2 bytes

typedef DlpVFSVolumeGetLabelReqType*		DlpVFSVolumeGetLabelReqPtr;

// Response argument structure:
//
#define dlpVFSVolumeGetLabelRespArgID		dlpFirstArgID

typedef struct DlpVFSVolumeGetLabelRespType {
												// OFFSET
	UInt8					label[2];		// 0;		label name text -- var size
												//			zero-terminated
	} DlpVFSVolumeGetLabelRespType;



//////////////////////////////////////////////////////////////////////////
//	dlpVFSVolumeSetLabel
//////////////////////////////////////////////////////////////////////////
//
//		Possible error codes

#pragma mark VFS Volume Set Label


// Request argument structure:
//
#define dlpVFSVolumeSetLabelReqArgID		dlpFirstArgID

typedef struct DlpVFSVolumeSetLabelReqHdrType {
												// OFFSET
	UInt16				volRefNum;		// 0: Volume reference
										
	} DlpVFSVolumeSetLabelReqHdrType;

typedef struct DlpVFSVolumeSetLabelReqType {
												// OFFSET
	DlpVFSVolumeSetLabelReqHdrType
							header;			//	0;		fixed-size header
												// FIXED SIZE: 2 bytes;
	UInt8					label[2];		// 2;		label name text -- var size
												//			zero-terminated
	} DlpVFSVolumeSetLabelReqType;

typedef DlpVFSVolumeSetLabelReqType*		DlpVFSVolumeSetLabelReqPtr;


//////////////////////////////////////////////////////////////////////////
//	dlpVFSVolumeSize
//////////////////////////////////////////////////////////////////////////
//		Get the used and total size of a volume
//
//		Possible error codes
//			
//			
#pragma mark VFS Volume Size
// Request argument structure:
//
#define dlpVFSVolumeSizeReqArgID			dlpFirstArgID

typedef struct DlpVFSVolumeSizeReqType {
										// OFFSET
	UInt16 volRefNum;				// 0:	Volume reference number
										// TOTAL: 2 bytes
	} DlpVFSVolumeSizeReqType;

typedef DlpVFSVolumeSizeReqType*	DlpVFSVolumeSizeReqPtr;

// Response argument structure:
//
#define dlpVFSVolumeSizeRespArgID		dlpFirstArgID

typedef struct DlpVFSVolumeSizeRespType {
												// OFFSET
	UInt32 volumeSizeUsed;				//	0;	Amount of the volume already used.
	UInt32 volumeSizeTotal;				// 4; Total size of the volume		
	} DlpVFSVolumeSizeRespType;		// TOTAL: 8 bytes


//////////////////////////////////////////////////////////////////////////
//	dlpVFSFileSeek
//////////////////////////////////////////////////////////////////////////
//		Set the used and total size of a volume
//
//		Possible error codes
//			
//			
#pragma mark VFS File Seek
// Request argument structure:
//
#define dlpVFSFileSeekReqArgID		dlpFirstArgID

typedef struct DlpVFSFileSeekReqType {
										// OFFSET
	FileRef fileRef;				// 0:	File Reference
	FileOrigin origin;			// 4: File origin
	Int32 offset;					// 6: offset
										// TOTAL: 10 bytes
	} DlpVFSFileSeekReqType;

typedef DlpVFSFileSeekReqType*	DlpVFSFileSeekReqPtr;


//////////////////////////////////////////////////////////////////////////
//	dlpExpSlotMediaType
//////////////////////////////////////////////////////////////////////////
//		Get the used and total size of a volume
//
//		Possible error codes
//			
//			
#pragma mark Slot Media Type
// Request argument structure:
//
#define dlpExpSlotMediaTypeReqArgID		dlpFirstArgID

typedef struct DlpExpSlotMediaTypeReqType {
										// OFFSET
	UInt16 slotNum;				// 0: Which slot to query
										// TOTAL: 2 bytes
	} DlpExpSlotMediaTypeReqType;

typedef DlpExpSlotMediaTypeReqType*	DlpExpSlotMediaTypeReqPtr;

// Response argument structure:
//
#define dlpExpSlotMediaTypeRespArgID		dlpFirstArgID

typedef struct DlpExpSlotMediaTypeRespType {
												// OFFSET
	UInt32 mediaType;						//	0;	The Media Type	
	} DlpExpSlotMediaTypeRespType;	// TOTAL: 4 bytes




/************************************************************
 * Macros
 *************************************************************/


#endif // __DLCOMMON_H__
