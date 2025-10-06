/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/**
 * @ingroup NETPREF
 */

/**
 * @file 	NetPrefLibTarget.h
 * @version 1.0
 * @date    12/12/2001
 *
 * @brief   This is the target parameters file for the NetPref Library.
 *
 * We break out these definitions so that other portable modules
 * could access them without PalmOS-specific definishions
 *
 */

/*
 * @author	vmk
 *
 * <hr>
 */

#ifndef _NET_PREF_LIB_TARAGET_H_
#define _NET_PREF_LIB_TARAGET_H_



// 'HsNP' was registered on 12/12/2001 @ 4:56pm
#define netPrefLibCreatorID	  'HsNP'	/**< NetPref Library database creator */

#define netPrefLibTypeID	  'libr'	/**< Standard library database type */

#define netPrefLibName		  "HsNetPrefLibrary.lib"  /**< Internal library name
														*  which can be passed to SysLibFind()
														*/


#endif // _NET_PREF_LIB_TARAGET_H_
