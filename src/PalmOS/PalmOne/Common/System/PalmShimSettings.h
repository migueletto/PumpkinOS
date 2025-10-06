/******************************************************************************
 * Copyright (c) 2005 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/** 
 *@ingroup System
 *
 */
 
/**
 * @file 	PalmShimSettings.h
 * @version 1.0
 * @date 	10/28/2002
 *
 * @brief Shim settings for NetIFSettingGet()/Set().
 *
 * <hr>
 */
 
#ifndef PALM_SHIM_SETTINGS_H_
#define PALM_SHIM_SETTINGS_H_ 

/**
 * NetIFSettings for the shim.
 * Used by NetIFSettingGet()/Set().
 */
typedef enum {

  netIFSettingShimUp = 0xA000,    /**< True if shim is up; read-only. */
  netIFSettingShimHideProgress,   /**< Defaults to false. */
  netIFSettingShimConfig,         /**< Mechanism for passing data between app & proto tasks. */
  netIFSettingShimLog,            /**< Copy the shim log; read-only. */
  netIFSettingShimCreator,        /**< Creator ID of shim; UInt32; read-only. */
  netIFSettingShimVersion,        /**< Version of the shim; Char[32]; read-only. */
  netIFSettingShimLibVersion,     /**< Version of the shim library; UInt32; little-endian; read-only; see sysMakeROMVersion() macro */  
  netIFSettingShimVPNStart,       /**< Connect VPN */
  netIFSettingShimVPNStop         /**< Disconnect VPN */       
} NetIFSettingShimEnum;

#define vpnLibEventNotification 'pVPN'		/**< 		*/

/**
 * IFMediaEvent notifications types
 */
enum VPNLibEventNotificationTypeEnum_
  {
	vpnConnectionUp = 1,			/**< 		*/
	vpnConnectionDown			/**< 		*/
  };

typedef UInt8 VPNLibEventNotificationTypeEnum;	/**< 		*/

/**
 * Notification structure sent in vpnLibEventNotification
 */
typedef struct vpnNotifyVPNLibTag
  {
	VPNLibEventNotificationTypeEnum eType;	/**< 		*/
	UInt8           padding1;		/**< 		*/
	UInt16          padding2;		/**< 		*/
	UInt32          ifCreator;		/**< interface creator */
	UInt16          ifInstance;		/**< interface instance */
	UInt16          padding3;		/**< 		*/
  }
VPNNotifyEventType;

#endif // PALM_SHIM_SETTINGS_H_
