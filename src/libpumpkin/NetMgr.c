#include <PalmOS.h>

#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "pwindow.h"
#include "sys.h"
#include "vfs.h"
#include "mem.h"
#include "pumpkin.h"
#include "timeutc.h"
#include "debug.h"
#include "xalloc.h"

#define NetLibIfCreator 'NetP'
#define NetLibIfName    "Default"

Err NetLibOpen(UInt16 libRefnum, UInt16 *netIFErrsP) {
  if (netIFErrsP) *netIFErrsP = 0;
  return errNone;
}

Err NetLibClose(UInt16 libRefnum, UInt16 immediate) {
  return errNone;
}

Err NetLibSleep(UInt16 libRefnum) {
  return errNone;
}

Err NetLibWake(UInt16 libRefnum) {
  return errNone;
}

Err NetLibFinishCloseWait(UInt16 libRefnum) {
  return errNone;
}

Err NetLibOpenIfCloseWait(UInt16 libRefnum) {
  return errNone;
}

Err NetLibOpenCount(UInt16 refNum, UInt16 *countP) {
  if (countP) *countP = 1;
  return errNone;
}

Err NetLibHandlePowerOff(UInt16 refNum, SysEventType *eventP) {
  return errNone;
}

// Converts an IP address from 32-bit network byte order into a dotted decimal ASCII string.
Char *NetLibAddrINToA(UInt16 libRefnum, NetIPAddr inet, Char *spaceP) {
  if (spaceP) {
    StrPrintF(spaceP, "%d.%d.%d.%d", (inet & 0xFF), ((inet >> 8) & 0xFF), ((inet >> 16) & 0xFF), ((inet >> 24) & 0xFF));
  }

  return spaceP;
}

NetIPAddr NetLibAddrAToIN(UInt16 libRefnum, const Char *a) {
  int b1, b2, b3, b4;
  NetIPAddr inet = -1;

  if (a) {
    if (sscanf(a, "%d.%d.%d.%d", &b1, &b2, &b3, &b4) == 4) {
      inet = (b1 & 0xFF) | ((b2 & 0XFF) << 8) | ((b3 & 0xFF) << 16) | ((b4 & 0xFF) << 24);
    }
  }

  return inet;
}

Err NetLibIFGet(UInt16 libRefNum, UInt16 index, UInt32 *ifCreatorP, UInt16 *ifInstanceP) {
  Err err = netErrInvalidInterface;

  if (index == 0) {
    if (ifCreatorP) *ifCreatorP = NetLibIfCreator;
    if (ifInstanceP) *ifInstanceP = 0;
    err = errNone;
  }

  return err;
}

Err NetLibIFUp(UInt16 libRefNum, UInt32 ifCreator, UInt16 ifInstance) {
  return (ifCreator == NetLibIfCreator && ifInstance == 0) ? errNone : netErrInterfaceNotFound;
}

Err NetLibIFDown(UInt16 libRefNum, UInt32 ifCreator, UInt16 ifInstance, Int32 timeout) {
  return (ifCreator == NetLibIfCreator && ifInstance == 0) ? errNone : netErrInterfaceNotFound;
}

Err NetLibIFAttach(UInt16 libRefNum, UInt32 ifCreator, UInt16 ifInstance, Int32 timeout) {
  return (ifCreator == NetLibIfCreator && ifInstance == 0) ? errNone : netErrInterfaceNotFound;
}

Err NetLibIFDetach(UInt16 libRefNum, UInt32 ifCreator, UInt16 ifInstance, Int32 timeout) {
  return (ifCreator == NetLibIfCreator && ifInstance == 0) ? errNone : netErrInterfaceNotFound;
}

Err NetLibTracePrintF(UInt16 libRefNum, const Char *formatStr, ...) {
  va_list ap;

  va_start(ap, formatStr);
  debugva(DEBUG_INFO, "NetMgr", formatStr, ap);
  va_end(ap);

  return errNone;
}

