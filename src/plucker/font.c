/*
 * $Id: font.c,v 1.48 2004/02/29 20:31:58 prussar Exp $
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

#include "debug.h"
#include "hires.h"
#include "list.h"
#include "os.h"
#include "prefsdata.h"
#include "util.h"
#include "const.h"
#include "grayfont.h"

#include "font.h"
#include "../libpit/debug.h"

#ifdef SUPPORT_VFS_FONTS
#include <VFSMgr.h>
#include "vfsfile.h"
#endif

/***********************************************************************
 *
 *      Internal Constants
 *
 ***********************************************************************/
#define EOL 0xFF




/***********************************************************************
 *
 *      Local functions
 *
 ***********************************************************************/
/* This should be called whenever the font package is changed */
static void ClearUserFontNameList( void );
static void SortAndMakeUserFontNameList( void );
#ifdef SUPPORT_VFS_FONTS
static void VFSFontCacheAddEntry ( const Char* name, const Char* volumeLabel,
       const Char* path, Int16 cache );
static Boolean VFSFontCacheFindEntry ( const Char* name );
#endif       




/***********************************************************************
 *
 *      Internal Types
 *
 ***********************************************************************/
typedef struct {
    FontModeType  fontMode;
    FontID*       style;
} StyleType;


typedef struct {
    DmResType     resourceType;
    DmResID       resourceID;
    FontID        fontID;
    FontID        substituteID;
} CustomFontType;


typedef struct
{
    UInt16     cardNo;
    LocalID    dbID;
    Char       name[ dmDBNameLength ];
    Boolean    onVFS;
    Char*      path;
    Char*      volumeLabel;
}
DBEntryType;


/* PalmOS 2 Fonts */
static FontID os2StandardStyles[ MAXSTYLES ] = {
    stdFont, boldFont, boldFont, boldFont,
    boldFont, stdFont, stdFont, stdFont, stdFont,
    stdFont, stdFont, stdFont
};

static FontID os2BoldStyles[ MAXSTYLES ] = {
    boldFont, boldFont, boldFont, boldFont,
    boldFont, boldFont, boldFont, boldFont, stdFont,
    boldFont, boldFont, boldFont
};

/* PalmOS 3+ Fonts */
static FontID os3NarrowStyles[ MAXSTYLES ] = {
    narrowFont, largeBoldFont, largeBoldFont, largeFont,
    largeFont, narrowBoldFont, narrowBoldFont, narrowBoldFont, narrowFixedFont,
    narrowFont, narrowFont, narrowFont
};

static FontID os3StandardStyles[ MAXSTYLES ] = {
    stdFont, largeBoldFont, largeBoldFont, largeFont,
    largeFont, boldFont, boldFont, boldFont, narrowFixedFont,
    stdFont, narrowFont, narrowFont
};

static FontID os3BoldStyles[ MAXSTYLES ] = {
    boldFont, largeBoldFont, largeBoldFont, largeBoldFont,
    largeBoldFont, boldFont, boldFont, boldFont, narrowFixedFont,
    stdFont, narrowBoldFont, narrowBoldFont
};

static FontID os3LargeStyles[ MAXSTYLES ] = {
    largeFont, largeBoldFont, largeBoldFont, largeFont,
    largeFont, largeBoldFont, largeBoldFont, largeBoldFont, narrowFixedFont,
    stdFont, stdFont, stdFont
};

static FontID os3LargeBoldStyles[ MAXSTYLES ] = {
    largeBoldFont, largeBoldFont, largeBoldFont, largeBoldFont,
    largeBoldFont, largeBoldFont, largeBoldFont, largeBoldFont, narrowFixedFont,
    boldFont, boldFont, boldFont
};

static FontID os3UserStyles[ MAXSTYLES ] = {
    userStdFont, userLargeBoldFont, userLargeBoldFont, userLargeFont,
    userLargeFont, userBoldFont, userBoldFont, userBoldFont, userFixedFont,
    userStdFont, userNarrowFont, userNarrowFont
};

/* Palm HiRes Fonts */
#ifdef HAVE_HIRES
static FontID palmHiResTinyStyles[ MAXSTYLES ] = {
    tinyFont_palm, boldFont, boldFont, smallBoldFont_palm,
    smallBoldFont_palm, tinyBoldFont_palm, tinyBoldFont_palm, tinyBoldFont_palm,
    stdFixedFont_palm, tinyFont_palm, tinyFont_palm, tinyFont_palm
};

static FontID palmHiResSmallStyles[ MAXSTYLES ] = {
    smallFont_palm, largeBoldFont, largeBoldFont, boldFont,
    boldFont, smallBoldFont_palm, smallBoldFont_palm, smallBoldFont_palm,
    stdFixedFont_palm, tinyFont_palm, tinyFont_palm, tinyFont_palm
};

static FontID palmHiResNarrowStyles[ MAXSTYLES ] = {
    narrowFont, largeBoldFont, largeBoldFont, boldFont,
    boldFont, narrowBoldFont, narrowBoldFont, narrowBoldFont,
    stdFixedFont_palm, narrowFont, narrowFont, narrowFont
};

static FontID palmHiResStandardStyles[ MAXSTYLES ] = {
    stdFont, largeBoldFont, largeBoldFont, boldFont,
    boldFont, boldFont, smallBoldFont_palm, boldFont,
    stdFixedFont_palm, stdFont, narrowFont, narrowFont
};

static FontID palmHiResLargeStyles[ MAXSTYLES ] = {
    largeFont, largeBoldFont, largeBoldFont, largeFont,
    largeFont, boldFont, boldFont, largeBoldFont, stdFixedFont_palm,
    stdFont, stdFont, stdFont
};

