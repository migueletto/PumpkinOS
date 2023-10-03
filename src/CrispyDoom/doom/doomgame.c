#include <stdio.h>

#include "game.h"

int gameVariants(void) {
  return 5;
}

char *gameName(void) {
  return "Doom";
}

char *gameVariant(int i) {
  switch (i) {
    case 0: return "Doom (shareware)";
    case 1: return "Doom (registered)";
    case 2: return "Doom II";
    case 3: return "The Plutonia Experiment";
    case 4: return "TNT: Evilution";
  }
  return NULL;
}

char *gameWad(int i) {
  switch (i) {
    case 0: return "DOOM1.WAD";
    case 1: return "DOOM.WAD";
    case 2: return "DOOM2.WAD";
    case 3: return "PLUTONIA.WAD";
    case 4: return "TNT.WAD";
  }
  return NULL;
}
