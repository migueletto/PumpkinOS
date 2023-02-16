#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>

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

void palmos_serialtrap(uint32_t sp, uint16_t idx, uint32_t sel) {
  char buf[256];
  Err err;

  switch (sel) {
    //case sysSerialInstall:
    case sysSerialOpen: {
      // Err SrmOpen(UInt32 port, UInt32 baud, UInt16 *newPortIdP)
      uint32_t port = ARG32;
      uint32_t baud = ARG32;
      uint32_t newPortIdP = ARG32;
      UInt16 newPortId;
      err = SrmOpen(port, baud, &newPortId);
      if (newPortIdP) m68k_write_memory_16(newPortIdP, newPortId);
      pumpkin_id2s(port, buf);
      debug(DEBUG_TRACE, "EmuPalmOS", "SrmOpen('%s', %d, 0x%08X [%d]): %d", buf, baud, newPortIdP, newPortId, err);
      m68k_set_reg(M68K_REG_D0, err);
    }
      break;
      //case sysSerialOpenBkgnd:
    case sysSerialClose: {
      // Err SrmClose(UInt16 portId)
      uint16_t portId = ARG16;
      err = SrmClose(portId);
      debug(DEBUG_TRACE, "EmuPalmOS", "SrmClose(%d): %d", portId, err);
      m68k_set_reg(M68K_REG_D0, err);
    }
      break;
      //case sysSerialSleep:
      //case sysSerialWake:
    case sysSerialGetDeviceCount: {
      // Err SrmGetDeviceCount(UInt16 *numOfDevicesP)
      uint32_t numOfDevicesP = ARG32;
      UInt16 numOfDevices = 0;
      err = SrmGetDeviceCount(numOfDevicesP ? &numOfDevices : NULL);
      if (numOfDevicesP) m68k_write_memory_16(numOfDevicesP, numOfDevices);
      debug(DEBUG_TRACE, "EmuPalmOS", "SrmGetDeviceCount(0x%08X [%d]): %d", numOfDevicesP, numOfDevices, err);
      m68k_set_reg(M68K_REG_D0, err);
    }
      break;
    case sysSerialGetDeviceInfo: {
      // Err SrmGetDeviceInfo(UInt32 deviceID, DeviceInfoType *deviceInfoP)
      uint32_t deviceID = ARG32;
      uint32_t deviceInfoP = ARG32;
      DeviceInfoType deviceInfo;
      err = SrmGetDeviceInfo(deviceID, deviceInfoP ? &deviceInfo : NULL);
      encode_deviceinfo(deviceInfoP, &deviceInfo);
      debug(DEBUG_TRACE, "EmuPalmOS", "SrmGetDeviceInfo(%d, 0x%08X): %d", deviceID, deviceInfoP, err);
      m68k_set_reg(M68K_REG_D0, err);
    }
      break;
      //case sysSerialGetStatus:
      //case sysSerialClearErr:
    case sysSerialControl: {
      // Err SrmControl(UInt16 portId, UInt16 op, void *valueP, UInt16 *valueLenP)
      uint16_t portId = ARG16;
      uint16_t op = ARG16;
      uint32_t valueP = ARG32;
      uint32_t valueLenP = ARG32;
      uint8_t *p = emupalmos_trap_sel_in(valueP, sysTrapSerialDispatch, sel, 0);
      UInt16 valueLen;
      if (valueLenP) valueLen = m68k_read_memory_16(valueLenP);
      switch (op) {
        case srmCtlSetBaudRate: // valueP = pointer to Int32
        case srmCtlGetBaudRate:
          break;
/*
	srmCtlSetFlags,					// Sets the current flag settings for the serial HW.
	srmCtlGetFlags,					// Gets the current flag settings the serial HW.
	srmCtlSetCtsTimeout,				// Sets the current Cts timeout value.
	srmCtlGetCtsTimeout,				// Gets the current Cts timeout value.
	srmCtlUserDef,						// Specifying this opCode passes through a user-defined function to the DrvControl function.
	srmCtlGetOptimalTransmitSize,	// This function will ask the port for the most efficient buffer size transmitting data packets. valueP = pointer to UInt32 --> return optimal buf size ValueLenP = sizeof(UInt32)
	srmCtlSetDTRAsserted,			// Enable or disable DTR.
	srmCtlGetDTRAsserted,			// Determine if DTR is enabled or disabled.
	srmCtlSetYieldPortCallback,   // Set the yield port callback
	srmCtlSetYieldPortRefCon,     // Set the yield port refNum
*/
      }
      err = SrmControl(portId, op, p, &valueLen);
      if (valueLenP) m68k_write_memory_16(valueLenP, valueLen);
      debug(DEBUG_TRACE, "EmuPalmOS", "SrmControl(%d, %d, 0x%08X, 0x%08X): %d", portId, op, valueP, valueLenP, err);
      m68k_set_reg(M68K_REG_D0, err);
    }
      break;
        //case sysSerialSend:
        //case sysSerialSendWait:
        //case sysSerialSendCheck:
        //case sysSerialSendFlush:
        //case sysSerialReceive:
        //case sysSerialReceiveWait:
        //case sysSerialReceiveCheck:
        //case sysSerialReceiveFlush:
        //case sysSerialSetRcvBuffer:
        //case sysSerialRcvWindowOpen:
        //case sysSerialRcvWindowClose:
        //case sysSerialSetWakeupHandler:
        //case sysSerialPrimeWakeupHandler:
        //case sysSerialOpenV4:
        //case sysSerialOpenBkgndV4:
        //case sysSerialCustomControl:
    default:
      snprintf(buf, sizeof(buf)-1, "SerialDispatch selector %d not mapped", sel);
      emupalmos_panic(buf, EMUPALMOS_INVALID_TRAP);
      break;
  }
}