static FontID palmHiResUserStyles[ MAXSTYLES ] = {
    userStdFont_palm, userLargeBoldFont_palm, userLargeBoldFont_palm,
    userLargeFont_palm,
    userLargeFont_palm, userBoldFont_palm, userBoldFont_palm, userBoldFont_palm,
    userFixedFont_palm, userStdFont_palm, userNarrowFont_palm,
    userNarrowFont_palm
};

/* Sony HiRes Fonts */
#ifdef HAVE_SONY_SDK
static FontID sonyHiResTinyStyles[ MAXSTYLES ] = {
    hrTinyFont, hrSmallBoldFont, hrSmallBoldFont, hrTinyBoldFont,
    hrTinyBoldFont, hrTinyBoldFont, hrTinyBoldFont, hrTinyBoldFont,
    narrowFixedFont, narrowFont, narrowFont, narrowFont
};

static FontID sonyHiResSmallStyles[ MAXSTYLES ] = {
    hrSmallFont, hrBoldFont, hrBoldFont, hrSmallBoldFont,
    hrSmallBoldFont, hrTinyBoldFont, hrTinyBoldFont, hrSmallBoldFont,
    narrowFixedFont, hrTinyFont, hrTinyFont, hrTinyFont
};

static FontID sonyHiResNarrowStyles[ MAXSTYLES ] = {
    narrowFont, largeBoldFont, largeBoldFont, largeFont,
    largeFont, narrowBoldFont, narrowBoldFont, narrowBoldFont, narrowFixedFont,
    narrowFont, narrowFont, narrowFont
};

static FontID sonyHiResStandardStyles[ MAXSTYLES ] = {
    hrStdFont, hrLargeBoldFont, hrLargeBoldFont, hrBoldFont,
    hrBoldFont, hrSmallBoldFont, hrSmallBoldFont, hrBoldFont,
    stdFixedFont_sony, hrSmallFont, hrSmallFont, hrSmallFont
};

static FontID sonyHiResLargeStyles[ MAXSTYLES ] = {
    hrLargeFont, hrLargeBoldFont, hrLargeBoldFont, hrLargeBoldFont,
    hrLargeBoldFont, hrBoldFont, hrBoldFont, hrLargeBoldFont,
    stdFixedFont_sony, hrStdFont, hrStdFont, hrStdFont
};

static FontID sonyHiResUserStyles[ MAXSTYLES ] = {
    userStdFont_sony, userLargeBoldFont_sony, userLargeBoldFont_sony,
    userLargeFont_sony,
    userLargeFont_sony, userBoldFont_sony, userBoldFont_sony,
    userBoldFont_sony, userFixedFont_sony,
    userStdFont_sony, userNarrowFont_sony, userNarrowFont_sony
};
#endif /* HAVE_SONY_SDK */

/* Handera HiRes Fonts */
#ifdef HAVE_HANDERA_SDK
/* These need to be built at run-time by HanderaConvertFontList() */
static FontID handeraHiResStandardStyles[ MAXSTYLES ] = {};
static FontID handeraHiResLargeStyles[ MAXSTYLES ] = {};
#endif /* HAVE_HANDERA_SDK */

#endif /* HAVE_HIRES */

static Boolean haveItalic[ NUM_ITALIC_USER_FONTS ];


/***********************************************************************
 *
 *      Private variables
 *
 ***********************************************************************/
static FontID*      StylesLibrary;
static FontID*      StylesMain;
static Int16        currentStyle        = DEFAULTSTYLE;
static Int16        currentStyleFont    = stdFont;
static Int16        prevFontHeight      = -1;
#ifdef SUPPORT_VFS_FONTS
static Char         vfsFontCacheName[]  = "PlkrVFSFontCache";
static Boolean      scannedVFS          = false;
#endif


static CustomFontType ListOfCustomStandardFonts[] = {
    { 'NFNT', narrowFontID, narrowFont, stdFont },
    { 'NFNT', narrowFixedFontID, narrowFixedFont, boldFont },
    { 'NFNT', narrowBoldFontID, narrowBoldFont, boldFont },
    { 'NFNT', userStdFontID, userStdFont, stdFont },
    { 'NFNT', userBoldFontID, userBoldFont, boldFont },
    { 'NFNT', userLargeFontID, userLargeFont, largeFont },
    { 'NFNT', userLargeBoldFontID, userLargeBoldFont, largeBoldFont },
    { 'NFNT', userFixedFontID, userFixedFont, boldFont },
    { 'NFNT', userNarrowFontID, userNarrowFont, stdFont },
    { 'NFNT', userStdFontID | ITALIC_FONTID_MASK,
                  userStdFont + ITALIC_FONT_DELTA, stdFont },
    { 'NFNT', userBoldFontID | ITALIC_FONTID_MASK,
                  userBoldFont + ITALIC_FONT_DELTA, boldFont },
    { 'NFNT', userLargeFontID | ITALIC_FONTID_MASK,
                  userLargeFont + ITALIC_FONT_DELTA, largeFont },
    { 'NFNT', userLargeBoldFontID | ITALIC_FONTID_MASK,
                  userLargeBoldFont + ITALIC_FONT_DELTA, largeBoldFont },
    { 'NFNT', userFixedFontID | ITALIC_FONTID_MASK,
                  userFixedFont + ITALIC_FONT_DELTA, narrowFixedFont },
    { 'NFNT', userNarrowFontID | ITALIC_FONTID_MASK,
                  userNarrowFont + ITALIC_FONT_DELTA, narrowFont },
    //{ EOL, NULL, NULL }
    { EOL, 0, 0 }
};

