/*
 * $Id: doclist.c,v 1.31 2004/03/06 19:37:48 nordstrom Exp $
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

#include "categoryform.h"
#include "const.h"
#include "debug.h"
#include "prefsdata.h"
#include "resourceids.h"
#include "util.h"
#include "os.h"
#include "ramfile.h"
#include "vfsfile.h"
#include "genericfile.h"
#include "libraryform.h"
#include "document.h"
#include "font.h"

#include "doclist.h"
#include "../libpit/debug.h"


/***********************************************************************
 *
 *      Internal Constants
 *
 ***********************************************************************/
#define UNFILED_CATEGORY    1
#define LABEL_LEN           255
#define PDB_SUFFIX          ".pdb"
#define PDB_SUFFIX_LEN      4
#define TRIGGER_EXTENT      60

static const Char PlkrDocListName[] = "PlkrDocList";


/***********************************************************************
 *
 *      Internal Types
 *
 ***********************************************************************/
typedef struct {
    UInt16  renamedCategories;
    Char    categoryLabels[ dmRecNumCategories ][ dmCategoryLength ];
    UInt8   categoryUniqIDs[ dmRecNumCategories ];
    UInt8   lastUniqID;
    UInt8   padding;
} PlkrAppInfoType;


/***********************************************************************
 *
 *      Local functions
 *
 ***********************************************************************/
static void InitPlkrAppInfo( DmOpenRef docRef ) DOCLIST_SECTION;;
static void CreateRemoveList( void ) DOCLIST_SECTION;;
static void DeactivateDocuments( void ) DOCLIST_SECTION;;
static void DeactivateOneDocument( UInt16 index ) DOCLIST_SECTION;;
static void ReleaseDeactivateList( void ) DOCLIST_SECTION;;
static void UpdateDocList( void ) DOCLIST_SECTION;;
static void CreateDocList( void ) DOCLIST_SECTION;;
static UInt8 GetCategoryIndex(Char *category) DOCLIST_SECTION;
static UInt8 AddCategoryToFreePosition(Char *name) DOCLIST_SECTION;


/***********************************************************************
 *
 *      Private variables
 *
 ***********************************************************************/
static DmOpenRef    plkrDocList;
static Boolean*     removeList;
static UInt16       removeListSize;
static DocumentInfo lastDocInfo;



/* Compare names ( ascending ) */
static Int16 CompareDocumentNames
    (
    void*               rec1,           /* Pointer to the record to sort */
    void*               rec2,           /* Pointer to the record to sort */
    Int16               other,          /* Any other custom information you want 
                                           passed to the comparison function */
    SortRecordInfoPtr   rec1SortInfo,   /* Pointer to SortRecordInfoType
                                           structure that specify unique
                                           sorting information 
                                           for the record */
    SortRecordInfoPtr   rec2SortInfo,   /* Pointer to SortRecordInfoType
                                           structure that specify unique
                                           sorting information 
                                           for the record */
    MemHandle           appInfoH        /* A handle to the document's
                                           application info block */
    )
{
    Int16 result;

    /*   0 if rec1.name = rec2.name */
    /* < 0 if rec1.name < rec2.name */
    /* > 0 if rec1.name > rec2.name */
    result = StrCompare( ( (DocumentInfo*) rec1 )->name,
        ( (DocumentInfo*) rec2 )->name );

    return result;
}



/* Store category info for document */
void StoreCategoriesInDocumentList
    (
    Char*  name,     /* name of document that should be updated */
    UInt16 newValue  /* category data */
    )
{
    MemHandle handle;

    ErrTry {
        handle = FindDocData( name, ALL_ELEMENTS, NULL );
        if ( handle != NULL ) {
            DocumentInfo* recordPtr;

            recordPtr = MemHandleLock( handle );
            DmWrite( recordPtr, OFFSETOF( DocumentData, categories ),
                &newValue, sizeof( UInt16 ) );
            MemHandleUnlock( handle );
        }
    }
    ErrCatch( UNUSED_PARAM( err ) ) {
    } ErrEndCatch
}



