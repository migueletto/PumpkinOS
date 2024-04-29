#define CSS_COLOR   "color"
#define CSS_BGCOLOR "background-color"
#define CSS_MARGIN  "margin"
#define CSS_PADDING "padding"

typedef enum {
  CSS_FILTER_TAG,
  CSS_FILTER_ID,
  CSS_FILTER_CLASS,
  CSS_FILTER_PSEUDO,
  CSS_FILTER_ATTR,
  CSS_FILTER_ANY,
  CSS_FILTER_OTHER
} css_filter_type_e;

typedef enum {
  CSS_ATTR_OP_EXISTS,
  CSS_ATTR_OP_EQUALS,
  CSS_ATTR_OP_INCLUDES,
  CSS_ATTR_OP_DASHMATCH,
  CSS_ATTR_OP_STARTS,
  CSS_ATTR_OP_ENDS,
  CSS_ATTR_OP_CONTAINS
} css_attr_op_e;

typedef struct {
  char *name;
  css_attr_op_e op;
  char *value; // is NULL for EXISTS
} css_filter_attr_t;

typedef struct css_filter_t {
  css_filter_type_e type;
  union {
    char *tag;
    char *id;
    char *cclass;
    char *pseudo;
    char *other;
    css_filter_attr_t attr;
  } data;
  struct css_filter_t *next;
} css_filter_t;

typedef enum {
  CSS_COMBINATOR_NONE,
  CSS_COMBINATOR_DESCENDANT,
  CSS_COMBINATOR_CHILD,
  CSS_COMBINATOR_ADJ_SIBLING,
  CSS_COMBINATOR_GEN_SIBLING,
  CSS_COMBINATOR_COMMA
} css_combinator_e;

typedef struct css_selector_t {
  css_filter_t *group;
  css_combinator_e combinator;
  struct css_selector_t *next;
} css_selector_t;

typedef enum {
  CSS_DECL_NONE,
  CSS_DECL_IDENT,
  CSS_DECL_STRING,
  CSS_DECL_COLOR,
  CSS_DECL_NUMBER,
  CSS_DECL_LIST,
  CSS_DECL_FUNCTION,
  CSS_DECL_VAR
} css_decl_type_e;

typedef enum {
  CSS_UNIT_NONE,
  CSS_UNIT_CM,
  CSS_UNIT_MM,
  CSS_UNIT_IN,
  CSS_UNIT_PT,
  CSS_UNIT_PC,
  CSS_UNIT_PX,
  CSS_UNIT_EM,
  CSS_UNIT_REM,
  CSS_UNIT_DEG,
  CSS_UNIT_PERCENT,
  CSS_UNIT_S
} css_number_unit_e;

typedef struct {
  int integer;
  union {
    int32_t i;
    double d;
  } value;
  css_number_unit_e unit;
} css_number_t;

typedef struct {
  struct css_list_element_t *elements;
  int size, len;
} css_list_t;

typedef struct {
  char *name;
  css_list_t args;
} css_function_t;

typedef struct css_decl_value_t {
  css_decl_type_e type;
  union {
    char *ident;
    char *string;
    char *var;
    uint32_t color;
    css_number_t number;
    css_list_t list;
    css_function_t function;
  } data;
} css_decl_value_t;

typedef struct css_list_element_t {
  char separator;
  css_decl_value_t value;
} css_list_element_t;

typedef struct css_declaration_t {
  char *name;
  css_decl_value_t value;
  struct css_declaration_t *next;
} css_declaration_t;

typedef struct css_rule_t {
  css_selector_t *selectors;
  css_declaration_t *declarations;
  struct css_rule_t *next;
} css_rule_t;

typedef struct {
  css_rule_t *rules;
} css_stylesheet_t;

void css_test(char *buffer, int len);
ht *css_color_names(void);
int css_parse(char *buffer, int len, int full_stylesheet, html_env_t *env, void (*callback)(css_declaration_t *decl, void *data), void *data);
