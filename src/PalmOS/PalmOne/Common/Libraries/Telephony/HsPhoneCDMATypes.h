/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/** @ingroup Telephony
 *
 */


/**
 *
 * @file 	HsPhoneCDMATypes.h
 *
 * @brief  Header File for Phone Library API ---- NETWORK CATEGORY
 *
 * NOTES:
 * 	All implementations of the Handspring Phone Library support a common API.
 * 	This API is broken up into various categories for easier management.  This file
 * 	defines the CDMA category.  These API calls are used to interact with the wireless network.
 */



#ifndef _HS_PHONE_CDMA_TYPES_H__
#define _HS_PHONE_CDMA_TYPES_H__
#include <PalmOS.h>
#include <PalmTypes.h>
#ifndef __CORE_COMPATIBILITY_H__
#include <PalmCompatibility.h>
	/** workaround for differing header files in sdk-3.5 and sdk-internal */
	#ifndef __CORE_COMPATIBILITY_H__
	#define __CORE_COMPATIBILITY_H__
	#endif
#endif


#define indicatorNoCallerIDChar          '?'  /**< CDMA Only -- Means there is no caller ID */
#define indicatorBlockedCallerIDChar     '!'  /**< CDMA Only -- Means the caller ID is blocked  */

/******************************************************************************
 * PDP related defines. KEEP THE FOLLOWING DEFINES IN SYNC WITH HIONEX.H !!
 ******************************************************************************/

#define phnPDPUidLen			  15		/**< Max PDP UID length   */
#define phnMaxNAILen			  72		/**< Max length of NAI */
#define phnMaxMNHASharedSecretLen 48			/**< HI_ONEX_MAX_MN_HA_SHARED_SECRET_LEN  */
#define phnMaxMNAAASharedSecretLen 48			/**< HI_ONEX_MAX_MN_AAA_SHARED_SECRET_LEN */
#define phnMaxSvcStrLen				22	/**< HI_ONEX_MAX_SVC_STR_LEN */
#define phnMaxDialStrLen		   15		/**< HI_ONEX_MAX_DIAL_STR_LEN */
#define phnMaxMslLen				6	/**< HI_ONEX_MSL_LEN */


// Fields that were successfully updated with OTASP
#define phnOtaspMobdirCommitFlag    0x0001		/**<		*/
#define phnOtaspFTCCommitFlag       0x0002		/**<		*/
#define phnOtaspRoamListCommitFlag  0x0004		/**<		*/
#define phnOtaspSPCCommitFlag       0x0008		/**<		*/
#define phnOtaspIMSICommitFlag      0x0010		/**<		*/
#define phnOtaspNamLockCommitFlag   0x0020		/**<		*/
#define phnOtaspAkeyCommitFlag      0x0040		/**<		*/
#define phnOtasp3GPDCommitFlag      0x0080		/**<		*/
#define phnOtaspMIPStatCommitFlag   0x0100		/**<		*/
#define phnOtaspMIPRetryCommitFlag  0x0200		/**<		*/
#define phnOtaspSPCFail             0x0400		/**<		*/
#define phnOtaspCommitFlag          0x0800  		/**<		*/
#define phnOtaspInitProgReqFlag     0x1000		/**<		*/
#define phnOtaspProgInProgressFlag  0x2000		/**<		*/
#define phnOtaspSSDCommitFlag       0x4000		/**<		*/

#define phnOtaspSuccessful          0x02FF		/**<		*/
#define phnOtaspUnsuccessful        0x03FF		/**<		*/


/**
 *
 **/
typedef enum  {
    phnOneXFailPrev=0,    	/**< current used PREV does not support the type of data call attempted */
    phnOneXFailNoSrv=1,   	/**< modem did not have service when the data call was attempted */
    phnOneXFailNetworkSORej=2, 	/**< The network rejected the service option for the data call */
    phnOneXFailModemSORej=3,   	/**< The modem does not support or restricts the service option for the data call */
    phnOneXFailNetworkBusy=4,  	/**< The network was busy */
	phnOneXFailUnknown=5	/**<		*/
} PhnOneXDataFailType;

#define isValidPhnOneXDataFailType(d) ((d >= phnOneXFailPrev) && (d <= phnOneXFailUnknown))	/**<		*/


/**
 *
 **/
