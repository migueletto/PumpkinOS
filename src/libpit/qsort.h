typedef int (*cmpfun0)(const void *, const void *);

void my_qsort(void *base, sys_size_t nel, sys_size_t width, cmpfun0 cmp);
