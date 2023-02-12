#include <PalmOS.h>
#include <PceNativeCall.h>

#include "debug.h"

UInt32 PceNativeCall(NativeFuncType *nativeFuncP, void *userDataP) {
  UInt32 r = 0;

  if (nativeFuncP) {
    r = nativeFuncP(NULL, userDataP, NULL);
  }

  return r;
}
