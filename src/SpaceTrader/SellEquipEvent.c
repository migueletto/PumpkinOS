/***********************************************************************
 *
 * SPACE TRADER 1.2.0
 *
 * SellEquipEvent.c
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
// Handling of the events of the Sell Equipment form.
// *************************************************************************
#include "external.h"

Boolean SellEquipmentFormHandleEvent( EventPtr eventP )
{
    Boolean handled = false;
    Boolean Sale;
	int i;

	switch (eventP->eType) 
	{
		case frmOpenEvent:
			DrawSellEquipment();
			handled = true;
			break;

		case frmUpdateEvent:
			DrawSellEquipment();
			handled = true;
			break;

		case ctlSelectEvent:
			if (FrmAlert( SellItemAlert ) != SellItemYes)
			{
				handled = true;
				break;
			}
		
			Sale = true;

			if (eventP->data.ctlSelect.controlID >= SellEquipmentSell0Button &&
				eventP->data.ctlSelect.controlID < SellEquipmentSell0Button+MAXWEAPON)
			{
				Credits += WEAPONSELLPRICE( eventP->data.ctlSelect.controlID - SellEquipmentSell0Button );
				for (i=eventP->data.ctlSelect.controlID - SellEquipmentSell0Button + 1; i<MAXWEAPON; ++i)
					Ship.Weapon[i-1] = Ship.Weapon[i];
				Ship.Weapon[MAXWEAPON-1] = -1;
			}

			if (eventP->data.ctlSelect.controlID >= SellEquipmentSell0Button+MAXWEAPON &&
				eventP->data.ctlSelect.controlID < SellEquipmentSell0Button+MAXWEAPON+MAXSHIELD)
			{
				Credits += SHIELDSELLPRICE( eventP->data.ctlSelect.controlID - SellEquipmentSell0Button - MAXWEAPON );
				for (i=eventP->data.ctlSelect.controlID - SellEquipmentSell0Button - MAXWEAPON + 1; i<MAXSHIELD; ++i)
				{
					Ship.Shield[i-1] = Ship.Shield[i];
					Ship.ShieldStrength[i-1] = Ship.ShieldStrength[i];
				}
				Ship.Shield[MAXSHIELD-1] = -1;
				Ship.ShieldStrength[MAXSHIELD-1] = 0;
			}

			if (eventP->data.ctlSelect.controlID >= SellEquipmentSell0Button+MAXWEAPON+MAXSHIELD &&
				eventP->data.ctlSelect.controlID < SellEquipmentSell0Button+MAXWEAPON+MAXSHIELD+MAXGADGET)
			{
				if (Ship.Gadget[eventP->data.ctlSelect.controlID - SellEquipmentSell0Button - MAXWEAPON - MAXSHIELD] == EXTRABAYS)
				{
					if (FilledCargoBays() > TotalCargoBays() - 5)
					{
						FrmAlert( CargoBaysFullAlert );
						Sale = false;						
					}
				}
			
				if (Sale)
				{
					Credits += GADGETSELLPRICE( eventP->data.ctlSelect.controlID - SellEquipmentSell0Button - MAXWEAPON - MAXSHIELD );
					for (i=eventP->data.ctlSelect.controlID - SellEquipmentSell0Button - MAXWEAPON - MAXSHIELD + 1; i<MAXGADGET; ++i)
						Ship.Gadget[i-1] = Ship.Gadget[i];
					Ship.Gadget[MAXGADGET-1] = -1;
				}
			}

			if (Sale)
				DrawSellEquipment();

			handled = true;
			break;

		default:
			break;
	}
	
	return handled;
}
