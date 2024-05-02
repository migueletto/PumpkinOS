/*  Vexed - gfx.c "The graphical functions for Vexed"
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
#include "SysUtils.h"

#include "vexed.h"
#include "protos.h"
#include "strings.h"

extern PlayGround    current;
extern PlayGroundIntro  currentI;
extern Preferences   VexedPreferences;
extern BitmapPtr     ObjectBitmapPtr[BitmapCount];
extern WinHandle     ObjectWindowHandles[BitmapCount];
extern Boolean       InBlockCheck;           // are we in a block check
extern Boolean       BlockCheckPending;      // waiting on a block check
extern UInt8         BlockCheckX,            // x,y of pending block check
                     BlockCheckY;
extern Boolean       InSolution;             // Are we displaying a solution
extern Boolean       InSolutionBack;         // are we doing a back move in solution?
extern Boolean       MoveNum;                // Which move of solution are we now displaying
extern Boolean       SolutionRun;            // Is solution in free-running mode
extern Boolean       DoingReplay;
extern Int16         levelReplay;
extern Boolean       in1Bit;                 // Are we in 1 bit mode (could be 4 bit capable down in 1 bit mode)

extern char gszEllipsis[4];

//-----------------------------------------------------------------------------

//{{{ DrawFXBusted							.
void DrawFXBusted() {
	Int16 x1, y1, x2, y2;

	x1 = 0;
	y1 = TopRow;
	x2 = (BlockSize*sizeX)-1;
	y2 = TopRow+(BlockSize*sizeY)-1;

	while( x1 <= (BlockSize*sizeX) )
		{
		WinEraseLine( x1, TopRow, x1, TopRow+(BlockSize*sizeY) );
		WinEraseLine( 0, y1, 160, y1 );

		WinEraseLine( x2, TopRow, x2, TopRow+(BlockSize*sizeY) );
		WinEraseLine( 0, y2, 160, y2 );

		SysTaskDelay(1);

		x1+=2;
		y1 = ( y1+2 > TopRow+(BlockSize*sizeY) ) ? TopRow+(BlockSize*sizeY) : y1+2;
		x2 = ( x2-2 < 0 ) ? 0 : x2-2;
		y2 = ( y2-2 < TopRow ) ? TopRow : y2-2;
		}
}
//}}}
//{{{ DrawFXFountainWave					.
void DrawFXFountainWave() {
   Int16   i, j;

	i = j = (BlockSize*sizeX)/2;
	for( ; i>=0 ; )
		{
		WinEraseLine( i, TopRow, i, TopRow+BlockSize*sizeY );
		WinEraseLine( j, TopRow, j, TopRow+BlockSize*sizeY );

		SysTaskDelay(1);

		i-=2;
		j+=2;
		}

	i = 1;
	for( ; i <= (BlockSize*sizeX)/2 ; )
		{
		WinEraseLine( i, TopRow, i, TopRow+BlockSize*sizeY );
		WinEraseLine( TopRow+BlockSize*sizeX-i, TopRow, TopRow+BlockSize*sizeX-i, TopRow+BlockSize*sizeY );
		SysTaskDelay(1);
		i+=2;
		}
}
//}}}
//{{{ DrawFXRainstorm						.
void DrawFXRainstorm() {
	Int16				x, y, step;
	Int16 			pri[16][16];
	RectangleType	rect;

	// init priority array
	step=10;
	for( x=0; x<sizeX; x++ )
		for( y=0; y<sizeY; y++ ) {
		//		pri[x][y] = 8 + (step++ % BlockSize);
			pri[x][y] = 8 + (SysRandom(0) % BlockSize);
		}

	for( step=BlockSize+8; step>=0; step-- ) {
		for( x=0; x<sizeX; x++ ) {
			for( y=0; y<sizeY; y++ ) {
				if( (pri[x][y] <= 8) && (pri[x][y] >= 0) ) {
					rect.topLeft.x = x*BlockSize + (pri[x][y]/2);
					rect.topLeft.y = TopRow + y*BlockSize + (pri[x][y]/2);
					rect.extent.x = rect.extent.y = BlockSize-pri[x][y];

					WinEraseRectangle( &rect, 0 );
				}

			pri[x][y]--;
			}
		}

	SysTaskDelay(1);
	}

}
//}}}
//{{{ DrawFXStripes							.
void DrawFXStripes() {
   Int16   i;

	for( i = 0; i < BlockSize*sizeY; i+=2 )
		{
		WinEraseLine( 0, TopRow+i, 160, TopRow+i );
		WinEraseLine( 0, TopRow+BlockSize*sizeY - (i+1), 160, TopRow+BlockSize*sizeY - (i+1) );
		SysTaskDelay(1);
		}
}
//}}}
//{{{ DrawFXJalousie							.
void DrawFXJalousie() {
	Int16				x;
	Int8				pri[ sizeX ];
	RectangleType	rect;

	rect.topLeft.y = TopRow;
	rect.extent.y  = BlockSize*sizeY;

	// init array
	for( x=0; x<sizeX; x++ )
		pri[ x ] = x*-1;

	// let's rock
	while( pri[ sizeX-1 ] <= BlockSize ) {
		for( x=0; x<sizeX; x++ ) {
			if( pri[x] > 0 ) {
				rect.topLeft.x = x*BlockSize;
				rect.extent.x = pri[x];
				WinEraseRectangle( &rect, 0 );
			}

		pri[x]++;

		if( x%2 )
			SysTaskDelay( 1 );
		}
	}
}
//}}}
//{{{ DrawFXStarWars							.
void DrawFXStarWars() {
	Int16				x1, y1, x2, y2;
	RectangleType	r;

	x1 = (BlockSize*sizeX) / 2;
	y1 = (BlockSize*sizeY) / 2;
	x2 = y2 = 2;

	r.topLeft.x = x1;
	r.topLeft.y = TopRow+y1;
  	r.extent.x = x2;
	r.extent.y = y2;

	while( (x1>=0) || (y1>=0) ) {
		WinEraseRectangleFrame( simpleFrame, &r );
		SysTaskDelay(1);

		x1-=2; y1-=2;
		x2+=4; y2+=4;

		r.topLeft.x = (x1 < 0) ? 0 : x1;
		r.topLeft.y = (y1 < 0) ? TopRow : TopRow+y1;
  		r.extent.x  = (x2 > (BlockSize*sizeX)) ? (BlockSize*sizeX) : x2;
		r.extent.y  = (y2 >= (BlockSize*sizeY-3)) ? (BlockSize*sizeY-3) : y2;
		}


	// 2nd wave...
	x1 = ((BlockSize*sizeX) / 2)-1;
	y1 = ((BlockSize*sizeY) / 2)-1;
	x2=y2=4;

	r.topLeft.x = x1;
	r.topLeft.y = TopRow+y1;
  	r.extent.x = x2;
	r.extent.y = y2;

	while( (x1>=0) || (y1>=0) ) {
		WinEraseRectangleFrame( simpleFrame, &r );
		SysTaskDelay(1);

		x1-=2; y1-=2;
		x2+=4; y2+=4;

		r.topLeft.x = (x1 < 0) ? 0 : x1;
		r.topLeft.y = (y1 < 0) ? TopRow : TopRow+y1;
  		r.extent.x  = (x2 > (BlockSize*sizeX)) ? (BlockSize*sizeX) : x2;
		r.extent.y  = (y2 >= (BlockSize*sizeY-2)) ? (BlockSize*sizeY-2) : y2;
		}

}
//}}}
//{{{ DrawFXFountain							.
void DrawFXFountain() {
   Int16   y1, y2;

	y1 = y2 = ((BlockSize*sizeY)/2)-1;
	for( ; y1>=0 ; )
		{
		WinEraseLine( 0, TopRow+y1, 160, TopRow+y1 );
		WinEraseLine( 0, TopRow+y2, 160, TopRow+y2 );

		SysTaskDelay(1);

		y1-=2;
		y2 = ( (y2+2) >= BlockSize*sizeY) ? BlockSize*sizeY-1 : y2+2;
		}

	y1 = y2 = (BlockSize*sizeY)/2;
	for( ; y1>=0 ; )
		{
		WinEraseLine( 0, TopRow + y1, 160, TopRow + y1 );
		WinEraseLine( 0, TopRow + y2, 160, TopRow + y2 );

		SysTaskDelay(1);

		y1-=2;
		y2 = ( (y2+2) >= BlockSize*sizeY) ? BlockSize*sizeY-1 : y2+2;
		}
}
//}}}
//{{{ DrawFXBlinds							.
void DrawFXBlinds() {
   Int16             i, x;
   RectangleType     r;

	#ifdef HIRESDOUBLE
	r.extent.y = 256;
	#else
	r.extent.y = 128;
	#endif
	r.topLeft.y = TopRow;

	for (i = 0; i <= BlockSize; i ++) {
		r.extent.x = i;
		for (x = 0; x <= BottomRow; x += BlockSize) {
			r.topLeft.x = x;
			WinEraseRectangle(&r, 0);
		}
		SysTaskDelay(1);
	}
}
//}}}

//{{{ DrawFX									.
static UInt8 LastFXTransitionUsed = 0;
void DrawFX() {
	
//	{
//	char str[30];
//	StrPrintF(str,"%d", LastFXTransitionUsed);
//	FrmCustomAlert( AlertDebug, str, "", "" );
//	}

   if (VexedPreferences.BlindsEffect && !InSolutionBack) {
	   switch( LastFXTransitionUsed ) {
			case 7:  DrawFXBusted(); break;
			case 6:  DrawFXFountainWave(); break;
			case 5:  DrawFXRainstorm(); break;
			case 4: 	DrawFXStripes(); break;
			case 3:  DrawFXJalousie(); break;
			case 2:  DrawFXStarWars(); break;
			case 1:	DrawFXFountain(); break;
			default:	DrawFXBlinds(); break;
		}

	LastFXTransitionUsed = (LastFXTransitionUsed+1) % 8;
	}

}
//}}}

//-----------------------------------------------------------------------------
void RedrawLevel() {
   Int8              x, y;
   RectangleType     r;

   // erase the whole screen first (Air)
   r.topLeft.x = 0;
   r.topLeft.y = BlockSize;
   r.extent.x = sizeX * BlockSize;
   r.extent.y = sizeY * BlockSize;
   WinEraseRectangle(&r, 0);

   if (VexedPreferences.LevelPack[0].LevelValid) {
      // set up bitmap rectangle copies
      // r.topLeft.x = 0;                    // already 0
      r.topLeft.y = 0;
      r.extent.x = r.extent.y = BlockSize;
      for (y = 0; y < sizeY; y++) {
         for (x = 0; x < sizeX; x++) {
            if (current[y][x] != Air) {
               //WinDrawBitmap(Bitmap[current[y][x]], x * BlockSize, y * BlockSize + 16);
               //WinCopyRectangle (ObjectWindowHandles[current[y][x] - 1], 0, &r,
               //                  x * BlockSize, y * BlockSize + 16, winPaint);
               DrawBM(current[y][x] - 1, x * BlockSize, y * BlockSize + BlockSize, false);

            }
         }
      }
   }

   UpdateStats();                            // display the on screen stats
   EvtFlushPenQueue();
   return;
}

//-----------------------------------------------------------------------------
void DrawLevel( Boolean drawFX ) {
   // do the cool F/X effect
	if( drawFX ) {
		DrawFX();
	}

   // Redraw Level
   RedrawLevel();
   return;
}

#ifdef HIRES
#define kcxStat 160-12			// 16-reserved for thumbnail icon - we want it always visible
#else
#define kcxStat 160
#endif
#define kcxBorder 3

//-----------------------------------------------------------------------------
void UpdateStats() {
	char szTitle[64];
	char szMoves[32];
	char szScore[32];
	char szScoreInteger[8];
	char sz[128];
	int cx, cy;
	UInt32 dwVer;
	UInt32 rgbBack, rgbFore;
	UInt32 rgbBackSav, rgbForeSav;
	FontID fnt, fntSav;
	RectangleType rc;
	int nScore;
	int cxScore;
	int cxScoreInteger;
   int nLevelCurrent;

   if (InSolution)
		return;

   nLevelCurrent = VexedPreferences.LevelPack[0].CurrentLevel;
   if (DoingReplay)
      nLevelCurrent = levelReplay;

	// Get the title and move count strings

   if (VexedPreferences.LevelPack[0].LevelValid) {
      if (GetLevelProperty(nLevelCurrent, "title", sz, sizeof(sz))) {
		   StrPrintF(szTitle, "%d: %s", nLevelCurrent+1, sz);
      } else {
		   StrPrintF(szTitle, "%s %d:", MSG_GFX_LEVEL, nLevelCurrent+1);
      }
	   if (VexedPreferences.LevelPack[0].MoveCountPar == -1) {
		   szMoves[0] = 0;
	   } else {
		   StrPrintF(szMoves, " %d/%d", VexedPreferences.LevelPack[0].MoveCountMade, VexedPreferences.LevelPack[0].MoveCountPar);
	   }
   } else {
      // Level not valid

      StrCopy(szTitle, MSG_GFX_LEVEL_NOT_FOUND);
      szMoves[0] = 0;
   }
	StrCopy(sz, szTitle);
	StrCat(sz, szMoves);

	// Get the score text

	nScore = CalcScore(0);
	StrIToA(szScoreInteger, nScore);
	StrCopy(szScore, MSG_GFX_SCORE " ");
	StrCat(szScore, szScoreInteger);

	// First figure out which font we'll use. Try to use bold font - looks better

	fnt = boldFont;
	fntSav = FntSetFont(fnt);
	cx = FntCharsWidth(sz, StrLen(sz));
	cxScore = FntCharsWidth(szScore, StrLen(szScore));
	cxScoreInteger = FntCharsWidth(szScoreInteger, StrLen(szScoreInteger));
	FntSetFont(fntSav);

	// See if it fits with full text
	if (kcxStat - kcxBorder - cx - kcxBorder - cxScore - kcxBorder < 0) {

		// See if it fits without "Score" text
		if (kcxStat - kcxBorder - cx - kcxBorder - cxScoreInteger - kcxBorder < 0) {

			fnt = stdFont;
			fntSav = FntSetFont(fnt);
			cx = FntCharsWidth(sz, StrLen(sz));
			cxScore = FntCharsWidth(szScore, StrLen(szScore));
			cxScoreInteger = FntCharsWidth(szScoreInteger, StrLen(szScoreInteger));
			FntSetFont(fntSav);

			// See if it fits with full text
			if (kcxStat - kcxBorder - cx - kcxBorder - cxScore - kcxBorder < 0) {
				int cxMoves, cxEllipsis, cxAllowed;
				Int16 cch, cchT;
				Boolean fIgnore;

				// Doesn't fit with full text, lop off "Score"
				StrCopy(szScore, szScoreInteger);
				cxScore = cxScoreInteger;

				// Draw as much of the text that'll fit. Put ... in the title,
				// always show the # moves

				cxMoves = FntCharsWidth(szMoves, StrLen(szMoves));
				cxEllipsis = FntCharsWidth(gszEllipsis, StrLen(gszEllipsis));
				cxAllowed = kcxStat - kcxBorder - cxMoves - cxEllipsis - kcxBorder - cxScore - kcxBorder;
				fIgnore = true;
				cch = StrLen(szTitle);
				cchT = cch;
				FntCharsInWidth(szTitle, (Int16 *)&cxAllowed, (Int16 *)&cch, &fIgnore);

				// cch has been updated with the # of chars that will fit. Terminate
				// here and add ellipsis if there is enough room

				if (cch != cchT) {
					szTitle[cch] = 0;
					if (sizeof(szTitle) - cch >= StrLen(gszEllipsis))
						StrCopy(&szTitle[cch], gszEllipsis);
				}

				StrCopy(sz, szTitle);
				StrCat(sz, szMoves);
				fntSav = FntSetFont(fnt);
				cx = FntCharsWidth(sz, StrLen(sz));
				FntSetFont(fntSav);
			}
		} else {
			// It does fit without "Score" text

			StrCopy(szScore, szScoreInteger);
			cxScore = cxScoreInteger;
		}
	}

	// Fill background appropriately
	// If >= OS 3.5, use UI colors, otherwise black and white

	FtrGet(sysFtrCreator, sysFtrNumROMVersion, &dwVer);
	if (dwVer < 0x03503000) {
		rgbFore = 0x00000000;
		rgbBack = 0x00ffffff;
	} else {
		UIColorGetTableEntryRGB(UIFormFrame, (RGBColorType *)&rgbFore);
		((RGBColorType *)&rgbFore)->index = WinRGBToIndex((RGBColorType *)&rgbFore);
		UIColorGetTableEntryRGB(UIFormFill, (RGBColorType *)&rgbBack);
		((RGBColorType *)&rgbBack)->index = WinRGBToIndex((RGBColorType *)&rgbBack);
	}

	// Erase and draw

   if (dwVer >= 0x03003000)
	   WinSetColors((RGBColorType *)&rgbFore, (RGBColorType *)&rgbForeSav, (RGBColorType *)&rgbBack, (RGBColorType *)&rgbBackSav);
	fntSav = FntSetFont(fnt);
	cy = FntCharHeight() + 2;
	rc.topLeft.x = 0;
	rc.topLeft.y = 0;
	rc.extent.y = cy;
	rc.extent.x = 160;
   WinDrawRectangle(&rc, 0);
	rc.extent.y = 1;
	rc.extent.x = 2;
   WinEraseRectangle(&rc, 0);
	rc.extent.y = 2;
	rc.extent.x = 1;
   WinEraseRectangle(&rc, 0);
	rc.topLeft.x = 158;
	rc.extent.y = 1;
	rc.extent.x = 2;
   WinEraseRectangle(&rc, 0);
	rc.topLeft.x = 159;
	rc.extent.y = 2;
	rc.extent.x = 1;
   WinEraseRectangle(&rc, 0);

	if (dwVer < 0x03503000) {
		WinDrawInvertedChars(sz, StrLen(sz), kcxBorder, 1);
		WinDrawInvertedChars(szScore, StrLen(szScore), kcxStat - kcxBorder - cxScore, 1);
	} else {
	   WinSetColors((RGBColorType *)&rgbBack, NULL, (RGBColorType *)&rgbFore, NULL);
      WinDrawChars(sz, StrLen(sz), kcxBorder, 1);
		WinDrawChars(szScore, StrLen(szScore), kcxStat - kcxBorder - cxScore, 1);
	}


#ifdef HIRES
	{
	// drawing thumb up/down, depending on the total user score...
   MemHandle h;
   BitmapType *pbm;
	WinDrawOperation oldDrawMode;

	h = DmGetResource( 'Tbmp', ( nScore > 0 ) ? BitmapThumbDown : BitmapThumbUp );
	pbm = (BitmapType *)MemHandleLock(h);
	oldDrawMode = WinSetDrawMode(winPaint);
	WinPaintBitmap(pbm, kcxStat+2, 2);
	WinSetDrawMode(oldDrawMode);
	MemHandleUnlock(h);
	DmReleaseResource(h);
	}
#endif

	// Restore
	FntSetFont(fntSav);
   if (dwVer >= 0x03003000)
	   WinSetColors((RGBColorType *)&rgbForeSav, NULL, (RGBColorType *)&rgbBackSav, NULL);
}

//-----------------------------------------------------------------------------
void WriteAboutInfo() {
   char              text [50];
   UInt8             idx;
	UInt16				x, y, step;
   RectangleType     r;


   // copy 5 letters of Vexed to top of screen
   idx = BitmapIntroOffset + 8;

   // copy all of bitmap
   r.topLeft.x = r.topLeft.y = 0;
   r.extent.x = BlockSizeX;
   r.extent.y = BlockSizeY;

   for (x = 17; x < 142; x += 25) {
      // Copy the source window (contains the image to draw) to the draw window.
      WinCopyRectangle (ObjectWindowHandles[idx++], 0, &r, x, 14, winPaint);
   }


	step = 13;
	y = 58;

   StrPrintF(text, "Version %d.%d.%d (%s)", VEXED_VERSION, VEXED_REVISION, VEXED_RELEASE, __DATE__ );
	WinDrawChars(text, StrLen(text), (156 - FntCharsWidth(text, StrLen(text)))/2, y);
	y += step;

   StrPrintF(text, "The %s release", VEXED_VERSION_NAME );
	WinDrawChars(text, StrLen(text), (156 - FntCharsWidth(text, StrLen(text)))/2, y);
	y += step + 7;

   StrPrintF(text, "Developed by the");
	WinDrawChars(text, StrLen(text), (156 - FntCharsWidth(text, StrLen(text)))/2, y);
	y += step;

   FntSetFont(1);
   StrPrintF(text, "Vexed SourceForge Project");
	WinDrawChars(text, StrLen(text), (156 - FntCharsWidth(text, StrLen(text)))/2, y);
   FntSetFont(0);
	y += step;

   StrPrintF(text, "http://vexed.sf.net");
	WinDrawChars(text, StrLen(text), (156 - FntCharsWidth(text, StrLen(text)))/2, y);
	y += step;
}

//-----------------------------------------------------------------------------
void DrawIntro() {

/*
 * NOTE: One time callable! Shall be fixed to allow multiple viewing per
 *       application launch (for user entertainment)
 */

   Int8              x, y;
   RectangleType     r;

   //intro screen moves
   MoveType moves[10] = {
      {5,2, Left,  1},
      {4,2, Left,  1},
      {0,0, Right, 1},
      {5,2, Left,  2},
      {3,3, Left,  1},
      {2,0, Left,  1},
      {4,0, Right, 1},
      {5,2, Left,  2},
      {3,3, Left,  1},
      {2,1, Right, 1}
   };

   // erase the whole screen first (Air)
   r.topLeft.x = 0;
   r.topLeft.y = 0;
   r.extent.x = ScreenSize;
   r.extent.y = ScreenSize;
   WinEraseRectangle(&r, 0);

	// drawing background
   for (y = 0; y < sizeYI; y++) {
      for (x = 0; x < sizeXI; x++) {
         if (currentI[y][x] != AirI) {
            DrawBMIntro(currentI[y][x], (x * BlockSizeX) + 5, y * BlockSizeY);
         }
      }
   }

	// Let's roll
   for (x = 0; x < (sizeof( moves ) / sizeof( MoveType )); x++) {
      PlayerMakeMoveI(moves[x].x, moves[x].y, moves[x].direction, moves[x].distance);
      if (EvtSysEventAvail(false)) {
         return;
      }
   }
   // erase the walls
   EraseWallsI();

   // move to top center
   r.topLeft.x = 5;
   r.topLeft.y = 120;
   r.extent.x = 125;
   r.extent.y = 40;
   WinEraseRectangle(&r, 0);

   WriteAboutInfo();
   return;
}

