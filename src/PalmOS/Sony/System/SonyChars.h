/******************************************************************************
 *                                                                            *
 *            (C) Copyright 2000-2002, Sony Corporation                       *
 *                                                                            *
 *----------------------------------------------------------------------------*
 *                                                                            *
 *    <IDENTIFICATION>                                                        *
 *       file name    : $Workfile: SonyChars.h $
 *                                                                            *
 *    <PROFILE>                                                               *
 *       Sony-specific Virtual character definition                           *
 *                                                                            *
 *    <HISTORY>                                                               *
 *       Started on   : 00/11/28                                              *
 *       Last Modified: $Date: 03/06/18 10:49 $
 *                                                                            *
 ******************************************************************************/
/* this file is best viewed by setting TAB-stop as 3 */

#ifndef __SONYCHARS_H__
#define __SONYCHARS_H__

/******************************************************************************
 *     Min/Max                                                                *
 ******************************************************************************/
#if defined(vchrSonyMin) || defined(vchrSonyMax)
	#undef vchrSonyMax
	#undef vchrSonyMin
#endif

#define vchrSonyMin				(0x1700)
#define vchrSonyMax				(0x17FF)			/* 256 command keys */

/******************************************************************************
 *     Chars                                                                  *
 ******************************************************************************/
/***** Jog *****/
	/* Developers are encouraged to use those chars */
#define vchrJogUp					(0x1700)
#define vchrJogDown				(0x1701)
#define vchrJogPushRepeat		(0x1702)
#define vchrJogPushedUp			(0x1703)
#define vchrJogPushedDown		(0x1704)
#define vchrJogPush				(0x1705)
#define vchrJogRelease			(0x1706)
#define vchrJogBack				(0x1707) 		/* added @ 2001 */
#define vchrJogLeft				(0x1708) 		/* added @ 2003 */
#define vchrJogRight				(0x1709) 		/* added @ 2003 */

/*** re-define old key names ***/
	/* Developpers are encouraged not to use those obsolete chars */
#define vchrJogPressRepeat		vchrJogPushRepeat
#define vchrJogPageUp			vchrJogPushedUp
#define vchrJogPageDown			vchrJogPushedDown
#define vchrJogPress				vchrJogPush	

/* movement & chars */
/*
        Up     PushedUp
        A        A
        |        |
        |      --
        |    --
           --
         ---------> Push/PushRepeat
Release <---------
           --
        |    --
        |      --
        |        |
        V        V
       Down    PushedDown


        -----------> Back
*/
/*** Access macros ***/
	/* eP must be EventPtr */
#define SonyKeyIsJog(eP)	\
	(((eP->data.keyDown.chr >= vchrJogUp)	\
		 && (eP->data.keyDown.chr <= vchrJogRight))? true: false) 

/*** Others ***/
#define vchrRmcKeyPush			(0x170A)
#define vchrRmcKeyRelease		(0x170B)
	/* each remocon-key is identified with keyCode field */
	/* autoRepeat modifier is set when KeyPush is repeated */

#define vchrCapture				(0x170C)
#define vchrVoiceRec				(0x170D)

#define vchrAdapterInt			(0x170E)
#define vchrExtCnt				vchrAdapterInt

#define vchrSonySysNotify		(0x170F)

#define vchrSilkResize			(0x1710)
#define vchrSilkLoader			(0x1711)
#define vchrSilkChangeSlkw		(0x1712)

#define vchrVolumeDialog		(0x1713)
#define vchrBatteryDialog		(0x1714)
#define vchrMediaInfoDialog	(0x1715)
#define vchrHwrKbdHelpDialog	(0x1716)
#define vchrGraffitiDialog		(0x1717)
#define vchrStatusDialog		(0x1718)

#define vchrCapFullRelease		vchrCapture
#define vchrCapHalfRelease		(0x171A)
#define vchrCapCancel			(0x171B)
	/* CapHalRelease is enqueued when Capture button is half-pressed. */
	/* CapCancel is enqueued when Capture button is released after half-pressed,
	     not thoroughly pressed. */
	/* CapHalfRelease and CapCancel are paired. */
	/* When pushing Capture button thoroughly, only CapFullRelease is enqueued
	     and CapHalfRelease is not enqueued ahead of CapFullRelease.
	   Allowable gap between CapHalf and CapFull must be under 50msec.
	   When CapFullRelease is emitted, CapCancel is not enqueued. */

