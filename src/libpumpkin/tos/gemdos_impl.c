#include <PalmOS.h>
#include <VFSMgr.h>

#ifdef ARMEMU
#include "armemu.h"
#endif
#include "m68k/m68k.h"
#include "m68k/m68kcpu.h"
#include "emupalmosinc.h"
#include "emupalmos.h"
#include "plibc.h"
#include "heap.h"
#include "tos.h"
#include "gemdos.h"
#include "gemdos_proto.h"
#include "bios_proto.h"
#include "debug.h"

void Pterm0(void) {
  emupalmos_finish(1);
}

int32_t Cconin(void) {
  return Bconin(2);
}

int32_t Cconout(int16_t c) {
  Bconout(2, c);
  return 1;
}

int32_t Cauxin(void) {
  return Bconin(1);
}

int32_t Cauxout(int16_t c) {
  Bconout(1, c);
  return 1;
}

int32_t Cprnout(int16_t c) {
  Bconout(0, c);
  return 1;
}

int32_t Crawio(int16_t w) {
  uint8_t b = w & 0xFF;
  return (b == 0xFF) ? Cconin() : Cconout(b);
}

int32_t Crawcin(void) {
  debug(DEBUG_ERROR, "TOS", "Crawcin not implemented");
  return 0;
}

int32_t Cnecin(void) {
  debug(DEBUG_ERROR, "TOS", "Cnecin not implemented");
  return 0;
}

int32_t Cconws(uint8_t *buf) {
  if (buf) plibc_write(1, buf, sys_strlen((char *)buf));
  return 0;
}

int32_t Cconrs(void) {
  debug(DEBUG_ERROR, "TOS", "Cconrs not implemented");
  return 0;
}

int32_t Cconis(void) {
  debug(DEBUG_ERROR, "TOS", "Cconis not implemented");
  return 0;
}

int32_t Dsetdrv(int16_t drv) {
  // Set the current drive and returns a bit-map of mounted drives
  emu_state_t *state = m68k_get_emu_state();
  tos_data_t *data = (tos_data_t *)state->extra;
  int32_t r = 0x07; // bitmap of drives a, b and c

  switch (drv) {
    case 0: data->volume = data->a; break;
    case 1: data->volume = data->b; break;
    case 2: data->volume = data->c; break;
  }

  return r;
}

int16_t Cconos(void) {
  debug(DEBUG_ERROR, "TOS", "Cconos not implemented");
  return 0;
}

int16_t Cprnos(void) {
  debug(DEBUG_ERROR, "TOS", "Cprnos not implemented");
  return 0;
}

int16_t Cauxis(void) {
  debug(DEBUG_ERROR, "TOS", "Cauxis not implemented");
  return 0;
}

int16_t Cauxos(void) {
  debug(DEBUG_ERROR, "TOS", "Cauxos not implemented");
  return 0;
}

int32_t Maddalt(void *start, int32_t size) {
  debug(DEBUG_ERROR, "TOS", "Maddalt not implemented");
  return 0;
}

int32_t Srealloc(int32_t len) {
  // allocate screen memory (TOS 4)
  // returns (if len has the value -1) the maximum possible size of the screen memory
  debug(DEBUG_ERROR, "TOS", "Srealloc not implemented");
  return 0;
}

int16_t Dgetdrv(void) {
  // returns the current drive number (A=0, B=1, etc)
  emu_state_t *state = m68k_get_emu_state();
  tos_data_t *data = (tos_data_t *)state->extra;

  if (data->volume == data->a) return 0;
  if (data->volume == data->b) return 1;
  if (data->volume == data->c) return 2;

  // should not happen
  return 0;
}

void Fsetdta(DTA *buf) {
  debug(DEBUG_ERROR, "TOS", "Fsetdta not implemented");
}

uint16_t Tgetdate(void) {
  // obtains the current date
  // returns a uint16_t number with the date, which is coded as follows:
  // Bits Meaning
  // 0-4  Day (1-31)
  // 5-8  Month (1-12)
  // 9-15 Year (0-119, 0=1980) 

  DateTimeType dateTime;
  UInt32 seconds = TimGetSeconds();
  TimSecondsToDateTime(seconds, &dateTime);
  uint16_t d = dateTime.day | (dateTime.month << 5) | ((dateTime.year - 1980) << 9);

  return d;
}

