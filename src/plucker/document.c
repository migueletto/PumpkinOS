/*
 * $Id: document.c,v 1.174 2004/04/18 15:34:47 prussar Exp $
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

#include "anchor.h"
#include "const.h"
#include "debug.h"
#include "genericfile.h"
#include "emailform.h"
#include "externalform.h"
#include "hires.h"
#include "history.h"
#include "image.h"
#include "os.h"
#include "paragraph.h"
#include "prefsdata.h"
#include "resourceids.h"
#include "screen.h"
#include "handera.h"
#include "uncompress.h"
#include "util.h"
#include "metadocument.h"
#include "link.h"
#include "search.h"
#include "font.h"
#include "table.h"
#include "fullscreenform.h"
#include "rotate.h"
#include "control.h"

#include "document.h"
#include "../libpit/debug.h"

/***********************************************************************
 *
 *      Internal Constants
 *
 ***********************************************************************/
#define ESTIMATE_RESOLUTION     50
#define BACKWARDS               true
#define CONTINUATION            true


/***********************************************************************
 *
 *      Local functions
 *
 ***********************************************************************/
static Boolean ViewRecordAt( UInt16 recordId, Boolean newPage,
            Int16 pOffset, Int16 cOffset, Coord yPosition, Boolean backwards,
            Boolean continuation, WriteModeType mode )
            DOCUMENT_SECTION;
static void UpdateFirstVisible( DirectionType direction ) DOCUMENT_SECTION;
static void InitViewportBounds( void ) DOCUMENT_SECTION;
static void UpdateScrollbar( MetaRecord* meta ) DOCUMENT_SECTION;
static void SetIdle( void ) DOCUMENT_SECTION;
static void ShowVerticalOffset( void ) DOCUMENT_SECTION;
static void SetFirstVisibleParagraph( const Int16 paragraph,
                const YOffset y ) DOCUMENT_SECTION;
static void InitializeTextContext( TextContext* tContext,
                const WriteModeType mode ) DOCUMENT_SECTION;
static void AdjustVerticalOffset( MemHandle recordHandle,
                YOffset adjustment ) DOCUMENT_SECTION;
static YOffset CalculatePosition( Header* record, Int16 pOffset,
                Int16 characterPosition, YOffset* verticalOffsetPtr,
                Boolean recalculateHeights, Coord yPosition,
                Boolean backwards, WriteModeType mode )
                DOCUMENT_SECTION;
static Boolean ViewPHTML( Header* record, Boolean newPage,
                Int16 pOffset, Int16 cOffset, Coord yPosition,
                Boolean backwards, WriteModeType mode )
                DOCUMENT_SECTION;
static void SaveExtents( void ) DOCUMENT_SECTION;
static Int16 SavedExtentY( void ) DOCUMENT_SECTION;
static Int16 SavedExtentX( void ) DOCUMENT_SECTION;
static Int16 SavedTopLeftY( void ) DOCUMENT_SECTION;
static Int16 SavedTopLeftX( void ) DOCUMENT_SECTION;
static void RestoreBounds( void ) DOCUMENT_SECTION;
static void ActivateSavedBounds( void ) DOCUMENT_SECTION;
static void RenderRecord( Header* record, WriteModeType mode,
            Boolean continuation, RectangleType* clipRect ) DOCUMENT_SECTION;
static void EstimateHeights( void ) DOCUMENT_SECTION;


/***********************************************************************
 *
 *      Private variables
 *
 ***********************************************************************/
static Int32            charToHeight;
static YOffset          estimatedHeight[ 2 ];
static YOffset          minVerticalOffset;
static MemHandle        currentRecord;
static MemHandle        metaRecord;
static RectangleType    viewportBounds;
static RectangleType    savedExtents;
static RectangleType    savedViewportBounds;
static RectangleType    oldViewportBounds;
static Char             offsetFormat[ 5 ];
static Int32            scrollbarScale;
static YOffset          trueMaxScrollbar;
static Int16            scaledMaxScrollbar;
static UInt16           currentRecordId = NO_RECORD;
#ifdef HAVE_ROTATE
static DirectionType    scrollbarDirection;
#endif
#ifdef SUPPORT_WORD_LOOKUP
static Coord            findPixelX = -1;
static Coord            findPixelY = -1;
#endif

#ifdef ENABLE_SCROLL_TO_BOTTOM
static Boolean          leftOff = false;
static RectangleType    leftOffBounds;
#endif

static Int16 (*OldTopLeftX)( void );
static Int16 (*OldTopLeftY)( void );
static Int16 (*OldExtentX)( void );
static Int16 (*OldExtentY)( void );
static UInt16 oldCoordSys;

static Boolean haveLastUncompressedRecordId;
static UInt16  lastUncompressedRecordId;



/* Call whenever the last uncompressed PHTML record might be overwritten */
void ResetLastUncompressedPHTML( void )
{
    haveLastUncompressedRecordId = false;
}



/* Initialize format of percentage indicator */
void InitializeOffsetFormat
    (
    const Char* lang
    )
{
    /* in some languages they say %10 instead of 10%, so we make the
       format configurable */
    if ( STREQ( lang, "tr" ) )
        StrCopy( offsetFormat, "%%%d" );
    else
        StrCopy( offsetFormat, "%d%%" );
}



/* Initialize boundaries for viewport */
static void InitViewportBounds( void )
{
    viewportBounds.topLeft.x    = RotTopLeftX();
    viewportBounds.topLeft.y    = RotTopLeftY();
    viewportBounds.extent.x     = RotExtentX();
    viewportBounds.extent.y     = RotExtentY();
}



#ifdef HAVE_ROTATE
/* Set whether the scrollbar is to run forwards (DIRECTION_DOWN) or 
   backwards (DIRECTION_UP) */
void SetScrollbarDirection
      (
      DirectionType direction
      )
{
    scrollbarDirection = direction;
}
#endif



/* Update the scrollbar.  We need to scale it because document heights
   can be 32-bits long now. */
static void UpdateScrollbar
    (
    MetaRecord* meta  /* metarecord containing height of document */
    )
{
    YOffset position;
    UInt16  prevCoordSys;
    FormType* mainForm;

    mainForm = FrmGetFormPtr( GetMainFormId() );

    if ( Prefs()->scrollbar == SCROLLBAR_NONE )
        return;

    prevCoordSys   = PalmSetCoordinateSystem( STANDARD );

    if ( viewportBounds.extent.y < GetSequenceHeight() ) {
        FrmShowObject( mainForm, FrmGetObjectIndex( mainForm, 
            frmMainScrollBar ) );

        trueMaxScrollbar   = GetSequenceHeight() - viewportBounds.extent.y;
        position           = GetSequenceOffset();
        scrollbarScale     = 1 + ( trueMaxScrollbar / 32000 );
        scaledMaxScrollbar = trueMaxScrollbar / scrollbarScale;

        SclDrawScrollBar( GetObjectPtr( frmMainScrollBar ) );

#ifdef HAVE_ROTATE
        if ( scrollbarDirection == DIRECTION_UP )
            SclSetScrollBar( GetObjectPtr( frmMainScrollBar ),
                -position / scrollbarScale, -scaledMaxScrollbar, 0,
              RotGetScrollValue() / scrollbarScale );
        else
#endif
            SclSetScrollBar( GetObjectPtr( frmMainScrollBar ),
                position / scrollbarScale, 0, scaledMaxScrollbar,
              RotGetScrollValue() / scrollbarScale );

    }
    else {
        FrmHideObject( mainForm, FrmGetObjectIndex( mainForm, 
            frmMainScrollBar ) );
    }

    PalmSetCoordinateSystem( prevCoordSys );
}



/* Scale scrollbar data */
YOffset ScrollbarScale( Int16 value )
{
#ifdef HAVE_ROTATE
    if ( scrollbarDirection == DIRECTION_UP )
        value = -value;
#endif
    if ( value == scaledMaxScrollbar )
        return trueMaxScrollbar;
    else
        return (YOffset)value * scrollbarScale;
}