#ifdef HAVE_HIRES
static CustomFontType ListOfCustomPalmHiResFonts[] = {
    { 'nfnt', tinyFontID_palm, tinyFont_palm, stdFont },
    { 'nfnt', tinyBoldFontID_palm, tinyBoldFont_palm, stdFont },
    { 'nfnt', smallFontID_palm, smallFont_palm, stdFont },
    { 'nfnt', smallBoldFontID_palm, smallBoldFont_palm, boldFont },
    { 'nfnt', stdFixedFontID_palm, stdFixedFont_palm, boldFont },
    { 'nfnt', userStdFontID_palm, userStdFont_palm, stdFont },
    { 'nfnt', userBoldFontID_palm, userBoldFont_palm, boldFont },
    { 'nfnt', userLargeFontID_palm, userLargeFont_palm, largeFont },
    { 'nfnt', userLargeBoldFontID_palm, userLargeBoldFont_palm, largeBoldFont },
    { 'nfnt', userFixedFontID_palm, userFixedFont_palm, stdFixedFont_palm },
    { 'nfnt', userNarrowFontID_palm, userNarrowFont_palm, stdFont },
    { 'nfnt', userStdFontID_palm | ITALIC_FONTID_MASK,
                  userStdFont_palm + ITALIC_FONT_DELTA, stdFont },
    { 'nfnt', userBoldFontID_palm | ITALIC_FONTID_MASK,
                  userBoldFont_palm + ITALIC_FONT_DELTA, boldFont },
    { 'nfnt', userLargeFontID_palm | ITALIC_FONTID_MASK,
                  userLargeFont_palm + ITALIC_FONT_DELTA, largeFont },
    { 'nfnt', userLargeBoldFontID_palm | ITALIC_FONTID_MASK,
                  userLargeBoldFont_palm + ITALIC_FONT_DELTA, largeBoldFont },
    { 'nfnt', userFixedFontID_palm | ITALIC_FONTID_MASK,
                  userFixedFont_palm + ITALIC_FONT_DELTA, stdFixedFont_palm },
    { 'nfnt', userNarrowFontID_palm | ITALIC_FONTID_MASK,
                  userNarrowFont_palm + ITALIC_FONT_DELTA, stdFont },
    //{ EOL, NULL, NULL }
    { EOL, 0, 0 }
};

#ifdef HAVE_SONY_SDK
static CustomFontType ListOfCustomSonyHiResFonts[] = {
    { 'NFNT', stdFixedFontID_sony, stdFixedFont_sony, hrBoldFont },
    { 'NFNT', userStdFontID_sony, userStdFont_sony, hrStdFont },
    { 'NFNT', userBoldFontID_sony, userBoldFont_sony, hrBoldFont },
    { 'NFNT', userLargeFontID_sony, userLargeFont_sony, hrLargeFont },
    { 'NFNT', userLargeBoldFontID_sony, userLargeBoldFont_sony, hrLargeBoldFont },
    { 'NFNT', userFixedFontID_sony, userFixedFont_sony, stdFixedFont_sony },
    { 'NFNT', userNarrowFontID_sony, userNarrowFont_sony, hrStdFont },
    { 'NFNT', userStdFontID_sony | ITALIC_FONTID_MASK,
                  userStdFont_sony + ITALIC_FONT_DELTA, hrStdFont },
    { 'NFNT', userBoldFontID_sony | ITALIC_FONTID_MASK,
                  userBoldFont_sony + ITALIC_FONT_DELTA, hrBoldFont },
    { 'NFNT', userLargeFontID_sony | ITALIC_FONTID_MASK,
                  userLargeFont_sony + ITALIC_FONT_DELTA, hrLargeFont },
    { 'NFNT', userLargeBoldFontID_sony | ITALIC_FONTID_MASK,
                  userLargeBoldFont_sony + ITALIC_FONT_DELTA, hrLargeBoldFont },
    { 'NFNT', userFixedFontID_sony | ITALIC_FONTID_MASK,
                  userFixedFont_sony + ITALIC_FONT_DELTA, stdFixedFont_sony },
    { 'NFNT', userNarrowFontID_sony | ITALIC_FONTID_MASK,
                  userNarrowFont_sony + ITALIC_FONT_DELTA, hrStdFont },
    //{ EOL, NULL, NULL }
    { EOL, 0, 0 }
};
#endif /* HAVE_SONY_SDK */

#endif /* HAVE_HIRES */

static StyleType ListOfFontStylesOS2[] = {
    { FONT_DEFAULT, os2StandardStyles },
    { FONT_BOLD, os2BoldStyles },
    //{ EOL, NULL }
    { EOL, 0 }
};

static StyleType ListOfFontStylesOS3[] = {
    { FONT_NARROW, os3NarrowStyles },
    { FONT_DEFAULT, os3StandardStyles },
    { FONT_BOLD, os3BoldStyles },
    { FONT_LARGE, os3LargeStyles },
    { FONT_LARGEBOLD, os3LargeBoldStyles },
    { FONT_USER, os3UserStyles },
    //{ EOL, NULL }
    { EOL, 0 }
};

#ifdef HAVE_HIRES
static StyleType ListOfFontStylesPalm[] = {
    { FONT_TINY, palmHiResTinyStyles },
    { FONT_SMALL, palmHiResSmallStyles },
    { FONT_NARROW, palmHiResNarrowStyles },
    { FONT_DEFAULT, palmHiResStandardStyles },
    { FONT_LARGE, palmHiResLargeStyles },
    { FONT_USER, palmHiResUserStyles },
    //{ EOL, NULL }
    { EOL, 0 }
};

