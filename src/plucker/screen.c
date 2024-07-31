/*
 * $Id: screen.c,v 1.48 2004/05/14 21:37:56 prussar Exp $
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

#include "cache.h"
#include "debug.h"
#include "dimensions.h"
#include "font.h"
#include "hires.h"
#include "mainform.h"
#include "os.h"
#include "prefsdata.h"
#include "resourceids.h"
#include "palmbitmap.h"
#include "handera.h"

#include "screen.h"


/***********************************************************************
 *
 *      Private variables
 *
 ***********************************************************************/
static RGBColorType deviceForeColors; 

static UInt32  screenDepth        = 1;
static HiRes   screenHiRes        = noHiRes;
static UInt32  screenWidth        = NORMAL_SCREEN_WIDTH;
static UInt32  screenHeight       = NORMAL_SCREEN_HEIGHT;
static Boolean forceDefaultScreen = false;
static UInt8*  windowPtr          = NULL;



/* Set screen bit depth for PalmOS3 */
void SetScreenModeOS3( void )
{
    Err     err;
    UInt16  activeFormID;
    Boolean resolutionChanged;
    Boolean depthChanged;

    if ( screenDepth == Prefs()->screenDepth &&
         screenHiRes == HiResType() )
        return;

    resolutionChanged = false;
    depthChanged      = false;

    /* Since HiResType() is set to unknownHiRes by default, the following two
       if statements will both return true to at least give HiResInitialize()
       the benifit of the doubt and the chance to initialize itself. If it
       cannot, then HiResType() is set to noHiRes, and no further attempts
       are made during the course of this session of the viewer */
    if ( ! IsHiResTypeNone( HiResType() ) ) {
        if ( IsHiResTypeNone( screenHiRes ) ) {
            err = HiResInitialize();
            if ( err == errNone )
                resolutionChanged = true;
            else if ( HiResStop != NULL )
                HiResStop();
        }
    }

    /* By default screenDepth is set to 0 when there aren't any preferences.
       Find the true default at this point instead of later on after falsly
       causing a display error */
    if ( Prefs()->screenDepth == 0 )
        SetDefaultScreenMode( true );

    /* Figure out the best resolution for this device */
    if ( ! forceDefaultScreen ) {
        HiRes hiResType;

        hiResType = HiResType();

        if ( IsHiResTypePalm( hiResType ) ) {
            err = WinScreenGetAttribute( winScreenHeight, &screenHeight );
            if ( err != errNone )
                screenHeight = PALM_SCREEN_HEIGHT;
            err = WinScreenGetAttribute( winScreenWidth, &screenWidth );
            if ( err != errNone )
                screenWidth  = PALM_SCREEN_WIDTH;
        }
        else if ( IsHiResTypeSony( hiResType ) ) {
            screenHeight = SONY_SCREEN_HEIGHT;
            screenWidth  = SONY_SCREEN_WIDTH;
        }
        else if ( IsHiResTypeHandera( hiResType ) ) {
            screenHeight = HANDERA_SCREEN_HEIGHT;
            screenWidth  = HANDERA_SCREEN_WIDTH;
        }
        else {
            screenHeight = NORMAL_SCREEN_HEIGHT;
            screenWidth  = NORMAL_SCREEN_WIDTH;
        }
    }

    /* If we no longer want hires... */
    if ( IsHiResTypeNone( HiResType() ) ) {
        /* ... and are currently in a hires mode, hires needs to be disabled */
        if ( ! IsHiResTypeNone( screenHiRes ) ) {
            if ( HiResStop != NULL )
                HiResStop();
            resolutionChanged = true;
        }
    }

    if ( screenDepth != Prefs()->screenDepth )
        depthChanged = true;

    screenDepth = Prefs()->screenDepth;
    screenHiRes = HiResType();

    MSG( _( "Setting screen to %ldx%ldx%ld\n", screenWidth, screenHeight,
        screenDepth ) );

    /* Handera dislikes being told what to set the values for
       screenWidth and screenHeight, so let it think what it wants */
    if ( IsHiResTypeHandera( HiResType() ) ) {
        err = WinScreenMode( winScreenModeSet, NULL, NULL,
            &screenDepth, NULL );
    }
    else {
        UInt16  prevCoordSys;

        prevCoordSys   = PalmSetCoordinateSystem( NATIVE );
        err = WinScreenMode( winScreenModeSet, &screenWidth, &screenHeight,
                  &screenDepth, NULL );
        if ( err != errNone ) {
            WinScreenMode( winScreenModeGetDefaults, &screenWidth, &screenHeight,
                NULL, NULL );
            err = WinScreenMode( winScreenModeSet, &screenWidth, &screenHeight,
                      &screenDepth, NULL );
        }
        PalmSetCoordinateSystem( prevCoordSys );
    }

    /* If we're having problems, set to hardware's default resolution */
    if ( err != errNone ) {
        MSG( _( "Unsuccessful. Trying 'safe' default values.\n" ) );
        if ( HiResStop != NULL )
            HiResStop();
        screenHiRes = noHiRes;

        SetDefaultScreenMode( true );
        resolutionChanged = true; /* assume it has */
        forceDefaultScreen = true;

        MSG( _( "Setting screen to %ldx%ldx%ld\n", screenWidth, screenHeight,
            screenDepth ) );
    }
    activeFormID = FrmGetActiveFormID();
    if ( IsVisibleToolbar( activeFormID ) ) {
        FrmEraseForm( FrmGetFormPtr( activeFormID ) );
        MainFormInit();
    }

    if ( resolutionChanged ) {
        SetHiResFunctions();
        InitializeViewportBoundaries();
    }

    if ( depthChanged ) {
        HanderaResetSilkScreen();
        ResetCache();
    }
    SetFontFunctions();
}



