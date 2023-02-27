#include "sys.h"
#include "script.h"
#include "thread.h"
#include "mutex.h"
#include "vfs.h"
#include "ptr.h"
#include "filter.h"
#include "shell.h"
#include "pit_io.h"
#include "telnet.h"
#include "login.h"
#include "sock.h"
#include "ts.h"
#include "timeutc.h"
#include "debug.h"
#include "xalloc.h"

#define TAG_SHELL    "SHELL"

#define MAX_LINE     256
#define MAX_BUF      1024

#define MAX_ARGS     8

#define MAX_CMDS     64
#define MAX_HISTORY  64

#define MAX_LOGIN    16
#define MAX_PASSWORD 16

struct shell_t {
  char *tag;
  int pe;
  script_ref_t ref;
  vfs_session_t *session;
  int try, logout;
  int cols, rows;
  int fd;
  vfs_file_t *fin, *fout;
  io_addr_t addr;
  char buf[MAX_BUF];
  char line[MAX_LINE];
  int linelen, linepos;
  char *history[MAX_HISTORY];
  int nhistory, ihistory, phistory;
  int used_history;
  void *orig;
  char login[MAX_LOGIN];
  char password[MAX_PASSWORD];
  int dotelnet;
  conn_filter_t *filter;
};

typedef struct {
  script_ref_t ref;
} cmd_data_t;

typedef struct {
  int pe;
  script_ref_t eval_ref;
  script_ref_t peek_ref;
  script_ref_t read_ref;
  script_ref_t write_ref;
} local_data_t;

static shell_provider_t provider;
static mutex_t *mutex;
static int ncmds;

static char *CLS       = "\x1B[H\x1B[2J";
static char *CRLF      = "\r\n";
static char *PROMPT    = "\x1B[33mpit> \x1B[37m";  // 33 = yellow
static char *ERASELINE = "\r\x1B[K";

static char *CURSOR_UP    = "\x1B[A";
static char *CURSOR_DOWN  = "\x1B[B";
static char *CURSOR_LEFT  = "\x1B[D";
static char *CURSOR_RIGHT = "\x1B[C";

static char *PF1  = "\x1BOP";
static char *PF2  = "\x1BOQ";
static char *PF3  = "\x1BOR";
static char *PF4  = "\x1BOS";
//static char *PF6  = "\x1B[17~";
//static char *PF7  = "\x1B[18~";
//static char *PF8  = "\x1B[19~";
//static char *PF9  = "\x1B[20~";
//static char *PF10 = "\x1B[21~";

static int libshell_peek(shell_t *shell, uint32_t us) {
  int r;

  if (shell->fin) {
    r = vfs_peek(shell->fin, us);
  } else {
    r = shell->filter->peek(shell->filter, us);
  }

  return r;
}

static int libshell_read(shell_t *shell, char *buf, int len) {
  uint8_t b;
  int nread, r;

  if (shell->fin) {
    r = vfs_read(shell->fin, (uint8_t *)buf, len);
  } else {
    if ((r = shell->filter->peek(shell->filter, -1)) == 1) {
      for (nread = 0; nread < len;) {
        r = shell->filter->peek(shell->filter, 0);
        if (r < 0) return -1;
        if (r == 0) return nread;
        r = shell->filter->read(shell->filter, &b);
        if (r <= 0) return -1;
        buf[nread++] = b;
      }
      r = nread;
    }
  }

  return r;
}

static char *libshell_gets(shell_t *shell, char *buf, int len) {
  int i, n;
  char c;

  for (i = 0; i < len-1;) {
    n = libshell_peek(shell, 0);
    if (n <= 0) break;
    n = libshell_read(shell, &c, 1);
    if (n <= 0) break;
    buf[i++] = c;
    if (c == '\n') break;
  }
  if (i) buf[i] = 0;

  return i ? buf : NULL;
}

static int libshell_write_aux(shell_t *shell, int err, char *buf, int len) {
  int r;

  if (len == -1) {
    len = sys_strlen(buf);
  }

  if (shell->fout && !err) {
    r = vfs_write(shell->fout, (uint8_t *)buf, len);
  } else {
    r = shell->filter->write(shell->filter, (uint8_t *)buf, len);
  }

  return r;
}

static int libshell_write(shell_t *shell, char *buf, int len) {
  return libshell_write_aux(shell, 0, buf, len);
}

static void libshell_print(shell_t *shell, int err, char *fmt, ...) {
  sys_va_list ap;

  sys_va_start(ap, fmt);
  sys_vsnprintf(shell->buf, MAX_BUF, fmt, ap);
  sys_va_end(ap);

  if (err) debug(DEBUG_ERROR, "SHELL", shell->buf);
  libshell_write_aux(shell, err, shell->buf, -1);
}

static int cmd_help(shell_t *shell, vfs_session_t *session, int pe, int argc, char *argv[], void *data);

static int cmd_exit(shell_t *shell, vfs_session_t *session, int pe, int argc, char *argv[], void *data) {
  shell->logout = 1;
  return 0;
}

static void libshell_cls(shell_t *shell) {
  libshell_write(shell, CLS, -1);
}

static void libshell_prompt(shell_t *shell) {
  libshell_write(shell, PROMPT, -1);
}

static int libshell_script_call(int pe, script_ref_t ref, unsigned char *buf, int len, script_arg_t *ret) {
  script_lstring_t lstr;

  lstr.s = (char *)buf;
  lstr.n = len;

  return script_call(pe, ref, ret, "L", &lstr);
}

static char *libshell_ret(int pe, script_arg_t *ret) {
  char buf[64], *val;

  switch (ret->type) {
    case SCRIPT_ARG_INTEGER:
      sys_snprintf(buf, sizeof(buf)-1, "%d", ret->value.i);
      val = xstrdup(buf);
      break;
    case SCRIPT_ARG_REAL:
      sys_snprintf(buf, sizeof(buf)-1, "%f", ret->value.d);
      val = xstrdup(buf);
      break;
    case SCRIPT_ARG_BOOLEAN:
      sys_strcpy(buf, ret->value.i ? "true" : "false");
      val = xstrdup(buf);
      break;
    case SCRIPT_ARG_STRING:
      val = ret->value.s;
      break;
    case SCRIPT_ARG_LSTRING:
      val = xcalloc(1, ret->value.l.n + 1);
      if (val) {
        xmemcpy(val, ret->value.l.s, ret->value.l.n);
      }
      xfree(ret->value.l.s);
      break;
    case SCRIPT_ARG_OBJECT:
      sys_strcpy(buf, "<object>");
      val = xstrdup(buf);
      script_remove_ref(pe, ret->value.r);
      break;
    case SCRIPT_ARG_FUNCTION:
      sys_strcpy(buf, "<function>");
      val = xstrdup(buf);
      script_remove_ref(pe, ret->value.r);
      break;
    case SCRIPT_ARG_FILE:
      sys_strcpy(buf, "<file>");
      val = xstrdup(buf);
      break;
    case SCRIPT_ARG_NULL:
      val = NULL;
      break;
    default:
      sys_snprintf(buf, sizeof(buf)-1, "type(%c)", ret->type);
      val = xstrdup(buf);
      break;
  }

  return val;
}