/* Store when the document is read */
void StoreReadTimeInDocumentList
    (
    Char*  name,     /* name of document that should be updated */
    UInt32 newValue  /* time from epoch */
    )
{
    MemHandle handle;

    ErrTry {
        handle = FindDocData( name, ALL_ELEMENTS, NULL );
        if ( handle != NULL ) {
            DocumentInfo* recordPtr;

            recordPtr = MemHandleLock( handle );
            DmWrite( recordPtr, OFFSETOF( DocumentData, timestamp ),
                &newValue, sizeof( UInt32 ) );
            MemHandleUnlock( handle );
        }
    }
    ErrCatch( UNUSED_PARAM( err ) ) {
    } ErrEndCatch
}



static void CreateRemoveList( void )
    /* THROWS */
{
    UInt16 i;

    /* create list for documents that have been deleted since the last
       session */
    removeListSize = DmNumRecords( plkrDocList );
    if ( removeListSize != 0 ) {
        removeList = SafeMemPtrNew( removeListSize * sizeof *removeList );
        for ( i = 0; i < removeListSize; i++)
            removeList[ i ] = true;
    }
}



static void DeactivateDocuments( void )
{
    UInt16 index;

    if ( removeList != NULL ) {
        index = removeListSize;
        while ( index-- ) {
            if ( removeList[ index ] )
                DeactivateOneDocument( index );
        }
        ReleaseDeactivateList();
    }
}



static void ReleaseDeactivateList( void )
{
    SafeMemPtrFree( removeList );
    removeList = NULL;
}



/* Populate document list with records */
static void UpdateDocList( void )
    /* THROWS */
{
    /* no automatic update if user has selected to handle it manually
       or if we have a low power situation */
    if ( Prefs()->syncPolicy == SYNC_MANUAL || ! HasEnoughPower() )
        return;
        
    /* create list for documents that have been deleted since the last
       session */
    CreateRemoveList();

    ErrTry {
        /* find all Plucker documents in RAM */
        EnumerateRAM();

        /* ... and on mounted external cards */
        if ( Prefs()->syncPolicy != SYNC_IGNORE_CARD ) {
#ifdef SUPPORT_VFS_FONTS
            if ( ! ScannedVFSFonts() ) {
                EnumerateCards( ENUMERATECARD_DOCS | ENUMERATECARD_FONTS );
                PostprocessVFSFonts();
            }
            else
#endif
            {
                EnumerateCards( ENUMERATECARD_DOCS );
            }
        }

        /* clean up */
        DeactivateDocuments();

        DmInsertionSort( plkrDocList, CompareDocumentNames, 0 );
    }
    ErrCatch( err ) {
        ReleaseDeactivateList();
        ErrThrow( err );
    } ErrEndCatch
}



/* Create an app info chunk if missing, return result from the database call */
static void InitPlkrAppInfo
    (
    DmOpenRef docRef    /* reference to document */
    )
    /* THROWS */
{
    UInt16              cardNo;
    MemHandle           handle;
    LocalID             dbID;
    LocalID             appInfoID;
    PlkrAppInfoType*    appInfoP;
    Err                 err;

    err = DmOpenDatabaseInfo( docRef, &dbID, NULL, NULL, &cardNo, NULL );
    THROW_IF( err != errNone, err );

    err = DmDatabaseInfo( cardNo, dbID, NULL, NULL, NULL, NULL, NULL,
            NULL, NULL, &appInfoID, NULL, NULL, NULL );
    THROW_IF( err != errNone, err );

    if ( appInfoID == 0 ) {
        handle = DmNewHandle( docRef, sizeof *appInfoP );
        THROW_IF( handle == NULL, dmErrMemError );

        appInfoID = MemHandleToLocalID( handle );
        DmSetDatabaseInfo( cardNo, dbID, NULL, NULL, NULL, NULL,
            NULL, NULL, NULL, &appInfoID, NULL, NULL, NULL );
    }
    appInfoP = MemLocalIDToLockedPtr( appInfoID, cardNo );
    DmSet( appInfoP, 0, sizeof *appInfoP, 0 );
    CategoryInitialize( ( AppInfoPtr ) appInfoP, strCatDefault );
    MemPtrUnlock( appInfoP );
}