int16_t Tsetdate(uint16_t date) {
  debug(DEBUG_ERROR, "TOS", "Tsetdate not implemented");
  return -1;
}

uint32_t Tgettime(void) {
  // returns the system time, coded as follows:
  // Bits  Meaning
  // 0-4   Seconds in units of two (0-29)
  // 5-10  Minutes (0-59)
  // 11-15 Hours (0-23) 

  DateTimeType dateTime;
  UInt32 seconds = TimGetSeconds();
  TimSecondsToDateTime(seconds, &dateTime);
  uint32_t t = dateTime.second | (dateTime.minute << 5) | (dateTime.hour << 11);

  return t;
}

int16_t Tsettime(uint16_t time) {
  debug(DEBUG_ERROR, "TOS", "Tsettime not implemented");
  return -1;
}

DTA *Fgetdta(void) {
  emu_state_t *state = m68k_get_emu_state();
  tos_data_t *data = (tos_data_t *)state->extra;

  return (DTA *)(data->memory + 0x80);
}

uint16_t Sversion(void) {
  return (17 << 8) | 0; // 0.17 -> TOS version 1.62 (for no particular reason)
}

void Ptermres(int32_t keepcnt, int16_t retcode) {
  emupalmos_finish(1);
}

int16_t Dfree(DISKINFO *buf, int16_t driveno) {
  debug(DEBUG_ERROR, "TOS", "Dfree not implemented");
  return 0;
}

int32_t Dcreate(char *path) {
  emu_state_t *state = m68k_get_emu_state();
  tos_data_t *data = (tos_data_t *)state->extra;

  return path ? plibc_mkdir(data->volume, path) : -1;
}

int32_t Ddelete(char *path) {
  emu_state_t *state = m68k_get_emu_state();
  tos_data_t *data = (tos_data_t *)state->extra;

  return path ? plibc_remove(data->volume, path) : -1;
}

int16_t Dsetpath(char *path) {
  emu_state_t *state = m68k_get_emu_state();
  tos_data_t *data = (tos_data_t *)state->extra;
  char *s;
  int i;
  int16_t r = -1;

  if (path) {
    s = sys_strdup(path);
    for (i = 0; path[i]; i++) {
      s[i] = path[i] == '\\' ? '/' : path[i];
    }
    r = plibc_chdir(data->volume, s);
    sys_free(s);
  }

  return r;
}

int16_t Fcreate(char *fname, int16_t attr) {
  // creates a new file, or truncates an existing one
  // attr   File attributes:
  // bit 0: File is write-protected
  // bit 1: File is hidden
  // bit 2: File is a system file
  // bit 3: Volume label (diskette name)
  // bit 5: Archive bit 

  emu_state_t *state = m68k_get_emu_state();
  tos_data_t *data = (tos_data_t *)state->extra;
  int16_t handle;

  if (fname) {
    if ((handle = plibc_open(data->volume, fname, PLIBC_CREAT | PLIBC_TRUNC)) != -1) {
      plibc_close(handle);
    }
  }

  // XXX what should be returned ?
  return 0;
}

int32_t Fopen(char *fname, int16_t mode) {
  emu_state_t *state = m68k_get_emu_state();
  tos_data_t *data = (tos_data_t *)state->extra;
  int32_t handle = -1;

  if (fname) {
    // mode:
    // bits 0..2 = access mode (0=read only, 1=write only, 2=read/write)
    // bit     3 = reserved (to be set to 0)
    // bit  4..6 = sharing modus
    // bit     7 = inheritance flag
    int flags;
    switch (mode & 0x07) {
      case 0: flags = PLIBC_RDONLY; break;
      case 1: flags = PLIBC_WRONLY; break;
      case 2: flags = PLIBC_RDWR; break;
      default: flags = PLIBC_RDONLY; break;
    }
    handle = plibc_open(data->volume, fname, flags);
    debug(DEBUG_TRACE, "TOS", "Fopen(\"%s\", %d): %d", fname, mode, handle);
  }

  return handle;
}

int16_t Fclose(int16_t handle) {
  int16_t r = -1;

  if (handle > 3) {
    // 0 Keyboard (stdin:)
    // 1 Screen (stdout:)
    // 2 Serial port (stdaux:)
    // 3 Parallel port (stdprn:)
    r = plibc_close(handle);
  }

  return r;
}

