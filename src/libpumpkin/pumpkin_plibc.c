#include <PalmOS.h>
#include <VFSMgr.h>

#include "pumpkin.h"
#include "plibc.h"

struct PLIBC_FILE {
  FileRef fileRef;
  char unget;
  int stdin_error;
};

static PLIBC_FILE plibc_fstdin  = { NULL, 0 };
static PLIBC_FILE plibc_fstdout = { NULL, 0 };
static PLIBC_FILE plibc_fstderr = { NULL, 0 };

PLIBC_FILE *plibc_stdin  = &plibc_fstdin;
PLIBC_FILE *plibc_stdout = &plibc_fstdout;
PLIBC_FILE *plibc_stderr = &plibc_fstderr;

PLIBC_FILE *plibc_fopen(const char *pathname, const char *mode) {
  PLIBC_FILE *stream = NULL;
  FileOrigin origin;
  UInt16 openMode;
  FileRef fileRef;
  Err err;
  int r = -1;

  if (pathname && mode) {
    switch (mode[0]) {
      case 'r':
        origin = vfsOriginBeginning;
        openMode = vfsModeRead;
        if (mode[1] == '+') openMode |= vfsModeWrite;
        r = 0;
        break;
      case 'w':
        openMode = 0;
        origin = vfsOriginBeginning;
        openMode = vfsModeWrite;
        if (mode[1] == '+') {
          openMode |= vfsModeRead;
        }
        r = 0;
        break;
      case 'a':
        origin = vfsOriginEnd;
        openMode = vfsModeWrite;
        if (mode[1] == '+') openMode |= vfsModeRead;
        r = 0;
        break;
      default:
        break;
    }

    if (r == 0 && mode[0] != 'r' && (openMode & vfsModeWrite)) {
      if (VFSFileOpen(1, pathname, vfsModeRead, &fileRef) != errNone) {
        r = VFSFileCreate(1, pathname) == errNone ? 0 : -1;
      } else {
        VFSFileClose(fileRef);
      }
    }

    if (r == 0 && VFSFileOpen(1, pathname, openMode, &fileRef) == errNone) {
      if ((err = VFSFileSeek(fileRef, origin, 0)) == errNone || err == vfsErrFileEOF) {
        if ((stream = sys_calloc(1, sizeof(PLIBC_FILE))) != NULL) {
          stream->fileRef = fileRef;
        } else {
          VFSFileClose(fileRef);
        }
      } else {
        VFSFileClose(fileRef);
      }
    }
  }

  return stream;
}

int plibc_fclose(PLIBC_FILE *stream) {
  int r = -1;

  if (stream && stream != plibc_stdin && stream != plibc_stdout && stream != plibc_stderr) {
    VFSFileClose(stream->fileRef);
    sys_free(stream);
  }

  return r;
}

char *plibc_tmpnam(void) {
  return VFSTmpName();
}

int plibc_rename(const char *oldpath, const char *newpath) {
  int r = -1;

  if (oldpath && newpath) {
    r = VFSFileRename(1, oldpath, newpath) == errNone;
  }

  return r;
}

int plibc_remove(const char *pathname) {
  int r = -1;

  if (pathname) {
    r = VFSFileDelete(1, pathname) == errNone;
  }

  return r;
}

sys_size_t plibc_fread(void *ptr, sys_size_t size, sys_size_t nmemb, PLIBC_FILE *stream) {
  char *s;
  int c;
  sys_size_t i, n = 0;
  UInt32 numBytesRead;

  if (ptr && stream && stream != plibc_stdout && stream != plibc_stderr) {
    if (stream == plibc_stdin) {
       n = size * nmemb;
       s = (char *)ptr;
       for (i = 0; i < n; i++) {
         c = pumpkin_getchar();
         if (c == -1) {
           stream->stdin_error = 1;
           break;
         }
         stream->stdin_error = 0;
         s[i] = c;
       }
       n = i;
    } else if (stream->fileRef) {
      if (VFSFileRead(stream->fileRef, size * nmemb, ptr, &numBytesRead) == errNone) {
        n = numBytesRead / size;
      }
    }
  }

  return n;
}

