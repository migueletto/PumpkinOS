/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/** @ingroup Telephony
 *
 */


/**
 *
 * @file 	HsPhoneTraps.h
 *
 * @brief  Traps of all functions exported by the Phone Library
 *
 * NOTES:
 */



#ifndef HS_PHONE_TRAPS_H
#define HS_PHONE_TRAPS_H

#if (CPU_TYPE == CPU_68K)
#define PHN_LIB_TRAP(trapNum) SYS_TRAP(trapNum)
#endif


#define     PhnLibTrapGetLibAPIVersion   		(0+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapUninstall			 	(1+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapRegister			 	(2+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapFirstAppForService		(3+sysLibTrapCustom)    /**< GSM Only */
#define     PhnLibTrapNewAddress		 	(4+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapGetField			 	(5+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapGetID				(6+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapSetField                          (7+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapSetNumber                         (8+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapSetID                             (9+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapAddressToText                     (10+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapHasName                           (11+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapEqual                             (12+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapDial				(13+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapSendDTMF			 	(14+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapDisconnect			(15+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapActivate				(16+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapConference			(17+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapHold				(18+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapGetConnectionInfo			(19+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapSetBarring                        (20+sysLibTrapCustom)   /**< GSM Only */
#define     PhnLibTrapGetBarring                        (21+sysLibTrapCustom)   /**< GSM Only */
#define     PhnLibTrapSetForwarding			(22+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapGetForwarding			(23+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapSetCallWaiting			(24+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapGetCallWaiting			(25+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapSetPhoneLock			(26+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapGetPhoneLock			(27+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapSetCLIP				(28+sysLibTrapCustom)   /**< GSM Only */
#define     PhnLibTrapGetCLIP				(29+sysLibTrapCustom)   /**< GSM Only */
#define     PhnLibTrapSetOperatorLock			(30+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapGetOperatorLock			(31+sysLibTrapCustom)   /**< GSM Only */
#define     PhnLibTrapChangePassword			(32+sysLibTrapCustom) 	/**< 		*/
#define     PhnLibTrapPasswordDialog			(33+sysLibTrapCustom)   /**< GSM Only */
#define     PhnLibTrapCurrentOperator			(34+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapCurrentProvider			(35+sysLibTrapCustom)   /**< GSM Only */
#define     PhnLibTrapGetOperatorList			(36+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapSetOperator			(37+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapGetOwnNumbers			(38+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapSetOwnNumbers			(39+sysLibTrapCustom)   /**< GSM Only */
#define     PhnLibTrapGetRingingInfo			(40+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapSetRingingInfo			(41+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapGetToneIDs			(42+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapGetToneName			(43+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapPlayTone				(44+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapStopTone				(45+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapFindEntry				(46+sysLibTrapCustom)   /**< GSM Only */
#define     PhnLibTrapSelectAddress			(47+sysLibTrapCustom)   /**< GSM Only */
#define     PhnLibTrapGetMicrophone			(48+sysLibTrapCustom)   /**< GSM Only */
#define     PhnLibTrapSetMicrophone			(49+sysLibTrapCustom)   /**< GSM Only */
#define     PhnLibTrapGetVolume				(50+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapSetVolume				(51+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapRegistered			(52+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapRoaming				(53+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapSignalQuality			(54+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapErrorRate				(55+sysLibTrapCustom)   /**< GSM Only */
#define     PhnLibTrapBattery				(56+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapCardInfo				(57+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapSIMInfo				(58+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapGetSIMStatus			(59+sysLibTrapCustom)   /**< GSM Only */
#define     PhnLibTrapParamText				(60+sysLibTrapCustom)   /**< GSM Only */
#define     PhnLibTrapGetErrorText			(61+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapGetDBRef                          (62+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapReleaseDBRef                      (63+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapNewMessage                        (64+sysLibTrapCustom)	/**< 		*/
#define	    PhnLibTrapDeleteMessage                     (65+sysLibTrapCustom)	/**< 		*/
#define	    PhnLibTrapSendMessage                       (66+sysLibTrapCustom)	/**< 		*/
#define	    PhnLibTrapSetText                           (67+sysLibTrapCustom)	/**< 		*/
#define	    PhnLibTrapSetDate                           (68+sysLibTrapCustom)	/**< 		*/
#define	    PhnLibTrapSetOptions                        (69+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapSetAddresses                      (70+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapSetStatus                         (71+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapSetFlags                          (72+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapSetOwner                          (73+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapGetText                           (74+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapGetDate                           (75+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapGetOptions                        (76+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapGetAddresses                      (77+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapGetStatus                         (78+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapGetFlags                          (79+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapGetOwner                          (80+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapGetType                           (81+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapIsLegalCharacter                  (82+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapMapCharacter                      (83+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapSetServiceCentreAddress           (84+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapGetServiceCentreAddress           (85+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapLength                            (86+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapGetSubstitution                   (87+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapNewAddressList                    (88+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapDisposeAddressList                (89+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapAddAddress                        (90+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapGetNth                            (91+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapSetNth                            (92+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapCount                             (93+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapSetModulePower			(94+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapModulePowered			(95+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapModuleButtonDown			(96+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapBoxInformation			(97+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapGetBoxNumber			(98+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapGetDataApplication		(99+sysLibTrapCustom)   /**< GSM Only */
#define     PhnLibTrapSetDataApplication		(100+sysLibTrapCustom)  /**< GSM Only */
#define     PhnLibTrapDebug			        (101+sysLibTrapCustom)  /**< GSM Only */
#define     PhnLibTrapGetPhoneBook			(102+sysLibTrapCustom)  /**< GSM Only */
#define     PhnLibTrapSetPhoneBook			(103+sysLibTrapCustom)  /**< GSM Only */
#define     PhnLibTrapSetPhoneBookEntry			(104+sysLibTrapCustom)  /**< GSM Only */
#define     PhnLibTrapGetPhoneBookIndex			(105+sysLibTrapCustom)  /**< GSM Only */
#define     PhnLibTrapBatteryCharge			(106+sysLibTrapCustom)  /**< GSM Only */
#define     PhnLibTrapGetEchoCancellation		(107+sysLibTrapCustom)  /**< GSM Only */
#define     PhnLibTrapSetEchoCancellation		(108+sysLibTrapCustom)  /**< GSM Only */
#define		PhnLibTrapGetEquipmentMode		(109+sysLibTrapCustom)	/**< 		*/
#define		PhnLibTrapSetEquipmentMode		(110+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapGetSMSRingInfo			(111+sysLibTrapCustom)  /**< GSM Only */
#define     PhnLibTrapSetSMSRingInfo			(112+sysLibTrapCustom)  /**< GSM Only */
#define		PhnLibTrapPlayDTMF			(113+sysLibTrapCustom)	/**< 		*/
#define 	PhnLibTrapStartVibrate                  (114+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapStopVibrate                       (115+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapSendUSSD			        (116+sysLibTrapCustom)  /**< GSM Only */
#define     PhnLibTrapSATGetMainMenu			(117+sysLibTrapCustom)  /**< GSM Only */
#define     PhnLibTrapSATSendRequest			(118+sysLibTrapCustom)  /**< GSM Only */
#define     PhnLibTrapSATEndSession			(119+sysLibTrapCustom)  /**< GSM Only */
#define     PhnLibTrapHomeOperatorID			(120+sysLibTrapCustom)  /**< GSM Only */
#define     PhnLibTrapCurrentOperatorID			(121+sysLibTrapCustom)  /**< GSM Only */
#define     PhnLibTrapUsableSignalStrengthThreshold	(122+sysLibTrapCustom)  /**< GSM Only */
#define     PhnLibTrapConnectionAvailable		(123+sysLibTrapCustom)  /**< GSM Only */
#define		PhnLibTrapGetPhoneCallStatus		(124+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapSendSilentDTMF			(125+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapSetCLIR			        (126+sysLibTrapCustom)  /**< GSM Only */
#define     PhnLibTrapGetCLIR			        (127+sysLibTrapCustom)  /**< GSM Only */
#define     PhnLibTrapMute				(128+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapIsCallMuted			(129+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapGetSMSGateway			(130+sysLibTrapCustom)  /**< GSM Only */
#define     PhnLibTrapDeactivateBarring			(131+sysLibTrapCustom)  /**< GSM Only */
#define     PhnLibTrapGPRSAttached                      (132+sysLibTrapCustom)  /**< GSM Only */
#define     PhnLibTrapGPRSPDPContextDefine              (133+sysLibTrapCustom)  /**< GSM Only */
#define     PhnLibTrapGPRSPDPContextListConstruct       (134+sysLibTrapCustom)  /**< GSM Only */
#define     PhnLibTrapGPRSPDPContextListDestruct        (135+sysLibTrapCustom)  /**< GSM Only */
#define     PhnLibTrapGetRemainingPasswordTries	        (136+sysLibTrapCustom)  /**< GSM Only */
#define     PhnLibTrapGPRSQualityOfServiceGet	          (137+sysLibTrapCustom)  /**< GSM Only */
#define     PhnLibTrapGPRSQualityOfServiceSet	          (138+sysLibTrapCustom)  /**< GSM Only */
#define     PhnLibTrapSetBoxNumber                      (139+sysLibTrapCustom)  /**< GSM Only */
#define     PhnLibTrapGetBinary                         (140+sysLibTrapCustom)  /**< GSM Only */
#define     PhnLibTrapNetworkAvailable                  (141+sysLibTrapCustom)  /**< GSM Only */
#define     PhnLibTrapSetFDNEntry                       (142+sysLibTrapCustom)  /**< GSM Only */
#define     PhnLibTrapGetFDNList                        (143+sysLibTrapCustom)  /**< GSM Only */
#define     PhnLibTrapDecodeMMI                         (144+sysLibTrapCustom)  /**< GSM Only */
#define     PhnLibTrapExecuteMMI                        (145+sysLibTrapCustom)  /**< GSM Only */
#define     PhnLibTrapSetActiveLine                     (146+sysLibTrapCustom)  /**< GSM Only */
#define     PhnLibTrapSetBand                           (147+sysLibTrapCustom)  /**< GSM Only */
#define     PhnLibTrapGetBand                           (148+sysLibTrapCustom)  /**< GSM Only */
#define     PhnLibTrapGetCustomServiceProfile           (149+sysLibTrapCustom)  /**< GSM Only */
#define     PhnLibTrapIsEmergencyNumber                 (150+sysLibTrapCustom)  /**< GSM Only */
#define     PhnLibTrapAttrGet                           (151+sysLibTrapCustom)  /**< GSM Only */
#define     PhnLibTrapAttrSet                           (152+sysLibTrapCustom)  /**< GSM Only */
#define     PhnLibTrapGetSDNList                        (153+sysLibTrapCustom)  /**< GSM Only */
#define     PhnLibTrapSendLongDTMF                      (154+sysLibTrapCustom)  /**< GSM Only */
#define     PhnLibTrapGetICCID                          (155+sysLibTrapCustom)  /**< GSM Only */
#define     PhnLibTrapGetSpn                            (156+sysLibTrapCustom)  /**< GSM Only */

