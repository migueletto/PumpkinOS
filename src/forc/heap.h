typedef struct heap_t heap_t;

heap_t *heap_init(uint8_t *start, uint32_t size);
void heap_finish(heap_t *heap);
uint8_t *heap_alloc(heap_t *heap, uint32_t size);
int heap_free(heap_t *heap, uint8_t *p);
int heap_inc(heap_t *heap, uint8_t *p);
int heap_dec(heap_t *heap, uint8_t *p);
void heap_dump(heap_t *heap);
