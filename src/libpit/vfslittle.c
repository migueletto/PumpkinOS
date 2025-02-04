#include "sys.h"
#include "vfs.h"
#include "script.h"
#include "lfs.h"
#include "debug.h"

#define VFS_LITTLE "littlefs"

typedef struct {
  char *local;
  vfs_ent_t current;
  struct lfs_config cfg;
  lfs_t lfs;
} vfsassets_mount_t;

typedef struct vfs_priv_t {
  lfs_t *lfs;
  vfs_ent_t current;
  char path[VFS_PATH];
  char aux[VFS_PATH];
  lfs_dir_t dir;
} vfs_priv_t;

typedef struct vfs_fpriv_t {
  lfs_t *lfs;
  lfs_file_t file;
  vfs_ent_t current;
} vfs_fpriv_t;

static int littlefs_block_device_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size);
static int littlefs_block_device_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size);
static int littlefs_block_device_erase(const struct lfs_config *c, lfs_block_t block);
static int littlefs_block_device_sync(const struct lfs_config *c);

static char *vfs_little_getmount(void *data);
static int vfs_little_checktype(char *path, void *data);
static vfs_priv_t *vfs_little_opendir(char *path, void *data);
static vfs_ent_t *vfs_little_readdir(vfs_priv_t *priv);
static int vfs_little_closedir(vfs_priv_t *priv);

static vfs_fpriv_t *vfs_little_open(char *path, int mode, void *data);
static int vfs_little_peek(vfs_fpriv_t *fpriv, uint32_t us);
static int vfs_little_read(vfs_fpriv_t *fpriv, uint8_t *buf, uint32_t len);
static int vfs_little_write(vfs_fpriv_t *fpriv, uint8_t *buf, uint32_t len);
static int vfs_little_close(vfs_fpriv_t *fpriv);
static uint32_t vfs_little_seek(vfs_fpriv_t *fpriv, uint32_t pos, int fromend);
static int vfs_little_truncate(vfs_fpriv_t *fpriv, uint32_t offset);
static vfs_ent_t *vfs_little_fstat(vfs_fpriv_t *fpriv);
static vfs_ent_t *vfs_little_stat(char *path, void *data, vfs_ent_t *ent);

static int vfs_little_rename(char *path1, char *path2, void *data);
static int vfs_little_unlink(char *path, void *data);
static int vfs_little_mkdir(char *path, void *data);
static int vfs_little_statfs(char *path, uint64_t *total, uint64_t *free, void *data);
static void *vfs_little_loadlib(char *path, int *first_load, void *data);

static vfs_callback_t little_callback = {
  NULL,
  NULL,
  vfs_little_getmount,
  vfs_little_checktype,
  vfs_little_opendir,
  vfs_little_readdir,
  vfs_little_closedir,
  vfs_little_open,
  vfs_little_peek,
  vfs_little_read,
  vfs_little_write,
  vfs_little_close,
  vfs_little_seek,
  vfs_little_fstat,
  vfs_little_stat,
  vfs_little_rename,
  vfs_little_unlink,
  vfs_little_mkdir,
  vfs_little_statfs,
  vfs_little_loadlib,
  vfs_little_truncate
};

int vfs_little_mount(char *local, char *path) {
  vfsassets_mount_t dummy, *data;
  char *abspath;

  dummy.local = local;
  if (vfs_little_checktype("", &dummy) == -1) {
    return -1;
  }

  if ((abspath = vfs_abspath("/", path)) == NULL) {
    return -1;
  }

  if ((data = sys_calloc(1, sizeof(vfsassets_mount_t))) == NULL) {
    sys_free(abspath);
    return -1;
  }

  if ((data->local = sys_strdup(local)) == NULL) {
    sys_free(data);
    sys_free(abspath);
    return -1;
  }

  if (vfs_map(VFS_LITTLE, abspath, data, &little_callback, 0) == -1) {
    sys_free(data->local);
    sys_free(data);
    sys_free(abspath);
    return -1;
  }
  sys_free(abspath);

  data->cfg.context = data;

  // block device operations
  data->cfg.read  = littlefs_block_device_read;
  data->cfg.prog  = littlefs_block_device_prog;
  data->cfg.erase = littlefs_block_device_erase;
  data->cfg.sync  = littlefs_block_device_sync;

  // block device configuration
  data->cfg.read_size = 16;
  data->cfg.prog_size = 16;
  data->cfg.block_size = 4096;
  data->cfg.block_count = 128;
  data->cfg.cache_size = 16;
  data->cfg.lookahead_size = 16;
  data->cfg.block_cycles = -1;

  if (lfs_mount(&data->lfs, &data->cfg) != 0) {
    lfs_format(&data->lfs, &data->cfg);
    lfs_mount(&data->lfs, &data->cfg);
  }

  return 0;
}

static char *vfs_little_getmount(void *_data) {
  vfsassets_mount_t *data;

  data = (vfsassets_mount_t *)_data;
  return data->local;
}

