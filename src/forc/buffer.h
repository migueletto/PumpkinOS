typedef struct {
  uint32_t size;
  uint32_t alloc_size;
  uint32_t i;
  uint8_t *buf;
} buffer_t;

void buffer_free(buffer_t *b);
void emit8(buffer_t *buffer, uint8_t b);
void emit16(buffer_t *buffer, uint16_t b);
void emit32(buffer_t *buffer, uint32_t b);
void emit64(buffer_t *buffer, uint64_t b);
void emit_string(buffer_t *buffer, char *s);
uint8_t get8(buffer_t *buffer);
uint16_t get16(buffer_t *buffer);
uint32_t get32(buffer_t *buffer);
uint64_t get64(buffer_t *buffer);
char *get_string(buffer_t *buffer);