typedef enum{
    PhnPDPAUncert0_5_M = 0x00,		/**< Standard deviation of 0.5 meters */
    PhnPDPAUncert0_75_M,		/**< Standard deviation of 0.75 meters */
    PhnPDPAUncert1_M,			/**< Standard deviation of 1 meter */
    PhnPDPAUncert1_5_M,			/**< Standard deviation of 1.5 meters */
    PhnPDPAUncert2_M,			/**< Standard deviation of 2 meters */
    PhnPDPAUncert3_M,		     	/**< Standard deviation of 3 meters */
    PhnPDPAUncert4_M,     		/**< Standard deviation of 4 meters */
    PhnPDPAUncert6_M,		     	/**< Standard deviation of 6 meters */
    PhnPDPAUncert8_M,     		/**< Standard deviation of 8 meters */
    PhnPDPAUncert12_M,     		/**< Standard deviation of 12 meters */
    PhnPDPAUncert16_M,		     	/**< Standard deviation of 16 meters */
    PhnPDPAUncert24_M,     		/**< Standard deviation of 24 meters */
    PhnPDPAUncert32_M,	     		/**< Standard deviation of 32 meters */
    PhnPDPAUncert48_M,	     		/**< Standard deviation of 48 meters */
    PhnPDPAUncert64_M,     		/**< Standard deviation of 64 meters */
    PhnPDPAUncert96_M,     		/**< Standard deviation of 96 meters */
    PhnPDPAUncert128_M,     		/**< Standard deviation of 128 meters */
    PhnPDPAUncert192_M,     		/**< Standard deviation of 192 meters */
    PhnPDPAUncert256_M,     		/**< Standard deviation of 256 meters */
    PhnPDPAUncert384_M,     		/**< Standard deviation of 384 meters */
    PhnPDPAUncert512_M,     		/**< Standard deviation of 512 meters */
    PhnPDPAUncert768_M,     		/**< Standard deviation of 768 meters */
    PhnPDPAUncert1024_M,     		/**< Standard deviation of 1,024 meters */
    PhnPDPAUncert1536_M,     		/**< Standard deviation of 1,536 meters */
    PhnPDPAUncert2048_M,     		/**< Standard deviation of 2,048 meters */
    PhnPDPAUncert3072_M,     		/**< Standard deviation of 3,072 meters */
    PhnPDPAUncert4096_M,     		/**< Standard deviation of 4,096 meters */
    PhnPDPAUncert6144_M,     		/**< Standard deviation of 6,144 meters */
    PhnPDPAUncert8192_M,     		/**< Standard deviation of 8,192 meters */
    PhnPDPAUncert12288_M,     		/**< Standard deviation of 12,2288 meters */
    PhnPDPAUncertGREATER_12288_M,     	/**< Standard deviation that is greater than 12,288 meters */
    PhnPDPAUncertNOT_COMPUTABLE_M,     	/**< Standard deviation is not computable */
    PhnPDPAUncertMAX
} PhnPDPAUncerttype;


/**
 *
 **/
typedef enum {

	phnOneXStatusPRevNoSvc,  	/**< Service is not available. */
    phnOneXStatusPRev1, 		/**< J-STD-008 service is in use. */
	phnOneXStatusPRev3, 		/**< IS-95A service is in use. */
	phnOneXStatusPRev4, 		/**< IS-95B service is in use. */
	phnOneXStatusPRev6, 		/**< IS-2000 release 0 service is in use. */
	phnOneXStatusPRev7		/**< IS-2000 release A service is in use. */
} _PhnOneXProtRevType;

typedef UInt8 PhnOneXProtRevType;	/**<		*/

#define isValidPhnOneXProtRevType(p) (p <= phnOneXStatusPRev7)		/**<		*/


/**
 *
 **/
typedef enum {

  phnPacketDataSessionNone,   		/**< No packet data session exists. */
  phnPacketDataSessionDormant,		/**<		*/
  phnPacketDataSessionActive		/**<		*/

} _PhnPacketDataSessionType;



typedef UInt8 PhnPacketDataSessionType;		/**<		*/

#define isValidPhnPacketDataSessionType(p) (p <= phnPacketDataSessionActive)		/**<		*/

/**
 *
 **/
typedef enum {

    phnMIPSessionNone,		/**<		*/
    phnMIPSession     		/**< A Mobile IP session exists. */

} _PhnMIPSessionType;

typedef UInt8 PhnMIPSessionType;		/**<		*/

#define isValidPhnMIPSessionType(m) (m <= phnMIPSession)		/**<		*/


/**
 *
 **/
typedef struct
{
  PhnOneXProtRevType		oneXStatusPRev;		/**<		*/
  PhnPacketDataSessionType	pdSessionStatus; 	/**<		*/
  PhnMIPSessionType		mipSessionType;	/**<		*/
}PhnOneXStatus;

/**
 *
 **/
typedef struct
{
  UInt32 timeInSecs;  		/**< Current time in minutes */
  Boolean daylightSavings;  	/**< Is this daylight savings  */
  UInt16 timeZone;  		/**< Number of minutes east of GMT  */


} PhnNetworkTime;


/**
 *
 **/
