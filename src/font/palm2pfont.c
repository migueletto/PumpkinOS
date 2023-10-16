#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/*
fontType 36864
ascent 9
descent 2

GLYPH 20
-####------
######-----
#----#--#--
######-#---
-####-#----
--##-------
--##--##-#-
--##-------
-###--#----
-###---#---
--##----#--
*/

/*
  // space
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
*/

#define MAX_HEIGHT 64

int main(int argc, char *argv[]) {
  char *name, *font, buf[256];
  char *line[256][MAX_HEIGHT];
  int defined[256], width[256], index[256];
  char bit, byte[9];
  int s, i, j, k, w, h, base, len, twidth, height, glyph, min, max;
  FILE *f;

  if (argc < 3) return 1;
  name = argv[1];
  font = argv[2];

  memset(line, 0, sizeof(line));
  memset(defined, 0, sizeof(defined));
  min = 256;
  max = -1;
  twidth = 0;

  f = fopen(font, "r");

  for (s = 0;;) {
    if (fgets(buf, sizeof(buf)-1, f) == NULL) buf[0] = 0;
    len = strlen(buf);
    if (len > 0 && buf[len-1] == '\n')  buf[len-1] = 0;
//printf("s%d buf [%s]\n", s, buf);

    if (s == 0) {
      if (buf[0] == 0) continue;
      if (strncmp(buf, "ascent", 6) == 0) {
        height = atoi(&buf[7]);
        continue;
      }
      if (strncmp(buf, "descent", 7) == 0) {
        height += atoi(&buf[8]);
        continue;
      }
      if (strncmp(buf, "GLYPH", 5) == 0) {
        glyph = atoi(&buf[6]);
//printf("glyph %d\n", glyph);
        defined[glyph] = 1;
        if (glyph < min) min = glyph;
        if (glyph > max) max = glyph;
        i = 0;
        s = 1;
      }
    } else {
      if (buf[0] == 0) {
        h = 8 - (height % 8);
        if (h != 8) {
          for (j = 0; j < width[glyph]; j++) {
            buf[j] = '-';
          }
          buf[j] = 0;
          for (j = 0; j < h; j++) {
//printf("line %2d [%s]\n", i, buf);
            line[glyph][i++] = strdup(buf);
          }
        }
        s = 0;
      } else if (i < height) {
        if (i == 0) {
          width[glyph] = len - 1;
          twidth += width[glyph];
        }
//printf("line %2d [%s]\n", i, buf);
        line[glyph][i++] = strdup(buf);
      }
    }
    if (feof(f)) break;
  }

  fclose(f);

  for (j = min; j <= max; j++) {
    if (!defined[j]) twidth++;
  }

  printf("#define FONT_%s_WIDTH  %d\n", name, 0);
  printf("#define FONT_%s_HEIGHT %d\n", name, height);
  printf("#define FONT_%s_MIN    %d\n", name, min);
  printf("#define FONT_%s_MAX    %d\n", name, max);
  printf("#define FONT_%s_LEN    %d\n", name, twidth);
  printf("\n");

  h = 8 - (height % 8);
  if (h != 8) height += h;

  len = (height / 8) * twidth;
  printf("static uint8_t font_%s[%d] = {\n", name, len);

  base = height - 8;
  for (k = 0;;) {
    for (glyph = min; glyph <= max; glyph++) {
      if (base == height - 8) index[glyph] = k;
      if (glyph < 32 || glyph >= 127 || glyph == '\\') {
        printf("  // %d\n", glyph);
      } else {
        printf("  // %c\n", glyph);
      }
      w = defined[glyph] ? width[glyph] : 1;
      for (j = 0; j < w; j++) {
        memset(byte, '0', 8);
        for (i = 0; i < 8; i++) {
          if (defined[glyph]) {
            bit = line[glyph][base + 7 - i][j] == '#' ? '1' : '0';
          } else {
            bit = '0';
          }
          byte[i] = bit;
        }
        byte[8] = 0;
        printf("  0b%s, // %d\n", byte, k);
        k++;
      }
      printf("\n");
    }
    if (base == 0) break;
    base -= 8;
  }

  printf("};\n\n");

  printf("static uint8_t width_%s[%d] = {\n", name, max - min + 1);
  for (glyph = min; glyph <= max; glyph++) {
    w = defined[glyph] ? width[glyph] : 1;
    if (glyph < 32 || glyph >= 127 || glyph == '\\') {
      printf("  %2d, // %d\n", w, glyph);
    } else {
      printf("  %2d, // %c\n", w, glyph);
    }
  }
  printf("};\n\n");

  printf("static int index_%s[%d] = {\n", name, max - min + 1);
  for (glyph = min; glyph <= max; glyph++) {
    if (glyph < 32 || glyph >= 127 || glyph == '\\') {
      printf("  %2d, // %d\n", index[glyph], glyph);
    } else {
      printf("  %2d, // %c\n", index[glyph], glyph);
    }
  }
  printf("};\n\n");

  printf("static font_t font%s = {\n", name);
  printf("  FONT_%s_WIDTH, FONT_%s_HEIGHT,\n", name, name);
  printf("  FONT_%s_MIN,   FONT_%s_MAX, FONT_%s_LEN,\n", name, name, name);
  printf("  font_%s,\n", name);
  printf("  width_%s,\n", name);
  printf("  index_%s\n", name);
  printf("};\n");

  return 0;
}
