/******************************************************************************
 *
 * Copyright (c) 2000-2003 PalmSource, Inc. All rights reserved.
 *
 * File: VFSMgr.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *		Header file for VFS Manager.
 *
 *****************************************************************************/

#ifndef __VFSMGR_H__
#define __VFSMGR_H__

#ifdef __cplusplus  
extern "C" { 
#endif

#include <PalmTypes.h>
#include <CoreTraps.h>
#include <SystemMgr.h>

#include "ExpansionMgr.h"

#ifdef BUILDING_AGAINST_PALMOS35
	#define sysTrapVFSMgr	sysTrapSysReserved3
	
	#define sysFileCVFSMgr		'vfsm'	// Creator type for VFSMgr...
	
	#define vfsErrorClass	0x2A00		// Post-3.5 this is defined in ErrorBase.h
#else
	#define sysTrapVFSMgr	sysTrapFileSystemDispatch
#endif



#ifdef PALMOS
#ifdef BUILDING_VFSMGR_DISPATCH
	#define VFSMGR_TRAP(VFSMgrSelectorNum)
#else
	#define VFSMGR_TRAP(sel) \
		_SYSTEM_API(_CALL_WITH_SELECTOR)(_SYSTEM_TABLE, sysTrapVFSMgr, sel)
#endif
#else
#define VFSMGR_TRAP(sel)
#endif




#define vfsFtrIDVersion		0	// ID of feature containing version of VFSMgr.
								// Check existence of this feature to see if VFSMgr is installed.

#define vfsFtrIDDefaultFS	1	// ID of feature containing the creator ID of the default filesystem library
								// this is the default choice when choosing a library for formatting/mounting

#define vfsMgrVersionNum	((UInt16)200)	// version of the VFSMgr, obtained from the feature


// MountClass constants:
#define vfsMountClass_SlotDriver		sysFileTSlotDriver
#define vfsMountClass_Simulator		sysFileTSimulator
#define vfsMountClass_POSE          'pose'



// Base MountParamType; others such as SlotMountParamType are extensions of this base type,
// switched on value of "mountClass" parameter.  It will make more sense someday when there
// are other kinds of FileSystems...  (Trust us.  :-)
typedef struct VFSAnyMountParamTag 
{
	UInt16 volRefNum;				// The volRefNum of the volume.
	UInt16 reserved;
	UInt32 mountClass;			// 'libs' for slotDriver-based filesystems
	
	// Other fields here, depending on value of 'mountClass'
	
} VFSAnyMountParamType;
typedef VFSAnyMountParamType *VFSAnyMountParamPtr ;


typedef struct VFSSlotMountParamTag 
{
	VFSAnyMountParamType	vfsMountParam;		// mountClass = VFSMountClass_SlotDriver = 'libs'
	UInt16				slotLibRefNum;
	UInt16				slotRefNum;
} VFSSlotMountParamType;

typedef struct VFSPOSEMountParamTag
{
	VFSAnyMountParamType vfsMountParam;    // mountClass = VFSMountClass_POSE = 'pose'
	UInt8                poseSlotNum;
} VFSPOSEMountParamType;


/* For Example...
typedef struct VFSOtherMountParamTag {
	VFSAnyMountParamType	vfsMountParam;		// mountClass = 'othr' (for example)
	UInt16				otherValue;
} VFSOtherMountParamType;
*/

typedef struct FileInfoTag
{
	UInt32	attributes;
	Char		*nameP;				// buffer to receive full name; pass NULL to avoid getting name
	UInt16	nameBufLen; 		// size of nameP buffer, in bytes
} FileInfoType, *FileInfoPtr;



typedef struct VolumeInfoTag
{
	UInt32	attributes;			// read-only etc.
	UInt32	fsType;				// Filesystem type for this volume (defined below)
	UInt32	fsCreator;			// Creator code of filesystem driver for this volume.  For use with VFSCustomControl().
	UInt32	mountClass;			// mount class that mounted this volume
	
	// For slot based filesystems: (mountClass = vfsMountClass_SlotDriver)
	UInt16	slotLibRefNum;		// Library on which the volume is mounted
	UInt16	slotRefNum;			// ExpMgr slot number of card containing volume
	UInt32	mediaType;			// Type of card media (mediaMemoryStick, mediaCompactFlash, etc...)
	UInt32	reserved;			// reserved for future use (other mountclasses may need more space)
} VolumeInfoType, *VolumeInfoPtr;


#ifdef PALMOS
typedef UInt32 FileRef;
#else
typedef void *FileRef;
#endif

#define	vfsInvalidVolRef		0		// constant for an invalid volume reference, guaranteed not to represent a valid one.  Use it like you would use NULL for a FILE*.
#define	vfsInvalidFileRef		0L		// constant for an invalid file reference, guaranteed not to represent a valid one.  Use it like you would use NULL for a FILE*.


/************************************************************
 * File Origin constants: (for the origins of relative offsets passed to 'seek' type routines).
 *************************************************************/
#define vfsOriginBeginning		0	// from the beginning (first data byte of file)
#define vfsOriginCurrent		1	// from the current position
#define vfsOriginEnd				2	// from the end of file (one position beyond last data byte, only negative offsets are legal)

typedef UInt16	FileOrigin;


/************************************************************
 * openMode flags passed to VFSFileOpen
 *************************************************************/
#define vfsModeExclusive			(0x0001U)		// don't let anyone else open it
#define vfsModeRead					(0x0002U)		// open for read access
#define vfsModeWrite					(0x0004U | vfsModeExclusive)		// open for write access, implies exclusive
#define vfsModeCreate				(0x0008U)		// create the file if it doesn't already exist.  Implemented in VFS layer, no FS lib call will ever have to handle this.
#define vfsModeTruncate				(0x0010U)		// Truncate file to 0 bytes after opening, removing all existing data.  Implemented in VFS layer, no FS lib call will ever have to handle this.
#define vfsModeReadWrite			(vfsModeWrite | vfsModeRead)		// open for read/write access
#define vfsModeLeaveOpen			(0x0020U)		// Leave the file open even if when the foreground task closes


		// Combination flag constants, for error checking purposes:
#define vfsModeAll					(vfsModeExclusive | vfsModeRead | vfsModeWrite | vfsModeCreate | vfsModeTruncate | vfsModeReadWrite | vfsModeLeaveOpen)
#define vfsModeVFSLayerOnly		(vfsModeCreate | vfsModeTruncate)		// flags only used apps & the VFS layer, FS libraries will never see these.


/************************************************************
 * File Attributes
 *************************************************************/
#define vfsFileAttrReadOnly		(0x00000001UL)
#define vfsFileAttrHidden			(0x00000002UL)
#define vfsFileAttrSystem			(0x00000004UL)
#define vfsFileAttrVolumeLabel	(0x00000008UL)
#define vfsFileAttrDirectory		(0x00000010UL)
#define vfsFileAttrArchive			(0x00000020UL)
#define vfsFileAttrLink				(0x00000040UL)

#define vfsFileAttrAll				(0x0000007fUL)


/************************************************************
 * Volume Attributes
 *************************************************************/
#define vfsVolumeAttrSlotBased	(0x00000001UL)		// reserved
#define vfsVolumeAttrReadOnly		(0x00000002UL)		// volume is read only
#define vfsVolumeAttrHidden		(0x00000004UL)		// volume should not be user-visible.

/************************************************************
 * Date constants (for use with VFSFileGet/SetDate)
 *************************************************************/
#define vfsFileDateCreated			1
#define vfsFileDateModified		2
#define vfsFileDateAccessed		3

/************************************************************
 * Iterator start and stop constants.
 * Used by VFSVolumeEnumerate, VFSDirEntryEnumerate, VFSDirEntryEnumerate
 *************************************************************/
#define vfsIteratorStart              0L
#define vfsIteratorStop               0xffffffffL


/************************************************************
 * 'handled' field bit constants 
 * (for use with Volume Mounted/Unmounted notifications)
 *************************************************************/
#define vfsHandledUIAppSwitch	0x01	// Any UI app switching has already been handled.  
												// The VFSMgr will not UIAppSwitch to the start.prc app 
												// (but it will loaded & sent the AutoStart launchcode), 
												// and the Launcher will not switch to itself.
#define vfsHandledStartPrc		0x02	// And automatic running of start.prc has already been handled.
												// VFSMgr will not load it, send it the AutoStart launchcode,
												// or UIAppSwitch to it.

/************************************************************
 * Format/Mount flags (for use with VFSVolumeFormat/Mount)
 *************************************************************/
#define vfsMountFlagsUseThisFileSystem		0x01	// Mount/Format the volume with the filesystem specified
//#define vfsMountFlagsPrivate1					0x02	// for system use only
//#define vfsMountFlagsPrivate2					0x04	// for system use only
#define vfsMountFlagsReserved1				0x08	// reserved
#define vfsMountFlagsReserved2				0x10	// reserved
#define vfsMountFlagsReserved3				0x20	// reserved
#define vfsMountFlagsReserved4				0x40	// reserved
#define vfsMountFlagsReserved5				0x80	// reserved


/************************************************************
 * Common filesystem types.  Used by FSFilesystemType and SlotCardIsFilesystemSupported.
 *************************************************************/
#define vfsFilesystemType_VFAT		'vfat'		// FAT12 and FAT16 extended to handle long file names
#define vfsFilesystemType_FAT			'fats'		// FAT12 and FAT16 which only handles 8.3 file names
#define vfsFilesystemType_NTFS		'ntfs'		// Windows NT filesystem
#define vfsFilesystemType_HFSPlus	'hfse'		// The Macintosh extended hierarchical filesystem
#define vfsFilesystemType_HFS			'hfss'		// The Macintosh standard hierarchical filesystem
#define vfsFilesystemType_MFS			'mfso'		// The Macintosh original filesystem
#define vfsFilesystemType_EXT2		'ext2'		// Linux filesystem
#define vfsFilesystemType_FFS			'ffsb'		// Unix Berkeley block based filesystem
#define vfsFilesystemType_NFS			'nfsu'		// Unix Networked filesystem
#define vfsFilesystemType_AFS			'afsu'		// Unix Andrew filesystem
#define vfsFilesystemType_Novell		'novl'		// Novell filesystem
#define vfsFilesystemType_HPFS		'hpfs'		// OS2 High Performance filesystem


/************************************************************
 * Error codes
 *************************************************************/
#define vfsErrBufferOverflow			(vfsErrorClass | 1)	// passed in buffer is too small
#define vfsErrFileGeneric				(vfsErrorClass | 2)	// Generic file error.
#define vfsErrFileBadRef				(vfsErrorClass | 3)	// the fileref is invalid (has been closed, or was not obtained from VFSFileOpen())
#define vfsErrFileStillOpen			(vfsErrorClass | 4)	// returned from FSFileDelete if the file is still open
#define vfsErrFilePermissionDenied	(vfsErrorClass | 5)	// The file is read only
#define vfsErrFileAlreadyExists		(vfsErrorClass | 6)	// a file of this name exists already in this location
#define vfsErrFileEOF					(vfsErrorClass | 7)	// file pointer is at end of file
#define vfsErrFileNotFound				(vfsErrorClass | 8)	// file was not found at the path specified
#define vfsErrVolumeBadRef				(vfsErrorClass | 9)	// the volume refnum is invalid.
#define vfsErrVolumeStillMounted		(vfsErrorClass | 10)	// returned from FSVolumeFormat if the volume is still mounted
#define vfsErrNoFileSystem				(vfsErrorClass | 11)	// no installed filesystem supports this operation
#define vfsErrBadData					(vfsErrorClass | 12)	// operation could not be completed because of invalid data (i.e., import DB from .PRC file)
#define vfsErrDirNotEmpty				(vfsErrorClass | 13)	// can't delete a non-empty directory
#define vfsErrBadName					(vfsErrorClass | 14)	// invalid filename, or path, or volume label or something...
#define vfsErrVolumeFull				(vfsErrorClass | 15)	// not enough space left on volume
#define vfsErrUnimplemented			(vfsErrorClass | 16)	// this call is not implemented
#define vfsErrNotADirectory			(vfsErrorClass | 17)	// This operation requires a directory
#define vfsErrIsADirectory          (vfsErrorClass | 18) // This operation requires a regular file, not a directory
#define vfsErrDirectoryNotFound		(vfsErrorClass | 19) // Returned from VFSFileCreate when the path leading up to the new file does not exist
#define vfsErrNameShortened			(vfsErrorClass | 20) // A volume name or filename was automatically shortened to conform to filesystem spec

/************************************************************
 * Selectors for routines found in the VFS manager. The order
 * of these selectors MUST match the jump table in VFSMgr.c.
 *************************************************************/
#define vfsTrapInit						0
#define vfsTrapCustomControl			1

#define vfsTrapFileCreate				2
#define vfsTrapFileOpen					3
#define vfsTrapFileClose				4
#define vfsTrapFileReadData			5
#define vfsTrapFileRead					6
#define vfsTrapFileWrite				7
#define vfsTrapFileDelete				8
#define vfsTrapFileRename				9
#define vfsTrapFileSeek					10
#define vfsTrapFileEOF					11
#define vfsTrapFileTell					12
#define vfsTrapFileResize				13
#define vfsTrapFileGetAttributes		14
#define vfsTrapFileSetAttributes		15
#define vfsTrapFileGetDate				16
#define vfsTrapFileSetDate				17
#define vfsTrapFileSize					18

#define vfsTrapDirCreate				19
#define vfsTrapDirEntryEnumerate		20
#define vfsTrapGetDefaultDirectory	21
#define vfsTrapRegisterDefaultDirectory	22
#define vfsTrapUnregisterDefaultDirectory	23

#define vfsTrapVolumeFormat			24
#define vfsTrapVolumeMount				25
#define vfsTrapVolumeUnmount			26
#define vfsTrapVolumeEnumerate		27
#define vfsTrapVolumeInfo				28
#define vfsTrapVolumeGetLabel			29
#define vfsTrapVolumeSetLabel			30
#define vfsTrapVolumeSize				31

#define vfsTrapInstallFSLib			32
#define vfsTrapRemoveFSLib				33
#define vfsTrapImportDatabaseFromFile	34
#define vfsTrapExportDatabaseToFile		35
#define vfsTrapFileDBGetResource		36
#define vfsTrapFileDBInfo				37
#define vfsTrapFileDBGetRecord		38

#define vfsTrapImportDatabaseFromFileCustom	39
#define vfsTrapExportDatabaseToFileCustom		40

// System use only
#define vfsTrapPrivate1					41

#define vfsMaxSelector					vfsTrapPrivate1


typedef Err	(*VFSImportProcPtr)
				(UInt32 totalBytes, UInt32 offset, void *userDataP);
typedef Err	(*VFSExportProcPtr)
				(UInt32 totalBytes, UInt32 offset, void *userDataP);


Err VFSInit(void)
		VFSMGR_TRAP(vfsTrapInit);

// if you pass NULL for fsCreator, VFS will iterate through 
// all installed filesystems until it finds one that does not return an error.
Err VFSCustomControl(UInt32 fsCreator, UInt32 apiCreator, UInt16 apiSelector, 
							void *valueP, UInt16 *valueLenP)
		VFSMGR_TRAP(vfsTrapCustomControl);

Err VFSFileCreate(UInt16 volRefNum, const Char *pathNameP)
		VFSMGR_TRAP(vfsTrapFileCreate);

Err VFSFileOpen(UInt16 volRefNum, const Char *pathNameP,
	UInt16 openMode, FileRef *fileRefP)
		VFSMGR_TRAP(vfsTrapFileOpen);

Err VFSFileClose(FileRef fileRef)
		VFSMGR_TRAP(vfsTrapFileClose);

Err VFSFileReadData(FileRef fileRef, UInt32 numBytes, void *bufBaseP, 
						UInt32 offset, UInt32 *numBytesReadP)
		VFSMGR_TRAP(vfsTrapFileReadData);

Err VFSFileRead(FileRef fileRef, UInt32 numBytes, void *bufP, UInt32 *numBytesReadP)
		VFSMGR_TRAP(vfsTrapFileRead);

Err VFSFileWrite(FileRef fileRef, UInt32 numBytes, const void *dataP, UInt32 *numBytesWrittenP)
		VFSMGR_TRAP(vfsTrapFileWrite);

// some file routines work on directories
Err VFSFileDelete(UInt16 volRefNum, const Char *pathNameP)
		VFSMGR_TRAP(vfsTrapFileDelete);

Err VFSFileRename(UInt16 volRefNum, const Char *pathNameP, const Char *newNameP)
		VFSMGR_TRAP(vfsTrapFileRename);

Err VFSFileSeek(FileRef fileRef, FileOrigin origin, Int32 offset)
		VFSMGR_TRAP(vfsTrapFileSeek);

Err VFSFileEOF(FileRef fileRef)
		VFSMGR_TRAP(vfsTrapFileEOF);

Err VFSFileTell(FileRef fileRef, UInt32 *filePosP)
		VFSMGR_TRAP(vfsTrapFileTell);

Err VFSFileSize(FileRef fileRef, UInt32 *fileSizeP)
		VFSMGR_TRAP(vfsTrapFileSize);

Err VFSFileResize(FileRef fileRef, UInt32 newSize)
		VFSMGR_TRAP(vfsTrapFileResize);

Err VFSFileGetAttributes(FileRef fileRef, UInt32 *attributesP)
		VFSMGR_TRAP(vfsTrapFileGetAttributes);

Err VFSFileSetAttributes(FileRef fileRef, UInt32 attributes)
		VFSMGR_TRAP(vfsTrapFileSetAttributes);

Err VFSFileGetDate(FileRef fileRef, UInt16 whichDate, UInt32 *dateP)
		VFSMGR_TRAP(vfsTrapFileGetDate);

Err VFSFileSetDate(FileRef fileRef, UInt16 whichDate, UInt32 date)
		VFSMGR_TRAP(vfsTrapFileSetDate);


Err VFSDirCreate(UInt16 volRefNum, const Char *dirNameP)
		VFSMGR_TRAP(vfsTrapDirCreate);

Err VFSDirEntryEnumerate(FileRef dirRef, UInt32 *dirEntryIteratorP, FileInfoType *infoP)
		VFSMGR_TRAP(vfsTrapDirEntryEnumerate);


Err VFSGetDefaultDirectory(UInt16 volRefNum, const Char *fileTypeStr,
			Char *pathStr, UInt16 *bufLenP)
		VFSMGR_TRAP(vfsTrapGetDefaultDirectory);

Err VFSRegisterDefaultDirectory(const Char *fileTypeStr, UInt32 mediaType, 
			const Char *pathStr)
		VFSMGR_TRAP(vfsTrapRegisterDefaultDirectory);

Err VFSUnregisterDefaultDirectory(const Char *fileTypeStr, UInt32 mediaType)
		VFSMGR_TRAP(vfsTrapUnregisterDefaultDirectory);



Err VFSVolumeFormat(UInt8 flags, UInt16 fsLibRefNum, VFSAnyMountParamPtr vfsMountParamP)
		VFSMGR_TRAP(vfsTrapVolumeFormat);

Err VFSVolumeMount(UInt8 flags, UInt16 fsLibRefNum, VFSAnyMountParamPtr vfsMountParamP)
		VFSMGR_TRAP(vfsTrapVolumeMount);

Err VFSVolumeUnmount(UInt16 volRefNum)
		VFSMGR_TRAP(vfsTrapVolumeUnmount);

Err VFSVolumeEnumerate(UInt16 *volRefNumP, UInt32 *volIteratorP)
		VFSMGR_TRAP(vfsTrapVolumeEnumerate);

Err VFSVolumeInfo(UInt16 volRefNum, VolumeInfoType *volInfoP)
		VFSMGR_TRAP(vfsTrapVolumeInfo);

Err VFSVolumeGetLabel(UInt16 volRefNum, Char *labelP, UInt16 bufLen)
		VFSMGR_TRAP(vfsTrapVolumeGetLabel);

Err VFSVolumeSetLabel(UInt16 volRefNum, const Char *labelP)
		VFSMGR_TRAP(vfsTrapVolumeSetLabel);

Err VFSVolumeSize(UInt16 volRefNum, UInt32 *volumeUsedP, UInt32 *volumeTotalP)
		VFSMGR_TRAP(vfsTrapVolumeSize);

Err VFSInstallFSLib(UInt32 creator, UInt16 *fsLibRefNumP)
		VFSMGR_TRAP(vfsTrapInstallFSLib);

Err VFSRemoveFSLib(UInt16 fsLibRefNum)
		VFSMGR_TRAP(vfsTrapRemoveFSLib);

Err VFSImportDatabaseFromFile(UInt16 volRefNum, const Char *pathNameP, 
							UInt16 *cardNoP, LocalID *dbIDP)
		VFSMGR_TRAP(vfsTrapImportDatabaseFromFile);

Err VFSImportDatabaseFromFileCustom(UInt16 volRefNum, const Char *pathNameP, 
							UInt16 *cardNoP, LocalID *dbIDP, VFSImportProcPtr importProcP,
							void *userDataP)
		VFSMGR_TRAP(vfsTrapImportDatabaseFromFileCustom);

Err VFSExportDatabaseToFile(UInt16 volRefNum, const Char *pathNameP, 
							UInt16 cardNo, LocalID dbID)
		VFSMGR_TRAP(vfsTrapExportDatabaseToFile);

Err VFSExportDatabaseToFileCustom(UInt16 volRefNum, const Char *pathNameP, 
							UInt16 cardNo, LocalID dbID, VFSExportProcPtr exportProcP,
							void *userDataP)
		VFSMGR_TRAP(vfsTrapExportDatabaseToFileCustom);

Err VFSFileDBGetResource(FileRef ref, DmResType type, DmResID resID, MemHandle *resHP)
		VFSMGR_TRAP(vfsTrapFileDBGetResource);

Err VFSFileDBInfo(FileRef ref, Char *nameP,
					UInt16 *attributesP, UInt16 *versionP, UInt32 *crDateP,
					UInt32 *modDateP, UInt32 *bckUpDateP,
					UInt32 *modNumP, MemHandle *appInfoHP,
					MemHandle *sortInfoHP, UInt32 *typeP,
					UInt32 *creatorP, UInt16 *numRecordsP)
		VFSMGR_TRAP(vfsTrapFileDBInfo);

Err VFSFileDBGetRecord(FileRef ref, UInt16 recIndex, MemHandle *recHP, 
									UInt8 *recAttrP, UInt32 *uniqueIDP)
		VFSMGR_TRAP(vfsTrapFileDBGetRecord);

char *VFSFileGets(FileRef fileRef, UInt32 numBytes, char *bufP);

Int32 VFSFilePrintF(FileRef fileRef, const char *format, ...);

Err VFSGetAttributes(UInt16 volRefNum, const Char *pathNameP, UInt32 *attributesP);

#ifdef __cplusplus
}
#endif

#endif	// __VFSMGR_H__
