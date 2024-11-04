#include <PalmOS.h>
#include <VFSMgr.h>

#include "pumpkin.h"
#include "plibc.h"

struct PLIBC_FILE {
  int fd;
  char unget;
};

#define MAX_FDS  256
#define FD_FILE  1

typedef struct {
  FileRef fileRef;
  int stdin_error;
} fd_t;

static uint32_t dummy0 = 0;
static uint32_t dummy1 = 0;
static uint32_t dummy2 = 0;

static const FileRef stdinFileRef  = &dummy0;
static const FileRef stdoutFileRef = &dummy1;
static const FileRef stderrFileRef = &dummy2;

PLIBC_FILE *plibc_stdin = NULL;
PLIBC_FILE *plibc_stdout = NULL;
PLIBC_FILE *plibc_stderr = NULL;

int plibc_init(void) {
  fd_t **table = pumpkin_gettable(MAX_FDS);

  plibc_finish();

  table[0] = sys_calloc(1, sizeof(fd_t));
  table[0]->fileRef = stdinFileRef;
  plibc_stdin = sys_calloc(1, sizeof(PLIBC_FILE));
  plibc_stdin->fd = 0;

  table[1] = sys_calloc(1, sizeof(fd_t));
  table[1]->fileRef = stdoutFileRef;
  plibc_stdout = sys_calloc(1, sizeof(PLIBC_FILE));
  plibc_stdout->fd = 1;

  table[2] = sys_calloc(1, sizeof(fd_t));
  table[2]->fileRef = stderrFileRef;
  plibc_stderr = sys_calloc(1, sizeof(PLIBC_FILE));
  plibc_stderr->fd = 2;

  return 0;
}

int plibc_finish(void) {
  fd_t **table = pumpkin_gettable(MAX_FDS);
  int fd;

  for (fd = 0; fd < MAX_FDS; fd++) {
    if (table[fd]) {
      sys_free(table[fd]);
      table[fd] = NULL;
    }
  }

  sys_free(plibc_stdin);
  sys_free(plibc_stdout);
  sys_free(plibc_stderr);

  return 0;
}

int plibc_setfd(int fd, void *fileRef) {
  fd_t **table = pumpkin_gettable(MAX_FDS);
  int r = -1;

  switch (fd) {
    case 0:
      table[0]->fileRef = fileRef ? (FileRef)fileRef : stdinFileRef;
      table[0]->stdin_error = 0;
      r = 0;
      break;
    case 1:
      table[1]->fileRef = fileRef ? (FileRef)fileRef : stdoutFileRef;
      r = 0;
      break;
    case 2:
      table[2]->fileRef = fileRef ? (FileRef)fileRef : stderrFileRef;
      r = 0;
      break;
  }

  return r;
}

int plibc_isatty(int fd) {
  fd_t **table = pumpkin_gettable(MAX_FDS);
  int r = 0;

  if (fd >= 0 && fd < MAX_FDS && table[fd]) {
    r = table[fd]->fileRef == stdinFileRef || table[fd]->fileRef == stdoutFileRef || table[fd]->fileRef == stderrFileRef;
  }

  return r;
}

int plibc_mkdir(int vol, const char *pathname) {
  return VFSDirCreate(vol, pathname) == errNone ? 0 : -1;
}

int plibc_chdir(int vol, const char *pathname) {
  return VFSChangeDir(vol, (char *)pathname) == errNone ? 0 : -1;
}

int plibc_getdir(int vol, char *pathname, int max) {
  return VFSCurrentDir(vol, pathname, max) == errNone ? 0 : -1;
}

int plibc_open(int vol, const char *pathname, int flags) {
  fd_t **table = pumpkin_gettable(MAX_FDS);
  FileOrigin origin;
  FileRef fileRef;
  UInt16 openMode;
  fd_t *f;
  Err err;
  int fd;

  switch (flags & PLIBC_ACCMODE) {
    case PLIBC_RDONLY: openMode = vfsModeRead; break;
    case PLIBC_WRONLY: openMode = vfsModeWrite; break;
    case PLIBC_RDWR:   openMode = vfsModeRead | vfsModeWrite; break;
    default:
      return -1;
  }

  for (fd = 0; fd < MAX_FDS; fd++) {
    if (table[fd] == NULL) break;
  }

  if (fd < MAX_FDS) {
    if (flags & PLIBC_CREAT) {
      if (VFSFileOpen(vol, pathname, vfsModeRead, &fileRef) != errNone) {
        VFSFileCreate(vol, pathname) == errNone ? 0 : -1;
      } else {
        VFSFileClose(fileRef);
      }
    }

    if (flags & PLIBC_TRUNC) {
    }

    if ((openMode & vfsModeWrite) && (flags & PLIBC_APPEND)) {
      origin = vfsOriginEnd;
    } else {
      origin = vfsOriginBeginning;
    }

    if (VFSFileOpen(vol, pathname, openMode, &fileRef) == errNone) {
      if ((err = VFSFileSeek(fileRef, origin, 0)) == errNone || err == vfsErrFileEOF) {
        if ((f = sys_calloc(1, sizeof(fd_t))) != NULL) {
          f->fileRef = fileRef;
          table[fd] = f;
        } else {
          VFSFileClose(fileRef);
          fd = -1;
        }
      } else {
        VFSFileClose(fileRef);
        fd = -1;
      }
    } else {
      fd = -1;
    }
  } else {
    fd = -1;
  }

  return fd;
}

