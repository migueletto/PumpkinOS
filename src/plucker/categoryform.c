/*
 * $Id: categoryform.c,v 1.35 2004/05/08 09:04:53 nordstrom Exp $
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

#include "libraryform.h"
#include "debug.h"
#include "prefsdata.h"
#include "resourceids.h"
#include "util.h"
#include "doclist.h"
#include "DIA.h"

#include "categoryform.h"


/***********************************************************************
 *
 *      Internal Constants
 *
 ***********************************************************************/
#define CATEGORY_DELETE         0
#define CATEGORY_RENAME         1
#define SELECT_OK               0
#define UNFILED                 0
#define NO_CATEGORY_SELECTED    0
#define ON                      true
#define OFF                     false


/***********************************************************************
 *
 *      Local functions
 *
 ***********************************************************************/
static Boolean HandleCategoryName(const EventType* event) CATEGORYFORM_SECTION;
static Boolean HandleCategoryPopUp(const EventType* event) CATEGORYFORM_SECTION;
static Boolean HandleControlEvent(EventType* event) CATEGORYFORM_SECTION;
static Boolean IsCategorySelected(UInt8 category) CATEGORYFORM_SECTION;
static Boolean IsUniqueCategoryName(UInt8 category) CATEGORYFORM_SECTION;
static Boolean SetNewCategoryName(Char* name) CATEGORYFORM_SECTION;
static void AddCategory(UInt8 category) CATEGORYFORM_SECTION;
static void CancelSelectedSettings(void) CATEGORYFORM_SECTION;
static void CategoryFormInit(void) CATEGORYFORM_SECTION;
static void CategoryNameFormInit(void) CATEGORYFORM_SECTION;
static void DisplayDocumentName(Char* name) CATEGORYFORM_SECTION;
static void DisplayDocumentSettings(void) CATEGORYFORM_SECTION;
static void DisplayFilterSettings(void) CATEGORYFORM_SECTION;
static void HideAndOrControls(void) CATEGORYFORM_SECTION;
static void HideCategoryName(UInt8 category) CATEGORYFORM_SECTION;
static void HideCategoryPopupIcon(UInt8 category) CATEGORYFORM_SECTION;
static void HideMultipleSelect(void) CATEGORYFORM_SECTION;
static void HighlightAllCategoryNames(Boolean enable) CATEGORYFORM_SECTION;
static void HighlightCategoryName(UInt8 category,
                Boolean enable) CATEGORYFORM_SECTION;
static void HighlightUnfiledCategoryName(Boolean enable) CATEGORYFORM_SECTION;
static void HighlightUserAssignedCategoryNames(
                Boolean enable) CATEGORYFORM_SECTION;
static void MergeListCategories(UInt8 dst, UInt8 src) CATEGORYFORM_SECTION;
static void RemoveCategory(UInt8 category) CATEGORYFORM_SECTION;
static void RemoveListCategoryName(UInt8 index) CATEGORYFORM_SECTION;
static void ResetCategories(void) CATEGORYFORM_SECTION;
static void SaveCategories( UInt16 newValue) CATEGORYFORM_SECTION;
static void SetAllCategories(void) CATEGORYFORM_SECTION;
static void SetCategories(UInt8 category) CATEGORYFORM_SECTION;
static void SetCategoryName(UInt8 category, Char* name) CATEGORYFORM_SECTION;
static void SetMultipleSelect(void) CATEGORYFORM_SECTION;
static void ShowAndOrControls(void) CATEGORYFORM_SECTION;
static void ShowCategoryName(UInt8 category) CATEGORYFORM_SECTION;
static void ShowCategoryPopupIcon(UInt8 category) CATEGORYFORM_SECTION;
static void ShowMultipleSelect(void) CATEGORYFORM_SECTION;
static void ToggleCategory(UInt8 category) CATEGORYFORM_SECTION;
static void UpdateCategorySettings(void) CATEGORYFORM_SECTION;


/***********************************************************************
 *
 *      Private variables
 *
 ***********************************************************************/
static UInt8        selectedCategory;
static UInt16       categories;
static UInt16       origCategories;
static Boolean      hasSavedPrefsData;
static Boolean      origMultipleSelect;
static Boolean      newCategory;
static FieldType*   fldPtr;
static FormType*    categoryForm;


static FilterType   origFilterMode = FILTER_OR;



