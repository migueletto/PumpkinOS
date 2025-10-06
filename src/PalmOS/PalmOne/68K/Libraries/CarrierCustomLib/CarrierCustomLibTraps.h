/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/**
 * @ingroup Carrier
 */

/**
 * @file 	CarrierCustomLibTraps.h
 * @version 1.0
 * @date 	12/11/2002
 *
 * @brief This file contains the trap for the Carrier Customization
 *        Library.
 *
 */

/*
 * @author    dla
 */


#ifndef __CARRIER_CUSTOMIZATION_LIBRARY_TRAPS_H__
#define __CARRIER_CUSTOMIZATION_LIBRARY_TRAPS_H__


/**
 * @name Function Trap Numbers
 * @brief Trap IDs for the Carrier Customization Library's public functions. The order of the traps
 *        must be the same as the routines are listed in NetPrefLibDispatchTable.c.
 */
/*@{*/
#define CCustomLibTrapVersionGet                   (sysLibTrapCustom + 0)
#define CCustomLibTrapGetCarrierSetting            (sysLibTrapCustom + 1)
#define CCustomLibTrapGetProfileSetting            (sysLibTrapCustom + 2)
#define CCustomLibTrapGetCarrierProfile            (sysLibTrapCustom + 3)
#define CCustomLibTrapReleaseCarrierProfile        (sysLibTrapCustom + 4)
#define CCustomLibTrapGetCarrierCode               (sysLibTrapCustom + 5)
#define CCustomLibTrapGetCarrierCodeLength         (sysLibTrapCustom + 6)
#define CCustomLibTrapRefresh                      (sysLibTrapCustom + 7)
#define CCustomLibTrapRetrieveNVSetting            (sysLibTrapCustom + 8)
/*@}*/
#endif // __CARRIER_CUSTOMIZATION_LIBRARY_TRAPS_H__
