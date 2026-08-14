/******************************************************************************
 *                                                                            *
 *            (C) Copyright 2000-2002, Sony Corporation                       *
 *                                                                            *
 *----------------------------------------------------------------------------*
 *                                                                            *
 *    <IDENTIFICATION>                                                        *
 *       file name    : $Workfile: SonySystemResources.h $
 *                                                                            *
 *    <PROFILE>                                                               *
 *       CreatorID/Type of DB, ID/Type of Resoueces for Sony System           *
 *                                                                            *
 *    <HISTORY>                                                               *
 *       Started on   : 00/11/28                                              *
 *       Last Modified: $Date: 03/07/23 10:42 $
 *                                                                            *
 ******************************************************************************/
/* this file is best viewed by setting TAB-stop as 3 */

#ifndef __SONYSYSTEMRESOURCES_H__
#define __SONYSYSTEMRESOURCES_H__

/******************************************************************************
 *    Includes                                                                *
 ******************************************************************************/
#include <SystemResources.h>
#include <UIResources.h>
#include <SonyTypes.h>

/******************************************************************************
 *    Fixes                                                                   *
 ******************************************************************************/
#ifdef _BUILD_FOR_PALMOS_5_

#ifndef sysFileTExtensionOEMARM
#define sysFileTExtensionOEMARM	'aexo'
#endif

#ifndef sysFileTExtensionARM
#define sysFileTExtensionARM		'aext'
#endif

#ifdef sysFileTExtension
#undef sysFileTExtension
#endif
#define sysFileTExtension			sysFileTExtensionARM

#ifdef sysFileTExtensionOEM
#undef sysFileTExtensionOEM
#endif
#define sysFileTExtensionOEM		sysFileTExtensionOEMARM

#endif

/******************************************************************************
 *    CreatorID and Type for Databases                                        *
 ******************************************************************************/

/*** Sony oveall ***/
#define sonySysFileCSony			'SoNy'	/* Sony overall */
#define	sonySysVfsCstmApiCreator	sonySysFileCSony	/* VFS Custom Control */

#define sonySysFileCSystem			'SsYs'	/* Sony overall System */

/*** Application ***/


/*** Extension ***/
#define sonySysFileCSonySys			'SeSy'
#define sonySysFileTSonySys		sysFileTExtensionOEM		/* 'aexo' */
#define sonySysLibNameSonySys		"Sony Sys Library"

/*** Library ***/
/* HR-Lib */
#define sonySysFileCHRLib			'SlHr'	/* High Resolution */
#define sonySysFileTHRLib			sysFileTLibrary		/* 'libr' */
#define sonySysLibNameHR			"Sony HR Library"

/* Msa-Lib */
#ifndef _BUILD_FOR_PALMOS_5_
#define sonySysFileCMsaLib			'SlMa'	/* MS Audio */
#define sonySysFileTMsaLib			sysFileTLibrary		/* 'libr' */
#define sonySysLibNameMsa			"Sony Msa Library"
#endif

/* Rmc-Lib */
#ifndef _BUILD_FOR_PALMOS_5_
#define sonySysFileCRmcLib			'SlRm'	/* Remote Control */
#define sonySysFileTRmcLib			sysFileTLibrary		/* 'libr' */
#define sonySysLibNameRmc			"Sony Rmc Library"
#else
#define sonySysFileCRmcLib			'SeRm'	/* Remote Control LibExtn */
#define sonySysFileTRmcLib			sysFileTExtension		/* 'aext' */
#define sonySysLibNameRmc			"Sony Rmc Library"
#endif

/* Scsi-Lib */	/* not officially supported (as of 2001/4) */
#define sonySysFileCScsiLib		'SlSc'	/* SCSI */
#define sonySysFileTScsiLib		sysFileTLibrary		/* 'libr' */
#define sonySysLibNameScsi			"Sony Scsi Library"

/* Irc-Lib */
#ifndef _BUILD_FOR_PALMOS_5_
#define sonySysFileCIrcLib			'SlIr'	/* Infrared Controller */
#define sonySysFileTIrcLib			sysFileTLibrary		/* 'libr' */
#define sonySysLibNameIrc			"Sony Irc Library"
#else
#define sonySysFileCIrcLib			'SeIR'	/* Infrared Controller LibExtn */
#define sonySysFileTIrcLib			sysFileTExtension		/* 'aext' */
#define sonySysLibNameIrc			"Sony Irc Library"
#endif

