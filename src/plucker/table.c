/*
 * $Id: table.c,v 1.65 2004/05/15 04:16:45 prussar Exp $
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
#include <BmpGlue.h>

#include "control.h"
#include "debug.h"
#include "document.h"
#include "genericfile.h"
#include "hires.h"
#include "loadbar.h"
#include "mainform.h"
#include "os.h"
#include "paragraph.h"
#include "prefsdata.h"
#include "resourceids.h"
#include "screen.h"
#include "uncompress.h"
#include "util.h"
#include "image.h"
#include "font.h"
#include "anchor.h"
#include "list.h"
#include "rotate.h"
#include "cache.h"

#include "table.h"

/***********************************************************************
 *
 *      Internal Constants
 *
 ***********************************************************************/
#define MAX_TABLE_SIZE 480000   /* 60k * 8bit */

#define ANCHOR_END            0x08
#define ANCHOR_BEGIN          0x0A
#define NAMED_ANCHOR_BEGIN    0x0C
#define SET_STYLE             0x11
#define IMAGE                 0x1A
#define NEW_LINE              0x38
#define ITALIC_BEGIN          0x40
#define ITALIC_END            0x48
#define SET_COLOR             0x53
#define MULTI_IMAGE           0x5C
#define UNDERLINE_BEGIN       0x60
#define UNDERLINE_END         0x68
#define STRIKE_BEGIN          0x70
#define STRIKE_END            0x78
#define NEW_ROW               0x90
#define NEW_TABLE             0x92
#define NEW_CELL              0x97

#define CELLS(row,col) cells[row*table->cols+col]
#define CELL_WIDTH(row,col) CELLS(row,col).width/CELLS(row,col).colspan
#define CELL_HEIGHT(row,col) CELLS(row,col).height/CELLS(row,col).rowspan

#define ICON_WIDTH  30
#define ICON_HEIGHT 20

/***********************************************************************
 *
 *      Internal Types
 *
 ***********************************************************************/

typedef struct {
    UInt16 size;
    UInt16 cols;
    UInt16 rows;
    UInt8  depth;
    UInt8  border;
    UInt32 border_color;
    UInt32 link_color;
} Table;

typedef struct {
    Int8   align;
    Coord  height;
    Coord  width;
    Int8   colspan;
    Int8   rowspan;
    UInt16 image;
    Char   *text;
    UInt16 len;
    Coord  splitHeight;
} Cell;

typedef struct {
    RectangleType   bounds;
    UInt16          reference;
    UInt16          altImage;
    UInt16          offset;
} TableAnchorType;

/***********************************************************************
 *
 *      Local functions
 *
 ***********************************************************************/

/* Add an image to a table */
static void AddImage( const UInt16 reference, Coord x, Coord y ) TABLE_SECTION;

/* Draw Table Cell border */
static void DrawCellBox( Coord x, Coord y, Coord width,
                Coord height ) TABLE_SECTION;

/* Add an anchor to the table */
static void DoAnchor( UInt16 reference, UInt16 altImage, UInt16 offset,
                Coord x, Coord y, UInt16 width, UInt16 height ) TABLE_SECTION;

/* Draw Cell text */
static void DrawTableText( Char* text, UInt16 len, Coord x, Coord y,
                Int8 align, UInt16 width, UInt16 height,
                UInt8 border, Boolean addAnchors ) TABLE_SECTION;

/* Get the height and width of the Table Cell */
static void GetTextMetrics( Char* text, UInt16 len,
                Coord* width, Coord* height, Boolean oneLine ) TABLE_SECTION;

/* Draw Table */
static Boolean DrawTable( Table* table, Cell* cells, Coord tableX,
                Coord tableY, Boolean addAnchors ) TABLE_SECTION;

/* Return table size */
static Boolean GetTableMetrics( Table* table, Cell* cells,
                Coord* tableWidth, Coord* tableHeight ) TABLE_SECTION;

/* Release MultiList */
static void ReleaseMultiList( LinkedList list ) TABLE_SECTION;

/* Calculate x and y sizes based on depth */
static void Chop( UInt16 depth, UInt16* X, UInt16 * Y ) TABLE_SECTION;

/* See if the table is in cache */
static Boolean FindTableHandle( Int32 reference ) TABLE_SECTION;

/* Release Anchor List */
static void ReleaseAnchorList( LinkedList* list ) TABLE_SECTION;

/* Calculate space to get align */
static Coord DoAlign( Int8 align, Coord width, Coord size ) TABLE_SECTION;

/***********************************************************************
 *
 *      Private variables
 *
 ***********************************************************************/
static WinHandle        osWindow;
static UInt16           depth;
static Boolean          isLargeTable = false;
static TextContext      text_color;
static TextContext      border_color;
static TextContext      link_color;
static LinkedList       tableAnchorList = NULL;
static Boolean          isItalic = false;
static Boolean          isStrike = false;
static MemHandle        fsTableHandle;






/* Draw 3D icon for table link */
void DrawIcon
    (
    Coord     x,
    Coord     y
    )
{
    RectangleType rect;

    x += 4;
    y += 2;

    RotDrawGrayLine( x, y, x + 23, y );
    RotDrawGrayLine( x, y, x, y + 15 );
    RotDrawLine( x + 7, y, x + 7, y + 15 );
    RotDrawGrayLine( x + 8, y, x + 8, y + 15 );
    RotDrawLine( x + 15, y, x + 15, y + 15 );
    RotDrawGrayLine( x + 16, y, x + 16, y + 15 );
    RotDrawLine( x, y + 7, x + 23, y + 7 );
    RotDrawGrayLine( x, y + 8, x + 23, y + 8 );

    rect.topLeft.x = x;
    rect.topLeft.y = y;
    rect.extent.x = 23;
    rect.extent.y = 15;
    RotDrawRectangleFrame( simpleFrame, &rect );

}



