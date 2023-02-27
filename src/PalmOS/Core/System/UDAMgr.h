/******************************************************************************
 *
 * Copyright (c) 1999-2003 PalmSource, Inc. All rights reserved.
 *
 * File: UDAMgr.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *              Unified Data Manager header file
 *  			Define type and generic macro to access data
 *
 *****************************************************************************/

#ifndef __UDAMGR_H__
#define __UDAMGR_H__

#include <PalmTypes.h>
#include <ErrorBase.h>
#include <ExgMgr.h>
#ifdef PALMOS
#include <unix_stdarg.h>
#define sys_va_list va_list
#endif

 /***********************************************************************
 * Generic options flags
 ************************************************************************/

#define kUDAEndOfReader 		((UInt16) 1)
#define kUDAMoreData    		((UInt16) 2)


 /***********************************************************************
 * Generic control 
 ************************************************************************/

#define	kUDAReinitialize		((UInt16) 1)

 /***********************************************************************
 * Generic error codes
 ************************************************************************/

#define udaErrControl			((Err) udaErrorClass | 1)

 /***********************************************************************
 * General types
 ************************************************************************/

typedef UInt16 UDABufferSize;

#define kUDAZeroTerminatedBuffer	0xFFFF

struct UDAObjectTag;
struct UDAReaderTag;
struct UDAFilterTag;
struct UDAWriterTag;

 /***********************************************************************
 * Types of callback functions
 ************************************************************************/

typedef void (*UDADeleteFunction) (struct UDAObjectTag** ioObject);
typedef Err (*UDAControlFunction) (struct UDAObjectTag* ioObject, UInt16 parameter, sys_va_list args);

typedef UDABufferSize (*UDAReadFunction) (struct UDAReaderTag* ioReader, UInt8* buffer, UDABufferSize bufferSize, Err* error);

typedef Err (*UDAWriteFunction)(struct UDAWriterTag* ioWriter);
typedef Err (*UDAFlushFunction)(struct UDAWriterTag* ioWriter);

typedef struct UDAObjectTag {
	UInt16			 	optionFlags;
	UDADeleteFunction   deleteF;
	UDAControlFunction 	controlF;
} UDAObjectType;

typedef struct UDAReaderTag {
	// The Reader is a base object
	UInt16			 	optionFlags;
	UDADeleteFunction   deleteF;
	UDAControlFunction 	controlF;
	
	// Specific Reader fields
	UDAReadFunction  	readF;
} UDAReaderType;

typedef struct UDAFilterTag {
	// The Filter is a base Object
	UInt16			 	optionFlags;
	UDADeleteFunction   deleteF;
	UDAControlFunction 	controlF;
	
	// The Filter is a Reader
	UDAReadFunction  	readF;
	
	// Specific Filter fields
	UDAReaderType* 		upperReader;
} UDAFilterType;

typedef struct UDAWriterTag {
	// The Writer is a base Object
	UInt16			 	optionFlags;
	UDADeleteFunction   deleteF;
	UDAControlFunction 	controlF;
	
	// Specific Writer fields
	UDAWriteFunction 	initiateWriteF;
	UDAFlushFunction 	flushF;
	UDAReaderType* 		upperReader;
} UDAWriterType;

 /***********************************************************************
 * Generic macro to access generic functions
 ************************************************************************/

#define UDADelete(ioObject) \
	((*(ioObject->deleteF))((UDAObjectType**)(&(ioObject))))

#define UDARead(ioReader, bufferToFillP, bufferSizeInBytes, error) \
	((*(ioReader->readF))((UDAReaderType*)(ioReader), (bufferToFillP), (bufferSizeInBytes), (error)))
	
#define UDAEndOfReader(ioReader) \
	(((ioReader)->optionFlags & kUDAEndOfReader) != 0)

#define UDAMoreData(ioReader) \
	(((ioReader)->optionFlags & kUDAMoreData) != 0)
	
#define UDAFilterJoin(ioFilter, ioReader) \
	(((UDAFilterType*)(ioFilter))->upperReader = ioReader)

#define UDAWriterJoin(ioWriter, ioReader) \
	(ioWriter->upperReader = ioReader)

#define UDAInitiateWrite(ioWriter) \
	((*(ioWriter)->initiateWriteF))(ioWriter)

#define UDAWriterFlush(ioWriter) \
	((*(ioWriter)->flushF))(ioWriter)
	

/*****************************************************************
 * UDA API
 ****************************************************************/

// For simulator builds, always direct calls
#ifndef BUILDING_UDA_MGR
#	if EMULATION_LEVEL != EMULATION_NONE 
#		define BUILDING_UDA_MGR	1		
#	endif
#endif

// When using UDAMgr, use systraps w/ selector
// When building UDAMgr, use direct calls
#ifndef BUILDING_UDA_MGR
#	define UDA_MGR_TRAP(udaSelectorNum) \
		_SYSTEM_API(_CALL_WITH_SELECTOR)(_SYSTEM_TABLE, sysTrapUdaMgrDispatch, udaSelectorNum)
#else
# 	define UDA_MGR_TRAP(udaSelectorNum)
#endif

// Public UDAMgr function selectors
#define sysUdaControl					0
#define sysUdaMemoryReaderNew			1
#define sysUdaExchangeReaderNew			11
#define sysUdaExchangeWriterNew			12


// UDAMgr function prototypes

#ifdef __cplusplus
extern "C" {
#endif

extern Err UDAControl(UDAObjectType* ioObject, UInt16 parameter, ...)
			UDA_MGR_TRAP(sysUdaControl);
	
extern UDAReaderType* UDAExchangeReaderNew(ExgSocketType* socket)
			UDA_MGR_TRAP(sysUdaExchangeReaderNew);

extern UDAWriterType* UDAExchangeWriterNew(ExgSocketType* socket, UDABufferSize bufferSize)
			UDA_MGR_TRAP(sysUdaExchangeWriterNew);

 /***********************************************************************
 * Memory reader
 ************************************************************************/

extern UDAReaderType* UDAMemoryReaderNew(const UInt8* bufferP, UDABufferSize bufferSizeInBytes)
	UDA_MGR_TRAP(sysUdaMemoryReaderNew);

#ifdef __cplusplus
}
#endif

#endif  // __UDAMGR_H__