typedef enum {

    phnMIPSuccess,				/**<		*/
	phnMIPNoSimBindings,			/**<		*/
    phnMIPFaReasonUnspecified=64,		/**<		*/
    phnMIPFaAdminProhibited=65,			/**<		*/
    phnMIPFaInsufficientResources=66,		/**<		*/
    phnMIPFaMobileNodeAuth=67,			/**<		*/
    phnMIPFaHomeAgentAuth=68,			/**<		*/
    phnMIPFaReqLifetimeTooLong=69,		/**<		*/
    phnMIPFaPoorlyFormedReq=70,			/**<		*/
    phnMIPFaPoorlyFormedReply=71,		/**<		*/
    phnMIPFaReqedEncapUnavail=72,		/**<		*/
    phnMIPFaReservedNUnavail=73,		/**<		*/
    phnMIPFaCantRevTun=74,			/**<		*/
    phnMIPFaMustRevTun=75,			/**<		*/
    phnMIPFaBadTtl=76,				/**<		*/
    phnMIPFaInvalidCareOfAddr=77,		/**<		*/
    phnMIPFaRegistration_Timeout=78,		/**<		*/
	PhnMIPFADelvStyleUnsupported=79,	/**<		*/
    phnMIPFaHome_Network_Unreachable=80,	/**<		*/
    phnMIPFaHa_Host_Unreachable=81,		/**<		*/
    phnMIPFaHa_Port_Unreachable=82,		/**<		*/
    phnMIPFaHa_Unreachable=88,			/**<		*/
	PhnMIPFANonzeroHomeAddrReq=96,		/**<		*/
	PhnMIPFAMissingNai=97,			/**<		*/
    phnMIPFaForeign_Agent=98,			/**<		*/
	PhnMIPFAMissingHomeAddr=99,		/**<		*/
    PhnMIPFAError1=100,     			/**<		*/
    PhnMIPFAError2 =101,			/**<		*/
    phnMIPFaUnknown_Challenge=104,		/**<		*/
    phnMIPFaMissing_Challenge=105,		/**<		*/
    phnMIPFaStale_Challenge=106,		/**<		*/
    phnMIPHaReasonUnspecified=128,		/**<		*/
    phnMIPHaAdminProhibited=129,		/**<		*/
    phnMIPHaInsufficientResources=130,		/**<		*/
    phnMIPHaMobileNodeAuth=131,			/**<		*/
    phnMIPHaForeignAgentAuth=132,		/**<		*/
    phnMIPHaRegIdMismatch=133,			/**<		*/
    phnMIPHaPoorlyFormedReq=134,     		/**<		*/
    phnMIPHaTooManySimMobBindings=135,  	/**<		*/
    phnMIPHaUnknownHaAddr=136,     		/**<		*/
    phnMIPHaCantRevTun=137,     		/**<		*/
    phnMIPHaMustRevTun=138,     		/**<		*/
    phnMIPHaReqEncapNotAvail=139,		/**<		*/
	phnMIPHAError1=140,			/**<		*/
	phnMIPHAError2=141			/**<		*/

} PhnMIPFailType;


/**
 *
 **/
typedef enum
{

  phnPDPWriteOK,			/**<		*/
  phnPDPWriteInvalidIndex,		/**<		*/
  phnPDPWriteIndexInUse,		/**<		*/
  phnPDPWriteNotAllowedAtIndex,		/**<		*/
  phnPDPWriteInvalidNAILen,		/**<		*/
  phnPDPWriteInvalidNAIName,		/**<		*/
  phnPDPWriteDuplicateNAI,		/**<		*/
  phnPDPWriteInvalidAAALen,		/**<		*/
  phnPDPInvalidHALen,			/**<		*/
  phnPDPWriteInvalidSvcStrLen,		/**<		*/
  phnPDPWriteInvalidDialStrLen,		/**<		*/
  phnPDPWriteIndexNotPrevSet,		/**<		*/
  phnPDPWriteInternalModemErr		/**<		*/

}PhnPDPWriteAck;

#define isValidPhnPDPWriteAck(w) ((w >= phnPDPWriteOK) && (w <= phnPDPWriteInternalModemErr))	/**<		*/

/**
 *
 **/
typedef enum
{
  phnPDPReadOK,				/**<		*/
  phnPDPReadInvalidIndex,		/**<		*/
  phnPDPReadNotAllowedAtIndex,		/**<		*/
  phnPDPReadIndexNotSet,		/**<		*/
  phnPDPReadInternalModemErr		/**<		*/
}PhnPDPReadAck;

#define isValidPDPReadAck(r) ((r >= phnPDPReadOK) && (r <= phnPDPReadInternalModemErr))		/**<		*/

/**
 *
 **/
typedef enum
{
  phnPDPDefaultIndex,		/**<		*/
  phnPDPTempIndex,		/**<		*/
  phnPDPGenericIndex		/**<		*/

} _PhnPDPIndexType;

typedef UInt8 PhnPDPIndexType;		/**<		*/

#define isValidPhnPDPIndexType(t) ((t >= phnPDPDefaultIndex) && (t <= phnPDPGenericIndex))	/**<		*/

/**
 *
 **/
typedef enum
{
  phnPDPCopyOK,				/**<		*/
  phnPDPCopyInvalidFromIndex,		/**<		*/
  phnPDPCopyFromIndexNotActive,		/**<		*/
  phnPDPCopyNotAllowedAtFromIndex,	/**<		*/
  phnPDPCopyInvalidToIndex,		/**<		*/
  phnPDPCopyToIndexInUse,		/**<		*/
  phnPDPCopyNotAllowedAtToIndex,	/**<		*/
  phnPDPCopyInvalidNAILen,		/**<		*/
  phnPDPCopyInvalidNAIName,		/**<		*/
  phnPDPCopyDuplicateNAI,		/**<		*/
  phnPDPCopyInvalidAAALen,		/**<		*/
  phnPDPCopyInvalidSvcStrLen,		/**<		*/
  phnPDPCopyInternalModemErr		/**<		*/
} _PhnPDPCopyAck;			/**<		*/

