/******************************************************************************
 * Copyright (c) 2005 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/**
 *
 * @ingroup SystemDef
 *
 */
 
/**
 * @file 	HsExtTraps.h
 * @version 
 * @date   
 *
 * @brief   Definitions of selectors used for the 68K trap based
 *	    calls to HsExtensions
 *	
 */



#ifndef	  __HS_EXT_TRAPS_H__
#define	  __HS_EXT_TRAPS_H__


#ifndef sysTrapOEMDispatch
  /// OEM trap in Palm OS 3.0 and later trap table (formerly sysTrapSysReserved4)
  #define sysTrapOEMDispatch			  0xA349		/**< 		*/
#endif // !sysTrapOEMDispatch

/**
 * @name This is the system trap that we base all HsExt calls off of.
 *
 */
/*@{*/
#define	sysTrapHsSelector				  sysTrapOEMDispatch	/**< 		*/
/*@}*/


#define	hsSelectorBase					  0		/**< 		*/

  #define hsSelInfo 						  0x0	/**< 		*/

#ifndef HS_STRIP_UNIMPLEMENTED	// not implemented on 5.x yet...
  #define hsSelPrvInit 					  0x1		/**< 		*/
  #define hsSelPrvCallSafely 				  0x2		/**< 		*/
  #define hsSelPrvCallSafelyNewStack 		  0x3			/**< 		*/

  #define hsSelDatabaseCopy 				  0x4		/**< 		*/
  #define hsSelExtKeyboardEnable 			  0x5		/**< 		*/
  #define hsSelCardAttrGet 				  0x6		/**< 		*/
  #define hsSelCardAttrSet 				  0x7		/**< 		*/
  #define hsSelCardEventPost 				  0x8		/**< 		*/
  #define hsSelPrvErrCatchListP 			  0x9		/**< 		*/
#endif

#define hsSelPrefGet 					  0xA		/**< 		*/
#define hsSelPrefSet 					  0xB		/**< 		*/

#ifndef HS_STRIP_UNIMPLEMENTED	// not implemented on 5.x yet...
  #define hsSelDmGetNextDBByTypeCreator	  0xC				/**< 		*/
  #define hsSelDmGetNextDBInit 			  0xD			/**< 		*/
	
  #define hsSelCardHdrUpdate 				  0xE		/**< 		*/

  #define hsSelAppEventHandlerSet 		  0xF			/**< 		*/
  #define hsSelAppEventPost 				  0x10		/**< 		*/

  #define hsSelUsbCommStatePtr 			  0x11			/**< 		*/

  #define hsSelCardPatchInstall 			  0x12		/**< 		*/
  #define hsSelCardPatchRemove 			  0x13			/**< 		*/

  #define hsSelEvtResetAutoOffTimer 		  0x14			/**< 		*/

  #define hsSelDmDatabaseUniqueIDSeed		  0x15			/**< 		*/
#endif

#define hsSelAboutHandspringApp 		  0x16			/**< 		*/

#ifndef HS_STRIP_UNIMPLEMENTED	// not implemented on 5.x yet...
  #define hsSelDmDatabaseIsOpen 			  0x17		/**< 		*/
  #define hsSelDmDatabaseIsProtected 		  0x18			/**< 		*/

  #define hsSelDlkForceSlowSync 			  0x19		/**< 		*/

  #define hsSelPrvHandleCardChangeEvent	  0x1A				/**< 		*/

  // 0x03523000

  #define hsSelCardPower 				  0x1B		/**< 		*/

  #define	hsSelDmDatabaseDeleted			  0x1C		/**< 		*/

  #define	hsSelDmLockFileSystem			  0x1D		/**< 		*/

  #define	hsSelPrvLaunchCompareFunc		  0x1E		/**< 		*/

  // 0x03523020

  #define hsSelPrvLEDCommand				  0x1F		/**< 		*/

  #define hsInstallSerialWrapper			  0x20		/**< 		*/

  #define hsUninstallSerialWrapper		  0x21			/**< 		*/

  #define hsPeriodicRegister				  0x22		/**< 		*/
  #define hsPeriodicUnregister			  0x23			/**< 		*/
  #define hsPeriodicPeriodsAvailable		  0x24			/**< 		*/

  // 0x03523030

  // No HsExtension calls added

  // 0x03523040

  // NOTE: Be sure to update 68000Instrs.h if these change
  #define hsSelPrvSetButtonDefault		  0x25			/**< 		*/
  #define hsSelPrvFindAppFromIDs			  0x26		/**< 		*/
