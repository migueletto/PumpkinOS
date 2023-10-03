#include "sys.h"
#include "pwindow.h"
#include "filter.h"
#include "computer.h"
#include "m6502.h"
#include "m6502priv.h"
#include "xalloc.h"
#include "debug.h"

enum { CLEAR_LINE=0, ASSERT_LINE};

struct m6502_t {
  PAIR pc_reg;
  PAIR a_reg;
  PAIR x_reg;
  PAIR y_reg;
  PAIR flag_reg;
  PAIR s_reg;

  uint32_t opcode;
  PAIR savepc;
  PAIR help;
  PAIR value;
  int32_t sum;
  int32_t saveflags;
  int32_t clockticks6502;

  uint32_t eventcount;
  uint32_t totalcycles;
  int irq_request;
  int nmi_request;
  int vsync_irq;
  uint32_t vsync, useevents, nevents, event, ecycle[280], earg[280];
  int stop;

  uint32_t period;
  uint32_t multiplier;
  void (*callback)(void *data, uint32_t cycles);
  void *data;
  computer_t *computer;
};

static uint8_t m6502_cycles[256] = {
  7, 6, 2, 2, 3, 3, 5, 2, 3, 3, 2, 2, 4, 4, 6, 2,
  2, 5, 3, 2, 3, 4, 6, 2, 2, 4, 2, 2, 4, 4, 7, 2,
  6, 6, 2, 2, 3, 3, 5, 2, 4, 3, 2, 2, 4, 4, 6, 2,
  2, 5, 3, 2, 4, 4, 6, 2, 2, 4, 2, 2, 4, 4, 7, 2,
  6, 6, 2, 2, 2, 3, 5, 2, 3, 3, 2, 2, 3, 4, 6, 2,
  2, 5, 3, 2, 2, 4, 6, 2, 2, 4, 3, 2, 2, 4, 7, 2,
  6, 6, 2, 2, 3, 3, 5, 2, 4, 3, 2, 2, 5, 4, 6, 2,
  2, 5, 3, 2, 4, 4, 6, 2, 2, 4, 4, 2, 6, 4, 7, 2,
  2, 6, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 4, 4, 4, 2,
  2, 6, 3, 2, 4, 4, 4, 2, 2, 5, 2, 2, 4, 5, 5, 2,
  3, 6, 3, 2, 3, 3, 3, 2, 2, 3, 2, 2, 4, 4, 4, 2,
  2, 5, 3, 2, 4, 4, 4, 2, 2, 4, 2, 2, 4, 4, 4, 2,
  3, 6, 2, 2, 3, 3, 5, 2, 2, 3, 2, 2, 4, 4, 6, 2,
  2, 5, 3, 2, 2, 4, 6, 2, 2, 4, 3, 2, 2, 4, 7, 2,
  3, 6, 2, 2, 3, 3, 5, 2, 2, 3, 2, 2, 4, 4, 6, 2,
  2, 5, 3, 2, 2, 4, 6, 2, 2, 4, 4, 2, 2, 4, 7, 2
};

static uint8_t mem_getb(m6502_t *m6502, uint16_t addr) {
  return m6502->computer->getb(m6502->computer, addr);
}

static void mem_putb(m6502_t *m6502, uint16_t addr, uint8_t b) {
  m6502->computer->putb(m6502->computer, addr, b);
}

static void m6502_optable(m6502_t *m6502);
static void m6502_adrmode(m6502_t *m6502);
static void nmi6502(m6502_t *m6502);
static void irq6502(m6502_t *m6502);

m6502_t *m6502_open(uint32_t period, uint32_t multiplier, void (*callback)(void *d, uint32_t cycles), void *data, computer_t *computer) {
  m6502_t *m6502;

  if ((m6502 = xcalloc(1, sizeof(m6502_t))) != NULL) {
    m6502->period = period;
    m6502->multiplier = multiplier;
    m6502->callback = callback;
    m6502->data = data;
    m6502->computer = computer;
  }

  return m6502;
}

int m6502_close(m6502_t *m6502) {
  int r = -1;

  if (m6502) {
    xfree(m6502);
    r = 0;
  }

  return r;
}

void m6502_reset(m6502_t *m6502) {
  m6502->eventcount = 0;
  m6502->totalcycles = 0;
  m6502->irq_request = 0;
  m6502->nmi_request = 0;
  m6502->vsync_irq = 0;

  A = X = Y = P = 0;
  P |= 0x20;
  S = 0xff;
  PC = RM16(0xfffc);
}

void m6502_stop(m6502_t *m6502) {
  m6502->stop = 1;
}

void m6502_set_irq_line(m6502_t *m6502, int irqline, int state) {
  if (irqline == M6502_NMI_LINE) {
    if (state != CLEAR_LINE) {
      nmi6502(m6502);
    }
  } else if (irqline == M6502_IRQ_LINE) {
    if (state != CLEAR_LINE) {
      irq6502(m6502);
    }
  }
}

uint32_t m6502_getcycles(m6502_t *m6502) {
  return m6502->totalcycles;
}

static uint32_t execute(m6502_t *m6502, uint32_t tcycles) {
  uint32_t opcycles;
  int32_t cycles = tcycles;

  while (cycles > 0) {
    if (m6502->nmi_request) {
      m6502_set_irq_line(m6502, M6502_NMI_LINE, ASSERT_LINE);
      m6502->nmi_request = 0;
    } else if (m6502->irq_request) {
      m6502_set_irq_line(m6502, M6502_IRQ_LINE, ASSERT_LINE);
      m6502->irq_request = 0;
    }

    m6502->opcode = RM(PC);
    PC++;

    opcycles = m6502_cycles[m6502->opcode];
    m6502_optable(m6502);

    m6502->totalcycles += opcycles;
    m6502->eventcount += opcycles;
    m6502->clockticks6502 += opcycles;
    cycles -= m6502->clockticks6502;
    m6502->clockticks6502 = 0;

    if (m6502->useevents && m6502->totalcycles >= m6502->ecycle[m6502->event]) {
      if (m6502->event == m6502->nevents) {
        m6502->totalcycles = 0;
        m6502->event = 0;
      }
      //IO_event(m6502, m6502->event, m6502->earg[m6502->event]);
      m6502->event++;
    }

    if (m6502->callback && m6502->period) {
      m6502->totalcycles += opcycles * m6502->multiplier;

      if (m6502->totalcycles >= m6502->period) {
        m6502->callback(m6502->data, m6502->totalcycles);
        m6502->totalcycles -= m6502->period;
        switch (m6502->vsync_irq) {
          case 1: m6502->irq_request = 1; break;
          case 2: m6502->nmi_request = 1;
        }
      }
    }
  }

  return 0;
}

