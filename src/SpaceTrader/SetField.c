// *************************************************************************
// SetField.c 
// 
// Modifications:
// mm/dd/yy - description - author
// *************************************************************************

#include "external.h"

Handle SetField( FormPtr frm, int Nr, char* Value, int Size, Boolean Focus )
{
	Word objIndex;
	CharPtr AmountP;
	Handle AmountH;
	
	objIndex = FrmGetObjectIndex( frm, Nr );
	AmountH = MemHandleNew( Size );
	AmountP = MemHandleLock( AmountH );
	StrCopy( AmountP, Value );
	MemPtrUnlock( AmountP );
	FldSetTextHandle( FrmGetObjectPtr( frm, objIndex ), AmountH );
	if (Focus)
		FrmSetFocus( frm, objIndex );
	
	return AmountH;
}