/* Update first visible paragraph data */
static void UpdateFirstVisible
    (
    DirectionType direction
    )
{
    /* FIXME: speed up direction==DIRECTION_UP handling */
    MetaParagraph*  metaParagraph;
    YOffset     position;
    Int16       i;
    MetaRecord* meta;
    Header*     record;
    Int16       numOfParagraphs;

    record   = MemHandleLock( currentRecord );
    numOfParagraphs = record->paragraphs;
    MemHandleUnlock( currentRecord );

    meta          = MemHandleLock( metaRecord );
    metaParagraph = ( MetaParagraph* ) ( (UInt8*) meta +
                                         sizeof( MetaRecord ) );
    position      = meta->verticalOffset;

    if ( direction == DIRECTION_DOWN ) {
        i              = meta->firstVisibleParagraph;
        position      += meta->firstParagraphY;
    }
    else
        i              = 0;

    for ( ; i < numOfParagraphs ; i++ ) {
        position += metaParagraph[ i ].height;
        if ( 0 <= position )
            break;
    }

    if ( i == numOfParagraphs && 0 < i )
        i--;
    SetFirstVisibleParagraph( i, position - meta->verticalOffset -
        metaParagraph[ i ].height );

    MemHandleUnlock( metaRecord );
}



/* Indicate that the application is working */
void SetWorking( void )
{
    FormType* mainForm;
    UInt16    prevCoordSys;

    if ( Prefs()->toolbar == TOOLBAR_NONE )
        return;

    mainForm = FrmGetFormPtr( GetMainFormId() );
    prevCoordSys = PalmSetCoordinateSystem( STANDARD );
    if ( Prefs()->scrollbar != SCROLLBAR_NONE )
        FrmHideObject( mainForm, FrmGetObjectIndex( mainForm,
                                     frmMainScrollBar ) );

    if ( Prefs()->toolbar == TOOLBAR_SILK ) {
        /* FIXME: figure this out */
    }
    else {
        FrmHideObject( mainForm, FrmGetObjectIndex( mainForm, bmpLeft ) );
        FrmHideObject( mainForm, FrmGetObjectIndex( mainForm, bmpRight ) );
        FrmHideObject( mainForm, FrmGetObjectIndex( mainForm, bmpHome ) );
#ifdef SUPPORT_WORD_LOOKUP
        FrmHideObject( mainForm, FrmGetObjectIndex( mainForm, bmpLookup ) );
#endif

        FrmShowObject( mainForm, FrmGetObjectIndex( mainForm, bmpWait ) );
    }
    PalmSetCoordinateSystem( prevCoordSys );
}



/* Indicate that the application is idle */
static void SetIdle( void )
{
    FormType* mainForm;
    UInt16    prevCoordSys;

    if ( Prefs()->toolbar == TOOLBAR_NONE )
        return;

    mainForm = FrmGetFormPtr( GetMainFormId() );
    prevCoordSys = PalmSetCoordinateSystem( STANDARD );
    if ( Prefs()->scrollbar != SCROLLBAR_NONE )
        FrmShowObject( mainForm, FrmGetObjectIndex( mainForm,
                                     frmMainScrollBar ) );

    if ( Prefs()->toolbar == TOOLBAR_SILK ) {
        /* FIXME: figure this out */
    }
    else {
        FrmHideObject( mainForm, FrmGetObjectIndex( mainForm, bmpWait ) );
#ifdef SUPPORT_WORD_LOOKUP
        FrmHideObject( mainForm, FrmGetObjectIndex( mainForm, bmpLookup ) );
#endif

        FrmShowObject( mainForm, FrmGetObjectIndex( mainForm, bmpLeft ) );
        FrmShowObject( mainForm, FrmGetObjectIndex( mainForm, bmpRight ) );
        FrmShowObject( mainForm, FrmGetObjectIndex( mainForm, bmpHome ) );
    }
    PalmSetCoordinateSystem( prevCoordSys );
}



/* Update percentage value for the vertical offset */
void UpdateVerticalOffset
    (
    const UInt16 percent
    )
{
    static Char offText[ 5 ];

    StrPrintF( offText, offsetFormat, percent );

    if ( Prefs()->toolbar == TOOLBAR_SILK ) {
        HanderaUpdateSilkVerticalOffset( offText );
    }
    else if ( Prefs()->toolbar != TOOLBAR_NONE ) {
        UInt16 prevCoordSys;

        prevCoordSys = PalmSetCoordinateSystem( STANDARD );
        CtlSetLabel( GetObjectPtr( frmMainPercentPopup ), offText );
        PalmSetCoordinateSystem( prevCoordSys );
    }
}



/* Show vertical offset for record */
static void ShowVerticalOffset( void )
{
    Int32       percent;
    MetaRecord* meta;
    Int32       totalHeight;

    if ( Prefs()->toolbar == TOOLBAR_NONE )
        return;

    meta = MemHandleLock( metaRecord );

    percent = 0;
    if ( 0 < meta->height ) {
        percent  = GetSequenceOffset() + RotExtentY();
        totalHeight = GetSequenceHeight();
        if ( 100000l < totalHeight ) {
            percent /= ( totalHeight / 100 );
        }
        else
            percent = percent * 100 / totalHeight;

        if ( 100 < percent )
            percent = 100;
        if ( percent < 0 )
            percent = 0;
    }
    MemHandleUnlock( metaRecord );

    UpdateVerticalOffset( percent );
}



/* Store information about the first visible paragraph */
static void SetFirstVisibleParagraph
    (
    const Int16 paragraph,  /* paragraph's index */
    const YOffset y           /* paragraph's y coordinate within record */
    )
{
    MetaRecord* meta;

    meta = MemHandleLock( metaRecord );

    DmWrite( meta, OFFSETOF( MetaRecord, firstVisibleParagraph ), &paragraph,
        sizeof( Int16 ) );

    DmWrite( meta, OFFSETOF( MetaRecord, firstParagraphY ), &y,
        sizeof( YOffset ) );

    MemHandleUnlock( metaRecord );
    SetHistoryFirstParagraph( paragraph, y );
    SetHistoryRecordId( currentRecordId );
}



/* Initialize text context */
static void InitializeTextContext
    (
    TextContext*        tContext,  /* pointer to text context */
    const WriteModeType mode
    )
{
    tContext->cursorX       = viewportBounds.topLeft.x;
    tContext->cursorY       = viewportBounds.topLeft.y;
    tContext->writeMode     = mode;
    tContext->activeAnchor  = false;
    tContext->findCharacterPosition = -1;
}



