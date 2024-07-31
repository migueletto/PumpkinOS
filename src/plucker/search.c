/*
 * $Id: search.c,v 1.72 2004/04/20 21:03:33 prussar Exp $
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

#include "debug.h"
#include "document.h"
#include "genericfile.h"
#include "hires.h"
#include "list.h"
#include "os.h"
#include "paragraph.h"
#include "prefsdata.h"
#include "resourceids.h"
#include "resultform.h"
//#include "searchxlit.h"
#include "uncompress.h"
#include "const.h"
#include "search8.h"
#include "control.h"
#include "xlit.h"

#include "search.h"

#ifdef BUILD_ARMLETS
#include <PceNativeCall.h>
#include "armlets/search.h"
#endif

/***********************************************************************
 *
 *      Internal Constants
 *
 ***********************************************************************/
#define SEARCH_AGAIN    false
#define SELECT_OK       0
#define SELECT_NEXT     0
#define SELECT_RESTART  1
/* ideal number of characters before pattern in result listing */
#define LEFT_CONTEXT    15
/* ideal number of characters in and after pattern in result listing */
#define RIGHT_CONTEXT   17
#define MAX_CONTEXT     ( LEFT_CONTEXT + RIGHT_CONTEXT )


/***********************************************************************
 *
 *      Internal Types
 *
 ***********************************************************************/
typedef struct 
{
    UInt16    reference;
    Int16     depth;
    Int16     order;
} QueueNode;


/***********************************************************************
 *
 *      Local functions
 *
 ***********************************************************************/
static void FindContext( Header* record, Char* text, UInt16 pos,
                Char* result, Int16 resultBufferSize )
                SEARCH_SECTION;
static void ParseSearchString(Char *pattern) SEARCH_SECTION;
static void InitSearchRecord(Header *record) SEARCH_SECTION;
static Boolean DoMultipleTermSearch(Char *text, UInt16 size,
    UInt8 matchMode, UInt16 xlitMode, UInt16* pos, UInt16* endFound,
    Int16 depth) SEARCH_SECTION;
static Boolean DoSearch( Char* text, UInt16 size, Char* pattern,
                UInt8 matchMode, UInt16 xlitMode, UInt16* pos, UInt16* endFound,
                Int16 depth ) SEARCH_SECTION;
static void PushNode( UInt16 reference, Int16 depth ) SEARCH_SECTION;
static void AddReference( UInt16 reference, Int16 depth ) SEARCH_SECTION;
static Boolean SearchRecord(MemHandle handle, Char* pattern, Boolean onlyOne)
    SEARCH_SECTION;
static Boolean SearchInSubPages( Char* pattern, Boolean startFromBeginning,
                Boolean* findMore ) SEARCH_SECTION;



/***********************************************************************
 *
 *      Private variables
 *
 ***********************************************************************/
static UInt16   recordNum;
static UInt16   position          = 0;
static UInt16   endPosition       = 0;
static Char     resultFormat[ 7 ] = { 0 };

static LinkedList searchQueue  = NULL;
static Int16      headOfQueue  = 0;
static Int16      pagesInDepth = 1;
static Int16      lastDepth    = 0;
static Int16      pagesInNextDepth = 0;
static Char*      searchTerms[ MAX_SEARCH_TERMS ];
static UInt16     numSearchTerms;
static Char       searchTermBuffer[ MAX_PATTERN_LEN + MAX_SEARCH_TERMS ];
static Boolean    alreadySearched[ MAX_SEARCH_TERMS ];
static UInt16     lastFound[ MAX_SEARCH_TERMS ];
static UInt16     endLastFound[ MAX_SEARCH_TERMS ];
static Paragraph* curSearchPara;
static UInt16     curSearchParaOffset;
static UInt16     lastSearchedRecordId = 0;
static UInt16     lastSearchedSequenceFirstRecordId = 0;
static UInt16     lastSearchedSequenceLastRecordId = 0;



/* record ID of last searched record;  0 if no search was done yet */
UInt16 GetLastSearchedRecordId( void )
{
    return lastSearchedRecordId;
}



/* set record ID of last searched record */
void SetLastSearchedRecordId
    (
    UInt16 uid
    )
{
    lastSearchedRecordId = uid;
    lastSearchedSequenceFirstRecordId = GetSequenceBoundaryRecordId( uid,
                                            DIRECTION_UP, NULL );
    lastSearchedSequenceLastRecordId = GetSequenceBoundaryRecordId( uid,
                                            DIRECTION_DOWN, NULL );
}



