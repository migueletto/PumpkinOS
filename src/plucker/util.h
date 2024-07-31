/*
 * $Id: util.h,v 1.71 2004/04/19 02:56:25 prussar Exp $
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

#ifndef PLUCKER_UTIL_H
#define PLUCKER_UTIL_H

#include "viewer.h"

#define NOT_FOUND  -1
#define NO_RECORD   0xFFFF
#define TOP_OFFSET  0
#define NO_OFFSET  -1
#define NO_VERTICAL_OFFSET 0x7FFFFFFF
#define PREVIOUS_FORM 0

#undef ASSERT
#undef ASSERT_MSG
#define ASSERT(c)           ErrFatalDisplayIf(!(c),#c)
#define ASSERT_MSG(msg,c)   ErrFatalDisplayIf(!(c),msg)

//#define OFFSETOF(TYPE, MEMBER) ((UInt32) &((TYPE *)0)->MEMBER)
#define OFFSETOF(TYPE, MEMBER) OffsetOf(TYPE, MEMBER)

#define STREQ(a,b)  (StrCompare((a),(b)) == 0)

/* if expression is true then throw exception -- only to be used within
   a Try/Catch block */
#define THROW_IF( exp, err )  do { \
                                if ( ( exp ) ) { \
                                    ErrThrow( ( err ) ); \
                                } \
                              } while ( 0 )

/* For ExtentXY() and TopLeftXY() calculations */
typedef enum {
    MARGINS_NORMAL = 0,
    MARGINS_WHEN_ROTATED
} MarginType;

typedef struct {
    Coord top;
    Coord bottom;
    Coord left;
    Coord right;
} Margins;

typedef struct {
    Coord   scrollbar;
    Coord   toolbar;
    Margins margins;
} WidthsType;


/* Switch draw function set for screen and memopad */
typedef enum
{
    WRITEMODE_NO_DRAW,
    WRITEMODE_DRAW_CHAR,
#ifdef SUPPORT_WORD_LOOKUP
    WRITEMODE_FIND_SELECTED_WORD,
#endif
    WRITEMODE_COPY_CHAR,
    WRITEMODE_SERIAL_TEXT,      /* hardcopy without link, newline, hline, etc */
    WRITEMODE_LAYOUT_ANCHORS    /* lay everything out, set up anchors, but
                                   don't draw */
} WriteModeType;

/* Does the write mode require us to precisely lay out every line on
   the whole screen? */
#define DO_LAYOUT_WHOLE_SCREEN( w )  ( ( w ) == WRITEMODE_DRAW_CHAR || \
                                       ( w ) == WRITEMODE_LAYOUT_ANCHORS )
/* Does it require us to lay out the contents of any line precisely? */
#ifdef SUPPORT_WORD_LOOKUP
#define DO_LAYOUT( w )               ( DO_LAYOUT_WHOLE_SCREEN( w ) || \
                                       ( w ) == WRITEMODE_FIND_SELECTED_WORD )
#else
#define DO_LAYOUT( w )               DO_LAYOUT_WHOLE_SCREEN( w )
#endif

/* Is the write mode such as to require us to set up the anchors? */
#define DO_ANCHORS( w )      ( ( w ) == WRITEMODE_DRAW_CHAR || \
                               ( w ) == WRITEMODE_LAYOUT_ANCHORS )

#undef min
#undef max
#define min(a,b) (((a) < (b)) ? (a) : (b))
#define max(a,b) (((b) < (a)) ? (a) : (b))

#define hlEnable  1
#define hlText    2

/*
   Define a "TextContext" which is a drawing context for a string
   (or possibly a series of strings) */
typedef struct {
    Int16           cursorX;
    YOffset         cursorY;
    WriteModeType   writeMode;
    Boolean         activeAnchor;
    RectangleType   italic;
    RectangleType   searchPattern;
    RectangleType   strike;
    RGBColorType    foreColor;
    Int16           findCharacterPosition;  /* position of character to find */
    YOffset         foundYPosition;         /* Y-position of it when found */
    Int16           characterPosition;      /* character position of viewport */
#ifdef SUPPORT_WORD_LOOKUP
    Coord           findPixelX;
    Coord           findPixelY;
#endif
} TextContext;


/* Definition of HighlightType in order to define certain schemes to use */
typedef enum {
    BUTTON = 0,
    TEXT
} HighlightType;


/* Initialize screen boundaries */
extern void InitializeViewportBoundaries(void) UTIL_SECTION;

/* Update the maxExtent[XY] coords only */
extern void UpdateDisplayExtent( void ) UTIL_SECTION;

/* Get the vertical extent to scroll for one page in NATIVE coordinates */
extern Int16 GetScrollValue( void ) UTIL_SECTION;

/* Return max width for viewport boundary */
extern Int16 MaxExtentX(void) UTIL_SECTION;

/* Return max height for viewport boundary */
extern Int16 MaxExtentY(void) UTIL_SECTION;

/* Set common pointers to goto standard functions */
extern void SetStandardFunctions( void );

/* Return X coordinate for viewport boundary */
extern Int16 StandardTopLeftX( void ) UTIL_SECTION;

/* Return Y coordinate for viewport boundary */
extern Int16 StandardTopLeftY( void ) UTIL_SECTION;

/* Return width for viewport boundary */
extern Int16 StandardExtentX( void ) UTIL_SECTION;

