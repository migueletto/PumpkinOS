/*******************************************************************************
 * Copyright (c) 2004-2005 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 ******************************************************************************/
/**
 * @defgroup	Codec Codec Plugin Manager
 * @brief		This library is used to provide support for audio/video/image
 *				codecs plugin and encoding/decoding routines.
 *
 * The Codec Plugin Manager library is used to unify all different type of
 * codecs that are available for palmOne devices and provide a generic
 * API for developers to access these codecs for encoding/decoding.
 *
 * A typical application would use the library the following way:
 * - Check if an input/output format pair is supported [optional]
 * - Create a CodecSession with parameters that match the formats
 * - Call EncodeDecode()
 * - ...
 * - Delete the session
 *
 * For more information, please also check the palmOne Developer Guide.
 *
 * @{
 * @}
 */
/**
 * @ingroup Codec
 */

/**
 * @file 	palmOneCodecPluginMgrCommon.h
 * @version 3.0
 *
 * @brief	Public 68K common header file for Codec Plugin Manager API - Library Definitions.
 *
 * This file contains the library constants and error codes used in the APIs.
 * <hr>
 */

#ifndef _PALMONECODECPLUGINMGRCOMMON_H_
#define _PALMONECODECPLUGINMGRCOMMON_H_

#include <PalmTypes.h>
#include <ErrorBase.h>
#include <SystemMgr.h>


/***********************************************************************
 * Type and creator of the Library
 ***********************************************************************/

#define	kCodecMgrLibName		"CodecPluginMgr"	/**< CodecPluginMgr library name. */
#define	kCodecMgrLibType		'aext'				/**< CodecPluginMgr library type. */
#define	kCodecMgrLibCreator		'CdMg'				/**< CodecPluginMgr creator ID. */


/***********************************************************************
 * Library versioning
 ***********************************************************************/

/**
 * @name Library Versions
 */
/*@{*/
#define	kCodecMgrLibVersion3	sysMakeROMVersion(3, 0, 0, sysROMStageRelease, 0)
#define	kCodecMgrLibVersion		kCodecMgrLibVersion3
/*@}*/

/***********************************************************************
 * Error codes
 ***********************************************************************/

/** Codec Plug-In Manager error codes. */
#define kCodecMgrLibErrorClass		(oemErrorClass + 0x100)

/**
 * @name Library Error Codes
 */
/*@{*/
/** Returned from CodecMgrLibClose() if the library is still open. */
#define kCodecMgrLibErrStillOpen	(kCodecMgrLibErrorClass | 0x01)
/** Internal error. */
#define	kCodecMgrLibErrInternal		(kCodecMgrLibErrorClass | 0x02)
/** Bad parameters. */
#define	kCodecMgrLibErrBadParam		(kCodecMgrLibErrorClass | 0x03)
/** API or Codec not supported. */
#define	kCodecMgrLibNotSupported	(kCodecMgrLibErrorClass | 0x04)
/** Bad Codec Manager version. */
#define kCodecMgrLibBadVersion		(kCodecMgrLibErrorClass | 0x05)
/** No codec available. */
#define kCodecMgrLibNoCodec			(kCodecMgrLibErrorClass | 0x06)
/*@}*/

/** Codec Plugins error codes. */
#define kCodecPluginErrorClass		(oemErrorClass + 0x300)

/**
 * @name Codec Session Error Codes
 */
/*@{*/
/** Codec Plug-In Bad parameters. */
#define	kCodecErrBadParam			(kCodecMgrLibErrorClass | 0x01)
/** Buffer Over-run. */
#define kCodecErrBufferOverrun		(kCodecPluginErrorClass | 0x02)
/** Buffer Under-run. */
#define kCodecErrBufferUnderrun		(kCodecPluginErrorClass | 0x03)
/** Frame Error. */
#define kCodecErrFrameError			(kCodecPluginErrorClass | 0x04)
/** Invalid Header. */
#define kCodecErrInvalidHeader		(kCodecPluginErrorClass | 0x05)
/** Sync not found. */
#define kCodecSyncNotFound			(kCodecPluginErrorClass | 0x06)
/** Invalide decoding/encoding state. */
#define kCodecErrInvalidState		(kCodecPluginErrorClass | 0x07)
/** Alignment error. */
#define kCodecErrAlignment			(kCodecPluginErrorClass | 0x08)
/** Parsing error. */
#define kCodecErrParsing			(kCodecPluginErrorClass | 0x09)
/** Out of memory. */
#define kCodecErrNoMemory			(kCodecPluginErrorClass | 0x10)
/** Unknown error. */
#define kCodecErrUnknown			(kCodecPluginErrorClass | 0xFF)
/*@}*/

/******************************************************************
 * Constants and Types
 ******************************************************************/

/**
 * @name CodecSupportedFormatEnumerate Iterator
 */
/*@{*/
#define palmCodecIteratorStart		(0xfffffffeL)	/**< Start iterator. */
#define palmCodecIteratorStop		(0xffffffffL)	/**< Stop iterator.  */
/*@}*/

#define	palmCodecNullID				(0)				/**< Null ID. */
#define palmCodecInvalidSession		(NULL)			/**< Invalid session. */

/** Palm Codec Session. */
typedef void	*PalmCodecSession;

/** Palm Codec Format. */
typedef UInt32  PalmCodecFormat;

/***********************************************************************
 * Library trap
 ***********************************************************************/

/**
 * @name Function Traps
 */
/*@{*/
#define kCodecMgrLibTrapLibAPIVersion			(sysLibTrapCustom + 0)
#define kCodecMgrLibTrapCreateSession			(sysLibTrapCustom + 1)
#define kCodecMgrLibTrapCreateSessionByID		(sysLibTrapCustom + 2)
#define kCodecMgrLibTrapDeleteSession			(sysLibTrapCustom + 3)
#define kCodecMgrLibTrapResetSession			(sysLibTrapCustom + 4)
#define kCodecMgrLibTrapGetMaxDestBufferSize	(sysLibTrapCustom + 5)
#define kCodecMgrLibTrapEncodeDecode			(sysLibTrapCustom + 6)
#define kCodecMgrLibTrapCustomControl			(sysLibTrapCustom + 7)
#define kCodecMgrLibTrapEnumerateFormats		(sysLibTrapCustom + 8)
#define kCodecMgrLibTrapIsFormatSupported		(sysLibTrapCustom + 9)
#define kCodecMgrLibTrapGetSessionInfo			(sysLibTrapCustom + 10)
/*@}*/

#endif  // _PALMONECODECPLUGINMGRCOMMON_H_
