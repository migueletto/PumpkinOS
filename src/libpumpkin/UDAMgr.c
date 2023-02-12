#include <PalmOS.h>
#include <UDAMgr.h>

#include "debug.h"

Err UDAControl(UDAObjectType* ioObject, UInt16 parameter, ...) {
  debug(DEBUG_ERROR, "PALMOS", "UDAControl not implemented");
  return 0;
}

UDAReaderType* UDAExchangeReaderNew(ExgSocketType* socket) {
  debug(DEBUG_ERROR, "PALMOS", "UDAExchangeReaderNew not implemented");
  return 0;
}

UDAWriterType* UDAExchangeWriterNew(ExgSocketType* socket, UDABufferSize bufferSize) {
  debug(DEBUG_ERROR, "PALMOS", "UDAExchangeWriterNew not implemented");
  return 0;
}

UDAReaderType* UDAMemoryReaderNew(const UInt8* bufferP, UDABufferSize bufferSizeInBytes) {
  debug(DEBUG_ERROR, "PALMOS", "UDAMemoryReaderNew not implemented");
  return 0;
}

