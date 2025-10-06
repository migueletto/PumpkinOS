/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/**
 * @defgroup Network Network Library
 *
 * @{
 * @}
 */
/**
 * @ingroup Network
 */

/**
 * @file 	PalmNetServices.h
 * @version 1.0
 *
 * @brief NetServices provides a set of API to access 802.11.
 *
 */

#ifndef PALM_NETSERVICES_H__
#define PALM_NETSERVICES_H__

#ifdef __MC68K__
#include <LibTraps.h>
#endif


/********************************************************************
 * Traps
 ********************************************************************/

/**
 * @name Function Trap Numbers
 */
/*@{*/
#define NetServicesLibTrapGetHWAddr			(sysLibTrapCustom+0)
#define NetServicesLibTrapStartScan			(sysLibTrapCustom+1)
#define NetServicesLibTrapGetScanResults	(sysLibTrapCustom+2)
#define NetServicesLibTrapGetChannel		(sysLibTrapCustom+3)
#define NetServicesLibTrapGetPowerState		(sysLibTrapCustom+4)
#define NetServicesLibTrapGetCommsQuality	(sysLibTrapCustom+5)
#define NetServicesLibTrapSetRadioState		(sysLibTrapCustom+6)
#define NetServicesLibTrapGetRadioState		(sysLibTrapCustom+7)
#define NetServicesLibTrapGetDeviceInfo		(sysLibTrapCustom+10)
#define NetServicesLibTrapGetChannelList	(sysLibTrapCustom+11)

#define NetServicesLibTrapProfileGetInfo	(sysLibTrapCustom+0x10)
#define NetServicesLibTrapProfileRead		(sysLibTrapCustom+0x11)
#define NetServicesLibTrapProfileWrite		(sysLibTrapCustom+0x12)
#define NetServicesLibTrapProfileAdd		(sysLibTrapCustom+0x13)
#define NetServicesLibTrapProfileDelete		(sysLibTrapCustom+0x14)
#define NetServicesLibTrapProfileSetOrder	(sysLibTrapCustom+0x15)
#define NetServicesLibTrapProfileGetOrder	(sysLibTrapCustom+0x16)
#define NetServicesLibTrapProfileFind		(sysLibTrapCustom+0x17)


/*@}*/

/********************************************************************
 * Library type and creator
 ********************************************************************/

#define ShlLibName		"NetServLib"	/**< PalmNetServices library name. */
#define ShlCreatorID	'NETs'			/**< PalmNetServices creator ID. */
#define ShlLibType		'libr'			/**< PalmNetServices library type. */


/********************************************************************
 * Typedefs and constants for the device information returned from
 * NetServicesGetDeviceInfo.
 ********************************************************************/

#define kNetDriverDateLength	16		/**< Length for the driver date info string. */
#define kNetFWNameLength		16		/**< Length for the firmware name info string. */
#define kNetSerialNumberLength	16		/**< Length for the serial number info string. */

/**
 * @brief Device information concerning Hardware and Firmware information for the "adapter".
 *
 * Revision information and product name data is returned.
 */
typedef struct Net80211DeviceInfo
{
	UInt32		validFlags;								/**< Bit flag indicating validity of fields. */
	UInt32		fwRev[2];								/**< Firmware revision. STA/BB (32 bits), PRI/MAC (32 bits). Valid is (validFlags & 1) == 1.*/
	Int32		serialNumberSize;						/**< Serial number size. Valid is (validFlags & 8) == 1. */
	UInt8		serialNumber[kNetSerialNumberLength];	/**< Serial number. Valid is (validFlags & 8) == 1. */
	Char		driverDate[kNetDriverDateLength];		/**< Driver date. Valid is (validFlags & 2) == 1. */
	Char		fwName[kNetFWNameLength];				/**< Firmware name. Valid is (validFlags & 4) == 1. */
} Net80211DeviceInfo, *Net80211DeviceInfoP;


/**
 * @brief Data returned from NetServicesProfileGetInfo.
 */
typedef struct NetServicesProfileInfo
{
	UInt16	profileRev;			/**< Revision of profile database. */
	UInt16	numProfiles;		/**< Number of profiles contained in the database. */
	UInt16	profileEntrySize;	/**< Size of each individual profile entry. */
} NetServicesProfileInfo, *NetServicesProfileInfoP;

/**
 * @brief Profile entries for SmartConnect
 */
