/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: FileStream.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *		Pilot File Stream equates -- File Streams were initially implemented
 *		in PalmOS v3.0 (not available in earlier versions)
 *
 *****************************************************************************/

#ifndef __FILESTREAM_H__
#define __FILESTREAM_H__

#include <PalmTypes.h>
#include <CoreTraps.h>
#include <ErrorBase.h>

/************************************************************
 * File Stream error codes
 * the constant dmErrorClass is defined in ErrorBase.h
 *************************************************************/

#define	fileErrMemError				(fileErrorClass | 1)	// out of memory error
#define	fileErrInvalidParam			(fileErrorClass | 2)	// invalid parameter value passed
#define	fileErrCorruptFile			(fileErrorClass | 3)	// the file is corrupted/invalid/not a stream file
#define	fileErrNotFound				(fileErrorClass | 4)	// couldn't find the file
#define	fileErrTypeCreatorMismatch	(fileErrorClass | 5)	// file's type and creator didn't match those expected
#define	fileErrReplaceError			(fileErrorClass | 6)	// couldn't replace an existing file
#define	fileErrCreateError			(fileErrorClass | 7)	// couldn't create a new file
#define	fileErrOpenError				(fileErrorClass | 8)	// generic open error
#define	fileErrInUse					(fileErrorClass | 9)	// file couldn't be opened or deleted because it is in use
#define	fileErrReadOnly				(fileErrorClass | 10)// couldn't open in write mode because db is read-only
#define	fileErrInvalidDescriptor	(fileErrorClass | 11)// invalid file descriptor (FileHandle)
#define	fileErrCloseError				(fileErrorClass | 12)// error closing the database
#define	fileErrOutOfBounds			(fileErrorClass | 13)// attempted operation went out of bounds of the file
#define	fileErrPermissionDenied		(fileErrorClass | 14)// couldn't write to a file open for read-only access
#define	fileErrIOError					(fileErrorClass | 15)// general I/O error
#define	fileErrEOF						(fileErrorClass | 16)// end-of-file error
#define	fileErrNotStream				(fileErrorClass | 17)// attempted to open a file that is not a stream



/************************************************************
 * File Stream handle type
 *************************************************************/
#ifdef PALMOS
typedef MemHandle	FileHand;
#else
typedef void *FileHand;
#endif

#define fileNullHandle	((FileHand)0)


/************************************************************
 * Mode flags passed to FileOpen
 *************************************************************/

// fileModeReadOnly, fileModeReadWrite, fileModeUpdate, and fileModeAppend are mutually exclusive - only
// pass one of them to FileOpen!
#define	fileModeReadOnly				(0x80000000UL)			// open for read access
#define	fileModeReadWrite				(0x40000000UL)			// create for read/write access, discarding previous if any */
#define	fileModeUpdate					(0x20000000UL)			// open/create for read/write, preserving previous if any
#define	fileModeAppend					(0x10000000UL)			// open/create for read/write, always writing at the end 

#define	fileModeLeaveOpen				(0x08000000UL)			// leave open when app quits
#define	fileModeExclusive				(0x04000000UL)			// don't let anyone else open it
#define	fileModeAnyTypeCreator		(0x02000000UL)			// if set, skip type/creator validation when
																			// opening or replacing an existing file

#define	fileModeTemporary				(0x01000000UL)			// will automatically delete the file when it is closed;
																			// if this bit is set and the file type passed to FileOpen is zero,
																			// FileOpen will use sysFileTTemp (defined in SystemResources.h for the file
																			// type (recommended) - this will enable automatic cleanup of undeleted
																			// temp files following a system crash in future PalmOS versions
																			// (post-crash cleanup will likely come after 3.0)

#define	fileModeDontOverwrite		(0x00800000UL)			// if set, will prevent fileModeReadWrite from discarding an existing file
																			// with the same name; may only be specified together with fileModeReadWrite

// For debugging/validation
#define fileModeAllFlags			(	fileModeReadOnly	|			\
												fileModeReadWrite	|			\
												fileModeUpdate		|			\
												fileModeAppend		|			\
												fileModeLeaveOpen |			\
												fileModeExclusive	|			\
												fileModeAnyTypeCreator	|	\
												fileModeTemporary	|			\
												fileModeDontOverwrite	)

/************************************************************
 * Origin passed to FileSetPos
 *************************************************************/
typedef enum FileOriginEnum {

	fileOriginBeginning	= 1,			// from the beginning (first data byte of file)
	fileOriginCurrent,					// from the current position
	fileOriginEnd							// from the end of file (one position beyond last data byte)

	} FileOriginEnum;



