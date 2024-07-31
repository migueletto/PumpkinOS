/*
 * $Id: prefsdata.h,v 1.95 2004/04/18 15:34:48 prussar Exp $
 *
 * Viewer - a part of Plucker, the free off-line HTML viewer for PalmOS
 * Copyright (c) 1998-2002, Mark Ian Lillywhite and Michael Nordstrom
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#ifndef PLUCKER_PREFERENCES_H
#define PLUCKER_PREFERENCES_H

#include "DIA.h"
#include "viewer.h"
#include "resourceids.h"


/* show/hide column in document list */
#define SHOW                    1
#define HIDE                    0

/* some hardcoded sizes for the tap, button, and gesture actions */
#define NUM_OF_CONTROL_MODES    2
#define NUM_OF_CONTROL_LISTS    4
#define NUM_OF_HW_BUTTONS       4
#define NUM_OF_GESTURES         5
#define NUM_OF_JOGEVENTS        6
#define NUM_OF_ARROW_BUTTONS    5

/* default values for using *_ARROW from within Prefs()->arrowMode[] */
#define UP_ARROW     UP_BUTTON - NUM_OF_HW_BUTTONS
#define DOWN_ARROW   DOWN_BUTTON - NUM_OF_HW_BUTTONS
#define LEFT_ARROW   LEFT_BUTTON - NUM_OF_HW_BUTTONS
#define RIGHT_ARROW  RIGHT_BUTTON - NUM_OF_HW_BUTTONS
#define SELECT_ARROW SELECT_BUTTON - NUM_OF_HW_BUTTONS

/* Minimal line spacing */
#define MINIMAL_LINE_SPACING   (-2)


/* NOTE: Don't change the order of the options */


typedef enum {
    DATEBOOK_BUTTON, /*          */
    ADDRESS_BUTTON,  /* Hardware */
    TODO_BUTTON,     /* Buttons  */
    MEMO_BUTTON,     /*          */

    UP_BUTTON,       /*          */
    DOWN_BUTTON,     /* Arrow    */
    LEFT_BUTTON,     /* Buttons  */
    RIGHT_BUTTON,    /*          */
    SELECT_BUTTON    /*          */
} Buttons;


typedef enum {
    GESTURES_UP,
    GESTURES_RIGHT,
    GESTURES_DOWN,
    GESTURES_LEFT,
    GESTURES_TAP
} Gestures;


typedef enum {
    JOGEVENTS_UP,
    JOGEVENTS_DOWN,
    JOGEVENTS_PUSH,
    JOGEVENTS_PUSHUP,
    JOGEVENTS_PUSHDOWN,
    JOGEVENTS_BACK
} JogEvents;

typedef enum {
    MODE1 = 0,
    MODE2 = 1,
    MODE3 = 2
} ModeType;


typedef enum {
    SELECT_NONE = 0,
    SELECT_FULL_PAGE_UP,
    SELECT_HALF_PAGE_UP,
    SELECT_HALF_PAGE_DOWN,
    SELECT_FULL_PAGE_DOWN,
    SELECT_GO_BACK,
    SELECT_GO_FORWARD,
    SELECT_GO_HOME,
    SELECT_GO_TO_TOP,
    SELECT_GO_TO_BOTTOM,
    SELECT_FIND,
    SELECT_FIND_AGAIN,
    SELECT_ADD_BOOKMARK,
    SELECT_VIEW_BOOKMARKS,
    SELECT_OPEN_LIBRARY,
    SELECT_DETAILS,
    SELECT_PREFS,
    SELECT_BUTTON_ACTION,
    SELECT_TAP_ACTION,
    SELECT_GESTURE_ACTION,
    SELECT_TOGGLE_AUTOSCROLL,
    SELECT_INCREASE_AUTOSCROLL,
    SELECT_DECREASE_AUTOSCROLL,
    SELECT_NEXT_ANCHOR,
    SELECT_PREV_ANCHOR,
    SELECT_GO_TO_LINK,
    SELECT_COPY_TO_MEMO,
    SELECT_DELETE_DOCUMENT,
    SELECT_FONT,
    SELECT_COMMAND_STROKE,
    SELECT_MENU,
    SELECT_TOGGLE_TOOLBAR,
    SELECT_TOGGLE_FULLSCREEN,
    SELECT_ONE_LINE_UP,
    SELECT_ONE_LINE_DOWN,
    SELECT_BRIGHTNESS_ADJUST,
    SELECT_CONTRAST_ADJUST,
    SELECT_TOGGLE_BACKLIGHT,
    SELECT_ROTATE_RIGHT,
    SELECT_ROTATE_LEFT,
    SELECT_SAVE_POSITION,
    SELECT_WORD_LOOKUP
} SelectType;


