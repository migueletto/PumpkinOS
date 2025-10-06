/******************************************************************************
 * Copyright (c) 2005 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/**
 *
 * @ingroup HSExt
 *
 */

/**
 * @file 	HsExt.h
 * @brief   Header file that 68K programs include to access
 * 	    Handspring Extensions functionality.
 *
 *
 */



#ifndef	  __HS_EXT_68K_H__
#define	  __HS_EXT_68K_H__

// Most of the definitions for HsExtensions are now in HsExtCommon.h so that
// they can be shared by both 68K and ARM programs...
#include <Common/System/HsExtCommon.h>
#include <Common/System/HsCreators.h>
#include <68K/System/HsExtTraps.h>

#if 0
#if (!(defined NO_HSEXT_TRAPS)) && (defined __GNUC__) && (EMULATION_LEVEL == EMULATION_NONE)
  #if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 95)

	#ifndef _Str
	#define _Str(X)  #X
	#endif

	#define _HS_OS_CALL_WITH_UNPOPPED_16BIT_SELECTOR(table, vector, selector)	\
		__attribute__ ((__callseq__ (										\
			"move.w #" _Str(selector) ",-(%%sp); "							\
			"trap #" _Str(table) "; dc.w " _Str(vector))))

	#define SYS_SEL_TRAP(trapNum, selector) \
		_HS_OS_CALL_WITH_UNPOPPED_16BIT_SELECTOR(sysDispatchTrapNum, trapNum, selector)


  #else // GNUC < 2.95

	#define SYS_SEL_TRAP(trapNum, selector) \
	    __attribute__ ((inline (0x3f3c, selector, m68kTrapInstr+sysDispatchTrapNum,trapNum)))

  #endif // GNUC < 2.95

#elif (!(defined NO_HSEXT_TRAPS)) && (defined (__MWERKS__))

	#define SYS_SEL_TRAP(trapNum, selector) \
		= {0x3f3c, selector, m68kTrapInstr+sysDispatchTrapNum,trapNum}

#else
  	#define SYS_SEL_TRAP(trapNum, selector)

#endif
#endif

#define SYS_SEL_TRAP(trapNum, selector)

/**
 *  @brief
 *
 *  @param appCardNo:	IN:
 *  @param appDbId:	IN:
 *  @param copyrightYearStrP:	IN:
 *  @param extraCreditsStrP:	IN:
 *  @retval Error code.
 **/
void
HsAboutHandspringApp (UInt16 appCardNo, LocalID appDbId,
					  Char* copyrightYearStrP, Char* extraCreditsStrP)
				SYS_SEL_TRAP (sysTrapHsSelector, hsSelAboutHandspringApp);

//ported from HsExt.h from the Visor-4.0 tree - VSB
// <chg 30-Jun-99 dia> Defined macros to make about box easier to call.
#define HsAboutHandspringAppWithYearId(yearId)								\
	do																		\
	  {																		\
		UInt16	appCardNo;													\
		LocalID	appDbId;													\
		MemHandle	yearStrH;												\
		Char* 	yearStrP;													\
																			\
		yearStrH = DmGetResource (strRsc, yearId);							\
		yearStrP = MemHandleLock (yearStrH);								\
		SysCurAppDatabase(&appCardNo, &appDbId);							\
		HsAboutHandspringApp (appCardNo, appDbId, yearStrP, NULL);			\
		MemPtrUnlock (yearStrP);											\
		DmReleaseResource (yearStrH);										\
	  }																		\
	while (0)

#define HsAboutHandspringAppWithYearCredId(yearId, creditsId)				\
	do																		\
	  {																		\
		UInt16	appCardNo;													\
		LocalID	appDbId;													\
		MemHandle	yearStrH, extraStrH;									\
		Char* 	yearStrP;													\
		Char*	extraStrP;													\
																			\
		yearStrH = DmGetResource (strRsc, yearId);							\
		yearStrP = MemHandleLock (yearStrH);								\
		extraStrH = DmGetResource (strRsc, creditsId);						\
		extraStrP = MemHandleLock (extraStrH);								\
		SysCurAppDatabase(&appCardNo, &appDbId);							\
		HsAboutHandspringApp (appCardNo, appDbId, yearStrP, extraStrP);		\
		MemPtrUnlock (extraStrP);											\
		DmReleaseResource (extraStrH);										\
		MemPtrUnlock (yearStrP);											\
		DmReleaseResource (yearStrH);										\
	  }																		\
	while (0)

