/*  Vexed - system.c "Application independant stuff"
    Copyright (C) 1999 James McCombe (cybertube@earthling.net)
	 Copyright (C) 2006 The Vexed Source Forge Project Team

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

extern Preferences   VexedPreferences;
extern PlayGround    current;
extern PlayGround    stored;

extern UInt16        cardNo;                 // for alarm calls
extern LocalID       appID;                  // for alarm calls

extern Boolean       InSolution;             // Are we displaying a solution
extern Int16 MoveCountMadeStored;
extern Int16 MoveCountParStored;

struct {
   Boolean
   PieceMoveAnim,                            // whether the movement of a piece by the user is animated
   GravityAnim,                              // whether the gravity effect is animated
   EliminationAnim,                          // whether piece elimination is animated
   Sound;                                    // whether game sound is on or off
   Int16 CurrentLevel, SolvedLevels;         // stores the level the user was last at
} OldPrefs;

//-----------------------------------------------------------------------------
void DeleteOldPreferences() {
   // this should only be called if old version 1.3-style preferences were encountered
   DmOpenRef         reference;
   Err               err;
   UInt32            recs;
   UInt16            idx;
   DmResType         resType;
   DmSearchStateType state;
   Boolean           found = false;
   UInt16            cardNo;
   LocalID           dbID;

   // first we need the index of what record the old 'vexd' preferences are in the saved prefs db

   // find the saved prefs db
   err = DmGetNextDatabaseByTypeCreator(true, &state, 'sprf', 'psys', false, &cardNo, &dbID);
   ErrFatalDisplayIf(err == dmErrCantFind, MSG_SYS_ERR_NO_SAVED_PREFS);

   // open saved prefs db in r/o

   reference = DmOpenDatabase(cardNo, dbID, dmModeReadOnly);
   ErrFatalDisplayIf(reference == 0, MSG_SYS_ERR_OPEN_DB_RO);

   // get the number of records in the db
   err = DmDatabaseSize(cardNo, dbID, &recs, NULL, NULL);
   ErrFatalDisplayIf(err, MSG_SYS_ERR_CANT_GET_REC);

   // read through the db looking for 'vexd' creator ID
   for (idx = 0; (idx < recs) && (idx < 400) && (!found); idx++) {
      // read the record
      err = DmResourceInfo(reference, idx, &resType, NULL, NULL);
      ErrFatalDisplayIf(err, "error reading record");

      if (resType == 'vexd') {               // is this the one?
         found = true;
         break;
      }
   }

   // close the db
   DmCloseDatabase(reference);

   if (found) {
      // now that we have the index into the db, delete that record

      reference = DmOpenDatabase(cardNo, dbID, dmModeReadWrite); // open in r/w
      ErrFatalDisplayIf(!reference, "cant open db in rw");

      err = DmRemoveResource(reference, idx);// delete it

      DmCloseDatabase(reference);            // close the db

   }
}

void InstallLevelPack(UInt16 idr, char *pszName)
{
   DmOpenRef pdb;
   LocalID lid;
   void *pvSrc, *pvDst;
   UInt32 cb;
   MemHandle hSrc, hDst;
   UInt16 nT;
   Err err;
   Boolean dirty = true;

   // Already exists? Then don't create it.

	lid = DmFindDatabase(0, pszName);
   if (lid != 0)
      return;

	// Create a new db.

   if (DmCreateDatabase(0, pszName, 'Vexd', 'DATA', false) != 0)
      return;

	// Find the database. Must ensure name uniqueness so search
	// by psz.

	lid = DmFindDatabase(0, pszName);
   if (lid == 0)
      return;

   // Open the database

	pdb = DmOpenDatabase(0, lid, dmModeReadWrite);
	if (pdb == NULL) {
		DmDeleteDatabase(0, lid);
      return;
	}

   // Get a pointer to the resource

   hSrc = DmGetResource('pack', idr);
   if (hSrc == NULL) {
Error:
      DmCloseDatabase(pdb);
		DmDeleteDatabase(0, lid);
      return;
   }

   // Create a new record

   nT = dmMaxRecordIndex;
   cb = MemHandleSize(hSrc);
   hDst = DmNewRecord(pdb, &nT, cb);
   if (hDst == NULL) {
      DmReleaseResource(hSrc);
      goto Error;
   }

   // Clear the busy bit
   err = DmReleaseRecord (pdb, nT, dirty);
   if (err != errNone) {
      DmReleaseResource(hSrc);
      goto Error;
   }

   // Copy the bits into this new record

   pvSrc = MemHandleLock(hSrc);
   pvDst = MemHandleLock(hDst);
   DmWrite(pvDst, 0, pvSrc, cb);
   MemHandleUnlock(hDst);
   MemHandleUnlock(hSrc);

   // All done; clean up

	DmCloseDatabase(pdb);
   DmReleaseResource(hSrc);
}

//-----------------------------------------------------------------------------
void LoadPreferences() {
   Int16             prefsRc;
   UInt16            prefsSize;

   // load application preferences into the current version of the preferences structure
   prefsSize = sizeof(VexedPreferences);
   prefsRc = PrefGetAppPreferences('Vexd', prefsSaved, &VexedPreferences, &prefsSize, true);
   if (prefsRc != kprefVersion || prefsRc == noPreferenceFound) {
      // Try to install the built in game packs

//      InstallLevelPack(1, OriginalLevelPackName);
//      InstallLevelPack(2, NewLevelsLevelPackName);

      // Clear Prefs
      MemSet(&VexedPreferences, sizeof(VexedPreferences), 0);

	   // default new prefs
      VexedPreferences.BlindsEffect = true;
      VexedPreferences.SkipIntro = false;
      VexedPreferences.BlockSet = 1;

      // try to read preferences from older version of Vexed
      if (!PrefGetAppPreferencesV10('vexd', 0, &OldPrefs, sizeof(OldPrefs))) {
         // couldn't read that, either, so default all
         VexedPreferences.PieceMoveAnim = 1;
         VexedPreferences.GravityAnim = 1;
         VexedPreferences.EliminationAnim = 1;
         VexedPreferences.Sound = 0;

      } else {
         // force default level pack
         LevelPackPrefs *pLevelPack = &VexedPreferences.LevelPack[0];

         pLevelPack->CurrentLevel = OldPrefs.CurrentLevel;
         pLevelPack->SolvedLevels = OldPrefs.SolvedLevels - 1;
         pLevelPack->MaxLevels = 59;
         if (pLevelPack->CurrentLevel < 0)
            pLevelPack->CurrentLevel = 0;
         if (pLevelPack->CurrentLevel > 59)
            pLevelPack->CurrentLevel = 59;
         if (pLevelPack->SolvedLevels < -1)
            pLevelPack->SolvedLevels = -1;
         if (pLevelPack->SolvedLevels > 59)
            pLevelPack->SolvedLevels = 59;

         if (FrmAlert(AlertOldPrefsFound) == 0) {
            StrCopy(pLevelPack->DbName, OriginalLevelPackName);
         } else {
            StrCopy(pLevelPack->DbName, NewLevelsLevelPackName);
         }

         // got some old style prefs data
         VexedPreferences.PieceMoveAnim = OldPrefs.PieceMoveAnim;
         VexedPreferences.GravityAnim = OldPrefs.GravityAnim;
         VexedPreferences.EliminationAnim = OldPrefs.EliminationAnim;
         VexedPreferences.Sound = OldPrefs.Sound;

         // delete the old 'vexd' preferences
         DeleteOldPreferences();
      }
   }

   // get a valid dbID and cardNo for future calls
   SysCurAppDatabase(&cardNo, &appID);

   return;
}

//-----------------------------------------------------------------------------

void SavePreferences() {
   // Remember last board for this level pack

   if (InSolution) {
      LevelToCLevel(stored, VexedPreferences.LevelPack[0].Level);
		VexedPreferences.LevelPack[0].MoveCountMade = MoveCountMadeStored;
		VexedPreferences.LevelPack[0].MoveCountPar = MoveCountParStored;
   }
   else {
      LevelToCLevel(current, VexedPreferences.LevelPack[0].Level);
   }

   // write out the application preferences
   PrefSetAppPreferences('Vexd', prefsSaved, kprefVersion, &VexedPreferences, sizeof(VexedPreferences), true);
   return;
}

