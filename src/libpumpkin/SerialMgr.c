#include <PalmOS.h>

#include "sys.h"
#include "thread.h"
#include "pwindow.h"
#include "vfs.h"
#include "pumpkin.h"
#include "debug.h"
#include "xalloc.h"

#define SERIAL_BUFFER  512

#define PALMOS_MODULE "Serial"

typedef struct {
  char *descr[MAX_SERIAL];
  uint16_t portId[MAX_SERIAL];
  uint8_t buffer[MAX_SERIAL][SERIAL_BUFFER];
  uint8_t *buf[MAX_SERIAL];
  uint32_t bufSize[MAX_SERIAL];
  uint32_t bufPtr[MAX_SERIAL];
  uint32_t bufLen[MAX_SERIAL];
  uint32_t refCon[MAX_SERIAL];
  uint32_t minBytes[MAX_SERIAL];
  int fg[MAX_SERIAL];
  int bg[MAX_SERIAL];
  WakeupHandlerProcPtr wakeUpProc[MAX_SERIAL];
} srm_module_t;

int SrmInitModule(void) {
  srm_module_t *module;
  int id;

  if ((module = xcalloc(1, sizeof(srm_module_t))) == NULL) {
    return -1;
  }

  for (id = 0; id < MAX_SERIAL; id++) {
    module->bufSize[id] = SERIAL_BUFFER;
    module->buf[id] = module->buffer[id];
  }

  pumpkin_set_local_storage(srm_key, module);

  return 0;
}

int SrmFinishModule(void) {
  srm_module_t *module = (srm_module_t *)pumpkin_get_local_storage(srm_key);

  if (module) {
    xfree(module);
  }

  return 0;
}

Err SerialMgrInstall(void) {
  return errNone;
}

static int SrmCheckPortId(UInt16 portId, Boolean invalidate) {
  srm_module_t *module = (srm_module_t *)pumpkin_get_local_storage(srm_key);
  int id, r = -1;

  if (portId) {
    for (id = 0; id < MAX_SERIAL; id++) {
      if (module->portId[id] == portId) {
        if (invalidate) module->portId[id] = 0;
        r = id;
        break;
      }
    }
  }

  return r;
}

static Err SrmOpenInternal(UInt32 port, UInt32 baud, UInt16 *newPortIdP, Boolean background) {
  srm_module_t *module = (srm_module_t *)pumpkin_get_local_storage(srm_key);
  uint32_t id;
  int portId;

  if (pumpkin_get_serial_by_creator(&id, NULL, port) == -1) {
    return serErrBadPort;
  }

  if (module->portId[id] == 0) {
    if ((portId = pumpkin_open_serial(id)) == -1) {
      return serErrBadPort;
    }

    if (pumpkin_baud_serial(id, baud) == -1) {
      pumpkin_close_serial(id);
      return serErrBadPort;
    }

    if (background) {
      module->bg[id] = 1;
    } else {
      module->fg[id] = 1;
    }

    module->portId[id] = portId;
    *newPortIdP = portId; // XXX int -> UInt16
    return errNone;
  }

  if (background) {
    // other task already opened the port in backgound mode
    return serErrAlreadyOpen;
  }

  if (module->fg[id]) {
    // other task already opened the port in foregound mode
    return serErrAlreadyOpen;
  }

  module->fg[id] = 1;
  *newPortIdP = module->portId[id];

  return errNone;
}

Err SrmOpen(UInt32 port, UInt32 baud, UInt16 *newPortIdP) {
  return SrmOpenInternal(port, baud, newPortIdP, false);
}
 
/*
  SrmOpenConfigType:

  UInt32 baud;     // Baud rate that the connection is to be opened at.
                   // Applications that use drivers that do not require
                   // baud rates can set this to zero or any other value.
                   // Drivers that do not require a baud rate should
                   // ignore this field
  UInt32 function; //  Designates the function of the connection. A value
                   // of zero indictates default behavior for the protocol.
                   // Drivers that do not support multiple functions should
                   // ignore this field.
*/

