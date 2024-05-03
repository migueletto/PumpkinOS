/***********************************************************************
 *
 * SPACE TRADER 1.2.0
 *
 * Field.c
 *
 * Copyright (C) 2000-2002 Pieter Spronck, All Rights Reserved
 *
 * Additional coding by Sam Anderson (rulez2@home.com)
 * Additional coding by Samuel Goldstein (palm@fogbound.net)
 *
 * Some code of Matt Lee's Dope Wars program has been used.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * 
 * You can contact the author at space_trader@hotmail.com
 *
 * For those who are familiar with the classic game Elite: many of the
 * ideas in Space Trader are heavily inspired by Elite.
 *
 **********************************************************************/

// *************************************************************************
// Field.c - Functions Include:
// Handle SetField( FormPtr frm, int Nr, char* Value, int Size, Boolean Focus )
// void GetField( FormPtr frm, int Nr, char* Value, Handle AmountH )
// void SetCheckBox( FormPtr frm, int Nr, Boolean Value )
// Boolean GetCheckBox( FormPtr frm, int Nr )
//
// Modifications:
// mm/dd/yy - description - author
// *************************************************************************

#include "external.h"

// *************************************************************************
// Set Field
// *************************************************************************
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


// *************************************************************************
// Get Field
// *************************************************************************
void GetField( FormPtr frm, int Nr, char* Value, Handle AmountH )
{
	Word objIndex;
	CharPtr AmountP;

	objIndex = FrmGetObjectIndex( frm, Nr );
	FldSetTextHandle( FrmGetObjectPtr( frm, objIndex ), 0 );
	AmountP = MemHandleLock( AmountH );
	StrCopy( Value, AmountP );
	MemPtrUnlock( AmountP );
	MemHandleFree( AmountH );
}

// *************************************************************************
// Set Trigger List value
// *************************************************************************
void SetTriggerList( FormPtr frm, int Nr, int Index )
{
	Word objIndex;
	ListPtr lp;
	
	objIndex = FrmGetObjectIndex( frm, Nr );
	lp = (ListPtr)FrmGetObjectPtr( frm, objIndex );
	LstSetSelection( lp, Index );
}

// *************************************************************************
// Set Control Label
// *************************************************************************
void SetControlLabel( FormPtr frm, int Nr, Char * Label )
{
	Word objIndex;
	ControlPtr cp;
	
	objIndex = FrmGetObjectIndex( frm, Nr );
	cp = (ControlPtr)FrmGetObjectPtr( frm, objIndex );
	CtlSetLabel( cp, Label );
}

// *************************************************************************
// Get Trigger List value
// *************************************************************************
int GetTriggerList( FormPtr frm, int Nr)
{
	Word objIndex;
	ListPtr lp;
	
	objIndex = FrmGetObjectIndex( frm, Nr );
	lp = (ListPtr)FrmGetObjectPtr( frm, objIndex );
	return LstGetSelection( lp );
}


// *************************************************************************
// Set Checkbox value
// *************************************************************************
void SetCheckBox( FormPtr frm, int Nr, Boolean Value )
{
	Word objIndex;
	ControlPtr cp;
	
	objIndex = FrmGetObjectIndex( frm, Nr );
	cp = (ControlPtr)FrmGetObjectPtr( frm, objIndex );
	CtlSetValue( cp, (Value ? 1 : 0) );
}


// *************************************************************************
// Get Checkbox value
// *************************************************************************
Boolean GetCheckBox( FormPtr frm, int Nr )
{
	Word objIndex;
	ControlPtr cp;

	objIndex = FrmGetObjectIndex( frm, Nr );
	cp = (ControlPtr)FrmGetObjectPtr( frm, objIndex );
	if (CtlGetValue( cp ) == 0)
		return false;
	else
		return true;
}