/* Add an image to a table */
static void AddImage
    (
    const UInt16   reference,
    Coord          x,
    Coord          y
    )
{
    TextContext    tContext;
    Coord          width;

    MemSet( &tContext, sizeof( TextContext ), 0 );
    tContext.writeMode = WRITEMODE_DRAW_CHAR;
    tContext.cursorX = x;
    tContext.cursorY = y;
    DrawInlineImage( reference, &tContext, &width );
}



/* Draw Table Cell border */
static void DrawCellBox
    (
    Coord x,
    Coord y,
    Coord width,
    Coord height
    )
{
    RectangleType rect;

    rect.topLeft.x = x + 1;
    rect.topLeft.y = y + 1;
    rect.extent.x = width - 1;
    rect.extent.y = height - 1;

    SetForeColor( &border_color );
    RotDrawRectangleFrame( simpleFrame, &rect );
    SetForeColor( &text_color );
}



/* Calculate space to get align */
static Coord DoAlign
    (
    Int8   align,
    Coord  width,
    Coord  size
    )
{
    Coord offset;

    switch ( align ) {
        case 1:
            offset = width - size - 2;
            break;

        case 2:
            offset = ( width - size ) / 2;
            break;

        default:
            offset = 2;
            break;
    }
    return offset;
}



/* Add an anchor to the table */
static void DoAnchor
    (
    UInt16      reference,
    UInt16      altImage,
    UInt16      offset,
    Coord       x,
    Coord       y,
    UInt16      width,
    UInt16      height
    )
{
    TextContext    tContext;

    MemSet( &tContext, sizeof( TextContext ), 0 );
    tContext.writeMode = WRITEMODE_DRAW_CHAR;

    if ( isLargeTable ) {
        if ( tableAnchorList != NULL ) {
            TableAnchorType* tableAnchor;

            ErrTry {
                tableAnchor = SafeMemPtrNew( sizeof *tableAnchor );
            }
            ErrCatch( UNUSED_PARAM(err) ) {
                return;
            } ErrEndCatch

            tableAnchor->bounds.topLeft.x = x;
            tableAnchor->bounds.topLeft.y = y;
            tableAnchor->bounds.extent.x = width;
            tableAnchor->bounds.extent.y = height;
            tableAnchor->reference = reference;
            tableAnchor->altImage = altImage;
            tableAnchor->offset = offset;
            ListAppend( tableAnchorList, tableAnchor );
        }
    }
    else {
        tContext.cursorX = x; 
        tContext.cursorY = y;
        AnchorStart( &tContext, reference, offset );
        if ( altImage )
            AnchorAppendImage( &tContext, 0, altImage );
        tContext.cursorX = x + width;
        tContext.cursorY = y + height;
        AnchorStop( &tContext, height );
    }
}



