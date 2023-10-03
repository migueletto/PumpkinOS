    /********************************************************************
    *                                                                   *
    *   Library         : lib_crc                                       *
    *   File            : lib_crc.h                                     *
    *   Author          : Lammert Bies  1999-2008                       *
    *   E-mail          : info@lammertbies.nl                           *
    *   Language        : ANSI C                                        *
    *                                                                   *
    *                                                                   *
    *   Description                                                     *
    *   ===========                                                     *
    *                                                                   *
    *   The file lib_crc.h contains public definitions  and  proto-     *
    *   types for the CRC functions present in lib_crc.c.               *
    *                                                                   *
    *                                                                   *
    *   Dependencies                                                    *
    *   ============                                                    *
    *                                                                   *
    *   none                                                            *
    *                                                                   *
    *                                                                   *
    ********************************************************************/
#ifndef _LIBCRC_H_
#define _LIBCRC_H_

extern void *crc_table;

void init_crc8_normal_tab(uint8_t polynom);
void init_crc8_reflected_tab(uint8_t polynom);

void init_crc16_normal_tab(uint16_t polynom);
void init_crc16_reflected_tab(uint16_t polynom);
void init_crc32_normal_tab(uint32_t polynom);
void init_crc32_reflected_tab(uint32_t polynom);

uint8_t update_crc8(uint8_t crc, uint8_t c);

uint16_t update_crc16_normal(uint16_t crc, char c );
uint16_t update_crc16_reflected(uint16_t crc, char c );
uint32_t update_crc32_normal(uint32_t crc, char c );
uint32_t update_crc32_reflected(uint32_t crc, char c );

#endif /* _LIBCRC_H_ */
