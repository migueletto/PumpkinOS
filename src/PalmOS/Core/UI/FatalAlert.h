/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: FatalAlert.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *        This file defines the system Fatal Alert support.
 *
 *****************************************************************************/

#ifndef __FATALALERT_H__
#define __FATALALERT_H__

#include <PalmTypes.h>
#include <CoreTraps.h>

// Value returned by SysFatalAlert
#define fatalReset			0
#define fatalEnterDebugger	1
#define fatalDoNothing		0xFFFFU

#ifdef __cplusplus
extern "C" {
#endif

UInt16 SysFatalAlert (const Char *msg)
		SYS_TRAP(sysTrapSysFatalAlert);

void SysFatalAlertInit (void)
		SYS_TRAP(sysTrapSysFatalAlertInit);

#ifdef __cplusplus 
}
#endif

#endif  // __FATALALERT_H__