/* return the first record ID in the last sequence searched */
void GetLastSearchedSequenceBoundaryRecordIds
     (
     UInt16* first,
     UInt16* last
     )
{
    *first = lastSearchedSequenceFirstRecordId;
    *last  = lastSearchedSequenceLastRecordId;
}



/* Initialize format of percentage indicator */
void InitializeResultFormat
    (
    const Char* lang
    )
{
    /* in some languages they say %10 instead of 10%, so we make the
       format configurable */
    if ( STREQ( lang, "tr" ) )
        StrCopy( resultFormat, "%%%ld" );
    else
        StrCopy( resultFormat, "%3ld%%" );
}



/* Load text into a WChar buffer from Plucker text, returning length loaded
   If more text is loaded into buffer than there is space for, then
   we get the right-most text.  This function fails if bufferLen > MAX_CONTEXT.
 */
static Int16 LoadText
    (
    WChar*       buffer,      /* buffer to load into */
    const Char*  text,        /* text stream to load from */
    const Char*  end,         /* points to after text stream end */
    Int16        bufferLen,   /* length of buffer */
    Int16        maxChars     /* maximum number of chars to load */
    )
{
    WChar        circularBuffer[MAX_CONTEXT];
    WChar        ch;
    Int16        circularIndex;
    Int16        charsFetched;
    Int16        streamSize;
    const Char*  inPtr;
    Int16        count;

    circularIndex = 0;
    charsFetched  = 0;
    streamSize    = end - text;
    
    if ( streamSize == 0 )
        return 0;

    if ( maxChars < bufferLen )
        bufferLen = maxChars;
    if ( streamSize < maxChars )
        maxChars = streamSize;

    if ( DeviceUses8BitChars() ) {
        if ( bufferLen + 8 <= maxChars ) {
            /* see if we can use a quick fetch method by checking
               if there are any anchors in range. */
            inPtr = text + maxChars - ( bufferLen + 8 );
            while ( inPtr < text + maxChars && '\0' != *inPtr )
                inPtr++;
            if ( inPtr == text + maxChars ) {
                inPtr = text + maxChars - bufferLen;
                while ( inPtr < end )
                    *buffer++ = ( UInt8 ) *inPtr++;
                return bufferLen;
            }
        }
        inPtr = text;
        while ( inPtr < end && charsFetched < maxChars ) {
            ch = ( UInt8 ) *inPtr++;
            if ( ch == 0 )
                inPtr += 1 + ( 7 & ( UInt8 ) *inPtr );
            else {
                charsFetched++;
                circularBuffer[ circularIndex++ ] = ch;
                if ( bufferLen <= circularIndex )
                    circularIndex = 0;
            }
        }
    }
    else {
        inPtr = text;
        while ( inPtr < end && charsFetched < maxChars ) {
            inPtr += TxtGlueGetNextChar( inPtr, 0, &ch );
            if ( ch == 0 )
                inPtr += 1 + ( 7 & ( UInt8 ) *inPtr );
            else {
                charsFetched++;
                circularBuffer[ circularIndex++ ] = ch;
                if ( bufferLen <= circularIndex )
                    circularIndex = 0;
            }
        }
    }

    if ( bufferLen < charsFetched ) {
        count        = bufferLen;
        charsFetched = bufferLen;
    }
    else
        count = charsFetched;
    circularIndex -= count;
    if ( circularIndex < 0 )
        circularIndex += bufferLen;
    while ( count-- ) {
        *buffer++ = circularBuffer[ circularIndex++ ];
        if ( bufferLen <= circularIndex )
            circularIndex = 0;
    }
    return charsFetched;
}