static char *libshell_eval(shell_t *shell, char *s) {
  script_arg_t ret;
  char *val = NULL;

  if (libshell_script_call(shell->pe, shell->ref, (uint8_t *)s, sys_strlen(s), &ret) == 0) {
    val = libshell_ret(shell->pe, &ret);
  }

  return val;
}

static int cmd_date(shell_t *shell, vfs_session_t *session, int pe, int argc, char *argv[], void *data) {
  int day, month, year, wday, hour, min, sec;
  uint64_t t;

  t = sys_time();
  ts2time(t, &day, &month, &year, &wday, &hour, &min, &sec);
  libshell_print(shell, 0, "%04d-%02d-%02d %02d:%02d:%02d UTC\r\n", year, month, day, hour, min, sec);

  return 0;
}

static int cmd_cls(shell_t *shell, vfs_session_t *session, int pe, int argc, char *argv[], void *data) {
  libshell_cls(shell);
  return 0;
}

static int cmd_pid(shell_t *shell, vfs_session_t *session, int pe, int argc, char *argv[], void *data) {
  libshell_print(shell, 0, "%u\r\n", sys_get_pid());
  return 0;
}

static int cmd_tid(shell_t *shell, vfs_session_t *session, int pe, int argc, char *argv[], void *data) {
  libshell_print(shell, 0, "%u\r\n", sys_get_tid());
  return 0;
}

static int cmd_ps(shell_t *shell, vfs_session_t *session, int pe, int argc, char *argv[], void *data) {
  thread_ps_t *ps;
  int i, r = -1;

  if ((ps = thread_ps()) != NULL) {
    libshell_write(shell, "  Tid  Handle  Load  Name\r\n", -1);

    for (i = 0; ps[i].tid; i++) {
      libshell_print(shell, 0, "%5u   %5u  %4.2f  ", ps[i].tid, ps[i].handle, ps[i].p);
      if (ps[i].name) {
        libshell_write(shell, ps[i].name, -1);
        xfree(ps[i].name);
      } else {
        libshell_write(shell, "???", -1);
      }
      libshell_write(shell, CRLF, -1);
    }
    xfree(ps);
    r = 0;

  } else {
    libshell_print(shell, 1, "ps failed\r\n");
  }

  return r;
}

static int cmd_kill(shell_t *shell, vfs_session_t *session, int pe, int argc, char *argv[], void *data) {
  int handle, r;

  handle = sys_atoi(argv[1]);

  if (handle == 0 || !sys_strcmp(argv[2], "MAIN")) {
    libshell_write(shell, "Can not kill main thread.\r\n", -1);

  } else {
    if (thread_end(argv[2], handle) == 0) {
      r = 0;
    } else {
      libshell_print(shell, 1, "kill failed\r\n");
    }
  }

  return r;
}

static int cmd_pwd(shell_t *shell, vfs_session_t *session, int pe, int argc, char *argv[], void *data) {
  char *reply = vfs_cwd(session);
  libshell_write(shell, reply, -1);
  libshell_write(shell, CRLF, -1);
  return 0;
}

static int cmd_cd(shell_t *shell, vfs_session_t *session, int pe, int argc, char *argv[], void *data) {
  char *dir;
  int r;

  if (argc == 1) {
    dir = "/";
  } else {
    dir = argv[1];
  }

  r = vfs_chdir(session, dir);
  if (r == -1) {
    libshell_print(shell, 1, "Error changing directory to \"%s\"\r\n", dir);
  }

  return r;
}

static int cmd_ls(shell_t *shell, vfs_session_t *session, int pe, int argc, char *argv[], void *data) {
  int day, month, year, wday, hour, min, sec;
  vfs_dir_t *dir;
  vfs_ent_t *ent;
  char *path;
  int r = -1;

  if (argc == 1) {
    path = ".";
  } else {
    path = argv[1];
  }
  debug(DEBUG_TRACE, "SHELL", "list [%s]", path);

  if ((dir = vfs_opendir(session, path)) != NULL) {
    for (;;) {
      ent = vfs_readdir(dir);
      if (ent == NULL) break;
      debug(DEBUG_TRACE, "SHELL", "list entry [%s]", ent->name);
      ts2time(ent->mtime, &day, &month, &year, &wday, &hour, &min, &sec);
      libshell_print(shell, 0, "%c%c%c %6d %04d-%02d-%02d %02d:%02d %s\r\n",
        ent->type == VFS_DIR ? 'd' : ' ', ent->rd ? 'r' : '-', ent->wr ? 'w' : '-',
        ent->size, year, month, day, hour, min, ent->name);
    }
    vfs_closedir(dir);
    debug(DEBUG_TRACE, "SHELL", "list end");
    r = 0;

  } else {
    libshell_print(shell, 1, "Error opening directory \"%s\"\r\n", path);
  }

  return r;
}

static int cmd_rm(shell_t *shell, vfs_session_t *session, int pe, int argc, char *argv[], void *data) {
  int r;

  if ((r = vfs_unlink(session, argv[1])) == -1) {
    libshell_print(shell, 1, "Error removing \"%s\"\r\n", argv[1]);
  }

  return r;
}

static int cmd_echo(shell_t *shell, vfs_session_t *session, int pe, int argc, char *argv[], void *data) {
  libshell_write(shell, argv[1], sys_strlen(argv[1]));
  return 0;
}

