#ifndef LOGTRAP_H
#define LOGTRAP_H

#ifdef __cplusplus
extern "C" {
#endif

#define logtrap_SP  1
#define logtrap_D0  8
#define logtrap_A0 16

typedef struct logtrap_t logtrap_t;

typedef struct {
  uint16_t type;
  uint16_t ptr;
  uint16_t size;
  uint16_t io;
  char *name;
} trap_arg_t;

typedef struct trap_t {
  uint16_t trap;
  int16_t selector;
  char *name;
  uint16_t rtype;
  uint16_t rptr;
  uint16_t rsize;
  uint16_t nargs;
  trap_arg_t args[16];
  uint16_t capsel;
  uint16_t numsel;
  uint16_t maxsel;
  struct trap_t *selectors;
} trap_t;

typedef struct {
  void *(*alloc)(uint32_t size, void *data);
  void *(*realloc)(void *p, uint32_t size, void *data);
  void (*free)(void *p, void *data);
  void (*print)(char *s, void *data);
  uint8_t (*read8)(uint32_t addr, void *data);
  uint16_t (*read16)(uint32_t addr, void *data);
  uint32_t (*read32)(uint32_t addr, void *data);
  uint32_t (*getreg)(uint32_t reg, void *data);
  void *data;

  void (*hook)(logtrap_t *lt, uint32_t pc);
  void (*hook2)(logtrap_t *lt, uint32_t pc);
  void (*rethook)(logtrap_t *lt, uint32_t pc);

  trap_t allTraps[0x1000];
} logtrap_def;

logtrap_def *logtrap_get_def(void);
int logtrap_global_init(logtrap_def *def);
logtrap_t *logtrap_init(logtrap_def *def);
void logtrap_start(logtrap_t *lt, int disasm, char *appname);
int logtrap_started(logtrap_t *lt);
void logtrap_finish(logtrap_t *lt);
char *logtrap_trapname(logtrap_t *lt, uint16_t trap, uint16_t *selector, int follow);

#ifdef __cplusplus
};
#endif

#endif
