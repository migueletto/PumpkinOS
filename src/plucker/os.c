/*
 * $Id: os.c,v 1.88 2004/04/18 15:34:48 prussar Exp $
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
 *
 */

#include <PalmOS.h>
#include <VFSMgr.h>
#include <CharLatin.h>
#include <TxtGlue.h>

#include "SysZLib.h"
#include "const.h"
#include "debug.h"
#include "font.h"
#include "hires.h"
#include "jogdial.h"
#include "resourceids.h"
#include "screen.h"
#include "fullscreenform.h"
#include "image.h"
#include "axxpacimp.h"
#include "skins.h"

#include "os.h"

#ifdef HAVE_PALMCUNIT
#include "mock_Header.h"
#endif


/***********************************************************************
 *
 *      Internal Constants
 *
 ***********************************************************************/
#define HIGH_DENSITY_FEATURE_SET_VERSION        4

#define MAX_CHARACTER_LENGTH 32 /* multi-byte characters cannot
                                   be longer than this */
static const UInt32 RomVersion20 = sysMakeROMVersion( 2, 0, 0,
                                    sysROMStageRelease, 0 );
static const UInt32 RomVersion30 = sysMakeROMVersion( 3, 0, 0,
                                    sysROMStageRelease, 0 );
static const UInt32 RomVersion31 = sysMakeROMVersion( 3, 1, 0,
                                    sysROMStageRelease, 0 );
static const UInt32 RomVersion33 = sysMakeROMVersion( 3, 3, 0,
                                    sysROMStageRelease, 0 );
static const UInt32 RomVersion35 = sysMakeROMVersion( 3, 5, 0,
                                    sysROMStageRelease, 0 );
static const UInt32 RomVersion40 = sysMakeROMVersion( 4, 0, 0,
                                    sysROMStageRelease, 0 );
/* The Tungsten T was actually released in the development stage?!? */
static const UInt32 RomVersion50 = sysMakeROMVersion( 5, 0, 0,
                                    sysROMStageDevelopment, 0 );


/***********************************************************************
 *
 *      Internal Types
 *
 ***********************************************************************/

typedef struct {
    UInt32   unicodeValue;      /* the Unicode value for the character */
    UInt16   palmCharValue;     /* the Palm charset character value */
} CharMapping;

/* a table of some of the non-Latin-1 characters supported in Palm's
   Latin-1 charset */
static CharMapping Latin1Mapping[] = {
  { 8211, chrEnDash },
  { 8212, chrEmDash },
  { 8216, chrLeftSingleQuotationMark },
  { 8217, chrRightSingleQuotationMark },
  { 8220, chrLeftDoubleQuotationMark },
  { 8221, chrRightDoubleQuotationMark },
  { 8226, chrBullet },
  { 8230, chrHorizontalEllipsis },
  { 8442, chrTradeMarkSign },
  {    0, 0 }
};


/***********************************************************************
 *
 *      Local functions
 *
 ***********************************************************************/
static void OS2(void);
static void OS30(void);
static void OS33(void);
static void OS35(void);
static void OS4(void);
static void OS5(void);
static Boolean IsDoubleByteSingleChar( UInt16 word );



/***********************************************************************
 *
 *      Private variables
 *
 ***********************************************************************/
static UInt32       romVersion;
static Boolean      supportZLib;
static Boolean      supportBeam;
static Boolean      supportVFS;
static Boolean      supportGraffiti2;
static Boolean      supportArmlets;
static Boolean      have68K   = true;
static UInt32       maxBitDepth;
static UInt32       charEncoding = charEncodingPalmLatin;
static UInt32       notifySupport;
static Boolean      support20 = false;
static Boolean      support30 = false;
static Boolean      support31 = false;
static Boolean      support33 = false;
static Boolean      support35 = false;
static Boolean      support40 = false;
static Boolean      support50 = false;
static UInt16       supportedCompression;
static Boolean      supportHighDensity = false;
static Boolean      uses8BitChars = true;
#if defined( HAVE_FIVEWAY_SDK ) || defined( HAVE_HANDSPRING_SDK )
static Boolean      haveFiveWay   = false;
#endif
#ifdef HAVE_HANDSPRING_SDK
static Boolean      haveHsNav     = false;
#endif
static Boolean      isSony        = false;
static Boolean      haveWhiteBackground = true;



/* check whether the two bytes passed here encode a single
   character in the current encoding */