static void SrmPrintConfig(SrmOpenConfigType *configP) {
  char *s, fn[8];

  switch (configP->function) {
    case serFncUndefined:   s = "undefined"; break;
    case serFncPPPSession:  s = "PPP"; break;
    case serFncSLIPSession: s = "SLIP"; break;
    case serFncHotSync:     s = "HotSync"; break;
    case serFncConsole:     s = "Console"; break;
    case serFncTelephony:   s = "Telephony"; break;
    default:
      pumpkin_id2s(configP->function, fn);
      s = fn;
      break;
  }
  debug(DEBUG_INFO, PALMOS_MODULE, "Serial function %s", s);
}

Err SrmExtOpen(UInt32 port, SrmOpenConfigType *configP, UInt16 configSize, UInt16 *newPortIdP) {
  UInt32 baud = 0;

  if (configP) {
    baud = configP->baud;
    SrmPrintConfig(configP);
  }

  return SrmOpenInternal(port, baud, newPortIdP, false);
}

Err SrmExtOpenBackground(UInt32 port, SrmOpenConfigType *configP, UInt16 configSize, UInt16 *newPortIdP) {
  UInt32 baud = 0;

  if (configP) {
    baud = configP->baud;
    SrmPrintConfig(configP);
  }

  return SrmOpenInternal(port, baud, newPortIdP, true);
}

Err SrmOpenBackground(UInt32 port, UInt32 baud, UInt16 *newPortIdP) {
  return SrmOpenInternal(port, baud, newPortIdP, true);
}

// Closes a serial port and makes it available to other applications,
// regardless of whether the port is a foreground or background port.
Err SrmClose(UInt16 portId) {
  srm_module_t *module = (srm_module_t *)pumpkin_get_local_storage(srm_key);
  int id;
  Err err = serErrBadPort;

  if ((id = SrmCheckPortId(portId, 1)) != -1) {
    module->bufLen[id] = 0;
    module->bufPtr[id] = 0;
    module->bufSize[id] = SERIAL_BUFFER;
    module->buf[id] = module->buffer[id];
    module->refCon[id] = 0;
    module->minBytes[id] = 0;
    module->bg[id] = 0;
    module->fg[id] = 0;

    if (pumpkin_close_serial(id) == 0) {
      err = errNone;
    }
  }

  return err;
}

Err SrmSleep() {
  return errNone;
}

Err SrmWake() {
  return errNone;
}

Err SrmGetDeviceCount(UInt16 *numOfDevicesP) {
  if (numOfDevicesP) {
    *numOfDevicesP = pumpkin_num_serial();
  }
  return errNone;
}

Err SrmGetDeviceInfo(UInt32 deviceID, DeviceInfoType *deviceInfoP) {
  uint32_t creator;
  char *str;
  Err err = serErrBadPort;

  // XXX deviceID: ID of serial device to get information for.
  // You can pass a zero-based index (0, 1, 2, ...), a valid port ID returned from
  // SrmOpen() or SrmExtOpen(), or a 4-character port name (such as 'u328', 'u650', or 'ircm').

  if (pumpkin_get_serial_by_creator(NULL, &str, deviceID) == 0) {
    deviceInfoP->serDevPortInfoStr = pumpkin_heap_alloc(StrLen(str) + 1, "serial_descr");
    StrCopy(deviceInfoP->serDevPortInfoStr, str);
    deviceInfoP->serDevCreator = deviceID;
    deviceInfoP->serDevFtrInfo = 0;
    deviceInfoP->serDevMaxBaudRate = 115200;
    deviceInfoP->serDevHandshakeBaud = 115200;
    err = errNone;

  } else if (pumpkin_get_serial(deviceID, &str, &creator) == 0) {
    deviceInfoP->serDevPortInfoStr = pumpkin_heap_alloc(StrLen(str) + 1, "serial_descr");
    StrCopy(deviceInfoP->serDevPortInfoStr, str);
    deviceInfoP->serDevCreator = creator;
    deviceInfoP->serDevFtrInfo = 0;
    deviceInfoP->serDevMaxBaudRate = 115200;
    deviceInfoP->serDevHandshakeBaud = 115200;
    err = errNone;
  }

/*
  deviceInfoP->serDevCreator = serPortCradlePort;
  deviceInfoP->serDevFtrInfo = serDevCradlePort | serDevRS232Serial;

  deviceInfoP->serDevCreator = serPortIrPort;
  deviceInfoP->serDevFtrInfo = serDevIRDACapable;
*/

  return err;
}

