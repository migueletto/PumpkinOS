/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/** @ingroup Telephony
 *
 */
 
 
/**	
 *
 * @file 	HsPhoneTrace.h
 *
 * @brief  Trace constants, types, and definitions for Phone Library.
 *				
 */




#ifndef HS_PHONE_TRACE_H
#define HS_PHONE_TRACE_H


/**
 *  @name Trace Subcomponent IDs.  These are used in TraceContext to identify area of Phone Library providing
 *        a particular trace/error entry.  These are helpful to allow useful filtering of trace log data.
 *
 **/
/*@{*/
#define PHN_SUBCOMPONENTID_LIB	'LIB!'		/**< general Phone Library (library activation/shutdown, API, etc) */
#define PHN_SUBCOMPONENTID_SMS	'SMS!'		/**< SMS Messaging */
#define PHN_SUBCOMPONENTID_NOT	'NOT!'		/**< Phone Notification Mgr */
#define PHN_SUBCOMPONENTID_RDO	'RDO!'		/**< Radio operation  */
#define PHN_SUBCOMPONENTID_READ	'READ'		/**< low level communication with radio - read */
#define PHN_SUBCOMPONENTID_WRIT 'WRIT'          /**< low level communication with radio - write  */
#define PHN_SUBCOMPONENTID_SIM	'SIM!'		/**< sim operation */
#define PHN_SUBCOMPONENTID_STK  'STK!'        	/**< sim toolkit */
/*@}*/

#endif
