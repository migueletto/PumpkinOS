/******************************************************************************
 *
 * Copyright (c) 1995-2002 PalmSource, Inc. All rights reserved.
 *
 * File: MineApp.c
 *
 * Release: Palm OS Developer Suite 5 SDK (68K) 4.0
 *
 *****************************************************************************/

#include <PalmOS.h>

#include <PalmUtils.h>

#include "Mine.h"
#include "MineRsc.h"
#include "pumpkin.h"
#include "debug.h"


// #define DEBUG_SAME_GAME			1		// always use same random seed


/***********************************************************************
 *	Constants
 ***********************************************************************/
#define numShuffleMoves		700				// Number of moves the game board is shuffled

#define kRedraw				true			// arguments to InitPieces()
#define kDoNotRedraw		false

// Mines left display bounds and font
#define mineCountLeft		74
#define mineCountTop		1
#define mineCountWidth		20
#define mineCountHeight		10
#define mineCountFontID		boldFont

#define pieceFontID			boldFont		// neighbor count font


/***********************************************************************
 *	Global variables
 ***********************************************************************/
static GameType	gGame;
static MinePrefType	gMinePref;

static WinHandle gOffscreenBoardWinH = 0;			// window for drawing offscreen board 
static WinHandle gGrayPieces = 0;					// window used for drawing gray piece bitmaps
static WinHandle gColorPieces[lastSquareGraphic];	// array of windows used for drawing color piece bitmaps

static const RGBColorType gClassicColors[] = 		// fixed colors used by classic mode
	{
	{0, 0x33, 0x66, 0xFF},
	{0, 0xDD, 0xDD, 0xDD},
	{0, 0x00, 0x00, 0x00},
	{0, 0xDD, 0xDD, 0xDD},
	{0, 0x33, 0x00, 0xDD},
	{0, 0xFF, 0x00, 0x00}
	};
static RGBColorType gUIColors[6];					// colors used by non-classic mode


/***********************************************************************
 *	Prototypes
 ***********************************************************************/
static Boolean 	FindNearestCoveredPiece(Coord left, Coord top, Coord right, Coord bottom, PieceCoordType* startP, PieceCoordType* foundPieceP);
static void 	DeadlyMove(void);
static void 	Victory(void);
static Boolean 	CheckIfWon(void);
static Boolean 	MapPenPosition(Int16 penX, Int16 penY, PieceCoordType* coordP);
static Boolean  HandleManualScroll(Boolean* inBoundsP, PieceCoordType* coordP);
static Boolean 	HandlePenDown(Int16 penX, Int16 penY, Boolean marking, Boolean* inBoundsP);

static void 	DrawTopBorder(Boolean drawBorder);
static void 	DrawLeftBorder(Boolean drawBorder);
static void 	DrawBottomBorder(Boolean drawBorder);
static void 	DrawRightBorder(Boolean drawBorder);
static void 	DrawScrollingBorders(void);
static void 	UpdateScrollView(void);
static void 	UpdateAutoScrollCoordinates(PieceCoordType* coordP);			
static void 	UpdateManualScrollCoordinates(PieceCoordType* penDownCoordP, PieceCoordType* coordP);

static void 	UpdateMinesLeftDisplay(void);
static void 	GetPieceRectangle(Int16 row, Int16 col, RectanglePtr rectP);
static void 	SetNumberColor(UInt8 number);
static void 	DrawPiece(WinHandle dstWinH, Int16 row, Int16 col);
static void 	DrawGameBoard(void);

static void 	InitColorTheme(void);
static void 	InitColors(void);
static Boolean 	ColorThemeChanged(void);
static void 	DrawPieceFrame(void);
static void 	DrawBitmap(WinHandle winHandle, Int16 resID, Int16 x, Int16 y);
static void 	InitPieces(Boolean redraw);
static void 	NewGameBoard(UInt32 moves);
static void 	LoadGameBoard(void);
static void 	SaveGameBoard(void);

static Err 		StartApplication(void);
static void 	StopApplication(void);
static Boolean 	ChangePreferences(void);
static Err 		RomVersionCompatible(UInt32 requiredVersion, UInt16 launchFlags);
static Boolean 	PromptToStartNewGame(void);

static void 	MainFormInit(FormPtr frm);
static Boolean 	MainFormDoCommand(UInt16 command);
static Boolean 	MainFormHandleEvent(EventPtr event);
static UInt32 	TimeToWait(void);
static Boolean 	AppHandleEvent(EventPtr eventP);
static void 	AppEventLoop(void);



/***********************************************************************
 *
 * FUNCTION:    FindNearestCoveredPiece
 *
 * DESCRIPTION:	Find the nearest covered piece.
 *
 * PARAMETERS:	none
 *
 * RETURNED:	nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			vmk		10/8/95		Initial Version
 *
 ***********************************************************************/
static Boolean FindNearestCoveredPiece(Coord left, Coord top, Coord right, Coord bottom, 
										PieceCoordType* startP, PieceCoordType* foundPieceP)
{
	Int32 	leastDistance = kMaxRows * kMaxRows + kMaxColumns * kMaxColumns;
	Int32	distance;
	Boolean found = false;
	Coord	i, j;
	
	for (i = top; i <= bottom; i++)
		for (j = left; j <= right; j++)
			{
			if (gGame.piece[i][j].state == covered)
				{
				distance = (startP->col - j) * (startP->col - j) + (startP->row - i) * (startP->row - i);
				
				if (distance < leastDistance)
					{
					distance = leastDistance;
					foundPieceP->col = j;
					foundPieceP->row = i;
					found = true;
					}
				}
			}
	
	return found;
}


/***********************************************************************
 *
 * FUNCTION:    DeadlyMove
 *
 * DESCRIPTION:	Uncovers all mined pieces which are not marked mined
 *				and puts up the "end of game" dialog.
 *
 * PARAMETERS:	none
 *
 * RETURNED:	nothing.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			vmk		10/08/95	Initial Version
 *			roger	01/09/97	Obey the system game sound setting
 *
 ***********************************************************************/
static void DeadlyMove()
{
	UInt16		i, j;
	GamePieceType*	pieceP;
	
	// Uncover all mined pieces which are not marked as mined
	for (i = 0; i < gGame.board.numRows; i++)
		for (j = 0; j < gGame.board.numCol; j++)
			{
			pieceP = &gGame.piece[i][j];
			if (pieceP->mined && pieceP->state != markedMine)
				pieceP->state = uncovered;
			}
			
	DrawGameBoard();
}


/***********************************************************************
 *
 * FUNCTION:    Victory
 *
 * DESCRIPTION:	Displays the "victory" dialog box.
 *
 * PARAMETERS:	none
 *
 * RETURNED:	nothing.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			vmk		10/8/95		Initial Version
 *
 ***********************************************************************/
static void Victory()
{
	FrmAlert(GameWonAlert);
}


/***********************************************************************
 *
 * FUNCTION:    CheckIfWon
 *
 * DESCRIPTION:	Check if the game has been won.
 *
 * PARAMETERS:	none
 *
 * RETURNED:	true if won.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			vmk		10/8/95		Initial Version
 *
 ***********************************************************************/
static Boolean CheckIfWon()
{
	UInt16			i, j;
	GamePieceType*	pieceP;
	
	// If we find a mined piece which is not marked as "mined" or
	// any other piece which is not uncovered, the game is not won
	for (i = 0; i < gGame.board.numRows; i++)
		for (j = 0; j < gGame.board.numCol; j++)
			{
			pieceP = &gGame.piece[i][j];
			if ((pieceP->mined && pieceP->state != markedMine) ||
				(!pieceP->mined && pieceP->state != uncovered))
				return(false);
			}
	
	return true;
}


/***********************************************************************
 *
 * FUNCTION:    MapPenPosition
 *
 * DESCRIPTION:	Map a screen-relative pen position to a game piece
 *				coordinate
 *
 * PARAMETERS:	penX	-- display-relative x position
 *				penY	-- display-relative y position
 *				coordP	-- MemPtr to store for coordinate
 *
 * RETURNED:	true if the pen went down on a valid game piece; false if not
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			vmk		10/8/95		Initial Version
 *
 ***********************************************************************/
static Boolean MapPenPosition(Int16 penX, Int16 penY, PieceCoordType* coordP)
{
	Int16		x;
	Int16		y;
	RectangleType	rect;
	
	// Map display relative coordinates to window-relative
	x = (Int16) penX;
	y = (Int16) penY;
	WinDisplayToWindowPt(&x, &y);
	
	rect.topLeft.x = gGame.board.origin.x;
	rect.topLeft.y = gGame.board.origin.y;
	rect.extent.x = (gGame.visible.numCol * kPieceBitmapWidth);
	rect.extent.y = (gGame.visible.numRows * kPieceBitmapHeight);
	
	if (! RctPtInRectangle(x, y, &rect))
		return(false);
	
	// Convert to board-relative coordinates
	x -= gGame.board.origin.x;
	y -= gGame.board.origin.y;
	
	if (x < 0)
		{
		ErrDisplay("board x is negative");
		x = 0;
		}
	if (y < 0)
		{
		ErrDisplay("board y is negative");
		y = 0;
		}
	
	// Get the piece position
	coordP->col = x / kPieceBitmapWidth + gGame.visible.origin.x;
	coordP->row = y / kPieceBitmapHeight + gGame.visible.origin.y;
	if (coordP->col > (gGame.board.numCol - 1))
		{
		ErrDisplay("column out of bounds");
		coordP->col = gGame.board.numCol - 1;
		}
	if (coordP->row > (gGame.board.numRows - 1))
		{
		ErrDisplay("row out of bounds");
		coordP->row = gGame.board.numRows - 1;
		}
	
	return true;
}



/***********************************************************************
 *
 * FUNCTION:    HandleManualScroll
 *
 * DESCRIPTION:	This function determines if manual scrolling, or a 
 *				"cancel tile uncover" action, needs to be processed.
 *
 *	Manual scrolling is activated if the current pen coordinate has moved more 
 *	than one piece away from the initial pen down position.  Scrolling is
 *	suspended when the pen moves in an opposite direction from the original
 *	pen movement.  Scrolling terminates with the pen up, although scrolling
 *	continues until a piece-aligned boundary is reached.
 *
 * 	Scrolling manually drags the  board.  Dragging the board down is equivalent 
 *	to scrolling up.  Dragging the board right is equivalent to scrolling left. 
 *
 *	If the pen up coordinate and the pen down coordinate are on adjacent pieces,
 *	this is interpreted as a "cancel" action, and this function returns true.
 *
 * PARAMETERS:	inBoundsP -- true if pen down is within bounds of board
 *				coordP	  -- pen down coordinate
 * 
 * RETURNED:	Boolean  -- true if pen movement is interpreted as
 *						    canceling the uncovering of a piece
 *
 * REVISION HISTORY:
 *				Name	Date		Description
 *				----	----		-----------
 *				acs		07/01/00	Initial version
 *
 ***********************************************************************/
