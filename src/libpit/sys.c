#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#ifdef WINDOWS
#include <winsock2.h>
#include <ws2ipdef.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <winnls.h>
#include <dbghelp.h>
#define _WINSOCK2API_
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#else
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#define __USE_GNU
#include <pthread.h>
#undef __USE_GNU
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <fcntl.h>
#include <termios.h>
#include <dirent.h>
#include <dlfcn.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#ifdef SERENITY
#include <sys/select.h>
#include <sys/statvfs.h>
#else
#include <sys/syscall.h>
#include <sys/vfs.h>
#endif
#include <sys/wait.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#endif

#include "sys.h"
#include "thread.h"
#include "ptr.h"
#include "debug.h"
#include "xalloc.h"

#define EN_US "en_US"

struct sys_dir_t {
#ifdef WINDOWS
  int first;
  HANDLE handle;
  WIN32_FIND_DATA ffd;
  char buf[FILE_PATH];
#else
  DIR *dir;
#endif
};

#ifndef WINDOWS
#define closesocket(s) close(s)
#endif

void sys_init(void) {
#ifdef WINDOWS
  WORD version;
  WSADATA data;
  int err;

  version = MAKEWORD(2, 2);
  if ((err = WSAStartup(version, &data)) != 0) {
    debug(DEBUG_ERROR, "SYS", "WSAStartup error %d", err);
  }
#endif
}

void sys_srand(int32_t seed) {
#ifdef WINDOWS
  srand(seed);
#else
  srandom(seed);
#endif
}

int32_t sys_rand(void) {
#ifdef WINDOWS
  return rand();
#else
  return random();
#endif
}

static int socket_select(int socket, uint32_t us, int nf) {
#ifdef WINDOWS
  TIMEVAL tv;
#else
  struct timeval tv;
#endif
  fd_set fds;
  int n, r;

  FD_ZERO(&fds);
  FD_SET(socket, &fds);

  tv.tv_sec = 0;
  tv.tv_usec = us;
  n = select(nf ? (socket + 1) : 0, &fds, NULL, NULL, us == ((uint32_t)-1) ? NULL : &tv);

  if (n == -1) {
    r = (errno == EINTR) ? 0 : -1; // a SIGCHLD can cause EINTR and it should not terminate the main thread wainting on its DGRAM socket
    debug_errno("SYS", "select socket");
  } else if (n == 0 || !FD_ISSET(socket, &fds)) {
    r = 0;
  } else {
    r = 1;
  }

  return r;
}

#ifdef WINDOWS
#define TAG_FD "SYS_FD"

#define FD_FILE   1
#define FD_PIPE   2
#define FD_SERIAL 3
#define FD_SOCKET 4

#define MAXBUF 65536

typedef uint32_t in_addr_t;

typedef struct {
  char *tag;
  int type;
  HANDLE handle;
  OVERLAPPED ovlr;
  OVERLAPPED ovlw;
  int socket;
  int waiting;
  uint8_t buf[MAXBUF];
} fd_t;

static void fd_destructor(void *p) {
  fd_t *f;

  f = (fd_t *)p;

  if (f) {
    debug(DEBUG_TRACE, "SYS", "fd_destructor type=%d f=0x%08X", f->type, f);

    switch (f->type) {
      case FD_FILE:
      case FD_PIPE:
      case FD_SERIAL:
        if (f->ovlr.hEvent) CloseHandle(f->ovlr.hEvent);
        if (f->ovlw.hEvent) CloseHandle(f->ovlw.hEvent);
        CloseHandle(f->handle);
        break;
      case FD_SOCKET:
        closesocket(f->socket);
        break;
    }
    sys_free(f);
  }
}

static int fd_open(int type, HANDLE handle, HANDLE eventr, HANDLE eventw, int socket) {
  fd_t *f;
  int fd;

  if ((f = sys_calloc(1, sizeof(fd_t))) == NULL) {
    return -1;
  }

  f->tag = TAG_FD;
  f->type = type;
  f->handle = handle;
  f->ovlr.hEvent = eventr;
  f->ovlw.hEvent = eventw;
  f->socket = socket;

  if ((fd = ptr_new(f, fd_destructor)) == -1) {
    if (f->ovlr.hEvent) CloseHandle(f->ovlr.hEvent);
    if (f->ovlw.hEvent) CloseHandle(f->ovlw.hEvent);
    if (f->handle) CloseHandle(f->handle);
    if (f->socket != -1) closesocket(f->socket);
    sys_free(f);
    return -1;
  }
  debug(DEBUG_TRACE, "SYS", "fd_open type=%d ptr=%d f=%p", type, fd, f);

  return fd;
}

static int fd_read_timeout(fd_t *f, unsigned char *buf, int n, int *nread, uint32_t us) {
  DWORD nbread, ms;
  int nr, r;

  *nread = 0;

  if (f->type == FD_SOCKET) {
    if ((r = socket_select(f->socket, us, 0)) <= 0) {
      return r;
    }

    nr = recv(f->socket, (char *)buf, n, 0);
    if (nr == -1) {
      debug_errno("SYS", "recv");
      return -1;
    }
    *nread = nr;
    return 1;
  }

  if (n > MAXBUF) n = MAXBUF;

  // if not waiting for data to arrive
  if (!f->waiting) {
    // try to read
    // XXX if ReadFile uses ovrl, file pointer does not advance
    if (ReadFile(f->handle, f->buf, n, &nbread, f->type == FD_FILE ? NULL : &f->ovlr)) {
      // data was read, return immediatelly
      sys_memcpy(buf, f->buf, nbread);
      *nread = nbread;
      return nbread ? 1 : 0;
    }
    // data was not available
    if (GetLastError() != ERROR_IO_PENDING) {
      // if the cause was not ERROR_IO_PENDING, return error
      debug_errno("SYS", "ReadFile");
      return -1;
    }
    // set waiting flag
    f->waiting = 1;
  }

  // if waiting for data to arrive
  if (f->waiting) {
    ms = us / 1000;
    // wait at most ms milliseconds for data to arrive
    switch (WaitForSingleObject(f->ovlr.hEvent, ms)) {
      case WAIT_OBJECT_0:
        f->waiting = 0;
        // data is available, try to read it
        if (GetOverlappedResult(f->handle, &f->ovlr, &nbread, TRUE)) {
          ResetEvent(f->ovlr.hEvent);
          // success
          if (nbread) {
            sys_memcpy(buf, f->buf, nbread);
            *nread = nbread;
            return 1;
          } else {
            return 0;
          }
        } else {
          // error
          debug(DEBUG_ERROR, "SYS", "overlapped read error");
        }
        break;
      case WAIT_TIMEOUT:
        // data is not available after timeout expired, return 0
        return 0;
      default:
        debug(DEBUG_ERROR, "SYS", "read wait error");
        break;
    }
  }

  return -1;
}

static int fd_write(fd_t *f, uint8_t *buf, int n) {
  DWORD written;
  int r = -1;

  switch (f->type) {
    case FD_FILE:
    case FD_PIPE:
      if (WriteFile(f->handle, buf, n, &written, NULL)) {
        r = written;
      } else {
        debug_errno("SYS", "WriteFile");
      }
      break;
    case FD_SERIAL:
      if (n > MAXBUF) n = MAXBUF;
      sys_memcpy(f->buf, buf, n);
      if (WriteFile(f->handle, f->buf, n, &written, &f->ovlw)) {
        r = written;
      } else {
        if (GetLastError() != ERROR_IO_PENDING) {
          debug_errno("SYS", "WriteFile");
        } else {
          switch (WaitForSingleObject(f->ovlw.hEvent, INFINITE)) {
            case WAIT_OBJECT_0:
              if (GetOverlappedResult(f->handle, &f->ovlw, &written, TRUE)) {
                ResetEvent(f->ovlw.hEvent);
                r = written;
              } else {
                debug(DEBUG_ERROR, "SYS", "overlapped write error");
              }
              break;
            case WAIT_TIMEOUT:
              break;
            default:
              debug(DEBUG_ERROR, "SYS", "write wait error");
              break;
          }
        }
      }
      break;
    case FD_SOCKET:
      if ((r = send(f->socket, (char *)buf, n, 0)) == -1) {
        debug_errno("SYS", "send");
      }
      break;
  }

  return r;
}
#endif

void sys_usleep(uint32_t us) {
  usleep(us);
}

uint64_t sys_time(void) {
  time_t t;
  return time(&t);
}

int sys_isdst(void) {
  sys_tm_t tm;
  uint64_t t;

  t = sys_time();
  sys_localtime(&t, &tm);

  return tm.tm_isdst > 0 ? 1 : 0;
}

uint64_t sys_timegm(sys_tm_t *tm) {
  struct tm stm;
  uint64_t t;

  xmemset(&stm, 0, sizeof(stm));
  stm.tm_year = tm->tm_year;
  stm.tm_mon = tm->tm_mon;
  stm.tm_mday = tm->tm_mday;
  stm.tm_wday = tm->tm_wday;
  stm.tm_hour = tm->tm_hour;
  stm.tm_min = tm->tm_min;
  stm.tm_sec = tm->tm_sec;
  stm.tm_isdst = tm->tm_isdst;
  stm.tm_yday = tm->tm_yday;
#ifdef WINDOWS32
  t = _mkgmtime(&stm);
#else
  t = timegm(&stm);
#endif

  return t;
}

uint64_t sys_timelocal(sys_tm_t *tm) {
  struct tm stm;
  uint64_t t;

  xmemset(&stm, 0, sizeof(stm));
  stm.tm_year = tm->tm_year;
  stm.tm_mon = tm->tm_mon;
  stm.tm_mday = tm->tm_mday;
  stm.tm_wday = tm->tm_wday;
  stm.tm_hour = tm->tm_hour;
  stm.tm_min = tm->tm_min;
  stm.tm_sec = tm->tm_sec;
  stm.tm_isdst = tm->tm_isdst;
  stm.tm_yday = tm->tm_yday;

#ifdef WINDOWS
  t = mktime(&stm);
#endif
#ifdef LINUX
  t = timelocal(&stm);
#endif
#ifdef SERENITY
  t = mktime(&stm);
#endif

  return t;
}

int sys_gmtime(const uint64_t *t, sys_tm_t *tm) {
  time_t tt;
#ifdef WINDOWS
  // the MSVC implementation of gmtime() is already thread safe
  struct tm *stm;
  tt = *t;
  stm = gmtime(&tt);
  tm->tm_year = stm->tm_year;
  tm->tm_mon = stm->tm_mon;
  tm->tm_mday = stm->tm_mday;
  tm->tm_wday = stm->tm_wday;
  tm->tm_hour = stm->tm_hour;
  tm->tm_min = stm->tm_min;
  tm->tm_sec = stm->tm_sec;
  tm->tm_isdst = stm->tm_isdst;
  tm->tm_yday = stm->tm_yday;
#else
  struct tm st;
  tt = *t;
  gmtime_r(&tt, &st);
  tm->tm_year = st.tm_year;
  tm->tm_mon = st.tm_mon;
  tm->tm_mday = st.tm_mday;
  tm->tm_wday = st.tm_wday;
  tm->tm_hour = st.tm_hour;
  tm->tm_min = st.tm_min;
  tm->tm_sec = st.tm_sec;
  tm->tm_isdst = st.tm_isdst;
  tm->tm_yday = st.tm_yday;
#endif

  return 0;
}

