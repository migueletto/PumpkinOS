#ifndef PIT_LOADFILE_H
#define PIT_LOADFILE_H

#ifdef __cplusplus
extern "C" {
#endif

char *load_fd(int fd, unsigned int *len);

char *load_stream(FILE *f, unsigned int *len);

char *load_file(char *filename, unsigned int *len);

#ifdef __cplusplus
}
#endif

#endif
