/*
 * $Id: vfsfile.c,v 1.43 2004/04/24 03:12:27 prussar Exp $
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
#include <VFSMgr.h>

#include "const.h"
#include "debug.h"
#include "resourceids.h"
#include "os.h"
#include "doclist.h"
#include "metadocument.h"
#include "document.h"
#include "axxpacimp.h"
#include "font.h"

#include "vfsfile.h"

#ifdef HAVE_PALMCUNIT
#include "mock_Header.h"
#endif


/***********************************************************************
 *
 *      Internal Constants
 *
 ***********************************************************************/
#define UNFILED_CATEGORY    1
#define BUFSIZE             32767
#define LABEL_LEN           256
#define PDB_SUFFIX          ".pdb"
#define PDB_SUFFIX_LEN      4
#define MAX_DOCNAME_LEN     dmDBNameLength - 6 /* "Plkr-" + <LOCATION> */
#define UID_CACHE_LEN       10

/* MIME type for Plucker documents */
#define ViewerMIMEType  "application/prs.plucker"


#define IsDocScan( e )     (( ( e ) & ENUMERATECARD_DOCS ) != 0 )
#define IsFontScan( e )    (( ( e ) & ENUMERATECARD_FONTS ) != 0 )


static const Char* PluckerDirList[] = {
    "INVALID",
    "/Palm/Programs/Plucker/",
    "/Palm/Ebooks/",
    "/Palm/Launcher/"
};

static const Char* FontDirList[] = {
    "/Palm/Programs/Plucker/Fonts/",
    "/Palm/Launcher/"
};

static const Char illegalFilenameChars[] = "\"*?\\/<>:|";
static const Char fakeVolumeNameFormat[] = "{Plkr--volume number %04X}";

/***********************************************************************
 *
 *      Local functions
 *
 ***********************************************************************/
static void GetPluckerVolumeLabel( UInt16 volRefNum, Char* volumeLabel, 
    UInt16 length );
static void AddVFSFile( UInt16 volRefNum, FileInfoType* info,
                const Char* dir, UInt16 location,
                EnumerateCardType enumerateWhat ) VFSFILE_SECTION;
static void EnumerateDir( UInt16 volRefNum, UInt16 location, const Char* dir,
                EnumerateCardType enumerateWhat ) VFSFILE_SECTION;
static void BeamStream( ExgSocketType* s, FileRef fileRef ) VFSFILE_SECTION;
static Err OpenVFSDatabase( UInt16 Ref, Char* path, FileRef* fileRef,
                UInt16* version, Char* name, UInt32* date,
                UInt16* numRecords ) VFSFILE_SECTION;
static void MakeLegalFilename( Char* s ) VFSFILE_SECTION;
static void VFSRenameMetaDocument( Char* newName, UInt16 oldLocation,
                DocumentInfo* docInfo ) VFSFILE_SECTION;
static MemHandle SearchVFSDocument ( UInt16 uid, Int16* index )
                VFSFILE_SECTION;
static UInt16 GetUID ( MemHandle handle ) VFSFILE_SECTION;



/***********************************************************************
 *
 *      Internal types
 *
 ***********************************************************************/
typedef struct {
    UInt16 uid;
    UInt16 recordNum;
} UIDCacheEntry;



/***********************************************************************
 *
 *      Private variables
 *
 ***********************************************************************/
static FileRef          vfsDocument;
static DocumentInfo*    lastDocInfo;
static UInt16           numOfElements;
static Char             lastVolumeLabel[ LABEL_LEN ];
static UInt16           lastVolumeRefNum;
static Boolean          haveLastVolumeLabel = false;
static UIDCacheEntry    uidCache[ UID_CACHE_LEN ];
static UInt16           numUIDCacheEntries;



/* Get a VFS volume label, replacing with a fake one if there is an error
   getting the real one. */
static void GetPluckerVolumeLabel
    (
    UInt16 volRefNum,
    Char*  volumeLabel,
    UInt16 length      /* must be at least large enough to hold fake name */
    )
{
    Err  err;
    err = VFSVolumeGetLabel( volRefNum, volumeLabel, length );
    if ( err != errNone )
        StrPrintF( volumeLabel, fakeVolumeNameFormat, volRefNum );
}        



