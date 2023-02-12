#define SHELL_PROVIDER "shell_provider"

typedef struct shell_t shell_t;

typedef struct {
  char *name;
  char *usage;
  int minargs, maxargs;
  int (*cmd)(shell_t *shell, vfs_session_t *session, int pe, int argc, char *argv[], void *data);
  char *help;
  void *data;
} shell_command_t;

typedef struct {
  int (*add)(shell_command_t *cmd);
  int (*peek)(shell_t *shell, uint32_t us);
  int (*read)(shell_t *shell, char *buf, int len);
  int (*write)(shell_t *shell, char *buf, int len);
  void (*print)(shell_t *shell, int err, char *fmt, ...);
  char *(*gets)(shell_t *shell, char *buf, int len);
  int (*window)(shell_t *shell, int *ncols, int *nrows);
  int (*run)(conn_filter_t *filter, int pe, script_ref_t ref);
} shell_provider_t;

