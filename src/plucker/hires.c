/*
 * $Id: hires.c,v 1.71 2004/04/29 01:52:05 prussar Exp $
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
#include <Core/Hardware/HwrMiscFlags.h>

#include "debug.h"
#include "dimensions.h"
#include "font.h"
#include "fullscreenform.h"
#include "os.h"
#include "prefsdata.h"
#include "resourceids.h"
#include "DIA.h"
#include "util.h"
#include "const.h"

#include "hires.h"
#include "../libpit/debug.h"


/***********************************************************************
 *
 *      Internal Constants
 *
 ***********************************************************************/
#define HIGH_DENSITY_FEATURE_SET_VERSION        4
#define MAX_BITMAP_SIZE                         64000


/***********************************************************************
 *
 *      Local functions
 *
 ***********************************************************************/
static void DetectAcerS50OrS60( void ) HIRES_SECTION;
static void AdjustCoordStandardToNative( Coord* coord ) HIRES_SECTION;
static void AdjustBoundsStandardToNative( RectangleType* r ) HIRES_SECTION;
static Boolean PalmHiResSupported( void ) HIRES_SECTION;
static Err PalmHiResInitialize( void ) HIRES_SECTION;
static Err PalmHiResStop( void ) HIRES_SECTION;
static Boolean SonyHiResSupported( void ) HIRES_SECTION;
static Err SonyHiResInitialize( void ) HIRES_SECTION;
static Err SonyHiResStop( void ) HIRES_SECTION;
static Boolean HanderaHiResSupported( void ) HIRES_SECTION;
static Err HanderaHiResInitialize( void ) HIRES_SECTION;
static Err HanderaHiResStop( void ) HIRES_SECTION;
static void HiResUpdateMargins( void ) HIRES_SECTION;

/* duplicate functions to the original system trap call */
//extern UInt16 FrmAlertSysTrap( UInt16 alertID ) SYS_TRAP( sysTrapFrmAlert );
//extern UInt16 FrmCustomAlertSysTrap( UInt16 alertID, const Char* s1, const Char* s2, const Char* s3 ) SYS_TRAP( sysTrapFrmCustomAlert );



/***********************************************************************
 *
 *      Private variables
 *
 ***********************************************************************/

static UInt16 sonyHiResRefNum = 0;

static HiRes hiResType = unknownHiRes;

static UInt16 nativeDensity  = kDensityLow;

static Boolean isAcerS50OrS60 = false;

static Margins hiResMargins;

static WidthsType palmHiResWidths[ NUM_ROTATE_TYPES ] = {
    { SCROLLBAR_WIDTH_HIRES_PALM, TOOLBAR_HEIGHT_HIRES_PALM,
          { TOP_MARGIN_HIRES_PALM, BOTTOM_MARGIN_HIRES_PALM,
            LEFT_MARGIN_HIRES_PALM, RIGHT_MARGIN_HIRES_PALM } }
#ifdef HAVE_ROTATE
    ,
    { SCROLLBAR_WIDTH_HIRES_PALM,
          TOOLBAR_HEIGHT_HIRES_PALM ,
          { LEFT_MARGIN_HIRES_PALM, RIGHT_MARGIN_HIRES_PALM, 0, 0 } },
    { SCROLLBAR_WIDTH_HIRES_PALM,
          TOOLBAR_HEIGHT_HIRES_PALM,
          { RIGHT_MARGIN_HIRES_PALM, LEFT_MARGIN_HIRES_PALM, 0, 0 } }
#endif
};

static WidthsType sonyHiResWidths[ NUM_ROTATE_TYPES ] = {
    { SCROLLBAR_WIDTH_HIRES_SONY, TOOLBAR_HEIGHT_HIRES_SONY,
          { TOP_MARGIN_HIRES_SONY, BOTTOM_MARGIN_HIRES_SONY,
            LEFT_MARGIN_HIRES_SONY, RIGHT_MARGIN_HIRES_SONY } }
#ifdef HAVE_ROTATE
    ,
    { SCROLLBAR_WIDTH_HIRES_SONY,
          TOOLBAR_HEIGHT_HIRES_SONY,
          { LEFT_MARGIN_HIRES_SONY, RIGHT_MARGIN_HIRES_SONY, 0, 0 } },
    { SCROLLBAR_WIDTH_HIRES_SONY,
          TOOLBAR_HEIGHT_HIRES_SONY,
          { RIGHT_MARGIN_HIRES_SONY, LEFT_MARGIN_HIRES_SONY, 0, 0 } }
#endif
};

