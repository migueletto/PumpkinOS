/*
 * $Id: util.c,v 1.87 2004/04/29 01:52:06 prussar Exp $
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
#include <PalmOS.h>
#include <stdarg.h>
#include <FntGlue.h>

#include "debug.h"
#include "dimensions.h"
#include "hires.h"
#include "prefsdata.h"
#include "os.h"
#include "document.h"
#include "search.h"
#include "resourceids.h"
#include "rotate.h"
#include "font.h"
#include "const.h"
#include "resize.h"

#include "util.h"
#include "pumpkin.h"
#include "../libpit/debug.h"

#ifdef HAVE_PALMCUNIT
#include "mock_Header.h"
#endif


/***********************************************************************
 *
 *      Internal Constants
 *
 ***********************************************************************/
static const Char internalMemoDBName[] = "Plkr-MemoDB";


/***********************************************************************
 *
 *      Local functions
 *
 ***********************************************************************/
static Char** GetActionStrings(void) UTIL_SECTION;
static Char** GetKeyStrings(void) UTIL_SECTION;
static void WriteToMemo( void ) UTIL_SECTION;


/***********************************************************************
 *
 *      Private variables
 *
 ***********************************************************************/
static Int16 maxExtentX;
static Int16 maxExtentY;
static MemHandle actionHandle;
static MemHandle actionList;
static UInt16 numberOfActionStrings;
static MemHandle keyHandle;
static MemHandle keyList;
static UInt16 numberOfKeyStrings;
static Margins currentMargins;
static WidthsType standardWidths[ NUM_ROTATE_TYPES ] = {
    { SCROLLBAR_WIDTH, TOOLBAR_HEIGHT,
          { TOP_MARGIN, BOTTOM_MARGIN, LEFT_MARGIN, RIGHT_MARGIN } }
#ifdef HAVE_ROTATE
    ,
    { SCROLLBAR_WIDTH, TOOLBAR_HEIGHT,
          { LEFT_MARGIN, RIGHT_MARGIN, 0, 0 } },
    { SCROLLBAR_WIDTH, TOOLBAR_HEIGHT,
          { RIGHT_MARGIN, LEFT_MARGIN, 0, 0 } }
#endif
};


/* Initialize screen boundaries */
void InitializeViewportBoundaries( void )
{
    RectangleType display;
    FormType*     activeForm;

    UpdateDisplayExtent();
    display.topLeft.x = 0;
    display.topLeft.y = 0;
    display.extent.x = maxExtentX;
    display.extent.y = maxExtentY;

    /* Because this function can be called before we have an active form,
       validate that there is one before we try adjusting any bounds */
    activeForm = FrmGetActiveForm();
    if ( activeForm != NULL )
        WinSetWindowBounds( FrmGetWindowHandle( activeForm ), &display );
}



/* Update the maxExtent[XY] coords only */
void UpdateDisplayExtent( void )
{
    WinGetDisplayExtent( &maxExtentX, &maxExtentY );
}



/* Set the current margins */
void SetMargins
        (
        Margins*    margins,
        WidthsType* widths
        )
{
    *margins = widths->margins;
    switch ( Prefs()->scrollbar ) {
        case SCROLLBAR_LEFT:
#ifdef HAVE_ROTATE
            if ( Prefs()->rotate == ROTATE_MINUS90 )
                margins->top  += widths->scrollbar;
            else if ( Prefs()->rotate == ROTATE_PLUS90 )
                margins->bottom += widths->scrollbar;
            else
#endif
                margins->left += widths->scrollbar;
            break;
        case SCROLLBAR_RIGHT:
#ifdef HAVE_ROTATE
            if ( Prefs()->rotate == ROTATE_MINUS90 )
                margins->bottom += widths->scrollbar;
            else if ( Prefs()->rotate == ROTATE_PLUS90 )
                margins->top    += widths->scrollbar;
            else
#endif
                margins->right  += widths->scrollbar;
            break;
        default:
            break;
    }
    switch ( Prefs()->toolbar ) {
        case TOOLBAR_TOP:
            margins->top    += widths->toolbar;
            break;
        case TOOLBAR_BOTTOM:
            margins->bottom += widths->toolbar;
            break;
        default:
            break;
    }
}



