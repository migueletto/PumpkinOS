/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/** @ingroup Telephony
 *
 */
 
 
/**
 * @file 	HsPhoneSMS.h
 *
 * @brief  Header File for Phone Library API ---- SMS CATEGORY
 *
 * Notes:
 * 	All implementations of the Handspring Phone Library support a common API. 
 * 	This API is broken up into various categories for easier management.  This file
 * 	defines the SMS category.  These API calls are used to send/receive Short Message 
 * 	Service (SMS) messages.							
 */

#ifndef HS_PHONESMS_H
#define HS_PHONESMS_H
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
 *  @brief 
 *
 *  @param refNum:	IN: 
 *  @param number: 	IN:
 *  @param id: 		IN:
 *  @retval PhnAddressHandle   
 **/
  extern PhnAddressHandle PhnLibNewAddress (UInt16 refNum, const CharPtr number, PhnDatabaseID id)
				  PHN_LIB_TRAP(PhnLibTrapNewAddress);

/**
 *  @brief This function returns the field’s value for a given address in a newly allocated
 *         block. This function returns 0 if there was an error while retrieving the data.
 *
 *  @param refNum:  	IN:
 *  @param address:	IN:
 *  @param fied:	IN:
 *  @retval CharPtr
 **/
  extern CharPtr PhnLibGetField (UInt16 refNum, PhnAddressHandle address, PhnAddressField field)
				  PHN_LIB_TRAP(PhnLibTrapGetField);

/**
 *  @brief 
 *
 *  @param refNum: 	IN: 
 *  @param address:	IN:
 *  @retval PhnDatabaseID    
 **/  
  extern PhnDatabaseID PhnLibGetID(UInt16 refNum, PhnAddressHandle address)
				PHN_LIB_TRAP(PhnLibTrapGetID);

/**
 *  @brief This function lets the specified field of address to the given data. This function
 *         returns 0 if the field was modified without an error.
 *
 *  @param refNum:  	IN:
 *  @param address:	IN:
 *  @param field:	IN:
 *  @param data:	IN:
 *  @retval Err Error Code.  
 **/			
  extern Err PhnLibSetField(UInt16 refNum, PhnAddressHandle address, PhnAddressField field, const char* data)
				PHN_LIB_TRAP(PhnLibTrapSetField);

/**
 *  @brief 
 *
 *  @param refNum:  	IN:
 *  @param address:	IN:
 *  @param number:	IN:
 *  @retval Err Error Code.   
 **/
  extern Err PhnLibSetNumber(UInt16 refNum, PhnAddressHandle address, const char* number)
				PHN_LIB_TRAP(PhnLibTrapSetNumber);

/**
 *  @brief 
 *
 *  @param refNum:  	IN:
 *  @param address:	IN:
 *  @param id:		IN:
 *  @retval Err Error code.    
 **/ 
  extern Err PhnLibSetID(UInt16 refNum, PhnAddressHandle address, PhnDatabaseID id)
				PHN_LIB_TRAP(PhnLibTrapSetID);

/**
 *  @brief 
 *
 *  @param refNum:  	IN:
 *  @param address:	IN:
 *  @param format:	IN:
 *  @retval Char    
 **/
  extern char* PhnLibAddressToText(UInt16 refNum, PhnAddressHandle address ,SMSAddressFormat format)
			  PHN_LIB_TRAP(PhnLibTrapAddressToText);

/**
 *  @brief 
 *
 *  @param refNum:  	IN:
 *  @param address:	IN:
 *  @retval Boolean   
 **/  
  extern Boolean PhnLibHasName(UInt16 refNum, PhnAddressHandle address)
				PHN_LIB_TRAP(PhnLibTrapHasName);

/**
 *  @brief 
 *
 *  @param refNum:  	IN:
 *  @param a:		IN:
 *  @param b:		IN:
 *  @retval Boolean    
 **/ 
  extern Boolean PhnLibEqual(UInt16 refNum, PhnAddressHandle a, PhnAddressHandle b)
				PHN_LIB_TRAP(PhnLibTrapEqual);

/**
 *  @brief 
 *
 *  @param refNum:  	IN:
 *  @retval DmOpenRef    
 **/
  extern DmOpenRef PhnLibGetDBRef(UInt16 refNum)
				PHN_LIB_TRAP(PhnLibTrapGetDBRef);

