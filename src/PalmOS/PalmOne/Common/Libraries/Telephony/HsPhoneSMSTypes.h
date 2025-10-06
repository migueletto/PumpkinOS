/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/** @ingroup Telephony
 *
 */
 
 
/**	
 *
 * @file 	HsPhoneSMSTypes.h
 *
 * @brief  Header File for Phone Library API ---- NETWORK CATEGORY
 *
 * NOTES:
 * 	All implementations of the Handspring Phone Library support a common API. 
 * 	This API is broken up into various categories for easier management.  This file
 * 	defines the SMS category.  These API calls are used to interact with the wireless network.				
 */



#ifndef _HS_PHONE_SMS_TYPES_H__
#define _HS_PHONE_SMS_TYPES_H__
#include <PalmOS.h>
#include <PalmTypes.h>
#ifndef __CORE_COMPATIBILITY_H__
#include <PalmCompatibility.h>
	/** workaround for differing header files in sdk-3.5 and sdk-internal */
	#ifndef __CORE_COMPATIBILITY_H__
	#define __CORE_COMPATIBILITY_H__
	#endif 
#endif 

#if defined (_MSC_VER) 	/**< Win 32 compiler */
#pragma pack ( push, 2 )
#endif


/**
 * Palm OS address book functions
 **/
enum AddressAppRecordFieldEnum {
	kName, 			/**<		*/
	kFirstName, 		/**<		*/
	kCompany, 		/**<		*/
	kPhone1, 		/**<		*/
	kPhone2, 		/**<		*/
	kPhone3, 		/**<		*/
	kPhone4,		/**<		*/
	kPhone5, 		/**<		*/
	kAddress, 		/**<		*/
	kCity, 			/**<		*/
	kState, 		/**<		*/
	kZipCode, 		/**<		*/
	kCountry, 		/**<		*/
	kTitle,			/**<		*/
	kCustom1, 		/**<		*/
	kCustom2, 		/**<		*/
	kCustom3, 		/**<		*/
	kCustom4, 		/**<		*/
	kNote,			/**<		*/
	kRecordFieldEnumCount /**< must be the last field */
};

#ifndef __cplusplus
typedef enum AddressAppRecordFieldEnum AddressAppRecordFieldEnum;
#endif


#define isValidAddressAppRecordFieldEnum(x) ((x >= kName) && (x < kRecordFieldEnumCount)) 	/**<		*/

/**
 *  Item description here
 **/
typedef enum  {
	addrWorkLabel, 		/**<		*/
	addrHomeLabel, 		/**<		*/
	addrFaxLabel, 		/**<		*/
	addrOtherLabel, 	/**<		*/
	addrEmailLabel, 	/**<		*/
	addrMainLabel,		/**<		*/
	addrPagerLabel, 	/**<		*/
	addrMobileLabel, 	/**<		*/
	addrNoLabel		/**<		*/
}AddressAppPhoneLabel;

typedef UInt8 AddressAppPhoneLabelType;		/**<		*/

/**
 * This should probably be private to the library
 **/
struct Address {
  // All fields should be considered read-only.
  AddressAppPhoneLabelType kind;	/**<		*/
  char filler;				/**<		*/
  UInt32 uniqueID;			/**<		*/
  UInt16 phoneBookIndex;		/**<		*/
  UInt16 count[3];			/**<		*/
  // immediately followed by address data
};


/**
 * Item description here
 **/
struct AddrDescriptor {
  UInt16 count; 	/**< Length of list */
	// immediately followed by count pairs of (UInt16,Address)
};

#ifndef _cplusplus
typedef struct Address Address;				/**<		*/
typedef struct AddrDescriptor AddrDescriptor;		/**<		*/
#endif


/**
 * Item description here
 **/
struct AddressType {
  UInt16 addressSize;		/**<		*/
  Address address;		/**<		*/
};

#ifndef _cplusplus
typedef struct AddressType AddressType;		/**<		*/
#endif

/**
 *  Item description here
 **/
struct AddressAppRecordHeader {
	UInt32 options;		/**<		*/
	UInt32 flags;		/**<		*/
	UInt8 companyOffset;	/**<		*/
	char data;		/**<		*/
};

