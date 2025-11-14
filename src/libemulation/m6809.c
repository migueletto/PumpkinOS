#include "sys.h"
#include "pwindow.h"
#include "filter.h"
#include "computer.h"
#include "m6809.h"
#include "m6809priv.h"
#include "dasm6809.h"
#include "xalloc.h"
#include "debug.h"

struct m6809_t {
  PAIR  pc;             // Program counter
  PAIR  ppc;            // Previous program counter
  PAIR  d;              // Accumulator a and b
  PAIR  dp;             // Direct Page register (page in MSB)
  PAIR  u, s;           // Stack pointers
  PAIR  x, y;           // Index registers
  PAIR  ea;

  uint32_t cc;
  uint32_t irq_state[2];
  uint32_t int_state;      // SYNC and CWAI flags
  uint32_t nmi_state;
  int32_t  extracycles;   // cycles used up by interrupts
  int32_t  count;

  uint32_t nmi_request;
  uint32_t irq_request;
  uint32_t firq_request;
  uint32_t vsync_irq;
  uint32_t totalcycles;
  uint32_t eventcount;
  uint32_t period;
  uint32_t t0;

  uint8_t *cycles;
  uint8_t *flags8i;
  uint8_t *flags8d;

  void (*callback)(void *data, uint32_t cycles);
  void *data;

  computer_t *computer;
};

static uint8_t cycles[256] = {
  6, 2, 2, 6, 6, 2, 6, 6, 6, 6, 6, 2, 6, 6, 3, 6,
  0, 0, 2, 4, 2, 2, 5, 9, 2, 2, 3, 2, 3, 2, 8, 6,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  4, 4, 4, 4, 5, 5, 5, 5, 2, 5, 3, 6, 20, 11, 2, 19,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  6, 2, 2, 6, 6, 2, 6, 6, 6, 6, 6, 2, 6, 6, 3, 6,
  7, 2, 2, 7, 7, 2, 7, 7, 7, 7, 7, 2, 7, 7, 4, 7,
  2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 2, 4, 7, 3, 2,
  4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 4, 6, 7, 5, 5,
  4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 4, 6, 7, 5, 5,
  5, 5, 5, 7, 5, 5, 5, 5, 5, 5, 5, 5, 7, 8, 6, 6,
  2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 2, 3, 2, 3, 3,
  4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5,
  4, 4, 4, 6, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5,
  5, 5, 5, 7, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6
};

static uint8_t flags8i[256] = {
  CC_Z,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  CC_N|CC_V,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
  CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
  CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
  CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
  CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
  CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
  CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
  CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N
};

static uint8_t flags8d[256] = {
  CC_Z,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,CC_V,
  CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
  CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
  CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
  CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
  CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
  CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
  CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,
  CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N,CC_N
};

static uint8_t m6809_getb(m6809_t *m6809, uint16_t addr) {
  return m6809->computer->getb(m6809->computer, addr);
}

uint8_t m6809_getbyte(void *data, uint16_t addr) {
  return m6809_getb((m6809_t *)data, addr);
}

static void m6809_putb(m6809_t *m6809, uint16_t addr, uint8_t b) {
  m6809->computer->putb(m6809->computer, addr, b);
}

static void illegal(m6809_t *m6809, uint32_t cod, uint8_t op) {
}

static void fetch_effective_address(m6809_t *m6809);

#include "m6809ops.c"

m6809_t *m6809_open(uint32_t period, void (*callback)(void *data, uint32_t cycles), void *data, computer_t *computer) {
  m6809_t *m6809;

  if ((m6809 = xcalloc(1, sizeof(m6809_t))) != NULL) {
    m6809->cycles = cycles;
    m6809->flags8i = flags8i;
    m6809->flags8d = flags8d;
    m6809->period = period;
    m6809->vsync_irq = 1;
    m6809->callback = callback;
    m6809->data = data;
    m6809->computer = computer;
  }

  return m6809;
}

int m6809_close(m6809_t *m6809) {
  int r = -1;

  if (m6809) {
    xfree(m6809);
    r = 0;
  }

  return r;
}

