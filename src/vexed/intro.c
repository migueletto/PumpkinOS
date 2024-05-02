/*  Vexed - intro.c "The animated intro functions for Vexed"
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

extern WinHandle     ObjectWindowHandles[BitmapCount];
extern PlayGroundIntro currentI;

void DrawBMIntro(Int8 id, UInt16 x, UInt16 y);

//-----------------------------------------------------------------------------
void MoveAnimationI(Int16 sx, Int16 sy, Int16 ddx, Int16 ddy) {
   UInt8             i;
   UInt16            rx,ry;
   RectangleType     rSrc;
   Int16             dx,dy;
   int               count;

   rx = (sx * BlockSizeX) + 5;
   ry = (sy * BlockSizeY);
   //r.extent.x = rSrc.extent.x = BlockSizeX;
   //r.extent.y = rSrc.extent.y = BlockSizeY;
   rSrc.topLeft.x = rSrc.topLeft.y = 0;
   //r.topLeft.x = rx;
   //r.topLeft.y = ry;
//  WinEraseRectangle(&r, 0);                 // erase old bitmap

   dx = ddx;
   dy = ddy;

   count = (ddx != 0) ? BlockSizeX : BlockSizeY;
   for (i = 1; i < count; i++) {
      // animate block move

      DrawBMIntro2(currentI[sy][sx], rx + dx, ry + dy, ddx, ddy);
      SysTaskDelay(1);
      //r.topLeft.x = rx + dx;
      //r.topLeft.y = ry + dy;
      //WinEraseRectangle(&r, 0);           // erase old bitmap
      dx += ddx;
      dy += ddy;

      if (EvtSysEventAvail(false)) {
         return;
      }
   }

   // draw final block
   DrawBMIntro2(currentI[sy][sx], rx + ddx * BlockSizeX, ry + ddy * BlockSizeY, ddx, ddy);

   SysTaskDelay(1);
   // Update current Playground
   currentI[sy + ddy][sx + ddx] = currentI[sy][sx];
   currentI[sy][sx] = AirI;
   return;
}

//-----------------------------------------------------------------------------
void EraseBlockI(Int16 x, Int16 y) {
   RectangleType     r;
   Int8              i;

   for (i = 5; i >= 0; i --) {
      r.topLeft.x = (x * BlockSizeX) + i + 5;
      r.topLeft.y = (y * BlockSizeY) + i;


      r.extent.x = 5 * (5 - i);
      r.extent.y = 2 * (20 - i);
      WinEraseRectangle(&r, 0);     // erase the on screen block
      SysTaskDelay(1);
   }
   // leaves one row of pixels, odd number in a block, delete the rest
   r.topLeft.x = (x * BlockSizeX) + 5;
   r.topLeft.y = y * BlockSizeY;
   r.extent.x = BlockSizeX;
   r.extent.y = BlockSizeY;
   WinEraseRectangle(&r, 0);     // erase the on screen block
}

//-----------------------------------------------------------------------------
Boolean MoveBlockI(Int16 sx, Int16 sy, Int8 direction) {
   Boolean           stop=0;

   // if current block is neither air nor a wall block and
   // if the one to the right is just air then go ahead and move it
   if ((currentI[sy][sx]     != AirI ) &&
       (currentI[sy][sx]     != WallI)) {
      switch (direction) {
      case Right:
         if ((sx < sizeXI-1) &&
             (currentI[sy][sx + 1] == AirI )) {
            stop=0;
            MoveAnimationI(sx,sy,1,0);
         } else {
            stop=1;
         }
         break;
      case Left:
         if ((sx > 0) &&
             (currentI[sy][sx - 1] == AirI )) {
            stop=0;
            MoveAnimationI(sx,sy,-1,0);
         } else {
            stop=1;
         }
         break;
      case Down:
         if ((sy < sizeYI-1) &&
             (currentI[sy + 1][sx] == AirI )) {
            stop=1;
            MoveAnimationI(sx,sy,0,1);
         }
         break;
      case Erase:
         stop=1;
         currentI[sy][sx] = AirI;            // erase the block from the current level array
         EraseBlockI(sx, sy);

         break;
      }
   }
   return stop;
}

//-----------------------------------------------------------------------------
Boolean CompareLevelI(PlayGroundIntro Array1, PlayGroundIntro Array2) {
   Int8              x, y;

   for (y = 0; y < sizeYI; y ++) {
      for (x = 0; x < sizeXI; x ++) {
         if (Array1[y][x] != Array2[y][x]) {
            return 1;
         }
      }
   }
   return 0;
}

//-----------------------------------------------------------------------------
void CopyLevelI(PlayGroundIntro Target, PlayGroundIntro Source) {
   Int8              x, y;

   for (y = 0; y < sizeYI; y++) {
      for (x = 0; x < sizeXI; x++) {
         Target[y][x] = Source[y][x];
      }
   }
   return;
}


//-----------------------------------------------------------------------------
Boolean CheckGravityI() {
   Int8              x, y;
   Boolean           change = 0;

   // go through it bottom to top so as all the blocks tumble down on top of each other
   for (y = sizeYI-2; y >= 0; y--) {
      for (x = sizeXI-1; x >= 0; x--) {
         change |= MoveBlockI(x, y, Down);
         if (EvtSysEventAvail(false)) {
            return change;
         }
      }
   }
   return change;
}

//-----------------------------------------------------------------------------
Boolean CheckMatchesI() {
   Int8              x, y;
   Boolean           change = 0;
   PlayGroundIntro   mark;                   // array for holding blocks marked for removal

   /* just go through from top to bottom marking pieces
    *  which have matches above, below, left and to the right of them */
   for (y = 0; y < sizeYI; y++) {
      for (x = 0; x < sizeXI; x++) {
         // ALWAYS initialise each element in the 'mark' array
         mark[y][x] = 0;
         if ((currentI[y][x] != AirI) && (currentI[y][x] != WallI)) {
            // only check if its not air or a wall block
            if (y != 0) {
               // if there is an identical piece above then mark this piece
               mark[y][x] |= (currentI[y - 1][x] == currentI[y][x]);
            }
            if (y != sizeYI-1) {
               // if there is an identical piece below then mark this piece
               mark[y][x] |= (currentI[y + 1][x] == currentI[y][x]);
            }
            if (x != 0) {
               // if there is an identical piece to the left then mark this piece
               mark[y][x] |= (currentI[y][x - 1] == currentI[y][x]);
            }
            if (x != sizeXI-1) {
               // if there is an identical piece to the right then mark this piece
               mark[y][x] |= (currentI[y][x + 1] == currentI[y][x]);
            }
         }
      }
   }
   // go through the current level array checking
   // if a piece is marked and if so, remove it
   for (y = 0; y < sizeYI; y ++) {
      for (x = 0; x < sizeXI; x ++) {
         if (mark[y][x]) {
            change |= MoveBlockI(x, y, Erase);
            if (EvtSysEventAvail(false)) {
               return change;
            }
         }
      }
   }
   return change;
}

