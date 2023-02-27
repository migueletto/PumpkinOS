#ifndef PIT_ENDIANESS_H
#define PIT_ENDIANESS_H

#ifdef __cplusplus
extern "C" {
#endif

int little_endian(void);

uint16_t sys_htole16(uint16_t host_16bits);
uint32_t sys_htole32(uint32_t host_32bits);
uint16_t sys_le16toh(uint16_t little_endian_16bits);
uint32_t sys_le32toh(uint32_t little_endian_32bits);

uint16_t sys_htobe16(uint16_t host_16bits);
uint32_t sys_htobe32(uint32_t host_32bits);
uint16_t sys_be16toh(uint16_t big_endian_16bits);
uint32_t sys_be32toh(uint32_t big_endian_32bits);

#ifdef __cplusplus
}
#endif

#endif
