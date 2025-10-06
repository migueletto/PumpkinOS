/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/** @ingroup Telephony
 *
 */
 
 
/**
 * @file 	HsPhonePower.h
 *
 * @brief  Header File for Phone Library API ---- POWER CATEGORY
 *
 * Notes:
 * 	All implementations of the Handspring Phone Library support a common API. 
 * 	This API is broken up into various categories for easier management.  This file
 * 	defines the POWER category.  This category provides routines to query & manage
 * 	the radio power. 
 *			
 */



#ifndef HS_PHONEPOWER_H
#define HS_PHONEPOWER_H
#include <PalmOS.h>
#include <PalmTypes.h>
#ifndef __CORE_COMPATIBILITY_H__
#include <PalmCompatibility.h>
	/** workaround for differing header files in sdk-3.5 and sdk-internal */
	#ifndef __CORE_COMPATIBILITY_H__
	#define __CORE_COMPATIBILITY_H__
	#endif 
#endif 
#include <Common/Libraries/Telephony/HsPhoneTraps.h>     /**< trap table definition for phone library calls */
#include <Common/Libraries/Telephony/HsPhoneErrors.h>    /**< error codes returned by phone library functions */
#include <Common/Libraries/Telephony/HsPhoneTypes.h>



/**
 *  @brief 
 *  
 *  @param refNum:	IN:  
 *  @param on:		IN:  
 *  @retval Err Error code.
 **/
  extern Err PhnLibSetModulePower (UInt16 refNum, Boolean on)
      PHN_LIB_TRAP(PhnLibTrapSetModulePower);


/**
 *  @brief 
 *  
 *  @param refNum:	IN:   
 *  @retval Err Error code.
 **/
  extern PhnPowerType PhnLibModulePowered (UInt16 refNum)
      PHN_LIB_TRAP (PhnLibTrapModulePowered);


/**
 *  @brief 
 *  
 *  @param refNum:	IN:
 *  @retval Err Error code.
 **/
extern Err PhnLibContinuePhonePowerOn (UInt16 refNum)
	  PHN_LIB_TRAP (PhnLibTrapContinuePhonePowerOn);


/**
 *  @brief 
 *  
 *  @param refNum:	IN:
 *  @retval Err Error code.
 **/
extern Err PhnLibContinuePhonePowerOff (UInt16 refNum)
	  PHN_LIB_TRAP (PhnLibTrapContinuePhonePowerOff);
	  
#endif
