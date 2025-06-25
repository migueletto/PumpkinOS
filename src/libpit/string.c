#include <string.h>

#include "sys.h"

int sys_atoi(const char *s) {
  int n = 0, neg = 0;
  
  while (sys_isspace(*s)) s++;
  switch (*s) {
    case '-': neg = 1; 
    case '+': s++;
  }
  // Compute n as a negative number to avoid overflow on INT_MIN
  while (sys_isdigit(*s)) {
    n = 10*n - (*s++ - '0');
  }

  return neg ? n : -n;
} 

double sys_atof(const char *nptr) {
  return sys_strtod(nptr, 0);
}

char *sys_strdup(const char *s) {
  sys_size_t l = sys_strlen(s);
  char *d = sys_malloc(l+1);
  if (!d) return NULL;
  return sys_memcpy(d, s, l+1);
}

char *sys_strndup(const char *s, sys_size_t n) {
  sys_size_t l = sys_strnlen(s, n);
  char *d = sys_malloc(l+1);
  if (!d) return NULL;
  sys_memcpy(d, s, l);
  d[l] = 0;
  return d;
}

char *sys_strcpy(char *dest, const char *src) {
  for (; (*dest = *src); src++, dest++);
  return dest;
}

char *sys_strncpy(char *dest, const char *src, sys_size_t n) {
  for (; n && (*dest = *src); n--, src++, dest++);
  sys_memset(dest, 0, n);
  return dest;
}

sys_size_t sys_strlen(const char *s) {
  const char *a = s;
  for (; *s; s++);
  return s-a;
}

sys_size_t sys_strnlen(const char *s, sys_size_t n) {
  const char *p = sys_memchr(s, 0, n);
  return p ? p-s : n;
}

void *sys_memchr(const void *src, int c, sys_size_t n) {
  const unsigned char *s = src;
  c = (unsigned char)c;
  for (; n && *s != c; s++, n--);
  return n ? (void *)s : 0;
}

char *sys_strchrnul(const char *s, int c) {
  c = (unsigned char)c;
  if (!c) return (char *)s + sys_strlen(s);
  for (; *s && *(unsigned char *)s != c; s++);
  return (char *)s;
}

char *sys_strchr(const char *s, int c) {
  char *r = sys_strchrnul(s, c);
  return *(unsigned char *)r == (unsigned char)c ? r : 0;
}

void *sys_memrchr(const void *m, int c, sys_size_t n) {
  const unsigned char *s = m;
  c = (unsigned char)c;
  while (n--) if (s[n]==c) return (void *)(s+n);
  return 0;
}

char *sys_strrchr(const char *s, int c) {
  return sys_memrchr(s, c, sys_strlen(s) + 1);
}

#define BITOP(a,b,op) \
 ((a)[(sys_size_t)(b)/(8*sizeof *(a))] op (sys_size_t)1<<((sys_size_t)(b)%(8*sizeof *(a))))

sys_size_t sys_strspn(const char *s, const char *c) {
  const char *a = s;
  sys_size_t byteset[32/sizeof(sys_size_t)] = { 0 };

  if (!c[0]) return 0;
  if (!c[1]) {
    for (; *s == *c; s++);
    return s-a;
  }

  for (; *c && BITOP(byteset, *(unsigned char *)c, |=); c++);
  for (; *s && BITOP(byteset, *(unsigned char *)s, &); s++);
  return s-a;
}

sys_size_t sys_strcspn(const char *s, const char *c) {
  const char *a = s;
  sys_size_t byteset[32/sizeof(sys_size_t)];

  if (!c[0] || !c[1]) return sys_strchrnul(s, *c)-a;

  sys_memset(byteset, 0, sizeof byteset);
  for (; *c && BITOP(byteset, *(unsigned char *)c, |=); c++);
  for (; *s && !BITOP(byteset, *(unsigned char *)s, &); s++);
  return s-a;
}

char *sys_strpbrk(const char *s, const char *accept) {
  s += sys_strcspn(s, accept);
  return *s ? (char *)s : 0;
}

