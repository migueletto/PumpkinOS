/*
 * $Id: DIA.c,v 1.11 2004/05/15 06:09:44 nordstrom Exp $
 *
 * Viewer - a part of Plucker, the free off-line HTML viewer for PalmOS
 * Copyright (c) 1998-2004, Mark Ian Lillywhite and Michael Nordstrom
 * and Alexander R. Pruss.
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

#include "resourceids.h"
#include "resize.h"

#include "DIA.h"

#ifdef SUPPORT_DIA_HANDERA
#include <Silk.h>
#endif

#ifdef SUPPORT_DIA_SONY
#include <SonyCLIE.h>

#ifndef vskResizeVertically
#  define vskResizeVertically     1
#endif
#ifndef vskResizeHorizontally
#  define vskResizeHorizontally   2
#endif
#endif

#define COORDSYS_STACK_LEN 10


static DIAHardwareType  hardware = DIA_HARDWARE_NONE;
static UInt32           displayChangeNotification = 0;
static Boolean          haveNotification;
static UInt16           card;
static LocalID          db;
static Boolean          palmHiRes;
static UInt16           coordSysStack[ 10 ];
static UInt16           coordSysStackPtr = 0;
#ifdef SUPPORT_DIA_SONY
static UInt16           sonyRefNum;
#endif



#ifdef SUPPORT_DIA_HANDERA
static DIAHardwareType InitializeHandera( void )
{
    UInt32 version;
    if ( _TRGSilkFeaturePresent( &version ) ) {
        return DIA_HARDWARE_HANDERA;
    }
    else {
        return DIA_HARDWARE_NONE;
    }
}
#else
# define InitializeHandera() DIA_HARDWARE_NONE
#endif



#ifdef SUPPORT_DIA_SONY
static DIAHardwareType InitializeSony( void )
{
    Err    err;
    UInt32 version;

    err = SysLibFind( sonySysLibNameSilk, &sonyRefNum );
    if ( err == sysErrLibNotFound ) {
        err = SysLibLoad( 'libr', sonySysFileCSilkLib, &sonyRefNum );
    }
    if ( err != errNone )
        return DIA_HARDWARE_NONE;
    if ( errNone == FtrGet( sonySysFtrCreator, sonySysFtrNumVskVersion, 
                        &version ) ) {
        /* Version 2 and up */
        err = VskOpen( sonyRefNum );
        if ( errNone == err )
            return DIA_HARDWARE_SONY2;
    }
    else {
        /* Version 1 and up */
        err = SilkLibOpen( sonyRefNum );
        if ( errNone == err ) {
            /* Make sure we are in a Hi-Res mode */
            UInt32 width;
            UInt32 height;
            UInt16 sonyHiResRefNum;

            err = SysLibFind( sonySysLibNameHR, &sonyHiResRefNum );
            if ( err == sysErrLibNotFound )
                err = SysLibLoad( 'libr', sonySysFileCHRLib, &sonyHiResRefNum );
            if ( err == errNone ) {
                err = HROpen( sonyHiResRefNum );
                HRWinScreenMode( sonyHiResRefNum, winScreenModeGet, &width,
                     &height, NULL, NULL );
                HRClose( sonyHiResRefNum );
                if ( width < 320 )
                    return DIA_HARDWARE_NONE;
            }
            else
                return DIA_HARDWARE_NONE;
            }
        return DIA_HARDWARE_SONY1;
    }
    return DIA_HARDWARE_NONE;
}
#else
# define InitializeSony() DIA_HARDWARE_NONE
#endif



#ifdef HAVE_PALM_DIA_SDK
static DIAHardwareType InitializePalm( void )
{
    UInt32 version;
    Err    err;
    err = FtrGet( pinCreator, pinFtrAPIVersion, &version );
    if ( err != errNone )
        return DIA_HARDWARE_NONE;
    if ( pinAPIVersion1_1 <= version )
        return DIA_HARDWARE_PALM11;
    else if ( pinAPIVersion1_0 <= version )
        return DIA_HARDWARE_PALM10;
    else
        return DIA_HARDWARE_NONE;
}
#else
# define InitializePalm() DIA_HARDWARE_NONE
#endif



/* Check if this is an unknown device with non-standard screen size
   so we can make proper use of the screen size. */
