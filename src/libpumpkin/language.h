#define langUTF8 lUnused

typedef struct {
  LocalID dbID;
  DmOpenRef dbRef;
  UInt32 (*nextChar)(UInt8 *s, UInt32 i, UInt32 len, UInt32 *w, void *data);
  UInt8 (*mapChar)(UInt32 w, FontType **f, void *data);
  void (*finish)(void *data);
  void *data;
} language_t;

typedef Err (*langSetF)(language_t *lang);

language_t *LanguageInit(UInt32 id);
int LanguageFinish(language_t *lang);

language_t *LanguageSelect(language_t *lang);
