/******************************************************************************
 *
 * Copyright (c) 1998-2003 PalmSource, Inc. All rights reserved.
 *
 * File: HostControl.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *	This file is part of the Palm OS Emulator.
 *
 *****************************************************************************/

#ifndef _HOSTCONTROL_H_
#define _HOSTCONTROL_H_

#include <PalmTypes.h>
#include <CoreTraps.h>

#ifdef __cplusplus
extern "C" {
#endif


/*
	Set the values for selectors. Note that these values MUST be
	two bytes long and have the high byte be non-zero. The reason
	for this has to do with the way SysGremlins was originally
	declared. It took a GremlinsSelector enumerated value. Originally,
	there was only one value, and it was zero. The way the 68K compiler
	works, it decides that GremlinsSelectors are only one byte long,
	so a call to SysGremlins would push one byte onto the stack. Because
	all values on the stack need to be word-aligned, the processor
	subtracts 1 from the stack before pushing on the byte. Therefore,
	the stack looks like:

			previous contents
			garbage byte
			selector
			return address

	When the two middle bytes are read together as a word, they appear
	as: <selector> * 256 + <garbage byte>.

	With this setup, we have two choices: leave the selector size at
	one byte and limit ourselves to 256 functions, or define the selector
	to be a two byte value, with the first 256 values (all those with 0x00
	in the upper byte) to be GremlinIsOn. The latter sounds preferable, so
	we start the new selectors at 0x0100.
*/


	// Host information selectors

#define hostSelectorGetHostVersion			0x0100
#define hostSelectorGetHostID				0x0101
#define hostSelectorGetHostPlatform			0x0102
#define hostSelectorIsSelectorImplemented	0x0103
#define hostSelectorGestalt					0x0104
#define hostSelectorIsCallingTrap			0x0105


	// Profiler selectors

#define hostSelectorProfileInit				0x0200
#define hostSelectorProfileStart			0x0201
#define hostSelectorProfileStop				0x0202
#define hostSelectorProfileDump				0x0203
#define hostSelectorProfileCleanup			0x0204
#define hostSelectorProfileDetailFn			0x0205
#define hostSelectorProfileGetCycles		0x0206


	// Std C Library wrapper selectors

#define hostSelectorErrNo					0x0300

#define hostSelectorFClose					0x0301
#define hostSelectorFEOF					0x0302
#define hostSelectorFError					0x0303
#define hostSelectorFFlush					0x0304
#define hostSelectorFGetC					0x0305
#define hostSelectorFGetPos					0x0306
#define hostSelectorFGetS					0x0307
#define hostSelectorFOpen					0x0308
#define hostSelectorFPrintF					0x0309		/* Floating point not yet supported in Poser */
#define hostSelectorFPutC					0x030A
#define hostSelectorFPutS					0x030B
#define hostSelectorFRead					0x030C
#define hostSelectorRemove					0x030D
#define hostSelectorRename					0x030E
#define hostSelectorFReopen					0x030F		/* Not yet implemented in Poser */
#define hostSelectorFScanF					0x0310		/* Not yet implemented */
#define hostSelectorFSeek					0x0311
#define hostSelectorFSetPos					0x0312
#define hostSelectorFTell					0x0313
#define hostSelectorFWrite					0x0314
#define hostSelectorTmpFile					0x0315
#define hostSelectorTmpNam					0x0316

#define hostSelectorGetEnv					0x0317

#define hostSelectorMalloc					0x0318
#define hostSelectorRealloc					0x0319
#define hostSelectorFree					0x031A

	// time.h wrappers
#define hostSelectorAscTime					0x0370
#define hostSelectorClock					0x0371
#define hostSelectorCTime					0x0372
// #define hostSelectorDiffTime				0x0373
#define hostSelectorGMTime					0x0374
#define hostSelectorLocalTime				0x0375
#define hostSelectorMkTime					0x0376
#define hostSelectorStrFTime				0x0377
#define hostSelectorTime					0x0378

	// dirent.h wrappers
#define hostSelectorOpenDir					0x0380
#define hostSelectorReadDir					0x0381
// #define hostSelectorRewindDir				0x0382
#define hostSelectorCloseDir				0x0383
// #define hostSelectorTellDir					0x0384
// #define hostSelectorSeekDir					0x0385
// #define hostSelectorScanDir					0x0386

	// fcntl.h wrappers
// #define hostSelectorOpen					0x0386
// #define hostSelectorCreat					0x0388
// #define hostSelectorFcntl					0x0389

	// unistd.h wrappers
// #define hostSelectorAccess					0x038A
// #define hostSelectorChDir					0x038B
// #define hostSelectorClose					0x038C
// #define hostSelectorDup						0x038D
// #define hostSelectorDup2					0x038E
// #define hostSelectorGetCwd					0x038F
// #define hostSelectorIsATTY					0x0390
// #define hostSelectorLink					0x0391
// #define hostSelectorLSeek					0x0392
// #define hostSelectorRead					0x0393
#define hostSelectorRmDir					0x0394
// #define hostSelectorTTYName					0x0395
// #define hostSelectorUnlink					0x0396
// #define hostSelectorWrite					0x0397

// #define hostSelectorFChDir					0x0398
// #define hostSelectorFChMod					0x0399
// #define hostSelectorFileNo					0X039A
// #define hostSelectorFSync					0x039B
// #define hostSelectorFTruncate				0x039C
// #define hostSelectorGetHostName				0x039D
// #define hostSelectorGetWD					0x039E
// #define hostSelectorMkSTemp					0x039F
// #define hostSelectorMkTemp					0x03A0
// #define hostSelectorRe_Comp					0x03A1
// #define hostSelectorRe_Exec					0x03A2
// #define hostSelectorReadLink				0x03A3
// #define hostSelectorSetHostName				0x03A4
// #define hostSelectorSymLink					0x03A5
// #define hostSelectorSync					0x03A6
#define hostSelectorTruncate				0x03A7

	// sys/stat.h wrappers
// #define hostSelectorChMod					0x03A8
// #define hostSelectorFStat					0x03A9
#define hostSelectorMkDir					0x03AA
#define hostSelectorStat					0x03AB
// #define hostSelectorLStat					0x03AC

	// sys/time.h wrappers
// #define hostSelectorGetTimeOfDay			0x03AD
#define hostSelectorUTime					0x03AE

	// DOS attr 
#define hostSelectorGetFileAttr				0x03AF
#define hostSelectorSetFileAttr				0x03B0

	// Gremlin selectors

#define hostSelectorGremlinIsRunning		0x0400
#define hostSelectorGremlinNumber			0x0401
#define hostSelectorGremlinCounter			0x0402
#define hostSelectorGremlinLimit			0x0403
#define hostSelectorGremlinNew				0x0404


	// Database selectors

#define hostSelectorImportFile				0x0500
#define hostSelectorExportFile				0x0501
#define hostSelectorSaveScreen				0x0502
#define hostSelectorImportFileWithID		0x0503

#define hostSelectorExgLibOpen				0x0580
#define hostSelectorExgLibClose				0x0581
#define hostSelectorExgLibSleep				0x0582
#define hostSelectorExgLibWake				0x0583
#define hostSelectorExgLibHandleEvent		0x0584
#define hostSelectorExgLibConnect			0x0585
#define hostSelectorExgLibAccept			0x0586
#define hostSelectorExgLibDisconnect		0x0587
#define hostSelectorExgLibPut				0x0588
#define hostSelectorExgLibGet				0x0589
#define hostSelectorExgLibSend				0x058A
#define hostSelectorExgLibReceive			0x058B
#define hostSelectorExgLibControl			0x058C
#define hostSelectorExgLibRequest			0x058D


	// Preferences selectors

#define hostSelectorGetPreference			0x0600
#define hostSelectorSetPreference			0x0601


	// Logging selectors

#define hostSelectorLogFile					0x0700
#define hostSelectorSetLogFileSize			0x0701


	// RPC selectors

#define hostSelectorSessionCreate			0x0800		/* Not yet implemented in Poser */
#define hostSelectorSessionOpen				0x0801		/* Not yet implemented in Poser */
#define hostSelectorSessionClose			0x0802
#define hostSelectorSessionQuit				0x0803
#define hostSelectorSignalSend				0x0804
#define hostSelectorSignalWait				0x0805
#define hostSelectorSignalResume			0x0806
#define hostSelectorSessionSave				0x0807


	// External tracing tool support

#define hostSelectorTraceInit				0x0900
#define hostSelectorTraceClose				0x0901
#define hostSelectorTraceOutputT			0x0902
#define hostSelectorTraceOutputTL			0x0903
#define hostSelectorTraceOutputVT			0x0904
#define hostSelectorTraceOutputVTL			0x0905
#define hostSelectorTraceOutputB			0x0906


	// External debugger support

#define hostSelectorDbgSetDataBreak			0x0980		// mcc 13 june 2001
#define hostSelectorDbgClearDataBreak		0x0981		// mcc 13 june 2001


	// Slot support

#define hostSelectorSlotMax					0x0A00
#define hostSelectorSlotRoot				0x0A01
#define hostSelectorSlotHasCard				0x0A02


	// File choosing support

#define hostSelectorGetFile					0x0B00
#define hostSelectorPutFile					0x0B01
#define hostSelectorGetDirectory			0x0B02

#define hostSelectorLastTrapNumber			0x0BFF


	// * Types

typedef UInt16	HostControlSelectorType;
typedef Int32	HostBoolType;
typedef Int32	HostClockType;
typedef Int32	HostErrType;
typedef Int32	HostIDType;
typedef Int32	HostPlatformType;
typedef Int32	HostSignalType;
typedef Int32	HostSizeType;
typedef Int32	HostTimeType;


	// * HostDIRType

struct HostDIRType
{
	long	_field;
};

typedef struct HostDIRType HostDIRType;


	// * HostDirEntType

#define P_HOST_NAME_MAX	255

struct HostDirEntType
{
	char	d_name[P_HOST_NAME_MAX + 1];
};

typedef struct HostDirEntType HostDirEntType;


	// * HostFILEType

struct HostFILEType
{
	long	_field;
};

typedef struct HostFILEType HostFILEType;


	// * HostGremlinInfoType

struct HostGremlinInfoType
{
	long		fFirstGremlin;
	long		fLastGremlin;
	long		fSaveFrequency;
	long		fSwitchDepth;
	long		fMaxDepth;
	char		fAppNames[200];	// Comma-seperated list of application names
								// to run Gremlins on.  If the string is empty,
								// all applications are fair game.  If the string
								// begins with a '-' (e.g., "-Address,Datebook"),
								// then all applications named in the list are
								// excluded instead of included.
};

typedef struct HostGremlinInfoType HostGremlinInfoType;


	// * HostStatType
	//		Note that the field names here have an underscore appended to
	//		them in order to differentiate them from "compatibility macros"
	//		under Solaris.

struct HostStatType
{
	unsigned long	st_dev_;
	unsigned long	st_ino_;
	unsigned long	st_mode_;
	unsigned long	st_nlink_;
	unsigned long	st_uid_;
	unsigned long	st_gid_;
	unsigned long	st_rdev_;
	HostTimeType	st_atime_;
	HostTimeType	st_mtime_;
	HostTimeType	st_ctime_;
	unsigned long	st_size_;
	unsigned long	st_blksize_;
	unsigned long	st_blocks_;
	unsigned long	st_flags_;
};

typedef struct HostStatType HostStatType;


	// * HostTmType
	//		Note that the field names here have an underscore appended to
	//		them for consistancy with HostStatType.

struct HostTmType
{
	long	tm_sec_;	/* seconds after the minute - [0,59] */
	long	tm_min_;	/* minutes after the hour - [0,59] */
	long	tm_hour_;	/* hours since midnight - [0,23] */
	long	tm_mday_;	/* day of the month - [1,31] */
	long	tm_mon_;	/* months since January - [0,11] */
	long	tm_year_;	/* years since 1900 */
	long	tm_wday_;	/* days since Sunday - [0,6] */
	long	tm_yday_;	/* days since January 1 - [0,365] */
	long	tm_isdst_;	/* daylight savings time flag */
};

typedef struct HostTmType HostTmType;


	// * HostUTimeType
	//		Note that the field names here have an underscore appended to
	//		them for consistancy with HostStatType.

struct HostUTimeType
{
	HostTimeType	crtime_;	/* creation time */
	HostTimeType	actime_;	/* access time */
	HostTimeType	modtime_;	/* modification time */
};

typedef struct HostUTimeType HostUTimeType;


	// * Backward compatiblity

typedef HostControlSelectorType	HostControlTrapNumber;
typedef HostBoolType			HostBool;
typedef HostErrType				HostErr;
typedef HostIDType				HostID;
typedef HostPlatformType		HostPlatform;
typedef HostSignalType			HostSignal;
typedef HostFILEType			HostFILE;



#ifndef hostErrorClass
	#define	hostErrorClass				0x1C00	// Host Control Manager
#else
	#if hostErrorClass != 0x1C00
		#error "You cannot change hostErrorClass without telling us."
	#endif
#endif

enum	// HostErrType values
{
	hostErrNone = 0,

	hostErrBase = hostErrorClass,

	hostErrUnknownGestaltSelector,
	hostErrDiskError,
	hostErrOutOfMemory,
	hostErrMemReadOutOfRange,
	hostErrMemWriteOutOfRange,
	hostErrMemInvalidPtr,
	hostErrInvalidParameter,
	hostErrTimeout,
	hostErrInvalidDeviceType,
	hostErrInvalidRAMSize,
	hostErrFileNotFound,
	hostErrRPCCall,				// Issued if the following functions are not called remotely:
								//		HostSessionCreate
								//		HostSessionOpen
								//		HostSessionClose
								//		HostSessionQuit
								//		HostSignalWait
								//		HostSignalResume
	hostErrSessionRunning,		// Issued by HostSessionCreate, HostSessionOpen, and
								// HostSessionQuit if a session is running.
	hostErrSessionNotRunning,	// Issued by HostSessionClose or HostSessionSave if no session is running.
	hostErrNoSignalWaiters,		// Issued by HostSendSignal if no one's waiting for a signal.
	hostErrSessionNotPaused,	// Issued when HostSignalResume, but the session was not
								// halted from a HostSignalSend call.
	
	hostErrPermissions,
	hostErrFileNameTooLong,
	hostErrNotADirectory,
	hostErrTooManyFiles,
	hostErrFileTooBig,
	hostErrReadOnlyFS,
	hostErrIsDirectory,
	hostErrExists,
	hostErrOpNotAvailable,
	hostErrDirNotEmpty,
	hostErrDiskFull,
	hostErrUnknownError,
	hostErrProfilingNotReady

	// (Add new error codes here at the end.)
};


enum	// HostIDType values
{
	hostIDPalmOS,			// The plastic thingy
	hostIDPalmOSEmulator,	// The Copilot thingy
	hostIDPalmOSSimulator	// The Mac libraries you link with thingy
};


enum	// HostPlatformType values
{
	hostPlatformPalmOS,
	hostPlatformWindows,
	hostPlatformMacintosh,
	hostPlatformUnix
};

enum	// HostSignalType values
{
	hostSignalReserved,
	hostSignalIdle,
	hostSignalQuit,
#if 0
	// (Proposed...not supported yet)
	hostSignalSessionStarted,
	hostSignalSessionStopped,
	hostSignalHordeStarted,
	hostSignalGremlinStarted,
	hostSignalGremlinSuspended,
	hostSignalGremlinResumed,
	hostSignalGremlinStopped,
	hostSignalHordeStopped,
#endif
	hostSignalUser	= 0x4000	// User-defined values start here and go up.
};

enum	// HostGet/SetFileAttr flags, matching EmFileAttr flags
{
	hostFileAttrReadOnly = 1,
	hostFileAttrHidden = 2,
	hostFileAttrSystem = 4
};

// Use these to call FtrGet to see if you're running under the
// Palm OS Emulator.  If not, FtrGet will return ftrErrNoSuchFeature.

#define kPalmOSEmulatorFeatureCreator	('pose')
#define kPalmOSEmulatorFeatureNumber	(0)


// Define this, since SysTraps.h doesn't have it.

#ifdef __SYSTRAPS_H_
#define sysTrapHostControl sysTrapSysGremlins
#endif


// Define HOST_TRAP

#if defined (_SYSTEM_API)

#define HOST_TRAP(selector)	\
	_SYSTEM_API(_CALL_WITH_16BIT_SELECTOR)(_SYSTEM_TABLE, sysTrapHostControl, selector)

#else

#define HOST_TRAP(selector)													\
	FIVEWORD_INLINE(														\
		0x3F3C, selector,					/* MOVE.W #selector, -(A7)	*/	\
		m68kTrapInstr + sysDispatchTrapNum,	/* TRAP $F					*/	\
		sysTrapHostControl,					/* sysTrapHostControl		*/	\
		0x544F)								/* ADD.Q #2, A7				*/ 
#endif


/* ==================================================================== */
/* Host environment-related calls										*/
/* ==================================================================== */

Int32				HostGetHostVersion(void)
						HOST_TRAP(hostSelectorGetHostVersion);

HostIDType			HostGetHostID(void)
						HOST_TRAP(hostSelectorGetHostID);

HostPlatformType	HostGetHostPlatform(void)
						HOST_TRAP(hostSelectorGetHostPlatform);

HostBoolType		HostIsSelectorImplemented(long selector)
						HOST_TRAP(hostSelectorIsSelectorImplemented);

HostErrType			HostGestalt(long gestSel, long* response)
						HOST_TRAP(hostSelectorGestalt);

HostBoolType		HostIsCallingTrap(void)
						HOST_TRAP(hostSelectorIsCallingTrap);


/* ==================================================================== */
/* Profiling-related calls												*/
/* ==================================================================== */

HostErrType			HostProfileInit(long maxCalls, long maxDepth)
						HOST_TRAP(hostSelectorProfileInit);

HostErrType			HostProfileDetailFn(void* addr, HostBoolType logDetails)
						HOST_TRAP(hostSelectorProfileDetailFn);

HostErrType			HostProfileStart(void)
						HOST_TRAP(hostSelectorProfileStart);

HostErrType			HostProfileStop(void)
						HOST_TRAP(hostSelectorProfileStop);

HostErrType			HostProfileDump(const char* filenameP)
						HOST_TRAP(hostSelectorProfileDump);

HostErrType			HostProfileCleanup(void)
						HOST_TRAP(hostSelectorProfileCleanup);

long				HostProfileGetCycles(void)
						HOST_TRAP(hostSelectorProfileGetCycles);


/* ==================================================================== */
/* Std C Library-related calls											*/
/* ==================================================================== */

long				HostErrNo(void)
						HOST_TRAP(hostSelectorErrNo);


long				HostFClose(HostFILEType* fileP)
						HOST_TRAP(hostSelectorFClose);

long				HostFEOF(HostFILEType* fileP)
						HOST_TRAP(hostSelectorFEOF);

long				HostFError(HostFILEType* fileP)
						HOST_TRAP(hostSelectorFError);

long				HostFFlush(HostFILEType* fileP)
						HOST_TRAP(hostSelectorFFlush);

long				HostFGetC(HostFILEType* fileP)
						HOST_TRAP(hostSelectorFGetC);

long				HostFGetPos(HostFILEType* fileP, long* posP)
						HOST_TRAP(hostSelectorFGetPos);

char*				HostFGetS(char* s, long n, HostFILEType* fileP)
						HOST_TRAP(hostSelectorFGetS);

HostFILEType*		HostFOpen(const char* name, const char* mode)
						HOST_TRAP(hostSelectorFOpen);

long				HostFPrintF(HostFILEType* fileP, const char* fmt, ...)
						HOST_TRAP(hostSelectorFPrintF);

long				HostFPutC(long c, HostFILEType* fileP)
						HOST_TRAP(hostSelectorFPutC);

long				HostFPutS(const char* s, HostFILEType* fileP)
						HOST_TRAP(hostSelectorFPutS);

long				HostFRead(void* buffer, long size, long count, HostFILEType* fileP)
						HOST_TRAP(hostSelectorFRead);

HostFILEType*		HostFReopen(const char* name, const char* mode, HostFILEType* fileP)
						HOST_TRAP(hostSelectorFReopen);

long				HostFScanF(HostFILEType* fileP, const char* fmt, ...)
						HOST_TRAP(hostSelectorFScanF);

long				HostFSeek(HostFILEType* fileP, long offset, long origin)
						HOST_TRAP(hostSelectorFSeek);

long				HostFSetPos(HostFILEType* fileP, long* pos)
						HOST_TRAP(hostSelectorFSetPos);

long				HostFTell(HostFILEType* fileP)
						HOST_TRAP(hostSelectorFTell);

long				HostFWrite(const void* buffer, long size, long count, HostFILEType* fileP)
						HOST_TRAP(hostSelectorFWrite);

long				HostRemove(const char* nameP)
						HOST_TRAP(hostSelectorRemove);

long				HostRename(const char* oldNameP, const char* newNameP)
						HOST_TRAP(hostSelectorRename);

HostFILEType*		HostTmpFile(void)
						HOST_TRAP(hostSelectorTmpFile);

char*				HostTmpNam(char* nameP)
						HOST_TRAP(hostSelectorTmpNam);

char*				HostGetEnv(const char* nameP)
						HOST_TRAP(hostSelectorGetEnv);


void*				HostMalloc(long size)
						HOST_TRAP(hostSelectorMalloc);

void*				HostRealloc(void* p, long size)
						HOST_TRAP(hostSelectorRealloc);

void				HostFree(void* p)
						HOST_TRAP(hostSelectorFree);


char*				HostAscTime(const HostTmType*)
						HOST_TRAP(hostSelectorAscTime);

HostClockType		HostClock(void)
						HOST_TRAP(hostSelectorClock);

char*				HostCTime(const HostTimeType*)
						HOST_TRAP(hostSelectorCTime);

//double				HostDiffTime(HostTimeType, HostTimeType)
//						HOST_TRAP(hostSelectorDiffTime);

HostTmType*			HostGMTime(const HostTimeType*)
						HOST_TRAP(hostSelectorGMTime);

HostTmType*			HostLocalTime(const HostTimeType*)
						HOST_TRAP(hostSelectorLocalTime);

HostTimeType		HostMkTime(HostTmType*)
						HOST_TRAP(hostSelectorMkTime);

HostSizeType		HostStrFTime(char*, HostSizeType, const char*, const HostTmType*)
						HOST_TRAP(hostSelectorStrFTime);

HostTimeType		HostTime(HostTimeType*)
						HOST_TRAP(hostSelectorTime);


long				HostMkDir(const char*)
						HOST_TRAP(hostSelectorMkDir);

long				HostRmDir(const char*)
						HOST_TRAP(hostSelectorRmDir);

HostDIRType*		HostOpenDir(const char*)
						HOST_TRAP(hostSelectorOpenDir);

HostDirEntType*		HostReadDir(HostDIRType*)
						HOST_TRAP(hostSelectorReadDir);

long				HostCloseDir(HostDIRType*)
						HOST_TRAP(hostSelectorCloseDir);


long				HostStat(const char*, HostStatType*)
						HOST_TRAP(hostSelectorStat);

long				HostTruncate(const char*, long)
						HOST_TRAP(hostSelectorTruncate);


long				HostUTime (const char*, HostUTimeType*)
						HOST_TRAP(hostSelectorUTime);

long				HostGetFileAttr(const char*, long*)
						HOST_TRAP(hostSelectorGetFileAttr);

long				HostSetFileAttr(const char*, long)
						HOST_TRAP(hostSelectorSetFileAttr);


/* ==================================================================== */
/* Gremlin-related calls												*/
/* ==================================================================== */

HostBoolType		HostGremlinIsRunning(void)
						HOST_TRAP(hostSelectorGremlinIsRunning);

long				HostGremlinNumber(void)
						HOST_TRAP(hostSelectorGremlinNumber);

long				HostGremlinCounter(void)
						HOST_TRAP(hostSelectorGremlinCounter);

long				HostGremlinLimit(void)
						HOST_TRAP(hostSelectorGremlinLimit);

HostErrType			HostGremlinNew(const HostGremlinInfoType*)
						HOST_TRAP(hostSelectorGremlinNew);


/* ==================================================================== */
/* Import/export-related calls											*/
/* ==================================================================== */

HostErrType			HostImportFile(const char* fileName, long cardNum)
						HOST_TRAP(hostSelectorImportFile);

HostErrType			HostImportFileWithID(const char* fileName, long cardNum, LocalID* newIDP)
						HOST_TRAP(hostSelectorImportFileWithID);

HostErrType			HostExportFile(const char* fileName, long cardNum, const char* dbName)
						HOST_TRAP(hostSelectorExportFile);

HostErrType			HostSaveScreen(const char* fileName)
						HOST_TRAP(hostSelectorSaveScreen);

	// These are private, internal functions.  Third party applications
	// should not be calling them.

Err					HostExgLibOpen			(UInt16 libRefNum)
						HOST_TRAP(hostSelectorExgLibOpen);

Err					HostExgLibClose			(UInt16 libRefNum)
						HOST_TRAP(hostSelectorExgLibClose);

Err					HostExgLibSleep			(UInt16 libRefNum)
						HOST_TRAP(hostSelectorExgLibSleep);

Err					HostExgLibWake			(UInt16 libRefNum)
						HOST_TRAP(hostSelectorExgLibWake);

Boolean				HostExgLibHandleEvent	(UInt16 libRefNum, void* eventP)
						HOST_TRAP(hostSelectorExgLibHandleEvent);

Err	 				HostExgLibConnect		(UInt16 libRefNum, void* exgSocketP)
						HOST_TRAP(hostSelectorExgLibConnect);

Err					HostExgLibAccept		(UInt16 libRefNum, void* exgSocketP)
						HOST_TRAP(hostSelectorExgLibAccept);

Err					HostExgLibDisconnect	(UInt16 libRefNum, void* exgSocketP,Err error)
						HOST_TRAP(hostSelectorExgLibDisconnect);

Err					HostExgLibPut			(UInt16 libRefNum, void* exgSocketP)
						HOST_TRAP(hostSelectorExgLibPut);

Err					HostExgLibGet			(UInt16 libRefNum, void* exgSocketP)
						HOST_TRAP(hostSelectorExgLibGet);

UInt32 				HostExgLibSend			(UInt16 libRefNum, void* exgSocketP, const void* const bufP, const UInt32 bufLen, Err* errP)
						HOST_TRAP(hostSelectorExgLibSend);

UInt32 				HostExgLibReceive		(UInt16 libRefNum, void* exgSocketP, void* bufP, const UInt32 bufSize, Err* errP)
						HOST_TRAP(hostSelectorExgLibReceive);

Err 				HostExgLibControl		(UInt16 libRefNum, UInt16 op, void* valueP, UInt16* valueLenP)
						HOST_TRAP(hostSelectorExgLibControl);

Err 				HostExgLibRequest		(UInt16 libRefNum, void* exgSocketP)
						HOST_TRAP(hostSelectorExgLibRequest);


/* ==================================================================== */
/* Preference-related calls												*/
/* ==================================================================== */

HostBoolType		HostGetPreference(const char*, char*)
						HOST_TRAP(hostSelectorGetPreference);

void				HostSetPreference(const char*, const char*)
						HOST_TRAP(hostSelectorSetPreference);


/* ==================================================================== */
/* Logging-related calls												*/
/* ==================================================================== */

HostFILEType*		HostLogFile(void)
						HOST_TRAP(hostSelectorLogFile);

void				HostSetLogFileSize(long)
						HOST_TRAP(hostSelectorSetLogFileSize);


/* ==================================================================== */
/* RPC-related calls													*/
/* ==================================================================== */

HostErrType			HostSessionCreate(const char* device, long ramSize, const char* romPath)
						HOST_TRAP(hostSelectorSessionCreate);

HostErrType			HostSessionOpen(const char* psfFileName)
						HOST_TRAP(hostSelectorSessionOpen);

HostBoolType		HostSessionSave(const char* saveFileName)
						HOST_TRAP(hostSelectorSessionSave);

HostErrType			HostSessionClose(const char* saveFileName)
						HOST_TRAP(hostSelectorSessionClose);

HostErrType			HostSessionQuit(void)
						HOST_TRAP(hostSelectorSessionQuit);

HostErrType			HostSignalSend(HostSignalType signalNumber)
						HOST_TRAP(hostSelectorSignalSend);

HostErrType			HostSignalWait(long timeout, HostSignalType* signalNumber)
						HOST_TRAP(hostSelectorSignalWait);

HostErrType			HostSignalResume(void)
						HOST_TRAP(hostSelectorSignalResume);


/* ==================================================================== */
/* Tracing calls														*/
/* ==================================================================== */

void				HostTraceInit(void)
						HOST_TRAP(hostSelectorTraceInit);

void				HostTraceClose(void)
						HOST_TRAP(hostSelectorTraceClose);

void				HostTraceOutputT(unsigned short, const char*, ...)
						HOST_TRAP(hostSelectorTraceOutputT);

void				HostTraceOutputTL(unsigned short, const char*, ...)
						HOST_TRAP(hostSelectorTraceOutputTL);

void				HostTraceOutputVT(unsigned short, const char*, char* /*va_list*/)
						HOST_TRAP(hostSelectorTraceOutputVT);

void				HostTraceOutputVTL(unsigned short, const char*, char* /*va_list*/)
						HOST_TRAP(hostSelectorTraceOutputVTL);

void				HostTraceOutputB(unsigned short, const void*, HostSizeType)
						HOST_TRAP(hostSelectorTraceOutputB);


/* ==================================================================== */
/* Debugger calls														*/
/* ==================================================================== */

HostErr				HostDbgSetDataBreak (UInt32 addr, UInt32 size)
						HOST_TRAP(hostSelectorDbgSetDataBreak);

HostErr				HostDbgClearDataBreak (void)
						HOST_TRAP(hostSelectorDbgClearDataBreak);


/* ==================================================================== */
/* Slot related calls													*/
/* ==================================================================== */

long				HostSlotMax(void)
						HOST_TRAP(hostSelectorSlotMax);

const char*			HostSlotRoot(long slotNo)
						HOST_TRAP(hostSelectorSlotRoot);

HostBoolType		HostSlotHasCard(long slotNo)
						HOST_TRAP(hostSelectorSlotHasCard);


/* ==================================================================== */
/* File Choosing support												*/
/* ==================================================================== */

const char*			HostGetFile(const char* prompt, const char* defaultDir)
						HOST_TRAP(hostSelectorGetFile);

const char*			HostPutFile(const char* prompt, const char* defaultDir, const char* defaultName)
						HOST_TRAP(hostSelectorPutFile);

const char*			HostGetDirectory(const char* prompt, const char* defaultDir)
						HOST_TRAP(hostSelectorGetDirectory);

#ifdef __cplusplus 
}
#endif

#endif /* _HOSTCONTROL_H_ */