/* Draw the record at most until the end of the visible screen */
static void RenderRecord
    (
    Header* record,  /* pointer to record header */
    WriteModeType mode,
    Boolean continuation,/* are we continuing a different record
                            on the same screen? */
    RectangleType* clipRect      /* NULL if default */
    )
{
    Paragraph*      paragraph;
    Paragraph*      firstParagraph;
    MetaParagraph*  metaParagraph;
    UInt16          numOfParagraphs;
    Boolean         needNewParagraph;
    YOffset         firstVisiblePixel;
    YOffset         lastVisiblePixel;
    YOffset         cursor;
    YOffset         lastCursor;
    Int16           paragraph_index;
    Int16           paragraphNum;
    TextContext     thisContext;
    MetaRecord*     meta;

    select_utf8(1);

    meta = MemHandleLock( metaRecord );

    SaveExtents();

    InitializeTextContext( &thisContext, mode );
#ifdef SUPPORT_WORD_LOOKUP
    if ( mode == WRITEMODE_FIND_SELECTED_WORD ) {
        thisContext.findPixelX = findPixelX;
        thisContext.findPixelY = findPixelY;
    }
#endif

    paragraph_index     = meta->firstVisibleParagraph;
    numOfParagraphs     = record->paragraphs - paragraph_index;

    paragraph           = GET_PARAGRAPH( record, paragraph_index );
    firstParagraph      = GET_PARAGRAPH( record, 0 );
    metaParagraph       = GET_METAPARAGRAPH( meta, paragraph_index );

    firstVisiblePixel   = -meta->verticalOffset;
    lastVisiblePixel    = viewportBounds.extent.y - meta->verticalOffset;

    cursor              = meta->firstParagraphY;
    thisContext.cursorY = cursor + meta->verticalOffset +
                          viewportBounds.topLeft.y;
    thisContext.characterPosition = -1;

    needNewParagraph    = false;

    if ( ! continuation && DO_ANCHORS( mode ) )
        AnchorListInit();

    /* If depth > 1, then save colors' state for a later restore */
    if ( 1 < Prefs()->screenDepth )
        SaveDrawState();

    if ( mode == WRITEMODE_DRAW_CHAR ) {
        if ( clipRect == NULL )
            clipRect = &viewportBounds;
        if ( ( clipRect->extent.y <= 0 || clipRect->extent.x <= 0 ) ) {
            mode = WRITEMODE_LAYOUT_ANCHORS;
        }
        else {
            if ( IsHiResTypeSony( HiResType() ) )
                /* There is some problem with hi-res clipping on Sony units */
                RotSetClip( &viewportBounds );
            else
                RotSetClip( clipRect );
            RotCharClipRange( clipRect->topLeft.y, clipRect->extent.y );
        }
    }    

    while ( numOfParagraphs-- ) {
        lastCursor = cursor;

        paragraphNum = paragraph - firstParagraph;

        if ( paragraphNum < GetLineCacheNeedToRenderFrom()
#ifdef SUPPORT_WORD_LOOKUP
             || ( mode == WRITEMODE_FIND_SELECTED_WORD &&
                  ( thisContext.findPixelY < thisContext.cursorY ||
                    thisContext.cursorY + metaParagraph->height <=
                        thisContext.findPixelY ) )
#endif
           ) {
            thisContext.cursorY += metaParagraph->height;
        }
        else {
            DrawParagraph( &thisContext, paragraph, record );
#ifdef SUPPORT_WORD_LOOKUP
            if ( mode == WRITEMODE_FIND_SELECTED_WORD && SelectedWordDone() )
                break;
#endif
        }

        cursor += metaParagraph->height;

        /*
            Save pointers to the first and last visible paragraphs. It is
            possible that this is the same paragraph, so be careful.

            Also note that the very first paragraph in the record is
            the initial "firstParagraph", so we don't need code to
            deal with that special case.
        */
        if ( cursor < firstVisiblePixel )
            needNewParagraph = true;
        else if ( needNewParagraph ) {
            SetFirstVisibleParagraph( paragraph_index, lastCursor );
            needNewParagraph = false;
        }
        if ( lastVisiblePixel < cursor ) {
            break;
        }
        paragraph++;
        metaParagraph++;
        paragraph_index++;
    }

    if ( 0 <= thisContext.characterPosition && DO_LAYOUT( mode ) ) {
        DmWrite( meta, OFFSETOF( MetaRecord, characterPosition ),
            &thisContext.characterPosition, sizeof( Int16 ) );
        SetHistoryCharacterPosition( thisContext.characterPosition );
        SetHistoryRecordId( currentRecordId );
    }

    MemHandleUnlock( metaRecord );

    /* Finished drawing page, restore device's colors */
    if ( 1 < Prefs()->screenDepth )
        RestoreDrawState();

    if ( mode == WRITEMODE_DRAW_CHAR )
        WinResetClip();
    RotCharClearClipRange();

    select_utf8(0);
}



/* Adjust vertical offset of record */
static void AdjustVerticalOffset
    (
    MemHandle recordHandle,   /* handle to record */
    YOffset   adjustment      /* adjustment in pixels */
    )
{
    RectangleType   vacatedRectangle;
    RectangleType   clipRectangle;
    MetaRecord      newRecord;
    YOffset         adjustAmount;
    MetaRecord*     meta;
    UInt16          nextRecordId;
    Boolean         backwards;
    Coord           yPosition;
    Boolean         continuation;
    Header*         record;

    nextRecordId = NO_RECORD;
    yPosition    = 0;
    backwards    = false;

    meta = MemHandleLock( metaRecord );

    /* Scrolling of Tbmp records is handled by another function... */
    if ( meta->height == 0 ) {
        MemHandleUnlock( metaRecord );
        return;
    }

#ifdef ENABLE_SCROLL_TO_BOTTOM
    if ( leftOff )
        RotInvertRectangle( &leftOffBounds, 0 );
    leftOff = false;
#endif

    continuation = false;
    /* Adjust the record */
    newRecord = *meta;

    /** FIX ME: The following code assumes that no non-final fragment can be 
        less than one screenful long. **/
    /* bottom of page */
    if ( newRecord.verticalOffset + adjustment < minVerticalOffset ) {
        if ( adjustment < 0 ) {
            /* at the bottom of the page (and beyond) */
            nextRecordId = GetSequentialRecordId( DIRECTION_DOWN );
            backwards = false;
            if ( nextRecordId != NO_RECORD ) {
                yPosition = RotExtentY() - ( minVerticalOffset -
                                ( newRecord.verticalOffset + adjustment ) );
            }
            else {
                /* at the bottom of the page it is only possible to
                   scroll up */
                if ( newRecord.verticalOffset <= minVerticalOffset ) {
                    adjustment = 0;
                }
#ifdef ENABLE_SCROLL_TO_BOTTOM
                else {
                    adjustment  = minVerticalOffset - newRecord.verticalOffset;
                    if ( 0 <= minVerticalOffset )
                        adjustment = -newRecord.verticalOffset;
                    leftOff     = true;
                }
#endif
            }
        }
        else {
            continuation = true;
        }
    }
    if ( nextRecordId == NO_RECORD &&
         0 < newRecord.verticalOffset + adjustment ) {
        /* top of page or beyond */
        if ( 0 < adjustment ) {
            nextRecordId = GetSequentialRecordId( DIRECTION_UP );
            backwards = true;
            if ( nextRecordId != NO_RECORD ) {
                yPosition = newRecord.verticalOffset + adjustment;
            }
            else {
                /* if we're already at the top of the page
                   we can't scroll any further */
                if ( 0 <= newRecord.verticalOffset ) {
                    adjustment = 0;
                }
                else {
                    /* scroll to the top */
                    adjustment = -newRecord.verticalOffset;
                }
            }
        }
        else {
            continuation = true;
        }
    }
    if ( adjustment == 0 ) {
        MemHandleUnlock( metaRecord );
        return;
    }
    adjustAmount                = Abs( adjustment );
    newRecord.verticalOffset   += adjustment;
    SetHistoryVerticalOffset( newRecord.verticalOffset );
    SetHistoryRecordId( currentRecordId );

    DmWrite( meta, OFFSETOF( MetaRecord, verticalOffset ),
        &newRecord.verticalOffset, sizeof( YOffset ) );

    RotSetClip( &viewportBounds );

    RotScrollRectangle( &viewportBounds, ( adjustment < 0 ) ? winUp : winDown,
        adjustAmount, &vacatedRectangle );

    RctGetIntersection( &viewportBounds, &vacatedRectangle, &clipRectangle );

    RotEraseRectangle( &clipRectangle, 0 );

    /* Adjust the on-screen anchors. Be careful to do this BEFORE drawing
        the new anchors! */
    AdjustVisibleAnchors( adjustment );

    /* This code also draws the new anchors */
    UpdateFirstVisible( adjustment < 0 ? DIRECTION_DOWN : DIRECTION_UP );

    record = MemHandleLock( recordHandle );

    RenderRecord( record, WRITEMODE_DRAW_CHAR, continuation, &clipRectangle );

    MemHandleUnlock( recordHandle );

    UpdateScrollbar( &newRecord );

    MemHandleUnlock( metaRecord );

    ShowVerticalOffset();

#ifdef ENABLE_SCROLL_TO_BOTTOM
    if ( leftOff ) {
        leftOffBounds.topLeft.x  = vacatedRectangle.topLeft.x;
        leftOffBounds.topLeft.y  = vacatedRectangle.topLeft.y;
        leftOffBounds.extent.x   = 5;
        leftOffBounds.extent.y   = 5;

        RotInvertRectangle( &leftOffBounds, 0 );
    }
#endif

    if ( nextRecordId != NO_RECORD ) {
        ViewRecordAt( nextRecordId, FROM_HISTORY, 0, 0, yPosition, backwards,
            CONTINUATION, WRITEMODE_DRAW_CHAR );
    }
}




