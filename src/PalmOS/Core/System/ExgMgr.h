/******************************************************************************
 *
 * Copyright (c) 1997-2003 PalmSource, Inc. All rights reserved.
 *
 * File: ExgMgr.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *		Include file for Exg system functions
 *
 *****************************************************************************/

#ifndef __EXGMGR_H__
#define __EXGMGR_H__

#include <PalmTypes.h>
#include <CoreTraps.h>

#include <ErrorBase.h>
#include <DataMgr.h>
#include <Rect.h>

#define exgMemError	 			(exgErrorClass | 1)
#define exgErrStackInit 		(exgErrorClass | 2)  // stack could not initialize
#define exgErrUserCancel 		(exgErrorClass | 3)
#define exgErrNoReceiver 		(exgErrorClass | 4)	// receiver device not found
#define exgErrNoKnownTarget		(exgErrorClass | 5)	// can't find a target app
#define exgErrTargetMissing		(exgErrorClass | 6)  // target app is known but missing
#define exgErrNotAllowed		(exgErrorClass | 7)  // operation not allowed
#define exgErrBadData			(exgErrorClass | 8)  // internal data was not valid
#define exgErrAppError			(exgErrorClass | 9)  // generic application error
#define exgErrUnknown			(exgErrorClass | 10) // unknown general error
#define exgErrDeviceFull		(exgErrorClass | 11) // device is full
#define exgErrDisconnected		(exgErrorClass | 12) // link disconnected
#define exgErrNotFound			(exgErrorClass | 13) // requested object not found
#define exgErrBadParam			(exgErrorClass | 14) // bad parameter to call
#define exgErrNotSupported		(exgErrorClass | 15) // operation not supported by this library
#define exgErrDeviceBusy		(exgErrorClass | 16) // device is busy
#define exgErrBadLibrary		(exgErrorClass | 17) // bad or missing ExgLibrary
#define exgErrNotEnoughPower	(exgErrorClass | 18) // Device has not enough power to perform the requested operation

#define exgSeparatorChar		'\t'				// char used to separate multiple registry entries

#define exgRegCreatorID			0xfffb				// creator ID registry
#define exgRegSchemeID			0xfffc				// URL scheme registry
#define exgRegExtensionID		0xfffd				// filename extension registry
#define exgRegTypeID			0xfffe				// MIME type registry

#define exgDataPrefVersion		0					// all isDefault bits clear
#define exgTitleBufferSize		20					// buffer size for title from exgLibCtlGetTitle, including null terminator
#define exgMaxTitleLen			exgTitleBufferSize	// deprecated
#define exgMaxTypeLength		80					// max length of extensions, MIME types, and URL schemes in registry, excluding null terminator
#define exgMaxDescriptionLength	80					// max length of descriptions in registry, excluding null terminator

#define exgLibCtlGetTitle		1					// get title for Exg dialogs
#define exgLibCtlGetVersion		2					// get version of exg lib API
#define exgLibCtlGetPreview		3					// find out if library supports preview
#define exgLibCtlGetURL			4					// get current URL
#define exgLibCtlSpecificOp		0x8000				// start of range for library specific control codes

#define exgLibAPIVersion		0					// current version of exg lib API

// Pre-defined URL schemes
#define exgBeamScheme			"_beam"		// general scheme for Beam commands
#define exgSendScheme			"_send"		// general scheme for Send commands
#define exgGetScheme			"_get"      // special scheme for supporting get

// Pre-defined URL prefixes
#define exgBeamPrefix			exgBeamScheme ":"
#define exgSendPrefix			"?" exgSendScheme ":"
#define exgSendBeamPrefix		"?" exgSendScheme ";" exgBeamScheme ":"
#define exgLocalPrefix			exgLocalScheme ":"
#define exgGetPrefix			exgGetScheme ":"

// A flag used for attachments.
#define exgUnwrap				0x0001

// A flag used for skipping the sysAppLaunchCmdExgAskUser sublaunch and the subsequent
// call to ExgDoDialog.
#define exgNoAsk				0x0002
#define exgGet					0x0004

