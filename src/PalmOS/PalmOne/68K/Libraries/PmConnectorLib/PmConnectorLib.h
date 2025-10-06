/******************************************************************************
 * Copyright (c) 2004-2005 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/**
 * @ingroup PmConnectorLib
 */

/**
 * @file  PmConnectorLib.h
 * @brief palmOne library that exports multi-connector related APIs.
 *
 */

#ifndef __PMCONNECTORLIB_H__
#define __PMCONNECTORLIB_H__


#include <Common/Libraries/PmConnectorLib/PmConnectorLibCommon.h>

// To Define when building the library
#if (CPU_TYPE != CPU_68K) || (defined BUILDING_PMCONNECTOR_LIB)
	#define PMCONNECTOR_LIB_TRAP(trapNum)
#else
	#include <LibTraps.h>
	#define PMCONNECTOR_LIB_TRAP(trapNum) SYS_TRAP(trapNum)
#endif


/********************************************************************
 * Prototypes
 ********************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Standard library open routine
 *
 * @param refNum:   IN: library reference number
 * @retval Err Error code.
 */
Err
PmConnectorLibOpen (UInt16 refNum)
      PMCONNECTOR_LIB_TRAP (kPmConnectorLibTrapOpen);

/**
 * @brief Standard library close routine
 *
 * @param refNum:   IN: library reference number
 * @retval Err Error code.
 */
Err
PmConnectorLibClose (UInt16 refNum)
      PMCONNECTOR_LIB_TRAP (kPmConnectorLibTrapClose);

/**
 * @brief Standard library sleep routine
 *
 * @param refNum:   IN: library reference number
 * @retval Err Error code.
 */
Err
PmConnectorLibSleep (UInt16 refNum)
      PMCONNECTOR_LIB_TRAP (kPmConnectorLibTrapSleep);

/**
 * @brief Standard library wake routine
 *
 * @param refNum:   IN: library reference number
 * @retval Err Error code.
 */
Err
PmConnectorLibWake (UInt16 refNum)
      PMCONNECTOR_LIB_TRAP (kPmConnectorLibTrapWake);

/**
 * @brief Checks the version of the library present on the device
 *
 * @param refNum:       IN: library reference number
 * @param sdkVersion:   IN: sdkVersion defined in the common header file
 * @param LibVersionP:  IN: version of the library on the device
 * @retval Err Error code. Function returns error when it finds incompatible library
 */
Err
PmConnectorLibGetVersion (UInt16 refNum, UInt32 sdkVersion, UInt32 *LibVersionP)
      PMCONNECTOR_LIB_TRAP (kPmConnectorLibTrapGetVersion);

/**
 * @brief Controls the pins on the connector. Currently supports power on and off
 *
 * @param refNum:   IN: library reference number
 * @param cmdId:    IN: type of control. @see PmConnectorLibControlType
 * @param parmP:    IN: contains the value needed depending on type of control
 * @retval Err Error code.
 */
Err
PmConnectorLibControl (UInt16 refNum, UInt16 cmdId, void *parmP)
      PMCONNECTOR_LIB_TRAP (kPmConnectorLibTrapControl);

/**
 * @brief Targets audio output to different audio peripherals.
 *
 * @param refNum            IN: library reference number
 * @param outputSetting:    IN: audio capabilities (headphone, speaker, etc)
 * @retval Err Error code.
 */
Err
PmConnectorLibSetAudioOutput(UInt16 refNum, PmConnectorAudioOutputSettingType outputSetting)
      PMCONNECTOR_LIB_TRAP (kPmConnectorLibTrapSetAudioOutput);

/**
 * @brief Targets audio input from different audio peripherals.
 *
 * @param refNum            IN: library reference number
 * @param inputSetting:    IN: audio capabilities (headset etc)
 * @retval Err Error code.
 */
Err
PmConnectorLibSetAudioInput(UInt16 refNum, PmConnectorAudioInputSettingType inputSetting)
      PMCONNECTOR_LIB_TRAP (kPmConnectorLibTrapSetAudioInput);


//
// To maintain the same dispatch entry as the MDF the next customcontrol should be  (sysLibTrapCustom + 6)
//
#ifdef __cplusplus
}
#endif


#endif  // __PMCONNECTORLIB_H__