int sys_localtime(const uint64_t *t, sys_tm_t *tm) {
  time_t tt;
#ifdef WINDOWS
  struct tm *stm;
  // the MSVC implementation of localtime() is already thread safe
  tt = *t;
  stm = localtime(&tt);
  tm->tm_year = stm->tm_year;
  tm->tm_mon = stm->tm_mon;
  tm->tm_mday = stm->tm_mday;
  tm->tm_wday = stm->tm_wday;
  tm->tm_hour = stm->tm_hour;
  tm->tm_min = stm->tm_min;
  tm->tm_sec = stm->tm_sec;
  tm->tm_isdst = stm->tm_isdst;
  tm->tm_yday = stm->tm_yday;
#else
  struct tm st;
  tt = *t;
  localtime_r(&tt, &st);
  tm->tm_year = st.tm_year;
  tm->tm_mon = st.tm_mon;
  tm->tm_mday = st.tm_mday;
  tm->tm_wday = st.tm_wday;
  tm->tm_hour = st.tm_hour;
  tm->tm_min = st.tm_min;
  tm->tm_sec = st.tm_sec;
  tm->tm_isdst = st.tm_isdst;
  tm->tm_yday = st.tm_yday;
#endif

  return 0;
}

int sys_timeofday(sys_timeval_t *tv) {
  struct timeval t;
  int r;

  r = gettimeofday(&t, NULL);
  tv->tv_sec = t.tv_sec;
  tv->tv_usec = t.tv_usec;

  return r;
}

char *sys_getenv(char *name) {
  return getenv(name);
}

/*
The name of a locale consists of language codes, character encoding, and the description of a selected variant.

A name starts with an ISO 639-1 lowercase two-letter language code, or an ISO 639-2 three-letter language code if the language has no two-letter code. For example, it is de for German, fr for French, and cel for Celtic. The code is followed for many but not all languages by an underscore _ and by an ISO 3166 uppercase two-letter country code. For example, this leads to de_CH for Swiss German, and fr_CA for a French-speaking system for a Canadian user likely to be located in Quebec.

Optionally, a dot . follows the name of the character encoding such as UTF-8, or ISO-8859-1, and the @ sign followed by the name of a variant. For example, the name en_IE.UTF-8@euro describes the setup for an English system for Ireland with UTF-8 character encoding, and the Euro as the currency symbol.
*/

int sys_country(char *country, int len) {
  int r = -1;

#ifdef WINDOWS
  char buf[32];
  GEOID myGEO = GetUserGeoID(GEOCLASS_NATION);
  if (GetGeoInfoA(myGEO, GEO_ISO2, buf, sizeof(buf), 0)) {
    sys_strncpy(country, buf, len-1);
    r = 0;
  }
#else
  char *s, *p, buf[32];
  if ((s = getenv("LANG")) != NULL) {
    sys_strncpy(buf, s, sizeof(buf)-1);
    if ((p = sys_strchr(buf, '.')) != NULL) {
      // "pt_BR.UTF-8" -> "pt_BR"
      *p = 0;
    }
    if (!strcmp(buf, "C")) {
      sys_strncpy(buf, EN_US, sizeof(buf)-1);
    }
    if ((p = sys_strchr(buf, '_')) != NULL) {
      // "pt_BR" -> "BR"
      sys_strncpy(country, p+1, len-1);
      r = 0;
    }
  }
#endif

  return r;
}

int sys_language(char *language, int len) {
  int r = -1;

#ifdef WINDOWS
  char buf[32];
  LCID lcid = GetUserDefaultLCID();
  if (GetLocaleInfoA(lcid, LOCALE_SISO639LANGNAME, buf, sizeof(buf))) {
    // 2 letter country code
    sys_strncpy(language, buf, len-1);
    r = 0;
  } else if (GetLocaleInfoA(lcid, LOCALE_SISO639LANGNAME2, buf, sizeof(buf))) {
    // 3 letter country code
    sys_strncpy(language, buf, len-1);
    r = 0;
  }
#else
  char *s, *p, buf[32];
  if ((s = getenv("LANG")) != NULL) {
    sys_strncpy(buf, s, sizeof(buf)-1);
    if ((p = sys_strchr(buf, '.')) != NULL) {
      // "pt_BR.UTF-8" -> "pt_BR"
      *p = 0;
    }
    if (!strcmp(buf, "C")) {
      sys_strncpy(buf, EN_US, sizeof(buf)-1);
    }
    if ((p = sys_strchr(buf, '_')) != NULL) {
      // "pt_BR" -> "pt"
      *p = 0;
    }
    sys_strncpy(language, buf, len-1);
    r = 0;
  }
#endif

  return r;
}

uint32_t sys_get_pid(void) {
#if WINDOWS
  return getpid();
#else
  return getpid();
#endif
}

uint32_t sys_get_tid(void) {
#ifdef WINDOWS
  return GetCurrentThreadId();
#endif
#ifdef LINUX
  return syscall(SYS_gettid);
#endif
#ifdef SERENITY
  return gettid();
#endif
}

int sys_errno(void) {
#ifdef WINDOWS
  int err;
  err = GetLastError();
  if (!err) err = WSAGetLastError();
  return err;
#else
  return errno;
#endif
}

void sys_strerror(int err, char *msg, int len) {
#ifdef WINDOWS
  int i;

  msg[0] = 0;
  FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, err, 0, msg, len-1, NULL);
  for (i = 0; msg[i]; i++) {
    if (msg[i] == '\r' || msg[i] == '\n') msg[i] = ' ';
  }
  for (i--; i >= 0; i--) {
    if (msg[i] != ' ') break;
    msg[i] = 0;
  }
#else
  strerror_r(err, msg, len);
#endif
}

#ifdef WINDOWS
static void normalize_path(const char *src, char *dst, int len) {
  int i;

  for (i = 0; src[i] && i < len-1; i++) {
    if (src[i] == '/') dst[i] = FILE_SEP;
    else dst[i] = src[i];
  }
  dst[i] = 0;
}
#endif

sys_dir_t *sys_opendir(const char *pathname) {
  sys_dir_t *dir;

  if ((dir = sys_calloc(1, sizeof(sys_dir_t))) == NULL) {
    return NULL;
  }

#ifdef WINDOWS
  normalize_path(pathname, dir->buf, FILE_PATH);

  int len = sys_strlen(dir->buf);
  if (len && dir->buf[len-1] == FILE_SEP) {
    dir->buf[len] = '*';
    dir->buf[len+1] = 0;
  }
  dir->first = 1;
  dir->handle = FindFirstFile(dir->buf, &dir->ffd);
  if (dir->handle == INVALID_HANDLE_VALUE) {
    debug_errno("SYS", "FindFirstFile(\"%s\")", dir->buf);
    sys_free(dir);
    dir = NULL;
  }
#else
  dir->dir = opendir(pathname);
  if (dir->dir == NULL) {
    debug_errno("SYS", "opendir(\"%s\")", pathname);
    sys_free(dir);
    dir = NULL;
  }
#endif

  return dir;
}

int sys_readdir(sys_dir_t *dir, char *name, int len) {
#ifdef WINDOWS
  if (dir->first) {
    dir->first = 0;
  } else {
    if (!FindNextFile(dir->handle, &dir->ffd)) return -1;
  }
  sys_strncpy(name, dir->ffd.cFileName, len);
#else
  struct dirent *ent;
  errno =0;
  if ((ent = readdir(dir->dir)) == NULL) {
    if (errno) {
      debug_errno("SYS", "readdir");
    }
    return -1;
  }
  sys_strncpy(name, ent->d_name, len);
#endif

  return 0;
}

int sys_closedir(sys_dir_t *dir) {
  int r = -1;

  if (dir) {
#ifdef WINDOWS
    FindClose(dir->handle);
    r = 0;
#else
    r = closedir(dir->dir);
#endif
    sys_free(dir);
  }

  return r;
}

int sys_chdir(char *path) {
  char buf[FILE_PATH];
  int r = -1;

  if (path) {
#ifdef WINDOWS
    normalize_path(path, buf, FILE_PATH);
    r = chdir(buf);
#else
    sys_strncpy(buf, path, FILE_PATH);
    r = chdir(buf);
#endif
  }

  return r;
}

int sys_getcwd(char *buf, int len) {
  getcwd(buf, len);

  return 0;
}

int sys_open(const char *pathname, int flags) {
  int r;

#ifdef WINDOWS
  char buf[FILE_PATH];
  HANDLE handle;
  DWORD access = 0;
  DWORD disposition = OPEN_EXISTING;

  if (flags & SYS_READ)  access |= GENERIC_READ;
  if (flags & SYS_WRITE) access |= GENERIC_WRITE;
  if (flags & SYS_TRUNC) disposition = CREATE_ALWAYS;

  normalize_path(pathname, buf, FILE_PATH);

  handle = CreateFile(buf, access, 0, 0, disposition, FILE_ATTRIBUTE_NORMAL, 0);
  if (handle == INVALID_HANDLE_VALUE) {
    debug_errno("SYS", "sys_open CreateFile(\"%s\", 0x%08X)", buf, flags);
    return -1;
  }

  r = fd_open(FD_FILE, handle, NULL, NULL, -1);
#else
  uint32_t f = 0;
  int rd, wr;

  rd = (flags & SYS_READ);
  wr = (flags & SYS_WRITE);

  if (rd && wr) f |= O_RDWR;
  else if (rd)  f |= O_RDONLY;
  else if (wr)  f |= O_WRONLY;
  if (flags & SYS_NONBLOCK) f |= O_NONBLOCK;
  if (flags & SYS_TRUNC)    f |= O_TRUNC;

  if ((r = open(pathname, f)) == -1) {
    debug_errno("SYS", "open(\"%s\")", pathname);
  }
#endif

  return r;
}

int sys_create(const char *pathname, int flags, uint32_t mode) {
  int r;

#ifdef WINDOWS
  char buf[FILE_PATH];
  HANDLE handle;
  DWORD access = 0;
  DWORD disposition = CREATE_NEW;

  if (flags & SYS_READ)  access |= GENERIC_READ;
  if (flags & SYS_WRITE) access |= GENERIC_WRITE;
  if (flags & SYS_TRUNC) disposition = CREATE_ALWAYS;

  normalize_path(pathname, buf, FILE_PATH);

  handle = CreateFile(buf, access, 0, 0, disposition, FILE_ATTRIBUTE_NORMAL, 0);
  if (handle == INVALID_HANDLE_VALUE) {
    debug_errno("SYS", "sys_create CreateFile(\"%s\", 0x%08X)", buf, flags);
    return -1;
  }

  r = fd_open(FD_FILE, handle, NULL, NULL, -1);
#else
  uint32_t f = O_CREAT;
  int rd, wr;

  rd = (flags & SYS_READ);
  wr = (flags & SYS_WRITE);

  if (rd && wr) f |= O_RDWR;
  else if (rd)  f |= O_RDONLY;
  else if (wr)  f |= O_WRONLY;
  if (flags & SYS_NONBLOCK) f |= O_NONBLOCK;
  if (flags & SYS_TRUNC)    f |= O_TRUNC;

  if ((r = open(pathname, f, mode)) == -1) {
    debug_errno("SYS", "open(\"%s\")", pathname);
  }
#endif

  return r;
}

