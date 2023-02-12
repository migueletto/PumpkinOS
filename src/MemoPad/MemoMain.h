/******************************************************************************
 *
 * Copyright (c) 1995-2002 PalmSource, Inc. All rights reserved.
 *
 * File: MemoMain.h
 *
 * Release: Palm OS Developer Suite 5 SDK (68K) 4.0
 *
 * Description:
 *		Include file the Memo application
 *
 *****************************************************************************/

#ifndef 	__MEMOMAIN_H__
#define	__MEMOMAIN_H__

#ifdef __GNUC__
#include "sections.h"
#endif

#ifndef __GNUC__
#define EXTRA_SECTION_ONE
#endif

#include <IMCUtils.h>
#include <ExgMgr.h>

#define memoDBName						"MemoDB"
#define memoDBType						'DATA'
#define memoMaxLength					4096		// note: must be same as tFLD 1109 max length!!!
															// dje - Was 8192. Changed to fix bug #24574.
#define memoExtension					"txt"
#define memoMIMEType						"text/plain"


/************************************************************
 * Function Prototypes
 *************************************************************/
#ifdef __cplusplus
extern "C" {
#endif


typedef UInt32 ReadFunctionF (const void * stream, Char * bufferP, UInt32 length);
typedef UInt32 WriteFunctionF (void * stream, const Char * const bufferP, Int32 length);


// From MemoTransfer.c
extern void MemoSendRecord (DmOpenRef dbP, Int16 recordNum, const Char * const prefix) EXTRA_SECTION_ONE;

extern void MemoSendCategory (DmOpenRef dbP, UInt16 categoryNum, const Char * const prefix, UInt16 noDataAlertID) EXTRA_SECTION_ONE;

extern Err MemoReceiveData(DmOpenRef dbP, ExgSocketPtr exgSocketP, UInt16 *numRecordsReceived) EXTRA_SECTION_ONE;
extern void MemoTransferPreview(ExgPreviewInfoType *infoP) EXTRA_SECTION_ONE;

extern Boolean MemoImportMime(DmOpenRef dbR, void * inputStream, ReadFunctionF inputFunc, 
	Boolean obeyUniqueIDs, Boolean beginAlreadyRead, UInt16 *numRecordsReceivedP, Char* descriptionP, UInt16 descriptionSize) EXTRA_SECTION_ONE;

extern void MemoExportMime(DmOpenRef dbP, Int16 index, MemoDBRecordType *recordP, 
	void * outputStream, WriteFunctionF outputFunc, 
	Boolean writeUniqueIDs, Boolean outputMimeInfo) EXTRA_SECTION_ONE;

extern void SetDBBackupBit(DmOpenRef dbP) EXTRA_SECTION_ONE;

#ifdef __cplusplus 
}
#endif

#endif	//	__MEMOMAIN_H__