#endif

#define	hsSelAttrGet					  0x27		/**< 		*/
#define	hsSelAttrSet					  0x28		/**< 		*/

#define hsSelKeyCurrentStateExt			  0x29			/**< 		*/
#define hsSelKeySetMaskExt				  0x2A		/**< 		*/

#define hsSelGrfSetStateExt				  0x2B		/**< 		*/
#define hsSelGrfGetStateExt				  0x2C		/**< 		*/

#ifndef HS_STRIP_UNIMPLEMENTED	// not implemented on 5.x yet...
  #define hsSelKeyChrToOptChr				  0x2D		/**< 		*/
  #define hsSelKeyChrToRegChr				  0x2E		/**< 		*/
  #define hsSelPrvKeyHandleEvent			  0x2F		/**< 		*/
  #define hsSelWordCorrectInvoke			  0x30		/**< 		*/
  #define hsSelWordCorrectUndo			  0x31			/**< 		*/
  #define hsSelPrvUpdateBatteryGadget		  0x32			/**< 		*/
  #define hsSelPrvUpdateSignalGadget		  0x33			/**< 		*/
#endif	

#define hsSelStatusSetGadgetType		  0x34			/**< 		*/

#ifndef HS_STRIP_UNIMPLEMENTED	// not implemented on 5.x yet...
  #define hsSelSysKeyboardReferenceDialog	  0x35			/**< 		*/
#endif

#define hsSelPutObjectAfterTitle		  0x36			/**< 		*/

#define hsSelEvtMetaEvent				  0x37		/**< 		*/

#define hsSelStatusUpdateGadgets		  0x38			/**< 		*/

#ifndef HS_STRIP_UNIMPLEMENTED		// not implemented on 5.x yet...
  #define hsSelLstPopupListExt			  0x39			/**< 		*/
  #define hsSelDmCreateDatabasesFromImages  0x3A	  		/**< DO NOT CHANGE used in object patch! */
  #define hsSelPrvLaunchDemoApp			  0x3B	 		/**< DO NOT CHANGE used in object patch! */
  #define hsSelPrefGetAppKeyCreator		  0x3C			/**< 		*/
#endif

#define hsSelUtilFrmDoDialogWithCallback  0x3D				/**< 		*/

  #define hsSelHsGetTrapAddress			  0x3E			/**< 		*/


#ifndef HS_STRIP_UNIMPLEMENTED	// not implemented on 5.x yet...
  #define hsSelHsSetTrapAddress			  0x3F			/**< 		*/
#endif

  #define hsSelUnimplemented				  0x40		/**< 		*/

#ifndef HS_STRIP_UNIMPLEMENTED	// not implemented on 5.x yet...
  #define hsSelIndicator					  0x41	/**< 		*/
#endif

  #define hsSelUtilSclUpdateScrollBar		  0x42			/**< 		*/
  #define hsSelUtilSclScroll				  0x43		/**< 		*/
  #define hsSelUtilSclScrollPage			  0x44		/**< 		*/

#ifndef HS_STRIP_UNIMPLEMENTED	// not implemented on 5.x yet...
  #define hsSelPrvInitPostProcess			  0x45		/**< 		*/
  #define hsSelPostProcPopupList			  0x46		/**< 		*/

  #define hsSelPeriodicUnregisterFromSelf	  0x47			/**< 		*/
#endif

  #define hsSelNetworkDropConnection		  0x48			/**< 		*/

#ifndef HS_STRIP_UNIMPLEMENTED	// not implemented on 5.x yet...
  #define hsSelPrvSetupGoToCustomForm			  0x49	  	/**< DO NOT CHANGE used in object patch! */
  #define hsSelPrvSetupInstallFormEventHandler  0x4A	  		/**< DO NOT CHANGE used in object patch! */

#endif
  #define hsSelUtilSliHandleKeyDown		  0x4B			/**< 		*/
  #define hsSelUtilWinGetMaxDepth			  0x4C		/**< 		*/

