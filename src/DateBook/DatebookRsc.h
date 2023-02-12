/******************************************************************************
 *
 * Copyright (c) 1998-2000 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: DatebookRsc.h
 *
 * Release: Palm OS SDK 4.0 (63220)
 *
 *****************************************************************************/

// Day View
#define DayView								1000
#define DayTitle								1001
#define DayPrevWeekButton					1002
#define DayDOW1Button						1003		// first day of week push button
#define DayDOW2Button						1004		// second day of week push button
#define DayDOW3Button						1005
#define DayDOW4Button						1006
#define DayDOW5Button						1007
#define DayDOW6Button						1008
#define DayDOW7Button						1009
#define DayNextWeekButton					1010
#define DayDayViewButton					1011
#define DayWeekViewButton					1012
#define DayMonthViewButton					1013
#define DayAgendaViewButton				1014
#define DayNewButton							1015
#define DayDetailsButton					1016
#define DayGoToButton						1017
#define DayUpButton							1018
#define DayDownButton						1019
#define DayTable								1020
#define DayOfWeekGroup						1
#define DayViewGroup							2
#if WRISTPDA
#define DayDOWPrevButton					1021
#define DayDOWNextButton					1022
#endif


// Week View
#define WeekView								1100
#define WeekPrevButton						1102
#define WeekNextButton						1103
#define WeekNumberLabel						1104
#define WeekDayViewButton					1105
#define WeekWeekViewButton					1106
#define WeekMonthViewButton				1107
#define WeekAgendaViewButton				1108
#define WeekGoToButton						1109
#define WeekUpButton							1110
#define WeekDownButton						1111
#define WeekDayDisplay						1112


// Month View 
#define MonthView								1200
#define MonthPrevButton						1202
#define MonthNextButton						1203
#define MonthDayViewButton					1204
#define MonthWeekViewButton				1205
#define MonthMonthViewButton				1206
#define MonthAgendaViewButton				1207
#define MonthGoToButton						1208
#define MonthGadget							1209
#if WRISTPDA
#define MonthMoveSelectionUp				1250
#define MonthMoveSelectionDown				1251
#define MonthGoToSelectedDay				1252
#endif

// Details Dialog Box
#define DetailsDialog						1300
#define DetailsTimeSelector				1303
#define DetailsDateSelector				1305
#define DetailsAlarmCheckbox				1307
#define DetailsAlarmAdvanceField			1308
#define DetailsAlarmAdvanceSelector		1310
#define DetailsAlarmAdvanceList			1311
#define DetailsRepeatSelector				1313
#define DetailsPrivateCheckbox			1315
#define DetailsOkButton						1316
#define DetailsCancelButton				1317
#define DetailsDeleteButton				1318
#define DetailsNoteButton					1319


// Repeat Dialog Box.
// Note: the code is depends on the current order of the ui object.
#define RepeatDialog							1400
#define RepeatNone							1402
#define RepeatDaily							1403
#define RepeatWeekly							1404
#define RepeatMonthly						1405
#define RepeatYearly							1406
#define RepeatEveryLabel					1407
#define RepeatFrequenceField				1408
#define RepeatDaysLabel						1409
#define RepeatWeeksLabel					1410
#define RepeatMonthsLabel					1411
#define RepeatYearsLabel					1412
#define RepeatEndOnLabel					1413
#define RepeatEndOnTrigger					1415
#define RepeatEndOnList						1416
#define RepeatRepeatOnLabel				1417
#define RepeatDayOfWeek1PushButton		1418
#define RepeatDayOfWeek2PushButton		1419
#define RepeatDayOfWeek3PushButton		1420
#define RepeatDayOfWeek4PushButton		1421
#define RepeatDayOfWeek5PushButton		1422
#define RepeatDayOfWeek6PushButton		1423
#define RepeatDayOfWeek7PushButton		1424
#define RepeatByLabel						1425
#define RepeatByDayPushButon				1426
#define RepeatByDatePushButon				1427
#define RepeatDescField						1428
#define RepeatNoRepeatLabel				1429
#define RepeatOkButton						1430
#define RepeatCancelButton					1431
#define RepeatDescRectGadget				1440
#define RepeatTypeGroup						1
#define RepeatByGroup						2
#if WRISTPDA
#define RepeatIncreaseType					1450
#define RepeatDecreaseType					1451
#endif

// Purge Dialog
#define PurgeDialog							1500
#define PurgeRangeList						1506
#define PurgeSaveBackup						1507
#define PurgeOk								1508
#define PurgeCancel							1509


// Delete Appt Dialog
#define DeleteApptDialog					1600
#define DeleteApptSaveBackup				1604
#define DeleteApptOk							1606
#define DeleteApptCancel					1607


