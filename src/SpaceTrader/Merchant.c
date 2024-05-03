/***********************************************************************
 *
 * SPACE TRADER 1.2.0
 *
 * Merchant.c
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

// Needed to determine the height and width of a bitmap below OS version 4.0.
// The direct access is only used below 4.0, otherwise the BmpGetDimensions function is used.
//#ifndef ALLOW_ACCESS_TO_INTERNALS_OF_BITMAPS
//#define ALLOW_ACCESS_TO_INTERNALS_OF_BITMAPS 1
//#endif

#include "external.h"
//#include <scrDriver.h>

#define ourMinVersion	sysMakeROMVersion(2,0,0,sysROMStageRelease,0)
void UnpackBooleans(Byte _val, Boolean *_a, Boolean *_b, Boolean *_c, Boolean *_d,
				  Boolean *_e, Boolean *_f, Boolean *_g, Boolean *_h);
Byte PackBooleans(Boolean _a, Boolean _b, Boolean _c, Boolean _d,
				  Boolean _e, Boolean _f, Boolean _g, Boolean _h);
Byte PackBytes(Byte _a, Byte _b);
void UnpackBytes(Byte _val, Byte *_a, Byte *_b);

#define BELOW35				(romVersion < sysMakeROMVersion( 3, 5, 0, sysROMStageRelease, 0 ))				
#define BELOW40				(romVersion < sysMakeROMVersion( 4, 0, 0, sysROMStageRelease, 0 ))				

static char* SAVEGAMENAME = "SpaceTraderSave";
static char* SWITCHGAMENAME = "SpaceTraderSwitch";

// *************************************************************************
// Get width of bitmap
// *************************************************************************
int GetBitmapWidth( BitmapPtr BmpPtr )
{
	Coord W;

	if (BELOW40)
		return BmpPtr->width;
	else
	{
		BmpGetDimensions( BmpPtr, &W, 0, 0 );
		return W;
	}
}

// *************************************************************************
// Get heigth of bitmap
// *************************************************************************
int GetBitmapHeight( BitmapPtr BmpPtr )
{
	Coord H;

	if (BELOW40)
		return BmpPtr->height;
	else
	{
		BmpGetDimensions( BmpPtr, 0, &H, 0 );
		return H;
	}
}
// *************************************************************************
// Determines if the OS supports this resolution
// *************************************************************************
static Err GraphicsSupport( void )
{
#ifdef BPP1

	return 0;

#else
	UInt32 depth, reqmode, reqmodedec;

#ifdef BPP8	
	Boolean color;
#endif

#ifdef BPP16	
	Boolean color;
#endif

	DWord romVersion;

   	// See if we're on in minimum required version of the ROM or later.
   	FtrGet( sysFtrCreator, sysFtrNumROMVersion, &romVersion );

	if (BELOW35)
	{
		FrmAlert( ErrGraphicsSupportAlert );
		return 1;
	}

#ifdef BPP16
	if (BELOW40)
	{
		FrmAlert( ErrGraphicsSupport40Alert );
		return 1;
	}
#endif

#ifdef BPP16
	reqmode = 0x8000;
	reqmodedec = 16;
#else
#ifdef BPP8
	reqmode = 0x0080;
	reqmodedec = 8;
#else // BPP4
	reqmode = 0x0008;
	reqmodedec = 4;
#endif
#endif

#ifndef BPP4		
    WinScreenMode( winScreenModeGetSupportsColor, NULL, NULL, NULL, &color );
	if (!color)
	{
		FrmAlert( ErrGraphicsSupportAlert );
		return 1;
	}
#endif

	WinScreenMode( winScreenModeGetSupportedDepths, NULL, NULL, &depth, NULL );
		
	if (((reqmode & depth) == 0) || (WinScreenMode( winScreenModeSet, NULL, NULL, &reqmodedec, NULL )))
	{
		FrmAlert( ErrGraphicsSupportAlert );
		return 1;
	}

   	return( 0 );
   	
#endif
}

/***********************************************************************
 * Resets the screen depth to default values.
 ***********************************************************************/
static void EndGraphicsSupport( void )
{
#ifndef BPP1

	WinScreenMode( winScreenModeSetToDefaults, NULL, NULL, NULL, NULL );

#endif

	return;
}

