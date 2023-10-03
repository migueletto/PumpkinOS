#include <stdio.h>

#include "sys.h"
#include "pwindow.h"
#include "filter.h"
#include "computer.h"
#include "z80.h"
#include "z80disasm.h"
#include "xalloc.h"
#include "debug.h"

#define FLAG_C   0x01
#define FLAG_N   0x02
#define FLAG_P   0x04
#define FLAG_X   0x08
#define FLAG_H   0x10
#define FLAG_Y   0x20
#define FLAG_Z   0x40
#define FLAG_S   0x80

#define getOpcode(a) mem_getop(z,a)
#define getByte(a) mem_getb(z,a)
#define getWord(a) mem_getw(z,a)
#define putByte(a,b) mem_putb(z,a,b)
#define putWord(a,b) mem_putw(z,a,b)

#define input(z, addr) io_in(z, (addr) & 0xFFFF)
#define output(z, addr, b) io_out(z, (addr) & 0xFFFF, b)

#define parity(n) PARTAB[(n) & 0xff]

#define lreg(x) ((x) & 0xff)
#define hreg(x) (((x) >> 8) & 0xff)
#define ldig(x) ((x) & 0x0f)
#define hdig(x) (((x) >> 4) & 0x0f)
#define setlreg(x, v) (((x) & 0xff00) | ((v) & 0xff))
#define sethreg(x, v) (((x) & 0xff) | (((v) & 0xff) << 8))

static const int CYCLES[] = {
  4,10, 7, 6, 4, 4, 7, 4, 4,11, 7, 6, 4, 4, 7, 4,
  8,10, 7, 6, 4, 4, 7, 4,12,11, 7, 6, 4, 4, 7, 4,
  7,10,16, 6, 4, 4, 7, 4, 7,11,16, 6, 4, 4, 7, 4,
  7,10,13, 6,11,11,10, 4, 7,11,13, 6, 4, 4, 7, 4,
  4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
  4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
  4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
  7, 7, 7, 7, 7, 7, 4, 7, 4, 4, 4, 4, 4, 4, 7, 4,
  4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
  4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
  4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
  4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
  5,10,10,10,10,11, 7,11, 5,10,10, 0,10,17, 7,11,
  5,10,10,11,10,11, 7,11, 5, 4,10,11,10, 0, 7,11,
  5,10,10,19,10,11, 7,11, 5, 4,10, 4,10, 0, 7,11,
  5,10,10, 4,10,11, 7,11, 5, 6,10, 4,10, 0, 7,11
};

static const int cycles_ed_opcode[] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 12, 15, 20, 8, 8, 8, 9, 12, 12, 15,
  20, 8, 8, 8, 9, 12, 12, 15, 20, 8, 8, 8, 9, 12, 12, 15, 20, 8, 8, 8, 9, 12,
  12, 15, 20, 8, 8, 8, 18, 12, 12, 15, 20, 8, 8, 8, 18, 12, 12, 15, 20, 8, 8,
  8, 8, 12, 12, 15, 20, 8, 8, 8, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 16, 16, 16, 16, 8, 8,
  8, 8, 16, 16, 16, 16, 8, 8, 8, 8, 16, 16, 16, 16, 8, 8, 8, 8, 16, 16, 16, 16,
  8, 8, 8, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static const int cycles_dd_opcode[] = {
  4, 4, 4, 4, 4, 4, 4, 4, 4, 15, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
  15, 4, 4, 4, 4, 4, 4, 4, 14, 20, 10, 8, 8, 11, 4, 4, 15, 20, 10, 8, 8, 11, 4,
  4, 4, 4, 4, 23, 23, 19, 4, 4, 15, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 8, 8, 19, 4,
  4, 4, 4, 4, 8, 8, 19, 4, 4, 4, 4, 4, 8, 8, 19, 4, 4, 4, 4, 4, 8, 8, 19, 4, 8,
  8, 8, 8, 8, 8, 19, 8, 8, 8, 8, 8, 8, 8, 19, 8, 19, 19, 19, 19, 19, 19, 4, 19,
  4, 4, 4, 4, 8, 8, 19, 4, 4, 4, 4, 4, 8, 8, 19, 4, 4, 4, 4, 4, 8, 8, 19, 4, 4,
  4, 4, 4, 8, 8, 19, 4, 4, 4, 4, 4, 8, 8, 19, 4, 4, 4, 4, 4, 8, 8, 19, 4, 4, 4,
  4, 4, 8, 8, 19, 4, 4, 4, 4, 4, 8, 8, 19, 4, 4, 4, 4, 4, 8, 8, 19, 4, 4, 4, 4,
  4, 4, 4, 4, 4, 4, 4, 4, 0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
  4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
  4, 4, 4, 4, 4, 4, 4, 4, 4,
};

static const int cycles_cb_opcode[] = {
  8, 8, 8, 8, 8, 8, 15, 8, 8, 8, 8, 8, 8, 8, 15, 8, 8, 8, 8, 8, 8, 8, 15, 8, 8,
  8, 8, 8, 8, 8, 15, 8, 8, 8, 8, 8, 8, 8, 15, 8, 8, 8, 8, 8, 8, 8, 15, 8, 8, 8,
  8, 8, 8, 8, 15, 8, 8, 8, 8, 8, 8, 8, 15, 8, 8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8,
  8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8,
  8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8,
  8, 12, 8, 8, 8, 8, 8, 8, 8, 15, 8, 8, 8, 8, 8, 8, 8, 15, 8, 8, 8, 8, 8, 8, 8,
  15, 8, 8, 8, 8, 8, 8, 8, 15, 8, 8, 8, 8, 8, 8, 8, 15, 8, 8, 8, 8, 8, 8, 8,
  15, 8, 8, 8, 8, 8, 8, 8, 15, 8, 8, 8, 8, 8, 8, 8, 15, 8, 8, 8, 8, 8, 8, 8,
  15, 8, 8, 8, 8, 8, 8, 8, 15, 8, 8, 8, 8, 8, 8, 8, 15, 8, 8, 8, 8, 8, 8, 8,
  15, 8, 8, 8, 8, 8, 8, 8, 15, 8, 8, 8, 8, 8, 8, 8, 15, 8, 8, 8, 8, 8, 8, 8,
  15, 8, 8, 8, 8, 8, 8, 8, 15, 8, 
};

