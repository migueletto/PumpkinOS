/*
 * $Id: jogdial.h,v 1.16 2004/05/08 08:57:55 nordstrom Exp $
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

#ifndef PLUCKER_JOGDIAL_H
#define PLUCKER_JOGDIAL_H

#include "viewer.h"

#ifdef HAVE_HANDERA_SDK
#include <Vga.h>
#endif

typedef enum {
    noJogdial = 0,
    sonyJogdialType1 = 1,
    sonyJogdialType2 = 2,
    handeraJogdial = 4,
    handspringJogdial = 8,
    unknownJogdial = 512
} Jogdial;

/* sonyJogdialType1: Up, Down, Push, Push Up, Push Down available
 * sonyJogdialType2: Type1 + Back available
 * handeraJogdial: Up, Down, Push, Back available
 * handspringJogdial: Up, Down, Push available */

#ifdef HAVE_JOGDIAL

extern void JogdialResetValues(void) JOGDIAL_SECTION;
extern void JogdialHighlightRow(Boolean enable) JOGDIAL_SECTION;
extern void JogdialSetRow(UInt16 row) JOGDIAL_SECTION;
extern UInt16 JogdialGetRow(void) JOGDIAL_SECTION;
extern Boolean JogdialLibraryHandler(EventType *event) JOGDIAL_SECTION;
extern Boolean JogdialMainHandler(EventType *event) JOGDIAL_SECTION;
extern Boolean JogdialFontHandler( EventType* event ) JOGDIAL_SECTION;
extern Boolean JogdialPrefHandler( EventType* event ) JOGDIAL_SECTION;

extern Jogdial JogdialType(void) JOGDIAL_SECTION;

#ifdef HAVE_SONY_SDK

extern void HandleJogAssistMask( Boolean block ) JOGDIAL_SECTION;
extern void JogAssistFrmHelp( UInt16 helpMsgID ) JOGDIAL_SECTION;

#define FrmHelp( helpMsgID ) \
    ( ( JogdialType() == sonyJogdialType1 || \
        JogdialType() == sonyJogdialType2 ) ? \
      JogAssistFrmHelp( helpMsgID ) : \
      FrmHelp( helpMsgID ) )

#define sonyJogUp                  vchrJogUp
#define sonyJogDown                vchrJogDown
#define sonyJogPush                vchrJogPush
#define sonyJogPushRepeat          vchrJogPushRepeat
#define sonyJogPushUp              vchrJogPushedUp
#define sonyJogPushDown            vchrJogPushedDown
#define sonyJogRelease             vchrJogRelease
#define sonyJogBack                vchrJogBack

#endif /* HAVE_SONY_SDK */


#ifdef HAVE_HANDERA_SDK

#define handeraJogUp               vchrPrevField
#define handeraJogDown             vchrNextField
#define handeraJogRelease          chrCarriageReturn
#define handeraJogBack             chrEscape

#endif /* HAVE_HANDERA_SDK */

#ifdef HAVE_HANDSPRING_SDK

#define handspringJogUp            vchrPageUp
#define handspringJogDown          vchrPageDown
#define handspringJogRelease       vchrThumbWheelPush

#endif /* HAVE_HANDERA_SDK */

/* If we have all three of Sony SDK, Handera SDK and Handspring SDK */
#if defined( HAVE_HANDERA_SDK ) && defined( HAVE_HANDSPRING_SDK ) && defined( HAVE_SONY_SDK )

#define IsJogdialUp( chr )         ( chr == sonyJogUp || \
                                     chr == handeraJogUp || \
                                     ( JogdialType() == handspringJogdial && \
                                     chr == handspringJogUp ) )
#define IsJogdialDown( chr )       ( chr == sonyJogDown || \
                                     chr == handeraJogDown || \
                                     ( JogdialType() == handspringJogdial && \
                                     chr == handspringJogDown ) )
#define IsJogdialPush( chr )       ( chr == sonyJogPush )
#define IsJogdialPushRepeat( chr ) ( chr == sonyJogPushRepeat )
#define IsJogdialPushUp( chr )     ( chr == sonyJogPushUp )
#define IsJogdialPushDown( chr )   ( chr == sonyJogPushDown )
#define IsJogdialRelease( chr )    ( chr == sonyJogRelease || \
                                     chr == handeraJogRelease || \
                                     chr == handspringJogRelease )
#define IsJogdialBack( chr )       ( chr == sonyJogBack || \
                                     chr == handeraJogBack )

#else

/* If we have both of Handera SDK and Handspring SDK */
#if defined( HAVE_HANDERA_SDK ) && defined( HAVE_HANDSPRING_SDK )

#define IsJogdialUp( chr )         ( chr == handeraJogUp || \
                                     ( JogdialType() == handspringJogdial && \
                                     chr == handspringJogUp ) )
#define IsJogdialDown( chr )       ( chr == handeraJogDown || \
                                     ( JogdialType() == handspringJogdial && \
                                     chr == handspringJogDown ) )
#define IsJogdialPush( chr )       ( chr == NULL )
#define IsJogdialPushRepeat( chr ) ( chr == NULL )
#define IsJogdialPushUp( chr )     ( chr == NULL )
#define IsJogdialPushDown( chr )   ( chr == NULL )
#define IsJogdialRelease( chr )    ( chr == handeraJogRelease || \
                                     chr == handspringJogRelease )
