#include <PalmOS.h>

#include "thread.h"
#include "pumpkin.h"
#include "Chat.h"
#include "debug.h"

int ChatQuery(char *host, uint16_t port, char *query, Boolean (*response)(char *buf, void *data), void *data) {
  NetSocketRef sock;
  NetSocketAddrINType addr;
  NetFDSetType fds;
  UInt16 refNum, len;
  Int16 n;
  Err err;
  char buf[256];
  int i, r = -1;

  if (host && query && query[0]) {
      // initialize NetLib
      if (SysLibFind(NetLibName, &refNum) == errNone) {

        // create an UDP socket
        sock = NetLibSocketOpen(refNum, netSocketAddrINET, netSocketTypeDatagram, 0, 0, &err);
        if (err == errNone) {
          // setup an IPv4 client address
          addr.family = netSocketAddrINET;
          // client address is localhost
          addr.addr = NetLibAddrAToIN(refNum, "127.0.0.1");
          // using port 0 means the OS will pick a random port number
          addr.port = NetHToNS(0);
          len = sizeof(NetSocketAddrINType);

          // bind the socket so that the server can send replies to it
          NetLibSocketBind(refNum, sock, (NetSocketAddrType *)&addr, len, 0, &err);

          if (err == errNone) {
            // server address passed as parameter
            addr.addr = NetLibAddrAToIN(refNum, host);
            // server port passed as parameter
            addr.port = NetHToNS(port);

            // send null terminated query to the server as a single UDP packet
            debug(DEBUG_INFO, "chat", "sending \"%s\"", query);
            NetLibSend(refNum, sock, query, StrLen(query) + 1, 0, &addr, len, 0, &err);

            if (err == errNone) {
              // the server may send the reply in multiple parts,
              // process each part until an "EOF" string is received

              debug(DEBUG_INFO, "chat", "waiting for reply ...");
              for (i = 0; i < 50 && !thread_must_end(); i++) {
                // wait 20 ticks for a packet
                netFDZero(&fds);
                netFDSet(sock, &fds);
                n = NetLibSelect(refNum, sock+1, &fds, NULL, NULL, 20, &err);

                if (err != errNone) {
                  // socket error, abort
                  debug(DEBUG_ERROR, "chat", "NetLibSelect failed");
                  break;
                }

                // no packet arrived in that time, wait again
                if (n == 0) continue;
                i = 0;

                // a packet is available, try to read it;
                // at most 256 bytes are read, if the packet is bigger than this,
                // the excess is silently discarded;
                // reply packets are null terminated strings
                n = NetLibReceive(refNum, sock, buf, sizeof(buf), 0, &addr, &len, 0, &err);

                if (err != errNone) {
                  // socket error, abort
                  debug(DEBUG_ERROR, "chat", "NetLibReceive failed");
                  break;
                }

                // if at least one packet is received, the command succeeds
                r = 0;

                if (n == 4 && !StrCompare(buf, "EOF")) {
                  // a packet with "EOF" means the server is done with the query,
                  // that is, all the parts of the reply have been sent
                  debug(DEBUG_INFO, "chat", "EOF reply");
                  break;
                }

                // print this part of the reply and wait for more parts
                debug(DEBUG_INFO, "chat", "received reply \"%s\"", buf);
                if (buf[0] && !response(buf, data)) break;
              }
              // there are no more parts, or an error occurred
              response("", data);

            } else {
              debug(DEBUG_ERROR, "chat", "NetLibSend failed");
            }
          } else {
            debug(DEBUG_ERROR, "chat", "NetLibSocketBind failed");
          }

          // we are done, close the socket
          NetLibSocketClose(refNum, sock, 0, &err);
        } else {
          debug(DEBUG_ERROR, "chat", "NetLibSocketOpen failed");
        }
      }
  }

  return r;
}
