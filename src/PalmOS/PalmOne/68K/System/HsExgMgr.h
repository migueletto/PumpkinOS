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
 * @file 	HsExgMgr.h
 * @version 
 * @date   
 *
 * @brief   Extensions to the Exchange Manager.  
 *
 * 	    These functions are not currently supported by Palm, but the plan to
 * 	    in future releases. They gave us these functions in sample code, and
 * 	    we are moving into the system for OS 5 products so applications can
 * 	    take advantage of them.
 *	
 */

/** 
 * @author	 Arun Mathias
 *
 * <hr>
 */

  

#ifndef HS_EXG_MGR__H
#define HS_EXG_MGR__H


/**
 *  @brief 
 *
 *  @param id:	IN:  
 *  @param *dataTypeP:	IN:  
 *  @param *targetP:	IN:  
 *  @retval 
 **/
Err HsExtExgSelectTarget( UInt16 id, const Char *dataTypeP, UInt32 *targetP)
                SYS_SEL_TRAP (sysTrapHsSelector, hsSelExgSelectTarget);

/**
 *  @brief 
 *
 *  @param exgSocketP:	IN:  
 *  @retval 
 **/
Err HsExtExgLocalAccept(ExgSocketPtr exgSocketP)
                SYS_SEL_TRAP (sysTrapHsSelector, hsSelExgLocalAccept);

/**
 *  @brief 
 *
 *  @param exgSocketP:	IN:  
 *  @param err:		IN:  
 *  @retval 
 **/
Err HsExtExgLocalDisconnect(ExgSocketPtr exgSocketP, Err err)
                SYS_SEL_TRAP (sysTrapHsSelector, hsSelExgLocalDisconnect);
#endif
