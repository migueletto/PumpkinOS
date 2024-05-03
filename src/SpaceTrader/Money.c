/***********************************************************************
 *
 * SPACE TRADER 1.2.0
 *
 * Money.c
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
// Functions Include:
//
// Int32 CurrentWorth( void )
// void PayInterest( void )
//

// *************************************************************************
// Current worth of commander
// *************************************************************************
Int32 CurrentWorth( void )
{
	return (CurrentShipPrice( false ) + Credits - Debt + (MoonBought ? COSTMOON : 0));
}


// *************************************************************************
// Pay interest on debt
// *************************************************************************
void PayInterest( void )
{
	Int32 IncDebt;

	if (Debt > 0)
	{
		IncDebt = max( 1, Debt / 10 );
		if (Credits > IncDebt)
			Credits -= IncDebt;
		else 
		{
			Debt += (IncDebt - Credits);
			Credits = 0;
		}
	}
}

