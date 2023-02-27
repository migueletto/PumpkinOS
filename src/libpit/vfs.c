#include "sys.h"
#include "script.h"
#include "vfs.h"
#include "mutex.h"
#include "xalloc.h"
#include "debug.h"

#define MAX_MOUNTS  32
#define MAX_BUF     1024

typedef struct {
  char *path;
  void *data;
  vfs_callback_t callback;
  int raw;
} vfs_mount_t;

struct vfs_session_t {
  char cwd[VFS_PATH];
};

struct vfs_dir_t {
  int type; // VFS_FILE or VFS_DIR
  vfs_priv_t *priv;
  vfs_ent_t *(*readdir)(vfs_priv_t *priv);
  int (*closedir)(vfs_priv_t *priv);
};

struct vfs_file_t {
  int type; // VFS_FILE or VFS_DIR
  vfs_fpriv_t *fpriv;
  int (*peek)(struct vfs_fpriv_t *f, uint32_t us);
  int (*read)(struct vfs_fpriv_t *f, uint8_t *buf, uint32_t len);
  int (*write)(struct vfs_fpriv_t *f, uint8_t *buf, uint32_t len);
  int (*close)(struct vfs_fpriv_t *f);
  uint32_t (*seek)(vfs_fpriv_t *f, uint32_t pos, int fromend);
  vfs_ent_t *(*fstat)(vfs_fpriv_t *fpriv);
  char buf[MAX_BUF];
};

static mutex_t *mutex;
static vfs_mount_t mounts[MAX_MOUNTS];
static int nmounts;

int vfs_init(void) {
  mutex = mutex_create("VFS");
  xmemset(mounts, 0, sizeof(mounts));
  nmounts = 0;

  return 0;
}

int vfs_refresh(void) {
  int i, r = -1;

  if (mutex_lock(mutex) == 0) {
    for (i = 0; i < nmounts; i++) {
      if (mounts[i].callback.refresh) mounts[i].callback.refresh(mounts[i].data);
    }
    mutex_unlock(mutex);
    r = 0;
  }

  return r;
}

int vfs_finish(void) {
  int i;

  for (i = 0; i < nmounts; i++) {
    if (mounts[i].callback.unmap) mounts[i].callback.unmap(mounts[i].data);
    if (mounts[i].path) xfree(mounts[i].path);
  }
  mutex_destroy(mutex);

  return 0;
}

vfs_session_t *vfs_open_session(void) {
  vfs_session_t *session;

  if ((session = xcalloc(1, sizeof(vfs_session_t))) != NULL) {
    session->cwd[0] = '/';
  }

  return session;
}

int vfs_close_session(vfs_session_t *session) {
  int r = -1;

  if (session) {
    xfree(session);
    r = 0;
  }

  return r;
}

static int vfs_backup_level(char *abspath, int j) {
  if (j <= 1) return -1;

  if (abspath[j-1] == '/') {
    abspath[--j] = 0;
    //debug(DEBUG_TRACE, "VFS", "searching slash abs=\"%s\"", abspath);
    for (j--; j > 0; j--) {
      if (abspath[j-1] == '/') {
        //debug(DEBUG_TRACE, "VFS", "found slash at j=%d", j-1);
        break;
      }
    }

    abspath[j] = 0;
    debug(DEBUG_TRACE, "VFS", "backup abs=\"%s\"", abspath);
  }

  return j;
}

