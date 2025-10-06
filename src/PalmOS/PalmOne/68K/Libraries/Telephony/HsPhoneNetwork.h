/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/** @ingroup Telephony
 *
 */
 
 
/**
 * @file 	HsPhoneNetwork.h
 *
 * @brief  Header File for Phone Library API ---- NETWORK CATEGORY
 *
 * Notes:
 * 	All implementations of the Handspring Phone Library support a common API. 
 * 	This API is broken up into various categories for easier management.  This file
 * 	defines the Network category.  These API calls are used to interact with the wireless network.			
 */


#ifndef HS_PHONENETWORK_H
#define HS_PHONENETWORK_H
#include <PalmOS.h>
#include <PalmTypes.h>
#ifndef __CORE_COMPATIBILITY_H__
#include <PalmCompatibility.h>
	/** workaround for differing header files in sdk-3.5 and sdk-internal */
	#ifndef __CORE_COMPATIBILITY_H__
	#define __CORE_COMPATIBILITY_H__
	#endif 
#endif 
#include <Common/Libraries/Telephony/HsPhoneTraps.h>     /**< trap table definition for phone library calls */
#include <Common/Libraries/Telephony/HsPhoneErrors.h>    /**< error codes returned by phone library functions */
#include <Common/Libraries/Telephony/HsPhoneTypes.h>

/** Phone Library functions */


/**
 *  @brief Gets information on a specific voicemail box as specified in the data argument.
 *
 *  @param refNum:	IN:  Library reference number returned by HsGetPhoneLibrary().
 *  @param data:	IN:  See “PhnMsgBoxDataType” for details
 *  @retval 0 for success; otherwise failed.
 **/

extern Err PhnLibBoxInformation(UInt16 refNum, PhnMsgBoxDataType* data)
			PHN_LIB_TRAP(PhnLibTrapBoxInformation);

/**
 *  @brief Gets or Set information on a specific voicemail box as specified in the data argument.
 *
 *  @param refNum:	IN:  Library reference number returned by HsGetPhoneLibrary().
 *  @param data:	IN:  See “PhnMsgBoxDataType” for details
 *  @param bGetInfo:	IN:  Indicates whether the operation is a get (True) or set (False).
 *  @retval 0 for success; otherwise failed.
 **/
extern Err PhnLibBoxInformationEx (UInt16 refNum, PhnMsgBoxDataType *
                                data, Boolean bGetInfo)
			PHN_LIB_TRAP (PhnLibTrapBoxInformationEx);

/**
 *  @brief Return the number of the voice box of the given type for the given line as 
 *         it is stored on the SIM.
 *
 *  @param refNum:	IN:  Library reference number returned by HsGetPhoneLibrary().
 *  @param type:	IN:  See “PhnMsgBoxType” for details. On Treo™ 600 smartphone by palmOne,
 *                           only the kBoxVoice type is supported
 *  @param line:	IN:  On a GSM/GPRS device, line can be either “1” or “2”. This is dependant on if
 *                           the subscriber account supports 1 or 2 line(s) (on GSM, this is called 
 *                           Alternative Line Service).
 *                           On the CDMA device, it is currently not used.
 *  @param number:	IN:  Returns the number as a string.
 *  @retval PhnLibGetBoxNumber returns 0 if the number could be determined without
 *          encountering an error.
 **/
extern Err PhnLibGetBoxNumber(UInt16 refNum, PhnMsgBoxType type, UInt16 line, CharPtr * number)
			PHN_LIB_TRAP(PhnLibTrapGetBoxNumber);

/**
 *  @brief Get value for home network’s operator ID (MCC+MNC) string
 *
 *  @param refNum:	IN:  Library reference number returned by HsGetPhoneLibrary().
 *  @param *buffer:	IN:  Buffer to hold returned string
 *  @param bufferSizeP:	IN:
 *  @retval 0 for success; otherwise failed
 **/
extern Err PhnLibHomeOperatorID (UInt16 refNum, char *buffer, Int16* bufferSizeP)
			PHN_LIB_TRAP(PhnLibTrapHomeOperatorID);

/**
 *  @brief Get value for current network’s operator ID (MCC+MNC) string
 *
 *  @param refNum:	IN:  Library reference number returned by HsGetPhoneLibrary().
 *  @param *buffer:	IN:  buffer to hold returned string
 *  @param bufferSizeP:	IN:
 *  @retval 0 for success; otherwise failed
 **/
extern Err PhnLibCurrentOperatorID (UInt16 refNum, char *buffer, Int16* bufferSizeP)
			PHN_LIB_TRAP(PhnLibTrapCurrentOperatorID);

    // Used only by the phone app, so change CDMA function to match this prototype -- ignore mode parameter 
