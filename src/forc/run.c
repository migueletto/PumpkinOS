#include "sys.h"
#include "plibc.h"
#include "section.h"
#include "heap.h"
#include "debug.h"

#define MEM_SIZE   1024*1024
#define STACK_SIZE 1024

#define STACK_MIN(n) \
  if (((MEM_SIZE - sp) / 4) < (n)) return run_error("stack underflow")

#define OPCODE(op) \
  op = memory[ip]; \
  ip++

#define RELATIVE \
        ff1.u = opcode & 0xffffff; \
        if (ff1.u & 0x800000) ff1.u |= 0xff000000; \
        ff1.i -= 4; \
        ip = ((uint32_t)(((int32_t)(ip << 2)) + ff1.i)) >> 2

#define FETCH(addr) memory[(addr) >> 2]
#define FETCH16(addr) memory16[(addr) >> 1]
#define FETCH8(addr) memory8[addr]

#define STORE(addr, value) memory[(addr) >> 2] = value
#define STORE16(addr, value) memory16[(addr) >> 1] = value
#define STORE8(addr, value) memory8[addr] = value

#define TOP  memory[sp]
#define PREV memory[sp+1]
#define DROP(n) sp += n
#define POP(n)  n = memory[sp]; sp++
#define PUSH(n) sp--; memory[sp] = n

#define binop(oper) \
        STACK_MIN(2); \
        PREV = PREV oper TOP; \
        DROP(1)

#define fbinop(oper, code, name) \
        STACK_MIN(2); \
        switch (opcode >> 16) { \
          case code: \
            PREV = PREV oper TOP; \
            s = name; \
            break; \
          case code+1: \
            ff1.u = PREV; \
            ff2.u = TOP; \
            ff1.f = ff1.f oper ff2.f; \
            PREV = ff1.u; \
            s = name "f"; \
            break; \
        } \
        DROP(1)

static int run_error(char *fmt, ...) {
  sys_va_list ap;

  debug(DEBUG_ERROR, APPNAME, "Runtime error");
  sys_va_start(ap, fmt);
  debugva(DEBUG_ERROR, APPNAME, fmt, ap);
  sys_va_end(ap);

  return -1;
}

