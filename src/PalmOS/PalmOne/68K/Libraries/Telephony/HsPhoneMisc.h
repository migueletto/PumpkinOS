/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/** @ingroup Telephony
 *
 */
 
 
/**
 * @file 	HsPhoneMisc.h
 *
 * @brief  Header File for Phone Library API ---- MISC CATEGORY
 *
 * Notes:
 * 	All implementations of the Handspring Phone Library support a common API. 
 * 	This API is broken up into various categories for easier management.  This file
 * 	defines the Miscellaneous category.  These API calls are Miscellaneous calls that don't
 * 	fit into any other category.  Many of them are device control.				
 */


#ifndef HS_PHONEMISC_H
#define HS_PHONEMISC_H
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
 *  @brief Retrieve the Line state.
 *  
 *  @param refNum:	IN:  Library reference number obtained from SysLibLoad.
 *  @param line:	IN:  Line to query for
 *  @param lineState:	IN:  State of the Line
 *  @retval 0 for success; otherwise failed and returned an error code as defined in “Phone
 *          Library Error Codes.”
 **/
extern Err PhnLibGetLineState(UInt16 refNum, UInt16 line, PhnLineStatePtr lineState)
      PHN_LIB_TRAP(PhnLibTrapGetLineState);

/**
 *  @brief Retrieve state information about the radio.
 *  
 *  @param refNum:	IN:  Library reference number obtained from SysLibLoad.
 *  @param radioState:	IN:  State of the Radio
 *  @retval 0 for success; otherwise failed and returned an error code as defined in “Phone
 *          Library Error Codes.”
 **/
extern Err PhnLibGetRadioState(UInt16 refNum, PhnRadioStatePtr radioState)
          PHN_LIB_TRAP(PhnLibTrapGetRadioState);

/**
 *  @brief 
 *  
 *  @param refNum:	IN:  
 *  @param manufacturer:	IN:  
 *  @param model:	IN:
 *  @param version:	IN:   
 *  @param serial	IN: 
 *  @retval Err Error code.
 **/
extern Boolean PhnLibCardInfo (UInt16 refNum, CharPtr* manufacturer, CharPtr* model, CharPtr* version, CharPtr* serial)
				  PHN_LIB_TRAP(PhnLibTrapCardInfo);

/** 
 *<chg 09-10-2002 TRS> prototype changed.  NOTE that previously was declared to
 *return Err, but code returned Boolean, so didn't work anyway.
 **/
/**
 *  @brief Checks to see if a connection can be made
 *  
 *  @param refNum:	IN:  Library reference number obtained from SysLibLoad.
 *  @param connection:	IN:  Type of connection to be made.
 *  @param pAvailable:	IN: Result indicating if a connection is available. 
 *  @retval 0 for success; otherwise failed and returned an error code as defined in “Phone
 *          Library Error Codes.”
 **/
extern Err PhnLibConnectionAvailable(UInt16 refNum, PhnConnectionEnum connection, Boolean * pAvailable)
				  PHN_LIB_TRAP(PhnLibTrapConnectionAvailable);

/**
 *  @brief This API have been deprecated. Please use “HsIndicatorState.”
 *  
 *  @param refNum:	IN:  
 *  @param pulse:	IN:  
 *  @param repeat:	IN:  
 *  @retval Err Error code.
 **/  
extern Err	PhnLibStartVibrate (UInt16 refNum, Boolean pulse, Boolean repeat)
				  PHN_LIB_TRAP(PhnLibTrapStartVibrate);  
 		
/**
 *  @brief This API have been deprecated. Please use “HsIndicatorState.”
 *  
 *  @param refNum:	IN:  
 *  @retval Err Error code.
 **/
extern Err	PhnLibStopVibrate (UInt16 refNum)
				  PHN_LIB_TRAP(PhnLibTrapStopVibrate);

/**
 *  @brief 
 *  
 *  @param refNum:	IN:  
 *  @param errorRate:	IN:   
 *  @retval Err Error code.
 **/
extern Err	PhnLibErrorRate (UInt16 refNum, WordPtr errorRate)
				  PHN_LIB_TRAP(PhnLibTrapErrorRate);

/**
 *  @brief 
 *  
 *  @param refNum:	IN:  
 *  @param error:	IN:  
 *  @param buffer:	IN:  
 *  @param bufferLen:	IN:
 *  @retval Err Error code.
 **/
extern void PhnLibGetErrorText (UInt16 refNum, Err error, CharPtr buffer, UInt16 bufferLen)
				  PHN_LIB_TRAP(PhnLibTrapGetErrorText);

/**
 *  @brief Locate the call item that has the specified service if such item exists return
 *         the total number of call items that has the same service.
 *  
 *  @param refNum:	IN:  Library reference number obtained from SysLibLoad.
 *  @param service:	IN:  The specified connection service (can OR bit combination)
 *  @retval Number of call items that has the same status.
 **/
extern UInt16     PhnLibGetCallCountByService (UInt16 refNum,
                                             PhoneServiceClassType service)
                PHN_LIB_TRAP (PhnLibTrapGetCallCountByService);

/**
 *  @brief 
 *  
 *  @param refNum:	IN:  
 *  @param echoCancellationOn:	IN:   
 *  @retval Err Error code.
 **/
extern Err PhnLibGetEchoCancellation(UInt16 refNum, Boolean* echoCancellationOn)
			  PHN_LIB_TRAP(PhnLibTrapGetEchoCancellation);

/**
 *  @brief 
 *  
 *  @param refNum:	IN:  
 *  @param echoCancellationOn:	IN:   
 *  @retval Err Error code.
 **/			  
extern Err PhnLibSetEchoCancellation(UInt16 refNum, Boolean echoCancellationOn)
			  PHN_LIB_TRAP(PhnLibTrapSetEchoCancellation);

