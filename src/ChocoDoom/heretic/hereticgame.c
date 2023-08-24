#include <stdio.h>

#include "game.h"

int gameVariants(void) {
  return 2;
}

char *gameName(void) {
  return "Heretic";
}

char *gameVariant(int i) {
  switch (i) {
    case 0: return "Heretic (shareware)";
    case 1: return "Heretic (registered)";
  }
  return NULL;
}

char *gameWad(int i) {
  switch (i) {
    case 0: return "HERETIC1.WAD";
    case 1: return "HERETIC.WAD";
  }
  return NULL;
}

char *gameMsgOn(void) {
  return "messageson";
}
