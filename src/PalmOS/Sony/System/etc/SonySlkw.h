/******************************************************************************
 *                                                                            *
 *                 (C) Copyright 2001, Sony Corporation                       *
 *                                                                            *
 *----------------------------------------------------------------------------*
 *                                                                            *
 *    <IDENTIFICATION>                                                        *
 *       file name    : $Workfile: SonySlkw.h $
 *                                                                            *
 *    <PROFILE>                                                               *
 *       Active Input Extension                                               *
 *                                                                            *
 *    <HISTORY>                                                               *
 *       Last Modified: $Date:
 *                                                                            *
 ******************************************************************************/
/* this file is best viewed by setting TAB-stop as 3 */

#ifndef __SILKWARE_H__
#define __SILKWARE_H__

#include <PalmOS.h>		// for bitmapRsc

#include <PalmTypes.h> 
#include <SystemPublic.h>

/* ------------------------------------- */
/*				Constant def					  */
/* ------------------------------------- */

/* define for SilkWare Icon */
/*  Icon Size Max :: Height:9(18 for DensityDouble), Width:15(30 for DensityDouble) */
#define SlkwAppIconFamilyID 1001

typedef enum SilkEventEnum {
	silkEventNull = 0,
	silkEventOpen,
	silkEventClose,
	silkEventStart,
	silkEventStop,
	silkEventDraw,
	silkEventDispChange,
	silkEventPenDown,
	silkEventPenMove,
	silkEventPenUp,
	silkEventTimerFired,
	silkEventDoCommand
} SilkEventEnumType;

/* ------------------------------------- */
/*				Type def							  */
/* ------------------------------------- */
typedef struct SilkEvent {
   SilkEventEnumType		eType;
   UInt32					depth;
   UInt32					reserved;
	union {
		struct silkOpen {					// silkEventOpen
			UInt32			width;
			UInt32			height;
		} open;

		PointType			point;		// silkEventPenDown
												// silkEventPenMove
												// silkEventPenUp

		struct doCmmd {					// silkEventDoCommand
			UInt16			cmdType;
			UInt32			data1;
			UInt32			data2;
			Boolean			fromARM;
		} cmd;

		//Others
												// silkEventClose
												// silkEventSleep
												// silkEventWake
												// silkEventStart
												// silkEventStop
												// silkEventDraw
												// silkEventDispChange
												// silkEventTimerFired
	
	} data;
} SilkEventType;
typedef SilkEventType *SilkEventP;

typedef Err (*SilkCallbackProcPtr)(void *silkWareP, SilkEventP eventP);


/* StatusBar SilkWare use only */
#define stsBarCmdMinEnable			(0x0000)
	// data1 is {true, false}
#define stsBarCmdResizeDone		(0x0001)
	// data1 is {vskResizeMax, vskResizeMin}

#endif //__SILKWARE_H__
