/*
 * $Id: pi-dlp.h,v 1.76 2006/10/17 13:24:06 desrod Exp $
 *
 * pi-dlp.h: Desktop Link Protocol implementation (ala SLP)
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

/** @file pi-dlp.h
 *  @brief Direct protocol interface to the device using the HotSync protocol.
 *
 * The DLP layer is the lowest interface layer applications can use to
 * access a handheld.  It provides equivalents to Palm Conduit Development
 * Kit (CDK)'s SyncXXX functions, as well as a number of convenience
 * functions that are not found in the CDK.
 *
 * Once you have a socket number and a device is connected, you can start
 * using DLP calls to talk with the device. All DLP calls are @b
 * synchronous: they are immediately sent to the device and the current
 * thread is blocked until either a response is received, or an error
 * occurs.
 *
 * It is a good pratice to always check errors returned by DLP calls. 
 * Usually, if the value is nagative, it is an error code. If the error is
 * #PI_ERR_DLP_PALMOS, an error code was returned by the device itself: you
 * can get this error code by calling pi_palmos_error() on the current
 * socket. Besides all the Palm OS error code defined in Palm's
 * documentation, there are a few values between #dlpErrNoError and
 * #dlpErrUnknown which are error returned by the DLP layer itself on the
 * device.
 *
 * The DLP protocol is the low level protocol that HotSync uses. Over the
 * years, there have been several iterations of DLP. Pre-Palm OS 5 devices
 * have DLP 1.2 or lower. Palm OS 5 devices have DLP 1.3 or 1.4 (Palm OS 5.2
 * and up). Cobalt (Palm OS 6) uses DLP 2.1.
 *
 * Devices with DLP 1.4 and later are known to support transfers of large
 * records and resources (of size bigger than 64k). This is the case of the
 * Tapwave Zodiac, for example.
 *
 * Note that some devices report an incorrect version of DLP. Some Palm OS 5
 * devices report using DLP 1.2 whereas they really support DLP 1.3.
 *
 * Depending on which devices you plan on being compatible with, you should adjust
 * #PI_DLP_VERSION_MAJOR and #PI_DLP_VERSION_MINOR. If you want to support
 * devices up to and including Palm OS 5, setting your DLP version to 1.4 is
 * a good idea. If you want to be able to connect to Palm OS 6, you need to
 * set your DLP version to 2.1.
 */

#ifndef _PILOT_DLP_H_
#define _PILOT_DLP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <unistd.h>

#include "pi-macros.h"		/* For recordid_t */
#include "pi-buffer.h"		/* For pi_buffer_t */
#include "pi-error.h"		/* For PI_ERR */

/* version of the DLP protocol supported in this version */
/* Hint for existing versions:
 * 1.2: Palm OS 4 / Palm OS 5 (OS 5 should be 1.3 but incorrectly reports 1.2)
 * 1.4: TapWave Palm OS 5
 * 2.1: Palm OS 6
 */
#define PI_DLP_VERSION_MAJOR 1		/**< Major DLP protocol version we report to the device. */
#define PI_DLP_VERSION_MINOR 4		/**< Minor DLP protocol version we report to the device. */

#ifndef SWIG
	#define DLP_BUF_SIZE 0xffff	/**< Kept for compatibility, applications should avoid using this value. */
#endif /* !SWIG */

/** @name Internal definitions used to assemble DLP calls */
/*@{*/
#ifndef SWIG
	#define PI_DLP_OFFSET_CMD  0
	#define PI_DLP_OFFSET_ARGC 1
	#define PI_DLP_OFFSET_ARGV 2

	#define PI_DLP_ARG_TINY_LEN  0x000000FFL
	#define PI_DLP_ARG_SHORT_LEN 0x0000FFFFL
	#define PI_DLP_ARG_LONG_LEN  0xFFFFFFFFL

	#define PI_DLP_ARG_FLAG_TINY  0x00
	#define PI_DLP_ARG_FLAG_SHORT 0x80
	#define PI_DLP_ARG_FLAG_LONG  0x40
	#define PI_DLP_ARG_FLAG_MASK  0xC0

	#define PI_DLP_ARG_FIRST_ID 0x20
#endif /* !SWIG */
/*@}*/

/** @name VFS definitions */
/*@{*/
#define vfsMountFlagsUseThisFileSystem	0x01	/**< Mount/Format the volume with the filesystem specified */
#define vfsMAXFILENAME			256	/**< The maximum size of a filename in a VFS volume */
#define vfsInvalidVolRef		0	/**< constant for an invalid volume reference, guaranteed not to represent a valid one.  Use it like you would use NULL for a FILE*. */
#define vfsInvalidFileRef		0L	/**< constant for an invalid file reference, guaranteed not to represent a valid one.  Use it like you would use NULL for a FILE*. */
/*@}*/

typedef uint32_t FileRef;			/**< Type for file references when working with VFS files and directories. */

/** @brief Information retrieved by dlp_VFSDirEntryEnumerate() */
struct VFSDirInfo {
	uint32_t attr;			/**< File or directory attributes (see VSF File attribute definitions) */
	char name[vfsMAXFILENAME];		/**< File or directory name */
};

/** @brief Information used to format a volume with dlp_VFSVolumeFormat() */
struct VFSAnyMountParam {
	unsigned short volRefNum;
	unsigned short reserved;
	uint32_t  mountClass;
};

/** @brief Information used to format a volume with dlp_VFSVolumeFormat() */
struct VFSSlotMountParam {
	struct VFSAnyMountParam vfsMountParam;
	unsigned short slotLibRefNum;
	unsigned short slotRefNum;
};

/** @brief Information about a VFS volume, returned by dlp_VFSVolumeInfo() */
struct VFSInfo {
	/* 0: read-only etc. */
	uint32_t   attributes;		/**< Volume attributes (see #dlpVFSVolumeAttributes enum) */

	/* 4: Filesystem type for this volume (defined below).
	      These you can expect to see in devices:
		  'vfat' (FAT12/FAT16 with long name support)
		  
	      Other values observed:
		  'twmf' (Tapwave Zodiac internal VFS)
		  
	      PalmSource defines these, but don't bet on device support:
		  'afsu' (Andrew network filesystem)
		  'ext2' (Linux ext2 filesystem)
		  'fats' (FAT12/FAT16 with 8.3 names)
		  'ffsb' (BSD block-based filesystem)
		  'hfse' (Macintosh HFS+)
		  'hfss' (Macintosh HFS, pre-8.x)
		  'hpfs' (OS/2 High Performance Filesystem)
		  'mfso' (Original Macintosh filesystem)
		  'nfsu' (NFS mount)
		  'novl' (Novell filesystem)
		  'ntfs' (Windows NT filesystem)
	*/
	uint32_t   fsType;			/**< File system time (four-char code, see above) */

	/* 8: Creator code of filesystem driver for this volume. */
	uint32_t   fsCreator;		/**< File system creator (four-char code) */

	/* For slot based filesystems: (mountClass = VFSMountClass_SlotDriver)
	   12: mount class that mounted this volume */
	uint32_t   mountClass;		/**< Mount class */

	/* 16: Library on which the volume is mounted */
	int slotLibRefNum;			/**< Slot library reference number */

	/* 18: ExpMgr slot number of card containing volume */
	int slotRefNum;				/**< Expansion manager slot number */

	/* 20: Type of card media (mediaMemoryStick, mediaCompactFlash, etc.)
	       These you can expect to see in devices:
		   'cfsh' (CompactFlash)
		   'mmcd' (MultiMedia Card)
		   'mstk' (Memory Stick)
		   'sdig' (SD card)

	       Other values observed:
		   'TFFS' (palmOne Tungsten T5 internal VFS)
		   'twMF' (Tapwave Zodiac internal VFS)

	       PalmSource also defines these:
		   'pose' (Host filesystem emulated by POSE)
		   'PSim' (Host filesystem emulated by Mac Simulator)
		   'ramd' (RAM disk)
		   'smed' (SmartMedia)
	*/
	uint32_t   mediaType;		/**< Media type (see above) */

	/* 24: reserved for future use (other mountclasses may need more space) */
	uint32_t   reserved;		/**< Reserved, set to 0 */
};

/** @brief Information about the handheld user
 *
 * This structure is used in dlp_ReadUserInfo() and dlp_WriteUserInfo()
 */
struct PilotUser {
	size_t 	passwordLength;
	char 	username[128];
	char	password[128];
	uint32_t userID;
	uint32_t viewerID;
	uint32_t lastSyncPC;
	time_t successfulSyncDate;
	time_t lastSyncDate;
};

/** @brief Device information.
 *
 * This structure is filled by dlp_ReadSysInfo()
 */
struct SysInfo {
	uint32_t romVersion;		/**< Version of the device ROM, of the form 0xMMmmffssbb where MM=Major, mm=minor, ff=fix, ss=stage, bb=build */
	uint32_t locale;			/**< Locale for this device */
	unsigned char prodIDLength;		/**< Length of the prodID string */
	char 	prodID[128];			/**< Product ID */
	unsigned short dlpMajorVersion;		/**< Major version of the DLP protocol on this device */
	unsigned short dlpMinorVersion;		/**< Minor version of the DLP protocol on this device */
	unsigned short compatMajorVersion;	/**< Minimum major version of DLP this device is compatible with */
	unsigned short compatMinorVersion;	/**< Minimum minor version of DLP this device is compatible with */
	uint32_t  maxRecSize;		/**< Maximum record size. Usually <=0xFFFF or ==0 for older devices (means records are limited to 64k), can be much larger for devices with DLP >= 1.4 (i.e. 0x00FFFFFE) */
};

/** @brief Database information.
 *
 * A database information block is returned by dlp_ReadDBList(), dlp_FindDBInfo(), dlp_FindDBByName(), dlp_FindDBByOpenHandle()
 * and dlp_FindDBByTypeCreator().
 */
