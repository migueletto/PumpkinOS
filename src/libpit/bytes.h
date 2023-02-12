#ifndef PIT_BYTES_H
#define PIT_BYTES_H

#ifdef __cplusplus
extern "C" {
#endif

int putID(char *id, uint8_t *buf, int i);

int getID(char *id, uint8_t *buf, int i);

int put1(uint8_t w, uint8_t *buf, int i);

int put2l(uint16_t w, uint8_t *buf, int i);

int put4l(uint32_t w, uint8_t *buf, int i);

int put2b(uint16_t w, uint8_t *buf, int i);

int put4b(uint32_t w, uint8_t *buf, int i);

int get1(uint8_t *w, uint8_t *buf, int i);

int get2l(uint16_t *w, uint8_t *buf, int i);

int get4l(uint32_t *w, uint8_t *buf, int i);

int get2b(uint16_t *w, uint8_t *buf, int i);

int get4b(uint32_t *w, uint8_t *buf, int i);

#ifdef __cplusplus
}
#endif

#endif
