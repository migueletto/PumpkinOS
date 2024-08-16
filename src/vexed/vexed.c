/*  Vexed - Vexed.c "The Main Stuff, i.e. event handlers, loops, etc"
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

#define ALLOW_ACCESS_TO_INTERNALS_OF_BITMAPS

#include "PalmOS.h"

#pragma pack(2)

#include "vexed.h"
#include "protos.h"
#include "webbrowsermanager.h"

//#define repeatDelta			(SysTicksPerSecond() / 4)	/* 0.5 seconds */

static Int16 StartApplication(void);
static void EventLoop(void);
static void StopApplication(void);
static Boolean frmMainEventH(EventPtr event);
static Boolean frmAboutEventH(EventPtr event);
static Boolean frmPrefsEventH(EventPtr event);
static Boolean frmInfoEventH(EventPtr event);
static Boolean frmIntroEventH(EventPtr event);
static Boolean frmSolutionEventH(EventPtr event);
void SetupButton(UInt16 controlId, Boolean upButton, Boolean turnedOn);
static Boolean LoadLastLevel();

char   gszTitle[64];

struct {
   struct {
      Int16 x, y;
   } begin, end, pending;
} playermove;

extern Preferences   VexedPreferences;
extern PlayGround    current;
extern PlayGround    stored;
extern Int8          CheatCount;
extern PlayGround    undo[UndoSize];
extern Int16         undoMovesMade[UndoSize];
extern UInt8         UndoCount;
extern UInt8         UndoIndex;
extern Boolean       memorized;
extern UInt8         ReplayCurrent,
                     ReplayPrevious;
extern MoveType      replay[2][ReplaySize];  // First row is previous moves, second row is current level
extern UInt8         ReplayCurrent,          // counts of moves
                     ReplayPrevious;
extern UInt16        cardNo;                 // for OS calls
extern LocalID       appID;                  // for OS calls
extern int				screenMode;             // number of bit color (1/4/8)
extern Boolean       refreshIcons;           // after preferences panel saved, refresh control icons
extern Int16         selectLevel;            // temp var for select level form
extern Int16         selectLevelRepeatCnt;   // Count of number of repeat events on select level form
extern Int8          selectDelta;            // count to add each button press
extern Boolean       cheatOn;
extern Boolean       InBlockCheck;           // are we in a block check
extern MemHandle	   ObjectBitmapHandles[BitmapCount];
extern BitmapPtr	   ObjectBitmapPtr[BitmapCount];
extern WinHandle	   ObjectWindowHandles[BitmapCount];
extern Boolean       InSolution;             // Are we displaying a solution
extern Boolean       InSolutionBack;         // are we doing a back move in solution?
extern Int16         MoveNum;                // Which move of solution are we now displaying
extern Boolean       SolutionRun;            // Is solution in free-running mode
extern Int8          OldSet;                 // Save off previous block set num
extern char          *MonoListP[];           // monochrome popup list has to be global
extern UInt16        GameVolume;             // volume from preferences
extern char          *GraySListP[];          // grayscale popup list has to be global
extern Boolean       in1Bit;                 // Are we in 1 bit mode (could be 4 bit capable down in 1 bit mode)
extern Boolean       movePending;            // Have they tapped, waiting for tap 2?

extern void CopyLevel(PlayGround Target, PlayGround Source);

char   gszEllipsis[4];
char   stringSolution[512];
Int16  MoveCountMadeStored;
Int16  MoveCountParStored;

UInt32            romVersion;


//---------------------------------------------------------------------------
UInt32 PilotMain(UInt16 cmd, void *cmdPBP, UInt16 launchFlags) {
   Int16             error;
   if (cmd == sysAppLaunchCmdNormalLaunch) {
      error = StartApplication();            // Application start code
      if (error)
         return error;
      EventLoop();                           // Event loop
      StopApplication ();                    // Application stop code
   }

   return 0;
}

//-----------------------------------------------------------------------------
void LoadBitmaps(Boolean LoadAll) {
   WinHandle         oldDrawWinH;            // save off old draw handle
   UInt8             i;
   Err               err;
   UInt8             idx;
   RectangleType     r;
   UInt8             MonoOffset;
   Coord width, height;
   #ifdef HIRES
   WindowFormatType  wft=nativeFormat;
   #else
   WindowFormatType  wft=screenFormat;
   #endif

   if (in1Bit) {                             // if we're using 1 bit bitmaps
      MonoOffset = MonoBMOffset;
   }
   else {
      MonoOffset = 0;                        // use 4 or 8 bit bitmaps
   }

   // Keep the Object graphics locked because they are frequently used
   oldDrawWinH = WinGetDrawWindow();
   r.topLeft.x = r.topLeft.y = 0;
   if (LoadAll) {
      for (i = 0; i < BitmapCount; i++) {
         int x, y;
         int cx;
         // 8 block bitmaps are at the front of the array, followed by the wall
         idx = i + 8;
         ObjectBitmapHandles[idx] = DmGetResource( bitmapRsc, firstBitmap + i + MonoOffset);
         ObjectBitmapPtr[idx] = MemHandleLock(ObjectBitmapHandles[idx]);
         BmpGetDimensions(ObjectBitmapPtr[idx], &width, &height, NULL);

         // Make erasing border around intro bitmaps
         x = 0;
         y = 0;
         cx = 0;
         //cy = 0;
         if (firstBitmap + i + MonoOffset >= BitmapIntroV && firstBitmap + i + MonoOffset <= BitmapIntroW) {
            x = 1;
            y = 1;
            cx = 2;
            //cy = 2;
         }
         ObjectWindowHandles[idx] = WinCreateOffscreenWindow(width + cx, height + cx, wft, &err);
         WinSetDrawWindow(ObjectWindowHandles[idx]);
         r.extent.x = width + cx;
         r.extent.y = height + cx;
         WinEraseRectangle(&r, 0);

         WinDrawBitmap(ObjectBitmapPtr[idx], x, y);
      }
   }
   for (i = 0; i < 8; i++) {
      if (screenMode > 2) {
         ObjectBitmapHandles[i] = DmGetResource( bitmapRsc, BitmapBS_Extras + (VexedPreferences.BlockSet * 10) + i);
      }
      else {
         ObjectBitmapHandles[i] = DmGetResource( bitmapRsc, MBitmapBS_Extras + (VexedPreferences.BlockSet * 10) + i);
      }
      ObjectBitmapPtr[i] = MemHandleLock(ObjectBitmapHandles[i]);
      BmpGetDimensions(ObjectBitmapPtr[i], &width, &height, NULL);

      ObjectWindowHandles[i] = WinCreateOffscreenWindow(width + 3, height + 2, wft, &err);
      WinSetDrawWindow(ObjectWindowHandles[i]);
      r.extent.x = width + 3;
      r.extent.y = height + 2;
      WinEraseRectangle(&r, 0);

      WinDrawBitmap(ObjectBitmapPtr[i], 1, 1);
   }

   // restore old draw window
   WinSetDrawWindow(oldDrawWinH);

}