/* Merge categories */
static void MergeListCategories
    (
    UInt8   dst,    /* destination category */
    UInt8   src     /* source category - this category will be removed */
    )
{
    UInt16 i;
    UInt16 category;
    UInt16 numberOfDatabases;

    category            = 1 << src;
    numberOfDatabases   = GetNumOfDocuments();
    for ( i = 0; i < numberOfDatabases; i++ ) {
        if ( ( DocInfo( i )->categories & category ) != 0 ) {
            DocInfo( i )->categories &= ~category;
            if ( dst != 0 )
                DocInfo( i )->categories |= ( 1 << dst );
            else if ( DocInfo( i )->categories == NO_CATEGORY_SELECTED )
                DocInfo( i )->categories = 1 << UNFILED;
        }
    }
}



/* Remove category */
static void RemoveListCategoryName
    (
    UInt8 index /* category index */
    )
{
    MergeListCategories( 0, index );
}



/* Initialize the add category form */
static void CategoryNameFormInit( void )
{
    Char        name[ dmCategoryLength ];
    FormType*   nameForm;

    nameForm = FrmGetFormPtr( frmNewCategory );

    GetCategoryName( selectedCategory, name );
    fldPtr = GetObjectPtr( frmNewCatName );
    FldSetSelection( fldPtr, 0, 0 );
    FldInsert( fldPtr, name, StrLen( name ) );
    FldSetSelection( fldPtr, 0, StrLen( name ) );
    FldSetDirty( fldPtr, false );

    FrmDrawForm( nameForm );
    FrmSetFocus( nameForm, FrmGetObjectIndex( nameForm, frmNewCatName ) );
}



static Boolean IsUniqueCategoryName(UInt8 category)
{
    return ( category == dmAllCategories );
}



static Boolean SetNewCategoryName(Char* name)
{
    UInt8 category;

    category = StoreCategoryName( selectedCategory, name );
    if ( ! IsUniqueCategoryName(category) ) {
        if ( newCategory ) {
            FrmCustomAlert( errCategoryExists, name, NULL, NULL );
            FldSetSelection( fldPtr, 0, StrLen( name ) );
            return false;
        }
        else {
            if ( FrmCustomAlert(confirmMergeCategory, name, NULL, NULL ) ==
                 SELECT_OK ) {
                MergeCategories( category, selectedCategory );
                MergeListCategories( category, selectedCategory );
            }
        }
    }
    else if ( newCategory ) {
        if ( IsSelectingCategoryFilter() && ! Prefs()->multipleSelect )
            ResetCategories();
        AddCategory(selectedCategory);
    }
    return true;
}



/* Event handler for the category name form */
Boolean CategoryNameFormHandleEvent
    (
    EventType* event  /* pointer to an EventType structure */
    )
{
    Boolean handled;

    switch ( event->eType ) {
        case ctlSelectEvent:
            if ( event->data.ctlEnter.controlID == frmNewCatOK ) {
                if ( FldDirty( fldPtr ) ) {
                    Char* name = FldGetTextPtr( fldPtr );
                    if ( *name != '\0' ) {
                        handled = SetNewCategoryName(name);
                        if ( ! handled )
                            break;
                    }
                }
            }
            else if ( event->data.ctlEnter.controlID != frmNewCatCancel ) {
                handled = false;
                break;
            }
            FrmReturnToForm( frmCategory );
            if ( event->data.ctlEnter.controlID != frmNewCatCancel )
                FrmUpdateForm( frmCategory, frmUpdateList );

            handled = true;
            break;

        case winEnterEvent:
            handled = ResizeHandleWinEnterEvent();
            break;

        case winDisplayChangedEvent:
            handled = ResizeHandleWinDisplayChangedEvent();
            break;

        case winExitEvent:
            handled = ResizeHandleWinExitEvent();
            break;

        case frmOpenEvent:
#ifdef HAVE_SILKSCREEN
            ResizeHandleFrmOpenEvent();
#endif
            CategoryNameFormInit();
            handled = true;
            break;

        case frmCloseEvent:
#ifdef HAVE_SILKSCREEN
            ResizeHandleFrmCloseEvent();
#endif
            handled = false;
            break;

        default:
            handled = false;
            break;
    }

    return handled;
}



static void DisplayDocumentName(Char* name)
{
    WinDrawChars( name, StrLen( name ),
        ( MaxExtentX() - FntCharsWidth( name, StrLen( name ) ) ) / 2,
        3 * MaxExtentY() / 4 );
}



