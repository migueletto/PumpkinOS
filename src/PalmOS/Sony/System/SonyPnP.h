/******************************************************************************
 *                                                                            *
 *            (C) Copyright 2000-2002, Sony Corporation                       *
 *                                                                            *
 *----------------------------------------------------------------------------*
 *                                                                            *
 *    <IDENTIFICATION>                                                        *
 *       file name    : $Workfile: SonyPnP.h $
 *                                                                            *
 *    <PROFILE>                                                               *
 *       Type/DataStructure of PnP (Plug and Play)                            *
 *                                                                            *
 *    <HISTORY>                                                               *
 *       Started on   : 02/03/03                                              *
 *       Last Modified: $Date: 03/01/27 9:37 $
 *                                                                            *
 ******************************************************************************/
/* this file is best viewed by setting TAB-stop as 3 */

#ifndef __SONYPNP_H__
#define __SONYPNP_H__

/******************************************************************************
 *    Includes                                                                *
 ******************************************************************************/
#include <SonyTypes.h>

/******************************************************************************
 *    Constants                                                               *
 ******************************************************************************/
/*** PnP Notification Priority ***/
#define sysNotifyPnpAttachPifPriority					(18)
#define sysNotifyPnpDetachPifPriority					(-18)
#define sysNotifyPnpAttachSpecificDrvPriorityHigh	(-10)
#define sysNotifyPnpAttachSpecificDrvPriorityLow	(-1)
#define sysNotifyPnpAttachNormalDrvPriority			(0)
#define sysNotifyPnpDetachNormalDrvPriority			(0)
#define sysNotifyPnpAttachGeneralDrvPriorityHigh	(1)
#define sysNotifyPnpAttachGeneralDrvPriorityLow		(10)

/*** pifClass ***/
typedef enum {
	sysPnpPifClassEnumAny = 0x0000,
	sysPnpPifClassEnumCF = 0x0001,
	sysPnpPifClassEnumMS = 0x0002,
	sysPnpPifClassEnumUSB = 0x0003,
	sysPnpPifClassEnumExtCnt = 0x0004,
	sysPnpPifClassEnumInternal,
	sysPnpPifClassEnumUnknown = 0xFFFF
} SysPnPPifClassEnumType, *SysPnPPifClassEnumPtr;

#define sysPnpPifClassAny		sysPnpPifClassEnumAny
#define sysPnpPifClassCF		sysPnpPifClassEnumCF
#define sysPnpPifClassMS		sysPnpPifClassEnumMS
#define sysPnpPifClassUSB		sysPnpPifClassEnumUSB
#define sysPnpPifClassExtCnt	sysPnpPifClassEnumExtCnt
#define sysPnpPifClassInternal	sysPnpPifClassEnumInternal
#define sysPnpPifClassUnknown	sysPnpPifClassEnumUnknown

/*  --- old type --- */
	/* alias */
#define pnpPifClassCF			sysPnpPifClassCF
#define pnpPifClassMS			sysPnpPifClassMS
#define pnpPifClassUSB			sysPnpPifClassUSB
#define pnpPifClassExtCnt		sysPnpPifClassExtCnt
/*  --- --- */

/*** PnPCategory */
typedef enum {
	sysPnPCategoryEnumAny = 0,
	sysPnPCategoryEnumMemory,
	sysPnPCategoryEnumNetwork,
	sysPnPCategoryEnumModem,
	sysPnPCategoryEnumSerial,
	sysPnPCategoryEnumParallel,
	sysPnPCategoryEnumBluetooth,
	sysPnPCategoryEnumGPS,
	sysPnPCategoryEnumCamera,
	sysPnPCategoryEnumVideo,
	sysPnPCategoryEnumSecurity,
	sysPnPCategoryEnumInstrument,
	sysPnPCategoryEnumInput,
	sysPnPCategoryEnumAdaptor,
	sysPnPCategoryEnumAVAdaptor,
	sysPnPCategoryEnumTuner,
	sysPnPCategoryEnumPrinter,
	sysPnPCategoryEnumCDROM,
	sysPnPCategoryEnumVendor = 0xFFFE,
	sysPnPCategoryEnumUnknown = 0xFFFF
} SysPnPCategoryEnumType; 

/*** string length ***/
#define sysPnPDriverInfoStringMaxLen				(31)
#define sysPnPDeviceInfoStringMaxLen				(31)
#define sysPnPDriverDescStringMaxLen				(255)

/*** Launch code ***/
#define sysAppLaunchCmdPnPBase				(sysAppLaunchCmdCustomBase + 0x1000)
#define sysAppLaunchCmdPnPCallback			(sysAppLaunchCmdPnPBase + 0)

/*** driver callback Launch code ***/
#define	sysPnPDrvCmdSetup							(0)
#define	sysPnPDrvCmdDelete							(1)
#define	sysPnPDrvCmdReinstall						(2)

/*** flags ***/
	/* device */
#define sysPnPDeviceAttrPIF				(0x00000001)
#define sysPnPDeviceAttrNoDriver		(0x00000002)
	/* driver */
