#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>

void key_down(int keycode);
void key_up(int keycode);
bool is_key_pressed(int keycode);

#endif