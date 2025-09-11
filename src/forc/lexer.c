#include "sys.h"
#include "plibc.h"
#include "ht.h"
#include "lexer.h"

struct lexer_t {
  int fd;
  char back;
  char token[MAX_STRING];
  ht *reserved;
  uint32_t line;
};

typedef struct {
  char *name;
  uint16_t token;
} reserved_t;

static reserved_t reserved[] = {
  { "import"    , TOKEN_IMPORT },
  { "const"     , TOKEN_CONST },
  { "struct"    , TOKEN_STRUCT },
  { "var"       , TOKEN_VAR },
  { "let"       , TOKEN_LET },
  { "uint8"     , TOKEN_UINT8 },
  { "int8"      , TOKEN_INT8 },
  { "uint16"    , TOKEN_UINT16 },
  { "int16"     , TOKEN_INT16 },
  { "uint32"    , TOKEN_UINT32 },
  { "int32"     , TOKEN_INT32 },
  { "float"     , TOKEN_FLOAT },
  { "string"    , TOKEN_STRING },
  { "null"      , TOKEN_NNULL },
  { "new"       , TOKEN_NEW },
  { "function"  , TOKEN_FUNCTION },
  { "if"        , TOKEN_IF },
  { "else"      , TOKEN_ELSE },
  { "switch"    , TOKEN_SWITCH },
  { "case"      , TOKEN_CASE },
  { "loop"      , TOKEN_LOOP },
  { "break"     , TOKEN_BREAK },
  { "continue"  , TOKEN_CONTINUE },
  { "return"    , TOKEN_RETURN },
  { "pi"        , TOKEN_PI },
  { NULL        , 0 }
};

static char *stoken[] = {
  "OPENC",
  "CLOSEC",
  "BOOL",
  "UINTEGER",
  "INTEGER",
  "HEXA",
  "OCTAL",
  "BINARY",
  "FLOATN",
  "STR",
  "IDENT",
  "UINT8",
  "INT8",
  "UINT16",
  "INT16",
  "UINT32",
  "INT32",
  "FLOAT",
  "STRING",
  "CONST",
  "STRUCT",
  "VAR",
  "LET",
  "NNULL",
  "NEW",
  "FUNCTION",
  "PLUS",
  "MINUS",
  "TIMES",
  "DIV",
  "MOD",
  "OPENP",
  "CLOSEP",
  "OPENB",
  "CLOSEB",
  "OPENSB",
  "CLOSESB",
  "COLON",
  "SCOLON",
  "COMMA",
  "DOT",
  "QUESTION",
  "OR",
  "AND",
  "NOT",
  "BOR",
  "BAND",
  "BNOT",
  "BXOR",
  "EQ",
  "LT",
  "GT",
  "LTE",
  "GTE",
  "NEQ",
  "LSHIFT",
  "RSHIFT",
  "INC",
  "DEC",
  "PI",
  "LOOP",
  "IF",
  "ELSE",
  "BREAK",
  "CONTINUE",
  "SWITCH",
  "CASE",
  "RANGE",
  "VARARG",
  "RETURN",
  "IMPORT",
};

char *lexer_stoken(token_e id) {
  return id ? stoken[id - 0x1000] : NULL;
}

lexer_t *lexer_init(int fd) {
  lexer_t *l;
  int i;

  if ((l = sys_calloc(1, sizeof(lexer_t))) != NULL) {
    l->fd = fd;
    l->line = 1;
    l->reserved = ht_create();
    for (i = 0; reserved[i].name; i++) {
      ht_set(l->reserved, reserved[i].name, &reserved[i]);
    }
  }

  return l;
}

void lexer_destroy(lexer_t *l) {
  if (l) {
    ht_destroy(l->reserved);
    sys_free(l);
  }
}

static uint16_t tok(lexer_t *l, uint16_t token, token_t *t) {
  t->id = token;
  t->value = NULL;

  switch (token) {
    case TOKEN_IDENT:
    case TOKEN_STRING:
    case TOKEN_BOOL:
    case TOKEN_INTEGER:
    case TOKEN_UINTEGER:
    case TOKEN_HEXA:
    case TOKEN_OCTAL:
    case TOKEN_BINARY:
    case TOKEN_FLOAT:
      t->value = sys_strdup(l->token);
      break;
  }

  return token;
}