struct DBInfo {
	int 	more;				/**< When reading database list using dlp_ReadDBList(), this flag is set if there are more databases to come */
	char name[34];				/**< Database name, 32 characters max. */
	unsigned int flags;			/**< Database flags (@see dlpDBFlags enum) */
	unsigned int miscFlags;			/**< Additional database flags filled by pilot-link (@see dlpDBMiscFlags enum) */
	unsigned int version;			/**< Database version number */
	uint32_t type;			/**< Database type (four-char code, i.e. 'appl') */
	uint32_t creator;			/**< Database creator (four-char code, i.e. 'DATA') */
	uint32_t modnum;			/**< Modification count */
	unsigned int index;			/**< Database index in database list */
	time_t createDate;			/**< Database creation date (using the machine's local time zone) */
	time_t modifyDate;			/**< Last time this database was modified (using the machine's local time zone). If the database was never modified, this field is set to 0x83DAC000 (Fri Jan  1 00:00:00 1904 GMT) */
	time_t backupDate;			/**< Last time this database was backed up using HotSync. If the database was never backed up, this field is set to 0x83DAC000 (Fri Jan  1 00:00:00 1904 GMT) */
};

/** @brief Size information for a database.
 *
 * Returned by dlp_FindDBByName(), dlp_FindDBByOpenHandle() and dlp_FindDBByTypeCreator().
 */ 
struct DBSizeInfo {
	uint32_t numRecords;		/**< Number of records or resources */
	uint32_t totalBytes;		/**< Total number of bytes occupied by the database, including header and records list */
	uint32_t dataBytes;		/**< Total number of data bytes contained in the database's records or resources */
	uint32_t appBlockSize;		/**< Size of the appInfo block */
	uint32_t sortBlockSize;		/**< Size of the sortInfo block */
	uint32_t maxRecSize;		/**< note: this field is always set to 0 on return from dlp_FindDBxxx */
};

/** @brief Information about a memory card.
 *
 * This structure describes a device's internal storage only, not removable media.
 * It is returned by dlp_ReadStorageInfo().
 */
struct CardInfo {
	int 	card;				/**< Memory card index (most devices only have one). */
	int	version;			/**< Version of the card */
	int	more;				/**< Set if there is another card after this one */
	time_t 	creation;			/**< Creation date (using the computer's local time zone) */
	uint32_t romSize;			/**< Size of the ROM block on this card (in bytes) */
	uint32_t ramSize;			/**< Size of the RAM block on this card (in bytes) */
	uint32_t ramFree;			/**< Total free RAM bytes */
	char 	name[128];			/**< Card name */
	char	manufacturer[128];		/**< Card manufacturer name */
};

/** @brief Network HotSync information.
 *
 * Returned by dlp_ReadNetSyncInfo(). Gives the network location of a remote handheld.
 */
struct NetSyncInfo {
	int 	lanSync;			/**< Non-zero if LanSync is turned on on the device */
	char 	hostName[256];			/**< Device hostname if any. Null terminated string. */
	char 	hostAddress[40];		/**< Device host address. Null terminated string. */
	char 	hostSubnetMask[40];		/**< Device subnet mask. Null terminated string */
};

#ifndef SWIG	/* no need to clutter the bindings with this */
enum dlpFunctions {
	/* range reserved for internal use */
	dlpReservedFunc = 0x0F,

	/* DLP 1.0 FUNCTIONS START HERE (PalmOS v1.0) */
	dlpFuncReadUserInfo,			/* 0x10 */
	dlpFuncWriteUserInfo,			/* 0x11 */
	dlpFuncReadSysInfo,			/* 0x12 */
	dlpFuncGetSysDateTime,			/* 0x13 */
	dlpFuncSetSysDateTime,			/* 0x14 */
	dlpFuncReadStorageInfo,			/* 0x15 */
	dlpFuncReadDBList,			/* 0x16 */
	dlpFuncOpenDB,				/* 0x17 */
	dlpFuncCreateDB,			/* 0x18 */
	dlpFuncCloseDB,				/* 0x19 */
	dlpFuncDeleteDB,			/* 0x1a */
	dlpFuncReadAppBlock,			/* 0x1b */
	dlpFuncWriteAppBlock,			/* 0x1c */
	dlpFuncReadSortBlock,			/* 0x1d */
	dlpFuncWriteSortBlock,			/* 0x1e */
	dlpFuncReadNextModifiedRec,		/* 0x1f */
	dlpFuncReadRecord,			/* 0x20 */
	dlpFuncWriteRecord,			/* 0x21 */
	dlpFuncDeleteRecord,			/* 0x22 */
	dlpFuncReadResource,			/* 0x23 */
	dlpFuncWriteResource,			/* 0x24 */
	dlpFuncDeleteResource,			/* 0x25 */
	dlpFuncCleanUpDatabase,			/* 0x26 */
	dlpFuncResetSyncFlags,			/* 0x27 */
	dlpFuncCallApplication,			/* 0x28 */
	dlpFuncResetSystem,			/* 0x29 */
	dlpFuncAddSyncLogEntry,			/* 0x2a */
	dlpFuncReadOpenDBInfo,			/* 0x2b */
	dlpFuncMoveCategory,			/* 0x2c */
	dlpProcessRPC,				/* 0x2d */
	dlpFuncOpenConduit,			/* 0x2e */
	dlpFuncEndOfSync,			/* 0x2f */
	dlpFuncResetRecordIndex,		/* 0x30 */
	dlpFuncReadRecordIDList,		/* 0x31 */

	/* DLP 1.1 FUNCTIONS ADDED HERE (PalmOS v2.0 Personal, and Professional) */
	dlpFuncReadNextRecInCategory,   	/* 0x32 */
	dlpFuncReadNextModifiedRecInCategory,   /* 0x33 */
	dlpFuncReadAppPreference,		/* 0x34 */
	dlpFuncWriteAppPreference,		/* 0x35 */
	dlpFuncReadNetSyncInfo,			/* 0x36 */
	dlpFuncWriteNetSyncInfo,		/* 0x37 */
	dlpFuncReadFeature,			/* 0x38 */

	/* DLP 1.2 FUNCTIONS ADDED HERE (PalmOS v3.0) */
	dlpFuncFindDB,				/* 0x39 */
	dlpFuncSetDBInfo,			/* 0x3a */

	/* DLP 1.3 FUNCTIONS ADDED HERE (PalmOS v4.0) */
	dlpLoopBackTest,			/* 0x3b */
	dlpFuncExpSlotEnumerate,		/* 0x3c */
	dlpFuncExpCardPresent,			/* 0x3d */
	dlpFuncExpCardInfo,			/* 0x3e */
	dlpFuncVFSCustomControl,		/* 0x3f */
	dlpFuncVFSGetDefaultDir,		/* 0x40 */
	dlpFuncVFSImportDatabaseFromFile,	/* 0x41 */
	dlpFuncVFSExportDatabaseToFile, 	/* 0x42 */
	dlpFuncVFSFileCreate,			/* 0x43 */
	dlpFuncVFSFileOpen,			/* 0x44 */
	dlpFuncVFSFileClose,			/* 0x45 */
	dlpFuncVFSFileWrite,			/* 0x46 */
	dlpFuncVFSFileRead,			/* 0x47 */
	dlpFuncVFSFileDelete,			/* 0x48 */
	dlpFuncVFSFileRename,			/* 0x49 */
	dlpFuncVFSFileEOF,			/* 0x4a */
	dlpFuncVFSFileTell,			/* 0x4b */
	dlpFuncVFSFileGetAttributes,		/* 0x4c */
	dlpFuncVFSFileSetAttributes,		/* 0x4d */
	dlpFuncVFSFileGetDate,			/* 0x4e */
	dlpFuncVFSFileSetDate,			/* 0x4f */
	dlpFuncVFSDirCreate,			/* 0x50 */
	dlpFuncVFSDirEntryEnumerate,		/* 0x51 */
	dlpFuncVFSGetFile,			/* 0x52 */
	dlpFuncVFSPutFile,			/* 0x53 */
	dlpFuncVFSVolumeFormat,			/* 0x54 */
	dlpFuncVFSVolumeEnumerate,		/* 0x55 */
	dlpFuncVFSVolumeInfo,			/* 0x56 */
	dlpFuncVFSVolumeGetLabel,		/* 0x57 */
	dlpFuncVFSVolumeSetLabel,		/* 0x58 */
	dlpFuncVFSVolumeSize,			/* 0x59 */
	dlpFuncVFSFileSeek,			/* 0x5a */
	dlpFuncVFSFileResize,			/* 0x5b */
	dlpFuncVFSFileSize,			/* 0x5c */

	/* DLP 1.4 functions added here (Palm OS 5.2+, ie Tapwave Zodiac) */
	dlpFuncExpSlotMediaType,		/* 0x5d */
	dlpFuncWriteRecordEx,			/* 0x5e - function to write >64k records in Tapwave */
	dlpFuncWriteResourceEx,			/* 0x5f - function to write >64k resources in Tapwave */
	dlpFuncReadRecordEx,			/* 0x60 - function to read >64k records by index in Tapwave */
	dlpFuncUnknown1,			/* 0x61 (may be bogus definition in tapwave headers, is listed as dlpFuncReadRecordStream)*/
	dlpFuncUnknown3,			/* 0x62 */
	dlpFuncUnknown4,			/* 0x63 */
	dlpFuncReadResourceEx,			/* 0x64 - function to read resources >64k by index in Tapwave */
	dlpLastFunc
};

#endif	/* !SWIG */