/**
 *  @brief 
 *
 *  @param refNum:  	IN:
 *  @param db:		IN:
 *  @retval Err Error code.    
 **/  
  extern Err PhnLibReleaseDBRef(UInt16 refNum, DmOpenRef db)
				PHN_LIB_TRAP(PhnLibTrapReleaseDBRef);

/**
 *  @brief Create a new message of the given message type and return the ID of the new
 *         record in the message database. All fields are either empty (e.g. message’s text) or
 *         contain default values (e.g. sending options).
 *
 *  @param refNum:  	IN:
 *  @param type:	IN:  Incoming or Outcoming message. See SMSMessageType for more details.
 *  @retval Returns either the ID of the new message record or phnErrUnknownID if there was
 *          not sufficient memory to allocate a new message.
 **/
  extern PhnDatabaseID PhnLibNewMessage(UInt16 refNum, SMSMessageType type)
				PHN_LIB_TRAP(PhnLibTrapNewMessage);

/**
 *  @brief This function deletes the message with the given msgID from the message database.
 *
 *  @param refNum:  	IN:
 *  @param msgID:	IN:
 *  @param archive:	IN:
 *  @retval This function returns 0 if there was no error during the deletion of the record. If
 *          there is no message with the given id, this function returns phnErrUnknownID.   
 **/   
  extern Err PhnLibDeleteMessage(UInt16 refNum, PhnDatabaseID msgID, Boolean archive)
				PHN_LIB_TRAP(PhnLibTrapDeleteMessage);

/**
 *  @brief This function sends the message to all the recipients.
 *
 *  @param refNum:  	IN:
 *  @param msgID:	IN:
 *  @param progress:	IN:
 *  @retval phnErrUnknownID: If there is no message with the given ID.
 *          phnErrIllegalStatus: If the message’s status is not kNone or kPending
 *          phnErrNotAllowed: If the error flag is set. It is not permitted to send messages
 *             when the error flag is set.    
 **/  
  extern Err PhnLibSendMessage(UInt16 refNum, PhnDatabaseID msgID, Boolean progress)
				PHN_LIB_TRAP(PhnLibTrapSendMessage);

/**
 *  @brief 
 *
 *  @param refNum:  	IN:
 *  @retval Err Error code.    
 **/   
  extern Err PhnLibSendPendingMessages(UInt16 refNum)
				PHN_LIB_TRAP(PhnLibTrapSendPendingMessages);

/**
 *  @brief This function sets the text of the message to the given text.
 *
 *  @param refNum:  	IN:
 *  @param msgID:	IN:
 *  @param data:	IN:
 *  @param size:	IN:
 *  @retval 0: if the text was updated successfully.
 *          phnErrIllegalChars: If the text contains illegal characters    
 **/  
  extern Err PhnLibSetText(UInt16 refNum, PhnDatabaseID msgID, const char* data, Int16 size)
				PHN_LIB_TRAP(PhnLibTrapSetText);

/**
 *  @brief This function sets the date of the given message. The date must be given in Palm
 *         OS format (as returned by TimGetSeconds).
 *
 *  @param refNum:  	IN:
 *  @param msgID:	IN:
 *  @param date:	IN:
 *  @retval Err Error code.    
 **/   
  extern Err PhnLibSetDate(UInt16 refNum, PhnDatabaseID msgID, UInt32 date)
				PHN_LIB_TRAP(PhnLibTrapSetDate);

/**
 *  @brief This function updates the sending options for the given message.
 *
 *  @param refNum:  	IN:
 *  @param msgID:	IN:
 *  @param options:	IN:
 *  @retval Err Error code.    
 **/  
  extern Err PhnLibSetOptions(UInt16 refNum, PhnDatabaseID msgID, const SMSSendOptions* options)
				PHN_LIB_TRAP(PhnLibTrapSetOptions);

/**
 *  @brief This function updates the list of addresses with the given list of addresses.
 *
 *  @param refNum:  	IN:
 *  @param msgID:	IN:
 *  @param list:	IN:
 *  @retval Err Error code.    
 **/  
  extern Err PhnLibSetAddresses(UInt16 refNum, PhnDatabaseID msgID, const PhnAddressList list)
				PHN_LIB_TRAP(PhnLibTrapSetAddresses);

/**
 *  @brief This function sets the status of the given message.
 *
 *  @param refNum:  	IN:
 *  @param msgID:	IN:
 *  @param status:	IN:
 *  @retval Err Error code.    
 **/    
  extern Err PhnLibSetStatus(UInt16 refNum, PhnDatabaseID msgID, SMSMessageStatus status)
				PHN_LIB_TRAP(PhnLibTrapSetStatus);

