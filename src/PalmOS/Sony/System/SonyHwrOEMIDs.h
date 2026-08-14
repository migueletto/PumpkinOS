/******************************************************************************
 *                                                                            *
 *            (C) Copyright 2000-2002, Sony Corporation                       *
 *                                                                            *
 *----------------------------------------------------------------------------*
 *                                                                            *
 *    <IDENTIFICATION>                                                        *
 *       file name    : $Workfile: SonyHwrOEMIDs.h $
 *                                                                            *
 *    <PROFILE>                                                               *
 *       GHwrOEMIDs (CompanyID, HALID, DeviceID) definitions                  *
 *                                                                            *
 *    <HISTORY>                                                               *
 *       Started on   : 00/12/02                                              *
 *       Last Modified: $Date: 03/11/04 13:23 $
 *                                                                            *
 ******************************************************************************/
/* this file is best viewed by setting TAB-stop as 3 */

#ifndef __SONYHWROEMIDS_H__
#define __SONYHWROEMIDS_H__

/******************************************************************************
 *    Include                                                                 *
 ******************************************************************************/
#include <HwrMiscFlags.h>


/******************************************************************************
 *    Definitions                                                             *
 ******************************************************************************/

/*** GHwrOEMCompanyID ***/
	/* All sony-based model has this ID. other one is not permitted */
	/* can be obtained by FtrGet(sysFtrCreator, sysFtrNumOEMCompanyID, &value) */
	/* hwrOEMCompanyIDSony may be defined in HwrMiscFlags.h someday :-) */
#define sonyHwrOEMCompanyID_Sony		'sony'	/* CAN'T be changed!! */
#ifndef hwrOEMCompanyIDSony
#define hwrOEMCompanyIDSony	sonyHwrOEMCompanyID_Sony		
#endif	// sonyHwrOEMCompanyIDSony

/*** GHwrOEMHALID ***/
	/* defined for each HAL source code, same codes have the same ID */
	/* can be obtained by FtrGet(sysFtrCreator, sysFtrNumOEMHALID, &value) */
#define sonyHwrOEMHALIDPda1Mono		hwrOEMHALIDEZRef			/* 'eref' */
#define sonyHwrOEMHALIDPda1Color		hwrOEMHALIDEZRefColor	/* 'cref' */
#define sonyHwrOEMHALIDYosemite		'ysmt'	/* 3.5 */
#define sonyHwrOEMHALIDNasca			'nsca'	/* 4.0 */
#define sonyHwrOEMHALIDYellowstone	'ystn'	/* 4.0 */
#define sonyHwrOEMHALIDYosemite2		sonyHwrOEMHALIDYosemite
	// instead of #define sonyHwrOEMHALIDYosemite2		'ysm2'	/* 4.0 */
#define sonyHwrOEMHALIDVenice			'vnce'	/* 4.1 */
#define sonyHwrOEMHALIDModena			'mdna'	/* 4.1 */
#define sonyHwrOEMHALIDNasca2			'nsc2'	/* 4.0 */
#define sonyHwrOEMHALIDRedwood		'rdwd'	/* 4.1 */
#define sonyHwrOEMHALIDNaples			'npls'	/* 4.1 */
#define sonyHwrOEMHALIDFortaleza		'frta'	/* 4.1 */
#define sonyHwrOEMHALIDCocos			'cocs'	/* 4.1 */
#define sonyHwrOEMHALIDHawaii			'hwai'	/* 4.1 */
#define sonyHwrOEMHALIDCordoba		'crdb'	/* 5.0 */
#define sonyHwrOEMHALIDGranada		'grnd'	/* 5.0 */
#define sonyHwrOEMHALIDGalapagos		'glps'	/* 4.1 */
#define sonyHwrOEMHALIDMcdonald		'mcnd'	/* 4.1 */
#define sonyHwrOEMHALIDVerona			'vrna'	/* 5.0 */
#define sonyHwrOEMHALIDMadrid			'mdrd'	/* 5.0 */
#define sonyHwrOEMHALIDToledo			'tldo'	/* 5.0 */
#define sonyHwrOEMHALIDPrimeur		'prmr'	/* 5.2 */
#define sonyHwrOEMHALIDAmano		'amno'  /* 5.2 */
#define sonyHwrOEMHALIDGoku			'goku'	/* 5.2 */
#define sonyHwrOEMHALIDGohan			'goha'	/* 5.2 */
#define sonyHwrOEMHALID_1001			'atom'	/* 5.2 */
#define sonyHwrOEMHALID_1002			'luke'	/* 5.2 */
#define sonyHwrOEMHALID_1003			'leia'	/* 5.2 */

