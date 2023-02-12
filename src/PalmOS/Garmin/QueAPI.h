/*********************************************************************
*
*   HEADER NAME:
*       QueAPI.h - Include file for QueAPI Library
*
*   DESCRIPTION:
*       Library function prototypes for the calling application.
*       This is the file that the calling application should
*       include in order to get access to the routines in the
*       library.
*
* Copyright 2004 by Garmin Ltd. or its subsidiaries.
*
*********************************************************************/

#ifndef __QUEAPILIB_H__
#define __QUEAPILIB_H__

/*--------------------------------------------------------------------
                           GENERAL INCLUDES
--------------------------------------------------------------------*/

#include <PalmOS.h>

/*--------------------------------------------------------------------
                          LITERAL CONSTANTS
--------------------------------------------------------------------*/

/*----------------------------------------------------------
Library name for use with SysLibFind()
----------------------------------------------------------*/
#define queAPILibName       "QueAPILib"

/*----------------------------------------------------------
Values for use with SysLibLoad()
----------------------------------------------------------*/
#define queAPILibType       'libr'
#define queAPILibCreator    'QAPI'

/*----------------------------------------------------------
Library version
----------------------------------------------------------*/
#define queAPIVersion       1

/*----------------------------------------------------------
Trap numbers.
----------------------------------------------------------*/
#define queAPILibTrapClosePoint                 ( sysLibTrapCustom +  4 )
#define queAPILibTrapCreatePoint                ( sysLibTrapCustom +  5 )
#define queAPILibTrapCreatePointFromAddress     ( sysLibTrapCustom +  6 )
#define queAPILibTrapCreatePointFromEvent       ( sysLibTrapCustom +  7 )
#define queAPILibTrapDeserializePoint           ( sysLibTrapCustom +  8 )
#define queAPILibTrapGetPointInfo               ( sysLibTrapCustom +  9 )
#define queAPILibTrapHandleEvent                ( sysLibTrapCustom + 10 )
#define queAPILibTrapRouteToPoint               ( sysLibTrapCustom + 11 )
#define queAPILibTrapSelectAddressFromFind      ( sysLibTrapCustom + 12 )
#define queAPILibTrapSelectPointFromFind        ( sysLibTrapCustom + 13 )
#define queAPILibTrapSelectPointFromMap         ( sysLibTrapCustom + 14 )
#define queAPILibTrapSerializePoint             ( sysLibTrapCustom + 15 )
#define queAPILibTrapSetRouteToItem             ( sysLibTrapCustom + 16 )
#define queAPILibTrapViewPointDetails           ( sysLibTrapCustom + 17 )
#define queAPILibTrapViewPointOnMap             ( sysLibTrapCustom + 18 )

#define queAPILibTrapCount                      ( sysLibTrapCustom + 19 )

/*----------------------------------------------------------
Other
----------------------------------------------------------*/
#define firstQueAPIErr          ( appErrorClass | 1 )

/*--------------------------------------------------------------------
                                TYPES
--------------------------------------------------------------------*/

/*----------------------------------------------------------
Basic data types
----------------------------------------------------------*/

typedef UInt8   uint8;
typedef UInt16  uint16;
typedef UInt32  uint32;
typedef Int8    sint8;
typedef Int16   sint16;
typedef Int32   sint32;
typedef Char    TCHAR;

/*--------------------------------------------------------------------
                           PROJECT INCLUDES
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                         SHARED TYPE INCLUDES
--------------------------------------------------------------------*/

#include "QueAPITypes.h"

/*--------------------------------------------------------------------
                           MEMORY CONSTANTS
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                              VARIABLES
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                                MACROS
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                              PROCEDURES
--------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
*
*   PROCEDURE NAME:
*       QueAPIOpen - Open Library
*
*   DESCRIPTION:
*       Opens the QueAPI library and prepares it for use.  Called
*       by any application or library that wants to use the services
*       that the QueAPI library provides.  The caller provides the
*       version of the QueAPI library that it was written to use;
*       queErrInvalidVersion is returned if this version of the
*       library is not compatible with the supplied version.
*
*       QueAPIOpen *must* be called before calling any other Que
*       procedures. If an error occurs during initialization,
*       QueAPIOpen returns the error code and exits cleanly.
*
*********************************************************************/
QueErrT16 QueAPIOpen
    ( const UInt16 refNum
    , const UInt16 version
    )
    SYS_TRAP( sysLibTrapOpen);

/*********************************************************************
*
*   PROCEDURE NAME:
*       QueAPIClose - Close Library
*
*   DESCRIPTION:
*       Closes QueAPI Library and disposes of the global data memory
*       if required. Called by any application or library that's been
*       using QueAPI Library and is now finished with it.
*
*       This shouldn't be called if QueAPIOpen failed. If other apps
*       are still using the QueAPI library we return the code
*       queErrStillOpen to tell the calling app not to call
*       SysLibRemove.
*
*********************************************************************/
QueErrT16 QueAPIClose
    ( const UInt16 refNum
    )
    SYS_TRAP( sysLibTrapClose );

