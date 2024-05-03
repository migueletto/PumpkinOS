/***********************************************************************
 *
 * SPACE TRADER 1.2.0
 *
 * Spacetrader.h
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
//
// Space Trader Main Include 
//
#define appFileCreator				'STra'
#define appVersionNum              	0x01
#define appPrefID                  	0x00
#define appPrefVersionNum          	0x01

// Special Enables                  // Comment these out to disable code
#define _STRA_CHEAT_                // Cheat Enable
#define _STRA_SHIPYARDCREDITS_      // Display Trade Credits in Ship Yard
#define _INCLUDE_DEBUG_DIALOGS_		// Include code for displaying Debug Alerts

// Add Ships, Weapons, Shields, and Gadgets that don't show up normally
#define EXTRAWEAPONS    1   // Number of weapons over standard
#define EXTRAGADGETS    1   // Number of Gadgets over standard
#define EXTRASHIELDS    1   // Number of Shields over standard
#define EXTRASHIPS      5   // Number of Ships over standard

// Tradeable items
#define MAXTRADEITEM   10   // Number of items traded 
#define MAXDIGITS       8   // Maximum amount of cash 99,999,999
#define MAXPRICEDIGITS  5   // Maximum price 99,999.
#define MAXQTYDIGITS    3   // Maximum quantity 999

// Activity level of police, traders or pirates
#define MAXACTIVITY     8
#define MAXSTATUS       8

// System status: normally this is uneventful, but sometimes a system has a 
// special event occurring. This influences some prices.
#define UNEVENTFUL      0
#define WAR             1
#define PLAGUE          2
#define DROUGHT         3
#define BOREDOM         4
#define COLD            5
#define CROPFAILURE     6
#define LACKOFWORKERS   7

// Difficulty levels
#define MAXDIFFICULTY   5
#define BEGINNER        0
#define EASY            1
#define NORMAL          2
#define HARD            3
#define IMPOSSIBLE      4

// Crewmembers. The commander is always the crewmember with index 0.
// Zeethibal is always the last
#define MAXCREWMEMBER  31

// Extra Crewmembers who won't be found randomly
#define MAXSKILL       10
#define NAMELEN        20

// Skills
#define PILOTSKILL		1
#define FIGHTERSKILL	2
#define TRADERSKILL		3
#define ENGINEERSKILL	4

// Tradeitems
#define WATER           0
#define FURS            1
#define FOOD            2
#define ORE             3
#define GAMES           4
#define FIREARMS        5
#define MEDICINE        6
#define MACHINERY       7
#define NARCOTICS       8
#define ROBOTS          9

// Ship types
#define MAXSHIPTYPE             10
#define MAXRANGE                20
#define MANTISTYPE     			MAXSHIPTYPE+2
#define SCARABTYPE				MAXSHIPTYPE+3
#define BOTTLETYPE	   			MAXSHIPTYPE+4

// Weapons
#define MAXWEAPONTYPE       3
#define PULSELASERWEAPON    0
#define PULSELASERPOWER    15
#define BEAMLASERWEAPON     1
#define BEAMLASERPOWER     25
#define MILITARYLASERWEAPON 2
#define MILITARYLASERPOWER 35
#define MORGANLASERWEAPON	3
#define MORGANLASERPOWER   85 // fixme!

// Shields
#define MAXSHIELDTYPE    2
#define ENERGYSHIELD     0
#define ESHIELDPOWER     100
#define REFLECTIVESHIELD 1
#define RSHIELDPOWER     200
#define LIGHTNINGSHIELD  2
#define LSHIELDPOWER     350

// Hull Upgrade
#define UPGRADEDHULL	 50

// Gadgets
#define MAXGADGETTYPE    5
#define EXTRABAYS        0
#define AUTOREPAIRSYSTEM 1
#define NAVIGATINGSYSTEM 2
#define TARGETINGSYSTEM  3
#define CLOAKINGDEVICE   4
#define FUELCOMPACTOR    5  // MAXGADGETTYPE + 1

// Skills
#define MAXSKILLTYPE     4
#define SKILLBONUS       3
#define CLOAKBONUS       2

// Police Action 
#define POLICE 0
#define POLICEINSPECTION 0 // Police asks to submit for inspection
#define POLICEIGNORE     1 // Police just ignores you
#define POLICEATTACK     2 // Police attacks you (sometimes on sight)
#define POLICEFLEE       3 // Police is fleeing
#define MAXPOLICE        9

// Pirate Actions
#define PIRATE 10
#define PIRATEATTACK    10 // Pirate attacks
#define PIRATEFLEE      11 // Pirate flees
#define PIRATEIGNORE    12 // Pirate ignores you (because of cloak)
#define PIRATESURRENDER 13 // Pirate surrenders
#define MAXPIRATE       19

// Trader Actions
#define TRADER          20
#define TRADERIGNORE    20 // Trader passes
#define TRADERFLEE      21 // Trader flees
#define TRADERATTACK    22 // Trader is attacking (after being provoked)
#define TRADERSURRENDER 23 // Trader surrenders 
#define TRADERSELL	    24 // Trader will sell products in orbit
#define TRADERBUY		25 // Trader will buy products in orbit
#define TRADERNOTRADE	26 // Player has declined to transact with Trader
#define MAXTRADER       29

// Space Monster Actions
#define SPACEMONSTERATTACK 30
#define SPACEMONSTERIGNORE 31
#define MAXSPACEMONSTER    39

// Dragonfly Actions
#define DRAGONFLYATTACK    40
#define DRAGONFLYIGNORE    41
#define MAXDRAGONFLY       49

#define MANTIS             50

// Scarab Actions
#define SCARABATTACK    60
#define SCARABIGNORE    61
#define MAXSCARAB       69

// Famous Captain
#define FAMOUSCAPTAIN	   			70
#define FAMOUSCAPATTACK    			71
#define CAPTAINAHABENCOUNTER		72
#define CAPTAINCONRADENCOUNTER		73
#define CAPTAINHUIEENCOUNTER		74
#define MAXFAMOUSCAPTAIN            79

// Other Special Encounters
#define MARIECELESTEENCOUNTER		80
#define BOTTLEOLDENCOUNTER          81
#define BOTTLEGOODENCOUNTER         82
#define POSTMARIEPOLICEENCOUNTER	83


// The commander's ship
#define MAXWEAPON         3
#define MAXSHIELD         3
#define MAXGADGET         3
#define MAXCREW           3
#define MAXTRIBBLES  100000

// Solar systems
#define MAXSOLARSYSTEM  120
#define ACAMARSYSTEM      0
#define BARATASSYSTEM     6
#define DALEDSYSTEM      17
#define DEVIDIASYSTEM    22
#define GEMULONSYSTEM    32
#define JAPORISYSTEM     41
#define KRAVATSYSTEM	 50
#define MELINASYSTEM     59
#define NIXSYSTEM		 67
#define OGSYSTEM         70
#define REGULASSYSTEM    82
#define SOLSYSTEM        92
#define UTOPIASYSTEM    109
#define ZALKONSYSTEM    118

// Special events
#define COSTMOON          500000L
#define MAXSPECIALEVENT        37
#define ENDFIXED                7
#define MAXTEXT                 9
#define DRAGONFLYDESTROYED      0
#define FLYBARATAS              1
#define FLYMELINA               2
#define FLYREGULAS              3
#define MONSTERKILLED           4
#define MEDICINEDELIVERY        5
#define MOONBOUGHT              6
// ----- fixed locations precede
#define MOONFORSALE             7
#define SKILLINCREASE           8
#define TRIBBLE                 9
#define ERASERECORD            10
#define BUYTRIBBLE             11
#define SPACEMONSTER           12
#define DRAGONFLY              13
#define CARGOFORSALE           14
#define INSTALLLIGHTNINGSHIELD 15
#define JAPORIDISEASE          16
#define LOTTERYWINNER          17
#define ARTIFACTDELIVERY       18
#define ALIENARTIFACT          19
#define AMBASSADORJAREK        20
#define ALIENINVASION          21
#define GEMULONINVADED         22
#define GETFUELCOMPACTOR       23
#define EXPERIMENT             24
#define TRANSPORTWILD          25
#define GETREACTOR			   26
#define GETSPECIALLASER        27
#define SCARAB			   28
#define GETHULLUPGRADED		   29
// ------ fixed locations follow
#define SCARABDESTROYED	   30
#define REACTORDELIVERED	   31
#define JAREKGETSOUT           32
#define GEMULONRESCUED         33
#define EXPERIMENTSTOPPED      34
#define EXPERIMENTNOTSTOPPED   35
#define WILDGETSOUT			   36

// Max Number of Tribble Buttons
#define TRIBBLESONSCREEN       31

// Other special events (Encounters)
// First is probability in 1000 that one could happen at all:
#define CHANCEOFVERYRAREENCOUNTER	5
#define MAXVERYRAREENCOUNTER		6
#define MARIECELESTE				0
#define CAPTAINAHAB					1
#define CAPTAINCONRAD				2
#define CAPTAINHUIE					3
#define BOTTLEOLD					4
#define BOTTLEGOOD				    5
// Already done this encounter?
#define ALREADYMARIE				1
#define ALREADYAHAB					2
#define ALREADYCONRAD				4
#define ALREADYHUIE					8
#define ALREADYBOTTLEOLD		   16
#define ALREADYBOTTLEGOOD          32

// Propability in 1000 that a trader will make offer while in orbit
#define CHANCEOFTRADEINORBIT		100

// Political systems (governments)
#define MAXPOLITICS  17
#define MAXSTRENGTH  8
#define ANARCHY      0

// Tech levels. 
#define MAXTECHLEVEL 8


// Cargo Dumping Codes. These identify the operation so we can reuse
// some of the Sell Cargo code.
// SELL is obvious, Dump is when in dock, Jettison is when in space.
#define SELLCARGO		1
#define	DUMPCARGO		2
#define JETTISONCARGO	3

// System sizes (influences the number of goods available)
#define MAXSIZE 5

// Newspaper Defines
// Newspaper Mastheads and Headlines have been moved into String Resources, where
// they belong. Mastheads starting with codes will have the codes replaced as follows:
// + -> System Name
// * -> The System Name
#define MAXMASTHEADS		3	// number of newspaper names per Political situation
#define MAXSTORIES			4 	// number of canned stories per Political situation
#define NEWSINDENT1			5	// pixels to indent 1st line of news story
#define NEWSINDENT2			5	// pixels to indent 2nd line of news story

#define STORYPROBABILITY	50/MAXTECHLEVEL	// probability of a story being shown
#define MAXSPECIALNEWSEVENTS		5		// maximum number of special news events to keep for a system

// News Events that don't exactly match special events
#define WILDARRESTED		   90
#define CAUGHTLITTERING		   91
#define EXPERIMENTPERFORMED	   92
#define ARRIVALVIASINGULARITY  93
#define CAPTAINHUIEATTACKED   100
#define CAPTAINCONRADATTACKED 101
#define CAPTAINAHABATTACKED   102
#define CAPTAINHUIEDESTROYED  110
#define CAPTAINCONRADDESTROYED 111
#define CAPTAINAHABDESTROYED  112


// Police record
#define MAXPOLICERECORD     10
#define ATTACKPOLICESCORE   -3
#define KILLPOLICESCORE     -6
#define CAUGHTWITHWILDSCORE	-4
#define ATTACKTRADERSCORE   -2
#define PLUNDERTRADERSCORE  -2
#define KILLTRADERSCORE     -4
#define ATTACKPIRATESCORE    0
#define KILLPIRATESCORE      1
#define PLUNDERPIRATESCORE  -1
#define TRAFFICKING         -1
#define FLEEFROMINSPECTION  -2
#define TAKEMARIENARCOTICS	-4

// Police Record Score
#define PSYCHOPATHSCORE -70
#define VILLAINSCORE    -30
#define CRIMINALSCORE   -10
#define DUBIOUSSCORE    -5
#define CLEANSCORE       0
#define LAWFULSCORE      5
#define TRUSTEDSCORE     10
#define HELPERSCORE      25
#define HEROSCORE        75

// Reputation (depends on number of kills)
#define MAXREPUTATION 9

#define HARMLESSREP          0
#define MOSTLYHARMLESSREP   10
#define POORREP             20
#define AVERAGESCORE        40
#define ABOVEAVERAGESCORE   80
#define COMPETENTREP       150
#define DANGEROUSREP       300
#define DEADLYREP          600
#define ELITESCORE        1500


// Debt Control
#define DEBTWARNING		 75000
#define DEBTTOOLARGE	100000

// Resources. Some systems have special resources, which influences some prices.
#define MAXRESOURCES 13

#define NOSPECIALRESOURCES 0
#define MINERALRICH 1
#define MINERALPOOR 2
#define DESERT 3
#define LOTSOFWATER 4
#define RICHSOIL 5
#define POORSOIL 6
#define RICHFAUNA 7
#define LIFELESS 8
#define WEIRDMUSHROOMS 9
#define LOTSOFHERBS 10
#define ARTISTIC 11
#define WARLIKE 12

// Wormholes
#define MAXWORMHOLE 6

#define GALAXYWIDTH	150
#define GALAXYHEIGHT 110
#define SHORTRANGEWIDTH 140
#define SHORTRANGEHEIGHT 140
#define SHORTRANGEBOUNDSX 10
#define BOUNDSX 5
#define BOUNDSY 20
#define MINDISTANCE 6
#define CLOSEDISTANCE 13
#define WORMHOLEDISTANCE 3
#define EXTRAERASE 3
// (There are new functions for crunching down booleans and
//  nibbles, which gain us a little room...)
#define MAXFORFUTUREUSE 96

// this is in percentage, and will decrease by one every day.
#define FABRICRIPINITIALPROBABILITY 25

#define MAXHIGHSCORE 3
#define KILLED 0
#define RETIRED 1
#define MOON 2

// size of a UInt16
#define MAX_WORD 65535

// these have been added to avoid tracking down the math library.
#define min( a, b ) ( (a) <= (b) ? (a) : (b) )
#define max( a, b ) ( (a) >= (b) ? (a) : (b) )


#define GetRandom( a ) (SysRandom( 0 )%(a))
#define TOLOWER( a ) ((a) >= 'A' && (a) <= 'Z' ? (a) - 'A' + 'a' : (a))
#define ABS( a ) ((a) < 0 ? (-(a)) : (a))
#define SQR( a ) ((a) * (a))
#define COMMANDER Mercenary[0]
#define CURSYSTEM SolarSystem[COMMANDER.CurSystem]
#define BASEWEAPONPRICE( a ) (BasePrice( Weapontype[a].TechLevel, Weapontype[a].Price ))
#define BASESHIELDPRICE( a ) (BasePrice( Shieldtype[a].TechLevel, Shieldtype[a].Price ))
#define BASEGADGETPRICE( a ) (BasePrice( Gadgettype[a].TechLevel, Gadgettype[a].Price ))
#define BASESHIPPRICE( a ) (((Shiptype[a].Price * (100 - TraderSkill( &Ship ))) / 100) /* * (Difficulty < 3 ? 1 : (Difficulty + 3)) / (Difficulty < 3 ? 1 : 5)*/)
#define WEAPONSELLPRICE( a ) (BaseSellPrice( Ship.Weapon[a], Weapontype[Ship.Weapon[a]].Price ))
#define SHIELDSELLPRICE( a ) (BaseSellPrice( Ship.Shield[a], Shieldtype[Ship.Shield[a]].Price ))
#define GADGETSELLPRICE( a ) (BaseSellPrice( Ship.Gadget[a], Gadgettype[Ship.Gadget[a]].Price ))
#define MERCENARYHIREPRICE( a ) (a < 0 || (a >= MAXCREWMEMBER && WildStatus == 2) ? 0 : ((Mercenary[a].Pilot + Mercenary[a].Fighter + Mercenary[a].Trader + Mercenary[a].Engineer) * 3))
#define ENCOUNTERPOLICE( a ) ((a) >= POLICE && (a) <= MAXPOLICE)
#define ENCOUNTERPIRATE( a ) ((a) >= PIRATE && (a) <= MAXPIRATE)
#define ENCOUNTERTRADER( a ) ((a) >= TRADER && (a) <= MAXTRADER)
#define ENCOUNTERMONSTER( a ) ((a) >= SPACEMONSTERATTACK && (a) <= MAXSPACEMONSTER)
#define ENCOUNTERDRAGONFLY( a ) ((a) >= DRAGONFLYATTACK && (a) <= MAXDRAGONFLY)
#define ENCOUNTERSCARAB( a ) ((a) >= SCARABATTACK && (a) <= MAXSCARAB)
#define ENCOUNTERFAMOUS( a )  ((a) >= FAMOUSCAPTAIN && (a) <= MAXFAMOUSCAPTAIN)
#define STRENGTHPOLICE( a ) (PoliceRecordScore < PSYCHOPATHSCORE ? 3 * Politics[SolarSystem[a].Politics].StrengthPolice : (PoliceRecordScore < VILLAINSCORE ? 2 * Politics[SolarSystem[a].Politics].StrengthPolice : Politics[SolarSystem[a].Politics].StrengthPolice))
#define STARTCOUNTDOWN (3 + Difficulty)

#define BELOW50	(romVersion < sysMakeROMVersion( 5, 0, 0, sysROMStageRelease, 0 ))				

#define __SPACETRADER_INC__

typedef MemHandle Handle;
typedef MemPtr Ptr;

// Always include DataTypes after all maximum definitions have occured.

#include "DataTypes.h"  // Structure Definitions
#include "Prototype.h"  // Function Prototypes