/* Find contextual information */
static void FindContext
    (
    Header* record,     /* record to work with */
    Char*   text,       /* text of record */
    UInt16  pos,        /* position of the pattern */
    Char*   result,     /* will contain the result string after return
                           of function */
    Int16   resultBufferSize /* how much space is available for the result */
    )
{
    WChar      context[ 2 * MAX_CONTEXT ];
    WChar*     right;
    Paragraph* paragraph;
    Int16      paragraphOffset;
    Int16      size;
    Int16      rightSize;
    Int16      leftSize;
    Int16      contextSize;
    Char*      endResult;
    WChar*     inPtr;

    paragraph = GET_PARAGRAPH( record, 0 );
    paragraphOffset = 0;
    while ( paragraphOffset + paragraph->size <= pos ) {
        paragraphOffset += paragraph->size;
        paragraph++;
    }
    text += paragraphOffset;
    pos  -= paragraphOffset;
    size  = paragraph->size;

    right     = context + MAX_CONTEXT;
    rightSize = LoadText( right, text + pos, text + size, MAX_CONTEXT,
                    MAX_CONTEXT );

    leftSize  = LEFT_CONTEXT;
    if ( rightSize < RIGHT_CONTEXT )
        leftSize += ( RIGHT_CONTEXT - rightSize );

    leftSize = LoadText( context, text, text + pos, leftSize, pos );

    if ( leftSize < LEFT_CONTEXT ) {
        Int16 tryRight;
        tryRight = RIGHT_CONTEXT + ( LEFT_CONTEXT - leftSize );
        if ( tryRight < rightSize )
            rightSize = tryRight;
    }
    else if ( RIGHT_CONTEXT < rightSize )
        rightSize = RIGHT_CONTEXT;

    MemMove ( context + leftSize, right, rightSize * sizeof(WChar) );
    contextSize = leftSize + rightSize;

    endResult = result + resultBufferSize - 1;
    inPtr     = context;

    while ( 0 < contextSize ) {
        Int16 charSize;
        charSize = TxtGlueCharSize( *inPtr );
        if ( endResult <= result + charSize )
            break;
        TxtGlueSetNextChar( result, 0, *inPtr );
        result += charSize;
        contextSize--;
        inPtr++;
    }
    *result = '\0';
}