static Boolean IsDoubleByteSingleChar
    (
    UInt16 word
    )
{
    Char s[ MAX_CHARACTER_LENGTH ];
    MemSet( s, MAX_CHARACTER_LENGTH, 0 );
    s[ 0 ] = word >> 8;
    s[ 1 ] = word & 0xFF;
    return 1 < TxtGlueGetNextChar( s, 0, NULL );
}



Boolean SupportCompressionType
    (
    BitmapCompressionType type
    )
{
    return ( ( ( 1 << type ) & supportedCompression ) != 0 );
}



Boolean Support20( void )
{
    return support20;
}



Boolean Support30( void )
{
    return support30;
}



Boolean Support31( void )
{
    return support31;
}



Boolean Support33( void )
{
    return support33;
}



Boolean Support35( void )
{
    return support35;
}



Boolean Support40( void )
{
    return support40;
}



Boolean Support50( void )
{
    return support50;
}



/* Return max bit depth for this OS */
UInt32 GetMaxBitDepth( void )
{
    return maxBitDepth;
}



/* Return the device's character encoding */
UInt32 GetCharEncoding( void )
{
    return charEncoding;
}



/* Do we have a Jogdial controller? */
Boolean HaveJogdial( void )
{
    return ( JogdialType() != noJogdial );
}



/* Do we support the high density feature set? */
Boolean SupportHighDensity( void )
{
    return supportHighDensity;
}



/* Do we have a Handspring's FiveWay controller? */
Boolean HaveHsNav( void )
{
#ifdef HAVE_HANDSPRING_SDK
    return haveHsNav;
#else
    return false;
#endif
}



/* Do we have a FiveWay controller? */
Boolean HaveFiveWay( void )
{
#if defined( HAVE_FIVEWAY_SDK ) || defined( HAVE_HANDSPRING_SDK )
    return haveFiveWay;
#else
    return false;
#endif
}



/* Do we have an 8-bit character set? */
Boolean DeviceUses8BitChars( void )
{
    return uses8BitChars;
}



/* Is this a Sony? */
Boolean IsSony( void )
{
    return isSony;
}



Boolean SupportNotification(void)
{
    return (notifySupport != 0);
}


/* Return status for ZLib support */
Boolean SupportZLib( void )
{
    return supportZLib;
}



/* Return status for beam support */
Boolean SupportBeam( void )
{
    return supportBeam;
}



/* Return status for VFS support */
Boolean SupportVFS( void )
{
    return supportVFS;
}



/* Return status for Graffiti 2 support */
Boolean SupportGraffiti2( void )
{
    return supportGraffiti2;
}



/* Return status for Armlet support */
Boolean SupportArmlets( void )
{
    return supportArmlets;
}



/* Do we have a 68K processor? */
Boolean Have68K( void )
{
    return have68K;
}



/* Do we have a white background?  (Only supported for OS3.5+) */
Boolean HaveWhiteBackground( void )
{
    return haveWhiteBackground;
}



/* Check to see if we have a Palm character for the given Unicode char */
UInt16 FindPalmCharForUnicodeChar
    (
    UInt32 charValue
    )
{
    UInt16 i;
    UInt16 entries;

    if ( charEncoding != charEncodingPalmLatin )
        return 0;

    entries = sizeof(Latin1Mapping)/sizeof(CharMapping);

    for ( i = 0 ;  i < entries;  i++ ) {
        if ( Latin1Mapping [ i ].unicodeValue == 0 )
            return 0;
        else if ( charValue < Latin1Mapping [ i ].unicodeValue )
            return 0;
        else if ( Latin1Mapping [ i ].unicodeValue == charValue )
            return Latin1Mapping[ i ].palmCharValue;
    }
    return 0;
}



