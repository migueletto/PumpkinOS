/******************************************************************************
 *
 * Copyright (c) 1996-2003 PalmSource, Inc. All rights reserved.
 *
 * File: AppLaunchCmd.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *		Pilot launch commands for applications.  Some launch commands
 *      are treated differently by different apps.  The different 
 *      parameter blocks used by the apps are kept here.
 *
 *****************************************************************************/

 #ifndef __APPLAUNCHCMD_H__
 #define __APPLAUNCHCMD_H__

// Include elementary types
#include <PalmTypes.h>


#define LaunchWithCommand(type, creator, command, commandParams) \
{ \
	UInt16				cardNo; \
	LocalID				dbID; \
	DmSearchStateType	searchState; \
	/*Err 					err;*/ \
	DmGetNextDatabaseByTypeCreator(true, &searchState, type, \
		creator, true,	&cardNo, &dbID); \
	ErrNonFatalDisplayIf(!dbID, "Could not find app"); \
	if (dbID) { \
		/*err =*/ SysUIAppSwitch(cardNo, dbID, command, commandParams); \
		ErrNonFatalDisplayIf(err, "Could not launch app"); \
		} \
	}
	
#define AppLaunchWithCommand(appCreator, appCommand, appCommandParams) \
	LaunchWithCommand (sysFileTApplication, appCreator, appCommand, appCommandParams)
	
#define AppCallWithCommand(appCreator, appCommand, appCommandParams) \
{ \
	UInt16				cardNo; \
	LocalID				dbID; \
	DmSearchStateType	searchState; \
	UInt32 				result; \
	/*Err 					err;*/ \
	DmGetNextDatabaseByTypeCreator(true, &searchState, sysFileTApplication, \
		appCreator, true,	&cardNo, &dbID); \
	ErrNonFatalDisplayIf(!dbID, "Could not find app"); \
	if (dbID) { \
		/*err =*/ SysAppLaunch(cardNo, dbID, 0, appCommand, (MemPtr) appCommandParams, &result); \
		ErrNonFatalDisplayIf(err, "Could not launch app"); \
		} \
	}
	
	

/************************************************************
 * Param Block passsed with the sysAppLaunchCmdLookup Command
 *************************************************************/

//-------------------------------------------------------------------
// sysAppLaunchCmdLookup parameter block for the Address Book
//-------------------------------------------------------------------

// This is a list of fields by which data may be looked up.
typedef enum {
	addrLookupName,
	addrLookupFirstName,
	addrLookupCompany,
	addrLookupAddress,
	addrLookupCity,
	addrLookupState,
	addrLookupZipCode,
	addrLookupCountry,
	addrLookupTitle,
	addrLookupCustom1,
	addrLookupCustom2,
	addrLookupCustom3,
	addrLookupCustom4,
	addrLookupNote,			// This field is assumed to be < 4K 
	addrLookupWork,
	addrLookupHome,
	addrLookupFax,
	addrLookupOther,
	addrLookupEmail,
	addrLookupMain,
	addrLookupPager,
	addrLookupMobile,
	addrLookupSortField,
	addrLookupListPhone,
	addrLookupFieldCount,	// add new fields above this one
	
	addrLookupNoField = 0xff
} AddressLookupFields;


#define addrLookupStringLength 12

typedef struct 
	{
	Char *title;					
		// Title to appear in the title bar.  If NULL the default is used.
		
	Char *pasteButtonText;
		// Text to appear in paste button.  If NULL "paste" is used.
		
	Char lookupString[addrLookupStringLength];
		// Buffer containing string to lookup.  If the string matches
		// only one record then that record is used without 
		// presenting the user with the lookup dialog.
		
	AddressLookupFields field1;
		// Field to search by.  This field appears on the left side
		// of the lookup dialog.  If the field is the sort field then
		// searches use a binary search.  If the field isn't the sort
		// field then the data does appear in sorted order and searching
		// is performed by a linear search (can get slow).
		
	AddressLookupFields field2;
		// Field to display on the right.  Often displays some 
		// information about the person.  If it is a phone field
		// and a record has multiple instances of the phone type
		// then the person appears once per instance of the phone
		// type. Either field1 or field2 may be a phone field but
		// not both.
		
	Boolean field2Optional;
		// True means that the record need not have field2 for
		// the record to be listed.  False means that field2 is
		// required in the record for it to be listed. 
		
	Boolean userShouldInteract;
		// True means that the user should resolve non unique 
		// lookups.  False means a non unique and complete lookup
		// returns resultStringH set to 0 and recordID set to 0;
		
	Char *formatStringP;
		// When the user selects the paste button a string is generated
		// to return data from the record.  The format of the result string
		// is controlled by this string.  All characters which appear
		// in this string are copied straight to the result string unless
		// they are a field (a '^' follow by the field name).  For 
		// example, the format string "^first - ^home" might result in
		// "Roger - 123-4567".
		
		// The field arguments are name, first, company, address, city
		// state, zipcode, country, title, custom1, custom2, custom3,
		// custom4, work, home, fax, other, email, main, pager, mobile, 
		// and listname.
		
	MemHandle resultStringH;
		// If there is a format string a result string is allocated on 
		// the dynamic heap and its handle is returned here.
	
	UInt32 uniqueID;
		// The unique ID of the found record or 0 if none was found.
		
	} AddrLookupParamsType;