typedef struct NetServicesProfile
{
	Char	profileName[32];	/**< ASCII string - profile reference. */
	UInt8	SSID[32];			/**< SSID associated with this profile. */
	UInt32	ssidLength;			/**< Length of bytes of the SSID. */
	UInt16	ipAllocType;		/**< DHCP/STATIC/IPIPA. */
	UInt16	preambleType;		/**< Long or short preamble. */
	UInt16	keyIndex;			/**< WEP key index.  Only valid if wep active. */
	UInt16	authentication;		/**< OPEN / 64 bit / 128 bit. */
	UInt32	opMode;				/**< BSS, IBSS. */
	UInt32	channel;			/**< DS Channel.  Only used for ad-hoc. */
	UInt32	reqIP;				/**< Static IP address. */
	UInt32	ipMask;				/**< IP mask. */
	UInt32	ipGateway;			/**< IP gateway. */
	UInt32	ipPrimaryDNS;		/**< IP primary DNS. */
	UInt32	ipSecondaryDNS;		/**< IP secondary DNS. */
	UInt8	wep[4][16];			/**< WEP keys. Up to 5 keys. */
	UInt8	dhcpRequestDNS;		/**< 0 = DNS info static, 1 = DNS from DHCP. */
#ifdef WIFI_DOT1X_SUPPORT
        UInt32  wppiCreator;			/**< Creator ID of Wi-Fi panel plugin (WPPI). */
        UInt32  authenticationMethodID;	/**< Identified authentication method to WPPI. */
        UInt32  authenticationCookie;	/**< Identifies credentials to WPPI. */
#endif // WIFI_DOT1X_SUPPORT
} NetServicesProfile, *NetServicesProfileP;

/**
 * @brief Profile entries version 1
 */
typedef struct NetServicesProfileV1
{
	Char	profileName[32];	/**< ASCII string - profile reference. */
	UInt8	SSID[32];			/**< SSID associated with this profile. */
	UInt32	ssidLength;			/**< Length of bytes of the SSID. */
	UInt16	ipAllocType;		/**< DHCP/STATIC/IPIPA. */
	UInt16	preambleType;		/**< Long or short preamble. */
	UInt16	keyIndex;			/**< WEP key index.  Only valid if wep active. */
	UInt16	authentication;		/**< OPEN / 64 bit / 128 bit. */
	UInt32	opMode;				/**< BSS, IBSS. */
	UInt32	channel;			/**< DS Channel.  Only used for ad-hoc. */
	UInt32	reqIP;				/**< Static IP address. */
	UInt32	ipMask;				/**< IP mask. */
	UInt32	ipGateway;			/**< IP gateway. */
	UInt32	ipPrimaryDNS;		/**< IP primary DNS. */
	UInt32	ipSecondaryDNS;		/**< IP secondary DNS. */
	UInt8	wep[4][16];			/**< WEP keys. Up to 5 keys. */
	UInt8	dhcpRequestDNS;		/**< 0 = DNS info static, 1 = DNS from DHCP. */
} NetServicesProfileV1, *NetServicesProfileV1P;



/********************************************************************
 * Priority order information
 ********************************************************************/

#define MAX_ACTIVE_PROFILES		32		/**< Maximum number of profiles. */
#define END_OF_LIST				0xFFFF	/**< End of list marker. */
#define PRIORITY_LIST_SIZE		(sizeof(UInt16) * (MAX_ACTIVE_PROFILES + 1)) /**< Priority list size. */
#define INVALID_INDEX			0xFFFF	/**< Invalid index tag. */

/**
 * @brief Priority order to smart connect.
 *
 * Provided for ease of reference.
 */
typedef struct NetServicesProfileOrder
{
	UInt16	order[MAX_ACTIVE_PROFILES+1];	/**< Array of order for each active profiles. */
} NetServicesProfileOrder, *NetServicesProfileOrderP;

/********************************************************************
 * Prototypes
 ********************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

#ifdef __MC68K__
/**
 * Gets the HW address (i.e. MAC address) for the interface
 *
 * @param  	libRefNum:	IN:  Reference number of the library.
 * @param	hwAddrP:	OUT: Pointer to store MAC address in.
 *							 Must be 6 bytes in length.
 *
 * @retval	Err			Error code.
 */
Err NetServicesGetHWAddr( UInt16 libRefNum, UInt8 *hwAddrP )
				SYS_TRAP(NetServicesLibTrapGetHWAddr);

/**
 * Inititate a BSS scan.
 * The scan completes by broadcasting a notification of
 * type WIFI_SCAN_COMPLETE_EVENT.
 *
 * @param  	libRefNum:	IN: Reference number of the library.
 *
 * @retval	Err			Error code.
 */
Err NetServicesStartScan( UInt16 libRefNum )
				SYS_TRAP(NetServicesLibTrapStartScan);

/**
 * Retreives scan results from a recent request for a  BSS scan.
 * The caller can determine the amount of data available by passing
 * a NULL pointer and a zero size for the buffer. The network device
 * driver will return the space required to store the result.
 *
 * @param  	libRefNum:		IN:  Reference number of the library.
 * @param	scanResultsP:	OUT: Pointer to storage area which will
 *							     hold the BSS scan data.
 * @param	scanBufSizeP:	OUT: Pointer to variable which holds the
 *							     size of the buffer.  Also used as
 *							     output to determine size of results.
 *
 * @retval	Err				Error code.
 */
