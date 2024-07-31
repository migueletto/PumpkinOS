/*
 * $Id: paragraph.c,v 1.154 2004/02/20 16:19:19 chrish Exp $
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
#include <TxtGlue.h>

#include "anchor.h"
#include "const.h"
#include "debug.h"
#include "document.h"
#include "font.h"
#include "genericfile.h"
#include "image.h"
#include "os.h"
#include "prefsdata.h"
#include "screen.h"
#include "uncompress.h"
#include "util.h"
#include "table.h"
#include "search.h"
#include "rotate.h"

#include "paragraph.h"

#ifdef HAVE_PALMCUNIT
#include "mock_Header.h"
#endif


/***********************************************************************
 *
 *      Internal Constants
 *
 ***********************************************************************/
#define SOFT_HYPHEN          0x00AD
#define TEXT_BUFFER_SIZE     80   /* maximum length of portion to work on
                                     at once */
#define MAX_WORD_LENGTH      30   /* for word lookup;  this includes any
                                     punctuation characters around the
                                     word */
#define MAX_WORD_STRING_LENGTH ( 2 * MAX_WORD_LENGTH )                                     

/***********************************************************************
 *
 *      Internal types
 *
 ***********************************************************************/
typedef enum {
    TOKEN_CHARACTER,
    TOKEN_PARAGRAPH_END,
    TOKEN_FUNCTION
} TokenType;


/*
   ParagraphContext is used to help parse paragraphs.
 */
typedef struct {
    Char*           recordTop;  /* Points to start of record */
    Char*           last;       /* Points to last character in paragraph + 1 */
    Char*           position;   /* Points to next character in paragraph */
    UInt8*          function;   /* Points to function if last token was a
                                   function */
    Int16           fontHeight; /* Height of line in current font */
    Int16           left;       /* Left margin */
    Int16           right;      /* Right margin */
    Int16           maxPixels;  /* Max number of pixels for line */
    Int16           linePixels; /* Number of pixels for current line */
    AlignmentType   type;       /* Alignment type */
    Int16           style;      /* font style at this position */
    Boolean         italic;     /* Are we in an italic situation? */
} ParagraphContext;


typedef enum { UNICODE16, UNICODE32 } UnicodeType;


/* We are only interested in IMAGE, NEWLINE and HRULE, but we include
   other types too to keep the type consistant */
typedef enum { NONE, ANCHOR, IMAGE, NEWLINE, HRULE, STYLE, ITALIC, UNDERLINE,
               FORECOLOR, STRIKETHROUGH, MARGIN, ALIGNMENT,
               UNICODE, TABLE } FunctionType;


/* we use the following struct to link function codes to the function that
   will do the requested action */
typedef FunctionType (*FunctionHandler)( ParagraphContext*, TextContext*,
                        Int16*);
typedef struct {
    UInt8           code;
    FunctionHandler doAction;
 } ParagraphFunction;

typedef struct
{
    Boolean Anchor;
    Boolean Italic;
    Boolean Underline;
    Boolean Strike;
    Boolean ForeColor;
} MultilineData;

typedef struct
{
   ParagraphContext     pContext;
   TextContext          tContext;
   MultilineData        multiline;
   Int16                style;
   Int16                prevFontHeight;
   Boolean              fixedWidthFont;
   Boolean              lineBreak;
} LineCacheEntry;


/***********************************************************************
 *
 *      Local functions
 *
 ***********************************************************************/
static void SaveLineCacheEntry(ParagraphContext *p, TextContext *t)
    PARAGRAPH_SECTION;
static void RestoreLineCacheEntry(ParagraphContext *p, TextContext *t)
    PARAGRAPH_SECTION;
static Int16 GetParagraphOffset( Header* record,
                 Paragraph* paragraph, Int16* paragraphNumPtr )
                 PARAGRAPH_SECTION;
static void AlignText( TextContext* tContext,
                ParagraphContext* pContext ) PARAGRAPH_SECTION;
static Boolean CharIsSpace( WChar c ) PARAGRAPH_SECTION;
static void SetStyle( TextContext* tContext,
                ParagraphContext* pContext, Int16 style )
    PARAGRAPH_SECTION;
static void StartItalic( TextContext* tContext,
    ParagraphContext* pContext ) PARAGRAPH_SECTION;
static void ContinueItalic( TextContext* tContext, 
    ParagraphContext* pContext ) PARAGRAPH_SECTION;
static void StopItalic( TextContext* tContext, 
    ParagraphContext* pContext ) PARAGRAPH_SECTION;
static void StartUnderline( void ) PARAGRAPH_SECTION;
static void ContinueUnderline( void ) PARAGRAPH_SECTION;
static void StopUnderline( void ) PARAGRAPH_SECTION;
static void StartStrike( TextContext* tContext ) PARAGRAPH_SECTION;
static void ContinueStrike( TextContext* tContext ) PARAGRAPH_SECTION;
static void StopStrike( TextContext* tContext,
                const Int16 height ) PARAGRAPH_SECTION;
static void ContinueForeColor( TextContext* tContext ) PARAGRAPH_SECTION;
static void DrawHorizontalRule( TextContext* tContext, Int16 height,
                Int16 width ) PARAGRAPH_SECTION;
static Int16 GetParagraphHeight( Int16 paragraphNum) PARAGRAPH_SECTION;
static void InitializeParagraphContext( ParagraphContext* pContext,
                Paragraph* paragraph, Header* record, Int16* paragraphNumPtr )
                PARAGRAPH_SECTION;
static FunctionType HandleFunction( ParagraphContext* pContext,
                TextContext* tContext, Int16* width ) PARAGRAPH_SECTION;
static TokenType GetNextToken( ParagraphContext* pContext,
                    WChar* nextToken ) PARAGRAPH_SECTION;
static void CalculateKerningMetrics ( void ) PARAGRAPH_SECTION;
static Boolean GetLineMetrics( ParagraphContext* pContext,
                TextContext* tContext,
                Boolean skipLeadingSpace, char** until,
                Int16* height ) PARAGRAPH_SECTION;
static Boolean PutNextToken( UInt16 token ) PARAGRAPH_SECTION;
static void WriteLine( ParagraphContext* pContext, TextContext* tContext,
                Char* until, Boolean skipLeadingSpace, Int16 paragraphNum ) 
                PARAGRAPH_SECTION;
static void QuickNoDrawLine( ParagraphContext* pContext, TextContext* tContext,
                Char* until ) PARAGRAPH_SECTION;
static void PushFunction( UInt8 code, FunctionHandler action )
    PARAGRAPH_SECTION;
static ParagraphFunction* PopFunction() PARAGRAPH_SECTION;
static FunctionType DoAnchorBegin( ParagraphContext* pContext,
                        TextContext* tContext, Int16* width ) PARAGRAPH_SECTION;
static FunctionType DoMultiImage( ParagraphContext* pContext,
                        TextContext* tContext, Int16* width ) PARAGRAPH_SECTION;
static FunctionType DoNamedAnchor( ParagraphContext* pContext,
                        TextContext* tContext, Int16* width ) PARAGRAPH_SECTION;
static FunctionType DoAnchorEnd( ParagraphContext* pContext,
                        TextContext* tContext, Int16* width ) PARAGRAPH_SECTION;
static FunctionType DoSetStyle( ParagraphContext* pContext,
                        TextContext* tContext, Int16* width ) PARAGRAPH_SECTION;
static FunctionType DoInlinedImage( ParagraphContext* pContext,
                        TextContext* tContext, Int16* width ) PARAGRAPH_SECTION;
static FunctionType DoSetMargin( ParagraphContext* pContext,
                        TextContext* tContext, Int16* width ) PARAGRAPH_SECTION;
static FunctionType DoAlignment( ParagraphContext* pContext,
                        TextContext* tContext, Int16* width ) PARAGRAPH_SECTION;
static FunctionType DoNewLine( ParagraphContext* pContext,
                        TextContext* tContext, Int16* width ) PARAGRAPH_SECTION;
static FunctionType DoHorizontalRule( ParagraphContext* pContext,
                        TextContext* tContext, Int16* width ) PARAGRAPH_SECTION;
static FunctionType DoItalicBegin( ParagraphContext* pContext,
                        TextContext* tContext, Int16* width ) PARAGRAPH_SECTION;
static FunctionType DoItalicEnd( ParagraphContext* pContext,
                        TextContext* tContext, Int16* width ) PARAGRAPH_SECTION;
static FunctionType DoUnderlineBegin( ParagraphContext* pContext,
                        TextContext* tContext, Int16* width ) PARAGRAPH_SECTION;
static FunctionType DoUnderlineEnd( ParagraphContext* pContext,
                        TextContext* tContext, Int16* width ) PARAGRAPH_SECTION;
static FunctionType DoStrikeBegin( ParagraphContext* pContext,
                        TextContext* tContext, Int16* width ) PARAGRAPH_SECTION;
static FunctionType DoStrikeEnd( ParagraphContext* pContext,
                        TextContext* tContext, Int16* width ) PARAGRAPH_SECTION;
static FunctionType DoSetForeColor( ParagraphContext* pContext,
                        TextContext* tContext, Int16* width ) PARAGRAPH_SECTION;
static FunctionType DoUnicode16( ParagraphContext* pContext,
                        TextContext* tContext, Int16* width ) PARAGRAPH_SECTION;
static FunctionType DoUnicode32( ParagraphContext* pContext,
                        TextContext* tContext, Int16* width ) PARAGRAPH_SECTION;
static FunctionType DoNoAction( ParagraphContext* pContext,
                        TextContext* tContext, Int16* width ) PARAGRAPH_SECTION;
static FunctionType DoCopyModeAnchorBegin( ParagraphContext* pContext,
                        TextContext* tContext, Int16* width ) PARAGRAPH_SECTION;
static FunctionType DoCopyModeAnchorEnd( ParagraphContext* pContext,
                        TextContext* tContext, Int16* width ) PARAGRAPH_SECTION;
static FunctionType DoCopyModeNewLine( ParagraphContext* pContext,
                        TextContext* tContext, Int16* width ) PARAGRAPH_SECTION;
static FunctionType DoCopyModeHorizontalRule( ParagraphContext* pContext,
                        TextContext* tContext, Int16* width ) PARAGRAPH_SECTION;
static FunctionType DoTable( ParagraphContext* pContext,
                        TextContext* tContext, Int16* width ) PARAGRAPH_SECTION;
static void WriteLinkList( void ) PARAGRAPH_SECTION;
static void SetDrawFunction( WriteModeType writeMode ) PARAGRAPH_SECTION;
static void DrawItalic(RectangleType *bounds) PARAGRAPH_SECTION;
static Int16 FindLink(UInt16 reference) PARAGRAPH_SECTION;
static FunctionType HandleUnicode( ParagraphContext* pContext,
#ifdef HAVE_IMODE
                        TextContext* tContext, Int16* width,
#endif
                        UnicodeType type ) PARAGRAPH_SECTION;
