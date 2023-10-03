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

#ifndef __DSDA_TIME__
#define __DSDA_TIME__

typedef enum {
  dsda_timer_realtime,
  dsda_timer_fps,
  dsda_timer_key_frame,
  dsda_timer_brute_force,
  dsda_timer_render_stats,
  dsda_timer_temp,
  DSDA_TIMER_COUNT
} dsda_timer_t;

extern int (*dsda_GetTick)(void);
extern unsigned long long (*dsda_TickElapsedTime)(void);

void dsda_StartTimer(int timer);
unsigned long long dsda_ElapsedTime(int timer);
unsigned long long dsda_ElapsedTimeMS(int timer);
void dsda_PrintElapsedTime(int timer, const char* message);
void dsda_LimitFPS(void);
int dsda_GetTickRealTime(void);
void dsda_ResetTimeFunctions(int fastdemo);

#endif