static void m6809_optable(uint8_t op, m6809_t *m6809) {
  switch (op) {
    case 0: neg_di(m6809); break;
    case 1: illegal(m6809, ILLEGAL0, op); break;
    case 2: illegal(m6809, ILLEGAL0, op); break;
    case 3: com_di(m6809); break;
    case 4: lsr_di(m6809); break;
    case 5: illegal(m6809, ILLEGAL0, op); break;
    case 6: ror_di(m6809); break;
    case 7: asr_di(m6809); break;
    case 8: asl_di(m6809); break;
    case 9: rol_di(m6809); break;
    case 10: dec_di(m6809); break;
    case 11: illegal(m6809, ILLEGAL0, op); break;
    case 12: inc_di(m6809); break;
    case 13: tst_di(m6809); break;
    case 14: jmp_di(m6809); break;
    case 15: clr_di(m6809); break;
    case 16: pref10(m6809); break;
    case 17: pref11(m6809); break;
    case 18: nop(m6809); break;
    case 19: sync(m6809); break;
    case 20: illegal(m6809, ILLEGAL0, op); break;
    case 21: illegal(m6809, ILLEGAL0, op); break;
    case 22: lbra(m6809); break;
    case 23: lbsr(m6809); break;
    case 24: illegal(m6809, ILLEGAL0, op); break;
    case 25: daa(m6809); break;
    case 26: orcc(m6809); break;
    case 27: illegal(m6809, ILLEGAL0, op); break;
    case 28: andcc(m6809); break;
    case 29: sex(m6809); break;
    case 30: exg(m6809); break;
    case 31: tfr(m6809); break;
    case 32: bra(m6809); break;
    case 33: brn(m6809); break;
    case 34: bhi(m6809); break;
    case 35: bls(m6809); break;
    case 36: bcc(m6809); break;
    case 37: bcs(m6809); break;
    case 38: bne(m6809); break;
    case 39: beq(m6809); break;
    case 40: bvc(m6809); break;
    case 41: bvs(m6809); break;
    case 42: bpl(m6809); break;
    case 43: bmi(m6809); break;
    case 44: bge(m6809); break;
    case 45: blt(m6809); break;
    case 46: bgt(m6809); break;
    case 47: ble(m6809); break;
    case 48: leax(m6809); break;
    case 49: leay(m6809); break;
    case 50: leas(m6809); break;
    case 51: leau(m6809); break;
    case 52: pshs(m6809); break;
    case 53: puls(m6809); break;
    case 54: pshu(m6809); break;
    case 55: pulu(m6809); break;
    case 56: illegal(m6809, ILLEGAL0, op); break;
    case 57: rts(m6809); break;
    case 58: abx(m6809); break;
    case 59: rti(m6809); break;
    case 60: cwai(m6809); break;
    case 61: mul(m6809); break;
    case 62: illegal(m6809, ILLEGAL0, op); break;
    case 63: swi(m6809); break;
    case 64: nega(m6809); break;
    case 65: illegal(m6809, ILLEGAL0, op); break;
    case 66: illegal(m6809, ILLEGAL0, op); break;
    case 67: coma(m6809); break;
    case 68: lsra(m6809); break;
    case 69: illegal(m6809, ILLEGAL0, op); break;
    case 70: rora(m6809); break;
    case 71: asra(m6809); break;
    case 72: asla(m6809); break;
    case 73: rola(m6809); break;
    case 74: deca(m6809); break;
    case 75: illegal(m6809, ILLEGAL0, op); break;
    case 76: inca(m6809); break;
    case 77: tsta(m6809); break;
    case 78: illegal(m6809, ILLEGAL0, op); break;
    case 79: clra(m6809); break;
    case 80: negb(m6809); break;
    case 81: illegal(m6809, ILLEGAL0, op); break;
    case 82: illegal(m6809, ILLEGAL0, op); break;
    case 83: comb(m6809); break;
    case 84: lsrb(m6809); break;
    case 85: illegal(m6809, ILLEGAL0, op); break;
    case 86: rorb(m6809); break;
    case 87: asrb(m6809); break;
    case 88: aslb(m6809); break;
    case 89: rolb(m6809); break;
    case 90: decb(m6809); break;
    case 91: illegal(m6809, ILLEGAL0, op); break;
    case 92: incb(m6809); break;
    case 93: tstb(m6809); break;
    case 94: illegal(m6809, ILLEGAL0, op); break;
    case 95: clrb(m6809); break;
    case 96: neg_ix(m6809); break;
    case 97: illegal(m6809, ILLEGAL0, op); break;
    case 98: illegal(m6809, ILLEGAL0, op); break;
    case 99: com_ix(m6809); break;
    case 100: lsr_ix(m6809); break;
    case 101: illegal(m6809, ILLEGAL0, op); break;
    case 102: ror_ix(m6809); break;
    case 103: asr_ix(m6809); break;
    case 104: asl_ix(m6809); break;
    case 105: rol_ix(m6809); break;
    case 106: dec_ix(m6809); break;
    case 107: illegal(m6809, ILLEGAL0, op); break;
    case 108: inc_ix(m6809); break;
    case 109: tst_ix(m6809); break;
    case 110: jmp_ix(m6809); break;
    case 111: clr_ix(m6809); break;
    case 112: neg_ex(m6809); break;
    case 113: illegal(m6809, ILLEGAL0, op); break;
    case 114: illegal(m6809, ILLEGAL0, op); break;
    case 115: com_ex(m6809); break;
    case 116: lsr_ex(m6809); break;
    case 117: illegal(m6809, ILLEGAL0, op); break;
    case 118: ror_ex(m6809); break;
    case 119: asr_ex(m6809); break;
    case 120: asl_ex(m6809); break;
    case 121: rol_ex(m6809); break;
    case 122: dec_ex(m6809); break;
    case 123: illegal(m6809, ILLEGAL0, op); break;
    case 124: inc_ex(m6809); break;
    case 125: tst_ex(m6809); break;
    case 126: jmp_ex(m6809); break;
    case 127: clr_ex(m6809); break;
    case 128: suba_im(m6809); break;
    case 129: cmpa_im(m6809); break;
    case 130: sbca_im(m6809); break;
    case 131: subd_im(m6809); break;
    case 132: anda_im(m6809); break;
    case 133: bita_im(m6809); break;
    case 134: lda_im(m6809); break;
    case 135: sta_im(m6809); break;
    case 136: eora_im(m6809); break;
    case 137: adca_im(m6809); break;
    case 138: ora_im(m6809); break;
    case 139: adda_im(m6809); break;
    case 140: cmpx_im(m6809); break;
    case 141: bsr(m6809); break;
    case 142: ldx_im(m6809); break;
    case 143: stx_im(m6809); break;
    case 144: suba_di(m6809); break;
    case 145: cmpa_di(m6809); break;
    case 146: sbca_di(m6809); break;
    case 147: subd_di(m6809); break;
    case 148: anda_di(m6809); break;
    case 149: bita_di(m6809); break;
    case 150: lda_di(m6809); break;
    case 151: sta_di(m6809); break;
    case 152: eora_di(m6809); break;
    case 153: adca_di(m6809); break;
    case 154: ora_di(m6809); break;
    case 155: adda_di(m6809); break;
    case 156: cmpx_di(m6809); break;
    case 157: jsr_di(m6809); break;
    case 158: ldx_di(m6809); break;
    case 159: stx_di(m6809); break;
    case 160: suba_ix(m6809); break;
    case 161: cmpa_ix(m6809); break;
    case 162: sbca_ix(m6809); break;
    case 163: subd_ix(m6809); break;
    case 164: anda_ix(m6809); break;
    case 165: bita_ix(m6809); break;
    case 166: lda_ix(m6809); break;
    case 167: sta_ix(m6809); break;
    case 168: eora_ix(m6809); break;
    case 169: adca_ix(m6809); break;
    case 170: ora_ix(m6809); break;
    case 171: adda_ix(m6809); break;
    case 172: cmpx_ix(m6809); break;
    case 173: jsr_ix(m6809); break;
    case 174: ldx_ix(m6809); break;
    case 175: stx_ix(m6809); break;
    case 176: suba_ex(m6809); break;
    case 177: cmpa_ex(m6809); break;
    case 178: sbca_ex(m6809); break;
    case 179: subd_ex(m6809); break;
    case 180: anda_ex(m6809); break;
    case 181: bita_ex(m6809); break;
    case 182: lda_ex(m6809); break;
    case 183: sta_ex(m6809); break;
    case 184: eora_ex(m6809); break;
    case 185: adca_ex(m6809); break;
    case 186: ora_ex(m6809); break;
    case 187: adda_ex(m6809); break;
    case 188: cmpx_ex(m6809); break;
    case 189: jsr_ex(m6809); break;
    case 190: ldx_ex(m6809); break;
    case 191: stx_ex(m6809); break;
    case 192: subb_im(m6809); break;
    case 193: cmpb_im(m6809); break;
    case 194: sbcb_im(m6809); break;
    case 195: addd_im(m6809); break;
    case 196: andb_im(m6809); break;
    case 197: bitb_im(m6809); break;
    case 198: ldb_im(m6809); break;
    case 199: stb_im(m6809); break;
    case 200: eorb_im(m6809); break;
    case 201: adcb_im(m6809); break;
    case 202: orb_im(m6809); break;
    case 203: addb_im(m6809); break;
    case 204: ldd_im(m6809); break;
    case 205: std_im(m6809); break;
    case 206: ldu_im(m6809); break;
    case 207: stu_im(m6809); break;
    case 208: subb_di(m6809); break;
    case 209: cmpb_di(m6809); break;
    case 210: sbcb_di(m6809); break;
    case 211: addd_di(m6809); break;
    case 212: andb_di(m6809); break;
    case 213: bitb_di(m6809); break;
    case 214: ldb_di(m6809); break;
    case 215: stb_di(m6809); break;
    case 216: eorb_di(m6809); break;
    case 217: adcb_di(m6809); break;
    case 218: orb_di(m6809); break;
    case 219: addb_di(m6809); break;
    case 220: ldd_di(m6809); break;
    case 221: std_di(m6809); break;
    case 222: ldu_di(m6809); break;
    case 223: stu_di(m6809); break;
    case 224: subb_ix(m6809); break;
    case 225: cmpb_ix(m6809); break;
    case 226: sbcb_ix(m6809); break;
    case 227: addd_ix(m6809); break;
    case 228: andb_ix(m6809); break;
    case 229: bitb_ix(m6809); break;
    case 230: ldb_ix(m6809); break;
    case 231: stb_ix(m6809); break;
    case 232: eorb_ix(m6809); break;
    case 233: adcb_ix(m6809); break;
    case 234: orb_ix(m6809); break;
    case 235: addb_ix(m6809); break;
    case 236: ldd_ix(m6809); break;
    case 237: std_ix(m6809); break;
    case 238: ldu_ix(m6809); break;
    case 239: stu_ix(m6809); break;
    case 240: subb_ex(m6809); break;
    case 241: cmpb_ex(m6809); break;
    case 242: sbcb_ex(m6809); break;
    case 243: addd_ex(m6809); break;
    case 244: andb_ex(m6809); break;
    case 245: bitb_ex(m6809); break;
    case 246: ldb_ex(m6809); break;
    case 247: stb_ex(m6809); break;
    case 248: eorb_ex(m6809); break;
    case 249: adcb_ex(m6809); break;
    case 250: orb_ex(m6809); break;
    case 251: addb_ex(m6809); break;
    case 252: ldd_ex(m6809); break;
    case 253: std_ex(m6809); break;
    case 254: ldu_ex(m6809); break;
    case 255: stu_ex(m6809); break;
  }
}

