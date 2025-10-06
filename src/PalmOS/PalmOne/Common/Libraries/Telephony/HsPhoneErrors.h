/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/** @ingroup Telephony
 *
 */


/**
 *
 * @file 	HsPhoneErrors.h
 *
 * @brief  Errors returned by API function in GSM/CDMA Library
 *
 * NOTES:
 * 	This file is meant to contain all error codes that are returned
 * 	by an API in the phone library. If the error is not being returned
 * 	by an API call, please do not add it to this file, but keep it in
 * 	a private header file in the libraries.
 */




#ifndef HS_PHONE_ERRORS_H
#define HS_PHONE_ERRORS_H

#include <Common/System/HsErrorClasses.h> // For hsPhoneLibErrorClass_1

/**
 *  PHN Library result codes
 **/
typedef Err PHNErrorCode;		/**<		*/

enum PHNErrorCodeRange_1
  {
	phnErrorClass = hsPhoneLibErrorClass_1,
	phnErrParam,					/**< 0x4001 - invalid [in] params or invalid state machine. */
	phnErrUnknownError,				/**< 0x4002	poorly used error code; in GSM library indicates unexpected error code from radio, or various random parameters having unexpected values */
	phnErrNoResponse,				/**< 0x4003	Fail to receive event from modem. */
	phnErrNotOpen,					/**< 0x4004	phone library is not loaded for universal function or	PhnLibOpen is not called before the function is called. */
	phnErrStillOpen,				/**< 0x4005	opencount of phone lib is > 0 after PhnLibClose is called. */
	phnErrMemory,					/**< 0x4006	insufficient memory to perform the operation */
	phnErrUnknownID,				/**< 0x4007	id from list is valid (e.g. from GSM connection list, SMS msg list, address book, etc) */
	phnErrNoPower,					/**< 0x4008	when phone is off. */
	phnErrNoNetwork,				/**< 0x4009	No service. */
	phnErrNoConnection,				/**< 0x400a	NOT USED IN GSM LIBRARY */
	phnErrNotAllowed,				/**< 0x400b	operation not allowed (WISMO CME ERROR: 3, CMS ERROR: 302) */
	phnErrIllegalFacility,				/**< 0x400c	invalid GSM barring facility specified when activating/deactivating call barring */
	phnErrIllegalCondition,				/**< 0x400d	provided condition for operation is invalid (such as GSM call forwarding condition) */
	phnErrIllegalStatus,				/**< 0x400e	invalid GSM SMS Message Status detected */
	phnErrIllegalIndex,				/**< 0x400f	index out of range in list (such as SIM PhoneBook or segmented SMS msg) */
	phnErrIllegalChars,				/**< 0x4010	character out of GSM valid set in SMS or other text */
	phnErrIllegalMsg,				/**< 0x4011	NOT USED IN GSM LIBRARY */
	phnErrIllegalType,				/**< 0x4012	unexpected type encountered (for example unexpected SMS message type for operation) */
	phnErrIllegalNumber,				/**< 0x4013	unexpected character in DTMF string or GSM phone number */
	phnErrTimeout,					/**< 0x4014	NOT USED IN GSM LIBRARY */
	phnErrUnknownApp,				/**< 0x4015	NOT USED IN GSM LIBRARY */
	phnErrUnknownNumber,				/**< 0x4016	given GSM phone number not found in address book */
	phnErrBufferTooSmall,				/**< 0x4017	NOT USED IN GSM LIBRARY */
	phnErrPasswordRequired,				/**< 0x4018	required GSM password or PIN is missing or invalid */
	phnErrResponsePending,				/**< 0x4019	operation failed because radio was busy processing another command */
	phnErrCancelled,				/**< 0x401a	operation cancelled by user (e.g. cancel while entering password) */
	phnErrNoRecipient,				/**< 0x401b	operation failed because GSM SMS message had no recipients specified */
	phnErrPhoneFailure,				/**< 0x401c	Error reading/writing value to phone. */
	phnErrPhoneNotConnected,        		/**< 0x401d WISMO CME ERROR: 1 */
	phnErrPhoneAdaptorLinkReserved, 		/**< 0x401e WISMO CME ERROR: 2 */
	phnErrNotSupported,             		/**< 0x401f	Operation not supported (WISMO CME ERROR: 4; CMS ERROR: 303) */
	phnErrPhPINRequired,            		/**< 0x4020	WISMO CME ERROR: 5 (PH-SIM PIN required (SIM lock)) */
	phnErrPhFPINRequired,          	 		/**< 0x4021	WISMO CME ERROR: 6 */
	phnErrPhFPUKRequired,           		/**< 0x4022	WISMO CME ERROR: 7 */
	phnErrNoSIM,                    		/**< 0x4023	WISMO CME ERROR: 10 (SIM not inserted) */
	phnErrPINRequired,              		/**< 0x4024	WISMO CME ERROR: 11 (SIM PIN required) */
	phnErrPUKRequired,              		/**< 0x4025	WISMO CME ERROR: 12 (SIM PUK required) */
	phnErrSIMFailure,               		/**< 0x4026	WISMO CME ERROR: 13 (SIM failure) */
	phnErrSIMBusy,                  		/**< 0x4027	WISMO CME ERROR: 14; CMS ERROR: 314; NOT DOCUMENTED IN WISMO DOC */
	phnErrSIMWrong,                 		/**< 0x4028	WISMO CME ERROR: 15; CMS ERROR: 315; NOT DOCUMENTED IN WISMO DOC */
	phnErrIncorrectPassword,        		/**< 0x4029	WISMO CME ERROR: 16 (Incorrect password) */
	phnErrPIN2Required,             		/**< 0x402a	WISMO CME ERROR: 17 (SIM PIN2 required) */
	phnErrPUK2Required,             		/**< 0x402b	WISMO CME ERROR: 18 (SIM PUK2 required) */
	phnErrMemoryFull,               		/**< 0x402c	WISMO CME ERROR: 20 (Memory full ) */
	phnErrInvalidMemIndex,          		/**< 0x402d	WISMO CME ERROR: 21 (Invalid index) */
	phnErrNotFound,                 		/**< 0x402e	WISMO CME ERROR: 22 (Not found) */
	phnErrMemFailure,               		/**< 0x402f	WISMO CME ERROR: 23 NOT DOCUMENTED IN WISMO DOC */
	phnErrStringTooLong,            		/**< 0x4030	WISMO CME ERROR: 24 (Text string too long) */
	phnErrInvalidTextChars,         		/**< 0x4031	WISMO CME ERROR: 25 NOT DOCUMENTED IN WISMO DOC */
	phnErrDialStringTooLong,        		/**< 0x4032	WISMO CME ERROR: 26 (Dial string too long) */
	phnErrInvalidDialChars,         		/**< 0x4033	WISMO CME ERROR: 27 (Invalid characters in dial string) */
	phnErrNoNetworkService,         		/**< 0x4034	WISMO CME ERROR: 30 (No network service); signal faded on CDMA */
	phnErrNetworkTimeout,           		/**< 0x4035 WISMO CME ERROR: 31; CMS ERROR: 332 NOT DOCUMENTED IN WISMO DOC */
	phnErrNetworkNotAllowed,        		/**< 0x4036	WISMO CME ERROR: 32 (Network not allowed - emergency calls only) */
	phnErrNetPINRequired,           		/**< 0x4037	WISMO CME ERROR: 40 (Network personalisation PIN required (Network lock)) */
	phnErrNetPUKRequired,           		/**< 0x4038	WISMO CME ERROR: 41 */
	phnErrNetSubPINRequired,        		/**< 0x4039	WISMO CME ERROR: 42 */
	phnErrNetSubPUKRequired,        		/**< 0x403a	WISMO CME ERROR: 43 */
	phnErrSPPINRequired,            		/**< 0x403b	WISMO CME ERROR: 44 */
	phnErrSPPUKRequired,            		/**< 0x403c	WISMO CME ERROR: 45 */
	phnErrCorpPINRequired,          		/**< 0x403d	WISMO CME ERROR: 46 */
	phnErrCorpPUKRequired,          		/**< 0x403e	WISMO CME ERROR: 47 */
	phnErrIllegalMS,                		/**< 0x403f	WISMO CME ERROR: 103 */
	phnErrIllegalME,                		/**< 0x4040	WISMO CME ERROR: 106 */
	phnErrGPRSNotAllowed,           		/**< 0x4041	WISMO CME ERROR: 107 */
	phnErrPLMNNotAllowed,           		/**< 0x4042	WISMO CME ERROR: 111 */
	phnErrLocAreaNotAllowed,        		/**< 0x4043	WISMO CME ERROR: 112 */
	phnErrRoamingNotAllowed,        		/**< 0x4044	WISMO CME ERROR: 113 */
	phnErrOptionNotSupported,       		/**< 0x4045	WISMO CME ERROR: 132 */
	phnErrReqOptionNotSubscribed,   		/**< 0x4046	WISMO CME ERROR: 133 */
	phnErrOptionTempOutOfOrder,     		/**< 0x4047	WISMO CME ERROR: 134 */
	phnErrUnspecifiedGPSRError,     		/**< 0x4048	WISMO CME ERROR: 148 */
	phnErrAuthenticationFailure,    		/**< 0x4049	WISMO CME ERROR: 149 */
	phnErrInvalidMobileClass,       		/**< 0x404a	WISMO CME ERROR: 150 */
	phnErrUnassignedNumber,         		/**< 0x404b	WISMO CMS ERROR: 1 */
	phnErrOperDeterminedBarring,    		/**< 0x404c	WISMO CMS ERROR: 8 */
	phnErrCallBarred,               		/**< 0x404d	WISMO CMS ERROR: 10 or 18  Call guard option does not allow call */
	phnErrSMSXferRejected,          		/**< 0x404e	WISMO CMS ERROR: 21 */
	phnErrDestOutOfService,         		/**< 0x404f	WISMO CMS ERROR: 27 */
	phnErrUnidentifedSubscriber,    		/**< 0x4050	WISMO CMS ERROR: 28 */
	phnErrFacRejected,              		/**< 0x4051	WISMO CMS ERROR: 29 */
	phnErrUnknownSubscriber,        		/**< 0x4052	WISMO CMS ERROR: 30 */
	phnErrNetworkOutOfOrder,        		/**< 0x4053	WISMO CMS ERROR: 38 */
	phnErrTemporaryFailure,         		/**< 0x4054	WISMO CMS ERROR: 41 */
	phnErrCongestion,               		/**< 0x4055	WISMO CMS ERROR: 42, APEX 2,10,16,17  */
	phnErrResourcesUnavailable,     		/**< 0x4056   No resources/network busy	 */
	phnErrReqFacNotSubscribed,      		/**< 0x4057	GSM Error */
	phnErrReqFacNotImplemented,     		/**< 0x4058	GSM Error */
	phnErrInvalidSMSReference,      		/**< 0x4059	GSM Error */
	phnErrInvalidMsg,              	 		/**< 0x405a	GSM Error */
	phnErrInvalidMandInfo,          		/**< 0x405b	GSM Error */
	phnErrMsgTypeNonExistent,       		/**< 0x405c	GSM Error */
	phnErrMsgNoCompatible,          		/**< 0x405d	GSM Error */
	phnErrInfoElemNonExistent,      		/**< 0x405e	GSM Error */
	phnErrProtocolError,            		/**< 0x405f	GSM Error */
	phnErrInterworking,             		/**< 0x4060	GSM Error */
	phnErrTelematicIWNotSupported,  		/**< 0x4061	GSM Error */
	phnErrSMType0NotSupported,      		/**< 0x4062	GSM Error */
	phnErrCannotReplaceMsg,         		/**< 0x4063	GSM Error */
	phnErrUnspecifiedTPPIDError,   	 		/**< 0x4064	GSM Error */
	phnErrAlphabetNotSupported,     		/**< 0x4065	GSM Error */
	phnErrMsgClassNotSupported,     		/**< 0x4066	GSM Error */
	phnErrUnspecifiedTPDCSError,    		/**< 0x4067	GSM Error */
	phnErrCmdCannotBeActioned,      		/**< 0x4068	GSM Error */
	phnErrCmdUnsupported,           		/**< 0x4069	GSM Error */
	phnErrUnspecifiedTPCmdError,    		/**< 0x406a	GSM Error */
	phnErrTPDUNotSupported,         		/**< 0x406b	GSM Error */
	phnErrSCBusy,                   		/**< 0x406c	GSM Error */
	phnErrNoSCSubscription,         		/**< 0x406d	GSM Error */
	phnErrSCSystemFailure,          		/**< 0x406e	GSM Error */
	phnErrInvalidSMEAddr,           		/**< 0x406f	GSM Error */
	phnErrDestSMEBarred,            		/**< 0x4070	GSM Error */
	phnErrSMRejectedDuplicate,      		/**< 0x4071	GSM Error */
	phnErrTPVPFNotSupported,        		/**< 0x4072	GSM Error */
	phnErrTPVPNotSupported,         		/**< 0x4073	GSM Error */
	phnErrSMSStorageFull,           		/**< 0x4074	GSM Error */
	phnErrNoSMSStorage,             		/**< 0x4075	GSM Error */
	phnErrErrorInMS,                		/**< 0x4076	GSM Error */
	phnErrSIMApplToolkitBusy,       		/**< 0x4077	GSM Error */
	phnErrMEFailure,                		/**< 0x4078	GSM Error */
	phnErrSMSServReserved,          		/**< 0x4079	WISMO CMS ERROR: 301 (SMS service of ME reserved) */
	phnErrInvalidParameter,         		/**< 0x407a	GSM Error */
	phnErrFiller,                   		/**< 0x407b	GSM Error */
	phnErrFiller2,                  		/**< 0x407c	GSM Error */
	phnErrFiller3,                  		/**< 0x407d	GSM Error */
	phnErrMemoryFailure,            		/**< 0x407e	Memory access failure */
	phnErrSCAddrUnknown,            		/**< 0x407f	GSM Error */
	phnErrNoCNMAAckExpected,        		/**< 0x4080	GSM Error */
		/** Errors returned by the firmware (NO CARRIER) */
	phnErrFDNMismatch,              		/**< 0x4081  GSM Error */
	phnErrEmergencyCallsOnly,       		/**< 0x4082  GSM Error */
	phnErrACMLimitExceeded,         		/**< 0x4083  GSM Error */
	phnErrHoldError,                		/**< 0x4084  GSM Error */
	phnErrNumberBlacklisted,        		/**< 0x4085  GSM Error */
	phnErrLidClosed,                		/**< 0x4086 */
	phnErrSATUnavailable,           		/**< 0x4087  GSM Error */
	phnErrSATInactive,              		/**< 0x4088  GSM Error */
	phnErrUNUSED,                   		/**< 0x4089  GSM Error */
	phnErrRadioNotAvailable,        		/**< 0x408a  GSM Error */
	phnErrIntermediateResult,       		/**< 0x408b  GSM Error */
	phnErrUnexpectedResponse,       		/**< 0x408c  GSM Error */
	phnErrDuplicatePage,            		/**< 0x408d  GSM Error */
	phnErrFirmwareBootNotInprogress,		/**< 0x408e  GSM Error */
	phnErrFirmwareBootInprogress,			/**< 0x408f  GSM Error */
		/** These error codes map directly to Wismo error
		    codes, but maybe could be used by other radios? */
	phnErrMMFailed,					/**< 0x4090  GSM Error */
	phnErrLowerLayer,				/**< 0x4091  GSM Error */
	phnErrCPError,					/**< 0x4092  GSM Error */
	phnErrCommandInProgress,			/**< 0x4093  GSM Error */
	phnErrSATNotSupported,				/**< 0x4094  GSM Error */
	phnErrSATNoInd,					/**< 0x4095  GSM Error */
	phnErrNeedResetModule,				/**< 0x4096  GSM Error */
	phnErrCOPSAbort,				/**< 0x4097  GSM Error */


	/** CSD CEER Errors */
	phnErrCSDUnassignedNumber,         		/**< 0x4098  GSM Error */
	phnErrCSDNoRouteToDestination,     		/**< 0x4099  GSM Error */
	phnErrCSDChannelUnacceptable,     	 	/**< 0x409a  GSM Error */
	phnErrCSDOperatorBarring,          		/**< 0x409b  GSM Error */
	phnErrCSDNumberBusy,               		/**< 0x409c  GSM Error */
	phnErrCSDNoUserResponse,           		/**< 0x409d  GSM Error */
	phnErrCSDNoAnswer,                 		/**< 0x409e  GSM Error */
	phnErrCSDCallRejected,             		/**< 0x409f  GSM Error */
	phnErrCSDNumberChanged,            		/**< 0x40a0  GSM Error */
	phnErrCSDDestinationOutOfOrder,    		/**< 0x40a1  GSM Error */
	phnErrCSDInvalidNumberFormat,      		/**< 0x40a2  GSM Error */
	phnErrCSDFacilityRejected,         		/**< 0x40a3  GSM Error */
	phnErrCSDNoCircuitAvailable,       		/**< 0x40a4  GSM Error */
	phnErrCSDNetworkOutOfOrder,        		/**< 0x40a5  GSM Error */
	phnErrCSDTempFailure,              		/**< 0x40a6  GSM Error */
	phnErrCSDSwitchingCongestion,      		/**< 0x40a7  GSM Error */
	phnErrCSDAccessInfoDiscarded,      		/**< 0x40a8  GSM Error */
	phnErrCSDReqCircuitNotAvailable,   		/**< 0x40a9  GSM Error */
	phnErrCSDResourceNotAvailable,     		/**< 0x40aa  GSM Error */
	phnErrCSDQOSNotAvailable,          		/**< 0x40ab  GSM Error */
	phnErrCSDFacilityNotSubscribed,    		/**< 0x40ac  GSM Error */
	phnErrCSDIncomingCallsBarredCUG,   		/**< 0x40ad  GSM Error */
	phnErrCSDBearerNotCapable,         		/**< 0x40ae  GSM Error */
	phnErrCSDBearerNotAvailable,       		/**< 0x40af  GSM Error */
	phnErrCSDServiceNotAvailble,       		/**< 0x40b0  GSM Error */
	phnErrCSDBearerNotImplemented,     		/**< 0x40b1  GSM Error */
	phnErrCSDACMOutOfRange,            		/**< 0x40b2  GSM Error */
	phnErrCSDFacilityNotImplemented,   		/**< 0x40b3  GSM Error */
	phnErrCSDRestrictedBearer,         		/**< 0x40b4  GSM Error */
	phnErrCSDServiceNotImplented,      		/**< 0x40b5  GSM Error */
	phnErrCSDInvalidTransID,           		/**< 0x40b6  GSM Error */
	phnErrCSDUserNotMemberCUG,         		/**< 0x40b7  GSM Error */
	phnErrCSDIncompatibleDestination,  		/**< 0x40b8  GSM Error */
	phnErrCSDInvalidTransitNetwork,    		/**< 0x40b9  GSM Error */
	phnErrCSDSemanticallyIncorrectMessage, 		/**< 0x40ba  GSM Error */
	phnErrCSDInvalidMandatoryInfo,     		/**< 0x40bb  GSM Error */
	phnErrCSDMessageTypeNotImplemented,		/**< 0x40bc  GSM Error */
	phnErrCSDMessageTypeNotCompatible, 		/**< 0x40bd  GSM Error */
	phnErrCSDInfoNotImplemented,       		/**< 0x40be  GSM Error */
	phnErrCSDIEError,                  		/**< 0x40bf  GSM Error */
	phnErrCSDMessageNotCompatible,     		/**< 0x40c0  GSM Error */
	phnErrCSDRecoveryOnTimerExpiry,    		/**< 0x40c1  GSM Error */
	phnErrCSDProtocolError,            		/**< 0x40c2  GSM Error */
	/** GPRS CEER Errors */
	phnErrGPRSRoamingNotAllowed,       		/**< 0x40c3  GSM Error */
	phnErrGPRSNetworkRequestDetach,    		/**< 0x40c4  GSM Error */
	phnErrGPRSNoService,               		/**< 0x40c5  GSM Error */
	phnErrGPRSNoAccess,                		/**< 0x40c6  GSM Error */
	phnErrGPRSServiceRefused,          		/**< 0x40c7  GSM Error */
	phnErrGPRSNetworkRequestPDPDeactivate,   	/**< 0x40c8  GSM Error */
	phnErrGPRSPDPDactivateLCCLinkActivation, 	/**< 0x40c9  GSM Error */
	phnErrGPRSPDPDeactivateNwkRactivate,     	/**< 0x40ca  GSM Error */
	phnErrGPRSPDPDactivateGMMAbort,          	/**< 0x40cb  GSM Error */
	phnErrGPRSPDPDeactivateSNDCPFailure,     	/**< 0x40cc  GSM Error */
	phnErrGPRSPDPActivateFailGMMError,       	/**< 0x40cd  GSM Error */
	phnErrGPRSPDPActivateFailNetReject,      	/**< 0x40ce  GSM Error */
	phnErrGPRSPDPActivateFailNoNSAPI,        	/**< 0x40cf  GSM Error */
	phnErrGPRSPDPActivateFailSMRefuse,       	/**< 0x40d0  GSM Error */
	/** MORE CSD Errors */
	phnErrCSDFDNError,                       	/**< 0x40d1  GSM Error */
	phnErrCSDCallOperationNotAllowed,        	/**< 0x40d2  GSM Error */
	phnErrCSDCallBarringOutgoing,            	/**< 0x40d3  GSM Error */
	phnErrCSDCallBarringIncoming,            	/**< 0x40d4  GSM Error */
	phnErrCSDCallImpossible,                 	/**< 0x40d5  GSM Error */
	phnErrCSDLowerLayerFailure,              	/**< 0x40d6  GSM Error */
	/** More GPRS Errors */
	phnErrGPRSAPNMissing,                    	/**< 0x40d7  GSM Error */
	/** More Errors */
	phnErrNoCarrier,                         	/**< 0x40d8  GSM Error */
	phnErrSMSFDNError,				/**< 0x40d9  GSM Error : SMS Fixed Dialing error (CMS ERROR: 531) */
	phnErrNullParam,			    	/**< 0x40da GSM Error : provided parameter is unexpectedly null */
	phnErrBadServiceCode,		    		/**< 0x40db GSM Error : intermediate response has unexpected service code */
	phnErrBadATResult,			    	/**< 0x40dc GSM Error : AT response from radio is missing or invalid */
	phnErrBadATCmd,				    	/**< 0x40dd GSM Error : AT cmd to radio is missing or invalid */
	phnErrSIMDataDownload,				/**< 0x40de GSM Error : SIM Data Download error (CMS ERROR: 213) */

	/**
 	 *  0x40df to 0x40FF are reserves for GSM errors in the original Treo's
	 *  Phone Library
 	 */



	/**
         *  IMPORTANT: the last valid error code for this error class is 0x40FF.
	 *  If you exceed this range, you will collide with the next error class
	 *  that belongs to PalmSource, and will have all kinds of conflicts, such
	 *  as wrong error strings and wrong interpretation of errors by
	 *  applications.
	 */


	// Add new error codes before this one
	phnErrLAST_1
  };