static uint16_t check_bool(lexer_t *l, uint16_t token, token_t *t) {
  char *s;
  int i;

  s = l->token;
  if (s[0] == '0' && (s[1] == 'x' || s[1] == 'o' || s[1] == 'b')) s += 2;

  for (i = 0; s[i]; i++) {
    if (s[i] != '0') return tok(l, token, t);
  }

  t->id = TOKEN_BOOL;
  t->value = sys_strdup(s);
  return token;
}

static uint16_t check_reserved(lexer_t *l, token_t *t) {
  reserved_t *r;

  if ((r = ht_get(l->reserved, l->token)) != NULL) {
    return tok(l, r->token, t);
  }

  return tok(l, TOKEN_IDENT, t);
}

uint32_t lexer_line(lexer_t *l) {
  return l->line;
}

uint16_t lexer_next(lexer_t *l, token_t *t) {
  int i, r, s;
  char c;

  s = 0;
  i = 0;
  l->token[0] = 0;

  for (;;) {
    if (l->back) {
      c = l->back;
      l->back = 0;
    } else {
      r = plibc_read(l->fd, &c, 1);
      if (r == -1) return tok(l, TOKEN_ERROR, t);
      if (r == 0) return tok(l, TOKEN_EOF, t);
    }

    switch (s) {
      case 0:
        switch (c) {
          case ' ':  break;
          case '\t': break;
          case '\n': l->line++; break;
          case '\r': break;
          case '(': return tok(l, TOKEN_OPENP, t);
          case ')': return tok(l, TOKEN_CLOSEP, t);
          case '{': return tok(l, TOKEN_OPENB, t);
          case '}': return tok(l, TOKEN_CLOSEB, t);
          case '[': return tok(l, TOKEN_OPENSB, t);
          case ']': return tok(l, TOKEN_CLOSESB, t);
          case '*': return tok(l, TOKEN_TIMES, t);
          case '+': return tok(l, TOKEN_PLUS, t);
          case ',': return tok(l, TOKEN_COMMA, t);
          case ';': return tok(l, TOKEN_SCOLON, t);
          case ':': return tok(l, TOKEN_COLON, t);
          case '~': return tok(l, TOKEN_BNOT, t);
          case '^': return tok(l, TOKEN_BXOR, t);
          case '%': return tok(l, TOKEN_MOD, t);
          case '?': return tok(l, TOKEN_QUESTION, t);
          case '!': return tok(l, TOKEN_NOT, t);
          case '=': return tok(l, TOKEN_EQ, t);
          case '0': s = 1; break;
          case '>': s = 3; break;
          case '<': s = 4; break;
          case '&': s = 5; break;
          case '|': s = 6; break;
          case '.': s = 8; break;
          case '-': s = 9; break;
          case '/': s = 10; break;
          case '"': s = 11; break;
          default:
            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
              if (i == MAX_TOKEN) return tok(l, TOKEN_ERROR, t);
              l->token[i++] = c;
              s = 24;
            } else if (c >= '1' && c <= '9') {
              if (i == MAX_TOKEN) return tok(l, TOKEN_ERROR, t);
              l->token[i++] = c;
              s = 25;
            } else {
              return tok(l, TOKEN_ERROR, t);
            }
            break;
        }
        break;
      case 1:
        switch (c) {
          case 'x': s = 21; break;
          case 'o': s = 22; break;
          case 'b': s = 23; break;
          case '.':
            if (i == MAX_TOKEN-1) return tok(l, TOKEN_ERROR, t);
            l->token[i++] = '0';
            l->token[i++] = c;
            s = 26;
            break;
          default:
            if (c >= '0' && c <= '9') {
              if (i == MAX_TOKEN-1) return tok(l, TOKEN_ERROR, t);
              l->token[i++] = '0';
              l->token[i++] = c;
              s = 25;
            } else if (c == 'u') {
              if (i == MAX_TOKEN) return tok(l, TOKEN_ERROR, t);
              l->token[i++] = '0';
              l->token[i] = 0;
              return tok(l, TOKEN_UINTEGER, t);
            } else {
              l->back = c;
              l->token[i] = 0;
              return check_bool(l, TOKEN_INTEGER, t);
            }
            break;
        }
        break;
      case 3:
        switch (c) {
          case '=': return tok(l, TOKEN_GTE, t);
          case '>': return tok(l, TOKEN_RSHIFT, t);
        }
        l->back = c;
        return tok(l, TOKEN_GT, t);
      case 4:
        switch (c) {
          case '>': return tok(l, TOKEN_NEQ, t);
          case '=': return tok(l, TOKEN_LTE, t);
          case '<': return tok(l, TOKEN_LSHIFT, t);
        }
        l->back = c;
        return tok(l, TOKEN_LT, t);
      case 5:
        if (c == '&') return tok(l, TOKEN_AND, t);
        l->back = c;
        return tok(l, TOKEN_BAND, t);
      case 6:
        if (c == '|') return tok(l, TOKEN_OR, t);
        l->back = c;
        return tok(l, TOKEN_BOR, t);
      case 8:
        if (c == '.') {
          s = 12;
        } else if (c >= '0' && c <= '9') {
          if (i == MAX_TOKEN-1) return tok(l, TOKEN_ERROR, t);
          l->token[i++] = '.';
          l->token[i++] = c;
          s = 26;
        } else {
          l->back = c;
          return tok(l, TOKEN_DOT, t);
        }
        break;
      case 9:
        if (c >= '0' && c <= '9') {
          if (i == MAX_TOKEN-1) return tok(l, TOKEN_ERROR, t);
          l->token[i++] = '-';
          l->token[i++] = c;
          s = 25;
        } else {
          l->back = c;
          return tok(l, TOKEN_MINUS, t);
        }
        break;
      case 10:
        if (c == '*') {
          s = 30;
        } else if (c == '/') {
          s = 32;
        } else {
          l->back = c;
          return tok(l, TOKEN_DIV, t);
        }
        break;
      case 11:
        if (c == '"') {
          l->token[i] = 0;
          return tok(l, TOKEN_STRING, t);
        } else {
          if (i == MAX_STRING) return tok(l, TOKEN_ERROR, t);
          l->token[i++] = c;
        }
        break;
      case 12:
        if (c == '.') return tok(l, TOKEN_VARARG, t);
        l->back = c;
        return tok(l, TOKEN_RANGE, t);
      case 21:
        if ((c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F') || (c >= '0' && c <= '9')) {
          if (i == MAX_TOKEN) return tok(l, TOKEN_ERROR, t);
          l->token[i++] = c;
        } else {
          l->back = c;
          l->token[i] = 0;
          return check_bool(l, TOKEN_HEXA, t);
        }
        break;
      case 22:
        if (c >= '0' && c <= '7') {
          if (i == MAX_TOKEN) return tok(l, TOKEN_ERROR, t);
          l->token[i++] = c;
        } else {
          l->back = c;
          l->token[i] = 0;
          return check_bool(l, TOKEN_OCTAL, t);
        }
        break;
      case 23:
        if (c == '0' || c == '1') {
          if (i == MAX_TOKEN) return tok(l, TOKEN_ERROR, t);
          l->token[i++] = c;
        } else {
          l->back = c;
          l->token[i] = 0;
          return check_bool(l, TOKEN_BINARY, t);
        }
        break;
      case 24:
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_') {
          if (i == MAX_TOKEN) return tok(l, TOKEN_ERROR, t);
          l->token[i++] = c;
        } else {
          l->back = c;
          l->token[i] = 0;
          return check_reserved(l, t);
        }
        break;
      case 25:
        if (c >= '0' && c <= '9') {
          if (i == MAX_TOKEN) return tok(l, TOKEN_ERROR, t);
          l->token[i++] = c;
        } else if (c == '.') {
          if (i == MAX_TOKEN) return tok(l, TOKEN_ERROR, t);
          l->token[i++] = c;
          s = 26;
        } else if (c == 'u') {
          l->token[i] = 0;
          return tok(l, TOKEN_UINTEGER, t);
        } else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') {
          return tok(l, TOKEN_ERROR, t);
        } else {
          l->back = c;
          l->token[i] = 0;
          return check_bool(l, TOKEN_INTEGER, t);
        }
        break;
      case 26:
        if (c >= '0' && c <= '9') {
          if (i == MAX_TOKEN) return tok(l, TOKEN_ERROR, t);
          l->token[i++] = c;
        } else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') {
          return tok(l, TOKEN_ERROR, t);
        } else {
          l->back = c;
          l->token[i] = 0;
          return tok(l, TOKEN_FLOAT, t);
        }
        break;
      case 30:
        if (c == '*') {
          s = 31;
        }
        break;
      case 31:
        if (c == '/') {
          s = 0;
        } else {
          s = 30;
        }
        break;
      case 32:
        if (c == '\n') {
          l->line++;
          s = 0;
        }
        break;
    }
  }

  return tok(l, TOKEN_ERROR, t);
}
