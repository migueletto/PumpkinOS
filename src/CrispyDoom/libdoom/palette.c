#include "i_video.h"
#include "w_wad.h"
#include "z_zone.h"

void setPalette(void) {
  I_SetPalette(W_CacheLumpName("PLAYPAL", PU_CACHE));
}