// *************************************************************************
// Determines if the ROM version is compatible
// *************************************************************************
Err RomVersionCompatible(DWord requiredVersion, Word launchFlags)
{
	DWord romVersion;

   	// See if we're on in minimum required version of the ROM or later.
   	FtrGet( sysFtrCreator, sysFtrNumROMVersion, &romVersion );
   	if (romVersion < requiredVersion)
    {
	  	if ((launchFlags & (sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp)) ==
			(sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp))
	    {
		 	FrmAlert (RomIncompatibleAlert);
		
		 	// Pilot 1.0 will continuously relaunch this app unless we switch to 
		 	// another safe one.
		 	if (romVersion < sysMakeROMVersion( 2,0,0,sysROMStageRelease,0 ))
		    	AppLaunchWithCommand(sysFileCDefaultApp, sysAppLaunchCmdNormalLaunch, NULL);
		}
		
	   	return (sysErrRomIncompatible);
	   
	}
	
   	return (0);
}


// *************************************************************************
// Determines if the software is outdated
// *************************************************************************
Err OutdatedSoftware( void )
{
#ifdef BETATEST				
   	ULong Secs;
   	DateTimeType Dt;
   	DateTimePtr DtP;
	
   	Dt.year = 2002;
   	Dt.month = 12;
   	Dt.day = 31;
   	Dt.hour = 0;
   	Dt.minute = 0;
   	Dt.second = 0;
	
   	DtP = &Dt;
   	Secs = TimDateTimeToSeconds( DtP );
   	if (Secs < TimGetSeconds())
   	{
	  	FrmAlert( OutdatedSoftwareAlert );
	  	return( 1 );
 	}
#endif	  
   	return( 0 );
}

