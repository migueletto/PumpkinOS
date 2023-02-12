#include <PalmOS.h>

#include "debug.h"

Err SlkOpen(void) {
  return errNone;
}

Err SlkClose(void) {
  return errNone;
}

Err SlkOpenSocket(UInt16 portID, UInt16 *socketP, Boolean staticSocket) {
  debug(DEBUG_ERROR, "PALMOS", "SlkOpenSocket not implemented");
  return 0;
}

Err SlkCloseSocket(UInt16 socket) {
  debug(DEBUG_ERROR, "PALMOS", "SlkCloseSocket not implemented");
  return 0;
}

Err SlkSocketSetTimeout(UInt16 socket, Int32 timeout) {
  debug(DEBUG_ERROR, "PALMOS", "SlkSocketSetTimeout not implemented");
  return 0;
}

Err SlkFlushSocket(UInt16 socket, Int32 timeout) {
  debug(DEBUG_ERROR, "PALMOS", "SlkFlushSocket not implemented");
  return 0;
}

// The socket listener procedure is called when a valid packet is received for the socket.
// Pointers to the packet header buffer and the packet body buffer are passed as parameters to the socket listener procedure.
Err SlkSetSocketListener(UInt16 socket,  SlkSocketListenPtr socketP) {
  debug(DEBUG_ERROR, "PALMOS", "SlkSetSocketListener not implemented");
  return 0;
}

Err SlkSendPacket(SlkPktHeaderPtr headerP, SlkWriteDataPtr writeList) {
  debug(DEBUG_ERROR, "PALMOS", "SlkSendPacket not implemented");
  return 0;
}

// system use only
Err SlkSysPktDefaultResponse(SlkPktHeaderPtr headerP, void *bodyP) {
  debug(DEBUG_ERROR, "PALMOS", "SlkSysPktDefaultResponse not implemented");
  return errNone;
}

// system use only
Err SlkProcessRPC(SlkPktHeaderPtr headerP, void *bodyP) {
  debug(DEBUG_ERROR, "PALMOS", "SlkProcessRPC not implemented");
  return errNone;
}

