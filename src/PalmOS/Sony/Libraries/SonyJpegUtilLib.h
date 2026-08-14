/******************************************************************************
 *                                                                            *
 *            (C) Copyright 2000-2002, Sony Corporation                       *
 *                                                                            *
 *----------------------------------------------------------------------------*
 *                                                                            *
 *    <IDENTIFICATION>                                                        *
 *       file name    : $Workfile: SonyJpegUtilLib.h $
 *                                                                            *
 *    <PROFILE>                                                               *
 *       Jpeg Util library prototype                                          *
 *                                                                            *
 *    <HISTORY>                                                               *
 *       Started on   : 01/10/20                                              *
 *       Last Modified: $Date: 03/06/17 17:21 $ 
 *                                                                            *
 ******************************************************************************/
#ifndef __JPEG_UTIL_LIB_H__
#define __JPEG_UTIL_LIB_H__

#include <Bitmap.h>
#include <VFSMgr.h>
#include <Progress.h>
#include <SonyErrorBase.h>

#if CPU_TYPE != CPU_68K
//#include <LibTraps68K.h>
#endif /* CPU_TYPE != CPU_68K */

/****************************************************************************
 * If we're actually compiling the library code, then we need to
 * eliminate the trap glue that would otherwise be generated from
 * this header file in order to prevent compiler errors in CW Pro 2.
 ***************************************************************************/
#if BUILDING_JPEG_UTIL_APP_OR_LIB
        #define JPEG_UTIL_LIB_TRAP(trapNum)
#else
        #define JPEG_UTIL_LIB_TRAP(trapNum) SYS_TRAP(trapNum)
#endif

/***************************************************************************
 * version number
 ***************************************************************************/
#define jpegUtilLibAPIVersion	(2)

/***************************************************************************
 * Error codes
 ***************************************************************************/
#define jpegUtilLibErrorBase					(0x00)
#define jpegUtilLibErrorClass					(sonyJpegErrorClass | jpegUtilLibErrorBase)

#define jpegUtilLibErrNone						(errNone)
#define jpegUtilLibErrBadParam				(jpegUtilLibErrorClass | 1)  // invalid parameter
#define jpegUtilLibErrNotOpen					(jpegUtilLibErrorClass | 2)  // library is not open
#define jpegUtilLibErrStillOpen				(jpegUtilLibErrorClass | 3)  // library still in used
#define jpegUtilLibErrNoMemory				(jpegUtilLibErrorClass | 4)
#define jpegUtilLibErrNotSupported			(jpegUtilLibErrorClass | 5)
#define jpegUtilLibErrNotJpegFormat			(jpegUtilLibErrorClass | 6)
#define jpegUtilLibErrNotExifFormat			(jpegUtilLibErrorClass | 7)
#define jpegUtilLibErrExifParamNotFound	(jpegUtilLibErrorClass | 8)
#define jpegUtilLibErrEncDecCanceled		(jpegUtilLibErrorClass | 9)
#define jpegUtilLibErrResourceBusy			(jpegUtilLibErrorClass | 10)

/***************************************************************************
 * Definition
 ***************************************************************************/
#define Incapable					(0)
#define Capable					(1)

/******************************************************************************
 *      Structure                                                             *
 ******************************************************************************/
typedef Boolean (*JpegPrgCallbackFunc)  (void *prgCbData);

typedef struct
{
	UInt16					percent;			// progress
	ProgressPtr				prgP;				// in case of using ProgressMgr
	JpegPrgCallbackFunc	prgCbFunc;		// callback function in the case not using ProgressMgr
	void						*prgCbData;
} PrgInfoType, *PrgInfoP;

typedef struct
{
	UInt32	numerator;
	UInt32	denominator;	
} RationalType;

/* capability */
typedef struct {
	UInt16 version:1;
	UInt16 latitudeRef:1;
	UInt16 latitude:1;
	UInt16 longitudeRef:1;
	UInt16 longitude:1;
	UInt16 altitudeRef:1;
	UInt16 altitude:1;
	UInt16 mapDatum:1;
	UInt16 reserved:8;
} GPSInfoCapabilityType;

