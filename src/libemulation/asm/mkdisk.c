#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include "disk.h"

#define BLOCK_SECTORS  8
#define BLOCK_LEN      (BLOCK_SECTORS * SECTOR_LEN)
#define TRACK_LEN      (FLOPPY_SECTORS * SECTOR_LEN)
#define EXTENT_BLOCKS  16
#define EXTENT_LEN     (EXTENT_BLOCKS * BLOCK_LEN)
#define EXTENT_SECTORS (EXTENT_BLOCKS * BLOCK_SECTORS)
#define DIR_ENTRY      32

int write_data(FILE *f, char *name, char *buf, int n) {
  FILE *g;

  if ((g = fopen(name, "rb")) == NULL) {
    perror("fopen");
    return -1;
  }
  memset(buf, 0, n);
  fread(buf, n, 1, g);
  fclose(g);
  fwrite(buf, n, 1, f);

  return 0;
}

int write_file(FILE *f, char *name, char *dir, int entry, int *iblock, int cpm_tracks) {
  FILE *g;
  char buf[2048];
  long size;
  int i, j, last, n, ext, nsectors, nblocks, nextents, block;
  char *dot, *s;

  if ((g = fopen(name, "rb")) == NULL) {
    perror("fopen");
    return 0;
  }

  fseek(f, cpm_tracks * TRACK_LEN + (*iblock) * BLOCK_LEN, SEEK_SET);
  size = 0;

  for (;;) {
    n = fread(buf, 1, sizeof(buf), g);
    if (n <= 0) break;
    fwrite(buf, 1, n, f);
    size += n;
  }
  fclose(g);

  nsectors = (size + SECTOR_LEN - 1) / SECTOR_LEN;
  nblocks  = (size + BLOCK_LEN - 1) / BLOCK_LEN;
  nextents = (nblocks + EXTENT_BLOCKS - 1) / EXTENT_BLOCKS;

  last = -1;
  for (i = 0; name[i]; i++) {
    if (name[i] == '/') last = i;
  }
  name = &name[last+1];

  for (i = 0; name[i]; i++) {
    name[i] = toupper(name[i]);
  }

  dot = strchr(name, '.');
  if (dot == NULL) dot = name + strlen(name);
  i = entry * DIR_ENTRY;
  printf("[%12s] %3d sectors, %2d blocks, %d extents, %5ld bytes\n", name, nsectors, nblocks, nextents, size);

  // ST F0 F1 F2 F3 F4 F5 F6 F7 E0 E1 E2 XL BC XH RC
  // AL AL AL AL AL AL AL AL AL AL AL AL AL AL AL AL

  for (ext = 0; ext < nextents; ext++) {
    // ST
    dir[i++] = 0x00; // user 0

    // F0 - F7
    for (j = 0, s = name; s[0] && s < dot && j < 8; s++, j++) {
      dir[i++] = toupper(s[0]);
    }
    for (; j < 8; j++) {
      dir[i++] = 0x20;
    }

    // E0 - E2
    for (j = 0, s = dot+1; s[0] && j < 3; s++, j++) {
      dir[i++] = toupper(s[0]);
    }
    for (; j < 3; j++) {
      dir[i++] = 0x20;
    }

    // XL and XH store the extent number.
    // Bit 5-7 of XL are 0, bit 0-4 store the lower bits of the extent number.
    // Bit 6 and 7 of XH are 0, bit 0-5 store the higher bits of the extent number.
    // RC and BC determine the length of the data used by this extent.
    // RC stores the number of 128 byte records of the last used logical extent.
    // BC stores the number of bytes in the last used record. 

    dir[i++] = ext & 0x1F;
    dir[i] = size % SECTOR_LEN;
    //if (dir[i] == 0) dir[i] = SECTOR_LEN;
    i++;
    dir[i++] = (ext >> 5) & 0x3F;

    if (ext == nextents-1) {
      n = nsectors % EXTENT_SECTORS;
      dir[i++] = n;
      n = nblocks % EXTENT_BLOCKS;
      if (n == 0) n = EXTENT_BLOCKS;

      for (block = 0; block < n; block++) {
        dir[i++] = (*iblock)++;
      }
      for (; block < EXTENT_BLOCKS; block++) {
        dir[i++] = 0x00;
      }
    } else {
      dir[i++] = EXTENT_SECTORS;

      for (block = 0; block < EXTENT_BLOCKS; block++) {
        dir[i++] = (*iblock)++;
      }
    }
  }

  return nextents;
}

static void usage(void) {
  printf("usage: mkdisk <disk.dsk> <numtracks> <cpmtracks> <boot.bin> <cpm.bin> <bios.bin> [ <file.ext> ... ]\n");
}

int main(int argc, char *argv[]) {
  FILE *f;
  int i, argi, num_tracks, cpm_tracks, entry, iblock;
  char boot[1 * SECTOR_LEN];
  char cpm[44 * SECTOR_LEN];
  char bios[7 * SECTOR_LEN];
  char dir[2 * BLOCK_LEN];
  char *name;

  if (argc < 7) {
    usage();
    exit(1);
  }

  name = argv[1];
  if ((f = fopen(name, "wb")) == NULL) {
    perror("fopen");
    exit(1);
  }

  num_tracks = atoi(argv[2]);
  cpm_tracks = atoi(argv[3]);

  // 1 + 44 + 7 = 52 = 2 tracks of 26 setors
  write_data(f, argv[4], boot, sizeof(boot));	//  1 sector
  write_data(f, argv[5], cpm,  sizeof(cpm));	// 44 sectors
  write_data(f, argv[6], bios, sizeof(bios));	//  7 sectors
  argi = 7;

  memset(dir, 0xE5, sizeof(dir));
  fseek(f, cpm_tracks * TRACK_LEN, SEEK_SET);
  fwrite(dir, sizeof(dir), 1, f);

  iblock = 2;  // skip 2 blocks used by the directory
  entry = 0;

  for (i = argi; i < argc; i++) {
    entry += write_file(f, argv[i], dir, entry, &iblock, cpm_tracks);
  }

  fseek(f, cpm_tracks * TRACK_LEN, SEEK_SET);
  fwrite(dir, sizeof(dir), 1, f);

  fseek(f, num_tracks * TRACK_LEN - 1, SEEK_SET);
  dir[0] = 0;
  fwrite(dir, 1, 1, f);
  fclose(f);

  exit(0);
}
