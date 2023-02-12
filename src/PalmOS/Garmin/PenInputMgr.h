/*********************************************************************
*
*   HEADER NAME:
*       PenInputMgr.h - Pen Input Manager 68K Include
*
*   Copyright 2002-2003 by Garmin Ltd. or its subsidiaries.
*
*********************************************************************/

#ifndef _PENINPUTMGR_H
#define _PENINPUTMGR_H

/*--------------------------------------------------------------------
                           GENERAL INCLUDES
--------------------------------------------------------------------*/

#include <PalmTypes.h>
#include <LibTraps.h>
#include "PenInputMgrCommon.h"
#include "PenInputMgrSelectorNums.h"

/*--------------------------------------------------------------------
                          LITERAL CONSTANTS
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                                TYPES
--------------------------------------------------------------------*/

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
                                MACROS
--------------------------------------------------------------------*/

/*----------------------------------------------------------
Selector based system trap macro
----------------------------------------------------------*/
#define PIN_TRAP( sel ) \
    _SYSTEM_API( _CALL_WITH_SELECTOR )( _SYSTEM_TABLE, sysTrapPinsDispatch, sel )

/*--------------------------------------------------------------------
                              PROCEDURES
--------------------------------------------------------------------*/

/*********************************************************************
*
*   PROCEDURE NAME:
*       PINGetInputAreaState - Get Input Area State
*
*   DESCRIPTION:
*       Returns the current state of the input area.
*
*********************************************************************/
PinInputAreaStateT16 PINGetInputAreaState( void )
    PIN_TRAP( pinPINGetInputAreaState );

/*********************************************************************
*
*   PROCEDURE NAME:
*       PINGetInputTriggerState - Get Input Trigger State
*
*   DESCRIPTION:
*       Returns the current state of the input trigger.
*
*********************************************************************/
PinInputTriggerStateT16 PINGetInputTriggerState( void )
    PIN_TRAP( pinPINGetInputTriggerState );

/*********************************************************************
*
*   PROCEDURE NAME:
*       PINSetInputAreaState - Set Input Area State
*
*   DESCRIPTION:
*       Sets the state of the input area.
*
*********************************************************************/
Err PINSetInputAreaState( const PinInputAreaStateT16 state )
    PIN_TRAP( pinPINSetInputAreaState );

/*********************************************************************
*
*   PROCEDURE NAME:
*       PINSetInputTriggerState - Set Input Trigger State
*
*   DESCRIPTION:
*       Sets the state of the input trigger.
*
*********************************************************************/
Err PINSetInputTriggerState( const PinInputTriggerStateT16 state )
    PIN_TRAP( pinPINSetInputTriggerState );

/*********************************************************************
*
*   PROCEDURE NAME:
*       FrmGetDIAPolicyAttr - Get Dynamic Input Area Policy Attribute
*
*   DESCRIPTION:
*       Returns a form’s dynamic input area policy.
*
*********************************************************************/
FrmDIAPolicyT16 FrmGetDIAPolicyAttr( FormPtr formP )
    PIN_TRAP( pinFrmGetDIAPolicyAttr );

/*********************************************************************
*
*   PROCEDURE NAME:
*       FrmSetDIAPolicyAttr - Set Dynamic Input Area Policy Attribute
*
*   DESCRIPTION:
*       Sets a form’s dynamic input area policy.
*
*********************************************************************/
Err FrmSetDIAPolicyAttr( FormPtr formP, const FrmDIAPolicyT16 diaPolicy )
    PIN_TRAP( pinFrmSetDIAPolicyAttr );

/*********************************************************************
*
*   PROCEDURE NAME:
*       WinSetConstraintsSize - Set Constraints Size
*
*    DESCRIPTION:
*       Sets the maximum, preferred, and minimum size constraints
*       for a window.
*
*********************************************************************/
Err WinSetConstraintsSize
    ( WinHandle    winHandle
    , const Coord  minHeight
    , const Coord  prefHeight
    , const Coord  maxHeight
    , const Coord  minWidth
    , const Coord  prefWidth
    , const Coord  maxWidth
    )
    PIN_TRAP( pinWinSetConstraintsSize );

#endif