// return -1: error
// return  0: fd is not set within tv
// return  1, fd is set
int sys_select(int fd, uint32_t us) {
#ifdef WINDOWS
  fd_t *f;
  DWORD available;
  int r = -1;

  if ((f = ptr_lock(fd, TAG_FD)) == NULL) {
    return -1;
  }

  switch (f->type) {
    case FD_FILE:
      r = 1;
      break;

    case FD_PIPE:
      if (PeekNamedPipe(f->handle, NULL, 0, NULL, &available, NULL)) {
        r = available > 0 ? 1 : 0;
        if (r == 0 && us > 0) {
          usleep(us); // XXX always sleeps for us microseconds
          if (PeekNamedPipe(f->handle, NULL, 0, NULL, &available, NULL)) {
            r = available > 0 ? 1 : 0;
          } else {
            debug_errno("SYS", "PeekNamedPipe (1)");
            r = -1;
          }
        }
      } else {
        debug_errno("SYS", "PeekNamedPipe (2)");
      }
      break;

    case FD_SERIAL:
      // XXX
      debug(DEBUG_ERROR, "SYS", "select on serial not implemented");
      break;

    case FD_SOCKET:
      r = socket_select(f->socket, us, 0);
      break;
  }

  ptr_unlock(fd, TAG_FD);
  return r;
#else
  return socket_select(fd, us, 1);
#endif
}

void sys_fdclr(int n, sys_fdset_t *fds) {
  int slot = n / 32;
  int idx = n % 32;
  fds->mask[slot] &= ~(1 << idx);
}

void sys_fdset(int n, sys_fdset_t *fds) {
  int slot = n / 32;
  int idx = n % 32;
  fds->mask[slot] |= (1 << idx);
}

void sys_fdzero(sys_fdset_t *fds) {
  int slot;
  for (slot = 0; slot < 32; slot++) {
    fds->mask[slot] = 0;
  }
}

int sys_fdisset(int n, sys_fdset_t *fds) {
  int slot = n / 32;
  int idx = n % 32;
  return (fds->mask[slot] & (1 << idx)) ? 1 : 0;
}

int sys_select_fds(int nfds, sys_fdset_t *readfds, sys_fdset_t *writefds, sys_fdset_t *exceptfds, sys_timeval_t *timeout) {
  fd_set rfds, wfds, efds;
  int r, i, isset;
#ifdef WINDOWS
  TIMEVAL tv;
  fd_t *f;
  int map[FDSET_SIZE];
  int s;
#else
  struct timeval tv;
#endif

  if (timeout) {
    tv.tv_sec = timeout->tv_sec;
    tv.tv_usec = timeout->tv_usec;
  }

#ifdef WINDOWS
  for (i = 0; i < FDSET_SIZE; i++) {
    map[i] = 0;
  }
  nfds = 0;
#endif

  if (readfds) {
    FD_ZERO(&rfds);
    for (i = 0; i < FDSET_SIZE; i++) {
      if (sys_fdisset(i, readfds)) {
#ifdef WINDOWS
        if (i > 0 && (f = ptr_lock(i, TAG_FD)) != NULL) {
          if (f->type == FD_SOCKET) {
            FD_SET(f->socket, &rfds);
            map[i] = f->socket;
          }
          ptr_unlock(i, TAG_FD);
        }
#else
        FD_SET(i, &rfds);
#endif
      }
    }
  }

  if (writefds) {
    FD_ZERO(&wfds);
    for (i = 0; i < FDSET_SIZE; i++) {
      if (sys_fdisset(i, writefds)) {
#ifdef WINDOWS
        if (i > 0 && (f = ptr_lock(i, TAG_FD)) != NULL) {
          if (f->type == FD_SOCKET) {
            FD_SET(f->socket, &wfds);
            map[i] = f->socket;
          }
          ptr_unlock(i, TAG_FD);
        }
#else
        FD_SET(i, &wfds);
#endif
      }
    }
  }

  if (exceptfds) {
    FD_ZERO(&efds);
    for (i = 0; i < FDSET_SIZE; i++) {
      if (sys_fdisset(i, exceptfds)) {
#ifdef WINDOWS
        if (i > 0 && (f = ptr_lock(i, TAG_FD)) != NULL) {
          if (f->type == FD_SOCKET) {
            FD_SET(f->socket, &efds);
            map[i] = f->socket;
          }
          ptr_unlock(i, TAG_FD);
        }
#else
        FD_SET(i, &efds);
#endif
      }
    }
  }

  r = select(nfds, readfds ? &rfds : NULL, writefds ? &wfds : NULL, exceptfds ? &efds : NULL, timeout ? &tv : NULL);

  if (readfds) {
    sys_fdzero(readfds);
    for (i = 0; i < FDSET_SIZE; i++) {
#ifdef WINDOWS
      s = map[i];
      isset = s > 0 && FD_ISSET(s, &rfds);
#else
      isset = FD_ISSET(i, &rfds);
#endif
      if (isset) {
        sys_fdset(i, readfds);
      } else {
        sys_fdclr(i, readfds);
      }
    }
  }

  if (writefds) {
    sys_fdzero(writefds);
    for (i = 0; i < FDSET_SIZE; i++) {
#ifdef WINDOWS
      s = map[i];
      isset = s > 0 && FD_ISSET(s, &wfds);
#else
      isset = FD_ISSET(i, &wfds);
#endif
      if (isset) {
        sys_fdset(i, writefds);
      } else {
        sys_fdclr(i, writefds);
      }
    }
  }

  if (exceptfds) {
    sys_fdzero(exceptfds);
    for (i = 0; i < FDSET_SIZE; i++) {
#ifdef WINDOWS
      s = map[i];
      isset = s > 0 && FD_ISSET(s, &efds);
#else
      isset = FD_ISSET(i, &efds);
#endif
      if (isset) {
        sys_fdset(i, exceptfds);
      } else {
        sys_fdclr(i, exceptfds);
      }
    }
  }

  return r;
}

// return -1: error
// return  0: nothing to read from fd
// return  1, nread = 0: nothing was read from fd
// return  1, nread > 0: read nread bytes from fd
int sys_read_timeout(int fd, uint8_t *buf, int len, int *nread, uint32_t us) {
  int r;

  *nread = 0;

#ifdef WINDOWS
  DWORD nbread;
  fd_t *f;

  if ((f = ptr_lock(fd, TAG_FD)) == NULL) {
    return -1;
  }

  if (f->type == FD_FILE) {
    if (ReadFile(f->handle, buf, len, &nbread, NULL)) {
      *nread = nbread;
      r = 1;
    } else {
      r = -1;
    }
  } else {
    r = fd_read_timeout(f, buf, len, nread, us);
  }

  ptr_unlock(fd, TAG_FD);

  return r;
#else
  if ((r = sys_select(fd, us)) <= 0) {
    return r;
  }

  if ((r = read(fd, buf, len)) < 0) {
    debug_errno("SYS", "read");
    return r;
  }

  *nread = r;
  return 1;
#endif
}

int sys_read(int fd, uint8_t *buf, int len) {
  int nread = 0, r;

  for (;;) {
    r = sys_read_timeout(fd, buf, len, &nread, 100000);
    if (r == -1) return -1;
    if (r > 0) break;
    if (nread >= len) break;
  }

  return nread ? nread : 0;
}

int sys_write(int fd, uint8_t *buf, int len) {
  int i;

  if (fd < 0) {
    debug(DEBUG_ERROR, "SYS", "write to fd %d", fd);
    return -1;
  }

  if (buf == NULL) {
    debug(DEBUG_ERROR, "SYS", "write NULL buffer");
    return -1;
  }

  if (len < 0) {
    debug(DEBUG_ERROR, "SYS", "write len %d", len);
    return -1;
  }

#ifdef WINDOWS
  fd_t *f;

  if ((f = ptr_lock(fd, TAG_FD)) == NULL) {
    return -1;
  }

  i = fd_write(f, buf, len);

  ptr_unlock(fd, TAG_FD);

#else
  int j, r;

  for (i = 0, j = 0; i < len && j < 20; j++) {
    r = write(fd, buf+i, len-i);

    if (r == -1) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        //debug(DEBUG_ERROR, "SYS", "write %d bytes to socket %d would block", len-i, fd);
        sys_usleep(1000);
        continue;
      }
      debug_errno("SYS", "write(%d, buf, %d)", fd, len-i);
      i = -1;
      break;
    }
    i += r;
  }
#endif

  if (i >= 0 && i < len) {
    debug(DEBUG_ERROR, "SYS", "write only %d/%d bytes", i, len);
  }

  return i;
}

int64_t sys_seek(int fd, int64_t offset, sys_seek_t whence) {
  int64_t r = -1;

#ifdef WINDOWS
  fd_t *f;
  LONG high;
  DWORD method, pos;

  switch (whence) {
    case SYS_SEEK_SET: method = FILE_BEGIN;   break;
    case SYS_SEEK_CUR: method = FILE_CURRENT; break;
    case SYS_SEEK_END: method = FILE_END;     break;
    default: return -1;
  }

  if ((f = ptr_lock(fd, TAG_FD)) == NULL) {
    return -1;
  }

  if (f->type == FD_FILE) {
    high = offset >> 32;
    pos = SetFilePointer(f->handle, offset, &high, method);
    if (pos != INVALID_SET_FILE_POINTER) {
       r = (((int64_t)high) << 32) | pos;
    } else {
      debug_errno("SYS", "SetFilePointer");
    }
  }

  ptr_unlock(fd, TAG_FD);

#else
  switch (whence) {
    case SYS_SEEK_SET: r = lseek(fd, offset, SEEK_SET); break;
    case SYS_SEEK_CUR: r = lseek(fd, offset, SEEK_CUR); break;
    case SYS_SEEK_END: r = lseek(fd, offset, SEEK_END); break;
    default: return -1;
  }

  if (r == -1) {
    debug_errno("SYS", "lseek");
  }
#endif

  return r;
}

int sys_pipe(int *fd) {
#ifdef WINDOWS
  HANDLE r, w;
  int fd0, fd1;

  if (!CreatePipe(&r, &w, NULL, 0)) {
    debug_errno("SYS", "CreatePipe");
    return -1;
  }

  if ((fd0 = fd_open(FD_PIPE, r, NULL, NULL, -1)) == -1) {
    CloseHandle(r);
    CloseHandle(w);
    return -1;
  }

  if ((fd1 = fd_open(FD_PIPE, w, NULL, NULL, -1)) == -1) {
    CloseHandle(r);
    CloseHandle(w);
    ptr_free(fd0, TAG_FD);
    return -1;
  }

  fd[0] = fd0;
  fd[1] = fd1;

#else
  if (socketpair(AF_UNIX, SOCK_STREAM, 0, fd) == -1) {
    debug_errno("SYS", "socketpair");
    return -1;
  }
#endif

  return 0;
}

