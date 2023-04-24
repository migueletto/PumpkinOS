#include <PalmOS.h>

#include "thread.h"
#include "pumpkin.h"
#include "Chat.h"
#include "debug.h"

struct ChatType {
  UInt16 refNum;
  NetSocketRef sock;
  NetSocketAddrINType serverAddr;
};

ChatType *ChatOpen(char *host, UInt16 port) {
  NetSocketAddrINType clientAddr;
  UInt16 refNum, addrLen;
  Err err;
  ChatType *chat = NULL;

  if (host) {
    // initialize NetLib
    if (SysLibFind(NetLibName, &refNum) == errNone) {
      if ((chat = MemPtrNew(sizeof(ChatType))) != NULL) {
        chat->refNum = refNum;
        // create an UDP socket
        chat->sock = NetLibSocketOpen(chat->refNum, netSocketAddrINET, netSocketTypeDatagram, 0, 0, &err);

        if (err == errNone) {
          MemSet(&clientAddr, sizeof(NetSocketAddrINType), 0);
          addrLen = sizeof(NetSocketAddrINType);

          // setup an IPv4 address
          clientAddr.family = netSocketAddrINET;
          // client address is localhost
          clientAddr.addr = NetLibAddrAToIN(refNum, "127.0.0.1");
          // using port 0 means the OS will pick a random port number
          clientAddr.port = NetHToNS(0);
          // bind the socket so that the server can send replies to it
          NetLibSocketBind(chat->refNum, chat->sock, (NetSocketAddrType *)&clientAddr, addrLen, 0, &err);

          if (err == errNone) {
            // setup an IPv4 address
            chat->serverAddr.family = netSocketAddrINET;
            // server address passed as parameter
            chat->serverAddr.addr = NetLibAddrAToIN(chat->refNum, host);
            // server port passed as parameter
            chat->serverAddr.port = NetHToNS(port);

          } else {
            debug(DEBUG_ERROR, "chat", "NetLibSocketBind failed");
            NetLibSocketClose(chat->refNum, chat->sock, 0, &err);
            MemPtrFree(chat);
            chat = NULL;
          }

        } else {
          debug(DEBUG_ERROR, "chat", "NetLibSocketOpen failed");
          MemPtrFree(chat);
          chat = NULL;
        }
      }
    } else {
      debug(DEBUG_ERROR, "chat", "NetLibSocketOpen failed");
    }
  }

  return chat;
}

void ChatClose(ChatType *chat) {
  Err err;

  if (chat) {
    NetLibSocketClose(chat->refNum, chat->sock, 0, &err);
  }
}

int ChatQuery(ChatType *chat, char *query, Boolean (*response)(char *buf, void *data), void *data) {
  NetSocketAddrINType addr;
  NetFDSetType fds;
  UInt16 addrLen;
  Int16 n;
  Err err;
  char buf[256];
  int i, r = -1;

  if (chat && query && query[0] && response) {
    addrLen = sizeof(NetSocketAddrINType);

    // send null terminated query to the server as a single UDP packet
    debug(DEBUG_INFO, "chat", "sending \"%s\"", query);
    NetLibSend(chat->refNum, chat->sock, query, StrLen(query) + 1, 0, &chat->serverAddr, addrLen, 0, &err);

    if (err == errNone) {
      // the server may send the reply in multiple parts,
      // process each part until an "#eof" string is received

      debug(DEBUG_INFO, "chat", "waiting for reply ...");
      for (i = 0; i < 50 && !thread_must_end(); i++) {
        // wait 20 ticks for a packet
        netFDZero(&fds);
        netFDSet(chat->sock, &fds);
        n = NetLibSelect(chat->refNum, chat->sock+1, &fds, NULL, NULL, 20, &err);

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
        n = NetLibReceive(chat->refNum, chat->sock, buf, sizeof(buf), 0, &addr, &addrLen, 0, &err);

        if (err != errNone) {
          // socket error, abort
          debug(DEBUG_ERROR, "chat", "NetLibReceive failed");
          break;
        }

        // if at least one packet is received, the command succeeds
        r = 0;

        if (n == 5 && !StrCaselessCompare(buf, "#eof")) {
          // a packet with "#eof" means the server is done with the query,
          // that is, all the parts of the reply have been sent
          debug(DEBUG_INFO, "chat", "#eof reply");
          break;
        }

        // print this part of the reply and wait for more parts
        debug(DEBUG_INFO, "chat", "received reply \"%s\"", buf);
        if (buf[0]) {
          if (!response(buf, data)) {
            // if the caller wants to stop, send "#stop"
            NetLibSend(chat->refNum, chat->sock, "#stop", 5, 0, &addr, addrLen, 0, &err);
            break;
          }
        }
      }
      // there are no more parts, or an error occurred
      response("", data);

    } else {
      debug(DEBUG_ERROR, "chat", "NetLibSend failed");
    }
  }

  return r;
}
