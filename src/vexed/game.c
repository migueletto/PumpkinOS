/*  Vexed- game.c "The actual code for making the game play"
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
#include "debug.h"

#if 0
extern Level         level;
#endif

extern PlayGround    current;
extern PlayGround    stored;

extern Preferences   VexedPreferences;
extern Int8          CheatCount;
extern UInt8         UndoCount;
extern UInt8         UndoIndex;
extern Boolean       memorized;
extern MoveType      replay[1][ReplaySize];  // First row is previous moves, second row is current level
extern UInt8         ReplayCurrent,          // counts of moves
                     ReplayPrevious;
extern Boolean       DoingReplay;
extern Int16         levelReplay;
extern WinHandle	   ObjectWindowHandles[BitmapCount];
extern CPlayGround   emptyLevel;
extern Boolean       InSolution;             // Are we displaying a solution
extern Boolean       InSolutionBack;         // Doing solution back function?
extern Boolean       SolutionRun;            // Is solution in free-running mode

//-----------------------------------------------------------------------------
void CLevelToLevel(CPlayGround clevel, PlayGround level)
{
   int x, y;

   for (y = 0; y < sizeY; y ++) {
      for (x = 0; x < sizeX/2; x ++) {
         level[y][(x<<1)+1] = clevel[y][x] & 0x0f;
         level[y][(x<<1)] = (clevel[y][x] >> 4) & 0x0f;
      }
   }
}

//-----------------------------------------------------------------------------
void LevelToCLevel(PlayGround level, CPlayGround clevel)
{
   int x, y;

   for (y = 0; y < sizeY; y++) {
      for (x = 0; x < sizeX; x ++) {
         if (x & 1) {
            clevel[y][x/2] = (clevel[y][x/2] & 0xf0) | (level[y][x] & 0xf);
         } else {
            clevel[y][x/2] = (clevel[y][x/2] & 0x0f) | (level[y][x] << 4);
         }
      }
   }
}

//-----------------------------------------------------------------------------
Boolean LoadLevel(char *levelPackName, Int16 levelnumber, Boolean fNoRedraw) {
   Int8              x, y;
   RectangleType     r;
   //UInt16            sumTotal = 0;
   Boolean           fSuccess = true;

   // Valid level pack?

   if (levelPackName == NULL)
      levelPackName = VexedPreferences.LevelPack[0].DbName;
   if (*levelPackName == 0)
      return false;

   // Level pack done?
   if (VexedPreferences.LevelPack[0].DbName[0] != 0) {
      if (levelnumber == VexedPreferences.LevelPack[0].MaxLevels) {           // game complete
         EndSequence();
         VexedPreferences.LevelPack[0].CurrentLevel = 0;
         levelnumber = 0;
      }
   }

   // Load this level from database.

   if (!LoadLevelFromPack(levelPackName, levelnumber)) {
      MemMove(VexedPreferences.LevelPack[0].Level, emptyLevel, sizeof(emptyLevel));
      VexedPreferences.LevelPack[0].LevelValid = false;
      fSuccess = false;
   }

   if (levelnumber < VexedPreferences.LevelPack[0].MaxLevels) {
      if (!DoingReplay && !InSolution) {
         for (y = 0; y < sizeY; y++) {
            for (x = 0; x < sizeX; x++) {
               stored[y][x] = 0;             // clear the game state memory
            }
         }
         CheatCount = 0;
         UndoCount = 0;
         UndoIndex = 0;

         if (!InSolution) {
            // clear the recall icon
            r.topLeft.x = 112;
            r.topLeft.y = 144;
            r.extent.x = BlockSize;
            r.extent.y = BlockSize;
            WinEraseRectangle(&r, 0);        // erase old bitmap

            // clear the undo icon
            r.topLeft.x = 96;
            WinEraseRectangle(&r, 0);        // erase old bitmap

            memorized = false;
         }
      }

      // Copy over the level data, draw to screen

      CLevelToLevel(VexedPreferences.LevelPack[0].Level, current);

      if (!fNoRedraw && !InSolutionBack)
         DrawLevel( true );
   }

   return fSuccess;
}

//-----------------------------------------------------------------------------
void MoveAnimation(Int8 sx, Int8 sy, Int8 ddx, Int8 ddy, Boolean AnimOn, int speed) {
   UInt8             i;
   UInt8             rx,ry;
   RectangleType     r, rSrc;

   if (!InSolutionBack) {                    // going through previous moves, skipping drawing?
      rx = (sx * BlockSize);
      ry = (sy * BlockSize) + BlockSize;
      r.extent.x = rSrc.extent.x = BlockSize;
      r.extent.y = rSrc.extent.y = BlockSize;
      rSrc.topLeft.x = rSrc.topLeft.y = 0;
      r.topLeft.x = rx;
      r.topLeft.y = ry;
//    WinEraseRectangle(&r, 0);              // erase old bitmap

      if (AnimOn) {
         Int8 dx,dy;
         dx = ddx;
         dy = ddy;
         for (i = 1; i < BlockSize; i++) {
            // animate block move

            DrawBM2(current[sy][sx] - 1, rx + dx, ry + dy, ddx, ddy, false);
            //WinCopyRectangle (ObjectWindowHandles[current[sy][sx] - 1], 0, &rSrc,
            //               rx + dx, ry + dy, winPaint);
            //WinDrawBitmap(Bitmap[current[sy][sx]], rx + dx, ry + dy);
            SysTaskDelay(speed);
            r.topLeft.x = rx + dx;
            r.topLeft.y = ry + dy;
            //WinEraseRectangle(&r, 0);        // erase old bitmap
            dx += ddx;
            dy += ddy;
         }
      } else {
         WinEraseRectangle(&r, 0);              // erase old bitmap
      }

      // draw final block
      DrawBM2(current[sy][sx] - 1, rx + ddx * BlockSize, ry + ddy * BlockSize, ddx, ddy, false);
      //WinCopyRectangle (ObjectWindowHandles[current[sy][sx] - 1], 0, &rSrc,
        //             rx + ddx * BlockSize, ry + ddy * BlockSize, winPaint);
      //WinDrawBitmap(Bitmap[current[sy][sx]], rx + ddx * BlockSize, ry + ddy * BlockSize);
      SysTaskDelay(speed);
   }
   // Update current Playground
   current[sy + ddy][sx + ddx] = current[sy][sx];
   current[sy][sx] = Air;
   return;
}

//-----------------------------------------------------------------------------
Boolean MoveBlock(Int8 sx, Int8 sy, Int8 direction, Int16 speed) {
   int               i;
   RectangleType     r;
   Boolean           stop=0;

   // if current block is neither air nor a wall block and
   // if the one to the right is just air then go ahead and move it
   if ((current[sy][sx]     != Air ) &&
       (current[sy][sx]     != Wall)) {
      switch (direction) {
      case Right:
         if ((sx < sizeX-1) &&
             (current[sy][sx + 1] == Air )) {
            stop=0;
            SoundEffect(HighBeep);
            MoveAnimation(sx,sy,1,0,VexedPreferences.PieceMoveAnim,speed);
            SoundEffect(LowBeep);
         } else {
            stop=1;
         }
         break;
      case Left:
         if ((sx > 0) &&
             (current[sy][sx - 1] == Air )) {
            stop=0;
            SoundEffect(HighBeep);
            MoveAnimation(sx,sy,-1,0,VexedPreferences.PieceMoveAnim,speed);
            SoundEffect(LowBeep);
         } else {
            stop=1;
         }
         break;
      case Down:
         if ((sy < sizeY-1) &&
             (current[sy + 1][sx] == Air )) {
            stop=1;
            SoundEffect(HighBeep);
            MoveAnimation(sx,sy,0,1,VexedPreferences.GravityAnim,speed);
            SoundEffect(LowBeep);
         }
         break;
      case Erase:
         stop=1;
         current[sy][sx] = Air;              // erase the block from the current level array
         if (!InSolutionBack) {
            SoundEffect(HighSweepLow);
            if (VexedPreferences.EliminationAnim) {
               // only animate of prefs says it should
               for (i = 8; i >= 0; i --) {
                  r.topLeft.x = (sx * BlockSize) + i;
                  r.topLeft.y = ((sy + 1) * BlockSize) + i;
                  r.extent.x = 2 * (BlockSize/2 - i);
                  r.extent.y = 2 * (BlockSize/2 - i);
                  WinEraseRectangle(&r, 0);  // erase the on screen block
                  SysTaskDelay(speed);
               }
            } else {
               r.topLeft.x = (sx * BlockSize);
               r.topLeft.y = ((sy + 1) * BlockSize);
               r.extent.x = BlockSize;
               r.extent.y = BlockSize;
               WinEraseRectangle(&r, 0);     // erase the on screen block
            }
         }
         //UpdateStats();                    // updates the statistics text on screen
         break;
      }
   }
   return stop;
}

//-----------------------------------------------------------------------------
Boolean CheckGravity() {
   Int8              x, y;
   Boolean           change = 0;

   // go through it bottom to top so as all the blocks tumble down on top of each other
   for (y = sizeY-2; y >= 0; y--) {
      for (x = sizeX-1; x >= 0; x--) {
         change |= MoveBlock(x, y, Down, 1);
      }
   }
   return change;
}

//-----------------------------------------------------------------------------
Boolean CheckMatches() {
   Int8              x, y;
   Boolean           change = 0;
   PlayGround        mark;                   // array for holding blocks marked for removal

   /* just go through from top to bottom marking pieces
    *  which have matches above, below, left and to the right of them */
   for (y = 0; y < sizeY; y++) {
      for (x = 0; x < sizeX; x++) {
         // ALWAYS initialise each element in the 'mark' array
         mark[y][x] = 0;
         if ((current[y][x] != Air) && (current[y][x] != Wall)) {
            // only check if its not air or a wall block
            if (y != 0) {
               // if there is an identical piece above then mark this piece
               mark[y][x] |= (current[y - 1][x] == current[y][x]);
            }
            if (y != sizeY-1) {
               // if there is an identical piece below then mark this piece
               mark[y][x] |= (current[y + 1][x] == current[y][x]);
            }
            if (x != 0) {
               // if there is an identical piece to the left then mark this piece
               mark[y][x] |= (current[y][x - 1] == current[y][x]);
            }
            if (x != sizeX-1) {
               // if there is an identical piece to the right then mark this piece
               mark[y][x] |= (current[y][x + 1] == current[y][x]);
            }
         }
      }
   }
   // go through the current level array checking
   // if a piece is marked and if so, remove it
   for (y = 0; y < sizeY; y ++) {
      for (x = 0; x < sizeX; x ++) {
         if (mark[y][x]) {
            change |= MoveBlock(x, y, Erase, 3);
         }
      }
   }
   return change;
}