/* Find pattern in text string */
static Boolean DoSearch
    (
    Char*   text,           /* text string */
    UInt16  size,           /* size of text */
    Char*   pattern,        /* pattern to search for */
    UInt8   matchMode,      /* indicates if the search should be
                               case-sensitive or not */
    UInt16  xlitMode,       /* how should we transliterate? */
    UInt16* pos,            /* start position, will contain the position
                               of the pattern if found */
    UInt16* endFound,       /* end of position where pattern is found */
    Int16   depth           /* depth of this page */
    )
{
    //Int16   startPos;
    Int16   charsize;
    Int16   topcharsize;
    UInt16  patternLen;
    UInt16  wplen;
    Int16   i;
    Int16   j;
    Int16   top;
    WChar   ch;
    WChar   wpattern[ 2 * ( MAX_PATTERN_LEN+1 ) ];
    Boolean guaranteed8Bits;
#ifdef BUILD_ARMLETS
    Boolean        useArmlet;
    void*          DoSearchArmlet;
    ArmSearchType  armData;
    MemHandle      armChunkH = NULL;
#endif
    static UInt8   lastMatchMode = SEARCH_UNINITIALIZED;
    static UInt16  lastXlitMode  = 0;
    static Char    xlat[ 256 ];
    static Boolean multibyteCharFix;
    static WChar   multibyteCharFixTo;
#ifdef SUPPORT_TRANSLITERATION
    static Boolean symmetricXlit;
#endif

    multibyteCharFix = false;
    if ( lastMatchMode != matchMode || xlitMode != lastXlitMode ) {
#ifdef SUPPORT_TRANSLITERATION
        symmetricXlit = SetTransliteration( xlitMode, xlat, &multibyteCharFix, 
                            &multibyteCharFixTo );
#else
        for ( i = 0 ; i < 256 ; i++ ) {
            xlat[ i ] = i;
        }
#endif
        if ( ! ( matchMode & SEARCH_CASESENSITIVE ) ) {
            for ( i = 0; i < 256; i++ )
                xlat[ i ] = TxtGlueLowerChar( ( UInt8 )xlat[ i ] );
        }
        lastMatchMode = matchMode;
    }

    //startPos    = *pos;
    charsize    = 1;
    patternLen  = StrLen( pattern );

    /* expand wchar string in advance*/
    i = wplen = 0;
    while ( i < patternLen ) {
        charsize = TxtGlueGetNextChar( pattern, i, &ch );
        if ( charsize == 1 ) {
#ifdef SUPPORT_TRANSLITERATION
            if ( symmetricXlit ) {
#endif
                ch = xlat[ (UInt8) ch ];
#ifdef SUPPORT_TRANSLITERATION
            }
            else if ( ! ( matchMode & SEARCH_CASESENSITIVE ) ) {
                ch = TxtGlueLowerChar( (UInt8) ch );
            }
#endif
        }
        else if ( multibyteCharFix )
            ch = multibyteCharFixTo;

        wpattern[ wplen ] = ch;

        i       += charsize;
        wplen   += 1;
    }

    guaranteed8Bits = DeviceUses8BitChars();
#ifdef BUILD_ARMLETS
    useArmlet       = false;
    DoSearchArmlet  = NULL;

    if ( SupportArmlets() && guaranteed8Bits ) {
        armChunkH = DmGetResource( ArmletResourceType, armDoSearch );
        if ( armChunkH != NULL ) {
            DoSearchArmlet = MemHandleLock( armChunkH );

            armData.size     = size;
            armData.depth    = depth;
            armData.text     = text;
            armData.wpattern = wpattern;
            armData.wplen    = wplen;
            armData.xlat     = xlat;

            useArmlet = true;
        }
    }
#endif

    i           = top = *pos;
    j           = 0;
    topcharsize = 0;
    while ( j < wplen && i < size ) {
#ifdef BUILD_ARMLETS
        if ( useArmlet ) {
            armData.i     = i;
            armData.j     = j;
            armData.top   = top;

            ch    = (WChar) PceNativeCall( DoSearchArmlet, &armData );

            i     = armData.i;
            j     = armData.j;
            top   = armData.top;

            if ( wplen <= j || size <= i )
                break;
            charsize = topcharsize = 1;
        }
        else
#endif
        if ( guaranteed8Bits ) {
            ch = DoSearch8BitText( text, size, wpattern, wplen, xlat,
                     (UInt16 *)&i, &j, &top, 0 < depth );
            if ( wplen <= j || size <= i )
                break;
            charsize = topcharsize = 1;
        }
        else
            charsize = TxtGlueGetNextChar( text, i, &ch );

        if ( j == 0 )
            topcharsize = charsize;

        if ( ch == '\0' ) {
            if ( 0 < depth ) {
                /* if function is anchor, store it to queue */
                if ( text[ i + 1 ] == 0x0A /* ANCHOR_START */ ){
                    UInt16 reference;
                    Int16  q;
                    Int16  listSize;

                    reference = 256 * (UInt8)text[ i + 2 ] +
                        (UInt8)text[ i + 3 ];

                    listSize = ListSize( searchQueue );
                    for ( q = 0; q < listSize; q ++ ){
                        /*look up in queue, append if not exist*/
                        QueueNode *node;
                        node = ListGet( searchQueue, q + 1 );
                        if ( node->reference == reference )
                            break;
                    }
                    if ( q == listSize )
                        AddReference( reference, depth );

                }
            }
            i  += 2 + ( text[ i + 1 ] & 0x07 );
            top = i;
            j   = 0;
            continue;
        }
        if ( charsize == 1 )
            ch = xlat[ (UInt8) ch ];
        else if ( multibyteCharFix )
            ch = multibyteCharFixTo;
        if ( ch != wpattern[ j ] ) {
            top += topcharsize;
            i    = top;
            j    = 0;
            continue;
        }
        i += charsize;
        j += 1;
    }
#ifdef BUILD_ARMLETS
    if ( armChunkH != NULL ) {
        MemHandleUnlock( armChunkH );
        DmReleaseResource( armChunkH );
    }
#endif
    if ( j == wplen ) {
        *pos      = top;
        *endFound = i;
        return true;
    }
    else {
        return false;
    }
}



/* Split search pattern into separate terms */
static void ParseSearchString
    (
    Char *pattern
    )
{
    Char *p;
    p = searchTermBuffer;
    StrCopy( p, pattern );
    for ( numSearchTerms = 0; *p && numSearchTerms < MAX_SEARCH_TERMS;
         numSearchTerms++ ) {
        while ( ' ' == *p )
            p++;
        if ( ! *p )
            break;
        searchTerms[ numSearchTerms ] = p;
        while ( *p && ' ' != *p)
            p++;
        if ( *p )
            *p++ = 0;
    }
}