static WidthsType handeraHiResWidths[ NUM_ROTATE_TYPES ] = {
    { SCROLLBAR_WIDTH_HIRES_HANDERA, TOOLBAR_HEIGHT_HIRES_HANDERA,
          { TOP_MARGIN_HIRES_HANDERA, BOTTOM_MARGIN_HIRES_HANDERA,
            LEFT_MARGIN_HIRES_HANDERA, RIGHT_MARGIN_HIRES_HANDERA } }
#ifdef HAVE_ROTATE
    ,
    { SCROLLBAR_WIDTH_HIRES_HANDERA,
          TOOLBAR_HEIGHT_HIRES_HANDERA,
          { LEFT_MARGIN_HIRES_HANDERA, RIGHT_MARGIN_HIRES_HANDERA, 0, 0 } },
    { SCROLLBAR_WIDTH_HIRES_HANDERA,
          TOOLBAR_HEIGHT_HIRES_HANDERA,
          { RIGHT_MARGIN_HIRES_HANDERA, LEFT_MARGIN_HIRES_HANDERA, 0, 0 } }
#endif
};




/* Since the palm still only accepts incoming events in a 160x160 screen,
   we need to translate them to the hires equivilant to handle our clicks
   properly. Except for elements such as the toolbar and scrollbar which
   still need to be in 'lowres' values in order to accept events. */
void TranslateHiResPenEvents
    (
    EventType* event  /* pointer t an EventType structure */
    )
{
    Int16 topLeftX;
    Int16 topLeftY;
    Int16 extentX;
    Int16 extentY;

    /* This is only necessary for sony and palm hires */
    if ( ( ! IsHiResTypeSony( hiResType ) &&
           ! IsHiResTypePalm( hiResType ) ) ||
         IsFullscreenformActive() )
        return;

    if ( IsHiResTypePalm( hiResType ) ) {
        if ( isAcerS50OrS60 ) {
            /* Fixes an apparent bug in Acer S60 (and presumably S50)
               handling of EvtGetPenNative().  Acer S50/S60 POSE ROM
               and SDK are apparently no longer available, so this is
               done empirically thanks to a helpful user.  It may not
               be the ideal solution. */
            event->screenX *= 2;
            event->screenY *= 2;
        }
        else {
            EvtGetPenNative( WinGetActiveWindow(), &( event->screenX ),
                &( event->screenY ), &( event->penDown ) );
        }    
    }
    else {
        topLeftX = TopLeftX() / 2;
        topLeftY = TopLeftY() / 2;
        extentX = ExtentX() / 2;
        extentY = ExtentY() / 2;

        /* Multiply our pen events by two to emulate the hires screen */
        if ( topLeftY < event->screenY &&
             event->screenY < ( topLeftY + extentY ) &&
             topLeftX < event->screenX &&
             event->screenX < ( topLeftX + extentX ) ) {
            event->screenX *= 2;
            event->screenY *= 2;
        }
    }
}




/* Convert a standard image into a higher nativeDensity version of itself */
Boolean ConvertImageToV3
    (
    BitmapType** imagePtr   /* Pointer to a pointer to a BitmapType */
    )
{
    BitmapTypeV3* imagePtrV3;

    if ( ! IsHiResTypePalm( hiResType ) )
        return false;

    imagePtrV3 = BmpCreateBitmapV3( *imagePtr, nativeDensity,
        BmpGetBits( *imagePtr ), NULL );

    if ( imagePtrV3 == NULL )
        return false;

    *imagePtr = (BitmapType*) imagePtrV3;

    return true;
}