//-----------------------------------------------------------------------------
int SetScreenMode()
{
   //Err    err;
   UInt16 cBpp;
	UInt32 dwVer;
   UInt32 dwDepth = 0;

#ifndef HIRES
	// Use the highest bpp possible on a given device
	// PalmOS 2.0 doesn't have ScrDisplayMode

	UInt32 dwDepthsSupported = 0;
	static Int16 gaBpps[] = { 0 /*24*/, 0 /* 16 */, 8, 4, 2, 1 };
   Int8   n;

	FtrGet(sysFtrCreator, sysFtrNumROMVersion, &dwVer);
	if (dwVer < 0x03003000) {
		cBpp = 1;
		return cBpp;
	}

	// Get the depths supported
	WinScreenMode(winScreenModeGetSupportedDepths, NULL, NULL, &dwDepthsSupported, NULL);

	// Set the highest depth possible

	cBpp = 1;
	for (n = 0; n < 6; n++) {
		if (dwDepthsSupported & (1UL << (gaBpps[n] - 1))) {
			WinScreenMode(winScreenModeGet, NULL, NULL, &dwDepth, NULL);
			if (dwDepth != gaBpps[n]) {
				dwDepth = gaBpps[n];
				err = WinScreenMode(winScreenModeSet, NULL, NULL, &dwDepth, NULL);
				if (err == 0) {
					cBpp = (int)dwDepth;
					break;
				}
			} else {
				cBpp = (int)dwDepth;
				break;
			}
		}
	}

#else

   #define ourMinVersion	sysMakeROMVersion(5,0,0,sysROMStageDevelopment,0)

   UInt32 dwHeight=320;
   UInt32 dwWidth=320;
   UInt32 version;
   UInt32 attr;

   // first check the ROM for high density
	FtrGet(sysFtrCreator, sysFtrNumROMVersion, &dwVer);
   FtrGet(sysFtrCreator, sysFtrNumWinVersion, &version);

	if ((dwVer < ourMinVersion) || (version < 4)) {
      FrmAlert (AlertNoHiRes);
		return( 0 );			// 0 - means - "Houston, we got problems"
   }
   else {
      // set up screen to be high density coordinate system
      // WinSetCoordinateSystem(kCoordinatesDouble);
      WinScreenGetAttribute(winScreenDensity, &attr);
      if (attr == kDensityDouble) {
         //the screen is double density
         dwDepth = 8;
         WinScreenMode(winScreenModeSet, &dwWidth, &dwHeight, &dwDepth, NULL);
         WinScreenMode(winScreenModeGet, &dwWidth, &dwHeight, &dwDepth, NULL);
      }
      cBpp = 8;            // 8 bit high density used
   }
#endif

   return cBpp;
}

//-----------------------------------------------------------------------------
void FreeBitmaps(Boolean fFreeAll)
{
   UInt8             i;
   UInt8             count;

   if (fFreeAll) {
      count = BitmapCount + 8;
   } else {
      count = 8;
   }

   // Unlock and release the locked bitmaps
   for (i = 0; i < count; i++) {
      MemPtrUnlock(ObjectBitmapPtr[i]);
      DmReleaseResource(ObjectBitmapHandles[i]);

      if (ObjectWindowHandles[i])
         WinDeleteWindow(ObjectWindowHandles[i], false);
   }
}

//-----------------------------------------------------------------------------
static Int16 StartApplication(void) {
   //UInt32            depth;
//   UInt32            romVersion;
   Boolean           fSkipIntro;
   SoundLevelTypeV20 V2GameVolume;


   screenMode = SetScreenMode();

	// screenMode == 0 means we got no hardware to work on
	// and shall quit the app with error
	if( screenMode == 0 ) {
		AppLaunchWithCommand( sysFileCDefaultApp, sysAppLaunchCmdNormalLaunch, NULL );
		return true;
	}

   // ensure bitmap offsets work for 1 bit monochrome
   if (screenMode < 4) {                     // hardware only supports 1bit mono
      in1Bit = true;
   }

   // determine game volume level for sound
   FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);

   if (romVersion >=  sysMakeROMVersion(3, 0, 0, sysROMStageRelease, 0)) {
      GameVolume = PrefGetPreference (prefGameSoundVolume);
   }
   else {
      V2GameVolume = PrefGetPreference (prefGameSoundLevelV20);
      if (V2GameVolume == slOff) {           // no sound wanted for games
         GameVolume = 0;
      }
      else {
         GameVolume = sndMaxAmp;             // they want sound
      }
   }
   // Get Ellipsis chars figured out. In OS 3.3 and above
   // there is a special ellipsis character.

   if (romVersion >= 0x03303000) {
      gszEllipsis[0] = '\30';
      gszEllipsis[1] = 0;
   } else {
      gszEllipsis[0] = '.';
      gszEllipsis[1] = '.';
      gszEllipsis[2] = '.';
      gszEllipsis[3] = 0;
   }


   // load preferences
   LoadPreferences();

   if (VexedPreferences.BlockSet > 6) {
      in1Bit = true;
   }

   // WinDrawBitmap maps foreground / background color to UIColors when loading bitmaps.
   // This affects color bitmaps differently than B&W bitmaps, of which Vexed
   // has a mix. In addition, the color bitmaps have antialiasing built into them which
   // is assuming mixing into a white background. To circumvent these problems, we're
   // setting the background colors to white.

	if (romVersion >= 0x03503000) {
      RGBColorType rgbc;
      rgbc.index = 0;
      rgbc.r = 0xff;
      rgbc.g = 0xff;
      rgbc.b = 0xff;
      rgbc.index = WinRGBToIndex(&rgbc);
		UIColorSetTableEntry(UIDialogFill, &rgbc);
		UIColorSetTableEntry(UIFormFill, &rgbc);
      WinSetBackColor(rgbc.index);
	}


   // load all of the bitmaps
   LoadBitmaps(true);

   // attempt to load a last played level
   fSkipIntro = VexedPreferences.SkipIntro;
   if( !LoadLastLevel() ) {
		if( GetLevelPackCount() > 0 ) {
			// ok, there're other levels so we just drop user a note
			// and let him pick up any other to fight agains

			Boolean Loaded = false;

			while( Loaded != true ) {
				// let's alert the user. S/he can quit now too
				if( FrmAlert( AlertLoadFailed ) == 0 ) {
					Loaded = DoLevelPackForm( true );
					fSkipIntro = true;
				} else {
					// wanna quit then let get him to the launcher
					AppLaunchWithCommand( sysFileCDefaultApp, sysAppLaunchCmdNormalLaunch, NULL );
					return false;
				}
			}
		} else {
			// no levels at all... bye bye
			FrmAlert( AlertNoLevelPacksInstalled );
			AppLaunchWithCommand( sysFileCDefaultApp, sysAppLaunchCmdNormalLaunch, NULL );
			return false;
		}
   }

   if (fSkipIntro) {
      FrmGotoForm(FormMain);
   }
   else {
      FrmGotoForm(FormIntro);
   }
   return false;        // default to no error
}

