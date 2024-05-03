/***********************************************************************
 *
 * SPACE TRADER 1.2.0
 *
 * BuyEquipEvent.c
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
// Draw an item on the screen
// *************************************************************************
static void DrawItem( char* Name, int y, Int32 Price )
{
	int j;

	FntSetFont( stdFont );
	DrawChars( Name, 30, y );

	EraseRectangle( 110, y, 56, 9 );
	StrIToA( SBuf, Price );
	StrCat( SBuf, " cr." );
	
	j = MAXDIGITS - StrLen( SBuf );
	if (Price > 0)
		DrawChars( SBuf, 124+j*5, y );
	else
		DrawChars( "not sold", 122, y );
}

static void DrawBuyEquipmentForm()
{
    FormPtr frmP = FrmGetActiveForm();
	int i;
	RectangularShortcuts( frmP, BuyEquipmentBButton );
	
	for (i=0; i<MAXWEAPONTYPE+MAXSHIELDTYPE+MAXGADGETTYPE; ++i)
	{
		RectangularButton( frmP, BuyEquipmentBuy0Button + i );
		
		if (i < MAXWEAPONTYPE)
		{
			if (BASEWEAPONPRICE( i ) <= 0)
				FrmHideObject( frmP, FrmGetObjectIndex( frmP, BuyEquipmentBuy0Button + i ) );
			else
				FrmShowObject( frmP, FrmGetObjectIndex( frmP, BuyEquipmentBuy0Button + i ) );
		}	
		else if (i < MAXWEAPONTYPE + MAXSHIELDTYPE)
		{
			if (BASESHIELDPRICE( i-MAXWEAPONTYPE ) <= 0)
				FrmHideObject( frmP, FrmGetObjectIndex( frmP, BuyEquipmentBuy0Button + i ) );
			else
				FrmShowObject( frmP, FrmGetObjectIndex( frmP, BuyEquipmentBuy0Button + i ) );
		}	
		else
		{
			if (BASEGADGETPRICE( i-MAXWEAPONTYPE-MAXSHIELDTYPE ) <= 0)
				FrmHideObject( frmP, FrmGetObjectIndex( frmP, BuyEquipmentBuy0Button + i ) );
			else
				FrmShowObject( frmP, FrmGetObjectIndex( frmP, BuyEquipmentBuy0Button + i ) );
		}
	}
	
	FrmDrawForm( frmP );
	
	for (i=0; i<MAXWEAPONTYPE; ++i)
		DrawItem( Weapontype[i].Name, 17+i*13, BASEWEAPONPRICE( i ) );

	for (i=0; i<MAXSHIELDTYPE; ++i)
		DrawItem( Shieldtype[i].Name, 17+(i+MAXWEAPONTYPE)*13, BASESHIELDPRICE( i ) );

	for (i=0; i<MAXGADGETTYPE; ++i)
		DrawItem( Gadgettype[i].Name, 17+(i+MAXWEAPONTYPE+MAXSHIELDTYPE)*13, BASEGADGETPRICE( i ) );

	DisplayTradeCredits();
}

// *************************************************************************
// Handling of the events of the Buy Equipment form.
// *************************************************************************
Boolean BuyEquipmentFormHandleEvent( EventPtr eventP )
{
    Boolean handled = false;

	switch (eventP->eType) 
	{
		case frmOpenEvent:
		case frmUpdateEvent:
			DrawBuyEquipmentForm();
			handled = true;
			break;

		case ctlSelectEvent:
			if (eventP->data.ctlSelect.controlID >= BuyEquipmentBuy0Button &&
				eventP->data.ctlSelect.controlID < BuyEquipmentBuy0Button+MAXWEAPONTYPE)
			{
				BuyItem( Shiptype[Ship.Type].WeaponSlots, 
					Ship.Weapon, 
					BASEWEAPONPRICE( eventP->data.ctlSelect.controlID - BuyEquipmentBuy0Button ), 
					Weapontype[eventP->data.ctlSelect.controlID - BuyEquipmentBuy0Button].Name, 
					eventP->data.ctlSelect.controlID - BuyEquipmentBuy0Button );
			}

			if (eventP->data.ctlSelect.controlID >= BuyEquipmentBuy0Button+MAXWEAPONTYPE &&
				eventP->data.ctlSelect.controlID < BuyEquipmentBuy0Button+MAXWEAPONTYPE+MAXSHIELDTYPE)
			{
				BuyItem( Shiptype[Ship.Type].ShieldSlots, 
					Ship.Shield, 
					BASESHIELDPRICE( eventP->data.ctlSelect.controlID - BuyEquipmentBuy0Button - MAXWEAPONTYPE ), 
					Shieldtype[eventP->data.ctlSelect.controlID - BuyEquipmentBuy0Button - MAXWEAPONTYPE].Name, 
					eventP->data.ctlSelect.controlID - BuyEquipmentBuy0Button - MAXWEAPONTYPE );
			}

			if (eventP->data.ctlSelect.controlID >= BuyEquipmentBuy0Button+MAXWEAPONTYPE+MAXSHIELDTYPE &&
				eventP->data.ctlSelect.controlID < BuyEquipmentBuy0Button+MAXWEAPONTYPE+MAXSHIELDTYPE+MAXGADGETTYPE )
			{
				if (HasGadget( &Ship, eventP->data.ctlSelect.controlID - BuyEquipmentBuy0Button - MAXWEAPONTYPE - MAXSHIELDTYPE ) &&
					EXTRABAYS != eventP->data.ctlSelect.controlID - BuyEquipmentBuy0Button - MAXWEAPONTYPE - MAXSHIELDTYPE)
					FrmAlert( NoMoreOfItemAlert );
				else
				{
					BuyItem( Shiptype[Ship.Type].GadgetSlots, 
						Ship.Gadget, 
						BASEGADGETPRICE( eventP->data.ctlSelect.controlID - BuyEquipmentBuy0Button - MAXWEAPONTYPE - MAXSHIELDTYPE ), 
						Gadgettype[eventP->data.ctlSelect.controlID - BuyEquipmentBuy0Button - MAXWEAPONTYPE - MAXSHIELDTYPE].Name, 
						eventP->data.ctlSelect.controlID - BuyEquipmentBuy0Button - MAXWEAPONTYPE - MAXSHIELDTYPE );
				}
			}

			handled = true;
			break;

		default:
			break;
	}
	
	return handled;
}