/* Open the specified database, return the result from the database call */
static Err OpenVFSDatabase
    (
    UInt16      volumeRef,  /* file's volume reference */
    Char*       path,       /* path to file */
    FileRef*    fileRef,    /* upon successful return, the access pointer */
    UInt16*     version,    /* upon successful return, the specific version
                               number */
    Char*       name,       /* upon successful return, the name */
    UInt32*     date,       /* upon successful return, the creation date */
    UInt16*     numRecords  /* upon successful return, the number of records */
    )
{
    Err err;

    if ( path == NULL || fileRef == NULL || version == NULL || name == NULL ||
         date == NULL || numRecords == NULL )
        return vfsErrBadData;

    err = VFSFileOpen( volumeRef, path, vfsModeRead, fileRef );
    if ( err != errNone )
       return err;

    err = VFSFileDBInfo( *fileRef, name, NULL, NULL, date, NULL, NULL, NULL,
            NULL, NULL, NULL, NULL, numRecords );

    return err;
}



/* Add file to doc list */
static void AddVFSFile
    (
    UInt16              volRefNum,
    FileInfoType*       info,
    const Char*         dir,
    UInt16              location,
    EnumerateCardType   enumerateWhat
    )
{
    Err             err;
    Char*           path;
    FileRef         ref;
    DocumentInfo    docInfo;
    UInt32          type;
    UInt32          creatorID;

    /* Set path to document */
    path = SafeMemPtrNew( StrLen( dir ) + StrLen( info->nameP ) + 2 );
    StrCopy( path, dir );
    StrCat( path, info->nameP );
    err = VFSFileOpen( volRefNum, path, vfsModeRead, &ref );
    if ( err == errNone ) {
        Char volumeLabel[ LABEL_LEN ];

        err = VFSFileDBInfo( ref, docInfo.name,
                &docInfo.attributes, NULL, &docInfo.created,
                NULL, NULL, NULL, NULL, NULL, &type, &creatorID,
                NULL );
        if ( err == errNone && IsDocScan( enumerateWhat ) &&
             type == ViewerDocumentType && creatorID == ViewerAppID ) {
            DocumentData*   handlePtr;
            MemHandle       handle;
            UInt16          index;

            VFSFileSize( ref, &docInfo.size );
            GetPluckerVolumeLabel( volRefNum, volumeLabel, LABEL_LEN );
            docInfo.cardNo      = 0;
            docInfo.filename    = info->nameP;

            VFSFileClose( ref );

            handle = FindDocData( docInfo.name, numOfElements, &index );
            if ( handle != NULL ) {
                UInt16  categories;
                UInt32  oldCreated;
                UInt16  oldLocation;
                Boolean oldActive;
                Char    oldVolumeLabel[ LABEL_LEN ];
                UInt16  fileLen;
                UInt16  volumeLabelLen;
                UInt32  timestamp;

                SetFoundDocument( index );
                handlePtr       = MemHandleLock( handle );
                oldCreated      = handlePtr->created;
                oldLocation     = handlePtr->location;
                oldActive       = handlePtr->active;
                categories      = handlePtr->categories;
                timestamp       = handlePtr->timestamp;
                fileLen         = StrLen( handlePtr->data ) + 1;
                volumeLabelLen  = StrLen( handlePtr->data + fileLen ) + 1;
                StrNCopy( oldVolumeLabel, handlePtr->data + fileLen,
                    volumeLabelLen );
                MemHandleUnlock( handle );
                /* If the document is newer than the stored one, or
                   moved to external card then updated info */
                if ( ! oldActive || oldCreated < docInfo.created ||
                     oldLocation != location ||
                     ! STREQ( volumeLabel, oldVolumeLabel ) ) {
                    /* Assign remaining document data */
                    docInfo.location    = location;
                    docInfo.active      = true;
                    docInfo.categories  = categories;
                    /* A "new" document should be indicated as unread */
                    if ( ! oldActive || oldCreated < docInfo.created )
                        docInfo.timestamp = 0;
                    else
                        docInfo.timestamp = timestamp;

                    /* If document has been moved to external card the
                       meta database must be renamed */
                    if ( oldLocation != location )
                        VFSRenameMetaDocument( docInfo.name, oldLocation,
                            &docInfo );

                    UpdateDocument( &docInfo, volumeLabel, index,
                        &handle );
                }
            }
            else {
                docInfo.location    = location;
                docInfo.volumeRef   = volRefNum;
                docInfo.active      = true;
                docInfo.timestamp   = 0;

                ErrTry {
                    docInfo.categories = GetDefaultCategories( &docInfo );
                }
                ErrCatch( UNUSED_PARAM( err ) ) {
                    docInfo.categories = UNFILED_CATEGORY;
                } ErrEndCatch
                AddDocument( &docInfo, volumeLabel );
            }
        }
#ifdef SUPPORT_VFS_FONTS
        else if ( err == errNone && IsFontScan( enumerateWhat ) &&
                  type == UserFontResourceType && creatorID == ViewerAppID ) {
            GetPluckerVolumeLabel( volRefNum, volumeLabel, LABEL_LEN );
            AddVFSFont( docInfo.name, volumeLabel, path );
            VFSFileClose( ref );
        }
#endif
        else {
            VFSFileClose( ref );
        }
    }
    SafeMemPtrFree( path );
}