static int cmd_type(shell_t *shell, vfs_session_t *session, int pe, int argc, char *argv[], void *data) {
  vfs_file_t *file;
  int i, k, n, r = -1;

  debug(DEBUG_TRACE, "SHELL", "type [%s]", argv[1]);
  if ((file = vfs_open(session, argv[1], VFS_READ)) != NULL) {
    for (;;) {
      n = vfs_read(file, (uint8_t *)shell->buf, MAX_BUF);
      if (n <= 0) break;
      debug(DEBUG_TRACE, "SHELL", "type read %d bytes", n);

      if (shell->fout) {
        // pass characters dirctly to fout
        libshell_write(shell, shell->buf, n);

      } else {
        // convert LF -> CR+LF
        for (i = 0, k = 0; i < n; i++) {
          if (shell->buf[i] == '\n') {
            libshell_write(shell, &shell->buf[k], i-k);
            libshell_write(shell, CRLF, -1);
            k = i+1;
          }
        }
        if (i > k) {
          libshell_write(shell, &shell->buf[k], i-k);
        }
      }
    }
    debug(DEBUG_TRACE, "SHELL", "type end");
    vfs_close(file);
    r = 0;

  } else {
    libshell_print(shell, 1, "Error opening file \"%s\"\r\n", argv[1]);
  }

  return r;
}

static int cmd_write(shell_t *shell, vfs_session_t *session, int pe, int argc, char *argv[], void *data) {
  vfs_file_t *file;
  int n, m, r = -1;

  debug(DEBUG_TRACE, "SHELL", "write [%s]", argv[1]);

  if ((file = vfs_open(session, argv[1], VFS_WRITE)) != NULL) {
    for (;;) {
      n = libshell_peek(shell, 0);
      if (n <= 0) break;
      n = libshell_read(shell, shell->buf, MAX_BUF);
      if (n <= 0) break;
      debug(DEBUG_TRACE, "SHELL", "write read %d bytes", n);
      m = vfs_write(file, (uint8_t *)shell->buf, n);
      if (n != m) break;
    }
    debug(DEBUG_TRACE, "SHELL", "write end");
    vfs_close(file);

  } else {
    libshell_print(shell, 1, "Error opening file \"%s\"\r\n", argv[1]);
  }

  return r;
}

static int cmd_dump(shell_t *shell, vfs_session_t *session, int pe, int argc, char *argv[], void *data) {
  vfs_file_t *file;
  uint8_t buf[16];
  char aux[16], hbuf[128];
  uint32_t offset, nread, len, i0;
  int hex, i, j, k, n, r = -1;

  hex = 1;
  file = NULL;
  offset = 0;
  len = -1;

  if (argc == 1) {
    debug(DEBUG_TRACE, "SHELL", "dump stdin");
    i0 = 0;

  } else {
    debug(DEBUG_TRACE, "SHELL", "dump file [%s]", argv[1]);
    if ((file = vfs_open(session, argv[1], VFS_READ)) == NULL) {
      libshell_print(shell, 1, "Error opening file \"%s\"\r\n", argv[1]);
      return -1;
    }
    if (argc > 2) {
      if (!sys_strncmp(argv[2], "0x", 2)) {
        offset = sys_strtoul(&argv[2][2], NULL, 16);
      } else {
        offset = sys_atoi(argv[2]);
      }
      debug(DEBUG_TRACE, "SHELL", "dump offset %u", offset);
      if (vfs_seek(file, offset, 0) == -1) {
        libshell_print(shell, 1, "Error seeking file \"%s\"\r\n", argv[1]);
        return -1;
      }
    }
    if (argc > 3) {
      if (!sys_strncmp(argv[3], "0x", 2)) {
        len = sys_strtoul(&argv[3][2], NULL, 16);
      } else {
        len = sys_atoi(argv[3]);
      }
      debug(DEBUG_TRACE, "SHELL", "dump length %u", len);
    }
    if (argc > 4) {
      hex = argv[4][0] == 'h' || argv[4][0] == 'H';
    }
    i0 = offset & 0xF;
  }

  for (nread = 0; nread <= len;) {
    if (file) {
      n = 16 - i0;
      n = ((nread + n) <= len) ? n : len - nread;
      n = vfs_read(file, &buf[i0], n);
    } else {
      n = libshell_peek(shell, 0);
      if (n > 0) n = libshell_read(shell, (char *)buf, 16);
    }

    if (n < 0) break;

    if (n == 0) {
      r = 0;
      break;
    }

    xmemset(hbuf, ' ', sizeof(hbuf));
    sys_sprintf(aux, "%08X", offset & 0xFFFFFFF0);
    offset += n;
    nread += n;

    if (hex) {
      j = 0;
      hbuf[j++] = aux[0];
      hbuf[j++] = aux[1];
      hbuf[j++] = aux[2];
      hbuf[j++] = aux[3];
      hbuf[j++] = aux[4];
      hbuf[j++] = aux[5];
      hbuf[j++] = aux[6];
      hbuf[j++] = aux[7];
      hbuf[j++] = ' ';
      hbuf[j++] = ' ';

      for (i = 0, k = j + 8*3 + 1 + 8*3 + 1; i < i0+n; i++) {
        sys_sprintf(aux, "%02X", buf[i]);
        hbuf[j++] = i >= i0 ? aux[0] : ' ';
        hbuf[j++] = i >= i0 ? aux[1] : ' ';
        hbuf[j++] = ' ';
        if (i == 7) hbuf[j++] = ' ';
        hbuf[k++] = i >= i0 ? (buf[i] < 32 || buf[i] >= 127 ? '.' : buf[i]) : ' ';
      }
      for (; i < 16; i++) {
        hbuf[j++] = ' ';
        hbuf[j++] = ' ';
        hbuf[j++] = ' ';
        if (i == 7) hbuf[j++] = ' ';
        hbuf[k++] = ' ';
      }
      hbuf[k] = 0;
      libshell_write(shell, hbuf, -1);
      libshell_write(shell, CRLF, -1);

    } else {
      libshell_write(shell, (char *)buf, n);
    }
    i0 = 0;
  }
  debug(DEBUG_TRACE, "SHELL", "dump end");
  if (file) vfs_close(file);

  return r;
}

