/***********************************************************************
 *
 * SPACE TRADER 1.2.0
 *
 * Traveller.c
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
// Traveler.c - Functions in this module:
// -------------------------------------------------------------------------
// Boolean AveragePricesFormHandleEvent(EventPtr eventP)
// Boolean RetireDestroyedUtopiaFormHandleEvent( EventPtr eventP, char EndStatus )
// Boolean UtopiaFormHandleEvent( EventPtr eventP )
// Boolean RetireFormHandleEvent( EventPtr eventP )
// Boolean NewCommanderFormHandleEvent(EventPtr eventP)
// Boolean StartNewGame( void )
// void IncDays( int Amount )  
// void Travel( void )                         // Used in Trader, Encounter 
// Boolean WormholeExists( int a, int b )
// void Scoop( void )
// void Arrival( void )                        // Used in Encounter 
// Boolean Cloaked( SHIP* Sh, SHIP* Opp )
// Boolean HasGadget( SHIP* Sh, char Gg )
// int GetFirstEmptySlot( char Slots, int* Item )
// void ViewHighScores( void )
// void ClearHighScores( void )
// Int32 GetScore( char EndStatus, int Days, Int32 Worth, char Level )
// Boolean haveWeapon(int minWeapon)
// Boolean AnyEmptySlots( SHIP *ship )
// 
//
// -------------------------------------------------------------------------
// Static Local Functions:
// -------------------------------------------------------------------------
// void DeterminePrices( Byte SystemID )
// void NewCommanderDrawSkills( void )
// void ShowAveragePrices( void )
// int NextSystemWithinRange( int Current, Boolean Back )
// void ShuffleStatus( void )
// void GenerateOpponent( char Opp )
// void ChangeQuantities( void )
// void InitializeTradeitems( const int Index )
// void ShowExecuteWarp( void )
// void EndOfGame( char EndStatus )
// void SaveStatus()
// Err OpenDatabase( void )
// Err CreateEmptyHighscoreTable( void )
// Int32 GetScore( char EndStatus, int Days, Int32 Worth, char Level )
// Int32 WormholeTax( int a, int b )
// int MercenaryMoney( void )
//
// -------------------------------------------------------------------------
// Modifications:
// mm/dd/yy - description - author
// 04/18/01 - Functions in HighScore using DmWrite and MemMove changed - Sra
// 06/30/01 - DoWarp changed to check for large debt - SRA
// -------------------------------------------------------------------------
//
// *************************************************************************

#include "external.h"

static char* HIGHSCORENAME = "SpaceTraderDB";

// *************************************************************************
// Money to pay for insurance
// *************************************************************************
Int32 InsuranceMoney( void )
{
	if (!Insurance)
		return 0;
	else
		return (max( 1, (((CurrentShipPriceWithoutCargo( true ) * 5) / 2000) * 
			(100 - min( NoClaim, 90 )) / 100) ));
}

// *************************************************************************
// Standard price calculation
// *************************************************************************
static Int32 StandardPrice( char Good, char Size, char Tech, char Government, int Resources )
{
    Int32 Price;

    if (((Good == NARCOTICS) && (!Politics[(int)Government].DrugsOK)) ||
		((Good == FIREARMS) &&	(!Politics[(int)Government].FirearmsOK)))
		return 0L ;
		
	// Determine base price on techlevel of system
	Price = Tradeitem[(int)Good].PriceLowTech + (Tech * (int)Tradeitem[(int)Good].PriceInc);
			
	// If a good is highly requested, increase the price
	if (Politics[(int)Government].Wanted == Good)
		Price = (Price * 4) / 3;	
		
	// High trader activity decreases prices
	Price = (Price * (100 - (2 * Politics[(int)Government].StrengthTraders))) / 100;

	// Large system = high production decreases prices
	Price = (Price * (100 - Size)) / 100;

	// Special resources price adaptation		
	if (Resources > 0)
	{
		if (Tradeitem[(int)Good].CheapResource >= 0)
			if (Resources == Tradeitem[(int)Good].CheapResource)
				Price = (Price * 3) / 4;
		if (Tradeitem[(int)Good].ExpensiveResource >= 0)
			if (Resources == Tradeitem[(int)Good].ExpensiveResource)
				Price = (Price * 4) / 3;
	}
	
	// If a system can't use something, its selling price is zero.
	if (Tech < Tradeitem[(int)Good].TechUsage)
		return 0L;

	if (Price < 0)
		return 0L;
		
	return Price;
}

// *************************************************************************
// What you owe the mercenaries daily
// *************************************************************************
static int MercenaryMoney( void )
{
	int i, ToPay;
	
	ToPay = 0;
	for (i=1; i<MAXCREW; ++i)
		if (Ship.Crew[i] >= 0)
			ToPay += MERCENARYHIREPRICE( Ship.Crew[i] );

	return ToPay;	
}

// *************************************************************************
// Calculate wormhole tax to be paid between systems a and b
// *************************************************************************
static Int32 WormholeTax( int a, int b )
{
	if (WormholeExists( a, b ))
		return( Shiptype[Ship.Type].CostOfFuel * 25L );

	return 0L;
}

// *************************************************************************
// Calculate the score
// *************************************************************************
Int32 GetScore( char EndStatus, int Days, Int32 Worth, char Level )
{
	Int32 d;

	Worth = (Worth < 1000000 ? Worth : 1000000 + ((Worth - 1000000) / 10) );

	if (EndStatus == KILLED)
		return (Level+1L)*(Int32)((Worth * 90) / 50000);
	else if (EndStatus == RETIRED)
		return (Level+1L)*(Int32)((Worth * 95) / 50000);
	else
	{
		d = ((Level+1L)*100) - Days;
		if (d < 0)
			d = 0;
		return (Level+1L)*(Int32)((Worth + (d * 1000)) / 500);
	}
} 

// *************************************************************************
// Initializing the high score table
// *************************************************************************
static void InitHighScores (void)
{
 	int i;
 	
	for (i=0; i<MAXHIGHSCORE; ++i)
	{
		Hscores[i].Name[0] = '\0';
		Hscores[i].Status = 0;
		Hscores[i].Days = 0;
		Hscores[i].Worth = 0;
		Hscores[i].Difficulty = 0;
		Hscores[i].ForFutureUse = 0;
	}
}

// *************************************************************************
// Database handling routines for reading the high scores.
// 04/18/01 - Modified MemMove function usage to use sizeof structure * Max 
// *************************************************************************
static DmOpenRef OpenDatabase( void )
{
  	UInt index = 0;
  	VoidHand RecHandle;
  	VoidPtr RecPointer;
  	DmOpenRef pmDB;
	LocalID DbId;

	DbId = DmFindDatabase( 0, HIGHSCORENAME );
	if (DbId == 0)
	{
	   	if (DmCreateDatabase( 0, HIGHSCORENAME, appFileCreator, 'Data', false ) != errNone)
      		return 0;
		DbId = DmFindDatabase( 0, HIGHSCORENAME );
		if (DbId == 0)
			return 0;

	   	InitHighScores();
    	
	   	pmDB = DmOpenDatabase( 0, DbId, dmModeReadWrite );
   		RecHandle = DmNewRecord( pmDB, &index, sizeof(HIGHSCORE) * MAXHIGHSCORE);
    	DmWrite( MemHandleLock( RecHandle ), 0, &Hscores, sizeof( HIGHSCORE ) * MAXHIGHSCORE );
	    MemHandleUnlock( RecHandle );
   		DmReleaseRecord( pmDB, index, true );
   		
   		return pmDB;
	}
				
   	pmDB = DmOpenDatabase( 0, DbId, dmModeReadWrite );

	// Load a saved game status.
  	RecHandle = DmGetRecord( pmDB, 0 );
  	RecPointer = MemHandleLock( RecHandle );
	MemMove( &Hscores, RecPointer, sizeof( HIGHSCORE ) * MAXHIGHSCORE);
  	MemHandleUnlock( RecHandle );
  	DmReleaseRecord( pmDB, 0, true );

  	return pmDB;
}

// *************************************************************************
// Save game status information.
// 04/18/01 - Modified DmWrite function usage to use sizeof structure * Max 
// *************************************************************************
static void SaveStatus( DmOpenRef pmDB )
{
	Word theAttrs;
	LocalID DbId;
    VoidPtr p = MemHandleLock( DmGetRecord( pmDB, 0 ) );

    DmWrite( p, 0, &Hscores, sizeof( HIGHSCORE ) * MAXHIGHSCORE);
    MemPtrUnlock( p );
    DmReleaseRecord( pmDB, 0, true );

	DmOpenDatabaseInfo( pmDB, &DbId, NULL, NULL, NULL, NULL );

	// Get the attributes for our database
	DmDatabaseInfo( 0, DbId, NULL, &theAttrs, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL);
	
	// Set the backup flag
	theAttrs |= dmHdrAttrBackup;
	
	// Set the attributes
	DmSetDatabaseInfo( 0, DbId, NULL, &theAttrs, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}


// *************************************************************************
// Handling of endgame: highscore table
// *************************************************************************
static void EndOfGame( char EndStatus )
{
	int i, j;
	Boolean Scored;
	Int32 a, b;
	FormPtr frmP;
	DmOpenRef pmDB;

	pmDB = OpenDatabase();

	Scored = false;
	i = 0;
	while (i<MAXHIGHSCORE)
	{
		a = GetScore( EndStatus, Days, CurrentWorth(), Difficulty );
		
		b =	GetScore( Hscores[i].Status, Hscores[i].Days, Hscores[i].Worth,
			Hscores[i].Difficulty );
		
		if ((a > b) || (a == b && CurrentWorth() > Hscores[i].Worth) ||
			(a == b && CurrentWorth() == Hscores[i].Worth && Days > Hscores[i].Days) ||
			Hscores[i].Name[0] == '\0')
		{
			for (j=MAXHIGHSCORE-1; j>i; --j)
			{
				StrCopy( Hscores[j].Name, Hscores[j-1].Name );
				Hscores[j].Status = Hscores[j-1].Status;
				Hscores[j].Days = Hscores[j-1].Days;
				Hscores[j].Worth = Hscores[j-1].Worth;
				Hscores[j].Difficulty = Hscores[j-1].Difficulty;
			}
			
			StrCopy( Hscores[i].Name, NameCommander );
			Hscores[i].Status = EndStatus;
			Hscores[i].Days = Days;
			Hscores[i].Worth = CurrentWorth();
			Hscores[i].Difficulty = Difficulty;
			
			Scored = true;
			
			if (!GameLoaded)
				SaveStatus( pmDB );
			break;
		}

		++i;
	}

	DmCloseDatabase( pmDB );

	frmP = FrmInitForm( FinalScoreForm );

	StrCopy( SBuf, "You achieved a score of " );
	StrIToA( SBuf2, (a / 50L) );
	StrCat( SBuf, SBuf2 );
	StrCat( SBuf, "." );
	StrIToA( SBuf2, ((a%50L) / 5) );
	StrCat( SBuf, SBuf2 );
	StrCat( SBuf, "%." );
	setLabelText( frmP, FinalScoreScoreLabel, SBuf );

	if (Scored && GameLoaded)
	{
		setLabelText( frmP, FinalScoreCongratulationsLabel, "Without loading a savegame, you" );
		setLabelText( frmP, FinalScoreHighScoreLabel, "would have made the high-score list." );
	}
	else if (Scored)
	{
		setLabelText( frmP, FinalScoreCongratulationsLabel, "Congratulations!" );
		setLabelText( frmP, FinalScoreHighScoreLabel, "You have made the high-score list!" );
	}
	else
	{
		setLabelText( frmP, FinalScoreCongratulationsLabel, "Alas! This is not enough to enter" );
		setLabelText( frmP, FinalScoreHighScoreLabel, "the high-score list." );
	}
	
	FrmDoDialog( frmP );

	FrmDeleteForm( frmP );

	CurForm = MainForm;
	if (Scored && !GameLoaded)
		ViewHighScores();
	FrmGotoForm( CurForm );
}

// *************************************************************************
// Initialize qunatities of trade items of system Index
// *************************************************************************
static void InitializeTradeitems( const int Index )
{
	int i;

	for (i=0; i<MAXTRADEITEM; ++i)
	{
		if (((i == NARCOTICS) &&
			(!Politics[SolarSystem[Index].Politics].DrugsOK)) ||
			((i == FIREARMS) &&
			(!Politics[SolarSystem[Index].Politics].FirearmsOK)) ||
			(SolarSystem[Index].TechLevel < Tradeitem[i].TechProduction))
		{
			SolarSystem[Index].Qty[i] = 0;
			continue;
		}
		
		SolarSystem[Index].Qty[i] = 
			((9 + GetRandom( 5 )) - 
			ABS( Tradeitem[i].TechTopProduction - SolarSystem[Index].TechLevel )) * 
			(1 + SolarSystem[i].Size);

		// Because of the enormous profits possible, there shouldn't be too many robots or narcotics available
		if (i == ROBOTS || i == NARCOTICS) 
			SolarSystem[Index].Qty[i] = ((SolarSystem[Index].Qty[i] * (5 - Difficulty)) / (6 - Difficulty)) + 1;

		if (Tradeitem[i].CheapResource >= 0)
			if (SolarSystem[Index]. SpecialResources ==
				Tradeitem[i].CheapResource)
				SolarSystem[Index].Qty[i] = (SolarSystem[Index].Qty[i] * 4) / 3;

		if (Tradeitem[i].ExpensiveResource >= 0)
			if (SolarSystem[Index].SpecialResources ==
				Tradeitem[i].ExpensiveResource)
				SolarSystem[Index].Qty[i] = (SolarSystem[Index].Qty[i] * 3) >> 2;

		if (Tradeitem[i].DoublePriceStatus >= 0)
			if (SolarSystem[Index].Status == Tradeitem[i].DoublePriceStatus)
				SolarSystem[Index].Qty[i] = SolarSystem[Index].Qty[i] / 5;

		SolarSystem[Index].Qty[i] = SolarSystem[Index].Qty[i] - GetRandom( 10 ) +
			GetRandom( 10 );

		if (SolarSystem[Index].Qty[i] < 0)
			SolarSystem[Index].Qty[i] = 0;
	}
}

// *************************************************************************
// Determine prices in specified system (changed from Current System) SjG
// *************************************************************************
static void DeterminePrices( Byte SystemID )
{
	int i;
	
	for (i=0; i<MAXTRADEITEM; ++i)
	{
		BuyPrice[i] = StandardPrice( i, SolarSystem[SystemID].Size, SolarSystem[SystemID].TechLevel,
			SolarSystem[SystemID].Politics, SolarSystem[SystemID].SpecialResources );

		if (BuyPrice[i] <= 0)
		{
			BuyPrice[i] = 0;
			SellPrice[i] = 0;
			continue;
		}
			
		// In case of a special status, adapt price accordingly
		if (Tradeitem[i].DoublePriceStatus >= 0)
			if (SolarSystem[SystemID].Status == Tradeitem[i].DoublePriceStatus)
				BuyPrice[i] = (BuyPrice[i] * 3) >> 1;

		// Randomize price a bit
		BuyPrice[i] = BuyPrice[i] + GetRandom( Tradeitem[i].Variance ) -
			GetRandom( Tradeitem[i].Variance );

		// Should never happen
		if (BuyPrice[i] <= 0)
		{
			BuyPrice[i] = 0;
			SellPrice[i] = 0;
			continue;
		}
			
		SellPrice[i] = BuyPrice[i];
		if (PoliceRecordScore < DUBIOUSSCORE)
		{
			// Criminals have to pay off an intermediary
			SellPrice[i] = (SellPrice[i] * 90) / 100;
		}
	}

	RecalculateBuyPrices(SystemID);
}


// *************************************************************************
// Execute a warp command
// *************************************************************************
void DoWarp( Boolean viaSingularity )
{
	int i, Distance;

	// if Wild is aboard, make sure ship is armed!
	if (WildStatus == 1)
	{	
		if (! HasWeapon(&Ship, BEAMLASERWEAPON, false))
		{
			if (FrmCustomAlert( WildWontGoAlert, SolarSystemName[CURSYSTEM.NameIndex], NULL, NULL) != WildWontGoSayGoodbyetoWild)
			{
				return;
			}
			else
			{
				FrmCustomAlert( WildLeavesShipAlert, SolarSystemName[CURSYSTEM.NameIndex], NULL, NULL );
				WildStatus = 0;
			}
		}
	}

    // Check for Large Debt
	if (Debt > DEBTTOOLARGE)
	{
	    FrmAlert( DebtTooLargeForTravelAlert );
	    return;
	}
	
	// Check for enough money to pay Mercenaries    
	if (MercenaryMoney() > Credits)
	{
		FrmAlert( MustPayMercenariesAlert );
		return;
	}

    // Check for enough money to pay Insurance
	if (Insurance)
	{
		if (InsuranceMoney() + MercenaryMoney() > Credits)
		{
			FrmAlert( CantPayInsuranceAlert );
			return;
		}
	}
		
	// Check for enough money to pay Wormhole Tax 					
	if (InsuranceMoney() + MercenaryMoney() + 
		WormholeTax( COMMANDER.CurSystem, WarpSystem ) > Credits)
	{
		FrmAlert( CantPayWormholeAlert );
		return;
	}

	if (! viaSingularity)
	{
		Credits -= WormholeTax( COMMANDER.CurSystem, WarpSystem );
		Credits -= MercenaryMoney();						
		Credits -= InsuranceMoney();
	}
						
	for (i=0; i<MAXSHIELD; ++i)
	{
		if (Ship.Shield[i] < 0)
			break;
		Ship.ShieldStrength[i] = Shieldtype[Ship.Shield[i]].Power;
	}

	CURSYSTEM.CountDown = STARTCOUNTDOWN;
	if (WormholeExists( COMMANDER.CurSystem, WarpSystem ) || viaSingularity)
	{
		Distance = 0;
		ArrivedViaWormhole = true;
	}
	else
	{
		Distance = RealDistance( CURSYSTEM, SolarSystem[WarpSystem] );
		Ship.Fuel -= min( Distance, GetFuel() );
		ArrivedViaWormhole = false;
	}

	resetNewsEvents();
	if (!viaSingularity)
	{
		// normal warp.
		PayInterest();
		IncDays( 1 );
		if (Insurance)
			++NoClaim;
	}
	else
	{
		// add the singularity news story
		addNewsEvent(ARRIVALVIASINGULARITY);
	}
	Clicks = 21;
	Raided = false;
	Inspected = false;
	LitterWarning = false;
	MonsterHull = (MonsterHull * 105) / 100;
	if (MonsterHull > Shiptype[SpaceMonster.Type].HullStrength)
		MonsterHull = Shiptype[SpaceMonster.Type].HullStrength;
	if (Days%3 == 0)
	{
		if (PoliceRecordScore > CLEANSCORE)
			--PoliceRecordScore;
	}
	if (PoliceRecordScore < DUBIOUSSCORE) {
		if (Difficulty <= NORMAL)
			++PoliceRecordScore;
		else if (Days%Difficulty == 0)
			++PoliceRecordScore;
        }
		
	PossibleToGoThroughRip=true;

	DeterminePrices(WarpSystem);
	Travel();
}


// *************************************************************************
// Show the target system form
// *************************************************************************
static void ShowExecuteWarp( void )
{
	FormPtr frmP;
	int Distance;

	frmP = FrmGetActiveForm();


	RectangularShortcuts( frmP, ExecuteWarpBButton );
	
	FrmDrawForm(frmP);

	EraseRectangle( 76, 18, 52, 12 );
	EraseRectangle( 76, 31, 80, 76 );
	EraseRectangle( 76, 109, 39, 12 );

	setLabelText( frmP, ExecuteWarpSystemNameLabel, SolarSystemName[SolarSystem[WarpSystem].NameIndex] );
	setLabelText( frmP, ExecuteWarpTechLevelLabel, TechLevel[SolarSystem[WarpSystem].TechLevel] );
	setLabelText( frmP, ExecuteWarpGovernmentLabel, Politics[SolarSystem[WarpSystem].Politics].Name );
	setLabelText( frmP, ExecuteWarpSizeLabel, SystemSize[SolarSystem[WarpSystem].Size] );
				
	if (WormholeExists( COMMANDER.CurSystem, WarpSystem ))
	{
		StrCopy( SBuf, "Wormhole" );
	}
	else
	{
		Distance = RealDistance( CURSYSTEM, SolarSystem[WarpSystem] );
		StrIToA( SBuf, Distance );
		StrCat( SBuf, " parsecs" );
	}

	setLabelText( frmP, ExecuteWarpRangeLabel, SBuf );
	setLabelText( frmP, ExecuteWarpPoliceLabel, Activity[Politics[SolarSystem[WarpSystem].Politics].StrengthPolice] );
	setLabelText( frmP, ExecuteWarpPiratesLabel, Activity[Politics[SolarSystem[WarpSystem].Politics].StrengthPirates] );
	StrIToA( SBuf, InsuranceMoney() + MercenaryMoney() + (Debt > 0 ?
		max( Debt / 10, 1 ) : 0 ) + WormholeTax( COMMANDER.CurSystem, WarpSystem ) );
	StrCat( SBuf, " cr." );
	setLabelText( frmP, ExecuteWarpCurrentCostsLabel, SBuf );

	if (WormholeExists( COMMANDER.CurSystem, WarpSystem ))
	{
		FrmShowObject( frmP, FrmGetObjectIndex( frmP, ExecuteWarpWarpButton ) );
		FrmShowObject( frmP, FrmGetObjectIndex( frmP, ExecuteWarpPricesButton ) );
		FrmHideObject( frmP, FrmGetObjectIndex( frmP, ExecuteWarpOutOfRangeLabel ) );
	}
	else if (Distance > GetFuel())
	{
		FrmHideObject( frmP, FrmGetObjectIndex( frmP, ExecuteWarpWarpButton ) );
		FrmHideObject( frmP, FrmGetObjectIndex( frmP, ExecuteWarpPricesButton ) );
		FrmShowObject( frmP, FrmGetObjectIndex( frmP, ExecuteWarpOutOfRangeLabel ) );
	}
	else if (Distance <= 0)
	{
		FrmHideObject( frmP, FrmGetObjectIndex( frmP, ExecuteWarpWarpButton ) );
		FrmHideObject( frmP, FrmGetObjectIndex( frmP, ExecuteWarpPricesButton ) );
		FrmHideObject( frmP, FrmGetObjectIndex( frmP, ExecuteWarpOutOfRangeLabel ) );
	}
	else
	{
		FrmShowObject( frmP, FrmGetObjectIndex( frmP, ExecuteWarpWarpButton ) );
		FrmShowObject( frmP, FrmGetObjectIndex( frmP, ExecuteWarpPricesButton ) );
		FrmHideObject( frmP, FrmGetObjectIndex( frmP, ExecuteWarpOutOfRangeLabel ) );
	}
			
	if (WormholeExists( COMMANDER.CurSystem, WarpSystem ) || Insurance ||
		Debt > 0 || Ship.Crew[1] >= 0)
		FrmShowObject( frmP, FrmGetObjectIndex( frmP, ExecuteWarpSpecificationButton ) );
	else
		FrmHideObject( frmP, FrmGetObjectIndex( frmP, ExecuteWarpSpecificationButton ) );

	FrmDrawForm ( frmP);
}

// *************************************************************************
// Draws the list of skill points on the New Commander screen
// *************************************************************************
static void NewCommanderDrawSkills( void )
   {
   int Remaining;

   EraseRectangle( 78, 40, 44, 12 );
   EraseRectangle( 78, 58, 14, 70 );

   FntSetFont( stdFont );
	
   DrawChars( DifficultyLevel[(int)Difficulty], 78, 40 );
	
   StrIToA( SBuf, COMMANDER.Pilot );
   StrCat( SBuf, " " );
   DrawChars( SBuf, 78, 73 );

   StrIToA( SBuf, COMMANDER.Fighter );
   StrCat( SBuf, " " );
   DrawChars( SBuf, 78, 88 );

   StrIToA( SBuf, COMMANDER.Trader );
   StrCat( SBuf, " " );
   DrawChars( SBuf, 78, 103 );

   StrIToA( SBuf, COMMANDER.Engineer );
   StrCat( SBuf, " " );
   DrawChars( SBuf, 78, 118 );
	
   Remaining = 2*MAXSKILL - COMMANDER.Pilot - COMMANDER.Fighter -
		COMMANDER.Trader - COMMANDER.Engineer;
   StrIToA( SBuf, Remaining );
   StrCat( SBuf, " " );
   DrawChars( SBuf, 78, 58 );
   }

// *************************************************************************
// Status of solar systems may change over time. Used on Arrival to a System
// *************************************************************************
static void ShuffleStatus( void )
{
	int i;
	
	for (i=0; i<MAXSOLARSYSTEM; ++i)
	{
		if (SolarSystem[i].Status > 0)
		{
			if (GetRandom( 100 ) < 15)
				SolarSystem[i].Status = UNEVENTFUL;
		}
		else if (GetRandom( 100 ) < 15)
			SolarSystem[i].Status = 1 + GetRandom( MAXSTATUS - 1 );
	}
}

// *************************************************************************
// Determine next system withing range
// *************************************************************************
static int NextSystemWithinRange( int Current, Boolean Back )
{
	int i = Current;

	(Back ? --i : ++i);
	
	while (true)
	{
		if (i < 0)
			i = MAXSOLARSYSTEM - 1;
		else if (i >= MAXSOLARSYSTEM)
			i = 0;
		if (i == Current)
			break;
			
		if (WormholeExists( COMMANDER.CurSystem, i ))
			return i;
		else if (RealDistance( CURSYSTEM, SolarSystem[i] ) <= GetFuel() &&
			RealDistance( CURSYSTEM, SolarSystem[i] ) > 0)
			return i;

		(Back ? --i : ++i);
	}

	return -1;
}

// *************************************************************************
// Show the average prices list
// *************************************************************************
static void ShowAveragePrices( void )
{
    FormPtr frmP;
    int i, j, pos;
    Int32 Price;

	frmP = FrmGetActiveForm();
			
	RectangularShortcuts( frmP, AveragePricesBButton );			

	FrmDrawForm( frmP );
	
	EraseRectangle( 0, 18, 128, 26 );

	if (SolarSystem[WarpSystem].Visited)
		StrCopy( SBuf, SpecialResources[SolarSystem[WarpSystem].SpecialResources] );
	else
		StrCopy( SBuf, "Special resources unknown" );
	setLabelText( frmP, AveragePricesResourcesLabel, SBuf );
	setLabelText( frmP, AveragePricesSystemNameLabel, SolarSystemName[SolarSystem[WarpSystem].NameIndex] );

	if (PriceDifferences)
	{
		FrmHideObject( frmP, FrmGetObjectIndex( frmP, AveragePricesPriceDifferencesButton ) );
		FrmShowObject( frmP, FrmGetObjectIndex( frmP, AveragePricesAbsolutePricesButton ) );
	}
	else
	{
		FrmHideObject( frmP, FrmGetObjectIndex( frmP, AveragePricesAbsolutePricesButton ) );
		FrmShowObject( frmP, FrmGetObjectIndex( frmP, AveragePricesPriceDifferencesButton ) );
	}

	FrmDrawForm ( frmP);
			
	SBufBays();
	FntSetFont( stdFont );
	i = 5 * StrLen( SBuf );
	StrCopy( SBuf2, "Bays: " );
	StrCat( SBuf2, SBuf );
	DrawChars( SBuf2, 160 - 23 - i, 113 );
			
	EraseRectangle( 0, 46, 160, 65 );
	
	for (i=0; i<MAXTRADEITEM; ++i)
	{
		Price = StandardPrice( i, SolarSystem[WarpSystem].Size,
			SolarSystem[WarpSystem].TechLevel, SolarSystem[WarpSystem].Politics,
			(SolarSystem[WarpSystem].Visited ? SolarSystem[WarpSystem].SpecialResources : -1) );
			
		if (Price > BuyPrice[i] && BuyPrice[i] > 0 && CURSYSTEM.Qty[i] > 0)
			FntSetFont( boldFont );
		else
			FntSetFont( stdFont );

		DrawChars( Tradeitem[i].Name, (i/5)*72, 46+(i%5)*13 );
		FntSetFont( stdFont );
		SBuf[0] = '\0';
		if (PriceDifferences && Price > BuyPrice[i])
			StrCat( SBuf, "+" );
		StrIToA( SBuf2, (PriceDifferences ? Price - BuyPrice[i] : Price) );
		StrCat( SBuf, SBuf2 );
		StrCat( SBuf, " cr." );
		if (Price <= 0 || (PriceDifferences && BuyPrice[i] <= 0))
			DrawChars( "---", (i/5)*95+50, 46+(i%5)*13 );
		else
		{
			j = MAXPRICEDIGITS - StrLen( SBuf );
			pos = (i/5)*95+49+j*5;
			if (PriceDifferences) {
				if (Price > BuyPrice[i])
					--pos;
				else if (Price < BuyPrice[i])
					++pos;
                        }
			DrawChars( SBuf, pos, 46+(i%5)*13 );
		}
	}			
}


// *************************************************************************
// Generate an opposing ship
// *************************************************************************
static void GenerateOpponent( char Opp )
{
	Boolean Redo;
	int i, j, sum, Tries;
	Int32 d, e, f, k, m;
	int Bays;

	Tries = 1;
	
	if (Opp == FAMOUSCAPTAIN)
	{
		// we just fudge for the Famous Captains' Ships...
		Opponent.Type = MAXSHIPTYPE - 1;
		for (i=0;i<MAXSHIELD;i++)
		{
			Opponent.Shield[i] = REFLECTIVESHIELD; 
			Opponent.ShieldStrength[i]= RSHIELDPOWER;
		}
		for (i=0;i<MAXWEAPON;i++)
		{
			Opponent.Weapon[i] = MILITARYLASERWEAPON; 
		}
		Opponent.Gadget[0]=TARGETINGSYSTEM;
		Opponent.Gadget[1]=NAVIGATINGSYSTEM;
		Opponent.Hull = Shiptype[MAXSHIPTYPE - 1].HullStrength;

		// these guys are bad-ass!
		Opponent.Crew[0] = MAXCREWMEMBER;
		Mercenary[Opponent.Crew[0]].Pilot = MAXSKILL;
		Mercenary[Opponent.Crew[0]].Fighter = MAXSKILL;
		Mercenary[Opponent.Crew[0]].Trader = MAXSKILL;
		Mercenary[Opponent.Crew[0]].Engineer = MAXSKILL;
		return;
	}

	if (Opp == MANTIS)
		Tries = 1+Difficulty;

	// The police will try to hunt you down with better ships if you are 
	// a villain, and they will try even harder when you are considered to
	// be a psychopath (or are transporting Jonathan Wild)
	
	if (Opp == POLICE)
	{
		if (PoliceRecordScore < VILLAINSCORE && WildStatus != 1)
			Tries = 3;
		else if (PoliceRecordScore < PSYCHOPATHSCORE || WildStatus == 1)
			Tries = 5;
		Tries = max( 1, Tries + Difficulty - NORMAL );
	}

	// Pirates become better when you get richer
	if (Opp == PIRATE)
	{
		Tries = 1 + (CurrentWorth() / 100000L);
		Tries = max( 1, Tries + Difficulty - NORMAL );
	}
		
	j = 0;
	if (Opp == TRADER)
		Opponent.Type = 0;
	else
		Opponent.Type = 1;

	k = (Difficulty >= NORMAL ? Difficulty - NORMAL : 0);

	while (j < Tries)
	{
		Redo = true;

		while (Redo)
		{
			d = GetRandom( 100 );
			i = 0;
			sum = Shiptype[0].Occurrence;

			while (sum < d)
			{
				if (i >= MAXSHIPTYPE-1)
					break;
				++i;
				sum += Shiptype[i].Occurrence;
			}

			if (Opp == POLICE && (Shiptype[i].Police < 0 || 
				Politics[SolarSystem[WarpSystem].Politics].StrengthPolice + k < Shiptype[i].Police))
				continue;

			if (Opp == PIRATE && (Shiptype[i].Pirates < 0 || 
				Politics[SolarSystem[WarpSystem].Politics].StrengthPirates + k < Shiptype[i].Pirates))
				continue;

			if (Opp == TRADER && (Shiptype[i].Traders < 0 || 
				Politics[SolarSystem[WarpSystem].Politics].StrengthTraders + k < Shiptype[i].Traders))
				continue;

			Redo = false;
		}
	
		if (i > Opponent.Type)
			Opponent.Type = i;
		++j;
	}

	if (Opp == MANTIS)
		Opponent.Type = MANTISTYPE;
	else	
		Tries = max( 1, (CurrentWorth() / 150000L) + Difficulty - NORMAL );

	// Determine the gadgets
	if (Shiptype[Opponent.Type].GadgetSlots <= 0)
		d = 0;
	else if (Difficulty <= HARD)
	{
		d = GetRandom( Shiptype[Opponent.Type].GadgetSlots + 1 );
		if (d < Shiptype[Opponent.Type].GadgetSlots) {
			if (Tries > 4)
				++d;
			else if (Tries > 2)
				d += GetRandom( 2 );
                }
	}
	else
		d = Shiptype[Opponent.Type].GadgetSlots;
	for (i=0; i<d; ++i)
	{
		e = 0;
		f = 0;
		while (e < Tries)
		{
			k = GetRandom( 100 );
			j = 0;
			sum = Gadgettype[0].Chance;
			while (k < sum)
			{
				if (j >= MAXGADGETTYPE - 1)
					break;
				++j;
				sum += Gadgettype[j].Chance;
			}
			if (!HasGadget( &Opponent, j ))
				if (j > f)
					f = j;
			++e;
		}
		Opponent.Gadget[i] = f;
	}
	for (i=d; i<MAXGADGET; ++i)
		Opponent.Gadget[i] = -1;

	// Determine the number of cargo bays
	Bays = Shiptype[Opponent.Type].CargoBays;
	for (i=0; i<MAXGADGET; ++i)
		if (Opponent.Gadget[i] == EXTRABAYS)
			Bays += 5;

	// Fill the cargo bays
	for (i=0; i<MAXTRADEITEM; ++i)
		Opponent.Cargo[i] = 0;

	if (Bays > 5)
	{
		if (Difficulty >= NORMAL)
		{
			m = 3 + GetRandom( Bays - 5 );
			sum = min( m, 15 );
		}
		else
			sum = Bays;
		if (Opp == POLICE)
			sum = 0;
		if (Opp == PIRATE)
		{
			if (Difficulty < NORMAL)
				sum = (sum * 4) / 5;
			else
				sum = sum / Difficulty;
		}
		if (sum < 1)
			sum = 1;
		
		i = 0;
		while (i < sum)
		{
			j = GetRandom( MAXTRADEITEM );
			k = 1 + GetRandom( 10 - j );
			if (i + k > sum)
				k = sum - i;
			Opponent.Cargo[j] += k;
			i += k;
		}
	}

	// Fill the fuel tanks
	Opponent.Fuel = Shiptype[Opponent.Type].FuelTanks;
	
	// No tribbles on board
	Opponent.Tribbles = 0;
			
	// Fill the weapon slots (if possible, at least one weapon)
	if (Shiptype[Opponent.Type].WeaponSlots <= 0)
		d = 0;
	else if (Shiptype[Opponent.Type].WeaponSlots <= 1)
		d = 1;
	else if (Difficulty <= HARD)
	{
		d = 1 + GetRandom( Shiptype[Opponent.Type].WeaponSlots );
		if (d < Shiptype[Opponent.Type].WeaponSlots) {
			if (Tries > 4 && Difficulty >= HARD)
				++d;
			else if (Tries > 3 || Difficulty >= HARD)
				d += GetRandom( 2 );
                }
	}
	else
		d = Shiptype[Opponent.Type].WeaponSlots;
	for (i=0; i<d; ++i)
	{
		e = 0;
		f = 0;
		while (e < Tries)
		{
			k = GetRandom( 100 );
			j = 0;
			sum = Weapontype[0].Chance;
			while (k < sum)
			{
				if (j >= MAXWEAPONTYPE - 1)
					break;
				++j;
				sum += Weapontype[j].Chance;
			}
			if (j > f)
				f = j;
			++e;
		}
		Opponent.Weapon[i] = f;
	}
	for (i=d; i<MAXWEAPON; ++i)
		Opponent.Weapon[i] = -1;

	// Fill the shield slots
	if (Shiptype[Opponent.Type].ShieldSlots <= 0)
		d = 0;
	else if (Difficulty <= HARD)
	{
		d = GetRandom( Shiptype[Opponent.Type].ShieldSlots + 1 );
		if (d < Shiptype[Opponent.Type].ShieldSlots) {
			if (Tries > 3)
				++d;
			else if (Tries > 1)
				d += GetRandom( 2 );
                }
	}
	else
		d = Shiptype[Opponent.Type].ShieldSlots;
	for (i=0; i<d; ++i)
	{
		e = 0;
		f = 0;
		
		while (e < Tries)
		{
			k = GetRandom( 100 );
			j = 0;
			sum = Shieldtype[0].Chance;
			while (k < sum)
			{
				if (j >= MAXSHIELDTYPE - 1)
					break;
				++j;
				sum += Shieldtype[j].Chance;
			}
			if (j > f)
				f = j;
			++e;
		}
		Opponent.Shield[i] = f;

		j = 0;
		k = 0;
		while (j < 5)
		{
			e = 1 + GetRandom( Shieldtype[Opponent.Shield[i]].Power );
			if (e > k)
				k = e;
			++j;
		}
		Opponent.ShieldStrength[i] = k;			
	}
	for (i=d; i<MAXSHIELD; ++i)
	{
		Opponent.Shield[i] = -1;
		Opponent.ShieldStrength[i] = 0;
	}

	// Set hull strength
	i = 0;
	k = 0;
	// If there are shields, the hull will probably be stronger
	if (Opponent.Shield[0] >= 0 && GetRandom( 10 ) <= 7)
		Opponent.Hull = Shiptype[Opponent.Type].HullStrength;
	else
	{
		while (i < 5)
		{
			d = 1 + GetRandom( Shiptype[Opponent.Type].HullStrength );
			if (d > k)
				k = d;
			++i;
		}
		Opponent.Hull = k;			
	}

	if (Opp == MANTIS || Opp == FAMOUSCAPTAIN)
		Opponent.Hull = Shiptype[Opponent.Type].HullStrength;


	// Set the crew. These may be duplicates, or even equal to someone aboard
	// the commander's ship, but who cares, it's just for the skills anyway.
	Opponent.Crew[0] = MAXCREWMEMBER;
	Mercenary[Opponent.Crew[0]].Pilot = 1 + GetRandom( MAXSKILL );
	Mercenary[Opponent.Crew[0]].Fighter = 1 + GetRandom( MAXSKILL );
	Mercenary[Opponent.Crew[0]].Trader = 1 + GetRandom( MAXSKILL );
	Mercenary[Opponent.Crew[0]].Engineer = 1 + GetRandom( MAXSKILL );
	if (WarpSystem == KRAVATSYSTEM && WildStatus == 1 && (GetRandom(10)<Difficulty + 1))
	{
		Mercenary[Opponent.Crew[0]].Engineer = MAXSKILL;
	}
	if (Difficulty <= HARD)
	{
		d = 1 + GetRandom( Shiptype[Opponent.Type].CrewQuarters );
		if (Difficulty >= HARD && d < Shiptype[Opponent.Type].CrewQuarters)
			++d;
	}
	else
		d = Shiptype[Opponent.Type].CrewQuarters;
	for (i=1; i<d; ++i)
		Opponent.Crew[i] = GetRandom( MAXCREWMEMBER );
	for (i=d; i<MAXCREW; ++i)
		Opponent.Crew[i] = -1;
}

// *************************************************************************
// After a warp, the quantities of items available form a system change
// slightly. After the countdown of a solar system has reached zero, the 
// quantities are reset. This ensures that it isn't really worth the commander's
// while to just travel between two neighboring systems.
// *************************************************************************
static void ChangeQuantities( void )
{
	int i, j;
	
	for (i=0; i<MAXSOLARSYSTEM; ++i)
	{
		if (SolarSystem[i].CountDown > 0)
		{
			--SolarSystem[i].CountDown;
			if (SolarSystem[i].CountDown > STARTCOUNTDOWN)
				SolarSystem[i].CountDown = STARTCOUNTDOWN;
			else if (SolarSystem[i].CountDown <= 0)
				InitializeTradeitems( i );
			else
			{
				for (j=0; j<MAXTRADEITEM; ++j)
				{
					if (((j == NARCOTICS) &&
						(!Politics[SolarSystem[i].Politics].DrugsOK)) ||
						((j == FIREARMS) &&
						(!Politics[SolarSystem[i].Politics].FirearmsOK)) ||
						(SolarSystem[i].TechLevel < Tradeitem[j].TechProduction))
					{
						SolarSystem[i].Qty[j] = 0;
						continue;
					}
					else
					{
						SolarSystem[i].Qty[j] = SolarSystem[i].Qty[j] +
							GetRandom( 5 ) - GetRandom( 5 );
						if (SolarSystem[i].Qty[j] < 0)
							SolarSystem[i].Qty[j] = 0;
					}
				}
			}
		}
	}
}

// *************************************************************************
// Money available to spend
// *************************************************************************
Int32 ToSpend( void )
{
	if (!ReserveMoney)
		return Credits;
	return max( 0,  Credits - MercenaryMoney() - InsuranceMoney() );
}

// *************************************************************************
// View high scores
// *************************************************************************
void ViewHighScores( void )
{
	FormPtr frm;
	int i;
	Int32 Percentage;
	DmOpenRef pmDB;

	pmDB = OpenDatabase();
  	DmCloseDatabase( pmDB );

	frm = FrmInitForm( HighScoresForm );
	
	for (i=0; i<MAXHIGHSCORE; ++i)
	{
		if (Hscores[i].Name[0] == '\0')
		{
			setLabelText( frm, HighScoresName0Label + i, "Empty" );
			setLabelText( frm, HighScoresScore0Label + i, "" );
			setLabelText( frm, HighScoresDays0Label + i, "" );
			setLabelText( frm, HighScoresModeWorth0Label + i, "" );
			continue;
		}
		
		setLabelText( frm, HighScoresName0Label + i, Hscores[i].Name );
		
		Percentage = GetScore( Hscores[i].Status, Hscores[i].Days, Hscores[i].Worth,
			Hscores[i].Difficulty );
		StrIToA( SBuf, (Percentage / 50L) );
		StrCat( SBuf, "." );
		StrIToA( SBuf2, ((Percentage%50L) / 5) );
		StrCat( SBuf, SBuf2 );
		StrCat( SBuf, "%" );
		setLabelText( frm, HighScoresScore0Label + i, SBuf );
		
		if (Hscores[i].Status == MOON)
			StrCopy( SBuf, "Claimed moon" );
		else if (Hscores[i].Status == RETIRED)
			StrCopy( SBuf, "Retired" );
		else
			StrCopy( SBuf, "Was killed" );
		StrCat( SBuf, " in " );
		SBufMultiples( Hscores[i].Days, "day" );
		StrCat( SBuf, ", worth" );
		setLabelText( frm, HighScoresDays0Label + i, SBuf );

		StrIToA( SBuf, Hscores[i].Worth );
		StrCat( SBuf, " credits on " );
		if (Hscores[i].Difficulty >= BEGINNER && Hscores[i].Difficulty <= IMPOSSIBLE)
		{
			StrCat( SBuf, DifficultyLevel[Hscores[i].Difficulty] );
			StrCat( SBuf, " level" );
		}
		StrToLower( SBuf2, SBuf );
		setLabelText( frm, HighScoresModeWorth0Label + i, SBuf2 );
	}
	
	FrmDoDialog( frm );

	FrmDeleteForm( frm );
}	

// *************************************************************************
// Clear high scores
// *************************************************************************
void ClearHighScores( void )
{
	int d;
	DmOpenRef pmDB;
	
	d = FrmAlert( ClearTableAlert );
	if (d == ClearTableYes)
	{
		pmDB = OpenDatabase();
		InitHighScores();
		SaveStatus( pmDB );
    	DmCloseDatabase( pmDB );
    }
}

// *************************************************************************
// Start a new game
// *************************************************************************
Boolean StartNewGame( void )
{
	int i, j, k, d, x, y;
	Boolean Redo, CloseFound, FreeWormhole;

	if (NameCommander[0] == '\0')
		StrCopy( NameCommander, "Shelby" );

	EraseRectangle( 0, 0, 160, 160 );

	FntSetFont( stdFont );
	DrawChars( "Creating Galaxy...", 44, 76 );

	// Initialize Galaxy
	i = 0;
	while (i < MAXSOLARSYSTEM)
	{
		if (i < MAXWORMHOLE)
		{
			// Place the first system somewhere in the centre
			SolarSystem[i].X = (char)(((CLOSEDISTANCE>>1) - 
				GetRandom( CLOSEDISTANCE )) + ((GALAXYWIDTH * (1 + 2*(i%3)))/6));		
			SolarSystem[i].Y = (char)(((CLOSEDISTANCE>>1) - 
				GetRandom( CLOSEDISTANCE )) + ((GALAXYHEIGHT * (i < 3 ? 1 : 3))/4));		
			Wormhole[i] = (char)i;
		}
		else
		{
			SolarSystem[i].X = (char)(1 + GetRandom( GALAXYWIDTH - 2 ));		
			SolarSystem[i].Y = (char)(1 + GetRandom( GALAXYHEIGHT - 2 ));		
		}
		
		CloseFound = false;
		Redo = false;
		if (i >= MAXWORMHOLE)
		{
			for (j=0; j<i; ++j)
			{
				//  Minimum distance between any two systems not to be accepted
				if (SqrDistance( SolarSystem[j], SolarSystem[i] ) <= SQR( MINDISTANCE + 1 ))
				{
					Redo = true;
					break;
				}

				// There should be at least one system which is closeby enough
				if (SqrDistance( SolarSystem[j], SolarSystem[i] ) < SQR( CLOSEDISTANCE ))
					CloseFound = true;
			}
		}
		if (Redo)
			continue;
		if ((i >= MAXWORMHOLE) && !CloseFound)
			continue;
		
		SolarSystem[i].TechLevel = (char)(GetRandom( MAXTECHLEVEL ));
		SolarSystem[i].Politics = (char)(GetRandom( MAXPOLITICS ));
		if (Politics[SolarSystem[i].Politics].MinTechLevel > SolarSystem[i].TechLevel)
			continue;
		if (Politics[SolarSystem[i].Politics].MaxTechLevel < SolarSystem[i].TechLevel)
			continue;

		if (GetRandom( 5 ) >= 3)
			SolarSystem[i].SpecialResources = (char)(1 + GetRandom( MAXRESOURCES - 1 ));
		else
			SolarSystem[i].SpecialResources = 0;

		SolarSystem[i].Size = (char)(GetRandom( MAXSIZE ));

		if (GetRandom( 100 ) < 15)
			SolarSystem[i].Status = 1 + GetRandom( MAXSTATUS - 1 );
		else			
			SolarSystem[i].Status = UNEVENTFUL;

		SolarSystem[i].NameIndex = i;
		SolarSystem[i].Special = -1;		
		SolarSystem[i].CountDown = 0;
		SolarSystem[i].Visited = false;
		
		InitializeTradeitems( i );
		
		++i;
	}

	// Randomize the system locations a bit more, otherwise the systems with the first
	// names in the alphabet are all in the centre
	for (i=0; i<MAXSOLARSYSTEM; ++i)
	{
		d = 0;
		while (d < MAXWORMHOLE)
		{
			if (Wormhole[d] == i)
				break;
			++d;
		}
		j = GetRandom( MAXSOLARSYSTEM );
		if (WormholeExists( j, -1 ))
			continue;
		x = SolarSystem[i].X;
		y = SolarSystem[i].Y;		
		SolarSystem[i].X = SolarSystem[j].X;
		SolarSystem[i].Y = SolarSystem[j].Y;		
		SolarSystem[j].X = x;
		SolarSystem[j].Y = y;		
		if (d < MAXWORMHOLE)
			Wormhole[d] = j;
	}

	// Randomize wormhole order
	for (i=0; i<MAXWORMHOLE; ++i)
	{
		j = GetRandom( MAXWORMHOLE );
		x = Wormhole[i];
		Wormhole[i] = Wormhole[j];
		Wormhole[j] = x;
	}

	// Initialize mercenary list
	Mercenary[0].NameIndex = 0;
	Mercenary[0].Pilot = 1;
	Mercenary[0].Fighter = 1;
	Mercenary[0].Trader = 1;
	Mercenary[0].Engineer = 1;
	
	i = 1;
	while (i <= MAXCREWMEMBER)
	{
		Mercenary[i].CurSystem = GetRandom( MAXSOLARSYSTEM );
		
		Redo = false;
		for (j=1; j<i; ++j)
		{
			// Not more than one mercenary per system
			if (Mercenary[j].CurSystem == Mercenary[i].CurSystem)
			{
				Redo = true;
				break;
			}
		}
		// can't have another mercenary on Kravat, since Zeethibal could be there
		if (Mercenary[i].CurSystem == KRAVATSYSTEM)
			Redo = true;
		if (Redo)
			continue;

		Mercenary[i].NameIndex = i;		
		Mercenary[i].Pilot = RandomSkill();
		Mercenary[i].Fighter = RandomSkill();
		Mercenary[i].Trader = RandomSkill();
		Mercenary[i].Engineer = RandomSkill();

		++i;
	}
    
    // special individuals: Zeethibal, Jonathan Wild's Nephew
    Mercenary[MAXCREWMEMBER-1].CurSystem = 255;

	// Place special events
	SolarSystem[ACAMARSYSTEM].Special  = MONSTERKILLED;
	SolarSystem[BARATASSYSTEM].Special = FLYBARATAS;
	SolarSystem[MELINASYSTEM].Special  = FLYMELINA;
	SolarSystem[REGULASSYSTEM].Special = FLYREGULAS;
	SolarSystem[ZALKONSYSTEM].Special  = DRAGONFLYDESTROYED;
	SolarSystem[JAPORISYSTEM].Special  = MEDICINEDELIVERY;
	SolarSystem[UTOPIASYSTEM].Special  = MOONBOUGHT;
	SolarSystem[DEVIDIASYSTEM].Special = JAREKGETSOUT;
	SolarSystem[KRAVATSYSTEM].Special  = WILDGETSOUT;
	
	// Assign a wormhole location endpoint for the Scarab.
	// It's possible that ALL wormhole destinations are already
	// taken. In that case, we don't offer the Scarab quest.
	FreeWormhole = false;
	k = 0;
	j = GetRandom( MAXWORMHOLE );
	while (SolarSystem[(int)Wormhole[j]].Special != -1 &&
		Wormhole[j] != GEMULONSYSTEM && Wormhole[j] != DALEDSYSTEM && Wormhole[j] != NIXSYSTEM && k < 20)
	{
		j = GetRandom( MAXWORMHOLE );
		k++;
	}
    if (k < 20)
    {
    	FreeWormhole = true;
    	SolarSystem[(int)Wormhole[j]].Special = SCARABDESTROYED;
    }

	d = 999;
	k = -1;
	for (i=0; i<MAXSOLARSYSTEM; ++i)
	{
		j = RealDistance( SolarSystem[NIXSYSTEM], SolarSystem[i] );
		if (j >= 70 && j < d && SolarSystem[i].Special < 0 &&
		    d != GEMULONSYSTEM && d!= DALEDSYSTEM)
		{
			k = i;
			d = j;
		}
	}
	if (k >= 0)
	{
		SolarSystem[k].Special = GETREACTOR;
		SolarSystem[NIXSYSTEM].Special = REACTORDELIVERED;
	}


	i = 0;
	while (i < MAXSOLARSYSTEM)
	{
		d = 1 + (GetRandom( MAXSOLARSYSTEM - 1 ));
		if (SolarSystem[d].Special < 0 && SolarSystem[d].TechLevel >= MAXTECHLEVEL-1 &&
		    d != GEMULONSYSTEM && d != DALEDSYSTEM)
		{
			SolarSystem[d].Special = ARTIFACTDELIVERY;
			break;
		}
		++i;
	}
	if (i >= MAXSOLARSYSTEM)
		SpecialEvent[ALIENARTIFACT].Occurrence = 0;


	d = 999;
	k = -1;
	for (i=0; i<MAXSOLARSYSTEM; ++i)
	{
		j = RealDistance( SolarSystem[GEMULONSYSTEM], SolarSystem[i] );
		if (j >= 70 && j < d && SolarSystem[i].Special < 0 &&
		    k != DALEDSYSTEM && k!= GEMULONSYSTEM)
		{
			k = i;
			d = j;
		}
	}
	if (k >= 0)
	{
		SolarSystem[k].Special = ALIENINVASION;
		SolarSystem[GEMULONSYSTEM].Special = GEMULONRESCUED;
	}

	d = 999;
	k = -1;
	for (i=0; i<MAXSOLARSYSTEM; ++i)
	{
		j = RealDistance( SolarSystem[DALEDSYSTEM], SolarSystem[i] );
		if (j >= 70 && j < d && SolarSystem[i].Special < 0)
		{
			k = i;
			d = j;
		}
	}
	if (k >= 0)
	{
		SolarSystem[k].Special = EXPERIMENT;
		SolarSystem[DALEDSYSTEM].Special = EXPERIMENTSTOPPED;
	}


	for (i=MOONFORSALE; i<MAXSPECIALEVENT-ENDFIXED; ++i)
	{
		for (j=0; j<SpecialEvent[i].Occurrence; ++j)
		{
			Redo = true;
			while (Redo)
			{
				d = 1 + GetRandom( MAXSOLARSYSTEM - 1 );
				if (SolarSystem[d].Special < 0) 
				{
					if (FreeWormhole || i != SCARAB)
						SolarSystem[d].Special = i;
					Redo = false;
				}
			}
		}
	}
	
	// Initialize Commander
	for (i=0; i<200; ++i)
	{
		COMMANDER.CurSystem = GetRandom( MAXSOLARSYSTEM );
		if (CURSYSTEM.Special >= 0)
			continue;
			
		// Seek at least an agricultural planet as startplanet (but not too hi-tech)
		if ((i < 100) && ((CURSYSTEM.TechLevel <= 0) ||
			(CURSYSTEM.TechLevel >= 6)))
			continue;

		// Make sure at least three other systems can be reached
		d = 0;
		for (j=0; j<MAXSOLARSYSTEM; ++j)
		{
			if (j == COMMANDER.CurSystem)
				continue;
			if (SqrDistance( SolarSystem[j], CURSYSTEM ) <= SQR( Shiptype[1].FuelTanks ))
			{
				++d;
				if (d >= 3)
					break;
			}
		}
		if (d < 3)
			continue;
			
		break;
	}

	Credits = 1000;
	Debt = 0;
	Days = 0;
	WarpSystem = COMMANDER.CurSystem;
	PoliceKills = 0; 
	TraderKills = 0; 
	PirateKills = 0; 
	PoliceRecordScore = 0;
	ReputationScore = 0;
	MonsterStatus = 0;
	DragonflyStatus = 0;
	ScarabStatus = 0;
	JaporiDiseaseStatus = 0;
	MoonBought = false;
	MonsterHull = Shiptype[SpaceMonster.Type].HullStrength;
	EscapePod = false;
	Insurance = false;
	RemindLoans = true;
	NoClaim = 0;
	ArtifactOnBoard = false;
	for (i=0; i<MAXTRADEITEM; ++i)
		BuyingPrice[i] = 0;
	TribbleMessage = false;
	JarekStatus = 0;
	InvasionStatus = 0;
	ExperimentStatus = 0;
	FabricRipProbability = 0;
	PossibleToGoThroughRip = false;
	ArrivedViaWormhole = false;
	VeryRareEncounter = 0;
	resetNewsEvents();
	WildStatus = 0;
	ReactorStatus = 0;
	TrackedSystem = -1;
	ShowTrackedRange = true;
	JustLootedMarie = false;
	ChanceOfVeryRareEncounter = CHANCEOFVERYRAREENCOUNTER;
	AlreadyPaidForNewspaper = false;
	CanSuperWarp = false;
	GameLoaded = false;

	// Initialize Ship
	Ship.Type =	1;
	for (i=0; i<MAXTRADEITEM; ++i)
		Ship.Cargo[i] = 0;
	Ship.Weapon[0] = 0;
	for (i=1; i<MAXWEAPON; ++i)
		Ship.Weapon[i] = -1;
	for (i=0; i<MAXSHIELD; ++i)
	{
		Ship.Shield[i] = -1;
		Ship.ShieldStrength[i] = 0;
	}
	for (i=0; i<MAXGADGET; ++i)
		Ship.Gadget[i] = -1;
	Ship.Crew[0] = 0;
	for (i=1; i<MAXCREW; ++i)
		Ship.Crew[i] = -1;
	Ship.Fuel = GetFuelTanks();
	Ship.Hull = Shiptype[Ship.Type].HullStrength;
	Ship.Tribbles = 0;

	EraseRectangle( 0, 0, 160, 160 );

	CurForm = NewCommanderForm;
	FrmGotoForm( CurForm );
	
	return true;
}

// *************************************************************************
// Increase Days (used in Encounter Module as well)
// *************************************************************************
void IncDays( int Amount )
{
	Days += Amount;
	if (InvasionStatus > 0 && InvasionStatus < 8)
	{
		InvasionStatus += Amount;
		if (InvasionStatus >= 8)
		{
			SolarSystem[GEMULONSYSTEM].Special = GEMULONINVADED;
			SolarSystem[GEMULONSYSTEM].TechLevel = 0;
			SolarSystem[GEMULONSYSTEM].Politics = ANARCHY;
		}
	}
	
	if (ReactorStatus > 0 && ReactorStatus < 21)
	{
		ReactorStatus += Amount;
		if (ReactorStatus > 20)
			ReactorStatus = 20;

	}
	
	if (ExperimentStatus > 0 && ExperimentStatus < 12)
	{
		ExperimentStatus += Amount;
		if (ExperimentStatus > 11)
		{
			FabricRipProbability = FABRICRIPINITIALPROBABILITY;
			SolarSystem[DALEDSYSTEM].Special = EXPERIMENTNOTSTOPPED;
			// in case Amount > 1
			ExperimentStatus = 12;
			FrmAlert(ExperimentPerformedAlert);
			addNewsEvent(EXPERIMENTPERFORMED);			
		}
	}
	else if (ExperimentStatus == 12 && FabricRipProbability > 0)
	{
		FabricRipProbability -= Amount;
	}
}

// *************************************************************************
// Travelling to the target system
// *************************************************************************
void Travel( void ) 
{
	int EncounterTest, StartClicks, i, j, Repairs, FirstEmptySlot, rareEncounter;
	Boolean Pirate, Trader, Police, Mantis, TryAutoRepair, FoodOnBoard, EasterEgg;
	Boolean HaveMilitaryLaser, HaveReflectiveShield;
	Int32 previousTribbles;

	Pirate = false;
	Trader = false;
	Police = false;
	Mantis = false;
	HaveMilitaryLaser = HasWeapon(&Ship, MILITARYLASERWEAPON, true);
	HaveReflectiveShield = HasShield(&Ship, REFLECTIVESHIELD);
			
	// if timespace is ripped, we may switch the warp system here.
	if (PossibleToGoThroughRip &&
	    ExperimentStatus == 12 && FabricRipProbability > 0 &&
	    (GetRandom(100) < FabricRipProbability || FabricRipProbability == 25))
	{
			FrmAlert(FlyInFabricRipAlert);
			WarpSystem = GetRandom(MAXSOLARSYSTEM);
	}
		
	PossibleToGoThroughRip=false;
	
	StartClicks = Clicks;
	--Clicks;
	
	while (Clicks > 0)
	{
		// Engineer may do some repairs
		Repairs = GetRandom( EngineerSkill( &Ship ) ) >> 1;
		Ship.Hull += Repairs;
		if (Ship.Hull > GetHullStrength())
		{
			Repairs = Ship.Hull - GetHullStrength();
			Ship.Hull = GetHullStrength();
		}
		else
			Repairs = 0;
		
		// Shields are easier to repair
		Repairs = 2 * Repairs;
		for (i=0; i<MAXSHIELD; ++i)
		{
			if (Ship.Shield[i] < 0)
				break;
			Ship.ShieldStrength[i] += Repairs;
			if (Ship.ShieldStrength[i] > Shieldtype[Ship.Shield[i]].Power)
			{
				Repairs = Ship.ShieldStrength[i] - Shieldtype[Ship.Shield[i]].Power;
				Ship.ShieldStrength[i] = Shieldtype[Ship.Shield[i]].Power;
			}
			else
				Repairs = 0;
		}
	
		// Encounter with space monster
		if ((Clicks == 1) && (WarpSystem == ACAMARSYSTEM) && (MonsterStatus == 1))
		{
			MemMove( &Opponent, &SpaceMonster, sizeof( Opponent ) );
			Opponent.Hull = MonsterHull;
			Mercenary[Opponent.Crew[0]].Pilot = 8 + Difficulty;
			Mercenary[Opponent.Crew[0]].Fighter = 8 + Difficulty;
			Mercenary[Opponent.Crew[0]].Trader = 1;
			Mercenary[Opponent.Crew[0]].Engineer = 1 + Difficulty;
			if (Cloaked( &Ship, &Opponent ))
				EncounterType = SPACEMONSTERIGNORE;
			else
				EncounterType = SPACEMONSTERATTACK;
			CurForm = EncounterForm;
			FrmGotoForm( CurForm );
			return;
		}
		
		// Encounter with the stolen Scarab
		if (Clicks == 20 && SolarSystem[WarpSystem].Special == SCARABDESTROYED &&
			ScarabStatus == 1 && ArrivedViaWormhole)
		{
			MemMove( &Opponent, &Scarab, sizeof( Opponent ) );
			Mercenary[Opponent.Crew[0]].Pilot = 5 + Difficulty;
			Mercenary[Opponent.Crew[0]].Fighter = 6 + Difficulty;
			Mercenary[Opponent.Crew[0]].Trader = 1;
			Mercenary[Opponent.Crew[0]].Engineer = 6 + Difficulty;
			if (Cloaked( &Ship, &Opponent ))
				EncounterType = SCARABIGNORE;
			else
				EncounterType = SCARABATTACK;
			CurForm = EncounterForm;
			FrmGotoForm( CurForm );
			return;
		} 
		// Encounter with stolen Dragonfly
		if ((Clicks == 1) && (WarpSystem == ZALKONSYSTEM) && (DragonflyStatus == 4))
		{
			MemMove( &Opponent, &Dragonfly, sizeof( Opponent ) );
			Mercenary[Opponent.Crew[0]].Pilot = 4 + Difficulty;
			Mercenary[Opponent.Crew[0]].Fighter = 6 + Difficulty;
			Mercenary[Opponent.Crew[0]].Trader = 1;
			Mercenary[Opponent.Crew[0]].Engineer = 6 + Difficulty;
			if (Cloaked( &Ship, &Opponent ))
				EncounterType = DRAGONFLYIGNORE;
			else
				EncounterType = DRAGONFLYATTACK;
			CurForm = EncounterForm;
			FrmGotoForm( CurForm );
			return;
		}
			
		if (WarpSystem == GEMULONSYSTEM && InvasionStatus > 7)
		{
			if (GetRandom( 10 ) > 4)
				Mantis = true;
		}
		else
		{
			// Check if it is time for an encounter
			EncounterTest = GetRandom( 44 - (2 * Difficulty) );
			
			// encounters are half as likely if you're in a flea.
			if (Ship.Type == 0)
				EncounterTest *= 2;
			
			if (EncounterTest < Politics[SolarSystem[WarpSystem].Politics].StrengthPirates &&
				!Raided) // When you are already raided, other pirates have little to gain
				Pirate = true;
			else if (EncounterTest < 
				Politics[SolarSystem[WarpSystem].Politics].StrengthPirates +
				STRENGTHPOLICE( WarpSystem ))
				// StrengthPolice adapts itself to your criminal record: you'll
				// encounter more police if you are a hardened criminal.
				Police = true;
			else if (EncounterTest < 
				Politics[SolarSystem[WarpSystem].Politics].StrengthPirates +
				STRENGTHPOLICE( WarpSystem ) +
				Politics[SolarSystem[WarpSystem].Politics].StrengthTraders)
				Trader = true;
			else if (WildStatus == 1 && WarpSystem == KRAVATSYSTEM)
			{
				// if you're coming in to Kravat & you have Wild onboard, there'll be swarms o' cops.
				rareEncounter = GetRandom(100);
				if (Difficulty <= EASY && rareEncounter < 25)
				{
					Police = true;
				}
				else if (Difficulty == NORMAL && rareEncounter < 33)
				{
					Police = true;
				}
				else if (Difficulty > NORMAL && rareEncounter < 50)
				{
					Police = true;
				}
			}	
			if (!(Trader || Police || Pirate))
				if (ArtifactOnBoard && GetRandom( 20 ) <= 3)
					Mantis = true;
		}
			
		// Encounter with police
		if (Police)
		{
			GenerateOpponent( POLICE );
			EncounterType = POLICEIGNORE;
			// If you are cloaked, they don't see you
			if (Cloaked( &Ship, &Opponent ))
				EncounterType = POLICEIGNORE;
			else if (PoliceRecordScore < DUBIOUSSCORE)
			{
				// If you're a criminal, the police will tend to attack
				if (TotalWeapons( &Opponent, -1, -1) <= 0)
				{
					if (Cloaked( &Opponent, &Ship ))
						EncounterType = POLICEIGNORE;
					else
						EncounterType = POLICEFLEE;
				}
				if (ReputationScore < AVERAGESCORE)
					EncounterType = POLICEATTACK;
				else if (GetRandom( ELITESCORE ) > (ReputationScore / (1 + Opponent.Type)))
					EncounterType = POLICEATTACK;
				else if (Cloaked( &Opponent, &Ship ))
					EncounterType = POLICEIGNORE;
				else
					EncounterType = POLICEFLEE;
			}
			else if (PoliceRecordScore >= DUBIOUSSCORE && 
				PoliceRecordScore < CLEANSCORE && !Inspected)
			{
				// If you're reputation is dubious, the police will inspect you
				EncounterType = POLICEINSPECTION;
				Inspected = true;
			}
			else if (PoliceRecordScore < LAWFULSCORE)
			{
				// If your record is clean, the police will inspect you with a chance of 10% on Normal
				if (GetRandom( 12 - Difficulty ) < 1 && !Inspected)
				{
					EncounterType = POLICEINSPECTION;
					Inspected = true;
				}
			}
			else
			{
				// If your record indicates you are a lawful trader, the chance on inspection drops to 2.5%
				if (GetRandom( 40 ) == 1 && !Inspected)
				{
					EncounterType = POLICEINSPECTION;
					Inspected = true;
				}
			}
			
			// if you're suddenly stuck in a lousy ship, Police won't flee even if you
			// have a fearsome reputation.
			if (EncounterType == POLICEFLEE && Opponent.Type > Ship.Type)
			{
				if (PoliceRecordScore < DUBIOUSSCORE)
				{
					EncounterType = POLICEATTACK;
				}
				else
				{
					EncounterType = POLICEINSPECTION;
				}
			}
			
			// If they ignore you and you can't see them, the encounter doesn't take place
			if (EncounterType == POLICEIGNORE && Cloaked( &Opponent, &Ship ))
			{
				--Clicks;
				continue;
			}


			// If you automatically don't want to confront someone who ignores you, the
			// encounter may not take place
			if (AlwaysIgnorePolice && (EncounterType == POLICEIGNORE || 
				EncounterType == POLICEFLEE))
			{
				--Clicks;
				continue;
			}

			CurForm = EncounterForm;
			FrmGotoForm( CurForm );
			return;
		}
		// Encounter with pirate
		else if (Pirate || Mantis)
		{
			if (Mantis)
				GenerateOpponent( MANTIS );
			else
				GenerateOpponent( PIRATE );

			// If you have a cloak, they don't see you
			if (Cloaked( &Ship, &Opponent ))
				EncounterType = PIRATEIGNORE;

			// Pirates will mostly attack, but they are cowardly: if your rep is too high, they tend to flee
			else if (Opponent.Type >= 7 ||
				GetRandom( ELITESCORE ) > (ReputationScore * 4) / (1 + Opponent.Type))
				EncounterType = PIRATEATTACK;
			else
				EncounterType = PIRATEFLEE;

			if (Mantis)
				EncounterType = PIRATEATTACK;

			// if Pirates are in a better ship, they won't flee, even if you have a very scary
			// reputation.
			if (EncounterType == PIRATEFLEE && Opponent.Type > Ship.Type)
			{
				EncounterType = PIRATEATTACK;
			}
			
			
			// If they ignore you or flee and you can't see them, the encounter doesn't take place
			if ((EncounterType == PIRATEIGNORE || EncounterType == PIRATEFLEE) && 
				Cloaked( &Opponent, &Ship ))
			{
				--Clicks;
				continue;
			}
			if (AlwaysIgnorePirates && (EncounterType == PIRATEIGNORE ||
				EncounterType == PIRATEFLEE))
			{
				--Clicks;
				continue;
			}
			CurForm = EncounterForm;
			FrmGotoForm( CurForm );
			return;
		}
		// Encounter with trader
		else if (Trader)
		{	
			GenerateOpponent( TRADER );
			EncounterType = TRADERIGNORE;
			// If you are cloaked, they don't see you
			if (Cloaked( &Ship, &Opponent ))
				EncounterType = TRADERIGNORE;
			// If you're a criminal, traders tend to flee if you've got at least some reputation
			else if (PoliceRecordScore <= CRIMINALSCORE)
			{
				if (GetRandom( ELITESCORE ) <= (ReputationScore * 10) / (1 + Opponent.Type))
				{
					if (Cloaked( &Opponent, &Ship ))
						EncounterType = TRADERIGNORE;
					else
						EncounterType = TRADERFLEE;
				}
			}
			
			// Will there be trade in orbit?
			if (EncounterType == TRADERIGNORE && (GetRandom(1000) < ChanceOfTradeInOrbit))
			{
				if (FilledCargoBays() < TotalCargoBays() &&
				    HasTradeableItems(&Opponent, WarpSystem, TRADERSELL))
					EncounterType = TRADERSELL;
				
				// we fudge on whether the trader has capacity to carry the stuff he's buying.
				if (HasTradeableItems(&Ship, WarpSystem, TRADERBUY) && EncounterType != TRADERSELL)
					EncounterType = TRADERBUY;
			}
			
			// If they ignore you and you can't see them, the encounter doesn't take place
			if ((EncounterType == TRADERIGNORE || EncounterType == TRADERFLEE ||
				 EncounterType == TRADERSELL || EncounterType == TRADERBUY) && 
				Cloaked( &Opponent, &Ship ))
			{
				--Clicks;
				continue;
			}
			// pay attention to user's prefs with regard to ignoring traders
			if (AlwaysIgnoreTraders && (EncounterType == TRADERIGNORE ||
				EncounterType == TRADERFLEE))
			{
				--Clicks;
				continue;
			}
			// pay attention to user's prefs with regard to ignoring trade in orbit
			if (AlwaysIgnoreTradeInOrbit && (EncounterType == TRADERBUY ||
				EncounterType == TRADERSELL))
			{
				--Clicks;
				continue;
			}
			
			CurForm = EncounterForm;
			FrmGotoForm( CurForm );
			return;
		}
		// Very Rare Random Events:
		// 1. Encounter the abandoned Marie Celeste, which you may loot.
		// 2. Captain Ahab will trade your Reflective Shield for skill points in Piloting.
		// 3. Captain Conrad will trade your Military Laser for skill points in Engineering.
		// 4. Captain Huie will trade your Military Laser for points in Trading.
		// 5. Encounter an out-of-date bottle of Captain Marmoset's Skill Tonic. This
		//    will affect skills depending on game difficulty level.
		// 6. Encounter a good bottle of Captain Marmoset's Skill Tonic, which will invoke
		//    IncreaseRandomSkill one or two times, depending on game difficulty.
		else if ((Days > 10) && (GetRandom(1000) < ChanceOfVeryRareEncounter ))
		{
			rareEncounter = GetRandom(MAXVERYRAREENCOUNTER);
			
			switch (rareEncounter)
				{
				case MARIECELESTE:
					if (!(VeryRareEncounter & (Byte)ALREADYMARIE))
					{
						VeryRareEncounter += ALREADYMARIE;
						EncounterType = MARIECELESTEENCOUNTER;
						GenerateOpponent( TRADER );
						for (i=0;i<MAXTRADEITEM;i++)
						{
							Opponent.Cargo[i]=0;
						}
						Opponent.Cargo[NARCOTICS] = min(Shiptype[Opponent.Type].CargoBays,5);
						CurForm = EncounterForm;
						FrmGotoForm( CurForm );
						return;
					}
				break;
				
				case CAPTAINAHAB:
					if (HaveReflectiveShield && COMMANDER.Pilot < 10 &&
					    PoliceRecordScore > CRIMINALSCORE &&
					    !(VeryRareEncounter & (Byte)ALREADYAHAB))
					{
						VeryRareEncounter += ALREADYAHAB;
						EncounterType = CAPTAINAHABENCOUNTER;
						GenerateOpponent( FAMOUSCAPTAIN );
						CurForm = EncounterForm;
						FrmGotoForm( CurForm );
						return;
					}
				break;
				
				case CAPTAINCONRAD:
					if (HaveMilitaryLaser && COMMANDER.Engineer < 10 &&
						PoliceRecordScore > CRIMINALSCORE &&
					    !(VeryRareEncounter & (Byte)ALREADYCONRAD))
					{
						VeryRareEncounter += ALREADYCONRAD;
						EncounterType = CAPTAINCONRADENCOUNTER;
						GenerateOpponent( FAMOUSCAPTAIN );
						CurForm = EncounterForm;
						FrmGotoForm( CurForm );
						return;
					}
				break;
				
				case CAPTAINHUIE:
					if (HaveMilitaryLaser && COMMANDER.Trader < 10 &&
						PoliceRecordScore > CRIMINALSCORE &&
					    !(VeryRareEncounter & (Byte)ALREADYHUIE))
					{
						VeryRareEncounter = VeryRareEncounter | ALREADYHUIE;
						EncounterType = CAPTAINHUIEENCOUNTER;
						GenerateOpponent( FAMOUSCAPTAIN );
						CurForm = EncounterForm;
						FrmGotoForm( CurForm );
						return;
					}
				break;
				case BOTTLEOLD:
					if  (!(VeryRareEncounter & (Byte)ALREADYBOTTLEOLD))
					{
						VeryRareEncounter = VeryRareEncounter | ALREADYBOTTLEOLD;
						EncounterType = BOTTLEOLDENCOUNTER;
						GenerateOpponent( TRADER );
						Opponent.Type = BOTTLETYPE;
						Opponent.Hull = 10;
						CurForm = EncounterForm;
						FrmGotoForm( CurForm );
						return;
					}
				break;
				case BOTTLEGOOD:
					if  (!(VeryRareEncounter & (Byte)ALREADYBOTTLEGOOD))
					{
						VeryRareEncounter = VeryRareEncounter | ALREADYBOTTLEGOOD;
						EncounterType = BOTTLEGOODENCOUNTER;
						GenerateOpponent( TRADER );
						Opponent.Type = BOTTLETYPE;
						Opponent.Hull = 10;
						CurForm = EncounterForm;
						FrmGotoForm( CurForm );
						return;
					}
				break;
				}
		}
				
		--Clicks;
	}
	
	// ah, just when you thought you were gonna get away with it...
	if (JustLootedMarie)
		{			
			GenerateOpponent( POLICE );
			EncounterType = POSTMARIEPOLICEENCOUNTER;
			JustLootedMarie = false;
			Clicks++;
			CurForm = EncounterForm;
			FrmGotoForm( CurForm );
			return;
		}

	// Arrival in the target system
	FrmAlert( (StartClicks > 20 ? UneventfulTripAlert : ArrivalAlert ) );
	
	// Check for Large Debt - 06/30/01 SRA 
	if (Debt >= 75000 ) FrmAlert (DebtWarningAlert);

	// Debt Reminder
	if (Debt > 0 && RemindLoans && Days % 5 == 0)
	{
		StrIToA(SBuf2, Debt);
		FrmCustomAlert( LoanAmountAlert, SBuf2, " ", " ");
	}
	
	Arrival();

	// Reactor warnings:	
	// now they know the quest has a time constraint!
	if (ReactorStatus == 2) FrmAlert( ReactorConsumeAlert);
	// better deliver it soon!
	else if (ReactorStatus == 16) FrmAlert( ReactorNoiseAlert);
	// last warning!
	else if (ReactorStatus == 18) FrmAlert( ReactorSmokeAlert);
	
	if (ReactorStatus == 20)
	{
		FrmAlert( ReactorMeltdownAlert );
		ReactorStatus = 0;
		if (EscapePod)
		{
			EscapeWithPod();
			return;
		}
		else
		{
			FrmAlert( ShipDestroyedAlert );
			CurForm = DestroyedForm;
			FrmGotoForm( CurForm );
			return;
		}
		
	}

	if (TrackAutoOff && TrackedSystem == COMMANDER.CurSystem)
	{
		TrackedSystem = -1;
	}

	FoodOnBoard = false;
	previousTribbles = Ship.Tribbles;
	
	if (Ship.Tribbles > 0 && ReactorStatus > 0 && ReactorStatus < 21)
	{
		Ship.Tribbles /= 2;
		if (Ship.Tribbles < 10)
		{
			Ship.Tribbles = 0;
			FrmAlert ( TribblesAllIrradiatedAlert );
		}
		else
		{
			FrmAlert ( TribblesIrradiatedAlert );
		}
	}
	else if (Ship.Tribbles > 0 && Ship.Cargo[NARCOTICS] > 0)
	{
		Ship.Tribbles = 1 + GetRandom( 3 );
		j = 1 + GetRandom( 3 );
		i = min( j, Ship.Cargo[NARCOTICS] );
		BuyingPrice[NARCOTICS] = (BuyingPrice[NARCOTICS] * 
			(Ship.Cargo[NARCOTICS] - i)) / Ship.Cargo[NARCOTICS];
		Ship.Cargo[NARCOTICS] -= i;
		Ship.Cargo[FURS] += i;
		FrmAlert( TribblesAteNarcoticsAlert );
	}
	else if (Ship.Tribbles > 0 && Ship.Cargo[FOOD] > 0)
	{
		Ship.Tribbles += 100 + GetRandom( Ship.Cargo[FOOD] * 100 );
		i = GetRandom( Ship.Cargo[FOOD] );
		BuyingPrice[FOOD] = (BuyingPrice[FOOD] * i) / Ship.Cargo[FOOD];
		Ship.Cargo[FOOD] = i; 
		FrmAlert( TribblesAteFoodAlert );
		FoodOnBoard = true;
	}

	if (Ship.Tribbles > 0 && Ship.Tribbles < MAXTRIBBLES)
		Ship.Tribbles += 1 + GetRandom( max( 1, (Ship.Tribbles >> (FoodOnBoard ? 0 : 1)) ) );
		
	if (Ship.Tribbles > MAXTRIBBLES)
		Ship.Tribbles = MAXTRIBBLES;

	if ((previousTribbles < 100 && Ship.Tribbles >= 100) ||
		(previousTribbles < 1000 && Ship.Tribbles >= 1000) ||
		(previousTribbles < 10000 && Ship.Tribbles >= 10000) ||
		(previousTribbles < 50000 && Ship.Tribbles >= 50000))
	{
		if (Ship.Tribbles >= MAXTRIBBLES)
			StrCopy( SBuf, "a dangerous number of" );
		else
			StrPrintF(SBuf, "%ld", Ship.Tribbles);
		FrmCustomAlert( TribblesOnBoardAlert, SBuf, NULL, NULL);
	}
	
	TribbleMessage = false;

	Ship.Hull += GetRandom( EngineerSkill( &Ship ) );
	if (Ship.Hull > GetHullStrength())
		Ship.Hull = GetHullStrength();

	TryAutoRepair = true;
	if (AutoFuel)
	{
		BuyFuel( 999 );
		if (GetFuel() < GetFuelTanks())
		{
			if (AutoRepair && Ship.Hull < GetHullStrength())
			{
				FrmAlert( NoFullTanksOrRepairsAlert );
				TryAutoRepair = false;
			}
			else
				FrmAlert( NoFullTanksAlert );
		}
	}
	
	if (AutoRepair && TryAutoRepair)
	{	
		BuyRepairs( 9999 );
		if (Ship.Hull < GetHullStrength())
			FrmAlert( NoFullRepairsAlert );
	}
	
    /* This Easter Egg gives the commander a Lighting Shield */
	if (COMMANDER.CurSystem == OGSYSTEM)
	{
		i = 0;
		EasterEgg = false;
		while (i < MAXTRADEITEM)		
		{
			if (Ship.Cargo[i] != 1)
				break;
			++i;
		}
		if (i >= MAXTRADEITEM)
	    {
 		    FrmAlert( EggAlert );
 		   
		    FirstEmptySlot = GetFirstEmptySlot( Shiptype[Ship.Type].ShieldSlots, Ship.Shield );
           
            if (FirstEmptySlot >= 0)
            {
		      	Ship.Shield[FirstEmptySlot] = LIGHTNINGSHIELD;  
			  	Ship.ShieldStrength[FirstEmptySlot] = Shieldtype[LIGHTNINGSHIELD].Power;
		      	EasterEgg = true;
		    }
		      
		      
		    if (EasterEgg)
		    {
			  	for (i=0; i<MAXTRADEITEM; ++i)
			    {
				 	Ship.Cargo[i] = 0;
				 	BuyingPrice[i] = 0;
				}
            }			
		}
	}
	
     
	// It seems a glitch may cause cargo bays to become negative - no idea how...
	for (i=0; i<MAXTRADEITEM; ++i)
		if (Ship.Cargo[i] < 0)
			Ship.Cargo[i] = 0;
			
	CurForm = SystemInformationForm;
	FrmGotoForm( CurForm );
}