/* capability */
typedef struct {
	UInt16 softName:1;
	UInt16 gpsInfo:1;
	UInt16 orientation:1;	// available ver.2.0
	UInt16 reserved:13;
} JpegDetailInfoCapabilityType;

typedef struct
{
	GPSInfoCapabilityType	gpsInfoCapability;
	Char							version[4];
	Char							latitudeRef[2];
	RationalType				latitude[3];
	Char							longitudeRef[2];
	RationalType				longitude[3];
	Char							altitudeRef;
	RationalType				altitude;
	Char							*mapDatum;
} GPSInfoType, *GPSInfoP;



typedef struct
{
	JpegDetailInfoCapabilityType	jpegDetailInfoCapability;	
	Char							dateTime[20];	/* ex) 2001:10:23 21:04:13 */
	Char							*softName;
	GPSInfoP 					gpsInfoP;
	// available ver.2.0
	Char							dateTimeOriginal[20];	/* ex) 2001:10:23 21:04:13 */
	Char							dateTimeDigitized[20];	/* ex) 2001:10:23 21:04:13 */
	UInt16						orientation;
	//JpegImageOrientation			orientation;	//add by tashiro 02/05/24
} JpegDetailInfoType, *JpegDetailInfoP;

/***************************************************************************
 * Enum
 ***************************************************************************/
typedef enum {
	jpegDecRatioNormal = 0,
	jpegDecRatioHalf,
	jpegDecRatioQuarter,
	jpegDecRatioOctant
} JpegImageRatio;

typedef enum {
	jpegDecModeNormal = 0,
	jpegDecModeThumbnail
} JpegImageType;

// available ver.2.0
typedef enum {
	JpegImageOriNotSupported = 0,
	JpegImageOriNormal,
	JpegImageOriNormalR,		//notsupported
	JpegImageOriCW180,
	JpegImageOriCW180R,		//notsupported
	JpegImageOriCW90R,		//notsupported
	JpegImageOriCW90,
	JpegImageOriCCW90R,		//notsupported
	JpegImageOriCCW90
} JpegImageOrientation;

/***************************************************************************
 * Macro
 ***************************************************************************/

#define FreeJpegDetailInfo(jpegDetailInfoP)	\
{	\
	if((jpegDetailInfoP)->jpegDetailInfoCapability.softName) {	\
		MemPtrFree((jpegDetailInfoP)->softName);	\
	}	\
	if((jpegDetailInfoP)->jpegDetailInfoCapability.gpsInfo) {	\
		if((jpegDetailInfoP)->gpsInfoP->gpsInfoCapability.mapDatum) {	\
			MemPtrFree((jpegDetailInfoP)->gpsInfoP->mapDatum);	\
		}	\
		MemPtrFree((jpegDetailInfoP)->gpsInfoP);	\
	}	\
}

/***************************************************************************
 * Jpeg Util library function trap ID's. Each library call gets a trap number:
 *   JpegLibTrapXXXX which serves as an index into the library's dispatch
 *   table. The constant sysLibTrapCustom is the first available trap number
 *   after the system predefined library traps Open,Close,Sleep & Wake.
 *
 * WARNING!!! The order of these traps MUST match the order of the dispatch
 *  table in LibDispatch.c!!!
 ****************************************************************************/