Err NetLibTracePutS(UInt16 libRefNum, Char *strP) {
  debug(DEBUG_INFO, "NetMgr", "%s", strP);

  return errNone;
}

Err NetLibConnectionRefresh(UInt16 refNum, Boolean refresh, UInt8 *allInterfacesUpP, UInt16 *netIFErrP) {
  if (allInterfacesUpP) *allInterfacesUpP = true;
  if (netIFErrP) *netIFErrP = 0;

  return errNone;
}

/*
typedef struct {
  Char *    nameP;         // official name of host
  Char **   nameAliasesP;  // array of alias's for the name
  UInt16    addrType;      // address type of return addresses
  UInt16    addrLen;       // the length, in bytes, of the addresse
                           // Note this denotes length of a address, not # of addresses.
  UInt8 **  addrListP;     // array of ptrs to addresses in HBO
} NetHostInfoType;

typedef struct {
  NetHostInfoType hostInfo;         // high level results of call are here

  // The following fields contain the variable length data that hostInfo points to
  Char  name[netDNSMaxDomainName+1];      // hostInfo->name
  Char *aliasList[netDNSMaxAliases+1];    // +1 for 0 termination.
  Char  aliases[netDNSMaxAliases][netDNSMaxDomainName+1];

  NetIPAddr *addressList[netDNSMaxAddresses];
  NetIPAddr address[netDNSMaxAddresses];

} NetHostInfoBufType;
*/

NetHostInfoPtr NetLibGetHostByName(UInt16 libRefNum, const Char *nameP, NetHostInfoBufPtr bufP, Int32 timeout, Err *errP) {
  NetHostInfoPtr r = NULL;
  UInt32 ip, i;

  if (nameP) {
    debug(DEBUG_INFO, "NetMgr", "NetLibGetHostByName \"%s\"", nameP);
    if ((ip = sys_socket_ipv4((char *)nameP)) != -1) {
      debug(DEBUG_INFO, "NetMgr", "NetLibGetHostByName \"%s\" ip ok", nameP);
      if (bufP) {
        MemSet(bufP, sizeof(NetHostInfoBufType), 0);
        r = &bufP->hostInfo;
        StrNCopy(bufP->name, nameP, netDNSMaxDomainName);
        r->nameP = bufP->name;

        // no aliases
        for (i = 0; i < netDNSMaxAliases; i++) {
          bufP->aliasList[i] = NULL;
        }
        r->nameAliasesP = bufP->aliasList;

        // just one IPv4 address
        bufP->address[0] = ip;
        bufP->addressList[0] = &bufP->address[0];
        for (i = 1; i < netDNSMaxAddresses; i++) {
          bufP->addressList[i] = NULL;
        }
        r->addrListP = (UInt8 **)bufP->addressList;
        r->addrLen = sizeof(NetIPAddr);
        r->addrType = netSocketAddrINET; // XXX is this the correct type here ?

        if (errP) *errP = errNone;

      } else {
        if (errP) *errP = netErrDNSBadName;
      }
    } else {
      if (errP) *errP = netErrDNSNonexistantName;
    }
  } else {
    if (errP) *errP = netErrDNSBadName;
  }

  return r;
}

NetHostInfoPtr NetLibGetHostByAddr(UInt16 libRefNum, UInt8 *addrP, UInt16 len, UInt16 type, NetHostInfoBufPtr bufP, Int32 timeout, Err *errP) {
  NetHostInfoPtr r = NULL;
  NetIPAddr *addr;
  char host[32];

  if (errP) *errP = netErrDNSBadName;

  if (addrP && len == 4 && type == netSocketAddrINET) {
    addr = (NetIPAddr *)addrP;
    NetLibAddrINToA(libRefNum, *addr, host);
    r = NetLibGetHostByName(libRefNum, host, bufP, timeout, errP);
    if (errP) *errP = errNone;
  }

  return r;
}

