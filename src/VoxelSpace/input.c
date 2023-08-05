#include <stdio.h>

#include "input.h"

#define MAX_KEYCODE 255

static bool keys[MAX_KEYCODE];

void key_down(int keycode) {
  if (keycode > MAX_KEYCODE) {
    return;
  }
  keys[keycode] = true;
}

void key_up(int keycode) {
  if (keycode > MAX_KEYCODE) {
    return;
  }
  keys[keycode] = false;
}

bool is_key_pressed(int keycode) {
  if (keycode > MAX_KEYCODE) {
    return false;
  }
  return keys[keycode];
}