/* Update margins: call whenever toolbar, scrollbar or rotation state
   changes */
void StandardUpdateMargins( void )
{
    if ( NUM_ROTATE_TYPES < Prefs()->rotate )
        Prefs()->rotate = ROTATE_ZERO;
    SetMargins( &currentMargins, &standardWidths[ Prefs()->rotate ] );
}



/* Set common pointers to goto standard functions */
void SetStandardFunctions( void )
{
    UpdateMargins  = StandardUpdateMargins;
    TopLeftX       = StandardTopLeftX;
    TopLeftY       = StandardTopLeftY;
    ExtentX        = StandardExtentX;
    ExtentY        = StandardExtentY;
    HiResStop      = NULL;
}



/* Return X coordinate for viewport boundary */
Int16 StandardTopLeftX( void )
{
    return currentMargins.left;
}



/* Return Y coordinate for viewport boundary */
Int16 StandardTopLeftY( void )
{
    return currentMargins.top;
}



/* Return width for viewport boundary */
Int16 StandardExtentX( void )
{
    return maxExtentX - currentMargins.right - currentMargins.left;
}



/* Return height for viewport boundary */
Int16 StandardExtentY( void )
{
    return maxExtentY - currentMargins.bottom - currentMargins.top;
}



/* Return max width for viewport boundary */
Int16 MaxExtentX( void )
{
    return maxExtentX;
}



/* Return max height for viewport boundary */
Int16 MaxExtentY( void )
{
    return maxExtentY;
}



/* Get the vertical extent to scroll for one page in NATIVE coordinates */
Int16 GetScrollValue( void )
{
    UInt16  prevCoordSys;
    Int16   scrollValue;

    prevCoordSys = PalmSetCoordinateSystem( NATIVE );

    scrollValue = ExtentY() - GetDefaultMainStyleHeight();

    PalmSetCoordinateSystem( prevCoordSys );

    return scrollValue;
}



/* Return a pointer to an object in a form */
void* GetObjectPtr
    (
    const Int16 objectID    /* ID of an object in the form */
    )
{
    FormType* frm;

    frm = FrmGetActiveForm();
    return FrmGetObjectPtr( frm, FrmGetObjectIndex( frm, objectID ) );
}



/* Trim text for specified length */
void TrimText
    (
    Char* text, /* pointer to text */
    Int16 len   /* max length of string, i.e. trim up to this length */
    )
{
    Int16 length;

    length = StrLen( text );
    if ( len < FntCharsWidth( text, length ) ) {
        text[ FntGlueWidthToOffset( text, length, len - 6, NULL, NULL ) ] = 0;
        StrCat( text, "..." );
    }
}



/* Move an object around the screen, relatively speaking */
void MoveObjectRelatively
    (
    FormType* form,
    UInt16    objectID,
    Coord     moveX,
    Coord     moveY
    )
{
    UInt16 index;
    Coord  x;
    Coord  y;

    index = FrmGetObjectIndex( form, objectID );
    FrmGetObjectPosition( form, index, &x, &y );
    FrmSetObjectPosition( form, index, x + moveX, y + moveY );
}



/* Send appStopEvent */
void SendAppStopEvent( void )
{
    EventType stop;

    MemSet( &stop, sizeof( EventType ), 0 );
    stop.eType = appStopEvent;
    EvtAddEventToQueue( &stop );
}



/* Add a strike-through for anchor/text */
void StrikeThrough
    (
    const RectangleType* box    /* boundaries for anchor/text */
    )
{
    Int16 anchorY;

    anchorY = box->topLeft.y + box->extent.y - FntCharHeight() / 2;
    RotDrawLine( box->topLeft.x, anchorY, box->topLeft.x + box->extent.x, anchorY );
}