/* Enumerate documents in directory */
static void EnumerateDir
    (
    UInt16              volRefNum,
    UInt16              location,
    const Char*         dir,
    EnumerateCardType   enumerateWhat
    )
    /* THROWS */
{
    FileInfoType    info;
    FileRef         dirRef;
    UInt32          dirIterator;
    Char*           filename = NULL;
    Err             err;

    err = VFSFileOpen( volRefNum, dir, vfsModeRead, &dirRef );
    if ( err == errNone ) {
        ErrTry {
            filename = SafeMemPtrNew( PATH_LEN );
        }
        ErrCatch( err ) {
            VFSFileClose( dirRef );
            ErrThrow( err );
        } ErrEndCatch

        info.nameP      = filename;
        info.nameBufLen = PATH_LEN;
        dirIterator     = vfsIteratorStart;
        while ( dirIterator != vfsIteratorStop ) {
            err = VFSDirEntryEnumerate( dirRef, &dirIterator, &info );
            if ( err == errNone ) {
                if ( info.attributes != vfsFileAttrDirectory ) {
                    ErrTry {
                        AddVFSFile( volRefNum, &info, dir, location,
                            enumerateWhat );
                    }
                    ErrCatch( err ) {
                        SafeMemPtrFree( filename );
                        VFSFileClose( dirRef );
                        ErrThrow( err );
                    } ErrEndCatch
                }
            }
            else if ( err != expErrEnumerationEmpty ) {
                SafeMemPtrFree( filename );
                VFSFileClose( dirRef );
                ErrThrow( err );
            }
        }
        SafeMemPtrFree( filename );
        VFSFileClose( dirRef );
    }
    else if ( err != vfsErrFileNotFound ) {
        ErrThrow( err );
    }
}



/* Send stream of data */
static void BeamStream
    (
    ExgSocketType*  s,      /* pointer to socket */
    FileRef         fileRef /* file reference */
    )
    /* THROWS */
{
    UInt32  sentBytes;
    UInt32  readBytes;
    UInt8*  buf;
    UInt8*  startOfBuf;
    Err     err;

    buf         = SafeMemPtrNew( BUFSIZE );
    startOfBuf  = buf;
    ErrTry {
        while ( ! VFSFileEOF( fileRef ) ) {
            /* Read the stream from VFS and send it */
            err = VFSFileRead( fileRef, BUFSIZE, buf, &readBytes );

            /* EOF is not an error at this point -- we handle that above
               using VFSFileEOF */
            THROW_IF( err != errNone && err != vfsErrFileEOF, err );

            while ( 0 < readBytes ) {
                sentBytes = ExgSend( s, buf, readBytes, &err );
                THROW_IF( err != errNone, err );

                readBytes  -= sentBytes;
                buf        += sentBytes;
            }
            buf = startOfBuf;
        }
        SafeMemPtrFree( startOfBuf );
    }
    ErrCatch( err ) {
        SafeMemPtrFree( startOfBuf );
        ErrThrow( err );
    } ErrEndCatch
}



/* return UID for last record in document, i.e. the max UID */
UInt16 GetVFSMaxUID( void )
{
    UID*        uid;
    MemHandle   handle;
    UInt16      lastIndex;
    UInt16      maxUID;
    Err         err;

    lastIndex   = lastDocInfo->numRecords - 1;
    err         = VFSFileDBGetRecord( vfsDocument, lastIndex, &handle, NULL,
                    NULL );
    if ( err != errNone ) {
        if ( handle != NULL )
            MemHandleFree( handle );
        return 1;
    }

    uid     = MemHandleLock( handle );
    maxUID  = uid->uid;
    MemHandleUnlock( handle );
    MemHandleFree( handle );
    
    return maxUID;
}