/* Initialize multiple term searching */
static void InitSearchRecord
    (
    Header *record
    )
{
    UInt16 i;

    for ( i = 0; i < numSearchTerms; i++ )
        alreadySearched[ i ] = false;
    curSearchPara = GET_PARAGRAPH( record, 0 );
    curSearchParaOffset = 0;
}



/* search for the records with an already parsed search and with a record
   that has been initialized */
static Boolean DoMultipleTermSearch
    (
    Char*   text,
    UInt16  size,
    UInt8   matchMode,
    UInt16  xlitMode,
    UInt16* pos,
    UInt16* endFound,
    Int16   depth
    )
{
    UInt16  pNext;
    UInt16  i;
    UInt16  startPos;
    UInt16  currentPos;
    Boolean inThisPara;

    startPos = *pos;
    if ( startPos < curSearchParaOffset )
        startPos = curSearchParaOffset;
    currentPos = startPos;
    for ( ;; )
    {
        while ( curSearchParaOffset + curSearchPara->size <= currentPos ) {
            curSearchParaOffset += curSearchPara->size;
            curSearchPara++;
        }
        if ( size <= curSearchParaOffset )
            return false;
        pNext = curSearchParaOffset + curSearchPara->size;
        inThisPara = true;
        for ( i = 0; i < numSearchTerms && inThisPara; i++) {
            if ( ! alreadySearched[ i ] ||
                 lastFound[ i ] < curSearchParaOffset ||
                 ( i == 0 && lastFound[ 0 ] < startPos ) ) {
                if ( i == 0 && currentPos == startPos )
                    lastFound[ i ] = startPos;
                else
                    lastFound[ i ] = curSearchParaOffset;

                if ( ! DoSearch( text, size, searchTerms[ i ], matchMode,
                        xlitMode, lastFound + i, endLastFound + i, i ? 0 : depth ) )
                    return false;

                alreadySearched[ i ] = true;
            }
            if ( pNext <= lastFound[ i ] )
                inThisPara = false;
        }
        if ( inThisPara ) {
            *pos      = lastFound[ 0 ];
            *endFound = endLastFound[ 0 ];
            return true;
        }
        for ( i = 0; i < numSearchTerms; i++ ) {
            if ( alreadySearched[ i ] && currentPos < lastFound[ i ] )
                currentPos = lastFound[ i ];
        }
    }
}



static Boolean SearchRecord
         (
         MemHandle  handle,   /* handle of record to search */
         Char*      pattern,  /* pattern to search for */
         Boolean    onlyOne   /* are we looking only for a single match? */
         )
{
    Boolean     multiple;
    Boolean     done;
    MemHandle   uncompressHandle;
    Header*     record;
    UInt16      uniqueID;
    Char*       text;
    Int16       size;
    Boolean     match;

    multiple = Prefs()->searchFlags & SEARCH_MULTIPLE;
    if ( multiple ) {
        ParseSearchString( pattern );
        if ( ! numSearchTerms )
            return false;
    }

    done                = false;
    record              = MemHandleLock( handle );
    uniqueID            = record->uid;
    lastSearchedRecordId = uniqueID;
    uncompressHandle    = NULL;
    if ( record->type == DATATYPE_PHTML_COMPRESSED ) {
        ErrTry {
            uncompressHandle    = Uncompress( record );
            text                = MemHandleLock( uncompressHandle );
            size                = MemPtrSize( text );
            ResetLastUncompressedPHTML();
        }
        ErrCatch( UNUSED_PARAM( err ) ) {
            MemHandleUnlock( handle );
            return false;
        } ErrEndCatch
    }
    else {
        text = (Char *)GET_DATA( record );
        size = record->size;
    }

    if ( multiple )
        InitSearchRecord( record );

    for ( ;; ) {
        if ( multiple )
            match = DoMultipleTermSearch( text, size,
                    Prefs()->searchFlags & SEARCH_MATCHMODE_MASK,
                    Prefs()->searchXlit,
                    &position, &endPosition, lastDepth + 1 );
        else
            match = DoSearch( text, size, pattern,
                    Prefs()->searchFlags & SEARCH_MATCHMODE_MASK,
                    Prefs()->searchXlit,
                    &position, &endPosition, lastDepth + 1 );

        if ( onlyOne ) {
            break;
        }
        else if ( match ) {
            Char result[ 3*MAX_CONTEXT + 1 ];

            FindContext( record, text, position, result, 3*MAX_CONTEXT + 1 );
            done = DrawResultString( uniqueID, position, endPosition - position,
                    result );
            position++;
            if ( done )
                break;
        }
        else {
            break;
        }
    }
    if ( record->type == DATATYPE_PHTML_COMPRESSED )
        MemHandleUnlock( uncompressHandle );

    MemHandleUnlock( handle );

    if ( onlyOne )
        return match;
    else
        return done;
}



