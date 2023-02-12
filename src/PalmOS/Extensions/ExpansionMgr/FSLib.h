/******************************************************************************
 *
 * Copyright (c) 1998-2003 PalmSource, Inc. All rights reserved.
 *
 * File: FSLib.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *              Sample file system library implementation.
 *
 *****************************************************************************/

/********************************************************************
 * Filename and Label conventions:
 *
 * All path names are absolute
 *
 * All filesystems must support filenames and labels that are up to 255 characters long,
 * using any normal character including spaces and lower case characters in any
 * character set and the following special characters:
 * $ % ' - _ @ ~ ` ! ( ) ^ # & + , ; = [ ]
 ********************************************************************
 * When creating the 8.3 name or label from a long filename or label:
 *  a) Create the name from the first 1-6 valid, non-space characters, before the last period.
 *		The only valid characters are:
 *			A-Z 0-9 $ % ' - _ @ ~ ` ! ( ) ^ # &
 *  b) the extension is the first three valid characters after the last period '.'
 *  c) the end of the 6 byte name is appended with ~1, or the next unique number.
 *
 * A label is created from the first 11 valid non-space characters.
 ********************************************************************/


#ifndef __FS_LIB_H__
#define __FS_LIB_H__


#include <VFSMgr.h>	//	FileRef, etc.


// When building the PalmOS 3.5 version of ExpansionMgr, 
// since this constant was not in the 3.5 SystemResources.h...
#ifdef BUILDING_AGAINST_PALMOS35	
#define	sysFileTFileSystem				'libf'	// File type for file system libraries
#endif // BUILDING_AGAINST_PALMOS35



// BUILDING_APPLICATION
#if BUILDING_APP_OR_LIB
	// direct link to library code
	#define FS_LIB_TRAP(trapNum)
#else
	// else someone else is including this public header file; use traps
	#define FS_LIB_TRAP(trapNum)	SYS_TRAP(trapNum)
#endif

#define fsLibAPIVersion				0x00000002


/********************************************************************
 * Type of FS Library database
 ********************************************************************/


/********************************************************************
 * FS library function trap ID's. Each library call gets a trap number:
 *   FSTrapXXXX which serves as an index into the library's dispatch table.
 *   The constant sysLibTrapCustom is the first available trap number after
 *   the system predefined library traps Open,Close,Sleep & Wake.
 *
 * WARNING!!! The order of these traps MUST match the order of the dispatch
 *  table in FSLibDispatch.c!!!
 ********************************************************************/

#define FSTrapLibAPIVersion		(sysLibTrapCustom)
#define FSTrapCustomControl		(sysLibTrapCustom+1)
#define FSTrapFilesystemType		(sysLibTrapCustom+2)

#define FSTrapFileCreate			(sysLibTrapCustom+3)
#define FSTrapFileOpen				(sysLibTrapCustom+4)
#define FSTrapFileClose				(sysLibTrapCustom+5)
#define FSTrapFileRead				(sysLibTrapCustom+6)
#define FSTrapFileWrite				(sysLibTrapCustom+7)
#define FSTrapFileDelete			(sysLibTrapCustom+8)
#define FSTrapFileRename			(sysLibTrapCustom+9)
#define FSTrapFileSeek				(sysLibTrapCustom+10)
#define FSTrapFileEOF				(sysLibTrapCustom+11)
#define FSTrapFileTell				(sysLibTrapCustom+12)
#define FSTrapFileResize			(sysLibTrapCustom+13)
#define FSTrapFileGetAttributes	(sysLibTrapCustom+14)
#define FSTrapFileSetAttributes	(sysLibTrapCustom+15)
#define FSTrapFileGetDate			(sysLibTrapCustom+16)
#define FSTrapFileSetDate			(sysLibTrapCustom+17)
#define FSTrapFileSize				(sysLibTrapCustom+18)

#define FSTrapDirCreate				(sysLibTrapCustom+19)
#define FSTrapDirEntryEnumerate	(sysLibTrapCustom+20)