/**
 *  @brief Get value for current network’s operator string
 *
 *  @param refNum:	IN:  Library reference number returned by HsGetPhoneLibrary().
 *  @param id:		IN:  returned operator ID value
 *  @param name:	IN:  returned operator name string
 *  @param mode:	IN:  returned registration mode for current operator
 *  @retval 0 for success; otherwise failed
 **/
  extern Err PhnLibCurrentOperator (UInt16 refNum, PhnOperatorID* id, CharPtr* name, GSMRegistrationMode* mode)
				PHN_LIB_TRAP(PhnLibTrapCurrentOperator);
  
  				
  // Use this as base -- CDMA function just returns phnErrNotSupported
/**
 *  @brief Set value of operator for radio to register to
 *
 *  @param refNum:	IN:  Library reference number returned by HsGetPhoneLibrary().
 *  @param op:		IN:  operator to register to
 *  @param regMode:	IN:  type of registration to make
 *  @retval 0 for success; otherwise failed
 **/
  extern Err PhnLibSetOperator (UInt16 refNum, PhnOperatorType* op, GSMRegistrationMode	regMode)
				PHN_LIB_TRAP(PhnLibTrapSetOperator);

    // Change to return err, and add PhnRoamStatus parameter 
/**
 *  @brief Return the roaming status
 *
 *  @param refNum:	IN:  Library reference number returned by HsGetPhoneLibrary().
 *  @param roamStatus:	IN:  PhnLibRoaming returns the roaming status.
 *  @retval 0 if no error; error code otherwise
 **/
  extern Err PhnLibRoaming(UInt16 refNum, PhnRoamStatus * roamStatus)
				PHN_LIB_TRAP(PhnLibTrapRoaming);

/**
 *  @brief Checks if the phone found any network cellular service.
 *
 *  @param refNum:	IN:  Library reference number returned by HsGetPhoneLibrary().
 *  @retval true if service is found.
 *          false if no service
 **/
  extern Boolean PhnLibRegistered (UInt16 refNum)
					PHN_LIB_TRAP(PhnLibTrapRegistered);

/**
 *  @brief Return the signal strength
 *
 *  @param refNum:	IN:  Library reference number returned by HsGetPhoneLibrary().
 *  @param quality:	IN:  Phone signal strength in dBm
 *  @retval 0 if no error
 **/
  extern Err	PhnLibSignalQuality (UInt16 refNum, UInt16 * quality)
					PHN_LIB_TRAP(PhnLibTrapSignalQuality);

/**
 *  @brief Get the signal quality and ecio level information.
 *
 *  @param refNum:	IN:  Library reference number returned by HsGetPhoneLibrary().
 *  @param quality:	IN:  Phone signal strength in dBm
 *  @param *bEcio:	IN:
 *  @param *ecioLvl:	IN:
 *  @retval 0 for success; otherwise failed
 **/
  extern Err   PhnLibSignalQualityEx (UInt16 refNum, UInt16 * quality, Boolean *bEcio, UInt16 *ecioLvl)
          PHN_LIB_TRAP (PhnLibTrapSignalQualityEx);  

/**
 *  @brief Get value for current network’s provider
 *
 *  @param refNum:	IN:  Library reference number returned by HsGetPhoneLibrary().
 *  @param name:	IN:  returned provider name string
 *  @retval 0 for success; otherwise failed
 **/
  extern Err PhnLibCurrentProvider(UInt16 refNum, char** name)
				PHN_LIB_TRAP(PhnLibTrapCurrentProvider);

/**
 *  @brief Get list of operators available to radio
 *
 *  @param refNum:	IN:  Library reference number returned by HsGetPhoneLibrary().
 *  @param list:	IN:  container for returned list of operator values
 *  @retval 0 for success; otherwise failed
 **/
  extern Err PhnLibGetOperatorList (UInt16 refNum, PhnOperatorListPtr * list)
				PHN_LIB_TRAP(PhnLibTrapGetOperatorList);

/**
 *  @brief Return a list of phone numbers from the device. These numbers could be 
 *         voicemail box, data line or fax line (if supported).
 *
 *  @param refNum:	IN:  Library reference number returned by HsGetPhoneLibrary().
 *  @param ownNumbers:	IN:  Pointer to the list of phone box number of the device.
 *  @retval 0 for success; otherwise failed
 **/
  extern Err PhnLibGetOwnNumbers (UInt16 refNum, PhnAddressList* ownNumbers)
				PHN_LIB_TRAP(PhnLibTrapGetOwnNumbers);

/**
 *  @brief <description to be provided>
 *
 *  @param refNum:	IN:  Library reference number returned by HsGetPhoneLibrary().
 *  @param ownNumbers:	IN:
 *  @retval 0 for success; otherwise failed
 **/
  extern Err PhnLibSetOwnNumbers (UInt16 refnum, PhnAddressList ownNumbers)
				PHN_LIB_TRAP(PhnLibTrapSetOwnNumbers);

