typedef struct pdb_t pdb_t;

pdb_t *pdb_new(char *name, char *type, char *creator);
int pdb_destroy(pdb_t *pdb);
int pdb_add_res(pdb_t *pdb, char *type, uint16_t id, uint32_t size, uint8_t *data);
int pdb_save(pdb_t *pdb, int f);