typedef enum {
	jpegUtilLibTrapGetLibAPIVersion = sysLibTrapCustom,

	jpegUtilLibTrapDecodeImageToBmp,					/* jpegUtilLibDecodeImageToBmp */
	jpegUtilLibTrapDecodeImageToWindow,				/* jpegUtilLibDecodeImageToWindow */
	jpegUtilLibTrapEncodeImageFromBmp,				/* jpegUtilLibEncodeImageFromBmp */
	jpegUtilLibTrapEncodeImageFromWindow,			/* jpegUtilLibEncodeImageFromWindow */
	jpegUtilLibTrapEncodeImageFromPGP,				/* jpegUtilLibEncodeImageFromPGP */
	jpegUtilLibTrapGetJpegInfo,						/* jpegUtilLibGetJpegInfo */
	// available ver.2.0
	jpegUtilLibTrapDecodeImageToBmpForFS,			/* jpegUtilLibDecodeImageToBmpForFS */
	jpegUtilLibTrapDecodeImageToWindowForFS,		/* jpegUtilLibDecodeImageToWindowForFS */
	jpegUtilLibTrapEncodeImageFromBmpForFS,		/* jpegUtilLibEncodeImageFromBmpForFS */
	jpegUtilLibTrapEncodeImageFromWindowForFS,	/* jpegUtilLibEncodeImageFromWindowForFS */
	jpegUtilLibTrapEncodeImageFromPGPForFS,		/* jpegUtilLibEncodeImageFromPGPForFS */
	jpegUtilLibTrapGetJpegInfoForFS,					/* jpegUtilLibGetJpegInfoForFS */
	jpegUtilLibTrapSetJpegOrientation,				/* jpegUtilLibSetJpegOrientation */
	jpegUtilLibTrapSetJpegOrientationForFS,		/* jpegUtilLibSetJpegOrientationForFS */
	jpegUtilLibTrapLast
} jpegUtilLibTrapNumberEnum;

/********************************************************************
 *              Library API Prototypes
 ********************************************************************/

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * Standard library open, close, sleep and wake functions
 *-------------------------------------------------------------------------*/

#if CPU_TYPE == CPU_68K

/* open the library */
extern Err jpegUtilLibOpen(UInt16 jpegUtilLibRefNum)
					JPEG_UTIL_LIB_TRAP(sysLibTrapOpen);

/* close the library */
extern Err jpegUtilLibClose(UInt16 jpegUtilLibRefNum)
					JPEG_UTIL_LIB_TRAP(sysLibTrapClose);

/* library sleep */
extern Err jpegUtilLibSleep(UInt16 jpegUtilLibRefNum)
					JPEG_UTIL_LIB_TRAP(sysLibTrapSleep);

/* library wakeup */
extern Err jpegUtilLibWake(UInt16 jpegUtilLibRefNum)
					JPEG_UTIL_LIB_TRAP(sysLibTrapWake);

/*--------------------------------------------------------------------------
 * Custom library API functions
 *--------------------------------------------------------------------------*/

/* Get our library API version */
extern UInt32 jpegUtilLibGetLibAPIVersion(UInt16 jpegUtilLibRefNum)
					JPEG_UTIL_LIB_TRAP(jpegUtilLibTrapGetLibAPIVersion);

extern Err jpegUtilLibDecodeImageToBmp(UInt16 jpegLibRefNum, FileRef fileRef,
														MemPtr inBufP, JpegImageType imageType,
														JpegImageRatio ratio, BitmapPtr *bitmapPP,
														PrgInfoP prgInfoP)
					JPEG_UTIL_LIB_TRAP(jpegUtilLibTrapDecodeImageToBmp);

extern Err jpegUtilLibDecodeImageToWindow(UInt16 jpegLibRefNum, FileRef fileRef,
														MemPtr inBufP, JpegImageType imageType,
														RectangleType *rP, PrgInfoP prgInfoP)
					JPEG_UTIL_LIB_TRAP(jpegUtilLibTrapDecodeImageToWindow);

extern Err jpegUtilLibEncodeImageFromBmp(UInt16 jpegLibRefNum, Boolean isExif,
														Char *dateTimeOriginal, Char *softName, GPSInfoP gpsInfoP, 
														UInt8 quality, BitmapPtr bitmapP,
														FileRef fileRef, MemPtr *outBufP, UInt32 *outBufSize,
														PrgInfoP prgInfoP)
					JPEG_UTIL_LIB_TRAP(jpegUtilLibTrapEncodeImageFromBmp);

extern Err jpegUtilLibEncodeImageFromWindow(UInt16 jpegLibRefNum, Boolean isExif,
															Char *dateTimeOriginal, Char *softName, GPSInfoP gpsInfoP,
															UInt8 quality, RectangleType *rP,
															FileRef fileRef, MemPtr *outBufP, UInt32 *outBufSize,
															PrgInfoP prgInfoP)
					JPEG_UTIL_LIB_TRAP(jpegUtilLibTrapEncodeImageFromWindow);

