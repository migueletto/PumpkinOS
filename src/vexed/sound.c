/*  Vexed - sound.c "Sound specific code for vexed"
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

extern Preferences   VexedPreferences;
extern Boolean       InSolutionBack;         // Doing solution back function?
extern UInt16        GameVolume;             // volume from preferences

void SoundEffect(Int16 effect) {
   SndCommandType    s;

   if (InSolutionBack)
      return;
   if (!VexedPreferences.Sound || GameVolume == 0)
      return;

   s.cmd = sndCmdFreqDurationAmp;
   s.param3 = GameVolume;             		// amplitude
   if (effect == HighBeep) {
      s.param1 = 4000;                    // frequency
      s.param2 = 20;                      // duration (in ms)
      SndDoCmd(0, &s, 0);
   } else if (effect == LowBeep) {
      s.param1 = 150;
      s.param2 = 20;
      SndDoCmd(0, &s, 0);
   } else if (effect == HighSweepLow) {
      s.param2 = 2;
      for (s.param1 = 2000; s.param1 >=1500; s.param1 -= 50)
         SndDoCmd(0, &s, 0);
   }
}