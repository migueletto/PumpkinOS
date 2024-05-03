/***********************************************************************
 *
 * SPACE TRADER 1.2.0
 *
 * Global.c
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

// Global Variable Storage
// Define any Global Variable Here and Reference it in external.h
// Include the external.h file in any new source file to have
// access to the global variable
//

#define PILOT_PRECOMPILED_HEADERS_OFF 1

#include <PalmOS.h>
#include <PalmCompatibility.h>
#include "spacetrader.h"
#include "MerchantRsc.h"

const POLICERECORD PoliceRecord[MAXPOLICERECORD] =
{
	{ "Psycho",   -100 },
	{ "Villain",  PSYCHOPATHSCORE },
	{ "Criminal", VILLAINSCORE },
	{ "Crook",    CRIMINALSCORE },
	{ "Dubious",  DUBIOUSSCORE },
	{ "Clean",    CLEANSCORE },
	{ "Lawful",   LAWFULSCORE },
	{ "Trusted",  TRUSTEDSCORE },
	{ "Liked",    HELPERSCORE },
	{ "Hero",     HEROSCORE }
};


const REPUTATION Reputation[MAXREPUTATION] =
{
	{ "Harmless",        HARMLESSREP },
	{ "Mostly harmless", MOSTLYHARMLESSREP },
	{ "Poor",            POORREP },
	{ "Average",         AVERAGESCORE },
	{ "Above average",   ABOVEAVERAGESCORE },
	{ "Competent",       COMPETENTREP },
	{ "Dangerous",       DANGEROUSREP },
	{ "Deadly",          DEADLYREP },
	{ "Elite",           ELITESCORE } 
};


const char* DifficultyLevel[MAXDIFFICULTY] =
{
	"Beginner",
	"Easy",
	"Normal",
	"Hard",
	"Impossible"
};


const char* SpecialResources[MAXRESOURCES] =
{
	"Nothing special",
	"Mineral rich",
	"Mineral poor",
	"Desert",
	"Sweetwater oceans",
	"Rich soil",
	"Poor soil",
	"Rich fauna",
	"Lifeless",
	"Weird mushrooms",
	"Special herbs",
	"Artistic populace",
	"Warlike populace"
};


const char* Status[MAXSTATUS] =
{
   "under no particular pressure", 	  // Uneventful
   "at war",						  // Ore and Weapons in demand
   "ravaged by a plague",			  // Medicine in demand
   "suffering from a drought",		  // Water in demand
   "suffering from extreme boredom",  // Games and Narcotics in demand
   "suffering from a cold spell",	  // Furs in demand
   "suffering from a crop failure",	  // Food in demand
   "lacking enough workers"			  // Machinery and Robots in demand
};


const char* Activity[MAXACTIVITY] =
{
   "Absent",
   "Minimal",
   "Few",
   "Some",
   "Moderate",
   "Many",
   "Abundant",
   "Swarms"
};


const TRADEITEM Tradeitem[MAXTRADEITEM] =
{
   { "Water",     0, 0, 2,   30,   +3,   4, DROUGHT,       LOTSOFWATER,    DESERT,        30,   50,   1 },
   { "Furs", 	  0, 0, 0,  250,  +10,  10, COLD,          RICHFAUNA,      LIFELESS,     230,  280,   5 },
   { "Food", 	  1, 0, 1,  100,   +5,   5, CROPFAILURE,   RICHSOIL,       POORSOIL,      90,  160,   5 },
   { "Ore", 	  2, 2, 3,  350,  +20,  10, WAR,           MINERALRICH,    MINERALPOOR,  350,  420,  10 },
   { "Games",     3, 1, 6,  250,  -10,   5, BOREDOM,       ARTISTIC,       -1,           160,  270,   5 },
   { "Firearms",  3, 1, 5, 1250,  -75, 100, WAR,           WARLIKE,        -1,           600, 1100,  25 },
   { "Medicine",  4, 1, 6,  650,  -20,  10, PLAGUE,        LOTSOFHERBS,    -1,           400,  700,  25 },
   { "Machines",  4, 3, 5,  900,  -30,   5, LACKOFWORKERS, -1,             -1,           600,  800,  25 },
   { "Narcotics", 5, 0, 5, 3500, -125, 150, BOREDOM,       WEIRDMUSHROOMS, -1,          2000, 3000,  50 },
   { "Robots",    6, 4, 7, 5000, -150, 100, LACKOFWORKERS, -1,             -1,          3500, 5000, 100 }
};


char NameCommander[NAMELEN+1] = "Jameson";

char* MercenaryName[MAXCREWMEMBER] =
{
	NameCommander,
	"Alyssa",
	"Armatur",
	"Bentos",
	"C2U2",			
	"Chi'Ti",
	"Crystal",
	"Dane",
	"Deirdre",		
	"Doc",
	"Draco",
	"Iranda",
	"Jeremiah",
	"Jujubal",
	"Krydon",
	"Luis",
	"Mercedez",
	"Milete",
	"Muri-L",		
	"Mystyc",
	"Nandi",		
	"Orestes",
	"Pancho",
	"PS37",			
	"Quarck",		
	"Sosumi",
	"Uma",			
	"Wesley",
	"Wonton",
	"Yorvick",
	"Zeethibal" // anagram for Elizabeth
};

CREWMEMBER Mercenary[MAXCREWMEMBER + 1];


const SHIPTYPE Shiptype[MAXSHIPTYPE+EXTRASHIPS] =
{
	{ "Flea",          10, 0, 0, 0, 1, MAXRANGE, 4,  1,   2000,   5,  2,  25, -1, -1,  0, 1, 0 },
	{ "Gnat",          15, 1, 0, 1, 1, 14,       5,  2,  10000,  50, 28, 100,  0,  0,  0, 1, 1 },
	{ "Firefly",       20, 1, 1, 1, 1, 17,       5,  3,  25000,  75, 20, 100,  0,  0,  0, 1, 1 },
	{ "Mosquito",      15, 2, 1, 1, 1, 13,       5,  5,  30000, 100, 20, 100,  0,  1,  0, 1, 1 },
	{ "Bumblebee",     25, 1, 2, 2, 2, 15,       5,  7,  60000, 125, 15, 100,  1,  1,  0, 1, 2 },
	{ "Beetle",        50, 0, 1, 1, 3, 14,       5, 10,  80000,  50,  3,  50, -1, -1,  0, 1, 2 },
	{ "Hornet",        20, 3, 2, 1, 2, 16, 	     6, 15, 100000, 200,  6, 150,  2,  3,  1, 2, 3 }, 
	{ "Grasshopper",   30, 2, 2, 3, 3, 15,       6, 15, 150000, 300,  2, 150,  3,  4,  2, 3, 3 },
	{ "Termite",       60, 1, 3, 2, 3, 13,       7, 20, 225000, 300,  2, 200,  4,  5,  3, 4, 4 },
	{ "Wasp",          35, 3, 2, 2, 3, 14,       7, 20, 300000, 500,  2, 200,  5,  6,  4, 5, 4 },
	// The ships below can't be bought
	{ "Space monster",  0, 3, 0, 0, 1,  1,       8,  1, 500000,   0,  0, 500,  8,  8,  8, 1, 4 },
	{ "Dragonfly",      0, 2, 3, 2, 1,  1,       8,  1, 500000,   0,  0,  10,  8,  8,  8, 1, 1 },
	{ "Mantis",         0, 3, 1, 3, 3,  1,       8,  1, 500000,   0,  0, 300,  8,  8,  8, 1, 2 },
    { "Scarab",        20, 2, 0, 0, 2,  1,       8,  1, 500000,   0,  0, 400,  8,  8,  8, 1, 3 },
    { "Bottle",         0, 0, 0, 0, 0,  1,       8,  1,    100,   0,  0,  10,  8,  8,  8, 1, 1 }
};


const WEAPON Weapontype[MAXWEAPONTYPE+EXTRAWEAPONS] =
{
	{ "Pulse laser",	PULSELASERPOWER,     2000, 5, 50 },
	{ "Beam laser",		BEAMLASERPOWER,     12500, 6, 35 },
	{ "Military laser", MILITARYLASERPOWER, 35000, 7, 15 },
	// The weapons below cannot be bought
	{ "Morgan's laser", MORGANLASERPOWER,   50000, 8, 0 }
};


const SHIELD Shieldtype[MAXSHIELDTYPE+EXTRASHIELDS] =
{
	{ "Energy shield",      ESHIELDPOWER,  5000, 5, 70 },
	{ "Reflective shield",  RSHIELDPOWER, 20000, 6, 30 },
	// The shields below can't be bought
	{ "Lightning shield",   LSHIELDPOWER, 45000, 8,  0 }
};


const GADGET Gadgettype[MAXGADGETTYPE+EXTRAGADGETS] =
{
	{ "5 extra cargo bays", 	2500, 4, 35 }, // 5 extra holds
	{ "Auto-repair system",     7500, 5, 20 }, // Increases engineer's effectivity
	{ "Navigating system", 	   15000, 6, 20 }, // Increases pilot's effectivity
	{ "Targeting system",	   25000, 6, 20 }, // Increases fighter's effectivity
	{ "Cloaking device",      100000, 7, 5  }, // If you have a good engineer, nor pirates nor police will notice you
	// The gadgets below can't be bought
	{ "Fuel compactor",        30000, 8, 0  }
};


// Note that these initializations are overruled by the StartNewGame function
SHIP Ship =
{ 
	1,                                     // Gnat
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },      // No cargo 
	{  0, -1, -1 },                        // One pulse laser
	{ -1, -1, -1 },{ 0,0,0 },              // No shields
	{ -1, -1, -1 },                        // No gadgets
	{  0, -1, -1 },                        // Commander on board
	14,                                    // Full tank
	100,                                   // Full hull strength
	0,                                     // No tribbles on board
	{ 0, 0, 0, 0 }                         // For future use
};

SHIP Opponent =
   { 
	1,                                     // Gnat
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },      // No cargo 
	{  0, -1, -1 },                        // One pulse laser
	{ -1, -1, -1 }, { 0, 0, 0 },           // No shields
	{ -1, -1, -1 },                        // No gadgets
	{  1, -1, -1 },                        // Alyssa on board
	14,                                    // Full tank
	100,                                   // Full hull strength
	0,                                     // No tribbles on board
	{ 0, 0, 0, 0 }                         // For future use
   };


SHIP SpaceMonster =
{ 
	MAXSHIPTYPE,                           // Space monster
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },      // No cargo 
	{  2,  2,  2 },                        // Three military lasers
	{ -1, -1, -1 }, { 0, 0, 0 },           // No shields
	{ -1, -1, -1 },                        // No gadgets
	{ MAXCREWMEMBER, -1, -1 },             // super stats
	1,                                     // Full tank
	500,                                   // Full hull strength
	0,                                     // No tribbles on board
	{ 0, 0, 0, 0 }                         // For future use
};

SHIP Scarab =
{ 
	MAXSHIPTYPE+3,                         // Scarab
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },      // No cargo 
	{  2,  2,  -1 },                       // Two military lasers
	{ -1, -1, -1 }, { 0, 0, 0 },           // No shields
	{ -1, -1, -1 },                        // No gadgets
	{ MAXCREWMEMBER, -1, -1 },             // super stats
	1,                                     // Full tank
	400,                                   // Full hull strength
	0,                                     // No tribbles on board
	{ 0, 0, 0, 0 }                         // For future use
};

SHIP Dragonfly =
{ 
	MAXSHIPTYPE+1,                         // Dragonfly
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },      // No cargo 
	{  2,  0, -1 },                        // One military laser and one pulse laser
	{  LIGHTNINGSHIELD,  LIGHTNINGSHIELD,  LIGHTNINGSHIELD }, // Three lightning shields
	{  LSHIELDPOWER,    LSHIELDPOWER,    LSHIELDPOWER },     
	{  AUTOREPAIRSYSTEM, TARGETINGSYSTEM, -1 }, // Gadgets
	{  MAXCREWMEMBER, -1, -1 },             // super stats
	1,                                      // Full tank
	10,                                     // Full hull strength (though this isn't much)
	0,                                      // No tribbles on board
	{ 0, 0, 0, 0 }                          // For future use
};


const char* SystemSize[MAXSIZE] =
{
	"Tiny",
	"Small",
	"Medium",
	"Large",
	"Huge"
};


const char* TechLevel[MAXTECHLEVEL] =
{
	"Pre-agricultural",
	"Agricultural",
	"Medieval",
	"Renaissance",
	"Early Industrial",
	"Industrial",
	"Post-industrial",
	"Hi-tech"
};


const POLITICS Politics[MAXPOLITICS] =
{
	{ "Anarchy",          0, 0, 7, 1, 0, 5, 7, true,  true,  FOOD },
	{ "Capitalist State", 2, 3, 2, 7, 4, 7, 1, true,  true,  ORE },
	{ "Communist State",  6, 6, 4, 4, 1, 5, 5, true,  true,  -1 },
	{ "Confederacy",      5, 4, 3, 5, 1, 6, 3, true,  true,  GAMES },
	{ "Corporate State",  2, 6, 2, 7, 4, 7, 2, true,  true,  ROBOTS },
	{ "Cybernetic State", 0, 7, 7, 5, 6, 7, 0, false, false, ORE },
	{ "Democracy",        4, 3, 2, 5, 3, 7, 2, true,  true,  GAMES },
	{ "Dictatorship",     3, 4, 5, 3, 0, 7, 2, true,  true,  -1 },
	{ "Fascist State",    7, 7, 7, 1, 4, 7, 0, false, true,  MACHINERY },
	{ "Feudal State",     1, 1, 6, 2, 0, 3, 6, true,  true,  FIREARMS },
	{ "Military State",   7, 7, 0, 6, 2, 7, 0, false, true,  ROBOTS },
	{ "Monarchy",         3, 4, 3, 4, 0, 5, 4, true,  true,  MEDICINE },
	{ "Pacifist State",   7, 2, 1, 5, 0, 3, 1, true,  false, -1 },
	{ "Socialist State",  4, 2, 5, 3, 0, 5, 6, true,  true,  -1 },
	{ "State of Satori",  0, 1, 1, 1, 0, 1, 0, false, false, -1 },
	{ "Technocracy",      1, 6, 3, 6, 4, 7, 2, true,  true,  WATER },
	{ "Theocracy",        5, 6, 1, 4, 0, 4, 0, true,  true,  NARCOTICS }
};

// Many of these names are from Star Trek: The Next Generation, or are small changes
// to names of this series. A few have different origins.
const char* SolarSystemName[MAXSOLARSYSTEM] =
{
    "Acamar",
    "Adahn",		// The alternate personality for The Nameless One in "Planescape: Torment"
    "Aldea",
    "Andevian",
    "Antedi",
    "Balosnee",
    "Baratas",
    "Brax",			// One of the heroes in Master of Magic
    "Bretel",		// This is a Dutch device for keeping your pants up.
    "Calondia",
    "Campor",
    "Capelle",		// The city I lived in while programming this game
    "Carzon",
    "Castor",		// A Greek demi-god
    "Cestus",
    "Cheron",		
    "Courteney",	// After Courteney Cox...
    "Daled",
    "Damast",
    "Davlos",
    "Deneb",
    "Deneva",
    "Devidia",
    "Draylon",
    "Drema",
    "Endor",
    "Esmee",		// One of the witches in Pratchett's Discworld
    "Exo",
    "Ferris",		// Iron
    "Festen",		// A great Scandinavian movie
    "Fourmi",		// An ant, in French
    "Frolix",		// A solar system in one of Philip K. Dick's novels
    "Gemulon",
    "Guinifer",		// One way of writing the name of king Arthur's wife
    "Hades",		// The underworld
    "Hamlet",		// From Shakespeare
    "Helena",		// Of Troy
    "Hulst",		// A Dutch plant
    "Iodine",		// An element
    "Iralius",
    "Janus",		// A seldom encountered Dutch boy's name
    "Japori",
    "Jarada",
    "Jason",		// A Greek hero
    "Kaylon",
    "Khefka",
    "Kira",			// My dog's name
    "Klaatu",		// From a classic SF movie
    "Klaestron",
    "Korma",		// An Indian sauce
    "Kravat",		// Interesting spelling of the French word for "tie"
    "Krios",
    "Laertes",		// A king in a Greek tragedy
    "Largo",
    "Lave",			// The starting system in Elite
    "Ligon",
    "Lowry",		// The name of the "hero" in Terry Gilliam's "Brazil"
    "Magrat",		// The second of the witches in Pratchett's Discworld
    "Malcoria",
    "Melina",
    "Mentar",		// The Psilon home system in Master of Orion
    "Merik",
    "Mintaka",
    "Montor",		// A city in Ultima III and Ultima VII part 2
    "Mordan",
    "Myrthe",		// The name of my daughter
    "Nelvana",
    "Nix",			// An interesting spelling of a word meaning "nothing" in Dutch
    "Nyle",			// An interesting spelling of the great river
    "Odet",
    "Og",			// The last of the witches in Pratchett's Discworld
    "Omega",		// The end of it all
    "Omphalos",		// Greek for navel
    "Orias",
    "Othello",		// From Shakespeare
    "Parade",		// This word means the same in Dutch and in English
    "Penthara",
    "Picard",		// The enigmatic captain from ST:TNG
    "Pollux",		// Brother of Castor
    "Quator",
    "Rakhar",
    "Ran",			// A film by Akira Kurosawa
    "Regulas",
    "Relva",
    "Rhymus",
    "Rochani",
    "Rubicum",		// The river Ceasar crossed to get into Rome
    "Rutia",
    "Sarpeidon",
    "Sefalla",
    "Seltrice",
    "Sigma",
    "Sol",			// That's our own solar system
    "Somari",
    "Stakoron",
    "Styris",
    "Talani",
    "Tamus",
    "Tantalos",		// A king from a Greek tragedy
    "Tanuga",
    "Tarchannen",
    "Terosa",
    "Thera",		// A seldom encountered Dutch girl's name
    "Titan",		// The largest moon of Jupiter
    "Torin",		// A hero from Master of Magic
    "Triacus",
    "Turkana",
    "Tyrus",
    "Umberlee",		// A god from AD&D, which has a prominent role in Baldur's Gate
    "Utopia",		// The ultimate goal
    "Vadera",
    "Vagra",
    "Vandor",
    "Ventax",
    "Xenon",
    "Xerxes",		// A Greek hero
    "Yew",			// A city which is in almost all of the Ultima games
    "Yojimbo",		// A film by Akira Kurosawa
    "Zalkon",
    "Zuul"			// From the first Ghostbusters movie
};


SOLARSYSTEM SolarSystem[MAXSOLARSYSTEM];


Byte Wormhole[MAXWORMHOLE]; // Systems which have a wormhole

// Quest descriptions have been moved into the Resources.
// This makes them marginally easier to edit, as well as moves them out
// of the heap during runtime. SjG
const SPECIALEVENT SpecialEvent[MAXSPECIALEVENT] =
{
	{ "Dragonfly Destroyed",	QuestDragonflyDestroyedString,		0, 0, true },
	{ "Weird Ship",				QuestWeirdShipString,				0, 0, true },
	{ "Lightning Ship",			QuestLightningShipString,			0, 0, true },
	{ "Strange Ship",			QuestStrangeShipString,				0, 0, true },
	{ "Monster Killed", 		QuestMonsterKilledString, 	   -15000, 0, true },
	{ "Medicine Delivery", 		QuestMedicineDeliveredString,		0, 0, true },
	{ "Retirement",     		QuestRetirementString,				0, 0, false },
	{ "Moon For Sale",  		QuestMoonForSaleString, 	 COSTMOON, 4, false },
	{ "Skill Increase", 		QuestSkillIncreaseString,		 3000, 3, false },
	{ "Merchant Prince",		QuestMerchantPrinceString,		 1000, 1, false },
	{ "Erase Record",   		QuestEraseRecordString,			 5000, 3, false },
	{ "Tribble Buyer",  		QuestTribbleBuyerString, 			0, 3, false },
	{ "Space Monster",  		QuestSpaceMonsterString, 			0, 1, true },
	{ "Dragonfly",      		QuestDragonflyString, 				0, 1, true },
	{ "Cargo For Sale", 		QuestCargoForSaleString, 	 	 1000, 3, false },
	{ "Lightning Shield", 		QuestLightningShieldString,	 		0, 0, false },
	{ "Japori Disease", 		QuestJaporiDiseaseString, 			0, 1, false },
	{ "Lottery Winner", 		QuestLotteryWinnerString, 		-1000, 0, true },
	{ "Artifact Delivery", 		QuestArtifactDeliveryString,	-20000, 0, true },
	{ "Alien Artifact", 		QuestAlienArtifactString, 			0, 1, false },
	{ "Ambassador Jarek", 		QuestAmbassadorJarekString,		 	0, 1, false },
	{ "Alien Invasion",			QuestAlienInvasionString,		 	0, 0, true },
	{ "Gemulon Invaded", 		QuestGemulonInvadedString, 			0, 0, true },
	{ "Fuel Compactor", 		QuestFuelCompactorString, 			0, 0, false },
	{ "Dangerous Experiment",   QuestDangerousExperimentString,		0, 0, true },
	{ "Jonathan Wild",  		QuestJonathanWildString, 			0, 1, false },
	{ "Morgan's Reactor",  		QuestMorgansReactorString,	 		0, 0, false },
	{ "Install Morgan's Laser", QuestInstallMorgansLaserString,	 	0, 0, false },
	{ "Scarab Stolen", 		QuestScarabStolenString,		 	0, 1, true},
	{ "Upgrade Hull", 			QuestUpgradeHullString, 			0, 0, false},
	{ "Scarab Destroyed", 	QuestScarabDestroyedString,	 	0, 0, true},
	{ "Reactor Delivered",  	QuestReactorDeliveredString, 		0, 0, true },
	{ "Jarek Gets Out",			QuestJarekGetsOutString, 			0, 0, true },
	{ "Gemulon Rescued", 		QuestGemulonRescuedString,	 		0, 0, true },
	{ "Disaster Averted",		QuestDisasterAvertedString,			0, 0, true },
	{ "Experiment Failed",		QuestExperimentFailedString, 		0, 0, true },
	{ "Wild Gets Out",			QuestWildGetsOutString,				0, 0, true }

};

const char *Shortcuts[11] = {
	"B","S","Y","E","Q","P","K","I","C","G","W"
};

int ShortcutTarget[11] = {
	MenuCommandBuyCargo,
	MenuCommandSellCargo,
	MenuCommandShipYard,
	MenuCommandBuyEquipment,
	MenuCommandSellEquipment,
	MenuCommandPersonnelRoster,
	MenuCommandBank,
	MenuCommandSystemInformation,
	MenuCommandCommanderStatus,
	MenuCommandGalacticChart,
	MenuCommandShortRangeChart	
};

// increased from 51 to 128 to handle the needs of the Newspaper, and various
// quest messages stored in String Resources
char SBuf[128];
char SBuf2[51];
 
//int NewsEvents[MAXSPECIALNEWSEVENTS];

 
Handle    SystemBmp;
Handle    CurrentSystemBmp;
Handle    ShortRangeSystemBmp;
Handle    WormholeBmp;
Handle    SmallWormholeBmp;
Handle    VisitedSystemBmp;
Handle    CurrentVisitedSystemBmp;
Handle    VisitedShortRangeSystemBmp;
Handle    ShipBmp[MAXSHIPTYPE+EXTRASHIPS];
Handle    DamagedShipBmp[MAXSHIPTYPE+EXTRASHIPS];
Handle    ShieldedShipBmp[MAXSHIPTYPE+EXTRASHIPS];
Handle    DamagedShieldedShipBmp[MAXSHIPTYPE+EXTRASHIPS];
Handle    IconBmp[5];
BitmapPtr SystemBmpPtr;
BitmapPtr CurrentSystemBmpPtr;
BitmapPtr ShortRangeSystemBmpPtr;
BitmapPtr WormholeBmpPtr;
BitmapPtr SmallWormholeBmpPtr;
BitmapPtr VisitedSystemBmpPtr;
BitmapPtr CurrentVisitedSystemBmpPtr;
BitmapPtr VisitedShortRangeSystemBmpPtr;
BitmapPtr ShipBmpPtr[MAXSHIPTYPE+EXTRASHIPS];
BitmapPtr DamagedShipBmpPtr[MAXSHIPTYPE+EXTRASHIPS];
BitmapPtr ShieldedShipBmpPtr[MAXSHIPTYPE+EXTRASHIPS];
BitmapPtr DamagedShieldedShipBmpPtr[MAXSHIPTYPE+EXTRASHIPS];
BitmapPtr IconBmpPtr[5];
 
UInt32 GalacticChartUpdateTicks = 0;
Handle NameH;
//DmOpenRef pmDB;
  
// The following globals are saved between sessions
// Note that these initializations are overruled by the StartNewGame function
Int32 Credits = 1000;            // Current credits owned
Int32 Debt    = 0;               // Current Debt
Int32 BuyPrice[MAXTRADEITEM];    // Price list current system
Int32 BuyingPrice[MAXTRADEITEM]; // Total price paid for trade goods
Int32 SellPrice[MAXTRADEITEM];   // Price list current system
Int32 ShipPrice[MAXSHIPTYPE];    // Price list current system (recalculate when buy ship screen is entered)
Int32 PoliceKills = 0;           // Number of police ships killed
Int32 TraderKills = 0;           // Number of trader ships killed
Int32 PirateKills = 0;           // Number of pirate ships killed
Int32 PoliceRecordScore = 0;     // 0 = Clean record
Int32 ReputationScore = 0;       // 0 = Harmless
Int32 MonsterHull = 500;         // Hull strength of monster

int Days = 0;                   // Number of days playing
int WarpSystem = 0;             // Target system for warp
int SelectedShipType = 0;       // Selected Ship type for Shiptype Info screen
int CheatCounter = 0;
int GalacticChartSystem = 0;    // Current system on Galactic chart
int EncounterType = 0;          // Type of current encounter
int CurForm = 0;                // Form to return to
int NoClaim = 0;                // Days of No-Claim
int LeaveEmpty = 0;             // Number of cargo bays to leave empty when buying goods
int NewsSpecialEventCount = 0;  // Simplifies tracking what Quests have just been initiated or completed for the News System. This is not important enough to get saved.
int TrackedSystem = -1;			// The short-range chart will display an arrow towards this system if the value is not -1

int Shortcut1 = 0;				// default shortcut 1 = Buy Cargo
int Shortcut2 = 1;				// default shortcut 2 = Sell Cargo
int Shortcut3 = 2;				// default shortcut 3 = Shipyard
int Shortcut4 = 10;				// default shortcut 4 = Short Range Warp


// the next two values are NOT saved between sessions -- they can only be changed via cheats.
int ChanceOfVeryRareEncounter	= CHANCEOFVERYRAREENCOUNTER;
int ChanceOfTradeInOrbit		= CHANCEOFTRADEINORBIT;

Byte MonsterStatus = 0;       // 0 = Space monster isn't available, 1 = Space monster is in Acamar system, 2 = Space monster is destroyed
Byte DragonflyStatus = 0;     // 0 = Dragonfly not available, 1 = Go to Baratas, 2 = Go to Melina, 3 = Go to Regulas, 4 = Go to Zalkon, 5 = Dragonfly destroyed
Byte JaporiDiseaseStatus = 0; // 0 = No disease, 1 = Go to Japori (always at least 10 medicine cannisters), 2 = Assignment finished or canceled
Byte Difficulty = NORMAL;     // Difficulty level
Byte JarekStatus = 0;         // Ambassador Jarek 0=not delivered; 1=on board; 2=delivered
Byte InvasionStatus = 0;      // Status Alien invasion of Gemulon; 0=not given yet; 1-7=days from start; 8=too late
Byte ExperimentStatus = 0;    // Experiment; 0=not given yet,1-11 days from start; 12=performed, 13=cancelled
Byte FabricRipProbability = 0; // if Experiment = 8, this is the probability of being warped to a random planet.
Byte VeryRareEncounter = 0;     // bit map for which Very Rare Encounter(s) have taken place (see traveler.c, around line 1850)
Byte WildStatus = 0;			// Jonathan Wild: 0=not delivered; 1=on board; 2=delivered
Byte ReactorStatus = 0;			// Unstable Reactor Status: 0=not encountered; 1-20=days of mission (bays of fuel left = 10 - (ReactorStatus/2); 21=delivered
Byte ScarabStatus = 0;		// Scarab: 0=not given yet, 1=not destroyed, 2=destroyed, upgrade not performed, 3=destroyed, hull upgrade performed

Boolean AutoFuel = false;            // Automatically get a full tank when arriving in a new system
Boolean AutoRepair = false;          // Automatically get a full hull repair when arriving in a new system
Boolean Clicks = 0;                  // Distance from target system, 0 = arrived
Boolean Raided = false;              // True when the commander has been raided during the trip
Boolean Inspected = false;           // True when the commander has been inspected during the trip
Boolean MoonBought = false;          // Indicates whether a moon is available at Utopia
Boolean EscapePod = false;           // Escape Pod in ship
Boolean Insurance = false;           // Insurance bought
Boolean AlwaysIgnoreTraders = false; // Automatically ignores traders when it is safe to do so
Boolean AlwaysIgnorePolice = true;   // Automatically ignores police when it is safe to do so
Boolean AlwaysIgnorePirates = false; // Automatically ignores pirates when it is safe to do so
Boolean AlwaysIgnoreTradeInOrbit = false; // Automatically ignores Trade in Orbit when it is safe to do so
Boolean ArtifactOnBoard = false;     // Alien artifact on board
Boolean ReserveMoney = false;        // Keep enough money for insurance and mercenaries
Boolean PriceDifferences = false;    // Show price differences instead of absolute prices
Boolean APLscreen = false;           // Is true is the APL screen was last shown after the SRC
Boolean TribbleMessage = false;      // Is true if the Ship Yard on the current system informed you about the tribbles
Boolean AlwaysInfo = false;          // Will always go from SRC to Info
Boolean TextualEncounters = false;   // Show encounters as text.
Boolean AutoAttack = false;			 // Auto-attack mode
Boolean AutoFlee = false;			 // Auto-flee mode
Boolean Continuous = false;			 // Continuous attack/flee mode
Boolean AttackIconStatus = false;	 // Show Attack Star or not
Boolean AttackFleeing = false;		 // Continue attack on fleeing ship
Boolean PossibleToGoThroughRip = false;	// if Dr Fehler's experiment happened, we can only go through one space-time rip per warp.
Boolean UseHWButtons = false;		// by default, don't use Hardware W buttons
Boolean NewsAutoPay = false;		// by default, ask each time someone buys a newspaper
Boolean ShowTrackedRange = true;	// display range when tracking a system on Short Range Chart
Boolean JustLootedMarie = 0;		// flag to indicate whether player looted Marie Celeste
Boolean ArrivedViaWormhole = false;	// flag to indicate whether player arrived on current planet via wormhole
Boolean AlreadyPaidForNewspaper = false; // once you buy a paper on a system, you don't have to pay again.
Boolean TrackAutoOff = true;		// Automatically stop tracking a system when you get to it?
Boolean RemindLoans = true;			// remind you every five days about outstanding loan balances
Boolean CanSuperWarp = false;		// Do you have the Portable Singularity on board?
Boolean GameLoaded = false;			// Indicates whether a game is loaded
Boolean LitterWarning = false;		// Warning against littering has been issued.
Boolean SharePreferences = true;	// Share preferences between switched games.
Boolean IdentifyStartup = false;	// Identify commander at game start
Boolean RectangularButtonsOn = false; // Indicates on OS 5.0 and higher whether rectangular buttons should be used.		

HIGHSCORE Hscores[MAXHIGHSCORE];

DWord romVersion;