#ifndef _cplusplus
typedef struct AddressAppRecordHeader AddressAppRecordHeader;	/**<		*/
#endif

/**
 *  Item description here
 **/
struct AddressAppRecord {
// private:
	// Fields within an address.
	
	// Header of an address. This is immediately followed by the address
	// data. The first database is part of the following structure.
	// (See AddressDB.h for a detailled explanation of the structure.)

	MemHandle mData;	/**<		*/
	UInt32 mID;		/**<		*/
	Boolean mLocked;	/**<		*/
	Int8  reserved1; 	/**< padding */
	AddressAppRecordHeader* mHeader;	/**<		*/
};

/**
 *  NBS defines
 **/

/**
 *  @name Identifier elements definitions 
 **/
/*@{*/
#define NBSHeaderID            0x00  /**< NBS fragment header information identifier */
#define NBSHeaderIDLen         0x03  /**< three bytes */

#define Addressing8bitID       0x04  /**< 8-bit address information identifier */
#define Addressing8bitIDLen    0x02  /**< 2 bytes... src port(1), dest port(1) */

#define Addressing16bitID      0x05  /**< 16-bit address information identifier */
#define Addressing16bitIDLen   0x04  /**< 4 bytes... src port(2), dest port(2) */
/*@}*/


/**
 *  @name Text Header elements
 **/
/*@{*/
#define NBSHeaderEscapeSeq     "//"      /**< start sequence of the header */
#define NBSTextHeader          "//SCK"   /**< beginning of an NBS text header */
#define NBSTextHeaderLength    5         /**< length of the header */
/*@}*/


/**
 *  @name 8bit addressing text header lengths, not including the //SCK
 **/
/*@{*/
#define Full8bitTextHeader     0x0A  /**< length for full header with 8-bit addressing    //SCKddoorrttnn */
#define Minimum8bitTextHeader  0x02  /**< length for minimum header with 8-bit addressing //SCKdd */
/*@}*/


/**
 *  @name 16bit addressing text header lengths, not including the //SCKL
 **/
/*@{*/
#define Full16bitTextHeader    0x0F  /**< length for full header with 16-bit addressing    //SCKLddddoooorrttnn */
#define Minimum16bitTextHeader 0x05  /**< length for minimum header with 16-bit addressing //SCKLdddd */
/*@}*/

#define TextHeaderTerminator   ' '   /**< text based headers terminate at the next space */

#define phnNBSEvent         'Hnbs'


/**
 *  @name NBS Port Numbers (Nokia Smart Messaging Spec)
 **/
/*@{*/
#define NBSPort_MimeVCard     0xE2	/**<		*/
#define NBSPort_MimeVCalendar 0xE4	/**<		*/
/*@}*/

/**
 *  SMS stuff
 **/
typedef enum {
  kMTIncoming, kMTOutgoing		/**<		*/
} SMSMessageType;

#define isValidSMSMessageType(m) ((m >= kMTIncoming) && (m <= kMTOutgoing))		/**<		*/

/**
 *  Item description here
 **/
typedef enum  {
  kNone,			/**<		*/
  kReceiving, kReceived,	/**<		*/
  kPending, kSending, kSent	/**<		*/
}SMSMessageStatus;

#define isValidSMSMessageStatus(m) ((m >= kNone) && (m <= kSent))		/**<		*/

/**
 * CDMA SMS Preference ?
 **/
typedef struct
  {
    Boolean         audioAlertEnable;	/**<		*/
    Boolean         confirmDeletion;	/**<		*/
  }
PhnSMSPrefType ,* PhnSMSPrefPtr;



/**
 *  
 **/
typedef enum  {
  kGreekSymbols = 1L << 0,		/**<		*/
  kMissingPart =	1L << 1,	/**<		*/
  kAutoDelete =	1L << 2,		/**<		*/
  kNotification = 1L << 3,		/**<		*/
  kDontEncode =	1L << 4,		/**<		*/
  kSubstitution = 1L << 5,		/**<		*/
  kFailed =		1L << 6,	/**<		*/
  kStatusReport = 1L << 7,		/**<		*/
  kFreeReply =	1L << 8,		/**<		*/
  kInternetEMail =1L << 9,		/**<		*/
  kTextSegments =	1L << 10,	/**<		*/
  kSMSErrorType1 = 1L << 11,		/**<		*/
  kSMSErrorType2 = 1L << 12,		/**<		*/
  kSMSErrorType3 = 1L << 13,		/**<		*/
  kSMSHighPriority = 1L << 14,		/**<		*/
  kSMSLowPriority = 1L << 15,		/**<		*/
		// application-level flags
  kRead = 1L << 16,			/**<		*/
  kDeferredDelivery = 1L << 17,		/**<		*/
  kWAPMsg = 1L << 18			/**<		*/
}SMSMessageFlags;

