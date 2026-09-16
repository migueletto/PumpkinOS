#include "sys.h"
#include "dbid.h"
#include "ht.h"
#include "debug.h"

#define DBID_BASE 0x5000000
#define DBID_SIZE 0x1000000

struct dbid_t {
  uint32_t next;
  ht *h;
};

typedef struct {
  uint32_t id;
  int removed;
} hash_item_t;

dbid_t *dbid_init(void) {
  dbid_t *d;

  if ((d = sys_malloc(sizeof(dbid_t))) != NULL) {
    if ((d->h = ht_create()) != NULL) {
      d->next = 1;
      debug(DEBUG_INFO, "DBID", "dbid store created");
    } else {
      sys_free(d);
      d = NULL;
    }
  }

  return d;
}

void dbid_finish(dbid_t *d) {
  hti i;

  if (d) {
    i = ht_iterator(d->h);
    for (;;) {
      if (!ht_next(&i)) break;
      if (i.value) sys_free(i.value);
    }
    ht_destroy(d->h);
    sys_free(d);
    debug(DEBUG_INFO, "DBID", "dbid store destroyed");
  }
}

uint32_t dbid_add(dbid_t *d, char *name, uint32_t id) {
  hash_item_t *item;

  if (d && name) {
    item = ht_get(d->h, name);

    if (item) {
      if (item->removed) {
        item->id = id ? id : DBID_BASE + d->next++;
        item->removed = 0;
        ht_set(d->h, name, item);
        debug(DEBUG_TRACE, "DBID", "name \"%s\" with id 0x%08X added (replaced)", name, item->id);
        id = item->id;
      } else if (id != 0 && id != item->id) {
        debug(DEBUG_ERROR, "DBID", "name \"%s\" already exists with id 0x%08X, new id 0x%08X", name, item->id, id);
        id = 0;
      } else {
        debug(DEBUG_TRACE, "DBID", "name \"%s\" with id 0x%08X already exists", name, item->id);
      }

    } else if (id == 0) {
      if ((item = sys_malloc(sizeof(hash_item_t))) != NULL) {
        item->id = DBID_BASE + d->next++;
        ht_set(d->h, name, item);
        id = item->id;
        debug(DEBUG_TRACE, "DBID", "name \"%s\" with id 0x%08X added (generated)", name, id);
      }
      
    } else {
      if ((item = ht_get(d->h, name)) == NULL) {
        if ((item = sys_malloc(sizeof(hash_item_t))) != NULL) {
          item->id = id;
          ht_set(d->h, name, item);
          debug(DEBUG_TRACE, "DBID", "name \"%s\" with id 0x%08X added", name, id);
        }
      }
    }

  } else {
    debug(DEBUG_ERROR, "DBID", "invalid parameters not added");
    id = 0;
  }

  return id;
}

uint32_t dbid_get(dbid_t *d, char *name) {
  hash_item_t *item;
  uint32_t id = 0;

  if (d && name) {
    if ((item = ht_get(d->h, name)) != NULL) {
      if (!item->removed) {
        id = item->id;
      }
    }
  }

  return id;
}

void dbid_remove(dbid_t *d, char *name) {
  hash_item_t *item;

  if (d && name) {
    if ((item = ht_get(d->h, name)) != NULL) {
      debug(DEBUG_TRACE, "DBID", "name \"%s\" with id 0x%08X removed", name, item->id);
      item->removed = 1;
    } else {
      debug(DEBUG_ERROR, "DBID", "name \"%s\" does not exist", name);
    }
  }
}

int dbid_valid(uint32_t id) {
  return id >= DBID_BASE && id < DBID_BASE + DBID_SIZE;
}
