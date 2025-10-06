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

#ifndef DATEBOOK_LIB_COMMON_H
#define DATEBOOK_LIB_COMMON_H

#include <PalmTypes.h>
#include <Form.h>
#include <Find.h>
#include <PdiLib.h>

#include <Common/Libraries/locs/LocsLibCommon.h>
//#include <palmOne_68KInt.h>


// Type and creator of P1cl Library 
#define P1clLibCreatorID				'P1CL'
#define P1clTypeID						sysFileTLibrary
#define	P1clLibName						"CalendarLib-P1CL"


//	SDS	-	First initial release			0x0100
//	SDS	-	Bug Fix release for 2005.1		0x0101
//	SDS	-	
#define		P1cl_CURRENT_VERSION			0x0200


#define apptMaxTimeZoneNameLen				kLocNameMaxLength
#define SysNotifyPIMRecordChange			'pmdb'		// notifyType that one of the PIM dbs changed


/********************************************************************
 * Error codes
 ********************************************************************/
#define P1clErrorClass					(appErrorClass |0x7000)
#define	P1clErrTooManyClients			(P1clErrorClass | 1)	//No More client can open the lib
#define	P1clErrInvalidHandle			(P1clErrorClass | 2)
#define	P1clErrInvalidParam				(P1clErrorClass | 3)	
#define	P1clErrUIdNotFound				(P1clErrorClass | 4)	
#define	P1clErrCategoryNotFound			(P1clErrorClass | 5)	
#define	P1clErrAlreadyInUse 			(P1clErrorClass | 6)	
#define	P1clErrVerNotSupported			(P1clErrorClass | 7)	
#define	P1clErrOutOfMemory				(P1clErrorClass | 8)	
#define	P1clErrSizeParamOverFlow		(P1clErrorClass | 9)	
#define	P1clErrEmptyDateBook			(P1clErrorClass | 10)	
#define	P1clErrAlarmDoesNotExist		(P1clErrorClass | 11)	
#define	P1clErrRepeatInfoDoesNotExist	(P1clErrorClass | 12)
#define	P1clErrDescDoesNotExist			(P1clErrorClass | 13)
#define	P1clErrNoteDoesNotExist			(P1clErrorClass | 14)
#define	P1clErrInvalidReference			(P1clErrorClass | 15)
#define	P1clErrApptNotFound				(P1clErrorClass | 16)
#define	P1clErrAttachAlreadyExist		(P1clErrorClass | 17)		//Attachment cannot be added it already exist in the record.
#define	P1clErrAttachNotExist			(P1clErrorClass | 18)		
#define	P1clErrAttachOverFlow			(P1clErrorClass | 19)		
#define	P1clErrRecordNotFound			(P1clErrorClass | 21)		
#define	P1clErrAttachOverflow			(P1clErrorClass | 22)		
#define	P1clErrInvalidField				(P1clErrorClass | 23)	
#define	P1clErrNoDateTime				(P1clErrorClass | 24)	
#define	P1clErrLibAlreadyInUse			(P1clErrorClass | 25)	
#define	P1clErrTooManySession			(P1clErrorClass | 26)	
#define	P1clErrAlreadyInWriteMode		(P1clErrorClass | 27)	//User have requested to create Session in ReadWriteMode but it is already open.
#define	P1clErrInvalidSession			(P1clErrorClass | 28)	
#define	P1clErrInvalidAtrib				(P1clErrorClass | 29)	
#define	P1clErrFieldNotFound			(P1clErrorClass | 30)
#define	P1clErrStringNotFound			(P1clErrorClass | 31)
#define	P1clErrInvalidAppInfoBlock		(P1clErrorClass | 32)
#define	P1clErrAppInfoIsMising			(P1clErrorClass | 33)
#define	P1clErrReadOnlySession			(P1clErrorClass | 34)
#define	P1clErrUIdExist					(P1clErrorClass | 35)
#define	P1clErrMaxLimitReached			(P1clErrorClass | 36)
#define	P1clErrListNotExist				(P1clErrorClass | 37)
#define	P1clErrApptNotLocked			(P1clErrorClass | 38)
#define	P1clErrApptBusy					(P1clErrorClass | 39)
#define	P1clErrEmptyAppt				(P1clErrorClass | 40)
#define	P1clErrFieldNotExist			(P1clErrorClass | 41)
#define	P1clErrRepeatTypeDoesNotExist	(P1clErrorClass | 42)
#define	P1clErrAttendeeNotFound			(P1clErrorClass | 43)
#define P1clErrAttendeeOutofLimit		(P1clErrorClass | 44)		
#define P1clErrLibStillOpen				(P1clErrorClass | 45)


