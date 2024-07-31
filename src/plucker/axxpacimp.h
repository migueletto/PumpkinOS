/*
 * $Id: axxpacimp.h,v 1.1 2003/06/08 12:42:40 nordstrom Exp $
 *
 * Viewer - a part of Plucker, the free off-line HTML viewer for PalmOS
 * Copyright (c) 1998-2001, Mark Ian Lillywhite and Michael Nordstrom
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


#ifndef PLUCKER_AXXPACIMP_H
#define PLUCKER_AXXPACIMP_H


#ifdef HAVE_AXXPAC

#include <VFSMgr.h>
#include <PalmCompatibility.h> /* axxPac.h requires this header file */
#include <axxPac.h>
#include "viewer.h"

/* Different values of the static variable axxPacStatus */
#define AXXPAC_STATUS_UNKNOWN 0   
#define AXXPAC_PRESENT 1
#define AXXPAC_NOT_PRESENT 2
#define AXXPAC_ERR_FILE_CLOSED 65506

typedef struct file_rec {
    char    name[256];
    axxPacFD  fd; /* stores the actual File Descriptor used by the axxPac */
    unsigned char attrib;
    Int32 size;
    Int32 position;
    axxPacMode mode;
    Int16 fileRef; /* stores an internally generated number to identify
                      the file this is what is externally seen as FileRef */
} file_rec_t;

/************************************************************
 * Structure of a record list extension. This is used if all
 * the database record/resource entries of a database can't fit
 * into the database header.
 *************************************************************/
typedef struct {
    LocalID nextRecordListID; /* local chunkID of next list */
    UInt16 numRecords; /* number of records in this list */
    UInt16 firstEntry; /* array of Record/Rsrc entries */
    /* starts here */
} RecordListType;


/************************************************************
 * Structure of a Database Header
 *************************************************************/
typedef struct {
    UInt8 name[dmDBNameLength]; /* name of database */
    UInt16 attributes; /* database attributes */
    UInt16 version; /* version of database */
    UInt32 creationDate; /* creation date of database */
    UInt32 modificationDate; /* latest modification date */
    UInt32 lastBackupDate; /* latest backup date */
    UInt32 modificationNumber; /* modification number of database */
    LocalID appInfoID; /* application specific info */
    LocalID sortInfoID; /* app specific sorting info */
    UInt32 type; /* database type */
    UInt32 creator; /* database creator */
    UInt32 uniqueIDSeed; /* used to generate unique IDs. */
    /* Note that only the low order
       3 bytes of this is used (in
       RecordEntryType.uniqueID).
       We are keeping 4 bytes for
       alignment purposes. */
    RecordListType recordList; /* first record list */
} DatabaseHdrType;


/* Wrappers to VFS functions */
extern Err vfsVolumeEnumerate(UInt16 *volRefNumP,
                UInt32 *volIteratorP) AXXPAC_SECTION;
extern Err vfsVolumeInfo(UInt16 volRefNum,
                VolumeInfoType *volInfoP) AXXPAC_SECTION;
extern Err vfsFileOpen(UInt16 volRefNum, const Char *pathNameP,
                UInt16 openMode, FileRef *fileRefP) AXXPAC_SECTION;
extern Err vfsDirEntryEnumerate(FileRef dirRef, UInt32 *dirEntryIteratorP,
            FileInfoType *infoP) AXXPAC_SECTION;
extern Err vfsFileDBInfo(FileRef ref, Char *nameP, UInt16 *attributesP,
                       UInt16 *versionP, UInt32 *crDateP, UInt32 *modDateP,
                       UInt32 *bckUpDateP, UInt32 *modNumP, 
                       MemHandle *appInfoHP, MemHandle *sortInfoHP, 
                       UInt32 *typeP, UInt32 *creatorP,
                       UInt16 *numRecordsP) AXXPAC_SECTION; 
extern Err vfsFileSize(FileRef fileRef, UInt32 *fileSizeP) AXXPAC_SECTION;
extern Err vfsFileGetAttributes(FileRef fileRef,
                UInt32 *attributesP) AXXPAC_SECTION;
extern Err vfsFileClose(FileRef fileRef) AXXPAC_SECTION;
extern Err vfsFileDBGetRecord(FileRef ref,UInt16 recIndex, MemHandle *recHP,
            UInt8 *recAttrP, UInt32 *uniqueIDP) AXXPAC_SECTION;
extern Err vfsDirCreate(UInt16 volRefNum, Char *dirNameP) AXXPAC_SECTION; 
extern Err vfsExportDatabaseToFile (UInt16 volRefNum, Char *pathNameP,
            UInt16 cardNo, LocalID dbID) AXXPAC_SECTION;
extern Err vfsFileDelete(UInt16 volRefNum, Char *pathNameP) AXXPAC_SECTION;
extern Err vfsImportDatabaseFromFile(UInt16 volRefNum, Char *pathNameP,
                UInt16 *cardNoP, LocalID *dbIDP) AXXPAC_SECTION;
extern Err vfsFileRead(FileRef fileRef, UInt32 numBytes, void *bufP,
                UInt32 *numBytesReadP) AXXPAC_SECTION;
extern Err vfsFileEOF(FileRef fileRef) AXXPAC_SECTION;
extern Err vfsVolumeGetLabel(UInt16 volRefNum, Char *labelP,
                UInt16 bufLen) AXXPAC_SECTION;
extern Err vfsFileWrite(FileRef fileRef, UInt32 numBytes, void *dataP,
                UInt32 *numBytesWrittenP) AXXPAC_SECTION;
extern Err vfsFileRename(UInt16 volRefNum, Char *pathNameP,
                Char *newNameP) AXXPAC_SECTION;

