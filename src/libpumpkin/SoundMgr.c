#include <PalmOS.h>
#include <VFSMgr.h>
  
#include "sys.h"
#include "thread.h"
#include "script.h"
#include "pwindow.h"
#include "media.h"
#include "vfs.h"
#include "wav.h"
#include "bytes.h"
#include "ptr.h"
#include "heap.h"
#include "emupalmosinc.h"
#include "pumpkin.h"
#include "debug.h"
#include "xalloc.h"

#define TAG_SOUND  "sound"

#define sndUnityGain 1024

typedef struct {
  audio_provider_t *ap;
  UInt16 alarmAmp;
  UInt16 sysAmp;
  UInt16 defAmp;
} snd_module_t;

extern thread_key_t *snd_key;

typedef struct {
  char *tag;
  SndStreamBufferCallback func;
  SndStreamVariableBufferCallback vfunc;
  void *userdata;
  UInt32 samplesize;
  Int32 volume;
  Int32 pan;
  int pcm, channels, rate;
  Boolean started, stopped;
  audio_t audio;
  Boolean first;
  heap_t *heap;
} SndStreamType;

typedef struct {
  int ptr;
} SndStreamArg;

int SndInitModule(audio_provider_t *ap) {
  snd_module_t *module;

  if ((module = xcalloc(1, sizeof(snd_module_t))) == NULL) {
    return -1;
  }

  module->ap = ap;
  thread_set(snd_key, module);

  return 0;
}

int SndFinishModule(void) {
  snd_module_t *module = (snd_module_t *)thread_get(snd_key);

  if (module) {
    xfree(module);
  }

  return 0;
}

Err SndInit(void) {
  return 0;
}

void SndSetDefaultVolume(UInt16 *alarmAmpP, UInt16 *sysAmpP, UInt16 *defAmpP) {
  snd_module_t *module = (snd_module_t *)thread_get(snd_key);

  if (alarmAmpP) module->alarmAmp = *alarmAmpP;
  if (sysAmpP) module->sysAmp = *sysAmpP;
  if (defAmpP) module->defAmp = *defAmpP;
}

void SndGetDefaultVolume(UInt16 *alarmAmpP, UInt16 *sysAmpP, UInt16 *masterAmpP) {
  snd_module_t *module = (snd_module_t *)thread_get(snd_key);

  if (alarmAmpP) *alarmAmpP = module->alarmAmp;
  if (sysAmpP) *sysAmpP = module->sysAmp;
  if (masterAmpP) *masterAmpP = module->defAmp; // XXX is this correct ?
}

// The Sound Manager only supports one channel
// of sound synthesis: You must pass NULL as the value of channelP.
//   typedef struct SndCommandType {
//    SndCmdIDType cmd;
//    UInt8 reserved;
//    Int32 param1;
//    UInt16 param2;
//    UInt16 param3;
//  } SndCommandType;

Err SndDoCmd(void * /*SndChanPtr*/ channelP, SndCommandPtr cmdP, Boolean noWait) {
  Err err = sndErrBadParam;

  if (channelP == NULL && cmdP != NULL) {
    debug(DEBUG_INFO, "Sound", "SndDoCmd %d", cmdP->cmd);

    switch (cmdP->cmd) {
      case sndCmdFreqDurationAmp:
        // Play a tone. SndDoCmd blocks until the tone has finished.
        // param1 is the tone’s frequency in Hertz.
        // param2 is its duration in milliseconds
        // param3 is its amplitude in the range [0, sndMaxAmp].
        // If the amplitude is 0, the sound isn’t played and the function returns immediately.
        break;
      case sndCmdFrqOn:
        // Initiate a tone. SndDoCmd returns immediately while the tone plays
        // in the background. Subsequent sound playback requests interrupt the tone.
        // param1 is the tone’s frequency in Hertz.
        // param2 is its duration in milliseconds
        // param3 is its amplitude in the range [0, sndMaxAmp].
        // If the amplitude is 0, the sound isn’t played and the function returns immediately.
        break;
      case sndCmdNoteOn:
        // Initiate a MIDI-defined tone. SndDoCmd returns immediately
        // while the tone plays in the background. Subsequent sound
        // playback requests interrupt the tone.
        // param1 is the tone’s pitch given as a MIDI key number in the range [0, 127].
        // param2 is the tone’s duration in milliseconds
        // param3 is its amplitude given as MIDI velocity [0, 127].
        break;
      case sndCmdQuiet:
        // Stop the playback of the currently generated tone.
        // All parameter values are ignored.
        break;
    }
  }

  return err;
}

