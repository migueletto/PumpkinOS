/******************************************************************************
 *
 * Copyright (c) 1997-2003 PalmSource, Inc. All rights reserved.
 *
 * File: NetBitUtils.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *	  Header file for bit manipulation routines used primarily
 *	by wireless network protocols.
 *	
 *	  These routines live in the NetLib but are broken out here into
 *	a separate header so that they can be more easily used by source
 *	files that don't need access to the other NetLib functions. 
 *
 *****************************************************************************/

#ifndef	__NETBITUTILS_H__
#define	__NETBITUTILS_H__

#include <PalmTypes.h>
#include <CoreTraps.h>

#include <NetMgr.h>

// # of bits in a byte
#define	bitsInByte			8


// Maximum size of variable-size ints in # of bits and bytes.  This is based on
// the largest variable size int, which is encoded as follows: 1111  Bit[32]
#define bitVarIntMaxBits	36
#define bitVarIntMaxBytes	((bitVarIntMaxBits + bitsInByte - 1) / bitsInByte)


//=====================================================
// BitXXX Macros
//
// The following macros are handy because they don't require that
//  the source code pass in the NetLib library refnum to every
//  call. 
//
// When building server code or other emulation code where th
//   library trap dispatcher is not used, the libRefNUm is unused 
//	 and can be 0.
// 
// When building for the viewer, the libRefNum must be the refNum
//  of the NetLib. For applications, this libRefNum must be put
//  into an application global named 'AppNetRefnum'.
//
//====================================================
#if USE_TRAPS == 0
	#define	netPrvRefnum 0
#else
	#define	netPrvRefnum AppNetRefnum
#endif


#define	BitMove( dstP,  dstBitOffsetP, srcP,  srcBitOffsetP, numBits)	\
	NetLibBitMove(netPrvRefnum, dstP, dstBitOffsetP, srcP, srcBitOffsetP, numBits)

#define	BitPutFixed( dstP,  dstBitOffsetP, value,  numBits)	\
	NetLibBitPutFixed(netPrvRefnum, dstP,  dstBitOffsetP, value,  numBits) 

#define	BitGetFixed(srcP, srcBitOffsetP, numBits)	\
	NetLibBitGetFixed(netPrvRefnum, srcP, srcBitOffsetP, numBits)	

#define	BitPutUIntV(dstP, dstBitOffsetP, value)	\
	NetLibBitPutUIntV(netPrvRefnum, dstP, dstBitOffsetP, value)

#define	BitGetUIntV(srcP, srcBitOffsetP)	\
	NetLibBitGetUIntV(netPrvRefnum, srcP, srcBitOffsetP)

#define	BitPutIntV(dstP, dstBitOffsetP,	value) \
	NetLibBitPutIntV(netPrvRefnum, dstP, dstBitOffsetP, value)

#define	BitGetIntV(srcP, srcBitOffsetP)	\
	NetLibBitGetIntV(netPrvRefnum, srcP, srcBitOffsetP)


//=====================================================
// Macros that convert native integers to and from
//  big-endian (network) order which is the order used to store
//  variable length integers by the BitMove utilities.
//====================================================

#if CPU_TYPE == CPU_x86

#define _NetSwap16(x) \
	((((x) >> 8) & 0xFF) | \
	 (((x) & 0xFF) << 8))

#define _NetSwap32(x) \
	((((x) >> 24) & 0x00FF) | \
	 (((x) >>  8) & 0xFF00) | \
	 (((x) & 0xFF00) <<  8) | \
	 (((x) & 0x00FF) << 24))

#define NetHToNS(x)	_NetSwap16(x)
#define NetHToNL(x)	_NetSwap32(x)
#define NetNToHS(x)	_NetSwap16(x)
#define NetNToHL(x)	_NetSwap32(x)

#elif CPU_TYPE == CPU_68K

#define NetHToNS(x)	(x)
#define NetHToNL(x)	(x)
#define NetNToHS(x)	(x)
#define NetNToHL(x)	(x)

#else
// We'll define these macros for any other architectures needed as we come
// across them.
#endif

//=====================================================
// Functions
//====================================================
#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------
// Bit Moving functions. For "slim" bit packing protocols
// used over wireless. 
//--------------------------------------------------
void		NetLibBitMove(UInt16 libRefNum, UInt8 *dstP, UInt32 *dstBitOffsetP,
					UInt8 *srcP, UInt32 *srcBitOffsetP,
					UInt32		numBits)
				SYS_TRAP(netLibTrapBitMove);

						
void		NetLibBitPutFixed(UInt16 libRefNum, UInt8 *dstP, UInt32 *dstBitOffsetP,
					UInt32 value, UInt16 numBits)
				SYS_TRAP(netLibTrapBitPutFixed);
						
UInt32 	NetLibBitGetFixed(UInt16 libRefNum, UInt8 *dstP, UInt32 *dstBitOffsetP, 
					UInt16 numBits)
				SYS_TRAP(netLibTrapBitGetFixed);
						
void		NetLibBitPutUIntV(UInt16 libRefNum, UInt8 *dstP, UInt32 *dstBitOffsetP, 
					UInt32 value)
				SYS_TRAP(netLibTrapBitPutUIntV);

UInt32 	NetLibBitGetUIntV(UInt16 libRefNum, UInt8 *dstP, UInt32 *dstBitOffsetP)
				SYS_TRAP(netLibTrapBitGetUIntV);

void		NetLibBitPutIntV(UInt16 libRefNum, UInt8 *dstP, UInt32 *dstBitOffsetP, 
					Int32 value)
				SYS_TRAP(netLibTrapBitPutUIntV);

Int32 	NetLibBitGetIntV(UInt16 libRefNum, UInt8 *dstP, UInt32 *dstBitOffsetP)
				SYS_TRAP(netLibTrapBitGetUIntV);

#ifdef __cplusplus
}
#endif


#endif	// __NETBITUTILS_H__
