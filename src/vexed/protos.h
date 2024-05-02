/*  Vexed v2.1 Beta 1 - globals.c "Global variable declarations"
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

// directions
#define Left 1
#define Right 2
#define Up 3
#define Down 4
#define Erase 5

// Level Setup
#define sizeX 10
#define sizeY 8

#ifdef HIRESDOUBLE
#define BlockSize 32
#define LocRestart 288
#define LocBack 32
#define LocForward 96
#define LocSelectLevel 64
#define LocFirst 0
#define LocLast 128
#define LocPending 160
#define LocUndo 192
#define LocRecall 224
#define LocMemory 256
#define TopRow 32
#define BottomRow 288
#define ScreenSize 320
#else
#define BlockSize 16
#define LocRestart 144
#define LocBack 16
#define LocForward 48
#define LocSelectLevel 32
#define LocFirst 0
#define LocLast 64
#define LocPending 80
#define LocUndo 96
#define LocRecall 112
#define LocMemory 128
#define TopRow 16
#define BottomRow 144
#define ScreenSize 160
#endif

#define Air 0
#define Wall 9

// Intro Setup
#define sizeXI 6
#define sizeYI 4
#define BlockSizeX 25
#define BlockSizeY 40
#define AirI 0
#define WallI 8

// Buttons
#define FirstButton    0
#define PreviousButton 1
#define SelectButton   2
#define NextButton     3
#define LastButton     4
#define InfoButton     5
#define PendingButton  5
#define UndoButton     6
#define RecallButton   7
#define MemorizeButton 8
#define RestartButton  9

// Sound
#define HighBeep 1
#define LowBeep 2
#define HighSweepLow 3

// Undo
#define UndoSize 15

// Recall
#define ReplaySize 100
#define ReplayCIndex 1
#define ReplayPIndex 0

// Number of level packs to remember prefs about
#define MaxLevelPackPrefs 20
#define DefaultLevelPackName "Classic Levels"
#define OriginalLevelPackName "Classic Levels"
#define NewLevelsLevelPackName "Classic II Levels"

/* Types */
// Single Level
typedef Int8 PlayGround [sizeY][sizeX];
// Compressed 2 Block/char Level
typedef Int8 CPlayGround [sizeY][sizeX/2];

// Level Setup
#if 0
typedef CPlayGround Level [MaxLevels];
#endif

// Intro Level
typedef Int8 PlayGroundIntro [sizeYI][sizeXI];

typedef struct {
  UInt8 MoveCountMade;
  UInt8 MoveCountPar;
} ScoreRecord;

// LevelPackPrefs. These may or may not be valid. When a level pack is
// loaded a check is made to see if previous prefs exist and if so,
// they are used.

#define kcParRecords 60
typedef struct {
  char DbName[dmDBNameLength];
  Int16 CurrentLevel;
  Int16 SolvedLevels;
  Int16 MaxLevels;
  CPlayGround Level;
  Int16 MoveCountPar;
  Int16 MoveCountMade;
  Boolean LevelValid;
  Int16 BaseMoveCountMade;
  Int16 BaseMoveCountPar;
  ScoreRecord Score[kcParRecords];
} LevelPackPrefs;

// Preferences
// This is the main preferences structure.  They are stored across hotsyncs.
typedef struct {
  Boolean PieceMoveAnim;                     // whether the movement of a piece by the user is animated
  Boolean GravityAnim;                       // whether the gravity effect is animated
  Boolean EliminationAnim;                   // whether piece elimination is animated
  Int8    BlockSet;                          // Which block set to use
  Boolean ColorIcons;                        // whether to draw control icons in 256 color mode, now unused
  Boolean Sound;                             // whether sound is on or off
  Boolean BlindsEffect;                      // whether to use the blinds effect
  Boolean SkipIntro;                         // whether to skip the intro
  LevelPackPrefs LevelPack[MaxLevelPackPrefs]; // MRU'd level pack prefs
} Preferences;

// Increment kprefVersion when prefs change. This'll ensure we only load prefs that
// this version knows about.

#define kprefVersion 3
#define prefsSaved 0
#define prefsUnsaved 1

// Data for a move
typedef struct {
   Int8 x;                                   // starting point of move
   Int8 y;
   Int8 direction;
   Int8 distance;
} MoveType;

// Data for saving for block check alarms
typedef struct {
} AlarmDataType;

// gfx.c
void RedrawLevel();
void DrawLevel( Boolean drawFX );
void UpdateStats();
void DrawIntro();
void EndSequence();
void BlockCheck(UInt8 x, UInt8 y);
void DrawBM(Int16 bitmapID, UInt16 x, UInt16 y, Boolean isId);
void DrawBM2(Int16 bitmapID, UInt16 x, UInt16 y, int ddx, int ddy, Boolean isId);
void DrawControlIcons();
void WriteAboutInfo();
void UpdateBlocks();

// game.c
void CLevelToLevel(CPlayGround clevel, PlayGround level);
void LevelToCLevel(PlayGround level, CPlayGround clevel);
Boolean LoadLevel(char *levelPackName, Int16 levelnumber, Boolean fNoRedraw);
Boolean MoveBlock(Int8 sx, Int8 sy, Int8 direction, Int16 speed);
Boolean PlayerMakeMove(Int8 sx, Int8 sy, Int8 direction, Int8 dist);
Boolean PlayerMakeMove2(Int8 sx, Int8 sy, Int8 direction, Int8 dist, Boolean fAddScore);
Boolean CompareLevel(PlayGround Array1, PlayGround Array2);
void ReplayLevel(Int8 which);
void AddMoveEvent(Int8 sx, Int8 sy, Int8 direction);
int CalcScore(int nPack);

// sound.c
void SoundEffect(Int16 effect);

// system.c
void LoadPreferences();
void SavePreferences();

// intro.c
void PlayerMakeMoveI(Int16 sx, Int16 sy, Int8 direction, Int16 dist);
void EraseWallsI();
void DrawBMIntro(Int8 id, UInt16 x, UInt16 y);
void DrawBMIntro2(Int8 id, UInt16 x, UInt16 y, int ddx, int ddy);

// pack.c
Boolean LoadLevelFromPack(char *pszPack, int nLevel);
Boolean GetLevelProperty(int nLevel, char *pszProp, char *pszValue, int cbValue);
int FindLevelPackPref(char *psz);

// levelpackform.c
Boolean DoLevelPackForm(Boolean fNoRedraw);
void BeamDatabase(int nCard, LocalID lid, char *pszName, char *pszDescription);
ControlType *GetControlPtr(FormType *pfrm, UInt16 id);
extern int GetLevelPackCount();

// selectlevel.c
void DoSelectLevel();

// congrats.cpp
Boolean InitCongrats();
void InitAnim();
void ExitCongrats();
Boolean Step();
