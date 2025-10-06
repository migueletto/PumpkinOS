/******************************************************************************
 *
 * Copyright (c) 1995-2002 PalmSource, Inc. All rights reserved.
 *
 * File: SampleMain.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *		Include file the Sample application
 *
 *****************************************************************************/

#ifndef __SAMPLEMAIN_H__
#define	__SAMPLEMAIN_H__

#include <IMCUtils.h>
#include <ExgMgr.h>

#define kAppCreator                     'SaDo'		// registered creator ID
#define itemDBName						"SampleDB-SaDo"
#define itemDBType						'DATA'
#define itemMaxLength					4096		// note: must be same as tFLD 1109 max length!!!
													// dje - Was 8192. Changed to fix bug #24574.
#define sampleExtension					"txt"
#define sampleMIMEType					"text/plain"


/************************************************************
 * Function Prototypes
 *************************************************************/
#ifdef __cplusplus
extern "C" {
#endif


typedef UInt32 ReadFunctionF (const void * stream, Char * bufferP, UInt32 length);
typedef UInt32 WriteFunctionF (void * stream, const Char * const bufferP, Int32 length);


// From SampleTransfer.c
extern void SampleSendRecord (DmOpenRef dbP, Int16 recordNum, const Char * const prefix);

extern void SampleSendCategory (DmOpenRef dbP, UInt16 categoryNum, const Char * const prefix, UInt16 noDataAlertID);

extern Err SampleReceiveData(DmOpenRef dbP, ExgSocketPtr exgSocketP, UInt16 *numRecordsReceived);
extern void SampleTransferPreview(ExgPreviewInfoType *infoP);

extern Boolean SampleImportMime(DmOpenRef dbR, void * inputStream, ReadFunctionF inputFunc, 
	Boolean obeyUniqueIDs, Boolean beginAlreadyRead, UInt16 *numRecordsReceivedP, Char* descriptionP, UInt16 descriptionSize);

extern void SampleExportMime(DmOpenRef dbP, Int16 index, ItemDBRecordType *recordP, 
	void * outputStream, WriteFunctionF outputFunc, 
	Boolean writeUniqueIDs, Boolean outputMimeInfo);
	
extern void SetDBBackupBit(DmOpenRef dbP);

#ifdef __cplusplus 
}
#endif

#endif	//	__SAMPLEMAIN_H__