static Boolean HandleManualScroll(Boolean* inBoundsP, PieceCoordType* coordP)
{
	PieceCoordType scrollCoord;
	PieceCoordType penDownCoord;
	Int16 newX, newY;
	UInt16 saveDirection;
	UInt16 direction = notScrolling;
	Boolean penDown = true;
	Boolean cancelUncover = false;
	
	penDownCoord = *coordP;
	
	while (penDown)
		{
		SysTaskDelay(SysTicksPerSecond() / 5);
		EvtGetPen(&newX, &newY, &penDown);
	
		*inBoundsP = MapPenPosition(newX, newY, coordP);

		// determine if this is a manual scroll:  if pen moved more than one piece away
		// from initial pen down position, then scroll
		if ((! gGame.scroll.scrolling) &&
		    ((coordP->col - penDownCoord.col > 1) || (coordP->row - penDownCoord.row > 1) ||
		     (coordP->col - penDownCoord.col < -1) || (coordP->row - penDownCoord.row < -1)))
		    {
		    scrollCoord = *coordP;
			gGame.scroll.scrolling = true;
			gGame.scroll.firstDraw = true;
			}
		
		if (gGame.scroll.scrolling)
			{
			saveDirection = direction;
			
			// Determine current scrolling direction by comparing original pen down position
			// with current pen down position.  
			if (penDownCoord.col > coordP->col)
				direction |= scrollRight;
			else if (penDownCoord.col < coordP->col)
				direction |= scrollLeft;
			if (penDownCoord.row > coordP->row)
				direction |= scrollDown;
			else if (penDownCoord.row < coordP->row)
				direction |= scrollUp;
				
			// don't allow scrolling to suspend until piece-aligned boundary reached
			if ((gGame.scroll.offset.x != 0) || (gGame.scroll.offset.y != 0))
				{
				direction = saveDirection;
				UpdateManualScrollCoordinates(&penDownCoord, &scrollCoord);
				}
			// suspend scrolling if pen moved in opposite direction
			else if ((! ((direction & scrollUp) && (direction & scrollDown))) &&
				(! ((direction & scrollLeft) && (direction & scrollRight))))
				{
		    	scrollCoord = *coordP;	// not opposite direction, so update scrolling coord
				UpdateManualScrollCoordinates(&penDownCoord, &scrollCoord);
				}
			}
	
		// don't stop scrolling until visible board is on piece-aligned boundary
		if (! penDown) 
			{
			while ((gGame.scroll.offset.x != 0) || (gGame.scroll.offset.y != 0))
				{
				SysTaskDelay(SysTicksPerSecond() / 5);
				UpdateScrollView();
				}

			gGame.scroll.scrolling = false;
			DrawScrollingBorders();
			}
		}	
		
	// if penUp coord != penDown coord, interpret this as a cancel action
	if (! ((coordP->col == penDownCoord.col) && (coordP->row == penDownCoord.row)))
		cancelUncover = true;
		
	return cancelUncover;
}


/***********************************************************************
 *
 * FUNCTION:    HandlePenDown
 *
 * DESCRIPTION:	Handles a pen down
 *
 * PARAMETERS:	penX		-- display-relative x position
 *				penY		-- display-relative y position
 *				marking		-- if true, pen down on game pieces is
 *									interpreted as marking
 *				inBoundsP	-- if pen landed in game board bounds, *inBoundsP
 *									will be set to true, otherwise to false
 *
 * RETURNED:	true if handled, false if not
 *
 * REVISION HISTORY:
 *				Name	Date		Description
 *				----	----		-----------
 *				vmk		10/08/95	Initial Version
 *				acs		07/01/00	version 3.6
 *
 ***********************************************************************/
static Boolean HandlePenDown(Int16 penX, Int16 penY, Boolean marking, Boolean* inBoundsP)
{
	PieceCoordType		coord;
	PieceCoordType		nearestCoord;
	Coord				i, j;
	GamePieceType		piece, *pieceP;
	Int16				savedMinesLeft;
	Boolean				deadlyMove = false;
	Boolean				victory = false;
	Boolean				uncoveredPieceVisible;
	Boolean				cancelUncover;
	
	// map pen position to game board row and column
	*inBoundsP = MapPenPosition(penX, penY, &coord);

	// bail if game over when auto scrolling, or pen position not on a game piece
	if (((gGame.state != gameInProgress) && gMinePref.autoScroll) || (! *inBoundsP))
		return false;
	
	// process manual scrolling and cancel action here
	if (! gMinePref.autoScroll)
		{
		cancelUncover = HandleManualScroll(inBoundsP, &coord);
		if (cancelUncover)
			return true;
		}
		
	if (gGame.scroll.scrolling || (! *inBoundsP))
		return true;
		
	// save mines left value for update optimization
	savedMinesLeft = gGame.minesLeft;
	
	piece = gGame.piece[coord.row][coord.col];
	pieceP = &gGame.piece[coord.row][coord.col];

	// nothing to do if we're already uncovered		
	if ( piece.state == uncovered )
		return true;
	
	// if marking
	if (marking)
		{
		if (piece.state == covered)
			{
			pieceP->state = markedMine;
			gGame.minesLeft--;
			}
		else if (piece.state == markedMine)
			{
			pieceP->state = markedUnknown;
			gGame.minesLeft++;
			}
		else if (piece.state == markedUnknown)
			pieceP->state = covered;
		}
	
	// Otherwise, if marked as a mine
	else if (pieceP->state == markedMine)
		return true;				// protect from accidental explosion
	
	// Otherwise, uncover the piece
	else
		{
		pieceP->state = uncovered;
		
		if (pieceP->mined)
			deadlyMove = true;
		
		// If piece is not mined and has no mined neighbors, automatically uncover any  
		// neighboring pieces which are not mined and which are neighbors of other uncovered
		// pieces without mined neighbors.
		if  ((pieceP->neighbors == 0) && (! pieceP->mined))
			{
			Boolean somethingMarked;
			
			// Clear marks
			for (i = 0; i < gGame.board.numRows; i++)
				for (j = 0; j < gGame.board.numCol; j++)
					{
					gGame.piece[i][j].mark = 0;
					}
			
			pieceP->mark = 1;
			
			do	
				{
				somethingMarked = false;
				for (i = 0; i < gGame.board.numRows; i++)
					for (j = 0; j < gGame.board.numCol; j++)
						{
						pieceP = &gGame.piece[i][j];
						if (pieceP->mark || pieceP->mined || pieceP->state == uncovered) 
							continue;

						// Look at the 3 neighbors to the left
						if (j > 0)
							{
							if (gGame.piece[i][j-1].mark && (! gGame.piece[i][j-1].neighbors))
								goto Redraw;
							if (i > 0 && gGame.piece[i-1][j-1].mark && (! gGame.piece[i-1][j-1].neighbors))
								goto Redraw;
							if (i < (gGame.board.numRows - 1) && gGame.piece[i+1][j-1].mark &&
									(! gGame.piece[i+1][j-1].neighbors))
								goto Redraw;
							}
							
						// Take care of the 3 neighbors to the right
						if  (j < (gGame.board.numCol - 1))
							{
							if (gGame.piece[i][j+1].mark && ! gGame.piece[i][j+1].neighbors)
								goto Redraw;
							if (i > 0 && gGame.piece[i-1][j+1].mark && (! gGame.piece[i-1][j+1].neighbors))
								goto Redraw;
							if (i < (gGame.board.numRows - 1) && gGame.piece[i+1][j+1].mark &&
									(! gGame.piece[i+1][j+1].neighbors))
								goto Redraw;
							}
						
						// Take care of the neighbor above
						if (i > 0 && gGame.piece[i-1][j].mark && (! gGame.piece[i-1][j].neighbors))
							goto Redraw;
						
						// Take care of the neighbor below
						if (i < (gGame.board.numRows - 1) && gGame.piece[i+1][j].mark &&
									(! gGame.piece[i+1][j].neighbors))
							goto Redraw;

						continue;
Redraw:
						pieceP->mark = 1;
						if (pieceP->state == markedMine)	// If it was marked as a mine, it was wrong!
							gGame.minesLeft++;				// Bump minesLeft counter before we uncover!
						pieceP->state = uncovered;
						// Draw clipped to the visible area
						if (i >= gGame.visible.origin.y && i < gGame.visible.origin.y + gGame.visible.numRows &&
							j >= gGame.visible.origin.x && j < gGame.visible.origin.x + gGame.visible.numCol)
							{
							DrawPiece(0/*dstWin*/, i, j);
							}
						somethingMarked = true;
						}	
				}
			while (somethingMarked);
			
			} // If piece is not mined and has no mined neighbors...
			
		} // Otherwise, uncover the piece
	
	// Check if we died or won
	if (deadlyMove)
		gGame.state = gameDead;
	else if ((victory = CheckIfWon()) != false)
		gGame.state = gameWon;
		
	// Draw the piece which was tapped
	DrawPiece(0/*dstWin*/, coord.row, coord.col);

	// Update mines left display
	if (gGame.minesLeft != savedMinesLeft)
		UpdateMinesLeftDisplay();
	
	// Check for end of game
	if (deadlyMove)
		DeadlyMove();
	else if (victory)
		Victory();
	else if (gMinePref.autoScroll)
		{
		// Make sure some uncovered pieces are visible.  Scroll if not.
		uncoveredPieceVisible = false;
#define coveredBorder 1	
		for (i = gGame.visible.origin.y + coveredBorder; i < gGame.visible.origin.y + gGame.visible.numRows - coveredBorder; i++)
			for (j = gGame.visible.origin.x + coveredBorder; j < gGame.visible.origin.x + gGame.visible.numCol - coveredBorder; j++)
				{
				if (gGame.piece[i][j].state == covered)
					{
					uncoveredPieceVisible = true;
					break;
					}
				}
		
		if (! uncoveredPieceVisible)
			{
			// Try to find the nearest covered piece on the visible board.  This is likely to happen, 
			// is cheaper than searching the whole board.
			if (! FindNearestCoveredPiece(gGame.visible.origin.x, gGame.visible.origin.y, 
				gGame.visible.origin.x + gGame.visible.numCol - 1, 
				gGame.visible.origin.y + gGame.visible.numRows - 1, 
				&coord, &nearestCoord))
				{
				// There isn't a covered piece on the visible board, so search the whole board.
				FindNearestCoveredPiece(0, 0, gGame.board.numCol - 1, gGame.board.numRows - 1, 
					&coord, &nearestCoord);
				}
			gGame.scroll.scrolling = true;
			}
		// If the user taps on the edge of the visible area, center it so they can work on
		// the pieces nearby.
		else if (coord.col == gGame.visible.origin.x || 
			coord.row == gGame.visible.origin.y ||
			coord.col == gGame.visible.origin.x + gGame.visible.numCol - 1 ||
			coord.row == gGame.visible.origin.y + gGame.visible.numRows - 1)
			{
			nearestCoord.row = coord.row;
			nearestCoord.col = coord.col;
			gGame.scroll.scrolling = true;
			}
			
		if (gGame.scroll.scrolling)
			{
			gGame.scroll.firstDraw = true;
			UpdateAutoScrollCoordinates(&nearestCoord);
			}
		}
	
	return true;
}


//#pragma mark -
/***********************************************************************
 *
 * FUNCTION:    DrawTopBorder
 *
 * DESCRIPTION:	Draw or erase the top border.
 *
 * PARAMETERS:	drawBorder -- 	if true, draw the border
 *								if false, draw the arrow
 *
 * RETURNED:	nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			acs		3/18/00		Initial Version
 *
 ***********************************************************************/