sys_size_t plibc_fwrite(const void *ptr, sys_size_t size, sys_size_t nmemb, PLIBC_FILE *stream) {
  sys_size_t n = 0;
  UInt32 numBytesWritten;

  if (ptr && stream && stream != plibc_stdin) {
    if (stream == plibc_stdout || stream == plibc_stderr) {
      pumpkin_write((char *)ptr, size * nmemb);
      n = size * nmemb;
    } else if (stream->fileRef) {
      if (VFSFileWrite(stream->fileRef, size * nmemb, ptr, &numBytesWritten) == errNone) {
        n = numBytesWritten / size;
      }
    }
  }

  return n;
}

int plibc_fseek(PLIBC_FILE *stream, long offset, int whence) {
  FileOrigin origin;
  int r = -1;

  if (stream && stream != plibc_stdin && stream != plibc_stdout && stream != plibc_stderr && stream->fileRef) {
    switch (whence) {
      case PLIBC_SEEK_SET: origin = vfsOriginBeginning; r = 0; break;
      case PLIBC_SEEK_CUR: origin = vfsOriginCurrent; r = 0; break;
      case PLIBC_SEEK_END: origin = vfsOriginEnd; r = 0; break;
    }

    if (r == 0) {
      if (VFSFileSeek(stream->fileRef, origin, offset) != errNone) {
        r = -1;
      }
    }
  }

  return r;
}

int plibc_ftruncate(PLIBC_FILE *stream, long offset) {
  int r = -1;

  if (stream && stream != plibc_stdin && stream != plibc_stdout && stream != plibc_stderr && stream->fileRef) {
    if (VFSFileTruncate(stream->fileRef, offset) == errNone) {
      r = 0;
    }
  }

  return r;
}

long plibc_ftell(PLIBC_FILE *stream) {
  UInt32 filePos = 0;

  if (stream && stream != plibc_stdin && stream != plibc_stdout && stream != plibc_stderr && stream->fileRef) {
    VFSFileTell(stream->fileRef, &filePos);
  }

  return filePos;
}

int plibc_feof(PLIBC_FILE *stream) {
  int r = 1;

  if (stream && stream != plibc_stdout && stream != plibc_stderr) {
    if (stream == plibc_stdin) {
      r = stream->stdin_error;
    } else if (stream->fileRef) {
      r = VFSFileEOF(stream->fileRef) == vfsErrFileEOF;
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

  if (stream && stream != plibc_stdout && stream != plibc_stderr) {
    stream->unget = c;
    r = c;
  }

  return r;
}

int plibc_fgetc(PLIBC_FILE *stream) {
  int c = PLIBC_EOF;
  char ch;

  if (stream && stream != plibc_stdout && stream != plibc_stderr) {
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
  char *r = NULL;

  if (s && size > 0 && stream && stream != plibc_stdout && stream != plibc_stderr) {
    if (stream == plibc_stdin) {
      pumpkin_gets(s, size, 1);
      r = s;
    } else if (stream->fileRef) {
      r = VFSFileGets(stream->fileRef, size, s);
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
  int r = 0;

  if (stream && stream != plibc_stdin && format) {
    if (stream == plibc_stdout || stream == plibc_stderr) {
      r = pumpkin_vprintf(format, ap);
    } else {
      r = VFSFileVPrintF(stream->fileRef, format, ap);
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

int plibc_errno(void) {
  return pumpkin_get_lasterr();
}

const char *plibc_strerror(int err) {
  return pumpkin_error_msg(err);
}

void plibc_error(const char *filename, const char *function, int line, char *msg) {
  ErrDisplayFileLineMsgEx(filename, function, line, msg, 0);
}