/************************************************************
 * Operation passed to FileControl
 *************************************************************/
typedef enum FileOpEnum {
	fileOpNone = 0,						// no-op

	fileOpDestructiveReadMode,			// switch to destructive read mode (there is no turning back);
												// implicitly rewinds the file to the beginning;
												// destructive read mode deletes file stream data blocks as
												// data is being read, thus freeing up storage automatically;
												// once in destructive read mode, FileWrite, FileSeek and FileTruncate
												// are not allowed; stream's contents after closing (or crash)
												// are undefined.
												// ARGUMENTS:
												// 	stream = open stream handle
												// 	valueP = NULL
												// 	valueLenP = NULL
												// RETURNS:
												//		zero on success; fileErr... on error
												
	fileOpGetEOFStatus,					// get end-of-file status (err = fileErrEOF indicates end of file condition);
												// use FileClearerr to clear this error status
												// ARGUMENTS:
												// 	stream = open stream handle
												// 	valueP = NULL
												// 	valueLenP = NULL
												// RETURNS:
												//		zero if _not_ end of file; non-zero if end of file

	fileOpGetLastError,					// get error code from last operation on file stream, and
												// clear the last error code value (will not change end of file
												// or I/O error status -- use FileClearerr to reset all error codes)
												// ARGUMENTS:
												//		stream = open stream handle
												// 	valueP = NULL
												// 	valueLenP = NULL
												// RETURNS:
												//		Error code from last file stream operation

	fileOpClearError,						// clear I/O and end of file error status, and last error
												// ARGUMENTS:
												// 	stream = open stream handle
												// 	valueP = NULL
												// 	valueLenP = NULL
												// RETURNS:
												//		zero on success; fileErr... on error

	fileOpGetIOErrorStatus,				// get I/O error status (like C runtime's ferror); use FileClearerr
												// to clear this error status
												// ARGUMENTS:
												// 	stream = open stream handle
												// 	valueP = NULL
												//		valueLenP = NULL
												// RETURNS:
												//		zero if _not_ I/O error; non-zero if I/O error is pending

	fileOpGetCreatedStatus,				// find out whether the FileOpen call caused the file to
												// be created
												// ARGUMENTS:
												//		stream = open stream handle
												//		valueP = ptr to Boolean type variable
												//		valueLenP = ptr to Int32 variable set to sizeof(Boolean)
												// RETURNS:
												//		zero on success; fileErr... on error;
												//		the Boolean variable will be set to non zero if the file was created.
												
	fileOpGetOpenDbRef,					// get the open database reference (handle) of the underlying
												// database that implements the stream (NULL if none);
												// this is needed for performing PalmOS-specific operations on
												// the underlying database, such as changing or getting creator/type,
												// version, backup/reset bits, etc.
												// ARGUMENTS:
												//		stream = open stream handle
												//		valueP = ptr to DmOpenRef type variable
												//		valueLenP = ptr to Int32 variable set to sizeof(DmOpenRef)
												// RETURNS:
												//		zero on success; fileErr... on error;
												//		the DmOpenRef variable will be set to the file's open db reference
												//		that may be passed to Data Manager calls;
												// WARNING:
												//		Do not make any changes to the data of the underlying database --
												//		this will cause the file stream to become corrupted.

	fileOpFlush,							// flush any cached data to storage
												// ARGUMENTS:
												//		stream = open stream handle
												//		valueP = NULL
												//		valueLenP = NULL
												// RETURNS:
												//		zero on success; fileErr... on error;




	fileOpLAST								// ***ADD NEW OPERATIONS BEFORE THIS ENTRY***
												// ***  AND ALWAYS AFTER EXISTING ENTRIES ***
												// ***     FOR BACKWARD COMPATIBILITY     ***
	} FileOpEnum;


/************************************************************
 * File Stream procedures
 *************************************************************/