static void DrawTopBorder(Boolean drawBorder)
{
	Int16 lineOriginX, lineOriginY, endPtX;
	IndexedColorType savedColor;
	
	// Draw the border with alternating shades if at the edge of the board;
	// otherwise, erase the border with base color
	if (! drawBorder)
		savedColor = WinSetForeColor(gGame.baseColor);
	else
		savedColor = WinSetForeColor(gGame.lightColor);

	lineOriginX = kBoardOriginX - 1;
	lineOriginY = kBoardOriginY - 1;
	endPtX = kBoardOriginX + kBoardWidth;
	WinDrawLine(lineOriginX, lineOriginY, endPtX, lineOriginY);

	if (drawBorder)
		WinSetForeColor(gGame.darkColor);
	lineOriginX--;
	lineOriginY--;
	endPtX++;
	WinDrawLine(lineOriginX, lineOriginY, endPtX, lineOriginY);

	if (drawBorder)
		WinSetForeColor(gGame.lightColor);
	lineOriginX--;
	lineOriginY--;
	endPtX++;
	WinDrawLine(lineOriginX, lineOriginY, endPtX, lineOriginY);

	if (drawBorder)
		WinSetForeColor(gGame.darkColor);
	lineOriginX--;
	lineOriginY--;
	endPtX++;
	WinDrawLine(lineOriginX, lineOriginY, endPtX, lineOriginY);

	if (drawBorder)
		WinSetForeColor(gGame.lightColor);
	lineOriginX--;
	lineOriginY--;
	endPtX++;
	WinDrawLine(lineOriginX, lineOriginY, endPtX, lineOriginY);
	
	WinSetForeColor(savedColor);
}	

	
/***********************************************************************
 *
 * FUNCTION:    DrawLeftBorder
 *
 * DESCRIPTION:	Draw or erase the left border.
 *
 * PARAMETERS:	drawBorder -- 	if true, draw the border
 *								if false, draw the arrow
 *
 * RETURNED:	nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			acs		3/18/00		Initial Version
 *
 ***********************************************************************/
static void DrawLeftBorder(Boolean drawBorder)
{
	Int16 lineOriginX, lineOriginY, endPtY;
	IndexedColorType savedColor;

	// Draw the border with alternating shades if at the edge of the board;
	// otherwise, erase the border with base color
	if (! drawBorder)
		savedColor = WinSetForeColor(gGame.baseColor);
	else
		savedColor = WinSetForeColor(gGame.lightColor);

	lineOriginX = kBoardOriginX - 1;
	lineOriginY = kBoardOriginY - 1;
	endPtY = kBoardOriginY + kBoardHeight;	
	WinDrawLine(lineOriginX, lineOriginY, lineOriginX, endPtY);
	
	if (drawBorder)
		WinSetForeColor(gGame.darkColor);
	lineOriginX--;
	lineOriginY--;
	endPtY++;
	WinDrawLine(lineOriginX, lineOriginY, lineOriginX, endPtY);
	
	if (drawBorder)
		WinSetForeColor(gGame.lightColor);
	lineOriginX--;
	lineOriginY--;
	endPtY++;
	WinDrawLine(lineOriginX, lineOriginY, lineOriginX, endPtY);
	
	if (drawBorder)
		WinSetForeColor(gGame.darkColor);
	lineOriginX--;
	lineOriginY--;
	endPtY++;
	WinDrawLine(lineOriginX, lineOriginY, lineOriginX, endPtY);
	
	if (drawBorder)
		WinSetForeColor(gGame.lightColor);
	lineOriginX--;
	lineOriginY--;
	endPtY++;
	WinDrawLine(lineOriginX, lineOriginY, lineOriginX, endPtY);
	
	WinSetForeColor(savedColor);
}
		
	
/***********************************************************************
 *
 * FUNCTION:    DrawBottomBorder
 *
 * DESCRIPTION:	Draw or erase the bottom border.
 *
 * PARAMETERS:	drawBorder -- 	if true, draw the border
 *								if false, draw the arrow
 *
 * RETURNED:	nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			acs		3/18/00		Initial Version
 *
 ***********************************************************************/
static void DrawBottomBorder(Boolean drawBorder)
{
	Int16 lineOriginX, lineOriginY, endPtX;
	IndexedColorType savedColor;

	// Draw the border with alternating shades if at the edge of the board;
	// otherwise, erase the border with base color
	if (! drawBorder)
		savedColor = WinSetForeColor(gGame.baseColor);
	else
		savedColor = WinSetForeColor(gGame.lightColor);

	lineOriginX = kBoardOriginX + kBoardWidth;
	lineOriginY = kBoardOriginY + kBoardHeight;
	endPtX = kBoardOriginX - 1;
	WinDrawLine(lineOriginX, lineOriginY, endPtX, lineOriginY);
	
	if (drawBorder)
		WinSetForeColor(gGame.darkColor);
	lineOriginX++;
	lineOriginY++;
	endPtX--; 
	WinDrawLine(lineOriginX, lineOriginY, endPtX, lineOriginY);
	
	if (drawBorder)
		WinSetForeColor(gGame.lightColor);
	lineOriginX++;
	lineOriginY++; 
	endPtX--; 
	WinDrawLine(lineOriginX, lineOriginY, endPtX, lineOriginY);
	
	if (drawBorder)
		WinSetForeColor(gGame.darkColor);
	lineOriginX++;
	lineOriginY++; 
	endPtX--; 
	WinDrawLine(lineOriginX, lineOriginY, endPtX, lineOriginY);
	
	if (drawBorder)
		WinSetForeColor(gGame.lightColor);
	lineOriginX++;
	lineOriginY++; 
	endPtX--; 
	WinDrawLine(lineOriginX, lineOriginY, endPtX, lineOriginY);
		
	WinSetForeColor(savedColor);
}		
	
	
/***********************************************************************
 *
 * FUNCTION:    DrawRightBorder
 *
 * DESCRIPTION:	Draw or erase the right border.
 *
 * PARAMETERS:	drawBorder -- 	if true, draw the border
 *								if false, draw the arrow
 *
 * RETURNED:	nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			acs		3/18/00		Initial Version
 *
 ***********************************************************************/
static void DrawRightBorder(Boolean drawBorder)
{
	Int16 lineOriginX, lineOriginY, endPtY;
	IndexedColorType savedColor;

	// Draw the border with alternating shades if at the edge of the board;
	// otherwise, erase the border with base color
	if (! drawBorder)
		savedColor = WinSetForeColor(gGame.baseColor);
	else
		savedColor = WinSetForeColor(gGame.lightColor);

	lineOriginX = kBoardOriginX + kBoardWidth;
	lineOriginY = kBoardOriginY + kBoardHeight;
	endPtY = kBoardOriginY - 1;
	WinDrawLine(lineOriginX, lineOriginY, lineOriginX, endPtY);
	
	if (drawBorder)
		WinSetForeColor(gGame.darkColor);
	lineOriginX++;
	lineOriginY++; 
	endPtY--; 
	WinDrawLine(lineOriginX, lineOriginY, lineOriginX, endPtY);
	
	if (drawBorder)
		WinSetForeColor(gGame.lightColor);
	lineOriginX++;
	lineOriginY++; 
	endPtY--; 
	WinDrawLine(lineOriginX, lineOriginY, lineOriginX, endPtY);
	
	if (drawBorder)
		WinSetForeColor(gGame.darkColor);
	lineOriginX++;
	lineOriginY++; 
	endPtY--; 
	WinDrawLine(lineOriginX, lineOriginY, lineOriginX, endPtY);
	
	if (drawBorder)
		WinSetForeColor(gGame.lightColor);
	lineOriginX++;
	lineOriginY++; 
	endPtY--; 
	WinDrawLine(lineOriginX, lineOriginY, lineOriginX, endPtY);

	WinSetForeColor(savedColor);
}


/***********************************************************************
 *
 * FUNCTION:    DrawScrollingBorders
 *
 * DESCRIPTION:	Draw the borders and scrolling arrows for the game board.
 *				Called from DrawGameBoard() when drawing the board for the 
 *				first time, and by HandlePenDown() when scrolling.
 *
 * PARAMETERS:	none
 *
 * RETURNED:	nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			acs		3/18/00		Initial Version
 *
 ***********************************************************************/
static void DrawScrollingBorders()
{
	if (gGame.visible.origin.y == 0) 
		{
		if (gGame.scroll.offset.y == 0)	
			DrawTopBorder(true);			// draw border
		else if (gGame.scroll.offset.y == kScrollingIncrement)
			DrawTopBorder(false);			// erase border
		}
		
	if (gGame.visible.origin.x == 0) 
		{
		if (gGame.scroll.offset.x == 0)	
			DrawLeftBorder(true);			// draw border
		else if (gGame.scroll.offset.x == kScrollingIncrement)
			DrawLeftBorder(false);			// erase border
		}
		
	if ((gGame.visible.origin.y == kMaxRows - kVisibleRows) && (gGame.scroll.offset.y == 0))
		DrawBottomBorder(true);				// draw border
	else if ((gGame.visible.origin.y == kMaxRows - kVisibleRows - 1) &&
			(gGame.scroll.offset.y == kPieceBitmapHeight - kScrollingIncrement))
		DrawBottomBorder(false);			// erase border
		
	if ((gGame.visible.origin.x == kMaxColumns - kVisibleColumns) && (gGame.scroll.offset.x == 0))
		DrawRightBorder(true);				// draw border
	else if ((gGame.visible.origin.x == kMaxColumns - kVisibleColumns - 1) &&
			(gGame.scroll.offset.x == kPieceBitmapWidth - kScrollingIncrement))
		DrawRightBorder(false);				// erase border
}


/***********************************************************************
 *
 * FUNCTION:    UpdateScrollView
 *
 * DESCRIPTION: This function udpates the board's visible origin and 
 *				scrolling offsets while scrolling.
 *				
 * 				It is called from AppEventLoop() for automatic scrolling, 
 *				and from UpdateManualScrollCoordinates() and 
 *				HandleManualScroll() when scrolling manually.
 *
 * PARAMETERS:	none
 *
 * RETURNED:	nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			acs		07/01/00	Initial Version
 *
 ***********************************************************************/
static void UpdateScrollView()
{
	if (gGame.visible.origin.x < gGame.scroll.origin.x)			// scroll right
		{
		gGame.scroll.offset.x += gGame.scroll.increment.x;

		if (gGame.scroll.offset.x == kPieceBitmapWidth)
			{
			gGame.visible.origin.x++;
			gGame.scroll.offset.x = 0;
			}
		}
	else if (gGame.visible.origin.x > gGame.scroll.origin.x)	// scroll left
		{
		gGame.scroll.offset.x += gGame.scroll.increment.x;
		
		if (gGame.scroll.offset.x < 0)
			{
			gGame.visible.origin.x--;
			gGame.scroll.offset.x = kPieceBitmapWidth - kScrollingIncrement;
			}
		}
	else if (gGame.scroll.increment.x)		// scroll final column left or right
		{
		if ((gGame.scroll.offset.x == kPieceBitmapWidth) || (gGame.scroll.offset.x == 0))
			{
		  	gGame.scroll.increment.x = 0;	// scroll completed
		  	gGame.scroll.offset.x = 0;
		  	}
		else
			gGame.scroll.offset.x += gGame.scroll.increment.x;
		}
					
	if (gGame.visible.origin.y < gGame.scroll.origin.y)			// scroll down
		{
		gGame.scroll.offset.y += gGame.scroll.increment.y;

		if (gGame.scroll.offset.y == kPieceBitmapHeight)
			{
			gGame.visible.origin.y++;
			gGame.scroll.offset.y = 0;
			}
		}
	else if (gGame.visible.origin.y > gGame.scroll.origin.y)	// scroll up
		{				
		gGame.scroll.offset.y += gGame.scroll.increment.y;
		
		if (gGame.scroll.offset.y < 0)
			{
			gGame.visible.origin.y--;
			gGame.scroll.offset.y = kPieceBitmapHeight - kScrollingIncrement;
			}
		}
	else if (gGame.scroll.increment.y)		// scroll final row up or down
		{
		if ((gGame.scroll.offset.y == kPieceBitmapHeight) || (gGame.scroll.offset.y == 0))
			{
			gGame.scroll.increment.y = 0;	// scroll completed
			gGame.scroll.offset.y = 0;
			}
		else
			gGame.scroll.offset.y += gGame.scroll.increment.y;
		}
	
	DrawGameBoard();
}


/***********************************************************************
 *
 * FUNCTION:    UpdateAutoScrollCoordinates
 *
 * DESCRIPTION:	This function initializes the board's scrolling variables
 *				when scrolling automatically.
 *
 *				This function is called from HandlePenDown().
 *
 * PARAMETERS:	coordP - the destination board coordinate
 *
 * RETURNED:	nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			acs		07/01/00	Initial Version
 *
 ***********************************************************************/