static void ContinueAnchor(TextContext* tContext,
                ParagraphContext* pContext) PARAGRAPH_SECTION;
static void StopAnchor( TextContext* tContext, UInt16 height) PARAGRAPH_SECTION;
static void ForceDefaultColor( TextContext* tContext, Boolean haveLink )
                        PARAGRAPH_SECTION;
static Boolean IsSoftHyphen( WChar token ) PARAGRAPH_SECTION;
#ifdef SUPPORT_WORD_LOOKUP
static Boolean IsLaxAlphaNum( WChar c ) PARAGRAPH_SECTION;
#endif


#define StoreString( str ) StoreChars(str, StrLen(str) )


/***********************************************************************
 *
 *      Private variables
 *
 ***********************************************************************/

/* list of supported paragraph functions */
/* extra 4 slots for replacements */
/* it might be better to use List in list.c */
static Int16 numFunctionCodes = 21;
static ParagraphFunction ListOfFunctionCodes[ 21 + 4 ] = {
    { 0x00, DoNoAction },
    { 0x0A, DoAnchorBegin },
    { 0x08, DoAnchorEnd },
    { 0x0C, DoNamedAnchor },
    { 0x11, DoSetStyle },
    { 0x1A, DoInlinedImage },
    { 0x22, DoSetMargin },
    { 0x29, DoAlignment },
    { 0x33, DoHorizontalRule },
    { 0x38, DoNewLine },
    { 0x40, DoItalicBegin },
    { 0x48, DoItalicEnd },
    { 0x53, DoSetForeColor },
    { 0x5C, DoMultiImage },
    { 0x60, DoUnderlineBegin },
    { 0x68, DoUnderlineEnd },
    { 0x70, DoStrikeBegin },
    { 0x78, DoStrikeEnd },
    { 0x83, DoUnicode16 },
    { 0x85, DoUnicode32 },
    { 0x92, DoTable }
};

/* variables for link list*/
static Int16  nlink = 0;
static UInt16 links[ 100 ];
static Int16  refNo;
/* variable for hardcopy */
static char* bufPtr;
static Int16 bufRemain;

/* Used to keep track of multi-line formatting */
static MultilineData multiline;

/* Used to know if margins should be added to the
   current line or only the following lines */
static Boolean addMarginToCurrent;

/* subscript adjustment */
static Int16   subHeight;

/* Used to keep track of invisible dashes at the end of a line */
static Boolean dashAtEOL;

/* Is there a linebreak after this line? */
static Boolean lineBreak;

/* Used to keep track of the search pattern */
static Int16   findPatternPos = 0;
static Int16   findPatternLen = -1;
static Int16   findParagraphNum = -1;
static Boolean nowSearching;

#ifdef SUPPORT_WORD_LOOKUP
static WChar         selectedWordBuffer[ MAX_WORD_LENGTH ];
static RectangleType selectedWordBounds[ MAX_WORD_LENGTH ];
static Int16         selectedWordOffset;
static Int16         selectedWordLength;
static Boolean       selectedWordDone;
static Char          selectedWord[ MAX_WORD_STRING_LENGTH ];
#endif



/* Used for full justification */
static Int16 spaceCount;    /* How many spaces in current line */
static Int16 bigSpace;      /* How many get 1 extra */
static Int16 littleSpace;   /* Extra pixels in each */

/* A one-character pushback for character tokens */
static Char  pushedChar     = 0;

/* Used to see if the current font is the fixed with font */
static Boolean fixedWidthFont = false;

/* For caching the last line for faster autoscrolling */
static Boolean lineCacheState  = false;   /* is the last line caching on? */
static Boolean inCache         = false;   /* is there anything cached? */
static UInt16  cachedParagraph = 0;       /* which paragraph is the cached stuff in? */
static LineCacheEntry lineCache;

/* Are we writing within the visible area? */
static Boolean visiblePosition;

static Coord minLeftKerning;
static Coord maxRightOverhang;



#ifdef SUPPORT_WORD_LOOKUP
void GetSelectedWordChars
     (
     Char* chars,
     Int16 len,
     const TextContext* tContext
     )
{
    YOffset yPos;
    Int16   height;

    if ( selectedWordDone )
        return;

    /* Get current y position for displayed text */
    height = FntCharHeight();
    yPos   = tContext->cursorY - height;

    if ( GetCurrentStyle() == SUBSTYLE )
        yPos += height / 2;
    else if ( GetCurrentStyle() == SUPSTYLE )
        yPos += height / 2 - GetPrevFontHeight();

    /* Is the tap inside the displayed line? */
    if ( yPos <= tContext->findPixelY &&
         tContext->findPixelY < yPos + height ) {
        UInt16 offset;
        Coord  charWidth;
        Coord  x;

        x      = tContext->cursorX;
        offset = 0;

        /* Iterate through the buffer to locate the word at the
           tapped position */
        while ( offset < len ) {
            WChar ch;

            offset   += TxtGlueGetNextChar( chars, offset, &ch );
            charWidth = TxtGlueCharWidth( ch );
            if ( CharIsSpace( ch ) ) {
                x += charWidth;
                /* If at end of a word, i.e. we found a space, and the x
                   coordinate for the current token is beyond the tapped
                   x position then we are done. */
                if ( tContext->findPixelX < x ) {
                    selectedWordDone  = true;
                    break;
                }
                /* Otherwise, given a space
                   we can restart looking for the selected word. */
                selectedWordOffset = -1;
                selectedWordLength = 0;
                continue;
            }
            /* Save current character and its boundaries */
            if ( selectedWordLength < MAX_WORD_LENGTH ) {
                selectedWordBounds[ selectedWordLength ].topLeft.x  = x;
                selectedWordBounds[ selectedWordLength ].topLeft.y  = yPos;
                selectedWordBounds[ selectedWordLength ].extent.x   = charWidth;
                selectedWordBounds[ selectedWordLength ].extent.y   = height;
                selectedWordBuffer[ selectedWordLength++ ] = ch;
            }
            /* Found the character at the tapped position. Store offset to
               character in the selected word; it's used to trim off any
               punctuation before and after this position. */
            if ( x <= tContext->findPixelX &&
                 tContext->findPixelX < x + charWidth ) {
                selectedWordOffset = selectedWordLength - 1;
            }
            x += charWidth;
        }
    }
}




/* This is a TxtCharIsAlpha() || TxtCharIsDigit() version which is more lax,
   and hopefully thus works with text in more encodings */
static Boolean IsLaxAlphaNum
    (
    WChar c
    )
{
    if ( c == selectedWordBuffer[ selectedWordOffset ] || IsSoftHyphen( c ) )
        return true;
    if ( ( UInt16 ) c < 128 )
        return TxtGlueCharIsAlpha( c ) || TxtGlueCharIsDigit( c );
    else
        return ! TxtGlueCharIsPunct( c ) && ! CharIsSpace( c );
}



void SelectedWordReset( void )
{
    selectedWordOffset  = -1;
    selectedWordLength  = 0;
    selectedWordDone    = false;
}



/* Done looking for selected word? */
Boolean SelectedWordDone( void )
{
    return selectedWordDone;
}



/* Find the word containing the tap position */
/* Trim off any punctuation.  Note that the character
   tapped never counts as punctuation wherever it occurs. */
Char* GetSelectedWord
    (
    RectangleType* bounds
    )
{
    Int16 start;
    Int16 i;
    Int16 stringSize;
    Coord topY;
    Coord bottomY;
    if ( selectedWordOffset < 0 ) {
        return NULL;
    }
    /* Find complete word */
    for ( i = selectedWordOffset - 1 ; 0 <= i ; i -- ) {
        if ( ! IsLaxAlphaNum( selectedWordBuffer[ i ] ) )
            break;
    }
    start = i + 1;
    stringSize = 0;
    topY       = 32000;
    bottomY    = -32000;
    for ( i = start;
          i < selectedWordLength &&
          stringSize < MAX_WORD_STRING_LENGTH ;
          i++ ) {
        Int16 charSize;
        WChar ch;

        ch = selectedWordBuffer[ i ];

        if ( IsSoftHyphen( ch ) )
            continue;
        charSize = TxtGlueCharSize( ch );
        if ( MAX_WORD_STRING_LENGTH <= charSize + stringSize ||
             ! IsLaxAlphaNum( ch ) )
            break;
        if ( selectedWordBounds[ i ].topLeft.y < topY )
            topY = selectedWordBounds[ i ].topLeft.y;
        if ( bottomY < selectedWordBounds[ i ].topLeft.y +
                                selectedWordBounds[ i ].extent.y )
            bottomY = selectedWordBounds[ i ].topLeft.y +
                          selectedWordBounds[ i ].extent.y;
        stringSize += TxtGlueSetNextChar( selectedWord, stringSize, ch );
    }
    selectedWord[ stringSize ] = '\0';
    if ( bounds != NULL ) {
        bounds->topLeft.x = selectedWordBounds[ start ].topLeft.x;
        bounds->topLeft.y = topY;
        bounds->extent.x  = selectedWordBounds[ i - 1 ].topLeft.x +
                                selectedWordBounds[ i - 1 ].extent.x -
                                bounds->topLeft.x;
        bounds->extent.y  = bottomY - topY;
        RotateRectangleInPlace( bounds );
    }
    return selectedWord;
}
#endif



/* if we are to force a default color, we do it here */
static void ForceDefaultColor
       (
       TextContext* tContext,
       Boolean      inAnchor    /* are we in an anchor? */
       )
{
    if ( tContext != NULL && Prefs()->forceDefaultColors &&
         1 < Prefs()->screenDepth ) {
        tContext->foreColor.r   = 0;
        tContext->foreColor.g   = 0;
        tContext->foreColor.b   = inAnchor ? 255 : 0;
        SetForeColor( tContext );
        multiline.ForeColor = true;
    }
}



/* Check if token is soft-hyphen. This can be disabled by the user,
   since some languages (e.g. Thai) uses the exact same value for
   a normal character */
static Boolean IsSoftHyphen
    (
    WChar token
    )
{
    return ( token == SOFT_HYPHEN ) && Prefs()->enableSoftHyphens &&
               DeviceUses8BitChars();
}



/* get position of pattern to find */
Int16 GetFindPatternPos( void )
{
    return findPatternPos;
}



/* insert data into last line cache */
static void SaveLineCacheEntry
    (
    ParagraphContext *p,
    TextContext      *t
    )
{
    MemMove( &( lineCache.pContext ), p, sizeof *p );
    MemMove( &( lineCache.tContext ), t, sizeof *t );
    MemMove( &( lineCache.multiline ), &multiline, sizeof multiline );
    lineCache.style          = GetCurrentStyle();
    lineCache.prevFontHeight = GetPrevFontHeight();
    lineCache.fixedWidthFont = fixedWidthFont;
    lineCache.lineBreak      = lineBreak;
}



