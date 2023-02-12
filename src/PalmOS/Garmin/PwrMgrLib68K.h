/*********************************************************************
*
*   HEADER NAME:
*       PwrMgrLib68K.h - 68K version of the library functions
*
* Copyright 2003 by Garmin Ltd. or its subsidiaries.
*
*********************************************************************/

#ifndef __PwrMgrLIB68K_H__
#define __PwrMgrLIB68K_H__

#include <PalmTypes.h>
#include <LibTraps.h>
#include "PwrMgrLib.h"
#include "PwrMgrLibSysTrapNums.h"

/********************************************************************
 * Prototypes
 ********************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
*
*   PROCEDURE NAME:
*       PwrMgrLibClose - Close PwrMgr
*
*   DESCRIPTION:
*       Closes PwrMgrLib and disposes of the global data memory if
*       required. Called by any application or library that's been
*       using PwrMgrLib and is now finished with it.
*
*********************************************************************/
Err PwrMgrLibClose( UInt16 refnum )
		SYS_TRAP( pwrMgrLibTrapClose );

/*********************************************************************
*
*   PROCEDURE NAME:
*       PwrMgrLibOpen - Open PwrMgrLib
*
*   DESCRIPTION:
*       Opens PwrMgrLib and prepares it for use. Called by any application or
*       library that wants to use the services that PwrMgrLib provides.
*
*********************************************************************/
Err PwrMgrLibOpen( UInt16 refnum )
		SYS_TRAP( pwrMgrLibTrapOpen );

/*********************************************************************
*
*   PROCEDURE NAME:
*       PwrSetLowPowerMode - Set Low Power Mode
*
*   DESCRIPTION:
*		Returns true if successful.
*
*********************************************************************/
Boolean PwrSetLowPowerMode( UInt16 refNum, const UInt32 creator, const Boolean enable )
		SYS_TRAP( pwrSetLowPowerMode );
		
#ifdef __cplusplus
}
#endif

#endif  //__PwrMgrLIB68K_H__




	


