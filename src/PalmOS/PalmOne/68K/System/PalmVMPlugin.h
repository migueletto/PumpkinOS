/******************************************************************************
 * Copyright (c) 2005 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/** 
 *@ingroup System
 *
 */
 
/**
 * @file 	PalmVMPlugin.h
 * @version 1.0
 * @date 	07/02/2003
 *
 * @brief  Defines APIs and structures for the VersaMail Plugin Service which 
 * 	   lets users write plugins to view/send attachments
 * 
 *
 * <hr>
 */

 
#ifndef __PALMVMPLUGIN_H__
#define __PALMVMPLUGIN_H__ 


#include <PalmCompatibility.h>


/** 
 * @name Application Launch Codes
 *
 */
/*@{*/
#define MMPRO_PLUGIN_LAUNCHCODE 		(sysAppLaunchCmdCustomBase + 2)		/**< Plugin receives data from VersaMail and do whatever it deems appropriate with it. */
#define MMPRO_PLUGIN_RECEIVE_LAUNCHCODE 	MMPRO_PLUGIN_LAUNCHCODE			/**<		*/

#define MMPRO_PLUGIN_QUERY_LAUNCHCODE   	(sysAppLaunchCmdCustomBase + 3)		/**< Plugin should return a list of attachments for the user to select from. */
#define MMPRO_PLUGIN_SEND_LAUNCHCODE		(sysAppLaunchCmdCustomBase + 4)		/**< The plugin should return the name of ? selected attachment for sending */
#define MMPRO_PLUGIN_EXTENDED_QUERY_LAUNCHCODE  (sysAppLaunchCmdCustomBase + 5)		/**< Plugin should support selection of attachment and should return */
#define MMPRO_PLUGIN_GET_INFO_LAUNCHCODE  	(sysAppLaunchCmdCustomBase + 8)		/**< VersaMail queries the plugin to get the details of Plugin information */

/*@}*/


#define ATTYPE_MAX_NAME_LEN	32	/**< Maximum array size for name of an attachment in listSendNames pointer */



/**
 * @brief PluginInfo is specifies what plugin information is needed.
 */
typedef enum {

	plugin_sendInfo,  	/**< Provides info about what can be passed as an email attachment. */
	plugin_receiveInfo,  	/**< Provides info about what file types can be viewed. */
	plugin_allInfo 		/**< Provides sendInfo and receiveInfo. */

} PluginInfo;

/**
 * @brief Structure used for launch code MMPRO_PLUGIN_GET_INFO_LAUNCHCODE.
 */
typedef struct _pluginGetInfoParams
{
	PluginInfo	getInfo;		/**< [in]  what plugin information needed. */
	
	UInt32		version;		/**< [out] plugin version. */
	Boolean		supportsSending;	/**< [out] can provide data to be passed as an email attachment.*/
	Boolean		supportsReceiving;	/**< [out] can view files passed to it.*/
	
	char**		listSendNames;		/**< [out] list of names of the types. (suitable for listing in popup)*/
	
	UInt16		numSendTypes;		/**< [out] the number of types it can send (each one should appear in popup)*/
	Int16		numReceiveTypes;	/**< [out] the number of types it can receive */

	char**		listSendFileExt;	/**< [out] list of supported file extensions to Send.*/
	char**		listReceiveFileExt;	/**< [out] list of supported file extensions it can read. */
	UInt32*		listMaxSizeExt;		/**< lout] list of max file size supported for each file ext. */
	
	char**		listSendMIMEtypes;	/**< [out] list of supported MIME types to Send. */
	char**		listReceiveMIMEtypes;	/**< [out] list of supported MIME types it can read. */
	UInt32*		listMaxSizeMime;	/**< lout] list of max file size supported for each MIME type. */
	UInt32      ret;          		/**< [out] return code for error handling and special cases; */
	Char Unused[50];      			/**< unused space for future releases */
	
} pluginGetInfoParams;