// *************************************************************************
// Returns true if there exists a wormhole from a to b. 
// If b < 0, then return true if there exists a wormhole 
// at all from a.
// *************************************************************************
Boolean WormholeExists( int a, int b )
{
	int i;

	i = 0;
	while (i<MAXWORMHOLE)
	{
		if (Wormhole[i] == a)
			break;
		++i;
	}
	
	if (i < MAXWORMHOLE)
	{
		if (b < 0)
			return true;
		else if (i < MAXWORMHOLE - 1)
		{
			if (Wormhole[i+1] == b)
				return true;
		}
		else if (Wormhole[0] == b)
			return true;
			
	}
	
	return false;
}


// *************************************************************************
// Standard handling of arrival
// *************************************************************************
void Arrival( void )
{
	COMMANDER.CurSystem = WarpSystem;
	ShuffleStatus();
	ChangeQuantities();
	DeterminePrices(COMMANDER.CurSystem);
	AlreadyPaidForNewspaper = false;

}


// *************************************************************************
// This routine is the event handler for the Retire, Destroyed and Utopia screens.
// *************************************************************************
Boolean RetireDestroyedUtopiaFormHandleEvent( EventPtr eventP, char EndStatus )
{
    Boolean handled = false;
    FormPtr frmP = FrmGetActiveForm();

	switch (eventP->eType) 
	{
		case frmOpenEvent:
			FrmDrawForm ( frmP );
			handled = true;
			break;

		case penDownEvent:
		case keyDownEvent:
			EraseRectangle( 0, 0, 160, 160 );
			EndOfGame( EndStatus );
			handled = true;
			break;

		default:
			break;
	}
	
	return handled;
}