/* General WinDrawBitmap() routine.  Works for OS5, Sony, etc. */
void GeneralWinDrawBitmap
        (
        BitmapType* bitmap,
        Coord       x,
        Coord       y
        )
{
    Boolean       converted;
    converted = ConvertImageToV3( &bitmap );
    WinDrawBitmap( bitmap, x, y );
    if ( converted )
        BmpDelete( bitmap );
}



/* set the hardware-specific functions to their usable counterparts */
void SetHiResFunctions( void )
{
    if ( IsHiResTypePalm( hiResType ) ) {
        TopLeftX       = PalmTopLeftX;
        TopLeftY       = PalmTopLeftY;
        ExtentX        = PalmExtentX;
        ExtentY        = PalmExtentY;
        HiResStop      = PalmHiResStop;
        UpdateMargins  = HiResUpdateMargins;
    }
    else if ( IsHiResTypeSony( hiResType ) ) {
        TopLeftX       = NonPalmHiResTopLeftX;
        TopLeftY       = NonPalmHiResTopLeftY;
        ExtentX        = NonPalmHiResExtentX;
        ExtentY        = NonPalmHiResExtentY;
        HiResStop      = SonyHiResStop;
        UpdateMargins  = HiResUpdateMargins;
    }
    else if ( IsHiResTypeHandera( hiResType ) ) {
        TopLeftX       = NonPalmHiResTopLeftX;
        TopLeftY       = NonPalmHiResTopLeftY;
        ExtentX        = NonPalmHiResExtentX;
        ExtentY        = NonPalmHiResExtentY;
        HiResStop      = HanderaHiResStop;
        UpdateMargins  = HiResUpdateMargins;
    }
    else {
        SetStandardFunctions();
    }
}



/* Is this an Acer S50/S60? */
void DetectAcerS50OrS60( void )
{
    Err err;
    UInt32  deviceID;
    UInt32  manufacturerID;
    isAcerS50OrS60 = false;
    err = FtrGet( sysFtrCreator, sysFtrNumOEMCompanyID, &manufacturerID );
    if ( err == errNone && manufacturerID == acerS50OrS60ManufacturerID ) {
        err = FtrGet( sysFtrCreator, sysFtrNumOEMDeviceID, &deviceID );
        if ( err == errNone && deviceID == acerS50OrS60DeviceID )
            isAcerS50OrS60 = true;
    }
}



/* call the system-dependant initilization procedures */
Err HiResInitialize( void )
{
    Err err;

    /* It's important to note whats going on here with Palm and Sony hires
       devices. We're explicitly checking checking for Palm hires before Sony
       because we want to support the newer OS5-based Sony devices within
       PalmSource's API, not Sony's.

       Sony's OS5 devices do support both APIs but the PalmSource version has
       been deemed the new standard for hires. Partially as a result, Sony's
       OS5 devices actually support hires better under the new standard then
       their own API. Generally speaking both Palm and Sony recommend you only
       use the Sony API under Sony OS4 and earlier devices.

       Basically hiResType will never resolve to sonyHiRes even on an OS5 Sony
       device */
    err = errNoHiRes;
    if ( PalmHiResSupported() ) {
        hiResType   = palmHiRes;
        err         = PalmHiResInitialize();
        DetectAcerS50OrS60();
    }
    else if ( SonyHiResSupported() ) {
        hiResType   = sonyHiRes;
        err         = SonyHiResInitialize();
    }
    else if ( HanderaHiResSupported() ) {
        hiResType   = handeraHiRes;
        err         = HanderaHiResInitialize();
    }

    if ( err != errNone ) {
        hiResType = noHiRes;
    }


    return err;
}



/* return what type of hires support is active */
HiRes HiResType( void )
{
    return hiResType;
}



void HiResAdjust
    (
    Coord* i,
    UInt16 type
    )
{
    if ( ( type & hiResType ) != 0 )
        AdjustCoordStandardToNative( i );
}