static const int PARTAB[] = {
  4,0,0,4,0,4,4,0,0,4,4,0,4,0,0,4,
  0,4,4,0,4,0,0,4,4,0,0,4,0,4,4,0,
  0,4,4,0,4,0,0,4,4,0,0,4,0,4,4,0,
  4,0,0,4,0,4,4,0,0,4,4,0,4,0,0,4,
  0,4,4,0,4,0,0,4,4,0,0,4,0,4,4,0,
  4,0,0,4,0,4,4,0,0,4,4,0,4,0,0,4,
  4,0,0,4,0,4,4,0,0,4,4,0,4,0,0,4,
  0,4,4,0,4,0,0,4,4,0,0,4,0,4,4,0,
  0,4,4,0,4,0,0,4,4,0,0,4,0,4,4,0,
  4,0,0,4,0,4,4,0,0,4,4,0,4,0,0,4,
  4,0,0,4,0,4,4,0,0,4,4,0,4,0,0,4,
  0,4,4,0,4,0,0,4,4,0,0,4,0,4,4,0,
  4,0,0,4,0,4,4,0,0,4,4,0,4,0,0,4,
  0,4,4,0,4,0,0,4,4,0,0,4,0,4,4,0,
  0,4,4,0,4,0,0,4,4,0,0,4,0,4,4,0,
  4,0,0,4,0,4,4,0,0,4,4,0,4,0,0,4
};

struct z80_t {
  int z80_af[2];
  int z80_bc[2];
  int z80_de[2];
  int z80_hl[2];
  int z80_IR;
  int z80_IX;
  int z80_IY;
  int z80_SP;
  int z80_PC;
  int z80_IFF1, z80_IFF2;
  int z80_sela;
  int z80_selr;
  int z80_IM;
  int z80_irqState;
  int z80_nmiState;
  int z80_waitNextOpcode;
  int z80_halt;
  int z80_stop;
  int32_t z80_iCount;
  uint32_t z80_opCycles;
  uint32_t z80_extraCycles;
  uint32_t z80_eventCount;
  uint32_t z80_callbackCycles;
  uint32_t z80_irqCycles;
  uint32_t z80_nextIrq;
  uint32_t z80_irqVector;
  uint32_t z80_period;
  uint32_t z80_multiplier;
  int halt_exits;
  int z80_debug;
  void (*z80_callback)(void *data, uint32_t cycles);
  void *z80_data;
  computer_t *z80_computer;
};

#define af z->z80_af
#define bc z->z80_bc
#define de z->z80_de
#define hl z->z80_hl
#define IR z->z80_IR
#define IX z->z80_IX
#define IY z->z80_IY
#define SP z->z80_SP
#define PC z->z80_PC
#define IFF1 z->z80_IFF1
#define IFF2 z->z80_IFF2
#define sela z->z80_sela
#define selr z->z80_selr
#define IM z->z80_IM
#define irqState z->z80_irqState
#define nmiState z->z80_nmiState
#define waitNextOpcode z->z80_waitNextOpcode
#define halt z->z80_halt
#define stop z->z80_stop
#define iCount z->z80_iCount
#define opCycles z->z80_opCycles
#define extraCycles z->z80_extraCycles
#define eventCount z->z80_eventCount
#define callbackCycles z->z80_callbackCycles
#define irqCycles z->z80_irqCycles
#define nextIrq z->z80_nextIrq
#define irqVector z->z80_irqVector
#define period z->z80_period
#define multiplier z->z80_multiplier
#define callback z->z80_callback
#define data z->z80_data
#define computer z->z80_computer

static uint8_t mem_getop(z80_t *z, uint16_t addr) {
  return computer->getop(computer, addr);
}

static uint8_t mem_getb(z80_t *z, uint16_t addr) {
  return computer->getb(computer, addr);
}

static void mem_putb(z80_t *z, uint16_t addr, uint8_t b) {
  computer->putb(computer, addr, b);
}

static uint16_t mem_getw(z80_t *z, uint16_t addr) {
  return mem_getb(z, addr) | ((uint16_t)mem_getb(z, addr+1) << 8);
}

static void mem_putw(z80_t *z, uint16_t addr, uint16_t b) {
  mem_putb(z, addr, b);
  mem_putb(z, addr+1, b >> 8);
}

static void io_out(z80_t *z, uint16_t port, uint8_t b) {
  if (computer->out) {
    computer->out(computer, port, b);
  }
}

static uint8_t io_in(z80_t *z, uint16_t port) {
  return computer->in ? computer->in(computer, port) : 0;
}

static void SETFLAG(z80_t *z, int f, int c) {
  af[sela] = (c != 0) ? af[sela] | f : af[sela] & ~f;
}

/*
static int TSTFLAG(int f) {
  return ((af[sela] & f) != 0) ? 1 : 0;
}
*/
#define TSTFLAG(f) (((af[sela] & f) != 0) ? 1 : 0)

static void PUSH(z80_t *z, int x) {
  SP--;
  putByte(SP, x >> 8);
  SP--;
  putByte(SP, x);
}