// *************************************************************************
// This routine is the event handler for the Utopia screen.
// *************************************************************************
Boolean UtopiaFormHandleEvent( EventPtr eventP )
{
	return RetireDestroyedUtopiaFormHandleEvent( eventP, MOON );
}


// *************************************************************************
// This routine is the event handler for the Destroyed screen.
// *************************************************************************
Boolean DestroyedFormHandleEvent( EventPtr eventP )
{
	return RetireDestroyedUtopiaFormHandleEvent( eventP, KILLED );
}


// *************************************************************************
// This routine is the event handler for the Retire screen.
// *************************************************************************
Boolean RetireFormHandleEvent( EventPtr eventP )
{
	return RetireDestroyedUtopiaFormHandleEvent( eventP, RETIRED );
}


// *************************************************************************
// Determine if ship is cloaked
// *************************************************************************
Boolean Cloaked( SHIP* Sh, SHIP* Opp )
{
	return (HasGadget( Sh, CLOAKINGDEVICE ) && (EngineerSkill( Sh ) > EngineerSkill( Opp )));
}


// *************************************************************************
// Determine first empty slot, return -1 if none
// *************************************************************************
int GetFirstEmptySlot( char Slots, int* Item )
{
	int FirstEmptySlot, j;

	FirstEmptySlot = -1;
	for (j=0; j<Slots; ++j)
	{
		if (Item[j] < 0)
		{
			FirstEmptySlot = j;
			break;
		}							
	}
	
	return FirstEmptySlot;
}


