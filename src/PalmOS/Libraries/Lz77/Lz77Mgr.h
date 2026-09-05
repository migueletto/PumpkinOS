/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: Lz77Mgr.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 *****************************************************************************/

#ifndef 	__LZ77MGR_H__
#define	__LZ77MGR_H__

#include <ErrorMgr.h>

//
// Common PalmOS and Windows section
//

#define	Lz77VerID					1
#define	Lz77LastSupportedVerID	1

#define	lz77Compress				true
#define	lz77Expand					false

typedef	Err	Lz77ErrorType ;

/********************************************************************
 * Error codes
 ********************************************************************/
#define			lz77Success											0x00
//				Non Fatal Errors
#define			lz77ErrNonFatalFirstErr							(lz77ErrorClass | 0x00)
#define			lz77ErrNonFatalInputBufferIncomplete		(lz77ErrorClass | 0x01)
#define			lz77ErrNonFatalOutputBufferFull				(lz77ErrorClass | 0x02)
#define			lz77ErrNonFatalLastErr							(lz77ErrorClass | 0x7F)
//				Fatal Errors
#define			lz77ErrFatalFirstErr								(lz77ErrorClass | 0x80)
#define			lz77ErrFatalUnfinishedInputBuffer			(lz77ErrorClass | 0x80)
#define			lz77ErrFatalInputBufferIncomplete			(lz77ErrorClass | 0x81)
#define			lz77ErrFatalInputBufferInvalid				(lz77ErrorClass | 0x82)
#define			lz77ErrFatalMemAllocation						(lz77ErrorClass | 0x83)
#define			lz77ErrFatalHandleInvalid						(lz77ErrorClass | 0x84)
#define			lz77ErrFatalCantChangeToCompress				(lz77ErrorClass | 0x85)
#define			lz77ErrFatalUnknownVersion						(lz77ErrorClass | 0x86)
#define			lz77ErrFatalOutputBufferTooSmall				(lz77ErrorClass | 0x87)
#define			lz77ErrFatalInvalidArgument					(lz77ErrorClass | 0x88)
#define			lz77ErrFatalLastErr								(lz77ErrorClass | 0xFF)

#define			lz77ErrIsFatal( err )	\
	((err !=lz77Success) && ((err <lz77ErrNonFatalFirstErr) || (err >lz77ErrNonFatalLastErr)))

#ifdef	_WIN32

#else		// PalmOS
//
// Specific PalmOS section
//

#include <PalmTypes.h>
#include <LibTraps.h>
#include <SystemResources.h>

#pragma mark Constants

// Creator. Used for both the database that contains the LZ77 Library and
//  it's features for the feature manager. 
#define	lz77Creator				sysFileCLz77Lib		// Lz77 Library creator
#define	lz77LibName				"Lz77.lib"				// pass in to SysLibFind()

/********************************************************************
 * LZ77 Library functions. 
 ********************************************************************/

#define lz77LibTrapChunk			(sysLibTrapCustom)
#define lz77LibTrapMaxBufferSize	(sysLibTrapCustom+1)
#define lz77LibTrapBufferGetInfo	(sysLibTrapCustom+2)
#define lz77LibTrapBufferSetInfo	(sysLibTrapCustom+3)