int32_t Fread(int16_t handle, int32_t count, void *buf) {
  int32_t i, r = -1;

  if (buf) {
    if (handle == 0) {
      char *b = (char *)buf;
      for (i = 0; i < count; i++) {
        b[i] = plibc_getchar();
        if (b[i] == 0) break;
      }
      r = i;
      debug(DEBUG_TRACE, "TOS", "Fread stdin [%.*s]", r, b);
    } else if (handle > 3) {
      r = plibc_read(handle, buf, count);
    } else {
      debug(DEBUG_ERROR, "TOS", "Fread from invalid handle %d", handle);
    }
  }

  return r;
}

int32_t Fwrite(int16_t handle, int32_t count, void *buf) {
  int32_t r = -1;

  if (buf) {
    if (handle == 1) {
      debug(DEBUG_TRACE, "TOS", "Fwrite stdout [%.*s]", count, (char *)buf);
      r = plibc_write(handle, buf, count);
    } else if (handle > 3) {
      r = plibc_write(handle, buf, count);
    } else {
      debug(DEBUG_ERROR, "TOS", "Fwrite to invalid handle %d", handle);
    }
  }

  return r;
}

int16_t Fdelete(char *fname) {
  emu_state_t *state = m68k_get_emu_state();
  tos_data_t *data = (tos_data_t *)state->extra;

  return fname ? plibc_remove(data->volume, fname) : -1;
}

int32_t Fseek(int32_t offset, int16_t handle, int16_t seekmode) {
  int whence;

  // 0 = From start of file
  // 1 = From current position
  // 2 = From end of file 

  switch (seekmode) {
    case 0: whence = PLIBC_SEEK_SET; break;
    case 1: whence = PLIBC_SEEK_CUR; break;
    case 2: whence = PLIBC_SEEK_END; break;
    default: return -1;
  }

  return plibc_lseek(handle, offset, whence);
}

int16_t Fattrib(char *filename, int16_t wflag, int16_t attrib) {
  debug(DEBUG_ERROR, "TOS", "Fattrib not implemented");
  return 0;
}

void *Mxalloc(int32_t amount, int16_t mode) {
  emu_state_t *state = m68k_get_emu_state();
  tos_data_t *data = (tos_data_t *)state->extra;
  uint8_t *p = NULL;

  if (amount > 0) {
    // XXX mode is ignored
    p = heap_alloc(data->heap, amount);
  }

  return p;
}

int16_t Fdup(int16_t handle) {
  return plibc_dup(handle);
}

int16_t Fforce(int16_t stdh, int16_t nonstdh) {
  // redirects a standard channel to a specific other channel created by the application. The following apply

  debug(DEBUG_ERROR, "TOS", "Fforce not implemented");
  return 0;
}

int16_t Dgetpath(char *path, int16_t driveno) {
  // obtains the current directory on the drive driveno

  emu_state_t *state = m68k_get_emu_state();
  tos_data_t *data = (tos_data_t *)state->extra;
  int volume;

  switch (driveno) {
    case 0: volume = data->a; break;
    case 1: volume = data->b; break;
    case 2: volume = data->c; break;
    default: volume = -1; break;
  }

  // XXX Dgetpath does not have a size limit on path
  return volume != -1 ? plibc_getdir(volume, path, 256) : -1;
}

int32_t Mfree(void *block) {
  emu_state_t *state = m68k_get_emu_state();
  tos_data_t *data = (tos_data_t *)state->extra;
  uint8_t *p = (uint8_t *)block;
  int32_t r = -1;

  if (p >= (data->memory + data->heapStart) && p < (data->memory + data->heapStart + data->heapSize)) {
    heap_free(data->heap, p);
    r = 0;
  }

  return r;
}

int32_t Mshrink(void *block, int32_t newsiz) {
  return 0;
}

void Pterm(uint16_t retcode) {
  emupalmos_finish(1);
}

int32_t Fsfirst(char *filename, int16_t attr) {
  // obtain information about the first occurrence of a file or subdirectory
  // the directory entry will fill the disk transfer area (DTA)
  // filename: pointer to the filename or subdirectory (may contain '*' and '?')
  // attr: attributes that should be matched by the file searched for
  //   bit 0: include read-only files
  //   bit 1: include hidden files
  //   bit 2: include system files
  //   bit 3: include volume labels
  //   bit 4: include subdirectories
  //   bit 5: include files with archive-bit set 

  debug(DEBUG_ERROR, "TOS", "Fsfirst not implemented");
  return 0;
}