// Functions were reserved for GSM/GPRS  (these were reserved when Treo 300 shipped). Do
// not need to be GSM/GPRS specific really now that we have one header file, so treat
// them just as free traps for future functions

#define     GSMReservedFirst                            (157+sysLibTrapCustom)	/**< 		*/
#define     GSMReserved0                                (0+GSMReservedFirst)	/**< 		*/
#define     GSMReserved1                                (1+GSMReservedFirst)	/**< 		*/
#define     GSMReserved2                                (2+GSMReservedFirst)	/**< 		*/
#define     GSMReserved3                                (3+GSMReservedFirst)	/**< 		*/
#define     GSMReserved4                                (4+GSMReservedFirst)	/**< 		*/
#define     GSMReserved5                                (5+GSMReservedFirst)	/**< 		*/
#define     GSMReserved6                                (6+GSMReservedFirst)	/**< 		*/
#define     GSMReserved7                                (7+GSMReservedFirst)	/**< 		*/
#define     GSMReserved8                                (8+GSMReservedFirst)	/**< 		*/
#define     GSMReserved9                                (9+GSMReservedFirst)	/**< 		*/
#define     GSMReserved10                               (10+GSMReservedFirst)	/**< 		*/
#define     GSMReserved11                               (11+GSMReservedFirst)	/**< 		*/
#define     GSMReserved12                               (12+GSMReservedFirst)	/**< 		*/
#define     GSMReserved13                               (13+GSMReservedFirst)	/**< 		*/
#define     GSMReserved14                               (14+GSMReservedFirst)	/**< 		*/
#define     GSMReserved15                               (15+GSMReservedFirst)	/**< 		*/
#define     GSMReserved16                               (16+GSMReservedFirst)	/**< 		*/
#define     GSMReserved17                               (17+GSMReservedFirst)	/**< 		*/
#define     GSMReserved18                               (18+GSMReservedFirst)	/**< 		*/
#define     GSMReserved19                               (19+GSMReservedFirst)	/**< 		*/
#define     GSMReserved20                               (20+GSMReservedFirst)	/**< 		*/
#define     GSMReserved21                               (21+GSMReservedFirst)	/**< 		*/
#define     GSMReserved22                               (22+GSMReservedFirst)	/**< 		*/
#define     GSMReservedLast                             (22+GSMReservedFirst)	/**< 		*/


