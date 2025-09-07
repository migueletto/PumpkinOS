#ifndef PLIBC_H
#define PLIBC_H

#define PLIBC_EOF -1

#define PLIBC_ACCMODE 03
#define PLIBC_RDONLY  00
#define PLIBC_WRONLY  01
#define PLIBC_RDWR    02
#define PLIBC_CREAT   0100
#define PLIBC_TRUNC   01000
#define PLIBC_APPEND  02000

typedef struct PLIBC_FILE PLIBC_FILE;

extern PLIBC_FILE *plibc_stdin;
extern PLIBC_FILE *plibc_stdout;
extern PLIBC_FILE *plibc_stderr;

enum { PLIBC_SEEK_SET, PLIBC_SEEK_CUR, PLIBC_SEEK_END };

int plibc_init(void);
int plibc_finish(void);
int plibc_setfd(int fd, void *fileRef);
int plibc_mkdir(int vol, const char *pathname);
int plibc_chdir(int vol, const char *pathname);
int plibc_getdir(int vol, char *pathname, int max);
int plibc_open(int vol, const char *pathname, int flags);
int plibc_close(int fd);
sys_ssize_t plibc_read(int fd, void *buf, sys_size_t count);
sys_ssize_t plibc_write(int fd, void *buf, sys_size_t count);
sys_ssize_t plibc_lseek(int fd, sys_ssize_t offset, int whence);
int plibc_dup(int oldfd);
int plibc_dup2(int oldfd, int newfd);
PLIBC_FILE *plibc_fdopen(int fd, const char *mode);
int plibc_fileno(PLIBC_FILE *stream);

PLIBC_FILE *plibc_fopen(int vol, const char *pathname, const char *mode);
int plibc_fclose(PLIBC_FILE *stream);
sys_size_t plibc_fread(void *ptr, sys_size_t size, sys_size_t nmemb, PLIBC_FILE *stream);
sys_size_t plibc_fwrite(const void *ptr, sys_size_t size, sys_size_t nmemb, PLIBC_FILE *stream);
int plibc_fseek(PLIBC_FILE *stream, long offset, int whence);
int plibc_ftruncate(PLIBC_FILE *stream, long offset);
long plibc_ftell(PLIBC_FILE *stream);
int plibc_feof(PLIBC_FILE *stream);
int plibc_fflush(PLIBC_FILE *stream);
int plibc_rename(int vol, const char *oldpath, const char *newpath);
int plibc_remove(int vol, const char *pathname);
char *plibc_tmpnam(void);
int plibc_errno(void);
const char *plibc_strerror(int err);

int plibc_fputc(int c, PLIBC_FILE *stream);
int plibc_fputs(const char *s, PLIBC_FILE *stream);

int plibc_fgetc(PLIBC_FILE *stream);
int plibc_ungetc(int c, PLIBC_FILE *stream);
char *plibc_fgets(char *s, int size, PLIBC_FILE *stream);

int plibc_haschar(void);

#define plibc_putchar(c) plibc_fputc(c, (PLIBC_FILE *)plibc_stdout)
#define plibc_putc plibc_fputc
#define plibc_puts(s) plibc_fputs(s, (PLIBC_FILE *)plibc_stdout)
#define plibc_getc plibc_fgetc
#define plibc_getchar() plibc_fgetc((PLIBC_FILE *)plibc_stdin)

int plibc_fprintf(PLIBC_FILE *stream, const char *format, ...);
int plibc_vfprintf(PLIBC_FILE *stream, const char *format, sys_va_list ap);
int plibc_printf(const char *format, ...);
int plibc_vprintf(const char *format, sys_va_list ap);

void plibc_error(const char *filename, const char *function, int lineNo, char *msg);

#define plibc_rewind(stream) (void)plibc_fseek(stream, 0, PLIBC_SEEK_SET)

#define plibc_assert(expr) if (!(expr)) plibc_error(__FILE__, __FUNCTION__, __LINE__, #expr);
#define plibc_abort() plibc_error(__FILE__, __FUNCTION__, __LINE__, "abort");

#endif
