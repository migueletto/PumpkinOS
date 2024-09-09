#ifndef SHA1_H
#define SHA1_H

/*
   SHA-1 in C
   By Steve Reid <steve@edmweb.com>
   100% Public Domain
 */

#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

#define SHA1_HASH_LEN 20

typedef struct {
  uint32_t state[5];
  uint32_t count[2];
  unsigned char buffer[64];
} SHA1_CTX;

void SHA1Transform(uint32_t state[5], const unsigned char buffer[64]);
void SHA1Init(SHA1_CTX *context);
void SHA1Update(SHA1_CTX *context, const unsigned char *data, uint32_t len);
void SHA1Final(unsigned char digest[SHA1_HASH_LEN], SHA1_CTX *context);
void SHA1(char *hash_out, const char *str, uint32_t len);

#if defined(__cplusplus)
}
#endif

#endif /* SHA1_H */