int m6502_loop(m6502_t *m6502, uint32_t cycles) {
  execute(m6502, cycles);
  if (m6502->stop) return 1;
  return 0;
}

// Implied
void implied6502(m6502_t *m6502) {
}

// #Immediate
void immediate6502(m6502_t *m6502) {
  SAVEPC = PC++;
}

// ABS
void abs6502(m6502_t *m6502) {
  SAVEPC = RM16(PC);
  PC++;
  PC++;
}

// Branch
void relative6502(m6502_t *m6502) {
  SAVEPC = RM(PC);
  PC++;
  if (SAVEPC & 0x80) SAVEPC -= 0x100;
  if ((SAVEPC >> 8) != (PC >> 8))
    m6502->clockticks6502++;
}

// (ABS)
void indirect6502(m6502_t *m6502) {
  HELP = RM16(PC);
  SAVEPC = RM16(HELP);
  PC++;
  PC++;
}

// ABS,X
void absx6502(m6502_t *m6502) {
  SAVEPC = RM16(PC);
  PC++;
  PC++;
  if (m6502_cycles[m6502->opcode] == 4)
    if ((SAVEPC >> 8) != ((SAVEPC + X) >> 8))
      m6502->clockticks6502++;
  SAVEPC += X;
}

// ABS,Y
void absy6502(m6502_t *m6502)
{
  SAVEPC = RM16(PC);
  PC++;
  PC++;
  if (m6502_cycles[m6502->opcode] == 4)
    if ((SAVEPC >> 8) != ((SAVEPC + Y) >> 8))
      m6502->clockticks6502++;
  SAVEPC += Y;
}

// ZP
void zp6502(m6502_t *m6502)
{
  SAVEPC = RM(PC);
  PC++;
}

// ZP,X
void zpx6502(m6502_t *m6502)
{
  SAVEPC = RM(PC) + X;
  PC++;
  SAVEPC &= 0x00ff;
}

// ZP,Y
void zpy6502(m6502_t *m6502)
{
  SAVEPC = RM(PC) + Y;
  PC++;
  SAVEPC &= 0x00ff;
}

// (ZP,X)
void indx6502(m6502_t *m6502)
{
  VALUE = RM(PC) + X;
  PC++;
  SAVEPC = RM16(VALUE);
}

// (ZP),Y
void indy6502(m6502_t *m6502)
{
  VALUE = RM(PC);
  PC++;
  SAVEPC = RM16(VALUE);
  if (m6502_cycles[m6502->opcode] == 5)
    if ((SAVEPC >> 8) != ((SAVEPC + Y) >> 8))
      m6502->clockticks6502++;
  SAVEPC += Y;
}

// (ABS,X)
void indabsx6502(m6502_t *m6502)
{
  HELP = RM16(PC) + X;
  SAVEPC = RM16(HELP);
}

// (ZP)
void indzp6502(m6502_t *m6502)
{
  VALUE = RM(PC);
  PC++;
  SAVEPC = RM16(VALUE);
}


void adc6502(m6502_t *m6502)
{
  m6502_adrmode(m6502);
  VALUE = RM(SAVEPC);
  m6502->saveflags = (P & 0x01);
  m6502->sum = ((char) A) + ((char) VALUE) + m6502->saveflags;
  if ((m6502->sum > 0x7f) || (m6502->sum <- 0x80)) P |= 0x40; else P &= 0xbf;
  m6502->sum = A + VALUE + m6502->saveflags;
  if (m6502->sum > 0xff) P |= 0x01; else P &= 0xfe;
  A = m6502->sum;
  if (P & 0x08) {
    P &= 0xfe;
    if ((A & 0x0f)>0x09)
      A += 0x06;
    if ((A & 0xf0) > 0x90) {
      A += 0x60;
      P |= 0x01;
    }
  } else
    m6502->clockticks6502++;
  if (A) P &= 0xfd; else P |= 0x02;
  if (A & 0x80) P |= 0x80; else P &= 0x7f;
}

void and6502(m6502_t *m6502)
{
  m6502_adrmode(m6502);
  VALUE = RM(SAVEPC);
  A &= VALUE;
  if (A) P &= 0xfd; else P |= 0x02;
  if (A & 0x80) P |= 0x80; else P &= 0x7f;
}

void asl6502(m6502_t *m6502)
{
  m6502_adrmode(m6502);
  VALUE = RM(SAVEPC);
  P = (P & 0xfe) | ((VALUE >> 7) & 0x01);
  VALUE = VALUE << 1;
  WM(SAVEPC, VALUE);
  if (VALUE) P &= 0xfd; else P |= 0x02;
  if (VALUE & 0x80) P |= 0x80; else P &= 0x7f;
}

void asla6502(m6502_t *m6502)
{
  P = (P & 0xfe) | ((A >> 7) & 0x01);
  A = A << 1;
  if (A) P &= 0xfd; else P |= 0x02;
  if (A & 0x80) P |= 0x80; else P &= 0x7f;
}

void bcc6502(m6502_t *m6502)
{
  if ((P & 0x01) == 0) {
    m6502_adrmode(m6502);
    PC += SAVEPC;
    m6502->clockticks6502++;
  } else {
    VALUE = RM(PC);
    PC++;
  }
}

void bcs6502(m6502_t *m6502)
{
  if (P & 0x01) {
    m6502_adrmode(m6502);
    PC += SAVEPC;
    m6502->clockticks6502++;
  } else {
   VALUE = RM(PC);
   PC++;
  }
}

void beq6502(m6502_t *m6502)
{
  if (P & 0x02) {
    m6502_adrmode(m6502);
    PC += SAVEPC;
    m6502->clockticks6502++;
  } else {
    VALUE = RM(PC);
    PC++;
  }
}

void bit6502(m6502_t *m6502)
{
  m6502_adrmode(m6502);
  VALUE = RM(SAVEPC);
  if (VALUE & A) P &= 0xfd; else P |= 0x02;
  P = (P & 0x3f) | (VALUE & 0xc0);
}

void bmi6502(m6502_t *m6502)
{
  if (P & 0x80) {
    m6502_adrmode(m6502);
    PC += SAVEPC;
    m6502->clockticks6502++;
  } else {
    VALUE = RM(PC);
    PC++;
  }
}

void bne6502(m6502_t *m6502)
{
  if ((P & 0x02) == 0) {
    m6502_adrmode(m6502);
    PC += SAVEPC;
    m6502->clockticks6502++;
  } else {
    VALUE = RM(PC);
    PC++;
  }
}

