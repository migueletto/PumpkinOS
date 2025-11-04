/******************************************************************************
 *
 * Copyright (c) 1997-2000 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: DateTransfer.h
 *
 * Release: Palm OS SDK 4.0 (63220)
 *
 * Description:
 *	  This file defines the datebook transfer functions.
 *
 * History:
 *		September 12, 1997	Created by Roger Flores
 *  	06/26/2000 ABa Integrate Pdi library
 *
 *****************************************************************************/

#include <PdiLib.h>

typedef Boolean ImportVToDoF(DmOpenRef dbP, UInt16 pdiRefNum, PdiReaderType* reader, 
    Boolean obeyUniqueIDs, Boolean beginAlreadyRead);

/************************************************************
 * Function Prototypes
 *************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

extern void DateSendRecord (DmOpenRef dbP, UInt16 recordNum, const Char * const prefix);

extern void DateSendCategory (DmOpenRef dbP, UInt16 categoryNum, const Char * const prefix);

extern Err DateReceiveData(DmOpenRef dbP, ExgSocketPtr exgSocketP);

extern void DateExportVCal(DmOpenRef dbP, Int16 index, ApptDBRecordPtr recordP,
    UInt16 pdiRefNum, PdiWriterType* writer, Boolean writeUniqueIDs);

extern Boolean DateImportVCal(DmOpenRef dbP, UInt16 pdiRefNum, PdiReaderType* reader, 
    Boolean obeyUniqueIDs, Boolean beginAlreadyRead, ImportVToDoF vToDoFunc);

extern void	DateTransferPreview(ExgPreviewInfoType *infoP);


#ifdef __cplusplus 
}
#endif