/* find volume ref with given volume label */
UInt16 FindVolRefNum
    (
    Char* label
    )
    /* THROWS */
{
    UInt32  volIterator;
    UInt16  volRefNum;
    Char*   volumeLabel;
    Err     err;

    if ( haveLastVolumeLabel && STREQ( lastVolumeLabel, label ) ) {
        return lastVolumeRefNum;
    }

    volRefNum   = 0;
    volumeLabel = SafeMemPtrNew( LABEL_LEN );
    volIterator = vfsIteratorStart;
    while ( volIterator != vfsIteratorStop ) {
        err = VFSVolumeEnumerate( &volRefNum, &volIterator );
        if ( err != errNone ) {
            volRefNum = 0;
            break;
        }

        /* get the volume label */
        GetPluckerVolumeLabel( volRefNum, volumeLabel, LABEL_LEN );
        if ( STREQ( label, volumeLabel ) ) {
            break;
        }
    }
    SafeMemPtrFree( volumeLabel );

    if ( volRefNum != 0 ) {
        StrNCopy( lastVolumeLabel, volumeLabel, LABEL_LEN );
        lastVolumeRefNum    = volRefNum;
        haveLastVolumeLabel = false;
    }
    return volRefNum;
}



/* Enumerate mounted volumes */
void EnumerateCards
     (
     EnumerateCardType enumerateWhat
     )
    /* THROWS */
{
    UInt16  volRefNum;
    UInt32  volIterator;
    //UInt16  volume;
    UInt16  dirIndex;
    Err     err;

    if ( ! SupportVFS() )
        return;

    haveLastVolumeLabel = false;

    /* set number of elements to search */
    if ( IsDocScan( enumerateWhat ) )
        numOfElements = GetNumOfDocuments();

    volRefNum   = 0;
    err         = errNone;

    /* Iterate all volumes */
    //volume      = 0;
    volIterator = vfsIteratorStart;
    while ( volIterator != vfsIteratorStop ) {
        err = VFSVolumeEnumerate( &volRefNum, &volIterator );
        if ( err == expErrEnumerationEmpty ) {
            break;
        }
        else if ( err != errNone ) {
            ErrThrow( err );
        }

        /* Find all Plucker documents on the volume */
        if ( IsDocScan( enumerateWhat ) ) {
            for ( dirIndex = 1;
                  dirIndex < sizeof PluckerDirList / sizeof PluckerDirList[ 0 ];
                  dirIndex++ ) {
                EnumerateDir( volRefNum, dirIndex, PluckerDirList[ dirIndex ],
                    ENUMERATECARD_DOCS );
            }
        }
        /* Find all fonts on the volume */
        if ( IsFontScan( enumerateWhat ) ) {
            for ( dirIndex = 0;
                  dirIndex < sizeof FontDirList / sizeof FontDirList[ 0 ];
                  dirIndex++ ) {
                EnumerateDir( volRefNum, dirIndex, FontDirList[ dirIndex ],
                    ENUMERATECARD_FONTS );
            }
        }
    }
}


/* Open the specified database using VFS, return the result from
   the database call */
Err OpenVFSDocument
    (
    DocumentInfo*   docInfo
    )
{
    Char    path[ PATH_LEN ];
    UInt16  version;
    Err     err;

    numUIDCacheEntries = 0;

    if ( docInfo == NULL || sizeof PluckerDirList <= docInfo->location )
        return vfsErrBadData;

    StrNCopy( path, PluckerDirList[ docInfo->location ], PATH_LEN );
    StrNCat( path, docInfo->filename, PATH_LEN );

    err = OpenVFSDatabase( docInfo->volumeRef, path, &vfsDocument, &version,
            docInfo->name, &docInfo->created, &docInfo->numRecords );
    if ( err != errNone )
        return err;

    lastDocInfo = docInfo;

    FindRecord              = FindVFSRecord;
    GetNumOfRecords         = GetVFSNumOfRecords;
    ReturnRecordHandle      = ReturnVFSRecordHandle;
    ReturnRecordHandleByIndex = ReturnVFSRecordHandleByIndex;
    ReturnNextRecordHandle  = ReturnNextVFSRecordHandle;
    GetMaxUID               = GetVFSMaxUID;
    FreeRecordHandle        = FreeVFSRecordHandle;

    return errNone;
}



