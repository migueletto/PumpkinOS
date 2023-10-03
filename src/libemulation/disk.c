#include "sys.h"
#include "vfs.h"
#include "bytes.h"
#include "pwindow.h"
#include "filter.h"
#include "computer.h"
#include "disk.h"
#include "xalloc.h"
#include "debug.h"

#define DISK_MAGIC          0x4B534944
#define DISK_FLAG_SECTOR0   0x01
#define DISK_FLAG_READONLY  0x02

#define MAX_SECTOR_LEN  512

// cpm3

// drives A-D: igual ao meu floppy, mas:
// 16 checked directories entries
// usa sector skew de 6:

// skew6: db       1,  7, 13, 19, 25
//        db       5, 11, 17, 23,  3
//        db       9, 15, 21,  2,  8
//        db      14, 20, 26,  6, 12
//        db      18, 24,  4, 10, 16
//        db      22

typedef struct {
  vfs_file_t *f;
  int skip, sector0, use0;
  uint16_t readonly;
  uint16_t tracks;     // tracks per disk
  uint16_t heads;      // heads per disk
  uint16_t sectors;    // sectors per track
  uint16_t sectorlen;  // bytes per sector
} drive_t;

struct disk_t {
  computer_t *c;
  vfs_session_t *session;
  int ndrives;
  int sectorlen;
  int dma;
  uint8_t buffer[MAX_SECTOR_LEN];
  int ptr;
  uint8_t drv;
  uint16_t track;
  uint16_t sector;
  uint8_t status;
  uint16_t dma_addr;
  drive_t drive[MAX_DRIVES];
};

// FDC command status:
// 0 - ok
// 1 - illegal drive
// 2 - illegal track
// 3 - illegal sector
// 4 - seek error
// 5 - read error
// 6 - write error
// 7 - illegal command to FDC

static int validate(disk_t *d) {
  if (d->drv >= d->ndrives) {
    debug(DEBUG_ERROR, "DISK", "invalid drive %d", d->drv);
    return 1;
  }

  if (d->track >= d->drive[d->drv].tracks) {
    debug(DEBUG_ERROR, "DISK", "invalid track %d drive %d", d->track, d->drv);
    return 2;
  }

  if (d->sector == 0 || d->sector > d->drive[d->drv].sectors) {
    debug(DEBUG_ERROR, "DISK", "invalid sector %d drive %d", d->sector, d->drv);
    return 3;
  }

  return 0;
}

static int read_sector(disk_t *d) {
  uint32_t offset;
  int r;

  debug(DEBUG_TRACE, "DISK", "read_sector d=%u t=%u s=%u", d->drv, d->track, d->sector);

  if ((r = validate(d)) != 0) {
    return r;
  }

  offset = d->drive[d->drv].skip + (d->track * d->drive[d->drv].sectors + d->sector - 1) * d->drive[d->drv].sectorlen;
  debug(DEBUG_TRACE, "DISK", "offset %u", offset);

  if (d->drive[d->drv].f != NULL) {
    if (vfs_seek(d->drive[d->drv].f, offset, 0) == offset) {
      if (vfs_read(d->drive[d->drv].f, d->buffer, d->drive[d->drv].sectorlen) == d->drive[d->drv].sectorlen) {
        return 0;
      } else {
        debug_errno("DISK", "read");
        return 5;
      }
    } else {
      return 4;
    }
  }

  return 1;
}

static int write_sector(disk_t *d) {
  uint32_t offset;
  int r;

  debug(DEBUG_TRACE, "DISK", "write_sector d=%u t=%u s=%u", d->drv, d->track, d->sector);

  if ((r = validate(d)) != 0) {
    return r;
  }

  if (d->drive[d->drv].readonly) {
    debug(DEBUG_INFO, "DISK", "write_sector readonly");
    return 6;
  }

  offset = d->drive[d->drv].skip + (d->track * d->drive[d->drv].sectors + d->sector - 1) * d->drive[d->drv].sectorlen;

  if (d->drive[d->drv].f != NULL) {
    if (vfs_seek(d->drive[d->drv].f, offset, 0) == offset) {
      if (vfs_write(d->drive[d->drv].f, d->buffer, d->drive[d->drv].sectorlen) == d->drive[d->drv].sectorlen) {
        return 0;
      } else {
        debug_errno("DISK", "write");
        return 6;
      }
    } else {
      return 4;
    }
  }

  return 1;
}

