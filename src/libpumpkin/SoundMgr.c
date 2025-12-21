#include <PalmOS.h>
#include <VFSMgr.h>
  
#include "sys.h"
#include "thread.h"
#include "mutex.h"
#include "script.h"
#include "pwindow.h"
#include "media.h"
#include "vfs.h"
#include "wav.h"
#include "bytes.h"
#include "ptr.h"
#include "logtrap.h"
#include "emupalmosinc.h"
#include "pumpkin.h"
#include "debug.h"
#include "xalloc.h"

#define TAG_SOUND  "sound"

#define sndUnityGain 1024

#define DOCMD_SAMPLE_RATE 22050

typedef struct {
  audio_provider_t *ap;
  UInt16 alarmAmp;
  UInt16 sysAmp;
  UInt16 defAmp;
  UInt16 midiNoteFreqency[128];
  Int32 lastFrequency;
  SndStreamRef lastChannel;
} snd_module_t;

typedef struct {
  char *tag;
  SndStreamBufferCallback func;
  SndStreamVariableBufferCallback vfunc;
  void *userdata;
  UInt32 func68k;
  UInt32 vfunc68k;
  UInt32 userdata68k;
  UInt32 funcArm;
  UInt32 vfuncArm;
  UInt32 userdataArm;
  uint8_t *buffer;
  UInt32 samplesize;
  Int32 volume;
  Int32 pan;
  int pcm, channels, rate;
  Boolean started, stopped;
  audio_t audio;
  Boolean first;
  Boolean m68k;
  audio_provider_t *ap;
} SndStreamType;

typedef struct {
  int ptr;
  int client;
} SndStreamArg;

typedef struct {
  SndStreamRef channel;
  FileRef f;
  UInt8 *buffer;
  UInt32 size;
  UInt32 pos;
  UInt32 sampleSize;
  Boolean async;
  Boolean finished;
} snd_param_t;

typedef struct {
  uint32_t id;
  void *buffer;
  int len;
  void *data;
} msg_audio_t; 

int SndInitModule(audio_provider_t *ap) {
  snd_module_t *module;
  int i;

  if ((module = sys_calloc(1, sizeof(snd_module_t))) == NULL) {
    return -1;
  }

  for (i = 0; i < 128; i++) {
    module->midiNoteFreqency[i] = (UInt16)(440.0 * sys_pow(2.0, (i - 69.0) / 12.0));
  }

  module->ap = ap;
  pumpkin_set_local_storage(snd_key, module);

  return 0;
}

int SndFinishModule(void) {
  snd_module_t *module = (snd_module_t *)pumpkin_get_local_storage(snd_key);

  if (module) {
    if (module->lastChannel) {
      SndStreamDelete(module->lastChannel);
    }
    sys_free(module);
  }

  return 0;
}

Err SndInit(void) {
  return 0;
}

void SndSetDefaultVolume(UInt16 *alarmAmpP, UInt16 *sysAmpP, UInt16 *defAmpP) {
  snd_module_t *module = (snd_module_t *)pumpkin_get_local_storage(snd_key);

  if (alarmAmpP) module->alarmAmp = *alarmAmpP;
  if (sysAmpP) module->sysAmp = *sysAmpP;
  if (defAmpP) module->defAmp = *defAmpP;
}

void SndGetDefaultVolume(UInt16 *alarmAmpP, UInt16 *sysAmpP, UInt16 *masterAmpP) {
  snd_module_t *module = (snd_module_t *)pumpkin_get_local_storage(snd_key);

  if (alarmAmpP) *alarmAmpP = module->alarmAmp;
  if (sysAmpP) *sysAmpP = module->sysAmp;
  if (masterAmpP) *masterAmpP = module->defAmp; // XXX is this correct ?
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
  debug(DEBUG_INFO, "Sound", "SndCreateMidiList %d record(s) in list 0x%08X", i, (UInt32)(h ? (uint8_t *)h - emupalmos_ram() : 0));

  return i > 0;
}