typedef UInt8 PhnPDPCopyAck;		/**<		*/

#define isValidPhnPDPCopyAck(c) ((c >= phnPDPCopyOK) && (c <= phnPDPCopyInternalModemErr))	/**<		*/


/**
 *
 **/
typedef enum
{
  phnPDPDeleteOK,			/**<		*/
  phnPDPDeleteInvalidIndex,		/**<		*/
  phnPDPDeleteIndexActive,		/**<		*/
  phnPDPDeleteIndexNotUsed,		/**<		*/
  phnPDPDeleteNotAllowedAtIndex,	/**<		*/
  phnPDPDeleteInternalModemErr		/**<		*/
} _PhnPDPDeleteAck;

typedef UInt8 PhnPDPDeleteAck;		/**<		*/

#define isValidPhnPDPDeleteAck(d) ((d >= phnPDPDeleteOK) && (d <= phnPDPDeleteInternalModemErr))	/**<		*/

/**
 *
 **/
typedef enum
{
  phnPDPReplaceOK,			/**<		*/
  phnPDPReplaceInvalidIndex,		/**<		*/
  phnPDPReplaceInvalidNotUsed,		/**<		*/
  phnPDPReplaceNotAllowedAtIndex,  	/**<		*/
  phnPDPReplaceInvalidLen,  		/**<		*/
  phnPDPReplaceInternalModemErr		/**<		*/

} _PhnPDPReplaceAck;

typedef UInt8 PhnPDPReplaceAck;		/**<		*/

#define isValidPhnPDPReplaceAck(r) ((r >= phnPDPReplaceOK) && (r <= phnPDPReplaceInternalModemErr))	/**<		*/



/**
 *  Indicates whether the profile is a mobile IP or simple IP profile.
 **/

/**
 * We changed the ordering of enum to match the TIL type and to keep it consistent with Crowdy NV
 * table for the PDP. Previously, this type was only used by NetPref we impact was minimal. The new ordering
 * also allows us to overload the NetMaster "isMobileIPValue" record value with this value. This way
 * any of 'if (isMobileIPValue)' will return true for both MIP and MIP with fallback. Later we can check the
 * specific in Netmaster as needed
 **/
typedef enum {
  phnPDPConnectionTypeSIP = 0,   	 	/**< PDP contains Simple IP settings  */
  phnPDPConnectionTypeMIPWithSIPFallback, /**< PDP contains MobileIP settings. Also allows Simple IP fallback - for Crowdy  */ 
  phnPDPConnectionTypeMIP,   		/**< PDP contains Mobile IP settings  */
  phnPDPConnectionTypeInvalid

} _PhnPDPConnectionType;

typedef UInt8 PhnPDPConnectionType;		/**<		*/

/**
 *  Bit mask that identifies the services a given profile should be used for
 **/
typedef enum
{
  phnPDPServiceUnknown = 0x00000000,		/**<		*/
  phnPDPServiceInternet = 0x00000001,  		/**< Profile to be used for generic internet  */
  phnPDPServiceWAP = 0x00000002,    		/**< Profile to be used for WAP  */
  phnPDPServicePrivate = 0x00000004,  		/**< Profile to be used for application specfic gateway  */
  phnPDPServiceMMS = 0x00000008,   		/**< Profile to be used for MMS  */
  phnPDPServiceBrowser = 0x00000010,  		/**< Profile to be used for Browser   */
  phnPDPServiceMail = 0x00000020,  		/**< Profile to be used for email  */
  phnPDPServiceIM = 0x00000040,   		/**< Profile to be used for IM  */
  phnPDPServiceDownloads = 0x00000080,   	/**< Profile to be used for downloads  */
  phnPDPServiceCorporateGateway = 0x00000100,   /**< Profile to be used for corporate network  */
  phnPDPServicePictureMail = 0x00000200  	/**< Profile for Picture Mail  */
} _PhnPDPService;

typedef UInt32 PhnPDPService;		/**<		*/

/**
 * None of the PDP strings (like service string, dial string, passwords etc.)
 * require null terminator and no need to count null terminator for the length.
 *
 * PhnPDPList structure lists status of all the packet data profile slots
 **/