/*
typedef enum {
  netSocketAddrRaw=0,                 // (AF_UNSPEC, AF_RAW)
  netSocketAddrINET=2                 // (AF_INET)
} NetSocketAddrEnum;

typedef struct NetSocketAddrType {
  Int16 family;
  UInt8 data[14];
} NetSocketAddrType;

typedef struct NetSocketAddrINType {
  Int16     family;         // Address family in HBO (Host UInt8 Order)
  UInt16    port;           // the UDP port in NBO (Network UInt8 Order)
  NetIPAddr addr;           // IP address in NBO (Network UInt8 Order)
} NetSocketAddrINType;

typedef struct NetSocketAddrRawType {
  Int16     family;         // Address family in HBO (Host UInt8 Order)
  UInt16    ifInstance;       // the interface instance number 
  UInt32    ifCreator;        // the interface creator
} NetSocketAddrRawType;
*/

Int16 NetLibSend(UInt16 libRefNum, NetSocketRef socket, void *bufP, UInt16 bufLen, UInt16 flags, void *toAddrP, UInt16 toLen, Int32 timeout, Err *errP) {
  NetSocketAddrType *addr;
  NetSocketAddrINType *inAddr;
  char host[32];
  int port;
  Int16 r = -1;

  if (toAddrP) {
    addr = (NetSocketAddrType *)toAddrP;
    if (addr->family == netSocketAddrINET) {
      inAddr = (NetSocketAddrINType *)addr;
      NetLibAddrINToA(libRefNum, inAddr->addr, host);
      port = NetNToHS(inAddr->port);
      r = sys_socket_sendto(socket, host, port, (uint8_t *)bufP, bufLen);
    } else {
      debug(DEBUG_ERROR, "NetMgr", "NetLibSend family %d not supported", addr->family);
    }
  } else {
    r = sys_write(socket, (uint8_t *)bufP, bufLen);
  }
  debug(DEBUG_TRACE, "NetMgr", "NetLibSend %d bytes", bufLen);
  debug_bytes(DEBUG_TRACE, "NetMgr", bufP, bufLen);
  debug(DEBUG_TRACE, "NetMgr", "NetLibSend r=%d", r);

  if (r == bufLen) {
    *errP = 0;
  } else {
    *errP = netErrParamErr;
    r = -1;
  }

  return r;
}

Int16 NetLibReceive(UInt16 libRefNum, NetSocketRef socket, void *bufP, UInt16 bufLen, UInt16 flags, void *fromAddrP, UInt16 *fromLenP, Int32 timeout, Err *errP) {
  NetSocketAddrINType *inAddr;
  struct timeval tv, *tvp;
  char host[32];
  int port, nread = 0;
  Int16 r = -1;

  if (fromAddrP) {
    if (timeout == -1) {
      tvp = NULL;
    } else {
      tv.tv_sec = 0;
      tv.tv_usec = timeout * (1000000 / SysTicksPerSecond());
      tvp = &tv;
    }
    r = sys_socket_recvfrom(socket, host, sizeof(host), &port, (uint8_t *)bufP, bufLen, tvp);

    if (r > 0) {
      inAddr = (NetSocketAddrINType *)fromAddrP;
      inAddr->family = netSocketAddrINET;
      inAddr->port = NetNToHS(port);
      inAddr->addr = NetLibAddrAToIN(libRefNum, host);
      nread = r;
      r = 1;
    }
  } else {
    debug(DEBUG_TRACE, "NetMgr", "NetLibReceive %d bytes ...", bufLen);
    r = sys_read_timeout(socket, (uint8_t *)bufP, bufLen, &nread, timeout == -1 ? -1 : timeout * (1000000 / SysTicksPerSecond()));
    if (r > 0 && nread > 0) debug_bytes(DEBUG_TRACE, "NetMgr", bufP, nread);
    debug(DEBUG_TRACE, "NetMgr", "NetLibReceive r=%d nread=%d", r, nread);
  }

  switch (r) {
    case -1:
      debug(DEBUG_INFO, "NetMgr", "NetLibReceive error");
      *errP = netErrParamErr;
      break;
    case  0:
      debug(DEBUG_INFO, "NetMgr", "NetLibReceive timeout");
      *errP = netErrTimeout;
      r = -1;
      break;
    default:
      if (nread == 0) {
        debug(DEBUG_INFO, "NetMgr", "NetLibReceive read nothing");
        *errP = netErrParamErr;
        r = -1;
      } else {
        debug(DEBUG_INFO, "NetMgr", "NetLibReceive read ok");
        *errP = 0;
        r = nread;
      }
      break;
  }

  return r;
}

