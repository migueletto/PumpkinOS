typedef struct trap_t {
  uint16_t trap, selector;
  char *name;
  char *rtype;
  int nargs;
  char *atype[16];
  char *aname[16];
  char *ptr;
  const struct trap_t *dispatch;
} trap_t;

typedef struct {
  uint32_t stackp;
  uint32_t stack[256];
  uint32_t stackt[256];
} trace_t;
