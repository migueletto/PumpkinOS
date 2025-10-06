/******************************************************************************
 * Copyright (c) 2005 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/** 
 *@ingroup System
 *
 */
 
/**
 * @file 	PalmNative.h
 * @version 1.0
 * @date 	10/28/2002
 *
 * @brief Functions and macros to help building ARM code.
 * 
 *
 * <hr>
 */


#ifndef PALM_NATIVE_H_
#define PALM_NATIVE_H_

#ifdef WIN32
#define NATIVE_CODE __declspec(dllexport)	/**< 		*/
#define INLINE					/**< 		*/
#else
	
#define NATIVE_CODE				/**< 		*/
#define INLINE      inline			/**< 		*/

#endif // WIN32


#ifndef __PALMTYPES_H__
/**
 * @name 
 *
 */
/*@{*/
#define false  0				/**< 		*/
#define true   1				/**< 		*/
/*@}*/

#ifndef NULL
#define NULL	0				/**< 		*/
#endif	// NULL

typedef unsigned char   UInt8;			/**< 		*/
typedef unsigned short  UInt16;			/**< 		*/
typedef unsigned long   UInt32;			/**< 		*/

typedef signed char     Int8;			/**< 		*/
typedef signed short    Int16;			/**< 		*/
typedef signed long     Int32;			/**< 		*/
typedef signed char     Char;			/**< 		*/

typedef unsigned char   Boolean;		/**< 		*/
typedef UInt16          Err;			/**< 		*/

static INLINE UInt16 SwapUInt16(UInt16 u)
{
  return
    (u & 0x00FF) << 8 |
    (u >> 8);
}

static INLINE Int16 SwapInt16(Int16 i)
{
  return (Int16)SwapUInt16((UInt16) i);
}

static INLINE UInt32 SwapUInt32(UInt32 u)
{
  return
    (u & 0x0000FFL) << 24 |
    (u & 0x00FF00L) << 8  |
    (u & 0xFF0000L) >> 8  |
    (u >> 24) & 0xFF;
}

static INLINE Int32 SwapInt32(Int32 i)
{
  return (Int32)SwapInt32((UInt32)i);
}

static INLINE void* SwapPtr(void* p)
{
  return (void*)SwapUInt32((UInt32)p);
}

#endif	// __PALMTYPES_H__

#endif // PALM_NATIVE_H_
