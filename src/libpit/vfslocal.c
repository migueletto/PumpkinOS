#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

#include "script.h"
#include "vfs.h"
#include "vfslocal.h"
#include "sys.h"
#include "xalloc.h"
#include "debug.h"

#define VFS_LOCAL "local filesystem"

typedef struct {
  char *local;
  vfs_ent_t current;
} vfsassets_mount_t;

typedef struct vfs_priv_t {
  vfs_ent_t current;
  char path[VFS_PATH];
  char aux[VFS_PATH];
  sys_dir_t *dir;
} vfs_priv_t;

typedef struct vfs_fpriv_t {
  int fd;
  vfs_ent_t current;
} vfs_fpriv_t;

static char *vfs_local_getmount(void *data);
static int vfs_local_checktype(char *path, void *data);
static vfs_priv_t *vfs_local_opendir(char *path, void *data);
static vfs_ent_t *vfs_local_readdir(vfs_priv_t *priv);
static int vfs_local_closedir(vfs_priv_t *priv);

static vfs_fpriv_t *vfs_local_open(char *path, int mode, void *data);
static int vfs_local_peek(vfs_fpriv_t *fpriv, uint32_t us);
static int vfs_local_read(vfs_fpriv_t *fpriv, uint8_t *buf, uint32_t len);
static int vfs_local_write(vfs_fpriv_t *fpriv, uint8_t *buf, uint32_t len);
static int vfs_local_close(vfs_fpriv_t *fpriv);
static uint32_t vfs_local_seek(vfs_fpriv_t *fpriv, uint32_t pos, int fromend);
static vfs_ent_t *vfs_local_fstat(vfs_fpriv_t *fpriv);
static vfs_ent_t *vfs_local_stat(char *path, void *data, vfs_ent_t *ent);

static int vfs_local_rename(char *path1, char *path2, void *data);
static int vfs_local_unlink(char *path, void *data);
static int vfs_local_mkdir(char *path, void *data);
static int vfs_local_statfs(char *path, uint64_t *total, uint64_t *free, void *data);
static void *vfs_local_loadlib(char *path, int *first_load, void *data);

static vfs_callback_t local_callback = {
  NULL,
  NULL,
  vfs_local_getmount,
  vfs_local_checktype,
  vfs_local_opendir,
  vfs_local_readdir,
  vfs_local_closedir,
  vfs_local_open,
  vfs_local_peek,
  vfs_local_read,
  vfs_local_write,
  vfs_local_close,
  vfs_local_seek,
  vfs_local_fstat,
  vfs_local_stat,
  vfs_local_rename,
  vfs_local_unlink,
  vfs_local_mkdir,
  vfs_local_statfs,
  vfs_local_loadlib
};

int vfs_local_mount(char *local, char *path) {
  vfsassets_mount_t dummy, *data;
  char *abspath;

  dummy.local = local;
  if (vfs_local_checktype("", &dummy) == -1) {
    return -1;
  }

  if ((abspath = vfs_abspath("/", path)) == NULL) {
    return -1;
  }

  if ((data = xcalloc(1, sizeof(vfsassets_mount_t))) == NULL) {
    xfree(abspath);
    return -1;
  }

  if ((data->local = xstrdup(local)) == NULL) {
    xfree(data);
    xfree(abspath);
    return -1;
  }

  if (vfs_map(VFS_LOCAL, abspath, data, &local_callback, 0) == -1) {
    xfree(data->local);
    xfree(data);
    xfree(abspath);
    return -1;
  }
  xfree(abspath);

  return 0;
}

static char *vfs_local_getmount(void *_data) {
  vfsassets_mount_t *data;

  data = (vfsassets_mount_t *)_data;
  return data->local;
}