// *************************************************************************
// Determine if there are any empty slots.
// *************************************************************************
Boolean AnyEmptySlots( SHIP *ship )
{
	int j;

	for (j=0; j<Shiptype[ship->Type].WeaponSlots; ++j)
	{
		if (ship->Weapon[j] < 0)
		{
			return true;
		}							
	}
	for (j=0; j<Shiptype[ship->Type].ShieldSlots; ++j)
	{
		if (ship->Shield[j] < 0)
		{
			return true;
		}							
	}
	for (j=0; j<Shiptype[ship->Type].GadgetSlots; ++j)
	{
		if (ship->Gadget[j] < 0)
		{
			return true;
		}							
	}
	
	return false;
}

// *************************************************************************
// Handling of the New Commander
// *************************************************************************
Boolean NewCommanderFormHandleEvent(EventPtr eventP)
{
   int  CommanderNameLen;
   Boolean handled = false;
   FormPtr frmP = FrmGetActiveForm();

	switch (eventP->eType) 
	{
		case ctlSelectEvent:
			// Closing the New Commander screen
			if (eventP->data.ctlSelect.controlID == NewCommanderOKButton)
			{
				if (2*MAXSKILL - COMMANDER.Pilot - COMMANDER.Fighter -
					COMMANDER.Trader - COMMANDER.Engineer > 0)
				{
					FrmAlert( MoreSkillPointsAlert);
					handled = true;
					break;
				}

				GetField( frmP, NewCommanderEditNameField, NameCommander, NameH );

				DeterminePrices(COMMANDER.CurSystem);

				if (Difficulty < NORMAL)
					if (CURSYSTEM.Special < 0)
						CURSYSTEM.Special = LOTTERYWINNER;
					
				CurForm = SystemInformationForm;
				FrmGotoForm( CurForm );
				handled = true;
				break;
			}
			// Tapping of one of the skill increase or decrease buttons
			switch (eventP->data.ctlSelect.controlID)
			{
				case NewCommanderDecDifficultyButton:
					if (Difficulty > 0)
						--Difficulty;
					break;

				case NewCommanderIncDifficultyButton:
					if (Difficulty < MAXDIFFICULTY - 1)
						++Difficulty;
					break;

				case NewCommanderDecPilotButton:
					if (COMMANDER.Pilot > 1)
						--COMMANDER.Pilot;
					break;

				case NewCommanderIncPilotButton:
					if (COMMANDER.Pilot < MAXSKILL)
						if (2*MAXSKILL - COMMANDER.Pilot - COMMANDER.Fighter -
							COMMANDER.Trader - COMMANDER.Engineer > 0)
							++COMMANDER.Pilot;
					break;

				case NewCommanderDecFighterButton:
					if (COMMANDER.Fighter > 1)
						--COMMANDER.Fighter;
					break;

				case NewCommanderIncFighterButton:
					if (COMMANDER.Fighter < MAXSKILL)
						if (2*MAXSKILL - COMMANDER.Pilot - COMMANDER.Fighter -
							COMMANDER.Trader - COMMANDER.Engineer > 0)
							++COMMANDER.Fighter;
					break;

				case NewCommanderDecTraderButton:
					if (COMMANDER.Trader > 1)
						--COMMANDER.Trader;
					break;

				case NewCommanderIncTraderButton:
					if (COMMANDER.Trader < MAXSKILL)
						if (2*MAXSKILL - COMMANDER.Pilot - COMMANDER.Fighter -
							COMMANDER.Trader - COMMANDER.Engineer > 0)
							++COMMANDER.Trader;
					break;

				case NewCommanderDecEngineerButton:
					if (COMMANDER.Engineer > 1)
						--COMMANDER.Engineer;
					break;

				case NewCommanderIncEngineerButton:
					if (COMMANDER.Engineer < MAXSKILL)
						if (2*MAXSKILL - COMMANDER.Pilot - COMMANDER.Fighter -
							COMMANDER.Trader - COMMANDER.Engineer > 0)
							++COMMANDER.Engineer;
					break;
			}
			NewCommanderDrawSkills();
			handled = true;
			break;

		case frmOpenEvent:
			// Set Commander name and skills	
			CommanderNameLen = NAMELEN+1;
			NameH = SetField( frmP, NewCommanderEditNameField, NameCommander, CommanderNameLen, true );
			GrfSetState( false, false, false );
			FrmDrawForm ( frmP);
			NewCommanderDrawSkills();
			handled = true;
			break;
			
		case frmUpdateEvent:
			FrmDrawForm ( frmP );
			NewCommanderDrawSkills();
			handled = true;
			break;

		default:
			break;
	}
	
	return handled;
}