// Monthly Repeat Dialog
#define MonthlyRepeatDialog				1700
#define MonthlyRepeatFourthButton		1704
#define MonthlyRepeatLastButton			1705
#define MonthlyRepeatOk						1706
#define MonthlyRepeatCancel				1707
#define MonthlyRepeatWeekGroup			1


// Preference Dialog
#define PreferencesDialog					1800
#define PreferStartUpButton				1803
#define PreferStartDownButton				1804
#define PreferEndUpButton					1806
#define PreferEndDownButton				1807
#define PreferAlarmCheckbox				1809
#define PreferAlarmField					1810
#define PreferAlarmUnitTrigger			1812
#define PreferAlarmList						1813
#define PreferOkButton						1814
#define PreferCancelButton					1815
#define PreferAlarmSoundTrigger			1817
#define PreferAlarmSoundList				1818
#define PreferRemindMeTrigger				1819
#define PreferRemindMeList					1820
#define PreferPlayEveryTrigger			1821
#define PreferPlayEveryList				1822


// Display Options Dialog
#define DisplayDialog						1900
#define DisplayShowTimeBarsCheckbox		1903
#define DisplayCompressDayViewCheckbox	1904
#define DisplayShowTimedCheckbox			1906
#define DisplayShowUntimedCheckbox		1907
#define DisplayShowRepeatingCheckbox	1908
#define DisplayOkButton						1909
#define DisplayCancelButton				1910


// Icon drawn when rendering alarm info
// into an attention manager detail view form
#define AlarmClockIcon						3000


// Exception Dialog
#define RangeDialog							2100
#define RangeCurrentButton					2103
#define RangeFutureButton					2104
#define RangeAllButton						2105
#define RangeCancelButton					2106


// Agenda View
#define AgendaView							2200
#define AgendaDivider						2202
#define AgendaDayViewButton				2203
#define AgendaWeekViewButton				2204
#define AgendaMonthViewButton				2205
#define AgendaAgendaViewButton			2206
#define AgendaGoToButton					2207
#define AgendaToDoCategoryTrigger		2208
#define AgendaToDoCategoryList			2209
#define AgendaDatebookTable				2210
#define AgendaSeparatorDefaultArea		2212
#define AgendaToDoTable						2213
#define AgendaScrollableArea				2214
#define AgendaPreviousDayButton			2215
#define AgendaNextDayButton				2216
#define AgendaCurrentDayButton			2217
#define AgendaDatebookScroller			2218
#define AgendaToDoScroller					2219
#define AgendaViewGroup						1			// Collection of view-switching buttons
#if WRISTPDA
#define AgendaApptListSelect				2250
#define AgendaToDoListSelect				2251
#define AgendaMoveApptSelectionDown			2252
#define AgendaMoveToDoSelectionDown			2253
#define AgendaMoveApptSelectionUp			2254
#define AgendaMoveToDoSelectionUp			2255
#endif

// Delete Note Alert
#define DeleteNoteAlert						2001
#define DeleteNoteYes						0
#define DeleteNoteNo             		1

// Select An Item Alert
#define SelectItemAlert						2002

// Repeat type changed Alert
#define RepeatTypeChangedAlert			2006
#define RepeatTypeChangedOkButton		0
#define RepeatTypeChangedCancelButton	1

// Alarm time text contains invalid chars Alert
#define AlarmInvalidAlert					2007

// Bitmaps
#define AlarmClockBitmap					1000

// Menus
#define DayViewMenu							1000
#define NoteViewMenu							1001
#define WeekViewMenu							1002

// Menu commands
#define NewItemCmd							100
#define DeleteCmd								101
#define CreateNoteCmd						102
#define DeleteNoteCmd						103
#define PurgeCmd								104
#define BeamRecordCmd						106
#define SendRecordCmd						107
#define UndoCmd								200
#define CutCmd									201
#define CopyCmd								202
#define PasteCmd								203
#define KeyboardCmd							205

#define DayFontsCmd							300
#define DayPreferencesCmd					301
#define DayDisplayOptionsCmd				302
#define DayPhoneLookup						303
#define DaySecurityCmd						304
#define DayGetInfoCmd						305

#define WeekPreferencesCmd					300
#define WeekSecurityCmd						301
#define WeekGetInfoCmd						302

#define MonthPreferencesCmd				300
#define MonthDisplayOptionsCmd			301
#define MonthSecurityCmd					302
#define MonthGetInfoCmd						303