typedef struct
{
    Boolean                         idx0_set ;
      /**< Boolean indicating if index contains a valid PDP in the modem.  */

    Boolean                         idx1_set ;
      /**< Boolean indicating if index contains a valid PDP in the modem.  */

    Boolean                         idx2_set ;
      /**< Boolean indicating if index contains a valid PDP in the modem.  */

    Boolean                         idx3_set ;
      /**< Boolean indicating if index contains a valid PDP in the modem.  */

    Boolean                         idx4_set ;
      /**< Boolean indicating if index contains a valid PDP in the modem.  */

    Boolean                         idx5_set ;
      /**< Boolean indicating if index contains a valid PDP in the modem.  */

    Boolean                         idx6_set ;
      /**< Boolean indicating if index contains a valid PDP in the modem.  */

    Boolean                         idx7_set ;
      /**< Boolean indicating if index contains a valid PDP in the modem.  */

    Boolean                         idx_RSVD8_set ;
      /**< Reserved field.  */

    Boolean                         idx_RSVD9_set ;
      /**< Reserved field.  */

    Boolean                         qnc_set ;
      /**< Boolean indicating if a valid QNC profile is stored in the modem. */

    Boolean                         active_idx_set ;
      /**< Boolean indicating if there is currently an active index set in the
         modem. */

    UInt8                            active_idx ;
      /**< Indication of which index is currently the active index in the modem
         if ACTIVE_IDX_SET. */

    UInt8                            idx0_uid[phnPDPUidLen] ;
      /**< The Unique ID for the PDP stored in index if index is set. */

    UInt8                            idx1_uid[phnPDPUidLen] ;
      /**< The Unique ID for the PDP stored in index if index is set. */

    UInt8                            idx2_uid[phnPDPUidLen] ;
      /**< The Unique ID for the PDP stored in index if index is set. */

    UInt8                            idx3_uid[phnPDPUidLen] ;
      /**< The Unique ID for the PDP stored in index if index is set. */

    UInt8                            idx4_uid[phnPDPUidLen] ;
      /**< The Unique ID for the PDP stored in index if index is set. */

    UInt8                            idx5_uid[phnPDPUidLen] ;
      /**< The Unique ID for the PDP stored in index if index is set. */

    UInt8                            idx6_uid[phnPDPUidLen] ;
      /**< The Unique ID for the PDP stored in index if index is set. */

    UInt8                            idx7_uid[phnPDPUidLen] ;
      /**< The Unique ID for the PDP stored in index if index is set. */

    UInt8                          idx_rsvd8_uid[phnPDPUidLen] ;
      /**< Reserved field. */

    UInt8                          idx_rsvd9_uid[phnPDPUidLen] ;
      /**< Reserved field. */

    UInt8                            qnc_uid[phnPDPUidLen] ;
      /**< The Unique ID for the QNC profile stored in the modem. */

	UInt8                            num_stand_pdp_idxs;
      /**< The number of standard PDPs in the modem. */

	UInt8                            first_stand_pdp_idx;
      /**< IF NUM_STAND_PDP_IDXS is greater than zero, indicates the first
       standard PDP index (assumes set of all standard PDP indexes are
       sequential). */
   	UInt8                            num_def_pdp_idxs;
      /**< The number of default PDPs in the modem that cannot be modified by
         an IOTA session. */

    UInt8                            first_def_pdp_idx;
      /**< IF NUM_DEF_PDP_IDXS is greater than zero, indicates the first default
       PDP index (assumes set of all default PDP indexes are sequential). */

    UInt8                            num_tmp_pdp_idxs ;
      /**< The Unique ID for the QNC profile stored in the modem. */

    UInt8                            first_tmp_pdp_idx;
      /**< IF NUM_TMP_PDP_IDXS is greater than zero, indicates the first default
       PDP index (assumes set of all default PDP indexes are sequential). */

} PhnPDPList;

/**
 *
 **/
