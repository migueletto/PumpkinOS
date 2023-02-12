/*********************************************************************
*
*   MODULE NAME:
*       GPSLibSysTrapNums.h - Contains the SYS_TRAP numbers for the
*           GPSLib library that can be used in both the 68K code that
*           uses the library, and the ARM shim code inside the
*           library.
*
*   Copyright 2002-2004 by Garmin Ltd. or its subsidiaries.
*
*********************************************************************/

#ifndef __GPSLIBSYSTRAPNUMS_H__
#define __GPSLIBSYSTRAPNUMS_H__

#ifndef __MC68K__
    #include <LibTraps.h>
#endif

/********************************************************************
 * Traps
 ********************************************************************/
#define gpsLibTrapClose             ( sysLibTrapCustom + 0 )
#define gpsLibTrapGetLibAPIVersion  ( sysLibTrapCustom + 1 )
#define gpsLibTrapGetMaxSatellites  ( sysLibTrapCustom + 2 )
#define gpsLibTrapGetPosition       ( sysLibTrapCustom + 3 )
#define gpsLibTrapGetPVT            ( sysLibTrapCustom + 4 )
#define gpsLibTrapGetSatellites     ( sysLibTrapCustom + 5 )
#define gpsLibTrapGetStatus         ( sysLibTrapCustom + 6 )
#define gpsLibTrapGetTime           ( sysLibTrapCustom + 7 )
#define gpsLibTrapGetVelocity       ( sysLibTrapCustom + 8 )
#define gpsLibTrapOpen              ( sysLibTrapCustom + 9 )

#endif  //__GPSLIBSYSTRAPNUMS_H__