typedef enum {
    TOOLBAR_TOP     = 0,
    TOOLBAR_BOTTOM,
    TOOLBAR_NONE,
    TOOLBAR_SILK
} ToolbarType;


typedef enum {
    SCROLLBAR_RIGHT = 0,
    SCROLLBAR_LEFT,
    SCROLLBAR_NONE
} ScrollbarType;


typedef enum {
    FILTER_OR = 0,
    FILTER_AND
} FilterType;


typedef enum {
    FONT_DEFAULT = 0,
    FONT_BOLD,
    FONT_LARGE,
    FONT_LARGEBOLD,
    FONT_NARROW,
    FONT_SMALL,
    FONT_TINY,
    FONT_USER
} FontModeType;


typedef enum {
    SORT_INVALID = 0,
    SORT_NAME,
    SORT_DATE,
    SORT_SIZE,
    SORT_TYPE
} SortType;


typedef enum {
    SORTORDER_ASC = 0,
    SORTORDER_DESC
} SortOrderType;


typedef enum {
    AUTOSCROLL_PIXELS = 0,
    AUTOSCROLL_PAGES
} AutoscrollModeType;


typedef enum {
    AUTOSCROLL_DOWN = 0,
    AUTOSCROLL_UP
} AutoscrollDirType;


typedef enum {
    CATEGORY_CLASSIC = 0,
    CATEGORY_ADVANCED
} CategoryStyleType;


typedef struct {
    unsigned int date   : 1;
    unsigned int size   : 1;
    unsigned int type   : 1;
    unsigned int        : 5;
} ColumnFlags;

typedef enum {
    HARDCOPY_DIALOG = 0,
    HARDCOPY_IMMEDIATE
} HardcopyActionType;

typedef enum {
    HARDCOPY_INVIEW = 0,
    HARDCOPY_VISIBLEPARAGRAPHS
} HardcopyRangeType;

typedef enum {
    HARDCOPY_ATBOTTOM = 0,
    HARDCOPY_INLINE,
    HARDCOPY_NOLINK
} HardcopyLinkType;

typedef enum {
    SEARCH_IN_ONE_PAGE = 0,
    SEARCH_IN_ALL_PAGES = 1,
    SEARCH_DOWN_IN_DOC = 2,
    SEARCH_IN_SUB_PAGES = 3
} SearchModeType;

#define SEARCH_CASESENSITIVE     1
#define SEARCH_MULTIPLE          0x80
#define SEARCH_MATCHMODE_MASK    1
#define SEARCH_UNINITIALIZED     0xFF

typedef enum {
    SYNC_AUTOMATIC = 0,
    SYNC_MANUAL,
    SYNC_IGNORE_CARD
} SyncPolicyType;

typedef enum {
    ROTATE_ZERO    = 0,
    ROTATE_MINUS90,
    ROTATE_PLUS90
} RotateType;

#define NUM_ROTATE_TYPES 3


/* The order here must match that in AlignmentType */
typedef enum {
    FORCE_ALIGN_NONE      = 0,
    FORCE_ALIGN_LEFT      = 1,
    FORCE_ALIGN_RIGHT     = 2,
    FORCE_ALIGN_CENTER    = 3,
    FORCE_ALIGN_JUSTIFY   = 4
} ForceAlignType;

typedef enum {
    SELECT_WORD_TAP_NONE = 0,
    SELECT_WORD_TAP_ONCE = 1
} SelectWordTapType;

typedef enum {
    SELECT_WORD_SEARCH_FORM  = 0,
    SELECT_WORD_PPI          = 1,
    SELECT_WORD_TO_CLIPBOARD = 2
} SelectedWordActionType;


/* The order here must match that in ForceAlignType */
typedef enum {
    ALIGNMENT_LEFT      = 0,
    ALIGNMENT_RIGHT     = 1,
    ALIGNMENT_CENTER    = 2,
    ALIGNMENT_JUSTIFY   = 3
} AlignmentType;



/*
   Viewer preferences
 */
