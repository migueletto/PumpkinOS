#define DIA_MODE_GRAF   1
#define DIA_MODE_LOWER  2
#define DIA_MODE_UPPER  3
#define DIA_MODE_NUMBER 4

typedef struct dia_t dia_t;

dia_t *dia_init(window_provider_t *wp, window_t *w, int encoding, int depth, int dbl);
void dia_set_wh(dia_t *dia, int mode, WinHandle wh, RectangleType *bounds);
int dia_update(dia_t *dia);
int dia_stroke(dia_t *dia, int x, int y);
void dia_draw_stroke(dia_t *dia, int x1, int y1, int x2, int y2);
int dia_clicked(dia_t *dia, int current_task, int x, int y, int down);
int dia_set_trigger(dia_t *dia, int trigger);
int dia_get_trigger(dia_t *dia);
int dia_set_state(dia_t *dia, int state);
int dia_get_state(dia_t *dia);
int dia_get_taskbar_dimension(dia_t *dia, int *width, int *height);
int dia_get_graffiti_dimension(dia_t *dia, int *width, int *height);
int dia_get_main_dimension(dia_t *dia, int *width, int *height);
void dia_set_graffiti_state(dia_t *dia, uint16_t state);
void dia_color(RGBColorType *fg, RGBColorType *bg);
int dia_finish(dia_t *dia);
