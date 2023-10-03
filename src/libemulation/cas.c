#include "sys.h"
#include "cas.h"
#include "xalloc.h"

int casInit(tape_t *tape) {
  tape->buf = NULL;
  tape->len = 0;
  tape->pos = 0;
  tape->lastCount = 0;

  return 0;
}

int casFinish(tape_t *tape) {
  if (tape->buf) {
    xfree(tape->buf);
  }

  return casInit(tape);
}

uint8_t casWaveValue(tape_t *tape) {
  uint8_t wave = LEVEL_ZERO;

  if (tape->pos < tape->len) {
    wave = tape->buf[tape->pos];
    tape->pos++;
  }

  return wave;
}

int casFinished(tape_t *tape) {
  return tape->pos >= tape->len;
}

int casPos(tape_t *tape) {
  return tape->pos;
}

int casSize(tape_t *tape) {
  return tape->len;
}
