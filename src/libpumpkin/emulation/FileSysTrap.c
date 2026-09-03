#include <PalmOS.h>
#include <VFSMgr.h>
#include <DLServer.h>
#include <Helper.h>
#include <CharAttr.h>
#include <HsNavCommon.h>
#include <INetMgr.h>

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

void palmos_FileSysTrap(uint32_t sp, uint16_t idx, uint32_t trap) {

  switch (trap) {
    case sysTrapFileControl: {
      // Err FileControl(FileOpEnum op, FileHand stream, inout void *valueP, inout Int32 *valueLenP)
      uint8_t op = ARG8;
      uint32_t stream = ARG32;
      FileHand l_stream = (FileHand)emupalmos_trap_in(stream, trap, 1);
      uint32_t valueP = ARG32;
      void *s_valueP = emupalmos_trap_in(valueP, trap, 2);
      uint32_t valueLenP = ARG32;
      emupalmos_trap_in(valueLenP, trap, 3);
      Int32 l_valueLenP;
      if (valueLenP) l_valueLenP = m68k_read_memory_32(valueLenP);
      // XXX read valueP
      Err res = FileControl(op, l_stream, s_valueP, &l_valueLenP);
      // XXX fill valueP
      if (valueLenP) m68k_write_memory_32(valueLenP, l_valueLenP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "FileControl(op=%d, stream=0x%08X, valueP=0x%08X, valueLenP=0x%08X): %d", op, stream, valueP, valueLenP, res);
    }
    break;
    case sysTrapFileOpen: {
      // FileHand FileOpen(UInt16 cardNo, in Char *nameP, UInt32 type, UInt32 creator, UInt32 openMode, out Err *errP)
      uint16_t cardNo = ARG16;
      uint32_t nameP = ARG32;
      uint8_t *ram = pumpkin_heap_base();
      char *s_nameP = nameP ? (char *)(ram + nameP) : NULL;
      uint32_t type = ARG32;
      uint32_t creator = ARG32;
      uint32_t openMode = ARG32;
      uint32_t errP = ARG32;
      Err l_errP;
      FileHand res = FileOpen(cardNo, nameP ? s_nameP : NULL, type, creator, openMode, errP ? &l_errP : NULL);
      if (errP) m68k_write_memory_16(errP, l_errP);
      uint32_t r_res = emupalmos_trap_out(res);
      m68k_set_reg(M68K_REG_A0, r_res);
      debug(DEBUG_TRACE, "EmuPalmOS", "FileOpen(cardNo=%d, nameP=0x%08X [%s], type=%d, creator=%d, openMode=0x%08X, errP=0x%08X): 0x%08X", cardNo, nameP, s_nameP, type, creator, openMode, errP, r_res);
    }
    break;
    case sysTrapFileClose: {
      // Err FileClose(FileHand stream)
      uint32_t stream = ARG32;
      uint8_t *ram = pumpkin_heap_base();
      FileHand l_stream = stream ? (FileHand)(ram + stream) : NULL;
      Err res = FileClose(stream ? l_stream : 0);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "FileClose(stream=0x%08X): %d", stream, res);
    }
    break;
    case sysTrapFileDelete: {
      // Err FileDelete(UInt16 cardNo, in Char *nameP)
      uint16_t cardNo = ARG16;
      uint32_t nameP = ARG32;
      uint8_t *ram = pumpkin_heap_base();
      char *s_nameP = nameP ? (char *)(ram + nameP) : NULL;
      Err res = FileDelete(cardNo, nameP ? s_nameP : NULL);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "FileDelete(cardNo=%d, nameP=0x%08X [%s]): %d", cardNo, nameP, s_nameP, res);
    }
    break;
    case sysTrapFileReadLow: {
      // Int32 FileReadLow(FileHand stream, out void *baseP, Int32 offset, Boolean dataStoreBased, Int32 objSize, Int32 numObj, out Err *errP)
      uint32_t stream = ARG32;
      uint8_t *ram = pumpkin_heap_base();
      FileHand l_stream = stream ? (FileHand)(ram + stream) : NULL;
      uint32_t baseP = ARG32;
      void *s_baseP = baseP ? (void *)(ram + baseP) : NULL;
      int32_t offset = ARG32;
      uint8_t dataStoreBased = ARG8;
      int32_t objSize = ARG32;
      int32_t numObj = ARG32;
      uint32_t errP = ARG32;
      Err l_errP;
      Int32 res = FileReadLow(stream ? l_stream : 0, baseP ? s_baseP : NULL, offset, dataStoreBased, objSize, numObj, errP ? &l_errP : NULL);
      if (errP) m68k_write_memory_16(errP, l_errP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "FileReadLow(stream=0x%08X, baseP=0x%08X, offset=%d, dataStoreBased=%d, objSize=%d, numObj=%d, errP=0x%08X): %d", stream, baseP, offset, dataStoreBased, objSize, numObj, errP, res);
    }
    break;
    case sysTrapFileWrite: {
      // Int32 FileWrite(FileHand stream, in void *dataP, Int32 objSize, Int32 numObj, out Err *errP)
      uint32_t stream = ARG32;
      uint8_t *ram = pumpkin_heap_base();
      FileHand l_stream = stream ? (FileHand)(ram + stream) : NULL;
      uint32_t dataP = ARG32;
      void *s_dataP = dataP ? (void *)(ram + dataP) : NULL;
      int32_t objSize = ARG32;
      int32_t numObj = ARG32;
      uint32_t errP = ARG32;
      Err l_errP;
      Int32 res = FileWrite(stream ? l_stream : 0, dataP ? s_dataP : NULL, objSize, numObj, errP ? &l_errP : NULL);
      if (errP) m68k_write_memory_16(errP, l_errP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "FileWrite(stream=0x%08X, dataP=0x%08X, objSize=%d, numObj=%d, errP=0x%08X): %d", stream, dataP, objSize, numObj, errP, res);
    }
    break;
    case sysTrapFileSeek: {
      // Err FileSeek(FileHand stream, Int32 offset, FileOriginEnum origin)
      uint32_t stream = ARG32;
      uint8_t *ram = pumpkin_heap_base();
      FileHand l_stream = stream ? (FileHand)(ram + stream) : NULL;
      int32_t offset = ARG32;
      uint8_t origin = ARG8;
      Err res = FileSeek(stream ? l_stream : 0, offset, origin);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "FileSeek(stream=0x%08X, offset=%d, origin=%d): %d", stream, offset, origin, res);
    }
    break;
    case sysTrapFileTell: {
      // Int32 FileTell(FileHand stream, out Int32 *fileSizeP, out Err *errP)
      uint32_t stream = ARG32;
      uint8_t *ram = pumpkin_heap_base();
      FileHand l_stream = stream ? (FileHand)(ram + stream) : NULL;
      uint32_t fileSizeP = ARG32;
      Int32 l_fileSizeP = 0;
      uint32_t errP = ARG32;
      Err l_errP;
      Int32 res = FileTell(stream ? l_stream : 0, fileSizeP ? &l_fileSizeP : NULL, errP ? &l_errP : NULL);
      if (fileSizeP) m68k_write_memory_32(fileSizeP, l_fileSizeP);
      if (errP) m68k_write_memory_16(errP, l_errP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "FileTell(stream=0x%08X, fileSizeP=0x%08X [%d], errP=0x%08X): %d", stream, fileSizeP, l_fileSizeP, errP, res);
    }
    break;
    case sysTrapFileTruncate: {
      // Err FileTruncate(FileHand stream, Int32 newSize)
      uint32_t stream = ARG32;
      uint8_t *ram = pumpkin_heap_base();
      FileHand l_stream = stream ? (FileHand)(ram + stream) : NULL;
      int32_t newSize = ARG32;
      Err res = FileTruncate(stream ? l_stream : 0, newSize);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "FileTruncate(stream=0x%08X, newSize=%d): %d", stream, newSize, res);
    }
    break;
  }
}