void HiResAdjustCurrentToNative
     (
     Coord* x
     )
{
    if ( ! IsHiResTypePalm( hiResType ) ||
         WinGetCoordinateSystem() != STANDARD )
        return;
    AdjustCoordStandardToNative( x );
}




/* Adjust native coordinates to standard if needed.  This may be an
   overestimate. */
void HiResAdjustNativeToCurrent
     (
     Coord* x
     )
{
    if ( ! IsHiResTypePalm( hiResType ) ||
         WinGetCoordinateSystem() != STANDARD )
        return;
    switch ( nativeDensity ) {
        case kDensityOneAndAHalf:
            *x = ( ( *x ) * 2 + 2 ) / 3;
            break;
        case kDensityDouble:
            *x = ( ( *x ) + 1 ) / 2;
            break;
        case kDensityTriple:
            *x = ( ( *x ) + 2 ) / 3;
            break;
        case kDensityQuadruple:
            *x = ( ( *x ) + 3 ) / 4;
            break;
        default:
            break;
    }
    return;
}




void HiResAdjustBounds
    (
    RectangleType* r,
    UInt16         type
    )
{
    if ( ( type & hiResType ) != 0 )
        AdjustBoundsStandardToNative( r );
}



/* Convert a coord from kCoordinatesStandard to kCoordinatesNative */
static void AdjustCoordStandardToNative
    (
    Coord* coord
    )
{
    switch ( nativeDensity ) {
        case kDensityOneAndAHalf:
            *coord += *coord / 2;
            break;
        case kDensityDouble:
            *coord *= 2;
            break;
        case kDensityTriple:
            *coord *= 3;
            break;
        case kDensityQuadruple:
            *coord *= 4;
            break;
        default:
            break;
    }
}



/* Convert bounds from kCoordinatesStandard to kCoordinatesDouble */
static void AdjustBoundsStandardToNative
    (
    RectangleType* r
    )
{
    AdjustCoordStandardToNative( &( r->topLeft.x ) );
    AdjustCoordStandardToNative( &( r->topLeft.y ) );
    AdjustCoordStandardToNative( &( r->extent.x ) );
    AdjustCoordStandardToNative( &( r->extent.y ) );
}



/* Identify if we support palm hires */
static Boolean PalmHiResSupported( void )
{
    if ( SupportHighDensity() && kDensityLow != PalmGetDensity() ) {
        MSG( _( "Palm HiRes supported\n" ) );
        return true;
    }
    MSG( _( "Palm HiRes not supported\n" ) );
    return false;
}



/* palm specific hires initialization procedure */
static Err PalmHiResInitialize( void )
{
    Err err;

    if ( IsHiResTypePalm( hiResType ) ) {
        LoadCustomFonts( PALM_FONTS );
        err = errNone;
    }
    else {
        err = errNoHiRes;
    }
    if ( err == errNone ) {
        nativeDensity = PalmGetDensity();
    }
    if ( err == errNone ) {
        /* Looks good, palm hires is available */
        MSG( _( "Palm HiRes initialized\n" ) );
    }
    else {
        MSG( _( "Palm HiRes failed to initialize\n" ) );
    }
    return err;
}



/* Get the nativeDensity. */
UInt16 PalmGetDensity( void )
{
    UInt32 densityValue;
    Err    err;
    err = WinScreenGetAttribute( winScreenDensity,
              &densityValue );
    if ( err == errNone )
        return ( UInt16 ) densityValue;
    else
        return kDensityLow;
}



/* palm specific hires stopping procedure */
static Err PalmHiResStop( void )
{
    /* no longer available, but support remains */
    hiResType     = noHiRes;
    nativeDensity = kDensityLow;
    SetStandardFunctions();
    nativeDensity = kDensityLow;
    return errNone;
}



/* Identify if we support sony hires */
static Boolean SonyHiResSupported( void )
{
#ifdef HAVE_SONY_SDK
    Err                err;
    SonySysFtrSysInfoP sonySysFtrSysInfoP;

    err = FtrGet( sonySysFtrCreator, sonySysFtrNumSysInfoP,
            (UInt32 *) &sonySysFtrSysInfoP );

    /* If we're on a sony system, and have access to the HRLib.. */
    if ( err == errNone &&
         sonySysFtrSysInfoP->libr & sonySysFtrSysInfoLibrHR ) {
        MSG( _( "Sony HiRes supported\n" ) );
        return true;
    }
#endif
    MSG( _( "Sony HiRes not supported\n" ) );
    return false;
}