// Strings
#define findDatebookHeaderStrID			100	// Find results section header
#define goToDateTitleStrID					101	// Date picker title
#define setTimeTitleStrID					102	// Time picker title
#define startDateTitleStrID				103	// Date picker title for selecting appt's date
#define endDateTitleStrID					104	// Date picker title for selecting repeat end date
#define daysOfWeekInitialsStrID			105	// Initials of the days of the week names	
#define noTimeStrID							126   // No time
#define repeatTypesStrID					127	// Names of repeat types.
#define dailyRepeatStrID					128	// description template
#define weeklyRepeat1DayStrID				129	// repeat description template
#define weeklyRepeat2DayStrID				130	// repeat description template
#define weeklyRepeat3DayStrID				131	// repeat description template
#define weeklyRepeat4DayStrID				132	// repeat description template
#define weeklyRepeat5DayStrID				133	// repeat description template
#define weeklyRepeat6DayStrID				134	// repeat description template
#define weeklyRepeat7DayStrID				135	// repeat description template
#define monthtlyByDayRepeatStrID			136	// repeat description template
#define monthtlyByDateRepeatStrID		137	// repeat description template
#define yearlyRepeatStrID					138	// repeat description template
#define freqOrdinalsStrID					139	// 1st, 2nd, 3rd, ..., 99st
#define weekOrdinalsStrID					140	// 1st, 2nd, 3rd, 4th, and last
#define dayOrdinalsStrID					141	// 1st - 31st
#define repeatNoneStrID						142	// "No Repeat"
#define everyDayRepeatStrID				143	// "Every day"
#define everyWeekRepeat1DayStrID			144	// "Every week on <DOW name>"
#define everyWeekRepeat2DayStrID			145	// "Every week on <DOW name> and <DOW name>"
#define everyWeekRepeat3DayStrID			146	//
#define everyWeekRepeat4DayStrID			147	//
#define everyWeekRepeat5DayStrID			148	//
#define everyWeekRepeat6DayStrID			149	//
#define everyWeekRepeat7DayStrID			150	//
#define everyMonthByDayRepeatStrID		151	// "The ^w ^d of every month"
#define everyMonthByDateRepeatStrID		152	// "The ^x of every month"
#define everyYearRepeatStrID				153	// "^m ^x every year"
#define repeatMonthNamesStrID				154	// "January February..."
#define repeatFullDOWNamesStrID			155	// "Sunday Monday..."
#define repeatShortDOWNamesStrID			156	// "Sun Mon..."

#define beamDescriptionStrID				1000
#define beamFilenameStrID					1001

// Strings 1002...1009 are templates used to create the Day View form title.
// They correspond to dfMDYWithSlashes...dfYMDWithDashes + dfMDYWithDashes
// (which is at a different ID range because exgDescriptionStrID filled in
// the range at 1009).
#define DayViewFirstTitleTemplateStrID	1002	// dfMDYLongWithComma
#define DayViewLastTitleTemplateStrID	1009
#define DayViewMDYWithDashesTitleStemplateStrID	1020

#define exgDescriptionStrID				1009
#define exgDesciptionUnknownStrID		1010	// preview unknown
#define exgDescriptionMultipleStrID		1011	// preview multiple events
#define exgDescriptionMultipleToDoStrID	1012	// preview multiple todos

	
#define weekNumberTemplateStrID			1100	// Week ^1
#define WeekViewTitleFullDateStrID		1101	// "Mmm 'YY"
#define WeekViewTitleShortDateStrID		1102	// "Mmm"
#define WeekViewTitleEmptyDateStrID		1103	// ""
#define WeekViewTitleTwoDatesStrID		1104	// "^0 - ^1"
#define WeekViewTitleOneDateStrID		1105	// "^0"

#define MonthViewTitleStrID				1200	// "Sep 1999"

#define drawAlarmDateTemplateStrID		2001	// "^0, ^1" -> "Wednesday, 08/11/99"
#define alarmPrivateApptStrID				2002	// "Private Appointment"
#define alarmTodayStrID						2003	// "today"

#define alarmSoundID							999	// Same as SndPlaySystemSound(sndAlarm).


#define agendaNoAppointmentsStrID		2201

#define defaultAlarmSoundNameID			1800	// must be equal to alarm name in General Panel's MidiSounds.rsrc

// String lists
#define freqOrdinal2ndStrlID				100	// 2nd (other)

// The agenda title date formats list is a list of 7 date template strings 
// corresonding to the first 7 format selections in the formats panel
// Since the 8th item (dfMDYWithSlashes)'s ID is discontiguous with the first seven 
// _and_ it is the same long date format, as dfMDYWithSlashes, the first entry
// corresponding to dfMDYWithSlashes is reused for this item.
#define agendaTitleDateFormatsListID	101

// Soft constants

#define weekViewYearFirst					1100	// True => display year first ('YY MMM - MMM)
#define soundTriggerLabelWidth			1800	// Pixel width for sound trigger label.

#if WRISTPDA
#undef  ErrDisplay
#define ErrDisplay(msg)
#endif
