#define MAX_DRIVES  4

#define DSKDRV    0           // set drive command
#define DSKTKH    1           // set track high command
#define DSKTKL    2           // set track low command
#define DSKSECL   3           // set sector command (low)
#define DSKCMD    4           // disk command/status
#define DSKDAT    5           // disk data
#define DSKSECH   6           // set sector command (high)
#define DSKDMAH   8           // set dma address (high)
#define DSKDMAL   9           // set dma address (low)
#define DSKHEAD   10          // set head command

typedef struct disk_t disk_t;

disk_t *disk_init(computer_t *c, int ndrives, int dma, vfs_session_t *session);
int disk_insert(disk_t *d, int drive, int skip, int tracks, int heads, int sectors, int sectorlen, int sector0, char *name);
void disk_out(disk_t *d, uint16_t port, uint8_t b);
uint8_t disk_in(disk_t *d, uint16_t port);
int disk_read_lba(disk_t *d, uint32_t lba);
int disk_read_lba2(disk_t *d, uint32_t lba, uint8_t *buffer, uint32_t len);
int disk_read(disk_t *d, uint8_t *buf, int len);
int disk_boot(disk_t *d, uint16_t addr);
int disk_close(disk_t *d);
int disk_validate(char *name, int *skip, int *disk_tracks, int *disk_heads, int *disk_sectors, int *disk_sectorlen, int *sector0, vfs_session_t *session);