/* Draw Cell text */
static void DrawTableText
    (
    Char*       text,
    UInt16      len,
    Coord       x,
    Coord       y,
    Int8        align,
    UInt16      width,
    UInt16      height,
    UInt8       border,
    Boolean     addAnchors
    )
{
    UInt16         count = 0;
    UInt16         size = 0;
    Coord          offX = border;
    Coord          offY = 2 + border;
    TextContext    tContext;
    UInt16         reference = 0;
    UInt16         offset = 0;
    Coord          anchorY = 0;
    Boolean        isAnchor = false;
    RectangleType  rect;
    Coord          high;
    Coord          wide;
    Coord          full_high;
    Coord          full_wide;


    MemSet( &tContext, sizeof( TextContext ), 0 );

    tContext.writeMode = WRITEMODE_DRAW_CHAR;
    tContext.cursorX = x;
    tContext.cursorY = y;

    GetTextMetrics(text, len , &full_wide, &full_high, false );
    offY += (height - full_high) / 2;
    GetTextMetrics(text, len , &wide, &high, true );
    offX += DoAlign( align, width, wide );

    while ( count < len ) {
        if ( text[0] == '\0' ) {
            switch ( ( UInt8 ) text[1] ) {
                case NEW_TABLE: {
                    Boolean goodTable;
                    Int16   len;
                    Coord   wide;
                    UInt16  table;
                    Char    name[20];
                    Coord   tableW;
                    Coord   tableH;

                    table = ( UInt8 ) text[2] * 256 + ( UInt8 ) text[3];
                    len = StrPrintF( name, " [TABLE %d] ", table ); 
                    wide = FntCharsWidth( name, StrLen( name ) );
                    goodTable = GetTableSize( table, &tableW, &tableH );

                    if ( ! goodTable ) {
                        RotDrawChars( name, len, x + offX,
                                      y + offY + high - FntCharHeight() );
                        offX += wide;
                    }
                    else if ( RotExtentX() < tableW ||
                              RotExtentY() < tableH ) {
                        DrawIcon( x + offX, y + offY + high - ICON_HEIGHT );
                        if ( addAnchors )
                            DoAnchor( table, 0, 0, x + offX,
                                      y + offY + high - ICON_HEIGHT,
                                      ICON_WIDTH, ICON_HEIGHT );
                        offX += ICON_WIDTH;
                    }
                    else {
                        InlineTable( table, x + offX, y + offY + high - tableH);
                        offX += tableW;
                    }
                }
                    break;

                case IMAGE: {
                    UInt16  image;
                    Coord   imageW;
                    Coord   imageH;

                    image = ( UInt8 ) text[2] * 256 + ( UInt8 ) text[3];
                    GetImageMetrics( image, &imageW, &imageH );
                    AddImage( image, x + offX, y + offY + high - 1 );
                    if ( isAnchor )
                        DoAnchor( reference, 0, 0, x + offX,
                                  y + offY + high - imageH - 1,
                                  imageW, imageH );
                    offX += imageW;
                }
                    break;

                case MULTI_IMAGE: {
                    UInt16  altImage;
                    UInt16  image;
                    Coord   imageW;
                    Coord   imageH;

                    altImage = ( UInt8 ) text[2] * 256 + ( UInt8 ) text[3];
                    image = ( UInt8 ) text[4] * 256 + ( UInt8 ) text[5];
                    GetImageMetrics( image, &imageW, &imageH );
                    AddImage( image, x + offX, y + offY + imageH - 1 );
                    if ( isAnchor )
                        DoAnchor( reference, altImage, offset, x + offX,
                                  y + offY + high - imageH, imageW, imageH );
                    else
                        DoAnchor( altImage, 0, offset, x + offX,
                                  y + offY + high - imageH, imageW, imageH );
                    offX += imageW;
                }
                    break;

                case SET_STYLE:
                    if ( MINSTYLES <= text[2] && text[2] < MAXSTYLES )
                        FntSetFont( GetMainStyleFont( text[2] ) );
                    break;

                case SET_COLOR:
                    text_color.foreColor.r   = ( UInt8 ) text[2];
                    text_color.foreColor.g   = ( UInt8 ) text[3];
                    text_color.foreColor.b   = ( UInt8 ) text[4]; 
                    SetForeColor( &text_color );
                    break;

                case NEW_LINE:
                    offY += high;
                    anchorY = offY;
                    GetTextMetrics(text + 2, len - count - 2,
                                   &wide, &high, true );
                    offX = border + DoAlign( align, width, wide );
                    break;

                case ANCHOR_BEGIN:
                    reference = ( text[2] << 8 ) + text[3];
                    offset = 0;
                    anchorY = offY;
                    if ( addAnchors )
                        isAnchor = true;
                    break;

                case NAMED_ANCHOR_BEGIN:
                    reference = ( text[2] << 8 ) + text[3];
                    offset = ( text[4] << 8 ) + text[5];
                    anchorY = offY;
                    if ( addAnchors )
                        isAnchor = true;
                    break;

                case ANCHOR_END:
                    isAnchor = false;
                    offset = 0;
                    break;

                case ITALIC_BEGIN:
                    if ( ! SetFontItalic() ) {
                        GrayFntSetBackgroundErase( true );
                        isItalic = true;
                    }
                    break;

                case ITALIC_END:
                    if ( isItalic ) {
                        isItalic = false;
                    }
                    else {
                        EndFontItalic();
                    }
                    break;

                case STRIKE_BEGIN:
                    isStrike = true;
                    break;

                case STRIKE_END:
                    isStrike = false;
                    break;

                case UNDERLINE_BEGIN:
                    WinSetUnderlineMode( solidUnderline );
                    break;

                case UNDERLINE_END:
                    WinSetUnderlineMode( noUnderline );
                    break;
            }
            count   += 2 + ( text[1] & 7 );
            text    += 2 + ( text[1] & 7 );
            continue;
        }

        size = FntCharsWidth( text, StrLen( text ) );

        if ( isAnchor ) {
            SetForeColor( &link_color );
        }

        RotDrawChars( text, StrLen( text ), x + offX,
                      y + offY + high - FntCharHeight() );

        if ( isAnchor ) {
            DoAnchor( reference, 0, offset, x + offX,
                      y + anchorY + high - FntCharHeight(),
                      size, FntCharHeight() );
            SetForeColor( &text_color );
        }
        if ( isItalic ) {
            rect.topLeft.x = x + offX;
            rect.topLeft.y = y + offY;
            rect.extent.x = size + 1;
            rect.extent.y = FntCharHeight() / 2 + 1;
            RotScrollRectangle( &rect, winRight, 1, &rect );
            RotEraseRectangle( &rect, 0 );
        }

        if ( isStrike ) {
            rect.topLeft.x = x + offX;
            rect.topLeft.y = y + offY;
            rect.extent.x = size - 1;
            rect.extent.y = FntCharHeight() - 1;
            StrikeThrough( &rect );
        }

        offX    += size;
        count   += StrLen( text );
        text    += StrLen( text );
    }

}



/* Get the height and width of the Table Cell */
static void GetTextMetrics
    (
    Char*   text,
    UInt16  len,
    Coord*  width,
    Coord*  height,
    Boolean oneLine
    )
{
    UInt16  count = 0;
    Coord   wide = 0;
    Coord   high = 0;

    *width = *height = 0;

    while ( count < len ) {
        if ( text[0] == '\0' ) {
            switch ( ( UInt8 ) text[1] ) {
                case NEW_TABLE: {
                    Boolean goodTable;
                    Int16   len;
                    UInt16  table;
                    Char    name[20];
                    Coord   tableW;
                    Coord   tableH;

                    table = ( UInt8 ) text[2] * 256 + ( UInt8 ) text[3];
                    len = StrPrintF(name, " [TABLE %d] ", table); 
                    goodTable = GetTableSize( table, &tableW, &tableH);

                    if ( ! goodTable ) {
                        wide += FntCharsWidth( name, len );
                        high = max( high, FontLineHeight() );
                    }
                    else if ( RotExtentX() < tableW ||
                                RotExtentY() < tableH ) {
                        wide += ICON_WIDTH;
                        high = max( high, ICON_HEIGHT );
                    }
                    else {
                        wide += tableW;
                        high = max( high, tableH );
                    }
                }
                    break;

                case IMAGE: {
                    UInt16  image;
                    Coord   imageW;
                    Coord   imageH;

                    image = ( UInt8 ) text[2] * 256 + ( UInt8 ) text[3];
                    GetImageMetrics( image, &imageW, &imageH );
                    wide += imageW;
                    high = max( high, imageH );
                }
                    break;

                case MULTI_IMAGE: {
                    UInt16  image;
                    Coord   imageW;
                    Coord   imageH;

                    image = ( UInt8 ) text[4] * 256 + ( UInt8 ) text[5];
                    GetImageMetrics( image, &imageW, &imageH );
                    wide += imageW;
                    high = max( high, imageH );
                }
                    break;

                case SET_STYLE:
                    if ( MINSTYLES <= text[2] && text[2] < MAXSTYLES )
                        FntSetFont( GetMainStyleFont( text[2] ) );
                    break;

                case NEW_LINE:
                    if ( *width < wide )
                        *width = wide;
                    if ( wide == 0 ) /* A bare <BR> or <P> for formatting */
                        high += FontLineHeight();
                    wide = 0;
                    *height += high;
                    high = 0;
                    if ( oneLine )
                        return;
                    break;
            }
            count += 2 + ( text[1] & 7 );
            text += 2 + ( text[1] & 7 );
            continue;
        }

        wide        += FntCharsWidth( text, StrLen( text ) ) + 4;
        count       += StrLen( text );
        text        += StrLen( text );
        high        = max( high, FontLineHeight() );
    }

    if ( *width < wide )
        *width = wide;
    *height += high;
    if ( ! oneLine )
        *height += 6;

    return;
}



