#include "sys.h"
#include "floatscan.h"
#include "intscan.h"
#include "vsscanf.h"

#include <limits.h>

#define SIZE_hh -2
#define SIZE_h  -1
#define SIZE_def 0
#define SIZE_l   1
#define SIZE_L   2
#define SIZE_ll  3

#define MB_CUR_MAX 1
#define CODEUNIT(c) (0xdfff & (signed char)(c))
#define OOB(c,b) (((((b)>>3)-0x10)|(((b)>>3)+((int32_t)(c)>>26))) & ~7)
#define SA 0xc2u
#define SB 0xf4u

#define R(a,b) ((uint32_t)((a==0x80 ? 0x40u-b : 0u-a) << 23))
#define C(x) ( x<2 ? -1 : ( R(0x80,0xc0) | x ) )
#define D(x) C((x+16))
#define E(x) ( ( x==0 ? R(0xa0,0xc0) : \
                 x==0xd ? R(0x80,0xa0) : \
                 R(0x80,0xc0) ) \
             | ( R(0x80,0xc0) >> 6 ) \
             | x )
#define F(x) ( ( x>=5 ? 0 : \
                 x==0 ? R(0x90,0xc0) : \
                 x==4 ? R(0x80,0x90) : \
                 R(0x80,0xc0) ) \
             | ( R(0x80,0xc0) >> 6 ) \
             | ( R(0x80,0xc0) >> 12 ) \
             | x )

static const uint32_t bittab[] = {
                C(0x2),C(0x3),C(0x4),C(0x5),C(0x6),C(0x7),
  C(0x8),C(0x9),C(0xa),C(0xb),C(0xc),C(0xd),C(0xe),C(0xf),
  D(0x0),D(0x1),D(0x2),D(0x3),D(0x4),D(0x5),D(0x6),D(0x7),
  D(0x8),D(0x9),D(0xa),D(0xb),D(0xc),D(0xd),D(0xe),D(0xf),
  E(0x0),E(0x1),E(0x2),E(0x3),E(0x4),E(0x5),E(0x6),E(0x7),
  E(0x8),E(0x9),E(0xa),E(0xb),E(0xc),E(0xd),E(0xe),E(0xf),
  F(0x0),F(0x1),F(0x2),F(0x3),F(0x4)
};

typedef struct __mbstate_t { unsigned __opaque1, __opaque2; } mbstate_t;

static int mbsinit(const mbstate_t *st) {
  return !st || !*(unsigned *)st;
}

static sys_size_t mbrtowc(uint16_t *wc, const char *src, sys_size_t n, mbstate_t *st) {   
  static unsigned internal_state;
  unsigned c;
  const unsigned char *s = (const void *)src;
  const sys_size_t N = n;
  uint16_t dummy;
  
  if (!st) st = (void *)&internal_state;
  c = *(unsigned *)st;
 
  if (!s) {
    if (c) goto ilseq;
    return 0;
  } else if (!wc) wc = &dummy;

  if (!n) return -2;
  if (!c) {
    if (*s < 0x80) return !!(*wc = *s);
    if (MB_CUR_MAX==1) return (*wc = CODEUNIT(*s)), 1;
    if (*s-SA > SB-SA) goto ilseq;
    c = bittab[*s++-SA]; n--;
  }

  if (n) {
    if (OOB(c,*s)) goto ilseq;
loop:
    c = (c<<6) | ((*s++)-0x80); n--;
    if (!(c&(1U<<31))) {
      *(unsigned *)st = 0;
      *wc = c;
      return N-n;
    }
    if (n) {
      if (*s-0x80u >= 0x40) goto ilseq;
      goto loop;
    }
  }

  *(unsigned *)st = c;
  return -2;
ilseq:
  *(unsigned *)st = 0;
  return -1;
}

static void shunget(floatscan_t *f) {
  if (f->pos > 0) {
    f->pos--;
  }
}

static int shgetc(floatscan_t *f) {
  int c = -1;

  if (f->pos < f->size && (f->lim == 0 || f->pos <= f->lim)) {
    c = f->buffer[f->pos++];
  }

  return c;
}

