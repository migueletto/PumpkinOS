/******************************************************************************
 *                                                                            *
 *            (C) Copyright 2000-2002, Sony Corporation                       *
 *                                                                            *
 *----------------------------------------------------------------------------*
 *                                                                            *
 *    <IDENTIFICATION>                                                        *
 *       file name    : $Workfile: SonyPreferences.h $
 *                                                                            *
 *    <PROFILE>                                                               *
 *       Type/DataStructure defintions for Sony Preferences                   *
 *                                                                            *
 *    <HISTORY>                                                               *
 *       Started on   : 02/06/11                                              *
 *       Last Modified: $Date: 03/07/23 10:42 $
 *                                                                            *
 ******************************************************************************/
/* this file is best viewed by setting TAB-stop as 3 */

#ifndef __SONYPREFERENCES_H__
#define __SONYPREFERENCES_H__

/******************************************************************************
 *    Includes                                                                *
 ******************************************************************************/
#include <SonyTypes.h>
#include <SonySystemResources.h>

/******************************************************************************
 *    Usage Example                                                           *
 ******************************************************************************/
/*
	To Get:
		SonyPrefAppCreatorCaptureType pref;

		if (PrefGetAppPreferences(sonyPrefCreator, sonyPrefIdAppCreatorCapture,
			&pref, (UInt16)sizeof(SonyPrefAppCreatorCaptureType), sonyPrefSaved)
			!= noPreferenceFound) {
		}


	To Set:
		SonyPrefAppCreatorCaptureType pref = 'SaAp';

		PrefSetAppPreferences(sonyPrefCreator, sonyPrefIdAppCreatorCapture,
			sonyPrefDataVerLatest,
			&pref, (UInt16)sizeof(SonyPrefAppCreatorCaptureType), sonyPrefSaved);
*/

/******************************************************************************
 *    Constants                                                               *
 ******************************************************************************/
#define sonyPrefCreator			sonySysFileCSony

#define sonyPrefDataVer1		(1)
#define sonyPrefDataVerLatest	sonyPrefDataVer1

/*** preference IDs ***/
enum SonyPrefIdTag {
	/* Sound */
	sonyPrefIdSndWelcome = 0x0001,
	sonyPrefIdSndFlipOpen,
	sonyPrefIdSndFlipClose,
	sonyPrefIdSndRotateRight,
	sonyPrefIdSndRotateLeft,
	sonyPrefIdSndCapture,
	sonyPrefIdSndHWKBClick,
	sonyPrefIdSndSilkMax,
	sonyPrefIdSndSilkMin,

	/* appCreator */
	sonyPrefIdAppCreatorCapture,
	sonyPrefIdAppCreatorVoiceRec,

	/* Virtual Silk */
	sonyPrefIdSilkGrafRsc,
	sonyPrefIdSlkwDefault,
	sonyPrefIdSlkwInputDefault,
	sonyPrefIdSlkwStatusDefault,

	sonyPrefIdCharInputBtnVchr,
	sonyPrefIdKbdBackLightPeriod,
	
	/* CLIE Files, Data(MS)Import appCreator */
	sonyPrefIdAppCreatorFileManagement,
	sonyPrefIdAppCreatorPCConnection,
	
	/* DeviceMgr and Media Info */
	sonyPrefIdDeviceMgrPrefs,
	
	/* Keyboard Launch Apps */
	sonyPrefIdKeyboardLaunchApps,

	/* Virtual Silk Addendum */
	sonyPrefIdSilkMode,
	
	sonyPrefIdNum = 0xFFFF
};
typedef enum SonyPrefIdTag SonyPrefId;


#define sonyPrefSaved		(true)

/******************************************************************************
 *    Type/DataStructure                                                      *
 ******************************************************************************/
	/* Sound */

	/* appCreator */
//	sonyPrefIdAppCreatorCapture,
typedef UInt32 SonyPrefAppCreatorCaptureType;
//	sonyPrefIdAppCreatorVoiceRec,
typedef UInt32 SonyPrefAppCreatorVoiceRecType;

	/* Virtual Silk */
//	sonyPrefIdSilkGrafRsc,
/* typedef SonyPrefSilkGrfRsc_S {
		UInt32 type;
		UInt32 creator;
} SonyPrefSilkGrfRscType;
*/

//	sonyPrefIdSlkwDefault,
//	sonyPrefIdSlkwInputDefault,
//	sonyPrefIdSlkwStatusDefault,
// UInt32 creator;
typedef UInt32 SonyPrefSlkwDefaultType;
typedef UInt32 SonyPrefSlkwInputDefaultType;
typedef UInt32 SonyPrefSlkwStatusDefaultType;

// sonyPrefIdDeviceMgrPrefs
typedef struct SonyPrefDeviceMgrPrefsType{
	Boolean home;
	Boolean wake;
	Boolean	wake_all;
	Boolean wake_fault;
}SonyPrefDeviceMgrPrefsType;
typedef SonyPrefDeviceMgrPrefsType *SonyPrefDeviceMgrPrefsP;

#endif	// __SONYPREFERENCES_H__