int16_t Fsnext(void) {
  // search for next file entry

  debug(DEBUG_ERROR, "TOS", "Fsnext not implemented");
  return 0;
}

int32_t Frename(char *oldname, char *newname) {
  emu_state_t *state = m68k_get_emu_state();
  tos_data_t *data = (tos_data_t *)state->extra;

  return oldname && newname ? plibc_rename(data->volume, oldname, newname) : -1;
}

void Fdatime(DOSTIME *timeptr, int16_t handle, int16_t wflag) {
  debug(DEBUG_ERROR, "TOS", "Fdatime not implemented");
}

int32_t Flock(int16_t handle, int16_t mode, int32_t start, int32_t length) {
  debug(DEBUG_ERROR, "TOS", "Flock not implemented");
  return 0;
}

int32_t Nversion(void) {
  debug(DEBUG_ERROR, "TOS", "Nversion not implemented");
  return 0;
}

int32_t Frlock(int16_t handle, int32_t start, int32_t length) {
  debug(DEBUG_ERROR, "TOS", "Frlock not implemented");
  return 0;
}

int32_t Frunlock(int16_t handle, int32_t start) {
  debug(DEBUG_ERROR, "TOS", "Frunlock not implemented");
  return 0;
}

int32_t Flock2(int16_t handle, int32_t length) {
  debug(DEBUG_ERROR, "TOS", "Flock2 not implemented");
  return 0;
}

int32_t Funlock(int16_t handle) {
  debug(DEBUG_ERROR, "TOS", "Funlock not implemented");
  return 0;
}

int32_t Fflush(int16_t handle) {
  debug(DEBUG_ERROR, "TOS", "Fflush not implemented");
  return 0;
}

void Syield(void) {
  debug(DEBUG_ERROR, "TOS", "Syield not implemented");
}

int32_t Fpipe(int16_t *usrh) {
  debug(DEBUG_ERROR, "TOS", "Fpipe not implemented");
  return 0;
}

int32_t Ffchown(int16_t fd, int16_t uid, int16_t gid) {
  debug(DEBUG_ERROR, "TOS", "Ffchown not implemented");
  return 0;
}

int32_t Ffchmod(int16_t fd, int16_t mode) {
  debug(DEBUG_ERROR, "TOS", "Ffchmod not implemented");
  return 0;
}

int16_t Fsync(int16_t handle) {
  debug(DEBUG_ERROR, "TOS", "Fsync not implemented");
  return 0;
}

int32_t Finstat(int16_t fh) {
  debug(DEBUG_ERROR, "TOS", "Finstat not implemented");
  return 0;
}

int32_t Foutstat(int16_t fh) {
  debug(DEBUG_ERROR, "TOS", "Foutstat not implemented");
  return 0;
}

int32_t Fgetchar(int16_t fh, int16_t mode) {
  debug(DEBUG_ERROR, "TOS", "Fgetchar not implemented");
  return 0;
}

int32_t Fputchar(int16_t fh, int32_t ch, int16_t mode) {
  debug(DEBUG_ERROR, "TOS", "Fputchar not implemented");
  return 0;
}

int32_t Pwait(void) {
  debug(DEBUG_ERROR, "TOS", "Pwait not implemented");
  return 0;
}

int16_t Pnice(int16_t delta) {
  debug(DEBUG_ERROR, "TOS", "Pnice not implemented");
  return 0;
}

int16_t Pgetpid(void) {
  debug(DEBUG_ERROR, "TOS", "Pgetpid not implemented");
  return 0;
}

int16_t Pgetppid(void) {
  debug(DEBUG_ERROR, "TOS", "Pgetppid not implemented");
  return 0;
}

int16_t Pgetpgrp(void) {
  debug(DEBUG_ERROR, "TOS", "Pgetpgrp not implemented");
  return 0;
}

int16_t Psetpgrp(int16_t pid, int16_t newgrp) {
  debug(DEBUG_ERROR, "TOS", "Psetpgrp not implemented");
  return 0;
}

int16_t Pgetuid(void) {
  debug(DEBUG_ERROR, "TOS", "Pgetuid not implemented");
  return 0;
}