typedef struct{
	UInt8                            idx  ;
      /**< The index of the PDP to write to in the modem.  The set of valid
         indexes that can be written to is dependent on the modem
         platform being used. */

	Boolean                         use_existing_nai  ;
      /**< Boolean indicating if the NAI currently stored in this PDP should
         be saved.  If set to FALSE, the following two fields are used for
         new NAI value. */

    UInt8                            nai_length  ;
      /**< The number of characters in the NAI. */

    UInt8                            nai[phnMaxNAILen]  ;
      /**< The user NAI character string in ASCII representation */

	Boolean                         use_existing_mn_aaa_ss  ;
      /**< Boolean indicating if the MN-AAA-SS currently stored in this PDP
         should be saved.  If set to FALSE, the following three fields are
         used for new MN-AAA-SS value. */


    Boolean                         store_mn_aaa_ss  ;
      /**< Boolean indicating if the MN-AAA shared secret parameter is to be
         stored with this PDP profile.  Setting this field to FALSE requires
         the host to send a temporary MN-AAA shared secret value to the modem
         before using this PDP to connect to a network.*/

    UInt8                            mn_aaa_ss_length  ;
      /**< Length of the MN-AAA shared secret parameter to store in the modem
         for this PDP if the STORE_MN_AAA_SS field is set to TRUE.  */

    UInt8  mn_aaa_ss[phnMaxMNAAASharedSecretLen ]  ;
      /**< Text/binary string of the MN-AAA shared secret parameter to store in the
          modem for this PDP if the STORE_MN_AAA_SS field is set to TRUE. */

    Boolean                         use_existing_mn_ha_ss  ;
      /**< Boolean indicating if the MN-HA-SS currently stored in this PDP should
       be saved.  If set to FALSE, the following three fields are used for
       new MN-HA-SS value. */

	UInt8                         reserved1 ;
      /**< for future use */

    UInt8                            mn_ha_ss_length  ;
      /**< Length of the MN-HA shared secret parameter to store in the modem for
         this PDP */

    UInt8  mn_ha_ss[phnMaxMNHASharedSecretLen]  ;
      /**< Text/binary string of the MN-HA shared secret parameter to store in the modem
         for this PDP */

    Boolean                         use_existing_mn_aaa_spi  ;
      /**< Boolean indicating if the MN-AAA SPI currently stored in this PDP
         should be saved.  If set to FALSE, the following three fields are
         used for new MN-AAA SPI value. */

	UInt8                         reserved2  ;
      /**< for future use */

    Boolean                         set_mn_aaa_spi  ;
      /**< Boolean indicating if the SPI for MN-AAA authentication is to be set
         for this PDP*/

    DWord                           mn_aaa_spi  ;
      /**< Integer value of the SPI for MN-AAA authentication parameter to store
         in the modem for this PDP if the SET_MN_AAA_SPI field is set to TRUE. */

	Boolean                         use_existing_mn_ha_spi  ;
      /**< Boolean indicating if the MN-HA SPI currently stored in this PDP
         should be saved.  If set to FALSE, the following three fields are
         used for new MN-HA SPI value. */

    UInt8                         reserved3  ;
      /**< for future use */

    Boolean                         set_mn_ha_spi  ;
      /**< Boolean indicating if the SPI for MN-HA authentication is to be set
         for this PDP */

    DWord                           mn_ha_spi  ;
      /**< Integer value of the SPI for MN-HA authentication parameter to store
         in the modem for this PDP if the SET_MN_HA_SPI field is set to TRUE. */

    Boolean                         use_existing_rev_tunnel_pref  ;
      /**< Boolean indicating if the Reverse Tunneling Preference currently
         stored in this PDP should be saved.  If set to FALSE, the following
         two fields are used for new Reverse Tunneling Preference values. */

	UInt8                         reserved4  ;
    /**< for future use */

    Boolean                         rev_tunnel_pref  ;
      /**< Boolean indicating if reverse tunnel is to be used for this PDP. */

    Boolean                         use_existing_home_addr  ;
      /**< Boolean indicating if the Home Address currently stored in this PDP
         should be saved.  If set to FALSE, the following two fields are used
         for new Home Address value. */

	UInt8                        reserved5;
      /**< for future use */

    DWord                           home_addr  ;
      /**< The home IP address of the modem.  Setting this address to 0.0.0.0
         will allow the IP address to be dynamically assigned by the PDSN for
         Mobile IP and Simple IP. */

    Boolean                         use_existing_ha_addr  ;
      /**< Boolean indicating if the Home Agent IP Address currently stored in
         this PDP should be saved.  If set to FALSE, the following three
         fields are used for new Home Agent IP Address values. */

	UInt8                         reserved6;
      /**< for future use */

    DWord                           prim_ha_addr  ;
      /**< The Primary Home Agent IP address for this PDP */

    DWord                           sec_ha_addr  ;
      /**< The Secondary Home Agent IP address for this PDP */

    Boolean                         use_existing_svc_str  ;
      /**< Boolean indicating if the Service String currently stored in this
         PDP should be saved.  If set to FALSE, the following three fields are
         used for new Service String value. */

	Boolean                         use_default_svc_str  ;
      /**< Boolean indicating if the service string name that the modem would
         create for this profile (the carrier name string + user name portion
         of NAI) should be stored as the service string for this PDP. */

    UInt8                            svc_str_length  ;
      /**< The length of the service string to be stored for this PDP if
         USE_DEFAULT_SVC_STR is set to FALSE. */

    UInt8  svc_str[phnMaxSvcStrLen]  ;
      /**< Text string of the service string to be stored for this PDP if
         USE_DEFAULT_SVC_STR is set to FALSE. */

	Boolean                         use_existing_dial_str  ;
      /**< Boolean indicating if the Dial String currently stored in this PDP
         should be saved.  If set to FALSE, the following three fields are
         used for new Dial String values. */

    Boolean                         use_default_dial_str  ;
      /**< Boolean indicating if the dial string to associated and stored
         with this PDP should be set to the default Packet Data dial string
         stored in the modem. */

    UInt8                            dial_str_length  ;
      /**< The length of the dial string to be associated and stored with this
         PDP if USE_DEFAULT_DIAL_STR is FALSE.   */

    UInt8  dial_str[phnMaxDialStrLen]  ;
      /**< The dial string to be associated and stored with this PDP if
         USE_DEFAULT_DIAL_STR is FALSE. */

	Boolean                         use_existing_dns_server  ;
      /**< Boolean indicating if the DNS Server currently stored in this PDP
         should be saved.  If set to FALSE, the following three fields are
         used for new DNS Server values. */

    Boolean                         dns_server_assigned  ;
      /**< Boolean indicating if specific DNS server IP addresses are to be
         associated and stored with this PDP. */

    DWord                           prim_dns_addr  ;
      /**< The Primary DNS server IP address for this PDP if the
         DNS_SERVER_ASSIGNED field is set to TRUE. */

    DWord                           sec_dns_addr  ;
      /**< The Secondary DNS server IP address for this PDP if the
         DNS_SERVER_ASSIGNED field is set to TRUE. */

	PhnPDPConnectionType            connection_type;
	  /**< Connection type of the PDP  */

	PhnPDPService                   pdp_services;
	 /**< Services for which this PDP should be used  */
}PhnPDPWrite;

