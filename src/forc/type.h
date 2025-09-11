typedef enum {
  TYPE_BOOL = 1,
  TYPE_INT8,
  TYPE_UINT8,
  TYPE_INT16,
  TYPE_UINT16,
  TYPE_INT32,
  TYPE_UINT32,
  TYPE_FLOAT,
  TYPE_STRING,
  TYPE_STRUCT = 256
} type_t;

typedef struct {
  type_t type;
  union {
    int8_t i8;
    uint8_t u8;
    int16_t i16;
    uint16_t u16;
    int32_t i32;
    uint32_t u32;
    int64_t i64;
    uint64_t u64;
    uint8_t b;
    float d;
    char *s;
    void *p;
  } value;
} value_t;

uint32_t type_size(type_t type);
uint32_t type_align(uint32_t size, uint32_t offset);
char *type_name(type_t type);
int type_integer(type_t t);
int type_signed(type_t t);
int type_unsigned(type_t t);
int type_numeric(type_t t);
int type_contains(type_t t1, type_t t2);
int type_coerce(value_t *value, type_t t, value_t *arg, int cast);
