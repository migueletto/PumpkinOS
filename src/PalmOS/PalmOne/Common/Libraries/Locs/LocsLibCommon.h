/*****************************************************************************
 *
 * Copyright (c) 2005 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File:			DateBookLibCommon.h
 *
 * Description:			
 *
 * History:			08/02/2005	Created by Scott Silverman
 *
 *   Name    Date        Description
 *
 *****************************************************************************/

#ifndef LOCS_LIB_COMMON_H
#define LOCS_LIB_COMMON_H

#include <DataMgr.h>
#include <LibTraps.h>
#include <LocaleMgr.h>
#include <PalmTypes.h>
#include <SystemResources.h>
#include <SysUtils.h>


/******************************************************************************
 *
 *  LIBRARY VERSION
 *
 * The library version scheme follows the system versioning scheme.
 * See sysMakeROMVersion and friends in SystemMgr.h.
 *
 * For reference:
 *
 * 0xMMmfsbbb, where
 *      MM is major version,
 *      m is minor version,
 *      f is bug fix,
 *      s is stage: 3-release, 2-beta, 1-alpha, 0-development,
 *      bbb is build number for non-releases.
 *  e.g.:
 *      V1.12b3   would be: 0x01122003
 *      V2.00a2   would be: 0x02001002
 *      V1.01     would be: 0x01013000
 *
 *  Stages are:
 *      sysROMStageDevelopment
 *      sysROMStageAlpha
 *      sysROMStageBeta
 *      sysROMStageRelease
 *
 *  Note: This scheme was taken from the SampleLib.c of the Palm OS Dev KB
 *        download.
 *
 *****************************************************************************/

#define kLocsLibVersion01               sysMakeROMVersion (1, 0, 0, sysROMStageDevelopment, 1)
#define kLocsLibVersion0121             sysMakeROMVersion (1, 2, 1, sysROMStageDevelopment, 1)
#define kLocsLibVersion013              sysMakeROMVersion (1, 3, 0, sysROMStageDevelopment, 1)
#define kLocsLibVersionCurrent          kLocsLibVersion013


/******************************************************************************
 *
 *  LocsLib ERROR CODES
 *
 *****************************************************************************/
#define locLErrorClass                  (appErrorClass | 0x0100)

#define locLErrNone                     0
#define locLErrCantFindLocsDB           (locLErrorClass | 1)
#define locLErrBadParam                 (locLErrorClass | 2)
#define locLErrBadRefNum                (locLErrorClass | 3)
#define locLErrNoGlobals                (locLErrorClass | 4)
#define locLErrNoMemForGlobals          (locLErrorClass | 5)
#define locLErrNoLibSysReference        (locLErrorClass | 6)
#define locLErrCantOpenLibRsc           (locLErrorClass | 7)
#define locLErrMemAllocFail             (locLErrorClass | 8)
#define locLErrBadVersion               (locLErrorClass | 9)
#define locLErrNotOpen                  (locLErrorClass | 10)
#define locLErrStillOpen                (locLErrorClass | 11)


/******************************************************************************
 *
 *  LocsLib General CONSTANTS
 *
 *****************************************************************************/

// Use this for SysLibFind calls:
#define kLocsLibName                    "LocsLib-locL"

// The locations library creator ID:
//  Note: this is registered with Palm, Inc.
#define kLocsFileCLocsLib               (UInt32)('locL')

// The feature number used to refer to the stored library reference number:
#define kLocsLibFtrNumRefNum            (UInt16)(1)

// The notification types for location change events:
//  Note: used the creator id for the base event, as this seems to be what the
//        other notifcation event types are based on and it seems the best way
//        to assure no confusion.  Apps checking the types based on the first
//        type should also verify the broadcaster is the Locs lib.
#define kLocsLibNotifyLocChangeEvent    kLocsFileCLocsLib
#define kLocsLibNotifyLocEditEvent      kLocsLibNotifyLocChangeEvent + 1
#define kLocsLibNotifyLocDeleteEvent    kLocsLibNotifyLocChangeEvent + 2
#define kLocsLibNotifyLocAddEvent       kLocsLibNotifyLocChangeEvent + 3

// Masks for determing which bits in the selection dirty bits are set:
#define kLocsLibSel01                   0x00000001
#define kLocsLibSel02                   0x00000002
#define kLocsLibSel03                   0x00000004
#define kLocsLibSel04                   0x00000008
#define kLocsLibSel05                   0x00000010
#define kLocsLibSel06                   0x00000020
#define kLocsLibSel07                   0x00000040
#define kLocsLibSel08                   0x00000080
#define kLocsLibSel09                   0x00000100
#define kLocsLibSelMax                  0x80000000


/******************************************************************************
 *
 * Location CONSTANTS
 *
 ***********************************************************************/

