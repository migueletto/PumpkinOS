/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: CPMLib68KInterface.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *    This module contains the interface definition for the Cryptographic
 *    Provider Manager library on Pilot for 68K code.
 *
 *****************************************************************************/

#ifndef _CPMLIB_68K_INTERFACE_H
#define _CPMLIB_68K_INTERFACE_H

#include "CPMLibCommon.h"

/** CPMLib Trap number selectors
 *
 * CPM library call ID's. Each library call gets the trap number:
 * cpmTrapXXXX which serves as an index into the library's dispatch table.
 * The constant sysLibTrapCustom is the first available trap number after
 * the system predefined library traps Open, Close, Sleep and Wake.
 *
 * <b>WARNING!!! This order of these traps MUST match the order of the dispatch
 * table in CPMLibDispatch.c!!!</b>
 */
#define cpmLibTrapOpen					sysLibTrapOpen
#define cpmLibTrapClose					sysLibTrapClose
#define cpmLibTrapSleep					sysLibTrapSleep
#define cpmLibTrapWake					sysLibTrapWake


#define cpmLibTrapGetInfo					sysLibTrapCustom
#define cpmLibTrapSetDebugLevel			(sysLibTrapCustom+1)

#define cpmLibTrapEnumerateProviders	(sysLibTrapCustom+2)
#define cpmLibTrapGetProviderInfo		(sysLibTrapCustom+3)
#define cpmLibTrapSetDefaultProvider	(sysLibTrapCustom+4)

#define cpmLibTrapGenerateRandomBytes	(sysLibTrapCustom+5)
#define cpmLibTrapAddRandomSeed			(sysLibTrapCustom+6)

#define cpmLibTrapGenerateKey				(sysLibTrapCustom+7)
#define cpmLibTrapImportKeyInfo			(sysLibTrapCustom+8)
#define cpmLibTrapExportKeyInfo			(sysLibTrapCustom+9)
#define cpmLibTrapReleaseKeyInfo			(sysLibTrapCustom+10)

#define cpmLibTrapHashInit					(sysLibTrapCustom+11)
#define cpmLibTrapHashUpdate				(sysLibTrapCustom+12)
#define cpmLibTrapHashFinal				(sysLibTrapCustom+13)
#define cpmLibTrapHash						(sysLibTrapCustom+14)
#define cpmLibTrapImportHashInfo			(sysLibTrapCustom+15)
#define cpmLibTrapExportHashInfo			(sysLibTrapCustom+16)
#define cpmLibTrapReleaseHashInfo		(sysLibTrapCustom+17)

#define cpmLibTrapEncryptInit				(sysLibTrapCustom+18)
#define cpmLibTrapEncryptUpdate			(sysLibTrapCustom+19)
#define cpmLibTrapEncryptFinal			(sysLibTrapCustom+20)
#define cpmLibTrapEncrypt					(sysLibTrapCustom+21)
#define cpmLibTrapDecryptInit				(sysLibTrapCustom+22)
#define cpmLibTrapDecryptUpdate			(sysLibTrapCustom+23)
#define cpmLibTrapDecryptFinal			(sysLibTrapCustom+24)
#define cpmLibTrapDecrypt					(sysLibTrapCustom+25)
#define cpmLibTrapImportCipherInfo		(sysLibTrapCustom+26)
#define cpmLibTrapExportCipherInfo		(sysLibTrapCustom+27)
#define cpmLibTrapReleaseCipherInfo		(sysLibTrapCustom+28)

#define cpmLibTrapVerifyInit				(sysLibTrapCustom+29)
#define cpmLibTrapVerifyUpdate			(sysLibTrapCustom+30)
#define cpmLibTrapVerifyFinal				(sysLibTrapCustom+31)
#define cpmLibTrapVerify					(sysLibTrapCustom+32)
#define cpmLibTrapImportVerifyInfo		(sysLibTrapCustom+33)
#define cpmLibTrapExportVerifyInfo		(sysLibTrapCustom+34)
#define cpmLibTrapReleaseVerifyInfo		(sysLibTrapCustom+35)
		
#define cpmLibTrapLast						(sysLibTrapCustom+36)


#ifndef TRAPS_ONLY
/*
 * Library initialization and shutdown
 */
Err CPMLibOpen(UInt16 refnum, UInt16 *numProviders)  SYS_TRAP(sysLibTrapOpen);
Err CPMLibClose(UInt16 refnum)  SYS_TRAP(sysLibTrapClose);
Err CPMLibSleep(UInt16 refnum)  SYS_TRAP(sysLibTrapSleep);
Err CPMLibWake(UInt16 refnum)  SYS_TRAP(sysLibTrapWake);