/* clear last line cache */
void LineCacheClear( void )
{
    inCache = false;
}



/* Get state of the autoscroll line cache */
Boolean IsLineCacheActive( void )
{
    return lineCacheState;
}


/* start up line cache */
void LineCacheActivate( void )
{
    lineCacheState = true;
    LineCacheClear();
}



/* stop line cache */
void LineCacheDeactivate( void )
{
    lineCacheState = false;
    LineCacheClear();
}



/* grab data from last line cache */
static void RestoreLineCacheEntry
    (
    ParagraphContext *p,
    TextContext *t
    )
{
    MemMove( p, &( lineCache.pContext ), sizeof *p );
    MemMove( t, &( lineCache.tContext ), sizeof *t );
    MemMove( &multiline, &( lineCache.multiline ), sizeof multiline );
    SetMainStyleFont( lineCache.style );
    SetPrevFontHeight( lineCache.prevFontHeight );
    fixedWidthFont = lineCache.fixedWidthFont;
    lineBreak      = lineCache.lineBreak;
}



/* Fetch paragraph number from last line cache, returning zero if nothing is cached or
   if paragraph #0 is in the cache.  The returned value tells us the first paragraph
   that needs to be rendered given the current line cache. */
UInt16 GetLineCacheNeedToRenderFrom( void )
{
    return inCache ? cachedParagraph : 0;
}



/* this is called when things are being scrolled */
void LineCacheScrollUp
    (
    Int16 adj
    )
{
    if ( inCache )
        lineCache.tContext.cursorY -= adj;
}



/* Refresh data about the current screen, such as anchors, that might be out of date due
   to the line cache. */
void LineCacheRefreshCurrentScreenData( void )
{
    if ( IsLineCacheActive() ) {
        LineCacheClear();
        ViewRecord( GetCurrentRecordId(), FROM_HISTORY, NO_OFFSET, NO_OFFSET,
            WRITEMODE_LAYOUT_ANCHORS );
    }
}



/* Set position and length of find pattern */
void SetFindPatternData
    (
    const Int16 pos,    /* position of find pattern */
    const Int16 len     /* length of find text in bytes */
    )
{
    findPatternPos    = pos;
    SetSearchPosition( findPatternPos + 1 );
    findPatternLen    = len;
    findParagraphNum  = -1;
}



/* Clear position and length of find pattern */
void ClearFindPatternData( void )
{
    findPatternPos    = -1;
    findPatternLen    = -1;
    findParagraphNum  = -1;
}




/* Find offset to paragraph in text stream, and get the
   paragraph number. */
static Int16 GetParagraphOffset
    (
    Header*     record,     /* pointer to record */
    Paragraph*  paragraph,  /* pointer to paragraph */
    Int16*      paragraphNumPtr /* pointer to paragraph number, NULL 
                                   if not needed */
    )
{
    Paragraph*  p;
    Int16       pOffset;

    pOffset = 0;
    p       = GET_PARAGRAPH( record, 0 );
    if ( paragraphNumPtr != NULL )
        *paragraphNumPtr = paragraph - p;
    while ( p < paragraph ) {
        pOffset += p->size;
        p++;
    }
    return pOffset;
}



/* Align text/image */
static void AlignText
    (
    TextContext*        tContext,   /* pointer to text context */
    ParagraphContext*   pContext    /* pointer to paragraph context */
    )
{
    Int16 diff;

    bigSpace    = 0;
    littleSpace = 0;

    diff = pContext->maxPixels - pContext->linePixels;
    if ( diff <= 0 )
        return;

    if ( pContext->type == ALIGNMENT_CENTER ) {
        tContext->cursorX += diff / 2;
    }
    else if ( pContext->type == ALIGNMENT_RIGHT ) {
        tContext->cursorX += diff;
    }
    else if ( pContext->type == ALIGNMENT_JUSTIFY && ! multiline.Anchor &&
              ! lineBreak && 0 < spaceCount && 
              diff / spaceCount < pContext->maxPixels / 4 ) {
        littleSpace = diff / spaceCount;    /* each space gets pixels */
        bigSpace    = diff % spaceCount;    /* this many get 1 extra */
    }
}



/* Check if given character is a non-visible character */
static Boolean CharIsSpace
    (
    WChar c
    )
{
    return ( ( 0 <= c ) && ( c <= ' ' ) );
}



/* Set new font style. */
static void SetStyle
    (
    TextContext*        tContext,   /* pointer to text context */
    ParagraphContext*   pContext,   /* pointer to paragraph context */
    Int16               style       /* new font style */
    )
{
    if ( MINSTYLES <= style && style < MAXSTYLES ) {
        Boolean  italic;

        italic = pContext->italic;
        if ( italic )
            StopItalic( tContext, pContext );

        if ( style == FIXEDSTYLE )
            fixedWidthFont = true;
        else
            fixedWidthFont = false;

        SetMainStyleFont( style );

        if ( italic )
            StartItalic( tContext, pContext );

        pContext->fontHeight = FontLineHeight();
        pContext->style      = style;
    }
}



static void ContinueAnchor
    (
    TextContext*      tContext,
    ParagraphContext* pContext
    )
{
    if ( multiline.Anchor ) {
        Int16 tempX;

        tempX = tContext->cursorX;

        AlignText( tContext, pContext );
        AnchorContinue( tContext );

        tContext->activeAnchor = true;
        tContext->cursorX      = tempX;

        ForceDefaultColor( tContext, true );
    }
}



static void StopAnchor
    (
    TextContext* tContext,
    UInt16 height
    )
{
    if ( multiline.Anchor )
        AnchorStop( tContext, height );
    ForceDefaultColor( tContext, false );
}



/* Draw block of italic text */
static void DrawItalic
    (
    RectangleType* bounds /* bounds for block of text */
    )
{
    /* Move upper half of text 1 pixel to the right -> italic font */
    RotScrollRectangle( bounds, winRight, 1, bounds );
    RotEraseRectangle( bounds, 0 );
}



/* Initialize a new block of italic text */
static void StartItalic
    (
    TextContext*      tContext,    /* tContext - pointer to text context */
    ParagraphContext* pContext
    )
{
    if ( ! SetFontItalic() && tContext != NULL ) {
        tContext->italic.topLeft.x = tContext->cursorX;
        GrayFntSetBackgroundErase( true );
    }
    pContext->fontHeight = FontLineHeight();
    pContext->italic     = true;
}



/* Handle several lines with italic text */
static void ContinueItalic
    (
    TextContext*      tContext,  /* pointer to text context */
    ParagraphContext* pContext
    )
{
    if ( multiline.Italic ) {
        StartItalic( tContext, pContext );
    }
}



/* Mark the end of a block of italic text */
static void StopItalic
    (
    TextContext*      tContext,  /* pointer to text context */
    ParagraphContext* pContext
    )
{
    if ( ! EndFontItalic() && tContext != NULL &&
         multiline.Italic && tContext->writeMode == WRITEMODE_DRAW_CHAR ) {
        Int16 height;
        height = FntCharHeight();
        tContext->italic.topLeft.y  = tContext->cursorY - height;
        tContext->italic.extent.x   = tContext->cursorX -
                                      tContext->italic.topLeft.x + 1;
        tContext->italic.extent.y   = height / 2 + 1;

        if ( 1 < tContext->italic.extent.x )
            DrawItalic( &tContext->italic );
        GrayFntSetBackgroundErase( false );
    }
    pContext->fontHeight = FontLineHeight();
    pContext->italic     = false;
}



/* Initialize a new block of underlined text */
static void StartUnderline( void )
{
    if ( Prefs()->underlineMode )
        WinSetUnderlineMode( solidUnderline );
}



/* Handle several lines with underlined text */
static void ContinueUnderline( void )
{
    if ( multiline.Underline )
        StartUnderline();
}



/* Mark the end of a block of underlined text */
static void StopUnderline( void )
{
    if ( multiline.Underline && Prefs()->underlineMode )
        WinSetUnderlineMode( noUnderline );
}



/* Initialize a new block of strikethrough text */
static void StartStrike
    (
    TextContext* tContext   /* pointer to text context */
    )
{
    tContext->strike.topLeft.x = tContext->cursorX - 1;
}



/* Handle several lines with strikethrough text */
static void ContinueStrike
    (
    TextContext* tContext   /* pointer to text context */
    )
{
    if ( multiline.Strike )
        StartStrike( tContext );
}



/* Mark the end of a block of strikethrough text */
static void StopStrike
    (
    TextContext*    tContext,   /* pointer to text context */
    const Int16     height      /* height of line */
    )
{
    if ( multiline.Strike && tContext->writeMode == WRITEMODE_DRAW_CHAR ) {
        tContext->strike.topLeft.y  = tContext->cursorY - height;
        tContext->strike.extent.x   = tContext->cursorX -
                                      tContext->strike.topLeft.x - 1;
        tContext->strike.extent.y   = height - 1;

        StrikeThrough( &tContext->strike );
    }
}



/* Handle several lines with ForeColor */
static void ContinueForeColor
    (
    TextContext* tContext   /* pointer to text context */
    )
{
    if ( multiline.ForeColor )
        SetForeColor( tContext );
}



/* Draw a horizontal rule */
static void DrawHorizontalRule
    (
    TextContext*    tContext,   /* pointer to text context */
    Int16           height,     /* height of horizontal rule */
    Int16           width       /* width of horizontal rule */
    )
{
    if ( tContext->writeMode == WRITEMODE_DRAW_CHAR ) {
        RectangleType hr;

        hr.topLeft.x    = tContext->cursorX;
        hr.topLeft.y    = tContext->cursorY - height;
        hr.extent.x     = width;
        hr.extent.y     = height;

        RotDrawRectangle( &hr, 0 );
    }
}



/* Get the paragraph height data */
static Int16 GetParagraphHeight
    (
    Int16 paragraphNum /* paragraph number */
    )
{
    MetaRecord*  meta;

    Int16        height;

    meta = MemHandleLock( GetMetaRecord() );

    if ( meta->height == 0 )
        height = 0;
    else
        height = GET_METAPARAGRAPH( meta, paragraphNum )->height;

    MemHandleUnlock( GetMetaRecord() );

    return height; 
}



