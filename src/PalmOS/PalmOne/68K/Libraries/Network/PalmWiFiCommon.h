/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/**
 * @ingroup Network
 */

/**
 * @file 	PalmWiFiCommon.h
 * @version 1.0
 *
 * @brief Constants and structures common to the wifi network panel and driver.
 *
 */

#ifndef PALM_WIFI_COMMON_H_
#define PALM_WIFI_COMMON_H_

#include <NetMgr.h>

/** Define the appropriate minimum OS version we support */
#define WIFI_MIN_OS_VERSION  sysMakeROMVersion(3,0,0,sysROMStageDevelopment,0)
/** Define the appropriate minimum OS version we support */
#define PALM_OS_20_VERSION   sysMakeROMVersion(2,0,0,sysROMStageDevelopment,0)

/** WiFi creator ID. */
#define WIFI_CREATOR_ID    'WiFi'
/** WiFi configuration name */
#define WIFI_CONFIG_NAME   "WiFi Config"

/** WiFi database type */
#define WIFI_DB_TYPE       'DATA'
/** WiFi database name */
#define WIFI_DB_NAME       "WiFiDB"

/** WiFi driver prefix */
#define WIFI_DRIVER_PREFIX "wi"
/** WiFi driver name */
#define WIFI_DRIVER_NAME   "WiFi Prism3"

/// @name Event Notifications
/// Event notifications from the WiFi -- can be used for SysNotifyRegister
/*@{*/
#define	WIFI_ASSOCIATED_EVENT				'WiAs'
#define WIFI_DISASSOCIATED_EVENT			'WiDi'
#define WIFI_SCAN_COMPLETE_EVENT			'WiSc'
#define WIFI_IF_UP_EVENT					'WiUp'
#define WIFI_IF_DOWN_EVENT					'WiDn'
/*@}*/


/** Supported Query requests for WiFi driver. */
typedef enum
{
	netQuery80211SignalStrength = 0x100,
	netQuery80211SignalQuality,
	netQuery80211AuthenticationMode,
	netQuery80211BssScan,
	netQuery80211BssScanList,
	netQuery80211Channel,
	netQuery80211PowerState,
	netQuery80211RadioState,
	netQuery80211SSID,
	netQuery80211Key,
	netQuery80211KeyIndex,
	netQuery80211ResetKeys,
	netQuery80211OperatingMode,
	netQuery80211WepStatus,
	netQuery80211FirmwareVersion,
	netQuery80211SerialNumber,
	netQuery80211FWName,
	netQuery80211DriverDate,
	netQuery80211WEPRequired,
	netQuery80211ChannelList,
	netQuery80211BSSID,
	netQueryCancel,
	netQuery80211MediaState,
	netQuery80211SecurityMode, // use security mode #defines
	netQuery80211StartConnection,
	netQuery80211Last
} NetQueryId;


/** IP address allocation strategies */
enum IPAllocType
{
	IPAllocType_DHCP   = 0,
	IPAllocType_APIPA  = 1,
	IPAllocType_STATIC = 2
};

enum SecurityModeType
{
	SecurityModeOpenSystem = 0,
	SecurityModeWEP,
	SecurityModeWPAPSK,
	SecurityModeLast
};

/// @name Encryption Algorithm
/*@{*/
#define	AUTHENTICATION_ALGORITHM_OPEN_SYSTEM	1
#define	AUTHENTICATION_ALGORITHM_SHARED_KEY		2 //40 bit
#define	AUTHENTICATION_ALGORITHM_SHARED_KEY_128	3
/*@}*/

/** Operating mode, referred to as port type in the configuration. */
#define	ESS		1



#define IBSS	4

/** @name ESS TX power mode. */
/*@{*/
#define	AUTO_TX_POWER			0
#define	ENHANCED_TX_POWER		1
/*@}*/

/** @name AdHoc power modes. */
/*@{*/
#define	FULL_TX_POWER			0
#define	_30_MW_TX_POWER			1
#define	_15_MW_TX_POWER			2
#define	_5_MW_TX_POWER			3
#define	_1_MW_TX_POWER			4
/*@}*/

/** @name AP density constants.
 *  @brief Referred as netIFSetting80211Environment in configuration.
 */
