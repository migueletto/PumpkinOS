/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/**
 * @defgroup CCP Communication Channel Provider Library
 *
 * @{
 * @}
 */

/**
 * @file  ComChannelProviderTypes.h
 * @brief Communication channel Public constants and structs that are
 *        used by both the ARM and 68K function calls.
 *
 */

/*
 * \author Robert Gosselin
 *
 * $Id: //device/handheld/dev/sdk/2.1/incs/common/libraries/comchannelprovider/ComChannelProviderTypes.h#4 $
 *
 *****************************************************************************/

// INCLUDE ONCE
#ifndef __COM_CHANNEL_PROVIDER_PUBLIC_COMMON__H__

#define __COM_CHANNEL_PROVIDER_PUBLIC_COMMON__H__

#if 0
#pragma mark -------- Structures --------
#endif

/**
 * There are the different types of channels that can be opened. They are passed
 * as the paramater to the CcpOpenChannel function call.
 **/
enum
{
  kCcpChannelTypeControl = 0,  /**< Open a channel to send/rcv control messages */
  kCcpChannelTypeModem,        /**< Open a channel to send/rcv modem type messages */
  kCcpChannelTypeDebug,        /**< Open a channel to rcv debug messages. */
  kCcpChannelTypeUpdate,       /**< Open a channle to update the firmare of the radio */
  kCcpChannelTypePassThrough   /**< Open a channle to do no MUX Passthrough
								    direct to the radio.*/
};
typedef UInt16 CcpChannelType; /**< Used with kCcpChannelTypeEnum:
                                 *  - kCcpChannelTypeControl
                                 *  - kCcpChannelTypeModem
                                 *  - kCcpChannelTypeDebug
                                 *  - kCcpChannelTypeUpdate
                                 *  - kCcpChannelTypePassThrough
                                 */



#endif // INCLUDE ONCE -- __COM_CHANNEL_PROVIDER_PUBLIC_COMMON__H__
