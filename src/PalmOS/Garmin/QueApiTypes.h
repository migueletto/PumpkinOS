/******************************************************************************
*
*   HEADER NAME:
*       QueApiTypes.h
*
*   DESCRIPTION:
*       Types for the Application Program Interface to the Que technology
*
*   Copyright 2004 by Garmin Ltd. or its subsidiaries.
*
******************************************************************************/

#ifndef __QUEAPITYPES_H__
#define __QUEAPITYPES_H__

/*-----------------------------------------------------------------------------
                                GENERAL INCLUDES
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
                                LITERAL CONSTANTS
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
                                      TYPES
-----------------------------------------------------------------------------*/

/*----------------------------------------------------------
Que API Result Codes
----------------------------------------------------------*/
typedef uint16 QueErrT16; enum
    {
    queErrNone    = 0,              /* Success                                       */
    queErrNotOpen = firstQueAPIErr, /* Close() without having called open() first    */
    queErrBadArg,                   /* Invalid parameter passed to function          */
    queErrMemory,                   /* Out of memory                                 */
    queErrNoData,                   /* No data available                             */
    queErrAlreadyOpen,              /* The Que API is already open                   */
    queErrInvalidVersion,           /* The Que API is an incompatable version        */
    queErrComm,                     /* There was an error communicating with the API */
    queErrCmndUnavail,              /* The command is unavaialbe                     */
    queErrStillOpen,                /* Library is still open                         */
    queErrFail,                     /* General failure                               */
    queErrCancel                    /* Action cancelled by user                      */
    };

/*----------------------------------------------------------
Que Time Types
----------------------------------------------------------*/

/* Garmin Time. Seconds since (TODO)                                                 */
typedef uint32 QueGarminTimeT32;

/*----------------------------------------------------------
Que Unit Types
----------------------------------------------------------*/
typedef uint16 QueUnitPreferenceT16; enum
    {
    queDistancePreference   = 1,
    queHeadingPreference    = 2,
    queElevationPreference  = 3,
    queDepthPreference      = 4,
    queVelocityPreference   = 5
    };

typedef uint16 QueUnitT16; enum
    {
    queDistStatute  = 0,    /* Statute units                        */
    queDistMetric   = 1,    /* Metric untis                         */
    queDistFathoms  = 2,    /* Fathoms (depth only)                 */
    queDistFirst    = queDistStatute,
    queDistLast     = queDistFathoms,

    queHdgCardinal  = 256,    /* Cardinal letters                     */
    queHdgNumeric   = 257,    /* Numeric degrees                      */
    queHdgMils      = 258,    /* Mils                                 */
	queHdgFirst		= queHdgCardinal,
    queHdgLast      = queHdgMils,

    queVelStatute   = queHdgLast + 1,    /* Statute units                     */
    queVelMetric    = queHdgLast + 2,    /* Metric units                      */
    queVelNautical  = queHdgLast + 3,    /* Nautical units                    */
	queVelFirst		= queVelStatute,
    queVelLast      = queVelNautical
    };

/*----------------------------------------------------------
Que Sun/Moon Rise and Set Data Type
----------------------------------------------------------*/
typedef struct
    {
    QueGarminTimeT32    rise;      /* Rise time */
    QueGarminTimeT32    set;       /* Set time  */
    } QueRiseSetType;

/*----------------------------------------------------------
Que Unit Info Type
----------------------------------------------------------*/
typedef struct
    {
    char                    name[ 32 ];         /* Product name           */
    uint32                  unit_id;            /* Unit ID                */
    uint32                  version;            /* Software version * 100 */
    uint32                  product;            /* Product number         */
    char                    unlock_code[ 25 ];  /* Unlock code            */
    char                    unlock_desc[ 64 ];  /* Unlock description     */
    } QueDeviceInfoType;