Int16 NetLibDmReceive(UInt16 libRefNum, NetSocketRef socket, void *recordP, UInt32 recordOffset, UInt16 rcvLen, UInt16 flags, void *fromAddrP, UInt16 *fromLenP, Int32 timeout, Err *errP) {
  UInt8 *p = (UInt8 *)recordP + recordOffset;
  return NetLibReceive(libRefNum, socket, p, rcvLen, flags, fromAddrP, fromLenP, timeout, errP);
}

/*
typedef enum {
  netSocketTypeStream=1,
  netSocketTypeDatagram=2,
  netSocketTypeRaw=3,
  netSocketTypeReliableMsg=4
} NetSocketTypeEnum
*/

NetSocketRef NetLibSocketOpen(UInt16 libRefnum, NetSocketAddrEnum domain, NetSocketTypeEnum type, Int16 protocol, Int32 timeout, Err *errP) {
  NetSocketRef socket = -1;

  *errP = netErrParamErr;

  if (domain != netSocketAddrINET) {
    debug(DEBUG_ERROR, "NetMgr", "NetLibSocketOpen domain %d not supported", domain);
    return -1;
  }

  switch (type) {
    case netSocketTypeStream:
      type = IP_STREAM;
      break;
    case netSocketTypeDatagram:
      type = IP_DGRAM;
      break;
    default:
      debug(DEBUG_ERROR, "NetMgr", "NetLibSocketOpen type %d not supported", type);
      return -1;
  }

  // protocol: Protocol to use. This parameter is currently ignored by PalmOS.
  // timeout: XXX it is being ignored.

  socket = sys_socket_open(type, 0);
  if (socket != -1) *errP = 0;

  return socket;
}

Int16 NetLibSocketClose(UInt16 libRefnum, NetSocketRef socket, Int32 timeout, Err *errP) {
  int r;

  r = sys_close(socket);
  *errP = (r == 0) ? 0 : netErrParamErr;

  return r;
}

Int16 NetLibSocketConnect(UInt16 libRefnum, NetSocketRef socket, NetSocketAddrType *sockAddrP, Int16 addrLen, Int32 timeout, Err *errP) {
  NetSocketAddrINType *inAddr;
  char host[32];
  int port, r = -1;

  *errP = netErrParamErr;

  if (sockAddrP->family == netSocketAddrINET) {
    inAddr = (NetSocketAddrINType *)sockAddrP;
    NetLibAddrINToA(libRefnum, inAddr->addr, host);
    port = NetNToHS(inAddr->port);
    r = sys_socket_connect(socket, host, port);
    if (r == 0) *errP = 0;
  } else {
    debug(DEBUG_ERROR, "NetMgr", "NetLibSocketConnect family %d not supported", sockAddrP->family);
  }

  return r;
}

Int16 NetLibSocketBind(UInt16 libRefnum, NetSocketRef socket, NetSocketAddrType *sockAddrP, Int16 addrLen, Int32 timeout, Err *errP) {
  NetSocketAddrINType *inAddr;
  char host[32];
  int port, r = -1;

  *errP = netErrParamErr;

  if (sockAddrP->family == netSocketAddrINET) {
    inAddr = (NetSocketAddrINType *)sockAddrP;
    NetLibAddrINToA(libRefnum, inAddr->addr, host);
    port = NetNToHS(inAddr->port);
    r = sys_socket_binds(socket, host, &port);
    if (r == 0) *errP = 0;
  } else {
    debug(DEBUG_ERROR, "NetMgr", "NetLibSocketBind family %d not supported", sockAddrP->family);
  }

  return r;
}