/** @name Database and record attributes */
/*@{*/
	/** @brief Database flags in DBInfo structure and also for dlp_CreateDB() */
	enum dlpDBFlags {
		dlpDBFlagResource 	= 0x0001,	/**< Resource database */
		dlpDBFlagReadOnly 	= 0x0002,	/**< Database is read only */
		dlpDBFlagAppInfoDirty 	= 0x0004,	/**< AppInfo data has been modified */
		dlpDBFlagBackup 	= 0x0008,	/**< Database should be backed up during HotSync */
		dlpDBFlagHidden		= 0x0100,	/**< Database is hidden */
		dlpDBFlagLaunchable	= 0x0200,	/**< Database is launchable data (show in Launcher, launch app by Creator) */
		dlpDBFlagRecyclable	= 0x0400,	/**< Database will be deleted shortly */
		dlpDBFlagBundle		= 0x0800,	/**< Database is bundled with others having same creator (i.e. for Beam) */
		dlpDBFlagOpen 		= 0x8000,	/**< Database is currently open */

		/* v2.0 specific */
		dlpDBFlagNewer 		= 0x0010,	/**< Newer version may be installed over open DB (Palm OS 2.0 and later) */
		dlpDBFlagReset 		= 0x0020,	/**< Reset after installation (Palm OS 2.0 and later) */

		/* v3.0 specific */
		dlpDBFlagCopyPrevention = 0x0040,	/**< Database should not be beamed or sent (Palm OS 3.0 and later) */
		dlpDBFlagStream 	= 0x0080,	/**< Database is a file stream (Palm OS 3.0 and later) */

		/* OS 6+ */
		dlpDBFlagSchema		= 0x1000,	/**< Schema database (Palm OS 6.0 and later) */
		dlpDBFlagSecure		= 0x2000,	/**< Secure database (Palm OS 6.0 and later) */
		dlpDBFlagExtended	= dlpDBFlagSecure, /**< Set if Schema not set and DB is Extended (Palm OS 6.0 and later) */
		dlpDBFlagFixedUp	= 0x4000	/**< Temp flag used to clear DB on write (Palm OS 6.0 and later) */
	};

	/** @brief Misc. flags in DBInfo structure */
	enum dlpDBMiscFlags {
		dlpDBMiscFlagExcludeFromSync = 0x80,	/**< DLP 1.1 and later: exclude this database from sync */
		dlpDBMiscFlagRamBased 	= 0x40		/**< DLP 1.2 and later: this database is in RAM */
	};

	/** @brief Database record attributes */
	enum dlpRecAttributes {
		dlpRecAttrDeleted 	= 0x80,		/**< Tagged for deletion during next sync */
		dlpRecAttrDirty 	= 0x40,		/**< Record modified */
		dlpRecAttrBusy 		= 0x20,		/**< Record locked (in use) */
		dlpRecAttrSecret 	= 0x10,		/**< Record is secret */
		dlpRecAttrArchived 	= 0x08		/**< Tagged for archival during next sync */
	};

	/** @brief Mode flags used in dlp_OpenDB() */
	enum dlpOpenFlags {
		dlpOpenRead 		= 0x80,		/**< Open database for reading */
		dlpOpenWrite 		= 0x40,		/**< Open database for writing */
		dlpOpenExclusive 	= 0x20,		/**< Open database with exclusive access */
		dlpOpenSecret 		= 0x10,		/**< Show secret records */
		dlpOpenReadWrite 	= 0xC0		/**< Open database for reading and writing (equivalent to (#dlpOpenRead | #dlpOpenWrite)) */
	};

	/** @brief Flags passed to dlp_ReadDBList() */
	enum dlpDBList {
		dlpDBListRAM 		= 0x80,		/**< List RAM databases */
		dlpDBListROM 		= 0x40,		/**< List ROM databases */
		dlpDBListMultiple	= 0x20		/**< DLP 1.2 and above: list as many databases as possible at once */
	};

	enum dlpFindDBOptFlags {
		dlpFindDBOptFlagGetAttributes	= 0x80,
		dlpFindDBOptFlagGetSize		= 0x40,
		dlpFindDBOptFlagMaxRecSize	= 0x20
	};

	enum dlpFindDBSrchFlags {
		dlpFindDBSrchFlagNewSearch	= 0x80,
		dlpFindDBSrchFlagOnlyLatest	= 0x40
	};

/*@}*/

/** @brief End status values for dlp_EndOfSync() */
enum dlpEndStatus {
	dlpEndCodeNormal 	= 0,		/**< Normal termination */
	dlpEndCodeOutOfMemory,			/**< End due to low memory on device */
	dlpEndCodeUserCan,			/**< Cancelled by user */
	dlpEndCodeOther				/**< dlpEndCodeOther and higher == "Anything else" */
};

/** @name Expansion manager and VFS manager constants */
/*@{*/
	/** @brief Expansion card capabilities, as returned by dlp_ExpCardInfo() */
	enum dlpExpCardCapabilities {
		dlpExpCapabilityHasStorage	= 0x00000001,	/**< Card supports reading (and maybe writing) */
		dlpExpCapabilityReadOnly	= 0x00000002,	/**< Card is read-only */
		dlpExpCapabilitySerial		= 0x00000004	/**< Card supports dumb serial interface */
	};

	/** @brief VFS volume attributes as found in the @a attributes member of a VFSInfo structure */
	enum dlpVFSVolumeAttributes {
		vfsVolAttrSlotBased	= 0x00000001,	/**< Volume is inserted is an expansion slot */
		vfsVolAttrReadOnly	= 0x00000002,	/**< Volume is read-only */
		vfsVolAttrHidden	= 0x00000004	/**< Volume is hidden */
	};

	/** @brief Constants for dlp_VFSFileSeek() */
	enum dlpVFSSeekConstants {
		vfsOriginBeginning	= 0,		/**< From the beginning (first data byte of file) */
		vfsOriginCurrent	= 1,		/**< from the current position */
		vfsOriginEnd		= 2		/**< From the end of file (one position beyond last data byte, only negative offsets are legally allowed) */
	};

	/** @brief Flags for dlp_VFSFileOpen() */
	enum dlpVFSOpenFlags {
		dlpVFSOpenExclusive 	= 0x01,		/**< For dlp_VFSFileOpen(). Exclusive access */
		dlpVFSOpenRead 		= 0x02,		/**< For dlp_VFSFileOpen(). Read only */
		dlpVFSOpenWrite 	= 0x05, 	/**< For dlp_VFSFileOpen(). Write only. Implies exclusive */
		dlpVFSOpenReadWrite 	= 0x07,		/**< For dlp_VFSFileOpen(). Read | write */

		/* Remainder are aliases and special cases not for VFSFileOpen */
		vfsModeExclusive 	= dlpVFSOpenExclusive,	/**< Alias to #dlpVFSOpenExclusive */
		vfsModeRead 		= dlpVFSOpenRead,	/**< Alias to #dlpVFSOpenRead */
		vfsModeWrite 		= dlpVFSOpenWrite,	/**< Alias to #dlpVFSOpenWrite */
		vfsModeReadWrite	= vfsModeRead | vfsModeWrite,	/**< Alias to #dlpVFSOpenReadWrite */
		vfsModeCreate 		= 0x08 		/**< Not for dlp_VFSFileOpen(). Create file if it doesn't exist. */,
		vfsModeTruncate 	= 0x10 		/**< Not for dlp_VFSFileOpen(). Truncate to 0 bytes on open. */,
		vfsModeLeaveOpen 	= 0x20 		/**< Not for dlp_VFSFileOpen(). Leave file open even if foreground task closes. */
	} ;

	/** @brief VFS file attribute constants */
	enum dlpVFSFileAttributeConstants {
		vfsFileAttrReadOnly	= 0x00000001,	/**< File is read only */
		vfsFileAttrHidden	= 0x00000002,	/**< File is hidden */
		vfsFileAttrSystem	= 0x00000004,	/**< File is a system file */
		vfsFileAttrVolumeLabel	= 0x00000008,	/**< File is the volume label */
		vfsFileAttrDirectory	= 0x00000010,	/**< File is a directory */
		vfsFileAttrArchive	= 0x00000020,	/**< File is archived */
		vfsFileAttrLink		= 0x00000040	/**< File is a link to another file */
	};

	/** @brief Constants for dlp_VFSFileGetDate() and dlp_VFSFileSetDate() */
	enum dlpVFSDateConstants {
		vfsFileDateCreated	= 1,		/**< The date the file was created. */
		vfsFileDateModified	= 2,		/**< The date the file was last modified. */
		vfsFileDateAccessed	= 3		/**< The date the file was last accessed. */
	};

	/** @brief VFS file iterator constants */
	enum dlpVFSFileIteratorConstants {
		vfsIteratorStart	= 0,		/** < Indicates that iterator is beginning */
		vfsIteratorStop		= -1		/**< Indicate that iterator has gone through all items */
	};
/*@}*/


/** @brief Error codes returned by DLP transactions
 *
 * After a DLP transaction, there may be a DLP or Palm OS error
 * if the result code is #PI_ERR_DLP_PALMOS. In this case, use
 * pi_palmos_error() to obtain the error code. It can be in the
 * DLP error range (0 > error < #dlpErrLastError), or otherwise
 * in the Palm OS error range (see Palm OS header files for
 * definitions, in relation with each DLP call)
 */
enum dlpErrors {
	dlpErrNoError = 0,	/**< No error */
	dlpErrSystem,		/**< System error (0x0001) */
	dlpErrIllegalReq,	/**< Illegal request, not supported by this version of DLP (0x0002) */
	dlpErrMemory,		/**< Not enough memory (0x0003) */
	dlpErrParam,		/**< Invalid parameter (0x0004) */
	dlpErrNotFound,		/**< File, database or record not found (0x0005) */
	dlpErrNoneOpen,		/**< No file opened (0x0006) */
	dlpErrAlreadyOpen,	/**< File already open (0x0007) */
	dlpErrTooManyOpen,	/**< Too many open files (0x0008) */
	dlpErrExists,		/**< File already exists (0x0009) */
	dlpErrOpen,		/**< Can't open file (0x000a) */
	dlpErrDeleted,		/**< File deleted (0x000b) */
	dlpErrBusy,		/**< Record busy (0x000c) */
	dlpErrNotSupp,		/**< Call not supported (0x000d) */
	dlpErrUnused1,		/**< @e Unused (0x000e) */
	dlpErrReadOnly,		/**< File is read-only (0x000f) */
	dlpErrSpace,		/**< Not enough space left on device (0x0010) */
	dlpErrLimit,		/**< Limit reached (0x0011) */
	dlpErrSync,		/**< Sync error (0x0012) */
	dlpErrWrapper,		/**< Wrapper error (0x0013) */
	dlpErrArgument,		/**< Invalid argument (0x0014) */
	dlpErrSize,		/**< Invalid size (0x0015) */

