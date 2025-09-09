#define syntaxPluginType  'sntx'

typedef struct syntax_highlight_t syntax_highlight_t;

typedef struct {
  syntax_highlight_t *(*syntax_create)(uint32_t bg);
  void (*syntax_destroy)(syntax_highlight_t *shigh);
  void (*syntax_begin_line)(syntax_highlight_t *shigh, void (*print)(void *data, uint32_t fg, uint32_t bg, char *s, int n), void *data);
  void (*syntax_char)(syntax_highlight_t *shigh, char c);
  void (*syntax_end_line)(syntax_highlight_t *shigh);
  void (*syntax_reset)(syntax_highlight_t *shigh);
} syntax_plugin_t;

syntax_plugin_t *syntax_get_plugin(char *ext);