/* Return table size by Reference */
Boolean GetTableSize
    (
    UInt16       reference,
    Coord*       tableWidth,
    Coord*       tableHeight
    )
{
    MemHandle   uncompressHandle = NULL;
    MemHandle   tableHandle;
    Header*     tableRecord;
    Table*      table = NULL;
    Boolean     result;
    UInt32      size;
    Cell*       cells;


    tableHandle = GetRecordHandle( reference );
    tableRecord = MemHandleLock( tableHandle );

    switch ( tableRecord->type ) {
        case DATATYPE_TABLE_COMPRESSED:
            ErrTry {
                uncompressHandle = Uncompress( tableRecord );
            }
            ErrCatch( UNUSED_PARAM( err ) ) {
                MemHandleUnlock( tableHandle );
                FreeRecordHandle( &tableHandle );
                return false;
            } ErrEndCatch
            table = MemHandleLock( uncompressHandle );
            break;

        case DATATYPE_TABLE:
            table = ( Table* ) ( tableRecord + 1 );
            break;
            
        default:
            MemHandleUnlock( tableHandle );
            FreeRecordHandle( &tableHandle );
            return false;
    }

    size = sizeof( Cell ) * ( table->cols ) * ( table->rows );

    ErrTry {
        cells = SafeMemPtrNew( size );
    }
    ErrCatch( UNUSED_PARAM(err) ) {
        FrmAlert( warnInsufficientMemory );
        if ( tableRecord->type == DATATYPE_TABLE_COMPRESSED ) {
            MemHandleUnlock( uncompressHandle );
            FreeUncompressTableRecord( tableRecord->uid );
        }
        return false;
    } ErrEndCatch

    MemSet( cells, size, 0 );

    result = GetTableMetrics( table, cells, tableWidth, tableHeight );

    if ( tableRecord->type == DATATYPE_TABLE_COMPRESSED ) {
        MemHandleUnlock( uncompressHandle );
        FreeUncompressTableRecord( tableRecord->uid );
    }
    MemHandleUnlock( tableHandle );
    FreeRecordHandle( &tableHandle );
    SafeMemPtrFree( cells );

    return result;
}



