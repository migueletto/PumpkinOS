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
//	DSDA Key Frame
//

#ifndef __DSDA_KEY_FRAME__
#define __DSDA_KEY_FRAME__

#include "doomtype.h"

struct auto_kf_s;

typedef struct {
  byte* buffer;
  struct auto_kf_s* auto_kf;
} parent_kf_t;

typedef struct {
  byte* buffer;
  int buffer_length;
  int game_tic_count;
  parent_kf_t parent;
} dsda_key_frame_t;

typedef struct auto_kf_s {
  int auto_index;
  dsda_key_frame_t kf;
  struct auto_kf_s* prev;
  struct auto_kf_s* next;
} auto_kf_t;

void dsda_StoreKeyFrame(dsda_key_frame_t* key_frame, byte complete, byte export);
void dsda_RestoreKeyFrame(dsda_key_frame_t* key_frame, dboolean skip_wipe);
void dsda_InitKeyFrame(void);
void dsda_ContinueKeyFrame(void);
int dsda_KeyFrameRestored(void);
void dsda_StoreTempKeyFrame(void);
void dsda_StoreQuickKeyFrame(void);
void dsda_RestoreQuickKeyFrame(void);
dboolean dsda_RestoreClosestKeyFrame(int tic);
void dsda_RewindAutoKeyFrame(void);
void dsda_ResetAutoKeyFrameTimeout(void);
void dsda_UpdateAutoKeyFrames(void);
void dsda_ForgetAutoKeyFrames(void);

#endif