	dlpErrUnknown = 127	/**< Unknown error (0x007F) */
};


#ifndef SWIG	/* no need to clutter the bindings with this */

/** @brief Internal DLP argument structure */
struct dlpArg {
	int 	id_;		/**< Argument ID (start at #PI_DLP_ARG_FIRST_ID) */
	size_t	len;		/**< Argument length */
	char *data;		/**< Argument data */
};

/** @brief Internal DLP command request structure */
struct dlpRequest {
	enum dlpFunctions cmd;	/**< Command ID */
	int argc;		/**< Number of arguments */
	struct dlpArg **argv;	/**< Ptr to arguments */
};

/** @brief Internal DLP command response structure */
struct dlpResponse {
	enum dlpFunctions cmd;	/**< Command ID as returned by device. If not the same than requested command, this is an error */
	enum dlpErrors err;	/**< DLP error (see #dlpErrors enum) */
	int argc;		/**< Number of response arguments */
	struct dlpArg **argv;	/**< Response arguments */
};

#endif	/* !SWIG */

/* @name Functions used internally by dlp.c */
/*@{*/
#ifndef SWIG	/* don't export these functions to bindings */
	extern struct dlpArg * dlp_arg_new PI_ARGS((int id_, size_t len));
	extern void dlp_arg_free PI_ARGS((struct dlpArg *arg));
	extern int dlp_arg_len PI_ARGS((int argc, struct dlpArg **argv));

	extern struct dlpRequest *dlp_request_new
		PI_ARGS((enum dlpFunctions cmd, int argc, ...));
	extern struct dlpRequest * dlp_request_new_with_argid
		PI_ARGS((enum dlpFunctions cmd, int argid, int argc, ...));
	extern void dlp_request_free PI_ARGS((struct dlpRequest *req));

	extern struct dlpResponse *dlp_response_new
		PI_ARGS((enum dlpFunctions cmd, int argc));
	extern ssize_t dlp_response_read PI_ARGS((struct dlpResponse **res,
		int sd));
	extern ssize_t dlp_request_write PI_ARGS((struct dlpRequest *req,
		int sd));
	extern void dlp_response_free PI_ARGS((struct dlpResponse *req));

	extern int dlp_exec PI_ARGS((int sd, struct dlpRequest *req,
		struct dlpResponse **res));

	extern char *dlp_errorlist[];
	extern char *dlp_strerror(int error);

	struct RPC_params;
	extern int dlp_RPC
		PI_ARGS((int sd, struct RPC_params * p,
			uint32_t *result));
#endif	/* !SWIG */
/*@}*/

/** @name DLP library functions */
/*@{*/
	/** @brief Set the version of the DLP protocol we report to the device.
	 *
	 * During the handshake phase, the device and the desktop exchange the
	 * version of the DLP protocol both support. If the device's DLP version
	 * is higher than the desktop's, the device usually refuses to connect.
	 *
	 * @note Call this function prior to accepting or initiating a connection.
	 * 
	 * @param major Protocol major version
	 * @param minor Protocol minor version
	 */
	extern void dlp_set_protocol_version
			PI_ARGS((int major, int minor));

	/** @brief Convert a Palm OS date to a local date
	 *
	 * Local dates are using the local machine's timezone. If the Palm OS date
	 * is undefined, the local date is set to @c 0x83DAC000 (Fri Jan  1 00:00:00 1904 GMT)
	 *
	 * @param timeDateData Ptr to a time/date data block returned by Palm OS
	 * @return converted date
	 */
	extern time_t dlp_ptohdate PI_ARGS((PI_CONST unsigned char *timeDateData));

	/** @brief Convert a date to Palm OS date
	 *
	 * If the local date is @c 0x83DAC000 (Fri Jan  1 00:00:00 1904 GMT) the Palm OS date
	 * is set to undefined. Otherwise the date is converted from local time to Palm OS
	 *
	 * @param palm_time The date to convert
	 * @param timeDateData Ptr to an 8 byte buffer to hold the Palm OS date
	 */
	extern void dlp_htopdate PI_ARGS((time_t palm_time, unsigned char *timeDateData));
/*@}*/

/** @name System functions */
/*@{*/
	/** @brief Get the time from the device and return it as a local time_t value
	 *
	 * @param sd Socket number
	 * @param palm_time Pointer to a time_t to fill
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_GetSysDateTime PI_ARGS((int sd, time_t *palm_time));

	/** @brief Set the time on the Palm using a local time_t value.
	 *
	 * @param sd Socket number
	 * @param palm_time New time to set the device to (expressed using the computer's timezone)
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_SetSysDateTime PI_ARGS((int sd, time_t palm_time));

	/** @brief Read the system information block
	 *
	 * @param sd Socket number
	 * @param sysinfo Returned system information
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_ReadSysInfo PI_ARGS((int sd, struct SysInfo *sysinfo));

	/** @brief Read information about internal handheld memory
	 *
	 * @param sd Socket number
	 * @param cardno Card number (zero based)
	 * @param cardinfo Returned information about the memory card.
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_ReadStorageInfo
		PI_ARGS((int sd, int cardno, struct CardInfo *cardinfo));

	/** @brief Read the device user information
	 *
	 * @param sd Socket number
	 * @param user Returned user info
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_ReadUserInfo
		PI_ARGS((int sd, struct PilotUser *user));

	/** @brief Change the device user information
	 *
	 * @param sd Socket number
	 * @param INPUT New user info
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_WriteUserInfo
		PI_ARGS((int sd, PI_CONST struct PilotUser *INPUT));

	/** @brief Convenience function to reset lastSyncPC in the UserInfo to 0
	 *
	 * @param sd Socket number
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_ResetLastSyncPC PI_ARGS((int sd));

	/** @brief Read Network HotSync information
	 *
	 * Supported on Palm OS 2.0 and later.
	 *
	 * @param sd Socket number
	 * @param OUTPUT On return, filled NetSyncInfo structure
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_ReadNetSyncInfo
		PI_ARGS((int sd, struct NetSyncInfo *OUTPUT));

	/** @brief Set Network HotSync information
	 *
	 * Supported on Palm OS 2.0 and later
	 *
	 * @param sd Socket number
	 * @param INPUT NetSyncInfo structure to set
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_WriteNetSyncInfo
		PI_ARGS((int sd, PI_CONST struct NetSyncInfo *INPUT));

	/** @brief State that a conduit has started running on the desktop
	 *
	 * Puts up a status message on the device. Calling this method regularly
	 * is also the only reliable way to know whether the user pressed the Cancel
	 * button on the device.
	 *
	 * @param sd Socket number
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_OpenConduit PI_ARGS((int sd));

	/** @brief Terminate connection with the device
	 *
	 * Required at the end of a session. The pi_socket layer
	 * will call this for you if you don't. After the device receives this
	 * command, it will terminate the connection.
	 *
	 * @param sd Socket number
	 * @param status End of sync status (see #dlpEndStatus enum)
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_EndOfSync PI_ARGS((int sd, int status));

	/** @brief Terminate HotSync _without_ notifying Palm.
	 *
	 * This will cause the Palm to time out, and should (if I remember right)
	 * lose any changes to unclosed databases. _Never_ use under ordinary
	 * circumstances. If the sync needs to be aborted in a reasonable
	 * manner, use EndOfSync with a non-zero status.
	 *
	 * @param sd Socket number
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_AbortSync PI_ARGS((int sd));

	/** @brief Read a Feature from the device
	 *
	 * @param sd Socket number
	 * @param creator Feature creator
	 * @param num Feature number
	 * @param feature On return, the feature value
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_ReadFeature
		PI_ARGS((int sd, uint32_t creator, int num,
			uint32_t *feature));

	/** @brief Emulation of the SysGetROMToken function on the device
	 *
	 * Supported on Palm OS 2.0 through 4.0. Using this function
	 * is not recommended.
	 *
	 * @warning This function uses 68K RPC calls to perform its duty,
	 * and is therefore not supported on devices running Palm OS 5.0
	 * and later. Actually, it may even crash the device.
	 *
	 * @param sd Socket number
	 * @param token ROM token to read
	 * @param databuf Buffer to store the token data in
	 * @param datasize Size of data to read
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_GetROMToken
		PI_ARGS((int sd, uint32_t token, void *databuf, size_t *datasize));

	/** @brief Add an entry into the HotSync log on the device
	 *
	 * Move to the next line with \\n, as usual. You may invoke this
	 * command once or more before calling dlp_EndOfSync(), but it is
	 * not required.
	 *
	 * @param sd Socket number
	 * @param string Nul-terminated string with the text to insert in the log
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_AddSyncLogEntry PI_ARGS((int sd, char *string));

	/** @brief Call an application on the device
	 *
	 * 32-bit retcode and data over 64k only supported on Palm OS 2.0 and later.
	 *
	 * This function allows calling an application (or any PRC that responds
	 * to launch codes) using a custom launch code and custom data. The
	 * application can return data too, using DlkControl() and the
	 * dlkCtlSendCallAppReply selector. See Palm OS documentation for more
	 * information.
	 *
	 * @param sd Socket number
	 * @param creator Creator code of the application to call
	 * @param type Type code of the application to call
	 * @param action Launch code to send to the application
	 * @param datasize Length of data block to pass to the application
	 * @param databuf Data block to pass to the application
	 * @param retcode On return, result code returned by the application
	 * @param retbuf Buffer allocated using pi_buffer_new(). On return contains the data returned by the application
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_CallApplication
		PI_ARGS((int sd, uint32_t creator, uint32_t type,
			int action, size_t datasize, PI_CONST void *databuf,
			uint32_t *retcode, pi_buffer_t *retbuf));

	/** @brief Convenience function to ead an app preference data block
	 *
	 * Supported on Palm OS 2.0 and later, emulated for Palm OS 1.x.
	 *
	 * @param sd Socket number
	 * @param creator Application creator
	 * @param prefid Preference ID
	 * @param backup If set, read from backup prefs (see Palm OS documentation). This flag is ignored on Palm OS 1.x.
	 * @param maxsize Maximum size of the data to return in buffer
	 * @param databuf If not NULL, buffer should be of size @p maxsize. On return, contains the preference data
	 * @param datasize If not NULL, on return contains the size of the preference data block
	 * @param version If not NULL
	 * @return A negative value if an error occured (see pi-error.h), otherwise the size of the preference block
	 */
	extern PI_ERR dlp_ReadAppPreference
		PI_ARGS((int sd, uint32_t creator, int prefid, int backup,
			int maxsize, void *databuf, size_t *datasize, int *version));

	/** @brief Write an app preference data block
	 *
	 * Supported on Palm OS 2.0 and later. Emulated on Palm OS 1.x.
	 *
	 * @param sd Socket number
	 * @param creator Application creator
	 * @param prefid Preference ID
	 * @param backup If set, write to backup prefs (see Palm OS documentation). This flag is ignored on Palm OS 1.x.
	 * @param version Version of the pref to write
	 * @param databuf Ptr to the data to write
	 * @param datasize Size of the data to write
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_WriteAppPreference
		PI_ARGS((int sd, uint32_t creator, int prefid, int backup,
			int version, PI_CONST void *databuf, size_t datasize));

	/** @brief Require reboot of device after HotSync terminates
	 *
	 * @param sd Socket number
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_ResetSystem PI_ARGS((int sd));

/*@}*/