//-----------------------------------------------------------------------------
Boolean CompareLevel(PlayGround Array1, PlayGround Array2) {
   Int8              x, y;

   for (y = 0; y < sizeY; y ++) {
      for (x = 0; x < sizeX; x ++) {
         if (Array1[y][x] != Array2[y][x]) {
            return 1;
         }
      }
   }
   return 0;
}

//-----------------------------------------------------------------------------
void CopyLevel(PlayGround Target, PlayGround Source) {
   Int8              x, y;

   for (y = 0; y < sizeY; y++) {
      for (x = 0; x < sizeX; x++) {
         Target[y][x] = Source[y][x];
      }
   }
   return;
}

//-----------------------------------------------------------------------------
int CountBlocksRemaining()
{
   int x;
   int y;
   int cBlocks;

   cBlocks = 0;
   for (x = 0; x < sizeX; x++) {
      for (y = 0; y < sizeY; y++) {
         if (current[y][x] != Wall && current[y][x] != Air)
            cBlocks++;
      }
   }
   return cBlocks;
}

//-----------------------------------------------------------------------------
Boolean PlayerMakeMove2(Int8 sx, Int8 sy, Int8 direction, Int8 dist, Boolean fAddScore) {
   // returns true if this move completes the level
   Int8              dx = 0;
   Boolean           stop = 0;
   MoveType          move;

   // to hold the level position before a check gravity call
   PlayGround previous;

   if (!DoingReplay && !InSolution) {

      // save the move for replay
      move.x = sx;
      move.y = sy;
      move.direction = direction;
      move.distance = 0;
      replay[ReplayCIndex][ReplayCurrent++] = move;
   }

   // simply pass parameters through to MoveBlock function
   while ((dist-- > 0) && (!stop)) {
      stop |= MoveBlock(sx+dx, sy, direction, 2);
      if (fAddScore) {
         if (!DoingReplay && !InSolution)
            replay[ReplayCIndex][ReplayCurrent - 1].distance += 1;
			VexedPreferences.LevelPack[0].MoveCountMade += 1;
         UpdateStats();
      }
      if (direction == Left) {
         dx -= 1;
      } else if (direction == Right) {
         dx += 1;
      }
      // keep cycling round until neither checking for gravity or matching blocks has any effect
      do {
         // keep checking for gravity until all blocks are resting on something
         do {
            CopyLevel(previous,current);
            stop |= CheckGravity();          // check for gravity defying blocks and bring them down to earth!
         } while (CompareLevel(previous, current));
         stop |= CheckMatches();             // check for any groups of matching blocks
      } while (CompareLevel(previous, current));
   }
   if (!DoingReplay && !InSolution) {
      // put the Undo Icon up
      DrawBM(BitmapUndo,96,144,true);
   }
   return CountBlocksRemaining() == 0;
}