/* PalmOS 2.x or higher specific operations. */
static void OS2( void )
{
    Err       err;
#if defined( HAVE_FIVEWAY_SDK ) || defined( HAVE_HANDSPRING_SDK ) || defined( HAVE_SONY_SDK )
    UInt32    value;
#endif

    /* Do we support the Notification Manager? */
    err = FtrGet(sysFtrCreator, sysFtrNumNotifyMgrVersion, &notifySupport);
    if (err != errNone)
        notifySupport = 0;

    maxBitDepth             = 1;
    SetScreenMode           = SetScreenModeOS2;
    SetDefaultScreenMode    = SetDefaultScreenModeOS2;
    SetForeColor            = SetForeColor_OS2;
    SaveDrawState           = SaveDrawState_OS2;
    RestoreDrawState        = RestoreDrawState_OS2;
    ScreenLock              = ScreenLockUnlock_None;
    ScreenUnlock            = ScreenLockUnlock_None;
    OptimizeImage           = OptimizeImage_None;

#ifdef HAVE_FIVEWAY_SDK
    err = FtrGet( navFtrCreator, navFtrVersion, &value );
    if ( err == errNone )
        haveFiveWay = true;
#endif
#ifdef HAVE_HANDSPRING_SDK
    err = FtrGet( sysFileCSystem, sysFtrNumUIHardwareHas5Way, &value );
    if ( err == errNone ) {
        haveFiveWay = true;
        err = FtrGet( hsFtrCreator, hsFtrIDNavigationSupported, &value );
        if ( err == errNone ) {
            haveHsNav = true;
        }
    }
#endif
#ifdef HAVE_SONY_SDK
    err = FtrGet( sysFtrCreator, sysFtrNumOEMCompanyID, &value );
    if ( err == errNone && value ==  sonyHwrOEMCompanyID_Sony ) {
        isSony = true;
    }
#endif
    if ( IsDoubleByteSingleChar( testDoubleByteBig5GB2312EUCJPKR ) ||
         IsDoubleByteSingleChar( testDoubleByteShiftJIS ) ||
         IsDoubleByteSingleChar( testDoubleByteJISKuten ) ) {
        uses8BitChars              = false;
    }
}



/* PalmOS 3.0 or higher specific operations */
static void OS30( void )
{
    Err     err;
    UInt32  vfsMgrVersion;
    UInt32  supportedDepths;
    UInt16  i;

    WinScreenMode( winScreenModeGetSupportedDepths, NULL, NULL,
        &supportedDepths, NULL );

    for ( i = 1; supportedDepths != 0; i++ ) {
        if (( supportedDepths & 0x01 ) == 0x01 ) {
            maxBitDepth = i;
        }
        supportedDepths >>= 1;
    }

    /* Palm OS is smart enough to format an image for a higher bitdepth
       than the device might support */
    if ( maxBitDepth < 2 )
    {
        maxBitDepth = 2;
    }

    SetForeColor        = SetForeColor_OS3;
    SaveDrawState       = SaveDrawState_OS3;
    RestoreDrawState    = RestoreDrawState_OS3;

    SetScreenMode           = SetScreenModeOS3;
    SetDefaultScreenMode    = SetDefaultScreenModeOS3;

    supportBeam = true; /* TODO: Add check for beam support since it
                           could be missing on an OS3+ device */

#if 0
   err = ZLSetup;
   if ( err == errNone ) {
        supportZLib = true;
    }
    else {
        supportZLib = false;
    }
#endif
    supportZLib = true;

#ifdef HAVE_AXXPAC
    err = InitializeAxxPac();
    if ( err != errNone )
#endif
    err = FtrGet( sysFileCVFSMgr, vfsFtrIDVersion, &vfsMgrVersion );
    if ( err == errNone )
        supportVFS = true;
    else
        supportVFS = false;

    LoadCustomFonts( STANDARD_FONTS );

    /* confirm that we are using 8 bit chars */
    if ( uses8BitChars ) {
        err = FtrGet( sysFtrCreator, sysFtrNumEncoding, &charEncoding );
        if ( err != errNone )
            charEncoding = charEncodingPalmLatin;  /* default encoding */
        else if ( charEncodingPalmLatin < charEncoding )
            uses8BitChars = false;
    }
}



/* PalmOS 3.3 or higher specific operations */
static void OS33( void )
{
    /* Palm OS is smart enough to format an image for a higher bitdepth
       than the device might support */
    if ( maxBitDepth < 4 )
    {
        maxBitDepth = 4;
    }

    supportedCompression = 1 << BitmapCompressionTypeScanLine;
}



