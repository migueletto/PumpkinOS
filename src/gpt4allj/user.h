#ifdef  __cplusplus
extern "C" {
#endif

int user_init(uint16_t port);
void user_finish(void);
int user_input(char *buf, uint16_t max);
int user_output(const char *buf);

#ifdef  __cplusplus
}
#endif
