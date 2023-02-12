/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: SysEvtMgr.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *		Header for the System Event Manager
 *
 *****************************************************************************/

#ifndef __SYSEVTMGR_H__
#define __SYSEVTMGR_H__

#include <PalmTypes.h>
#include <SysEvent.h>

/************************************************************
 * System Event Manager Errors
 *************************************************************/
#define	evtErrParamErr			(evtErrorClass | 1)
#define	evtErrQueueFull		(evtErrorClass | 2)
#define	evtErrQueueEmpty		(evtErrorClass | 3)


/************************************************************
 * Commands for EvtSetAutoOffTimer()
 *************************************************************/
typedef enum
{
	SetAtLeast,		// turn off in at least xxx seconds
	SetExactly,		// turn off in xxx seconds
	SetAtMost,		// turn off in at most xxx seconds
	SetDefault,		// change default auto-off timeout to xxx seconds
	ResetTimer		// reset the timer to the default auto-off timeout
	
} EvtSetAutoOffCmd;


/************************************************************
 * Pen button info structure. This structure is used
 *  to hold the bounds of each button on the silk screen and
 *  the ascii code and modifiers byte that each will generate
 *	 when tapped by the user.
 *************************************************************/
typedef struct PenBtnInfoType {
	RectangleType	boundsR;						// bounding rectangle of button
	WChar				asciiCode;					// ascii code for key event
	UInt16			keyCode;						// virtual key code for key event
	UInt16			modifiers;					// modifiers for key event
	} PenBtnInfoType;
typedef PenBtnInfoType* PenBtnInfoPtr;
	
typedef struct PenBtnListType {
	UInt16			numButtons;					// Count of number of buttons
	PenBtnInfoType	buttons[1];					// Placeholder for one or more buttons
	} PenBtnListType;


/************************************************************
 * Silkscreen area info structure. An array of these structures
 * is returned by the EvtGetSilkscreenAreaList function.
 *************************************************************/

// Different types of rectangles on the display. For new vendor areas,
// the type should be set to the vendor's creator code, as assigned
// by 3Com's Partner Engineering group.
#define	silkscreenRectScreen		'scrn'
#define	silkscreenRectGraffiti	'graf'

// Values for SilkscreenAreaType.index if areaType = silkscreenRectGraffiti
#define	alphaGraffitiSilkscreenArea		0
#define	numericGraffitiSilkscreenArea		1

// One silkscreen area. The areaType field tells us which type of
// area it is, while the index field has different meanings depending
// on the area type.
typedef struct SilkscreenAreaType {
	RectangleType			bounds;
	UInt32					areaType;	// four byte creator code.
	UInt16					index;
	} SilkscreenAreaType;
	

/************************************************************
 * System Event Manager procedures
 *************************************************************/