void bpl6502(m6502_t *m6502)
{
  if ((P & 0x80) == 0) {
    m6502_adrmode(m6502);
    PC += SAVEPC;
    m6502->clockticks6502++;
  } else {
    VALUE = RM(PC);
    PC++;
  }
}

void brk6502(m6502_t *m6502)
{
  PC++;
  WM(0x0100+S--, PC >> 8);
  WM(0x0100+S--, PC & 0xff);
  WM(0x0100+S--, P);
  P |= 0x14;
  PC = RM16(0xfffe);
}

void bvc6502(m6502_t *m6502)
{
  if ((P & 0x40) == 0) {
    m6502_adrmode(m6502);
    PC += SAVEPC;
    m6502->clockticks6502++;
  } else {
    VALUE = RM(PC);
    PC++;
  }
}

void bvs6502(m6502_t *m6502)
{
  if (P & 0x40) {
    m6502_adrmode(m6502);
    PC += SAVEPC;
    m6502->clockticks6502++;
  } else {
    VALUE = RM(PC);
    PC++;
  }
}

void clc6502(m6502_t *m6502)
{
  P &= 0xfe;
}

void cld6502(m6502_t *m6502)
{
  P &= 0xf7;
}

void cli6502(m6502_t *m6502)
{
  P &= 0xfb;
}

void clv6502(m6502_t *m6502)
{
  P &= 0xbf;
}

void cmp6502(m6502_t *m6502)
{
  m6502_adrmode(m6502);
  VALUE = RM(SAVEPC);
  if (A + 0x100 - VALUE > 0xff) P |= 0x01; else P &= 0xfe;
  VALUE = A + 0x100 - VALUE;
  if (VALUE) P &= 0xfd; else P |= 0x02;
  if (VALUE & 0x80) P |= 0x80; else P &= 0x7f;
}

void cpx6502(m6502_t *m6502)
{
  m6502_adrmode(m6502);
  VALUE = RM(SAVEPC);
  if (X + 0x100 - VALUE > 0xff) P |= 0x01; else P &= 0xfe;
  VALUE = X + 0x100 - VALUE;
  if (VALUE) P &= 0xfd; else P |= 0x02;
  if (VALUE & 0x80) P |= 0x80; else P &= 0x7f;
}

void cpy6502(m6502_t *m6502)
{
  m6502_adrmode(m6502);
  VALUE = RM(SAVEPC);
  if (Y + 0x100 - VALUE > 0xff) P |= 0x01; else P &= 0xfe;
  VALUE = Y + 0x100 - VALUE;
  if (VALUE) P &= 0xfd; else P |= 0x02;
  if (VALUE & 0x80) P |= 0x80; else P &= 0x7f;
}

void dec6502(m6502_t *m6502)
{
  uint8_t tmp;
  m6502_adrmode(m6502);
  tmp = RM(SAVEPC);
  WM(SAVEPC, tmp - 1);
  VALUE = RM(SAVEPC);
  if (VALUE) P &= 0xfd; else P |= 0x02;
  if (VALUE & 0x80) P |= 0x80; else P &= 0x7f;
}

void dex6502(m6502_t *m6502)
{
  X--;
  if (X) P &= 0xfd; else P |= 0x02;
  if (X & 0x80) P |= 0x80; else P &= 0x7f;
}

void dey6502(m6502_t *m6502)
{
  Y--;
  if (Y) P &= 0xfd; else P |= 0x02;
  if (Y & 0x80) P |= 0x80; else P &= 0x7f;
}

void eor6502(m6502_t *m6502)
{
  m6502_adrmode(m6502);
  A ^= RM(SAVEPC);
  if (A) P &= 0xfd; else P |= 0x02;
  if (A & 0x80) P |= 0x80; else P &= 0x7f;
}

void inc6502(m6502_t *m6502)
{
  uint8_t tmp;
  m6502_adrmode(m6502);
  tmp = RM(SAVEPC);
  WM(SAVEPC, tmp + 1);
  VALUE = RM(SAVEPC);
  if (VALUE) P &= 0xfd; else P |= 0x02;
  if (VALUE & 0x80) P |= 0x80; else P &= 0x7f;
}

void inx6502(m6502_t *m6502)
{
  X++;
  if (X) P &= 0xfd; else P |= 0x02;
  if (X & 0x80) P |= 0x80; else P &= 0x7f;
}

void iny6502(m6502_t *m6502)
{
  Y++;
  if (Y) P &= 0xfd; else P |= 0x02;
  if (Y & 0x80) P |= 0x80; else P &= 0x7f;
}

void jmp6502(m6502_t *m6502)
{
  m6502_adrmode(m6502);
  PC = SAVEPC;
}

void jsr6502(m6502_t *m6502)
{
  PC++;
  WM(0x0100+S--, PC >> 8);
  WM(0x0100+S--, PC & 0xff);
  PC--;
  m6502_adrmode(m6502);
  PC = SAVEPC;
}

void lda6502(m6502_t *m6502)
{
  m6502_adrmode(m6502);
  A = RM(SAVEPC);
  // set the zero flag
  if (A) P &= 0xfd; else P |= 0x02;
  // set the negative flag
  if (A & 0x80) P |= 0x80; else P &= 0x7f;
}

void ldx6502(m6502_t *m6502)
{
  m6502_adrmode(m6502);
  X = RM(SAVEPC);
  if (X) P &= 0xfd; else P |= 0x02;
  if (X & 0x80) P |= 0x80; else P &= 0x7f;
}

void ldy6502(m6502_t *m6502)
{
  m6502_adrmode(m6502);
  Y = RM(SAVEPC);
  if (Y) P &= 0xfd; else P |= 0x02;
  if (Y & 0x80) P |= 0x80; else P &= 0x7f;
}

void lsr6502(m6502_t *m6502)
{
  m6502_adrmode(m6502);
  VALUE = RM(SAVEPC);

  // set carry flag if shifting right causes a bit to be lost
  P = (P & 0xfe) | (VALUE & 0x01);

  VALUE = VALUE >> 1;
  WM(SAVEPC, VALUE);

  // set zero flag if value is zero
  if (VALUE != 0) P &= 0xfd; else P |= 0x02;

  // set negative flag if bit 8 set??? can this happen on an LSR?
  if ((VALUE & 0x80) == 0x80)
    P |= 0x80;
  else
    P &= 0x7f;
}

void lsra6502(m6502_t *m6502)
{
  P = (P & 0xfe) | (A & 0x01);
  A = A >>1;
  if (A) P &= 0xfd; else P |= 0x02;
  if (A & 0x80) P |= 0x80; else P &= 0x7f;
}

void nop6502(m6502_t *m6502)
{
}

