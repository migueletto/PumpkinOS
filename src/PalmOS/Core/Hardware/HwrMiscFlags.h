/******************************************************************************
 *
 * Copyright (c) 1995-2003 PalmSource, Inc. All rights reserved.
 *
 * File: HwrMiscFlags.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *		Bit constants for the hardware MiscFlags
 *
 *****************************************************************************/

#ifdef	NON_PORTABLE	   // So app's don't mistakenly include this

#ifndef __HWR_MISCFLAGS_H__
#define __HWR_MISCFLAGS_H__



/**************************************************************************
 * General Equates
 ***************************************************************************/
 
// ----------------------------------------------------------------------
// NOTE: In some ROMs between 3.0 and 3.3 (inclusive), OEMs may have
// ROM tokens present in the ROM that were detected by the boot code
// in order to set the various HwrMiscFlags and GHwrMiscFlagsExt
// settings.  That scheme is no longer relevant, since starting with
// version 3.5 and later, it is now the responsibility of the HAL
// to set these flags, using whatever means necessary to determine
// what "features" the device has that the higher level OS may want
// to know about.
//
// These flags are defined in this public header file since both
// of these low memory globals are published as system features
// (sysFtrNumHwrMiscFlags and sysFtrNumHwrMiscFlagsExt) in <SystemMgr.h>.
// These features are for third party software that may (for whatever
// reason) want to know about certain hardware differences without
// having to read the low memory globals directly.
//
// Palm OS v3.1 was the first OS to publish sysFtrNumHwrMiscFlags as a feature.
// 	Call FtrGet first; if the feature doesn't exist, check the OS version:
// Palm OS v2.0 and 3.0 have GHwrMiscFlags defined as a low memory global.
// Palm OS v1.0 did not have GHwrMiscFlags, so its contents are unpredictable.
//		Any devices running Palm OS v1.0 devices should assume zero for all flags.
// ----------------------------------------------------------------------

// Bits in the low memory global GHwrMiscFlags (UInt16)
#define	hwrMiscFlagHasBacklight		0x0001			// set if backlight is present
#define	hwrMiscFlagHasMbdIrDA		0x0002			// set if IrDA is present (on main board)
#define	hwrMiscFlagHasCardIrDA		0x0004			// set if IrDA is present (on memory card)
#define	hwrMiscFlagHasBurrBrown		0x0008			// set if BurrBrown A/D is present
#define	hwrMiscFlagHasJerryHW		0x0010			// set if Jerry Hardware is present
#define	hwrMiscFlagNoRTCBug			0x0020			// set if using rev of DragonBall (3G or later)
																	//  that doesn't require the RealTimeClock
																	//  bug work-around (see TimeMgr68328.c).
																	//  <chg 3-27-98 RM>
#define	hwrMiscFlagHas3vRef			0x0040			// set if switchable 3v reference is present
#define	hwrMiscFlagHasAntennaSw		0x0080			// set if viewer has an antenna raised switch
#define	hwrMiscFlagHasCradleDetect	0x0100			// set if we have an A/D converter on hotsync port used for ID'ing the attached device
#define	hwrMiscFlagHasSWContrast	0x0200			// set if UI should support software contrast
#define	hwrMiscFlagInvertLCDForBL	0x0400			// set if we need to invert LCD w/Backlight
#define	hwrMiscFlagHasMiscFlagExt	0x0800			// set if we have new hwrMiscFlagsExt

			// The following bit flags are set by HwrIdentifyFeatures.
			// They allow software to read the hardware ID without poking at hardware.
			// They also provide some isolation from different ID detection schemes
			// such as if the ID detection mechanism should change with EZ...
#define	hwrMiscFlagID1				0x1000				// set if ID bit keyBitHard1 was set
#define	hwrMiscFlagID2				0x2000				// set if ID bit keyBitHard2 was set
#define	hwrMiscFlagID3				0x4000				// set if ID bit keyBitHard3 was set
#define	hwrMiscFlagID4				0x8000				// set if ID bit keyBitHard4 was set
#define	hwrMiscFlagIDMask			0xF000
#define	hwrMiscFlagIDOffset		12						// Bits to shift to get a numeric ID