static void UpdateAutoScrollCoordinates(PieceCoordType* coordP)			
{
	Int16	distance, distanceX, distanceY;
	
	// Center the covered piece in the visible board.
	gGame.scroll.origin.x = coordP->col - gGame.visible.numCol / 2;
	gGame.scroll.origin.y = coordP->row - gGame.visible.numRows / 2;
	
	// Make sure the visible board is within the board.
	if (gGame.scroll.origin.x < 0)
		gGame.scroll.origin.x = 0;
	else if (gGame.scroll.origin.x >= gGame.board.numCol - gGame.visible.numCol)
		gGame.scroll.origin.x = gGame.board.numCol - gGame.visible.numCol;

	if (gGame.scroll.origin.y < 0)
		gGame.scroll.origin.y = 0;
	else if (gGame.scroll.origin.y >= gGame.board.numRows - gGame.visible.numRows)
		gGame.scroll.origin.y = gGame.board.numRows - gGame.visible.numRows;
	
	// Determine the distance to scroll.  
	distanceX = gGame.scroll.origin.x - gGame.visible.origin.x;
	
	if (distanceX < 0) 					// scroll left
		{
		distanceX = -distanceX;
		gGame.scroll.increment.x = -kScrollingIncrement;
		}
	else if (distanceX > 0)				// scroll right
		gGame.scroll.increment.x = kScrollingIncrement;
		
	distanceY = gGame.scroll.origin.y - gGame.visible.origin.y;
	
	if (distanceY < 0) 					// scroll up
		{
		distanceY = -distanceY;
		gGame.scroll.increment.y = -kScrollingIncrement;
		}
	else if (distanceY > 0)				// scroll down
		gGame.scroll.increment.y = kScrollingIncrement;
	
	distance = (distanceX > distanceY) ? distanceX : distanceY;
	
	if (distance > 0)
		{
		gGame.scroll.rate = SysTicksPerSecond() / 5;
		gGame.scroll.timeToScroll = TimGetTicks();			// Scroll right away
		}
}


/***********************************************************************
 *
 * FUNCTION:    UpdateManualScrollCoordinates
 *
 * DESCRIPTION:	This function initializes the board's scrolling variables
 *				when scrolling manually.
 *
 *				This function is called from HandlePenDown().
 *
 * PARAMETERS:	penDownCoordP - the pen down board coordinate
 *				curCoordP	  - the current pen down coordinate
 *
 * RETURNED:	nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			acs		07/01/00	Initial Version
 *
 ***********************************************************************/
static void UpdateManualScrollCoordinates(PieceCoordType* penDownCoordP, PieceCoordType* curCoordP)
{
	Boolean scrolling = false;
	
	if (penDownCoordP->col > curCoordP->col)
		{
		// initialize scroll right variables (dragging the board left)
		scrolling = true;
		gGame.scroll.origin.x = gGame.visible.origin.x;
		gGame.scroll.increment.x = kScrollingIncrement;
			
		if (gGame.scroll.origin.x < (kMaxColumns - kVisibleColumns))
			gGame.scroll.origin.x++;
		}
	else if (penDownCoordP->col < curCoordP->col)
		{
		// initialize scroll left (drag right) variables
		scrolling = true;
		gGame.scroll.origin.x = gGame.visible.origin.x;
		gGame.scroll.increment.x = -kScrollingIncrement;
			
		if (gGame.scroll.origin.x > 0)
			gGame.scroll.origin.x--;
		}
		
	if (penDownCoordP->row > curCoordP->row)
		{
		// initialize scroll down (drag up) variables
		scrolling = true;
		gGame.scroll.origin.y = gGame.visible.origin.y;
		gGame.scroll.increment.y = kScrollingIncrement;
			
		if (gGame.scroll.origin.y < (kMaxRows - kVisibleRows))
			gGame.scroll.origin.y++;
		}
	else if (penDownCoordP->row < curCoordP->row)
		{
		// initialize scroll up (drag down) variables
		scrolling = true;
		gGame.scroll.origin.y = gGame.visible.origin.y;
		gGame.scroll.increment.y = -kScrollingIncrement;
			
		if (gGame.scroll.origin.y > 0)
			gGame.scroll.origin.y--;
		}
	
	if (scrolling || (gGame.scroll.increment.x != 0) || (gGame.scroll.increment.y != 0))
		UpdateScrollView();
}


//#pragma mark -
/***********************************************************************
 *
 * FUNCTION:    UpdateMinesLeftDisplay
 *
 * DESCRIPTION:	Update the number of mines left display.
 *
 * PARAMETERS:	none
 *
 * RETURNED:	nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			vmk		04/16/96	Initial Version
 *			acs		07/01/00	version 3.6
 *
 ***********************************************************************/
static void UpdateMinesLeftDisplay()
{
	Char			text[32];
	Char*			cP;
	UInt16			textLen;
	Int16			x, y;
	//FontID			oldFontID;
	RectangleType	r;
	Int16			absMinesLeft;

	WinPushDrawState();
	
	if (gGame.colorMode)
		WinSetBackColor(UIColorGetTableEntryIndex(UIFormFill));
	
	// Erase the old display, first
	r.topLeft.x = mineCountLeft;
	r.topLeft.y = mineCountTop;
	r.extent.x = mineCountWidth;
	r.extent.y = mineCountHeight;
	WinEraseRectangle(&r, 0 /*cornerDiam*/);
	
	/*oldFontID =*/ FntSetFont(mineCountFontID);		// change font
	
	cP = text;
	absMinesLeft = gGame.minesLeft;
	StrIToA(cP, absMinesLeft);
	textLen = StrLen(text);
	
	x = mineCountLeft;
	y = mineCountTop;
	WinDrawChars(text, textLen, x, y);
	
	WinPopDrawState();
}


/***********************************************************************
 *
 * FUNCTION:    GetPieceRectangle
 *
 * DESCRIPTION:	Get the rectangle coordinates of a game piece.  Called
 *				by DrawPiece().
 *
 * PARAMETERS:	row		-- piece row (0-based)
 *				col		-- piece column (0-based)
 *				rectP	-- pointer to rectangle structure
 *
 * RETURNED:	nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			vmk		10/11/95	Initial Version
 *
 ***********************************************************************/
static void GetPieceRectangle(Int16 row, Int16 col, RectanglePtr rectP)
{
	
	ErrFatalDisplayIf(row >= gGame.board.numRows, "bad row");
	ErrFatalDisplayIf(col >= gGame.board.numCol, "bad column");
	ErrFatalDisplayIf(! rectP, "bad param");
	
	rectP->topLeft.x = gGame.board.origin.x + kPieceBitmapWidth * (col - gGame.visible.origin.x);
	rectP->topLeft.y = gGame.board.origin.y + kPieceBitmapHeight * (row - gGame.visible.origin.y);
	rectP->extent.x = kPieceBitmapWidth;
	rectP->extent.y = kPieceBitmapHeight;
}


/***********************************************************************
 *
 * FUNCTION:    SetNumberColor
 *
 * DESCRIPTION:	This function sets the text color to the appropriate
 *				color representing the number of neighboring mines.
 *
 *				This function is called from DrawPiece().
 *
 * PARAMETERS:	number		-- number of neighboring mines
 *
 * RETURNED:	nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			acs		07/01/00	Initial Version
 *
 ***********************************************************************/
static void SetNumberColor(UInt8 number)
 {
 	switch (number)
 		{
 		case 1:
 			WinSetTextColor(UIColorGetTableEntryIndex(UIFormFrame));
 			break;
 		case 2:
 			WinSetTextColor(UIColorGetTableEntryIndex(UIOK));
 			break;
 		case 3:
 			WinSetTextColor(UIColorGetTableEntryIndex(UICaution));
 			break;
 		default:
 			WinSetTextColor(UIColorGetTableEntryIndex(UIWarning));
 			break;
 		}
 }
 
 
/***********************************************************************
 *
 * FUNCTION:    DrawPiece
 *
 * DESCRIPTION:	Draw a game piece.  
 *
 *				This function is called from DrawGameBoard(), when the board 
 *				is drawn for the first time, or when new pieces become visible
 *				due to scrolling.  This function is also called from 
 *				HandlePenDown(), when a visible piece changes its appearance 
 *				due to uncovering, marking, or unmarking.
 *
 * PARAMETERS:	dstWinH	-- destination window MemHandle (0 for current window)
 *				row		-- piece row (0-based)
 *				col		-- piece column (0-based)
 *
 * RETURNED:	nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			vmk		10/08/95	Initial Version
 *			acs		07/01/00	version 3.6
 *
 ***********************************************************************/
static void DrawPiece(WinHandle dstWinH, Int16 row, Int16 col)
{
	RectangleType	rect, srcRect;
	//FontID			oldFontID;
	GamePieceType	piece;
	//Int16			bitmapID = 0;
	Int16			bitmapIndex = 0;
	Boolean			emptySquare = false;

	// Get dst bounds of the game piece
	GetPieceRectangle(row, col, &rect);
		
	// Determine which bitmap should be drawn
	piece = gGame.piece[row][col];
	
	if ((piece.state == uncovered) && (! piece.mined))
		emptySquare = true;

	switch (piece.state)
		{
		case covered:
			bitmapIndex = coveredSquareGraphic;
			break;
		case markedUnknown:
			bitmapIndex = markedUnknownSquareGraphic;
			break;
		case markedMine:
			if (gGame.state != gameDead || piece.mined)
				bitmapIndex = markedMineSquareGraphic;
			else
				bitmapIndex = notMineSquareGraphic;
			break;
		case uncovered:
			bitmapIndex = (piece.mined ? mineSquareGraphic : emptySquareGraphic);
			break;
		default:
			ErrDisplay("unhandled game piece state");
			break;
		}

	srcRect.topLeft.y = 0;
	srcRect.extent.x = kPieceBitmapWidth;
	srcRect.extent.y = kPieceBitmapHeight;

	// Draw the bitmap by copying from the offscreen bitmaps
	if (gGame.colorMode)
		{
		srcRect.topLeft.x = 0;
		WinCopyRectangle(gColorPieces[bitmapIndex], dstWinH, &srcRect, rect.topLeft.x, rect.topLeft.y, winPaint);
		}
	else
		{		
		srcRect.topLeft.x = bitmapIndex * kPieceBitmapWidth;
		WinCopyRectangle(gGrayPieces, dstWinH, &srcRect, rect.topLeft.x, rect.topLeft.y, winPaint);
		}
	
	// If this is an unmined uncovered square, draw the neighbor count
	if (emptySquare && piece.neighbors)
		{
		Char		text[32];
		UInt16		textLen;
		Int16		textHeight, textWidth;
		Int16		x, y;
		WinHandle 	currDrawWindow;
		
		// Set a new draw window, saving the current one
		if (dstWinH)
			currDrawWindow = WinSetDrawWindow(dstWinH);
			
		WinPushDrawState();
		
		/*oldFontID =*/ FntSetFont(pieceFontID);			// change font
		if (gGame.colorMode)
			SetNumberColor(piece.neighbors);
		StrIToA(text, piece.neighbors);
		textLen = StrLen( text );
		textHeight = FntLineHeight();
		textWidth = FntCharsWidth(text, textLen);
		x = rect.topLeft.x + ((rect.extent.x - textWidth) / 2);
		y = rect.topLeft.y + ((rect.extent.y - textHeight) / 2);
		WinDrawChars(text, textLen, x, y);
		
		WinPopDrawState();
		
		// Restore the original draw window
		if (dstWinH)
			WinSetDrawWindow(currDrawWindow);
		}
}