/*@{*/
#define AP_DENSITY_LOW	0
#define AP_DENSITY_MED	1
#define AP_DENSITY_HIGH	2
/*@}*/

/** @name Adapter Preamble */
/*@{*/
#define	LONG_PREAMBLE		0
#define	SHORT_PREAMBLE		1
#define AUTO_PREAMBLE		2
/*@}*/

/** Special code required to be passed to reset wep keys */
#define net80211ResetWepPasscode	0x41149339

/** Definitions of media states returned from netQuery80211MediaState */
#define netMediaDisconnected		0
/** Definitions of media states returned from netQuery80211MediaState */
#define netMediaConnected			1

enum WiFiCustomIFSettings
{
	netIFSetting80211PowerMode = netIFSettingCustom,
	netIFSetting80211Diversity,
	netIFSetting80211Channel,
	netIFSetting80211EncryptionKey1,
	netIFSetting80211EncryptionKey2,
	netIFSetting80211EncryptionKey3,
	netIFSetting80211EncryptionKey4,
	netIFSetting80211EncryptionKeyID,
	netIFSetting80211MandatoryBSSId,
	netIFSetting80211MUEncryptionAlgorithm,
	netIFSetting80211PortType,  // ESS, PsIBBS, IBSS
	netIFSetting80211EssTxPower,
	netIFSetting80211AdhocTxPower,
	netIFSetting80211StationName,
	netIFSetting80211Environment,
	netIFSetting80211LongPreamble,
	netIFSetting80211Gateway,
	netIFSettingPrimaryDNS,
	netIFSettingSecondaryDNS,
	netIFSettingDHCPRequestDNS,
	netIFSettingIPAllocType, // 0,1,2=DHCP,APIPA,STATIC
	netIFSetting80211SmartConnect,
	netIFSettingUIProgHoldOff,
	netIFSettingWiFiHideProgress,
	netIFSettingAutoPowerDownOnClose,
	netIFSettingInterfaceQSize,
	netIFSettingDriverQSize,
	netIFSetting8021x,
	netIFSetting80211MediaDisconnectLingerTime,
	netIFSetting80211MobileIP,

#ifdef WIFI_DOT1X_SUPPORT
	netIFSettingDot1XWPPICreator,
	netIFSettingDot1XAuthenticationMethodID,
	netIFSettingDot1XAuthenticationCookie,
#endif // WIFI_DOT1X_SUPPORT

	netIFSetting80211Last
};

#define netMacAddrLength		6   /**< max length MAC address */
#define netSSIDMaxLength		32  /**< max length SSID */
#define netNum80211Rates		8   /**< no definition */

/**	Wep status codes */
enum NetWepStatus
{
	netWEPEnabled,
	netWEPDisabled,
	netWEPKeyAbsent,
	netWEPNotSupported
};

/** @name Network Device Driver Interface Error Codes */
/*@{*/
#define errNetQueryBufTooSmall	11
#define errNetOutOfMemory		12
#define errNetLibMissing		13
#define errNetInterfaceNotFound	14
/*@}*/

// The following are added to support a structure independent method
// for presenting the BSS Scan data to an application.  Macros
// are also provide to access data within a buffer.
// NOTE:  These offsets do not account for the first 4 bytes which
// must always precede the data.  This count field indicates the
// number of BSS structures will follow

/** Offset = 0.  Present/validity flags for each of the entities defined below */
/*@{*/
#define BSSINFO_VFLAG					0
#define BSSINFO_VFLAG_SIZE				4
/*@}*/

/** Offset = 4.  MAC/HW Address for device. */
/*@{*/
#define BSSINFO_MAC_ADDR_OFFSET			(BSSINFO_VFLAG + BSSINFO_VFLAG_SIZE)
#define BSSINFO_MAC_ADDR_SIZE			8				// Add padding to allow easy access for structure definiton.
#define BSSINFO_MAC_ADDR_VFLAG			0x00000001L
/*@}*/

/** Offset = 12.  Length of the following SSID */
/*@{*/
#define BSSINFO_SSIDLENGTH_OFFSET		(BSSINFO_MAC_ADDR_OFFSET + BSSINFO_MAC_ADDR_SIZE)
#define BSSINFO_SSIDLENGTH_SIZE			4
#define BSSINFO_SSIDLENGTH_VFLAG		0x00000002L
/*@}*/