#define sysPnPDriverAttrPIF				(0x00000001)
#define sysPnPDriverAttrSetup			(0x00000002)
#define sysPnPDriverAttrDisabled		(0x00000004)
#define	sysPnPDriverAttrActive			(0x00000008)
#define	sysPnPDriverAttrGeneral			(0x00000010)
	/* driver callback */
#define	sysPnPDrvCmdSetupFlagNoAppl		(0x00000001)

/*** error ***/
#define pnpErrUnsupportedCommand		(0x00000001)

/*** macros ***/
#define	sysPnPSetDriverVersion(x,y,z)		(x << 24 | y << 16 | z << 8 | 0x00)
#define	sysPnPSetDriverVersionS(x,y,z,s)	(sysPnPSetDriverVersion(x,y,z) | s)
#define	sysPnPSetDriverDate(x,y,z)			(x << 16 | y << 8 | z)

/******************************************************************************
 *    Type/DataStructure of Notifications                                     *
 ******************************************************************************/
/*** for sysNotifyPnPDeviceAttachedEvent & sysNotifyPnPDeviceDetachedEvent */
	/* (void *)notifyDetailsP is pointer to SysNotifyPnPDeviceType */
/*
typedef struct {
	UInt16 pifClass;
	UInt16 pifId;
	UInt32 infoSize;
	void *infoP;
	UInt32 reserved;
} SysNotifyPnPDeviceType;
*/

typedef struct SysNotifyPnPDeviceTag {
	SysPnPPifClassEnumType pifClass;	/* Physical I/F Class (MS/CF/USB/ExtCnt/c) */
	UInt16 pifId;		/* SlotNo for MS/CF, RootPortNo for USB, */
	UInt32 infoSize;	/* size of data pointed by infoP */
	void *infoP;		/* pointer to data supplied by DeviceDriver */
	UInt32 rsv;
} SysNotifyPnPDeviceType, *SysNotifyPnPDevicePtr;
typedef SysNotifyPnPDeviceType SysPnPDeviceType;
typedef SysNotifyPnPDevicePtr SysPnPDevicePtr;


/*** pifId ***/
	/* CF: slotRefNum */
	/* MS: slotRefNum */
	/* ExtCnt: unused */

/* infoP points to structures as follows */
	/* CF */
typedef struct {
    UInt16	manfID;
    UInt16	cardID;
} SysPnPDeviceCFManfIDType, *SysPnPDeviceCFManfIDPtr;

typedef struct {
    UInt8	majorVersion;
    UInt8	minorVersion;
    UInt8	numString;
    UInt8	offset[4];
    Char	string[254];
}SysPnPDeviceCFVers1InfoType, *SysPnPDeviceCFVers1InfoPtr;

#define sysPnpCfFuncIdMulti				0x00
#define sysPnpCfFuncIdMemory			0x01
#define sysPnpCfFuncIdSerial			0x02
#define sysPnpCfFuncIdParallel			0x03
#define sysPnpCfFuncIdFixed				0x04
#define sysPnpCfFuncIdVideo				0x05
#define sysPnpCfFuncIdNetwork			0x06
#define sysPnpCfFuncIdAims				0x07
#define sysPnpCfFuncIdScsi				0x08
#define sysPnpCfFuncIdSecurity			0x09
#define sysPnpCfFuncIdInstruments		0x0A
#define sysPnpCfFuncIdSerialIOBus		0x0B
#define sysPnpCfFuncIdVenderSpecific	0xFE
#define sysPnpCfFuncIdDoNotUse			0xFF

#define sysPnpCfSysInitPost	0x01
#define sysPnpCfSysInitRom	0x02

typedef struct {
    UInt8	function;
    UInt8	sysinit;
} SysPnPDeviceCFFuncIDType, *SysPnPDeviceCFFuncIDPtr;

typedef struct {
	SysPnPDeviceCFManfIDType	manfID;
	SysPnPDeviceCFVers1InfoType	ver1Info;
	UInt8						numFunc;
	SysPnPDeviceCFFuncIDType	funcID[8];
}SysPnPDeviceCFInfoType, *SysPnPDeviceCFInfoPtr;
	/* MS */
typedef struct {
	UInt8 typeNum;
	UInt8 categoryNum;
	UInt8 classNum;
}SysPnPDeviceMSFuncType, *SysPnPDeviceMSFuncPtr;

typedef struct {
	Char companyName[33];
	Char productName[33];
	Char versionNum[17];
} SysPnPDeviceMSAttributeInfoType, *SysPnPDeviceMSAttributeInfoPtr;

typedef struct {
	SysPnPDeviceMSAttributeInfoType	attrInfo;
	UInt8							numFunc;
	SysPnPDeviceMSFuncType			funcID[8];
}SysPnPDeviceMSInfoType, *SysPnPDeviceMSInfoPtr;

	/* ExtCnt */