typedef	struct	
{
	UInt8		Action;						//Action can be ADD/Delete/Update
	UInt32		RecordUId;					//UID of the Record that is effected
	UInt16		RecordIndex;				//Index of the record after that all the records are effeced.
	UInt32		CreatorId;					//Creator Id of the caller application.
}	P1clRecordDetails;

typedef P1clRecordDetails * P1clRecordDetailsPtr;


enum _P1clListFieldType
{
	P1clListStartTime,
	P1clListEndTime,
	P1clListStartSec,
	P1clListEndSec,	
	P1clListStartedDayBefore,
	P1clListContinuesOnNextDay,
	P1clListRepeatsDaily,
	P1clListIsFirstOccurrence,
	P1clListIsLastOccurrence,
	P1clListUId,
	P1clListIndex
};

typedef UInt16 P1clListFieldType;


enum _P1clTimeZoneType
{
	P1clDeviceTimeZone,		//Current Time Zone of the Device 
	P1clEventTimeZone		//Time zone of the Event. I.e. Time zone of the event when it was created/updated.
};

typedef UInt16 P1clTimeZoneType;


enum _P1clActionType
{
	P1clAdd, 
	P1clUpdate, 
	P1clDelete
};

typedef UInt16 P1clActionType;


enum _P1clAlarmUnitType
{
	P1clMinutes, 
	P1clHours, 
	P1clDays
};

typedef UInt8 P1clAlarmUnitType;


enum _P1clRepeatTypes
{
	P1clrepeatNone, 
	P1clrepeatDaily, 
	P1clrepeatWeekly, 
	P1clrepeatMonthlyByDay, 
	P1clrepeatMonthlyByDate,
	P1clrepeatYearly
};

typedef UInt8 P1clRepeatTypes;


enum _P1clFieldsTypes
{
	P1clStartTime,
	P1clEndTime,
	P1clDateOfAppt,
	P1clrepeatEndDate,
	P1clrepeatFreq,
	P1clrepeatOn,
	P1clrepeatStartofWeek,
	P1clrepeatType,
	P1clalarmAdvance,
	P1clalarmUnit,
	P1clNote,
	P1clDescription,
	P1clLocation,
	P1clExceptionDate,
	P1clNumOfAttendee,
	P1clStartDateTimeOfAppt,					//Date and time of the appointment in Sec elapsed from 12:00AM Jan 1, 1904
	P1clUID,							//Place for the attributes These will be after P1ctaddressFieldsCount
	P1clIndex,
	P1clCategory,
	P1clColor,
	P1clPrivate,
	P1clBusy,
	P1clDirty,
	P1clTzuTC,
	P1clTzdSTStart,
	P1clTzdSTEnd,		
	P1clTzdSTAdjustmentInMinutes,
	P1clTzCountry,
	P1clTzcustom,
	P1clTzLocName,
	P1clApptStatus,
	P1clNumFields
};

typedef UInt16 P1clFieldTypes;


enum _P1clDeleteType
{
	P1clNoType,						//This will be true if the Appointment is non Repeat event
	P1clDeleteCurrent,				//This will indicate delete of the event for the current date(Date passed as param to the P1clDeleteAppt)
	P1clDeleteCurrentFuture,		//It will indicate the delete of all the future event of the Repeating event 
	P1clDeleteAll,					//It will indicate the delete of all the event of the Repeating event 
	P1clRemoveAll					//It will Remove the record from the DB same as DmRemoveRecord ()
	
};

typedef UInt16 P1clDeleteType;


enum _P1clAttachType
{
	P1clCreatorIDType,
	P1clBlobSizeType,
	P1clDataBlobType	
};

typedef UInt16 AttachType;


enum _P1clAttendeeRoleType
{
	P1cloriginator,					// The person who set up the meeting.
	P1clrequiredAttendee,			// An attendee that the originator feels is required.
	P1cloptionalAttendee
};

typedef UInt16 P1clAttendeeRoleType;


enum _P1clAttendeeFieldType
{
	P1clAttendeeRole,			// The person who set up the meeting.
	P1clAttendeeName,			// An attendee that the originator feels is required.
	P1clAttendeeEmail
};

typedef UInt16 P1clAttendeeFieldType;


enum _P1clTxnEntry
{
	P1clNewEntry,
	P1clModifyEntry,
	P1clDeleteEntry
 };
 