/** Offset = 16 (0x10).  SSID */
/*@{*/
#define BSSINFO_SSID_OFFSET				(BSSINFO_SSIDLENGTH_OFFSET + BSSINFO_SSIDLENGTH_SIZE)
#define BSSINFO_SSID_SIZE				netSSIDMaxLength
#define BSSINFO_SSID_VFLAG				0x00000004L
/*@}*/

/** Offset = 48 (0x30).  WEP enabled boolean */
/*@{*/
#define BSSINFO_WEP_OFFSET				(BSSINFO_SSID_OFFSET + BSSINFO_SSID_SIZE)
#define BSSINFO_WEP_SIZE				4
#define BSSINFO_WEP_VFLAG				0x00000008L
/*@}*/

/** Offset = 52 (0x34).   Received Signal Strength */
/*@{*/
#define BSSINFO_RSSI_OFFSET				(BSSINFO_WEP_OFFSET + BSSINFO_WEP_SIZE)
#define BSSINFO_RSSI_SIZE				4
#define BSSINFO_RSSI_VFLAG				0x00000010L
/*@}*/

/** Offset = 56 (0x38).  Network type in use (Freq. hopping, DSS) */
/*@{*/
#define BSSINFO_NETWORK_TYPE_OFFSET		(BSSINFO_RSSI_OFFSET + BSSINFO_RSSI_SIZE)
#define BSSINFO_NETWORK_TYPE_SIZE		4
#define BSSINFO_NETWORK_TYPE_VFLAG		0x00000020L
/*@}*/

/** Offset = 60 (0x3C).  Beacon period (in KuSec) */
/*@{*/
#define BSSINFO_BEACON_PERIOD_OFFSET	(BSSINFO_NETWORK_TYPE_OFFSET + BSSINFO_NETWORK_TYPE_SIZE)
#define BSSINFO_BEACON_PERIOD_SIZE		4
#define BSSINFO_BEACON_PERIOD_VFLAG		0x00000040L
/*@}*/

/** Offset = 64 (0x40).  ATim window (in KuSec) */
/*@{*/
#define BSSINFO_ATIM_WINDOW_OFFSET		(BSSINFO_BEACON_PERIOD_OFFSET + BSSINFO_BEACON_PERIOD_SIZE)
#define BSSINFO_ATIM_WINDOW_SIZE		4
#define BSSINFO_ATIM_WINDOW_VFLAG		0x00000080L
/*@}*/

/** Offset = 68 (0x44). DS frequency (in kHz) */
/*@{*/
#define BSSINFO_DSFREQ_OFFSET			(BSSINFO_ATIM_WINDOW_OFFSET + BSSINFO_ATIM_WINDOW_SIZE)
#define BSSINFO_DSFREQ_SIZE				4
#define BSSINFO_DSFREQ_VFLAG			0x00000100L
/*@}*/

/** Offset = 72 (0x48).  Frequency hopping pattern */
/*@{*/
#define BSSINFO_HOPPATTERN_OFFSET		(BSSINFO_DSFREQ_OFFSET + BSSINFO_DSFREQ_SIZE)
#define BSSINFO_HOPPATTERN_SIZE			4
#define BSSINFO_HOPPATTERN_VFLAG		0x00000200L
/*@}*/

/** Offset = 76 (0x4C).  Frequency hopping set */
/*@{*/
#define BSSINFO_HOPSET_OFFSET			(BSSINFO_HOPPATTERN_OFFSET + BSSINFO_HOPPATTERN_SIZE)
#define BSSINFO_HOPSET_SIZE				4
#define BSSINFO_HOPSET_VFLAG			0x00000400L
/*@}*/

/** Offset = 80 (0x50). Frequency hopping dwell time */
/*@{*/
#define BSSINFO_HOPDWELL_OFFSET			(BSSINFO_HOPSET_OFFSET + BSSINFO_HOPSET_SIZE)
#define BSSINFO_HOPDWELL_SIZE			4
#define BSSINFO_HOPDWELL_VFLAG			0x00000800L
/*@}*/