/*
If you’re playing an alarm (sndAlarm), the user’s alarm volume
preference setting is used. For all other system sounds, the system
volume preference is used.
Alarm sounds (sndAlarm) are played synchronously:
SndPlaySystemSound blocks until the sound has been played.
All other sounds are played asynchronously.
*/
void SndPlaySystemSound(SndSysBeepType beepID) {
  debug(DEBUG_INFO, "Sound", "SndPlaySystemSound %d", beepID);

  switch (beepID) {
    case sndInfo:
    case sndWarning:
    case sndError:
    case sndStartUp:
    case sndAlarm:
    case sndConfirmation:
    case sndClick:
      break;
  }
}

/*
Generates a list of MIDI records. Returns true if records were found, otherwise returns false.
creator: Creator ID of the database in which the function looks for MIDI records. Pass 0 to search all databases.
multipleDBs: Pass true to search multiple databases for MIDI records. Pass false to search only in the first database that meets the search criteria.
recordCount: Returns the number of MIDI records that were found.
recordList: Returns a pointer to an array of SndMidiListItemType structures, one structure for each record that was found.

Structure of records in the MIDI sound database:

50 4D 72 63 0C 00 41 6C 61 72 6D 00 4D 54 68 64
00 00 00 06 00 00 00 01 01 90 4D 54 72 6B 00 00
00 2E 00 90 62 7F 4A 62 00 00 5F 7F 82 28 5F 00
81 14 62 7F 4A 62 00 00 5F 7F 82 28 5F 00 81 14
62 7F 4A 62 00 00 5F 7F 82 28 5F 00 00 FF 2F 00

Each MIDI record consists of a record header followed immediately by the Standard MIDI File (SMF) data stream.
Only SMF format #0 is presently supported. The first byte of the record header is the byte offset from the beginning of the record
to the SMF data stream. The name of the record follows the byte offset field. sndMidiNameLength is the limit on name size (including NULL).

typedef struct SndMidiRecHdrType {
  UInt32  signature;    // set to sndMidiRecSignature 'PMrc' (0x50 0x4D 0x72 0x63)
  UInt8   bDataOffset;  // offset from the beginning of the record to the Standard Midi File data stream (0x0C)
  UInt8   reserved;     // set to zero (0x00)
} SndMidiRecHdrType;

typedef struct SndMidiRecType {
  SndMidiRecHdrType hdr; // offset from the beginning of the record to the Standard Midi File data stream
  Char name[2];          // Track name: 1 or more chars including NULL terminator. If a track has no name, the NULL character must still be provided. (0x41 0x6C 0x61 0x72 0x6D 0x00)
} SndMidiRecType;

Midi records found by SndCreateMidiList:

typedef struct SndMidiListItemType {
  Char     name[sndMidiNameLength]; // including NULL terminator
  UInt32   uniqueRecID;
  LocalID  dbID;
  UInt16   cardNo;
} SndMidiListItemType;
*/

#define MidiItemSize 42

