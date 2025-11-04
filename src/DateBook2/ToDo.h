/******************************************************************************
 *
 * Copyright (c) 1997-2000 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: ToDo.h
 *
 * Release: Palm OS SDK 4.0 (63220)
 *
 * Description:
 *		Header for To Do.
 *
 * History:
 *   	8/28/97  roger - Created
 *		10/4/99	jmp - Added SetDBBackupBit() prototype.
 *
 *****************************************************************************/

#include <IMCUtils.h>
#include <ExgMgr.h>
#include <PdiLib.h>


#include "ToDoDB.h"

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
   Boolean obeyUniqueIDs, Boolean beginAlreadyRead, UInt32 * uniqueIDP);


extern void ToDoSetGoToParams (DmOpenRef dbP, ExgSocketPtr exgSocketP, UInt32 uniqueID);
extern void ToDoSendRecord (DmOpenRef dbP, Int16 recordNum,
	const Char * const prefix, UInt16 noDataAlertID);
extern void ToDoSendCategory (DmOpenRef dbP, UInt16 categoryNum,
	const Char * const prefix, UInt16 noDataAlertID);
extern Err ToDoReceiveData(DmOpenRef dbP, ExgSocketPtr exgSocketP);

extern Boolean ToDoImportVCal(DmOpenRef dbP, UInt16 pdiRefNum, PdiReaderType* reader, 
	Boolean obeyUniqueIDs, Boolean beginAlreadyRead, ImportVEventF vEventFunc);

extern void ToDoExportVCal(DmOpenRef dbP, Int16 index, ToDoDBRecordPtr recordP, 
	UInt16 pdiRefNum, PdiWriterType* writer, Boolean writeUniqueIDs);

	
extern void SetDBBackupBit(DmOpenRef dbP);


#ifdef __cplusplus 
}
#endif