static int run1(uint32_t *memory, uint32_t start, uint32_t end, int32_t arg) {
  uint32_t ip, sp0,sp, fp, opcode, aux, size, i;
  uint16_t *memory16 = (uint16_t *)memory;
  uint8_t *memory8 = (uint8_t *)memory;
  uint8_t *heap_start, *ptr;
  heap_t *heap;
  union {
    uint32_t u;
    int32_t i;
    float f;
  } ff1, ff2;
  char buf[256], *s;
  int verbose = 0, r;

  //fprintf(stderr, "argument: %u\n", arg);

  ip = start >> 2;
  sp = MEM_SIZE >> 2;
  sp0 = sp;
  PUSH(0); // return value
  PUSH(arg); // 
  PUSH(4); // parameter size
  PUSH(0); // old fp
  PUSH(0xffffffff); // ret addr
  fp = sp;

  heap_start = memory8 + end;
  heap = heap_init(heap_start, MEM_SIZE - end - STACK_SIZE);
  if (verbose) {
    //fprintf(stderr, "heap %p\n", heap_start);
  }

  for (r = 0; r == 0;) {
    if (ip == (0xffffffff >> 2)) {
      if (verbose) {
        for (i = sp0-1; i >= sp; i--) {
          //fprintf(stderr, "stack[%u] 0x%08x: [0x%08x]\n", i-sp, i << 2, memory[i]);
        }
        //fprintf(stderr, "end of execution\n");
      }
      break;
    }
    aux = ip;
    OPCODE(opcode);
    if (verbose) {
      for (i = sp0-1; i >= sp; i--) {
        //fprintf(stderr, "stack[%u] 0x%08x: [0x%08x]\n", i-sp, i << 2, memory[i]);
      }
      sys_snprintf(buf, sizeof(buf)-1, "ip=0x%08x sp=%08x fp=%08x op=0x%08x", aux << 2, sp << 2, fp << 2, opcode);
    }

    switch (opcode >> 24) {
      case 0x10: // bool
        STACK_MIN(1);
        TOP = TOP ? 0xffffffff : 0;
        s = "bool";
        break;
      case 0x11: // not
        STACK_MIN(1);
        TOP = ~TOP;
        s = "not";
        break;
      case 0x12: // mod
        binop(%);
        s = "mod";
        break;
      case 0x13: // and
        binop(&);
        s = "and";
        break;
      case 0x14: // or
        binop(|);
        s = "or";
        break;
      case 0x15: // xor
        binop(^);
        s = "xor";
        break;
      case 0x16: // eq
        binop(==);
        s = "eq";
        break;
      case 0x17: // neq
        binop(!=);
        s = "neq";
        break;
      case 0x18: // lsh
        binop(<<);
        s = "lsh";
        break;
      case 0x19: // rsh
        binop(>>);
        s = "rsh";
        break;
      case 0x1a: // sign
        // XXX nothing to do?
        s = "sign";
        break;
      case 0x1b: // swap
        STACK_MIN(2);
        aux = PREV;
        PREV = TOP;
        TOP = aux;
        s = "swap";
        break;
      case 0x1c: // drop
        STACK_MIN(1);
        DROP(1);
        break;
      case 0x1d: // dup
        STACK_MIN(1);
        aux = TOP;
        PUSH(aux);
        break;
      case 0x1e: // over
        STACK_MIN(2);
        aux = PREV;
        PUSH(aux);
        s = "over";
        break;
      case 0x20: // push32
        OPCODE(aux);
        PUSH(aux);
        s = "push32";
        break;
      case 0x21: // fetch32
        STACK_MIN(1);
        TOP = FETCH(TOP);
        s = "fetch32";
        break;
      case 0x22: // link
        aux = fp;
        PUSH(aux << 2);
        fp = sp;
        s = "link";
        break;
      case 0x23: // unlink
        STACK_MIN(2);
        POP(aux);  // old fp
        POP(size); // param size
        sp += size >> 2;
        fp = aux >> 2;
        s = "unlink";
        break;
      case 0x24: // ret
        STACK_MIN(1);
        POP(aux);
        ip = aux >> 2;
        s = "ret";
        break;
      case 0x25: // new
        STACK_MIN(1);
        aux = TOP;
        ptr = heap_alloc(heap, aux);
        TOP = ptr ? (uint32_t)(ptr - memory8) : 0;
        s = "new";
        break;
      case 0x26: // hinc
        STACK_MIN(1);
        POP(aux);
        heap_inc(heap, memory8 + aux);
        s = "hinc";
        break;
      case 0x27: // hdec
        STACK_MIN(1);
        POP(aux);
        if (aux) heap_dec(heap, memory8 + aux);
        s = "hdec";
        break;
      case 0x28: // floati
        STACK_MIN(1);
        ff1.i = TOP;
        ff2.f = (float)ff1.i;
        TOP = ff2.i;
        s = "floati";
        break;
      case 0x29: // floatu
        STACK_MIN(1);
        ff1.u = TOP;
        ff2.f = (float)ff1.u;
        TOP = ff2.u;
        s = "floatu";
        break;
      case 0x30: // add
        fbinop(+, 0x3000, "add");
        break;
      case 0x31: // sub
        fbinop(-, 0x3100, "sub");
        break;
      case 0x32: // mul
        fbinop(*, 0x3200, "mul");
        break;
      case 0x33: // div
        fbinop(/, 0x3300, "div");
        break;
      case 0x34: // gt
        fbinop(>, 0x3400, "gt");
        break;
      case 0x35: // lt
        fbinop(<, 0x3500, "lt");
        break;
      case 0x36: // gte
        fbinop(>=, 0x3600, "gte");
        break;
      case 0x37: // lte
        fbinop(<=, 0x3700, "lte");
        break;
      case 0x40: // pushi8, pushu8, pushi16, pushu16
        switch (opcode >> 16) {
          case 0x4000: // pushu8
            PUSH(opcode & 0xff);
            s = "pushu8";
            break;
          case 0x4080: // pushi8
            aux = opcode & 0xff;
            if (aux & 0x80) aux |= 0xffffff00;
            PUSH(aux);
            s = "pushi8";
            break;
          case 0x4001: // pushu16
            PUSH(opcode & 0xffff);
            s = "pushu16";
            break;
          case 0x4081: // pushi16
            aux = opcode & 0xffff;
            if (aux & 0x8000) aux |= 0xffff0000;
            PUSH(aux);
            s = "pushi16";
            break;
        }
        break;
      case 0x50: // fetchi8, fetchu8, fetchi16, fetchu16
        STACK_MIN(1);
        switch (opcode >> 16) {
          case 0x5000: // fetchu8
            TOP = FETCH8(TOP);
            s = "fetchu8";
            break;
          case 0x5080: // fetchi8
            aux = FETCH8(TOP);
            if (aux & 0x80) aux |= 0xffffff00;
            TOP = aux;
            s = "fetchi8";
            break;
          case 0x5001: // fetchu16
            TOP = FETCH16(TOP);
            s = "fetchu16";
            break;
          case 0x5081: // fetchi16
            aux = FETCH16(TOP);
            if (aux & 0x8000) aux |= 0xffff0000;
            TOP = aux;
            s = "fetchi16";
            break;
          default:
            return run_error("invalid opcode 0x%08x", opcode);
        }
        break;
      case 0x60: // store8, store16, store32
        STACK_MIN(2);
        switch (opcode >> 16) {
          case 0x6000: // store8
            STORE8(TOP, PREV);
            s = "store8";
            break;
          case 0x6001: // store16
            STORE16(TOP, PREV);
            s = "store16";
            break;
          case 0x6002: // store32
            STORE(TOP, PREV);
            s = "store32";
            break;
          default:
            return run_error("invalid opcode 0x%08x", opcode);
        }
        DROP(2);
        break;
      case 0x80: // faddr
        aux = opcode & 0xffff;
        aux = (fp << 2) + aux;
        PUSH(aux);
        s = "faddr";
        break;
      case 0x90: // jpa
        OPCODE(aux);
        ip = aux;
        s = "jpa";
        break;
      case 0xa0: // jp
        RELATIVE;
        s = "jp";
        break;
      case 0xa1: // jz
        STACK_MIN(1);
        POP(aux);
        if (!aux) {
          RELATIVE;
        }
        s = "jz";
        break;
      case 0xa2: // call
        PUSH(ip << 2);
        fp = sp;
        RELATIVE;
        s = "call";
        break;
      case 0xb0: // frame
        aux = opcode & 0xffff;
        aux = (aux + 3) >> 2;
        STACK_MIN(aux);
        for (i = 0; i < aux; i++) {
          PUSH(0);
        }
        fp = sp;
        s = "frame";
        break;
      case 0xb1: // unframe
        aux = opcode & 0xffff;
        aux = (aux + 3) >> 2;
        sp += aux;
        s = "unframe";
        break;
      default:
        return run_error("invalid opcode 0x%08x", opcode);
    }

    if (verbose) debug(DEBUG_INFO, APPNAME, "%s %s", buf, s);
  }

  heap_finish(heap);

  if (r == 0) {
    POP(aux);
    POP(aux);
    POP(aux);
    if (verbose) {
      for (i = sp0-1; i >= sp; i--) {
        //fprintf(stderr, "stack[%u] 0x%08x: [0x%08x]\n", i-sp, i << 2, memory[i]);
      }
    }

    if (sp == (MEM_SIZE >> 2)) {
      //fprintf(stderr, "no return value\n");
    } else if (sp == (MEM_SIZE >> 2) - 1) {
      //fprintf(stderr, "return value: %u\n", TOP);
    } else {
      //fprintf(stderr, "unbalanced stack: %u\n", sp << 2);
    }
  } else {
    //fprintf(stderr, "execution aborted\n");
  }

  return r;
}