/* sony specific hires initialization procedure */
static Err SonyHiResInitialize( void )
{
    Err err;

#ifdef HAVE_SONY_SDK
    if ( IsHiResTypeSony( hiResType ) ) {
        err = SysLibFind( sonySysLibNameHR, &sonyHiResRefNum );
        if ( err == sysErrLibNotFound )
            err = SysLibLoad( 'libr', sonySysFileCHRLib, &sonyHiResRefNum );
        if ( err == errNone )
            err = HROpen( sonyHiResRefNum );
    }
    else {
        err = errNoHiRes;
    }

    if ( err == errNone ) {
        /* Looks good, sony hires is available */
        MSG( _( "Sony OS4 HiRes initialized\n" ) );
        LoadCustomFonts( SONY_FONTS );
        nativeDensity = kDensityDouble;
    }
    else {
        err = errNoHiRes; /* just incase HROpen fails */
        sonyHiResRefNum = NULL;
        MSG( _( "Sony HiRes failed to initialize\n" ) );
    }
#else
    sonyHiResRefNum = 0;
    err = errNoHiRes;
#endif
    return err;
}



/* retrive value of sonyHiResRefNum */
UInt16 SonyHiResRefNum( void )
{
    return sonyHiResRefNum;
}



/* sony specific hires stopping procedure */
static Err SonyHiResStop( void )
{
#ifdef HAVE_SONY_SDK
    if ( sonyHiResRefNum == NULL ) {
        MSG( _( "Sony HiRes doesn't need to be stopped\n" ) );
        return errNoHiRes;
    }
    WinScreenMode( winScreenModeSetToDefaults, NULL, NULL, NULL, NULL );
    HRClose( sonyHiResRefNum );
    sonyHiResRefNum = NULL;
    MSG( _( "Sony HiRes stopped\n" ) );

    /* no longer available, but support remains */
    hiResType     = noHiRes;
    nativeDensity = kDensityLow;
#endif
    SetStandardFunctions();
    return errNone;
}



/* Identify if we support handera hires */
static Boolean HanderaHiResSupported( void )
{
#ifdef HAVE_HANDERA_SDK
    Err    err;
    UInt32 vgaVersion;

    err = FtrGet( TRGSysFtrID, TRGVgaFtrNum, &vgaVersion );

    /* Do something with vgaVersion? */

    if ( err == errNone ) {
        MSG( _( "Handera HiRes supported\n" ) );
        return true;
    }
#endif
    MSG( _( "Handera HiRes not supported\n" ) );
    return false;
}



/* handera specific hires initialization procedure */
static Err HanderaHiResInitialize( void )
{
    Err err;

#ifdef HAVE_HANDERA_SDK
    if ( IsHiResTypeHandera( hiResType ) )
        err = VgaSetScreenMode( screenMode1To1, rotateModeNone );
    else
        err = errNoHiRes;

    if ( err == errNone ) {
        nativeDensity = kDensityOneAndAHalf;
        MSG( _( "Handera HiRes initialized\n" ) );
    }
    else {
        MSG( _( "Handera HiRes not initialized\n" ) );
    }
#else
    err = errNoHiRes;
#endif
    return err;
}



/* handera specific hires stopping procedure */
static Err HanderaHiResStop( void )
{
#ifdef HAVE_HANDERA_SDK
    VgaSetScreenMode( screenModeScaleToFit, rotateModeNone );

    hiResType     = noHiRes;
    nativeDensity = kDensityLow;
    MSG( _( "Handera HiRes stopped\n" ) );
#endif
    SetStandardFunctions();
    return errNone;
}