// Max location name length:
#define kLocNameMaxLength           (UInt16)(100)

//Max location note length
#define kLocNoteMaxLength           (UInt16)(4096)

// Location custom member values:
#define kLocNoDisplayInCustom       (UInt8)(0)
#define kLocDisplayInCustom         (UInt8)(1)

//Undefined position
#define kUndefinedPosition			(Int16)999


/******************************************************************************
 *
 * LocsLibEditList CONSTANTS
 *
 ***********************************************************************/
#define kLocsLibListUpdateClass      0x0400
#define kLocsLibListUpdate           (UInt16)(kLocsLibListUpdateClass | 1)


/******************************************************************************
 *
 * LocationsDB CONSTANTS
 *
 ***********************************************************************/

#define kLocDBVersion               (UInt16)(2)
#define kLocDBMaxLocNameLength      kLocNameMaxLength   // Max # of bytes for a location name stored.

// Database names and types:
#define kLocFileCDatabase           (UInt32)('locL')
#define kLocFileTDefLocDB           (UInt32)('DATA')
#define kLocFileTCusLocDB           (UInt32)('DATC')
#define kLocFileNDefaultDB          "locLDefLocationDB"
#define kLocFileNCustomDB           "locLCusLocationDB"

// Location record reserve member values:
#define kLocDBTypeDefault           (UInt8)(0)
#define kLocDBTypeCustom            (UInt8)(1)

// Location database sort constants:
enum _LocDBSortOrderType {
    kSortByNameCountryTZ,
    kSortByCountryTZName,
    kSortByCountryCodeTZName
};

typedef UInt8 LocDBSortOrderType;

/******************************************************************************
 *
 * LocsLibDSTAlarm CONSTANTS
 *
 ***********************************************************************/

#define kLocsLibDSTAlert            (UInt32)(1)
#define kLocsLibDSTStartAlert       (UInt32)(2)
#define kLocsLibDSTEndAlert         (UInt32)(3)



/******************************************************************************
 *
 * Location OBJECT TYPES
 *  Important!!  Any changes made to the following location data types require
 *                a recompile of the DDEditor and a rebuild of the default db
 *                to match the new format.
 *
 ***********************************************************************/

// Daylight saving time (DST) data format:
//
typedef struct DSTDataTag DSTType;
struct DSTDataTag
{
    UInt8 hour;         // 1 - 24, usually 1 for 1 a.m.
    UInt8 dayOrd;       // 0 = Sun, 1 = Mon, 2 = Tues, 3 = Weds, 4 = Thurs, 5 = Fri, 6 = Sat.
    UInt8 weekOrd;      // 0 = 1st, 1 = 2nd, 2 = 3rd, 3 = 4th, 4 = Last
    UInt8 month;        // 1 - 12 = Jan - Dec
                        // No year member, as the DST date should be consistent
                        //  from year to year in regards to this "1st Sunday in
                        //  October" format used here. DST boundary alerts are
                        //  always set for the current year, or to the following
                        //  year as needed.
};

// Note: It's possible that DST adjustments may change in the future,
//        and at that time, a UI can be provided to set the proper adjustment,
//        or an updated default db can be distributed and installed.


// Location data format:
//

typedef struct{
				Int16	degree;
				Int16	minute;
			   }Coordinate;

typedef struct{
				Coordinate	latitude;
				Coordinate	longitude;
			   }PosType;

typedef struct LocationDataTag LocationType;
struct LocationDataTag
{
    Int16       uTC;
    DSTType     dSTStart;
    DSTType     dSTEnd;
    Int16       dSTAdjustmentInMinutes;  // 0 = DST not observed, 60 = observed.
    CountryType country;
    UInt8       custom   : 1,
                reserved : 7;
    Char*       name;					 // for db record type use name[];
    PosType		position;				 // Position (Longitude & Latitude) values
	Char*		note;                    // Note for each City
};


/******************************************************************************
 *
 * LocationsDB OBJECT TYPES
 *
 ***********************************************************************/

// Helpful struct for passing db info as an arg:
//
typedef struct LocDBInfoTag LocDBInfoType;
struct LocDBInfoTag
{
    Char*  dbName;
    UInt32 dbCreatorID;
    UInt32 dbType;
    UInt16 dbCardNum;
};


// Location database record format:
//
typedef struct LocDBRecordTag LocDBRecordType;
struct LocDBRecordTag
{
    Int16       uTC;
    DSTType     dSTStart;
    DSTType     dSTEnd;
    Int16       dSTAdjustmentInMinutes;  // 0 = DST not observed, 60 = observed.
    CountryType country;
    UInt8       custom   : 1,
                reserved : 7;
    Char        name[0];          // the LocationType uses Char* name;
};


