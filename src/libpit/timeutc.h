#ifndef PIT_TIMEUTC_H
#define PIT_TIMEUTC_H

#ifdef __cplusplus
extern "C" {
#endif

uint64_t timeutc(sys_tm_t *tm);

sys_tm_t *utctime(const uint64_t *t, sys_tm_t *tm);

int timeofday(sys_timeval_t *tv);

int daysinmonth(int year, int month);

#ifdef __cplusplus
}
#endif

#endif