/* Calculate the total height of the record and store it in the record header if
   needed, or at least seek to the right place. */
static YOffset CalculatePosition
    (
    Header*  record,     /* pointer to record header */
    Int16    pOffset,    /* offset to first paragraph */
    Int16    characterPosition, /* position that display should start with */
    YOffset* verticalOffsetPtr, /* pointer to vertical offset, and if */
                                /* NO_VERTICAL_OFFSET to be adjusted to match
                                   characterPosition */
    Boolean  recalculateHeights,/* do we need to recalculate the heights? */
    Coord    yPosition,         /* start drawing here */
    Boolean  backwards,         /* if backwards is set, yPosition indicates
                                   where the end of the record is to be */
    WriteModeType mode          /* do we do any actual drawing? */
    )
{
    Paragraph*  paragraph;
    TextContext thisContext;
    UInt16      numOfParagraphs;
    YOffset     undrawnHeight;
    Int16       sizeSoFar;
    YOffset     offsetInParagraph;
    Int16       i;
    YOffset     drawnHeight;
    MetaParagraph* metaParagraph;
    MetaRecord* meta;

    /* If depth > 1, then save colors' state for a later restore */
    if ( 1 < Prefs()->screenDepth )
        SaveDrawState();

    meta          = MemHandleLock( metaRecord );
    metaParagraph = GET_METAPARAGRAPH( meta, 0 );


    InitializeTextContext( &thisContext, mode );

    numOfParagraphs = record->paragraphs;
    drawnHeight     = 0;

    if ( 0 == numOfParagraphs ) {
        MemHandleUnlock( metaRecord );
        if ( 1 < Prefs()->screenDepth )
            RestoreDrawState();
        return 0;
    }

    InitViewportBounds();
    RotSetClip( &viewportBounds );
    RotCharClipRange( viewportBounds.topLeft.y, viewportBounds.extent.y );

    if ( ! backwards ) {
        if ( pOffset == NO_OFFSET && 0 < characterPosition ) {
            i         = 0;
            paragraph = GET_PARAGRAPH( record, 0 );
            sizeSoFar = 0;
            for ( i = 0 ; i < numOfParagraphs ; i++ ) {
                sizeSoFar += paragraph->size;
                if ( characterPosition < sizeSoFar )
                    break;
                paragraph++;
            }
            if ( i == numOfParagraphs ) {
                i                 = 0;
                characterPosition = 0;
                paragraph         = GET_PARAGRAPH( record, 0 );
            }
            thisContext.cursorY               = yPosition;
            thisContext.findCharacterPosition = characterPosition;
            thisContext.foundYPosition        = -1;
            thisContext.writeMode             = WRITEMODE_NO_DRAW;
            DrawParagraph( &thisContext, paragraph, record );
            if ( 0 <= thisContext.foundYPosition ) {
                offsetInParagraph = yPosition - thisContext.foundYPosition;
                pOffset           = i;
            }
            else {
                offsetInParagraph = 0;
                pOffset           = 0;
            }
        }
        else if ( pOffset == NO_OFFSET ) {
            pOffset           = 0;
            offsetInParagraph = 0;
        }
        else
            offsetInParagraph = 0;

        paragraph     = GET_PARAGRAPH( record, pOffset );

        InitializeTextContext( &thisContext, mode );
        thisContext.cursorY += yPosition + offsetInParagraph;

        for ( i = pOffset ; i < numOfParagraphs ; i++ ) {
            YOffset paragraphHeight;
            YOffset lastCursor;

            lastCursor = thisContext.cursorY;

            if ( ! recalculateHeights &&
                viewportBounds.extent.y + viewportBounds.topLeft.y < lastCursor
              )
                break;

            DrawParagraph( &thisContext, paragraph, record );

            paragraphHeight  = thisContext.cursorY - lastCursor;
            drawnHeight     += paragraphHeight;

            if ( recalculateHeights )
                DmWrite( meta, sizeof( MetaRecord ) +
                    i * sizeof( MetaParagraph ), &paragraphHeight,
                    sizeof( YOffset ) );

            paragraph++;

        }
    }
    else {
        /* render backwards */
        YOffset currentY;
        YOffset paragraphHeight;

        paragraph   = GET_PARAGRAPH( record, numOfParagraphs - 1 );
        currentY    = RotTopLeftY() + yPosition;

        for ( i = numOfParagraphs - 1;
              0 <= i && RotTopLeftY() < currentY;
              i-- ) {
            if ( recalculateHeights ) {
                InitializeTextContext( &thisContext, WRITEMODE_NO_DRAW );
                DrawParagraph( &thisContext, paragraph, record );
                paragraphHeight = thisContext.cursorY - RotTopLeftY();
                DmWrite( meta, sizeof( MetaRecord ) +
                    i * sizeof( MetaParagraph ), &paragraphHeight,
                    sizeof( YOffset ) );
            }
            else {
                paragraphHeight = metaParagraph[ i ].height;
            }
            currentY -= paragraphHeight;
            InitializeTextContext( &thisContext, mode );

            thisContext.cursorY = currentY;
            DrawParagraph( &thisContext, paragraph, record );
            drawnHeight += paragraphHeight;
            paragraph--;
        }
        pOffset           = i + 1;
        offsetInParagraph = currentY - RotTopLeftY();
        yPosition         = 0;
    }

    WinResetClip();
    RotCharClearClipRange();

    InitializeTextContext( &thisContext, WRITEMODE_NO_DRAW );

    paragraph             = GET_PARAGRAPH( record, 0 );

    undrawnHeight = 0;

    for ( i = 0 ; i < pOffset ; i++ ) {

        if ( recalculateHeights ) {
            thisContext.cursorY = 0;
            DrawParagraph( &thisContext, paragraph, record );
            undrawnHeight += thisContext.cursorY;
            DmWrite( meta, sizeof( MetaRecord ) +
                i * sizeof( MetaParagraph ), &thisContext.cursorY,
                sizeof( YOffset ) );
        }
        else
            undrawnHeight += metaParagraph[ i ].height;

        paragraph++;
    }
    *verticalOffsetPtr = offsetInParagraph - undrawnHeight + yPosition;

    DmWrite( meta, OFFSETOF( MetaRecord, verticalOffset ), verticalOffsetPtr,
        sizeof( YOffset ) );
    MemHandleUnlock( metaRecord );

    SetFirstVisibleParagraph( pOffset, undrawnHeight );
    SetHistoryVerticalOffset( *verticalOffsetPtr );
    SetHistoryRecordId( currentRecordId );

    /* If depth > 1, then save colors' state for a later restore */
    if ( 1 < Prefs()->screenDepth )
        RestoreDrawState();

    if ( recalculateHeights )
        return undrawnHeight + drawnHeight;
    else
        return meta->height;
}




