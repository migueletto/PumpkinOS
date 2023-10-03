//
// Copyright(C) 2020 by Ryan Krafnick
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//	DSDA Time
//

#include <time.h>
#include <string.h>

#include "i_system.h"
#include "lprintf.h"

#include "dsda/configuration.h"

#include "time.h"

// clock_gettime implementation for msvc
// NOTE: Only supports CLOCK_MONOTONIC
#ifdef _MSC_VER

#include <windows.h>

#define CLOCK_MONOTONIC -1

static int clock_gettime(int clockid, struct timespec *tp) {
  static unsigned long long timer_frequency = 0;
  unsigned long long time;

  // Get number of timer counts per second
  if (!timer_frequency)
    QueryPerformanceFrequency((LARGE_INTEGER*) &timer_frequency);

  // Get timer counts
  QueryPerformanceCounter((LARGE_INTEGER*) &time);

  // Convert timer counts to timespec (that is, nanoseconds and seconds)
  tp->tv_nsec = time % timer_frequency * 1000000000 / timer_frequency;
  tp->tv_sec = time / timer_frequency;

  return 0;
}

#endif //_MSC_VER

static struct timespec dsda_time[DSDA_TIMER_COUNT];

void dsda_StartTimer(int timer) {
  clock_gettime(CLOCK_MONOTONIC, &dsda_time[timer]);
}

unsigned long long dsda_ElapsedTime(int timer) {
  struct timespec now;

  clock_gettime(CLOCK_MONOTONIC, &now);

  return (unsigned long long) (
           (signed long long) (now.tv_nsec - dsda_time[timer].tv_nsec) / 1000 +
           (signed long long) (now.tv_sec - dsda_time[timer].tv_sec) * 1000000
         );
}

unsigned long long dsda_ElapsedTimeMS(int timer) {
  return dsda_ElapsedTime(timer) / 1000;
}

void dsda_PrintElapsedTime(int timer, const char* message) {
  unsigned long long result;

  result = dsda_ElapsedTime(timer);
  lprintf(LO_INFO, "%s: %lf\n", message, (double) result / 1000);
}

static void dsda_Throttle(int timer, unsigned long long target_time) {
  unsigned long long elapsed_time;
  unsigned long long remaining_time;

  while (1) {
    elapsed_time = dsda_ElapsedTime(timer);

    if (elapsed_time >= target_time) {
      dsda_StartTimer(timer);
      return;
    }

    // Sleeping doesn't have high accuracy
    remaining_time = target_time - elapsed_time;
    if (remaining_time > 1000)
      I_uSleep(remaining_time - 1000);
  }
}

void dsda_LimitFPS(void) {
  extern int movement_smooth;

  int fps_limit;

  fps_limit = dsda_IntConfig(dsda_config_fps_limit);

  if (movement_smooth && fps_limit) {
    unsigned long long target_time;

    target_time = 1000000 / fps_limit;

    dsda_Throttle(dsda_timer_fps, target_time);
  }
}

#define TICRATE 35

int dsda_GameSpeed(void);

static unsigned long long dsda_RealTime(void) {
  static dboolean started = false;

  if (!started)
  {
    started = true;
    dsda_StartTimer(dsda_timer_realtime);
  }

  return dsda_ElapsedTime(dsda_timer_realtime);
}

static unsigned long long dsda_ScaledTime(void) {
  return dsda_RealTime() * dsda_GameSpeed() / 100;
}

extern int ms_to_next_tick;

// During a fast demo, each call yields a new tick
static int dsda_GetTickFastDemo(void)
{
  static int tick;
  return tick++;
}

int dsda_GetTickRealTime(void) {
  int i;
  unsigned long long t;

  t = dsda_RealTime();

  i = t * TICRATE / 1000000;
  ms_to_next_tick = (i + 1) * 1000 / TICRATE - t / 1000;
  if (ms_to_next_tick > 1000 / TICRATE) ms_to_next_tick = 1;
  if (ms_to_next_tick < 1) ms_to_next_tick = 0;
  return i;
}

static int dsda_TickMS(int n) {
  return n * 1000 * 100 / dsda_GameSpeed() / TICRATE;
}

static int dsda_GetTickScaledTime(void) {
  int i;
  unsigned long long t;

  t = dsda_RealTime();

  i = t * TICRATE * dsda_GameSpeed() / 100 / 1000000;
  ms_to_next_tick = dsda_TickMS(i + 1) - t / 1000;
  if (ms_to_next_tick > dsda_TickMS(1)) ms_to_next_tick = 1;
  if (ms_to_next_tick < 1) ms_to_next_tick = 0;
  return i;
}

// During a fast demo, no time elapses in between ticks
static unsigned long long dsda_TickElapsedTimeFastDemo(void) {
  return 0;
}

static unsigned long long dsda_TickElapsedRealTime(void) {
  int tick = dsda_GetTick();

  return dsda_RealTime() - (unsigned long long) tick * 1000000 / TICRATE;
}

static unsigned long long dsda_TickElapsedScaledTime(void) {
  int tick = dsda_GetTick();

  return dsda_ScaledTime() - (unsigned long long) tick * 1000000 / TICRATE;
}

int (*dsda_GetTick)(void) = dsda_GetTickRealTime;
unsigned long long (*dsda_TickElapsedTime)(void) = dsda_TickElapsedRealTime;

void dsda_ResetTimeFunctions(int fastdemo) {
  if (fastdemo) {
    dsda_GetTick = dsda_GetTickFastDemo;
    dsda_TickElapsedTime = dsda_TickElapsedTimeFastDemo;
  }
  else if (dsda_GameSpeed() != 100) {
    dsda_GetTick = dsda_GetTickScaledTime;
    dsda_TickElapsedTime = dsda_TickElapsedScaledTime;
  }
  else {
    dsda_GetTick = dsda_GetTickRealTime;
    dsda_TickElapsedTime = dsda_TickElapsedRealTime;
  }
}
