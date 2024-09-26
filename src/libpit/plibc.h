#ifndef PLIBC_H
#define PLIBC_H

#define PLIBC_EOF -1

typedef struct PLIBC_FILE PLIBC_FILE;

extern PLIBC_FILE *plibc_stdin;
extern PLIBC_FILE *plibc_stdout;
extern PLIBC_FILE *plibc_stderr;

enum { PLIBC_SEEK_SET, PLIBC_SEEK_CUR, PLIBC_SEEK_END };

PLIBC_FILE *plibc_fopen(const char *pathname, const char *mode);
int plibc_fclose(PLIBC_FILE *stream);
sys_size_t plibc_fread(void *ptr, sys_size_t size, sys_size_t nmemb, PLIBC_FILE *stream);
sys_size_t plibc_fwrite(const void *ptr, sys_size_t size, sys_size_t nmemb, PLIBC_FILE *stream);
int plibc_fseek(PLIBC_FILE *stream, long offset, int whence);
int plibc_ftruncate(PLIBC_FILE *stream, long offset);
long plibc_ftell(PLIBC_FILE *stream);
int plibc_feof(PLIBC_FILE *stream);
int plibc_fflush(PLIBC_FILE *stream);
int plibc_rename(const char *oldpath, const char *newpath);
int plibc_remove(const char *pathname);
char *plibc_tmpnam(void);
int plibc_errno(void);
const char *plibc_strerror(int err);

int plibc_fputc(int c, PLIBC_FILE *stream);
int plibc_fputs(const char *s, PLIBC_FILE *stream);

int plibc_fgetc(PLIBC_FILE *stream);
int plibc_ungetc(int c, PLIBC_FILE *stream);
char *plibc_fgets(char *s, int size, PLIBC_FILE *stream);

#define plibc_putchar(c) plibc_fputc(c, plibc_stdout)
#define plibc_putc plibc_fputc
#define plibc_puts(s) plibc_fputs(s, plibc_stdout)
#define plibc_getc plibc_fgetc
#define plibc_getchar() plibc_fgetc(plibc_stdin)

int plibc_fprintf(PLIBC_FILE *stream, const char *format, ...);
int plibc_vfprintf(PLIBC_FILE *stream, const char *format, sys_va_list ap);
int plibc_printf(const char *format, ...);
int plibc_vprintf(const char *format, sys_va_list ap);
int plibc_sprintf(char *str, const char *format, ...);
int plibc_snprintf(char *str, sys_size_t size, const char *format, ...);

int plibc_iscntrl(int c);
int plibc_isblank(int c);
int plibc_isspace(int c);
int plibc_isgraph(int c);
int plibc_ispunct(int c);
int plibc_islower(int c);
int plibc_isupper(int c);
int plibc_isalpha(int c);
int plibc_isdigit(int c);
int plibc_isalnum(int c);
int plibc_isxdigit(int c);
int plibc_isprint(int c);

void plibc_error(const char *filename, const char *function, int lineNo, char *msg);

#define plibc_rewind(stream) (void)plibc_fseek(stream, 0, PLIBC_SEEK_SET)

#define plibc_assert(expr) if (!(expr)) plibc_error(__FILE__, __FUNCTION__, __LINE__, #expr);
#define plibc_abort() plibc_error(__FILE__, __FUNCTION__, __LINE__, "abort");

#define plibc_malloc      sys_malloc
#define plibc_calloc      sys_calloc
#define plibc_realloc     sys_realloc
#define plibc_free        sys_free

#define plibc_memcmp      sys_memcmp
#define plibc_memmove     sys_memmove
#define plibc_memcpy      sys_memcpy
#define plibc_memset      sys_memset

#define plibc_strdup      sys_strdup
#define plibc_strndup     sys_strndup
#define plibc_strcpy      sys_strcpy
#define plibc_strncpy     sys_strncpy
#define plibc_strlen      sys_strlen
#define plibc_strchr      sys_strchr
#define plibc_strrchr     sys_strrchr
#define plibc_strstr      sys_strstr
#define plibc_strpbrk     sys_strpbrk
#define plibc_strcmp      sys_strcmp
#define plibc_strncmp     sys_strncmp
#define plibc_strcasecmp  sys_strcasecmp
#define plibc_strncasecmp sys_strncasecmp
#define plibc_strcat      sys_strcat
#define plibc_strncat     sys_strncat
#define plibc_strtol      sys_strtol
#define plibc_strtoul     sys_strtoul
#define plibc_strtod      sys_strtod

#define plibc_vsprintf    sys_vsprintf
#define plibc_vsnprintf   sys_vsnprintf
#define plibc_sscanf      sys_sscanf

#define plibc_tolower     sys_tolower
#define plibc_toupper     sys_toupper
#define plibc_atoi        sys_atoi

#endif