/* Set screen bit depth for PalmOS2 */
void SetScreenModeOS2( void )
{
    SetDefaultScreenModeOS2( true );
    SetFontFunctions();
}



/* Restore screen mode ( PalmOS 3.x ) */
void SetDefaultScreenModeOS3
    (
    Boolean save    /* store screen depth */
    )
{
    WinScreenMode( winScreenModeGetDefaults, &screenWidth, &screenHeight,
        &screenDepth, NULL );
    WinScreenMode( winScreenModeSetToDefaults, NULL, NULL, NULL, NULL );
    if ( save )
        Prefs()->screenDepth = screenDepth;
}



/* Restore screen mode ( PalmOS 2.x ) */
void SetDefaultScreenModeOS2
    (
    Boolean save    /* store screen depth */
    )
{
    screenWidth             = NORMAL_SCREEN_WIDTH;
    screenHeight            = NORMAL_SCREEN_HEIGHT;
    screenDepth             = 1;
    Prefs()->screenDepth    = screenDepth;
}



/* Initialize a new block of ForeColor; OS 3.5 or greater will use the
   good APIs */
void SetForeColor_OS35
    (
     TextContext* tContext /* pointer to text context */
    )
{
    RGBColorType       rgb;
    IndexedColorType   indexedColor;

    rgb = tContext->foreColor;

    /* Convert the RGB to an indexed color, so OS3.5 devices work too */
    indexedColor  = WinRGBToIndex( &rgb );
    /* Set both TextColor and ForeColor, as ForeColor will be used for
       hr's, underlines, anchor underlines, and strikethroughs */
    WinSetTextColor( indexedColor );
    WinSetForeColor( indexedColor );
}



/* Initialize a new block of ForeColor; OS 3.0 to OS 3.3 will use the
   old APIs */
void SetForeColor_OS3
    (
     TextContext* tContext /* pointer to text context */
    )
{
    RGBColorType rgb;

    rgb = tContext->foreColor;

    /* Syntax: new-forecolor, old-forecolor, new-backcolor, old-backcolor */
    WinSetColors( &rgb, NULL, NULL, NULL );
}



/* NOP for OS 2.0 */
void SetForeColor_OS2
    (
     TextContext* tContext /* pointer to text context */
    )
{
    /* no action */
}



/* Save the device's colors before drawing, so can later restore 
   to original when done drawing record; OS 3.5 or greater will
   use the good APIs */
void SaveDrawState_OS35( void )
{
    WinPushDrawState();
}



/* Save the device's colors before drawing, so can later restore 
   to original when done drawing record; OS 3.0 to OS 3.3 will
   use the old APIs */
void SaveDrawState_OS3( void )
{
    RGBColorType rgb;
        
    /* Store the draw state as a private deviceForeColors variable 
       before rendering page */    
    WinSetColors( 0, &rgb, 0, 0 );  
    deviceForeColors = rgb;
}



