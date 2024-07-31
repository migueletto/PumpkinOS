/*
 * $Id: resize.c,v 1.24 2004/05/02 14:50:35 prussar Exp $
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

#include "util.h"
#include "DIA.h"
#ifdef SUPPORT_DIA_HANDERA
#include <Vga.h>
#endif

#define DebugInfo(s) /* { char ss[500];StrPrintF(ss,"%s(%d) ",s,activeFormID);\
        ClipboardAppendItem(clipboardText,ss,StrLen(ss)); } */

static DIAFormEntryType* formList = NULL;
static DIAStateType*     bins;
static UInt16            numBins;
static UInt16            formCount = 0;
static UInt16            activeFormID;
static FormType*         activeFormPtr;
static UInt16            activeIndex;
static UInt16            activeFlags;
static DIAFormEntryType* activeEntry;
static Boolean           haveWinDisplayChangedEvent = false;



static UInt16 StandardExtent( void )
{
#ifdef SUPPORT_DIA_HANDERA
    if ( GetDIAHardware() == DIA_HARDWARE_HANDERA )
        return 240;
    else
#endif
        return 160;
}



/* Update form, but only once */
void UniqueUpdateForm( UInt16 formID, UInt16 code )
{
    EventType e;
    MemSet( &e, sizeof(EventType), 0 );
    e.eType                     = frmUpdateEvent;
    e.data.frmUpdate.formID     = formID;
    e.data.frmUpdate.updateCode = code;
    EvtAddUniqueEventToQueue( &e, 0, true );
    DebugInfo("Queuing frmUpdateForm");
}



/* Same as FrmGetObjectIndex() but works with GSI_OBJECT_ID, too */
static UInt16 MyFrmGetObjectIndex( FormPtr formPtr, UInt16 id )
{
    UInt16 i;
    if ( id != GSI_OBJECT_ID )
        return FrmGetObjectIndex( formPtr, id );
    for ( i = FrmGetNumberOfObjects( formPtr ) - 1 ; 0 < i ; i -- ) {
        if ( FrmGetObjectType( formPtr, i ) == frmGraffitiStateObj ) {
            return i;
        }
    }
    return 0;
}



void WinGlueGetBounds( WinHandle winH, RectangleType* rect )
{
    Err     err;
    UInt32  version;
    err = FtrGet( sysFtrCreator, sysFtrNumROMVersion, &version );
    if ( err == errNone && 0x04003000 <= version ) {
        WinGetBounds( winH, rect );
    }
    else {
        WinHandle oldH;
        oldH = WinSetDrawWindow( winH );
        WinGetDrawWindowBounds( rect );
        WinSetDrawWindow( oldH );
    }
}



static void AddForm( UInt16 indexInList, MemHandle handle, UInt16 formID )
{
    UInt16         numObjects;
    DIAConfigType* config;

    formList[ indexInList ].configHandle = handle;
    if ( handle == NULL ) {
        return;
    }

    config          = MemHandleLock( handle );
    numObjects      = ( ( config->sizeInWords + 1 ) * sizeof( UInt16 ) -
                        sizeof( DIAConfigType ) ) /
                          sizeof( DIAConfigEntryType );

    formList[ indexInList ].formID         = formID;
    formList[ indexInList ].config         = config;
    formList[ indexInList ].numObjects     = numObjects;
    formList[ indexInList ].open           = false;

    if ( config->flags & DIA_FORM_USE_BIN &&
         numBins < 1 + config->bin )
        numBins = 1 + config->bin;
}



/* This sets up the original bounds array as needed */
static void InitialSetupForm( UInt16 indexInList )
{
    RectangleType* originalBounds;
    FormType*      formPtr;
    UInt16         numObjects;
    UInt16         i;

    numObjects = formList[ indexInList ].numObjects;
    if ( numObjects == 0 )
        return;
    if ( formList[ indexInList ].originalBounds != NULL )
        return;
    formPtr = FrmGetFormPtr( formList[ indexInList ].formID );
    if ( formPtr == NULL )
        return;

    originalBounds = SafeMemPtrNew( numObjects * sizeof( RectangleType ) );
    formList[ indexInList ].originalBounds = originalBounds;

    /* First object is the form itself */
    WinGlueGetBounds( FrmGetWindowHandle( formPtr ), &( originalBounds[ 0 ] ) );

    /* Now on to the other objects */
    for ( i = 1 ; i < numObjects ; i++ ) {
         UInt16 objectIndex;
         objectIndex = MyFrmGetObjectIndex( formPtr,
                     formList[ indexInList ].config->objectList[ i ].objectID );
         FrmGetObjectBounds( formPtr, objectIndex, &( originalBounds[ i ] ) );
    }
}