/* Return table size */
static Boolean GetTableMetrics
    (
    Table* table,
    Cell*  cells,
    Coord* tableWidth,
    Coord* tableHeight
    )
{
    UInt16     row;
    UInt16     row2;
    UInt16     col;
    UInt16     col2;
    UInt8*     ptr;
    UInt8*     last;
    Cell*      acell;
    UInt16*    row_base = NULL;
    UInt16*    col_base = NULL;
    UInt16     prevCoordSys;
    Coord      saveWidth;
    Coord      saveHeight;

    prevCoordSys   = PalmSetCoordinateSystem( NATIVE );

    ErrTry {
        col_base = SafeMemPtrNew( sizeof( UInt16 ) * table->cols );
        row_base = SafeMemPtrNew( sizeof( UInt16 ) * table->rows );
    }
    ErrCatch( UNUSED_PARAM(err) ) {
        if ( col_base != NULL )
            SafeMemPtrFree( col_base );
        FrmAlert( warnInsufficientMemory );
        return false;
    } ErrEndCatch


    depth = min( table->depth, Prefs()->screenDepth );

    /* Set defaults */
    for ( row = 0; row < table->rows; row++ ) {
        for ( col = 0; col < table->cols; col++ ) {
            CELLS( row, col ).colspan = 1;
            CELLS( row, col ).rowspan = 1;
        }
    }

    ptr     = (UInt8*) ( table + 1 );
    last    = ptr + table->size - 1;

    row = col = -1;

    while ( ptr < last ) {
        if ( ptr[0] == '\0' ) {
            switch ( ptr[1] ) {
                case NEW_ROW:
                    row++;
                    col = -1;
                    ptr += 2;
                    break;

                case NEW_CELL:
                    col++;
                    acell = &CELLS( row, col );
                    acell->width = acell->height = 0;
                    saveWidth = saveHeight = 0;
                    acell->align = ptr[2];
                    acell->image = ptr[3] * 256 + ptr[4];
                    acell->colspan = ptr[5];
                    acell->rowspan = ptr[6];
                    acell->len = ptr[7] * 256 + ptr[8];
                    ptr += 9;
                    acell->text = (Char *)ptr;
                    if ( acell->image )
                        GetImageMetrics( acell->image, &saveWidth,
                                         &saveHeight );
                    acell->splitHeight = saveHeight;
                    if ( acell->text )
                        GetTextMetrics( (Char *)ptr, acell->len, &acell->width,
                                        &acell->height, false );
                    acell->splitHeight = saveHeight;
                    acell->width = max( acell->width, saveWidth );
                    acell->height = acell->height + saveHeight;
                    acell->width += table->border * 2;
                    acell->height += table->border * 2;
                    /* Skip text */
                    ptr += acell->len;
                    /* add empty cells for colspan */
                    if (1 < acell->colspan)
                        col += acell->colspan - 1;
                    break;

                default:
                    ptr += 2 + ( ptr[1] & 0x07 );
            }
        } else {
            return false;
        }
    }

    /* Calc table height and cell heights (add empty cells for rowspan) */
    *tableHeight = 1;
    for ( row = 0; row < table->rows; row++ ) {
        row_base[row] = 0;
        for ( col = 0; col < table->cols; col++ ) {
            if ( row_base[row] < CELL_HEIGHT( row, col ) ) {
                row_base[row] = CELL_HEIGHT( row,col );
            }
            if ( 1 < CELLS( row, col ).rowspan ) {
                for ( row2 = row + 1; row2 < row + CELLS( row, col ).rowspan
                        && row2 < table->rows; row2++ ) {
                    for ( col2 = table->cols - 1; col < col2; col2-- )
                        MemMove( &CELLS( row2, col2 ),
                                &CELLS( row2, col2 - 1 ), sizeof( Cell ) );
                    MemSet( &CELLS( row2, col ), sizeof( Cell ), 0 );
                    CELLS( row2, col ).height = CELL_HEIGHT( row, col );
                    CELLS( row2, col ).colspan = 1;
                    CELLS( row2, col ).rowspan = 1;
                }
            }
        }
        for ( col = 0; col < table->cols; col++ )
            if ( CELLS( row, col ).rowspan == 1 )
                CELLS( row, col ).height = row_base[row];
        *tableHeight += row_base[row];
    }

    /* Calc table width and cell widths */
    *tableWidth = 1;
    for ( col = 0; col < table->cols; col++ ) {
        col_base[col] = 0;
        for ( row = 0; row < table->rows; row++ ) {
            if ( col_base[col] <  CELL_WIDTH( row, col ) ) {
                col_base[col] = CELL_WIDTH( row, col );
            }
            if ( 1 < CELLS( row, col ).colspan ) {
                for ( col2 = col; col2 < col + CELLS( row, col ).colspan &&
                      col2 < table->cols; col2++ )
                    CELLS( row, col2 ).width = col_base[col];
            }
        }
        for ( row = 0; row < table->rows; row++ ) {
            if ( CELLS( row, col ).colspan == 1 ) {
                CELLS( row, col ).width = col_base[col];
            }
        }
        *tableWidth += col_base[col];
    }

    /* Calc 'rowspan' cell heights*/
    for ( col = 0; col < table->cols; col++ ) {
        for ( row = 0; row < table->rows; row++ )
            if ( 1 < CELLS( row, col ).rowspan ) {
                CELLS( row, col ).height = 0;
                for ( row2 = row; row2 < row + CELLS( row, col ).rowspan;
                        row2++ )
                    CELLS( row, col ).height += row_base[row2];
            }
    }

    SafeMemPtrFree( col_base );
    SafeMemPtrFree( row_base );

    PalmSetCoordinateSystem( prevCoordSys );

    return true;
}



/* Draw Table */
static Boolean DrawTable
    (
    Table*   table,
    Cell*    cells,
    Coord    tableX,
    Coord    tableY,
    Boolean  addAnchors
    )
{
    UInt16     row;
    UInt16     col;
    Cell*      acell;
    Coord      x;
    Coord      y;
    UInt16     row_height = 0;
    UInt16     prevCoordSys;

    prevCoordSys   = PalmSetCoordinateSystem( NATIVE );
    FntSetFont( GetMainStyleFont( DEFAULTSTYLE ) );

    MemSet( &border_color, sizeof( TextContext ), 0 );
    MemSet( &link_color, sizeof( TextContext ), 0 );
    MemSet( &text_color, sizeof( TextContext ), 0 );

    border_color.foreColor.r   = ( table->border_color >> 16 ) & 0xff;
    border_color.foreColor.g   = ( table->border_color >> 8 ) & 0xff;
    border_color.foreColor.b   = ( table->border_color ) & 0xff;
    link_color.foreColor.r   = ( table->link_color >> 16 ) & 0xff;
    link_color.foreColor.g   = ( table->link_color >> 8 ) & 0xff;
    link_color.foreColor.b   = ( table->link_color ) & 0xff;

    isItalic = isStrike = false;

    y = tableY;
    x = tableX;
    for ( row = 0; row < table->rows; row++ ) {
        row_height = 0;
        for ( col = 0; col < table->cols; col++ ) {
            acell = &CELLS ( row, col );
            if ( acell->colspan ) {
                if ( table->border && ( acell->image || acell->text ) )
                    DrawCellBox( x, y, acell->width, acell->height );
                if ( acell->image )
                    AddImage( acell->image, x + table->border,
                              y + table->border + acell->height - 1 );
                if ( acell->text )
                    DrawTableText( acell->text, acell->len, x,
                                   y + acell->splitHeight,
                                   acell->align, acell->width,
                                   acell->height - acell->splitHeight,
                                   table->border, addAnchors );
                x += acell->width;
                if ( acell->rowspan == 1 )
                    row_height = acell->height;
            }
        }
        y += row_height;
        x = tableX;
    }

    WinSetUnderlineMode( noUnderline );

    PalmSetCoordinateSystem( prevCoordSys );

    return true;
}