void ora6502(m6502_t *m6502)
{
  m6502_adrmode(m6502);
  A |= RM(SAVEPC);
  if (A) P &= 0xfd; else P |= 0x02;
  if (A & 0x80) P |= 0x80; else P &= 0x7f;
}

void pha6502(m6502_t *m6502)
{
  WM(0x100+S--, A);
}

void php6502(m6502_t *m6502)
{
  WM(0x100+S--, P);
}

void pla6502(m6502_t *m6502)
{
  A = RM(++S+0x100);
  if (A) P &= 0xfd; else P |= 0x02;
  if (A & 0x80) P |= 0x80; else P &= 0x7f;
}

void plp6502(m6502_t *m6502)
{
  P = RM(++S+0x100) | 0x20;
}

void rol6502(m6502_t *m6502)
{
  m6502->saveflags = (P & 0x01);
  m6502_adrmode(m6502);
  VALUE = RM(SAVEPC);
  P = (P & 0xfe) | ((VALUE >>7) & 0x01);
  VALUE = VALUE << 1;
  VALUE |= m6502->saveflags;
  WM(SAVEPC, VALUE);
  if (VALUE) P &= 0xfd; else P |= 0x02;
  if (VALUE & 0x80) P |= 0x80; else P &= 0x7f;
}

void rola6502(m6502_t *m6502)
{
  m6502->saveflags = P & 0x01;
  P = (P & 0xfe) | ((A >> 7) & 0x01);
  A = A << 1;
  A |= m6502->saveflags;
  if (A) P &= 0xfd; else P |= 0x02;
  if (A & 0x80) P |= 0x80; else P &= 0x7f;
}

void ror6502(m6502_t *m6502)
{
  m6502->saveflags = P & 0x01;
  m6502_adrmode(m6502);
  VALUE = RM(SAVEPC);
  P= (P & 0xfe) | (VALUE & 0x01);
  VALUE = VALUE >> 1;
  if (m6502->saveflags) VALUE |= 0x80;
  WM(SAVEPC, VALUE);
  if (VALUE) P &= 0xfd; else P |= 0x02;
  if (VALUE & 0x80) P |= 0x80; else P &= 0x7f;
}

void rora6502(m6502_t *m6502)
{
  m6502->saveflags = P & 0x01;
  P = (P & 0xfe) | (A & 0x01);
  A = A >> 1;
  if (m6502->saveflags) A |= 0x80;
  if (A) P &= 0xfd; else P |= 0x02;
  if (A & 0x80) P |= 0x80; else P &= 0x7f;
}

void rti6502(m6502_t *m6502)
{
  P = RM(++S+0x100) | 0x20;
  PC = RM(++S+0x100);
  PC |= RM(++S+0x100) << 8;
}

void rts6502(m6502_t *m6502)
{
  PC = RM(++S+0x100);
  PC |= RM(++S+0x100) << 8;
  PC++;
}

void sbc6502(m6502_t *m6502)
{
  m6502_adrmode(m6502);
  VALUE = RM(SAVEPC) ^ 0xff;
  m6502->saveflags = P & 0x01;
  m6502->sum= ((char) A) + ((char) VALUE) + (m6502->saveflags << 4);
  if ((m6502->sum > 0x7f) || (m6502->sum <- 0x80)) P |= 0x40; else P &= 0xbf;
  m6502->sum= A + VALUE + m6502->saveflags;
  if (m6502->sum > 0xff) P |= 0x01; else P &= 0xfe;
  A=m6502->sum;
  if (P & 0x08) {
    A -= 0x66;  
    P &= 0xfe;
    if ((A & 0x0f) > 0x09)
      A += 0x06;
    if ((A & 0xf0) > 0x90) {
      A += 0x60;
      P |= 0x01;
    }
  } else
    m6502->clockticks6502++;
  if (A) P &= 0xfd; else P |= 0x02;
  if (A & 0x80) P |= 0x80; else P &= 0x7f;
}

void sec6502(m6502_t *m6502)
{
  P |= 0x01;
}

void sed6502(m6502_t *m6502)
{
  P |= 0x08;
}

void sei6502(m6502_t *m6502)
{
  P |= 0x04;
}

void sta6502(m6502_t *m6502)
{
  m6502_adrmode(m6502);
  WM(SAVEPC, A);
}

void stx6502(m6502_t *m6502)
{
  m6502_adrmode(m6502);
  WM(SAVEPC, X);
}

void sty6502(m6502_t *m6502)
{
  m6502_adrmode(m6502);
  WM(SAVEPC, Y);
}

void tax6502(m6502_t *m6502)
{
  X = A;
  if (X) P &= 0xfd; else P |= 0x02;
  if (X & 0x80) P |= 0x80; else P &= 0x7f;
}

void tay6502(m6502_t *m6502)
{
  Y = A;
  if (Y) P &= 0xfd; else P |= 0x02;
  if (Y & 0x80) P |= 0x80; else P &= 0x7f;
}

void tsx6502(m6502_t *m6502)
{
  X = S;
  if (X) P &= 0xfd; else P |= 0x02;
  if (X & 0x80) P |= 0x80; else P &= 0x7f;
}

void txa6502(m6502_t *m6502)
{
  A = X;
  if (A) P &= 0xfd; else P |= 0x02;
  if (A & 0x80) P |= 0x80; else P &= 0x7f;
}

void txs6502(m6502_t *m6502)
{
  S = X;
}

void tya6502(m6502_t *m6502)
{
  A = Y;
  if (A) P &= 0xfd; else P |= 0x02;
  if (A & 0x80) P |= 0x80; else P &= 0x7f;
}

void bra6502(m6502_t *m6502)
{
  m6502_adrmode(m6502);
  PC += SAVEPC;
  m6502->clockticks6502++;
}

void dea6502(m6502_t *m6502)
{
  A--;
  if (A) P &= 0xfd; else P |= 0x02;
  if (A & 0x80) P |= 0x80; else P &= 0x7f;
}

void ina6502(m6502_t *m6502)
{
  A++;
  if (A) P &= 0xfd; else P |= 0x02;
  if (A & 0x80) P |= 0x80; else P &= 0x7f;
}

void phx6502(m6502_t *m6502)
{
  WM(0x100+S--, X);
}

void plx6502(m6502_t *m6502)
{
  X = RM(++S+0x100);
  if (X) P &= 0xfd; else P |= 0x02;
  if (X & 0x80) P |= 0x80; else P &= 0x7f;
}

void phy6502(m6502_t *m6502)
{
  WM(0x100+S--, Y);
}

void ply6502(m6502_t *m6502)
{
  Y = RM(++S+0x100);
  if (Y) P &= 0xfd; else P |= 0x02;
  if (Y & 0x80) P |= 0x80; else P &= 0x7f;
}