void m6809_reset(m6809_t *m6809) {
  m6809->int_state = 0;
  m6809->nmi_state = CLEAR_LINE;
  m6809->irq_state[0] = CLEAR_LINE;
  m6809->irq_state[1] = CLEAR_LINE;
  m6809->count = 0;
  m6809->extracycles = 0;

  m6809->eventcount = 0;
  m6809->totalcycles = 0;
  m6809->firq_request = 0;

  DPD = 0;	// Reset direct page register
  CC |= CC_II;	// IRQ disabled
  CC |= CC_IF;	// FIRQ disabled

  PC = RM16(0xfffe);
}

void m6809_setpc(m6809_t *m6809, uint16_t pc) {
  PC = pc;
}

void m6809_set_irq_line(m6809_t *m6809, int32_t irqline, int32_t state) {
  if (irqline == M6809_NMI_LINE) {
    if (m6809->nmi_state == state)
      return;

    m6809->nmi_state = state;
    if( state == CLEAR_LINE )
      return;

    // if the stack was not yet initialized
    if (!(m6809->int_state & M6809_LDS))
      return;

    m6809->int_state &= ~M6809_SYNC;

    // HJB 990225: state already saved by CWAI?
    if (m6809->int_state & M6809_CWAI) {
      m6809->int_state &= ~M6809_CWAI;
      m6809->extracycles += 7;	// subtract +7 cycles next time
    } else {
      CC |= CC_E;	// save entire state
      PUSHWORD(pPC);
      PUSHWORD(pU);
      PUSHWORD(pY);
      PUSHWORD(pX);
      PUSHBYTE(DP);
      PUSHBYTE(B);
      PUSHBYTE(A);
      PUSHBYTE(CC);
      m6809->extracycles += 19;	// subtract +19 cycles next time
    }
    CC |= CC_IF | CC_II;	// inhibit FIRQ and IRQ
    PC = RM16(0xfffc);
  } else if (irqline < 2) {
    m6809->irq_state[irqline] = state;
    if (state == CLEAR_LINE)
      return;
    CHECK_IRQ_LINES;
  }
}