Int16 NetLibSocketListen(UInt16 libRefnum, NetSocketRef socket, UInt16 queueLen, Int32 timeout, Err *errP) {
  int r;

  r = sys_socket_listen(socket, queueLen);
  *errP = (r == 0) ? 0 : netErrParamErr;

  return r;
}

Int16 NetLibSocketAccept(UInt16 libRefnum, NetSocketRef socket, NetSocketAddrType *sockAddrP, Int16 *addrLenP, Int32 timeout, Err *errP) {
  NetSocketAddrINType *inAddr;
  struct timeval tv;
  char host[32];
  int port, r;

  if (timeout >= 0) {
    tv.tv_sec = 0;
    tv.tv_usec = timeout * (1000000 / SysTicksPerSecond());
  }

  MemSet(host, sizeof(host), 0);
  r = sys_socket_accept(socket, host, sizeof(host)-1, &port, timeout < 0 ? NULL : &tv);

  if (r == 0) {
    if (sockAddrP && addrLenP && *addrLenP >= sizeof(NetSocketAddrINType)) {
      inAddr = (NetSocketAddrINType *)sockAddrP;
      inAddr->family = netSocketAddrINET;
      inAddr->port = NetNToHS(port);
      inAddr->addr = NetLibAddrAToIN(libRefnum, host);
      *errP = 0;
    }
  } else {
    *errP = netErrParamErr;
  }

  return r;
}

// Opens a TCP (streams-based) socket and connects it to a server.
// This function is the equivalent of calling NetLibSocketOpen and NetLibSocketConnect.
// serviceName: The name of a network service. Possible services are “echo”, “discard”, “daytime”,
// “qotd”, “chargen”, “ftp-data”, “ftp”, “telnet”, “smtp”, “time”, “name”, “finger”, “pop2”,
// “pop3”, “nntp”, “imap2”. The value of this parameter is ignored if the port parameter is greater than zero.

NetSocketRef NetUTCPOpen(const Char *hostName, const Char *serviceName, Int16 port) {
  NetSocketRef socket = -1;

  if (hostName) {
    if (port > 0) {
      socket = sys_socket_open_connect((char *)hostName, port, IP_STREAM);
    } else {
      debug(DEBUG_ERROR, "NetMgr", "NetUTCPOpen serviceName %s not supported", serviceName ? serviceName : "NULL");
    }
  }

  return socket;
}

// This function repeatedly calls NetLibReceive until numBytes have been read or until NetLibReceive returns an error.

Int32 NetUReadN(NetSocketRef fd, UInt8 *bufP, UInt32 numBytes) {
  Int32 nread, r;
  Err err;

  // XXX timeout == 10
  for (nread = 0; nread < numBytes;) {
    r = NetLibReceive(NetLibRefNum, fd, &bufP[nread], numBytes-nread, 0, NULL, NULL, 10, &err);
    if (r <= 0) break;
    nread += r;
  }

  return nread;
}

Int32 NetUWriteN(NetSocketRef fd, UInt8 *bufP, UInt32 numBytes) {
  Err err;

  // XXX timeout == 0
  return NetLibSend(NetLibRefNum, fd, bufP, numBytes, 0, NULL, 0, 0, &err);
}