/** @name Database access functions */
/*@{*/
	/** @brief Read the database list from the device
	 *
	 * The database list can be read either one database at a time (slower),
	 * or passing ::dlpDBListMultiple in the @p flags member. Pass ::dlpDBListRAM
	 * in @p flags to get the list of databases in RAM, and ::dlpDBListROM to get
	 * the list of databases in ROM. You can mix flags to obtain the desired
	 * result. Passing ::dlpDBListMultiple will return several DBInfo
	 * structures at once (usually 20). Use (info->used / sizeof(DBInfo)) to
	 * know how many database information blocks were returned.
	 * For the next call, pass the last DBInfo->index value + 1 to start to
	 * the next database. @n @n
	 * When all the database informations have been retrieved, this function returns
	 * #PI_ERR_DLP_PALMOS and pi_palmos_error() returns #dlpErrNotFound.
	 * 
	 * @param sd Socket number
	 * @param cardno Card number (should be 0)
	 * @param flags Flags (see #dlpDBList enum)
	 * @param start Index of first database to list (zero based)
	 * @param dblist Buffer filled with one or more DBInfo structure
	 * @return A negative value if an error occured or the DB list is exhausted (see pi-error.h)
	 *
	 */
	extern PI_ERR dlp_ReadDBList
		PI_ARGS((int sd, int cardno, int flags, int start,
			pi_buffer_t *dblist));

	/** @brief Find a database by name
	 *
	 * Supported on Palm OS 3.0 (DLP 1.2) and later.
	 *
	 * @param sd Socket number
	 * @param cardno Memory card number (usually 0)
	 * @param dbname Database name
	 * @param localid If not NULL, on return contains the LocalID of the database if it was found
	 * @param dbhandle If not NULL, on return contains the handle of the database if it is currently open
	 * @param dbInfo If not NULL, on return contains information about the database
	 * @param dbSize If not NULL, on return contains information about the database size
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_FindDBByName
		PI_ARGS((int sd, int cardno, PI_CONST char *dbname, uint32_t *localid, int *dbhandle,
			 struct DBInfo *dbInfo, struct DBSizeInfo *dbSize));

	/** @brief Get information about an open database
	 *
	 * Supported on Palm OS 3.0 (DLP 1.2) and later.
	 *
	 * @param sd Socket number
	 * @param dbhandle Open database handle, obtained from dlp_OpenDB()
	 * @param cardno If not NULL, on return contains the cardno of the memory card the database resides on
	 * @param localid If not NULL, on return contains the LocalID of the database
	 * @param dbInfo If not NULL, on return contains information about the database
	 * @param dbSize If not NULL, on return contains information about the database size
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_FindDBByOpenHandle
		PI_ARGS((int sd, int dbhandle, int *cardno, uint32_t *localid,
			 struct DBInfo *dbInfo, struct DBSizeInfo *dbSize));

	/** @brief Find databases by type and/or creator
	 *
	 * Supported on Palm OS 3.0 (DLP 1.2) and later. To look for multiple databases,
	 * make a first call with @p start set to 1, then subsequent calls with @p start set to 0
	 * until no more database is found.
	 *
	 * @param sd Socket number
	 * @param type If not 0, type code to look for
	 * @param creator If not 0, creator code to look for
	 * @param start If set, start a new search
	 * @param latest If set, returns the database with the latest version if there are several identical databases
	 * @param cardno If not NULL, on return contains the memory card number the database resides on
	 * @param localid If not NULL, on return contains the LocalID of the database
	 * @param dbhandle If not NULL, on return contains the handle of the database if it is currently open
	 * @param dbInfo If not NULL, on return contains information about the database
	 * @param dbSize If not NULL, on return contains information about the database size
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_FindDBByTypeCreator
		PI_ARGS((int sd, uint32_t type, uint32_t creator, int start,
			 int latest, int *cardno, uint32_t *localid, int *dbhandle,
			 struct DBInfo *dbInfo, struct DBSizeInfo *dbSize));

	/** @brief Look for a database on the device
	 *
	 * This function does not match any DLP layer function, but is
	 * intended as a shortcut for programs looking for databases. It
	 * uses a fairly byzantine mechanism for ordering the RAM databases
	 * before the ROM ones. You must feed the @a index slot from the
	 * returned info in @p start the next time round.
	 *
	 * @param sd Socket number
	 * @param cardno Card number (should be 0)
	 * @param start Index of first database to list (zero based)
	 * @param dbname If not NULL, look for a database with this name
	 * @param type If not 0, matching database must have this type
	 * @param creator If not 0, matching database must have this creator code
	 * @param OUTPUT Returned database information on success
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_FindDBInfo
		PI_ARGS((int sd, int cardno, int start, PI_CONST char *dbname,
			uint32_t type, uint32_t creator,
			struct DBInfo *OUTPUT));

	/** @brief Open a database on the Palm.
	 *
	 * @param sd Socket number
	 * @param cardno Card number (should be 0)
	 * @param mode Open mode (see #dlpOpenFlags enum)
	 * @param dbname Database name
	 * @param dbhandle Returned database handle to use if other calls like dlp_CloseDB()
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_OpenDB
		PI_ARGS((int sd, int cardno, int mode, PI_CONST char *dbname,
			int *dbhandle));

	/** @brief Close an opened database
	 *
	 * @param sd Socket number
	 * @param dbhandle The DB handle returned by dlp_OpenDB()
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_CloseDB PI_ARGS((int sd, int dbhandle));

	/** @brief Close all opened databases
	 *
	 * @param sd Socket number
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_CloseDB_All PI_ARGS((int sd));

	/** @brief Delete an existing database from the device
	 *
	 * @param sd Socket number
	 * @param cardno Card number (should be 0)
	 * @param dbname Database name
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_DeleteDB
		PI_ARGS((int sd, int cardno, PI_CONST char *dbname));

	/** @brief Create database on the device
	 *
	 * After creation, the database is open and ready for use. You should
	 * call dlp_CloseDB() once you're done with the database.
	 *
	 * @param sd Socket number
	 * @param creator Creator code for the new database (four-char code)
	 * @param type Type code for the new database (four-char code)
	 * @param cardno Card number (should be 0)
	 * @param flags Database flags (see #dlpDBFlags enum)
	 * @param version Database version number
	 * @param dbname Database name
	 * @param dbhandle On return, DB handle to pass to other calls like dlp_CloseDB()
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_CreateDB
		PI_ARGS((int sd, uint32_t creator, uint32_t type,
			int cardno, int flags, unsigned int version,
			PI_CONST char *dbname, int *dbhandle));

	/** @brief Return the number of records in an opened database.
	 *
	 * @param sd Socket number
	 * @param dbhandle Open database handle, obtained from dlp_OpenDB()
	 * @param numrecs On return, number of records in the database
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_ReadOpenDBInfo
		PI_ARGS((int sd, int dbhandle, int *numrecs));

	/** @brief Change information for an open database
	 *
	 * Supported on Palm OS 3.0 (DLP 1.2) and later.
	 *
	 * @param sd Socket number
	 * @param dbhandle Open database handle, obtained from dlp_OpenDB()
	 * @param flags Flags to set for this database (see #dlpDBFlags enum)
	 * @param clearFlags Flags to clear for this database (see #dlpDBFlags enum)
	 * @param version Version of this database
	 * @param createDate Creation date of this database
	 * @param modifyDate Modification date of this database (use @c 0x83DAC000 to unset)
	 * @param backupDate Last backup date of this database (use @c 0x83DAC000 to unset)
	 * @param type Database type code (four-char code)
	 * @param creator Database creator code (four-char code)
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_SetDBInfo
		PI_ARGS((int sd, int dbhandle, int flags, int clearFlags, unsigned int version,
			 time_t createDate, time_t modifyDate, time_t backupDate,
			 uint32_t type, uint32_t creator));

	/** @brief Delete a category from a database
	 *
	 * Any record in that category will be moved to the Unfiled category.
	 *
	 * @param sd Socket number
	 * @param dbhandle Open database handle, obtained from dlp_OpenDB()
	 * @param category Category to delete
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_DeleteCategory
		PI_ARGS((int sd, int dbhandle, int category));

	/** @brief Move all records from a category to another category
	 *
	 * @param sd Socket number
	 * @param dbhandle Open database handle, obtained from dlp_OpenDB()
	 * @param fromcat Category to move from (0-15)
	 * @param tocat Category to move to (0-15)
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_MoveCategory
		PI_ARGS((int sd, int dbhandle, int fromcat, int tocat));

	/** @brief Read a database's AppInfo block
	 *
	 * @param sd Socket number
	 * @param dbhandle Open database handle, obtained from dlp_OpenDB()
	 * @param offset Offset to start reading from (0 based)
	 * @param reqbytes Number of bytes to read (pass -1 to read all data from @p offset to the end of the AppInfo block)
	 * @param retbuf Buffer allocated using pi_buffer_new(). On return contains the data from the AppInfo block
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_ReadAppBlock
		PI_ARGS((int sd, int dbhandle, int offset, int reqbytes,
			pi_buffer_t *retbuf));

	/** @brief Write a database's AppInfo block
	 *
	 * @param sd Socket number
	 * @param dbhandle Open database handle, obtained from dlp_OpenDB()
	 * @param databuf Pointer to the new AppInfo data.
	 * @param datasize Length of the new AppInfo data. If 0, the AppInfo block is removed.
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_WriteAppBlock
		PI_ARGS((int sd, int dbhandle, PI_CONST void *databuf, size_t datasize));

	/** @brief Read a database's SortInfo block
	 *
	 * @param sd Socket number
	 * @param dbhandle Open database handle, obtained from dlp_OpenDB()
	 * @param offset Offset to start reading from (0 based)
	 * @param reqbytes Number of bytes to read (pass -1 to read all data from @p offset to the end of the SortInfo block)
	 * @param retbuf Buffer allocated using pi_buffer_new(). On return contains the data from the SortInfo block
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_ReadSortBlock
		PI_ARGS((int sd, int dbhandle, int offset, int reqbytes,
			pi_buffer_t *retbuf));

	/** @brief Write a database's SortInfo block
	 *
	 * @param sd Socket number
	 * @param dbhandle Open database handle, obtained from dlp_OpenDB()
	 * @param databuf Pointer to the new SortInfo data.
	 * @param datasize Length of the new SortInfo data. If 0, the SortInfo block is removed.
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_WriteSortBlock
		PI_ARGS((int sd, int dbhandle, PI_CONST void *databuf,
			size_t datasize));

	/** @brief Clean up a database by removing deleted/archived records
	 *
	 * Delete all records in the opened database which are marked as
	 * archived or deleted.
	 *
	 * @param sd Socket number
	 * @param dbhandle Open database handle, obtained from dlp_OpenDB()
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_CleanUpDatabase PI_ARGS((int sd, int dbhandle));

	/** @brief Reset dirty record flags, update sync time
	 *
	 * For record databases, reset all dirty flags. For both record and
	 * resource databases, set the last sync time to now.
	 *
	 * @param sd Socket number
	 * @param dbhandle Open database handle, obtained from dlp_OpenDB()
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_ResetSyncFlags PI_ARGS((int sd, int dbhandle));

	/** @brief Reset the nextRecord position used in dlp_ReadNextRecInCategory()
	 *
	 * This resets the nextRecord both internally and on the device.
	 *
	 * @param sd Socket number
	 * @param dbhandle Open database handle, obtained from dlp_OpenDB()
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_ResetDBIndex PI_ARGS((int sd, int dbhandle));

	/** @brief Read the list of record IDs from an open database
	 *
	 * @param sd Socket number
	 * @param dbhandle Open database handle, obtained from dlp_OpenDB()
	 * @param sort If non-zero, the on-device application with the same DB creator will be called to re-sort the records prior to returning the list
	 * @param start Index of first record ID to return (zero based)
	 * @param max Maximum number of record IDs to return
	 * @param recuids On return, @p count record UIDs
	 * @param count On return, the number of record IDs found in @p IDs
	 * @return A negative value if an error occured (see pi-error.h)
	 */