/* Adjust one coordinate */
static Coord Adjust
    (
    Coord value,            /* old value to adjust */
    Coord oldObjectExtent,  /* old extent if relevant */
    Coord newObjectExtent,  /* already adjusted extent if relevant */
    Coord oldExtent,        /* old form/screen extent */
    Coord newExtent,        /* new form/screen extent */
    UInt16 shiftedFlag      /* adjustment flag, shifted down as needed */
    )
{
    Coord delta;
    
    delta = newExtent - oldExtent;

    switch ( shiftedFlag ) {
        case DIA_X_FIX >> DIA_SHIFT_X:
            return value;
        case DIA_X_RATIO >> DIA_SHIFT_X:
            return value * ( Int32 )newExtent / oldExtent;
        case DIA_X_RIGHT >> DIA_SHIFT_X:
            return newExtent - ( oldExtent - value - oldObjectExtent ) - newObjectExtent;
        case DIA_X_PLUS_DW >> DIA_SHIFT_X:
            return value + delta;
        case DIA_X_PLUS_1_2_DW >> DIA_SHIFT_X:
            return value + delta / 2;
        case DIA_X_PLUS_1_3_DW >> DIA_SHIFT_X:
            return value + delta / 3;
        case DIA_X_PLUS_2_3_DW >> DIA_SHIFT_X:
            return value + 2 * delta / 3;
        case DIA_X_PLUS_1_4_DW >> DIA_SHIFT_X:
            return value + delta / 4;
        case DIA_X_PLUS_3_4_DW >> DIA_SHIFT_X:
            return value + 3 * delta / 4;
        default: /* should not happen */
            return value;
    }
}



/* Get new bounds for an object, adjusted for DIA */
static void GetNewBounds( UInt16 displayExtentX, UInt16 displayExtentY,
    UInt16 stdFormExtentX, UInt16 stdFormExtentY,
    RectangleType* oldBounds, RectangleType* newBounds,
    DIAConfigEntryType* configEntry )
{
    UInt16 flags;
    flags = configEntry->flags;

    newBounds->extent.x  = Adjust( oldBounds->extent.x, 0, 0, stdFormExtentX,
                               displayExtentX,
                               ( flags & DIA_MASK_W ) >> DIA_SHIFT_W );
    newBounds->extent.y  = Adjust( oldBounds->extent.y, 0, 0, stdFormExtentY,
                               displayExtentY,
                               ( flags & DIA_MASK_H ) >> DIA_SHIFT_H );
    newBounds->topLeft.x = Adjust( oldBounds->topLeft.x, oldBounds->extent.x,
                               newBounds->extent.x,
                               stdFormExtentX, displayExtentX,
                               ( flags & DIA_MASK_X ) >> DIA_SHIFT_X );
    newBounds->topLeft.y = Adjust( oldBounds->topLeft.y, oldBounds->extent.y,
                               newBounds->extent.y,
                               stdFormExtentY, displayExtentY,
                               ( flags & DIA_MASK_Y ) >> DIA_SHIFT_Y );
}



/* This compares two rectangles.  If sonyDoubleSecond is set and the first
   bounds are exactly double the size of the first and DIA hardware is
   DIA_HARDWARE_SONY1, then they also match. */
static Boolean SameBounds( RectangleType* a, RectangleType* b,
    Boolean sonyDoubleSecond )
{
    Boolean compare;
    compare = ( a->extent.y == b->extent.y && a->extent.x == b->extent.x &&
                a->topLeft.y == b->topLeft.y && a->topLeft.x == b->topLeft.x );
#ifdef SUPPORT_DIA_SONY
    if ( ! compare && GetDIAHardware() == DIA_HARDWARE_SONY1 ) {
        return a->extent.y == 2 * b->extent.y &&
               a->extent.x == 2 * b->extent.x &&
               a->topLeft.y == 2 * b->topLeft.y &&
               a->topLeft.x == 2 * b->topLeft.x;
    }
#endif
    return compare;
}



