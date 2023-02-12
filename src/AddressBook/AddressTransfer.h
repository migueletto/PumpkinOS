/******************************************************************************
 *
 * Copyright (c) 2000 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: AddressTransfer.h
 *
 * Release: Palm OS SDK 4.0 (63220)
 *
 *****************************************************************************/

#ifndef ADDRESSTRANSFER_H
#define ADDRESSTRANSFER_H

#include <PdiLib.h>


/************************************************************
 * Function Prototypes
 *************************************************************/

void	TransferRegisterData ();
void	TransferSendRecord (DmOpenRef dbP, Int16 recordNum, const Char * const prefix, UInt16 noDataAlertID);
void	TransferSendCategory (DmOpenRef dbP, UInt16 categoryNum, const Char * const prefix, UInt16 noDataAlertID);
Err		TransferReceiveData(DmOpenRef dbP, ExgSocketPtr obxSocketP);
Boolean TransferImportVCard(DmOpenRef dbP, UInt16 pdiRefNum, PdiReaderType* reader, Boolean obeyUniqueIDs, Boolean beginAlreadyRead);
void	TransferExportVCard(DmOpenRef dbP, Int16 index, AddrDBRecordType *recordP, UInt16 pdiRefNum, PdiWriterType* writer, Boolean writeUniqueIDs);
void	TransferPreview(ExgPreviewInfoType *infoP);

#endif // ADDRESSTRANSFER_H
