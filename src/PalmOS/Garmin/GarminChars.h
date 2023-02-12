/*********************************************************************
*
*   HEADER NAME:
*       GarminChars.h - Garmin-specific virtual character definitions
*
* Copyright 2002-2003 by Garmin Ltd. or its subsidiaries.
*
*********************************************************************/

#ifndef _GARMINCHARS_H
#define _GARMINCHARS_H

/*--------------------------------------------------------------------
                           GENERAL INCLUDES
--------------------------------------------------------------------*/

/*--------------------------------------------------------------------
                          LITERAL CONSTANTS
--------------------------------------------------------------------*/

/*----------------------------------------------------------
Values placed in the keyCode field of keyDownEvents for
Garmin keys.  The values placed in the chr field are in
the comments.
----------------------------------------------------------*/

#define vchrGarminMin               ( 0x1A00 )

#define vchrGarminThumbWheelUp      ( vchrGarminMin +  0 )  /* vchrPageUp           */
#define vchrGarminThumbWheelDown    ( vchrGarminMin +  1 )  /* vchrPageDown         */
#define vchrGarminThumbWheelIn      ( vchrGarminMin +  2 )  /* chrCarriageReturn    */
#define vchrGarminEscape            ( vchrGarminMin +  3 )  /* vchrGarminEscape     */
#define vchrGarminEscapeHeld        ( vchrGarminMin +  4 )  /* vchrGarminEscapeHeld */
#define vchrGarminRecord            ( vchrGarminMin +  5 )  /* vchrGarminRecord     */
#define vchrGarminRecordHeld        ( vchrGarminMin +  6 )  /* vchrGarminRecordHeld */

#define vchrGarminMax               ( 0x1A0F )

/*----------------------------------------------------------
Values returned by KeyCurrentState() for Garmin keys.
----------------------------------------------------------*/
#define keyBitGarminThumbWheelUp        0x10000000
#define keyBitGarminThumbWheelDown      0x04000000
#define keyBitGarminThumbWheelIn        0x08000000
#define keyBitGarminEscape              0x01000000
#define keyBitGarminRecord              0x40000000


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
Is the keydown event a Garmin key?
eventP is a keyDown event pointer.
----------------------------------------------------------*/
#define GarminKeyIsGarmin( eventP ) ( ( ( eventP )->data.keyDown.keyCode >= vchrGarminMin ) && ( ( eventP )->data.keyDown.keyCode <= vchrGarminMax ) )

/*--------------------------------------------------------------------
                              PROCEDURES
--------------------------------------------------------------------*/

#endif /* _GARMINCHARS_H */