//-----------------------------------------------------------------------------
Boolean PlayerMakeMove(Int8 sx, Int8 sy, Int8 direction, Int8 dist) {
	return PlayerMakeMove2(sx, sy, direction, dist, false);
}

//-----------------------------------------------------------------------------
void ReplayLevel(Int8 which) {
   int               i;
   MoveType          m;
   RectangleType     r;

   DoingReplay = true;

   // replay the current level
   if (which == ReplayCIndex) {

      // redraw current level
      levelReplay = VexedPreferences.LevelPack[0].CurrentLevel;
      LoadLevel(NULL, VexedPreferences.LevelPack[0].CurrentLevel, false);
      for (i = 0; i < ReplayCurrent; i++) {
         m = replay[ReplayCIndex][i];
         // can't be done with this level to get here
         PlayerMakeMove2(m.x, m.y, m.direction, m.distance, true); // get to the end of the level?
      }
   }

   // replay the previous level
   else {
      // Save current level
      int MovesMadeSav = VexedPreferences.LevelPack[0].MoveCountMade;
      int MovesParSav = VexedPreferences.LevelPack[0].MoveCountPar;
      CopyLevel(stored, current);

      // redraw previous level
      levelReplay = VexedPreferences.LevelPack[0].CurrentLevel - 1;
      LoadLevel(NULL, VexedPreferences.LevelPack[0].CurrentLevel - 1, false);
      for (i = 0; i < ReplayPrevious; i++) {
         m = replay[ReplayPIndex][i];
         PlayerMakeMove2(m.x, m.y, m.direction, m.distance, true); // get to the end of the level?
      }
      LoadLevel(NULL, VexedPreferences.LevelPack[0].CurrentLevel, true);

      // Restore current level
      VexedPreferences.LevelPack[0].MoveCountMade = MovesMadeSav;
      VexedPreferences.LevelPack[0].MoveCountPar = MovesParSav;
      CopyLevel(current, stored);

      DoingReplay = false;
      DrawLevel( true );

      // Empty stored array
      stored[0][0] = 0;

      // clear the recall icon
      r.topLeft.x = 112;
      r.topLeft.y = 144;
      r.extent.x = BlockSize;
      r.extent.y = BlockSize;
      WinEraseRectangle(&r, 0);              // erase old bitmap
   }

   DoingReplay = false;

}

