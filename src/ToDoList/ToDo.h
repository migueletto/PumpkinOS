/******************************************************************************
 *
 * Copyright (c) 1995-2002 PalmSource, Inc. All rights reserved.
 *
 * File: ToDo.h
 *
 * Release: Palm OS Developer Suite 5 SDK (68K) 4.0
 *
 * Description:
 *		Header for To Do.
 *
 *****************************************************************************/

#include <IMCUtils.h>
#include <ExgMgr.h>
#include <PdiLib.h>


#include "ToDoDB.h"

#ifdef __GNUC__
#include "sections.h"
#endif

#ifndef __GNUC__
//#define EXTRA_SECTION_TWO
#endif
/***********************************************************************
 *
 *	Internal Constants
 *
 ***********************************************************************/
#define toDoVersionNum					3
#define toDoPrefsVersionNum			3
#define todoPrefID						0x00
#define toDoDBName						"ToDoDB"
#define toDoDBType						'DATA'


/************************************************************
 * Application specific launch codes
 * ABa 04/07/00
 *************************************************************/
enum {
	todoLaunchCmdImportVObject = sysAppLaunchCmdCustomBase
};



/************************************************************
 * Function Prototypes
 *************************************************************/
#ifdef __cplusplus
extern "C" {
#endif


// From ToDoTransfer.c
typedef Boolean ImportVEventF (DmOpenRef dbP, UInt16 pdiRefNum, PdiReaderType* reader, 
	Boolean obeyUniqueIDs, Boolean beginAlreadyRead, UInt32 * uniqueIDP);
	
extern Boolean ToDoImportVToDo(DmOpenRef dbP, UInt16 pdiRefNum, PdiReaderType* reader, 
   Boolean obeyUniqueIDs, Boolean beginAlreadyRead, UInt32 * uniqueIDP) EXTRA_SECTION_TWO;


extern void ToDoSetGoToParams (DmOpenRef dbP, ExgSocketPtr exgSocketP, UInt32 uniqueID) EXTRA_SECTION_TWO;
extern void ToDoSendRecord (DmOpenRef dbP, Int16 recordNum,
	const Char * const prefix, UInt16 noDataAlertID) EXTRA_SECTION_TWO;
extern void ToDoSendCategory (DmOpenRef dbP, UInt16 categoryNum,
	const Char * const prefix, UInt16 noDataAlertID) EXTRA_SECTION_TWO;
extern Err ToDoReceiveData(DmOpenRef dbP, ExgSocketPtr exgSocketP) EXTRA_SECTION_TWO;

extern Boolean ToDoImportVCal(DmOpenRef dbP, UInt16 pdiRefNum, PdiReaderType* reader, 
	Boolean obeyUniqueIDs, Boolean beginAlreadyRead, ImportVEventF vEventFunc) EXTRA_SECTION_TWO;

extern void ToDoExportVCal(DmOpenRef dbP, Int16 index, ToDoDBRecordPtr recordP, 
	UInt16 pdiRefNum, PdiWriterType* writer, Boolean writeUniqueIDs) EXTRA_SECTION_TWO;


extern void SetDBBackupBit(DmOpenRef dbP) EXTRA_SECTION_TWO;


#ifdef __cplusplus 
}
#endif