static int vfs_local_checktype(char *path, void *_data) {
  vfsassets_mount_t *data;
  sys_stat_t st;
  char *aux;

  data = (vfsassets_mount_t *)_data;

  if ((aux = xcalloc(1, VFS_PATH)) == NULL) {
    return -1;
  }
  snprintf(aux, VFS_PATH-1, "%s%s", data->local, path);

  if (sys_stat(aux, &st) == -1) {
    xfree(aux);
    return -1;
  }

  if (st.mode & SYS_IFDIR) {
    xfree(aux);
    return VFS_DIR;
  }

  if (st.mode & SYS_IFREG) {
    xfree(aux);
    return VFS_FILE;
  }

  debug(DEBUG_ERROR, "VFS", "\"%s\" is not file or directory", aux);
  xfree(aux);
  return -1;
}

static vfs_ent_t *vfs_local_readdir(vfs_priv_t *priv) {
  char name[FILE_PATH];
  sys_stat_t st;

  if (!priv || !priv->dir) {
    return NULL;
  }

  for (;;) {
    if (sys_readdir(priv->dir, name, sizeof(name)-1) == -1) {
      return NULL;
    }

    strncpy(priv->aux, priv->path, VFS_PATH-1);
    strncat(priv->aux, name, VFS_PATH-strlen(priv->aux)-1);

    if (sys_stat(priv->aux, &st) == -1) {
      return NULL;
    }

    if (st.mode & (SYS_IFREG | SYS_IFDIR)) {
      break;
    }
  }

  xmemset(&priv->current, 0, sizeof(vfs_ent_t));
  strncpy(priv->current.name, name, VFS_NAME-1);
  priv->current.size  = st.size;
  priv->current.atime = st.atime;
  priv->current.mtime = st.mtime;
  priv->current.ctime = st.ctime;
  priv->current.type  = (st.mode & SYS_IFREG) ? VFS_FILE : VFS_DIR;
  priv->current.rd    = (st.mode & SYS_IRUSR) ? 1 : 0;
  priv->current.wr    = (st.mode & SYS_IWUSR) ? 1 : 0;

  return &priv->current;
}

static int vfs_local_closedir(vfs_priv_t *priv) {
  int r = -1;

  if (priv) {
    if (priv->dir) {
      r = sys_closedir(priv->dir);
    }
    xfree(priv);
  }

  return r;
}

static vfs_priv_t *vfs_local_opendir(char *path, void *_data) {
  vfsassets_mount_t *data;
  vfs_priv_t *priv;

  data = (vfsassets_mount_t *)_data;

  if ((priv = xcalloc(1, sizeof(vfs_priv_t))) == NULL) {
    return NULL;
  }

  debug(DEBUG_TRACE, "VFS", "vfs_local_opendir \"%s\"", path);
  if (path[0] == 0) {
    strncpy(priv->path, data->local, VFS_PATH-1);
  } else {
    snprintf(priv->path, VFS_PATH-1, "%s%s%c", data->local, path, '/');
  }
  debug(DEBUG_TRACE, "VFS", "aux \"%s\"", priv->path);

  if ((priv->dir = sys_opendir(priv->path)) == NULL) {
    xfree(priv);
    return NULL;
  }

  return priv;
}

static int vfs_local_peek(vfs_fpriv_t *fpriv, uint32_t us) {
  uint32_t pos1, pos2;

  pos1 = sys_seek(fpriv->fd, 0, SYS_SEEK_CUR);
  if (pos1 == -1) return -1;
  pos2 = sys_seek(fpriv->fd, 0, SYS_SEEK_END);
  if (pos2 == -1) return -1;
  if (sys_seek(fpriv->fd, pos1, SYS_SEEK_SET) == -1) return -1;

  return pos1 < pos2 ? 1 : 0;
}

static int vfs_local_read(vfs_fpriv_t *fpriv, uint8_t *buf, uint32_t len) {
  int r = -1;

  if (fpriv && buf) {
    if (fpriv->fd) {
      r = sys_read(fpriv->fd, buf, len);
    }
  }

  return r;
}

static int vfs_local_write(vfs_fpriv_t *fpriv, uint8_t *buf, uint32_t len) {
  int r = -1;

  if (fpriv && buf) {
    if (fpriv->fd) {
      r = sys_write(fpriv->fd, buf, len);
    }
  }

  return r;
}

