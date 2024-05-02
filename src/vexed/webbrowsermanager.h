/*************************************************

 WebBrowserManager.h
 
 header file for Web Browser Manager
 
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
 
#ifndef WEBBROWSERMANAGER_H_
#define WEBBROWSERMANAGER_H_

#include <PalmOS.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Returned when you try to open a URL and there are no 
 * supported web browsers on the device */
#define wbmErrNoValidBrowserFound (appErrorClass + 0)

/* The general flow of using the web browser manager sample code
 * is to call WBM_Init in the application's initialization code.
 * WBM_GetBrowserList and WBM_GetBrowserCreator are used in code
 * that implements a "selected web browser" preference for your
 * application.  Finally, WBM_OpenWebPage is used to actually
 * switch to the web browser.  This switch is done using 
 * SysUIAppSwitch, so you may want to save state in an unsaved
 * preference to let the user return to their current location
 * in your application when they return.  Most web browsers don't
 * have a "exit" button, so your app may not be the next app to
 * be launched after the web browser ends. In the application
 * shutdown code, WBM_End is called to free memory used by the
 * sample code */

/* call before first call to web browser manager to initialize
 * the in-memory list of supported browsers. Returns number of
 * supported web browsers found on device. */
void WBM_Init(Int16 *numSupportedBrowsersP);

/* call before program exits to free in-memory list of supported
 * web browsers */
void WBM_End(void);

/* return MemHandle that holds list of "const char *" pointers to web
 * browser names in LstSetListChoices-compatible format.  Caller should
 * use MemHandleFree on this MemHandle when finished. */
MemHandle WBM_GetBrowserList(void);

/* return information about selected web browser choice from 
 * web browser list */
UInt32 WBM_GetBrowserCreator(Int16 index);

/* open the web page using the selected browser.  If you pass
 * 0 for the browserCreator, this will use the first browser found on
 * the device. */
Err WBM_OpenWebPage(UInt32 browserCreator, const char *url);

#ifdef __cplusplus
}
#endif

#endif /* WEBBROWSERMANAGER_H_ */