/*** GHwrOEMDeviceID ***/
	/* defined for each model. more than one models have the same HALID, but
	     not DeviceID */
	/* can be obtained by FtrGet(sysFtrCreator, sysFtrNumOEMDeviceID, &value) */
#define sonyHwrOEMDeviceIDPda1Mono			(0x00010001)
#define sonyHwrOEMDeviceIDPda1Color			(0x00010002)
#define sonyHwrOEMDeviceIDYosemite			'ysmt'
#define sonyHwrOEMDeviceIDNasca				'nsca'
#define sonyHwrOEMDeviceIDYellowstone		'ystn'
#define sonyHwrOEMDeviceIDYosemite2			'ysm2'
#define sonyHwrOEMDeviceIDVenice				'vnce'
#define sonyHwrOEMDeviceIDModena				'mdna'
#define sonyHwrOEMDeviceIDNasca2				'nsc2'
#define sonyHwrOEMDeviceIDRedwood			'rdwd'
#define sonyHwrOEMDeviceIDNaples				'npls'
#define sonyHwrOEMDeviceIDFortaleza			'frta'
#define sonyHwrOEMDeviceIDCocos				'cocs'
#define sonyHwrOEMDeviceIDHawaii				'hwai'
#define sonyHwrOEMDeviceIDCordoba			'crdb'
#define sonyHwrOEMDeviceIDGranada			'grnd'
#define sonyHwrOEMDeviceIDGalapagos			'glps'
#define sonyHwrOEMDeviceIDMcdonald			'mcnd'
#define sonyHwrOEMDeviceIDVerona				'vrna'
#define sonyHwrOEMDeviceIDMadrid				'mdrd'
#define sonyHwrOEMDeviceIDToledo				'tldo'
#define sonyHwrOEMDeviceIDPrimeur			'prmr'
#define sonyHwrOEMDeviceIDAmano				'amno'	
#define sonyHwrOEMDeviceIDGoku				'goku'
#define sonyHwrOEMDeviceIDGohan				'goha'
#define sonyHwrOEMDeviceID_1001				'atom'
#define sonyHwrOEMDeviceID_1002				'luke'
#define sonyHwrOEMDeviceID_1003				'leia'

/******************************************************************************
 *    References                                                              *
 ******************************************************************************/
/* CAUTIONS: This information is provided just for your information, and not
     guaranteed to be correct all the time */

/***** PalmOS 3.5 *****/
/* PEG-S300 */
#define sonyHwrOEMHALID_S300			sonyHwrOEMHALIDPda1Mono
#define sonyHwrOEMDeviceID_S300		sonyHwrOEMDeviceIDPda1Mono

/* PEG-S500C(J) */
#define sonyHwrOEMHALID_S500C			sonyHwrOEMHALIDPda1Color
#define sonyHwrOEMDeviceID_S500C		sonyHwrOEMDeviceIDPda1Color

/* PEG-N700C(J),N710C(US) */
#define sonyHwrOEMHALID_N700C			sonyHwrOEMHALIDYosemite
#define sonyHwrOEMHALID_N710C			sonyHwrOEMHALIDYosemite
#define sonyHwrOEMDeviceID_N700C		sonyHwrOEMDeviceIDYosemite
#define sonyHwrOEMDeviceID_N710C		sonyHwrOEMDeviceIDYosemite

