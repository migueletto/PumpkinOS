/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: PhoneLookup.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *	  This file defines phone number lookup structures and routines.
 *
 *****************************************************************************/

#ifndef __PHONE_LOOKUP_H__
#define __PHONE_LOOKUP_H__

#include <PalmTypes.h>
#include <CoreTraps.h>
#include <AppLaunchCmd.h>

#include <Field.h>
#include <DataMgr.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void PhoneNumberLookup (FieldType *fldP)
			SYS_TRAP(sysTrapPhoneNumberLookup);

extern void PhoneNumberLookupCustom (FieldType *fldP, AddrLookupParamsType* params, Boolean useClipboard)
			SYS_TRAP(sysTrapPhoneNumberLookupCustom);

#ifdef __cplusplus 
}
#endif


#endif	// __PHONE_LOOKUP_H__