// return -1: error or client disconnected
// return  0: nothing to read from fd
// return  1, something to read from fd
int sys_peek(int fd) {
  int r;

  if ((r = sys_select(fd, 0)) <= 0) {
    return r;
  }

#ifndef WINDOWS
  // XXX how to do this on Windows ?
  char buf;
  if ((r = recv(fd, &buf, 1, MSG_PEEK)) < 0) {
    debug_errno("SYS", "recv");
  }
#endif

  return r;
}

int sys_fstat(int fd, sys_stat_t *st) {
  int r = -1;

#ifdef WINDOWS
  BY_HANDLE_FILE_INFORMATION info;
  fd_t *f;

  if (st) {
    xmemset(st, 0, sizeof(sys_stat_t));

    if ((f = ptr_lock(fd, TAG_FD)) != NULL) {
      if (f->type == FD_FILE) {
        if (GetFileInformationByHandle(f->handle, &info)) {
          st->mode = SYS_IFREG;
          st->mode |= SYS_IRUSR;
          st->mode |= SYS_IWUSR;
          st->size = (((uint64_t)info.nFileSizeHigh) << 32) | info.nFileSizeLow;
          r = 0;
        }
      } else if (f->type == FD_FILE) {
        st->mode = SYS_IFDIR;
        r = 0;
      }
      ptr_unlock(fd, TAG_FD);
    }
  }

#else
  struct stat sb;

  if (st) {
    if ((r = fstat(fd, &sb)) == 0) {
      st->mode = 0;
      if (S_ISREG(sb.st_mode)) st->mode |= SYS_IFREG;
      if (S_ISDIR(sb.st_mode)) st->mode |= SYS_IFDIR;
      if (sb.st_mode & S_IRUSR) st->mode |= SYS_IRUSR;
      if (sb.st_mode & S_IWUSR) st->mode |= SYS_IWUSR;

      st->size = sb.st_size;
      st->atime = sb.st_atime;
      st->mtime = sb.st_mtime;
      st->ctime = sb.st_ctime;
    } else {
      debug_errno("SYS", "fstat");
    }
  }
#endif

  return r;
}

int sys_stat(const char *pathname, sys_stat_t *st) {
  char buf[FILE_PATH];
  struct stat sb;
  int r = -1;

  if (pathname && st) {
#ifdef WINDOWS
    normalize_path(pathname, buf, FILE_PATH);
#else
    sys_strncpy(buf, pathname, FILE_PATH);
#endif

    if ((r = stat(buf, &sb)) == 0) {
      st->mode = 0;
      if (S_ISREG(sb.st_mode)) st->mode |= SYS_IFREG;
      if (S_ISDIR(sb.st_mode)) st->mode |= SYS_IFDIR;
      if (sb.st_mode & S_IRUSR) st->mode |= SYS_IRUSR;
      if (sb.st_mode & S_IWUSR) st->mode |= SYS_IWUSR;

      st->size = sb.st_size;
      st->atime = sb.st_atime;
      st->mtime = sb.st_mtime;
      st->ctime = sb.st_ctime;
    } else {
      if (errno != ENOENT) {
        debug_errno("SYS", "stat(\"%s\")", buf);
      }
    }
  }

  return r;
}

int sys_statfs(const char *pathname, sys_statfs_t *st) {
  int r = -1;

#ifdef WINDOWS
  char buf[FILE_PATH];
  ULARGE_INTEGER lpTotalNumberOfBytes;
  ULARGE_INTEGER lpTotalNumberOfFreeBytes;

  normalize_path(pathname, buf, FILE_PATH);
  if (GetDiskFreeSpaceEx(pathname, NULL, &lpTotalNumberOfBytes, &lpTotalNumberOfFreeBytes)) {
    st->total = lpTotalNumberOfBytes.QuadPart;
    st->free = lpTotalNumberOfFreeBytes.QuadPart;
    r = 0;
  }
#endif
#ifdef LINUX
  struct statfs sb;

  if (statfs(pathname, &sb) == 0) {
    st->total = sb.f_blocks * sb.f_bsize;
    st->free = sb.f_bfree * sb.f_bsize;
    r = 0;
  }
#endif
#ifdef SERENITY
  struct statvfs sb;

  if (statvfs(pathname, &sb) == 0) {
    r = 0;
  }
#endif

  return r;
}

int sys_close(int fd) {
  int r = -1;

#ifdef WINDOWS
  r = ptr_free(fd, TAG_FD);
#else
  if (fd != -1) {
    r = close(fd);
  }
#endif

  return r;
}

int sys_rename(const char *pathname1, const char *pathname2) {
  char buf1[FILE_PATH], buf2[FILE_PATH];
  int r;

#ifdef WINDOWS
  normalize_path(pathname1, buf1, FILE_PATH);
  normalize_path(pathname2, buf2, FILE_PATH);
  r = rename(buf1, buf2);
#else
  sys_strncpy(buf1, pathname1, FILE_PATH);
  sys_strncpy(buf2, pathname2, FILE_PATH);
  r = rename(buf1, buf2);
#endif

  if (r == -1) {
    debug_errno("SYS", "rename(\"%s\", \"%s\")", buf1, buf2);
  }

  return r;
}

int sys_unlink(const char *pathname) {
  char buf[FILE_PATH];
  int r;

#ifdef WINDOWS
  normalize_path(pathname, buf, FILE_PATH);
  r = unlink(buf);
#else
  sys_strncpy(buf, pathname, FILE_PATH);
  r = unlink(buf);
#endif

  if (r == -1) {
    debug_errno("SYS", "unlink(\"%s\")", buf);
  }

  return r;
}

int sys_rmdir(const char *pathname) {
  char buf[FILE_PATH];
  int r;

#ifdef WINDOWS
  normalize_path(pathname, buf, FILE_PATH);
  r = rmdir(buf);
#else
  sys_strncpy(buf, pathname, FILE_PATH);
  r = rmdir(buf);
#endif

  if (r == -1) {
    debug_errno("SYS", "rmdir(\"%s\")", buf);
  }

  return r;
}

int sys_mkdir(const char *pathname) {
  char buf[FILE_PATH];
  int r;

#ifdef WINDOWS
  normalize_path(pathname, buf, FILE_PATH);
#ifdef WINDOWS32
  r = mkdir(buf);
#else
  r = mkdir(buf, 0755);
#endif
#else
  sys_strncpy(buf, pathname, FILE_PATH);
  r = mkdir(buf, 0755);
#endif
  if (r == -1) {
    debug_errno("SYS", "mkdir(\"%s\")", buf);
  }

  return r;
}

int sys_serial_open(char *device, char *word, int baud) {
#ifdef WINDOWS
  HANDLE handle, eventr, eventw;
  COMMTIMEOUTS timeouts;
  DCB dcb;

  if ((eventr = CreateEvent(NULL, TRUE, FALSE, NULL)) == INVALID_HANDLE_VALUE) {
    debug_errno("SYS", "CreateEvent");
    return -1;
  }

  if ((eventw = CreateEvent(NULL, TRUE, FALSE, NULL)) == INVALID_HANDLE_VALUE) {
    debug_errno("SYS", "CreateEvent");
    CloseHandle(eventr);
    return -1;
  }

  handle = CreateFile(device, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);
  if (handle == INVALID_HANDLE_VALUE) {
    debug_errno("SYS", "CreateFile");
    CloseHandle(eventr);
    CloseHandle(eventw);
    return -1;
  }

  xmemset(&dcb, 0, sizeof(dcb));

  if (!GetCommState(handle, &dcb)) {
    debug_errno("SYS", "GetCommState");
    CloseHandle(eventr);
    CloseHandle(eventw);
    CloseHandle(handle);
    return -1;
  }

  dcb.DCBlength = sizeof(dcb);
  dcb.BaudRate = baud;
  dcb.fBinary = TRUE;
  dcb.fParity = TRUE;
  dcb.fOutxCtsFlow = FALSE;
  dcb.fOutxDsrFlow = FALSE;
  dcb.fDtrControl = DTR_CONTROL_DISABLE;
  dcb.fOutX = FALSE;
  dcb.fInX = FALSE;
  dcb.fNull = FALSE;
  dcb.fRtsControl = RTS_CONTROL_DISABLE;
  dcb.fAbortOnError = FALSE;

  switch (tolower(word[0])) {
    case 'n': dcb.Parity = NOPARITY; break;
    case 'o': dcb.Parity = ODDPARITY; break;
    case 'e': dcb.Parity = EVENPARITY; break;
    default:
      debug(DEBUG_ERROR, "SYS", "invalid parity %c", word[0]);
      return -1;
  }

  switch (word[1]) {
    case '5': dcb.ByteSize = 5; break;
    case '6': dcb.ByteSize = 6; break;
    case '7': dcb.ByteSize = 7; break;
    case '8': dcb.ByteSize = 8; break;
    default:
      debug(DEBUG_ERROR, "SYS", "invalid word size %c", word[1]);
      return -1;
  }

  switch (word[2]) {
    case '1': dcb.StopBits = ONESTOPBIT; break;
    case '2': dcb.StopBits = TWOSTOPBITS; break;
    default:
      debug(DEBUG_ERROR, "SYS", "invalid stop bits %c", word[2]);
      return -1;
  }

  if (!SetCommState(handle, &dcb)) {
    debug_errno("SYS", "SetCommState");
    CloseHandle(eventr);
    CloseHandle(eventw);
    CloseHandle(handle);
    return -1;
  }

  if (!GetCommTimeouts(handle, &timeouts)) {
    debug_errno("SYS", "GetCommTimeouts");
    CloseHandle(eventr);
    CloseHandle(eventw);
    CloseHandle(handle);
    return -1;
  }
  debug(DEBUG_INFO, "SYS", "read timeouts interval=%d, mult=%d, const=%d",
    timeouts.ReadIntervalTimeout, timeouts.ReadTotalTimeoutMultiplier, timeouts.ReadTotalTimeoutConstant);

  timeouts.ReadIntervalTimeout = MAXDWORD;
  timeouts.ReadTotalTimeoutMultiplier = MAXDWORD;
  timeouts.ReadTotalTimeoutConstant = 100;
  timeouts.WriteTotalTimeoutMultiplier = 0;
  timeouts.WriteTotalTimeoutConstant = 0;

  if (!SetCommTimeouts(handle, &timeouts)) {
    debug_errno("SYS", "SetCommTimeouts");
  }

  return fd_open(FD_SERIAL, handle, eventr, eventw, -1);
#else
  int serial;

  debug(DEBUG_INFO, "SYS", "trying to connect to device %s", device);

  if ((serial = open(device, O_RDWR | O_NONBLOCK)) == -1) {
    debug_errno("SYS", "open \"%s\"", device);
    return -1;
  }

  if (sys_serial_baud(serial, baud) == -1) {
    close(serial);
    return -1;
  }

  if (sys_serial_word(serial, word) == -1) {
    close(serial);
    return -1;
  }

  debug(DEBUG_INFO, "SYS", "fd %d connected to device %s at %d", serial, device, baud);

  return serial;
#endif
}