#define hsSelLightMode					  0x4D		/**< 		*/
#define hsSelLightCircumstance			  0x4E			/**< 		*/

#define hsSelUtilBbutInstallFromResource  0x4F				/**< 		*/
#define hsSelUtilBbutHitBigButton		  0x50			/**< 		*/
#define hsSelUtilFrmDoDialog			  0x51			/**< 		*/

#define hsSelUtilStrCSpn				  0x52		/**< 		*/
#define hsSelUtilFrmDoTimedDialog         0x53				/**< 		*/

// NOTE: Please use up reserved spaces during
// 3.5H5 development; this will reduce merge
// issues...

#ifndef HS_STRIP_UNIMPLEMENTED	// not implemented on 5.x yet...
  #define hsSelReserved352H5_54			  0x54			/**< 		*/
  #define hsSelReserved352H5_55			  0x55			/**< 		*/
  #define hsSelReserved352H5_56			  0x56			/**< 		*/
  #define hsSelReserved352H5_57			  0x57			/**< 		*/
  #define hsSelReserved352H5_58			  0x58			/**< 		*/
  #define hsSelReserved352H5_59			  0x59			/**< 		*/
  #define hsSelReserved352H5_5A			  0x5A			/**< 		*/
  #define hsSelReserved352H5_5B			  0x5B			/**< 		*/
  #define hsSelReserved352H5_5C			  0x5C			/**< 		*/
  #define hsSelReserved352H5_5D			  0x5D			/**< 		*/

  // NOTE: Please use up reserved spaces during
  // 3.5H6 development; this will reduce merge
  // issues...

  #define hsSelReserved352H6_5E			  0x5E			/**< 		*/
  #define hsSelReserved352H6_5F			  0x5F			/**< 		*/
  #define hsSelReserved352H6_60			  0x60			/**< 		*/
  #define hsSelReserved352H6_61			  0x61			/**< 		*/
  #define hsSelReserved352H6_62			  0x62			/**< 		*/
  #define hsSelReserved352H6_63			  0x63			/**< 		*/
  #define hsSelReserved352H6_64			  0x64			/**< 		*/
  #define hsSelReserved352H6_65			  0x65			/**< 		*/
  #define hsSelReserved352H6_66			  0x66			/**< 		*/
  #define hsSelReserved352H6_67			  0x67			/**< 		*/
  #define hsSelReserved352H6_68			  0x68			/**< 		*/
  #define hsSelReserved352H6_69			  0x69			/**< 		*/
  #define hsSelReserved352H6_6A			  0x6A			/**< 		*/
#endif

// 0x04103010
#ifndef HS_STRIP_UNIMPLEMENTED	// not implemented on 5.x yet...
  #define hsSelPrvShowKeyboardLockOnDialog  0x6B			/**< 		*/
  #define hsSelPrvHandleKeyboardLock		  0x6C			/**< 		*/
#endif

  #define hsSelSpringboardNotSupported	  0x6D				/**< 		*/

// Reserve some traps for pre-OS5 development
#ifndef HS_STRIP_UNIMPLEMENTED	// not implemented on 5.x yet...
  #define hsSelReserved41_6E                0x6E			/**< 		*/
  #define hsSelReserved41_6F                0x6F			/**< 		*/
  #define hsSelReserved41_70                0x70			/**< 		*/
  #define hsSelReserved41_71                0x71			/**< 		*/
  #define hsSelReserved41_72                0x72			/**< 		*/
#endif

/**
 * @name OS-5 
 *
 */
/*@{*/
#define hsSelUtilGetObjectUsable          0x73				/**< 		*/
#define hsSelUtilFrmGetDefaultButtonID    0x74				/**< 		*/
#define hsSelUtilFrmGetEventHandler       0x75				/**< 		*/
#define hsSelUtilLstGetItemsText          0x76				/**< 		*/
#define hsSelUtilLstGetTopItem            0x77				/**< 		*/
#define hsSelUtilCtlGetControlStyle       0x78				/**< 		*/
/*@}*/

/**
 * @name Trace Functions
 *
 */