static int vfs_little_checktype(char *path, void *_data) {
  vfsassets_mount_t *data = (vfsassets_mount_t *)_data;
  struct lfs_info info;

  if (lfs_stat(&data->lfs, path, &info) < 0) {
    return -1;
  }

  if (info.type == LFS_TYPE_DIR) {
    return VFS_DIR;
  }

  if (info.type & LFS_TYPE_REG) {
    return VFS_FILE;
  }

  debug(DEBUG_ERROR, "LITTLEFS", "\"%s\" is not file or directory", path);
  return -1;
}

static vfs_ent_t *vfs_little_readdir(vfs_priv_t *priv) {
  struct lfs_info info;

  if (!priv) {
    return NULL;
  }

  if (lfs_dir_read(priv->lfs, &priv->dir, &info) < 0) {
    return NULL;
  }

  sys_memset(&priv->current, 0, sizeof(vfs_ent_t));
  sys_strncpy(priv->current.name, info.name, VFS_NAME-1);
  priv->current.size = info.size;
  priv->current.type = (info.type == LFS_TYPE_REG) ? VFS_FILE : VFS_DIR;
  priv->current.rd   = 1;
  priv->current.wr   = 1;

  return &priv->current;
}

static int vfs_little_closedir(vfs_priv_t *priv) {
  int r = -1;

  if (priv) {
    r = lfs_dir_close(priv->lfs, &priv->dir);
    sys_free(priv);
  }

  return r;
}

static vfs_priv_t *vfs_little_opendir(char *path, void *_data) {
  vfsassets_mount_t *data = (vfsassets_mount_t *)_data;
  vfs_priv_t *priv;

  if ((priv = sys_calloc(1, sizeof(vfs_priv_t))) == NULL) {
    return NULL;
  }

  debug(DEBUG_TRACE, "LITTLEFS", "vfs_little_opendir \"%s\"", path);

  if (lfs_dir_open(&data->lfs, &priv->dir, path) < 0) {
    sys_free(priv);
    return NULL;
  }

  priv->lfs = &data->lfs;

  return priv;
}

static int vfs_little_peek(vfs_fpriv_t *fpriv, uint32_t us) {
  lfs_soff_t pos1, pos2;

  pos1 = lfs_file_seek(fpriv->lfs, &fpriv->file, 0, LFS_SEEK_CUR);
  if (pos1 < 0) return -1;
  pos2 = lfs_file_seek(fpriv->lfs, &fpriv->file, 0, LFS_SEEK_END);
  if (pos2 < 0) return -1;
  if (lfs_file_seek(fpriv->lfs, &fpriv->file, pos1, LFS_SEEK_SET) < 0) return -1;

  return pos1 < pos2 ? 1 : 0;
}

static int vfs_little_read(vfs_fpriv_t *fpriv, uint8_t *buf, uint32_t len) {
  int r = -1;

  if (fpriv && buf) {
    r = lfs_file_read(fpriv->lfs, &fpriv->file, buf, len);
  }

  return r;
}

static int vfs_little_write(vfs_fpriv_t *fpriv, uint8_t *buf, uint32_t len) {
  int r = -1;

  if (fpriv && buf) {
    r = lfs_file_write(fpriv->lfs, &fpriv->file, buf, len);
  }

  return r;
}

static int vfs_little_close(vfs_fpriv_t *fpriv) {
  int r = -1;

  if (fpriv) {
    r = lfs_file_close(fpriv->lfs, &fpriv->file);
    sys_free(fpriv);
  }

  return r;
}

static vfs_ent_t *vfs_little_fstat(vfs_fpriv_t *fpriv) {
  vfs_ent_t *ent = NULL;

  if (fpriv) {
    sys_memset(&fpriv->current, 0, sizeof(vfs_ent_t));
    fpriv->current.type = (fpriv->file.type == LFS_TYPE_REG) ? VFS_FILE : VFS_DIR;
    fpriv->current.size = lfs_file_size(fpriv->lfs, &fpriv->file);
    ent = &fpriv->current;
  }

  return ent;
}

static vfs_ent_t *vfs_little_stat(char *path, void *_data, vfs_ent_t *ent) {
  vfsassets_mount_t *data = (vfsassets_mount_t *)_data;
  struct lfs_info info;
  vfs_ent_t *r = NULL;

  debug(DEBUG_TRACE, "LITTLEFS", "vfs_little_stat \"%s\"", path);

  if (lfs_stat(&data->lfs, path, &info) == 0) {
    sys_memset(ent, 0, sizeof(vfs_ent_t));
    ent->type = (info.type == LFS_TYPE_REG) ? VFS_FILE : VFS_DIR;
    ent->size = info.size;
    r = ent;
  }

  return r;
}

static uint32_t vfs_little_seek(vfs_fpriv_t *fpriv, uint32_t pos, int fromend) {
  int whence = SYS_SEEK_SET;
  uint32_t r = -1;

  switch (fromend) {
    case -1: whence = LFS_SEEK_CUR; break;
    case  0: whence = LFS_SEEK_SET; break;
    case  1: whence = LFS_SEEK_END; break;
  }

  if (fpriv) {
    r = lfs_file_seek(fpriv->lfs, &fpriv->file, pos, whence);
  }

  return r;
}