// NOTE: Currently, the '328 IDs don't overlap with the 'EZ IDs.  This is NOT a requirement,
// but is convenient for the time being as it makes it one step easier to identify a device.
// If the spaces are forced to overlap, it will be necessary to first check the processor
// type (328 or EZ) and then parse the product ID code.  Fortunately, this scheme is rapidly
// becoming obsolete since it was based on reading the keyboard I/O pins, and new products
// are starting to move their keyboard I/O bits to new places.  With the introduction of
// different HAL modules, identifying the actual hardware is now something the HAL code
// will do when the device boots.  The HAL need only do whatever it needs to do to uniquely
// tell the difference between those devices on which it is capable of operating.  Once
// the hardware is identified, the appropriate hwrMiscFlag and hwrMiscFlagExt bits can be
// set to tell the OS what features are present, and the appropriate hardware ID information
// can also be set so higher level software can uniquely identify the OEM/Device/HAL info.
//
// Changes
//  3/16/99 SCL: Documented '328 and 'EZ IDs and how the space could overlap if necessary
//  3/31/99 SRJ: hwrMiscFlagIDUndetermined created, used specifically during the boot sequence
//               before we have done HwrIdentifyFeatures().
// 10/29/99 SCL: Renamed hwrMiscFlagIDOther to hwrMiscFlagIDCheckROMToken
// 10/29/99 SCL: Assigned hwrMiscFlagIDUnused1 to hwrMiscFlagIDUndetermined for Palm OS 3.5
// 10/29/99 SCL: Assigned hwrMiscFlagIDUnused2 to hwrMiscFlagIDCheckOEMFtrs for Palm OS 3.5
// 11/ 2/99 SCL: Assigned hwrMiscFlagIDUnused3 to hwrMiscFlagIDCobra2 for Palm OS 3.5


// hwrMiscFlagIDCheckROMToken indicates that the actual device ID information
// should be read from hwrROMTokenHardwareID using SysGetROMToken or HwrGetROMToken.
// Attached to this token is the OEM ID and the OEM-specific Product ID.
// This scheme was used in Palm OS releases prior to 3.5.  See <HwrROMToken.h> for details.
// This ID is also reported when booting on PalmPilot devices (aka 2.0 hardware).
#define	hwrMiscFlagIDCheckROMToken	(0)						// used to be hwrMiscFlagIDOther
#define	hwrMiscFlagIDPalmPilot		(0)						// since it was never explicitly set

// hwrMiscFlagIDUndetermined is what the OS initializes the ID to when booting.
// The HAL is responsible for setting the ID to something valid (and meaningful).
#define	hwrMiscFlagIDUndetermined	(hwrMiscFlagID1)		// used to be hwrMiscFlagIDUnused1

// hwrMiscFlagIDCheckOEMFtrs indicates that the OEM/Device/HAL identification
// information should be read from the new Palm OS 3.5 System Features
// (sysFtrNumOEMCompanyID, sysFtrNumOEMDeviceID, and sysFtrNumOEMHALID)
// or system globals (hwrOEMCompanyID, hwrOEMDeviceID, and hwrOEMHALID).
// This method of hardware device ID is for HAL-based devices starting with Palm OS
// 3.5, but some devices may continue to report valid old-style hwrMiscFlagIDxxx tags.
#define	hwrMiscFlagIDCheckOEMFtrs	(hwrMiscFlagID2)		// used to be hwrMiscFlagIDUnused2

// Old-style Hardware IDs for DragonBall '328 based products
#define	hwrMiscFlagIDThumper		(hwrMiscFlagID4 | hwrMiscFlagID2)
#define	hwrMiscFlagIDJerry			(hwrMiscFlagID4 | hwrMiscFlagID3)
#define	hwrMiscFlagIDRocky			(hwrMiscFlagID4 | hwrMiscFlagID3 | hwrMiscFlagID2)
#define	hwrMiscFlagIDTouchdown		(hwrMiscFlagID4 | hwrMiscFlagID3 | hwrMiscFlagID2 | hwrMiscFlagID1)