static int cmd_wc(shell_t *shell, vfs_session_t *session, int pe, int argc, char *argv[], void *data) {
  vfs_file_t *file;
  char buf[MAX_LINE], *s;
  int count, r = -1;

  if (argc == 1) {
    debug(DEBUG_TRACE, "SHELL", "wc stdin");
    file = NULL;
  } else {
    debug(DEBUG_TRACE, "SHELL", "wc [%s]", argv[1]);
    if ((file = vfs_open(session, argv[1], VFS_READ)) == NULL) {
      libshell_print(shell, 1, "Error opening file \"%s\"\r\n", argv[1]);
      return -1;
    }
  }

  for (count = 0;; count++) {
    if (file) {
      s = vfs_gets(file, buf, MAX_LINE);
    } else {
      s = libshell_gets(shell, buf, MAX_LINE);
    }

    if (s == NULL) {
      r = 0;
      break;
    }
  }

  libshell_print(shell, 0, "%d\r\n", count);

  return r;
}

static int cmd_cp(shell_t *shell, vfs_session_t *session, int pe, int argc, char *argv[], void *data) {
  vfs_file_t *file1, *file2;
  int n, m, r = -1;

  debug(DEBUG_TRACE, "SHELL", "cp [%s] [%s]", argv[1], argv[2]);
  if ((file1 = vfs_open(session, argv[1], VFS_READ)) != NULL) {
    if ((file2 = vfs_open(session, argv[2], VFS_WRITE | VFS_TRUNC)) != NULL) {
      for (;;) {
        if ((n = vfs_read(file1, (uint8_t *)shell->buf, MAX_BUF)) <= 0) {
          if (n == 0) r = 0;
          break;
        }
        if ((m = vfs_write(file2, (uint8_t *)shell->buf, n)) != n) {
          if (m != -1) {
            debug(DEBUG_TRACE, "SHELL", "wrote fewer bytes");
          }
          break;
        }
      }
      debug(DEBUG_TRACE, "SHELL", "cp end");
      vfs_close(file2);
  
    } else {
      libshell_print(shell, 1, "Error opening file \"%s\"\r\n", argv[2]);
    }
    vfs_close(file1);

  } else {
    libshell_print(shell, 1, "Error opening file \"%s\"\r\n", argv[1]);
  }

  return r;
}

static int libshell_window(shell_t *shell, int *cols, int *rows) {
  *cols = shell->cols;
  *rows = shell->rows;

  if (*cols <= 0 || *rows <= 0) {
    *cols = 80;
    *rows = 25;
  }

  return 0;
}

static int cmd_window(shell_t *shell, vfs_session_t *session, int pe, int argc, char *argv[], void *data) {
  int cols, rows;

  cols = sys_atoi(argv[1]);
  rows = sys_atoi(argv[2]);

  if (cols > 0 && rows > 0) {
    shell->cols = cols;
    shell->rows = rows;
    debug(DEBUG_TRACE, "SHELL", "setting window size %dx%d", shell->cols, shell->rows);
  }

  return 0;
}

static int cmd_env(shell_t *shell, vfs_session_t *session, int pe, int argc, char *argv[], void *data) {
  char *value;

  value = sys_getenv(argv[1]);

  if (value) {
    libshell_write(shell, value, -1);
  }
  libshell_write(shell, CRLF, -1);

  return 0;
}

static int cmd_finish(shell_t *shell, vfs_session_t *session, int pe, int argc, char *argv[], void *data) {
  int code;

  code = sys_atoi(argv[1]);
  sys_set_finish(code);

  return 0;
}

static shell_command_t cmd[MAX_CMDS] = {
  { "help",   "help [ <command> ]",   0, 1, cmd_help,   "List available commands.", NULL },
  { "exit",   "exit",                 0, 0, cmd_exit,   "Close session.", NULL },
  { "date",   "date",                 0, 0, cmd_date,   "Show system date and time in UTC.", NULL },
  { "cls",    "cls",                  0, 0, cmd_cls,    "Clear screen.", NULL },
  { "pid",    "pid",                  0, 0, cmd_pid,    "Show current pid (process id).", NULL },
  { "tid",    "tid",                  0, 0, cmd_tid,    "Show current tid (thread id).", NULL },
  { "ps",     "ps",                   0, 0, cmd_ps,     "List active threads.", NULL },
  { "kill",   "kill <handle> <name>", 2, 2, cmd_kill,   "Kill a thread.", NULL },
  { "pwd",    "pwd",                  0, 0, cmd_pwd,    "Show current directory.", NULL },
  { "cd",     "cd [ <dir> ]",         0, 1, cmd_cd,     "Change directory.", NULL },
  { "ls",     "ls [ <dir> ]",         0, 1, cmd_ls,     "List directory.", NULL },
  { "rm",     "rm <file> | <dir>",    1, 1, cmd_rm,     "Delete file or directory.", NULL },
  { "cp",     "cp <src> <dst>",       2, 2, cmd_cp,     "Copy source file to destination file.", NULL },
  { "echo",   "echo <arg>",           1, 1, cmd_echo,   "Write argument to output.", NULL },
  { "type",   "type <file>",          1, 1, cmd_type,   "Write file contents to output.", NULL },
  { "write",  "write <file>",         1, 1, cmd_write,  "Write input to file.", NULL },
  { "dump",   "dump [ <file> <offs> <len> h|b ]",      0, 4, cmd_dump,   "Dump file contents.", NULL },
  { "wc",     "wc [ <file> ]",        0, 1, cmd_wc,     "Count lines of file.", NULL },
  { "window", "window <cols> <rows>", 2, 2, cmd_window, "Forces number of columns and rows.", NULL },
  { "env",    "env <var>",            1, 1, cmd_env,    "Show the value of an environment variable.", NULL },
  { "finish", "finish <code>",        1, 1, cmd_finish, "Terminate with status code.", NULL },
  { NULL,     NULL,                   0, 0, NULL,       NULL, NULL }
};

static int cmd_help(shell_t *shell, vfs_session_t *session, int pe, int argc, char *argv[], void *data) {
  int i;

  if (mutex_lock(mutex) == 0) {
    if (argc == 1) {
      libshell_write(shell, "Available commands:\r\n\r\n", -1);

      for (i = 0; i < ncmds; i++) {
        libshell_write(shell, cmd[i].name, -1);
        libshell_write(shell, "\t", 1);
        libshell_write(shell, cmd[i].help, -1);
        libshell_write(shell, CRLF, -1);
      }

      libshell_write(shell, "\r\nUse help <command> for more information\r\n", -1);

    } else {
      for (i = 0; i < ncmds; i++) {
        if (!sys_strcmp(argv[1], cmd[i].name)) {
          libshell_write(shell, cmd[i].help, -1);
          libshell_write(shell, CRLF, -1);
          libshell_write(shell, "Usage: ", -1);
          libshell_write(shell, cmd[i].usage, -1);
          libshell_write(shell, CRLF, -1);
          break;
        }
      }

      if (!cmd[i].name) {
        libshell_write(shell, "There is no such command\r\n", -1);
      }
    }
    mutex_unlock(mutex);
  }

  return 0;
}