Int16 NetLibSelect(UInt16 libRefNum, UInt16 width, NetFDSetType *readFDs, NetFDSetType *writeFDs, NetFDSetType *exceptFDs, Int32 timeout, Err *errP) {
  struct timeval tv;
  Boolean hasUserEvent;
  Int16 r;

  if (readFDs && netFDIsSet(sysFileDescStdIn, readFDs)) {
    // In the Palm OS environment, a descriptor is either a NetSocketRef or the "stdin" descriptor, sysFileDescStdIn.
    // The sysFileDescStdIn descriptor will be ready for input whenever a user event is available like a pen tap or key press.
    netFDClr(sysFileDescStdIn, readFDs);
    hasUserEvent = EvtSysEventAvail(true);
  } else {
    hasUserEvent = false;
  }

  // timeout: maximum timeout in system ticks; -1 means wait forever.
  if (timeout >= 0) {
    tv.tv_sec = 0;
    tv.tv_usec = timeout * (1000000 / SysTicksPerSecond());
  }

  debug(DEBUG_INFO, "NetMgr", "NetLibSelect timeout=%d ...", timeout);
  r = sys_select_fds(width, readFDs, writeFDs, exceptFDs, timeout < 0 ? NULL : &tv);
  debug(DEBUG_INFO, "NetMgr", "NetLibSelect r=%d", r);

  if (hasUserEvent && r >= 0) {
    netFDSet(sysFileDescStdIn, readFDs);
    r++;
  }

  if (errP && r == -1) {
    *errP = netErrSocketNotOpen; // XXX which error should be set here ? It can not be netErrTimeout, because timeout is signaled by returning 0
  }

  return r;
}

Int16 NetLibSocketOptionGet(UInt16 libRefnum, NetSocketRef socket, UInt16 level, UInt16 option, void *optValueP, UInt16 *optValueLenP, Int32 timeout, Err *errP) {
  debug(DEBUG_ERROR, "NetMgr", "NetLibSocketOptionGet not implemented");
  return -1;
}

Int16 NetLibSocketOptionSet(UInt16 libRefnum, NetSocketRef socket, UInt16 level, UInt16 option, void *optValueP, UInt16 optValueLen, Int32 timeout, Err *errP) {
  int ilevel, ioption, r;

  switch (level) {
    case netSocketOptLevelIP:
      ilevel = SYS_LEVEL_IP;
      switch (option) {
        case netSocketOptIPOptions:       // options in IP header (IP_OPTIONS)
        default:
          if (errP) *errP = netErrParamErr;
          return -1;
      }
      break;
    case netSocketOptLevelTCP:
      ilevel = SYS_LEVEL_TCP;
      switch (option) {
        case netSocketOptTCPNoDelay:      // don't delay send to coalesce packets
          ioption = SYS_TCP_NODELAY;
          break;
        case netSocketOptTCPMaxSeg:       // TCP maximum segment size (TCP_MAXSEG)
        default:
          if (errP) *errP = netErrParamErr;
          return -1;
      }
      break;
    case netSocketOptLevelSocket:
      ilevel = SYS_LEVEL_SOCK;
      switch (option) {
        case netSocketOptSockDebug:       // turn on debugging info recording
        case netSocketOptSockAcceptConn:  // socket has had listen
        case netSocketOptSockReuseAddr:   // allow local address reuse
        case netSocketOptSockKeepAlive:   // keep connections alive
        case netSocketOptSockDontRoute:   // just use interface addresses
        case netSocketOptSockBroadcast:   // permit sending of broadcast msgs
        case netSocketOptSockUseLoopback: // bypass hardware when possible
        case netSocketOptSockLinger:      // linger on close if data present
        case netSocketOptSockOOBInLine:   // leave received OutOfBand data in line
        case netSocketOptSockSndBufSize:  // send buffer size
        case netSocketOptSockRcvBufSize:  // receive buffer size
        case netSocketOptSockSndLowWater: // send low-water mark
        case netSocketOptSockRcvLowWater: // receive low-water mark
        case netSocketOptSockSndTimeout:  // send timeout
        case netSocketOptSockRcvTimeout:  // receive timeout
        case netSocketOptSockErrorStatus: // get error status and clear
        case netSocketOptSockSocketType:  // get socket type

        // The following are Pilot specific options
        case netSocketOptSockNonBlocking:     // set non-blocking mode on or off
        case netSocketOptSockRequireErrClear: // return error from all further calls to socket unless netSocketOptSockErrorStatus is cleared.
        case netSocketOptSockMultiPktAddr:    // for SOCK_RDM (RMP) sockets. This is the fixed IP addr (i.e. Mobitex MAN #) to use for multiple packet requests.

        // for socket notification
        case netSocketOptSockNotice:       // prime socket for notification

        default:
          if (errP) *errP = netErrParamErr;
          return -1;
      }
      break;
    default:
      if (errP) *errP = netErrParamErr;
      return -1;
  }

  if ((r = sys_setsockopt(socket, ilevel, ioption, optValueP, optValueLen)) == -1) {
    if (errP) *errP = netErrParamErr;
  }

  return r;
}

