/******************************************************************************
 *
 * Copyright (c) 1995-2002 PalmSource, Inc. All rights reserved.
 *
 * File: MineRsc.h
 *
 * Release: Palm OS Developer Suite 5 SDK (68K) 4.0
 *
 *****************************************************************************/

// Main Form
#define MainForm						1000
#define MainFormTitle					1001
#define MainFormNewGameButton			1002
//#define MainFormHelpButton				1003

// Lost game dialog box

// Won game dialog box


//
// ALERTS
//

// Game was lost alert
//#define GameLostAlert					1001

// Game was won alert
#define GameWonAlert					1002

// Game was won alert
#define NewGameAlert					1003

// Rom Incompatible Alert
#define RomIncompatibleAlert			1004


//
// Menus
//

#define MainFormMenuBar					1000


// Menu commands
//#define MainOptionsNewGameCmd		100	// removed to eliminate redundancy with New Game button
#define MainOptionsHelpCmd				100
#define MainOptionsPrefCmd				101
#define MainOptionsAboutSeparator		102
#define MainOptionsAboutCmd				103


//
// Strings
//

#define GameTipsStr							1001


// Graphic Bitmaps
#define kFirstBitmap						2001
#define BitmapNumber(n) 					(kFirstBitmap + (n))
#define kFirstCustomBitmap					2101
#define CustomBitmapNumber(n)				(kFirstCustomBitmap + (n))


// Preferences View
#define PrefView							1200
#define PrefViewClassicCheckbox				1201
#define PrefViewOkButton					1202
#define PrefViewCancelButton				1203
#define PrefViewScrollingCheckbox			1204

#define PrefViewDifficultyGroup				1
#define PrefViewDifficultyEasyPBN			1210
#define PrefViewDifficultyIntermediatePBN	1211
#define PrefViewDifficultyMoreDifficultPBN	1212
#define PrefViewDifficultyImpossiblePBN		1213


// Info Dialog Box
#define InfoDialog							1400
#define InfoVersionLabel					1404