/* execute instructions on this CPU until icount expires */
int32_t m6809_execute(m6809_t *m6809, int32_t cycles) {
  //char buf[256];
  uint8_t ireg;

  m6809_ICount = cycles - m6809->extracycles;
  m6809->extracycles = 0;

  if (m6809->int_state & (M6809_CWAI | M6809_SYNC))
    m6809_ICount = 0;
  else {
    do {
/*
      if (PC >= 0xF000) {
        m6809_dasm(m6809, PC, buf);
        debug(DEBUG_INFO, "6809", "A=%02X B=%02X X=%04X Y=%04X U=%04X S=%04X DP=%02X CC=%02X %s", A, B, X, Y, U, S, DP, CC, buf);
        if (PC == 0xF014) m6809->t0 = m6809->totalcycles;
        if (PC == 0xF097) {
          debug(1, "XXX", "total = %u", m6809->totalcycles - m6809->t0);
        }
      }
*/
      pPPC = pPC;
      ireg = ROP(PC);
      PC++;

      m6809_optable(ireg, m6809);
      m6809_ICount -= m6809->cycles[ireg];
      m6809->totalcycles += m6809->cycles[ireg];
      m6809->eventcount += m6809->cycles[ireg];

      if (m6809->firq_request) {
        m6809->firq_request = 0;
        m6809_set_irq_line(m6809, M6809_FIRQ_LINE, ASSERT_LINE);
        break;

      } else {
        if (m6809->totalcycles >= m6809->period) {
          m6809->callback(m6809->data, m6809->totalcycles);
          m6809->totalcycles -= m6809->period;
          if (m6809->vsync_irq)
            m6809_set_irq_line(m6809, M6809_IRQ_LINE, ASSERT_LINE);
        }
      }

    } while (m6809_ICount > 0);

    m6809_ICount -= m6809->extracycles;
    m6809->extracycles = 0;
  }

  return cycles - m6809_ICount;
}

