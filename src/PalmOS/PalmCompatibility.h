/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: PalmCompatibility.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *		Type & macro definitions for compile-time compatibility between
 *		old and new header files.
 *
 *****************************************************************************/

#ifndef __PALMCOMPATIBILITY_H__
#define __PALMCOMPATIBILITY_H__

#include <PalmTypes.h>

// The data types Byte, Word, DWord and so on are now deprecated.  We
// recommend that you use the corresponding new data types: for example,
// use Int16 instead of SWord and UInt32 instead of DWord.  In particular,
// the unfortunate distinction between Handle/VoidHand has been fixed:
// use MemHandle instead.

typedef Int8		SByte;
#ifdef PALMOS
#if __DEFINE_TYPES_						
typedef UInt8		Byte;
#endif
#else
typedef UInt8		Byte;
#endif

typedef Int16		SWord;
typedef UInt16		Word;

typedef Int32		SDWord;
typedef UInt32		DWord;


// Logical data types
typedef Int8		SChar;
typedef UInt8		UChar;

typedef Int16		Short;
typedef UInt16		UShort;
  
typedef Int16		Int;
typedef UInt16		UInt;

typedef Int32		Long;
typedef UInt32		ULong;


// Pointer Types
typedef MemPtr		VoidPtr;
typedef MemHandle	VoidHand;

#if __DEFINE_TYPES_
typedef MemPtr		Ptr;
typedef MemHandle	Handle;
#endif


// Because "const BytePtr" means "const pointer to Byte" rather than "pointer
// to const Byte", all these XXXXPtr types are deprecated: you're better off
// just using "Byte *" and so on.  (Even better, use "UInt8 *"!)

typedef SByte*		SBytePtr;
#if __DEFINE_TYPES_						 
typedef Byte*		BytePtr;
#endif

typedef SWord*		SWordPtr;
typedef Word*		WordPtr;
typedef UInt16*	UInt16Ptr;

typedef SDWord*	SDWordPtr;
typedef DWord*		DWordPtr;

// Logical data types
typedef Boolean*	BooleanPtr;

typedef Char*		CharPtr;
typedef SChar*		SCharPtr;
typedef UChar*		UCharPtr;

typedef WChar*		WCharPtr;

typedef Short*		ShortPtr;
typedef UShort*	UShortPtr;

typedef Int*		IntPtr;
#ifdef PALMOS
typedef UInt*		UIntPtr;
#endif

typedef Long*		LongPtr;
typedef ULong*		ULongPtr;

// Instead of indexing through countries and languages, developers should call
//	the Locale Manager to index through known locales:
#define languageFirst			lEnglish							// From Preferences.h
#define languageLast				lDutch							// From Preferences.h
#define languageCount			(languageLast - languageFirst + 1)	// From Preferences.h
#define countryFirst				cAustralia						// From Preferences.h
#define countryLast				cTaiwan							// From Preferences.h
#define countryCount				(countryLast - countryFirst + 1)	// From Preferences.h

// Incorporated into the Locale Manager:
#define countryNameLength		(kMaxCountryNameLen+1)		//	From Preferences.h
#define currencyNameLength		(kMaxCurrencyNameLen+1)		//	From Preferences.h
#define currencySymbolLength	(kMaxCurrencySymbolLen+1)	//	From Preferences.h

/********************************************************************
 *
 *				Deprecated screen stuff
 *
 ********************************************************************/

#define scrCopy									winPaint
#define scrAND										winErase
#define scrANDNOT									winMask
#define scrXOR										winInvert
#define scrOR										winOverlay
#define scrCopyNOT								winPaintInverse

#define scrDisplayModeGetDefaults			winScreenModeGetDefaults
#define scrDisplayModeGet						winScreenModeGet
#define scrDisplayModeSetToDefaults			winScreenModeSetToDefaults
#define scrDisplayModeSet						winScreenModeSet
#define scrDisplayModeGetSupportedDepths	winScreenModeGetSupportedDepths
#define scrDisplayModeGetSupportsColor		winScreenModeGetSupportsColor

#define ScrOperation								WinDrawOperation

