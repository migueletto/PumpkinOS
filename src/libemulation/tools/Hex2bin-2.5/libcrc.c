/*********************************************************************
 *                                                                   *
 *   Library         : lib_crc                                       *
 *   File            : lib_crc.c                                     *
 *   Author          : Lammert Bies  1999-2008                       *
 *   E-mail          : info@lammertbies.nl                           *
 *   Language        : ANSI C                                        *
 *                                                                   *
 *                                                                   *
 *   Description                                                     *
 *   ===========                                                     *
 *                                                                   *
 *   The file lib_crc.c contains the private  and  public  func-     *
 *   tions  used  for  the  calculation of CRC-16, CRC-CCITT and     *
 *   CRC-32 cyclic redundancy values.                                *
 *                                                                   *
 *                                                                   *
 *   Dependencies                                                    *
 *   ============                                                    *
 *                                                                   *
 *   libcrc.h       CRC definitions and prototypes                   *
 *                                                                   *
 ********************************************************************/
#include <stdint.h>

#ifndef G_GUINT64_CONSTANT
#define G_GUINT64_CONSTANT(val) (val##UL)
#endif

void *crc_table;

/* private */

void init_crc8_normal_tab(uint8_t polynom)
{
  int i, j;
  uint8_t crc;
  uint8_t *p;

  p = (uint8_t *) crc_table;

  for (i=0; i<256; i++)
    {
	  crc = (uint8_t) i;

	  for (j=0; j<8; j++)
        {
          if (crc & 0x80) crc = (crc << 1) ^ polynom;
          else            crc <<= 1;
        }
	  *p++ = crc;
    }
}

void init_crc8_reflected_tab(uint8_t polynom)
{
  int i, j;
  uint8_t crc;
  uint8_t *p;

  p = (uint8_t *) crc_table;

  for (i=0; i<256; i++)
    {
	  crc = (uint8_t) i;

	  for (j=0; j<8; j++)
        {
          if (crc & 0x01) crc = (crc >> 1) ^ polynom;
          else            crc >>= 1;
        }
	  *p++ = crc;
    }
}

/* Common routines for calculations */
void init_crc16_normal_tab(uint16_t polynom)
{
  int i, j;
  uint16_t crc;
  uint16_t *p;

  p = (uint16_t *) crc_table;

  for (i=0; i<256; i++)
    {
	  crc = ((uint16_t) i) << 8;

	  for (j=0; j<8; j++)
        {
		  if ( crc & 0x8000 ) crc = ( crc << 1 ) ^ polynom;
		  else                crc <<= 1;
        }
	  *p++ = crc;
    }
}

void init_crc16_reflected_tab(uint16_t polynom)
{
  int i, j;
  uint16_t crc;
  uint16_t *p;

  p = (uint16_t *) crc_table;

  for (i=0; i<256; i++)
    {
	  crc   = (uint16_t) i;

	  for (j=0; j<8; j++)
        {
		  if ( crc & 0x0001 ) crc = ( crc >> 1 ) ^ polynom;
		  else                crc >>= 1;
        }
	  *p++ = crc;
    }
}

void init_crc32_normal_tab(uint32_t polynom)
{
  int i, j;
  uint32_t crc;
  uint32_t *p;

  p = (uint32_t *) crc_table;

  for (i=0; i<256; i++)
    {
	  crc = ((uint32_t) i) << 24;

	  for (j=0; j<8; j++)
        {
		  if ( crc & 0x80000000L ) crc = ( crc << 1 ) ^ polynom;
		  else                     crc <<= 1;
        }
	  *p++ = crc;
    }
}

void init_crc32_reflected_tab(uint32_t polynom)
{
  int i, j;
  uint32_t crc;
  uint32_t *p;

  p = (uint32_t *) crc_table;

  for (i=0; i<256; i++)
    {
	  crc = (uint32_t) i;

	  for (j=0; j<8; j++)
        {
		  if ( crc & 0x00000001L ) crc = ( crc >> 1 ) ^ polynom;
		  else                     crc >>= 1;
        }
	  *p++ = crc;
    }
}

/* Common routines for calculations */

uint8_t update_crc8(uint8_t crc, uint8_t c)
{
  return (((uint8_t *) crc_table)[crc ^ c]);
}

uint16_t update_crc16_normal(uint16_t crc, char c )
{
  uint16_t short_c;

  short_c  = 0x00ff & (uint16_t) c;

  /* Normal form */
  return (crc << 8) ^ ((uint16_t *) crc_table)[(crc >> 8) ^ short_c];
}

uint16_t update_crc16_reflected(uint16_t crc, char c )
{
  uint16_t short_c;

  short_c  = 0x00ff & (uint16_t) c;

  /* Reflected form */
  return (crc >> 8) ^ ((uint16_t *) crc_table)[(crc ^ short_c) & 0xff];
}

uint32_t update_crc32_normal(uint32_t crc, char c )
{
  uint32_t long_c;

  long_c = 0x000000ffL & (uint32_t) c;

  return (crc << 8) ^ ((uint32_t *) crc_table)[((crc >> 24) ^ long_c) & 0xff];
}

uint32_t update_crc32_reflected(uint32_t crc, char c )
{
  uint32_t long_c;

  long_c = 0x000000ffL & (uint32_t) c;

  return (crc >> 8) ^ ((uint32_t *) crc_table)[(crc ^ long_c) & 0xff];
}
