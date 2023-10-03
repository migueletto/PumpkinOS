#define FLOPPY_TRACKS  77
#define FLOPPY_SECTORS 26

#define HD_TRACKS  2048
#define HD_SECTORS 32

#define SECTOR_LEN 128

void disk_init(void);
void disk_finish(void);
int disk_open(int drive, char *name);

uint8_t read_sector(uint8_t drive, uint16_t track, uint8_t sector, uint8_t *buf);
uint8_t write_sector(uint8_t drive, uint16_t track, uint8_t sector, uint8_t *buf);