#ifdef __cplusplus
extern "C" {  
#endif


//-----------------------------------------------------------------
// High Level Calls
//------------------------------------------------------------------
Err				EvtSysInit(void)
						SYS_TRAP(sysTrapEvtSysInit);

// Return next "System" event. This routine will send strokes to Graffiti as necessary
//  and return a key event. Otherwise, it will return a simple pen down or pen
//  up event, or put the processor to sleep for a max time of 'timeout' if 
// no events are available. 
void				EvtGetSysEvent(SysEventType *eventP, Int32 timeout)
						SYS_TRAP(sysTrapEvtGetSysEvent);


// Return true if there is a low level system event (pen or key) available
Boolean			EvtSysEventAvail(Boolean ignorePenUps)
						SYS_TRAP(sysTrapEvtSysEventAvail);



// Translate a stroke in the silk screen area to a key event
Err				EvtProcessSoftKeyStroke(PointType *startPtP, PointType *endPtP)
						SYS_TRAP(sysTrapEvtProcessSoftKeyStroke);


//-----------------------------------------------------------------
// Pen Queue Utilties
//------------------------------------------------------------------

// Replace current pen queue with another of the given size
Err				EvtSetPenQueuePtr(MemPtr penQueueP, UInt32 size)
						SYS_TRAP(sysTrapEvtSetPenQueuePtr);

// Return size of current pen queue in bytes
UInt32			EvtPenQueueSize(void)
						SYS_TRAP(sysTrapEvtPenQueueSize);

// Flush the pen queue
Err				EvtFlushPenQueue(void)
						SYS_TRAP(sysTrapEvtFlushPenQueue);


// Append a point to the pen queue. Passing -1 for x and y means 
//  pen-up (terminate the current stroke). Called by digitizer interrupt routine
Err				EvtEnqueuePenPoint(PointType *ptP)
						SYS_TRAP(sysTrapEvtEnqueuePenPoint);


// Return the stroke info for the next stroke in the pen queue. This MUST
//  be the first call when removing a stroke from the queue
Err				EvtDequeuePenStrokeInfo(PointType *startPtP, PointType *endPtP)
						SYS_TRAP(sysTrapEvtDequeuePenStrokeInfo);

// Dequeue the next point from the pen queue. Returns non-0 if no
//  more points. The point returned will be (-1,-1) at the end
//  of the stroke.
Err				EvtDequeuePenPoint(PointType *retP)
						SYS_TRAP(sysTrapEvtDequeuePenPoint);


// Flush the entire stroke from the pen queue and dispose it
Err				EvtFlushNextPenStroke()
						SYS_TRAP(sysTrapEvtFlushNextPenStroke);




//-----------------------------------------------------------------
// Key Queue Utilties
//------------------------------------------------------------------

// Replace current key queue with another of the given size. This routine will
//  intialize the given key queue before installing it
Err				EvtSetKeyQueuePtr(MemPtr keyQueueP, UInt32 size)
						SYS_TRAP(sysTrapEvtSetKeyQueuePtr);

// Return size of current key queue in bytes
UInt32			EvtKeyQueueSize(void)
						SYS_TRAP(sysTrapEvtKeyQueueSize);

// Flush the key queue
Err				EvtFlushKeyQueue(void)
						SYS_TRAP(sysTrapEvtFlushKeyQueue);


// Append a key to the key queue. 
Err				EvtEnqueueKey(WChar ascii, UInt16 keycode, UInt16 modifiers)
						SYS_TRAP(sysTrapEvtEnqueueKey);

// Return true of key queue empty.
Boolean			EvtKeyQueueEmpty(void)
						SYS_TRAP(sysTrapEvtKeyQueueEmpty);


// Pop off the next key event from the key queue and fill in the given
//  event record structure. Returns non-zero if there aren't any keys in the
//  key queue. If peek is non-zero, key will be left in key queue.
Err				EvtDequeueKeyEvent(SysEventType *eventP, UInt16 peek)
						SYS_TRAP(sysTrapEvtDequeueKeyEvent);


//-----------------------------------------------------------------
// Silkscreen information calls
//------------------------------------------------------------------

// Return pointer to the pen based button list
const PenBtnInfoType* EvtGetPenBtnList(UInt16* numButtons)
						SYS_TRAP(sysTrapEvtGetPenBtnList);

// Return pointer to the silkscreen area list
const SilkscreenAreaType* EvtGetSilkscreenAreaList(UInt16* numAreas)
						SYS_TRAP(sysTrapEvtGetSilkscreenAreaList);
						

//-----------------------------------------------------------------
// General Utilities
//------------------------------------------------------------------
// Force the system to wake-up. This will result in a null event being
//  sent to the current app.
Err				EvtWakeup(void)
						SYS_TRAP(sysTrapEvtWakeup);

// Force the system to wake-up. This will NOT result in a null event being
//  sent to the current app.
Err 			EvtWakeupWithoutNilEvent()
							SYS_TRAP(sysTrapEvtWakeupWithoutNilEvent);

// Reset the auto-off timer. This is called by the SerialLink Manager in order
//  so we don't auto-off while receiving data over the serial port.
Err				EvtResetAutoOffTimer(void)
						SYS_TRAP(sysTrapEvtResetAutoOffTimer);

Err				EvtSetAutoOffTimer(EvtSetAutoOffCmd cmd, UInt16 timeout)
						SYS_TRAP(sysTrapEvtSetAutoOffTimer);

// Set Graffiti enabled or disabled.
void	EvtEnableGraffiti(Boolean enable)
						SYS_TRAP(sysTrapEvtEnableGraffiti);
						
// Force a NullEvent at or before tick
Boolean EvtSetNullEventTick(UInt32 tick)
						SYS_TRAP(sysTrapEvtSetNullEventTick);


#ifdef __cplusplus
}
#endif


/************************************************************
 * Assembly Function Prototypes
 *************************************************************/
#define	_EvtEnqueuePenPoint		\
				ASM_SYS_TRAP(sysTrapEvtEnqueuePenPoint)



#endif //__SYSEVTMGR_H__
