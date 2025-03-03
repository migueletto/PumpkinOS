/******************************************************************************
 *
 * Copyright (c) 1995-2002 PalmSource, Inc. All rights reserved.
 *
 * File: Mine.h
 *
 * Release: Palm OS Developer Suite 5 SDK (68K) 4.0
 *
 * Description:
 *		This is the MineHunt application's header file.
 *
 *****************************************************************************/

#ifndef __MINE_H__
#define __MINE_H__

#include <PalmTypes.h>


/***********************************************************************
 *
 *	Internal Constants
 *
 ***********************************************************************/
#define kMineAppCreator			'mine'
#define kMineAppVersionNum		0x0307	// increment when GameType or MinePrefType structs change

// Define the minimum OS version supported (0S version 3.5)
#define kMinVersion				sysMakeROMVersion(3, 5, 0, sysROMStageRelease, 0)

#define kPieceBitmapWidth		15
#define kPieceBitmapHeight		15
#define kScrollingIncrement		3		// scroll by increments of 1/5 of a row or column
#define kPieceFrameWidth		2

#define kBoardWidth 			(kPieceBitmapWidth * kVisibleColumns)
#define kBoardHeight 			(kPieceBitmapHeight * kVisibleRows)
#define kBoardOriginX			5
#define kBoardOriginY			20
#define kArrowWidth			5
#define kArrowHeight			4

#define kContrast				0x20
#define kMinColorThreshold		kContrast
#define kMaxColorThreshold		(0xFF - kContrast)

#define notScrolling			0x0000
#define scrollUp				0x0001
#define scrollLeft				0x0002
#define scrollDown				0x0004
#define scrollRight				0x0008

enum
	{
	white,
	lightGray,
	darkGray,
	black
	};

enum 
	{
	formFrameColor,
	formFillColor,
	objectFgColor,
	objectFillColor,
	cautionColor,
	warningColor
	};
	

/***********************************************************************
 *
 *	Internal Structures
 *
 ***********************************************************************/
typedef struct PieceCoordType 
	{
	Int16		row;					// 0-based
	Int16		col;					// 0-based
	} PieceCoordType;
	
// States of a game piece
enum 
	{
	covered 		= 0,
	markedMine 		= 1,
	markedUnknown 	= 2,
	uncovered 		= 3
	};
	
typedef struct GamePieceType 
	{
	unsigned mined 		: 1;			// if set, there is a mine at this position
	unsigned state 		: 2;			// covered, uncovered, markedMine, markedUnknown
	unsigned neighbors	: 4;			// number of neighbors
	unsigned mark 		: 1;			// used for computing which pieces to uncover automatically
	} GamePieceType;


// States of the game
typedef struct BoardType 
	{
	PointType		origin;				// board origin (x,y)
	UInt8			numRows;			// number of rows
	UInt8			numCol;				// number of columns
	} BoardType;
	
// States of the game
typedef struct ScrollType 
	{
	PointType		origin;				// board origin (x,y)
	PointType		offset;				// fractional piece offset
	PointType		increment;			// fractional part of a piece to scroll
	UInt32			timeToScroll;
	UInt8			rate;		
				
	unsigned		scrolling 	: 1;
	unsigned		firstDraw 	: 1;
	unsigned		reserved	: 6;
	} ScrollType;
	
// States of the game
typedef enum GameStateType 
	{
	gameInProgress,
	gameWon,
	gameDead,
	gameRestart
	} GameStateType;

typedef struct GameType 
	{
	BoardType		board;				// entire game board
	BoardType		visible;			// visible game board
	ScrollType		scroll;				// scrolling state of board
	GameStateType	state;				// playing state of game
	
	// state of each of the pieces in the game board			
	GamePieceType	piece[32][32];	
	
	Int16			minesLeft;			// number of mines left to uncover
	UInt8			numMines;			// number of mines
	
	unsigned		restored 	: 1;	// if set, the game was restored from preferences
	unsigned		colorMode 	: 1;	// distinguish between color and grayscale games
	unsigned		reserved 	: 6;
	
	IndexedColorType baseColor;			// colors used when drawing pieces and frames
	IndexedColorType lightColor;
	IndexedColorType darkColor;
	} GameType;
	
typedef enum SquareGraphicType 
	{
	coveredSquareGraphic = 0,
	markedUnknownSquareGraphic,	// bitmap showing piece marked as unknown by the player
	markedMineSquareGraphic,	// bitmap showing piece marked as a mine by the player
	mineSquareGraphic,
	notMineSquareGraphic,		// bitmap showing unmined piece that was marked as a mine
	emptySquareGraphic,			// bitmap for uncovered piece with no mine
	lastSquareGraphic
	} SquareGraphicType;
	

// App preferences structures

// Game difficulty
typedef enum DifficultyType 
	{
	difficultyEasy = 0,			// must start at zero for mapping tables to work correctly
	difficultyIntermediate,
	difficultyMoreDifficult,
	difficultyImpossible
	} DifficultyType;

#define defaultDifficulty		difficultyIntermediate;
	
// Preferences resource type
typedef struct MinePrefType 
	{
	UInt32			signature;			// signature for preferences resource validation
	GameType		game;				// saved game info
	DifficultyType	difficulty;			// difficulty level
	Boolean			classicLook;
	Boolean			autoScroll;
  UInt32 kMaxRows;
  UInt32 kMaxColumns;
	} MinePrefType;

#define	minePrefSignature	'MiNe'	


#endif	// __MINE_H__