static int POP(z80_t *z) {
  int y = getByte(SP);
  SP++;
  int x = y + (getByte(SP) << 8);
  SP++;
  return x;
}

void z80_irq(z80_t *z) {
  irqState = 1;
  irqCycles = 0;
  //fprintf(stderr, "z80_irq %d (IFF1=%d)\n", eventCount, IFF1);
}

void z80_irqM2(z80_t *z, uint8_t vector) {
  irqVector = vector;
  nextIrq = eventCount + 10;
  //fprintf(stderr, "z80_irqM2 nextIrq = %d\n", nextIrq);
}

void z80_irqn(z80_t *z, uint32_t cycles) {
  nextIrq = eventCount + cycles;
}

void z80_nmi(z80_t *z, int _waitNextOpcode) {
  waitNextOpcode = _waitNextOpcode;
  nmiState = 1;
}

static void checkIrqLine(z80_t *z) {
  if (nextIrq > 0 && eventCount >= nextIrq) {
    nextIrq = 0;
    z80_irq(z);
  }

  //if (halt) fprintf(stderr, "halted irqState=%d IFF1=%d\n", irqState, IFF1);
  if (irqState != 0 && IFF1 != 0) {
    int vector = 0xFF;
    halt = 0;

    IFF1 = IFF2 = 0;

    switch (IM) {
      case 0:
        PUSH(z, PC);
        PC = vector & 0x0038;
        break;
      case 1:
        PUSH(z, PC);
        PC = 0x0038;
        extraCycles += CYCLES[0xFF] + 2;
        break;
      case 2:
        vector = (irqVector & 0x00FF) | (IR & 0xFF00);
        PUSH(z, PC);
        PC = getWord(vector);
        //fprintf(stderr, "IRQ mode 2, vector = 0x%04X, new PC = 0x%04X\n", vector, PC);
        extraCycles += CYCLES[0xCD];
    }

    irqState = 0;
  }
}

static void checkNmiLine(z80_t *z) {
  if (nmiState != 0) {
    if (waitNextOpcode) {
      waitNextOpcode = 0;
      return;
    }
    halt = 0;
    IFF1 = 0;
    PUSH(z, PC);
    PC = 0x66;
    extraCycles += 11;
    nmiState = 0;
  }
}

static void cbshflg1(z80_t *z, int temp, int cbits) {
  af[sela] = (af[sela] & ~0xff) | (temp & 0xa8) | ((((temp & 0xff) == 0) ? 1 : 0) << 6) |  parity(temp) | (cbits != 0 ? 1 : 0) ; //!!cbits;
}

static void cbPrefix(z80_t *z, int adr) {
  int cbits;

  int temp = 0;
  int acu = 0;
  int op = getByte(PC);
  opCycles += cycles_cb_opcode[op];

  switch (op & 0x07) {
    case 0: PC++; acu = hreg(bc[selr]); break;
    case 1: PC++; acu = lreg(bc[selr]); break;
    case 2: PC++; acu = hreg(de[selr]); break;
    case 3: PC++; acu = lreg(de[selr]); break;
    case 4: PC++; acu = hreg(hl[selr]); break;
    case 5: PC++; acu = lreg(hl[selr]); break;
    case 6: PC++; acu = getByte(adr);   break;
    case 7: PC++; acu = hreg(af[sela]); break;
  }

  switch (op & 0xc0) {
  case 0x00:    /* shift/rotate */
    switch (op & 0x38) {
    case 0x00:  /* RLC */
      temp = (acu << 1) | (acu >> 7);
      cbits = temp & 1;
      cbshflg1(z, temp, cbits);
      break;
    case 0x08:  /* RRC */
      temp = (acu >> 1) | (acu << 7);
      cbits = temp & 0x80;
      cbshflg1(z, temp, cbits);
      break;
    case 0x10:  /* RL */
      temp = (acu << 1) | TSTFLAG(FLAG_C);
      cbits = acu & 0x80;
      cbshflg1(z, temp, cbits);
      break;
    case 0x18:  /* RR */
      temp = (acu >> 1) | (TSTFLAG(FLAG_C) << 7);
      cbits = acu & 1;
      cbshflg1(z, temp, cbits);
      break;
    case 0x20:  /* SLA */
      temp = acu << 1;
      cbits = acu & 0x80;
      cbshflg1(z, temp, cbits);
      break;
    case 0x28:  /* SRA */
      temp = (acu >> 1) | (acu & 0x80);
      cbits = acu & 1;
      cbshflg1(z, temp, cbits);
      break;
    case 0x30:  /* SLIA */
      temp = (acu << 1) | 1;
      cbits = acu & 0x80;
      cbshflg1(z, temp, cbits);
      break;
    case 0x38:  /* SRL */
      temp = acu >> 1;
      cbits = acu & 1;
      cbshflg1(z, temp, cbits);
    }
    break;
  case 0x40:    /* BIT */
    if ((acu & (1 << ((op >> 3) & 7))) != 0) {
      af[sela] = (af[sela] & ~0xfe) | 0x10 | ((((op & 0x38) == 0x38) ? 1 : 0) << 7);
    } else {
      af[sela] = (af[sela] & ~0xfe) | 0x54;
    }
    if ((op & 7) != 6) {
      af[sela] |= (acu & 0x28);
    }
    temp = acu;
    break;
  case 0x80:    /* RES */
    temp = acu & ~(1 << ((op >> 3) & 7));
    break;
  case 0xc0:    /* SET */
    temp = acu | (1 << ((op >> 3) & 7));
    break;
  }

  switch (op & 0x07) {
    case 0: bc[selr] = sethreg(bc[selr], temp); break;
    case 1: bc[selr] = setlreg(bc[selr], temp); break;
    case 2: de[selr] = sethreg(de[selr], temp); break;
    case 3: de[selr] = setlreg(de[selr], temp); break;
    case 4: hl[selr] = sethreg(hl[selr], temp); break;
    case 5: hl[selr] = setlreg(hl[selr], temp); break;
    case 6: putByte(adr, temp); break;
    case 7: af[sela] = sethreg(af[sela], temp); break;
  }
}

