/******************************************************************************
*
*   HEADER NAME:
*       GPSLib.h
*
*   DESCRIPTION:
*       Application Program Interface to the Global Positioning System Library.
*
*   Copyright 2002-2003 by Garmin Ltd. or its subsidiaries.
*
******************************************************************************/

#ifndef __GPSLIB_H__
#define __GPSLIB_H__

/*-----------------------------------------------------------------------------
                                GENERAL INCLUDES
-----------------------------------------------------------------------------*/

#include <PalmTypes.h>
#include <SystemResources.h>

/*-----------------------------------------------------------------------------
                                LITERAL CONSTANTS
-----------------------------------------------------------------------------*/

#define gpsLibName              "GPSLib"
#define gpsLibType              sysFileTLibrary
#define gpsLibCreator           'gpsL'
#define sysNotifyGPSDataEvent   'gpsD'          /* GPS Data Event   */

#define gpsAPIVersion       3       /* GPS library API version      */
#define gpsInvalidSVID      255     /* invalid space vehicle ID     */

/*----------------------------------------------------------
GPSLib Result Codes
----------------------------------------------------------*/
#define gpsErrNone          0
#define gpsErrNotOpen       ( appErrorClass | 1 )
#define gpsErrStillOpen     ( appErrorClass | 2 )
#define gpsErrMemory        ( appErrorClass | 3 )
#define gpsErrNoData        ( appErrorClass | 4 )

/*----------------------------------------------------------
Extended notification information
----------------------------------------------------------*/
#define gpsLocationChange       0
#define gpsStatusChange         1
#define gpsLostFix              2
#define gpsSatDataChange        3
#define gpsModeChange           4
#define gpsNeedInitialized      5
#define gpsDisplayLegalese      6

/*----------------------------------------------------------
Satellite Status Bitfield Mask Values
----------------------------------------------------------*/
#define gpsSatEphMask       1       /* ephemeris mask               */
#define gpsSatDifMask       2       /* differential mask            */
#define gpsSatUsedMask      4       /* used in solution mask        */
#define gpsSatRisingMask    8       /* rising mask                  */

/*-----------------------------------------------------------------------------
                                      TYPES
-----------------------------------------------------------------------------*/

/*----------------------------------------------------------
Satellite Data Type
----------------------------------------------------------*/
typedef struct
    {
    UInt8       svid;       /* space vehicle identifier             */
    UInt8       status;     /* status bitfield                      */
    Int16       snr;        /* signal to noise ratio * 100 (dB Hz)  */
    float       azimuth;    /* azimuth (radians)                    */
    float       elevation;  /* elevation (radians)                  */
    } GPSSatDataType;

/*----------------------------------------------------------
GPS Mode Type
----------------------------------------------------------*/
typedef Int8 GPSModeT8; enum
    {
    gpsModeOff      = 0,    /* GPS is off                           */
    gpsModeNormal   = 1,    /* continuous satellite tracking        */
    gpsModeBatSaver = 2,    /* periodic satellite tracking          */
    gpsModeSim      = 3,    /* simulated GPS information            */
    gpsModeExternal = 4,    /* external source of GPS information   */
    gpsModeCount    = 5     /* count of mode type enumerations      */
    };

/*----------------------------------------------------------
GPS Fix Type
----------------------------------------------------------*/
typedef Int8 GPSFixT8; enum
    {
    gpsFixUnusable  = 0,    /* failed integrity check               */
    gpsFixInvalid   = 1,    /* invalid or unavailable               */
    gpsFix2D        = 2,    /* 2 dimension                          */
    gpsFix3D        = 3,    /* 3 dimension                          */
    gpsFix2DDiff    = 4,    /* 2 dimension differential             */
    gpsFix3DDiff    = 5,    /* 3 dimension differential             */
    gpsFixCount     = 6     /* count of fix type enumerations       */
    };

/*----------------------------------------------------------
GPS Status Data Type
----------------------------------------------------------*/
typedef struct
    {
    GPSModeT8   mode;       /* mode type                                    */
    GPSFixT8    fix;        /* fix type                                     */
    UInt16      filler2;    /* alignment padding                            */
    float       epe;        /* estimated position error, 1-sigma (meters)   */
    float       eph;        /* epe, horizontal only (meters)                */
    float       epv;        /* epe, vertical only (meters)                  */
    } GPSStatusDataType;

/*----------------------------------------------------------
GPS Position Data Type

The GPSPositionDataType uses integers to indicate latitude
and longitude in semicircles, where 2^31 semicircles equals
180 degrees. North latitudes and East longitudes are
indicated with positive numbers; South latitudes and West
longitudes are indicated with negative numbers. The
following formulas show how to convert between degrees and
semicircles:

    degrees = semicircles * ( 180 / 2^31 )
    semicircles = degrees * ( 2^31 / 180 )
----------------------------------------------------------*/
typedef struct
    {
    Int32       lat;        /* latitude  (semicircles)                      */
    Int32       lon;        /* longitude (semicircles)                      */
    float       altMSL;     /* altitude above mean sea level (meters)       */
    float       altWGS84;   /* altitude above WGS84 ellipsoid (meters)      */
    } GPSPositionDataType;

/*----------------------------------------------------------
GPS Velocity Data Type
----------------------------------------------------------*/
typedef struct
    {
    float       east;       /* east (m/s)                           */
    float       north;      /* north (m/s)                          */
    float       up;         /* upwards (m/s)                        */
    float       track;      /* track (radians)                      */
    float       speed;      /* speed, horizontal only (m/s)         */
    } GPSVelocityDataType;

/*----------------------------------------------------------
GPS Time Data Type
----------------------------------------------------------*/
typedef struct
    {
    UInt32      seconds;        /* seconds since midnight (UTC)     */
    UInt32      fracSeconds;    /* 0..1 second * 2^32               */
    } GPSTimeDataType;

/*----------------------------------------------------------
GPS Comprehensive Data Type
----------------------------------------------------------*/
typedef struct
    {
    GPSStatusDataType       status;
    GPSPositionDataType     position;
    GPSVelocityDataType     velocity;
    GPSTimeDataType         time;
    } GPSPVTDataType;

/*-----------------------------------------------------------------------------
                                   PROCEDURES
-----------------------------------------------------------------------------*/

#endif  /* __GPSLIB_H__ */
