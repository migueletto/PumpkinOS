#include "host.h"
#include "d_event.h"

void M_Quit(void) {
  event_t event;
  event.type = ev_quit;
  D_PostEvent(&event);
}
