/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: List.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *	  This file defines list structures and routines.
 *
 *****************************************************************************/

#ifndef __LIST_H__
#define __LIST_H__

#include <PalmTypes.h>
#include <CoreTraps.h>

#include <Event.h>

#define noListSelection    		-1

//-------------------------------------------------------------------
// List structures
//-------------------------------------------------------------------

typedef struct ListAttrTag
#ifdef ALLOW_ACCESS_TO_INTERNALS_OF_LISTS	// These fields will not be available in the next OS release!
{
	UInt16 usable				:1;		// set if part of ui 
	UInt16 enabled				:1;		// set if interactable (not grayed out)
	UInt16 visible				:1;		// set if drawn
   UInt16 poppedUp			:1;     	// set if choices displayed in popup win.
	UInt16 hasScrollBar		:1;		// set if the list has a scroll bar
	UInt16 search				:1;		// set if incremental search is enabled
   UInt16 reserved			:10;		// reserved for future use
}
#endif
ListAttrType;


// Load data callback routine prototype
typedef void ListDrawDataFuncType (Int16 itemNum, RectangleType *bounds, 
	Char **itemsText);
	
typedef ListDrawDataFuncType *ListDrawDataFuncPtr;


typedef struct ListType
#ifdef ALLOW_ACCESS_TO_INTERNALS_OF_LISTS	// These fields will not be available in the next OS release!
{
	UInt16					id;
   RectangleType			bounds;
   ListAttrType			attr;
   Char *					*itemsText;
   Int16						numItems;        		// number of choices in the list
   Int16 	  				currentItem;     		// currently display choice
   Int16   					topItem;         		// top item visible when poped up
   FontID           		font;						// font used to draw list
	UInt8 					reserved;
	WinHandle   			popupWin;				// used only by popup lists
   ListDrawDataFuncPtr	drawItemsCallback;	// 0 indicates no function

	WinHandle bitsBehind;
  Int16 visibleItems;
  void *formP;
  UInt16 controlID;
  uint32_t m68k_drawfunc;
  char *aux[1]; // pointer array allocated after list type header
}
#endif
ListType;

typedef ListType *ListPtr;


//-------------------------------------------------------------------
// List routines
//-------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif

extern void LstDrawList (ListType *listP)
							SYS_TRAP(sysTrapLstDrawList);

extern void LstEraseList (ListType *listP)
							SYS_TRAP(sysTrapLstEraseList);

extern Int16 LstGetSelection (const ListType *listP)
							SYS_TRAP(sysTrapLstGetSelection);

extern Char * LstGetSelectionText (const ListType *listP, Int16 itemNum)
							SYS_TRAP(sysTrapLstGetSelectionText);

extern Boolean LstHandleEvent (ListType *listP, const EventType *eventP)
							SYS_TRAP(sysTrapLstHandleEvent);

extern void LstSetHeight (ListType *listP, Int16 visibleItems)
							SYS_TRAP(sysTrapLstSetHeight);

extern void LstSetPosition (ListType *listP, Coord x, Coord y)
							SYS_TRAP(sysTrapLstSetPosition);

extern void LstSetSelection (ListType *listP, Int16 itemNum)
							SYS_TRAP(sysTrapLstSetSelection);

extern void LstSetListChoices (ListType *listP, Char **itemsText, Int16 numItems)
							SYS_TRAP(sysTrapLstSetListChoices);

extern void LstSetDrawFunction (ListType *listP, ListDrawDataFuncPtr func)
							SYS_TRAP(sysTrapLstSetDrawFunction);

extern void LstSetTopItem (ListType *listP, Int16 itemNum)
							SYS_TRAP(sysTrapLstSetTopItem);

extern void LstMakeItemVisible (ListType *listP, Int16 itemNum)
							SYS_TRAP(sysTrapLstMakeItemVisible);

extern Int16 LstGetNumberOfItems (const ListType *listP)
							SYS_TRAP(sysTrapLstGetNumberOfItems);

extern Int16 LstPopupList (ListType *listP)
							SYS_TRAP(sysTrapLstPopupList);

extern Boolean LstScrollList(ListType *listP, WinDirectionType direction, Int16 itemCount)
							SYS_TRAP(sysTrapLstScrollList);

extern Int16 LstGetVisibleItems (const ListType *listP)
							SYS_TRAP(sysTrapLstGetVisibleItems);

extern Err LstNewList (void **formPP, UInt16 id, 
	Coord x, Coord y, Coord width, Coord height, 
	FontID font, Int16 visibleItems, Int16 triggerId)
							SYS_TRAP(sysTrapLstNewList);

extern Int16 LstGetTopItem (const ListType *listP)
							SYS_TRAP(sysTrapLstGetTopItem);


#ifdef __cplusplus 
}
#endif

#endif // __LIST_H__