int16_t Psetuid(int16_t id) {
  debug(DEBUG_ERROR, "TOS", "Psetuid not implemented");
  return 0;
}

int32_t Pkill(int16_t pid, int16_t sig) {
  debug(DEBUG_ERROR, "TOS", "Pkill not implemented");
  return 0;
}

int32_t Psignal(int16_t sig, int32_t handler) {
  debug(DEBUG_ERROR, "TOS", "Psignal not implemented");
  return 0;
}

int16_t Pvfork(void) {
  debug(DEBUG_ERROR, "TOS", "Pvfork not implemented");
  return 0;
}

int16_t Pgetgid(void) {
  debug(DEBUG_ERROR, "TOS", "Pgetgid not implemented");
  return 0;
}

int16_t Psetgid(int16_t id) {
  debug(DEBUG_ERROR, "TOS", "Psetgid not implemented");
  return 0;
}

int32_t Psigblock(int32_t mask) {
  debug(DEBUG_ERROR, "TOS", "Psigblock not implemented");
  return 0;
}

int32_t Psigsetmask(int32_t mask) {
  debug(DEBUG_ERROR, "TOS", "Psigsetmask not implemented");
  return 0;
}

int32_t Pusrval(int32_t val) {
  debug(DEBUG_ERROR, "TOS", "Pusrval not implemented");
  return 0;
}

int16_t Pdomain(int16_t dom) {
  debug(DEBUG_ERROR, "TOS", "Pdomain not implemented");
  return 0;
}

void Psigreturn(void) {
  debug(DEBUG_ERROR, "TOS", "Psigreturn not implemented");
}

int16_t Pfork(void) {
  debug(DEBUG_ERROR, "TOS", "Pfork not implemented");
  return 0;
}

int32_t Pwait3(int16_t flag, int32_t *rusage) {
  debug(DEBUG_ERROR, "TOS", "Pwait3 not implemented");
  return 0;
}

int32_t Fselect(uint16_t timeout, int32_t *rfds, int32_t *wfds) {
  debug(DEBUG_ERROR, "TOS", "Fselect not implemented");
  return 0;
}

int32_t Prusage(int32_t *r) {
  debug(DEBUG_ERROR, "TOS", "Prusage not implemented");
  return 0;
}

int32_t Psetlimit(int16_t lim, int32_t value) {
  debug(DEBUG_ERROR, "TOS", "Psetlimit not implemented");
  return 0;
}

int32_t Talarm(int32_t time) {
  debug(DEBUG_ERROR, "TOS", "Talarm not implemented");
  return 0;
}

void Pause(void) {
  debug(DEBUG_ERROR, "TOS", "Pause not implemented");
}

int32_t Sysconf(int16_t n) {
  debug(DEBUG_ERROR, "TOS", "Sysconf not implemented");
  return 0;
}

int32_t Psigpending(void) {
  debug(DEBUG_ERROR, "TOS", "Psigpending not implemented");
  return 0;
}

int32_t Dpathconf(uint8_t *name, int16_t mode) {
  debug(DEBUG_ERROR, "TOS", "Dpathconf not implemented");
  return 0;
}

int32_t Pmsg(int16_t mode, int32_t mbox, void *msg) {
  debug(DEBUG_ERROR, "TOS", "Pmsg not implemented");
  return 0;
}

int32_t Fmidipipe(int16_t pid, int16_t in, int16_t out) {
  debug(DEBUG_ERROR, "TOS", "Fmidipipe not implemented");
  return 0;
}

int32_t Prenice(int16_t pid, int16_t delta) {
  debug(DEBUG_ERROR, "TOS", "Prenice not implemented");
  return 0;
}

int32_t Dopendir(char *name, int16_t flag) {
  debug(DEBUG_ERROR, "TOS", "Dopendir not implemented");
  return 0;
}

int32_t Dreaddir(int16_t len, int32_t dirhandle, char *buf) {
  debug(DEBUG_ERROR, "TOS", "Dreaddir not implemented");
  return 0;
}

int32_t Drewinddir(int32_t handle) {
  debug(DEBUG_ERROR, "TOS", "Drewinddir not implemented");
  return 0;
}

int32_t Dclosedir(int32_t dirhandle) {
  debug(DEBUG_ERROR, "TOS", "Dclosedir not implemented");
  return 0;
}