/******************************************************************************
 *
 *  LocsLibUtils OBJECT TYPES
 *
 *****************************************************************************/

// Struct used to pass notification location data to interested apps:
typedef struct LocsLibNotifyLocsDataTag LocsLibNotifyLocsDataType;
struct LocsLibNotifyLocsDataTag
{
	LocationType locationDataNew;
	const Char*  locationNameOldP;
};


// Helpful struct for passing notification info as an arg:
//
typedef struct LocsLibNotifyDataTag LocsLibNotifyDataType;
struct LocsLibNotifyDataTag
{
    Boolean                   showWaitDialogue;    // If true, a "Please Wait" dialogue will be displayed while waiting for broadcast completion
    UInt32                    notifyType;          // One of the Locs lib notification type constants.
    LocsLibNotifyLocsDataType locationData;        // The location data to broadcast.
};


/******************************************************************************
 *
 *  LocsLib OBJECT TYPES
 *
 *****************************************************************************/

typedef struct LocsLibSelLocsTag LocsLibSelLocsType;
struct LocsLibSelLocsTag
{
	LocationType* locationP;          // Location data of the location selected from the Add Sel list. Caller must allocate the memory.
    UInt32        dirtySelections;    // indicates which selections were modified
                                      //  e.g.: the 1st selection, element 0, would be
                                      //        indicated by dirtySelections & kLocsLibSel01
    Boolean       needSelLoc;         // true if the caller wants a location name being chosen from the Add Sel list to be saved in the selLocsList.
    Boolean       warnIfLocExists;    // false if the caller doesn't want the user warned if selecting a location that already exists in the custom db.
    Boolean       noEditOnSelect;     // true if the caller wants the "Edit Location" dialogue to NOT be displayed after user selects a location; only applies to LocsLibLaunchSelDialogue().
    Boolean       preSelect;          // true if the caller wants the current location to be preselected in the Add Sel list; only applies to ADD/SEL location dialogue (shows master default list of locations).
    Boolean       sendChangeNotify;   // true if the caller wants the selected location data to be broadcast to all registered apps; only applies to LocsLibLaunchSelDialogue().
    Boolean       noAddToCustomDB;    // true if the caller doesn't want the selected location to be added to the custom db.
    UInt8         numSelLocs;         // Number of items in the selLocsList
    UInt8         reserved;
    Char**        selLocsListP;       // List of locations names that are "selected" and the caller needs to know if they are changed.
};



/******************************************************************************
 *
 * Location MACROS
 *
 ***********************************************************************/

// Accessor macros:
#define LocationGetUTC(locationP)          (locationP)->uTC
#define LocationGetDSTStart(locationP)     &((locationP)->dSTStart)
#define LocationGetDSTEnd(locationP)       &((locationP)->dSTEnd)
#define LocationGetDSTAdj(locationP)       (locationP)->dSTAdjustmentInMinutes
#define LocationGetCountry(locationP)      (locationP)->country
#define LocationGetCustom(locationP)       (locationP)->custom
#define LocationGetReserved(locationP)     (locationP)->reserved
#define LocationGetName(locationP)         (locationP)->name

// Mutator macros:
#define LocationSetUTC(locationP, newUTC) \
                   ((locationP)->uTC = newUTC)
#define LocationSetDSTStart(locationP, newDstStart) \
                   ((locationP)->dSTStart = newDstStart)
#define LocationSetDSTEnd(locationP, newDstEnd) \
                   ((locationP)->dSTEnd = newDstEnd)
#define LocationSetDSTAdj(locationP, newDstAdj) \
                   ((locationP)->dSTAdjustmentInMinutes = newDstAdj)
#define LocationSetCountry(locationP, newCountry) \
                   ((locationP)->country = newCountry)
#define LocationSetCustom(locationP, newCustom) \
                   ((locationP)->custom = newCustom)
#define LocationSetReserved(locationP, newReserved) \
                   ((locationP)->reserved = newReserved)
// Note: SetName is implemented as a function.

// Predicate macros:
#define LocationAreDSTDatesEqual(iDST01P, iDST02P) \
                   (   (iDST01P)->hour    == (iDST02P)->hour    \
                    && (iDST01P)->dayOrd  == (iDST02P)->dayOrd  \
                    && (iDST01P)->weekOrd == (iDST02P)->weekOrd \
                    && (iDST01P)->month   == (iDST02P)->month)

/******************************************************************************
 *
 * LocationsDB MACROS
 *
 ***********************************************************************/
#define LocDBCompareUTC(iUTC01, iUTC02) \
                (iUTC01 - iUTC02)
#define LocDBCompareCountries(iCountry01, iCountry02) \
                (iCountry01 - iCountry02)
#define LocDBCompareNames(iName01, iName02) \
                (StrCompare (iName01, iName02))