/*@{*/
#define hsSelTraceLevelsSet				  0x79		/**< 		*/
#define hsSelTraceLevelsGet				  0x7A		/**< 		*/
#define hsSelTrace						  0x7B	/**< 		*/
#define hsSelTraceData					  0x7C		/**< 		*/
#define hsSelTraceRecordsDump			  0x7D			/**< 		*/
#define hsSelTraceClear					  0x7E		/**< 		*/
#define hsSelTraceOutputLevelsSet		  0x7F			/**< 		*/
#define hsSelTraceOutputLevelsGet		  0x80			/**< 		*/
#define hsSelTraceText					  0x81		/**< 		*/
#define hsSelTraceFunction				  0x82		/**< 		*/
#define hsSelTraceError					  0x83		/**< 		*/
#define hsSelTraceFunctionError			  0x84			/**< 		*/
#define hsSelTraceEvent					  0x85		/**< 		*/
/*@}*/

#define hsSelGetPhoneLibrary              0x86				/**< 		*/
#define hsSelUtilFrmSetLabel			  0x87			/**< 		*/
#define hsSelUtilCtlGetFont				  0x88		/**< 		*/

/**
 * @name Mutex functions -- 68K bridge to ARM-native functions
 *
 */
/*@{*/
#define hsSelUtilMutexCreate			  0x89 			/**< 		*/
#define hsSelUtilMutexDelete			  0x8A			/**< 		*/
#define hsSelUtilMutexReserve			  0x8B			/**< 		*/
#define hsSelUtilMutexRelease			  0x8C			/**< 		*/
/*@}*/

#define hsSelGetARMActiveFormPtr          0x8E				/**< 		*/

#define hsSelTwoFontSelect				  0x8F		/**< 		*/
#define hsSelOpenDialNumberDialog		  0x90			/**< 		*/
#define hsSelTurnRadioOn				  0x91		/**< 		*/
#define hsSelCreateNewMessage			  0x92			/**< 		*/
#define hsSelCreateNewEmail				  0x93		/**< 		*/
#define hsSelBrowseUrl					  0x94		/**< 		*/

#define hsSelNavGetFocusColor			  0x95			/**< 		*/
#define hsSelNavSetFocusColor			  0x96			/**< 		*/
#define hsSelNavGetFocusRingInfo		  0x97			/**< 		*/
#define hsSelNavDrawFocusRing			  0x98			/**< 		*/
#define hsSelNavRemoveFocusRing			  0x99			/**< 		*/
#define hsSelNavLstGetTempSelection		  0x9A			/**< 		*/
#define	hsSelNavLstSetTempSelection		  0x9B			/**< 		*/
// Trap defined at the end of the list
//#define hsSelNavObjectTakeFocus			  0xB7

#define hsSelIndicatorState				  0x9C		/**< 		*/

#define hsSelKeysPressed				  0x9D		/**< 		*/
#define hsSelKeyEnableKey				  0x9E		/**< 		*/
#define hsSelKeyChrCodeToKeyCode		  0x9F			/**< 		*/
#define hsSelKeyKeyCodeToChrCode		  0xA0			/**< 		*/
#define hsSelKeyEventIsFromKeyboard		  0xA1			/**< 		*/

#define hsSelTraceContextInit             0xA2				/**< 		*/
#define hsSelTraceContextDelete           0xA3				/**< 		*/

#define hsSelDoSaveAsDialog               0xA4				/**< 		*/
#define hsSelExgSelectTarget              0xA5				/**< 		*/
#define hsSelExgLocalAccept               0xA6				/**< 		*/
#define hsSelExgLocalDisconnect           0xA7				/**< 		*/

#define hsSelUtilAttnNagInfo			  0xA8			/**< 		*/

#define hsSelCrashLogGetLast			  0xA9			/**< 		*/
#define hsSelCrashLogDisplayLast		  0xAA			/**< 		*/

#define hsSelTxtPrepFindString			  0xAB			/**< 		*/

#define hsSelUtilFrmSetHelpID			  0xAC			/**< 		*/
#define hsSelUtilFrmSetLabelFont		  0xAD			/**< 		*/

// NOTE: This trap selector was never implemented and never used, therefore it is safe to be reassigned
#define hsNeverUsedBeforeAvailableForUse1 0xAE				/**< 		*/

/**
 * @name Additional Trace Utility functions
 *
 */
/*@{*/
#define hsSelTraceOpenDB                  0xB0				/**< 		*/
#define hsSelTraceCloseDB                 0xB1				/**< 		*/
/*@}*/