int sys_serial_baud(int serial, int baud) {
#ifdef WINDOWS
  debug(DEBUG_ERROR, "SYS", "sys_serial_baud not implemented on windows");
  return -1;
#else
  struct termios termios;

  switch (baud) {
    case     50: baud = B50; break;
    case     75: baud = B75; break;
    case    110: baud = B110; break;
    case    134: baud = B134; break;
    case    150: baud = B150; break;
    case    200: baud = B200; break;
    case    300: baud = B300; break;
    case    600: baud = B600; break;
    case   1200: baud = B1200; break;
    case   1800: baud = B1800; break;
    case   2400: baud = B2400; break;
    case   4800: baud = B4800; break;
    case   9600: baud = B9600; break;
    case  19200: baud = B19200; break;
    case  38400: baud = B38400; break;
    case  57600: baud = B57600; break;
#ifdef B115200
    case 115200: baud = B115200; break;
#endif
#ifdef B230400
    case 230400: baud = B230400; break;
#endif
#ifdef B460800
    case 460800: baud = B460800; break;
#endif
#ifdef B921600
    case 921600: baud = B921600; break;
#endif
#ifdef B1000000
    case 1000000: baud = B1000000; break;
#endif
#ifdef B2000000
    case 2000000: baud = B2000000; break;
#endif
#ifdef B3000000
    case 3000000: baud = B3000000; break;
#endif
#ifdef B4000000
    case 4000000: baud = B4000000; break;
#endif

    default:
      debug(DEBUG_ERROR, "SYS", "invalid baud rate %d", baud);
      return -1;
  }

  if (tcgetattr(serial, &termios) == -1) {
    debug_errno("SYS", "tcgetattr");
    return -1;
  }

  cfmakeraw(&termios);
  cfsetispeed(&termios, baud);
  cfsetospeed(&termios, baud);

  if (tcsetattr(serial, TCIOFLUSH | TCSANOW, &termios) == -1) {
    debug_errno("SYS", "tcsetattr");
    return -1;
  }

  return 0;
#endif
}

int sys_serial_word(int serial, char *word) {
#ifdef WINDOWS
  debug(DEBUG_ERROR, "SYS", "sys_serial_word not implemented on windows");
  return -1;
#else
  struct termios termios;
  int parity = 0, wordsize = 0, stopbits = 0;

  if (!word || sys_strlen(word) != 3) {
    debug(DEBUG_ERROR, "SYS", "invalid word specification");
    return 1;
  }

  switch (tolower(word[0])) {
    case 'n': parity = 0; break;
    case 'o': parity = PARENB | PARODD; break;
    case 'e': parity = PARENB; break;
    default:
      debug(DEBUG_ERROR, "SYS", "invalid parity %c", word[0]);
      return -1;
  }

  switch (word[1]) {
    case '5': wordsize = CS5; break;
    case '6': wordsize = CS6; break;
    case '7': wordsize = CS7; break;
    case '8': wordsize = CS8; break;
    default:
      debug(DEBUG_ERROR, "SYS", "invalid word size %c", word[1]);
      return -1;
  }

  switch (word[2]) {
    case '1': stopbits = 0; break;
    case '2': stopbits = CSTOPB; break;
    default:
      debug(DEBUG_ERROR, "SYS", "invalid stop bits %c", word[2]);
      return -1;
  }

  if (tcgetattr(serial, &termios) == -1) {
    debug_errno("SYS", "tcgetattr");
    return -1;
  }

  termios.c_iflag = parity ? IGNPAR | INPCK | ISTRIP : 0;
  termios.c_oflag = 0;
  termios.c_lflag = 0;

  termios.c_cc[VEOF] = 1; // VMIN
  termios.c_cc[VEOL] = 0; // VTIME

  termios.c_cflag &= ~(CSIZE | PARENB | PARODD | CSTOPB);
  termios.c_cflag |= wordsize | parity | stopbits | CREAD;

  if (tcsetattr(serial, TCIOFLUSH | TCSANOW, &termios) == -1) {
    debug_errno("SYS", "tcsetattr");
    return -1;
  }

  return 0;
#endif
}

void *sys_tty_raw(int fd) {
#ifdef WINDOWS
  return NULL;
#else
  struct termios term;
  struct termios *orig;

  if ((orig = sys_calloc(1, sizeof(struct termios))) == NULL) {
    return NULL;
  }

  tcgetattr(fd, orig);
  tcgetattr(fd, &term);
  cfmakeraw(&term);
  term.c_lflag &= ~ECHO;
  tcsetattr(fd, TCIOFLUSH | TCSANOW, &term);

  return orig;
#endif
}

int sys_tty_restore(int fd, void *p) {
#ifdef WINDOWS
  return 0;
#else
  int r = -1;

  if (p) {
    r = tcsetattr(fd, TCIOFLUSH | TCSANOW, (struct termios *)p);
    sys_free(p);
  }

  return r;
#endif
}

int sys_termsize(int fd, int *cols, int *rows) {
#ifdef WINDOWS
  return -1;
#else
  struct winsize ws;
  int r = -1;

  if (ioctl(fd, TIOCGWINSZ, &ws) == 0) {
    *cols = ws.ws_col;
    *rows = ws.ws_row;
    r = 0;
  } else {
    debug_errno("SYS", "ioctl");
  }

  return r;
#endif
}

int sys_isatty(int fd) {
#ifdef WINDOWS
  return 0;
#else
  return isatty(fd);
#endif
}

int sys_daemonize(void) {
#ifdef WINDOWS
  return -1;
#else
  pid_t pid;

  // first fork to become a daemon
  if ((pid = fork()) == -1) {
    debug_errno("SYS", "fork");
    return -1;
  }

  if (pid) {
    // parent exits
    exit(0);
  }

  // child continues

  if (setsid() == -1) {
    debug_errno("SYS", "setsid");
    return -1;
  }

  // second fork to prevent a controlling tty
  if ((pid = fork()) == -1) {
    debug_errno("SYS", "fork");
    return -1;
  }

  if (pid) {
    // child exits
    exit(0);
  }

  // grand-child continues

  // close file descriptoes 0, 1, 2 (stdin, stdout and stderr)
  if (close(0) == -1 || close(1) == -1 || close(2) == -1) {
    debug_errno("SYS", "close");
    return -1;
  }

  // assign file descriptor 0
  if (open("/dev/null", O_RDONLY) == -1) {
    debug_errno("SYS", "open");
    return -1;
  }

  // assign file descriptor 1
  if (open("/dev/null", O_RDONLY) == -1) {
    debug_errno("SYS", "open");
    return -1;
  }

  // assign file descriptor 2
  if (open("/dev/null", O_RDONLY) == -1) {
    debug_errno("SYS", "open");
    return -1;
  }

  return 0;
#endif
}

#ifndef WINDOWS
#if _POSIX_TIMERS > 0
static int64_t gettime(int type) {
  struct timespec tp;
  int64_t ts = -1;

  if (clock_gettime(type, &tp) == 0) {
    ts = ((int64_t)tp.tv_sec) * 1000000 + ((int64_t)tp.tv_nsec) / 1000;
  } else {
    debug_errno("SYS", "clock_gettime");
  }

  return ts;
}
#endif
#endif

int64_t sys_get_clock(void) {
  int64_t ts = -1;

#if WINDOWS
  LARGE_INTEGER ticks_per_second, ticks;
  QueryPerformanceFrequency(&ticks_per_second);
  QueryPerformanceCounter(&ticks);
  ticks.QuadPart *= 1000000;
  ticks.QuadPart /= ticks_per_second.QuadPart;
  ts = ticks.QuadPart;
#else
#if _POSIX_TIMERS > 0
  ts = gettime(CLOCK_MONOTONIC_RAW);
#else
  debug(DEBUG_ERROR, "SYS", "clock_gettime is not implemented");
#endif
#endif

  return ts;
}

int sys_get_clock_ts(sys_timespec_t *ts) {
  struct timespec t;
  int r;

#if WINDOWS
  r = clock_gettime(CLOCK_REALTIME, &t);
#else
#if _POSIX_TIMERS > 0
  r = clock_gettime(CLOCK_REALTIME, &t);
#else
  debug(DEBUG_ERROR, "SYS", "clock_gettime is not implemented");
  r = -1;
#endif
#endif

  ts->tv_sec = t.tv_sec;
  ts->tv_nsec = t.tv_nsec;

  return r;
}

int64_t sys_get_process_time(void) {
  int64_t ts = -1;

#ifdef WINDOWS
  FILETIME creationTime, exitTime, kernelTime, userTime;
  if (GetProcessTimes(GetCurrentProcess(), &creationTime, &exitTime, &kernelTime, &userTime)) {
    ts = ((((int64_t)userTime.dwHighDateTime) << 32) | userTime.dwLowDateTime) / 10;
  } else {
    debug(DEBUG_ERROR, "SYS", "GetProcessTimes failed");
  }
#else
#ifdef LINUX
  ts = gettime(CLOCK_PROCESS_CPUTIME_ID);
#else
  debug(DEBUG_ERROR, "SYS", "CLOCK_PROCESS_CPUTIME_ID is not implemented");
#endif
#endif

  return ts;
}

int64_t sys_get_thread_time(void) {
  int64_t ts = -1;

#ifdef WINDOWS
  FILETIME creationTime, exitTime, kernelTime, userTime;
  if (GetThreadTimes(GetCurrentThread(), &creationTime, &exitTime, &kernelTime, &userTime)) {
    ts = ((((int64_t)userTime.dwHighDateTime) << 32) | userTime.dwLowDateTime) / 10;
  } else {
    debug(DEBUG_ERROR, "SYS", "GetThreadTimes failed");
  }
#else
#ifdef LINUX
  ts = gettime(CLOCK_THREAD_CPUTIME_ID);
#else
  debug(DEBUG_ERROR, "SYS", "CLOCK_THREAD_CPUTIME_ID is not implemented");
#endif
#endif

  return ts;
}

int sys_set_thread_name(char *name) {
#ifdef LINUX
  pthread_setname_np(pthread_self(), name);
#endif
  return 0;
}

#ifndef WINDOWS
static void sys_set_signals(int op) {
  sigset_t set;

  sigemptyset(&set);
  sigaddset(&set, SIGINT);
  sigaddset(&set, SIGTERM);
  sigaddset(&set, SIGQUIT);
  sigaddset(&set, SIGTTIN);
  sigaddset(&set, SIGTTOU);
  sigaddset(&set, SIGCHLD);
  sigaddset(&set, SIGUSR1);
  sigaddset(&set, SIGUSR2);

  pthread_sigmask(op, &set, NULL);
}
#endif

void sys_block_signals(void) {
#ifndef WINDOWS
  sys_set_signals(SIG_BLOCK);
#endif
}

void sys_unblock_signals(void) {
#ifndef WINDOWS
  sys_set_signals(SIG_UNBLOCK);
#endif
}

void sys_install_handler(int signum, void (*handler)(int)) {
#ifdef WINDOWS
  signal(signum, handler);
#else
  struct sigaction action;
  xmemset(&action, 0, sizeof(action));
  action.sa_handler = handler;
  sigaction(signum, &action, NULL);
#endif
}

void sys_wait(int *status) {
#ifndef WINDOWS
  wait(status);
#endif
}

