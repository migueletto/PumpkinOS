/***********************************************************************
 *
 * SPACE TRADER 1.2.0
 *
 * Skill.c
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
// Skill.c - Functions in this module:
//
// char TraderSkill( SHIP* _Sh );
// void RecalculateBuyPrices(Byte SystemID);
// void IncreaseRandomSkill();
// char FighterSkill( SHIP* _Sh );
// char PilotSkill( SHIP* _Sh );
// char EngineerSkill( SHIP* _Sh );
// char AdaptDifficulty( char _Level );
// void RecalculateSellPrices();
// char RandomSkill( void )
// Boolean HasGadget( SHIP* Sh, char Gg )
// Byte NthLowestSkill( SHIP* Sh, Byte n )
//
// Modifications:
// mm/dd/yy - description - author
// *************************************************************************

#include "external.h"


// *************************************************************************
// NthLowest Skill. Returns skill with the nth lowest score
// (i.e., 2 is the second worst skill). If there is a tie, it will return
// in the order of Pilot, Fighter, Trader, Engineer.
// *************************************************************************
Byte NthLowestSkill( SHIP* Sh, Byte n )
{
	Byte i = 0, lower = 1, retVal;
	Boolean looping = true;
	while (looping)
	{
		retVal = 0;
		if (Mercenary[Sh->Crew[0]].Pilot == i)
		{
			if (lower == n)
			{
				looping = false;
				retVal = PILOTSKILL;
			}
			
			lower++;
		}
		if (Mercenary[Sh->Crew[0]].Fighter == i)
		{
			if (lower == n)
			{
				looping = false;
				retVal = FIGHTERSKILL;
			}
			
			lower++;
		}
		if (Mercenary[Sh->Crew[0]].Trader == i)
		{
			if (lower == n)
			{
				looping = false;
				retVal = TRADERSKILL;
			}
			
			lower++;
		}
		if (Mercenary[Sh->Crew[0]].Engineer == i)
		{
			if (lower == n)
			{
				looping = false;
				retVal = ENGINEERSKILL;
			}
			
			lower++;
		}
		i++;
	}
	return retVal;
}

// *************************************************************************
// Trader skill
// *************************************************************************
char TraderSkill( SHIP* Sh )
{
	int i;
	char MaxSkill;
	
	MaxSkill = Mercenary[Sh->Crew[0]].Trader;
	
	for (i=1; i<MAXCREW; ++i)
	{
		if (Sh->Crew[i] < 0)
			break;
		if (Mercenary[Sh->Crew[i]].Trader > MaxSkill)
			MaxSkill = Mercenary[Sh->Crew[i]].Trader;
	}
			
	if (JarekStatus >= 2)
		++MaxSkill;			
			
	return AdaptDifficulty( MaxSkill );
}

// *************************************************************************
// After changing the trader skill, buying prices must be recalculated.
// Revised to be callable on an arbitrary Solar System
// *************************************************************************
void RecalculateBuyPrices( Byte SystemID )
{
	int i;

	for (i=0; i<MAXTRADEITEM; ++i)
	{
		if (SolarSystem[SystemID].TechLevel < Tradeitem[i].TechProduction)
			BuyPrice[i] = 0;
		else if (((i == NARCOTICS) && (!Politics[SolarSystem[SystemID].Politics].DrugsOK)) ||
			((i == FIREARMS) &&	(!Politics[SolarSystem[SystemID].Politics].FirearmsOK)))
			BuyPrice[i] = 0;
		else
		{
			if (PoliceRecordScore < DUBIOUSSCORE)
				BuyPrice[i] = (SellPrice[i] * 100) / 90;
			else 
				BuyPrice[i] = SellPrice[i];
			// BuyPrice = SellPrice + 1 to 12% (depending on trader skill (minimum is 1, max 12))
			BuyPrice[i] = (BuyPrice[i] * (103 + (MAXSKILL - TraderSkill( &Ship ))) / 100);
			if (BuyPrice[i] <= SellPrice[i])
				BuyPrice[i] = SellPrice[i] + 1;
		}
	}
}

// *************************************************************************
// Increase one of the skills of the commander
// *************************************************************************
void IncreaseRandomSkill( void )
{
	Boolean Redo;
	int d, oldtraderskill;
	
	if (COMMANDER.Pilot >= MAXSKILL && COMMANDER.Trader >= MAXSKILL &&
		COMMANDER.Fighter >= MAXSKILL && COMMANDER.Engineer >= MAXSKILL)
		return;
	
	oldtraderskill = TraderSkill( &Ship );
	
	Redo = true;
	while (Redo)
	{
		d = (GetRandom( MAXSKILLTYPE ));
		if ((d == 0 && COMMANDER.Pilot < MAXSKILL) ||
			(d == 1 && COMMANDER.Fighter < MAXSKILL) ||
			(d == 2 && COMMANDER.Trader < MAXSKILL) ||
			(d == 3 && COMMANDER.Engineer < MAXSKILL))
			Redo = false;
	}
	if (d == 0)
		COMMANDER.Pilot += 1;
	else if (d == 1)
		COMMANDER.Fighter += 1;
	else if (d == 2)
	{
		COMMANDER.Trader += 1;
		if (oldtraderskill != TraderSkill( &Ship ))
			RecalculateBuyPrices(COMMANDER.CurSystem);
	}
	else 
		COMMANDER.Engineer += 1;
}

// *************************************************************************
// Decrease one of the skills of the commander
// *************************************************************************
void DecreaseRandomSkill( int amount )
{
	Boolean Redo;
	int d, oldtraderskill;
	
	if (COMMANDER.Pilot >= MAXSKILL && COMMANDER.Trader >= MAXSKILL &&
		COMMANDER.Fighter >= MAXSKILL && COMMANDER.Engineer >= MAXSKILL)
		return;
	
	oldtraderskill = TraderSkill( &Ship );
	
	Redo = true;
	while (Redo)
	{
		d = (GetRandom( MAXSKILLTYPE ));
		if ((d == 0 && COMMANDER.Pilot > amount) ||
			(d == 1 && COMMANDER.Fighter > amount) ||
			(d == 2 && COMMANDER.Trader > amount) ||
			(d == 3 && COMMANDER.Engineer > amount))
			Redo = false;
	}
	if (d == 0)
		COMMANDER.Pilot -= amount;
	else if (d == 1)
		COMMANDER.Fighter -= amount;
	else if (d == 2)
	{
		COMMANDER.Trader -= amount;
		if (oldtraderskill != TraderSkill( &Ship ))
			RecalculateBuyPrices(COMMANDER.CurSystem);
	}
	else 
		COMMANDER.Engineer -= amount;
}



// *************************************************************************
// Randomly tweak one of the skills of the commander
// *************************************************************************
void TonicTweakRandomSkill( void )
{
	int oldPilot, oldFighter, oldTrader, oldEngineer;
	oldPilot = COMMANDER.Pilot;
	oldFighter = COMMANDER.Fighter;
	oldTrader = COMMANDER.Trader;
	oldEngineer = COMMANDER.Engineer;
	
	if (Difficulty < HARD)
	{
		// add one to a random skill, subtract one from a random skill
		while (	oldPilot == COMMANDER.Pilot &&
			oldFighter == COMMANDER.Fighter &&
			oldTrader == COMMANDER.Trader &&
			oldEngineer == COMMANDER.Engineer)
		{
			IncreaseRandomSkill();
			DecreaseRandomSkill(1);
		}
	}
	else
	{
		// add one to two random skills, subtract three from one random skill
		IncreaseRandomSkill();
		IncreaseRandomSkill();
		DecreaseRandomSkill(3);
	}
}


// *************************************************************************
// Fighter skill
// *************************************************************************
char FighterSkill( SHIP* Sh )
{
	int i;
	char MaxSkill;
	
	MaxSkill = Mercenary[Sh->Crew[0]].Fighter;
	
	for (i=1; i<MAXCREW; ++i)
	{
		if (Sh->Crew[i] < 0)
			break;
		if (Mercenary[Sh->Crew[i]].Fighter > MaxSkill)
			MaxSkill = Mercenary[Sh->Crew[i]].Fighter;
	}
			
	if (HasGadget( Sh, TARGETINGSYSTEM ))
		MaxSkill += SKILLBONUS;			
			
	return AdaptDifficulty( MaxSkill );
}

// *************************************************************************
// Pilot skill
// *************************************************************************
char PilotSkill( SHIP* Sh )
{
	int i;
	char MaxSkill;
	
	MaxSkill = Mercenary[Sh->Crew[0]].Pilot;
	
	for (i=1; i<MAXCREW; ++i)
	{
		if (Sh->Crew[i] < 0)
			break;
		if (Mercenary[Sh->Crew[i]].Pilot > MaxSkill)
			MaxSkill = Mercenary[Sh->Crew[i]].Pilot;
	}
			
	if (HasGadget( Sh, NAVIGATINGSYSTEM ))
		MaxSkill += SKILLBONUS;			
	if (HasGadget( Sh, CLOAKINGDEVICE ))
		MaxSkill += CLOAKBONUS;			
			
	return AdaptDifficulty( MaxSkill );
}

// *************************************************************************
// Engineer skill
// *************************************************************************
char EngineerSkill( SHIP* Sh )
{
	int i;
	char MaxSkill;
	
	MaxSkill = Mercenary[Sh->Crew[0]].Engineer;
	
	for (i=1; i<MAXCREW; ++i)
	{
		if (Sh->Crew[i] < 0)
			break;
		if (Mercenary[Sh->Crew[i]].Engineer > MaxSkill)
			MaxSkill = Mercenary[Sh->Crew[i]].Engineer;
	}
			
	if (HasGadget( Sh, AUTOREPAIRSYSTEM ))
		MaxSkill += SKILLBONUS;			
			
	return AdaptDifficulty( MaxSkill );
}

// *************************************************************************
// Adapt a skill to the difficulty level
// *************************************************************************
char AdaptDifficulty( char Level )
{
	if (Difficulty == BEGINNER || Difficulty == EASY)
		return (Level+1);
	else if (Difficulty == IMPOSSIBLE)
		return max( 1, Level-1 );
	else
		return Level;
}

// *************************************************************************
// After erasure of police record, selling prices must be recalculated
// *************************************************************************
void RecalculateSellPrices( void )
{
	int i;

	for (i=0; i<MAXTRADEITEM; ++i)
		SellPrice[i] = (SellPrice[i] * 100) / 90;
}

// *************************************************************************
// Random mercenary skill
// *************************************************************************
char RandomSkill( void )
{
	return 1 + GetRandom( 5 ) + GetRandom( 6 );
}

// *************************************************************************
// Determines whether a certain gadget is on board
// *************************************************************************
Boolean HasGadget( SHIP* Sh, char Gg )
{
    int i;
	
    for (i=0; i<MAXGADGET; ++i)
    {
	    if (Sh->Gadget[i] < 0)
	        continue;
        if (Sh->Gadget[i] == Gg)
	        return true;
    }
   
    return false;
}

// *************************************************************************
// Determines whether a certain shield type is on board
// *************************************************************************
Boolean HasShield( SHIP* Sh, char Gg )
{
    int i;
	
    for (i=0; i<MAXSHIELD; ++i)
    {
	    if (Sh->Shield[i] < 0)
	        continue;
        if (Sh->Shield[i] == Gg)
	        return true;
    }
   
    return false;
}

// *************************************************************************
// Determines whether a certain weapon type is on board. If exactCompare is
// false, then better weapons will also return TRUE
// *************************************************************************
Boolean HasWeapon( SHIP* Sh, char Gg, Boolean exactCompare )
{
    int i;
	
    for (i=0; i<MAXWEAPON; ++i)
    {
	    if (Sh->Weapon[i] < 0)
	        continue;
        if ((Sh->Weapon[i] == Gg) || (Sh->Weapon[i] > Gg && !exactCompare))
	        return true;
    }
   
    return false;
}