#define IsJogdialBack( chr )       ( chr == handeraJogBack )

#else

/* If we have both Sony SDK and Handspring SDK */
#if defined( HAVE_HANDSPRING_SDK ) && defined( HAVE_SONY_SDK )

#define IsJogdialUp( chr )         ( chr == sonyJogUp || \
                                     ( JogdialType() == handspringJogdial && \
                                     chr == handspringJogUp ) )
#define IsJogdialDown( chr )       ( chr == sonyJogDown || \
                                     ( JogdialType() == handspringJogdial && \
                                     chr == handspringJogDown ) )
#define IsJogdialPush( chr )       ( chr == sonyJogPush )
#define IsJogdialPushRepeat( chr ) ( chr == sonyJogPushRepeat )
#define IsJogdialPushUp( chr )     ( chr == sonyJogPushUp )
#define IsJogdialPushDown( chr )   ( chr == sonyJogPushDown )
#define IsJogdialRelease( chr )    ( chr == sonyJogRelease || \
                                     chr == handspringJogRelease )
#define IsJogdialBack( chr )       ( chr == sonyJogBack )

#else

/* If we have both Sony SDK and Handera SDK */
#if defined( HAVE_HANDERA_SDK ) && defined( HAVE_SONY_SDK )

#define IsJogdialUp( chr )         ( chr == sonyJogUp || \
                                     chr == handeraJogUp )
#define IsJogdialDown( chr )       ( chr == sonyJogDown || \
                                     chr == handeraJogDown )
#define IsJogdialPush( chr )       ( chr == sonyJogPush )
#define IsJogdialPushRepeat( chr ) ( chr == sonyJogPushRepeat )
#define IsJogdialPushUp( chr )     ( chr == sonyJogPushUp )
#define IsJogdialPushDown( chr )   ( chr == sonyJogPushDown )
#define IsJogdialRelease( chr )    ( chr == sonyJogRelease || \
                                     chr == handeraJogRelease )
#define IsJogdialBack( chr )       ( chr == sonyJogBack || \
                                     chr == handeraJogBack )

#else

#define IsJogdialUp( chr )         ( chr == jogDialUp )
#define IsJogdialDown( chr )       ( chr == jogDialDown )
#define IsJogdialPush( chr )       ( chr == jogDialPush )
#define IsJogdialPushRepeat( chr ) ( chr == jogDialPushRepeat )
#define IsJogdialPushUp( chr )     ( chr == jogDialPushUp )
#define IsJogdialPushDown( chr )   ( chr == jogDialPushDown )
#define IsJogdialRelease( chr )    ( chr == jogDialRelease )
#define IsJogdialBack( chr )       ( chr == jogDialBack )

/* If we only have Sony SDK */
#ifdef HAVE_SONY_SDK

#define jogDialUp                  sonyJogUp
#define jogDialDown                sonyJogDown
#define jogDialPush                sonyJogPush
#define jogDialPushRepeat          sonyJogPushRepeat
#define jogDialPushUp              sonyJogPushUp
#define jogDialPushDown            sonyJogPushDown
#define jogDialRelease             sonyJogRelease
#define jogDialBack                sonyJogBack

#else

/* If we only have Handera SDK */
#ifdef HAVE_HANDERA_SDK

#define jogDialUp                  handeraJogUp
#define jogDialDown                handeraJogDown
#define jogDialPush                NULL
#define jogDialPushRepeat          NULL
#define jogDialPushUp              NULL
#define jogDialPushDown            NULL
#define jogDialRelease             handeraJogRelease
#define jogDialBack                handeraJogBack

#else

/* If we only have Handspring SDK */
#ifdef HAVE_HANDSPRING_SDK

#define jogDialUp                  handspringJogUp
#define jogDialDown                handspringJogDown
#define jogDialPush                NULL
#define jogDialPushRepeat          NULL
#define jogDialPushUp              NULL
#define jogDialPushDown            NULL
#define jogDialRelease             handspringJogRelease
#define jogDialBack                NULL

#endif /* HAVE_HANDSPRING_SDK */

#endif /* HAVE_HANDERA_SDK */

#endif /* HAVE_SONY_SDK */

#endif /* HAVE_HANDERA_SDK && HAVE_SONY_SDK */

#endif /* HAVE_HANDSPRING_SDK && HAVE_SONY_SDK */

#endif /* HAVE_HANDERA_SDK && HAVE_HANDSPRING_SDK */

#endif /* HAVE_HANDERA_SDK && HAVE_HANDSPRING_SDK && HAVE_SONY_SDK */

#else

#define JogdialType()                  noJogdial
#define JogdialResetValues()
#define JogdialHighlightRow( enable )
#define JogdialSetRow( row )
#define JogdialGetRow()                NO_ROW
#define JogdialLibraryHandler( event ) false
#define JogdialMainHandler( event )    false
#define JogdialFontHandler( event )    false
#define JogdialPrefHandler( event )    false

#define IsJogdialUp( chr )             false
#define IsJogdialDown( chr )           false
#define IsJogdialPush( chr )           false
#define IsJogdialPushRepeat( chr )     false
#define IsJogdialPushUp( chr )         false
#define IsJogdialPushDown( chr )       false
#define IsJogdialRelease( chr )        false
#define IsJogdialBack( chr )           false


#endif /* HAVE_JOGDIAL */

#endif /* PLUCKER_JOGDIAL_H */

