/*
 * $Id: screen.h,v 1.26 2003/11/14 17:26:57 prussar Exp $
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

#ifndef PLUCKER_SCREEN_H
#define PLUCKER_SCREEN_H

#include "hires.h"
#include "util.h"

#include "viewer.h"


/* Set screen bit depth for PalmOS3 */
extern void SetScreenModeOS3( void ) SCREEN_SECTION;

/* Set screen bit depth for PalmOS2 */
extern void SetScreenModeOS2( void ) SCREEN_SECTION;

/* Restore screen mode ( PalmOS 3.x ) */
extern void SetDefaultScreenModeOS3( Boolean save ) SCREEN_SECTION;

/* Restore screen mode ( PalmOS 2.x ) */
extern void SetDefaultScreenModeOS2( Boolean save ) SCREEN_SECTION;

/* Initialize a new block of ForeColor; OS 3.5 or greater will use the
   good APIs */
extern void SetForeColor_OS35( TextContext* tContext ) SCREEN_SECTION;

/* Initialize a new block of ForeColor; OS 3.0 to OS 3.4 will use the
   old APIs */
extern void SetForeColor_OS3( TextContext* tContext ) SCREEN_SECTION;

/* NOP for OS2 */
extern void SetForeColor_OS2( TextContext* tContext ) SCREEN_SECTION;

/* Save the device's colors before drawing, so can later restore 
   to original when done drawing record; OS 3.5 or greater will
   use the good APIs */
extern void SaveDrawState_OS35 (void) SCREEN_SECTION;

/* Save the device's colors before drawing, so can later restore 
   to original when done drawing record; OS 3.0 to OS 3.3 will
   use the old APIs */
extern void SaveDrawState_OS3 (void) SCREEN_SECTION;

/* NOP for OS2 */
extern void SaveDrawState_OS2( void ) SCREEN_SECTION;

/* Restore the device's colors after done drawing record; OS 3.5
   or greater will use the good APIs  */
extern void RestoreDrawState_OS35 (void) SCREEN_SECTION;

/* Restore the device's colors after done drawing record; OS 3.0
   to OS 3.3 will use the old APIs  */
extern void RestoreDrawState_OS3 (void) SCREEN_SECTION;

/* NOP for OS2 */
extern void RestoreDrawState_OS2 (void) SCREEN_SECTION;

/* Wrapper to make sure WinScreenLock is used only on OS3.5 or higher */
extern void ScreenLock_OS35( void ) SCREEN_SECTION;

/* Wrapper to make sure WinScreenUnLock is used only on OS3.5 or higher */
extern void ScreenUnlock_OS35( void ) SCREEN_SECTION;

/* NOP for pre-OS3.5 */
extern void ScreenLockUnlock_None( void ) SCREEN_SECTION;

/* Delete a more portable bitmap */
extern Err PortableBmpDelete ( BitmapType* bitmap ) SCREEN_SECTION;

/* Create a more portable bitmap */
extern BitmapType* PortableBmpCreate ( Coord bitmapX, Coord
    bitmapY, UInt8 depth, ColorTableType* colorTable, Err* err )
    SCREEN_SECTION;

/* Get the size of a bitmap */
UInt32 PortableBmpSize ( BitmapType* bitmap ) SCREEN_SECTION;


PLKR_GLOBAL void (*SetScreenMode)( void );
PLKR_GLOBAL void (*SetDefaultScreenMode)( Boolean );
PLKR_GLOBAL void (*SetForeColor)( TextContext* );
PLKR_GLOBAL void (*SaveDrawState)(void);
PLKR_GLOBAL void (*RestoreDrawState)(void);
PLKR_GLOBAL void (*ScreenLock)( void );
PLKR_GLOBAL void (*ScreenUnlock)( void );


#endif

