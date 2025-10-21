#ifdef __cplusplus
extern "C" {
#endif

#define logtrap_A0 1
#define logtrap_D0 2
#define logtrap_D2 3
#define logtrap_SP 4

typedef struct logtrap_t logtrap_t;

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

  void (*hook)(logtrap_t *lt, uint32_t pc, uint16_t trap);
  void (*rethook)(logtrap_t *lt, uint32_t pc);
} logtrap_def;

logtrap_t *logtrap_init(logtrap_def *def);
void logtrap_start(logtrap_t *lt, char *appname);
void logtrap_finish(logtrap_t *lt);
char *logtrap_trapname(logtrap_t *lt, uint16_t trap, uint16_t *selector, int follow);

#ifdef __cplusplus
};
#endif