/* Create database for list of documents */
static void CreateDocList( void )
    /* THROWS */
{
    UInt16  cardNo;
    Err     err;
    LocalID dbID;
    UInt16  version;

    /* list is always put on first card in RAM */
    cardNo  = 0;
    err     = errNone;

    err = DmCreateDatabase( cardNo, PlkrDocListName, ViewerAppID,
            PlkrDocListType, false );
    THROW_IF( err != errNone, memErrNotEnoughSpace );

    dbID    = DmFindDatabase( cardNo, PlkrDocListName );
    version = PlkrDocListVersion;
    err     = DmSetDatabaseInfo( cardNo, dbID, NULL, NULL, &version, 
                NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL );

    plkrDocList = DmOpenDatabaseByTypeCreator( PlkrDocListType, ViewerAppID,
                    dmModeReadWrite );

    ErrTry {
        InitPlkrAppInfo( plkrDocList );
    }
    ErrCatch( UNUSED_PARAM( err ) ) {
        LocalID dbID;

        DmOpenDatabaseInfo( plkrDocList, &dbID, NULL, NULL, &cardNo, NULL );
        DmCloseDatabase( plkrDocList );
        DmDeleteDatabase( cardNo, dbID );
        MSG( "Couldn't initialize Plkr document list [ appInfo ]\n" );
        ErrThrow( memErrNotEnoughSpace );
    } ErrEndCatch
}



/* Update Plucker document */
void UpdateDocument
    (
    DocumentInfo*   docInfo,
    Char*           volumeLabel,
    UInt16          index,
    MemHandle*      handle
    )
    /* THROWS */
{
    DocumentData*   dataPtr;
    UInt16          infoSize;
    UInt16          dataSize;

    infoSize = sizeof *dataPtr;
    if ( docInfo->location == RAM )
        dataSize = 0;
    else
        dataSize = StrLen( docInfo->filename ) + StrLen( volumeLabel ) + 2;

    *handle = DmResizeRecord( plkrDocList, index, infoSize + dataSize );
    dataPtr = MemHandleLock( *handle );

    DmWrite( dataPtr, 0, docInfo, infoSize );
    if ( docInfo->location != RAM ) {
        DmWrite( dataPtr, infoSize, docInfo->filename,
            StrLen( docInfo->filename ) + 1 );
        DmWrite( dataPtr, infoSize + StrLen( docInfo->filename ) + 1,
            volumeLabel, StrLen( volumeLabel ) + 1 );
    }
    MemHandleUnlock( *handle );
}
                    


/* Deactivate Plucker document */
static void DeactivateOneDocument
    (
    UInt16 index
    )
{
    MemHandle   handle;
    Boolean     active;

    active = false;
    handle = DmQueryRecord( plkrDocList, index );
    if ( handle != NULL ) {
        DocumentData* handlePtr;

        handlePtr = MemHandleLock( handle );
        if ( Prefs()->syncPolicy != SYNC_IGNORE_CARD ||
             handlePtr->location == RAM )
            DmWrite( handlePtr, OFFSETOF( DocumentData, active ), &active,
                sizeof( Boolean ) );
        MemHandleUnlock( handle );
    }
}
                    