/* SonySilkLib */
#ifndef _BUILD_FOR_PALMOS_5_
#define sonySysFileCSilkLib		'SlSi'
#define sonySysFileTSilkLib		sysFileTLibrary		/* 'libr' */
#else
#define sonySysFileCSilkExtn		'SeSi'
#define sonySysFileTSilkExtn		sysFileTExtension		/* 'aext' */
#define sonySysResTSilkExtn		sysResTModuleCode		/* 'amdc' */
#define sonySysFileCSilkLib		sonySysFileCSilkExtn
#define sonySysFileTSilkLib		sonySysFileTSilkExtn	/* 'aext' */
#endif
#define sonySysLibNameSilk			"Sony Silk Library"

/* SonySoundLib */
#define sonySysFileTPcmLib			'pcmR'
#ifndef _BUILD_FOR_PALMOS_5_
#define sonySysFileCSoundLib		'SlSd'	/* Sony Sound Lib  */
#define sonySysFileTSoundLib		sysFileTLibrary		/* 'libr' */
#define sonySysLibNameSound		"Sony Sound Library"
#else
#define sonySysFileCSoundLib		'SeSd'	/* Sony Sound LibExtn  */
#define sonySysFileTSoundLib		sysFileTExtension		/* 'aext' */
#define sonySysLibNameSound		"Sony Sound Library"
#endif

/* JpegUtil-Lib */
#define sonySysFileCJpegUtilLib	'SlJU'	/* Jpeg Util Lib */
#define sonySysFileTJpegUtilLib	sysFileTLibrary	/* 'libr' */
#define sonySysLibNameJpegUtil	"Sony Jpeg Util Library"

/* Capture-Lib */
#ifndef _BUILD_FOR_PALMOS_5_
#define sonySysFileCCaptureLib	'SlCp'	/* Capture Lib */
#define sonySysFileTCaptureLib	sysFileTLibrary	/* 'libr' */
#define sonySysLibNameCapture		"Sony Capture Library"
#endif

/* MM-Lib */
#ifndef _FOR_SDK_
#define sonySysFileCMMLib			'SlMM'	/* MMLib */
#define sonySysFileTMMLib			sysFileTLibrary		/* 'libr' */
#define sonySysLibNameMM			"Sony MM Library"
#endif

/* MMUrl-Lib */
#ifndef _FOR_SDK_
#define sonySysFileCMMUrlLib		'SlMU'	/* MMUrlLib */
#define sonySysFileTMMUrlLib		sysFileTLibrary		/* 'libr' */
#define sonySysLibNameMMUrl		"Sony MMUrl Library"
#endif

/* MMInfo-Lib */
#ifndef _FOR_SDK_
#define sonySysFileCMMInfoLib		'SlMI'	/* MMInfoLIb */
#define sonySysFileTMMInfoLib		sysFileTLibrary		/* 'libr' */
#define sonySysLibNameMMInfo		"Sony MMInfo Library"
#endif

/* Print-Lib */
#define sonySysFileCPrintLib		'SlPr'	/* Print Lib */
#define sonySysFileTPrintLib		sysFileTLibrary	/* 'libr' */
#define sonySysLibNamePrintLib		"Sony Print Library"

/* VOut-Lib */
#define sonySysFileCVOutLib 		'SlVo'	/* VOut Lib */
#define sonySysFileTVOutLib		sysFileTLibrary	/* 'libr' */
#define sonySysLibNameVOutLib		"Sony VOut library"

/*** System Application ***/
/* JogPanel Preference */
#ifndef _BUILD_FOR_PALMOS_5_
#define sonySysFileCJogPanel		'SnJp'	/* Jog Panel */
#define sonySysFileTJogPanel		sysFileTPanel		/* 'panl' */
#endif

/******************************************************************************
 *    PalmOS5 or later (ARM-device)                                           *
 ******************************************************************************/
	/* There's no change for 68K-Appl on those devices, but some changes made
	     for ARM-Appl development */

/*** SilkWare plug-in ***/
#define sonySysFileTSilkWare			'Slkw'
#define sonySysFileTSilkWareARM		'aSkw'
#define sonySysFileTStatusWare		'Staw'
#define sonySysFileTStatusWareARM	'aStw'

/*** SilkWare ***/
/* ActiveInput */
#define sonySysFileCActiveInput		'SkAi'	/* Default Silkware */
#define sonySysFileTActiveInput		sonySysFileTSilkWareARM	/* 'aSkw' */
#define sonySysFileTActInSoftGSkin	'SknG'

/*** Slot Storage Libraly ***/
#define sonySysFileTStorageLib		'StrG'

/* BackupPanel Preference */
#define sonySysFileCBackupPanel		'HIBP'
#define sonySysFileTBackupPanel		sysFileTPanel		/* 'panel' */

/******************************************************************************
 *    Misc CreatorID                                                          *
 ******************************************************************************/

/*** for Half Font Size for temporary ***/
#define fontFtrCreator						'font'
#define fontFtrHalfScaleFontIDBase   	0

#endif	// __SONYSYSTEMRESOURCES_H__