void disk_out(disk_t *d, uint16_t port, uint8_t b) {
  int i;

  switch (port) {
    case DSKDRV:
      d->drv = b;
      debug(DEBUG_TRACE, "DISK", "drv = %d", d->drv);
      d->ptr = 0;
      break;
    case DSKTKH:
      d->track = (d->track & 0x00FF) | (((uint16_t)b) << 8);
      debug(DEBUG_TRACE, "DISK", "track (high) = %d", d->track);
      d->ptr = 0;
      break;
    case DSKTKL:
      d->track = (d->track & 0xFF00) | b;
      debug(DEBUG_TRACE, "DISK", "track (low) = %d", d->track);
      d->ptr = 0;
      break;
    case DSKSECL:
      d->sector = (d->sector & 0xFF00) | b;
      if (d->drive[d->drv].sector0 && d->drive[d->drv].use0) d->sector++;
      debug(DEBUG_TRACE, "DISK", "sector (low) = %d", d->sector);
      d->ptr = 0;
      break;
    case DSKSECH:
      d->sector = (d->sector & 0x00FF) | (((uint16_t)b) << 8);
      if (d->drive[d->drv].sector0) d->sector++;
      debug(DEBUG_TRACE, "DISK", "sector (high) = %d", d->sector);
      d->ptr = 0;
      break;
    case DSKCMD:
      if (b == 0) {
        d->status = read_sector(d);
        debug(DEBUG_TRACE, "DISK", "read sector: %d", d->status);
        if (d->dma) {
          for (i = 0; i < d->drive[d->drv].sectorlen; i++) {
            d->c->putb(d->c, d->dma_addr + i, d->buffer[i]);
          }
        }

      } else if (b == 1) {
        if (d->dma) {
          for (i = 0; i < d->drive[d->drv].sectorlen; i++) {
            d->buffer[i] = d->c->getb(d->c, d->dma_addr + i);
          }
        }
        d->status = write_sector(d);
        debug(DEBUG_TRACE, "DISK", "write sector: %d", d->status);

      } else {
        debug(DEBUG_ERROR, "DISK", "invalid command %d", b);
        d->status = 7;
      }
      d->ptr = 0;
      break;
    case DSKDAT:
      debug(DEBUG_TRACE, "DISK", "write data %3d = 0x%02X (%c)", d->ptr, b, (b < 32 ? '.' : b));
      d->buffer[d->ptr++] = b;
      if (d->ptr == d->drive[d->drv].sectorlen) d->ptr = 0;
      break;
    case DSKDMAH:
      d->dma_addr = (d->dma_addr & 0x00FF) | (((uint16_t)b) << 8);
      break;
    case DSKDMAL:
      d->dma_addr = (d->dma_addr & 0xFF00) | b;
      break;
  }
}

uint8_t disk_in(disk_t *d, uint16_t port) {
  uint8_t b = 0;

  switch (port) {
    case DSKDRV:
      b = d->drv;
      break;
    case DSKTKH:
      b = d->track >> 8;
      break;
    case DSKTKL:
      b = d->track & 0xFF;
      break;
    case DSKSECL:
      b = d->sector & 0xFF;
      break;
    case DSKSECH:
      b = d->sector >> 8;
      break;
    case DSKCMD:
      b = d->status;
      debug(DEBUG_TRACE, "DISK", "status = %d", d->status);
      d->status = 0;
      break;
    case DSKDAT:
      b = d->buffer[d->ptr];
      debug(DEBUG_TRACE, "DISK", "read data %3d = 0x%02X (%c)", d->ptr, b, (b < 32 ? '.' : b));
      d->ptr++;
      if (d->ptr == d->drive[d->drv].sectorlen) d->ptr = 0;
      break;
    case DSKDMAH:
      b = d->dma_addr >> 8;
      break;
    case DSKDMAL:
      b = d->dma_addr & 0xFF;
      break;
  }

  return b;
}

