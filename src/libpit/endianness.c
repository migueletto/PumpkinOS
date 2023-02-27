#include "sys.h"
#include "endianness.h"

static const int e = 1;
static const char *le = (char *)&e;

int little_endian(void) {
  // runtime check for endianness
  return le[0];
}

#ifdef WINDOWS32
#define swap16(x) \
        ((((x) >> 8) & 0xFF) | \
         (((x) & 0xFF) << 8))

#define swap32(x) \
        ((((x) >> 24) & 0x00FF) | \
         (((x) >>  8) & 0xFF00) | \
         (((x) & 0xFF00) <<  8) | \
         (((x) & 0x00FF) << 24))

uint16_t htole16(uint16_t host_16bits) {
  return le[0] ? host_16bits : swap16(host_16bits);
}

uint32_t htole32(uint32_t host_32bits) {
  return le[0] ? host_32bits : swap32(host_32bits);
}

uint16_t le16toh(uint16_t little_endian_16bits) {
  return le[0] ? little_endian_16bits : swap16(little_endian_16bits);
}

uint32_t le32toh(uint32_t little_endian_32bits) {
  return le[0] ? little_endian_32bits : swap32(little_endian_32bits);
}

uint16_t htobe16(uint16_t host_16bits) {
  return le[0] ? swap16(host_16bits) : host_16bits;
}

uint32_t htobe32(uint32_t host_32bits) {
  return le[0] ? swap32(host_32bits) : host_32bits;
}

uint16_t be16toh(uint16_t big_endian_16bits) {
  return le[0] ? big_endian_16bits : swap16(big_endian_16bits);
}

uint32_t be32toh(uint32_t big_endian_32bits) {
  return le[0] ? big_endian_32bits : swap32(big_endian_32bits);
}
#endif
