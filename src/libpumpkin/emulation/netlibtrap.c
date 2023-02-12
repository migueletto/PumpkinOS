#include <stdlib.h>
#include <stdarg.h>
#include <time.h>

#include <PalmOS.h>
#include <VFSMgr.h>

#ifdef ARMEMU
#include "armemu.h"
#endif
#include "pumpkin.h"
#include "m68k.h"
#include "m68kcpu.h"
#include "emupalmos.h"
#include "debug.h"

void palmos_netlibtrap(uint16_t trap) {
  uint32_t sp;
  uint16_t idx;
  char buf[256];
  Err err;

  sp = m68k_get_reg(NULL, M68K_REG_SP);
  idx = 0;

  switch (trap) {
    case sysLibTrapOpen: {
      uint16_t refNum = ARG16;
      uint32_t netIFErrsP = ARG32;
      UInt16 netIFErrs;
      err = NetLibOpen(refNum, netIFErrsP ? &netIFErrs : NULL);
      if (netIFErrsP) m68k_write_memory_16(netIFErrsP, netIFErrs);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibOpen(refNum=%d, netIFErrsP=0x%08X): %d", refNum, netIFErrsP, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case sysLibTrapClose: {
      uint16_t refNum = ARG16;
      uint16_t immediate = ARG16;
      err = NetLibClose(refNum, immediate);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibClose(refNum=%d, immediate=%d): %d", refNum, immediate, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case sysLibTrapSleep: {
      uint16_t refNum = ARG16;
      err = NetLibSleep(refNum);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibSleep(refNum=%d): %d", refNum, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case sysLibTrapWake: {
      uint16_t refNum = ARG16;
      err = NetLibWake(refNum);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibWake(refNum=%d): %d", refNum, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case netLibTrapFinishCloseWait: {
      // Err NetLibFinishCloseWait(UInt16 libRefnum);
      uint16_t libRefnum = ARG16;
      Err res = NetLibFinishCloseWait(libRefnum);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibFinishCloseWait(libRefnum=%d): %d", libRefnum, res);
    }
    break;
    case netLibTrapOpenIfCloseWait: {
      // Err NetLibOpenIfCloseWait(UInt16 libRefnum);
      uint16_t libRefnum = ARG16;
      Err res = NetLibOpenIfCloseWait(libRefnum);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibOpenIfCloseWait(libRefnum=%d): %d", libRefnum, res);
    }
    break;
    case netLibTrapOpenCount: {
      // Err NetLibOpenCount(UInt16 refNum, out UInt16 *countP);
      uint16_t refNum = ARG16;
      uint32_t countP = ARG32;
      UInt16 l_countP;
      Err res = NetLibOpenCount(refNum, countP ? &l_countP : NULL);
      if (countP) m68k_write_memory_16(countP, l_countP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibOpenCount(refNum=%d, countP=0x%08X [%d]): %d", refNum, countP, l_countP, res);
    }
    break;
    case netLibTrapHandlePowerOff: {
      // Err NetLibHandlePowerOff(UInt16 refNum, in SysEventType *eventP);
      uint16_t refNum = ARG16;
      uint32_t eventP = ARG32;
      SysEventType l_eventP;
      Err res = NetLibHandlePowerOff(refNum, eventP ? &l_eventP : NULL);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibHandlePowerOff(refNum=%d, eventP=0x%08X): %d", refNum, eventP, res);
    }
    break;
    case netLibTrapConnectionRefresh: {
      // Err NetLibConnectionRefresh(UInt16 refNum, Boolean refresh, out UInt8 *allInterfacesUpP, out UInt16 *netIFErrP);
      uint16_t refNum = ARG16;
      uint8_t refresh = ARG8;
      uint32_t allInterfacesUpP = ARG32;
      UInt8 l_allInterfacesUpP;
      uint32_t netIFErrP = ARG32;
      UInt16 l_netIFErrP;
      Err res = NetLibConnectionRefresh(refNum, refresh, allInterfacesUpP ? &l_allInterfacesUpP : NULL, netIFErrP ? &l_netIFErrP : NULL);
      if (allInterfacesUpP) m68k_write_memory_8(allInterfacesUpP, l_allInterfacesUpP);
      if (netIFErrP) m68k_write_memory_16(netIFErrP, l_netIFErrP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibConnectionRefresh(refNum=%d, refresh=%d, allInterfacesUpP=0x%08X, netIFErrP=0x%08X [%d]): %d", refNum, refresh, allInterfacesUpP, netIFErrP, l_netIFErrP, res);
    }
    break;
    case netLibTrapAddrINToA: {
      // Char *NetLibAddrINToA(UInt16 libRefnum, NetIPAddr inet, out Char *spaceP);
      uint16_t libRefnum = ARG16;
      uint32_t inet = ARG32;
      uint32_t spaceP = ARG32;
      char *s_spaceP = emupalmos_trap_in(spaceP, trap, 2);
      Char *res = NetLibAddrINToA(libRefnum, inet, spaceP ? s_spaceP : NULL);
      uint32_t r_res = emupalmos_trap_out(res);
      m68k_set_reg(M68K_REG_A0, r_res);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibAddrINToA(libRefnum=%d, inet=%d, spaceP=0x%08X [%s]): 0x%08X", libRefnum, inet, spaceP, s_spaceP, r_res);
    }
    break;
    case netLibTrapAddrAToIN: {
      // NetIPAddr NetLibAddrAToIN(UInt16 libRefnum, in Char *a);
      uint16_t libRefnum = ARG16;
      uint32_t a = ARG32;
      char *s_a = emupalmos_trap_in(a, trap, 1);
      NetIPAddr res = NetLibAddrAToIN(libRefnum, a ? s_a : NULL);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibAddrAToIN(libRefnum=%d, a=0x%08X [%s]): %d", libRefnum, a, s_a, res);
    }
    break;
    case netLibTrapSocketOpen: {
      // NetSocketRef NetLibSocketOpen(UInt16 libRefnum, NetSocketAddrEnum domain, NetSocketTypeEnum type, Int16 protocol, Int32 timeout, out Err *errP);
      uint16_t libRefnum = ARG16;
      uint8_t domain = ARG8;
      uint8_t type = ARG8;
      int16_t protocol = ARG16;
      int32_t timeout = ARG32;
      uint32_t errP = ARG32;
      Err l_errP;
      NetSocketRef res = NetLibSocketOpen(libRefnum, domain, type, protocol, timeout, errP ? &l_errP : NULL);
      if (errP) m68k_write_memory_16(errP, l_errP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibSocketOpen(libRefnum=%d, domain=%d, type=%d, protocol=%d, timeout=%d, errP=0x%08X): %d", libRefnum, domain, type, protocol, timeout, errP, res);
    }
    break;
    case netLibTrapSocketClose: {
      // Int16 NetLibSocketClose(UInt16 libRefnum, NetSocketRef socket, Int32 timeout, out Err *errP);
      uint16_t libRefnum = ARG16;
      int16_t socket = ARG16;
      int32_t timeout = ARG32;
      uint32_t errP = ARG32;
      Err l_errP;
      Int16 res = NetLibSocketClose(libRefnum, socket, timeout, errP ? &l_errP : NULL);
      if (errP) m68k_write_memory_16(errP, l_errP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibSocketClose(libRefnum=%d, socket=%d, timeout=%d, errP=0x%08X): %d", libRefnum, socket, timeout, errP, res);
    }
    break;
    case netLibTrapSocketBind: {
      // Int16 NetLibSocketBind(UInt16 libRefnum, NetSocketRef socket, in NetSocketAddrType *sockAddrP, Int16 addrLen, Int32 timeout, out Err *errP);
      uint16_t libRefnum = ARG16;
      int16_t socket = ARG16;
      uint32_t sockAddrP = ARG32;
      NetSocketAddrType l_sockAddrP;
      decode_NetSocketAddrType(sockAddrP, &l_sockAddrP);
      int16_t addrLen = ARG16;
      int32_t timeout = ARG32;
      uint32_t errP = ARG32;
      Err l_errP;
      Int16 res = NetLibSocketBind(libRefnum, socket, sockAddrP ? &l_sockAddrP : NULL, addrLen, timeout, errP ? &l_errP : NULL);
      if (errP) m68k_write_memory_16(errP, l_errP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibSocketBind(libRefnum=%d, socket=%d, sockAddrP=0x%08X, addrLen=%d, timeout=%d, errP=0x%08X): %d", libRefnum, socket, sockAddrP, addrLen, timeout, errP, res);
    }
    break;
    case netLibTrapSocketConnect: {
      // Int16 NetLibSocketConnect(UInt16 libRefnum, NetSocketRef socket, in NetSocketAddrType *sockAddrP, Int16 addrLen, Int32 timeout, out Err *errP);
      uint16_t libRefnum = ARG16;
      int16_t socket = ARG16;
      uint32_t sockAddrP = ARG32;
      NetSocketAddrType l_sockAddrP;
      decode_NetSocketAddrType(sockAddrP, &l_sockAddrP);
      int16_t addrLen = ARG16;
      int32_t timeout = ARG32;
      uint32_t errP = ARG32;
      Err l_errP;
      Int16 res = NetLibSocketConnect(libRefnum, socket, sockAddrP ? &l_sockAddrP : NULL, addrLen, timeout, errP ? &l_errP : NULL);
      if (errP) m68k_write_memory_16(errP, l_errP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibSocketConnect(libRefnum=%d, socket=%d, sockAddrP=0x%08X, addrLen=%d, timeout=%d, errP=0x%08X): %d", libRefnum, socket, sockAddrP, addrLen, timeout, errP, res);
    }
    break;
    case netLibTrapSocketListen: {
      // Int16 NetLibSocketListen(UInt16 libRefnum, NetSocketRef socket, UInt16 queueLen, Int32 timeout, out Err *errP);
      uint16_t libRefnum = ARG16;
      int16_t socket = ARG16;
      uint16_t queueLen = ARG16;
      int32_t timeout = ARG32;
      uint32_t errP = ARG32;
      Err l_errP;
      Int16 res = NetLibSocketListen(libRefnum, socket, queueLen, timeout, errP ? &l_errP : NULL);
      if (errP) m68k_write_memory_16(errP, l_errP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibSocketListen(libRefnum=%d, socket=%d, queueLen=%d, timeout=%d, errP=0x%08X): %d", libRefnum, socket, queueLen, timeout, errP, res);
    }
    break;
    case netLibTrapSocketAccept: {
      // Int16 NetLibSocketAccept(UInt16 libRefnum, NetSocketRef socket, out NetSocketAddrType *sockAddrP, inout Int16 *addrLenP, Int32 timeout, out Err *errP);
      uint16_t libRefnum = ARG16;
      int16_t socket = ARG16;
      uint32_t sockAddrP = ARG32;
      NetSocketAddrType l_sockAddrP;
      uint32_t addrLenP = ARG32;
      Int16 l_addrLenP;
      if (addrLenP) l_addrLenP = m68k_read_memory_16(addrLenP);
      int32_t timeout = ARG32;
      uint32_t errP = ARG32;
      Err l_errP;
      Int16 res = NetLibSocketAccept(libRefnum, socket, sockAddrP ? &l_sockAddrP : NULL, addrLenP ? &l_addrLenP : NULL, timeout, errP ? &l_errP : NULL);
      encode_NetSocketAddrType(sockAddrP, &l_sockAddrP);
      if (addrLenP) m68k_write_memory_16(addrLenP, l_addrLenP);
      if (errP) m68k_write_memory_16(errP, l_errP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibSocketAccept(libRefnum=%d, socket=%d, sockAddrP=0x%08X, addrLenP=0x%08X [%d], timeout=%d, errP=0x%08X): %d", libRefnum, socket, sockAddrP, addrLenP, l_addrLenP, timeout, errP, res);
    }
    break;
    case netLibTrapSocketShutdown: {
      // Int16 NetLibSocketShutdown(UInt16 libRefnum, NetSocketRef socket, Int16 direction, Int32 timeout, out Err *errP);
      uint16_t libRefnum = ARG16;
      int16_t socket = ARG16;
      int16_t direction = ARG16;
      int32_t timeout = ARG32;
      uint32_t errP = ARG32;
      Err l_errP;
      Int16 res = NetLibSocketShutdown(libRefnum, socket, direction, timeout, errP ? &l_errP : NULL);
      if (errP) m68k_write_memory_16(errP, l_errP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibSocketShutdown(libRefnum=%d, socket=%d, direction=%d, timeout=%d, errP=0x%08X): %d", libRefnum, socket, direction, timeout, errP, res);
    }
    break;
    case netLibTrapDmReceive: {
      // Int16 NetLibDmReceive(UInt16 libRefNum, NetSocketRef socket, out void *recordP, UInt32 recordOffset, UInt16 rcvLen, UInt16 flags, in void *fromAddrP, inout UInt16 *fromLenP, Int32 timeout, out Err *errP);
      uint16_t libRefNum = ARG16;
      int16_t socket = ARG16;
      uint32_t recordP = ARG32;
      char *s_recordP = emupalmos_trap_in(recordP, trap, 2);
      uint32_t recordOffset = ARG32;
      uint16_t rcvLen = ARG16;
      uint16_t flags = ARG16;
      uint32_t fromAddrP = ARG32;
      char *s_fromAddrP = emupalmos_trap_in(fromAddrP, trap, 6);
      uint32_t fromLenP = ARG32;
      UInt16 l_fromLenP;
      if (fromLenP) l_fromLenP = m68k_read_memory_16(fromLenP);
      int32_t timeout = ARG32;
      uint32_t errP = ARG32;
      Err l_errP;
      Int16 res = NetLibDmReceive(libRefNum, socket, recordP ? s_recordP : NULL, recordOffset, rcvLen, flags, fromAddrP ? s_fromAddrP : NULL, fromLenP ? &l_fromLenP : NULL, timeout, errP ? &l_errP : NULL);
      if (fromLenP) m68k_write_memory_16(fromLenP, l_fromLenP);
      if (errP) m68k_write_memory_16(errP, l_errP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibDmReceive(libRefNum=%d, socket=%d, recordP=0x%08X, recordOffset=%d, rcvLen=%d, flags=%d, fromAddrP=0x%08X, fromLenP=0x%08X [%d], timeout=%d, errP=0x%08X): %d", libRefNum, socket, recordP, recordOffset, rcvLen, flags, fromAddrP, fromLenP, l_fromLenP, timeout, errP, res);
    }
    break;
    case netLibTrapIFGet: {
      // Err NetLibIFGet(UInt16 libRefNum, UInt16 index, out UInt32 *ifCreatorP, out UInt16 *ifInstanceP);
      uint16_t libRefNum = ARG16;
      uint16_t index = ARG16;
      uint32_t ifCreatorP = ARG32;
      UInt32 l_ifCreatorP;
      uint32_t ifInstanceP = ARG32;
      UInt16 l_ifInstanceP;
      Err res = NetLibIFGet(libRefNum, index, ifCreatorP ? &l_ifCreatorP : NULL, ifInstanceP ? &l_ifInstanceP : NULL);
      if (ifCreatorP) m68k_write_memory_32(ifCreatorP, l_ifCreatorP);
      if (ifInstanceP) m68k_write_memory_16(ifInstanceP, l_ifInstanceP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibIFGet(libRefNum=%d, index=%d, ifCreatorP=0x%08X [%d], ifInstanceP=0x%08X [%d]): %d", libRefNum, index, ifCreatorP, l_ifCreatorP, ifInstanceP, l_ifInstanceP, res);
    }
    break;
    case netLibTrapIFAttach: {
      // Err NetLibIFAttach(UInt16 libRefNum, UInt32 ifCreator, UInt16 ifInstance, Int32 timeout);
      uint16_t libRefNum = ARG16;
      uint32_t ifCreator = ARG32;
      uint16_t ifInstance = ARG16;
      int32_t timeout = ARG32;
      Err res = NetLibIFAttach(libRefNum, ifCreator, ifInstance, timeout);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibIFAttach(libRefNum=%d, ifCreator=%d, ifInstance=%d, timeout=%d): %d", libRefNum, ifCreator, ifInstance, timeout, res);
    }
    break;
    case netLibTrapIFDetach: {
      // Err NetLibIFDetach(UInt16 libRefNum, UInt32 ifCreator, UInt16 ifInstance, Int32 timeout);
      uint16_t libRefNum = ARG16;
      uint32_t ifCreator = ARG32;
      uint16_t ifInstance = ARG16;
      int32_t timeout = ARG32;
      Err res = NetLibIFDetach(libRefNum, ifCreator, ifInstance, timeout);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibIFDetach(libRefNum=%d, ifCreator=%d, ifInstance=%d, timeout=%d): %d", libRefNum, ifCreator, ifInstance, timeout, res);
    }
    break;
    case netLibTrapIFUp: {
      // Err NetLibIFUp(UInt16 libRefNum, UInt32 ifCreator, UInt16 ifInstance);
      uint16_t libRefNum = ARG16;
      uint32_t ifCreator = ARG32;
      uint16_t ifInstance = ARG16;
      Err res = NetLibIFUp(libRefNum, ifCreator, ifInstance);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibIFUp(libRefNum=%d, ifCreator=%d, ifInstance=%d): %d", libRefNum, ifCreator, ifInstance, res);
    }
    break;
    case netLibTrapIFDown: {
      // Err NetLibIFDown(UInt16 libRefNum, UInt32 ifCreator, UInt16 ifInstance, Int32 timeout);
      uint16_t libRefNum = ARG16;
      uint32_t ifCreator = ARG32;
      uint16_t ifInstance = ARG16;
      int32_t timeout = ARG32;
      Err res = NetLibIFDown(libRefNum, ifCreator, ifInstance, timeout);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibIFDown(libRefNum=%d, ifCreator=%d, ifInstance=%d, timeout=%d): %d", libRefNum, ifCreator, ifInstance, timeout, res);
    }
    break;
    case netLibTrapSelect: {
      // Int16 NetLibSelect(UInt16 libRefNum, UInt16 width, in NetFDSetType *readFDs, in NetFDSetType *writeFDs, in NetFDSetType *exceptFDs, Int32 timeout, out Err *errP);
      uint16_t libRefNum = ARG16;
      uint16_t width = ARG16;
      uint32_t readFDs = ARG32;
      NetFDSetType l_readFDs;
      if (readFDs) l_readFDs = m68k_read_memory_32(readFDs);
      uint32_t writeFDs = ARG32;
      NetFDSetType l_writeFDs;
      if (writeFDs) l_writeFDs = m68k_read_memory_32(writeFDs);
      uint32_t exceptFDs = ARG32;
      NetFDSetType l_exceptFDs;
      if (exceptFDs) l_exceptFDs = m68k_read_memory_32(exceptFDs);
      int32_t timeout = ARG32;
      uint32_t errP = ARG32;
      Err l_errP;
      Int16 res = NetLibSelect(libRefNum, width, readFDs ? &l_readFDs : NULL, writeFDs ? &l_writeFDs : NULL, exceptFDs ? &l_exceptFDs : NULL, timeout, errP ? &l_errP : NULL);
      if (errP) m68k_write_memory_16(errP, l_errP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibSelect(libRefNum=%d, width=%d, readFDs=0x%08X, writeFDs=0x%08X, exceptFDs=0x%08X, timeout=%d, errP=0x%08X): %d", libRefNum, width, readFDs, writeFDs, exceptFDs, timeout, errP, res);
    }
    break;
    case netLibTrapTracePutS: {
      // Err NetLibTracePutS(UInt16 libRefNum, in Char *strP);
      uint16_t libRefNum = ARG16;
      uint32_t strP = ARG32;
      char *s_strP = emupalmos_trap_in(strP, trap, 1);
      Err res = NetLibTracePutS(libRefNum, strP ? s_strP : NULL);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibTracePutS(libRefNum=%d, strP=0x%08X [%s]): %d", libRefNum, strP, s_strP, res);
    }
    break;
    case netLibTrapOpenConfig: {
      // Err NetLibOpenConfig(UInt16 refNum, UInt16 configIndex, UInt32 openFlags, out UInt16 *netIFErrP);
      uint16_t refNum = ARG16;
      uint16_t configIndex = ARG16;
      uint32_t openFlags = ARG32;
      uint32_t netIFErrP = ARG32;
      UInt16 l_netIFErrP;
      Err res = NetLibOpenConfig(refNum, configIndex, openFlags, netIFErrP ? &l_netIFErrP : NULL);
      if (netIFErrP) m68k_write_memory_16(netIFErrP, l_netIFErrP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibOpenConfig(refNum=%d, configIndex=%d, openFlags=%d, netIFErrP=0x%08X [%d]): %d", refNum, configIndex, openFlags, netIFErrP, l_netIFErrP, res);
    }
    break;
    case netLibTrapConfigMakeActive: {
      // Err NetLibConfigMakeActive(UInt16 refNum, UInt16 configIndex);
      uint16_t refNum = ARG16;
      uint16_t configIndex = ARG16;
      Err res = NetLibConfigMakeActive(refNum, configIndex);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibConfigMakeActive(refNum=%d, configIndex=%d): %d", refNum, configIndex, res);
    }
    break;
    case netLibTrapConfigIndexFromName: {
      // Err NetLibConfigIndexFromName(UInt16 refNum, in NetConfigNameType *nameP, out UInt16 *indexP);
      uint16_t refNum = ARG16;
      uint32_t nameP = ARG32;
      NetConfigNameType l_nameP;
      decode_NetConfigNameType(nameP, &l_nameP);
      uint32_t indexP = ARG32;
      UInt16 l_indexP;
      Err res = NetLibConfigIndexFromName(refNum, nameP ? &l_nameP : NULL, indexP ? &l_indexP : NULL);
      if (indexP) m68k_write_memory_16(indexP, l_indexP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibConfigIndexFromName(refNum=%d, nameP=0x%08X, indexP=0x%08X [%d]): %d", refNum, nameP, indexP, l_indexP, res);
    }
    break;
    case netLibTrapConfigDelete: {
      // Err NetLibConfigDelete(UInt16 refNum, UInt16 index);
      uint16_t refNum = ARG16;
      uint16_t index = ARG16;
      Err res = NetLibConfigDelete(refNum, index);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibConfigDelete(refNum=%d, index=%d): %d", refNum, index, res);
    }
    break;
    case netLibTrapConfigSaveAs: {
      // Err NetLibConfigSaveAs(UInt16 refNum, in NetConfigNameType *nameP);
      uint16_t refNum = ARG16;
      uint32_t nameP = ARG32;
      NetConfigNameType l_nameP;
      decode_NetConfigNameType(nameP, &l_nameP);
      Err res = NetLibConfigSaveAs(refNum, nameP ? &l_nameP : NULL);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibConfigSaveAs(refNum=%d, nameP=0x%08X): %d", refNum, nameP, res);
    }
    break;
    case netLibTrapConfigRename: {
      // Err NetLibConfigRename(UInt16 refNum, UInt16 index, in NetConfigNameType *newNameP);
      uint16_t refNum = ARG16;
      uint16_t index = ARG16;
      uint32_t newNameP = ARG32;
      NetConfigNameType l_newNameP;
      decode_NetConfigNameType(newNameP, &l_newNameP);
      Err res = NetLibConfigRename(refNum, index, newNameP ? &l_newNameP : NULL);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibConfigRename(refNum=%d, index=%d, newNameP=0x%08X): %d", refNum, index, newNameP, res);
    }
    break;
    case netLibTrapConfigAliasSet: {
      // Err NetLibConfigAliasSet(UInt16 refNum, UInt16 configIndex, UInt16 aliasToIndex);
      uint16_t refNum = ARG16;
      uint16_t configIndex = ARG16;
      uint16_t aliasToIndex = ARG16;
      Err res = NetLibConfigAliasSet(refNum, configIndex, aliasToIndex);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibConfigAliasSet(refNum=%d, configIndex=%d, aliasToIndex=%d): %d", refNum, configIndex, aliasToIndex, res);
    }
    break;
    case netLibTrapConfigAliasGet: {
      // Err NetLibConfigAliasGet(UInt16 refNum, UInt16 aliasIndex, out UInt16 *indexP, out Boolean *isAnotherAliasP);
      uint16_t refNum = ARG16;
      uint16_t aliasIndex = ARG16;
      uint32_t indexP = ARG32;
      UInt16 l_indexP;
      uint32_t isAnotherAliasP = ARG32;
      Boolean l_isAnotherAliasP;
      Err res = NetLibConfigAliasGet(refNum, aliasIndex, indexP ? &l_indexP : NULL, isAnotherAliasP ? &l_isAnotherAliasP : NULL);
      if (indexP) m68k_write_memory_16(indexP, l_indexP);
      if (isAnotherAliasP) m68k_write_memory_8(isAnotherAliasP, l_isAnotherAliasP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibConfigAliasGet(refNum=%d, aliasIndex=%d, indexP=0x%08X [%d], isAnotherAliasP=0x%08X): %d", refNum, aliasIndex, indexP, l_indexP, isAnotherAliasP, res);
    }
    break;
    case netLibTrapConfigList: {
      // Err NetLibConfigList(UInt16 refNum, out NetConfigNameType nameArray[], inout UInt16 *arrayEntriesP)
      uint16_t libRefNum = ARG16;
      uint32_t nameArrayP = ARG32;
      uint32_t arrayEntriesP = ARG32;
      NetConfigNameType nameArray[16];
      UInt16 arrayEntries = 0;
      if (arrayEntriesP) arrayEntries = m68k_read_memory_16(arrayEntriesP);
      if (arrayEntries > 16) arrayEntries = 16;
      err = NetLibConfigList(libRefNum, nameArray, &arrayEntries);
      if (arrayEntries > 16) arrayEntries = 16;
      for (UInt16 i = 0; i < arrayEntries; i++) {
        encode_NetConfigNameType(nameArrayP + i*4, &nameArray[i]);
      }
      if (arrayEntriesP) m68k_write_memory_16(arrayEntriesP, arrayEntries);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibConfigList(refNum=%d, nameArray=0x%08X, arrayEntriesP=0x%08X [%d]): %d", libRefNum, nameArray, arrayEntriesP, arrayEntries, err);
      m68k_set_reg(M68K_REG_D0, err);
      }
      break;
    case netLibTrapGetHostByName: {
      // NetHostInfoType *NetLibGetHostByName(UInt16 libRefNum, in Char *nameP, out NetHostInfoBufType *bufP, Int32 timeout, out Err *errP);
      uint16_t libRefNum = ARG16;
      uint32_t nameP = ARG32;
      char *s_nameP = emupalmos_trap_in(nameP, trap, 1);
      uint32_t bufP = ARG32;
      NetHostInfoBufType l_bufP;
      int32_t timeout = ARG32;
      uint32_t errP = ARG32;
      Err l_errP;
      NetHostInfoType *res = NetLibGetHostByName(libRefNum, s_nameP, &l_bufP, timeout, &l_errP);
      encode_NetHostInfoBufType(bufP, &l_bufP);
      if (errP) m68k_write_memory_16(errP, l_errP);
      uint32_t r_res = res ? bufP : 0;
      m68k_set_reg(M68K_REG_A0, r_res);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibGetHostByName(libRefNum=%d, nameP=0x%08X, bufP=0x%08X, timeout=%d, errP=0x%08X): 0x%08X", libRefNum, nameP, bufP, timeout, errP, r_res);
      }
      break;
    case netLibTrapGetHostByAddr: {
      // NetHostInfoType *NetLibGetHostByAddr(UInt16 libRefNum, in UInt8 *addrP, UInt16 len, UInt16 type, out NetHostInfoBufType *bufP, Int32 timeout, out Err *errP);
      uint16_t libRefNum = ARG16;
      uint32_t addrP = ARG32;
      UInt32 l_addrP;
      if (addrP) l_addrP = m68k_read_memory_32(addrP);
      uint16_t len = ARG16;
      uint16_t type = ARG16;
      uint32_t bufP = ARG32;
      NetHostInfoBufType l_bufP;
      int32_t timeout = ARG32;
      uint32_t errP = ARG32;
      Err l_errP;
      NetHostInfoType *res = NetLibGetHostByAddr(libRefNum, (UInt8 *)&l_addrP, len, type, &l_bufP, timeout, &l_errP);
      encode_NetHostInfoBufType(bufP, &l_bufP);
      if (errP) m68k_write_memory_16(errP, l_errP);
      uint32_t r_res = res ? bufP : 0;
      m68k_set_reg(M68K_REG_A0, r_res);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibGetHostByAddr(libRefNum=%d, addrP=0x%08X, len=%d, type=%d, bufP=0x%08X, timeout=%d, errP=0x%08X): 0x%08X", libRefNum, addrP, len, type, bufP, timeout, errP, r_res);
      }
      break;
    case netLibTrapSend: {
      // Int16 NetLibSend(UInt16 libRefNum, NetSocketRef socket, in void *bufP, UInt16 bufLen, UInt16 flags, in void *toAddrP, UInt16 toLen, Int32 timeout, out Err *errP);
      uint16_t libRefNum = ARG16;
      int16_t socket = ARG16;
      uint32_t bufP = ARG32;
      char *s_bufP = emupalmos_trap_in(bufP, trap, 2);
      uint16_t bufLen = ARG16;
      uint16_t flags = ARG16;
      uint32_t toAddrP = ARG32;
      NetSocketAddrType toAddr;
      decode_NetSocketAddrType(toAddrP, &toAddr);
      uint16_t toLen = ARG16;
      int32_t timeout = ARG32;
      uint32_t errP = ARG32;
      Err l_errP;
      Int16 res = NetLibSend(libRefNum, socket, s_bufP, bufLen, flags, toAddrP ? &toAddr : NULL, toLen, timeout, &l_errP);
      if (errP) m68k_write_memory_16(errP, l_errP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibSend(libRefNum=%d, socket=%d, bufP=0x%08X, bufLen=%d, flags=%d, toAddrP=0x%08X, toLen=%d, timeout=%d, errP=0x%08X): %d", libRefNum, socket, bufP, bufLen, flags, toAddrP, toLen, timeout, errP, res);
      }
      break;
    case netLibTrapReceive: {
      // Int16 NetLibReceive(UInt16 libRefNum, NetSocketRef socket, out void *bufP, UInt16 bufLen, UInt16 flags, out void *fromAddrP, inout UInt16 *fromLenP, Int32 timeout, out Err *errP);
      uint16_t libRefNum = ARG16;
      int16_t socket = ARG16;
      uint32_t bufP = ARG32;
      char *s_bufP = emupalmos_trap_in(bufP, trap, 2);
      uint16_t bufLen = ARG16;
      uint16_t flags = ARG16;
      uint32_t fromAddrP = ARG32;
      NetSocketAddrType fromAddr;
      uint32_t fromLenP = ARG32;
      UInt16 l_fromLenP;
      if (fromLenP) l_fromLenP = m68k_read_memory_16(fromLenP);
      int32_t timeout = ARG32;
      uint32_t errP = ARG32;
      Err l_errP;
      Int16 res = NetLibReceive(libRefNum, socket, s_bufP, bufLen, flags, fromAddrP ? &fromAddr : NULL, fromLenP ? &l_fromLenP : NULL, timeout, &l_errP);
      encode_NetSocketAddrType(fromAddrP, &fromAddr);
      if (fromLenP) m68k_write_memory_16(fromLenP, l_fromLenP);
      if (errP) m68k_write_memory_16(errP, l_errP);
      m68k_set_reg(M68K_REG_D0, res);
      debug(DEBUG_TRACE, "EmuPalmOS", "NetLibReceive(libRefNum=%d, socket=%d, bufP=0x%08X, bufLen=%d, flags=%d, fromAddrP=0x%08X, fromLenP=0x%08X, timeout=%d, errP=0x%08X): %d", libRefNum, socket, bufP, bufLen, flags, fromAddrP, fromLenP, timeout, errP, res);
      }
      break;
    default:
      snprintf(buf, sizeof(buf)-1, "NetLib trap 0x%04X not mapped", trap);
      emupalmos_panic(buf, EMUPALMOS_INVALID_TRAP);
      break;
  }
}