disk_t *disk_init(computer_t *c, int ndrives, int dma, vfs_session_t *session) {
  disk_t *d;

  if ((d = xcalloc(1, sizeof(disk_t))) != NULL) {
    d->session = session;
    d->c = c;
    d->ndrives = ndrives;
    d->dma = dma;
  }

  return d;
}

int disk_insert(disk_t *d, int drive, int skip, int tracks, int heads, int sectors, int sectorlen, int sector0, char *name) {
  int r = -1;

  if (drive >= 0 && drive < d->ndrives && name) {
    d->drive[drive].f = vfs_open(d->session, name, d->drive[drive].readonly ? VFS_READ : VFS_RDWR);
    if (d->drive[drive].f != NULL) {
      d->drive[drive].skip = skip;
      d->drive[drive].tracks = tracks;
      d->drive[drive].heads = heads;
      d->drive[drive].sectors = sectors;
      d->drive[drive].sectorlen = sectorlen;
      d->drive[drive].sector0 = sector0;
      d->drive[drive].readonly = 0;
      debug(DEBUG_INFO, "DISK", "disk %s inserted", name);
      debug(DEBUG_INFO, "DISK", "geometry skip=%d, tracks=%d, heads=%d, sectors=%d, sectorlen=%d, sector0=%d", skip, tracks, heads, sectors, sectorlen, sector0);
      r = 0;
    }
  }

  return r;
}

int disk_read_lba(disk_t *d, uint32_t lba) {
  uint32_t offset;

  debug(DEBUG_TRACE, "DISK", "read_sector d=%u lba=%u", d->drv, lba);

  if (d->drv >= d->ndrives) {
    debug(DEBUG_ERROR, "DISK", "invalid drive %d", d->drv);
    return 1;
  }

  if (d->drive[d->drv].f != NULL) {
    if (lba == 0) {
      sys_memset(d->buffer, 0, d->drive[d->drv].sectorlen);
      d->buffer[0x1C2] = 0x06; // file system ID
      d->buffer[0x1C6] = 0x01;
      d->buffer[0x1FE] = 0x55;
      d->buffer[0x1FF] = 0xAA;
      return 0;
    }
    lba--;
    offset = lba * d->drive[d->drv].sectorlen;
    if (vfs_seek(d->drive[d->drv].f, offset, 0) == offset) {
      if (vfs_read(d->drive[d->drv].f, d->buffer, d->drive[d->drv].sectorlen) == d->drive[d->drv].sectorlen) {
        return 0;
      } else {
        debug_errno("DISK", "read");
        return 5;
      }
    } else {
      return 4;
    }
  }

  return 1;
}

int disk_read_lba2(disk_t *d, uint32_t lba, uint8_t *buffer, uint32_t len) {
  uint32_t offset;

  debug(DEBUG_TRACE, "DISK", "read_sector d=%u lba=%u", d->drv, lba);

  if (d->drv >= d->ndrives) {
    debug(DEBUG_ERROR, "DISK", "invalid drive %d", d->drv);
    return 1;
  }

  if (d->drive[d->drv].f != NULL) {
    if (lba == 0) {
      sys_memset(buffer, 0, d->drive[d->drv].sectorlen);
      buffer[0x1C2] = 0x06; // file system ID
      buffer[0x1C6] = 0x01;
      buffer[0x1FE] = 0x55;
      buffer[0x1FF] = 0xAA;
      return 0;
    }
    lba--;
    offset = lba * d->drive[d->drv].sectorlen;
    if (vfs_seek(d->drive[d->drv].f, offset, 0) == offset) {
      if (vfs_read(d->drive[d->drv].f, buffer, len) == len) {
        return 0;
      } else {
        debug_errno("DISK", "read");
        return 5;
      }
    } else {
      return 4;
    }
  }

  return 1;
}

