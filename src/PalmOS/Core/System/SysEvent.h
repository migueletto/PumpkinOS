/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: SysEvent.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *        This file defines event structures and routines.
 *
 *****************************************************************************/

#ifndef __SYSEVENT_H__
#define __SYSEVENT_H__

#include <PalmTypes.h>
#include <CoreTraps.h>
#include <Font.h>
#include <Rect.h>
#include <Window.h>


typedef enum {
	sysEventNilEvent 			= 0,
	sysEventPenDownEvent,
	sysEventPenUpEvent,
	sysEventPenMoveEvent,
	sysEventKeyDownEvent,
	sysEventWinEnterEvent,
	sysEventWinExitEvent,
	sysEventAppStopEvent 		= 22,
	sysEventTsmConfirmEvent 	= 35,
	sysEventTsmFepButtonEvent,
	sysEventTsmFepModeEvent,
	sysEventFrmTitleChangedEvent,

	// add future UI level events in this numeric space
	// to save room for new system level events
	sysEventNextUIEvent = 0x0800,

	// <chg 2-25-98 RM> Equates added for library events
	sysEventFirstINetLibEvent 	= 0x1000,
	sysEventFirstWebLibEvent 	= 0x1100,
	
	// Can't add these to the system event range because PACE won't pass them through,
	// add them to the licensee range here:
	sysEventKeyUpEvent 			= 0x4000,
	sysEventKeyHoldEvent 		= 0x4001,
	sysEventFrmObjectFocusTakeEvent 	= 0x4002,
	sysEventFrmObjectFocusLostEvent 	= 0x4003,
	
	// <chg 10/9/98 SCL> Changed firstUserEvent from 32767 (0x7FFF) to 0x6000
	// Enums are signed ints, so 32767 technically only allowed for ONE event.
	sysEventFirstUserEvent 		= 0x6000,

	sysEventLastUserEvent  		= 0x7FFF
} SysEventsEnum;



// keyDownEvent modifers
#define shiftKeyMask       0x0001
#define capsLockMask       0x0002
#define numLockMask        0x0004
#define commandKeyMask     0x0008
#define optionKeyMask      0x0010
#define controlKeyMask     0x0020
#define autoRepeatKeyMask  0x0040      // True if generated due to auto-repeat
#define doubleTapKeyMask   0x0080      // True if this is a double-tap event
#define poweredOnKeyMask   0x0100      // True if this is a double-tap event
#define appEvtHookKeyMask  0x0200      // True if this is an app hook key
#define libEvtHookKeyMask  0x0400      // True if this is a library hook key

// define mask for all "virtual" keys
#define virtualKeyMask	(appEvtHookKeyMask | libEvtHookKeyMask | commandKeyMask)


// Event timeouts
#define  evtWaitForever    -1
#define  evtNoWait          0

struct _GenericEventType {
	UInt16         datum[8];
   };

struct _PenUpEventType {
   PointType      start;            // display coord. of stroke start
   PointType      end;              // display coord. of stroke start
   };

struct _KeyDownEventType {
   WChar          chr;              // ascii code
   UInt16         keyCode;          // virtual key code
   UInt16         modifiers;
   };

struct _KeyUpEventType {
   WChar          chr;              // ascii code
   UInt16         keyCode;          // virtual key code
   UInt16         modifiers;
   };

struct _KeyHoldEventType {
   WChar          chr;              // ascii code
   UInt16         keyCode;          // virtual key code
   UInt16         modifiers;
   };

struct _WinEnterEventType {
   WinHandle      enterWindow;
   WinHandle      exitWindow;
   };

struct _WinExitEventType {
   WinHandle      enterWindow;
   WinHandle      exitWindow;
   };

struct _TSMConfirmType {
	Char *			yomiText;
	UInt16			formID;
   };

struct _TSMFepButtonType {
	UInt16			buttonID;
   };

struct _TSMFepModeEventType {
	UInt16         mode;					
   };


// The event record.
typedef struct SysEventType {
   SysEventsEnum  eType;
   Boolean        penDown;
   UInt8          tapCount;
   Coord          screenX;
   Coord          screenY;
   union {
      struct _GenericEventType		generic;

      struct _PenUpEventType		penUp;
      struct _KeyDownEventType		keyDown;
      struct _KeyUpEventType		keyUp;
      struct _KeyHoldEventType		keyHold;
      struct _WinEnterEventType		winEnter;
      struct _WinExitEventType		winExit;
      struct _TSMConfirmType		tsmConfirm;
      struct _TSMFepButtonType		tsmFepButton;
      struct _TSMFepModeEventType	tsmFepMode;

      } data;

} SysEventType;


// Events are stored in the event queue with some extra fields:
typedef  struct {
   SysEventType   event;
   UInt32         id;                  // used to support EvtAddUniqueEvent
   } SysEventStoreType;

#define     PenGetPoint(a,b,c)    EvtGetPen(a,b,c)



#endif // __SYSEVENT_H__
