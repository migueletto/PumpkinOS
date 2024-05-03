/***********************************************************************
 *
 * SPACE TRADER 1.2.0
 *
 * SystemInfoEvent.c
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

static int NewsEvents[MAXSPECIALNEWSEVENTS];

// *************************************************************************
// Draw one mercenary
// *************************************************************************
static void DrawMercenary( int Index, int y )
{
	int j, d;

	DrawChars( MercenaryName[Mercenary[Index].NameIndex], 30, y );

	StrIToA( SBuf, MERCENARYHIREPRICE( Index ) );
	StrCat( SBuf, " cr." );
	j = MAXDIGITS - StrLen( SBuf );
	d = 101+j*5;
	StrCat( SBuf, " daily" );
	DrawChars( SBuf, d, y );
		
	StrCopy( SBuf, "Pilot: " );
	StrIToA( SBuf2, Mercenary[Index].Pilot );
	StrCat( SBuf, SBuf2 );
	DrawChars( SBuf, 30, y + 13 );
		
	StrCopy( SBuf, "Trader: " );
	StrIToA( SBuf2, Mercenary[Index].Trader );
	StrCat( SBuf, SBuf2 );
	DrawChars( SBuf, 30, y + 26 );
		
	StrCopy( SBuf, "Fighter: " );
	StrIToA( SBuf2, Mercenary[Index].Fighter );
	StrCat( SBuf, SBuf2 );
	DrawChars( SBuf, 80, y + 13 );
		
	StrCopy( SBuf, "Engineer: " );
	StrIToA( SBuf2, Mercenary[Index].Engineer );
	StrCat( SBuf, SBuf2 );
	DrawChars( SBuf, 80, y + 26 );
}

// *************************************************************************
// Return available crew quarters
// *************************************************************************
static char AvailableQuarters( void )
{
	return Shiptype[Ship.Type].CrewQuarters - (JarekStatus == 1 ? 1 : 0) -
		 (WildStatus == 1 ? 1 : 0);
}



// *************************************************************************
// Determine which mercenary is for hire in the current system
// *************************************************************************
static int GetForHire( void )
{
	int ForHire = -1;
	int i;
	
	for (i=1; i<MAXCREWMEMBER; ++i)
	{
		if (i == Ship.Crew[1] || i == Ship.Crew[2])
			continue;
		if (Mercenary[i].CurSystem == Mercenary[0].CurSystem)
		{
			ForHire = i;
			break;
		}
	}
	
	return ForHire;
}

// *************************************************************************
// Drawing the Personnel Roster screen
// *************************************************************************
void DrawPersonnelRoster( void )
{
	FormPtr frmP;
	RectangleType a;
	int i, ForHire;

	frmP = FrmGetActiveForm();

	RectangularShortcuts( frmP, PersonnelRosterBButton );

	for (i=0; i<3; ++i)
		RectangularButton( frmP, PersonnelRosterFire0Button + i );

	FrmDrawForm( frmP );

	EraseRectangle( 30, 18, 130, 142 );

	a.topLeft.x = 0;
	a.topLeft.y = BOUNDSY + 38;
	a.extent.x = 160;
	a.extent.y = 2;
	WinDrawRectangle( &a, 0 );
			
	a.topLeft.y = BOUNDSY + 83;
	WinDrawRectangle( &a, 0 );

	FntSetFont( stdFont );
	
	for (i=0; i<2; ++i)
	{
		if (i == Shiptype[Ship.Type].CrewQuarters-2 && (JarekStatus == 1 || WildStatus == 1))
		{
			if (JarekStatus == 1)
				DrawChars( "Jarek's quarters", 30, 30 + i*45 );
			else
				DrawChars( "Wild's quarters", 30, 30 + i*45 );
			FrmHideObject( frmP, FrmGetObjectIndex( frmP, PersonnelRosterFire0Button + i ) );
			continue;
		}
	
		if (Shiptype[Ship.Type].CrewQuarters <= i+1)
		{
			DrawChars( "No quarters available", 30, 30 + i*45 );
			FrmHideObject( frmP, FrmGetObjectIndex( frmP, PersonnelRosterFire0Button + i ) );
			continue;
		}
		
		if (Ship.Crew[i+1] < 0)
		{
			DrawChars( "Vacancy", 30, 30 + i*45 );
			FrmHideObject( frmP, FrmGetObjectIndex( frmP, PersonnelRosterFire0Button + i ) );
			continue;
		}
			
		FrmShowObject( frmP, FrmGetObjectIndex( frmP, PersonnelRosterFire0Button + i ) );
		DrawMercenary( Ship.Crew[i+1], 17+i*45 );
	}

	ForHire = GetForHire();
	if (ForHire < 0)
	{
		DrawChars( "No one for hire", 30, 120 );
		FrmHideObject( frmP, FrmGetObjectIndex( frmP, PersonnelRosterHire0Button ) );
	}
	else
	{	
		FrmShowObject( frmP, FrmGetObjectIndex( frmP, PersonnelRosterHire0Button ) );
		DrawMercenary( ForHire, 107 );		
	}

	DisplayTradeCredits();
}


// *************************************************************************
// Add a news event flag
// *************************************************************************
void addNewsEvent(int eventFlag)
{
	if (NewsSpecialEventCount < MAXSPECIALNEWSEVENTS - 1)
		NewsEvents[NewsSpecialEventCount++] = eventFlag;
}


// *************************************************************************
// replace a news event flag with another
// *************************************************************************
void replaceNewsEvent(int originalEventFlag, int replacementEventFlag)
{
	int i;
	
	if (originalEventFlag == -1)
	{
		addNewsEvent(replacementEventFlag);
	}
	else
	{
		for (i=0;i<NewsSpecialEventCount; i++)
		{
			if (NewsEvents[i] == originalEventFlag)
				NewsEvents[i] = replacementEventFlag;
		}
	}
}

// *************************************************************************
// Reset news event flags
// *************************************************************************
void resetNewsEvents(void)
{
	NewsSpecialEventCount = 0;
}

// *************************************************************************
// get most recently addded news event flag
// *************************************************************************
int latestNewsEvent(void)
{
	if (NewsSpecialEventCount == 0)
		return -1;
	else
		return NewsEvents[NewsSpecialEventCount - 1];
}


// *************************************************************************
// Query news event flags
// *************************************************************************
Boolean isNewsEvent(int eventFlag)
{
	int i;

	for (i=0;i<NewsSpecialEventCount; i++)
	{
		if (NewsEvents[i] == eventFlag)
			return true;
	}
	return false;
}


static void DrawSystemInformationForm()
{
    FormPtr frmP;
    int OpenQ = OpenQuests();
	frmP = FrmGetActiveForm();
	setLabelText( frmP, SystemInformationSystemNameLabel, 
		SolarSystemName[CURSYSTEM.NameIndex] );
	setLabelText( frmP, SystemInformationTechLevelLabel, 
		TechLevel[CURSYSTEM.TechLevel] );
	setLabelText( frmP, SystemInformationGovernmentLabel, 
		Politics[CURSYSTEM.Politics].Name );
	setLabelText( frmP, SystemInformationResourcesLabel, 
		SpecialResources[CURSYSTEM.SpecialResources] );
	setLabelText( frmP, SystemInformationStatusLabel, 
		Status[CURSYSTEM.Status] );
	setLabelText( frmP, SystemInformationSizeLabel, 
		SystemSize[CURSYSTEM.Size] );
	setLabelText( frmP, SystemInformationPoliceLabel, 
		Activity[Politics[CURSYSTEM.Politics].StrengthPolice] );
	setLabelText( frmP, SystemInformationPiratesLabel, 
		Activity[Politics[CURSYSTEM.Politics].StrengthPirates] );
	if ((CURSYSTEM.Special < 0) || 
		(CURSYSTEM.Special == BUYTRIBBLE && Ship.Tribbles <= 0) ||
		(CURSYSTEM.Special == ERASERECORD && PoliceRecordScore >= DUBIOUSSCORE) ||
		(CURSYSTEM.Special == CARGOFORSALE && (FilledCargoBays() > TotalCargoBays() - 3)) ||
		((CURSYSTEM.Special == DRAGONFLY || CURSYSTEM.Special == JAPORIDISEASE ||
		CURSYSTEM.Special == ALIENARTIFACT || CURSYSTEM.Special == AMBASSADORJAREK ||
		CURSYSTEM.Special == EXPERIMENT) && (PoliceRecordScore < DUBIOUSSCORE)) ||
		(CURSYSTEM.Special == TRANSPORTWILD && (PoliceRecordScore >= DUBIOUSSCORE)) ||
		(CURSYSTEM.Special == GETREACTOR && (PoliceRecordScore >= DUBIOUSSCORE || ReputationScore < AVERAGESCORE || ReactorStatus != 0)) ||
		(CURSYSTEM.Special == REACTORDELIVERED && !(ReactorStatus > 0 && ReactorStatus < 21)) ||
		(CURSYSTEM.Special == MONSTERKILLED && MonsterStatus < 2) ||
		(CURSYSTEM.Special == EXPERIMENTSTOPPED && !(ExperimentStatus >= 1 && ExperimentStatus < 12)) ||
		(CURSYSTEM.Special == FLYBARATAS && DragonflyStatus < 1) ||
		(CURSYSTEM.Special == FLYMELINA && DragonflyStatus < 2) ||
		(CURSYSTEM.Special == FLYREGULAS && DragonflyStatus < 3) ||
		(CURSYSTEM.Special == DRAGONFLYDESTROYED && DragonflyStatus < 5) ||
		(CURSYSTEM.Special == SCARAB && (ReputationScore < AVERAGESCORE || ScarabStatus != 0)) ||
		(CURSYSTEM.Special == SCARABDESTROYED && ScarabStatus != 2) ||
		(CURSYSTEM.Special == GETHULLUPGRADED && ScarabStatus != 2) ||
		(CURSYSTEM.Special == MEDICINEDELIVERY && JaporiDiseaseStatus != 1) ||
		(CURSYSTEM.Special == JAPORIDISEASE && (JaporiDiseaseStatus != 0)) ||
		(CURSYSTEM.Special == ARTIFACTDELIVERY && !ArtifactOnBoard) ||
		(CURSYSTEM.Special == JAREKGETSOUT && JarekStatus != 1) ||
		(CURSYSTEM.Special == WILDGETSOUT && WildStatus != 1) ||
		(CURSYSTEM.Special == GEMULONRESCUED && !(InvasionStatus >= 1 && InvasionStatus <= 7)) ||
		(CURSYSTEM.Special == MOONFORSALE && (MoonBought || CurrentWorth() < (COSTMOON * 4) / 5)) ||
		(CURSYSTEM.Special == MOONBOUGHT && MoonBought != true))
		FrmHideObject( frmP, FrmGetObjectIndex( frmP, SystemInformationSpecialButton ) );
	else if (OpenQ > 3 &&
		(CURSYSTEM.Special == TRIBBLE ||
		 CURSYSTEM.Special == SPACEMONSTER ||
		 CURSYSTEM.Special == DRAGONFLY ||
		 CURSYSTEM.Special == JAPORIDISEASE ||
		 CURSYSTEM.Special == ALIENARTIFACT ||
		 CURSYSTEM.Special == AMBASSADORJAREK ||
		 CURSYSTEM.Special == ALIENINVASION ||
		 CURSYSTEM.Special == EXPERIMENT ||
		 CURSYSTEM.Special == TRANSPORTWILD ||
		 CURSYSTEM.Special == GETREACTOR ||
		 CURSYSTEM.Special == SCARAB))
		FrmHideObject( frmP, FrmGetObjectIndex( frmP, SystemInformationSpecialButton ) );
	else
		FrmShowObject( frmP, FrmGetObjectIndex( frmP, SystemInformationSpecialButton ) );
	if (GetForHire() < 0)
		FrmHideObject( frmP, FrmGetObjectIndex( frmP, SystemInformationMercenaryForHireButton ) );
	else
		FrmShowObject( frmP, FrmGetObjectIndex( frmP, SystemInformationMercenaryForHireButton ) );
	RectangularShortcuts( frmP, SystemInformationBButton );
	FrmDrawForm( frmP );
}

// *************************************************************************
// Handling of events on the System Information screen
// *************************************************************************
Boolean SystemInformationFormHandleEvent(EventPtr eventP)
{
    Boolean handled = false;
    OpenQuests();

	switch (eventP->eType) 
	{
		// Show information on current system
		case frmOpenEvent:
			// also, we decide if we want to set the News Flags here...
			if (CURSYSTEM.Special > -1)
			{
				if (CURSYSTEM.Special == MONSTERKILLED && MonsterStatus == 2)
					addNewsEvent(MONSTERKILLED);
				else if (CURSYSTEM.Special == DRAGONFLY)
					addNewsEvent(DRAGONFLY);
				else if (CURSYSTEM.Special == SCARAB)
					addNewsEvent(SCARAB);
				else if (CURSYSTEM.Special == SCARABDESTROYED && ScarabStatus == 2)
					addNewsEvent(SCARABDESTROYED);
				else if (CURSYSTEM.Special == FLYBARATAS && DragonflyStatus == 1)
					addNewsEvent(FLYBARATAS);
				else if (CURSYSTEM.Special == FLYMELINA && DragonflyStatus == 2)
					addNewsEvent(FLYMELINA);
				else if (CURSYSTEM.Special == FLYREGULAS && DragonflyStatus == 3)
					addNewsEvent(FLYREGULAS);
				else if (CURSYSTEM.Special == DRAGONFLYDESTROYED && DragonflyStatus == 5)
					addNewsEvent(DRAGONFLYDESTROYED);
				else if (CURSYSTEM.Special == MEDICINEDELIVERY && JaporiDiseaseStatus == 1)
					addNewsEvent(MEDICINEDELIVERY);
				else if (CURSYSTEM.Special == ARTIFACTDELIVERY && ArtifactOnBoard)
					addNewsEvent(ARTIFACTDELIVERY);
				else if (CURSYSTEM.Special == JAPORIDISEASE && JaporiDiseaseStatus == 0)
					addNewsEvent(JAPORIDISEASE);
				else if (CURSYSTEM.Special == JAREKGETSOUT && JarekStatus == 1)
					addNewsEvent(JAREKGETSOUT);
				else if (CURSYSTEM.Special == WILDGETSOUT && WildStatus == 1)
					addNewsEvent(WILDGETSOUT);
				else if (CURSYSTEM.Special == GEMULONRESCUED && InvasionStatus > 0 && InvasionStatus < 8)
					addNewsEvent(GEMULONRESCUED);
				else if (CURSYSTEM.Special == ALIENINVASION)
					addNewsEvent(ALIENINVASION);
				else if (CURSYSTEM.Special == EXPERIMENTSTOPPED && ExperimentStatus > 0 && ExperimentStatus < 12)
					addNewsEvent(EXPERIMENTSTOPPED);
				else if (CURSYSTEM.Special == EXPERIMENTNOTSTOPPED)
					addNewsEvent(EXPERIMENTNOTSTOPPED);
					
			}
			DrawSystemInformationForm();
			CURSYSTEM.Visited = true;
			handled = true;
			break;

		case frmUpdateEvent:
			DrawSystemInformationForm();
			handled = true;
			break;

		// Special event
		case ctlSelectEvent:
			if (eventP->data.ctlSelect.controlID == SystemInformationSpecialButton)
			{
				CurForm = SpecialEventForm;
			}
			else if (eventP->data.ctlSelect.controlID == SystemInformationMercenaryForHireButton)
			{
				CurForm = PersonnelRosterForm;
			}
			else if (eventP->data.ctlSelect.controlID == SystemInformationNewsButton)
			{
				StrIToA( SBuf, Difficulty + 1 );
				if (!AlreadyPaidForNewspaper && ToSpend() < (long)(Difficulty + 1.0) )
				{
					FrmCustomAlert(CantAffordPaperAlert, SBuf, NULL, NULL);
					return true;
				}
				else
				{	
					if (!NewsAutoPay && !AlreadyPaidForNewspaper)
						if (FrmCustomAlert( BuyNewspaperAlert, SBuf, NULL, NULL ) == BuyNewspaperCancel)
							return true;
					if (!AlreadyPaidForNewspaper)
					{
						Credits -= (Difficulty + 1);
						AlreadyPaidForNewspaper = true;
					}
					CurForm = NewspaperForm;
				}
			}

			FrmGotoForm( CurForm );
			handled = true;
			break;
				
		default:
			break;
	}
	
	return handled;
}


static void DrawNewspaperForm()
{
    FormPtr frmP;
    int line = 18, i, j;
    Boolean shown[MAXSTORIES];
    Boolean realNews = false;
	frmP = FrmGetActiveForm();
	FrmDrawForm( frmP );

	i = WarpSystem % MAXMASTHEADS;
	SysStringByIndex(AnarchyMastheadsStringList + CURSYSTEM.Politics * 100,i,SBuf2,50);
	if (StrNCompare(SBuf2,"*",1) == 0)
	{
		StrCopy(SBuf,"The ");
		StrCat(SBuf, SolarSystemName[CURSYSTEM.NameIndex]);
		StrCat(SBuf, SBuf2 + 1);
		//DrawCharsCentered(SBuf, line, true);
		setCurrentWinTitle(SBuf);
		
	}
	else if (StrNCompare(SBuf2,"+",1) == 0)
	{
		StrCopy(SBuf, SolarSystemName[CURSYSTEM.NameIndex]);
		StrCat(SBuf, SBuf2 + 1);
		//DrawCharsCentered(SBuf, line, true);
		setCurrentWinTitle(SBuf);

	}
	else
	{
		//DrawCharsCentered(SBuf2, line, true);
		setCurrentWinTitle(SBuf2);
	}
	
				
	RandSeed( WarpSystem, Days );
						    
	// Special Events get to go first, crowding out other news
	if  (isNewsEvent(CAPTAINHUIEATTACKED))
	{
		DisplayHeadline("Famed Captain Huie Attacked by Brigand!", &line);
	}
	if  (isNewsEvent(EXPERIMENTPERFORMED))
	{
		DisplayHeadline("Travelers Report Timespace Damage, Warp Problems!", &line);
	}
	if  (isNewsEvent(CAPTAINHUIEDESTROYED))
	{
		DisplayHeadline("Citizens Mourn Destruction of Captain Huie's Ship!", &line);
	}
	if  (isNewsEvent(CAPTAINAHABATTACKED))
	{
		DisplayHeadline("Thug Assaults Captain Ahab!", &line);
	}
	if  (isNewsEvent(CAPTAINAHABDESTROYED))
	{
		DisplayHeadline("Destruction of Captain Ahab's Ship Causes Anger!", &line);
	}
	if  (isNewsEvent(CAPTAINCONRADATTACKED))
	{
		DisplayHeadline("Captain Conrad Comes Under Attack By Criminal!", &line);
	}
	if  (isNewsEvent(CAPTAINCONRADDESTROYED))
	{
		DisplayHeadline("Captain Conrad's Ship Destroyed by Villain!", &line);
	}
	if  (isNewsEvent(MONSTERKILLED))
	{
		DisplayHeadline("Hero Slays Space Monster! Parade, Honors Planned for Today.", &line);
	}
	if  (isNewsEvent(WILDARRESTED))
	{
		DisplayHeadline("Notorious Criminal Jonathan Wild Arrested!", &line);
	}
	if  (CURSYSTEM.Special == MONSTERKILLED &&  MonsterStatus == 1)
	{
		DisplayHeadline("Space Monster Threatens Homeworld!", &line);
	}
	if  (CURSYSTEM.Special == SCARABDESTROYED &&  ScarabStatus == 1)
	{
		DisplayHeadline("Wormhole Travelers Harassed by Unusual Ship!", &line);
	}
	if (isNewsEvent(EXPERIMENTSTOPPED))
	{
		DisplayHeadline("Scientists Cancel High-profile Test! Committee to Investigate Design.", &line);
	}
	if (isNewsEvent(EXPERIMENTNOTSTOPPED))
	{
		DisplayHeadline("Huge Explosion Reported at Research Facility.", &line);
	}
	if (isNewsEvent(DRAGONFLY))
	{
		DisplayHeadline("Experimental Craft Stolen! Critics Demand Security Review.", &line);
	}
	if (isNewsEvent(SCARAB))
	{
		DisplayHeadline("Security Scandal: Test Craft Confirmed Stolen.", &line);
	}
	if (isNewsEvent(FLYBARATAS))
	{
		DisplayHeadline("Investigators Report Strange Craft.", &line);
	}
	if (isNewsEvent(FLYMELINA))
	{
		DisplayHeadline("Rumors Continue: Melina Orbitted by Odd Starcraft.", &line);
	}
	if (isNewsEvent(FLYREGULAS))
	{
		DisplayHeadline("Strange Ship Observed in Regulas Orbit.", &line);
	}
	if (CURSYSTEM.Special == DRAGONFLYDESTROYED && DragonflyStatus == 4 &&
	    !isNewsEvent(DRAGONFLYDESTROYED))
	{
		DisplayHeadline("Unidentified Ship: A Threat to Zalkon?", &line);
	}
	if (isNewsEvent(DRAGONFLYDESTROYED))
	{
		DisplayHeadline("Spectacular Display as Stolen Ship Destroyed in Fierce Space Battle.", &line);
	}
	if (isNewsEvent(SCARABDESTROYED))
	{
		DisplayHeadline("Wormhole Traffic Delayed as Stolen Craft Destroyed.", &line);
	}
	if (isNewsEvent(MEDICINEDELIVERY))
	{
		DisplayHeadline("Disease Antidotes Arrive! Health Officials Optimistic.", &line);
	}
	if (isNewsEvent(JAPORIDISEASE))
	{
		DisplayHeadline("Editorial: We Must Help Japori!",&line);
	}
	if (isNewsEvent(ARTIFACTDELIVERY))
	{
		DisplayHeadline("Scientist Adds Alien Artifact to Museum Collection.", &line);
	}
	if (isNewsEvent(JAREKGETSOUT))
	{
		DisplayHeadline("Ambassador Jarek Returns from Crisis.", &line);
	} 
	if (isNewsEvent(WILDGETSOUT))
	{
		DisplayHeadline("Rumors Suggest Known Criminal J. Wild May Come to Kravat!", &line);
	} 
	if (isNewsEvent(GEMULONRESCUED))
	{
		DisplayHeadline("Invasion Imminent! Plans in Place to Repel Hostile Invaders.", &line);
	}
	if (CURSYSTEM.Special == GEMULONRESCUED && !isNewsEvent(GEMULONRESCUED))
	{
		DisplayHeadline("Alien Invasion Devastates Planet!", &line);
	}
	if (isNewsEvent(ALIENINVASION))
	{
		DisplayHeadline("Editorial: Who Will Warn Gemulon?", &line);
	}
	if (isNewsEvent(ARRIVALVIASINGULARITY))
	{
		DisplayHeadline("Travelers Claim Sighting of Ship Materializing in Orbit!", &line);
	}


	// local system status information
	if (CURSYSTEM.Status > 0)
	{
		switch (CURSYSTEM.Status)
		{
			case WAR:
				DisplayHeadline("War News: Offensives Continue!", &line);
				break;
			case PLAGUE:
				DisplayHeadline("Plague Spreads! Outlook Grim.", &line);
				break;
			case DROUGHT:
				DisplayHeadline("No Rain in Sight!",&line);
				break;
			case BOREDOM:
				DisplayHeadline("Editors: Won't Someone Entertain Us?",&line);
				break;
			case COLD:
				DisplayHeadline("Cold Snap Continues!", &line);
				break;
			case CROPFAILURE:
				DisplayHeadline("Serious Crop Failure! Must We Ration?", &line);
				break;
			case LACKOFWORKERS:
				DisplayHeadline("Jobless Rate at All-Time Low!", &line);
				break;
		}
	}
	
	// character-specific news.
	if (PoliceRecordScore <= VILLAINSCORE)
	{
		j = GetRandom2(4);
		switch (j)
		{
			case 0:
				StrCopy( SBuf, "Police Warning: ");
				StrCat( SBuf, NameCommander);
				StrCat( SBuf, " Will Dock At ");
				StrCat( SBuf, SolarSystemName[CURSYSTEM.NameIndex]);
				StrCat( SBuf, "!");
				break;
			case 1:
				StrCopy(SBuf,"Notorious Criminal ");
				StrCat(SBuf, NameCommander);
				StrCat(SBuf, " Sighted in ");					
				StrCat( SBuf, SolarSystemName[CURSYSTEM.NameIndex]);
				StrCat( SBuf, "!");
				break;
			case 2:
				StrCopy(SBuf,"Locals Rally to Deny Spaceport Access to ");
				StrCat(SBuf, NameCommander);
				StrCat(SBuf, "!");
				break;
			case 3:
				StrCopy(SBuf,"Terror Strikes Locals on Arrival of ");
				StrCat(SBuf, NameCommander);
				StrCat(SBuf, "!");
				break;
		}
		DisplayHeadline(SBuf, &line);
	}

	if (PoliceRecordScore == HEROSCORE)
	{
		j = GetRandom2(3);
		switch (j)
		{
			case 0:
				StrCopy(SBuf,"Locals Welcome Visiting Hero ");
				StrCat(SBuf, NameCommander);
				StrCat(SBuf, "!");
				break;
			case 1:
				StrCopy(SBuf,"Famed Hero ");
				StrCat(SBuf, NameCommander);
				StrCat(SBuf, " to Visit System!");					
				break;
			case 2:
				StrCopy(SBuf,"Large Turnout At Spaceport to Welcome ");
				StrCat(SBuf, NameCommander);
				StrCat(SBuf, "!");
				break;
		}
		DisplayHeadline(SBuf, &line);
	}
	
	// caught littering?
	if  (isNewsEvent(CAUGHTLITTERING))
	{
		StrCopy( SBuf, "Police Trace Orbiting Space Litter to ");
		StrCat( SBuf, NameCommander );
		StrCat( SBuf, ".");
		DisplayHeadline(SBuf, &line);
	}

	
	// and now, finally, useful news (if any)
	// base probability of a story showing up is (50 / MAXTECHLEVEL) * Current Tech Level
	// This is then modified by adding 10% for every level of play less than Impossible
	for (i=0; i < MAXSOLARSYSTEM; i++)
	{
		if (i != COMMANDER.CurSystem &&
		    ((RealDistance(CURSYSTEM, SolarSystem[i]) <= Shiptype[Ship.Type].FuelTanks)
		    ||
		    (WormholeExists( COMMANDER.CurSystem, i )))
		    &&
		    SolarSystem[i].Status > 0)
		    
		{
			// Special stories that always get shown: moon, millionaire
			if (SolarSystem[i].Special == MOONFORSALE)
			{
				StrCopy(SBuf, "Seller in ");
				StrCat(SBuf,SolarSystemName[i]);
				StrCat(SBuf, " System has Utopian Moon available.");
				DisplayHeadline(SBuf, &line);
			}
			if (SolarSystem[i].Special == BUYTRIBBLE)
			{
				StrCopy(SBuf, "Collector in ");
				StrCat(SBuf,SolarSystemName[i]);
				StrCat(SBuf, " System seeks to purchase Tribbles.");
				DisplayHeadline(SBuf, &line);
			}
			
			// And not-always-shown stories
			if (GetRandom2(100) <= STORYPROBABILITY * CURSYSTEM.TechLevel + 10 * (5 - Difficulty))
			{
				j = GetRandom2(6);
				switch (j)
				{
					case 0:
						StrCopy(SBuf, "Reports of");
						break; 
					case 1:
						StrCopy(SBuf, "News of");
						break;
					case 2:
						StrCopy(SBuf, "New Rumors of");
						break;
					case 3:
						StrCopy(SBuf, "Sources say");
						break;
					case 4:
						StrCopy(SBuf, "Notice:");
						break;
					case 5:
						StrCopy(SBuf, "Evidence Suggests");
						break;
				}
				StrCat(SBuf, " ");
				switch (SolarSystem[i].Status)
				{
					case WAR:
						StrCat(SBuf,"Strife and War");
						break;
					case PLAGUE:
						StrCat(SBuf, "Plague Outbreaks");
						break;
					case DROUGHT:
						StrCat(SBuf, "Severe Drought");
						break;
					case BOREDOM:
						StrCat(SBuf, "Terrible Boredom");
						break;
					case COLD:
						StrCat(SBuf, "Cold Weather");
						break;
					case CROPFAILURE:
						StrCat(SBuf, "Crop Failures");
						break;
					case LACKOFWORKERS:
						StrCat(SBuf, "Labor Shortages");
						break;
				}
				StrCat(SBuf, " in the ");
				StrCat(SBuf,SolarSystemName[i]);
				StrCat(SBuf, " System.");
				DisplayHeadline(SBuf, &line);
				realNews = true;
			}
		}
	}
	
	// if there's no useful news, we throw up at least one
	// headline from our canned news list.
	if (! realNews)
	{
		for (i=0; i< MAXSTORIES; i++)
		{
			shown[i]= false;
		}
		for (i=0; i <=GetRandom2(MAXSTORIES); i++)
		{
			j = GetRandom2(MAXSTORIES);
			if (!shown[j] && line <= 150) 
			{
				SysStringByIndex(AnarchyHeadlinesStringList + CURSYSTEM.Politics * 100,j,SBuf,63);
				DisplayHeadline(SBuf, &line);
				shown[j] = true;
			}
		}
	}
	

}

// Show the newspaper screen
Boolean NewspaperFormHandleEvent(EventPtr eventP)
{
    Boolean handled = false;

	switch (eventP->eType) 
	{
		// Show information on current system
		case frmOpenEvent:
		case frmUpdateEvent:
			DrawNewspaperForm();
			handled = true;
			break;

		// Special event
		case ctlSelectEvent:
			if (eventP->data.ctlSelect.controlID == NewspaperDoneButton)
			{
				CurForm = SystemInformationForm;
			}
			FrmGotoForm( CurForm );
			handled = true;
			break;
				
		default:
			break;
	}
	
	return handled;
}


// *****************************************************************
// Handling of the events of the Personnel Roster form
// *****************************************************************
Boolean PersonnelRosterFormHandleEvent( EventPtr eventP )
{
    Boolean handled = false;
    Boolean Sale;
	int FirstFree, ForHire, oldtraderskill;

	switch (eventP->eType) 
	{
		case frmOpenEvent:
			DrawPersonnelRoster();
			handled = true;
			break;

		case frmUpdateEvent:
			DrawPersonnelRoster();
			handled = true;
			break;

			
		case ctlSelectEvent:
			Sale = true;
			oldtraderskill = TraderSkill( &Ship );
			if (eventP->data.ctlSelect.controlID == PersonnelRosterFire0Button) 
			{
				if (FrmAlert( FireMercenaryAlert ) == 0)
				{
					Ship.Crew[1] = Ship.Crew[2];
					Ship.Crew[2] = -1;
				}
			}
			else if (eventP->data.ctlSelect.controlID == PersonnelRosterFire1Button) 
			{
				if (FrmAlert( FireMercenaryAlert ) == 0)
				{
					Ship.Crew[2] = -1;
				}
			}
			else if (eventP->data.ctlSelect.controlID == PersonnelRosterHire0Button) 
			{
			 /*
				ForHire = -1;
				for (i=1; i<MAXCREWMEMBER; ++i)
				{
					if (i == Ship.Crew[1] || i == Ship.Crew[2])
						continue;
					if (Mercenary[i].CurSystem == Mercenary[0].CurSystem)
					{
						ForHire = i;
						break;
					}
				}
	          */
	          ForHire = GetForHire();
	
				FirstFree = -1;
				if (Ship.Crew[1] == -1)
					FirstFree = 1;
				else if (Ship.Crew[2] == -1)
					FirstFree = 2;
					
				if ((FirstFree < 0) ||
					(AvailableQuarters() <= FirstFree))
				{
					FrmAlert( NoFreeQuartersAlert );
					Sale = false;
				}
				else
					Ship.Crew[FirstFree] = ForHire;
			}
			if (Sale)
			{
				DrawPersonnelRoster();
				if (oldtraderskill != TraderSkill( &Ship ))
					RecalculateBuyPrices(COMMANDER.CurSystem);
			}
			handled = true;
			break;

		default:
			break;
	}
	
	return handled;
}