/* Initialize paragraph context with data from a paragraph */
static void InitializeParagraphContext
    (
    ParagraphContext*   pContext,    /* pointer to paragraph context */
    Paragraph*          paragraph,   /* pointer to paragraph */
    Header*             record,      /* pointer to record */
    Int16*              paragraphNumPtr /* pointer to paragraph number */
    )
{
    if ( record->type == DATATYPE_PHTML_COMPRESSED ) {
        MemHandle h = GetUncompressTextHandle();
        pContext->recordTop = MemHandleLock( h );
    } else
        pContext->recordTop = (Char *)GET_DATA( record );

    pContext->position      = pContext->recordTop +
                                  GetParagraphOffset( record, paragraph,
                                      paragraphNumPtr );
    pContext->last          = pContext->position + paragraph->size;
    pContext->function      = NULL;
    pContext->left          = 0;
    pContext->right         = 0;
    pContext->maxPixels     = RotExtentX() + minLeftKerning - maxRightOverhang;
    pContext->linePixels    = 0;
    pContext->italic        = 0;
    if ( Prefs()->forceAlign == FORCE_ALIGN_NONE )
        pContext->type = ALIGNMENT_LEFT;
    else
        pContext->type = Prefs()->forceAlign - FORCE_ALIGN_LEFT +
                             ALIGNMENT_LEFT;
    pContext->style          = DEFAULTSTYLE;

    SetStyle( NULL, pContext, DEFAULTSTYLE );

    if ( record->type == DATATYPE_PHTML_COMPRESSED )
        MemHandleUnlock( GetUncompressTextHandle() );
}



/* Handle table */
static FunctionType DoTable
    (
    ParagraphContext*   pContext,
    TextContext*        tContext,
    Int16*              width
    )
{
    Coord   high;
    Coord   wide;
    Boolean goodTable;
    UInt8*  functionArgs;
    UInt16  reference;
    Char    name[20];
    Int16   length;

    functionArgs = pContext->function + 1;
    reference = *functionArgs * 256 + *( functionArgs + 1 );

    length = StrPrintF(name, "[TABLE %d]", reference);

    goodTable = GetTableSize(reference, &wide, &high);

    if ( tContext == NULL ) {
        if ( ! goodTable ) {
            *width = FntCharsWidth( name, length );
            pContext->fontHeight = FontLineHeight();
        }
        else if ( pContext->maxPixels < wide ) {
            *width = 30;
            pContext->fontHeight = 20;
        }
        else {
            *width = wide;
            pContext->fontHeight = high;
        }
    }
    else {
        if ( tContext->writeMode == WRITEMODE_COPY_CHAR || ! goodTable ) {
            DrawText( name, length, tContext );
            *width = FntCharsWidth( name, length );
        }
        else if ( pContext->maxPixels < wide ) {
            DoAnchorBegin( pContext, tContext, width );
            if ( tContext->writeMode == WRITEMODE_DRAW_CHAR )
                DrawIcon(tContext->cursorX, tContext->cursorY - 20);
            *width = 30;
            AnchorStopImage( tContext, pContext->fontHeight, *width );
            DoAnchorEnd( pContext, tContext, width );
        }
        else {
            if ( tContext->writeMode == WRITEMODE_DRAW_CHAR )
                InlineTable(reference, tContext->cursorX,
                            tContext->cursorY - high);
            *width = wide;
        }
    }

    return TABLE;
}



/* Handle anchor */
static FunctionType DoAnchorBegin
    (
    ParagraphContext*   pContext,
    TextContext*        tContext,
    Int16*              width
    )
{
    if ( tContext != NULL ) {
        UInt8*  functionArgs;
        UInt16  reference;

        tContext->activeAnchor  = true;
        multiline.Anchor        = true;

        functionArgs    = pContext->function + 1;
        reference       = *functionArgs * 256 + *( functionArgs + 1 );

        AnchorStart( tContext, reference, TOP_OFFSET );
        ForceDefaultColor( tContext, true );
    }
    return ANCHOR;
}



/* Handle name anchor */
static FunctionType DoNamedAnchor
    (
    ParagraphContext*   pContext,
    TextContext*        tContext,
    Int16*              width
    )
{
    if ( tContext != NULL ) {
        UInt8*  functionArgs;
        UInt16  reference;
        Int16   paragraphOffset;

        tContext->activeAnchor  = true;
        multiline.Anchor        = true;

        functionArgs    = pContext->function + 1;
        reference       = *functionArgs * 256 + *( functionArgs + 1 );
        paragraphOffset = *( functionArgs + 2 ) * 256 + *( functionArgs + 3 );

        AnchorStart( tContext, reference, paragraphOffset );
        ForceDefaultColor( tContext, true );
    }
    return ANCHOR;
}


/* Handle end of anchor data */
static FunctionType DoAnchorEnd
    (
    ParagraphContext*   pContext,
    TextContext*        tContext,
    Int16*              width
    )
{
    if ( tContext != NULL ) {
        tContext->activeAnchor  = false;
        multiline.Anchor         = false;
        AnchorStop( tContext, pContext->fontHeight );
        ForceDefaultColor( tContext, false );
    }
    return ANCHOR;
}



/* Handle image + anchor */
static FunctionType DoMultiImage
    (
    ParagraphContext*   pContext,
    TextContext*        tContext,
    Int16*              width
    )
{
    UInt8*  functionArgs;
    Int16   alternateImage;
    Int16   inlinedImage;

    functionArgs    = pContext->function + 1;
    alternateImage  = *functionArgs * 256 + *( functionArgs + 1 );
    inlinedImage    = *( functionArgs + 2 ) * 256 + *( functionArgs + 3 );

    if ( tContext == NULL ) {
        GetImageMetrics( inlinedImage, width, &pContext->fontHeight );
    }
    else if ( tContext->activeAnchor ) {
        /* if we have an active anchor the image should be "connected" to 
           that anchor; this will link the inlined image both to the
           referenced page and to the alternate image */
        AnchorAppendImage( tContext, pContext->fontHeight, alternateImage );
        DrawInlineImage( inlinedImage, tContext, width );
        AnchorStopImage( tContext, pContext->fontHeight, *width );
    }
    else {
        /* draw inlined image that links to the alternate image */
        AnchorStart( tContext, alternateImage, TOP_OFFSET );
        DrawInlineImage( inlinedImage, tContext, width );
        AnchorStopImage( tContext, pContext->fontHeight, *width );
        AnchorStop( tContext, pContext->fontHeight );
    }
    return IMAGE;
}



/* Include inlined image */
static FunctionType DoInlinedImage
    (
    ParagraphContext*   pContext,
    TextContext*        tContext,
    Int16*              width
    )
{
    UInt8* functionArgs;
    Int16  inlinedImage;
    
    functionArgs    = pContext->function + 1;
    inlinedImage    = *functionArgs * 256 + *( functionArgs + 1 );

    if ( tContext == NULL ) {
        GetImageMetrics( inlinedImage, width, &pContext->fontHeight );
    }
    else if ( tContext->activeAnchor ) {
        /* draw inlined image that links to the referenced page */
        RestartAnchor( tContext, pContext->fontHeight );
        DrawInlineImage( inlinedImage, tContext, width );
        AnchorStopImage( tContext, pContext->fontHeight, *width );
    }
    else {
        DrawInlineImage( inlinedImage, tContext, width );
    }
    return IMAGE;
}



/* Set new style */
static FunctionType DoSetStyle
    (
    ParagraphContext*   pContext,
    TextContext*        tContext,
    Int16*              width
    )
{
    UInt8* functionArgs;

    functionArgs = pContext->function + 1;

    SetStyle( tContext, pContext, (Int16) *functionArgs );

    return STYLE;
}



/* Add margin */
static FunctionType DoSetMargin
    (
    ParagraphContext*   pContext,
    TextContext*        tContext,
    Int16*              width
    )
{
    UInt8* functionArgs;
    Coord  left;
    Coord  right;

    functionArgs = pContext->function + 1;

    left  = *functionArgs;
    right = *( functionArgs + 1 );
    if ( addMarginToCurrent ) {
        *width = left - pContext->left;
        if ( tContext == NULL )
            *width += right - pContext->right;
    }
    pContext->left  = left;
    pContext->right = right;

    return MARGIN;
}



/* Set alignment */
static FunctionType DoAlignment
    (
    ParagraphContext*   pContext,
    TextContext*        tContext,
    Int16*              width
    )
{
    UInt8* functionArgs;

    functionArgs = pContext->function + 1;

    if ( tContext == NULL && Prefs()->forceAlign == FORCE_ALIGN_NONE )
        pContext->type = (AlignmentType) *functionArgs;

    return ALIGNMENT;
}



/* Add newline */
static FunctionType DoNewLine
    (
    ParagraphContext*   pContext,
    TextContext*        tContext,
    Int16*              width
    )
{
    return NEWLINE;
}



/* Include horizontal rule */
static FunctionType DoHorizontalRule
    (
    ParagraphContext*   pContext,
    TextContext*        tContext,
    Int16*              width
    )
{
    UInt8* functionArgs;
    Int16  hruleWidth;
    Int16  hruleWidthPercentage;

    functionArgs            = pContext->function + 1;
    pContext->fontHeight    = *functionArgs;
    hruleWidth              = *( functionArgs + 1 );
    hruleWidthPercentage    = *( functionArgs + 2 );

    /* Check values and assign default value if necessary */
    if ( pContext->fontHeight == 0 )
        pContext->fontHeight = DEFAULT_HRULE_SIZE;

    if ( hruleWidth != 0 )
        *width = hruleWidth;
    else if ( hruleWidthPercentage != 0 )
        *width = pContext->maxPixels * hruleWidthPercentage / 100;
    else
        *width = pContext->maxPixels;

    if ( tContext != NULL ) {
        DrawHorizontalRule( tContext, pContext->fontHeight, *width );

        /* Restore the current font height */
        pContext->fontHeight = FontLineHeight();
    }
    return HRULE;
}



/* Begin block of italics */
static FunctionType DoItalicBegin
    (
    ParagraphContext*   pContext,
    TextContext*        tContext,
    Int16*              width
    )
{
    if ( tContext != NULL ) {
        multiline.Italic = true;
    }
    StartItalic( tContext, pContext );
    return ITALIC;
}



/* End block of italics */
static FunctionType DoItalicEnd
    (
    ParagraphContext*   pContext,
    TextContext*        tContext,
    Int16*              width
    )
{
    StopItalic( tContext, pContext );
    if ( tContext != NULL ) {
        multiline.Italic = false;
    }
    return ITALIC;
}




/* Begin block of underlined text */
static FunctionType DoUnderlineBegin
    (
    ParagraphContext*   pContext,
    TextContext*        tContext,
    Int16*              width
    )
{
    if ( tContext != NULL ) {
        multiline.Underline = true;
        StartUnderline();
    }
    return UNDERLINE;
}



/* End block of underlined text */
static FunctionType DoUnderlineEnd
    (
    ParagraphContext*   pContext,
    TextContext*        tContext,
    Int16*              width
    )
{
    if ( tContext != NULL ) {
        StopUnderline();
        multiline.Underline = false;
    }
    return UNDERLINE;
}



/* Begin block of strikethrough text */
static FunctionType DoStrikeBegin
    (
    ParagraphContext*   pContext,
    TextContext*        tContext,
    Int16*              width
    )
{
    if ( tContext != NULL ) {
        multiline.Strike = true;
        StartStrike( tContext );
    }
    return STRIKETHROUGH;
}



