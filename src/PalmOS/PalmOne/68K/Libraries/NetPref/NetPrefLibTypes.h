/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/**
 * @ingroup NETPREF
 */

/**
 * @file 	NetPrefLibTypes.h
 * @version 1.0
 * @date    12/12/2001
 *
 * @brief This file contains public data types exported by the NetPref Library
 *
 */

/*
 * @private author   Vitaly Kruglikov
 */


#ifndef _NET_PREF_LIB_TYPES_H_
#define _NET_PREF_LIB_TYPES_H_

/**
 * @name Current library API version number
 * @brief Returned by NetPrefLibVersionGet()
 */
/*@{*/
#define netPrefLibVersionMajor	  0
#define	netPrefLibVersionMinor	  2
#define netPrefLibVersionBugFix	  0
/*@}*/

// Changes by minor version number:
//
// 0.1 -- original
// 0.2 -- added functions from NetPrefUtils.c to NetPrefLib API - this
//        change obsoletes most functions in NetPrefUtils.c. The obsoleted
//		  functions in NetPrefUtils.c have been suppressed with conditional
//		  compillation directive.



// Opaque structures
struct NetPrefContextTypeTag;
struct NetPrefRecordTypeTag;



/** Record field identifiers */
typedef UInt16 NetPrefRecFieldEnum;

/**
 * @name Required Macros for Record Field Identifier
 * @brief Define the required macros and then include NetPrefLibFieldInfoTable.h in
 *        order to define the record field indentifier values (see
 *        NetPrefLibFieldInfoTable.h for usage notes and currently defined fieldIDs)
 */
/*@{*/
#define NetPrefRecFieldInfoPreamble()

#define NetPrefRecFieldInfoFIRST(fieldID, idValue)		fieldID = idValue,

#define NetPrefRecFieldInfoGrpStart(fieldID, idValue)	fieldID = idValue,

#define NetPrefRecFieldInfoGrpEnd(fieldID)				fieldID,

#define NetPrefRecFieldInfoExtended(fieldID, idValue, type, maxSize, flags, tagText, tagSize) \
														fieldID = idValue,

#define NetPrefRecFieldInfoLegacy(fieldID, idValue, type, maxSize, flags, tagText, tagSize) \
														fieldID = idValue,

#define NetPrefRecFieldInfoLAST(fieldID)				fieldID
/*@}*/

enum
  {
  	// undefine this to insure the #include is always processed
 	#ifdef __NETPREFLIBFIELDINFOTABLE_H__
 		#undef __NETPREFLIBFIELDINFOTABLE_H__
 	#endif

  	// See NetPrefLibFieldInfoTable.h for usage notes
  	#include <68K/Libraries/NetPref/NetPrefLibFieldInfoTable.h>

 	// In performing the following error check, we take advantage of
 	// our inside knowledge that NetPrefLibFieldInfoTable.h undefines
 	// the macros that we defined above
 	#ifdef NetPrefRecFieldInfoLAST
 	  #error inclusion of NetPrefLibFieldInfoTable.h failed
 	#endif

  };



/** NetPref Record ID type */
typedef UInt32	NetPrefRecIDType;



/** Field type identifiers */
typedef UInt16	NetPrefFieldTypeEnum;
enum
  {
	netPrefFieldTypeStr		= 1,  /**< ascii string -- size must include zero - terminator*/
	netPrefFieldTypeUInt8	= 2,
	netPrefFieldTypeUInt16	= 3,
	netPrefFieldTypeUInt32	= 4,
	netPrefFieldTypeBin		= 5,  /**< binary*/
	netPrefFieldTypeZStrList= 6   /**< string list: list of zero-terminated strings terminated by empty string. Size must include zero-terminator*/
  };

