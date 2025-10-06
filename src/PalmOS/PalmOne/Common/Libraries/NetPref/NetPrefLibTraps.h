/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/**
 * @ingroup NETPREF
 */

/**
 * @file 	NetPrefLibTraps.h
 * @version 1.0
 * @date    12/12/2001
 *
 * @brief   Trap definitions for the NetPref Library.
 *
 */

/*
 * @author	vmk
 *
 * <hr>
 */


#ifndef __NET_PREF_LIB_TRAPS_H__
#define __NET_PREF_LIB_TRAPS_H__

/**
 * @name Trap IDs
 *
 * @brief Trap IDs for the NetPref Library's public functions.
 *
 * The order of the traps must be the same as the routines are listed in NetPrefLibDispatchTable.c.
 */
/*@{*/
#define netPrefLibTrapVersionGet						(sysLibTrapCustom + 0)
#define netPrefLibTrapRecCountGet						(sysLibTrapCustom + 1)
#define netPrefLibTrapRecIDGetByIndex					(sysLibTrapCustom + 2)
#define netPrefLibTrapRecNew							(sysLibTrapCustom + 3)
#define netPrefLibTrapRecLoad							(sysLibTrapCustom + 4)
#define netPrefLibTrapRecRelease						(sysLibTrapCustom + 5)
#define netPrefLibTrapRecSave							(sysLibTrapCustom + 6)
#define netPrefLibTrapRecIDGet							(sysLibTrapCustom + 7)
#define netPrefLibTrapRecIndexGet						(sysLibTrapCustom + 8)
#define netPrefLibTrapRecDelete							(sysLibTrapCustom + 9)
#define netPrefLibTrapRecFieldGet						(sysLibTrapCustom + 10)
#define netPrefLibTrapRecFieldSet						(sysLibTrapCustom + 11)
#define netPrefLibTrapDefaultTargetGet					(sysLibTrapCustom + 12)
#define netPrefLibTrapDefaultTargetSet					(sysLibTrapCustom + 13)
#define netPrefLibTrapRecObjCopy						(sysLibTrapCustom + 14)
#define netPrefLibTrapRecIsDirty						(sysLibTrapCustom + 15)
#define netPrefLibTrapRecIndexGetByID					(sysLibTrapCustom + 16)
#define netPrefLibTrapRecFieldViewSet					(sysLibTrapCustom + 17)
#define netPrefLibTrapRecFieldAttrsGet					(sysLibTrapCustom + 18)
#define netPrefLibTrapUpdateFromRadioNV					(sysLibTrapCustom + 19)

#define netPrefLibTrapRecFieldSetDefine					(sysLibTrapCustom + 20)
#define netPrefLibTrapRecFieldSetGet					(sysLibTrapCustom + 21)
#define netPrefLibTrapRecFieldSetDefineStd				(sysLibTrapCustom + 22)
#define netPrefLibTrapRecFieldAddToSet					(sysLibTrapCustom + 23)

#define netPrefLibTrapRecReadOnlyOverrideStart			(sysLibTrapCustom + 24)
#define netPrefLibTrapRecReadOnlyOverrideEnd			(sysLibTrapCustom + 25)
#define netPrefLibTrapRecDirtyFlagsReset				(sysLibTrapCustom + 26)
#define netPrefLibTrapRecMarkDirty						(sysLibTrapCustom + 27)
#define netPrefLibTrapRecObjAlloc						(sysLibTrapCustom + 28)
#define netPrefLibTrapRecIsAttached						(sysLibTrapCustom + 29)
#define netPrefLibTrapRecBindingErrorGet				(sysLibTrapCustom + 30)
#define netPrefLibTrapHandleHotSyncNotify				(sysLibTrapCustom + 31)

#define netPrefLibTrapRecShortFieldGet					(sysLibTrapCustom + 32)
#define netPrefLibTrapRecByteFieldGet					(sysLibTrapCustom + 33)
#define netPrefLibTrapRecFieldViewGet					(sysLibTrapCustom + 34)
#define netPrefLibTrapRecLongFieldSetAsFlags			(sysLibTrapCustom + 35)
#define netPrefLibTrapRecFieldSetFromBinHandle			(sysLibTrapCustom + 36)
#define netPrefLibTrapRecFieldSetFromStrHandle			(sysLibTrapCustom + 37)
#define netPrefLibTrapRecFieldSetFromStrPtr				(sysLibTrapCustom + 38)
#define netPrefLibTrapRecConnectionInfoGet				(sysLibTrapCustom + 39)
#define netPrefLibTrapRecMediumDerive					(sysLibTrapCustom + 40)
#define netPrefLibTrapRecIPAddrFieldSet					(sysLibTrapCustom + 41)
#define netPrefLibTrapUtilZStringListSizeGet			(sysLibTrapCustom + 42)
#define netPrefLibTrapRecPhoneStringCompose				(sysLibTrapCustom + 43)
#define netPrefLibTrapUtilPlatformIDGet					(sysLibTrapCustom + 44)
#define netPrefLibTrapUtilDefWirelessDriverIDGet		(sysLibTrapCustom + 45)
#define netPrefLibTrapRecLongFieldGet					(sysLibTrapCustom + 46)
/*@}*/

#endif /* __NET_PREF_LIB_TRAPS_H__ */
