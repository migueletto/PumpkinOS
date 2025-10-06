/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/**
 * @ingroup Network
 */

/**
 * @file 	PalmWPPI.h
 * @version 1.0
 * @date 	05/16/2003
 *
 * @brief This is the API for Wi-Fi panel plug-ins (WPPIs).
 *
 * The Wi-Fi panel allows the user to manage their network profiles. Each profile includes a
 * plugin, an authentication method, and a cookie used to identify the
 * user's credentials. Each plugin provides support for one or more
 * authentication methods; e.g., LEAP, PEAP, etc. A WPPI provides access to
 * the user interface of an 802.1x client.
 *
 */

#ifndef _PALM_WPPI_H_
#define _PALM_WPPI_H_


// A WPPI is much like an application. It has a PilotMain function that responds
// to certain launch codes. Applications are databases of type 'appl'. WPPIs use
// this type instead:

#define	sysFileTWiFiPanelPlugin		'Wppi'	/**< database type for Wi-Fi panel plug-ins */

// The creator ID of the database should be registered as usual. WPPI's don't have
// user-visible names; only authentication methods are visible to the user. A WPPI
// will typically have form resources, etc. These can be in the normal sub-10000 range
// reserved for application.

// A WPPI is always is always sublaunched, so it never has access to global variables
// or multiple code segments. The value returned by the PilotMain function is always
// ignored. Instead, an err field in the command parameter block is used to indicate
// success or failure. This is only used for out-of-memory and stuff like that, not
// for user-cancel. The Wi-Fi panel sets the err field to a non-zero value before
// calling the WPPI. The WPPI is responsible for zeroing the err field before returning
// if it's successful. The error code used doesn't matter; it is only used to indicate
// success or failure.

// Each WPPI is responsible for storing the credentials associated with accounts that
// use its authentication methods.

// The Wi-Fi panel queries each WPPI for a list of authentication methods and then
// merges the results. The query uses the following launch code and command parameter
// block:

#define sysWPPILaunchCmdGetAuthenticationMethods		sysAppLaunchCmdCustomBase   // 0x8000

#define kMaxWiFiAuthenticationMethodNameLength			31

typedef struct
	{
	UInt32  wppiCreator; /**< the creator ID of the WPPI that provides the authentication method */
	UInt32  authenticationMethodID; /**< an arbitrary 32-bit ID used by the WPPI to identify the authentication method */
	Char    authenticationMethodName[kMaxWiFiAuthenticationMethodNameLength + 1]; /**< the user-visible name for the authentication method (max width is 50 pixels) */
	} WiFiAuthenticationMethodType;

typedef struct
	{
	UInt32                        numAuthenticationMethods;
	WiFiAuthenticationMethodType* authenticationMethods;
	Err                           err;
	} SysWPPILaunchCmdGetAuthenticationMethodsType;

// The Wi-Fi panel will zero numAuthenticationMethods and authenticationMethods before
// calling the WPPI. It should allocate a chunk in the dynamic heap to store the
// authentication methods and put a pointer to this chunk in the authenticationMethods
// field. The Wi-Fi panel will free this chunk. Note that the WPPI creator ID is
// duplicated in each account.

// A WPPI is expected to maintain a database of credentials for each network that uses
// one of its authentication methods. The records in this database are identified by
// their unique ID. The Wi-Fi panel saves this unique ID, or cookie, along with all the
// other network settings, in its own database. A value of zero is used to indicate
// that there are no credentials because this is a new network or because the user
// switched authentication methods. The WPPI shouldn't save the user's credentials into
// its database until the Wi-Fi panel tells it to. This happens when the user taps the
// OK button in the Edit Network dialog. Until then, the entered credentials should be
// saved in a temporary location such as one or more features.

// The Wi-Fi panel makes the following sequence of calls into the WPPI:
// - sysWPPILaunchCmdOpen
// - sysWPPILaunchCmdDialog (zero or more times)
// - sysWPPILaunchCmdClose
// When the Edit Network dialog opens, the Wi-Fi panel calls sysWPPILaunchCmdOpen
// for the current WPPI and authentication method. When the user taps on the
// credentials selector trigger, the Wi-Fi panel calls sysWPPILaunchCmdDialog.
// When the user taps OK or Cancel in the Edit Network dialog, the Wi-Fi panel
// calls sysWPPILaunchCmdClose. If the user switches authentication methods, the
// Wi-Fi panel calls sysWPPILaunchCmdClose for the old method and
// sysWPPILaunchCmdOpen for the new one. All of these take the same command
// parameter block:

#define kMaxWiFiCredentialsLabelLength						31	// excludes terminator
#define kMaxWiFiCredentialsSelectorTriggerLength		31	// excludes terminator

