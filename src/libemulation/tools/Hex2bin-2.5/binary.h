#ifndef _BINARY_H_
#define _BINARY_H_

typedef enum {false, true} bool;

extern const unsigned char Reflect8[256];

uint16_t Reflect16(uint16_t Value16);
uint32_t Reflect24(uint32_t Value24);
uint32_t Reflect32(uint32_t Value32);
uint64_t Reflect40(uint64_t Value40);
uint64_t Reflect64(uint64_t Value64);

uint8_t u16_hi(uint16_t value);
uint8_t u16_lo(uint16_t value);

uint8_t u32_b3(uint32_t value);
uint8_t u32_b2(uint32_t value);
uint8_t u32_b1(uint32_t value);
uint8_t u32_b0(uint32_t value);

uint8_t u64_b7(uint64_t value);
uint8_t u64_b6(uint64_t value);
uint8_t u64_b5(uint64_t value);
uint8_t u64_b4(uint64_t value);
uint8_t u64_b3(uint64_t value);
uint8_t u64_b2(uint64_t value);
uint8_t u64_b1(uint64_t value);
uint8_t u64_b0(uint64_t value);

uint8_t nibble2ascii(uint8_t value);
bool cs_isdecdigit(char c);
unsigned char tohex(unsigned char c);
unsigned char todecimal(unsigned char c);

#endif /* _BINARY_H_ */