/**
 *  NOTE: The last error code allowed in Phone library error class #1 is
 *  0x40FF. We reserve a second error base to allow for more error codes
 *  below.  hsPhoneLibErrorClass_2 is 0x7300.
 */
enum PHNErrorCodeRange_2
  {
	phnErrorClass_2 = hsPhoneLibErrorClass_2,

	// NOTE: phnErrSaveNV was assigned an incorrect value of 0x4150 on Treo300.
	// This placed it and all subsequent error codes in Phone Library into
	// the error class 0x4100.  We're not allowed to use the error class 0x4100
	// because it belongs to PalmSource.  Per approval of Arun Mathias, and
	// Kiran Prasad we're redefining these error codes to start at the error
	// base hsPhoneLibErrorClass_2, which belongs to Handspring.  According to Arun, the
	// error codes which were defined starting with 0x4150 were low-level
	// error codes that were only used internally.
	//
	phnErrSaveNV,			/**< 0x7301 CDMA Error : Fail to save params to Modem's NV. */
    phnErrReadNV,                   	/**< 0x7302 CDMA Error : Fail to read params from Modem's NV. */
    phnErrFiller4,                  	/**< 0x7303 Filler -- you can use this error but dont remove. */
    phnErrFiller5,                  	/**< 0x7304 Filler -- you can use this error but dont remove. */
//    phnErrCmdCannotBeActioned,    	/**< was 0x4154 CDMA Error : Fail to send command to modem. */
// ***** DUPLICATE *****
    phnErrDuplicatePlaceHoldler,	/**< 0x7305 ************************************************* */
    phnErrFiller6,                  	/**< 0x7306 Filler -- you can use this error but dont remove. */
    phnErrBatteryLowAlert,         	/**< 0x7307 CDMA Error : Phone battery is under the first low alert level. */
    phnErrBatteryCharging,          	/**< 0x7308 CDMA Error : Phone battery is being charged. */
    phnErrBadCRC,			/**< 0x7309 CDMA Error : bad CRC observed */
    phnErrNoAppRegistered,          	/**< 0x730a CDMA Error : Need to have at least an app to register for the corresponding service. */

    /** SOUND Management Errors */
    phnErrSndDelClass,              	/**< 0x730b CDMA Error : Should never send this value!!! */
    phnErrSndDelInvalidRingerID,    	/**< 0x730c CDMA Error : The ringer index is not a valid ringer ID index */
    phnErrSndDelUnusedRingerID,     	/**< 0x730d CDMA Error : The ringer index does not contain an active ringer idx to delete */
    phnErrSndDelUndeletableRingerID,	/**< 0x730e CDMA Error : The ringer index is a perminent ringer index */
    phnErrSndDelInternalMdmError,   	/**< 0x730f CDMA Error : An internal modem error occurred when attempting to delete the file. */
    phnErrSndSaveClass,             	/**< 0x7310 CDMA Error : Should never send this value!!! */
    phnErrSndSaveFileNameInvalid,   	/**< 0x7311 CDMA Error : The null terminated string was not found as the file name. */
    phnErrSndSaveBadDataLen,        	/**< 0x7312 CDMA Error : The file data length and/or the block data length is invalid. */
    phnErrSndSaveBadRingerID,       	/**< 0x7313 CDMA Error : The ringer index specified is invalid (not a write-able index or out of range) */
    phnErrSndSaveBadSoundType,      	/**< 0x7314 CDMA Error : The file type is invalid or not currently supported */
    phnErrSndSaveOutOfMem,          	/**< 0x7315 CDMA Error : First block of file could not be written due to limited memory */
    phnErrSndSaveInternalModem,     	/**< 0x7316 CDMA Error : An internal modem error occurred when attempting to download this */
                                    	/**< block of the file (file is discarded). */
    phnErrSndSaveBadRsp,            	/**< 0x7317 CDMA Error : Modem thinks that the file is downloaded completely, but the host still */
                                    	/**< has not sent all or vice versa. */
    phnErrSndSaveContClass,         	/**< 0x7318 CDMA Error : Should never use this value */
    phnErrSndSaveContBadDataLen,    	/**< 0x7319 CDMA Error : The block data length of the file data in this block is invalid. */
    phnErrSndSaveContSeqNum,        	/**< 0x731a CDMA Error : The sequence number is not in series with previous file download command. */
    phnErrSndSaveContOutOfMem,      	/**< 0x731b CDMA Error : Next block of file could not be written due to limited memory. */
    phnErrSndSaveContInternalModem, 	/**< 0x731c CDMA Error : An internal modem error occurred when attempting to download this block of the file. */

    phnErrModemBusy,			/**< 0x731d */
    phnErrModemOffline,			/**< 0x731e */
    /** IOTA errors */
    phnErrIOTACommitFailed,         	/**< 0x731f CDMA Error : IOTA commit failed */
    phnErrIOTACommitInProgress,     	/**< 0x7320 CDMA Error : IOTA commit is in progress */
    phnErrIS683ProtRevNotSupported, 	/**< 0x7321 CDMA Error : The IOTA protocol revision is not supported */
    phnErrInvalidNAM,               	/**< 0x7322 CDMA Error : The NAM selected is invalid */
    phnErrNAMMismatched,            	/**< 0x7323 CDMA Error : The NAM selected is not the same as the NAM currently used in the current session */
    phnErrIOTASessionActive,        	/**< 0x7324 CDMA Error : An IOTA session is active */
    phnErrIOTANoSession,            	/**< 0x7325 CDMA Error : No IOTA session currently */
    phnErrNVUnInitialized,		/**< 0x7326 CDMA Error : Uninitialized NV item */
	phnErrNvBadItem,		/**< 0x7327 CDMA Error : Bad or restricted NV item */


	phnErrPoweringDown,		/**< 0x7328 Error: The radio is currently */
					/**< powering down and we can not do anything */
	phnErrDenied,			/**< 0x7329 Request denied */
	phnErrNotYetSupported,          /**< 0x732A Feature not yet supported */
	phnErrSMSMemoryFull,		/**< 0x732B No more memory to receive incoming SMS */
	phnErrSMSMemoryOK,		/**< 0x732C There is now enough memory to receive incoming SMS */
	phnErrCauseCode16,		/**< 0x732D GSM04.08: Cause code 16 */
    phnErrMissingCarKitDriver, /**< 0x732E The wired car kit driver is missing */ 

	// Add new error codes before this one
	phnErrLAST_2
  };

#endif
