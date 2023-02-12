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

void DateSendRecord (DmOpenRef dbP, UInt16 recordNum, const Char * const prefix) EXTRA_SECTION_TWO;

void DateSendCategory (DmOpenRef dbP, UInt16 categoryNum, const Char * const prefix) EXTRA_SECTION_TWO;

Err DateReceiveData(DmOpenRef dbP, ExgSocketPtr exgSocketP) EXTRA_SECTION_TWO;

void DateExportVCal(DmOpenRef dbP, Int16 index, ApptDBRecordPtr recordP,
    UInt16 pdiRefNum, PdiWriterType* writer, Boolean writeUniqueIDs) EXTRA_SECTION_TWO;

Boolean DateImportVCal(DmOpenRef dbP, UInt16 pdiRefNum, PdiReaderType* reader, 
    Boolean obeyUniqueIDs, Boolean beginAlreadyRead, ImportVToDoF vToDoFunc) EXTRA_SECTION_TWO;

void	DateTransferPreview(ExgPreviewInfoType *infoP) EXTRA_SECTION_TWO;


#ifdef __cplusplus 
}
#endif
