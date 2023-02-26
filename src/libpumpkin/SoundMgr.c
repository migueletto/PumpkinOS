#include <PalmOS.h>
  
#include "sys.h"
#include "thread.h"
#include "pwindow.h"
#include "vfs.h"
#include "mem.h"
#include "bytes.h"
#include "emupalmosinc.h"
#include "pumpkin.h"
#include "debug.h"
#include "xalloc.h"

typedef struct {
  window_provider_t *wp;
} snd_module_t;

extern thread_key_t *snd_key;

int SndInitModule(window_provider_t *wp) {
  snd_module_t *module;

  if ((module = xcalloc(1, sizeof(snd_module_t))) == NULL) {
    return -1;
  }

  module->wp = wp;
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
  debug(DEBUG_ERROR, "PALMOS", "SndSetDefaultVolume not implemented");
}

void SndGetDefaultVolume(UInt16 *alarmAmpP, UInt16 *sysAmpP, UInt16 *masterAmpP) {
  debug(DEBUG_ERROR, "PALMOS", "SndGetDefaultVolume not implemented");
}

Err SndDoCmd(void * /*SndChanPtr*/ channelP, SndCommandPtr cmdP, Boolean noWait) {
  debug(DEBUG_ERROR, "PALMOS", "SndDoCmd not implemented");
  return sysErrParamErr;
}

void SndPlaySystemSound(SndSysBeepType beepID) {
  debug(DEBUG_ERROR, "PALMOS", "SndPlaySystemSound not implemented");
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
  Err err = sysErrParamErr;

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

      if (module->wp) {
        debug(DEBUG_INFO, "Sound", "SndPlaySmf playing %d bytes", i);
        if (volume >= sndMaxAmp) {
          volume = 128;
        } else if (volume > 0) {
          volume <<= 1; // PalmOS: 0-64, SDL: 0-128
        }
        module->wp->mixer_play(smfP, i, volume);
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
  Err err = sysErrParamErr;

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
  Err err = sysErrParamErr;

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
  Err err = sysErrParamErr;

  if (module->wp) {
    module->wp->mixer_stop();
    err = errNone;
  }

  return err;
}

Err SndStreamDelete(SndStreamRef channel) {
  debug(DEBUG_ERROR, "PALMOS", "SndStreamDelete not implemented");
  return sysErrParamErr;
}

Err SndStreamStart(SndStreamRef channel) {
  debug(DEBUG_ERROR, "PALMOS", "SndStreamStart not implemented");
  return sysErrParamErr;
}

Err SndStreamPause(SndStreamRef channel, Boolean pause) {
  debug(DEBUG_ERROR, "PALMOS", "SndStreamPause not implemented");
  return sysErrParamErr;
}

Err SndStreamStop(SndStreamRef channel) {
  debug(DEBUG_ERROR, "PALMOS", "SndStreamStop not implemented");
  return sysErrParamErr;
}

Err SndStreamSetVolume(SndStreamRef channel, Int32 volume) {
  debug(DEBUG_ERROR, "PALMOS", "SndStreamSetVolume not implemented");
  return sysErrParamErr;
}

Err SndStreamGetVolume(SndStreamRef channel, Int32 *volume) {
  debug(DEBUG_ERROR, "PALMOS", "SndStreamGetVolume not implemented");
  return sysErrParamErr;
}

Err SndPlayResource(SndPtr sndP, Int32 volume, UInt32 flags) {
  debug(DEBUG_ERROR, "PALMOS", "SndPlayResource not implemented");
  return sysErrParamErr;
}

Err SndStreamSetPan(SndStreamRef channel, Int32 panposition) {
  debug(DEBUG_ERROR, "PALMOS", "SndStreamSetPan not implemented");
  return sysErrParamErr;
}

Err SndStreamGetPan(SndStreamRef channel, Int32 *panposition) {
  debug(DEBUG_ERROR, "PALMOS", "SndStreamGetPan not implemented");
  return sysErrParamErr;
}

Err SndStreamDeviceControl(SndStreamRef channel, Int32 cmd, void* param, Int32 size) {
  debug(DEBUG_ERROR, "PALMOS", "SndStreamDeviceControl not implemented");
  return sysErrParamErr;
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

  debug(DEBUG_ERROR, "PALMOS", "SndStreamCreate not implemented");
  return sysErrParamErr;
}

Err SndStreamCreateExtended(
  SndStreamRef *channel,  /* channel-ID is stored here */
  SndStreamMode mode,   /* input/output, enum */
  SndFormatType format, /* enum, e.g., sndFormatMP3 */
  UInt32 samplerate,    /* in frames/second, e.g. 44100, 48000 */
  SndSampleType type,   /* enum, e.g. sndInt16, if applicable, or 0 otherwise */
  SndStreamWidth width, /* enum, e.g. sndMono */
  SndStreamVariableBufferCallback func, /* function that gets called to fill a buffer */
  void *userdata,     /* gets passed in to the above function */
  UInt32 buffsize,      /* preferred buffersize, use 0 for default */
  Boolean armNative) {

  debug(DEBUG_ERROR, "PALMOS", "SndStreamCreateExtended not implemented");
  return sysErrParamErr;
}
