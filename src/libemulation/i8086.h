typedef struct i8086_t i8086_t;

i8086_t *i8086_open(uint32_t period, void (*callback)(void *d, uint32_t cycles), void *data, computer_t *computer);
int i8086_close(i8086_t *i8086);
void i8086_reset(i8086_t *i8086, uint16_t cs, uint32_t ip);
int i8086_loop(i8086_t *i8086);
void i8086_stop(i8086_t *z);
void i8086_irq(i8086_t *z);
uint8_t *i8086_mem(i8086_t *i8086);
void i8086_debug(i8086_t *z, int on);

void i8086_cls(computer_t *computer);
void i8086_set_cursor(computer_t *computer, uint8_t row, uint8_t col);
void i8086_get_cursor(computer_t *computer, uint8_t *row, uint8_t *col);
void i8086_char_cursor(computer_t *computer, uint8_t *code, uint8_t *color);
void i8086_putc(computer_t *computer, uint8_t b);
void i8086_direct_video(computer_t *computer, uint32_t addr, uint16_t data, int w);
uint8_t i8086_kbhit(computer_t *computer);
uint8_t i8086_getc(computer_t *computer);
uint32_t i8086_seek(computer_t *computer, uint8_t d, uint32_t lba);
uint32_t i8086_read(computer_t *computer, uint8_t d, void *p, uint32_t len);
uint32_t i8086_write(computer_t *computer, uint8_t d, void *p, uint32_t len);