// Enum for preview operations. Also used as masks for the query operation.
#define exgPreviewQuery			((UInt16)0x0000)
#define exgPreviewShortString	((UInt16)0x0001)
#define exgPreviewLongString	((UInt16)0x0002)
#define exgPreviewDraw			((UInt16)0x0004)
#define exgPreviewDialog		((UInt16)0x0008)
#define exgPreviewFirstUser		((UInt16)0x0400)		// used for app-specific operations
#define exgPreviewLastUser		((UInt16)0x8000)		//


typedef struct {
	UInt16				dbCardNo;			// card number of the database	
	LocalID				dbID;				// LocalID of the database
	UInt16 				recordNum;			// index of record that contain a match
	UInt32				uniqueID;			// postion in record of the match.
	UInt32				matchCustom;		// application specific info
} ExgGoToType;	

typedef ExgGoToType *ExgGoToPtr;


typedef struct ExgSocketType {
	UInt16	libraryRef;	// identifies the Exg library in use
	UInt32 	socketRef;	// used by Exg library to identify this connection
	UInt32 	target;		// Creator ID of application this is sent to
	UInt32	count;		// # of objects in this connection (usually 1)
	UInt32	length;		// # total byte count for all objects being sent (optional)
	UInt32	time;		// last modified time of object (optional)
	UInt32	appData;	// application specific info
	UInt32 	goToCreator; // creator ID of app to launch with goto after receive
	ExgGoToType goToParams;	// If launchCreator then this contains goto find info
	UInt16	localMode:1; // Exchange with local machine only mode 
	UInt16	packetMode:1;// Use connectionless packet mode (Ultra)
	UInt16	noGoTo:1; 	// Do not go to app (local mode only)
	UInt16 	noStatus:1; // Do not display status dialogs
	UInt16 	preview:1;	// Preview in progress: don't throw away data as it's read
	UInt16	reserved:11;// reserved system flags
	Char *description;	// text description of object (for user)
	Char *type;		// Mime type of object (optional)
	Char *name;		// name of object, generally a file name (optional)
} ExgSocketType;
typedef ExgSocketType *ExgSocketPtr;


// structures used for sysAppLaunchCmdExgAskUser launch code parameter
// default is exgAskDialog (ask user with dialog...
typedef enum { exgAskDialog,exgAskOk,exgAskCancel } ExgAskResultType;

typedef struct {
	ExgSocketType *socketP;
	ExgAskResultType result;			// what to do with dialog	
	UInt8 reserved;
} ExgAskParamType;	
typedef ExgAskParamType *ExgAskParamPtr;

// Optional parameter structure used with ExgDoDialog for category control
typedef struct {
	UInt16			version;		// version of this structure (should be zero)
	DmOpenRef		db;				// open database ref (for category information)
	UInt16			categoryIndex;	// index of selected category
} ExgDialogInfoType;

typedef struct {
	UInt16 version;
	ExgSocketType *socketP;
	UInt16 op;
	Char *string;
	UInt32 size;
	RectangleType bounds;
	UInt16 types;
	Err error;
} ExgPreviewInfoType;

// used with exgLibCtlGetURL
typedef struct ExgCtlGetURLType {
	ExgSocketType	*socketP;
	Char			*URLP;
	UInt16		URLSize;
} ExgCtlGetURLType;

typedef Err	(*ExgDBReadProcPtr) 
				(void *dataP, UInt32 *sizeP, void *userDataP);

typedef Boolean	(*ExgDBDeleteProcPtr)
				(const char *nameP, UInt16 version, UInt16 cardNo,
				LocalID dbID, void *userDataP);

typedef Err	(*ExgDBWriteProcPtr)
				(const void *dataP, UInt32 *sizeP, void *userDataP);

