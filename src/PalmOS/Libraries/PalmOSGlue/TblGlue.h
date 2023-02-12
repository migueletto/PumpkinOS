/******************************************************************************
 *
 * Copyright (c) 2000-2003 PalmSource, Inc. All rights reserved.
 *
 * File: TblGlue.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *    Glue providing compatibility for applications that wish to make calls to
 *    some recent table functions, but which might actually be running on a
 *    system which does not support newer calls.
 *
 *****************************************************************************/

#ifndef __TBLGLUE_H__
#define __TBLGLUE_H__

#include <Table.h>

#ifdef __cplusplus
extern "C" {
#endif

extern Int16 TblGlueGetNumberOfColumns (const TableType *tableP);
extern Int16 TblGlueGetTopRow (const TableType *tableP);
extern void TblGlueSetSelection (TableType *tableP, Int16 row, Int16 column);

extern Boolean TblGlueGetColumnMasked (const TableType *tableP, Int16 column);

extern void* TblGlueGetItemPtr(const TableType* tableP, Int16 row, Int16 column); // for <3.5 compatibility

#ifdef __cplusplus
}
#endif

#endif