/**
 * @brief Structure used for datalinks while sending /rcving data.
 */
typedef struct _dataLinks
{
	CharPtr data; 		/**< pointer to data */
	ULong size; 		/**< size of data */
	
} dataLinks;


/**
 * @brief Structure used for launch code MMPRO_PLUGIN_RECEIVE_LAUNCHCODE.
 */
typedef struct _pluginParams
{
	ULong version;			/**< [in] plugin version */
	Char *MIMEtype; 		/**< [in] attachment MIME type */
	Char *fileExt; 			/**< [in] attachment file extention */
	ULong size;			/**< [in] size of data */
	CharPtr fname;			/**< [in] filename of attachment */
	CharPtr data; 			/**< [in] the data */
	DmOpenRef db; 			/**< [in] reference to open database for large memory allocations */
	UInt numDataLinks;  		/**< [in] number of data links */
	dataLinks *moreData;		/**< [in] ptr to data links */
	UInt32 ret;         		/**< [out] return code for error handling and special cases;*/
	Char Unused[50];    		/**< unused space for future releases */
	
}pluginParams;



/**
 * @brief Structure used for launch code MMPRO_PLUGIN_QUERY_LAUNCHCODE.
 */
typedef struct _pluginQueryParams
{
	Char *fileExt; 		        /**< [in] file extention of list of files requested */
	Char *MIMEtype;			/**< [in] MIME type of list of files requested */
	DmOpenRef db; 			/**< [in]reference to open database for large memory allocations */
	int numItems;			/**< [out] number of items of this type */
	UInt32 *itemSizes;		/**< [out] size of items */
	char** listItems;		/**< [out] the items */
	void**	itemDescriptors;	/**< [out] private descriptors defined/recognized by plugin. This memory will be freed by VersaMail  */
	UInt32 ret;        		/**< [out] return code for error handling and special cases; */	
	Char Unused[50];      		/**< unused space for future releases */
								
}pluginQueryParams;


/**
 * @brief Structure used for launch code MMPRO_PLUGIN_SEND_LAUNCHCODE.
 */
typedef struct _pluginSendParams
{
	Char *MIMEtype; 	    	/**< [in] attachment MIME type */
	Char *fileExt; 		    	/**< [in] attachment file extention */

	DmOpenRef db;			/**< [in]reference to open database for large memory allocations */
	int selectedItem;		/**< [in] item number that was selected */
	void *selectedDescriptor;	/**< [in] descriptor of item that was selected */
	char* item;			/**< [in] the item selected */
	char* name;			/**< [out] name of the attachment file */
	char* ptr;			/**< [out] attachment rendered and ready to send! */
					/**< this memory will be freed by MMPRO */
	ULong size;			/**< [out] size of attachment above */

	UInt numDataLinks;		/**< [out] number of data links */
	dataLinks *moreData;		/**< [out] ptr to data links */
	UInt32 ret;           		/**< [out] return code for error handling and special cases; */	
	Char Unused[50];      		/**< unused space for future releases */

}pluginSendParams;


/**
 * @brief Structure used for launch code MMPRO_PLUGIN_EXTENDED_QUERY_LAUNCHCODE.
 */
typedef struct _pluginExtendedQueryParams
{
	int type; 			/**< [in] attachment type  */
	Char *MIMEtype; 		/**< [in] attachment MIME type */
	Char *fileExt; 			/**< [in] attachment file extention */

	DmOpenRef db;			/**< [in]reference to open database for large memory allocations */
	Boolean supported;  		/**< [out] we support this extension */
	int numItems;			/**< [out] number of items selected */
	char** listNames;		/**< [out] name of the attachment files */
	char** listPtrs;		/**< [out] attachments renders and ready to send */
	ULong** size;			/**< [out] size for each attachment */
	UInt32 ret;         		/**< [out] return code for error handling and special cases; */
	Char Unused[50];    		/**< unused space for future releases */
	
}pluginExtendedQueryParams;

#endif // PALMVMPLUGIN_H_