/* will always return a valid pointer to allocated memory or throw
   an exception */
MemPtr SafeMemPtrNew
    (
    UInt32 size
    )
    /* THROWS */
{
    MemPtr buf;

    if ( size == 0 )
        ErrThrow( memErrInvalidParam );

    buf = MemPtrNew( size );
    if ( buf == NULL )
        ErrThrow( memErrNotEnoughSpace );

    return buf;
}



/* free a pointer to allocated memory */
void SafeMemPtrFree
    (
    MemPtr ptr
    )
{
    if ( ptr != NULL )
        MemPtrFree( ptr );
}



/* will always return a valid handle to allocated memory or throw
   an exception */
MemHandle SafeMemHandleNew
    (
    UInt32 size
    )
    /* THROWS */
{
    MemHandle handle;

    if ( size == 0 )
        ErrThrow( memErrInvalidParam );

    handle = MemHandleNew( size );
    if ( handle == NULL )
        ErrThrow( memErrNotEnoughSpace );

    return handle;
}



/* free a handle to allocated memory */
void SafeMemHandleFree
    (
    MemHandle handle
    )
{
    if ( handle != NULL )
        MemHandleFree( handle );
}



/* PalmOne did some "tricks" with the PIM databases on T|3 and T|E,
   which prevents us from working directly with the MemoDB. Instead
   we create an 'internal' MemoDB and write to that database; before
   the app exits we write the stored data to the real MemoDB */
Err CreateInternalMemoDB( void )
{
    /* If the device crashed before writing the text to the real MemoDB
       we will have a left-over internal MemoDB; write the text to the
       real MemoDB and remove the internal MemoDB */
    DeleteInternalMemoDB();

    return DmCreateDatabase( 0, internalMemoDBName, ViewerAppID,
            PlkrMemoDBType, false );
}



/* Delete the internal MemoDB (after writing the data to the real MemoDB). */
void DeleteInternalMemoDB( void )
{
    LocalID dbId;

    dbId = DmFindDatabase( 0, internalMemoDBName );
    if ( dbId != 0 ) {
        WriteToMemo();
        DmDeleteDatabase( 0, dbId );
    }
}



/* Write data from internal MemoDB to real MemoDB. */
static void WriteToMemo( void )
{
    DmOpenRef   MemopadDB;
    DmOpenRef   InternalMemopadDB;
    UInt16      i;

    /* Open real and internal MemoDBs. */
    MemopadDB = DmOpenDatabaseByTypeCreator( 'DATA', 'memo', dmModeReadWrite );
    if ( MemopadDB == NULL ) {
        return;
    }
    InternalMemopadDB = DmOpenDatabaseByTypeCreator( PlkrMemoDBType,
                        ViewerAppID, dmModeReadWrite );
    if ( InternalMemopadDB == NULL ) {
        DmCloseDatabase( MemopadDB );
        return;
    }

    /* Traverse all records in the internal MemoDB and write the
       data to the real MemoDB. */
    for ( i = 0; i < DmNumRecords( InternalMemopadDB ); i++ ) {
        MemHandle srcHandle;

        srcHandle = DmQueryRecord( InternalMemopadDB, i );
        if ( srcHandle != NULL ) {
            MemHandle   destHandle;
            UInt16      index;
            UInt16      attr;
            Int16       category;

            index       = dmMaxRecordIndex;
            destHandle  = DmNewRecord( MemopadDB, &index,
                            MemHandleSize( srcHandle ) );
            if ( destHandle != NULL ) {
                MemPtr  srcPtr;
                MemPtr  destPtr;

                srcPtr  = MemHandleLock( srcHandle );
                destPtr = MemHandleLock( destHandle );
                DmWrite( destPtr, 0, srcPtr, MemHandleSize( srcHandle ) );
                MemHandleUnlock( destHandle );
                MemHandleUnlock( srcHandle );
            
                /* If there is a Plucker category we assign that
                   category to the record. Otherwise, we use the
                   Unfiled category. */
                DmRecordInfo( MemopadDB, index, &attr, NULL, NULL );
                attr &= ~dmRecAttrCategoryMask;

                category = CategoryFind( MemopadDB, "Plucker" );
                if ( category == dmAllCategories )
                    attr |= dmUnfiledCategory;
                else
                    attr |= category;

                DmSetRecordInfo( MemopadDB, index, &attr, NULL );
                DmReleaseRecord( MemopadDB, index, true );
            }
        }
    }
    DmCloseDatabase( InternalMemopadDB );
    DmCloseDatabase( MemopadDB );
}