int run(char *exe, int arg) {
  uint32_t aux, size, start;
  uint32_t *memory;
  int fd, r = 0;

  if ((fd = plibc_open(1, exe, PLIBC_RDONLY)) == -1) {
    run_error("error opening executable '%s'", exe);
    return -1;
  }

  if (plibc_read(fd, &aux, 4) != 4) {
    run_error("I/O error 1");
    plibc_close(fd);
    return -1;
  }

  if (aux != EXE_MAGIC) {
    run_error("invalid executable '%s'", exe);
    plibc_close(fd);
    return -1;
  }

  if (plibc_read(fd, &start, 4) != 4) {
    run_error("I/O error 2");
    plibc_close(fd);
    return -1;
  }

  if ((size = plibc_lseek(fd, 0, PLIBC_SEEK_END)) == -1 || plibc_lseek(fd, 8, PLIBC_SEEK_SET) == -1) {
    run_error("I/O error 3");
    plibc_close(fd);
    return -1;
  }
  size -= 8;

  if (size < 4) {
    run_error("invalid executable '%s'", exe);
    plibc_close(fd);
    return -1;
  }

  if ((start % 4) != 0 || start >= size) {
    run_error("invalid executable '%s'", exe);
    plibc_close(fd);
  }

  if ((memory = sys_calloc(1, MEM_SIZE)) == NULL) {
    run_error("internal error");
    plibc_close(fd);
    return -1;
  }

  if (plibc_read(fd, memory, size) != size) {
    run_error("I/O error 4");
    sys_free(memory);
    plibc_close(fd);
    return -1;
  }

  r = run1(memory, start, size, arg);
  sys_free(memory);

  return r;
}