/*----------------------------------------------------------
Que App Types
----------------------------------------------------------*/
typedef uint8 QueAppT8; enum
    {
    queAppQueMap,
    queAppQueFind,
    queAppQueGps,
    queAppQueRoutes,
    queAppQueTurns,
    queAppQueTrip,
    queAppQuePreferences,
    queAppMarkWaypoint,

    /* Unpublished */
    queAppStart,
    queAppTerminate,
    queAppPickMap,
    queAppPickFind,

    queAppInvalid = 255
    };

/*----------------------------------------------------------
Que Point Types
----------------------------------------------------------*/

#define queInvalidSemicircles   ( ( sint32 ) 0x80000000 )
#define queInvalidAltitude      ( ( float ) 1.0e25 )
#define queInvalidPointHandle   ( NULL )
#define quePointIdLen           ( 25 )
#define queInvalidSymbol        ( 0xFFFF )

typedef struct
    {
    sint32      lat;        /* Latitude  (semicircles)                      */
    sint32      lon;        /* Longitude (semicircles)                      */
    float       altMSL;     /* Altitude above mean sea level (meters)       */
    } QuePositionDataType;

typedef uint32 QuePointHandle;

typedef uint16 QueSymbolT16;

typedef struct
    {
    char                    id[ quePointIdLen ];    /* Point id       */
    QueSymbolT16            smbl;                   /* Point symbol   */
    QuePositionDataType     posn;                   /* Point position */
    } QuePointType;

/*----------------------------------------------------------
Address type for specifying addresses to find.
----------------------------------------------------------*/
typedef struct
    {
    const TCHAR *streetAddress;
    const TCHAR *city;
    const TCHAR *state;
    const TCHAR *country;
    const TCHAR *postalCode;
    } QueSelectAddressType;

/*----------------------------------------------------------
Que Waypoint Types (Used by ActiveSync/RAPI)
----------------------------------------------------------*/

#define queWaypointIndexInvalid  ( 0xFFFFFFFF )

typedef struct
    {
    char                    id[ 25 ];    /* Waypoint id                      */
    char                    cmnt[ 101 ]; /* Waypoint comment                 */
    sint32                  lat;         /* Waypoint position (semicircles)  */
    sint32                  lon;         /* Waypoint position (semicircles)  */
    float                   altMSL;      /* Altitude above mean sea level*/
    uint16                  smbl;        /* Waypoint symbol                  */
    uint16                  cat;         /* Waypoint category                */
    } QueWaypointType;

typedef char                    QueWaypointCategoryType[ 17 ];
typedef QueWaypointCategoryType QueWaypointCategoriesType[ 17 ];

/*----------------------------------------------------------
Que Track Types (Used by ActiveSync/RAPI)
----------------------------------------------------------*/

#define queTrackIndexInvalid ( 0xFFFFFFFF )

typedef struct
    {
    uint8       dspl;           /* Display options for the track                            */
    uint8       color;          /* Color of the track (IOP_color_type)                      */
    uint32      pt_cnt;         /* Number of points in the track                            */
    char        ident[ 51 ];    /* Null terminated string (max len of 50 + null terminator) */
    } QueTrackInfoType;

typedef struct
    {
    sint32              lat;        /* Point position lat (semicircles)         */
    sint32              lon;        /* Point position lon (semicircles)         */
    QueGarminTimeT32    time;       /* Time (Garmin time)                       */
    float               altMSL;     /* Altitude (Meters MSL)                    */
    uint8               strt;       /* True if this is the start of a new track */
    } QueTrackPointType;

/*----------------------------------------------------------
Que Map Types
----------------------------------------------------------*/

typedef TCHAR QueMapPathType[ 256 ];

typedef struct
    {
    uint32              usedBytes;
    uint32              freeBytes;
    char                name[ 32 ];
    QueMapPathType      path;
    uint8               isBasemap;
    } QueStorageLocation;

/* We only allow 8 storage locations */
typedef QueStorageLocation QueAllStorageLocations[ 8 ];

/*-----------------------------------------------------------------------------
                                   END OF FILE
-----------------------------------------------------------------------------*/

#endif  /* __QUEAPITYPES_H__ */