/***********************************************************************
 *
 * FUNCTION:    DrawGameBoard
 *
 * DESCRIPTION:	Draw the game board.  
 *
 *	This function is called from InitPieces() when starting a new game, 
 *	from MainFormHandleEvent in response to a frmUpdateEvent or frmOpenEvent,
 *	from DeadlyMove(), and from UpdateScrollView().
 *
 *	In response to pen strokes, HandlePenDown() will have already called
 *  DrawPiece, redrawing the appropriate pieces directly to the screen.
 *
 *  If the offscreen window is available, it is used here to optimize
 *  redrawing the entire board.  Rather than redrawing each piece in
 *  separate operations to the screen, the entire board is drawn to the
 *  offscreen window, and then the offscreen window is copied in one 
 *  operation to the screen.
 *			
 * PARAMETERS:	none
 *
 * RETURNED:	nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			vmk		10/08/95	Initial Version
 *			acs		07/01/00	version 3.6
 *
 ***********************************************************************/
static void DrawGameBoard()
{
	Int16			yInit, yIndex, yEndIndex, xInit, xIndex, xEndIndex;
	PointType		savedOrigin;
	RectangleType 	srcRect;

	UpdateMinesLeftDisplay();

	xInit = gGame.visible.origin.x;
	xEndIndex = xInit + kVisibleColumns;

	yInit = gGame.visible.origin.y;
	yEndIndex = yInit + kVisibleRows;
	
	// Unlike the onscreen board, the offscreen window always has a a (0,0) origin.
	// Keep the board and window in sync, since DrawPiece will use this origin in its
	// calls to GetPieceRectangle().  The onscreen origin will be restored before
	// copying the offscreen window to the screen. 
	savedOrigin = gGame.board.origin;
	gGame.board.origin.x = 0;
	gGame.board.origin.y = 0;
	
	// if scrolling, shift block of pieces already drawn to the offscreen window, and
	// draw to the offscreen window at most one newly exposed row or column
	if (gGame.scroll.scrolling)
		{
		// if this is the first time drawing to the offscreen window, draw all visible pieces,
		// including one row below the last full visible row for the fractional row column 
		// that scrolling has uncovered if scrolling down or right.
		if (gGame.scroll.firstDraw)
			{
			if (gGame.visible.origin.y < kMaxRows - kVisibleRows)
				yEndIndex++;
		
			if (gGame.visible.origin.x < kMaxColumns - kVisibleColumns)
				xEndIndex++;
				
			// draw all pieces
			for (yIndex = yInit; yIndex < yEndIndex; yIndex++)
				for (xIndex = xInit; xIndex < xEndIndex; xIndex++)
					DrawPiece(gOffscreenBoardWinH, yIndex, xIndex);
					
			gGame.scroll.firstDraw = false;	
			}
		else	// since actively scrolling, draw only newly exposed pieces
			{
			Boolean shiftOffscreenWindow = false;
			Boolean	drawNewCol = false;
			Boolean	drawNewRow = false;
			Int16 dstX, dstY, newRow, newCol;
		
			// use srcRect to define what portion of the offscreen window is to be shifted
			srcRect.topLeft.x = 0;
			srcRect.topLeft.y = 0;
			srcRect.extent.x = kVisibleColumns * kPieceBitmapWidth;
			srcRect.extent.y = kVisibleRows * kPieceBitmapHeight;
			
			// use dstX and dstY to define where the still valid portion of the offscreen 
			// window is to be shifted
			dstX = 0;
			dstY = 0;
			
			// if scrolling down
			if (gGame.scroll.increment.y > 0)
				{
				if (gGame.scroll.offset.y == 0)
					{
					// scrolled down to row boundary, so adjust offscreen window down one row,
					// clipping off the old top row
					srcRect.topLeft.y = kPieceBitmapHeight;
					shiftOffscreenWindow = true;
					}

				else if (gGame.scroll.offset.y == kScrollingIncrement)
					{
					// new row has is now visible, so draw it into offscreen window
					newRow = gGame.visible.origin.y + kVisibleRows;
					drawNewRow = true;
					}
				}
			// if scrolling up
			else if (gGame.scroll.increment.y < 0)
				{
				if (gGame.scroll.offset.y == kPieceBitmapHeight - kScrollingIncrement)
					{
					// new row has is now visible, so draw it into offscreen window 
					newRow = gGame.visible.origin.y;
					drawNewRow = true;
					
					// adjust offscreen window, old second row is now the top row
					dstY = kPieceBitmapHeight;
					shiftOffscreenWindow = true;
					}
				}
			
			// if scrolling right
			if (gGame.scroll.increment.x > 0)
				{
				if (gGame.scroll.offset.x == 0)
					{
					// scrolled right to column boundary, so adjust offscreen window right one column,
					// clipping off the old left column
					srcRect.topLeft.x = kPieceBitmapWidth;
					shiftOffscreenWindow = true;
					}

				else if (gGame.scroll.offset.x == kScrollingIncrement)
					{
					// new column has is now visible, so draw it into offscreen window
					newCol = gGame.visible.origin.x + kVisibleColumns;
					drawNewCol = true;
					}
				}
			// if scrolling left
			else if (gGame.scroll.increment.x < 0)
				{
				if (gGame.scroll.offset.x == kPieceBitmapWidth - kScrollingIncrement)
					{
					// new column has is now visible, so draw it into offscreen window
					newCol = gGame.visible.origin.x;
					drawNewCol = true;
					
					// adjust offscreen window, old second column is now the first column
					dstX = kPieceBitmapWidth;
					shiftOffscreenWindow = true;
					}
				}
				
			// move already drawn pieces to the appropriate part of offscreen window
			if (shiftOffscreenWindow)
				WinCopyRectangle(gOffscreenBoardWinH, gOffscreenBoardWinH, &srcRect, dstX, dstY, winPaint);
				
			// draw new row into offscreen window
			if (drawNewRow)
				{
				for (xIndex = xInit; xIndex < xEndIndex; xIndex++)
					DrawPiece(gOffscreenBoardWinH, newRow, xIndex);
				}
			
			// draw new column into offscreen window
			if (drawNewCol)
				{
				for (yIndex = yInit; yIndex < yEndIndex; yIndex++)
					DrawPiece(gOffscreenBoardWinH, yIndex, newCol);
				}

			}	// else not first draw
		} // if scrolling
	
	// draw all pieces to offscreen window
	else
		{
		for (yIndex = yInit; yIndex < yEndIndex; yIndex++)
			for (xIndex = xInit; xIndex < xEndIndex; xIndex++)
				DrawPiece(gOffscreenBoardWinH, yIndex, xIndex);
		}

	// Restore the game's real origin
	gGame.board.origin = savedOrigin;
	
	// initialize srcRect to the bounds of the visible board, including any partialy scrolled pieces
	srcRect.topLeft.x = gGame.scroll.offset.x;
	srcRect.topLeft.y = gGame.scroll.offset.y;
	srcRect.extent.x = kVisibleColumns * kPieceBitmapWidth;
	srcRect.extent.y = kVisibleRows * kPieceBitmapHeight;

	// copy from offscreen window to the screen
	WinCopyRectangle(gOffscreenBoardWinH, 0/*dstWin*/, &srcRect,
						gGame.board.origin.x, gGame.board.origin.y, winPaint);
						
	//DrawScrollingBorders();
}


//#pragma mark -
/***********************************************************************
 *
 * FUNCTION:    InitColorTheme
 *
 * DESCRIPTION:	Pieces are always drawn using the system UI colors.
 *				This function initializes the UI colors to either the
 *				fixed "classic" colors, or to the system's UI colors
 *				when in non-classic mode.  
 *
 *				This function is called from InitColors().
 *
 * PARAMETERS:	none
 *
 * RETURNED:	nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			acs		07/01/00	Initial Version
 *
 ***********************************************************************/
static void InitColorTheme()
{
	//Err err;
	
	if (gMinePref.classicLook)
		{
		/*err =*/ UIColorSetTableEntry(UIFormFrame, &gClassicColors[formFrameColor]);		
		/*err =*/ UIColorSetTableEntry(UIFormFill, &gClassicColors[formFillColor]);		
		/*err =*/ UIColorSetTableEntry(UIObjectForeground, &gClassicColors[objectFgColor]);		
		/*err =*/ UIColorSetTableEntry(UIObjectFill, &gClassicColors[objectFillColor]);			
		/*err =*/ UIColorSetTableEntry(UICaution, &gClassicColors[cautionColor]);			
		/*err =*/ UIColorSetTableEntry(UIWarning, &gClassicColors[warningColor]);	
		}
	else
		{
		/*err =*/ UIColorSetTableEntry(UIFormFrame, &gUIColors[formFrameColor]);		
		/*err =*/ UIColorSetTableEntry(UIFormFill, &gUIColors[formFillColor]);		
		/*err =*/ UIColorSetTableEntry(UIObjectForeground, &gUIColors[objectFgColor]);		
		/*err =*/ UIColorSetTableEntry(UIObjectFill, &gUIColors[objectFillColor]);			
		/*err =*/ UIColorSetTableEntry(UICaution, &gUIColors[cautionColor]);			
		/*err =*/ UIColorSetTableEntry(UIWarning, &gUIColors[warningColor]);	
		}
}


/***********************************************************************
 *
 * FUNCTION:    InitColors
 *
 * DESCRIPTION:	This function initializes the global colors that are
 *				used for drawing board and piece frames.
 *
 *				This function is called from InitPieces().
 *
 * PARAMETERS:	none
 *
 * RETURNED:	nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			acs		07/01/00	Initial Version
 *
 ***********************************************************************/
static void InitColors()
{
	if (! gGame.colorMode)
		{
		gGame.baseColor = lightGray;
		gGame.lightColor = white;
		gGame.darkColor = black;
		}
	else		// color mode
		{
		RGBColorType rgb, rgbMod;
		UInt8 delta;
		//Err err;

		if (ColorThemeChanged())		// initialize UI colors if color theme changed
			InitColorTheme();

		gGame.baseColor = UIColorGetTableEntryIndex(UIFormFill);
		WinIndexToRGB(gGame.baseColor, &rgb);
		
		// Adjust baseColor if it is too dark or too light in order to obtain appropriate contrast 
		// between base, light, and dark colors.  This adjustment is made only if all three
		// RGB components are either greater than the maximum threshold or less than the minimum
		// threshold.
		
		if ((rgb.r < kMinColorThreshold) && (rgb.g < kMinColorThreshold) && (rgb.b < kMinColorThreshold))
			{
			// base is too dark, so lighten it while maintaining same relative differences in components
			delta = kMinColorThreshold - rgb.r;
			delta = min(delta, kMinColorThreshold - rgb.g);
			delta = min(delta, kMinColorThreshold - rgb.b);
			rgb.r += delta;
			rgb.g += delta;
			rgb.b += delta;
			gGame.baseColor = WinRGBToIndex(&rgb);
			}
		
		else if ((rgb.r > kMaxColorThreshold) && (rgb.g > kMaxColorThreshold) && (rgb.b > kMaxColorThreshold))
			{
			// base is too light, so darken it while maintaining same relative differences in components
			delta = rgb.r - kMaxColorThreshold;
			delta = min(delta, rgb.g - kMaxColorThreshold);
			delta = min(delta, rgb.b - kMaxColorThreshold);
			rgb.r -= delta;
			rgb.g -= delta;
			rgb.b -= delta;
			gGame.baseColor = WinRGBToIndex(&rgb);
			}
		
		// adjust UIFormFill based on adjustment to base color
		/*err =*/ UIColorSetTableEntry(UIFormFill, &rgb);		

		// determine lighter color	
		rgbMod.r = (rgb.r >= kMaxColorThreshold) ? 0xFF : rgb.r + kContrast;
		rgbMod.g = (rgb.g >= kMaxColorThreshold) ? 0xFF : rgb.g + kContrast;
		rgbMod.b = (rgb.b >= kMaxColorThreshold) ? 0xFF : rgb.b + kContrast;
		gGame.lightColor = WinRGBToIndex(&rgbMod);
		
		// determine darker color
		rgbMod.r = (rgb.r <= kMinColorThreshold) ? 0x00 : rgb.r - kContrast;
		rgbMod.g = (rgb.g <= kMinColorThreshold) ? 0x00 : rgb.g - kContrast;
		rgbMod.b = (rgb.b <= kMinColorThreshold) ? 0x00 : rgb.b - kContrast;
		gGame.darkColor = WinRGBToIndex(&rgbMod);
		}
}


