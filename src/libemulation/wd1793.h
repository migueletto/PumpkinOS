#define WD1793_COMMAND   0
#define WD1793_STATUS    0
#define WD1793_TRACK     1
#define WD1793_SECTOR    2
#define WD1793_DATA      3

typedef struct wd1793_t wd1793_t;

wd1793_t *wd1793_init(disk_t *d, void *data);
void wd1793_set_dr(wd1793_t *d, void (*wdDataRequest)(int r, void *cdata));
void wd1793_set_ir(wd1793_t *d, void (*wdInterruptRequest)(int r, void *cdata));
int wd1793_insert(wd1793_t *d, int drive, int skip, int tracks, int heads, int sectors, int sectorlen, int sector0, char *name);
int wd1793_close(wd1793_t *d);

void wd1793_set_drive(wd1793_t *d, int drive);
void wd1793_set_head(wd1793_t *d, int head);
void wd1793_write(wd1793_t *d, int cmd, uint8_t b);
uint8_t wd1793_read(wd1793_t *d, int cmd);