/* palm version of TopLeftX() */
Int16 PalmTopLeftX( void )
{
    if ( WinGetCoordinateSystem() == NATIVE )
        return hiResMargins.left;
    else
        return StandardTopLeftX();
}



/* palm version of TopLeftY() */
Int16 PalmTopLeftY( void )
{
    if ( WinGetCoordinateSystem() == NATIVE )
        return hiResMargins.top;
    else
        return StandardTopLeftY();
}



/* hi-res version of UpdateMargins() */
void HiResUpdateMargins( void )
{
    if ( NUM_ROTATE_TYPES < Prefs()->rotate )
        Prefs()->rotate = ROTATE_ZERO;
    if ( IsHiResTypePalm( HiResType() ) ) {
        SetMargins( &hiResMargins, &palmHiResWidths[ Prefs()->rotate ] );
        StandardUpdateMargins();
    }
    else if ( IsHiResTypeSony( hiResType ) ) {
        SetMargins( &hiResMargins, &sonyHiResWidths[ Prefs()->rotate ] );
    }
    else if ( IsHiResTypeHandera( hiResType ) ) {
        SetMargins( &hiResMargins, &handeraHiResWidths[ Prefs()->rotate ] );
    }
}



/* palm version of ExtentX() */
Int16 PalmExtentX( void )
{
    if ( WinGetCoordinateSystem() == NATIVE )
        return MaxExtentX() - hiResMargins.right - hiResMargins.left;
    else
        return StandardExtentX();
}



/* palm version of ExtentY() */
Int16 PalmExtentY( void )
{
    if ( WinGetCoordinateSystem() == NATIVE )
        return MaxExtentY() - hiResMargins.top - hiResMargins.bottom;
    else
        return StandardExtentY();
}



/* get the current coordinate system being used for palm hires devices */
UInt16 PalmGetCoordinateSystem( void )
{
    if ( ! IsHiResTypePalm( hiResType ) )
        return STANDARD;

    return WinGetCoordinateSystem();
}



/* set the coordinate system to be used for palm hires devices */
UInt16 PalmSetCoordinateSystem
    (
    UInt16 coordSys
    )
{
    UInt16 prevCoordSys;

    if ( ! IsHiResTypePalm( hiResType ) )
        return STANDARD;

    prevCoordSys = WinSetCoordinateSystem( coordSys );

    UpdateDisplayExtent();

    return prevCoordSys;
}



/* Wrapper function to make sure FrmAlert() always displays properly */
UInt16 PalmStandardCoordFrmAlert
    (
    UInt16 alertID
    )
{
    UInt16 prevCoordSys;
    UInt16 result;

    prevCoordSys = PalmSetCoordinateSystem( STANDARD );
    //result = FrmAlertSysTrap( alertID );
    result = FrmAlert( alertID );
    PalmSetCoordinateSystem( prevCoordSys );

    return result;
}



/* Wrapper function to make sure FrmCustomAlert() always displays properly */
UInt16 PalmStandardCoordFrmCustomAlert
    (
    UInt16 alertID,
    const Char* s1,
    const Char* s2,
    const Char* s3
    )
{
    UInt16 prevCoordSys;
    UInt16 result;

    prevCoordSys = PalmSetCoordinateSystem( STANDARD );
    //result = FrmCustomAlertSysTrap( alertID, s1, s2, s3 );
    result = FrmCustomAlert( alertID, s1, s2, s3 );
    PalmSetCoordinateSystem( prevCoordSys );

    return result;
}



/* non-Palm hi-res version of TopLeftX() */
Int16 NonPalmHiResTopLeftX( void )
{
    return hiResMargins.left;
}



/* non-Palm hi-res version of TopLeftY() */
Int16 NonPalmHiResTopLeftY( void )
{
    return hiResMargins.top;
}



/* non-Palm hi-res version of ExtentX() */
Int16 NonPalmHiResExtentX( void )
{
    return MaxExtentX() - hiResMargins.right - hiResMargins.left;
}



/* non-Palm hi-res version of ExtentY() */
Int16 NonPalmHiResExtentY( void )
{
    return MaxExtentY() - hiResMargins.top - hiResMargins.bottom;
}