// Fills global variables
static void FillGlobals( SAVEGAMETYPE* sg, int State )
{
  	int i;
	Boolean OldVersion = false;
	Boolean Trash;

	if (sg->VersionMinor < 4)
		OldVersion = true;
		
	Credits = sg->Credits;
	Debt = sg->Debt;
	Days = sg->Days;
	WarpSystem = sg->WarpSystem;
	SelectedShipType = sg->SelectedShipType;
	for (i=0; i<MAXTRADEITEM; ++i)
	{
		BuyPrice[i] = sg->BuyPrice[i];
		SellPrice[i] = sg->SellPrice[i];
	}
	for (i=0; i<MAXSHIPTYPE; ++i)
		ShipPrice[i] = sg->ShipPrice[i];
	GalacticChartSystem = sg->GalacticChartSystem;
	PoliceKills = sg->PoliceKills;
	TraderKills = sg->TraderKills;
	PirateKills = sg->PirateKills;
	PoliceRecordScore = sg->PoliceRecordScore;
	ReputationScore = sg->ReputationScore;
	Clicks = sg->Clicks;
	EncounterType = sg->EncounterType;
	Raided = sg->Raided;
	MonsterStatus = sg->MonsterStatus;
	DragonflyStatus = sg->DragonflyStatus;
	JaporiDiseaseStatus = sg->JaporiDiseaseStatus;
	MoonBought = sg->MoonBought;
	MonsterHull = sg->MonsterHull;
	StrCopy( NameCommander, sg->NameCommander );
	CurForm = sg->CurForm;
	MemMove( &Ship, &(sg->Ship), sizeof( Ship ) );
	MemMove( &Opponent, &(sg->Opponent), sizeof( Opponent) );
	for (i=0; i<MAXCREWMEMBER+1; ++i)
		MemMove( &Mercenary[i], &(sg->Mercenary[i]), sizeof( Mercenary[i] ) );
	for (i=0; i<MAXSOLARSYSTEM; ++i)
		MemMove( &SolarSystem[i], &(sg->SolarSystem[i]), sizeof( SolarSystem[i] ) );
	EscapePod = sg->EscapePod;
	Insurance = sg->Insurance;
	NoClaim = sg->NoClaim;
	Inspected = sg->Inspected;
	LitterWarning = sg->LitterWarning;
	Difficulty = sg->Difficulty;
	for (i=0; i<MAXWORMHOLE; ++i)
		Wormhole[i] = sg->Wormhole[i];
	for (i=0; i<MAXTRADEITEM; ++i)
		if (Ship.Cargo[i] <= 0)
			BuyingPrice[i] = 0;
		else
			BuyingPrice[i] = sg->BuyingPrice[i];
	ArtifactOnBoard = sg->ArtifactOnBoard;
	JarekStatus = sg->JarekStatus;
	InvasionStatus = sg->InvasionStatus;
	
	if (OldVersion)
	{
		ExperimentStatus = 0;
		FabricRipProbability = 0;
		VeryRareEncounter = 0;
		WildStatus = 0;
		ReactorStatus = 0;
		TrackedSystem = -1;
		JustLootedMarie = false;
		AlreadyPaidForNewspaper = false;
		GameLoaded = false;
		// if Nix already has a special, we have to get rid of the
		// spurious GETREACTOR quest.
		if (SolarSystem[NIXSYSTEM].Special == -1)
			SolarSystem[NIXSYSTEM].Special = REACTORDELIVERED;
		else
		{
			for (i=0;i<MAXSOLARSYSTEM;i++)
				if (SolarSystem[i].Special == GETREACTOR)
					SolarSystem[i].Special = -1;
		}

		// To make sure 1.1.2 is upgradeable to 1.2.0
		if (SolarSystem[GEMULONSYSTEM].Special == 27)
			SolarSystem[GEMULONSYSTEM].Special = GEMULONRESCUED;
		if (SolarSystem[DEVIDIASYSTEM].Special == 26)
			SolarSystem[DEVIDIASYSTEM].Special = JAREKGETSOUT;

		ScarabStatus = 0;
		ArrivedViaWormhole = false;
		CanSuperWarp = false;
		SharePreferences = true;
		
		if (State != 2 || !SharePreferences)
		{
			Shortcut1 = 0;
			Shortcut2 = 1;
			Shortcut3 = 2;
			Shortcut4 = 10;
			UseHWButtons = false;
			NewsAutoPay = false;
			AlwaysIgnoreTradeInOrbit = false;
			ShowTrackedRange = true;
			TrackAutoOff = true;
			RemindLoans = true;
			IdentifyStartup = false;
		}
	}
	else
	{
		UnpackBytes(sg->ExperimentAndWildStatus, (Byte *)&ExperimentStatus, (Byte *)&WildStatus);
		FabricRipProbability = sg->FabricRipProbability;
		VeryRareEncounter = sg->VeryRareEncounter;
		SharePreferences = sg->SharePreferences;
		
		if (State != 2 || !SharePreferences)
			UnpackBooleans(sg->BooleanCollection, &UseHWButtons, &NewsAutoPay,
				&ShowTrackedRange, &JustLootedMarie, &ArrivedViaWormhole, &TrackAutoOff,
				&RemindLoans, &CanSuperWarp);
		else
			UnpackBooleans(sg->BooleanCollection, &Trash, &Trash,
				&Trash, &JustLootedMarie, &ArrivedViaWormhole, &Trash,
				&Trash, &CanSuperWarp);
		
		ReactorStatus = sg-> ReactorStatus;
		TrackedSystem = sg->TrackedSystem;
		ScarabStatus = sg->ScarabStatus;
		AlreadyPaidForNewspaper = sg->AlreadyPaidForNewspaper;
		GameLoaded = sg->GameLoaded;

		if (State != 2 || !SharePreferences)
		{
			AlwaysIgnoreTradeInOrbit = sg->AlwaysIgnoreTradeInOrbit;
			Shortcut1 = sg->Shortcut1;
			Shortcut2 = sg->Shortcut2;
			Shortcut3 = sg->Shortcut3;
			Shortcut4 = sg->Shortcut4;
			IdentifyStartup = sg->IdentifyStartup;
			RectangularButtonsOn = sg->RectangularButtonsOn;
		}
	}

	if (State != 2 || !SharePreferences)
	{
		ReserveMoney = sg->ReserveMoney;
		PriceDifferences = sg->PriceDifferences;
		APLscreen = sg->APLscreen;
		TribbleMessage = sg->TribbleMessage;
		AlwaysInfo = sg->AlwaysInfo;
		LeaveEmpty = sg->LeaveEmpty;
		TextualEncounters = sg->TextualEncounters;
		Continuous = sg->Continuous;
		AttackFleeing = sg->AttackFleeing;
		AutoFuel = sg->AutoFuel;
		AutoRepair = sg->AutoRepair;
		AlwaysIgnoreTraders = sg->AlwaysIgnoreTraders;
		AlwaysIgnorePolice = sg->AlwaysIgnorePolice;
		AlwaysIgnorePirates = sg->AlwaysIgnorePirates;
	}
	
	SolarSystem[UTOPIASYSTEM].Special = MOONBOUGHT;
}