/* End block of strikethrough text */
static FunctionType DoStrikeEnd
    (
    ParagraphContext*   pContext,
    TextContext*        tContext,
    Int16*              width
    )
{
    if ( tContext != NULL ) {
        StopStrike( tContext, pContext->fontHeight );
        multiline.Strike = false;
    }
    return STRIKETHROUGH;
}



/* Initialize a new block of ForeColor. Note there is no StopForeColor()
   needed, as the device saves the device's current colors before
   rendering page, and then restores those colors after page rendered.
   [See DrawRecord() in document.c] */
static FunctionType DoSetForeColor
    (
    ParagraphContext*   pContext,
    TextContext*        tContext,
    Int16*              width
    )
{
    /* If b&w screendepth, don't bother handling color */
    if ( tContext != NULL && 1 < Prefs()->screenDepth ) {
        if ( Prefs()->forceDefaultColors ) {
            ForceDefaultColor( tContext, tContext->activeAnchor );
        }
        else {
            UInt8* functionArgs;

            multiline.ForeColor = true;

            /* Parse out the function argument to get the red, green, blue
               UInt8 nunbers to make an RGB color type */
            functionArgs            = pContext->function + 1;
            tContext->foreColor.r   = (UInt8) *functionArgs;
            tContext->foreColor.g   = (UInt8) *( functionArgs + 1 );
            tContext->foreColor.b   = (UInt8) *( functionArgs + 2 );
            SetForeColor( tContext );
        }
    }
    return FORECOLOR;
}



static FunctionType HandleUnicode
    (
    ParagraphContext* pContext,
#ifdef HAVE_IMODE
    TextContext*      tContext,
    Int16*            width,
#endif
    UnicodeType type
    )
{
    UInt8*  functionArgs;
    UInt32  charValue;
    UInt8   charsToSkip;
    UInt16  palmChar;
#ifdef HAVE_IMODE
    DmOpenRef  plkrImodeDB;
#endif

    functionArgs    = pContext->function + 1;
    charsToSkip     = *functionArgs;
    charValue       = *( functionArgs + 1 ) * 256 + *( functionArgs + 2 );
    if ( type == UNICODE32 ) {
        charValue   = charValue * 65536 + *( functionArgs + 3 ) * 256 +
                      *( functionArgs + 4 );
    }

#ifdef HAVE_IMODE
    /* Check for Imode DB */
    plkrImodeDB = DmOpenDatabaseByTypeCreator( PlkrImodeType, ViewerAppID,
                    dmModeReadOnly );
    if ( plkrImodeDB != NULL ) {
        palmChar = charValue;
        /* The parser puts in SHIFT-JIS characters as unicode.
           We want to move these into the unicode range */
        if ( 63647 <= palmChar && palmChar <= 63740 ) {
            charValue = charValue - 4705;
        }
        if ( 63808 <= palmChar && palmChar <= 63870 ) {
            charValue = charValue - 4772;
        }
        if ( 63872 <= palmChar && palmChar <= 63920 ) {
             charValue = charValue - 4773;
        }
        palmChar = charValue - 58942;
        if ( palmChar < 206 ) {
             DisplayImode( plkrImodeDB, tContext, palmChar, width,
                &pContext->fontHeight );
             DmCloseDatabase( plkrImodeDB );
             pContext->position += charsToSkip;
             return IMAGE;
        }
    }
#endif

    palmChar = FindPalmCharForUnicodeChar( charValue );
    if ( 0 < palmChar && PutNextToken( palmChar ) ) {
        pContext->position += charsToSkip;
    }
    return UNICODE;
}



/* handle unicode chars */
static FunctionType DoUnicode16
    (
    ParagraphContext*   pContext,
    TextContext*        tContext,
    Int16*              width
    )
{
#ifdef HAVE_IMODE
    return HandleUnicode(pContext, tContext, width, UNICODE16);
#else
    return HandleUnicode(pContext, UNICODE16);
#endif
}



/* handle unicode chars */
static FunctionType DoUnicode32
    (
    ParagraphContext*   pContext,
    TextContext*        tContext,
    Int16*              width
    )
{
#ifdef HAVE_IMODE
    return HandleUnicode(pContext, tContext, width, UNICODE32);
#else
    return HandleUnicode(pContext, UNICODE32);
#endif
}



/* Do nothing */
static FunctionType DoNoAction
    (
    ParagraphContext*   pContext,
    TextContext*        tContext,
    Int16*              width
    )
{
    /* no action */
    return NONE;
}



/* Handle a function */
static FunctionType HandleFunction
    (
    ParagraphContext*   pContext,   /* pointer to paragraph context */
    TextContext*        tContext,   /* pointer to text context */
    Int16*              width       /* upon return, set to the width of a
                                       handled inline image or the size of
                                       a horizontal rule ( will be 0 for
                                       other function codes ) */
    )
{
    ParagraphFunction*  pFunc;
    UInt8               functionCode;
    Int16               i;

    *width          = 0;
    functionCode    = *pContext->function;
    pFunc           = ListOfFunctionCodes; /* first element == DoNoAction */

    for ( i = numFunctionCodes - 1; 0 <= i; i-- ) {
        pFunc = &ListOfFunctionCodes[i];
        if ( pFunc->code == functionCode )
            break;
    }

    return pFunc->doAction( pContext, tContext, width );
}



/* Get next token from paragraph context */
static TokenType GetNextToken
    (
    ParagraphContext*   pContext,   /* pointer to paragraph context */
    WChar*              nextToken   /* token value */
    )
{
    WChar   nextChar;
    Int16   offset;

    if ( pushedChar != 0 ) {
        *nextToken = ( UInt8 )pushedChar;
        pushedChar = 0;
        return TOKEN_CHARACTER;
    }

    if ( pContext->last <= pContext->position )
        return TOKEN_PARAGRAPH_END;

    pContext->position += TxtGlueGetNextChar( pContext->position, 0,
                            &nextChar );

    if ( nextChar != '\0' ) {
        *nextToken = nextChar;
        return TOKEN_CHARACTER;
    }
    pContext->function  = (UInt8*) pContext->position;
    offset              = 1 + ( *pContext->position & 0x07 );
    pContext->position += offset;

    return TOKEN_FUNCTION;
}



/* Put a single character back on the token stack. */
static Boolean PutNextToken
    (
    UInt16 nextToken
    )
{
    if ( pushedChar != 0 )
        return false;

    if ( 256 <= nextToken )
        return false;

    pushedChar = (Char) nextToken;

    return true;
}



/* Find the number of characters that will fit in a line and return
   true if the line is to be drawn. */