#ifdef __cplusplus
extern "C" {
#endif


// Open/create a file stream (name must all be valid -- non-null, non-empty)
// (errP is optional - set to NULL to ignore)
extern FileHand FileOpen(UInt16 cardNo, const Char * nameP, UInt32 type, UInt32 creator,
	UInt32 openMode, Err *errP)
							SYS_TRAP(sysTrapFileOpen);

// Close the file stream
extern Err FileClose(FileHand stream)
							SYS_TRAP(sysTrapFileClose);

// Delete a file
extern Err FileDelete(UInt16 cardNo, const Char *nameP)
							SYS_TRAP(sysTrapFileDelete);
							

/***********************************************************************
 *
 * MACRO:		FileRead
 *
 * DESCRIPTION:	Read data from a file into a buffer.  If you need to read into a data storage
 *						heap-based chunk, record or resource, you _must_ use FileDmRead instead.
 *
 * PROTOTYPE:	Int32 FileRead(FileHand stream, void *bufP, Int32 objSize, Int32 numObj, Err *errP)
 *
 * PARAMETERS:	stream		-- handle of open file
 *					bufP			-- buffer for reading data
 *					objSize		-- size of each object to read
 *					numObj		-- number of objects to read
 *					errP			-- ptr to variable for returning the error code (fileErr...)
 *										(OPTIONAL -- pass NULL to ignore)
 *
 * RETURNED:	the number of objects that were read - this may be less than
 *					the number of objects requested
 *
 ***********************************************************************/
#define FileRead(stream, bufP, objSize, numObj, errP)							\
	FileReadLow((stream), (bufP), 0/*offset*/, false/*dataStoreBased*/,	\
		(objSize), (numObj), (errP))
		
		
/***********************************************************************
 *
 * MACRO:		FileDmRead
 *
 * DESCRIPTION:	Read data from a file into a data storage heap-based chunk, record
 *						or resource.
 *
 * PROTOTYPE:	Int32 FileDmRead(FileHand stream, void *startOfDmChunkP, Int32 destOffset,
 *							Int32 objSize, Int32 numObj, Err *errP)
 *
 * PARAMETERS:	stream		-- handle of open file
 *					startOfDmChunkP
 *									-- ptr to beginning of data storage heap-based chunk, record or resource
 *					destOffset	-- offset from base ptr to the destination area (must be >= 0)
 *					objSize		-- size of each object to read
 *					numObj		-- number of objects to read
 *					errP			-- ptr to variable for returning the error code (fileErr...)
 *										(OPTIONAL -- pass NULL to ignore)
 *
 * RETURNED:	the number of objects that were read - this may be less than
 *					the number of objects requested
 *
 ***********************************************************************/
#define FileDmRead(stream, startOfDmChunkP, destOffset, objSize, numObj, errP)		\
	FileReadLow((stream), (startOfDmChunkP), (destOffset), true/*dataStoreBased*/,	\
		(objSize), (numObj), (errP))


// Low-level routine for reading data from a file stream -- use helper macros FileRead and FileDmRead
// instead of calling this function directly;
// (errP is optional - set to NULL to ignore)
extern Int32 FileReadLow(FileHand stream, void *baseP, Int32 offset, Boolean dataStoreBased, Int32 objSize,
	Int32 numObj, Err *errP)
							SYS_TRAP(sysTrapFileReadLow);

// Write data to a file stream
// (errP is optional - set to NULL to ignore)
extern Int32 FileWrite(FileHand stream, const void *dataP, Int32 objSize,
	Int32 numObj, Err *errP)
							SYS_TRAP(sysTrapFileWrite);

// Set position within a file stream
extern Err FileSeek(FileHand stream, Int32 offset, FileOriginEnum origin)
							SYS_TRAP(sysTrapFileSeek);

#define FileRewind(__stream__)		\
	(FileClearerr((__stream__)), FileSeek((__stream__), 0, fileOriginBeginning))

// Get current position and filesize
// (fileSizeP and errP are optional - set to NULL to ignore)
extern Int32 FileTell(FileHand stream, Int32 *fileSizeP, Err *errP)
							SYS_TRAP(sysTrapFileTell);

// Truncate a file
extern Err FileTruncate(FileHand stream, Int32 newSize)
							SYS_TRAP(sysTrapFileTruncate);

// Returns the error code from the last operation on this file stream;
// if resetLastError is non-zero, resets the error status
extern Err FileControl(FileOpEnum op, FileHand stream, void *valueP, Int32 *valueLenP)
							SYS_TRAP(sysTrapFileControl);

#define FileEOF(__stream__)				\
	(FileControl(fileOpGetEOFStatus, (__stream__), NULL, NULL) == fileErrEOF)

#define FileError(__stream__)				\
	FileControl(fileOpGetIOErrorStatus, (__stream__), NULL, NULL)

#define FileClearerr(__stream__)			\
	FileControl(fileOpClearError, (__stream__), NULL, NULL)

#define FileGetLastError(__stream__)	\
	FileControl(fileOpGetLastError, (__stream__), NULL, NULL)


#define FileFlush(__stream__)				\
	FileControl(fileOpFlush, (__stream__), NULL, NULL)



#ifdef __cplusplus
}
#endif


#endif //  #ifndef __FILESTREAM_H__