/* Add Plucker document */
void AddDocument
    (
    DocumentInfo*   docInfo,
    Char*           volumeLabel
    )
    /* THROWS */
{
    MemHandle   handle;
    UInt8*      dataPtr;
    UInt16      infoSize;
    UInt16      dataSize;
    UInt16      dbIndex;

    infoSize = sizeof *docInfo - 2 * sizeof( UInt16) - sizeof( Char* );
    if ( docInfo->location == RAM )
        dataSize = 0;
    else
        dataSize = StrLen( docInfo->filename ) + StrLen( volumeLabel ) + 2;

    dbIndex = dmMaxRecordIndex;
    handle  = DmNewRecord( plkrDocList, &dbIndex, infoSize + dataSize );
    THROW_IF( handle == NULL, DmGetLastErr() );
    dataPtr = MemHandleLock( handle );
    DmWrite( dataPtr, 0, docInfo, infoSize );
    if ( docInfo->location != RAM ) {
        DmWrite( dataPtr, infoSize, docInfo->filename,
            StrLen( docInfo->filename ) + 1 );
        DmWrite( dataPtr, infoSize + StrLen( docInfo->filename ) + 1,
            volumeLabel, StrLen( volumeLabel ) + 1 );
    }
    MemHandleUnlock( handle );
    DmReleaseRecord( plkrDocList, dbIndex, true );
}
                    


/* Find record with document data in doclist, always returns a valid
   handle or throws an exception */
MemHandle FindDocData
    (
    Char*       name,           /* document name to search for */
    UInt16      numOfElements,  /* number of elements to search, 
                                   set to ALL_ELEMENTS to search 
                                   all elements */
    UInt16*     index           /* upon successful return, the index of
                                   the record, pass NULL for this
                                   parameter if you don't want to
                                   retrieve this value */
    )
    /* THROWS */
{
    THROW_IF( plkrDocList == NULL, dmErrNoOpenDatabase );
    THROW_IF( name == NULL, dmErrInvalidParam );

    if ( numOfElements == ALL_ELEMENTS )
        numOfElements = DmNumRecords( plkrDocList );

    return SearchRAMDocument( plkrDocList, CompareNames, name, numOfElements,
            (Int16 *)index );
}



void SetFoundDocument
    (
    UInt16 index
    )
{
    if ( index < removeListSize )
        removeList[ index ] = false;
}



/* Retrieve the default category/categories from the document */
UInt16 GetDefaultCategories
    (
    DocumentInfo* docInfo
    )
    /* THROWS */
{
    UInt16      categories;
    MemHandle   handle;
    Err         err;

    handle      = NULL;
    categories  = 0;

    if ( docInfo->location == RAM )
        err = OpenRAMDocument( docInfo );
    else
        err = OpenVFSDocument( docInfo );
    THROW_IF( err != errNone, err );

    GetIndexData();

    ErrTry {
        Header* categoryRecord;
        Char*   name;
        UInt16  totalSize;
        UInt16  size;
        UInt8   index;

        handle = FindRecord( CATEGORY_ID, NULL );
        if ( handle != NULL ) {
            categoryRecord  = MemHandleLock( handle );
            size            = 0;
            totalSize       = categoryRecord->size;
            name            = (Char*)( categoryRecord + 1 );
            do {
                index = GetCategoryIndex( name );
                if ( index == dmAllCategories ) {
                    index = AddCategoryToFreePosition( name );
                }
                if ( index != dmAllCategories )
                    categories |= ( 1 << index );

                size    += StrLen( name ) + 1;
                name    += StrLen( name ) + 1;
            } while ( size < totalSize );

            MemHandleUnlock( handle );
        }
        else {
            categories = 0;
        }
    }
    ErrCatch( UNUSED_PARAM( err ) ) {
        categories = 0;
    } ErrEndCatch

    if ( docInfo->location == RAM ) {
        CloseRAMDocument();
    }
    else {
        if ( handle != NULL )
            MemHandleFree( handle );
        CloseVFSDocument();
    }

    if ( categories == 0 )
        return UNFILED_CATEGORY;
    else
        return categories;
}



/* Get a handle to record containing document info */
MemHandle ReturnDocInfoHandle
    (
    const UInt16 index  /* record index */
    )
    /* THROWS */
{
    THROW_IF( plkrDocList == NULL, dmErrNoOpenDatabase );

    return DmQueryRecord( plkrDocList, index );
}



/* Get category name */
void GetCategoryName
    (
    UInt8   index,  /* category index */
    Char*   name    /* upon successful return, the name of
                       the category, otherwise an empty string */
    )
{
    CategoryGetName( plkrDocList, index, name );
}



