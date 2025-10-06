/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/**
 * @ingroup Transparency
 */

/**
 * @file  TransparencyLib.h
 * @brief Transparency Library that couples a Ccp channel and a serial port
 *
 * This library can be used to couple data between a Ccp channel and a serial
 * port. It is meant to be used for diagnostics and tethered mode. Since it is a
 * library, it can work without being in a certain application.
 */

#ifndef __TRANSPARENCY_LIB_INT__H__
#define __TRANSPARENCY_LIB_INT__H__

#ifdef __cplusplus
extern "C" {
#endif

/// Open the Transparency Library and connect the radio modem directly with a
/// known port.
///
/// @param refNum:			IN: Library Reference Number.
/// @param portAChannel:	IN: CcpChannelType of radio port.
/// @param portB:			IN: Port ID of the second port. This is the serial or USB port.
/// @retval Err				Error code.
///
/// @code
/// err = SysLibFind (transLibName, &refNum);
/// if (!err)
///     err=TransLibOpenConnection(refNum,kCcpChannelTypeModem, 0x8000);
/// @endcode
Err TransLibOpenConnection (UInt16 refNum, CcpChannelType portAChannel, UInt32 portB)
                                               SYS_TRAP (transLibTrapOpenConnection);

/// Close the Transparency Library previously open and set the radio module in the
/// correct state.
///
/// @param refNum: IN: Library Reference Number.
/// @retval Err Error code.
Err TransLibCloseConnection (UInt16 refNum)
                           SYS_TRAP (transLibTrapCloseConnection);
#ifdef __cplusplus
}
#endif

#endif // __TRANSPARENCY_LIB_INT__H__