#ifdef HAVE_SONY_SDK
static StyleType ListOfFontStylesSony[] = {
    { FONT_TINY, sonyHiResTinyStyles },
    { FONT_SMALL, sonyHiResSmallStyles },
    { FONT_NARROW, sonyHiResNarrowStyles },
    { FONT_DEFAULT, sonyHiResStandardStyles },
    { FONT_LARGE, sonyHiResLargeStyles },
    { FONT_USER, sonyHiResUserStyles },
    //{ EOL, NULL }
    { EOL, 0 }
};
#endif /* HAVE_SONY_SDK */

#ifdef HAVE_HANDERA_SDK
static StyleType ListOfFontStylesHandera[] = {
    { FONT_TINY, os3StandardStyles },
    { FONT_SMALL, os3LargeStyles },
    { FONT_NARROW, os3NarrowStyles },
    { FONT_DEFAULT, handeraHiResStandardStyles },
    { FONT_LARGE, handeraHiResLargeStyles },
    //{ EOL, NULL }
    { EOL, 0 }
};
#endif /* HAVE_HANDERA_SDK */

#endif /* HAVE_HIRES */

static LinkedList userFontDBList          = NULL;
static Int16      numberOfUserFonts       = 0;
static Char**     userFontNames           = NULL;
static Char*      emptyUserFontNameList[] = {
    "--"
};
static DmOpenRef  currentUserFontDBRef;
static Int16      currentUserFontNumber   = NO_SUCH_USER_FONT;
static DeviceFontType  currentDeviceFont;
#ifdef SUPPORT_VFS_FONTS
static Boolean    currentFontIsOnVFS = false;
static FileRef    currentVFSFontRef;
#endif


/* Load custom fonts into memory */
void LoadCustomFonts
    (
    DeviceFontType  deviceFont
    )
{
    CustomFontType* fontList;
    FontType*       font;

    currentDeviceFont = deviceFont;

    GrayFntClearSubstitutionList();

    ReleaseCustomFonts();

    switch ( deviceFont ) {
        case STANDARD_FONTS:
            fontList = ListOfCustomStandardFonts;
            break;
#ifdef HAVE_HIRES
        case PALM_FONTS:
            fontList = ListOfCustomPalmHiResFonts;
            break;
#ifdef HAVE_SONY_SDK
        case SONY_FONTS:
            fontList = ListOfCustomSonyHiResFonts;
            break;
#endif
#endif
        default:
            return;
    }

    for ( ; fontList->resourceType != EOL; fontList++ ) {
        MemHandle        fontHandle;
        FontResourceKind kind;
        fontHandle = GetFontResource( fontList->resourceType,
                                      fontList->resourceID,
                                      &kind );
        if ( fontHandle != NULL ) {
            Err err;
            font   = MemHandleLock( fontHandle );
            debug(DEBUG_INFO, "Plucker", "FntDefineFont resID %d font %d", fontList->resourceID, fontList->fontID);
            if ((err = FntDefineFont( fontList->fontID, font ) != errNone)) {
              debug(DEBUG_ERROR, "Plucker", "FntDefineFont error %d", err);
            }
            MemHandleUnlock( fontHandle );
            if ( FONT_IS_ITALIC( fontList->fontID ) ) {
                haveItalic[ fontList->fontID - START_ITALIC_USER_FONTS ] = true;
            }
        }
        else {
            if ( FONT_IS_ITALIC( fontList->fontID ) ) {
                haveItalic[ fontList->fontID - START_ITALIC_USER_FONTS ] =
                                                                          false;
            }
            else {
                GrayFntSubstitute( fontList->fontID, fontList->substituteID );
            }
        }
        ReleaseFontResource( fontHandle, kind );
    }
}



/* Release custom fonts from memory */
void ReleaseCustomFonts( void )
{
}



/* Refresh custom fonts in memory */
void RefreshCustomFonts( void )
{
    if ( Support30() )
        LoadCustomFonts( currentDeviceFont );
}



/* Return the current font style */
Int16 GetCurrentStyle( void )
{
    return currentStyle;
}



/* Return the previous font style */
Int16 GetPrevFontHeight( void )
{
    if ( prevFontHeight < 0 )
        return FntCharHeight(); /* FIXME! */
    else
        return prevFontHeight;
}



/* Set the previous font height */
void SetPrevFontHeight
      (
      Int16  height
      )
{
      prevFontHeight = height;
}



/* Return main style font */
FontID GetMainStyleFont
    (
    const Int16 style   /* numerical representation for font style */
    )
{
    return StylesMain[ style ];
}



/* Get the default style font height */
Int16 GetDefaultMainStyleHeight( void )
{
    FontID oldFont;
    Int16  height;
    UInt16 prevCoordSys;

    prevCoordSys = PalmSetCoordinateSystem( NATIVE );

    oldFont = FntGetFont();
    FntSetFont( StylesMain[ DEFAULTSTYLE ] );
    height = FntLineHeight() + Prefs()->lineSpacing;
    FntSetFont( oldFont );

    PalmSetCoordinateSystem( prevCoordSys );
    
    return height;
}



/* Set and return main style font */
FontID SetMainStyleFont
    (
    const Int16 style   /* numerical representation for font style */
    )
{
    FntSetFont ( StylesMain[ currentStyle ] );
    prevFontHeight   = FntCharHeight();
    currentStyle     = style;
    currentStyleFont = StylesMain[ currentStyle ];
    FntSetFont( currentStyleFont );
    return currentStyleFont;
}



/* Return library style font */
FontID GetLibraryStyleFont
    (
    const Int16 style   /* numerical representation for font style */
    )
{
    return StylesLibrary[ style ];
}