#ifndef SWIG			/* bindings provide a native implementation */
	extern PI_ERR dlp_ReadRecordIDList
		PI_ARGS((int sd, int dbhandle, int sort, int start, int max,
			recordid_t *recuids, int *count));
#endif

	/** @brief Read a record using its unique ID
	 *
	 * Read a record identified by its unique ID. Make sure you only
	 * request records that effectively exist in the database (use
	 * dlp_ReadRecordIDList() to retrieve the unique IDs of all records
	 * in the database).
	 *
	 * @param sd Socket number
	 * @param dbhandle Open database handle, obtained from dlp_OpenDB()
	 * @param recuid Record unique ID
	 * @param retbuf If not NULL, a buffer allocated using pi_buffer_new(). On return, contains the record contents
	 * @param recindex If not NULL, contains the record index on return.
	 * @param recattrs If not NULL, contains the record attributes on return.
	 * @param category If not NULL, contains the record category on return.
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_ReadRecordById
		PI_ARGS((int sd, int dbhandle, recordid_t recuid, pi_buffer_t *retbuf,
			int *recindex, int *recattrs, int *category));

	/** @brief Read a record using its index
	 *
	 * Read a record by record index (zero-based). Make sure you only
	 * request records within the bounds of database records
	 *
	 * @param sd Socket number
	 * @param dbhandle Open database handle, obtained from dlp_OpenDB()
	 * @param recindex Record index (zero based)
	 * @param retbuf If not NULL, a buffer allocated using pi_buffer_new(). On return, contains the record contents
	 * @param recuid If not NULL, contains the record UID on return.
	 * @param recattrs If not NULL, contains the record attributes on return.
	 * @param category If not NULL, contains the record category on return.
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_ReadRecordByIndex
		PI_ARGS((int sd, int dbhandle, int recindex, pi_buffer_t *retbuf,
			recordid_t *recuid, int *recattrs, int *category));

	/** @brief Iterate through modified records in database
	 *
	 * Return subsequent modified records on each call. Use dlp_ResetDBIndex()
	 * prior to starting iterations. Once all the records have been seen,
	 * this function returns PI_ERR_DLP_PALMOS and pi_palmos_error() returns
	 * #dlpErrNotFound.
	 *
	 * @param sd Socket number
	 * @param dbhandle Open database handle, obtained from dlp_OpenDB()
	 * @param retbuf If not NULL, a buffer created using pi_buffer_new(). Buffer is cleared first using pi_buffer_clear(). On return, contains the record data
	 * @param recuid If not NULL, contains the record unique ID on return
	 * @param recindex If not NULL, contains the record index on return
	 * @param recattrs If not NULL, contains the record attributes on return (see #dlpRecAttributes enum)
	 * @param category If not NULL, contains the record category on return
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_ReadNextModifiedRec
		PI_ARGS((int sd, int dbhandle, pi_buffer_t *retbuf, recordid_t *recuid,
			int *recindex, int *recattrs, int *category));

	/** @brief Iterate through modified records in category
	 *
	 * Return subsequent modified records on each call. Use dlp_ResetDBIndex()
	 * prior to starting iterations. Once all the records have been seen,
	 * this function returns PI_ERR_DLP_PALMOS and pi_palmos_error() returns
	 * #dlpErrNotFound.
	 *
	 * @param sd Socket number
	 * @param dbhandle Open database handle, obtained from dlp_OpenDB()
	 * @param category The category to iterate into
	 * @param retbuf If not NULL, a buffer created using pi_buffer_new(). Buffer is cleared first using pi_buffer_clear(). On return, contains the record data
	 * @param recuid If not NULL, contains the record unique ID on return
	 * @param recindex If not NULL, contains the record index on return
	 * @param recattrs If not NULL, contains the record attributes on return (see #dlpRecAttributes enum)
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_ReadNextModifiedRecInCategory
		PI_ARGS((int sd, int dbhandle, int category, pi_buffer_t *retbuf,
			recordid_t *recuid, int *recindex, int *recattrs));

	/** @brief Iterate through records in category
	 *
	 * Return subsequent records on each call. Use dlp_ResetDBIndex()
	 * prior to starting iterations. Once all the records have been seen,
	 * this function returns PI_ERR_DLP_PALMOS and pi_palmos_error() returns
	 * #dlpErrNotFound.
	 *
	 * @param sd Socket number
	 * @param dbhandle Open database handle, obtained from dlp_OpenDB()
	 * @param category The category to iterate into
	 * @param retbuf If not NULL, a buffer created using pi_buffer_new(). Buffer is cleared first using pi_buffer_clear(). On return, contains the record data
	 * @param recuid If not NULL, contains the record unique ID on return
	 * @param recindex If not NULL, contains the record index on return
	 * @param recattrs If not NULL, contains the record attributes on return (see #dlpRecAttributes enum)
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_ReadNextRecInCategory
		PI_ARGS((int sd, int dbhandle, int category, pi_buffer_t *retbuf,
			recordid_t *recuid, int *recindex, int *recattrs));

	/** @brief Create a new record in a database
	 *
	 * Use this call to add records to a database. On DLP 1.4 and later, you can create records
	 * bigger than 64k. Set the record ID to 0 to have the device generate the record ID itself,
	 * or assign a record ID of your own. Read Palm's documentation for information about
	 * record IDs, as there is a way to indicate which records were created by the desktop and
	 * which ones were created by the device applications.
	 *
	 * If you pass -1 as the data length, the function will treat the data as a string and use
	 * strlen(data)+1 as the data length (that is, the string is written including the
	 * terminating nul character).
	 *
	 * @param sd Socket number
	 * @param dbhandle Open database handle, obtained from dlp_OpenDB()
	 * @param flags Record attributes (see #dlpRecAttributes enum)
	 * @param recuid Record ID of the new record. If 0, device will generate a new record ID for this record.
	 * @param catid Category of the new record
	 * @param databuf Ptr to record data
	 * @param datasize Record data length
	 * @param newrecuid On return, record ID that was assigned to this record
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_WriteRecord
		PI_ARGS((int sd, int dbhandle, int flags, recordid_t recuid,
			int catid, PI_CONST void *databuf, size_t datasize,
			recordid_t *newrecuid));

	/** @brief Delete an existing record from a database
	 *
	 * @param sd Socket number
	 * @param dbhandle Open database handle, obtained from dlp_OpenDB()
	 * @param all If set, ALL records are deleted from the database.
	 * @param recuid Record ID of record to delete if @p all == 0.
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_DeleteRecord
		PI_ARGS((int sd, int dbhandle, int all, recordid_t recuid));

	/** @brief Read a resource identified by its type and ID
	 *
	 * @note To read resources larger than 64K, you should use dlp_ReadResourceByIndex().
	 *
	 * @param sd Socket number
	 * @param dbhandle Open database handle, obtained from dlp_OpenDB()
	 * @param type Type code for the resource (four-char code)
	 * @param resid Resource ID
	 * @param retbuf If not NULL, a buffer allocated using pi_buffer_new(). On return, contains the resource contents
	 * @param resindex If not NULL, on return contains the resource index
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_ReadResourceByType
		PI_ARGS((int sd, int dbhandle, uint32_t type, int resid,
			pi_buffer_t *retbuf, int *resindex));

	/** @brief Read a resource identified by its resource index
	 *
	 * This function supports reading resources larger than 64k on
	 * DLP 1.4 and later (Palm OS 5.2 and later).
	 *
	 * @param sd Socket number
	 * @param dbhandle Open database handle, obtained from dlp_OpenDB()
	 * @param resindex Resource index
	 * @param retbuf If not NULL, a buffer allocated using pi_buffer_new(). On return, contains the resource contents
	 * @param restype If not NULL, on return contains the resource type
	 * @param resid If not NULL, on return contains the resource ID
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_ReadResourceByIndex
		PI_ARGS((int sd, int dbhandle, unsigned int resindex, pi_buffer_t *retbuf,
			uint32_t *restype, int *resid));

	/** @brief Create a new resource of overwrite an existing one
	 *
	 * This function supports writing resources larger than 64k on
	 * DLP 1.4 and later (Palm OS 5.2 and later).
	 *
	 * @param sd Socket number
	 * @param dbhandle Open database handle, obtained from dlp_OpenDB()
	 * @param restype Resource type (four-char code)
	 * @param resid Resource ID
	 * @param databuf Ptr to resource data
	 * @param datasize Length of resource data to write
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_WriteResource
		PI_ARGS((int sd, int dbhandle, uint32_t restype, int resid,
			PI_CONST void *databuf, size_t datasize));

	/** @brief Delete a resource or all resources from a resource file
	 *
	 * @param sd Socket number
	 * @param dbhandle Open database handle, obtained from dlp_OpenDB()
	 * @param all If set, all resources are removed from this database (@p restype and @p resid are ignored)
	 * @param restype Resource type (four-char code)
	 * @param resid Resource ID
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_DeleteResource
		PI_ARGS((int sd, int dbhandle, int all, uint32_t restype,
			int resid));
/*@}*/