/**
 *  Packet data profile payload
 **/
 typedef struct {
	UInt8                            uid[phnPDPUidLen]   ;
      /**< The binary UID for this PDP.  */

    Boolean                         restrict_nai   ;
	  /**< Boolean indicating if the NAI field is restricted from being read from
         this PDP index.  None of the following NAI fields will contain valid
         data if set to TRUE.  */

    UInt8                            nai_length   ;
	  /**< The number of characters in the NAI.   */

    UInt8                            nai[phnMaxNAILen]   ;
	  /**< The user NAI character string in ASCII representation.   */

    Boolean                         restrict_mn_aaa_ss   ;
      /**< Boolean indicating if the MN-AAA shared secret field is restricted
         from being read from this PDP index.  None of the following MN-AAA
         shared secret fields will contain valid data if set to TRUE. */

    Boolean                         stored_mn_aaa_ss   ;
      /**< Boolean indicating if the MN-AAA shared secret parameter is stored
         with this PDP profile. */

    UInt8                            mn_aaa_ss_length   ;
      /**< Length of the MN-AAA shared secret parameter for this PDP if the
         STORE_MN_AAA_SS field is set to FALSE. Maximum length shall be 16. */

    UInt8  mn_aaa_ss[phnMaxMNAAASharedSecretLen]   ;
      /**< Text/binary string of the MN-AAA shared secret parameter stored in the modem
         for this PDP. */

    Boolean                         restrict_mn_ha_ss   ;
      /**< Boolean indicating if the MN-HA shared secret field is restricted from
         being read from this PDP index.  None of the following MN-HA shared
         secret fields will contain valid data if set to TRUE. */

    UInt8                            mn_ha_ss_length   ;
      /**< Length of the MN-HA shared secret parameter stored in the modem for
         this PDP.  Maximum length shall be 16. */

    UInt8  mn_ha_ss[phnMaxMNHASharedSecretLen]   ;
      /**< Text/binary string of the MN-HA shared secret parameter stored in the modem
         for this PDP. */

    Boolean                         restrict_mn_aaa_spi   ;
      /**< Boolean indicating if the SPI for MN-AAA authentication field is
         restricted from being read from this PDP index.  None of the following
         SPI for MN-AAA authentication fields will contain valid data if set to
         TRUE. */

    Boolean                         set_mn_aaa_spi   ;
      /**< Boolean indicating if the SPI for MN-AAA authentication is set for
         this PDP */

    DWord                           mn_aaa_spi   ;
      /**< Integer value of the SPI for MN-AAA authentication for this PDP. */

    Boolean                         restrict_mn_ha_spi   ;
      /**< Boolean indicating if the SPI for MN-HA authentication field is
         restricted from being read from this PDP index.  None of the following
         SPI for MN-AAA authentication fields will contain valid data if set to
         TRUE. */

    Boolean                         set_mn_ha_spi   ;
      /**< Boolean indicating if the SPI for MN-HA authentication is set for this
         PDP. */

    DWord                           mn_ha_spi   ;
      /**< Integer value of the SPI for MN-HA authentication parameter for this
         PDP. */

    Boolean                         restrict_rev_tunnel_pref   ;
      /**< Boolean indicating if the reverse tunnel setting is restricted from
         being read from this PDP index.  The REV_TUNNEL_PREF field will not
         contain valid data if set to TRUE. */

    Boolean                         rev_tunnel_pref   ;
      /**< Boolean indicating if reverse tunnel is used for this PDP. */

    Boolean                         restrict_home_addr   ;
      /**< Boolean indicating if the modem's Home Address setting is
         restricted from being read from this PDP index.  The HOME_ADDR field
         will not contain valid data if set to TRUE. */

    DWord                           home_addr   ;
      /**< The home IP address of the modem for this PDP.  An address of 0.0.0.0
         indicates the IP address will be dynamically assigned by the PDSN for
         Mobile IP and Simple IP. */

    Boolean                         restrict_ha_addr   ;
      /**< Boolean indicating if the Home Agent IP address fields are restricted
         from being read from this PDP index.  None of the following Home Agent
         IP address fields will contain valid data if set to TRUE. */

    DWord                           prim_ha_addr   ;
      /**< The Primary Home Agent IP address for this PDP  */

    DWord                           sec_ha_addr   ;
      /**< The Secondary Home Agent IP address for this PDP  */

    UInt8                            svc_str_length   ;
      /**< The length of the service string for this PDP. */

    UInt8  svc_str[phnMaxSvcStrLen]   ;
      /**< Text string of the service string for this PDP. */

    UInt8                            dial_str_length   ;
      /**< The length of the dial string for this PDP. */

    UInt8  dial_str[phnMaxDialStrLen]   ;
      /**< The dial string for this PDP. */

    Boolean                         dns_server_assigned   ;
      /**< Boolean indicating if specific DNS server IP addresses are assigned
         for this PDP.  */

    DWord                           prim_dns_addr   ;
      /**< The Primary DNS server IP address for this PDP if the
         DNS_SERVER_ASSIGNED field is set to TRUE. */

    DWord                           sec_dns_addr   ;
      /**< The Secondary DNS server IP address for this PDP if the
         DNS_SERVER_ASSIGNED field is set to TRUE. */

	PhnPDPConnectionType            connection_type;
	  /**< Connection type of the PDP  */

	PhnPDPService                   pdp_services;
	 /**< Services for which this PDP should be used  */

} PhnPDPPayload;