//-----------------------------------------------------------------------------
void EraseWallsI() {
   Int8              x, y;

   /* just go through from top to bottom marking pieces
    *  which have matches above, below, left and to the right of them */
   for (y = 0; y < sizeYI; y++) {
      for (x = 0; x < sizeXI; x++) {
         if (currentI[y][x] == WallI) {
            EraseBlockI(x, y);
         }
      }
   }
   return;
}

//-----------------------------------------------------------------------------
void DrawBMIntro2(Int8 id, UInt16 x, UInt16 y, int ddx, int ddy) {
   RectangleType     r;

   // copy entire window, one Intro block
   r.topLeft.x = r.topLeft.y = 1;
   r.extent.x = BlockSizeX;
   r.extent.y = BlockSizeY;

   // Blocks have a 1 pixel border that is used for erasing when a block
   // is animating.
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

   id += BitmapIntroOffset + 7;              // +8 - 1

   ErrFatalDisplayIf (ObjectWindowHandles[id] == 0, MSG_ERR_UNHANDLED_IMAGE );

   // Copy the source window (contains the image to draw) to the draw window.
   WinCopyRectangle (ObjectWindowHandles[id], 0, &r, x, y, winPaint);

}

//-----------------------------------------------------------------------------
void DrawBMIntro(Int8 id, UInt16 x, UInt16 y) {
   DrawBMIntro2(id, x, y, 0, 0);
}

//-----------------------------------------------------------------------------
void PlayerMakeMoveI(Int16 sx, Int16 sy, Int8 direction, Int16 dist) {
   // returns true if this move completes the level
   Int8              dx = 0;
   Boolean           stop = 0;

   // to hold the level position before a check gravity call
   PlayGroundIntro   previous;

   // simply pass parameters through to MoveBlockI function
   while ((dist-- > 0) && (!stop)) {
      stop |= MoveBlockI(sx+dx, sy, direction);
      if (EvtSysEventAvail(false)) {
         return;
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
            CopyLevelI(previous,currentI);
            stop |= CheckGravityI();          // check for gravity defying blocks and bring them down to earth!
         } while (CompareLevelI(previous, currentI));
         stop |= CheckMatchesI();             // check for any groups of matching blocks
      } while (CompareLevelI(previous, currentI));
   }
   return;
}


