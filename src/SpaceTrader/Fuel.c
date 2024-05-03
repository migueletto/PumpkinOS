/***********************************************************************
 *
 * SPACE TRADER 1.2.0
 *
 * Fuel.c
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
// Fuel.c - Functions include:
// char GetFuelTanks( void )
// char GetFuel( void )
// void BuyFuel( int Amount )
//
// Modifications:
// mm/dd/yy - description - author
// *************************************************************************

#include "external.h"

// *************************************************************************
// Determine size of fueltanks
// *************************************************************************
char GetFuelTanks( void )
{
	return (HasGadget( &Ship, FUELCOMPACTOR ) ? 18 : Shiptype[Ship.Type].FuelTanks);
}


// *************************************************************************
// Determine fuel in tank
// *************************************************************************
char GetFuel( void )
{
	return min( Ship.Fuel, GetFuelTanks() );
}


// *************************************************************************
// Buy Fuel for Amount credits
// *************************************************************************
void BuyFuel( int Amount )
{
	int MaxFuel;
	int Parsecs;
	
	MaxFuel = (GetFuelTanks() - GetFuel()) * Shiptype[Ship.Type].CostOfFuel;
	if (Amount > MaxFuel)
		Amount = MaxFuel;
	if (Amount > Credits)
		Amount = Credits;
		
	Parsecs = Amount / Shiptype[Ship.Type].CostOfFuel;
	
	Ship.Fuel += Parsecs;
	Credits -= Parsecs * Shiptype[Ship.Type].CostOfFuel;
}
