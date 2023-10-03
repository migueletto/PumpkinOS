#ifndef DOOM_GENERIC
#define DOOM_GENERIC

#include <stdlib.h>
#include <stdint.h>

#include "sys.h"

typedef struct dg_file_t dg_file_t;
typedef struct dg_dir_t dg_dir_t;

uint16_t *DG_GetScreenBuffer(void);
void DG_DrawFrame(void);
int DG_SleepMs(uint32_t ms);
uint32_t DG_GetTicksMs(void);
int DG_GetKey(int* pressed, unsigned char* key);
void DG_ClearMsg(void);
void DG_PrintMsg(char *s);
void DG_SetWindowTitle(const char * title);
void DG_Fatal(const char *filename, const char *function, int lineNo, char *msg);
void DG_StartFrame(void);
void DG_sectorTag(int tag);

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
int DG_eof(dg_file_t *f);
int DG_printf(dg_file_t *f, const char *fmt, ...);
int DG_vprintf(dg_file_t *f, const char *fmt, sys_va_list ap);
char *DG_fgets(dg_file_t *f, char *s, int size);
dg_dir_t *DG_opendir(char *name);
int DG_readdir(dg_dir_t *f, char *name, int len);
void DG_closedir(dg_dir_t *f);

uint16_t DG_color16(uint8_t r, uint8_t g, uint8_t b);

void DG_debug_full(const char *file, const char *func, int line, int level, const char *fmt, ...);
#define DG_debug(level, fmt, args...) DG_debug_full(__FILE__, __FUNCTION__, __LINE__, level, fmt, ##args)

void DG_debugva_full(const char *file, const char *func, int line, int level, const char *fmt, sys_va_list ap);
#define DG_debugva(level, fmt, ap) DG_debugva_full(__FILE__, __FUNCTION__, __LINE__, level, fmt, ap)

void M_Quit(void);
void setPalette(void);
void saveIcons(void);

#endif //DOOM_GENERIC
