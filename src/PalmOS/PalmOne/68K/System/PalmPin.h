/******************************************************************************
 * Copyright (c) 2005 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/** 
 *@ingroup System
 *
 */
 
/**
 * @file 	PalmPin.h
 * @version 1.0
 * @date 	03/03/2003
 *
 * @brief  Public API for the Pen Input Manager Library
 * 
 *
 * <hr>
 */


#ifndef __PINLIB_H__ 
#define __PINLIB_H__

#include <PalmTypes.h>
#include <LibTraps.h>

/** 
 * @name Library type and creator
 *
 */
/*@{*/
#define pinLibType			sysFileTLibrary		/**<		*/
#define pinLibCreator			'pinM'			/**<		*/	
#define pinLibName			"PinLib"		/**<		*/
#define pinFtrNumVersion		0			/**<		*/
/*@}*/

/** 
 * @name Pinlet IDs
 *
 */
/*@{*/
#define pinClassic			"Classic"	/**<		*/
#define pinTriCell			"Tri Cell"	/**<		*/
#define pinStdKeyboard			"StdKB"		/**<		*/
#define pinletStdKbNum			"StdKBNum"	/**<		*/
#define pinletStdKbIntl			"StdKBIntl"	/**<		*/
/*@}*/

/** 
 * @name PIN Manager Errors
 *
 */
/*@{*/
#define pinErrorClass			(appErrorClass | 0x0A00) 	/**<		*/
#define pinErrInvalidState		(pinErrorClass + 1)		/**<		*/
#define pinErrUnknownID			(pinErrorClass + 2)		/**<		*/
#define pinErrInvalidInputMode		(pinErrorClass + 3)		/**<		*/
/*@}*/

/** 
 * @name Input area states
 *
 */
/*@{*/
#define pinInputAreaNone		0	/**<		*/
#define pinInputAreaShow		1	/**<		*/
#define pinInputAreaHide		2	/**<		*/
#define pinInputAreaLegacyMode		3	/**<		*/
#define pinInputAreaFullScreen		4	/**<		*/
/*@}*/

/** 
 * @name Input Modes
 *
 */
/*@{*/
#define pinInputModeNormal		0	/**<		*/
#define pinInputModeShift		1	/**<		*/
#define pinInputModeCapsLock		2	/**<		*/
#define pinInputModePunctuation		3	/**<		*/
#define pinInputModeNumeric		4	/**<		*/
#define pinInputModeExtended		5	/**<		*/
#define pinInputModeHiragana		6	/**<		*/
#define pinInputModeKatakana		7	/**<		*/
/*@}*/

/** 
 * @name AIA event and notification
 *
 */
/*@{*/
#define sysNotifyAiaEvent		'Aian'		/**<		*/
#define AiaExtentChangedEvent		0x5000		/**<		*/
/*@}*/

typedef struct AiaExtentChangedEventDataType
{
	RectangleType newDim;		/**<		*/
	RectangleType oldDim;		/**<		*/
} AiaExtentChangedEventDataType;

/**
 * Macro to simplify getting the data out of the event structure.
 * 
 * Example:
 * yDiff = AiaExtentChangedData(eventP)->newDim->extent.y -
 *         aiaExtentChangedData(eventP)->oldDim->extent.y;
 **/
#define aiaExtentChangedData(eventP) ((AiaExtentChangedEventDataType *)(&((eventP)->data.generic)))	/**<		*/


/** 
 * @name Library Traps
 *
 */
/*@{*/
#define kPinLibTrapOpen				sysLibTrapOpen			/**<		*/
#define kPinLibTrapClose			sysLibTrapClose			/**<		*/
#define kPinLibTrapPinGetInputAreaState		(sysLibTrapCustom + 17)		/**<		*/
#define kPinLibTrapPinSetInputAreaState		(sysLibTrapCustom + 18)		/**<		*/
#define kPinLibTrapPinResetInputState		(sysLibTrapCustom + 24)		/**<		*/
#define kPinLibTrapPinGetCurrentPinletID	(sysLibTrapCustom + 20)		/**<		*/
#define kPinLibTrapPinSwitchToPinlet		(sysLibTrapCustom + 21)		/**<		*/
#define kPinLibTrapPinGetInputMode		(sysLibTrapCustom + 23)		/**<		*/
#define kPinLibTrapPinSetInputMode		(sysLibTrapCustom + 22)		/**<		*/
#define kPinLibTrapPINShowReferenceDialog	(sysLibTrapCustom + 25)		/**<		*/
/*@}*/

/**
 * Prototypes
 **/
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief
 *
 * @param refnum:	IN:  
 * @retval Err error code.
 **/
Err	PinLibOpen(UInt16 refnum)
				SYS_TRAP(kPinLibTrapOpen);

/**
 * @brief
 *
 * @param refnum:	IN:  
 * @retval Err error code.
 **/
Err	PinLibClose(UInt16 refnum)
				SYS_TRAP(kPinLibTrapClose);

/**
 * @brief
 *
 * @param refnum:	IN:  
 * @retval Err error code.
 **/
UInt16 PinGetInputAreaState(UInt16 refnum)
				SYS_TRAP(kPinLibTrapPinGetInputAreaState);
				
/**
 * @brief
 *
 * @param refnum:	IN:  
 * @param state:	IN:  
 * @retval Err error code.
 **/
Err PinSetInputAreaState(UInt16 refnum, UInt16 state)
				SYS_TRAP(kPinLibTrapPinSetInputAreaState);

/**
 * @brief
 *
 * @param refnum:	IN:  
 * @retval Err error code.
 **/
void PinResetInputState(UInt16 refnum)
				SYS_TRAP(kPinLibTrapPinResetInputState);
				
/**
 * @brief
 *
 * @param refnum:	IN:  
 * @retval Err error code.
 **/
const char* PinGetCurrentPinletID(UInt16 refnum)
				SYS_TRAP(kPinLibTrapPinGetCurrentPinletID);
			
/**
 * @brief
 *
 * @param refnum:	IN:  
 * @param pinletID:	IN:  
 * @param initialInputMode:	IN:  
 * @retval Err error code.
 **/	
Err PinSwitchToPinlet(UInt16 refnum, const char* pinletID, UInt16 initialInputMode)
				SYS_TRAP(kPinLibTrapPinSwitchToPinlet);

/**
 * @brief
 *
 * @param refnum:	IN:  
 * @retval Err error code.
 **/
UInt16 PinGetInputMode(UInt16 refnum)
				SYS_TRAP(kPinLibTrapPinGetInputMode);
			
/**
 * @brief
 *
 * @param refnum:	IN:  
 * @param inputMode:	IN:  
 * @retval Err error code.
 **/	
void PinSetInputMode(UInt16 refnum, UInt16 inputMode)
				SYS_TRAP(kPinLibTrapPinSetInputMode);
	
				
				
#ifdef __cplusplus
}
#endif


#endif  //__PINLIB_H__
