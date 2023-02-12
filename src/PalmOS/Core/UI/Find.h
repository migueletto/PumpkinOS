/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: Find.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *	  This file defines field structures and routines.
 *
 *****************************************************************************/

#ifndef __FIND_H__
#define __FIND_H__

#include <PalmTypes.h>
#include <CoreTraps.h>

#include <DataMgr.h>
#include <Rect.h>

#define maxFinds			9
#define maxFindStrLen	16

typedef struct {
	UInt16			appCardNo;			// card number of the application
	LocalID			appDbID;				// LocalID of the application
	Boolean			foundInCaller;		// true if found in app that called Find
	UInt8 			reserved;

	UInt16			dbCardNo;			// card number of the database record was found in
	LocalID			dbID;					// LocalID of the database record was found in
	UInt16 			recordNum;			// index of record that contain a match
	UInt16			matchPos;			// postion in record of the match.
	UInt16			matchFieldNum;		// field number
	UInt32			matchCustom;		// app specific data
	} FindMatchType;

typedef FindMatchType *FindMatchPtr;

typedef struct {

	// These fields are used by the applications.
	UInt16			dbAccesMode;		// read mode and maybe show secret
	UInt16 			recordNum;			// index of last record that contained a match
	Boolean			more;					// true of more matches to display
	Char				strAsTyped [maxFindStrLen+1];	// search string as entered
	Char				strToFind [maxFindStrLen+1];	// search string is lower case
	UInt8 			reserved1;


	// The lineNumber field can be modified by the app. The continuation field can
	// be tested by the app. All other fields are private to the Find routine and
	// should NOT be accessed by applications.
#ifdef ALLOW_ACCESS_TO_INTERNALS_OF_FINDPARAMS	// These fields will not be available in the next OS release!
	UInt16			numMatches;			// # of matches
	UInt16			lineNumber;			// next line in the results tabel
	Boolean			continuation;		// true if contining search of same app
	Boolean			searchedCaller;	// true after we've searched app that initiated the find

	LocalID			callerAppDbID;		// dbID of app that initiated search
	UInt16			callerAppCardNo;	// cardNo of app that initiated search

	LocalID			appDbID;				// dbID of app that we're currently searching
	UInt16			appCardNo;			// card number of app that we're currently searching
	
	Boolean			newSearch;			// true for first search
	UInt8 			reserved2;
	DmSearchStateType	searchState;	// search state
	FindMatchType	match [maxFinds];
#else
	UInt16			noAccessAllowed1;	// # of matches
	UInt16			lineNumber;			// next line in the results tabel
	Boolean			continuation;		// true if contining search of same app
	Boolean			noAccessAllowed2;	// true after we've searched app that initiated the find

	LocalID			noAccessAllowed3;	// dbID of app that initiated search
	UInt16			noAccessAllowed4;	// cardNo of app that initiated search

	LocalID			noAccessAllowed5;	// dbID of app that we're currently searching
	UInt16			noAccessAllowed6;	// card number of app that we're currently searching
	
	Boolean			noAccessAllowed7;	// true for first search
	UInt8 			noAccessAllowed8;
	DmSearchStateType	noAccessAllowed9;	// search state
	FindMatchType	noAccessAllowed10[maxFinds];
#endif
	RectangleType rect;
	UInt16 idx[maxFinds];
} FindParamsType;

typedef FindParamsType *FindParamsPtr;


// Param Block passsed with the sysAppLaunchCmdGoto Command
typedef struct {
	Int16				searchStrLen;		// length of search string.
	UInt16			dbCardNo;			// card number of the database	
	LocalID			dbID;					// LocalID of the database
	UInt16 			recordNum;			// index of record that contain a match
	UInt16			matchPos;			// postion in record of the match.
	UInt16			matchFieldNum;		// field number string was found int
	UInt32			matchCustom;		// application specific info
	} GoToParamsType;

typedef GoToParamsType *GoToParamsPtr;


//----------------------------------------------------------
//	Find Functions
//----------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

void Find (GoToParamsPtr goToP)
			SYS_TRAP(sysTrapFind);
		

Boolean FindStrInStr (Char const *strToSearch, Char const *strToFind, 
	UInt16 *posP)
			SYS_TRAP(sysTrapFindStrInStr);

Boolean FindSaveMatch (FindParamsPtr findParams, UInt16 recordNum, 
	UInt16 pos, UInt16 fieldNum, UInt32 appCustom, UInt16 cardNo, LocalID dbID)
			SYS_TRAP(sysTrapFindSaveMatch);

void FindGetLineBounds (const FindParamsType *findParams, RectanglePtr r)
			SYS_TRAP(sysTrapFindGetLineBounds);

Boolean FindDrawHeader (FindParamsPtr findParams, Char const *title)
			SYS_TRAP(sysTrapFindDrawHeader);

#ifdef __cplusplus 
}
#endif

#endif //__FIND_H__