/**
 *  @brief Enable/disable call waiting for phone calls on radio
 *
 *  @param refNum:	IN:  Library reference number returned by HsGetPhoneLibrary().
 *  @param enabled:	IN:  flag to enable/disable call waiting
 *  @retval 0 for success; otherwise failed
 **/
  extern Err	PhnLibSetCallWaiting (UInt16 refNum, Boolean enabled)
					PHN_LIB_TRAP(PhnLibTrapSetCallWaiting);

/**
 *  @brief Get call waiting setting for phone calls on radio
 *
 *  @param refNum:	IN:  Library reference number returned by HsGetPhoneLibrary().
 *  @param enabled:	IN:  return value of call waiting service on radio.
 *                           true if call waiting is enabled
 *                           false if call waiting is disabled
 *                           value is undefined if return value of function is not 0 (ERROR)
 *  @retval 0 for success; otherwise failed
 **/
  extern Err	PhnLibGetCallWaiting (UInt16 refNum, BooleanPtr enabled)
					PHN_LIB_TRAP(PhnLibTrapGetCallWaiting);

/**
 *  @brief 
 *
 *  @param refNum:	IN:  Library reference number returned by HsGetPhoneLibrary().
 *  @param enable:	IN:  True to enable CLIP.
 *                           False to disable it.
 *  @retval Err Error code.
 **/
  extern Err PhnLibSetCLIP (UInt16 refNum, Boolean enable)
				PHN_LIB_TRAP(PhnLibTrapSetCLIP);

/**
 *  @brief (TBC)
 *
 *  @param refNum:	IN:  Library reference number returned by HsGetPhoneLibrary().
 *  @param enabled:	IN:
 *  @retval Err Error code.
 **/
  extern Err PhnLibGetCLIP(UInt16 refNum, Boolean* enabled)
				PHN_LIB_TRAP(PhnLibTrapGetCLIP);

    // Used only by the phone app, so change CDMA function to match this prototype
/**
 *  @brief Returns the forwarded number stored in smartphone radio.
 *
 *  @param refNum:	IN:  Library reference number returned by HsGetPhoneLibrary().
 *  @param condition:	IN:  forwarding condition to get values for. Currently CDMA only supports
 *                           “phnForwardUnconditional” as defined in “PhnForwardType”.
 *  @param destination:	IN:  returned forwarding address for given forwarding condition
 *  @retval Err 0 for success; otherwise failed
 **/
  extern Err	PhnLibGetForwarding (UInt16 refNum, PhnForwardType condition, PhnAddressHandle * destination)
					PHN_LIB_TRAP(PhnLibTrapGetForwarding);

/**
 *  @brief 
 *
 *  @param refNum:	IN:  Library reference number returned by HsGetPhoneLibrary().
 *  @param mode:	IN:  (TBC)
 *  @retval 0 for success; otherwise failed.
 **/
  extern Err PhnLibSetCLIR (UInt16 refNum, GSMDialCLIRMode mode)
				PHN_LIB_TRAP(PhnLibTrapSetCLIR);

/**
 *  @brief (TBC)
 *
 *  @param refNum:	IN:  Library reference number returned by HsGetPhoneLibrary().
 *  @param mode:	IN:
 *  @param status:	IN:
 *  @retval Err Error code.
 **/
  extern Err PhnLibGetCLIR (UInt16 refNum, GSMDialCLIRMode* mode, PhnCLIRStatus* status)
				PHN_LIB_TRAP(PhnLibTrapGetCLIR);

/**
 *  @brief Used to Activate a Barring Service
 *
 *  @param refNum:	IN:  Library reference number returned by HsGetPhoneLibrary().
 *  @param facility:	IN:  Barring condition to be set
 *  @param password:	IN:  Password to set Barring. This will be used when changing barring option.
 *  @retval 0 for success; otherwise failed
 **/
  extern Err PhnLibSetBarring (UInt16 refNum, PhnBarFacilityType facility, CharPtr password)
					PHN_LIB_TRAP(PhnLibTrapSetBarring);

/**
 *  @brief Get Barring Service state for given facility
 *
 *  @param refNum:	IN:  Library reference number returned by HsGetPhoneLibrary().
 *  @param facility:	IN:  barring condition to get state for
 *  @param enabled:	IN:  returned state of given barring service; true = activated.
 *  @retval 0 for success; otherwise failed
 **/
  extern Err PhnLibGetBarring (UInt16 refNum, PhnBarFacilityType facility, Boolean* enabled)
					PHN_LIB_TRAP(PhnLibTrapGetBarring);