char *vfs_abspath(char *cwd, char *relpath) {
  char *abspath;
  int s, i, j;

  if (!relpath || !relpath[0]) {
    debug(DEBUG_ERROR, "VFS", "invalid relpath");
    return NULL;
  }

  if ((abspath = xcalloc(1, VFS_PATH)) == NULL) {
    return NULL;
  }

  i = j = 0;
  if (relpath[i] != '/') {
    sys_strncpy(abspath, cwd, VFS_PATH-1);
    j = sys_strlen(abspath);
  }

  for (s = 0; relpath[i] && j < VFS_PATH-1; i++) {
    //debug(DEBUG_TRACE, "VFS", "c=%c abs=\"%s\"", relpath[i], abspath);
    switch (s) {
      case 0:
        if (relpath[i] == '.') {
          //debug(DEBUG_TRACE, "VFS", "1st dot");
          s = 1;
        } else {
          abspath[j++] = relpath[i];
        }
        break;
      case 1:
        switch (relpath[i]) {
          case '.':
            //debug(DEBUG_TRACE, "VFS", "2nd dot");
            if (relpath[i+1] == 0) {
              //debug(DEBUG_TRACE, "VFS", "dot dot end");
              j = vfs_backup_level(abspath, j);
              if (j == -1) {
                debug(DEBUG_ERROR, "VFS", "attempt to backup past root \"%s\"", relpath);
                xfree(abspath);
                return NULL;
              }
            } else {
              s = 2;
            }
            break;
          case '/':
            //debug(DEBUG_TRACE, "VFS", "dot slash");
            s = 0;
            break;
          default:
            abspath[j++] = '.';
            abspath[j++] = relpath[i];
            s = 0;
            break;
        }
        break;
      case 2:
        if (relpath[i] == '/') {
          //debug(DEBUG_TRACE, "VFS", "dot dot slash");
          j = vfs_backup_level(abspath, j);
          if (j == -1) {
            debug(DEBUG_ERROR, "VFS", "attempt to backup past root \"%s\"", relpath);
            xfree(abspath);
            return NULL;
          }
          s = 0;
        } else {
          abspath[j++] = '.';
          abspath[j++] = '.';
          abspath[j++] = relpath[i];
        }
        break;
    }
  }
  abspath[j] = 0;

  return abspath;
}

static vfs_mount_t *vfs_find(char *path, int *pos) {
  int i, len, imax, max, r;

  if (!path || !path[0]) {
    return NULL;
  }

  for (i = 0, imax = -1, max = 0; i < nmounts; i++) {
    len = sys_strlen(mounts[i].path);
    r = sys_strncmp(mounts[i].path, path, len);
    if (r == 0) {
      if (len > max) {
        imax = i;
        max = len;
      }
    }
  }

  if (imax != -1) {
    *pos = max;
    return &mounts[imax];
  }

  return NULL;
}

int vfs_map(char *label, char *path, void *data, vfs_callback_t *callback, int raw) {
  if (!path || path[0] != '/' || !callback) {
    debug(DEBUG_ERROR, "VFS", "invalid map arguments");
    return -1;
  }

  if (mutex_lock(mutex) == -1) {
    return -1;
  }

  if (nmounts == MAX_MOUNTS) {
    mutex_unlock(mutex);
    debug(DEBUG_ERROR, "VFS", "max number of mounts reached");
    return -1;
  }

  mounts[nmounts].path = xstrdup(path);
  if (!mounts[nmounts].path) {
    mutex_unlock(mutex);
    return -1;
  }
  mounts[nmounts].raw = raw;

  debug(DEBUG_INFO, "VFS", "mapped \"%s\" to %s", path, label);
  xmemcpy(&mounts[nmounts].callback, callback, sizeof(vfs_callback_t));
  mounts[nmounts].data = data;
  nmounts++;
  mutex_unlock(mutex);

  return 0;
}

char *vfs_getmount(vfs_session_t *session, char *path) {
  vfs_mount_t *mount;
  char *abspath, *s = NULL;
  int pos;

  if ((abspath = vfs_abspath(session->cwd, path)) == NULL) {
    return NULL;
  }

  if (mutex_lock(mutex) == 0) {
    if ((mount = vfs_find(abspath, &pos)) != NULL) {
      s = mount->callback.getmount ? mount->callback.getmount(mount->data) : NULL;
    }
    mutex_unlock(mutex);
  }

  return s;
}

