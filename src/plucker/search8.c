/*
 * $Id: search8.c,v 1.6 2003/11/25 00:45:38 prussar Exp $
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
#include "search8.h"

/* This does the actual searching of 8 bit texts */
/* This routine is armletable, and hence is not allowed to call any
   OS functions or to have any writable static data */
Char DoSearch8BitText(
    Char*           text,         /* pointer to text to be searched */
    UInt16          size,         /* length of text to search */
    WChar*          wpattern,     /* pattern to search for */
    UInt16          patternLen,   /* length of pattern */
    Char*           xlat,         /* translation table for text */
    UInt16*         currentOffsetInTextPtr,    /* current position in text */
    Int16*          currentOffsetInPatternPtr, /* current position in pattern */
    Int16*          offsetOfPlaceFoundPtr, /* pattern found starts here */
                                           /* (equal to currentOffsetInText */
                                           /* if not found yet) */
    Boolean         haveDepth               /* need to recurse from here? */
    )
{
    Char*           placeInText;
    Char*           placeInPattern;
    Char*           placeFound;
    Char*           endOfPattern;
    Char*           endOfText;
    Char*           pattern;  /* the pattern is stored every second character */
    Char            ch;

    pattern        = 1 + (Char*) wpattern;
    placeInText    = text + ( *currentOffsetInTextPtr );
    placeInPattern = pattern + 2 * ( *currentOffsetInPatternPtr );
    placeFound     = text + ( *offsetOfPlaceFoundPtr );
    endOfPattern   = pattern + 2 * patternLen;
    endOfText      = text + size;

    ch = 0;

    while ( placeInPattern < endOfPattern && placeInText < endOfText ) {
        ch = xlat[ ( UInt8 ) *placeInText ];
        if ( ! ch  ) {
            if ( haveDepth && placeInText[ 1 ] == 0x0A )
                /* anchor: caller handles */
                break;
            placeInText   += 2 + ( UInt8 )( placeInText[ 1 ] & 0x07 );
            placeFound     = placeInText;
            placeInPattern = pattern;
        }
        else if ( ch != *placeInPattern ) {
            placeInText    = ++placeFound;
            placeInPattern = pattern;
            continue;
        }
        else {
            placeInText++;
            placeInPattern += 2;
        }
    }

    *currentOffsetInTextPtr    = placeInText - text;
    *currentOffsetInPatternPtr = ( placeInPattern - pattern ) / 2;
    *offsetOfPlaceFoundPtr     = placeFound - text;
    return ch;
}

