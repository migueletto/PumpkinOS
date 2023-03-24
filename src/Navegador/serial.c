#include <PalmOS.h>

#include "serial.h"
#include "error.h"

//#define TEST		1
#define MAX_CONNECTIONS	2
#define TAM_SERBUF	4096

static MemHandle BufferH[MAX_CONNECTIONS];
static MemPtr Buffer[MAX_CONNECTIONS];
static UInt16 refnum[MAX_CONNECTIONS];

#ifdef TEST
#include "app.h"
#include "format.h"
static Int16 NMEASerialReceive(UInt16 AppSerRefnum, UInt8 *buf, Int16 tam, Err *err);
static char rmcbuf[128], latbuf[32], lonbuf[32];
#endif

static char *CreatorToString(UInt32 creator);
static char *StrDup(char *s);

static char *CreatorToString(UInt32 creator)
{
  char s[8];
  s[0] = '(';
  s[1] = (char)(creator >> 24);
  s[2] = (char)(creator >> 16);
  s[3] = (char)(creator >> 8);
  s[4] = (char)creator;
  s[5] = ')';
  s[6] = '\0';
  return StrDup(s);
}

static char *StrDup(char *s)
{
  Int16 i = StrLen(s);
  char *d = MemPtrNew(i+1);
  StrNCopy(d, s, i);
  d[i] = '\0';
  return d;
}

void SerialInit(void)
{
  Int16 i;

  for (i = 0; i < MAX_CONNECTIONS; i++)
    refnum[i] = 0;
}

Err SerialList(char **deviceNames, UInt32 *deviceCreators, UInt16 *numDevices)
{
  DeviceInfoType dev;
  UInt16 num, i;
  Err err;

  if ((err = SrmGetDeviceCount(&num)) != 0) {
    InfoDialog(ERROR, "Serial Manager error (%d)", err);
    return err;
  }

  if (num > *numDevices)
    num = *numDevices;

  for (i = 0; i < num; i++) {
    if ((err = SrmGetDeviceInfo(i, &dev)) != 0) {
      InfoDialog(ERROR, "Serial Manager error (%d)", err);
      return err;
    }
    deviceNames[i] = dev.serDevPortInfoStr[0] ?
                       StrDup(dev.serDevPortInfoStr) :
                       CreatorToString(dev.serDevCreator);
    deviceCreators[i] = dev.serDevCreator;
  }

  *numDevices = num;
  return 0;
}

Err SerialOnline(UInt16 *AppSerRefnum, UInt32 baud, UInt16 bits, UInt16 parity,
                 UInt16 stopBits, UInt16 xonXoff, UInt16 rts, UInt16 cts,
		 UInt32 device)
{
  UInt32 flags;
  UInt16 len;
  UInt32 value;
  Int16 i;
  Err err;

  for (i = 0; i < MAX_CONNECTIONS; i++)
    if (refnum[i] == 0)
      break;

  if (i == MAX_CONNECTIONS) {
    InfoDialog(ERROR, "No more connections");
    return -1;
  }

  if ((BufferH[i] = MemHandleNew(TAM_SERBUF+64)) == NULL) {
    InfoDialog(ERROR, "Could not create handle");
    return -1;
  }
 
  if ((Buffer[i] = MemHandleLock(BufferH[i])) == NULL) {
    MemHandleFree(BufferH[i]);
    InfoDialog(ERROR, "Could not lock handle");
    return -1;
  }

  err = SrmOpen(device, baud, AppSerRefnum);

  if (err != 0 && err != serErrAlreadyOpen) {
    MemHandleUnlock(BufferH[i]);
    MemHandleFree(BufferH[i]);
    InfoDialog(ERROR, "Error opening the serial port (%d)", err);
    return -1;
  }
 
  flags = 0;
 
  if (xonXoff)
    flags |= srmSettingsFlagXonXoffM;
 
  if (rts) {
    flags |= srmSettingsFlagRTSAutoM;
    flags |= srmSettingsFlagFlowControlIn;
  }

  if (cts)
    flags |= srmSettingsFlagCTSAutoM;
 
  switch (bits) {
    case 5: flags |= srmSettingsFlagBitsPerChar5;
	    break;
    case 6: flags |= srmSettingsFlagBitsPerChar6;
	    break;
    case 7: flags |= srmSettingsFlagBitsPerChar7;
	    break;
    case 8: flags |= srmSettingsFlagBitsPerChar8;
  }
 
  flags |= srmSettingsFlagParityOnM;
 
  switch (parity) {
    case 0: flags &= ~srmSettingsFlagParityOnM;
	    break;
    case 1: flags |= srmSettingsFlagParityEvenM;
	    break;
    case 2: flags &= ~srmSettingsFlagParityEvenM;
  }
 
  switch (stopBits) {
    case 1: flags |= srmSettingsFlagStopBits1;
	    break;
    case 2: flags |= srmSettingsFlagStopBits2;
  }

  len = sizeof(flags);
  err = SrmControl(*AppSerRefnum, srmCtlSetFlags, &flags, &len);

  value = srmDefaultCTSTimeout;
  len = sizeof(value);
  if (err == 0)
    err = SrmControl(*AppSerRefnum, srmCtlSetCtsTimeout, &value, &len);

  if (err != 0) {
    MemHandleUnlock(BufferH[i]);
    MemHandleFree(BufferH[i]);
    SrmClose(*AppSerRefnum);
    InfoDialog(ERROR, "Error configuring the serial port (%d)", err);
    return -1;
  }
 
  err = SrmSetReceiveBuffer(*AppSerRefnum, Buffer[i], TAM_SERBUF+64);

  if (err != 0) {
    MemHandleUnlock(BufferH[i]);
    MemHandleFree(BufferH[i]);
    SrmClose(*AppSerRefnum);
    InfoDialog(ERROR, "Error setting the serial buffer (%d)", err);
    return -1;
  }
 
  refnum[i] = *AppSerRefnum;

  return 0;
}

