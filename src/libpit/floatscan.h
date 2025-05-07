typedef struct {
  char *buffer;
  int pos, lim, size;
} floatscan_t;

long double floatscan(floatscan_t *f, int prec, int pok);
