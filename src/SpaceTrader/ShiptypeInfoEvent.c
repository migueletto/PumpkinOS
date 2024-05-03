/***********************************************************************
 *
 * SPACE TRADER 1.2.0
 *
 * ShiptypeInfoEvent.c
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
// Handling of the events of the Shiptype Information form.
// *************************************************************************

#include "external.h"

static void DrawShiptypeInfoForm()
{
    FormPtr frmP;
	frmP = FrmGetActiveForm();
	setLabelText( frmP, ShiptypeInfoShipNameLabel, 
		Shiptype[SelectedShipType].Name );
	setLabelText( frmP, ShiptypeInfoSizeLabel, 
		SystemSize[Shiptype[SelectedShipType].Size] );
	setLabelText( frmP, ShiptypeInfoCargoBaysLabel, 
		StrIToA( SBuf, Shiptype[SelectedShipType].CargoBays ) );
	setLabelText( frmP, ShiptypeInfoWeaponSlotsLabel, 
		StrIToA( SBuf, Shiptype[SelectedShipType].WeaponSlots ) );
	setLabelText( frmP, ShiptypeInfoShieldSlotsLabel, 
		StrIToA( SBuf, Shiptype[SelectedShipType].ShieldSlots ) );
	setLabelText( frmP, ShiptypeInfoGadgetSlotsLabel, 
		StrIToA( SBuf, Shiptype[SelectedShipType].GadgetSlots ) );
	setLabelText( frmP, ShiptypeInfoCrewQuartersLabel, 
		StrIToA( SBuf, Shiptype[SelectedShipType].CrewQuarters ) );
	StrIToA( SBuf, Shiptype[SelectedShipType].FuelTanks );
	StrCat( SBuf, " parsecs" );
	setLabelText( frmP, ShiptypeInfoMaximumRangeLabel, SBuf ); 
	setLabelText( frmP, ShiptypeInfoHullStrengthLabel, 
		StrIToA( SBuf, Shiptype[SelectedShipType].HullStrength ) );
	FrmDrawForm( frmP );
	
	WinDrawBitmap( ShipBmpPtr[SelectedShipType], 
		94+((60-GetBitmapWidth( ShipBmpPtr[SelectedShipType] ))>>1),
		83+((48-GetBitmapHeight( ShipBmpPtr[SelectedShipType] ))>>1) );
}

Boolean ShiptypeInfoFormHandleEvent( EventPtr eventP )
{
    Boolean handled = false;

	switch (eventP->eType) 
	{
		case frmOpenEvent:
		case frmUpdateEvent:
			DrawShiptypeInfoForm();
			handled = true;
			break;
			
		case ctlSelectEvent:
			CurForm = BuyShipForm;
			FrmGotoForm( CurForm );
			handled = true;
			break;
			
		default:
			break;
	}
	
	return handled;
}
