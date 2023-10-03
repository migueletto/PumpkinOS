//
// Copyright(C) 2021 by Ryan Krafnick
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
//	DSDA Split Tracker
//

#ifndef __DSDA_SPLIT_TRACKER__
#define __DSDA_SPLIT_TRACKER__

typedef struct {
  int current;
  int best;
  int best_delta;
  int session_best;
  int session_best_delta;
  int ref;
  int ref_delta;
} dsda_split_time_t;

typedef struct {
  dsda_split_time_t leveltime;
  dsda_split_time_t totalleveltimes;
  int episode;
  int map;
  int first_time;
  int run_counter;
  int exits;
} dsda_split_t;

void dsda_RecordSplit(void);
dsda_split_t* dsda_CurrentSplit(void);
void dsda_WriteSplits(void);
void dsda_ResetSplits(void);
int dsda_DemoAttempts(void);

#endif
