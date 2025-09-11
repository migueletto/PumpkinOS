#define EXE_MAGIC 'xecf'
#define OBJ_MAGIC 'bocf'

#define SEC_CODE  'edoc'
#define SEC_SYMB  'bmys'
#define SYM_CONS  'snoc'
#define SYM_FUNC  'cnuf'
#define SYM_RELR  'rler'
#define SYM_ABSR  'rsba'

uint8_t *section_read(uint32_t id, char *name, uint32_t *size, PLIBC_FILE *f);
