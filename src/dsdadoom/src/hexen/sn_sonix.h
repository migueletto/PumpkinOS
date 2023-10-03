//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
// Copyright(C) 2005-2014 Simon Howard
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

#ifndef __HEXEN_SN_SONIX__
#define __HEXEN_SN_SONIX__

#include "p_mobj.h"

typedef struct seqnode_s seqnode_t;
struct seqnode_s
{
  int *sequencePtr;
  int sequence;
  mobj_t *mobj;
  int currentSoundID;
  int delayTics;
  int volume;
  int stopSound;
  seqnode_t *prev;
  seqnode_t *next;
};

extern int ActiveSequences;
extern seqnode_t *SequenceListHead;

void SN_InitSequenceScript(void);
void SN_StartSequence(mobj_t * mobj, int sequence);
void SN_StartSequenceName(mobj_t * mobj, const char *name);
void SN_StopSequence(mobj_t * mobj);
void SN_UpdateActiveSequences(void);
void SN_StopAllSequences(void);
int SN_GetSequenceOffset(int sequence, int *sequencePtr);
void SN_ChangeNodeData(int nodeNum, int seqOffset, int delayTics, int volume, int currentSoundID);

#endif
