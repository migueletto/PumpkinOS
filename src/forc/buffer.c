#include "sys.h"
#include "buffer.h"

#define INCR_ALLOC 256

static void incr_buf(buffer_t *b) {
  if (b->size == b->alloc_size) {
    b->alloc_size += INCR_ALLOC;
    b->buf = b->buf ? sys_realloc(b->buf, b->alloc_size) : sys_calloc(1, b->alloc_size);
  }
}

void buffer_free(buffer_t *buffer) {
  if (buffer->buf) sys_free(buffer->buf);
  sys_free(buffer);
}

void emit8(buffer_t *buffer, uint8_t b) {
  incr_buf(buffer);
  buffer->buf[buffer->size++] = b;
}

void emit16(buffer_t *buffer, uint16_t b) {
  incr_buf(buffer);
  buffer->buf[buffer->size++] = b;
  buffer->buf[buffer->size++] = b >> 8;
}

void emit32(buffer_t *buffer, uint32_t b) {
  incr_buf(buffer);
  buffer->buf[buffer->size++] = b;
  buffer->buf[buffer->size++] = b >> 8;
  buffer->buf[buffer->size++] = b >> 16;
  buffer->buf[buffer->size++] = b >> 24;
}

void emit64(buffer_t *buffer, uint64_t b) {
  incr_buf(buffer);
  buffer->buf[buffer->size++] = b;
  buffer->buf[buffer->size++] = b >> 8;
  buffer->buf[buffer->size++] = b >> 16;
  buffer->buf[buffer->size++] = b >> 24;
  buffer->buf[buffer->size++] = b >> 32;
  buffer->buf[buffer->size++] = b >> 40;
  buffer->buf[buffer->size++] = b >> 48;
  buffer->buf[buffer->size++] = b >> 56;
}

void emit_string(buffer_t *buffer, char *s) {
  uint32_t aux, len, i;
  uint8_t b = 0;

  len = sys_strlen(s);
  emit32(buffer, len);
  for (i = 0; i < len; i++) {
    emit8(buffer, s[i]);
  }
  aux = len % 4;
  if (aux) {
    aux = 4 - aux;
    for (i = 0; i < aux; i++) {
      emit8(buffer, b);
    }
  }
}

uint8_t get8(buffer_t *buffer) {
  return buffer->buf[buffer->i++];
}

uint16_t get16(buffer_t *buffer) {
  uint16_t b;
  b = buffer->buf[buffer->i++];
  b |= (uint16_t)buffer->buf[buffer->i++] << 8;
  return b;
}

uint32_t get32(buffer_t *buffer) {
  uint32_t b;
  b = buffer->buf[buffer->i++];
  b |= (uint32_t)buffer->buf[buffer->i++] << 8;
  b |= (uint32_t)buffer->buf[buffer->i++] << 16;
  b |= (uint32_t)buffer->buf[buffer->i++] << 24;
  return b;
}

uint64_t get64(buffer_t *buffer) {
  uint64_t b;
  b = buffer->buf[buffer->i++];
  b |= (uint64_t)buffer->buf[buffer->i++] << 8;
  b |= (uint64_t)buffer->buf[buffer->i++] << 16;
  b |= (uint64_t)buffer->buf[buffer->i++] << 24;
  b |= (uint64_t)buffer->buf[buffer->i++] << 32;
  b |= (uint64_t)buffer->buf[buffer->i++] << 40;
  b |= (uint64_t)buffer->buf[buffer->i++] << 48;
  b |= (uint64_t)buffer->buf[buffer->i++] << 56;
  return b;
}

char *get_string(buffer_t *buffer) {
  uint32_t len, i;
  char *s;

  len = get32(buffer);
  if ((s = sys_calloc(len+1, 1)) != NULL) {
    for (i = 0; i < len; i++) {
      s[i] = get8(buffer);
    }
    len = len % 4;
    if (len) {
      // skip alignment padding
      len = 4 - len;
      for (i = 0; i < len; i++) {
        get8(buffer);
      }
    }
  }

  return s;
}