static Boolean GetLineMetrics
    (
    ParagraphContext*   pContext,           /* pointer to paragraph context */
    TextContext*        tContext,           /* pointer to text context */
    Boolean             skipLeadingSpace,   /* true if initial non-visible
                                               characters should be skipped */
    Char**              until,              /* ending position */
    Int16*              height              /* upon return, set to the height
                                               of the line */
    )
{
    Char*   startPosition;
    Boolean lastSpaceIsVisible;
    Boolean initialFixedWidthFont;
    FontID  initialFontStyle;
    Int16   initialLeftMargin;
    Int16   initialRightMargin;
    Boolean initialItalic;
    Int16   tokenCount;
    Int16   lastSpace;
    Int16   linePixels;
    Int16   lastSpacePixels;
    Int16   lastSpaceHeight;
    Int16   charWidth;
    UInt16  adjacentDashes;
    Int16   invisibleDash;
    Int16   lastInvisibleDash;
    Int16   invisibleWidth;
    Int16   marginAdjustment;
    Char*   prevPosition;
    Boolean useSavedPContext;
    Int16   savedSubHeight;
    ParagraphContext savedPContext;
    Boolean savedFixedWidthFont;
    Int16   initialPrevFontHeight;
    Int16   savedPrevFontHeight;
    Boolean measureOnly;

    pushedChar              = 0;

    subHeight               = 0;
    savedSubHeight          = 0;

    useSavedPContext        = false;

    startPosition           = pContext->position;
    initialFixedWidthFont   = fixedWidthFont;
    savedFixedWidthFont     = fixedWidthFont;
    initialFontStyle        = pContext->style;
    initialLeftMargin       = pContext->left;
    initialRightMargin      = pContext->right;
    initialItalic           = pContext->italic;
    initialPrevFontHeight   = GetPrevFontHeight();
    savedPrevFontHeight     = initialPrevFontHeight;

    addMarginToCurrent      = true;

    *height                 = 0;
    tokenCount              = 0;

    spaceCount              = 0;
    lastSpace               = 0;
    linePixels              = 0;
    lastSpacePixels         = 0;
    lastSpaceHeight         = 0;
    lastSpaceIsVisible      = false;
    adjacentDashes          = 0;
    lastInvisibleDash       = -1;
    invisibleDash           = -1;
    invisibleWidth          = 0;

    lineBreak               = false;

    if ( pContext->italic ) {
        SetFontItalic();
    }

    /*
       Predict whether we shall have to draw the line on the screen or
       just measure it.  Lines above the screen must actually be
       drawn to set multiline formatting, but they can be drawn quickly.
       If we are not in DO_LAYOUT( writeMode ) mode, however, we do not need 
       to draw any lines since we are only after height data, and height data 
       is not affected by multiline formatting (the current style is
       stored in pContext, not in tContext).
     */
    measureOnly = ! lineCacheState && ! nowSearching &&
                     ( tContext->writeMode == WRITEMODE_NO_DRAW ||
                       ( DO_LAYOUT_WHOLE_SCREEN( tContext->writeMode ) &&
                         RotTopLeftY() + RotExtentY() <= tContext->cursorY
                       )
#ifdef SUPPORT_WORD_LOOKUP
                    || ( tContext->writeMode == WRITEMODE_FIND_SELECTED_WORD &&
                         tContext->findPixelY < tContext->cursorY )
#endif
                     );
    if ( measureOnly )
        MemMove( &savedPContext, pContext, sizeof( ParagraphContext ) );

    for ( ;; ) {
        WChar       nextToken;
        TokenType   nextTokenType;

        prevPosition  = pContext->position;

        if ( DeviceUses8BitChars() &&
             *prevPosition != 0 && pushedChar == 0 &&
             prevPosition < pContext->last &&
             ! GrayFntIsCurrentGray()
           ) {
            nextToken     = (UInt8) *prevPosition;
            nextTokenType = TOKEN_CHARACTER;
            if ( ! CharIsSpace( nextToken ) && ! IsSoftHyphen( nextToken )) {
                Int16  offset;
                Int16  portionLength;
                Char*  src;
                Char*  dest;
                Char   portion[ TEXT_BUFFER_SIZE + 1 ];
                portionLength = pContext->last - prevPosition;
                if ( TEXT_BUFFER_SIZE < portionLength )
                    portionLength = TEXT_BUFFER_SIZE;

                /* copy up to portionLength from prevPosition to portion,
                   copying only text */
                src  = prevPosition;
                dest = portion;
                do {
                    *dest++ = *src++;
                    portionLength--;
                } while ( *src != 0 && 0 < portionLength );
                *dest = 0;

                /* call the OS for the wordwrapping--this is faster than
                   doing it ourselves */
                offset = FntWordWrap( portion,
                             pContext->maxPixels - linePixels );

                /* go back to last space or soft hyphen */
                while ( 0 < offset ) {
                    nextToken = ( UInt8 ) portion[ --offset ];
                    if ( IsSoftHyphen( nextToken ) ||
                         CharIsSpace( nextToken ) )
                        break;
                }
                /* skip over the spaces and soft hyphens */
                while ( 0 < offset ) {
                    nextToken = ( UInt8 ) portion[ --offset ];
                    if ( ! IsSoftHyphen( nextToken ) &&
                        ! CharIsSpace( nextToken ))
                       break;
                }
                /* update horizontal positions and so on */
                if ( 0 < offset ) {
                    Char *p;
                    linePixels += FntCharsWidth( portion, offset );
                    if ( pContext->style == SUBSTYLE &&
                         subHeight < GetPrevFontHeight() / 2
                       )
                        subHeight = GetPrevFontHeight() / 2;
                    prevPosition += offset;
                    p = portion;
                    while ( offset-- ) {
                        if ( *p == ' ' )
                            spaceCount++;
                        p++;
                    }
                }
            }
            pContext->position = prevPosition + 1;
        }
        else
            nextTokenType = GetNextToken( pContext, &nextToken );

        if ( nextTokenType == TOKEN_PARAGRAPH_END ) {
            break;
        }
        else if ( nextTokenType == TOKEN_FUNCTION ) {
            FunctionType funcType;

            funcType = HandleFunction( pContext, NULL, &charWidth );

            tokenCount++;
            if ( funcType == NEWLINE ) {
                lineBreak = true;
                break;
            }
            if ( funcType == HRULE ) {
                linePixels     += charWidth;
                *height         = pContext->fontHeight;
                break;
            }
            if ( funcType == IMAGE || funcType == TABLE ) {
                skipLeadingSpace = false;
                if ( ( ( linePixels + charWidth ) <= pContext->maxPixels ) ||
                     linePixels == 0 ) {
                    if ( *height < pContext->fontHeight )
                        *height = pContext->fontHeight;
                    lastSpaceIsVisible  = false;
                    lastSpacePixels     = linePixels + charWidth;
                    lastSpaceHeight     = *height;
                    lastSpace           = tokenCount;
                    if ( measureOnly ) {
                        MemMove( &savedPContext, pContext,
                            sizeof( ParagraphContext ) );
                        savedFixedWidthFont    = fixedWidthFont;
                        savedPrevFontHeight    = GetPrevFontHeight();
                    }
                    else
                        savedPContext.position = pContext->position;
                    savedSubHeight         = subHeight;
                }
                else {
                    tokenCount--;
                    pContext->position = prevPosition;
                    break;
                }
            }
            linePixels += charWidth;
            if ( pContext->maxPixels < linePixels )
                break;

            continue;
        }
        addMarginToCurrent = false;

        if ( skipLeadingSpace && CharIsSpace( nextToken ) && ! fixedWidthFont )
            continue;

        skipLeadingSpace = false;

        /* Count the spaces, leading ones are skipped above */
        if ( nextToken == ' ' ) {
            spaceCount++;
            lastSpaceIsVisible  = true;
            lastSpacePixels     = linePixels;
            lastSpaceHeight     = *height;
            lastSpace           = tokenCount;
            if ( measureOnly ) {
                MemMove( &savedPContext, pContext, sizeof( ParagraphContext ) );
                savedFixedWidthFont    = fixedWidthFont;
                savedPrevFontHeight    = GetPrevFontHeight();
            }
            savedPContext.position = prevPosition;
            savedSubHeight         = subHeight;

        /*
           The idea here is to treat multiple adjacent hyphenation characters
           as if they were a single long hyphen. If the line needs to wrap in
           the middle of this hyphen, we instead wrap on the "lastSpace"
           character. Note that we're looking for four different types of
           hyphens:

             ASCII 0x2D '-'  : the standard ASCII dash, or minus, character
             ASCII 0x96 '-'  : Windows CP1252 'n-dash' character
             ASCII 0x97 '--' : Windows CP1252 'm-dash' character
             ASCII 0xAD '­'  : in the HTML 4.0 list of character entities,
                               this is the 'soft' or 'discretionary' hyphen.
                               Also part of Latin-1.

           TODO: are there more types of hyphens and/or word-break characters
                 we should be looking for?
         */
        }
        else if ( nextToken == '-' || IsSoftHyphen( nextToken ) ||
                  nextToken == 0x96 || nextToken == 0x97 ) {
            adjacentDashes++;
        }
        else if ( adjacentDashes != 0 ) {
            if ( ! ( adjacentDashes == 1 && lastSpace == tokenCount - 2 ) ) {
                lastSpaceIsVisible  = false;
                lastSpacePixels     = linePixels;
                lastSpaceHeight     = *height;
                lastSpace           = tokenCount;
                lastInvisibleDash   = invisibleDash;
                if ( measureOnly ) {
                    MemMove( &savedPContext, pContext,
                        sizeof( ParagraphContext ) );
                    savedFixedWidthFont    = fixedWidthFont;
                    savedPrevFontHeight    = GetPrevFontHeight();
                }
                savedPContext.position = prevPosition;
                savedSubHeight = subHeight;
            }
            adjacentDashes = 0;
        }

        /*
           if the token is a 'soft' hyphen, then we treat it as invisible
           (non-printable).  We indicate that the last encountered invisible
           dash ends at the next token and set the charWidth to 0.  Otherwise,
           we set the charwidth to be the width of the current token.
         */
        if ( IsSoftHyphen( nextToken )) {
            charWidth       = 0;
            invisibleDash   = tokenCount + 1;
        }
        else {
            charWidth = TxtGlueCharWidth( nextToken );
        }

        /*
           if the lastSpace position (the last place where we could validly
           break the line) is the same as the lastInvisibleDash position,
           then we need to make sure we account for a '-' at the end of the
           line.  The 'invisibleWidth' variable will be 0 if the lastSpace
           was not an invisible dash, otherwise it will be the length of a
           hyphen character.
         */
        if ( ( lastSpace == lastInvisibleDash ) ||
             ( IsSoftHyphen( nextToken ))) {
            invisibleWidth = TxtGlueCharWidth( '-' );
        }
        else {
            invisibleWidth = 0;
        }

        /*
           we use the invisibleWidth to compute the "actual" line width,
           taking into account the possible existence of a soft hyphen
           at EOL.
         */
        if ( pContext->maxPixels <
             ( charWidth + linePixels + invisibleWidth ) ) {
            if ( ( pContext->maxPixels / 2 ) < lastSpacePixels ) {
                if ( lastSpaceIsVisible )
                    spaceCount--;
                tokenCount       = lastSpace;
                linePixels       = lastSpacePixels;
                *height          = lastSpaceHeight;
                invisibleDash    = lastInvisibleDash;
                useSavedPContext = true;

            /*
               If adjacentDashes is greater than 0, then we're in the middle
               of a series of adjacent hyphens, and we wrap instead of the
               previous space character. If there were more than 3 adjacent
               dashes, then we just break in the middle, wherever we were.
             */
            }
            else if ( 0 < adjacentDashes && adjacentDashes <= 3 ) {
                tokenCount       = lastSpace;
                linePixels       = lastSpacePixels;
                *height          = lastSpaceHeight;
                invisibleDash    = lastInvisibleDash;
                useSavedPContext = true;
            }
            else
                pContext->position = prevPosition;

            if ( lastSpace == lastInvisibleDash ) {
                invisibleWidth = TxtGlueCharWidth( '-' );
            }
            else {
                invisibleWidth = 0;
            }
            break;
        }
        linePixels += charWidth;

        pContext->fontHeight = FontLineHeight();
        if ( *height < pContext->fontHeight )
            *height = pContext->fontHeight;

        if ( pContext->style == SUBSTYLE && 0 < charWidth &&
             nextTokenType == TOKEN_CHARACTER &&
             subHeight < GetPrevFontHeight() / 2
           )
            subHeight = GetPrevFontHeight() / 2;

        tokenCount++;

        /* CRLF can be inserted anywhere after a multibyte char */
        if ( 1 < TxtGlueCharSize( nextToken ) ) {
            lastSpacePixels       = linePixels;
            lastSpace             = tokenCount;
            lastSpaceHeight       = *height;
            lastSpaceIsVisible    = false;
            if ( measureOnly ) {
                MemMove( &savedPContext, pContext,
                    sizeof( ParagraphContext ) );
                savedFixedWidthFont    = fixedWidthFont;
                savedPrevFontHeight    = GetPrevFontHeight();
            }
            else {
                savedPContext.position = pContext->position;
            }
            savedSubHeight         = subHeight;
        }
    }

    /* Line Spacing */
    *height += Prefs()->lineSpacing;

    pContext->fontHeight = *height;

    /*
       if the character at the end of the line (the character on which the
       line is being broken) is an invisible dash, then we set our 'dashAtEOL'
       flag to be true.  This flag is used in the WriteLine function (below).
     */

    if ( invisibleDash == tokenCount ) {
        dashAtEOL = true;
    }
    else {
        dashAtEOL = false;
    }

    marginAdjustment        = ( initialLeftMargin - pContext->left ) +
                              ( initialRightMargin - pContext->right );

    if ( useSavedPContext ) {
        subHeight = savedSubHeight;
        *until    = savedPContext.position;
    }
    else
        *until    = pContext->position;

    if ( *until == pContext->last )
         lineBreak = true;

    if ( measureOnly && useSavedPContext ) {
        MemMove( pContext, &savedPContext, sizeof( ParagraphContext ) );
        fixedWidthFont = savedFixedWidthFont;
    }

    pContext->linePixels    = linePixels + invisibleWidth + marginAdjustment;
    pContext->maxPixels    += marginAdjustment;

    if ( measureOnly ) {
        pContext->position  = *until;
        SetStyle( tContext, pContext, pContext->style );
        if ( useSavedPContext )
            SetPrevFontHeight( savedPrevFontHeight );
    }
    else {
        pContext->position      = startPosition;

        /* Restore font style and margins */
        SetStyle( tContext, pContext, initialFontStyle );
        SetPrevFontHeight( savedPrevFontHeight );
        fixedWidthFont   = initialFixedWidthFont;
        pContext->left   = initialLeftMargin;
        pContext->right  = initialRightMargin;
        pContext->italic = initialItalic;
    }
    if ( ! pContext->italic )
        EndFontItalic();
    /* Need to restore this after SetStyle(), for image and table anchors */
    pContext->fontHeight = *height;

    return measureOnly;
}