// Load game and returns true if OK
Boolean LoadGame( int State )
{
  	Word prefsSize;
  	SWord prefsVersion = noPreferenceFound;
	SAVEGAMETYPE* sg;
	Handle sgH;
	Boolean DontLoad = false;
  	VoidHand RecHandle;
  	VoidPtr RecPointer;
	LocalID DbId;
	DmOpenRef pmDB;
	
	sgH = MemHandleNew( sizeof( SAVEGAMETYPE ) );
	sg = MemHandleLock( sgH );

	if (State == 0)
	{
	  	prefsSize = sizeof( SAVEGAMETYPE );
  		prefsVersion = PrefGetAppPreferences ( appFileCreator, appPrefID, sg, &prefsSize, true );
	
		if (prefsVersion > 1)
    		prefsVersion = noPreferenceFound;

	  	if (prefsVersion == noPreferenceFound)
  		{
    		CurForm = MainForm;
    		DontLoad = true;
	  	}
    }
    else
    {
		DbId = DmFindDatabase( 0, (State == 1 ? SAVEGAMENAME : SWITCHGAMENAME ) );
		if (DbId == 0)
	  	{
	  		if (State == 1)
		  		FrmAlert( CannotLoadAlert );
			MemPtrUnlock( sg );
			MemHandleFree( sgH );
			return false;
		}
	   	pmDB = DmOpenDatabase( 0, DbId, dmModeReadWrite );
	  	RecHandle = DmGetRecord( pmDB, 0 );
	  	if (RecHandle == NULL)
	  	{
	  		if (State == 1)
		  		FrmAlert( CannotLoadAlert );
	  		DmReleaseRecord( pmDB, 0, true );
			DmCloseDatabase( pmDB );
			MemPtrUnlock( sg );
			MemHandleFree( sgH );
			return false;
	  	}
  		RecPointer = MemHandleLock( RecHandle );
		MemMove( sg, RecPointer, sizeof( SAVEGAMETYPE ) );
	  	MemHandleUnlock( RecHandle );
  		DmReleaseRecord( pmDB, 0, true );
		DmCloseDatabase( pmDB );
    }
		
	if (!DontLoad)
		FillGlobals( sg, State );
	
	MemPtrUnlock( sg );
	MemHandleFree( sgH );
	
	return true;
}


// *************************************************************************
// Get the current application's preferences.
// *************************************************************************
static Err AppStart(void)
{
   	FtrGet( sysFtrCreator, sysFtrNumROMVersion, &romVersion );

	LoadGame( 0 );

	ChanceOfVeryRareEncounter = CHANCEOFVERYRAREENCOUNTER;
	ChanceOfTradeInOrbit = CHANCEOFTRADEINORBIT;

   	return 0;
}


// *************************************************************************
// Pack two limited range bytes into a two nibbles (a single byte) for storage.
// Use this with caution -- it doesn't check that the values are really
// within the range of 0-15
// *************************************************************************
Byte PackBytes(Byte a, Byte b)
{
	return (Byte)(a<<4 | (b & (Byte)0x0F));
}

// *************************************************************************
// Unpack two limited range bytes from a single byte from storage.
// High order 4 bits become Byte a, low order 4 bits become Byte b
// *************************************************************************
void UnpackBytes(Byte val, Byte *a, Byte *b)
{
	*a = val >> 4;
	*b = val & (Byte) 15; // 0x0F
}


// *************************************************************************
// Pack eight booleans into a single byte for storage.
// *************************************************************************
Byte PackBooleans(Boolean a, Boolean b, Boolean c, Boolean d,
				  Boolean e, Boolean f, Boolean g, Boolean h)
{
	Byte retByte = 0;
	if (a) retByte |= (Byte)128; // 0x80
	if (b) retByte |= (Byte) 64; // 0x40
	if (c) retByte |= (Byte) 32; // 0x20
	if (d) retByte |= (Byte) 16; // etc.
	if (e) retByte |= (Byte)  8;
	if (f) retByte |= (Byte)  4;
	if (g) retByte |= (Byte)  2;
	if (h) retByte |= (Byte)  1;
	return retByte;
}

