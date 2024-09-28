#ifndef PIT_SYS_H
#define PIT_SYS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <setjmp.h>

typedef enum {
  SYS_SEEK_SET, SYS_SEEK_CUR, SYS_SEEK_END
} sys_seek_t;

#define SYS_READ     0x01
#define SYS_WRITE    0x02
#define SYS_NONBLOCK 0x04
#define SYS_TRUNC    0x08
#define SYS_EXCL     0x10
#define SYS_RDWR     (SYS_READ | SYS_WRITE)

#define SYS_IFREG    0x01
#define SYS_IFDIR    0x02
#define SYS_IRUSR    0x04
#define SYS_IWUSR    0x08

#define SYS_LEVEL_IP    1
#define SYS_LEVEL_TCP   2
#define SYS_LEVEL_SOCK  3

#define SYS_SHUTDOWN_RD    1
#define SYS_SHUTDOWN_WR    2
#define SYS_SHUTDOWN_RDWR  3

#define SYS_TCP_NODELAY  1
#define SYS_SOCK_LINGER  100

#define FILE_PATH    256

#ifdef WINDOWS
#define FILE_SEP     '\\'
#define SFILE_SEP    "\\"
#else
#define FILE_SEP     '/'
#define SFILE_SEP    "/"
#endif

#define IP_STREAM    1
#define IP_DGRAM     2

#define SYS_EOF      -1

#ifndef NULL
#define NULL ((void *)0)
#endif

typedef struct {
  uint32_t mode;   // file type and mode
  uint64_t size;   // total size, in bytes
  uint64_t atime;  // time of last access
  uint64_t mtime;  // time of last modification
  uint64_t ctime;  // time of last status change
} sys_stat_t;

typedef struct {
  uint64_t total;  // size of filesystem in bytes
  uint64_t free;   // free bytes in filesystem
} sys_statfs_t;

typedef struct {
  int l_onoff;
  int l_linger;
} sys_linger_t;

typedef struct {
  int tm_year;
  int tm_mon;
  int tm_mday;
  int tm_wday;
  int tm_hour;
  int tm_min;
  int tm_sec;
  int tm_isdst;
  int tm_yday;
} sys_tm_t;

typedef struct {
  uint32_t tv_sec;
  uint32_t tv_usec;
} sys_timeval_t;

typedef struct {
  uint64_t tv_sec;
  uint64_t tv_nsec;
} sys_timespec_t;

#if UINTPTR_MAX == 0xffffffff
typedef uint32_t sys_size_t;
typedef int32_t sys_ssize_t;
typedef int32_t sys_ptrdiff_t;
#elif UINTPTR_MAX == 0xffffffffffffffff
typedef uint64_t sys_size_t;
typedef int64_t sys_ssize_t;
typedef int64_t sys_ptrdiff_t;
#else
#error "Word size not known"
#endif

typedef struct sys_dir_t sys_dir_t;

#define FDSET_SIZE 1024

typedef struct {
  uint32_t mask[32];
} sys_fdset_t;

typedef __builtin_va_list sys_va_list;
#define sys_va_start(v,l) __builtin_va_start(v,l)
#define sys_va_end(v)     __builtin_va_end(v)
#define sys_va_arg(v,l)   __builtin_va_arg(v,l)

typedef jmp_buf sys_jmp_buf;

void sys_init(void);

void sys_usleep(uint32_t us);

uint64_t sys_time(void);

int sys_isdst(void);

uint64_t sys_timegm(sys_tm_t *tm);

uint64_t sys_timelocal(sys_tm_t *tm);

int sys_localtime(const uint64_t *t, sys_tm_t *tm);

int sys_gmtime(const uint64_t *t, sys_tm_t *tm);

int sys_timeofday(sys_timeval_t *tv);

char *sys_getenv(char *name);

int sys_country(char *country, int len);

int sys_language(char *language, int len);

uint32_t sys_get_pid(void);

uint32_t sys_get_tid(void);

int sys_errno(void);

void sys_strerror(int err, char *msg, int len);

int sys_daemonize(void);

sys_dir_t *sys_opendir(const char *pathname);

int sys_readdir(sys_dir_t *dir, char *name, int len);

int sys_closedir(sys_dir_t *dir);

int sys_chdir(char *path);

int sys_getcwd(char *buf, int len);

int sys_open(const char *pathname, int flags);

int sys_create(const char *pathname, int flags, uint32_t mode);

int sys_select(int fd, uint32_t us);

int sys_read_timeout(int fd, uint8_t *buf, int len, int *nread, uint32_t us);

int sys_read(int fd, uint8_t *buf, int len);

int sys_write(int fd, uint8_t *buf, int len);

int64_t sys_seek(int fd, int64_t offset, sys_seek_t whence);

int sys_truncate(int fd, int64_t offset);

int sys_pipe(int *fd);

int sys_peek(int fd);

int sys_stat(const char *pathname, sys_stat_t *st);

int sys_fstat(int fd, sys_stat_t *st);

int sys_statfs(const char *pathname, sys_statfs_t *st);

int sys_close(int fd);

int sys_rename(const char *pathname1, const char *pathname2);

int sys_unlink(const char *pathname);

int sys_rmdir(const char *pathname);

int sys_mkdir(const char *pathname);

int sys_select_fds(int nfds, sys_fdset_t *readfds, sys_fdset_t *writefds, sys_fdset_t *exceptfds,
                   sys_timeval_t *timeout);

void sys_fdclr(int n, sys_fdset_t *fds);