/* Close the specified file */
void CloseVFSDocument( void )
{
    if ( vfsDocument != 0 ) {
        VFSFileClose( vfsDocument );
        vfsDocument             = 0;
    }
}



/* Delete specified document */
void DeleteVFSDocument
    (
    DocumentInfo*   docInfo
    )
    /* THROWS */
{
    Err     err;
    Char    path[ PATH_LEN ];
 
    THROW_IF( docInfo == NULL, vfsErrBadData );
    THROW_IF( sizeof PluckerDirList <= docInfo->location, vfsErrBadData );
    THROW_IF( vfsDocument != 0, vfsErrFileStillOpen );

    StrNCopy( path, PluckerDirList[ docInfo->location ], PATH_LEN );
    StrNCat( path, docInfo->filename, PATH_LEN );

    err = VFSFileDelete( docInfo->volumeRef, path );
    THROW_IF( err != errNone, err );
}



/* Return a handle to a record */
MemHandle ReturnVFSRecordHandle
    (
    UInt16 uid  /* unique ID to search for */
    )
    /* THROWS */
{
    //THROW_IF( uid == 0, vfsErrBadData );
    return SearchVFSDocument( uid, NULL );
}



/* Return a handle to next record */
MemHandle ReturnNextVFSRecordHandle
    (
    UInt16* index  /* pointer to index of a known record */
    )
    /* THROWS */
{
    MemHandle   handle;
    Header*     record;
    UInt16      lastIndex;

    if ( *index == 0 )
        *index = 1;

    handle      = NULL;
    lastIndex   = lastDocInfo->numRecords - 1;
    while ( *index <= lastIndex ) {
        Err err;

        err = VFSFileDBGetRecord( vfsDocument, *index, &handle, NULL, NULL );
        if ( err != errNone ) {
            if ( handle != NULL ) {
                MemHandleFree( handle );
                handle = NULL;
            }
            break;
        }

        record = MemHandleLock( handle );
        if ( record->type == DATATYPE_PHTML ||
             record->type == DATATYPE_PHTML_COMPRESSED ) {
            MemHandleUnlock( handle );
            break;
        }
        MemHandleUnlock( handle );
        MemHandleFree( handle );
        handle = NULL;
        ( *index )++;
    }
    return handle;
}



MemHandle ReturnVFSRecordHandleByIndex
    (
    const UInt16 index  /* index of a record */
    )
{
    Err err;
    MemHandle handle;

    err = VFSFileDBGetRecord( vfsDocument, index, &handle, NULL, NULL );
    if ( err != errNone ) {
        if ( handle != NULL ) {
            MemHandleFree( handle );
            handle = NULL;
        }
    }
    return handle;
}



static UInt16 GetUID
    (
    MemHandle handle
    )
{
    UID*    currentUID;
    UInt16  uid;

    currentUID  = MemHandleLock( handle );
    uid         = currentUID->uid;
    MemHandleUnlock( handle );

    return  uid;
}



