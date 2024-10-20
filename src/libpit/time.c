#include "sys.h"

static int64_t __year_to_secs(int64_t year, int *is_leap) {
	if (year-2 <= 136) {
		int y = year;
		int leaps = (y-68)>>2;
		if (!((y-68)&3)) {
			leaps--;
			if (is_leap) *is_leap = 1;
		} else if (is_leap) *is_leap = 0;
		return 31536000*(y-70) + 86400*leaps;
	}

	int cycles, centuries, leaps, rem, dummy;

	if (!is_leap) is_leap = &dummy;
	cycles = (year-100) / 400;
	rem = (year-100) % 400;
	if (rem < 0) {
		cycles--;
		rem += 400;
	}
	if (!rem) {
		*is_leap = 1;
		centuries = 0;
		leaps = 0;
	} else {
		if (rem >= 200) {
			if (rem >= 300) centuries = 3, rem -= 300;
			else centuries = 2, rem -= 200;
		} else {
			if (rem >= 100) centuries = 1, rem -= 100;
			else centuries = 0;
		}
		if (!rem) {
			*is_leap = 0;
			leaps = 0;
		} else {
			leaps = rem / 4U;
			rem %= 4U;
			*is_leap = !rem;
		}
	}

	leaps += 97*cycles + 24*centuries - *is_leap;

	return (year-100) * 31536000LL + leaps * 86400LL + 946684800 + 86400;
}

static int __month_to_secs(int month, int is_leap) {
  static const int secs_through_month[] = {
    0, 31*86400, 59*86400, 90*86400,
    120*86400, 151*86400, 181*86400, 212*86400,
    243*86400, 273*86400, 304*86400, 334*86400
  };
  int t = secs_through_month[month];
  if (is_leap && month >= 2) t += 86400;

  return t;
}

static int64_t __tm_to_secs(sys_tm_t *tm) {
  int64_t t, year = tm->tm_year;
  int month = tm->tm_mon;
  int is_leap;

  if (month >= 12 || month < 0) {
    int adj = month / 12;
    month %= 12;
    if (month < 0) {
      adj--;
      month += 12;
    }
    year += adj;
  }
  t = __year_to_secs(year, &is_leap);
  t += __month_to_secs(month, is_leap);
  t += 86400LL * (tm->tm_mday-1);
  t += 3600LL * tm->tm_hour;
  t += 60LL * tm->tm_min;
  t += tm->tm_sec;

  return t;
}

// 2000-03-01 (mod 400 year, immediately after feb29
#define LEAPOCH (946684800LL + 86400*(31+29))

#define DAYS_PER_400Y (365*400 + 97)
#define DAYS_PER_100Y (365*100 + 24)
#define DAYS_PER_4Y   (365*4   + 1)

static int __secs_to_tm(int64_t t, sys_tm_t *tm) {
	int64_t days, secs, years;
	int remdays, remsecs, remyears;
	int qc_cycles, c_cycles, q_cycles;
	int months;
	int wday, yday, leap;
	static const char days_in_month[] = {31,30,31,30,31,31,30,31,30,31,31,29};

	secs = t - LEAPOCH;
	days = secs / 86400;
	remsecs = secs % 86400;
	if (remsecs < 0) {
		remsecs += 86400;
		days--;
	}

	wday = (3+days)%7;
	if (wday < 0) wday += 7;

	qc_cycles = days / DAYS_PER_400Y;
	remdays = days % DAYS_PER_400Y;
	if (remdays < 0) {
		remdays += DAYS_PER_400Y;
		qc_cycles--;
	}

	c_cycles = remdays / DAYS_PER_100Y;
	if (c_cycles == 4) c_cycles--;
	remdays -= c_cycles * DAYS_PER_100Y;

	q_cycles = remdays / DAYS_PER_4Y;
	if (q_cycles == 25) q_cycles--;
	remdays -= q_cycles * DAYS_PER_4Y;

	remyears = remdays / 365;
	if (remyears == 4) remyears--;
	remdays -= remyears * 365;

	leap = !remyears && (q_cycles || !c_cycles);
	yday = remdays + 31 + 28 + leap;
	if (yday >= 365+leap) yday -= 365+leap;

	years = remyears + 4*q_cycles + 100*c_cycles + 400LL*qc_cycles;

	for (months=0; days_in_month[months] <= remdays; months++)
		remdays -= days_in_month[months];

	if (months >= 10) {
		months -= 12;
		years++;
	}

	tm->tm_year = years + 100;
	tm->tm_mon = months + 2;
	tm->tm_mday = remdays + 1;
	tm->tm_wday = wday;
	tm->tm_yday = yday;

	tm->tm_hour = remsecs / 3600;
	tm->tm_min = remsecs / 60 % 60;
	tm->tm_sec = remsecs % 60;

	return 0;
}

uint64_t sys_timegm(sys_tm_t *tm) {
  return __tm_to_secs(tm);
}

int sys_gmtime(const uint64_t *t, sys_tm_t *tm) {
  __secs_to_tm(*t, tm);
  return 0;
}

int sys_timeofday(sys_timeval_t *tv) {
  sys_timespec_t ts;

  if (tv && sys_get_clock_ts(&ts) == 0) {
    tv->tv_sec = ts.tv_sec;
    tv->tv_usec = (int)ts.tv_nsec / 1000;
  }

  return 0;
}
