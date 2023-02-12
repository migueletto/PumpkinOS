/******************************************************************************
 *
 * Copyright (c) 1995-2000 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: DateAlarm.h
 *
 * Release: Palm OS SDK 4.0 (63220)
 *
 * Description:
 *	  This file defines the alarm functions.
 *
 * History:
 *		August 29, 1995	Created by Art Lamb
 *			Name		Date		Description
 *			----		----		-----------
 *			frigino	9/9/97		Added extern ref to PlayAlarmSound
 *			gap		07/28/00	Added preliminary attention manager support.
 *
 *****************************************************************************/

// There are several occasions where the app will need to use
// the attention manager iteration routine to update or validate
// the information that exists in the attention manager queue.
// The following constants designate the type of iteration requested.
#define	SoundRepeatChanged		1
#define	PostHotsyncVerification	2
#define	DeviceTimeChanged			3

#define AlarmUpdateType			UInt8 


Err 		FindRecordByID (DmOpenRef dbP, UInt32 uniqueID, UInt16 * recordNum) EXTRA_SECTION_TWO;

void 	AlarmReset (Boolean newerOnly) EXTRA_SECTION_TWO;
void 	RescheduleAlarms (DmOpenRef dbP) EXTRA_SECTION_TWO;

void 	PlayAlarmSound(UInt32 uniqueRecID) EXTRA_SECTION_TWO;

UInt32 	AlarmGetTrigger (UInt32* refP) EXTRA_SECTION_TWO;
void 	AlarmSetTrigger (UInt32 alarmTime, UInt32 ref) EXTRA_SECTION_TWO;
void 	AlarmTriggered (SysAlarmTriggeredParamType * cmdPBP) EXTRA_SECTION_TWO;

Boolean DeleteAlarmIfPosted (UInt16 recordNum) EXTRA_SECTION_TWO;
void		UpdatePostedAlarms (AlarmUpdateType  updateType) EXTRA_SECTION_TWO;
Boolean	AttentionBottleNeckProc(AttnLaunchCodeArgsType * paramP) EXTRA_SECTION_TWO;