/** Service Medium identifiers (used with netPrefRecFieldSvcMediumAlias) */
typedef UInt16 NetPrefSvcMediumEnum;
enum
  {
	netPrefSvcMediumUnknown = 0,	/**< reserve 0*/
	netPrefSvcMediumDialUp	= 1,	/**< basic dial-up - communication driver selection determines service attributes [DEFAULT]*/
	netPrefSvcMediumDirect	= 2,	/**< basic, non-modem type of connection*/
	netPrefSvcMedium1xRTT	= 3,	/**< CDMA 3G PDP service*/
	netPrefSvcMediumGPRS	= 4,	/**< GSM 2.5G PDP service*/
	netPrefSvcMediumQNC		= 5,	/**< CDMA Quick Net Connect*/
	netPrefSvcMediumGISDN	= 6,	/**< GSM ISDN over CSD service*/
	netPrefSvcMediumGCSD	= 7,	/**< GSM circuit switched data service (AKA wireless dial-up)*/
	netPrefSvcMediumCCSD	= 8,	/**< CDMA circuit switched data service (AKA wireless dial-up)*/
	netPrefSvcMediumOEM		= 9,	/**< This service is a place-holder for an external OEM plugin*/
	netPrefSvcMediumLAST			/**< ALWAYS KEEP THIS ENTRY AT THE END!!!*/
  };

/** Service Binding identifiers (use with netPrefRecFieldBinding); */
typedef UInt16 NetPrefSvcBindingEnum;
enum
  {
	netPrefSvcBindingUnknown  = 0,	/**< reserve 0*/
	netPrefSvcBindingIOTA	  = 1,	/**< Provisioned via IOTA*/
	netPrefSvcBindingCCSM	  = 2,	/**< Default Wireless profile based on the CCSM tables */
	netPrefSvcBindingCustom	  = 3,	/**< Created by user or "user agent" on behalf of the user [DEFAULT]*/
	netPrefSvcBindingLAST			/**< ALWAYS KEEP THIS ENTRY AT THE END!!!*/
  };


/** 1xRTT Tunneling identifiers (use with netPrefRecField1xRTTTunneling) */
typedef UInt16 NetPref1xRTTTunnelingEnum;
enum
  {
	netPref1xRTTTunnelingUnknown = 0,	/**< reserve 0*/
	netPref1xRTTTunnelingReverse = 1,	/**< reverse tunneling*/
	netPref1xRTTTunnelingForward = 2,	/**< forward tunneling*/
	netPref1xRTTTunnelingLAST			/**< ALWAYS KEEP THIS ENTRY AT THE END!!!*/
  };

/** Default 1xRTT Tunneling value */
#define netPref1xRTTTunnelingDefault  netPref1xRTTTunnelingReverse


/**
 * Service Target identifiers used with NetPrefDefaultTargetGet()
 * and NetPrefDefaultTargetSet().
 *
 * NOTE: These values need to correlate with NetPrefSvcGatewayType.
 *
 * IMPORTANT: These settings are to be used
 * for carrier-provisioned services only, and are applied by the
 * system's configurator app!!!  Others MUST treat these settings
 * as read-only!
 */
typedef UInt16 NetPrefSvcTargetEnum;
enum
  {
	netPrefSvcTargetUnknown	  = 0,	/**< reserve 0*/
	netPrefSvcTargetInternet  = 1,	/**< provides access to the Internet*/
	netPrefSvcTargetWAP		  = 2,	/**< provides access to WAP*/
	netPrefSvcTargetReserved1 = 3,	/**< reserved*/
	netPrefSvcTargetMMS		  = 4,	/**< provides access to MMS*/
	netPrefSvcTargetBrowser	  = 5,	/**< provides access to Web via web browser*/
	netPrefSvcTargetMail	  = 6,	/**< provides access to e-mail*/
	netPrefSvcTargetIM		  = 7,	/**< provides access to Instant Messaging*/
	netPrefSvcTargetDownloads = 8,	/**< provides access to Downloads*/
	netPrefSvcTargetCorporate = 9,	/**< provides access to Corporate network*/
	netPrefSvcTargetPictureMail = 10, /**< provides access to Picture Mail*/
	netPrefSvcTargetLAST			/**< ALWAYS KEEP THIS ENTRY AT THE END!!!*/
  };

/** Record field view attributes bitwise-or'ed together */
typedef UInt32	NetPrefRecFieldViewType;

/** @name Record File View Attributes */
/*@{*/
#define netPrefRecFieldViewRead		0x00000001UL  /**< read allowed by application*/
#define netPrefRecFieldViewWrite	0x00000002UL  /**< write allowed by application*/
#define netPrefRecFieldViewVisible	0x00000004UL  /**< field value is user-visible*/
#define netPrefRecFieldViewEditable	0x00000008UL  /**< field value is user-editable*/
  /** Field value is stored exernally -- such as in the CCSM table, radio module's
   *  flash memory, etc., i.e. not in the NetPref database
   */