#ifdef __cplusplus
extern "C" {
#endif

/**
 *  @brief Retrieve a null terminated version string.
 *
 *  @param selector:	IN:  Selects which version string to get
 *  @param outStrP:	IN:  Buffer to receive the NULL terminated version string. The buffer should be
 *			     large enough to hold hsVersionStringSize bytes
 *  @param sizeP:	IN:  If not NULL, returns the number of bytes stored in *outStrP, including the
 *			     NULL terminator
 *  @retval 0 if no error
 **/
Err
HsGetVersionString (UInt16 /*HsVerStrEnum*/selector, Char* outStrP,
					UInt16* sizeP)
				SYS_SEL_TRAP (sysTrapHsSelector, hsSelGetVersionString);

/**
 *  @brief Get any Handspring specific attributes that aren’t appropriate to be accessed
 *         through a feature. (e.g., dynamic information about device state) @see HsAttrEnum
 *
 *  @param attr:	IN:  Which attribute is being retrieved. Most attributes are defined in
 *			     “HsExtCommon.h” in the HsAttrEnum enumeration.
 *  @param param:	IN:  Meaning depends on ’attr’ (currently implemented attributes only take 0 for
 *			     this argument).
 *  @param valueP:	IN:  Cast depending on attr (refer HsAttrEnum for data type this argument should
 *                           point to).
 *  @retval Err 0 if no error
 **/
Err
HsAttrGet (UInt16 attr /*HsAttrEnum*/, UInt32 param, UInt32* valueP)
				SYS_SEL_TRAP (sysTrapHsSelector, hsSelAttrGet);

/**
 *  @brief Set Handspring specific attributes. @see HsAttrEnum
 *
 *  @param attr:	IN:  Which attribute is being set.
 *  @param param:	IN:  Meaning depends on ’attr’ (currently implemented attributes only take 0 for
 *   			     this argument).
 *  @param valueP:	IN:  Cast depending on attr (refer HsAttrEnum for data type this argument should
 *			     point to).
 *  @retval Err 0 if no error hsErrNotSupported otherwise.
 **/
Err
HsAttrSet (UInt16 attr /*HsAttrEnum*/, UInt32 param, UInt32* valueP)
				SYS_SEL_TRAP (sysTrapHsSelector, hsSelAttrSet);

/**
 *  @brief HsExtensions maintains and utilizes a set of preferences that are not a part of the
 * 	   Palm OS system preferences. They can be retrieved using this function. @see HsPrefEnum
 *
 *  @param pref:	IN:  IN which preference to retrieve (refer HsPrefEnum)
 *  @param bufP:	IN:  IN buffer to store preference (refer HsPrefEnum for the size of the buffer needed).
 *  @param prefSizeP:	IN:  IN/OUT on entry, size of bufP on exit, size of stored pref. If larger than size of
 * 			     bufP, then hsErrBufferTooSmall is returned.
 *  @retval Err 0 if no err
 *	        hsErrInvalidParam if pref is invalid
 *	        hsErrBufferTooSmall if *prefSizeP was smaller than actual pref
 **/
Err
HsPrefGet (UInt16 pref /*HsPrefEnum*/, void* bufP,
					UInt32* prefSizeP)
				SYS_SEL_TRAP (sysTrapHsSelector, hsSelPrefGet);

/**
 *  @brief HsExtensions maintains and utilizes a set of preferences that are not a part of the
 *	   Palm OS system preferences. They can be set using this function. @see HsPrefEnum
 *
 *  @param pref:	IN:  which pref to retrieve, a HsPrefEnum
 *  @param bufP:	IN:  buffer to store pref
 *  @param prefSize:	IN:  size of pref to store
 *  @retval Err 0 if no err
 *              hsErrInvalidParam if pref is invalid
 **/
Err
HsPrefSet (UInt16 pref /*HsPrefEnum*/, void* bufP,
					UInt32 prefSize)
				SYS_SEL_TRAP (sysTrapHsSelector, hsSelPrefSet);

/**
 *  @brief Function that gets the reference number to the phone library. It decides whether it
 *	   should return the GSM or the CDMA library based on the type of device. If the
 *	   library is not loaded, it loads the library.
 *
 *  @param refNum:	IN:  Library reference number to be used in other function.
 *  @retval Err 0 if no error.
 **/
Err
HsGetPhoneLibrary (UInt16 * refNum)
				SYS_SEL_TRAP (sysTrapHsSelector, hsSelGetPhoneLibrary);

/**
 *  @brief Tell the system that a gadget on the form is a certain type of status gadget so that
 *	   the system can properly draw and update it.
 *
 *  Tell the system that a gadget on the form is a certain type of status gadget so that
 *  the system can properly draw and update it.
 *  A form can only have one gadget for each status type. If more than one gadget is
 *  set to a certain type, only the last one set will be recorded. This function can also be
 *  used to erase and release a gadget whose type has already been set. To do this, type
 *  should be set to 0.
 *
 *  NOTE The gadget dimensions in the current implementation are hardcoded to
 *  values of 10 X 12 pixels for the battery gadget and 16 X 12 pixels for signal gadget.
 *  Scaling is not supported, so define your gadget resources accordingly.
 *
 *  @param frmP:	IN:  pointer to the form with the gadget (we will convert to an ID before saving but
 *			     want a ptr to be passed as a param to ensure that the caller is setting the gadget
 *			     for a loaded form)
 *  @param gadgetID:	IN:  id of the gadget whose type is being specified
 *  @param type:	IN:  type of the gadget (pass a member of HsStatusGadgetTypeEnum—defined in
 *		    	     HsExt.h or 0 to erase and release a gadget)
 *  @retval Err 0 if no error.
 **/
Err
HsStatusSetGadgetType (void* frmP, UInt16 gadgetID,
								   UInt16 type /*HsStatusGadgetTypeEnum*/)
				SYS_SEL_TRAP (sysTrapHsSelector, hsSelStatusSetGadgetType);

/**
 *  @brief Explicitly update all status gadgets on the current form.
 *
 **/
void
HsStatusUpdateGadgets (void)
				SYS_SEL_TRAP (sysTrapHsSelector, hsSelStatusUpdateGadgets);

/**
 *  @brief Extended version of GrfSetState to allow setting an option and option lock state.
 *
 *  The original GrfSetState does not have an autoShift parameter because it always
 *  assumed that anyone setting the shift state through the API call wanted an
 *  automatic shift. We do not make this assumption.
 *  The shift state can be set by calling either HsGrfSetStateExt() or GrfSetState().
 *  Either way, we have to make sure that both the keyboard shift state and the Graffiti
 *  engine’s shift state get set correctly.
 *
 *  @param capsLock:	IN:  Set to turn caps lock on
 *  @param numLock:	IN:  Set to true to turn num lock on
 *  @param optLock:	IN:  Set to true to turn option lock on
 *  @param upperShift:	IN:  Set to true to put into upper shift
 *  @param optShift:	IN:  Set to true to put into option shift
 *  @param autoShift:	IN:  Valid only if upperShift is true. Set to true if the graffiti engine should consider
 *			     the upper shift an auto shift.
 *  @retval Err Always returns 0.
 **/
Err
HsGrfSetStateExt (Boolean capsLock, Boolean numLock, Boolean optLock,
				  Boolean upperShift, Boolean optShift, Boolean autoShift)
				SYS_SEL_TRAP (sysTrapHsSelector, hsSelGrfSetStateExt);

/**
 *  @brief Extended version of GrfGetState to include detection of option and option lock state.
 *
 *  @param capsLockP:	IN:  returns true if caps lock on
 *  @param numLockP:	IN:  returns true if num lock on
 *  @param optLockP:	IN:  returns true if opt lock on
 *  @param tempShiftP:	IN:  returns current temporary shift
 *  @param autoShiftedP:	IN:  returns TRUE if shift not set by the user but by the system, for example, at the
 *				     beginning of a line.
 *  @retval Err Always returns 0.
 **/
Err
HsGrfGetStateExt (Boolean* capsLockP, Boolean* numLockP, Boolean* optLockP,
				  UInt16* tempShiftP, Boolean* autoShiftedP)
				SYS_SEL_TRAP (sysTrapHsSelector, hsSelGrfGetStateExt);

/**
 *  @brief Places a UI object to the right of the form title. This function is useful for adjusting
 * 	   object positions for localized titles.
 *
 *  @param voidFrmP:	IN:  ptr to form containing the object
 *  @param objID:	IN:  id of the object being positioned
 *  @param titleOffset:	IN:  number of pixels between title bar and status gadget
 *  @retval Boolean True if the object was moved from it’s original position.
 *  	    False otherwise
 **/
Boolean	HsPutObjectAfterTitle (void* voidFrmP, UInt16 objID, UInt8 titleOffset)
				SYS_SEL_TRAP (sysTrapHsSelector, hsSelPutObjectAfterTitle);


//HsInfo, HsEvtMetaEvent and HsSpringboardNotSupported are nop functions that are
//included for backwards compatibility
/**
 *  @brief This call will return info on a particular attribute of the Handspring extensions.
 *
 *  This call is designed to return info that won’t fit into a Palm OS feature call (a
 *  UInt32).
 *
 *  @param item:	IN:  which info to return
 *  @param paramSize:	IN:  size of paramP block passed in
 *  @param paramP:	IN:  info is returned here
 *  @retval UInt32 0 if no error
 **/
UInt32
HsInfo (UInt16 item, UInt16 paramSize,  void* paramP)
				SYS_SEL_TRAP (sysTrapHsSelector, hsSelInfo);

/**
 *  @brief
 *
 *  @param eventP:	IN:
 *  @retval UInt16 Error code.
 **/
UInt16
HsEvtMetaEvent (EventPtr eventP)
				SYS_SEL_TRAP (sysTrapHsSelector, hsSelEvtMetaEvent);

/**
 *  @brief Used to determine if the device supports the Springboard expansion slot or not.
 *
 *  On springboard devices, this function is unimplemented. To determine whether
 *  the device has a Springboard extension slot or not, use HsGetTrapAddress() to
 *  obtain the trap addresses of this function and HsUnimplemented. If they are equal,
 *  the device has the Springboard extension slot, otherwise it doesn’t.
 *
 *  @retval Err All of the Springboard functions return an error on the Treo 600.
 **/
Err
HsSpringboardNotSupported (void)
				SYS_SEL_TRAP (sysTrapHsSelector, hsSelSpringboardNotSupported);

/**
 *  @brief Works like SysGetTrapAddress, but for Handspring trap numbers.
 *
 *  For every HsExt function implemented in ARM native, there exists a wrapper 68K
 *  function that simply calls the HsExt function in question. This may seem
 *  roundabout, but HsGetTrapAddress is retained for backward compatibility.
 *
 *  @param trapNum:	IN:  the address of the trap handler for the given selector.
 *  @retval void* If the selector is invalid, returns the address of HsUnimplemented().
 **/
void*
HsGetTrapAddress (UInt16 trapNum)
				SYS_SEL_TRAP (sysTrapHsSelector, hsSelHsGetTrapAddress);

// Disconnects all open NetLib connections.  ***IMPORTANT*** This function MUST
//  be called only from the context of the UI task!
/**
 *  @brief This function will completely close the currently active TCP/IP network
 *	   connection. The open count will be 0 after this call is made.
 *
 *  @retval Err 0 if successful.
 **/
Err
HsNetworkDropConnection (void)
				SYS_SEL_TRAP (sysTrapHsSelector, hsSelNetworkDropConnection);


//will display a fatal error message since this trap number will not be handled by
//the trap dispatcher
/**
 *  @brief
 *
 **/
void
HsUnimplemented (void)
				SYS_SEL_TRAP (sysTrapHsSelector, hsSelUnimplemented);


// Returns the pointer to the ARM form. We need this because the 68K
// form might be deleted.
/**
 *  @brief
 *
 **/
void*
HsGetARMActiveFormPtr (void)
                SYS_SEL_TRAP (sysTrapHsSelector, hsSelGetARMActiveFormPtr);

/**
 *  @brief Allows the user to select a small or a large font size.
 *
 *  This is just like the Palm OS routine FontSelect, except it gives the user a choice of
 *  only a small and a large font. If the originalFont is not bold, then the two choices
 *  are stdFont and largeFont. If the original choice is bold, then the two choices are
 *  boldFont and largeBoldFont.
 *
 *  @param originalFont:	IN:  The original font ID (used to select the current font).
 *  @retval FontID The new font ID.
 **/
FontID
HsTwoFontSelect(FontID originalFont)
                SYS_SEL_TRAP (sysTrapHsSelector, hsSelTwoFontSelect);

/**
 *  @brief Opens a window with the number filled in the address text field.
 *
 *  This function can be used by any application that wants to dial a phone number,
 *  but first give the user the option of editing the number. This functionality
 *  originally existed in the STE, but was moved to the system.
 *
 *  @param numberP:	IN:  A pointer to a string that contains the phone number to put into the number field.
 *  @param nameP:	IN:  A pointer to a string that contains the name to put into the title of the dialog.
 *  @param callerID:	IN:  If this is true, then we will attempt to match the phone number with an entry in
 *  			     the address book. If a match is found, then the title of the dialog will contain the name.
 **/
void
HsOpenDialNumberDialog(char* numberP, char* nameP, Boolean callerID)
                SYS_SEL_TRAP (sysTrapHsSelector, hsSelOpenDialNumberDialog);

/**
 *  @brief Turn the radio (cell phone) on if it’s off.
 *
 *  @retval Err 0 if successful.
 **/
Err
HsTurnRadioOn(void)
                SYS_SEL_TRAP (sysTrapHsSelector, hsSelTurnRadioOn);

/**
 *  @brief Launches the messaging application (SMS) to the new message form with some
 *	   fields optionally prefilled with the specified values.
 *
 *  This function can be used by any application that wants to launch the messaging
 *  application and have some of the fields prefilled without using the helper APIs.
 *
 *  @param addressP:	IN:  A pointer to a string that contains the phone number to put into the To: field. (optional)
 *  @param nameP:	IN:  A pointer to a string that contains the name to put above the To: field. (optional)
 *  @param msgP:	IN:  A pointer to a string that contains the message text of the new message. (optional)
 *  @retval Err 0 on success or the error that occurred.
 **/
Err
HsCreateNewMessage(char* addressP, char* nameP, char* msgP)
                SYS_SEL_TRAP (sysTrapHsSelector, hsSelCreateNewMessage);


/**
 *  @brief Launches the email application to the new email form with some fields optionally
 *	   prefilled with the specified values.
 *
 *  This function can be used by any application that wants to launch the email
 *  application and have some of the fields prefilled without using the helper APIs.
 *
 *  @param addressP:	IN:  A pointer to a string that contains the address to put in the To: field.
 *  @param ccP:		IN:  A pointer to a string that contains the address to put in the cc: field. (optional)
 *  @param subjectP:	IN:  A pointer to a string that contains the subject of the new email. (optional)
 *  @param msgP:	IN:  A pointer to a string that contains the message text of the new email. (optional)
 *  @retval Err 0 on success or the error that occurred.
 **/
Err
HsCreateNewEmail(char* addressP, char* ccP, char* subjectP, char* msgP)
                SYS_SEL_TRAP (sysTrapHsSelector, hsSelCreateNewEmail);

/**
 *  @brief Launches the browser to the specified URL.
 *
 *  This function can be used by any application that wants to launch the web browser
 *  to a specified URL without using the helper APIs.
 *
 *  @param urlP:	IN:  A pointer to a string that contains the phone number to put into the number field.
 *  @retval Err 0 on success or the error that occurred.
 **/
Err
HsBrowseUrl(char* urlP)
                SYS_SEL_TRAP (sysTrapHsSelector, hsSelBrowseUrl);

/**
 *  @brief If a form has a system drawn focus ring, gets info about the ring
 *
 *  @param formP:	IN:  Ptr to form whose ring info is to be obtained
 *  @param objectIDP:	IN:  If not NULL, on exit will point to the object with the system-drawn focus ring
 *  @param extraInfoP:	IN:  If not NULL, on exit will point to any extra info associated with the focus ring.
 *			     Extra info is any info that the caller wants to associate with a ring.
 *			     hsNavFocusRingNoExtraInfo (0xFFFF) is used to represent no extra info.
 *  @param boundsInsideRingP:	IN:  If not NULL, on exit will point to the bounds inside the system-drawn focus ring
 *  @param ringStyleP:	IN:  If not NULL, on exit will point to the style of the focus ring
 *  @retval Err Error code.
 **/
Err
HsNavGetFocusRingInfo (const FormType* formP, UInt16* objectIDP,
					   Int16* extraInfoP, RectangleType* boundsInsideRingP,
					   HsNavFocusRingStyleEnum* ringStyleP)
				SYS_SEL_TRAP (sysTrapHsSelector, hsSelNavGetFocusRingInfo);
/**
 *  @brief Tells the system to draw a focus ring for an object
 *
 *  @param formP:	IN:  Form containing object whose ring is being drawn
 *  @param objectID:	IN:  ID of object getting the focus ring
 *  @param extraInfo:	IN:  Any info that caller wants associated with the focus ring. For example, the
 *			     system uses this field to store the temporarily-selected item of a focused,
 *			     embedded list. Caller should pass hsNavFocusRingNoExtraInfo (0xFFFF) if there
 *			     is no associated info.
 *  @param boundsInsideRingP:	IN:  Points to the bounds around which the ring should be drawn
 *  @param ringStyle:	IN:  Enumeration value that represents the desired style for the ring.
 *			     hsNavFocusRingStyleObjectTypeDefault can be passed if caller just wants the
 *			     default ring style for the object’s type.
 *  @param forceRestore:	IN:  If ring is already on the object, whether to force restoration of what’s behind the
 *				     ring before redrawing the ring.
 *  @retval Err Returns errNone if no error, error code otherwise.
 **/
Err
HsNavDrawFocusRing (FormType* formP, UInt16 objectID, Int16 extraInfo,
				    RectangleType* boundsInsideRingP,
					HsNavFocusRingStyleEnum ringStyle, Boolean forceRestore)
				SYS_SEL_TRAP (sysTrapHsSelector, hsSelNavDrawFocusRing);

/**
 *  @brief Removes system-drawn ring from around currently focused object
 *
 *  @param formP:	IN:  Ptr to form containing the object with the ring
 *  @retval Err Returns errNone if no error, error code otherwise
 **/
Err
HsNavRemoveFocusRing (FormType* formP)
				SYS_SEL_TRAP (sysTrapHsSelector, hsSelNavRemoveFocusRing);

/**
 *  @brief Sends a frmObjectFocusTake event for an object
 *
 *  @param formP:	IN:  Ptr to form that contains the object
 *  @param objID:	IN:  ID of the object
 **/
void
HsNavObjectTakeFocus (const FormType* formP, UInt16 objID)
				SYS_SEL_TRAP (sysTrapHsSelector, hsSelNavObjectTakeFocus);

/**
 *  @brief Obtains a focus color
 *
 *  @param color:	IN:  Enumeration value that specifies which focus color to get
 *  @param rgbColorP:	IN:  On exit, points to the focus color
 **/
void
HsNavGetFocusColor (HsNavFocusColorEnum color, RGBColorType* rgbColorP)
				SYS_SEL_TRAP (sysTrapHsSelector, hsSelNavGetFocusColor);

/**
 *  @brief Sets new focus color
 *
 *  @param color:	IN:  Enumeration value that specifies which focus color to set
 *  @param rgbColorP:	IN:  Points to the new color
 *  @param oldRgbColorP:	IN:  If not NULL, on exit it will point to the old focus color
 **/
void
HsNavSetFocusColor (HsNavFocusColorEnum color, RGBColorType* rgbColorP,
					RGBColorType* oldRgbColorP)
				SYS_SEL_TRAP (sysTrapHsSelector, hsSelNavSetFocusColor);

/**
 *  @brief Set or get the state of the LED or Vibrator.
 *
 *  @param count:	IN:  Count of how many times to blink or pulse the indicator, or one of the
 *		  	     HsIndicatorCountEnum values to either run forever or get the current state.
 *  @param indicatorType:	IN:  HsIndicatorTypeEnum (LED or vibrator)
 *  @param stateP:	IN:  Pointer to HsIndicatorStateEnum If setting, points to state to set. If getting, as
 *			     IN param points to “none” state of the source to query (or kIndicatorStateNull
 *			     to query whatever source has priority). As OUT param, contains the result of
 *			     the query (kIndicatorStateNull if querying for the active source and there is none)
 *  @retval Err 0 if no error
 **/
Err
HsIndicatorState (UInt16 count, UInt16 indicatorType, UInt16* stateP)
                SYS_SEL_TRAP (sysTrapHsSelector, hsSelIndicatorState);

/**
 *  @brief Get or set the current mode.
 *
 *  @param set:		IN:  If true, we’ll switch to *modeP; if false we’ll put the current mode in *modeP
 *  @param modeP:	IN:  If we’re setting, we’ll switch to this mode (refer HsLightModeEnum). If we’re
 *			     getting, we’ll put the current mode here.
 *  @retval Err An error code; 0 if no error.
 **/
Err
HsLightMode (Boolean set, UInt16* modeP)
                SYS_SEL_TRAP (sysTrapHsSelector, hsSelLightMode);

/**
 *  @brief Add/remove special lighting circumstances
 *
 *  This function does reference counting on added circumstances, so be sure to not
 *  add a circumstance without eventually removing it (unless you intended that).
 *  Note that some circumstances don’t take effect until some point in time later (in
 *  SysHandleEvent). That’s because some circumstances only take effect when the
 *  device is idle for some amount of time.
 *
 *  @param add:		IN:  If true, we’re adding a circumstance, If false, we’re removing it.
 *  @param circumstance:	IN:  The circumstance we’re adding or taking away (refer
 *			             HsLightCircumstanceEnum).
 *  @retval Err An error code; 0 if no error
 **/
Err
HsLightCircumstance (Boolean add, UInt16 circumstance)
                SYS_SEL_TRAP (sysTrapHsSelector, hsSelLightCircumstance);

/**
 *  @brief
 *
 **/
UInt16 HsCurrentLightCircumstance(void)
				SYS_SEL_TRAP (sysTrapHsSelector, hsGetCurrentLightCircumstance);

/**
 *  @brief Converts HsKeysPressed() results into a bitfield.
 *
 *  This function provides compatibility with earlier Handspring SDKs. The preferred
 *  method of querying key states is to call HsKeysPressed().
 *
 *  @param keys:	IN:  Array of three bitfields representing which keys are depressed.
 **/
void
HsKeyCurrentStateExt (UInt32 keys[3])
                SYS_SEL_TRAP (sysTrapHsSelector, hsSelKeyCurrentStateExt);

/**
 *  @brief Glue to HsKeyEnabled() based on bitfields.
 *
 *  This function provides compatibility with earlier Handspring SDKs. The preferred
 *  method of enabling keys is to call HsKeyEnableKey().
 *
 *  @param keyMaskNew:	IN:  new enabled mask
 *  @param keyMaskOld:	IN:  previous enabled mask
 **/
void
HsKeySetMaskExt (const UInt32 keyMaskNew[3], UInt32 keyMaskOld[3])
                SYS_SEL_TRAP (sysTrapHsSelector, hsSelKeySetMaskExt);

/**
 *  @brief Get the current state of specified keys.
 *
 *  This extends KeyCurrentState().
 *
 *  @param count:	IN:  number of elements in keyCodes[] and pressed[]
 *  @param keyCodes[]:	IN:  Array of keyCodes to test
 *  @param pressed[]:	IN:  Each element is set to true if the key specified by the corresponding element in
 *			     keyCodes[] is pressed, and set to false if it’s not pressed. It is acceptable to pass
 *			     NULL if only the return value is of interest.
 *  @retval UInt16 Count of keys in keyCodes[] that are pressed
 **/
UInt16
HsKeysPressed (UInt16 count, const UInt16 keyCodes[], Boolean pressed[])
                SYS_SEL_TRAP (sysTrapHsSelector, hsSelKeysPressed);

/**
 *  @brief Stop a key from sending any more autorepeat events until it is released.
 *
 *  @param keyCode:	IN:  key to stop
 *  @retval Err errNone if the keyCode is the last key pressed and it’s still held.
 **/
Err
HsKeyStop (UInt16 keyCode)
                SYS_SEL_TRAP (sysTrapHsSelector, hsSelKeyStop);

/**
 *  @brief Enable/disable a key from generating key events.
 *
 *  @param keyCode:	IN:  which key to enable/disable
 *  @param enabled:	IN:  true to enable, false to disable
 *  @retval Boolean true if previously enabled.
 **/
Boolean
HsKeyEnableKey (UInt16 keyCode, Boolean enabled)
                SYS_SEL_TRAP (sysTrapHsSelector, hsSelKeyEnableKey);

/**
 *  @brief Determine which key generates a certain character.
 *
 *  @param chrCode:	IN:  the character to search for
 *  @retval UInt16 The key code.
 **/
UInt16
HsKeyChrCodeToKeyCode (UInt16 chrCode)
                SYS_SEL_TRAP (sysTrapHsSelector, hsSelKeyChrCodeToKeyCode);

/**
 *  @brief Translate a key code to the character generated by that key.
 *
 *  @param keyCode:	IN:  The key to lookup
 *  @param modifiersIn:	IN:  Modifier mask indicating which character should be returned
 *  @param chrP:	IN:  The character produced by the key
 *  @param modifiersOutP:	IN:  The keyMask bits that must be set if a keyDown event is formed with the
 *				     character. (Not including the bits set in modifiersIn)
 **/
void
HsKeyKeyCodeToChrCode (UInt16 keyCode, UInt16 modifiersIn,
					   UInt16* chrP, UInt16* modifiersOutP)
                SYS_SEL_TRAP (sysTrapHsSelector, hsSelKeyKeyCodeToChrCode);

/**
 *  @brief Determine if an event came from the physical keyboard.
 *
 *  @param eventP:	IN:  The event to check
 *  @retval Boolean True if the event came from the keyboard
 **/
Boolean
HsKeyEventIsFromKeyboard (EventPtr eventP)
                SYS_SEL_TRAP (sysTrapHsSelector, hsSelKeyEventIsFromKeyboard);

/**
 *  @brief Displays a dialog to the user allowing them to confirm the saving of data, and the
 *	   name that they wish to assign.
 *
 *  @param name:	IN:  Comes in as the default name and returns the name the user chose
 *  @retval Boolean True if the user confirmed that the save should take place.
 *	    False otherwise.
 **/
Boolean
HsExtDoSaveAsDialog (Char * name)
                SYS_SEL_TRAP (sysTrapHsSelector, hsSelDoSaveAsDialog);

/**
 *  @brief
 *
 *  @param inSrcP:	IN:
 *  @param inSrcLen:	IN:
 *  @param outDstP:	IN:
 *  @param inDstSize:	IN:
 *  @retval UInt16
 **/
UInt16
HsTxtPrepFindString(const Char* inSrcP, UInt16 inSrcLen,
						Char* outDstP, UInt16 inDstSize)
                SYS_SEL_TRAP (sysTrapHsSelector, hsSelTxtPrepFindString);

// These are convenient multi-byte friendly text routines
/**
 *  @brief
 *
 *  @param textP:	IN:
 *  @retval UInt32
 **/
UInt32
HsTxtNumChars(Char const* textP)
                SYS_SEL_TRAP (sysTrapHsSelector, hsSelTxtNumChars);

/**
 *  @brief
 *
 *  @param textP:	IN:
 *  @param maxPixels:	IN:
 *  @param keepLabel:	IN:
 **/
void
HsTxtTruncateString(Char* textP, UInt16 maxPixels, Boolean keepLabel)
                SYS_SEL_TRAP (sysTrapHsSelector, hsSelTxtTruncateString);

/**
 *  @brief
 *
 *  @param textP:	IN:
 *  @param textLen:	IN:
 *  @retval Boolean
 **/
Boolean
HsTxtIsAscii(Char* textP, UInt16 textLen)
                SYS_SEL_TRAP (sysTrapHsSelector, hsSelTxtIsAscii);

/**
 *  @brief
 *  @retval Boolean
 **/
Boolean
HsIsHTTPLibraryLoaded (void)
		 		SYS_SEL_TRAP (sysTrapHsSelector, hsSelIsHTTPLibraryLoaded);

/**
 *  @brief Obtains the temporarily selected item of an embedded list
 *
 *  @param formP:	IN:  Pointer to the form containing the list whose temporary selection is to be obtained
 *  @param listID:	IN:  ID of list whose temporary selection is to be obtained
 *  @retval Int16 Returns the temporarily selected list item
 **/
Int16
HsNavLstGetTempSelection (const FormType* formP, UInt16 listID)
				SYS_SEL_TRAP (sysTrapHsSelector, hsSelNavLstGetTempSelection);

/**
 *  @brief Sets the temporarily selected item for an embedded list
 *
 *  @param formP:	IN:  Pointer to the form containing the list whose temporary selection is to be set
 *  @param listID:	IN:  ID of list whose temporary selection is to be set
 *  @param itemNum:	IN:  Number of the item that should be temporarily selected
 **/
void
HsNavLstSetTempSelection (const FormType* formP, UInt16 listID, Int16 itemNum)
				SYS_SEL_TRAP (sysTrapHsSelector, hsSelNavLstSetTempSelection);

#ifdef __cplusplus
}
#endif

#endif	  // __HS_EXT_68K_H__