static DIAHardwareType InitializeUnknown( void )
{
    Coord extentX;
    Coord extentY;
    PushCoordinateSystemToStandard();
    WinGetDisplayExtent( &extentX, &extentY );
    PopCoordinateSystem();
    if ( extentX != STD_EXTENT_X || extentY != STD_EXTENT_Y )
        return DIA_HARDWARE_UNKNOWN;
    else
        return DIA_HARDWARE_NONE;
}



static void RegisterNotification( void )
{
    Err      err;

    haveNotification = false;
    switch ( hardware ) {
#ifdef SUPPORT_DIA_SONY
        case DIA_HARDWARE_SONY1:
        case DIA_HARDWARE_SONY2:
            displayChangeNotification = sysNotifyDisplayChangeEvent;
            break;
#endif
#ifdef SUPPORT_DIA_HANDERA
        case DIA_HARDWARE_HANDERA:
            displayChangeNotification = trgNotifySilkEvent;
            break;
#endif
#ifdef HAVE_PALM_DIA_SDK
        case DIA_HARDWARE_PALM10:
        case DIA_HARDWARE_PALM11:
            displayChangeNotification = sysNotifyDisplayResizedEvent;
            break;
#endif
        default:
            return;
    }
    err = SysCurAppDatabase( &card, &db );
    if ( err != errNone )
        return;
    err = SysNotifyRegister( card, db, displayChangeNotification, NULL,
        sysNotifyNormalPriority, 0 );
    haveNotification = ( err == errNone );
}



static void UnregisterNotification( void )
{
    if ( haveNotification ) {
        SysNotifyUnregister( card, db, displayChangeNotification,
            sysNotifyNormalPriority );
        haveNotification = false;
    }
}



DIAHardwareType InitializeDIA( void )
{
    Err    err;
    UInt32 version;

    err       = FtrGet( sysFtrCreator, sysFtrNumWinVersion, &version );

    palmHiRes = ( err == errNone && 4 <= version );

    hardware = InitializeHandera();
    if ( hardware == DIA_HARDWARE_NONE )
        hardware = InitializePalm();
    if ( hardware == DIA_HARDWARE_NONE )
        hardware = InitializeSony();
    if ( hardware == DIA_HARDWARE_NONE )
        hardware = InitializeUnknown();
    RegisterNotification();

    return hardware;
}



void TerminateDIA( void )
{
    UnregisterNotification();
    if ( GetDIAState() == DIA_STATE_NO_STATUS_BAR )
        SetDIAState( DIA_STATE_MIN );
    switch ( hardware ) {
#ifdef SUPPORT_DIA_SONY
        case DIA_HARDWARE_SONY1:
            SilkLibClose( sonyRefNum );
            break;
        case DIA_HARDWARE_SONY2:
            VskClose( sonyRefNum );
            break;
#endif
        default:
            break;
    }
    hardware = DIA_HARDWARE_NONE;
}




void SetDIAState( DIAStateType state )
{
    if ( DIA_HARDWARE_HANDERA == hardware && GetDIAState() == state )
        return;
    switch ( hardware ) {
#ifdef SUPPORT_DIA_SONY
        case DIA_HARDWARE_SONY1:
            switch ( state ) {
                case DIA_STATE_MAX:
                    SilkLibResizeDispWin( sonyRefNum, silkResizeNormal );
                    break;
                case DIA_STATE_MIN:
                    SilkLibResizeDispWin( sonyRefNum, silkResizeToStatus );
                    break;
                case DIA_STATE_NO_STATUS_BAR:
                    SilkLibResizeDispWin( sonyRefNum, silkResizeMax );
                    break;
                default:
                    break;
            }
            break;
        case DIA_HARDWARE_SONY2:
            switch ( state ) {
                case DIA_STATE_MAX:
                    VskSetState( sonyRefNum, vskStateResize, vskResizeMax );
                    break;
                case DIA_STATE_MIN:
                    VskSetState( sonyRefNum, vskStateResize, vskResizeMin );
                    break;
                case DIA_STATE_NO_STATUS_BAR:
                    VskSetState( sonyRefNum, vskStateResize, vskResizeNone );
                    break;
                default:
                    break;
            }
            break;
#endif
#ifdef SUPPORT_DIA_HANDERA
        case DIA_HARDWARE_HANDERA:
            switch ( state ) {
                case DIA_STATE_MAX:
                    SilkMaximizeWindow();
                    break;
                case DIA_STATE_MIN:
                case DIA_STATE_NO_STATUS_BAR:
                    SilkMinimizeWindow();
                    break;
                default:
                    break;
            }
            break;
#endif
#ifdef HAVE_PALM_DIA_SDK
        case DIA_HARDWARE_PALM10:
        case DIA_HARDWARE_PALM11:
            switch ( state ) {
                case DIA_STATE_MAX:
                    PINSetInputAreaState( pinInputAreaOpen );
                    break;
                case DIA_STATE_MIN:
                case DIA_STATE_NO_STATUS_BAR:
                    PINSetInputAreaState( pinInputAreaClosed );
                    break;
                default:
                    break;
            }
            break;
#endif
        default:
            break;
    }
}



