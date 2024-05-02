/*  Vexed - pack.c "loading from level packs"
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
#include "ini.h"
#include "debug.h"

typedef UInt32 dword;
typedef UInt16 word;
typedef UInt8  byte;

extern PlayGround current;
extern Preferences VexedPreferences;
Boolean ParseLevelNotation(char *pszLevel, CPlayGround *plevel);
int LoadLevelRow(byte *pb, char *psz, char *pszMax);

Boolean GetLevelProperty(int nLevel, char *pszProp, char *pszValue, int cbValue)
{
	Ini *pini;
	int n;
	IniSection *psec;

   // First try to open this ini

	*pszValue = 0;
   pini = OpenIni(VexedPreferences.LevelPack[0].DbName);
   if (pini == NULL)
      return false;

	// Find the level

   n = 0;
   psec = FindFirstIniSection(pini);
   while (psec != NULL) {
      if (StrNCompare("Level", (char *)(psec + 1), 5) == 0) {
         if (n == nLevel)
            break;
         n++;
      }
      psec = FindNextIniSection(pini);
   }

   // Property present?

	if (psec == NULL || !FindProperty(psec, pszProp, pszValue, cbValue)) {
		CloseIni(pini);
		return false;
	}

   CloseIni(pini);
	return true;
}

void InitLevelPackPref(Ini *pini, char *pszPack, Int16 nLevelCurrent, Int16 cLevelsSolved, LevelPackPrefs *plvlp)
{
   IniSection *psec;
   int cLevels;

   MemSet(plvlp, sizeof(*plvlp), 0);
   StrCopy(plvlp->DbName, pszPack);

   cLevels = 0;
   psec = FindFirstIniSection(pini);
   while (psec != NULL) {
      if (StrNCompare("Level", (char *)(psec + 1), 5) == 0)
         cLevels++;
      psec = FindNextIniSection(pini);
   }

   plvlp->MaxLevels = cLevels;
   plvlp->LevelValid = true;
   plvlp->SolvedLevels = cLevelsSolved;
   plvlp->CurrentLevel = nLevelCurrent;
}

int FindLevelPackPref(char *psz)
{
   int nPack;

   for (nPack = 0; nPack < MaxLevelPackPrefs; nPack++) {
      if (StrCompare(psz, VexedPreferences.LevelPack[nPack].DbName) == 0)
         return nPack;
   }
   return -1;
}

Boolean LoadLevelFromPack(char *pszPack, int nLevel)
{
   int n;
   int nPack;
   IniSection *psec;
   char szT[256];
   CPlayGround level;
   Ini *pini;
   Int32 cMoves = -1;

   // First try to open this ini

   pini = OpenIni(pszPack);
   if (pini == NULL)
      return false;

   // This is our new game pack. Is it in the prefs?

   nPack = FindLevelPackPref(pszPack);

   // If not found, then we're going to load the first level

   if (nPack == -1)
      nLevel = 0;

   // Try to find the requested level.
   // -1 means just load this pack, no specific level

   if (nLevel != -1) {
      n = 0;
      psec = FindFirstIniSection(pini);
      while (psec != NULL) {
         if (StrNCompare("Level", (char *)(psec + 1), 5) == 0) {
            if (n == nLevel)
               break;
            n++;
         }
         psec = FindNextIniSection(pini);
      }

      // board property present?

      if (!FindProperty(psec, "board", szT, sizeof(szT))) {
         CloseIni(pini);
         return false;
      }

      // Attempt to parse this board - may fail

      if (!ParseLevelNotation(szT, &level)) {
         CloseIni(pini);
         return false;
      }

      // Success. Remember the move count

      cMoves = -1;
      if (FindProperty(psec, "solution", szT, sizeof(szT)))
         cMoves = StrLen(szT) / 2;
   }

   // MRU it to the front of the prefs

   if (nPack >= 0) {
      // It is in the prefs. Make it first.

      if (nPack != 0) {
         LevelPackPrefs pref = VexedPreferences.LevelPack[nPack];
         MemMove(&VexedPreferences.LevelPack[1], &VexedPreferences.LevelPack[0], nPack * sizeof(LevelPackPrefs));
         VexedPreferences.LevelPack[0] = pref;
      }
   } else {
      // It isn't so shift all the entries down and make it first

      MemMove(&VexedPreferences.LevelPack[1], &VexedPreferences.LevelPack[0], (MaxLevelPackPrefs - 1) * sizeof(LevelPackPrefs));
      InitLevelPackPref(pini, pszPack, nLevel, nLevel - 1, &VexedPreferences.LevelPack[0]);
   }

   // If we loaded a level, make it current

   if (nLevel != -1) {
      MemMove(&VexedPreferences.LevelPack[0].Level, &level, sizeof(level));
      VexedPreferences.LevelPack[0].LevelValid = true;
		VexedPreferences.LevelPack[0].MoveCountMade = 0;
		VexedPreferences.LevelPack[0].MoveCountPar = cMoves;
   }

   CloseIni(pini);
   return true;
}

static char *my_strchr(char *psz, char ch)
{
   char chT;
   while ((chT = *psz) != 0) {
      if (chT == ch)
         return psz;
      psz++;
   }
   return NULL;
}

Boolean ParseLevelNotation(char *pszLevel, CPlayGround *plevel)
{
   PlayGround level;
   byte *pbLoad;

   // Validate the board and get the dimensions

	char *pszLast = pszLevel;
	Boolean fLoop = true;
	int cRows = 0;
	int cCols = -1;
	while (fLoop) {
      int cColsT;
		char *pszNext = my_strchr(pszLast, '/');
		if (pszNext == NULL) {
			pszNext = pszLast + StrLen(pszLast);
			fLoop = false;
		}
		cColsT = LoadLevelRow(NULL, pszLast, pszNext);
		if (cCols == -1) {
			cCols = cColsT;
		} else if (cCols != cColsT) {
			return false;
		}
		cRows++;
		pszLast = pszNext + 1;
	}

	// Vexed wants these sizes

	if (cCols != sizeX || cRows != sizeY)
		return false;

	// Load it this time

	pbLoad = (byte *)&level[0][0];
	pszLast = pszLevel;
	fLoop = true;
	while (fLoop) {
		// Find the end of this row

		char *pszNext = my_strchr(pszLast, '/');
		if (pszNext == NULL) {
			pszNext = pszLast + StrLen(pszLast);
			fLoop = false;
		}

		// Load the row

		LoadLevelRow(pbLoad, pszLast, pszNext);

		// Next row...

		pbLoad += sizeX;
		pszLast = pszNext + 1;
	}

   // Convert into *pLevel format

   LevelToCLevel(level, *plevel);

   return true;
}

int LoadLevelRow(byte *pb, char *psz, char *pszMax)
{
	int cBlocks = 0;
	for (; psz < pszMax; psz++) {
		char ch = *psz;

		// Is this a number (non-moveable blocks?)

		int c = 0;
		while (ch >= '0' && ch <= '9') {
			c = c * 10 + ch - '0';
			psz++;
			if (psz >= pszMax)
				break;
			ch = *psz;
		}
		if (c != 0) {
			cBlocks += c;
			if (pb != NULL) {
				while (c != 0) {
					*pb++ = 9;
					c--;
				}
			}
			psz--;
			continue;
		}

		// Is this empty space?

		if (ch == '~') {
			cBlocks++;
			if (pb != NULL)
				*pb++ = 0;
			continue;
		}

		// This is a block type. Remember it verbatim

      if (ch < 'a' || ch > 'h')
         return -1;

		cBlocks++;
		if (pb != NULL)
			*pb++ = ch - 'a' + 1;
	}

	return cBlocks;
}