//-----------------------------------------------------------------------------
void EndSequence() {
   char              text [32];
   Int16             x1, x2, randnum1, randnum2;
   Int16             penx, peny;
   Boolean           bstate;
   RectangleType     r;
   EventType         event;
   //Err               errorID;

   // flush key/pen queues
   EvtFlushKeyQueue();
   EvtFlushPenQueue();

   // do the new congrats intro
   r.topLeft.x = 0;
   r.topLeft.y = 0;                          // was 16
   r.extent.x = 160;
   r.extent.y = 160;                         // was 128
   WinEraseRectangle(&r, 0);

   if (!InitCongrats())
      return;
   InitAnim();
   while (!Step())
      SysTaskDelay(1);

   while (bstate) EvtGetPen(&penx, &peny, &bstate);

   // draw some text of adulation to the rather smart user!
   FntSetFont(0);
   StrPrintF(text, MSG_GFX_CONGRAT_SCORE_FINAL);
   WinDrawChars(text, StrLen(text), 58, 40);

   if (VexedPreferences.LevelPack[0].MoveCountMade == 0) {
      StrPrintF(text, MSG_GFX_CONGRAT_PAR );
		WinDrawChars(text, StrLen(text), (156 - FntCharsWidth(text, StrLen(text)))/2, 68);
   }
   else {
      StrPrintF(text, "%d", Abs(VexedPreferences.LevelPack[0].MoveCountMade));
		WinDrawChars(text, StrLen(text), (156 - FntCharsWidth(text, StrLen(text)))/2, 55);
      if (VexedPreferences.LevelPack[0].MoveCountMade > 0) {
         StrPrintF(text, MSG_GFX_CONGRAT_OVER_PAR );
			WinDrawChars(text, StrLen(text), (156 - FntCharsWidth(text, StrLen(text)))/2, 68);
      }
      else {
         StrPrintF(text, MSG_GFX_CONGRAT_UNDER_PAR);
			WinDrawChars(text, StrLen(text), (156 - FntCharsWidth(text, StrLen(text)))/2, 68);
      }
   }
   StrPrintF(text, MSG_GFX_CONGRAT_PACK_COMPLETED);
	WinDrawChars(text, StrLen(text), (156 - FntCharsWidth(text, StrLen(text)))/2, 104);

   r.extent.x = BlockSize;
   r.extent.y = BlockSize;
   // fire some random blocks along the top and bottom of the screen!
   while (1) {

      randnum1 = (Int16) ((SysRandom(0) / 4096) + 1);
      randnum2 = (Int16) ((SysRandom(0) / 4096) + 1);
      SoundEffect(HighBeep);
      for (x1 = 0; x1 <= 160; x1 ++) {
         r.topLeft.x = x1 - 1;
         r.topLeft.y = 16;
         WinEraseRectangle(&r, 0);
         DrawBM(randnum1 - 1, x1, 16, false);
         //WinDrawBitmap(Bitmap[randnum1], x1, 16);

         x2 = (160 - x1);

         r.topLeft.x = x2 + 1;
         r.topLeft.y = 128;
         WinEraseRectangle(&r, 0);
         DrawBM(randnum2 - 1, x2, 128, false);
         //WinDrawBitmap(Bitmap[randnum2], x2, 128);

         // allow them to power off in the congrats form
         // check for an event
         EvtGetEvent(&event, 0);
         if (event.eType != nilEvent) {
            // dispatch on the event
            if (! SysHandleEvent(&event))    // allow power off
               ; //FrmDispatchEvent(&event); // throw the event away
         }
         SysTaskDelay(2);
         EvtGetPen(&penx, &peny, &bstate);
         if (bstate)
            break;
      }
      SoundEffect(LowBeep);
      r.topLeft.x = 0;
      r.topLeft.y = 128;
      WinEraseRectangle(&r, 0);
      if (bstate)
         break;
   }

   // do the congrats exit

   while (!Step())
      SysTaskDelay(3);
   ExitCongrats();

   // flush key/pen queues
   EvtFlushKeyQueue();
   EvtFlushPenQueue();

   SoundEffect(HighSweepLow);
   r.topLeft.x = 0;
   r.topLeft.y = 0;                          // was 16
   r.extent.x = 160;
   r.extent.y = 160;                         // was 128
   WinEraseRectangle(&r, 0);
   DrawControlIcons();
   return;
}