/* Draw inline table */
Boolean InlineTable
    (
    UInt16 reference,
    Coord  tableX,
    Coord  tableY
    )
{
    Coord       tableWidth;
    Coord       tableHeight;
    Table*      table = NULL;
    MemHandle   uncompressHandle = NULL;
    MemHandle   tableHandle;
    Header*     tableRecord;
    UInt32      size;
    Cell*       cells;


    tableHandle = GetRecordHandle( reference );
    tableRecord = MemHandleLock( tableHandle );

    switch ( tableRecord->type ) {
        case DATATYPE_TABLE_COMPRESSED:
            ErrTry {
                uncompressHandle = Uncompress( tableRecord );
            }
            ErrCatch( UNUSED_PARAM( err ) ) {
                MemHandleUnlock( tableHandle );
                FreeRecordHandle( &tableHandle );
                return false;
            } ErrEndCatch
            table = MemHandleLock( uncompressHandle );
            break;

        case DATATYPE_TABLE:
            table = ( Table* ) ( tableRecord + 1 );
            break;

        default:
            return false;
        }


    size = sizeof( Cell ) * ( table->cols ) * ( table->rows );

    ErrTry {
        cells = SafeMemPtrNew( size );
    }
    ErrCatch( UNUSED_PARAM(err) ) {
        FrmAlert( warnInsufficientMemory );
        if ( tableRecord->type == DATATYPE_TABLE_COMPRESSED ) {
            MemHandleUnlock( uncompressHandle );
            FreeUncompressTableRecord( tableRecord->uid );
        }
        MemHandleUnlock( tableHandle );
        FreeRecordHandle( &tableHandle );
        return false;
    } ErrEndCatch

    MemSet( cells, size, 0 );

    if ( !GetTableMetrics( table, cells, &tableWidth, &tableHeight ) ) {
        if ( tableRecord->type == DATATYPE_TABLE_COMPRESSED ) {
            MemHandleUnlock( uncompressHandle );
            FreeUncompressTableRecord( tableRecord->uid );
        }
        MemHandleUnlock( tableHandle );
        FreeRecordHandle( &tableHandle );
        SafeMemPtrFree( cells );
        return false;
    }

    DrawTable( table, cells, tableX, tableY, true );

    if ( tableRecord->type == DATATYPE_TABLE_COMPRESSED ) {
        MemHandleUnlock( uncompressHandle );
        FreeUncompressTableRecord( tableRecord->uid );
    }
    MemHandleUnlock( tableHandle );
    FreeRecordHandle( &tableHandle );
    SafeMemPtrFree( cells );

    return true;
}


/* See if the table is in cache */
Boolean FindTableHandle( Int32 reference )
{
    MemHandle             tableHandle;
    MultiImageNodeType*   node;
    ImageType*            tablePtr;

    tableHandle = LoadFromCache( IMAGEHANDLE, reference );
    if ( tableHandle != NULL ) {
        tablePtr        = MemHandleLock( tableHandle );
        isLargeTable    = true;
        if ( tablePtr->type == MULTIIMAGE ) {
            node = ListFirst( tablePtr->data.multiList );
            while ( isLargeTable && node != NULL ) {
                isLargeTable    = FindTableHandle( node->reference );
                node            = ListNext( tablePtr->data.multiList, node );
            }
        }
        MemHandleUnlock( tableHandle );
        fsTableHandle = tableHandle;

        return isLargeTable;
    }
    return false;
}



/* Calculate x and y sizes based on depth */
/* They all add up to MAX_TABLE_SIZE (480000 bits) */
void Chop( UInt16 depth, UInt16* X, UInt16 * Y )
{
    switch ( depth )
        {
        case 1:
            *X = 800;
            *Y = 600;
            break;

        case 2:
            *X = 600;
            *Y = 400;
            break;

        case 4:
            *X = 400;
            *Y = 300;
            break;

        case 8:
            *X = 300;
            *Y = 200;
            break;

        case 16:
        default:
            *X = 300;
            *Y = 100;
            break;
        }
}