extern Err InitializeAxxPac(void) AXXPAC_SECTION;
extern Boolean IsAxxPacPresent(void) AXXPAC_SECTION;
extern void TeardownAxxPac(void) AXXPAC_SECTION;

#define VFSVolumeEnumerate(volRefNumP, volIteratorP) \
            (IsAxxPacPresent() ? \
            vfsVolumeEnumerate(volRefNumP, volIteratorP) : \
            VFSVolumeEnumerate(volRefNumP, volIteratorP))

#define VFSVolumeInfo(volRefNum, volInfoP) \
            (IsAxxPacPresent() ? \
            vfsVolumeInfo(volRefNum, volInfoP) : \
            VFSVolumeInfo(volRefNum, volInfoP))

#define VFSFileOpen(volRefNum, pathNameP, openMode, fileRefP) \
            (IsAxxPacPresent() ? \
            vfsFileOpen(volRefNum, pathNameP, openMode, fileRefP) : \
            VFSFileOpen(volRefNum, pathNameP, openMode, fileRefP))

#define VFSDirEntryEnumerate(dirRef, dirEntryIteratorP, infoP) \
            (IsAxxPacPresent() ? \
            vfsDirEntryEnumerate(dirRef, dirEntryIteratorP, infoP) : \
            VFSDirEntryEnumerate(dirRef, dirEntryIteratorP, infoP))

#define VFSFileDBInfo(ref, nameP, attributesP, versionP, crDateP, \
                      modDateP, bckUpDateP, modNumP, appInfoHP, \
                      sortInfoHP, typeP, creatorP, numRecordsP) \
            (IsAxxPacPresent() ? \
            vfsFileDBInfo(ref, nameP, attributesP, versionP, crDateP, \
                          modDateP, bckUpDateP, modNumP, appInfoHP, \
                          sortInfoHP, typeP, creatorP, numRecordsP) : \
            VFSFileDBInfo(ref, nameP, attributesP, versionP, crDateP, \
                          modDateP, bckUpDateP, modNumP, appInfoHP, \
                          sortInfoHP, typeP, creatorP, numRecordsP))

#define VFSFileSize(fileRef, fileSizeP) \
            (IsAxxPacPresent() ? \
            vfsFileSize(fileRef, fileSizeP) : \
            VFSFileSize(fileRef, fileSizeP))

#define VFSFileGetAttributes(fileRef, attributesP) \
            (IsAxxPacPresent() ? \
            vfsFileGetAttributes(fileRef, attributesP) : \
            VFSFileGetAttributes(fileRef, attributesP))

#define VFSFileClose(fileRef) \
            (IsAxxPacPresent() ? \
            vfsFileClose(fileRef) : \
            VFSFileClose(fileRef))

#define VFSFileDBGetRecord(ref, recIndex, recHP, recAttrP, uniqueIDP) \
            (IsAxxPacPresent() ? \
            vfsFileDBGetRecord(ref, recIndex, recHP, recAttrP, uniqueIDP) : \
            VFSFileDBGetRecord(ref, recIndex, recHP, recAttrP, uniqueIDP))

#define VFSDirCreate(volRefNum, dirNameP) \
            (IsAxxPacPresent() ? \
            vfsDirCreate(volRefNum, dirNameP) : \
            VFSDirCreate(volRefNum, dirNameP))

#define VFSExportDatabaseToFile(volRefNum, pathNameP, cardNo, dbID) \
            (IsAxxPacPresent() ? \
            vfsExportDatabaseToFile(volRefNum, pathNameP, cardNo, dbID) : \
            VFSExportDatabaseToFile(volRefNum, pathNameP, cardNo, dbID))

#define VFSFileDelete(volRefNum, pathNameP) \
            (IsAxxPacPresent() ? \
            vfsFileDelete(volRefNum, pathNameP) : \
            VFSFileDelete(volRefNum, pathNameP))

#define VFSImportDatabaseFromFile(volRefNum, pathNameP, cardNoP, dbIDP) \
            (IsAxxPacPresent() ? \
            vfsImportDatabaseFromFile(volRefNum, pathNameP, cardNoP, dbIDP) : \
            VFSImportDatabaseFromFile(volRefNum, pathNameP, cardNoP, dbIDP))

#define VFSFileRead(fileRef, numBytes, bufP, numBytesReadP) \
            (IsAxxPacPresent() ? \
            vfsFileRead(fileRef, numBytes, bufP, numBytesReadP) : \
            VFSFileRead(fileRef, numBytes, bufP, numBytesReadP))

#define VFSFileEOF(fileRef) \
            (IsAxxPacPresent() ? \
            vfsFileEOF(fileRef) : \
            VFSFileEOF(fileRef))

#define VFSVolumeGetLabel(volRefNum, labelP, bufLen) \
            (IsAxxPacPresent() ? \
            vfsVolumeGetLabel(volRefNum, labelP, bufLen) : \
            VFSVolumeGetLabel(volRefNum, labelP, bufLen))

#define VFSFileWrite(fileRef, numBytes, dataP, numBytesWrittenP) \
            (IsAxxPacPresent() ? \
            vfsFileWrite(fileRef, numBytes, dataP, numBytesWrittenP) : \
            VFSFileWrite(fileRef, numBytes, dataP, numBytesWrittenP))

#define VFSFileRename(volRefNum, pathNameP, newNameP) \
            (IsAxxPacPresent() ? \
            vfsFileRename(volRefNum, pathNameP, newNameP) : \
            VFSFileRename(volRefNum, pathNameP, newNameP))

#endif /* HAVE_AXXPAC */

#endif