// Old-style Hardware IDs for DragonBall 'EZ based products
#define	hwrMiscFlagIDJerryEZ		(hwrMiscFlagID3 | hwrMiscFlagID2)
#define	hwrMiscFlagIDSumo			(hwrMiscFlagID4 | hwrMiscFlagID2 | hwrMiscFlagID1)
#define	hwrMiscFlagIDBrad			(hwrMiscFlagID4 | hwrMiscFlagID3 | hwrMiscFlagID1)
#define	hwrMiscFlagIDAustin			(hwrMiscFlagID4 | hwrMiscFlagID1)
#define	hwrMiscFlagIDCobra2			(hwrMiscFlagID2 | hwrMiscFlagID1)
#define	hwrMiscFlagIDCalvin			(hwrMiscFlagID3 | hwrMiscFlagID1)


// Hardware SubIDs used to detect hardware type early in boot process
#define	hwrMiscFlagExtSubIDBrad				0x0
#define	hwrMiscFlagExtSubIDSumo				0x2
#define	hwrMiscFlagExtSubIDCobra			0x4
#define	hwrMiscFlagExtSubIDCobra2_16		0x6
#define	hwrMiscFlagExtSubIDCobra2_20		0x7



// Old-style Hardware IDs still unused
#define	hwrMiscFlagIDUnused4			(hwrMiscFlagID3)
#define	hwrMiscFlagIDUnused5			(hwrMiscFlagID3 | hwrMiscFlagID1)
#define	hwrMiscFlagIDUnused7			(hwrMiscFlagID3 | hwrMiscFlagID2 | hwrMiscFlagID1)
#define	hwrMiscFlagIDUnused8			(hwrMiscFlagID4)


// Bits in the low memory global GHwrMiscFlagsExt (UInt32)
#define	hwrMiscFlagExtSubID1		0x00000001		// subtype ID (for feature select in device)
#define	hwrMiscFlagExtSubID2		0x00000002		// subtype ID (for feature select in device)
#define	hwrMiscFlagExtSubID3		0x00000004		// subtype ID (for feature select in device)
#define	hwrMiscFlagExtSubIDMask		0x00000007		// sybtype ID Mask

#define	hwrMiscFlagExtHasLiIon		0x00000010		// set if we have Lithium Ion battery rechargable in the cradle
#define	hwrMiscFlagExtHasRailIO		0x00000020		// set if we have Rail I/O hardware
#define	hwrMiscFlagExtHasFlash		0x00000040		// set (by OS or HAL) if we have Flash ROM
#define	hwrMiscFlagExtHasFParms		0x00000080		// set (by OS or HAL) if we have Flash parms area

#define	hwrMiscFlagExt115KIrOK		0x00000100		// device supports 115K IR transfers
#define	hwrMiscFlagExtHasExtLCD		0x00000200		// device has external LCD controller
#define	hwrMiscFlagExtHasSWBright	0x00000400		// device has software controlled brightness

// Added by BGT, 08/01/2000
#define hwrMiscFlagExtNeedsLpr		0x00000800		// DRAM needs special LP Refresh

// Assigned values for hwrOEMCompanyID (aka sysFtrNumOEMCompanyID):
// Values are assigned by the PalmSource Platform Engineering group.
//
// Note: These values are different from the values that may be found in some
// OEM devices which used HwrROMTokens on versions of Palm OS prior to 3.5.

#define hwrOEMCompanyIDUnspecified	0x00000000		// hwrOEMCompanyID not specified by HAL
#define hwrOEMHALIDUnspecified		0x00000000		// hwrOEMHALID not specified by HAL
#define hwrOEMDeviceIDUnspecified	0x00000000		// hwrOEMDeviceID not specified by HAL