/* Search the document in one of the supported modes */
/* In single page mode, returns true if something is found,
   else returns false.  Otherwise, returns true if finished,
   and false if not. */
Boolean SearchDocument
    (
    Char*       pattern,            /* pattern to search for */
    Boolean     startFromBeginning, /* start from beginning of record */
    Boolean*    findMore,           /* indicates if there are remaining
                                       records to search through */
    SearchModeType searchMode
    )
{
    Boolean done;
    Boolean onlyOnePage;
    UInt16  numOfRecords;
    UInt16  initialRecordNum;

    if ( searchMode == SEARCH_IN_SUB_PAGES )
        return SearchInSubPages( pattern, startFromBeginning, findMore );

    onlyOnePage = ( searchMode == SEARCH_IN_ONE_PAGE );

    lastDepth = -1;

    if ( startFromBeginning ) {
        position    = 0;
        if ( onlyOnePage ) {
            GetSequenceBoundaryRecordId( GetCurrentRecordId(), DIRECTION_UP,
                &recordNum );
        }
        else {
            recordNum = 0;
        }
    }

    if ( NULL != findMore )
        *findMore = true;
    done = false;

    numOfRecords   = GetNumOfRecords();
    initialRecordNum = recordNum;

    for ( ;; ) {
        MemHandle   handle;
        Char        header[ 5 ];
        Int32       percent;

        if ( ! onlyOnePage ) {
            if ( EvtSysEventAvail( true ) ) {
                done = false;
                break;
            }

            percent = recordNum * 100L / numOfRecords;
            StrPrintF( header, resultFormat, percent );

            done = DrawResultHeader( header );
            if ( done )
                break;
        }

        handle = ReturnNextRecordHandle( &recordNum );
        if ( handle == NULL ) {
            if ( onlyOnePage ) {
                if ( FrmAlert( confirmEndOfDoc ) == SELECT_OK ) {
                    position  = 0;
                    recordNum = initialRecordNum;
                    continue;
                }
                else {
                    done = false;
                    break;
                }
            }
            recordNum   = 0;
            if ( findMore != NULL )
                *findMore = false;
            done = true;
            break;
        }

        done = SearchRecord( handle, pattern, onlyOnePage );

        if ( onlyOnePage && ! done ) {
            Header*     record;
            UInt8       flags;

            record = MemHandleLock( handle );
            flags  = record->flags;
            MemHandleUnlock( handle );
            FreeRecordHandle( &handle );

            if ( ! ( flags & HEADER_FLAG_CONTINUATION ) ) {
                Int16  selection;
                selection = FrmAlert( confirmEndOfPage );
                if ( selection == SELECT_RESTART ) {
                    position  = 0;
                    recordNum = initialRecordNum;
                    continue;
                }
                else if ( selection != SELECT_NEXT ) {
                    break;
                }
            }
        }
        else {
            FreeRecordHandle(&handle);
        }

        if ( done )
            break;

        recordNum++;
        position = 0;
    }
    if ( onlyOnePage ) {
        if ( done )
            SetFindPatternData( position, endPosition - position );
        else
            FrmUpdateForm( Prefs()->lastForm, frmRedrawUpdateCode );
    }

    SetLastSearchedRecordId( lastSearchedRecordId );

    return done;
}



/* Push links in search process. Do FIFO using link.c*/
static void PushNode
    (
    UInt16 reference,
    Int16 depth
    )
{
    QueueNode *node;

    node = SafeMemPtrNew( sizeof (QueueNode) );
    node->reference  = reference;
    node->depth      = depth;
    ListAppend( searchQueue, node );
    pagesInNextDepth += 1;
    node->order      = pagesInNextDepth;
}