/* Load table */
Boolean LoadTable
    (
    Header*         record, /* pointer to record */
    const Boolean   newPage /* true if the page is from the history */
    )
{
    Err                   error;
    WinHandle             screenWindow;
    Coord                 tableWidth;
    Coord                 tableHeight;
    MemHandle             uncompressHandle = NULL;
    Table*                table = NULL;
    UInt32                size;
    Cell*                 cells;
    ImageType*            tablePtr;
    LinkedList            list;
    MultiImageNodeType*   node;
    UInt16                chopX;
    UInt16                chopY;
    UInt16                x;
    UInt16                y;
    MemHandle             subTableHandle;
    BitmapType*           bitmap    = NULL;
    BitmapType*           oldBitmap = NULL;
    LoadBarType*          loadBar   = NULL;
    Boolean               convert   = false;
    Int32                 dynamicUid;
    Int32                 anchorListUid;
    UInt16                pieces;
    Boolean               memError = false;
#ifdef HAVE_ROTATE
    RotateType            oldRotate;
#endif

    /* Check if we already have a valid imageHandle stored in cache */
    if ( FindTableHandle( record->uid ) )
        return true;

    screenWindow = WinGetDrawWindow();
    Chop( depth, &chopX, &chopY );

    switch ( record->type ) {
        case DATATYPE_TABLE_COMPRESSED:
            ErrTry {
                uncompressHandle = Uncompress( record );
            }
            ErrCatch( UNUSED_PARAM( err ) ) {
                return false;
            } ErrEndCatch
            table = MemHandleLock( uncompressHandle );
            break;

        case DATATYPE_TABLE:
            table = (Table*)( record + 1 );
            break;

        default:
            return false;
    }

    size = sizeof( Cell ) * table->cols * table->rows;

    ErrTry {
        cells = SafeMemPtrNew( size );
    }
    ErrCatch( UNUSED_PARAM(err) ) {
        FrmAlert( warnInsufficientMemory );
        if ( record->type == DATATYPE_TABLE_COMPRESSED ) {
            MemHandleUnlock( uncompressHandle );
            FreeUncompressTableRecord( record->uid );
        }
        return false;
    } ErrEndCatch

    MemSet( cells, size, 0 );

    SetWorking();

    if ( ! GetTableMetrics( table, cells, &tableWidth, &tableHeight ) ) {
        if ( record->type == DATATYPE_TABLE_COMPRESSED ) {
            MemHandleUnlock( uncompressHandle );
            FreeUncompressTableRecord( record->uid );
        }
        SafeMemPtrFree( cells );
        return false;
    }

    dynamicUid      = record->uid * 100;
    dynamicUid      = -dynamicUid;
    anchorListUid   = dynamicUid;
    isLargeTable    = true;
    tableAnchorList = ListCreate();
    list            = ListCreate();

    x = tableWidth / chopX + ( tableWidth % chopX ? 1 : 0 );
    y = tableHeight / chopY + ( tableHeight % chopY ? 1 : 0 );
    pieces = x * y;
    if ( 1 < pieces )
        loadBar = LoadBarNew( pieces );

#ifdef HAVE_ROTATE
    oldRotate = Prefs()->rotate;
    Prefs()->rotate = ROTATE_ZERO;
#endif

    x = y = 0;
    while ( x < tableWidth ) {
        while ( y < tableHeight ) {
            Coord bitmapX = min( tableWidth - x, chopX );
            Coord bitmapY = min( tableHeight - y, chopY );

            dynamicUid--;   /* -(record->uid * 100) is tableAnchorList */

            WinSetDrawWindow( screenWindow );
            LoadBarNextStep( loadBar );

            if ( Support35() ) {
                bitmap    = PortableBmpCreate( bitmapX, bitmapY, depth, NULL, &error );
                oldBitmap = bitmap;
                convert   = ConvertImageToV3( &bitmap );
                osWindow  = WinCreateBitmapWindow( bitmap, &error );
            } else {
                osWindow = WinCreateOffscreenWindow( bitmapX, bitmapY,
                               screenFormat, &error );
            }
            if ( osWindow == NULL ) {
                memError = true;
                goto Error;
            }

            WinSetDrawWindow( osWindow );
            PalmSetCoordinateSystem( NATIVE );
            WinEraseWindow();
            if ( ! DrawTable( table, cells, -x, -y, ( x == 0 && y == 0 ) ) )
                goto Error;

            ErrTry {
                node = SafeMemPtrNew( sizeof( MultiImageNodeType ) );
            }
            ErrCatch( UNUSED_PARAM(err) ) {
                memError = true;
                goto Error;
            } ErrEndCatch

            MemSet( node, sizeof( MultiImageNodeType ), 0 );
            node->reference             = dynamicUid;
            node->column                = ( x / chopX ) + 1;
            node->row                   = ( y / chopY ) + 1;
            node->position.extent.x     = bitmapX;
            node->position.extent.y     = bitmapY;
            node->position.topLeft.x    = x;
            node->position.topLeft.y    = y;

            ErrTry {
                subTableHandle = SafeMemHandleNew( sizeof( ImageType ) );
            }
            ErrCatch( UNUSED_PARAM(err) ) {
                SafeMemPtrFree( node );
                memError = true;
                goto Error;
            } ErrEndCatch

            tablePtr = MemHandleLock( subTableHandle );
            MemSet( tablePtr, sizeof( ImageType ), 0 );

            if ( Support35() ) {
                tablePtr->data.bitmap = convert ? oldBitmap : bitmap;
                WinDeleteWindow( osWindow, false );
                tablePtr->type = DIRECT;
            } else {
                tablePtr->data.window = osWindow;
                tablePtr->type = WINDOW_HANDLE;
            }
            tablePtr->reference     = dynamicUid;
            tablePtr->totalImages   = 1;
            tablePtr->width         = bitmapX;
            tablePtr->height        = bitmapY;
            tablePtr->pixelDepth    = depth;
            node->imageHandle       = subTableHandle;
            MemHandleUnlock( subTableHandle );
            if ( Support35() && 1 < pieces ) {
                AddPtrToCache( IMAGEHANDLE, dynamicUid, subTableHandle,
                               FreeImageHandle );
                //SaveImageInStorageCache( subTableHandle );
                if ( convert ) {
                    BmpDelete( bitmap );
                    PortableBmpDelete( oldBitmap );
                }
                else {
                    PortableBmpDelete( bitmap );
                }
            }
            ListAppend( list, node );
            y += chopY;
        }
        x += chopX;
        y = 0;
    }

    WinSetDrawWindow( screenWindow );
    LoadBarFree( loadBar );

    if ( record->type == DATATYPE_TABLE_COMPRESSED ) {
        MemHandleUnlock( uncompressHandle );
        FreeUncompressTableRecord( record->uid );
    }
    SafeMemPtrFree( cells );

    if ( tableAnchorList != NULL ) {
        if ( ListSize( tableAnchorList ) )
            AddPtrToCache( ANCHORLIST, anchorListUid, tableAnchorList,
                           ReleaseAnchorList ); 
        else
            ListRelease( tableAnchorList );
        tableAnchorList = NULL;
    }

    if ( 1 < pieces ) {
        ErrTry {
            fsTableHandle = SafeMemHandleNew( sizeof( ImageType ) );
        }
        ErrCatch( UNUSED_PARAM(err) ) {
            memError = true;
            goto Error;
        } ErrEndCatch

        tablePtr = MemHandleLock( fsTableHandle );
        MemSet( tablePtr, sizeof( ImageType ), 0 );

        tablePtr->reference         = record->uid;
        tablePtr->totalImages       = ListSize( list );
        tablePtr->width             = tableWidth;
        tablePtr->height            = tableHeight;
        tablePtr->pixelDepth        = depth;
        tablePtr->type              = MULTIIMAGE;
        tablePtr->data.multiList    = list;
        MemHandleUnlock( fsTableHandle );
        if ( Support35() )
            AddPtrToCache( IMAGEHANDLE, record->uid, fsTableHandle,
                           FreeImageHandle );
    } else {
        node = ListFirst( list );
        fsTableHandle = node->imageHandle;
        if ( Support35() ) {
            AddPtrToCache( IMAGEHANDLE, record->uid, fsTableHandle,
                           FreeImageHandle );
            tablePtr            = MemHandleLock( fsTableHandle );
            tablePtr->reference = record->uid;
            MemHandleUnlock( fsTableHandle );
            //SaveImageInStorageCache( fsTableHandle );
            if ( convert ) {
                BmpDelete( bitmap );
                PortableBmpDelete( oldBitmap );
            }
            else {
                PortableBmpDelete( bitmap );
            }
        }
        ListRelease( list );
    }

#ifdef HAVE_ROTATE
    Prefs()->rotate = oldRotate;
#endif
    return true;

Error:
    if ( record->type == DATATYPE_TABLE_COMPRESSED ) {
        MemHandleUnlock( uncompressHandle );
        FreeUncompressTableRecord( record->uid );
    }

    if ( Support35() ) {
        if ( convert ) {
            BmpDelete( bitmap );
            PortableBmpDelete( oldBitmap );
        }
        else {
            PortableBmpDelete( bitmap );
        }
    }

    ReleaseMultiList( list );
    ListRelease( tableAnchorList );
    AnchorListRelease();

#ifdef HAVE_ROTATE
    Prefs()->rotate = oldRotate;
#endif

    if ( osWindow != NULL )
        WinDeleteWindow( osWindow, false );
    WinSetDrawWindow( screenWindow );
    SafeMemPtrFree( cells );
    isLargeTable = false;
    LoadBarFree( loadBar );
    if ( memError )
        FrmAlert( warnInsufficientMemory );

    return false;
}