/***********************************************************************
 *
 * FUNCTION:    ColorThemeChanged
 *
 * DESCRIPTION:	This function determines whether the system's UI colors
 *				match the color preference.
 *				
 *				This function is called from InitColors().
 *
 * PARAMETERS:	none
 *
 * RETURNED:	Boolean  --	true if system UI colors and color preference
 *							does not match
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			acs		07/01/00	Initial Version
 *
 ***********************************************************************/
static Boolean ColorThemeChanged()
{
	Boolean changed;

	if (gGame.restored)		// always initialize theme when game was just launched
		changed = true;
	else
		{
		RGBColorType currentTheme[6];
		RGBColorType* desiredTheme;
		
		UIColorGetTableEntryRGB(UIFormFrame, &currentTheme[formFrameColor]);
		UIColorGetTableEntryRGB(UIFormFill, &currentTheme[formFillColor]);
		UIColorGetTableEntryRGB(UIObjectForeground, &currentTheme[objectFgColor]);
		UIColorGetTableEntryRGB(UIObjectFill, &currentTheme[objectFillColor]);
		UIColorGetTableEntryRGB(UICaution, &currentTheme[cautionColor]);
		UIColorGetTableEntryRGB(UIWarning, &currentTheme[warningColor]);
		
		desiredTheme = gMinePref.classicLook ? (RGBColorType*) &gClassicColors[0] : &gUIColors[0];
		changed = MemCmp((const void*) currentTheme, (const void*) desiredTheme, sizeof(gClassicColors));
		}
		
	return changed;
}


/***********************************************************************
 *
 * FUNCTION:    DrawPieceFrame
 *
 * DESCRIPTION:	Draw a frame around a game piece, attempting to produce
 *				a raised tile effect.
 *
 * PARAMETERS:	none
 *
 * RETURNED:	nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			acs		07/01/00	Initial Version
 *
 ***********************************************************************/
static void DrawPieceFrame()
{					
	// draw lighter frame on top and left edges
	WinSetForeColor(gGame.lightColor);
	WinDrawLine(0, 0, kPieceBitmapWidth - 1, 0);
	WinDrawLine(0, 0, 0, kPieceBitmapHeight - 1);
	WinDrawLine(1, 1, kPieceBitmapWidth - 1, 1);
	WinDrawLine(1, 1, 1, kPieceBitmapHeight - 1);
				
	// draw darker frame on bottom and right edges
	WinSetForeColor(gGame.darkColor);
	WinDrawLine(0, kPieceBitmapHeight - 1, kPieceBitmapWidth - 1, kPieceBitmapHeight - 1);
	WinDrawLine(kPieceBitmapWidth - 1, 0, kPieceBitmapWidth - 1, kPieceBitmapHeight - 1);
	WinDrawLine(1, kPieceBitmapHeight - 2, kPieceBitmapWidth - 2, kPieceBitmapHeight - 2);
	WinDrawLine(kPieceBitmapWidth - 2, 1, kPieceBitmapWidth - 2, kPieceBitmapHeight - 2);
}


/***********************************************************************
 *
 * FUNCTION:	DrawBitmap
 *
 * DESCRIPTION:	Convenience routine to draw a bitmap at specified location
 *
 * PARAMETERS:	winHandle	-- MemHandle of window to draw to (0 for current window)
 *				resID		-- bitmap resource id
 *				x, y		-- bitmap origin relative to current window
 *
 * RETURNED:	nothing.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			vmk		10/9/95		Initial Revision
 *
 ***********************************************************************/
static void DrawBitmap(WinHandle winHandle, Int16 resID, Int16 x, Int16 y)
{
	MemHandle	resH;
	BitmapPtr	resP;
	WinHandle 	currDrawWindow;
	
	// If passed a non-null window Handle, set it up as the draw window, saving
	// the current draw window
	if (winHandle)
		currDrawWindow = WinSetDrawWindow(winHandle);
		
	resH = DmGetResource(bitmapRsc, resID);
	ErrFatalDisplayIf(! resH, "no bitmap");
	resP = MemHandleLock(resH);
	WinDrawBitmap (resP, x, y);
	MemPtrUnlock(resP);
	DmReleaseResource(resH);
	
	// Restore the current draw window
	if (winHandle)
		WinSetDrawWindow(currDrawWindow);
}


/***********************************************************************
 *
 * FUNCTION:    InitPieces
 *
 * DESCRIPTION:	Based on preference settings, this function draws the 
 *				piece bitmaps into an offscreen cache.
 *
 *				This function is called by StartApplication(), 
 *				MainFormHandleEvent() when the new game button is pressed,
 *				ChangePreferences() when the color or scrolling preference
 *				changes, and from PromptToStartNewGame().
 *
 * PARAMETERS:	redraw 	--	if true, call DrawGameBoard.  This argument
 *							is false when called by StartApplication(),
 *							before the offscreen window is allocated.
 *
 * RETURNED:	nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			acs		07/01/00	Initial Version
 *
 ***********************************************************************/
static void InitPieces(Boolean redraw)
{
	RectangleType pieceRect, customClip, saveClip;
	IndexedColorType objectFillIndex;
	WinHandle oldDrawWinH;
	int counter;

	LoadGameBoard();
	
	InitColors();
	
	if (gGame.colorMode)		// draw color bitmaps using UI colors
		{
		pieceRect.topLeft.x = 0;
		pieceRect.topLeft.y = 0;
		pieceRect.extent.x = kPieceBitmapWidth;
		pieceRect.extent.y = kPieceBitmapHeight;
		
		customClip.topLeft.x = kPieceFrameWidth;
		customClip.topLeft.y = kPieceFrameWidth;
		customClip.extent.x = kPieceBitmapWidth - (kPieceFrameWidth * 2);
		customClip.extent.y = kPieceBitmapHeight - (kPieceFrameWidth * 2);
		
		objectFillIndex = UIColorGetTableEntryIndex(UIObjectFill);

		oldDrawWinH = WinGetDrawWindow();
		for (counter = 0; counter < lastSquareGraphic; counter++)
			{
			WinSetDrawWindow(gColorPieces[counter]);
							
			if (counter == coveredSquareGraphic)
				{
				RectangleType top;
				
				// This bitmap is used to construct others.
				// The graphic drawn is a raised blank area.
				top.topLeft.x = 1;
				top.topLeft.y = 1;
				top.extent.x = kPieceBitmapWidth - 2;
				top.extent.y = kPieceBitmapHeight - 2;
				
				WinSetForeColor(objectFillIndex);
				WinPaintRectangle(&top, 0);
				DrawPieceFrame();
				}
			else if (counter == markedUnknownSquareGraphic)
				{
				// start with the covered square bitmap
				WinCopyRectangle(gColorPieces[coveredSquareGraphic], gColorPieces[counter], 
									&pieceRect, 0, 0, winPaint);
				
				// reduce clipping rect so that raised border is preserved
				WinGetClip(&saveClip);
				WinSetClip(&customClip);
				
				WinSetForeColor(UIColorGetTableEntryIndex(UICaution));
				WinSetBackColor(objectFillIndex);
				DrawBitmap(0, CustomBitmapNumber(counter), 0, 0);
				
				WinSetClip(&saveClip);
				DrawPieceFrame();
				}
			else if (counter == markedMineSquareGraphic)
				{
				// start with the covered square bitmap
				WinCopyRectangle(gColorPieces[coveredSquareGraphic], gColorPieces[counter], 
									&pieceRect, 0, 0, winPaint);
				
				// reduce clipping rect so that raised border is preserved
				WinGetClip(&saveClip);
				WinSetClip(&customClip);
				
				WinSetForeColor(UIColorGetTableEntryIndex(UIWarning));
				WinSetBackColor(objectFillIndex);
				DrawBitmap(0, CustomBitmapNumber(counter), 0, 0);
				
				WinSetClip(&saveClip);
				DrawPieceFrame();
				}
			else if (counter == mineSquareGraphic || counter == notMineSquareGraphic)
				{
				// reduce clipping rect so that raised border is preserved
				WinGetClip(&saveClip);
				WinSetClip(&customClip);
				
				WinSetForeColor(UIColorGetTableEntryIndex(UIWarning));		
				DrawBitmap(0, CustomBitmapNumber(counter), 0, 0);
				
				WinSetClip(&saveClip);
				DrawPieceFrame();
				}
			else if (counter == emptySquareGraphic)
				{
				WinSetForeColor(gGame.baseColor);
				WinPaintRectangle(&pieceRect, 0);
				
				// draw the border
				WinSetForeColor(gGame.darkColor);
				WinDrawLine(0, 0, kPieceBitmapWidth - 1, 0);
				WinDrawLine(0, 0, 0, kPieceBitmapHeight - 1);
				}
			}	// end for 
			
		WinSetDrawWindow(oldDrawWinH);
		
		// set background color to base color so that when game pieces are uncovered, the
		// text for the count of neighbors are drawn with the background color of empty pieces
		WinSetBackColor(gGame.baseColor);

		}	// end if gGame.colorMode
		
	else	// draw non-color pieces directly from the bitmap family resource
		{
		for (counter = 0; counter < lastSquareGraphic; counter++)
			DrawBitmap(gGrayPieces, BitmapNumber(counter), (kPieceBitmapWidth * counter), 0);
		}
		
	if (redraw)
		{
		FrmDrawForm(FrmGetActiveForm());		// redraw menu bar with current fill color
		DrawGameBoard();
		}
}


/***********************************************************************
 *
 * FUNCTION:    NewGameBoard
 *
 * DESCRIPTION:	Shuffle the game board.
 *
 * PARAMETERS:	moves	-- number of moves to shuffle
 *
 * RETURNED:	nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			vmk		10/8/95		Initial Version
 *
 ***********************************************************************/