static char escape_char(char c) {
  switch (c) {
    case '\\': c = '\\'; break;
    case '"':  c = '"'; break;
    case 'b':  c = 8; break;
    case 't':  c = 9; break;
    case 'n':  c = 10; break;
    case 'r':  c = 13; break;
    default:   c = 0;
  }

  return c;
}

static int libshell_findargs(shell_t *shell, char *buf, char *argv[]) {
  char aux[MAX_LINE], expr[MAX_LINE], *reply;
  int s, i, j, k, e, prev, argc;

  for (s = 0, i = 0, j = 0, argc = 0; buf[i]; i++) {
    switch (s) {
      case 0:
        switch (buf[i]) {
          case ' ':
            break;
          case '"':
            s = 2;
            break;
          case '`':
            e = 0;
            s = 4;
            break;
          case '\\':
            prev = 1;
            s = 3;
            break;
          default:
            aux[j++] = buf[i];
            s = 1;
            break;
        }
        break;

      case 1:
        switch (buf[i]) {
          case ' ':
            aux[j] = 0;
            j = 0;
            if (argc < MAX_ARGS) {
              argv[argc++] = xstrdup(aux);
            }
            s = 0;
            break;
          case '`':
            e = 0;
            s = 4;
            break;
          case '\\':
            prev = 1;
            s = 3;
            break;
          default:
            if (j < MAX_LINE-1) {
              aux[j++] = buf[i];
            }
            break;
        }
        break;

      case 2:
        switch (buf[i]) {
          case '"':
            aux[j] = 0;
            j = 0;
            if (argc < MAX_ARGS) {
              argv[argc++] = xstrdup(aux);
            }
            s = 0;
            break;
          case '\\':
            prev = 2;
            s = 3;
            break;
          default:
            if (j < MAX_LINE-1) {
              aux[j++] = buf[i];
            }
            break;
        }
        break;

      case 3:
        if (j < MAX_LINE-1) {
          aux[j] = escape_char(buf[i]);
          if (aux[j]) j++;
        }
        s = prev;
        break;

      case 4:
        switch (buf[i]) {
          case '`':
            expr[e] = 0;
            reply = libshell_eval(shell, expr);
            if (reply) {
              for (k = 0; reply[k] && j < MAX_LINE-1; k++) {
                aux[j++] = reply[k];
              }
              xfree(reply);
            }
            s = 1;
            break;
          case '\\':
            s = 5;
            break;
          default:
            if (e < MAX_LINE-1) {
              expr[e++] = buf[i];
            }
            break;
        }
        break;

      case 5:
        if (e < MAX_LINE-1) {
          expr[e] = escape_char(buf[i]);
          if (expr[e]) e++;
        }
        s = 4;
        break;
    }
  }

  if (j) {
    if (s == 1) {
      aux[j] = 0;
      if (argc < MAX_ARGS) {
        argv[argc++] = xstrdup(aux);
      }
    } else {
      debug(DEBUG_ERROR, "SHELL", "missing end quote");
      for (i = 0; i < argc; i++) {
        if (argv[i]) xfree(argv[i]);
      }
      argc = 0;
    }
  }

  return argc;
}

static int libshell_cmd(shell_t *shell, char *line) {
  char *argv[MAX_ARGS];
  int argc, i, r = -1;

  debug(DEBUG_TRACE, "SHELL", "libshell_cmd(\"%s\")", line);

  argc = libshell_findargs(shell, line, argv);
  if (argc) {
    for (i = 0; i < argc; i++) {
      debug(DEBUG_TRACE, "SHELL", "argv[%d] = \"%s\"", i, argv[i]);
    }

    if (mutex_lock(mutex) == 0) {
      for (i = 0; i < ncmds; i++) {
        if (!sys_strcmp(argv[0], cmd[i].name)) {
          if ((argc-1) < cmd[i].minargs || (argc-1) > cmd[i].maxargs) {
            libshell_write(shell, "Usage: ", -1);
            libshell_write(shell, cmd[i].usage, -1);
            libshell_write(shell, CRLF, -1);
          } else {
            debug(DEBUG_TRACE, "SHELL", "executing command \"%s\"", argv[0]);
            r = cmd[i].cmd(shell, shell->session, shell->pe, argc, argv, cmd[i].data);
          }
          break;
        }
      }
      if (!cmd[i].name) {
        libshell_print(shell, 1, "Unknown command \"%s\"\r\n", argv[0]);
      }
      mutex_unlock(mutex);
    }
    for (i = 0; i < argc; i++) {
      if (argv[i]) xfree(argv[i]);
    }
  } else {
    libshell_print(shell, 1, "Syntax error\r\n");
  }

  return r;
}

static int libshell_add_history(shell_t *shell, char *line) {
  char *s;
  int r = -1;

  if ((s = xstrdup(line)) != NULL) {
    if (shell->nhistory < MAX_HISTORY) {
      shell->history[shell->nhistory] = s;
      shell->ihistory = shell->nhistory;
      shell->nhistory++;
      r = 0;

    } else {
      if (shell->history[shell->phistory]) xfree(shell->history[shell->phistory]);
      shell->history[shell->phistory] = s;
      shell->ihistory = shell->phistory;
      shell->phistory++;
      if (shell->phistory == MAX_HISTORY) shell->phistory = 0;
      r = 0;
    }
  }

  return r;
}

static char *libshell_get_history(shell_t *shell, int up) {
  char *s = NULL;

  if (shell->nhistory) {
    if (up) {
      shell->ihistory--;
      if (shell->ihistory < 0) shell->ihistory = shell->nhistory - 1;
    } else {
      shell->ihistory++;
      if (shell->ihistory == shell->nhistory) shell->ihistory = 0;
    }

    s = shell->history[shell->ihistory];

    if (s) {
      debug(DEBUG_TRACE, "SHELL", "history %d [%s]", shell->ihistory, s);
    }
  }

  return s;
}

