#include "sys.h"
#include "thread.h"
#include "ptr.h"
#include "template.h"
#include "mime.h"
#include "loadfile.h"
#include "xalloc.h"
#include "debug.h"

#define MAX_COMMAND   256

struct template_t {
  char *tag;
  char prefix[256];
  char mimetype[256];
  char buf[256];
  char *body;
  unsigned int bodylen;
  char *script;
  unsigned int scriptlen;
};

template_t *template_create(char *prefix, char *filename, char *mimetype) {
  template_t *t;

  if ((t = xcalloc(1, sizeof(template_t))) != NULL) {
    t->tag = TAG_TEMPLATE;
    if (prefix) sys_strncpy(t->prefix, prefix, sizeof(t->prefix)-1);
    sys_strncpy(t->mimetype, mimetype, sizeof(t->mimetype)-1);
    sys_snprintf(t->buf, sizeof(t->buf)-1, "%s/%s", prefix, filename);
    t->body = load_file(t->buf, &t->bodylen);
  }

  return t;
}

int template_tag(template_t *t) {
  return t && ptr_check_tag(t->tag, TAG_TEMPLATE);
}

char *template_gettype(template_t *t) {
  if (template_tag(t)) {
    return t->mimetype;
  }

  return NULL;
}

char *template_getscript(template_t *t) {
  if (template_tag(t)) {
    return t->script;
  }

  return NULL;
}

static int command_get(template_t *t, char *expr, int fd) {
  sys_write(fd, (uint8_t *)"_stream_write(", 14);
  sys_write(fd, (uint8_t *)expr, sys_strlen(expr));
  sys_write(fd, (uint8_t *)")\n", 2);
  return 0;
}

static int command_set(template_t *t, char *name, char *expr, int fd) {
  int r = -1;

  if (name[0] != '_') {
    sys_write(fd, (uint8_t *)name, sys_strlen(name));
    sys_write(fd, (uint8_t *)" = ", 3);
    sys_write(fd, (uint8_t *)expr, sys_strlen(expr));
    sys_write(fd, (uint8_t *)"\n", 1);
    r = 0;

  } else {
    debug(DEBUG_ERROR, "WEB", "invalid template variable \"%s\"", name);
  }

  return r;
}

static int command_if(template_t *t, char *expr, int fd) {
  sys_write(fd, (uint8_t *)"if ", 3);
  sys_write(fd, (uint8_t *)expr, sys_strlen(expr));
  sys_write(fd, (uint8_t *)" then\n ", 6);
  return 0;
}

static int command_foreach(template_t *t, char *var, char *name, int fd) {
  sys_write(fd, (uint8_t *)"_it_pos(", 8);
  sys_write(fd, (uint8_t *)name, sys_strlen(name));
  sys_write(fd, (uint8_t *)", 0)\n", 5);

  sys_write(fd, (uint8_t *)"while _it_next(", 15);
  sys_write(fd, (uint8_t *)name, sys_strlen(name));
  sys_write(fd, (uint8_t *)") do\n", 5);

  sys_write(fd, (uint8_t *)"  local ", 8);
  sys_write(fd, (uint8_t *)var, sys_strlen(var));
  sys_write(fd, (uint8_t *)" = _it_obj(", 11);
  sys_write(fd, (uint8_t *)name, sys_strlen(name));
  sys_write(fd, (uint8_t *)")\n", 2);

  return 0;
}

static int command_while(template_t *t, char *expr, int fd) {
  sys_write(fd, (uint8_t *)"while ", 6);
  sys_write(fd, (uint8_t *)expr, sys_strlen(expr));
  sys_write(fd, (uint8_t *)" do\n", 4);
  return 0;
}

static int command_leave(template_t *t, char *expr, int fd) {
  sys_write(fd, (uint8_t *)"if ", 3);
  sys_write(fd, (uint8_t *)expr, sys_strlen(expr));
  sys_write(fd, (uint8_t *)" then break end\n ", 16);
  return 0;
}

static int command_end(template_t *t, int fd) {
  sys_write(fd, (uint8_t *)"end\n", 4);
  return 0;
}

static int command_include(template_t *t, char *filename, int fd) {
  template_t *included;
  int r = -1;

  if ((included = template_create(NULL, filename, t->mimetype)) != NULL) {
    if (template_compile(included) != -1) {
      sys_write(fd, (uint8_t *)included->script, included->scriptlen);
      r = 0;
    }
    template_destroy(included);
  }

  return r;
}