/* Font list needs to be converted into VGA-specific fonts at run-time */
void HanderaConvertFontList
    (
    FontID* oldFontList,
    FontID* newFontList
    )
{
#ifdef HAVE_HANDERA_SDK
    UInt16 i;

    if ( IsHiResTypeHandera( hiResType ) ) {
        for ( i = 0; i < MAXSTYLES; i++ ) {
            if ( oldFontList[ i ] != narrowFont &&
                 oldFontList[ i ] != narrowFixedFont )
                newFontList[ i ] = VgaBaseToVgaFont( oldFontList[ i ] );
        }
    }
#endif
}



UInt16 HiResFontImage
    (
    FontID font,
    UInt16 stdImage,
    UInt16 palmHalfImage,
    UInt16 nonPalmDoubleImage
    )
{
    if ( IsHiResTypePalm( hiResType ) &&
         ( font == tinyFont_palm ||
           font == tinyBoldFont_palm ||
           font == smallFont_palm ||
           font == smallBoldFont_palm ||
           font == userStdFont_palm ) )
        return palmHalfImage;
    else
#ifdef HAVE_SONY_SDK
    if ( IsHiResTypeSony( hiResType ) &&
         ( font == hrStdFont ||
           font == hrBoldFont ||
           font == hrLargeFont ||
           font == hrLargeBoldFont ||
           font == userStdFont_sony ) )
        return nonPalmDoubleImage;
    else
#endif
#ifdef HAVE_HANDERA_SDK
    if ( IsHiResTypeHandera( hiResType ) &&
         ( font == VgaBaseToVgaFont( stdFont ) ||
           font == VgaBaseToVgaFont( boldFont ) ||
           font == VgaBaseToVgaFont( largeFont ) ||
           font == VgaBaseToVgaFont( largeBoldFont ) ) )
        return nonPalmDoubleImage;
    else
#endif
        return stdImage;
}



FontID HiResFont
    (
    FontID font
    )
{
    FontID hrFont;

#ifdef HAVE_SONY_SDK
    if ( IsHiResTypeSony( hiResType ) ) {
        switch ( font ) {
            case stdFont:
                hrFont = hrStdFont;
                break;
            case boldFont:
                hrFont = hrBoldFont;
                break;
            case largeFont:
                hrFont = hrLargeFont;
                break;
            case largeBoldFont:
                hrFont = hrLargeBoldFont;
                break;
            case symbolFont:
                hrFont = hrSymbolFont;
                break;
            default:
                hrFont = font;
                break;
        }
    }
    else
#endif
#ifdef HAVE_HANDERA_SDK
    if ( IsHiResTypeHandera( hiResType ) ) {
        hrFont = VgaBaseToVgaFont( font );
    }
    else
#endif
        hrFont = font;

    return hrFont;
}



#ifdef HAVE_SONY_SDK
#undef WinDrawBitmap
#undef WinDrawChars
#undef WinCopyRectangle
void HiResDrawBitmap( BitmapType* bitmapP, Coord x, Coord y )
{
    if ( IsHiResTypeSony( hiResType ) )
        HRWinDrawBitmap( sonyHiResRefNum, bitmapP, x, y );
    else
        WinDrawBitmap( bitmapP, x, y );
}


void HiResDrawChars( const Char* chars, UInt16 len, Coord x, Coord y )
{
    if ( IsHiResTypeSony( hiResType ) )
        HRWinDrawChars( sonyHiResRefNum, chars, len, x, y );
    else
        WinDrawChars( chars, len, x, y );
}

void HiResCopyRectangle( WinHandle srcWin, WinHandle dstWin, RectangleType* srcRect,
     Coord destX, Coord destY, WinDrawOperation mode )
{
    if ( IsHiResTypeSony( hiResType ) )
        HRWinCopyRectangle( sonyHiResRefNum, srcWin, dstWin, srcRect, destX, destY, mode );
    else
        WinCopyRectangle( srcWin, dstWin, srcRect, destX, destY, mode );
}
#endif