int32_t Fxattr(int16_t flag, char *name, XATTR *xattr) {
  debug(DEBUG_ERROR, "TOS", "Fxattr not implemented");
  return 0;
}

int32_t Flink(char *oldname, char *newname) {
  debug(DEBUG_ERROR, "TOS", "Flink not implemented");
  return 0;
}

int32_t Fsymlink(char *oldname, char *newname) {
  debug(DEBUG_ERROR, "TOS", "Fsymlink not implemented");
  return 0;
}

int32_t Freadlink(int16_t bufsiz, char *buf, char *name) {
  debug(DEBUG_ERROR, "TOS", "Freadlink not implemented");
  return 0;
}

int32_t Dcntl(int16_t cmd, char *name, int32_t arg) {
  debug(DEBUG_ERROR, "TOS", "Dcntl not implemented");
  return 0;
}

int32_t Fchown(char *name, int16_t uid, int16_t gid) {
  debug(DEBUG_ERROR, "TOS", "Fchown not implemented");
  return 0;
}

int32_t Fchmod(char *name, int16_t mode) {
  debug(DEBUG_ERROR, "TOS", "Fchmod not implemented");
  return 0;
}

int16_t Pumask(int16_t mode) {
  debug(DEBUG_ERROR, "TOS", "Pumask not implemented");
  return 0;
}

int32_t Psemaphore(int16_t mode, int32_t id, int32_t timeout) {
  debug(DEBUG_ERROR, "TOS", "Psemaphore not implemented");
  return 0;
}

int32_t Dlock(int16_t mode, int16_t drv) {
  debug(DEBUG_ERROR, "TOS", "Dlock not implemented");
  return 0;
}

void Psigpause(int32_t mask) {
  debug(DEBUG_ERROR, "TOS", "Psigpause not implemented");
}

int32_t Psigaction(int16_t sig, struct sigaction *act, struct sigaction *oact) {
  debug(DEBUG_ERROR, "TOS", "Psigaction not implemented");
  return 0;
}

int32_t Pgeteuid(void) {
  debug(DEBUG_ERROR, "TOS", "Pgeteuid not implemented");
  return 0;
}

int32_t Pgetegid(void) {
  debug(DEBUG_ERROR, "TOS", "Pgetegid not implemented");
  return 0;
}

int32_t Pwaitpid(int16_t pid, int16_t flag, int32_t *rusage) {
  debug(DEBUG_ERROR, "TOS", "Pwaitpid not implemented");
  return 0;
}

int32_t Dgetcwd(char *path, int16_t drv, int16_t size) {
  debug(DEBUG_ERROR, "TOS", "Dgetcwd not implemented");
  return 0;
}

void Salert(char *msg) {
  debug(DEBUG_ERROR, "TOS", "Salert not implemented");
}

int32_t Tmalarm(int32_t time) {
  debug(DEBUG_ERROR, "TOS", "Tmalarm not implemented");
  return 0;
}

int32_t Psigintr(int16_t vec, int16_t sig) {
  debug(DEBUG_ERROR, "TOS", "Psigintr not implemented");
  return 0;
}

int32_t Suptime(int32_t *uptime, int32_t *loadaverage) {
  debug(DEBUG_ERROR, "TOS", "Suptime not implemented");
  return 0;
}

int16_t Ptrace(int16_t request, int16_t pid, void *addr, int32_t data) {
  debug(DEBUG_ERROR, "TOS", "Ptrace not implemented");
  return 0;
}

int32_t Mvalidate(int16_t pid, void *start, int32_t size, int32_t *flags) {
  debug(DEBUG_ERROR, "TOS", "Mvalidate not implemented");
  return 0;
}

int32_t Dxreaddir(int16_t ln, int32_t dirh, char *buf, XATTR *xattr, int32_t *xr) {
  debug(DEBUG_ERROR, "TOS", "Dxreaddir not implemented");
  return 0;
}

int32_t Pseteuid(int16_t euid) {
  debug(DEBUG_ERROR, "TOS", "Pseteuid not implemented");
  return 0;
}

int32_t Psetegid(int16_t egid) {
  debug(DEBUG_ERROR, "TOS", "Psetegid not implemented");
  return 0;
}

int16_t Pgetauid(void) {
  debug(DEBUG_ERROR, "TOS", "Pgetauid not implemented");
  return 0;
}