#define     PhnLibTrapAPGetNth				(180+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapGetMSID				(181+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapGetPhnOprtMode			(182+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapSetPhnOprtMode			(183+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapSendSMSMTAck			(184+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapQuerySMSMT			(185+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapIsPhoneActivated			(186+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapUpdateMessageCount		(187+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapGetSMSPreference			(188+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapSetSMSPreference			(189+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapGetSpMode				(190+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapLedAlert				(191+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapExit911Mode			(192+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapMiscDebugInfo			(193+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapGetHeadsetConnectedInfo	        (194+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapExitPowerSaveMode			(195+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapGetToneLength			(196+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapSetToneLength			(197+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapUnlockPhoneLock			(198+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapLoadCDMATime			(199+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapGetCallCountByService		(200+sysLibTrapCustom)    /**< 		*/
#define     PhnLibTrapGetDefaultDataApps		(201+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapModemIssueResetInfo		(202+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapGetRoamPrefMode			(203+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapSetRoamPrefMode			(204+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapGetRoamPrefInfo			(205+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapSetRoamPrefInfo			(206+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapIsSpecialNumber			(207+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapSetSpecialNumbers			(208+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapGetSpecialNumbers			(209+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapGetDTMFburstMode			(210+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapSetDTMFburstMode			(211+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapSetupDataConfig			(212+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapGetTotalDataTxBytes		(213+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapGetTotalDataRxBytes		(214+sysLibTrapCustom)    /**< CDMA Only */
#define     PrvPhnLibTrapCheckPowerSync			(215+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapSetPhoneSSMode			(216+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapEnableDM				(217+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapPowerSaveDisable                  (218+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapCloseNetIF			(219+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapGetMdmPosition			(220+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapGetPDSessionConfigParam	        (221+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapSetPDSessionConfigParam	        (222+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapStopPDSession			(223+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapSndSave               	        (224+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapSndDelete   			(225+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapListPDP				(226+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapSetPDP				(227+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapGetPDP				(228+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapCopyPDP				(229+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapDeletePDP				(230+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapReplacePDPPassword		(231+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapGetTotalPDPs			(232+sysLibTrapCustom)    /**< CDMA Only */
#define		PhnLibTrapGetOneXStatus			(233+sysLibTrapCustom)    /**< CDMA Only */
#define		PhnLibTrapUpdateCustomizedNV		(234+sysLibTrapCustom)    /**< CDMA Only */
#define		PhnLibTrapSetMdmAutoMsgMask		(235+sysLibTrapCustom)    /**< CDMA Only */
#define		PhnLibTrapGetPhoneSSMode		(236+sysLibTrapCustom)    /**< CDMA Only */
#define		PhnLibTrapGetPDPIndexRange	 	(237+sysLibTrapCustom)    /**< CDMA Only */
#define		PhnLibTrapGetCktSwitchProfile		(238+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapStartIOTASession                  (239+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapEndIOTASession                    (240+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapTunnelIS683ReqMsg                 (241+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapSelectNAM                         (242+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapGetNAMInUse                       (243+sysLibTrapCustom)    /**< vCDMA Only */
#define     PhnLibTrapCommitIOTAParams                  (244+sysLibTrapCustom)    /**< vCDMA Only */
#define     PhnLibTrapReadIOTAItem                      (245+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapWriteIOTAItem                     (246+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapGetIOTAStatus                     (247+sysLibTrapCustom)    /**< CDMA Only */
#define		PhnLibTrapSetCurrentUsageHandler        (248+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapSendDTMFBurst	 		(249+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapAPNewAddress                      (250+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapAPGetField                        (251+sysLibTrapCustom)    /**< CDMA Only */
#define     Reserved2                                   (252+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapCardInfoEx			(253+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapBoxInformationEx                  (254+sysLibTrapCustom)    /**< CDMA Only */
#define		PhnLibTrapSignalQualityEx	        (255+sysLibTrapCustom)    /**< CDMA Only */
#define		PhnLibTrapReplacePDPNAI			(256+sysLibTrapCustom)    /**< CDMA Only */
#define		PhnLibTrapReplacePDPSvcString	        (257+sysLibTrapCustom)    /**< CDMA Only */
#define		PhnLibTrapPDPLockUnlock			(258+sysLibTrapCustom)    /**< CDMA Only */
#define		PhnLibTrapPhoneRestore			(259+sysLibTrapCustom)    /**< CDMA Only */
#define		PhnLibTrapVerifyICDVer			(260+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapGetToneIDsEx                      (261+sysLibTrapCustom)    /**< CDMA only */
#define     PhnLibTrapPlayToneEx                        (262+sysLibTrapCustom)    /**< CDMA only */
#define     PhnLibTrapEnhancedRoamIndicator             (263+sysLibTrapCustom)    /**< CDMA oOnly */
#define     PhnLibTrapNewMOMessage                      (264+sysLibTrapCustom)    /**< CDMA oOnly */
#define     PhnLibTrapGetTTYSetting                     (265+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapSetTTYSetting                     (266+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapGetRadioState                     (267+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapGetLineState                      (268+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapGetLengthDetails                  (269+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapGetPicturesToken                  (270+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapSetPicturesToken                  (271+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapGetPicturesServerInfo             (272+sysLibTrapCustom)    /**< CDMA Only */
#define     PhnLibTrapSendPendingMessages               (273+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapForceDataDormant                  (274+sysLibTrapCustom)    /**< CDMA Only */
#define		PhnLibTrapGetSysPrefMode		(275+sysLibTrapCustom)	  /**< CDMA Only */
#define		PhnLibTrapSetSysPrefMode		(276+sysLibTrapCustom)	  /**< CDMA Only */
#define		PhnLibTrapGetLifeTimer	                (277+sysLibTrapCustom) 	/**< 		*/
#define		PhnLibTrapGetVoicePrivacyMode		(278+sysLibTrapCustom)	  /**< CDMA Only */
#define		PhnLibTrapSetVoicePrivacyMode		(279+sysLibTrapCustom)	  /**< CDMA Only */
#define     PhnLibTrapGetMMSServerInfo                  (280+sysLibTrapCustom)	  /**< CDMA Only */
#define     PhnLibTrapGetSMSSegmentInfo                 (281+sysLibTrapCustom)	  /**< CDMA Only */
#define		PhnLibTrapDebugHook			(282+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapContinuePhonePowerOn              (283+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapContinuePhonePowerOff             (284+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapSetSMSCallbackNumber              (285+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapGetDeviceId                       (286+sysLibTrapCustom)	/**< 		*/
#define     PhnLibTrapSendFlash                         (287+sysLibTrapCustom) /**<CDMA ONLY*/
#define     PhnLibTrapGetCDMASystemInfo                 (288+sysLibTrapCustom) /**<CDMA ONLY*/
#define     PhnLibTrapGetHomepageFromRadio              (289+sysLibTrapCustom) /**<CDMA ONLY*/
#define     PhnLibTrapGetProxyInfo                      (290+sysLibTrapCustom) /**<CDMA ONLY*/
#define     PhnLibTrapIsRadioGenericCDMA                (291+sysLibTrapCustom) /**<CDMA ONLY*/
#define     PhnLibTrapGetGenericCDMAPreference          (292+sysLibTrapCustom) /**<CDMA ONLY*/          
#define     PhnLibTrapIsCarrierDefinedEmergencyNumber   (293+sysLibTrapCustom) /**<CDMA ONLY*/
#define		PhnLibTrapLast				                (293+sysLibTrapCustom) /**< 		*/

#endif