static void NewGameBoard(UInt32 moves)
{
	UInt16		randRow, randCol;
	UInt16		i, j;
	Int16		numMines;
	GamePieceType tempPiece;
	
	// Mapping from difficulty level to the number of mines
	Int16 difficultyToNumMinesMapping[] =	
		{
		10,		// difficultyEasy
		20,		// difficultyIntermediate
		30,		// difficultyMoreDifficult
		60		// difficultyImpossible
		};
	
	// Initialize the board
	gGame.restored = false;
	gGame.minesLeft = difficultyToNumMinesMapping[gMinePref.difficulty];
	gGame.minesLeft = gGame.minesLeft * kMaxRows * kMaxColumns / 200;
	gGame.numMines = gGame.minesLeft;
	gGame.state = gameInProgress;
	gGame.board.numRows = kMaxRows;
	gGame.board.numCol = kMaxColumns;
	gGame.board.origin.x = kBoardOriginX;
	gGame.board.origin.y = kBoardOriginY;
	gGame.visible.numRows = kVisibleRows;
	gGame.visible.numCol = kVisibleColumns;
	gGame.visible.origin.x = (kMaxColumns - kVisibleColumns) / 2;
	gGame.visible.origin.y = (kMaxRows - kVisibleRows) / 2;
	gGame.scroll.origin.x = gGame.visible.origin.x;
	gGame.scroll.origin.y = gGame.visible.origin.y;
	gGame.scroll.offset.x = 0;
	gGame.scroll.offset.y = 0;
	
	// init the scroll arrow flag to force a correct first draw
	gGame.scroll.scrolling = false;
	gGame.scroll.firstDraw = true;
	gGame.scroll.increment.x = 0;
	gGame.scroll.increment.y = 0;
		
	numMines = gGame.numMines;
	
	for (i = 0; i < gGame.board.numRows; i++)
		for (j = 0; j < gGame.board.numCol; j++)
			{
			gGame.piece[i][j].mined = (numMines > 0) ? 1 : 0;
			numMines--;
			gGame.piece[i][j].state = covered;
			gGame.piece[i][j].neighbors = 0;
			}
	
	// Shuffle the pieces
	do	
		{
		for (i = 0; i < gGame.board.numRows; i++)
			for (j = 0; j < gGame.board.numCol; j++)
				{
				randRow = (UInt16) SysRandom(0) % gGame.board.numRows;
				randCol = (UInt16) SysRandom(0) % gGame.board.numCol;
				tempPiece = gGame.piece[i][j];
				gGame.piece[i][j] = gGame.piece[randRow][randCol];
				gGame.piece[randRow][randCol] = tempPiece;
				if (! (--moves))
					goto Done;
				}
		}
	while (true);
	
Done:

	// Count the neighbors
	for (i = 0; i < gGame.board.numRows; i++)
		for (j = 0; j < gGame.board.numCol; j++)
			{
			if (! gGame.piece[i][j].mined)
				continue;
			
			// Take care of the 3 neighbors to the left
			if (j > 0)
				{
				gGame.piece[i][j-1].neighbors += 1;
				if (i > 0)
					gGame.piece[i-1][j-1].neighbors += 1;
				if (i < (gGame.board.numRows - 1))
					gGame.piece[i+1][j-1].neighbors += 1;
				}
				
			// Take care of the 3 neighbors to the right
			if (j < (gGame.board.numCol - 1))
				{
				gGame.piece[i][j+1].neighbors += 1;
				if ( i > 0 )
					gGame.piece[i-1][j+1].neighbors += 1;
				if ( i < (gGame.board.numRows - 1) )
					gGame.piece[i+1][j+1].neighbors += 1;
				}
			
			// Take care of the neighbor above
			if (i > 0)
				gGame.piece[i-1][j].neighbors += 1;
			
			// Take care of the neighbor below
			if (i < (gGame.board.numRows - 1))
				gGame.piece[i+1][j].neighbors += 1;

			}	// for ( j=0; j < gGame.numCol; j++ )


	return;
}


/***********************************************************************
 *
 * FUNCTION:    LoadGameBoard
 *
 * DESCRIPTION:	Load saved game from app preferences.
 *
 * PARAMETERS:	none
 *
 * RETURNED:	nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			vmk		10/8/95		Initial Version
 *
 ***********************************************************************/
static void LoadGameBoard()
{
	Int16  version;
	UInt16 size = sizeof(gMinePref);
	
	// Try loading a saved game board - if that fails, generate a new one
	version = PrefGetAppPreferences(kMineAppCreator, 0, &gMinePref, &size, false);

	if ((version == kMineAppVersionNum) && (gMinePref.signature == minePrefSignature))
		{
		// don't let prefs overwrite gGame if user is starting a new game
 		if (gGame.state == gameRestart)
	 		{
			NewGameBoard(numShuffleMoves);
			gMinePref.game = gGame;
			}
		else	
			{
			// Don't overwrite colorMode, which represents the current screen bit depth.
			// Restore it regardless of preferences, since the user may have changed the
			// screen bit depth since MineHunt was last run.
			Boolean colorMode = gGame.colorMode;
			
			gGame = gMinePref.game;
			gGame.colorMode = colorMode;
			gGame.restored = true;
			}
		}
	else	// generate the prefs and a new board
		{
		gMinePref.signature = minePrefSignature;
		gMinePref.difficulty = defaultDifficulty;
		gMinePref.classicLook = true;
		//gMinePref.autoScroll = true;
		gMinePref.autoScroll = false;
		NewGameBoard(numShuffleMoves);
		gMinePref.game = gGame;
		}
}


/***********************************************************************
 *
 * FUNCTION:    SaveGameBoard
 *
 * DESCRIPTION:	Save game in app preferences.
 *
 * PARAMETERS:	none
 *
 * RETURNED:	nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			vmk		10/8/95		Initial Version
 *
 ***********************************************************************/
static void SaveGameBoard()
{
	gGame.visible.origin.x = gGame.scroll.origin.x;
	gGame.visible.origin.y = gGame.scroll.origin.y;

	gMinePref.game = gGame;
	
	// Save our preferences to the Preferences database
	PrefSetAppPreferences(kMineAppCreator, 0, kMineAppVersionNum, &gMinePref, sizeof(gMinePref), false);
}


//#pragma mark -
/***********************************************************************
 *
 * FUNCTION:    StartApplication
 *
 * DESCRIPTION:	Initialize application.
 *
 *				This function initializes teh random number generator,  
 *				creates and initializes the  offscreen window for board 
 *				drawing optimization, and calls InitPieces().
 *
 * PARAMETERS:   none
 *
 * RETURNED:     errNone on success
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			vmk		10/8/95		Initial Version
 *			acs		3/18/00		support non-classic look; deallocate windows;
 *								use offscreen board window
 *
 ***********************************************************************/
static Err StartApplication()
{
	Err error;
	UInt32 depth;
	int counter;

#ifdef DEBUG_SAME_GAME
	// use constant seed if deterministic game behavior needed when debugging
	SysRandom(1234);	
#else
	// Initialize the random number seed
	SysRandom(TimGetSeconds());
#endif
	
	FrmCenterDialogs(true);

	error = WinScreenMode(winScreenModeGet, NULL, NULL, &depth, NULL);
	ErrNonFatalDisplayIf(error, "WinScreenMode error");
	
	// run in color if available
	gGame.colorMode = depth > 4;
	
	// Allocate offscreen buffer where the bitmaps for all possible appearances of
	// individual game pieces are cached. 
	 
	if (gGame.colorMode)		// cache color bitmaps in an array of windows
		{
		for (counter = 0; counter < lastSquareGraphic; counter++)
			{
			gColorPieces[counter] = WinCreateOffscreenWindow(kPieceBitmapWidth, kPieceBitmapHeight, nativeFormat, &error);
			ErrNonFatalDisplayIf(error, "Error allocating offscreen windows");
			}
			
		// save system's UI colors
		UIColorGetTableEntryRGB(UIFormFrame, &gUIColors[formFrameColor]);
		UIColorGetTableEntryRGB(UIFormFill, &gUIColors[formFillColor]);
		UIColorGetTableEntryRGB(UIObjectForeground, &gUIColors[objectFgColor]);
		UIColorGetTableEntryRGB(UIObjectFill, &gUIColors[objectFillColor]);
		UIColorGetTableEntryRGB(UICaution, &gUIColors[cautionColor]);
		UIColorGetTableEntryRGB(UIWarning, &gUIColors[warningColor]);
		}
	else				// cache non-color bitmaps in a single buffer
		{
		UInt32 preferredGreyScaleDepth = 2;

		// If system does not support color, convert to 2 bit depth.  Depth conversion must
		// be done before allocating offscreen window, so that the offscreen window will match
		// the system's depth.
		error = WinScreenMode(winScreenModeSet, NULL, NULL, &preferredGreyScaleDepth, NULL);
		ErrNonFatalDisplayIf(error, "WinScreenMode error");

		gGrayPieces = WinCreateOffscreenWindow(kPieceBitmapWidth * lastSquareGraphic, kPieceBitmapHeight, nativeFormat, &error);
		ErrNonFatalDisplayIf(error, "Error allocating offscreen bitmaps");
		}
	
	// InitPieces will either reload or generate a game board.  Do not allocate the 
	// offscreen board until after this call.
	InitPieces(kDoNotRedraw);
	
	if (error == errNone)
		{
		// Allocate an offscreen buffer for the entire playing board to speed up mass redraw. 
		// This speeds up the board initialization, restoration, and screen update.
		gOffscreenBoardWinH = WinCreateOffscreenWindow(
				(gGame.visible.numCol + 1) * kPieceBitmapWidth, 
				(gGame.visible.numRows + 1) * kPieceBitmapHeight,
				nativeFormat, &error);
		ErrFatalDisplayIf(error, "Error allocating offscreen window");
		}
		
	return error;
}


/***********************************************************************
 *
 * FUNCTION:	StopApplication
 *
 * DESCRIPTION:	Save the current state of the application, close all
 *						forms, and deletes the offscreen window.
 *
 * PARAMETERS:	none
 *
 * RETURNED:	nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			vmk		10/8/95		Initial Version
 *			acs		3/18/00		Deallocate windows
 *
 ***********************************************************************/
static void StopApplication()
{
	SaveGameBoard();
	FrmCloseAllForms();
	
	// Free windows used for drawing gray bitmaps
	if (gGrayPieces)
		{
		WinDeleteWindow(gGrayPieces, false/*eraseIt*/);
		gGrayPieces = 0;
		}

	// Free windows used for drawing color bitmaps
	if (gGame.colorMode)
		{
		int counter;
	
		for (counter = 0; counter < lastSquareGraphic; counter++)
			{
			if (gColorPieces[counter])
				{
				WinDeleteWindow( gColorPieces[counter], false/*eraseIt*/ );
				gColorPieces[counter] = 0;
				}
			}
		}

	// Free the offscreen window
	if (gOffscreenBoardWinH)
		{
		WinDeleteWindow(gOffscreenBoardWinH, false/*eraseIt*/);
		gOffscreenBoardWinH = 0;
		}
}


/***********************************************************************
 *
 * FUNCTION:	ChangePreferences
 *
 * DESCRIPTION:	Display the preferences dialog and save the new settings to
 *				be applied to the next game.
 *
 * PARAMETERS:	none
 *
 * RETURNED:	true if the user changed difficulty level
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			vmk		5/13/96		Initial Version
 *			acs		3/18/00		add classic look preference
 *
 ***********************************************************************/
