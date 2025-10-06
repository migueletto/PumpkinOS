/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/** @ingroup Telephony
 *
 */
 
 
/**
 * @file 	HsPhoneGSM.h
 *
 * @brief  Header File for Phone Library API ---- GSM CATEGORY
 *
 * Notes:
 * 	All implementations of the Handspring Phone Library support a common API. 
 * 	This API is broken up into various categories for easier management.  This file
 * 	defines the SMS category.  These API calls are used for features that only apply
 * 	to GSM networks.							
 */


#ifndef HS_PHONEGSM_H
#define HS_PHONEGSM_H
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
 *  @brief Sets the active line number.
 *
 *  @param refNum: 	IN:  Library reference number obtained from SysLibLoad.
 *  @param line:	IN:  Line number to be set to active.
 *  @retval 0 for success; otherwise failed and returned an error code 
 *          as defined in “Phone Library Error Codes.”
 **/
extern Err PhnLibSetActiveLine(UInt16 refNum, Int16 line)
        PHN_LIB_TRAP(PhnLibTrapSetActiveLine);


/**
 *  @brief Retrieve the Fixed Dialing Numbers from the SIM.
 *
 *  @param refNum: 	IN:  Library reference number obtained from SysLibLoad.
 *  @param aList: 	IN:  Returned with the list of numbers
 *  @param info: 	IN:
 *  @retval 0 for success; otherwise failed and returned an error code 
 *          as defined in “Phone Library Error Codes.”
 **/
extern Err PhnLibGetFDNList(UInt16 refNum, PhnAddressList* aList, PhnPhoneBookInfoPtr info)
        PHN_LIB_TRAP(PhnLibTrapGetFDNList);

/**
 *  @brief Set entry in the Fix Dialing field on the SIM.
 *
 *  @param refNum: 	IN:  Library reference number obtained from SysLibLoad.
 *  @param index:	IN:  index into SIM
 *  @param name: 	IN:  name value for SIM entry
 *  @param number:	IN:  phone number for SIM entry
 *  @retval 0 for success; otherwise failed and returned an error code 
 *          as defined in “Phone Library Error Codes.”
 **/
extern Err PhnLibSetFDNEntry (UInt16 refnum, UInt16 index, char* name, char * number)
			  PHN_LIB_TRAP(PhnLibTrapSetFDNEntry);

/**
 *  @brief This routine reads the PhoneBook of phone numbers from your SIM (GSM only).
 *         This command can take several seconds if the PhoneBook is large.
 *
 *  @param refNum: 	IN:  Library reference number obtained from SysLibLoad.
 *  @param numbers:	IN:
 *  @param info: 	IN:
 *  @retval 0 for success; otherwise failed and returned an error code 
 *          as defined in “Phone Library Error Codes.”
 **/
extern Err PhnLibGetPhoneBook (UInt16 refNum, PhnAddressList* numbers, PhnPhoneBookInfoPtr info)
			  PHN_LIB_TRAP(PhnLibTrapGetPhoneBook);

/**
 *  @brief This routine writes the PhoneBook of phone numbers from your SIM (GSM only).
 *         This command can take several seconds if the PhoneBook is large.
 *
 *  @param refNum: 	IN:  Library reference number obtained from SysLibLoad.
 *  @param numbers:	IN:
 *  @retval 0 for success; otherwise failed and returned an error code 
 *          as defined in “Phone Library Error Codes.”
 **/
extern Err PhnLibSetPhoneBook (UInt16 refnum, PhnAddressList numbers)
			  PHN_LIB_TRAP(PhnLibTrapSetPhoneBook);

/**
 *  @brief Set entry in SIM Phone Book
 *
 *  @param refNum: 	IN:  Library reference number obtained from SysLibLoad.
 *  @param index:	IN:  index into SIM Phone Book
 *  @param name:	IN:  name value for SIM Phone Book entry
 *  @param number:	IN:  phone number for SIM Phone Book entry
 *  @retval 0 for success; otherwise failed and returned an error code 
 *          as defined in “Phone Library Error Codes.”
 **/
extern Err PhnLibSetPhoneBookEntry (UInt16 refnum, UInt16 index, char* name, char * number)
			  PHN_LIB_TRAP(PhnLibTrapSetPhoneBookEntry);

/**
 *  @brief This routine gets the index of the given PhoneBook address on your SIM (GSM only).
 *
 *  @param unused:	IN:  for the future, set to 0.
 *  @param address: 
 *  @retval Index number.
 **/
extern UInt32 PhnLibGetPhoneBookIndex(UInt16, PhnAddressHandle address)
				  PHN_LIB_TRAP(PhnLibTrapGetPhoneBookIndex);


/**
 *  @brief
 *
 *  @param refNum: 	IN:
 *  @param aList: 	IN:
 *  @param info: 	IN:
 *  @retval Err Error code.
 **/
extern Err PhnLibGetSDNList(UInt16 refNum, PhnAddressList* aList, PhnPhoneBookInfoPtr info)
        PHN_LIB_TRAP(PhnLibTrapGetSDNList);