#define MAX_MSG_LEN     3000

/* Write message to memo. */
void WriteMemoEntry
    (
    UInt16* index,  /* record index in MemoDB */
    UInt32* offset, /* offset to location in record */
    Char*   fmt,    /* format, see printf( 3 ) for details */
    ...             /* arguments */
    )
{
    static Char msg[ MAX_MSG_LEN ];

    va_list     args;
    DmOpenRef   MemopadDB;
    MemHandle   recH;
    UInt16      length;
    Boolean     releaseRecord = false;

    va_start( args, fmt );
    StrVPrintF( msg, fmt, args );
    va_end( args );

    length = StrLen( msg );
    if ( length == 0 )
        return;

    MemopadDB = DmOpenDatabaseByTypeCreator( PlkrMemoDBType, ViewerAppID,
                    dmModeReadWrite );
    if ( MemopadDB == NULL )
        return;

    if ( *index == dmMaxRecordIndex )
    {
        recH = DmNewRecord( MemopadDB, index, length + 1 );
        releaseRecord = true;
    }
    else
    {
        recH = DmResizeRecord( MemopadDB, *index, *offset + length + 1 );
        releaseRecord = false;
    }

    if ( recH != NULL ) {
        Char    null;
        MemPtr  recP;

        null = '\0';
        recP = MemHandleLock( recH );
        DmWrite( recP, *offset, msg, length );
        DmWrite( recP, *offset + length, &null, 1 );
        MemHandleUnlock( recH );
        if ( releaseRecord )
        {
            DmReleaseRecord( MemopadDB, *index, true );
        }

        *offset += length;
    }
    DmCloseDatabase( MemopadDB );
}



/* Initialize language dependant features */
void InitializeLanguageSupport( void )
{
    Char lang[ 6 ] = { 0 }; /* Max strlen for a language ISO-CODE is 5+1 */

    SysCopyStringResource( lang, strLang );

    InitializeOffsetFormat( lang );
    InitializeResultFormat( lang );
}



/* return the hires equivilant for a given form, only really useful for
   translation between regular and handera specific forms */
UInt16 GetValidForm
    (
    UInt16 formId    /* form to use as a base */
    )
{
    UInt16           properFormId;
    HiRes            hiResType;

    properFormId  = formId;
    hiResType     = HiResType();

    switch ( formId ) {
        case frmMainTop:
        case frmMainTopHandera:
            if ( IsHiResTypeHandera( hiResType ) )
                properFormId = frmMainTopHandera;
            else
                properFormId = frmMainTop;
            break;

        case frmMainBottom:
        case frmMainBottomHandera:
            if ( IsHiResTypeHandera( hiResType ) )
                properFormId = frmMainBottomHandera;
            else
                properFormId = frmMainBottom;
            break;

        case frmMainNone:
        case frmMainNoneHandera:
            if ( IsHiResTypeHandera( hiResType ) )
                properFormId = frmMainNoneHandera;
            else
                properFormId = frmMainNone;
            break;

        case frmLibrary:
        case frmLibraryHandera:
            if ( IsHiResTypeHandera( hiResType ) )
                properFormId = frmLibraryHandera;
            else
                properFormId = frmLibrary;
            break;

        case frmFont:
        case frmFontOS2:
#ifdef HAVE_HANDERA_SDK
        case frmFontHandera:
            if ( IsHiResTypeHandera( HiResType() ) )
                properFormId = frmFontHandera;
            else
#endif
            if ( Support30() )
                properFormId = frmFont;
            else
                properFormId = frmFontOS2;
            break;
    }

    return properFormId;
}