Int16 NetLibGetMailExchangeByName(UInt16 libRefNum, Char *mailNameP, UInt16 maxEntries, Char hostNames[][255+1], UInt16 priorities[], Int32 timeout, Err *errP) {
  debug(DEBUG_ERROR, "NetMgr", "NetLibGetMailExchangeByName not implemented");
  return -1;
}

NetServInfoPtr NetLibGetServByName(UInt16 libRefNum, const Char *servNameP, const Char *protoNameP, NetServInfoBufPtr bufP, Int32 timeout, Err *errP) {
  debug(DEBUG_ERROR, "NetMgr", "NetLibGetServByName not implemented");
  return NULL;
}

Err NetLibIFSettingGet(UInt16 libRefNum, UInt32 ifCreator, UInt16 ifInstance, UInt16 setting, void *valueP, UInt16 *valueLenP) {
  debug(DEBUG_ERROR, "NetMgr", "NetLibIFSettingGet not implemented");
  return netErrParamErr;
}

Err NetLibIFSettingSet(UInt16 libRefNum, UInt32 ifCreator, UInt16 ifInstance, UInt16 setting, void *valueP, UInt16 valueLen) {
  debug(DEBUG_ERROR, "NetMgr", "NetLibIFSettingSet not implemented");
  return netErrParamErr;
}

Err NetLibMaster(UInt16 libRefNum, UInt16 cmd, NetMasterPBPtr pbP, Int32 timeout) {
  debug(DEBUG_ERROR, "NetMgr", "NetLibMaster not implemented");
  return netErrParamErr;
}

Err NetLibOpenConfig(UInt16 refNum, UInt16 configIndex, UInt32 openFlags, UInt16 *netIFErrP) {
  Err err = netErrConfigNotFound;

  if (configIndex == 0) {
    if (netIFErrP) *netIFErrP = errNone;
    err = errNone;
  }

  return err;
}

Err NetLibConfigAliasGet(UInt16 refNum, UInt16 aliasIndex, UInt16 *indexP, Boolean *isAnotherAliasP) {
  if (indexP) *indexP = 0;
  if (isAnotherAliasP) *isAnotherAliasP = 0;
  return netErrConfigNotAlias;
}

Err NetLibConfigAliasSet(UInt16 refNum, UInt16 configIndex, UInt16 aliasToIndex) {
  return netErrConfigNotAlias;
}

Err NetLibConfigDelete(UInt16 refNum, UInt16 index) {
  return netErrConfigCantDelete;
}

/*
typedef struct {
  Char name[netConfigNameSize]; // name of configuration
} NetConfigNameType;
*/

Err NetLibConfigIndexFromName(UInt16 refNum, NetConfigNamePtr nameP, UInt16 *indexP) {
  Err err = netErrConfigNotFound;

  if (nameP && indexP && !StrCompare(nameP->name, NetLibIfName)) {
    *indexP = 0;
    err = errNone;
  }

  return err;
}

Err NetLibConfigList(UInt16 refNum, NetConfigNameType nameArray[], UInt16 *arrayEntriesP) {
  Err err = netErrOutOfCmdBlocks;

  if (nameArray && arrayEntriesP && *arrayEntriesP > 0) {
    StrNCopy(nameArray[0].name, NetLibIfName, netConfigNameSize);
    *arrayEntriesP = 1;
    err = errNone;
  }

  return err;
}

