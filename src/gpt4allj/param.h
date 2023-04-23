#ifdef  __cplusplus
extern "C" {
#endif

typedef struct {
  const char *name;
  char type;
  void *p;
  int32_t imin, imax;
  float fmin, fmax;
  const char *help;
} param_info_t;

int param_check(param_info_t pinfo[], char *name, char *value);
void param_usage(param_info_t pinfo[]);

#ifdef  __cplusplus
}
#endif