Boolean SndCreateMidiList(UInt32 creator, Boolean multipleDBs, UInt16 *wCountP, MemHandle *entHP) {
  DmSearchStateType stateInfo;
  Boolean newSearch;
  UInt16 cardNo;
  LocalID dbID;
  DmOpenRef dbRef;
  MemHandle h, hrec;
  SndMidiListItemType *list;
  UInt16 numRecs, index, i, max;
  UInt32 type, uniqueID, sig;
  char ct[8];
  UInt8 *p, *plist, *q;

  // File type for Standard MIDI File record databases
  type = sysFileTMidi;
  max = 32;
  i = 0;

  pumpkin_id2s(creator, ct);
  debug(DEBUG_INFO, "Sound", "SndCreateMidiList '%s'", ct);
  if ((h = MemHandleNew(max * sizeof(SndMidiListItemType))) != NULL) {
    if ((list = MemHandleLock(h)) != NULL) {
      plist = (UInt8 *)list;
      for (newSearch = true; i < max; newSearch = false) {
        if (DmGetNextDatabaseByTypeCreator(newSearch, &stateInfo, type, creator, false, &cardNo, &dbID) != errNone) break;
        debug(DEBUG_INFO, "Sound", "SndCreateMidiList searching database 0x%08X", dbID);
        if ((dbRef = DmOpenDatabase(cardNo, dbID, dmModeReadOnly)) != NULL) {
          numRecs = DmNumRecords(dbRef);
          debug(DEBUG_INFO, "Sound", "SndCreateMidiList database 0x%08X has %u records", dbID, numRecs);
          for (index = 0; index < numRecs && i < max; index++) {
            if (DmRecordInfo(dbRef, index, NULL, &uniqueID, NULL) == errNone) {
              debug(DEBUG_INFO, "Sound", "SndCreateMidiList record %u uniqueID 0x%08X", index, uniqueID);
              if ((hrec = DmGetRecord(dbRef, index)) != NULL) {
                if ((p = MemHandleLock(hrec)) != NULL) {
                  get4b(&sig, p, 0);
                  if (sig == sndMidiRecSignature) {
                    debug(DEBUG_INFO, "Sound", "SndCreateMidiList MIDI %d name \"%s\"", i, (char *)&p[6]);
                    if (pumpkin_is_m68k()) {
                      q = (UInt8 *)&plist[i * MidiItemSize];
                      StrNCopy((char *)q, (char *)&p[6], sndMidiNameLength-1);
                      put4b(uniqueID, q, sndMidiNameLength);
                      put4b(dbID,     q, sndMidiNameLength + 4);
                      put2b(cardNo,   q, sndMidiNameLength + 8);
                    } else {
                      StrNCopy(list[i].name, (char *)&p[6], sndMidiNameLength-1);
                      list[i].uniqueRecID = uniqueID;
                      list[i].dbID = dbID;
                      list[i].cardNo = cardNo;
                    }
                    i++;
                  } else {
                    debug(DEBUG_ERROR, "Sound", "SndCreateMidiList wrong signature 0x%08X", sig);
                  }
                  MemHandleUnlock(hrec);
                }
                DmReleaseRecord(dbRef, index, false);
              }
            }
          }
          DmCloseDatabase(dbRef);
        }
        if (!multipleDBs) {
          debug(DEBUG_INFO, "Sound", "SndCreateMidiList searching only first database");
          break;
        }
      }
      MemHandleUnlock(h);
    }
  }

  if (i == 0 && h != NULL) {
    MemHandleFree(h);
    h = NULL;
  }

  if (wCountP) *wCountP = i;
  if (entHP) *entHP = h;
  debug(DEBUG_INFO, "Sound", "SndCreateMidiList %d record(s) in list 0x%08X", i, h ? (uint8_t *)h - emupalmos_ram() : 0);

  return i > 0;
}

Err SndPlaySmf(void *chanP, SndSmfCmdEnum cmd, UInt8 *smfP, SndSmfOptionsType *selP, SndSmfChanRangeType *chanRangeP, SndSmfCallbacksType *callbacksP, Boolean bNoWait) {
  snd_module_t *module = (snd_module_t *)thread_get(snd_key);
  UInt32 i, j, id, len;
  UInt16 volume, mthd[3];
  Err err = sndErrBadParam;

  if (smfP != NULL) {
    debug(DEBUG_INFO, "Sound", "SndPlaySmf cmd %d, noWait %d", cmd, bNoWait);
    if (selP) {
      debug(DEBUG_INFO, "Sound", "SndPlaySmf start %d, end %d, amp %d, int %d", selP->dwStartMilliSec, selP->dwEndMilliSec, selP->amplitude, selP->interruptible);
      if (selP->dwStartMilliSec != 0) {
        debug(DEBUG_ERROR, "Sound", "SndPlaySmf start %d not supported", selP->dwStartMilliSec);
      }
      if (selP->dwEndMilliSec != sndSmfPlayAllMilliSec) {
        debug(DEBUG_ERROR, "Sound", "SndPlaySmf end %d not supported", selP->dwEndMilliSec);
      }
      volume = selP->amplitude;
    } else {
      volume = sndMaxAmp;
    }

    i = get4b(&id, smfP, 0);
    i += get4b(&len, smfP, i);
    if (id == 'MThd' && len == 6) {
      i += get2b(&mthd[0], smfP, i);
      i += get2b(&mthd[1], smfP, i);
      i += get2b(&mthd[2], smfP, i);
      debug(DEBUG_INFO, "Sound", "SndPlaySmf format %d, ntrks %d, division 0x%04X", mthd[0], mthd[1], mthd[2]);

      for (j = 0; j < mthd[1]; j++) {
        i += get4b(&id, smfP, i);
        i += get4b(&len, smfP, i);
        if (id == 'MTrk') {
          debug(DEBUG_INFO, "Sound", "SndPlaySmf track %d length %d", j, len);
        } else {
          debug(DEBUG_INFO, "Sound", "SndPlaySmf unknown id 0x%08X", id);
        }
        i += len;
      }

      if (module->ap && module->ap->mixer_play) {
        debug(DEBUG_INFO, "Sound", "SndPlaySmf playing %d bytes", i);
        if (volume >= sndMaxAmp) {
          volume = 128;
        } else if (volume > 0) {
          volume <<= 1; // PalmOS: 0-64, SDL: 0-128
        }
        module->ap->mixer_play(smfP, i, volume);
      }

    } else {
      debug(DEBUG_ERROR, "Sound", "SndPlaySmf wrong header id 0x%08X or length %d", id, len);
    }

    err = errNone;
  }

  return err;
}

