#ifndef PIT_ENDIANESS_H
#define PIT_ENDIANESS_H

#ifdef __cplusplus
extern "C" {
#endif

int little_endian(void);

#ifdef WINDOWS32
uint16_t htole16(uint16_t host_16bits);
uint32_t htole32(uint32_t host_32bits);
uint16_t le16toh(uint16_t little_endian_16bits);
uint32_t le32toh(uint32_t little_endian_32bits);

uint16_t htobe16(uint16_t host_16bits);
uint32_t htobe32(uint32_t host_32bits);
uint16_t be16toh(uint16_t big_endian_16bits);
uint32_t be32toh(uint32_t big_endian_32bits);
#else
#include <endian.h>
#endif

#ifdef __cplusplus
}
#endif

#endif
