#ifndef DOOM_GENERIC
#define DOOM_GENERIC

#include <stdlib.h>
#include <stdint.h>

//#define DOOMGENERIC_RESX 640
//#define DOOMGENERIC_RESY 400
#define DOOMGENERIC_RESX 320
#define DOOMGENERIC_RESY 200


extern uint32_t* DG_ScreenBuffer;

typedef struct dg_file_t dg_file_t;

void DG_Init();
void DG_DrawFrame();
int DG_SleepMs(uint32_t ms);
uint32_t DG_GetTicksMs();
int DG_GetKey(int* pressed, unsigned char* key);
void DG_StatusTop(char *s);
void DG_Status(int health, int armor, int ammo0, int maxammo0, int ammo1, int maxammo1, int ammo2, int maxammo2, int ammo3, int maxammo3, int faceindex);
void DG_ClearMsg(void);
void DG_PrintMsg(char *s);
void DG_SetWindowTitle(const char * title);
void DG_Fatal(char *msg);

int DG_create(char *name);
dg_file_t *DG_open(char *name, int wr);
void DG_close(dg_file_t *f);
int DG_read(dg_file_t *f, void *buf, int n);
int DG_write(dg_file_t *f, void *buf, int n);
int DG_seek(dg_file_t *f, int offset);
int DG_tell(dg_file_t *f);
int DG_filesize(dg_file_t *f);
int DG_mkdir(char *name);
int DG_remove(char *name);
int DG_rename(char *oldname, char *newname);

void DG_debug_full(const char *file, const char *func, int line, int level, const char *fmt, ...);
#define DG_debug(level, fmt, args...) DG_debug_full(__FILE__, __FUNCTION__, __LINE__, level, fmt, ##args)

#endif //DOOM_GENERIC
