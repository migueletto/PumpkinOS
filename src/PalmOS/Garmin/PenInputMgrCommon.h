/*********************************************************************
*
*   HEADER NAME:
*       PenInputMgrCommon.h - Pen Input Manager common include file
*
* Copyright 2002-2003 by Garmin Ltd. or its subsidiaries.
*
*********************************************************************/

#ifndef _PENINPUTMGRCOMMON_H
#define _PENINPUTMGRCOMMON_H

/*--------------------------------------------------------------------
                           GENERAL INCLUDES
--------------------------------------------------------------------*/

#include <PalmTypes.h>
#include <ErrorBase.h>


/*--------------------------------------------------------------------
                                MACROS
--------------------------------------------------------------------*/

/*----------------------------------------------------------
Set len bits, starting at bit b.
----------------------------------------------------------*/
#define SetBits( b, len )      ( ( ( 1U << ( ( len ) - 1 ) ) - 1U + ( 1U << ( ( len ) - 1 ) ) ) << ( b ) )

/*----------------------------------------------------------
Returns maximum signed integral value for any valid argument
for sizeof.
----------------------------------------------------------*/
#define MaxSintVal( t )     ( SetBits( 0, ( sizeof( t ) * 8 ) - 1 ) )


/*--------------------------------------------------------------------
                          LITERAL CONSTANTS
--------------------------------------------------------------------*/

/*----------------------------------------------------------
Pen Input Manager Features
----------------------------------------------------------*/
#define pinCreator           'pins'
#define pinFtrAPIVersion     1

/*----------------------------------------------------------
PINS API version number
----------------------------------------------------------*/
#define pinAPIVersion1_0     0x01000000
#define pinAPIVersion        pinAPIVersion1_0

/*----------------------------------------------------------
Maximum sizes for setting constraint sizes
----------------------------------------------------------*/
#define pinMaxConstraintSize    MaxSintVal( Coord )

/*----------------------------------------------------------
Pen Input Manager errors
----------------------------------------------------------*/
#define pinErrNoSoftInputArea   ( appErrorClass | 1 )   /* There is no dynamic input area on this device    */
#define pinErrInvalidParam      ( appErrorClass | 2 )   /* You have entered an invalid state parameter      */
#define pinErrPinletNotFound    ( appErrorClass | 3 )   /* There is no pinlet with the specified name       */

/*----------------------------------------------------------
Notification
----------------------------------------------------------*/
#define sysNotifyDisplayResizedEvent    'scrs'

/*----------------------------------------------------------
Trap
----------------------------------------------------------*/
#define sysTrapPinsDispatch 0xA470


/*--------------------------------------------------------------------
                                TYPES
--------------------------------------------------------------------*/

/*----------------------------------------------------------
Input area states
----------------------------------------------------------*/
typedef UInt16 PinInputAreaStateT16; enum
    { pinInputAreaOpen      /* The dynamic input area is being displayed        */
    , pinInputAreaClosed    /* The dynamic input area is not being displayed    */
    , pinInputAreaNone      /* There is no dynamic input area                   */

    , pinInputAreaCount
    , pinInputAreaRedraw
    };


/*----------------------------------------------------------
Input trigger states
----------------------------------------------------------*/
typedef UInt16 PinInputTriggerStateT16; enum
    { pinInputTriggerEnabled    /* The status bar icon is enabled   */
    , pinInputTriggerDisabled   /* The status bar icon is disabled  */
    , pinInputTriggerNone       /* There is no dynamic input area   */

    , pinInputTriggerCount
    };


/*----------------------------------------------------------
Form Dynamic Input Area Policies
----------------------------------------------------------*/
typedef UInt16 FrmDIAPolicyT16; enum
    { frmDIAPolicyStayOpen  /* The dynamic input area stays open                        */
    , frmDIAPolicyCustom    /* The dynamic input area is controlled by the application  */

    , frmDIAPolicyCount
    };


/*--------------------------------------------------------------------
                           PROJECT INCLUDES
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                           MEMORY CONSTANTS
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                              VARIABLES
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                              PROCEDURES
--------------------------------------------------------------------*/

#endif