Err SndPlaySmfResource(UInt32 resType, Int16 resID, SystemPreferencesChoice volumeSelector) {
  SndSmfOptionsType options;
  MemHandle h;
  UInt8 *smf;
  Err err = sndErrBadParam;

  if ((h = DmGetResource(resType, resID)) != NULL) {
    if ((smf = MemHandleLock(h)) != NULL) {
      options.dwStartMilliSec = 0;
      options.dwEndMilliSec = sndSmfPlayAllMilliSec;
      options.interruptible = true;
      switch (volumeSelector) {
        case prefSysSoundVolume:
        case prefGameSoundVolume:
        case prefAlarmSoundVolume:
          options.amplitude = PrefGetPreference(volumeSelector);
          break;
        default:
          options.amplitude = PrefGetPreference(prefSysSoundVolume);
          break;
      }
      err = SndPlaySmf(NULL, sndSmfCmdPlay, smf, &options, NULL, NULL, false);
      MemHandleUnlock(h);
    }
    DmReleaseResource(h);
  }

  return err;
}

Err SndPlaySmfResourceIrregardless(UInt32 resType, Int16 resID, SystemPreferencesChoice volumeSelector) {
  SndSmfOptionsType options;
  MemHandle h;
  UInt8 *smf;
  Err err = sndErrBadParam;

  if ((h = DmGetResource(resType, resID)) != NULL) {
    if ((smf = MemHandleLock(h)) != NULL) {
      options.dwStartMilliSec = 0;
      options.dwEndMilliSec = sndSmfPlayAllMilliSec;
      options.interruptible = true;
      switch (volumeSelector) {
        case prefSysSoundVolume:
        case prefGameSoundVolume:
        case prefAlarmSoundVolume:
          options.amplitude = PrefGetPreference(volumeSelector);
          break;
        default:
          options.amplitude = PrefGetPreference(prefSysSoundVolume);
          break;
      }
      SndInterruptSmfIrregardless();
      err = SndPlaySmf(NULL, sndSmfCmdPlay, smf, &options, NULL, NULL, false);
      MemHandleUnlock(h);
    }
    DmReleaseResource(h);
  }

  return err;
}

Err SndInterruptSmfIrregardless(void) {
  snd_module_t *module = (snd_module_t *)thread_get(snd_key);

  if (module->ap && module->ap->mixer_stop) {
    module->ap->mixer_stop();
  }

  return errNone;
}

Err SndStreamDelete(SndStreamRef channel) {
  Err err = sndErrBadParam;

  if (channel > 0) {
    ptr_free(channel, TAG_SOUND);
    err = errNone;
  }

  return err;
}

static float getSample(int pcm, void *buffer, UInt32 i) {
  Int8 *s8;
  UInt8 *u8;
  Int16 *s16;
  Int32 *s32;
  float *flt;
  float sample = 0;

  switch (pcm) {
    case PCM_S8:
      s8 = (Int8 *)buffer;
      sample = s8[i];
      break;
    case PCM_U8:
      u8 = (UInt8 *)buffer;
      sample = u8[i];
      break;
    case PCM_S16:
      s16 = (Int16 *)buffer;
      sample = s16[i];
      break;
    case PCM_S32:
      s32 = (Int32 *)buffer;
      sample = s32[i];
      break;
    case PCM_FLT:
      flt = (float *)buffer;
      sample = flt[i];
      break;
  }

  return sample;
}