int16_t Psetauid(int16_t id) {
  debug(DEBUG_ERROR, "TOS", "Psetauid not implemented");
  return 0;
}

int32_t Pgetgroups(int16_t len, int16_t *gidset) {
  debug(DEBUG_ERROR, "TOS", "Pgetgroups not implemented");
  return 0;
}

int32_t Psetgroups(int16_t len, int16_t *gidset) {
  debug(DEBUG_ERROR, "TOS", "Psetgroups not implemented");
  return 0;
}

int32_t Tsetitimer(int16_t which, int32_t *interval, int32_t *value, int32_t *ointerval, int32_t *ovalue) {
  debug(DEBUG_ERROR, "TOS", "Tsetitimer not implemented");
  return 0;
}

int32_t Dchroot(char *path) {
  debug(DEBUG_ERROR, "TOS", "Dchroot not implemented");
  return 0;
}

int32_t Fstat64(int16_t flag, char *name, STAT *stat) {
  debug(DEBUG_ERROR, "TOS", "Fstat64 not implemented");
  return 0;
}

int32_t Fseek64(int32_t hioffset, uint32_t lowoffset, int16_t handle, int16_t seekmode, int64_t *newpos) {
  debug(DEBUG_ERROR, "TOS", "Fseek64 not implemented");
  return 0;
}

int32_t Dsetkey(int32_t hidev, int32_t lowdev, char *key, int16_t cipher) {
  debug(DEBUG_ERROR, "TOS", "Dsetkey not implemented");
  return 0;
}

int32_t Psetreuid(int16_t ruid, int16_t euid) {
  debug(DEBUG_ERROR, "TOS", "Psetreuid not implemented");
  return 0;
}

int32_t Psetregid(int16_t rgid, int16_t egid) {
  debug(DEBUG_ERROR, "TOS", "Psetregid not implemented");
  return 0;
}

void Sync(void) {
  debug(DEBUG_ERROR, "TOS", "Sync not implemented");
}

int32_t Shutdown(int32_t mode) {
  debug(DEBUG_ERROR, "TOS", "Shutdown not implemented");
  return 0;
}

int32_t Dreadlabel(char *path, char *label, int16_t length) {
  debug(DEBUG_ERROR, "TOS", "Dreadlabel not implemented");
  return 0;
}

int32_t Dwritelabel(char *path, char *label) {
  debug(DEBUG_ERROR, "TOS", "Dwritelabel not implemented");
  return 0;
}

int32_t Ssystem(int16_t mode, int32_t arg1, int32_t arg2) {
  debug(DEBUG_ERROR, "TOS", "Ssystem not implemented");
  return 0;
}

int32_t Tgettimeofday(struct timeval *tv, timezone *tzp) {
  debug(DEBUG_ERROR, "TOS", "Tgettimeofday not implemented");
  return 0;
}

int32_t Tsettimeofday(struct timeval *tv, timezone *tzp) {
  debug(DEBUG_ERROR, "TOS", "Tsettimeofday not implemented");
  return 0;
}

int Tadjtime(struct timeval *delta, struct timeval *olddelta) {
  debug(DEBUG_ERROR, "TOS", "Tadjtime not implemented");
  return 0;
}

int32_t Pgetpriority(int16_t which, int16_t who) {
  debug(DEBUG_ERROR, "TOS", "Pgetpriority not implemented");
  return 0;
}

int32_t Psetpriority(int16_t which, int16_t who, int16_t pri) {
  debug(DEBUG_ERROR, "TOS", "Psetpriority not implemented");
  return 0;
}

int32_t Fpoll(POLLFD *fds, uint32_t nfds, uint32_t timeout) {
  debug(DEBUG_ERROR, "TOS", "Fpoll not implemented");
  return 0;
}

int32_t Fwritev(int16_t handle, struct iovec *iov, int32_t niov) {
  debug(DEBUG_ERROR, "TOS", "Fwritev not implemented");
  return 0;
}

int32_t Freadv(int16_t handle, struct iovec *iov, int32_t niov) {
  debug(DEBUG_ERROR, "TOS", "Freadv not implemented");
  return 0;
}

int32_t Ffstat64(int16_t fd, STAT *stat) {
  debug(DEBUG_ERROR, "TOS", "Ffstat64 not implemented");
  return 0;
}

