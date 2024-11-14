#ifndef TOS_H
#define TOS_H

#define AES_GLOBAL_LEN 15

#define AES_INTIN_LEN   16
#define AES_INTOUT_LEN  16

#define AES_ADRIN_LEN   16
#define AES_ADROUT_LEN  16

#define TosType 'ctos'

#include "heap.h"

typedef struct {
  uint8_t *memory;
  uint32_t memorySize;
  uint8_t *io;
  uint32_t ioSize;
  uint32_t basePageStart;
  uint32_t heapStart;
  uint32_t heapSize;
  heap_t *heap;
  int supervisor;
  uint32_t kbdvbase;
  uint32_t physbase;
  uint32_t logbase;
  uint32_t lineaVars;
  uint16_t volume;
  uint16_t a, b, c;
  int debug_m68k;
  uint8_t ikbd_cmd, ikbd_state, ikbd_button;
  uint16_t ikbd_x, ikbd_y;
  uint64_t last_refresh;
  uint16_t tos_color[16];
  uint16_t pumpkin_color[16];
  uint16_t *screen;
  int screen_res;
  int screen_updated;
} tos_data_t;

typedef struct {
  int32_t pcontrol;
  int32_t pintin;
  int32_t pptsin;
  int32_t pintout;
  int32_t pptsout;
  int16_t opcode;
  int16_t sub_opcode;
  int16_t workstation;
  int16_t ptsin_len;
  int16_t intin_len;
  int16_t ptsout_len;
  int16_t intout_len;
} vdi_pb_t;

typedef struct {
  int32_t pcontrol;
  int32_t pglobal;
  int32_t pintin;
  int32_t pintout;
  int32_t padrin;
  int32_t padrout;
  int16_t opcode;
  int16_t intin_len;
  int16_t adrin_len;
  int16_t intout_len;
  int16_t adrout_len;
  int16_t global[AES_GLOBAL_LEN];
  int16_t intin[AES_INTIN_LEN];
  int16_t intout[AES_INTOUT_LEN];
  int32_t adrin[AES_ADRIN_LEN];
  int32_t adrout[AES_ADROUT_LEN];
} aes_pb_t;

UInt32 TOSMain(void);

uint16_t tos_convert_color(uint16_t color);
void tos_write_byte(tos_data_t *data, uint32_t address, uint8_t value);
void tos_write_screen(uint16_t *screen, uint32_t offset, uint16_t value, uint8_t *m, uint16_t *palette, int res, int le, int dbl);
int32_t vdi_call(vdi_pb_t *vdi_pb);
int32_t aes_call(aes_pb_t *aes_pb);

#endif