/**
 *  @brief This function updates a message’s flags.
 *
 *  @param refNum:  	IN:
 *  @param msgID:	IN:
 *  @param flags:	IN:
 *  @retval Err Error code.    
 **/   
  extern Err PhnLibSetFlags(UInt16 refNum, PhnDatabaseID msgID, UInt32 flags)
				PHN_LIB_TRAP(PhnLibTrapSetFlags);

/**
 *  @brief This function updates the given message’s owner.
 *
 *  @param refNum:  	IN:
 *  @param msgID:	IN:
 *  @param owner:	IN
 *  @retval Err Error code.    
 **/    
  extern Err PhnLibSetOwner(UInt16 refNum, PhnDatabaseID msgID, UInt32 owner)
				PHN_LIB_TRAP(PhnLibTrapSetOwner);

/**
 *  @brief This function copies the text of the message into a new block of memory that must
 *         be disposed of by the caller.
 *
 *  @param refNum:  	IN:
 *  @param msgID:	IN:
 *  @param data:	IN:
 *  @retval Err Error code.    
 **/ 
  extern Err PhnLibGetText(UInt16 refNum, PhnDatabaseID msgID, MemHandle* data)
				PHN_LIB_TRAP(PhnLibTrapGetText);

/**
 *  @brief This function returns the date of the given message in Palm OS format.
 *
 *  @param refNum:  	IN:
 *  @param msgID:	IN:
 *  @param date:	IN:
 *  @retval Err Error code.    
 **/ 
  extern Err PhnLibGetDate(UInt16 refNum, PhnDatabaseID msgID, UInt32* date)
				PHN_LIB_TRAP(PhnLibTrapGetDate);

/**
 *  @brief This function returns the sending options for the given message.
 *
 *  @param refNum:  	IN:
 *  @param msgID:	IN:
 *  @param options:	IN:
 *  @retval Err Error code.    
 **/   
  extern Err PhnLibGetOptions(UInt16 refNum, PhnDatabaseID msgID, SMSSendOptions* options)
				PHN_LIB_TRAP(PhnLibTrapGetOptions);

/**
 *  @brief This function returns the list of addresses for the given message.
 *
 *  @param refNum:  	IN:
 *  @param msgID:	IN:
 *  @param list:	IN:
 *  @retval Err Error code.    
 **/  
  extern Err PhnLibGetAddresses(UInt16 refNum, PhnDatabaseID msgID, PhnAddressList* list)
				PHN_LIB_TRAP(PhnLibTrapGetAddresses);

/**
 *  @brief This function returns the status field of the message.
 *
 *  @param refNum:  	IN:
 *  @param msgID:	IN:
 *  @param status:	IN:
 *  @retval Err Error code.    
 **/   
  extern Err PhnLibGetStatus(UInt16 refNum, PhnDatabaseID msgID, SMSMessageStatus* status)
				PHN_LIB_TRAP(PhnLibTrapGetStatus);

/**
 *  @brief This function returns the message’s flags.
 *
 *  @param refNum:  	IN:  Library reference number returned by HsGetPhoneLibrary().
 *  @param msgID:	IN:
 *  @param flags:	IN:
 *  @retval Err Error code.    
 **/   
  extern Err PhnLibGetFlags(UInt16 refNum, PhnDatabaseID msgID, UInt32* flags)
				PHN_LIB_TRAP(PhnLibTrapGetFlags);

/**
 *  @brief This function returns the message’s owner.
 *
 *  @param refNum:  	IN:  Library reference number returned by HsGetPhoneLibrary().
 *  @param msgID:	IN:
 *  @param owner:	IN:
 *  @retval Error code.    
 **/   
  extern Err PhnLibGetOwner(UInt16 refNum, PhnDatabaseID msgID, UInt32* owner)
				PHN_LIB_TRAP(PhnLibTrapGetOwner);

/**
 *  @brief This function returns the message’s type. A client application typically uses the
 *         type to map messages to categories.
 *
 *  @param refNum:  	IN:
 *  @param msgID:	IN:
 *  @param type:	IN:
 *  @retval Err Error code.    
 **/   
  extern Err PhnLibGetType(UInt16 refNum, PhnDatabaseID msgID, SMSMessageType* type)
				PHN_LIB_TRAP(PhnLibTrapGetType);