static int dfdPrefix(z80_t *z, int IXY) {
  int temp, adr, acu, op, sum, cbits;

  PC++;
  op = getByte(PC-1);
  opCycles += cycles_dd_opcode[op];

  switch (op) {
  case 0x09:      /* ADD IXY,bc[selr] */
    IXY &= 0xffff;
    bc[selr] &= 0xffff;
    sum = IXY + bc[selr];
    cbits = (IXY ^ bc[selr] ^ sum) >> 8;
  IXY = sum;
  af[sela] = (af[sela] & ~0x3b) | ((sum >> 8) & 0x28) |
      (cbits & 0x10) | ((cbits >> 8) & 1);
  break;
  case 0x19:      /* ADD IXY,de[selr] */
    IXY &= 0xffff;
    de[selr] &= 0xffff;
    sum = IXY + de[selr];
    cbits = (IXY ^ de[selr] ^ sum) >> 8;
      IXY = sum;
      af[sela] = (af[sela] & ~0x3b) | ((sum >> 8) & 0x28) |
          (cbits & 0x10) | ((cbits >> 8) & 1);
      break;
      case 0x21:      /* LD IXY,nnnn */
        IXY = getWord(PC);
        PC += 2;
        break;
      case 0x22:      /* LD (nnnn),IXY */
        temp = getWord(PC);
        putWord(temp, IXY);
        PC += 2;
        break;
      case 0x23:      /* INC IXY */
        ++IXY;
        break;
      case 0x24:      /* INC IXYH */
        IXY += 0x100;
        temp = hreg(IXY);
        af[sela] = (af[sela] & ~0xfe) | (temp & 0xa8) |
            (((temp & 0xff) == 0 ? 1 : 0) << 6) |
            (((temp & 0xf) == 0 ? 1 : 0) << 4) |
            ((temp == 0x80 ? 1 : 0) << 2);
        break;
      case 0x25:      /* DEC IXYH */
        IXY -= 0x100;
        temp = hreg(IXY);
        af[sela] = (af[sela] & ~0xfe) | (temp & 0xa8) |
            (((temp & 0xff) == 0 ? 1 : 0) << 6) |
            (((temp & 0xf) == 0xf ? 1 : 0) << 4) |
            ((temp == 0x7f ? 1 : 0) << 2) | 2;
        break;
      case 0x26:      /* LD IXYH,nn */
        IXY = sethreg(IXY, getByte(PC)); ++PC;
        break;
      case 0x29:      /* ADD IXY,IXY */
        IXY &= 0xffff;
        sum = IXY + IXY;
        cbits = (IXY ^ IXY ^ sum) >> 8;
            IXY = sum;
            af[sela] = (af[sela] & ~0x3b) | ((sum >> 8) & 0x28) |
                (cbits & 0x10) | ((cbits >> 8) & 1);
            break;
            case 0x2A:      /* LD IXY,(nnnn) */
              temp = getWord(PC);
              IXY = getWord(temp);
              PC += 2;
              break;
            case 0x2B:      /* DEC IXY */
              --IXY;
              break;
            case 0x2C:      /* INC IXYL */
              temp = lreg(IXY)+1;
              IXY = setlreg(IXY, temp);
              af[sela] = (af[sela] & ~0xfe) | (temp & 0xa8) |
                  (((temp & 0xff) == 0 ? 1 : 0) << 6) |
                  (((temp & 0xf) == 0 ? 1 : 0) << 4) |
                  ((temp == 0x80 ? 1 : 0) << 2);
              break;
            case 0x2D:      /* DEC IXYL */
              temp = lreg(IXY)-1;
              IXY = setlreg(IXY, temp);
              af[sela] = (af[sela] & ~0xfe) | (temp & 0xa8) |
                  (((temp & 0xff) == 0 ? 1 : 0) << 6) |
                  (((temp & 0xf) == 0xf ? 1 : 0) << 4) |
                  ((temp == 0x7f ? 1 : 0) << 2) | 2;
              break;
            case 0x2E:      /* LD IXYL,nn */
              IXY = setlreg(IXY, getByte(PC)); ++PC;
              break;
            case 0x34:      /* INC (IXY+dd) */
              adr = IXY + (char)getByte(PC); ++PC;
              temp = getByte(adr)+1;
              putByte(adr, temp);
              af[sela] = (af[sela] & ~0xfe) | (temp & 0xa8) |
                  (((temp & 0xff) == 0 ? 1 : 0) << 6) |
                  (((temp & 0xf) == 0 ? 1 : 0) << 4) |
                  ((temp == 0x80 ? 1 : 0) << 2);
              break;
            case 0x35:      /* DEC (IXY+dd) */
              adr = IXY + (char)getByte(PC); ++PC;
              temp = getByte(adr)-1;
              putByte(adr, temp);
              af[sela] = (af[sela] & ~0xfe) | (temp & 0xa8) |
                  (((temp & 0xff) == 0 ? 1 : 0) << 6) |
                  (((temp & 0xf) == 0xf ? 1 : 0) << 4) |
                  ((temp == 0x7f ? 1 : 0) << 2) | 2;
              break;
            case 0x36:      /* LD (IXY+dd),nn */
              adr = IXY + (char)getByte(PC); ++PC;
              putByte(adr, getByte(PC)); ++PC;
              break;
            case 0x39:      /* ADD IXY,SP */
              IXY &= 0xffff;
              SP &= 0xffff;
              sum = IXY + SP;
              cbits = (IXY ^ SP ^ sum) >> 8;
                  IXY = sum;
                  af[sela] = (af[sela] & ~0x3b) | ((sum >> 8) & 0x28) |
                      (cbits & 0x10) | ((cbits >> 8) & 1);
                  break;
                  case 0x44:      /* LD B,IXYH */
                    bc[selr] = sethreg(bc[selr], hreg(IXY));
                    break;
                  case 0x45:      /* LD B,IXYL */
                    bc[selr] = sethreg(bc[selr], lreg(IXY));
                    break;
                  case 0x46:      /* LD B,(IXY+dd) */
                    adr = IXY + (char)getByte(PC); ++PC;
                    bc[selr] = sethreg(bc[selr], getByte(adr));
                    break;
                  case 0x4C:      /* LD C,IXYH */
                    bc[selr] = setlreg(bc[selr], hreg(IXY));
                    break;
                  case 0x4D:      /* LD C,IXYL */
                    bc[selr] = setlreg(bc[selr], lreg(IXY));
                    break;
                  case 0x4E:      /* LD C,(IXY+dd) */
                    adr = IXY + (char)getByte(PC); ++PC;
                    bc[selr] = setlreg(bc[selr], getByte(adr));
                    break;
                  case 0x54:      /* LD D,IXYH */
                    de[selr] = sethreg(de[selr], hreg(IXY));
                    break;
                  case 0x55:      /* LD D,IXYL */
                    de[selr] = sethreg(de[selr], lreg(IXY));
                    break;
                  case 0x56:      /* LD D,(IXY+dd) */
                    adr = IXY + (char)getByte(PC); ++PC;
                    de[selr] = sethreg(de[selr], getByte(adr));
                    break;
                  case 0x5C:      /* LD E,H */
                    de[selr] = setlreg(de[selr], hreg(IXY));
                    break;
                  case 0x5D:      /* LD E,L */
                    de[selr] = setlreg(de[selr], lreg(IXY));
                    break;
                  case 0x5E:      /* LD E,(IXY+dd) */
                    adr = IXY + (char)getByte(PC); ++PC;
                    de[selr] = setlreg(de[selr], getByte(adr));
                    break;
                  case 0x60:      /* LD IXYH,B */
                    IXY = sethreg(IXY, hreg(bc[selr]));
                    break;
                  case 0x61:      /* LD IXYH,C */
                    IXY = sethreg(IXY, lreg(bc[selr]));
                    break;
                  case 0x62:      /* LD IXYH,D */
                    IXY = sethreg(IXY, hreg(de[selr]));
                    break;
                  case 0x63:      /* LD IXYH,E */
                    IXY = sethreg(IXY, lreg(de[selr]));
                    break;
                  case 0x64:      /* LD IXYH,IXYH */
                    /* nop */
                    break;
                  case 0x65:      /* LD IXYH,IXYL */
                    IXY = sethreg(IXY, lreg(IXY));
                    break;
                  case 0x66:      /* LD H,(IXY+dd) */
                    adr = IXY + (char)getByte(PC); ++PC;
                    hl[selr] = sethreg(hl[selr], getByte(adr));
                    break;
                  case 0x67:      /* LD IXYH,A */
                    IXY = sethreg(IXY, hreg(af[sela]));
                    break;
                  case 0x68:      /* LD IXYL,B */
                    IXY = setlreg(IXY, hreg(bc[selr]));
                    break;
                  case 0x69:      /* LD IXYL,C */
                    IXY = setlreg(IXY, lreg(bc[selr]));
                    break;
                  case 0x6A:      /* LD IXYL,D */
                    IXY = setlreg(IXY, hreg(de[selr]));
                    break;
                  case 0x6B:      /* LD IXYL,E */
                    IXY = setlreg(IXY, lreg(de[selr]));
                    break;
                  case 0x6C:      /* LD IXYL,IXYH */
                    IXY = setlreg(IXY, hreg(IXY));
                    break;
                  case 0x6D:      /* LD IXYL,IXYL */
                    /* nop */
                    break;
                  case 0x6E:      /* LD L,(IXY+dd) */
                    adr = IXY + (char)getByte(PC); ++PC;
                    hl[selr] = setlreg(hl[selr], getByte(adr));
                    break;
                  case 0x6F:      /* LD IXYL,A */
                    IXY = setlreg(IXY, hreg(af[sela]));
                    break;
                  case 0x70:      /* LD (IXY+dd),B */
                    adr = IXY + (char)getByte(PC); ++PC;
                    putByte(adr, hreg(bc[selr]));
                    break;
                  case 0x71:      /* LD (IXY+dd),C */
                    adr = IXY + (char)getByte(PC); ++PC;
                    putByte(adr, lreg(bc[selr]));
                    break;
                  case 0x72:      /* LD (IXY+dd),D */
                    adr = IXY + (char)getByte(PC); ++PC;
                    putByte(adr, hreg(de[selr]));
                    break;
                  case 0x73:      /* LD (IXY+dd),E */
                    adr = IXY + (char)getByte(PC); ++PC;
                    putByte(adr, lreg(de[selr]));
                    break;
                  case 0x74:      /* LD (IXY+dd),H */
                    adr = IXY + (char)getByte(PC); ++PC;
                    putByte(adr, hreg(hl[selr]));
                    break;
                  case 0x75:      /* LD (IXY+dd),L */
                    adr = IXY + (char)getByte(PC); ++PC;
                    putByte(adr, lreg(hl[selr]));
                    break;
                  case 0x77:      /* LD (IXY+dd),A */
                    adr = IXY + (char)getByte(PC); ++PC;
                    putByte(adr, hreg(af[sela]));
                    break;
                  case 0x7C:      /* LD A,IXYH */
                    af[sela] = sethreg(af[sela], hreg(IXY));
                    break;
                  case 0x7D:      /* LD A,IXYL */
                    af[sela] = sethreg(af[sela], lreg(IXY));
                    break;
                  case 0x7E:      /* LD A,(IXY+dd) */
                    adr = IXY + (char)getByte(PC); ++PC;
                    af[sela] = sethreg(af[sela], getByte(adr));
                    break;
                  case 0x84:      /* ADD A,IXYH */
                    temp = hreg(IXY);
                    acu = hreg(af[sela]);
                    sum = acu + temp;
                    cbits = acu ^ temp ^ sum;
                    af[sela] = ((sum & 0xff) << 8) | (sum & 0xa8) |
                        (((sum & 0xff) == 0 ? 1 : 0) << 6) | (cbits & 0x10) |
                        (((cbits >> 6) ^ (cbits >> 5)) & 4) |
                        ((cbits >> 8) & 1);
                    break;
                  case 0x85:      /* ADD A,IXYL */
                    temp = lreg(IXY);
                    acu = hreg(af[sela]);
                    sum = acu + temp;
                    cbits = acu ^ temp ^ sum;
                    af[sela] = ((sum & 0xff) << 8) | (sum & 0xa8) |
                        (((sum & 0xff) == 0 ? 1 : 0) << 6) | (cbits & 0x10) |
                        (((cbits >> 6) ^ (cbits >> 5)) & 4) |
                        ((cbits >> 8) & 1);
                    break;
                  case 0x86:      /* ADD A,(IXY+dd) */
                    adr = IXY + (char)getByte(PC); ++PC;
                    temp = getByte(adr);
                    acu = hreg(af[sela]);
                    sum = acu + temp;
                    cbits = acu ^ temp ^ sum;
                    af[sela] = ((sum & 0xff) << 8) | (sum & 0xa8) |
                        (((sum & 0xff) == 0 ? 1 : 0) << 6) | (cbits & 0x10) |
                        (((cbits >> 6) ^ (cbits >> 5)) & 4) |
                        ((cbits >> 8) & 1);
                    break;
                  case 0x8C:      /* ADC A,IXYH */
                    temp = hreg(IXY);
                    acu = hreg(af[sela]);
                    sum = acu + temp + TSTFLAG(FLAG_C);
                    cbits = acu ^ temp ^ sum;
                    af[sela] = ((sum & 0xff) << 8) | (sum & 0xa8) |
                        (((sum & 0xff) == 0 ? 1 : 0) << 6) | (cbits & 0x10) |
                        (((cbits >> 6) ^ (cbits >> 5)) & 4) |
                        ((cbits >> 8) & 1);
                    break;
                  case 0x8D:      /* ADC A,IXYL */
                    temp = lreg(IXY);
                    acu = hreg(af[sela]);
                    sum = acu + temp + TSTFLAG(FLAG_C);
                    cbits = acu ^ temp ^ sum;
                    af[sela] = ((sum & 0xff) << 8) | (sum & 0xa8) |
                        (((sum & 0xff) == 0 ? 1 : 0) << 6) | (cbits & 0x10) |
                        (((cbits >> 6) ^ (cbits >> 5)) & 4) |
                        ((cbits >> 8) & 1);
                    break;
                  case 0x8E:      /* ADC A,(IXY+dd) */
                    adr = IXY + (char)getByte(PC); ++PC;
                    temp = getByte(adr);
                    acu = hreg(af[sela]);
                    sum = acu + temp + TSTFLAG(FLAG_C);
                    cbits = acu ^ temp ^ sum;
                    af[sela] = ((sum & 0xff) << 8) | (sum & 0xa8) |
                        (((sum & 0xff) == 0 ? 1 : 0) << 6) | (cbits & 0x10) |
                        (((cbits >> 6) ^ (cbits >> 5)) & 4) |
                        ((cbits >> 8) & 1);
                    break;
                  case 0x94:      /* SUB IXYH */
                    temp = hreg(IXY);
                    acu = hreg(af[sela]);
                    sum = acu - temp;
                    cbits = acu ^ temp ^ sum;
                    af[sela] = ((sum & 0xff) << 8) | (sum & 0xa8) |
                        (((sum & 0xff) == 0 ? 1 : 0) << 6) | (cbits & 0x10) |
                        (((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
                        ((cbits >> 8) & 1);
                    break;
                  case 0x95:      /* SUB IXYL */
                    temp = lreg(IXY);
                    acu = hreg(af[sela]);
                    sum = acu - temp;
                    cbits = acu ^ temp ^ sum;
                    af[sela] = ((sum & 0xff) << 8) | (sum & 0xa8) |
                        (((sum & 0xff) == 0 ? 1 : 0) << 6) | (cbits & 0x10) |
                        (((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
                        ((cbits >> 8) & 1);
                    break;
                  case 0x96:      /* SUB (IXY+dd) */
                    adr = IXY + (char)getByte(PC); ++PC;
                    temp = getByte(adr);
                    acu = hreg(af[sela]);
                    sum = acu - temp;
                    cbits = acu ^ temp ^ sum;
                    af[sela] = ((sum & 0xff) << 8) | (sum & 0xa8) |
                        (((sum & 0xff) == 0 ? 1 : 0) << 6) | (cbits & 0x10) |
                        (((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
                        ((cbits >> 8) & 1);
                    break;
                  case 0x9C:      /* SBC A,IXYH */
                    temp = hreg(IXY);
                    acu = hreg(af[sela]);
                    sum = acu - temp - TSTFLAG(FLAG_C);
                    cbits = acu ^ temp ^ sum;
                    af[sela] = ((sum & 0xff) << 8) | (sum & 0xa8) |
                        (((sum & 0xff) == 0 ? 1 : 0) << 6) | (cbits & 0x10) |
                        (((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
                        ((cbits >> 8) & 1);
                    break;
                  case 0x9D:      /* SBC A,IXYL */
                    temp = lreg(IXY);
                    acu = hreg(af[sela]);
                    sum = acu - temp - TSTFLAG(FLAG_C);
                    cbits = acu ^ temp ^ sum;
                    af[sela] = ((sum & 0xff) << 8) | (sum & 0xa8) |
                        (((sum & 0xff) == 0 ? 1 : 0) << 6) | (cbits & 0x10) |
                        (((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
                        ((cbits >> 8) & 1);
                    break;
                  case 0x9E:      /* SBC A,(IXY+dd) */
                    adr = IXY + (char)getByte(PC); ++PC;
                    temp = getByte(adr);
                    acu = hreg(af[sela]);
                    sum = acu - temp - TSTFLAG(FLAG_C);
                    cbits = acu ^ temp ^ sum;
                    af[sela] = ((sum & 0xff) << 8) | (sum & 0xa8) |
                        (((sum & 0xff) == 0 ? 1 : 0) << 6) | (cbits & 0x10) |
                        (((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
                        ((cbits >> 8) & 1);
                    break;
                  case 0xA4:      /* AND IXYH */
                    sum = ((af[sela] & (IXY)) >> 8) & 0xff;
                    af[sela] = (sum << 8) | (sum & 0xa8) |
                        ((sum == 0 ? 1 : 0) << 6) | 0x10 | parity(sum);
                    break;
                  case 0xA5:      /* AND IXYL */
                    sum = ((af[sela] >> 8) & IXY) & 0xff;
                    af[sela] = (sum << 8) | (sum & 0xa8) | 0x10 |
                        ((sum == 0 ? 1 : 0) << 6) | parity(sum);
                    break;
                  case 0xA6:      /* AND (IXY+dd) */
                    adr = IXY + (char)getByte(PC); ++PC;
                    sum = ((af[sela] >> 8) & getByte(adr)) & 0xff;
                    af[sela] = (sum << 8) | (sum & 0xa8) | 0x10 |
                        ((sum == 0 ? 1 : 0) << 6) | parity(sum);
                    break;
                  case 0xAC:      /* XOR IXYH */
                    sum = ((af[sela] ^ (IXY)) >> 8) & 0xff;
                    af[sela] = (sum << 8) | (sum & 0xa8) | ((sum == 0 ? 1 : 0) << 6) | parity(sum);
                    break;
                  case 0xAD:      /* XOR IXYL */
                    sum = ((af[sela] >> 8) ^ IXY) & 0xff;
                    af[sela] = (sum << 8) | (sum & 0xa8) | ((sum == 0 ? 1 : 0) << 6) | parity(sum);
                    break;
                  case 0xAE:      /* XOR (IXY+dd) */
                    adr = IXY + (char)getByte(PC); ++PC;
                    sum = ((af[sela] >> 8) ^ getByte(adr)) & 0xff;
                    af[sela] = (sum << 8) | (sum & 0xa8) | ((sum == 0 ? 1 : 0) << 6) | parity(sum);
                    break;
                  case 0xB4:      /* OR IXYH */
                    sum = ((af[sela] | (IXY)) >> 8) & 0xff;
                    af[sela] = (sum << 8) | (sum & 0xa8) | ((sum == 0 ? 1 : 0) << 6) | parity(sum);
                    break;
                  case 0xB5:      /* OR IXYL */
                    sum = ((af[sela] >> 8) | IXY) & 0xff;
                    af[sela] = (sum << 8) | (sum & 0xa8) | ((sum == 0 ? 1 : 0) << 6) | parity(sum);
                    break;
                  case 0xB6:      /* OR (IXY+dd) */
                    adr = IXY + (char)getByte(PC); ++PC;
                    sum = ((af[sela] >> 8) | getByte(adr)) & 0xff;
                    af[sela] = (sum << 8) | (sum & 0xa8) | ((sum == 0 ? 1 : 0) << 6) | parity(sum);
                    break;
                  case 0xBC:      /* CP IXYH */
                    temp = hreg(IXY);
                    af[sela] = (af[sela] & ~0x28) | (temp & 0x28);
                    acu = hreg(af[sela]);
                    sum = acu - temp;
                    cbits = acu ^ temp ^ sum;
                    af[sela] = (af[sela] & ~0xff) | (sum & 0x80) |
                        (((sum & 0xff) == 0 ? 1 : 0) << 6) | (temp & 0x28) |
                        (((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
                        (cbits & 0x10) | ((cbits >> 8) & 1);
                    break;
                  case 0xBD:      /* CP IXYL */
                    temp = lreg(IXY);
                    af[sela] = (af[sela] & ~0x28) | (temp & 0x28);
                    acu = hreg(af[sela]);
                    sum = acu - temp;
                    cbits = acu ^ temp ^ sum;
                    af[sela] = (af[sela] & ~0xff) | (sum & 0x80) |
                        (((sum & 0xff) == 0 ? 1 : 0) << 6) | (temp & 0x28) |
                        (((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
                        (cbits & 0x10) | ((cbits >> 8) & 1);
                    break;
                  case 0xBE:      /* CP (IXY+dd) */
                    adr = IXY + (char)getByte(PC); ++PC;
                    temp = getByte(adr);
                    af[sela] = (af[sela] & ~0x28) | (temp & 0x28);
                    acu = hreg(af[sela]);
                    sum = acu - temp;
                    cbits = acu ^ temp ^ sum;
                    af[sela] = (af[sela] & ~0xff) | (sum & 0x80) |
                        (((sum & 0xff) == 0 ? 1 : 0) << 6) | (temp & 0x28) |
                        (((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
                        (cbits & 0x10) | ((cbits >> 8) & 1);
                    break;
                  case 0xCB:      /* CB prefix */
                    adr = IXY + (char)getByte(PC); ++PC;
                    cbPrefix(z, adr);
                    break;
                  case 0xE1:      /* POP IXY */
                    IXY = POP(z);
                    break;
                  case 0xE3:      /* EX (SP),IXY */
                    temp = IXY; IXY = POP(z); PUSH(z, temp);
                    break;
                  case 0xE5:      /* PUSH IXY */
                    PUSH(z, IXY);
                    break;
                  case 0xE9:      /* JP (IXY) */
                    PC = IXY;
                    break;
                  case 0xF9:      /* LD SP,IXY */
                    SP = IXY;
                    break;
                  default: PC--;    /* ignore DD */
  }

  return IXY;
}

void z80_reset(z80_t *z, uint32_t pc) {
  SETFLAG(z, FLAG_Z, 1);
  IX = IY = 0xffff;
  PC = 0;
  IM = 0;
  PC = pc;

  nmiState = 0;
  irqState = 0;
  halt = 0;

  extraCycles = 0;
  eventCount = 0;
  callbackCycles = 0;
  irqCycles = 0;
  nextIrq = 0;
}

static void CALLC(z80_t *z, int cond) {
  if (cond) {            
    uint32_t adrr = getWord(PC);      
    PUSH(z, PC+2);          
    PC = adrr;          
  } else {      
    PC += 2;
  }
}

static inline void JPC(z80_t *z, int cond) {
  PC = cond ? getWord(PC) : PC+2;
}

void z80_halt(z80_t *z, int h) {
  if (halt != h) {
    halt = h;
    if (halt && callback) {
      callback(data, 0);
    }
    //fprintf(stderr, "halt=%d PC=0x%04X\n", halt, PC);
  }
}

#include "z80opcodes.c"

static uint32_t execute(z80_t *z, uint32_t cycles) {
  int ireg;

  iCount = cycles;

  do {
    if (halt) {
      ireg = 0x00; // NOP
    } else {
      ireg = getOpcode(PC);

      if (z->z80_debug) {
        fprintf(stderr, "AF=%04X BC=%04X DE=%04X HL=%04X SP=%04X IX=%04X IY=%04X CNPXHYZS=%d%d%d%d%d%d%d%d IFF12=%d%d  ",
          af[sela]&0xFFFF, bc[selr]&0xFFFF, de[selr]&0xFFFF, hl[selr]&0xFFFF, SP&0xFFFF, IX&0xFFFF, IY&0xFFFF,
          (af[sela]&FLAG_C)?1:0, (af[sela]&FLAG_N)?1:0, (af[sela]&FLAG_P)?1:0, (af[sela]&FLAG_X)?1:0,
          (af[sela]&FLAG_H)?1:0, (af[sela]&FLAG_Y)?1:0, (af[sela]&FLAG_Z)?1:0, (af[sela]&FLAG_S)?1:0, IFF1?1:0, IFF2?1:0);
        z80_disasm(computer, PC);
        fprintf(stderr, "\n");
      }

      PC++;
    }

    opCycles = CYCLES[ireg];
    optable(z, ireg);

    extraCycles = 0;
    checkNmiLine(z);
    checkIrqLine(z);

    iCount -= opCycles + extraCycles;
    irqCycles += opCycles + extraCycles;
    eventCount += opCycles + extraCycles;

    if (callback && period) {
      callbackCycles += (opCycles + extraCycles) * multiplier;

      if (callbackCycles >= period) {
        callback(data, callbackCycles);
        callbackCycles -= period;
      }
    }

  } while (iCount > 0 && !stop);

  return cycles - iCount;
}

z80_t *z80_open(uint32_t _period, uint32_t _multiplier, void (*_callback)(void *d, uint32_t cycles), void *_data, computer_t *_computer) {
  z80_t *z;

  if ((z = xcalloc(1, sizeof(z80_t))) != NULL) {
    period = _period;
    multiplier = _multiplier;
    callback = _callback;
    data = _data;
    computer = _computer;
    z80_reset(z, 0);
  }

  return z;
}

int z80_close(z80_t *z) {
  int r = -1;

  if (z) {
    xfree(z);
    r = 0;
  }

  return r;
}

uint32_t z80_get_event_count(z80_t *z) {
  return eventCount;
}

void z80_set_event_count(z80_t *z, uint32_t _eventCount) {
  eventCount = _eventCount;
}

uint32_t z80_irq_cycles(z80_t *z) {
  return irqCycles;
}

int z80_loop(z80_t *z, uint32_t cycles) {
  execute(z, cycles);
  if (stop) return 1;
  if (halt && z->halt_exits) return -1;
  return 0;
}

void z80_stop(z80_t *z) {
  z->z80_stop = 1;
}

void z80_halt_exits(z80_t *z) {
  z->halt_exits = 1;
}

void z80_debug(z80_t *z, int on) {
  z->z80_debug = on;
}

void z80_disasm_mem(z80_t *z, uint32_t addr) {
  for (; addr;) {
    addr = z80_disasm(computer, addr);
    fprintf(stderr, "\n");
  }
}

uint16_t z80_getpc(z80_t *z) {
  return PC;
}
