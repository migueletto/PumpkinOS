#include <PalmOS.h>

#include "plibc.h"
#include "gemdos.h"
#include "bios_proto.h"
#include "debug.h"

void Getmpb(MPB *ptr) {
  debug(DEBUG_ERROR, "TOS", "Getmpb not implemented");
}

int16_t Bconstat(int16_t dev) {
  // returns -1 when there are characters waiting in the buffer, and 0 if this is not the case
  debug(DEBUG_ERROR, "TOS", "Bconstat not implemented");
  return 0;
}

int32_t Bconin(int16_t dev) {
  int32_t c = 0;

  // returns the read-in character as an ASCII value in the bits 0..7

  switch (dev) {
    case 2: // con: (Console)
      // when reading from the console, the bits 16 to 23 contain the scan-code of the relevant key
      // if, in addition, the corresponding bit of the system variable conterm is set, then the bits 24 to 31 contain the current value of Kbshift
      c = plibc_getchar();
      break;
    case 4: // Keyboard port
      break;
  }

  return c;
}

void Bconout(int16_t dev, int16_t c) {
  switch (dev) {
    case 2: // con: (Console)
      plibc_putchar(c);
      break;
  }
}

int32_t Rwabs(int16_t rwflag, void *buff, int16_t cnt, int16_t recnr, int16_t dev, int32_t lrecno) {
  debug(DEBUG_ERROR, "TOS", "Rwabs not implemented");
  return 0;
}

int32_t Setexc(int16_t number, void *vec) {
  debug(DEBUG_ERROR, "TOS", "Setexc not implemented");
  return 0;
}

int32_t Tickcal(void) {
  // returns the number of milliseconds that have elapsed between two calls of the system timer
  // tor this the function accesses the _timr_ms system variable
  debug(DEBUG_ERROR, "TOS", "Tickcal not implemented");
  return 0;
}

BPB *Getbpb(int16_t dev) {
  debug(DEBUG_ERROR, "TOS", "Getbpb not implemented");
  return 0;
}

int32_t Bcostat(int16_t dev) {
  debug(DEBUG_ERROR, "TOS", "Bcostat not implemented");
  return 0;
}

int32_t Mediach(int16_t dev) {
  debug(DEBUG_ERROR, "TOS", "Mediach not implemented");
  return 0;
}

int32_t Drvmap(void) {
  debug(DEBUG_ERROR, "TOS", "Drvmap not implemented");
  return 0;
}

int32_t Kbshift(int16_t mode) {
  debug(DEBUG_ERROR, "TOS", "Kbshift not implemented");
  return 0;
}
