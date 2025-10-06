/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/**
 * @ingroup Network
 */

/**
 * @file 	PalmVPPI.h
 * @version 1.0
 *
 * @brief This is the API for VPN panel plug-ins (VPPIs).
 *
 * The VPN panel allows the user to manage their VPN accounts. Each plug-in provides support for a
 * specific VPN protocol such as PPTP or IPSec. A VPPI provides access to
 * the user interface of a VPN client. An NPPI (Network panel plug-in)
 * provides the underlying implementation as well as an alternate user
 * interface.
 *
 */

#ifndef _PALM_VPPI_H_
#define _PALM_VPPI_H_

/** Database type for VPN panel plug-ins. */
#define	sysFileTVPNPanelPlugin		'Vppi'

/** Account name maximum length. */
#define kMaxVPNAccountNameLength	31

/** @brief VPN Account structure. */
typedef struct
{
	UInt32  vppiCreator;	/**< Creator ID of the VPPI that "owns" the account. */
	UInt32  accountID;		/**< Arbitrary 32-bit ID used by the VPI to identify the account. */
	Char    accountName[kMaxVPNAccountNameLength + 1]; /**< User-specified account name. */
} VPNAccountType;


/** Launch code used by the VPN Panel to get the account list. */
#define sysVPPILaunchCmdGetAccounts		sysAppLaunchCmdCustomBase   // 0x8000

/** @brief Structure used by the VPN Panel to query the account from a VPPI. */
typedef struct
{
	UInt32          numAccounts;	/**< Number of accounts. */
	VPNAccountType* accounts;		/**< Array of accounts. */
	Err             err;			/**< Error set by the VPPI before returning the account list. */
} SysVPPILaunchCmdGetAccountsType;


/** Launch code used by the VPN Panel to edit an account. */
#define sysVPPILaunchCmdEditAccount		(sysAppLaunchCmdCustomBase + 1)   // 0x8001

/** @brief Structure used by the VPN Panel to edit an account. */
typedef struct
{
	Boolean        useWizardUI;		/**< Whether to show Previous & Done buttons instead of OK & Cancel buttons. */
	VPNAccountType account;			/**< Account edited or created. */
	Err            err;				/**< Return error. */
	Boolean        useSavedAccount;	/**< Whether the account information is temporary (for saved state) or permanent. */
	Boolean        dirtyBit;		/**< Whether any account settings have been changed. */
} SysVPPILaunchCmdEditAccountType;


/** Launch code used by the VPN Panel to delete an account. */
#define sysVPPILaunchCmdDeleteAccount		(sysAppLaunchCmdCustomBase + 2)   // 0x8002

/** @brief Structure used by the VPN Panel to delete an account. */
typedef struct
{
	VPNAccountType account;	/**< Account to delete. */
	Err            err;		/**< Error code. */
} SysVPPILaunchCmdDeleteAccountType;


/** Launch code used by the VPN Panel to set the current account. */
#define sysVPPILaunchCmdSetCurrentAccount	(sysAppLaunchCmdCustomBase + 3)   // 0x8003

/** @brief Structure used by the VPN Panel to set the current account. */
typedef struct
{
	VPNAccountType account;	/**< Account to set as current account. */
	Err            err;		/**< Error code. */
} SysVPPILaunchCmdSetCurrentAccountType;


/** Launch code used by the VPN Panel to request the VPPI to display a dialog. */
#define sysVPPILaunchCmdPromptAndConfigure      (sysAppLaunchCmdCustomBase + 4)   // 0x8004

/** @brief Structure used by the VPN Panel to request the VPPI to display a dialog. */
typedef struct
{
	UInt32   origCreator;   /**< Creator that launched the prompt dialog (in). */
	UInt32   origInstance;  /**< Instance that laucnhed the prompt dialog (in). */
	Err      errOut;        /**< Error code (out). */
} SysVPPILaunchCmdPromptAndConfigureType;


/** Launch code used by the VPN Panel to show the progess dialog. */
#define sysVPPILaunchCmdProgressPaint   (sysAppLaunchCmdCustomBase + 5)   // 0x8005

/** @brief Structure used by the VPN Panel to show the progess dialog. */
typedef struct
{
	UInt32        stage;		/**< Stage calue sent by the VPN Shim to the VPPI (in). */
	RectangleType clientRect;	/**< Rectangle in which the progress dialog box is displayed (in). */
	Err           errOut;		/**< Error code (out). */
} SysVPPILaunchCmdProgressPaintType;


/** Launch code used by the VPN Panel to show the error dialog. */
#define sysVPPILaunchCmdErrorDialog     (sysAppLaunchCmdCustomBase + 6)   // 0x8006

/** @brief Structure used by the VPN Panel to show the error dialog. */
typedef struct
{
	UInt32        error;		/**< Error value sent by the VPN Shim (in). */
	Err           errOut;		/**< Error code (out). */
} SysVPPILaunchCmdErrorDialogType;


/** Launch code used by the VPN Panel to clear the password cache. */
#define sysVPPILaunchCmdClearPWCache	(sysAppLaunchCmdCustomBase + 7)   // 0x8007

/** @brief Structure used by the VPN Panel to clear the password cache. */
typedef struct
{
	Err           errOut;		/**< Error code. */
} SysVPPILaunchCmdClearPWCacheType;


/** Launch code used by the VPN Panel to determine is the VPPI will prompt for password. */
#define sysVPPILaunchCmdWillPWPrompt    (sysAppLaunchCmdCustomBase + 8)   // 0x8008

/** @brief Structure used by the VPN Panel to determine is the VPPI will prompt for password. */
typedef struct
{
	UInt32        secsFromNow;	/**< Seconds from now at which the VPPI will prompt for password (in). */
	Err           errOut;		/**< Error code (out). */
} SysVPPILaunchCmdWillPWPromptType;

#endif  //_PALM_VPPI_H_
