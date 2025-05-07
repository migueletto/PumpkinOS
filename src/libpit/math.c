#include "sys.h"

#if defined(KERNEL)

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
  return 0;
}

int sys_isnan(double x) {
  return 0;
}

int sys_isinf(double x) {
  return 0;
}

int sys_signbit(double x) {
  return 0;
}

#else

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
