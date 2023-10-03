#define LEVEL_LOW    0
#define LEVEL_ZERO 127
#define LEVEL_HIGH 255

typedef struct {
  uint8_t *buf;
  int pos, len;
  uint32_t lastCount;
} tape_t;

int casInit(tape_t *tape);
int casFinish(tape_t *tape);
uint8_t casWaveValue(tape_t *tape);
int casFinished(tape_t *tape);
int casPos(tape_t *tape);
int casSize(tape_t *tape);