/* PalmOS 3.5 or higher specific operations */
static void OS35( void )
{
    RGBColorType backColor;

    /* Palm OS is smart enough to format an image for a higher bitdepth
       than the device might support */
    if ( maxBitDepth < 8 )
    {
        maxBitDepth = 8;
    }

    SetForeColor        = SetForeColor_OS35;
    SaveDrawState       = SaveDrawState_OS35;
    RestoreDrawState    = RestoreDrawState_OS35;
    ScreenLock          = ScreenLock_OS35;
    ScreenUnlock        = ScreenUnlock_OS35;
    OptimizeImage       = OptimizeImage_OS35;

    WinIndexToRGB( UIColorGetTableEntryIndex( UIFormFill ), &backColor );
    haveWhiteBackground = ( backColor.r == 255 && backColor.g == 255 &&
                              backColor.b == 255 );

#ifdef HAVE_SONY_SDK
    /* Set Plucker-specific JogAssist mask if applicable */
    HandleJogAssistMask( true );
#endif

    supportedCompression |= ( 1 << BitmapCompressionTypeRLE );
}



/* PalmOS 4.x or higher specific operations */
static void OS4( void )
{
    Err    err;
    UInt32 version;
    UInt32 empty;

    /* Palm OS is smart enough to format an image for a higher bitdepth
       than the device might support */
    if ( maxBitDepth < 16 )
    {
        maxBitDepth = 16;
    }

    if ( ( FtrGet( 'grft', 1110, &empty ) == errNone ) ||
         ( FtrGet( 'grf2', 1110, &empty ) == errNone ) )
        supportGraffiti2 = true;
    else
        supportGraffiti2 = false;

    supportedCompression |= ( 1 << BitmapCompressionTypePackBits );

    err = FtrGet( sysFtrCreator, sysFtrNumWinVersion, &version );

    supportHighDensity = ( err == errNone &&
                             HIGH_DENSITY_FEATURE_SET_VERSION <= version );
}



/* PalmOS 5.x or higher specific operations */
static void OS5( void )
{
    Err    err;
    UInt32 processorType;

    err = FtrGet( sysFileCSystem, sysFtrNumProcessorID, &processorType );
    if ( err == errNone )
        have68K = sysFtrNumProcessorIs68K( processorType );
#ifdef BUILD_ARMLETS
    if ( err == errNone && sysFtrNumProcessorIsARM( processorType ) )
        supportArmlets = true;
    else
#endif
        supportArmlets = false;

    supportedCompression ^= ( 1 << BitmapCompressionTypePackBits );
}



Err RomVersionCompatible
    (
    UInt32 reqVersion
    )
{
    Err err = errNone;
    if ( romVersion < reqVersion ) {
        MSG( _( "wrong ROM version detected ( %lx )\n", romVersion ) );
        FrmAlert( infoWrongROMVersion );
        if ( sysGetROMVerMajor( romVersion ) == 0x01 )
            AppLaunchWithCommand( sysFileCDefaultApp,
                sysAppLaunchCmdNormalLaunch, NULL );
        return sysErrRomIncompatible;
    }
    return errNone;
}



/* Initialize OS specific features */
void OS_Init( void )
{
#ifdef HAVE_PALMCUNIT
    /* When we run the test version we might call OS_Init more than
       once during a session, so we have to reset these flags */
    support50 = false;
    support40 = false;
    support35 = false;
    support33 = false;
    support31 = false;
    support30 = false;
    support20 = false;
#endif

    OpenSkins();

    FtrGet( sysFtrCreator, sysFtrNumROMVersion, &romVersion );
    if ( RomVersion20 <= romVersion ) {
        support20 = true;
    }
    if ( RomVersion30 <= romVersion ) {
        support30 = true;
    }
    if ( RomVersion31 <= romVersion ) {
        support31 = true;
    }
    if ( RomVersion33 <= romVersion ) {
        support33 = true;
    }
    if ( RomVersion35 <= romVersion ) {
        support35 = true;
    }
    if ( RomVersion40 <= romVersion ) {
        support40 = true;
    }
    if ( RomVersion50 <= romVersion ) {
        support50 = true;
    }

    if (support20) OS2();
    if (support30) OS30();
    if (support33) OS33();
    if (support35) OS35();
    if (support40) OS4();
    if (support50) OS5();

    InitializeUserFontDBs();
}



/* Release OS specific features. */
void OS_Release( void )
{
    if ( HiResStop != NULL )
        HiResStop();

#ifdef HAVE_SONY_SDK
    /* Restore original JogAssist mask */
    if ( support35 )
        HandleJogAssistMask( false );
#endif

    ReleaseCustomFonts();

#ifdef HAVE_AXXPAC
    TeardownAxxPac();
#endif
    //ZLTeardown;
    DeinitializeUserFontDBs();
    CloseSkins();
}

