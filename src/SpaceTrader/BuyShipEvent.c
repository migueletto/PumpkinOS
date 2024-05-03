/***********************************************************************
 *
 * SPACE TRADER 1.2.0
 *
 * BuyShipEvent.c
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

#include "external.h"

// *************************************************************************
// Create a new ship.
// *************************************************************************
static void CreateShip( int Index )
{
	int i;

	Ship.Type = Index;
	
	for (i=0; i<MAXWEAPON; ++i)
    {
	   Ship.Weapon[i] = -1;
    }
	
	for (i=0; i<MAXSHIELD; ++i)
    {
	   Ship.Shield[i] = -1;
	   Ship.ShieldStrength[i] = 0;
    }
	
	for (i=0; i<MAXGADGET; ++i)
    {
	   Ship.Gadget[i] = -1;
    }
	   
	for (i=0; i<MAXTRADEITEM; ++i)
	{
	   Ship.Cargo[i] = 0;
	   BuyingPrice[i] = 0;
	}
	   
   Ship.Fuel = GetFuelTanks();
   Ship.Hull = Shiptype[Ship.Type].HullStrength;
}


// *************************************************************************
// Buy a new ship.
// *************************************************************************
static void BuyShip( int Index )
{
	CreateShip( Index );
	Credits -= ShipPrice[Index];
	if (ScarabStatus == 3)
		ScarabStatus = 0;
}


// *************************************************************************
// Determine Ship Prices depending on tech level of current system.
// *************************************************************************
static void DetermineShipPrices( void )
{
	int i;

	for (i=0; i<MAXSHIPTYPE; ++i)
	{
		if (Shiptype[i].MinTechLevel <= CURSYSTEM.TechLevel)
		{
			ShipPrice[i] = BASESHIPPRICE( i ) - CurrentShipPrice( false );
			if (ShipPrice[i] == 0) 
				ShipPrice[i] = 1;
		}
		else
			ShipPrice[i] = 0;
	}
}

// *************************************************************************
// You get a Flea
// *************************************************************************
void CreateFlea( void )
{
	int i;

	CreateShip( 0 );

	for (i=1; i<MAXCREW; ++i)
		Ship.Crew[i] = -1;
	
	EscapePod = false;
	Insurance = false;
	NoClaim = 0;
}	

static void DrawBuyShipForm()
{
	FormPtr frmP = FrmGetActiveForm();
	int i, j;
	RectangularShortcuts( frmP, BuyShipBButton );

	DetermineShipPrices();
	for (i=0; i<MAXSHIPTYPE; ++i)
	{
		RectangularButton( frmP, BuyShipInfo0Button + i );
		RectangularButton( frmP, BuyShipBuy0Button + i );

		if (ShipPrice[i] == 0 || Ship.Type == i)
			FrmHideObject( frmP, FrmGetObjectIndex( frmP, BuyShipBuy0Button + i ) );
		else
			FrmShowObject( frmP, FrmGetObjectIndex( frmP, BuyShipBuy0Button + i ) );
		FrmShowObject( frmP, FrmGetObjectIndex( frmP, BuyShipInfo0Button + i ) );
	}
	FrmDrawForm( frmP );
	for (i=0; i<MAXSHIPTYPE; ++i)
	{
		FntSetFont( stdFont );
		DrawChars( Shiptype[i].Name, 30, 17+i*13 );

		EraseRectangle( 110, 17+i*13, 56, 9 );

		StrIToA( SBuf, ShipPrice[i] );
		StrCat( SBuf, " cr." );
		j = MAXDIGITS - StrLen( SBuf );
		if (ShipPrice[i] == 0)
			DrawChars( "not sold", 122, 17+i*13 );
		else if (Ship.Type == i)
			DrawChars( "got one", 123, 17+i*13 );
		else 
			DrawChars( SBuf, 124+j*5+(ShipPrice[i] < 0 ? 1 : 0), 17+i*13 );
	}
	DisplayTradeCredits();

	if (Ship.Tribbles > 0 && !TribbleMessage)
	{
		FrmAlert( ShipNotWorthMuchAlert );
		TribbleMessage = true;
	}		
}

// *************************************************************************
// Handling of the events of the Buy Ship form.
// *************************************************************************
Boolean BuyShipFormHandleEvent( EventPtr eventP )
{
	Boolean handled = false;
	Boolean addLightning, addCompactor, addMorganLaser;
	Boolean hasLightning, hasCompactor, hasMorganLaser;
	long extra;
	int d, i, j;

   	switch (eventP->eType) 
   	{
		case frmOpenEvent:
		case frmUpdateEvent:
			DrawBuyShipForm();
			handled = true;
			break;

		case ctlSelectEvent:
			if (eventP->data.ctlSelect.controlID >= BuyShipInfo0Button &&
				eventP->data.ctlSelect.controlID <= BuyShipInfo9Button)
			{
				SelectedShipType = eventP->data.ctlSelect.controlID - BuyShipInfo0Button;
				CurForm = ShiptypeInfoForm;
				FrmGotoForm( CurForm );
			}
			else if (eventP->data.ctlSelect.controlID >= BuyShipBuy0Button &&
				eventP->data.ctlSelect.controlID <= BuyShipBuy9Button)
			{
				j = 0;
				for (i=0; i<MAXCREW; ++i)
					if (Ship.Crew[i] >= 0)
						++j;
				if (ShipPrice[eventP->data.ctlSelect.controlID - BuyShipBuy0Button] == 0)
					FrmAlert( ShipNotAvailableAlert );
				else if ((ShipPrice[eventP->data.ctlSelect.controlID - BuyShipBuy0Button] > 0) &&
					(Debt > 0))
					FrmAlert( YoureInDebtAlert );
				else if (ShipPrice[eventP->data.ctlSelect.controlID - BuyShipBuy0Button] > ToSpend())
					FrmAlert( CantBuyShipAlert );
				else if ((JarekStatus == 1) && (Shiptype[eventP->data.ctlSelect.controlID - 
						BuyShipBuy0Button].CrewQuarters < 2))
						FrmCustomAlert( PassengerNeedsQuartersAlert, "Ambassador Jarek", NULL, NULL );
				else if ((WildStatus == 1) && (Shiptype[eventP->data.ctlSelect.controlID - 
						BuyShipBuy0Button].CrewQuarters < 2))
						FrmCustomAlert( PassengerNeedsQuartersAlert, "Jonathan Wild", NULL, NULL );
				else if (ReactorStatus > 0 && ReactorStatus < 21)
					FrmAlert ( CantSellShipWithReactorAlert );
				else
				{	

					extra = 0;
					hasLightning = false;
					hasCompactor = false;
					hasMorganLaser = false;
					addLightning = false;
					addCompactor = false;
					addMorganLaser = false;

					if (HasShield( &Ship, LIGHTNINGSHIELD ))
					{

						if (Shiptype[eventP->data.ctlSelect.controlID - BuyShipBuy0Button].ShieldSlots == 0)
						{
							// can't transfer the Lightning Shields. How often would this happen?
							FrmCustomAlert(CantTransferSlotAlert, Shiptype[eventP->data.ctlSelect.controlID - 
								BuyShipBuy0Button].Name, "Lightning Shield", "Shield");
						}
						hasLightning = true;
						extra += 30000;
					}

					if (HasGadget( &Ship, FUELCOMPACTOR ))
					{
						if (Shiptype[eventP->data.ctlSelect.controlID - BuyShipBuy0Button].GadgetSlots == 0)
						{
							// can't transfer the Fuel Compactor
							FrmCustomAlert(CantTransferSlotAlert, Shiptype[eventP->data.ctlSelect.controlID - 
								BuyShipBuy0Button].Name, "Fuel Compactor", "Gadget");
						}
						hasCompactor = true;
						extra += 20000;
					}

					if (HasWeapon( &Ship, MORGANLASERWEAPON, true ))
					{
						if (Shiptype[eventP->data.ctlSelect.controlID - BuyShipBuy0Button].WeaponSlots == 0)
						{
							// can't transfer the Laser
							FrmCustomAlert(CantTransferSlotAlert, Shiptype[eventP->data.ctlSelect.controlID - 
								BuyShipBuy0Button].Name, "Morgan's Laser", "Weapon");
						}
						extra += 33333;
						hasMorganLaser = true;
					}

					if (ShipPrice[eventP->data.ctlSelect.controlID - BuyShipBuy0Button] + extra > ToSpend())
						FrmCustomAlert( CantBuyShipWithEquipmentAlert, SBuf, NULL, NULL );

					extra = 0;
					
					if (hasLightning && Shiptype[eventP->data.ctlSelect.controlID - BuyShipBuy0Button].ShieldSlots > 0)
					{
						if (ShipPrice[eventP->data.ctlSelect.controlID - BuyShipBuy0Button] + extra <= ToSpend())
						{
							d = FrmAlert( TransferLightningShieldAlert );
							if (d == 0)
							{
								addLightning = true;
								extra += 30000;
							}
						}
						else
						{
							FrmCustomAlert ( CantTransferAlert, "Lightning Shield", NULL, NULL );
						}
					}
					
					if (hasCompactor && Shiptype[eventP->data.ctlSelect.controlID - BuyShipBuy0Button].GadgetSlots > 0)
					{
						if (ShipPrice[eventP->data.ctlSelect.controlID - BuyShipBuy0Button] + extra <= ToSpend())
						{
							d = FrmAlert( TransferFuelCompactorAlert );
							if (d == 0)
							{
								addCompactor = true;
								extra += 20000;
							}
						}
						else
						{
							FrmCustomAlert( CantTransferAlert, "Fuel Compactor", NULL, NULL);
						}
					}

					if (hasMorganLaser && Shiptype[eventP->data.ctlSelect.controlID - BuyShipBuy0Button].WeaponSlots > 0)
					{
						if (ShipPrice[eventP->data.ctlSelect.controlID - BuyShipBuy0Button] + extra <= ToSpend())
						{
							d = FrmAlert( TransferMorganLaserAlert );
							if (d == 0)
							{
								addMorganLaser = true;
								extra += 33333;
							}
						}
						else
						{
							FrmCustomAlert( CantTransferAlert, "Morgan's Laser", NULL, NULL);
						}
					}

									
					/*
					if (ShipPrice[eventP->data.ctlSelect.controlID - BuyShipBuy0Button] + extra > ToSpend())
						FrmCustomAlert( CantBuyShipWithEquipmentAlert, SBuf, NULL, NULL );
					
					*/	
					if (j > Shiptype[eventP->data.ctlSelect.controlID - 
						BuyShipBuy0Button].CrewQuarters)
						FrmAlert( TooManyCrewmembersAlert );
					else
					{
						/*
						frmP = FrmInitForm( TradeInShipForm );
						StrCopy( SBuf, Shiptype[Ship.Type].Name );
						StrCat( SBuf, " for a new " );
						StrCat( SBuf, Shiptype[eventP->data.ctlSelect.controlID - 
							BuyShipBuy0Button].Name );
						StrCat( SBuf, "?" );
						setLabelText( frmP, TradeInShipTradeInShipLabel, SBuf );
	
						d = FrmDoDialog( frmP );

						FrmDeleteForm( frmP );
						*/
						if (addCompactor || addLightning || addMorganLaser)
						{
							StrCopy(SBuf, ", and transfer your unique equipment to the new ship?");
						}
						else
						{
							StrCopy(SBuf, "?");
						}
							
						d = FrmCustomAlert( TradeShipAlert, Shiptype[Ship.Type].Name,
							Shiptype[eventP->data.ctlSelect.controlID -  BuyShipBuy0Button].Name,
							SBuf);
						

						if (d == TradeShipYes)
						{
							BuyShip( eventP->data.ctlSelect.controlID - BuyShipBuy0Button );
							Credits -= extra;
							if (addCompactor)
								Ship.Gadget[0] = FUELCOMPACTOR;
							if (addLightning)
								Ship.Shield[0] = LIGHTNINGSHIELD;
							if (addMorganLaser)
								Ship.Weapon[0] = MORGANLASERWEAPON;
							Ship.Tribbles = 0;
							CurForm = BuyShipForm;
							FrmGotoForm( CurForm );
						}
					}
				}
			}
			handled = true;
			break;

		default:
			break;
   }
	
   return handled;
}

