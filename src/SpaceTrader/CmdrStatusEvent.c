/***********************************************************************
 *
 * SPACE TRADER 1.2.0
 *
 * CmdrStatusEvent.c
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
// Commander Status Event Module
// *************************************************************************
#include "external.h"

// *************************************************************************
// Show skill on the Commander Status screen
// *************************************************************************
static void DisplaySkill( int Skill, int AdaptedSkill, FormPtr frmP, Int32 Label )
{
	StrIToA( SBuf, Skill );
	StrCat( SBuf, " [" );
	StrIToA( SBuf2, AdaptedSkill );
	StrCat( SBuf, SBuf2 );
	StrCat( SBuf, "]" );
	setLabelText( frmP, Label, SBuf );
} 

// *************************************************************************
// Show the Commander Status screen
// *************************************************************************
static void ShowCommanderStatus( void )
{
	FormPtr frmP;
	int i;
					
	frmP = FrmGetActiveForm();
	
	RectangularShortcuts( frmP, CommanderStatusBButton );
		
	
	setLabelText( frmP, CommanderStatusNameLabel, NameCommander );

	DisplaySkill( COMMANDER.Pilot, PilotSkill( &Ship ), frmP, CommanderStatusPilotLabel ); 
	DisplaySkill( COMMANDER.Fighter, FighterSkill( &Ship ), frmP, CommanderStatusFighterLabel ); 
	DisplaySkill( COMMANDER.Trader, TraderSkill( &Ship ), frmP, CommanderStatusTraderLabel ); 
	DisplaySkill( COMMANDER.Engineer, EngineerSkill( &Ship ), frmP, CommanderStatusEngineerLabel ); 

	StrIToA( SBuf, PoliceKills + TraderKills + PirateKills );
	setLabelText( frmP, CommanderStatusKillsLabel, SBuf );

	i = 0;
	while (i < MAXPOLICERECORD && PoliceRecordScore >= PoliceRecord[i].MinScore)
		++i;
	--i;
	if (i < 0)
		++i;
	setLabelText( frmP, CommanderStatusPoliceRecordLabel, PoliceRecord[i].Name );

	i = 0;
	while (i < MAXREPUTATION && ReputationScore >= Reputation[i].MinScore)
		++i;
	--i;
	if (i < 0)
		i = 0;
	setLabelText( frmP, CommanderStatusReputationLabel, Reputation[i].Name );

	setLabelText( frmP, CommanderStatusLevelLabel, DifficultyLevel[(int)Difficulty] );

	StrIToA( SBuf, Days );
	StrCat( SBuf, " days" );
	setLabelText( frmP, CommanderStatusDaysLabel, SBuf );

	StrIToA( SBuf, Credits );
	StrCat( SBuf, " cr." );
	setLabelText( frmP, CommanderStatusCreditsLabel, SBuf );

	StrIToA( SBuf, Debt );
	StrCat( SBuf, " cr." );
	setLabelText( frmP, CommanderStatusDebtLabel, SBuf );

	StrIToA (SBuf, CurrentWorth());
	StrCat (SBuf, " cr." );
	setLabelText(frmP, CommanderStatusNetWorthLabel, SBuf);

	#ifdef _STRA_CHEAT_
	if (CheatCounter == 3)
		FrmShowObject( frmP, FrmGetObjectIndex( frmP, CommanderStatusCheatButton ));
	else
		FrmHideObject( frmP, FrmGetObjectIndex( frmP, CommanderStatusCheatButton ));
    #endif
    
    FrmDrawForm( frmP );
    }

// *************************************************************************
// Handling of events on the Commander Status screen
// *************************************************************************
Boolean CommanderStatusFormHandleEvent(EventPtr eventP)
{
    Boolean handled = false;
    static FormPtr frm;
    static int i;
	static Handle AmountH[4];

	switch (eventP->eType) 
	{
		// Show information on current system
		case frmOpenEvent:
			ShowCommanderStatus();
			CheatCounter = 0;
			handled = true;
			break;

		case frmUpdateEvent:
			ShowCommanderStatus();
			handled = true;
			break;


		case ctlSelectEvent:
            
            #ifdef _STRA_CHEAT_ // cheat enable defined ?
            		
			if (eventP->data.ctlSelect.controlID == CommanderStatusCheatButton)
			{
				frm = FrmInitForm( CheatForm );
	
				StrIToA( SBuf, Credits );
				AmountH[0] = SetField( frm, CheatCreditsField, SBuf, 9, true );
			
				StrIToA( SBuf, Debt );
				AmountH[1] = SetField( frm, CheatDebtField, SBuf, 9, true );
				
				StrIToA( SBuf, ReputationScore );
				AmountH[2] = SetField( frm, CheatReputationField, SBuf, 5, false );

				StrIToA( SBuf, ABS( PoliceRecordScore ) );
				AmountH[3] = SetField( frm, CheatPoliceRecordField, SBuf, 5, false );

				SetCheckBox( frm, CheatNegativeCheckbox, (PoliceRecordScore < 0) );
				SetCheckBox( frm, CheatMoonCheckbox, MoonBought );
				SetCheckBox( frm, CheatLightningShieldCheckbox, HasShield( &Ship, LIGHTNINGSHIELD ) );
				SetCheckBox( frm, CheatFuelCompactorCheckbox, HasGadget( &Ship, FUELCOMPACTOR ) );
				SetCheckBox( frm, CheatMorganLaserCheckbox, HasWeapon( &Ship, MORGANLASERWEAPON, true ) );
				SetCheckBox( frm, CheatSingularityCheckbox, CanSuperWarp );

				FrmDoDialog( frm );

				GetField( frm, CheatCreditsField, SBuf, AmountH[0] );
				if (SBuf[0] != '\0')
					Credits = StrAToI( SBuf );
                
                GetField(frm, CheatDebtField, SBuf, AmountH[1]);
                if (SBuf[0] != '\0')
                    Debt = StrAToI(SBuf);
                    
				GetField( frm, CheatReputationField, SBuf, AmountH[2] );
				if (SBuf[0] != '\0')
					ReputationScore = StrAToI( SBuf );

				GetField( frm, CheatPoliceRecordField, SBuf, AmountH[3] );
				if (SBuf[0] != '\0')
					PoliceRecordScore = StrAToI( SBuf );

				if (GetCheckBox( frm, CheatNegativeCheckbox ))
					PoliceRecordScore = -PoliceRecordScore;

				MoonBought = GetCheckBox( frm, CheatMoonCheckbox );

				if (GetCheckBox( frm, CheatLightningShieldCheckbox ) && !HasShield( &Ship, LIGHTNINGSHIELD ) &&
					Shiptype[Ship.Type].ShieldSlots > 0)
					Ship.Shield[0] = LIGHTNINGSHIELD;
				else if (!GetCheckBox( frm, CheatLightningShieldCheckbox ) && HasShield( &Ship, LIGHTNINGSHIELD ))
				{
					for (i=0; i<MAXSHIELD; ++i)
						if (Ship.Shield[i] == LIGHTNINGSHIELD)
							Ship.Shield[i] = 1;
				}

				if (GetCheckBox( frm, CheatFuelCompactorCheckbox ) && !HasGadget( &Ship, FUELCOMPACTOR ) &&
					Shiptype[Ship.Type].GadgetSlots > 0)
					Ship.Gadget[0] = FUELCOMPACTOR;
				else if (!GetCheckBox( frm, CheatFuelCompactorCheckbox ) && HasGadget( &Ship, FUELCOMPACTOR ))
				{
					for (i=0; i<MAXGADGET; ++i)
						if (Ship.Gadget[i] == FUELCOMPACTOR)
							Ship.Gadget[i] = 1;
				}
				if (GetCheckBox( frm, CheatMorganLaserCheckbox ) && !HasWeapon( &Ship, MORGANLASERWEAPON, true) &&
					Shiptype[Ship.Type].WeaponSlots > 0)
					Ship.Weapon[0] = MORGANLASERWEAPON;
				else if (!GetCheckBox( frm, CheatMorganLaserCheckbox ) && HasWeapon( &Ship, MORGANLASERWEAPON, true ))
				{
					for (i=0; i<MAXWEAPON; ++i)
						if (Ship.Weapon[i] == MORGANLASERWEAPON)
							Ship.Weapon[i] = 1;
				}

				CanSuperWarp = GetCheckBox( frm, CheatSingularityCheckbox );
				FrmDeleteForm( frm );
				
				ShowCommanderStatus();
			}
			else 
			
			#endif  // Cheat Enabled End 
			
			if (eventP->data.ctlSelect.controlID == CommanderStatusQuestsButton)
			{
				CurForm = QuestsForm;
			}
			else if (eventP->data.ctlSelect.controlID == CommanderStatusSpecialButton)
			{
				CurForm = SpecialCargoForm;
			}
			else
			{
				CurForm = CurrentShipForm;
			}
			FrmGotoForm( CurForm );
			handled = true;
			break;
			
		default:
			break;
	}
	
	return handled;
}

