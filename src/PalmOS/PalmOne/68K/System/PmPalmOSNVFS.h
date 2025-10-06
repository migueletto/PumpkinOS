/******************************************************************************
 * Copyright (c) 2005 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/** 
 *@ingroup System
 *
 */
 
/**
 * @file 	PmPalmOSNVFS.h
 * @version 
 * @date 	
 *
 * @brief  
 * 
 *
 * <hr> 
 */


#ifndef __PMPALMOSNVFS_H__
#define __PMPALMOSNVFS_H__

#define sysFtrNumDmAutoBackup		31		/**< Is Data Manager Auto Backup supported? */
#define vfsVolumeAttrNonRemovable   	(0x00000008UL)	/**< media is non-removable */
#define expCapabilityNonRemovable	0x00000008	/**< card is non-removable */
#define expCapabilityHidden		0x00000010	/**< card is hidden */
#define sysTrapDmSyncDatabase		0xA476		/**< 		*/
#define vfsIncludePrivateVolumes	0x80000000	/**< Use this flag to show hidden volumes if the device supports hidden volumes */
#define expIncludePrivateSlots		0x80000000	/**< Use this flag to show hidden expansion slots if the device supports hidden slots */
#define dbCacheFlag			0x8000		/**< Flag is used in query the DbCache size when ORd with Storage heap id */

#ifdef  sysTrapLastTrapNumber

  #undef  sysTrapLastTrapNumber			/**< 		*/
  #define sysTrapLastTrapNumber		0xA477	/**< 		*/

#endif // sysTrapLastTrapNumber

/**
 * @brief Commit changes in the database to the non-volatile memory
 *
 * @param dbRef: 	IN: Reference to the opened database to be synced/committed 
 * @retval Err error code.
 */
Err		DmSyncDatabase (DmOpenRef dbRef)
		SYS_TRAP (sysTrapDmSyncDatabase);
                

#endif // __PMPALMOSNVFS_H__