//-----------------------------------------------------------------------------
Boolean LoadLastLevel()
{
   //int x, y;
   int n;
   Boolean fLoaded;
   LevelPackPrefs *pLevelPack = &VexedPreferences.LevelPack[0];

   // If the pref board is valid, we have nothing to do.

   if (pLevelPack->LevelValid) {
      CLevelToLevel(pLevelPack->Level, current);
      return true;
   }
   // attempt to load one of the previous levels
   fLoaded = false;
   for (n = 0; n < MaxLevelPackPrefs; n++) {
      if (LoadLevel(VexedPreferences.LevelPack[n].DbName, VexedPreferences.LevelPack[n].CurrentLevel, true)) {
         fLoaded = true;
         break;
      }
   }

   // if failed to load any level, try the classic levels
   if (!fLoaded) {
      // force default level pack
      if (LoadLevel(DefaultLevelPackName, 0, true))
         fLoaded = true;
   }

   // return if this succeeded.
   return fLoaded;
}

//{{{ EventLoop -----------------------------------------------------------------------------
static void EventLoop(void) {
   Err             err;
   Int16             formID;
   FormPtr           form;
   EventType         event;

   do {
      EvtGetEvent(&event, 200);
      if (SysHandleEvent(&event)) {
         continue;
      }
      if (MenuHandleEvent((void *)0, &event, &err)) {
         continue;
      }
      if (event.eType == frmLoadEvent) {
         formID = event.data.frmLoad.formID;
         form = FrmInitForm(formID);
         FrmSetActiveForm(form);
         switch (formID) {
         case FormMain:
            FrmSetEventHandler(form, (FormEventHandlerPtr) frmMainEventH);
            break;

         case FormAbout:
            FrmSetEventHandler(form, (FormEventHandlerPtr) frmAboutEventH);
            break;

         case FormPrefs:
            FrmSetEventHandler(form, (FormEventHandlerPtr) frmPrefsEventH);
            break;

         case FormInfo:
            FrmSetEventHandler(form, (FormEventHandlerPtr) frmInfoEventH);
            break;

         case FormIntro:
            FrmSetEventHandler(form, (FormEventHandlerPtr) frmIntroEventH);
            break;

         case FormSolution:
            FrmSetEventHandler(form, (FormEventHandlerPtr) frmSolutionEventH);
            break;
         }
      }
      FrmDispatchEvent(&event);
   } while (event.eType != appStopEvent);
   return;
}
//}}}
//-----------------------------------------------------------------------------
static void StopApplication(void) {
   FreeBitmaps(true);
   SavePreferences();
   FrmCloseAllForms();
}

//-----------------------------------------------------------------------------
void NextLevel(void) {
   Int16             i;
   LevelPackPrefs *pprefs = &VexedPreferences.LevelPack[0];

	// integrate move score
	if (pprefs->CurrentLevel < kcParRecords) {
      // If this level was played already with a lower score, don't record the higher score
      if (pprefs->SolvedLevels >= pprefs->CurrentLevel) {
         // Is the new score less than new score? If so take it.
         if (pprefs->MoveCountMade < pprefs->Score[pprefs->CurrentLevel].MoveCountMade)
		      pprefs->Score[pprefs->CurrentLevel].MoveCountMade = (UInt8)pprefs->MoveCountMade;
      } else {
		   pprefs->Score[pprefs->CurrentLevel].MoveCountMade = (UInt8)pprefs->MoveCountMade;
      }
		pprefs->Score[pprefs->CurrentLevel].MoveCountPar = (UInt8)pprefs->MoveCountPar;
	} else {
		pprefs->BaseMoveCountMade += pprefs->MoveCountMade;
		pprefs->BaseMoveCountPar += pprefs->MoveCountPar;
	}
	pprefs->MoveCountMade = 0;
	pprefs->MoveCountPar = 0;

   // move onto next level if current is completed
   pprefs->CurrentLevel ++;
   //pprefs->CurrentLevel = 59; // $$TEMP debug help for congrats screen
   if (pprefs->CurrentLevel > pprefs->SolvedLevels + 1) {
      pprefs->SolvedLevels = pprefs->CurrentLevel - 1;
   }
   LoadLevel(NULL, pprefs->CurrentLevel, false);
   ReplayPrevious = ReplayCurrent;
   for (i=0; i < ReplayCurrent; i++) {
      replay[ReplayPIndex][i] = replay[ReplayCIndex][i];
   }
   ReplayCurrent = 0;

   SavePreferences();
   return;
}