int vfs_checktype(vfs_session_t *session, char *path) {
  vfs_mount_t *mount;
  char *abspath;
  int pos, r;

  if ((abspath = vfs_abspath(session->cwd, path)) == NULL) {
    return -1;
  }

  if (mutex_lock(mutex) == -1) {
    xfree(abspath);
    return -1;
  }

  if ((mount = vfs_find(abspath, &pos)) == NULL) {
    debug(DEBUG_ERROR, "VFS", "unmapped path \"%s\"", abspath);
    mutex_unlock(mutex);
    xfree(abspath);
    return -1;
  }

  r = mount->callback.checktype ? mount->callback.checktype(&abspath[pos], mount->data) : -1;
  xfree(abspath);
  mutex_unlock(mutex);

  return r;
}

int vfs_statfs(vfs_session_t *session, char *path, uint64_t *total, uint64_t *free) {
  vfs_mount_t *mount;
  char *abspath;
  int pos, r;

  if ((abspath = vfs_abspath(session->cwd, path)) == NULL) {
    return -1;
  }

  if (mutex_lock(mutex) == -1) {
    xfree(abspath);
    return -1;
  }

  if ((mount = vfs_find(abspath, &pos)) == NULL) {
    debug(DEBUG_ERROR, "VFS", "unmapped path \"%s\"", abspath);
    mutex_unlock(mutex);
    xfree(abspath);
    return -1;
  }

  r = mount->callback.statfs ? mount->callback.statfs(&abspath[pos], total, free, mount->data) : -1;
  mutex_unlock(mutex);
  xfree(abspath);

  return r;
}

int vfs_chdir(vfs_session_t *session, char *path) {
  vfs_mount_t *mount;
  char *abspath;
  int pos, n;

  if ((abspath = vfs_abspath(session->cwd, path)) == NULL) {
    return -1;
  }

  if (mutex_lock(mutex) == -1) {
    xfree(abspath);
    return -1;
  }

  if ((mount = vfs_find(abspath, &pos)) == NULL) {
    debug(DEBUG_ERROR, "VFS", "unmapped path \"%s\"", abspath);
    mutex_unlock(mutex);
    xfree(abspath);
    return -1;
  }

  if ((mount->callback.checktype ? mount->callback.checktype(&abspath[pos], mount->data) : -1) != VFS_DIR) {
    mutex_unlock(mutex);
    debug(DEBUG_ERROR, "VFS", "\"%s\" is not a directory", abspath);
    xfree(abspath);
    return -1;
  }
  mutex_unlock(mutex);

  sys_strncpy(session->cwd, abspath, VFS_PATH-2);
  n = sys_strlen(session->cwd);
  if (n && session->cwd[n-1] != '/') {
    session->cwd[n] = '/';
    session->cwd[n+1] = 0;
  }

  xfree(abspath);
  debug(DEBUG_INFO, "VFS", "current directory \"%s\"", session->cwd);

  return 0;
}

int vfs_mkdir(vfs_session_t *session, char *path) {
  vfs_mount_t *mount;
  char *abspath;
  int pos, r;

  if ((abspath = vfs_abspath(session->cwd, path)) == NULL) {
    return -1;
  }

  if (mutex_lock(mutex) == -1) {
    xfree(abspath);
    return -1;
  }

  if ((mount = vfs_find(abspath, &pos)) == NULL) {
    debug(DEBUG_ERROR, "VFS", "unmapped path \"%s\"", abspath);
    mutex_unlock(mutex);
    xfree(abspath);
    return -1;
  }

  r = mount->callback.mkdir ? mount->callback.mkdir(&abspath[pos], mount->data) : -1;
  mutex_unlock(mutex);
  xfree(abspath);

  return r;
}

char *vfs_cwd(vfs_session_t *session) {
  return session->cwd;
}

int vfs_type(void *p) {
  vfs_dir_t *vdir;
  int r = -1;

  if (p) {
    vdir = (vfs_dir_t *)p;
    r = vdir->type;
  }

  return r;
}

