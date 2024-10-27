    case netLibConfigAliasGet: {
      UInt16 refNum = sys_va_arg(ap, UInt32);
      UInt16 aliasIndex = sys_va_arg(ap, UInt32);
      UInt16 *indexP = sys_va_arg(ap, UInt16 *);
      Boolean *isAnotherAliasP = sys_va_arg(ap, Boolean *);
      Err ret = NetLibConfigAliasGet(refNum, aliasIndex, indexP, isAnotherAliasP);
      *iret = ret;
      }
      break;

    case netLibConfigAliasSet: {
      UInt16 refNum = sys_va_arg(ap, UInt32);
      UInt16 configIndex = sys_va_arg(ap, UInt32);
      UInt16 aliasToIndex = sys_va_arg(ap, UInt32);
      Err ret = NetLibConfigAliasSet(refNum, configIndex, aliasToIndex);
      *iret = ret;
      }
      break;

    case netLibConfigDelete: {
      UInt16 refNum = sys_va_arg(ap, UInt32);
      UInt16 index = sys_va_arg(ap, UInt32);
      Err ret = NetLibConfigDelete(refNum, index);
      *iret = ret;
      }
      break;

    case netLibConfigIndexFromName: {
      UInt16 refNum = sys_va_arg(ap, UInt32);
      NetConfigNamePtr nameP = sys_va_arg(ap, NetConfigNamePtr);
      UInt16 *indexP = sys_va_arg(ap, UInt16 *);
      Err ret = NetLibConfigIndexFromName(refNum, nameP, indexP);
      *iret = ret;
      }
      break;

    case netLibConfigList: {
      UInt16 refNum = sys_va_arg(ap, UInt32);
      NetConfigNameType *nameArray = sys_va_arg(ap, NetConfigNameType *);
      UInt16 *arrayEntriesP = sys_va_arg(ap, UInt16 *);
      Err ret = NetLibConfigList(refNum, nameArray, arrayEntriesP);
      *iret = ret;
      }
      break;

    case netLibConfigMakeActive: {
      UInt16 refNum = sys_va_arg(ap, UInt32);
      UInt16 configIndex = sys_va_arg(ap, UInt32);
      Err ret = NetLibConfigMakeActive(refNum, configIndex);
      *iret = ret;
      }
      break;

    case netLibConfigRename: {
      UInt16 refNum = sys_va_arg(ap, UInt32);
      UInt16 index = sys_va_arg(ap, UInt32);
      NetConfigNamePtr newNameP = sys_va_arg(ap, NetConfigNamePtr);
      Err ret = NetLibConfigRename(refNum, index, newNameP);
      *iret = ret;
      }
      break;

    case netLibConfigSaveAs: {
      UInt16 refNum = sys_va_arg(ap, UInt32);
      NetConfigNamePtr nameP = sys_va_arg(ap, NetConfigNamePtr);
      Err ret = NetLibConfigSaveAs(refNum, nameP);
      *iret = ret;
      }
      break;

    case netLibOpenConfig: {
      UInt16 refNum = sys_va_arg(ap, UInt32);
      UInt16 configIndex = sys_va_arg(ap, UInt32);
      UInt32 openFlags = sys_va_arg(ap, UInt32);
      UInt16 *netIFErrP = sys_va_arg(ap, UInt16 *);
      Err ret = NetLibOpenConfig(refNum, configIndex, openFlags, netIFErrP);
      *iret = ret;
      }
      break;

    case netLibTrapAddrAToIN: {
      UInt16 libRefnum = sys_va_arg(ap, UInt32);
      const Char *a = sys_va_arg(ap, Char *);
      NetIPAddr ret = NetLibAddrAToIN(libRefnum, a);
      *iret = ret;
      }
      break;

    case netLibTrapAddrINToA: {
      UInt16 libRefnum = sys_va_arg(ap, UInt32);
      NetIPAddr inet = sys_va_arg(ap, NetIPAddr);
      Char *spaceP = sys_va_arg(ap, Char *);
      Char *ret = NetLibAddrINToA(libRefnum, inet, spaceP);
      *pret = (void *)ret;
      }
      break;

    case netLibTrapBitGetFixed: {
      UInt16 libRefNum = sys_va_arg(ap, UInt32);
      UInt8 *dstP = sys_va_arg(ap, UInt8 *);
      UInt32 *dstBitOffsetP = sys_va_arg(ap, UInt32 *);
      UInt16 numBits = sys_va_arg(ap, UInt32);
      UInt32 ret = NetLibBitGetFixed(libRefNum, dstP, dstBitOffsetP, numBits);
      *iret = ret;
      }
      break;

    case netLibTrapBitGetUIntV: {
      UInt16 libRefNum = sys_va_arg(ap, UInt32);
      UInt8 *dstP = sys_va_arg(ap, UInt8 *);
      UInt32 *dstBitOffsetP = sys_va_arg(ap, UInt32 *);
      UInt32 ret = NetLibBitGetUIntV(libRefNum, dstP, dstBitOffsetP);
      *iret = ret;
      }
      break;

    case netLibTrapBitMove: {
      UInt16 libRefNum = sys_va_arg(ap, UInt32);
      UInt8 *dstP = sys_va_arg(ap, UInt8 *);
      UInt32 *dstBitOffsetP = sys_va_arg(ap, UInt32 *);
      UInt8 *srcP = sys_va_arg(ap, UInt8 *);
      UInt32 *srcBitOffsetP = sys_va_arg(ap, UInt32 *);
      UInt32 numBits = sys_va_arg(ap, UInt32);
      NetLibBitMove(libRefNum, dstP, dstBitOffsetP, srcP, srcBitOffsetP, numBits);
      }
      break;

    case netLibTrapBitPutFixed: {
      UInt16 libRefNum = sys_va_arg(ap, UInt32);
      UInt8 *dstP = sys_va_arg(ap, UInt8 *);
      UInt32 *dstBitOffsetP = sys_va_arg(ap, UInt32 *);
      UInt32 value = sys_va_arg(ap, UInt32);
      UInt16 numBits = sys_va_arg(ap, UInt32);
      NetLibBitPutFixed(libRefNum, dstP, dstBitOffsetP, value, numBits);
      }
      break;

    case netLibTrapBitPutUIntV: {
      UInt16 libRefNum = sys_va_arg(ap, UInt32);
      UInt8 *dstP = sys_va_arg(ap, UInt8 *);
      UInt32 *dstBitOffsetP = sys_va_arg(ap, UInt32 *);
      UInt32 value = sys_va_arg(ap, UInt32);
      NetLibBitPutUIntV(libRefNum, dstP, dstBitOffsetP, value);
      }
      break;

    case netLibTrapConnectionRefresh: {
      UInt16 refNum = sys_va_arg(ap, UInt32);
      Boolean refresh = sys_va_arg(ap, UInt32);
      UInt8 *allInterfacesUpP = sys_va_arg(ap, UInt8 *);
      UInt16 *netIFErrP = sys_va_arg(ap, UInt16 *);
      Err ret = NetLibConnectionRefresh(refNum, refresh, allInterfacesUpP, netIFErrP);
      *iret = ret;
      }
      break;

    case netLibTrapDmReceive: {
      UInt16 libRefNum = sys_va_arg(ap, UInt32);
      NetSocketRef socket = sys_va_arg(ap, NetSocketRef);
      void *recordP = sys_va_arg(ap, void *);
      UInt32 recordOffset = sys_va_arg(ap, UInt32);
      UInt16 rcvLen = sys_va_arg(ap, UInt32);
      UInt16 flags = sys_va_arg(ap, UInt32);
      void *fromAddrP = sys_va_arg(ap, void *);
      UInt16 *fromLenP = sys_va_arg(ap, UInt16 *);
      Int32 timeout = sys_va_arg(ap, Int32);
      Err *errP = sys_va_arg(ap, Err *);
      Int16 ret = NetLibDmReceive(libRefNum, socket, recordP, recordOffset, rcvLen, flags, fromAddrP, fromLenP, timeout, errP);
      *iret = ret;
      }
      break;

    case netLibTrapFinishCloseWait: {
      UInt16 libRefnum = sys_va_arg(ap, UInt32);
      Err ret = NetLibFinishCloseWait(libRefnum);
      *iret = ret;
      }
      break;

    case netLibTrapGetHostByAddr: {
      UInt16 libRefNum = sys_va_arg(ap, UInt32);
      UInt8 *addrP = sys_va_arg(ap, UInt8 *);
      UInt16 len = sys_va_arg(ap, UInt32);
      UInt16 type = sys_va_arg(ap, UInt32);
      NetHostInfoBufPtr bufP = sys_va_arg(ap, NetHostInfoBufPtr);
      Int32 timeout = sys_va_arg(ap, Int32);
      Err *errP = sys_va_arg(ap, Err *);
      NetHostInfoPtr ret = NetLibGetHostByAddr(libRefNum, addrP, len, type, bufP, timeout, errP);
      *pret = (void *)ret;
      }
      break;

    case netLibTrapGetHostByName: {
      UInt16 libRefNum = sys_va_arg(ap, UInt32);
      const Char *nameP = sys_va_arg(ap, Char *);
      NetHostInfoBufPtr bufP = sys_va_arg(ap, NetHostInfoBufPtr);
      Int32 timeout = sys_va_arg(ap, Int32);
      Err *errP = sys_va_arg(ap, Err *);
      NetHostInfoPtr ret = NetLibGetHostByName(libRefNum, nameP, bufP, timeout, errP);
      *pret = (void *)ret;
      }
      break;

    case netLibTrapGetServByName: {
      UInt16 libRefNum = sys_va_arg(ap, UInt32);
      const Char *servNameP = sys_va_arg(ap, Char *);
      const Char *protoNameP = sys_va_arg(ap, Char *);
      NetServInfoBufPtr bufP = sys_va_arg(ap, NetServInfoBufPtr);
      Int32 timeout = sys_va_arg(ap, Int32);
      Err *errP = sys_va_arg(ap, Err *);
      NetServInfoPtr ret = NetLibGetServByName(libRefNum, servNameP, protoNameP, bufP, timeout, errP);
      *pret = (void *)ret;
      }
      break;

    case netLibTrapHandlePowerOff: {
      UInt16 refNum = sys_va_arg(ap, UInt32);
      SysEventType *eventP = sys_va_arg(ap, SysEventType *);
      Err ret = NetLibHandlePowerOff(refNum, eventP);
      *iret = ret;
      }
      break;

    case netLibTrapIFAttach: {
      UInt16 libRefNum = sys_va_arg(ap, UInt32);
      UInt32 ifCreator = sys_va_arg(ap, UInt32);
      UInt16 ifInstance = sys_va_arg(ap, UInt32);
      Int32 timeout = sys_va_arg(ap, Int32);
      Err ret = NetLibIFAttach(libRefNum, ifCreator, ifInstance, timeout);
      *iret = ret;
      }
      break;

    case netLibTrapIFDetach: {
      UInt16 libRefNum = sys_va_arg(ap, UInt32);
      UInt32 ifCreator = sys_va_arg(ap, UInt32);
      UInt16 ifInstance = sys_va_arg(ap, UInt32);
      Int32 timeout = sys_va_arg(ap, Int32);
      Err ret = NetLibIFDetach(libRefNum, ifCreator, ifInstance, timeout);
      *iret = ret;
      }
      break;

    case netLibTrapIFDown: {
      UInt16 libRefNum = sys_va_arg(ap, UInt32);
      UInt32 ifCreator = sys_va_arg(ap, UInt32);
      UInt16 ifInstance = sys_va_arg(ap, UInt32);
      Int32 timeout = sys_va_arg(ap, Int32);
      Err ret = NetLibIFDown(libRefNum, ifCreator, ifInstance, timeout);
      *iret = ret;
      }
      break;

    case netLibTrapIFGet: {
      UInt16 libRefNum = sys_va_arg(ap, UInt32);
      UInt16 index = sys_va_arg(ap, UInt32);
      UInt32 *ifCreatorP = sys_va_arg(ap, UInt32 *);
      UInt16 *ifInstanceP = sys_va_arg(ap, UInt16 *);
      Err ret = NetLibIFGet(libRefNum, index, ifCreatorP, ifInstanceP);
      *iret = ret;
      }
      break;

    case netLibTrapIFSettingGet: {
      UInt16 libRefNum = sys_va_arg(ap, UInt32);
      UInt32 ifCreator = sys_va_arg(ap, UInt32);
      UInt16 ifInstance = sys_va_arg(ap, UInt32);
      UInt16 setting = sys_va_arg(ap, UInt32);
      void *valueP = sys_va_arg(ap, void *);
      UInt16 *valueLenP = sys_va_arg(ap, UInt16 *);
      Err ret = NetLibIFSettingGet(libRefNum, ifCreator, ifInstance, setting, valueP, valueLenP);
      *iret = ret;
      }
      break;

    case netLibTrapIFSettingSet: {
      UInt16 libRefNum = sys_va_arg(ap, UInt32);
      UInt32 ifCreator = sys_va_arg(ap, UInt32);
      UInt16 ifInstance = sys_va_arg(ap, UInt32);
      UInt16 setting = sys_va_arg(ap, UInt32);
      void *valueP = sys_va_arg(ap, void *);
      UInt16 valueLen = sys_va_arg(ap, UInt32);
      Err ret = NetLibIFSettingSet(libRefNum, ifCreator, ifInstance, setting, valueP, valueLen);
      *iret = ret;
      }
      break;

    case netLibTrapIFUp: {
      UInt16 libRefNum = sys_va_arg(ap, UInt32);
      UInt32 ifCreator = sys_va_arg(ap, UInt32);
      UInt16 ifInstance = sys_va_arg(ap, UInt32);
      Err ret = NetLibIFUp(libRefNum, ifCreator, ifInstance);
      *iret = ret;
      }
      break;

    case netLibTrapMaster: {
      UInt16 libRefNum = sys_va_arg(ap, UInt32);
      UInt16 cmd = sys_va_arg(ap, UInt32);
      NetMasterPBPtr pbP = sys_va_arg(ap, NetMasterPBPtr);
      Int32 timeout = sys_va_arg(ap, Int32);
      Err ret = NetLibMaster(libRefNum, cmd, pbP, timeout);
      *iret = ret;
      }
      break;

    case netLibTrapOpenCount: {
      UInt16 refNum = sys_va_arg(ap, UInt32);
      UInt16 *countP = sys_va_arg(ap, UInt16 *);
      Err ret = NetLibOpenCount(refNum, countP);
      *iret = ret;
      }
      break;

    case netLibTrapOpenIfCloseWait: {
      UInt16 libRefnum = sys_va_arg(ap, UInt32);
      Err ret = NetLibOpenIfCloseWait(libRefnum);
      *iret = ret;
      }
      break;

    case netLibTrapReceive: {
      UInt16 libRefNum = sys_va_arg(ap, UInt32);
      NetSocketRef socket = sys_va_arg(ap, NetSocketRef);
      void *bufP = sys_va_arg(ap, void *);
      UInt16 bufLen = sys_va_arg(ap, UInt32);
      UInt16 flags = sys_va_arg(ap, UInt32);
      void *fromAddrP = sys_va_arg(ap, void *);
      UInt16 *fromLenP = sys_va_arg(ap, UInt16 *);
      Int32 timeout = sys_va_arg(ap, Int32);
      Err *errP = sys_va_arg(ap, Err *);
      Int16 ret = NetLibReceive(libRefNum, socket, bufP, bufLen, flags, fromAddrP, fromLenP, timeout, errP);
      *iret = ret;
      }
      break;

    case netLibTrapReceivePB: {
      UInt16 libRefNum = sys_va_arg(ap, UInt32);
      NetSocketRef socket = sys_va_arg(ap, NetSocketRef);
      NetIOParamType *pbP = sys_va_arg(ap, NetIOParamType *);
      UInt16 flags = sys_va_arg(ap, UInt32);
      Int32 timeout = sys_va_arg(ap, Int32);
      Err *errP = sys_va_arg(ap, Err *);
      Int16 ret = NetLibReceivePB(libRefNum, socket, pbP, flags, timeout, errP);
      *iret = ret;
      }
      break;

    case netLibTrapSelect: {
      UInt16 libRefNum = sys_va_arg(ap, UInt32);
      UInt16 width = sys_va_arg(ap, UInt32);
      NetFDSetType *readFDs = sys_va_arg(ap, NetFDSetType *);
      NetFDSetType *writeFDs = sys_va_arg(ap, NetFDSetType *);
      NetFDSetType *exceptFDs = sys_va_arg(ap, NetFDSetType *);
      Int32 timeout = sys_va_arg(ap, Int32);
      Err *errP = sys_va_arg(ap, Err *);
      Int16 ret = NetLibSelect(libRefNum, width, readFDs, writeFDs, exceptFDs, timeout, errP);
      *iret = ret;
      }
      break;

    case netLibTrapSend: {
      UInt16 libRefNum = sys_va_arg(ap, UInt32);
      NetSocketRef socket = sys_va_arg(ap, NetSocketRef);
      void *bufP = sys_va_arg(ap, void *);
      UInt16 bufLen = sys_va_arg(ap, UInt32);
      UInt16 flags = sys_va_arg(ap, UInt32);
      void *toAddrP = sys_va_arg(ap, void *);
      UInt16 toLen = sys_va_arg(ap, UInt32);
      Int32 timeout = sys_va_arg(ap, Int32);
      Err *errP = sys_va_arg(ap, Err *);
      Int16 ret = NetLibSend(libRefNum, socket, bufP, bufLen, flags, toAddrP, toLen, timeout, errP);
      *iret = ret;
      }
      break;

    case netLibTrapSendPB: {
      UInt16 libRefNum = sys_va_arg(ap, UInt32);
      NetSocketRef socket = sys_va_arg(ap, NetSocketRef);
      NetIOParamType *pbP = sys_va_arg(ap, NetIOParamType *);
      UInt16 flags = sys_va_arg(ap, UInt32);
      Int32 timeout = sys_va_arg(ap, Int32);
      Err *errP = sys_va_arg(ap, Err *);
      Int16 ret = NetLibSendPB(libRefNum, socket, pbP, flags, timeout, errP);
      *iret = ret;
      }
      break;

    case netLibTrapSettingGet: {
      UInt16 libRefNum = sys_va_arg(ap, UInt32);
      UInt16 setting = sys_va_arg(ap, UInt32);
      void *valueP = sys_va_arg(ap, void *);
      UInt16 *valueLenP = sys_va_arg(ap, UInt16 *);
      Err ret = NetLibSettingGet(libRefNum, setting, valueP, valueLenP);
      *iret = ret;
      }
      break;

    case netLibTrapSettingSet: {
      UInt16 libRefNum = sys_va_arg(ap, UInt32);
      UInt16 setting = sys_va_arg(ap, UInt32);
      void *valueP = sys_va_arg(ap, void *);
      UInt16 valueLen = sys_va_arg(ap, UInt32);
      Err ret = NetLibSettingSet(libRefNum, setting, valueP, valueLen);
      *iret = ret;
      }
      break;

    case netLibTrapSocketAccept: {
      UInt16 libRefnum = sys_va_arg(ap, UInt32);
      NetSocketRef socket = sys_va_arg(ap, NetSocketRef);
      NetSocketAddrType *sockAddrP = sys_va_arg(ap, NetSocketAddrType *);
      Int16 *addrLenP = sys_va_arg(ap, Int16 *);
      Int32 timeout = sys_va_arg(ap, Int32);
      Err *errP = sys_va_arg(ap, Err *);
      Int16 ret = NetLibSocketAccept(libRefnum, socket, sockAddrP, addrLenP, timeout, errP);
      *iret = ret;
      }
      break;

    case netLibTrapSocketAddr: {
      UInt16 libRefnum = sys_va_arg(ap, UInt32);
      NetSocketRef socketRef = sys_va_arg(ap, NetSocketRef);
      NetSocketAddrType *locAddrP = sys_va_arg(ap, NetSocketAddrType *);
      Int16 *locAddrLenP = sys_va_arg(ap, Int16 *);
      NetSocketAddrType *remAddrP = sys_va_arg(ap, NetSocketAddrType *);
      Int16 *remAddrLenP = sys_va_arg(ap, Int16 *);
      Int32 timeout = sys_va_arg(ap, Int32);
      Err *errP = sys_va_arg(ap, Err *);
      Int16 ret = NetLibSocketAddr(libRefnum, socketRef, locAddrP, locAddrLenP, remAddrP, remAddrLenP, timeout, errP);
      *iret = ret;
      }
      break;

    case netLibTrapSocketBind: {
      UInt16 libRefnum = sys_va_arg(ap, UInt32);
      NetSocketRef socket = sys_va_arg(ap, NetSocketRef);
      NetSocketAddrType *sockAddrP = sys_va_arg(ap, NetSocketAddrType *);
      Int16 addrLen = sys_va_arg(ap, Int32);
      Int32 timeout = sys_va_arg(ap, Int32);
      Err *errP = sys_va_arg(ap, Err *);
      Int16 ret = NetLibSocketBind(libRefnum, socket, sockAddrP, addrLen, timeout, errP);
      *iret = ret;
      }
      break;

    case netLibTrapSocketClose: {
      UInt16 libRefnum = sys_va_arg(ap, UInt32);
      NetSocketRef socket = sys_va_arg(ap, NetSocketRef);
      Int32 timeout = sys_va_arg(ap, Int32);
      Err *errP = sys_va_arg(ap, Err *);
      Int16 ret = NetLibSocketClose(libRefnum, socket, timeout, errP);
      *iret = ret;
      }
      break;

    case netLibTrapSocketConnect: {
      UInt16 libRefnum = sys_va_arg(ap, UInt32);
      NetSocketRef socket = sys_va_arg(ap, NetSocketRef);
      NetSocketAddrType *sockAddrP = sys_va_arg(ap, NetSocketAddrType *);
      Int16 addrLen = sys_va_arg(ap, Int32);
      Int32 timeout = sys_va_arg(ap, Int32);
      Err *errP = sys_va_arg(ap, Err *);
      Int16 ret = NetLibSocketConnect(libRefnum, socket, sockAddrP, addrLen, timeout, errP);
      *iret = ret;
      }
      break;

    case netLibTrapSocketListen: {
      UInt16 libRefnum = sys_va_arg(ap, UInt32);
      NetSocketRef socket = sys_va_arg(ap, NetSocketRef);
      UInt16 queueLen = sys_va_arg(ap, UInt32);
      Int32 timeout = sys_va_arg(ap, Int32);
      Err *errP = sys_va_arg(ap, Err *);
      Int16 ret = NetLibSocketListen(libRefnum, socket, queueLen, timeout, errP);
      *iret = ret;
      }
      break;

    case netLibTrapSocketOpen: {
      UInt16 libRefnum = sys_va_arg(ap, UInt32);
      NetSocketAddrEnum domain = sys_va_arg(ap, NetSocketAddrEnum);
      NetSocketTypeEnum type = sys_va_arg(ap, NetSocketTypeEnum);
      Int16 protocol = sys_va_arg(ap, Int32);
      Int32 timeout = sys_va_arg(ap, Int32);
      Err *errP = sys_va_arg(ap, Err *);
      NetSocketRef ret = NetLibSocketOpen(libRefnum, domain, type, protocol, timeout, errP);
      *iret = ret;
      }
      break;

    case netLibTrapSocketOptionGet: {
      UInt16 libRefnum = sys_va_arg(ap, UInt32);
      NetSocketRef socket = sys_va_arg(ap, NetSocketRef);
      UInt16 level = sys_va_arg(ap, UInt32);
      UInt16 option = sys_va_arg(ap, UInt32);
      void *optValueP = sys_va_arg(ap, void *);
      UInt16 *optValueLenP = sys_va_arg(ap, UInt16 *);
      Int32 timeout = sys_va_arg(ap, Int32);
      Err *errP = sys_va_arg(ap, Err *);
      Int16 ret = NetLibSocketOptionGet(libRefnum, socket, level, option, optValueP, optValueLenP, timeout, errP);
      *iret = ret;
      }
      break;

    case netLibTrapSocketOptionSet: {
      UInt16 libRefnum = sys_va_arg(ap, UInt32);
      NetSocketRef socket = sys_va_arg(ap, NetSocketRef);
      UInt16 level = sys_va_arg(ap, UInt32);
      UInt16 option = sys_va_arg(ap, UInt32);
      void *optValueP = sys_va_arg(ap, void *);
      UInt16 optValueLen = sys_va_arg(ap, UInt32);
      Int32 timeout = sys_va_arg(ap, Int32);
      Err *errP = sys_va_arg(ap, Err *);
      Int16 ret = NetLibSocketOptionSet(libRefnum, socket, level, option, optValueP, optValueLen, timeout, errP);
      *iret = ret;
      }
      break;

    case netLibTrapSocketShutdown: {
      UInt16 libRefnum = sys_va_arg(ap, UInt32);
      NetSocketRef socket = sys_va_arg(ap, NetSocketRef);
      Int16 direction = sys_va_arg(ap, Int32);
      Int32 timeout = sys_va_arg(ap, Int32);
      Err *errP = sys_va_arg(ap, Err *);
      Int16 ret = NetLibSocketShutdown(libRefnum, socket, direction, timeout, errP);
      *iret = ret;
      }
      break;

    case netLibTrapTracePrintF: {
      UInt16 libRefNum = sys_va_arg(ap, UInt32);
      const Char *formatStr = sys_va_arg(ap, Char *);
      Err ret = NetLibTracePrintF(libRefNum, formatStr, ap);
      *iret = ret;
      }
      break;

    case netLibTrapTracePutS: {
      UInt16 libRefNum = sys_va_arg(ap, UInt32);
      Char *strP = sys_va_arg(ap, Char *);
      Err ret = NetLibTracePutS(libRefNum, strP);
      *iret = ret;
      }
      break;

    case sysLibTrapClose: {
      UInt16 libRefnum = sys_va_arg(ap, UInt32);
      UInt16 immediate = sys_va_arg(ap, UInt32);
      Err ret = NetLibClose(libRefnum, immediate);
      *iret = ret;
      }
      break;

    case sysLibTrapOpen: {
      UInt16 libRefnum = sys_va_arg(ap, UInt32);
      UInt16 *netIFErrsP = sys_va_arg(ap, UInt16 *);
      Err ret = NetLibOpen(libRefnum, netIFErrsP);
      *iret = ret;
      }
      break;

    case sysLibTrapSleep: {
      UInt16 libRefnum = sys_va_arg(ap, UInt32);
      Err ret = NetLibSleep(libRefnum);
      *iret = ret;
      }
      break;

    case sysLibTrapWake: {
      UInt16 libRefnum = sys_va_arg(ap, UInt32);
      Err ret = NetLibWake(libRefnum);
      *iret = ret;
      }
      break;

