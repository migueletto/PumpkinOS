/******************************************************************************
 *
 * Copyright (c) 1995-2003 PalmSource, Inc. All rights reserved.
 *
 * File: Launcher.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *	  These are the routines for the launcher.
 *
 *****************************************************************************/

#ifndef __LAUNCHER_H__
#define __LAUNCHER_H__

#include <PalmTypes.h>
#include <CoreTraps.h>


/************************************************************
 * Launcher procedures
 *************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

// We're leaving the trap in place for now, but it just does a SysUIAppSwitch to
// launch the real launcher.  --Bob 21-Jul-00

void 		SysAppLauncherDialog()
					SYS_TRAP(sysTrapSysAppLauncherDialog);


#ifdef __cplusplus
}
#endif

#endif // __LAUNCHER_H__
