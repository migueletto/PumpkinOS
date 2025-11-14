typedef struct m6809_t m6809_t;

m6809_t *m6809_open(uint32_t period, void (*callback)(void *data, uint32_t cycles), void *data, computer_t *computer);
int m6809_close(m6809_t *m6809);

void m6809_reset(m6809_t *m6809);
int32_t m6809_execute(m6809_t *m6809, int32_t cycles);
void m6809_set_irq_line(m6809_t *m6809, int32_t irqline, int32_t state);

void m6809_setpc(m6809_t *m6809, uint16_t pc);