/***** PalmOS 4.x *****/
/* PEG-S320(US) */
#define sonyHwrOEMHALID_S320			sonyHwrOEMHALIDNasca
#define sonyHwrOEMDeviceID_S320		sonyHwrOEMDeviceIDNasca

/* PEG-N600C(J),N610C(US) */
#define sonyHwrOEMHALID_N600C			sonyHwrOEMHALIDYellowstone
#define sonyHwrOEMHALID_N610C			sonyHwrOEMHALIDYellowstone
#define sonyHwrOEMDeviceID_N600C		sonyHwrOEMDeviceIDYellowstone
#define sonyHwrOEMDeviceID_N610C		sonyHwrOEMDeviceIDYellowstone

/* PEG-N750C(J),N760C(US),N760C/G(GVD),N770C/U(UK),N770C/E(EFG) */
#define sonyHwrOEMHALID_N750C			sonyHwrOEMHALIDYosemite2
#define sonyHwrOEMHALID_N760C			sonyHwrOEMHALIDYosemite2
#define sonyHwrOEMHALID_N770C			sonyHwrOEMHALIDYosemite2
#define sonyHwrOEMDeviceID_N750C		sonyHwrOEMDeviceIDYosemite2
#define sonyHwrOEMDeviceID_N760C		sonyHwrOEMDeviceIDYosemite2
#define sonyHwrOEMDeviceID_N770C		sonyHwrOEMDeviceIDYosemite2

/* PEG-T400(J),T415(US),T415/G(GVD),T425/U(UK),T425/E(EFG) */

#define sonyHwrOEMHALID_T400			sonyHwrOEMHALIDVenice
#define sonyHwrOEMHALID_T415			sonyHwrOEMHALIDVenice
#define sonyHwrOEMHALID_T425			sonyHwrOEMHALIDVenice
#define sonyHwrOEMDeviceID_T400		sonyHwrOEMDeviceIDVenice
#define sonyHwrOEMDeviceID_T415		sonyHwrOEMDeviceIDVenice
#define sonyHwrOEMDeviceID_T425		sonyHwrOEMDeviceIDVenice

/* PEG-T600C(J),T615C(US),T615C/G(GVD),T625C/U(UK),T625C/E(EFG) */
#define sonyHwrOEMHALID_T600C			sonyHwrOEMHALIDModena
#define sonyHwrOEMHALID_T615C			sonyHwrOEMHALIDModena
#define sonyHwrOEMHALID_T625C			sonyHwrOEMHALIDModena
#define sonyHwrOEMDeviceID_T600C		sonyHwrOEMDeviceIDModena
#define sonyHwrOEMDeviceID_T615C		sonyHwrOEMDeviceIDModena
#define sonyHwrOEMDeviceID_T625C		sonyHwrOEMDeviceIDModena

/* PEG-S360(US) */
#define sonyHwrOEMHALID_S360			sonyHwrOEMHALIDNasca2
#define sonyHwrOEMDeviceID_S360		sonyHwrOEMDeviceIDNasca2

/* PEG-NR70,NR70V(J,US) */
#define sonyHwrOEMHALID_NR70			sonyHwrOEMHALIDRedwood
#define sonyHwrOEMDeviceID_NR70		sonyHwrOEMDeviceIDRedwood

/* PEG-T650C(J),T665(US) */
#define sonyHwrOEMHALID_T650C			sonyHwrOEMHALIDNaples
#define sonyHwrOEMHALID_T665C			sonyHwrOEMHALIDNaples
#define sonyHwrOEMDeviceID_T650C		sonyHwrOEMDeviceIDNaples
#define sonyHwrOEMDeviceID_T665C		sonyHwrOEMDeviceIDNaples