/* Set the general pointer to its arch-dependant function */
void SetFontFunctions( void )
{
    if ( Support30() )
        SetFontStyles = SetFontStylesOS3;
    else
        SetFontStyles = SetFontStylesOS2;

#ifdef HAVE_HIRES
    if ( IsHiResTypePalm( HiResType() ) )
        SetFontStyles = SetFontStylesPalm;
#endif
#ifdef HAVE_SONY_SDK
    if ( IsHiResTypeSony( HiResType() ) )
        SetFontStyles = SetFontStylesSony;
#endif
#ifdef HAVE_HANDERA_SDK
    if ( IsHiResTypeHandera( HiResType() ) )
        SetFontStyles = SetFontStylesHandera;
#endif

    SetFontStyles();
}



/* Set selected font style */
void SetFontStylesOS2( void )
{
    FontModeType prefsFont;
    StyleType*   list;

    prefsFont = Prefs()->fontModeMain;
    for ( list = ListOfFontStylesOS2; list->fontMode != EOL; list++ ) {
        if ( list->fontMode == prefsFont ) {
            StylesMain = list->style;
            break;
        }
    }
    prefsFont = Prefs()->fontModeLibrary;
    for ( list = ListOfFontStylesOS2; list->fontMode != EOL; list++ ) {
        if ( list->fontMode == prefsFont ) {
            StylesLibrary = list->style;
            break;
        }
    }
}



/* Set selected font style */
void SetFontStylesOS3( void )
{
    FontModeType prefsFont;
    StyleType*   list;

    prefsFont = Prefs()->fontModeMain;
    for ( list = ListOfFontStylesOS3; list->fontMode != EOL; list++ ) {
        if ( list->fontMode == prefsFont ) {
            StylesMain = list->style;
            break;
        }
    }
    prefsFont = Prefs()->fontModeLibrary;
    for ( list = ListOfFontStylesOS3; list->fontMode != EOL; list++ ) {
        if ( list->fontMode == prefsFont ) {
            StylesLibrary = list->style;
            break;
        }
    }
}



#ifdef HAVE_HIRES
/* Set selected font style */
void SetFontStylesPalm( void )
{
    FontModeType prefsFont;
    StyleType*   list;

    prefsFont = Prefs()->fontModeMain;
    for ( list = ListOfFontStylesPalm; list->fontMode != EOL; list++ ) {
        if ( list->fontMode == prefsFont ) {
            StylesMain = list->style;
            break;
        }
    }
    prefsFont = Prefs()->fontModeLibrary;
    for ( list = ListOfFontStylesPalm; list->fontMode != EOL; list++ ) {
        if ( list->fontMode == prefsFont ) {
            StylesLibrary = list->style;
            break;
        }
    }
}
#endif



#if defined( HAVE_HIRES ) && defined( HAVE_SONY_SDK )
/* Set selected font style */
void SetFontStylesSony( void )
{
    FontModeType prefsFont;
    StyleType*   list;

    prefsFont = Prefs()->fontModeMain;
    for ( list = ListOfFontStylesSony; list->fontMode != EOL; list++ ) {
        if ( list->fontMode == prefsFont ) {
            StylesMain = list->style;
            break;
        }
    }
    prefsFont = Prefs()->fontModeLibrary;
    for ( list = ListOfFontStylesSony; list->fontMode != EOL; list++ ) {
        if ( list->fontMode == prefsFont ) {
            StylesLibrary = list->style;
            break;
        }
    }
}
#endif



#if defined( HAVE_HIRES ) && defined( HAVE_HANDERA_SDK )
/* Set selected font style */
void SetFontStylesHandera( void )
{
    FontModeType prefsFont;
    StyleType*   list;

    /* Convert font list into VGA-specific versions of font */
    HanderaConvertFontList( os3StandardStyles, handeraHiResStandardStyles );
    HanderaConvertFontList( os3LargeStyles, handeraHiResLargeStyles );

    prefsFont = Prefs()->fontModeMain;
    for ( list = ListOfFontStylesHandera; list->fontMode != EOL; list++ ) {
        if ( list->fontMode == prefsFont ) {
            StylesMain = list->style;
            break;
        }
    }
    prefsFont = Prefs()->fontModeLibrary;
    for ( list = ListOfFontStylesHandera; list->fontMode != EOL; list++ ) {
        if ( list->fontMode == prefsFont ) {
            StylesLibrary = list->style;
            break;
        }
    }
}
#endif



/* Change the current font to an italic style if available.  Return true
   if successful. */
Boolean SetFontItalic( void )
{
    FontID  font;
    font = FntGetFont();
    if ( FONT_IS_ITALIC( font ) )
        return true;
    if ( FONT_IS_USER( font ) && haveItalic[ font - START_USER_FONTS ] ) {
        FntSetFont( font + ITALIC_FONT_DELTA );
        return true;
    }
    return false;
}



/* Change the current font back to upright (non-italic) style.  Return 
   true if we were in an italic style */
Boolean EndFontItalic( void )
{
    FontID  font;
    font = FntGetFont();
    if ( ! FONT_IS_ITALIC( font ) )
        return false;
    FntSetFont( font - ITALIC_FONT_DELTA );
    return true;
}



/* Display the example font character centred in the given object ID */
void DisplayChar
    (
    FontID      fontID, /* font to print */
    const char  letter, /* letter to display */
    FormType*   form,   /* pointer form to print to */
    UInt16      objID   /* object to print in form */
    )
{
    RectangleType bounds;
    Coord         x;
    Coord         y;
#ifdef HAVE_GRAY_FONT
    Char          orientation;
    orientation   = GrayFntSetOrientation( GRAY_FONT_NORMAL );
#endif

    FrmGetObjectBounds( form, FrmGetObjectIndex( form, objID ), &bounds );
    FntSetFont( fontID );

    /* double our input values for sony */
    HiResAdjustBounds( &bounds, sonyHiRes );

    /* find approximate centered position for character */
    x = bounds.topLeft.x +
        ( bounds.extent.x / 2 ) -
        ( FntCharWidth( letter ) / 2 );
    y = bounds.topLeft.y +
        ( bounds.extent.y / 2 ) -
        ( FntCharHeight() / 2 );

    WinDrawChars( &letter, 1, x, y );

#ifdef HAVE_GRAY_FONT
    GrayFntSetOrientation( orientation );
#endif
}



