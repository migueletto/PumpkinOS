#include <PalmOS.h>

#include "debug.h"

Int16 TblGlueGetNumberOfColumns(const TableType *tableP) {
  return TblGetNumberOfColumns(tableP);
}

Int16 TblGlueGetTopRow(const TableType *tableP) {
  return TblGetTopRow(tableP);
}

void TblGlueSetSelection(TableType *tableP, Int16 row, Int16 column) {
  TblSetSelection(tableP, row, column);
}

Boolean TblGlueGetColumnMasked(const TableType *tableP, Int16 column) {
  Boolean masked = false;

  if (tableP && column >= 0 && column < tableP->numColumns) {
    masked = tableP->columnAttrs[column].masked;
  }

  return masked;
}

void *TblGlueGetItemPtr(const TableType* tableP, Int16 row, Int16 column) {
  return TblGetItemPtr(tableP, row, column);
}