void *sys_lib_load(char *libname, int *first_load) {
  char buf[FILE_PATH];
  void *lib;
  int len;

  *first_load = 0;

#ifdef WINDOWS
  normalize_path(libname, buf, FILE_PATH-1);
  len = sys_strlen(buf);
  if (strstr(buf, ".dll") == NULL && sys_strchr(buf, '.') == NULL && FILE_PATH-len > 5) {
    sys_strcat(buf, ".dll");
  }

  // check if library is already loaded
  lib = (void *)GetModuleHandle(buf);

  if (lib == NULL) {
    // not loaded yet: load it
    lib = (void *)LoadLibrary(buf);

    if (lib != NULL) {
      debug(DEBUG_INFO, "SYS", "library %s loaded", buf);
      *first_load = 1;
    } else {
      debug_errno("SYS", "LoadLibrary \"%s\"", buf);
    }
  } else {
    // already loaded
    debug(DEBUG_INFO, "SYS", "library %s already loaded", buf);
  }
#else
  sys_strncpy(buf, libname, FILE_PATH-1);
  len = sys_strlen(buf);
  if (strstr(buf, ".so") == NULL && sys_strchr(buf, '.') == NULL && FILE_PATH-len > 4) {
    sys_strcat(buf, ".so");
  }

  // check if library is already loaded
  dlerror();

/* XXX RTLD_NODELETE can cause problems since global variables are not reinitialized
#ifdef RTLD_NODELETE
#define NODELETE RTLD_NODELETE
#else
#define NODELETE 0
#endif
*/
#define NODELETE 0

#ifdef RTLD_NOLOAD
  lib = dlopen(buf, RTLD_NOW | RTLD_NOLOAD);
#else
  lib = NULL;
#endif

  if (lib == NULL) {
    // not loaded yet: load it
    dlerror();
    lib = dlopen(buf, RTLD_NOW | NODELETE);

    if (lib != NULL) {
      debug(DEBUG_INFO, "SYS", "library %s loaded", buf);
      *first_load = 1;
    } else {
      debug(DEBUG_ERROR, "SYS", "dlopen \"%s\"", dlerror());
    }
  } else {
    // already loaded
    debug(DEBUG_INFO, "SYS", "library %s already loaded", buf);
  }
#endif

  return lib;
}

void *sys_lib_defsymbol(void *lib, char *name, int mandatory) {
  void *sym;

#ifdef WINDOWS
  sym = (void *)GetProcAddress((HMODULE)lib, name);

  if (sym == NULL && mandatory) {
    debug_errno("SYS", "GetProcAddress \"%s\"", name);
  }
#else
  dlerror();
  sym = dlsym(lib, name);

  if (sym == NULL && mandatory) {
    debug(DEBUG_ERROR, "SYS", "dlsym \"%s\"", dlerror());
  }
#endif

  return sym;
}

int sys_lib_close(void *lib) {
#ifdef WINDOWS
  return FreeLibrary((HMODULE)lib);
#else
  return dlclose(lib);
#endif
}

void sys_exit(int r) {
#ifndef ANDROID
  exit(r);
#endif
}

void sys_set_finish(int status) {
  debug(DEBUG_INFO, "SYS", "received finish request %d", status);
  thread_set_status(status);
  thread_set_flags(FLAG_FINISH);
}

static int sys_tcpip_fill_addr(struct sockaddr_storage *a, char *host, int port, int *len, int ipv4_only) {
  struct sockaddr_in *addr;
  struct sockaddr_in6 *addr6;
  struct addrinfo hints, *res, *pr;
  char name[256], *s;
  in_addr_t ip;
  int i, r = -1;

  xmemset((char *)a, 0, sizeof(struct sockaddr_storage));

  if (strchr(host, ':') != NULL) {
    // ipv6 numeric address
    addr6 = (struct sockaddr_in6 *)a;
    xmemset((char *)addr6, 0, sizeof(struct sockaddr_in6));
    if ((r = inet_pton(AF_INET6, host, &(addr6->sin6_addr))) != 1) {
      if (r == 0) {
        debug(DEBUG_ERROR, "SYS", "invalid ipv6 address %s", host);
      } else {
        debug_errno("SYS", "inet_pton");
      }
      r = -1;
    } else {
      addr6->sin6_family = AF_INET6;
      addr6->sin6_port = htons(port);
      //__be32 sin6_flowinfo;
      //addr6->sin6_scope_id = htonl(0x20);
      *len = sizeof(struct sockaddr_in6);
      r = 1;
    }

  } else if (host[0] && !isalpha((int)host[strlen(host)-1])) {
    // ipv4 numeric address
    addr = (struct sockaddr_in *)a;
    xmemset((char *)addr, 0, sizeof(struct sockaddr_in));
    if ((ip = inet_addr(host)) == INADDR_NONE && sys_strcmp(host, "255.255.255.255")) {
      debug(DEBUG_ERROR, "SYS", "invalid ipv4 address %s", host);
    }
    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = ip;
    addr->sin_port = htons(port);
    *len = sizeof(struct sockaddr_in);
    r = 0;

  } else {
    // host name
    xmemset((char *)&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    if ((r = getaddrinfo(host, NULL, &hints, &res)) != 0) {
      debug(DEBUG_ERROR, "SYS", "getaddrinfo %s error \"%s\"", host, gai_strerror(r));
      r = -1;

    } else {
      for (pr = res, i = 0, r = -1; pr && r == -1; pr = pr->ai_next, i++) {
        switch (pr->ai_family) {
          case AF_INET:
            addr = (struct sockaddr_in *)a;
            sys_memcpy(addr, pr->ai_addr, pr->ai_addrlen);
            s = (char *)inet_ntoa(addr->sin_addr);
            if (s) {
              addr->sin_family = AF_INET;
              addr->sin_port = htons(port);
              debug(DEBUG_TRACE, "SYS", "host %s resolves to ipv4 address %s len %d", host, s, pr->ai_addrlen);
              *len = pr->ai_addrlen;
              r = 0;
            }
            break;
          case AF_INET6:
            if (!ipv4_only) {
              addr6 = (struct sockaddr_in6 *)a;
              sys_memcpy(addr6, pr->ai_addr, pr->ai_addrlen);
              xmemset(name, 0, sizeof(name));
              if (inet_ntop(AF_INET6, &(addr6->sin6_addr), name, sizeof(name)-1)) {
                addr6->sin6_family = AF_INET6;
                addr6->sin6_port = htons(port);
                debug(DEBUG_TRACE, "SYS", "host %s resolves to ipv6 address %s len %d", host, name, pr->ai_addrlen);
                *len = pr->ai_addrlen;
                r = 1;
              }
            }
            break;
        }
      }
      freeaddrinfo(res);
    }
  }

  return r;
}

uint32_t sys_socket_ipv4(char *host) {
  struct sockaddr_storage a;
  struct sockaddr_in *addr;
  int len;
  uint32_t r = -1;

  if (sys_tcpip_fill_addr(&a, host, 0, &len, 1) == 0) {
    addr = (struct sockaddr_in *)&a;
    r = addr->sin_addr.s_addr;
  }

  return r;
}

int sys_socket_fill_addr(void *a, char *host, int port, int *len) {
  return sys_tcpip_fill_addr((struct sockaddr_storage *)a, host, port, len, 0);
}

static int sys_tcpip_type(int *type, int *proto) {
  switch (*type) {
    case IP_STREAM:
      *type = SOCK_STREAM;
      if (proto) *proto = IPPROTO_TCP;
      break;
    case IP_DGRAM:
      *type = SOCK_DGRAM;
      if (proto) *proto = IPPROTO_UDP;
      break;
    default:
      debug(DEBUG_ERROR, "SYS", "invalid socket type %d", *type);
      return -1;
  }

  return 0;
}

static int sys_tcpip_socket(char *host, int port, int *type, struct sockaddr_storage *addr, int *addrlen, int *ipv6, char *label) {
  int sock, proto;

  if (sys_tcpip_type(type, &proto) == -1) {
    return -1;
  }

  if (host && addr) {
    if ((*ipv6 = sys_tcpip_fill_addr(addr, host, port, addrlen, 0)) == -1) {
      return -1;
    }

    if (label) {
      debug(DEBUG_TRACE, "SYS", "trying to %s to %s host %s port %d (ipv%d)",
            label, *type == SOCK_STREAM ? "TCP" : "UDP", host, port, *ipv6 ? 6 : 4);
    }
  }

  if ((sock = socket(*ipv6 ? AF_INET6 : AF_INET, *type, proto)) == -1) {
    debug_errno("SYS", "socket");
    return -1;
  }

  return sock;
}

static int sys_tcpip_connect(char *host, int port, int type, uint32_t us) {
  struct sockaddr_storage addr;
  int sock, addrlen, ipv6, r;
  sys_timeval_t timeout;
  sys_fdset_t fds;

  if ((sock = sys_tcpip_socket(host, port, &type, &addr, &addrlen, &ipv6, "connect")) == -1) {
    return -1;
  }

#ifndef WINDOWS
  if (fcntl(sock, F_SETFL, O_NONBLOCK) != 0) {
    debug_errno("SYS", "fcntl O_NONBLOCK");
    closesocket(sock);
    return -1;
  }
#endif

  if (connect(sock, (struct sockaddr *)&addr, addrlen) == -1) {
    if (sys_errno() != EINPROGRESS) {
      debug_errno("SYS", "connect to %s port %d (ipv%d)", host, port, ipv6 ? 6 : 4);
      closesocket(sock);
      return -1;
    }
    sys_fdzero(&fds);
    sys_fdset(sock, &fds);
    timeout.tv_sec = 0;
    timeout.tv_usec = us;
    r = sys_select_fds(sock+1, NULL, &fds, NULL, us == -1 ? NULL : &timeout);
    if (r == -1) {
      debug_errno("SYS", "connect to %s port %d (ipv%d)", host, port, ipv6 ? 6 : 4);
      closesocket(sock);
      return -1;
    }
    if (r == 0) {
      debug(DEBUG_ERROR, "SYS", "connect to %s port %d (ipv%d) timeout", host, port, ipv6 ? 6 : 4);
      closesocket(sock);
      return -1;
    }
  }

  debug(DEBUG_TRACE, "SYS", "fd %d connected to %s host %s port %d (ipv%d)",
        sock, type == SOCK_STREAM ? "TCP" : "UDP", host, port, ipv6 ? 6 : 4);

  return sock;
}

int sys_socket_open_connect_timeout(char *host, int port, int type, uint32_t us) {
#ifdef WINDOWS
  int sock;

  if ((sock = sys_tcpip_connect(host, port, type, us)) == -1) {
    return -1;
  }

  return fd_open(FD_SOCKET, NULL, NULL, NULL, sock);
#else
  return sys_tcpip_connect(host, port, type, us);
#endif
}

int sys_socket_open_connect(char *host, int port, int type) {
  return sys_socket_open_connect_timeout(host, port, type, -1);
}

int sys_socket_open(int type, int ipv6) {
  int sock, addrlen;

  if ((sock = sys_tcpip_socket(NULL, 0, &type, NULL, &addrlen, &ipv6, NULL)) == -1) {
    return -1;
  }

#ifdef WINDOWS
  return fd_open(FD_SOCKET, NULL, NULL, NULL, sock);
#else
  return sock;
#endif
}


int sys_socket_connect(int sock, char *host, int port) {
  struct sockaddr_storage addr;
  int ipv6, len;

  if ((ipv6 = sys_tcpip_fill_addr(&addr, host, port, &len, 0)) == -1) {
    return -1;
  }

#ifdef WINDOWS
  fd_t *f;

  if ((f = ptr_lock(sock, TAG_FD)) == NULL) {
    return -1;
  }

  if (f->type != FD_SOCKET) {
    ptr_unlock(sock, TAG_FD);
    return -1;
  }

  if (connect(f->socket, (struct sockaddr *)&addr, len) == -1) {
    ptr_unlock(sock, TAG_FD);
    debug_errno("SYS", "connect to %s port %d (ipv%d)", host, port, ipv6 ? 6 : 4);
    return -1;
  }

  ptr_unlock(sock, TAG_FD);
#else
  if (connect(sock, (struct sockaddr *)&addr, len) == -1 && errno != EINPROGRESS) {
    debug_errno("SYS", "connect to %s port %d (ipv%d)", host, port, ipv6 ? 6 : 4);
    return -1;
  }
#endif

  return 0;
}

static int sys_tcpip_bind(int sock, char *host, int *pport) {
  struct sockaddr_storage addr;
  struct sockaddr_in *addr4;
  struct sockaddr_in6 *addr6;
  int addrlen, ipv6, reuse, type, bcast;
  socklen_t socklen;
  char *s, aux[256];
  int port, r;

  port = *pport;

  if ((ipv6 = sys_tcpip_fill_addr(&addr, host, port, &addrlen, 0)) == -1) {
    return -1;
  }

  reuse = 1;
  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuse, sizeof(reuse)) == -1) {
    debug_errno("SYS", "setsockopt SO_REUSEADDR");
    return -1;
  }

  socklen = sizeof(type);
  if (getsockopt(sock, SOL_SOCKET, SO_TYPE, (char *)&type, &socklen) == 0 && type == SOCK_DGRAM) {
    bcast = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (const char *)&bcast, sizeof(bcast)) == -1) {
      debug_errno("SYS", "setsockopt SO_BROADCAST");
#ifndef SERENITY
      closesocket(sock);
      return -1;
#endif
    } else {
      debug(DEBUG_TRACE, "SYS", "broadcast enabled");
    }
  }