void putSample(int pcm, void *buffer, UInt32 i, float sample) {
  Int8 *s8;
  UInt8 *u8;
  Int16 *s16;
  Int32 *s32;
  float *flt;

  switch (pcm) {
    case PCM_S8:
      s8 = (Int8 *)buffer;
      if (sample < -128) sample = -128;
      else if (sample > 127) sample = 127;
      s8[i] = (Int8)sample;
      break;
    case PCM_U8:
      u8 = (UInt8 *)buffer;
      if (sample < 0) sample = 0;
      else if (sample > 255) sample = 255;
      u8[i] = (UInt8)sample;
      break;
    case PCM_S16:
      s16 = (Int16 *)buffer;
      if (sample < -32768) sample = -32768;
      else if (sample > 32767) sample = 32767;
      s16[i] = (Int16)sample;
      break;
    case PCM_S32:
      s32 = (Int32 *)buffer;
      if (sample < INT32_MIN) sample = INT32_MIN;
      else if (sample > INT32_MAX) sample = INT32_MAX;
      s32[i] = (Int32)sample;
      break;
    case PCM_FLT:
      flt = (float *)buffer;
      flt[i] = sample;
      break;
  }
}

static int SndGetAudio(void *buffer, int len, void *data) {
  SndStreamArg *arg = (SndStreamArg *)data;
  SndStreamType *snd;
  UInt32 nsamples, i, j;
  Int32 volume, pan;
  Err err;
  Boolean freeArg = false;
  int pcm, channels;
  float sample, gain, leftGain, rightGain;
  char tname[16];
  int r = -1;

  if (buffer && len > 0) {
    if ((snd = ptr_lock(arg->ptr, TAG_SOUND)) != NULL) {
      thread_get_name(tname, sizeof(tname)-1);
      if (tname[0] == '?') {
        thread_set_name("Audio");
        debug(DEBUG_INFO, "Sound", "pumpkin_sound_init");
        pumpkin_sound_init();
        snd->first = false;
      }

      nsamples = 0;
      volume = sndUnityGain;
      pan = sndPanCenter;
      pcm = channels = 0;

      if (snd->started && !snd->stopped) {
        nsamples = len / snd->samplesize;
        if (snd->func) {
          err = snd->func(snd->userdata, arg->ptr, buffer, nsamples);
        } else {
          err = snd->vfunc(snd->userdata, arg->ptr, buffer, &nsamples);
          len = nsamples * snd->samplesize;
        }
        if (err == errNone) {
          channels = snd->channels;
          pcm = snd->pcm;
          pan = snd->pan;
          volume = snd->volume;
          r = len;
        } else {
          nsamples = 0;
          snd->started = false;
          snd->userdata = NULL;
          freeArg = true;
        }
      } else {
        r = 0;
      }

      ptr_unlock(arg->ptr, TAG_SOUND);

      if (nsamples > 0) {
        if ((volume != sndUnityGain || (pan != sndPanCenter && channels == 2))) {
          if (volume == 0) {
            debug(DEBUG_INFO, "Sound", "%d samples (zero volume)", nsamples);
            sys_memset(buffer, 0, len);
          } else {
            if (channels == 1) {
              gain = (float)volume / (float)sndUnityGain;
              debug(DEBUG_INFO, "Sound", "%d samples (mono gain %.3f)", nsamples, gain);
              for (i = 0, j = 0; i < nsamples; i++) {
                sample = getSample(pcm, buffer, i);
                putSample(pcm, buffer, i, sample * gain);
              }
            } else {
              leftGain = (float)volume / (float)sndUnityGain;
              if (pan > sndPanCenter) {
                leftGain *= (float)(1024 - pan) / 1024.0f;
              }
              rightGain = (float)volume / (float)sndUnityGain;
              if (pan < sndPanCenter) {
                pan = -pan;
                rightGain *= (float)(1024 - pan) / 1024.0f;
              }
              debug(DEBUG_INFO, "Sound", "%d samples (left gain %.3f, right gain %.3f)", nsamples, leftGain, rightGain);
              for (i = 0, j = 0; i < nsamples; i++, j += 2) {
                sample = getSample(pcm, buffer, j);
                putSample(pcm, buffer, j, sample * leftGain);
                sample = getSample(pcm, buffer, j);
                putSample(pcm, buffer, j+1, sample * rightGain);
              }
            }
          }
        } else {
          debug(DEBUG_INFO, "Sound", "%d samples", nsamples);
        }
      }

      if (freeArg) {
        debug(DEBUG_INFO, "Sound", "pumpkin_sound_finish");
        pumpkin_sound_finish();
        xfree(arg);
      }
    }
  }

  return r;
}

