#define AES_GLOBAL_LEN 15

#define AES_INTIN_LEN   16
#define AES_INTOUT_LEN  16

#define AES_ADRIN_LEN   16
#define AES_ADROUT_LEN  16

#define XBIOS_KBDVBASE_SIZE  37

#define screenSize    32768
#define lineaSize      1024

#define TosType 'ctos'

typedef struct {
  uint8_t *memory;
  uint32_t memorySize;
  uint32_t basePageStart;
  uint32_t heapStart;
  uint32_t heapSize;
  heap_t *heap;
  int supervisor;
  uint32_t kbdvbase;
  uint32_t physbase;
  uint16_t volume;
  uint16_t a, b, c;
  uint32_t lineaVars;
  int debug_m68k;
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

int tos_main_vfs(char *path, int argc, char *argv[]);
int tos_main_resource(DmResType type, DmResID id, int argc, char *argv[]);

int32_t vdi_call(vdi_pb_t *vdi_pb);
int32_t aes_call(aes_pb_t *aes_pb);
