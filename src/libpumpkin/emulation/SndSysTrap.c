#include <PalmOS.h>
#include <VFSMgr.h>
#include <DLServer.h>
#include <Helper.h>
#include <CharAttr.h>
#include <HsNavCommon.h>

#include "sys.h"
#ifdef ARMEMU
#include "armemu.h"
#include "armp.h"
#endif
#include "pumpkin.h"
#include "mutex.h"
#include "storage.h"
#include "logtrap.h"
#include "m68k/m68k.h"
#include "m68k/m68kcpu.h"
#include "emupalmos.h"
#include "debug.h"

void palmos_SndSysTrap(uint32_t sp, uint16_t idx, uint32_t trap) {

  switch (trap) {
    case sysTrapSndPlaySmf: {
      // Err SndPlaySmf(void *chanP, SndSmfCmdEnum cmd, UInt8 *smfP, SndSmfOptionsType *selP, SndSmfChanRangeType *chanRangeP, SndSmfCallbacksType *callbacksP, Boolean bNoWait)
      uint32_t chanP = ARG32;
      uint8_t cmd = ARG8;
      uint32_t smfP = ARG32;
      uint32_t selP = ARG32;
      uint32_t chanRangeP = ARG32;
      uint32_t callbacksP = ARG32;
      uint8_t bNoWait = ARG8;
      emupalmos_trap_in(chanP, trap, 0);
      emupalmos_trap_in(selP, trap, 3);
      emupalmos_trap_in(chanRangeP, trap, 4);
      emupalmos_trap_in(callbacksP, trap, 5);
      SndSmfOptionsType options;
      decode_smfoptions(selP, &options);
      Err res = SndPlaySmf(NULL, cmd, (UInt8 *)emupalmos_trap_in(smfP, trap, 2), selP ? &options : NULL, NULL, NULL, bNoWait);
      debug(DEBUG_TRACE, "EmuPalmOS", "SndPlaySmf(0x%08X, %d, 0x%08X, 0x%08X, 0x%08X, 0x%08X, %d): %d", chanP, cmd, smfP, selP, chanRangeP, callbacksP, bNoWait, res);
      m68k_set_reg(M68K_REG_D0, res);
    }
    break;
    case sysTrapSndPlaySmfResource: {
      //Err SndPlaySmfResource(UInt32 resType, Int16 resID, SystemPreferencesChoice volumeSelector)
      uint32_t resType = ARG32;
      int16_t resID = ARG32;
      uint8_t volumeSelector = ARG8;
      Err res = SndPlaySmfResource(resType, resID, volumeSelector);
      debug(DEBUG_TRACE, "EmuPalmOS", "SndPlaySmfResource(0x%08X, %d, %d): %d", resType, resID, volumeSelector, res);
      m68k_set_reg(M68K_REG_D0, res);
    }
    break;
    case sysTrapSndCreateMidiList: {
      // Boolean SndCreateMidiList(UInt32 creator, Boolean multipleDBs, UInt16 *wCountP, MemHandle *entHP)
      uint32_t creator = ARG32;
      uint8_t multipleDBs = ARG8;
      uint32_t wCountP = ARG32;
      uint32_t entHP = ARG32;
      emupalmos_trap_in(wCountP, trap, 2);
      emupalmos_trap_in(entHP, trap, 3);
      UInt16 wCount;
      MemHandle entH;
      Boolean res = SndCreateMidiList(creator, multipleDBs, wCountP ? &wCount : NULL, entHP ? &entH : NULL);
      if (wCountP) m68k_write_memory_16(wCountP, wCount);
      if (entHP) m68k_write_memory_32(entHP, emupalmos_trap_out(entH));
      char screator[8];
      pumpkin_id2s(creator, screator);
      debug(DEBUG_TRACE, "EmuPalmOS", "SndCreateMidiList('%s', %d, 0x%08X, 0x%08X): %d", screator, multipleDBs, wCountP, entHP, res);
      m68k_set_reg(M68K_REG_D0, res);
    }
    break;
    case sysTrapSndPlaySystemSound: {
      // void SndPlaySystemSound(SndSysBeepType beepID)
      uint8_t beepID = ARG8;
      SndPlaySystemSound(beepID);
      debug(DEBUG_TRACE, "EmuPalmOS", "SndSysBeepType(%d)", beepID);
    }
    break;
    case sysTrapSndPlayResource: {
      // Err SndPlayResource(SndPtr sndP, Int32 volume, UInt32 flags)
      uint32_t sndP = ARG32;
      int32_t volume = ARG32;
      uint32_t flags = ARG32;
      void *sndPtr = (void *)emupalmos_trap_in(sndP, trap, 0);
      Err res = SndPlayResource(sndPtr, volume, flags);
      debug(DEBUG_TRACE, "EmuPalmOS", "SndPlayResource(0x%08X, %d, 0x%08X): %d", sndP, volume, flags, res);
      m68k_set_reg(M68K_REG_D0, res);
    }
    break;
    case sysTrapSndDoCmd: {
      // Err SndDoCmd(void *channelP, SndCommandPtr cmdP, Boolean noWait)
      uint32_t channelP = ARG32;
      uint32_t cmdP = ARG32;
      uint8_t noWait = ARG8;
      emupalmos_trap_in(channelP, trap, 0);
      SndCommandType cmd;
      decode_sndcmd(cmdP, &cmd);
      Err err = SndDoCmd(NULL, cmdP ? &cmd : NULL, noWait);
      debug(DEBUG_TRACE, "EmuPalmOS", "SndDoCmd(0x%08X, 0x%08X, %d): %d", channelP, cmdP, noWait, err);
      m68k_set_reg(M68K_REG_D0, err);
    }
    break;
    case sysTrapSndGetDefaultVolume: {
      // void SndGetDefaultVolume(UInt16 *alarmAmpP, UInt16 *sysAmpP, UInt16 *masterAmpP)
      uint32_t alarmAmpP = ARG32;
      uint32_t sysAmpP = ARG32;
      uint32_t masterAmpP = ARG32;
      emupalmos_trap_in(alarmAmpP, trap, 0);
      emupalmos_trap_in(sysAmpP, trap, 1);
      emupalmos_trap_in(masterAmpP, trap, 2);
      UInt16 alarmAmp, sysAmp, masterAmp;
      SndGetDefaultVolume(alarmAmpP ? &alarmAmp : NULL, sysAmpP ? &sysAmp : NULL, masterAmpP ? &masterAmp : NULL);
      if (alarmAmpP) m68k_write_memory_16(alarmAmpP, alarmAmp);
      if (sysAmpP) m68k_write_memory_16(sysAmpP, sysAmp);
      if (masterAmpP) m68k_write_memory_16(masterAmpP, masterAmp);
      debug(DEBUG_TRACE, "EmuPalmOS", "SndGetDefaultVolume(0x%08X, 0x%08X, 0x%08X)", alarmAmpP, sysAmpP, masterAmpP);
    }
    break;
    case sysTrapSndSetDefaultVolume: {
      // void SndSetDefaultVolume(UInt16 *alarmAmpP, UInt16 *sysAmpP, UInt16 *defAmpP)
      uint32_t alarmAmpP = ARG32;
      uint32_t sysAmpP = ARG32;
      uint32_t defAmpP = ARG32;
      emupalmos_trap_in(alarmAmpP, trap, 0);
      emupalmos_trap_in(sysAmpP, trap, 1);
      emupalmos_trap_in(defAmpP, trap, 2);
      UInt16 alarmAmp = alarmAmpP ? m68k_read_memory_16(alarmAmpP) : 0;
      UInt16 sysAmp = sysAmpP ? m68k_read_memory_16(sysAmpP) : 0;
      UInt16 defAmp = defAmpP ? m68k_read_memory_16(defAmpP) : 0;
      SndSetDefaultVolume(alarmAmpP ? &alarmAmp : NULL, sysAmpP ? &sysAmp : NULL, defAmpP ? &defAmp : NULL);
      debug(DEBUG_TRACE, "EmuPalmOS", "SndSetDefaultVolume(0x%08X, 0x%08X, 0x%08X)", alarmAmpP, sysAmpP, defAmpP);
    }
    break;
    case sysTrapSndStreamCreate: {
      // Err SndStreamCreate(SndStreamRef *channel, SndStreamMode mode, UInt32 samplerate, SndSampleType type, SndStreamWidth width, SndStreamBufferCallback func, void *userdata, UInt32 buffsize, Boolean armNative)
      uint32_t channelP = ARG32;
      uint8_t mode = ARG8;
      uint32_t samplerate = ARG32;
      uint16_t type = ARG16;
      uint8_t width = ARG8;
      uint32_t funcP = ARG32;
      uint32_t userdataP = ARG32;
      uint32_t buffsize = ARG32;
      uint8_t armNative = ARG8;
      SndStreamRef *channel = (SndStreamRef *)emupalmos_trap_in(channelP, trap, 0);
      SndStreamBufferCallback func = (SndStreamBufferCallback)emupalmos_trap_in(funcP, trap, 5);
      void *userdata = emupalmos_trap_in(userdataP, trap, 6);
      Err err = SndStreamCreate(channel, mode, samplerate, type, width, func, userdata, buffsize, armNative);
      if (channelP) m68k_write_memory_32(channelP, *channel);
      debug(DEBUG_TRACE, "EmuPalmOS", "SndStreamCreate(0x%08X, %d, %d, %d, %d, 0x%08X, 0x%08X, %d, %d): %d",
        channelP, mode, samplerate, type, width, funcP, userdataP, buffsize, armNative, err);
      m68k_set_reg(M68K_REG_D0, err);
    }
    break;
    case sysTrapSndStreamCreateExtended: {
      // Err SndStreamCreateExtended(SndStreamRef *channel, SndStreamMode mode, SndFormatType format, UInt32 samplerate, SndSampleType type, SndStreamWidth width, SndStreamVariableBufferCallback func, void *userdata, UInt32 buffsize, Boolean armNative)
      uint32_t channelP = ARG32;
      uint8_t mode = ARG8;
      uint32_t format = ARG32;
      uint32_t samplerate = ARG32;
      uint16_t type = ARG16;
      uint8_t width = ARG8;
      uint32_t funcP = ARG32;
      uint32_t userdataP = ARG32;
      uint32_t buffsize = ARG32;
      uint8_t armNative = ARG8;
      SndStreamRef *channel = (SndStreamRef *)emupalmos_trap_in(channelP, trap, 0);
      SndStreamVariableBufferCallback func = (SndStreamVariableBufferCallback)emupalmos_trap_in(funcP, trap, 6);
      void *userdata = emupalmos_trap_in(userdataP, trap, 7);
      Err err = SndStreamCreateExtended(channel, mode, format, samplerate, type, width, func, userdata, buffsize, armNative);
      debug(DEBUG_TRACE, "EmuPalmOS", "SndStreamCreateExtented(0x%08X, %d, %d, %d %d, %d, 0x%08X, 0x%08X, %d, %d): %d",
        channelP, mode, format, samplerate, type, width, funcP, userdataP, buffsize, armNative, err);
      m68k_set_reg(M68K_REG_D0, err);
    }
    break;
    case sysTrapSndStreamDelete: {
      // Err SndStreamDelete(SndStreamRef channel)
      uint32_t channel = ARG32;
      Err err = SndStreamDelete(channel);
      debug(DEBUG_TRACE, "EmuPalmOS", "SndStreamDelete(0x%08X): %d", channel, err);
      m68k_set_reg(M68K_REG_D0, err);
    }
    break;
    case sysTrapSndStreamSetVolume: {
      // Err SndStreamSetVolume(SndStreamRef channel, Int32 volume)
      uint32_t channel = ARG32;
      uint32_t volume = ARG32;
      Err err = SndStreamSetVolume(channel, volume);
      debug(DEBUG_TRACE, "EmuPalmOS", "SndStreamSetVolume(0x%08X, %d): %d", channel, volume, err);
      m68k_set_reg(M68K_REG_D0, err);
    }
    break;
    case sysTrapSndStreamStart: {
      // Err SndStreamStart(SndStreamRef channel)
      uint32_t channel = ARG32;
      Err err = SndStreamStart(channel);
      debug(DEBUG_TRACE, "EmuPalmOS", "SndStreamStart(0x%08X): %d", channel, err);
      m68k_set_reg(M68K_REG_D0, err);
    }
    break;
    case sysTrapSndStreamStop: {
      // Err SndStreamStop(SndStreamRef channel)
      uint32_t channel = ARG32;
      Err err = SndStreamStop(channel);
      debug(DEBUG_TRACE, "EmuPalmOS", "SndStreamStop(0x%08X): %d", channel, err);
      m68k_set_reg(M68K_REG_D0, err);
    }
    break;
  }
}