#define netPrefRecFieldViewExternal	0x00000010UL
#define netPrefRecFieldViewCache	0x00000020UL  /**< the external field value is cached locally*/
#define netPrefRecFieldViewPrivate	0x00000040UL  /**< PRESENTLY NOT RESPECTED; If set, do not display the actual value to the user*/
/*@}*/

// WARNING: THIS CONSTANT IS FOR INTERNAL USE ONLY -- IT WILL CHANGE
// IN FUTURE VERSIONS!!! (used for error-checking only).
#define netPrefRecFieldViewCurrentlyValidFlags		\
					(	netPrefRecFieldViewRead		\
					  |	netPrefRecFieldViewWrite	\
					  |	netPrefRecFieldViewVisible	\
					  | netPrefRecFieldViewEditable	\
					  | netPrefRecFieldViewExternal	\
					  | netPrefRecFieldViewCache	\
					  | netPrefRecFieldViewPrivate)

/** Special value to pass in the flagsToClear parameter to NetPrefRecFieldViewSet in order to clear all field view flags */
#define netPrefRecFieldViewClearAll	0xFFFFFFFFUL

/** Record field attributes (retrieved via NetPrefRecFieldAttrsGet) */
typedef UInt32	NetPrefRecFieldAttrType;

/** @name Record File Attributes */
/*@{*/
	/** R/O: set if the field's
	 * data or flags are part of the record's field
	 * set.
	 */
	#define netPrefRecFieldAttrInSet	0x10000000UL

	/** R/O: set if the field's
	 * data or flags have been modified in the current
	 * instance of the record object
	 */
	#define netPrefRecFieldAttrDirty	0x20000000UL
/*@}*/

/** Special values used with netPrefRecFieldCloseWaitTime.  Other, standard,
 *  values are expressed in milliseconds.
 *  IMPORTANT: Beginning with Palm OS 5.0, netPrefRecFieldInactivityTimer
 *  must be used to configure the idle timeout of the TCP/IP stack. The
 *  "CloseWait" settting is used to control NetLib's CloseWait behavior
 *  only
 */
typedef UInt32 NetPrefNetCloseWaitEnum; // was NetPrefNetTimeoutEnum
enum
  {
	/** Use this value to configure the network interface to be shut down
	 * when the device (i.e. LCD screen) powers off.
	 */
	netPrefNetCloseWaitEndOnPowerOff	= 0xffffffffUL, // was netPrefNetTimeoutOnPowerOff

	/** Use this value to configure the network interface to *not* be shut down
	 * when the device (i.e. LCD screen) powers off.
	 */
	netPrefNetCloseWaitMaintainSession	= 0xfffffffeUL, // was netPrefNetTimeoutOnWirelessOff

	/** Use this value to configure the CloseWait behavior with the carrier-
	 * default value.
	 */
	netPrefNetCloseWaitCarrierDefault	= 0xfffffffdUL  // was netPrefNetTimeoutCarrierDefault
  };


/** Special values used with netPrefRecFieldInactivityTimer.
 *  Other, standard, values are expressed in seconds.
 */
typedef UInt16 NetPrefNetIdleTimeoutEnum;
enum
  {
	netPrefNetIdleTimeoutNever			= (UInt16) 0	/**< Never time out due to data idleness*/
  };



/** Record protection settings -- used with netPrefRecFieldRecProtection */
typedef UInt32 NetPrefRecProtectionType;
enum
  {
	/** Record deletion options*/
	netPrefRecProtectionDeletionMask				  = 0x00000007UL,
	netPrefRecProtectionDeletionAfterWarning		  = 0x0UL,
	netPrefRecProtectionDeletionNotAllowed			  = 0x1UL,


	/** Duplication options*/
	netPrefRecProtectionDuplicationMask				  = (0x7UL << 3),
	netPrefRecProtectionDuplicationAllowed			  = (0x0UL << 3),
	netPrefRecProtectionDuplicationNotAllowed		  = (0x1UL << 3),


	/** Editing options*/
	netPrefRecProtectionEditingMask					  = (0x7UL << 6),
	netPrefRecProtectionEditingAllowed				  = (0x0UL << 6),
	netPrefRecProtectionEditingNotAllowed			  = (0x1UL << 6),
	netPrefRecProtectionEditingAfterWarning			  = (0x2UL << 6)
  };