typedef struct
	{
	UInt32  authenticationMethodID; /**< a 32-bit ID used by the WPPI to identify the authentication method */
	UInt32  cookie; /**< a unique ID or other identifier used to look up the credentials */
	Boolean save; /**< whether to save the user's changes (only for sysWPPILaunchCmdClose) */
	Boolean cancel; /**< whether the user tapped Cancel (only for sysWPPILaunchCmdDialog) */
	Boolean assigned; /**< whether credentials are considered assigned for validation purposes */
	Char    credentialsLabel[kMaxWiFiCredentialsLabelLength + 1]; /**< the label to use for the credentials line in the Edit Network dialog */
	Char    credentialsSelectorTrigger[kMaxWiFiCredentialsSelectorTriggerLength + 1]; /**< the selector trigger label indicating whether credentials have been assigned */
	Err     err; /**< non-zero on entry; set by WPPI to indicate success or failure */
	} SysWPPILaunchCmdType;

#define sysWPPILaunchCmdOpen		sysAppLaunchCmdCustomBase + 1                    // 0x8001

/// If a non-zero cookie is passed in, we're editing an existing network with existing
/// credentials. The WPPI should find the credentials in its database and put them in
/// a temporary location. If a zero cookie is passed in, we're either creating a new
/// network, editing an existing network that didn't previous have credentials, or
/// switching authentication methods. The WPPI should set the credentialsLabel and
/// credentialsSelectorTrigger. Typically, the credentialsLabel is "Credentials:" and
/// the credentialsSelectorTrigger is "-Assigned-" or "-Unassigned-". Every call to
/// sysWPPILaunchCmdOpen is matched with a call to sysWPPILaunchCmdClose.

#define sysWPPILaunchCmdClose		sysAppLaunchCmdCustomBase + 2                    // 0x8002

/// If the save flag is set, the WPPI should write the user's credentials to its
/// database. If the cookie is zero, it should create a new record and save its
/// unique ID in the cookie field of the command parameter block. If the cookie is
/// non-zero, it should overwrite the old record.

#define sysWPPILaunchCmdDialog	sysAppLaunchCmdCustomBase + 3                    // 0x8003

/// The WPPI should open its dialog and allow the user to edit the credentials. It
/// should read from the temporary location to initialize the username and password
/// fields or other UI elements as appropriate. It should write to the temporary
/// location when the user taps OK. It shouldn't access its database. Whether the
/// user tapped Cancel should be stored in the cancel flag in the command parameter
/// block. If the user taps OK and this changes the current credentials state
/// (typically assigned/unassigned), the credentialsSelectorTrigger field should
/// be updated by the WPPI. Note that the user may delete his credentials causing
/// the state to change from assigned to unassigned.

#define sysWPPILaunchCmdDelete	sysAppLaunchCmdCustomBase + 4                    // 0x8004

/// When the user deletes a network, the Wi-Fi panel notifies the WPPI to delete
/// the corresponding credentials. This uses the same command parameter block as
/// above. The cookie field is used to indentify the record in the WPPI's
/// database. A zero value should be ignored. If the user switches authentication
/// methods and taps OK to save the edited network, the Wi-Fi panel notifies the
/// old WPPI to delete the old credentials.

/// Finally, the Wi-Fi Setup wizard displays a detailed description of the selected
/// authentication method. To do this, it sublaunches the WPPI with the following
/// launch code and command parameter block.

#define sysWPPILaunchCmdGetDescription		sysAppLaunchCmdCustomBase + 5           // 0x8005

typedef struct
	{
	UInt32 authenticationMethodID;
	Char*  description;
	UInt32 descriptionSize;
	Err    err;
	} SysWPPILaunchCmdGetDescriptionType;

/// The WPPI should write into the description buffer supplied. The buffer is
/// allocated by the Wi-Fi panel. The WPPI should avoid writing past the end of
/// the buffer. The size of the buffer is included in the command parameter block.
/// The description is displayed in an 8 line, full width field in the normal
/// font.

///
/// This is invoked to bring up the prompt dialog (WPPI decides if
/// prompt is necessary) or pass any kind of data to the WPPI.
/// Optionally, before exiting, a WPPI can set the
/// netIFSettingDot1XConfig setting to pass information back to the
/// 802.1X plugin.
///
#define sysWPPILaunchCmdPromptAndConfigure      (sysAppLaunchCmdCustomBase + 6)   // 0x8006

typedef struct
{
  UInt32   ifCreator;   // in
  UInt32   ifInstance;  // in
  Err      errOut;      // out
} SysWPPILaunchCmdPromptAndConfigureType;


///
/// This is invoked to put up an error dialog.
///
#define sysWPPILaunchCmdErrorDialog     (sysAppLaunchCmdCustomBase + 7)   // 0x8007

typedef struct
{
  UInt32        error;          // in
  Err           errOut;         // out
} SysWPPILaunchCmdErrorDialogType;

///
/// This is invoked to request a password cache clear.
/// Removes temporary passwords from all accounts managed by the WPPI.
///
#define sysWPPILaunchCmdClearPWCache	(sysAppLaunchCmdCustomBase + 8)   // 0x8008

typedef struct
{
  Err           errOut;         // out
} SysWPPILaunchCmdClearPWCache;


#endif  //_PALM_WPPI_H_
