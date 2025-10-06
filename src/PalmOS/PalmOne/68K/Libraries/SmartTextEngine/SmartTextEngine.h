/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/**
 * @defgroup STE Smart Text Engine Library
 *
 * @{
 * @}
 */

/**
 *
 * @file  SmartTextEngine.h
 * @brief Public 68K include file for Smart Text Engine shared library.
 *
 * The Smart Text Engine (STE) Library can be used to enable hyperlink
 * in your application and display icon as defined in the Resource Header file.
 *
 * It will enable user to tap on:
 *	- Phone number to dial using the phone application
 *	- URL to browse
 *	- Email to send an email by launching the default email application
 *
 * The calling application should always load this library with
 * SysLibLoad() before use, even if it is already open by another
 * application(ie, SysLibFind() returns a valid refnum). When
 * the application is done with the library, it should be
 * unloaded with SysLibRemove(). We do this because there is
 * no good way to synchronize loading and unloading of libraries
 * among multiple applications. It also greatly simplifies internal
 * synchronization.
 *
 * @code
 *	Err LoadSmartTextEngine(UInt16* refNumP, Boolean* libLoadedP)
 *		{
 * 		Err error = 0;
 *
 *		// Routine is pointless without this parameter
 * 		if (!refNumP)
 *			return memErrInvalidParam;
 *
 * 		// always load the library, so we have our own instance of the engine
 *  		error = SysLibLoad(sysFileTLibrary, hsFileCSmartTextEngine, refNumP);
 *		if (error)
 *			return error;
 *		*libLoadedP = true;
 *
 * 		return error;
 *		}
 * @endcode
 */

#ifndef __SMARTTEXTENGINE68K_H__
#define __SMARTTEXTENGINE68K_H__

#include <PalmTypes.h>
#include <LibTraps.h>

#include <Common/Libraries/SmartTextEngine/SmartTextEngineTraps.h>
#include <Common/Libraries/SmartTextEngine/SmartTextEngineDef.h>
#include <Common/Libraries/SmartTextEngine/SmartTextEngineErrors.h>