static char *next_arg(char *cmd, int *i) {
  int j, s;
  char *arg;

  s = 1;
  arg = NULL;

  for (j = 0; cmd[j]; j++) {
    if (s) {
      if (cmd[j] != ' ') {
        arg = &cmd[j];
        s = 0;
      }
    } else {
      if (cmd[j] == ' ' || cmd[j] == 0) {
        cmd[j] = 0;
        s = 1;
        if (arg) break;
      }
    }
  }

  *i = j;
  return arg;
}

static int command(template_t *t, char *begin, char *end, int fd) {
  char buf[MAX_COMMAND];
  char *cmd, *arg[2];
  int n, i;

  n = end - begin - 4;

  if (n > 0 && n < MAX_COMMAND-1) {
    sys_memset(buf, 0, sizeof(buf));
    sys_strncpy(buf, begin+2, n);
    cmd = buf;

    for (i = n-1; n > 0; n--) {
      if (cmd[i] != ' ') break;
      cmd[i] = 0;
    }

    arg[0] = next_arg(cmd, &i);

    if (arg[0]) {
      cmd = &cmd[i+1];
      i = 0;

      if (!sys_strcmp(arg[0], "get")) {
        return i < n ? command_get(t, cmd, fd) : -1;
      }

      if (!sys_strcmp(arg[0], "set")) {
        arg[1] = next_arg(cmd, &i);
        return arg[1] && i < n ? command_set(t, arg[1], &cmd[i+1], fd) : -1;
      }

      if (!sys_strcmp(arg[0], "if")) {
        return i < n ? command_if(t, cmd, fd) : -1;
      }

      if (!sys_strcmp(arg[0], "foreach")) {
        arg[1] = next_arg(cmd, &i);
        return arg[1] && i < n ? command_foreach(t, arg[1], &cmd[i+1], fd) : -1;
      }

      if (!sys_strcmp(arg[0], "while")) {
        return i < n ? command_while(t, cmd, fd) : -1;
      }

      if (!sys_strcmp(arg[0], "leave")) {
        return i < n ? command_leave(t, cmd, fd) : -1;
      }

      if (!sys_strcmp(arg[0], "end")) {
        return command_end(t, fd);
      }

      if (!sys_strcmp(arg[0], "include")) {
        return i < n ? command_include(t, cmd, fd) : -1;
      }

      debug(DEBUG_ERROR, "WEB", "unknown template command \"%s\"", arg[0]);

    } else {
      debug(DEBUG_ERROR, "WEB", "invalid template command");
    }
  }

  return -1;
}

int template_compile(template_t *t) {
  char *p, *open_escape, *close_escape;
  int fd, r = -1;

  if (template_tag(t)) {
    if ((fd = sys_mkstemp()) != -1) {
      sys_write(fd, (uint8_t *)"-- \n", 4);
      r = 0;

      for (p = t->body; p < t->body + t->bodylen && !thread_must_end();) {
        open_escape = sys_strstr(p, "<%");
        if (!open_escape) break;

        if ((open_escape - p) > 0) {
          sys_write(fd, (uint8_t *)"_stream_write([[", 16);
          sys_write(fd, (uint8_t *)p, open_escape - p);
          sys_write(fd, (uint8_t *)"]])\n", 4);
        }

        close_escape = sys_strstr(open_escape + 2, "%>");
        if (!close_escape) {
          p = t->body + t->bodylen;
          r = -1;
          break;
        }

        p = close_escape + 2;
        if (command(t, open_escape, p, fd) == -1) {
          r = -1;
          break;
        }
      }

      if (r == 0 && p < t->body + t->bodylen) {
        sys_write(fd, (uint8_t *)"_stream_write([[", 16);
        sys_write(fd, (uint8_t *)p, t->body + t->bodylen - p);
        sys_write(fd, (uint8_t *)"]])\n", 4);
      }

      if (r == 0) {
        sys_seek(fd, 0, SYS_SEEK_SET);
        t->script = load_fd(fd, &t->scriptlen);
      }
      sys_close(fd);
    }
  }

  return r;
}

int template_destroy(template_t *t) {
  int r = -1;

  if (template_tag(t)) {
    if (t->body) xfree(t->body);
    if (t->script) xfree(t->script);
    xfree(t);
    r = 0;
  }

  return r;
}
