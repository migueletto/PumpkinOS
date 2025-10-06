/******************************************************************************
 * Copyright (c) 2004 PalmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/

/**
 * @file 	MmsHelperCommon.h
 * @version 1.0
 * @date 	12/17/2002
 *
 * This is the Type and structure parameters file for the 
 * MMS Helper Functions.
 *
 * <hr>
 */
 
#ifndef HELPER_MMS_H
#define HELPER_MMS_H

//New Helper classes registered with Palm Source for sending by MMS

#define    kHelperServiceClassIDMMS    'mms_'

//Linked list of Objects to be sent
typedef struct _HelperServiceMMSObjectType 
{
	Int16	pageNum;			//IN: page of message this content should appear on.
	Char* 	mimeType;			//IN: IANA Mime type for this object (compulsory!)
	
								// 	  The object may be passed in a buffer or via a file.
								//	  Either set bufferP and bufferLen or set tempFileName 
								
	MemPtr	bufferP;			//IN: The object is stored in a buffer if this is non-NULL
	UInt32	bufferLen;			//IN: Length of the buffer
	
	Char* 	tempFileName;		//IN: The temporary file its stored in if bufferP is NULL
	Boolean isVFSFileName; 		//IN: Is the file local or under VFS e.g Memory card?
	Boolean freeTempFile;		//IN: If set, the file should be deleted by the helper application
	
	Char* 	name;				//IN: the name of the object as sent to us (maybe the filename)
	
	struct 	_HelperServiceMMSObjectType* nextP; 				//IN: Linked list pointer of objects to send.

} HelperServiceMMSObjectType;

//
// Helpers of this Service Class send an MMS message.
//
// "To" address(es) are passed in the 'pData' element of the main structure
// (HelperNotifyExecuteType)
//
// The 'pDetails' struct member may optionally point to
// HelperServiceMMSDetailsType for this service class.
//
typedef struct _HelperServiceMMSDetailsType
{
	UInt16	version;			  		// this is version 1

	Char*	cc;					  		// IN: carbon copy address string or NULL -- will
								  		//  be duplicated by helper if necessary;
								  		//  multiple addresses are separated by
								  		//  semicolon (ex. "john@host.com; jane@host.com")
								  		
	Char*	subject;			  		// IN: subject string or NULL -- will be duplicated
								  		//  by helper if necessary (ex. "helper API")
								  		
	Char*	message;			  		// IN: initial message body string or NULL -- will be
								  		//  duplicated by helper if necessary (ex.
								 		//  "Lets discuss the helper API tomorrow.")
								 		//	Message will be put on first page.
								 		
	HelperServiceMMSObjectType* object; //IN: Head of Linked list of objects to send or NULL

	Char* 	bcc;						// IN: blind carbon copy address string or NULL -- will
								  		//  be duplicated by helper if necessary;
								  		//  multiple addresses are separated by
								  		//  semicolon (ex. "john@host.com; jane@host.com")
								  		
	Boolean justSend; 			  		// IN: Set this to true to send without invoking any UI

} HelperServiceMMSDetailsType;


#endif //HELPER_MMS_H