static int vfs_local_close(vfs_fpriv_t *fpriv) {
  int r = -1;

  if (fpriv) {
    if (fpriv->fd) {
      r = sys_close(fpriv->fd);
    }
    xfree(fpriv);
  }

  return r;
}

static vfs_ent_t *vfs_local_fstat(vfs_fpriv_t *fpriv) {
  sys_stat_t st;
  vfs_ent_t *ent = NULL;

  if (fpriv && fpriv->fd) {
    if (sys_fstat(fpriv->fd, &st) == 0) {
      if (st.mode & (SYS_IFREG | SYS_IFDIR)) {
        xmemset(&fpriv->current, 0, sizeof(vfs_ent_t));
        fpriv->current.type = (st.mode & SYS_IFREG) ? VFS_FILE : VFS_DIR;
        fpriv->current.size = st.size;
        fpriv->current.atime = st.atime;
        fpriv->current.mtime = st.mtime;
        fpriv->current.ctime = st.ctime;
        ent = &fpriv->current;
      }
    } else {
      debug_errno("VFS", "sys_fstat(%d)", fpriv->fd);
    }
  }

  return ent;
}

static vfs_ent_t *vfs_local_stat(char *path, void *_data, vfs_ent_t *ent) {
  vfsassets_mount_t *data;
  char *aux;
  sys_stat_t st;
  vfs_ent_t *r = NULL;

  data = (vfsassets_mount_t *)_data;
  debug(DEBUG_TRACE, "VFS", "vfs_local_stat \"%s\"", path);

  if ((aux = xcalloc(1, VFS_PATH)) == NULL) {
    return NULL;
  }

  snprintf(aux, VFS_PATH-1, "%s%s", data->local, path);
  debug(DEBUG_TRACE, "VFS", "aux \"%s\"", aux);

  if (sys_stat(aux, &st) == 0) {
    if (st.mode & (SYS_IFREG | SYS_IFDIR)) {
      xmemset(ent, 0, sizeof(vfs_ent_t));
      ent->type = (st.mode & SYS_IFREG) ? VFS_FILE : VFS_DIR;
      ent->size = st.size;
      ent->atime = st.atime;
      ent->mtime = st.mtime;
      ent->ctime = st.ctime;
      r = ent;
    }
  } else {
    debug_errno("VFS", "sys_stat(\"%s\")", aux);
  }

  xfree(aux);

  return r;
}

static uint32_t vfs_local_seek(vfs_fpriv_t *fpriv, uint32_t pos, int fromend) {
  int whence = SEEK_SET;
  uint32_t r = -1;

  switch (fromend) {
    case -1: whence = SYS_SEEK_CUR; break;
    case  0: whence = SYS_SEEK_SET; break;
    case  1: whence = SYS_SEEK_END; break;
  }

  if (fpriv && fpriv->fd) {
    r = sys_seek(fpriv->fd, pos, whence);
  }

  return r;
}

static vfs_fpriv_t *vfs_local_open(char *path, int mode, void *_data) {
  vfsassets_mount_t *data;
  vfs_fpriv_t *fpriv;
  char *aux;
  int fd;

  data = (vfsassets_mount_t *)_data;
  debug(DEBUG_TRACE, "VFS", "vfs_local_open \"%s\" mode 0x%04X", path, mode);

  if ((aux = xcalloc(1, VFS_PATH)) == NULL) {
    return NULL;
  }

  snprintf(aux, VFS_PATH-1, "%s%s", data->local, path);
  debug(DEBUG_TRACE, "VFS", "aux \"%s\"", aux);

  // vfs mode == sys mode
  if (mode & VFS_TRUNC) {
    fd = sys_create(aux, mode, 0644);
  } else {
    fd = sys_open(aux, mode);
  }

  if (fd == -1) {
    debug_errno("VFS", "open(\"%s\", %d)", aux, mode);
    xfree(aux);
    return NULL;
  }

  xfree(aux);
  if ((fpriv = xcalloc(1, sizeof(vfs_fpriv_t))) == NULL) {
    sys_close(fd);
    return NULL;
  }

  fpriv->fd = fd;

  return fpriv;
}

