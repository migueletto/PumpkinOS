/*
 * $Id: font.h,v 1.16 2003/12/13 15:47:26 prussar Exp $
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

#ifndef PLUCKER_FONT_H
#define PLUCKER_FONT_H

#include "viewer.h"

/* font style*/
#define MINSTYLES       0
#define MAXSTYLES       12

#define DEFAULTSTYLE    0

#define BOLDSTYLE       7
#define FIXEDSTYLE      8
#define SUBSTYLE        10
#define SUPSTYLE        11

#define NO_SUCH_USER_FONT    -1

#define fontCacheOff    -1
#define fontCacheDoc     0
#define fontCacheLibrary 1

#define numFontCaches    2

typedef enum {
    STANDARD_FONTS = 0,
    PALM_FONTS,
    SONY_FONTS
} DeviceFontType;

typedef enum {
    fontResourceStorage = 0,
    fontResourceVFS
} FontResourceKind;

/* Load custom fonts into memory */
extern void LoadCustomFonts( DeviceFontType deviceFont );

/* Release custom fonts from memory */
extern void ReleaseCustomFonts( void );

/* Refresh the custom font list */
extern void RefreshCustomFonts( void );

/* Return the current font style */
extern Int16 GetCurrentStyle( void );

/* Return the previous font height */
extern Int16 GetPrevFontHeight( void );

/* Set the previous font height */
extern void SetPrevFontHeight( Int16 height );

/* Return main style font */
extern FontID GetMainStyleFont( const Int16 style );

/* Set & return main style font */
extern FontID SetMainStyleFont( const Int16 style );

/* Return library style font */
extern FontID GetLibraryStyleFont( const Int16 style );

/* Set the general pointer to its arch-dependant function */
extern void SetFontFunctions( void );

/* Set selected font style */
extern void SetFontStylesOS2( void );

/* Set selected font style */
extern void SetFontStylesOS3( void );

#ifdef HAVE_HIRES
/* Set selected font style */
extern void SetFontStylesPalm( void );
#endif

#ifdef HAVE_SONY_SDK
/* Set selected font style */
extern void SetFontStylesSony( void );
#endif

#ifdef HAVE_HANDERA_SDK
/* Set selected font style */
extern void SetFontStylesHandera( void );
#endif

/* Display the example font character centered in the given object ID */
extern void DisplayChar( FontID fontID, const char letter, FormType* form,
    UInt16 objID );

/* Replacement for FntLineHeight() that takes into account subscript and
 * superscript text */
extern Int16 FontLineHeight( void );

PLKR_GLOBAL void (*SetFontStyles)( void );

/* Change the current font to an italic style if available.  Return true
   if successful. */
extern Boolean SetFontItalic( void );

/* Return the current font to non-italic style.  Return true if
   we really were in an italic style */
extern Boolean EndFontItalic( void );

/* Initialize internal lists of user font prcs */
extern void InitializeUserFontDBs( void );

/* Clear internal lists of user font prcs */
extern void DeinitializeUserFontDBs( void );

/* Go from name to position in user font list, or NO_SUCH_USER_FONT */
extern Int16 GetUserFontNumber( const Char* name, Boolean scanVFS,
    Int16 cache );

/* Go from position in user font list to name */
extern Char* GetUserFontName( Int16 number );

/* Load a particular user font prc */
/* Can take NO_SUCH_USER_FONT as an argument */
extern void LoadUserFont( Int16 number );

/* Unload the current user font prc, if any */
extern void CloseUserFont( void );

/* Return the number of the current user font */
extern Int16 GetCurrentUserFontNumber( void );

/* Initialize drop down list of user fonts and close any user font */
extern void InitializeUserFontList( UInt16 listID );

/* How many user font prcs are there? */
extern Int16 GetNumberOfUserFonts( void );

/* Get the default style font height */
extern Int16 GetDefaultMainStyleHeight( void );

#ifdef SUPPORT_VFS_FONTS
void AddVFSFont( const Char* name, Char* volumeLabel, Char* path );
extern Boolean ScannedVFSFonts( void );
extern void ScanVFSFonts( void );
extern void PostprocessVFSFonts( void );
#endif



/* Get a font resource in the current user font */
extern MemHandle GetFontResource ( UInt32 resourceType, UInt32 resourceID,
     FontResourceKind* kindP );

/* Release a font resource */
extern void ReleaseFontResource ( MemHandle handle, FontResourceKind kind );

#endif