vfs_dir_t *vfs_opendir(vfs_session_t *session, char *path) {
  vfs_mount_t *mount;
  vfs_dir_t *vdir;
  char *abspath;
  int pos;

  debug(DEBUG_TRACE, "VFS", "vfs_opendir \"%s\"", path);
  if ((abspath = vfs_abspath(session->cwd, path)) == NULL) {
    return NULL;
  }
  debug(DEBUG_TRACE, "VFS", "abspath \"%s\"", abspath);

  if (mutex_lock(mutex) == -1) {
    xfree(abspath);
    return NULL;
  }

  if ((mount = vfs_find(abspath, &pos)) == NULL) {
    debug(DEBUG_ERROR, "VFS", "unmapped path \"%s\"", abspath);
    mutex_unlock(mutex);
    xfree(abspath);
    return NULL;
  }

  if ((vdir = xcalloc(1, sizeof(vfs_dir_t))) == NULL) {
    mutex_unlock(mutex);
    xfree(abspath);
    return NULL;
  }

  if ((vdir->priv = (mount->callback.opendir ? mount->callback.opendir(&abspath[pos], mount->data) : NULL)) == NULL) {
    mutex_unlock(mutex);
    xfree(abspath);
    xfree(vdir);
    return NULL;
  }

  vdir->type = VFS_DIR;
  vdir->readdir = mount->callback.readdir;
  vdir->closedir = mount->callback.closedir;
  mutex_unlock(mutex);

  return vdir;
}

vfs_ent_t *vfs_readdir(vfs_dir_t *dir) {
  if (dir) {
    return dir->readdir ? dir->readdir(dir->priv) : NULL;
  }
  return NULL;
}

int vfs_closedir(vfs_dir_t *dir) {
  if (dir) {
    return dir->closedir ? dir->closedir(dir->priv) : -1;
  }
  return -1;
}

vfs_file_t *vfs_open_special(vfs_fpriv_t *fpriv,
  int (*peek)(struct vfs_fpriv_t *f, uint32_t us),
  int (*read)(struct vfs_fpriv_t *f, uint8_t *buf, uint32_t len),
  int (*write)(struct vfs_fpriv_t *f, uint8_t *buf, uint32_t len)) {

  vfs_file_t *vfile;

  if ((vfile = xcalloc(1, sizeof(vfs_file_t))) != NULL) {
    vfile->type = VFS_FILE;
    vfile->fpriv = fpriv;
    vfile->peek = peek;
    vfile->read = read;
    vfile->write = write;
  }

  return vfile;
}

vfs_file_t *vfs_open(vfs_session_t *session, char *path, int mode) {
  vfs_mount_t *mount;
  vfs_file_t *vfile;
  char *abspath;
  int pos;

  debug(DEBUG_TRACE, "VFS", "vfs_open \"%s\" mode 0x%04X", path, mode);
  if ((abspath = vfs_abspath(session->cwd, path)) == NULL) {
    return NULL;
  }
  debug(DEBUG_TRACE, "VFS", "abspath \"%s\"", abspath);

  if (mutex_lock(mutex) == -1) {
    xfree(abspath);
    return NULL;
  }

  if ((mount = vfs_find(abspath, &pos)) == NULL) {
    debug(DEBUG_ERROR, "VFS", "unmapped path \"%s\"", abspath);
    mutex_unlock(mutex);
    xfree(abspath);
    return NULL;
  }
  debug(DEBUG_TRACE, "VFS", "find \"%s\" (%d)", &abspath[pos], pos);

  if ((vfile = xcalloc(1, sizeof(vfs_file_t))) == NULL) {
    mutex_unlock(mutex);
    xfree(abspath);
    return NULL;
  }

  if ((vfile->fpriv = (mount->callback.open ? mount->callback.open(&abspath[pos], mode, mount->data) : NULL)) == NULL) {
    mutex_unlock(mutex);
    xfree(abspath);
    xfree(vfile);
    return NULL;
  }

  vfile->type = VFS_FILE;
  vfile->peek  = mount->callback.peek;
  vfile->read  = mount->callback.read;
  vfile->write = mount->callback.write;
  vfile->close = mount->callback.close;
  vfile->seek  = mount->callback.seek;
  vfile->fstat = mount->callback.fstat;
  mutex_unlock(mutex);

  return vfile;
}