/*********************************************************************
*
*   PROCEDURE NAME:
*       QueClosePoint - Close Point
*
*   DESCRIPTION:
*       Closes the handle to a point.  This must be called for
*       all open point handles before exiting your application.
*
*   NOTES:
*       After calling this procedure, the caller should set the
*       point handle to queInvalidPointHandle to indicate that
*       it has been closed.
*
*********************************************************************/
QueErrT16 QueClosePoint
    ( const UInt16          refNum      /* Input */
    , const QuePointHandle  point       /* Input */
    )
    SYS_TRAP( queAPILibTrapClosePoint );

/*********************************************************************
*
*   PROCEDURE NAME:
*       QueCreatePoint - Create Point
*
*   DESCRIPTION:
*       Creates a point handle with the specified data.
*       QueClosePoint must be called to close the point handle
*       before exiting your application.
*
*********************************************************************/
QueErrT16 QueCreatePoint
    ( const UInt16          refNum      /* Input    */
    , const QuePointType    *pointData  /* Input    */
    , QuePointHandle        *point      /* Output   */
    )
    SYS_TRAP( queAPILibTrapCreatePoint );

/*********************************************************************
*
*   PROCEDURE NAME:
*       QueCreatePointFromAddress - Create Point From Address
*
*   DESCRIPTION:
*       Creates a Point at the specified address.  Returns the point
*       handle through a launch of the specified application. If
*       a single address match cannot be found an invalid point
*       handle will be returned in the launch.
*
*********************************************************************/
QueErrT16 QueCreatePointFromAddress
    ( const UInt16                  refNum              /* Input  */
    , const QueSelectAddressType    *address            /* Input  */
    , const UInt32                  relaunchAppCreator  /* Input  */
    )
    SYS_TRAP( queAPILibTrapCreatePointFromAddress );

/*********************************************************************
*
*   PROCEDURE NAME:
*       QueCreatePointFromEvent - Create Point From Event
*
*   DESCRIPTION:
*       Creates a point handle from an event that contains point
*       data. QueClosePoint must be called to close the point
*       handle before exiting your application.
*
*   NOTES:
*       This should only be called if QueHandleEvent return true.
*
*********************************************************************/
QueErrT16 QueCreatePointFromEvent
    ( const UInt16      refNum      /* Input  */
    , const EventType   *event      /* Input  */
    , QuePointHandle    *point      /* Output */
    )
    SYS_TRAP( queAPILibTrapCreatePointFromEvent );

/*********************************************************************
*
*   PROCEDURE NAME:
*       QueDeserializePoint - Deserialze Point
*
*   DESCRIPTION:
*       Creates a handle to a point from serialized data.
*
*********************************************************************/
QueErrT16 QueDeserializePoint
    ( const UInt16      refNum          /* Input  */
    , const void        *pointData      /* Input  */
    , const UInt32      pointDataSize   /* Input  */
    , QuePointHandle    *point          /* Output */
    )
    SYS_TRAP( queAPILibTrapDeserializePoint );

/*********************************************************************
*
*   PROCEDURE NAME:
*       QueGetPointInfo - Get Point Information
*
*   DESCRIPTION:
*       Returns the point information for the given point
*       handle.
*
*********************************************************************/
QueErrT16 QueGetPointInfo
    ( const UInt16          refNum      /* Input  */
    , const QuePointHandle  point       /* Input  */
    , QuePointType          *pointInfo  /* Output */
    )
    SYS_TRAP( queAPILibTrapGetPointInfo );

/*********************************************************************
*
*   PROCEDURE NAME:
*       QueHandleEvent - Handle Event
*
*   DESCRIPTION:
*       Handles QueAPI library events.  Returns true if the
*       event contains point data.
*
*********************************************************************/
Boolean QueHandleEvent
    ( const UInt16      refNum      /* Input */
    , const EventType   *event      /* Input */
    )
    SYS_TRAP( queAPILibTrapHandleEvent );

/*********************************************************************
*
*   PROCEDURE NAME:
*       QueRouteToPoint - Route To Point
*
*   DESCRIPTION:
*       Creates a route from the current location to the point.
*       If showMap is true, This exits the current application
*       and switches to the map application, centered on the
*       vehicle.
*
*********************************************************************/
QueErrT16 QueRouteToPoint
    ( const UInt16          refNum      /* Input */
    , const QuePointHandle  point       /* Input */
    , const Boolean         showMap     /* Input */
    )
    SYS_TRAP( queAPILibTrapRouteToPoint );

