/*  Vexed - globals.c "Global variable declarations"
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

#pragma pack(2)

#include "vexed.h"
#include "protos.h"
#include "strings.h"

// Calls of LoadLevel() don't handle failure robustly, so in case of failure
// this level is used.

CPlayGround emptyLevel = {
    {0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00}
};


PlayGround           current;
PlayGround           stored;                 // for storing the current state when the memory button is pushed
PlayGround           undo[UndoSize];
Int16                undoMovesMade[UndoSize];

PlayGroundIntro      currentI = {            // animated intro level
   {6,0,2,8,3,0},
   {8,0,4,0,8,6},
   {8,0,8,0,0,7},
   {1,0,7,0,5,8}
};

UInt8                UndoCount = 0;
UInt8                UndoIndex = 0;
Int8                 CheatCount = 0;
Boolean              memorized;              // has memorized button been pushed
Boolean              InBlockCheck;           // are we in a block check
Preferences          VexedPreferences;       // saved prefs

MoveType             replay[2][ReplaySize];  // First row is previous moves, second row is current level
UInt8                ReplayCurrent,          // counts of moves
                     ReplayPrevious;
Boolean              DoingReplay = false;    // replay underway
Int16                levelReplay;            // which level is being replayed

UInt16               cardNo;                 // for OS calls
LocalID              appID;                  // for OS calls
int	               screenMode;             // number of bit color (1/4/8)
Boolean              refreshIcons;           // after preferences are changed, refresh control icons
Int16                selectLevel;            // temp var for select level form
Int16                selectLevelRepeatCnt;   // Count of number of repeat events on select level form
Int8                 selectDelta;            // count to add each button press
Boolean              cheatOn;                // have they enabled cheating?

MemHandle	         ObjectBitmapHandles[BitmapCount + 8];  // vars for offscreen writing of bitmaps
BitmapPtr	         ObjectBitmapPtr[BitmapCount + 8];
WinHandle	         ObjectWindowHandles[BitmapCount + 8];

Boolean              InSolution;             // Are we displaying a solution
Boolean              InSolutionBack;         // Doing solution back function?
Int16                MoveNum;                // Which move of solution are we now displaying
Boolean              SolutionRun;            // Is solution in free-running mode
Int8                 OldSet;                 // Save off previous block set num
char                 *MonoListP[] = { MSG_MONO_LIST_0, MSG_MONO_LIST_1, MSG_MONO_LIST_2 }; // monochrome popup list has to be global
UInt16               GameVolume;             // volume from preferences
char                 *GraySListP[] = { MSG_GRAY_LIST_0, MSG_GRAY_LIST_1, MSG_GRAY_LIST_2, MSG_GRAY_LIST_3,
		  											MSG_GRAY_LIST_4, MSG_GRAY_LIST_5, MSG_GRAY_LIST_6, MSG_GRAY_LIST_7, MSG_GRAY_LIST_8 }; // grayscale popup list has to be global
Boolean              canDoGrayScale;         // save whether this device does grayscale
Boolean              in1Bit;                 // Are we in 1 bit mode?
Boolean              movePending;            // Have they tapped, waiting for tap 2?
WinHandle            gpwinOffScreen;         // congrats screen window handle