static void libshell_line(shell_t *shell, char *line) {
  char *p, *in, *out, *tmp;
  char tmp1[16], tmp2[16];
  int s, i, err, usepipe;

  libshell_write(shell, CRLF, -1);

  if (line[0]) {
    if (shell->used_history) {
      shell->used_history = 0;
    } else {
      libshell_add_history(shell, line);
    }

    sys_snprintf(tmp1, sizeof(tmp1), "%c_tmp1", FILE_SEP);
    sys_snprintf(tmp2, sizeof(tmp2), "%c_tmp2", FILE_SEP);

    for (i = 0, s = 0, p = line, usepipe = 0, err = 0, in = tmp1, out = tmp2; p[i] && !err;) {
      switch (s) {
        case 0:
          switch (p[i]) {
            case '\\':
              s = 1;
              i++;
              break;
            case '|':
              if (usepipe) {
                shell->fin = vfs_open(shell->session, in, VFS_READ);
              }
              shell->fout = vfs_open(shell->session, out, VFS_WRITE | VFS_TRUNC);
              p[i] = 0;
              if (libshell_cmd(shell, p) == -1) {
                err = 1;
              }
              p = &p[i+1];
              i = 0;
              if (shell->fin) vfs_close(shell->fin);
              if (shell->fout) vfs_close(shell->fout);
              shell->fin = NULL;
              shell->fout = NULL;
              tmp = out;
              out = in;
              in = tmp;
              usepipe = 1;
              break;
            default:
              i++;
              break;
          }
          break;

        case 1:
          s = 0;
          i++;
          break;
      }
    }
    shell->linelen = 0;
    shell->linepos = 0;

    if (!err) {
      if (usepipe) {
        shell->fin = vfs_open(shell->session, in, VFS_READ);
      }
      if (libshell_cmd(shell, p) == -1) {
        err = 1;
      }
      if (shell->fin) vfs_close(shell->fin);
      shell->fin = NULL;
    }

    vfs_unlink(shell->session, in);
    vfs_unlink(shell->session, out);
  }
  if (!shell->logout) libshell_prompt(shell);
}

static int libshell_escape_sequence(shell_t *shell, char *seq) {
  char *s = NULL;

 debug(DEBUG_TRACE, "SHELL", "escape sequence [%s]", seq);

  if (!sys_strcmp(seq, CURSOR_UP)) {
    debug(DEBUG_TRACE, "SHELL", "cursor up");
    s = libshell_get_history(shell, 1);

  } else if (!sys_strcmp(seq, CURSOR_DOWN)) {
    debug(DEBUG_TRACE, "SHELL", "cursor down");
    s = libshell_get_history(shell, 0);

  } else if (!sys_strcmp(seq, CURSOR_LEFT)) {
    debug(DEBUG_TRACE, "SHELL", "cursor left");
    if (shell->linepos) {
      libshell_write(shell, "\b", 1);
      shell->linepos--;
    }

  } else if (!sys_strcmp(seq, CURSOR_RIGHT)) {
    debug(DEBUG_TRACE, "SHELL", "cursor right");
    if (shell->linepos < shell->linelen) {
      libshell_write(shell, &shell->line[shell->linepos], 1);
      shell->linepos++;
    }

  } else if (!sys_strcmp(seq, PF1)) {
    debug(DEBUG_TRACE, "SHELL", "PF1");
    libshell_write(shell, CRLF, -1);
    cmd_help(shell, shell->session, shell->pe, 1, NULL, NULL);
    libshell_prompt(shell);

  } else if (!sys_strcmp(seq, PF2)) {
    debug(DEBUG_TRACE, "SHELL", "PF2");

  } else if (!sys_strcmp(seq, PF3)) {
    debug(DEBUG_TRACE, "SHELL", "PF3");

  } else if (!sys_strcmp(seq, PF4)) {
    debug(DEBUG_TRACE, "SHELL", "PF4");
  }

  if (s) {
    libshell_write(shell, ERASELINE, -1);
    libshell_prompt(shell);
    sys_strncpy(shell->line, s, MAX_LINE-1);
    shell->linelen = sys_strlen(s);
    shell->linepos = shell->linelen;
    libshell_write(shell, shell->line, shell->linelen);
    shell->used_history = 1;
  }

  return 0;
}

static void libshell_buildline(shell_t *shell, uint8_t *buf, int n) {
  char seq[8];
  int e, i, j;

  debug(DEBUG_TRACE, "SHELL", "received [%.*s]", n, (char *)buf);

  for (i = 0, e = 0; i < n; i++) {
    switch (buf[i]) {
      case 8:    // backspace
      case 127:  // delete
        debug(DEBUG_TRACE, "SHELL", "backspace 0x%02X", buf[i]);
        if (shell->linepos) {
          if (shell->linepos == shell->linelen) {
            libshell_write(shell, "\b \b", -1);
            shell->linelen--;
            shell->linepos--;

          } else {
            libshell_write(shell, "\b\x1B[P", -1);
            shell->linelen--;
            shell->linepos--;
            for (j = shell->linepos; j < shell->linelen; j++) {
              shell->buf[j] = shell->buf[j+1];
            }
          }
        }
        break;

      case 10:   // LF
        break;

      case 13:   // CR
        shell->line[shell->linepos] = 0;
        libshell_line(shell, shell->line);
        shell->linelen = 0;
        shell->linepos = 0;
        break;

      case 27:   // ESC
        seq[e++] = buf[i];
        break;

      default:
        switch (e) {
          case 0:
            debug(DEBUG_TRACE, "SHELL", "echoing [%c]", buf[i]);
            if (buf[i] < 32) {
              libshell_print(shell, 0, "<%02X>", buf[i]);
            } else {
              libshell_write(shell, (char *)&buf[i], 1);
            }
            if (shell->linepos < MAX_LINE-1) {
              shell->line[shell->linepos] = (char)buf[i];
              if (shell->linelen == shell->linepos) shell->linelen++;
              shell->linepos++;
            }
            break;
          case 1:
            if (buf[i] == '[' || buf[1] == 'O') {
              seq[e++] = buf[i];
            } else {
              e = 0;
            }
            break;
          default:
            if (buf[i] >= '0' && buf[i] <= '9') {
              if (e < 7) seq[e++] = buf[i];
            } else {
              if (e < 7) seq[e++] = buf[i];
              seq[e] = 0;
              libshell_escape_sequence(shell, seq);
              e = 0;
            }
            break;
        }
        break;
    }
  }
}