void stz6502(m6502_t *m6502)
{
  m6502_adrmode(m6502);
  WM(SAVEPC, 0);
}

void tsb6502(m6502_t *m6502)
{
  uint8_t tmp;
  m6502_adrmode(m6502);
  tmp = RM(SAVEPC) | A;
  WM(SAVEPC, tmp);
  if (RM(SAVEPC)) P &= 0xfd; else P |= 0x02;
}

void trb6502(m6502_t *m6502)
{
  uint8_t tmp;
  m6502_adrmode(m6502);
  tmp = RM(SAVEPC) & (A ^ 0xff);
  WM(SAVEPC, tmp);
  if (RM(SAVEPC)) P &= 0xfd; else P |= 0x02;
}

static void nmi6502(m6502_t *m6502)
{
  WM(0x0100+S--, PC >> 8);
  WM(0x0100+S--, PC & 0xff);
  WM(0x0100+S--, P);
  P |= 0x04;
  PC = RM16(0xfffa);
}

static void irq6502(m6502_t *m6502)
{
  if (!(P & 0x04)) {
    WM(0x0100+S--, PC >> 8);
    WM(0x0100+S--, PC & 0xff);
    WM(0x0100+S--, P);
    P |= 0x04;
    PC = RM16(0xfffe);
  }
}

