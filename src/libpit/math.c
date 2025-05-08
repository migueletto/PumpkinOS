#include "sys.h"

#if defined(KERNEL)

#include <limits.h>
#include <float.h>

#include "ldshape.h"

#define FP_NAN       0
#define FP_INFINITE  1
#define FP_ZERO      2
#define FP_SUBNORMAL 3
#define FP_NORMAL    4

#if 0
#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
static int __fpclassify(double x) {
  union {double f; uint64_t i;} u = {x};
  int e = u.i>>52 & 0x7ff;
  if (!e) return u.i<<1 ? FP_SUBNORMAL : FP_ZERO;
  if (e==0x7ff) return u.i<<12 ? FP_NAN : FP_INFINITE;
  return FP_NORMAL;
}

static int __fpclassifyl(long double x) {
	return __fpclassify(x);
}
#elif LDBL_MANT_DIG == 64 && LDBL_MAX_EXP == 16384
static int __fpclassifyl(long double x) {
	union ldshape u = {x};
	int e = u.i.se & 0x7fff;
	int msb = u.i.m>>63;
	if (!e && !msb)
		return u.i.m ? FP_SUBNORMAL : FP_ZERO;
	if (e == 0x7fff) {
		/* The x86 variant of 80-bit extended precision only admits
		 * one representation of each infinity, with the mantissa msb
		 * necessarily set. The version with it clear is invalid/nan.
		 * The m68k variant, however, allows either, and tooling uses
		 * the version with it clear. */
		if (__BYTE_ORDER == __LITTLE_ENDIAN && !msb)
			return FP_NAN;
		return u.i.m << 1 ? FP_NAN : FP_INFINITE;
	}
	if (!msb)
		return FP_NAN;
	return FP_NORMAL;
}
#elif LDBL_MANT_DIG == 113 && LDBL_MAX_EXP == 16384
static int __fpclassifyl(long double x) {
	union ldshape u = {x};
	int e = u.i.se & 0x7fff;
	u.i.se = 0;
	if (!e)
		return u.i2.lo | u.i2.hi ? FP_SUBNORMAL : FP_ZERO;
	if (e == 0x7fff)
		return u.i2.lo | u.i2.hi ? FP_NAN : FP_INFINITE;
	return FP_NORMAL;
}
#endif

#define fpclassify(x) ( \
  sizeof(x) == sizeof(float) ? __fpclassifyf(x) : \
  sizeof(x) == sizeof(double) ? __fpclassify(x) : \
  __fpclassifyl(x) )

#define isnan(x) ( \
  sizeof(x) == sizeof(float) ? (__FLOAT_BITS(x) & 0x7fffffff) > 0x7f800000 : \
  sizeof(x) == sizeof(double) ? (__DOUBLE_BITS(x) & -1ULL>>1) > 0x7ffULL<<52 : \
  __fpclassifyl(x) == FP_NAN)

#define isnormal(x) ( \
  sizeof(x) == sizeof(float) ? ((__FLOAT_BITS(x)+0x00800000) & 0x7fffffff) >= 0x01000000 : \
  sizeof(x) == sizeof(double) ? ((__DOUBLE_BITS(x)+(1ULL<<52)) & -1ULL>>1) >= 1ULL<<53 : \
  __fpclassifyl(x) == FP_NORMAL)

#define isfinite(x) ( \
  sizeof(x) == sizeof(float) ? (__FLOAT_BITS(x) & 0x7fffffff) < 0x7f800000 : \
  sizeof(x) == sizeof(double) ? (__DOUBLE_BITS(x) & -1ULL>>1) < 0x7ffULL<<52 : \
  __fpclassifyl(x) > FP_INFINITE)
#endif

static __inline unsigned long long __DOUBLE_BITS(double __f) {
  union {double __f; unsigned long long __i;} __u;
  __u.__f = __f;
  return __u.__i;
}

double sys_floor(double x) {
  return 0;
}

double sys_ceil(double x) {
  return 0;
}

double sys_log(double x) {
  return 0;
}

double sys_exp(double x) {
  return 0;
}

double sys_sqrt(double x) {
  return 0;
}

double sys_pow(double x, double y) {
  return 0;
}

double sys_sin(double x) {
  return 0;
}

double sys_cos(double x) {
  return 0;
}

double sys_pi(void) {
  return 3.141592653589793;
}

double sys_atan2(double y, double x) {
  return 0;
}

double sys_modf(double x, double *iptr) {
  union {double f; uint64_t i;} u = {x};
  uint64_t mask;
  int e = (int)(u.i>>52 & 0x7ff) - 0x3ff;

  /* no fractional part */
  if (e >= 52) {
    *iptr = x;
    if (e == 0x400 && u.i<<12 != 0) /* nan */
      return x;
    u.i &= 1ULL<<63;
    return u.f;
  }

  /* no integral part*/
  if (e < 0) {
    u.i &= 1ULL<<63;
    *iptr = u.f;
    return x;
  }

  mask = -1ULL>>12>>e;
  if ((u.i & mask) == 0) {
    *iptr = x;
    u.i &= 1ULL<<63;
    return u.f;
  }
  u.i &= ~mask;
  *iptr = u.f;
  return x - u.f;
}

int sys_isnan(double x) {
  return (__DOUBLE_BITS(x) & -1ULL>>1) > 0x7ffULL<<52;
}

int sys_isinf(double x) {
  return (__DOUBLE_BITS(x) & -1ULL>>1) == 0x7ffULL<<52;
}

int sys_isfinite(double x) {
  return (__DOUBLE_BITS(x) & -1ULL>>1) < 0x7ffULL<<52;
}

int sys_signbit(double x) {
  return (int)(__DOUBLE_BITS(x)>>63);
}

#else

#include <math.h>

double sys_floor(double x) {
  return floor(x);
}

double sys_ceil(double x) {
  return ceil(x);
}

double sys_log(double x) {
  return log(x);
}

double sys_exp(double x) {
  return exp(x);
}

double sys_sqrt(double x) {
  return sqrt(x);
}

double sys_pow(double x, double y) {
  return pow(x, y);
}

double sys_sin(double x) {
  return sin(x);
}

double sys_cos(double x) {
  return cos(x);
}

double sys_pi(void) {
  return M_PI;
}

double sys_atan2(double y, double x) {
  return atan2(y, x);
}

long double sys_fmodl(long double x, long double y) {
  return fmodl(x, y);
}

double sys_modf(double x, double *iptr) {
  return modf(x, iptr);
}

int sys_isnan(double x) {
  return isnan(x);
}

int sys_isinf(double x) {
  return isinf(x);
}

int sys_signbit(double x) {
  return signbit(x);
}

#endif