/* Replacement for FntLineHeight() that takes into account subscript and
   superscript text */
extern Int16 FontLineHeight( void )
{
    Int16 height;

    if ( currentStyle == SUPSTYLE ) {
        height = FntLineHeight() / 4 + GetPrevFontHeight();
    }
    else
        height = FntLineHeight();

    return height;
}



#ifdef SUPPORT_VFS_FONTS
/* Add a font in VFS */
void AddVFSFont
     (
     const Char* name,
     Char*       volumeLabel,
     Char*       path
     )
{
    if ( userFontDBList == NULL )
        return;

    if ( GetUserFontNumber( name, false, fontCacheOff ) == NO_SUCH_USER_FONT ) {
        DBEntryType* dbListEntry;

        dbListEntry                 = SafeMemPtrNew( sizeof( DBEntryType ) );
        dbListEntry->volumeLabel    = SafeMemPtrNew( StrLen( volumeLabel ) + 1 );
        dbListEntry->path           = SafeMemPtrNew( StrLen( path ) + 1 );
        dbListEntry->onVFS          = true;
        StrCopy( dbListEntry->name, name );
        StrCopy( dbListEntry->volumeLabel, volumeLabel );
        StrCopy( dbListEntry->path, path );

        ListAppend( userFontDBList, dbListEntry );
    }
}
#endif



/* Get a font resource in the current user font */
MemHandle GetFontResource
     (
     UInt32             resourceType,
     UInt32             resourceID,
     FontResourceKind*  resourceKindP
     )
{
#ifdef SUPPORT_VFS_FONTS
    if ( currentFontIsOnVFS ) {
        Err         err;
        MemHandle   handle;

        err = VFSFileDBGetResource( currentVFSFontRef, resourceType, resourceID,
                  &handle );
        if ( err == errNone ) {
            *resourceKindP = fontResourceVFS;
            return handle;
        }
    }
#endif
    *resourceKindP = fontResourceStorage;
    return DmGetResource( resourceType, resourceID );
}




/* Release a font resource */
void ReleaseFontResource
     (
     MemHandle        handle,
     FontResourceKind kind
     )
{
     if ( handle == NULL )
         return;
#ifdef SUPPORT_VFS_FONTS
     if ( kind == fontResourceVFS ) {
         MemHandleFree( handle );
     }
     else
#endif
     {
         DmReleaseResource( handle );
     }
     return;
}



static void ClearUserFontNameList( void )
{
    if ( userFontNames == NULL )
        return;

    SafeMemPtrFree( userFontNames );
    userFontNames = NULL;
}



static Int16 DBEntryCompare
    (
    void*  a,
    void*  b,
    Int32  other
    )
{
    return StrCompare( ( *( DBEntryType** )a )->name,
                       ( *( DBEntryType** )b )->name );
}



static void SortAndMakeUserFontNameList( void )
{
    UInt16        i;
    DBEntryType*  dbListEntry;
    DBEntryType** userFontArray;

    currentUserFontNumber = NO_SUCH_USER_FONT;
    ClearUserFontNameList();
    if ( userFontDBList == NULL )
        return;

    numberOfUserFonts = ListSize( userFontDBList );
    if ( numberOfUserFonts == 0 )
        return;

    userFontArray   = SafeMemPtrNew( numberOfUserFonts * sizeof( DBEntryType* ) );
    userFontNames   = SafeMemPtrNew( numberOfUserFonts * sizeof( Char* ) );
    dbListEntry     = ListFirst( userFontDBList );
    for ( i = 0; i < numberOfUserFonts ; i++ ) {
        userFontArray[ i ] = dbListEntry;
        dbListEntry        = ListNext( userFontDBList, dbListEntry );
    }
    SysQSort( userFontArray, numberOfUserFonts, sizeof( DBEntryType* ),
        DBEntryCompare, 0 );
    ListDelete( userFontDBList );
    userFontDBList = ListCreate();
    for ( i = 0 ; i < numberOfUserFonts ; i++ ) {
        ListAppend( userFontDBList, userFontArray[ i ] );
        userFontNames[ i ] = userFontArray[ i ]->name;
    }
    SafeMemPtrFree( userFontArray );
}




/* Initialize internal lists of user font prcs */
void InitializeUserFontDBs( void )
{
    DmSearchStateType   stateInfo;
    UInt16              cardNo;
    LocalID             dbID;
    Err                 err;
    DBEntryType*        dbListEntry;

    userFontDBList = ListCreate();

    if ( userFontDBList == NULL )
        return;

    err = DmGetNextDatabaseByTypeCreator( true, &stateInfo,
              (UInt32) UserFontResourceType, (UInt32) ViewerAppID,
              false, &cardNo, &dbID );

    while ( err == errNone ) {
        dbListEntry = SafeMemPtrNew( sizeof( DBEntryType ) );
        err = DmDatabaseInfo( cardNo, dbID, dbListEntry->name, NULL, NULL,
                NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL );
        if ( err == errNone ) {
            dbListEntry->cardNo = cardNo;
            dbListEntry->dbID   = dbID;
            dbListEntry->onVFS  = false;
            ListAppend( userFontDBList, dbListEntry );
        }
        else {
            SafeMemPtrFree( dbListEntry );
        }
        err = DmGetNextDatabaseByTypeCreator( false, &stateInfo,
                (UInt32) UserFontResourceType, (UInt32) ViewerAppID, false,
                &cardNo, &dbID );
    }
    SortAndMakeUserFontNameList();
}



