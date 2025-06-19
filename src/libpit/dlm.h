void dlm_init(void *h);
void *dlm_malloc(void *h, sys_size_t bytes);
void* dlm_realloc(void *h, void* oldmem, sys_size_t bytes);
void dlm_free(void *h, void *mem);