#define ScrDisplayMode(op, widthP, heightP, depthP, enableColorP) \
		  WinScreenMode(op, widthP, heightP, depthP, enableColorP)

#define ScrInit() WinScreenInit()


/********************************************************************
 *
 *				Deprecated resource ids
 *
 ********************************************************************/

// Resources with system ids (>= 10000) are subject to change, and
// should _not_ be relied upon.

// System date string resources. You should use DateTemplateToAscii 
// (Palm OS 3.5 or later) or DateGlueTemplateToAscii (backwards
// compatible) instead of these resources.
#define daysOfWeekStrID							10000
#define dayFullNamesStrID						10001
#define monthNamesStrID							10002
#define monthFullNamesStrID					10003

// More system date string resources, introduced in Palm OS 3.5.  If you use
// these, you are limiting yourself to running on nothing earlier than 3.5,
// so you likely might as well use DateTempalateToAscii instead.
#define daysOfWeekShortStrListID				10200
#define daysOfWeekStdStrListID				10201
#define daysOfWeekLongStrListID				10202
#define monthNamesShortStrListID				10203
#define monthNamesStdStrListID				10204
#define monthNamesLongStrListID				10205

// The country table resource has changed between versions, and is
// now completely obsolete. Use LmGetLocaleSetting (4.0 or later)
// or LmGlueGetLocaleSetting instead of this resource.
#define sysResTCountries						'cnty'
#define sysResIDCountries						10000

/********************************************************************
 *
 *				Deprecated SYS_TRAP macro machinery
 *
 ********************************************************************/

// This nWORD_INLINE stuff was never really portable to GCC.  See the
// new SYS_TRAP framework in PalmTypes.h for most uses of these macros.
// Generally if you just want to use CodeWarrior, you can use these
// macros or just use the = { x, y, z} notation directly.  To be
// portable between CodeWarrior and m68k-palmos-gcc, you should use
// the ={x,y,z} notation on the CW side and __attribute__((callseq))
// on the GCC side (see the Palm OS-specific manual).

#if USE_TRAPS == 0
// Disable Palm OS System and Library trap inline code
// Header files become function prototypes for direct-linking
#define ONEWORD_INLINE(trapNum)
#define TWOWORD_INLINE(w1, w2)
#define THREEWORD_INLINE(w1, w2, w3)
#define FOURWORD_INLINE(w1, w2, w3, w4)
#define FIVEWORD_INLINE(w1, w2, w3, w4, w5)
#define SIXWORD_INLINE(w1, w2, w3, w4, w5, w6)
#define SEVENWORD_INLINE(w1, w2, w3, w4, w5, w6, w7)
#define EIGHTWORD_INLINE(w1, w2, w3, w4, w5, w6, w7, w8)
#define NINEWORD_INLINE(w1, w2, w3, w4, w5, w6, w7, w8, w9)
#define TENWORD_INLINE(w1, w2, w3, w4, w5, w6, w7, w8, w9, w10)
#define ELEVENWORD_INLINE(w1, w2, w3, w4, w5, w6, w7, w8, w9, w10, w11)
#define TWELVEWORD_INLINE(w1, w2, w3, w4, w5, w6, w7, w8, w9, w10, w11, w12)

#else
// Enable Palm OS System and Library trap inline code
// Header files contain inline glue (opcodes) to be inserted in calling code
#define ONEWORD_INLINE(trapNum) 	\
	= trapNum
#define TWOWORD_INLINE(w1, w2) 	\
	= {w1,w2}
#define THREEWORD_INLINE(w1, w2, w3)  \
	= {w1,w2,w3}
#define FOURWORD_INLINE(w1, w2, w3, w4)  \
	= {w1,w2,w3,w4}
#define FIVEWORD_INLINE(w1, w2, w3, w4, w5)  \
	= {w1,w2,w3,w4,w5}
#define SIXWORD_INLINE(w1, w2, w3, w4, w5, w6)  \
	= {w1,w2,w3,w4,w5,w6}
#define SEVENWORD_INLINE(w1, w2, w3, w4, w5, w6, w7)  \
	= {w1,w2,w3,w4,w5,w6,w7}