int vfs_peek(vfs_file_t *f, uint32_t us) {
  if (f) {
    return f->peek ? f->peek(f->fpriv, us) : -1;
  }
  return -1;
}

int vfs_read(vfs_file_t *f, uint8_t *buf, uint32_t len) {
  if (f) {
    return f->read ? f->read(f->fpriv, buf, len) : -1;
  }
  return -1;
}

int vfs_write(vfs_file_t *f, uint8_t *buf, uint32_t len) {
  if (f) {
    return f->write ? f->write(f->fpriv, buf, len) : -1;
  }
  return -1;
}

// getc() reads the next character from stream and returns it as an unsigned char cast to an int, or EOF on end of file or error

int vfs_getc(vfs_file_t *f) {
  uint8_t b;
  int r = SYS_EOF;

  if (vfs_read(f, &b, 1) == 1) {
    r = b;
  }

  return r;
}

// fgets()  reads  in  at  most one less than size characters from stream and stores them into the buffer pointed to by s.
// Reading stops after an EOF or a newline. If a newline is read, it is stored into the buffer.
// A terminating null byte ('\0') is stored after the last character in the buffer.
// fgets() returns s on success, and NULL on error or when end of file occurs while no characters have been read.

char *vfs_gets(vfs_file_t *f, char *s, uint32_t len) {
  int c, i;

  for (i = 0; i < len-1;) {
    c = vfs_getc(f);
    if (c == SYS_EOF) break;
    s[i++] = c;
    if (c == '\n') break;
  }
  if (i) s[i] = 0;

  return i ? s : NULL;
}

int vfs_putc(vfs_file_t *f, int c) {
  uint8_t b;
  int r = SYS_EOF;

  b = c;
  if (vfs_write(f, &b, 1) == 1) {
    r = b;
  }

  return r;
}

int vfs_printf(vfs_file_t *f, char *fmt, ...) {
  sys_va_list ap;
  int r;

  xmemset(f->buf, 0, MAX_BUF);

  sys_va_start(ap, fmt);
  sys_vsnprintf(f->buf, MAX_BUF-1, fmt, ap);
  sys_va_end(ap);

  r = sys_strlen(f->buf);
  if (r > 0) vfs_write(f, (uint8_t *)f->buf, r);

  return r;
}

int vfs_close(vfs_file_t *f) {
  if (f) {
    return f->close ? f->close(f->fpriv) : -1;
  }
  return -1;
}

uint32_t vfs_seek(vfs_file_t *f, uint32_t pos, int fromend) {
  if (f) {
    return f->seek ? f->seek(f->fpriv, pos, fromend) : -1;
  }

  return -1;
}

void vfs_rewind(vfs_file_t *f) {
  vfs_seek(f, 0, 0);
}

vfs_ent_t *vfs_fstat(vfs_file_t *f) {
  if (f) {
    return f->fstat ? f->fstat(f->fpriv) : NULL;
  }

  return NULL;
}

vfs_ent_t *vfs_stat(vfs_session_t *session, char *path, vfs_ent_t *ent) {
  vfs_mount_t *mount;
  char *abspath;
  int pos;

  if ((abspath = vfs_abspath(session->cwd, path)) == NULL) {
    return NULL;
  }

  if (mutex_lock(mutex) == -1) {
    xfree(abspath);
    return NULL;
  }

  if ((mount = vfs_find(abspath, &pos)) == NULL) {
    debug(DEBUG_ERROR, "VFS", "unmapped path \"%s\"", abspath);
    mutex_unlock(mutex);
    xfree(abspath);
    return NULL;
  }

  ent = mount->callback.stat ? mount->callback.stat(&abspath[pos], mount->data, ent) : NULL;
  mutex_unlock(mutex);
  xfree(abspath);

  return ent;
}