int plibc_close(int fd) {
  fd_t **table = pumpkin_gettable(MAX_FDS);
  int i, r = -1;

  if (fd >= 0 && fd < MAX_FDS && table[fd]) {
    for (i = 0; i < MAX_FDS; i++) {
      if (i != fd && table[i] && table[i]->fileRef == table[fd]->fileRef) break;
    }
    if (i == MAX_FDS) {
      // it is the only fd pointing to this fileRef
      if (table[fd]->fileRef != stdinFileRef && table[fd]->fileRef != stdoutFileRef && table[fd]->fileRef != stderrFileRef) {
        VFSFileClose(table[fd]->fileRef);
      }
    }
    sys_free(table[fd]);
    table[fd] = NULL;
    r = 0;
  }

  return r;
}

sys_ssize_t plibc_read(int fd, void *buf, sys_size_t count) {
  fd_t **table = pumpkin_gettable(MAX_FDS);
  sys_size_t i;
  UInt32 numBytesRead;
  char *s;
  int c;
  sys_ssize_t r = -1;

  if (fd >= 0 && fd < MAX_FDS && buf && table[fd] && table[fd]->fileRef) {
    if (table[fd]->fileRef == stdinFileRef) {
       s = (char *)buf;
       for (i = 0; i < count; i++) {
         c = pumpkin_getchar();
         if (c == -1) {
           table[fd]->stdin_error = 1;
           break;
         } 
         table[fd]->stdin_error = 0;
         s[i] = c;
       }
       r = i;
    } else {
      if (VFSFileRead(table[fd]->fileRef, count, buf, &numBytesRead) == errNone) {
        r = numBytesRead;
      }
    }
  }

  return r;
}

sys_ssize_t plibc_write(int fd, void *buf, sys_size_t count) {
  fd_t **table = pumpkin_gettable(MAX_FDS);
  UInt32 numBytesWritten;
  sys_ssize_t r = -1;
  
  if (fd >= 0 && fd < MAX_FDS && buf && table[fd] && table[fd]->fileRef) {
    if (table[fd]->fileRef == stdoutFileRef || table[fd]->fileRef == stderrFileRef) {
       pumpkin_write((char *)buf, count);
       r = count;
    } else { 
      if (VFSFileWrite(table[fd]->fileRef, count, buf, &numBytesWritten) == errNone) {
        r = numBytesWritten; 
      }
    }
  }

  return r;
}

sys_ssize_t plibc_lseek(int fd, sys_ssize_t offset, int whence) {
  fd_t **table = pumpkin_gettable(MAX_FDS);
  FileOrigin origin;
  UInt32 pos;
  sys_ssize_t r = -1;

  if (fd >= 0 && fd < MAX_FDS && table[fd] && table[fd]->fileRef) {
    switch (whence) {
      case PLIBC_SEEK_SET: origin = vfsOriginBeginning; break;
      case PLIBC_SEEK_CUR: origin = vfsOriginCurrent; break;
      case PLIBC_SEEK_END: origin = vfsOriginEnd; break;
      default: return -1;
    }
  
    if (VFSFileSeek(table[fd]->fileRef, origin, offset) == errNone) {
      if (VFSFileTell(table[fd]->fileRef, &pos) == errNone) {
        r = pos;
      }
    }
  }

  return r;
}

int plibc_dup(int oldfd) {
  fd_t **table = pumpkin_gettable(MAX_FDS);
  int fd = -1;

  if (oldfd >= 0 && oldfd < MAX_FDS && table[oldfd]) {
    for (fd = 0; fd < MAX_FDS; fd++) {
      if (table[fd] == NULL) break;
    }

    if (fd < MAX_FDS) {
      if ((table[fd] = sys_calloc(1, sizeof(fd_t))) != NULL) {
        table[fd]->fileRef = table[oldfd]->fileRef;
      } else {
        fd = -1;
      }
    } else {
      fd = -1;
    }
  }

  return fd;
}