// *************************************************************************
// unpack eight booleans from a single byte out of storage.
// *************************************************************************
void UnpackBooleans(Byte val, Boolean *a, Boolean *b, Boolean *c, Boolean *d,
				  Boolean *e, Boolean *f, Boolean *g, Boolean *h)
{

	if ( val & (Byte)128 )
		*a = true;
	else
		*a = false;	
	if (val & (Byte) 64 )
		*b = true;
	else
		*b = false;
	if (val & (Byte) 32 )
		*c = true;
	else
		*c = false;
	if (val & (Byte) 16 )
		*d = true;
	else
		*d = false;
	if (val & (Byte)  8 )
		*e = true;
	else
		*e = false;
	if (val & (Byte)  4 )
		*f = true;
	else
		*f = false;
	if (val & (Byte)  2 )
		*g = true;
	else
		*g = false;
	if (val & (Byte)  1 )
		*h = true;
	else
		*h = false;

}

// Save the game, either in the Statesave or in a file
Boolean SaveGame( int State )
{
	int i;
	SAVEGAMETYPE* sg;
	Handle sgH;
  	Err err;
  	VoidHand RecHandle;
	LocalID DbId;
	UInt index = 0;
	VoidPtr p;
	DmOpenRef pmDB;
	Word theAttrs;
	
	sgH = MemHandleNew( sizeof( SAVEGAMETYPE ) );
	sg = MemHandleLock( sgH );

	sg->Credits = Credits;
	sg->Debt = Debt;
	sg->Days = Days;
	sg->WarpSystem = WarpSystem;
	sg->SelectedShipType = SelectedShipType;
	for (i=0; i<MAXTRADEITEM; ++i)
	{
		sg->BuyPrice[i] = BuyPrice[i];
		sg->SellPrice[i] = SellPrice[i];
	}
	for (i=0; i<MAXSHIPTYPE; ++i)
		sg->ShipPrice[i] = ShipPrice[i];
   	sg->GalacticChartSystem = GalacticChartSystem;
	sg->PoliceKills = PoliceKills;
	sg->TraderKills = TraderKills;
	sg->PirateKills = PirateKills;
	sg->PoliceRecordScore = PoliceRecordScore;
	sg->ReputationScore = ReputationScore;
	sg->AutoFuel = AutoFuel;
	sg->AutoRepair = AutoRepair;
	sg->Clicks = Clicks;
	sg->EncounterType = EncounterType;
	sg->Raided = Raided;
	sg->MonsterStatus = MonsterStatus;
	sg->DragonflyStatus = DragonflyStatus;
	sg->JaporiDiseaseStatus = JaporiDiseaseStatus;
	sg->MoonBought = MoonBought;
	sg->MonsterHull = MonsterHull;
	StrCopy( sg->NameCommander, NameCommander );
	sg->CurForm = CurForm;
	MemMove( &(sg->Ship), &Ship, sizeof( sg->Ship ) );
	MemMove( &(sg->Opponent), &Opponent, sizeof( sg->Opponent ) );
	for (i=0; i<MAXCREWMEMBER+1; ++i)
		MemMove( &(sg->Mercenary[i]), &Mercenary[i], sizeof( sg->Mercenary[i] ) );
	for (i=0; i<MAXSOLARSYSTEM; ++i)
		MemMove( &(sg->SolarSystem[i]), &SolarSystem[i], sizeof( sg->SolarSystem[i] ) );
	for (i=0; i<MAXFORFUTUREUSE; ++i)
		sg->ForFutureUse[i] = 0;
	sg->EscapePod = EscapePod;
	sg->Insurance = Insurance;
	sg->NoClaim = NoClaim;
	sg->Inspected = Inspected;
	sg->LitterWarning = LitterWarning;
	sg->AlwaysIgnoreTraders = AlwaysIgnoreTraders;
	sg->AlwaysIgnorePolice = AlwaysIgnorePolice;
	sg->AlwaysIgnorePirates = AlwaysIgnorePirates;
	sg->Difficulty = Difficulty;
	sg->VersionMajor = 1;
	// changed from 3 to 4. SjG
	sg->VersionMinor = 4;
	for (i=0; i<MAXWORMHOLE; ++i)
		sg->Wormhole[i] = Wormhole[i];
	for (i=0; i<MAXTRADEITEM; ++i)
		sg->BuyingPrice[i] = BuyingPrice[i];
	sg->ArtifactOnBoard = ArtifactOnBoard;
	sg->ReserveMoney = ReserveMoney;
	sg->PriceDifferences = PriceDifferences;
	sg->APLscreen = APLscreen;
	sg->TribbleMessage = TribbleMessage;
	sg->AlwaysInfo = AlwaysInfo;
	sg->LeaveEmpty = LeaveEmpty;
	sg->TextualEncounters = TextualEncounters;
	sg->JarekStatus = JarekStatus;
	sg->InvasionStatus = InvasionStatus;
	sg->Continuous = Continuous;
	sg->AttackFleeing = AttackFleeing;
	sg->ExperimentAndWildStatus = PackBytes(ExperimentStatus, WildStatus);
	sg->FabricRipProbability = FabricRipProbability;
	sg->VeryRareEncounter = VeryRareEncounter;
	sg->BooleanCollection = PackBooleans(UseHWButtons, NewsAutoPay,
				ShowTrackedRange, JustLootedMarie, ArrivedViaWormhole, TrackAutoOff,
				RemindLoans, CanSuperWarp);
    sg->ReactorStatus = ReactorStatus;
    sg->TrackedSystem = TrackedSystem;
    sg->ScarabStatus = ScarabStatus;
 	sg->AlwaysIgnoreTradeInOrbit = AlwaysIgnoreTradeInOrbit;
	sg->AlreadyPaidForNewspaper = AlreadyPaidForNewspaper;
	sg->GameLoaded = GameLoaded;
	sg->SharePreferences = SharePreferences;
	sg->IdentifyStartup = IdentifyStartup;
	sg->Shortcut1 = Shortcut1;
	sg->Shortcut2 = Shortcut2;
	sg->Shortcut3 = Shortcut3;
	sg->Shortcut4 = Shortcut4;
	sg->RectangularButtonsOn = RectangularButtonsOn;

	if (State == 0)
		PrefSetAppPreferences( appFileCreator, appPrefID, appPrefVersionNum, 
			sg, sizeof( SAVEGAMETYPE ), true );
	else 
	{
		if (State == 2)
		{
			if (!LoadGame( 2 ))
				NameCommander[0] = '\0';
		}
		
		DbId = DmFindDatabase( 0, (State == 1 ? SAVEGAMENAME : SWITCHGAMENAME ) );
		if (DbId == 0)
		{
		   	err = DmCreateDatabase( 0, (State == 1 ? SAVEGAMENAME : SWITCHGAMENAME ), appFileCreator, 'Data', false );
	      	if (err != errNone)
	      	{
	      		FrmAlert( (State == 1 ? CannotSaveAlert : CannotSwitchAlert ) );
	      		if (State == 2)
	      			FillGlobals( sg, State );
			MemPtrUnlock( sg );
			MemHandleFree( sgH );
	      		return false;
	      	}
			DbId = DmFindDatabase( 0, (State == 1 ? SAVEGAMENAME : SWITCHGAMENAME ) );
			if (DbId == 0)
			{
	      		FrmAlert( (State == 1 ? CannotSaveAlert : CannotSwitchAlert ) );
	      		if (State == 2)
	      			FillGlobals( sg, State ); // Restore previous setting
			MemPtrUnlock( sg );
			MemHandleFree( sgH );
			return false;
			}
			
		   	pmDB = DmOpenDatabase( 0, DbId, dmModeReadWrite );
		   	RecHandle = DmNewRecord( pmDB, &index, sizeof( SAVEGAMETYPE ) );
		    DmWrite( MemHandleLock( RecHandle ), 0, sg, sizeof( SAVEGAMETYPE ) );
		    MemHandleUnlock( RecHandle );
		}
		else
		{
		   	pmDB = DmOpenDatabase( 0, DbId, dmModeReadWrite );
		   	p = MemHandleLock( DmGetRecord( pmDB, 0 ) );
		    DmWrite( p, 0, sg, sizeof( SAVEGAMETYPE ) );
		    MemPtrUnlock( p );
		}

	    DmReleaseRecord( pmDB, 0, true );

		// Get the attributes for our database
		DmDatabaseInfo( 0, DbId, NULL, &theAttrs, NULL, NULL,
			NULL, NULL, NULL, NULL, NULL, NULL, NULL);
	
		// Set the backup flag
		theAttrs |= dmHdrAttrBackup;
	
		// Set the attributes
		DmSetDatabaseInfo( 0, DbId, NULL, &theAttrs, NULL, NULL,
			NULL, NULL, NULL, NULL, NULL, NULL, NULL);

		DmCloseDatabase( pmDB );
	}

	MemPtrUnlock( sg );
	MemHandleFree( sgH );
	
	return true;
}