//-----------------------------------------------------------------------------
void BlockCheck(UInt8 x, UInt8 y) {
   Int8              thisBlock;              // what kind of block is the pen on?
   RectangleType     r;

   if (InBlockCheck) {                       // already running?
      FrmCustomAlert(AlertDebug, "Debug!", "Should never get here!", "");                 // should never get here  $$TEMP$$
      return;
   }

   // got to here, we are starting a valid block check

   thisBlock = current[y][x];

   // erase blocks not like this one
   for (y = 0; y < sizeY; y ++) {
      for (x = 0; x < sizeX; x ++) {
         if ((current[y][x] != Air) && (current[y][x] != Wall) && (current[y][x] != thisBlock)) {
            r.topLeft.x = (x * BlockSize);
            r.topLeft.y = ((y + 1) * BlockSize);
            r.extent.x = BlockSize;
            r.extent.y = BlockSize;
            WinEraseRectangle(&r, 0);        // erase the on screen block
         }
      }
   }

   // Signify block check mode is on
   InBlockCheck = true;

   return;
}

//-----------------------------------------------------------------------------
void DrawBM2(Int16 bitmapID, UInt16 x, UInt16 y, int ddx, int ddy, Boolean isId) {
   RectangleType     r;
   Int8              id;

   if (isId) {
      // translate resource ID into index into the windowhandle array
      id = (Int8) (bitmapID - firstBitmap + 8);
   } else {
      id = (Int8) bitmapID;
   }

   if (isId || bitmapID == 8) {
      // copy entire window, one square BlockSize
      r.topLeft.x = r.topLeft.y = 0;
      r.extent.x = r.extent.y = BlockSize;
   } else {
      // Blocks have a 1 pixel border that is used for erasing when a block
      // is animating.
      r.topLeft.x = r.topLeft.y = 1;
      r.extent.x = r.extent.y = BlockSize;
      if (ddx == 1) {
         r.topLeft.x -= 1;
         r.extent.x += 1;
         x--;
      }
      if (ddx == -1)
         r.extent.x += 1;
      if (ddy == 1) {
         r.topLeft.y -= 1;
         r.extent.y += 1;
         y--;
      }
      if (ddy == -1)
         r.extent.y += 1;
   }

   ErrFatalDisplayIf (ObjectWindowHandles[id] == 0, MSG_ERR_UNHANDLED_IMAGE );

   // Copy the source window (contains the image to draw) to the draw window.
   WinCopyRectangle (ObjectWindowHandles[id], 0, &r, x, y, winPaint);
}

