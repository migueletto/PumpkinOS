/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/** @ingroup Telephony
 *
 */
 
 
/**	
 *
 * @file HsPhoneCallControlTypes.h
 *
 * @brief  Types for Phone Library call control functionality
 *
 * 					
 */


#ifndef HS_PHONE_CALL_CONTROL_TYPES__H
#define HS_PHONE_CALL_CONTROL_TYPES__H
/**
 * Phone connection state
 **/
typedef enum
  {
    phnConnectionActive,		/**<		*/
    phnConnectionHeld,			/**<		*/
    phnConnectionDialing,		/**<		*/
    phnConnectionAlerting,		/**<		*/
    phnConnectionIncoming,		/**<		*/
    phnConnectionWaiting,		/**<		*/
    phnConnectionUnknown,		/**<		*/
	phnConnectionDormant
  }
PhnConnectStateType;

#define isValidPhnConnectStateType(c) ((c >= phnConnectionActive) && (c <= phnConnectionDormant))

/**< meaning of feature bits in a GSMSATEventMenuRec */



/**
 * Phone connection info
 **/
typedef struct
  {
    PhnConnectionID id;			/**<		*/
    PhnConnectStateType state;		/**<		*/
    PhoneServiceClassType service;	/**<		*/
    Boolean         incoming;		/**<		*/
    Boolean         multiparty;		/**<		*/
    PhnAddressHandle address;		/**<		*/
    DWord           owner;		/**<		*/
    Int16           lineNumber;		/**<		*/
  }
PhnConnectionType,* PhnConnectionPtr;
#endif