static void fetch_effective_address(m6809_t *m6809) {
  uint8_t postbyte = ROP_ARG(PC);
  PC++;

  switch(postbyte) {
	case 0x00: EA=X;	m6809_ICount-=1;   break;
	case 0x01: EA=X+1;	m6809_ICount-=1;   break;
	case 0x02: EA=X+2;	m6809_ICount-=1;   break;
	case 0x03: EA=X+3;	m6809_ICount-=1;   break;
	case 0x04: EA=X+4;	m6809_ICount-=1;   break;
	case 0x05: EA=X+5;	m6809_ICount-=1;   break;
	case 0x06: EA=X+6;	m6809_ICount-=1;   break;
	case 0x07: EA=X+7;	m6809_ICount-=1;   break;
	case 0x08: EA=X+8;	m6809_ICount-=1;   break;
	case 0x09: EA=X+9;	m6809_ICount-=1;   break;
	case 0x0a: EA=X+10; 	m6809_ICount-=1;   break;
	case 0x0b: EA=X+11; 	m6809_ICount-=1;   break;
	case 0x0c: EA=X+12; 	m6809_ICount-=1;   break;
	case 0x0d: EA=X+13; 	m6809_ICount-=1;   break;
	case 0x0e: EA=X+14; 	m6809_ICount-=1;   break;
	case 0x0f: EA=X+15; 	m6809_ICount-=1;   break;
	case 0x10: EA=X-16; 	m6809_ICount-=1;   break;
	case 0x11: EA=X-15; 	m6809_ICount-=1;   break;
	case 0x12: EA=X-14; 	m6809_ICount-=1;   break;
	case 0x13: EA=X-13; 	m6809_ICount-=1;   break;
	case 0x14: EA=X-12; 	m6809_ICount-=1;   break;
	case 0x15: EA=X-11; 	m6809_ICount-=1;   break;
	case 0x16: EA=X-10; 	m6809_ICount-=1;   break;
	case 0x17: EA=X-9;	m6809_ICount-=1;   break;
	case 0x18: EA=X-8;	m6809_ICount-=1;   break;
	case 0x19: EA=X-7;	m6809_ICount-=1;   break;
	case 0x1a: EA=X-6;	m6809_ICount-=1;   break;
	case 0x1b: EA=X-5;	m6809_ICount-=1;   break;
	case 0x1c: EA=X-4;	m6809_ICount-=1;   break;
	case 0x1d: EA=X-3;	m6809_ICount-=1;   break;
	case 0x1e: EA=X-2;	m6809_ICount-=1;   break;
	case 0x1f: EA=X-1;	m6809_ICount-=1;   break;
	case 0x20: EA=Y;	m6809_ICount-=1;   break;
	case 0x21: EA=Y+1;	m6809_ICount-=1;   break;
	case 0x22: EA=Y+2;	m6809_ICount-=1;   break;
	case 0x23: EA=Y+3;	m6809_ICount-=1;   break;
	case 0x24: EA=Y+4;	m6809_ICount-=1;   break;
	case 0x25: EA=Y+5;	m6809_ICount-=1;   break;
	case 0x26: EA=Y+6;	m6809_ICount-=1;   break;
	case 0x27: EA=Y+7;	m6809_ICount-=1;   break;
	case 0x28: EA=Y+8;	m6809_ICount-=1;   break;
	case 0x29: EA=Y+9;	m6809_ICount-=1;   break;
	case 0x2a: EA=Y+10; 	m6809_ICount-=1;   break;
	case 0x2b: EA=Y+11; 	m6809_ICount-=1;   break;
	case 0x2c: EA=Y+12; 	m6809_ICount-=1;   break;
	case 0x2d: EA=Y+13; 	m6809_ICount-=1;   break;
	case 0x2e: EA=Y+14; 	m6809_ICount-=1;   break;
	case 0x2f: EA=Y+15; 	m6809_ICount-=1;   break;
	case 0x30: EA=Y-16; 	m6809_ICount-=1;   break;
	case 0x31: EA=Y-15; 	m6809_ICount-=1;   break;
	case 0x32: EA=Y-14; 	m6809_ICount-=1;   break;
	case 0x33: EA=Y-13; 	m6809_ICount-=1;   break;
	case 0x34: EA=Y-12; 	m6809_ICount-=1;   break;
	case 0x35: EA=Y-11; 	m6809_ICount-=1;   break;
	case 0x36: EA=Y-10; 	m6809_ICount-=1;   break;
	case 0x37: EA=Y-9;	m6809_ICount-=1;   break;
	case 0x38: EA=Y-8;	m6809_ICount-=1;   break;
	case 0x39: EA=Y-7;	m6809_ICount-=1;   break;
	case 0x3a: EA=Y-6;	m6809_ICount-=1;   break;
	case 0x3b: EA=Y-5;	m6809_ICount-=1;   break;
	case 0x3c: EA=Y-4;	m6809_ICount-=1;   break;
	case 0x3d: EA=Y-3;	m6809_ICount-=1;   break;
	case 0x3e: EA=Y-2;	m6809_ICount-=1;   break;
	case 0x3f: EA=Y-1;	m6809_ICount-=1;   break;
	case 0x40: EA=U;	m6809_ICount-=1;   break;
	case 0x41: EA=U+1;	m6809_ICount-=1;   break;
	case 0x42: EA=U+2;	m6809_ICount-=1;   break;
	case 0x43: EA=U+3;	m6809_ICount-=1;   break;
	case 0x44: EA=U+4;	m6809_ICount-=1;   break;
	case 0x45: EA=U+5;	m6809_ICount-=1;   break;
	case 0x46: EA=U+6;	m6809_ICount-=1;   break;
	case 0x47: EA=U+7;	m6809_ICount-=1;   break;
	case 0x48: EA=U+8;	m6809_ICount-=1;   break;
	case 0x49: EA=U+9;	m6809_ICount-=1;   break;
	case 0x4a: EA=U+10; 	m6809_ICount-=1;   break;
	case 0x4b: EA=U+11; 	m6809_ICount-=1;   break;
	case 0x4c: EA=U+12; 	m6809_ICount-=1;   break;
	case 0x4d: EA=U+13; 	m6809_ICount-=1;   break;
	case 0x4e: EA=U+14; 	m6809_ICount-=1;   break;
	case 0x4f: EA=U+15; 	m6809_ICount-=1;   break;
	case 0x50: EA=U-16; 	m6809_ICount-=1;   break;
	case 0x51: EA=U-15; 	m6809_ICount-=1;   break;
	case 0x52: EA=U-14; 	m6809_ICount-=1;   break;
	case 0x53: EA=U-13; 	m6809_ICount-=1;   break;
	case 0x54: EA=U-12; 	m6809_ICount-=1;   break;
	case 0x55: EA=U-11; 	m6809_ICount-=1;   break;
	case 0x56: EA=U-10; 	m6809_ICount-=1;   break;
	case 0x57: EA=U-9;	m6809_ICount-=1;   break;
	case 0x58: EA=U-8;	m6809_ICount-=1;   break;
	case 0x59: EA=U-7;	m6809_ICount-=1;   break;
	case 0x5a: EA=U-6;	m6809_ICount-=1;   break;
	case 0x5b: EA=U-5;	m6809_ICount-=1;   break;
	case 0x5c: EA=U-4;	m6809_ICount-=1;   break;
	case 0x5d: EA=U-3;	m6809_ICount-=1;   break;
	case 0x5e: EA=U-2;	m6809_ICount-=1;   break;
	case 0x5f: EA=U-1;	m6809_ICount-=1;   break;
	case 0x60: EA=S;	m6809_ICount-=1;   break;
	case 0x61: EA=S+1;	m6809_ICount-=1;   break;
	case 0x62: EA=S+2;	m6809_ICount-=1;   break;
	case 0x63: EA=S+3;	m6809_ICount-=1;   break;
	case 0x64: EA=S+4;	m6809_ICount-=1;   break;
	case 0x65: EA=S+5;	m6809_ICount-=1;   break;
	case 0x66: EA=S+6;	m6809_ICount-=1;   break;
	case 0x67: EA=S+7;	m6809_ICount-=1;   break;
	case 0x68: EA=S+8;	m6809_ICount-=1;   break;
	case 0x69: EA=S+9;	m6809_ICount-=1;   break;
	case 0x6a: EA=S+10; 	m6809_ICount-=1;   break;
	case 0x6b: EA=S+11; 	m6809_ICount-=1;   break;
	case 0x6c: EA=S+12; 	m6809_ICount-=1;   break;
	case 0x6d: EA=S+13; 	m6809_ICount-=1;   break;
	case 0x6e: EA=S+14; 	m6809_ICount-=1;   break;
	case 0x6f: EA=S+15; 	m6809_ICount-=1;   break;
	case 0x70: EA=S-16; 	m6809_ICount-=1;   break;
	case 0x71: EA=S-15; 	m6809_ICount-=1;   break;
	case 0x72: EA=S-14; 	m6809_ICount-=1;   break;
	case 0x73: EA=S-13; 	m6809_ICount-=1;   break;
	case 0x74: EA=S-12; 	m6809_ICount-=1;   break;
	case 0x75: EA=S-11; 	m6809_ICount-=1;   break;
	case 0x76: EA=S-10; 	m6809_ICount-=1;   break;
	case 0x77: EA=S-9;	m6809_ICount-=1;   break;
	case 0x78: EA=S-8;	m6809_ICount-=1;   break;
	case 0x79: EA=S-7;	m6809_ICount-=1;   break;
	case 0x7a: EA=S-6;	m6809_ICount-=1;   break;
	case 0x7b: EA=S-5;	m6809_ICount-=1;   break;
	case 0x7c: EA=S-4;	m6809_ICount-=1;   break;
	case 0x7d: EA=S-3;	m6809_ICount-=1;   break;
	case 0x7e: EA=S-2;	m6809_ICount-=1;   break;
	case 0x7f: EA=S-1;	m6809_ICount-=1;   break;

	case 0x80: EA=X;	X++;  m6809_ICount-=2;   break;
	case 0x81: EA=X;	X+=2; m6809_ICount-=3;   break;
	case 0x82: X--; 	EA=X; m6809_ICount-=2;   break;
	case 0x83: X-=2;	EA=X; m6809_ICount-=3;   break;
	case 0x84: EA=X;	break;
	case 0x85: EA=X+SIGNED(B); m6809_ICount-=1;   break;
	case 0x86: EA=X+SIGNED(A); m6809_ICount-=1;   break;
	case 0x87: EA=0; break; /*   ILLEGAL*/
	case 0x88: IMMBYTE(EA); 	EA=X+SIGNED(EA);					m6809_ICount-=1;   break; /* this is a hack to make Vectrex work. It should be m6809_ICount-=1. Dunno where the cycle was lost :( */
	case 0x89: IMMWORD(mEA); 	EA+=X;								m6809_ICount-=4;   break;
	case 0x8a: EA=0;																   break; /*   ILLEGAL*/
	case 0x8b: EA=X+D;												m6809_ICount-=4;   break;
	case 0x8c: IMMBYTE(EA); 	EA=PC+SIGNED(EA);					m6809_ICount-=1;   break;
	case 0x8d: IMMWORD(mEA); 	EA+=PC; 							m6809_ICount-=5;   break;
	case 0x8e: EA=0;																   break; /*   ILLEGAL*/
	case 0x8f: IMMWORD(mEA); 										m6809_ICount-=5;   break;

	case 0x90: EA=X;	X++;						EAD=RM16(EAD);	m6809_ICount-=5;   break; /* Indirect ,R+ not in my specs */
	case 0x91: EA=X;	X+=2;						EAD=RM16(EAD);	m6809_ICount-=6;   break;
	case 0x92: X--; 	EA=X;						EAD=RM16(EAD);	m6809_ICount-=5;   break;
	case 0x93: X-=2;	EA=X;						EAD=RM16(EAD);	m6809_ICount-=6;   break;
	case 0x94: EA=X;								EAD=RM16(EAD);	m6809_ICount-=3;   break;
	case 0x95: EA=X+SIGNED(B);						EAD=RM16(EAD);	m6809_ICount-=4;   break;
	case 0x96: EA=X+SIGNED(A);						EAD=RM16(EAD);	m6809_ICount-=4;   break;
	case 0x97: EA=0;																   break; /*   ILLEGAL*/
	case 0x98: IMMBYTE(EA); 	EA=X+SIGNED(EA);	EAD=RM16(EAD);	m6809_ICount-=4;   break;
	case 0x99: IMMWORD(mEA); 	EA+=X;				EAD=RM16(EAD);	m6809_ICount-=7;   break;
	case 0x9a: EA=0;																   break; /*   ILLEGAL*/
	case 0x9b: EA=X+D;								EAD=RM16(EAD);	m6809_ICount-=7;   break;
	case 0x9c: IMMBYTE(EA); 	EA=PC+SIGNED(EA);	EAD=RM16(EAD);	m6809_ICount-=4;   break;
	case 0x9d: IMMWORD(mEA); 	EA+=PC; 			EAD=RM16(EAD);	m6809_ICount-=8;   break;
	case 0x9e: EA=0;																   break; /*   ILLEGAL*/
	case 0x9f: IMMWORD(mEA); 						EAD=RM16(EAD);	m6809_ICount-=5;   break;

	case 0xa0: EA=Y;	Y++;										m6809_ICount-=2;   break;
	case 0xa1: EA=Y;	Y+=2;										m6809_ICount-=3;   break;
	case 0xa2: Y--; 	EA=Y;										m6809_ICount-=2;   break;
	case 0xa3: Y-=2;	EA=Y;										m6809_ICount-=3;   break;
	case 0xa4: EA=Y;																   break;
	case 0xa5: EA=Y+SIGNED(B);										m6809_ICount-=1;   break;
	case 0xa6: EA=Y+SIGNED(A);										m6809_ICount-=1;   break;
	case 0xa7: EA=0;																   break; /*   ILLEGAL*/
	case 0xa8: IMMBYTE(EA); 	EA=Y+SIGNED(EA);					m6809_ICount-=1;   break;
	case 0xa9: IMMWORD(mEA); 	EA+=Y;								m6809_ICount-=4;   break;
	case 0xaa: EA=0;																   break; /*   ILLEGAL*/
	case 0xab: EA=Y+D;												m6809_ICount-=4;   break;
	case 0xac: IMMBYTE(EA); 	EA=PC+SIGNED(EA);					m6809_ICount-=1;   break;
	case 0xad: IMMWORD(mEA); 	EA+=PC; 							m6809_ICount-=5;   break;
	case 0xae: EA=0;																   break; /*   ILLEGAL*/
	case 0xaf: IMMWORD(mEA); 										m6809_ICount-=5;   break;

	case 0xb0: EA=Y;	Y++;						EAD=RM16(EAD);	m6809_ICount-=5;   break;
	case 0xb1: EA=Y;	Y+=2;						EAD=RM16(EAD);	m6809_ICount-=6;   break;
	case 0xb2: Y--; 	EA=Y;						EAD=RM16(EAD);	m6809_ICount-=5;   break;
	case 0xb3: Y-=2;	EA=Y;						EAD=RM16(EAD);	m6809_ICount-=6;   break;
	case 0xb4: EA=Y;								EAD=RM16(EAD);	m6809_ICount-=3;   break;
	case 0xb5: EA=Y+SIGNED(B);						EAD=RM16(EAD);	m6809_ICount-=4;   break;
	case 0xb6: EA=Y+SIGNED(A);						EAD=RM16(EAD);	m6809_ICount-=4;   break;
	case 0xb7: EA=0;																   break; /*   ILLEGAL*/
	case 0xb8: IMMBYTE(EA); 	EA=Y+SIGNED(EA);	EAD=RM16(EAD);	m6809_ICount-=4;   break;
	case 0xb9: IMMWORD(mEA); 	EA+=Y;				EAD=RM16(EAD);	m6809_ICount-=7;   break;
	case 0xba: EA=0;																   break; /*   ILLEGAL*/
	case 0xbb: EA=Y+D;								EAD=RM16(EAD);	m6809_ICount-=7;   break;
	case 0xbc: IMMBYTE(EA); 	EA=PC+SIGNED(EA);	EAD=RM16(EAD);	m6809_ICount-=4;   break;
	case 0xbd: IMMWORD(mEA); 	EA+=PC; 			EAD=RM16(EAD);	m6809_ICount-=8;   break;
	case 0xbe: EA=0;																   break; /*   ILLEGAL*/
	case 0xbf: IMMWORD(mEA); 						EAD=RM16(EAD);	m6809_ICount-=5;   break;

	case 0xc0: EA=U;			U++;								m6809_ICount-=2;   break;
	case 0xc1: EA=U;			U+=2;								m6809_ICount-=3;   break;
	case 0xc2: U--; 			EA=U;								m6809_ICount-=2;   break;
	case 0xc3: U-=2;			EA=U;								m6809_ICount-=3;   break;
	case 0xc4: EA=U;																   break;
	case 0xc5: EA=U+SIGNED(B);										m6809_ICount-=1;   break;
	case 0xc6: EA=U+SIGNED(A);										m6809_ICount-=1;   break;
	case 0xc7: EA=0;																   break; /*ILLEGAL*/
	case 0xc8: IMMBYTE(EA); 	EA=U+SIGNED(EA);					m6809_ICount-=1;   break;
	case 0xc9: IMMWORD(mEA); 	EA+=U;								m6809_ICount-=4;   break;
	case 0xca: EA=0;																   break; /*ILLEGAL*/
	case 0xcb: EA=U+D;												m6809_ICount-=4;   break;
	case 0xcc: IMMBYTE(EA); 	EA=PC+SIGNED(EA);					m6809_ICount-=1;   break;
	case 0xcd: IMMWORD(mEA); 	EA+=PC; 							m6809_ICount-=5;   break;
	case 0xce: EA=0;																   break; /*ILLEGAL*/
	case 0xcf: IMMWORD(mEA); 										m6809_ICount-=5;   break;

	case 0xd0: EA=U;	U++;						EAD=RM16(EAD);	m6809_ICount-=5;   break;
	case 0xd1: EA=U;	U+=2;						EAD=RM16(EAD);	m6809_ICount-=6;   break;
	case 0xd2: U--; 	EA=U;						EAD=RM16(EAD);	m6809_ICount-=5;   break;
	case 0xd3: U-=2;	EA=U;						EAD=RM16(EAD);	m6809_ICount-=6;   break;
	case 0xd4: EA=U;								EAD=RM16(EAD);	m6809_ICount-=3;   break;
	case 0xd5: EA=U+SIGNED(B);						EAD=RM16(EAD);	m6809_ICount-=4;   break;
	case 0xd6: EA=U+SIGNED(A);						EAD=RM16(EAD);	m6809_ICount-=4;   break;
	case 0xd7: EA=0;																   break; /*ILLEGAL*/
	case 0xd8: IMMBYTE(EA); 	EA=U+SIGNED(EA);	EAD=RM16(EAD);	m6809_ICount-=4;   break;
	case 0xd9: IMMWORD(mEA); 	EA+=U;				EAD=RM16(EAD);	m6809_ICount-=7;   break;
	case 0xda: EA=0;																   break; /*ILLEGAL*/
	case 0xdb: EA=U+D;								EAD=RM16(EAD);	m6809_ICount-=7;   break;
	case 0xdc: IMMBYTE(EA); 	EA=PC+SIGNED(EA);	EAD=RM16(EAD);	m6809_ICount-=4;   break;
	case 0xdd: IMMWORD(mEA); 	EA+=PC; 			EAD=RM16(EAD);	m6809_ICount-=8;   break;
	case 0xde: EA=0;																   break; /*ILLEGAL*/
	case 0xdf: IMMWORD(mEA); 						EAD=RM16(EAD);	m6809_ICount-=5;   break;

	case 0xe0: EA=S;	S++;										m6809_ICount-=2;   break;
	case 0xe1: EA=S;	S+=2;										m6809_ICount-=3;   break;
	case 0xe2: S--; 	EA=S;										m6809_ICount-=2;   break;
	case 0xe3: S-=2;	EA=S;										m6809_ICount-=3;   break;
	case 0xe4: EA=S;																   break;
	case 0xe5: EA=S+SIGNED(B);										m6809_ICount-=1;   break;
	case 0xe6: EA=S+SIGNED(A);										m6809_ICount-=1;   break;
	case 0xe7: EA=0;																   break; /*ILLEGAL*/
	case 0xe8: IMMBYTE(EA); 	EA=S+SIGNED(EA);					m6809_ICount-=1;   break;
	case 0xe9: IMMWORD(mEA); 	EA+=S;								m6809_ICount-=4;   break;
	case 0xea: EA=0;																   break; /*ILLEGAL*/
	case 0xeb: EA=S+D;												m6809_ICount-=4;   break;
	case 0xec: IMMBYTE(EA); 	EA=PC+SIGNED(EA);					m6809_ICount-=1;   break;
	case 0xed: IMMWORD(mEA); 	EA+=PC; 							m6809_ICount-=5;   break;
	case 0xee: EA=0;																   break;  /*ILLEGAL*/
	case 0xef: IMMWORD(mEA); 										m6809_ICount-=5;   break;

	case 0xf0: EA=S;	S++;						EAD=RM16(EAD);	m6809_ICount-=5;   break;
	case 0xf1: EA=S;	S+=2;						EAD=RM16(EAD);	m6809_ICount-=6;   break;
	case 0xf2: S--; 	EA=S;						EAD=RM16(EAD);	m6809_ICount-=5;   break;
	case 0xf3: S-=2;	EA=S;						EAD=RM16(EAD);	m6809_ICount-=6;   break;
	case 0xf4: EA=S;								EAD=RM16(EAD);	m6809_ICount-=3;   break;
	case 0xf5: EA=S+SIGNED(B);						EAD=RM16(EAD);	m6809_ICount-=4;   break;
	case 0xf6: EA=S+SIGNED(A);						EAD=RM16(EAD);	m6809_ICount-=4;   break;
	case 0xf7: EA=0;																   break; /*ILLEGAL*/
	case 0xf8: IMMBYTE(EA); 	EA=S+SIGNED(EA);	EAD=RM16(EAD);	m6809_ICount-=4;   break;
	case 0xf9: IMMWORD(mEA); 	EA+=S;				EAD=RM16(EAD);	m6809_ICount-=7;   break;
	case 0xfa: EA=0;																   break; /*ILLEGAL*/
	case 0xfb: EA=S+D;								EAD=RM16(EAD);	m6809_ICount-=7;   break;
	case 0xfc: IMMBYTE(EA); 	EA=PC+SIGNED(EA);	EAD=RM16(EAD);	m6809_ICount-=4;   break;
	case 0xfd: IMMWORD(mEA); 	EA+=PC; 			EAD=RM16(EAD);	m6809_ICount-=8;   break;
	case 0xfe: EA=0;																   break; /*ILLEGAL*/
	case 0xff: IMMWORD(mEA); 						EAD=RM16(EAD);	m6809_ICount-=5;   break;
  }
}
