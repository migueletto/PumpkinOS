#ifndef PIT_TS_H
#define PIT_TS_H

#ifdef __cplusplus
extern "C" {
#endif

time_t time2ts(int day, int month, int year, int hour, int min, int sec);

void ts2time(time_t ts, int *day, int *month, int *year, int *wday, int *hour, int *min, int *sec);

#ifdef __cplusplus
}
#endif

#endif