/**
 * Record Service gateway type -- used with netPrefRecFieldSvcGatewayType;
 * 1 or more values can be bitwise or'ed together to mark
 * a record as supporting those gateway types; absense of
 * any service type falgs implies Internet.
 *
 * NOTE: These values need to correlate with NetPrefSvcTargetEnum.
 *
 * IMPORTANT: These settings are to be used
 * for carrier-provisioned services only, and are applied by the
 * system's configurator app!!!  Others MUST treat these settings
 * as read-only!
 */
typedef UInt32 NetPrefSvcGatewayType;
enum
  {
	netPrefSvcGatewayUnknown		= 0UL,			/**< reserved*/
	netPrefSvcGatewayInternet		= 0x00000001UL, /**< Generic Internet "gateway"*/
	netPrefSvcGatewayWAP			= 0x00000002UL, /**< Generic WAP gateway*/
	netPrefSvcGatewayPrivate		= 0x00000004UL,	/**< Application-specific "gateway"*/
	netPrefSvcGatewayMMS			= 0x00000008UL,	/**< Generic MMS "gateway"*/
	netPrefSvcGatewayBrowser		= 0x00000010UL,	/**< Web via web browser*/
	netPrefSvcGatewayMail			= 0x00000020UL,	/**< e-mail*/
	netPrefSvcGatewayIM				= 0x00000040UL,	/**< Instant Messaging*/
	netPrefSvcGatewayDownloads		= 0x00000080UL,	/**< Downloads*/
	netPrefSvcGatewayCorporate		= 0x00000100UL,	/**< Corporate network*/
	netPrefSvcGatewayPictureMail    = 0x00000200L,  /**< Picture Mail*/

	netPrefSvcGatewayLAST			  /**< ALWAYS KEEP THIS ENTRY AT THE END!!!*/
  };


/**
 * @brief NetPrefGPRSQOSType -- GPRS Quality Of Service (QOS) info
 */
typedef struct
  {
	// IMPORTANT: maintain this structure at fixed size; remove
	// space for new fields from the reserved fields, making sure
	// to adjust the reserved field size so the overall structure
	// size is preserved exactly.  Use reserved space wisely :-)

	// NOTE: "ccsmCS_QOS..." in the following comments refers to Hanspring-
	// internal data structures.  Developer support will need to provide
	// clarification for the use of these fields.

	Int8	  qosReqPrecedenceClass;  /**< see ccsmCS_QOSReqPrecClassField*/
	Int8	  qosReqDelayClass;		  /**< see ccsmCS_QOSReqDelayClassField*/
	Int8	  qosReqReliabilityClass; /**< see ccsmCS_QOSReqRelClassField*/
	Int8	  qosReqPeakThruClass;	  /**< see ccsmCS_QOSReqPeakThruClassField*/
	Int8	  qosReqMeanThruClass;	  /**< see ccsmCS_QOSReqMeanThruClassField*/
	Int8	  qosMinPrecedenceClass;  /**< see ccsmCS_QOSMinPrecClassField*/
	Int8	  qosMinDelayClass;		  /**< see ccsmCS_QOSMinDelayClassField*/
	Int8	  qosMinReliabilityClass; /**< see ccsmCS_QOSMinRelClassField*/
	Int8	  qosMinPeakThruClass;	  /**< see ccsmCS_QOSMinPeakThruClassField*/
	Int8	  qosMinMeanThruClass;	  /**< see ccsmCS_QOSMinMeanThruClassField*/
	Int8	  pdpDataCompression;	  /**< see ccsmCS_PDPDataComprField*/
	Int8	  pdpHeaderCompression;	  /**< see ccsmCS_PDPHeaderComprField*/

	UInt32	  reserved[4];			  /**< Reserved by Handspring -- init to zero*/
  }
NetPrefGPRSQOSType;