Err SrmGetStatus(UInt16 portId, UInt32 *statusFieldP, UInt16 *lineErrsP) {
  if (SrmCheckPortId(portId, 0) == -1) {
    return serErrBadPort;
  }

  if (statusFieldP) *statusFieldP = 0;
  if (lineErrsP) *lineErrsP = 0;

  return errNone;
}

Err SrmClearErr(UInt16 portId) {
  if (SrmCheckPortId(portId, 0) == -1) {
    return serErrBadPort;
  }

  return errNone;
}

Err SrmControl(UInt16 portId, UInt16 op, void *valueP, UInt16 *valueLenP) {
  Err err = serErrBadPort;
  char word[4];
  UInt32 flags, baud;
  Int32 value;
  int id;

  if (SrmCheckPortId(portId, 0) == -1) {
    return serErrBadPort;
  }

  word[0] = 'N';
  word[1] = '8';
  word[2] = '1';
  word[3] = 0;

  switch (op) {
    case srmCtlSetFlags:
      flags = *(UInt32 *)valueP;
      if (flags & srmSettingsFlagXonXoffM) {
        debug(DEBUG_ERROR, PALMOS_MODULE, "SrmControl Xon/Xoff ignored");
      }
      if (flags & srmSettingsFlagRTSAutoM) {
        debug(DEBUG_ERROR, PALMOS_MODULE, "SrmControl Auto RTS ignored");
      }
      if (flags & srmSettingsFlagCTSAutoM) {
        debug(DEBUG_ERROR, PALMOS_MODULE, "SrmControl Auto CTS ignored");
      }

      if (flags & srmSettingsFlagBitsPerChar8) {
        word[1] = '8';
      } else if (flags & srmSettingsFlagBitsPerChar7) {
        word[1] = '7';
      } else if (flags & srmSettingsFlagBitsPerChar6) {
        word[1] = '6';
      } else if (flags & srmSettingsFlagBitsPerChar5) {
        word[1] = '5';
      }

      if (flags & srmSettingsFlagParityOnM) {
        if (flags & srmSettingsFlagParityEvenM) {
          word[0] = 'E';
        } else {
          word[0] = 'O';
        }
      }

      if (flags & srmSettingsFlagStopBits2) {
        word[2] = '2';
      }

      if ((id = SrmCheckPortId(portId, 0)) != -1) {
        if (pumpkin_word_serial(id, word) == 0) {
          err = errNone;
        }
      }
      break;

    case srmCtlSetBaudRate:
      baud = *(UInt32 *)valueP;

      if ((id = SrmCheckPortId(portId, 0)) != -1) {
        if (pumpkin_baud_serial(id, baud) == 0) {
          err = errNone;
        }
      }
      break;

    case srmCtlSetCtsTimeout:
      value = *(UInt32 *)valueP;
      debug(DEBUG_ERROR, PALMOS_MODULE, "SrmControl CTS timeout %u ignored", value);
      err = errNone;
      break;

    case srmCtlStartBreak:
      debug(DEBUG_ERROR, PALMOS_MODULE, "SrmControl start break ignored");
      err = errNone;
      break;

    case srmCtlStopBreak:
      debug(DEBUG_ERROR, PALMOS_MODULE, "SrmControl stop break ignored");
      err = errNone;
      break;

    default:
      debug(DEBUG_ERROR, PALMOS_MODULE, "SrmControl op %d not implemented", op);
      break;
  }

  return err;
}

UInt32 SrmSend(UInt16 portId, const void *bufP, UInt32 count, Err *errP) {
  int r;

  if (SrmCheckPortId(portId, 0) == -1) {
    *errP = serErrBadPort;
    return 0;
  }

  r = sys_write(portId, (uint8_t *)bufP, count);

  if (r == count) {
    *errP = 0;
  } else {
    *errP = serErrTimeOut;
    r = 0;
  }

  return r;
}