#ifdef __cplusplus
extern "C" {
#endif

#if			EMULATION_LEVEL == EMULATION_NONE
#else	// EMULATION_LEVEL == EMULATION_NONE
	// The following functions are Traps on Palm OS
	//	and functions on the Simulator and on Windows Servers
	#define	Lz77LibOpen					Lz77Open
	#define	Lz77LibClose				Lz77Close
	#define	Lz77LibChunk				Lz77Chunk
	#define	Lz77LibMaxBufferSize		Lz77MaxBufferSize
	#define	Lz77LibBufferGetInfo		Lz77BufferGetInfo
	#define	Lz77LibBufferSetInfo		Lz77BufferSetInfo
#endif	// EMULATION_LEVEL == EMULATION_NONE

//--------------------------------------------------
// Library initialization, shutdown, sleep and wake
//--------------------------------------------------
Err		Lz77LibOpen(
	UInt16 		libRefnum,				// Palm OS reference calling number
	MemHandle*		lz77HandleP,		// <-  Pointer to returning LZ77 handle (NULL for error)
	Boolean			compressFlag,		// ->  TRUE = Compress; FALSE = Expand
	UInt32			sourceSize,			// ->  Source size in bytes
	MemHandle*		destHP,				// <-> If (*destHP != NULL) => use pre allocated memory
												//		 (*destHP and *destSizeP)
												//     If (*destHP == NULL) => allocate memory in *destHP
	UInt32 *			destSizeP,			// <-> If (*destSizeP ==0) THEN *destP must be NULL
												//     => Lz77Open will calculate maximum buffer size
												//		 based on compressFlag and sourceSize
												//		 If (*destSizeP !=0) THEN it indicate
												//		 the size in bytes of the destination buffer
	UInt16			useVerNum,			// ->  if (useVerNum !=0) THEN Use Version numbering
												//     (Compress will write the value useVerNum in the
												//		  output buffer Expand will verify if the Version
												//		  in the source buffer is compatible)
	UInt8 *			primerP,				// ->  if (compressFlag ==lz77Compress)
												//				UncompressPrimer buffer pointer
												//		 else CompressPrimer buffer pointer
												//				Must be valid compressed lz77 data
												//				compressed without a primer.
												//		 NULL means no primer
	UInt32			primerL,				// ->  Byte length of primer
	UInt32			processedPrimerL)	// ->  Byte length of processed primer
	//	Note: The output buffer must be large enough to include the emtire processed primer.
	//			When Expanding, the compressed primer is passed to the Open routine and
	//			the output buffer must be large enough to contain the expanded primer.
	SYS_TRAP(sysLibTrapOpen);

Err		Lz77LibClose(
	UInt16 			libRefnum,			// Palm OS reference calling number
	MemHandle		lz77Handle,			// ->  Lz77 Handle
	UInt32 *			ResultingSizeP )	// <-  Size in bytes of output generated buffer
												// Output buffer will be resized to the resulting size
												// if Lz77Open have allocated the output buffer.
												// Output buffer must be free by the calling application
	SYS_TRAP(sysLibTrapClose);

Err		Lz77LibSleep( UInt16 libRefnum)
	SYS_TRAP(sysLibTrapSleep);
					
Err		Lz77LibWake( UInt16 libRefnum)
	SYS_TRAP(sysLibTrapWake);

Err		Lz77LibChunk(
	UInt16 			libRefnum,					// Palm OS reference calling number
	MemHandle		lz77Handle,					// ->  Lz77 Handle
	UInt8 *			sourceP,						// ->  Source buffer pointer
	UInt32			sourceSize,					// ->  Source buffer Size (bytes)
	UInt32 *			sourceBitReadOffset )	// <-> Next bit to read from source
	SYS_TRAP(lz77LibTrapChunk);
			
Err		Lz77LibMaxBufferSize(
	UInt16 			libRefnum,			// Palm OS reference calling number
	Boolean			compressFlag,		// -> TRUE = Compress; FALSE = Expand
	UInt32			sourceSize,			//	-> Size of Source buffer
	UInt32*			maxBufferSizeP )	// <- result size pointer
	SYS_TRAP(lz77LibTrapMaxBufferSize);

Err		Lz77LibBufferGetInfo(
	UInt16 			libRefnum,			// Palm OS reference calling number
	MemHandle		lz77Handle,			// ->  Lz77 Handle
	Boolean *		compressFlagP,		// <-  Get compressFlag (true = compress mode; false = expand mode)
	MemHandle*		bufferHP,			// <-  Get the Pointer to the accumulated destination buffer
	UInt32 *			bufferByteSizeP,	// <-  Get destination buffer size in bytes
	UInt32 *			destBitOffsetP )	// <-  Get destination bit offset
	SYS_TRAP(lz77LibTrapBufferGetInfo);

Err		Lz77LibBufferSetInfo(
	UInt16 			libRefnum,			// Palm OS reference calling number
	MemHandle		lz77Handle,			// ->  Lz77 Handle
	Boolean			compressFlag,		// ->  Set compressFlag (true = compress mode; false = expand mode)
	MemHandle		destH,				// ->  Set a Pointer to the accumulated destination buffer
	UInt32			destByteSize,		// ->  Set destination buffer size in bytes
	UInt32			destBitOffset )	// ->  Set destination bit offset
	SYS_TRAP(lz77LibTrapBufferSetInfo);

#ifdef __cplusplus 
}
#endif

#endif	//	_WIN32

#endif 	//__LZ77LIB_H__
