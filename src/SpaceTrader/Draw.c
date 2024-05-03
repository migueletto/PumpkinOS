/***********************************************************************
 *
 * SPACE TRADER 1.2.0
 *
 * Draw.c
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
// ControlPtr RectangularButton( FormPtr frmP, int Nr )
// void SBufMultiples( Int32 Count, char* Txt )
// void setFormTitle( int formID, CharPtr s )
// void setCurrentWinTitle( CharPtr s )
// void setLabelText( FormPtr frm, int labelid, CharPtr s )
// void RectangularShortcuts( FormPtr frm, int First )
// void EraseRectangle( int x, int y, int ex, int ey )
// void DrawChars( char* Text, int x, int y )
// int ScrollButton( EventPtr eventP )
//
// Modifications:
// mm/dd/yy - description - author
// *************************************************************************

// Needed to change the frames of buttons to rectangles. There is currently no function
// that allows to do it. When an OS is published that fails on this access (probably OS 5.0),
// the RectangularButton function must be adapted.
//#ifndef ALLOW_ACCESS_TO_INTERNALS_OF_CONTROLS
//#define ALLOW_ACCESS_TO_INTERNALS_OF_CONTROLS 1
//#endif

#include "external.h"

// *************************************************************************
// Make button rectangular
// *************************************************************************
ControlPtr RectangularButton( FormPtr frmP, int Nr )
{
	int objindex;
	ControlPtr cp;

	objindex = FrmGetObjectIndex( frmP, Nr );
	cp = (ControlPtr)FrmGetObjectPtr( frmP, objindex );
	if (BELOW50 || RectangularButtonsOn)
		cp->attr.frame = rectangleButtonFrame;
	
	return cp;
}


// *************************************************************************
// String number and qualifier together
// *************************************************************************
void SBufMultiples( Int32 Count, char* Txt )
{
	StrIToA( SBuf2, Count );
	StrCat( SBuf, SBuf2 );
	StrCat( SBuf, " " );
	StrCat( SBuf, Txt );
	if (Count != 1)
		StrCat( SBuf, "s" );
}

// *************************************************************************
// String number and qualifier together, but no digit for singulars
// *************************************************************************
void SBufShortMultiples( Int32 Count, char* Txt )
{
	if (Count != 1)
	{
		StrIToA( SBuf2, Count );
		StrCat( SBuf, SBuf2 );
		StrCat( SBuf, " " );
		StrCat( SBuf, Txt );
		if (Count != 1)
			StrCat( SBuf, "s" );
	}
	else
	{
		StrCat( SBuf, Txt );
	}
}


// *************************************************************************
// Setting the title of a form.
// CW 8 required a rewrite, as FrmGetTitle now returns a const Char *
// *************************************************************************
void setFormTitle( int formID, CharPtr s )
{
	FormPtr frm;
//	CharPtr wintitle;

	frm = FrmGetFormPtr( formID );
//	wintitle = FrmGetTitle( frm );
//	StrCopy( wintitle, s );
//	FrmSetTitle( frm, wintitle );

	FrmCopyTitle( frm, s );

}


// *************************************************************************
// Setting the title of the current form.
// *************************************************************************
void setCurrentWinTitle( CharPtr s )
{
	FormPtr frm = FrmGetActiveForm();
	FrmSetTitle( frm, s );
}		


// *************************************************************************
// Setting the text of a label.
// CW 8 required a rewrite, as CtlGetLabel now returns a const Char *
// *************************************************************************
void setLabelText( FormPtr frm, int labelid, CharPtr s )
{
	int objindex;
//	ControlPtr cp;
//	CharPtr c;
	
	objindex = FrmGetObjectIndex( frm, labelid );
//	cp = (ControlPtr)FrmGetObjectPtr( frm, objindex );
//	c = CtlGetLabel( cp );
//	StrCopy( c, s );
	
	FrmHideObject(frm, objindex);
	FrmCopyLabel(frm, labelid, s);
	FrmShowObject(frm, objindex);
}


// *************************************************************************
// Make shortcut buttons rectangular
// *************************************************************************
void RectangularShortcuts( FormPtr frm, int First )
{
	
	SetControlLabel(frm, First+0, Shortcuts[Shortcut1]);
	RectangularButton( frm, First+0 );
	SetControlLabel(frm, First+1, Shortcuts[Shortcut2]);
	RectangularButton( frm, First+1 );
	SetControlLabel(frm, First+2, Shortcuts[Shortcut3]);
	RectangularButton( frm, First+2 );
	SetControlLabel(frm, First+3, Shortcuts[Shortcut4]);
	RectangularButton( frm, First+3 );
	
}


// *************************************************************************
// Erases a rectangle
// *************************************************************************
void EraseRectangle( int x, int y, int ex, int ey )
{
	RectangleType a;

	a.topLeft.x = x;
	a.topLeft.y = y;
	a.extent.x = ex;
	a.extent.y = ey;
	WinEraseRectangle( &a, 0 );
}


// *************************************************************************
// Draws a text on the screen
// *************************************************************************
void DrawChars( char* Text, int x, int y )
{
    WinDrawChars( Text, StrLen( Text ), x, y );
}

// ******
// Debug Alerts
// ******
// only compile this in if desired. The resource does get included regardless,
// but that's only at a cost of a handful of bytes.
#ifdef _INCLUDE_DEBUG_DIALOGS_ 

void DebugIntAlert(int a, int b, int c)
{
char as[51],bs[51],cs[51];
StrIToA(as, a);
StrIToA(bs, b);
StrIToA(cs, c);
FrmCustomAlert(DebugAlert, as, bs, cs);
}

void DebugLongAlert(Int32 a, Int32 b, Int32 c)
{
char as[51],bs[51],cs[51];
StrPrintF(as, "%ld", a);
StrPrintF(bs, "%ld", b);
StrPrintF(cs, "%ld", c);
FrmCustomAlert(DebugAlert, as, bs, cs);
}

void DebugCharAlert(char *a, char *b, char *c)
{
FrmCustomAlert(DebugAlert, a,b,c);
}

#endif // end of debug code

// *************************************************************************
// Draws a text centered on the screen
// *************************************************************************
void DrawCharsCentered( char* Text, int y, Boolean useBold )
{
	FontID originalFont;
	Int16 width=160, length=StrLen(Text);
	Boolean fits;
	if (useBold)
	{
		originalFont = FntSetFont(boldFont);
	}
	else
	{
		originalFont = FntSetFont(stdFont);
	}
	FntCharsInWidth(Text, &width, &length, &fits);
	// should probably find the width of the screen for the hi-rez crowd.
    WinDrawChars( Text, StrLen( Text ), (int)(80 - width/2), y );
    FntSetFont(originalFont);
}


// *****************************************************************
// Utility function for displaying headlines and incrementing the
// line counter. If we're too far down the page, we simply don't
// display the headline (we'll return a false if that's the case)
// *****************************************************************
Boolean DisplayHeadline(char *text, int *y_coord)
{
	int offset=0;
	Int16 width=158 - (NEWSINDENT1 * 2), length=StrLen(text);
	Boolean fits;
	char theText[72];

	FntCharsInWidth(text, &width, &length, &fits);
	
	// check to see if we've got room to put it on one line
	if (fits)
	{
		// do we have room on the page?
		if (*y_coord > 128)
			return false;
		DrawChars(text, NEWSINDENT1, *y_coord);
	}
	else
	{
		// do we have room for two lines on the page?
		if (*y_coord > 107)
			return false;
			
		// is our text too long? 
		if (StrLen(text) > 71)
		{
			return false;
		}
		StrCopy(theText, text);
		offset = length;
		while (offset > 0 && theText[offset] != ' ')
			offset--;
			
		theText[offset]='\0';
		DrawChars(theText, NEWSINDENT1, *y_coord);
		*y_coord += 11;
		DrawChars(theText + offset + 1, NEWSINDENT2, *y_coord);
	}
	
	*y_coord += 15;
	return true;
}

// *****************************************************************
// Utility function for displaying a page of text.
// *****************************************************************
void DisplayPage(char *text, int start_y)
{
	int offset=0;
	int internalOffset = 0;
	int vert = start_y;
	Int16 width=148, length, origLength=StrLen(text);
	Boolean fits, looping;
	char theText[46];

	looping = true;
	while (looping)
	{
			
		StrNCopy(theText, text + offset, min(45, origLength-offset));

		theText[min (45, origLength - offset) ] = '\0';
		length=StrLen(theText);
		
		FntCharsInWidth(theText, &width, &length, &fits);
		
		if (fits)
		{
			offset += 45;
		}
		else
		{
			internalOffset = length;
			while (internalOffset > 0 &&
				theText[internalOffset] != ' ' && theText[internalOffset] != '-')
				internalOffset--;
				
			if (theText[internalOffset] == '-' && internalOffset < length)
			{
				theText[internalOffset+1]='\0';
			}
			else
			{
				theText[internalOffset]='\0';
			}
			offset += internalOffset + 1;
		}
		DrawChars(theText, 8, vert);
		vert += 12;
		if (offset >= origLength)
		{
			looping = false;
			theText[45]='\0';
		}
	}
}


// *************************************************************************
// Return scrollbutton that is pushed
// *************************************************************************
int ScrollButton( EventPtr eventP )
{
	ULong Ticks;

	if ((eventP->data.keyDown.chr == chrPageUp) ||
		(eventP->data.keyDown.chr == chrPageDown))
	{
		Ticks = TimGetTicks();
		if (Ticks < GalacticChartUpdateTicks + (SysTicksPerSecond() >> 1))
			return 0;
		GalacticChartUpdateTicks = Ticks;
		return eventP->data.keyDown.chr;
	}
	
	return 0;
}

