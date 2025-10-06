/** 
 * \file HsKeyCommon.h
 *
 * Header file with additional keyboard related defninitions
 * that should eventually appear in Palm's headers (in OS 6)
 *
 * \license
 * 
 * Copyright (c) 2003 Handspring Inc., All Rights Reserved
 *
 * $Id: //device/handheld/dev/sdk/2.1/incs/common/system/HsKeyCommon.h#4 $
 *
 ****************************************************************/

#ifndef	  __HS_KEY_COMMON_H__
#define	  __HS_KEY_COMMON_H__


#if 0
#pragma mark -------- Incs:Core:System:Chars.h -----------
#endif

#define vchrHardRockerCenter			0x013D	// Character tied directly to rocker-
												//	center hardware key.  vchrRockerCenter
												//	is associated with select action of
												//	rocker-center (which happens on key-up
												//	of rocker-center hardware key).

#if 0
#pragma mark -------- Incs:Core:System:KeyMgr.h -----------
#endif

//*********************************************************
// Bit definitions for new hardware keys
//*********************************************************

#define keyBitThumbWheelUp	  0x00001000  // Thumb-wheel up
#define keyBitThumbWheelDown  0x00002000  // Thumb-wheel down
#define keyBitThumbWheelPress 0x00004000  // Press/center on thumb-wheel
#define keyBitThumbWheelBack  0x00008000  // Thumb-wheel back button

#define keyBitRockerUp		  0x00010000  // 5-way rocker up
#define keyBitRockerDown	  0x00020000  // 5-way rocker down
#define keyBitRockerLeft	  0x00040000  // 5-way rocker left
#define keyBitRockerRight	  0x00080000  // 5-way rocker right
#define keyBitRockerCenter	  0x00100000  // 5-way rocker center/press


#if 0
#pragma mark -------- Incs:Core:System:SystemMgr.h ---------
#endif

//*********************************************************
// Feature definitions for publishing what hardware keys
//	are available on a device
//*********************************************************

#define sysFtrNumUIHardwareFlags			  27          // Additional User Input Hardware (PalmOS 5.x)
#define sysFtrNumUIHardwareHas5Way			  0x00000001  // Device has a 5-way rocker
#define sysFtrNumUIHardwareHasThumbWheel	  0x00000002  // Device has a thumb wheel
#define sysFtrNumUIHardwareHasThumbWheelBack  0x00000004  // Device has a thumb wheel with a 'back' button
#define sysFtrNumUIHardwareHasKbd			  0x00000008  // Device has a dedicated keyboard


#if 0
#pragma mark -------- Incs:Core:System:SysEvent.h (68K) -------
#pragma mark ------ Incs:Core:Common:CmnKeyTypes.h (ARM) ------
#endif

//*********************************************************
// New modifiers mask for keyDown event
//*********************************************************

#define willSendUpKeyMask  0x0800   // True if a keyUp event will be sent later



#endif	  // __HS_KEY_COMMON_H__