/* PEG-SL10(US) */
#define sonyHwrOEMHALID_SL10			sonyHwrOEMHALIDFortaleza
#define sonyHwrOEMDeviceID_SL10		sonyHwrOEMDeviceIDFortaleza

/* PEG-SJ20(US) */
#define sonyHwrOEMHALID_SJ20			sonyHwrOEMHALIDCocos
#define sonyHwrOEMDeviceID_SJ20		sonyHwrOEMDeviceIDCocos

/* PEG-SJ30(J,US) */
#define sonyHwrOEMHALID_SJ30			sonyHwrOEMHALIDHawaii
#define sonyHwrOEMDeviceID_SJ30		sonyHwrOEMDeviceIDHawaii

/* PEG-SJ22 */
#define sonyHwrOEMHALID_SJ22			sonyHwrOEMHALIDGalapagos
#define sonyHwrOEMDeviceID_SJ22		sonyHwrOEMDeviceIDGalapagos

/* PEG-SJ33 */
#define sonyHwrOEMHALID_SJ33			sonyHwrOEMHALIDMcdonald
#define sonyHwrOEMDeviceID_SJ33		sonyHwrOEMDeviceIDMcdonald

/***** PalmOS 5.x *****/
/* PEG-NX70(V) */
#define sonyHwrOEMHALID_NX70			sonyHwrOEMHALIDCordoba
#define sonyHwrOEMDeviceID_NX70		sonyHwrOEMDeviceIDCordoba

/* PEG-NZ90 */
#define sonyHwrOEMHALID_NZ90			sonyHwrOEMHALIDGranada
#define sonyHwrOEMDeviceID_NZ90		sonyHwrOEMDeviceIDGranada

/* PEG-TG50 */
#define sonyHwrOEMHALID_TG50			sonyHwrOEMHALIDVerona
#define sonyHwrOEMDeviceID_TG50		sonyHwrOEMDeviceIDVerona

/* PEG-NX73V */
#define sonyHwrOEMHALID_NX73			sonyHwrOEMHALIDToledo
#define sonyHwrOEMDeviceID_NX73		sonyHwrOEMDeviceIDToledo

/* PEG-NX80V */
#define sonyHwrOEMHALID_NX80			sonyHwrOEMHALIDMadrid
#define sonyHwrOEMDeviceID_NX80		sonyHwrOEMDeviceIDMadrid

/* PEG-UX50 */
#define sonyHwrOEMHALID_UX50			sonyHwrOEMHALIDPrimeur
#define sonyHwrOEMDeviceID_UX50		sonyHwrOEMDeviceIDPrimeur

/* PEG-UX40 */
#define sonyHwrOEMHALID_UX40			sonyHwrOEMHALIDAmano
#define sonyHwrOEMDeviceID_UX40		sonyHwrOEMDeviceIDAmano

/* PEG-TJ35 */
#define sonyHwrOEMHALID_TJ35			sonyHwrOEMHALIDGoku
#define sonyHwrOEMDeviceID_TJ35		sonyHwrOEMDeviceIDGoku

/* PEG-TJ25 */
#define sonyHwrOEMHALID_TJ25			sonyHwrOEMHALIDGohan
#define sonyHwrOEMDeviceID_TJ25		sonyHwrOEMDeviceIDGohan

/* PEG-TH55 */
#define sonyHwrOEMHALID_TH55			sonyHwrOEMHALID_1001
#define sonyHwrOEMDeviceID_TH55		sonyHwrOEMDeviceID_1001

/* PEG-TJ37 */
#define sonyHwrOEMHALID_TJ37			sonyHwrOEMHALID_1002
#define sonyHwrOEMDeviceID_TJ37		sonyHwrOEMDeviceID_1002

/* PEG-TJ27 */
#define sonyHwrOEMHALID_TJ27        sonyHwrOEMHALID_1003
#define sonyHwrOEMDeviceID_TJ27     sonyHwrOEMDeviceID_1003


#endif	// __SONYHWROEMIDS_H__