/* 
 * Library settings
 */
Err CPMLibGetInfo(UInt16 refnum, CPMInfoType *infoP)  SYS_TRAP(cpmLibTrapGetInfo);
Err CPMLibSetDebugLevel(UInt16 refnum, UInt8 debugLevel)  SYS_TRAP(cpmLibTrapSetDebugLevel);

/* 
 * Provider information
 */
Err CPMLibEnumerateProviders(UInt16 refnum, UInt32 providerIDs[], UInt16 *numProviders)  SYS_TRAP(cpmLibTrapEnumerateProviders);
Err CPMLibGetProviderInfo(UInt16 refnum, UInt32 providerID, APProviderInfoType *providerInfoP)  SYS_TRAP(cpmLibTrapGetProviderInfo);
Err CPMLibSetDefaultProvider(UInt16 refnum, UInt32 providerID)  SYS_TRAP(cpmLibTrapSetDefaultProvider);

/*
 * Random bytes
 */
Err CPMLibGenerateRandomBytes(UInt16 refnum, UInt8 *bufferP, UInt32 *bufLenP)  SYS_TRAP(cpmLibTrapGenerateRandomBytes);
Err CPMLibAddRandomSeed(UInt16 refnum, UInt8 *seedDataP, UInt32 dataLen)  SYS_TRAP(cpmLibTrapAddRandomSeed);

/*
 * Symmetric key generation operations
 */
Err CPMLibGenerateKey(UInt16 refnum, UInt8 *keyDataP, UInt32 dataLen, APKeyInfoType *keyInfoP)  SYS_TRAP(cpmLibTrapGenerateKey);
Err CPMLibImportKeyInfo(UInt16 refnum, UInt8 encoding, UInt8 *importDataP, UInt32 dataLen, APKeyInfoType *keyInfoP)  SYS_TRAP(cpmLibTrapImportKeyInfo);
Err CPMLibExportKeyInfo(UInt16 refnum, APKeyInfoType *keyInfoP, UInt8 encoding, UInt8 *exportDataP, UInt32 *dataLenP)  SYS_TRAP(cpmLibTrapExportKeyInfo);
Err CPMLibReleaseKeyInfo(UInt16 refnum, APKeyInfoType *keyInfoP)  SYS_TRAP(cpmLibTrapReleaseKeyInfo);

/*
 * Message digest operations
 */
Err CPMLibHashInit(UInt16 refnum, APHashInfoType *hashInfoP)  SYS_TRAP(cpmLibTrapHashInit);
Err CPMLibHashUpdate(UInt16 refnum, APHashInfoType *hashInfoP, UInt8 *bufIn, UInt32 bufInLen)  SYS_TRAP(cpmLibTrapHashUpdate);
Err CPMLibHashFinal(UInt16 refnum, APHashInfoType *hashInfoP, UInt8 *bufIn, UInt32 bufInLen, UInt8 *bufOut, UInt32 *bufOutLenP)  SYS_TRAP(cpmLibTrapHashFinal);
Err CPMLibHash(UInt16 refnum, APHashEnum type, APHashInfoType *hashInfoP, UInt8 *bufIn, UInt32 bufInLen, UInt8 *bufOut, UInt32 *bufOutLenP)  SYS_TRAP(cpmLibTrapHash);
Err CPMLibImportHashInfo(UInt16 refnum, UInt8 encoding, UInt8 *importDataP, UInt32 dataLen, APHashInfoType *hashInfoP)  SYS_TRAP(cpmLibTrapImportHashInfo);
Err CPMLibExportHashInfo(UInt16 refnum, APHashInfoType *hashInfoP, UInt8 encoding, UInt8 *exportDataP, UInt32 *dataLenP)  SYS_TRAP(cpmLibTrapExportHashInfo);
Err CPMLibReleaseHashInfo(UInt16 refnum, APHashInfoType *hashInfoP)  SYS_TRAP(cpmLibTrapReleaseHashInfo);

/*
 * Cipher operations
 */
