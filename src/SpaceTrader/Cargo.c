/***********************************************************************
 *
 * SPACE TRADER 1.2.0
 *
 * Cargo.c
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
// Cargo.c - Functions in this module include:
//
// int GetAmountToBuy( int Index ) 
//
// void DisplayTradeQuantity( int Line, int Qty, int FirstQtyButton )
// Boolean PlunderFormHandleEvent( EventPtr eventP )
// Boolean BuyCargoFormHandleEvent(EventPtr eventP)
// Boolean SellCargoFormHandleEvent( EventPtr eventP )
// void PlunderCargo( int Index, int Amount )
// int GetAmountToPlunder( int Index )
// void BuyCargo( int Index, int Amount, Boolean DisplayInfo )
// void SellCargo( int Index, int Amount, Byte Operation )
// int GetAmountToSell( int Index )
// void DisplayTradeItemName( int i, Byte Operation )
// int TotalCargoBays( void )
// int FilledCargoBays( void )
// void DisplayPlunderCargoBays( void )
// void BuyItem( char Slots, int* Item, Int32 Price, char* Name, int ItemIndex )
// Int32 BasePrice( char ItemTechLevel, Int32 Price )
// Int32 BaseSellPrice( int Index, Int32 Price )
// void DrawSellEquipment( void )
// void SBufBays( void )
// void DisplayTradeCargoBays( void )
// void DisplayTradeCredits( void )
//
// Static Local Functions
// ----------------------
// int GetAmountToSell( int Index )
// 
// -------------------------------------------------------------------------
// Modifications:
// mm/dd/yy - description - author
// -------------------------------------------------------------------------
// 06/30/01 -  Debt is checked in BuyCargo - SRA
// *************************************************************************

#include "external.h"

static char QtyBuf[MAXTRADEITEM][4];  // This variable cannot be moved without
									  // Doing dynamic memory allocation to a 
									  // "char*** QtyBuf" through a loop that
									  // allocates memory for each row. SRA 04/18/01

// *************************************************************************
// Let the commander indicate how many he wants to sell or dump
// Operation is SELLCARGO or DUMPCARGO
// *************************************************************************
static int GetAmountToSell( int Index, Byte Operation  )
{
	FormPtr frm;
	int d, Amount;
	Handle AmountH;

	frm = FrmInitForm( AmountToSellForm );
	
	AmountH = (Handle) SetField( frm, AmountToSellToSellField, "", 4, true );
	
	if (Operation == SELLCARGO)
	{
		StrCopy( SBuf, "Sell " );
	}
	else
	{
		StrCopy( SBuf, "Discard " );
	}
	StrCat( SBuf, Tradeitem[Index].Name );
	setFormTitle( AmountToSellForm, SBuf );

	if (Operation == SELLCARGO)
	{
		StrCopy( SBuf, "You can sell up to " );
		StrIToA( SBuf2, Ship.Cargo[Index] );
		StrCat( SBuf, SBuf2 );
		StrCat( SBuf, " at " );
		StrIToA( SBuf2, SellPrice[Index] );
		StrCat( SBuf, SBuf2 );
		StrCat( SBuf, " cr. each." );
	}
	else if (Operation == DUMPCARGO)
	{
		StrCopy( SBuf, "You can dump up to " );
		StrIToA( SBuf2, min(Ship.Cargo[Index], ToSpend()/ (5 * (Difficulty + 1))) );
		StrCat( SBuf, SBuf2 );
		StrCat( SBuf, "." );
	}
	else
	{
		StrCopy( SBuf, "You can jettison up to " );
		StrIToA( SBuf2, Ship.Cargo[Index]);
		StrCat( SBuf, SBuf2 );
		StrCat( SBuf, "." );
	}
	setLabelText( frm, AmountToSellMaxToSellLabel, SBuf );
	
	StrCopy( SBuf, "You paid about " );
	StrIToA( SBuf2, BuyingPrice[Index] / Ship.Cargo[Index] );
	StrCat( SBuf, SBuf2 );
	StrCat( SBuf, " cr. per unit." );
	setLabelText( frm, AmountToSellAveragePriceLabel, SBuf );
	
	if (Operation == SELLCARGO)
	{
		if (BuyingPrice[Index] / Ship.Cargo[Index] > SellPrice[Index])
		{
			StrCopy( SBuf, "Your loss per unit is " );
			StrIToA( SBuf2, (BuyingPrice[Index] / Ship.Cargo[Index]) - SellPrice[Index] );
			StrCat( SBuf, SBuf2 );
			StrCat( SBuf, " cr." );
			setLabelText( frm, AmountToSellAverageProfitLabel, SBuf );
		}
		else if (BuyingPrice[Index] / Ship.Cargo[Index] < SellPrice[Index])
		{
			StrCopy( SBuf, "Your profit per unit is " );
			StrIToA( SBuf2, SellPrice[Index] - (BuyingPrice[Index] / Ship.Cargo[Index]) );
			StrCat( SBuf, SBuf2 );
			StrCat( SBuf, " cr." );
			setLabelText( frm, AmountToSellAverageProfitLabel, SBuf );
		}
		else
		{
			StrCopy( SBuf, "You won't make a profit." );
			setLabelText( frm, AmountToSellAverageProfitLabel, SBuf );
		}
		setLabelText( frm, AmountToSellHowManyLabel, "How many do you want to sell?" );
	}
	else if (Operation == DUMPCARGO)
	{
		StrCopy (SBuf, "It costs ");
		StrIToA ( SBuf2, (5 * (Difficulty + 1)));
		StrCat (SBuf, SBuf2);
		StrCat (SBuf, " cr. per unit for disposal.");
		setLabelText( frm, AmountToSellAverageProfitLabel, SBuf );
		setLabelText( frm, AmountToSellHowManyLabel, "How many will you dump?" );
	}
	else
	{
		setLabelText( frm, AmountToSellAverageProfitLabel, "It costs nothing to jettison cargo." );
		setLabelText( frm, AmountToSellHowManyLabel, "How many will you dump?" );	
	}
	
		
	d = FrmDoDialog( frm );

	GetField( frm, AmountToSellToSellField, SBuf, AmountH );
	if (SBuf[0] == '\0')
		Amount = 0;
	else
		Amount = StrAToI( SBuf );

	FrmDeleteForm( frm );

	if (d == AmountToSellAllButton)
		return( 999 );
	else if (d == AmountToSellNoneButton)
		return( 0 );
	
	return( Amount );
}	

// *************************************************************************
// Determines if a given ship is carrying items that can be bought or sold
// in a specified system.
// *************************************************************************
Boolean HasTradeableItems (SHIP *sh, Byte theSystem, Byte Operation)
{
	int i;
	Boolean ret = false, thisRet;
	for (i = 0; i< MAXTRADEITEM; i++)
	{
		// trade only if trader is selling and the item has a buy price on the
		// local system, or trader is buying, and there is a sell price on the
		// local system.
		thisRet = false;
		if (sh->Cargo[i] > 0 && Operation == TRADERSELL && BuyPrice[i] > 0)
			thisRet = true;
		else if (sh->Cargo[i] > 0 && Operation == TRADERBUY && SellPrice[i] > 0)
			thisRet = true;
			
		// Criminals can only buy or sell illegal goods, Noncriminals cannot buy
		// or sell such items.
		if (PoliceRecordScore < DUBIOUSSCORE && i != FIREARMS && i != NARCOTICS)
		    thisRet = false;
		else if (PoliceRecordScore >= DUBIOUSSCORE && (i == FIREARMS || i == NARCOTICS))
		    thisRet = false;
		    
		if (thisRet)
			ret = true;
		 

	}
	
	return ret;
}

// *************************************************************************
// Returns the index of a trade good that is on a given ship that can be
// sold in a given system.
// *************************************************************************
int GetRandomTradeableItem (SHIP *sh, Byte Operation)
{
	Boolean looping = true;
	int i=0, j;
	
	while (looping && i < 10) 
	{
		j = GetRandom(MAXTRADEITEM);
		// It's not as ugly as it may look! If the ship has a particulat item, the following
		// conditions must be met for it to be tradeable:
		// if the trader is buying, there must be a valid sale price for that good on the local system
		// if the trader is selling, there must be a valid buy price for that good on the local system
		// if the player is criminal, the good must be illegal
		// if the player is not criminal, the good must be legal 
		if ( (sh->Cargo[j] > 0 && Operation == TRADERSELL && BuyPrice[j] > 0) &&
		     ((PoliceRecordScore < DUBIOUSSCORE && (j == FIREARMS || j == NARCOTICS)) ||
		      (PoliceRecordScore >= DUBIOUSSCORE && j != FIREARMS && j != NARCOTICS)) )
			looping = false;
		else if ( (sh->Cargo[j] > 0 && Operation == TRADERBUY &&  SellPrice[j] > 0)  &&
		     ((PoliceRecordScore < DUBIOUSSCORE && (j == FIREARMS || j == NARCOTICS)) ||
		      (PoliceRecordScore >= DUBIOUSSCORE && j != FIREARMS && j != NARCOTICS)) )
			looping = false;
		// alles klar?
		else
		{
			j = -1;
			i++;
		}
	}
	// if we didn't succeed in picking randomly, we'll pick sequentially. We can do this, because
	// this routine is only called if there are tradeable goods.
	if (j == -1)
	{
		j = 0;
		looping = true;
		while (looping)
		{
			// see lengthy comment above.
			if ( (((sh->Cargo[j] > 0) && (Operation == TRADERSELL) &&  (BuyPrice[j] > 0)) ||
			    ((sh->Cargo[j] > 0) && (Operation == TRADERBUY) &&  (SellPrice[j] > 0))) &&
		     	((PoliceRecordScore < DUBIOUSSCORE && (j == FIREARMS || j == NARCOTICS)) ||
		      	(PoliceRecordScore >= DUBIOUSSCORE && j != FIREARMS && j != NARCOTICS)) )
			    
			{
				looping = false;
			}
			else
			{
				j++;
				if (j == MAXTRADEITEM)
				{
					// this should never happen!
					looping = false;
				}
			}
		}
	}
	return j;
}



// *************************************************************************
// Let the commander indicate how many he wants to buy
// *************************************************************************
int GetAmountToBuy( int Index )       // used in Traveler.c also
{
	FormPtr frm;
	Int32 count;
	int d, Amount;
	Handle AmountH;

	if (BuyPrice[Index] <= 0 || CURSYSTEM.Qty[Index] <= 0)
	{
		FrmAlert( ItemNotSoldAlert );
		return( 0 );
	}

	frm = FrmInitForm( AmountToBuyForm );
	
	AmountH = (Handle) SetField( frm, AmountToBuyToBuyField, "", 4, true );

	StrCopy( SBuf, "Buy " );
	StrCat( SBuf, Tradeitem[Index].Name );
	
	setFormTitle( AmountToBuyForm, SBuf );
	
	StrCopy( SBuf, "At " );
	StrIToA( SBuf2, BuyPrice[Index] );
	StrCat( SBuf, SBuf2 );
	StrCat( SBuf, " cr. each, you can afford " );
	count = min( ToSpend() / BuyPrice[Index], CURSYSTEM.Qty[Index] );
	if (count <= 0)
		StrCat( SBuf, "none" );
	else if (count < 1000)
	{
		StrIToA( SBuf2, count );
		StrCat( SBuf, SBuf2 );
	}
	else
		StrCat( SBuf, "a lot" );	
	StrCat( SBuf, "." );
	setLabelText( frm, AmountToBuyMaxToBuyLabel, SBuf );
		
	d = FrmDoDialog( frm );

	GetField( frm, AmountToBuyToBuyField, SBuf, AmountH );

	if (SBuf[0] == '\0')
		Amount = 0;
	else
		Amount = StrAToI( SBuf );

	FrmDeleteForm( frm );

	if (d == AmountToBuyAllButton)
		return( 999 );
	else if (d == AmountToBuyNoneButton)
		return( 0 );
	
	return( Amount );
}	


// *************************************************************************
// Display quantity on trade screen button
// Cannot be moved without moving all functions that use QtyBuf to same file
// *************************************************************************
void DisplayTradeQuantity( int Line, int Qty, int FirstQtyButton )
{
	int objindex;
	ControlPtr cp;
	FormPtr frmP;		
		
	frmP = FrmGetActiveForm();
	objindex = FrmGetObjectIndex( frmP, FirstQtyButton + Line );
	cp = (ControlPtr)FrmGetObjectPtr( frmP, objindex );
	StrIToA(QtyBuf[Line], Qty );
	CtlSetLabel( cp, QtyBuf[Line] );
}

static void DrawPlunderForm()
{
    FormPtr frmP = FrmGetActiveForm();
	ControlPtr cp;
	int i;

	for (i=0; i<MAXTRADEITEM; ++i)
	{
		RectangularButton( frmP, PlunderAll0Button + i );
		cp = (ControlPtr) RectangularButton( frmP, PlunderQty0Button + i );
		
		QtyBuf[i][0] = '\0';
		CtlSetLabel( cp, QtyBuf[i] );
	}
	FrmDrawForm( frmP );
	for (i=0; i<MAXTRADEITEM; ++i)
	{
		FntSetFont( stdFont );
		DrawChars( Tradeitem[i].Name, 35, 18+i*13 );
		DisplayTradeQuantity( i, Opponent.Cargo[i], PlunderQty0Button );
	}
	DisplayPlunderCargoBays();
}

// *************************************************************************
// Handling the plundering of a trader.
// Cannot be moved without moving all functions that use QtyBuf to same file
// *************************************************************************
Boolean PlunderFormHandleEvent( EventPtr eventP )
{
    Boolean handled = false;
    FormPtr frmP = FrmGetActiveForm();
	static int narcs;
	int Amount;

	switch (eventP->eType) 
	{
		case frmOpenEvent:
			narcs = Ship.Cargo[NARCOTICS];
			DrawPlunderForm();
			handled = true;
			break;
			
		case frmUpdateEvent:
			DrawPlunderForm();
			handled = true;
			break;

		case ctlSelectEvent:
			if (eventP->data.ctlSelect.controlID >= PlunderAll0Button &&
				eventP->data.ctlSelect.controlID <= PlunderAll9Button)
				PlunderCargo( eventP->data.ctlSelect.controlID - PlunderAll0Button, 999 );
			else if (eventP->data.ctlSelect.controlID >= PlunderQty0Button &&
				eventP->data.ctlSelect.controlID <= PlunderQty9Button)
			{
				if (Opponent.Cargo[eventP->data.ctlSelect.controlID - PlunderQty0Button] <= 0)
					FrmAlert( VictimHasntGotAnyAlert );
				else
				{
					Amount = GetAmountToPlunder( eventP->data.ctlSelect.controlID -
						PlunderQty0Button );
					if (Amount > 0)						
						PlunderCargo( eventP->data.ctlSelect.controlID - PlunderQty0Button, Amount );
				}
			}
			else if (eventP->data.ctlSelect.controlID == PlunderDumpButton)
			{
				frmP = FrmInitForm( DumpCargoForm );
				FrmSetEventHandler( frmP, DiscardCargoFormHandleEvent );
				FrmDoDialog( frmP );
				FrmDeleteForm( frmP ); 
				DrawPlunderForm();
			}
			else
			{
				if (EncounterType == MARIECELESTEENCOUNTER && Ship.Cargo[NARCOTICS] > narcs)
					JustLootedMarie = true;
				Travel();
			}
			handled = true;
			break;

		default:
			break;
	}
	
	return handled;
}


static void DrawBuyCargoForm()
{
    FormPtr frmP = FrmGetActiveForm();
	int i, j;
	ControlPtr cp;
	RectangularShortcuts( frmP, BuyCargoBButton );

	for (i=0; i<MAXTRADEITEM; ++i)
	{
		RectangularButton( frmP, BuyCargoAll0Button + i );
		cp = (ControlPtr) RectangularButton( frmP, BuyCargoQty0Button + i );
		
		QtyBuf[i][0] = '\0';
		CtlSetLabel( cp, QtyBuf[i]);
		
		if (BuyPrice[i] <= 0)
		{
			FrmHideObject( frmP, FrmGetObjectIndex( frmP, BuyCargoQty0Button + i ) );
			FrmHideObject( frmP, FrmGetObjectIndex( frmP, BuyCargoAll0Button + i ) );
		}
		else
		{
			FrmShowObject( frmP, FrmGetObjectIndex( frmP, BuyCargoQty0Button + i ) );
			FrmShowObject( frmP, FrmGetObjectIndex( frmP, BuyCargoAll0Button + i ) );
		}
	}
	FrmDrawForm( frmP );
	for (i=0; i<MAXTRADEITEM; ++i)
	{
		FntSetFont( stdFont );
		DrawChars( Tradeitem[i].Name, 30, 17+i*13 );

		EraseRectangle( 117, 17+i*13, 43, 9 );
		
		StrIToA( SBuf, BuyPrice[i] );
		StrCat( SBuf, " cr." );
		j = MAXPRICEDIGITS - StrLen( SBuf );
		if (BuyPrice[i] > 0)
			DrawChars( SBuf, 139+j*5, 17+i*13 );
		else
			DrawChars( "not sold", 122, 17+i*13 );
		DisplayTradeQuantity( i, CURSYSTEM.Qty[i], BuyCargoQty0Button );
	}
	DisplayTradeCargoBays();
	DisplayTradeCredits();

}

// *************************************************************************
// Handling the events of the Buy Cargo form.
// Cannot be moved without moving all functions that use QtyBuf to same file
// *************************************************************************
Boolean BuyCargoFormHandleEvent(EventPtr eventP)
{
    Boolean handled = false;
    int Amount;

	switch (eventP->eType) 
	{
		case frmOpenEvent:
		case frmUpdateEvent:
			DrawBuyCargoForm();
			handled = true;
			break;
			
		case ctlSelectEvent:
			if (eventP->data.ctlSelect.controlID >= BuyCargoAll0Button &&
				eventP->data.ctlSelect.controlID <= BuyCargoAll9Button)
				BuyCargo( eventP->data.ctlSelect.controlID - BuyCargoAll0Button, 999, true );
			else
			{
				Amount = GetAmountToBuy( eventP->data.ctlSelect.controlID -
					BuyCargoQty0Button );
				if (Amount > 0)						
					BuyCargo( eventP->data.ctlSelect.controlID - BuyCargoQty0Button, Amount, true );
			}
			handled = true;
			break;

		default:
			break;
	}
	
	return handled;
}

static void DrawSellCargoForm()
{
    FormPtr frmP = FrmGetActiveForm();
	int i, j;
	ControlPtr cp;
	RectangularShortcuts( frmP, SellCargoBButton );
	FrmSetMenu(frmP, MenuMainMenuBar);
	
	
	for (i=0; i<MAXTRADEITEM; ++i)
	{
		cp = (ControlPtr) RectangularButton( frmP, SellCargoQty0Button + i );
		
		QtyBuf[i][0] = '\0';
		CtlSetLabel( cp, QtyBuf[i] );

		if (SellPrice[i] <= 0)
		{
			FrmHideObject( frmP, FrmGetObjectIndex( frmP, SellCargoQty0Button + i ) );
			FrmHideObject( frmP, FrmGetObjectIndex( frmP, SellCargoAll0Button + i ) );
			FrmShowObject( frmP, FrmGetObjectIndex( frmP, SellCargoDump0Button + i ) );
			RectangularButton (frmP, SellCargoDump0Button + i);
		}
		else
		{
			FrmHideObject( frmP, FrmGetObjectIndex( frmP, SellCargoDump0Button + i ) );
			FrmShowObject( frmP, FrmGetObjectIndex( frmP, SellCargoQty0Button + i ) );
			FrmShowObject( frmP, FrmGetObjectIndex( frmP, SellCargoAll0Button + i ) );
			RectangularButton( frmP, SellCargoAll0Button + i );
		}
	}
	
	FrmDrawForm( frmP );

	for (i=0; i<MAXTRADEITEM; ++i)
	{
		DisplayTradeItemName( i, SELLCARGO );

		EraseRectangle( 117, 17+i*13, 43, 9 );
		
		StrIToA( SBuf, SellPrice[i] );
		StrCat( SBuf, " cr." );
		j = MAXPRICEDIGITS - StrLen( SBuf );
		if (SellPrice[i] > 0)
			DrawChars( SBuf, 139+j*5, 17+i*13 );
		else
			DrawChars( "no trade", 119, 17+i*13 );
			
		if (SellPrice[i] > 0)
			DisplayTradeQuantity( i, Ship.Cargo[i], SellCargoQty0Button );
		else
		{
			StrIToA( SBuf, Ship.Cargo[i] );
			DrawChars( SBuf, 11-((StrLen( SBuf )*5)>>1), 17+i*13 );
		}
	}

	DisplayTradeCargoBays();
	DisplayTradeCredits();
}

// *************************************************************************
// Handling the events of the Sell Cargo form.
// Cannot be moved without moving all functions that use QtyBuf to same file
// *************************************************************************
Boolean SellCargoFormHandleEvent( EventPtr eventP )
{
    Boolean handled = false;
    int Amount;

	switch (eventP->eType) 
	{
		case frmOpenEvent:
		case frmUpdateEvent:
			DrawSellCargoForm();
			handled = true;
			break;

		case ctlSelectEvent:
			if (eventP->data.ctlSelect.controlID >= SellCargoAll0Button &&
				eventP->data.ctlSelect.controlID <= SellCargoAll9Button)
			{
				SellCargo( eventP->data.ctlSelect.controlID - SellCargoAll0Button, 999, SELLCARGO );
				DisplayTradeItemName( eventP->data.ctlSelect.controlID - SellCargoAll0Button, SELLCARGO );
			}
			else if (eventP->data.ctlSelect.controlID >= SellCargoDump0Button &&
				eventP->data.ctlSelect.controlID <= SellCargoDump9Button)
			{
				if (Ship.Cargo[eventP->data.ctlSelect.controlID - SellCargoDump0Button] <= 0)
				{
					FrmAlert( NothingToDumpAlert );
					return true;
				}
				Amount = GetAmountToSell( eventP->data.ctlSelect.controlID -
					SellCargoDump0Button, DUMPCARGO );
				if (Amount > 0)						
					SellCargo( eventP->data.ctlSelect.controlID - SellCargoDump0Button, Amount, DUMPCARGO );
				DisplayTradeItemName( eventP->data.ctlSelect.controlID - SellCargoDump0Button, DUMPCARGO );
			}
			else
			{
				if (Ship.Cargo[eventP->data.ctlSelect.controlID - SellCargoQty0Button] <= 0)
				{
					FrmAlert( NothingForSaleAlert );
					return true;
				}
				if (SellPrice[eventP->data.ctlSelect.controlID - SellCargoQty0Button] <= 0)
				{
					FrmAlert( NotInterestedAlert );
					return true;
				}
				Amount = GetAmountToSell( eventP->data.ctlSelect.controlID -
					SellCargoQty0Button, SELLCARGO );
				if (Amount > 0)						
					SellCargo( eventP->data.ctlSelect.controlID - SellCargoQty0Button, Amount, SELLCARGO );
				DisplayTradeItemName( eventP->data.ctlSelect.controlID - SellCargoQty0Button, SELLCARGO );
			}
			handled = true;
			break;

		default:
			break;
	}
	
	return handled;
}


// *************************************************************************
// Show contents of Dump cargo form.
// *************************************************************************
void ShowDumpCargo( void )
{
    FormPtr frmP = FrmGetActiveForm();
	int i;
	ControlPtr cp;

	for (i=0; i<MAXTRADEITEM; ++i)
	{
		RectangularButton( frmP, DumpCargoAll0Button + i );
		cp = (ControlPtr) RectangularButton( frmP, DumpCargoQty0Button + i );
				
		QtyBuf[i][0] = '\0';
		CtlSetLabel( cp, QtyBuf[i] );

		FrmShowObject( frmP, FrmGetObjectIndex( frmP, DumpCargoQty0Button + i ) );
		FrmShowObject( frmP, FrmGetObjectIndex( frmP, DumpCargoAll0Button + i ) );
	}

	FrmDrawForm( frmP );

	for (i=0; i<MAXTRADEITEM; ++i)
	{
		DisplayTradeItemName( i, JETTISONCARGO );

		DisplayTradeQuantity( i, Ship.Cargo[i], DumpCargoQty0Button );
	}

	DisplayPlunderCargoBays();
}

// *************************************************************************
// Handling the events of the Discard Cargo form.
// Cannot be moved without moving all functions that use QtyBuf to same file
// *************************************************************************
Boolean DiscardCargoFormHandleEvent( EventPtr eventP )
{
    Boolean handled = false;
	int Amount;
	static Boolean DontUpdate = false;

	switch (eventP->eType) 
	{
		case winEnterEvent:
			if (DontUpdate)
				DontUpdate = false;
			else
			{
				ShowDumpCargo();
				handled = true;
			}
			break;
	
		case ctlSelectEvent:
			if (eventP->data.ctlSelect.controlID >= DumpCargoAll0Button &&
				eventP->data.ctlSelect.controlID <= DumpCargoAll9Button)
			{
				if (Ship.Cargo[eventP->data.ctlSelect.controlID - DumpCargoAll0Button] <= 0)
				{
					FrmAlert( NothingToDumpAlert );
					DontUpdate = true;
					return true;
				}
				StrIToA(SBuf, BuyingPrice[eventP->data.ctlSelect.controlID - DumpCargoAll0Button]);
				if (FrmCustomAlert(DumpAllAlert,SBuf,NULL,NULL) == DumpAllYes)
				{
					SellCargo( eventP->data.ctlSelect.controlID - DumpCargoAll0Button, 999, JETTISONCARGO );
					DisplayTradeItemName( eventP->data.ctlSelect.controlID - DumpCargoAll0Button, JETTISONCARGO );
					DontUpdate = true;
				}
				handled = true;
			}
			else if (eventP->data.ctlSelect.controlID >= DumpCargoDoneButton)
			{
				// Nothing
			}
			else
			{
				if (Ship.Cargo[eventP->data.ctlSelect.controlID - DumpCargoQty0Button] <= 0)
				{
					FrmAlert( NothingToDumpAlert );
					DontUpdate = true;
					return true;
				}
				Amount = GetAmountToSell( eventP->data.ctlSelect.controlID -
					DumpCargoQty0Button, JETTISONCARGO );
				if (Amount > 0)						
					SellCargo( eventP->data.ctlSelect.controlID - DumpCargoQty0Button, Amount, JETTISONCARGO );
				DisplayTradeItemName( eventP->data.ctlSelect.controlID - DumpCargoQty0Button, JETTISONCARGO );
				DontUpdate = true;
				handled = true;
			}
			break;
			
		default:
			break;
	}
	return handled;
}


// *************************************************************************
// Plunder amount of cargo
// *************************************************************************
void PlunderCargo( int Index, int Amount )
{
	int ToPlunder;
	
	if (Opponent.Cargo[Index] <= 0)
	{
		FrmAlert( VictimHasntGotAnyAlert );
		return;
	}

	if (TotalCargoBays() - FilledCargoBays() <= 0)
	{
		FrmAlert( NoEmptyBaysAlert );
		return;
	}
	
	ToPlunder = min( Amount, Opponent.Cargo[Index] );
	ToPlunder = min( ToPlunder, TotalCargoBays() - FilledCargoBays() );
	
	Ship.Cargo[Index] += ToPlunder;
	Opponent.Cargo[Index] -= ToPlunder;
	
	DisplayTradeQuantity( Index, Opponent.Cargo[Index], PlunderQty0Button );
	DisplayPlunderCargoBays();
}


// *************************************************************************
// Let the commander indicate how many he wants to plunder
// *************************************************************************
int GetAmountToPlunder( int Index )
{
	FormPtr frm;
	int d, Amount;
	Handle AmountH;

	frm = FrmInitForm( AmountToPlunderForm );
	
	AmountH = (Handle) SetField( frm, AmountToPlunderToPlunderField, "", 4, true );
	
	StrCopy( SBuf, "Steal " );
	StrCat( SBuf, Tradeitem[Index].Name );
	setFormTitle( AmountToPlunderForm, SBuf );
	
	StrCopy( SBuf, "Your victim has " );
	StrIToA( SBuf2, Opponent.Cargo[Index] );
	StrCat( SBuf, SBuf2 );
	StrCat( SBuf, " of these goods." );
	setLabelText( frm, AmountToPlunderMaxToPlunderLabel, SBuf );
	
	d = FrmDoDialog( frm );

	GetField( frm, AmountToPlunderToPlunderField, SBuf, AmountH );
	if (SBuf[0] == '\0')
		Amount = 0;
	else
		Amount = StrAToI( SBuf );

	FrmDeleteForm( frm );

	if (d == AmountToPlunderAllButton)
		return( 999 );
	else if (d == AmountToPlunderNoneButton)
		return( 0 );
	
	return( Amount );
}	


// *************************************************************************
// Buy amount of cargo
// *************************************************************************
void BuyCargo( int Index, int Amount, Boolean DisplayInfo )
{
	int ToBuy;
	
    if (Debt > DEBTTOOLARGE)
    {
        FrmAlert( DebtTooLargeForBuyAlert );
        return;
    }
       
	if (CURSYSTEM.Qty[Index] <= 0 || BuyPrice[Index] <= 0)
	{
		FrmAlert( NothingAvailableAlert );
		return;
	}

	if (TotalCargoBays() - FilledCargoBays() - LeaveEmpty <= 0)
	{
		FrmAlert( NoEmptyBaysAlert );
		return;
	}
	
	if (ToSpend() < BuyPrice[Index] )
	{
		FrmAlert( CantAffordAlert );
		return;
	}
	
	ToBuy = min( Amount, CURSYSTEM.Qty[Index] );
	ToBuy = min( ToBuy, TotalCargoBays() - FilledCargoBays() - LeaveEmpty );
	ToBuy = min( ToBuy, ToSpend() / BuyPrice[Index] );
	
	Ship.Cargo[Index] += ToBuy;
	Credits -= ToBuy * BuyPrice[Index];
	BuyingPrice[Index] += ToBuy * BuyPrice[Index];
	CURSYSTEM.Qty[Index] -= ToBuy;
	
	if (DisplayInfo)
	{
		DisplayTradeQuantity( Index, CURSYSTEM.Qty[Index], BuyCargoQty0Button );
		DisplayTradeCargoBays();
		DisplayTradeCredits();
	}
}


// *************************************************************************
// Sell or Jettison amount of cargo
// Operation is SELLCARGO, DUMPCARGO, or JETTISONCARGO
// *************************************************************************
void SellCargo( int Index, int Amount, Byte Operation )
{
	int ToSell;
	
	if (Ship.Cargo[Index] <= 0)
	{
		if (Operation == SELLCARGO)
			FrmAlert( NothingForSaleAlert );
		else
			FrmAlert( NothingToDumpAlert );
		return;
	}

	if (SellPrice[Index] <= 0 && Operation == SELLCARGO)
	{
		FrmAlert( NotInterestedAlert );
		return;
	}

	if (Operation == JETTISONCARGO)
	{
		if (PoliceRecordScore > DUBIOUSSCORE && !LitterWarning)
		{
			LitterWarning = true;
			if (FrmAlert( SpaceLitteringAlert ) == SpaceLitteringNo)
				return;
		}
	}

	ToSell = min( Amount, Ship.Cargo[Index] );
	
	if (Operation == DUMPCARGO)
	{
		ToSell = min(ToSell, ToSpend() / 5 * (Difficulty + 1));
	}
	
	BuyingPrice[Index] = (BuyingPrice[Index] * (Ship.Cargo[Index] - ToSell)) / Ship.Cargo[Index];
	Ship.Cargo[Index] -= ToSell;
	if (Operation == SELLCARGO)
		Credits += ToSell * SellPrice[Index];
	if (Operation == DUMPCARGO)
		Credits -= ToSell * 5 * (Difficulty + 1);
	if (Operation == JETTISONCARGO)
	{
		if (GetRandom( 10 ) < Difficulty + 1)
		{
			if (PoliceRecordScore > DUBIOUSSCORE)
				PoliceRecordScore = DUBIOUSSCORE;
			else
				--PoliceRecordScore;
			addNewsEvent(CAUGHTLITTERING);
		}
	}
	
	if (Operation == SELLCARGO || Operation == DUMPCARGO)
	{
		// DisplayTradeQuantity( Index, Ship.Cargo[Index],	SellCargoQty0Button );

		if (SellPrice[Index] > 0)
			DisplayTradeQuantity( Index, Ship.Cargo[Index], SellCargoQty0Button );
		else
		{
			StrIToA( SBuf, Ship.Cargo[Index] );
			DrawChars( SBuf, 11-((StrLen( SBuf )*5)>>1), 17+Index*13 );
		}
		DisplayTradeCargoBays();
		DisplayTradeCredits();
	}
	else
	{
		DisplayTradeQuantity( Index, Ship.Cargo[Index],	DumpCargoQty0Button );
		DisplayPlunderCargoBays();
	}
}


// *************************************************************************
// Display the name of a trade item on the sell cargo screen
// Operation is either SELLCARGO, DUMPCARGO, or JETTISONCARGO
// *************************************************************************
void DisplayTradeItemName( int i, Byte Operation )
{
	if (Ship.Cargo[i] > 0 && SellPrice[i] > BuyingPrice[i] / Ship.Cargo[i] &&
		Operation == SELLCARGO)
		FntSetFont( boldFont );
	else
		FntSetFont( stdFont );
	if (Operation == SELLCARGO)
	{
		EraseRectangle( 30, 17+i*13, 50, 12 );
		DrawChars( Tradeitem[i].Name, 30, 17+i*13 );
	}
	else if (Operation == JETTISONCARGO)
	{
		DrawChars( Tradeitem[i].Name, 35, 18+i * 13);
	}
	else
	{
		//EraseRectangle( 30, 18+i*13, 56, 12 );
		DrawChars( Tradeitem[i].Name, 30, 18+i*13 );
	}
	FntSetFont( stdFont );
}
	

// *************************************************************************
// Calculate total cargo bays
// *************************************************************************
int TotalCargoBays( void )
{
	int Bays;
	int i;
	
	Bays = Shiptype[Ship.Type].CargoBays;
	for (i=0; i<MAXGADGET; ++i)
		if (Ship.Gadget[i] == EXTRABAYS)
			Bays += 5;
	if (JaporiDiseaseStatus == 1)
		Bays -= 10;
	// since the quest ends when the reactor
	if (ReactorStatus > 0 && ReactorStatus < 21)
		Bays -= (5 + 10 - (ReactorStatus - 1)/2);
	
	return Bays;
}


// *************************************************************************
// Calculate total filled cargo bays
// *************************************************************************
int FilledCargoBays( void )
{
	int sum, i;

	sum = 0;
	for (i=0; i<MAXTRADEITEM; ++i)
		sum = sum + Ship.Cargo[i];

	return sum;
}


// *************************************************************************
// Display cargo bays on plunder screen
// *************************************************************************
void DisplayPlunderCargoBays( void )
{
	EraseRectangle( 118, 32, 155, 39 );

	SBufBays();
	FntSetFont( stdFont );
	DrawChars( SBuf, 118, 32 );
}


// *************************************************************************
// Buy an item: Slots is the number of slots, Item is the array in the
// Ship record which contains the item type, Price is the costs,
// Name is the name of the item and ItemIndex is the item type number
// *************************************************************************
void BuyItem( char Slots, int* Item, Int32 Price, char* Name, int ItemIndex )
{
	int FirstEmptySlot;
	FormPtr frmP;

	FirstEmptySlot = GetFirstEmptySlot( Slots, Item );

	if (Price <= 0)
		FrmAlert( ItemNotSoldAlert );
	else if (Debt > 0)
		FrmAlert( YoureInDebtAlert );
	else if (Price > ToSpend())
		FrmAlert( CantBuyItemAlert );
	else if (FirstEmptySlot < 0)
		FrmAlert( NotEnoughSlotsAlert );
	else
	{
		frmP = FrmInitForm( BuyItemForm );

		StrCopy( SBuf, "Buy " );
		StrCat( SBuf, Name );
		setFormTitle( BuyItemForm, SBuf );
					
		StrIToA( SBuf, Price );
		StrCat( SBuf, " credits?" );
		setLabelText( frmP, BuyItemPriceLabel, SBuf );
	
		if (FrmDoDialog( frmP ) == BuyItemYesButton)
		{
			Item[FirstEmptySlot] = ItemIndex;
			Credits -= Price;
		}
					
		FrmDeleteForm( frmP );

		DisplayTradeCredits();
	}
}


// *************************************************************************
// Determine base price of item
// *************************************************************************
Int32 BasePrice( char ItemTechLevel, Int32 Price )
{
	return ((ItemTechLevel > CURSYSTEM.TechLevel) ? 0 : 
		((Price * (100 - TraderSkill( &Ship ))) / 100));
}

// *************************************************************************
// Determine selling price
// *************************************************************************
Int32 BaseSellPrice( int Index, Int32 Price )
{
	return (Index >= 0 ? ((Price * 3) / 4) : 0);
}

// *************************************************************************
// Drawing the Sell Equipment screen.
// *************************************************************************
void DrawSellEquipment( void )
{
	FormPtr frmP;
	RectangleType a;
	int i, j;

	frmP = FrmGetActiveForm();

	RectangularShortcuts( frmP, SellEquipmentBButton );

	for (i=0; i<MAXWEAPON+MAXSHIELD+MAXGADGET; ++i)
		RectangularButton( frmP, SellEquipmentSell0Button + i );

	FrmDrawForm( frmP );

	EraseRectangle( 30, 18, 130, 142 );

	a.topLeft.x = 0;
	a.topLeft.y = BOUNDSY + 38;
	a.extent.x = 160;
	a.extent.y = 2;
	WinDrawRectangle( &a, 0 );
			
	a.topLeft.y = BOUNDSY + 83;
	WinDrawRectangle( &a, 0 );

	if (Ship.Weapon[0] < 0)
		DrawChars( "No weapons", 30, 30 );
			
	for (i=0; i<MAXWEAPON; ++i)
	{
		if (Ship.Weapon[i] < 0)
		{
			FrmHideObject( frmP, FrmGetObjectIndex( frmP, SellEquipmentSell0Button + i ) );
			continue;
		}
	
		FrmShowObject( frmP, FrmGetObjectIndex( frmP, SellEquipmentSell0Button + i ) );
		
		FntSetFont( stdFont );
		DrawChars( Weapontype[Ship.Weapon[i]].Name, 30, 17+i*13 );

		StrIToA( SBuf, WEAPONSELLPRICE( i ) );
		StrCat( SBuf, " cr." );
		j = MAXDIGITS - StrLen( SBuf );
		DrawChars( SBuf, 124+j*5, 17+i*13 );
	}

	if (Ship.Shield[0] < 0)
		DrawChars( "No shields", 30, 75 );
			
	for (i=0; i<MAXSHIELD; ++i)
	{
		if (Ship.Shield[i] < 0)
		{
			FrmHideObject( frmP, FrmGetObjectIndex( frmP, SellEquipmentSell0Button + MAXWEAPON + i ) );
			continue;
		}
	
		FrmShowObject( frmP, FrmGetObjectIndex( frmP, SellEquipmentSell0Button + MAXWEAPON + i ) );
		
		FntSetFont( stdFont );
		DrawChars( Shieldtype[Ship.Shield[i]].Name, 30, 62+i*13 );

		StrIToA( SBuf, SHIELDSELLPRICE( i ) );
		StrCat( SBuf, " cr." );
		j = MAXDIGITS - StrLen( SBuf );
		DrawChars( SBuf, 124+j*5, 62+i*13 );
	}

	if (Ship.Gadget[0] < 0)
		DrawChars( "No gadgets", 30, 120 );
			
	for (i=0; i<MAXGADGET; ++i)
	{
		if (Ship.Gadget[i] < 0)
		{
			FrmHideObject( frmP, FrmGetObjectIndex( frmP, SellEquipmentSell0Button + MAXWEAPON + MAXSHIELD + i ) );
			continue;
		}
	
		FrmShowObject( frmP, FrmGetObjectIndex( frmP, SellEquipmentSell0Button + MAXWEAPON + MAXSHIELD + i ) );
		
		FntSetFont( stdFont );
		DrawChars( Gadgettype[Ship.Gadget[i]].Name, 30, 107+i*13 );

		StrIToA( SBuf, GADGETSELLPRICE( i ) );
		StrCat( SBuf, " cr." );
		j = MAXDIGITS - StrLen( SBuf );
		DrawChars( SBuf, 124+j*5, 107+i*13 );
	}

	DisplayTradeCredits();
}


// *************************************************************************
// Create cargo bay status filled/total
// *************************************************************************
void SBufBays( void )
{
    StrIToA( SBuf, 	FilledCargoBays() );
    StrCat( SBuf, "/" );
    StrIToA( SBuf2, TotalCargoBays() );
    StrCat( SBuf, SBuf2 );
}	


// *************************************************************************
// Display cargo bays on trade screen
// *************************************************************************
void DisplayTradeCargoBays( void )
{
	EraseRectangle( 0, 150, 62, 9 );
	
	SBufBays();
	StrCopy( SBuf2, "Bays:  " );
	StrCat( SBuf2, SBuf );
	
	FntSetFont( stdFont );
	DrawChars( SBuf2, 0, 150 );
}


// *************************************************************************
// Display credits on trade screen
// *************************************************************************
void DisplayTradeCredits( void )
{
	int j;
	
	EraseRectangle( 78, 150, 83, 9 );
	
	StrIToA( SBuf, Credits );
	j = MAXDIGITS - StrLen( SBuf );
	StrCat( SBuf, " cr." );
	StrCopy( SBuf2, "Cash: " );
	StrCat( SBuf2, SBuf );

	FntSetFont( stdFont );
	DrawChars( SBuf2, 81 + j*5, 150 );
}

// *************************************************************************
// Display credits on Dump (or Plunder?) screen
// *************************************************************************
void DisplayDumpCredits( void )
{
	StrIToA( SBuf, Credits );
	StrCat( SBuf, " cr." );

	FntSetFont( stdFont );
	DrawChars( SBuf, 118, 64 );
}
