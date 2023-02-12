/******************************************************************************
 *
 * Copyright (c) 2002-2003 PalmSource, Inc. All rights reserved.
 *
 * File: FixedMath.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *        This file contains fixed point math definitions.
 *
 *****************************************************************************/

#ifndef __FIXEDMATH_H__
#define __FIXEDMATH_H__

#include <PalmTypes.h>
#include <ErrorMgr.h>


// turn on 32 bit fixed point for 1.5x scaling support
#define FIXED_POINT_32_BIT
#ifdef FIXED_POINT_32_BIT

//-----------------------------------------------
// 32 bit (signed 16.16) Fixed Point definitions
//-----------------------------------------------

typedef	Int32					FixedType;

#define kFixedBias				16		
#define	kFixedFractionMask		0x0000FFFF		

#define kFixedOneHalf			0x00008000L
#define kFixedTwo				0x00020000L
#define	kFixedOneAndOneHalf		0x00018000L
#define kFixedTwoThirds			0x0000AAABL

// convert integer to Fixed
#define FixedFromInteger(x)		(((FixedType) (x)) << kFixedBias)
// convert Fixed to integer
#define FixedToInteger(x)		((FixedType) (x) < 0) ? -(-((FixedType) (x)) >> (kFixedBias)) : ((x) >> (kFixedBias))

#define FixedFraction(x)		((x) & kFixedFractionMask)

#define FixedAdd(lhs, rhs) 		((lhs) + (rhs))
#define FixedSub(lhs, rhs) 		((lhs) - (rhs))

// if lhs and rhs are fixed, product fixed
// if lhs or rhs are int, product int			
#if ERROR_CHECK_LEVEL == ERROR_CHECK_FULL
#define FixedMul(lhs, rhs) 		ECFixedMul(lhs, rhs)

#else
#define FixedMul(lhs, rhs) 	 	(((lhs) * (rhs)) >> kFixedBias)

#endif

// if lhs and rhs are fixed, quotient fixed
// if lhs and rhs are int, quotient fixed
// lhs is int, rhs is fixed, quotient int
#if ERROR_CHECK_LEVEL == ERROR_CHECK_FULL

#define	FixedDiv(lhs, rhs) 		ECFixedDiv(lhs, rhs)

#else

// Optimization check for 72/108, which is coommon for QVGA.
#define	FixedDiv(lhs, rhs)		((((lhs) == 72) && ((rhs) == 108))  ? kFixedTwoThirds : (((FixedType) (lhs) << kFixedBias) / (rhs)))

#endif

// optimization
#define FixedPower2Mul(x, power)  	((x) << (power))
#define FixedPower2Div(x, power)  	((x) >> (power))


#else

//-----------------------------------------------
// 16 bit (signed 10.6) Fixed Point definitions
//-----------------------------------------------

typedef Int16 					FixedType;
typedef Int32					FixedIntermediate;	// used internally

#define kFixedBias				(6)					// use 10 bits for integer, 6 bits for fraction
#define kFixedFractionMask		(0x3F)				// 6 bits turned on

#define kFixedOneHalf			(1 << (kFixedBias - 1))

// convert integer to FixedType
#define FixedFromInteger(x)		((x) << (kFixedBias))
// convert FixedType to integer
#define FixedToInteger(x)		((FixedType) (x) < 0) ? -(-((FixedType) (x)) >> (kFixedBias)) : ((x) >> (kFixedBias))

#define FixedFraction(x)		((x) & kFixedFractionMask)

#define FixedAdd(lhs, rhs) 		((lhs) + (rhs))
#define FixedSub(lhs, rhs) 		((lhs) - (rhs))


#if defined __MWERKS__ && defined __MC68K__
// if lhs and rhs are fixed, product fixed
// if lhs or rhs are int, product int

// However, FixedMul is implemented as an inline assembly function because the compiler 
// does not generate the desired instructions:  instead of using the muls instruction,
// the compiler makes a call to the multiply library function.
inline asm FixedType FixedMul(FixedType lhs:__D0, FixedType rhs:__D1):__D0
{
	muls.w	d1,d0
	asr.l	#kFixedBias,d0
	// rts
}
#else
#define FixedMul(lhs, rhs)		((((Int32) (lhs) * (rhs))) >> kFixedBias)
#endif


// if lhs and rhs are fixed, quotient fixed
// if lhs and rhs are int, quotient fixed
#define FixedDiv(lhs, rhs)		((FixedType)((((Int32) (lhs)) << kFixedBias) / ((Int32) (rhs))))
// lhs is int, rhs is fixed, quotient int
#define	DivIntByFixedResultInt(lhs, rhs)	((((Int32) (lhs)) << kFixedBias) / ((Int32) (rhs)))

// optimization
#define FixedPower2Mul(x, power)  	((x) << (power))
#define FixedPower2Div(x, power)  	((x) >> (power))

#endif	// ifdef FIXED_POINT_32_BIT



FixedType ECFixedMul(Int32 lhs, Int32 rhs)	SYS_TRAP(sysTrapECFixedMul);
FixedType ECFixedDiv(Int32 lhs, Int32 rhs)	SYS_TRAP(sysTrapECFixedDiv);



#endif 	// __FIXEDMATH_H__