/* View Plucker HTML record */
static Boolean ViewPHTML
    (
    Header* record,     /* pointer to record header */
    Boolean newPage,    /* true if the page is not from the history */
    Int16   pOffset,    /* offset to first paragraph */
    Int16   cOffset,    /* character offset */
    Coord   yPosition,  /* position on screen */
    Boolean backwards,  /* make record fit before yPosition? */
    WriteModeType mode  /* mode to write in */
    )
    /* THROWS */
{
    UInt16      prevCoordSys;
    MetaRecord* meta;
    MetaRecord  recordData;
    Boolean     didDraw;
    Boolean     lineCache;

    lineCache = IsLineCacheActive();
    LineCacheDeactivate();

    /* Re-validate coordinate system as NATIVE even though we just did in
       other functions. After viewing a fullscreen image, we seem to
       mysteriously get pushed into STANDARD mode by this point. Resetting
       it to NATIVE ensures everything displays properly. */
    prevCoordSys = PalmSetCoordinateSystem( NATIVE );

    if ( cOffset != NO_OFFSET )
        pOffset = NO_OFFSET;

    didDraw = false;

    if ( mode == WRITEMODE_DRAW_CHAR )
        SetWorking();

    if ( record->type == DATATYPE_PHTML_COMPRESSED &&
         ( ! haveLastUncompressedRecordId ||
           lastUncompressedRecordId != record->uid ) ) {
        haveLastUncompressedRecordId = true;
        lastUncompressedRecordId     = record->uid;
        Uncompress( record );
    }

    meta       = MemHandleLock( metaRecord );
    recordData = *meta;

    if ( recordData.verticalOffset == NO_VERTICAL_OFFSET )
        recordData.height = 0;
    if ( newPage || recordData.height == 0 ) {
        recordData.verticalOffset = yPosition;

        DmWrite( meta, OFFSETOF( MetaRecord, verticalOffset ),
            &recordData.verticalOffset, sizeof( YOffset ) );

        SetHistoryVerticalOffset( recordData.verticalOffset );
        SetHistoryRecordId( currentRecordId );
        SetFirstVisibleParagraph( 0, 0 );
    }
    if ( recordData.height == 0 ) {
        if ( yPosition == 0 && DO_ANCHORS( mode ) )
            AnchorListInit();

        if ( cOffset == NO_OFFSET )
            recordData.height = CalculatePosition( record, pOffset,
                recordData.characterPosition, &recordData.verticalOffset,
                true, yPosition, backwards, mode );
        else
            recordData.height = CalculatePosition( record, pOffset,
                cOffset, &recordData.verticalOffset, true, yPosition,
                backwards, mode );

        didDraw = true;

        if ( yPosition == 0 && DO_ANCHORS( mode ) )
            AnchorListRelease();

        DmWrite( meta, OFFSETOF( MetaRecord, height ), &recordData.height,
            sizeof( YOffset ) );
    }
    if ( ( backwards || cOffset != NO_OFFSET || pOffset != NO_OFFSET ) &&
         ! didDraw ) {
        CalculatePosition( record, pOffset, cOffset, &recordData.verticalOffset,
            false, yPosition, backwards, mode );
        DmWrite( meta, OFFSETOF( MetaRecord, verticalOffset ),
            &recordData.verticalOffset, sizeof( YOffset ) );
        didDraw = true;
    }
    /* If needed, re-adjust the position so we don't display past the end of
       the record. */
#ifdef ENABLE_SCROLL_TO_BOTTOM
    if ( yPosition == 0 && ! backwards &&
         recordData.height < -recordData.verticalOffset + RotExtentY() &&
         NO_RECORD == GetSequentialRecordId( DIRECTION_DOWN )
         ) {
        recordData.verticalOffset = RotExtentY() - recordData.height;
        if ( 0 < recordData.verticalOffset )
            recordData.verticalOffset = 0;
        DmWrite( meta, OFFSETOF( MetaRecord, verticalOffset ),
            &recordData.verticalOffset, sizeof( YOffset ) );
        SetHistoryVerticalOffset( recordData.verticalOffset );
        SetHistoryRecordId( currentRecordId );
        if ( didDraw && mode == WRITEMODE_DRAW_CHAR )
            RotEraseRectangle( &viewportBounds, 0 );
    }
#endif
    UpdateFirstVisible( DIRECTION_NONE );

    minVerticalOffset = viewportBounds.extent.y - recordData.height;

    MemHandleUnlock( metaRecord );

    if ( lineCache )
        LineCacheActivate();
    RenderRecord( record, mode, yPosition != 0, NULL );

    if ( mode == WRITEMODE_DRAW_CHAR )
        SetIdle();

    PalmSetCoordinateSystem( prevCoordSys );

    return true;
}



/* Move page up/down */
void DoPageMove
    (
    const YOffset pixels  /* number of pixels to move ( positive values move
                             up, negative down ) */
    )
{
    UInt16    prevCoordSys;
    Header*   record;
    UInt16    recordId;
    Int16     cOffset;
    YOffset   abspixels;
    Boolean   useFullScreen;

    UnselectActualAnchor();

    prevCoordSys = PalmSetCoordinateSystem( NATIVE );

    abspixels = ( pixels < 0 ) ? -pixels : pixels;

    if ( RotExtentY() < abspixels ) {
        recordId  = DeltaToRecordId( pixels, &cOffset );
        if ( recordId != currentRecordId ) {
            PalmSetCoordinateSystem( prevCoordSys );
            ViewRecord( recordId, false, NO_OFFSET, cOffset, WRITEMODE_DRAW_CHAR );
            return;
        }
    }

    InitViewportBounds();

    if ( pixels < 0 )
        LineCacheScrollUp( abspixels );
    else
        LineCacheClear( );

    /* if image record or table, call fullscreenform's adjustment */

    record = MemHandleLock( currentRecord );

    useFullScreen = ( record->type == DATATYPE_TBMP ||
         record->type == DATATYPE_TBMP_COMPRESSED ||
         record->type == DATATYPE_TABLE ||
         record->type == DATATYPE_TABLE_COMPRESSED );

    MemHandleUnlock( currentRecord );

    if ( useFullScreen )
        FsAdjustVerticalOffset( pixels );
    else
        AdjustVerticalOffset( currentRecord, pixels );

    PalmSetCoordinateSystem( prevCoordSys );
}



/* Return a handle to the meta data */
MemHandle GetMetaHandle
    (
    const UInt16 recordId,   /* record ID */
    Boolean      update      /* if true, update meta data */
    )
    /* THROWS */
{
    MemHandle   handle;
    MemHandle   metaHandle;
    Header*     record;
    UInt16      numOfParagraphs;

    handle          = GetRecordHandle( recordId );
    record          = MemHandleLock( handle );
    numOfParagraphs = record->paragraphs;
    MemHandleUnlock( handle );
    FreeRecordHandle( &handle );

    if ( update )
        ReRenderAllIfNeeded();
    metaHandle = ReturnMetaHandle( recordId, numOfParagraphs );

    return metaHandle;
}



/* Return a handle to a record */
MemHandle GetRecordHandle
    (
    const UInt16 recordId  /* record ID */
    )
    /* THROWS */
{
    MemHandle record;

    record = ReturnRecordHandle( recordId );
    if ( record != NULL )
        return record;
    else
        return ReturnRecordHandle( PLUCKER_LINKS_ID );
}



/* View record */
Boolean ViewRecord
    (
    UInt16          recordId,   /* record ID */
    Boolean         newPage,    /* indicates if the page is not from the
                                   history */
    Int16           pOffset,    /* offset to first paragraph */
    Int16           cOffset,    /* offset to first character */
    WriteModeType   mode
    )
{
    UInt16  prevCoordSys;
    Boolean retValue;

    currentRecordId = NO_RECORD;

    prevCoordSys = PalmSetCoordinateSystem( NATIVE );

    InitViewportBounds();
    WinResetClip();  /* FIXME: This shouldn't really be needed! */
    if ( mode == WRITEMODE_DRAW_CHAR )
        RotEraseRectangle( &viewportBounds, 0 );

    retValue = ViewRecordAt( recordId, newPage, pOffset, cOffset, 0,
               ! BACKWARDS, ! CONTINUATION, mode );

    PalmSetCoordinateSystem( prevCoordSys );

    return retValue;
}