#define hwrOEMCompanyIDPalmPlatform	'psys'			// Reference Platforms made by Palm Computing
#define hwrOEMCompanyIDPalmDevices	'palm'			// Devices made by Palm Computing

#define hwrOEMCompanyIDSymbol		'smbl'			// Devices made by Symbol Technologies
#define hwrOEMCompanyIDQualcomm		'qcom'			// Devices made by Qualcomm
#define hwrOEMCompanyIDTRG			'trgp'			// Devices made by TRG Products
#define hwrOEMCompanyIDHandspring	'hspr'			// Devices made by Handspring
#define hwrOEMCompanyIDSony			'sony'			// Devices made by Sony

// Note that values for hwrOEMDeviceID (aka sysFtrNumOEMDeviceID) and
// hwrOEMHALID (aka sysFtrNumOEMHALID) are OEM vendor-specific, and not
// necessarily tracked by this Palm OS header file, though it may be
// worthwhile to include "known" values here for third party developers.
//
// It is recommended that OEM vendors choose values for these globals that
// are four-digit human-readable ASCII values, rather than numeric codes,
// though this is not a requirement.

// HALs that belong to hwrOEMCompanyIDPalmPlatform

#define hwrOEMHALIDEZRef			'eref'			// (Mono 160x160) EZ Reference Platform (PalmSource)
#define hwrOEMHALIDEZRefColor		'cref'			// (Color 160x160) EZ Reference Platform (PalmSource)

#define hwrOEMHALIDVZRefMono1X		'vref'			// (Mono 160x160) VZ Reference Platform (PalmSource)
#define hwrOEMHALIDVZRefMonoQX		'vrfq'			// (Mono QVGA) VZ Reference Platform (PalmSource)
#define hwrOEMHALIDVZRefMono2X		'vrfd'			// (Mono 320x320) VZ Reference Platform (PalmSource)
#define	hwrOEMHALIDVZRef			hwrOEMHALIDVZRefMono1X	// Old name for mono VZRef ROM.

#define hwrOEMHALIDVZRefColor1X		'cvrf'			// (Color 160x160) VZ Reference Platform (PalmSource)
#define hwrOEMHALIDVZRefColorQX		'cvrq'			// (Color QVGA) VZ Reference Platform (PalmSource)
#define hwrOEMHALIDVZRefColor2X		'cvrd'			// (Color 320x320) VZ Reference Platform (PalmSource)
#define	hwrOEMHALIDVZRefColor		hwrOEMHALIDVZRefColor1X	// Old name for color VZRef ROM.

#define hwrOEMHALIDSZRefMono1X		'sref'			// (Mono 160x220) SZ Reference Platform (PalmSource)
#define hwrOEMHALIDSZRefMonoQX		'srfq'			// (Mono QVGA) SZ Reference Platform (PalmSource)
#define hwrOEMHALIDSZRefMono2X		'srfd'			// (Mono 320x440) SZ Reference Platform (PalmSource)

#define hwrOEMHALIDSZRefColor1X		'csrf'			// (Color 160x220) SZ Reference Platform (PalmSource)
#define hwrOEMHALIDSZRefColorQX		'csrq'			// (Color QVGA) SZ Reference Platform (PalmSource)
#define hwrOEMHALIDSZRefColor2X		'csrd'			// (Color 320x440) SZ Reference Platform (PalmSource)

// HALs that belong to hwrOEMCompanyIDPalmDevices
#define hwrOEMHALID328Jerry			'jery'			// Palm VII HAL (Palm Computing)
#define hwrOEMHALID328Rocky			'rcky'			// Pilot, PalmPilot, Palm III HAL (Palm Computing)
#define hwrOEMHALIDEZSumo			'sumo'			// Palm IIIx/V/Vx HAL (Palm Computing)
#define hwrOEMHALIDEZAustin			'astn'			// Palm IIIc (Palm Computing)
#define hwrOEMHALIDEZCalvin			'clvn'			// Palm m100 (Palm Computing)


#endif 	//__HWR_MISCFLAGS_H__

#endif 	// NON_PORTABLE