static void m6502_optable(m6502_t *m6502) {
  switch (m6502->opcode) {
    case 0x00: brk6502(m6502); break;
    case 0x01: ora6502(m6502); break;
    case 0x02: nop6502(m6502); break;
    case 0x03: nop6502(m6502); break;
    case 0x04: tsb6502(m6502); break;
    case 0x05: ora6502(m6502); break;
    case 0x06: asl6502(m6502); break;
    case 0x07: nop6502(m6502); break;
    case 0x08: php6502(m6502); break;
    case 0x09: ora6502(m6502); break;
    case 0x0a: asla6502(m6502); break;
    case 0x0b: nop6502(m6502); break;
    case 0x0c: tsb6502(m6502); break;
    case 0x0d: ora6502(m6502); break;
    case 0x0e: asl6502(m6502); break;
    case 0x0f: nop6502(m6502); break;
    case 0x10: bpl6502(m6502); break;
    case 0x11: ora6502(m6502); break;
    case 0x12: ora6502(m6502); break;
    case 0x13: nop6502(m6502); break;
    case 0x14: trb6502(m6502); break;
    case 0x15: ora6502(m6502); break;
    case 0x16: asl6502(m6502); break;
    case 0x17: nop6502(m6502); break;
    case 0x18: clc6502(m6502); break;
    case 0x19: ora6502(m6502); break;
    case 0x1a: ina6502(m6502); break;
    case 0x1b: nop6502(m6502); break;
    case 0x1c: trb6502(m6502); break;
    case 0x1d: ora6502(m6502); break;
    case 0x1e: asl6502(m6502); break;
    case 0x1f: nop6502(m6502); break;
    case 0x20: jsr6502(m6502); break;
    case 0x21: and6502(m6502); break;
    case 0x22: nop6502(m6502); break;
    case 0x23: nop6502(m6502); break;
    case 0x24: bit6502(m6502); break;
    case 0x25: and6502(m6502); break;
    case 0x26: rol6502(m6502); break;
    case 0x27: nop6502(m6502); break;
    case 0x28: plp6502(m6502); break;
    case 0x29: and6502(m6502); break;
    case 0x2a: rola6502(m6502); break;
    case 0x2b: nop6502(m6502); break;
    case 0x2c: bit6502(m6502); break;
    case 0x2d: and6502(m6502); break;
    case 0x2e: rol6502(m6502); break;
    case 0x2f: nop6502(m6502); break;
    case 0x30: bmi6502(m6502); break;
    case 0x31: and6502(m6502); break;
    case 0x32: and6502(m6502); break;
    case 0x33: nop6502(m6502); break;
    case 0x34: bit6502(m6502); break;
    case 0x35: and6502(m6502); break;
    case 0x36: rol6502(m6502); break;
    case 0x37: nop6502(m6502); break;
    case 0x38: sec6502(m6502); break;
    case 0x39: and6502(m6502); break;
    case 0x3a: dea6502(m6502); break;
    case 0x3b: nop6502(m6502); break;
    case 0x3c: bit6502(m6502); break;
    case 0x3d: and6502(m6502); break;
    case 0x3e: rol6502(m6502); break;
    case 0x3f: nop6502(m6502); break;
    case 0x40: rti6502(m6502); break;
    case 0x41: eor6502(m6502); break;
    case 0x42: nop6502(m6502); break;
    case 0x43: nop6502(m6502); break;
    case 0x44: nop6502(m6502); break;
    case 0x45: eor6502(m6502); break;
    case 0x46: lsr6502(m6502); break;
    case 0x47: nop6502(m6502); break;
    case 0x48: pha6502(m6502); break;
    case 0x49: eor6502(m6502); break;
    case 0x4a: lsra6502(m6502); break;
    case 0x4b: nop6502(m6502); break;
    case 0x4c: jmp6502(m6502); break;
    case 0x4d: eor6502(m6502); break;
    case 0x4e: lsr6502(m6502); break;
    case 0x4f: nop6502(m6502); break;
    case 0x50: bvc6502(m6502); break;
    case 0x51: eor6502(m6502); break;
    case 0x52: eor6502(m6502); break;
    case 0x53: nop6502(m6502); break;
    case 0x54: nop6502(m6502); break;
    case 0x55: eor6502(m6502); break;
    case 0x56: lsr6502(m6502); break;
    case 0x57: nop6502(m6502); break;
    case 0x58: cli6502(m6502); break;
    case 0x59: eor6502(m6502); break;
    case 0x5a: phy6502(m6502); break;
    case 0x5b: nop6502(m6502); break;
    case 0x5c: nop6502(m6502); break;
    case 0x5d: eor6502(m6502); break;
    case 0x5e: lsr6502(m6502); break;
    case 0x5f: nop6502(m6502); break;
    case 0x60: rts6502(m6502); break;
    case 0x61: adc6502(m6502); break;
    case 0x62: nop6502(m6502); break;
    case 0x63: nop6502(m6502); break;
    case 0x64: stz6502(m6502); break;
    case 0x65: adc6502(m6502); break;
    case 0x66: ror6502(m6502); break;
    case 0x67: nop6502(m6502); break;
    case 0x68: pla6502(m6502); break;
    case 0x69: adc6502(m6502); break;
    case 0x6a: rora6502(m6502); break;
    case 0x6b: nop6502(m6502); break;
    case 0x6c: jmp6502(m6502); break;
    case 0x6d: adc6502(m6502); break;
    case 0x6e: ror6502(m6502); break;
    case 0x6f: nop6502(m6502); break;
    case 0x70: bvs6502(m6502); break;
    case 0x71: adc6502(m6502); break;
    case 0x72: adc6502(m6502); break;
    case 0x73: nop6502(m6502); break;
    case 0x74: stz6502(m6502); break;
    case 0x75: adc6502(m6502); break;
    case 0x76: ror6502(m6502); break;
    case 0x77: nop6502(m6502); break;
    case 0x78: sei6502(m6502); break;
    case 0x79: adc6502(m6502); break;
    case 0x7a: ply6502(m6502); break;
    case 0x7b: nop6502(m6502); break;
    case 0x7c: jmp6502(m6502); break;
    case 0x7d: adc6502(m6502); break;
    case 0x7e: ror6502(m6502); break;
    case 0x7f: nop6502(m6502); break;
    case 0x80: bra6502(m6502); break;
    case 0x81: sta6502(m6502); break;
    case 0x82: nop6502(m6502); break;
    case 0x83: nop6502(m6502); break;
    case 0x84: sty6502(m6502); break;
    case 0x85: sta6502(m6502); break;
    case 0x86: stx6502(m6502); break;
    case 0x87: nop6502(m6502); break;
    case 0x88: dey6502(m6502); break;
    case 0x89: bit6502(m6502); break;
    case 0x8a: txa6502(m6502); break;
    case 0x8b: nop6502(m6502); break;
    case 0x8c: sty6502(m6502); break;
    case 0x8d: sta6502(m6502); break;
    case 0x8e: stx6502(m6502); break;
    case 0x8f: nop6502(m6502); break;
    case 0x90: bcc6502(m6502); break;
    case 0x91: sta6502(m6502); break;
    case 0x92: sta6502(m6502); break;
    case 0x93: nop6502(m6502); break;
    case 0x94: sty6502(m6502); break;
    case 0x95: sta6502(m6502); break;
    case 0x96: stx6502(m6502); break;
    case 0x97: nop6502(m6502); break;
    case 0x98: tya6502(m6502); break;
    case 0x99: sta6502(m6502); break;
    case 0x9a: txs6502(m6502); break;
    case 0x9b: nop6502(m6502); break;
    case 0x9c: stz6502(m6502); break;
    case 0x9d: sta6502(m6502); break;
    case 0x9e: stz6502(m6502); break;
    case 0x9f: nop6502(m6502); break;
    case 0xa0: ldy6502(m6502); break;
    case 0xa1: lda6502(m6502); break;
    case 0xa2: ldx6502(m6502); break;
    case 0xa3: nop6502(m6502); break;
    case 0xa4: ldy6502(m6502); break;
    case 0xa5: lda6502(m6502); break;
    case 0xa6: ldx6502(m6502); break;
    case 0xa7: nop6502(m6502); break;
    case 0xa8: tay6502(m6502); break;
    case 0xa9: lda6502(m6502); break;
    case 0xaa: tax6502(m6502); break;
    case 0xab: nop6502(m6502); break;
    case 0xac: ldy6502(m6502); break;
    case 0xad: lda6502(m6502); break;
    case 0xae: ldx6502(m6502); break;
    case 0xaf: nop6502(m6502); break;
    case 0xb0: bcs6502(m6502); break;
    case 0xb1: lda6502(m6502); break;
    case 0xb2: lda6502(m6502); break;
    case 0xb3: nop6502(m6502); break;
    case 0xb4: ldy6502(m6502); break;
    case 0xb5: lda6502(m6502); break;
    case 0xb6: ldx6502(m6502); break;
    case 0xb7: nop6502(m6502); break;
    case 0xb8: clv6502(m6502); break;
    case 0xb9: lda6502(m6502); break;
    case 0xba: tsx6502(m6502); break;
    case 0xbb: nop6502(m6502); break;
    case 0xbc: ldy6502(m6502); break;
    case 0xbd: lda6502(m6502); break;
    case 0xbe: ldx6502(m6502); break;
    case 0xbf: nop6502(m6502); break;
    case 0xc0: cpy6502(m6502); break;
    case 0xc1: cmp6502(m6502); break;
    case 0xc2: nop6502(m6502); break;
    case 0xc3: nop6502(m6502); break;
    case 0xc4: cpy6502(m6502); break;
    case 0xc5: cmp6502(m6502); break;
    case 0xc6: dec6502(m6502); break;
    case 0xc7: nop6502(m6502); break;
    case 0xc8: iny6502(m6502); break;
    case 0xc9: cmp6502(m6502); break;
    case 0xca: dex6502(m6502); break;
    case 0xcb: nop6502(m6502); break;
    case 0xcc: cpy6502(m6502); break;
    case 0xcd: cmp6502(m6502); break;
    case 0xce: dec6502(m6502); break;
    case 0xcf: nop6502(m6502); break;
    case 0xd0: bne6502(m6502); break;
    case 0xd1: cmp6502(m6502); break;
    case 0xd2: cmp6502(m6502); break;
    case 0xd3: nop6502(m6502); break;
    case 0xd4: nop6502(m6502); break;
    case 0xd5: cmp6502(m6502); break;
    case 0xd6: dec6502(m6502); break;
    case 0xd7: nop6502(m6502); break;
    case 0xd8: cld6502(m6502); break;
    case 0xd9: cmp6502(m6502); break;
    case 0xda: phx6502(m6502); break;
    case 0xdb: nop6502(m6502); break;
    case 0xdc: nop6502(m6502); break;
    case 0xdd: cmp6502(m6502); break;
    case 0xde: dec6502(m6502); break;
    case 0xdf: nop6502(m6502); break;
    case 0xe0: cpx6502(m6502); break;
    case 0xe1: sbc6502(m6502); break;
    case 0xe2: nop6502(m6502); break;
    case 0xe3: nop6502(m6502); break;
    case 0xe4: cpx6502(m6502); break;
    case 0xe5: sbc6502(m6502); break;
    case 0xe6: inc6502(m6502); break;
    case 0xe7: nop6502(m6502); break;
    case 0xe8: inx6502(m6502); break;
    case 0xe9: sbc6502(m6502); break;
    case 0xea: nop6502(m6502); break;
    case 0xeb: nop6502(m6502); break;
    case 0xec: cpx6502(m6502); break;
    case 0xed: sbc6502(m6502); break;
    case 0xee: inc6502(m6502); break;
    case 0xef: nop6502(m6502); break;
    case 0xf0: beq6502(m6502); break;
    case 0xf1: sbc6502(m6502); break;
    case 0xf2: sbc6502(m6502); break;
    case 0xf3: nop6502(m6502); break;
    case 0xf4: nop6502(m6502); break;
    case 0xf5: sbc6502(m6502); break;
    case 0xf6: inc6502(m6502); break;
    case 0xf7: nop6502(m6502); break;
    case 0xf8: sed6502(m6502); break;
    case 0xf9: sbc6502(m6502); break;
    case 0xfa: plx6502(m6502); break;
    case 0xfb: nop6502(m6502); break;
    case 0xfc: nop6502(m6502); break;
    case 0xfd: sbc6502(m6502); break;
    case 0xfe: inc6502(m6502); break;
    case 0xff: nop6502(m6502); break;
  }
}