//-----------------------------------------------------------------------------
void DrawBM(Int16 bitmapID, UInt16 x, UInt16 y, Boolean isId) {
   DrawBM2(bitmapID, x, y, 0, 0, isId);
}

//-----------------------------------------------------------------------------
void DrawControlIcons() {
   if (!InSolution) {
      // draw graphic controls at bottom of screen
      DrawBM(BitmapRestart, LocRestart, BottomRow, true);
      DrawBM(BitmapBack, LocBack, BottomRow, true);
      DrawBM(BitmapForward, LocForward, BottomRow, true);
      DrawBM(BitmapSelectLevel, LocSelectLevel, BottomRow, true);
      DrawBM(BitmapFirst, LocFirst, BottomRow, true);
      DrawBM(BitmapLast, LocLast, BottomRow, true);
      DrawBM(BitmapMemory, LocMemory, BottomRow, true);
   }
   return;
}

//-----------------------------------------------------------------------------
void UpdateBlocks(void) {
   Int8              y,x;                    // what kind of block is the pen on?
   RectangleType     r;

   r.extent.x = BlockSize;
   r.extent.y = BlockSize;

   // changed block sets, so redraw all non-wall blocks
   for (y = 0; y < sizeY; y ++) {
      for (x = 0; x < sizeX; x ++) {
         if ((current[y][x] != Air) && (current[y][x] != Wall)) {
            DrawBM(current[y][x] - 1, x * BlockSize, y * BlockSize + BlockSize, false);
         }
         if (current[y][x] == Air) {
            r.topLeft.x = x * BlockSize;
            r.topLeft.y = y * BlockSize + BlockSize;
            WinEraseRectangle(&r, 0);
         }
      }
   }

   return;
}