/* Add a reference, being sure to include all the fragments. */
static void AddReference
    (
    UInt16 reference,
    Int16  depth
    )
{
    UInt16     recordNum;
    MemHandle  handle;

    reference = GetSequenceBoundaryRecordId( reference, DIRECTION_UP,
                    &recordNum );
    /* Is this a non-existent reference, i.e., external link? */
    if ( reference == NO_RECORD )
        return;
    PushNode( reference, depth );
    handle = GetSequentialRecordHandle( &recordNum, DIRECTION_DOWN );
    while ( NULL != handle ) {
        Header* record;

        record = MemHandleLock( handle );
        PushNode( record->uid, depth );
        MemHandleUnlock( handle );
        FreeRecordHandle( &handle );

        handle = GetSequentialRecordHandle( &recordNum, DIRECTION_DOWN );
    }
}



void ReleaseSearchQueue( void )
{
    QueueNode *node;
    QueueNode *nextNode;

    if ( searchQueue == NULL )
        return;
    node = ListFirst( searchQueue );
    while ( node != NULL ) {
        nextNode = ListNext( searchQueue, node );
        ListTakeOut( searchQueue, node );
        SafeMemPtrFree( node );
        node = nextNode;
    }
    ListDelete( searchQueue );
    searchQueue = NULL;
    headOfQueue  = 0;
    pagesInDepth = 1;
    lastDepth    = 0;
    pagesInNextDepth = 0;
}
    


/* Search in sub pages */
/* starting from current pages, follow links in depth order*/
static Boolean SearchInSubPages
    (
    Char*       pattern,            /* pattern to search for */
    Boolean     startFromBeginning, /* start from beginning of record */
    Boolean*    findMore            /* indicates if there are remaining
                                       records to search through */
    )
{
    Boolean done;
    MemHandle record;

    /*if marker is NULL */
    if ( startFromBeginning ) {
        Header    *r;
        MemHandle m;

        position = 0;
        /*push current page to the list*/
        ReleaseSearchQueue();
        searchQueue = ListCreate();
        r = MemHandleLock( m = GetCurrentRecord() );
        AddReference( r->uid , 0 );
        MemHandleUnlock( m );
        /*!!! isnt it a picture?*/
    }

    *findMore   = true;
    done        = false;

    while ( headOfQueue < ListSize( searchQueue ) ) {
        Char        header[ 30 ];
        QueueNode *node;

        if ( EvtSysEventAvail( true ) ) {
            done = false;
            break;
        }

        node = ListGet( searchQueue, headOfQueue + 1 );
        record = ReturnRecordHandle( node->reference );
        if ( record == NULL ) {
            headOfQueue++;
            position = 0;
            continue;
        }
        if ( node->depth != lastDepth ){
            pagesInDepth     = pagesInNextDepth;
            pagesInNextDepth = 0;
            lastDepth        = node->depth;
        }
        StrPrintF( header, "Depth:%2d [%d/%d]", node->depth, node->order,
            pagesInDepth );

        done = DrawResultHeader( header );
        if ( done )
            break;

        done = SearchRecord( record, pattern, false );
        FreeRecordHandle( &record );
        if ( done )
            break;


        headOfQueue += 1;
        position = 0;
    }
    if ( ListSize( searchQueue ) <= headOfQueue ) {
        recordNum   = 0;
        *findMore   = false;
        done        = true;
    }
    return done;
}



/* Search again in current page */
void SearchAgain( void )
{
    Char    pattern[ MAX_PATTERN_LEN + 1 ];

    GetSearchString( pattern );

    if ( StrLen( pattern ) == 0 )
        return;

    if ( SearchDocument( pattern, false, NULL, SEARCH_IN_ONE_PAGE ) )
        GoToSearchResult();
}




/* Set search position */
void SetSearchPosition
    (
    UInt16 pos
    )
{
    position = pos;
}



/* Search from here! */
void SetSearchFromHere( void )
{
    MetaRecord*  meta;
    MemHandle    handle;

    meta = MemHandleLock( GetMetaRecord() );
    SetSearchPosition( meta->characterPosition );
    MemHandleUnlock( GetMetaRecord() );

    SetLastSearchedRecordId( GetCurrentRecordId() );

    handle = FindRecord( lastSearchedRecordId, (Int16 *)&recordNum );
    
    if ( handle == NULL )
        recordNum = 0;
    else
        FreeRecordHandle( &handle );
}



/* Go to the search result position */
void GoToSearchResult( void )
{
    JumpToRecord( lastSearchedRecordId, NO_OFFSET, GetFindPatternPos() );
}