DIAStateType GetDIAState( void )
{
    switch ( hardware ) {
#ifdef SUPPORT_DIA_SONY
        case DIA_HARDWARE_SONY1: {
            Coord extentY;
            PushCoordinateSystemToStandard();
            WinGetDisplayExtent( NULL, &extentY );
            PopCoordinateSystem();
            if ( 240 <= extentY )
                return DIA_STATE_NO_STATUS_BAR;
            else if ( 225 <= extentY )
                return DIA_STATE_MIN;
            else
                return DIA_STATE_MAX;
            break;
        }
        case DIA_HARDWARE_SONY2: {
            UInt16 state;
            Err    err;
            err = VskGetState( sonyRefNum, vskStateResize, &state );
            if ( err != errNone )
                return DIA_STATE_UNDEFINED;
            switch ( state ) {
                case vskResizeMax:
                    return DIA_STATE_MAX;
                case vskResizeMin:
                    return DIA_STATE_MIN;
                case vskResizeNone:
                    return DIA_STATE_NO_STATUS_BAR;
                default:
                    return DIA_STATE_UNDEFINED;
            }
        }
#endif
#ifdef SUPPORT_DIA_HANDERA
        case DIA_HARDWARE_HANDERA:
            if ( SilkWindowMaximized() )
                return DIA_STATE_MAX;
            else
                return DIA_STATE_MIN;
#endif
#ifdef HAVE_PALM_DIA_SDK
        case DIA_HARDWARE_PALM10:
        case DIA_HARDWARE_PALM11:
            switch ( PINGetInputAreaState() ) {
                case pinInputAreaOpen:
                    return DIA_STATE_MAX;
                case pinInputAreaClosed:
                case pinInputAreaNone:
                    return DIA_STATE_MIN;
                default:
                    return DIA_STATE_UNDEFINED;
            }
#endif            
        default:
            return DIA_STATE_MAX;
    }
}



void SetDIAAllowResize( Boolean allow )
{
    switch ( hardware ) {
#ifdef SUPPORT_DIA_SONY
        case DIA_HARDWARE_SONY2:
            if ( allow ) {
                /* If available, enable horizontal resize */
                if ( 0x03 <= VskGetAPIVersion( sonyRefNum ) )
                    VskSetState( sonyRefNum, vskStateEnable,
                        vskResizeHorizontally );
                /* Enable vertical resize */
                VskSetState( sonyRefNum, vskStateEnable,
                    vskResizeVertically );
            }
            else {
                VskSetState( sonyRefNum, vskStateEnable, 0 );
            }
            break;
        case DIA_HARDWARE_SONY1:
            if ( allow )
                SilkLibEnableResize( sonyRefNum );
            else
                SilkLibDisableResize( sonyRefNum );
            break;
#endif
#ifdef HAVE_PALM_DIA_SDK
        case DIA_HARDWARE_PALM11:
        case DIA_HARDWARE_PALM10:
            PINSetInputTriggerState( allow ? pinInputTriggerEnabled :
                                                 pinInputTriggerDisabled );
            SysSetOrientationTriggerState( allow ?
                sysOrientationTriggerEnabled : sysOrientationTriggerDisabled );
            break;
#endif
        /* Note: On Handera, resizing is always enabled */
        default:
            break;
    }
}




static Boolean MatchLastExtents( void )
{
    static Coord lastX;
    static Coord lastY;
    Coord        extentX;
    Coord        extentY;

    extentX = lastX;
    extentY = lastY;

    PushCoordinateSystemToStandard();

    WinGetDisplayExtent( &lastX, &lastY );

    PopCoordinateSystem();

    return extentX == lastX && extentY == lastY;
}




