#define commandExternalType 'CmdE'

#define commandNameType 'CmdN'
#define commandHelpType 'CmdH'
#define commandSafeType 'CmdS'
#define commandCRLFType 'CmdL'

typedef struct command_internal_data_t command_internal_data_t;

typedef struct {
  command_internal_data_t *idata;
} command_data_t;
