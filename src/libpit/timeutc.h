#ifndef PIT_TIMEUTC_H
#define PIT_TIMEUTC_H

#ifdef __cplusplus
extern "C" {
#endif

time_t timeutc(struct tm *tm);

struct tm *utctime(const time_t *t, struct tm *tm);

int timeofday(struct timeval *tv);

int daysinmonth(int year, int month);

#ifdef __cplusplus
}
#endif

#endif
