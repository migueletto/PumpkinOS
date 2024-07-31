/*
 * $Id: viewer.h,v 1.53 2004/04/18 15:34:48 prussar Exp $
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

#ifndef PLUCKER_VIEWER_H
#define PLUCKER_VIEWER_H

#include <config.h>

#include <PalmOS.h>
#include "pluckererror.h"

#ifdef HAVE_FIVEWAY_SDK
#include <PalmChars.h>
#endif

#ifdef HAVE_HANDSPRING_SDK
#include <68K/Hs.h>
#endif

#ifdef HAVE_SONY_SDK
#include <SonyCLIE.h>
#endif

#define UNUSED_PARAM_ID(id) id
#define UNUSED_PARAM_ATTR   __attribute__ ((__unused__))

#define BOOKMARKFORM_SECTION
#define CATEGORYFORM_SECTION
#define DETAILSFORM_SECTION
#define EMAILFORM_SECTION
#define FONTFORM_SECTION
#define FULLSCREENFORM_SECTION
#define HARDCOPYFORM_SECTION
#define LIBRARYFORM_SECTION
#define MAINFORM_SECTION
#define RENAMEDOCFORM_SECTION
#define RESULTFORM_SECTION
#define SEARCHFORM_SECTION
#define TIMEOUT_SECTION

#define CACHE_SECTION
#define CONTROL_SECTION
#define HISTORY_SECTION
#define IMAGE_SECTION
#define LINK_SECTION
#define LIST_SECTION
#define METADOCUMENT_SECTION
#define PREFSDATA_SECTION
#define SEARCH_SECTION
#define SEARCH8_SECTION
#define SESSION_SECTION
#define TABLE_SECTION

#define AXXPAC_SECTION
#define ANCHOR_SECTION
#define BOOKMARK_SECTION
#define DOCLIST_SECTION
#define GENERICFILE_SECTION
#define OS_SECTION
#define RAMFILE_SECTION
#define UTIL_SECTION
#define VFSFILE_SECTION
#define ROTATE_SECTION

#define FIVEWAY_SECTION
#define HIRES_SECTION
#define JOGDIAL_SECTION
#define PREFSFORM_SECTION
#define SCREEN_SECTION
#define HANDERA_SECTION
#define KEYBOARD_SECTION
#define KEYBOARDFORM_SECTION

#define DOCUMENT_SECTION
#define PARAGRAPH_SECTION
#define GRAYFONT_SECTION
#define UNCOMPRESS_SECTION
#define SKINS_SECTION
#define XLIT_SECTION
#define EXTERNALFORM_SECTION
#define RESIZE_SECTION
#define DIA_SECTION

#ifdef HAVE_PALMCUNIT
#define VIEWER_SECTION
#define UNIT_TEST_SECTION
#define MOCK_SECTION
#else
#define VIEWER_SECTION
#endif

#if 0
#if defined __GNUC__

#define UNUSED_PARAM_ID(id) id
#define UNUSED_PARAM_ATTR   __attribute__ ((__unused__))

#define BOOKMARKFORM_SECTION    __attribute__ ((section( "sec1" )))
#define CATEGORYFORM_SECTION    __attribute__ ((section( "sec1" )))
#define DETAILSFORM_SECTION     __attribute__ ((section( "sec1" )))
#define EMAILFORM_SECTION       __attribute__ ((section( "sec1" )))
#define FONTFORM_SECTION        __attribute__ ((section( "sec1" )))
#define FULLSCREENFORM_SECTION  __attribute__ ((section( "sec1" )))
#define HARDCOPYFORM_SECTION    __attribute__ ((section( "sec1" )))
#define LIBRARYFORM_SECTION     __attribute__ ((section( "sec1" )))
#define MAINFORM_SECTION        __attribute__ ((section( "sec1" )))
#define RENAMEDOCFORM_SECTION   __attribute__ ((section( "sec1" )))
#define RESULTFORM_SECTION      __attribute__ ((section( "sec1" )))
#define SEARCHFORM_SECTION      __attribute__ ((section( "sec1" )))
#define TIMEOUT_SECTION         __attribute__ ((section( "sec1" )))

#define CACHE_SECTION           __attribute__ ((section( "sec2" )))
#define CONTROL_SECTION         __attribute__ ((section( "sec2" )))
#define HISTORY_SECTION         __attribute__ ((section( "sec2" )))
#define IMAGE_SECTION           __attribute__ ((section( "sec2" )))
#define LINK_SECTION            __attribute__ ((section( "sec2" )))
#define LIST_SECTION            __attribute__ ((section( "sec2" )))
#define METADOCUMENT_SECTION    __attribute__ ((section( "sec2" )))
#define PREFSDATA_SECTION       __attribute__ ((section( "sec2" )))
#define SEARCH_SECTION          __attribute__ ((section( "sec2" )))
#define SEARCH8_SECTION         __attribute__ ((section( "sec2" )))
#define SESSION_SECTION         __attribute__ ((section( "sec2" )))
#define TABLE_SECTION           __attribute__ ((section( "sec2" )))

#define AXXPAC_SECTION          __attribute__ ((section( "sec3" )))
#define ANCHOR_SECTION          __attribute__ ((section( "sec3" )))
#define BOOKMARK_SECTION        __attribute__ ((section( "sec3" )))
#define DOCLIST_SECTION         __attribute__ ((section( "sec3" )))
#define GENERICFILE_SECTION     __attribute__ ((section( "sec3" )))
#define OS_SECTION              __attribute__ ((section( "sec3" )))
#define RAMFILE_SECTION         __attribute__ ((section( "sec3" )))
#define UTIL_SECTION            __attribute__ ((section( "sec3" )))
#define VFSFILE_SECTION         __attribute__ ((section( "sec3" )))
#define ROTATE_SECTION          __attribute__ ((section( "sec3" )))

#define FIVEWAY_SECTION         __attribute__ ((section( "sec4" )))
#define HIRES_SECTION           __attribute__ ((section( "sec4" )))
#define JOGDIAL_SECTION         __attribute__ ((section( "sec4" )))
#define PREFSFORM_SECTION       __attribute__ ((section( "sec4" )))
#define SCREEN_SECTION          __attribute__ ((section( "sec4" )))
#define HANDERA_SECTION      __attribute__ ((section( "sec4" )))
#define KEYBOARD_SECTION        __attribute__ ((section( "sec4" )))
#define KEYBOARDFORM_SECTION    __attribute__ ((section( "sec4" )))

#define DOCUMENT_SECTION        __attribute__ ((section( "sec5" )))
#define PARAGRAPH_SECTION       __attribute__ ((section( "sec5" )))
#define GRAYFONT_SECTION        __attribute__ ((section( "sec5" )))
#define UNCOMPRESS_SECTION      __attribute__ ((section( "sec5" )))
#define SKINS_SECTION           __attribute__ ((section( "sec5" )))
#define XLIT_SECTION            __attribute__ ((section( "sec5" )))
#define EXTERNALFORM_SECTION    __attribute__ ((section( "sec5" )))
#define RESIZE_SECTION          __attribute__ ((section( "sec5" )))
#define DIA_SECTION             __attribute__ ((section( "sec5" )))

#ifdef HAVE_PALMCUNIT
#define VIEWER_SECTION          __attribute__ ((section( "unit" )))
#define UNIT_TEST_SECTION       __attribute__ ((section( "unit" )))
#define MOCK_SECTION            __attribute__ ((section( "mock" )))
#else
#define VIEWER_SECTION
#endif

#elif defined __MWERKS__

#define UNUSED_PARAM_ID(id)
#define UNUSED_PARAM_ATTR

#define BOOKMARKFORM_SECTION
#define BOOKMARK_SECTION
#define CATEGORYFORM_SECTION
#define DETAILSFORM_SECTION
#define EMAILFORM_SECTION
#define EXTERNALFORM_SECTION
#define FONTFORM_SECTION
#define FULLSCREENFORM_SECTION
#define LIBRARYFORM_SECTION
#define MAINFORM_SECTION
#define PREFSFORM_SECTION
#define RENAMEDOCFORM_SECTION
#define RESULTFORM_SECTION
#define SEARCHFORM_SECTION

#define CACHE_SECTION
#define CONTROL_SECTION
#define HIRES_SECTION
#define HISTORY_SECTION
#define IMAGE_SECTION
#define JOGDIAL_SECTION
#define LINK_SECTION
#define LIST_SECTION
#define METADOCUMENT_SECTION
#define PREFSDATA_SECTION
#define SCREEN_SECTION
#define SEARCH_SECTION
#define SESSION_SECTION
#define HANDERA_SECTION
#define TABLE_SECTION
#define TIMEOUT_SECTION
#define UNCOMPRESS_SECTION

#define ANCHOR_SECTION
#define AXXPAC_SECTION
#define DOCLIST_SECTION
#define DOCUMENT_SECTION
#define GENERICFILE_SECTION
#define OS_SECTION
#define PARAGRAPH_SECTION
#define RAMFILE_SECTION
#define UTIL_SECTION
#define VFSFILE_SECTION
#define GRAYFONT_SECTION
#define VIEWER_SECTION
#define UNIT_TEST_SECTION

#define RESIZE_SECTION
#define DIA_SECTION

#else

#define UNUSED_PARAM_ID(id)  id
#define UNUSED_PARAM_ATTR

#define BOOKMARKFORM_SECTION
#define BOOKMARK_SECTION
#define CATEGORYFORM_SECTION
#define DETAILSFORM_SECTION
#define EMAILFORM_SECTION
#define EXTERNALFORM_SECTION
#define FONTFORM_SECTION
#define FULLSCREENFORM_SECTION
#define HARDCOPYFORM_SECTION
#define LIBRARYFORM_SECTION
#define MAINFORM_SECTION
#define PREFSFORM_SECTION
#define RENAMEDOCFORM_SECTION
#define RESULTFORM_SECTION
#define SEARCHFORM_SECTION

#define CACHE_SECTION
#define CONTROL_SECTION
#define HIRES_SECTION
#define HISTORY_SECTION
#define IMAGE_SECTION
#define JOGDIAL_SECTION
#define LINK_SECTION
#define LIST_SECTION
#define METADOCUMENT_SECTION
#define PREFSDATA_SECTION
#define SCREEN_SECTION
#define SEARCH_SECTION
#define SESSION_SECTION
#define HANDERA_SECTION
#define TABLE_SECTION
#define TIMEOUT_SECTION
#define UNCOMPRESS_SECTION

#define ANCHOR_SECTION
#define AXXPAC_SECTION
#define DOCLIST_SECTION
#define DOCUMENT_SECTION
#define GENERICFILE_SECTION
#define OS_SECTION
#define PARAGRAPH_SECTION
#define RAMFILE_SECTION
#define UTIL_SECTION
#define VFSFILE_SECTION
#define GRAYFONT_SECTION
#define VIEWER_SECTION
#define UNIT_TEST_SECTION

#define RESIZE_SECTION
#define DIA_SECTION

#endif
#endif

#define UNUSED_PARAM(id)  UNUSED_PARAM_ID(id) UNUSED_PARAM_ATTR


#ifdef PLUCKER_GLOBALS_HERE

#define PLKR_GLOBAL

#else

#define PLKR_GLOBAL extern

#endif


/* Y-position measurement type */
typedef Int32 YOffset;

#endif

