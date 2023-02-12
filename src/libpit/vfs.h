#ifndef PIT_VFS_H
#define PIT_VFS_H

#ifdef __cplusplus
extern "C" {
#endif

#define VFS_PATH  512
#define VFS_NAME  256

#define VFS_FILE  1
#define VFS_DIR   2

#define VFS_READ  SYS_READ
#define VFS_WRITE SYS_WRITE
#define VFS_RDWR  SYS_RDWR
#define VFS_TRUNC SYS_TRUNC

int vfs_init(void);

int vfs_refresh(void);

int vfs_finish(void);

typedef struct vfs_ent_t {
  char name[VFS_NAME];
  uint32_t size;
  time_t atime;
  time_t mtime;
  time_t ctime;
  uint8_t type;
  uint8_t rd, wr;
} vfs_ent_t;

typedef struct vfs_session_t vfs_session_t;
typedef struct vfs_dir_t vfs_dir_t;
typedef struct vfs_file_t vfs_file_t;

typedef struct vfs_priv_t vfs_priv_t;
typedef struct vfs_fpriv_t vfs_fpriv_t;

typedef struct {
  int (*refresh)(void *data);

  int (*unmap)(void *data);

  char *(*getmount)(void *data);

  int (*checktype)(char *path, void *data);

  vfs_priv_t *(*opendir)(char *path, void *data);

  vfs_ent_t *(*readdir)(vfs_priv_t *priv);

  int (*closedir)(vfs_priv_t *priv);

  vfs_fpriv_t *(*open)(char *path, int mode, void *data);

  int (*peek)(vfs_fpriv_t *f, uint32_t us);

  int (*read)(vfs_fpriv_t *f, uint8_t *buf, uint32_t len);

  int (*write)(vfs_fpriv_t *f, uint8_t *buf, uint32_t len);

  int (*close)(vfs_fpriv_t *f);

  uint32_t (*seek)(vfs_fpriv_t *f, uint32_t pos, int fromend);

  vfs_ent_t *(*fstat)(vfs_fpriv_t *fpriv);

  vfs_ent_t *(*stat)(char *path, void *data, vfs_ent_t *ent);

  int (*rename)(char *path1, char *path2, void *data);

  int (*unlink)(char *path, void *data);

  int (*mkdir)(char *path, void *data);

  int (*statfs)(char *path, uint64_t *total, uint64_t *free, void *data);

  void *(*loadlib)(char *path, int *first_load, void *data);
} vfs_callback_t;

int vfs_map(char *label, char *path, void *data, vfs_callback_t *callback, int raw);

char *vfs_abspath(char *cwd, char *relpath);

vfs_session_t *vfs_open_session(void);

int vfs_close_session(vfs_session_t *session);

int vfs_statfs(vfs_session_t *session, char *path, uint64_t *total, uint64_t *free);

char *vfs_getmount(vfs_session_t *session, char *path);

int vfs_checktype(vfs_session_t *session, char *path);

int vfs_chdir(vfs_session_t *session, char *path);

int vfs_mkdir(vfs_session_t *session, char *path);

char *vfs_cwd(vfs_session_t *session);

int vfs_type(void *p);

vfs_dir_t *vfs_opendir(vfs_session_t *session, char *path);

vfs_ent_t *vfs_readdir(vfs_dir_t *dir);

int vfs_closedir(vfs_dir_t *dir);

vfs_file_t *vfs_open(vfs_session_t *session, char *path, int mode);

int vfs_peek(vfs_file_t *f, uint32_t us);

int vfs_read(vfs_file_t *f, uint8_t *buf, uint32_t len);

int vfs_write(vfs_file_t *f, uint8_t *buf, uint32_t len);

int vfs_close(vfs_file_t *f);

uint32_t vfs_seek(vfs_file_t *f, uint32_t pos, int fromend);

void vfs_rewind(vfs_file_t *f);

vfs_ent_t *vfs_fstat(vfs_file_t *f);

vfs_ent_t *vfs_stat(vfs_session_t *session, char *path, vfs_ent_t *ent);

int vfs_rename(vfs_session_t *session, char *path1, char *path2);

int vfs_unlink(vfs_session_t *session, char *path);

int vfs_getc(vfs_file_t *f);

char *vfs_gets(vfs_file_t *f, char *s, uint32_t len);

int vfs_putc(vfs_file_t *f, int c);

int vfs_printf(vfs_file_t *f, char *fmt, ...);

void *vfs_loadlib(vfs_session_t *session, char *path, int *first_load);

#ifdef __cplusplus
}
#endif

#endif
