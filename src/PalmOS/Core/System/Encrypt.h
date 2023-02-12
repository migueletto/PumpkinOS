/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: Encrypt.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *		Equates for encryption/digestion routines in pilot
 *
 *****************************************************************************/

#ifndef __ENCRYPT_H__
#define __ENCRYPT_H__

// Include elementary types
#include <PalmTypes.h>					// Basic types
#include <CoreTraps.h>					// Trap Numbers.


/************************************************************
 * Function Prototypes
 *************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

// Perform reversible encryption or decryption of 8 byte string in
//  srcP using 8 byte key keyP. Place 8 byte result in dstP.
Err	EncDES(UInt8 *srcP, UInt8 *keyP, UInt8 *dstP, Boolean encrypt)
			SYS_TRAP(sysTrapEncDES);


// Digest a string of bytes and produce a 128 bit result using
//   the MD4 algorithm.
Err	EncDigestMD4(UInt8 *strP, UInt16 strLen, UInt8 digestP[16])
			SYS_TRAP(sysTrapEncDigestMD4);


// Digest a string of bytes and produce a 128 bit result using
//   the MD5 algorithm.
Err	EncDigestMD5(UInt8 *strP, UInt16 strLen, UInt8 digestP[16])
			SYS_TRAP(sysTrapEncDigestMD5);



#ifdef __cplusplus 
}
#endif



#endif	//__ENCRYPT_H__