extern Err jpegUtilLibEncodeImageFromPGP(UInt16 jpegLibRefNum, Boolean isExif,
														Char *softName, GPSInfoP gpsInfoP,
														UInt8 quality, DmOpenRef dRef, FileRef inFileRef,
														FileRef outFileRef, MemPtr *outBufP, UInt32 *outBufSize,
														PrgInfoP prgInfoP)
					JPEG_UTIL_LIB_TRAP(jpegUtilLibTrapEncodeImageFromPGP);

extern Err jpegUtilLibGetJpegInfo(UInt16 jpegLibRefNum, FileRef fileRef,
												MemPtr inBufP, UInt32 *imgHeight, UInt32 *imgWidth, 
												Boolean *isThumbnail, JpegDetailInfoP jpegDetailInfoP)
					JPEG_UTIL_LIB_TRAP(jpegUtilLibTrapGetJpegInfo);

// available ver.2.0
extern Err jpegUtilLibDecodeImageToBmpForFS(UInt16 jpegLibRefNum, FileHand stream,
														JpegImageType imageType,
														JpegImageRatio ratio, BitmapPtr *bitmapPP,
														PrgInfoP prgInfoP)
					JPEG_UTIL_LIB_TRAP(jpegUtilLibTrapDecodeImageToBmpForFS);

extern Err jpegUtilLibDecodeImageToWindowForFS(UInt16 jpegLibRefNum, FileHand stream,
														JpegImageType imageType,
														RectangleType *rP, PrgInfoP prgInfoP)
					JPEG_UTIL_LIB_TRAP(jpegUtilLibTrapDecodeImageToWindowForFS);

extern Err jpegUtilLibEncodeImageFromBmpForFS(UInt16 jpegLibRefNum, Boolean isExif,
														Char *dateTimeOriginal, Char *softName, GPSInfoP gpsInfoP, 
														UInt8 quality, BitmapPtr bitmapP,
														FileHand stream, PrgInfoP prgInfoP)
					JPEG_UTIL_LIB_TRAP(jpegUtilLibTrapEncodeImageFromBmpForFS);

extern Err jpegUtilLibEncodeImageFromWindowForFS(UInt16 jpegLibRefNum, Boolean isExif,
															Char *dateTimeOriginal, Char *softName, GPSInfoP gpsInfoP,
															UInt8 quality, RectangleType *rP,
															FileHand stream, PrgInfoP prgInfoP)
					JPEG_UTIL_LIB_TRAP(jpegUtilLibTrapEncodeImageFromWindowForFS);

extern Err jpegUtilLibEncodeImageFromPGPForFS(UInt16 jpegLibRefNum, Boolean isExif,
														Char *softName, GPSInfoP gpsInfoP,
														UInt8 quality, DmOpenRef dRef, FileRef inFileRef,
														FileHand stream, PrgInfoP prgInfoP)
					JPEG_UTIL_LIB_TRAP(jpegUtilLibTrapEncodeImageFromPGPForFS);

extern Err jpegUtilLibGetJpegInfoForFS(UInt16 jpegLibRefNum, FileHand stream,
												UInt32 *imgHeight, UInt32 *imgWidth, 
												Boolean *isThumbnail, JpegDetailInfoP jpegDetailInfoP)
					JPEG_UTIL_LIB_TRAP(jpegUtilLibTrapGetJpegInfoForFS);

extern Err jpegUtilLibSetJpegOrientation(UInt16 jpegLibRefNum, FileRef fileRef,
												MemPtr inBufP, JpegImageOrientation orientation)
					JPEG_UTIL_LIB_TRAP(jpegUtilLibTrapSetJpegOrientation);

extern Err jpegUtilLibSetJpegOrientationForFS(UInt16 jpegLibRefNum, FileHand stream,
												JpegImageOrientation orientation)
					JPEG_UTIL_LIB_TRAP(jpegUtilLibTrapSetJpegOrientationForFS);

#else /* CPU_TYPE != CPU_68K */

/* open the library */
extern Err jpegUtilLibOpen(void);

