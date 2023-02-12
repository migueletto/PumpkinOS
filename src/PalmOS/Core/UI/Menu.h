/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: Menu.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *	  This file defines menu structures and routines.
 *
 *****************************************************************************/

#ifndef __MENU_H__
#define __MENU_H__

#include <PalmTypes.h>
#include <CoreTraps.h>

#include <Font.h>
#include <Window.h>
#include <Event.h>

// Errors returned by Menu routines

#define menuErrNoMenu				(menuErrorClass | 1)
#define menuErrNotFound				(menuErrorClass | 2)
#define menuErrSameId				(menuErrorClass | 3)
#define menuErrTooManyItems		(menuErrorClass | 4)
#define menuErrOutOfMemory			(menuErrorClass | 5)


// Command bar structures

typedef enum {
	menuCmdBarResultNone,			// send nothing (this'd be quite unusual but is allowed)
	menuCmdBarResultChar,			// char to send (with commandKeyMask bit set)
	menuCmdBarResultMenuItem,		// id of the menu item
	menuCmdBarResultNotify			// Nofication Manager notification type
} MenuCmdBarResultType;
	
// maximum length of the prompt string to display in the command bar
#define menuCmdBarMaxTextLength 20

typedef struct MenuCmdBarButtonTag
#ifdef ALLOW_ACCESS_TO_INTERNALS_OF_MENUS	// These fields will not be available in the next OS release!
{
	UInt16						bitmapId;
	Char							name[menuCmdBarMaxTextLength];
	MenuCmdBarResultType		resultType;
	UInt8							reserved; // alignment padding
	UInt32						result;
}
#endif
MenuCmdBarButtonType;

typedef struct MenuCmdBarType
#ifdef ALLOW_ACCESS_TO_INTERNALS_OF_MENUS	// These fields will not be available in the next OS release!
{
	WinHandle				bitsBehind;
	Int32						timeoutTick; // tick to disappear on
	Coord						top;
	Int16						numButtons;
	Boolean					insPtWasEnabled;
	Boolean					gsiWasEnabled;
	Boolean					feedbackMode;  // set when just displaying confirmation feedback
	MenuCmdBarButtonType	*	buttonsData;
}
#endif
MenuCmdBarType;

// to tell MenuCmdBarAddButton where to add the button: on right or left.
#define menuCmdBarOnRight 0
#define menuCmdBarOnLeft  0xff



////Menu-specific

#define noMenuSelection			-1
#define noMenuItemSelection	-1
#define separatorItemSelection	-2

// cause codes for menuOpen Event
#define menuButtonCause				0
#define menuCommandCause			1

// To match Apple's ResEdit the first byte of a menu item's text can
// be a special char indicating a special menu item.
#define MenuSeparatorChar			'-'

typedef struct MenuItemTag
#ifdef ALLOW_ACCESS_TO_INTERNALS_OF_MENUS	// These fields will not be available in the next OS release!
{
	UInt16					id;				// id of the menu item
	Char						command;			// command key
	UInt8						hidden: 1;		// true if menu item is hidden
	UInt8 					reserved: 7;   
	Char *					itemStr;			// string to be displayed

	RectangleType localBounds, bounds;
}
#endif
MenuItemType;


typedef struct MenuPullDownTag
#ifdef ALLOW_ACCESS_TO_INTERNALS_OF_MENUS	// These fields will not be available in the next OS release!
{
	WinHandle				menuWin;			// window of pull-down menu
	RectangleType			bounds;			// bounds of the pulldown
	WinHandle				bitsBehind;		// saving bits behind pull-down menu
	RectangleType			titleBounds;	// bounds of the title in menu bar
	Char *					title;			// menu title displayed in menu bar
	UInt16					hidden: 1;		// true if pulldown is hidden
	UInt16					numItems: 15;	// number of items in the menu
	MenuItemType *			items;			// array of menu items

  Int16 selectedItem;
}
#endif
MenuPullDownType;