/**
 *  @brief Send USSD value to radio
 *
 *  @param refNum: 	IN:  Library reference number obtained from SysLibLoad.
 *  @param cmd:		IN:  USSD value to be sent
 *  @retval 0 for success; otherwise failed and returned an error code 
 *          as defined in “Phone Library Error Codes.”
 **/
extern Err	PhnLibSendUSSD (UInt16 refNum, CharPtr cmd)
				  PHN_LIB_TRAP(PhnLibTrapSendUSSD);

/**
 *  @brief Retrieve SAT main menu.
 *
 *  @param refNum: 		IN:  Library reference number obtained from SysLibLoad.
 *  @param menuHandlePtr:	IN:  Pointer to a GSMSATEventMenuPtr.
 *  @retval 0 for success; otherwise failed and returned an error code 
 *          as defined in “Phone Library Error Codes.” And the pointer 
 *          is filled with a pointer to a main menu or phnErrSATUnavailable 
 *          if there is no active session.
 **/
extern Err PhnLibSATGetMainMenu(UInt16 refNum, MemHandle* menuHandlePtr)
			  PHN_LIB_TRAP(PhnLibTrapSATGetMainMenu);

/**
 *  @brief Send application’s request to firmware
 *
 *  @param refNum: 	IN:  Library reference number obtained from SysLibLoad.
 *  @param request:	IN:  Request type
 *  @param decision:	IN:  Decision Type
 *  @param data:	IN:  Data to send
 *  @param length:	IN:  Length of data to send (only for Input)
 *  @retval 0 if succeed
 *          phnErrIllegalCondition:unrecognized parameters or an unneeded param is not zero.
 *          phnErrSATUnavailable:SAT is not available
 *          phnErrSATInactive : no active SAT session; must call
 *              PhnLibSATGetMainMenu() before calling this function().
 **/
extern Err PhnLibSATSendRequest(UInt16 refNum, SATRequestType request, SATDecisionType decision, UInt32 data, UInt32 length)
			  PHN_LIB_TRAP(PhnLibTrapSATSendRequest);

/**
 *  @brief Terminate an active SAT session.
 *
 *  @param refNum: 	IN: Library reference number obtained from SysLibLoad.
 *  @retval 0 if succeeded or an error from PhnLibSATSendRequest() session
 **/
extern Err PhnLibSATEndSession(UInt16 refNum)
			  PHN_LIB_TRAP(PhnLibTrapSATEndSession);

/**
 *  @brief This routine reads the IMSI number from the SIM (Subscriber Information
 *         Module) on your GSM device. IMSI stands for International Mobile Subscriber
 *         Identify, and is used to uniquely identify a user to the wireless network.
 *
 *  @param refNum: 	IN:  Library reference number obtained from SysLibLoad.
 *  @param imsi:	IN:  imsi number is a string pointer.
 *  @retval Boolean
 **/
extern Boolean PhnLibSIMInfo (UInt16 refNum, CharPtr* imsi)
			  PHN_LIB_TRAP(PhnLibTrapSIMInfo);

/**
 *  @brief This routine returns status of the SIM (Subscriber Information Module) on your
 *         GSM device. The SIM is managed by the radio module on the handset. The SIM
 *         must be present and active before the radio is allowed to make phone calls (other
 *         than emergency calls). The SIM also stores various user information and settings.
 *
 *  @param refNum: 	IN: Library reference number obtained from SysLibLoad.
 *  @retval Sim Status
 **/
extern GSMSIMStatus PhnLibGetSIMStatus(UInt16 refNum)
			  PHN_LIB_TRAP(PhnLibTrapGetSIMStatus);

/**
 *  @brief Check if we are currently attached to the GPRS Network.
 *
 *  @param refNum: 	IN: Library reference number obtained from SysLibLoad.
 *  @retval True if we are attached
 **/
extern Boolean PhnLibGPRSAttached (UInt16 refNum)
				  PHN_LIB_TRAP(PhnLibTrapGPRSAttached);

/**
 *  @brief This function will store the given settings as a profile in the radios flash
 *         memory. The profile id will be the same as the context id that you pass 
 *         with valid values between 1 and 32.
 *
 *  @param refNum: 	IN:  Library reference number obtained from SysLibLoad.
 *  @param contextId:	IN:
 *  @param pdpType:	IN:
 *  @param *apn:	IN:
 *  @param apnLen:	IN:
 *  @param *pdpAddr:	IN:
 *  @param pdpAddrLen:	IN:
 *  @param pdpDataCompression:	IN:
 *  @param pdpHdrCompression:	IN:
 *  @retval 0 for success; otherwise failed and returned an error code 
 *          as defined in “Phone Library Error Codes.”
 **/
extern Err PhnLibGPRSPDPContextDefine(UInt16 refNum, 
  UInt16 contextId, 
  PhnGPRSPDPType pdpType, const char* apn, UInt16 apnLen, 
  const char* pdpAddr, UInt16 pdpAddrLen, 
  UInt16 pdpDataCompression, UInt16 pdpHdrCompression)
			  PHN_LIB_TRAP(PhnLibTrapGPRSPDPContextDefine);

