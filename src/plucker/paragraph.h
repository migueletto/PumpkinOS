/*
 * $Id: paragraph.h,v 1.45 2004/02/01 11:26:33 nordstrom Exp $
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

#ifndef PLUCKER_PARAGRAPH_H
#define PLUCKER_PARAGRAPH_H

#include "viewer.h"
#include "document.h"
#include "util.h"

/*
    A paragraph as it appears in the input data stream. The height of the
    paragraph is calculated on the device.

    A paragraph contains either characters or "functions". A function is
    introduced by a NUL (\0), followed by a function code, followed by up
    to 7 bytes of data (at the moment). The function code's least significant
    3 bits represent the remaining function code length; the most significant
    5 bits represent the function code. For example, the 'ForeColor begin'
    function is 0x83, or 10000011. Breaking it down, 10000 is the unique
    function code, and 011 tells that there is 3 arguments to the function--
    in this case, red, green, and blue.
    
    To get the next character/function, you add the length of the function
    operators + 2 to the pointer to the start of the function.

    Defined function codes:

    00000xxx    0x00  Dummy code (used internally)
    00001010    0x0A  Anchor begins (record ID)
    00001100    0x0C  Named anchor begins (record ID and paragraph offset)
    00001000    0x08  Anchor ends. (==anchor begin with no data)
    00010001    0x11  Set style. (Style is always #0 at start of paragraph)
    00011010    0x1A  Include image (record ID)
    00100010    0x22  Set left and/or right margin (margins in pixels)
    00101001    0x29  Alignment of text (left = 0, right = 1, center = 2)
    00110011    0x33  Horizontal rule
    00111000    0x38  New line
    01000000    0x40  Italic text begins
    01001000    0x48  Italic text ends
    01010011    0x53  Set forecolor (red, green, blue)
    01011100    0x5C  Multi-image
    01100000    0x60  Underlined text begins
    01101000    0x68  Underlined text ends
    01110000    0x70  Strikethrough text begins
    01111000    0x78  Strikethrough text ends
    10000011    0x83  16-bit Unicode character
    10000101    0x85  32-bit Unicode character
    10001110    0x8E  Begin custom font span
    10001100    0x8C  Adjust custom font glyph position
    10001010    0x8A  Change font page
    10001000    0x88  End custom font span
    10010000    0x90  Start Row (in table doc)
    10010010    0x92  Include table (record ID)
    10010111    0x97  Table cell (align, image_ref, colspan, rowspan, text len)

    Note about attributes:
    For Paragraph objects, the attributes consist of the following bits:
    MSB \
    x    \
    x     + Unused.
    x    /
    x   /
    x   \ 
    x    + These bits indicates the extra paragraph spacing required
    LSB /  above the paragraph (value * DEFAULT_PARAGRAPH_SPACING (
           defined in const.h) pixels of spacing).
*/

typedef struct {
    Int16   size;         /* Size of text */
    Int16   attributes;   /* Paragraph info (see above) */
} Paragraph;


typedef struct {
    YOffset height;       /* Height of the paragraph in pixels (see above) */
} MetaParagraph;

/* Activate and clear the autoscroll line cache and reset it */
extern void LineCacheActivate( void ) PARAGRAPH_SECTION;

/* Deactivate autoscroll line cache */
extern void LineCacheDeactivate( void ) PARAGRAPH_SECTION;

/* Clear autoscroll line cache */
extern void LineCacheClear( void ) PARAGRAPH_SECTION;

/* Get state of the autoscroll line cache */
Boolean IsLineCacheActive( void ) PARAGRAPH_SECTION;

/* Fetch paragraph number from last line cache, returning zero if nothing is
   cached or if paragraph #0 is in the cache.  The returned value tells us the
   first paragraph that needs to be rendered given the current line cache. */
extern UInt16 GetLineCacheNeedToRenderFrom( void ) PARAGRAPH_SECTION;

/* Scroll the autoscroll line cache up */
extern void LineCacheScrollUp( Int16 pixels ) PARAGRAPH_SECTION;

/* Refresh data about the current screen, such as anchors, that might be out
   of date due to the line cache. */
extern void LineCacheRefreshCurrentScreenData( void ) PARAGRAPH_SECTION;

/* Draw a paragraph using the given text context */
extern void DrawParagraph( TextContext* tContext, Paragraph* paragraph,
                Header* record ) PARAGRAPH_SECTION;

/* Set position and length of find pattern */
extern void SetFindPatternData( const Int16 pos, const Int16 len ) PARAGRAPH_SECTION;

/* Clear position and length of find pattern */
extern void ClearFindPatternData( void ) PARAGRAPH_SECTION;

/* Add URL of the record to the buffer specified by SetCopyBuffer */
extern Boolean AddURL ( Int16 index ) PARAGRAPH_SECTION;

extern void StartCopyMode( void ) PARAGRAPH_SECTION;
extern void StopCopyMode( void ) PARAGRAPH_SECTION;

/* Set hardcopy buffer and size */
extern void SetCopyBuffer( char* buffer, Int16 length ) PARAGRAPH_SECTION;

extern void NoDrawChars( Char *chars, Int16 storeChar,
                const TextContext *tContext ) PARAGRAPH_SECTION;
extern void DrawChars( Char *chars, Int16 storeChar,
                const TextContext *tContext ) PARAGRAPH_SECTION;
extern void DrawInvertedChars( Char *chars, Int16 storeChar,
                const TextContext *tContext ) PARAGRAPH_SECTION;
extern void CopyChars( Char* chars, Int16 storeChar,
                const TextContext* tContext ) PARAGRAPH_SECTION;
#ifdef SUPPORT_WORD_LOOKUP
extern void GetSelectedWordChars( Char* chars, Int16 length,
                const TextContext* tContext ) PARAGRAPH_SECTION;
extern void SelectedWordReset( void ) PARAGRAPH_SECTION;
/* Find the word containing the word lookup tap position */
extern Char* GetSelectedWord( RectangleType* bounds ) PARAGRAPH_SECTION;
/* Done looking for selected word? */
extern Boolean SelectedWordDone( void ) PARAGRAPH_SECTION;
#endif

/* Get position of pattern to find */
extern Int16 GetFindPatternPos( void ) PARAGRAPH_SECTION;

PLKR_GLOBAL void (*DrawText) ( Char *chars, Int16 storeChar,
                    const TextContext *tContext );



/* Return pointer to paragraph with given index */
#define GET_PARAGRAPH( RECORD, INDEX )      ( (Paragraph*) ( (UInt8*) ( ( RECORD ) + 1 ) + ( INDEX ) * sizeof( Paragraph ) ) )

/* Return pointer to meta paragraph with given index */
#define GET_METAPARAGRAPH( RECORD, INDEX )  ( (MetaParagraph*) ( (UInt8*) ( ( RECORD ) + 1 ) + ( INDEX ) * sizeof( MetaParagraph ) ) )

/* Return pointer to data in given record */
#define GET_DATA( RECORD )                  ( (UInt8*) ( ( RECORD ) + 1 ) + ( RECORD )->paragraphs * sizeof( Paragraph ) )

#endif

