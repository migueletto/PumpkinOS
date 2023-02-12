/******************************************************************************
 *
 * Copyright (c) 1999-2000 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: DateAgenda.h
 *
 * Release: Palm OS SDK 4.0 (63220)
 *
 * Description:
 *	  Display and support code for Datebook's Agenda view.
 *
 * History:
 *		May 28, 1999	Created by Russell Brenner
 *			Name	Date		Description
 *			----	----		-----------
 *			rbb	5/28/99	Initial Revision
 *
 *****************************************************************************/

#define agendaTitleDateColumn				0
#define agendaTitleTimeColumn				1

#define agendaTitleInset					4
#define agendaTitleDateColumnSpacing	0

// Columns in the agenda view's appointment table
#define agendaApptTimeColumn				0
#define agendaApptDescColumn				1

#define agendaApptTimeColumnSpacing		3


#define agendaToDoTitleColumn				0
#define agendaToDoCategoryColumn			1

#define agendaToDoTitleInset				4		// Pixels between title and left margin
#define agendaToDoCategoryInset			4		// Pixels between category menu and right margin

#define agendaToDoTitleColumnSpacing	0		// Space between title and category columns


// Columns in the agenda view's task table



enum {
	agendaSectionUnused,
	agendaSectionMainTitle,
	agendaSectionAppointmentsTitle,
	agendaSectionAppointmentsView,
	agendaSectionTasksTitle,
	agendaSectionTasksView
};
#define agendaSectionMax			agendaSectionTasksView




/***********************************************************************
 *
 *	Agenda structure
 *
 ***********************************************************************/

// In a better world, these would be the members of class Agenda
typedef struct {
	DmOpenRef						apptDB;
	MemHandle						apptsH;
	UInt16							apptCount;
	FontID							apptFont;
	UInt8								reserved1;
	UInt16							apptLineHeight;
	UInt16							apptLineCount;
	UInt16							apptTopVisibleIndex;
	
	DmOpenRef						todoDB;
	MemHandle						todosH;
	UInt16							todoCount;
	FontID							todoFont;
	UInt8								reserved2;
	UInt16							todoLineHeight;
	UInt16							todoLineCount;
	UInt16							todoCategoryID;	// DOLATER ??? - or is it Index?
	char								todoCategoryName [dmCategoryLength];
	Boolean							todoShowCategories;
	UInt8								reserved3;

	
//	DateType							date;
	FontID							dateFont;
	DateFormatType					dateFormat;
	DateFormatType					longDateFormat;
	
	FontID							timeFont;
	TimeFormatType					timeFormat;
	UInt8								reserved4;
	UInt32							timeSeconds;
} AgendaType;


/***********************************************************************
 *
 *	Functions
 *
 ***********************************************************************/
#ifdef __cplusplus
extern "C" {
#endif


Boolean				AgendaViewHandleEvent (
								EventType*							event) EXTRA_SECTION_THREE;

void					AgendaViewInit (
								FormPtr							frm ) EXTRA_SECTION_THREE;

Boolean				SeekCurrentRecord (
								DmOpenRef						inDB,
								UInt16*							ioIndexP,
								UInt16							inOffset,
								Int16								inDirection ) EXTRA_SECTION_THREE;

#ifdef __cplusplus
}
#endif
