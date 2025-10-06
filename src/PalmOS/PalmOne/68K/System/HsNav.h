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
 * @file 	HsNav.h
 * @version 
 * @date   
 *
 * @brief   Header file that 68K programs include to access 
 * HsNav functionality.	
 *	
 */


#ifndef	  __HS_NAV_68K_H__
#define	  __HS_NAV_68K_H__

/**
 * Most of the definitions for HsNav are in HsNavCommon.h so that 
 * they can be shared by both 68K and ARM programs...
 **/
#include <Common/System/HsNavCommon.h>
#include <CoreTraps.h>

/**
 * SYS_SEL_TRAP technically should be defined here as it is defined at the
 *	beginning of HsExt.h.  However, since HsNav.h is always included with
 *	HsExt.h (Hs.h and HsInt.h include both), we are fine.  If HsNav.h is ever
 *	included without HsExt.h, then we'll have to do something different.
 **/

#if PALMOS_SDK_VERSION < 0x0541

/**
 *  @brief Obtains the number of objects in a form’s navigation tab order.
 *
 *  @param formP:	IN:  Pointer to the form whose navigation information is to be obtained.
 *  @retval UInt16 Returns the number of objects in the navigation tab order.
 **/
UInt16
FrmCountObjectsInNavOrder (const FormType * formP)
		SYS_SEL_TRAP (sysTrapNavSelector, NavSelectorFrmCountObjectsInNavOrder);

/**
 *  @brief Obtains information about the navigation order of a form.
 *
 *  @param formP:	IN:  Pointer to the form whose focus order is to be obtained
 *  @param navHeaderP:	IN:  If not NULL, filled with the header information for the form’s navigation order
 *  @param navOrderP:	IN:  If not NULL, will point to an array with info for each object in the navigation order
 *  @param numObjectsP:	IN:  On entry, the number of entries allocated in the navigation order array. On exit,
 *                           the number of entries filled in the navigation order array.
 *  @retval Err Returns 0 if no error, error code otherwise.
 **/
Err
FrmGetNavOrder (const FormType* formP, FrmNavHeaderType* navHeaderP,
				FrmNavOrderEntryType* navOrderP, UInt16* numObjectsP)
		SYS_SEL_TRAP (sysTrapNavSelector, NavSelectorFrmGetNavOrder);

/**
 *  @brief Sets the navigation order of a form. If the object with the current focus is removed
 *         from the order, the object will be redrawn in its non-focused state and the form will
 *         have no current focus. Makes a COPY of all information passed-in.
 *
 *  @param formP:	IN:  Pointer to the form whose focus order is to being set
 *  @param navHeaderP:	IN:  Sets the header information for the form’s navigation order. This parameter
 *                           cannot be NULL.
 *  @param navOrderP:	IN:  Specifies the new navigation order for the form. The number of entries in the
 *                           array must be equal to the header parameter’s numberOfObjects field. Caller can
 *                           pass NULL if they do not want to change the form’s navigation order (they just
 *                           want to change the header information).
 *  @retval Err Returns 0 if no error, error code otherwise
 **/
Err
FrmSetNavOrder (FormType* formP, FrmNavHeaderType* navHeaderP,
				FrmNavOrderEntryType* navOrderP)
		SYS_SEL_TRAP (sysTrapNavSelector, NavSelectorFrmSetNavOrder);

/**
 *  @brief Gets the navigation information for a single object in the form.
 *
 *  @param formP:		IN:  Pointer to the form whose navigation order is being obtained
 *  @param targetObjectID:	IN:  The ID of the object whose navigation information is being obtained
 *  @param afterObjectIDP:	IN:  On exit, will point to the ID of the object which is to the left of the target object.
 *                                   If the target object is the first one in the tab order, will point to an ID of 0. Caller
 *                                   can pass NULL if they do not need this information.
 *  @param aboveObjectIDP:	IN:  On exit, will point to the ID of the object that is above the target object. An ID of
 *                                   0 means the target object is in the top row of the form. Caller can pass NULL if
 *                                   they do not need this information.
 *  @param belowObjectIDP:	IN:  On exit, will point to the ID of the object that is below the target object. An ID of
 *                                   0 means the target object is in the bottom row of the form. Caller can pass NULL
 *                                   if they do not need this information.
 *  @param objectFlagsP:	IN:  On exit, will point to the object flags for the target object. Caller can pass NULL
 *                                   if they do not need this information.
 *  @retval Err Returns 0 if no error, error code otherwise.
 **/
Err
FrmGetNavEntry (const FormType* formP, UInt16 targetObjectID,
				UInt16* afterObjectIDP, UInt16* aboveObjectIDP,
				UInt16* belowObjectIDP, FrmNavObjectFlagsType* objectFlagsP)
		SYS_SEL_TRAP (sysTrapNavSelector, NavSelectorFrmGetNavEntry);