#define EIGHTWORD_INLINE(w1, w2, w3, w4, w5, w6, w7, w8)  \
	= {w1,w2,w3,w4,w5,w6,w7,w8}
#define NINEWORD_INLINE(w1, w2, w3, w4, w5, w6, w7, w8, w9)  \
	= {w1,w2,w3,w4,w5,w6,w7,w8,w9}
#define TENWORD_INLINE(w1, w2, w3, w4, w5, w6, w7, w8, w9, w10)  \
	= {w1,w2,w3,w4,w5,w6,w7,w8,w9,w10}
#define ELEVENWORD_INLINE(w1, w2, w3, w4, w5, w6, w7, w8, w9, w10, w11)  \
	= {w1,w2,w3,w4,w5,w6,w7,w8,w9,w10,w11}
#define TWELVEWORD_INLINE(w1, w2, w3, w4, w5, w6, w7, w8, w9, w10, w11, w12)  \
	= {w1,w2,w3,w4,w5,w6,w7,w8,w9,w10,w11,w12}

#endif

/********************************************************************
 *
 *				Deprecated Expansion Manager names
 *
 ********************************************************************/

// Expansion Manager
#define invalidSlotRefNum					expInvalidSlotRefNum
#define expErrInvalidSlotRefNumber		expErrInvalidSlotRefNum
#define ExpMediaType_Any					expMediaType_Any
#define ExpMediaType_MemoryStick			expMediaType_MemoryStick
#define ExpMediaType_CompactFlash		expMediaType_CompactFlash
#define ExpMediaType_SecureDigital		expMediaType_SecureDigital
#define ExpMediaType_MultiMediaCard		expMediaType_MultiMediaCard
#define ExpMediaType_SmartMedia			expMediaType_SmartMedia
#define ExpMediaType_RAMDisk				expMediaType_RAMDisk
#define ExpMediaType_PoserHost			expMediaType_PoserHost
#define ExpMediaType_MacSim				expMediaType_MacSim

// VFS Manager:
#define VFSMountClass_SlotDriver			vfsMountClass_SlotDriver
#define VFSMountClass_Simulator			vfsMountClass_Simulator
#define fsOriginBeginning					vfsOriginBeginning
#define fsOriginCurrent						vfsOriginCurrent
#define fsOriginEnd							vfsOriginEnd
#define fsFilesystemType_VFAT				vfsFilesystemType_VFAT
#define fsFilesystemType_FAT				vfsFilesystemType_FAT
#define fsFilesystemType_NTFS				vfsFilesystemType_NTFS
#define fsFilesystemType_HFSPlus			vfsFilesystemType_HFSPlus
#define fsFilesystemType_HFS				vfsFilesystemType_HFS
#define fsFilesystemType_MFS				vfsFilesystemType_MFS
#define fsFilesystemType_EXT2				vfsFilesystemType_EXT2
#define fsFilesystemType_FFS				vfsFilesystemType_FFS
#define fsFilesystemType_NFS				vfsFilesystemType_NFS
#define fsFilesystemType_AFS				vfsFilesystemType_AFS
#define fsFilesystemType_Novell			vfsFilesystemType_Novell
#define fsFilesystemType_HPFS				vfsFilesystemType_HPFS
#define VFSFileAttributesGet				VFSFileGetAttributes
#define VFSFileAttributesSet				VFSFileSetAttributes
#define VFSFileDateGet						VFSFileGetDate
#define VFSFileDateSet						VFSFileSetDate
#define VFSVolumeLabelGet					VFSVolumeGetLabel
#define VFSVolumeLabelSet					VFSVolumeSetLabel
			
// FSLib:
#define FS_LIB_APIVersion					fsLibAPIVersion
#define FSFileAttributesGet				FSFileGetAttributes
#define FSFileAttributesSet				FSFileSetAttributes
#define FSFileDateGet						FSFileGetDate
#define FSFileDateSet						FSFileSetDate
#define FSVolumeLabelGet					FSVolumeGetLabel
#define FSVolumeLabelSet					FSVolumeSetLabel
	
// SlotDrvrLib:
#define SlotDrvr_LIB_APIVersion			slotDrvrAPIVersion
#define Slot_SECTOR_SIZE					slotSectorSize


#endif