#define LocDBGetUTC(locRecP)          (locRecP)->uTC
#define LocDBGetCountry(locRecP)      (locRecP)->country

/******************************************************************************
 *
 *  LOCATIONS LIBRARY TRAP NUMBERS
 *
 *****************************************************************************/

// These are LocsLib's trap identifiers:
//  Note: The PalmOS constant 'sysLibTrapCustom' is the first trap number
//        that can be used after open, close, sleep, and wake:

#define    locsLibTrapGetVersion				(sysLibTrapCustom + 0)
#define    locsLibTrapLaunchEditListDlg			(sysLibTrapCustom + 1)
#define    locsLibTrapLaunchSelDialogue			(sysLibTrapCustom + 2)
#define    locsLibTrapLocCopy					(sysLibTrapCustom + 3)
#define    locsLibTrapLocFinal					(sysLibTrapCustom + 4)
#define    locsLibTrapLocGetSize				(sysLibTrapCustom + 5)
#define    locsLibTrapLocInit					(sysLibTrapCustom + 6)
#define    locsLibTrapLocIsDSTPast				(sysLibTrapCustom + 7)
#define    locsLibTrapLocMakeGeneric			(sysLibTrapCustom + 8)
#define    locsLibTrapLocSetName				(sysLibTrapCustom + 9)
#define    locsLibTrapDBChangeRecord			(sysLibTrapCustom + 10)	
#define    locsLibTrapDBCmpRecordsSimple		(sysLibTrapCustom + 11)
#define    locsLibTrapDBConvertRecord			(sysLibTrapCustom + 12)
#define    locsLibTrapDBCopyLocation			(sysLibTrapCustom + 13)
#define    locsLibTrapDBDeleteRecord			(sysLibTrapCustom + 14)
#define    locsLibTrapDBFindCustomRecord		(sysLibTrapCustom + 15)
#define    locsLibTrapDBFindFirst				(sysLibTrapCustom + 16)
#define    locsLibTrapDBFindFirstByName			(sysLibTrapCustom + 17)
#define    locsLibTrapDBFindFirstByUTCCtry		(sysLibTrapCustom + 18)
#define    locsLibTrapDBFindFirstInList			(sysLibTrapCustom + 19)
#define    locsLibTrapDBGetCustomList			(sysLibTrapCustom + 20)
#define    locsLibTrapDBGetRecordList			(sysLibTrapCustom + 21)
#define    locsLibTrapDBGetRecord				(sysLibTrapCustom + 22)
#define    locsLibTrapDBInitDBs					(sysLibTrapCustom + 23)
#define    locsLibTrapDBNewRecord				(sysLibTrapCustom + 24)
#define    locsLibTrapDBOpenCustom				(sysLibTrapCustom + 25)
#define    locsLibTrapDBOpenDefault				(sysLibTrapCustom + 26)
#define    locsLibTrapDBRecordSize				(sysLibTrapCustom + 27)
#define    locsLibTrapDBSetRecordCustom			(sysLibTrapCustom + 28)
#define    locsLibTrapDBSort					(sysLibTrapCustom + 29)
#define    locsLibTrapDSTAlarmClear				(sysLibTrapCustom + 30)
#define    locsLibTrapDSTAlarmIsDST				(sysLibTrapCustom + 31)
#define    locsLibTrapDSTAlarmSet				(sysLibTrapCustom + 32)
#define    locsLibTrapLocDBNewRecordV20			(sysLibTrapCustom + 33)
#define    locsLibTrapLocDBGetRecordV20			(sysLibTrapCustom + 34)
#define    locsLibTrapLocDBConvertRecordV20		(sysLibTrapCustom + 35)
#define    locsLibTrapLocDBChangeRecordV20		(sysLibTrapCustom + 36)
#define    locsLibTrapLocDBGetRecordListV20		(sysLibTrapCustom + 37)
#define    locsLibTrapLocDBGetNearestLocation	(sysLibTrapCustom + 38)
#define    locsLibTrapLocationInitV20			(sysLibTrapCustom + 39)
#define    locsLibTrapLocationFinalV20			(sysLibTrapCustom + 40)
#define    locsLibTrapLocDBGetVersion			(sysLibTrapCustom + 41)
#define    locsLibTrapLocationSetNote			(sysLibTrapCustom + 42)
#define    locsLibTrapLocationSetPosition		(sysLibTrapCustom + 43)
#define    locsLibTrapLocsLibLocationNote		(sysLibTrapCustom + 44)
#define    locsLibTrapLocationGetSizeV20		(sysLibTrapCustom + 45)
#define    locsLibTrapLocationCopyV20			(sysLibTrapCustom + 46)
#define    locsLibTrapLocDBFindCustomRecordV20	(sysLibTrapCustom + 47)


#endif