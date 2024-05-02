/*  Vexed - selectlevelform.c "select level from level pack"
    Copyright (C) 2001-2006 Scott Ludwig (scottlu@eskimo.com)
	 Copyright (C) 2006 The Vexed Source Forge Project Team

    August 17, 2001

    This file is part of Vexed.

    Vexed is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Vexed is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Vexed; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "PalmOS.h"
#include "vexed.h"
#include "protos.h"
#include "ini.h"

typedef UInt32 dword;
typedef UInt16 word;
typedef UInt8  byte;

#define kcxScrollArrowPadding 9

extern Preferences VexedPreferences;
extern char gszEllipsis[];

FormType *gpfrmLevel;
ListType *gplstLevels;
int gcxLevelList;

void LevelListDrawProc(short int n, RectangleType *prc, char **ppsz)
{
	char szTitle[64];
	char szMoves[32];
	char sz[128];
   int cxEllipsis;
   Int16 cxAllowed;
   Boolean fIgnore;
   int cch;
   int cchT;
   int cxMoves;
   FontID fnt, fntSav;

	// Get the title string
   if (GetLevelProperty(n, "title", sz, sizeof(sz))) {
		StrPrintF(szTitle, "%d: %s", n+1, sz);
   } else {
      if (n > VexedPreferences.LevelPack[0].SolvedLevels) {
         StrPrintF(szTitle, "Level %d", n);
      } else {
         StrPrintF(szTitle, "Level %d:", n);
      }
   }

   // Get the move count string
   if (n <= VexedPreferences.LevelPack[0].SolvedLevels) {
		StrPrintF(szMoves, " %d/%d", VexedPreferences.LevelPack[0].Score[n].MoveCountMade, VexedPreferences.LevelPack[0].Score[n].MoveCountPar);
   } else if (n == VexedPreferences.LevelPack[0].SolvedLevels + 1) {
      if (n == VexedPreferences.LevelPack[0].CurrentLevel) {
		   StrPrintF(szMoves, " ?/%d", VexedPreferences.LevelPack[0].MoveCountPar);
      } else {
		   StrPrintF(szMoves, " ?/%d", VexedPreferences.LevelPack[0].Score[n].MoveCountPar);
      }
   } else {
      szMoves[0] = 0;
   }

   // Make current level is bold
   fnt = stdFont;
   if (n == VexedPreferences.LevelPack[0].CurrentLevel)
      fnt = boldFont;
   fntSav = FntSetFont(fnt);

   // Make sure the title string is ellipsified properly
   cxMoves = FntCharsWidth(szMoves, StrLen(szMoves));

   // Draw as much of the text that'll fit
   StrCopy(sz, szTitle);
   cxEllipsis = FntCharsWidth(gszEllipsis, StrLen(gszEllipsis));
   cxAllowed = gcxLevelList - kcxScrollArrowPadding - cxEllipsis - cxMoves;
   fIgnore = true;
   cch = StrLen(sz);
   cchT = cch;
   FntCharsInWidth(sz, (Int16 *)&cxAllowed, (Int16 *)&cch, &fIgnore);

   // cch has been updated with the # of chars that will fit. Terminate
   // here and add ellipsis if there is enough room
   if (cch != cchT) {
      sz[cch] = 0;
      if (sizeof(sz) - cch >= StrLen(gszEllipsis))
         StrCopy(&sz[cch], gszEllipsis);
   }

   StrCat(sz, szMoves);
   WinDrawChars(sz, StrLen(sz), prc->topLeft.x, prc->topLeft.y);
   FntSetFont(fntSav);
}

void SelectLevelInit(FormType *pfrm)
{
   FieldType *pfld;
   RectangleType rc;
   int cLevels;

   // Set field to name of current level pack
   pfld = (FieldType *)GetControlPtr(gpfrmLevel, FieldLevelPackName);
   FldSetTextPtr(pfld, VexedPreferences.LevelPack[0].DbName);

	// The list has this many choices
	FrmGetObjectBounds(pfrm, FrmGetObjectIndex(pfrm, ListSelectLevel), &rc);
	gcxLevelList = rc.extent.x;

   cLevels = VexedPreferences.LevelPack[0].MaxLevels;
	gplstLevels = (ListType *)GetControlPtr(pfrm, ListSelectLevel);
	LstSetListChoices(gplstLevels, NULL, cLevels);
   LstSetSelection(gplstLevels, VexedPreferences.LevelPack[0].CurrentLevel);
	LstMakeItemVisible(gplstLevels, VexedPreferences.LevelPack[0].CurrentLevel);
	LstSetDrawFunction(gplstLevels, LevelListDrawProc);
}

Boolean LevelListProc(EventType *pevt)
{
   switch (pevt->eType) {
   case keyDownEvent:
      switch (pevt->data.keyDown.chr) {
   	case pageUpChr:
			LstScrollList(gplstLevels, winUp, 8);
			return true;

		case pageDownChr:
			LstScrollList(gplstLevels, winDown, 8);
			return true;
		}
      break;

	case ctlSelectEvent:
		switch (pevt->data.ctlEnter.controlID) {
		case ButtonOK:
         if (LstGetSelection(gplstLevels) > VexedPreferences.LevelPack[0].SolvedLevels + 1) {
            FrmAlert(AlertLevelNotSolved);
            return true;
         }
			return false;
      }
   default:
     break;
   }

   return false;
}

void DoSelectLevel()
{
   word id;
   int nLevel;

   // Init and bring up the form

	gpfrmLevel = FrmInitForm(FormSelectLevel);
	if (gpfrmLevel == NULL)
		return;
   SelectLevelInit(gpfrmLevel);
   FrmSetEventHandler(gpfrmLevel, LevelListProc);
   id = FrmDoDialog(gpfrmLevel);
   nLevel = LstGetSelection(gplstLevels);
   FrmDeleteForm(gpfrmLevel);

   if( (id == ButtonOK) && (nLevel != VexedPreferences.LevelPack[0].CurrentLevel) ) {
      VexedPreferences.LevelPack[0].Score[VexedPreferences.LevelPack[0].CurrentLevel].MoveCountPar = VexedPreferences.LevelPack[0].MoveCountPar;
      VexedPreferences.LevelPack[0].MoveCountMade = 0;
	   VexedPreferences.LevelPack[0].MoveCountPar = 0;
      VexedPreferences.LevelPack[0].CurrentLevel = nLevel;
      LoadLevel(NULL, nLevel, false);
   }
}