/* View record ( or image ) */
static Boolean ViewRecordAt
    (
    UInt16          recordId,   /* record ID */
    Boolean         newPage,    /* indicates if the page is from the
                                   history or not */
    Int16           pOffset,    /* offset to first paragraph */
    Int16           cOffset,    /* offset to first character */
    Coord           yPosition,  /* position on screen */
    Boolean         backwards,  /* if true, yPosition marks end of record */
    Boolean         continuation,/* is a record on a screen that already
                                    has some materials? */
    WriteModeType   mode        /* how do we display it? */
    )
{
    UInt16  prevCoordSys;
    Boolean success;
    Header* record;
    UInt16  nextRecordId;
    static  Boolean haveLastRecordId;
    static  UInt16  lastRecordId;
    Boolean textRecord;
    UInt16  searchFirstRecordId;
    UInt16  searchLastRecordId;

    select_utf8(1);

    if ( Prefs()->rotate != GetHistoryPtr()->rotate )
        ReRenderAllIfNeeded();

    textRecord = false;

    prevCoordSys = PalmSetCoordinateSystem( NATIVE );

    nextRecordId = recordId;

    GetLastSearchedSequenceBoundaryRecordIds( &searchFirstRecordId,
        &searchLastRecordId );

    if ( recordId < searchFirstRecordId ||
         searchLastRecordId < recordId ) {
        SetSearchPosition( 0 );
        SetLastSearchedRecordId( recordId );
    }

    do {
        recordId     = nextRecordId;
        nextRecordId = NO_RECORD;

        MSG( _( "record #%d\n", recordId ) );

        if ( ! continuation ) {
            if ( DO_ANCHORS( mode ) )
                AnchorListRelease();
            if ( mode == WRITEMODE_DRAW_CHAR ) {
                ReleaseFsTableHandle();
                ReleaseFsImageHandle();
            }
        }

        if ( currentRecordId != recordId ) {
            FreeRecordHandle( &currentRecord );

            ErrTry {
                currentRecord   = GetRecordHandle( recordId );
                currentRecordId = recordId;
                metaRecord      = GetMetaHandle( recordId, true );

            }
            ErrCatch( UNUSED_PARAM( err ) ) {
                SetLinkIndex( recordId );
                FrmGotoForm( frmExternalLinks );

                PalmSetCoordinateSystem( prevCoordSys );

                select_utf8(0);
                return true;
            } ErrEndCatch
        }

        if ( ! haveLastRecordId || lastRecordId != recordId ) {
            LineCacheClear();
            lastRecordId     = recordId;
            haveLastRecordId = true;
        }

        success = false;
        record  = MemHandleLock( currentRecord );
        ErrTry {
            switch ( record->type ) {
                case DATATYPE_PHTML:
                case DATATYPE_PHTML_COMPRESSED:

                    textRecord = true;

                    if ( newPage )
                        AddToHistory( recordId );
                    else
                        SetHistoryRecordId( recordId );

                    success = ViewPHTML( record, continuation || newPage,
                                pOffset, cOffset, yPosition, backwards,
                                mode );

                    if ( ( backwards || ! continuation ) &&
                         GetVerticalOffset() < 0 ) {
                        nextRecordId = GetSequentialRecordId( DIRECTION_UP );
                        if ( nextRecordId != NO_RECORD ) {
                            yPosition    = - GetVerticalOffset();
                            pOffset      = 0;
                            cOffset      = 0;
                            newPage      = false;
                            backwards    = true;
                            continuation = true;
                        }
                    }
                    if ( ! backwards &&
                         GetMinVerticalOffset() < GetVerticalOffset() ) {
                        nextRecordId = GetSequentialRecordId( DIRECTION_DOWN );
                        yPosition    = RotExtentY() - ( GetVerticalOffset() -
                                                     GetMinVerticalOffset() );
                        pOffset      = 0;
                        cOffset      = 0;
                        newPage      = false;
                        continuation = true;
                    }
                    break;

                case DATATYPE_TBMP:
                case DATATYPE_TBMP_COMPRESSED:
                case DATATYPE_MULTIIMAGE:
                    if ( newPage )
                        AddToHistory( recordId );

                    success     = LoadFullScreenImage( record, newPage );
                    if ( success )
                        FrmPopupForm( frmFullscreen );
                    break;

                case DATATYPE_TABLE:
                case DATATYPE_TABLE_COMPRESSED:
                    if ( newPage )
                        AddToHistory( recordId );

                    success     = LoadTable( record, newPage );
                    if ( success )
                        FrmPopupForm( frmFullscreen );
                    break;

                case DATATYPE_MAILTO:
                    SetMailto( recordId );
                    FrmGotoForm( frmEmail );

                    success = true;
                    break;

                case DATATYPE_LINK_INDEX:
                    SetLinkIndex( recordId );
                    FrmGotoForm( frmExternalLinks );

                    success = true;
                    break;

                default:
                    ErrThrow( dmErrCorruptDatabase );
                    break;
            }
        }
        ErrCatch( err ) {
            switch ( err ) {
            case errZLibMemError:
                success = false;
                break;

            default:
                MemHandleUnlock( currentRecord );
                ErrThrow( err );
                break;
            }
        } ErrEndCatch

        MemHandleUnlock( currentRecord );

    } while ( nextRecordId != NO_RECORD );

    PalmSetCoordinateSystem( prevCoordSys );

    if ( textRecord && mode == WRITEMODE_DRAW_CHAR ) {
        MetaRecord* meta;
        EstimateHeights();
        ShowVerticalOffset();
        meta = MemHandleLock( metaRecord );
        UpdateScrollbar( meta );
        MemHandleUnlock( metaRecord );
    }

    select_utf8(0);
    return success;
}



/* Set height of record to zero */
void ResetHeight( void )
{
    if ( metaRecord != NULL ) {
        YOffset       height;
        MetaRecord* meta;

        meta = MemHandleLock( metaRecord );

        height = 0;
        DmWrite( meta, OFFSETOF( MetaRecord, height ), &height,
            sizeof( YOffset ) );

        MemHandleUnlock( metaRecord );
    }
}



/* reset the record references (e.g. when the document is closed) */
void ResetRecordReferences( void )
{
    currentRecord   = NULL;
    metaRecord      = NULL;
}



/* Return handle to current record */
MemHandle GetCurrentRecord( void )
{
    return currentRecord;
}



/* get uid of current record */
UInt16 GetCurrentRecordId( void )
{
    return currentRecordId;
}



/* Return handle to meta record */
MemHandle GetMetaRecord( void )
{
    return metaRecord;
}



/* Return vertical offset, negated */
YOffset GetVerticalOffset( void )
{
    YOffset     verticalOffset;
    MetaRecord* meta;

    meta = MemHandleLock( metaRecord );
    verticalOffset = -meta->verticalOffset;
    MemHandleUnlock( metaRecord );

    return verticalOffset;
}



/* Return minimum vertical offset, negated */
YOffset GetMinVerticalOffset( void )
{
    return -minVerticalOffset;
}



/* Are we on the last screen? */
Boolean OnLastScreen( void )
{
    return GetMinVerticalOffset() <= GetVerticalOffset() &&
              NO_RECORD == GetSequentialRecordId( DIRECTION_DOWN );
}



/* Are we on the first screen? */
Boolean OnFirstScreen( void )
{
    return GetVerticalOffset() <= 0 &&
               NO_RECORD == GetSequentialRecordId( DIRECTION_UP );
}



/* Get the first visible record id in a sequence */
UInt16 GetFirstVisibleRecordId( void )
{
   UInt16      lastRecordId;
   MemHandle   metaHandle;
   MetaRecord* meta;
   Boolean     done;
   UInt16      recordId;

   recordId        = currentRecordId;

   do {
       if ( recordId == currentRecordId )
           metaHandle = metaRecord;
       else
           metaHandle = GetMetaHandle( recordId, false );
       meta = MemHandleLock( metaHandle );
       done = ( meta->verticalOffset <= 0 );
       MemHandleUnlock( metaHandle );
       lastRecordId = recordId;
       if ( ! done )
           recordId = GetSequentialRecordId( DIRECTION_UP );
   } while ( ! done && recordId != NO_RECORD );

   return lastRecordId;
}