typedef MenuPullDownType *MenuPullDownPtr;

typedef struct MenuBarAttrTag
#ifdef ALLOW_ACCESS_TO_INTERNALS_OF_MENUS	// These fields will not be available in the next OS release!
{
	UInt16 visible				:1;			// set if menu bar is drawn
	UInt16 commandPending	:1;			// set if next key is a command
	UInt16 insPtEnabled		:1;			// set if insPt was on when menu was drawn
	UInt16 needsRecalc 		:1;			// if set then recalc menu dimensions
	UInt16 attnIndicatorIsAllowed :1;	// set if attn indicator was allowed when menu was drawn
	UInt16 reserved 			:11;			// reserved for future use
}
#endif
MenuBarAttrType;

typedef struct MenuBarTag
#ifdef ALLOW_ACCESS_TO_INTERNALS_OF_MENUS	// These fields will not be available in the next OS release!
{
	WinHandle				barWin;			// window of menu bar
	WinHandle				bitsBehind;		// saving bits behind menu bar
	WinHandle				savedActiveWin;
	WinHandle				bitsBehindStatus;
	MenuBarAttrType 		attr;
	Int16						curMenu;			// current menu or -1 if none
	Int16						curItem;			// current item in curMenu, -1 if none
	Int32						commandTick;	//
	Int16						numMenus;		// number of menus
	MenuPullDownPtr  		menus;			// array of menus

  Int16 selectedMenu;
}
#endif
MenuBarType;

typedef MenuBarType *MenuBarPtr;

	

#ifdef __cplusplus
extern "C" {
#endif

extern MenuBarType *MenuInit (UInt16 resourceId)
							SYS_TRAP(sysTrapMenuInit);

extern MenuBarType *MenuGetActiveMenu (void)
							SYS_TRAP(sysTrapMenuGetActiveMenu);

extern MenuBarType *MenuSetActiveMenu (MenuBarType *menuP)
							SYS_TRAP(sysTrapMenuSetActiveMenu);

extern void MenuDispose (MenuBarType *menuP)
							SYS_TRAP(sysTrapMenuDispose);

extern Boolean MenuHandleEvent (MenuBarType *menuP, EventType *event, UInt16 *error)
							SYS_TRAP(sysTrapMenuHandleEvent);

extern void MenuDrawMenu (MenuBarType *menuP)
							SYS_TRAP(sysTrapMenuDrawMenu);

extern void MenuEraseStatus (MenuBarType *menuP)
							SYS_TRAP(sysTrapMenuEraseStatus);

extern void MenuSetActiveMenuRscID (UInt16 resourceId)
							SYS_TRAP(sysTrapMenuSetActiveMenuRscID);
							
extern Err MenuCmdBarAddButton(UInt8 where, UInt16 bitmapId,
						MenuCmdBarResultType resultType, UInt32 result, Char *nameP)
							SYS_TRAP(sysTrapMenuCmdBarAddButton);

extern Boolean MenuCmdBarGetButtonData(Int16 buttonIndex, UInt16 *bitmapIdP, 
						MenuCmdBarResultType *resultTypeP, UInt32 *resultP, Char *nameP)
							SYS_TRAP(sysTrapMenuCmdBarGetButtonData);
			
extern void MenuCmdBarDisplay (void)
							SYS_TRAP(sysTrapMenuCmdBarDisplay);
							
extern Boolean MenuShowItem(UInt16 id)
 							SYS_TRAP(sysTrapMenuShowItem);

extern Boolean MenuHideItem(UInt16 id) 
								SYS_TRAP(sysTrapMenuHideItem);
						
extern Err MenuAddItem(UInt16 positionId, UInt16 id, Char cmd, const Char *textP)
							SYS_TRAP(sysTrapMenuAddItem);

							
			
#ifdef __cplusplus 
}
#endif

#endif //__MENU_H__