Err SndStreamStart(SndStreamRef channel) {
  snd_module_t *module = (snd_module_t *)thread_get(snd_key);
  SndStreamType *snd;
  SndStreamArg *arg;
  Err err = sndErrBadParam;

  if (channel > 0 && (snd = ptr_lock(channel, TAG_SOUND)) != NULL) {
    if (snd->started) {
      snd->stopped = false;
      err = errNone;
    } else {
      if ((arg = xcalloc(1, sizeof(SndStreamArg))) != NULL) {
        snd->first = true;
        snd->heap = heap_get();
        arg->ptr = channel;
        snd->started = true;
        snd->stopped = false;
        if (module->ap->start(snd->audio, SndGetAudio, arg) == 0) {
          err = errNone;
        } else {
          snd->started = false;
          xfree(arg);
        }
      }
    }
    ptr_unlock(channel, TAG_SOUND);
  }

  return err;
}

// Currently, SndStreamPause simply calls SndStreamStop() (if pause is true) or
// SndStreamStart() (if pause is false).
// You can’t nest pauses; a single resume request is effective,
// regardless of the number of times the stream has been told to pause.

Err SndStreamPause(SndStreamRef channel, Boolean pause) {
  Err err;

  if (pause) {
    err = SndStreamStop(channel);
  } else {
    err = SndStreamStart(channel);
  }

  return err;
}

Err SndStreamStop(SndStreamRef channel) {
  SndStreamType *snd;
  Err err = sndErrBadParam;

  if (channel > 0 && (snd = ptr_lock(channel, TAG_SOUND)) != NULL) {
    snd->stopped = true;
    ptr_unlock(channel, TAG_SOUND);
    err = errNone;
  }

  return err;
}

// volume: amplitude scalar in the range [0, 32k].
// Values less than 0 are converted to 1024 (unity gain).
// The volume value is applied as an amplitude scalar on the samples
// that this stream’s callback function produces. The scalar is in the
// range [0, 32k], where 1024 is unity gain (i.e. the samples are
// multiplied by 1.0). The mapping of volume to a scalar is linear; thus
// a volume of 512 scales the samples by ~.5, 2048 scales by ~2.0, and
// so on.
// To specify a user preference volume setting, supply one of the
// constants documented under “Volume Constants” on page 957.
// These values are guaranteed to be less than unity gain.
// If the stream is stereo, both channels are scaled by the same
// amplitude scalar.

Err SndStreamSetVolume(SndStreamRef channel, Int32 volume) {
  SndStreamType *snd;
  Err err = sndErrBadParam;

  if (channel > 0 && (snd = ptr_lock(channel, TAG_SOUND)) != NULL) {
    switch (volume) {
      case sndSystemVolume:
        volume = PrefGetPreference(prefSysSoundVolume);
        break;
      case sndGameVolume:
        volume = PrefGetPreference(prefGameSoundVolume);
        break;
      case sndAlarmVolume:
        volume = PrefGetPreference(prefAlarmSoundVolume);
        break;
    }

    if (volume < 0) volume = sndUnityGain;
    else if (volume > 32768) volume = 32768;

    snd->volume = volume;
    ptr_unlock(channel, TAG_SOUND);
    err = errNone;
  }

  return err;
}

Err SndStreamGetVolume(SndStreamRef channel, Int32 *volume) {
  SndStreamType *snd;
  Err err = sndErrBadParam;

  if (channel > 0 && volume && (snd = ptr_lock(channel, TAG_SOUND)) != NULL) {
    *volume = snd->volume;
    ptr_unlock(channel, TAG_SOUND);
    err = errNone;
  }

  return err;
}