static void store_int(void *dest, int size, unsigned long long i) {
	if (!dest) return;
	switch (size) {
	case SIZE_hh:
		*(char *)dest = i;
		break;
	case SIZE_h:
		*(short *)dest = i;
		break;
	case SIZE_def:
		*(int *)dest = i;
		break;
	case SIZE_l:
		*(long *)dest = i;
		break;
	case SIZE_ll:
		*(long long *)dest = i;
		break;
	}
}

static void *arg_n(sys_va_list ap, unsigned int n) {
	void *p;
	unsigned int i;
	sys_va_list ap2;
	sys_va_copy(ap2, ap);
	for (i=n; i>1; i--) sys_va_arg(ap2, void *);
	p = sys_va_arg(ap2, void *);
	sys_va_end(ap2);
	return p;
}

int my_vsscanf(const char *str, const char *fmt, sys_va_list ap) {
  floatscan_t f;
	int width;
	int size;
	int alloc = 0;
	int base;
	const unsigned char *p;
	int c, t;
	char *s;
	uint16_t *wcs;
	mbstate_t st;
	void *dest=NULL;
	int invert;
	int matches=0;
	unsigned long long x;
	long double y;
	unsigned char scanset[257];
	sys_size_t i, k;
	uint16_t wc;

  if (str == NULL || fmt == NULL) return -1;

  f.buffer = (char *)str;
  f.pos = 0;
  f.size = sys_strlen(str);

	for (p=(const unsigned char *)fmt; *p; p++) {
		alloc = 0;

		if (sys_isspace(*p)) {
			while (sys_isspace(p[1])) p++;
      f.lim = 0;
			while (sys_isspace(shgetc(&f)));
			shunget(&f);
			continue;
		}
		if (*p != '%' || p[1] == '%') {
      f.lim = 0;
			if (*p == '%') {
				p++;
				while (sys_isspace((c=shgetc(&f))));
			} else {
				c = shgetc(&f);
			}
			if (c!=*p) {
				shunget(&f);
				if (c<0) goto input_fail;
				goto match_fail;
			}
			continue;
		}

		p++;
		if (*p=='*') {
			dest = 0; p++;
		} else if (sys_isdigit(*p) && p[1]=='$') {
			dest = arg_n(ap, *p-'0'); p+=2;
		} else {
			dest = sys_va_arg(ap, void *);
		}

		for (width=0; sys_isdigit(*p); p++) {
			width = 10*width + *p - '0';
		}

		if (*p=='m') {
			wcs = 0;
			s = 0;
			alloc = !!dest;
			p++;
		} else {
			alloc = 0;
		}

		size = SIZE_def;
		switch (*p++) {
		case 'h':
			if (*p == 'h') p++, size = SIZE_hh;
			else size = SIZE_h;
			break;
		case 'l':
			if (*p == 'l') p++, size = SIZE_ll;
			else size = SIZE_l;
			break;
		case 'j':
			size = SIZE_ll;
			break;
		case 'z':
		case 't':
			size = SIZE_l;
			break;
		case 'L':
			size = SIZE_L;
			break;
		case 'd': case 'i': case 'o': case 'u': case 'x':
		case 'a': case 'e': case 'f': case 'g':
		case 'A': case 'E': case 'F': case 'G': case 'X':
		case 's': case 'c': case '[':
		case 'S': case 'C':
		case 'p': case 'n':
			p--;
			break;
		default:
			goto fmt_fail;
		}

		t = *p;

		/* C or S */
		if ((t&0x2f) == 3) {
			t |= 32;
			size = SIZE_l;
		}

		switch (t) {
		case 'c':
			if (width < 1) width = 1;
		case '[':
			break;
		case 'n':
			store_int(dest, size, f.pos);
			/* do not increment match count, etc! */
			continue;
		default:
      f.lim = 0;
			while (sys_isspace(shgetc(&f)));
			shunget(&f);
		}

    f.lim = width;
		if (shgetc(&f) < 0) goto input_fail;
		shunget(&f);

		switch (t) {
		case 's':
		case 'c':
		case '[':
			if (t == 'c' || t == 's') {
				sys_memset(scanset, -1, sizeof scanset);
				scanset[0] = 0;
				if (t == 's') {
					scanset[1+'\t'] = 0;
					scanset[1+'\n'] = 0;
					scanset[1+'\v'] = 0;
					scanset[1+'\f'] = 0;
					scanset[1+'\r'] = 0;
					scanset[1+' '] = 0;
				}
			} else {
				if (*++p == '^') p++, invert = 1;
				else invert = 0;
				sys_memset(scanset, invert, sizeof scanset);
				scanset[0] = 0;
				if (*p == '-') p++, scanset[1+'-'] = 1-invert;
				else if (*p == ']') p++, scanset[1+']'] = 1-invert;
				for (; *p != ']'; p++) {
					if (!*p) goto fmt_fail;
					if (*p=='-' && p[1] && p[1] != ']')
						for (c=p++[-1]; c<*p; c++)
							scanset[1+c] = 1-invert;
					scanset[1+*p] = 1-invert;
				}
			}
			wcs = 0;
			s = 0;
			i = 0;
			k = t=='c' ? width+1U : 31;
			if (size == SIZE_l) {
				if (alloc) {
					wcs = sys_malloc(k*sizeof(uint16_t));
					if (!wcs) goto alloc_fail;
				} else {
					wcs = dest;
				}
				st = (mbstate_t){0};
				while (scanset[(c=shgetc(&f))+1]) {
					switch (mbrtowc(&wc, &(char){c}, 1, &st)) {
					case -1:
						goto input_fail;
					case -2:
						continue;
					}
					if (wcs) wcs[i++] = wc;
					if (alloc && i==k) {
						k+=k+1;
						uint16_t *tmp = sys_realloc(wcs, k*sizeof(uint16_t));
						if (!tmp) goto alloc_fail;
						wcs = tmp;
					}
				}
				if (!mbsinit(&st)) goto input_fail;
			} else if (alloc) {
				s = sys_malloc(k);
				if (!s) goto alloc_fail;
				while (scanset[(c=shgetc(&f))+1]) {
					s[i++] = c;
					if (i==k) {
						k+=k+1;
						char *tmp = sys_realloc(s, k);
						if (!tmp) goto alloc_fail;
						s = tmp;
					}
				}
			} else if ((s = dest)) {
				while (scanset[(c=shgetc(&f))+1])
					s[i++] = c;
			} else {
				while (scanset[(c=shgetc(&f))+1]);
			}
			shunget(&f);
			if (!f.pos) goto match_fail;
			if (t == 'c' && f.pos != width) goto match_fail;
			if (alloc) {
				if (size == SIZE_l) *(uint16_t **)dest = wcs;
				else *(char **)dest = s;
			}
			if (t != 'c') {
				if (wcs) wcs[i] = 0;
				if (s) s[i] = 0;
			}
			break;
		case 'p':
		case 'X':
		case 'x':
			base = 16;
			goto int_common;
		case 'o':
			base = 8;
			goto int_common;
		case 'd':
		case 'u':
			base = 10;
			goto int_common;
		case 'i':
			base = 0;
		int_common:
			x = intscan(&f, base, 0, ULLONG_MAX);
			if (!f.pos) goto match_fail;
			if (t=='p' && dest) *(void **)dest = (void *)(uintptr_t)x;
			else store_int(dest, size, x);
			break;
		case 'a': case 'A':
		case 'e': case 'E':
		case 'f': case 'F':
		case 'g': case 'G':
			y = floatscan(&f, size, 0);
			if (!f.pos) goto match_fail;
			if (dest) switch (size) {
			case SIZE_def:
				*(float *)dest = y;
				break;
			case SIZE_l:
				*(double *)dest = y;
				break;
			case SIZE_L:
				*(long double *)dest = y;
				break;
			}
			break;
		}

		if (dest) matches++;
	}
	if (0) {
fmt_fail:
alloc_fail:
input_fail:
		if (!matches) matches--;
match_fail:
		if (alloc) {
			sys_free(s);
			sys_free(wcs);
		}
	}

	return matches;
}