Err SrmSendWait(UInt16 portId) {
  if (SrmCheckPortId(portId, 0) == -1) {
    return serErrBadPort;
  }

  return errNone;
}

Err SrmSendCheck(UInt16 portId, UInt32 *numBytesP) {
  if (SrmCheckPortId(portId, 0) == -1) {
    return serErrBadPort;
  }

  if (numBytesP) *numBytesP = 0;
  return errNone;
}

Err SrmSendFlush(UInt16 portId) {
  if (SrmCheckPortId(portId, 0) == -1) {
    return serErrBadPort;
  }

  return errNone;
}

static int SrmGetByte(int id, uint8_t *b, int64_t us) {
  srm_module_t *module = (srm_module_t *)pumpkin_get_local_storage(srm_key);
  int nread, r;

  if (module->bufPtr[id] == module->bufLen[id]) {
    // return -1: error
    // return  0: nothing to read from fd
    // return  1, nread = 0: nothing was read from fd
    // return  1, nread > 0: read nread bytes from fd
    if ((r = sys_read_timeout(module->portId[id], module->buf[id], module->bufSize[id], &nread, us)) <= 0) {
      return r;
    }
    if (r == 1 && nread == 0) {
      return -1;
    }
    module->bufLen[id] = nread;
    module->bufPtr[id] = 0;

    if (nread > 0 && module->wakeUpProc[id] && module->minBytes[id]) {
      // XXX can it be called from here ?
      module->wakeUpProc[id](module->refCon[id]);
      module->minBytes[id] = 0;
    }
  }

  if (module->bufPtr[id] < module->bufLen[id]) {
    if (b) {
      *b = module->buf[id][module->bufPtr[id]];
      module->bufPtr[id]++;
    }
    return 1;
  }

  return 0;
}

UInt32 SrmReceive(UInt16 portId, void *rcvBufP, UInt32 count, Int32 timeout, Err *errP) {
  uint64_t t, te;
  uint32_t us, j;
  uint8_t *p;
  int id, r;

  if ((id = SrmCheckPortId(portId, 0)) == -1) {
    *errP = serErrBadPort;
    return 0;
  }

  us = timeout > 0 ? timeout * (1000000 / SysTicksPerSecond()) : 0;
  te = sys_get_clock() + us;
  p = (uint8_t *)rcvBufP;
  r = 0;

  for (j = 0; j < count; j++) {
    t = sys_get_clock();
    if (t >= te) break;
    r = SrmGetByte(id, &p[j], te - t);
    if (r < 0) break;
  }

  if (r == -1) {
    *errP = serErrLineErr;
    r = 0;
  } else if (j == 0) {
    *errP = serErrTimeOut;
    r = 0;
  } else {
    *errP = errNone;
    r = j;
  }

  return r;
}

static void SrmShiftBuffer(int id) {
  srm_module_t *module = (srm_module_t *)pumpkin_get_local_storage(srm_key);
  uint32_t len, i;

  if (module->bufLen[id] && module->bufPtr[id]) {
    len = module->bufLen[id] - module->bufPtr[id];
    for (i = 0; i < len; i++) {
      module->buf[id][i] = module->buf[id][module->bufPtr[id] + i];
    }
    module->bufLen[id] = len;
    module->bufPtr[id] = 0;
  }
}

Err SrmReceiveWait(UInt16 portId, UInt32 bytes, Int32 timeout) {
  srm_module_t *module = (srm_module_t *)pumpkin_get_local_storage(srm_key);
  uint32_t us;
  int id, nread, r;

  if ((id = SrmCheckPortId(portId, 0)) == -1) {
    return serErrBadPort;
  }

  if ((module->bufLen[id] - module->bufPtr[id]) >= bytes) {
    return errNone;
  }
  SrmShiftBuffer(id);
  bytes -= module->bufLen[id];

  if (bytes > (module->bufSize[id] - module->bufLen[id])) {
    bytes = module->bufSize[id] - module->bufLen[id];
  }

  // return -1: error
  // return  0: nothing to read from fd
  // return  1, nread = 0: nothing was read from fd
  // return  1, nread > 0: read nread bytes from fd
  us = timeout > 0 ? timeout * (1000000 / SysTicksPerSecond()) : 0;
  if ((r = sys_read_timeout(module->portId[id], &module->buf[id][module->bufLen[id]], bytes, &nread, us)) < 0) {
    return serErrLineErr;
  }
  if (r == 0) {
    return module->bufLen[id] ? errNone : serErrTimeOut;
  }
  if (r == 1 && nread == 0) {
    return serErrLineErr;
  }
  module->bufLen[id] += nread;

  if (nread > 0 && module->wakeUpProc[id] && module->minBytes[id]) {
    // XXX can it be called from here ?
    module->wakeUpProc[id](module->refCon[id]);
    module->minBytes[id] = 0;
  }

  return errNone;
}

