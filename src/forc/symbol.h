#define SYMBOL_CONST    0x01
#define SYMBOL_VAR      0x02
#define SYMBOL_FUNCTION 0x04
#define SYMBOL_STRING   0x08
#define SYMBOL_STRUCT   0x10
#define SYMBOL_ALL      0xFF

typedef struct {
  type_t type;
  int array;
  int external;
  uint32_t offset;
  uint8_t *value;
} symbol_const_t;

typedef struct {
  char *name;
  struct symbol_table_t *st;
} symbol_struct_t;

typedef struct {
  type_t type;
  int array, use, set;
  uint32_t offset;
  struct symbol_table_t *st; // for structs
} symbol_var_t;

typedef struct {
  type_t type;
  uint32_t offset;
  uint32_t return_address;
  uint32_t param_size;
  uint16_t end_label;
  char *name;
  int nparams;
  int external;
  char *vararg;
  struct symbol_table_t *st;
  struct block_t *block;
} symbol_function_t;

typedef struct {
  uint32_t id;
  char *s;
} symbol_string_t;

typedef struct {
  uint32_t index;
  uint32_t type;
  char *name;
  struct symbol_table_t *st;
  union {
    symbol_const_t cons;
    symbol_struct_t struc;
    symbol_var_t var;
    symbol_function_t function;
    symbol_string_t string;
  } symbol;
} symbol_t;

typedef struct symbol_table_t {
  int size_symbols;
  int num_symbols;
  int global;
  uint32_t frame_size;
  symbol_t *symbol;
} symbol_table_t;