Boolean ProcessSelectCategory(FormType* form, Boolean title,
            UInt16* category, Char* name)
{
    return CategorySelect( plkrDocList, form, frmLibraryCategoryPopup,
            frmLibraryCategoryList, title, category, name, 1,
            categoryEditStrID );
}



UInt16 GetNextCategory(UInt16 currentCategory)
{
    return CategoryGetNext( plkrDocList, currentCategory );
}



/* Set category name, return dmAllCategories if successful, otherwise
   the index for the already existing category */
UInt8 StoreCategoryName
    (
    UInt8   index,  /* category index */
    Char*   name    /* category name */
    )
{
    UInt8 i;

    i = CategoryFind( plkrDocList, name );
    if ( i == dmAllCategories )
        CategorySetName( plkrDocList, index, name );

    return i;
}



/* Add a new category to a free position, returns the index of the
   added category or dmAllCategories if no free position was found */
static UInt8 AddCategoryToFreePosition
    (
    Char* name
    )
{
    UInt8 category;

    for ( category = 0; category < dmRecNumCategories; category++ ) {
        if ( ! CategoryExists(category) ) {
            CategorySetName( plkrDocList, category, name );
            Prefs()->categories |= ( 1 << category );
            return category;
        }
    }
    return dmAllCategories;
}



/* Rename document name in document list */
void UpdateDocumentName
    (
    UInt16      index,  /* record index */
    const Char* name,   /* new document name */
    const Char* filename  /* new filename */
    )
    /* THROWS */
{
    MemHandle       handle;
    DocumentData*   handlePtr;

    THROW_IF( name == NULL || *name == '\0', errNoDocumentName );

    handle = DmQueryRecord( plkrDocList, index );
    THROW_IF( handle == NULL, DmGetLastErr() );

    handlePtr = MemHandleLock( handle );
    DmWrite( handlePtr, OFFSETOF( DocumentData, name ),
        name, StrLen( name ) + 1 );
    if ( handlePtr->location != RAM ) {
        DocumentData*   dataPtr;
        UInt16          infoSize;
        UInt16          dataSize;
        Char            volumeLabel[ LABEL_LEN ];
        UInt16          fileLength;
        UInt16          volumeLabelLength;

        fileLength          = StrLen( handlePtr->data ) + 1;
        volumeLabelLength   = StrLen( handlePtr->data + fileLength ) + 1;
        StrNCopy( volumeLabel, handlePtr->data + fileLength,
            volumeLabelLength );

        MemHandleUnlock( handle );

        infoSize = sizeof *dataPtr;
        dataSize = StrLen( filename ) + StrLen( volumeLabel ) + 2;

        handle  = DmResizeRecord( plkrDocList, index, infoSize + dataSize );
        dataPtr = MemHandleLock( handle );
        DmWrite( dataPtr, infoSize, filename, StrLen( filename ) + 1 );
        DmWrite( dataPtr, infoSize + StrLen( filename ) + 1, volumeLabel,
            StrLen( volumeLabel ) + 1 );
    }
    MemHandleUnlock( handle );
    DmInsertionSort( plkrDocList, CompareDocumentNames, 0 );
}



/* Remove record with document info, return result from database call */
void RemoveDocInfoRecord
    (
    UInt16 index    /* record index */
    )
    /* THROWS */
{
    Err err;

    err = DmRemoveRecord( plkrDocList, index );
    THROW_IF( err != errNone, err );
}