/** @name Expansion manager functions */
/*@{*/
	/** @brief Enumerate expansion slots
	 *
	 * Supported on Palm OS 4.0 and later. Expansion slots are physical slots
	 * present on the device. To check whether a card is inserted in a slot,
	 * use dlp_ExpCardPresent().
	 *
	 * @param sd Socket number
	 * @param numslots On input, maximum number of slots that can be returned in the slotRefs array. On return, the actual number of slot references returned in @p slotRefs.
	 * @param slotrefs On return, @p numSlots slot references
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_ExpSlotEnumerate
		PI_ARGS((int sd, int *numslots, int *slotrefs));

	/** @brief Checks whether a card is inserted in a slot
	 *
	 * Supported on Palm OS 4.0 and later. Returns >=0 if a card
	 * is inserted in the slot.
	 *
	 * @param sd Socket number
	 * @param slotref The slot reference as returned by dlp_ExpSlotEnumerate().
	 * @return A negative value if an error occured (see pi-error.h), >=0 if a card is inserted
	 */
	extern PI_ERR dlp_ExpCardPresent
		PI_ARGS((int sd, int slotref));

	/** @brief Get information about a removable card inserted in an expansion slot
	 *
	 * Supported on Palm OS 4.0 and later. The info strings are returned in a
	 * single malloc()'ed buffer as a suite of nul-terminated string, one
	 * after the other.
	 *
	 * @param sd Socket number
	 * @param slotref The slot reference as returned by dlp_ExpSlotEnumerate().
	 * @param expflags If not NULL, the card flags (see #dlpExpCardCapabilities enum)
	 * @param numstrings On return, the number of strings found in the @p strings array
	 * @param strings If not NULL, ptr to a char*. If there are strings to return, this function allocates a buffer to hold the strings. You are responsible for free()'ing the buffer once you're done with it.
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_ExpCardInfo
		PI_ARGS((int sd, int slotref, uint32_t *expflags,
			 int *numstrings, char **strings));

	/** @brief Return the type of media supported by an expansion slot
	 *
	 * Supported on Palm OS 5.2 and later (DLP 1.4 and later).
	 *
	 * @param sd Socket number
	 * @param slotref The slot reference as returned by dlp_ExpSlotEnumerate().
	 * @param mediatype On return, the media type
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_ExpSlotMediaType
		PI_ARGS((int sd, int slotref, uint32_t *mediatype));
/*@}*/