/* Return height for viewport boundary */
extern Int16 StandardExtentY( void ) UTIL_SECTION;

/* Move an object around the screen, relatively speaking */
extern void MoveObjectRelatively( FormType* form, UInt16 objectID, Coord moveX,
                Coord moveY ) UTIL_SECTION;

/* Send appStopEvent */
extern void SendAppStopEvent( void ) UTIL_SECTION;

/* Add a strike-through for anchor/text */
extern void StrikeThrough( const RectangleType* box ) UTIL_SECTION;

/* Trim text for specified length */
extern void TrimText( Char* text, Int16 len ) UTIL_SECTION;

/* Return a pointer to an object in a form */
extern void* GetObjectPtr( const Int16 objectID ) UTIL_SECTION;

/* will always return a valid pointer to allocated memory or throw
   an exception */
extern MemPtr SafeMemPtrNew( UInt32 size ) UTIL_SECTION;

/* free a pointer to allocated memory */
extern void SafeMemPtrFree( MemPtr ptr ) UTIL_SECTION;

/* will always return a valid handle to allocated memory or throw
   an exception */
extern MemHandle SafeMemHandleNew( UInt32 size ) UTIL_SECTION;

/* free a handle to allocated memory */
extern void SafeMemHandleFree( MemHandle ptr ) UTIL_SECTION;

/* write message to memo */
extern void WriteMemoEntry( UInt16* index, UInt32* offset,
                Char* fmt, ... ) UTIL_SECTION;

/* Initialize language dependant features */
extern void InitializeLanguageSupport( void ) UTIL_SECTION;

/* return the hires equivilant for a given form, only really useful for
 * translation between regular and handera specific forms */
extern UInt16 GetValidForm( UInt16 currentFormId ) UTIL_SECTION;

/* set a pointer to form variable to its actual GetValidForm() value,
 * returns true if GetValueForm()'s results altered from provided formId */
extern Boolean SetValidForm( UInt16* formId ) UTIL_SECTION;

/* return whether or not provided formId has a visible toolbar */
extern Boolean IsVisibleToolbar( UInt16 formId ) UTIL_SECTION;

/* return whether or not provided formId is a 'main' form */
extern Boolean IsFormMain( UInt16 formId ) UTIL_SECTION;

/* return whether or not provided formId is a 'library' form */
extern Boolean IsFormLibrary( UInt16 formId );

/* return formId for toolbar that is currently set via Prefs()->toolbar */
extern UInt16 GetMainFormId( void ) UTIL_SECTION;

/* Highlight either a button or text according to the UI Standards
 * for the device */
extern void HighlightRectangle( RectangleType* rp, UInt16 cornerDiam,
                Boolean enable, HighlightType highlight ) UTIL_SECTION;


extern void InitializeActionList(UInt16 listID) UTIL_SECTION;
extern void ReleaseActionList(void) UTIL_SECTION;

extern void InitializeKeyList(UInt16 listID) UTIL_SECTION;
extern void ReleaseKeyList(void) UTIL_SECTION;

/* Properly place and draw the help icon */
extern void DrawHelpIcon( FormType* form, UInt16 objectID ) UTIL_SECTION;

/* Properly place and draw the help icon */
extern void DrawHelpIconAtXY( FormType* form, UInt16 objectID,
    Coord x, Coord y ) UTIL_SECTION;

/* Just return the value for STANDARD coordinate system */
extern UInt16 StandardCoordinateSystem( void ) UTIL_SECTION;

/* Update margins */
extern void StandardUpdateMargins( void ) UTIL_SECTION;

/* Set the margins according to the widths and current environmental
   settings. */
extern void SetMargins(Margins* margins, WidthsType* widths) UTIL_SECTION;

#if 0
/*Show small message window without any control*/
extern FormType* PopupMessage( Int16 formId ) UTIL_SECTION;

/*Hide popup window created by PopupMessage */
extern void HidePopup( FormType* frm ) UTIL_SECTION;
#endif

/* Insert text in to a textfield and scroll to the top */
extern void InsertText( FieldType* field, Char* text ) UTIL_SECTION;

/* Scrolls the message field */
extern void ScrollMessage(FieldType* field, Int16 lines) UTIL_SECTION;

/* Quick and dirty check for enough memory to send a message or copy
   a page of text to memo ( 5000 bytes for body 150 for headers and some
   more for palm ) */
extern void CheckMem(UInt16 alertID) UTIL_SECTION;

/* Updates the scrollbar and scroll arrows */
extern void UpdateFieldScrollbar( FieldType* field,
                ScrollBarType* scrollbar ) UTIL_SECTION;

/* Set position of object in form */
extern void SetObjectPosition( FormType* form, UInt16 id,
                Boolean firstColumn ) UTIL_SECTION;

/* Create internal MemoDB */
extern Err CreateInternalMemoDB( void ) UTIL_SECTION;

/* Delete the internal MemoDB (after writing the data to the real MemoDB). */
extern void DeleteInternalMemoDB( void ) UTIL_SECTION;

/* Check low power situation */
extern Boolean HasEnoughPower( void ) UTIL_SECTION;


PLKR_GLOBAL void (*UpdateMargins)( void );
PLKR_GLOBAL Int16 (*TopLeftX)( void );
PLKR_GLOBAL Int16 (*TopLeftY)( void );
PLKR_GLOBAL Int16 (*ExtentX)( void );
PLKR_GLOBAL Int16 (*ExtentY)( void );

#endif