Err NetServicesGetScanResults( UInt16 libRefNum, UInt8 *scanResultsP, UInt32 *scanBufSizeP )
				SYS_TRAP(NetServicesLibTrapGetScanResults);

/**
 * Retreives the current channel number being used by the adapter.
 * The exact method to get this data is determined by the adapter driver.
 *
 * @param  	libRefNum:	IN:  Reference number of the library.
 * @param	channelP	OUT: Pointer to storage area to store current
 *						     channel number.
 *
 * @retval	Err			Error code.
 */
Err NetServicesGetChannel( UInt16 libRefNum, UInt16 *channelP )
				SYS_TRAP(NetServicesLibTrapGetChannel);

/**
 * Retreives the current mode (i.e. DSS, Freq. Hop) being  used
 * by the adapter.  The exact method to get this data is determined
 * by the adapter driver.
 *
 * @param  	libRefNum:	IN:  Reference number of the library.
 * @param	stateP:		OUT: Pointer to storage area to store current mode.
 *
 * @retval	Err			Error code.
 */
Err NetServicesGetPowerState( UInt16 libRefNum, UInt16 *stateP )
				SYS_TRAP(NetServicesLibTrapGetPowerState);

/**
 * Retreives the comms quality from the driver / firmware.
 * The comms quality is the signal strength and quality.
 *
 * @param  	libRefNum:	IN:  Reference number of the library.
 * @param	strengthP:	OUT: Pointer to storage area to store signal strength.
 * @param	qualityP:	OUT: Pointer to storage area for signal quality.
 *
 * @retval	Err			Error code.
 */
Err NetServicesGetCommsQuality( UInt16 libRefNum, UInt8 *strengthP, UInt8 *qualityP )
				SYS_TRAP(NetServicesLibTrapGetCommsQuality);

/**
 * Enables/disable the radio at the driver.
 *
 * @param  	libRefNum:	IN: Reference number of the library.
 * @param	radioState:	IN: Boolean value indicating on/off.
 *
 * @retval	Err			Error code.
 */
Err NetServicesSetRadioState( UInt16 libRefNum, UInt32 radioState )
				SYS_TRAP(NetServicesLibTrapSetRadioState);

/**
 * Returns current status of the radio (0 = disabled, 1 = enabled).
 *
 * @param  	libRefNum:		IN:  Reference number of the library.
 * @param	radioStateP:	OUT: Pointer to storage area for the radio status.
 *
 * @retval	Err				Error code.
 */
Err NetServicesGetRadioState( UInt16 libRefNum, UInt32 *radioStateP )
				SYS_TRAP(NetServicesLibTrapGetRadioState);


/**
 * Queries the Firmware on the 802.11 chipset for revision and status info.
 *
 * @param  	libRefNum:	IN:  Reference number of the library.
 * @param	infoP:		OUT: Pointer to storage area for the firmware info.
 *
 * @retval	Err			Error code.
 */
Err NetServicesGetDeviceInfo( UInt16 libRefNum, Net80211DeviceInfoP infoP )
				SYS_TRAP(NetServicesLibTrapGetDeviceInfo);

/**
 * Retrieves the allowed channel list from the device driver.
 * Used for regulatory domain control.  The channel list is a 16 bit,
 * bit significant field with each bit indicating the channel + 1
 * (i.e. bit 0 = CHN1).
 *
 * @param  	libRefNum:		IN:  Reference number of the library.
 * @param	channelListP:	OUT: Pointer to storage area for the channel list.
 *
 * @retval	Err				Error code.
 */
Err NetServicesGetChannelList( UInt16 libRefNum, UInt16 *channelListP )
				SYS_TRAP(NetServicesLibTrapGetChannelList);

/**
 * Provides information about the current profile database and profile manager.
 *
 * @param  	libRefNum:	IN:  Reference number of the library.
 * @param	infoP:		OUT: Pointer to storage area where the info will be stored.
 *
 * @retval	Err			Error code.
 */
Err NetServicesProfileGetInfo( UInt16 libRefNum, NetServicesProfileInfoP infoP )
				SYS_TRAP(NetServicesLibTrapProfileGetInfo);

/**
 * Reads the specified record from the database.  The index is verified
 * first and then the data read into the user space.
 *
 * @param  	libRefNum:	IN:  Reference number of the library.
 * @param	profIndex:	IN:  Profile index number.
 * @param	profileP:	OUT: Pointer to area where the profile will be read.
 *
 * @retval	Err			Error code.
 */
Err NetServicesProfileRead( UInt16 libRefNum, UInt16 profIndex, NetServicesProfileP profileP )
				SYS_TRAP(NetServicesLibTrapProfileRead);

