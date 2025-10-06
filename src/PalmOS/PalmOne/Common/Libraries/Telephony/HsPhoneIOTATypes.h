/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/** @ingroup Telephony
 *
 */


/**
 *
 * @file 	HsPhoneIOTATypes.h
 *
 * @brief  Header File for Phone Library API ---- IOTA CATEGORY
 *
 * NOTES:
 * 	All implementations of the Handspring Phone Library support a common API.
 * 	This API is broken up into various categories for easier management.  This file
 * 	defines the IOTA category.  These API calls are used for features specific to
 * 	IOTA operation on CDMA networks.
 */


#ifndef HS_PHONEIOTA_H
#define HS_PHONEIOTA_H
#include <PalmOS.h>
#include <PalmTypes.h>
#ifndef __CORE_COMPATIBILITY_H__
#include <PalmCompatibility.h>
	/** workaround for differing header files in sdk-3.5 and sdk-internal */
	#ifndef __CORE_COMPATIBILITY_H__
	#define __CORE_COMPATIBILITY_H__
	#endif
#endif
#include <Common/Libraries/Telephony/HsPhoneTraps.h>     /** trap table definition for phone library calls */
#include <Common/Libraries/Telephony/HsPhoneErrors.h>    /** error codes returned by phone library functions */
#include <Common/Libraries/Telephony/HsPhoneTypes.h>


/**
 *
 **/
typedef enum
{
  phnIOTAUnknown = 0,		/**<		*/
	phnIOTAStart,		/**<		*/
	phnIOTAEnd,		/**<		*/
	phnIOTACommit,		/**<		*/
	phnIOTANAMSelect	/**<		*/
}
PhnIOTAStatus;

/**
 * @brief
 *
 **/
typedef struct
{
  PhnIOTAStatus		status;			/**< the status that the modem changes into */
  UInt8			namInUse;		/**< namInUse4IOTA currently */
  PHNErrorCode		error;			/**< == 0 normally, only != 0 for phnIOTAEnd and phnIOTACommit if Commit failed. */
}
PhnIOTAReportType;




// IOTA defines
/**
 * @name Text Header elements
 *
 **/
/*@{*/
#define IOTAHeaderEscapeSeq     "X-Wap-Application-Id: "                            /**< start sequence of the header */
#define IOTATextHeader          "X-Wap-Application-Id: x-wap-application:iota.ua"   /**< beginning of an IOTA header */
#define IOTATextHeaderLength    47                                                  /**< length of the header */

#define IOTATextHeaderTerminator   '\n' 		/**< IOTA headers terminate at the line feed */
/*@}*/

#define phnIOTAEvent         'Hita'		/**<		*/


typedef UInt8 PhnIOTAItemType;  /**< let's hope it never gets beyond 256 items */
/**
 * Items stored in the radio NV (mostly for Sprint support).
 * Some of these items may be used for other carriers in the future.
 * Most of these items will be match the initial TIL implementation
 * since we are focusing on Sprint support.
 * We may be able to leverage the NVFlash support in the future and
 * get away from the radio NV for most of these (unless we need UPST support).
 **/
