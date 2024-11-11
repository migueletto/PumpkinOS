#include <PalmOS.h>
#include <VFSMgr.h>

#ifdef ARMEMU
#include "armemu.h"
#endif
#include "m68k/m68k.h"
#include "m68k/m68kcpu.h"
#include "emupalmosinc.h"
#include "emupalmos.h"
#include "plibc.h"
#include "heap.h"
#include "tos.h"
#include "gemdos.h"
#include "bios_proto.h"
#include "debug.h"

// BIOS devices:
// 0 prn: (Printer/Parallel port)
// 1 aux: (Aux device, the RS-232 port)
// 2 con: (Console, VT-52 terminal)
// 3 MIDI port
// 4 Keyboard port
// 5 Screen
// 6 ST compatible RS-232 port (Modem 1)
// 7 SCC channel B (Modem 2)
// 8 TTMFP serial port (Modem 3)
// 9 SCC channel A (Modem 4)

void Getmpb(MPB *ptr) {
  debug(DEBUG_ERROR, "TOS", "Getmpb not implemented");
}

int16_t Bconstat(int16_t dev) {
  int32_t r = 0;

  // returns -1 when there are characters waiting in the buffer, and 0 if this is not the case

  switch (dev) {
    case 2: // con: (Console)
      r = plibc_haschar() ? -1 : 0;
      break;
  }

  return r;
}

int32_t Bconin(int16_t dev) {
  int32_t c = 0;

  // returns the read-in character as an ASCII value in the bits 0..7

  switch (dev) {
    case 2: // console
      // when reading from the console, the bits 16 to 23 contain the scan-code of the relevant key
      // if, in addition, the corresponding bit of the system variable conterm is set, then the bits 24 to 31 contain the current value of Kbshift
      c = plibc_getchar();
      break;
    case 4: // keyboard port
      break;
  }

  return c;
}

void Bconout(int16_t dev, int16_t c) {
  emu_state_t *state = m68k_get_emu_state();
  tos_data_t *data = (tos_data_t *)state->extra;

  switch (dev) {
    case 2: // console
      plibc_putchar(c);
      break;
    case 4: // keyboard port
      tos_write_byte(data, 0x00FFFC02, c);
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
  emu_state_t *state = m68k_get_emu_state();
  tos_data_t *data = (tos_data_t *)state->extra;
  int32_t r = 0x01;
  if (data->b != 0xffff) r |= 0x2;
  if (data->c != 0xffff) r |= 0x4;

  return r;
}

int32_t Kbshift(int16_t mode) {
  debug(DEBUG_ERROR, "TOS", "Kbshift not implemented");
  return 0;
}