#define FSTrapVolumeFormat			(sysLibTrapCustom+21)
#define FSTrapVolumeMount			(sysLibTrapCustom+22)
#define FSTrapVolumeUnmount		(sysLibTrapCustom+23)
#define FSTrapVolumeInfo			(sysLibTrapCustom+24)
#define FSTrapVolumeGetLabel		(sysLibTrapCustom+25)
#define FSTrapVolumeSetLabel		(sysLibTrapCustom+26)
#define FSTrapVolumeSize			(sysLibTrapCustom+27)

#define FSMaxSelector				FSTrapVolumeSize


/********************************************************************
 * API Prototypes
 ********************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/********************************************************************
 * Standard library open, close, sleep and wake APIs:
 ********************************************************************/

extern Err FSLibOpen(UInt16 fsLibRefNum)
				FS_LIB_TRAP(sysLibTrapOpen);

extern Err FSLibClose(UInt16 fsLibRefNum)
				FS_LIB_TRAP(sysLibTrapClose);

extern Err FSLibSleep(UInt16 fsLibRefNum)
				FS_LIB_TRAP(sysLibTrapSleep);
	
extern Err FSLibWake(UInt16 fsLibRefNum)
				FS_LIB_TRAP(sysLibTrapWake);


/********************************************************************
 * Custom library APIs:
 ********************************************************************/

extern UInt32 FSLibAPIVersion(UInt16 fsLibRefNum)
				FS_LIB_TRAP(FSTrapLibAPIVersion);

extern Err FSCustomControl(UInt16 fsLibRefNum, UInt32 apiCreator, UInt16 apiSelector, 
							void *valueP, UInt16 *valueLenP)
				FS_LIB_TRAP(FSTrapCustomControl);
				
extern Err FSFilesystemType(UInt16 fsLibRefNum, UInt32 *filesystemTypeP)
				FS_LIB_TRAP(FSTrapFilesystemType);
				

/********************************************************************
 * File Stream APIs:
 ********************************************************************/
 
extern Err FSFileCreate(UInt16 fsLibRefNum, UInt16 volRefNum, const Char *pathNameP)
				FS_LIB_TRAP(FSTrapFileCreate);

extern Err FSFileOpen(UInt16 fsLibRefNum, UInt16 volRefNum, const Char *pathNameP,
	UInt16 openMode, FileRef *fileRefP)
				FS_LIB_TRAP(FSTrapFileOpen);

extern Err FSFileClose(UInt16 fsLibRefNum, FileRef fileRef)
				FS_LIB_TRAP(FSTrapFileClose);

extern Err FSFileRead(UInt16 fsLibRefNum, FileRef fileRef, UInt32 numBytes, 
						void *bufBaseP, UInt32 offset, Boolean dataStoreBased, 
						UInt32 *numBytesReadP)
				FS_LIB_TRAP(FSTrapFileRead);

extern Err FSFileWrite(UInt16 fsLibRefNum, FileRef fileRef, UInt32 numBytes, 
						const void *dataP, UInt32 *numBytesWrittenP)
				FS_LIB_TRAP(FSTrapFileWrite);

extern Err FSFileDelete(UInt16 fsLibRefNum, UInt16 volRefNum, const Char *pathNameP)
				FS_LIB_TRAP(FSTrapFileDelete);

extern Err FSFileRename(UInt16 fsLibRefNum, UInt16 volRefNum, const Char *pathNameP, const Char *newNameP)
				FS_LIB_TRAP(FSTrapFileRename);

extern Err FSFileSeek(UInt16 fsLibRefNum, FileRef fileRef, FileOrigin origin, Int32 offset)
				FS_LIB_TRAP(FSTrapFileSeek);

extern Err FSFileEOF(UInt16 fsLibRefNum, FileRef fileRef)
				FS_LIB_TRAP(FSTrapFileEOF);

extern Err FSFileTell(UInt16 fsLibRefNum, FileRef fileRef, UInt32 *filePosP)
				FS_LIB_TRAP(FSTrapFileTell);