MemHandle GetFullscreenTableHandle( void )
{
    return fsTableHandle;
}


/* Check if a large table is displayed */
Boolean IsLargeTable( void )
{
    return isLargeTable;
}



/* Copy visible anchors from large table to screen */
void CopyTableAnchors
    (
    Int32 reference,
    Int16 x,
    Int16 y
    )
{
    LinkedList        list;
    TextContext       tContext;
    TableAnchorType*  tableAnchor;
    RectangleType     screen;
    RectangleType     anchorBounds;
#ifdef HAVE_ROTATE
    RotateType        oldRotate;
#endif

    list = LoadFromCache( ANCHORLIST, -( reference * 100 ) );
    if ( list == NULL )
        return;

#ifdef HAVE_ROTATE
    oldRotate = Prefs()->rotate;
    Prefs()->rotate = ROTATE_ZERO;
#endif

    screen.topLeft.x   = x;
    screen.topLeft.y   = y;
    screen.extent.x    = RotMaxExtentX();
    screen.extent.y    = RotMaxExtentY();

    AnchorListInit();
    MemSet( &tContext, sizeof( TextContext ), 0 );
    tContext.writeMode = WRITEMODE_DRAW_CHAR;

    if ( list != NULL ) {
        tableAnchor = ListFirst( list );
        while ( tableAnchor != NULL ) {
            RctGetIntersection( &tableAnchor->bounds, &screen, &anchorBounds );
            if ( anchorBounds.extent.x && anchorBounds.extent.y ) {
                tContext.cursorX = anchorBounds.topLeft.x - x;
                tContext.cursorY = anchorBounds.topLeft.y - y;
                AnchorStart( &tContext, tableAnchor->reference,
                             tableAnchor->offset );
                if ( tableAnchor->altImage != 0 )
                    AnchorAppendImage( &tContext, 0, tableAnchor->altImage );
                tContext.cursorX = tContext.cursorX + anchorBounds.extent.x;
                tContext.cursorY = tContext.cursorY + anchorBounds.extent.y;
                AnchorStop( &tContext, anchorBounds.extent.y );
            }
            tableAnchor = ListNext( list, tableAnchor );
        }
    }
#ifdef HAVE_ROTATE
    Prefs()->rotate = oldRotate;
#endif
}



/* Release Anchor List */
void ReleaseAnchorList( LinkedList* list )
{
    ListRelease( *list );
}



/* Release MultiList */
void ReleaseMultiList( LinkedList list )
{
    ImageType*          subTablePtr;
    MultiImageNodeType* node;

    node = ListFirst( list );
    while ( node != NULL ) {
        subTablePtr = MemHandleLock( node->imageHandle );
        if ( subTablePtr->type == WINDOW_HANDLE ) {
            WinDeleteWindow( subTablePtr->data.window, false );
            MemHandleUnlock( node->imageHandle );
            SafeMemHandleFree( node->imageHandle );
        }
        /* Reset/RemoveCache will take care of CACHE_STORAGE */
        else if ( subTablePtr->type != CACHE_STORAGE ) {
            MemHandleUnlock( node->imageHandle );
            FreeImageHandle( &node->imageHandle );
        }
        node = ListNext( list, node );
    }
    ListRelease( list );
}



/* Delete table window, table images */
void ReleaseFsTableHandle( void )
{
    ImageType*          tablePtr;


    /* Reset/RemoveCache will take care of CACHE_STORAGE */
    if ( fsTableHandle != NULL ) {
        tablePtr = MemHandleLock( fsTableHandle );
        if ( HasCacheNode( IMAGEHANDLE, tablePtr->reference ) ) {
            MemHandleUnlock( fsTableHandle );
        }
        else if ( tablePtr->type == WINDOW_HANDLE ) {
            WinDeleteWindow( tablePtr->data.window, false );
            MemHandleUnlock( fsTableHandle );
            SafeMemHandleFree( fsTableHandle );
        }
        else if ( tablePtr->type == MULTIIMAGE ) {
            ReleaseMultiList( tablePtr->data.multiList );
            MemHandleUnlock( fsTableHandle );
            SafeMemHandleFree( fsTableHandle );
        }
        else if ( tablePtr->type != CACHE_STORAGE ) {
            MemHandleUnlock( fsTableHandle );
            FreeImageHandle( &fsTableHandle );
        }
        fsTableHandle = NULL;
    }

    isLargeTable = false;
}

