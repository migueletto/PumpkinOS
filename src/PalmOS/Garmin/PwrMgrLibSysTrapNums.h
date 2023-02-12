/*********************************************************************
*
*   MODULE NAME:
*       PwrMgrLibSysTrapNums.h - Contains the SYS_TRAP numbers for the 
*           PwrMgrLib library that can be used in both the 68K code that 
*           uses the library, and the ARM shim code inside the 
*           library.
*
*   Copyright 2003 by GARMIN Corporation.
*
*********************************************************************/

#ifndef __PWRMGRLIBSYSTRAPNUMS_H__
#define __PWRMGRLIBSYSTRAPNUMS_H__

#ifndef __MC68K__
    #include <LibTraps68K.h>
#endif

/********************************************************************
 * Traps
 ********************************************************************/
enum
	{
	pwrMgrLibTrapOpen = sysLibTrapOpen,
	pwrMgrLibTrapClose = sysLibTrapClose,
	pwrSetLowPowerMode = sysLibTrapCustom
	};

#endif  //__PWRMGRLIBSYSTRAPNUMS_H__