typedef UInt16 P1clTxnEntry;


// P1clApptStatus values set/getable via P1clSetUInt8Field() or P1clGetUInt8Field
enum  _P1clApptStatusType
{
	P1clShowAsBusy,					// (default) Show as busy to strongly discourage conflicts.
	P1clShowAsFree,					// Allow others to schedule meetings that conflict with this one.
	P1clShowAsTentative,			// Show as tentative to discourage conflicts.
	P1clShowAsOutOfOffice			// Show as out of office to even more strongly discourage conflicts.
};

typedef UInt16 P1clApptStatusType;


// Data from the time zone blob.
// Event time zones are similar to locations library locations.
// If no time zone blob is present, name will be NULL. Typically only some events will have a time zone blob.
// If no time zone blob is present, the event is treated as being in the current device time zone.
typedef struct {
	Int16				uTC;									// Offset from UTC in minutes.
	DSTType				dSTStart;								// When daylight saving time starts (if observed).
	DSTType				dSTEnd;									// When daylight saving time ends (if observed).
	Int16				dSTAdjustmentInMinutes;					// 0 = DST not observed, 60 = observed.
	CountryType			country;								// Which country this region is in.
	UInt8				custom   : 1,							// Whether this location was created by the user.
						reserved : 7;							// Usused.
	Char				name[apptMaxTimeZoneNameLen + 1];			// Name of location (typically city). Not a pointer!
} ApptTimeZoneType;

/* *********************** Field Types
P1clListStartTime			TimeType			Start time of the event. This time may be different than the event Start time if the device is in different time zone than the event. 	

P1clListEndTime				TimeType 			End time of the event. This time may be different than the event End Time if the device is in different time zone than the event.	
P1clListStartSec			UInt32				Start Sec after Since 1904.	
P1clListEndSec				UInt32				End Sec after Since 1904.	
P1clListStartedDayBefore	UInt16				Spans midnight at start of day.	
P1clListContinuesOnNextDay	UInt16				Spans or ends at midnight end of day.	
P1clListRepeatsDaily		UInt16				Part of a daily repeating event.	
P1clListIsFirstOccurrence	UInt16				Part or first occurrence of repeating event.	
P1clListIsLastOccurrence	UInt16				Part of last occurrence of repeating event.	
P1clListUId					UInt32				UId of the Event	
P1clListIndex				UInt16				Index of the Event	
P1clStartTime				TimeType			Start time of the event. This time may be different than the event Start time if the device is in different time zone than the event. 	
P1clEndTime					TimeType			End time of the event. This time may be different than the event End Time if the device is in different time zone than the event.	TimeType
P1clDateOfAppt				DateType			Date of the Event.	
P1clrepeatEndDate			DateType			Repeating End Date of the Event.	
P1clrepeatFreq				UInt8				Frequency of the Repeating event.	
P1clrepeatOn				UInt8				This is valid for monthly by day and repeat Weekly.	
P1clrepeatStartofWeek		UInt8				Part of a daily repeating event.	
P1clrepeatType				UInt8				Part or first occurrence of repeating event.	
P1clalarmAdvance			Int8				Alarm advance the value will be -1 if no alarm is set.	
P1clalarmUnit				UInt8				Unit of the alarm Minute, hours, day	
P1clNote					String(char*)		Notes attached to the event.	
P1clDescription				String(char*)		Description of the event.	
P1clLocation				String(char*)		Location of the event. 	
P1clExceptionDate			Date				Which date in the repeating event is exception.	
P1clNumOfAttendee			UInt16				Number of attendee included in the event	
P1clStartDateTimeOfAppt		UInt32				Date and time of the appointment in Sec elapsed from 12:00AM Jan 1, 1904	
P1clUID						UInt32				UId of the Event in Calendar DB.	
P1clIndex					UInt16				Index of the Event in Calendar DB.	
P1clCategory				UInt16				Category of the Event.	
P1clColor					UInt16				Color of the Event.	
P1clPrivate					UInt8				Private attribute of the Event	
P1clBusy					UInt8				Event is Busy or not. This refer to the busy in Data Manger term.	UInt8
P1clDirty					UInt8				Event is Dirty or not. This refer to the busy in Data Manger term.	UInt8
P1clTzuTC					Int16				Offset from UTC in minutes.	
P1clTzdSTStart				DSTType				When daylight saving time starts (if observed).	
P1clTzdSTEnd				DSTType				When daylight saving time ends (if observed).	
P1clTzdSTAdjustmentInMinutes	Int16			0 = DST not observed, 60 = observed.	
P1clTzCountry				CountryType			Which country this region is in.	
P1clTzcustom				UInt8				Whether this location was created by the user.	
P1clTzLocName				Char*				Name of location (typically city). 	


*/

