/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
 
/**
 * @file 	HsHelper.h
 * @version 1.0
 * @date 	
 *  Description:
 *	Public header file for the Handspring helper functions
 */

#ifndef HSHELPER_H
#define HSHELPER_H

#include <Helper.h>
#define sysNotifyHelperEvent   'hlpr'

/**
 * MACRO: HelperEnumerateEnqueueEntry
 *
 * Enqueues a node of type HelperEvtEnumerateNodeType. Each node
 *  must be individually allocated from the heap by the helper.
 *
 * PROTOTYPE:
 *	  void HelperEnumNodeEnqueue(HelperNotifyEnumerateType* enumEvtDataP,
 *								 HelperNotifyEnumerateListType* entryP);
 */
#define HelperEnumerateEnqueueEntry(enumEvtDataP, entryP)	  \
  do												  \
	{												  \
	(entryP)->nextP = (enumEvtDataP) 	;	  \
	(enumEvtDataP) = (entryP);				  \
	}												  \
  while (0)


/**
 * @brief Param structure for pmNotifyGetNewMsgCountFromHelper notifications
 *
 */
typedef struct 
{
  UInt32 helperServiceClass;			/**<  		*/
} PmGetNewMsgCountNotifyParamType, * PmGetNewMsgCountNotifyParamPtr;


/**
 * @brief Param structure for pmNotifyBroadcastNewMsgCount notifications
 *
 */
typedef struct
{
  UInt32  helperServiceClass;			/**<  		*/
  UInt16  msgCount;				/**<  		*/
} PmBroadcastNewMsgCountNotifyParamType, * PmBroadcastNewMsgCountNotifyParamPtr;

#endif