Err NetLibConfigMakeActive(UInt16 refNum, UInt16 configIndex) {
  Err err = netErrConfigNotFound;

  if (configIndex == 0) {
    err = errNone;
  }

  return err;
}

Err NetLibConfigRename(UInt16 refNum, UInt16 index, NetConfigNamePtr newNameP) {
  return netErrConfigCantDelete;
}

Err NetLibConfigSaveAs(UInt16 refNum, NetConfigNamePtr nameP) {
  return netErrConfigTooMany;
}

/*
typedef struct {
  UInt8 *addrP;
  UInt16 addrLen;
  NetIOVecType *iov;
  UInt16 iovLen;
  UInt8 *accessRights;
  UInt16 accessRightsLen;
} NetIOParamType;

typedef struct {
  UInt8 *bufP;   // buffer address
  UInt16 bufLen; // buffer length
} NetIOVecType;
*/

Int16 NetLibReceivePB(UInt16 libRefNum, NetSocketRef socket, NetIOParamType *pbP, UInt16 flags, Int32 timeout, Err *errP) {
  Int16 n, nread = -1;
  UInt16 i;

  if (pbP) {
    nread = 0;
    for (i = 0; i < pbP->iovLen; i++) {
      n = NetLibReceive(libRefNum, socket, pbP->iov[i].bufP, pbP->iov[i].bufLen, flags, pbP->addrP, &pbP->addrLen, timeout, errP);
      if (n <= 0) break;
      nread += n;
    }
  }

  if (errP) *errP = (nread == -1) ? netErrParamErr : errNone;

  return nread;
}

Int16 NetLibSendPB(UInt16 libRefNum, NetSocketRef socket, NetIOParamType *pbP, UInt16 flags, Int32 timeout, Err *errP) {
  Int16 n, nwritten = -1;
  UInt16 i;

  if (pbP) {
    nwritten = 0;
    for (i = 0; i < pbP->iovLen; i++) {
      n = NetLibSend(libRefNum, socket, pbP->iov[i].bufP, pbP->iov[i].bufLen, flags, pbP->addrP, pbP->addrLen, timeout, errP);
      if (n <= 0) break;
      nwritten += n;
    }
  }

  if (errP) *errP = (nwritten == -1) ? netErrParamErr : errNone;

  return nwritten;
}

Err NetLibSettingGet(UInt16 libRefNum, UInt16 setting, void *valueP, UInt16 *valueLenP) {
  debug(DEBUG_ERROR, "NetMgr", "NetLibSettingGet not implemented");
  return netErrUnknownSetting;
}

Err NetLibSettingSet(UInt16 libRefNum, UInt16 setting, void *valueP, UInt16 valueLen) {
  debug(DEBUG_ERROR, "NetMgr", "NetLibSettingSet not implemented");
  return netErrUnknownSetting;
}

Int16 NetLibSocketAddr(UInt16 libRefnum, NetSocketRef socketRef, NetSocketAddrType *locAddrP, Int16 *locAddrLenP, NetSocketAddrType *remAddrP, Int16 *remAddrLenP, Int32 timeout, Err *errP) {
  debug(DEBUG_ERROR, "NetMgr", "NetLibSocketAddr not implemented");
  if (errP) *errP = netErrParamErr;
  return -1;
}

Int16 NetLibSocketShutdown(UInt16 libRefnum, NetSocketRef socket, Int16 direction, Int32 timeout, Err *errP) {
  Err err = netErrParamErr;
  Int16 r = -1;

  switch (direction) {
    case netSocketDirInput:   direction = SYS_SHUTDOWN_RD;
    case netSocketDirOutput:  direction = SYS_SHUTDOWN_WR;
    case netSocketDirBoth:    direction = SYS_SHUTDOWN_RDWR;
    default: direction = -1;
  }

  if (direction != -1) {
    r = sys_socket_shutdown(socket, direction); 
    if (r == 0) err = errNone;
  }

  if (errP) *errP = err;

  return r;
}