#define hsSelSupressDbgLockout			  0xB2			/**< 		*/
#define hsSelIsDbgLockoutSupressed		  0xB3			/**< 		*/

#define hsSelKeyStop					  0xB4		/**< 		*/

#define hsSelGetVersionString			  0xB5			/**< 		*/

#define hsSelUtilEnableAttentionMgr		  0xB6			/**< 		*/

#define hsSelNavObjectTakeFocus			  0xB7			/**< 		*/

#define hsSelPostProcessPopupList		  0xB8			/**< 		*/

#define hsSelTraceSetMaxRecords			  0xB9			/**< 		*/
#define hsSelTraceGetMaxRecords			  0xBA			/**< 		*/

/**
 * @name multi-byte friendly Txt routines
 *
 */
/*@{*/
#define hsSelTxtNumChars				  0xBB		/**< 		*/
#define hsSelTxtTruncateString			  0xBC			/**< 		*/
#define hsSelTxtIsAscii					  0xBD		/**< 		*/
/*@}*/

#define hsSelWaitForGPRSAttach			  0xBE			/**< 		*/
#define hsSelIsHTTPLibraryLoaded          0xBF				/**< 		*/

/**
 * @name This range is reserved for last-minute fixes in OS-5H1-C/D/E
 *
 */
/*@{*/
#define hsSelReserved5H1_C0				  0xC0		/**< 		*/
#define hsSelReserved5H1_C1				  0xC1		/**< 		*/
#define hsSelReserved5H1_C2				  0xC2		/**< 		*/
#define hsSelReserved5H1_C3				  0xC3		/**< 		*/
/*@}*/

/**
 * @name New AcDc traps
 *
 */
/*@{*/
#define hsSelDispatchService			  0xC4			/**< 		*/

#define hsSelOpenDialNumberDialogExt  	  0xC5				/**< 		*/

// This trap is technically free.  It was temporarily used during Treo650 development but we never shipped
//	with it.  We continue to define and dispatch it just to be extra safe.  There's a small chance that
//	not all apps were properly transitioned to the PmUIUtil version of this call during Treo650 development.
//  3rd party apps should have never had access to this function because it is defined in a palmOne
//	internal header and apps built with the Treo650 ROM should've always be built against the latest trap
//	definitions.  There's the slight possibility that palmOne apps built as components, however, were
//	never built against the newest trap definitions.
#define hsSelAttnGetAttentionExt		  0xC6			/**< 		*/

#define hsSelUtilBbutInstallNewButton	  0xC7				/**< 		*/

#define hsSelUtilBbutSetLabel			  0xC8			/**< 		*/

// This trap is technically free.  It was temporarily used during Treo650 development but we never shipped
//	with it.  We continue to define and dispatch it just to be extra safe.  There's a small chance that
//	not all apps were properly transitioned to the PmUIUtil version of this call during Treo650 development.
//  3rd party apps should have never had access to this function because it is defined in a palmOne
//	internal header and apps built with the Treo650 ROM should've always be built against the latest trap
//	definitions.  There's the slight possibility that palmOne apps built as components, however, were
//	never built against the newest trap definitions.
#define hsSelAttnUpdateExt				  0xC9		/**< 		*/

#define hsSelUtilAttnIndicatorAllow		  0xCA			/**< 		*/

#define hsSelUtilBbutSetBitmap			  0xCB			/**< 		*/

#define hsSelMenuSetCurMenuCurItem		  0xCC			/**< 		*/

#define hsSelTraceSetCommitInterval		  0xCD			/**< 		*/
#define hsSelTraceGetCommitInterval		  0xCE			/**< 		*/

#define hsSelWriteROMTokens               0xCF				/**< 		*/
/*@}*/
	
// for application to get current light circumstance
#define hsGetCurrentLightCircumstance	  0xD0				/**< 		*/

// WARNING! Please do not re-order the trap numbers.
// If you need to add a new trap, add it to the bottom of the list!!

// WARNING!  Leave this one at the end! 
// When adding new traps, renumber it
// to one greater than the last trap.
#define hsSelLast  				  0xD1			/**< 		*/

#define	hsNumSels	 (hsSelLast - hsSelectorBase)			/**< 		*/


#endif	  // __HS_EXT_TRAPS_H__
