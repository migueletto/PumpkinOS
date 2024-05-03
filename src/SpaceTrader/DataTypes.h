/***********************************************************************
 *
 * SPACE TRADER 1.2.0
 *
 * Datatypes.h
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

// Data Types Defined for Space Trader 

#ifndef __SPACETRADER_INC__
  #include "spacetrader.h"    // we need our maximums defined before here
#endif

typedef struct {
	Byte Type;
	int  Cargo[MAXTRADEITEM];
	int  Weapon[MAXWEAPON];
	int  Shield[MAXSHIELD];
	long ShieldStrength[MAXSHIELD];
	int  Gadget[MAXGADGET];
	int  Crew[MAXCREW];
	Byte Fuel;
	long Hull;
	long Tribbles;
	long ForFutureUse[4];
} SHIP;


typedef struct {
	char* Name;
	long  Price;
	Byte  TechLevel;
	Byte  Chance; // Chance that this is fitted in a slot
} GADGET;


typedef struct {
	char* Name;
	long  Power;
	long  Price;
	Byte  TechLevel;
	Byte  Chance; // Chance that this is fitted in a slot
} WEAPON;


typedef struct {
	char* Name;
	long  Power;
	long  Price;
	Byte  TechLevel;
	Byte  Chance; // Chance that this is fitted in a slot
} SHIELD;


typedef struct {
	Byte NameIndex;
	Byte Pilot;
	Byte Fighter;
	Byte Trader;
	Byte Engineer;
	Byte CurSystem;
} CREWMEMBER;


typedef struct {
	char* Name;
	Byte CargoBays;		// Number of cargo bays
	Byte WeaponSlots;	// Number of lasers possible
	Byte ShieldSlots;	// Number of shields possible
	Byte GadgetSlots;	// Number of gadgets possible (e.g. docking computers)
	Byte CrewQuarters;	// Number of crewmembers possible
	Byte FuelTanks;		// Each tank contains enough fuel to travel 10 parsecs
	Byte MinTechLevel;	// Minimum tech level needed to build ship
	Byte CostOfFuel;	// Cost to fill one tank with fuel
	long Price;			// Base ship cost
	int Bounty;			// Base bounty
	int Occurrence;		// Percentage of the ships you meet
	long HullStrength;	// Hull strength
	int Police;			// Encountered as police with at least this strength
	int Pirates;		// idem Pirates
	int Traders;		// idem Traders
	Byte RepairCosts;	// Repair costs for 1 point of hull strength.
	Byte Size;			// Determines how easy it is to hit this ship
} SHIPTYPE;


typedef struct {
	Byte NameIndex;
	Byte TechLevel;			// Tech level
	Byte Politics;			// Political system
	Byte Status;			// Status
	Byte X;					// X-coordinate (galaxy width = 150)
	Byte Y;					// Y-coordinate (galaxy height = 100)
	Byte SpecialResources;	// Special resources
	Byte Size;				// System size
	int Qty[MAXTRADEITEM];	// Quantities of tradeitems. These change very slowly over time.
	Byte CountDown;			// Countdown for reset of tradeitems.
	Boolean Visited;		// Visited Yes or No
	int Special;			// Special event
} SOLARSYSTEM;


typedef struct {
	char* Name;
	Byte TechProduction; 	// Tech level needed for production
	Byte TechUsage;			// Tech level needed to use
	Byte TechTopProduction;	// Tech level which produces this item the most
	int PriceLowTech;		// Medium price at lowest tech level			
	int PriceInc;			// Price increase per tech level
	int Variance;			// Max percentage above or below calculated price
	int DoublePriceStatus;	// Price increases considerably when this event occurs
	int CheapResource;		// When this resource is available, this trade item is cheap
	int ExpensiveResource;  // When this resource is available, this trade item is expensive
	int MinTradePrice;		// Minimum price to buy/sell in orbit
	int MaxTradePrice;		// Minimum price to buy/sell in orbit
	int RoundOff;			// Roundoff price for trade in orbit
} TRADEITEM;

typedef struct {
	char* Name;
	Byte ReactionIllegal;   // Reaction level of illegal goods 0 = total acceptance (determines how police reacts if they find you carry them)
	Byte StrengthPolice;	// Strength level of police force 0 = no police (determines occurrence rate)
	Byte StrengthPirates;	// Strength level of pirates 0 = no pirates
	Byte StrengthTraders;	// Strength levcel of traders 0 = no traders
	Byte MinTechLevel;      // Mininum tech level needed
	Byte MaxTechLevel; 		// Maximum tech level where this is found
	Byte BribeLevel;		// Indicates how easily someone can be bribed 0 = unbribeable/high bribe costs
	Boolean DrugsOK;		// Drugs can be traded (if not, people aren't interested or the governemnt is too strict)
	Boolean FirearmsOK;		// Firearms can be traded (if not, people aren't interested or the governemnt is too strict)
	int Wanted;				// Tradeitem requested in particular in this type of government
} POLITICS;

typedef struct {
	char* Title;
	int QuestStringID;
	long Price;
	Byte Occurrence;
	Boolean JustAMessage;
} SPECIALEVENT;

typedef struct {
	char* Name;
	int MinScore;
} POLICERECORD;


typedef struct {
	char* Name;
	int MinScore;
} REPUTATION;


typedef struct 
{
	long Credits;
	long Debt;
	int Days;
	int WarpSystem;
	int SelectedShipType;
	long BuyPrice[MAXTRADEITEM];
	long SellPrice[MAXTRADEITEM];
	long ShipPrice[MAXSHIPTYPE];
    int GalacticChartSystem;
	long PoliceKills;
	long TraderKills;
	long PirateKills;
	long PoliceRecordScore;
	long ReputationScore;
	Boolean AutoFuel;
	Boolean AutoRepair;
	Boolean Clicks;
	int EncounterType;
	Boolean Raided;
	Byte MonsterStatus;
	Byte DragonflyStatus;
	Byte JaporiDiseaseStatus;
	Boolean MoonBought;
	long MonsterHull;
	char NameCommander[NAMELEN+1];
	int CurForm;
	SHIP Ship;
	SHIP Opponent;
	CREWMEMBER Mercenary[MAXCREWMEMBER+1];
	SOLARSYSTEM SolarSystem[MAXSOLARSYSTEM];
	Boolean EscapePod;
	Boolean Insurance; 
	int NoClaim;
	Boolean Inspected;
	Boolean AlwaysIgnoreTraders;
	Byte Wormhole[MAXWORMHOLE];
	Byte Difficulty;
	Byte VersionMajor;
	Byte VersionMinor;
	long BuyingPrice[MAXTRADEITEM];
	Boolean ArtifactOnBoard;
	Boolean ReserveMoney;
	Boolean PriceDifferences;
	Boolean APLscreen;
	int LeaveEmpty;
	Boolean TribbleMessage;
	Boolean AlwaysInfo;
	Boolean AlwaysIgnorePolice;
	Boolean AlwaysIgnorePirates;
	Boolean TextualEncounters;
	Byte JarekStatus;
	Byte InvasionStatus;
	Boolean Continuous;
	Boolean AttackFleeing;
	Byte ExperimentAndWildStatus;
	Byte FabricRipProbability;
	Byte VeryRareEncounter;
	Byte BooleanCollection;
	Byte ReactorStatus;
	int TrackedSystem;
	Byte ScarabStatus;
	Boolean AlwaysIgnoreTradeInOrbit;
	Boolean AlreadyPaidForNewspaper;
	Boolean GameLoaded;
	int Shortcut1;
	int Shortcut2;
	int Shortcut3;
	int Shortcut4;
	Boolean LitterWarning;
	Boolean SharePreferences;
	Boolean IdentifyStartup;
	Boolean RectangularButtonsOn;
	Byte ForFutureUse[MAXFORFUTUREUSE]; // Make sure this is properly adapted or savegames won't work after an upgrade!
} SAVEGAMETYPE;


typedef struct HighScoreType {
	char Name[NAMELEN+1];
	Byte Status; // 0 = killed, 1 = Retired, 2 = Bought moon
	int Days;
	long Worth;
	Byte Difficulty;
	Byte ForFutureUse;
} HIGHSCORE;

#define __DATATYPES_INC__