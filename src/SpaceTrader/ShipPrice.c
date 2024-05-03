/***********************************************************************
 *
 * SPACE TRADER 1.2.0
 *
 * Shipprice.c
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
// ShipPrice.c - Functions include:
// Int32 CurrentShipPriceWithoutCargo( Boolean ForInsurance )
// Int32 CurrentShipPrice( Boolean ForInsurance )
//
// Modifications:
// mm/dd/yy - description - author
// *************************************************************************

#include "external.h"

// *************************************************************************
// Determine value of ship
// *************************************************************************
Int32 EnemyShipPrice( SHIP* Sh )
{
	int i;
	Int32 CurPrice;
	
	CurPrice = Shiptype[Sh->Type].Price;
	for (i=0; i<MAXWEAPON; ++i)
		if (Sh->Weapon[i] >= 0)
			CurPrice += Weapontype[Sh->Weapon[i]].Price;
	for (i=0; i<MAXSHIELD; ++i)
		if (Sh->Shield[i] >= 0)
			CurPrice += Shieldtype[Sh->Shield[i]].Price;
	// Gadgets aren't counted in the price, because they are already taken into account in
	// the skill adjustment of the price.
			
	CurPrice = CurPrice * (2 * PilotSkill( Sh ) + EngineerSkill( Sh ) + 3 * FighterSkill( Sh ))	/ 60;
			
	return CurPrice;
}	

// *************************************************************************
// Determine value of current ship, including equipment.
// *************************************************************************
Int32 CurrentShipPriceWithoutCargo( Boolean ForInsurance )
{
	int i;
	Int32 CurPrice;
	
	CurPrice = 
		// Trade-in value is three-fourths the original price
		((Shiptype[Ship.Type].Price * (Ship.Tribbles > 0 && !ForInsurance? 1 : 3)) / 4)
		// subtract repair costs
		- (GetHullStrength() - Ship.Hull) * Shiptype[Ship.Type].RepairCosts 
		// subtract costs to fill tank with fuel
		- (Shiptype[Ship.Type].FuelTanks - GetFuel()) * Shiptype[Ship.Type].CostOfFuel;
	// Add 2/3 of the price of each item of equipment
	for (i=0; i<MAXWEAPON; ++i)
		if (Ship.Weapon[i] >= 0)
			CurPrice += WEAPONSELLPRICE( i );
	for (i=0; i<MAXSHIELD; ++i)
		if (Ship.Shield[i] >= 0)
			CurPrice += SHIELDSELLPRICE( i );
	for (i=0; i<MAXGADGET; ++i)
		if (Ship.Gadget[i] >= 0)
			CurPrice += GADGETSELLPRICE( i );
			
	return CurPrice;
}	


// *************************************************************************
// Determine value of current ship, including goods and equipment.
// *************************************************************************
Int32 CurrentShipPrice( Boolean ForInsurance )
{
	int i;
	Int32 CurPrice;
	
	CurPrice = CurrentShipPriceWithoutCargo( ForInsurance );
	for (i=0; i<MAXTRADEITEM; ++i)
		CurPrice += BuyingPrice[i];
			
	return CurPrice;
}	
