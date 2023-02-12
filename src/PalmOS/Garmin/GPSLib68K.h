/*********************************************************************
*
*   MODULE NAME:
*       GPSLib68K.h - 68K header file for ARM-native GPS library
*
*   Copyright 2001-2002 by GARMIN Corporation.
*
*********************************************************************/

#ifndef __GPSLIB68K_H__
#define __GPSLIB68K_H__

#include <PalmTypes.h>
#include <LibTraps.h>
#include "GPSLib.h"
#include "GPSLibSysTrapNums.h"

/********************************************************************
 * Prototypes
 ********************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
*
*   PROCEDURE NAME:
*       GPSClose - Close GPSLib
*
*   DESCRIPTION:
*       Closes GPSLib and disposes of the global data memory if
*       required. Called by any application or library that's been
*       using GPSLib and is now finished with it.
*
*       This shouldn't be called if GPSOpen failed. If other apps
*       are still using GPSLib we return the code gpsErrStillOpen to
*       tell the calling app not to call SysLibRemove just yet.
*
*********************************************************************/
Err GPSClose( const UInt16 refNum )
        SYS_TRAP(gpsLibTrapClose);

/*********************************************************************
*
*   PROCEDURE NAME:
*       GPSGetLibAPIVersion - Get GPSLib API Version
*
*   DESCRIPTION:
*       Returns the GPSLIB_VERS constant.
*
*       Can be called without opening GPSLib first.
*
*********************************************************************/
UInt16 GPSGetLibAPIVersion( const UInt16 refNum )
        SYS_TRAP(gpsLibTrapGetLibAPIVersion);

/*********************************************************************
*
*   PROCEDURE NAME:
*       GPSGetMaxSatellites - Get the Maximum Number of Satellites
*
*   DESCRIPTION:
*       Returns the maximum number of satellites that are currently supported.
*
*       The value returned by this routine should be used in the dynamic
*       allocation of the array of satellites (GPSSatDataType).
*
*********************************************************************/
UInt8 GPSGetMaxSatellites( const UInt16 refNum )
        SYS_TRAP(gpsLibTrapGetMaxSatellites);

/*********************************************************************
*
*   PROCEDURE NAME:
*       GPSGetPosition - Get Current Position Data
*
*   DESCRIPTION:
*       Returns a GPSPositionDataType structure with the latest position
*       from the GPS.
*
*       If the return value is not gpsErrNone the data should be
*       considered invalid.
*
*********************************************************************/
Err GPSGetPosition( const UInt16 refNum, GPSPositionDataType *position )
        SYS_TRAP(gpsLibTrapGetPosition);

/*********************************************************************
*
*   PROCEDURE NAME:
*       GPSGetPVT - Get Current Position/Velocity/Time Data
*
*   DESCRIPTION:
*       Returns a GPSPVTDataType structure with the latest position,
*       velocity, and time data from the GPS.
*
*       If p_pvt->status.fix is equal to gpsFixUnusable or
*       gpsFixInvalid, the rest of the data in the structure should
*       be assumed to be invalid.
*
*********************************************************************/
Err GPSGetPVT( const UInt16 refNum, GPSPVTDataType *pvt )
        SYS_TRAP(gpsLibTrapGetPVT);

/*********************************************************************
*
*   PROCEDURE NAME:
*       GPSGetSatellites - Get Current Satellite Data
*
*   DESCRIPTION:
*       Returns a GPSSatDataType structure with the latest satellite
*       information from the GPS.
*
*********************************************************************/
Err GPSGetSatellites( const UInt16 refNum, GPSSatDataType *sat )
        SYS_TRAP(gpsLibTrapGetSatellites);

/*********************************************************************
*
*   PROCEDURE NAME:
*       GPSGetStatus - Get Current Status Data
*
*   DESCRIPTION:
*       Returns a GPSStatusDataType structure with the latest status
*       from the GPS.
*
*       If the return value is not gpsErrNone the data should be
*       considered invalid.
*
*********************************************************************/
Err GPSGetStatus( const UInt16 refNum, GPSStatusDataType *status )
        SYS_TRAP(gpsLibTrapGetStatus);

/*********************************************************************
*
*   PROCEDURE NAME:
*       GPSGetTime - Get Current Time Data
*
*   DESCRIPTION:
*       Returns a GPSTimeDataType structure with the latest time
*       from the GPS.
*
*       If the return value is not gpsErrNone the data should be
*       considered invalid.
*
*********************************************************************/
Err GPSGetTime( const UInt16 refNum, GPSTimeDataType *time )
        SYS_TRAP(gpsLibTrapGetTime);

/*********************************************************************
*
*   PROCEDURE NAME:
*       GPSGetVelocity - Get Current Velocity Data
*
*   DESCRIPTION:
*       Returns a GPSVelocityDataType structure with the latest velocity
*       from the GPS.
*
*       If the return value is not gpsErrNone the data should be
*       considered invalid.
*
*********************************************************************/
Err GPSGetVelocity( const UInt16 refNum, GPSVelocityDataType *velocity )
        SYS_TRAP(gpsLibTrapGetVelocity);

/*********************************************************************
*
*   PROCEDURE NAME:
*       GPSOpen - Open GPSLib
*
*   DESCRIPTION:
*       Opens GPSLib and prepares it for use. Called by any application or
*       library that wants to use the services that GPSLib provides.
*
*       GPSOpen *must* be called before calling any other GPS
*       functions, with the exception of GPSGetLibAPIVersion.
*       If an error occurs during initialization, GPSOpen returns the
*       error code and exits cleanly.
*
*********************************************************************/
Err GPSOpen( const UInt16 refNum )
        SYS_TRAP(gpsLibTrapOpen);

#ifdef __cplusplus
}
#endif

#endif  //__GPSLIB68K_H__