static int vfs_little_truncate(vfs_fpriv_t *fpriv, uint32_t offset) {
  int r = -1;

  if (fpriv) {
    r = lfs_file_truncate(fpriv->lfs, &fpriv->file, offset);
  }

  return r;
}

static vfs_fpriv_t *vfs_little_open(char *path, int mode, void *_data) {
  vfsassets_mount_t *data = (vfsassets_mount_t *)_data;
  vfs_fpriv_t *fpriv;
  int flags, err;

  debug(DEBUG_TRACE, "VFS", "vfs_little_open \"%s\" mode 0x%04X", path, mode);

  flags = 0;
  if (mode & VFS_READ) {
    flags |= LFS_O_RDONLY;
  }
  if (mode & VFS_WRITE) {
    flags |= LFS_O_WRONLY | LFS_O_CREAT;
  }
  if (mode & VFS_TRUNC) {
    flags |= LFS_O_TRUNC | LFS_O_CREAT;
  }

  if ((fpriv = sys_calloc(1, sizeof(vfs_fpriv_t))) == NULL) {
    return NULL;
  }

  if ((err = lfs_file_open(&data->lfs, &fpriv->file, path, flags)) < 0) {
    debug(DEBUG_ERROR, "LITTLEFS", "lfs_file_open(\"%s\", %d): %d", path, mode, err);
    sys_free(fpriv);
    return NULL;
  }

  fpriv->lfs = &data->lfs;

  return fpriv;
}

static int vfs_little_rename(char *path1, char *path2, void *_data) {
  vfsassets_mount_t *data = (vfsassets_mount_t *)_data;
  int r;

  r = lfs_rename(&data->lfs, path1, path2);

  return r;
}

static int vfs_little_unlink(char *path, void *_data) {
  vfsassets_mount_t *data = (vfsassets_mount_t *)_data;
  int r;

  debug(DEBUG_TRACE, "LITTLEFS", "vfs_little_unlink \"%s\"", path);
  r = lfs_remove(&data->lfs, path);

  return r;
}

static int vfs_little_mkdir(char *path, void *_data) {
  vfsassets_mount_t *data = (vfsassets_mount_t *)_data;
  int r;

  debug(DEBUG_TRACE, "LITTLEFS", "vfs_little_mkdir \"%s\"", path);
  r = lfs_mkdir(&data->lfs, path);

  return r;
}

static int vfs_little_statfs(char *path, uint64_t *total, uint64_t *free, void *_data) {
  vfsassets_mount_t *data = (vfsassets_mount_t *)_data;
  struct lfs_fsinfo fsinfo;
  int r;

  debug(DEBUG_TRACE, "LITTLEFS", "vfs_little_statfs \"%s\"", path);

  if ((r = lfs_fs_stat(&data->lfs, &fsinfo)) == 0) {
    *total = fsinfo.block_count * fsinfo.block_size;
    *free = 0; // XXX how to obtain free space ?
  }

  return r;
}

#define LOAD_BUF_SIZE 65536

static void *vfs_little_loadlib(char *path, int *first_load, void *_data) {
  vfsassets_mount_t *data = (vfsassets_mount_t *)_data;
  lfs_file_t file;
  char name[32];
  int r, fd;
  void *buf, *lib = NULL;

  debug(DEBUG_TRACE, "LITTLEFS", "vfs_little_loadlib \"%s\"", path);

  if (lfs_file_open(&data->lfs, &file, path, LFS_O_RDONLY) == 0) {
    if ((buf = sys_malloc(LOAD_BUF_SIZE)) != NULL) {
      sys_strcpy(name, "tmpXXXXXX");
      if ((fd = sys_mkstempfile(name)) != -1) {
        for (;;) {
          if ((r = lfs_file_read(&data->lfs, &file, buf, LOAD_BUF_SIZE)) < 0) break;
          if (r > 0) sys_write(fd, buf, r);
          if (r < LOAD_BUF_SIZE) break;
        }
        sys_close(fd);
      }
      sys_free(buf);
      lib = sys_lib_load(name, first_load);
      sys_unlink(name);
    }
    lfs_file_close(&data->lfs, &file);
  }

  return lib;
}

static int littlefs_block_device_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size) {
  vfsassets_mount_t *data = (vfsassets_mount_t *)c->context;

  debug(DEBUG_INFO, "LITTLEFS", "block read block=%u offset=%u size=%u", block, off, size);

  return 0;
}

static int littlefs_block_device_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size) {
  vfsassets_mount_t *data = (vfsassets_mount_t *)c->context;

  debug(DEBUG_INFO, "LITTLEFS", "block write block=%u offset=%u size=%u", block, off, size);

  return 0;
}

static int littlefs_block_device_erase(const struct lfs_config *c, lfs_block_t block) {
  return 0;
}

static int littlefs_block_device_sync(const struct lfs_config *c) {
  return 0;
}
