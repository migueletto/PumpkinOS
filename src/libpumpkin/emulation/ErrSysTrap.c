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

void palmos_ErrSysTrap(uint32_t sp, uint16_t idx, uint32_t trap) {

  switch (trap) {
    case sysTrapErrDisplayFileLineMsg: {
      // void ErrDisplayFileLineMsg(const Char * const filename, UInt16 lineNo, const Char * const msg)
      uint32_t filenameP = ARG32;
      uint16_t lineNo = ARG16;
      uint32_t msgP = ARG32;
      char *filename = emupalmos_trap_in(filenameP, trap, 0);
      char *msg = emupalmos_trap_in(msgP, trap, 2);
      ErrDisplayFileLineMsg(filename, lineNo, msg);
      debug(DEBUG_INFO, "EmuPalmOS", "ErrDisplayFileLineMsg(0x%08X \"%s\", %d, 0x%08X \"%s\")", filenameP, filename ? filename : "", lineNo, msgP, msg ? msg : "");
    }
    break;
    case sysTrapErrExceptionList: {
      // MemPtr *ErrExceptionList(void)
      uint8_t *e = (uint8_t *)ErrExceptionList();
      uint32_t a = emupalmos_trap_out(e);
      debug(DEBUG_TRACE, "EmuPalmOS", "ErrExceptionList(): 0x%08X", a);
      m68k_set_reg(M68K_REG_A0, a);
    }
    break;
    case sysTrapErrThrow: {
      // void ErrThrow(Int32 err)
      uint32_t code = ARG32;
      uint8_t *e = (uint8_t *)ErrExceptionList();
      uint32_t a = emupalmos_trap_out(e);
      uint32_t exceptionP = m68k_read_memory_32(a);
    
      // typedef struct ErrExceptionType {
      //   struct ErrExceptionType *nextP;  // next exception type
      //   ErrJumpBuf state;                // setjmp/longjmp storage
      //   Int32 err;                       // Error code
      // } ErrExceptionType;
      uint32_t nextP = m68k_read_memory_32(exceptionP);
      m68k_write_memory_32(a, nextP);
      uint32_t bufP = exceptionP + 4;
      uint32_t aux = m68k_read_memory_32(bufP);
      m68k_set_reg(M68K_REG_D3, aux);
      aux = m68k_read_memory_32(bufP + 4);
      m68k_set_reg(M68K_REG_D4, aux);
      aux = m68k_read_memory_32(bufP + 8);
      m68k_set_reg(M68K_REG_D5, aux);
      aux = m68k_read_memory_32(bufP + 12);
      m68k_set_reg(M68K_REG_D6, aux);
      aux = m68k_read_memory_32(bufP + 16);
      m68k_set_reg(M68K_REG_D7, aux);
      aux = m68k_read_memory_32(bufP + 20);
      m68k_set_reg(M68K_REG_PC, aux);
      aux = m68k_read_memory_32(bufP + 24);
      m68k_set_reg(M68K_REG_A2, aux);
      aux = m68k_read_memory_32(bufP + 28);
      m68k_set_reg(M68K_REG_A3, aux);
      aux = m68k_read_memory_32(bufP + 32);
      m68k_set_reg(M68K_REG_A4, aux);
      aux = m68k_read_memory_32(bufP + 36);
      m68k_set_reg(M68K_REG_A5, aux);
      aux = m68k_read_memory_32(bufP + 40);
      m68k_set_reg(M68K_REG_A6, aux);
      aux = m68k_read_memory_32(bufP + 44);
      m68k_set_reg(M68K_REG_A7, aux);
      debug(DEBUG_TRACE, "EmuPalmOS", "ErrThrow(%d)", code);
      m68k_set_reg(M68K_REG_D0, code);
    }
    break;
    case sysTrapErrSetJump: {
      // Int16 ErrSetJump(ErrJumpBuf buf)
      uint32_t bufP = ARG32;
      emupalmos_trap_in(bufP, trap, 0);
      // typedef long *ErrJumpBuf[12];  // D3-D7,PC,A2-A7
      uint32_t aux = m68k_get_reg(NULL, M68K_REG_D3);
      m68k_write_memory_32(bufP, aux);
      aux = m68k_get_reg(NULL, M68K_REG_D4);
      m68k_write_memory_32(bufP + 4, aux);
      aux = m68k_get_reg(NULL, M68K_REG_D5);
      m68k_write_memory_32(bufP + 8, aux);
      aux = m68k_get_reg(NULL, M68K_REG_D6);
      m68k_write_memory_32(bufP + 12, aux);
      aux = m68k_get_reg(NULL, M68K_REG_D7);
      m68k_write_memory_32(bufP + 16, aux);
      aux = m68k_get_reg(NULL, M68K_REG_PC);
      m68k_write_memory_32(bufP + 20, aux);
      aux = m68k_get_reg(NULL, M68K_REG_A2);
      m68k_write_memory_32(bufP + 24, aux);
      aux = m68k_get_reg(NULL, M68K_REG_A3);
      m68k_write_memory_32(bufP + 28, aux);
      aux = m68k_get_reg(NULL, M68K_REG_A4);
      m68k_write_memory_32(bufP + 32, aux);
      aux = m68k_get_reg(NULL, M68K_REG_A5);
      m68k_write_memory_32(bufP + 36, aux);
      aux = m68k_get_reg(NULL, M68K_REG_A6);
      m68k_write_memory_32(bufP + 40, aux);
      aux = m68k_get_reg(NULL, M68K_REG_A7);
      m68k_write_memory_32(bufP + 44, aux);
      debug(DEBUG_TRACE, "EmuPalmOS", "ErrSetJump(0x%08X): %d", bufP, 0);
      m68k_set_reg(M68K_REG_D0, 0); // XXX not calling ErrSetJump()
    }
    break;
    case sysTrapErrLongJump: {
      // void ErrLongJump(ErrJumpBuf buf, Int16 result)
      uint32_t bufP = ARG32;
      int16_t result = ARG16;
      emupalmos_trap_in(bufP, trap, 0);
      uint32_t aux = m68k_read_memory_32(bufP);
      m68k_set_reg(M68K_REG_D3, aux);
      aux = m68k_read_memory_32(bufP + 4);
      m68k_set_reg(M68K_REG_D4, aux);
      aux = m68k_read_memory_32(bufP + 8);
      m68k_set_reg(M68K_REG_D5, aux);
      aux = m68k_read_memory_32(bufP + 12);
      m68k_set_reg(M68K_REG_D6, aux);
      aux = m68k_read_memory_32(bufP + 16);
      m68k_set_reg(M68K_REG_D7, aux);
      aux = m68k_read_memory_32(bufP + 20);
      m68k_set_reg(M68K_REG_PC, aux);
      aux = m68k_read_memory_32(bufP + 24);
      m68k_set_reg(M68K_REG_A2, aux);
      aux = m68k_read_memory_32(bufP + 28);
      m68k_set_reg(M68K_REG_A3, aux);
      aux = m68k_read_memory_32(bufP + 32);
      m68k_set_reg(M68K_REG_A4, aux);
      aux = m68k_read_memory_32(bufP + 36);
      m68k_set_reg(M68K_REG_A5, aux);
      aux = m68k_read_memory_32(bufP + 40);
      m68k_set_reg(M68K_REG_A6, aux);
      aux = m68k_read_memory_32(bufP + 44);
      m68k_set_reg(M68K_REG_A7, aux);
      debug(DEBUG_TRACE, "EmuPalmOS", "ErrLongJump(0x%08X, %d)", bufP, result);
      m68k_set_reg(M68K_REG_D0, result);
    }
    break;
    case sysTrapErrAlertCustom: {
      // Int16 ErrAlertCustom(Err errCode, Char *errMsgP, Char *preMsgP, Char *postMsgP)
      uint16_t errCode = ARG16;
      uint32_t errMsgP = ARG32;
      uint32_t preMsgP = ARG32;
      uint32_t postMsgP = ARG32;
      char *errMsg = emupalmos_trap_in(errMsgP, trap, 1);
      char *preMsg = emupalmos_trap_in(preMsgP, trap, 2);
      char *postMsg = emupalmos_trap_in(postMsgP, trap, 3);
      Int16 res = ErrAlertCustom(errCode, errMsg, preMsg, postMsg);
      debug(DEBUG_TRACE, "EmuPalmOS", "ErrAlertCustom(%u, 0x%08X, 0x%08X, 0x%08X): %d", errCode, errMsgP, preMsgP, postMsgP, res);
      m68k_set_reg(M68K_REG_D0, res);
    }
    break;
  }
}
