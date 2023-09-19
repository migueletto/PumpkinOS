#define ATTR_INVERSE     0x80
#define ATTR_BRIGHT      0x40
#define ATTR_UNDERSCORE  0x20
#define ATTR_BLINK       0x10

typedef enum { BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE } color_e;

typedef struct pterm_t pterm_t;

typedef struct {
  int (*draw)(uint8_t col, uint8_t row, uint8_t code, uint32_t fg, uint32_t bg, uint8_t attr, void *data);
  int (*erase)(uint8_t col1, uint8_t row1, uint8_t col2, uint8_t row2, uint32_t bg, uint8_t attr, void *data);
  void (*reply)(char *buf, int n, void *data);
  void *data;
} pterm_callback_t;

pterm_t *pterm_init(int cols, int rows, int rgb);
void pterm_callback(pterm_t *pterm, pterm_callback_t *cb);
void pterm_close(pterm_t *pterm);
void pterm_send(pterm_t *t, uint8_t *buf, int n);
void pterm_home(pterm_t *pterm);
void pterm_cls(pterm_t *t);
void pterm_clreol(pterm_t *t);
void pterm_setx(pterm_t *t, int col);
void pterm_sety(pterm_t *t, int row);
int pterm_getx(pterm_t *t);
int pterm_gety(pterm_t *t);
void pterm_setfg(pterm_t *t, uint32_t c);
void pterm_setbg(pterm_t *t, uint32_t c);
uint32_t pterm_getfg(pterm_t *t);
uint32_t pterm_getbg(pterm_t *t);
void pterm_cursor(pterm_t *t, int show);
void pterm_cursor_blink(pterm_t *t);
void pterm_cursor_enable(pterm_t *t, int enabled);

void pterm_getsize(pterm_t *pterm, uint8_t *cols, uint8_t *rows);
int pterm_getcursor(pterm_t *pterm, uint8_t *col, uint8_t *row);
void pterm_getchar(pterm_t *pterm, uint32_t index, uint8_t *code, uint32_t *fg, uint32_t *bg);
void pterm_putchar(pterm_t *pterm, uint32_t index, uint8_t code, uint32_t fg, uint32_t bg);
char pterm_getstate(pterm_t *t);