/* close the library */
extern Err jpegUtilLibClose(void);

/* library sleep */
extern Err jpegUtilLibSleep(void);

/* library wakeup */
extern Err jpegUtilLibWake(void);

/*--------------------------------------------------------------------------
 * Custom library API functions
 *--------------------------------------------------------------------------*/

/* Get our library API version */
extern UInt32 jpegUtilLibGetLibAPIVersion(void);

extern Err jpegUtilLibDecodeImageToBmp(FileRef fileRef, MemPtr inBufP, JpegImageType imageType,
													JpegImageRatio ratio, BitmapPtr *bitmapPP,
													PrgInfoP prgInfoP);

extern Err jpegUtilLibDecodeImageToWindow(FileRef fileRef, MemPtr inBufP, JpegImageType imageType,
														RectangleType *rP, PrgInfoP prgInfoP);

extern Err jpegUtilLibEncodeImageFromBmp(Boolean isExif, Char *dateTimeOriginal,
														Char *softName, GPSInfoP gpsInfoP, 
														UInt8 quality, BitmapPtr bitmapP,
														FileRef fileRef, MemPtr *outBufP, UInt32 *outBufSize,
														PrgInfoP prgInfoP);

extern Err jpegUtilLibEncodeImageFromWindow(Boolean isExif, Char *dateTimeOriginal,
															Char *softName, GPSInfoP gpsInfoP,
															UInt8 quality, RectangleType *rP,
															FileRef fileRef, MemPtr *outBufP, UInt32 *outBufSize,
															PrgInfoP prgInfoP);

extern Err jpegUtilLibEncodeImageFromPGP(Boolean isExif,
														Char *softName, GPSInfoP gpsInfoP,
														UInt8 quality, DmOpenRef dRef, FileRef inFileRef,
														FileRef outFileRef, MemPtr *outBufP, UInt32 *outBufSize,
														PrgInfoP prgInfoP);

extern Err jpegUtilLibGetJpegInfo(FileRef fileRef, MemPtr inBufP,
												UInt32 *imgHeight, UInt32 *imgWidth, 
												Boolean *isThumbnail, JpegDetailInfoP jpegDetailInfoP);

// available ver.2.0
extern Err jpegUtilLibDecodeImageToBmpForFS(FileHand stream, JpegImageType imageType,
															JpegImageRatio ratio, BitmapPtr *bitmapPP,
															PrgInfoP prgInfoP);

extern Err jpegUtilLibDecodeImageToWindowForFS(FileHand stream, JpegImageType imageType,
																RectangleType *rP, PrgInfoP prgInfoP);

extern Err jpegUtilLibEncodeImageFromBmpForFS(Boolean isExif, Char *dateTimeOriginal,
															Char *softName, GPSInfoP gpsInfoP, 
															UInt8 quality, BitmapPtr bitmapP,
															FileHand stream, PrgInfoP prgInfoP);

extern Err jpegUtilLibEncodeImageFromWindowForFS(Boolean isExif, Char *dateTimeOriginal, 
																Char *softName, GPSInfoP gpsInfoP,
																UInt8 quality, RectangleType *rP,
																FileHand stream, PrgInfoP prgInfoP);

extern Err jpegUtilLibEncodeImageFromPGPForFS(Boolean isExif,
															Char *softName, GPSInfoP gpsInfoP,
															UInt8 quality, DmOpenRef dRef, FileRef inFileRef,
															FileHand stream, PrgInfoP prgInfoP);

extern Err jpegUtilLibGetJpegInfoForFS(FileHand stream, UInt32 *imgHeight, UInt32 *imgWidth, 
													Boolean *isThumbnail, JpegDetailInfoP jpegDetailInfoP);

extern Err jpegUtilLibSetJpegOrientation(FileRef fileRef, MemPtr inBufP,
														JpegImageOrientation orientation);

extern Err jpegUtilLibSetJpegOrientationForFS(FileHand stream,	JpegImageOrientation orientation);

#endif /* CPU_TYPE == CPU_68K */


#ifdef __cplusplus
}
#endif

#endif	// __JPEG_UTIL_LIB_H__

