/******************************************************************************
 *
 * Copyright (c) 2000 Palm Computing, Inc. or its subsidiaries.
 * All rights reserved.
 *
 *****************************************************************************/

/******************************************************************************
 *
 * File:    LocsLib.h
 *
 * Description:
 *          Shared library functionality interface definition.
 *
 * History:
 *			Name	Date		Description
 *			----	----		-----------
 *          scs     2001.11.13  Copied and modified from MySharedLib.h
 *                               from Palm OS Dev KB article ID 1670.
 *
 *****************************************************************************/


#ifndef _LOCSLIB_H_
#define _LOCSLIB_H_

// PalmSource includes:
#include <DataMgr.h>
#include <LibTraps.h>
#include <LocaleMgr.h>
#include <PalmTypes.h>
#include <SystemResources.h>
#include <SysUtils.h>
#include <Common/Libraries/locs/LocsLibCommon.h>


/******************************************************************************
 *
 *  PROTOTYPES
 *
 *****************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif


// Required entry points:
extern Err               LocsLibOpen                   (UInt16 iLibRefNum)
    SYS_TRAP (sysLibTrapOpen);
extern Err               LocsLibClose                  (UInt16 iLibRefNum)
    SYS_TRAP (sysLibTrapClose);
extern Err               LocsLibSleep                  (UInt16 iLibRefNum)
    SYS_TRAP (sysLibTrapSleep);
extern Err               LocsLibWake	               (UInt16 iLibRefNum)
    SYS_TRAP (sysLibTrapWake);

// LocsLib general public custom functions (library extends to callers):
extern Err               LocsLibGetVersion             (UInt16 iLibRefNum, UInt32 iSDKVersion, UInt32* oVersionP)
    SYS_TRAP (locsLibTrapGetVersion);
extern Boolean           LocsLibLaunchEditListDialogue (UInt16 iLibRefNum, LocsLibSelLocsType* ioSelLocsP)
    SYS_TRAP (locsLibTrapLaunchEditListDlg);
extern Boolean           LocsLibLaunchSelDialogue      (UInt16 iLibRefNum, LocsLibSelLocsType* ioSelLocsP)
    SYS_TRAP (locsLibTrapLaunchSelDialogue);

// Location public custom functions:
extern void              LocationCopy                  (UInt16 iLibRefNum, LocationType* thisP, const LocationType* iLocToCopyP)
    SYS_TRAP (locsLibTrapLocCopy);
extern void              LocationFinal                 (UInt16 iLibRefNum, LocationType* thisP)
    SYS_TRAP (locsLibTrapLocFinal);
extern UInt16            LocationGetSize               (UInt16 iLibRefNum, const LocationType* thisP)
    SYS_TRAP (locsLibTrapLocGetSize);
extern void              LocationInit                  (UInt16 iLibRefNum, LocationType* thisP)
    SYS_TRAP (locsLibTrapLocInit);
extern Boolean           LocationIsDSTPast             (UInt16 iLibRefNum, DSTType* iDstDateP)
    SYS_TRAP (locsLibTrapLocIsDSTPast);
extern Err               LocationMakeGeneric           (UInt16 iLibRefNum, LocationType* thisP, CountryType iCountryCode, Int16 iUTC)
    SYS_TRAP (locsLibTrapLocMakeGeneric);
extern void              LocationSetName               (UInt16 iLibRefNum, LocationType* thisP, const Char* iNameP)
    SYS_TRAP (locsLibTrapLocSetName);

// LocationsDB public custom functions:
extern Err               LocDBChangeRecord          (UInt16 iLibRefNum, DmOpenRef dbP, UInt16* ioRecordIndexP, const LocationType* iLocationP)
    SYS_TRAP (locsLibTrapDBChangeRecord);
extern Int16             LocDBCompareRecordsSimple  (UInt16 iLibRefNum, LocDBRecordType** iRecord01P, LocDBRecordType** iRecord02P, Int32 iFlags)
    SYS_TRAP (locsLibTrapDBCmpRecordsSimple);
extern void              LocDBConvertRecord         (UInt16 iLibRefNum, LocationType* ioLocationP, const LocDBRecordType* iRecordP)
    SYS_TRAP (locsLibTrapDBConvertRecord);
extern void              LocDBCopyLocation          (UInt16 iLibRefNum, LocDBRecordType* ioRecordP, const LocationType* iLocationP)
    SYS_TRAP (locsLibTrapDBCopyLocation);
extern Err               LocDBDeleteRecord          (UInt16 iLibRefNum, DmOpenRef dbP, UInt16 iRecordIndex)
    SYS_TRAP (locsLibTrapDBDeleteRecord);


extern LocDBRecordType*  LocDBFindCustomRecord      (UInt16 iLibRefNum, DmOpenRef iDbP, UInt16* ioRecordIndex, MemHandle* oRecordH)
    SYS_TRAP (locsLibTrapDBFindCustomRecord);

extern Err               LocDBFindFirst             (UInt16 iLibRefNum, DmOpenRef dbP, UInt16* oRecordIndexP, LocDBRecordType* iMatchValueP)
    SYS_TRAP (locsLibTrapDBFindFirst);
extern Err               LocDBFindFirstByName       (UInt16 iLibRefNum, DmOpenRef dbP, UInt16* oRecordIndexP, const Char* iNameP)
    SYS_TRAP (locsLibTrapDBFindFirstByName);
extern Err               LocDBFindFirstByUTCCountry (UInt16 iLibRefNum, DmOpenRef dbP, UInt16* oRecordIndexP, CountryType iCountry, Int16 iUTC)
    SYS_TRAP (locsLibTrapDBFindFirstByUTCCtry);
extern UInt16            LocDBFindFirstInList       (UInt16 iLibRefNum, UInt16 iLocsListItemsCount, LocDBRecordType** iLocsListP, const LocationType* iMatchLocationP)
    SYS_TRAP (locsLibTrapDBFindFirstInList);
extern MemHandle         LocDBGetCustomList         (UInt16 iLibRefNum, DmOpenRef iDbP, UInt16* oListCount)
    SYS_TRAP (locsLibTrapDBGetCustomList);
extern LocDBRecordType** LocDBGetRecordList         (UInt16 iLibRefNum, DmOpenRef iDbP, UInt16* oListCount, CmpFuncPtr iCompareF, Int32 iCompareFlags)
    SYS_TRAP (locsLibTrapDBGetRecordList);
extern LocDBRecordType*  LocDBGetRecord             (UInt16 iLibRefNum, DmOpenRef dbP, UInt16 iRecordIndex, MemHandle* oRecordHP)
    SYS_TRAP (locsLibTrapDBGetRecord);
extern Err               LocDBInitDBs               (UInt16 iLibRefNum, DmOpenRef* oDefDbPP, DmOpenRef* oCusDbPP)
    SYS_TRAP (locsLibTrapDBInitDBs);
extern Err               LocDBNewRecord             (UInt16 iLibRefNum, DmOpenRef dbP, UInt16* oRecordIndexP, const LocationType* iLocationP)
    SYS_TRAP (locsLibTrapDBNewRecord);
extern LocalID           LocDBOpenCustom            (UInt16 iLibRefNum, const LocDBInfoType* iDbInfoP, UInt16 iOpenMode, DmOpenRef* oDbPP, const DmOpenRef iDefDbP)
    SYS_TRAP (locsLibTrapDBOpenCustom);
extern LocalID           LocDBOpenDefault           (UInt16 iLibRefNum, DmOpenRef* oDbPP)
    SYS_TRAP (locsLibTrapDBOpenDefault);
extern UInt16            LocDBRecordSize            (UInt16 iLibRefNum, const LocDBRecordType* iLocRecordP)
    SYS_TRAP (locsLibTrapDBRecordSize);
extern Err               LocDBSetRecordCustom       (UInt16 iLibRefNum, LocDBRecordType* ioLocRecordP, UInt8 iValue)
    SYS_TRAP (locsLibTrapDBSetRecordCustom);
extern Err               LocDBSort                  (UInt16 iLibRefNum, DmOpenRef iDbP, LocDBSortOrderType iSortOrder)
    SYS_TRAP (locsLibTrapDBSort);

// LocsLibDSTAlarm public functions:
extern void              LocsLibDSTAlarmClear       (UInt16 iLibRefNum)
    SYS_TRAP (locsLibTrapDSTAlarmClear);
extern Boolean           LocsLibDSTAlarmIsDST       (UInt16 iLibRefNum, const DSTType* iDSTStartP, const DSTType* iDSTEndP, UInt32 iTimeInSecs)
    SYS_TRAP (locsLibTrapDSTAlarmIsDST);
extern UInt32            LocsLibDSTAlarmSet         (UInt16 iLibRefNum, const DSTType* iDstStartP, const DSTType* iDstEndP)
    SYS_TRAP (locsLibTrapDSTAlarmSet);


extern Err			      LocDBNewRecordV20         (UInt16 iLibRefNum, DmOpenRef dbP, UInt16* oRecordIndexP, LocationType* iLocationP)
    SYS_TRAP (locsLibTrapLocDBNewRecordV20);
extern LocationType*  LocDBGetRecordV20       		(UInt16 iLibRefNum, DmOpenRef dbP, UInt16 iRecordIndex, MemHandle* oRecordHP)
    SYS_TRAP (locsLibTrapLocDBGetRecordV20);
extern void              LocDBConvertRecordV20      (UInt16 iLibRefNum, LocationType* destRecordP, const LocationType* srcRecordP)
    SYS_TRAP (locsLibTrapLocDBConvertRecordV20);
extern Err               LocDBChangeRecordV20       (UInt16 iLibRefNum, DmOpenRef dbP, UInt16* ioRecordIndexP, LocationType* iLocationP)
    SYS_TRAP (locsLibTrapLocDBChangeRecordV20);
extern LocationType** LocDBGetRecordListV20   		(UInt16 iLibRefNum, DmOpenRef iDbP, UInt16* oListCount, CmpFuncPtr iCompareF, Int32 iCompareFlags)
    SYS_TRAP (locsLibTrapLocDBGetRecordListV20);
extern Err 			LocDBGetNearestLocation			(UInt16 iLibRefNum,DmOpenRef iDb1P,DmOpenRef iDb2P,PosType* iPosP,LocationType* oLocP)
    SYS_TRAP (locsLibTrapLocDBGetNearestLocation);
extern void              LocationInitV20            (UInt16 iLibRefNum, LocationType* thisP)
    SYS_TRAP (locsLibTrapLocationInitV20);
extern void              LocationFinalV20           (UInt16 iLibRefNum, LocationType* thisP)
    SYS_TRAP (locsLibTrapLocationFinalV20);
extern Err               LocDBGetVersion            (UInt16 iLibRefNum, UInt16 cardNo, const Char *dbNameP, UInt16 *version)
    SYS_TRAP (locsLibTrapLocDBGetVersion);
extern void              LocationSetNote            (UInt16 iLibRefNum, LocationType* thisP, const Char* iNameP)
    SYS_TRAP (locsLibTrapLocationSetNote);
extern void              LocationSetPosition        (UInt16 iLibRefNum, LocationType* thisP, const PosType* iPosP)
    SYS_TRAP (locsLibTrapLocationSetPosition);
extern Boolean		         LocsLibLocationNote        (UInt16 iLibRefNum, const Char* iNameP, Boolean noteStatus)
    SYS_TRAP (locsLibTrapLocsLibLocationNote);
extern UInt16           LocationGetSizeV20          (UInt16  iLibRefNum,  LocationType* locationP)
    SYS_TRAP (locsLibTrapLocationGetSizeV20);
extern void              LocationCopyV20            (UInt16 iLibRefNum, LocationType* thisP, const LocationType* iLocToCopyP)
    SYS_TRAP (locsLibTrapLocationCopyV20);
extern LocationType*  LocDBFindCustomRecordV20      	(UInt16 iLibRefNum, DmOpenRef iDbP, UInt16* ioRecordIndex, MemHandle* oRecordH)
    SYS_TRAP (locsLibTrapLocDBFindCustomRecordV20);


#ifdef __cplusplus
}
#endif



#endif


/******************************************************************************
 *
 * END LocsLib.h
 *
 *****************************************************************************/