/**
 *  @brief Edits the navigation information for a single object in the form.
 *
 *  @param formP:		IN:  Pointer to the form whose navigation order is being changed
 *  @param targetObjectID:	IN:  The ID of the object which is being edited, added, or removed.
 *  @param afterObjectID:	IN:  The ID of the object which will be to the left of the target object. The object that
 *                                   is currently to the right of the after object will then be to the right of the target
 *				     object. An ID of 0 means the target object should be placed at the beginning of
 *				     the order. An ID of 0xFFFF (frmInvalidObjectId) means that the target object
 *				     should be completely removed from the navigation order.
 *  @param aboveObjectID:	IN:  The ID of the object that will be above the target object. An ID of 0 means the
 *				     target object is in the top row of the form. 
 *  @param belowObjectID:	IN:  The ID of the object that will be below the target object. An ID of 0 means the
 *				     target object is in the bottom row of the form.
 *  @param objectFlags:		IN:  The new object flags for the target object.
 *  @retval Err Returns 0 if no error, error code otherwise.
 **/
Err
FrmSetNavEntry (FormType* formP, UInt16 targetObjectID, 
				UInt16 afterObjectID, UInt16 aboveObjectID, 
				UInt16 belowObjectID, FrmNavObjectFlagsType objectFlags)
		SYS_SEL_TRAP (sysTrapNavSelector, NavSelectorFrmSetNavEntry);

/**
 *  @brief Gets the current navigation state of a form.
 *
 *  @param formP:	IN:  Pointer to the form whose navigation state is to be obtained.
 *  @param stateFlagsP:	IN:  On exit, points to a state flags field that describes the current navigation state
 *  @retval Err Returns 0 if no error, error code otherwise
 **/
Err
FrmGetNavState (const FormType* formP, FrmNavStateFlagsType* stateFlagsP)
		SYS_SEL_TRAP (sysTrapNavSelector, NavSelectorFrmGetNavState);

/**
 *  @brief Sets the navigation state of a form
 *
 *  @param formP:	IN:  Pointer to the form whose navigation state is being set
 *  @param stateFlags:	IN:  State flags that describe the new navigation state 
 *  @retval Err Returns 0 if no error, error code otherwise
 **/
Err
FrmSetNavState (FormType* formP, FrmNavStateFlagsType stateFlags)
		SYS_SEL_TRAP (sysTrapNavSelector, NavSelectorFrmSetNavState);

/**
 *  @brief 
 *
 *  @param formP:	IN:  
 *  @param objectID:	IN:  
 *  @param extraInfo:	IN:  
 *  @param boundsInsideRingP:	IN:  
 *  @param ringStyle:	IN:  
 *  @param forceRestore:	IN:  
 *  @retval Err Error code.
 **/
Err
FrmNavDrawFocusRing (FormType* formP, UInt16 objectID, Int16 extraInfo,
				     RectangleType* boundsInsideRingP,
					 FrmNavFocusRingStyleEnum ringStyle, Boolean forceRestore)
		SYS_SEL_TRAP (sysTrapNavSelector, NavSelectorFrmNavDrawFocusRing);

/**
 *  @brief 
 *
 *  @param formP:	IN:  
 *  @retval Err Error code.
 **/
Err
FrmNavRemoveFocusRing (FormType* formP)
		SYS_SEL_TRAP (sysTrapNavSelector, NavSelectorFrmNavRemoveFocusRing);

/**
 *  @brief 
 *
 *  @param formP:	IN:  
 *  @param objectIDP:	IN:  
 *  @param extraInfoP:	IN:  
 *  @param boundsInsideRingP:	IN:  
 *  @param ringStyleP:	IN:  
 *  @retval Err Error code.
 **/
Err
FrmNavGetFocusRingInfo (const FormType* formP, UInt16* objectIDP, 
					    Int16* extraInfoP, RectangleType* boundsInsideRingP,
					    FrmNavFocusRingStyleEnum* ringStyleP)
		SYS_SEL_TRAP (sysTrapNavSelector, NavSelectorFrmNavGetFocusRingInfo);

#endif // PALMOS_SDK_VERSION < 0x0541

/**
 *  @brief 
 *
 *  @param formP:	IN:  
 *  @param objID:	IN:  
 **/
void
FrmNavObjectTakeFocus (const FormType* formP, UInt16 objID)
		SYS_SEL_TRAP (sysTrapNavSelector, NavSelectorFrmNavObjectTakeFocus);

#endif	  // __HS_NAV_68K_H__
