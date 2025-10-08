#include <PalmOS.h>
#include <VFSMgr.h>
#include <FileBrowserLibCommon.h>
#include <FileBrowserLib68K.h>

#include "sys.h"
#ifdef ARMEMU
#include "armemu.h"
#endif
#include "pumpkin.h"
#include "m68k/m68k.h"
#include "m68k/m68kcpu.h"
#include "emupalmos.h"
#include "debug.h"

void palmos_pfillibtrap(uint16_t trap) {
  uint32_t sp;
  uint16_t idx;
  char buf[256];
  Err err;

  sp = m68k_get_reg(NULL, M68K_REG_SP);
  idx = 0;
  debug(DEBUG_TRACE, "EmuPalmOS", "FileBrowser trap 0x%04X", trap);

  switch (trap) {
    case sysLibTrapOpen: {
      uint16_t refNum = ARG16;
      err = FileBrowserLibOpen(refNum);
      debug(DEBUG_TRACE, "EmuPalmOS", "FileBrowserLibOpen(refNum=%d): %d", refNum, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case sysLibTrapClose: {
      uint16_t refNum = ARG16;
      err = FileBrowserLibClose(refNum);
      debug(DEBUG_TRACE, "EmuPalmOS", "FileBrowserLibClose(refNum=%d): %d", refNum, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case sysLibTrapSleep: {
      uint16_t refNum = ARG16;
      err = FileBrowserLibSleep(refNum);
      debug(DEBUG_TRACE, "EmuPalmOS", "FileBrowserLibSleep(refNum=%d): %d", refNum, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case sysLibTrapWake: {
      uint16_t refNum = ARG16;
      err = FileBrowserLibWake(refNum);
      debug(DEBUG_TRACE, "EmuPalmOS", "FileBrowserLibWake(refNum=%d): %d", refNum, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case exgLibTrapHandleEvent: {
      // Boolean ExgLibHandleEvent(UInt16 libRefnum, void *eventP)
      uint16_t refNum = ARG16;
      uint32_t eventP = ARG32;
      EventType *event = emupalmos_trap_in(eventP, trap, 1);
      debug(DEBUG_TRACE, "EmuPalmOS", "FileBrowser ExgLibHandleEvent(refNum=%d, event=%s)",
        refNum, event ? EvtGetEventName(event->eType) : NULL);
      m68k_set_reg(M68K_REG_D0, false);
      }
      break;
    case exgLibTrapRequest: {
      // Err ExgLibRequest(UInt16 libRefNum, ExgSocketType *socketP)
      uint16_t refNum = ARG16;
      uint32_t socketP = ARG32;
      uint8_t *socket = emupalmos_trap_in(socketP, trap, 1);
      debug(DEBUG_TRACE, "EmuPalmOS", "FileBrowser ExgLibRequest(refNum=%d, socketP=0x%08X)", refNum, socketP);
      if (socket) {
        debug(DEBUG_TRACE, "EmuPalmOS", "ExgSocketType");
        debug_bytes(DEBUG_INFO, "EmuPalmOS", socket, 60);
      }
      m68k_set_reg(M68K_REG_D0, 0);
      }
      break;
    case kFileBrowserLibTrapShowOpenDialog: {
      // Boolean FileBrowserLibShowOpenDialog(UInt16 refNum,
      //   UInt16 *volRefNumP, Char *path, UInt16 numExtensions,
      //   const Char **extensions, const Char *fileType, const Char *title,
      //   UInt32 flags)
      uint16_t refNum = ARG16;
      debug(DEBUG_TRACE, "EmuPalmOS", "FileBrowserLibShowOpenDialog(refNum=%d)", refNum);
      m68k_set_reg(M68K_REG_D0, 0);
      }
      break;
    case kFileBrowserLibTrapShowSaveAsDialog: {
      // Boolean FileBrowserLibShowSaveAsDialog(UInt16 refNum, UInt16 *volRefNumP,
      //   Char *path, UInt16 numExtensions, const Char **extensions,
      //   const Char *defaultExtension, const Char *fileType,
      //   const Char *title, UInt32 flags)
      uint16_t refNum = ARG16;
      debug(DEBUG_TRACE, "EmuPalmOS", "FileBrowserLibShowSaveAsDialog(refNum=%d)", refNum);
      m68k_set_reg(M68K_REG_D0, 0);
      }
      break;
    case kFileBrowserLibTrapParseFileURL: {
      // Err FileBrowserLibParseFileURL(UInt16 refNum, const Char *urlP, UInt16 *volRefNumP, Char **filePathP)
      debug(DEBUG_TRACE, "EmuPalmOS", "FileBrowserLibParseFileURL 0x%04X", kFileBrowserLibTrapParseFileURL);
      uint16_t refNum = ARG16;
      uint32_t urlP = ARG32;
      uint32_t volRefNumP = ARG32;
      uint32_t filePathP = ARG32;
      char *url = emupalmos_trap_in(urlP, trap, 1);
      emupalmos_trap_in(volRefNumP, trap, 2);
      emupalmos_trap_in(filePathP, trap, 3);
      UInt16 volRefNum;
      char *filePath;
      err = FileBrowserLibParseFileURL(refNum, url, volRefNumP ? &volRefNum : NULL, filePathP ? &filePath : NULL);
      if (volRefNumP) m68k_write_memory_16(volRefNumP, volRefNum);
      if (filePathP) m68k_write_memory_32(filePathP, emupalmos_trap_out(filePath));
      debug(DEBUG_TRACE, "EmuPalmOS", "FileBrowserLibParseFileURL(refNum=%d, url=0x%08X \"%s\", volRefNumP=0x%08X, filePathP=0x%08X)", refNum, urlP, url, volRefNumP, filePathP);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case kFileBrowserLibTrapShowMultiselectDialog: {
      // Boolean FileBrowserLibShowMultiselectDialog(UInt16 refNum,
      //   UInt16 *volRefNumP, Char *path, UInt16 *numFilesP,
      //   UInt16 maxFiles, Char **filenames, UInt16 numExtensions,
      //   const Char **extensions, const Char *fileType,
      //   const Char *title, UInt32 flags)
      uint16_t refNum = ARG16;
      debug(DEBUG_TRACE, "EmuPalmOS", "FileBrowserLibShowMultiselectDialog(refNum=%d)", refNum);
      m68k_set_reg(M68K_REG_D0, 0);
      }
      break;
    default:
      sys_snprintf(buf, sizeof(buf)-1, "FileBrowserLib trap 0x%04X not mapped", trap);
      emupalmos_panic(buf, EMUPALMOS_INVALID_TRAP);
      break;
  }
}
