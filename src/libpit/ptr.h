#ifndef PIT_PTR_H
#define PIT_PTR_H

#ifdef __cplusplus
extern "C" {
#endif

int ptr_init(void);

int ptr_close(void);

int ptr_new(void *p, void (*destructor)(void *p));

int ptr_new_c(void *p, void (*destructor)(void *p));

int ptr_free_full(const char *file, const char *func, int line, int id, char *tag);

void *ptr_lock_full(const char *file, const char *func, int line, int id, char *tag);

void ptr_unlock_full(const char *file, const char *func, int line, int id, char *tag);

int ptr_wait_full(const char *file, const char *func, int line, int id, int us, char *tag);

int ptr_signal_full(const char *file, const char *func, int line, int id, char *tag);

int ptr_check_tag(char *received, char *expected);

#define ptr_free(id, tag) ptr_free_full(__FILE__, __FUNCTION__, __LINE__, id, tag)
#define ptr_lock(id, tag) ptr_lock_full(__FILE__, __FUNCTION__, __LINE__, id, tag)
#define ptr_unlock(id, tag) ptr_unlock_full(__FILE__, __FUNCTION__, __LINE__, id, tag)
#define ptr_wait(id, us, tag) ptr_wait_full(__FILE__, __FUNCTION__, __LINE__, id, us, tag)
#define ptr_signal(id, tag) ptr_signal_full(__FILE__, __FUNCTION__, __LINE__, id, tag)

#ifdef __cplusplus
}
#endif

#endif