/** Offset = 84 (0x54).  802.11 network mode */
/*@{*/
#define BSSINFO_NETWORK_MODE_OFFSET		(BSSINFO_HOPDWELL_OFFSET + BSSINFO_HOPDWELL_SIZE)
#define BSSINFO_NETWORK_MODE_SIZE		4
#define BSSINFO_NETWORK_MODE_VFLAG		0x00001000L
/*@}*/

/** Offset = 88 (0x58).  Supported rates on interface. */
/*@{*/
#define BSSINFO_SUPPORTED_RATES_OFFSET	(BSSINFO_NETWORK_MODE_OFFSET + BSSINFO_NETWORK_MODE_SIZE)
#define BSSINFO_SUPPORTED_RATES_SIZE	8
#define BSSINFO_SUPPORTED_RATES_VFLAG	0x00002000L
/*@}*/

/** Offset = 96 (0x60).  Channel ID */
/*@{*/
#define BSSINFO_CHID_OFFSET				(BSSINFO_SUPPORTED_RATES_OFFSET + BSSINFO_SUPPORTED_RATES_SIZE)
#define BSSINFO_CHID_SIZE				4
#define BSSINFO_CHID_VFLAG				0x00004000L
/*@}*/

/** Offset = 96 (0x60).  Channel ID */
/*@{*/
#define BSSINFO_ANL_OFFSET				(BSSINFO_CHID_OFFSET + BSSINFO_CHID_SIZE)
#define BSSINFO_ANL_SIZE				4
#define BSSINFO_ANL_VFLAG				0x00008000L
/*@}*/

/** Total size of the BSS info supplied. */
#define BSSINFO_SIZE					(BSSINFO_ANL_OFFSET + BSSINFO_ANL_SIZE)

/// @name Macros for accessing individual fields
/*@{*/
#define BssInfoMacAddr( infoP, destP )	MemMove( destP, &((UInt8 *) infoP)[BSSINFO_MAC_ADDR_OFFSET], BSSINFO_MAC_ADDR_SIZE )
#define BssInfoSsidLength( infoP, destP ) MemMove( destP, &((UInt8 *) infoP)[BSSINFO_SSIDLENGTH_OFFSET], BSSINFO_SSIDLENGTH_SIZE )
#define BssInfoSsid( infoP, destP ) MemMove( destP, &((UInt8 *) infoP)[BSSINFO_SSID_OFFSET], BSSINFO_SSID_SIZE )
#define BssInfoWep( infoP, destP ) MemMove( destP, &((UInt8 *) infoP)[BSSINFO_WEP_OFFSET], BSSINFO_WEP_SIZE )
#define BssInfoRssi( infoP, destP ) MemMove( destP, &((UInt8 *) infoP)[BSSINFO_RSSI_OFFSET], BSSINFO_RSSI_SIZE )
#define BssInfoNetworkType( infoP, destP ) MemMove( destP, &((UInt8 *) infoP)[BSSINFO_NETWORK_TYPE_OFFSET], BSSINFO_NETWORK_TYPE_SIZE )
#define BssInfoBeaconPeriod( infoP, destP ) MemMove( destP, &((UInt8 *) infoP)[BSSINFO_BEACON_PERIOD_OFFSET], BSSINFO_BEACON_PERIOD_SIZE )
#define BssInfoAtimWindow( infoP, destP ) MemMove( destP, &((UInt8 *) infoP)[BSSINFO_ATIM_WINDOW_OFFSET], BSSINFO_ATIM_WINDOW_SIZE )
#define BssInfoDSFreq( infoP, destP ) MemMove( destP, &((UInt8 *) infoP)[BSSINFO_DSFREQ_OFFSET], BSSINFO_DSFREQ_SIZE )
#define BssInfoHopPattern( infoP, destP ) MemMove( destP, &((UInt8 *) infoP)[BSSINFO_HOPPATTERN_OFFSET], BSSINFO_HOPPATTERN_SIZE )
#define BssInfoHopSet( infoP, destP ) MemMove( destP, &((UInt8 *) infoP)[BSSINFO_HOPSET_OFFSET], BSSINFO_HOPSET_SIZE )
#define BssInfoHopDwell( infoP, destP ) MemMove( destP, &((UInt8 *) infoP)[BSSINFO_HOPDWELL_OFFSET], BSSINFO_HOPDWELL_SIZE )
#define BssInfoNetworkMode( infoP, destP ) MemMove( destP, &((UInt8 *) infoP)[BSSINFO_NETWORK_MODE_OFFSET], BSSINFO_NETWORK_MODE_SIZE )
#define BssInfoSupportedRates( infoP, destP ) MemMove( destP, &((UInt8 *) infoP)[BSSINFO_SUPPORTED_RATES_OFFSET], BSSINFO_SUPPORTED_RATES_SIZE )
#define BssInfoChannelId( infoP, destP ) MemMove( destP, &((UInt8 *) infoP)[BSSINFO_CHID_OFFSET], BSSINFO_CHID_SIZE )
#define BssInfoANL( infoP, destP ) MemMove( destP, &((UInt8 *) infoP)[BSSINFO_ANL_OFFSET], BSSINFO_ANL_SIZE )
/*@}*/