//{{{ frmIntroEventH -----------------------------------------------------------------------------
static Boolean frmIntroEventH(EventPtr event) {
   FormPtr           form;
   Int16             handled = 0;
   //Err               err;
   Int16             penx, peny;
   Boolean           bstate;

   switch (event->eType) {
   case frmOpenEvent:

      // flush key/pen queues
      EvtFlushKeyQueue();
      EvtFlushPenQueue();
      form = FrmGetActiveForm();

      FrmDrawForm(form);
      FrmHideObject(form, FrmGetObjectIndex(form, ButtonOK3));
      DrawIntro();                           // draws the intro screen upon game startup
      if (EvtSysEventAvail(false)) {         // they want to quit early
         EvtFlushPenQueue();
         SysTaskDelay(50);
         FrmGotoForm(FormMain);
      }
      else {
         FrmShowObject(form, FrmGetObjectIndex(form, ButtonOK3));
      }
      handled = 1;
      break;
   case ctlSelectEvent:
      if (event->data.ctlEnter.controlID == ButtonOK3) {
         EvtFlushPenQueue();
         FrmGotoForm(FormMain);
         handled = 1;
      }
      break;
	case penUpEvent:
      EvtFlushPenQueue();					// flush any other pen events
      EvtGetPen(&penx, &peny, &bstate);
      EvtFlushPenQueue();
      handled = 1;
      break;
  default:
      break;
   }
   return handled;
}
//}}}
//{{{ ExitSolution -------------------------------------------------------------------------------
void ExitSolution() {
	CopyLevel(current,stored);
	VexedPreferences.LevelPack[0].MoveCountMade = MoveCountMadeStored;
	VexedPreferences.LevelPack[0].MoveCountPar = MoveCountParStored;
	InSolution = false;
	stored[0][0] = 0;								// clear memorized area
	memorized = false;
	FrmReturnToForm(0);
	UpdateStats();
}
//}}}
//{{{ MakeSolutionMove ---------------------------------------------------------------------------
void MakeSolutionMove(UInt8 BackNum) {
	int x, y;
	char chX, chY;
	int count;
	Int8 direction;

   // if in back function we know it won't be the end
   if (InSolutionBack) {
      // Get the move


      chX = stringSolution[BackNum * 2];
      chY = stringSolution[BackNum * 2 + 1];

   }
   else {
	   // End of the solution?

	   count = StrLen(stringSolution);
	   if (MoveNum * 2 >= count) {
		   ExitSolution();
	   	return;
   	}
      // Get the move


      chX = stringSolution[MoveNum * 2];
      chY = stringSolution[MoveNum * 2 + 1];
   }

	x = chX - 'a';
	if (chX <= 'Z') {
		direction = Left;
		x = chX - 'A';
	}
	y = chY - 'a';
	if (chY <= 'Z') {
		direction = Right;
		y = chY - 'A';
	}

	// Check for validity

	if (x < 0 || x >= sizeX || y < 0 || y >= sizeY) {
		ExitSolution();
		return;
	}

	// Add the event
   if (InSolutionBack) {                     // if doing a solution back event, just do the move
      (void) PlayerMakeMove(x, y, direction, 1);
   }
   else {                                    // otherwise queue it
      AddMoveEvent(x, y, direction);
      MoveNum++;
   }
	return;
}
//}}}
//{{{ frmSolutionEventH --------------------------------------------------------------------------
static Boolean frmSolutionEventH(EventPtr event) {
   FormPtr           form;
   Int16             handled = 0;
   //Err               err;
   //Boolean           tmp;
   //RectangleType     r;
   Int16             penx, peny;
   Boolean           bstate;
   Int8              i;

   switch (event->eType) {
   case frmOpenEvent:
      InSolution = true;
      InSolutionBack = false;
      SolutionRun = true;                    // free-running until they tap
      MoveNum = 0;

      // memorize state of current level
      CopyLevel(stored,current);
		MoveCountMadeStored = VexedPreferences.LevelPack[0].MoveCountMade;
		MoveCountParStored = VexedPreferences.LevelPack[0].MoveCountPar;

      // flush key/pen queues
      EvtFlushKeyQueue();
      EvtFlushPenQueue();
      form = FrmGetActiveForm();

      if (GetLevelProperty(VexedPreferences.LevelPack[0].CurrentLevel, "title", gszTitle, sizeof(gszTitle)))
         FrmSetTitle(form, gszTitle);

      FrmDrawForm(form);
      DrawBM(BitmapRestart, LocRestart, BottomRow, true);
      LoadLevel(NULL, VexedPreferences.LevelPack[0].CurrentLevel, false);

      // add the first move into the queue
		MakeSolutionMove(0);
      handled = 1;
      break;
   case ctlSelectEvent:
      if (event->data.ctlEnter.controlID == ButtonDone) {
         ExitSolution();
         handled = 1;
      }
      break;
   case keyDownEvent:
      if (SolutionRun) {                     // we were running in automatic mode
         SolutionRun = false;
      }
      else if (event->data.keyDown.chr == pageDownChr) {   // page up key pressed?
         MakeSolutionMove(0);
      }
      else if (event->data.keyDown.chr == pageUpChr) {   // page down key pressed?
         goto SolutionBack;
      }
      handled = 1;
      break;
   case MakeMoveEvent:
      if (PlayerMakeMove(event->data.generic.datum[0], event->data.generic.datum[1],
                           event->data.generic.datum[2], 1)) {
			// all done
         // cycle back to the beginning instead of exiting
         SolutionRun = false;
         LoadLevel(NULL, VexedPreferences.LevelPack[0].CurrentLevel, false);
         MoveNum = 0;
		}
		else if (SolutionRun) {

			// don't queue another if there's a pen event out there
			if (!EvtSysEventAvail(false)) {     // they want to quit early
				MakeSolutionMove(0);					// if all done
			}
		}
      handled = 1;
      break;
	case penUpEvent:
      EvtFlushPenQueue();					// flush any other pen events
      EvtGetPen(&penx, &peny, &bstate);
      if (SolutionRun) {                     // we were running in automatic mode
         SolutionRun = false;
         // did they press the done button?
         #ifdef HIRESDOUBLE
         if ((penx >10) && (penx < 88) && (peny > 294) && (peny < 318)) {
         #else
         if ((penx >5) && (penx < 44) && (peny > 147) && (peny < 159)) {
         #endif
            MakeSolutionMove(0);
         }
      }
      else {
			// did they tap on a control icon at the bottom?
			if (peny > 143) {                   // command icon area?
				// ignore
				if (penx < 70) {
				}
				// back
				else if (penx < 80) {
SolutionBack:
               if (MoveNum > 0) {
                  InSolutionBack = true;
                  LoadLevel(NULL, VexedPreferences.LevelPack[0].CurrentLevel, false);
                  for (i = 0; i < MoveNum - 1; i++) {
                     MakeSolutionMove(i);
                  }
                  UpdateBlocks();            // redraw the blocks
                  MoveNum--;
                  InSolutionBack = false;
               }
				}
				// step
				else if (penx < 90) {
					MakeSolutionMove(0);
				}
            // restart
            else if (penx > 143) {
               SolutionRun = true;
               LoadLevel(NULL, VexedPreferences.LevelPack[0].CurrentLevel, false);
               MoveNum = 0;
					MakeSolutionMove(0);
            }
			}
      }
      handled = 1;
      break;
   default:
      break;
   }
   return handled;
}
//}}}
//{{{ DoTitleSelect ------------------------------------------------------------------------------
void DoTitleSelect()
{
   EventType event;

   MemSet (&event, sizeof(EventType), 0);
   event.eType = keyDownEvent;
   event.data.keyDown.chr = menuChr;
   event.data.keyDown.keyCode = 0;
   event.data.keyDown.modifiers = commandKeyMask;
   EvtAddEventToQueue (&event);
}
//}}}
//{{{ clearMovePending ---------------------------------------------------------------------------
void clearMovePending() {
   RectangleType     r;

   movePending = false;
   // clear the pending icon
   r.topLeft.x = 80;
   r.topLeft.y = BottomRow;
   r.extent.x = BlockSize;
   r.extent.y = BlockSize;
   WinEraseRectangle(&r, 0);
}
//}}}

//{{{ frmMainEventH ------------------------------------------------------------------------------
Boolean	controlButtonPressed = false;
static Boolean frmMainEventH(EventPtr event)
{
   FormPtr           form;
   Int16             handled = 0;
   Int8              dist;
   Int16             penx, peny;
   Boolean           bstate;
   UInt8             Button;
   RectangleType     r;
   //UInt32            KeyStatus;
   //Err               err;


   switch (event->eType) {
//{{{ frmOpenEvent			.
   case frmOpenEvent:
#ifdef HIRESDOUBLE
	//  WinSetCoordinateSystem(kCoordinatesDouble);
#endif
      form = FrmGetActiveForm();
      FrmDrawForm(form);

      // Draw current level
      DrawLevel( false );

      // Reset replay counts, other initialization
      ReplayCurrent = ReplayPrevious = 0;
      cheatOn = false;
      InBlockCheck = false;                  // are we in a block check?
      movePending = false;                   // are we in between taps of a move?

      // draw graphic controls at bottom of screen
      DrawControlIcons();
      EvtFlushPenQueue();
      handled = 1;
      break;
//}}}
//{{{ frmUpdateEvent			.
   case frmUpdateEvent:                      // fix draw of screen after modal dialogs
      form = FrmGetActiveForm();
      FrmDrawForm(form);
      RedrawLevel();
      DrawControlIcons();
      if (UndoCount > 0) {
         DrawBM(BitmapUndo, LocUndo, BottomRow, true);
      }
      if (memorized) {                       // anything memorized?
         DrawBM(BitmapRecall, LocRecall, BottomRow, true);// load the recall icon
      }
      if (movePending) {
         DrawBM(BitmapPending, LocPending, BottomRow, true);// load the pending icon
      }
      handled = 1;
      break;
//}}}
//{{{ penDownEvent			.
   case penDownEvent:
      EvtGetPen(&penx, &peny, &bstate);
      if (peny < TopRow) {                    // title bar area?
         DoTitleSelect();
         handled = 1;
         break;                              // return to OS
      }

      if (peny < BottomRow) {                      // the gameplay area
         playermove.begin.x = (Int16) (penx / BlockSize);
         playermove.begin.y = (Int16) ((peny / BlockSize) - 1);

         // first determine if in a valid block
         if ((current[playermove.begin.y][playermove.begin.x] != Air ) &&
             (current[playermove.begin.y][playermove.begin.x] != Wall)) {

            // ensure PageUp key is down
            if (KeyCurrentState() & keyBitPageUp) {
               BlockCheck(playermove.begin.x, playermove.begin.y);
            }
         }
         handled = 1;
         break;
      } else {
			Boolean DoSwitchTheLevel = true;				// determines if we shall change the level.
																	// used for the First/Prev/Next/Last Button
																	// only
	      Button = penx >> 4;

			if( controlButtonPressed == false ) {

        // XXX commented because the condition above would not be satisfied the
        // second time the button was clicked
				//controlButtonPressed = true;

				switch (Button) {
					//{{{ FirstButton - First level icon			.
					case FirstButton:
						DoSwitchTheLevel = ( ReplayCurrent > 0 ) ? (FrmAlert(AlertConfirmLevelChange) == 0) : true;

						if( DoSwitchTheLevel ) {
							if (VexedPreferences.LevelPack[0].CurrentLevel != 0) {
								VexedPreferences.LevelPack[0].CurrentLevel = 0;
								LoadLevel(NULL, VexedPreferences.LevelPack[0].CurrentLevel, false);
							}
							CheatCount=0;
							ReplayCurrent = 0;
							clearMovePending();              // clear flag, icon
						}
						handled = 1;
						break;
					//}}}
					//{{{ PreviousButton - Previous level icon	.
					case PreviousButton:
						DoSwitchTheLevel = ( ReplayCurrent > 0 ) ? (FrmAlert(AlertConfirmLevelChange) == 0) : true;

						if( DoSwitchTheLevel ) {
							if (VexedPreferences.LevelPack[0].CurrentLevel != 0) {
								VexedPreferences.LevelPack[0].CurrentLevel--;
								LoadLevel(NULL, VexedPreferences.LevelPack[0].CurrentLevel, false);
							}
							CheatCount=0;
							ReplayCurrent = 0;
							clearMovePending();              // clear flag, icon
						}
						handled = 1;
						break;
					//}}}
					//{{{ SelectButton - Select level icon			.
					case SelectButton:
						DoSelectLevel();
						clearMovePending();              // clear flag, icon
						handled = 1;
						break;
					//}}}
					//{{{ NextButton - Next level icon				.
					case NextButton:
						DoSwitchTheLevel = ( ReplayCurrent > 0 ) ? (FrmAlert(AlertConfirmLevelChange) == 0) : true;

						if( DoSwitchTheLevel ) {
							// $$TEMP$$  changed cheatcount to 1, change back 4
							if ((	(VexedPreferences.LevelPack[0].SolvedLevels != -1)
									&& (VexedPreferences.LevelPack[0].CurrentLevel <= VexedPreferences.LevelPack[0].SolvedLevels))
									|| ((cheatOn) && (CheatCount >= 1))
								) {
								VexedPreferences.LevelPack[0].CurrentLevel++;
								LoadLevel(NULL, VexedPreferences.LevelPack[0].CurrentLevel, false);
							} else {
								CheatCount++;
								}
							clearMovePending();              // clear flag, icon
							ReplayCurrent = 0;
						}
						handled = 1;
						break;
					//}}}
					//{{{ LastButton - Last level icon				.
					case LastButton:
						DoSwitchTheLevel = ( ReplayCurrent > 0 ) ? (FrmAlert(AlertConfirmLevelChange) == 0) : true;

						if( DoSwitchTheLevel ) {
							// if current != Solved
							if (VexedPreferences.LevelPack[0].SolvedLevels != -1) {
								if (VexedPreferences.LevelPack[0].CurrentLevel < VexedPreferences.LevelPack[0].SolvedLevels + 1) {
									VexedPreferences.LevelPack[0].CurrentLevel = VexedPreferences.LevelPack[0].SolvedLevels + 1;
									if (VexedPreferences.LevelPack[0].CurrentLevel == VexedPreferences.LevelPack[0].MaxLevels) {
										VexedPreferences.LevelPack[0].CurrentLevel = VexedPreferences.LevelPack[0].MaxLevels - 1;
									}
									LoadLevel(NULL, VexedPreferences.LevelPack[0].CurrentLevel, false);
								}
								CheatCount=0;
								ReplayCurrent=0;
							}
							clearMovePending();              // clear flag, icon
						}
						handled = 1;
						break;
					//}}}

					//{{{ PendingButton - Pending icon				.
					case PendingButton:
						if (movePending) {
							clearMovePending();
						}
						handled = 1;
						break;
					//}}}
					//{{{ UndoButton - Undo icon						.
					case UndoButton:
						if (UndoCount > 0) {
							UndoCount--;
							UndoIndex = (UndoIndex - 1 + UndoSize) % UndoSize;
							CopyLevel(current,undo[UndoIndex]);
							VexedPreferences.LevelPack[0].MoveCountMade = undoMovesMade[UndoIndex];
							RedrawLevel();
							if (UndoCount == 0) {
								// clear the undo icon
								r.topLeft.x = LocUndo;
								r.topLeft.y = BottomRow;
								r.extent.x = BlockSize;
								r.extent.y = BlockSize;
								WinEraseRectangle(&r, 0);
							}
							// decrement the replay count
							ReplayCurrent--;
							clearMovePending();                 // clear flag, icon
						}
						handled = 1;
						break;
					//}}}
					//{{{ MemorizeButton - Memorize icon			.
					case MemorizeButton:
						CopyLevel(stored,current);
						MoveCountMadeStored = VexedPreferences.LevelPack[0].MoveCountMade;
						MoveCountParStored = VexedPreferences.LevelPack[0].MoveCountPar;

						// display the recall icon
						DrawBM(BitmapRecall, LocRecall, BottomRow, true);  // load the recall icon
						memorized = true;

						handled = 1;
						break;
					//}}}

					//{{{ RecallButton - Recall icon					.
					case RecallButton:
						if (stored[0][0] != 0) {
							CopyLevel(current,stored);
							VexedPreferences.LevelPack[0].MoveCountMade = MoveCountMadeStored;
							VexedPreferences.LevelPack[0].MoveCountPar = MoveCountParStored;
							DrawLevel( true );
							clearMovePending();           // clear flag, icon
						}
						handled = 1;
						break;
					//}}}

					//{{{ RestartButton - Restart level icon		.
					case RestartButton:
						if( VexedPreferences.LevelPack[0].MoveCountMade > 0 ) {
							if( FrmAlert( AlertConfirmLevelRestart ) == 0) {
								LoadLevel(NULL, VexedPreferences.LevelPack[0].CurrentLevel, false);
								ReplayCurrent = 0;
								clearMovePending();				// clear flag, icon
								}
							}
						handled = 1;
						break;
					//}}}
				}
			}
		}
	break;
//}}}
//{{{ penUpEvent				.
   case penUpEvent:

      // check to see if block check mode is now over
      if (InBlockCheck) {
         // end block check mode
         EvtFlushKeyQueue();           // flush key queue
         EvtFlushPenQueue();
         RedrawLevel();
         InBlockCheck = false;               // cleared for next time
         handled = 1;
         break;
      }

      EvtGetPen(&penx, &peny, &bstate);
      if (peny < TopRow) {                   // title bar area?
         break;                              // return to OS
      }

      if (peny < BottomRow) {                      // penUpEvent is only an issue in the gameplay area
         playermove.end.x = (Int16) (penx / BlockSize);
         playermove.end.y = (Int16) ((peny / BlockSize) - 1);

         // if we're lifting stylus from same block
         if (playermove.begin.y == playermove.end.y && playermove.begin.x == playermove.end.x) {
            if (movePending) {               // end of move?
//               if (current[playermove.begin.y][playermove.begin.x] == Air &&  // only move to air spot
//                   playermove.begin.y == playermove.pending.y) {  // only move if same row
               if (current[playermove.begin.y][playermove.begin.x] == Air) { // only move to air spot
                  if (playermove.end.x > playermove.pending.x) {
                     dist = (Int8) (playermove.end.x - playermove.pending.x);
                     if (UndoCount < UndoSize) {
                        UndoCount++;
                     }
                     CopyLevel(undo[UndoIndex],current);
                     undoMovesMade[UndoIndex] = VexedPreferences.LevelPack[0].MoveCountMade;
                     UndoIndex = (UndoIndex + 1) % UndoSize;
                     if (PlayerMakeMove2(playermove.pending.x, playermove.pending.y, Right, dist, true)) {
                        clearMovePending();  // clear flag, icon
                        NextLevel();
                        handled = 1;
                        break;
                     }
                  } else if (playermove.end.x < playermove.pending.x) {
                     dist = (Int8) (playermove.pending.x - playermove.end.x);
                     if (UndoCount < UndoSize) {
                        UndoCount++;
                     }
                     CopyLevel(undo[UndoIndex],current);
                     undoMovesMade[UndoIndex] = VexedPreferences.LevelPack[0].MoveCountMade;
                     UndoIndex = (UndoIndex + 1) % UndoSize;
                     if (PlayerMakeMove2(playermove.pending.x, playermove.pending.y, Left, dist, true)) {
                        clearMovePending();  // clear flag, icon
                        NextLevel();
                        handled = 1;
                        break;
                     }
                  }
               }
               clearMovePending();           // clear flag, icon
            }
            else {                           // start of move
               // only start move if on a valid block
               if (current[playermove.end.y][playermove.end.x] != Air && current[playermove.end.y][playermove.end.x] != Wall) {
                  movePending = true;
                  playermove.pending.y = playermove.end.y;  // save x,y of start of move
                  playermove.pending.x = playermove.end.x;
                  DrawBM(BitmapPending, LocPending, BottomRow, true);  // load the pending icon
               }
            }
            handled = 1;
            break;
         }
         if (playermove.begin.y == playermove.end.y &&
             current[playermove.begin.y][playermove.begin.x] != Air &&
             current[playermove.begin.y][playermove.begin.x] != Wall &&
             current[playermove.end.y][playermove.end.x] == Air) {  // destination must be open space, not a wall
            if (playermove.end.x > playermove.begin.x) {
               dist = (Int8) (playermove.end.x - playermove.begin.x);
               if (UndoCount < UndoSize) {
                  UndoCount++;
               }
               CopyLevel(undo[UndoIndex],current);
               undoMovesMade[UndoIndex] = VexedPreferences.LevelPack[0].MoveCountMade;
               UndoIndex = (UndoIndex + 1) % UndoSize;
               if (PlayerMakeMove2(playermove.begin.x, playermove.begin.y, Right, dist, true)) {
                  clearMovePending();           // clear flag, icon
                  NextLevel();
                  handled = 1;
                  break;
               }
            } else if (playermove.end.x < playermove.begin.x) {
               dist = (Int8) (playermove.begin.x - playermove.end.x);
               if (UndoCount < UndoSize) {
                  UndoCount++;
               }
               CopyLevel(undo[UndoIndex],current);
               undoMovesMade[UndoIndex] = VexedPreferences.LevelPack[0].MoveCountMade;
               UndoIndex = (UndoIndex + 1) % UndoSize;
               if (PlayerMakeMove2(playermove.begin.x, playermove.begin.y, Left, dist, true)) {
                  clearMovePending();           // clear flag, icon
                  NextLevel();
                  handled = 1;
                  break;
               }
            }
            clearMovePending();
         }
      } else {
			controlButtonPressed = false;
		}
		handled = 1;
      break;
//}}}
//{{{ menuEvent				.
	case menuEvent:
      switch (event->data.menu.itemID) {

		case ItemAdjustBrightness:
			if (romVersion >= 0x03103000 )
				UIBrightnessAdjust();
			else
				FrmAlert( AlertNoBrightnessAdjust );
			handled = 1;
			break;

		case ItemCheckForUpdate:
			{
			Int16 numSupportedBrowsers;

			WBM_Init( &numSupportedBrowsers );

			if( numSupportedBrowsers > 0 ) {
				Boolean DoBrowseTheWeb = true;

				if( VexedPreferences.LevelPack[0].MoveCountMade > 0 ) {
					DoBrowseTheWeb = ( FrmAlert( AlertConfirmWebBrowsing ) == 0 );
				}
				
				if( DoBrowseTheWeb ) {
					Char URL[80];
					StrPrintF( URL, "http://vexed.sf.net/version.php?version=%d&revision=%d&release=%d", VEXED_VERSION, VEXED_REVISION, VEXED_RELEASE );
					if( WBM_OpenWebPage( 0, URL ) == wbmErrNoValidBrowserFound ) {
						FrmAlert( AlertNoWebBrowserFound );
					}
				}
			} else {
				FrmAlert( AlertNoWebBrowserFound );
			}

			WBM_End();
			handled = 1;
			}
			break;

      case ItemAbout:
         FrmPopupForm(FormAbout);
         handled = 1;
         break;

		case ItemBeamVexed:
			BeamDatabase(cardNo, appID, "Vexed.prc", "Vexed!");
			break;

      case ItemSolution:
			// First try to get the solution. Perhaps there isn't one
			if (!GetLevelProperty(VexedPreferences.LevelPack[0].CurrentLevel, "solution", stringSolution, sizeof(stringSolution))) {
				FrmAlert(AlertNoSolution);
				handled = 1;
				break;
			}
         // If this level has been played already with a score < par, then it's ok to show the solution
         // without a penalty

         {
            Boolean fPenalize = true;
            LevelPackPrefs *pprefs = &VexedPreferences.LevelPack[0];
            // Have we already solved this level?
            if (pprefs->SolvedLevels >= pprefs->CurrentLevel) {
               // Is the score less than par? If so, don't penalize for solution
               if (pprefs->Score[pprefs->CurrentLevel].MoveCountMade <= pprefs->Score[pprefs->CurrentLevel].MoveCountPar)
                  fPenalize = false;
            }

            if (fPenalize) {
			      // Really bring up solution?
			      if (FrmAlert(AlertShowSolution) != 0) {
				      handled = 1;
				      break;
			      }
			      // Add 5 to BaseMoveCountMade so that redoing the level doesn't
			      // "forgive" these 5 points
			      VexedPreferences.LevelPack[0].BaseMoveCountMade += 5;
            }
         }

         FrmPopupForm(FormSolution);
         handled = 1;
         break;

      case ItemPreferences:
         FrmPopupForm(FormPrefs);
         handled = 1;
         break;

      case ItemHowToPlay:
         FrmHelp(StringInfo);
         handled = 1;
         break;

         // clear levels back to 0
      case ItemClear:
         if (!FrmAlert(AlertResetYesNo)) {
            VexedPreferences.LevelPack[0].SolvedLevels = -1;
            VexedPreferences.LevelPack[0].CurrentLevel = 0;
				VexedPreferences.LevelPack[0].BaseMoveCountMade = 0;
            LoadLevel(NULL, 0, false);
         }
         handled = 1;
         break;

      case ItemLevelPacks:
         DoLevelPackForm(false);
         handled = 1;
         break;

         // Level Info
      case ItemInfo:
         FrmPopupForm(FormInfo);
         handled = 1;
         break;

         // Replay current level
      case ItemReplayCurrent:
         if (!ReplayCurrent) {               // Current level replay data empty?
            FrmAlert(AlertNoReplayCurrent);
         } else {
            ReplayLevel(ReplayCIndex);       // Replay current level
         }
         handled = 1;
         break;

         // Replay previous level
      case ItemReplayPrevious:
         if (VexedPreferences.LevelPack[0].CurrentLevel == 0) { // on the first level
            FrmAlert(AlertNoPreviousLevel);
         } else if (!ReplayPrevious) {       // Previous level replay data empty?
            FrmAlert(AlertNoReplayPrevious);
         } else {
            ReplayLevel(ReplayPIndex);       // Replay previous level
         }
         handled = 1;
         break;
      }
//}}}
//{{{ keyDownEvent			.
   // check for Graffiti character, page up key
   case keyDownEvent:
      if (event->data.keyDown.chr == 'c') {
         cheatOn = true;
         CheatCount = 0;
         handled = 1;
      }
      else if (event->data.keyDown.chr == pageUpChr) {   // page up key pressed?
         // find out if the pen is down and in a valid block
         // only do this on initial key press, not autorepeat ones
         if (!InBlockCheck && !(event->data.keyDown.modifiers & autoRepeatKeyMask)) {
            EvtGetPen(&penx, &peny, &bstate);
            if (bstate) {                          // if pen is down
               if ((peny >= TopRow) && (peny < BottomRow)) {     // in the game board?
                  playermove.begin.x = (Int16) (penx / BlockSize);
                  playermove.begin.y = (Int16) ((peny / BlockSize) - 1);

                  // first determine if in a valid block
                  if ((current[playermove.begin.y][playermove.begin.x] != Air ) &&
                     (current[playermove.begin.y][playermove.begin.x] != Wall)) {
                     BlockCheck(playermove.begin.x, playermove.begin.y);
                  }
               }
            }
         }
         handled = 1;
      }
      break;
//}}}
//{{{ nilEvent					.
	case nilEvent:
      handled = 1;
      break;
//}}}
  default:
     break;
   }


	if (refreshIcons) {
      RedrawLevel();                         // redraw the screen
      DrawControlIcons();
      if (memorized) {                       // anything memorized?
         DrawBM(BitmapRecall, LocRecall, BottomRow, true);// load the recall icon
      }
      if (movePending) {                      // move pending?
         DrawBM(BitmapPending, LocPending, BottomRow, true);// load the pending icon
      }
      if (UndoCount > 0) {
         DrawBM(BitmapUndo, LocUndo, BottomRow, true);
      }
//      UpdateBlocks();                        // update blocks, too
      refreshIcons = false;
   }
   return handled;
}
//}}}
//{{{ frmAboutEventH -----------------------------------------------------------------------------
static Boolean frmAboutEventH(EventPtr event) {
   FormPtr           form;
   Int16             handled = 0;

   switch (event->eType) {
   case frmUpdateEvent:                      // fix draw of screen after modal dialogs
      form = FrmGetActiveForm();
      FrmDrawForm(form);
      WriteAboutInfo();
      handled = 1;
      break;
   case frmOpenEvent:
      form = FrmGetActiveForm();
      FrmDrawForm(form);
      WriteAboutInfo();
      handled = 1;
      break;
   case ctlSelectEvent:
      if (event->data.ctlEnter.controlID == ButtonOK) {
         FrmReturnToForm(0);
         handled = 1;
         break;
      }
      else if (event->data.ctlEnter.controlID == ButtonCredits) {
         FrmHelp(StringCredits);
         handled = 1;
         break;
      }
     break;
   default:
     break;
   }
   return handled;
}
//}}}
//{{{ frmInfoEventH ------------------------------------------------------------------------------
static Boolean frmInfoEventH(EventPtr event) {
   FormPtr           form;
   Int16             handled = 0;
   Int16             BlockCount[16];
   Int8              BlockCountStr[10][3];
   Int8              x,y,i;

   switch (event->eType) {
   case frmOpenEvent:
      form = FrmGetActiveForm();
      for (i=0;i < 16; i++) {
         BlockCount[i] = 0;
      }
      for (y = 0; y < sizeY; y ++) {
         for (x = 0; x < sizeX; x ++) {
            BlockCount[current[y][x]]++;     // increment count
         }
      }
      for (i=0; i<10; i++) {
			if( BlockCount[i] > 0 )
         	StrPrintF((char *)BlockCountStr[i], "%i", BlockCount[i]);
			else
				StrCopy((char *)BlockCountStr[i], "");
      }


      FrmCopyLabel (form, LabelBlock1Count, (char *)BlockCountStr[1]);
      FrmCopyLabel (form, LabelBlock2Count, (char *)BlockCountStr[2]);
      FrmCopyLabel (form, LabelBlock3Count, (char *)BlockCountStr[3]);
      FrmCopyLabel (form, LabelBlock4Count, (char *)BlockCountStr[4]);
      FrmCopyLabel (form, LabelBlock5Count, (char *)BlockCountStr[5]);
      FrmCopyLabel (form, LabelBlock6Count, (char *)BlockCountStr[6]);
      FrmCopyLabel (form, LabelBlock7Count, (char *)BlockCountStr[7]);
      FrmCopyLabel (form, LabelBlock8Count, (char *)BlockCountStr[8]);

      FrmDrawForm(form);

      // now need to draw the blocks from memory to support multiple block sets
		i=1;
		if( BlockCount[i++] > 0 )
     		DrawBM(0, 10, 20, false);
		if( BlockCount[i++] > 0 )
	      DrawBM(1, 10, 36, false);
		if( BlockCount[i++] > 0 )
	      DrawBM(2, 10, 52, false);
		if( BlockCount[i++] > 0 )
   	   DrawBM(3, 10, 68, false);
		if( BlockCount[i++] > 0 )
      	DrawBM(4, 72, 20, false);
		if( BlockCount[i++] > 0 )
     		DrawBM(5, 72, 36, false);
		if( BlockCount[i++] > 0 )
      	DrawBM(6, 72, 52, false);
		if( BlockCount[i++] > 0 )
      	DrawBM(7, 72, 68, false);

      handled = 1;
      break;

   case ctlSelectEvent:
      if (event->data.ctlEnter.controlID == InfoButtonOK) {
         FrmReturnToForm(0);
         handled = 1;
         break;
      }
     break;
   default:
     break;
   }
   return handled;
}
//}}}
//{{{ frmPrefsEventH -----------------------------------------------------------------------------
static Boolean frmPrefsEventH(EventPtr event) {
   FormPtr           form;
   Int16             handled = 0;
   ListType          *list;
   ControlType       *trigger;

   form = FrmGetActiveForm();
   list = FrmGetObjectPtr(form, FrmGetObjectIndex(form, PopupBlockSetList));

   switch (event->eType) {
   case frmOpenEvent:

      // set the check boxes and list to the current preferences state
      trigger = FrmGetObjectPtr(form, FrmGetObjectIndex(form, PopupBlockSet));

      FrmSetControlValue(form, FrmGetObjectIndex(form, CheckBoxPieceAnim), VexedPreferences.PieceMoveAnim);
      FrmSetControlValue(form, FrmGetObjectIndex(form, CheckBoxGravityAnim), VexedPreferences.GravityAnim);
      FrmSetControlValue(form, FrmGetObjectIndex(form, CheckBoxEliminationAnim), VexedPreferences.EliminationAnim);
      FrmSetControlValue(form, FrmGetObjectIndex(form, CheckBoxBlindsEffect), VexedPreferences.BlindsEffect);
      FrmSetControlValue(form, FrmGetObjectIndex(form, CheckBoxSound), VexedPreferences.Sound);
      FrmSetControlValue(form, FrmGetObjectIndex(form, CheckBoxSkipIntro), VexedPreferences.SkipIntro);
      switch (screenMode) {
      case 8:
         LstSetHeight(list, 6);
         break;
      case 4:
         LstSetHeight(list, 6);
         LstSetListChoices(list, GraySListP, 6);   // grayscale list of blocksets
         break;
      default:
         // scale back list for monochrome or unknown devices
         LstSetListChoices(list, MonoListP, 3);
         LstSetHeight(list, 3);
         LstMakeItemVisible(list, 2);
      }

      // set up the block set list
      LstSetSelection(list, VexedPreferences.BlockSet - 1);
      OldSet = VexedPreferences.BlockSet;

      // set up label on the popup trigger
      CtlSetLabel(trigger, LstGetSelectionText(list, VexedPreferences.BlockSet - 1));


      // fall through
   case frmUpdateEvent:
      FrmDrawForm(form);
      // Draw a separator line
      WinDrawLine(6,70,150,70);

      handled = 1;
      break;
   case lstSelectEvent:
      switch (event->data.lstSelect.listID) {
      case PopupBlockSetList:
         // VexedPreferences.BlockSet = LstGetSelection(list) + 1;
         handled = 1;
      }
      break;
   case ctlSelectEvent:
      switch (event->data.ctlEnter.controlID) {
      case ButtonOK2:

         // set the preferences state to the state of the checkboxes on the form
         VexedPreferences.PieceMoveAnim =   FrmGetControlValue(form, FrmGetObjectIndex(form, CheckBoxPieceAnim));
         VexedPreferences.GravityAnim =     FrmGetControlValue(form, FrmGetObjectIndex(form, CheckBoxGravityAnim));
         VexedPreferences.EliminationAnim = FrmGetControlValue(form, FrmGetObjectIndex(form, CheckBoxEliminationAnim));
         VexedPreferences.BlindsEffect =    FrmGetControlValue(form, FrmGetObjectIndex(form, CheckBoxBlindsEffect));
         VexedPreferences.Sound =           FrmGetControlValue(form, FrmGetObjectIndex(form, CheckBoxSound));
         VexedPreferences.SkipIntro =       FrmGetControlValue(form, FrmGetObjectIndex(form, CheckBoxSkipIntro));
         VexedPreferences.BlockSet =        LstGetSelection(list) + 1;

         // if there is a new block set, load the bitmaps
         if (OldSet != VexedPreferences.BlockSet) {
            if (screenMode > 2) {            // not in low res
               if (VexedPreferences.BlockSet > 6) {   // 1 bit mono in 4 bit mode
                  in1Bit = true;
               }
               else {
                  in1Bit = false;
               }
            }
            FreeBitmaps(true);
            LoadBitmaps(true);               // load the new bitmap set
         }

         // leave the preferences form
         refreshIcons = true;
         FrmReturnToForm(0);
         handled = 1;
         break;
      case ButtonCancel:
         FrmReturnToForm(0);
         handled = 1;
         break;
      }
      break;
   default:
      break;
   }
   return handled;
}
//}}}