typedef struct {
	 UInt8  venderId;		// What type of event occurred?
	 UInt8  modelId;		// normally creator code of broadcasting app
	 UInt8  rsv1;			// ptr to notification-specific data, if any
	 UInt8  versionNo;	// user specified ptr passed back with notification
	 UInt8  feature;		// true if event is handled yet
	 UInt8  rsv2;
	 UInt8  rsv3;
	 EventType *eventP;
} SysPnPDeviceExtCntInfoType;
typedef SysPnPDeviceExtCntInfoType *SysPnPDeviceExtCntInfoPtr;
typedef SysPnPDeviceExtCntInfoType SonyPnPDeviceExtCntInfoType;
typedef SonyPnPDeviceExtCntInfoType *SonyPnPDeviceExtCntInfoPtr;


/*** device info ***/
typedef struct SysPnPDeviceInfoTypeTag {
	UInt16 revision;
	Char maker[sysPnPDeviceInfoStringMaxLen+1];
	Char product[sysPnPDeviceInfoStringMaxLen+1];
	Char type[sysPnPDriverInfoStringMaxLen+1];
	SysPnPPifClassEnumType pifClass;
	UInt16 pifId;
	UInt32 flags;
	SysPnPDeviceType *pnpDeviceP;
	UInt32 creator;
	UInt32 dbType;
	UInt16 bmpRscID;
	UInt16 reserve0;
	UInt32 reserve1;
} SysPnPDeviceInfoType, *SysPnPDeviceInfoPtr;

typedef struct SysPnPDeviceInfoListTypeTag {
	UInt16 revision;
	struct SysPnPDeviceInfoListTypeTag *nextP;
	SysPnPDeviceInfoType *deviceP;
} SysPnPDeviceInfoListType, *SysPnPDeviceInfoListPtr;


/*** driver info ***/
typedef struct {
	MemHandle dbH;
	UInt32 reserve0;
} SysPnPDriverCallbackArgType, *SysPnPDriverCallbackArgPtr;

typedef struct {	/* used with LaunchCmdPnPCallback, pointed by cmdPBP */
	UInt32 cmd;
	SysPnPDriverCallbackArgPtr argP;
	void *valueP;
	UInt32 *valueLenP;
	void *userDataP;
} SysPnPDriverLaunchArgType, *SysPnPDriverLaunchArgPtr;

typedef Err (*SysPnPDriverProcPtr)(UInt32, SysPnPDriverLaunchArgPtr, 
									void *, UInt32 *, void *);
								// (cmd, argP, valueP, valueLenP, userDataP)

typedef struct SysPnPDriverInfoTypeTag {
	UInt16 revision;
	Char name[sysPnPDriverInfoStringMaxLen+1];
	Char provider[sysPnPDriverInfoStringMaxLen+1];
	UInt32 version;
	UInt32 date;
	Char description[sysPnPDriverDescStringMaxLen+1];
	SysPnPCategoryEnumType category;
	Char categoryName[sysPnPDriverInfoStringMaxLen+1];  // used when vendor-unique
	SysPnPPifClassEnumType pifClass;
	UInt16 pifId;
	UInt32 flags;
	SysPnPDriverProcPtr callbackP;
	MemHandle dbH;
	void *userDataP;
	UInt32 creator;
	UInt32 dbType;
	UInt16 bmpRscID;
	UInt8	functionNo;
	UInt8	reserve0;
	UInt32 reserve1;
} SysPnPDriverInfoType, *SysPnPDriverInfoPtr;

typedef struct SysPnPDriverInfoListTypeTag {
	UInt16 revision;
	struct SysPnPDriverInfoListTypeTag *nextP;
	SysPnPDriverInfoType *driverP;
} SysPnPDriverInfoListType, *SysPnPDriverInfoListPtr;

typedef struct SysPnPDevice2DriverTypeTag {
	SysPnPDeviceType *deviceP;
	SysPnPDriverInfoListType *driverListP;
	UInt32 reserve;
} SysPnPDevice2DriverType, *SysPnPDevice2DriverPtr;


/*** handled ***/
	/* CF */
#define sysPnpHandledCFFunc0	(0x01)
#define sysPnpHandledCFFunc1	(0x02)
#define sysPnpHandledCFFunc2	(0x04)
#define sysPnpHandledCFFunc3	(0x08)
#define sysPnpHandledCFFunc4	(0x10)
#define sysPnpHandledCFFunc5	(0x20)
#define sysPnpHandledCFFunc6	(0x40)
#define sysPnpHandledCFFunc7	(0x80)

	/* MS */
#define sysPnpHandledMSFunc0	(0x01)
#define sysPnpHandledMSFunc1	(0x02)
#define sysPnpHandledMSFunc2	(0x04)
#define sysPnpHandledMSFunc3	(0x08)
#define sysPnpHandledMSFunc4	(0x10)
#define sysPnpHandledMSFunc5	(0x20)
#define sysPnpHandledMSFunc6	(0x40)
#define sysPnpHandledMSFunc7	(0x80)

	/* ExtCnt */

#endif	// __SONYPNP_H__

