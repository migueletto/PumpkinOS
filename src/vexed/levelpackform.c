/*  Vexed - levelpackform.c "level pack form"
    Copyright (C) 2001 Scott Ludwig (scottlu@eskimo.com)
	 Copyright (C) 2006 The Vexed Source Forge Project Team

    September 1, 2001

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
#include "strings.h"
#include "ini.h"

typedef UInt32 dword;
typedef UInt16 word;
typedef UInt8  byte;

#define kcxScrollArrowPadding 9

FormType *gpfrmPack;
ListType *gplstPacks;
int gcxPackList;
char gszAuthor[128];
char gszUrl[128];
char gszDescription[512];

extern char gszEllipsis[4];
extern Preferences VexedPreferences;
extern PlayGround current;
void DisplayLevelPackProperties(ListType *plst, int nSelected);

Boolean GetDatabaseN(int nDb, LocalID *plid, int *pnCard, int *pc)
{
   // Look for databases of our creator id, with type id of kdwSyncDb or kdwGamesDb

   UInt32 dwType;
	Boolean fNewSearch = true;
	DmSearchStateType cookie;
	*pc = 0;
	while (true) {
		Err err = DmGetNextDatabaseByTypeCreator(fNewSearch, &cookie, 0, 'Vexd',
				false, (UInt16 *)pnCard, plid);
		fNewSearch = false;
		if (err != 0)
			return false;
		DmDatabaseInfo(*pnCard, *plid, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &dwType, NULL);
		if (dwType != 'DATA')
			continue;
		(*pc)++;
		if (nDb == (*pc) - 1)
			return true;
	}
}

int GetLevelPackCount()
{
	int c = 0;
	LocalID lid;
	int nCard;
	GetDatabaseN(-1, &lid, &nCard, &c);

	return c;
}

Boolean GetLevelPackName(int n, char *psz)
{
  LocalID lid;
	int nCard;
	int c;

	*psz = 0;
	if (!GetDatabaseN(n, &lid, &nCard, &c))
		return false;
	DmDatabaseInfo(nCard, lid, psz, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
   return true;
}

ControlType *GetControlPtr(FormType *pfrm, word id)
{
   return FrmGetObjectPtr(pfrm, FrmGetObjectIndex(pfrm, id));
}

void LevelPackListDrawProc(short int n, RectangleType *prc, char **ppsz)
{
   char sz[128];
   char szPlayed[32];
   int cxEllipsis;
   Int16 cxAllowed;
   Boolean fIgnore;
   int cch;
   int cchT;
   int nPack;
   int cxPlayed;
   FontID fnt, fntSav;

   // Get name

   GetLevelPackName(n, sz);

   // Make current level pack bold

   fnt = stdFont;
   if (StrCompare(sz, VexedPreferences.LevelPack[0].DbName) == 0)
      fnt = boldFont;
   fntSav = FntSetFont(fnt);

   // Find in prefs for # levels played and overall # levels

   nPack = FindLevelPackPref(sz);
   cxPlayed = 0;
   szPlayed[0] = 0;
   if (nPack != -1) {
      LevelPackPrefs *ppref = &VexedPreferences.LevelPack[nPack];
      StrPrintF(szPlayed, ": %d (%d/%d)", CalcScore(nPack), ppref->SolvedLevels + 1, ppref->MaxLevels);
      cxPlayed = FntCharsWidth(szPlayed, StrLen(szPlayed));
   }

   // Draw as much of the text that'll fit

   cxEllipsis = FntCharsWidth(gszEllipsis, StrLen(gszEllipsis));
   cxAllowed = gcxPackList - kcxScrollArrowPadding - cxEllipsis - cxPlayed;
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

   StrCat(sz, szPlayed);
   WinDrawChars(sz, StrLen(sz), prc->topLeft.x, prc->topLeft.y);
   FntSetFont(fntSav);
}

int SelectCurrentLevelPack(int cPacks)
{
	int nSelected;
	int n;
	char sz[64];

	if (cPacks == -1)
		cPacks = GetLevelPackCount();

   // Figure out which pack is selected, if any

   nSelected = -1;
   for (n = 0; n < cPacks; n++) {
      GetLevelPackName(n, sz);
      if (StrCompare(sz, VexedPreferences.LevelPack[0].DbName) == 0) {
         nSelected = n;
         break;
      }
   }
   if (nSelected >= 0) {
      LstSetSelection(gplstPacks, nSelected);
      LstMakeItemVisible(gplstPacks, nSelected);
   }
   return nSelected;
}

void LevelPackInit(FormType *pfrm)
{
   RectangleType rc;
   UInt32 dwVer;
   int cPacks;
   int nSelected;

	gplstPacks = (ListType *)GetControlPtr(pfrm, ListLevelPacks);

	// The list has this many choices

	FrmGetObjectBounds(pfrm, FrmGetObjectIndex(pfrm, ListLevelPacks), &rc);
	gcxPackList = rc.extent.x;

   cPacks = GetLevelPackCount();
	LstSetListChoices(gplstPacks, NULL, cPacks);

	nSelected = SelectCurrentLevelPack(cPacks);
	LstSetDrawFunction(gplstPacks, LevelPackListDrawProc);

   if (nSelected >= 0)
      DisplayLevelPackProperties(gplstPacks, nSelected);

	FtrGet(sysFtrCreator, sysFtrNumROMVersion, &dwVer);
	if (dwVer < 0x03003000)
      FrmHideObject(pfrm, FrmGetObjectIndex(pfrm, ButtonBeam));
}

void strncpyz(char *pszDst, char *pszSrc, int cb)
{
   cb--;
   while (cb-- != 0) {
      if ((*pszDst++ = *pszSrc++) == 0)
         return;
   }
   *pszDst = 0;
}

void SetFieldText(FormType *pfrm, word id, char *psz)
{
   FieldType *pfld = (FieldType *)GetControlPtr(pfrm, id);
   if (pfld == NULL)
      return;
   FldSetTextPtr(pfld, psz);
   FldRecalculateField(pfld, true);
   //FldDrawField(pfld);
}

void DisplayLevelPackProperties(ListType *plst, int nSelected)
{
   char sz[128];
   Ini *pini;
   IniSection *psec;

   // Initial settings

   strncpyz(gszAuthor, MSG_LPACK_UNKNOWN_AUTHOR, sizeof(gszAuthor));
   strncpyz(gszUrl, MSG_LPACK_UNKNOWN_WEBSITE, sizeof(gszUrl));
   strncpyz(gszDescription, MSG_LPACK_UNKNOWN_DESC, sizeof(gszDescription));

   // Load settings from level pack

   sz[0] = 0;
   GetLevelPackName(nSelected, sz);
   pini = OpenIni(sz);
   if (pini != NULL) {
      psec = FindIniSection(pini, "General");
      if (psec != NULL) {
         FindProperty(psec, "Author", gszAuthor, sizeof(gszAuthor));
         FindProperty(psec, "URL", gszUrl, sizeof(gszUrl));
         FindProperty(psec, "Description", gszDescription, sizeof(gszDescription));
      }
      CloseIni(pini);
   }

   // Set text on controls

   SetFieldText(gpfrmPack, FieldName, gszAuthor);
   SetFieldText(gpfrmPack, FieldURL, gszUrl);
   SetFieldText(gpfrmPack, FieldDescription, gszDescription);
}

LocalID FindLevelPackDatabase(char *psz, int *pnCard)
{
	int cCards;
	LocalID lid;
	int n;

   lid = 0;
	cCards = MemNumCards();
   for (n = 0; n < cCards; n++) {
      lid = DmFindDatabase(n, psz);
      if (lid != 0) {
			*pnCard = n;
			return lid;
		}
   }
	return 0;
}

void DoDeleteLevelPack()
{
	int nSelected;
	LocalID lid;
	int nCard;
	char sz[64];

   nSelected = LstGetSelection(gplstPacks);
	if (nSelected == -1)
		return;
	GetLevelPackName(nSelected, sz);

	// Find this database

	lid = FindLevelPackDatabase(sz, &nCard);
   if (lid == 0)
      return;

	// Current level pack?

	if (StrCompare(sz, VexedPreferences.LevelPack[0].DbName) == 0) {
		FrmAlert(AlertCannotDeleteCurrent);
		return;
	}

	// Confirm delete

	if (FrmAlert(AlertDeleteLevelPack) != 0)
		return;
	DmDeleteDatabase(nCard, lid);

	// Re-init and redraw

   LevelPackInit(gpfrmPack);
	LstDrawList(gplstPacks);
}

Err WriteDBData(const void *pv, UInt32 *pcb, void *pvUser)
{
	Err err;
	*pcb = ExgSend((ExgSocketPtr)pvUser, (void *)pv, *pcb, &err);
	return err;
}


void BeamDatabase(int nCard, LocalID lid, char *pszName, char *pszDescription)
{
	ExgSocketType exgSocket;
	Err err;
   UInt32 dwVer;

	FtrGet(sysFtrCreator, sysFtrNumROMVersion, &dwVer);
   if (dwVer < 0x03003000) {
      FrmAlert(AlertNoBeam);
      return;
   }

	MemSet(&exgSocket, sizeof(exgSocket), 0);
	exgSocket.description = pszDescription;
	exgSocket.name = pszName;
	err = ExgPut(&exgSocket);
	if (err == 0) {
		err = ExgDBWrite(WriteDBData, &exgSocket, NULL, lid, nCard);
		err = ExgDisconnect(&exgSocket, err);
	}
}


void DoBeamLevelPack()
{
	int nCard;
	LocalID lid;
	char sz[64];
	int nSelected;

   nSelected = LstGetSelection(gplstPacks);
	if (nSelected == -1)
		return;
	GetLevelPackName(nSelected, sz);
	lid = FindLevelPackDatabase(sz, &nCard);
   if (lid == 0)
      return;
	BeamDatabase(nCard, lid, "vexed_data.pdb", sz);
}

Boolean LevelPackProc(EventType *pevt)
{
   switch (pevt->eType) {
   case keyDownEvent:
      switch (pevt->data.keyDown.chr) {
   	case pageUpChr:
			LstScrollList(gplstPacks, winUp, 5);
			return true;

		case pageDownChr:
			LstScrollList(gplstPacks, winDown, 5);
			return true;
		}
      break;

   case lstSelectEvent:
      DisplayLevelPackProperties(gplstPacks, pevt->data.lstSelect.selection);
      break;

	case ctlSelectEvent:
		switch (pevt->data.ctlEnter.controlID) {
		case ButtonDelete:
			DoDeleteLevelPack();
			return true;

		case ButtonBeam:
			DoBeamLevelPack();
			return true;
		}
		break;
   default:
		break;
   }

   return false;
}

Boolean DoLevelPackForm( Boolean fNoRedraw )
{
   word id;
   int nSelected;
   char sz[128];
	Boolean Loaded = false;

   // Init and bring up the form

	gpfrmPack = FrmInitForm(FormLevelPack);
	if (gpfrmPack == NULL)
		return Loaded;
   LevelPackInit(gpfrmPack);
   FrmSetEventHandler(gpfrmPack, LevelPackProc);
	id = FrmDoDialog(gpfrmPack);

   // Load this level pack.
   if (id == ButtonOK) {
      // What is the name of the selected pack?
      nSelected = LstGetSelection(gplstPacks);
		if (nSelected != -1) {
			//sz[0];
			GetLevelPackName(nSelected, sz);

			// Load only if not current
			if (StrCompare(sz, VexedPreferences.LevelPack[0].DbName) != 0) {

				// Remember the level in the pref before we load the new pack
				LevelToCLevel(current, VexedPreferences.LevelPack[0].Level);

				// Load it. -1 means default: "use prefs or 0".
				LoadLevel(sz, -1, fNoRedraw);

				// Report we did it
				Loaded = true;
			}
		}
   }
   FrmDeleteForm(gpfrmPack);

	return( Loaded );
}