/* Return info for last openned document */
DocumentInfo* GetLastDocInfo( void )
{
    DocumentInfo* docInfo;

    docInfo = NULL;

    if ( plkrDocList == NULL ) {
        plkrDocList = DmOpenDatabaseByTypeCreator( PlkrDocListType, ViewerAppID,
                        dmModeReadWrite );
    }
    if ( plkrDocList != NULL ) {
        DocumentData*   recordPtr;
        MemHandle       handle;

        ErrTry {
            /* assign doc info values for document */
            handle = FindDocData( Prefs()->docName, ALL_ELEMENTS, NULL );
            if ( handle != NULL ) {
                recordPtr = MemHandleLock( handle );

                StrNCopy( lastDocInfo.name, recordPtr->name, dmDBNameLength );
                lastDocInfo.cardNo      = recordPtr->cardNo;
                lastDocInfo.created     = recordPtr->created;
                lastDocInfo.attributes  = recordPtr->attributes;
                lastDocInfo.size        = recordPtr->size;
                lastDocInfo.categories  = recordPtr->categories;
                lastDocInfo.location    = recordPtr->location;
                lastDocInfo.timestamp   = recordPtr->timestamp;
                if ( lastDocInfo.location != RAM ) {
                    UInt16 fileLength;

                    ReleaseLastDocInfo();

                    fileLength              = StrLen( recordPtr->data ) + 1;
                    lastDocInfo.filename    = SafeMemPtrNew( fileLength );
                    StrNCopy( lastDocInfo.filename, recordPtr->data,
                        fileLength );
                    lastDocInfo.volumeRef   = FindVolRefNum( recordPtr->data +
                                                             fileLength );
                }
                MemHandleUnlock( handle );
                CloseDocList();

                docInfo = &lastDocInfo;
            }
        }
        ErrCatch( UNUSED_PARAM( err ) ) {
        } ErrEndCatch

        CloseDocList();
    }
    return docInfo;
}



/* Release memory allocated for last openned document */
void ReleaseLastDocInfo( void )
{
    SafeMemPtrFree( lastDocInfo.filename );
    lastDocInfo.filename = NULL;
}



/* Open document list */
void OpenDocList( void )
    /* THROWS */
{
    if ( plkrDocList != NULL )
        return;

    plkrDocList = DmOpenDatabaseByTypeCreator( PlkrDocListType, ViewerAppID,
                    dmModeReadWrite );

    if ( plkrDocList != NULL ) {
        Err     err;
        LocalID dbID;
        UInt16  cardNo;
        UInt16  version;

        err = DmOpenDatabaseInfo( plkrDocList, &dbID, NULL, NULL, &cardNo,
                NULL );
        THROW_IF( err != errNone, err );

        err = DmDatabaseInfo( cardNo, dbID, NULL, NULL, &version, NULL, NULL,
                NULL, NULL, NULL, NULL, NULL, NULL );
        THROW_IF( err != errNone, err );

        /* if older version of document list then update list */
        if ( version != PlkrDocListVersion ) {
            UInt16  index;
            UInt16  dataOffset;
            Boolean writeLocation;
            Boolean writeActive;
            Boolean writeTimestamp;

            dataOffset      = OFFSETOF( DocumentData, data );
            writeLocation   = false;
            writeActive     = false;
            writeTimestamp  = false;
            if ( version < 4 ) {
                writeTimestamp  = true;
                dataOffset     -= 4;
            }
            if ( version < 3 ) {
                writeActive     = true;
                dataOffset     -= 2;
            }
            if ( version < 2 ) {
                writeLocation   = true;
                dataOffset      = 0;
            }

            for ( index = 0; index < DmNumRecords( plkrDocList ); index++ ) {
                UInt32          newSize;
                UInt32          dataSize;
                MemHandle       handle;
                DocumentData*   dataPtr;

                if ( dataOffset != 0 ) {
                    handle      = ReturnDocInfoHandle( index );
                    dataSize    = MemHandleSize( handle ) - dataOffset;
                }
                else {
                    dataSize = 0;
                }
                newSize = sizeof *dataPtr + dataSize;
                handle  = DmResizeRecord( plkrDocList, index, newSize );
                if ( handle == NULL ) {
                    DmCloseDatabase( plkrDocList );
                    DmDeleteDatabase( cardNo, dbID );
                    plkrDocList = NULL;
                    break;
                }
                dataPtr = MemHandleLock( handle );
                if ( dataSize != 0 ) {
                    MemPtr src;
                    MemPtr dst;

                    dst = SafeMemPtrNew( dataSize );
                    src = MemHandleLock( handle );
                    MemMove( dst, src + dataOffset, dataSize );
                    DmWrite( dataPtr, OFFSETOF( DocumentData, data ),
                        dst, dataSize );
                    MemHandleUnlock( handle );
                    SafeMemPtrFree( dst );
                }
                if ( writeLocation ) {
                    const UInt16 location = RAM;

                    DmWrite( dataPtr, OFFSETOF( DocumentData, location ),
                        &location, sizeof location );
                }
                if ( writeActive ) {
                    const Boolean active = true;

                    DmWrite( dataPtr, OFFSETOF( DocumentData, active ),
                        &active, sizeof active );
                }
                if ( writeTimestamp ) {
                    UInt32  timestamp;
                    LocalID metaID;
                    Char    metaName[ dmDBNameLength ];
                    
                    StrCopy( metaName, "Plkr" );
                    if ( dataPtr->location != RAM ) {
                        Char locationString[ 2 ];

                        StrIToA( locationString, dataPtr->location );
                        StrCat( metaName, locationString );
                    }
                    StrCat( metaName, "-" );
                    StrNCat( metaName, dataPtr->name, dmDBNameLength - 6 );
                    metaID = DmFindDatabase( cardNo, metaName );
                    if ( metaID != 0 ) {
                        timestamp = TimGetSeconds();
                    }
                    else {
                        timestamp = 0;
                    }
                    DmWrite( dataPtr, OFFSETOF( DocumentData, timestamp ),
                        &timestamp, sizeof timestamp );
                }
                MemHandleUnlock( handle );
            }
            version = PlkrDocListVersion;
            err     = DmSetDatabaseInfo( cardNo, dbID, NULL, NULL, &version, 
                        NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL );
            THROW_IF( err != errNone, err );

        }
    }

    if ( plkrDocList == NULL )
        CreateDocList();

    UpdateDocList();
}



