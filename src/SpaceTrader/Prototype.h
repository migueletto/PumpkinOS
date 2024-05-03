/***********************************************************************
 *
 * SPACE TRADER 1.2.0
 *
 * Prototype.h
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

// Prototype.h  
// All Function prototypes are defined here and can be referenced
// from any source file.  Functions are referenced from multiple 
// sources.

// Shipyard.c
extern void BuyRepairs( int _Amount );
extern Boolean ShipYardFormHandleEvent( EventPtr eventP );
extern Int32 GetHullStrength( void );

// Math.c
extern int my_sqrt( int _a );
extern Int32 SqrDistance( SOLARSYSTEM _a, SOLARSYSTEM _b );
extern Int32 RealDistance( SOLARSYSTEM _a, SOLARSYSTEM _b );
extern UInt16 Rand();
extern void RandSeed( UInt16 _seed1, UInt16 _seed2 );
extern int GetRandom2(int _maxVal);

// Money.c
extern Int32 CurrentWorth();
extern void PayInterest();

// Field.c
extern Handle SetField( FormPtr _frm, int _Nr, char* _Value, int _Size, Boolean _Focus );
extern void GetField( FormPtr _frm, int _Nr, char* _Value, Handle _AmountH );
extern void SetCheckBox( FormPtr _frm, int _Nr, Boolean _Value );
extern Boolean GetCheckBox( FormPtr _frm, int _Nr );
extern void SetTriggerList( FormPtr _frm, int _Nr, int _Index );
extern int GetTriggerList( FormPtr _frm, int _Nr);
extern void SetControlLabel( FormPtr _frm, int _Nr, Char * _Label );


// Fuel.c
extern char GetFuelTanks();
extern char GetFuel();
extern void BuyFuel( int _Amount );

// Skill.c
extern char TraderSkill( SHIP* _Sh );
extern void RecalculateBuyPrices(Byte SystemID);
extern void IncreaseRandomSkill();
extern void TonicTweakRandomSkill();
extern char FighterSkill( SHIP* _Sh );
extern char PilotSkill( SHIP* _Sh );
extern char EngineerSkill( SHIP* _Sh );
extern char AdaptDifficulty( char _Level );
extern void RecalculateSellPrices();
extern char RandomSkill();
extern Boolean HasGadget( SHIP* _Sh, char _Gg );
extern Boolean HasShield( SHIP* _Sh, char _Gg );
extern Boolean HasWeapon( SHIP* _Sh, char _Gg, Boolean _exactMatch );
extern Byte NthLowestSkill( SHIP* _Sh, Byte _n );
extern int DifficultyRange(int _low, int _high);

// Draw.c
extern void DrawShortRange( int _Index );
extern void DrawCircle( int _Xs, int _Ys, int _R );
extern void DrawGalaxy( int _Index );
extern ControlPtr RectangularButton( FormPtr _frmP, int _Nr );
extern void SBufMultiples( Int32 _Count, char* _Txt );
extern void SBufShortMultiples( Int32 _Count, char* _Txt );
extern void setFormTitle( int _formID, CharPtr _s );
extern void setCurrentWinTitle( CharPtr _s );
extern void setLabelText( FormPtr _frm, int _labelid, CharPtr _s );
extern void RectangularShortcuts( FormPtr _frm, int _First );
extern void EraseRectangle( int _x, int _y, int _ex, int _ey );
extern void DrawChars( char* _Text, int _x, int _y );
extern void DrawCharsCentered( char* _Text, int _y, Boolean _useBold );
extern int ScrollButton( EventPtr _eventP );
extern Boolean DisplayHeadline(char* _text, int* _y_coord);
extern void DisplayPage(char* _text, int _start_y);

// Cargo.c
// Note the following four functions must stay together
// These functions use QtyBuf which cannot be referenced externally
// in it's current form.
// ------------------------------------------------------------------
extern void DisplayTradeQuantity( int Line, int Qty, int FirstQtyButton );
extern Boolean PlunderFormHandleEvent( EventPtr eventP );
extern Boolean BuyCargoFormHandleEvent(EventPtr eventP);
extern Boolean SellCargoFormHandleEvent( EventPtr eventP );
// ------------------------------------------------------------------
extern int GetAmountToBuy( int Index );   // used in Traveler.c also

extern void PlunderCargo( int _Index, int _Amount );
extern int GetAmountToPlunder( int _Index );
extern void BuyCargo( int _Index, int _Amount, Boolean _DisplayInfo );
extern void SellCargo( int _Index, int _Amount, Byte Operation );
extern void DisplayTradeItemName( int _i, Byte Operation );
extern int TotalCargoBays();
extern int FilledCargoBays();
extern void DisplayPlunderCargoBays();
extern void BuyItem( char _Slots, int* _Item, Int32 _Price, char* _Name, int _ItemIndex );
extern Int32 BasePrice( char _ItemTechLevel, Int32 _Price );
extern Int32 BaseSellPrice( int _Index, Int32 _Price );
extern void DrawSellEquipment();
extern void SBufBays( void );
extern void DisplayTradeCargoBays();
extern void DisplayTradeCredits();
extern void DisplayDumpCredits();
extern Boolean HasTradeableItems (SHIP *_sh, Byte _theSystem, Byte _Operation);
extern int GetRandomTradeableItem (SHIP *_sh, Byte _Operation);


// Encounter.c
extern Boolean EncounterFormHandleEvent( EventPtr _eventP );
extern Int32 TotalWeapons( SHIP* _Sh, int _minWeapon, int _maxWeapon );        				// used in Traveler also
extern void ShowShip( SHIP* _Sh, int _offset, Boolean _commandersShip );
extern void EscapeWithPod();												// used in IncDays for Reactor meltdown

// Traveler.c
extern Boolean AveragePricesFormHandleEvent( EventPtr _eventP );
extern Boolean RetireDestroyedUtopiaFormHandleEvent( EventPtr _eventP, char _EndStatus );
extern Boolean UtopiaFormHandleEvent( EventPtr _eventP );
extern Boolean RetireFormHandleEvent( EventPtr _eventP );
extern Boolean ExecuteWarpFormHandleEvent(EventPtr _eventP);
extern Boolean AnyEmptySlots( SHIP* _ship );
extern void DoWarp(Boolean _viaSingularity ); //referenced in WarpForm for the Singularity.
extern void Travel(); 
extern Boolean StartNewGame();
extern Boolean WormholeExists( int _a, int _b );
extern void Arrival();
extern Boolean Cloaked( SHIP* _Sh, SHIP* _Opp );
extern int GetFirstEmptySlot( char _Slots, int* _Item );
extern void IncDays( int _Amount );
extern void ViewHighScores( void );                  
extern Int32 GetScore( char EndStatus, int Days, Int32 Worth, char Level ); // Traveler.c
extern Int32 InsuranceMoney();
extern Int32 ToSpend();

// ShipPrice.c
extern Int32 EnemyShipPrice( SHIP* Sh );
extern Int32 CurrentShipPriceWithoutCargo( Boolean _ForInsurance );
extern Int32 CurrentShipPrice( Boolean _ForInsurance );

// BuyShipEvent.c
extern void CreateFlea();
extern Boolean BuyShipFormHandleEvent( EventPtr _eventP );

// SystemInfoEvent.c
extern void DrawPersonnelRoster();
extern Boolean SystemInformationFormHandleEvent( EventPtr _eventP );
extern Boolean NewspaperFormHandleEvent( EventPtr _eventP );
extern void addNewsEvent(int _eventFlag);
extern void resetNewsEvents();
extern Boolean isNewsEvent(int _eventFlag);
extern void replaceNewsEvent(int _originalEventFlag, int _replacementEventFlag);
extern int latestNewsEvent();

// Merchant
Boolean Savegame( int State ); 
Boolean LoadGame( int State );
int GetBitmapWidth( BitmapPtr BmpPtr );
int GetBitmapHeight( BitmapPtr BmpPtr );

// Form Event Handlers 
extern void ShowMessage( char *str1, char* str2, char* str3, Boolean ShowAlways );
extern void ShowDumpCargo( void );
extern int OpenQuests( void );
extern Boolean MainFormHandleEvent( EventPtr _eventP );
extern Boolean NewCommanderFormHandleEvent( EventPtr _eventP);
extern Boolean QuestsFormHandleEvent( EventPtr _eventP );
extern Boolean GalacticChartFormHandleEvent( EventPtr _eventP );
extern Boolean WarpFormHandleEvent( EventPtr _eventP );
extern Boolean ExecuteWarpFormHandleEvent( EventPtr _eventP );
extern Boolean BuyEquipmentFormHandleEvent( EventPtr eventP );
extern Boolean SellEquipmentFormHandleEvent( EventPtr _eventP );
extern Boolean ShiptypeInfoFormHandleEvent( EventPtr _eventP );
extern Boolean PersonnelRosterFormHandleEvent( EventPtr _eventP );
extern Boolean SpecialEventFormHandleEvent( EventPtr _eventP );
extern Boolean CommanderStatusFormHandleEvent( EventPtr _eventP );
extern Boolean BankFormHandleEvent( EventPtr _eventP );
extern Boolean CurrentShipFormHandleEvent( EventPtr _eventP );
extern Boolean RetireFormHandleEvent( EventPtr _eventP );
extern Boolean UtopiaFormHandleEvent( EventPtr _eventP );
extern Boolean DestroyedFormHandleEvent( EventPtr _eventP );
extern Boolean AppHandleEvent( EventPtr _eventP);
extern Boolean SpecialEventFormHandleEvent( EventPtr _eventP );
extern Boolean SpecialCargoFormHandleEvent( EventPtr _eventP );
extern Boolean DiscardCargoFormHandleEvent( EventPtr _eventP );

// ----------------------------------------------------------
// Static Functions and their locations
// Function Name                             Module
// ----------------------------------------------------------  
// Err  AppStart();                       // merchant.c
// void AppStop();                        // merchant.c
// Boolean DockedFormDoCommand(Word _command); // AppHandleEvent.c

// void ShowBank( void )			      // Bank.c
// Int32 MaxLoan( void )			          // Bank.c
// void GetLoan( Int32 _Loan );            // Bank.c
// void PayBack( Int32 _Cash );            // Bank.c

// char AvailableQuarters( void )		  // SystemInfoEvent.c
// int GetForHire();					  // SystemInfoEvent.c
// void DrawMercenary( int Index, int y ) // SystemInfoEvent.c
// static int GetForHire( void )          // SystemInfoEvent.c

// Int32 TotalShields( SHIP* _Sh );        // Encounter.c
// Int32 TotalShieldStrength( SHIP* _Sh ); // Encounter.c
// void ShowShip(SHIP* _Sh, int _offset); // Encounter.c
// Boolean ExecuteAttack( SHIP* _Attacker, SHIP* _Defender, Boolean _Flees, Boolean _CommanderUnderAttack ); // Encounter.c
// int ExecuteAction( Boolean _CommanderFlees ); // Encounter.c
// void Arrested();                       // Encounter.c
// void EscapeWithPod();                  // Encounter.c
// void Scoop();                          // Encounter.c
// void EncounterButtons( void )          // Encounter.c

// void DetermineShipPrices();            // BuyShipEvent.c
// void BuyShip( int _Index );            // BuyShipEvent.c      
// void CreateShip ( int _Index );        // BuyShipEvent.c
// void NewCommanderDrawSkills();         // NewCmdrEvent.c

// void ShowCommanderStatus();            // CmdrStatusEvent.c
// void DisplaySkill( int Skill, int AdaptedSkill, FormPtr frmP, Int32 Label ) // CmdrStatusEvent.c

// void ShowAveragePrices();                      // Traveler.c
// void ShuffleStatus();				          // Traveler.c
// void DeterminePrices(Byte SystemID);	          // Traveler.c
// void GenerateOpponent( char _Opp );            // Traveler.c
// void ChangeQuantities();				          // Traveler.c
// void InitializeTradeitems( const int _Index ); // Traveler.c
// void ShowExecuteWarp( void );                  // Traveler.c
// void InitHighScores (void);			          // Traveler.c
// Err CreateEmptyHighscoreTable( void );         // Traveler.c
// Err OpenDatabase( void );                      // Traveler.c
void ClearHighScores( void );                  // Traveler.c
// Int32 WormholeTax( int a, int b )               // Traveler.c
// int MercenaryMoney();						  // Traveler.c
// extern Int32 StandardPrice( char _Good, char _Size, char _Tech, char _Government, int _Resources );  // Traveler.c
// int NextSystemWithinRange(int Current, Boolean Back); // Traveler.c

// void DrawShortRange( int Index )         // WarpFormEvent.c
// void DrawCircle( int Xs, int Ys, int R ) // WarpFormEvent.c
// void DrawGalaxy( int Index )             // WarpFormEvent.c

// int GetAmountToSell( int Index )         // Trader.c

// int GetAmountForFuel( void )				// Shipyard.c
// int GetAmountForRepairs( void )			// Shipyard.c
// void ShowShipYard();                     // Shipyard.c

// void DrawItem( char* Name, int y, Int32 Price ) // BuyEquipEvent.c

Boolean SaveGame( int State );
