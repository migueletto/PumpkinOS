/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/**
 * @ingroup NETMASTER
 */

/**
 * @file 	NetMasterLibTraps.h
 * @version 1.0
 * @date    01/31/2002
 *
 * @brief Trap definitions for the NetMaster Library.
 *
 */


#ifndef __NET_MASTER_LIB_TRAPS_H__
#define __NET_MASTER_LIB_TRAPS_H__


// Trap IDs for the NetMaster Library's public functions. The order of the traps
// must be the same as the routines are listed in NetMasterLibDispatchTable.c.
/**
 *@name NetMaster Function Trap Numbers
 */
/*@{*/
#define netMasterLibTrapLibVersionGet					(sysLibTrapCustom + 0)
#define netMasterLibTrapSCNetAttach						(sysLibTrapCustom + 1)
#define netMasterLibTrapLibInit							(sysLibTrapCustom + 2)
#define netMasterLibTrapAutoLoginSettingGet				(sysLibTrapCustom + 3)
#define netMasterLibTrapAutoLoginSettingSet				(sysLibTrapCustom + 4)
#define netMasterLibTrapServiceSetUp					(sysLibTrapCustom + 5)
#define netMasterLibTrapNetLibOpenIfFullyUp				(sysLibTrapCustom + 6)
#define netMasterLibTrapNetLibIsFullyUp					(sysLibTrapCustom + 7)
#define netMasterLibTrapPhoneServiceClassesGet			(sysLibTrapCustom + 8)
#define netMasterLibTrapHandlePhoneEvent				(sysLibTrapCustom + 9)
#define netMasterLibTrapControl							(sysLibTrapCustom + 10)
#define netMasterLibTrapNetInterfacesShutDown			(sysLibTrapCustom + 11)
#define netMasterLibTrapNetLibIsLoggingIn				(sysLibTrapCustom + 12)
#define netMasterLibTrapNetLibOpenWithOptions			(sysLibTrapCustom + 13)
#define netMasterLibTrapImplementNetLibOpenConfig		(sysLibTrapCustom + 14)
#define netMasterLibTrapImplementNetLibConnectionRefresh (sysLibTrapCustom + 15)
#define netMasterLibTrapImplementNetLibHandlePowerOff	(sysLibTrapCustom + 16)
#define netMasterLibTrapOverrideErrorGetAndClear		(sysLibTrapCustom + 17)
#define netMasterLibTrapOverrideMdmInitStrGetPtr		(sysLibTrapCustom + 18)
#define netMasterLibTrapLoginSMFlagsSetAndGet			(sysLibTrapCustom + 19)
#define netMasterLibTrapDataSessionErrorNotify			(sysLibTrapCustom + 20)
#define netMasterLibTrapSCNetDetach						(sysLibTrapCustom + 21)
#define netMasterLibTrapSCSessionIDGetFromContext		(sysLibTrapCustom + 22)
#define netMasterLibTrapSCSessionCountGet				(sysLibTrapCustom + 23)
#define netMasterLibTrapSCSessionsEnumerate				(sysLibTrapCustom + 24)
#define netMasterLibTrapSCSessionSvcRecIDGet			(sysLibTrapCustom + 25)
#define netMasterLibTrapSCSessionIsActive				(sysLibTrapCustom + 26)
#define netMasterLibTrapSCSessionShutDown				(sysLibTrapCustom + 27)
#define netMasterLibTrapLoginExecuteFromUI				(sysLibTrapCustom + 28)
#define netMasterLibTrapSCContextAnchorTimeoutSet		(sysLibTrapCustom + 29)
#define netMasterLibTrapPPPPayloadReport				(sysLibTrapCustom + 30)
#define netMasterLibTrapDataCounterGetAndReset			(sysLibTrapCustom + 31)
/*@}*/

#endif /* __NET_MASTER_LIB_TRAPS_H__ */