/* Find record in document, returns handle to record or NULL if not found */
static MemHandle SearchVFSDocument
    (
    UInt16          uid,       /* item to search for */
    Int16*          index       /* upon successful return, the index of the
                                   record, pass NULL for this parameter if
                                   you don't want to retrieve this value */
    )
{
/* This routine is highly optimized to handle large VFS documents, ones with
   1000 or 2000 records.  The optimizations are based on the fact that most
   of the cost is in the calls to VFSFileDBGetRecord().  Any processing that
   we do to save such calls is worth it in terms of execution time.
   */
    MemHandle   handle;
    UInt16      lower;    /* lower record number bound for search */
    UInt16      upper;    /* upper record number bound for search */
    Err         err;
    UInt16      uidUpper; /* overestimate for uid[upper]; else 0 */
    UInt16      uidLower; /* underestimate for uid[lower] */
    UInt16      i;
    UInt16      current;  /* current record number */

    if ( vfsDocument == 0 )
        return NULL;

    handle      = NULL;
    current     = 0;
    lower       = 0;
    upper       = lastDocInfo->numRecords - 1;
    uidLower    = 0;
    uidUpper    = 0;

    for ( i = 0 ; i < numUIDCacheEntries ; i++ ) {
        /* Check the cache whether we have the target uid
           already in it. */
        if ( uidCache[ i ].uid == uid ) {
            if ( index != NULL ) {
                *index = uidCache[ i ].recordNum;
            }
            err  = VFSFileDBGetRecord( vfsDocument, uidCache[ i ].recordNum,
                       &handle, NULL, NULL );
            if ( err != errNone ) {
                if ( handle != NULL ) {
                    MemHandleFree( handle );
                    handle = NULL;
                }
                ErrThrow( err );
            }
            return handle;
        }
        /* Adjust the binary search bounds if applicable */
        if ( uidCache[ i ].uid < uid ) {
            if ( lower <= uidCache[ i ].recordNum ) {
                lower    = uidCache[ i ].recordNum + 1;
                uidLower = uidCache[ i ].uid + 1;
            }
        }
        else if ( uidCache[ i ].recordNum <= upper ) {
            upper    = uidCache[ i ].recordNum - 1;
            uidUpper = uidCache[ i ].uid - 1;
        }
    }

    if ( uidUpper == 0 ) {
        uidUpper    = GetVFSMaxUID();
        if ( uidUpper < uid )
            return NULL;
    }

    while ( lower <= upper ) {
        UInt16      currentUID;
        UInt16      oldUpper;

        /*
           We now make some arithmetical adjustments to to the lower and upper
           bounds of the binary search.  These adjustments speed things up
           considerably if the UIDs are spaced apart by 1, speed things up
           somewhat in some other cases, and never slow things down
           (the cost of the adjustments is negligible compared to the cost
           of VFSFileDBGetRecord()).  The adjustments are based on two
           inequalities, where uid[n] is the uid of the nth record,
           and we are searching for current, knowing uid[x], and knowing that
           lower <= current and current <= upper.  Both inequalities are based
           on the fact that for any i and j, if i <= j, then:
             j - i <= uid[j] - uid[i].
           From this follow two helpful inequalities:
             (1) x <= lower + ( uid[x] - uid[lower] )
             (2) upper - ( uid[upper] - uid[x] ) <= x
           What this means is that we can replace the upper bound, upper, of
           the binary search by lower + (uid[current] - uid[lower]) when this
           is less than upper, and the lower bound by
           upper - ( uid[upper] - uid[current] ).
           Moreover, we are free to replace uid[lower] by an underestimate and
           uid[upper] by an overestimate here, which is all that might be
           available at a given time.
         */
        oldUpper = upper;
        if ( ( UInt32 ) lower + ( UInt32 ) uid <
                 ( UInt32 ) upper + ( UInt32 ) uidLower )
            upper = lower + uid - uidLower;
        if ( 0 < uidUpper &&
             ( UInt32 ) lower + ( UInt32 ) uidUpper <
                 ( UInt32 ) oldUpper + ( UInt32 ) uid )
            lower = oldUpper + uid - uidUpper;

        /* we are basically doing a binary search,
           subject to the adjustments */
        current = ( UInt16 ) ( ( ( UInt32 ) upper + ( UInt32 ) lower ) / 2 );

        err  = VFSFileDBGetRecord( vfsDocument, current, &handle, NULL, NULL );
        if ( err != errNone ) {
            if ( handle != NULL ) {
                MemHandleFree( handle );
                handle = NULL;
            }
            ErrThrow( err );
        }

        currentUID = GetUID( handle );
        if ( currentUID == uid ) {
            if ( index != NULL )
                *index = current;
            break;
        }
        if ( uid < currentUID ) {
            upper    = current - 1;
            uidUpper = currentUID - 1;
        }
        else {
            lower    = current + 1;
            uidLower = currentUID + 1;
        }

        MemHandleFree( handle );
        handle = NULL;
    }

    if ( handle != NULL ) {
        if ( numUIDCacheEntries == UID_CACHE_LEN ) {
            MemMove( uidCache, uidCache + 1, ( UID_CACHE_LEN - 1 ) *
                sizeof( UIDCacheEntry ) );
            numUIDCacheEntries--;
        }
        uidCache[ numUIDCacheEntries ].uid       = uid;
        uidCache[ numUIDCacheEntries ].recordNum = current;
        numUIDCacheEntries++;
    }

    return handle;
}