void sys_fdset(int n, sys_fdset_t *fds);

void sys_fdzero(sys_fdset_t *fds);

int sys_fdisset(int n, sys_fdset_t *fds);

void sys_srand(int32_t seed);

int32_t sys_rand(void);

int sys_serial_open(char *device, char *word, int baud);

int sys_serial_baud(int serial, int baud);

int sys_serial_word(int serial, char *word);

void *sys_tty_raw(int fd);

int sys_tty_restore(int fd, void *p);

int sys_termsize(int fd, int *cols, int *rows);

int sys_isatty(int fd);

int64_t sys_get_clock(void);

int sys_get_clock_ts(sys_timespec_t *ts);

int64_t sys_get_process_time(void);

int64_t sys_get_thread_time(void);

int sys_set_thread_name(char *name);

void sys_block_signals(void);

void sys_unblock_signals(void);

void sys_install_handler(int signum, void (*handler)(int));

void sys_wait(int *status);

void *sys_lib_load(char *filename, int *first_load);

void *sys_lib_defsymbol(void *lib, char *name, int mandatory);

int sys_lib_close(void *lib);

uint32_t sys_socket_ipv4(char *host);

int sys_socket_fill_addr(void *a, char *host, int port, int *len);

int sys_socket_open(int type, int ipv6);

int sys_socket_connect(int sock, char *host, int port);

int sys_socket_open_connect_timeout(char *host, int port, int type, uint32_t us);

int sys_socket_open_connect(char *host, int port, int type);

int sys_socket_binds(int sock, char *host, int *port);

int sys_socket_listen(int sock, int n);

int sys_socket_bind(char *host, int *port, int type);

int sys_socket_bind_connect(char *src_host, int src_port, char *host, int port, int type);

int sys_socket_accept(int sock, char *host, int hlen, int *port, sys_timeval_t *tv);

int sys_socket_sendto(int sock, char *host, int port, unsigned char *buf, int n);

int sys_socket_recvfrom(int sock, char *host, int hlen, int *port, unsigned char *buf, int n,
                        sys_timeval_t *tv);

int sys_socket_shutdown(int sock, int dir);

int sys_setsockopt(int sock, int level, int optname, const void *optval, int optlen);

void sys_exit(int r);

void sys_set_finish(int status);

int sys_fork_exec(char *filename, char *argv[], int fd);

int sys_list_symbols(char *libname);

int sys_tmpname(char *buf, int max);

int sys_mkstemp(void);

void *sys_malloc(sys_size_t size);

void sys_free(void *ptr);

void *sys_calloc(sys_size_t nmemb, sys_size_t size);

void *sys_realloc(void *ptr, sys_size_t size);

char *sys_strdup(const char *s);

char *sys_strndup(const char *s, sys_size_t n);

char *sys_strcpy(char *dest, const char *src);

char *sys_strncpy(char *dest, const char *src, sys_size_t n);

uint32_t sys_strlen(const char *s);

char *sys_strchr(const char *s, int c);

char *sys_strrchr(const char *s, int c);

char *sys_strstr(const char *haystack, const char *needle);

char *sys_strpbrk(const char *s, const char *accept);

int sys_strcmp(const char *s1, const char *s2);

int sys_strncmp(const char *s1, const char *s2, sys_size_t n);

int sys_strcasecmp(const char *s1, const char *s2);

int sys_strncasecmp(const char *s1, const char *s2, sys_size_t n);

char *sys_strcat(char *dest, const char *src);

char *sys_strncat(char *dest, const char *src, sys_size_t n);

int sys_atoi(const char *nptr);

double sys_atof(const char *nptr);

long sys_strtol(const char *nptr, char **endptr, int base);

unsigned long int sys_strtoul(const char *nptr, char **endptr, int base);

double sys_strtod(const char *nptr, char **endptr);

int sys_memcmp(const void *s1, const void *s2, sys_size_t n) ;

void *sys_memmove(void *dest, const void *src, sys_size_t n);

void *sys_memcpy(void *dest, const void *src, sys_size_t n);

void *sys_memset(void *s, int c, sys_size_t n);

int sys_abs(int x);

double sys_floor(double x);

double sys_ceil(double x);

double sys_fabs(double x);

double sys_log(double x);

double sys_exp(double x);

double sys_sqrt(double x);

double sys_pow(double x, double y);

double sys_sin(double x);

double sys_cos(double x);

double sys_atan2(double y, double x);

double sys_pi(void);

double sys_modf(double x, double *iptr);

int sys_isnan(double x);

int sys_isinf(double x);

int sys_signbit(double x);

void sys_qsort(void *base, sys_size_t nmemb, sys_size_t size, int (*compar)(const void *, const void *));

void *sys_bsearch(const void *key, const void *base, sys_size_t nmemb, sys_size_t size, int (*compar)(const void *, const void *));

int sys_toupper(int c);

int sys_tolower(int c);

int sys_sprintf(char *str, const char *format, ...);

int sys_snprintf(char *str, sys_size_t size, const char *format, ...);

int sys_vsprintf(char *str, const char *format, sys_va_list ap);

int sys_vsnprintf(char *str, sys_size_t size, const char *format, sys_va_list ap);

int sys_sscanf(const char *str, const char *format, ...);

sys_size_t sys_getpagesize(void);

int sys_setjmp(sys_jmp_buf env);

void sys_longjmp(sys_jmp_buf env, int val);

#ifdef __cplusplus
}
#endif

#endif
