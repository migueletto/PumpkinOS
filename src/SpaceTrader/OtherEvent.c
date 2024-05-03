/***********************************************************************
 *
 * SPACE TRADER 1.2.0
 *
 * OtherEvent.c
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
//
// Special Cargo Screen Event Handler 
//
// *************************************************************************
#include "external.h"

static Boolean MessageHasBeenShown = false;

static void DrawSpecialCargoForm()
{
    FormPtr frmP;
	int Line;
	
	frmP = FrmGetActiveForm();
	
	RectangularShortcuts( frmP, SpecialCargoBButton );
	
	FrmDrawForm( frmP );

	Line = 18;

	FntSetFont ( stdFont );			
	if (Ship.Tribbles > 0)
	{
		if (Ship.Tribbles >= MAXTRIBBLES)
			StrCopy( SBuf, "An infestation of tribbles." );
		else
		{
			StrCopy( SBuf, "" );
			SBufMultiples( Ship.Tribbles, "cute, furry tribble" );
			StrCat( SBuf, "." );
		}
		DrawChars (SBuf, 0, Line);
		Line += 14;
	}
	
	if (JaporiDiseaseStatus == 1)
	{
		DrawChars( "10 bays of antidote.", 0, Line);
		Line += 14;
	}
	if (ArtifactOnBoard)
	{
		DrawChars( "An alien artifact.", 0, Line);
		Line += 14;
	}
	if (JarekStatus == 2)
	{
		DrawChars( "A haggling computer.", 0, Line);
		Line += 14;
	}
	if (ReactorStatus > 0 && ReactorStatus < 21)
	{
		DrawChars( "An unstable reactor taking up 5 bays.", 0, Line);
		Line += 14;
		StrCopy( SBuf, "" );
		SBufMultiples( 10 - ((ReactorStatus - 1) / 2), "bay" );
		StrCat( SBuf, " of enriched fuel." );
		DrawChars (SBuf, 0, Line);
		Line += 14;
		
	}
	if (CanSuperWarp)
	{
		DrawChars( "A Portable Singularity.", 0, Line);
		Line += 14;
	}


	if (Line == 18)
	{
		DrawChars ("No special items.", 0, Line);
	}

}


Boolean SpecialCargoFormHandleEvent( EventPtr eventP )
{
    Boolean handled = false;

	switch (eventP->eType) 
	{
		case ctlSelectEvent:
			if (eventP->data.ctlSelect.controlID ==SpecialCargoStatusButton)
			{
				CurForm = CommanderStatusForm;
			}
			else if (eventP->data.ctlSelect.controlID == SpecialCargoShipButton)
			{
				CurForm = CurrentShipForm;
			}
			else if (eventP->data.ctlSelect.controlID == SpecialCargoQuestsButton)
			{
				CurForm = QuestsForm;
			}
			FrmGotoForm( CurForm );
			handled = true;
			break;

		case frmOpenEvent:
		case frmUpdateEvent:
			DrawSpecialCargoForm();
			handled = true;
			break;
			
		default:
			break;
	}
	
	return handled;
}


/***********************************************************************
 * Show message shows a general message, usually for debugging.
 ***********************************************************************/
void ShowMessage( char *str1, char* str2, char* str3, Boolean ShowAlways )
{
	if (MessageHasBeenShown && !ShowAlways)
		return;

	FrmCustomAlert( GeneralMessageAlert, str1, str2, str3 );
	
	MessageHasBeenShown = true;
	
	return;
}