/* Returns true if there is moving done */
static Boolean MoveObjectsInForm( UInt16 indexInList )
{
    Coord     extentX;
    Coord     extentY;
    UInt16    i;
    UInt16    focus;
    FormType* formPtr;
    DIAFormEntryType* entry;

    RectangleType  newBounds;
    RectangleType  curBounds;
    
    entry = &( formList[ indexInList ] );

    formPtr = FrmGetFormPtr( entry->formID );
    if ( formPtr == NULL )
        return false;
    InitialSetupForm( indexInList );
    WinGetDisplayExtent( &extentX, &extentY );

    /* First object is the form itself */
    GetNewBounds( extentX, extentY, StandardExtent(), StandardExtent(),
         &( formList[ indexInList ].originalBounds[ 0 ] ),
         &newBounds, &( entry->config->objectList[ 0 ] ) );
    WinGlueGetBounds( FrmGetWindowHandle( formPtr ), &curBounds );

    if ( ( SameBounds( &curBounds, &newBounds, true ) ||
           ( formList[ indexInList ].open &&
           SameBounds( &newBounds, &( formList[ indexInList ].lastBounds ), false ) ) ) &&
           extentX == entry->lastExtentX && extentY == entry->lastExtentY ) {
        DebugInfo( "skipping resize" );
        return false;
    }

    entry->lastBounds  = newBounds;
    entry->lastExtentX = extentX;
    entry->lastExtentY = extentY;
    WinSetBounds( FrmGetWindowHandle(formPtr), &newBounds );

    /* The rest is resized relative to the form itself */
    extentX = newBounds.extent.x;
    extentY = newBounds.extent.y;
    focus   = FrmGetFocus( formPtr );

    for ( i = 1 ; i < entry->numObjects ; i++ ) {
         UInt16         objectIndex;
         objectIndex = MyFrmGetObjectIndex( formPtr,
                     entry->config->objectList[ i ].objectID );
         GetNewBounds( extentX, extentY,
             entry->originalBounds[ 0 ].extent.x,
             entry->originalBounds[ 0 ].extent.y,
             &( entry->originalBounds[ i ] ),
             &newBounds, &( entry->config->objectList[ i ] ) );
         FrmGetObjectBounds( formPtr, objectIndex, &curBounds );
         if ( ! SameBounds( &curBounds, &newBounds, false ) ) {
             FrmSetObjectBounds( formPtr, objectIndex, &newBounds );
             switch ( FrmGetObjectType( formPtr, objectIndex ) ) {
                 case frmFieldObj:
    /*             case frmScrollBarObj: */
                 {
                     FieldType* field;
                     UInt16     insPt;

                     field   = ( FieldType* ) FrmGetObjectPtr( formPtr, objectIndex );
                     insPt   = FldGetInsPtPosition( field );
                     FldRecalculateField( field, true );
                     FldSetInsPtPosition( field, insPt );
                     FldSendChangeNotification( field );
                     break;
                 }
                 default:
                     break;
             }
         }
    }

    if ( FrmGetFocus( formPtr ) != focus )
        FrmSetFocus( formPtr, focus );

    return true;
}



void InitializeResizeSupport( UInt16 formMapId )
{
    MemHandle     indexHandle;
    DIAIndexType* index;
    UInt16        i;

    if ( formList != NULL )
        return;
    indexHandle = DmGetResource( WORD_LIST_TYPE, formMapId );
    if ( indexHandle == NULL )
        return;

    if ( DIA_HARDWARE_NONE == InitializeDIA() )
        return;

    index     = MemHandleLock( indexHandle );
    numBins   = 0;
    formCount = index->count * sizeof( UInt16 ) /
                    sizeof( DIAIndexEntryType );

    if ( formCount == 0 ) {
        numBins = 0;
        return;
    }

    formList  = SafeMemPtrNew( formCount * sizeof( DIAFormEntryType ) );
    MemSet( formList, formCount * sizeof( DIAFormEntryType ), 0 );

    for ( i = 0 ; i < formCount ; i++ ) {
         AddForm( i, DmGetResource( WORD_LIST_TYPE, index->mapList[ i ].to ),
             index->mapList[ i ].from );
    }

    MemHandleUnlock( indexHandle );
    DmReleaseResource( indexHandle );

    if ( 0 < numBins ) {
        bins = SafeMemPtrNew( numBins * sizeof( DIAStateType ) );
        for ( i = 0 ; i < numBins ; i++ )
             bins[ i ] = DIA_STATE_UNDEFINED;
    }
}




