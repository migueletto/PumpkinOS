/******************************************************************************
 * Copyright (c) 2005 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/** 
 *@ingroup System
 *
 */
 
/**
 * @file 	PalmVMLaunch.h
 * @version 1.0
 * @date 	07/21/2003
 *
 * @brief  Defines APIs and structures for launching VersaMail to add a new message
 *	   from an application	
 * 
 *
 * <hr>
 */

 
#ifndef __PALMVMLAUNCH_H__
#define __PALMVMLAUNCH_H__

#include <PalmCompatibility.h>
#include <AppLaunchCmd.h>  /**< Declares MailAddRecordParamsType */


/** 
 * @name Launch Codes
 *
 */
/*@{*/
#define MMPRO_LAUNCH_CODE 			(sysAppLaunchCmdCustomBase + 2)		/**< Launch code for launching VersaMail through SysUIAppSwitch.*/
#define MMPRO_ADD_MESSAGE_WITH_ATTACHMENT	(sysAppLaunchCmdCustomBase + 16)	/**< Launch for adding a message to VersaMail with attachments, more powerful than sysAppLaunchCmdAddRecord */
/*@}*/

/**
 * Typedefs and constants used in VM Launch library
 **/

/** 
 * @name 
 *
 */
/*@{*/
#define MM_MAX_ATTACHMENTS		10		/**< Maximum no. of attach */
/*@}*/

/**
 * @brief Structure for adding attachments in MMProLaunchStruct
 */
typedef struct
{
	int type;			/**< Type of attachment */
	char* attachName;  		/**< Name of attachment */
	char* attachment;   		/**< Pointer to attachment data*/
	unsigned long aSize;		/**< Size of attachment */
} MMProAttachStruct;

/**
 * @brief Structure to add a message + attachment passed with MMPRO_LAUNCH_CODE
 */
typedef struct 
{
	Boolean interactive;		/**< Not used */
	char* to;			/**< To field */	
	char* cc;			/**< cc field */	
	char* bcc;			/**< bcc field */	
	char* subject;			/**< subject field */	
	char* body;			/**< body field */	
	int numAttach;			/**< Number of attachments */
	MMProAttachStruct* attachments; /**<Array of attachment(s) */
}MMProLaunchStruct;

/**
 * @brief Data links used in an attachment, used in MMProAttachStruct2
 */
typedef struct
{
	CharPtr data; 		/**< Pointer to the data link */
	ULong size;   		/**< Size of data link */
} MMProAttachDataLinks;

/**
 * @brief Structure for adding attachments in MMProLaunchStruct2
 */
typedef struct
{
	int type;			/**< Type of attachment */
	char* attachName;		/**< Name of attachment */
	char* attachment;		/**< Pointer to attachment data*/
	unsigned long aSize;		/**< Size of attachment */
	UInt numDataLinks;		/**< No. of datalinks */
	MMProAttachDataLinks *moreData; /**< Array of data link(s) */
} MMProAttachStruct2;

/**
 * @brief  Structure to add a message + attachment passed with MMPRO_LAUNCH_CODE
 */
typedef struct 
{
	Boolean interactive;
	char* to;			/**< To field */
	char* cc;			/**< cc field */	
	char* bcc;			/**< bcc field */
	char* subject;			/**< subject field */
	char* body;			/**< body field */	
	int numAttach;			/**< Number of attachments */
	MMProAttachStruct2* attachments; /**< Pointer to attachment array */
}MMProLaunchStruct2;


/**
 * @brief  Structure to add attachment in MailAddRecordParamsTypePlus
 */
typedef struct
{
	char*	attachType; 		/**< Mime Type of attachment */
	char*	attachDescription;	/**< Attachment description*/	
	char*	attachFileName;		/**< FileName of attachment */
	union 
	{
		char*	attachData; 	/**<Pointer to attachment data */
		LocalID	attachDBID;	/**<Currently not supported */
	} data;
	
	UInt32		attachSize; 	/**< Size of attachment */
	
}	MailAddRecordAttachment;

/**
 * @brief  Structure to add a message + attachment passed with MMPRO_ADD_MESSAGE_WITH_ATTACHMENT
 *	   Categories & MailAddRecordParamsType are defined in AppLaunchCmd.h.
 *
 */
typedef struct
{
	MailAddRecordParamsType		mainParams;	/**< Email data */   
	UInt8				category;  	/**< Mail Category usually Oubox / Draft */   
	MailAddRecordAttachment		attachments[MM_MAX_ATTACHMENTS]; 	/**< Pointer to array of attachments */
	Int16				numAttachments; /**< No. of attachments */

} MailAddRecordParamsTypePlus;

							
#endif 