/* Wrapper for RenderRecord. Replace function set and execute */
void CopyRecord( Char* buf, Int16 bufsize )
{
    SetReRenderCheck( false );
    ActivateSavedBounds();
    SetCopyBuffer( buf, bufsize );
    StartCopyMode();
    ViewRecordAt( GetFirstVisibleRecordId(), FROM_HISTORY, NO_OFFSET,
        NO_OFFSET, 0, ! BACKWARDS, ! CONTINUATION, WRITEMODE_COPY_CHAR );
    StopCopyMode();
    RestoreBounds();
    SetReRenderCheck( true );
}



/* Wrapper for RenderRecord. Replace function set and execute. */
void TextifyRecord( Char* buf, Int16 bufsize )
{
    if ( IsFullscreenformActive() ) {
        if ( IsLargeTable() )
            StrPrintF( buf, "[TABLE %u]", currentRecordId );
        else
            StrPrintF( buf, "[IMG %u]", currentRecordId );
        return;
    }
    SetReRenderCheck( false );
    ActivateSavedBounds();
    SetCopyBuffer( buf, bufsize );
    ViewRecordAt( GetFirstVisibleRecordId(), FROM_HISTORY, NO_OFFSET,
        NO_OFFSET, 0, ! BACKWARDS, ! CONTINUATION, WRITEMODE_COPY_CHAR );
    RestoreBounds();
    SetReRenderCheck( true );
}



#ifdef SUPPORT_WORD_LOOKUP
/* Wrapper for RenderRecord. Replace function set and execute. */
void DoFindSelectedWord
    (
    Coord x,
    Coord y
    )
{
    SelectedWordReset();
    SetReRenderCheck( false );
    ActivateSavedBounds();
    RotFromScreenXY( &x, &y );
    findPixelX = x;
    findPixelY = y;
    ViewRecordAt( GetFirstVisibleRecordId(), FROM_HISTORY, NO_OFFSET,
        NO_OFFSET, 0, ! BACKWARDS, ! CONTINUATION,
        WRITEMODE_FIND_SELECTED_WORD );
    RestoreBounds();
    SetReRenderCheck( true );
}
#endif



static void SaveExtents( void )
{
    savedViewportBounds = viewportBounds;
    savedExtents.topLeft.x = TopLeftX();
    savedExtents.topLeft.y = TopLeftY();
    savedExtents.extent.x = ExtentX();
    savedExtents.extent.y = ExtentY();
}



/* use the saved viewport data to get the screen bound */
static Int16 SavedExtentY( void )
{
    return savedExtents.extent.y;
}



/* use the saved viewport data to get the screen bound */
static Int16 SavedExtentX( void )
{
    return savedExtents.extent.x;
}



/* use the saved viewport data to get the screen bound */
static Int16 SavedTopLeftY( void )
{
    return savedExtents.topLeft.y;
}



/* use the saved viewport data to get the screen bound */
static Int16 SavedTopLeftX( void )
{
    return savedExtents.topLeft.x;
}



/* set the saved-data extent/topleft functions and bounds to be active */
static void ActivateSavedBounds( void )
{
    oldCoordSys = PalmSetCoordinateSystem( NATIVE );
    OldExtentY  = ExtentY;
    OldExtentX  = ExtentX;
    OldTopLeftY = TopLeftY;
    OldTopLeftX = TopLeftX;
    oldViewportBounds = viewportBounds;
    ExtentY     = SavedExtentY;
    ExtentX     = SavedExtentX;
    TopLeftY    = SavedTopLeftY;
    TopLeftX    = SavedTopLeftX;
    viewportBounds = savedViewportBounds;
}



/* restore the default extent/topleft functions and bounds */
static void RestoreBounds( void )
{
    PalmSetCoordinateSystem( oldCoordSys );
    ExtentY = OldExtentY;
    ExtentX = OldExtentX;
    TopLeftY = OldTopLeftY;
    TopLeftX = OldTopLeftX;
    viewportBounds = oldViewportBounds;
}



/* Get the first/last record in the sequence */
UInt16 GetSequenceBoundaryRecordId
    (
    UInt16        recordId,
    DirectionType direction,
    UInt16*       recordNumPtr
    )
{
    UInt16    recordNum;
    MemHandle handle;

    handle = FindRecord( recordId, (Int16 *)&recordNum );
    if ( handle == NULL )
        return NO_RECORD;
    else
        FreeRecordHandle( &handle );
    if ( recordNumPtr != NULL )
        *recordNumPtr = recordNum;

    for (;;) {
         Header*   record;
         MemHandle handle;

         handle = GetSequentialRecordHandle( &recordNum, direction );
         if ( handle != NULL ) {
             record   = MemHandleLock( handle );
             recordId = record->uid;
             MemHandleUnlock( handle );
             FreeRecordHandle( &handle );
             if ( recordNumPtr != NULL )
                 *recordNumPtr = recordNum;
         }
         else {
             return recordId;
         }
    }
}




/* Get the next record in the current sequence */
MemHandle GetSequentialRecordHandle
    (
    UInt16*       recordNumPtr,
    DirectionType direction
    )
{
    MemHandle handle;
    Header*   record;
    PluckerDataType type;
    UInt16    recordNum;
    //UInt16    recordId;
    UInt8     flags;

    recordNum = *recordNumPtr;
    handle   = ReturnRecordHandleByIndex( recordNum );
    record   = MemHandleLock( handle );
    //recordId = record->uid;
    flags    = record->flags;
    MemHandleUnlock( handle );

    if ( direction == DIRECTION_DOWN && ! Prefs()->joinUpAllRecords &&
         ! ( flags & HEADER_FLAG_CONTINUATION ) ) {
        FreeRecordHandle( &handle );
        return NULL;
    }

    do {
        FreeRecordHandle( &handle );
        if ( direction == DIRECTION_UP ) {
            if ( recordNum < 2 )
                return NULL;
            recordNum--;
        }
        else
            recordNum++;
        handle = ReturnRecordHandleByIndex( recordNum );
        if ( handle == NULL )
            return NULL;
        record   = MemHandleLock( handle );
        type     = record->type;
        //recordId = record->uid;
        flags    = record->flags;
        MemHandleUnlock( handle );
        if ( ( type == DATATYPE_PHTML_COMPRESSED ||
               type == DATATYPE_PHTML ) &&
             direction == DIRECTION_UP && ! Prefs()->joinUpAllRecords &&
             ! ( flags & HEADER_FLAG_CONTINUATION ) ) {
            FreeRecordHandle( &handle );
            return NULL;
        }
    } while ( type != DATATYPE_PHTML_COMPRESSED &&
              type != DATATYPE_PHTML );
    *recordNumPtr = recordNum;
    return handle;
}



/* Get the next record ID in a natural sequence */
UInt16 GetSequentialRecordId
    (
    DirectionType direction
    )
{
    MemHandle handle;
    Header*   record;
    UInt16    recordId;
    UInt16    recordNum;

    handle = FindRecord( GetCurrentRecordId(), (Int16 *)&recordNum );
    if ( handle == NULL )
        return NO_RECORD;
    else
        FreeRecordHandle( &handle );

    handle = GetSequentialRecordHandle( &recordNum, direction );
    if ( handle == NULL )
        return NO_RECORD;

    record   = MemHandleLock( handle );
    recordId = record->uid;
    MemHandleUnlock( handle );
    FreeRecordHandle( &handle );

    return recordId;
}



