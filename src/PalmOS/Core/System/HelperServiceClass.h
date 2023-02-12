/******************************************************************************
 *
 * Copyright (c) 1995-2003 PalmSource, Inc. All rights reserved.
 *
 * File: HelperServiceClass.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *	Public header file for the service class ID's and extended details
 *	data structures used with the "address book helper" API.
 *
 *	For each Service Class ID, this header file also defines the
 *	corresponding extended details data structures that may
 *	optionally be passed as the 'pDetails' element in the
 *	HelperNotifyExecuteType structure. We strongly recommend that
 *	every extra details data structure include a version number,
 *	which will alow the structure to be extended by adding new structure
 *	elements later.
 *
 *	The Service Class ID is a 32-bit value that uniquely identifies the
 *	class of service performed by the Helper -- for example, making a
 *	voice telephone call, sending an internet e-mail, sending an SMS
 *	message, sending a fax, etc.  Palm defines some common Service Class
 *	ID's and the corresponding extra details structures in this header file.
 *
 *	3rd party developers:
 *	If none of these service class ID's match the service performed by your
 *	helper, you must register a unique service class ID using the Creator ID
 *	registry on Palm's web site (or use a creator ID that you already own).
 *	A group of developers may elect to support the same service class ID for
 *	interoperability. 
 *
 *****************************************************************************/

#ifndef HELPERSERVICECLASS_H
#define HELPERSERVICECLASS_H

#include <PalmTypes.h>


//------------------------------------------------------------------------
// Current Helper Service Class ID's
//------------------------------------------------------------------------

//
// Helpers of this Service Class make a voice telephone call.
//
// The telephone number to dial is passed in the 'pData' element of the main
// structure (HelperNotifyExecuteType)
//
// The 'pDetails' struct member is NULL for this service class.
//
#define kHelperServiceClassIDVoiceDial	'voic'


//
// Helpers of this Service Class send an Internet mail message.
//
// "To" address(es) are passed in the 'pData' element of the main structure
// (HelperNotifyExecuteType)
//
// The 'pDetails' struct member may optionally point to
// HelperServiceEMailDetailsType for this service class.
//
#define kHelperServiceClassIDEMail		'mail'
typedef struct _HelperServiceEMailDetailsType
{
	UInt16	version;			  // this is version 1

	Char*	cc;					  // IN: carbon copy address string or NULL -- will
								  //  be duplicated by helper if necessary;
								  //  multiple addresses are separated by
								  //  semicolon (ex. "john@host.com; jane@host.com")
	Char*	subject;			  // IN: subject string or NULL -- will be duplicated
								  //  by helper if necessary (ex. "helper API")
	Char*	message;			  // IN: initial message body string or NULL -- will be
								  //  duplicated by helper if necessary (ex.
								  //  "Lets discuss the helper API tomorrow.")

} HelperServiceEMailDetailsType;


//
// Helpers of this Service Class send an SMS message.
//
// SMS mailbox number is passed in the 'pData' element of the main structure
// (HelperNotifyExecuteType).
//
// The 'pDetails' struct member may optionally point to
//  HelperServiceSMSDetailsType for this service class.
//
#define kHelperServiceClassIDSMS			'sms_'
typedef struct _HelperServiceSMSDetailsType
{
	UInt16	version;			  // this is version 1

	Char*	message;			  // IN: initial message body string or NULL -- will be
								  //  duplicated by helper if necessary (ex.
								  //  "Lets discuss the helper API tomorrow.")

} HelperServiceSMSDetailsType;


//
// Helpers of this Service Class send a fax.
//
// The fax number is passed in the 'pData' element of the main structure
// (HelperNotifyExecuteType).
//
// The 'pDetails' struct member is NULL for this service class.
//
#define kHelperServiceClassIDFax			'fax_'


#endif // HELPERSERVICECLASS_H