/**
 * Updates the content of the specified record.
 *
 * @param  	libRefNum:	IN: Reference number of the library.
 * @param	profIndex:	IN: Record index number.
 * @param	profileP:	IN: Pointer to profile data to be written to the database.
 *
 * @retval	Err 		Error code.
 */
Err NetServicesProfileWrite( UInt16 libRefNum, UInt16 profIndex, NetServicesProfileP profileP )
				SYS_TRAP(NetServicesLibTrapProfileWrite);

/**
 * Deletes/removes the specified record from the database.
 *
 * @param  	libRefNum:	IN: Reference number of the library.
 * @param	profIndex:	IN: Profile index number.
 *
 * @retval	Err			Error code.
 */
Err NetServicesProfileDelete( UInt16 libRefNum, UInt16 profIndex )
				SYS_TRAP(NetServicesLibTrapProfileDelete);

/**
 * Adds a new record to the end of the profiles database.
 *
 * @param  	libRefNum:	IN: Reference number of the library.
 * @param	profileP:	IN: Pointer to new profile entry.
 *
 * @retval	Err			Error code.
 */
Err NetServicesProfileAdd( UInt16 libRefNum, NetServicesProfileP profileP )
				SYS_TRAP(NetServicesLibTrapProfileAdd);

/**
 * Requests to update the priority order of the profiles.
 * The routine verifies all the profile entries exist and
 * then updates the profile header with the info.
 *
 * @param  	libRefNum:	IN: Reference number of the library.
 * @param	orderP:		IN: Pointer to new priority list.
 *						    Must be terminated by END_OF_LIST (0xFFFF).
 *
 * @retval	Err			Error code.
 */
Err NetServicesProfileSetOrder( UInt16 libRefNum, NetServicesProfileOrderP orderP )
				SYS_TRAP(NetServicesLibTrapProfileSetOrder);

/**
 * Requests to retreive the current priority order to the SmartConnect.
 *
 * @param  	libRefNum:	IN: Reference number of the library.
 * @param	orderP:		OUT: Pointer to storage area where the list will be stored.
 *
 * @retval	Err			Error code.
 */
Err NetServicesProfileGetOrder( UInt16 libRefNum, NetServicesProfileOrderP orderP )
				SYS_TRAP(NetServicesLibTrapProfileGetOrder);

/**
 * Searches through the records in the database for the specified
 * profile name.  If the profile exists, the index is returned to
 * the caller. A value of INVALID_INDEX indicates the profile was not found.
 *
 * @param  	libRefNum:	IN:  Reference number of the library.
 * @param	profNameP:	OUT: Pointer to storage area where the list will be stored.
 *
 * @retval	Err			Error code.
 */
Err NetServicesProfileFind( UInt16 libRefNum, Char *profNameP )
				SYS_TRAP(NetServicesLibTrapProfileFind);


#else	// __MC68K__
Err NetServicesGetHWAddr( UInt16 refnum, UInt8 *hwAddrP );
Err NetServicesStartScan( UInt16 refnum );
Err NetServicesGetScanResults( UInt16 refnum, UInt8 *scanResultsP, UInt32 *scanBufSizeP );
Err NetServicesGetChannel( UInt16 refnum, UInt16 *channelP );
Err NetServicesGetPowerState( UInt16 refnum, UInt16 *modeP );
Err NetServicesGetCommsQuality( UInt16 refNum, UInt8 *strengthP, UInt8 *qualityP );
Err NetServicesSetRadioState( UInt16 refNum, UInt32 enable );
Err NetServicesGetRadioState( UInt16 libRef, UInt32 *radioStateP );
Err NetServicesGetDeviceInfo( UInt16 refNum, Net80211DeviceInfoP storageP );
Err NetServicesGetChannelList( UInt16 refNum, UInt16 *channelP );

Err NetServicesProfileGetInfo( UInt16 libRef, NetServicesProfileInfoP infoP );
Err NetServicesProfileRead( UInt16 libRef, UInt16 profIndex, NetServicesProfileP profileP );
Err NetServicesProfileWrite( UInt16 libRef, UInt16 profIndex, NetServicesProfileP profileP );
Err NetServicesProfileDelete( UInt16 libRef, UInt16 profIndex );
Err NetServicesProfileAdd( UInt16 libRef, NetServicesProfileP profileP );
Err NetServicesProfileSetOrder( UInt16 libRef, NetServicesProfileOrderP orderP );
Err NetServicesProfileGetOrder( UInt16 libRef, NetServicesProfileOrderP orderP );
UInt16 NetServicesProfileFind( UInt16 libRef, Char *profNameP );


#endif 	// __MC68K__

#ifdef __cplusplus
}
#endif

#endif // PALM_NETSERVICES_H__