static void ShowAndOrControls(void)
{
    FrmShowObject( categoryForm, FrmGetObjectIndex( categoryForm, frmCatOR ) );
    FrmShowObject( categoryForm, FrmGetObjectIndex( categoryForm, frmCatAND ) );
}


static void HideAndOrControls(void)
{
    FrmHideObject( categoryForm, FrmGetObjectIndex( categoryForm, frmCatOR ) );
    FrmHideObject( categoryForm, FrmGetObjectIndex( categoryForm, frmCatAND ) );
}



static void ShowMultipleSelect(void)
{
    FrmShowObject( categoryForm, FrmGetObjectIndex( categoryForm,
                                    frmCatMultiple ) );
}



static void HideMultipleSelect(void)
{
    FrmHideObject( categoryForm, FrmGetObjectIndex( categoryForm,
                                    frmCatMultiple ) );
}



static void DisplayFilterSettings(void)
{
    CtlSetValue( GetObjectPtr( frmCatMultiple ), Prefs()->multipleSelect );
    CtlSetValue( GetObjectPtr( frmCatOR ), Prefs()->filterMode == FILTER_OR );
    CtlSetValue( GetObjectPtr( frmCatAND ), Prefs()->filterMode == FILTER_AND );

    ShowMultipleSelect();
    if ( Prefs()->multipleSelect )
        ShowAndOrControls();
    else
        HideAndOrControls();
}



static void DisplayDocumentSettings(void)
{
    HideMultipleSelect();
    HideAndOrControls();
}



static Boolean IsCategorySelected(UInt8 category)
{
    return ((categories & (1 << category)) != 0);
}



static void ShowCategoryPopupIcon(UInt8 category)
{
    FrmShowObject( categoryForm, FrmGetObjectIndex( categoryForm,
                                    frmCatOpen1 + category ) );
}


static void HideCategoryPopupIcon(UInt8 category)
{
    FrmHideObject( categoryForm, FrmGetObjectIndex( categoryForm,
                                    frmCatOpen1 + category ) );
}


static void ShowCategoryName(UInt8 category)
{
    FrmShowObject( categoryForm, FrmGetObjectIndex( categoryForm,
                                    frmCat1 + category ) );
}



static void HideCategoryName(UInt8 category)
{
    FrmHideObject( categoryForm, FrmGetObjectIndex( categoryForm,
                                    frmCat1 + category ) );
}



static void SetCategoryName(UInt8 category, Char* name)
{
    CtlSetLabel( GetObjectPtr( frmCat1 + category ), name);
}



static void UpdateCategorySettings(void)
{
    UInt8 category;

    if (IsCategorySelected(UNFILED))
        HighlightUnfiledCategoryName(ON);
    else
        HighlightUnfiledCategoryName(OFF);

    for (category = 1; category <= dmRecNumCategories - 1; category++) {
        static Char categoryName[dmRecNumCategories][dmCategoryLength];

        HideCategoryName(category);
        HighlightCategoryName(category, OFF);
        GetCategoryName(category, categoryName[category]);
        SetCategoryName(category, categoryName[category]);
        if (CategoryExists(category)) {
            ShowCategoryPopupIcon(category);
            if (IsCategorySelected(category))
                HighlightCategoryName(category, ON);
        }
        else {
            HideCategoryPopupIcon(category);
        }
        ShowCategoryName(category);
    }
}



static void CategoryFormInit( void )
{
    static Char docName[ dmDBNameLength ];

    categoryForm = FrmGetFormPtr( frmCategory );
    FrmDrawForm( categoryForm );

    if ( ! hasSavedPrefsData ) {
        if ( IsSelectingCategoryFilter() ) {
            categories = Prefs()->categories;
            DisplayFilterSettings();
        }
        else {
            categories = DocInfo(ReturnLastIndex())->categories;
            StrNCopy( docName, DocInfo(ReturnLastIndex())->name,
                dmDBNameLength );
            DisplayDocumentSettings();
        }
    }
    UpdateCategorySettings();
    SaveCategories( categories );
    if ( ! IsSelectingCategoryFilter() )
        DisplayDocumentName( docName );

    if ( ! hasSavedPrefsData ) {
        origMultipleSelect  = Prefs()->multipleSelect;
        origFilterMode      = Prefs()->filterMode;
        origCategories      = categories;
        hasSavedPrefsData   = true;
    }
}