/* Draw characters from a paragraph onto the display */
static void WriteLine
    (
    ParagraphContext*   pContext,           /* pointer to paragraph context */
    TextContext*        tContext,           /* pointer to text context */
    Char*               until,              /* draw until this position */
    Boolean             skipLeadingSpace,   /* whether initial non-visible
                                               characters should be skipped */
    Int16               paragraphNum        /* current paragraph number */
    )
{
    const Char* findPatternStart  = pContext->recordTop + findPatternPos;
    const Char* findPatternStop   = findPatternStart + findPatternLen;

    Char      chars[ TEXT_BUFFER_SIZE ];
    Boolean   invertPattern;
    Int16     len;

    invertPattern   = false;
    len             = 0;

    addMarginToCurrent = true;

    AlignText( tContext, pContext );

    while ( pContext->position < until || pushedChar != 0 ) {
        TokenType nextTokenType;
        WChar     nextChar;
        Int16     imageWidth;
        Char*     prevPosition;

        prevPosition  = pContext->position;
        nextTokenType = GetNextToken( pContext, &nextChar );

        if ( nextTokenType == TOKEN_PARAGRAPH_END ) {
            break;
        }
        else if ( nextTokenType == TOKEN_FUNCTION ) {
            FunctionType funcType;

            if ( 0 < len ) {
                DrawText( chars, len, tContext );
                tContext->cursorX  += FntCharsWidth( chars, len );
                len                 = 0;
            }
            funcType = HandleFunction( pContext, tContext, &imageWidth );

            tContext->cursorX   += imageWidth;
            if ( funcType == IMAGE ) {
                skipLeadingSpace = false;
            }
            continue;
        }
        addMarginToCurrent = false;

        if ( skipLeadingSpace && CharIsSpace( nextChar ) && ! fixedWidthFont ) {
            continue;
        }

        skipLeadingSpace = false;

        if ( 0 <= findPatternPos && findPatternStart <= prevPosition &&
             prevPosition < findPatternStop && ! invertPattern ) {
            if ( 0 < len ) {
                DrawText( chars, len, tContext );
                tContext->cursorX  += FntCharsWidth( chars, len );
                len                 = 0;
            }
            if ( tContext->writeMode == WRITEMODE_DRAW_CHAR )
                DrawText = DrawInvertedChars;
            invertPattern           = true;
            findParagraphNum        = paragraphNum;
        }

        if ( invertPattern && findPatternStop <= prevPosition ) {
            DrawText( chars, len, tContext );
            if ( tContext->writeMode == WRITEMODE_DRAW_CHAR )
                DrawText = DrawChars;
            tContext->cursorX  += FntCharsWidth( chars, len );
            invertPattern       = false;
            len                 = 0;
        }

        /* ignore discretionary hyphens */
        if ( ! IsSoftHyphen( nextChar )) {
            Int16 charSize;
            charSize      = TxtGlueCharSize( nextChar );
            if ( TEXT_BUFFER_SIZE < len + charSize ) {
                DrawText( chars, len, tContext );
                tContext->cursorX += FntCharsWidth( chars, len );
                len = 0;
            }
            len          += TxtGlueSetNextChar( chars, len, nextChar );
        }

        if ( pContext->type == ALIGNMENT_JUSTIFY && nextChar == ' ' ) {
            if ( 0 < len ) {
                DrawText( chars, len, tContext );
                tContext->cursorX  += FntCharsWidth( chars, len );
                len                 = 0;
            }
            if ( bigSpace ) {
                bigSpace--;
                tContext->cursorX++;
            }
            tContext->cursorX += littleSpace;
#ifdef SUPPORT_WORD_LOOKUP
            if ( tContext->writeMode == WRITEMODE_FIND_SELECTED_WORD && 
                 ! selectedWordDone ) {
                selectedWordOffset = -1;
                selectedWordLength = 0;
            }
#endif
        }
    }

    /* if there is a dash at the end of the line, be sure to display one */
    if ( dashAtEOL ) {
      chars[ len++ ] = '-';
    }

    if ( 0 < len ) {
        DrawText( chars, len, tContext );
        tContext->cursorX += FntCharsWidth( chars, len );
    }
    if ( invertPattern && tContext->writeMode == WRITEMODE_DRAW_CHAR )
        DrawText = DrawChars;
}



/* Don't draw characters from a paragraph onto the display, but do update
   multiline stuff and act with respect to anchors and formatting as if
   we actually drew the paragraph. */
static void QuickNoDrawLine
    (
    ParagraphContext*   pContext,           /* pointer to paragraph context */
    TextContext*        tContext,           /* pointer to text context */
    Char*               until               /* draw until this position */
    )
{
    Boolean guaranteed8Bits;
    
    guaranteed8Bits = DeviceUses8BitChars();

    while ( pContext->position < until ) {
        TokenType nextTokenType;
        WChar     nextChar;
        Int16     imageWidth;

        nextTokenType = GetNextToken( pContext, &nextChar );
        if ( nextTokenType == TOKEN_PARAGRAPH_END )
            break;
        else if ( nextTokenType == TOKEN_FUNCTION )
            HandleFunction( pContext, tContext, &imageWidth );
        else if ( nextTokenType == TOKEN_CHARACTER && guaranteed8Bits ) {
            Char *p;
            p = pContext->position;
            while ( p < until && *p != 0 )
                p++;
            pContext->position = p;
        }
    }
}



/* Get the current kerning metrics */
static void CalculateKerningMetrics( void )
{
#if 0
    UInt16   i;
    Coord    leftKerning;
    Coord    rightOverhang;

    minLeftKerning   = 0;
    maxRightOverhang = 0;
    for ( i = 0 ; i < MAXSTYLES ; i++ ) {
         FontID  font;
         font          = GetMainStyleFont( i );
         leftKerning   = GrayFntMinLeftKerning( font );
         rightOverhang = GrayFntMaxRightOverhang( font );
         if ( leftKerning < minLeftKerning )
             minLeftKerning = leftKerning;
         if ( maxRightOverhang < rightOverhang )
             maxRightOverhang = rightOverhang;
    }
#endif
}




/* Draw a paragraph using the given text context */
void DrawParagraph
    (
    TextContext*    tContext,   /* pointer to text context */
    Paragraph*      paragraph,  /* pointer to paragraph */
    Header*         record      /* pointer to record */
    )
{
    ParagraphContext    pContext;
    Int16               prevPosition;
    Boolean             handleCaching;
    Boolean             measureOnly;
    Char*               until;
    Int16               paragraphHeight;
    YOffset             startY;
    Int16               paragraphNum;
    Coord               topLeftX;
    //Coord               extentX;
    Int16               height;

    CalculateKerningMetrics();
    topLeftX = RotTopLeftX() - minLeftKerning;
    //extentX  = RotExtentX() + minLeftKerning - maxRightOverhang;

    ASSERT( tContext != NULL && paragraph != NULL && record != NULL );

    InitializeParagraphContext( &pContext, paragraph, record, &paragraphNum );

    nowSearching = ( 0 <= findPatternLen &&
                     ( findParagraphNum < 0 || findParagraphNum == paragraphNum )
                   );

    measureOnly   = false;

    handleCaching = lineCacheState && tContext->writeMode == WRITEMODE_DRAW_CHAR;

/* FIXME: to remove this we must be able to assign a meta record
   for the test version or rewrite the code... */
#ifdef HAVE_PALMCUNIT
    paragraphHeight = 0;
#else
    paragraphHeight = GetParagraphHeight( paragraphNum );
#endif
    startY = tContext->cursorY;

    /* default is to display strings on the screen in write mode */
    SetDrawFunction( tContext->writeMode );

    RotOpenArmlet();

    tContext->cursorX     = topLeftX;

    GrayFntSetBackgroundErase( false );
    multiline.Anchor      = false;
    multiline.Italic      = false;
    multiline.Underline   = false;
    multiline.Strike      = false;
    multiline.ForeColor   = false;
    tContext->foreColor.r = 0;
    tContext->foreColor.g = 0;
    tContext->foreColor.b = 0;
    SetForeColor( tContext );

    /* Check for extra paragraph spacing */
    tContext->cursorY += ( paragraph->attributes & 0x07 ) *
                         Prefs()->paragraphSpacing;

    if ( record->type == DATATYPE_PHTML_COMPRESSED )
        MemHandleLock( GetUncompressTextHandle() );

    if ( inCache && cachedParagraph == paragraphNum )
        RestoreLineCacheEntry( &pContext, tContext ) ;

    if ( handleCaching ) {
        cachedParagraph = paragraphNum;
        inCache = true;
    }

    for ( ;; ) {
        Boolean wasItalic;

        prevPosition = pContext.position - pContext.recordTop;
        if ( handleCaching )
            SaveLineCacheEntry( &pContext, tContext );

        measureOnly = GetLineMetrics( &pContext, tContext, true, &until,
                          &height );

        if ( until <= pContext.recordTop + prevPosition )
            break;

        tContext->cursorY += height;

#ifdef SUPPORT_WORD_LOOKUP
        if ( tContext->writeMode == WRITEMODE_FIND_SELECTED_WORD ) {
            visiblePosition = ( tContext->cursorY - height <=
                                tContext->findPixelY );
        }
        else
#endif        
        if ( RotTopLeftY() < tContext->cursorY + subHeight ) {

            visiblePosition = ( tContext->cursorY - height <
                                RotTopLeftY() + RotExtentY() );

            if ( tContext->characterPosition < 0 )
                tContext->characterPosition = prevPosition;
        }
        else {
            visiblePosition = false;
        }

        if ( measureOnly ) {
            if ( prevPosition <= tContext->findCharacterPosition &&
                 tContext->findCharacterPosition < ( pContext.position -
                                                     pContext.recordTop )
               )
                tContext->foundYPosition = tContext->cursorY - height;
            tContext->cursorX     = topLeftX + pContext.left;
            tContext->cursorY    += subHeight;
            continue;
        }

        ContinueAnchor( tContext, &pContext);
        ContinueItalic( tContext, &pContext );
        ContinueUnderline();
        ContinueStrike( tContext );
        ContinueForeColor( tContext );

        /* We are now in a position to check whether we really need to
           draw the line, or just need to give it a quick scan for
           multiline formatting data.  Basically, at this point we
           just do the quick scan if the line is before the current
           screen (and we're not in the mode for searching for the
           current pattern or in line caching mode). */
        if ( DO_LAYOUT( tContext->writeMode ) &&
             ! visiblePosition && ! nowSearching && ! lineCacheState )
        {
            QuickNoDrawLine( &pContext, tContext, until );
        }
        else
        {
            WriteLine( &pContext, tContext, until, true, paragraphNum );
        }

        if ( prevPosition <= tContext->findCharacterPosition &&
             tContext->findCharacterPosition < ( pContext.position -
                                                 pContext.recordTop )
           )
            tContext->foundYPosition = tContext->cursorY - height;

        StopAnchor( tContext, height );
        wasItalic = pContext.italic;
        StopItalic( tContext, &pContext );
        pContext.italic = wasItalic;
        StopUnderline();
        StopStrike( tContext, height );

        tContext->cursorX   = topLeftX + pContext.left;
        pContext.fontHeight = FontLineHeight();
        tContext->cursorY  += subHeight;

#ifdef SUPPORT_WORD_LOOKUP
        if ( tContext->writeMode == WRITEMODE_FIND_SELECTED_WORD &&
             tContext->findPixelY < tContext->cursorY ) {
            selectedWordDone = true;
            break;
        }
#endif
        if ( RotTopLeftY() + RotExtentY() <= tContext->cursorY &&
             ( handleCaching || ( paragraphHeight != 0 &&
                                  DO_LAYOUT( tContext->writeMode ) &&
                                  ! nowSearching &&
                                  tContext->findCharacterPosition < 0 )
             )
           ) {
            tContext->cursorY = startY + paragraphHeight;
            break;
        }

        /* when copying the text to a memo we have to add a space for each
           line or the last word on the current line will be "connected" to
           the first word on the next line */
        if ( tContext->writeMode == WRITEMODE_COPY_CHAR )
            CopyChars( " ", 1, tContext );
    }

    /* append newline at the bottom of a paragraph when copying text to memo */
    if ( tContext->writeMode == WRITEMODE_COPY_CHAR )
        CopyChars( "\n", 1, tContext );

    if ( record->type == DATATYPE_PHTML_COMPRESSED )
        MemHandleUnlock( GetUncompressTextHandle() );

    GrayFntSetBackgroundErase( true );
    RotCloseArmlet();
}



