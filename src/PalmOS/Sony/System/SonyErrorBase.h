/******************************************************************************
 *                                                                            *
 *            (C) Copyright 2000-2002, Sony Corporation                       *
 *                                                                            *
 *----------------------------------------------------------------------------*
 *                                                                            *
 *    <IDENTIFICATION>                                                        *
 *       file name    : $Workfile: SonyErrorBase.h $
 *                                                                            *
 *    <PROFILE>                                                               *
 *       The defines of error base in sony CLIE system                        *
 *                                                                            *
 *    <HISTORY>                                                               *
 *       Started on   : 00/10/30                                              *
 *       Last Modified: $Date: 03/06/17 17:21 $
 *                                                                            *
 ******************************************************************************/
/* this file is best viewed by setting TAB-stop as 3 */

#ifndef __SONYERRORBASE_H__
#define __SONYERRORBASE_H__

#include <ErrorBase.h>
#include <SonyTypes.h>

// error code
#define sonyErrorClass				(oemErrorClass)
#define sonySysErrorClass			(sonyErrorClass | 0x000)
#define sonyVFSErrorClass			(sonyErrorClass | 0x100)
#define sonyHRErrorClass			(sonyErrorClass | 0x200)
#ifndef _BUILD_FOR_PALMOS_5_
#define sonyMsaErrorClass			(sonyErrorClass | 0x300)
#endif
#ifndef _FOR_SDK_
#define sonyMMInfoErrorClass		(sonyErrorClass | 0x340)
#endif
#define sonyRmcErrorClass			(sonyErrorClass | 0x400)
#define sonyScsiErrorClass			(sonyErrorClass | 0x500)
#ifndef _FOR_SDK_
#define	sonyUSBDevErrorClass		(sonyErrorClass	| 0x540)
#define	sonyUSBHostErrorClass		(sonyErrorClass	| 0x580)
#define	sonyUSBOTGErrorClass		(sonyErrorClass	| 0x5C0)
#endif
#define sonyIrcErrorClass			(sonyErrorClass | 0x600)
#define	sonyUSBPrtClsErrorClass		(sonyErrorClass | 0x640)
#define	sonyPrintMgrErrorClass		(sonyErrorClass | 0x680)
#define	sonyPrintDrvErrorClass		(sonyErrorClass | 0x6C0)
#define sonySilkErrorClass			(sonyErrorClass | 0x700)
#define	sonyVOutErrorClass 		(sonyErrorClass | 0x740)
#define	sonyUsbCamVdClDrvErrClass (sonyErrorClass | 0x780)
#define	sonyCamLibErrClass		(sonyErrorClass | 0x7C0)

#ifndef _BUILD_FOR_PALMOS_5_
#define sonyCapErrorClass			(sonyErrorClass | 0x800)
#endif
#define sonyJpegErrorClass			(sonyErrorClass | 0x900)
#define sonyCFLibErrorClass			(sonyErrorClass | 0x9C0)
#endif // __SONYERRORBASE_H__

