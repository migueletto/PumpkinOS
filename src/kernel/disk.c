#include "sys.h"
#include "ff.h"
#include "diskio.h"
#include "disk.h"

#define SECTOR_LEN FF_MIN_SS

#define DISK_SIZE 100 * 1024 * 1024

static uint8_t disk[DISK_SIZE] = {
#include "diskbin.h"
};

static FATFS fatfs;

void disk_init(void) {
  f_mount(&fatfs, "", 0);
}

DSTATUS disk_initialize(BYTE pdrv) {
  return 0;
}

DSTATUS disk_status(BYTE pdrv) {
  return 0;
}

DRESULT disk_read(BYTE pdrv, BYTE *buff, LBA_t sector, UINT count) {
  uint32_t offset = sector * SECTOR_LEN;
  UINT cnt;
  DRESULT r = RES_ERROR;

  if (offset < DISK_SIZE) {
    cnt = (DISK_SIZE - offset) / SECTOR_LEN;
    if (count > cnt) count = cnt;
    if (count > 0) {
      sys_memcpy(buff, &disk[offset], count * SECTOR_LEN);
      r = RES_OK;
    }
  }

  return r;
}

DRESULT disk_write(BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count) {
  uint32_t offset = sector * SECTOR_LEN;
  UINT cnt;
  DRESULT r = RES_ERROR;

  if (offset < DISK_SIZE) {
    cnt = (DISK_SIZE - offset) / SECTOR_LEN;
    if (count > cnt) count = cnt;
    if (count > 0) {
      sys_memcpy(&disk[offset], buff, count * SECTOR_LEN);
      r = RES_OK;
    }
  }

  return r;
}

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buff) {
  LBA_t *size;
  DRESULT r = RES_ERROR;

  switch (cmd) {
    case CTRL_SYNC:
      r = RES_OK;
      break;
    case GET_SECTOR_COUNT:
      size = (LBA_t *)buff;
      *size = DISK_SIZE / SECTOR_LEN;
      r = RES_OK;
      break;
  }

  return r;
}