/* set a pointer to form variable to its actual GetValidForm() value,
   returns true if GetValueForm()'s results altered from provided formId */
Boolean SetValidForm
    (
    UInt16* formId      /* formId to check/set against */
    )
{
    UInt16  currentFormId;

    currentFormId = *formId;
    *formId = GetValidForm( currentFormId );
    if ( currentFormId != *formId ) {
        /* Immediatly jump to that form */
        FrmGotoForm( *formId );
        return true;
    }
    else {
        return false;
    }
}



/* return whether or not provided formId has a visible toolbar */
Boolean IsVisibleToolbar
    (
    UInt16 formId   /* formId to check against */
    )
{
/* meant to be a quick and easy way to skip stupid wasteful statments like:
  
   if ( formId == frmMainTop ||
        formId == frmMainBottom ) {
       [...]
   }
 */

    if ( formId == frmMainTop ||
         formId == frmMainBottom ||
         formId == frmMainTopHandera ||
         formId == frmMainBottomHandera )
        return true;
    else
        return false;
}



/* return whether or not provided formId is a 'main' form */
Boolean IsFormMain
    (
    UInt16 formId   /* formId to check against */
    )
{
/* meant to be a quick and easy way to skip stupid wasteful statments like:
  
   if ( formId == GetValidForm( frmMainTop ) ||
        formId == GetValidForm( frmMainBottom ) ||
        formId == GetValidForm( frmMainNone ) ) {
       [...]
   }
 */

    if ( formId == frmMainTop ||
         formId == frmMainBottom ||
         formId == frmMainNone ||
         formId == frmMainTopHandera ||
         formId == frmMainBottomHandera ||
         formId == frmMainBottomHanderaLow ||
         formId == frmMainNoneHandera )
        return true;
    else
        return false;
}



/* return whether or not provided formId is a 'library' form */
Boolean IsFormLibrary
    (
    UInt16 formId   /* formId to check against */
    )
{
    if ( formId == frmLibrary           ||
         formId == frmLibraryHandera    ||
         formId == frmLibraryHanderaLow ||
         formId == frmLibrarySonyWide )
        return true;
    else
        return false;
}




/* return formId for toolbar that is currently set via Prefs()->toolbar */
UInt16 GetMainFormId( void )
{
    const ToolbarType toolbar = Prefs()->toolbar;
    UInt16            toolbarId;

    switch ( toolbar ) {
        case TOOLBAR_TOP:
            toolbarId = GetValidForm( frmMainTop );
            break;

        case TOOLBAR_BOTTOM:
            toolbarId = GetValidForm( frmMainBottom );
            break;

        case TOOLBAR_NONE:
        case TOOLBAR_SILK:
        default:
            toolbarId = GetValidForm( frmMainNone );
            break;
    }
    return toolbarId;
}



/* Highlight either a button or text according to the UI Standards
   for the device */