static void m6502_adrmode(m6502_t *m6502) {
  switch (m6502->opcode) {
    case 0x00: implied6502(m6502); break;
    case 0x01: indx6502(m6502); break;
    case 0x02: implied6502(m6502); break;
    case 0x03: implied6502(m6502); break;
    case 0x04: zp6502(m6502); break;
    case 0x05: zp6502(m6502); break;
    case 0x06: zp6502(m6502); break;
    case 0x07: implied6502(m6502); break;
    case 0x08: implied6502(m6502); break;
    case 0x09: immediate6502(m6502); break;
    case 0x0a: implied6502(m6502); break;
    case 0x0b: implied6502(m6502); break;
    case 0x0c: abs6502(m6502); break;
    case 0x0d: abs6502(m6502); break;
    case 0x0e: abs6502(m6502); break;
    case 0x0f: implied6502(m6502); break;
    case 0x10: relative6502(m6502); break;
    case 0x11: indy6502(m6502); break;
    case 0x12: indzp6502(m6502); break;
    case 0x13: implied6502(m6502); break;
    case 0x14: zp6502(m6502); break;
    case 0x15: zpx6502(m6502); break;
    case 0x16: zpx6502(m6502); break;
    case 0x17: implied6502(m6502); break;
    case 0x18: implied6502(m6502); break;
    case 0x19: absy6502(m6502); break;
    case 0x1a: implied6502(m6502); break;
    case 0x1b: implied6502(m6502); break;
    case 0x1c: abs6502(m6502); break;
    case 0x1d: absx6502(m6502); break;
    case 0x1e: absx6502(m6502); break;
    case 0x1f: implied6502(m6502); break;
    case 0x20: abs6502(m6502); break;
    case 0x21: indx6502(m6502); break;
    case 0x22: implied6502(m6502); break;
    case 0x23: implied6502(m6502); break;
    case 0x24: zp6502(m6502); break;
    case 0x25: zp6502(m6502); break;
    case 0x26: zp6502(m6502); break;
    case 0x27: implied6502(m6502); break;
    case 0x28: implied6502(m6502); break;
    case 0x29: immediate6502(m6502); break;
    case 0x2a: implied6502(m6502); break;
    case 0x2b: implied6502(m6502); break;
    case 0x2c: abs6502(m6502); break;
    case 0x2d: abs6502(m6502); break;
    case 0x2e: abs6502(m6502); break;
    case 0x2f: implied6502(m6502); break;
    case 0x30: relative6502(m6502); break;
    case 0x31: indy6502(m6502); break;
    case 0x32: indzp6502(m6502); break;
    case 0x33: implied6502(m6502); break;
    case 0x34: zpx6502(m6502); break;
    case 0x35: zpx6502(m6502); break;
    case 0x36: zpx6502(m6502); break;
    case 0x37: implied6502(m6502); break;
    case 0x38: implied6502(m6502); break;
    case 0x39: absy6502(m6502); break;
    case 0x3a: implied6502(m6502); break;
    case 0x3b: implied6502(m6502); break;
    case 0x3c: absx6502(m6502); break;
    case 0x3d: absx6502(m6502); break;
    case 0x3e: absx6502(m6502); break;
    case 0x3f: implied6502(m6502); break;
    case 0x40: implied6502(m6502); break;
    case 0x41: indx6502(m6502); break;
    case 0x42: implied6502(m6502); break;
    case 0x43: implied6502(m6502); break;
    case 0x44: implied6502(m6502); break;
    case 0x45: zp6502(m6502); break;
    case 0x46: zp6502(m6502); break;
    case 0x47: implied6502(m6502); break;
    case 0x48: implied6502(m6502); break;
    case 0x49: immediate6502(m6502); break;
    case 0x4a: implied6502(m6502); break;
    case 0x4b: implied6502(m6502); break;
    case 0x4c: abs6502(m6502); break;
    case 0x4d: abs6502(m6502); break;
    case 0x4e: abs6502(m6502); break;
    case 0x4f: implied6502(m6502); break;
    case 0x50: relative6502(m6502); break;
    case 0x51: indy6502(m6502); break;
    case 0x52: indzp6502(m6502); break;
    case 0x53: implied6502(m6502); break;
    case 0x54: implied6502(m6502); break;
    case 0x55: zpx6502(m6502); break;
    case 0x56: zpx6502(m6502); break;
    case 0x57: implied6502(m6502); break;
    case 0x58: implied6502(m6502); break;
    case 0x59: absy6502(m6502); break;
    case 0x5a: implied6502(m6502); break;
    case 0x5b: implied6502(m6502); break;
    case 0x5c: implied6502(m6502); break;
    case 0x5d: absx6502(m6502); break;
    case 0x5e: absx6502(m6502); break;
    case 0x5f: implied6502(m6502); break;
    case 0x60: implied6502(m6502); break;
    case 0x61: indx6502(m6502); break;
    case 0x62: implied6502(m6502); break;
    case 0x63: implied6502(m6502); break;
    case 0x64: zp6502(m6502); break;
    case 0x65: zp6502(m6502); break;
    case 0x66: zp6502(m6502); break;
    case 0x67: implied6502(m6502); break;
    case 0x68: implied6502(m6502); break;
    case 0x69: immediate6502(m6502); break;
    case 0x6a: implied6502(m6502); break;
    case 0x6b: implied6502(m6502); break;
    case 0x6c: indirect6502(m6502); break;
    case 0x6d: abs6502(m6502); break;
    case 0x6e: abs6502(m6502); break;
    case 0x6f: implied6502(m6502); break;
    case 0x70: relative6502(m6502); break;
    case 0x71: indy6502(m6502); break;
    case 0x72: indzp6502(m6502); break;
    case 0x73: implied6502(m6502); break;
    case 0x74: zpx6502(m6502); break;
    case 0x75: zpx6502(m6502); break;
    case 0x76: zpx6502(m6502); break;
    case 0x77: implied6502(m6502); break;
    case 0x78: implied6502(m6502); break;
    case 0x79: absy6502(m6502); break;
    case 0x7a: implied6502(m6502); break;
    case 0x7b: implied6502(m6502); break;
    case 0x7c: indabsx6502(m6502); break;
    case 0x7d: absx6502(m6502); break;
    case 0x7e: absx6502(m6502); break;
    case 0x7f: implied6502(m6502); break;
    case 0x80: relative6502(m6502); break;
    case 0x81: indx6502(m6502); break;
    case 0x82: implied6502(m6502); break;
    case 0x83: implied6502(m6502); break;
    case 0x84: zp6502(m6502); break;
    case 0x85: zp6502(m6502); break;
    case 0x86: zp6502(m6502); break;
    case 0x87: implied6502(m6502); break;
    case 0x88: implied6502(m6502); break;
    case 0x89: immediate6502(m6502); break;
    case 0x8a: implied6502(m6502); break;
    case 0x8b: implied6502(m6502); break;
    case 0x8c: abs6502(m6502); break;
    case 0x8d: abs6502(m6502); break;
    case 0x8e: abs6502(m6502); break;
    case 0x8f: implied6502(m6502); break;
    case 0x90: relative6502(m6502); break;
    case 0x91: indy6502(m6502); break;
    case 0x92: indzp6502(m6502); break;
    case 0x93: implied6502(m6502); break;
    case 0x94: zpx6502(m6502); break;
    case 0x95: zpx6502(m6502); break;
    case 0x96: zpy6502(m6502); break;
    case 0x97: implied6502(m6502); break;
    case 0x98: implied6502(m6502); break;
    case 0x99: absy6502(m6502); break;
    case 0x9a: implied6502(m6502); break;
    case 0x9b: implied6502(m6502); break;
    case 0x9c: abs6502(m6502); break;
    case 0x9d: absx6502(m6502); break;
    case 0x9e: absx6502(m6502); break;
    case 0x9f: implied6502(m6502); break;
    case 0xa0: immediate6502(m6502); break;
    case 0xa1: indx6502(m6502); break;
    case 0xa2: immediate6502(m6502); break;
    case 0xa3: implied6502(m6502); break;
    case 0xa4: zp6502(m6502); break;
    case 0xa5: zp6502(m6502); break;
    case 0xa6: zp6502(m6502); break;
    case 0xa7: implied6502(m6502); break;
    case 0xa8: implied6502(m6502); break;
    case 0xa9: immediate6502(m6502); break;
    case 0xaa: implied6502(m6502); break;
    case 0xab: implied6502(m6502); break;
    case 0xac: abs6502(m6502); break;
    case 0xad: abs6502(m6502); break;
    case 0xae: abs6502(m6502); break;
    case 0xaf: implied6502(m6502); break;
    case 0xb0: relative6502(m6502); break;
    case 0xb1: indy6502(m6502); break;
    case 0xb2: indzp6502(m6502); break;
    case 0xb3: implied6502(m6502); break;
    case 0xb4: zpx6502(m6502); break;
    case 0xb5: zpx6502(m6502); break;
    case 0xb6: zpy6502(m6502); break;
    case 0xb7: implied6502(m6502); break;
    case 0xb8: implied6502(m6502); break;
    case 0xb9: absy6502(m6502); break;
    case 0xba: implied6502(m6502); break;
    case 0xbb: implied6502(m6502); break;
    case 0xbc: absx6502(m6502); break;
    case 0xbd: absx6502(m6502); break;
    case 0xbe: absy6502(m6502); break;
    case 0xbf: implied6502(m6502); break;
    case 0xc0: immediate6502(m6502); break;
    case 0xc1: indx6502(m6502); break;
    case 0xc2: implied6502(m6502); break;
    case 0xc3: implied6502(m6502); break;
    case 0xc4: zp6502(m6502); break;
    case 0xc5: zp6502(m6502); break;
    case 0xc6: zp6502(m6502); break;
    case 0xc7: implied6502(m6502); break;
    case 0xc8: implied6502(m6502); break;
    case 0xc9: immediate6502(m6502); break;
    case 0xca: implied6502(m6502); break;
    case 0xcb: implied6502(m6502); break;
    case 0xcc: abs6502(m6502); break;
    case 0xcd: abs6502(m6502); break;
    case 0xce: abs6502(m6502); break;
    case 0xcf: implied6502(m6502); break;
    case 0xd0: relative6502(m6502); break;
    case 0xd1: indy6502(m6502); break;
    case 0xd2: indzp6502(m6502); break;
    case 0xd3: implied6502(m6502); break;
    case 0xd4: implied6502(m6502); break;
    case 0xd5: zpx6502(m6502); break;
    case 0xd6: zpx6502(m6502); break;
    case 0xd7: implied6502(m6502); break;
    case 0xd8: implied6502(m6502); break;
    case 0xd9: absy6502(m6502); break;
    case 0xda: implied6502(m6502); break;
    case 0xdb: implied6502(m6502); break;
    case 0xdc: implied6502(m6502); break;
    case 0xdd: absx6502(m6502); break;
    case 0xde: absx6502(m6502); break;
    case 0xdf: implied6502(m6502); break;
    case 0xe0: immediate6502(m6502); break;
    case 0xe1: indx6502(m6502); break;
    case 0xe2: implied6502(m6502); break;
    case 0xe3: implied6502(m6502); break;
    case 0xe4: zp6502(m6502); break;
    case 0xe5: zp6502(m6502); break;
    case 0xe6: zp6502(m6502); break;
    case 0xe7: implied6502(m6502); break;
    case 0xe8: implied6502(m6502); break;
    case 0xe9: immediate6502(m6502); break;
    case 0xea: implied6502(m6502); break;
    case 0xeb: implied6502(m6502); break;
    case 0xec: abs6502(m6502); break;
    case 0xed: abs6502(m6502); break;
    case 0xee: abs6502(m6502); break;
    case 0xef: implied6502(m6502); break;
    case 0xf0: relative6502(m6502); break;
    case 0xf1: indy6502(m6502); break;
    case 0xf2: indzp6502(m6502); break;
    case 0xf3: implied6502(m6502); break;
    case 0xf4: implied6502(m6502); break;
    case 0xf5: zpx6502(m6502); break;
    case 0xf6: zpx6502(m6502); break;
    case 0xf7: implied6502(m6502); break;
    case 0xf8: implied6502(m6502); break;
    case 0xf9: absy6502(m6502); break;
    case 0xfa: implied6502(m6502); break;
    case 0xfb: implied6502(m6502); break;
    case 0xfc: implied6502(m6502); break;
    case 0xfd: absx6502(m6502); break;
    case 0xfe: absx6502(m6502); break;
    case 0xff: implied6502(m6502); break;
  }
}