Err SrmReceiveCheck(UInt16 portId, UInt32 *numBytesP) {
  srm_module_t *module = (srm_module_t *)pumpkin_get_local_storage(srm_key);
  int id;

  if ((id = SrmCheckPortId(portId, 0)) == -1) {
    return serErrBadPort;
  }

  if (numBytesP) *numBytesP = module->bufLen[id] - module->bufPtr[id];

  return errNone;
}

Err SrmReceiveFlush(UInt16 portId, Int32 timeout) {
  srm_module_t *module = (srm_module_t *)pumpkin_get_local_storage(srm_key);
  int id;

  if ((id = SrmCheckPortId(portId, 0)) == -1) {
    return serErrBadPort;
  }

  module->bufLen[id] = 0;
  module->bufPtr[id] = 0;

  return errNone;
}

Err SrmSetReceiveBuffer(UInt16 portId, void *bufP, UInt16 bufSize) {
  srm_module_t *module = (srm_module_t *)pumpkin_get_local_storage(srm_key);
  int id;
  Err err = serErrBadPort;

  if ((id = SrmCheckPortId(portId, 0)) != -1) {
    if (bufP && bufSize) {
      module->bufSize[id] = bufSize;
      module->buf[id] = bufP;
    } else {
      module->bufSize[id] = SERIAL_BUFFER;
      module->buf[id] = module->buffer[id];
    }
    module->bufLen[id] = 0;
    module->bufPtr[id] = 0;
  }

  return err;
}

Err SrmReceiveWindowOpen(UInt16 portId, UInt8 **bufPP, UInt32 *sizeP) {
  srm_module_t *module = (srm_module_t *)pumpkin_get_local_storage(srm_key);
  int id;

  if ((id = SrmCheckPortId(portId, 0)) == -1) {
    return serErrBadPort;
  }

  //SrmReceiveWait(portId, module->bufSize[id], 0);
  SrmShiftBuffer(id);

  if (bufPP) *bufPP = module->buf[id];
  if (sizeP) *sizeP = module->bufLen[id];

  return errNone;
}

Err SrmReceiveWindowClose(UInt16 portId, UInt32 bytesPulled) {
  srm_module_t *module = (srm_module_t *)pumpkin_get_local_storage(srm_key);
  int id;

  if ((id = SrmCheckPortId(portId, 0)) == -1) {
    return serErrBadPort;
  }

  if (bytesPulled >= module->bufLen[id]) {
    module->bufLen[id] = 0;
    module->bufPtr[id] = 0;
  } else if (bytesPulled > 0) {
    module->bufPtr[id] += bytesPulled;
    SrmShiftBuffer(id);
  }

  return errNone;
}

Err SrmSetWakeupHandler(UInt16 portId, WakeupHandlerProcPtr procP, UInt32 refCon) {
  srm_module_t *module = (srm_module_t *)pumpkin_get_local_storage(srm_key);
  int id;

  if ((id = SrmCheckPortId(portId, 0)) == -1) {
    return serErrBadPort;
  }

  module->wakeUpProc[id] = procP;
  module->refCon[id] = refCon;
  module->minBytes[id] = 0;

  return errNone;
}

Err SrmPrimeWakeupHandler(UInt16 portId, UInt16 minBytes) {
  srm_module_t *module = (srm_module_t *)pumpkin_get_local_storage(srm_key);
  int id;

  if ((id = SrmCheckPortId(portId, 0)) == -1) {
    return serErrBadPort;
  }

  module->minBytes[id] = minBytes;

  return errNone;
}