/**
 *  @brief Get a linked list of pdp contexts currently stored on the device. It is 
 *         necessary to call the Destruct after use of the list to free the memory.
 *
 *  @param refNum: 	IN:  Library reference number obtained from SysLibLoad.
 *  @param listHeadPP:	IN:  Pointer to a pointer where the head of the linked list will be stored.
 *  @retval 0 for success; otherwise failed and returned an error code 
 *          as defined in “Phone Library Error Codes.”
 **/
extern Err PhnLibGPRSPDPContextListConstruct(UInt16 refNum, struct PhnGPRSPDPContextListNodeType** listHeadPP)
			  PHN_LIB_TRAP(PhnLibTrapGPRSPDPContextListConstruct);

/**
 *  @brief Function that cleans up the list of pdp contexts created by the construct functions.
 *
 *  @param refNum: 	IN:  Library reference number obtained from SysLibLoad.
 *  @param headNodePP:	IN:  Pointer to a pointer to the head of the list.
 *  @retval 0 for success; otherwise failed and returned an error code 
 *          as defined in “Phone Library Error Codes.”
 **/
extern Err PhnLibGPRSPDPContextListDestruct(UInt16 refNum, struct PhnGPRSPDPContextListNodeType** headNodePP)
			  PHN_LIB_TRAP(PhnLibTrapGPRSPDPContextListDestruct);

/**
 *  @brief Set either the Required or Minimum QOS service parameters. If 0 is passed for the
 *         parameter, the network will decide the settings. Valid settings are listed below
 *
 *  @param refNum: 	IN:  Library reference number obtained from SysLibLoad.
 *  @param qosType:	IN:  Required or Minimum.
 *  @param contextId:	IN:  id of the profile that you are setting with these QOS Values.
 *  @param precedence:	IN:  (1-3) 1 being highest priority
 *  @param delay:	IN:  (1-3) Predictive 4 - Best Effort
 *  @param reliability:	IN:  (1-5) - Unknown
 *  @param peakThroughput:	IN:  (1-9) 1 is the lowest and 9 the highest
 *  @param meanThroughput:	IN:  (1-9) 1 is the lowest and 9 the highest
 *  @retval 0 for success; otherwise failed and returned an error code 
 *          as defined in “Phone Library Error Codes.”
 **/
extern Err PhnLibGPRSQualityOfServiceGet(UInt16 refNum, PhnGPRSQOSType qosType, UInt16 contextId, 
  UInt16* precedence, UInt16* delay, UInt16* reliability, UInt16* peakThroughput,
  UInt16* meanThroughput)
			  PHN_LIB_TRAP(PhnLibTrapGPRSQualityOfServiceGet);

/**
 *  @brief Set either the Required or Minimum QOS service parameters. If 0 is passed for the
 *         parameter, the network will decide the settings. Valid settings are listed below.
 *
 *  @param refNum: 	IN:  Library reference number obtained from SysLibLoad.
 *  @param qosType: 	IN:  Required or Minimum.
 *  @param  contextId:	IN:  id of the profile that you are setting with these QOS Values.
 *  @param  precedence:	IN:  (1-3) 1 being highest priority
 *  @param  delay:	IN:  (1-3) Predictive 4 - Best Effort
 *  @param  reliability:	IN:  (1-5) - Unknown
 *  @param  peakThroughput:	IN:  (1-9) 1 is the lowest and 9 the highest
 *  @param  meanThroughput:	IN:  (1-9) 1 is the lowest and 9 the highest
 *  @retval 0 for success; otherwise failed and returned an error code 
 *          as defined in “Phone Library Error Codes.”
 **/
extern Err PhnLibGPRSQualityOfServiceSet(UInt16 refNum, PhnGPRSQOSType qosType, UInt16 contextId, 
  UInt16 precedence, UInt16 delay, UInt16 reliability, UInt16 peakThroughput,
  UInt16 meanThroughput)
				PHN_LIB_TRAP(PhnLibTrapGPRSQualityOfServiceSet);

/**
 *  @brief
 *
 *  @param refNum: 	IN:
 *  @param digit:	IN:
 *  @param sendMode:	IN:
 *  @param on:		IN:
 *  @retval Err Error code.
 **/
extern Err PhnLibSendLongDTMF(UInt refNum, Char digit, PhnAudioSendMode sendMode, 
    Boolean on)
		  PHN_LIB_TRAP(PhnLibTrapSendLongDTMF);

/**
 *  @brief
 *
 *  @param refNum: 	IN:
 *  @param iccid:	IN:
 *  @retval Err Error code.
 **/
extern Err PhnLibGetICCID (UInt refNum, CharPtr* iccid)
			  PHN_LIB_TRAP(PhnLibTrapGetICCID);

/**
 *  @brief
 *
 *  @param refNum: 	IN: Library reference number obtained from SysLibLoad
 *  @param spn:	    IN: Service Provider Name from SIM
 *  @retval Err Error code.
 **/
extern Err PhnLibGetSpn (UInt refNum, CharPtr* spn)
			  PHN_LIB_TRAP(PhnLibTrapGetSpn);
#endif