typedef struct {
  UInt8 *buffer;
  UInt32 size;
  UInt32 pos;
  UInt32 sampleSize;
  Boolean finished;
} snd_res_t;

static Err SndPlayResourceCallback(void *userdata, SndStreamRef sound, void *buffer, UInt32 numberofframes) {
  snd_res_t *param = (snd_res_t *)userdata;
  UInt32 n;
  Err err = sndErrBadParam;

  if (param && buffer) {
    n = numberofframes;
    if (n > (param->size - param->pos)) n = param->size - param->pos;
    if (n > 0) {
      sys_memcpy(buffer, &param->buffer[param->pos], n * param->sampleSize);
      param->pos += n;
      err = errNone;
    }
  }

  if (err != errNone) {
    param->finished = true;
  }

  return err;
}

Err SndPlayResource(SndPtr sndP, Int32 volume, UInt32 flags) {
  SndStreamRef sound;
  SndStreamWidth width;
  SndSampleType type;
  UInt32 rate;
  snd_res_t param;
  Err err = sndErrBadParam;

  if (sndP && flags == sndFlagSync) {
    if (WavBufferHeader((UInt8 *)sndP, &rate, &type, &width)) {
      param.size = MemPtrSize(sndP);

      if (param.size > 0) {
        param.finished = false;
        param.buffer = sndP;
        param.pos = 0;
        param.sampleSize = 1;
        if (width == sndStereo) param.sampleSize <<= 1;
        if (type == sndInt16) param.sampleSize <<= 1;
        else if (type == sndInt32 || type == sndFloat) param.sampleSize <<= 2;
        param.size /= param.sampleSize;

        if (SndStreamCreate(&sound, sndOutput, rate, type, width, SndPlayResourceCallback, &param, 0, false) == errNone) {
          SndStreamSetVolume(sound, volume);
          if (SndStreamStart(sound) == errNone) {
            for (; !param.finished && !thread_must_end();) {
              SysTaskDelay(5);
            }
          } else {
            SndStreamDelete(sound);
          }
        }
      }
    } else {
      err = sndErrFormat;
    }
  }

  return err;
}

// Pan value in the range [-1024 (full left), 1024 (full right)].
// Center balance is 0.
// The pan value is used as a scalar on a channel’s volume such that a
// channel increases from 0 (inaudible) to full volume as the pan value
// moves from an extreme to 0.

Err SndStreamSetPan(SndStreamRef channel, Int32 panposition) {
  SndStreamType *snd;
  Err err = sndErrBadParam;

  if (channel > 0 && (snd = ptr_lock(channel, TAG_SOUND)) != NULL) {
    if (panposition < sndPanFullLeft) panposition = sndPanFullLeft;
    else if (panposition > sndPanFullRight) panposition = sndPanFullRight;

    snd->pan = panposition;
    ptr_unlock(channel, TAG_SOUND);
    err = errNone;
  }

  return err;
}

Err SndStreamGetPan(SndStreamRef channel, Int32 *panposition) {
  SndStreamType *snd;
  Err err = sndErrBadParam;

  if (channel > 0 && panposition && (snd = ptr_lock(channel, TAG_SOUND)) != NULL) {
    *panposition = snd->pan;
    ptr_unlock(channel, TAG_SOUND);
    err = errNone;
  }

  return err;
}

Err SndStreamDeviceControl(SndStreamRef channel, Int32 cmd, void* param, Int32 size) {
  // marked as system use only
  return sndErrBadParam;
}

void SndStreamRefDestructor(void *p) {
  snd_module_t *module = (snd_module_t *)thread_get(snd_key);
  SndStreamType *snd = (SndStreamType *)p;

  if (snd) {
    if (snd->audio) {
      module->ap->destroy(snd->audio);
    }
    xfree(snd);
  }
}

