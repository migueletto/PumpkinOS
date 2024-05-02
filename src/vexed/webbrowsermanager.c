/*************************************************

 WebBrowserManager.c

 Web Browser Manager implementation file

 Copyright (c) 2004, PalmSource, Inc. or its subsidiaries
 All rights reserved.

 Sample Code Disclaimer

 You may incorporate this sample code (the "Code") into your applications
 for Palm OS(R) platform products and may use the Code to develop
 such applications without restriction.  The Code is provided to you on
 an "AS IS" basis and the responsibility for its operation is 100% yours.
 PALMSOURCE, INC. AND ITS SUBSIDIARIES (COLLECTIVELY, "PALMSOURCE") DISCLAIM
 ALL WARRANTIES, TERMS AND CONDITIONS WITH RESPECT TO THE CODE, EXPRESS,
 IMPLIED, STATUTORY OR OTHERWISE, INCLUDING WARRANTIES, TERMS OR
 CONDITIONS OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE,
 NONINFRINGEMENT AND SATISFACTORY QUALITY.  You are not permitted to
 redistribute the Code on a stand-alone basis and you may only
 redistribute the Code in object code form as incorporated into your
 applications.  TO THE FULL EXTENT ALLOWED BY LAW, PALMSOURCE ALSO EXCLUDES ANY
 LIABILITY, WHETHER BASED IN CONTRACT OR TORT (INCLUDING NEGLIGENCE), FOR
 INCIDENTAL, CONSEQUENTIAL, INDIRECT, SPECIAL OR PUNITIVE DAMAGES OF ANY
 KIND, OR FOR LOSS OF REVENUE OR PROFITS, LOSS OF BUSINESS, LOSS OF
 INFORMATION OR DATA, OR OTHER FINANCIAL LOSS ARISING OUT OF OR IN
 CONNECTION WITH THE USE OR PERFORMANCE OF THE CODE.  The Code is subject
 to Restricted Rights for U.S. government users and export regulations.

 *************************************************/

#include <PalmOS.h>
#include <PalmOSGlue.h>

#include "webbrowsermanager.h"

/* LOCAL STATIC DATA */

// we list creator codes of known browsers here, but for the
// selection list, we will fetch the browser names from the
// tAIN resources.

static const UInt32 s_knownBrowsers[] =
{
	'BLZ5', // Blazer 3.0 (NetFront-based)
	'BLZ1', // Blazer 1.0/2.0
	'NF3T', // Netfront (Sony CLIE)
	'NOVR', // Novarra Web Pro
	'NF3P', // PalmSource Web Browser/NetFront
	'clpr', // Clipper (Web Clipping)
	'AvGo', // AvantGo
	'QCwb', // EudoraWeb
};

// Creator codes of web browsers that don't support external launch
// 'PLNK' - PocketLink
// 'PsCp' - Xiino / PalmScape

static char *s_webBrowserNameList = NULL;
static UInt32 *s_webBrowserCreatorList = NULL;

/* LOCAL FUNCTIONS */

static Err AddToBrowserList(UInt32 webAppCreator, const Char *name)
{
	char *newNameP;
	Int16 nameLength = StrLen(name) + 1;
	UInt32 *newCreatorP;
	Int16 listSize;
	Err err;
	MemHandle webBrowserNameListHandle;
	MemHandle webBrowserCreatorListHandle;

	if (s_webBrowserNameList == NULL)
	{
		webBrowserNameListHandle = MemHandleNew(nameLength);
		if (webBrowserNameListHandle == NULL)
			return memErrNotEnoughSpace;
		s_webBrowserNameList = newNameP = (char *)MemHandleLock(webBrowserNameListHandle);
	}
	else
	{
		listSize = MemPtrSize(s_webBrowserNameList);
		webBrowserNameListHandle = MemPtrRecoverHandle(s_webBrowserNameList);
		MemHandleUnlock(webBrowserNameListHandle);
		err = MemHandleResize(webBrowserNameListHandle, listSize + nameLength);
		if (err != errNone)
			return err;
		s_webBrowserNameList = (char *)MemHandleLock(webBrowserNameListHandle);
		newNameP = s_webBrowserNameList + listSize;
	}

	MemMove(newNameP, name, nameLength);

	if (s_webBrowserCreatorList == NULL)
	{
		webBrowserCreatorListHandle = MemHandleNew(sizeof(UInt32));
		if (webBrowserCreatorListHandle == NULL)
			return memErrNotEnoughSpace;
		s_webBrowserCreatorList = newCreatorP = (UInt32 *)MemHandleLock(webBrowserCreatorListHandle);
	}
	else
	{
		listSize = MemPtrSize(s_webBrowserCreatorList);
		webBrowserCreatorListHandle = MemPtrRecoverHandle(s_webBrowserCreatorList);
		MemHandleUnlock(webBrowserCreatorListHandle);
		err = MemHandleResize(webBrowserCreatorListHandle, listSize + sizeof(UInt32));
		if (err != errNone)
			return err;
		s_webBrowserCreatorList = (UInt32 *)MemHandleLock(webBrowserCreatorListHandle);
		newCreatorP = (UInt32 *)((char *)s_webBrowserCreatorList + listSize);
	}

	*newCreatorP = webAppCreator;

	return errNone;
}