/** Options for NetPrefRecFieldSetDefineStd and NetPrefRecFieldSetDefine */
typedef UInt32	  NetPrefFieldSetDefineOptionsType;
enum
  {
	netPrefFieldSetDefineOptionNone = 0,		/**< Pass this value for "no options"*/

	// If set, NetPrefRecFieldSetDefineStd and NetPrefRecFieldSetDefine
	// will temporarily keep around the data of out-of-set fields; however,
	// it will not save such data when saving the field; this feature
	// is a convenience for the Network panel that allows the user to
	// experiment with different connection mediuma (dial-up, direct, 3G, etc.)
	// without losing the data that is in one service medium's field set,
	// but not in another.
	netPrefFieldSetDefineOptionKeepData = 0x00000001UL
  };



// ----------------------------------------------------------------------------
// Fallback info -- used with netPrefRecFieldFallbackInfo
// ----------------------------------------------------------------------------

// NOTE: The Handspring NetMaster library implements the fallback logic.


/** Fallback algorithm */
typedef UInt8	  NetPrefFallbackAlgrorithmEnum;
enum
  {
	/**  Basic algorithm: each time: try primary; if fails, try fallback; */
	netPrefFallbackAlgrorithmBasic = 0,

	/**
	 * Mutually exclusive, sticky algorithm: The primary and fallback record
	 * MUST be cross-linked -- each MUST specify the other as its fallback;
	 * both records MUST also be tagged with this algorithm.  Regardless of
	 * which of the cross-linked services is requested during login (unless
	 * "primary only" is requested -- see NetMaster API), the algorithm will
	 * attempt to connect with the one that succeeded last time. The first time
	 * or if both failed last time, it will try the requested service first. If
	 * the attempt fails, it will try the one specified as its fallback.
	 */
	netPrefFallbackAlgrorithmMutexSticky = 1,



	// ADD NEW VALUES *BEFORE* THIS ONE
	netPrefFallbackAlgrorithmLAST
  };


typedef UInt8 NetPrefFallbackStatusFlagsType;
enum
  {
	// If set, inidicates that NetPrefFallbackInfoType.status has
	// been initialized.
	netPrefFallbackStatusFlagInitialized = 0x01

  };


/** Flags used in the "Mutex Sticky" fallback algoritm status structure. */
typedef UInt16 NetPrefMutexStickyFBAStatusEnum;
enum
  {
	netPrefMutexStickyFBAStatusUnknown		= 0, /**< Indicates that the last login status of the corresponding network service is unknown.*/

	netPrefMutexStickyFBAStatusFailed		= 1, /**< Indicates that the last login attempt using the corresponding network service failed.*/

	netPrefMutexStickyFBAStatusSucceeded	= 2  /**< Indicates that the last login attempt using the corresponding network service succeeded.*/
  };


/**
 * @brief Fallback info structure
 */
typedef struct
  {
	/** Fallback algorithm ID.
	 *
	 * IMPORTANT: when changing fallback algorithm, the
	 * caller MUST zero-initialize the rest of the NetPrefFallbackInfoType
	 * structure.
	 */
	NetPrefFallbackAlgrorithmEnum	algorithm;

	/** Fallback algoritm status. There is a field here for every fallback
	 * algorithm that requires status to be maintained.
	 *
	 * THIS STATUS STRUCTURE IS FOR USE BY THE HANDSPRING NetMaster LIBRARY
	 * ONLY!!!
	 */
	NetPrefFallbackStatusFlagsType	fbStatusFlags;

	union
	  {
		/** Status for netPrefFallbackAlgrorithmMutexSticky fallback algorithm */
		struct NetPrefMutexStickyFBAStatusTypeTag
		  {
			NetPrefMutexStickyFBAStatusEnum  statusID;
		  }
		mutexSticky;

	  }
	status;
  }
NetPrefFallbackInfoType;


// ----------------------------------------------------------------------------
// Platform IDs
// ----------------------------------------------------------------------------
typedef UInt16	  NetPrefPlatformEnum;
enum
  {
	netPrefPlatformUnknown	  = 0,

	netPrefPlatformGSM		  = 1,
	netPrefPlatformCDMA		  = 2,

	netPrefPlatformLAST
  };



/**
 * Max size of a phone number, *not including* zero-terminator
 */
 #define netPrefMaxPhoneStringSize 80



#endif // _NET_PREF_LIB_TYPES_H_