static Boolean ChangePreferences()
{
	UInt16	buttonHit;
	FormPtr	prefFormP;
	UInt16	controlID;
	DifficultyType oldDifficultyLevelPref;
	Boolean oldLookPref/*, oldScrollPref*/;
	Boolean savePrefs = false;
	Boolean	restartGame = false;
	
	// Create the form
	prefFormP = FrmInitForm(PrefView);
		
	// Initialize difficulty level selection
	switch(gMinePref.difficulty)
		{
		case difficultyEasy:
			controlID = PrefViewDifficultyEasyPBN;
			break;

		case difficultyIntermediate:
			controlID = PrefViewDifficultyIntermediatePBN;
			break;

		case difficultyMoreDifficult:
			controlID = PrefViewDifficultyMoreDifficultPBN;
			break;

		case difficultyImpossible:
			controlID = PrefViewDifficultyImpossiblePBN;
			break;

		default:
			controlID = PrefViewDifficultyEasyPBN;
			break;
		}
	FrmSetControlGroupSelection(prefFormP, PrefViewDifficultyGroup, controlID);
	
	oldDifficultyLevelPref = gMinePref.difficulty;
	
	oldLookPref = gMinePref.classicLook;
	controlID = FrmGetObjectIndex(prefFormP, PrefViewClassicCheckbox);
	FrmSetControlValue(prefFormP, controlID, gMinePref.classicLook);
	
	//oldScrollPref = gMinePref.autoScroll;
	//controlID = FrmGetObjectIndex(prefFormP, PrefViewScrollingCheckbox);
	//FrmSetControlValue(prefFormP, controlID, gMinePref.autoScroll);
	
	// Put up the form
	buttonHit = FrmDoDialog(prefFormP);
	
	// Collect new setting
	if (buttonHit == PrefViewOkButton)
		{
		controlID = FrmGetObjectId(prefFormP, FrmGetControlGroupSelection(prefFormP, PrefViewDifficultyGroup));
		switch ( controlID )
			{
			case PrefViewDifficultyEasyPBN:
				gMinePref.difficulty = difficultyEasy;
				break;
				
			case PrefViewDifficultyIntermediatePBN:
				gMinePref.difficulty = difficultyIntermediate;
				break;
				
			case PrefViewDifficultyMoreDifficultPBN:
				gMinePref.difficulty = difficultyMoreDifficult;
				break;
				
			case PrefViewDifficultyImpossiblePBN:
				gMinePref.difficulty = difficultyImpossible;
				break;
			}
			
		// If changed is passed back as true, the caller will display restart game prompt.
		// Game should only be restarted if the difficulty level has changed. 
		if (gMinePref.difficulty != oldDifficultyLevelPref)
			{
			savePrefs = true;
			restartGame = true;
			}

		controlID = FrmGetObjectIndex(prefFormP, PrefViewClassicCheckbox);
		gMinePref.classicLook = FrmGetControlValue( prefFormP, controlID );
		
		if (gMinePref.classicLook != oldLookPref)
			savePrefs = true;
		
		//controlID = FrmGetObjectIndex(prefFormP, PrefViewScrollingCheckbox);
		//gMinePref.autoScroll = FrmGetControlValue(prefFormP, controlID);
		gMinePref.autoScroll = false;
		
		//if (gMinePref.autoScroll != oldScrollPref)
		//	savePrefs = true;
		}
		
	if (savePrefs)	
		{
		SaveGameBoard();

		// if classic look pref or scrolling pref changed, redraw screen.  Do not redraw if 
		// difficulty level changed, since that choice takes effect with next new game.
		if (! restartGame)
			InitPieces(kRedraw);
		}
		
	// Delete the form
	FrmDeleteForm(prefFormP);
		
	return restartGame;
}


/***********************************************************************
 *
 * FUNCTION:    RomVersionCompatible
 *
 * DESCRIPTION: This routine checks that a ROM version meets a
 *              minimum requirement.
 *
 * PARAMETERS:  requiredVersion - minimum rom version required
 *                                (see sysFtrNumROMVersion in SystemMgr.h 
 *                                for format)
 *              launchFlags     - flags that indicate if the application 
 *                                UI is initialized.
 *
 * RETURNED:    error code or zero if rom is compatible
 *                             
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	11/15/96	Initial Revision
 *
 ***********************************************************************/
static Err RomVersionCompatible(UInt32 requiredVersion, UInt16 launchFlags)
{
	UInt32 romVersion;

	// See if we're on in minimum required version of the ROM or later.
	FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);
	if (romVersion < requiredVersion)
		{
		if ((launchFlags & (sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp)) ==
			(sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp))
			{
			FrmAlert (RomIncompatibleAlert);
			}
		
		return sysErrRomIncompatible;
		}

	return errNone;
}


/***********************************************************************
 *
 * FUNCTION:	PromptToStartNewGame
 *
 * DESCRIPTION:	Prompt the user to start a new game and start it if so.
 *
 * PARAMETERS:	none
 *
 * RETURNED:	true if new game started
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			vmk		5/13/96		Initial Version
 *
 ***********************************************************************/
static Boolean PromptToStartNewGame()
{
	Boolean	newGame = false;
	
	// Display the new game prompt
	if (FrmAlert(NewGameAlert) == 0)
		{
		gGame.state = gameRestart;
		InitPieces(kRedraw);
		newGame = true;
		}
	
	return newGame;
}


//#pragma mark -
/***********************************************************************
 *
 * FUNCTION:    MainFormInit
 *
 * DESCRIPTION: This routine initializes the "Main View"
 *
 * PARAMETERS:  frm  - a pointer to the MainForm form
 *
 * RETURNED:    nothing.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			vmk	10/8/95	Initial Version
 *
 ***********************************************************************/
static void MainFormInit(FormPtr UNUSED_PARAM(frm))
{
}


/***********************************************************************
 *
 * FUNCTION:    MainFormDoCommand
 *
 * DESCRIPTION: This routine performs the menu command specified.
 *
 * PARAMETERS:  command  - menu item id
 *
 * RETURNED:    true if the command was handled
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			vmk		10/8/95		Initial Version
 *			acs		3/18/00		save preferences if they change
 *
 ***********************************************************************/
static Boolean MainFormDoCommand(UInt16 command)
{
	Boolean	handled = false;

	MenuEraseStatus(MenuGetActiveMenu());

	switch (command)
		{
		case MainOptionsHelpCmd:
			FrmHelp(GameTipsStr);
			handled = true;
			break;
			
		case MainOptionsPrefCmd:
			if (ChangePreferences())
				PromptToStartNewGame();			// ask if the user wants to start a new game
			handled = true;
			break;

		case MainOptionsAboutCmd:
			AbtShowAbout(kMineAppCreator);
			handled = true;
			break;
		}
		
	return handled;
}

static void resize(FormType *frm) {
  WinHandle wh;
  RectangleType rect;
  UInt32 swidth, sheight;

  WinScreenMode(winScreenModeGet, &swidth, &sheight, NULL, NULL);
  wh = FrmGetWindowHandle(frm);
  RctSetRectangle(&rect, 0, 0, swidth, sheight);
  WinSetBounds(wh, &rect);
}

/***********************************************************************
 *
 * FUNCTION:    MainFormHandleEvent
 *
 * DESCRIPTION: This routine is the event handler for the "Main View"
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event has MemHandle and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			vmk		10/9/95		Initial Revision
 *
 ***********************************************************************/
static Boolean MainFormHandleEvent(EventPtr event)
{
	FormPtr frm;
	Boolean handled = false;

	if (event->eType == ctlSelectEvent)
		{
		switch (event->data.ctlSelect.controlID)
			{
			case MainFormNewGameButton:
				gGame.state = gameRestart;
				InitPieces(kRedraw);
				handled = true;
				break;
			
			default:
				break;
			}
		}

	else if (event->eType == penDownEvent)
		{
		Boolean	inBounds;
				
		handled = HandlePenDown(event->screenX, event->screenY,
				((KeyCurrentState() & (keyBitPageUp | keyBitPageDown)) != 0), &inBounds);
		
		// Prompt for a new game if pen down is on game board, but HandlePenDown() did not 
		// handle it because no game is in progress.  Don't prompt if manual scroll, 
		// howewver, since user may want scroll around to see where mines are located.
		if ((! handled) && inBounds && gMinePref.autoScroll)
			{
			// See if the user wants to start a new game
			PromptToStartNewGame();
			handled = true;
			}
		
		} 
	
				
	else if (event->eType == menuEvent)
		{
		return MainFormDoCommand(event->data.menu.itemID);
		}


	else if (event->eType == frmUpdateEvent)
		{
		FrmDrawForm(FrmGetActiveForm());
		DrawGameBoard();
		handled = true;
		}
	
		
	else if (event->eType == frmOpenEvent)
		{
		frm = FrmGetActiveForm();
		resize(frm);
		MainFormInit(frm);
		FrmDrawForm(frm);
		DrawGameBoard();
		handled = true;
		}
	
	else if (event->eType == frmCloseEvent)
		{
		}

	return handled;
}


/***********************************************************************
 *
 * FUNCTION:    AppHandleEvent
 *
 * DESCRIPTION: This routine loads form resources and set the event
 *              handler for the form loaded.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event has MemHandle and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static Boolean AppHandleEvent(EventPtr eventP)
{
	UInt16 formId;
	FormPtr frmP;

	if (eventP->eType == frmLoadEvent)
		{
		// Load the form resource.
		formId = eventP->data.frmLoad.formID;
		frmP = FrmInitForm(formId);
		FrmSetActiveForm(frmP);

		// Set the event handler for the form.  The handler of the currently
		// active form is called by FrmHandleEvent each time is receives an
		// event.
		switch (formId)
			{
			case MainForm:
				FrmSetEventHandler(frmP, MainFormHandleEvent);
				break;

			default:
				ErrNonFatalDisplay("Invalid Form Load Event");
				break;
			}
		return true;
		}
	
	return false;
}


/***********************************************************************
 *
 * FUNCTION:    TimeToWait
 *
 * DESCRIPTION:	This function is called by AppEventLoop(), and is used
 *				to determine the event loop timeout.
 *
 * PARAMETERS:	none
 *
 * RETURNED:	number of ticks for timeout
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			acs		07/01/00	Initial Version
 *
 ***********************************************************************/
static UInt32 TimeToWait()
{
	Int32 ticks;
	
	if (! gGame.scroll.scrolling)
		return evtWaitForever;
	else
		{
		ticks = (Int32) (gGame.scroll.timeToScroll - TimGetTicks());
		return (ticks < 0) ? 0 : ticks;
		}
}


/***********************************************************************
 *
 * FUNCTION:    AppEventLoop
 *
 * DESCRIPTION: This routine is the event loop for the application.  
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 *		vmk	1/23/98		Restored the "else" to make sure that when up/down
 *							is held down, the event doesn't get passed to the system
 *		CSS	06/22/99	Standardized keyDownEvent handling
 *							(TxtCharIsHardKey, commandKeyMask, etc.)
 *
 ***********************************************************************/
static void AppEventLoop()
{
	UInt16 error;
	EventType event;

	do 
		{
		EvtGetEvent(&event, TimeToWait());
		
		// Swallow up/down arrow key chars to prevent the clicking sound
		// when the keys are being used as modifiers for marking cells
		if ((event.eType == keyDownEvent)
			&& (! TxtCharIsHardKey(event.data.keyDown.modifiers, event.data.keyDown.chr))
			&& (EvtKeydownIsVirtual(&event))
			&& ((event.data.keyDown.chr == vchrPageUp) || (event.data.keyDown.chr == vchrPageDown)))
			{
			;		// Do nothing
			}
		
		else if (! SysHandleEvent(&event))
			if (! MenuHandleEvent(0, &event, &error))
				if (! AppHandleEvent(&event))
					FrmDispatchEvent(&event);
					
		if (gGame.scroll.scrolling && gMinePref.autoScroll && gGame.scroll.timeToScroll <= TimGetTicks())
			{
			UpdateScrollView();
	
			if ((gGame.visible.origin.x == gGame.scroll.origin.x) &&
				(gGame.visible.origin.y == gGame.scroll.origin.y) && 
				(gGame.scroll.increment.x == 0) && (gGame.scroll.increment.y == 0))
				{
				gGame.scroll.scrolling = false;		
				}
			else
				gGame.scroll.timeToScroll = TimGetTicks() + gGame.scroll.rate;
			}

#if EMULATION_LEVEL != EMULATION_NONE
		// Check the heaps after each event
		MemHeapCheck(0);
		MemHeapCheck(1);
#endif
		} 
	while (event.eType != appStopEvent);
}


/***********************************************************************
 *
 * FUNCTION:    PilotMain
 *
 * DESCRIPTION: This is the main entry point for the application.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    err
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			vmk		10/8/95		Initial Version
 *
 ***********************************************************************/
UInt32 PilotMain(UInt16 cmd, void * UNUSED_PARAM(cmdPBP), UInt16 launchFlags)
{
	Err error;
	
	error = RomVersionCompatible(kMinVersion, launchFlags);
	if (error) 
		return (UInt32) error;

	if (cmd == sysAppLaunchCmdNormalLaunch)
		{
		error = StartApplication();
		if (error) 
			return (UInt32) error;

		FrmGotoForm(MainForm);

		AppEventLoop();
		StopApplication();
		}

	return (UInt32) errNone;
}