static void GetApplicationInfo(LocalID appID, char **nameP, UInt32 *creatorP)
{
	DmOpenRef browserAppDB;
	MemHandle browserAppName;
	//char *nameCopyP = NULL;

	*nameP = NULL;
	*creatorP = 0;

	browserAppDB = DmOpenDatabase(0, appID, dmModeReadOnly);
	if (browserAppDB != NULL)
	{
		LocalID browserAppLocalID;
		UInt16 browserAppCardNo;
		char dbName[dmDBNameLength];
		Err err;

		err = DmOpenDatabaseInfo(
			browserAppDB, &browserAppLocalID, NULL, NULL,
			&browserAppCardNo, NULL);
		if (err == errNone)
		{
			err = DmDatabaseInfo(
				browserAppCardNo, browserAppLocalID, dbName,
				NULL, NULL, NULL, NULL, NULL,
				NULL, NULL, NULL, NULL, creatorP);
		}

		browserAppName = DmGet1Resource(ainRsc, 1000);
		if (browserAppName == NULL)
		{
			browserAppName = DmGet1Resource(ainRsc, 1);
		}

		if (browserAppName != NULL)
		{
			const char *resP = MemHandleLock(browserAppName);
			*nameP = MemPtrNew(StrLen(resP) + 1);
			StrCopy(*nameP, resP);
			MemHandleUnlock(browserAppName);
		}
		else
		{
			*nameP = MemPtrNew(StrLen(dbName) + 1);
			StrCopy(*nameP, dbName);
		}

		DmCloseDatabase(browserAppDB);
	}
}



/* EXPORTED FUNCTIONS */

void WBM_Init( Int16 *numSupportedBrowsersP )
{
	UInt16 cardNo = 0;
	Int16 found = 0;
	LocalID webAppID = 0;
	const int numKnownBrowsers = sizeof(s_knownBrowsers) / sizeof(UInt32);
	int i;

	// just in case we got nested WBM_Init() class....
	WBM_End();

	for (i = 0; i < numKnownBrowsers; ++i)
	{
		DmSearchStateType state;

		if( DmGetNextDatabaseByTypeCreator( true, &state, sysFileTApplication, s_knownBrowsers[i], true, &cardNo, &webAppID) == errNone )
			{
			Char *name;
			UInt32 creator;

			GetApplicationInfo( webAppID, &name, &creator);
			AddToBrowserList( creator, name );
			MemPtrFree( name );

			found++;
			}
	}

	if (s_webBrowserNameList == NULL)
		AddToBrowserList(0, "-- None --");

	*numSupportedBrowsersP = found;
}

/* call before program exits to free in-memory list of supported
 * web browsers */
void WBM_End(void)
{
	if (s_webBrowserNameList != NULL)
	{
		MemPtrFree(s_webBrowserNameList);
		s_webBrowserNameList = NULL;
	}

	if (s_webBrowserCreatorList != NULL)
	{
		MemPtrFree(s_webBrowserCreatorList);
		s_webBrowserCreatorList = NULL;
	}
}

/* return MemHandle that holds list of "const char *" pointers to web
 * browser names in LstSetListChoices-compatible format.  Caller should
 * use MemHandleFree on this MemHandle when finished. */
MemHandle WBM_GetBrowserList(void)
{
	Int16 numStrings = MemPtrSize(s_webBrowserCreatorList) / sizeof(UInt32);
	return SysFormPointerArrayToStrings(s_webBrowserNameList, numStrings);
}

/* return information about selected web browser choice from
 * web browser list */
UInt32 WBM_GetBrowserCreator(Int16 index)
{
	Int16 numCreators = MemPtrSize(s_webBrowserCreatorList) / sizeof(UInt32);
	if (index < numCreators)
		return s_webBrowserCreatorList[index];
	else
		return 0;
}

/* open the web page using the selected browser.  If you pass
 * 0 for the browserCreator, this will use the first browser found on
 * the device. */
Err WBM_OpenWebPage(UInt32 browserCreator, const char *url)
{
	UInt16 cardNo;
	LocalID webAppID;
	UInt32 launchCode;
	Err err;
	DmSearchStateType searchState;
	char *urlCopy;

//	FrmCustomAlert( AlertDebug, "here", url,"");
//	FrmCustomAlert( AlertDebug, url, s_webBrowserNameList[0], "" );

	if( browserCreator == 0 )
		browserCreator = s_webBrowserCreatorList[ 0 ];

	if( browserCreator == 0 )
		return wbmErrNoValidBrowserFound;


	urlCopy = (char *)MemPtrNew(StrLen(url) + 1);
	StrCopy(urlCopy, url);

	// reassign owner memory for the parameter block
	MemPtrSetOwner(urlCopy, 0);

	// special EudoraWeb launch URL code
	launchCode = (browserCreator == 'QCwb') ? 4000 : sysAppLaunchCmdGoToURL;

	err = DmGetNextDatabaseByTypeCreator(true, &searchState,
			 		sysFileTApplication, browserCreator, true,
			 		&cardNo, &webAppID);

	// switch to the web browser application
	if (err == errNone)
	{
		err = SysUIAppSwitch(
			cardNo, webAppID, launchCode, urlCopy);
	}


	// return directly to caller if launch was good
	if (err == errNone)
	{
		return errNone;
	}

	MemPtrFree(urlCopy);
	return err;
}