static int libshell_loop(int fd, int ptr) {
  shell_t *shell;
  shell_t newshell;
  conn_filter_t *telnet;
  conn_filter_t *sock;
  uint8_t buf[256];
  int cols, rows;
  int n, i;

  if ((shell = ptr_lock(ptr, TAG_SHELL)) == NULL) {
    return -1;
  }

  xmemset(&newshell, 0, sizeof(shell_t));
  newshell.pe = shell->pe;
  newshell.ref = script_dup_ref(shell->pe, shell->ref);
  newshell.dotelnet = shell->dotelnet;
  sys_strncpy(newshell.login, shell->login, MAX_LOGIN-1);
  sys_strncpy(newshell.password, shell->password, MAX_PASSWORD-1);
  ptr_unlock(ptr, TAG_SHELL);
  newshell.session = vfs_open_session();

  sock = conn_filter(fd);
  if (newshell.dotelnet) {
    telnet = telnet_filter(sock);
    telnet_linemode(telnet, 0);
    telnet_echo(telnet, 1);
    telnet_naws(telnet);
    newshell.filter = telnet;
  } else {
    telnet = NULL;
    newshell.filter = sock;
  }

  if (newshell.login[0] == 0 || login_loop(newshell.filter, newshell.login, newshell.password) == 0) {
    xmemset(buf, 0, sizeof(buf));
    if (telnet) telnet_term(telnet, (char *)buf, sizeof(buf)-1, &cols, &rows);
    if (cols && rows) {
      newshell.cols = cols;
      newshell.rows = rows;
    }
    libshell_cls(&newshell);
    libshell_prompt(&newshell);

    for (; !thread_must_end();) {
      n = libshell_peek(&newshell, 10000);
      if (n == -1) break;
      if (n == 0) continue;

      n = libshell_read(&newshell, (char *)buf, sizeof(buf));
      if (n == -1) break;
      if (n == 0) continue;

      libshell_buildline(&newshell, buf, n);

      if (newshell.logout) {
        debug(DEBUG_INFO, "SHELL", "terminating connection");
        break;
      }
    }
  }

  conn_close(sock);
  if (telnet) telnet_close(telnet);
  if (newshell.session) vfs_close_session(newshell.session);
  for (i = 0; i < newshell.nhistory; i++) {
    if (newshell.history[i]) xfree(newshell.history[i]);
  }
  script_remove_ref(newshell.pe, newshell.ref);

  return 0;
}

static void shell_destructor(void *p) {
  shell_t *shell;

  shell = (shell_t *)p;
  xfree(shell);
}

static int libshell_server(int pe) {
  script_ref_t ref;
  script_int_t port;
  char *host = NULL;
  char *login = NULL;
  char *password = NULL;
  bt_provider_t *bt;
  io_addr_t addr;
  shell_t *shell;
  int handle, p, r = -1;

  if (script_get_function(pe, 0, &ref) == 0 &&
      script_get_string(pe,   1, &host) == 0 &&
      script_get_integer(pe,  2, &port) == 0 &&
      script_get_string(pe,   3, &login) == 0 &&
      script_get_string(pe,   4, &password) == 0) {

    if ((shell = xcalloc(1, sizeof(shell_t))) != NULL) {
      shell->tag = TAG_SHELL;
      if ((p = ptr_new(shell, shell_destructor)) != -1) {
        if (io_fill_addr(host, port, &addr) == 0) {
          shell->pe = pe;
          shell->ref = ref;
          sys_strncpy(shell->login, login, MAX_LOGIN-1);
          sys_strncpy(shell->password, password, MAX_PASSWORD-1);
          bt = script_get_pointer(pe, BT_PROVIDER);

          switch (addr.addr_type) {
            case IO_IP_ADDR:
              shell->dotelnet = 1;
              handle = io_simple_server(TAG_SHELL, &addr, libshell_loop, p, bt);
              break;
            case IO_BT_ADDR:
              handle = io_simple_server(TAG_SHELL, &addr, libshell_loop, p, bt);
              break;
            default:
              debug(DEBUG_ERROR, "SHELL", "invalid address type %d", addr.addr_type);
              handle = -1;
              break;
          }

          if (handle != -1) {
            r = script_push_integer(pe, handle);
          } else {
            ptr_free(p, TAG_SHELL);
          }
        } else {
          debug(DEBUG_ERROR, "SHELL", "could not parse address %s", host);
          ptr_free(p, TAG_SHELL);
        }
      } else {
        xfree(shell);
      }
    }
  }

  if (host) xfree(host);
  if (login) xfree(login);
  if (password) xfree(password);

  return r;
}

static int libshell_action_peek(struct conn_filter_t *filter, uint32_t us) {
  local_data_t *data = (local_data_t *)filter->data;
  script_arg_t ret;
  uint64_t t0;
  int r = -1;

  t0 = sys_get_clock();

  for (;;) {
    if (script_call(data->pe, data->peek_ref, &ret, "") == -1) {
      r = -1;
      break;
    }
    r = script_returned_value(&ret);
    if (r) {
      break;
    }
    if ((sys_get_clock() - t0) >= us) {
      r = 0;
      break;
    }
    sys_usleep(1000);
  }

  return r;
}

static int libshell_action_read(struct conn_filter_t *filter, uint8_t *b) {
  local_data_t *data = (local_data_t *)filter->data;
  script_arg_t ret;
  int r = 0;

  if (script_call(data->pe, data->read_ref, &ret, "") == 0) {
    *b = script_returned_value(&ret);
    r = 1;
  }

  return r;
}

static int libshell_action_write(struct conn_filter_t *filter, uint8_t *b, int n) {
  local_data_t *data = (local_data_t *)filter->data;
  script_lstring_t lstr;
  script_arg_t ret;
  int r = 0;

  lstr.s = (char *)b;
  lstr.n = n;
  if (script_call(data->pe, data->write_ref, &ret, "L", &lstr) == 0) {
    r = n;
  }

  return r;
}