typedef AddrLookupParamsType *AddrLookupParamsPtr;




/************************************************************
 * Param Block passsed with the sysAppLaunchCmdSetActivePanel Command
 *************************************************************/

#define prefAppLaunchCmdSetActivePanel		(sysAppLaunchCmdCustomBase + 1)	
																// Record this panel so switching to the Prefs app
																// causes this panel to execute.

typedef struct 
	{
	UInt32 activePanel;
		// The creator ID of a panel.  Usually sent by a panel so the prefs 
		// apps will switch to it.  This allows the last used panel to appear
		// when switching to the Prefs app.
		
	} PrefActivePanelParamsType;

typedef PrefActivePanelParamsType *PrefActivePanelParamsPtr;



/************************************************************
 * Param Block passsed with the sysAppLaunchCmdAddRecord Command
 *************************************************************/

//-------------------------------------------------------------------
// sysAppLaunchCmdAddRecord parameter block for the Mail application
//-------------------------------------------------------------------
// Param Block passsed with the sysAppLaunchCmdAddRecord Command

typedef enum { mailPriorityHigh, mailPriorityNormal, mailPriorityLow } MailMsgPriorityType;

typedef struct {
	Boolean					secret;
		// True means that the message should be marked secret
		
	Boolean					signature;
		// True means that signature from the Mail application's preferences
		// should be attached to the message.
		
	Boolean					confirmRead;
		// True means that a comfirmation should be sent when the message
		// is read.
		
	Boolean					confirmDelivery;
		// True means that a comfirmation should be sent when the message
		// is deliveried
		
	MailMsgPriorityType	priority;
		// high, normial, or low.
	
	UInt8		 			padding;

	Char *					subject;
		// Message's subject, a null-terminated string (optional).
		
	Char *					from;
		// Message's send, a null-terminated string (not currently used).

	Char *					to;
		// Address the the recipient, a null-terminated string (required).

	Char *					cc;
		// Copy Addresses, a null-terminated string (required).

	Char *					bcc;
		// Blind copy Addresses, a null-terminated string (required).

	Char *					replyTo;
		// Reply to address, a null-terminated string (required).

	Char *					body;
		// The text of the message, a null-terminated string (required).

} MailAddRecordParamsType;

typedef MailAddRecordParamsType *MailAddRecordParamsPtr;



//-------------------------------------------------------------------
// sysAppLaunchCmdAddRecord parameter block for the Messaging application
//-------------------------------------------------------------------
// Param Block passsed with the sysAppLaunchCmdAddRecord Command

//category defines
#define MsgInboxCategory					0
#define MsgOutboxCategory					1
#define MsgDeletedCategory					2
#define MsgFiledCategory					3
#define MsgDraftCategory					4

typedef struct 
	{
	UInt16					category;
		//is this an outgoing mesage? Or should it be put into a different category
		
	Boolean					edit;
		// True means that the message should be opened in the editor,instead of 
		// just dropped into the category (only applies to outBox category)
		
	Boolean					signature;
		// True means that signature from the Mail application's preferences
		// should be attached to the message.
							
	Char *					subject;
		// Message's subject, a null-terminated string (optional).
		
	Char *					from;
		// Message's send, a null-terminated string (not currently used).

	Char *					to;
		// Address the the recipient, a null-terminated string (required).

	Char *					replyTo;
		// Reply to address, a null-terminated string (required).

	Char *					body;
		// The text of the message, a null-terminated string (required).
	
	} MsgAddRecordParamsType;

typedef MsgAddRecordParamsType *MsgAddRecordParamsPtr;

#endif  //__APPLAUNCHCMD_H__
