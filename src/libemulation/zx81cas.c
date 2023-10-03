#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "sys.h"
#include "vfs.h"
#include "cas.h"
#include "zx81cas.h"
#include "debug.h"
#include "xalloc.h"

#define SAMPLE_RATE 44100

#define PILOT_SAMPLES (5 * SAMPLE_RATE) // 5 seconds
#define PAUSE_SAMPLES 57

#define INCR 1024

// Each byte consists of 8 bits (MSB first) without any start and stop bits, directly followed by the next byte.
// A "0" bit consists of four high pulses, a "1" bit of nine pulses, either one followed by a silence period.
// 0:  /\/\/\/\________
// 1:  /\/\/\/\/\/\/\/\/\________

// Each pulse is split into a 150us High period, and 150us Low period. The duration of the silence between each bit is 1300us.
// The baud rate is thus 400 bps (for a "0" filled area) downto 250 bps (for a "1" filled area).
// Average medium transfer rate is approx. 307 bps (38 bytes/sec) for files that contain 50% of "0" and "1" bits each.

static void putLevel(tape_t *tape, uint8_t value, int n) {
  // n samples
  for (int i = 0; i < n; i++) {
    if (tape->buf) {
      if (tape->pos == tape->len) {
        tape->len += 1024;
        tape->buf = xrealloc(tape->buf, tape->len);
      }
    } else {
      tape->buf = xcalloc(1, INCR);
      tape->len = INCR;
      tape->pos = 0;
    }
    tape->buf[tape->pos++] = value;
  }
}

static void putPulse(tape_t *tape) {
  // pulse = 13 samples = 13 / 44100 us ~= 294 us (ideal would be 300 us)
  putLevel(tape, LEVEL_HIGH, 7);
  putLevel(tape, LEVEL_LOW, 6);
}

static void putBit(tape_t *tape, uint8_t bit) {
  int n = bit != 0 ? 9 : 4;

  // bits:
  // bit 0 = 4 pulses ~= 1179 us (ideal would be 1200 us)
  // bit 1 = 9 pulses ~= 2653 us (ideal would be 2700 us)
  for (int i = 0; i < n; i++) {
    putPulse(tape);
  }

  // pause (57 samples = 57 / 44100 s ~= 1292 us)
  putLevel(tape, LEVEL_ZERO, PAUSE_SAMPLES);

  // bit 0: 1269 + 1292 = 2471 us (ideal would be 2500 us)
  // bit 1: 2857 + 1292 = 3945 us (ideal would be 4000 us)
}

static void putByte(tape_t *tape, int b) {
  for (int i = 0; i < 8; i++) {
    putBit(tape, (b >> (7 - i)) & 0x01);
  }
}

int zx81_casRead(vfs_session_t *session, tape_t *tape, char *file) {
  uint8_t b;
  vfs_file_t *f;

  if ((f = vfs_open(session, file, VFS_READ)) == NULL) {
    debug_errno("ZX81", "open");
    return -1;
  }

  // pilot
  putLevel(tape, LEVEL_ZERO, PILOT_SAMPLES);

  // file name
  putByte(tape, 0x3F | 0x80); // dummy filename "Z"

  // data
  for (;;) {
    if (vfs_read(f, &b, 1) <= 0) break;
    putByte(tape, b);
  }
  vfs_close(f);

  tape->buf = tape->pos ? xrealloc(tape->buf, tape->pos) : NULL;
  tape->len = tape->pos;
  tape->pos = 0;
  debug(DEBUG_INFO, "ZX81", "file \"%s\" has %d samples", file, tape->len);

  return 0;
}

int zx81_casInput(tape_t *tape, uint32_t eventCount, uint32_t clock) {
  uint32_t d, cyclesPerBaud;
  int r = 0;

  if (casFinished(tape)) {
    return r;
  }

  if (tape->lastCount == 0) {
    debug(DEBUG_INFO, "ZX81", "cas begin");
    tape->lastCount = eventCount;
  }
  d = eventCount - tape->lastCount;
  cyclesPerBaud = clock / SAMPLE_RATE;

  if (d >= cyclesPerBaud) {
    for (; d >= cyclesPerBaud; d -= cyclesPerBaud) {
      if (casFinished(tape)) {
        debug(DEBUG_INFO, "ZX81", "cas end");
        break;
      }
      if ((casPos(tape) % 100000) == 0) {
        debug(DEBUG_INFO, "ZX81", "cas %d", casPos(tape) / 100000);
      }
      r = casWaveValue(tape) > LEVEL_ZERO;
    }
    tape->lastCount = eventCount - d;
  }

  return r;
}
