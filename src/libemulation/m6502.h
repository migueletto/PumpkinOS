#ifndef _M6502_H
#define _M6502_H

#define M6502_IRQ_LINE	0
#define M6502_NMI_LINE	1

typedef struct m6502_t m6502_t;

m6502_t *m6502_open(uint32_t period, uint32_t multiplier, void (*callback)(void *d, uint32_t cycles), void *data, computer_t *computer);
int m6502_close(m6502_t *m6502);
void m6502_reset(m6502_t *m6502);
void m6502_stop(m6502_t *m6502);
int m6502_loop(m6502_t *m6502, uint32_t cycles);
void m6502_set_irq_line(m6502_t *m6502, int irqline, int state);
uint32_t m6502_getcycles(m6502_t *m6502);

#endif