static int vfs_local_rename(char *path1, char *path2, void *_data) {
  vfsassets_mount_t *data;
  char *aux1, *aux2;
  int r;

  data = (vfsassets_mount_t *)_data;

  if (vfs_local_checktype(path1, _data) == -1) {
    return -1;
  }

  if ((aux1 = xcalloc(1, VFS_PATH)) == NULL) {
    return -1;
  }

  if ((aux2 = xcalloc(1, VFS_PATH)) == NULL) {
    xfree(aux1);
    return -1;
  }

  snprintf(aux1, VFS_PATH-1, "%s%s", data->local, path1);
  snprintf(aux2, VFS_PATH-1, "%s%s", data->local, path2);

  r = sys_rename(aux1, aux2);
  xfree(aux2);
  xfree(aux1);

  return r;
}

static int vfs_local_unlink(char *path, void *_data) {
  vfsassets_mount_t *data;
  char *aux;
  int r;

  data = (vfsassets_mount_t *)_data;
  debug(DEBUG_TRACE, "VFS", "vfs_local_unlink \"%s\"", path);

  if ((aux = xcalloc(1, VFS_PATH)) == NULL) {
    return -1;
  }

  snprintf(aux, VFS_PATH-1, "%s%s", data->local, path);

  switch (vfs_local_checktype(path, _data)) {
    case VFS_FILE:
      if ((r = sys_unlink(aux)) == -1) {
        debug_errno("VFS", "unlink(\"%s\")", aux);
      }
      break;
    case VFS_DIR:
      if ((r = sys_rmdir(aux)) == -1) {
        debug_errno("VFS", "rmdir(\"%s\")", aux);
      }
      break;
    default:
      r = -1;
      break;
  }

  xfree(aux);

  return r;
}

static int vfs_local_mkdir(char *path, void *_data) {
  vfsassets_mount_t *data;
  char *aux;
  int r;

  data = (vfsassets_mount_t *)_data;
  debug(DEBUG_TRACE, "VFS", "vfs_local_mkdir \"%s\"", path);

  if ((aux = xcalloc(1, VFS_PATH)) == NULL) {
    return -1;
  }

  snprintf(aux, VFS_PATH-1, "%s%s", data->local, path);
  r = sys_mkdir(aux);
  xfree(aux);

  return r;
}

static int vfs_local_statfs(char *path, uint64_t *total, uint64_t *free, void *_data) {
  vfsassets_mount_t *data;
  sys_statfs_t sb;
  char *aux;
  int r;

  data = (vfsassets_mount_t *)_data;
  debug(DEBUG_TRACE, "VFS", "vfs_local_statfs \"%s\"", path);

  if ((aux = xcalloc(1, VFS_PATH)) == NULL) {
    return -1;
  }

  snprintf(aux, VFS_PATH-1, "%s%s", data->local, path);
  r = sys_statfs(aux, &sb);
  xfree(aux);

  if (r == 0) {
    *total = sb.total;
    *free = sb.free;
  }

  return r;
}

static void *vfs_local_loadlib(char *path, int *first_load, void *_data) {
  vfsassets_mount_t *data;
  char *aux;
  void *lib = NULL;

  data = (vfsassets_mount_t *)_data;
  debug(DEBUG_TRACE, "VFS", "vfs_local_loadlib \"%s\"", path);

  if ((aux = xcalloc(1, VFS_PATH)) == NULL) {
    return NULL;
  }

  snprintf(aux, VFS_PATH-1, "%s%s", data->local, path);
  if (vfs_local_checktype(path, _data) == VFS_FILE) {
    lib = sys_lib_load(aux, first_load);
  }
  xfree(aux);

  return lib;
}
