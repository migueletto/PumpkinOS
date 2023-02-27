#include "sys.h"
#include "script.h"
#include "pwindow.h"
#include "ptr.h"
#include "media.h"
#include "pfont.h"
#include "graphic.h"
#include "vfont.h"
#include "debug.h"
#include "xalloc.h"

int vfont_init(int pe) {
  graphic_vfont_t *vf;
  int r = -1;

  if ((vf = graphic_vfont_init()) != NULL) {
    r = script_set_pointer(pe, VFONT, vf);
  }

  return r;
}

int vfont_finish(int pe) {
  graphic_vfont_t *vf;
  int r = -1;

  if ((vf = script_get_pointer(pe, VFONT)) != NULL) {
    graphic_vfont_finish(vf);
    script_set_pointer(pe, VFONT, NULL);
    r = 0;
  }

  return r;
}