#define vchrCamStateChange		(0x171C)
	/* enqueued when camera internal state is changed, like AF begins, AF locks,
	     Exposure begins, Exposure ends,... 
	   Those states are identified by keyCode field */

#define vchrHomePush			(0x171D)
#define vchrGraffitiDialogPush	(0x171E)
#define vchrInputDialog			(0x171F)

#define vchrHWKeyboardPush		(0x1720)
#define vchrHWKeyboardRelease	(0x1721)

#define vchrSonyShortcut		(0x1722)		// Debug use-only

#define vchrBatteryAlert		(0x1723)

#define	vchrCloseDialog			(0x1724)		// close dialog
#define	vchrDeviceMgr			(0x1725)

#define	vchrHibernation			(0x1726)		// enter hibernation

#define	vchrCamOpen				(0x1727)		// Camera lens open

/******************************************************************************
 *     KeyCodes                                                               *
 ******************************************************************************/
/*** vchrSonySysNotify ***/
#define keyCodeNotifyNull			(0x0000)
#define keyCodeNotifyHoldMask		(0x0100)
#define keyCodeNotifyHoldOn		(keyCodeNotifyHoldMask + 0x0001)
#define keyCodeNotifyHoldOff		(keyCodeNotifyHoldMask + 0x0002)
#define keyCodeNotifyHoldAlert	(keyCodeNotifyHoldMask + 0x0003)

/*** vchrRmcKeyPushed/vchrRmcKeyReleased ***/
#define keyCodeRmcPlay			(3303)
#define keyCodeRmcFRPlay		(3098)
#define keyCodeRmcFFPlay		(2498)
#define keyCodeRmcStop			(1993)
#define keyCodeRmcVolDown		(1856)
#define keyCodeRmcVolUp			(1713)

/*** vchrCamStateChange ***/
#define keyCodeCamUnknown		(0)
#define keyCodeCamAFStart		(1)
#define keyCodeCamAFEnd			(2)
#define keyCodeCamExpStart		(3)
#define keyCodeCamExpEnd		(4)

/*** vchrSilkChangeSlkw ***/
#define keyCodeSilkDef			(0)
#define keyCodeSilkInputDef	(1)
#define keyCodeSilkPrev			(2)
#define keyCodeSilkFwd			(3)
#define keyCodeSilkBack			(4)

/*** vchrMediaInfoDialog ***/
// keyCode is slotRefNum

/*** vchrSonyShortcut ***/
#define keyCodeSCVersionSony	(0x0001)		// Version
#define keyCodeSCSilkMgr		(0x0002)		// Resize Speed (High/Low/None)
#define keyCodeSCCpuConfig		(0x0003)		// Cpu Config Set (Level0/1/2...)
#define keyCodeSCCpuConfig2	(0x0004)		// Cpu Config Get
#define keyCodeSCDeviceReset	(0x0005)		// DeviceReset
#define keyCodeSCIllegalBattery	(0x0006)		// Illegal battery
#define keyCodeSCTraceInit		(0x0007)		// Call HostTraceInit
#define keyCodeSCTraceClose	(0x0008)		// Call HostTraceClose
#define keyCodeSCBackupStorage	(0x0009)		// backup storage heap
#define keyCodeSCRestoreStorage	(0x000a)		// restore storage heap
#define keyCodeSCNANDFormat	    (0x000b)		// Enable NAND Format

	/* Option Set */	/* those options are all able to be On/Off(default) */
#define keyCodeSCOptionSet0	(0x0010)		// 0:
		// those codes from Set0 thru Get0 are reserved for SetX
	/* Option Get */
#define keyCodeSCOptionGet0	(0x0020)		// Get option bitfield
		// those codes from Get0 thru Get are reserved for GetX
#define keyCodeSCOptionGet		(0x0030)		// Get option bitfield

#endif // __SONYCHARS_H__