Err CPMLibEncryptInit(UInt16 refnum, APKeyInfoType *keyInfoP, APCipherInfoType *cipherInfoP)  SYS_TRAP(cpmLibTrapEncryptInit);
Err CPMLibEncryptUpdate(UInt16 refnum, APKeyInfoType *keyInfoP, APCipherInfoType *cipherInfoP, UInt8 *bufIn, UInt32 bufInLen, UInt8 *bufOut, UInt32 *bufOutLenP)  SYS_TRAP(cpmLibTrapEncryptUpdate);
Err CPMLibEncryptFinal(UInt16 refnum, APKeyInfoType *keyInfoP, APCipherInfoType *cipherInfoP, UInt8 *bufIn, UInt32 bufInLen, UInt8 *bufOut, UInt32 *bufOutLenP)  SYS_TRAP(cpmLibTrapEncryptFinal);
Err CPMLibEncrypt(UInt16 refnum, APKeyInfoType *keyInfoP, APCipherInfoType *cipherInfoP, UInt8 *bufIn, UInt32 bufInLen, UInt8 *bufOut, UInt32 *bufOutLenP)  SYS_TRAP(cpmLibTrapEncrypt);
Err CPMLibDecryptInit(UInt16 refnum, APKeyInfoType *keyInfoP, APCipherInfoType *cipherInfoP)  SYS_TRAP(cpmLibTrapDecryptInit);
Err CPMLibDecryptUpdate(UInt16 refnum, APKeyInfoType *keyInfoP, APCipherInfoType *cipherInfoP, UInt8 *bufIn, UInt32 bufInLen, UInt8 *bufOut, UInt32 *bufOutLenP)  SYS_TRAP(cpmLibTrapDecryptUpdate);
Err CPMLibDecryptFinal(UInt16 refnum, APKeyInfoType *keyInfoP, APCipherInfoType *cipherInfoP, UInt8 *bufIn, UInt32 bufInLen, UInt8 *bufOut, UInt32 *bufOutLenP)  SYS_TRAP(cpmLibTrapDecryptFinal);
Err CPMLibDecrypt(UInt16 refnum, APKeyInfoType *keyInfoP, APCipherInfoType *cipherInfoP, UInt8 *bufIn, UInt32 bufInLen, UInt8 *bufOut, UInt32 *bufOutLenP)  SYS_TRAP(cpmLibTrapDecrypt);
Err CPMLibImportCipherInfo(UInt16 refnum, UInt8 encoding, UInt8 *importDataP, UInt32 dataLen, APCipherInfoType *cipherInfoP)  SYS_TRAP(cpmLibTrapImportCipherInfo);
Err CPMLibExportCipherInfo(UInt16 refnum, APCipherInfoType *cipherInfoP, UInt8 encoding, UInt8 *exportDataP, UInt32 *dataLenP)  SYS_TRAP(cpmLibTrapExportCipherInfo);
Err CPMLibReleaseCipherInfo(UInt16 refnum, APCipherInfoType *cipherInfoP)  SYS_TRAP(cpmLibTrapReleaseCipherInfo);

/*
 * Verification operations
 */
Err CPMLibVerifyInit(UInt16 refnum, APKeyInfoType *keyInfoP, APVerifyInfoType *verifyInfoP)  SYS_TRAP(cpmLibTrapVerifyInit);
Err CPMLibVerifyUpdate(UInt16 refnum, APKeyInfoType *keyInfoP, APVerifyInfoType *verifyInfoP, UInt8 *bufIn, UInt32 bufInLen)  SYS_TRAP(cpmLibTrapVerifyUpdate);
Err CPMLibVerifyFinal(UInt16 refnum, APKeyInfoType *keyInfoP, APVerifyInfoType *verifyInfoP, UInt8 *bufIn, UInt32 bufInLen, UInt8 *bufOut, UInt32 *bufOutLenP, UInt8 *signature, UInt32 signatureLen, VerifyResultType *verifyResultP)  SYS_TRAP(cpmLibTrapVerifyFinal);
Err CPMLibVerify(UInt16 refnum, APKeyInfoType *keyInfoP, APVerifyInfoType *verifyInfoP, UInt8 *bufIn, UInt32 bufInLen, UInt8 *bufOut, UInt32 *bufOutLenP, UInt8 *signature, UInt32 signatureLen, VerifyResultType *verifyResultP)  SYS_TRAP(cpmLibTrapVerify);
Err CPMLibImportVerifyInfo(UInt16 refnum, UInt8 encoding, UInt8 *importDataP, UInt32 dataLen, APVerifyInfoType *verifyInfoP)  SYS_TRAP(cpmLibTrapImportVerifyInfo);
Err CPMLibExportVerifyInfo(UInt16 refnum, APVerifyInfoType *verifyInfoP, UInt8 encoding, UInt8 *exportDataP, UInt32 *dataLenP)  SYS_TRAP(cpmLibTrapExportVerifyInfo);
Err CPMLibReleaseVerifyInfo(UInt16 refnum, APVerifyInfoType *verifyInfoP)  SYS_TRAP(cpmLibTrapReleaseVerifyInfo);

#endif /* TRAPS_ONLY */
#endif /* _CPMLIB_68K_INTERFACE_H */