void TerminateResizeSupport( void )
{
     UInt16  i;

     if ( formList == NULL )
         return;
     TerminateDIA();
     for ( i = 0 ; i < formCount ; i++ ) {
          if ( NULL != formList[ i ].configHandle ) {
              MemHandleUnlock( formList[ i ].configHandle );
              DmReleaseResource( formList[ i ].configHandle );
              if ( formList[ i ].originalBounds != NULL )
                  SafeMemPtrFree( formList[ i ].originalBounds );
          }
     }
     SafeMemPtrFree( formList );
     formCount = 0;
     if ( 0 < numBins ) {
         SafeMemPtrFree( bins );
     }
}




void LoadResizePrefs( UInt32 appID, UInt16 prefID )
{
    UInt16 size;

    size = 0;

    if ( formList == NULL || numBins == 0 ||
         noPreferenceFound ==
             PrefGetAppPreferences( appID, prefID, NULL, &size, true ) ) {
        return;
    }

    if ( numBins * sizeof( DIAStateType ) < size ) {
        size = numBins * sizeof( DIAStateType );
    }

    PrefGetAppPreferences( appID, prefID, bins, &size, true );
}



void SaveResizePrefs( UInt32 appID, UInt16 prefID, Int16 version )
{
    if ( formList == NULL || numBins == 0 )
        return;

    PrefSetAppPreferences( appID, prefID, version, bins,
        numBins * sizeof( DIAStateType ), true );
}



static UInt16 GetFormIndex( UInt16 formID )
{
    UInt16 i;

    if ( formList == NULL )
        return 0;

    for ( i = 0 ; i < formCount ; i++ )
        if ( formList[ i ].formID == formID )
            break;

    return i;
}



void SetResizePolicy( UInt16 formID )
{
    if ( GetFormIndex( formID ) < formCount )
        SetCustomDIAPolicy( formID );
}



static Boolean GetFormInfo( void )
{
    activeFormID  = FrmGetActiveFormID();
    activeFormPtr = FrmGetActiveForm();
    activeIndex   = GetFormIndex( activeFormID );

    if ( activeIndex < formCount ) {
        activeEntry = &( formList[ activeIndex ] );
        activeFlags = activeEntry->config->flags;
        return true;
    }
    else {
        return false;
    }
}





static Boolean PrepareForm( void )
{
    if ( activeIndex == formCount ) {
        SetDIAState( DIA_STATE_MAX );
        SetDIAAllowResize( false );
        return false;
    }

    return MoveObjectsInForm( activeIndex );
}



static void SaveDIAState( void )
{
    if ( activeIndex == formCount )
        return;

    if ( activeEntry->config->flags & DIA_FORM_USE_BIN ) {
        bins[ activeEntry->config->bin ] = GetDIAState();
    }
}




static Boolean FormStartup( void )
{
    DIAStateType newState;
    Boolean   big;
    Boolean   allowBig;
    Boolean   changed;

    if ( activeIndex == formCount ) {
        SetDIAState( DIA_STATE_MAX );
        SetDIAAllowResize( false );
        SetDIAConstraints( FrmGetWindowHandle( activeFormPtr ), false, false );
        return false;
    }

    newState = GetDIAState();

    if ( ( activeFlags & DIA_FORM_USE_BIN ) &&
         ( bins[ activeEntry->config->bin ] != DIA_STATE_UNDEFINED ) ) {
        newState = bins[ activeEntry->config->bin ];
/*        if ( newState == DIA_STATE_MIN ) DebugInfo("bin = min");
        if ( newState == DIA_STATE_MAX ) DebugInfo("bin = max"); */
    }
    else if ( ! ( activeFlags & DIA_FORM_KEEP_LAST ) ) {
        newState = activeEntry->config->preferredState;
    }

    if ( activeFlags & DIA_FORM_NO_RESIZE ) {
        SetDIAAllowResize( false );
        allowBig = false;
    }
    else {
        SetDIAAllowResize( true );
        allowBig = true;
    }

    big      = ( newState == DIA_STATE_MIN || newState == DIA_STATE_NO_STATUS_BAR );
    allowBig = allowBig || big;

    SetDIAConstraints( FrmGetWindowHandle( activeFormPtr ), allowBig, allowBig );

    SetDIAState( newState );

    changed = PrepareForm();

/*    if ( changed ) DebugInfo("Changed in FormStartup"); */

    SaveDIAState();

    return changed;
}


static void ForceRedrawAll( void )
{
    UInt16 i;
    for ( i = 0 ; i < formCount ; i++ ) {
        if ( formList[ i ].open )
            formList[ i ].forceRedraw = true;
    }
}