// *************************************************************************
// Save the current state of the application.
// *************************************************************************
static void AppStop(void)
{
	SaveGame( 0 );
	FrmCloseAllForms();
}

// *************************************************************************
// This routine is the event loop for the application. 
// ************************************************************************* 
static void AppEventLoop(void)
{
	Word error;
	EventType event;
	UInt32 LastTicks;

	do 
	{
		if (Continuous && (AutoAttack || AutoFlee) && (CurForm == EncounterForm))
		{
			LastTicks = TimGetTicks();
	
			EvtGetEvent( &event, SysTicksPerSecond());
			
			if (event.eType == nilEvent && 
			    (3 * (TimGetTicks() - LastTicks) > 2 * SysTicksPerSecond()))
			{
				EncounterFormHandleEvent( &event );
				continue;
			}
		}
		else
			EvtGetEvent(&event, evtWaitForever);
		
		if ( (UseHWButtons && event.eType == keyDownEvent && (event.data.keyDown.chr == vchrHard1 ||
		      event.data.keyDown.chr == vchrHard2 || event.data.keyDown.chr == vchrHard3 ||
		      event.data.keyDown.chr == vchrHard4) ) || 
		     !SysHandleEvent(&event))
			if (!MenuHandleEvent(0, &event, &error))
				if (!AppHandleEvent(&event))
					FrmDispatchEvent(&event);

		if (CurForm != EncounterForm)
		{
			AutoAttack = false;
			AutoFlee = false;
		}

	} while (event.eType != appStopEvent);
}

