#include <stdio.h>
#include <stdint.h>
#include "ld80.h"

#define	CHUNK	32
/* Write a record out to an Intel Hex file.  */
/* This function is shamelessly stolen from GNU binutils, bfd/ihex.c */

static int
ihex_write_record (FILE *f, int count, int addr, int type, unsigned char *data)
{
  static const char digs[] = "0123456789ABCDEF";
  char buf[9 + CHUNK * 2 + 4];
  char *p;
  unsigned int chksum;
  unsigned int i;

#define TOHEX(buf, v) \
  ((buf)[0] = digs[((v) >> 4) & 0xf], (buf)[1] = digs[(v) & 0xf])

  buf[0] = ':';
  TOHEX (buf + 1, count);
  TOHEX (buf + 3, (addr >> 8) & 0xff);
  TOHEX (buf + 5, addr & 0xff);
  TOHEX (buf + 7, type);

  chksum = count + addr + (addr >> 8) + type;

  for (i = 0, p = buf + 9; i < count; i++, p += 2, data++)
    {
      TOHEX (p, *data);
      chksum += *data;
    }

  TOHEX (p, (- chksum) & 0xff);
  p[2] = '\r';
  p[3] = '\n';

  if (fwrite (buf, 1, 9 + count * 2 + 4, f) != 9 + count * 2 + 4)
    return 0;

  return 1;
}

static
void ihex_write_block(FILE *f, unsigned char *p, int addr, int count)
{
	int i;

	if (addr%32) {
		i=32-(addr%32);
		if (i>count) i=count;
		ihex_write_record(f, i, addr, 0, p);
		p += i;
		addr += i;
		count -= i;
	}
	while (count/32) {
		ihex_write_record(f, 32, addr, 0, p);
		p += 32;
		addr += 32;
		count -= 32;
	}
	if (count) ihex_write_record(f, count, addr, 0, p);
}

static
void cmd_write_block(FILE *f, unsigned char *p, int addr, int count)
{
	while (count > 0) {
		int len = count > 256 ? 256 : count;
		fprintf(f, "%c%c%c%c", 1, len + 2, addr, addr >> 8);
		fwrite(p, len, 1, f);
		p += len;
		addr += len;
		count -= len;
	}
}

static
void write_gap(FILE *f, int count, int oformat)
{
	unsigned char fillchar = '\xff';

	switch(oformat) {
	case F_IHEX:	/* do nothing */
		break;
	case F_BIN00:
		fillchar = '\0';
		/* FALL THROUGH */
	case F_BINFF:
		while (count--) fputc(fillchar, f);
		break;
	case F_CMD:		/* do nothing */
		break;
	default:
		die(E_USAGE, "Output format %d is unimplemented\n", oformat);
	}
}

static
void write_block(FILE *f, unsigned char *aseg, int addr,
		int section_len, int oformat)
{
	switch(oformat) {
	case F_IHEX:
		ihex_write_block(f, aseg, addr, section_len);
		break;
	case F_BIN00:
	case F_BINFF:
		fwrite(aseg, 1, section_len, f);
		break;
	case F_CMD:
		cmd_write_block(f, aseg, addr, section_len);
		break;
	default:
		die(E_USAGE, "Output format %d is unimplemented\n", oformat);
	}
}

static
void finalize_out(FILE *f, int oformat, int entry_point)
{
	switch (oformat) {
	case F_IHEX:
		if (entry_point < 0)
			entry_point = 0;

		ihex_write_record(f, 0, entry_point, 1, NULL);
		break;
	case F_BIN00:
	case F_BINFF:
		break;
	case F_CMD:
		if (entry_point >= 0)
			fprintf(f, "%c%c%c%c", 2, 2, entry_point, entry_point >> 8);
		break;
	default:
		die(E_USAGE, "Output format %d is unimplemented\n", oformat);
	}
}

static
int marked_len(int addr)
{
	int len = 0;

	while (addr<=0xffff && (addr%32) && MARKED(addr)) {
		addr++;
		len++;
	}
	while (addr<=0xffff && MARKED32(addr)) {
		addr+=32;
		len+=32;
	}
	while (addr<=0xffff && MARKED(addr)) {
		addr++;
		len++;
	}
	return len;
}

static
int unmarked_len(int addr)
{
	int len = 0;

	while (addr<=0xffff && (addr%32) && !MARKED(addr)) {
		addr++;
		len++;
	}
	while (addr<=0xffff && UNMARKED32(addr)) {
		addr+=32;
		len+=32;
	}
	while (addr<=0xffff && !MARKED(addr)) {
		addr++;
		len++;
	}
	return len;
}

int do_out(FILE *f, int oformat, int entry_point)
{
	int addr = 0;
	int gap_len, section_len;
	extern unsigned char *aseg;

	while (1) {
		gap_len = unmarked_len(addr);
		addr += gap_len;
		section_len = marked_len(addr);

		if (section_len) write_gap(f, gap_len, oformat);
		else break;

		write_block(f, aseg+addr, addr, section_len, oformat);
		addr += section_len;
	}
	finalize_out(f, oformat, entry_point);
	return 0;
}