Boolean HandleResizeNotification( UInt32 notificationType )
{
    EventType e;

    if ( notificationType != displayChangeNotification )
        return false;
    if ( MatchLastExtents() )
        return false;

    SetHaveWinDisplayChangedEvent( true );
    if ( hardware != DIA_HARDWARE_PALM11 ) {
        MemSet( &e, sizeof(EventType), 0 );
        e.eType = winDisplayChangedEvent;
        EvtAddUniqueEventToQueue( &e, 0, true );
    }
    return true;
}



void SetCustomDIAPolicy( UInt16 formID )
{
#ifdef HAVE_PALM_DIA_SDK
    if ( hardware == DIA_HARDWARE_PALM10 || hardware == DIA_HARDWARE_PALM11 ) {
        FormType* formPtr;
        formPtr = FrmGetFormPtr( formID );
        if ( formPtr != NULL ) {
            FrmSetDIAPolicyAttr( formPtr, frmDIAPolicyCustom );
        }
    }
#endif
}



DIAHardwareType GetDIAHardware( void )
{
    return hardware;
}



void SetDIAConstraints( WinHandle winH, Boolean big, Boolean allowBig )
{
#ifdef HAVE_PALM_DIA_SDK
    if ( hardware == DIA_HARDWARE_PALM10 || hardware == DIA_HARDWARE_PALM11 ) {
        PushCoordinateSystemToStandard();

        WinSetConstraintsSize( winH,
            STD_EXTENT_Y,
            big ? MAX_EXTENT_Y : STD_EXTENT_Y,
            allowBig ? MAX_EXTENT_Y : STD_EXTENT_Y,
            STD_EXTENT_X,
            big ? MAX_EXTENT_X : STD_EXTENT_X,
            allowBig ? MAX_EXTENT_X : STD_EXTENT_X );

        PopCoordinateSystem();
    }
#endif
}



/* Check which DIA state covers more screen space */
Int16 CompareDIAState( DIAStateType x, DIAStateType y )
{
    if ( y == DIA_STATE_UNDEFINED )
        y = DIA_STATE_MAX;
    if ( x == DIA_STATE_UNDEFINED )
        x = DIA_STATE_MAX;
    if ( x == y )
        return 0;
    switch ( x ) {
        case DIA_STATE_MIN:
            return y == DIA_STATE_NO_STATUS_BAR ? 1 : -1;
        case DIA_STATE_NO_STATUS_BAR:
            return -1;
        case DIA_STATE_MAX:
        default:
            return 1;
    }
}



static UInt16 SafeWinSetCoordinateSystem( UInt16 coordSys )
{
    if ( ! palmHiRes || NULL == WinGetDrawWindow() )
        return kCoordinatesStandard;
    else
        return WinSetCoordinateSystem( coordSys );
}



void PushCoordinateSystemToStandard( void )
{
    coordSysStack[ coordSysStackPtr++ ] =
        SafeWinSetCoordinateSystem( kCoordinatesStandard );
}




void PopCoordinateSystem( void )
{
    SafeWinSetCoordinateSystem( coordSysStack[ --coordSysStackPtr ] );
}



void GetHiddenStatusBarArea( RectangleType* area )
{
    if ( DIA_STATE_NO_STATUS_BAR != GetDIAState() ) {
        MemSet( area, sizeof( RectangleType ), 0 );
    }
    else {
        Coord extentX;
        Coord extentY;

        WinGetDisplayExtent( &extentX, &extentY );
        switch ( extentX ) {
            case 160:  /* 160 x 240 */
                area->topLeft.x = 0;
                area->topLeft.y = 225;
                area->extent.x  = 160;
                area->extent.y  = 240 - 225;
                break;
            case 240: /* 240 x 160 */
                area->topLeft.x = 225;
                area->topLeft.y = 0;
                area->extent.x  = 240 - 225;
                area->extent.y  = 160;
                break;
            case 320: /* 320 x 480 */
                area->topLeft.x = 0;
                area->topLeft.y = 450;
                area->extent.x  = 320;
                area->extent.y  = 480 - 450;
                break;
            case 480: /* 480 x 320 */
                area->topLeft.x = 450;
                area->topLeft.y = 0;
                area->extent.x  = 480 - 450;
                area->extent.y  = 320;
                break;
            default:
                MemSet( area, sizeof( RectangleType ), 0 );
                break;
        }
    }
}