/**
 *  @brief Structure to hold BSS Info data
 *
 *  Accessing the BSS scan info returned via the network interface via
 *  the following structure needs to be verified for each specific toolchain
 *  and compiler.  It has been verified with the combination of ADS V1.2 and
 *  CW 8.3.  The data format has been done to be as toolchain/architecture/alignment
 *  independent as possible, however, if a different combination other than the
 *  one listed is used, the format must be verified.  The macro's defined
 *  above can also be used to access the individual entities of the returned
 *  BSS scan data.
 */
typedef struct NetBssInfoType
{
	UInt32	validFlag;
	UInt8	bssMacAddr[BSSINFO_MAC_ADDR_SIZE];
	UInt32	bssSsidLength;
	UInt8	bssSsid[BSSINFO_SSID_SIZE];
	UInt32	bssWepEnabled;
	Int32	bssRssi;
	UInt32	bssNetworkType;						// Net80211NetworkType
	UInt32	bssBeaconPeriod;
	UInt32	bssAtimWindow;
	UInt32	bssDSFreq;
	UInt32	bssHopPattern;
	UInt32	bssHopSet;
	UInt32	bssHopDwell;
	UInt32	bssNetworkMode;						// Net80211InfrastructureModeType
	UInt8	bssSupportedRates[BSSINFO_SUPPORTED_RATES_SIZE];
	UInt32	bssChannelId;
	Int32	bssANL;
} NetBssInfoType, *NetBssInfoTypeP;

/**
 * @brief Structure to hold network query data
 */
typedef struct NetworkDeviceQueryData
{
	UInt32	qid;
	void	*InfoBuffer;
	UInt32	InfoBufferLength;
	UInt32	InfoSize;
	UInt32	InfoSizeRequired;

	/**
	 * Added a link pointer. This is required if the driver needs to provide
	 * an intermediate buffer space as a translation process.
	 */
	struct NetworkDeviceQueryData	*linkP;

} NetQueryType, *NetQueryTypeP;

typedef enum _80211NetworkTypeEnum
{
	net80211_FH,
	net80211_DS,
	net80211_Max			// Never returned from scan.  Can be used for bounds checking
} Net80211NetworkType;

/** Infrastructure mode info returned from a BSS scan */
typedef enum _80211InfrastructureEnum
{
	net80211Bss,
	net80211Infrastructure,
	net80211AutoUnknown,
	net80211InfrastructureMax			// Never returned from scan.  Can be used for bounds checking.
} Net80211InfrastructureModeType;

/**
 * @brief BSS list.
 *
 * This data structure is returned from a BSS scan.  The count field indicates
 * how many BSS structures are provide.
 */
typedef struct BSSListStruct
{
	UInt32			count;
	NetBssInfoType	BSS[1];
} NetBssListType;

/**
 * @brief Structure used to get and set the current SSID for the driver (not the Network interface)
 */
typedef struct NetSSID
{
	UInt32		ssidLength;
	UInt8		ssid[32];
} NetSSID, *NetSSIDP;

/**
 * @brief Structure used to set the wep keys.
 *
 * The API's do not return the wep keys via a query.
 */
typedef struct NetWepKey
{
	UInt32		keyIndex;
	UInt32		keyLength;
	UInt8		key[16];
} NetWepKey, *NetWepKeyP;






#endif // PALM_WIFI_COMMON_H_
