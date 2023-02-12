/*********************************************************************
*
*   HEADER NAME:
*       PwrMgrLib.h - Library Functions
*
* Copyright 2003 by Garmin Ltd. or its subsidiaries.
*
*********************************************************************/
#ifndef __PwrMgrLIB_H__
#define __PwrMgrLIB_H__

/*-----------------------------------------------------------------------------
                                GENERAL INCLUDES
-----------------------------------------------------------------------------*/
#include <PalmTypes.h>
#include <SystemMgr.h>

/*-----------------------------------------------------------------------------
                                LITERAL CONSTANTS
-----------------------------------------------------------------------------*/
#define kPwrMgrLibType     				sysFileTLibrary
#define kPwrMgrLibCreator  				'pwrL'
#define kPwrMgrLibName     				"PwrMgrLib"
#define PwrMgrLibAPIVersion            ( sysMakeROMVersion( 3, 5, 0, sysROMStageRelease, 0 ) )

#endif  //__PwrMgrLIB_H__