/* estimate heights for the sequence containing recordId */
static void EstimateHeights( void )
{
    MemHandle handle;
    MemHandle metaRecordHandle;
    Header*   record;
    MetaRecord*     meta;
    //PluckerDataType type;
    UInt16    recordNum;
    UInt16    initialRecordNum;
    Int32     charSizeWhereHeightAvail[2];
    Int32     charSizeOther[2];
    YOffset   totalCharSizeWhereHeightAvail;
    Int32     totalHeightWhereAvail;
    YOffset   heightWhereAvail[2];
    DirectionType direction;
    Int16     i;
    UInt16    recordId;
    Int16     size;
    Int16     paragraphs;
    Boolean   done;

    handle = FindRecord( GetCurrentRecordId(), (Int16 *)&recordNum );
    if ( handle == NULL ) {
        estimatedHeight[ DIRECTION_UP ]   = 0;
        estimatedHeight[ DIRECTION_DOWN ] = 0;
    }
    else
        FreeRecordHandle( &handle );
    initialRecordNum = recordNum;

    for ( i = 0 ; i < 2 ; i++ ) {
         charSizeWhereHeightAvail[ i ] = 0;
         charSizeOther[ i ]            = 0;
         heightWhereAvail[ i ]         = 0;
    }

    direction = DIRECTION_DOWN;
    done      = false;

    do {
         handle = GetSequentialRecordHandle( &recordNum, direction );
         if ( handle == NULL ) {
             if ( direction == DIRECTION_DOWN ) {
                 recordNum = initialRecordNum;
                 direction = DIRECTION_UP;
                 continue;
             }
            else
                done = true;
         }
         else {
            record     = MemHandleLock( handle );
            //type       = record->type;
            recordId   = record->uid;
            size       = record->size;
            paragraphs = record->paragraphs;
            MemHandleUnlock( handle );
            FreeRecordHandle( &handle );
            metaRecordHandle = ReturnMetaHandle( recordId, paragraphs );
            meta = MemHandleLock( metaRecordHandle );
            if ( meta->verticalOffset != NO_VERTICAL_OFFSET &&
                 meta->height != 0 ) {
                heightWhereAvail[ direction ] += meta->height;
                charSizeWhereHeightAvail[ direction ] += size;
            }
            else {
                charSizeOther[ direction ] += size;
            }
            MemHandleUnlock( metaRecordHandle );
        }
    } while ( ! done );

    meta = MemHandleLock( metaRecord );
    totalHeightWhereAvail = heightWhereAvail[ DIRECTION_UP ] +
                           heightWhereAvail[ DIRECTION_DOWN ] +
                           meta->height;
    MemHandleUnlock( metaRecord );

    record = MemHandleLock( currentRecord );
    totalCharSizeWhereHeightAvail =
        charSizeWhereHeightAvail[ DIRECTION_UP ] +
        charSizeWhereHeightAvail[ DIRECTION_DOWN ] +
        record->size;
    MemHandleUnlock( currentRecord );

    if ( totalCharSizeWhereHeightAvail == 0 ) {
        totalHeightWhereAvail         = GetDefaultMainStyleHeight();
        totalCharSizeWhereHeightAvail = 40;
    }
    charToHeight = ESTIMATE_RESOLUTION * totalHeightWhereAvail /
                       totalCharSizeWhereHeightAvail;

    for ( i = 0 ; i < 2 ; i++ ) {
        estimatedHeight[ i ] = heightWhereAvail[ i ] +
                                   charToHeight * (Int32)charSizeOther[ i ] /
                                       ESTIMATE_RESOLUTION;
    }
}



/* Convert a height-based delta to a record number and position in
   the sequence.  If the record number is the same, no offset is
   returned. */
UInt16 DeltaToRecordId
            (
            YOffset   delta,       /* distance in height from current position */
            Int16*    cOffsetPtr   /* character position in record, set to
                                      NO_OFFSET if irrelevant */
            )
{
    MemHandle handle;
    MemHandle metaRecordHandle;
    Header*   record;
    MetaRecord*     meta;
    PluckerDataType type;
    UInt16    recordNum;
    YOffset   height;
    UInt16    recordId;
    UInt16    lastRecordId;
    Int16     size;
    Int16     lastSize;
    Int16     paragraphs;
    Boolean   done;
    Boolean   estimated;
    DirectionType direction;

    delta  = -delta;
    meta   = MemHandleLock( metaRecord );
    height = meta->height;
    MemHandleUnlock( metaRecord );

    record       = MemHandleLock( currentRecord );
    lastSize     = record->size;
    lastRecordId = record->uid;
    MemHandleUnlock( currentRecord );

    if ( ( delta < 0 && 0 <= delta + GetVerticalOffset() ) ||
         ( 0 <= delta && delta + GetVerticalOffset() < height ) ) {
        return lastRecordId;
    }

    done = false;

    handle = FindRecord( lastRecordId, (Int16 *)&recordNum );
    if ( handle == 0 )
        return lastRecordId;
    else
        FreeRecordHandle( &handle );

    if ( delta < 0 ) {
        delta     += GetVerticalOffset();
        direction  = DIRECTION_UP;
    }
    else {
        delta     -= height - GetVerticalOffset();
        direction  = DIRECTION_DOWN;
    }

    do {
        handle = GetSequentialRecordHandle( &recordNum, direction );
        if ( handle != NULL ) {
            record     = MemHandleLock( handle );
            type       = record->type;
            recordId   = record->uid;
            size       = record->size;
            paragraphs = record->paragraphs;
            MemHandleUnlock( handle );
            FreeRecordHandle( &handle );
            if ( type == DATATYPE_PHTML_COMPRESSED || type == DATATYPE_PHTML ) {
                metaRecordHandle = ReturnMetaHandle( recordId, paragraphs );
                meta             = MemHandleLock( metaRecordHandle );
                height           = meta->height;
                estimated        = false;
                if ( meta->verticalOffset == NO_VERTICAL_OFFSET ||
                     height == 0 ) {
                    height    = (Int32)size * charToHeight /
                                    ESTIMATE_RESOLUTION;
                    estimated = true;
                }
                MemHandleUnlock( metaRecordHandle );

                if ( direction == DIRECTION_UP ) {
                    delta += height;
                    if ( 0 <= delta )
                        done = true;
                }
                else {
                    if ( delta < height )
                        done = true;
                    else
                        delta -= height;
                }
                if ( done ) {
                    if ( estimated ) {
                        *cOffsetPtr = delta * ESTIMATE_RESOLUTION /
                                          charToHeight;
                        if ( size <= *cOffsetPtr ) {
                            if ( 0 < size )
                                *cOffsetPtr = size - 1;
                            else
                                *cOffsetPtr = 0;
                        }
                    }
                    else {
                        YOffset verticalOffset;
                        meta = MemHandleLock( metaRecordHandle );
                        verticalOffset = -delta;
                        DmWrite( meta, OFFSETOF( MetaRecord, verticalOffset ),
                            &verticalOffset, sizeof( YOffset ) );
                        MemHandleUnlock( metaRecordHandle );
                        *cOffsetPtr = NO_OFFSET;
                    }
                    return recordId;
                }
                lastRecordId   = recordId;
                lastSize       = size;
            }
        }
        else
            done = true;
    } while ( ! done );
    if ( direction == DIRECTION_UP )
        *cOffsetPtr        = 0;
    else {
        if ( 0 < lastSize )
            *cOffsetPtr = lastSize - 1;
        else
            *cOffsetPtr = 0;
    }
    return lastRecordId;
}



/* Get the (estimated) height of a sequence of records */
YOffset GetSequenceHeight( void )
{
    MetaRecord* meta;
    YOffset     height;
    meta   = MemHandleLock( metaRecord );
//debug(1, "XXX", "GetSequenceHeight metaRecord");
//debug_bytes(1, "XXX", (UInt8 *)meta, sizeof(MetaRecord));
    height = meta->height + estimatedHeight[ DIRECTION_UP ] + 
                 estimatedHeight[ DIRECTION_DOWN ];
    MemHandleUnlock( metaRecord );
    return height;
}



/* Like GetVerticalOffset(), but within sequence */
YOffset GetSequenceOffset( void )
{
    return GetVerticalOffset()+estimatedHeight[ DIRECTION_UP ];
}