Boolean ResizeHandleFrmOpenEvent( void )
{
    GetFormInfo();
#ifdef SUPPORT_DIA_HANDERA
    if ( GetDIAHardware() == DIA_HARDWARE_HANDERA &&
         ( activeIndex == formCount || ! ( activeFlags & DIA_FORM_HANDERA ) )
       ) {
        VgaFormModify( activeFormPtr, vgaFormModify160To240 );
    }
#endif
    if ( activeIndex < formCount ) {
        MemSet( &( activeEntry->lastBounds ), sizeof( RectangleType ), 0 );
        activeEntry->lastExtentX = -1;
        activeEntry->lastExtentY = -1;
        activeEntry->forceRedraw = false;
    }
    DebugInfo("FrmOpen");
    PushCoordinateSystemToStandard();
    FormStartup();
    if ( activeIndex < formCount ) {
        activeEntry->open = true;
    }
    PopCoordinateSystem();
    haveWinDisplayChangedEvent = false;
    return true;
}



Boolean ResizeHandleFrmCloseEvent( void )
{
    if ( GetFormInfo() ) {
        DebugInfo("FrmClose");
        activeEntry->open        = false;
        activeEntry->forceRedraw = false;
        SaveDIAState();
    }
    return true;
}



Boolean ResizeHandleWinDisplayChangedEvent( void )
{
    haveWinDisplayChangedEvent = false;
    if ( ! GetFormInfo() )
        return true;

    PushCoordinateSystemToStandard();

    DebugInfo("WinDisplayChanged");
    if ( PrepareForm() || activeEntry->forceRedraw ) {
        UniqueUpdateForm( activeFormID, frmRedrawUpdateCode );
        activeEntry->forceRedraw = false;
    }
    SaveDIAState();

    PopCoordinateSystem();

    return true;
}



Boolean ResizeHandleWinExitEvent( void )
{
    DebugInfo("WinExit");

    return true;
}



Boolean ResizeHandleFrmRedrawUpdateCode( void )
{
    DebugInfo("FrmUpdate");
    GetFormInfo();
    if ( activeIndex < formCount )
        activeEntry->forceRedraw = false;
    return true;
}



Boolean ResizeHandleWinEnterEvent( void )
{
    GetFormInfo();
    DebugInfo("WinEnter");
    if ( activeIndex == formCount ) {
        return true;
    }

    PushCoordinateSystemToStandard();

    /* If the last form we have record of being in is this one, then
       we may have missed what happened on a form. */

    if ( activeEntry->open && ( FormStartup() ||
         activeEntry->forceRedraw ) )
       {
        EventType e;
        DebugInfo("WinEnter: Possibly missed winDisplayChangedEvent");
        MemSet( &e, sizeof( EventType ), 0 );
        e.eType = winDisplayChangedEvent;
        EvtAddUniqueEventToQueue( &e, 0, true );
        DebugInfo("Queuing display change");
        activeEntry->forceRedraw = true;
    }

    PopCoordinateSystem();

    return true;
}




void SetHaveWinDisplayChangedEvent( Boolean value )
{
    DebugInfo("SetHaveWinDisplayChangedEvent()");
    haveWinDisplayChangedEvent = value;
    ForceRedrawAll();
}




void ResizeRefreshCurrentForm( void )
{
    GetFormInfo();
    if ( activeIndex == formCount )
        return;

    PushCoordinateSystemToStandard();

    FormStartup();
    SaveDIAState();
    UniqueUpdateForm( activeFormID, frmRedrawUpdateCode );

    PopCoordinateSystem();
}




Boolean ResizeHandleEvent( EventType* event )
{
    Boolean handled = false;

    switch ( event->eType ) {
        case frmOpenEvent:
            ResizeHandleFrmOpenEvent();
            break;
        case frmCloseEvent:
            ResizeHandleFrmCloseEvent();
            break;
        case winEnterEvent:
            ResizeHandleWinEnterEvent();
            break;
        case winExitEvent:
            ResizeHandleWinExitEvent();
            break;
        case winDisplayChangedEvent:
            ResizeHandleWinDisplayChangedEvent();
            break;
        case frmUpdateEvent:
            if ( event->data.frmUpdate.updateCode == frmRedrawUpdateCode ) {
                ResizeHandleFrmRedrawUpdateCode();
            }
            break;
        default:
            break;
    }

    return handled;
}