int vfs_rename(vfs_session_t *session, char *path1, char *path2) {
  vfs_mount_t *mount1, *mount2;
  char *abspath1, *abspath2;
  int pos1, pos2;

  if ((abspath1 = vfs_abspath(session->cwd, path1)) == NULL) {
    return -1;
  }

  if ((abspath2 = vfs_abspath(session->cwd, path2)) == NULL) {
    xfree(abspath1);
    return -1;
  }

  if (mutex_lock(mutex) == -1) {
    xfree(abspath2);
    xfree(abspath1);
    return -1;
  }

  if ((mount1 = vfs_find(abspath1, &pos1)) == NULL) {
    debug(DEBUG_ERROR, "VFS", "unmapped path \"%s\"", abspath1);
    mutex_unlock(mutex);
    xfree(abspath2);
    xfree(abspath1);
    return -1;
  }

  if ((mount2 = vfs_find(abspath2, &pos2)) == NULL) {
    debug(DEBUG_ERROR, "VFS", "unmapped path \"%s\"", abspath2);
    mutex_unlock(mutex);
    xfree(abspath2);
    xfree(abspath1);
    return -1;
  }

  if (mount1 != mount2) {
    debug(DEBUG_ERROR, "VFS", "can not rename to different fs");
    mutex_unlock(mutex);
    xfree(abspath2);
    xfree(abspath1);
    return -1;
  }

  if ((mount1->callback.rename ? mount1->callback.rename(&abspath1[pos1], &abspath2[pos2], mount1->data) : -1) == -1) {
    mutex_unlock(mutex);
    xfree(abspath2);
    xfree(abspath1);
    return -1;
  }
  mutex_unlock(mutex);

  xfree(abspath2);
  xfree(abspath1);

  return 0;
}

int vfs_unlink(vfs_session_t *session, char *path) {
  vfs_mount_t *mount;
  char *abspath;
  int pos;

  if ((abspath = vfs_abspath(session->cwd, path)) == NULL) {
    return -1;
  }

  if (mutex_lock(mutex) == -1) {
    xfree(abspath);
    return -1;
  }

  if ((mount = vfs_find(abspath, &pos)) == NULL) {
    debug(DEBUG_ERROR, "VFS", "unmapped path \"%s\"", abspath);
    mutex_unlock(mutex);
    xfree(abspath);
    return -1;
  }

  if ((mount->callback.unlink ? mount->callback.unlink(&abspath[pos], mount->data) : -1) == -1) {
    mutex_unlock(mutex);
    xfree(abspath);
    return -1;
  }
  mutex_unlock(mutex);

  xfree(abspath);
  return 0;
}

void *vfs_loadlib(vfs_session_t *session, char *path, int *first_load) {
  vfs_mount_t *mount;
  char *abspath;
  int pos;
  void *lib;

  if ((abspath = vfs_abspath(session->cwd, path)) == NULL) {
    return NULL;
  }

  if (mutex_lock(mutex) == -1) {
    xfree(abspath);
    return NULL;
  }

  if ((mount = vfs_find(abspath, &pos)) == NULL) {
    debug(DEBUG_ERROR, "VFS", "unmapped path \"%s\"", abspath);
    mutex_unlock(mutex);
    xfree(abspath);
    return NULL;
  }

  if ((lib = (mount->callback.loadlib ? mount->callback.loadlib(&abspath[pos], first_load, mount->data) : NULL)) == NULL) {
    mutex_unlock(mutex);
    xfree(abspath);
    return NULL;
  }
  mutex_unlock(mutex);

  xfree(abspath);
  return lib;
}
