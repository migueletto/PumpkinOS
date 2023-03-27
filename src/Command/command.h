#define commandPluginType 'CmdP'

typedef struct {
  char *name;
  int (*function)(int pe);
  int (*main)(int argc, char *argv[]);
} command_builtin_t;

typedef struct command_internal_data_t command_internal_data_t;

typedef struct {
  void (*putc)(command_internal_data_t *idata, char c);
  void (*puts)(command_internal_data_t *idata, char *s);
} command_external_data_t;

typedef struct {
  command_external_data_t *edata;
  command_internal_data_t *idata;
} command_data_t;
