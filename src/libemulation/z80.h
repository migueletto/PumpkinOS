typedef struct z80_t z80_t;

z80_t *z80_open(uint32_t period, uint32_t multiplier, void (*callback)(void *d, uint32_t cycles), void *data, computer_t *computer);
int z80_close(z80_t *z80);
void z80_reset(z80_t *z80, uint32_t pc);
int z80_loop(z80_t *z80, uint32_t cycles);
void z80_stop(z80_t *z);
void z80_halt(z80_t *z, int halt);
void z80_irq(z80_t *z);
void z80_irqM2(z80_t *z, uint8_t vector);
void z80_irqn(z80_t *z, uint32_t cycles);
void z80_nmi(z80_t *z, int waitNextOpcode);
uint32_t z80_irq_cycles(z80_t *z);
uint32_t z80_get_event_count(z80_t *z80);
void z80_set_event_count(z80_t *z80, uint32_t eventCount);
void z80_halt_exits(z80_t *z);
void z80_debug(z80_t *z, int on);
void z80_disasm_mem(z80_t *z, uint32_t addr);  
uint16_t z80_getpc(z80_t *z);