/* Close document list */
void CloseDocList( void )
{
    if ( plkrDocList != NULL ) {
        DmCloseDatabase( plkrDocList );
        plkrDocList = NULL;
    }
}



/* Return number of Plucker documents */
UInt16 GetNumOfDocuments( void )
    /* THROWS */
{
    THROW_IF( plkrDocList == NULL, dmErrNoOpenDatabase );

    return DmNumRecords( plkrDocList );
}



/* Return category index if given category is included in list of categories, 
   otherwise dmAllCategories */
static UInt8 GetCategoryIndex
    (
    Char* name  /* category name */
    )
{
    return CategoryFind( plkrDocList, name );
}



/* Check if category already exists */
Boolean CategoryExists
    (
    UInt8 index /* category index */
    )
{
    Char name[ dmRecNumCategories ];

    GetCategoryName( index, name );
    if ( name[ 0 ] == '\0' )
        return false;
    else
        return true;
}



/* Remove category */
void RemoveCategoryName
    (
    UInt8 index /* category index */
    )
{
    MergeCategories( 0, index );
}



/* Merge categories */
void MergeCategories
    (
    UInt8 dst,  /* destination category */
    UInt8 src   /* source category */
    )
{
    UInt16 i;
    UInt16 category;

    category = 1 << src;

    for ( i = 0; i < DmNumRecords( plkrDocList ); i++ ) {
        MemHandle       handle;
        DocumentData*   handlePtr;
        UInt16          categories;

        handle = DmQueryRecord( plkrDocList, i );
        if ( handle == NULL ) {
            break;
        }
        handlePtr = MemHandleLock( handle );
        categories = handlePtr->categories;
        if ( ( categories & category ) != 0 ) {
            categories &= ~category;
            if ( dst != 0 )
                categories |= ( 1 << dst );
            else if ( categories == 0 )
                categories = 1;
            DmWrite( handlePtr, OFFSETOF( DocumentData, categories ),
                &categories, sizeof categories );
        }
        MemHandleUnlock( handle );
    }
    CategorySetName( plkrDocList, src, NULL );
}