static void SaveCategories( UInt16 newValue)
{
    if ( IsSelectingCategoryFilter() ) {
        Prefs()->categories = newValue;
    }
    else {
        StoreCategoriesInDocumentList(DocInfo(ReturnLastIndex())->name,
            newValue);
        DocInfo(ReturnLastIndex())->categories = newValue;
    }
}



static void CancelSelectedSettings(void)
{
    Prefs()->multipleSelect = origMultipleSelect;
    Prefs()->filterMode = origFilterMode;

    SaveCategories( origCategories );
}



static void HighlightCategoryName(UInt8 category, Boolean enable)
{
    CtlSetValue(GetObjectPtr(frmCat1 + category), enable);
}



static void HighlightUnfiledCategoryName(Boolean enable)
{
    HighlightCategoryName(UNFILED, enable);
}



static void HighlightUserAssignedCategoryNames(Boolean enable)
{
    UInt8 category;

    for ( category = 1; category <= dmRecNumCategories - 1; category++ )
        if ( CategoryExists( category ) )
            HighlightCategoryName(category, enable);
}



static void HighlightAllCategoryNames(Boolean enable)
{
    if ( IsSelectingCategoryFilter() )
        HighlightUnfiledCategoryName( enable );
    else
        HighlightUnfiledCategoryName( OFF );

    HighlightUserAssignedCategoryNames( enable );
}



static Boolean HandleCategoryPopUp(const EventType* event)
{
    ListType*   list;
    Int16       selection;

    selectedCategory = event->data.ctlEnter.controlID - frmCatOpen1;

    list = GetObjectPtr( frmCatList );
    LstSetPosition( list, event->screenX, event->screenY );

    selection = LstPopupList( list );
    if ( selection == noListSelection ) {
        return true;
    }
    else if ( selection == CATEGORY_DELETE ) {
        Char name[ dmCategoryLength ];

        GetCategoryName( selectedCategory, name );
        if ( FrmCustomAlert( confirmDelete, name, NULL, NULL ) == SELECT_OK ) {
            RemoveCategoryName( selectedCategory );
            RemoveListCategoryName( selectedCategory );
            RemoveCategory( selectedCategory );
            FrmEraseForm( FrmGetFormPtr( frmCategory ) );
            CategoryFormInit();
        }
    }
    else if ( selection == CATEGORY_RENAME ) {
        newCategory = false;
        FrmPopupForm( frmNewCategory );
    }
    return true;
}



static Boolean HandleCategoryName(const EventType* event)
{
    selectedCategory = event->data.ctlEnter.controlID - frmCat1;

    if ( ! IsSelectingCategoryFilter()) {
        if ( selectedCategory == UNFILED ) {
            SetCategories(UNFILED);
            HighlightUnfiledCategoryName(ON);
            HighlightUserAssignedCategoryNames(OFF);
        }
        else if ( IsCategorySelected(UNFILED) ) {
            ResetCategories();
            HighlightUnfiledCategoryName(OFF);
        }
    }

    if ( CategoryExists( selectedCategory ) ) {
        Boolean handled;

        if ( ! IsSelectingCategoryFilter() || Prefs()->multipleSelect ) {
            ToggleCategory(selectedCategory);
            handled = false;
        }
        else {
            SetCategories(selectedCategory);
            Prefs()->categories = categories;
            hasSavedPrefsData   = false;

            FrmGotoForm( GetValidForm( frmLibrary ) );
            FrmUpdateForm( GetValidForm( frmLibrary ), frmUpdateList );

            handled = true;
        }
        if (IsCategorySelected(selectedCategory))
            HighlightCategoryName(selectedCategory, ON);
        else
            HighlightCategoryName(selectedCategory, OFF);

        return handled;
    }
    else {
        HighlightCategoryName(selectedCategory, OFF);

        newCategory = true;
        FrmPopupForm( frmNewCategory );

        return true;
    }
}



static void SetCategories(UInt8 category)
{
    categories = 1 << category;
}



static void AddCategory(UInt8 category)
{
    categories |= 1 << category;
}



static void SetAllCategories(void)
{
    UInt8 category;

    if ( IsSelectingCategoryFilter() )
        SetCategories(UNFILED);
    else
        ResetCategories();

    for ( category = 1; category <= dmRecNumCategories - 1; category++ )
        if ( CategoryExists( category ) )
            AddCategory(category);
}


static void ToggleCategory(UInt8 category)
{
    categories ^= 1 << category;
    if ( categories == NO_CATEGORY_SELECTED )
        categories = 1 << category;
}



