#include <PalmOS.h>

#include "pumpkin.h"
#include "host.h"

#include "strife/strife_icon.c"
#include "doom/doom_icon.c"
#include "heretic/heretic_icon.c"
#include "hexen/hexen_icon.c"

static void saveIcon(const unsigned int *data, int width, int height, char *name) {
  surface_t *s;
  uint32_t *buf, aux, r, g, b, a;
  int i, n, len;

  s = surface_create(width, height, SURFACE_ENCODING_ARGB);
  buf = s->getbuffer(s->data, &len);
  n = width * height;
  for (i = 0; i < n; i++) {
    aux = data[i];
    a = aux & 0xff;
    if (a > 0x80) a = 0xff; else a = 0;
    b = (aux >> 8) & 0xff;
    g = (aux >> 16) & 0xff;
    r = (aux >> 24) & 0xff;
    buf[i] = (a << 24) | (r << 16) | (g << 8) | b;
  }
  surface_save(s, name, 0);
  surface_destroy(s);
}

void saveIcons(void) {
  saveIcon(doom_icon_data, doom_icon_w, doom_icon_h, "doom.png");
  saveIcon(heretic_icon_data, heretic_icon_w, heretic_icon_h, "heretic.png");
  saveIcon(hexen_icon_data, hexen_icon_w, hexen_icon_h, "hexen.png");
  saveIcon(strife_icon_data, strife_icon_w, strife_icon_h, "strife.png");
}