int plibc_dup2(int oldfd, int newfd) {
  fd_t **table = pumpkin_gettable(MAX_FDS);

  if (oldfd >= 0 && oldfd < MAX_FDS && newfd >= 0 && newfd < MAX_FDS && table[oldfd]) {
    plibc_close(newfd);

    if ((table[newfd] = sys_calloc(1, sizeof(fd_t))) != NULL) {
      table[newfd]->fileRef = table[oldfd]->fileRef;
    } else {
      newfd = -1;
    }
  } else {
    newfd = -1;
  }

  return newfd;
}

PLIBC_FILE *plibc_fdopen(int fd, const char *mode) {
  fd_t **table = pumpkin_gettable(MAX_FDS);
  PLIBC_FILE *stream = NULL;

  // XXX mode is ignored

  if (fd >= 0 && fd < MAX_FDS && table[fd] && table[fd]->fileRef) {
    if ((stream = sys_calloc(1, sizeof(PLIBC_FILE))) != NULL) {
      stream->fd = fd;
    }
  }

  return stream;
}

int plibc_fileno(PLIBC_FILE *stream) {
  int fd = -1;

  if (stream) {
    fd = stream->fd;
  }

  return fd;
}

PLIBC_FILE *plibc_fopen(int vol, const char *pathname, const char *mode) {
  PLIBC_FILE *stream = NULL;
  int flags;

  if (pathname && mode) {
    switch (mode[0]) {
      case 'r':
        flags = (mode[1] == '+') ? PLIBC_RDWR : PLIBC_RDONLY;
        break;
      case 'w':
        flags = (mode[1] == '+') ? PLIBC_RDWR : PLIBC_WRONLY;
        flags |= PLIBC_CREAT | PLIBC_TRUNC;
        break;
      case 'a':
        flags = (mode[1] == '+') ? PLIBC_RDWR : PLIBC_WRONLY;
        flags |= PLIBC_CREAT | PLIBC_APPEND;
        break;
      default:
        return NULL;
    }

    if ((stream = sys_calloc(1, sizeof(PLIBC_FILE))) != NULL) {
      if ((stream->fd = plibc_open(vol, pathname, flags)) == -1) {
        sys_free(stream);
        stream = NULL;
      }
    }
  }

  return stream;
}

int plibc_fclose(PLIBC_FILE *stream) {
  int r = -1;

  if (stream) {
    r = plibc_close(stream->fd);
    sys_free(stream);
  }

  return r;
}

char *plibc_tmpnam(void) {
  return VFSTmpName();
}

int plibc_rename(int vol, const char *oldpath, const char *newpath) {
  int r = -1;

  if (oldpath && newpath) {
    r = VFSFileRename(vol, oldpath, newpath) == errNone;
  }

  return r;
}

int plibc_remove(int vol, const char *pathname) {
  int r = -1;

  if (pathname) {
    r = VFSFileDelete(vol, pathname) == errNone;
  }

  return r;
}

sys_size_t plibc_fread(void *ptr, sys_size_t size, sys_size_t nmemb, PLIBC_FILE *stream) {
  sys_ssize_t n;
  sys_size_t r = 0;

  if (ptr && stream) {
    if ((n = plibc_read(stream->fd, ptr, size * nmemb)) != -1) {
      r = n / size;
    }
  }

  return r;
}

sys_size_t plibc_fwrite(const void *ptr, sys_size_t size, sys_size_t nmemb, PLIBC_FILE *stream) {
  sys_ssize_t n;
  sys_size_t r = 0;
  
  if (ptr && stream) { 
    if ((n = plibc_write(stream->fd, (void *)ptr, size * nmemb)) != -1) {
      r = n / size;
    } 
  }

  return r; 
}

int plibc_fseek(PLIBC_FILE *stream, long offset, int whence) {
  int r = -1;

  if (stream) {
    r = plibc_lseek(stream->fd, offset, whence) == -1 ? -1 : 0;
  }

  return r;
}

int plibc_ftruncate(PLIBC_FILE *stream, long offset) {
  fd_t **table = pumpkin_gettable(MAX_FDS);
  int r = -1;

  if (stream && table[stream->fd] &&
      table[stream->fd]->fileRef != stdinFileRef && table[stream->fd]->fileRef != stdoutFileRef && table[stream->fd]->fileRef != stderrFileRef) {
    if (VFSFileTruncate(table[stream->fd]->fileRef, offset) == errNone) {
      r = 0;
    }
  }

  return r;
}