#ifdef __cplusplus
extern "C" {
#endif

Err ExgInit(void)  
		SYS_TRAP(sysTrapExgInit);

Err ExgConnect(ExgSocketType *socketP)
		SYS_TRAP(sysTrapExgConnect);

Err ExgPut(ExgSocketType *socketP)
		SYS_TRAP(sysTrapExgPut);

Err ExgGet(ExgSocketType *socketP)
		SYS_TRAP(sysTrapExgGet);

Err ExgAccept(ExgSocketType *socketP)
		SYS_TRAP(sysTrapExgAccept);

Err ExgDisconnect(ExgSocketType *socketP, Err error)
		SYS_TRAP(sysTrapExgDisconnect);

UInt32 ExgSend(ExgSocketType *socketP, const void *bufP, UInt32 bufLen, Err *err)
		SYS_TRAP(sysTrapExgSend);

UInt32 ExgReceive(ExgSocketType *socketP, void *bufP, UInt32 bufLen, Err *err)
		SYS_TRAP(sysTrapExgReceive);

Err ExgControl(ExgSocketType *socketP, UInt16 op, void *valueP, UInt16 *valueLenP)
		SYS_TRAP(sysTrapExgControl);

Err ExgRegisterData(UInt32 creatorID, UInt16 id, const Char *dataTypesP)
		SYS_TRAP(sysTrapExgRegisterData);

Err ExgRegisterDatatype(UInt32 creatorID, UInt16 id, const Char *dataTypesP,
	const Char *descriptionsP, UInt16 flags)
		SYS_TRAP(sysTrapExgRegisterDatatype);

// This function was documented as System Use Only in 3.5, so no third party
// code should have been calling it.  So the addition of "V35" to the name
// should not affect anyone.
Err ExgNotifyReceiveV35(ExgSocketType *socketP)
		SYS_TRAP(sysTrapExgNotifyReceiveV35);

Err ExgNotifyReceive(ExgSocketType *socketP, UInt16 flags)
		SYS_TRAP(sysTrapExgNotifyReceive);

Err ExgNotifyGoto(ExgSocketType *socketP, UInt16 flags)
		SYS_TRAP(sysTrapExgNotifyGoto);

Err	ExgDBRead(
		ExgDBReadProcPtr		readProcP,
		ExgDBDeleteProcPtr		deleteProcP,
		void*					userDataP,
		LocalID*				dbIDP,
		UInt16					cardNo,
		Boolean*				needResetP,
		Boolean					keepDates)
		SYS_TRAP(sysTrapExgDBRead);

Err	ExgDBWrite(
		ExgDBWriteProcPtr		writeProcP,
		void*					userDataP,
		const char*				nameP,
		LocalID					dbID,
		UInt16					cardNo)
		SYS_TRAP(sysTrapExgDBWrite);


Boolean ExgDoDialog(ExgSocketType *socketP, ExgDialogInfoType *infoP, Err *errP)
		SYS_TRAP(sysTrapExgDoDialog);

Err ExgRequest(ExgSocketType *socketP)
		SYS_TRAP(sysTrapExgRequest);

Err ExgSetDefaultApplication(UInt32 creatorID, UInt16 id, const Char *dataTypeP)
		SYS_TRAP(sysTrapExgSetDefaultApplication);

Err ExgGetDefaultApplication(UInt32 *creatorIDP, UInt16 id, const Char *dataTypeP)
		SYS_TRAP(sysTrapExgGetDefaultApplication);

Err ExgGetTargetApplication(ExgSocketType *socketP, Boolean unwrap, UInt32 *creatorIDP, Char *descriptionP, UInt32 descriptionSize)
		SYS_TRAP(sysTrapExgGetTargetApplication);

Err ExgGetRegisteredApplications(UInt32 **creatorIDsP, UInt32 *numAppsP, Char **namesP, Char **descriptionsP, UInt16 id, const Char *dataTypeP)
		SYS_TRAP(sysTrapExgGetRegisteredApplications);

Err ExgGetRegisteredTypes(Char **dataTypesP, UInt32 *sizeP, UInt16 id)
		SYS_TRAP(sysTrapExgGetRegisteredTypes);

Err ExgNotifyPreview(ExgPreviewInfoType *infoP)
		SYS_TRAP(sysTrapExgNotifyPreview);

#ifdef __cplusplus 
}
#endif

#endif  // __EXGMGR_H__