static Err SndStreamCreateInternal(
  SndStreamRef *channel,        /* channel-ID is stored here */
  SndStreamMode mode,           /* input/output, enum */
  SndFormatType format,         /* enum, e.g., sndFormatMP3 */
  UInt32 samplerate,            /* in frames/second, e.g. 44100, 48000 */
  SndSampleType type,           /* enum, e.g. sndInt16 */
  SndStreamWidth width,         /* enum, e.g. sndMono */
  SndStreamBufferCallback func, /* function that gets called to fill a buffer */
  SndStreamVariableBufferCallback vfunc, /* function that gets called to fill a buffer */
  void *userdata,               /* gets passed in to the above function */
  UInt32 buffsize,              /* preferred buffersize in frames, not guaranteed, use 0 for default */
  Boolean armNative)            /* true if callback is arm native */ {

  snd_module_t *module = (snd_module_t *)thread_get(snd_key);
  SndStreamType *snd;
  int ptr, pcm, samplesize;
  Err err = sndErrBadParam;

  if (module->ap && channel && format == sndFormatPCM && (func || vfunc) && mode == sndOutput) {
    switch (type) {
      case sndInt8:
        pcm = PCM_S8;
        samplesize = 1;
        break;
      case sndUInt8:
        pcm = PCM_U8;
        samplesize = 1;
        break;
      case sndInt16:
        pcm = PCM_S16;
        samplesize = 2;
        break;
      case sndInt32:
        pcm = PCM_S32;
        samplesize = 4;
        break;
      case sndFloat:
        pcm = PCM_FLT;
        samplesize = 4;
        break;
      default:
        debug(DEBUG_ERROR, "Sound", "SndStreamCreate unsupported SndSampleType %d", type);
        pcm = -1;
        break;
    }

    if (pcm != -1) {
      if ((snd = xcalloc(1, sizeof(SndStreamType))) != NULL) {
        snd->tag = TAG_SOUND;
        snd->func = func;
        snd->vfunc = vfunc;
        snd->userdata = userdata;
        snd->pcm = pcm;
        snd->rate = samplerate;
        if (width == sndMono) {
          snd->channels = 1;
        } else {
          snd->channels = 2;
          samplesize *= 2;
        }
        snd->samplesize = samplesize;
        debug(DEBUG_INFO, "Sound", "SndStreamCreate sample rate %d", snd->rate);
        debug(DEBUG_INFO, "Sound", "SndStreamCreate sample size %d", snd->samplesize);

        snd->volume = sndUnityGain;
        snd->pan = sndPanCenter;

        if ((snd->audio = module->ap->create(snd->pcm, snd->channels, snd->rate, module->ap->data)) != -1) {
          if ((ptr = ptr_new(snd, SndStreamRefDestructor)) != -1) {
            *channel = ptr;
            err = errNone;
          } else {
            module->ap->destroy(snd->audio);
            xfree(snd);
          }
        } else {
          xfree(snd);
        }
      }
    }
  } else {
    debug(DEBUG_ERROR, "Sound", "SndStreamCreate invalid arguments");
  }

  return err;
}

Err SndStreamCreate(
  SndStreamRef *channel,        /* channel-ID is stored here */
  SndStreamMode mode,           /* input/output, enum */
  UInt32 samplerate,            /* in frames/second, e.g. 44100, 48000 */
  SndSampleType type,           /* enum, e.g. sndInt16 */
  SndStreamWidth width,         /* enum, e.g. sndMono */
  SndStreamBufferCallback func, /* function that gets called to fill a buffer */
  void *userdata,               /* gets passed in to the above function */
  UInt32 buffsize,              /* preferred buffersize in frames, not guaranteed, use 0 for default */
  Boolean armNative)            /* true if callback is arm native */ {

  return SndStreamCreateInternal(channel, mode, sndFormatPCM, samplerate, type, width, func, NULL, userdata, buffsize, armNative);
}

Err SndStreamCreateExtended(
  SndStreamRef *channel,        /* channel-ID is stored here */
  SndStreamMode mode,           /* input/output, enum */
  SndFormatType format,         /* enum, e.g., sndFormatMP3 */
  UInt32 samplerate,            /* in frames/second, e.g. 44100, 48000 */
  SndSampleType type,           /* enum, e.g. sndInt16, if applicable, or 0 otherwise */
  SndStreamWidth width,         /* enum, e.g. sndMono */
  SndStreamVariableBufferCallback func, /* function that gets called to fill a buffer */
  void *userdata,               /* gets passed in to the above function */
  UInt32 buffsize,              /* preferred buffersize, use 0 for default */
  Boolean armNative) {

  return SndStreamCreateInternal(channel, mode, format, samplerate, type, width, NULL, func, userdata, buffsize, armNative);
}