/**
 *  @brief Used to Deactivate a Barring Service
 *
 *  @param refNum:	IN:  Library reference number returned by HsGetPhoneLibrary().
 *  @param facility:	IN:  barring condition to be deactivated
 *  @param password:	IN:
 *  @retval 0 for success; otherwise failed
 **/
  extern Err PhnLibDeactivateBarring (UInt16 refNum, PhnBarFacilityType facility, CharPtr password)
					PHN_LIB_TRAP(PhnLibTrapDeactivateBarring);

/**
 *  @brief Retrieves the roaming indicator text that needs to be displayed if the
 *         phone is in enhanced roaming mode.
 *
 *  @param refNum:	IN:  Library reference number returned by HsGetPhoneLibrary().
 *  @param roamTxt:	IN:  Pointer to a char of “PhnEnhancedRoamIndMaxLength” maximum length.
 *  @param *iconState:	IN:
 *  @retval phnErrNotAllowed if called when the phone is not in enhanced roaming.
 **/
  extern Err PhnLibGetEnhancedRoamIndicator (UInt16 refNum, char * roamTxt,
	  PhnRoamIconState *iconState)
                PHN_LIB_TRAP(PhnLibTrapEnhancedRoamIndicator);

/**
 *  @brief (TBD)
 *
 *  @param refNum:	IN:  Library reference number returned by HsGetPhoneLibrary().
 *  @param Mode:	IN:
 *  @retval Err Error code.
 **/
  extern Err PhnLibGetRoamPrefMode (UInt16 refNum, Boolean * Mode)
                  PHN_LIB_TRAP (PhnLibTrapGetRoamPrefMode);

/**
 *  @brief Set the roaming mode. This API is the same as used when the user 
 *         checks or unchecks the “Enable Digital roaming” in the Phone 
 *         Preference on Treo 600. If set, then digital roaming will be allowed.
 *
 *  @param refNum:	IN:  Library reference number returned by HsGetPhoneLibrary().
 *  @param Mode:	IN:  True enables Digital roaming.
 *                           False disable it.
 *  @retval Error status
 **/
  extern Err PhnLibSetRoamPrefMode (UInt16 refNum, Boolean Mode)
                  PHN_LIB_TRAP (PhnLibTrapSetRoamPrefMode);

/**
 *  @brief 
 *
 *  @param refNum:	IN:  
 *  @param pInfo:	IN:
 *  @retval Err Error code.
 **/
  extern Err PhnLibSetRoamPrefInfo (UInt16 refNum, PhnRoamPrefInfoPtr pInfo)
                  PHN_LIB_TRAP (PhnLibTrapSetRoamPrefInfo);

/**
 *  @brief This function indicate if the Operator has locked the phone to its 
 *         network (i.e. it can’t be used on other network)
 *
 *  @param refNum:	IN:  Library reference number returned by HsGetPhoneLibrary().
 *  @param facilityType:	IN:  optional facility type, or 0 (TBC)
 *  @param enabled:	IN:  pointer to output value
 *  @retval True if a network was found even if we are unable to register to it.
 *          False if no network can be found.
 **/
  extern Err PhnLibGetOperatorLock(UInt16 refNum, UInt16 facilityType, Boolean* enabled)
				PHN_LIB_TRAP(PhnLibTrapGetOperatorLock);

/**
 *  @brief (TBD)
 *
 *  @param refNum:	IN:  Library reference number returned by HsGetPhoneLibrary().
 *  @retval Err Error code.
 **/
  extern Boolean PhnLibNetworkAvailable (UInt16 refNum)
					PHN_LIB_TRAP(PhnLibTrapNetworkAvailable);

  // GSM and CDMA differ
  // Used only by the phone app, so change CDMA function to match this prototype
/**
 *  @brief Set call forwarding settings
 *
 *  @param refNum:	IN:  Library reference number returned by HsGetPhoneLibrary().
 *  @param condition:	IN:  forwarding condition to be set
 *  @param mode:	IN:  forwarding condition value to be set
 *  @param destination:	IN:  forwarding address value
 *  @retval Err 0 for success; otherwise failed
 **/
  extern Err	PhnLibSetForwarding (UInt16 refNum, PhnForwardType condition, PhnForwardModeType mode, PhnAddressHandle destination)
					PHN_LIB_TRAP(PhnLibTrapSetForwarding);

/**
 *  @brief This function gives the threshold level of signal quality at which 
 *         the radio is capable of sending data.
 *
 *  @param refNum:	IN:  Library reference number returned by HsGetPhoneLibrary().
 *  @param threshold:	IN:  (TBC)
 *  @retval 0 for success; otherwise failed
 **/
  extern Err	PhnLibUsableSignalStrengthThreshold (UInt16 refNum, WordPtr threshold)
					PHN_LIB_TRAP(PhnLibTrapUsableSignalStrengthThreshold);

#endif