/**
 *  
 **/
typedef enum  {
  smsFmtIgnoreKind = 0,		/**<		*/
  smsFmtMatchKind = 8,		/**<		*/
  smsFmtExceptKind = 16,	/**<		*/
  smsFmtAllKinds = 24,		/**<		*/
  smsFmtLastNameFirst = 32,	/**<		*/
  smsFmtShortLabel = 64		/**<		*/
}SMSAddressFormat;

/**
 *  
 **/
typedef struct  {
  Boolean freeReply;		/**<		*/
  Boolean statusReport;		/**<		*/
  unsigned char validity;	/**<		*/
}SMSSendOptions;		

/**
 *  
 **/
typedef struct  {
  UInt32 owner;			/**< application owning this message */
  SMSMessageType type;		/**< message type */
  SMSMessageStatus status;	/**< message status */
  UInt32 date;			/**< date of sending or receipt */
  UInt32 flags;			/**< miscellaneous flags */
  UInt8 validity;		/**< validity period */
  UInt8 segments;		/**< number of segments (incoming message) */
  				/**< Size of field Addresses. If there is segmentation information for */
 		 		/**< a message this field contains also the sizes of the parts. The array */
  				/**< is therefore a variable-size array. */
  UInt16 size[1];		/**<		*/
}SmsHeader;


/**
 *  Structure passed to the callbacks registered for incoming NBS notifications
 **/
struct NBSNotificationEventType
{
	UInt16 version;  	/**< version number to provide future backwards compatibility */

	/** helper fields */
	Boolean NBSdatagram;  	/**< flag if it is an NBS datagram */
	Boolean binary;       	/**< true if binary data */

	void *headerP;   	/**< pointer to raw header */
	UInt8 headerLen;  	/**< length of headerP */
	Int8  reserved0; 	/**< padding */
	void *dataP;      	/**< pointer to data body */
	UInt8 dataLen;    	/**< length of dataP */

	/** NBS datagram fields */
	UInt8 refNum;    	/**< NBS reference number */
	UInt8 maxNum;    	/**< max segment number 1-255 */
	UInt8 seqNum;    	/**< sequence number    1-255, no more than maxNum */
	Int8  reserved1; 	/**< padding */
	Int8  reserved1a; 	/**< mo padding */

	UInt32 srcPort;  	/**< source port */
	UInt32 dstPort;  	/**< destination port */

	/** SMS related fields */
	UInt32 msgID; 		/**< ID into the SMS database to reference this */
                  		/**< message this ID is not gauranteed to be */
                 		/**< valid once the notification callback */
                  		/**< returns.  Users should make a copy of the */
                  		/**< msg if they want to work on it after the */
                  		/**< callback returns. */
                  

	char   *senderP;   	/**< sender number - null terminated */
	UInt32 datetime;   	/**< date/time stamp */
	Int32  reserved2;  	/**< reserved*/
	Int32  reserved3;  	/**< reserved*/
};

#ifndef _cplusplus
typedef struct NBSNotificationEventType NBSNotificationEventType;		/**<		*/
#endif

/**
 * NBS 
 **/
enum { kParseError = -1, kDoneParsing=0, kParseInformationID, kParseInformationLength, kParseInformationData };

/**
 *  SMS length information
 **/
typedef struct 
{
  UInt16 size;			/**<		*/
  Boolean  substitution;	/**<		*/
  UInt16	 length;	/**<		*/
  UInt16  segmentCount;  	/**<		*/
}PhnLibLengthInfoType;

#if defined (_MSC_VER)
#pragma pack ( pop )
#endif


#endif // _HS_PHONE_SMS_TYPES_H__
