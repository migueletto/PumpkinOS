#include "game.h"

int gameVariants(void) {
  return 1;
}

char *gameName(void) {
  return "Hexen";
}

char *gameVariant(int i) {
  return "Hexen";
}

char *gameWad(int i) {
  return "HEXEN.WAD";
}

char *gameMsgOn(void) {
  return "messageson";
}