typedef enum
{
  phnIOTAItemBootURL                    = 0x00, /**< provisioning URL */
  phnIOTAItemBootNAIURL                 = 0x01, /**< provisioning URL */
  phnIOTAItemTrustedDomain              = 0x02, /**< trusted domain for provisioning.  All URLs must come from this */
  phnIOTAItemHomepageURL                = 0x03, /**< default browser homepage */
  phnIOTAItemVendingURL                 = 0x04, /**< vending URL */
  phnIOTAItemDownloadServerURL          = 0x05, /**< download server URL */
  phnIOTAItemDownloadServerLock         = 0x06, /**< state of items downloaded from the download server (locked or not) */
  phnIOTAItemConnectionTimeout          = 0x07, /**< timeout for IOTA connections */
  phnIOTAItemPriWAPPushProxyGatewayURL  = 0x08, /**< Primary WAP push gateway */
  phnIOTAItemSecWAPPushProxyGatewayURL  = 0x09, /**< Secondary WAP push gateway */

  // The following items are indexed for each PDP profile
  // Sprint has 2 profiles, so these are indexed 0..1
  phnIOTAItemPriProxyWDPAddress         = 0x0A, /**< WDP address. Translate to the NGG for profile 1 */
  phnIOTAItemSecProxyWDPAddress         = 0x0B, /**< WDP address. Translate to the NGG for profile 1 */
  phnIOTAItemPriSIPConfigServer         = 0x0C, /**< Primary Config server */
  phnIOTAItemSecSIPConfigServer         = 0x0D, /**< Secondary Config server */
  phnIOTAItemPriSIPEdgeProxyGateway     = 0x0E, /**< Primary SIP Proxy Edge gateway */
  phnIOTAItemSecSIPEdgeProxyGateway     = 0x0F, /**< Secondary SIP Proxy Edge gateway */
  phnIOTAItemPriDataExchangeURL         = 0x10, /**< Primary Data Exchange URL */
  phnIOTAItemSecDataExchangeURL         = 0x11, /**< Secondary Data Exchange URL */
  phnIOTAItemPriDataGatewayURL          = 0x12, /**< Primary Data Gateway URL */
  phnIOTAItemSecDataGatewayURL          = 0x13, /**< Secondary Data Gateway URL */
  phnIOTAItemPriSIPRetryTimer           = 0x14, /**< Primary SIP Retry timer */
  phnIOTAItemSecSIPRetryTimer           = 0x15, /**< Secondary SIP Retry timer */
  phnIOTAItemPriSIPRetryCount           = 0x16, /**< Primary SIP Retry count */
  phnIOTAItemSecSIPRetryCount           = 0x17, /**< Secondary SIP Retry count */
  phnIOTAItemPriIMMessageStore          = 0x18, /**< Primary IM Message store */
  phnIOTAItemSecIMMessageStore          = 0x19, /**< Secondary IM Message store */


  phnIOTAItemRootCertificate1           = 0x40, /**< Root certificate #1 */
  phnIOTAItemRootCertificate2           = 0x41, /**< Root certificate #2 */
  phnIOTAItemRootCertificate3           = 0x42, /**< Root certificate #3 */
  phnIOTAItemRootCertificate4           = 0x43, /**< Root certificate #4 */

  // Sprint items that used to be hard-coded in the IOTA app but now
  // moved here, since different radios may possibly support different values.
  phnIOTAItemPRLStorageSize             = 0x50, /**< max PRL size radio supports */

  // items that are listed as Verizon requirements
  phnIOTAItemSIPUsername                = 0x51, /**< Username for SIP accounts */
  phnIOTAItemSIPPassword                = 0x52, /**< password for SIP accounts */

  // last item
  phnIOTAItemLastNVItem                 = 0x60, /**< last IOTA NV Item */

  phnIOTAItemUnknown                    = 0xFF  /**< unknown item */
}
PhnIOTAItemEnum;

#define isValidPhnIOTAItemType(i) ((i >= phnIOTAPhnBootNAIUrl) && (i <phnLibIOTAItemLast))	/**<		*/


// ========================= Begining of IOTA type section =======================

#define kIOTACertMaxBlocks              132     /**< Index should be < 12, starting from 0 */
#define kIOTACertMaxBlockLen            127	/**<		*/
/**
 * @brief
 *
 **/
typedef struct
  {
  UInt8  index;					/**<		*/
  UInt8  cert [kIOTACertMaxBlockLen];		/**<		*/
  }
PhnIOTAPhnTlsRcUrlType;

#define kIOTAProxyADDRMaxLen           20	/**<		*/
#define kIOTANaiSipEdgePxyMaxBlocks	   4	/**<		*/
/**
 * @brief
 *
 **/
typedef struct
  {
  UInt8  index;					/**<		*/
  UInt8  addr [kIOTAProxyADDRMaxLen];		/**<		*/
  }
PhnIOTAPhnNAISIPEdgeProxyType;			/**<		*/

#define kIOTAPxyAddrWdpMaxBlocks 4		/**<		*/
/**
 * @brief
 *
 **/
typedef struct
  {
  UInt8  index;					/**<		*/
  UInt8  addr [kIOTAProxyADDRMaxLen];		/**<		*/
  }
PhnIOTAPhnProxyAddrWdpType;

#define kIOTAUrlMaxLen                128	/**<		*/
#define kIOTAUrlArrayMaxLen		65	/**<		*/

