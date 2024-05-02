/*  Vexed - congrats.c ""Congratulations" special effect"
    Copyright (C) 2003-2006 Scott Ludwig (scottlu@eskimo.com)

    November 29, 2002 - By Scott Ludwig, integrated into Vexed by Mark Ingebretson

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

typedef UInt16 word;
typedef UInt8  byte;
typedef UInt32 dword;

#define kcxSuit0 48
#define kcySuit0 112
#define kcxScreen (160)
#define kyScreen 17
#define kcyScreen (160 - kyScreen)
#define abs(n) ((n) < 0 ? (-(n)) : (n))
#define kcAnims 17
#define kbidChO 0
#define kbidChG 1
#define kbidChS 2
#define kbidChC 3
#define kbidChL 4
#define kbidChR 5
#define kbidChN 6
#define kbidChU 7
#define kbidChI 8
#define kbidChA 9
#define kbidChBang 10
#define kbidChT 11
#define kcxBuffer 10
#define kcyBuffer 9

typedef struct {
   Int16 x;
   Int16 y;
} POINT;

typedef struct {
	POINT ptOrg;	// start
	POINT pt1;		// current
	POINT pt2;		// end
	POINT ptLast;	// last point
	Int16 dx, dy;		// # of x's and y's
	Int16 sx, sy;		// sign of x and y direction
	Int16 dc;			// fraction tracking
	Int16 bid;		// which bitmap
	Int16 dpTriggerNew; // when to trigger a new anim
	Int16 nRadius;	// for rotation
} Anim;

static RectangleType garc[] = {
	{ { 0, 0 }, { 8, 9 } }, // O
	{ { 8, 0 }, { 8, 9 } }, // G
	{ { 16, 0 }, { 8, 9 } }, // S
	{ { 24, 0 }, { 8, 9 } }, // C
	{ { 32, 0 }, { 8, 9 } }, // L
	{ { 40, 0 }, { 8, 9 } }, // R
	{ { 0, 9 }, { 8, 9 } }, // N
	{ { 8, 9 }, { 8, 9 } }, // U
	{ { 16, 9 }, { 4, 9 } }, // I
	{ { 24, 9 }, { 10, 9 } }, // A
	{ { 40, 9 }, { 3, 9 } }, // !
	{ { 0, 18 }, { 10, 9 } } // T
};

POINT gapt[kcAnims];
Anim gaanim[kcAnims];
Int16 gnAnimNext = 0;
Int16 gcStop = 0;
Boolean gfToScreen = true;
Int16 gngl;

// CONGRATULATIONS!!
Int16 gmpnChbidCh[] = {
	kbidChC, kbidChO, kbidChN, kbidChG, kbidChR, kbidChA, kbidChT, kbidChU,
	kbidChL, kbidChA, kbidChT, kbidChI, kbidChO, kbidChN, kbidChS, kbidChBang, kbidChBang
};

// Spacing between characters
Int16 gmpnChDxTrailing[] = {
	1, 2, 2, 2, 1, 0, 0, 2, 0, 0, 0, 1, 2, 2, 0, 2, 0
};

// 0-89 sin / cos values in 8:8 format

unsigned short ga88cos[] = {
	0x0100,	0x00ff,	0x00ff,	0x00ff,	0x00ff,	0x00ff,	0x00fe,	0x00fe,	0x00fd,	0x00fc,
	0x00fc,	0x00fb,	0x00fa,	0x00f9,	0x00f8,	0x00f7,	0x00f6,	0x00f4,	0x00f3,	0x00f2,
	0x00f0,	0x00ee,	0x00ed,	0x00eb,	0x00e9,	0x00e8,	0x00e6,	0x00e4,	0x00e2,	0x00df,
	0x00dd,	0x00db,	0x00d9,	0x00d6,	0x00d4,	0x00d1,	0x00cf,	0x00cc,	0x00c9,	0x00c6,
	0x00c4,	0x00c1,	0x00be,	0x00bb,	0x00b8,	0x00b5,	0x00b1,	0x00ae,	0x00ab,	0x00a7,
	0x00a4,	0x00a1,	0x009d,	0x009a,	0x0096,	0x0092,	0x008f,	0x008b,	0x0087,	0x0083,
	0x0080,	0x007c,	0x0078,	0x0074,	0x0070,	0x006c,	0x0068,	0x0064,	0x005f,	0x005b,
	0x0057,	0x0053,	0x004f,	0x004a,	0x0046,	0x0042,	0x003d,	0x0039,	0x0035,	0x0030,
	0x002c,	0x0028,	0x0023,	0x001f,	0x001a,	0x0016,	0x0011,	0x000d,	0x0008,	0x0004
};

unsigned short ga88sin[] = {
	0x0000,	0x0004,	0x0008,	0x000d,	0x0011,	0x0016,	0x001a,	0x001f,	0x0023,	0x0028,
	0x002c,	0x0030,	0x0035,	0x0039,	0x003d,	0x0042,	0x0046,	0x004a,	0x004f,	0x0053,
	0x0057,	0x005b,	0x005f,	0x0064,	0x0068,	0x006c,	0x0070,	0x0074,	0x0078,	0x007c,
	0x007f,	0x0083,	0x0087,	0x008b,	0x008f,	0x0092,	0x0096,	0x009a,	0x009d,	0x00a1,
	0x00a4,	0x00a7,	0x00ab,	0x00ae,	0x00b1,	0x00b5,	0x00b8,	0x00bb,	0x00be,	0x00c1,
	0x00c4,	0x00c6,	0x00c9,	0x00cc,	0x00cf,	0x00d1,	0x00d4,	0x00d6,	0x00d9,	0x00db,
	0x00dd,	0x00df,	0x00e2,	0x00e4,	0x00e6,	0x00e8,	0x00e9,	0x00eb,	0x00ed,	0x00ee,
	0x00f0,	0x00f2,	0x00f3,	0x00f4,	0x00f6,	0x00f7,	0x00f8,	0x00f9,	0x00fa,	0x00fb,
	0x00fc,	0x00fc,	0x00fd,	0x00fe,	0x00fe,	0x00ff,	0x00ff,	0x00ff,	0x00ff,	0x00ff
};

extern WinHandle            gpwinOffScreen;         // congrats screen window handle

void NewAnim();

//
// Animation code
//

Boolean InitCongrats()
{
	// Create off-screen window
	// UNDONE: Get the dimensions from the bitmap itself

	word w = 0;
   WinHandle pwinSav;
   MemHandle h;
   BitmapType *pbm;

   gpwinOffScreen = WinCreateOffscreenWindow(kcxSuit0, kcySuit0, screenFormat, &w);
	if (gpwinOffScreen == NULL)
		return false;

	// Set it as the current draw window

	pwinSav = WinGetDrawWindow();
	WinSetDrawWindow(gpwinOffScreen);

	// Copy our bitmap into the off screen window

	h = DmGetResource('Tbmp', kidrBitmapCongrats);
	pbm = (BitmapType *)MemHandleLock(h);
	WinDrawBitmap(pbm, 0, 0);
	MemHandleUnlock(h);
	DmReleaseResource(h);

	// Restore

	WinSetDrawWindow(pwinSav);
	return true;
}

void ExitCongrats()
{
	// Destroy offscreen window

	if (gpwinOffScreen != NULL)
		WinDeleteWindow(gpwinOffScreen, 0);
}

void InitAnim()
{
	// Figure out the ending locations of each character of "CONGRATULATIONS!!"

	Int16 cy = garc[kbidChC].extent.y;
	Int16 cx = 0;
	Int16 n,y,x;

	for (n = 0; n < kcAnims; n++) {
		cx += garc[gmpnChbidCh[n]].extent.x;
		cx += gmpnChDxTrailing[n];
	}

	y = kyScreen + (kcyScreen - cy) / 2;
	x = (kcxScreen - cx) / 2;

	// Figure out the pos of each character

	gapt[0].y = y;
	gapt[0].x = x;
	for (n = 1; n < kcAnims; n++) {
		gapt[n].x = gapt[n - 1].x + garc[gmpnChbidCh[n - 1]].extent.x + gmpnChDxTrailing[n - 1];
		gapt[n].y = gapt[0].y;
	}

	// Most anims not doing anything

	for (n = 0; n < kcAnims; n++) {
		Anim *panim = &gaanim[n];
		panim->dx = 0;
		panim->dy = 0;
		panim->sx = 0;
		panim->sy = 0;
		panim->pt1.x = -20;
		panim->pt1.y = -20;
		panim->pt2 = panim->pt1;
		panim->ptLast = panim->pt1;
		panim->bid = gmpnChbidCh[n];
	}

	// Start a new anim

	gngl = 0;
	gfToScreen = true;
	gnAnimNext = 0;
	gcStop = 0;
	NewAnim();
}

void PickOutsidePoint(POINT *ppt)
{
	switch (SysRandom(0) & 3) {
	case 0:
		ppt->x = -kcxBuffer;
		ppt->y = (SysRandom(0) % (kcyScreen + kcyBuffer)) - kcyBuffer + kyScreen;
		break;

	case 1:
		ppt->x = (SysRandom(0) % (kcxScreen + kcxBuffer)) - kcxBuffer;
		ppt->y = kyScreen - kcyBuffer;
		break;

	case 2:
		ppt->x = kcxScreen;
		ppt->y = (SysRandom(0) % (kcyScreen + kcyBuffer)) - kcyBuffer + kyScreen;
		break;

	case 3:
		ppt->x = (SysRandom(0) % (kcxScreen + kcxBuffer)) - kcxBuffer;
		ppt->y = kyScreen + kcyScreen;
		break;
	}
}

Boolean FCheckQuad(POINT *ppt1, POINT *ppt2)
{
	// Must be at least 1 quad away

	Int16 x1 = ppt1->x >= 0 ? ppt1->x : 0;
	Int16 x2 = ppt2->x >= 0 ? ppt2->x : 0;
	Int16 y1 = ppt1->y >= 0 ? ppt1->y : 0;
	Int16 y2 = ppt2->y >= 0 ? ppt2->y : 0;

	if (abs(x1 - x2) < kcxScreen / 2)
		return false;
	if (abs(y1 - y2) < kcxScreen / 2)
		return false;

	return true;
}

void NewPos(POINT *ppt1, POINT *ppt2)
{
	while (true) {
		PickOutsidePoint(ppt1);
		if (ppt2 == NULL)
			return;
		if (!FCheckQuad(ppt1, ppt2))
			return;
	}
}

void NewAnim()
{
   Anim *panim;

	if (gnAnimNext == kcAnims)
		return;

	panim = &gaanim[gnAnimNext];
	if (gfToScreen) {
		panim->pt2 = gapt[gnAnimNext];
		NewPos(&panim->pt1, &panim->pt2);
	} else {
		panim->pt1 = gapt[gnAnimNext];
		NewPos(&panim->pt2, &panim->pt1);
	}
	panim->ptOrg = panim->pt1;
	gnAnimNext++;

	// Number of x's and y's to travel on each axis

	panim->dc = 0;
    panim->dx = panim->pt2.x - panim->pt1.x;
    if (panim->dx < 0)
        panim->dx = -panim->dx;
    panim->dy = panim->pt2.y - panim->pt1.y;
    if (panim->dy < 0)
        panim->dy = -panim->dy;

    // Direction on each axis

    if (panim->pt1.x < panim->pt2.x) {
        panim->sx = 1;
    } else {
        panim->sx = -1;
    }
    if (panim->pt1.y < panim->pt2.y) {
        panim->sy = 1;
    } else {
        panim->sy = -1;
    }

	// When the distance is less than this, a new anim triggers

	panim->dpTriggerNew = (panim->dx + panim->dy) * 15 / 16;
}

void UpdateRadius(Anim *panim)
{
	// Min from current to edges, origin, and destination

	Int16 n;

   panim->nRadius = 1000;
	n = abs(-kcxBuffer - panim->pt1.x);
	if (n < panim->nRadius)
		panim->nRadius = n;
	n = abs(kcxScreen - panim->pt1.x);
	if (n < panim->nRadius)
		panim->nRadius = n;
	n = abs(-kcyBuffer - panim->pt1.y);
	if (n < panim->nRadius)
		panim->nRadius = n;
	n = abs(kyScreen + kcyScreen - panim->pt1.y);
	if (n < panim->nRadius)
		panim->nRadius = n;

	// This puts more interesting limits on the
	// radius but can result in occasional mostly
	// straight trajectories

	n = abs(panim->pt2.x - panim->pt1.x);
	if (n < panim->nRadius)
		panim->nRadius = n;
	n = abs(panim->pt2.y - panim->pt1.y);
	if (n < panim->nRadius)
		panim->nRadius = n;
	n = abs(panim->ptOrg.x - panim->pt1.x);
	if (n < panim->nRadius)
		panim->nRadius = n;
	n = abs(panim->ptOrg.y - panim->pt1.y);
	if (n < panim->nRadius)
		panim->nRadius = n;
}

Boolean Step()
{
	// Clip to card area so we don't draw over foundation
	// stacks

	RectangleType rc;
   Int16 n88cos;
   Int16 n88sin;
   Int16 x88Round;
   Int16 y88Round;
   Int16 c, n, dx, dy, xNew, yNew;
   Boolean fDone;

   rc.topLeft.x = 0;
	rc.topLeft.y = kyScreen;
	rc.extent.x = 160;
	rc.extent.y = 160 - rc.topLeft.y;
	WinSetClip(&rc);

	gngl += 3;
	if (gngl >= 360)
		gngl -= 360;

	if (gngl < 90) {
		n88cos = (Int16)ga88cos[gngl];
		n88sin = (Int16)ga88sin[gngl];
		x88Round = 128;
		y88Round = 128;
	} else if (gngl < 180) {
		n88cos = -(Int16)ga88cos[179 - gngl];
		n88sin = (Int16)ga88sin[179 - gngl];
		x88Round = -128;
		y88Round = 128;
	} else if (gngl < 270) {
		n88cos = -(Int16)ga88cos[gngl - 180];
		n88sin = -(Int16)ga88sin[gngl - 180];
		x88Round = -128;
		y88Round = -128;
	} else {
		n88cos = (Int16)ga88cos[359 - gngl];
		n88sin = -(Int16)ga88sin[359 - gngl];
		x88Round = 128;
		y88Round = -128;
	}

	for (n = 0; n < kcAnims; n++) {

		Anim *panim = &gaanim[n];

		c = 2;
		while (c--) {
			if (panim->dx > panim->dy) {
				panim->pt1.x += panim->sx;
				panim->dc += panim->dy;
				if (panim->dc >= panim->dx) {
					panim->dc -= panim->dx;
					panim->pt1.y += panim->sy;
				}
			} else {
				panim->pt1.y += panim->sy;
				panim->dc += panim->dx;
				if (panim->dc >= panim->dy) {
					panim->dc -= panim->dy;
					panim->pt1.x += panim->sx;
				}
			}

			// If the anim is at the dest, stop

			if (panim->pt1.x == panim->pt2.x && panim->pt1.y == panim->pt2.y) {
				if (panim->dx != 0 || panim->dy != 0)
					gcStop++;
				panim->dx = 0;
				panim->dy = 0;
				panim->sx = 0;
				panim->sy = 0;
			}
		}

		// If it's close enough to trigger new, ask for new anims

		dx = panim->pt2.x - panim->pt1.x;
		if (dx < 0)
			dx = -dx;
		dy = panim->pt2.y - panim->pt1.y;
		if (dy < 0)
			dy = -dy;
		if (dx + dy < panim->dpTriggerNew) {
			panim->dpTriggerNew = -1;
			NewAnim();
		}

		// Draw the anim

		UpdateRadius(panim);
		if (panim->nRadius == 0) {
			dx = 0;
			dy = 0;
		} else {
			dx = (panim->nRadius * n88cos + x88Round) >> 8;
			dy = (panim->nRadius * n88sin + y88Round) >> 8;
		}

		// XOR in the new pos, then XOR out the old pos

		xNew = panim->pt1.x + dx;
		yNew = panim->pt1.y + dy;
		if (xNew != panim->ptLast.x || yNew != panim->ptLast.y) {
			WinCopyRectangle(gpwinOffScreen, NULL, &garc[panim->bid], xNew, yNew, winInvert);
			WinCopyRectangle(gpwinOffScreen, NULL, &garc[panim->bid], panim->ptLast.x, panim->ptLast.y, winInvert);
			panim->ptLast.x = xNew;
			panim->ptLast.y = yNew;
		}
	}

	// Start the next animation?

	fDone = (gcStop == kcAnims);
	if (fDone) {
		if (gfToScreen)
			SysTaskDelay(100);
		gcStop = 0;
		gnAnimNext = 0;
		gfToScreen ^= true;
		NewAnim();
	}

	// Turn off clip rectangle

	WinResetClip();
	return fDone;
}