typedef struct {
    Char                docName[ dmDBNameLength ];
    UInt16              cardNo;
    UInt16              location;
    UInt16              categories;
    UInt16              lastForm;
    UInt16              screenDepth;
    UInt16              searchEntries;
    UInt16              toolbarButton;
    UInt16              autoscrollInterval;
    UInt16              autoscrollLastScrollTime;
    UInt16              autoscrollJump;
    Boolean             autoscrollEnabled;
    Boolean             autoscrollStayOn;
    Boolean             strikethrough;
    Boolean             linkClick;
    Boolean             multipleSelect;
    Boolean             underlineMode;
    SearchModeType      searchMode;
    UInt8               searchFlags;
    Boolean             hardKeys;
    Boolean             gestures;
    SyncPolicyType      syncPolicy;
    AutoscrollDirType   autoscrollDir;
    AutoscrollModeType  autoscrollMode;
    ColumnFlags         column;
    FilterType          filterMode;
    FontModeType        fontModeMain;
    ModeType            controlMode;
    ScrollbarType       scrollbar;
    SortOrderType       sortOrder;
    SortType            sortType;
    ToolbarType         toolbar;
    SelectType          hwMode[ NUM_OF_HW_BUTTONS ];
    SelectType          gestMode[ NUM_OF_GESTURES ];
    SelectType          select[ NUM_OF_CONTROL_MODES ][ NUM_OF_CONTROL_LISTS ];
    SelectType          jogMode[ NUM_OF_JOGEVENTS ];
    Boolean             jogEnabled;
    UInt8               unused0;  /* used to be previousSilkStatus */
    FontModeType        fontModeLibrary;
    CategoryStyleType   categoryStyle;
    Boolean             enableSoftHyphens;
    HardcopyActionType  hardcopyAction;
    HardcopyRangeType   hardcopyRange;
    HardcopyLinkType    hardcopyLink;
    Int16               lineSpacing;
    Int16               paragraphSpacing;
    SelectType          arrowMode[ NUM_OF_ARROW_BUTTONS ];
    Boolean             arrowKeys;
    Boolean             dynamicScrollbar;
    Boolean             visualAid;
    Boolean             indicateOpened;
    Char                mainUserFontName[ dmDBNameLength ];
    Char                libraryUserFontName[ dmDBNameLength ];
    Boolean             individualDocumentFonts;
    FontModeType        defaultFontModeMain;
    Int16               defaultLineSpacing;
    Int16               defaultParagraphSpacing;
    Char                defaultMainUserFontName[ dmDBNameLength ];
    Boolean             forceDefaultColors;
    ToolbarType         savedToolbar;
    ScrollbarType       savedScrollbar;
    DIAStateType        savedSilkscreen;
    Boolean             pageControlsLink;
    ForceAlignType      forceAlign;
    Boolean             joinUpAllRecords;
    RotateType          rotate;
    RotateType          defaultRotate;
    SelectWordTapType   selectWordTap;
    SelectedWordActionType selectedWordAction;
    Boolean             hideSonyStatusBar;
    Boolean             useDateTime;
    UInt16              searchXlit;
} Preferences;


/*
   Old viewer preferences
 */
typedef struct {
    LocalID             dbID;
    Boolean             strikethrough;
    Boolean             showDate;
    Boolean             linkClick;
    Boolean             multipleSelect;
    ScrollbarType       scrollbar;
    ModeType            controlMode;
    SelectType          select[ NUM_OF_CONTROL_MODES ][ NUM_OF_CONTROL_LISTS ];
    UInt16              toolbar;
    UInt16              screenDepth;
    Boolean             searchInAllPages;
    Boolean             caseSensitive;
    UInt16              searchEntries;
    Boolean             showSize;
    UInt16              categories;
    UInt16              cardNo;
    Char                docName[ dmDBNameLength ];
    UInt16              toolbarButton;
    FilterType          filterMode;
    UInt16              lastForm;
    Boolean             underlineMode;
    FontModeType        fontMode;
    Boolean             hardKeys;
    SelectType          hwMode[ NUM_OF_HW_BUTTONS ];
    SortType            sortType;
    SortOrderType       sortOrder;
    Boolean             gestures;
    SelectType          gestMode[ NUM_OF_GESTURES ];
    Boolean             autoscrollEnabled;
    UInt16              autoscrollInterval;
    UInt16              autoscrollLastScrollTime;
    UInt16              autoscrollJump;
    AutoscrollModeType  autoscrollMode;
    AutoscrollDirType   autoscrollDir;
    Boolean             autoscrollStayOn;
    UInt16              location;
    Boolean             doManualSync;
} OldPreferences;


/* Retrieve search patterns, call ReleaseSearchPatterns to release the memory
   allocated by this function */
extern Char* GetSearchPatterns( void ) PREFSDATA_SECTION;

/* Retrieve the value of a preference ( through the pointer ) */
extern Preferences* Prefs( void ) PREFSDATA_SECTION;

/* Add new search string to patterns list */
extern void AddSearchString( Char* string ) PREFSDATA_SECTION;

/* Retrieve search string */
extern void GetSearchString( Char* string ) PREFSDATA_SECTION;

/* Retrieve preferences from the Preferences database */
extern void ReadPrefs( void ) PREFSDATA_SECTION;

/* Release memory allocated by GetSearchPatterns */
extern void ReleaseSearchPatterns( void ) PREFSDATA_SECTION;

/* Store preferences in the Preferences database */
extern void WritePrefs( void ) PREFSDATA_SECTION;

#endif

