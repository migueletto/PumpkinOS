typedef struct dbid_t dbid_t;

dbid_t *dbid_init(void);
void dbid_finish(dbid_t *d);
uint32_t dbid_add(dbid_t *d, char *name, uint32_t id);
uint32_t dbid_get(dbid_t *d, char *name);
int dbid_valid(uint32_t id);
void dbid_remove(dbid_t *d, char *name);