#ifdef SUPPORT_VFS_FONTS
Boolean ScannedVFSFonts( void )
{
    return scannedVFS;
}



void PostprocessVFSFonts( void )
{
    SortAndMakeUserFontNameList();
    scannedVFS = true;
}
#endif



/* Scan VFS for fonts */
void ScanVFSFonts( void )
{
#ifdef SUPPORT_VFS_FONTS
    if ( ! scannedVFS ) {
        ErrTry {
            EnumerateCards( ENUMERATECARD_FONTS );
            PostprocessVFSFonts();
        }
        ErrCatch( UNUSED_PARAM( err ) ) {
        } ErrEndCatch
    }
#endif
}



/* Go from name to position in user font list, or NO_SUCH_USER_FONT */
Int16 GetUserFontNumber
    (
    const Char* name,      /* name of user font to search for */
    Boolean     scanVFS,   /* do we scan VFS if we need to? */
    Int16       cache      /* which cache to add to */
    )
{
    Int16        i;
    DBEntryType* dbListEntry;
    
    if ( *name == '\0' )
        return NO_SUCH_USER_FONT;

    dbListEntry = ListFirst( userFontDBList );
    i = 0;
    while( dbListEntry != NULL ) {
        DBEntryType* nextDBListEntry;
        nextDBListEntry = ListNext( userFontDBList, dbListEntry );
        if ( ! StrCompare( dbListEntry->name, name ) ) {
#ifdef SUPPORT_VFS_FONTS
            if ( cache != fontCacheOff && dbListEntry->onVFS ) {
                VFSFontCacheAddEntry( dbListEntry->name,
                    dbListEntry->volumeLabel,
                    dbListEntry->path,
                    cache );
            }
#endif
            return i;
        }
        dbListEntry = nextDBListEntry;
        i++;
    }
#ifdef SUPPORT_VFS_FONTS
    if ( ! scannedVFS && scanVFS ) {
        if ( VFSFontCacheFindEntry( name ) ) {
            /* we found it and added it in at the end of the list */
            SortAndMakeUserFontNameList();
            return GetUserFontNumber( name, true, cache );
        }
        else {
            ScanVFSFonts();
            return GetUserFontNumber( name, false, cache );
        }
    }
#endif
    return NO_SUCH_USER_FONT;
}




/* Go from position in user font list to name */
Char* GetUserFontName
    (
    Int16 number    /* number of user font */
    )
{
    if ( numberOfUserFonts == 0 || number == NO_SUCH_USER_FONT )
        return "";
    else
        return userFontNames[ number ];
}



/* Load a particular user font prc */
/* Can take NO_SUCH_USER_FONT as an argument */
void LoadUserFont
    (
    Int16 number  /* number of user font to load */
    )
{
    DBEntryType*  dbListEntry;
    if ( number == currentUserFontNumber )
        return;
    if ( numberOfUserFonts == 0 || number == NO_SUCH_USER_FONT ) {
        currentUserFontNumber = NO_SUCH_USER_FONT;
        RefreshCustomFonts();
        return;
    }
    dbListEntry = ListGet( userFontDBList, number + 1 );
    if ( dbListEntry != NULL ) {
#ifdef SUPPORT_VFS_FONTS
        if ( dbListEntry->onVFS ) {
            Err err;
            err = VFSFileOpen( FindVolRefNum( dbListEntry->volumeLabel ),
                      dbListEntry->path, vfsModeRead, &currentVFSFontRef );
            if ( err == errNone ) {
                currentUserFontNumber = number;
                currentFontIsOnVFS    = true;
            }
            else {
                currentUserFontNumber  = NO_SUCH_USER_FONT;
                currentFontIsOnVFS     = false;
            }
        }
        else
#endif
        {
            currentUserFontDBRef   = DmOpenDatabase( dbListEntry->cardNo,
                                         dbListEntry->dbID, dmModeReadOnly );
            currentUserFontNumber  = number;
#ifdef SUPPORT_VFS_FONTS
            currentFontIsOnVFS     = false;
#endif
        }
    }
    else {
        currentUserFontNumber  = NO_SUCH_USER_FONT;
#ifdef SUPPORT_VFS_FONTS
        currentFontIsOnVFS     = false;
#endif
    }
    return;
}



/* Unload the current user font prc, if any */
void CloseUserFont( void )
{
  if ( currentUserFontNumber != NO_SUCH_USER_FONT ) {
#ifdef SUPPORT_VFS_FONTS
        if ( currentFontIsOnVFS ) {
            VFSFileClose( currentVFSFontRef );
            currentFontIsOnVFS = false;
        }
        else
#endif
        {
            DmCloseDatabase( currentUserFontDBRef );
        }
        currentUserFontNumber = NO_SUCH_USER_FONT;
  }
}



/* Check the current user font number */
Int16 GetCurrentUserFontNumber( void )
{
    return currentUserFontNumber;
}



/* Clear internal lists of user font prcs and close any user font */
void DeinitializeUserFontDBs( void )
{
    DBEntryType*  dbListEntry;
    DBEntryType*  nextDBListEntry;

    CloseUserFont();

    if ( NULL == userFontDBList )
        return;

    dbListEntry = ListFirst( userFontDBList );
    while ( dbListEntry != NULL ) {
        nextDBListEntry = ListNext( userFontDBList, dbListEntry );
        if ( dbListEntry->onVFS ) {
            SafeMemPtrFree( dbListEntry->volumeLabel );
            SafeMemPtrFree( dbListEntry->path );
        }
        SafeMemPtrFree( dbListEntry );
        dbListEntry = nextDBListEntry;
    }

    ListDelete( userFontDBList );
    userFontDBList = NULL;

    ClearUserFontNameList();
}