/*********************************************************************
*
*   PROCEDURE NAME:
*       QueSelectAddressFromFind - Select Address From Find
*
*   DESCRIPTION:
*       Allows the user to create a point by selecting an address
*       from the find address form. Returns the point handle
*       through a launch of the specified application.
*
*   NOTES:
*       If Try To Create First is false, the address form will
*       always be displayed, which will be pre-filled with the
*       given address data.
*
*       If Try To Create First is true, this acts much like
*       QueCreatePointFromAddress, in that it will attempt to find
*       a single address match in the data.  if a single match is
*       found, it will be returned and the find form will not be
*       displayed.  If a single match cannot be found, the find
*       address form will be displayed, which will be pre-filled
*       with the given address data.
*
*       If the user cancels the find, an invalid point handle will
*       be returned in the launch.
*
*********************************************************************/
QueErrT16 QueSelectAddressFromFind
    ( const UInt16                  refNum              /* Input */
    , const QueSelectAddressType    *address            /* Input */
    , const UInt32                  relaunchAppCreator  /* Input */
    , const Boolean                 tryToCreateFirst    /* Input */
    )
    SYS_TRAP( queAPILibTrapSelectAddressFromFind );

/*********************************************************************
*
*   PROCEDURE NAME:
*       QueSelectPointFromFind - Select Point From Find
*
*   DESCRIPTION:
*       Allows the user to use find to select a point. Returns the
*       point handle through a launch of the specified application.
*
*   NOTES:
*       If the user cancels finding the point, an invalid point
*       handle will be returned in the launch.
*
*********************************************************************/
QueErrT16 QueSelectPointFromFind
    ( const UInt16  refNum              /* Input */
    , const UInt32  relaunchAppCreator  /* Input */
    )
    SYS_TRAP( queAPILibTrapSelectPointFromFind );

/*********************************************************************
*
*   PROCEDURE NAME:
*       QueSelectPointFromMap - Select Point From Map
*
*   DESCRIPTION:
*       Allows the user to use a map to select a point. Returns the
*       point handle through a launch of the specified application.
*
*   NOTES:
*       If the user cancels finding the point on the map, an invalid
*       point handle will be returned in the launch.
*
*********************************************************************/
QueErrT16 QueSelectPointFromMap
    ( const UInt16  refNum              /* Input */
    , const UInt32  relaunchAppCreator  /* Input */
    )
    SYS_TRAP( queAPILibTrapSelectPointFromMap );

/*********************************************************************
*
*   PROCEDURE NAME:
*       QueSerializePoint - Serialze Point
*
*   DESCRIPTION:
*       Returns the data needed to recreate the point handle to the
*       caller. The point can then be reconstituted by a call to
*       QueDeserializePoint. This is used for long term storage of
*       points.
*
*   NOTES:
*       The procedure returns the size of the serialized data. If
*       the buffer passed in is not big enough, no data will be
*       written into the buffer.
*
*       Typical usage is to call the procedure once with a size
*       of zero, then use the returned size to allocate a buffer
*       of the needed size. Then call the procedure again with
*       this proper sized buffer.
*
*********************************************************************/
UInt32 QueSerializePoint
    ( const UInt16          refNum          /* Input  */
    , const QuePointHandle  point           /* Input  */
    , void                  *pointData      /* Output */
    , const UInt32          pointDataSize   /* Input  */
    )
    SYS_TRAP( queAPILibTrapSerializePoint );

/*********************************************************************
*
*   PROCEDURE NAME:
*       QueSetRouteToItem - Set Route To Item
*
*   DESCRIPTION:
*       Set the route form "Route To" item to the point.  The
*       "Route To" item will be cleared when the library is closed;
*       it can also be cleared by the calling application by passing
*       an invalid point handle.
*
*********************************************************************/
QueErrT16 QueSetRouteToItem
    ( const UInt16          refNum      /* Input */
    , const QuePointHandle  point       /* Input */
    )
    SYS_TRAP( queAPILibTrapSetRouteToItem );

/*********************************************************************
*
*   PROCEDURE NAME:
*       QueViewPointDetails - View Point Details
*
*   DESCRIPTION:
*       Displays a modal form containing a map and other
*       details about the point.
*
*********************************************************************/
QueErrT16 QueViewPointDetails
    ( const UInt16          refNum      /* Input */
    , const QuePointHandle  point       /* Input */
    )
    SYS_TRAP( queAPILibTrapViewPointDetails );

/*********************************************************************
*
*   PROCEDURE NAME:
*       QueViewPointOnMap - View Point On Map
*
*   DESCRIPTION:
*       This exits the current application and switches to the
*       map application, centered on the specified point.  There
*       is no easy way to return to the calling application.
*
*********************************************************************/
QueErrT16 QueViewPointOnMap
    ( const UInt16          refNum      /* Input */
    , const QuePointHandle  point       /* Input */
    )
    SYS_TRAP( queAPILibTrapViewPointOnMap );

#ifdef __cplusplus
}
#endif

#endif /* __QUEAPILIB_H__ */
