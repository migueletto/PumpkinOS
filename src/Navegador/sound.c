#include <PalmOS.h>
  
#include "sound.h"

void PlaySound(UInt16 id)
{
  SndCommandType snd;

  switch (id) {
    case SOUND_BEEP:
      snd.cmd = sndCmdFreqDurationAmp;
      snd.param1 = 800; // Hz
      snd.param2 = 5;   // ms
      snd.param3 = sndDefaultAmp;
      SndDoCmd(NULL, &snd, true);
      break;
    case SOUND_WARNING:
      snd.cmd = sndCmdFreqDurationAmp;
      snd.param1 = 500; // Hz
      snd.param2 = 225; // ms
      snd.param3 = sndDefaultAmp;
      SndDoCmd(NULL, &snd, true);
      snd.param1 = 200; // Hz
      snd.param2 = 225; // ms
      SndDoCmd(NULL, &snd, true);
      break;
    case SOUND_ALARM:
      snd.cmd = sndCmdFreqDurationAmp;
      snd.param1 = 200; // Hz
      snd.param2 = 150;   // ms
      snd.param3 = sndDefaultAmp;
      SndDoCmd(NULL, &snd, true);
      snd.param1 = 400; // Hz
      SndDoCmd(NULL, &snd, true);
      snd.param1 = 600; // Hz
      SndDoCmd(NULL, &snd, true);
  }
}
