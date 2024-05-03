/***********************************************************************
 *
 * SPACE TRADER 1.2.0
 *
 * AppHandleEvent.c
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

// *************************************************************************
// The standard menu handling of all "Docked" forms.
// *************************************************************************
static Boolean DockedFormDoCommand(Word command)
{
	Boolean handled = true;
	FormPtr	frm;
	Handle AmountH;
	int NewForm = -1;
	UInt16 buttonPressed = 0;
	Boolean inOpts;
	Boolean SkipOptions1;
	char label1[21], label2[21], label3[21], label4[21];
	
	switch (command)
	{
		case MenuGameNewGame:
			if (FrmAlert( NewGameAlert) == NewGameYes)
				StartNewGame();
			break;

		case MenuGameSnapshot:
			if (SaveGame( 1 ))
				FrmAlert( GameSavedAlert );
			break;
			
		case MenuGameSwitchGame:
			if (FrmAlert( SwitchGameAlert ) != SwitchGameYes)
				break;
			if (SaveGame( 2 ))
			{
				if (NameCommander[0] == '\0')
				{
					FrmAlert( SwitchToNewAlert );
					IdentifyStartup = true;
					StartNewGame();
				}
				else
				{
					FrmCustomAlert( SwitchedAlert, NameCommander, "", "" );
					FrmGotoForm( CurForm );
				}
			}
			break;

		case MenuGameHighScores:
			ViewHighScores();
			break;

		case MenuGameClearHighScores:
			ClearHighScores();
			break;

		case MenuHelpAbout:
			AutoAttack = false;
			AutoFlee = false;
			MenuEraseStatus( 0 );
			frm = FrmInitForm( AboutForm );
			FrmDoDialog( frm );
			FrmDeleteForm( frm );
			break;

		case MenuCommandGalacticChart:
			NewForm = GalacticChartForm;
			break;
	
		case MenuCommandSystemInformation:
			NewForm = SystemInformationForm;
			break;

		case MenuCommandShortRangeChart:
			NewForm = WarpForm;
			break;

		case MenuCommandBuyCargo:
			NewForm = BuyCargoForm;
			break;

		case MenuCommandSellCargo:
			NewForm = SellCargoForm;
			break;

		case MenuCommandShipYard:
			NewForm = ShipYardForm;
			break;

		case MenuCommandBuyEquipment:
			NewForm = BuyEquipmentForm;
			break;

		case MenuCommandSellEquipment:
			NewForm = SellEquipmentForm;
			break;

		case MenuCommandPersonnelRoster:
			NewForm = PersonnelRosterForm;
			break;

		case MenuCommandCommanderStatus:
			NewForm = CommanderStatusForm;
			break;

		case MenuCommandBank:
			NewForm = BankForm;
			break;

		case MenuGameRetire:
			if (FrmAlert( RetireAlert) == RetireYes)
				NewForm = RetireForm;
			break;
			
		case MenuGameShortcuts:
			AutoAttack = false;
			AutoFlee = false;
			MenuEraseStatus( 0 );
			frm = FrmInitForm( ShortcutsForm );
			SetTriggerList(frm, ShortcutsShortcuts1List, Shortcut1);
			SetTriggerList(frm, ShortcutsShortcuts2List, Shortcut2);
			SetTriggerList(frm, ShortcutsShortcuts3List, Shortcut3);
			SetTriggerList(frm, ShortcutsShortcuts4List, Shortcut4);
			
			SysStringByIndex(ShortcutTextStringList,Shortcut1, label1, 21);
			SetControlLabel(frm, ShortcutsShortcut1PopTrigger, label1);
			
			SysStringByIndex(ShortcutTextStringList,Shortcut2, label2, 21);
			SetControlLabel(frm, ShortcutsShortcut2PopTrigger, label2);
			
			SysStringByIndex(ShortcutTextStringList,Shortcut3, label3, 21);
			SetControlLabel(frm, ShortcutsShortcut3PopTrigger, label3);
			
			SysStringByIndex(ShortcutTextStringList,Shortcut4, label4, 21);
			SetControlLabel(frm, ShortcutsShortcut4PopTrigger, label4);
			
			buttonPressed = FrmDoDialog( frm );
			Shortcut1 = GetTriggerList(frm, ShortcutsShortcuts1List);
			Shortcut2 = GetTriggerList(frm, ShortcutsShortcuts2List);
			Shortcut3 = GetTriggerList(frm, ShortcutsShortcuts3List);
			Shortcut4 = GetTriggerList(frm, ShortcutsShortcuts4List);
			FrmDeleteForm( frm );
			if (CurForm == SystemInformationForm || CurForm == WarpForm || CurForm == GalacticChartForm ||
				CurForm == ExecuteWarpForm || CurForm == BuyCargoForm || CurForm == SellCargoForm ||
				CurForm == ShipYardForm || CurForm == BuyShipForm || CurForm == BuyEquipmentForm ||
				CurForm == SellEquipmentForm || CurForm == PersonnelRosterForm ||
				CurForm == CommanderStatusForm || CurForm == BankForm || CurForm == CurrentShipForm ||
				CurForm == QuestsForm || CurForm == SpecialCargoForm)
			{
				FrmGotoForm(CurForm);
			}

			break;
			
		case MenuGameOptions:
			AutoAttack = false;
			AutoFlee = false;
			SkipOptions1 = false;

			MenuEraseStatus( 0 );
			inOpts = true;
			while (inOpts)
			{
				if (!SkipOptions1)
				{
					frm = FrmInitForm( OptionsForm );

					SetCheckBox( frm, OptionsAutoFuelCheckbox, AutoFuel );
					SetCheckBox( frm, OptionsAutoRepairCheckbox, AutoRepair );
					SetCheckBox( frm, OptionsAlwaysIgnoreTradersCheckbox, AlwaysIgnoreTraders );
					SetCheckBox( frm, OptionsAlwaysIgnorePoliceCheckbox, AlwaysIgnorePolice );
					SetCheckBox( frm, OptionsAlwaysIgnorePiratesCheckbox, AlwaysIgnorePirates );
					SetCheckBox( frm, OptionsTradeInOrbitCheckbox, AlwaysIgnoreTradeInOrbit );
					SetCheckBox( frm, OptionsReserveMoneyCheckbox, ReserveMoney );
					SetCheckBox( frm, OptionsAlwaysInfoCheckbox, AlwaysInfo );
					SetCheckBox( frm, OptionsContinuousCheckbox, Continuous );
					SetCheckBox( frm, OptionsAttackFleeingCheckbox, AttackFleeing );
				
					StrIToA( SBuf, min( 99, LeaveEmpty ) );
					AmountH = (Handle) SetField( frm, OptionsLeaveEmptyField, SBuf, 4, true );
					
					buttonPressed = FrmDoDialog( frm );

					GetField( frm, OptionsLeaveEmptyField, SBuf, AmountH );
					if (SBuf[0] == '\0')
						LeaveEmpty = 0;
					else
						LeaveEmpty = StrAToI( SBuf );

					AutoFuel = GetCheckBox( frm, OptionsAutoFuelCheckbox );
					AutoRepair = GetCheckBox( frm, OptionsAutoRepairCheckbox );
					AlwaysIgnoreTraders = GetCheckBox( frm, OptionsAlwaysIgnoreTradersCheckbox );
					AlwaysIgnorePolice = GetCheckBox( frm, OptionsAlwaysIgnorePoliceCheckbox );
					AlwaysIgnorePirates = GetCheckBox( frm, OptionsAlwaysIgnorePiratesCheckbox );
					AlwaysIgnoreTradeInOrbit = GetCheckBox( frm, OptionsTradeInOrbitCheckbox );
					ReserveMoney = GetCheckBox( frm, OptionsReserveMoneyCheckbox );
					AlwaysInfo = GetCheckBox( frm, OptionsAlwaysInfoCheckbox );
					Continuous = GetCheckBox( frm, OptionsContinuousCheckbox );
					AttackFleeing = GetCheckBox( frm, OptionsAttackFleeingCheckbox );
					
					FrmDeleteForm( frm );
				
					if (buttonPressed != OptionsOptions2Button)
					{
						inOpts = false;
					}
					
					SkipOptions1 = true;
				}
				else
				{
					SkipOptions1 = false;
					
					frm = FrmInitForm( Options2Form );

#ifdef SELECTRECT
					if (BELOW50)
					{
						FrmHideObject( frm, FrmGetObjectIndex( frm, Options2RectOnButton ) );
						FrmHideObject( frm, FrmGetObjectIndex( frm, Options2RectOffButton ) );
					}
					else if (RectangularButtonsOn)
					{
						FrmHideObject( frm, FrmGetObjectIndex( frm, Options2RectOnButton ) );
						FrmShowObject( frm, FrmGetObjectIndex( frm, Options2RectOffButton ) );
					}
					else
					{
						FrmHideObject( frm, FrmGetObjectIndex( frm, Options2RectOffButton ) );
						FrmShowObject( frm, FrmGetObjectIndex( frm, Options2RectOnButton ) );
					}
#endif
					
					SetCheckBox( frm, Options2AutoNewsPayCheckbox, NewsAutoPay );
					SetCheckBox( frm, Options2UseHWButtonsCheckbox, UseHWButtons );
					SetCheckBox( frm, Options2ShowRangeCheckbox, ShowTrackedRange );
					SetCheckBox( frm, Options2TrackAutoOffCheckbox, TrackAutoOff );
					SetCheckBox( frm, Options2TextualEncountersCheckbox, TextualEncounters );
					SetCheckBox( frm, Options2RemindLoansCheckbox, RemindLoans );
					SetCheckBox( frm, Options2SharePreferencesCheckbox, SharePreferences );
					SetCheckBox( frm, Options2IdentifyStartupCheckbox, IdentifyStartup );
					
					buttonPressed = FrmDoDialog( frm );

					NewsAutoPay = GetCheckBox( frm, Options2AutoNewsPayCheckbox );
					UseHWButtons = GetCheckBox( frm, Options2UseHWButtonsCheckbox );
					ShowTrackedRange = GetCheckBox( frm, Options2ShowRangeCheckbox );
					TrackAutoOff = GetCheckBox( frm, Options2TrackAutoOffCheckbox );
					TextualEncounters = GetCheckBox( frm, Options2TextualEncountersCheckbox );
					RemindLoans = GetCheckBox( frm, Options2RemindLoansCheckbox );
					SharePreferences = GetCheckBox( frm, Options2SharePreferencesCheckbox );
					IdentifyStartup = GetCheckBox( frm, Options2IdentifyStartupCheckbox );
						
					FrmDeleteForm( frm );

#ifdef SELECTRECT					
					if (buttonPressed == Options2RectOnButton || buttonPressed == Options2RectOffButton)
					{
						if (buttonPressed == Options2RectOnButton)
						{
							if (FrmAlert( RectangularButtonsOnAlert ) == RectangularButtonsOnYes)
							{
								RectangularButtonsOn = true;
								FrmAlert( AttemptRectangularAlert );
							}
						}
						else
							RectangularButtonsOn = false;
						SkipOptions1 = true;
					}
					else 
#endif
					if (buttonPressed != Options2Options1Button)
					{
						inOpts = false;
					}
				}
			}
			break;
			
		case MenuHelpHowToPlay:
			FrmHelp( HowToPlayString );
			break;			
			
		case MenuHelpTrading:
			FrmHelp( TradingString );
			break;			
			
		case MenuHelpTraveling:
			FrmHelp( TravellingString );
			break;			
			
		case MenuHelpShipEquipment:
			FrmHelp( ShipEquipmentString );
			break;			
			
		case MenuHelpSkills:
			FrmHelp( SkillsString );
			break;			
			
		case MenuHelpAcknowledgements:
			FrmHelp( AcknowledgementsString );
			break;			
			
		case MenuHelpFirstSteps:
			FrmHelp( FirstStepsString );
			break;			
			
		case MenuHelpHelpOnMenu:
			FrmHelp( HelpOnMenuString );
			break;			
			
		case MenuHelpHelpCurrentScreen:
			AutoAttack = false;
			AutoFlee = false;

			switch (FrmGetActiveFormID())
			{
				case BuyCargoForm:
					FrmHelp( BuyCargoString );
					break;
					
				case SellCargoForm:
					FrmHelp( SellCargoString );
					break;
					
				case ShipYardForm:
					FrmHelp( ShipYardString );
					break;
					
				case BuyShipForm:
					FrmHelp( BuyShipString );
					break;
					
				case ShiptypeInfoForm:
					FrmHelp( ShiptypeInfoString );
					break;
					
				case BuyEquipmentForm:
					FrmHelp( BuyEquipmentString );
					break;
					
				case SellEquipmentForm:
					FrmHelp( SellEquipmentString );
					break;
					
				case PersonnelRosterForm:
					FrmHelp( PersonnelRosterString );
					break;
					
				case SystemInformationForm:
					FrmHelp( SystemInformationString );
					break;
					
				case GalacticChartForm:
					FrmHelp( GalacticChartString );
					break;
					
				case WarpForm:
					FrmHelp( WarpString );
					break;
					
				case ExecuteWarpForm:
					FrmHelp( ExecuteWarpString );
					break;
					
				case CommanderStatusForm:
					FrmHelp( CommanderStatusString );
					break;
					
				case AveragePricesForm:
					FrmHelp( AveragePricesString );
					break;
					
				case BankForm:
					FrmHelp( BankString );
					break;
					
				case CurrentShipForm:
					FrmHelp( CurrentShipString );
					break;
					
				case QuestsForm:
					FrmHelp( QuestsString );
					break;
					
				case PlunderForm:
					FrmHelp( PlunderString );
					break;
					
				case EncounterForm:
					FrmHelp( EncounterString );
					break;
					
				case MainForm:
					FrmHelp( HowToPlayString );
					break;					
					
				case NewCommanderForm:
					FrmHelp( NewCommanderString );
					break;					
					
				case RetireForm:
				case UtopiaForm:
				case DestroyedForm:
					FrmHelp( RetireDestroyedUtopiaString );
					break;

				default:
					FrmHelp( MainString );
					break;
			}
			break;
			
		default:
			handled = false;
			break;
	}

	if (NewForm >= 0)
	{
		CurForm = NewForm;
		FrmGotoForm( CurForm );
	}

	return handled;
}


// *************************************************************************
// This routine loads form resources and set the event handler for the form loaded.
// *************************************************************************
Boolean AppHandleEvent( EventPtr eventP)
{
	Word formId;
	FormPtr frmP;
	int i, Dock;
	FormEventHandlerType* Set;

	Set = 0L;
	Dock = -1;

	if (eventP->eType == frmLoadEvent)
	{
		// Load the form resource.
		formId = eventP->data.frmLoad.formID;
		frmP = FrmInitForm(formId);
		FrmSetActiveForm(frmP);

		// Set the event handler for the form.  The handler of the currently
		// active form is called by FrmHandleEvent each time is receives an
		// event.
		switch (formId)
		{
			case MainForm:
				Set = MainFormHandleEvent;
				break;

			case NewCommanderForm:
				Set = NewCommanderFormHandleEvent;
				break;

			case SpecialCargoForm:
				Set = SpecialCargoFormHandleEvent;
				break;

			case SystemInformationForm:
				Set = SystemInformationFormHandleEvent;
				break;

			case GalacticChartForm:
				Set = GalacticChartFormHandleEvent;
				break;

			case WarpForm:
				Set = WarpFormHandleEvent;
				break;

			case ExecuteWarpForm:
				Set = ExecuteWarpFormHandleEvent;
				break;

			case AveragePricesForm:
				Set = AveragePricesFormHandleEvent;
				break;

			case BuyCargoForm:
				Set = BuyCargoFormHandleEvent;
				break;

			case SellCargoForm:
				Set = SellCargoFormHandleEvent;
				break;
			
			case DumpCargoForm:
				Set = DiscardCargoFormHandleEvent;
				break;

			case ShipYardForm:
				Set = ShipYardFormHandleEvent;
				break;

			case BuyShipForm:
				Set = BuyShipFormHandleEvent;
				break;

			case ShiptypeInfoForm:
				Set = ShiptypeInfoFormHandleEvent;
				break;

			case BuyEquipmentForm:
				Set = BuyEquipmentFormHandleEvent;
				break;

			case SellEquipmentForm:
				Set = SellEquipmentFormHandleEvent;
				break;

			case PersonnelRosterForm:
				Set = PersonnelRosterFormHandleEvent;
				break;

			case SpecialEventForm:
				Set = SpecialEventFormHandleEvent;
				break;

			case CommanderStatusForm:
				Set = CommanderStatusFormHandleEvent;
				break;

			case BankForm:
				Set = BankFormHandleEvent;
				break;

			case EncounterForm:
				Set = EncounterFormHandleEvent;
				break;

			case PlunderForm:
				Set = PlunderFormHandleEvent;
				break;

			case CurrentShipForm:
				Set = CurrentShipFormHandleEvent;
				break;

			case QuestsForm:
				Set = QuestsFormHandleEvent;
				break;

			case RetireForm:
				Set = RetireFormHandleEvent;
				break;

			case UtopiaForm:
				Set = UtopiaFormHandleEvent;
				break;

			case DestroyedForm:
				Set = DestroyedFormHandleEvent;
				break;
				
			case NewspaperForm:
				Set = NewspaperFormHandleEvent;
				break;
				
			default:
				break;

		}

		if (Set != 0L)
			FrmSetEventHandler( frmP, Set );

		return true;
	}
	else if (eventP->eType == menuEvent)
	{
		return DockedFormDoCommand( eventP->data.menu.itemID );
	}
	else if (eventP->eType == ctlSelectEvent)
	{
		if (eventP->data.ctlSelect.controlID >= CurForm+49 && 
			eventP->data.ctlSelect.controlID <= CurForm+53)
		{
			i = eventP->data.ctlSelect.controlID - CurForm - 49;
			
			switch (i)
			{
				case 0:
					return true;

				case 1:
					Dock = ShortcutTarget[Shortcut1];
					break;
					
				case 2:
					Dock = ShortcutTarget[Shortcut2];
					break;
					
				case 3:
					Dock = ShortcutTarget[Shortcut3];
					break;
					
				case 4:
					Dock = ShortcutTarget[Shortcut4];
					break;
			}

			if (Dock >= 0)
				return DockedFormDoCommand( Dock );

			return false;
		}
	}
	else if (eventP->eType == keyDownEvent &&
	   	!((eventP->data.keyDown.chr == chrPageUp) ||
		 (eventP->data.keyDown.chr == chrPageDown)))
	{

		// trap accidental hw button presses
		if ( (eventP->data.keyDown.chr == hard1Chr ||
			  eventP->data.keyDown.chr == hard2Chr ||
			  eventP->data.keyDown.chr == hard3Chr ||
			  eventP->data.keyDown.chr == hard4Chr) &&
			 CurForm != EncounterForm && CurForm != SystemInformationForm &&
			 CurForm != WarpForm && CurForm != GalacticChartForm && CurForm != ExecuteWarpForm &&
			 CurForm != BuyCargoForm && CurForm != SellCargoForm && CurForm != ShipYardForm &&
			 CurForm != BuyShipForm && CurForm != BuyEquipmentForm && CurForm != SellEquipmentForm &&
			 CurForm != PersonnelRosterForm && CurForm != CommanderStatusForm &&
			 CurForm != BankForm && CurForm != CurrentShipForm && CurForm != QuestsForm &&
			 CurForm != SpecialCargoForm)
			 return true;

	
		if (CurForm != EncounterForm && CurForm != MainForm && CurForm != NewCommanderForm &&
			CurForm != PlunderForm && CurForm != DestroyedForm && CurForm != RetireForm &&
			CurForm != UtopiaForm && CurForm != SpecialEventForm)
		{			
		
			switch (TOLOWER( eventP->data.keyDown.chr ))
			{
			
				// Address HW Button maps to Buy Cargo
				case hard1Chr:
					Dock = ShortcutTarget[Shortcut1];
					break;
					
				// Phone HW Button maps to Sell Cargo
				case hard2Chr:
					Dock = ShortcutTarget[Shortcut2];
					break;
					
				// To-do HW Button maps to Ship Yard
				case hard3Chr:
					Dock = ShortcutTarget[Shortcut3];
					break;
					
				// Memo HW Button maps to Short Range Chart
				case hard4Chr:
					Dock = ShortcutTarget[Shortcut4];
					break;
					
				case 'b':
					Dock = MenuCommandBuyCargo;
					break;		

				case 's':
					Dock = MenuCommandSellCargo;		
					break;

				case 'y':
					Dock = MenuCommandShipYard;		
					break;

				case 'e':
					Dock = MenuCommandBuyEquipment;		
					break;
					
				case 'q':
					Dock = MenuCommandSellEquipment;		
					break;

				case 'p':
					Dock = MenuCommandPersonnelRoster;		
					break;

				case 'k':
					Dock = MenuCommandBank;		
					break;

				case 'i':
					Dock = MenuCommandSystemInformation;		
					break;

				case 'c':
					Dock = MenuCommandCommanderStatus;		
					break;

				case 'g':
					Dock = MenuCommandGalacticChart;		
					break;

				case 'w':
					Dock = MenuCommandShortRangeChart;		
					break;
			}
		}

		if (CurForm != NewCommanderForm)
		{
			switch (TOLOWER( eventP->data.keyDown.chr ))
			{
				case 'h':
					Dock = MenuHelpHelpCurrentScreen;		
					break;

				case 'o':
					Dock = MenuGameOptions;		
					break;
					
			}
		}

		if (Dock >= 0)
			return DockedFormDoCommand( Dock );
	}
	
	return false;
}