typedef MemHandle		P1clApptHandle;
typedef MemHandle		P1clAttachHandle;
typedef MemHandle		P1clApptListHandle;
typedef	MemHandle		P1clSession;
typedef	MemHandle		P1clAttendeeHandle;			//Store info about the Attendee
typedef	MemHandle		P1clTxnIterator;
typedef UInt32 			P1clSyncSource;
typedef UInt32 			P1clPersistentID;

/*
	Unique id that represents a data source Id. Each SAM and SAS will need to provide a unique id for 
	the data source that it is syncing with. Note that in the case where a SAM and SAS are
	syncing to the same data source (i.e. the same exchange server) they must use the same Data source id. 
	For HotSyncing with Palm Desktops the data source id equates to the PCID used by HotSync
*/
typedef UInt32		IDatSyncSource;				//Data Source Id

typedef Boolean	(* DateBookCompAttachFucPtr) (void*	pSource, void *SearchString);


	#define P1clTrapGetVersion						(sysLibTrapCustom)	
	#define P1clTrapCheckVersion					(sysLibTrapCustom + 1)	
	#define P1clTrapOpenSession						(sysLibTrapCustom + 2)	
	#define P1clTrapCloseSession					(sysLibTrapCustom + 3)														
	#define P1clTrapCreateHandle					(sysLibTrapCustom + 4)					
	#define P1clTrapReleaseHandle					(sysLibTrapCustom + 5)	
	#define P1clTrapGetAppt							(sysLibTrapCustom + 6)	
	#define P1clTrapGetFirstAppt					(sysLibTrapCustom + 7)		
	#define P1clTrapGetNextAppt						(sysLibTrapCustom + 8)	
	#define P1clTrapIsEmptyAppt						(sysLibTrapCustom + 9)	
	#define P1clTrapSplitRepeatingEvent				(sysLibTrapCustom + 10)		
	#define P1clTrapApptFindNextRepeat				(sysLibTrapCustom + 11)				
	#define P1clTrapApptRepeatsOnDate				(sysLibTrapCustom + 12)
	#define P1clTrapHasMultipleOccurences			(sysLibTrapCustom + 13)	
	#define P1clTrapGetApptList						(sysLibTrapCustom + 14)	
	#define P1clTrapGetUInt16FromListHandle			(sysLibTrapCustom + 15)	
	#define P1clTrapGetTimeFromListHandle			(sysLibTrapCustom + 16)					
	#define P1clTrapGetUInt32FromListHandle			(sysLibTrapCustom + 17)
	#define P1clTrapGetBooleanFromListHandle		(sysLibTrapCustom + 18)								
	#define P1clTrapGetApptFromListHandle			(sysLibTrapCustom + 19)										
	#define P1clTrapUpdateAppt						(sysLibTrapCustom + 20)	
	#define P1clTrapAddAppt							(sysLibTrapCustom + 21)	
	#define P1clTrapDeleteAppt						(sysLibTrapCustom + 22)
	#define P1clTrapFindNextAlarm					(sysLibTrapCustom + 23)	
	#define P1clTrapSortCalendar					(sysLibTrapCustom + 24)
	#define P1clTrapFindSaveMatch					(sysLibTrapCustom + 25)		
	#define P1clTrapPurgeAppts						(sysLibTrapCustom + 26)
	#define P1clTrapAttnForgetIt					(sysLibTrapCustom + 27)		
	#define P1clTrapApptPostTriggeredAlarms			(sysLibTrapCustom + 28)
	#define P1clTrapGetCalendarDBInfo				(sysLibTrapCustom + 29)	
	#define P1clTrapCreateException					(sysLibTrapCustom + 30)
	#define P1clTrapDeleteExceptionList				(sysLibTrapCustom + 31)	
	#define P1clTrapFindConflict					(sysLibTrapCustom + 32)
	#define P1clTrapGetStringField					(sysLibTrapCustom + 33)		
	#define P1clTrapSetStringField					(sysLibTrapCustom + 34)	
	#define P1clTrapGetUInt8Field					(sysLibTrapCustom + 35)	
	#define P1clTrapSetUInt8Field					(sysLibTrapCustom + 36)	
	#define P1clTrapGetInt8Field					(sysLibTrapCustom + 37)	
	#define P1clTrapSetInt8Field					(sysLibTrapCustom + 38)		
	#define P1clTrapGetUInt16Field					(sysLibTrapCustom + 39)	
	#define P1clTrapSetUInt16Field					(sysLibTrapCustom + 40)	
	#define P1clTrapGetInt16Field					(sysLibTrapCustom + 41)	
	#define P1clTrapSetInt16Field					(sysLibTrapCustom + 42)	
	#define P1clTrapGetUInt32Field					(sysLibTrapCustom + 43)	
	#define P1clTrapSetUInt32Field					(sysLibTrapCustom + 44)	
	#define P1clTrapGetDateTimeField				(sysLibTrapCustom + 45)	
	#define P1clTrapSetDateTimeField				(sysLibTrapCustom + 46)	
	#define P1clTrapGetTimeField					(sysLibTrapCustom + 47)	
	#define P1clTrapSetTimeField					(sysLibTrapCustom + 48)	
	#define P1clTrapGetDateField					(sysLibTrapCustom + 49)	
	#define P1clTrapSetDateField					(sysLibTrapCustom + 50)	
	#define P1clTrapGetDSTField						(sysLibTrapCustom + 51)
	#define P1clTrapSetDSTField						(sysLibTrapCustom + 52)
	#define P1clTrapGetNumOfException				(sysLibTrapCustom + 53)	
	#define P1clTrapIsException						(sysLibTrapCustom + 54)
	#define P1clTrapAddExceptionDate				(sysLibTrapCustom + 55)	
	#define P1clTrapCreateAttendeeHandle			(sysLibTrapCustom + 56)	
	#define P1clTrapReleaseAttendeeHandle			(sysLibTrapCustom + 57)	
	#define P1clTrapGetFirstAttendee				(sysLibTrapCustom + 58)	
	#define P1clTrapGetNextAttendee					(sysLibTrapCustom + 59)	
	#define P1clTrapGetAttendee						(sysLibTrapCustom + 60)	
	#define P1clTrapAddAttendee						(sysLibTrapCustom + 61)	
	#define P1clTrapUpdateAttendee					(sysLibTrapCustom + 62)	
	#define P1clTrapDeleteAttendee					(sysLibTrapCustom + 63)	
	#define P1clTrapGetAttendeeUInt8Field			(sysLibTrapCustom + 64)	
	#define P1clTrapSetAttendeeUInt8Field			(sysLibTrapCustom + 65)	
	#define P1clTrapGetAttendeeStringField			(sysLibTrapCustom + 66)	
	#define P1clTrapSetAttendeeStringField			(sysLibTrapCustom + 67)	
	#define P1clTrapCreateBlobHandle				(sysLibTrapCustom + 68)	
	#define P1clTrapReleaseBlobHandle				(sysLibTrapCustom + 69)	
	#define P1clTrapGetBlob							(sysLibTrapCustom + 70)	
	#define P1clTrapSetBlob							(sysLibTrapCustom + 71)			
	#define P1clTrapDeleteBlob						(sysLibTrapCustom + 72)	
	#define P1clTrapGetBlobData						(sysLibTrapCustom + 73)
	#define P1clTrapSetBlobData						(sysLibTrapCustom + 74)	
	#define P1clTrapGetBlobCreatorId				(sysLibTrapCustom + 75)	
	#define P1clTrapSetBlobCreatorId				(sysLibTrapCustom + 76)	
	#define P1clTrapGetLastError					(sysLibTrapCustom + 77)	
	#define P1clTrapTxnPrepareSync					(sysLibTrapCustom + 78)	
	#define P1clTrapTxnGetChange					(sysLibTrapCustom + 79)	
	#define P1clTrapTxnGetNumChanges				(sysLibTrapCustom + 80)		
	#define P1clTrapLogMarkAllCompleted				(sysLibTrapCustom + 81)	
	#define P1clTrapTxnLogMarkCompleted				(sysLibTrapCustom + 82)			
	#define P1clTrapTxnSyncComplete					(sysLibTrapCustom + 83)	
	#define P1clTrapDeleteServerId					(sysLibTrapCustom + 84)	
	#define P1clTrapGetApptFromServerId				(sysLibTrapCustom + 85)	
	#define P1clTrapSetServerId						(sysLibTrapCustom + 86)
	#define P1clTrapCategoryTruncateName			(sysLibTrapCustom + 87)
	#define P1clTrapCategorySetTriggerLabel			(sysLibTrapCustom + 88)
	#define P1clTrapCategoryGetNext					(sysLibTrapCustom + 89)
	#define P1clTrapCategoryPopupList				(sysLibTrapCustom + 90)
	#define P1clTrapCategorySelect					(sysLibTrapCustom + 91)
	#define P1clTrapCategoryEdit					(sysLibTrapCustom + 92)
	#define P1clTrapCategoryDrawBullet				(sysLibTrapCustom + 93)
	#define P1clTrapCategorySetColor				(sysLibTrapCustom + 94)
	#define P1clTrapCategoryGetRGBColor				(sysLibTrapCustom + 95)
	#define P1clTrapCategoryGetColor				(sysLibTrapCustom + 96)
	#define P1clTrapCategoryGetUnusedColor			(sysLibTrapCustom + 97)
	#define P1clTrapCategorySetName					(sysLibTrapCustom + 98)
	#define P1clTrapCategoryGetName					(sysLibTrapCustom + 99)
	#define P1clTrapCategoryFind					(sysLibTrapCustom + 100)
	#define P1clTrapCategoryFreeList				(sysLibTrapCustom + 101)
	#define P1clTrapCategoryCreateList				(sysLibTrapCustom + 102)
	#define P1clTrapCategoryGetVersion				(sysLibTrapCustom + 103)
	#define P1clTrapGiveAllTimedEventsTimeZones		(sysLibTrapCustom + 104)
	#define P1clTrapConvertFromDeviceTimeZone		(sysLibTrapCustom + 105)
	#define P1clTrapConvertToDeviceTimeZone			(sysLibTrapCustom + 106)
	#define P1clTrapCountryForTimeZoneAbbreviation	(sysLibTrapCustom + 107)
	#define P1clTrapApptTimeZoneAbbreviation		(sysLibTrapCustom + 108)
	#define P1clTrapApptTimeZoneCompare				(sysLibTrapCustom + 109)
	#define P1clTrapGetExtendedDescription			(sysLibTrapCustom + 110)
	#define P1clTrapHasExtendedDescription			(sysLibTrapCustom + 111)		
	#define P1clTrapApptGetFirstOccurrence			(sysLibTrapCustom + 112)			
	#define P1clTrapUpdateEventsWithDeletedTimeZone	(sysLibTrapCustom + 113)			
	#define P1clTrapUpdateEventsWithEditedTimeZone	(sysLibTrapCustom + 114)
	#define P1clTrapCopyStringField					(sysLibTrapCustom + 115)	
	#define P1clTrapDateExportVCal					(sysLibTrapCustom + 116)	
	#define P1clTrapNumRecordsInCategory			(sysLibTrapCustom + 117)	
	#define P1clTrapSeekRecordInCategory			(sysLibTrapCustom + 118)		
	#define P1clTrapDateSetGoToParams				(sysLibTrapCustom + 119)		
	#define P1clTrapGetSessionInfo					(sysLibTrapCustom + 120)	
	#define P1clTrapCopyRepeatDescription			(sysLibTrapCustom + 121)
	#define P1clTrapGetConflict						(sysLibTrapCustom + 122)	
	#define	P1clTrapGetDefaultTimeZone				(sysLibTrapCustom + 123)
	#define	P1clTrapSetRepeateEndDate				(sysLibTrapCustom + 124)
	#define	P1clTrapAddMultiDayEvent				(sysLibTrapCustom + 125)
	#define	P1clTrapAddAllToTransactionLog			(sysLibTrapCustom + 126)
	#define	P1clTrapSetExceptionExt					(sysLibTrapCustom + 127)
	#define	P1clTrapAddExceptionExt					(sysLibTrapCustom + 128)
	#define	P1clTrapGetExceptionDate				(sysLibTrapCustom + 129)
	#define	P1clTrapDeleteRepeatInfo				(sysLibTrapCustom + 130)	
	#define	P1clTrapDeleteAlarmInfo					(sysLibTrapCustom + 131)		
	#define	P1clTrapAccountReset					(sysLibTrapCustom + 132)		
	#define	P1clTrapAddMultiDayEvent2				(sysLibTrapCustom + 133)		
	#define	P1clTrapTxnGetChange2					(sysLibTrapCustom + 134)		
	#define	P1clTrapPurgeAllAppts					(sysLibTrapCustom + 135)		
	#define	P1clTrapDeleteDuplicates				(sysLibTrapCustom + 136)		
	#define P1clTrapAccountResetServerId			(sysLibTrapCustom + 137)
	#define P1clTrapAddLocalToTransactionLog		(sysLibTrapCustom + 138)

#endif