/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/**
 * @ingroup NETMASTER
 */

/**
 * @file 	NetMasterLibErrors.h
 * @version 1.0
 * @date 01/31/2002
 *
 * @brief This  is the error declarations file for the NetMaster Library.
 *
 */

#ifndef _NET_MASTER_LIB_ERRORS_H_
#define _NET_MASTER_LIB_ERRORS_H_

#include <Common/HsCommon.h>	  // for <HsErrorClasses.h>

// WARNING:
//	  DO NOT MOVE OR REASSIGN ERROR CODE VALUES TO EXISTING CONSTANTS BECAUSE
//	  THEY ARE HARD-CODED IN OTHER MODULES, SUCH AS NETLIB PATCHES, SYSTEM
//	  PATCHES, AND ERROR STRING LISTS IN THE SYSTEM RESOURCE DATABASE.

/** @name Net Master Error Codes
 */
/**@{*/
#define netMasterErrClass			(hsNetMasterErrorClass)		/**< hsNetMasterErrorClass is defined in HsErrorClasses.h as 0x7100 */
#define netMasterErrMemory			(netMasterErrClass + 0x01)  /**< runtime memory error */
#define netMasterErrStorage			(netMasterErrClass + 0x02)  /**< storage memory error */
#define netMasterErrBadArg			(netMasterErrClass + 0x03)  /**< invalid argument, unknown ID */
#define netMasterErrNetNotInstalled	(netMasterErrClass + 0x04)  /**< NetLib has not been installed */
#define netMasterErrDispatchTable	(netMasterErrClass + 0x05)  /**< Unrecognized dipatch table format */
#define netMasterErrNoNetPrefLib	(netMasterErrClass + 0x06)  /**< NetPref library not found */
#define netMasterErrConfigNotFound	(netMasterErrClass + 0x07)  /**< NetPref config not found */
#define netMasterErrConfigFailed	(netMasterErrClass + 0x08)  /**< Attempt to configure network settings failed  */
#define netMasterErrNo1xService		(netMasterErrClass + 0x09)	/**< 1xRTT is not available  */
#define netMasterErrMIPFailed		(netMasterErrClass + 0x0A)	/**< Generic Mobile-IP failure  */
#define netMasterErrBadPort			(netMasterErrClass + 0x0B)	/**< Invalid port number -- we remap this from serErrBadPort. */
/**@}*/

/** @name MIP Error Codes
 *  Sprint's Mobile-IP (MIP) error codes.
 */
/**@{*/
#define netMasterErrMIPNoSimBindings			(netMasterErrClass + 0x0C)	//
#define netMasterErrMIPReasonUnspecified64		(netMasterErrClass + 0x0D)	//
#define netMasterErrMIPAdminProhibited65		(netMasterErrClass + 0x0E)	//
#define netMasterErrMIPInsufficientResources66	(netMasterErrClass + 0x0F)	//
#define netMasterErrMIPMobileNodeAuth67			(netMasterErrClass + 0x10)	//
#define netMasterErrMIPHomeAgentAuth68			(netMasterErrClass + 0x11)	//
#define netMasterErrMIPReqLifetimeTooLong69		(netMasterErrClass + 0x12)	//
#define netMasterErrMIPPoorlyFormedReq70		(netMasterErrClass + 0x13)	//
#define netMasterErrMIPPoorlyFormedReply71		(netMasterErrClass + 0x14)	//
#define netMasterErrMIPReqedEncapUnavail72		(netMasterErrClass + 0x15)	//
#define netMasterErrMIPReservedNUnavail73		(netMasterErrClass + 0x16)	//
#define netMasterErrMIPCantRevTun74				(netMasterErrClass + 0x17)	//
#define netMasterErrMIPMustRevTun75				(netMasterErrClass + 0x18)	//
#define netMasterErrMIPBadTtl76					(netMasterErrClass + 0x19)	//
#define netMasterErrMIPInvalidCareOfAddr77		(netMasterErrClass + 0x1A)	//
#define netMasterErrMIPRegistration_Timeout78	(netMasterErrClass + 0x1B)	//
#define netMasterErrMIPDelivStyleNotSupported79	(netMasterErrClass + 0x1C)	//
#define netMasterErrMIPHome_Network_Unreachable80 (netMasterErrClass + 0x1D)	//
#define netMasterErrMIPHa_Host_Unreachable81	(netMasterErrClass + 0x1E)	//
#define netMasterErrMIPHa_Port_Unreachable82	(netMasterErrClass + 0x1F)	//
#define netMasterErrMIPHa_Unreachable88			(netMasterErrClass + 0x20)	//
#define netMasterErrMIPNonZeroHaRequested96		(netMasterErrClass + 0x21)	//
#define netMasterErrMIPMissingNai97				(netMasterErrClass + 0x22)	//
#define netMasterErrMIPForeign_Agent98			(netMasterErrClass + 0x23)	//
#define netMasterErrMIPMissingHa99				(netMasterErrClass + 0x24)	//
#define netMasterErrMIPErrorFa1_100				(netMasterErrClass + 0x25)	//
#define netMasterErrMIPErrorFa2_101				(netMasterErrClass + 0x26)	//
#define netMasterErrMIPUnknown_Challenge104		(netMasterErrClass + 0x27)	//
#define netMasterErrMIPMissing_Challenge105		(netMasterErrClass + 0x28)	//
#define netMasterErrMIPStale_Challenge106		(netMasterErrClass + 0x29)	//
#define netMasterErrMIPReasonUnspecified128		(netMasterErrClass + 0x2A)	//
#define netMasterErrMIPAdminProhibited129		(netMasterErrClass + 0x2B)	//
#define netMasterErrMIPInsufficientResources130	(netMasterErrClass + 0x2C)	//
#define netMasterErrMIPMobileNodeAuth131		(netMasterErrClass + 0x2D)	//
#define netMasterErrMIPForeignAgentAuth132		(netMasterErrClass + 0x2E)	//
#define netMasterErrMIPRegIdMismatch133			(netMasterErrClass + 0x2F)	//
#define netMasterErrMIPPoorlyFormedReq134		(netMasterErrClass + 0x30)	//
#define netMasterErrMIPTooManySimMobBindings135	(netMasterErrClass + 0x31)	//
#define netMasterErrMIPUnknownHaAddr136			(netMasterErrClass + 0x32)	//
#define netMasterErrMIPCantRevTun137			(netMasterErrClass + 0x33)	//
#define netMasterErrMIPMustRevTun138			(netMasterErrClass + 0x34)	//
#define netMasterErrMIPReqEncapNotAvail139		(netMasterErrClass + 0x35)	//
#define netMasterErrMIPErrorHa1_140				(netMasterErrClass + 0x36)	//
#define netMasterErrMIPErrorHa2_141				(netMasterErrClass + 0x37)	//