#ifdef __cplusplus
extern "C" {
#endif


/// Standard library open routine.
/// Library should always be loaded with SysLibLoad()(not SysLibFind())
/// before calling STEOpen(). See note at top of file.
///
/// @param refNum:		  IN:  Library reference number
/// @retval STEErr Error Code or zero if no error occurs
extern STEErr STEOpen(UInt16 refNum)
	SYS_TRAP(sysLibTrapOpen);

/// Standard library close routine.
/// Library should always be unloaded with SysLibRemove() after
/// STEClose() is called. See note at top of file.
///
/// @param refNum:		  IN:  Library reference number
/// @retval STEErr Error Code or zero if no error occurs
extern STEErr STEClose(UInt16 refNum)
	SYS_TRAP(sysLibTrapClose);


/// Get Library API Version number.
///
/// @param refNum:		  IN:  Library reference number
/// @param majVerP:		  OUT: Major version number
/// @param minVerP:		  OUT: Minor version number
/// @retval STEErr Error Code or zero if no error occurs
extern STEErr STEGetAPIVersion(UInt16 refNum, UInt32* majVerP, UInt32* minVerP)
	SYS_TRAP(STETrapGetAPIVersion);

/// Styled Text Engine initialization function
/// Before most of the STE can be used, it must be initialized.  This function will return
///	an engineRefNum which is to be used in the STE functions that require the STE
///	to be initialized.
///
/// There are some functions that do not require the STE to be initialized.  They are noted below.
///
/// @param	  refNum:			IN:   The STE refNum
/// @param	  engineRefNum: 	OUT:  The STE engine refNum to be used in subsequent calls to the STE
///	@param    listBounds:		IN:   The bounds of the STE list that will be displayed
/// @param	  scrollBarID:		IN:   The ID of the scroll used by the STE list.  If this is 0, the list will not
///							          scroll.  If there is more text than can be displayed, an ellipsis will be
///							          appended to the end.
/// @param	  flags:			IN:   The flags for the STE
/// @param	  phoneNumberColor: IN:   The color of the phone number hyperlinks
/// @param	  urlColor: 		IN:   The color of the URL hyperlinks
/// @param	  emailColor:		IN:   The color of the email hyperlinks
/// @retval STEErr Error Code or zero if no error occurs
extern STEErr STEInitializeEngine(UInt16 refNum, UInt16* engineRefNum, RectangleType* listBounds, UInt16 scrollBarID, UInt32 flags,
								  UInt8 phoneNumberColor, UInt8 urlColor, UInt8 emailColor)
	SYS_TRAP(STETrapInitializeEngine);

///Frees up the STE engine refNum.  For each STEInitializeEngine, there must be a corresponding
///STEResetEngine.
///
///@param	  refNum:			IN:   The STE refNum
///@param	  engineRefNum: 	IN:   The STE engine refNum
///@retval STEErr Error Code or zero if no error occurs
extern STEErr STEResetEngine(UInt16 refNum, UInt16 engineRefNum)
	SYS_TRAP(STETrapResetEngine);

///Initializes the STE to the initial state right after initializing it.  This frees up any text that
///has been entered into the STE.
///
///@param  refNum:			IN:   The STE refNum
///@param  engineRefNum: 	IN:   The STE engine refNum
///@retval STEErr Error Code or zero if no error occurs
extern STEErr STEReinitializeEngine(UInt16 refNum, UInt16 engineRefNum)
	SYS_TRAP(STETrapReinitializeEngine);

///Function that is used to add text to the STE.  Each text string that is appended
///is called a record.  Record numbers are 1 based, meaning the first record is record 1.
///
///@param	  refNum:			IN:   The STE refNum
///@param	  engineRefNum: 	IN:   The STE engine refNum
///@param	  textP:			IN:   The text to append to the STE, including any STE delimiters
///@param	  renderImmediate:	IN:   If true, the text will be displayed immediately.	If false, it
///							  will not be displayed until STERenderList is called.
///@retval	  -
extern void STEAppendTextToEngine(UInt16 refNum, UInt16 engineRefNum, Char* textP, Boolean renderImmediate)
	SYS_TRAP(STETrapAppendTextToEngine);

///Removes the last text that was appended to the engine.
///You must call STERenderList to update the display after removing the text.
///
///@param	  refNum:			IN:   The STE refNum
///@param	  engineRefNum: 	IN:   The STE engine refNum
///@retval	  -
extern void STERemoveLastRecordFromEngine(UInt16 refNum, UInt16 engineRefNum)
	SYS_TRAP(STETrapRemoveLastRecordFromEngine);

///Renders the text that is currently in the STE.
///
///@param	  refNum:			IN:   The STE refNum
///@param	  engineRefNum: 	IN:   The STE engine refNum
///@retval	  -
extern void STERenderList(UInt16 refNum, UInt16 engineRefNum)
	SYS_TRAP(STETrapRenderList);

///Rerenders the text that is currently in the STE.  This routine should be used if a change is made
///that can change the formatting (i.e. font size).
///
///@param	  refNum:			IN:   The STE refNum
///@param	  engineRefNum: 	IN:   The STE engine refNum
///@retval	  -
extern void STERerenderList(UInt16 refNum, UInt16 engineRefNum)
	SYS_TRAP(STETrapRerenderList);


///This routine should be called in the form's event handler.	If there is no special handling of
///events, then this routine can be used to handle all the different events that can affect the STE.
///
///@param	  refNum:			IN:   The STE refNum
///@param	  engineRefNum: 	IN:   The STE engine refNum
///@param	  eventP:			IN:   A pointer to the event record.
///@retval	  Boolean True if handled, False if not handled
extern Boolean STEHandleEvent(UInt16 refNum, UInt16 engineRefNum, EventPtr eventP)
	SYS_TRAP(STETrapHandleEvent);

///This routine handles only the sclRepeat event.	This routine should be used if there is special
///handling of the events by the application before (or after) the STE handles the event.
///
///@param	  refNum:			IN:   The STE refNum
///@param	  engineRefNum: 	IN:   The STE engine refNum
///@param	  eventP:			IN:   A pointer to the event record.
///@retval	  Boolean True if handled, False if not handled
extern Boolean STEHandleSclRepeatEvent(UInt16 refNum, UInt16 engineRefNum, EventPtr eventP)
	SYS_TRAP(STETrapHandleSclRepeatEvent);

///This routine handles only the penDown event.  This routine should be used if there is special
///handling of the events by the application before (or after) the STE handles the event.
///
///@param	  refNum:			IN:   The STE refNum
///@param	  engineRefNum: 	IN:   The STE engine refNum
///@param	  eventP:			IN:   A pointer to the event record.
///@retval	  Boolean True if handled, False if not handled
extern Boolean STEHandlePenDownEvent(UInt16 refNum, UInt16 engineRefNum, EventPtr eventP)
	SYS_TRAP(STETrapHandlePenDownEvent);

///This routine handles only the penMove event.  This routine should be used if there is special
///handling of the events by the application before (or after) the STE handles the event.
///
///@param	  refNum:			IN:   The STE refNum
///@param	  engineRefNum: 	IN:   The STE engine refNum
///@param	  eventP:			IN:   A pointer to the event record.
///@retval	  Boolean True if handled, False if not handled
extern Boolean STEHandlePenMoveEvent(UInt16 refNum, UInt16 engineRefNum, EventPtr eventP)
	SYS_TRAP(STETrapHandlePenMoveEvent);

///This routine handles only the penUp event.	This routine should be used if there is special
///handling of the events by the application before (or after) the STE handles the event.
///
///@param	  refNum:			IN:   The STE refNum
///@param	  engineRefNum: 	IN:   The STE engine refNum
///@param	  eventP:			IN:   A pointer to the event record.
///@retval	  Boolean True if handled, False if not handled
extern Boolean STEHandlePenUpEvent(UInt16 refNum, UInt16 engineRefNum, EventPtr eventP)
	SYS_TRAP(STETrapHandlePenUpEvent);

///This routine handles only the keyDown event.  This routine should be used if there is special
///handling of the events by the application before (or after) the STE handles the event.
///
///@param	  refNum:			IN:   The STE refNum
///@param	  engineRefNum: 	IN:   The STE engine refNum
///@param	  eventP:			IN:   A pointer to the event record.
///@retval	  Boolean True if handled, False if not handled
extern Boolean STEHandleKeyDownEvent(UInt16 refNum, UInt16 engineRefNum, EventPtr eventP)
	SYS_TRAP(STETrapHandleKeyDownEvent);

///This routine returns the current scroll position of the STE's scroll bar.
///
///@param	  refNum:			IN:   The STE refNum
///@param	  engineRefNum: 	IN:   The STE engine refNum
///@retval 	  UInt16 The position of the scroll bar.
extern UInt16 STEGetScroll(UInt16 refNum, UInt16 engineRefNum)
	SYS_TRAP(STETrapGetScroll);

///This routine sets the current scroll position of the STE's scroll bar.
///
///@param	  refNum:			IN:   The STE refNum
///@param	  engineRefNum: 	IN:   The STE engine refNum
///@param	  scrollValue:		IN:   The position to set the scroll bar
///@retval	  -
extern void STESetScroll(UInt16 refNum, UInt16 engineRefNum, UInt16 scrollValue)
	SYS_TRAP(STETrapSetScroll);

///This routine returns the current scroll position of the STE's scroll bar as a percentage
///of the entire scroll bar.
///
///@param	  refNum:			IN:   The STE refNum
///@param	  engineRefNum: 	IN:   The STE engine refNum
///@retval 	  UInt16 The position of the scroll bar in percentage.
extern UInt16 STEGetScrollPct(UInt16 refNum, UInt16 engineRefNum)
	SYS_TRAP(STETrapGetScrollPct);

///This routine sets the current scroll position percentage of the STE's scroll bar.
///
///@param	  refNum:			IN:   The STE refNum
///@param	  engineRefNum: 	IN:   The STE engine refNum
///@param	  scrollPct:		IN:   The position to set the scroll bar in percentage
///@retval	  -
extern void STESetScrollPct(UInt16 refNum, UInt16 engineRefNum, UInt16 scrollPct)
	SYS_TRAP(STETrapSetScrollPct);

///This routine sets the current scroll bar position so that the specified record
///appears at the top of the list.
///
///@param	  refNum:			IN:   The STE refNum
///@param	  engineRefNum: 	IN:   The STE engine refNum
///@param	  recordNum:		IN:   The record number to display at the top of the list.
///@retval	  -
extern void STESetScrollToRecordNum(UInt16 refNum, UInt16 engineRefNum, UInt16 recordNum)
	SYS_TRAP(STETrapSetScrollToRecordNum);

///This routine sets the STE flags
///
///@param	  refNum:			IN:   The STE refNum
///@param	  engineRefNum: 	IN:   The STE engine refNum
///@param	  flagsToSet:		IN:   Bitmap representing the flags to set
///@param	  flagsToClear: 	IN:   Bitmap representing the flags to clear
///@retval	  -
extern void STESetFlags(UInt16 refNum, UInt16 engineRefNum, UInt32 flagsToSet, UInt32 flagsToClear)
	SYS_TRAP(STETrapSetFlags);

///This routine returns the current STE flags
///
///@param	  refNum:			IN:   The STE refNum
///@param	  engineRefNum: 	IN:   The STE engine refNum
///@retval 	  UInt32 The STE flags
extern UInt32 STEGetFlags(UInt16 refNum, UInt16 engineRefNum)
	SYS_TRAP(STETrapGetFlags);

///This routine returns the number of lines in the STE.
///
///@param	  refNum:			IN:   The STE refNum
///@param	  engineRefNum: 	IN:   The STE engine refNum
///@retval 	  UInt16 Number of lines in the STE
extern UInt16 STEGetNumLines(UInt16 refNum, UInt16 engineRefNum)
	SYS_TRAP(STETrapGetNumLines);

///This routine returns the number of records in the STE.
///
///@param	  refNum:			IN:   The STE refNum
///@param	  engineRefNum: 	IN:   The STE engine refNum
///@retval 	  UInt16 Number of records in the STE
extern UInt16 STEGetNumRecords(UInt16 refNum, UInt16 engineRefNum)
	SYS_TRAP(STETrapGetNumRecords);

///This routine checks if there is a current hot rect selection.  A hot rect
///is a hyperlink that has been selected with the keyboard.
///
///@param	  refNum:			IN:   The STE refNum
///@param	  engineRefNum: 	IN:   The STE engine refNum
///@retval 	  Boolean True if there is a current hot rect selected
extern Boolean STEHasHotRectSelection(UInt16 refNum, UInt16 engineRefNum)
	SYS_TRAP(STETrapHasHotRectSelection);

///This routine checks if there is a current message selected.
///
///@param	  refNum:			IN:   The STE refNum
///@param	  engineRefNum:		IN:   The STE engine refNum
///@retval 	  Boolean True if there is a current message selected
extern Boolean STEHasMessageSelection(UInt16 refNum, UInt16 engineRefNum)
	SYS_TRAP(STETrapHasMessageSelection);

///This routine checks if there is any text currently selected.
///
///@param	  refNum:			IN:   The STE refNum
///@param	  engineRefNum: 	IN:   The STE engine refNum
///@retval 	  Boolean True if there is any text selected
extern Boolean STEHasTextSelection(UInt16 refNum, UInt16 engineRefNum)
	SYS_TRAP(STETrapHasTextSelection);

///This routine clears any currently selected text or hot rect.
///
///@param	  refNum:			IN:   The STE refNum
///@param	  engineRefNum: 	IN:   The STE engine refNum
///@retval	  -
extern void STEClearCurrentSelection(UInt16 refNum, UInt16 engineRefNum)
	SYS_TRAP(STETrapClearCurrentSelection);

///This routine will select the specified text selection.	The display will be updated
///to show the selected text as highlighted.
///
///@param	  refNum:			IN:   The STE refNum
///@param	  engineRefNum: 	IN:   The STE engine refNum
///@param	  messageNumber:	IN:   The record number to start the selection
///@param	  startLocation:	IN:   The byte offset to start the text selection
///@param	  selectionLength:	IN:   The number of bytes in the text selection
///@retval	  UInt16 Returns the line number of the topmost line of the selection
extern UInt16 STESetCurrentTextSelection(UInt16 refNum, UInt16 engineRefNum, UInt16 messageNumber, UInt32 startLocation, UInt32 selectionLength)
	SYS_TRAP(STETrapSetCurrentTextSelection);

///This routine will select the specified text selection.	The display will be updated
///to show the selected text as highlighted.
///
///@param	  refNum:			IN:   The STE refNum
///@param	  engineRefNum: 	IN:   The STE engine refNum
///@param	  messageNumber:	OUT:  The record number of the start the selection
///@param	  startLocation:	OUT:  The byte offset of the start the text selection
///@param	  selectionLength:	OUT:  The number of bytes in the text selection
///@retval	  -
extern void STEGetCurrentTextSelection(UInt16 refNum, UInt16 engineRefNum, UInt16* messageNumber, UInt16* startLocation, UInt16* selectionLength)
	SYS_TRAP(STETrapGetCurrentTextSelection);

///This routine returns a copy of the actual text that is currently selected.
///
///@param	  refNum:			IN:   The STE refNum
///@param	  engineRefNum: 	IN:   The STE engine refNum
///@retval 	  Char* Returns a copy of the current text selection.  This must be freed by the caller.
extern Char* STEGetSelectedText(UInt16 refNum, UInt16 engineRefNum)
	SYS_TRAP(STETrapGetSelectedText);

///This routine copies the selected text to the system's clipboard.
///
///@param	  refNum:			IN:   The STE refNum
///@param	  engineRefNum: 	IN:   The STE engine refNum
///@retval	  -
extern void STECopySelectionToClipboard(UInt16 refNum, UInt16 engineRefNum)
	SYS_TRAP(STETrapCopySelectionToClipboard);

///This routine returns the location of the last tap.	If the user drags the stylus, it is not a tap.
///
///@param	  refNum:			IN:   The STE refNum
///@param	  engineRefNum: 	IN:   The STE engine refNum
///@param	  messageNumber:	OUT:  The record number of the text that was tapped
///@param	  tapLocation:		OUT:  The number of bytes offset from the start of the text to the location tapped
///@retval	  -
extern void STEGetLastTapInfo(UInt16 refNum, UInt16 engineRefNum, UInt16* messageNumber, UInt16* tapLocation)
	SYS_TRAP(STETrapGetLastTapInfo);

///This routine sets the procedure to call as a result of tapping a custom hyperlink
///
///@param	  refNum:			IN:   The STE refNum
///@param	  engineRefNum: 	IN:   The STE engine refNum
///@param	  callback: 		IN:   The routine to call
///@param	  armCallback:		IN:   True if the callback routine is ARM code, false if it is 68K code
///@retval	  -
extern void STESetCustomHyperlinkCallback(UInt16 refNum, UInt16 engineRefNum, STECallbackType callback, Boolean armCallback)
	SYS_TRAP(STETrapSetCustomHyperlinkCallback);

///This routine will render the specified text as a "one shot deal".  The text will
///not respond to any events, and hyperlinks do not work.	This can be used to display
///rich text with graphics easily in your application.
///
///This does NOT require the STE to be initialized.
///
///@param	  refNum:			IN:   The STE refNum
///@param	  listBounds:		IN:   The bounds of the STE list that will be displayed
///@param	  flags:			IN:   The flags for the STE
///@param	  phoneNumberColor: IN:   The color of the phone number hyperlinks
///@param	  urlColor: 		IN:   The color of the URL hyperlinks
///@param	  emailColor:		IN:   The color of the email hyperlinks
///@param	  msgP: 			IN:   The text to display
///@retval	  -
extern void STEQuickRender(UInt16 refNum, RectangleType* listBounds, UInt32 flags,
						   UInt8 phoneNumberColor, UInt8 urlColor, UInt8 emailColor, Char* msgP)
	SYS_TRAP(STETrapQuickRender);

///This routine will parse the text string for URLs, email addresses, and/or phone numbers.
///
///This does NOT require the STE to be initialized.
///
///@param	  refNum:			IN:   The STE refNum
///@param	  textP:			IN:   The text string to parse
///@param	  msgNumber:		IN:   The message number of this text string  (Used internally by the STE)
///@param	  replaceLineFeeds: IN:   If true, any line feed will be replaced with a space, but a parsed item will be create
///							          that specifies a line feed to be at that location
///@param	  flags:			IN:   The flags indicating what type of item to parse out
///@retval    ParsedInfoList* A parsed info list that has each of the parsed items found in the text string.
extern ParsedInfoList* STEGetParsedInformation(UInt16 refNum, Char* textP, UInt16 msgNumber, Boolean replaceLineFeeds, UInt32 flags)
	SYS_TRAP(STETrapGetParsedInformation);


///This routine append a ParsedInfoList to another ParsedInfoList.
///
///This does NOT require the STE to be initialized.
///This is used internally by the STE.  You probably will not use this function.
///
///@param	  refNum:			 IN:   The STE refNum
///@param	  destParsedInfoH:	 IN:   A handle to the destination parsed info list
///@param	  sourceParsedInfoP: IN:   The source parsed info list.
///@retval	  -
extern void STEAppendParsedInfo(UInt16 refNum, MemHandle destParsedInfoH, ParsedInfoList* sourceParsedInfoP)
	SYS_TRAP(STETrapAppendParsedInfo);



#ifdef __cplusplus
}
#endif




#endif	//__SMARTTEXTENGINE68K_H__