int sys_strcmp(const char *l, const char *r) {
  for (; *l==*r && *l; l++, r++);
  return *(unsigned char *)l - *(unsigned char *)r;
}

int sys_strncmp(const char *_l, const char *_r, sys_size_t n) {
  const unsigned char *l=(void *)_l, *r=(void *)_r;
  if (!n--) return 0;
  for (; *l && *r && n && *l == *r ; l++, r++, n--);
  return *l - *r;
}

int sys_strcasecmp(const char *_l, const char *_r) {
  const unsigned char *l=(void *)_l, *r=(void *)_r;
  for (; *l && *r && (*l == *r || sys_tolower(*l) == sys_tolower(*r)); l++, r++);
  return sys_tolower(*l) - sys_tolower(*r);
}

int sys_strncasecmp(const char *_l, const char *_r, sys_size_t n) {
  const unsigned char *l=(void *)_l, *r=(void *)_r;
  if (!n--) return 0;
  for (; *l && *r && n && (*l == *r || sys_tolower(*l) == sys_tolower(*r)); l++, r++, n--);
  return sys_tolower(*l) - sys_tolower(*r);
}

char *sys_strcat(char *dest, const char *src) {
  sys_strcpy(dest + sys_strlen(dest), src);
  return dest;
}

char *sys_strncat(char *dest, const char *src, sys_size_t n) {
  char *a = dest;
  dest += sys_strlen(dest);
  while (n && *src) n--, *dest++ = *src++;
  *dest++ = 0;
  return a;
}

int sys_memcmp(const void *vl, const void *vr, sys_size_t n) {
#if defined(KERNEL)
  const unsigned char *l = vl, *r = vr;
  for (; n && *l == *r; n--, l++, r++);
  return n ? *l-*r : 0;
#else
  return memcmp(vl, vr, n);
#endif
}

void *sys_memcpy(void *dest, const void *src, sys_size_t n) {
#if defined(KERNEL)
  unsigned char *d = dest;
  const unsigned char *s = src;
  for (; n; n--) *d++ = *s++;
  return dest;
#else
  return memcpy(dest, src, n);
#endif
}

void *sys_memmove(void *dest, const void *src, sys_size_t n) {
#if defined(KERNEL)
  char *d = dest;
  const char *s = src;

  if (d == s) return d;
  if ((uintptr_t)s - (uintptr_t)d - n <= -2*n) return sys_memcpy(d, s, n);

  if (d < s) {
    for (; n; n--) *d++ = *s++;
  } else {
    while (n) n--, d[n] = s[n];
  }

  return dest;
#else
  return memmove(dest, src,  n);
#endif
}

void *sys_memset(void *dest, int c, sys_size_t n) {
#if defined(KERNEL)
  unsigned char *d = dest;
  for (; n; n--) *d++ = c;
  return dest;
#else
  return memset(dest, c, n);
#endif
}

static char *twobyte_strstr(const unsigned char *h, const unsigned char *n) {
  uint16_t nw = n[0]<<8 | n[1], hw = h[0]<<8 | h[1];
  for (h++; *h && hw != nw; hw = hw<<8 | *++h);
  return *h ? (char *)h-1 : 0;
}

static char *threebyte_strstr(const unsigned char *h, const unsigned char *n) {
  uint32_t nw = (uint32_t)n[0]<<24 | n[1]<<16 | n[2]<<8;
  uint32_t hw = (uint32_t)h[0]<<24 | h[1]<<16 | h[2]<<8;
  for (h+=2; *h && hw != nw; hw = (hw|*++h)<<8);
  return *h ? (char *)h-2 : 0;
}

static char *fourbyte_strstr(const unsigned char *h, const unsigned char *n) {
  uint32_t nw = (uint32_t)n[0]<<24 | n[1]<<16 | n[2]<<8 | n[3];
  uint32_t hw = (uint32_t)h[0]<<24 | h[1]<<16 | h[2]<<8 | h[3];
  for (h+=3; *h && hw != nw; hw = hw<<8 | *++h);
  return *h ? (char *)h-3 : 0;
}

