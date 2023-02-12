#ifdef PALMOS
#define SEC(name) __attribute__ ((section (name)))
#else
#define SEC(name)
#endif