// *************************************************************************
// This routine is the event handler for the Start screen.
// *************************************************************************
Boolean MainFormHandleEvent(EventPtr eventP)
{
    Boolean handled = false;
    FormPtr frmP = FrmGetActiveForm();

	switch (eventP->eType) 
	{
		case frmOpenEvent:
			FrmDrawForm ( frmP);
			handled = true;
			break;
			
		case frmUpdateEvent:
			FrmDrawForm ( frmP );
			handled = true;
			break;

		// Start new game
		case penDownEvent:
		case keyDownEvent:
			StartNewGame();
			handled = true;
			break;

		default:
			break;
	}
	
	return handled;
}


// *************************************************************************
// Handling of the events of several forms.
// *************************************************************************
Boolean DefaultFormHandleEvent(EventPtr eventP)
{
    Boolean handled = false;
    FormPtr frmP = FrmGetActiveForm();

	switch (eventP->eType) 
	{
		case frmOpenEvent:
			FrmDrawForm ( frmP);
			handled = true;
			break;
		case frmUpdateEvent:
			FrmDrawForm ( frmP );
			handled = true;
			break;
                default:
			break;
	}
	
	return handled;
}


// *************************************************************************
// This is the main application.
// *************************************************************************
long MerchantPilotMain( Word cmd, Ptr cmdPBP, Word launchFlags )
{
	Err error;
	int i;

	switch (cmd)
	{
		case sysAppLaunchCmdNormalLaunch:
			error = RomVersionCompatible (ourMinVersion, launchFlags);
			if (error) 
				return error;

			error = OutdatedSoftware();
			if (error) 
				return error;

			error = GraphicsSupport();
			if (error) 
				return error;

			SystemBmp = DmGetResource( bitmapRsc, SystemBitmapFamily );
			SystemBmpPtr = MemHandleLock( SystemBmp );	
			CurrentSystemBmp = DmGetResource( bitmapRsc, CurrentSystemBitmapFamily );
			CurrentSystemBmpPtr = MemHandleLock( CurrentSystemBmp );	
			ShortRangeSystemBmp = DmGetResource( bitmapRsc, ShortRangeSystemBitmapFamily );
			ShortRangeSystemBmpPtr = MemHandleLock( ShortRangeSystemBmp );	
			WormholeBmp = DmGetResource( bitmapRsc, WormholeBitmapFamily );
			WormholeBmpPtr = MemHandleLock( WormholeBmp );	
			SmallWormholeBmp = DmGetResource( bitmapRsc, SmallWormholeBitmapFamily );
			SmallWormholeBmpPtr = MemHandleLock( SmallWormholeBmp );	
			VisitedSystemBmp = DmGetResource( bitmapRsc, VisitedSystemBitmapFamily );
			VisitedSystemBmpPtr = MemHandleLock( VisitedSystemBmp );	
			CurrentVisitedSystemBmp = DmGetResource( bitmapRsc, CurrentVisitedSystemBitmapFamily );
			CurrentVisitedSystemBmpPtr = MemHandleLock( CurrentVisitedSystemBmp );	
			VisitedShortRangeSystemBmp = DmGetResource( bitmapRsc, VisitedShortRangeSystemBitmapFamily );
			VisitedShortRangeSystemBmpPtr = MemHandleLock( VisitedShortRangeSystemBmp );

            // Load bitmaps for the ships (4 Bitmap Family Groups Per Ship) FleaBitmapFamily is the Base
			for (i=0; i<MAXSHIPTYPE+EXTRASHIPS; i++)
			{
				ShipBmp[i]                   = DmGetResource( bitmapRsc, FleaBitmapFamily + i*400);
				ShipBmpPtr[i]                = MemHandleLock( ShipBmp[i] );	
				DamagedShipBmp[i]            = DmGetResource( bitmapRsc, FleaDamagedBitmapFamily + i*400 );
				DamagedShipBmpPtr[i]         = MemHandleLock( DamagedShipBmp[i] );	
				
				if (Shiptype[i].ShieldSlots <= 0)
				{
					ShieldedShipBmp[i]           = ShipBmp[i];
					ShieldedShipBmpPtr[i]        = ShipBmpPtr[i];	
					DamagedShieldedShipBmp[i]    = DamagedShipBmp[i];
					DamagedShieldedShipBmpPtr[i] = DamagedShipBmpPtr[i];	
				}
				else
				{
					ShieldedShipBmp[i]           = DmGetResource( bitmapRsc, FireflyShieldedBitmapFamily + (i-2)*400);
					ShieldedShipBmpPtr[i]        = MemHandleLock( ShieldedShipBmp[i] );	
					DamagedShieldedShipBmp[i]    = DmGetResource( bitmapRsc, FireflyShDamBitmapFamily + (i-2)*400);
					DamagedShieldedShipBmpPtr[i] = MemHandleLock( DamagedShieldedShipBmp[i] );	
				}
			}
			
			
			for (i=0; i<5; i++)
			{
				IconBmp[i] = DmGetResource( bitmapRsc, PirateBitmapFamily + i*100);
				IconBmpPtr[i] = MemHandleLock( IconBmp[i] );	
				
			}
			
			
			error = AppStart();
			if (error) 
				return error;

			if (IdentifyStartup)
				FrmCustomAlert( IdentifyStartupAlert, NameCommander, "", "" );
				
			FrmGotoForm( CurForm );
			AppEventLoop();
			AppStop();
			
			for (i=4; i>=0; i--)
			{
				MemHandleUnlock( IconBmp[i] );	
				DmReleaseResource( IconBmp[i] );
			}
			
			for (i=MAXSHIPTYPE+EXTRASHIPS-1; i>=0; i--)
			{
				if (Shiptype[i].ShieldSlots > 0)
				{
					MemHandleUnlock( DamagedShieldedShipBmp[i] );	
					DmReleaseResource( DamagedShieldedShipBmp[i] );
					MemHandleUnlock( ShieldedShipBmp[i] );	
					DmReleaseResource( ShieldedShipBmp[i] );
				}
				MemHandleUnlock( DamagedShipBmp[i] );	
				DmReleaseResource( DamagedShipBmp[i] );
				MemHandleUnlock( ShipBmp[i] );	
				DmReleaseResource( ShipBmp[i] );
			}
			
			MemHandleUnlock( VisitedShortRangeSystemBmp );	
			DmReleaseResource( VisitedShortRangeSystemBmp );
			MemHandleUnlock( CurrentVisitedSystemBmp );	
			DmReleaseResource( CurrentVisitedSystemBmp );
			MemHandleUnlock( VisitedSystemBmp );	
			DmReleaseResource( VisitedSystemBmp );
			MemHandleUnlock( SmallWormholeBmp );	
			DmReleaseResource( SmallWormholeBmp );
			MemHandleUnlock( WormholeBmp );	
			DmReleaseResource( WormholeBmp );
			MemHandleUnlock( ShortRangeSystemBmp );	
			DmReleaseResource( ShortRangeSystemBmp );
			MemHandleUnlock( CurrentSystemBmp );	
			DmReleaseResource( CurrentSystemBmp );
			MemHandleUnlock( SystemBmp );	
			DmReleaseResource( SystemBmp );
			break;

		default:
			break;
	}
	
	EndGraphicsSupport();
	return 0;
}

// *************************************************************************
// This is the main entry point for the application.
// *************************************************************************
DWord PilotMain( Word cmd, Ptr cmdPBP, Word launchFlags)
{
    return MerchantPilotMain(cmd, cmdPBP, launchFlags);
}