void HighlightRectangle
    (
    RectangleType* r,          /* Pointer to RectangleType to highlight */
    UInt16         diam,       /* corner diam for rounded edges if != 0 */
    Boolean        enable,     /* true to enable highlight, false to remove */
    HighlightType  highlight   /* highlight scheme to use */
    )
{
    /* Pre 3.5 devices do not support the fancy WinPush, WinPop, WinPaint
       functions. Instead just invert and leave it at that */
    if ( ! Support35() ) {
        WinInvertRectangle( r, diam );
        return;
    }

    WinPushDrawState();
    WinSetDrawMode( winSwap );
    WinSetPatternType( blackPattern );

    /* If we're highlighting TEXT, set it to have a yellow background */
    if ( highlight == TEXT )
        WinSetForeColor( UIColorGetTableEntryIndex( UIFieldTextHighlightBackground ) );
    /* If we're highlighting a BUTTON, set it to have blue a background
       and the white text (handled by first inverting, then swapping) */
    else if ( highlight == BUTTON )
        WinSetBackColor( UIColorGetTableEntryIndex( UIObjectSelectedFill ) );

    /* If we're highlighting by an unexpected method, winSwap/WinPaintRectangle
       will default to inverting white and black */
    if ( enable ) {
        if ( highlight == BUTTON )
            WinInvertRectangle( r, diam );
        WinPaintRectangle( r, diam );
    }
    else {
        WinPaintRectangle( r, diam );
        if ( highlight == BUTTON )
            WinInvertRectangle( r, diam );
    }
    WinPopDrawState();
}



static Char** GetActionStrings(void)
{
    static Char** actionStrings;

    if ( actionHandle == NULL ) {
        MemPtr actionTable;
        UInt16 prefixLen;

        actionHandle = DmGetResource('tSTL', strTblActions);
        actionTable = MemHandleLock(actionHandle);

        prefixLen = StrLen((Char*)actionTable);
        numberOfActionStrings = (*((UInt8*)(actionTable +
                                            prefixLen + 1)) << 8) |
                                 *((UInt8*)(actionTable + prefixLen + 2));
        actionList = SysFormPointerArrayToStrings( actionTable + prefixLen + 3,
                        numberOfActionStrings );
        actionStrings = MemHandleLock( actionList );
    }
    return actionStrings;
}


void InitializeActionList(UInt16 listID)
{
    ListType*   list;
    Char**      actions;

    list = GetObjectPtr( listID );

    actions = GetActionStrings();

    LstSetListChoices( list, actions, numberOfActionStrings );
    LstSetHeight( list, numberOfActionStrings );
}



void ReleaseActionList(void)
{
    if ( actionHandle != NULL ) {
        MemHandleUnlock(actionList);
        MemHandleFree(actionList);

        MemHandleUnlock(actionHandle);
        DmReleaseResource(actionHandle);
        actionHandle = NULL;
    }
}



static Char** GetKeyStrings(void)
{
    static Char** keyStrings;

    if ( keyHandle == NULL ) {
        MemPtr keyTable;
        UInt16 prefixLen;

        keyHandle = DmGetResource('tSTL', strTblKeys);
        keyTable = MemHandleLock(keyHandle);

        prefixLen = StrLen((Char*)keyTable);
        numberOfKeyStrings = (*((UInt8*)(keyTable +
                                            prefixLen + 1)) << 8) |
                                 *((UInt8*)(keyTable + prefixLen + 2));
        keyList = SysFormPointerArrayToStrings( keyTable + prefixLen + 3,
                        numberOfKeyStrings );
        keyStrings = MemHandleLock( keyList );
    }
    return keyStrings;
}


void InitializeKeyList(UInt16 listID)
{
    ListType*   list;
    Char**      keys;

    list = GetObjectPtr( listID );

    keys = GetKeyStrings();

    LstSetListChoices( list, keys, numberOfKeyStrings );
    LstSetHeight( list, numberOfKeyStrings );
}



void ReleaseKeyList(void)
{
    if ( keyHandle != NULL ) {
        MemHandleUnlock(keyList);
        MemHandleFree(keyList);

        MemHandleUnlock(keyHandle);
        DmReleaseResource(keyHandle);
        keyHandle = NULL;
    }
}



