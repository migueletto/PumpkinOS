#ifndef RENDERER_H
#define RENDERER_H

#include "microui.h"

void r_draw_rect(mu_Rect rect, mu_Color color);
void r_draw_text(char *text, mu_Vec2 pos, mu_Font font, mu_Color color, mu_Color back);
void r_draw_icon(int id, mu_Rect rect, mu_Color color);
void r_draw_image(void *img, mu_Rect rect);
void r_set_clip_rect(mu_Rect rect);
void r_clear(mu_Color color);
void r_present(void);

#endif

