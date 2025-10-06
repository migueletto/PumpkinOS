/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/** @ingroup STE
 *
 */

/**
 *
 * @file 	SmartTextEngineTraps.h
 *
 * @brief	Trap definitions for the Smart Text Engine shared library.
 *
 */

//			Trap IDs for the Smart Text Engine Library's public functions.
//			The order of the traps must be the same as the routines are
//			listed in SmartTextEngineDispatchTable.c.


#ifndef _SMARTTEXTENGINETRAPS_H_
#define _SMARTTEXTENGINETRAPS_H_


#define STETrapOpen 						sysLibTrapOpen	/**<Open Trap*/
#define STETrapClose						sysLibTrapClose	/**<Close Trap*/

#define STETrapGetAPIVersion				(sysLibTrapCustom + 0)	/**<Trap ID for Get API function*/
#define STETrapInitializeEngine 			(sysLibTrapCustom + 1)	/**<Trap ID for Initialize Engine function*/
#define STETrapResetEngine					(sysLibTrapCustom + 2)	/**<Trap ID for Reset Engine function*/
#define STETrapReinitializeEngine			(sysLibTrapCustom + 3)	/**<Trap ID for Reinitialize Engine function*/
#define STETrapAppendTextToEngine			(sysLibTrapCustom + 4)	/**<Trap ID for Append Text to Engine*/
#define STETrapRemoveLastRecordFromEngine	(sysLibTrapCustom + 5)	/**<Trap ID for Remove last record from engine*/
#define STETrapRenderList					(sysLibTrapCustom + 6)	/**<Trap ID for render list*/
#define STETrapRerenderList 				(sysLibTrapCustom + 7)	/**<Trap ID for Rerender list*/
#define STETrapHandleEvent					(sysLibTrapCustom + 8)	/**<Trap ID for Handle Event*/
#define STETrapHandleSclRepeatEvent 		(sysLibTrapCustom + 9)	/**<Trap ID for Handle Scl repeat event*/
#define STETrapHandlePenDownEvent			(sysLibTrapCustom + 10)	/**<Trap ID for Handle pen down event*/
#define STETrapHandlePenMoveEvent			(sysLibTrapCustom + 11)	/**<Trap ID for Handle pen move event*/
#define STETrapHandlePenUpEvent 			(sysLibTrapCustom + 12)	/**<Trap ID for Handle pen up event*/
#define STETrapHandleKeyDownEvent			(sysLibTrapCustom + 13)	/**<Trap ID for Handle key down event*/
#define STETrapGetScroll					(sysLibTrapCustom + 14)	/**<Trap ID for Get scroll*/
#define STETrapSetScroll					(sysLibTrapCustom + 15)	/**<Trap ID for Set scroll*/
#define STETrapGetScrollPct 				(sysLibTrapCustom + 16)	/**<Trap ID for Get scroll pct*/
#define STETrapSetScrollPct 				(sysLibTrapCustom + 17)	/**<Trap ID for Set scroll pct*/
#define STETrapSetScrollToRecordNum 		(sysLibTrapCustom + 18)	/**<Trap ID for Set scroll to record num*/
#define STETrapSetFlags 					(sysLibTrapCustom + 19)	/**<Trap ID for Set Flags*/
#define STETrapGetNumLines					(sysLibTrapCustom + 20)	/**<Trap ID for Get num lines*/
#define STETrapGetNumRecords				(sysLibTrapCustom + 21)	/**<Trap ID for Get num records*/
#define STETrapHasHotRectSelection			(sysLibTrapCustom + 22)	/**<Trap ID for Has hot rect selection*/
#define STETrapHasTextSelection 			(sysLibTrapCustom + 23)	/**<Trap ID for Has text selection*/
#define STETrapClearCurrentSelection		(sysLibTrapCustom + 24)	/**<Trap ID for Clear current selection*/
#define STETrapSetCurrentTextSelection		(sysLibTrapCustom + 25)	/**<Trap ID for Set current text selection*/
#define STETrapGetCurrentTextSelection		(sysLibTrapCustom + 26)	/**<Trap ID for Get current selection*/
#define STETrapGetSelectedText				(sysLibTrapCustom + 27)	/**<Trap ID for Get selected text*/
#define STETrapCopySelectionToClipboard 	(sysLibTrapCustom + 28)	/**<Trap ID for Copy selection clipboard*/
#define STETrapGetLastTapInfo				(sysLibTrapCustom + 29)	/**<Trap ID for Last Tap info*/
#define STETrapSetCustomHyperlinkCallback	(sysLibTrapCustom + 30)	/**<Trap ID for Set custom hyperlink callback*/
#define STETrapGetFlags						(sysLibTrapCustom + 31)	/**<Trap ID for Get flags*/
#define STETrapHasMessageSelection			(sysLibTrapCustom + 32)	/**<Trap ID for Has message selection*/
#define STETrapGetParsedInformation 		(sysLibTrapCustom + 33)	/**<Trap ID for Get parsed information*/
#define STETrapAppendParsedInfo 			(sysLibTrapCustom + 34)	/**<Trap ID for Append parsed info*/
#define STEDepracated5						(sysLibTrapCustom + 35)	/**<Trap ID for Depracated5*/
#define STETrapQuickRender					(sysLibTrapCustom + 36)	/**<Trap ID for Quick Render*/

#endif /* _SMARTTEXTENGINETRAPS_H_ */