/**
 *  @brief This function returns true if the given Palm OS character is a legal GSM character.
 *         If this function returns false, the given character does not exist within 
 *         the GSM alphabet.
 *
 *  @param refNum:  	IN:
 *  @param c:		IN:
 *  @retval Boolean   
 **/   
  extern Boolean PhnLibIsLegalCharacter(UInt16 refNum, char c)
				PHN_LIB_TRAP(PhnLibTrapIsLegalCharacter);

/**
 *  @brief This function maps the given Palm OS character to its equivalent 
 *         in the GSM alphabet.
 *
 *  @param refNum:  	IN:
 *  @param c:		IN:
 *  @retval char    
 **/   
  extern char PhnLibMapCharacter(UInt16 refNum, char c)
				PHN_LIB_TRAP(PhnLibTrapMapCharacter);

/**
 *  @brief This function sets the number of the service center to be used 
 *         for sending the SMS messages.
 *
 *  @param refNum:  	IN:
 *  @param address:	IN:
 *  @retval Err Error code.    
 **/   
  extern Err PhnLibSetServiceCentreAddress(UInt16 refNum, const PhnAddressHandle address)
				PHN_LIB_TRAP(PhnLibTrapSetServiceCentreAddress);

/**
 *  @brief This function returns the number of the service center currently used 
 *         for sending messages in a newly allocated address.
 *
 *  @param refNum:  	IN:
 *  @param address:	IN:
 *  @retval Error code.    
 **/   
  extern Err PhnLibGetServiceCentreAddress(UInt16 refNum, PhnAddressHandle* address)
				PHN_LIB_TRAP(PhnLibTrapGetServiceCentreAddress);

/**
 *  @brief This function returns the length of the given text in characters or messages.
 *
 *  @param refNum:  	IN:
 *  @param text:	IN:
 *  @param inMessages:	IN:
 *  @param substitution:	IN:
 *  @retval Int16   
 **/   
  extern Int16 PhnLibLength(UInt16 refNum, const char* text, Boolean inMessages, Boolean substitution)
				PHN_LIB_TRAP(PhnLibTrapLength);

/**
 *  @brief 
 *
 *  @param refNum:  	IN:
 *  @param text:	IN:
 *  @param address:	IN:
 *  @param info:	IN:
 *  @retval Err Error code.    
 **/   
  extern Err PhnLibGetLengthDetails(UInt16 refNum, const char* text, const char* address, PhnLibLengthInfoType* info)
        PHN_LIB_TRAP(PhnLibTrapGetLengthDetails);

/**
 *  @brief This function returns the substitution string for a given GSM character.
 *
 *  @param refNum:  	IN:
 *  @param c:		IN:
 *  @retval const 
 **/   
  extern const char* PhnLibGetSubstitution(UInt16 refNum, char c)
				PHN_LIB_TRAP(PhnLibTrapGetSubstitution);

/**
 *  @brief This function returns a new address list. The list is initially empty 
 *         and must be disposed of by the caller using PhnLibDisposeAddressList() 
 *         or MemHandleFree().
 *
 *  @param refNum:  	IN:
 *  @retval PhnAddressList    
 **/   
  extern PhnAddressList PhnLibNewAddressList(UInt16 refNum)
				PHN_LIB_TRAP(PhnLibTrapNewAddressList);

/**
 *  @brief This function disposes of the memory used by the given address list. 
 *         It should be called after your done with PhnLibNewAddressList().
 *
 *  @param refNum:  	IN:
 *  @param list:	IN:
 *  @retval Err Error code.    
 **/   
  extern Err PhnLibDisposeAddressList(UInt16 refNum, PhnAddressList list)
				PHN_LIB_TRAP(PhnLibTrapDisposeAddressList);
  
/**
 *  @brief This function adds a copy of address to the end of list. 
 *         The address is not disposed of.
 *
 *  @param refNum:  	IN:
 *  @param list:	IN:
 *  @param address:	IN:
 *  @retval Err Error code.    
 **/   
  extern Err PhnLibAddAddress(UInt16 refNum, PhnAddressList list, const PhnAddressHandle address)
				PHN_LIB_TRAP(PhnLibTrapAddAddress);
  