void SerialOffline(UInt16 AppSerRefnum)
{
  Int16 i;

  for (i = 0; i < MAX_CONNECTIONS; i++)
    if (refnum[i] == AppSerRefnum)
      break;

  if (i == MAX_CONNECTIONS)
    return;

  refnum[i] = 0;

  SrmSetReceiveBuffer(AppSerRefnum, NULL, 0);
  SrmClose(AppSerRefnum);

  MemHandleUnlock(BufferH[i]);
  MemHandleFree(BufferH[i]);
}

Int16 SerialReceiveWait(UInt16 AppSerRefnum, UInt8 *buf, Int16 tam,
                        Int32 wait, Err *err)
{
  UInt32 n;

#ifdef TEST
   return NMEASerialReceive(AppSerRefnum, buf, tam, err);
#endif

  SrmClearErr(AppSerRefnum);
  if ((*err = SrmReceiveWait(AppSerRefnum, 1, wait)) != 0)
    return -1;
 
  SrmClearErr(AppSerRefnum);
  SrmReceiveCheck(AppSerRefnum, &n);
  if (n > tam)
    n = tam;
 
  SrmClearErr(AppSerRefnum);
  return SrmReceive(AppSerRefnum, buf, n, -1, err);
}

Int16 SerialReceive(UInt16 AppSerRefnum, UInt8 *buf, Int16 tam, Err *err)
{
  return SerialReceiveWait(AppSerRefnum, buf, tam, SysTicksPerSecond()/4, err);
}

Int16 SerialSend(UInt16 AppSerRefnum, UInt8 *buf, Int16 tam, Err *err)
{
  return SrmSend(AppSerRefnum, buf, tam, err);
}

UInt16 SerialGetStatus(UInt16 AppSerRefnum)
{
  UInt16 lineErrs;
  UInt32 status;

  return SrmGetStatus(AppSerRefnum, &status, &lineErrs) ? 0 : lineErrs;
}

Err SerialBreak(UInt16 AppSerRefnum)
{
  SrmControl(AppSerRefnum, srmCtlStartBreak, NULL, NULL);
  SysTaskDelay(3*SysTicksPerSecond()/10);	// 300 milissegundos
  SrmControl(AppSerRefnum, srmCtlStopBreak, NULL, NULL);

  return 0;
}

#ifdef TEST
static Int16 NMEASerialReceive(UInt16 AppSerRefnum, UInt8 *buf,
                               Int16 tam, Err *err)
{
  static char *list[] = {
    "$GPRMB,A,,,,,,,,,,,,A,A*0B",
    "$GPGGA,002454,3553.5295,N,13938.6570,E,1,05,2.2,18.3,M,39.0,M,,*7F",
    "$GPGSA,A,3,01,04,07,16,20,,,,,,,,3.6,2.2,2.7*35",
    "$GPGSV,3,1,09,01,38,103,37,02,23,215,00,04,38,297,37,05,00,328,00*70",
    "$GPGSV,3,2,09,07,77,299,47,11,07,087,00,16,74,041,47,20,38,044,43*73",
    "$GPGSV,3,3,09,24,12,282,00*4D",
    "$GPGLL,3553.5295,N,13938.6570,E,002454,A,A*4F",
    "$GPBOD,,T,,M,,*47",
    "$PGRME,8.6,M,9.6,M,12.9,M*15",
    "$PGRMZ,51,f*30",
    "$HCHDG,101.1,,,7.1,W*3C",
    "$GPRTE,1,1,c,*37",
    NULL
  };

  // speedx = speedy = 14.142135 KNOTS :  2*(speed^2) = 20^2

  static char *rmc = 
    "$GPRMC,114346,A,%s,S,0%s,W,20.0,43.1,280902,7.1,W,A*";

  static UInt32 last = 0;
  static double latitude  = 19.932644;
  static double longitude = 43.951012;
  static double dlat = -(14.142135 * 1852.0 / 3600.0) / 108000.0;
  static double dlon = -(14.142135 * 1852.0 / 3600.0) / 108000.0;
  char chksum[8];
  UInt32 t;
  Int16 i;
  UInt8 sum;

  t = TimGetSeconds();
  buf[0] = 0;

  if ((t - last) >= 1) {
    last = t;
    for (i = 0; list[i]; i++) {
      StrCat(buf, list[i]);
      StrCat(buf, "\r\n");
    }

    gnStrPrintF(latbuf, latitude, 2);
    gnStrPrintF(lonbuf, longitude, 3);
    StrPrintF(rmcbuf, rmc, latbuf, lonbuf);
    for (i = 1, sum = 0; rmcbuf[i] != '*'; i++)
      sum ^= rmcbuf[i];
    StrPrintF(chksum, "%02X", sum);
    StrCat(buf, rmcbuf);
    StrCat(buf, &chksum[2]);
    StrCat(buf, "\r\n");
    latitude += dlat;
    longitude += dlon;
  }

  *err = 0;
  return StrLen(buf);
}
#endif