Err SndPlaySmf(void *chanP, SndSmfCmdEnum cmd, UInt8 *smfP, SndSmfOptionsType *selP, SndSmfChanRangeType *chanRangeP, SndSmfCallbacksType *callbacksP, Boolean bNoWait) {
  snd_module_t *module = (snd_module_t *)pumpkin_get_local_storage(snd_key);
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

Err SndPlaySmfIrregardless (void *channelP, SndSmfCmdEnum command, UInt8 *midiDataP, SndSmfOptionsType *optionsP, SndSmfChanRangeType *channelRangeP, SndSmfCallbacksType *callbacksP, Boolean noWait) {
  // XXX
  return SndPlaySmf(channelP, command, midiDataP, optionsP, channelRangeP, callbacksP, noWait);
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
  snd_module_t *module = (snd_module_t *)pumpkin_get_local_storage(snd_key);

  if (module->ap && module->ap->mixer_stop) {
    module->ap->mixer_stop();
  }

  return errNone;
}

Err SndStreamDelete(SndStreamRef channel) {
  Err err = sndErrBadParam;

  pumpkin_audio_check(0);

  debug(DEBUG_TRACE, "Sound", "SndStreamDelete(%d)", channel);
  if (channel > 0) {
    ptr_free(channel, TAG_SOUND);
    err = errNone;
  }

  return err;
}

static Int32 getSample(int pcm, uint8_t *buffer, UInt32 i) {
  UInt8 u8;
  Int16 s16;
  Int32 sample = 0;

  switch (pcm) {
    case PCM_U8:
      u8 = buffer[i];
      sample = (u8 >= 128) ? (u8 & 0x7F) << 24 : (u8 << 24) | 0x80000000;
      break;
    case PCM_S16:
      s16 = ((int16_t *)buffer)[i];
      sample = s16 << 16;
      break;
    case PCM_S32:
      sample = ((int32_t *)buffer)[i];
      break;
  }

  return sample;
}

void putSample(int pcm, uint8_t *buffer, UInt32 i, Int32 sample) {
  switch (pcm) {
    case PCM_U8:
      buffer[i] = sample >= 0 ? (sample >> 24) | 0x80 : (sample >> 24) & 0x7F;
      break;
    case PCM_S16:
      ((int16_t *)buffer)[i] = sample >> 16;
      break;
    case PCM_S32:
      ((int32_t *)buffer)[i] = sample;
      break;
  }
}

#define SAVE_PREFIX(name) \
static int count = 0; \
static int fd = 0; \
if (count == 0) \
fd = sys_create(name, SYS_TRUNC|SYS_WRITE, 0644)

#define SAVE_SUFFIX(buffer, len) \
debug(1, "XXX", "audio buffer %p", buffer); \
count++; \
if (count < 64) { \
sys_write(fd, buffer, len); \
} \
if (count == 64) { \
sys_close(fd); \
}

static int SndGetAudio(void *buffer, int len, void *data) {
  SndStreamArg *arg = (SndStreamArg *)data;
  SndStreamType *snd;
  UInt32 nbytes, nsamples, i, j;
  Int32 volume, pan;
  Err err;
  Boolean inited = false;
  Boolean freeArg = false;
  Boolean freeBuffer = false;
  uint8_t *ram;
  int pcm, channels;
  float sample, gain, leftGain, rightGain;
  char tname[16];
  int r = -1;

  thread_get_name(tname, sizeof(tname)-1);
  if (tname[0] == '?') {
    thread_set_name("Audio");
  }
  debug(DEBUG_TRACE, "Sound", "get audio arg=%p ptr=%d", arg, arg->ptr);

  if (buffer && len > 0) {
    sys_memset(buffer, 0, len);

    if ((snd = ptr_lock(arg->ptr, TAG_SOUND)) != NULL) {
      thread_get_name(tname, sizeof(tname)-1);
      if (snd->first && !sys_strcmp(tname, "Audio")) {
        debug(DEBUG_INFO, "Sound", "pumpkin_audio_task_init");
        pumpkin_audio_task_init();
        snd->first = false;
        inited = true;
      }

      nsamples = 0;
      volume = sndUnityGain;
      pan = sndPanCenter;
      pcm = channels = 0;

      if (snd->started && !snd->stopped) {
        nsamples = len / snd->samplesize;
        if (snd->func) {
          debug(DEBUG_TRACE, "Sound", "GetAudio native func len=%d nsamples=%d", len, nsamples);
          err = snd->func(snd->userdata, arg->ptr, buffer, nsamples);
        } else if (snd->vfunc) {
          debug(DEBUG_TRACE, "Sound", "GetAudio native vfunc len=%d", len);
          nbytes = len;
          err = snd->vfunc(snd->userdata, arg->ptr, buffer, &nbytes);
          if (nbytes > len) {
            debug(DEBUG_ERROR, "Sound", "GetAudio native returned more bytes (%d) than the buffer size (%d)", nbytes, len);
          } else {
            len = nbytes;
          }
          nsamples = nbytes / snd->samplesize;
          debug(DEBUG_TRACE, "Sound", "GetAudio native vfunc got len=%d nsamples=%d", len, nsamples);
        } else if (snd->func68k) {
          ram = pumpkin_heap_base();
          if (snd->buffer == NULL) {
            snd->buffer = pumpkin_heap_alloc(len, "snd_buffer");
            freeBuffer = true;
          }
          debug(DEBUG_TRACE, "Sound", "GetAudio m68k func len=%d nsamples=%d", len, nsamples);
          err = CallSndFunc(snd->func68k, snd->userdata68k, arg->ptr, snd->buffer - ram, nsamples);
          MemMove(buffer, snd->buffer, len);
        } else if (snd->funcArm) {
          ram = pumpkin_heap_base();
          if (snd->buffer == NULL) {
            snd->buffer = pumpkin_heap_alloc(len, "snd_buffer");
            freeBuffer = true;
          }
          debug(DEBUG_TRACE, "Sound", "GetAudio ARM func len=%d nsamples=%d", len, nsamples);
          err = CallSndFuncArm(snd->funcArm, snd->userdataArm, arg->ptr, snd->buffer - ram, nsamples);
          MemMove(buffer, snd->buffer, len);
        } else if (snd->vfunc68k) {
          ram = pumpkin_heap_base();
          if (snd->buffer == NULL) {
            snd->buffer = pumpkin_heap_alloc(len, "snd_buffer");
            freeBuffer = true;
          }
          debug(DEBUG_TRACE, "Sound", "GetAudio m68k vfunc len=%d", len);
          nbytes = len;
          err = CallSndVFunc(snd->func68k, snd->userdata68k, arg->ptr, snd->buffer - ram, &nbytes);
          if (nbytes > len) {
            debug(DEBUG_ERROR, "Sound", "GetAudio m68k returned more bytes (%d) than the buffer size (%d)", nbytes, len);
          } else {
            len = nbytes;
          }
          MemMove(buffer, snd->buffer, len);
          nsamples = nbytes / snd->samplesize;
          debug(DEBUG_TRACE, "Sound", "GetAudio m68k vfunc got len=%d nsamples=%d", len, nsamples);
        } else if (snd->vfuncArm) {
          ram = pumpkin_heap_base();
          if (snd->buffer == NULL) {
            snd->buffer = pumpkin_heap_alloc(len, "snd_buffer");
            freeBuffer = true;
          }
          debug(DEBUG_TRACE, "Sound", "GetAudio ARM vfunc len=%d", len);
          nbytes = len;
          err = CallSndVFuncArm(snd->funcArm, snd->userdataArm, arg->ptr, snd->buffer - ram, &nbytes);
          if (nbytes > len) {
            debug(DEBUG_ERROR, "Sound", "GetAudio ARM returned more bytes (%d) than the buffer size (%d)", nbytes, len);
          } else {
            len = nbytes;
          }
          MemMove(buffer, snd->buffer, len);
          nsamples = nbytes / snd->samplesize;
          debug(DEBUG_TRACE, "Sound", "GetAudio ARM vfunc got len=%d nsamples=%d", len, nsamples);
        } else {
          debug(DEBUG_ERROR, "Sound", "GetAudio func and vfunc are not set");
          err = sndErrBadParam;
        }
        if (err == errNone) {
          debug(DEBUG_TRACE, "Sound", "GetAudio no error");
          channels = snd->channels;
          pcm = snd->pcm;
          pan = snd->pan;
          volume = snd->volume;
          r = len;
        } else {
          debug(DEBUG_TRACE, "Sound", "GetAudio error %d freeArg", err);
          nsamples = 0;
          snd->started = false;
          snd->userdata = NULL;
          freeArg = true;
        }
      } else {
        debug(DEBUG_TRACE, "Sound", "GetAudio started %d stopped %d", snd->started, snd->stopped);
        r = 0;
      }

      ptr_unlock(arg->ptr, TAG_SOUND);

      if (nsamples > 0) {
        if ((volume != sndUnityGain || (pan != sndPanCenter && channels == 2))) {
          if (volume == 0) {
            debug(DEBUG_TRACE, "Sound", "%d samples (zero volume)", nsamples);
            switch (pcm) {
              case PCM_U8:
                sys_memset(buffer, 128, len);
                break;
              case PCM_S16:
              case PCM_S32:
                sys_memset(buffer, 0, len);
                break;
            }
          } else {
            if (channels == 1) {
              gain = (float)volume / (float)sndUnityGain;
              debug(DEBUG_TRACE, "Sound", "%d samples (mono gain %.3f)", nsamples, gain);
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
              debug(DEBUG_TRACE, "Sound", "%d samples (left gain %.3f, right gain %.3f)", nsamples, leftGain, rightGain);
              for (i = 0, j = 0; i < nsamples; i++, j += 2) {
                sample = getSample(pcm, buffer, j);
                putSample(pcm, buffer, j, sample * leftGain);
                sample = getSample(pcm, buffer, j);
                putSample(pcm, buffer, j+1, sample * rightGain);
              }
            }
          }
        } else {
          debug(DEBUG_TRACE, "Sound", "%d samples", nsamples);
        }
      } else {
        debug(DEBUG_TRACE, "Sound", "no samples");
      }

      if (freeBuffer) {
        debug(DEBUG_TRACE, "Sound", "freeBuffer");
        if ((snd = ptr_lock(arg->ptr, TAG_SOUND)) != NULL) {
          pumpkin_heap_free(snd->buffer, "snd_buffer");
          snd->buffer = NULL;
          ptr_unlock(arg->ptr, TAG_SOUND);
        }
      }
      if (freeArg) {
        debug(DEBUG_INFO, "Sound", "freeArg");
        if (inited) {
          debug(DEBUG_INFO, "Sound", "pumpkin_audio_task_finish");
          pumpkin_audio_task_finish();
        }
        debug(DEBUG_TRACE, "Sound", "free arg=%p ptr=%d", arg, arg->ptr);
        sys_free(arg);
      }
    }
  }

  return r;
}

int SndGetAudioReply(void *data, int len) {
  msg_audio_t *msg;
  int r = -1;

  if (len == sizeof(msg_audio_t)) {
    msg = (msg_audio_t *)data;

    if (msg->id == MSG_AUDIO) {
      debug(DEBUG_TRACE, "Sound", "received MSG_AUDIO");
      msg->id = MSG_RAUDIO;
      msg->len = SndGetAudio(msg->buffer, msg->len, msg->data);
      r = 0;
    } else {
      debug(DEBUG_ERROR, "Sound", "received msg is not MSG_AUDIO (%u)", msg->id);
      msg->id = MSG_RAUDIO;
      msg->len = 0;
    }
  } else {
    debug(DEBUG_ERROR, "Sound", "received msg has invalid len=%u bytes", len);
  }

  return r;
}

static int SndGetAudioCall(void *buffer, int len, void *data) {
  SndStreamArg *arg = (SndStreamArg *)data;
  msg_audio_t msg, *reply;
  unsigned char *rmsg;
  unsigned int rlen;
  int client, i, r = -1;

  debug(DEBUG_TRACE, "Sound", "get audio arg=%p ptr=%d client=%d", arg, arg->ptr, arg->client);
  msg.id = MSG_AUDIO;
  msg.buffer = buffer;
  msg.len = len;
  msg.data = data;

  if (thread_client_write(arg->client, (uint8_t *)&msg, sizeof(msg)) == sizeof(msg)) {
    for (i = 0; i < 100 && !thread_must_end(); i++) {
      debug(DEBUG_TRACE, "Sound", "waiting reply from client %d ...", arg->client);
      if ((r = thread_server_read_timeout_from(10000, &rmsg, &rlen, &client)) == 0) {
        debug(DEBUG_TRACE, "Sound", "no reply from client %d", arg->client);
        continue;
      }
      if (r == -1) {
        debug(DEBUG_ERROR, "Sound", "error receiving MSG_RAUDIO from client %d", arg->client);
        break;
      }

      debug(DEBUG_TRACE, "Sound", "received reply len=%u bytes from client %d", rlen, client);
      if (client != arg->client) {
        debug(DEBUG_ERROR, "Sound", "ignoring reply from client %d != %d", client, arg->client);
        continue;
      }

      if (rlen == sizeof(msg_audio_t)) {
        reply = (msg_audio_t *)rmsg;
        if (reply->id == MSG_RAUDIO) {
          debug(DEBUG_TRACE, "Sound", "received msg MSG_RAUDIO len=%d", reply->len);
          r = reply->len;
        } else {
          debug(DEBUG_ERROR, "Sound", "received msg is not MSG_RAUDIO (%u)", reply->id);
        }
      } else {
        debug(DEBUG_ERROR, "Sound", "received msg has invalid len=%u bytes", rlen);
      }
      break;
    }
  } else {
    debug(DEBUG_ERROR, "Sound", "error sending MSG_AUDIO to client %d", arg->client);
  }

  return r;
}

Err SndStreamStart(SndStreamRef channel) {
  snd_module_t *module = (snd_module_t *)pumpkin_get_local_storage(snd_key);
  SndStreamType *snd;
  SndStreamArg *arg;
  int handle;
  Err err = sndErrBadParam;

  debug(DEBUG_TRACE, "Sound", "SndStreamStart(%d)", channel);

  if (channel > 0 && module->ap && module->ap->start && (snd = ptr_lock(channel, TAG_SOUND)) != NULL) {
    if (snd->started) {
      snd->stopped = false;
      err = errNone;
    } else if ((handle = pumpkin_audio_get(NULL, NULL, NULL)) > 0) {
      if ((arg = sys_calloc(1, sizeof(SndStreamArg))) != NULL) {
        arg->ptr = channel;
        arg->client = thread_get_handle();
        snd->first = true;
        snd->started = true;
        snd->stopped = false;
        debug(DEBUG_TRACE, "Sound", "starting audio arg=%p ptr=%d", arg, arg->ptr);

        if (module->ap->start(handle, snd->audio, (snd->func || snd->vfunc) ? SndGetAudio : SndGetAudioCall, arg) == 0) {
          debug(DEBUG_TRACE, "Sound", "SndStreamStart(%d) ok", channel);
          pumpkin_audio_check(1);
          err = errNone;
        } else {
          debug(DEBUG_TRACE, "Sound", "SndStreamStart(%d) failed", channel);
          snd->started = false;
          debug(DEBUG_TRACE, "Sound", "free arg=%p ptr=%d", arg, arg->ptr);
          sys_free(arg);
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

  debug(DEBUG_TRACE, "Sound", "SndStreamPause(%d, %d)", channel, pause);
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

  pumpkin_audio_check(0);

  debug(DEBUG_TRACE, "Sound", "SndStreamStop(%d)", channel);
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

static Err SndVariableBufferCallback(void *userdata, SndStreamRef channel, void *buffer, UInt32 *bufferSizeP) {
  snd_param_t *param = (snd_param_t *)userdata;
  UInt32 n;
  Boolean finish = false;
  Err err = sndErrBadParam;

  if (param && buffer && bufferSizeP) {
    sys_memset(buffer, 0, *bufferSizeP);
    n = *bufferSizeP / param->sampleSize;
    if (n > (param->size - param->pos)) n = param->size - param->pos;
    debug(DEBUG_TRACE, "Sound", "SndVariableBufferCallback channel=%d bufferSize=%d size=%d pos=%d n=%d", param->channel, *bufferSizeP, param->size, param->pos, n);
    if (n > 0) {
      if (n < *bufferSizeP) {
        finish = true;
      }
      sys_memcpy(buffer, &param->buffer[param->pos], n * param->sampleSize);
      param->pos += n;
      *bufferSizeP = n * param->sampleSize;
      err = errNone;
    } else {
      *bufferSizeP = 0;
    }
  }

  if (err != errNone) {
    if (param->async) {
      debug(DEBUG_TRACE, "Sound", "SndVariableBufferCallback %d async finish (error)", param->channel);
      SndStreamDelete(param->channel);
      sys_free(param->buffer);
      sys_free(param);
    } else {
      debug(DEBUG_TRACE, "Sound", "SndVariableBufferCallback %d sync finish (error)", param->channel);
      param->finished = true;
    }
  } else if (param->async && finish) {
    debug(DEBUG_TRACE, "Sound", "SndVariableBufferCallback %d async finish (end)", param->channel);
    SndStreamDelete(param->channel);
    sys_free(param->buffer);
    sys_free(param);
  }

  return err;
}

static Err SndPlayBuffer(SndStreamRef *channel, SndPtr sndP, UInt32 size, UInt32 rate, SndSampleType type, SndStreamWidth width, Int32 volume, UInt32 flags) {
  MemHandle h;
  UInt8 *snd;
  UInt32 headerSize;
  snd_param_t *param;
  Err err = sndErrBadParam;

  debug(DEBUG_TRACE, "Sound", "SndPlayBuffer(%p, %u, %d, 0x%08X)", sndP, size, volume, flags);

  if (sndP) {
    param = sys_calloc(1, sizeof(snd_param_t));
    snd = (UInt8 *)sndP;

    if (size == 0) {
      // sndP is a locked handle
      h = MemPtrRecoverHandle(snd);
      if (h) {
        param->size = MemHandleSize(h);
        debug(DEBUG_TRACE, "Sound", "SndPlayBuffer handle size %d", param->size);
      } else {
        // the game Joggle locks a resource and calls SndPlayResource passing not the locked pointer,
        // but pointer+4. Therefore, if MemPtrRecoverHandle fails, we try pointer-4.
        h = MemPtrRecoverHandle(snd - 4);
        if (h) {
          param->size = MemHandleSize(h) - 4;
          debug(DEBUG_TRACE, "Sound", "SndPlayBuffer handle size (-4) %d", param->size);
        } else {
          debug(DEBUG_ERROR, "Sound", "SndPlayBuffer sndP is not a handle");
        }
      }
    } else {
      // sndP is a memory buffer (not a locked handle)
      param->size = size;
    }

    // if rate is 0, the buffer contains a WAV file (header + data)
    // if rate is not 0, the buffer contains only the raw data
    headerSize = rate == 0 ? WAV_HEADER_SIZE : 0;

    if (param->size >= headerSize) {
      if (rate || WavBufferHeader(snd, &rate, &type, &width)) {
        param->finished = false;
        param->size -= headerSize;
        param->buffer = sys_calloc(1, param->size);
        sys_memcpy(param->buffer, snd + headerSize, param->size);
        param->pos = 0;
        param->sampleSize = 1;
        if (width == sndStereo) param->sampleSize <<= 1;
        if (type == sndInt16) param->sampleSize <<= 1;
        else if (type == sndInt32 || type == sndFloat) param->sampleSize <<= 2;
        param->size /= param->sampleSize;
        param->async = flags == sndFlagAsync;
        debug(DEBUG_TRACE, "Sound", "SndPlayBuffer %d samples of size %d", param->size, param->sampleSize);

        if (SndStreamCreateEx(&param->channel, sndOutput, sndFormatPCM, rate, type, width, NULL, SndVariableBufferCallback, param, 0, false, false, false) == errNone) {
          if (channel) *channel = param->channel;
          SndStreamSetVolume(param->channel, volume);
          if (SndStreamStart(param->channel) == errNone) {
            if (flags == sndFlagAsync) {
              debug(DEBUG_TRACE, "Sound", "SndPlayBuffer async");
            } else {
              debug(DEBUG_TRACE, "Sound", "SndPlayBuffer sync loop begin");
              for (; !param->finished && !thread_must_end();) {
                debug(DEBUG_TRACE, "Sound", "SndPlayBuffer wait finished=%d", param->finished);
                SysTaskDelay(5);
              }
              debug(DEBUG_TRACE, "Sound", "SndPlayBuffer sync loop end");
              SndStreamDelete(param->channel);
              sys_free(param->buffer);
              sys_free(param);
            }
            err = errNone;
          } else {
            SndStreamDelete(param->channel);
            sys_free(param->buffer);
            sys_free(param);
          }
        } else {
          sys_free(param->buffer);
          sys_free(param);
        }
      }
    } else {
      err = sndErrFormat;
    }
  }

  debug(DEBUG_TRACE, "Sound", "SndPlayBuffer(%p, %d, 0x%08X): %d", sndP, volume, flags, err);
  return err;
}

Err SndPlayResource(SndPtr sndP, Int32 volume, UInt32 flags) {
  return SndPlayBuffer(NULL, sndP, 0, 0, 0, 0, volume, flags);
}

static Err playFrequency(Int32 frequency, UInt16 duration, UInt16 volume) {
  snd_module_t *module = (snd_module_t *)pumpkin_get_local_storage(snd_key);
  UInt32 size, i;
  UInt8 *buffer;
  double angle, pi2;
  SndStreamRef channel;
  Err err = sndErrBadParam;

  size = (duration * DOCMD_SAMPLE_RATE) / 1000;
  debug(DEBUG_TRACE, "Sound", "playFrequency frequency=%dHz duration=%dms volume=%d size=%d", frequency, duration, volume, size);
  if ((buffer = sys_calloc(1, size)) != NULL) {
    pi2 = 2.0 * sys_pi();
    for (i = 0; i < size; i++) {
      angle = (i * pi2 * frequency) / DOCMD_SAMPLE_RATE;
      buffer[i] = (sys_sin(angle) + 1.0) * 127;
    }
    if (module->lastChannel) {
      debug(DEBUG_TRACE, "Sound", "playFrequency delete previous channel %d", module->lastChannel);
      SndStreamDelete(module->lastChannel);
      module->lastChannel = 0;
    }
    if (SndPlayBuffer(&channel, buffer, size, DOCMD_SAMPLE_RATE, sndInt8, sndMono, volume, sndFlagAsync) == errNone) {
      module->lastChannel = channel;
      debug(DEBUG_TRACE, "Sound", "playFrequency new channel %d", module->lastChannel);
    }
    sys_free(buffer);
    err = errNone;
  }

  return err;
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
  snd_module_t *module = (snd_module_t *)pumpkin_get_local_storage(snd_key);
  
  Err err = sndErrBadParam;

  if (channelP == NULL && cmdP != NULL) {
    debug(DEBUG_TRACE, "Sound", "SndDoCmd %d noWait %d", cmdP->cmd, noWait);

    switch (cmdP->cmd) {
      case sndCmdFreqDurationAmp:
        // Play a tone. SndDoCmd blocks until the tone has finished.
        // param1 is the tone’s frequency in Hertz.
        // param2 is its duration in milliseconds
        // param3 is its amplitude in the range [0, sndMaxAmp].
        // If the amplitude is 0, the sound isn’t played and the function returns immediately.

        if (cmdP->param2 > 0 && cmdP->param3 > 0) {
          module->lastFrequency = cmdP->param1;
          err = playFrequency(cmdP->param1, cmdP->param2, cmdP->param3);
        }
        break;
      case sndCmdFrqOn:
        // Initiate a tone. SndDoCmd returns immediately while the tone plays
        // in the background. Subsequent sound playback requests interrupt the tone.
        // param1 is the tone’s frequency in Hertz.
        // param2 is its duration in milliseconds
        // param3 is its amplitude in the range [0, sndMaxAmp].
        // If the amplitude is 0, the sound isn’t played and the function returns immediately.

        if (cmdP->param2 > 0 && cmdP->param3 > 0) {
          module->lastFrequency = cmdP->param1;
          err = playFrequency(cmdP->param1, cmdP->param2, cmdP->param3);
        }
        break;
      case sndCmdNoteOn:
        // Initiate a MIDI-defined tone. SndDoCmd returns immediately
        // while the tone plays in the background. Subsequent sound
        // playback requests interrupt the tone.
        // param1 is the tone’s pitch given as a MIDI key number in the range [0, 127].
        // param2 is the tone’s duration in milliseconds
        // param3 is its amplitude given as MIDI velocity [0, 127].

        if (cmdP->param1 >= 0 && cmdP->param1 < 128 && cmdP->param2 > 0 && cmdP->param3 > 0) {
          // XXX convert from MIDI velocity (?) to volume
          module->lastFrequency = module->midiNoteFreqency[cmdP->param1];
          err = playFrequency(module->lastFrequency, cmdP->param2, cmdP->param3);
        }
        break;
      case sndCmdQuiet:
        // Stop the playback of the currently generated tone.
        // All parameter values are ignored.
        if (module->lastFrequency) {
          playFrequency(module->lastFrequency, 1, 0);
          module->lastFrequency = 0;
        }
        err = errNone;
        break;
    }
  }

  return err;
}

static Err SndPlayFileCallback(void *userdata, SndStreamRef sound, void *buffer, UInt32 numberofframes) {
  snd_param_t *param = (snd_param_t *)userdata;
  UInt32 n, nread;
  Err err = sndErrBadParam;

  if (param && buffer) {
    n = numberofframes;
    if (n > (param->size - param->pos)) n = param->size - param->pos;
    if (n > 0) {
      if ((err = VFSFileRead(param->f, n * param->sampleSize, buffer, &nread)) == errNone) {
        param->pos += nread / param->sampleSize;
      }
    }
  }

  if (err != errNone) {
    VFSFileClose(param->f);
    if (param->async) {
      debug(DEBUG_TRACE, "Sound", "SndPlayFileCallback async finish");
      sys_free(param);
    } else {
      debug(DEBUG_TRACE, "Sound", "SndPlayFileCallback sync finish");
      param->f = NULL;
      param->finished = true;
    }
  }

  return err;
}

Err SndPlayFile(FileRef f, Int32 volume, UInt32 flags) {
  SndStreamWidth width;
  SndSampleType type;
  UInt32 rate, size;
  snd_param_t *param;
  Err err = sndErrBadParam;

  debug(DEBUG_TRACE, "Sound", "SndPlayFile(%p, %d, 0x%08X)", f, volume, flags);

  if (f && VFSFileSize(f, &size) == errNone && size >= WAV_HEADER_SIZE) {
    if (WavFileHeader(f, &rate, &type, &width)) {
      param = sys_calloc(1, sizeof(snd_param_t));
      param->finished = false;
      param->f = f;
      param->pos = 0;
      param->sampleSize = 1;
      if (width == sndStereo) param->sampleSize <<= 1;
      if (type == sndInt16) param->sampleSize <<= 1;
      else if (type == sndInt32 || type == sndFloat) param->sampleSize <<= 2;
      param->size -= WAV_HEADER_SIZE;
      param->size /= param->sampleSize;
      param->async = flags == sndFlagAsync;

      if (SndStreamCreate(&param->channel, sndOutput, rate, type, width, SndPlayFileCallback, param, 0, false) == errNone) {
        SndStreamSetVolume(param->channel, volume);
        if (SndStreamStart(param->channel) == errNone) {
          if (flags == sndFlagAsync) {
            debug(DEBUG_TRACE, "Sound", "SndPlayFile async");
          } else {
            debug(DEBUG_TRACE, "Sound", "SndPlayFile sync loop begin");
            for (; !param->finished && !thread_must_end();) {
              SysTaskDelay(5);
            }
            sys_free(param);
            debug(DEBUG_TRACE, "Sound", "SndPlayFile sync loop end");
          }
          err = errNone;
        } else {
          SndStreamDelete(param->channel);
          sys_free(param);
        }
      } else {
        sys_free(param);
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

static void SndStreamRefDestructor(void *p) {
  SndStreamType *snd = (SndStreamType *)p;

  if (snd) {
    debug(DEBUG_TRACE, "Sound", "SndStreamRefDestructor(%p)", snd);
    if (snd->audio && snd->ap && snd->ap->destroy) {
      snd->ap->destroy(snd->audio);
      if (snd->buffer) pumpkin_heap_free(snd->buffer, "snd_buffer");
    }
    sys_free(snd);
  }
}

Err SndStreamCreateEx(
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
  Boolean armNative,            /* true if callback is arm native */
  Boolean m68k,
  Boolean arm) {

  snd_module_t *module = (snd_module_t *)pumpkin_get_local_storage(snd_key);
  SndStreamType *snd;
  int ptr, pcm, samplesize;
  uint8_t *ram;
  Err err = sndErrBadParam;

  if (module->ap && module->ap->create && channel && format == sndFormatPCM && (func || vfunc) && mode == sndOutput) {
    switch (type) {
      case sndInt8:
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
      default:
        debug(DEBUG_ERROR, "Sound", "SndStreamCreate unsupported SndSampleType %d", type);
        pcm = -1;
        break;
    }

    if (pcm != -1) {
      if ((snd = sys_calloc(1, sizeof(SndStreamType))) != NULL) {
        snd->tag = TAG_SOUND;
        if (m68k) {
          snd->func = NULL;
          snd->vfunc = NULL;
          snd->userdata = NULL;
          ram = pumpkin_heap_base();
          snd->func68k = func ? (uint8_t *)func - ram : 0;
          snd->vfunc68k = vfunc ? (uint8_t *)vfunc - ram : 0;
          snd->userdata68k = userdata ? (uint8_t *)userdata - ram : 0;
          snd->funcArm = 0;
          snd->vfuncArm = 0;
          snd->userdataArm = 0;
        } else if (arm) {
          snd->func = NULL;
          snd->vfunc = NULL;
          snd->userdata = NULL;
          ram = pumpkin_heap_base();
          snd->funcArm = func ? (uint8_t *)func - ram : 0;
          snd->vfuncArm = vfunc ? (uint8_t *)vfunc - ram : 0;
          snd->userdataArm = userdata ? (uint8_t *)userdata - ram : 0;
          snd->func68k = 0;
          snd->vfunc68k = 0;
          snd->userdata68k = 0;
        } else {
          snd->func = func;
          snd->vfunc = vfunc;
          snd->userdata = userdata;
          snd->func68k = 0;
          snd->vfunc68k = 0;
          snd->userdata68k = 0;
          snd->funcArm = 0;
          snd->vfuncArm = 0;
          snd->userdataArm = 0;
        }
        snd->pcm = pcm;
        snd->rate = samplerate;
        if (width == sndMono) {
          snd->channels = 1;
        } else {
          snd->channels = 2;
          samplesize *= 2;
        }
        snd->samplesize = samplesize;
        debug(DEBUG_TRACE, "Sound", "SndStreamCreate sample rate %d", snd->rate);
        debug(DEBUG_TRACE, "Sound", "SndStreamCreate sample size %d", snd->samplesize);

        snd->volume = sndUnityGain;
        snd->pan = sndPanCenter;
        snd->ap = module->ap;

        if ((snd->audio = module->ap->create(snd->pcm, snd->channels, snd->rate, module->ap->data)) != -1) {
          if ((ptr = ptr_new(snd, SndStreamRefDestructor)) != -1) {
            *channel = ptr;
            err = errNone;
          } else {
            if (module->ap->destroy) module->ap->destroy(snd->audio);
            sys_free(snd);
          }
        } else {
          sys_free(snd);
        }
      }
    }
  } else {
    debug(DEBUG_ERROR, "Sound", "SndStreamCreate invalid arguments");
  }

  debug(DEBUG_TRACE, "Sound", "SndStreamCreate(%d): %d", *channel, err);
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

  return SndStreamCreateEx(channel, mode, sndFormatPCM, samplerate, type, width, func, NULL, userdata, buffsize, armNative, pumpkin_is_m68k(), false);
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

  return SndStreamCreateEx(channel, mode, format, samplerate, type, width, NULL, func, userdata, buffsize, armNative, pumpkin_is_m68k(), false);
}