//-----------------------------------------------------------------------------
void AddMoveEvent(Int8 sx, Int8 sy, Int8 direction) {
   EventType         customEvent;            // make our own event type

   MemSet(&customEvent, sizeof(customEvent), 0);  // Clear event.
   customEvent.eType = MakeMoveEvent;        // set our custom event
   customEvent.data.generic.datum[0] = (UInt16) sx; // Store x position of move
   customEvent.data.generic.datum[1] = (UInt16) sy; // Store y position of move
   customEvent.data.generic.datum[2] = (UInt16) direction; // Store which way to go
   EvtAddEventToQueue( &customEvent );       // toss it out

}

//-----------------------------------------------------------------------------
int CalcScore(int nPack) {
	int nScore;
	int n;
	int count;
	int nScoreLevel;
   int cMovesMade;
   LevelPackPrefs *ppref;

	// Calc past level scores

   ppref = &VexedPreferences.LevelPack[nPack];
	nScore = ppref->BaseMoveCountMade - ppref->BaseMoveCountPar;
	count = ppref->SolvedLevels + 1;
	if (count > kcParRecords)
		count = kcParRecords;
   for (n = 0; n < count; n++) {
      // Sometimes Vexed will think it's already solved a level when in fact it's never
      // been played, such as the cases of cheat to skip ahead levels, and when
      // starting either Classic I or II in the middle of the pack. In these cases,
      // assume those levels were played at par.

      cMovesMade = ppref->Score[n].MoveCountMade;
      if (cMovesMade != 0)
   		nScore += cMovesMade - ppref->Score[n].MoveCountPar;
   }

	// Add in this level's scores if more moves have been made than the solution
	// (in typical golf style). If this level has been played already then
   // take the lower of the two scores for this level.

   if (ppref->SolvedLevels >= ppref->CurrentLevel) {
      cMovesMade = ppref->Score[ppref->CurrentLevel].MoveCountMade;
      if (cMovesMade > ppref->MoveCountMade)
         cMovesMade = ppref->MoveCountMade;
   } else {
      cMovesMade = ppref->MoveCountMade;
   }

	nScoreLevel = cMovesMade - ppref->MoveCountPar;
	if (nScoreLevel > 0)
		nScore += nScoreLevel;

	return nScore;
}