/* Properly place and draw the help icon */
void DrawHelpIconAtXY
    (
    FormType* form,
    UInt16    objectID,
    Coord     x,
    Coord     y
    )
{
    Coord         rad;
    RectangleType r;
    FontID        oldFont;
    Char          helpChar;

    rad = 4;

    FrmGetObjectBounds( form, FrmGetObjectIndex( form, objectID ), &r );

    HiResAdjustBounds( &r, sonyHiRes );
    HiResAdjust( &rad, sonyHiRes );
    HiResAdjust( &x, sonyHiRes | handeraHiRes );
    HiResAdjust( &y, sonyHiRes | handeraHiRes );

    WinEraseRectangle( &r, rad );

    oldFont = FntSetFont( HiResFont( symbolFont ) );
    helpChar = symbolHelp;
    WinDrawChars( &helpChar, 1, r.topLeft.x + x, y );
    FntSetFont( oldFont );
}



/* Properly place and draw the help icon */
void DrawHelpIcon
    (
    FormType* form,
    UInt16    objectID
    )
{
    DrawHelpIconAtXY( form, objectID, 3, 2 );
}



/* Just return the value for STANDARD coordinate system */
UInt16 StandardCoordinateSystem( void )
{
    /* Only meant as a fallback function if we're building without hires
       support. Because PalmSetCoordinateSystem() is used throughout, replacing
       it simply by defining the function name as STANDARD results in build
       errors. It needs to be a real function unfortunatly */
    return STANDARD;
}



#if 0
/*Show small message window without any control*/
FormType* PopupMessage
    (
    Int16 formId
    )
{
    FormType* frm;

    frm = FrmInitForm( frmExportLink );
    FrmDrawForm( frm );

    return frm;
}




/*Hide popup window created by PopupMessage */
void HidePopup
    (
    FormType* frm
    )
{
    FrmEraseForm( frm );
    FrmDeleteForm( frm );
}
#endif


/* Insert text in to a textfield and scroll to the top */
void InsertText
    (
    FieldType*  field,  /* field to insert into */
    Char*       text    /* text to insert */
    )
{
    FldInsert( field, text, StrLen( text ) );
    while ( FldScrollable( field, winUp ) )
        FldScrollField( field, 1, winUp );
}



/* Set position of object in form */
void SetObjectPosition
    (
    FormType* form,
    UInt16  id,         /* object ID */
    Boolean firstColumn /* object should be placed in first column */
    )
{
    UInt16      index;
    Coord       x;
    Coord       y;

    index   = FrmGetObjectIndex( form, id );
    FrmGetObjectPosition( form, index, &x, &y );

    if ( firstColumn )
        FrmSetObjectPosition( form, index, 0, y );
    else
        FrmSetObjectPosition( form, index, x + 7, y );
}



/* Quick and dirty check for enough memory to send a message or copy
   a page of text to memo ( 5000 bytes for body 150 for headers and some
   more for palm ) */
void CheckMem( UInt16 alertID )
{
    MemHandle test;

    test = MemHandleNew( 7000 );
    if ( test == NULL )
        FrmAlert( alertID );
    else
        MemHandleFree( test );
}


/* Scrolls the message field */
void ScrollMessage
    (
    FieldType* field,
    Int16 lines /* lines to scroll */
    )
{
    if ( lines < 0 )
        FldScrollField( field, -lines, winUp );
    else if ( 0 < lines )
        FldScrollField( field, lines, winDown );
}


/* Updates the scrollbar and scroll arrows */
void UpdateFieldScrollbar( FieldType* field, ScrollBarType* scrollbar )
{
    UInt16 scrollPos;
    UInt16 textHeight;
    UInt16 fieldHeight;
    UInt16 maxValue;

    FldGetScrollValues( field, &scrollPos, &textHeight, &fieldHeight );
    if ( fieldHeight < textHeight )
        maxValue = textHeight - fieldHeight;
    else
        maxValue = scrollPos;
    SclSetScrollBar( scrollbar, scrollPos, 0, maxValue, 6 );
}


/* Check low power situation */
Boolean HasEnoughPower( void )
{
    UInt16 warnThreshold;
    UInt16 currentVoltage;

    currentVoltage = SysBatteryInfoV20( false, &warnThreshold,
                        NULL, NULL, NULL, NULL );

    return ( warnThreshold <= currentVoltage );
}