/**
 *  @brief 
 *  
 *  @param refNum:	IN:  
 *  @param equipmentMode:	IN:   
 *  @retval Err Error code.
 **/
extern Err PhnLibGetEquipmentMode(UInt16 refNum, PhnEquipmentMode* equipmentMode)
			  PHN_LIB_TRAP(PhnLibTrapGetEquipmentMode);

/**
 *  @brief 
 *  
 *  @param refNum:	IN:  
 *  @param equipmentMode:	IN:   
 *  @retval Err Error code.
 **/			  
extern Err PhnLibSetEquipmentMode(UInt16 refNum, PhnEquipmentMode equipmentMode)
			  PHN_LIB_TRAP(PhnLibTrapSetEquipmentMode);

/**
 *  @brief 
 *  
 *  @param refNum:	IN:  
 *  @param headsetConnected:	IN:   
 *  @retval 0 for success; otherwise failed and returned an error code as defined in “Phone
 *          Library Error Codes.”
 **/
extern Err      PhnLibGetHeadsetConnectedInfo (UInt16 refNum, Boolean * headsetConnected)
    PHN_LIB_TRAP (PhnLibTrapGetHeadsetConnectedInfo);

/**
 *  @brief 
 *  
 *  @param refNum:	IN:  
 *  @param gain:	IN:  
 *  @retval Err Error code.
 **/
extern Err	PhnLibGetMicrophone (UInt16 refNum, Int16* gain)
				  PHN_LIB_TRAP(PhnLibTrapGetMicrophone);

/**
 *  @brief 
 *  
 *  @param refNum:	IN:  
 *  @param gain:	IN:  
 *  @retval Err Error code.
 **/
extern Err    PhnLibSetMicrophone (UInt16 refNum, Int16 gain)
			  PHN_LIB_TRAP(PhnLibTrapSetMicrophone);

/**
 *  @brief 
 *  
 *  @param refNum:	IN:  
 *  @param manufacturer:	IN:  
 *  @param model:	IN:  
 *  @param modemSWRev:	IN:
 *  @param esn:		IN:
 *  @param prlRev:	IN:
 *  @param hostSWRev:	IN:
 *  @param modemHWRev:	IN:
 *  @param priChecksum:	IN:
 *  @param *eriver:	IN:
 *  @param *isEriSet:	IN:
 *  @retval Err Error code.
 **/
 extern Boolean  PhnLibCardInfoEx (UInt16 refNum, CharPtr * manufacturer,
                                  CharPtr * model, CharPtr * modemSWRev,
                                  CharPtr * esn, CharPtr * prlRev, CharPtr * hostSWRev, 
								  CharPtr * modemHWRev, CharPtr *priChecksum, Word *eriVer,
								  Boolean *isEriSet)
                  PHN_LIB_TRAP (PhnLibTrapCardInfoEx);

/**
 *  @brief 
 *  
 *  @param refNum:	IN:  
 *  @param phnFlags:	IN:  
 *  @retval Err Error code.
 **/
extern Err PhnLibGetPhoneCallStatus(UInt16 refNum, UInt32* phnFlags)
				  PHN_LIB_TRAP(PhnLibTrapGetPhoneCallStatus);

/**
 *  @brief 
 *  
 *  @param refNum:	IN:  
 *  @param toMute:	IN:  
 *  @retval Err Error code.
 **/
extern Err      PhnLibMute (UInt16 refNum, Boolean toMute)
                PHN_LIB_TRAP (PhnLibTrapMute);

/**
 *  @brief 
 *  
 *  @param refNum:	IN:  
 *  @param isMute:	IN:  
 *  @retval Err Error code.
 **/
extern Err      PhnLibIsCallMuted (UInt16 refNum, Boolean * isMute)
                PHN_LIB_TRAP (PhnLibTrapIsCallMuted);

/**
 *  @brief 
 *  
 *  @param refNum:	IN:   
 *  @retval Err Error code.
 **/
extern Boolean  PhnLibIsPhoneActivated (UInt16 refNum)
                PHN_LIB_TRAP (PhnLibTrapIsPhoneActivated);

/**
 *  @brief 
 *  
 *  @param refNum:	IN:  
 *  @param bSet:	IN:  
 *  @param debugInfoP	IN:  
 *  @param infoLen:	IN:
 *  @retval Err Error code.
 **/
extern Err      PhnLibMiscDebugInfo (UInt16 refNum, Boolean bSet, VoidPtr
                                     debugInfoP, UInt16 infoLen)
                PHN_LIB_TRAP (PhnLibTrapMiscDebugInfo);

/**
 *  @brief 
 *  
 *  @param refNum:	IN:  
 *  @param attr:	IN:  
 *  @param flags:	IN:
 *  @param dataP:	IN:  
 *  @retval Err Error code.
 **/
extern Err      PhnLibAttrGet (UInt16 refNum, UInt16 attr, UInt32 flags, void* dataP)
                PHN_LIB_TRAP (PhnLibTrapAttrGet);

/**
 *  @brief 
 *  
 *  @param refNum:	IN:  
 *  @param attr:	IN:  
 *  @param flags:	IN:  
 *  @param dataP:	IN:
 *  @retval Err Error code.
 **/
extern Err      PhnLibAttrSet (UInt16 refNum, UInt16 attr, UInt32 flags, void* dataP)
                PHN_LIB_TRAP (PhnLibTrapAttrSet);

/**
 *  @brief 
 *  
 *  @param refNum:	IN:  
 *  @param deviceId:	IN:    
 *  @retval Err Error code.
 **/
extern Err      PhnLibGetDeviceID (UInt16 refNum, CharPtr * deviceId)
				PHN_LIB_TRAP (PhnLibTrapGetDeviceId);
#endif
