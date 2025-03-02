#include "sys.h"
#include "thread.h"
#include "pwindow.h"

// fake "average" function just for testing the UI
int average_click(window_provider_t *wp, window_t *window, int *x, int *y, int ms) {
  int arg1, arg2;

  for (;;) {
    if (thread_must_end()) return -1;

    switch (wp->event2(window, 1, &arg1, &arg2)) {
      case WINDOW_BUTTONUP:
        return 1;
      case WINDOW_MOTION:
        *x = arg1;
        *y = arg2;
        break;
      case 0:
        if (ms == -1) continue;
        if (ms == 0) return 0;
        ms--;
        break;
      case -1:
        return -1;
    }
  }

  return -1;
}
