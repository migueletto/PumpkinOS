typedef struct {
  uint8_t *block;
  uint32_t blockSize;
  uint32_t heapStart;
  uint32_t heapSize;
  heap_t *heap;
} tos_data_t;

int tos_main(int argc, char *argv[]);
