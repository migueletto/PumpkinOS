/***********************************************************************
 *
 * SPACE TRADER 1.2.0
 *
 * QuestEvent.c
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
// Quests Screen Event Handler 
//
// *************************************************************************
#include "external.h"

// Returns number of open quests.
int OpenQuests( void )
{
	int r = 0;

	if (MonsterStatus == 1)
		++r;

	if (DragonflyStatus >= 1 && DragonflyStatus <= 4)
		++r;
	else if (SolarSystem[ZALKONSYSTEM].Special == INSTALLLIGHTNINGSHIELD)
		++r;

	if (JaporiDiseaseStatus == 1)
		++r;

	if (ArtifactOnBoard)
		++r;

	if (WildStatus == 1)
		++r;

	if (JarekStatus == 1)
		++r;

	if (InvasionStatus >= 1 && InvasionStatus < 7)
		++r;
	else if (SolarSystem[GEMULONSYSTEM].Special == GETFUELCOMPACTOR)
		++r;

	if (ExperimentStatus >= 1 && ExperimentStatus < 11)
		++r;

	if (ReactorStatus >= 1 && ReactorStatus < 21)
		++r;

	if (SolarSystem[NIXSYSTEM].Special == GETSPECIALLASER)
		++r;

	if (ScarabStatus == 1)
		++r;
			
	if (Ship.Tribbles > 0)
		++r;
			
	if (MoonBought)
		++r;
		
	return r;
}

static void DrawQuestsForm()
{
    FormPtr frmP;
	int Line;
	frmP = FrmGetActiveForm();
	
	RectangularShortcuts( frmP, QuestsBButton );
	
	FrmDrawForm( frmP );

	FntSetFont( stdFont );

	Line = 18;

	if (MonsterStatus == 1)
	{
		DrawChars( "Kill the space monster at Acamar.", 0, Line );
		Line += 16;
	}

	if (DragonflyStatus >= 1 && DragonflyStatus <= 4)
	{
		StrCopy( SBuf, "Follow the Dragonfly to " );
		if (DragonflyStatus == 1)
			StrCat( SBuf, "Baratas." );
		else if (DragonflyStatus == 2)
			StrCat( SBuf, "Melina." );
		else if (DragonflyStatus == 3)
			StrCat( SBuf, "Regulas." );
		else if (DragonflyStatus == 4)
			StrCat( SBuf, "Zalkon." );
		DrawChars( SBuf, 0, Line );			
		Line += 16;
	}
	else if (SolarSystem[ZALKONSYSTEM].Special == INSTALLLIGHTNINGSHIELD)
	{
		DrawChars( "Get your lightning shield at Zalkon.", 0, Line );
		Line += 16;
	}
	
	if (JaporiDiseaseStatus == 1)
	{
		DrawChars( "Deliver antidote to Japori.", 0, Line );
		Line += 16;
	}

	if (ArtifactOnBoard)
	{
		DrawChars( "Deliver the alien artifact to professor", 0, Line );
		Line += 12;
		DrawChars( "Berger at some hi-tech system.", 0, Line );			
		Line += 16;
	}
	if (WildStatus == 1)
	{
		DrawChars( "Smuggle Jonathan Wild to Kravat.", 0, Line );
		Line += 16;
	}

	if (JarekStatus == 1)
	{
		DrawChars( "Bring ambassador Jarek to Devidia.", 0, Line );
		Line += 16;
	}

	// I changed this, and the reused the code in the Experiment quest.
	// I think it makes more sense to display the time remaining in
	// this fashion. SjG 10 July 2002
	if (InvasionStatus >= 1 && InvasionStatus < 7)
	{
		DrawChars( "Inform Gemulon about alien invasion", 0, Line );
		Line += 12;
		if (InvasionStatus == 6)
		    StrCopy( SBuf, "by tomorrow" );
		else
		{
		    StrCopy( SBuf, "within " );
		    SBufMultiples( 7 - InvasionStatus, "day" );
		}
		StrCat( SBuf, "." );
		DrawChars( SBuf, 0, Line );
		Line += 16;
	}
	else if (SolarSystem[GEMULONSYSTEM].Special == GETFUELCOMPACTOR)
	{
		DrawChars( "Get your fuel compactor at Gemulon.", 0, Line );
		Line += 16;
	}

	if (ExperimentStatus >= 1 && ExperimentStatus < 11)
	{
		DrawChars( "Stop Dr. Fehler's experiment at Daled", 0, Line );
		Line += 12;
		if (ExperimentStatus == 10)
		    StrCopy( SBuf, "by tomorrow" );
		else
		{
		    StrCopy( SBuf, "within " );
		    SBufMultiples( 11 - ExperimentStatus, "day" );
		}
		StrCat( SBuf, "." );
		DrawChars( SBuf, 0, Line );
		Line += 16;
	}

	if (ReactorStatus >= 1 && ReactorStatus < 21)
	{
		DrawChars( "Deliver the unstable reactor to Nix", 0, Line );
		Line += 12;
		if (ReactorStatus < 2)
		{
			DrawChars( "for Henry Morgan.", 0, Line );
		}
		else
		{
			DrawChars( "before it consumes all its fuel.", 0, Line );
		}
		Line += 16;
	}

	if (SolarSystem[NIXSYSTEM].Special == GETSPECIALLASER)
	{
		DrawChars( "Get your special laser at Nix.", 0, Line );
		Line += 16;
	}

	if (ScarabStatus == 1)
	{
		DrawChars( "Find and destroy the Scarab (which", 0, Line );
		Line += 12;
		DrawChars( "is hiding at the exit to a wormhole).", 0, Line );
		Line += 16;
	}
	
	if (Ship.Tribbles > 0)
	{
		DrawChars( "Get rid of those pesky tribbles.", 0, Line );
		Line += 16;
	}
	
	if (MoonBought)
	{
		DrawChars( "Claim your moon at Utopia.", 0, Line );			
		Line += 16;
	}

	if (Line <= 18)
		DrawChars( "There are no open quests.", 0, Line );
}

Boolean QuestsFormHandleEvent( EventPtr eventP )
{
    Boolean handled = false;

	switch (eventP->eType) 
	{
		case frmOpenEvent:
		case frmUpdateEvent:
			DrawQuestsForm();
			handled = true;
			break;
			

		case ctlSelectEvent:
			if (eventP->data.ctlSelect.controlID == QuestsStatusButton)
			{
				CurForm = CommanderStatusForm;
			}
			else if (eventP->data.ctlSelect.controlID == QuestsShipButton)
			{
				CurForm = CurrentShipForm;
			}
			else if (eventP->data.ctlSelect.controlID == QuestsSpecialButton)
			{
				CurForm = SpecialCargoForm;
			}
			
			FrmGotoForm( CurForm );
			handled = true;
			break;

		default:
			break;
	}
	
	return handled;
}