int32_t Psysctl(int32_t *name, uint32_t namelen, void *old, uint32_t *oldlenp, void *new, uint32_t newlen) {
  debug(DEBUG_ERROR, "TOS", "Psysctl not implemented");
  return 0;
}

int32_t Fsocket(int32_t domain, int32_t type, int32_t protocol) {
  debug(DEBUG_ERROR, "TOS", "Fsocket not implemented");
  return 0;
}

int32_t Fsocketpair(int32_t domain, int32_t type, int32_t protocol, int16_t *fds) {
  debug(DEBUG_ERROR, "TOS", "Fsocketpair not implemented");
  return 0;
}

int32_t Faccept(int16_t fd, struct sockaddr *name, uint32_t *anamelen) {
  debug(DEBUG_ERROR, "TOS", "Faccept not implemented");
  return 0;
}

int32_t Fconnect(int16_t fd, struct sockaddr *name, uint32_t anamelen) {
  debug(DEBUG_ERROR, "TOS", "Fconnect not implemented");
  return 0;
}

int32_t Fbind(int16_t fd, struct sockaddr *name, uint32_t anamelen) {
  debug(DEBUG_ERROR, "TOS", "Fbind not implemented");
  return 0;
}

int32_t Flisten(int16_t fd, int32_t backlog) {
  debug(DEBUG_ERROR, "TOS", "Flisten not implemented");
  return 0;
}

int32_t Frecvmsg(int16_t fd, struct msghdr *msg, int32_t flags) {
  debug(DEBUG_ERROR, "TOS", "Frecvmsg not implemented");
  return 0;
}

int32_t Fsendmsg(int16_t fd, struct msghdr *msg, int32_t flags) {
  debug(DEBUG_ERROR, "TOS", "Fsendmsg not implemented");
  return 0;
}

int32_t Frecvfrom(int16_t fd, void *buf, int32_t buflen, int32_t flags, struct sockaddr *to, uint32_t *addrlen) {
  debug(DEBUG_ERROR, "TOS", "Frecvfrom not implemented");
  return 0;
}

int32_t Fsendto(int16_t fd, void *buf, int32_t buflen, int32_t flags, struct sockaddr *to, uint32_t addrlen) {
  debug(DEBUG_ERROR, "TOS", "Fsendto not implemented");
  return 0;
}

int32_t Fsetsockopt(int16_t fd, int32_t level, int32_t name, void *val, uint32_t valsize) {
  debug(DEBUG_ERROR, "TOS", "Fsetsockopt not implemented");
  return 0;
}

int32_t Fgetsockopt(int16_t fd, int32_t level, int32_t name, void *val, uint32_t *valsize) {
  debug(DEBUG_ERROR, "TOS", "Fgetsockopt not implemented");
  return 0;
}

int32_t Fgetpeername(int16_t fd, struct sockaddr *asa, uint32_t *alen) {
  debug(DEBUG_ERROR, "TOS", "Fgetpeername not implemented");
  return 0;
}

int32_t Fgetsockname(int16_t fd, struct sockaddr *asa, uint32_t *alen) {
  debug(DEBUG_ERROR, "TOS", "Fgetsockname not implemented");
  return 0;
}

int32_t Fshutdown(int16_t fd, int32_t how) {
  debug(DEBUG_ERROR, "TOS", "Fshutdown not implemented");
  return 0;
}

int32_t Maccess(void *start, int32_t size, int16_t mode) {
  debug(DEBUG_ERROR, "TOS", "Maccess not implemented");
  return 0;
}

int32_t Fchown16(char *name, int16_t uid, int16_t gid, int16_t flag) {
  debug(DEBUG_ERROR, "TOS", "Fchown16 not implemented");
  return 0;
}

int32_t Fchdir(int16_t handle) {
  debug(DEBUG_ERROR, "TOS", "Fchdir not implemented");
  return 0;
}

int32_t Ffdopendir(int16_t fd) {
  debug(DEBUG_ERROR, "TOS", "Ffdopendir not implemented");
  return 0;
}

int16_t Fdirfd(int32_t handle) {
  debug(DEBUG_ERROR, "TOS", "Fdirfd not implemented");
  return 0;
}

int32_t Dxopendir(char *name, int16_t flag) {
  debug(DEBUG_ERROR, "TOS", "Dxopendir not implemented");
  return 0;
}