#define MAX(a,b) ((a)>(b)?(a):(b))
#define MIN(a,b) ((a)<(b)?(a):(b))

#undef BITOP
#define BITOP(a,b,op) \
 ((a)[(uint32_t)(b)/(8*sizeof *(a))] op (uint32_t)1<<((uint32_t)(b)%(8*sizeof *(a))))

static char *twoway_strstr(const unsigned char *h, const unsigned char *n) {
  const unsigned char *z;
  uint32_t l, ip, jp, k, p, ms, p0, mem, mem0;
  uint32_t byteset[32 / sizeof(uint32_t)] = { 0 };
  uint32_t shift[256];

  /* Computing length of needle and fill shift table */
  for (l=0; n[l] && h[l]; l++)
    BITOP(byteset, n[l], |=), shift[n[l]] = l+1;
  if (n[l]) return 0; /* hit the end of h */

  /* Compute maximal suffix */
  ip = -1; jp = 0; k = p = 1;
  while (jp+k<l) {
    if (n[ip+k] == n[jp+k]) {
      if (k == p) {
        jp += p;
        k = 1;
      } else k++;
    } else if (n[ip+k] > n[jp+k]) {
      jp += k;
      k = 1;
      p = jp - ip;
    } else {
      ip = jp++;
      k = p = 1;
    }
  }
  ms = ip;
  p0 = p;

  /* And with the opposite comparison */
  ip = -1; jp = 0; k = p = 1;
  while (jp+k<l) {
    if (n[ip+k] == n[jp+k]) {
      if (k == p) {
        jp += p;
        k = 1;
      } else k++;
    } else if (n[ip+k] < n[jp+k]) {
      jp += k;
      k = 1;
      p = jp - ip;
    } else {
      ip = jp++;
      k = p = 1;
    }
  }
  if (ip+1 > ms+1) ms = ip;
  else p = p0;

  /* Periodic needle? */
  if (sys_memcmp(n, n+p, ms+1)) {
    mem0 = 0;
    p = MAX(ms, l-ms-1) + 1;
  } else mem0 = l-p;
  mem = 0;

  /* Initialize incremental end-of-haystack pointer */
  z = h;

  /* Search loop */
  for (;;) {
    /* Update incremental end-of-haystack pointer */
    if (z-h < l) {
      /* Fast estimate for MAX(l,63) */
      uint32_t grow = l | 63;
      const unsigned char *z2 = sys_memchr(z, 0, grow);
      if (z2) {
        z = z2;
        if (z-h < l) return 0;
      } else z += grow;
    }

    /* Check last byte first; advance by shift on mismatch */
    if (BITOP(byteset, h[l-1], &)) {
      k = l-shift[h[l-1]];
      if (k) {
        if (k < mem) k = mem;
        h += k;
        mem = 0;
        continue;
      }
    } else {
      h += l;
      mem = 0;
      continue;
    }

    /* Compare right half */
    for (k=MAX(ms+1,mem); n[k] && n[k] == h[k]; k++);
    if (n[k]) {
      h += k-ms;
      mem = 0;
      continue;
    }
    /* Compare left half */
    for (k=ms+1; k>mem && n[k-1] == h[k-1]; k--);
    if (k <= mem) return (char *)h;
    h += p;
    mem = mem0;
  }
}

char *sys_strstr(const char *h, const char *n) {
  /* Return immediately on empty needle */
  if (!n[0]) return (char *)h;

  /* Use faster algorithms for short needles */
  h = sys_strchr(h, *n);
  if (!h || !n[1]) return (char *)h;
  if (!h[1]) return 0;
  if (!n[2]) return twobyte_strstr((void *)h, (void *)n);
  if (!h[2]) return 0;
  if (!n[3]) return threebyte_strstr((void *)h, (void *)n);
  if (!h[3]) return 0;
  if (!n[4]) return fourbyte_strstr((void *)h, (void *)n);

  return twoway_strstr((void *)h, (void *)n);
}