int disk_read(disk_t *d, uint8_t *buf, int len) {
  int i, r = -1;

  if (read_sector(d) == 0) {
    for (i = 0; i < d->drive[d->drv].sectorlen && i < len; i++) {
      buf[i] = d->buffer[i];
    }
    r = 0;
  }

  if (d->drive[d->drv].sector0 && !d->drive[d->drv].use0) {
    debug(DEBUG_INFO, "DISK", "using sector0 from now on");
    d->drive[d->drv].use0 = 1;
  }

  return r;
}

int disk_boot(disk_t *d, uint16_t addr) {
  int i, r = -1;

  if (d->c) {
    d->drv = 0;
    d->track = 0;
    d->sector = 1;

    if (read_sector(d) == 0) {
      for (i = 0; i < d->drive[d->drv].sectorlen; i++) {
        d->c->putb(d->c, addr + i, d->buffer[i]);
      }
      r = 0;
    }
  }

  return r;
}

int disk_close(disk_t *d) {
  int i, r = -1;

  if (d) {
    for (i = 0; i < d->ndrives; i++) {
      if (d->drive[i].f != NULL) {
        vfs_close(d->drive[i].f);
        d->drive[i].f = NULL;
      }
    }
    xfree(d);
    r = 0;
  }

  return r;
}

int disk_validate(char *name, int *skip, int *tracks, int *heads, int *sectors, int *sectorlen, int *sector0, vfs_session_t *session) {
  unsigned int disk_magic, disk_flags, total_sectors;
  uint32_t disk_tracks, disk_heads, disk_sectors, disk_sectorlen;
  uint32_t header[6];
  vfs_file_t *f;

  if ((f = vfs_open(session, name, VFS_READ)) == NULL) {
    return -1;
  }

  if (vfs_read(f, (uint8_t *)header, sizeof(header)) != sizeof(header)) {
    debug(DEBUG_ERROR, "EMU", "error reading header");
    vfs_close(f);
    return -1;
  }
  vfs_close(f);

  get4l(&disk_magic, (unsigned char *)&header[0], 0);
  if (disk_magic != DISK_MAGIC) {
    debug(DEBUG_INFO, "EMU", "disk magic not found");
    return 0;
  }

  get4l(&disk_flags,     (unsigned char *)&header[1], 0);
  get4l(&disk_tracks,    (unsigned char *)&header[2], 0);
  get4l(&disk_heads,     (unsigned char *)&header[3], 0);
  get4l(&disk_sectors,   (unsigned char *)&header[4], 0);
  get4l(&disk_sectorlen, (unsigned char *)&header[5], 0);
  total_sectors = disk_tracks * disk_heads * disk_sectors;

  debug(DEBUG_INFO, "EMU", "disk flags 0x%08X", disk_flags);
  if (disk_flags & DISK_FLAG_SECTOR0) {
    debug(DEBUG_INFO, "EMU", "disk numbers sector from 0");
    *sector0 = 1;
  } else {
    *sector0 = 0;
  }
  if (disk_flags & DISK_FLAG_READONLY) {
    debug(DEBUG_INFO, "EMU", "disk is readonly");
  }
  debug(DEBUG_INFO, "EMU", "disk tracks %d", disk_tracks);
  debug(DEBUG_INFO, "EMU", "disk heads %d", disk_heads);
  debug(DEBUG_INFO, "EMU", "disk sectors %d", disk_sectors);
  debug(DEBUG_INFO, "EMU", "disk sectorlen %d", disk_sectorlen);
  debug(DEBUG_INFO, "EMU", "disk total sectors %d", total_sectors);

  *skip = sizeof(header) + total_sectors * 4;
  *tracks = disk_tracks;
  *heads = disk_heads;
  *sectors = disk_sectors;
  *sectorlen = disk_sectorlen;

  return 1;
}
