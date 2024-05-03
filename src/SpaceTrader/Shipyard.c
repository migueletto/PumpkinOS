/***********************************************************************
 *
 * SPACE TRADER 1.2.0
 *
 * Shipyard.c
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
// Shipyard.c - Shipyard Module.
//
// Boolean ShipYardFormHandleEvent( EventPtr eventP )
// void BuyRepairs( int Amount )
//
// Static Local Functions
// -------------------------------
// int GetAmountForRepairs( void )
// int GetAmountForFuel( void )
// void ShowShipYard( void )
//
// *************************************************************************

#include "external.h"

// *************************************************************************
// Let the commander indicate how much he wants to spend on repairs
// *************************************************************************
static int GetAmountForRepairs( void )
{
	FormPtr frm;
	int d, Amount;
	Handle AmountH;

	frm = FrmInitForm( BuyRepairsForm );
	
	AmountH = (Handle) SetField( frm, BuyRepairsForRepairsField, "", 5, true );
	
	d = FrmDoDialog( frm );

	GetField( frm, BuyRepairsForRepairsField, SBuf, AmountH );
	if (SBuf[0] == '\0')
		Amount = 0;
	else
		Amount = StrAToI( SBuf );

	FrmDeleteForm( frm );

	if (d == BuyFuelAllButton)
		return( 9999 );
	else if (d == BuyFuelNoneButton)
		return( 0 );
	
	return( Amount );
}	

// *************************************************************************
// Let the commander indicate how much he wants to spend on fuel
// *************************************************************************
static int GetAmountForFuel( void )
{
	FormPtr frm;
	int d, Amount;
	Handle AmountH;

	frm = FrmInitForm( BuyFuelForm );
	
	AmountH = (Handle) SetField( frm, BuyFuelForFuelField, "", 4, true );
	
	d = FrmDoDialog( frm );

	GetField( frm, BuyFuelForFuelField, SBuf, AmountH );
	if (SBuf[0] == '\0')
		Amount = 0;
	else
		Amount = StrAToI( SBuf );

	FrmDeleteForm( frm );

	if (d == BuyFuelAllButton)
		return( 999 );
	else if (d == BuyFuelNoneButton)
		return( 0 );
	
	return( Amount );
}	


// *************************************************************************
// Determine ship's hull strength
// *************************************************************************
Int32 GetHullStrength(void)
{
	if (ScarabStatus == 3)
		return Shiptype[Ship.Type].HullStrength + UPGRADEDHULL;
	else
		return Shiptype[Ship.Type].HullStrength;
}

// *************************************************************************
// Display the Ship Yard form.
// Modified by SRA 04/19/01 - DisplayTradeCredits if Enabled
// *************************************************************************
static void ShowShipYard( void )
{
    FormPtr frmP;
	
	frmP = FrmGetActiveForm();

	RectangularShortcuts( frmP, ShipYardBButton );

	if (GetFuel() < GetFuelTanks())
	{
		FrmShowObject( frmP, FrmGetObjectIndex( frmP, ShipYardBuyFuelButton ) );
		FrmShowObject( frmP, FrmGetObjectIndex( frmP, ShipYardFullTankButton ) );
	}
	else
	{
		FrmHideObject( frmP, FrmGetObjectIndex( frmP, ShipYardBuyFuelButton ) );
		FrmHideObject( frmP, FrmGetObjectIndex( frmP, ShipYardFullTankButton ) );
	}

	if (Ship.Hull < GetHullStrength())
	{
		FrmShowObject( frmP, FrmGetObjectIndex( frmP, ShipYardRepairButton ) );
		FrmShowObject( frmP, FrmGetObjectIndex( frmP, ShipYardFullRepairsButton ) );
	}
	else
	{
		FrmHideObject( frmP, FrmGetObjectIndex( frmP, ShipYardRepairButton ) );
		FrmHideObject( frmP, FrmGetObjectIndex( frmP, ShipYardFullRepairsButton ) );
	}

	if (CURSYSTEM.TechLevel >= Shiptype[0].MinTechLevel)
	{
		FrmHideObject( frmP, FrmGetObjectIndex( frmP, ShipYardShipInformationButton ) );
		FrmShowObject( frmP, FrmGetObjectIndex( frmP, ShipYardBuyNewShipButton ) );
	}
	else
	{
		FrmHideObject( frmP, FrmGetObjectIndex( frmP, ShipYardBuyNewShipButton ) );
		FrmShowObject( frmP, FrmGetObjectIndex( frmP, ShipYardShipInformationButton ) );
	}
	if (EscapePod || ToSpend() < 2000 || CURSYSTEM.TechLevel < Shiptype[0].MinTechLevel)
		FrmHideObject( frmP, FrmGetObjectIndex( frmP, ShipYardEscapePodButton ) );
	else
		FrmShowObject( frmP, FrmGetObjectIndex( frmP, ShipYardEscapePodButton ) );

	FrmDrawForm( frmP );
			
	FntSetFont( stdFont );

	EraseRectangle( 0, 18, 160, 25 );
	
	StrCopy( SBuf, "You have fuel to fly " );
	SBufMultiples( GetFuel(), "parsec" );
	StrCat( SBuf, "." );
	DrawChars( SBuf, 0, 18 );

	if (GetFuel() < GetFuelTanks())
	{
		StrCopy( SBuf, "A full tank costs " );
		StrIToA( SBuf2, (GetFuelTanks() - GetFuel()) * Shiptype[Ship.Type].CostOfFuel );
		StrCat( SBuf, SBuf2 );
		StrCat( SBuf, " cr." );
		DrawChars( SBuf, 0, 30 );
	}
	else
		DrawChars( "Your tank cannot hold more fuel.", 0, 30 );
	
	EraseRectangle( 0, 60, 160, 25 );

	StrCopy( SBuf, "Your hull strength is at " );
	StrIToA( SBuf2, (Ship.Hull * 100) / GetHullStrength() );
	StrCat( SBuf, SBuf2 );
	StrCat( SBuf, "%." );
	DrawChars( SBuf, 0, 60 );

	if (Ship.Hull < GetHullStrength())
	{
		StrCopy( SBuf, "Full repair will cost " );
		StrIToA( SBuf2, (GetHullStrength() - Ship.Hull) * 
			Shiptype[Ship.Type].RepairCosts );
		StrCat( SBuf, SBuf2 );
		StrCat( SBuf, " cr." );
		DrawChars( SBuf, 0, 72 );
	}
	else
		DrawChars( "No repairs are needed.", 0, 72 );

	EraseRectangle( 0, 102, 160, 12 );

	if (CURSYSTEM.TechLevel >= Shiptype[0].MinTechLevel)
		DrawChars( "There are new ships for sale.", 0, 102 );
	else
		DrawChars( "No new ships are for sale.", 0, 102 );
	
    #ifdef _STRA_SHIPYARDCREDITS_
    DisplayTradeCredits(); // SRA 04/19/01
    #endif
    
	EraseRectangle( 0, 132, 160, 12 );

	if (EscapePod)
		DrawChars( "You have an escape pod installed.", 0, 132 );
	else if (CURSYSTEM.TechLevel < Shiptype[0].MinTechLevel)
		DrawChars( "No escape pods are for sale.", 0, 132 );
	else if (ToSpend() < 2000)
		DrawChars( "You need 2000 cr. for an escape pod.", 0, 132 );
	else
		DrawChars( "You can buy an escape pod for 2000 cr.", 0, 132 );
    

}
// *************************************************************************
// Repair Ship for Amount credits
// *************************************************************************
void BuyRepairs( int Amount )
{
	int MaxRepairs;
	int Percentage;
	
	MaxRepairs = (GetHullStrength() - Ship.Hull) * 
		Shiptype[Ship.Type].RepairCosts;
	if (Amount > MaxRepairs)
		Amount = MaxRepairs;
	if (Amount > Credits)
		Amount = Credits;
		
	Percentage = Amount / Shiptype[Ship.Type].RepairCosts;
	
	Ship.Hull += Percentage;
	Credits -= Percentage * Shiptype[Ship.Type].RepairCosts;
}

// *************************************************************************
// Ship Yard Form Event Handler.
// *************************************************************************
Boolean ShipYardFormHandleEvent( EventPtr eventP )
{
    Boolean handled = false;
	int Amount;

	switch (eventP->eType) 
	{
		case frmOpenEvent:
			ShowShipYard();
			handled = true;
			break;

		case ctlSelectEvent:
			switch (eventP->data.ctlSelect.controlID)
			{
				case ShipYardBuyFuelButton:
					Amount = GetAmountForFuel();
					if (Amount > 0)						
						BuyFuel( Amount );
					ShowShipYard();
					break;
					
				case ShipYardFullTankButton:
					BuyFuel( 999 );
					ShowShipYard();
					break;
					
				case ShipYardRepairButton:
					Amount = GetAmountForRepairs();
					if (Amount > 0)						
						BuyRepairs( Amount );
					ShowShipYard();
					break;
					
				case ShipYardFullRepairsButton:
					BuyRepairs( 9999 );
					ShowShipYard();
					break;
					
				case ShipYardBuyNewShipButton:
					CurForm = BuyShipForm;
					FrmGotoForm( CurForm );
					break;
					
				case ShipYardShipInformationButton:
					CurForm = BuyShipForm;
					FrmGotoForm( CurForm );
					break;

				case ShipYardEscapePodButton:
					if (FrmAlert( BuyEscapePodAlert ) == BuyEscapePodYes)
					{
						Credits -= 2000;
						EscapePod = true;
						ShowShipYard();
					}
					break;
			}
			handled = true;
			break;

		default:
			break;
	}
	
	return handled;
}
