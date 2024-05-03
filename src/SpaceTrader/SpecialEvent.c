/***********************************************************************
 *
 * SPACE TRADER 1.2.2
 *
 * SpecialEvent.c
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
// Handling of events on the Special Event screen
// *************************************************************************
#include "external.h"

static void DrawSpecialEventForm()
{
    FormPtr frmP;
	char theText[320];

	frmP = FrmGetActiveForm();
	setCurrentWinTitle( SpecialEvent[CURSYSTEM.Special].Title );
	if (SpecialEvent[CURSYSTEM.Special].JustAMessage)
	{
		FrmHideObject( frmP, FrmGetObjectIndex( frmP, SpecialEventYesButton ) );
		FrmHideObject( frmP, FrmGetObjectIndex( frmP, SpecialEventNoButton ) );
		FrmShowObject( frmP, FrmGetObjectIndex( frmP, SpecialEventOKButton ) );
	}
	else
	{
		FrmShowObject( frmP, FrmGetObjectIndex( frmP, SpecialEventYesButton ) );
		FrmShowObject( frmP, FrmGetObjectIndex( frmP, SpecialEventNoButton ) );
		FrmHideObject( frmP, FrmGetObjectIndex( frmP, SpecialEventOKButton ) );
	}
	FrmDrawForm ( frmP );
	
	SysCopyStringResource(theText, SpecialEvent[CURSYSTEM.Special].QuestStringID);
	DisplayPage(theText, 18);
}

Boolean SpecialEventFormHandleEvent( EventPtr eventP )
{
    Boolean handled = false;
	int i, FirstEmptySlot;

	switch (eventP->eType) 
	{
		case frmOpenEvent:
		case frmUpdateEvent:
			DrawSpecialEventForm();
			handled = true;
			break;

		// Special event
		case ctlSelectEvent:
			if (eventP->data.ctlSelect.controlID != SpecialEventNoButton)
			{
				if (ToSpend() < SpecialEvent[CURSYSTEM.Special].Price)
				{
					FrmAlert( NotEnoughForEventAlert );
					handled = true;
					break;
				}
			
				Credits -= SpecialEvent[CURSYSTEM.Special].Price;
				
				switch (CURSYSTEM.Special)
				{
				
					case GETREACTOR:
						if (FilledCargoBays() > TotalCargoBays() - 15)
						{
							FrmAlert( NotEnoughBaysAlert );
							handled = true;
							break;
						}
						else if (WildStatus == 1)
						{
							if (FrmCustomAlert(WildWontStayOnboardAlert, SolarSystemName[CURSYSTEM.NameIndex],
							    NULL, NULL) == WildWontStayOnboardSayGoodbyetoWild)
							{
								WildStatus = 0;
								FrmCustomAlert( WildLeavesShipAlert,SolarSystemName[CURSYSTEM.NameIndex],
								NULL, NULL);
							}
							else
							{
								handled = true;
								break;
							}
							    
						}
						FrmAlert( ReactorAlert );
						ReactorStatus = 1;
						break;
						
					case REACTORDELIVERED:
						CURSYSTEM.Special = GETSPECIALLASER;
						ReactorStatus = 21;
						handled = true;
						break;	
						
					case MONSTERKILLED:
						break;
						
					case SCARAB:
						ScarabStatus = 1;
						break;

					case SCARABDESTROYED:
						ScarabStatus = 2;
						CURSYSTEM.Special = GETHULLUPGRADED;
						handled = true;
						break;	

					case GETHULLUPGRADED:
						FrmAlert( HullUpgradeAlert );
						Ship.Hull += UPGRADEDHULL;
						ScarabStatus = 3;
						handled = true;
						break;	
						
					case EXPERIMENT:
						ExperimentStatus = 1;
						break;
						
					case EXPERIMENTSTOPPED:
						ExperimentStatus = 13;
						CanSuperWarp = true;
						break;
						
					case EXPERIMENTNOTSTOPPED:
						break;
				
					case ARTIFACTDELIVERY:
						ArtifactOnBoard = false;
						break;
				
					case ALIENARTIFACT:
						ArtifactOnBoard = true;
						break;
				
					case FLYBARATAS:
					case FLYMELINA:
					case FLYREGULAS:
						++DragonflyStatus;
						break;
				
					case DRAGONFLYDESTROYED:
						CURSYSTEM.Special = INSTALLLIGHTNINGSHIELD;
						handled = true;
						break;
				
					case GEMULONRESCUED:
						CURSYSTEM.Special = GETFUELCOMPACTOR;
						InvasionStatus = 0;
						handled = true;
						break;
				
					case MEDICINEDELIVERY:
						JaporiDiseaseStatus = 2;
						IncreaseRandomSkill();
						IncreaseRandomSkill();
						break;

					case MOONFORSALE:
						FrmAlert( BoughtMoonAlert );
						MoonBought = true;
						break;
						
					case MOONBOUGHT:
						EraseRectangle( 0, 0, 160, 160 );
						CurForm = UtopiaForm;
						FrmGotoForm( UtopiaForm );
						return true;
						break;
						
					case SKILLINCREASE:
						FrmAlert( SkillIncreaseAlert );
						IncreaseRandomSkill();
						break;
					
					case TRIBBLE:
						FrmAlert( YouHaveATribbleAlert );						
						Ship.Tribbles = 1;
						break;
					
					case BUYTRIBBLE:
						FrmAlert( BeamOverTribblesAlert );
						Credits += (Ship.Tribbles >> 1);
						Ship.Tribbles = 0;
						break;
					
					case ERASERECORD:
						FrmAlert( CleanRecordAlert );
						PoliceRecordScore = CLEANSCORE;
						RecalculateSellPrices();
						break;

					case SPACEMONSTER:
						MonsterStatus = 1;
						for (i=0; i<MAXSOLARSYSTEM; ++i)
							if (SolarSystem[i].Special == SPACEMONSTER)
								SolarSystem[i].Special = -1;
						break;

					case DRAGONFLY:
						DragonflyStatus = 1;
						for (i=0; i<MAXSOLARSYSTEM; ++i)
							if (SolarSystem[i].Special == DRAGONFLY)
								SolarSystem[i].Special = -1;
						break;
						
					case AMBASSADORJAREK:
						if (Ship.Crew[Shiptype[Ship.Type].CrewQuarters-1] >= 0)
						{
							FrmCustomAlert( NoQuartersAvailableAlert, "Ambassador Jarek", NULL, NULL );
							handled = true;
							break;
						}
						FrmCustomAlert( PassengerTakenOnBoardAlert, "Ambassador Jarek", NULL, NULL );
						JarekStatus = 1;
						break;

					case TRANSPORTWILD:
										
						if (Ship.Crew[Shiptype[Ship.Type].CrewQuarters-1] >= 0)
						{
							FrmCustomAlert( NoQuartersAvailableAlert, "Jonathan Wild", NULL, NULL );
							handled = true;
							break;
						}
						if (!HasWeapon(&Ship, BEAMLASERWEAPON, false))
						{
							FrmAlert (WildWontGetAboardAlert);
							handled = true;
							break;
						}
						if (ReactorStatus > 0 && ReactorStatus < 21)
						{
							FrmAlert( WildAfraidOfReactorAlert );
							handled = true;
							break;
						}
						FrmCustomAlert( PassengerTakenOnBoardAlert, "Jonathan Wild", NULL, NULL );
						WildStatus = 1;
						break;

						
					case ALIENINVASION:
						InvasionStatus = 1;
						break;
						
					case JAREKGETSOUT:
						JarekStatus = 2;
						RecalculateBuyPrices(COMMANDER.CurSystem);
						break;

					case WILDGETSOUT:
						WildStatus = 2;
						Mercenary[MAXCREWMEMBER-1].CurSystem = KRAVATSYSTEM;
						// Zeethibal has a 10 in player's lowest score, an 8
						// in the next lowest score, and 5 elsewhere.
						Mercenary[MAXCREWMEMBER-1].Pilot = 5;
						Mercenary[MAXCREWMEMBER-1].Fighter = 5;
						Mercenary[MAXCREWMEMBER-1].Trader = 5;
						Mercenary[MAXCREWMEMBER-1].Engineer = 5;
						switch (NthLowestSkill(&Ship, 1))
						{
							case PILOTSKILL:
								Mercenary[MAXCREWMEMBER-1].Pilot = 10;
								break;
							case FIGHTERSKILL:
								Mercenary[MAXCREWMEMBER-1].Fighter = 10;
								break;
							case TRADERSKILL:
								Mercenary[MAXCREWMEMBER-1].Trader = 10;
								break;
							case ENGINEERSKILL:
								Mercenary[MAXCREWMEMBER-1].Engineer = 10;
								break;
						}
						switch (NthLowestSkill(&Ship, 2))
						{
							case PILOTSKILL:
								Mercenary[MAXCREWMEMBER-1].Pilot = 8;
								break;
							case FIGHTERSKILL:
								Mercenary[MAXCREWMEMBER-1].Fighter = 8;
								break;
							case TRADERSKILL:
								Mercenary[MAXCREWMEMBER-1].Trader = 8;
								break;
							case ENGINEERSKILL:
								Mercenary[MAXCREWMEMBER-1].Engineer = 8;
								break;
						}

						if (PoliceRecordScore < CLEANSCORE)
							PoliceRecordScore = CLEANSCORE;
						break;

						
					case CARGOFORSALE:
						FrmAlert( SealedCannistersAlert );
						i = GetRandom( MAXTRADEITEM );
						Ship.Cargo[i] += 3;
						BuyingPrice[i] += SpecialEvent[CURSYSTEM.Special].Price;
						break;
						
					case INSTALLLIGHTNINGSHIELD:
						FirstEmptySlot = GetFirstEmptySlot( Shiptype[Ship.Type].ShieldSlots, Ship.Shield );
						if (FirstEmptySlot < 0)
						{
							FrmAlert( NotEnoughSlotsAlert );
							handled = true;
						}
						else
						{
							FrmAlert( LightningShieldAlert );
							Ship.Shield[FirstEmptySlot] = LIGHTNINGSHIELD;
							Ship.ShieldStrength[FirstEmptySlot] = Shieldtype[LIGHTNINGSHIELD].Power;
						}
						break;

					case GETSPECIALLASER:
						FirstEmptySlot = GetFirstEmptySlot( Shiptype[Ship.Type].WeaponSlots, Ship.Weapon );
						if (FirstEmptySlot < 0)
						{
							FrmAlert( NotEnoughSlotsAlert );
							handled = true;
						}
						else
						{
							FrmAlert(MorganLaserAlert );
							Ship.Weapon[FirstEmptySlot] = MORGANLASERWEAPON;
						}
						break;

					case GETFUELCOMPACTOR:
						FirstEmptySlot = GetFirstEmptySlot( Shiptype[Ship.Type].GadgetSlots, Ship.Gadget );
						if (FirstEmptySlot < 0)
						{
							FrmAlert( NotEnoughSlotsAlert );
							handled = true;
						}
						else
						{
							FrmAlert( FuelCompactorAlert );
							Ship.Gadget[FirstEmptySlot] = FUELCOMPACTOR;
							Ship.Fuel = GetFuelTanks();
						}
						break;

					case JAPORIDISEASE:
						if (FilledCargoBays() > TotalCargoBays() - 10)
						{
							FrmAlert( NotEnoughBaysAlert );
							handled = true;
						}
						else
						{
							FrmAlert( AntidoteAlert );
							JaporiDiseaseStatus = 1;
						}
						break;
				}

				if (!handled)				
					CURSYSTEM.Special = -1;
			}
			CurForm = SystemInformationForm;
			FrmGotoForm( CurForm );
			handled = true;
			break;
				
		default:
			break;
	}
	
	return handled;
}
