#define commandExternalType 'CmdE'

#define commandNameType 'CmdN'
#define commandSafeType 'CmdS'

typedef struct {
  char *name;
  int (*function)(int pe);
  int (*main)(int argc, char *argv[]);
} command_builtin_t;

typedef struct command_internal_data_t command_internal_data_t;

typedef struct {
  command_internal_data_t *idata;
} command_data_t;