/**
 *
 **/
typedef struct
  {
    DWord                       lat;
     /**< Two's complement value of the latitude of the modem, in units of
         360/2^25 degrees. */

    DWord                       lon;
     /**< Two's complement value of the longitude of the modem, in units of
         360/2^26 degrees. */

    DWord                       time_stamp;
     /**< CDMA system time, in seconds, at the time the solution was valid. */

    UInt8                        loc_uncertainty_ang;
     /**< Angle of axis with respect to True North for position uncertainty,
         in units of 5.625 degrees, in the range from 0 to 84.375 degrees,
         where 0 degrees is True North and the angle increases towards the
         East. */

    PhnPDPAUncerttype         loc_uncertainty_a;
     /**< Standard deviation of axis along angle specified for position
         uncertainty. */

    PhnPDPAUncerttype         loc_uncertainty_p;
     /**< Standard deviation of axis perpendicular to angle specified for
         position uncertainty. */

    UInt8                        phn_pd_svc_mask;
     /**< Bit mask indicating which of the following optional fields contain
         valid data. */

    UInt16                        altitude;
     /**< Height of the modem, in units of 1 meter, in the range from -500 m
         to 15883 m, where the binary value of the field conveys the height
         plus 500 m.  Valid if OPT_FIELD_MASK field has
         PDSM_PD_ALTITUDE_VALID bit set. */

    UInt16                        heading;
     /**< Direction of the modem, in units of 360/2^10 degrees, in the range
         from 0 to 360x(1-2^10) degrees (where 0 degrees is True North and
         the angle increases towards the East.  Valid if OPT_FIELD_MASK
         field has PDSM_PD_VELOCITY_VALID bit set. */

    UInt16                        velocity_hor;
     /**< Horizontal component of the velocity of the modem, in units of
         0.25 m/s, in the range from 0 to 127.75 m/s.  Valid if
         OPT_FIELD_MASK field has PDSM_PD_VELOCITY_VALID bit set. */

    Boolean                     fix_t;
     /**< Boolean indicating fixed type.  0 means 2D fix and 1 means 3D
         fix. */

    UInt16                        velocity_ver;
     /**< Two's complement value of the vertical component of the velocity of
         the modem, in units of 0.5 m/s, in the range from -64m/s to
         +63.5m/s.  Valid if OPT_FIELD_MASK field has
         PDSM_PD_VELOCITY_VALID bit set and FIX_T is set to TRUE. */

    PhnPDPAUncerttype         loc_uncertainty_v;
     /**< Standard deviation of vertical error for position uncertainty.
         Valid if OPT_FIELD_MASK field has PDSM_PD_VELOCITY_VALID bit
         set. */
  }
PhnPDDataType, *PhnPDDataPtr;

/**
 *
 **/
typedef enum
{
  phnSysPrefHomeOnly,		/**<		*/
  phnSysPrefAOnly,		/**<		*/
  phnSysPrefBOnly,		/**<		*/
  phnSysPrefStandard		/**<		*/

} _PhnSysPrefSetting;

typedef UInt8 PhnSysPrefSetting;		/**<		*/

/**
 *
 **/
typedef enum
{
  phnGenericCDMASMSEncoding,		/**<		*/
  phnGenericCDMAVoiceMailNumber,   	/**<		*/
  phnGenericCDMAVoiceMailOption,
  phnGenericCDMAAllowPDPSlotModification
} _PhnGenericCDMAPreference;

typedef UInt32 PhnGenericCDMAPreference;  /**<		*/

/**
 *
 **/
typedef enum
{
  phnSMSEncodingGSM = 0,		/**<		*/
  phnSMSEncodingASCII,	        /**<		*/
  phnSMSEncodingISO8859_1	    /**<		*/
} _PhnGenericCDMASMSEncodingType;

typedef UInt32 PhnGenericCDMASMSEncodingType;  /**<		*/

/**
 *
 **/
typedef enum
{
  phnCDMAFirmwareSprint = 0,		/**<		*/
  phnCDMAFirmwareVerizon,	        /**<		*/
  phnCDMAFirmwareBellMobility,	    /**<		*/
  phnCDMAFirmwareEarthlink,         /**<		*/
  phnCDMAFirmwareCrowdy,            /**<CDMA rest of world*/
  phnCDMAFirmwareAlltel             /**<		*/
} _PhnCDMAFirmwareType;

typedef UInt32 PhnCDMAFirmwareType; /**<		*/


typedef enum
{
  phnCDMAVoicemailMDN = 0,
  phnCDMAVoicemailMDN1,
  phnCDMAVoicemailDigits
} _PhnCDMAVoicemailType;

typedef UInt32 PhnCDMAVoicemailType;

#endif // _HS_PHONE_CDMA_TYPES_H__