/* NOP for OS 2.0 */
void SaveDrawState_OS2( void )
{
    /* no action */
}



/* Restore the device's colors after done drawing record; OS 3.5
   or greater will use the good APIs */
void RestoreDrawState_OS35( void )
{
    WinPopDrawState();
}



/* Restore the device's colors after done drawing record; OS 3.0
   to OS 3.3 will use the old APIs */
void RestoreDrawState_OS3( void )
{
    /* Restore the draw state from deviceForeColors variable */    
    WinSetColors( &deviceForeColors, 0, 0, 0 );
}



/* NOP for OS 2.0 */
void RestoreDrawState_OS2( void )
{
    /* no action */
}



/* Wrapper to make sure WinScreenLock is used only on OS3.5 or higher */
void ScreenLock_OS35( void )
{
    windowPtr = WinScreenLock( winLockErase );
}



/* Wrapper to make sure WinScreenUnlock is used only on OS3.5 or higher */
void ScreenUnlock_OS35( void )
{
    if ( windowPtr != NULL ) {
        WinScreenUnlock();
        windowPtr = NULL;
    }
}



/* NOP for pre-OS3.5 */
void ScreenLockUnlock_None( void )
{
    /* no action */
}



/* BmpCreate() under Palm Hi-Res can create a V2 bitmap that does
   not conform to PalmSource documentation.  However, the cache
   code needs a compliant bitmap, and hence we need to do this
   ourselves manually.  Bitmaps created in this way must be removed
   with PortableBmpDelete().   Any color table specified must be
   in the PalmSource documented format. */
/* Do NOT mix PortableBbmCreate() with BmpDelete() or
   PortableBmpDelete() with BmpCreate() calls! */
BitmapType* PortableBmpCreate
    (
    Coord           bitmapX,
    Coord           bitmapY,
    UInt8           depth,
    ColorTableType* colorTable,
    Err*            err
    )
{
    PalmBitmapTypeV2* bitmap;
    UInt16            rowBytes;
    UInt32            size;
    UInt16            colorTableSize;
    if ( ! SupportHighDensity() )
        return BmpCreate( bitmapX, bitmapY, depth, NULL, err );
    rowBytes = ( bitmapX * depth + 15 ) / 16 * 2;
    if ( colorTable != NULL ) {
        colorTableSize = sizeof( PalmColorTableType ) +
                             sizeof( RGBColorType ) *
                                 ( ( ( PalmColorTableType* )colorTable )->
                                     numEntries & 0xFF );
    }
    else
        colorTableSize = 0;
    size     = ( UInt32 ) rowBytes * bitmapY + sizeof( PalmBitmapTypeV2 ) +
                   colorTableSize;
    bitmap   = SafeMemPtrNew( size );
    MemSet( bitmap, size, 0 );
    bitmap->rowBytes = rowBytes;
    bitmap->width    = bitmapX;
    bitmap->height   = bitmapY;
    bitmap->version  = 2;
    bitmap->pixelSize   = depth;
    MemMove( ( ( UInt8* )bitmap ) + sizeof( PalmBitmapTypeV2 ), colorTable,
               colorTableSize );
    if ( err != NULL )
        *err = errNone;
    return ( BitmapType* )bitmap;
}




/* Delete a more portable bitmap */
Err PortableBmpDelete
      (
      BitmapType* bitmap
      )
{
    if ( ! SupportHighDensity() ) 
        return BmpDelete( bitmap );
    SafeMemPtrFree( bitmap );
    return errNone;
}



/* Get the size of a bitmap that follows PalmSource specs for
   non-OS-generated bitmaps */
UInt32 PortableBmpSize
       (
       BitmapType* bitmap
       )
{
    UInt32 headerSize;
    switch ( ( ( PalmBitmapType*) bitmap )->version ) {
        case 0:
             headerSize = sizeof( PalmBitmapTypeV0 );
             break;
        case 1:
             headerSize = sizeof( PalmBitmapTypeV1 );
             break;
        case 2:
             headerSize = sizeof( PalmBitmapTypeV2 );
             break;
        case 3:
             headerSize = sizeof( PalmBitmapTypeV3 );
             break;
        default:
             return 0;
    }
    return headerSize + ( UInt32 )( ( ( PalmBitmapType*) bitmap )->rowBytes ) *
                             ( ( PalmBitmapType*) bitmap )->height;
}