static void RemoveCategory(UInt8 category)
{
    categories &= ~( 1 << category );
    if ( categories == NO_CATEGORY_SELECTED ) {
        categories = 1 << UNFILED;
        origCategories = categories;
    }
}



static void ResetCategories(void)
{
    categories = NO_CATEGORY_SELECTED;
}



static void SetMultipleSelect(void)
{
    Prefs()->multipleSelect = true;
    CtlSetValue( GetObjectPtr( frmCatMultiple ), Prefs()->multipleSelect );
}



static Boolean HandleControlEvent(EventType* event)
{
    Boolean handled;

    handled = false;
    switch ( event->data.ctlEnter.controlID ) {
        case frmCatOK:
            if ( categories == NO_CATEGORY_SELECTED ) {
                FrmAlert( errNoCategory );
            }
            else {
                SaveCategories( categories );
                hasSavedPrefsData = false;
                FrmGotoForm( GetValidForm( frmLibrary ) ) ;
                FrmUpdateForm( GetValidForm( frmLibrary ), frmUpdateList );
            }
            handled = true;
            break;

        case frmCatCancel:
            CancelSelectedSettings();
            hasSavedPrefsData = false;
            FrmGotoForm( GetValidForm( frmLibrary ) );
            handled = true;
            break;

        case frmCatAll:
            if ( IsSelectingCategoryFilter() ) {
                SetMultipleSelect();
                ShowAndOrControls();
            }
            SetAllCategories();
            HighlightAllCategoryNames(ON);
            handled = true;
            break;

        case frmCatNone:
            ResetCategories();
            HighlightAllCategoryNames(OFF);
            handled = true;
            break;

        case frmCatMultiple:
            Prefs()->multipleSelect = CtlGetValue( GetObjectPtr(
                                                    frmCatMultiple ) );
            if ( ! Prefs()->multipleSelect ) {
                SetCategories(UNFILED);
                HighlightUnfiledCategoryName(ON);
                HighlightUserAssignedCategoryNames(OFF);
                HideAndOrControls();
            }
            else {
                ShowAndOrControls();
            }
            break;

        case frmCatOR:
            Prefs()->filterMode = FILTER_OR;
            break;

        case frmCatAND:
            Prefs()->filterMode = FILTER_AND;
            break;

        case frmCat1:
        case frmCat2:
        case frmCat3:
        case frmCat4:
        case frmCat5:
        case frmCat6:
        case frmCat7:
        case frmCat8:
        case frmCat9:
        case frmCat10:
        case frmCat11:
        case frmCat12:
        case frmCat13:
        case frmCat14:
        case frmCat15:
        case frmCat16:
            handled = HandleCategoryName(event);
            break;

        case frmCatOpen1:
        case frmCatOpen2:
        case frmCatOpen3:
        case frmCatOpen4:
        case frmCatOpen5:
        case frmCatOpen6:
        case frmCatOpen7:
        case frmCatOpen8:
        case frmCatOpen9:
        case frmCatOpen10:
        case frmCatOpen11:
        case frmCatOpen12:
        case frmCatOpen13:
        case frmCatOpen14:
        case frmCatOpen15:
        case frmCatOpen16:
            handled = HandleCategoryPopUp(event);
            break;

        default:
            break;
    }
    return handled;
}



/* Event handler for the category form */
Boolean CategoryFormHandleEvent
    (
    EventType* event  /* pointer to an EventType structure */
    )
{
    Boolean handled;

    handled = false;
    switch ( event->eType ) {
        case ctlSelectEvent:
            handled = HandleControlEvent(event);
            break;

        case frmCloseEvent:
#ifdef HAVE_SILKSCREEN
            ResizeHandleFrmCloseEvent();
#endif
            handled = false;
            break;

        case winEnterEvent:
            handled = ResizeHandleWinEnterEvent();
            break;

        case winDisplayChangedEvent:
            handled = ResizeHandleWinDisplayChangedEvent();
            break;

        case winExitEvent:
            handled = ResizeHandleWinExitEvent();
            break;

        case frmOpenEvent:
#ifdef HAVE_SILKSCREEN
            ResizeHandleFrmOpenEvent();
#endif
            CategoryFormInit();
            handled = true;
            break;

        case frmUpdateEvent:
            if ( event->data.frmUpdate.updateCode == frmUpdateList ) {
                FrmEraseForm( FrmGetFormPtr(frmCategory) );
                CategoryFormInit();
                handled = true;
            }
            break;

        default:
            handled = false;
    }

    return handled;
}