/**
 *  @brief This function retrieves an item with the given index from list and 
 *         returns it as a new block on the heap specified by address.
 *
 *  @param refNum:  	IN:
 *  @param list:	IN:
 *  @param index:	IN:
 *  @param address:	IN:
 *  @retval Err Error code.    
 **/   
  extern Err PhnLibGetNth(UInt16 refNum, const PhnAddressList list, Int16 index, PhnAddressHandle* address)
				PHN_LIB_TRAP(PhnLibTrapGetNth);
  
/**
 *  @brief This function replaces the data of the item with the given index from 
 *         list by the given address.
 *
 *  @param refNum:  	IN:
 *  @param list:	IN:
 *  @param index:	IN:
 *  @param address:	IN:
 *  @retval Err Error code.    
 **/   
  extern Err PhnLibSetNth(UInt16 refNum, PhnAddressList list, Int16 index, const PhnAddressHandle address)
				PHN_LIB_TRAP(PhnLibTrapSetNth);

/**
 *  @brief This function returns the length of the given list. If list is 0, 
 *         this function returns phnErrParam.
 *
 *  @param refNum:  	IN:
 *  @param list:	IN:
 *  @param count:	IN:
 *  @retval Err Error code.    
 **/   
  extern Err PhnLibCount(UInt16 refNum, PhnAddressList list, UInt16* count)
				PHN_LIB_TRAP(PhnLibTrapCount);

/**
 *  @brief 
 *
 *  @param refNum:  	IN:
 *  @param info:	IN:
 *  @retval Err Error code.    
 **/   
  extern Err	PhnLibGetSMSRingInfo (UInt16 refNum, PhnRingingInfoPtr info)
				PHN_LIB_TRAP(PhnLibTrapGetSMSRingInfo);

/**
 *  @brief 
 *
 *  @param refNum:  	IN:
 *  @param info:	IN:
 *  @retval Err Error code.    
 **/    
  extern Err	PhnLibSetSMSRingInfo (UInt16 refNum, PhnRingingInfoPtr info)
					PHN_LIB_TRAP(PhnLibTrapSetSMSRingInfo);

/**
 *  @brief 
 *
 *  @param refNum:  	IN:
 *  @param smsGateway:	IN:
 *  @retval Err Error code.    
 **/   
  extern Err	PhnLibGetSMSGateway (UInt16 refNum, char** smsGateway)
					PHN_LIB_TRAP(PhnLibTrapGetSMSGateway);

/**
 *  @brief 
 *
 *  @param refNum:  	IN:
 *  @param prefP:	IN:
 *  @retval Err Error code.    
 **/   
  extern Err      PhnLibGetSMSPreference (UInt16 refNum, PhnSMSPrefPtr prefP)
                  PHN_LIB_TRAP (PhnLibTrapGetSMSPreference);

/**
 *  @brief 
 *
 *  @param refNum:  	IN:
 *  @param prefP:	IN:
 *  @retval Err Error code.    
 **/   
  extern Err      PhnLibSetSMSPreference (UInt16 refNum, PhnSMSPrefPtr prefP)
                  PHN_LIB_TRAP (PhnLibTrapSetSMSPreference);


/**
 *  @brief 
 *
 *  @param refNum:  	IN:
 *  @param list:	IN:
 *  @param index:	IN:
 *  @retval Err Error code.    
 **/   
  extern Err      PhnLibAPGetNth (UInt16 refNum, PhnAddressList list, Int16 index,
                                PhnAddressHandle * address)
                  PHN_LIB_TRAP (PhnLibTrapAPGetNth);

/**
 *  @brief 
 *
 *  @param refNum:  	IN:
 *  @param transactionID:	IN:
 *  @param acK:		IN:
 *  @retval Err Error code.    
 **/   
  extern Err      PhnLibSendSMSMTAck (UInt16 refNum, UInt8 transactionId,
                                      Boolean ack)
                  PHN_LIB_TRAP (PhnLibTrapSendSMSMTAck);

/**
 *  @brief 
 *
 *  @param refNum:  	IN:
 *  @retval Err Error code.    
 **/   
  extern Err      PhnLibQuerySMSMT (UInt16 refNum)
                  PHN_LIB_TRAP (PhnLibTrapQuerySMSMT);
  
/**
 *  @brief 
 *
 *  @param refNum:  	IN:
 *  @param number:	IN:
 *  @param msgID:	IN:
 *  @retval Err Error code.    
 **/   
  extern Err PhnLibSetSMSCallbackNumber (UInt16 refNum, const CharPtr number, PhnDatabaseID msgID)
                  PHN_LIB_TRAP (PhnLibTrapSetSMSCallbackNumber);

#endif