/**
 * 801 Username suspension due to repeated incorrect passwords being sent
 */
#define netMasterErrMIPUsernameSuspension801	(netMasterErrClass + 0x38)
#define netMasterErrMIPReserved2				(netMasterErrClass + 0x39)	/**< Reserved for future use */
#define netMasterErrMIPReserved3				(netMasterErrClass + 0x3A)	/**< Reserved for future use */
#define netMasterErrMIPReserved4				(netMasterErrClass + 0x3B)	/**< Reserved for future use */
/**@}*/

/** @name Net Master Error Codes
 */
/**@{*/

#define netMasterErrAutoLoginNotPossible		(netMasterErrClass + 0x3C)	/**< auto-login did not take place */
#define netMasterErrCDMACarrierNetworkBusy		(netMasterErrClass + 0x3D)	/**< "network busy" event was received from the carrier's network. */
#define netMasterErrOneXServiceUnavailable		(netMasterErrClass + 0x3E)	/**<< requested service unavailable */

/** Authentication UI prompt (such as username/password or script plug-in)
 *  was requested during background login -- this is not supported
 *  presently.
 */
#define netMasterErrAuthReqDuringBGLogin		(netMasterErrClass + 0x3F)

/** UI RPC timed out during a background-initiated login. This is an internal
 * error that might occur if the current UI app is not processing events
 * for a long time.
 */
#define netMasterErrUIRPCTimeout				(netMasterErrClass + 0x40)

/** @internal
 * INTERNAL ERROR (not returned outside of NetMaster lib). This may
 * be returned internally by various network setting configuration
 * routines to indicate that the level of service necessary to establish
 * the requested connection is not available (such as when a GPRS connection
 * is requested, but GPRS service is not available). When this error is
 * returned to the login logic, the login logic may still choose to attempt
 * the connection for the sole purpose of getting a more detailed error
 * code (which may not have been available during configuration) to display
 * to the user, or, if fallback is available, it may skip directly to the
 * fallback service.  When returning this error code, the configuration
 * routines still perform the full requested configuration.
 */
#define netMasterErrInternalConnWouldBeImpossible (netMasterErrClass + 0x41)


/** Net Guard prompt would have been required during background login. Prompting
 * is presently not supported during background login.
 */
#define netMasterErrNetGuardReqDuringBGLogin	(netMasterErrClass + 0x42)


/** Generic GPRS login error -- returned when we don't have enough specific
 * info about the cause of login failure.
 */
#define netMasterErrGPRSGenLoginErr				(netMasterErrClass + 0x43)


/** Sprint OneX service is unavailable while roaming
 */
#define netMasterErrNoOneXServiceWhileRoaming	(netMasterErrClass + 0x44)

/**@}*/


// NOTE:
// Assign error codes out of this unassigned error code block next.  These
// error codes were inserted because someone mistakenly defined
// netMasterErrOneXServiceNotEnabled as 0x4F instead of 0x3F in the
// Visor3.5CDMA Maint branch, and we need to maintain that value for backward
// compatibility. We need to account for all intervening error codes because
// SysErrString and related functionality doesn't allow gaps in error strings.

/** @name Reserved for future use
 */
/**@{*/
#define netMasterErrUnused45					(netMasterErrClass + 0x45)	// assign this one next
#define netMasterErrUnused46					(netMasterErrClass + 0x46)	// assign this one next
#define netMasterErrUnused47					(netMasterErrClass + 0x47)	// assign this one next
#define netMasterErrUnused48					(netMasterErrClass + 0x48)	// assign this one next
#define netMasterErrUnused49					(netMasterErrClass + 0x49)	// assign this one next
#define netMasterErrUnused4A					(netMasterErrClass + 0x4A)	// assign this one next
#define netMasterErrUnused4B					(netMasterErrClass + 0x4B)	// assign this one next
#define netMasterErrUnused4C					(netMasterErrClass + 0x4C)	// assign this one next
#define netMasterErrUnused4D					(netMasterErrClass + 0x4D)	// assign this one next
#define netMasterErrUnused4E					(netMasterErrClass + 0x4E)	// assign this one next
/**@}*/

/** @name Net Master Error Codes
 */
/**@{*/
#define netMasterErrOneXServiceNotEnabled		(netMasterErrClass + 0x4F)	/**< modem doesn't support requested service */
/**@}*/

//End NetMaster error code group.


#endif // _NET_MASTER_LIB_ERRORS_H_