/* Initialize drop down list of user fonts */
void InitializeUserFontList
    (
    UInt16 listID    /* ID number of list for user font names */
    )
{
    ListType*   list;

    list = GetObjectPtr( listID );

    if ( 0 < numberOfUserFonts ) {
        LstSetListChoices( list, userFontNames, numberOfUserFonts );
        LstSetHeight( list, numberOfUserFonts );
    }
    else {
        LstSetListChoices( list, emptyUserFontNameList, 1 );
        LstSetHeight( list, 1 );
    }
}



/* How many user font prcs are there? */
Int16 GetNumberOfUserFonts( void )
{
    return numberOfUserFonts;
}



#ifdef SUPPORT_VFS_FONTS
/* The following functions support the VFS font name cache.
   This is a Palm database where each records consists of three
   null-terminated strings.  The first string is the font name.
   The next is the volume label.  The third is the path. */

/* Look for an entry in the VFS font cache.  If found, add it
   to the current font list. */
static Boolean VFSFontCacheFindEntry
     (
     const Char*   name
     )
{
    LocalID   cacheID;
    UInt16    cardNo;
    DmOpenRef ref;
    UInt16    numRecords;
    UInt16    i;

    cardNo     = 0;
    cacheID    = DmFindDatabase( cardNo, vfsFontCacheName );
    if ( cacheID == 0 )
        return false;
    ref        = DmOpenDatabase( cardNo, cacheID, dmModeReadOnly );
    numRecords = DmNumRecords( ref );

    for ( i = 0 ; i < numRecords ; i++ ) {
         MemHandle record;
         Char*     data;
         record = DmGetRecord( ref, i );

         if ( record != NULL ) {
             data = MemHandleLock( record );
             if ( 3 < MemHandleSize( record ) && ! StrCompare( data, name ) ) {
                 Char*   volumeLabel;
                 Char*   path;
                 Err     err;
                 FileRef fontRef;
                 UInt32  type;
                 UInt32  creatorID;
                 Boolean found;

                 found       = false;
                 volumeLabel = data + StrLen( data ) + 1;
                 path        = volumeLabel + StrLen( volumeLabel ) + 1;
                 err         = VFSFileOpen( FindVolRefNum( volumeLabel ),
                                   path, vfsModeRead, &fontRef );

                 if ( err == errNone ) {
                     Char thisName[ dmDBNameLength + 1 ];
                     err = VFSFileDBInfo( fontRef, thisName, NULL, NULL, NULL,
                               NULL, NULL, NULL, NULL, NULL, &type,
                               &creatorID, NULL );
                     VFSFileClose( fontRef );
                     if ( err == errNone && type == UserFontResourceType &&
                          creatorID == ViewerAppID &&
                          ! StrCompare( thisName, name ) ) {
                         AddVFSFont( name, volumeLabel, path );
                         found = true;
                     }
                 }
                 MemHandleUnlock( record );
                 DmReleaseRecord( ref, i, false );
                 DmCloseDatabase( ref );
                 return found;
             }
             MemHandleUnlock( record );
             DmReleaseRecord( ref, i, false );
         }
    }
    DmCloseDatabase( ref );
    return false;
}



static void VFSFontCacheAddEntry
     (
     const Char* name,
     const Char* volumeLabel,
     const Char* path,
     Int16       cache
     )
{
    LocalID   cacheID;
    UInt16    cardNo;
    DmOpenRef ref;
    UInt16    nameLen;
    UInt16    volumeLabelLen;
    UInt16    pathLen;
    UInt16    dataLen;
    MemHandle record;
    Char*     recordData;

    if ( cache == fontCacheOff )
        return;

    cardNo     = 0;
    cacheID    = DmFindDatabase( cardNo, vfsFontCacheName );
    if ( cacheID == 0 ) {
        UInt16 i;
        Err    err;
        err    = DmCreateDatabase( cardNo, vfsFontCacheName, ViewerAppID,
                     PlkrVFSFontCacheType, false );
        if ( err != errNone )
            return;
        cacheID = DmFindDatabase( cardNo, vfsFontCacheName );
        if ( cacheID == 0 )
            return;
        ref = DmOpenDatabase( cardNo, cacheID, dmModeReadWrite );
        for ( i = 0 ; i < numFontCaches ; i++ ) {
            UInt16  index;
            index = i;
            DmNewRecord( ref, &index, 1 );
            DmReleaseRecord( ref, index, false );
        }
    }
    else {
        ref = DmOpenDatabase( cardNo, cacheID, dmModeReadWrite );
    }

    nameLen        = StrLen( name );
    volumeLabelLen = StrLen( volumeLabel );
    pathLen        = StrLen( path );
    dataLen        = nameLen + volumeLabelLen + pathLen + 3;

    record = DmResizeRecord( ref, cache, dataLen );

    if ( record != NULL ) {
        recordData = MemHandleLock( record );
        DmWrite( recordData, 0, name, nameLen + 1 );
        DmWrite( recordData, nameLen + 1, volumeLabel,
            volumeLabelLen + 1 );
        DmWrite( recordData, nameLen + volumeLabelLen + 2, path,
            pathLen + 1 );
        MemHandleUnlock( record );
    }
    DmReleaseRecord( ref, cache, false );
    DmCloseDatabase( ref );
}
#endif
