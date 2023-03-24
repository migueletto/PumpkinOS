#include <PalmOS.h>

#include "http.h"

static NetSocketRef sock_open(char *host, UInt16 port, Err *err);
static void sock_close(NetSocketRef sock);

static UInt16 AppNetRefnum = 0;
static NetSocketAddrINType addr;
static NetHostInfoBufType hinfo;
static UInt32 lookupTimeout, connectTimeout, appTimeout;
static char buf[2048];

Err http_init(UInt32 _lookupTimeout, UInt32 _connectTimeout, UInt32 _appTimeout)
{
  UInt8 aiu;
  Err err, ifErr;

  if (SysLibFind("Net.lib", &AppNetRefnum) != 0)
    return HTTP_LIBERR;

  if (((err = NetLibOpen(AppNetRefnum, &ifErr)) != 0 &&
       err != netErrAlreadyOpen) || ifErr)
    return HTTP_LIBERR;

  if (((err = NetLibConnectionRefresh(AppNetRefnum, true, &aiu, &ifErr)) != 0 &&
       err != netErrAlreadyOpen) || ifErr) {
    NetLibClose(AppNetRefnum, true);
    return HTTP_LIBERR;
  }

  lookupTimeout = _lookupTimeout;
  connectTimeout = _connectTimeout;
  appTimeout = _appTimeout;

  return HTTP_OK;
}

Err http_finish(void)
{
  if (AppNetRefnum)
    NetLibClose(AppNetRefnum, true);

  AppNetRefnum = 0;
  return HTTP_OK;
}

static NetSocketRef sock_open(char *host, UInt16 port, Err *err)
{
  UInt16 len;
  NetHostInfoPtr hinfop;
  NetSocketRef sock;

  if (!AppNetRefnum) {
    *err = HTTP_LIBERR;
    return 0;
  }

  if ((sock = NetLibSocketOpen(AppNetRefnum, netSocketAddrINET,
                netSocketTypeStream, 0, SysTicksPerSecond()/2, err)) == -1) {
    *err = HTTP_LIBERR;
    return 0;
  }

  len = sizeof(addr);
  addr.family = netSocketAddrINET;
  addr.port = NetHToNS(port);

  addr.addr = NetLibAddrAToIN(AppNetRefnum, host);
  if (addr.addr == -1) {
    if ((hinfop = NetLibGetHostByName(AppNetRefnum, host, &hinfo,
          lookupTimeout, err)) == 0) {
      NetLibSocketClose(AppNetRefnum, sock, -1, err);
      *err = HTTP_NOTFOUND;
      return 0;
    }
    addr.addr = hinfo.address[0];
  }

  if (NetLibSocketConnect(AppNetRefnum, sock, (NetSocketAddrType *)&addr, len,
        connectTimeout, err) == -1) {
    NetLibSocketClose(AppNetRefnum, sock, -1, err);
    *err = HTTP_CONNECTERR;
    return 0;
  }

  *err = HTTP_OK;
  return sock;
}

static void sock_close(NetSocketRef sock)
{
  Err err;

  if (AppNetRefnum)
    NetLibSocketClose(AppNetRefnum, sock, -1, &err);
}

Err http_get(char *host, UInt16 port, char *path, http_buf f)
{
  NetSocketRef sock;
  Int16 n, i, response;
  char *s;
  Boolean first;
  Err err, ret;

  if (!AppNetRefnum)
    return HTTP_LIBERR;

  sock = sock_open(host, port, &err);
  if (err != HTTP_OK)
    return err;

  for (i = 0; path[i]; i++)
    if (path[i] == ' ')
      path[i] = '+';

  StrPrintF(buf, "GET %s HTTP/1.0\n\n", path);
  NetLibSend(AppNetRefnum, sock, buf, StrLen(buf),
             0, 0, 0, appTimeout, &err);

  for (first = true, ret = HTTP_ERR, response = -1;; first = false) {
    n = NetLibReceive(AppNetRefnum, sock, buf, sizeof(buf)-1,
                      0, 0, 0, appTimeout, &err);
    if (n <= 0)
      break;

    if (first && (s = StrStr(buf, "HTTP/")) != NULL && 
                 (StrStr(s, "200 OK")) != NULL)
      ret = HTTP_OK;

    if (response == 0)
      f(buf, n);

    for (i = 0; response < 0 && i < n; i++) {
      switch (response) {
        case -1:	// inicio
          if (buf[i] == '\r')
            response = -2;
          break;
        case -2:	// CR
          if (buf[i] == '\n')
            response = -3;
          else
            response = -1;
          break;
        case -3:	// CF LF
          if (buf[i] == '\r')
            response = -4;
          else
            response = -1;
          break;
        case -4:	// CR LF CR
          if (buf[i] == '\n')
            response = -5;
          else
            response = -1;
          break;
        case -5:	// CR LF CR LF
          if (n-i > 0)
            f(&buf[i], n-i);
          response = 0;
      }
    }
  }

  sock_close(sock);
  return ret;
}

char *http_error(Err err)
{
  switch (err) {
    case HTTP_OK:
      return "OK";
    case HTTP_LIBERR:
      return "Network library error";
    case HTTP_NOTFOUND:
      return "Could not find server";
    case HTTP_CONNECTERR:
      return "Coult not connect to server";
    case HTTP_ERR:
      return "Server error";
  }

  return "";
}
