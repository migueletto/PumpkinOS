/***********************************************************************
 *
 * SPACE TRADER 1.2.0
 *
 * Encounter.c
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
// Encounter.c - Functions in this module:
// -------------------------------------------------------------------- 
//
// Boolean EncounterFormHandleEvent ( EventPtr eventP ) 
//
// -------------------------------------------------------------------- 
// Static Local Functions
// --------------------------------------------------------------------
// Boolean ExecuteAttack( SHIP* Attacker, SHIP* Defender, Boolean Flees, Boolean CommanderUnderAttack )
// int ExecuteAction( Boolean CommanderFlees )
// Int32 TotalWeapons( SHIP* Sh )
// Int32 TotalShields( SHIP* Sh )
// Int32 TotalShieldStrength( SHIP* Sh )
// void ShowShip( SHIP* Sh, int offset, Boolean commandersShip )
// void EncounterDisplayShips( void )
// void EscapeWithPod( void )
// void Arrested( void ) 
// void Scoop( void )
// void EncounterDisplayNextAction( Boolean FirstDisplay )
// void EncounterButtons( void )
//
// -------------------------------------------------------------------------
// Modifications:
// mm/dd/yy - description - author
// 06/30/01 - Police Encounter text changed - SRA
// -------------------------------------------------------------------------
//
// *************************************************************************

#include "external.h"

static Boolean playerShipNeedsUpdate, opponentShipNeedsUpdate;

#ifdef HWATTACK
int hwbutton[4];
#endif

#define BELOW35				(romVersion < sysMakeROMVersion( 3, 5, 0, sysROMStageRelease, 0 ))

// *************************************************************************
// Calculate Bounty
// *************************************************************************
static Int32 GetBounty( SHIP* Sh )
{
	Int32 Bounty = EnemyShipPrice( Sh );
	
	Bounty /= 200;
	Bounty /= 25;	
	Bounty *= 25;
	if (Bounty <= 0)
		Bounty = 25;
	if (Bounty > 2500)
		Bounty = 2500;
	
	return Bounty;
}

// *************************************************************************
// Buttons on the encounter screen
// *************************************************************************
static void EncounterButtons( void )
{
	FormPtr frmP;

	frmP = FrmGetActiveForm();

	FrmHideObject( frmP, FrmGetObjectIndex( frmP, EncounterAttackButton ) );
	FrmHideObject( frmP, FrmGetObjectIndex( frmP, EncounterSurrenderButton ) );
	FrmHideObject( frmP, FrmGetObjectIndex( frmP, EncounterIgnoreButton ) );
	FrmHideObject( frmP, FrmGetObjectIndex( frmP, EncounterFleeButton ) );
	FrmHideObject( frmP, FrmGetObjectIndex( frmP, EncounterSubmitButton ) );
	FrmHideObject( frmP, FrmGetObjectIndex( frmP, EncounterBribeButton ) );
	FrmHideObject( frmP, FrmGetObjectIndex( frmP, EncounterPlunderButton ) );
	FrmHideObject( frmP, FrmGetObjectIndex( frmP, EncounterInterruptButton ) );
	FrmHideObject( frmP, FrmGetObjectIndex( frmP, EncounterDrinkButton ) );
	FrmHideObject( frmP, FrmGetObjectIndex( frmP, EncounterBoardButton ) );
	FrmHideObject( frmP, FrmGetObjectIndex( frmP, EncounterMeetButton ) );
	FrmHideObject( frmP, FrmGetObjectIndex( frmP, EncounterYieldButton ) );
	FrmHideObject( frmP, FrmGetObjectIndex( frmP, EncounterTradeButton ) );
	FrmHideObject( frmP, FrmGetObjectIndex( frmP, EncounterAttackBitMap ) );
	FrmHideObject( frmP, FrmGetObjectIndex( frmP, EncounterAttack2BitMap ) );

	if (AutoAttack || AutoFlee)
	{
		FrmShowObject( frmP, FrmGetObjectIndex( frmP, EncounterInterruptButton ) );
		AttackIconStatus = !AttackIconStatus;
		if (AttackIconStatus)
			FrmShowObject( frmP, FrmGetObjectIndex( frmP, EncounterAttackBitMap ) );
		else
			FrmShowObject( frmP, FrmGetObjectIndex( frmP, EncounterAttack2BitMap ) );
	}
	
	if (EncounterType == POLICEINSPECTION)
	{
		FrmShowObject( frmP, FrmGetObjectIndex( frmP, EncounterAttackButton ) );
		FrmShowObject( frmP, FrmGetObjectIndex( frmP, EncounterFleeButton ) );
		FrmShowObject( frmP, FrmGetObjectIndex( frmP, EncounterSubmitButton ) );
		FrmShowObject( frmP, FrmGetObjectIndex( frmP, EncounterBribeButton ) );
#ifdef HWATTACK
		hwbutton[0] = EncounterAttackButton;
		hwbutton[1] = EncounterFleeButton;
		hwbutton[2] = EncounterSubmitButton;
		hwbutton[3] = EncounterBribeButton;
#endif
	}
	else if (EncounterType == POSTMARIEPOLICEENCOUNTER)
	{
		FrmShowObject( frmP, FrmGetObjectIndex( frmP, EncounterAttackButton ) );
		FrmShowObject( frmP, FrmGetObjectIndex( frmP, EncounterFleeButton ) );
		FrmShowObject( frmP, FrmGetObjectIndex( frmP, EncounterYieldButton ) );
		FrmShowObject( frmP, FrmGetObjectIndex( frmP, EncounterBribeButton ) );
#ifdef HWATTACK
		hwbutton[0] = EncounterAttackButton;
		hwbutton[1] = EncounterFleeButton;
		hwbutton[2] = EncounterYieldButton;
		hwbutton[3] = EncounterBribeButton;
#endif
	}
	else if (EncounterType == POLICEFLEE || EncounterType == TRADERFLEE ||
		EncounterType == PIRATEFLEE)
	{
		FrmShowObject( frmP, FrmGetObjectIndex( frmP, EncounterAttackButton ) );
		FrmShowObject( frmP, FrmGetObjectIndex( frmP, EncounterIgnoreButton ) );
#ifdef HWATTACK
		hwbutton[0] = EncounterAttackButton;
		hwbutton[1] = EncounterIgnoreButton;
		hwbutton[2] = 0;
		hwbutton[3] = 0;
#endif
	}
	else if (EncounterType == PIRATEATTACK || EncounterType == POLICEATTACK  ||
		EncounterType == SCARABATTACK)
	{
		FrmShowObject( frmP, FrmGetObjectIndex( frmP, EncounterAttackButton ) );
		FrmShowObject( frmP, FrmGetObjectIndex( frmP, EncounterFleeButton ) );
		FrmShowObject( frmP, FrmGetObjectIndex( frmP, EncounterSurrenderButton ) );
#ifdef HWATTACK
		hwbutton[0] = EncounterAttackButton;
		hwbutton[1] = EncounterFleeButton;
		hwbutton[2] = EncounterSurrenderButton;
		hwbutton[3] = 0;
#endif
	}
	else if (EncounterType == FAMOUSCAPATTACK)
	{
		FrmShowObject( frmP, FrmGetObjectIndex( frmP, EncounterAttackButton ) );
		FrmShowObject( frmP, FrmGetObjectIndex( frmP, EncounterFleeButton ) );
#ifdef HWATTACK
		hwbutton[0] = EncounterAttackButton;
		hwbutton[1] = EncounterFleeButton;
		hwbutton[2] = 0;
		hwbutton[3] = 0;
#endif
	}
	else if (EncounterType == TRADERATTACK ||
		EncounterType == SPACEMONSTERATTACK || EncounterType == DRAGONFLYATTACK)
	{
		FrmShowObject( frmP, FrmGetObjectIndex( frmP, EncounterAttackButton ) );
		FrmShowObject( frmP, FrmGetObjectIndex( frmP, EncounterFleeButton ) );
#ifdef HWATTACK
		hwbutton[0] = EncounterAttackButton;
		hwbutton[1] = EncounterFleeButton;
		hwbutton[2] = 0;
		hwbutton[3] = 0;
#endif
	}
	else if (EncounterType == TRADERIGNORE || EncounterType == POLICEIGNORE ||
		EncounterType == PIRATEIGNORE ||
		EncounterType == SPACEMONSTERIGNORE || EncounterType == DRAGONFLYIGNORE ||
		EncounterType == SCARABIGNORE)
	{
		FrmShowObject( frmP, FrmGetObjectIndex( frmP, EncounterAttackButton ) );
		FrmShowObject( frmP, FrmGetObjectIndex( frmP, EncounterIgnoreButton ) );
#ifdef HWATTACK
		hwbutton[0] = EncounterAttackButton;
		hwbutton[1] = EncounterIgnoreButton;
		hwbutton[2] = 0;
		hwbutton[3] = 0;
#endif
	}
	else if (EncounterType == TRADERSURRENDER || EncounterType == PIRATESURRENDER)
	{
		FrmShowObject( frmP, FrmGetObjectIndex( frmP, EncounterAttackButton ) );
		FrmShowObject( frmP, FrmGetObjectIndex( frmP, EncounterPlunderButton ) );
#ifdef HWATTACK
		hwbutton[0] = EncounterAttackButton;
		hwbutton[1] = EncounterPlunderButton;
		hwbutton[2] = 0;
		hwbutton[3] = 0;
#endif
	}
	else if (EncounterType == MARIECELESTEENCOUNTER)
	{
		FrmShowObject( frmP, FrmGetObjectIndex( frmP, EncounterBoardButton ) );
		FrmShowObject( frmP, FrmGetObjectIndex( frmP, EncounterIgnoreButton ) );
#ifdef HWATTACK
		hwbutton[0] = EncounterBoardButton;
		hwbutton[1] = EncounterIgnoreButton;
		hwbutton[2] = 0;
		hwbutton[3] = 0;
#endif
	}
	else if (ENCOUNTERFAMOUS(EncounterType))
	{
		FrmShowObject( frmP, FrmGetObjectIndex( frmP, EncounterAttackButton ) );
		FrmShowObject( frmP, FrmGetObjectIndex( frmP, EncounterIgnoreButton ) );
		FrmShowObject( frmP, FrmGetObjectIndex( frmP, EncounterMeetButton ) );
#ifdef HWATTACK
		hwbutton[0] = EncounterAttackButton;
		hwbutton[1] = EncounterIgnoreButton;
		hwbutton[2] = EncounterMeetButton;
		hwbutton[3] = 0;
#endif
	}
	else if (EncounterType == BOTTLEOLDENCOUNTER ||
	         EncounterType == BOTTLEGOODENCOUNTER)
	{
		FrmShowObject( frmP, FrmGetObjectIndex( frmP, EncounterDrinkButton ) );
		FrmShowObject( frmP, FrmGetObjectIndex( frmP, EncounterIgnoreButton ) );
#ifdef HWATTACK
		hwbutton[0] = EncounterDrinkButton;
		hwbutton[1] = EncounterIgnoreButton;
		hwbutton[2] = 0;
		hwbutton[3] = 0;
#endif
	}
	else if (EncounterType == TRADERSELL || EncounterType == TRADERBUY)
	{
		FrmShowObject( frmP, FrmGetObjectIndex( frmP, EncounterAttackButton ) );
		FrmShowObject( frmP, FrmGetObjectIndex( frmP, EncounterIgnoreButton ) );
		FrmShowObject( frmP, FrmGetObjectIndex( frmP, EncounterTradeButton ) );
#ifdef HWATTACK
		hwbutton[0] = EncounterAttackButton;
		hwbutton[1] = EncounterIgnoreButton;
		hwbutton[2] = EncounterTradeButton;
		hwbutton[3] = 0;
#endif
	}
	
	if (!TextualEncounters)
	{
		FrmHideObject( frmP, FrmGetObjectIndex( frmP, EncounterYouLabel ) );
		FrmHideObject( frmP, FrmGetObjectIndex( frmP, EncounterOpponentLabel ) );
	}
}


// *************************************************************************
// Display on the encounter screen what the next action will be
// *************************************************************************
static void EncounterDisplayNextAction( Boolean FirstDisplay )
{
	if (EncounterType == POLICEINSPECTION)
	{
		DrawChars( "The police summon you to submit", 6, 106 );
		DrawChars( "to an inspection.", 6, 119 );
	}
	else if (EncounterType == POSTMARIEPOLICEENCOUNTER)
	{
		DrawChars( "\"We know you removed illegal", 6, 93 );
		DrawChars( "goods from the Marie Celeste.", 6,106 );
		DrawChars( "You must give them up at once!\"", 6, 119 );
	}
	else if (FirstDisplay && EncounterType == POLICEATTACK && 
		PoliceRecordScore > CRIMINALSCORE)
	{
		DrawChars( "The police hail they want", 6, 106 );
		DrawChars( "you to surrender.", 6, 119 );
	}
	else if (EncounterType == POLICEFLEE || EncounterType == TRADERFLEE ||
		EncounterType == PIRATEFLEE)
		DrawChars( "Your opponent is fleeing.", 6, 106 );
	else if (EncounterType == PIRATEATTACK || EncounterType == POLICEATTACK ||
		EncounterType == TRADERATTACK || EncounterType == SPACEMONSTERATTACK ||
		EncounterType == DRAGONFLYATTACK || EncounterType == SCARABATTACK ||
		EncounterType == FAMOUSCAPATTACK)
		DrawChars( "Your opponent attacks.", 6, 106 );
	else if (EncounterType == TRADERIGNORE || EncounterType == POLICEIGNORE ||
		EncounterType == SPACEMONSTERIGNORE || EncounterType == DRAGONFLYIGNORE || 
		EncounterType == PIRATEIGNORE || EncounterType == SCARABIGNORE)
	{
		if (Cloaked( &Ship, &Opponent ))
			DrawChars( "It doesn't notice you.", 6, 106 );
		else
			DrawChars( "It ignores you.", 6, 106 );
	}
	else if (EncounterType == TRADERSELL || EncounterType == TRADERBUY)
	{
		DrawChars( "You are hailed with an offer", 6, 106 );
		DrawChars( "to trade goods.", 6, 119 );
	}
	else if (EncounterType == TRADERSURRENDER || EncounterType == PIRATESURRENDER)
	{
		DrawChars( "Your opponent hails that he", 6, 106 );
		DrawChars( "surrenders to you.", 6, 119 );
	}
	else if (EncounterType == MARIECELESTEENCOUNTER)
	{
		DrawChars( "The Marie Celeste appears to", 6, 106 );
		DrawChars( "be completely abandoned.", 6, 119 );
	}
	else if (ENCOUNTERFAMOUS(EncounterType) && EncounterType != FAMOUSCAPATTACK)
	{
		DrawChars( "The Captain requests a brief", 6, 106 );
		DrawChars( "meeting with you.", 6, 119 );
	}
	else if (EncounterType == BOTTLEOLDENCOUNTER ||
	         EncounterType == BOTTLEGOODENCOUNTER)
	{
		DrawChars( "It appears to be a rare bottle", 6, 106);
		DrawChars( "of Captain Marmoset's Skill Tonic!", 6, 119);
    }

}


// *************************************************************************
// You can pick up cannisters left by a destroyed ship
// *************************************************************************
static void Scoop( void )
{
	int d, ret;
	FormPtr frm;

	// Chance 50% to pick something up on Normal level, 33% on Hard level, 25% on
	// Impossible level, and 100% on Easy or Beginner
	if (Difficulty >= NORMAL)
		if (GetRandom( Difficulty ) != 1)
			return;
	
	// More chance to pick up a cheap good
	d = GetRandom( MAXTRADEITEM );
	if (d >= 5)
		d = GetRandom( MAXTRADEITEM );
	
	frm = FrmInitForm( PickCannisterForm );
	
	StrCopy( SBuf, "ship, labeled " );
	StrCat( SBuf, Tradeitem[d].Name );
	StrCat( SBuf, ", drifts" );
	setLabelText( frm, PickCannisterCannisterLabel, SBuf );

	ret = FrmDoDialog( frm );
	FrmDeleteForm( frm ); 

	if (ret == PickCannisterPickItUpButton)
	{
		if (FilledCargoBays() >= TotalCargoBays())
		{
			if (FrmAlert( NoRoomToScoopAlert ) == NoRoomToScoopLetitgo)
				return;

			frm = FrmInitForm( DumpCargoForm );
			FrmSetEventHandler( frm, DiscardCargoFormHandleEvent );
			FrmDoDialog( frm );
			FrmDeleteForm( frm ); 
		}

		if (FilledCargoBays() < TotalCargoBays())
			++Ship.Cargo[d];
		else
			FrmAlert( NoDumpNoScoopAlert );
	}

}

// *************************************************************************
// Calculate total possible shield strength
// *************************************************************************
static Int32 TotalShields( SHIP* Sh )
{
	int i;
	Int32 j;

	j = 0;
	for (i=0; i<MAXSHIELD; ++i)
	{
		if (Sh->Shield[i] < 0)
			break;
		j += Shieldtype[Sh->Shield[i]].Power;
	}

	return j;
}


// *************************************************************************
// Calculate total shield strength
// *************************************************************************
static Int32 TotalShieldStrength( SHIP* Sh )
{
	int i;
	Int32 k;

	k = 0;
	for (i=0; i<MAXSHIELD; ++i)
	{
		if (Sh->Shield[i] < 0)
			break;
		k += Sh->ShieldStrength[i];
	}

	return k;
}


// *************************************************************************
// Show the ship stats on the encounter screen
// *************************************************************************
void ShowShip( SHIP* Sh, int offset, Boolean commandersShip )
{
	int x, y, x2, y2, startdamage, startshield, icon;
	RectangleType bounds;

	FntSetFont( stdFont );
	
	if (TextualEncounters)
	{
		EraseRectangle( offset, 32, 70, 38 );
	
		DrawChars( Shiptype[Sh->Type].Name, offset, 32 );
		
		if (ENCOUNTERMONSTER( EncounterType ) && offset > 10)
			StrCopy( SBuf, "Hide at " );
		else
			StrCopy( SBuf, "Hull at " );
	
		if (commandersShip)
			StrIToA( SBuf2, ((Sh->Hull * 100) / GetHullStrength()) );			
		else
			StrIToA( SBuf2, ((Sh->Hull * 100) / Shiptype[Sh->Type].HullStrength) );
		StrCat( SBuf, SBuf2 );
		StrCat( SBuf, "%" );
		DrawChars( SBuf, offset, 45 );

		if (Sh->Shield[0] < 0)
			StrCopy( SBuf, "No shields" );
		else
		{
			StrCopy( SBuf, "Shields at " );
			if (TotalShields( Sh ) > 0)
				StrIToA( SBuf2, ((TotalShieldStrength( Sh ) * 100) / TotalShields( Sh )) );
			else
				StrCopy( SBuf2, "0" );
			StrCat( SBuf, SBuf2 );
			StrCat( SBuf, "%" );
		}
		DrawChars( SBuf, offset, 58 );
	}
	else
	{
		EraseRectangle( offset, 18, 70, 52 );

		x = offset + (((64 - (GetBitmapWidth( ShipBmpPtr[Sh->Type] )))>>1));
		y = 18 + (((52 - (GetBitmapHeight( ShipBmpPtr[Sh->Type] )))>>1));

		x2 = offset + (((64 - (GetBitmapWidth( ShieldedShipBmpPtr[Sh->Type] )))>>1));
		y2 = 18 + (((52 - (GetBitmapHeight( ShieldedShipBmpPtr[Sh->Type] )))>>1));

		if (commandersShip)
		{
			startdamage = x + GetBitmapWidth( ShipBmpPtr[Sh->Type] ) - 
				((Sh->Hull * GetBitmapWidth( ShipBmpPtr[Sh->Type] )) / GetHullStrength());
		}
		else
		{
			startdamage = x + GetBitmapWidth( ShipBmpPtr[Sh->Type] ) - 
				((Sh->Hull * GetBitmapWidth( ShipBmpPtr[Sh->Type] )) / Shiptype[Sh->Type].HullStrength);
		}
		startshield = x2 + GetBitmapWidth( ShieldedShipBmpPtr[Sh->Type] ) -
			(Sh->Shield[0] < 0 ? 0 : 
			(TotalShieldStrength( Sh ) * GetBitmapWidth( ShieldedShipBmpPtr[Sh->Type] )) / TotalShields( Sh ) );

		bounds.topLeft.x = x2;
		bounds.topLeft.y = 18;
		bounds.extent.x = min( startdamage, startshield ) - x2;
		bounds.extent.y = 52;
		WinSetClip( &bounds );
		WinDrawBitmap( DamagedShipBmpPtr[Sh->Type], x, y );
		WinResetClip();

		bounds.topLeft.x = min( startdamage, startshield );
		bounds.extent.x = max( startdamage, startshield ) - x2;
		WinSetClip( &bounds );
		if (startdamage < startshield)
			WinDrawBitmap( ShipBmpPtr[Sh->Type], x, y );
		else
			WinDrawBitmap( DamagedShieldedShipBmpPtr[Sh->Type], x2, y2 );
		WinResetClip();

		bounds.topLeft.x = max( startdamage, startshield );
		bounds.extent.x = GetBitmapWidth( ShieldedShipBmpPtr[Sh->Type] );
		WinSetClip( &bounds );
		WinDrawBitmap( ShieldedShipBmpPtr[Sh->Type], x2, y2 );
		WinResetClip();
		
		if (offset > 10)
		{
			icon = -1;
			if (ENCOUNTERPIRATE( EncounterType ))
			{
				if (Sh->Type == MANTISTYPE)
					icon = 3;
				else
					icon = 0;
			}
			else if (ENCOUNTERPOLICE( EncounterType ))
				icon = 1;
			else if (ENCOUNTERTRADER( EncounterType ))
				icon = 2;
			else if (EncounterType == CAPTAINHUIEENCOUNTER ||
				EncounterType == MARIECELESTEENCOUNTER ||
				EncounterType == CAPTAINAHABENCOUNTER ||
				EncounterType == CAPTAINCONRADENCOUNTER)
				icon = 4;
			if (icon >= 0)
				WinDrawBitmap( IconBmpPtr[icon], 143, 13 );
		}
	}
}


// *************************************************************************
// Display on the encounter screen the ships (and also wipe it)
// *************************************************************************
static void EncounterDisplayShips( void )
{

    if (opponentShipNeedsUpdate)
    {
    	opponentShipNeedsUpdate = false;
		ShowShip( &Opponent, (TextualEncounters ? 84 : 80), false );
    }
    if (playerShipNeedsUpdate)
    {
    	playerShipNeedsUpdate = false;
		ShowShip( &Ship, 6, true );
    }

	EraseRectangle( 5, 75, 151, 64 );



}


// *************************************************************************
// Your escape pod ejects you
// *************************************************************************
void EscapeWithPod( void )
{
	FrmAlert( EscapePodActivatedAlert );

	if (ScarabStatus == 3)
		ScarabStatus = 0;

	Arrival();

	if (ReactorStatus > 0 && ReactorStatus < 21)
	{
		FrmAlert( ReactorDestroyedAlert );
		ReactorStatus = 0;
	}

	if (JaporiDiseaseStatus == 1)
	{
		FrmAlert( AntidoteDestroyedAlert );
		JaporiDiseaseStatus = 0;
	}
	
	if (ArtifactOnBoard)
	{
		FrmAlert( ArtifactNotSavedAlert );
		ArtifactOnBoard = false;
	}

	if (JarekStatus == 1)
	{
		FrmAlert( JarekTakenHomeAlert );
		JarekStatus = 0;
	}

	if (WildStatus == 1)
	{
		FrmAlert( WildArrestedAlert );
		PoliceRecordScore += CAUGHTWITHWILDSCORE;
		addNewsEvent(WILDARRESTED);
		WildStatus = 0;
	}
	
	if (Ship.Tribbles > 0)
	{
		FrmAlert( TribbleSurvivedAlert );
		Ship.Tribbles = 0;
	}
	
	if (Insurance)
	{
		FrmAlert( InsurancePaysAlert );
		Credits += CurrentShipPriceWithoutCargo( true );
	}

	FrmAlert( FleaBuiltAlert );
	
	if (Credits > 500)
		Credits -= 500;
	else
	{
		Debt += (500 - Credits);
		Credits = 0;
	}

	IncDays( 3 );	

	CreateFlea();

	CurForm = SystemInformationForm;
	FrmGotoForm( CurForm );
}


// *************************************************************************
// You get arrested
// *************************************************************************
static void Arrested( void )
{
	FormPtr frm;
	Int32 Fine, Imprisonment;
	int i;
	
	Fine = ((1 + (((CurrentWorth() * min( 80, -PoliceRecordScore )) / 100) / 500)) * 500);
	if (WildStatus == 1)
	{
		Fine *= 1.05;
	}
	Imprisonment = max( 30, -PoliceRecordScore );

	FrmAlert( ArrestedAlert );

	frm = FrmInitForm( ConvictionForm );
	
	StrCopy( SBuf, "You are convicted to " );
	StrIToA( SBuf2, Imprisonment );
	StrCat( SBuf, SBuf2 );
	StrCat( SBuf, " days in" );
	setLabelText( frm, ConvictionImprisonmentLabel, SBuf );
	
	StrCopy( SBuf, "prison and a fine of " );
	StrIToA( SBuf2, Fine );
	StrCat( SBuf, SBuf2 );
	StrCat( SBuf, " credits." );
	setLabelText( frm, ConvictionFineLabel, SBuf );
	
	FrmDoDialog( frm );
	FrmDeleteForm( frm );

	if (Ship.Cargo[NARCOTICS] > 0 || Ship.Cargo[FIREARMS] > 0)
	{
		FrmAlert( ImpoundAlert );
		Ship.Cargo[NARCOTICS] = 0;
		Ship.Cargo[FIREARMS] = 0;
	}

	if (Insurance)
	{
		FrmAlert( InsuranceLostAlert );
		Insurance = false;
		NoClaim = 0;
	}

	if (Ship.Crew[1] >= 0)
	{
		FrmAlert( MercenariesLeaveAlert );
		for (i=1; i<MAXCREW; ++i)
			Ship.Crew[i] = -1;
	}

	if (JaporiDiseaseStatus == 1)
	{
		FrmAlert( AntidoteRemovedAlert );
		JaporiDiseaseStatus = 2;
	}

	if (JarekStatus == 1)
	{
		FrmAlert( JarekTakenHomeAlert );
		JarekStatus = 0;
	}

	if (WildStatus == 1)
	{
		FrmAlert( WildArrestedAlert );
		addNewsEvent(WILDARRESTED);
		WildStatus = 0;
	}

	if (ReactorStatus > 0 && ReactorStatus < 21)
	{
		FrmAlert( PoliceConfiscateReactorAlert );
		ReactorStatus = 0; 
	}
	
	Arrival();

	IncDays( Imprisonment );

	if (Credits >= Fine)
		Credits -= Fine;
	else
	{
		Credits += CurrentShipPrice( true );

		if (Credits >= Fine)
			Credits -= Fine;
		else
			Credits = 0;

		FrmAlert( ShipSoldAlert );

		if (Ship.Tribbles > 0)
		{
			FrmAlert( TribblesSoldAlert );
			Ship.Tribbles = 0;
		}
	
		FrmAlert( FleaReceivedAlert );

		CreateFlea();
	}
	
	PoliceRecordScore = DUBIOUSSCORE;

	if (Debt > 0)
	{
		if (Credits >= Debt)
		{
			Credits -= Debt;
			Debt = 0;
		}
		else
		{
			Debt -= Credits;
			Credits = 0;
		}
	}
	
	for (i=0; i<Imprisonment; ++i)
		PayInterest();

	CurForm = SystemInformationForm;
	FrmGotoForm( CurForm );
}


// *************************************************************************
// An attack: Attacker attacks Defender, Flees indicates if Defender is fleeing
// *************************************************************************
static Boolean ExecuteAttack( SHIP* Attacker, SHIP* Defender, Boolean Flees, Boolean CommanderUnderAttack )
{
	Int32 Damage, prevDamage;
	int i;

	// On beginner level, if you flee, you will escape unharmed.
	if (Difficulty == BEGINNER && CommanderUnderAttack && Flees)
		return false;

	// Fighterskill attacker is pitted against pilotskill defender; if defender
	// is fleeing the attacker has a free shot, but the chance to hit is smaller
	if (GetRandom( FighterSkill( Attacker ) + Shiptype[Defender->Type].Size ) < 
		(Flees ? 2 : 1) * GetRandom( 5 + (PilotSkill( Defender ) >> 1) ))
		// Misses
		return false;

	if (TotalWeapons( Attacker, -1, -1) <= 0)
		Damage = 0L;
	else if (Defender->Type == SCARABTYPE)
	{
		if (TotalWeapons( Attacker, PULSELASERWEAPON, PULSELASERWEAPON ) <= 0 &&
		    TotalWeapons( Attacker, MORGANLASERWEAPON, MORGANLASERWEAPON ) <= 0)
			Damage = 0L;
		else
			Damage =  GetRandom( ((TotalWeapons( Attacker, PULSELASERWEAPON, PULSELASERWEAPON ) +
			   TotalWeapons( Attacker, MORGANLASERWEAPON, MORGANLASERWEAPON )) * (100 + 2*EngineerSkill( Attacker )) / 100) );
	}
	else
		Damage = GetRandom( (TotalWeapons( Attacker, -1, -1 ) * (100 + 2*EngineerSkill( Attacker )) / 100) );

	if (Damage <= 0L)
		return false;

	// Reactor on board -- damage is boosted!
	if (CommanderUnderAttack && ReactorStatus > 0 && ReactorStatus < 21)
	{
		if (Difficulty < NORMAL)
			Damage *= 1 + (Difficulty + 1)*0.25;
		else
			Damage *= 1 + (Difficulty + 1)*0.33;
	}
	
	// First, shields are depleted
	for (i=0; i<MAXSHIELD; ++i)
	{
		if (Defender->Shield[i] < 0)
			break;
		if (Damage <= Defender->ShieldStrength[i])
		{
			Defender->ShieldStrength[i] -= Damage;
			Damage = 0;
			break;
		}
		Damage -= Defender->ShieldStrength[i];
		Defender->ShieldStrength[i] = 0;
	}

	prevDamage = Damage;
	
	// If there still is damage after the shields have been depleted, 
	// this is subtracted from the hull, modified by the engineering skill
	// of the defender.
	if (Damage > 0)
	{
		Damage -= GetRandom( EngineerSkill( Defender ) );
		if (Damage <= 0)
			Damage = 1;
		// At least 2 shots on Normal level are needed to destroy the hull 
		// (3 on Easy, 4 on Beginner, 1 on Hard or Impossible). For opponents,
		// it is always 2.
		if (CommanderUnderAttack && ScarabStatus == 3)
			Damage = min( Damage, (GetHullStrength()/
				(CommanderUnderAttack ? max( 1, (IMPOSSIBLE-Difficulty) ) : 2)) );
		else
			Damage = min( Damage, (Shiptype[Defender->Type].HullStrength/
				(CommanderUnderAttack ? max( 1, (IMPOSSIBLE-Difficulty) ) : 2)) );
		Defender->Hull -= Damage;
		if (Defender->Hull < 0)
			Defender->Hull = 0;
	}

	if (Damage != prevDamage)
	{
		if (CommanderUnderAttack)
		{
			playerShipNeedsUpdate = true;
		}
		else
		{
			opponentShipNeedsUpdate = true;
		}
	}

	return true;
}


// *************************************************************************
// A fight round
// Return value indicates whether fight continues into another round
// *************************************************************************
static int ExecuteAction( Boolean CommanderFlees )
{
	FormPtr frmP;
	Boolean CommanderGotHit, OpponentGotHit;
	Int32 OpponentHull, ShipHull;
	int y, i, objindex;
	int PrevEncounterType;
	ControlPtr cp;

	CommanderGotHit = false;
	OpponentHull = Opponent.Hull;
	ShipHull = Ship.Hull;
	
	// Fire shots
	if (EncounterType == PIRATEATTACK || EncounterType == POLICEATTACK ||
		EncounterType == TRADERATTACK || EncounterType == SPACEMONSTERATTACK ||
		EncounterType == DRAGONFLYATTACK || EncounterType == POSTMARIEPOLICEENCOUNTER ||
		EncounterType == SCARABATTACK || EncounterType == FAMOUSCAPATTACK)
	{
		CommanderGotHit = ExecuteAttack( &Opponent, &Ship, CommanderFlees, true );
	}

	OpponentGotHit = false;
	
	if (!CommanderFlees)
	{
		if (EncounterType == POLICEFLEE || EncounterType == TRADERFLEE ||
			EncounterType == PIRATEFLEE)	
		{
			OpponentGotHit = ExecuteAttack( &Ship, &Opponent, true, false );
		}
		else
		{
			OpponentGotHit = ExecuteAttack( &Ship, &Opponent, false, false );
		}
	}

	if (CommanderGotHit)
	{
		playerShipNeedsUpdate = true;
	}
	if (OpponentGotHit)
	{
		 opponentShipNeedsUpdate = true;
	}

	// Determine whether someone gets destroyed
	if (Ship.Hull <= 0 && Opponent.Hull <= 0)
	{
		AutoAttack = false;
		AutoFlee = false;
	
		if (EscapePod)
		{
			EscapeWithPod();
			return( true );
		}
		else
		{
			FrmAlert( BothDestroyedAlert );
			CurForm = DestroyedForm;
			FrmGotoForm( CurForm );
		}
		return false;
	}
	else if (Opponent.Hull <= 0)
	{
		AutoAttack = false;
		AutoFlee = false;
				
		if (ENCOUNTERPIRATE( EncounterType ) && Opponent.Type != MANTISTYPE && PoliceRecordScore >= DUBIOUSSCORE)
		{
				
			frmP = FrmInitForm( BountyForm );
			StrCopy( SBuf, "You earned a bounty of " );
//			StrIToA( SBuf2, Shiptype[Opponent.Type].Bounty );
			StrIToA( SBuf2, GetBounty( &Opponent ) );
			StrCat( SBuf, SBuf2 );
			StrCat( SBuf, " cr." );
			setLabelText( frmP, BountyBountyLabel, SBuf );
			FrmDoDialog( frmP );
			FrmDeleteForm( frmP );
		}
		else
		{
			FrmAlert( OpponentDestroyedAlert );
		}
		if (ENCOUNTERPOLICE( EncounterType ))
		{
			++PoliceKills;
			PoliceRecordScore += KILLPOLICESCORE;
		}
		else if (ENCOUNTERFAMOUS( EncounterType))
		{
			if (ReputationScore < DANGEROUSREP)
			{
				ReputationScore = DANGEROUSREP;
			}
			else
			{
				ReputationScore += 100;
			}
			// bump news flag from attacked to ship destroyed
			replaceNewsEvent(latestNewsEvent(), latestNewsEvent() + 10);
			
		}
		else if (ENCOUNTERPIRATE( EncounterType ))
		{
			if (Opponent.Type != MANTISTYPE)
			{
//				Credits += Shiptype[Opponent.Type].Bounty;
				Credits += GetBounty( &Opponent );
				PoliceRecordScore += KILLPIRATESCORE;
				Scoop();
			}
			++PirateKills;
		}
		else if (ENCOUNTERTRADER( EncounterType ))
		{
			++TraderKills;
			PoliceRecordScore += KILLTRADERSCORE;
			Scoop();
		}
		else if (ENCOUNTERMONSTER( EncounterType ))
		{
			++PirateKills;
			PoliceRecordScore += KILLPIRATESCORE;
			MonsterStatus = 2;
		}
		else if (ENCOUNTERDRAGONFLY( EncounterType ))
		{
			++PirateKills;
			PoliceRecordScore += KILLPIRATESCORE;
			DragonflyStatus = 5;
		}
		else if (ENCOUNTERSCARAB( EncounterType ))
		{
			++PirateKills;
			PoliceRecordScore += KILLPIRATESCORE;
			ScarabStatus = 2;
		}
		ReputationScore += 1 + (Opponent.Type>>1);
		return false;
	}
	else if (Ship.Hull <= 0)
	{
		AutoAttack = false;
		AutoFlee = false;
	
		if (EscapePod)
		{
			EscapeWithPod();
			return( true );
		}
		else
		{
			FrmAlert( ShipDestroyedAlert );
			CurForm = DestroyedForm;
			FrmGotoForm( CurForm );
		}
		return false;
	}
	
	// Determine whether someone gets away.
	if (CommanderFlees)
	{
		if (Difficulty == BEGINNER)
		{
			AutoAttack = false;
			AutoFlee = false;
	
			FrmAlert( YouEscapedAlert );
			if (ENCOUNTERMONSTER( EncounterType ))
				MonsterHull = Opponent.Hull;

			return false;
		}
		else if ((GetRandom( 7 ) + (PilotSkill( &Ship ) / 3)) * 2 >= 
			GetRandom( PilotSkill( &Opponent ) ) * (2 + Difficulty))
		{
			AutoAttack = false;
			AutoFlee = false;
			if (CommanderGotHit)
			{
				ShowShip( &Ship, 6, true );
				frmP = FrmGetActiveForm();
				for (i=0; i<TRIBBLESONSCREEN; ++i)
				{
					objindex = FrmGetObjectIndex( frmP, EncounterTribble0Button + i );
					cp = (ControlPtr)FrmGetObjectPtr( frmP, objindex );
					CtlDrawControl( cp );
				}
				FrmAlert( YouEscapedWithDamageAlert );
			}
			else
				FrmAlert( YouEscapedAlert );
			if (ENCOUNTERMONSTER( EncounterType ))
				MonsterHull = Opponent.Hull;
				
			return false;
		}
	}
	else if (EncounterType == POLICEFLEE || EncounterType == TRADERFLEE ||
		EncounterType == PIRATEFLEE || EncounterType == TRADERSURRENDER ||
		EncounterType == PIRATESURRENDER)	
	{
		if (GetRandom( PilotSkill( &Ship ) ) * 4 <= 
			GetRandom( (7 + (PilotSkill( &Opponent ) / 3))) * 2)
		{
			AutoAttack = false;
			AutoFlee = false;
			FrmAlert( OpponentEscapedAlert );
			return false;
		}
	}
	
	// Determine whether the opponent's actions must be changed
	PrevEncounterType = EncounterType;
	
	if (Opponent.Hull < OpponentHull)
	{
		if (ENCOUNTERPOLICE( EncounterType ))
		{
			if (Opponent.Hull < OpponentHull >> 1) {
				if (Ship.Hull < ShipHull >> 1)
				{
					if (GetRandom( 10 ) > 5)
						EncounterType = POLICEFLEE;
				}	
				else
					EncounterType = POLICEFLEE;
                        }
		}
		else if (EncounterType == POSTMARIEPOLICEENCOUNTER)
		{
			EncounterType = POLICEATTACK;
		}
		else if (ENCOUNTERPIRATE( EncounterType ))
		{
			if (Opponent.Hull < (OpponentHull * 2) / 3)
			{
				if (Ship.Hull < (ShipHull * 2) / 3)
				{
					if (GetRandom( 10 ) > 3)
						EncounterType = PIRATEFLEE;
				}
				else
				{
					EncounterType = PIRATEFLEE;
					if (GetRandom( 10 ) > 8 && Opponent.Type < MAXSHIPTYPE)
						EncounterType = PIRATESURRENDER;
				}
			}
		}
		else if (ENCOUNTERTRADER( EncounterType ))
		{
			if (Opponent.Hull < (OpponentHull * 2) / 3)
			{
				if (GetRandom( 10 ) > 3)
					EncounterType = TRADERSURRENDER;
				else
					EncounterType = TRADERFLEE;
			}
			else if (Opponent.Hull < (OpponentHull * 9) / 10)
			{
				if (Ship.Hull < (ShipHull * 2) / 3)
				{
					// If you get damaged a lot, the trader tends to keep shooting
					if (GetRandom( 10 ) > 7)
						EncounterType = TRADERFLEE;
				}
				else if (Ship.Hull < (ShipHull * 9) / 10)
				{
					if (GetRandom( 10 ) > 3)
						EncounterType = TRADERFLEE;
				}
				else
					EncounterType = TRADERFLEE;
			}
		}
	}

	if (PrevEncounterType != EncounterType)
	{
		if (!(AutoAttack &&
			(EncounterType == TRADERFLEE || EncounterType == PIRATEFLEE || EncounterType == POLICEFLEE)))
			AutoAttack = false;
		AutoFlee = false;
	}
	
	// Show new status
	frmP = FrmGetActiveForm();

	EncounterButtons();
	// FrmDrawForm( frmP );
	EncounterDisplayShips();

	if (ENCOUNTERPOLICE( PrevEncounterType ))
		StrCopy( SBuf2, "police ship" );
	else if (ENCOUNTERPIRATE( PrevEncounterType ))
	{
		if (Opponent.Type == MANTISTYPE)
			StrCopy( SBuf2, "alien ship" );
		else	
			StrCopy( SBuf2, "pirate ship" );
	}
	else if (ENCOUNTERTRADER( PrevEncounterType ))
		StrCopy( SBuf2, "trader ship" );
	else if (ENCOUNTERMONSTER( PrevEncounterType ))
		StrCopy( SBuf2, "monster" );
	else if (ENCOUNTERDRAGONFLY( PrevEncounterType ))
		StrCopy( SBuf2, "Dragonfly" );
	else if (ENCOUNTERSCARAB( PrevEncounterType ))
		StrCopy( SBuf2, "Scarab" );
	else if (ENCOUNTERFAMOUS( PrevEncounterType))
		StrCopy( SBuf2, "Captain");
	
	y = 75;

	if (CommanderGotHit)
	{
		StrCopy( SBuf, "The " );
		StrCat( SBuf, SBuf2 );
		StrCat( SBuf, " hits you." );
		DrawChars( SBuf, 6, y );			
		y += 13;
	}

	if (!(PrevEncounterType == POLICEFLEE || PrevEncounterType == TRADERFLEE ||
		PrevEncounterType == PIRATEFLEE) && !CommanderGotHit)
	{
		StrCopy( SBuf, "The " );
		StrCat( SBuf, SBuf2 );
		StrCat( SBuf, " missed you." );
		DrawChars( SBuf, 6, y );			
		y += 13;
	}

	if (OpponentGotHit)
	{
		StrCopy( SBuf, "You hit the " );
		StrCat( SBuf, SBuf2 );
		StrCat( SBuf, "." );
		DrawChars( SBuf, 6, y );			
		y += 13;
	}

	if (!CommanderFlees && !OpponentGotHit)
	{
		StrCopy( SBuf, "You missed the " );
		StrCat( SBuf, SBuf2 );
		StrCat( SBuf, "." );
		DrawChars( SBuf, 6, y );			
		y += 13;
	}
	
	if (PrevEncounterType == POLICEFLEE || PrevEncounterType == TRADERFLEE ||
		PrevEncounterType == PIRATEFLEE)	
	{
		StrCopy( SBuf, "The " );
		StrCat( SBuf, SBuf2 );
		StrCat( SBuf, " didn't get away." );
		DrawChars( SBuf, 6, y );			
		y += 13;
	}

	if (CommanderFlees)
	{
		StrCopy( SBuf, "The " );
		StrCat( SBuf, SBuf2 );
		StrCat( SBuf, " is still following you." );
		DrawChars( SBuf, 6, y );			
		y += 13;
	}
	
	EncounterDisplayNextAction( false );

	for (i=0; i<TRIBBLESONSCREEN; ++i)
	{
		objindex = FrmGetObjectIndex( frmP, EncounterTribble0Button + i );
		cp = (ControlPtr)FrmGetObjectPtr( frmP, objindex );
		CtlDrawControl( cp );
	}

	return true;
}


// *************************************************************************
// Calculate total possible weapon strength
// Modified to allow an upper and lower limit on which weapons work.
// Weapons equal to or between minWeapon and maxWeapon (e.g., PULSELASERWEAPON)
// will do damage. Use -1 to allow damage from any weapon, which is almost
// always what you want. SjG
// *************************************************************************
Int32 TotalWeapons( SHIP* Sh, int minWeapon, int maxWeapon ) 
{
    int i;
    Int32 j;

    j = 0;
    for (i=0; i<MAXWEAPON; ++i)
    {
	if (Sh->Weapon[i] < 0)
	        break;
	        
	if ((minWeapon != -1 && Sh->Weapon[i] < minWeapon) ||
	    (maxWeapon != -1 && Sh->Weapon[i] > maxWeapon))
		continue;
	        
	j += Weapontype[Sh->Weapon[i]].Power;
    }
    
    return j;
}


static void DrawEncounterForm()
{
	FormPtr frmP;
 	int d, i;
	int objindex;
	ControlPtr cp;

	frmP = FrmGetActiveForm();

	EncounterButtons();
	FrmDrawForm( frmP );

	// This is an ugly hack. It seems to fix a PalmOS 3.0 bug.
	if (BELOW35)
		EncounterButtons();

	playerShipNeedsUpdate=true;
	opponentShipNeedsUpdate=true;

	EncounterDisplayShips();
	EncounterDisplayNextAction( true );

	if (EncounterType == POSTMARIEPOLICEENCOUNTER)
	{
		DrawChars( "You encounter the Customs Police.", 6, 75 );
	}
	else
	{
		StrCopy( SBuf, "At " );
		SBufMultiples( Clicks, "click" );
		StrCat( SBuf, " from " );
		StrCat( SBuf, SolarSystemName[SolarSystem[WarpSystem].NameIndex] );
		StrCat( SBuf, ", you" );
		DrawChars( SBuf, 6, 75 );			

		StrCopy( SBuf, "encounter " );
		
		if (ENCOUNTERPOLICE( EncounterType ))
			StrCat( SBuf, "a police " );
		else if (ENCOUNTERPIRATE( EncounterType ))
		{
			if (Opponent.Type == MANTISTYPE)
				StrCat( SBuf, "an alien " );
			else
				StrCat( SBuf, "a pirate " );
		}
		else if (ENCOUNTERTRADER( EncounterType ))
			StrCat( SBuf, "a trader " );
		else if (ENCOUNTERMONSTER( EncounterType ))
			StrCat( SBuf, " " );
		else if (EncounterType == MARIECELESTEENCOUNTER)
			StrCat(SBuf,"a drifting ship");
		else if (EncounterType == CAPTAINAHABENCOUNTER)
			StrCat(SBuf, "the famous Captain Ahab");
		else if (EncounterType == CAPTAINCONRADENCOUNTER)
			StrCat(SBuf, "Captain Conrad");
		else if (EncounterType == CAPTAINHUIEENCOUNTER)
			StrCat(SBuf, "Captain Huie");
		else if (EncounterType == BOTTLEOLDENCOUNTER || EncounterType == BOTTLEGOODENCOUNTER)
			StrCat(SBuf, "a floating bottle.");
		else
			StrCat( SBuf, "a stolen " );
		if (EncounterType != MARIECELESTEENCOUNTER && EncounterType != CAPTAINAHABENCOUNTER &&
			EncounterType != CAPTAINCONRADENCOUNTER && EncounterType != CAPTAINHUIEENCOUNTER &&
			EncounterType != BOTTLEOLDENCOUNTER && EncounterType != BOTTLEGOODENCOUNTER)
		{	
			StrCopy( SBuf2, Shiptype[Opponent.Type].Name );
			SBuf2[0] = TOLOWER( SBuf2[0] );
			StrCat( SBuf, SBuf2 );
		}
		StrCat( SBuf, "." );
		
		DrawChars( SBuf, 6, 88 );
	}			

	d = my_sqrt( Ship.Tribbles/250 );
	for (i=0; i<d; ++i)
	{
		objindex = FrmGetObjectIndex( frmP, EncounterTribble0Button +
			GetRandom( TRIBBLESONSCREEN ) );
		cp = (ControlPtr)FrmGetObjectPtr( frmP, objindex );
		CtlShowControl( cp );
	}


}

// *************************************************************************
// Encounter screen Event Handler
// *************************************************************************
Boolean EncounterFormHandleEvent ( EventPtr eventP )
{
    Boolean handled = false;
	FormPtr frmP, frm;
	Int32 Fine;
	Int32 Bribe;
 	int d, i, j, m, Bays, TotalCargo;
	Handle QuantityH;
	Int32 Blackmail;
	Boolean RedrawButtons = false;
#ifdef HWATTACK
	EventType event;
#endif

	switch (eventP->eType) 
	{
		case frmOpenEvent:
		case frmUpdateEvent:
			DrawEncounterForm();
			handled = true;
			break;

#ifdef HWATTACK
		case keyDownEvent:
			if (! UseHWButtons)
				return false;
			MemSet(&event, sizeof(event), 0);
			event.eType = ctlSelectEvent;
			if (eventP->data.keyDown.chr == hard1Chr)
				event.data.ctlSelect.controlID = hwbutton[0];
			else if (eventP->data.keyDown.chr == hard2Chr)
				event.data.ctlSelect.controlID = hwbutton[1];
			else if (eventP->data.keyDown.chr == hard3Chr)
				event.data.ctlSelect.controlID = hwbutton[2];
			else if (eventP->data.keyDown.chr == hard4Chr)
				event.data.ctlSelect.controlID = hwbutton[3];
	     	if (event.data.ctlSelect.controlID > 0)
	     		EvtAddEventToQueue(&event);	
	     	else
	     		return true;
			handled = true;
			break;
#endif

		case nilEvent:
			if (!Continuous)
				return true;
			if (!(AutoAttack || AutoFlee))
				return true;
			// Don't put anything in between the nilEvent and the ctlSelectEvent; these two
			// run into each other!				
		case ctlSelectEvent:
			
			if (eventP->data.ctlSelect.controlID >= EncounterTribble0Button &&
				eventP->data.ctlSelect.controlID < EncounterTribble0Button + TRIBBLESONSCREEN)
			{
				if (AutoAttack || AutoFlee)
					RedrawButtons = true;
				AutoAttack = false;
				AutoFlee = false;
				if (RedrawButtons)
					EncounterButtons();
					
				FrmAlert( SqueekAlert );
				handled = true;
				break;
			}

			if (((eventP->eType == nilEvent) && AutoAttack) || 
				(eventP->data.ctlSelect.controlID == EncounterAttackButton)) // Attack
			{
				if (AutoAttack || AutoFlee)
					RedrawButtons = true;
				AutoAttack = false;
				AutoFlee = false;
				if (RedrawButtons)
					EncounterButtons();
					
				if (TotalWeapons( &Ship, -1, -1 ) <= 0)
				{
					FrmAlert( NoWeaponsAlert );
					return true;
				}
					
				if (EncounterType == POLICEINSPECTION && Ship.Cargo[FIREARMS] <= 0 &&
					Ship.Cargo[NARCOTICS] <= 0)
				{
					if (FrmAlert( SureToFleeOrBribeAlert ) == SureToFleeOrBribeOKIwont)
						return true;
				}
					
				if (ENCOUNTERPOLICE( EncounterType ) || EncounterType == POSTMARIEPOLICEENCOUNTER)
				{
					if (PoliceRecordScore > CRIMINALSCORE &&
						FrmAlert( AttackByAccidentAlert ) == AttackByAccidentNo)
							return true;
					if (PoliceRecordScore > CRIMINALSCORE)
						PoliceRecordScore = CRIMINALSCORE;
						
					PoliceRecordScore += ATTACKPOLICESCORE;
					
					if (EncounterType == POLICEIGNORE || EncounterType == POLICEINSPECTION ||
						EncounterType == POSTMARIEPOLICEENCOUNTER)
					{
						EncounterType = POLICEATTACK;
					}
				}
				else if (ENCOUNTERPIRATE( EncounterType ))
				{
					if (EncounterType == PIRATEIGNORE)
						EncounterType = PIRATEATTACK;
				}
				else if (ENCOUNTERTRADER( EncounterType ))
				{
					if (EncounterType == TRADERIGNORE || EncounterType == TRADERBUY ||
					    EncounterType == TRADERSELL)
					{
						if (PoliceRecordScore >= CLEANSCORE)
						{
							if (FrmAlert( AttackTraderAlert ) == AttackTraderNo)
								return true;
							PoliceRecordScore = DUBIOUSSCORE;
						}
						else
							PoliceRecordScore += ATTACKTRADERSCORE;
					}
					if (EncounterType != TRADERFLEE)
					{
						if (TotalWeapons( &Opponent, -1, -1 ) <= 0)
							EncounterType = TRADERFLEE;
						else if (GetRandom( ELITESCORE ) <= (ReputationScore * 10) / (1 + Opponent.Type))
							EncounterType = TRADERFLEE;
						else
							EncounterType = TRADERATTACK;
					}
				}
				else if (ENCOUNTERMONSTER( EncounterType ))
				{
					if (EncounterType == SPACEMONSTERIGNORE)
						EncounterType = SPACEMONSTERATTACK;
				}
				else if (ENCOUNTERDRAGONFLY( EncounterType ))
				{
					if (EncounterType == DRAGONFLYIGNORE)
						EncounterType = DRAGONFLYATTACK;
				}
				else if (ENCOUNTERSCARAB( EncounterType ))
				{
					if (EncounterType == SCARABIGNORE)
						EncounterType = SCARABATTACK;
				}
				else if (ENCOUNTERFAMOUS( EncounterType ))
				{
					if (EncounterType != FAMOUSCAPATTACK &&
					    FrmAlert( SureToAttackFamousAlert ) == SureToAttackFamousOKIWont)
						return true;
					if (PoliceRecordScore > VILLAINSCORE)
						PoliceRecordScore = VILLAINSCORE;
					PoliceRecordScore += ATTACKTRADERSCORE;
					if (EncounterType == CAPTAINHUIEENCOUNTER)
						addNewsEvent(CAPTAINHUIEATTACKED);
					else if (EncounterType == CAPTAINAHABENCOUNTER)
						addNewsEvent(CAPTAINAHABATTACKED);
					else if (EncounterType == CAPTAINCONRADENCOUNTER)
						addNewsEvent(CAPTAINCONRADATTACKED);

					EncounterType = FAMOUSCAPATTACK;
						
				}
				if (Continuous)
					AutoAttack = true;
				if (ExecuteAction( false ))
					return true;
				if (Ship.Hull <= 0)
					return true;
			}					
			else if (((eventP->eType == nilEvent) && AutoFlee) || 
				(eventP->data.ctlSelect.controlID == EncounterFleeButton)) // Flee
			{			
				if (AutoAttack || AutoFlee)
					RedrawButtons = true;
				AutoAttack = false;
				AutoFlee = false;
				if (RedrawButtons)
					EncounterButtons();
					
				if (EncounterType == POLICEINSPECTION && Ship.Cargo[FIREARMS] <= 0 &&
					Ship.Cargo[NARCOTICS] <= 0 && WildStatus != 1 && (ReactorStatus == 0 || ReactorStatus == 21))
				{
					if (FrmAlert( SureToFleeOrBribeAlert ) == SureToFleeOrBribeOKIwont)
						return true;
				}

				if (EncounterType == POLICEINSPECTION)
				{
					EncounterType = POLICEATTACK;
					if (PoliceRecordScore > DUBIOUSSCORE)
						PoliceRecordScore = DUBIOUSSCORE - (Difficulty < NORMAL ? 0 : 1);
					else
						PoliceRecordScore += FLEEFROMINSPECTION;
				}
				else if (EncounterType == POSTMARIEPOLICEENCOUNTER)
					{
						if (FrmAlert( SureToFleePostMarieAlert ) != SureToFleePostMarieOKIwont)
						{
							EncounterType = POLICEATTACK;
							if (PoliceRecordScore >= CRIMINALSCORE)
								PoliceRecordScore = CRIMINALSCORE;
							else
								PoliceRecordScore += ATTACKPOLICESCORE;
						}
						else
						{
						return true;
						}
					}

				if (Continuous)
					AutoFlee = true;
				if (ExecuteAction( true ))
					return true;
				if (Ship.Hull <= 0)
					return true;
			}
			else if (eventP->data.ctlSelect.controlID == EncounterIgnoreButton) // Ignore
			{			
				// Only occurs when opponent either ignores you or flees, so just continue
				if (AutoAttack || AutoFlee)
					RedrawButtons = true;
				AutoAttack = false;
				AutoFlee = false;
				if (RedrawButtons)
					EncounterButtons();
			}					
			else if (eventP->data.ctlSelect.controlID == EncounterTradeButton) // Trade in Orbit
			{			
				if (EncounterType == TRADERBUY)
				{				
					i = GetRandomTradeableItem (&Ship, TRADERBUY);
					
					if (i == NARCOTICS || i == FIREARMS)
					{
						if (GetRandom(100) <= 45)
							SellPrice[i] *= 0.8;
						else
							SellPrice[i] *= 1.1;
					}
					else
					{
						if (GetRandom(100) <= 10)
							SellPrice[i] *= 0.9;
						else
							SellPrice[i] *= 1.1;
					}
				
					SellPrice[i] /= Tradeitem[i].RoundOff;
					++SellPrice[i];
					SellPrice[i] *= Tradeitem[i].RoundOff;
					if (SellPrice[i] < Tradeitem[i].MinTradePrice)
						SellPrice[i] = Tradeitem[i].MinTradePrice;
					if (SellPrice[i] > Tradeitem[i].MaxTradePrice)
						SellPrice[i] = Tradeitem[i].MaxTradePrice;
				
					frm = FrmInitForm( TradeInOrbitForm );

					StrCopy( SBuf, "The trader wants to buy ");
					StrCat ( SBuf, Tradeitem[i].Name);
					StrCat ( SBuf, ",");
					setLabelText( frm, TradeInOrbitDealLabel, SBuf );
					
					
					StrCopy( SBuf, "and offers ");
					StrIToA( SBuf2, SellPrice[i]);
					StrCat ( SBuf, SBuf2);
					StrCat ( SBuf, " cr. each.");
					setLabelText( frm, TradeInOrbitNumberItemsLabel, SBuf);
					
					StrCopy ( SBuf, "You have ");
					StrIToA ( SBuf2, Ship.Cargo[i]);
					StrCat ( SBuf, SBuf2);
					if (Ship.Cargo[i] > 1)
						StrCat ( SBuf, " units");
					else
						StrCat (SBuf, " unit");
					StrCat( SBuf, " available.");
					setLabelText( frm, TradeInOrbitQuantityAvailableLabel, SBuf );
					
					StrCopy( SBuf, "You paid about " );
					StrIToA( SBuf2, BuyingPrice[i] / Ship.Cargo[i] );
					StrCat( SBuf, SBuf2 );
					StrCat( SBuf, " cr. per unit." );
					setLabelText( frm, TradeInOrbitAveragePriceLabel, SBuf );
					setLabelText( frm, TradeInOrbitHowManyLabel, "How many do you wish to sell?");

					QuantityH = (Handle) SetField( frm, TradeInOrbitQuantityField, "", 5, true );
					d = FrmDoDialog(frm);
					j = 0;
					if (d == TradeInOrbitOKButton)
					{
						GetField( frm, TradeInOrbitQuantityField, SBuf, QuantityH );
						if (SBuf[0] != '\0')
						{
							j = max(0, min(Ship.Cargo[i], StrAToI(SBuf)));
						}
					}
					else if (d == TradeInOrbitAllButton)
					{
						j = Ship.Cargo[i];
					}
					j = min( j, Shiptype[Opponent.Type].CargoBays );
					if (j > 0)
					{
						BuyingPrice[i] = BuyingPrice[i]*(Ship.Cargo[i]-j)/Ship.Cargo[i];
						Ship.Cargo[i] -= j;
						Opponent.Cargo[i] = j;
						Credits += j * SellPrice[i];
						FrmCustomAlert(OrbitTradeCompletedAlert,"Thanks for selling us the", Tradeitem[i].Name, " ");
					}
					
					FrmDeleteForm( frm );
				}
			else if (EncounterType == TRADERSELL)
				{				
					i = GetRandomTradeableItem (&Opponent, TRADERSELL);
					
					if (i == NARCOTICS || i == FIREARMS)
					{
						if (GetRandom(100) <= 45)
							BuyPrice[i] *= 1.1;
						else
							BuyPrice[i] *= 0.8;
					}
					else
					{
						if (GetRandom(100) <= 10)
							BuyPrice[i] *= 1.1;
						else
							BuyPrice[i] *= 0.9;
					}

					BuyPrice[i] /= Tradeitem[i].RoundOff;
					BuyPrice[i] *= Tradeitem[i].RoundOff;
					if (BuyPrice[i] < Tradeitem[i].MinTradePrice)
						BuyPrice[i] = Tradeitem[i].MinTradePrice;
					if (BuyPrice[i] > Tradeitem[i].MaxTradePrice)
						BuyPrice[i] = Tradeitem[i].MaxTradePrice;
	
					frm = FrmInitForm( TradeInOrbitForm );

					StrCopy( SBuf, "The trader wants to sell ");
					StrCat ( SBuf, Tradeitem[i].Name);
					setLabelText( frm, TradeInOrbitDealLabel, SBuf );
					
					StrCopy( SBuf, "for the price of ");
					StrIToA( SBuf2, BuyPrice[i]);
					StrCat ( SBuf, SBuf2);
					StrCat ( SBuf, " cr. each.");
					setLabelText( frm, TradeInOrbitNumberItemsLabel, SBuf);
					
					StrCopy ( SBuf, "The trader has ");
					StrIToA ( SBuf2, Opponent.Cargo[i]);
					StrCat ( SBuf, SBuf2);
					if (Opponent.Cargo[i] > 1)
						StrCat ( SBuf, " units");
					else
						StrCat (SBuf, " unit");
					StrCat ( SBuf, " for sale.");
					setLabelText( frm, TradeInOrbitQuantityAvailableLabel, SBuf );
					
					StrCopy( SBuf, "You can afford " );
					StrIToA( SBuf2, Credits / BuyPrice[i] );
					StrCat( SBuf, SBuf2 );
					if (Credits/BuyPrice[i] > 1)
						StrCat( SBuf, " units." );
					else
						StrCat( SBuf, " unit.");
					setLabelText( frm, TradeInOrbitAveragePriceLabel, SBuf );
					setLabelText( frm, TradeInOrbitHowManyLabel, "How many do you wish to buy?");

					QuantityH = (Handle) SetField( frm, TradeInOrbitQuantityField, "", 5, true );
					d = FrmDoDialog(frm);
					j = 0;
					if (d == TradeInOrbitOKButton)
					{
						GetField( frm, TradeInOrbitQuantityField, SBuf, QuantityH );
						if (SBuf[0] != '\0')
						{
							j = max(0, min(Opponent.Cargo[i], StrAToI(SBuf)));;
						}
					}
					else if (d == TradeInOrbitAllButton)
					{
						j = min(Opponent.Cargo[i], (TotalCargoBays()-FilledCargoBays()));
					}

					j = min ( j, (Credits / BuyPrice[i]));
					
					if (j > 0)
					{
						Ship.Cargo[i] += j;
						Opponent.Cargo[i] -= j;
						BuyingPrice[i] += (j * BuyPrice[i]);
						Credits -= (j * BuyPrice[i]);
						FrmCustomAlert(OrbitTradeCompletedAlert,"Thanks for buying the", Tradeitem[i].Name, " ");
					}
					
					FrmDeleteForm( frm );
				}
			}					
			else if (eventP->data.ctlSelect.controlID == EncounterYieldButton) // Yield Narcotics from Marie Celeste
			{	
				
			    if (WildStatus == 1)
			    {
					if (FrmCustomAlert( WantToSurrenderAlert, "You have Jonathan Wild on board! ", "Wild will be arrested, too. ", NULL ) == WantToSurrenderNo)
						return true;
				}
				else if (ReactorStatus > 0 && ReactorStatus < 21)
				{
					if (FrmCustomAlert( WantToSurrenderAlert, "You have an illegal Reactor on board! ", "They will destroy the reactor. ", NULL) == WantToSurrenderNo)
						return true;
				}
				
				if (WildStatus == 1 || (ReactorStatus > 0 && ReactorStatus < 21))
				{
					Arrested();
				}
				else
				{					
					// Police Record becomes dubious, if it wasn't already.
					if (PoliceRecordScore > DUBIOUSSCORE)
						PoliceRecordScore = DUBIOUSSCORE;
					Ship.Cargo[NARCOTICS]=0;
					Ship.Cargo[FIREARMS]=0;
					
					FrmAlert(YieldNarcoticsAlert);
				}
			}					
			else if (eventP->data.ctlSelect.controlID == EncounterSurrenderButton) // Surrender
			{
				if (AutoAttack || AutoFlee)
					RedrawButtons = true;
				AutoAttack = false;
				AutoFlee = false;
				if (RedrawButtons)
					EncounterButtons();
					
				if (Opponent.Type == MANTISTYPE)
				{
					if (ArtifactOnBoard)
					{
						if (FrmAlert( WantToSurrenderToAliensAlert ) == WantToSurrenderToAliensNo)
							return true;
						else
						{
							FrmAlert( ArtifactStolenAlert );
							ArtifactOnBoard = 0;
						}
					}
					else
					{
						FrmAlert( NoSurrenderAlert );
						return true;
					}
				}
				else if (ENCOUNTERPOLICE( EncounterType ))
				{
					if (PoliceRecordScore <= PSYCHOPATHSCORE)
					{
						FrmAlert( NoSurrenderAlert );
						return true;
					}
					else
					{
					    if (WildStatus == 1)
					    {
							if (FrmCustomAlert( WantToSurrenderAlert, "You have Jonathan Wild on board! ", "Wild will be arrested, too. ", NULL ) == WantToSurrenderNo)
								return true;
						}
						else if (ReactorStatus > 0 && ReactorStatus < 21)
						{
							if (FrmCustomAlert( WantToSurrenderAlert, "You have an illegal Reactor on board! ", "They will destroy the reactor. ", NULL) == WantToSurrenderNo)
								return true;
						}
						else
						{
							if (FrmCustomAlert( WantToSurrenderAlert, NULL, NULL, NULL ) == WantToSurrenderNo)
								return true;
						}
					
						Arrested();
						return true;
					}
				}
				else
				{
					Raided = true;
					
					TotalCargo = 0;
					for (i=0; i<MAXTRADEITEM; ++i)
						TotalCargo += Ship.Cargo[i];
					if (TotalCargo <= 0)
					{
						Blackmail = min( 25000, max( 500, CurrentWorth() / 20 ) );
						FrmAlert( PiratesFindNoCargoAlert );
						if (Credits >= Blackmail)
							Credits -= Blackmail;
						else
						{
							Debt += (Blackmail - Credits);
							Credits = 0;
						}
					}		
					else
					{	
												
						FrmAlert( PiratesPlunderAlert );									
									
						Bays = Shiptype[Opponent.Type].CargoBays;
						for (i=0; i<MAXGADGET; ++i)
							if (Opponent.Gadget[i] == EXTRABAYS)
								Bays += 5;
						for (i=0; i<MAXTRADEITEM; ++i)
							Bays -= Opponent.Cargo[i];

						// Pirates steal everything					
						if (Bays >= TotalCargo)
						{
							for (i=0; i<MAXTRADEITEM; ++i)
							{
								Ship.Cargo[i] = 0;
								BuyingPrice[i] = 0;
							}
						}		
						else
						{		
							// Pirates steal a lot
							while (Bays > 0)
							{
								i = GetRandom( MAXTRADEITEM );
								if (Ship.Cargo[i] > 0)
								{
									BuyingPrice[i] = (BuyingPrice[i] * (Ship.Cargo[i] - 1)) / Ship.Cargo[i];
									--Ship.Cargo[i];
									--Bays;
								}
							}
						}
					}
					if ((WildStatus == 1) && (Shiptype[Opponent.Type].CrewQuarters > 1))
					{
						// Wild hops onto Pirate Ship
						WildStatus = 0;
						FrmAlert( WildGoesWithPiratesAlert );
					}
					else if (WildStatus == 1)
					{
						// no room on pirate ship
						FrmAlert( WildStaysAboardAlert );
					}
					if (ReactorStatus > 0 && ReactorStatus < 21)
					{
						// pirates puzzled by reactor
						FrmAlert( PiratesDontStealReactorAlert );
					}
				}
			}
			else if (eventP->data.ctlSelect.controlID == EncounterBribeButton) // Bribe
			{			
				if (AutoAttack || AutoFlee)
					RedrawButtons = true;
				AutoAttack = false;
				AutoFlee = false;
				if (RedrawButtons)
					EncounterButtons();
					
				if (Politics[SolarSystem[WarpSystem].Politics].BribeLevel <= 0)
				{
					FrmAlert( CantBeBribedAlert );
					return true;
				}
				if (EncounterType == POSTMARIEPOLICEENCOUNTER)
				{
					FrmAlert( MarieCantBeBribedAlert );
					return true;
				}
					
				if (EncounterType == POLICEINSPECTION && Ship.Cargo[FIREARMS] <= 0 &&
					Ship.Cargo[NARCOTICS] <= 0 && WildStatus != 1)
				{
					if (FrmAlert( SureToFleeOrBribeAlert ) == SureToFleeOrBribeOKIwont)
						return true;
				}

				// Bribe depends on how easy it is to bribe the police and commander's current worth
				Bribe = CurrentWorth() / 
					((10L + 5L * (IMPOSSIBLE - Difficulty)) * Politics[SolarSystem[WarpSystem].Politics].BribeLevel);
				if (Bribe % 100 != 0)
					Bribe += (100 - (Bribe % 100));
				if (WildStatus == 1 || (ReactorStatus > 0 && ReactorStatus < 21))
				{
					if (Difficulty <= NORMAL)
						Bribe *= 2;
					else
						Bribe *= 3;
				}
				Bribe = max( 100, min( Bribe, 10000 ) );
					
				frmP = FrmInitForm( BribeForm );

				StrCopy( SBuf, "of " );
				StrIToA( SBuf2, Bribe );
				StrCat( SBuf, SBuf2 );
				StrCat( SBuf, " credits." );
				setLabelText( frmP, BribeBribeLabel, SBuf );
	
				d = FrmDoDialog( frmP );

				FrmDeleteForm( frmP );

				if (d == BribeOfferBribeButton)
				{
					if (Credits < Bribe)
					{
						FrmAlert( NoMoneyForBribeAlert );
						return true;
					}
						
					Credits -= Bribe;
				}
				else
					return true;					
			}
			else if (eventP->data.ctlSelect.controlID == EncounterSubmitButton) // Submit
			{
				if (AutoAttack || AutoFlee)
					RedrawButtons = true;
				AutoAttack = false;
				AutoFlee = false;
				if (RedrawButtons)
					EncounterButtons();
					
				if (EncounterType == POLICEINSPECTION && (Ship.Cargo[FIREARMS] > 0 ||
					Ship.Cargo[NARCOTICS] > 0 || WildStatus == 1 ||
					(ReactorStatus > 1 && ReactorStatus < 21)))
				{
					if (WildStatus == 1)
					{
						if (Ship.Cargo[FIREARMS] > 0 || Ship.Cargo[NARCOTICS] > 0)
						{
							StrCopy( SBuf, "Jonathan Wild and illegal goods");
						}
						else
						{
							StrCopy( SBuf, "Jonathan Wild");
						}
						StrCopy(SBuf2, "You will be arrested!");
					}
					else if (ReactorStatus > 0 && ReactorStatus < 21)
					{
						if (Ship.Cargo[FIREARMS] > 0 || Ship.Cargo[NARCOTICS] > 0)
						{
							StrCopy( SBuf, "an illegal Ion Reactor and other illegal goods");
						}
						else
						{
							StrCopy( SBuf, "an illegal Ion Reactor");
						}
						StrCopy(SBuf2, "You will be arrested!");
					}
					else
					{
						StrCopy( SBuf, "illegal goods");
						StrCopy( SBuf2, " ");
					}
					if (FrmCustomAlert( SureToSubmitAlert, SBuf, SBuf2, NULL ) == SureToSubmitNo)
							return true;
					
				}

				if ((Ship.Cargo[FIREARMS] > 0) || (Ship.Cargo[NARCOTICS] > 0))
				{
					// If you carry illegal goods, they are impounded and you are fined
					Ship.Cargo[FIREARMS] = 0;
					BuyingPrice[FIREARMS] = 0;
					Ship.Cargo[NARCOTICS] = 0;
					BuyingPrice[NARCOTICS] = 0;
					Fine = CurrentWorth() / ((IMPOSSIBLE+2-Difficulty) * 10L);
					if (Fine % 50 != 0)
						Fine += (50 - (Fine % 50));
					Fine = max( 100, min( Fine, 10000 ) );
					if (Credits >= Fine)
						Credits -= Fine;
					else
					{
						Debt += (Fine - Credits);
						Credits = 0;
					}

					frmP = FrmInitForm( IllegalGoodsForm );

					StrIToA( SBuf, Fine );
					StrCat( SBuf, " credits." );
					setLabelText( frmP, IllegalGoodsFineLabel, SBuf );
	
					FrmDoDialog( frmP );

					FrmDeleteForm( frmP );
						
					PoliceRecordScore += TRAFFICKING;
				}
				else if (WildStatus != 1)
				{
					// If you aren't carrying illegal goods, the police will increase your lawfulness record
					FrmAlert( NoIllegalGoodsAlert );
					PoliceRecordScore -= TRAFFICKING;
				}
				if (WildStatus == 1)
				{
					// Jonathan Wild Captured, and your status damaged.
					Arrested();
					return true;
				}
				if (ReactorStatus > 0 && ReactorStatus < 21)
				{
					// Police confiscate the Reactor.
					// Of course, this can only happen if somehow your
					// Police Score gets repaired while you have the
					// reactor on board -- otherwise you'll be arrested
					// before we get to this point. (no longer true - 25 August 2002)
					FrmAlert( PoliceConfiscateReactorAlert );
					ReactorStatus = 0;

				}
			}		
			else if (eventP->data.ctlSelect.controlID == EncounterPlunderButton) // Plunder
			{
				if (AutoAttack || AutoFlee)
					RedrawButtons = true;
				AutoAttack = false;
				AutoFlee = false;
				if (RedrawButtons)
					EncounterButtons();
					
				if (ENCOUNTERTRADER( EncounterType ))
					PoliceRecordScore += PLUNDERTRADERSCORE;
				else
					PoliceRecordScore += PLUNDERPIRATESCORE;
				CurForm = PlunderForm;
				FrmGotoForm( CurForm );
				return true;
			}
			else if (eventP->data.ctlSelect.controlID == EncounterInterruptButton) // Interrupt automatic attack/flight
			{
				if (AutoAttack || AutoFlee)
					RedrawButtons = true;
				AutoAttack = false;
				AutoFlee = false;
				if (RedrawButtons)
					EncounterButtons();
					
				return true;
			}
			else if (eventP->data.ctlSelect.controlID == EncounterMeetButton) // Meet with Famous Captain
			{
				if (EncounterType == CAPTAINAHABENCOUNTER)
				{
					// Trade a reflective shield for skill points in piloting?
					if (FrmAlert( EngageCaptainAhabAlert) == EngageCaptainAhabYesTradeShield)
					{
						// remove the last reflective shield
						i=MAXSHIELD - 1;
						while (i >= 0)
						{
							if (Ship.Shield[i] == REFLECTIVESHIELD)
							{
								for (m=i+1; m<MAXSHIELD; ++m)
								{
									Ship.Shield[m-1] = Ship.Shield[m];
									Ship.ShieldStrength[m-1] = Ship.ShieldStrength[m];
								}
								Ship.Shield[MAXSHIELD-1] = -1;
								Ship.ShieldStrength[MAXSHIELD-1] = 0;
								i = -1;
							}
							i--;
						}
						// add points to piloting skill
						// two points if you're on beginner-normal, one otherwise
						if (Difficulty < HARD)
							COMMANDER.Pilot += 2;
						else
							COMMANDER.Pilot += 1;
							
						if (COMMANDER.Pilot > MAXSKILL)
						{
							COMMANDER.Pilot = MAXSKILL;
						}
						FrmAlert( TrainingCompletedAlert );
					}
				}
				else if (EncounterType == CAPTAINCONRADENCOUNTER)
				{
					// Trade a military laser for skill points in engineering?
					if (FrmAlert( EngageCaptainConradAlert) == EngageCaptainConradYesTradeLaser)
					{
						// remove the last military laser
						i=MAXWEAPON - 1;
						while (i>=0)
						{
							if (Ship.Weapon[i] == MILITARYLASERWEAPON)
							{
								for (m=i+1; m<MAXWEAPON; ++m)
								{
									Ship.Weapon[m-1] = Ship.Weapon[m];
								}
								Ship.Weapon[MAXWEAPON-1] = -1;
								i = -1;
							}
							i--;
						}
						// add points to engineering skill
						// two points if you're on beginner-normal, one otherwise
						if (Difficulty < HARD)
							COMMANDER.Engineer += 2;
						else
							COMMANDER.Engineer += 1;
							
						if (COMMANDER.Engineer > MAXSKILL)
						{
							COMMANDER.Engineer = MAXSKILL;
						}
						FrmAlert( TrainingCompletedAlert );
					}
				}
				else if (EncounterType == CAPTAINHUIEENCOUNTER)
				{
					// Trade a military laser for skill points in trading?
					if (FrmAlert( EngageCaptainHuieAlert) == EngageCaptainHuieYesTradeLaser)
					{
						// remove the last military laser
						i=MAXWEAPON - 1;
						while (i>=0)
						{
							if (Ship.Weapon[i] == MILITARYLASERWEAPON)
							{
								for (m=i+1; m<MAXWEAPON; ++m)
								{
									Ship.Weapon[m-1] = Ship.Weapon[m];
								}
								Ship.Weapon[MAXWEAPON-1] = -1;
								i = -1;
							}
							i--;
						}
						// add points to trading skill
						// two points if you're on beginner-normal, one otherwise
						if (Difficulty < HARD)
							COMMANDER.Trader += 2;
						else
							COMMANDER.Trader += 1;
							
						if (COMMANDER.Trader > MAXSKILL)
						{
							COMMANDER.Trader = MAXSKILL;
						}
						RecalculateBuyPrices(COMMANDER.CurSystem);
						FrmAlert( TrainingCompletedAlert );
					}
				}
			}
			else if (eventP->data.ctlSelect.controlID == EncounterBoardButton) // Board Marie Celeste
			{
				if (EncounterType == MARIECELESTEENCOUNTER)
				{
					// take the cargo of the Marie Celeste?
					if (FrmAlert( EngageMarieAlert ) == EngageMarieYesTakeCargo)
					{
						CurForm = PlunderForm;
						FrmGotoForm( CurForm );
						return true;
					}
				}
			}		
			else if (eventP->data.ctlSelect.controlID == EncounterDrinkButton) // Drink Tonic?
			{
				if (EncounterType == BOTTLEGOODENCOUNTER)
				{
					// Quaff the good bottle of Skill Tonic?
					if (FrmAlert( EngageBottleAlert ) == EngageBottleYesDrinkIt)
					{
						// two points if you're on beginner-normal, one otherwise
						IncreaseRandomSkill();
						if (Difficulty < HARD)
							IncreaseRandomSkill();
						FrmAlert( GoodDrinkAlert );
					}
				}
				else if (EncounterType == BOTTLEOLDENCOUNTER)
				{
					// Quaff the out of date bottle of Skill Tonic?
					if (FrmAlert( EngageBottleAlert ) == EngageBottleYesDrinkIt)
					{
						TonicTweakRandomSkill();
						FrmAlert( StrangeDrinkAlert );
					}
				}

			}
			Travel();
			handled = true;
			break;
				
		default:
			break;
	}
	
	return handled;
}