extern Err FSFileResize(UInt16 fsLibRefNum, FileRef fileRef, UInt32 newSize)
				FS_LIB_TRAP(FSTrapFileResize);

extern Err FSFileGetAttributes(UInt16 fsLibRefNum, FileRef fileRef, UInt32 *attributesP)
				FS_LIB_TRAP(FSTrapFileGetAttributes);

extern Err FSFileSetAttributes(UInt16 fsLibRefNum, FileRef fileRef, UInt32 attributes)
				FS_LIB_TRAP(FSTrapFileSetAttributes);

extern Err FSFileGetDate(UInt16 fsLibRefNum, FileRef fileRef, UInt16 whichDate, UInt32 *dateP)
				FS_LIB_TRAP(FSTrapFileGetDate);

extern Err FSFileSetDate(UInt16 fsLibRefNum, FileRef fileRef, UInt16 whichDate, UInt32 date)
				FS_LIB_TRAP(FSTrapFileSetDate);

extern Err FSFileSize(UInt16 fsLibRefNum, FileRef fileRef, UInt32 *fileSizeP)
				FS_LIB_TRAP(FSTrapFileSize);


/********************************************************************
 * Directory APIs:
 ********************************************************************/
 
extern Err FSDirCreate(UInt16 fsLibRefNum, UInt16 volRefNum, const Char *dirNameP)
				FS_LIB_TRAP(FSTrapDirCreate);


/************************************************************
 *
 *  MACRO:			FSDirDelete
 *
 *  DESCRIPTION:	Delete a closed directory.
 *
 *  PARAMETERS:	fsLibRefNum				-- FS library reference number
 *				volRefNum				-- Volume reference number returned by FSVolumeMount
 *				pathNameP				-- Full path of the directory to be deleted
 *
 *  RETURNS:	errNone					-- no error
 *				expErrNotOpen			-- FS driver library has not been opened
 *				vfsErrFileStillOpen		-- Directory is still open
 *				vfsErrFileNotFound		-- the file could not be found 
 *				vfsErrVolumeBadRef		-- the volume has not been mounted with FSVolumeMount
 *
 *************************************************************/
#define FSDirDelete(fsLibRefNum, volRefNum, dirNameP)	\
		FSFileDelete(fsLibRefNum, volRefNum, dirNameP)

extern Err FSDirEntryEnumerate(UInt16 fsLibRefNum, FileRef dirRef, UInt32 *dirEntryIteratorP, FileInfoType *infoP)
				FS_LIB_TRAP(FSTrapDirEntryEnumerate);

/********************************************************************
 * Volume APIs:
 ********************************************************************/
 

extern Err FSVolumeFormat(UInt16 fsLibRefNum, VFSAnyMountParamPtr vfsMountParamP)
				FS_LIB_TRAP(FSTrapVolumeFormat);

extern Err FSVolumeMount(UInt16 fsLibRefNum, VFSAnyMountParamPtr vfsMountParamP)
				FS_LIB_TRAP(FSTrapVolumeMount);

extern Err FSVolumeUnmount(UInt16 fsLibRefNum, UInt16 volRefNum)
				FS_LIB_TRAP(FSTrapVolumeUnmount);

extern Err FSVolumeInfo(UInt16 fsLibRefNum, UInt16 volRefNum, VolumeInfoType *volInfoP)
				FS_LIB_TRAP(FSTrapVolumeInfo);

extern Err FSVolumeGetLabel(UInt16 fsLibRefNum, UInt16 volRefNum, Char *labelP, UInt16 bufLen)
				FS_LIB_TRAP(FSTrapVolumeGetLabel);

extern Err FSVolumeSetLabel(UInt16 fsLibRefNum, UInt16 volRefNum, const Char *labelP)
				FS_LIB_TRAP(FSTrapVolumeSetLabel);

extern Err FSVolumeSize(UInt16 fsLibRefNum, UInt16 volRefNum, UInt32 *volumeUsedP, UInt32 *volumeTotalP)
				FS_LIB_TRAP(FSTrapVolumeSize);


#ifdef __cplusplus 
}
#endif


#endif	// __FS_LIB_H__