/** @name VFS manager functions */
/*@{*/
	/** @brief Returns a list of connected VFS volumes
	 *
	 * Supported on Palm OS 4.0 and later.
	 *
	 * @param sd Socket number
	 * @param numvols On input, the maximum number of volume references that can be returned. On output, the actual number of volume references
	 * @param volrefs On output, @p numVols volume references
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_VFSVolumeEnumerate
		PI_ARGS((int sd, int *numvols, int *volrefs));

	/** @brief Returns information about a VFS volume
	 *
	 * Supported on Palm OS 4.0 and later.
	 *
	 * @param sd Socket number
	 * @param volref Volume reference number (obtained from dlp_VFSVolumeEnumerate())
	 * @param volinfo On return, volume information
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_VFSVolumeInfo
		PI_ARGS((int sd, int volref, struct VFSInfo *volinfo));

	/** @brief Return the label (name) of a VFS volume
	 *
	 * Supported on Palm OS 4.0 and later.
	 *
	 * @param sd Socket number
	 * @param volref Volume reference number (obtained from dlp_VFSVolumeEnumerate())
	 * @param len On input, the maximum size of the name buffer. On output, the name length (including the ending nul byte)
	 * @param name On output, the nul-terminated volume name
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_VFSVolumeGetLabel
		PI_ARGS((int sd, int volref, int *len, char *name));

	/** @brief Change the label (name) of a VFS volume
	 *
	 * Supported on Palm OS 4.0 and later.
	 *
	 * @param sd Socket number
	 * @param volref Volume reference number (obtained from dlp_VFSVolumeEnumerate())
	 * @param name New volume name
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_VFSVolumeSetLabel
		PI_ARGS((int sd, int volref, PI_CONST char *name));

	/** @brief Return the total and used size of a VFS volume
	 *
	 * Supported on Palm OS 4.0 and later.
	 *
	 * @param sd Socket number
	 * @param volref Volume reference number (obtained from dlp_VFSVolumeEnumerate())
	 * @param usedbytes On return, number of bytes used on the volume
	 * @param totalbytes On return, total size of the volume in bytes
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_VFSVolumeSize
		PI_ARGS((int sd, int volref, int32_t *usedbytes, int32_t *totalbytes));

	/** @brief Format a VFS volume
	 *
	 * Supported on Palm OS 4.0 and later.
	 *
	 * @param sd Socket number
	 * @param fmtflags Format flags (undocumented for now)
	 * @param fsLibRef File system lib ref (undocumented for now)
	 * @param param Slot mount parameters (undocumented for now)
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_VFSVolumeFormat
		PI_ARGS((int sd, unsigned char fmtflags, int fsLibRef,
			struct VFSSlotMountParam *param));

	/** @brief Get the default storage directory for a given file type
	 *
	 * Supported on Palm OS 4.0 and later. Return the default directory
	 * for a file type. File types as expressed as MIME types, for
	 * example "image/jpeg", or as a simple file extension (i.e. ".jpg")
	 *
	 * @param sd Socket number
	 * @param volref Volume reference number (obtained from dlp_VFSVolumeEnumerate())
	 * @param name MIME type to get the default directory for
	 * @param dir A buffer to hold the default path
	 * @param len On input, the length of the @p dir buffer. On return, contains the length of the path string (including the nul terminator)
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_VFSGetDefaultDir
		PI_ARGS((int sd, int volref, PI_CONST char *name,
			char *dir, int *len));

	/** @brief Iterate through the entries in a directory
	 *
	 * Supported on Palm OS 4.0 and later. At the beginning you set
	 * @p dirIterator to #vfsIteratorStart, then call this function
	 * repeatedly until it returns an error code of the iterator becomes
	 * #vfsIteratorStop.
	 *
	 * @bug On some early OS 5 devices like Tungsten T and Sony NX70, NX73 this
	 * call crashes the device. This has been confirmed to be a bug in HotSync on
	 * the device, as tests showed that a regular HotSync conduit does crash the
	 * device with this call too.
	 *
	 * @param sd Socket number
	 * @param dirref Directory reference obtained from dlp_VFSFileOpen()
	 * @param diriterator Ptr to an iterator. Start with #vfsIteratorStart
	 * @param maxitems On input, the max number of VFSDirInfo structures stored in @p dirItems. On output, the actual number of items.
	 * @param diritems Preallocated array that contains a number of VFSDirInfo structures on return.
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_VFSDirEntryEnumerate
		PI_ARGS((int sd, FileRef dirref, uint32_t *diriterator,
			int *maxitems, struct VFSDirInfo *diritems));

	/** @brief Create a new directory on a VFS volume
	 *
	 * Supported on Palm OS 4.0 and later.
	 *
	 * @param sd Socket number
	 * @param volref Volume reference number (obtained from dlp_VFSVolumeEnumerate())
	 * @param path Full path for the directory to create
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_VFSDirCreate
		PI_ARGS((int sd, int volref, PI_CONST char *path));

	/** @brief Import a VFS file to a database on the handheld
	 *
	 * Supported on Palm OS 4.0 and later. The file is converted to a
	 * full fledged database and stored in the handheld's RAM.
	 *
	 * @param sd Socket number
	 * @param volref Volume reference number (obtained from dlp_VFSVolumeEnumerate())
	 * @param pathname Path of the file to transfer to the handheld
	 * @param cardno On return, card number the database was created on (usually 0)
	 * @param localid On return, LocalID of the database that was created
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_VFSImportDatabaseFromFile
		PI_ARGS((int sd, int volref, PI_CONST char *pathname,
			 int *cardno, uint32_t *localid));

	/** @brief Export a database to a VFS file
	 *
	 * Supported on Palm OS 4.0 and later. The database is converted to a
	 * .prc, .pdb or .pqa file on the VFS volume.
	 *
	 * @param sd Socket number
	 * @param volref Volume reference number (obtained from dlp_VFSVolumeEnumerate())
	 * @param pathname Path of the file to create on the VFS volume
	 * @param cardno Card number the database resides on (usually 0)
	 * @param localid LocalID of the database to export
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_VFSExportDatabaseToFile
		PI_ARGS((int sd, int volref, PI_CONST char *pathname,
			int cardno, unsigned int localid));

	/** @brief Create a new file on a VFS volume
	 *
	 * Supported on Palm OS 4.0 and later.
	 *
	 * @param sd Socket number
	 * @param volref Volume reference number (obtained from dlp_VFSVolumeEnumerate())
	 * @param pathname Full path of the file to create
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_VFSFileCreate
		PI_ARGS((int sd, int volref, PI_CONST char *pathname));

	/** @brief Open an existing file on a VFS volume
	 *
	 * Supported on Palm OS 4.0 and later. On some devices, it is required to open the
	 * file using the #dlpOpenReadWrite mode to be able to write to it (using
	 * #dlpOpenWrite is not enough).
	 *
	 * @param sd Socket number
	 * @param volref Volume reference number (obtained from dlp_VFSVolumeEnumerate())
	 * @param path Full path of the file to open
	 * @param mode Open mode flags (see #dlpVFSOpenFlags enum)
	 * @param fileref On return, file reference to the open file
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_VFSFileOpen
		PI_ARGS((int sd, int volref, PI_CONST char *path, int mode,
			FileRef *fileref));

	/** @brief Close an open VFS file
	 *
	 * Supported on Palm OS 4.0 and later.
	 *
	 * @param sd Socket number
	 * @param fileref File reference obtained from dlp_VFSFileOpen()
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_VFSFileClose
		PI_ARGS((int sd, FileRef fileref));

	/** @brief Write data to an open file
	 *
	 * Supported on Palm OS 4.0 and later. Will return the number of bytes
	 * written if successful.
	 *
	 * @param sd Socket number
	 * @param fileref File reference obtained from dlp_VFSFileOpen()
	 * @param databuf Ptr to the data to write
	 * @param datasize Length of the data to write
	 * @return A negative value if an error occured (see pi-error.h), the number of bytes written otherwise.
	 */
	extern PI_ERR dlp_VFSFileWrite
		PI_ARGS((int sd, FileRef fileref, PI_CONST void *databuf, size_t datasize));

	/** @brief Read data from an open file
	 *
	 * Supported on Palm OS 4.0 and later. Will return the total number of bytes
	 * actually read.
	 *
	 * @param sd Socket number
	 * @param fileref File reference obtained from dlp_VFSFileOpen()
	 * @param retbuf Buffer allocated using pi_buffer_new(). Buffer is being emptied first with pi_buffer_clear(). On return contains the data read from the file.
	 * @param reqbytes Number of bytes to read from the file.
	 * @return A negative value if an error occured (see pi-error.h), or the total number of bytes read
	 */
	extern PI_ERR dlp_VFSFileRead
		PI_ARGS((int sd, FileRef fileref, pi_buffer_t *retbuf, size_t reqbytes));

	/** @brief Delete an existing file from a VFS volume
	 *
	 * Supported on Palm OS 4.0 and later.
	 *
	 * @param sd Socket number
	 * @param volref Volume reference number (obtained from dlp_VFSVolumeEnumerate())
	 * @param pathname Full access path to the file to delete
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_VFSFileDelete
		PI_ARGS((int sd, int volref, PI_CONST char *pathname));

	/** @brief Rename an existing file
	 *
	 * Supported on Palm OS 4.0 and later.
	 *
	 * @note This function can NOT be used to move a file from one place
	 * to another. You can only rename a file that will stay in the same
	 * directory.
	 *
	 * @param sd Socket number
	 * @param volref Volume reference number (obtained from dlp_VFSVolumeEnumerate())
	 * @param pathname Full access path to the file to rename
	 * @param newname New file name, without the rest of the access path
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_VFSFileRename
		PI_ARGS((int sd, int volref, PI_CONST char *pathname,
			PI_CONST char *newname));

	/** @brief Checks whether the current position is at the end of file
	 *
	 * Supported on Palm OS 4.0 and later.
	 *
	 * @param sd Socket number
	 * @param fileref File reference obtained from dlp_VFSFileOpen()
	 * @return A negative value if an error occured (see pi-error.h). 0 if not at EOF, >0 if at EOF.
	 */
	extern PI_ERR dlp_VFSFileEOF
		PI_ARGS((int sd, FileRef fileref));

	/** @brief Return the current seek position in an open file
	 *
	 * Supported on Palm OS 4.0 and later.
	 *
	 * @param sd Socket number
	 * @param fileref File reference obtained from dlp_VFSFileOpen()
	 * @param position On return, current absolute position in the file
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_VFSFileTell
		PI_ARGS((int sd, FileRef fileref, int *position));

	/** @brief Return the attributes of an open file
	 *
	 * Supported on Palm OS 4.0 and later.
	 *
	 * @param sd Socket number
	 * @param fileref File reference obtained from dlp_VFSFileOpen()
	 * @param fileattrs On return, file attributes (see #dlpVFSFileAttributeConstants enum)
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_VFSFileGetAttributes
		PI_ARGS((int sd, FileRef fileref, uint32_t *fileattrs));

	/** @brief Change the attributes of an open file
	 *
	 * Supported on Palm OS 4.0 and later.
	 *
	 * @param sd Socket number
	 * @param fileref File reference obtained from dlp_VFSFileOpen()
	 * @param fileattrs n-New file attributes (see #dlpVFSFileAttributeConstants enum)
	 * @return A negative value if an error occured (see pi-error.h).
	 */
	extern PI_ERR dlp_VFSFileSetAttributes
		PI_ARGS((int sd, FileRef fileref, uint32_t fileattrs));

	/** @brief Return one of the dates associated with an open file or directory
	 *
	 * Supported on Palm OS 4.0 and later.
	 *
	 * @param sd Socket number
	 * @param fileref File or directory reference obtained from dlp_VFSFileOpen()
	 * @param which The date you want (see #dlpVFSDateConstants enum)
	 * @param date On return, the requested date
	 * @return A negative value if an error occured (see pi-error.h).
	 */
	extern PI_ERR dlp_VFSFileGetDate
		PI_ARGS((int sd, FileRef fileref, int which, time_t *date));

	/** @brief Change one of the dates for an open file or directory
	 *
	 * Supported on Palm OS 4.0 and later.
	 *
	 * @param sd Socket number
	 * @param fileref File or directory reference obtained from dlp_VFSFileOpen()
	 * @param which The date you want to change (see #dlpVFSDateConstants enum)
	 * @param date The new date to set
	 * @return A negative value if an error occured (see pi-error.h).
	 */
	extern PI_ERR dlp_VFSFileSetDate
		PI_ARGS((int sd, FileRef fileref, int which, time_t date));

	/** @brief Change the current seek position in an open file
	 *
	 * Supported on Palm OS 4.0 and later.
	 *
	 * @param sd Socket number
	 * @param fileref File or directory reference obtained from dlp_VFSFileOpen()
	 * @param origin Where to seek from (see #dlpVFSSeekConstants enum)
	 * @param offset Seek offset
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_VFSFileSeek
		PI_ARGS((int sd, FileRef fileref, int origin, int offset));

	/** @brief Resize an open file
	 *
	 * Supported on Palm OS 4.0 and later.
	 *
	 * @param sd Socket number
	 * @param fileref File or directory reference obtained from dlp_VFSFileOpen()
	 * @param newsize New file size
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_VFSFileResize
		PI_ARGS((int sd, FileRef fileref, int newsize));

	/** @brief Return the size of an open file
	 *
	 * Supported on Palm OS 4.0 and later.
	 *
	 * @param sd Socket number
	 * @param fileref File or directory reference obtained from dlp_VFSFileOpen()
	 * @param size On return, the actual size of the file
	 * @return A negative value if an error occured (see pi-error.h)
	 */
	extern PI_ERR dlp_VFSFileSize
		PI_ARGS((int sd, FileRef fileref, int *size));
/*@}*/

#ifdef __cplusplus
}
#endif
#endif				/*_PILOT_DLP_H_*/