#ifndef WINDOWS
  if (fcntl(sock, F_SETFL, O_NONBLOCK) != 0) {
    debug_errno("SYS", "fcntl O_NONBLOCK");
    return -1;
  }
#endif

#ifdef SERENITY
  if (port == 0) {
    // XXX if port==0, bind() should pick a randon port number, but
    // Serenity does not dot that. So we do the hard and inneficient way.
    r = -1;
    for (port = 16384; port < 65536; port++) {
      sys_tcpip_fill_addr(&addr, host, port, &addrlen, 0);
      r = bind(sock, (struct sockaddr *)&addr, addrlen);
      if (r == 0) break;
    }
  } else {
    r = bind(sock, (struct sockaddr *)&addr, addrlen);
  }
#else
  r = bind(sock, (struct sockaddr *)&addr, addrlen);
#endif
  if (r != 0) {
    debug_errno("SYS", "bind to port %d", port);
    return -1;
  }

  xmemset(&addr, 0, sizeof(addr));
  socklen = sizeof(addr);
  if (getsockname(sock, (struct sockaddr *)&addr, &socklen) == -1) {
    debug_errno("SYS", "getsockname");
    return -1;
  }

  switch (addr.ss_family) {
    case AF_INET:
      addr4 = (struct sockaddr_in *)&addr;
      s = (char *)inet_ntoa(addr4->sin_addr);
      *pport = ntohs(addr4->sin_port);
      break;
    case AF_INET6:
      addr6 = (struct sockaddr_in6 *)&addr;
      inet_ntop(AF_INET6, &(addr6->sin6_addr), aux, sizeof(aux)-1);
      s = aux;
      *pport = ntohs(addr6->sin6_port);
      break;
    default:
      debug(DEBUG_ERROR, "SYS", "fd %d received invalid address family %d", sock, addr.ss_family);
      return -1;
  }

  debug(DEBUG_TRACE, "SYS", "fd %d bound to host %s port %d (ipv%d)", sock, s, *pport, ipv6 ? 6 : 4);

  return 0;
}

int sys_socket_binds(int sock, char *host, int *port) {
#ifdef WINDOWS
  fd_t *f;
  int r = -1;

  if ((f = ptr_lock(sock, TAG_FD)) == NULL) {
    return -1;
  }

  r = sys_tcpip_bind(f->socket, host, port);

  ptr_unlock(sock, TAG_FD);

  return r;
#else
  return sys_tcpip_bind(sock, host, port);
#endif
}

int sys_socket_listen(int sock, int n) {
  if (listen(sock, n) == -1) {
    debug_errno("SYS", "listen");
    return -1;
  }

  return 0;
}

int sys_socket_bind(char *host, int *port, int type) {
  struct sockaddr_storage addr;
  int sock, addrlen, ipv6;

  if ((sock = sys_tcpip_socket(host, *port, &type, &addr, &addrlen, &ipv6, NULL)) == -1) {
    return -1;
  }

  if (sys_tcpip_bind(sock, host, port) == -1) {
    closesocket(sock);
    return -1;
  }

  if (type == SOCK_STREAM) {
    if (sys_socket_listen(sock, 5) == -1) {
      closesocket(sock);
      return -1;
    }
  }

#ifdef WINDOWS
  return fd_open(FD_SOCKET, NULL, NULL, NULL, sock);
#else
  return sock;
#endif
}

static int sys_tcpip_bind_connect(char *src_host, int src_port, char *host, int port, int type) {
  struct sockaddr_storage src_addr, addr;
  int sock, addrlen, src_ipv6, ipv6, reuse, len;

  if ((sock = sys_tcpip_socket(host, port, &type, &addr, &addrlen, &ipv6, "connect")) == -1) {
    return -1;
  }

  reuse = 1;
  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuse, sizeof(reuse)) == -1) {
    debug_errno("SYS", "setsockopt SO_REUSEADDR");
    closesocket(sock);
    return -1;
  }

  if ((src_ipv6 = sys_tcpip_fill_addr(&src_addr, src_host, src_port, &len, 0)) == -1) {
    closesocket(sock);
    return -1;
  }

  if (bind(sock, (struct sockaddr *)&src_addr, len) != 0) {
    debug_errno("SYS", "bind to port %d", src_port);
    closesocket(sock);
    return -1;
  }

  if (connect(sock, (struct sockaddr *)&addr, addrlen) == -1) {
    debug_errno("SYS", "connect to %s port %d (ipv%d)", host, port, ipv6 ? 6 : 4);
    closesocket(sock);
    return -1;
  }

  debug(DEBUG_TRACE, "SYS", "fd %d connected to %s host %s port %d (ipv%d)",
        sock, type == SOCK_STREAM ? "TCP" : "UDP", host, port, ipv6 ? 6 : 4);

  return sock;
}

int sys_socket_bind_connect(char *src_host, int src_port, char *host, int port, int type) {
#ifdef WINDOWS
  int sock;

  if ((sock = sys_tcpip_bind_connect(src_host, src_port, host, port, type)) == -1) {
    return -1;
  }

  return fd_open(FD_SOCKET, NULL, NULL, NULL, sock);
#else
  return sys_tcpip_bind_connect(src_host, src_port, host, port, type);
#endif
}

static int sys_tcpip_accept(int sock, char *host, int hlen, int *port, sys_timeval_t *tv, int nf) {
  struct sockaddr_storage addr;
  struct sockaddr_in *addr4;
  struct sockaddr_in6 *addr6;
  socklen_t addrlen;
  char *s;
  int csock, ipv6, r;

  if ((r = socket_select(sock, tv ? (tv->tv_sec * 1000000 + tv->tv_usec) : -1, nf)) <= 0) {
    return r;
  }

  xmemset((char *)&addr, 0, sizeof(addr));
  addrlen = sizeof(addr);
  csock = accept(sock, (struct sockaddr *)&addr, &addrlen);

  if (csock == -1) {
    debug_errno("SYS", "accept");
    return -1;
  }

  xmemset(host, 0, hlen);

  switch (addr.ss_family) {
    case AF_INET:
      addr4 = (struct sockaddr_in *)&addr;
      s = (char *)inet_ntoa(addr4->sin_addr);
      if (s) sys_strncpy(host, s, hlen-1);
      *port = ntohs(addr4->sin_port);
      ipv6 = 0;
      break;
    case AF_INET6:
      addr6 = (struct sockaddr_in6 *)&addr;
      inet_ntop(AF_INET6, &(addr6->sin6_addr), host, hlen-1);
      *port = ntohs(addr6->sin6_port);
      ipv6 = 1;
      break;
    default:
      debug(DEBUG_ERROR, "SYS", "fd %d accepted invalid address family %d", csock, addr.ss_family);
      closesocket(csock);
      return -1;
  }

#ifndef WINDOWS
  if (fcntl(csock, F_SETFL, O_NONBLOCK) != 0) {
    debug_errno("SYS", "fcntl");
    closesocket(csock);
    return -1;
  }
#endif

  debug(DEBUG_TRACE, "SYS", "fd %d accepted from host %s port %d (ipv%d)", csock, host, *port, ipv6 ? 6 : 4);

  return csock;
}

int sys_socket_accept(int sock, char *host, int hlen, int *port, sys_timeval_t *tv) {
#ifdef WINDOWS
  fd_t *f;
  int csock = -1;

  if ((f = ptr_lock(sock, TAG_FD)) == NULL) {
    return -1;
  }

  if (f->type == FD_SOCKET) {
    csock = sys_tcpip_accept(f->socket, host, hlen, port, tv, 0);
  }

  ptr_unlock(sock, TAG_FD);

  return csock > 0 ? fd_open(FD_SOCKET, NULL, NULL, NULL, csock) : csock;
#else
  return sys_tcpip_accept(sock, host, hlen, port, tv, 1);
#endif
}

static int sys_tcpip_sendto(int sock, char *host, int port, unsigned char *buf, int n) {
  struct sockaddr_storage addr;
  int len, r;

  if (sys_tcpip_fill_addr(&addr, host, port, &len, 0) == -1) {
    return -1;
  }

  r = sendto(sock, (const char *)buf, n, 0, (struct sockaddr *)&addr, len);
  if (r == -1) {
    debug_errno("SYS", "sendto");
  }

  return r;
}

int sys_socket_sendto(int sock, char *host, int port, unsigned char *buf, int n) {
  int r = -1;

#ifdef WINDOWS
  fd_t *f;

  if ((f = ptr_lock(sock, TAG_FD)) == NULL) {
    return -1;
  }

  if (f->type == FD_SOCKET) {
    r = sys_tcpip_sendto(f->socket, host, port, buf, n);
  }

  ptr_unlock(sock, TAG_FD);
#else
  r = sys_tcpip_sendto(sock, host, port, buf, n);
#endif

  return r;
}