/* Find record with given uid in document, returns true if successful */
MemHandle FindVFSRecord
    (
    UInt16          uid,        /* uid to search for */
    Int16*          index       /* upon successful return, the index of the
                                   record, pass NULL for this parameter if
                                   you don't want to retrieve this value */
    )
    /* THROWS */
{
    //THROW_IF( uid == 0, vfsErrBadData );
    return SearchVFSDocument( uid, index );
}



/* Make a name more likely to be legal */
static void MakeLegalFilename( Char* s )
{
    while ( *s ) {
        if ( NULL != StrChr( illegalFilenameChars, ( UInt8 )*s ) ||
             (UInt8)*s < ' ' )
            *s = '_';
        s++;
    }
}



/* Rename document and its associated meta document */
void RenameVFSDocument
    (
    Char*           newName,        /* the new database name is entered here */
    DocumentInfo*   docInfo,
    Char*           filename        /* here the new filename is returned */
    )
    /* THROWS */
{
    Err     err;
    Char    path[ PATH_LEN ];
    FileRef fileRef;

    THROW_IF( newName == NULL || *newName == '\0', errNoDocumentName );
    THROW_IF( docInfo == NULL, vfsErrBadData );

    StrNCopy( path, PluckerDirList[ docInfo->location ], PATH_LEN );
    StrNCat( path, docInfo->filename, PATH_LEN -
        StrLen( PluckerDirList[ docInfo->location ] ) );

    /* rename document -- both the filename and document name */
    err = VFSFileOpen( docInfo->volumeRef, path, vfsModeWrite, &fileRef );
    THROW_IF( err != errNone, err );
    err = VFSFileWrite( fileRef, StrLen( newName ) + 1, newName, NULL );
    THROW_IF( err != errNone, err );
    err = VFSFileClose( fileRef );
    THROW_IF( err != errNone, err );

    StrNCopy( filename, newName, dmDBNameLength );
    StrCat( filename, PDB_SUFFIX );

    MakeLegalFilename( filename );

    /* Ignore the error code here */
    err = VFSFileRename( docInfo->volumeRef, path, filename );

    /* Same name as before */
    if ( err != errNone )
        StrCopy( filename, docInfo->filename );

    VFSRenameMetaDocument( newName, docInfo->location, docInfo );

}



static void VFSRenameMetaDocument
    (
    Char*           newName,
    UInt16          oldLocation,
    DocumentInfo*   docInfo
    )
    /* THROWS */
{
    Char    metaNewName[ dmDBNameLength ];
    Char    locationString[ 2 ];

    StrIToA( locationString, docInfo->location );
    StrCopy( metaNewName, "Plkr" );
    StrCat( metaNewName, locationString );
    StrCat( metaNewName, "-" );
    StrNCat( metaNewName, newName, MAX_DOCNAME_LEN );

    RenameMetaDocument( metaNewName, oldLocation, docInfo );
}



/* Beam Plucker document located on external card */
void BeamVFSDocument
    (
    DocumentInfo* docInfo
    )
    /* THROWS */
{
    ExgSocketType   s;
    Err             err;

    THROW_IF( docInfo == NULL, vfsErrBadData );

    MemSet( &s, sizeof( ExgSocketType ), 0 );

    s.name          = docInfo->filename;
    s.type          = ViewerMIMEType;
    s.description   = docInfo->name;
    s.target        = ViewerAppID;

    err = ExgPut( &s );
    THROW_IF( err != errNone, err );

    ErrTry {
        /* Open the database so we can get a fileRef */
        err = OpenVFSDocument( docInfo );
        THROW_IF( err != errNone, err );
        BeamStream( &s, vfsDocument );
        CloseVFSDocument();
        err = ExgDisconnect( &s, err );
        THROW_IF( err != errNone, err );
    }
    ErrCatch( err ) {
        CloseVFSDocument();
        err = ExgDisconnect( &s, err );
        ErrThrow( err );
    } ErrEndCatch
}



/* Return number of records in document */
UInt16 GetVFSNumOfRecords( void )
{
    return lastDocInfo->numRecords;
}



/* Free allocated handle */
void FreeVFSRecordHandle
    (
    MemHandle* handle
    )
{
    if ( handle != NULL && *handle != NULL ) {
        MemHandleFree( *handle );
        *handle = NULL;
    }
}