// *************************************************************************
// Handling of the Average Prices form
// *************************************************************************
Boolean AveragePricesFormHandleEvent(EventPtr eventP)
{
	int i, d, Amount;
    Boolean handled = false;

	switch (eventP->eType) 
	{
		case penDownEvent:
			for (i=0; i<MAXTRADEITEM; ++i)
			{
				if ((eventP->screenX >= (i/5)*72) &&
					(eventP->screenX < (i<5 ? 72 : 160)) &&
					(eventP->screenY >= 46+(i%5)*13) &&
					(eventP->screenY < 46+(1+(i%5))*13))
				{
					Amount = GetAmountToBuy( i );
					if (Amount > 0)			
					{			
						BuyCargo( i, Amount, false );
						ShowAveragePrices();
					}
					handled = true;
				}
			}
			break;

		case keyDownEvent:
			d = ScrollButton( eventP );
			if (d == chrPageUp || d == chrPageDown)
			{
				i = NextSystemWithinRange( WarpSystem, (d == chrPageDown) );
				if (i >= 0)
				{
					WarpSystem = i;
					ShowAveragePrices();
				}
				handled = true;
			}
		    break;

		case ctlSelectEvent:
			switch (eventP->data.ctlSelect.controlID)
			{
				case AveragePricesScrollBackButton:
				case AveragePricesScrollFwdButton:
				{
					i = NextSystemWithinRange( WarpSystem,
						(eventP->data.ctlSelect.controlID == AveragePricesScrollBackButton) );
					if (i >= 0)
					{
						WarpSystem = i;
						ShowAveragePrices();
					}
					break;
				}

				case AveragePricesWarpButton:
				{
					DoWarp(false);
					break;
				}

				case AveragePricesSystemInfoButton:
				{
					CurForm = ExecuteWarpForm;
					FrmGotoForm( CurForm );
					break;
				}

				case AveragePricesPriceDifferencesButton:
				case AveragePricesAbsolutePricesButton:
				{
					PriceDifferences = !PriceDifferences;
					ShowAveragePrices();
					break;
				}
				
				default:
				{
					CurForm = WarpForm;
					FrmGotoForm( CurForm );
					break;
				}
			}
			handled = true;
			break;

		case frmOpenEvent:
			ShowAveragePrices();
			APLscreen = true;
			handled = true;
			break;
			
		case frmUpdateEvent:
			ShowAveragePrices();
			handled = true;
			break;

		default:
			break;
	}
	
	return handled;
}