#define kIOTASipCfgBlocks 4			/**<		*/
/**
 * @brief
 *
 **/
typedef struct
{
  UInt8  index;					/**<		*/
  UInt8  url [kIOTAUrlArrayMaxLen];		/**<		*/
}
PhnIOTAPhnNAISIPCfgSvr;

#define kIOTANaiPpgMaxBlocks 4			/**<		*/
/**
 * @brief
 *
 **/
typedef struct
{
  UInt8  index;					/**<		*/
  UInt8  url [kIOTAUrlArrayMaxLen];		/**<		*/
}
PhnIOTAPhnNAIPPGType;

#define kIOTANaiIMStoreMaxBlocks 8		/**<		*/
/**
 * @brief
 *
 **/
typedef struct
{
  UInt8  index;					/**<		*/
  UInt8  url [kIOTAUrlArrayMaxLen];		/**<		*/
}
PhnIOTAPhnNAIIMStore;

#define kIOTANaiSipDataxchMaxBlocks 8		/**<		*/
/**
 * @brief
 *
 **/
typedef struct
{
  UInt8  index ;				/**<		*/
  UInt8  url [kIOTAUrlArrayMaxLen] ;		/**<		*/
}
PhnIOTAPhnNAISIPDataxchType;

#define kIOTANaiSipDdatgwyMaxBlocks 8		/**<		*/
/**
 * @brief
 *
 **/
typedef struct
{
  UInt8  index ;				/**<		*/
  UInt8  url [kIOTAUrlArrayMaxLen]  ;		/**<		*/
}
PhnIOTAPhnNAISIPDatagwyType;

// Type to hold the SIP re-transmission timers - specified in seconds
#define kIOTAPhnSipRetryTimerMaxLen 9		/**<		*/
#define kIOTAPhnSipRetryTimerMaxBloacks 4	/**<		*/
/**
 * @brief
 *
 **/
typedef struct
{
  UInt8  index;					/**<		*/
  UInt8  timer [kIOTAPhnSipRetryTimerMaxLen];	/**<		*/
}
PhnIOTAPhnSIPRetryTimerType;

// Type to hold the IP re-transmission counters
#define kIOTAPhnSipRetryCntMaxLen 9		/**<		*/
#define kIOTAPhnSipRetryCntMaxBlocks 4		/**<		*/
/**
 * @brief
 *
 **/
typedef struct
{
  UInt8  index;					/**<		*/
  UInt8  cnt [kIOTAPhnSipRetryCntMaxLen];	/**<		*/
}
PhnIOTAPhnSIPRetryCntType;

/**
 * @brief Holds lock state of user's ability to change the download server URL address.
 *
 **/
typedef struct
{
  Boolean iota_ph_dls_lock  ;			/**<		*/
}
PhnIOTAPhnDlsLockType;
/**
 * @brief
 *
 **/
typedef struct
{
  UInt8  url [kIOTAUrlMaxLen] ;			/**<		*/
}
PhnIOTAPhnHomepagetype;


#define kIOTATrustedDomainMaxLen 64		/**<		*/
/**
 * @brief
 *
 **/
typedef struct
{
  UInt8  url[kIOTATrustedDomainMaxLen];		/**<		*/
}
PhnIOTAPhnTrustedDomainType;


// ========================= End of IOTA type section =======================
/**
 * Structure passed to the callbacks registered for incoming IOTA notifications
 **/
typedef struct tagIOTANotificationEventType
{
	UInt16 version;  		/**< version number to provide future backwards compatibility */

	void *headerP;    		/**< pointer to raw header */
	UInt8 headerLen;  		/**< length of headerP */
	void *dataP;      		/**< pointer to data body */
	UInt8 dataLen;    		/**< length of dataP */

	/** SMS related fields */
	UInt32 msgID; 			/**< ID into the SMS database to reference this
                  			 * message this ID is not gauranteed to be
                  			 * valid once the notification callback
                 			 * returns.  Users should make a copy of the
                 			 * msg if they want to work on it after the
                 			 * callback returns.
                 			 */
	UInt32 datetime;   		/**< date/time stamp */
	Int32  reserved2; 		/**< reserved*/
	Int32  reserved3;  		/**< reserved*/
}
IOTANotificationEventType;


#endif  // HS_PHONEIOTA_H