void NoDrawChars( Char* chars, Int16 len, const TextContext* tContext )
{
}





void DrawChars( Char *chars, Int16 len, const TextContext *tContext )
{
    YOffset yPos;
    Int16   currentHeight;

    if ( ! visiblePosition )
        return;

    currentHeight = FntCharHeight();

    yPos = tContext->cursorY - currentHeight;

    if ( GetCurrentStyle() == SUBSTYLE )
        yPos += currentHeight / 2;
    else if ( GetCurrentStyle() == SUPSTYLE )
        yPos += currentHeight / 2 - GetPrevFontHeight();

    RotDrawChars(chars, len, tContext->cursorX, (Coord)yPos);
}



void DrawInvertedChars
    (
    Char* chars,
    Int16 len,
    const TextContext* tContext
    )
{
    RotDrawInvertedChars( chars, len, tContext->cursorX,
        tContext->cursorY - FntCharHeight() );
}



static void SetDrawFunction
    (
    WriteModeType writeMode
    )
{
    switch ( writeMode ){
    case WRITEMODE_NO_DRAW:
    case WRITEMODE_LAYOUT_ANCHORS:
        DrawText = NoDrawChars;
        break;

    case WRITEMODE_DRAW_CHAR:
        DrawText = DrawChars;
        break;

    case WRITEMODE_COPY_CHAR:
    case WRITEMODE_SERIAL_TEXT:
        DrawText = CopyChars;
        break;

#ifdef SUPPORT_WORD_LOOKUP
    case WRITEMODE_FIND_SELECTED_WORD:
        DrawText = GetSelectedWordChars;
        break;
#endif
    }
}



static void PushFunction
    (
    UInt8           code,
    FunctionHandler action
    )
{
    ListOfFunctionCodes[ numFunctionCodes ].code = code;
    ListOfFunctionCodes[ numFunctionCodes ].doAction = action;
    numFunctionCodes++;
}



static ParagraphFunction* PopFunction()
{
    numFunctionCodes --;
    return &ListOfFunctionCodes[ numFunctionCodes ];
}



void StartCopyMode()
{
    /* replace some functions in the table */
    PushFunction( 0x0A, DoCopyModeAnchorBegin );
    PushFunction( 0x08, DoCopyModeAnchorEnd );
    PushFunction( 0x33, DoCopyModeHorizontalRule );
    PushFunction( 0x38, DoCopyModeNewLine );
}



void StopCopyMode()
{
    /*append link list if offered*/
    if ( Prefs()->hardcopyLink == HARDCOPY_ATBOTTOM )
        WriteLinkList();

    /*restore functions*/
    PopFunction();
    PopFunction();
    PopFunction();
    PopFunction();
}



void SetCopyBuffer
    (
    char* buffer,
    Int16 bufLength
    )
{
    /*reset global variables*/
    /* reset link list */
    nlink = 0;
    /* set buffer */
    bufPtr = buffer;
    bufRemain = bufLength - 2;
}



Boolean InsideCopyRegion
    (
    const TextContext* tContext
    )
{
    if ( Prefs()->hardcopyRange == HARDCOPY_VISIBLEPARAGRAPHS )
        return true;
    else
        return visiblePosition;
}



/*Always Store*/
void StoreChars
    (
    Char* chars,
    Int16 len
    )
{
    if ( 0 < bufRemain ){
        if ( bufRemain < len ){
            /*it may split last multibyte char.*/
            /*TxtCharsBounds() should be used instead.*/
            len = bufRemain;
        }
        StrNCopy( bufPtr, chars, len );
        bufPtr += len;
        *bufPtr = '\0';        /*always terminate*/
        bufRemain -= len;
    }
}


/*Store chars if inside the copyarea*/
void CopyChars
    (
    Char* chars,
    Int16 len,
    const TextContext* tContext /*dummy, or NULL*/
    )
{
    /*if tContext is given (i.e. in the document body)*/
    if ( tContext != NULL )
        if ( InsideCopyRegion( tContext ) )
            StoreChars( chars, len );
}



/* Return the relative order of the link in the view */
static Int16 FindLink
    (
    UInt16 reference
    )
{
    UInt16 i;

    if ( reference == NOT_FOUND )
        return -1;

    for ( i = 0; i < nlink; i++ )
        if ( links[ i ] == reference )
            return i;

    links[ nlink ] = reference;

    return nlink++;
}



/* Handle end of anchor data */
static FunctionType DoCopyModeAnchorEnd
    (
    ParagraphContext*   pContext,
    TextContext*        tContext,
    Int16*              width
    )
{
    if ( tContext != NULL ) {
        if ( InsideCopyRegion( tContext ) && 0 <= refNo ){
            Char str[ 20 ];

            /* insert linkref */
            if ( Prefs()->hardcopyLink == HARDCOPY_ATBOTTOM ){
                StrPrintF( str, "[%d]", refNo + 1 );
                StoreString( str );
            }
            else if ( Prefs()->hardcopyLink == HARDCOPY_INLINE ){
                StoreString( "[" );
                AddURL( links[ refNo ] );
                StoreString( "]" );
            }
            else{
                /* Do nothing */
            }
        }
    }
    return DoAnchorEnd( pContext, tContext, width );
}



/* Handle anchor */
static FunctionType DoCopyModeAnchorBegin
    (
    ParagraphContext*   pContext,
    TextContext*        tContext,
    Int16*              width
    )
{
    if ( tContext != NULL ) {
        UInt8*  functionArgs;
        UInt16  reference;

        tContext->activeAnchor  = true;
        multiline.Anchor        = true;

        functionArgs    = pContext->function + 1;
        reference       = *functionArgs * 256 + *( functionArgs + 1 );

        AnchorStart( tContext, reference, TOP_OFFSET );
        if ( InsideCopyRegion( tContext ) )
            refNo = FindLink( reference );
        else
            refNo = -1;
    }
    return ANCHOR;
}



/* Add newline */
static FunctionType DoCopyModeNewLine
    (
    ParagraphContext*   pContext,
    TextContext*        tContext,
    Int16*              width
    )
{
    CopyChars( "\n", 1, tContext );
    return DoNewLine( pContext, tContext, width );
}



/* Include horizontal rule */
static FunctionType DoCopyModeHorizontalRule
    (
    ParagraphContext*   pContext,
    TextContext*        tContext,
    Int16*              width
    )
{
    CopyChars( "\n------------------\n", 20, tContext );
    return DoHorizontalRule( pContext, tContext, width );
}



/* Write link list at the bottom of buffer */
static void WriteLinkList( void )
{
    Int16 i;

    if ( 0 < nlink ) {
        StoreString( "\nLinks\n" );
        for ( i = 0; i < nlink; i++ ) {
            Char str[ 20 ];
            StrPrintF( str, "\n%4d. ", i + 1 );
            StoreString( str );
            AddURL( links[ i ] );
        }
    }
}



/* Add URL string to buffer */
Boolean AddURL
    (
    Int16 index   /* index in URL record */
    )
{
    MemHandle   linkHandle;
    Char*       text;
    Boolean     status;

    status      = false;
    text        = NULL;
    ErrTry {
        linkHandle  = ReturnRecordHandle( PLUCKER_LINKS_ID );
        if ( linkHandle != NULL ) {
            MemHandle   uncompressHandle;
            Header*     linkDocument;
            Int16       count;
            Int16       offset;
            Int16*      numP;

            count   = index;
            offset  = 0;

            linkDocument    = (Header*) MemHandleLock( linkHandle );
            numP            = (Int16*) ( linkDocument + 1 );
            while ( *numP < index ) {
                offset  = *numP;
                numP   += 2;
            }
            numP    += 1;
            count   -= offset;

            MemHandleUnlock( linkHandle );
            FreeRecordHandle(&linkHandle);

            linkHandle = ReturnRecordHandle( *numP );
            if ( linkHandle != NULL ) {
                uncompressHandle    = NULL;
                linkDocument        = (Header*) MemHandleLock( linkHandle );
                if ( linkDocument->type == DATATYPE_LINKS_COMPRESSED ) {
                    uncompressHandle = Uncompress( linkDocument );
                    if ( uncompressHandle != NULL )
                        text = (Char*) MemHandleLock( uncompressHandle );
                }
                else if ( linkDocument->type == DATATYPE_LINKS ) {
                    text = (Char*) ( linkDocument + 1 );
                }

                if ( text != NULL ) {
                    while ( --count )
                        text += StrLen( text ) + 1;
                    StoreString( text );
                    status = true;
                }
                if ( uncompressHandle != NULL )
                    MemHandleUnlock( uncompressHandle );
                if ( linkHandle != NULL ) {
                    MemHandleUnlock( linkHandle );
                    FreeRecordHandle(&linkHandle);
                }
            }
        }
    }
    ErrCatch( UNUSED_PARAM( err ) ) {
        Char msg[ 100 ];

        SysCopyStringResource( msg, strExternNoURL );
        StoreString( msg );
    } ErrEndCatch
    return status;
}