static int sys_tcpip_recvfrom(int sock, char *host, int hlen, int *port, unsigned char *buf, int n, sys_timeval_t *tv, int nf) {
  struct sockaddr_storage addr;
  struct sockaddr_in *addr4;
  struct sockaddr_in6 *addr6;
  socklen_t addrlen;
  char *s;
  int r;

  if ((r = socket_select(sock, tv ? (tv->tv_sec * 1000000 + tv->tv_usec) : -1, nf)) <= 0) {
    return r;
  }

  addrlen = sizeof(addr);
  xmemset(&addr, 0, addrlen);
  r = recvfrom(sock, (char *)buf, n, 0, (struct sockaddr *)&addr, &addrlen);

  if (r != -1) {
    //debug(DEBUG_TRACE, "SYS", "fd %d received %d bytes", sock, r);
    xmemset(host, 0, hlen);

    switch (addr.ss_family) {
      case AF_INET:
        addr4 = (struct sockaddr_in *)&addr;
        s = (char *)inet_ntoa(addr4->sin_addr);
        if (s) sys_strncpy(host, s, hlen-1);
        *port = ntohs(addr4->sin_port);
        break;
      case AF_INET6:
        addr6 = (struct sockaddr_in6 *)&addr;
        inet_ntop(AF_INET6, &(addr6->sin6_addr), host, hlen-1);
        *port = ntohs(addr6->sin6_port);
        break;
      default:
        debug(DEBUG_ERROR, "SYS", "fd %d received invalid address family %d", sock, addr.ss_family);
        return -1;
    }
  } else {
    debug_errno("SYS", "recvfrom");
  }

  return r;
}

int sys_socket_recvfrom(int sock, char *host, int hlen, int *port, unsigned char *buf, int n, sys_timeval_t *tv) {
  int r = -1;

#ifdef WINDOWS
  fd_t *f;

  if ((f = ptr_lock(sock, TAG_FD)) == NULL) {
    return -1;
  }

  if (f->type == FD_SOCKET) {
    r = sys_tcpip_recvfrom(f->socket, host, hlen, port, buf, n, tv, 0);
  }

  ptr_unlock(sock, TAG_FD);
#else
  r = sys_tcpip_recvfrom(sock, host, hlen, port, buf, n, tv, 1);
#endif

  return r;
}

int sys_setsockopt(int sock, int level, int optname, const void *optval, int optlen) {
  int ilevel, ioptname, r = -1;

  switch (level) {
    case SYS_LEVEL_IP:
      ilevel = IPPROTO_IP;
      break;
    case SYS_LEVEL_TCP:
      ilevel = IPPROTO_TCP;
      break;
    case SYS_LEVEL_SOCK:
      ilevel = SOL_SOCKET;
      break;
    default:
      return -1;
  }

  switch (optname) {
    case SYS_TCP_NODELAY:
      ioptname = TCP_NODELAY;
      break;
    case SYS_SOCK_LINGER:
      ioptname = SO_LINGER;
      break;
    default:
      return -1;
  }

#ifdef WINDOWS
  fd_t *f;

  if ((f = ptr_lock(sock, TAG_FD)) == NULL) {
    return -1;
  }

  if (f->type == FD_SOCKET) {
    r = setsockopt(f->socket, ilevel, ioptname, optval, optlen);
  }

  ptr_unlock(sock, TAG_FD);
#else
  r = setsockopt(sock, ilevel, ioptname, optval, optlen);
#endif

  return r;
}

int sys_socket_shutdown(int sock, int dir) {
  int r = -1, d = 0;

#ifdef WINDOWS
  fd_t *f;

  if (dir & SYS_SHUTDOWN_RD) d |= SD_RECEIVE;
  if (dir & SYS_SHUTDOWN_WR) d |= SD_SEND;

  if ((f = ptr_lock(sock, TAG_FD)) == NULL) {
    return -1;
  }

  if (f->type == FD_SOCKET) {
    r = shutdown(sock, d);
  }

  ptr_unlock(sock, TAG_FD);
#else
  if (dir & SYS_SHUTDOWN_RD) d |= SHUT_RD;
  if (dir & SYS_SHUTDOWN_WR) d |= SHUT_WR;
  r = shutdown(sock, d);
#endif

  return r;
}

int sys_fork_exec(char *filename, char *argv[], int fd) {
#ifdef WINDOWS
  return -1;
#else
  char *envp[] = { NULL };
  pid_t pid;
  int r = -1;

  if (filename && argv) {
    if ((pid = fork()) == -1) {
      debug_errno("SYS", "fork");
      return -1;
    }

    if (pid) {
      // parent
      r = 0;
    } else {
      // child

      if (fd != -1) {
        close(0);
        close(1);
        dup(fd);
        dup(fd);
      }

      execve(filename, argv, envp);
      exit(0);
    }
  }

  return r;
#endif
}

#ifdef WINDOWS
int sys_tmpname(char *buf, int max) {
  char *t;
  int n, r = -1;

  if (max >= 256 + L_tmpnam + 1) {
    t = getenv("TEMP");
    xmemset(buf, 0, max);
    sys_strncpy(buf, t ? t : "\\", 255);
    n = sys_strlen(buf);
    if (tmpnam(&buf[n])) {
      r = 0;
    }
  }

  return r;
}
#endif

int sys_mkstemp(void) {
  char buf[32];
  int fd;

  sys_strcpy(buf, "tmpXXXXXX");
  fd = mkstemp(buf);
  unlink(buf);

  return fd;
}

void *sys_malloc(sys_size_t size) {
  return malloc(size);
}

void sys_free(void *ptr) {
  free(ptr);
}

void *sys_calloc(sys_size_t nmemb, sys_size_t size) {
  return calloc(nmemb, size);
}

void *sys_realloc(void *ptr, sys_size_t size) {
  return realloc(ptr, size);
}

char *sys_strdup(const char *s) {
  return strdup(s);
}

char *sys_strcpy(char *dest, const char *src) {
  return strcpy(dest, src);
}

char *sys_strncpy(char *dest, const char *src, sys_size_t n) {
  return strncpy(dest, src, n);
}

uint32_t sys_strlen(const char *s) {
  return strlen(s);
}

char *sys_strchr(const char *s, int c) {
  return strchr(s, c);
}

char *sys_strrchr(const char *s, int c) {
  return strrchr(s, c);
}

char *sys_strstr(const char *haystack, const char *needle) {
  return strstr(haystack, needle);
}

int sys_atoi(const char *nptr) {
  return atoi(nptr);
}

double sys_atof(const char *nptr) {
  return atof(nptr);
}

unsigned long int sys_strtoul(const char *nptr, char **endptr, int base) {
  return strtoul(nptr, endptr, base);
}

int sys_strcmp(const char *s1, const char *s2) {
  return strcmp(s1, s2);
}

int sys_strncmp(const char *s1, const char *s2, sys_size_t n) {
  return strncmp(s1, s2, n);
}

int sys_strcasecmp(const char *s1, const char *s2) {
  return strcasecmp(s1, s2);
}

int sys_strncasecmp(const char *s1, const char *s2, sys_size_t n) {
  return strncasecmp(s1, s2, n);
}

char *sys_strcat(char *dest, const char *src) {
  return strcat(dest, src);
}

char *sys_strncat(char *dest, const char *src, sys_size_t n) {
  return strncat(dest, src, n);
}

int sys_memcmp(const void *s1, const void *s2, sys_size_t n) {
  return memcmp(s1, s2, n);
}

void *sys_memcpy(void *dest, const void *src, sys_size_t n) {
  return memcpy(dest, src, n);
}

void *sys_memmove(void *dest, const void *src, sys_size_t n) {
  return memmove(dest, src, n);
}

void *sys_memset(void *s, int c, sys_size_t n) {
  return memset(s, c, n);
}

int sys_abs(int x) {
  return abs(x);
}
double sys_floor(double x) {
  return floor(x);
}

double sys_ceil(double x) {
  return ceil(x);
}

double sys_fabs(double x) {
  return fabs(x);
}

double sys_sqrt(double x) {
  return sqrt(x);
}

double sys_sin(double x) {
  return sin(x);
}

double sys_cos(double x) {
  return cos(x);
}

double sys_pi(void) {
  return M_PI;
}

double sys_atan2(double y, double x) {
  return atan2(y, x);
}

void sys_qsort(void *base, sys_size_t nmemb, sys_size_t size, int (*compar)(const void *, const void *)) {
  return qsort(base, nmemb, size, compar);
}

void *sys_bsearch(const void *key, const void *base, sys_size_t nmemb, sys_size_t size, int (*compar)(const void *, const void *)) {
  return bsearch(key, base, nmemb, size, compar);
}

int sys_toupper(int c) {
  return toupper(c);
}

int sys_tolower(int c) {
  return tolower(c);
}

int sys_sprintf(char *str, const char *format, ...) {
  sys_va_list ap;
  int r;

  sys_va_start(ap, format);
  r = vsprintf(str, format, ap);
  sys_va_end(ap);

  return r;
}

int sys_vsprintf(char *str, const char *format, sys_va_list ap) {
  return vsprintf(str, format, ap);
}

int sys_vsnprintf(char *str, sys_size_t size, const char *format, sys_va_list ap) {
  return vsnprintf(str, size, format, ap);
}

int sys_snprintf(char *str, sys_size_t size, const char *format, ...) {
  sys_va_list ap;
  int r;

  sys_va_start(ap, format);
  r = sys_vsnprintf(str, size, format, ap);
  sys_va_end(ap);

  return r;
}

int sys_sscanf(const char *str, const char *format, ...) {
  sys_va_list ap;
  int r;

  sys_va_start(ap, format);
  r = vsscanf(str, format, ap);
  sys_va_end(ap);

  return r;
}

sys_size_t sys_getpagesize(void) {
  return sysconf(_SC_PAGE_SIZE);
}

int sys_setjmp(sys_jmp_buf env) {
  return setjmp(env);
}

void sys_longjmp(sys_jmp_buf env, int val) {
  longjmp(env, val);
}

/*
// link with -ldbghelp

#ifdef WINDOWS
static BOOL CALLBACK EnumSymProc(PSYMBOL_INFO pSymInfo, ULONG SymbolSize, PVOID UserContext) {
  UNREFERENCED_PARAMETER(UserContext);
  debug(DEBUG_INFO, "SYS", "symbol %08X %4u %s", pSymInfo->Address, SymbolSize, pSymInfo->Name);
  return TRUE;
}
#endif

int sys_list_symbols(char *libname) {
#ifdef WINDOWS
  HANDLE hProcess = GetCurrentProcess();
  DWORD64 BaseOfDll;
  char *Mask = "*";
  int r = 0;

  debug(DEBUG_INFO, "SYS", "listing symbols of \"%s\"", libname);

  if (!SymInitialize(hProcess, NULL, FALSE)) {
    return -1;
  }

  BaseOfDll = SymLoadModuleEx(hProcess, NULL, libname, NULL, 0, 0, NULL, 0);
  if (BaseOfDll == 0) {
    SymCleanup(hProcess);
    return -1;
  }

  if (!SymEnumSymbols(hProcess, BaseOfDll, Mask, EnumSymProc, NULL)) {
    debug_errno("SYS", "SymEnumSymbols");
    r = -1;
  }

  SymCleanup(hProcess);
  debug(DEBUG_INFO, "SYS", "end listing symbols");

  return r;
#else
  return -1;
#endif
}
*/