long plibc_ftell(PLIBC_FILE *stream) {
  fd_t **table = pumpkin_gettable(MAX_FDS);
  UInt32 pos;
  long r = -1;

  if (stream && table[stream->fd] &&
      table[stream->fd]->fileRef != stdinFileRef && table[stream->fd]->fileRef != stdoutFileRef && table[stream->fd]->fileRef != stderrFileRef) {
    if (VFSFileTell(table[stream->fd]->fileRef, &pos) == errNone) {
      r = pos;
    }
  }

  return r;
}

int plibc_feof(PLIBC_FILE *stream) {
  fd_t **table = pumpkin_gettable(MAX_FDS);
  int r = 1;

  if (stream && table[stream->fd]) {
    if (table[stream->fd]->fileRef == stdinFileRef) {
      r = table[stream->fd]->stdin_error;
    } else if (table[stream->fd]->fileRef != stdoutFileRef && table[stream->fd]->fileRef != stderrFileRef) {
      r = VFSFileEOF(table[stream->fd]->fileRef) == vfsErrFileEOF;
    }
  }

  return r;
}

int plibc_fflush(PLIBC_FILE *stream) {
  return 0;
}

int plibc_fputc(int c, PLIBC_FILE *stream) {
  uint8_t b;
  int r = PLIBC_EOF;

  if (stream) {
    b = (uint8_t)c;
    if (plibc_fwrite(&b, 1, 1, stream) == 1) {
      r = (int)b;
    }
  }

  return r;
}

int plibc_fputs(const char *s, PLIBC_FILE *stream) {
  int len, r = PLIBC_EOF;

  if (s && stream) {
    len = sys_strlen(s);
    if (plibc_fwrite(s, 1, len, stream) == len) {
      r = 0;
    }
  }

  return r;
}

int plibc_ungetc(int c, PLIBC_FILE *stream) {
  int r = PLIBC_EOF;

  if (stream) {
    stream->unget = c;
    r = c;
  }

  return r;
}

int plibc_fgetc(PLIBC_FILE *stream) {
  int c = PLIBC_EOF;
  char ch;

  if (stream) {
    if (stream->unget) {
      c = stream->unget;
      stream->unget = 0;
    } else {
      if (plibc_fread(&ch, 1, 1, stream) == 1) {
        c = ch;
      }
    }
  }

  return c;
}

char *plibc_fgets(char *s, int size, PLIBC_FILE *stream) {
  fd_t **table = pumpkin_gettable(MAX_FDS);
  char *r = NULL;

  if (s && size > 0 && stream && table[stream->fd]) {
    if (table[stream->fd]->fileRef == stdinFileRef) {
      pumpkin_gets(s, size, 1);
      r = s;
    } else {
      r = VFSFileGets(table[stream->fd]->fileRef, size, s);
    }
  }

  return r;
}

int plibc_fprintf(PLIBC_FILE *stream, const char *format, ...) {
 sys_va_list ap;
  int r;

  sys_va_start(ap, format);
  r = plibc_vfprintf(stream, format, ap);
  sys_va_end(ap);

  return r;
}

int plibc_printf(const char *format, ...) {
  sys_va_list ap;
  int r = 0;

  if (format) {
    sys_va_start(ap, format);
    r = plibc_vprintf(format, ap);
    sys_va_end(ap);
  }

  return r;
}

int plibc_vfprintf(PLIBC_FILE *stream, const char *format, sys_va_list ap) {
  fd_t **table = pumpkin_gettable(MAX_FDS);
  int r = 0;

  if (stream && format && table[stream->fd]) {
    if (table[stream->fd]->fileRef == stdoutFileRef || table[stream->fd]->fileRef == stderrFileRef) {
      r = pumpkin_vprintf(format, ap);
    } else {
      r = VFSFileVPrintF(table[stream->fd]->fileRef, format, ap);
    }
  }

  return r;
}

int plibc_vprintf(const char *format, sys_va_list ap) {
  int r = 0;

  if (format) {
    r = pumpkin_vprintf(format, ap);
  }

  return r;
}

int plibc_haschar(void) {
  return pumpkin_haschar();
}

int plibc_errno(void) {
  return pumpkin_get_lasterr();
}

const char *plibc_strerror(int err) {
  return pumpkin_error_msg(err);
}

void plibc_error(const char *filename, const char *function, int line, char *msg) {
  ErrDisplayFileLineMsgEx(filename, function, line, msg, 0);
}