static int libshell_action(void *arg) {
  local_data_t *data = (local_data_t *)arg;
  shell_t shell;
  conn_filter_t filter;
  uint8_t buf[256];
  int n;

  xmemset(&filter, 0, sizeof(conn_filter_t));
  filter.peek = libshell_action_peek;
  filter.read = libshell_action_read;
  filter.write = libshell_action_write;
  filter.data = data;

  xmemset(&shell, 0, sizeof(shell_t));
  shell.pe = data->pe;
  shell.ref = data->eval_ref;
  shell.session = vfs_open_session();
  shell.filter = &filter;

  libshell_cls(&shell);
  libshell_prompt(&shell);

  for (; !thread_must_end();) {
    n = libshell_peek(&shell, 10000);
    if (n == -1) break;
    if (n == 0) continue;

    n = libshell_read(&shell, (char *)buf, sizeof(buf));
    if (n == -1) break;
    if (n == 0) continue;

    libshell_buildline(&shell, buf, n);

    if (shell.logout) {
      debug(DEBUG_INFO, "SHELL", "terminating connection");
      break;
    }
  }

  vfs_close_session(shell.session);
  script_remove_ref(data->pe, data->eval_ref);
  script_remove_ref(data->pe, data->peek_ref);
  script_remove_ref(data->pe, data->read_ref);
  script_remove_ref(data->pe, data->write_ref);
  xfree(data);

  if (shell.logout) {
    sys_set_finish(0);
  }

  return 0;
}

static int libshell_run(int pe) {
  script_ref_t eval_ref, peek_ref, read_ref, write_ref;
  local_data_t *data;
  int handle, r = -1;

  if (script_get_function(pe, 0, &eval_ref) == 0 &&
      script_get_function(pe, 1, &peek_ref) == 0 &&
      script_get_function(pe, 2, &read_ref) == 0 &&
      script_get_function(pe, 3, &write_ref) == 0) {

    if ((data = xcalloc(1, sizeof(local_data_t))) != NULL) {
      data->pe = pe;
      data->eval_ref = eval_ref;
      data->peek_ref = peek_ref;
      data->read_ref = read_ref;
      data->write_ref = write_ref;
      if ((handle = thread_begin(TAG_SHELL, libshell_action, data)) != -1) {
        r = script_push_integer(pe, handle);
      } else {
        xfree(data);
      }
    }
  }

  return r;
}

static int libshell_add_command(shell_command_t *command) {
  int r = -1;

  if (command) {
    if (mutex_lock(mutex) == 0) {
      if (ncmds < MAX_CMDS) {
        debug(DEBUG_INFO, "SHELL", "adding command \"%s\"", command->name);
        xmemcpy(&cmd[ncmds++], command, sizeof(shell_command_t));
        r = 0;
      } else {
        debug(DEBUG_ERROR, "SHELL", "max number of commands reached");
      }
      mutex_unlock(mutex);
    }
  }

  return r;
}

static int cmd_script(shell_t *shell, vfs_session_t *session, int pe, int argc, char *argv[], void *_data) {
  script_arg_t args[MAX_ARGS];
  script_arg_t ret;
  cmd_data_t *data;
  char *val;
  int i, n, r = -1;

  data = (cmd_data_t *)_data;

  for (i = 0, n = 0; i < argc-1 && n < MAX_ARGS; i++, n++) {
    args[n].type = SCRIPT_ARG_STRING;
    args[n].value.s = argv[i+1];
  }

  if (script_call_args(pe, data->ref, &ret, n, args) == 0) {
    val = libshell_ret(pe, &ret);
    if (val) {
      if (val[0]) {
        libshell_write(shell, val, -1);
        libshell_write(shell, CRLF, -1);
      }
      xfree(val);
    }
    r = 0;
  }

  return r;
}

static int libshell_add(int pe) {
  shell_command_t command;
  script_ref_t ref;
  script_int_t minargs, maxargs;
  char *name = NULL;
  char *usage = NULL;
  char *help = NULL;
  cmd_data_t *data;
  int r = -1;

  if (script_get_function(pe, 0, &ref) == 0 &&
      script_get_string(pe,   1, &name) == 0 &&
      script_get_string(pe,   2, &usage) == 0 &&
      script_get_integer(pe,  3, &minargs) == 0 &&
      script_get_integer(pe,  4, &maxargs) == 0 &&
      script_get_string(pe,   5, &help) == 0) {

    if ((data = xcalloc(1, sizeof(cmd_data_t))) != NULL) {
      xmemset(&command, 0, sizeof(shell_command_t));
      command.name = name;
      command.usage = usage;
      command.minargs = minargs;
      command.maxargs = maxargs;
      command.cmd = cmd_script;
      command.help = help;
      command.data = data;
      data->ref = ref;
      r = 0;

      if (libshell_add_command(&command) == -1) {
        script_remove_ref(pe, ref);
        xfree(command.data);
        xfree(name);
        xfree(usage);
        xfree(help);
      }
    }

  } else {
    if (ref) script_remove_ref(pe, ref);
    if (name) xfree(name);
    if (usage) xfree(usage);
    if (help) xfree(help);
  }

  return script_push_boolean(pe, r == 0);
}

static int libshell_close(int pe) {
  return sock_stream_close(TAG_SHELL, pe);
}

int libshell_run_shell(conn_filter_t *filter, int pe, script_ref_t ref) {
  shell_t shell;
  uint8_t buf[256];
  int n;

  xmemset(&shell, 0, sizeof(shell_t));
  shell.filter = filter;
  shell.pe = pe;
  shell.ref = ref;
  shell.session = vfs_open_session();

  libshell_cls(&shell);
  libshell_prompt(&shell);

  for (; !thread_must_end();) {
    n = libshell_peek(&shell, 10000);
    if (n == -1) break;
    if (n == 0) continue;

    n = libshell_read(&shell, (char *)buf, sizeof(buf));
    if (n == -1) break;
    if (n == 0) continue;

    libshell_buildline(&shell, buf, n);
    if (shell.logout) break;
  }

  vfs_close_session(shell.session);

  return 0;
}

int libshell_load(void) {
  for (ncmds = 0; cmd[ncmds].name; ncmds++);

  mutex = mutex_create("SHELL");
  provider.run = libshell_run_shell;
  provider.add = libshell_add_command;
  provider.peek = libshell_peek;
  provider.read = libshell_read;
  provider.write = libshell_write;
  provider.print = libshell_print;
  provider.gets = libshell_gets;
  provider.window = libshell_window;

  return 0;
}

int libshell_unload(void) {
  mutex_destroy(mutex);
  return 0;
}

int libshell_init(int pe, script_ref_t obj) {
  debug(DEBUG_INFO, "SHELL", "registering provider %s", SHELL_PROVIDER);
  script_set_pointer(pe, SHELL_PROVIDER, &provider);

  script_add_function(pe, obj, "server", libshell_server);
  script_add_function(pe, obj, "run",    libshell_run);
  script_add_function(pe, obj, "add",    libshell_add);
  script_add_function(pe, obj, "close",  libshell_close);

  return 0;
}
