/***********************************************************************
 *
 * SPACE TRADER 1.2.0
 *
 * Math.c
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
// Math.c
// *************************************************************************

#include "external.h"

// *************************************************************************
// Temporary implementation of square root
// *************************************************************************
int my_sqrt( int a )
{
	int i;
	
	i = 0;
	while (SQR( i ) < a)
		++i;
	if (i > 0)
		if ((SQR( i ) - a) > (a - SQR( i-1 )))
			--i;
	return( i );
}

// *************************************************************************
// Square of the distance between two solar systems
// *************************************************************************
Int32 SqrDistance( SOLARSYSTEM a, SOLARSYSTEM b )
{
	return (SQR( a.X - b.X ) + SQR( a.Y - b.Y ));
}


// *************************************************************************
// Distance between two solar systems
// *************************************************************************
Int32 RealDistance( SOLARSYSTEM a, SOLARSYSTEM b )
{
	return (my_sqrt( SqrDistance( a, b ) ));
}


// *************************************************************************
// Pieter's new random functions, tweaked a bit by SjG
// *************************************************************************

#define DEFSEEDX 521288629
#define DEFSEEDY 362436069

static UInt16 SeedX = DEFSEEDX & 0xffff;
static UInt16 SeedY = DEFSEEDY & 0xffff;

int GetRandom2(int maxVal)
{
	return (int)(Rand() % maxVal);	
}

UInt16 Rand()
{
   static UInt16 a = 18000;
   static UInt16 b = 30903;

   SeedX = a*(SeedX&MAX_WORD) + (SeedX>>16);
   SeedY = b*(SeedY&MAX_WORD) + (SeedY>>16);

   return ((SeedX<<16) + (SeedY&MAX_WORD));
}

void RandSeed( UInt16 seed1, UInt16 seed2 )
{
   if (seed1)
       SeedX = seed1;   /* use default seeds if parameter is 0 */
   else
       SeedX = DEFSEEDX & 0xffff;

   if (seed2)
       SeedY = seed2;
   else
       SeedY = DEFSEEDY & 0xffff;
} 
