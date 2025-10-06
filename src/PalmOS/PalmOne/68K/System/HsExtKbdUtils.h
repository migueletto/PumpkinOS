/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/**
 *
 * @ingroup SystemDef
 *
 */
 
/**
 * @file 	HsExtKbdUtils.h
 * @version 
 * @date   
 *
 * @brief   
 *	
 */


#ifndef _HS_EXT_KBD_UTILS_H
#define _HS_EXT_KBD_UTILS_H

//#include <Hs.h>
/**
 *  @brief 
 *
 *  @param overrideFont:	IN:  
 *  @param overrideFontID:	IN:  
 *  @retval Error code.
 **/
Int16
HsPostProcessPopupList (Boolean overrideFont, FontID overrideFontID)
		  SYS_SEL_TRAP (sysTrapHsSelector, hsSelPostProcessPopupList);


#endif // _HS_EXT_KBD_UTILS_H