// *************************************************************************
// Handling of the Execute Warp form
// *************************************************************************
Boolean ExecuteWarpFormHandleEvent(EventPtr eventP)
{
    Boolean handled = false;
    FormPtr frmP;
    int i, d;
	Int32 IncDebt, Total;

	switch (eventP->eType) 
	{
		case keyDownEvent:
			d = ScrollButton( eventP );
			if (d == chrPageUp || d == chrPageDown)
			{
				i = NextSystemWithinRange( WarpSystem, (d == chrPageDown) );
				if (i >= 0)
				{
					WarpSystem = i;
					ShowExecuteWarp();
				}
				handled = true;
			}
		    break;

		case ctlSelectEvent:
			switch (eventP->data.ctlSelect.controlID)
			{
				case ExecuteWarpScrollBackButton:
				case ExecuteWarpScrollFwdButton:
				{
					i = NextSystemWithinRange( WarpSystem,
						(eventP->data.ctlSelect.controlID == ExecuteWarpScrollBackButton) );
					if (i >= 0)
					{
						WarpSystem = i;
						ShowExecuteWarp();
					}
					break;
				}

				// Warp	to another system. This can only be selected if the warp is indeed possible		
				case ExecuteWarpWarpButton:
				{
					DoWarp(false);
					break;
				}
				
				case ExecuteWarpPricesButton:
				{
					CurForm = AveragePricesForm;
					FrmGotoForm( CurForm );
					break;
				}
				
				case ExecuteWarpSpecificationButton:
				{
					frmP = FrmInitForm( SpecificationForm );
	
					Total = 0L;
					
					StrIToA( SBuf, MercenaryMoney() );
					StrCat( SBuf, " cr." );
					setLabelText( frmP, SpecificationMercenariesLabel, SBuf );
					Total += MercenaryMoney();
					StrIToA( SBuf, InsuranceMoney() );
					Total += InsuranceMoney();
					StrCat( SBuf, " cr." );
					setLabelText( frmP, SpecificationInsuranceLabel, SBuf );
	
					StrIToA( SBuf, WormholeTax( COMMANDER.CurSystem, WarpSystem ) );
					StrCat( SBuf, " cr." );
					setLabelText( frmP, SpecificationWormholeTaxLabel, SBuf );
					Total += WormholeTax( COMMANDER.CurSystem, WarpSystem );

					if (Debt > 0)
					{
						IncDebt = max( 1, Debt / 10 );
						StrIToA( SBuf, IncDebt );
						Total += IncDebt;
					}
					else
						StrCopy( SBuf, "0" );
					StrCat( SBuf, " cr." );
					setLabelText( frmP, SpecificationInterestLabel, SBuf );

					StrIToA( SBuf, Total );
					StrCat( SBuf, " cr." );
					setLabelText( frmP, SpecificationTotalLabel, SBuf );

					FrmDoDialog( frmP );

					FrmDeleteForm( frmP );
					break;
				}
				
				default:
				{
					CurForm = WarpForm;
					FrmGotoForm( CurForm );
					break;
				}
			}
			handled = true;
			break;

		// Give information on the system selected on the short range chart
		case frmOpenEvent:
			ShowExecuteWarp();
			APLscreen = false;
			handled = true;
			break;
			
		case frmUpdateEvent:
			ShowExecuteWarp();
			handled = true;
			break;

		default:
			break;
	}
	
	return handled;
}

