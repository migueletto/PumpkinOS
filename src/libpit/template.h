#ifndef PIT_TEMPLATE_H
#define PIT_TEMPLATE_H

#define TAG_TEMPLATE "template"

typedef struct template_t template_t;

int template_tag(template_t *t);
template_t *template_create(char *prefix, char *filename, char *mimetype);
int template_destroy(template_t *t);
char *template_gettype(template_t *t);
char *template_getscript(template_t *t);
int template_compile(template_t *t);

#endif

