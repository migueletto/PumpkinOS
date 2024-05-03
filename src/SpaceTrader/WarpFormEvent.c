/***********************************************************************
 *
 * SPACE TRADER 1.2.0
 *
 * WarpFormEvent.c
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
// To draw a circle with centre Xs, Ys and radius R.
// *************************************************************************
void DrawCircle( int Xs, int Ys, int R )
{
	int Xp, Yp, i, Xt, Yt;
	
	Xp = Xs;
	Yp = Ys - R;
	for (i=0; i<=R; ++i)
	{
		Xt = Xs + i;
		Yt = Ys - my_sqrt( SQR( R ) - SQR( i ) );
		WinDrawLine( Xp, Yp, Xt, Yt );
		Xp = Xt;
		Yp = Yt;
	}
	Xp = Xs;
	Yp = Ys + R;
	for (i=0; i<=R; ++i)
	{
		Xt = Xs + i;
		Yt = Ys + my_sqrt( SQR( R ) - SQR( i ) );
		WinDrawLine( Xp, Yp, Xt, Yt );
		Xp = Xt;
		Yp = Yt;
	}
	Xp = Xs;
	Yp = Ys + R;
	for (i=0; i<=R; ++i)
	{
		Xt = Xs - i;
		Yt = Ys + my_sqrt( SQR( R ) - SQR( i ) );
		WinDrawLine( Xp, Yp, Xt, Yt );
		Xp = Xt;
		Yp = Yt;
	}
	Xp = Xs;
	Yp = Ys - R;
	for (i=0; i<=R; ++i)
	{
		Xt = Xs - i;
		Yt = Ys - my_sqrt( SQR( R ) - SQR( i ) );
		WinDrawLine( Xp, Yp, Xt, Yt );
		Xp = Xt;
		Yp = Yt;
	}
}

// *************************************************************************
// Draw the short range chart
// *************************************************************************
void DrawShortRange( int Index )
{
    FormPtr frmP;
	RectangleType bounds;
	int i, j, Xs, Ys, Xp, Yp, delta;
	int dX, dY, dX3, dY3, distToTracked;

	frmP = FrmGetActiveForm();

	RectangularShortcuts( frmP, WarpBButton );

	FrmDrawForm ( frmP);

	// Rectangle for chart
	bounds.topLeft.x = 0;
	bounds.topLeft.y = BOUNDSY - EXTRAERASE;
	bounds.extent.x = 160;
	bounds.extent.y = 160 - BOUNDSY + EXTRAERASE;
	WinEraseRectangle( &bounds, 0 );

	WinSetClip( &bounds );
	
	// Centre of chart
	Xs = (int)((SHORTRANGEWIDTH >> 1) + SHORTRANGEBOUNDSX);
	Ys = (int)((SHORTRANGEHEIGHT >> 1) + BOUNDSY);
	delta = (SHORTRANGEWIDTH / (MAXRANGE << 1));

	FntSetFont( stdFont );
	
	// Draw the maximum range circle
	if (GetFuel() > 0)
		DrawCircle( Xs, Ys, GetFuel()*delta );

	// show the tracked system (if any)
	if (TrackedSystem >= 0)
	{
		distToTracked = RealDistance(SolarSystem[COMMANDER.CurSystem], SolarSystem[TrackedSystem]);
		if (distToTracked > 0)
		{		
		
			dX = (int)(25.0 * ((float)SolarSystem[COMMANDER.CurSystem].X - (float)SolarSystem[TrackedSystem].X) / 
				(float)distToTracked);
			dY = (int)(25.0 * ((float)SolarSystem[COMMANDER.CurSystem].Y - (float)SolarSystem[TrackedSystem].Y) / 
				(float)distToTracked);
				
			// draw directional arrow from planet -- I'd do this in color if it were easier.
				
			dY3 = -(int)(4.0 * ((float)SolarSystem[COMMANDER.CurSystem].X - (float)SolarSystem[TrackedSystem].X) / 
				(float)distToTracked);		
			dX3 = (int)(4.0 * ((float)SolarSystem[COMMANDER.CurSystem].Y - (float)SolarSystem[TrackedSystem].Y) / 
				(float)distToTracked);

			WinDrawLine( Xs - dX, Ys - dY, Xs - dX3, Ys - dY3 );
			WinDrawLine( Xs - dX, Ys - dY, Xs + dX3, Ys + dY3 );			
		}
	}


	// Two loops: first draw the names and then the systems. The names may
	// overlap and the systems may be drawn on the names, but at least every
	// system is visible.
	for (j=0; j<2; ++j)
	{
		for (i=0; i<MAXSOLARSYSTEM; ++i)
		{
			if ((ABS( SolarSystem[i].X - SolarSystem[Index].X ) <= MAXRANGE) &&
				(ABS( SolarSystem[i].Y - SolarSystem[Index].Y ) <= MAXRANGE))
			{
				Xp = (int)((SHORTRANGEWIDTH >> 1) + 
					(SolarSystem[i].X - SolarSystem[Index].X) * 
					(SHORTRANGEWIDTH / (MAXRANGE << 1)) +
					SHORTRANGEBOUNDSX - EXTRAERASE);
				Yp = (int)((SHORTRANGEHEIGHT >> 1) + 
					(SolarSystem[i].Y - SolarSystem[Index].Y) * 
					(SHORTRANGEHEIGHT / (MAXRANGE << 1)) +
					BOUNDSY - EXTRAERASE);
				if (j == 1)
				{
					if (i == WarpSystem)
					{
						WinDrawLine( Xp-2, Yp+3, Xp+8, Yp+3 );
						WinDrawLine( Xp+3, Yp-2, Xp+3, Yp+8 );
					}
					WinDrawBitmap( (SolarSystem[i].Visited ? VisitedShortRangeSystemBmpPtr :
						ShortRangeSystemBmpPtr), Xp, Yp );
					if (WormholeExists( i, -1 ))
					{
						delta = WORMHOLEDISTANCE * (SHORTRANGEWIDTH / (MAXRANGE << 1));
						if (WormholeExists( i, WarpSystem ))
						{
							WinDrawLine( Xp-2+delta, Yp+3, Xp+8+delta, Yp+3 );
							WinDrawLine( Xp+3+delta, Yp-2, Xp+3+delta, Yp+8 );
						}
						WinDrawBitmap( WormholeBmpPtr, Xp + delta, Yp );
					}
				}
				else
				{
					DrawChars( SolarSystemName[SolarSystem[i].NameIndex], 
						Xp - StrLen( SolarSystemName[SolarSystem[i].NameIndex] ) * 2, 
						Yp - 12 );
				}
			}
		}
	}
	
   // if they're tracking, and they want range info:
	if (TrackedSystem >= 0 && ShowTrackedRange)
	{
		StrPrintF(SBuf, "%d", distToTracked);
		StrCat(SBuf, " parsecs to ");
		StrCat(SBuf, SolarSystemName[SolarSystem[TrackedSystem].NameIndex]);
		DrawChars( SBuf, 0, 149);
	}
   
   WinResetClip();
}

// *************************************************************************
// Draw the galactic chart, with system Index selected.
// *************************************************************************
void DrawGalaxy( int Index )
{
    FormPtr frmP;
	RectangleType bounds;
	int i;

	frmP = FrmGetActiveForm();

	RectangularShortcuts( frmP, GalacticChartBButton );

	FrmDrawForm ( frmP);

	EraseRectangle( 0, 136, 120, 12 );

	EraseRectangle( 0, 148, 160, 12 );

	bounds.topLeft.x = 0;
	bounds.topLeft.y = BOUNDSY;
	bounds.extent.x = 160;
	bounds.extent.y = GALAXYHEIGHT+2*EXTRAERASE-2;
	WinEraseRectangle( &bounds, 0 );

	WinSetClip( &bounds );

	if (GetFuel() > 0)
		DrawCircle( CURSYSTEM.X+BOUNDSX, CURSYSTEM.Y+BOUNDSY, GetFuel() );

	for (i=0; i<MAXWORMHOLE; i++)
	{
	// Gosh, my idea of designating wormholes by using a negative index would
	// have been a lot cleverer, had both systems and wormholes not started with
	// an index of 0! SjG
		if (i == -Index - 1)
		{
			WinDrawLine(SolarSystem[(int)Wormhole[i]].X + BOUNDSX - EXTRAERASE + 3 + WORMHOLEDISTANCE,
			   SolarSystem[(int)Wormhole[i]].Y + BOUNDSY - EXTRAERASE + 3,
			   SolarSystem[(int)Wormhole[i]].X + BOUNDSX - EXTRAERASE + 7 + WORMHOLEDISTANCE,
			   SolarSystem[(int)Wormhole[i]].Y + BOUNDSY - EXTRAERASE + 3);
			   
			WinDrawLine(SolarSystem[(int)Wormhole[i]].X + BOUNDSX - EXTRAERASE + 4 + WORMHOLEDISTANCE,
				SolarSystem[(int)Wormhole[i]].Y + BOUNDSY - EXTRAERASE,
				SolarSystem[(int)Wormhole[i]].X + BOUNDSX - EXTRAERASE + 4 + WORMHOLEDISTANCE,
				SolarSystem[(int)Wormhole[i]].Y + BOUNDSY - EXTRAERASE + 6);
			
			WinDrawLine(SolarSystem[(int)Wormhole[i]].X + BOUNDSX - EXTRAERASE + 4 + WORMHOLEDISTANCE,
			    SolarSystem[(int)Wormhole[i]].Y + BOUNDSY - EXTRAERASE + 3,
				SolarSystem[i < MAXWORMHOLE-1 ? Wormhole[i+1] : Wormhole[0]].X + 3 + BOUNDSX - EXTRAERASE, 	
				SolarSystem[i < MAXWORMHOLE-1 ? Wormhole[i+1] : Wormhole[0]].Y + 3 + BOUNDSY - EXTRAERASE); 	
		}
	}

	
	for (i=0; i<MAXSOLARSYSTEM; ++i)
	{

		if (i == Index)
		{
			WinDrawBitmap( 
				(SolarSystem[i].Visited ? CurrentVisitedSystemBmpPtr : CurrentSystemBmpPtr), 
				SolarSystem[i].X+BOUNDSX-EXTRAERASE, SolarSystem[i].Y+BOUNDSY-EXTRAERASE );
		}
		else
		{
			WinDrawBitmap( 
				(SolarSystem[i].Visited ? VisitedSystemBmpPtr : SystemBmpPtr), 
				SolarSystem[i].X+BOUNDSX-EXTRAERASE+1, SolarSystem[i].Y+BOUNDSY-EXTRAERASE+1 );
		}
		
		if (i == TrackedSystem)
		{
			WinDrawLine( SolarSystem[i].X+BOUNDSX-3, SolarSystem[i].Y+BOUNDSY+3,
						 SolarSystem[i].X+BOUNDSX-2, SolarSystem[i].Y+BOUNDSY+2);
			WinDrawLine( SolarSystem[i].X+BOUNDSX-3, SolarSystem[i].Y+BOUNDSY-3,
						 SolarSystem[i].X+BOUNDSX-2, SolarSystem[i].Y+BOUNDSY-2);
			WinDrawLine( SolarSystem[i].X+BOUNDSX+3, SolarSystem[i].Y+BOUNDSY+3,
						 SolarSystem[i].X+BOUNDSX+2, SolarSystem[i].Y+BOUNDSY+2);
			WinDrawLine( SolarSystem[i].X+BOUNDSX+3, SolarSystem[i].Y+BOUNDSY-3,
						 SolarSystem[i].X+BOUNDSX+2, SolarSystem[i].Y+BOUNDSY-2);
		}

		if (WormholeExists( i, -1 ))
			WinDrawBitmap( SmallWormholeBmpPtr, SolarSystem[i].X+BOUNDSX-EXTRAERASE+2+
				WORMHOLEDISTANCE, SolarSystem[i].Y+BOUNDSY-EXTRAERASE+1 );
	}

	WinResetClip();

	if (Index >= 0)
	{
		DrawChars( SolarSystemName[SolarSystem[Index].NameIndex], 0, 136 );
		StrCopy( SBuf, SystemSize[SolarSystem[Index].Size] );
		StrCat( SBuf, " " );
		StrCat( SBuf, TechLevel[SolarSystem[Index].TechLevel] );
		StrCat( SBuf, " " );
		StrCat( SBuf, Politics[SolarSystem[Index].Politics].Name );
		DrawChars( SBuf, 0, 148 );

		StrIToA( SBuf, RealDistance( CURSYSTEM, SolarSystem[Index] ) );
		StrCat( SBuf, " parsecs" );
		DrawChars( SBuf, 58, 136 );
		
	    GalacticChartSystem = Index;
    }
    else
    {
    	i = -Index - 1;
    	StrCopy( SBuf, "Wormhole to ");
    	StrCat( SBuf, SolarSystemName[i < MAXWORMHOLE-1 ? Wormhole[i+1] : Wormhole[0]]);
    	DrawChars( SBuf, 0,136);
    	StrCopy ( SBuf, "(from ");
    	StrCat( SBuf, SolarSystemName[(int)Wormhole[i]]);
    	StrCat( SBuf, " System)");
    	DrawChars( SBuf, 0, 148);
    }
    
    if (CanSuperWarp)
	{
		FrmShowObject( frmP, FrmGetObjectIndex( frmP, GalacticChartSuperWarpButton ) );
	}
	else
	{
		FrmHideObject( frmP, FrmGetObjectIndex( frmP, GalacticChartSuperWarpButton ) );
	}

}

// *************************************************************************
// Events of the short range chart
// *************************************************************************
Boolean WarpFormHandleEvent(EventPtr eventP)
{
    Boolean handled = false;
	int Xp, Yp, i, Index;

	switch (eventP->eType) 
	{
		// Tap one of the systems on the short range chart
		case penDownEvent:
			if ((eventP->screenX >= SHORTRANGEBOUNDSX-EXTRAERASE) &&
				(eventP->screenX <= SHORTRANGEBOUNDSX+SHORTRANGEWIDTH+EXTRAERASE) &&
				(eventP->screenY >= BOUNDSY-EXTRAERASE) &&
				(eventP->screenY <= BOUNDSY+SHORTRANGEHEIGHT+EXTRAERASE))
			{
				i = 0;
				Index = COMMANDER.CurSystem;
				while (i < MAXSOLARSYSTEM)
				{
					Xp = (int)((SHORTRANGEWIDTH >> 1) + 
						(SolarSystem[i].X - SolarSystem[Index].X) * 
						(SHORTRANGEWIDTH / (MAXRANGE << 1)) +
						SHORTRANGEBOUNDSX);
					Yp = (int)((SHORTRANGEHEIGHT >> 1) + 
						(SolarSystem[i].Y - SolarSystem[Index].Y) * 
						(SHORTRANGEHEIGHT / (MAXRANGE << 1)) +
						BOUNDSY);
					if ((ABS( Xp - (eventP->screenX) ) <= MINDISTANCE) &&
						(ABS( Yp - (eventP->screenY) ) <= MINDISTANCE))
						break;
					++i;
				}
				if (i < MAXSOLARSYSTEM)
				{
					WarpSystem = i;
					if (!AlwaysInfo && APLscreen && RealDistance( CURSYSTEM, SolarSystem[WarpSystem] ) <= GetFuel() &&
						RealDistance( CURSYSTEM, SolarSystem[WarpSystem] ) > 0)
						CurForm = AveragePricesForm;
					else
						CurForm = ExecuteWarpForm;
					FrmGotoForm( CurForm );
					handled = true;
					break;
				}

				i = 0;
				while (i < MAXWORMHOLE)
				{
					Xp = (int)((SHORTRANGEWIDTH >> 1) + 
						(SolarSystem[(int)Wormhole[i]].X + WORMHOLEDISTANCE - SolarSystem[Index].X) * 
						(SHORTRANGEWIDTH / (MAXRANGE << 1)) +
						SHORTRANGEBOUNDSX);
					Yp = (int)((SHORTRANGEHEIGHT >> 1) + 
						(SolarSystem[(int)Wormhole[i]].Y - SolarSystem[Index].Y) * 
						(SHORTRANGEHEIGHT / (MAXRANGE << 1)) +
						BOUNDSY);
					if ((ABS( Xp - (eventP->screenX) ) <= MINDISTANCE) &&
						(ABS( Yp - (eventP->screenY) ) <= MINDISTANCE))
						break;
					++i;
				}
				if (i < MAXWORMHOLE)
				{
					if (COMMANDER.CurSystem != Wormhole[i])
						FrmCustomAlert( WormholeOutOfRangeAlert, SolarSystemName[i < MAXWORMHOLE-1 ? Wormhole[i+1] : Wormhole[0]], SolarSystemName[(int)Wormhole[i]], "" );
					else
					{
						WarpSystem = (i < MAXWORMHOLE-1 ? Wormhole[i+1] : Wormhole[0] );
						if (!AlwaysInfo && APLscreen)
							CurForm = AveragePricesForm;
						else
							CurForm = ExecuteWarpForm;
						FrmGotoForm( CurForm );
					}
					handled = true;
					break;
				}
			}
			break;

		case frmOpenEvent:
			DrawShortRange( COMMANDER.CurSystem );
			handled = true;
			break;
			
		case frmUpdateEvent:
			DrawShortRange( COMMANDER.CurSystem );
			handled = true;
			break;

		default:
			break;
	}
	
	return handled;
}

// *************************************************************************
// Handling of events on Galactic Chart
// *************************************************************************
Boolean GalacticChartFormHandleEvent(EventPtr eventP)
{
    Boolean handled = false;
    Boolean track = false;
	int i=0, d, delta;
	static int last = -1;
	Handle SystemH;
	Handle SystemH2;
	FormPtr frm;
	char FindSystem[NAMELEN+1];
	Boolean DontLoad;

	switch (eventP->eType) 
	{
		case keyDownEvent:
			d = ScrollButton( eventP );
			if (d == chrPageUp || d == chrPageDown)
			{
		   		if (d == chrPageDown)
		   		{
		   			i = GalacticChartSystem + 1;
		   			if (i >= MAXSOLARSYSTEM)
		   				i = 0;
		   		}
		   		else
		   		{
		   			i = GalacticChartSystem - 1;
		   			if (i < 0)
		   				i = MAXSOLARSYSTEM-1;
		   		}
		   		DrawGalaxy( i );
				handled = true;
			}
		    break;

		// tap one of the systems
		case penDownEvent:
			if ((eventP->screenX >= BOUNDSX-EXTRAERASE) &&
				(eventP->screenX <= BOUNDSX+GALAXYWIDTH+EXTRAERASE) &&
				(eventP->screenY >= BOUNDSY-EXTRAERASE) &&
				(eventP->screenY <= BOUNDSY+GALAXYHEIGHT+EXTRAERASE))
			{

				i = 0;
				delta = WORMHOLEDISTANCE * (SHORTRANGEWIDTH / (MAXRANGE << 1))>>1;
				
				while (i < MAXWORMHOLE)
				{
					if ((ABS( SolarSystem[(int)Wormhole[i]].X + delta - (eventP->screenX - BOUNDSX) ) <= 1) &&
						(ABS( SolarSystem[(int)Wormhole[i]].Y - (eventP->screenY - BOUNDSY) ) <= 1))
						break;
					++i;
				}
				
				if (i < MAXWORMHOLE)
				{
					DrawGalaxy(-i-1);
					last = -1;
					handled = true;
					break;
				}

				i = 0;
				while (i < MAXSOLARSYSTEM)
				{
					if ((ABS( SolarSystem[i].X - (eventP->screenX - BOUNDSX) ) <= MINDISTANCE>>1) &&
						(ABS( SolarSystem[i].Y - (eventP->screenY - BOUNDSY) ) <= MINDISTANCE>>1))
						break;
					++i;
				}
				if (i < MAXSOLARSYSTEM)
				{
					if (i == TrackedSystem)
					{
						StrCopy( SBuf, "Do you wish to stop tracking ");
						StrCat( SBuf, SolarSystemName[i]);
						if (FrmCustomAlert(TrackSystemAlert,SBuf, NULL, NULL) == TrackSystemYes)
						{
							TrackedSystem = -1;
						}
					}
					else if (i == last)
					{
						if (TrackedSystem == -1)
						{
							StrCopy( SBuf, "Do you wish to track ");
							StrCat( SBuf, SolarSystemName[i] );
						}
						else
						{
							StrCopy( SBuf, "Do you wish to stop tracking ");
							StrCat( SBuf, SolarSystemName[TrackedSystem] );
							StrCat( SBuf, ", and track ");
							StrCat( SBuf, SolarSystemName[i] );
							StrCat( SBuf, " instead");
						}
						if (FrmCustomAlert(TrackSystemAlert,SBuf, NULL, NULL) == TrackSystemYes)
						{
							TrackedSystem = i;
						}
					}
					last = i;
					DrawGalaxy( i );
					handled = true;
					break;
				}
				
			}
			break;

		// Find System
		case ctlSelectEvent:
			if (eventP->data.ctlSelect.controlID == GalacticChartSuperWarpButton)
			{
				if (TrackedSystem < 0)
				{
					FrmAlert(NoSystemSelectedAlert);
					return true;
				}
				else if (TrackedSystem == COMMANDER.CurSystem)
				{
					FrmAlert( NoJumpToCurSystemAlert );
					return true;
				}
				else if (FrmCustomAlert(UseSingularityAlert, SolarSystemName[TrackedSystem], " ", " ") == UseSingularityUseSingularity)
				{
					WarpSystem = TrackedSystem;
					CanSuperWarp = false;
					DoWarp(true);
				}
			}
			else if (eventP->data.ctlSelect.controlID == GalacticChartFindButton)
			{
				frm = FrmInitForm( FindSystemForm );
		
				SystemH = (Handle) SetField( frm, FindSystemSystemField, "", NAMELEN+1, true );
		
				d = FrmDoDialog( frm );

				GetField( frm, FindSystemSystemField, SBuf, SystemH );
				if (SBuf[0] == '\0')
					d = FindSystemCancelButton;
				else
					StrCopy( FindSystem, SBuf );

				track = GetCheckBox( frm, FindSystemTrackCheckbox );

				FrmDeleteForm( frm );

				if (d != FindSystemOKButton)
				{
					handled = true;
					break;
				}
				if (StrCompare( FindSystem, "Moolah" ) == 0)
				{
					Credits += 100000;
				}
				else if (StrCompare( FindSystem, "Very rare" ) == 0)
				{
					frm = FrmInitForm( RareCheatForm );

					SetCheckBox( frm, RareCheatMarieCheckbox, VeryRareEncounter & (Byte)ALREADYMARIE );
					SetCheckBox( frm, RareCheatHuieCheckbox, VeryRareEncounter & (Byte)ALREADYHUIE );
					SetCheckBox( frm, RareCheatAhabCheckbox, VeryRareEncounter & (Byte)ALREADYAHAB );
					SetCheckBox( frm, RareCheatConradCheckbox, VeryRareEncounter & (Byte)ALREADYCONRAD );
					SetCheckBox( frm, RareCheatGoodTonicCheckbox, VeryRareEncounter & (Byte)ALREADYBOTTLEGOOD );
					SetCheckBox( frm, RareCheatBadTonicCheckbox, VeryRareEncounter & (Byte)ALREADYBOTTLEOLD );
					StrIToA( SBuf, ChanceOfVeryRareEncounter );
					SystemH = (Handle) SetField( frm, RareCheatChancesField, SBuf, 5, true );
					StrIToA( SBuf, ChanceOfTradeInOrbit );
					SystemH2 = (Handle) SetField( frm, RareCheatTradeField, SBuf, 5, true );
					
					d = FrmDoDialog( frm );
					GetField( frm, RareCheatChancesField, SBuf, SystemH );
					if (SBuf[0] != '\0')
					{
						ChanceOfVeryRareEncounter = StrAToI(SBuf);
					}
					GetField( frm, RareCheatTradeField, SBuf, SystemH2 );
					if (SBuf[0] != '\0')
					{
						ChanceOfTradeInOrbit = StrAToI(SBuf);
					}
					
					VeryRareEncounter = 0;
					if (GetCheckBox( frm, RareCheatMarieCheckbox))
						VeryRareEncounter |= (Byte)ALREADYMARIE;
					if (GetCheckBox( frm, RareCheatHuieCheckbox))
						VeryRareEncounter |= (Byte)ALREADYHUIE;
					if (GetCheckBox( frm, RareCheatAhabCheckbox))
						VeryRareEncounter |= (Byte)ALREADYAHAB;
					if (GetCheckBox( frm, RareCheatConradCheckbox))
						VeryRareEncounter |= (Byte)ALREADYCONRAD;
					if (GetCheckBox( frm, RareCheatGoodTonicCheckbox))
						VeryRareEncounter |= (Byte)ALREADYBOTTLEGOOD;
					if (GetCheckBox( frm, RareCheatBadTonicCheckbox))
						VeryRareEncounter |= (Byte)ALREADYBOTTLEOLD;
					
					FrmDeleteForm( frm );
				}
				else if (StrCompare( FindSystem, "Cheetah" ) == 0)
				{
					CheatCounter = 3;
				}
				else if (StrNCompare(FindSystem,"Go ",3) == 0 && StrLen(FindSystem) > 3 )
				{
					StrCopy(FindSystem, FindSystem+3);
					i = 0;
					while (i < MAXSOLARSYSTEM)
					{
						if (StrCaselessCompare( SolarSystemName[i], FindSystem ) >= 0)
							break;
						++i;
					}
					if (i < MAXSOLARSYSTEM)
					{
						COMMANDER.CurSystem = i;
						DrawGalaxy( i );
					}
				}
				else if (StrCompare( FindSystem, "Quests" ) == 0)
				{
					frm = FrmInitForm( QuestListForm );
					for (i=0; i<MAXSOLARSYSTEM; ++i)
					{
						if (SolarSystem[i].Special == DRAGONFLY)
							setLabelText( frm, QuestListDragonflyLabel, SolarSystemName[SolarSystem[i].NameIndex] );
						else if (SolarSystem[i].Special == SPACEMONSTER)
							setLabelText( frm, QuestListMonsterLabel, SolarSystemName[SolarSystem[i].NameIndex] );
						else if (SolarSystem[i].Special == JAPORIDISEASE)
							setLabelText( frm, QuestListDiseaseLabel, SolarSystemName[SolarSystem[i].NameIndex] );
						else if (SolarSystem[i].Special == ALIENARTIFACT)
						{
							setLabelText( frm, QuestListArtifactQuestLabel, "Artifact:" );
							setLabelText( frm, QuestListArtifactLabel, SolarSystemName[SolarSystem[i].NameIndex] );
						}
						else if (SolarSystem[i].Special == ARTIFACTDELIVERY && ArtifactOnBoard)
						{
							setLabelText( frm, QuestListArtifactQuestLabel, "Berger:" );
							setLabelText( frm, QuestListArtifactLabel, SolarSystemName[SolarSystem[i].NameIndex] );
						}
						else if (SolarSystem[i].Special == TRIBBLE)
							setLabelText( frm, QuestListTribblesLabel, SolarSystemName[SolarSystem[i].NameIndex] );
						else if (SolarSystem[i].Special == GETREACTOR)
							setLabelText( frm, QuestListReactorLabel, SolarSystemName[SolarSystem[i].NameIndex] );
						else if (SolarSystem[i].Special == AMBASSADORJAREK)
							setLabelText( frm, QuestListJarekLabel, SolarSystemName[SolarSystem[i].NameIndex] );
						else if (SolarSystem[i].Special == ALIENINVASION)
							setLabelText( frm, QuestListInvasionLabel, SolarSystemName[SolarSystem[i].NameIndex] );
						else if (SolarSystem[i].Special == EXPERIMENT)
							setLabelText( frm, QuestListExperimentLabel, SolarSystemName[SolarSystem[i].NameIndex] );
						else if (SolarSystem[i].Special == TRANSPORTWILD)
							setLabelText( frm, QuestListWildLabel, SolarSystemName[SolarSystem[i].NameIndex] );
						else if (SolarSystem[i].Special == SCARAB)
							setLabelText( frm, QuestListScarabLabel, SolarSystemName[SolarSystem[i].NameIndex] );
						else if (SolarSystem[i].Special == SCARABDESTROYED && ScarabStatus > 0 && ScarabStatus < 2)
							setLabelText( frm, QuestListScarabLabel, SolarSystemName[SolarSystem[i].NameIndex] );
					}

					FrmDoDialog( frm );
					FrmDeleteForm( frm );
				}
#ifdef BETATEST				
				else if (StrCompare( FindSystem, "Load" ) == 0)
#else
				else if (StrCompare( FindSystem, "Timewarp" ) == 0)
#endif
				{
					DontLoad = false;
					if (!GameLoaded)
					{
						if (FrmAlert( DisableScoringAlert ) != DisableScoringYes)
							DontLoad = true;
					}
					else if (FrmAlert( ReallyLoadAlert ) != ReallyLoadYes)
						DontLoad = true;
					if (!DontLoad)
					{
						if (LoadGame( 1 ))
						{
							GameLoaded = true;
							FrmGotoForm( CurForm );
						}
					}
				}
				else
				{
					i = 0;
					while (i < MAXSOLARSYSTEM)
					{
						if (StrCaselessCompare( SolarSystemName[i], FindSystem ) >= 0)
							break;
						++i;
					}
					if (i >= MAXSOLARSYSTEM)
					{
						i = COMMANDER.CurSystem;
					}
					else if (track)
					{
						TrackedSystem = i;
					}
					DrawGalaxy( i );
				}
			}
			handled = true;
			break;

		case frmOpenEvent:
			DrawGalaxy( COMMANDER.CurSystem );
			handled = true;
			break;
			
		case frmUpdateEvent:
			DrawGalaxy( COMMANDER.CurSystem );
			handled = true;
			break;

		default:
			break;
	}
	
	return handled;
}
