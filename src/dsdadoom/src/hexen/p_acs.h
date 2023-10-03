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

#ifndef __HEXEN_P_ACS__
#define __HEXEN_P_ACS__

#include "r_defs.h"
#include "p_mobj.h"

#define MAX_ACS_SCRIPT_VARS 10
#define MAX_ACS_MAP_VARS 32
#define MAX_ACS_WORLD_VARS 64
#define ACS_STACK_DEPTH 32
#define MAX_ACS_STORE 20

typedef enum
{
    ASTE_INACTIVE,
    ASTE_RUNNING,
    ASTE_SUSPENDED,
    ASTE_WAITINGFORTAG,
    ASTE_WAITINGFORPOLY,
    ASTE_WAITINGFORSCRIPT,
    ASTE_TERMINATING
} aste_t;

typedef struct acs_s acs_t;
typedef struct acsInfo_s acsInfo_t;

struct acsInfo_s
{
    int number;
    int offset;
    int argCount;
    aste_t state;
    int waitValue;
};

struct acs_s
{
    thinker_t thinker;
    mobj_t *activator;
    line_t *line;
    int side;
    int number;
    int infoIndex;
    int delayCount;
    int stack[ACS_STACK_DEPTH];
    int stackPtr;
    int vars[MAX_ACS_SCRIPT_VARS];
    int ip;
};

typedef struct
{
    int map;                    // Target map
    int script;                 // Script number on target map
    byte args[4];               // Padded to 4 for alignment
} acsstore_t;

void P_LoadACScripts(int lump);
dboolean P_StartACS(int number, int map, byte * args, mobj_t * activator, line_t * line, int side);
dboolean P_StartLockedACS(line_t * line, byte * args, mobj_t * mo, int side);
dboolean P_TerminateACS(int number, int map);
dboolean P_SuspendACS(int number, int map);
void T_InterpretACS(acs_t * script);
void P_TagFinished(int tag);
void P_PolyobjFinished(int po);
void P_ACSInitNewGame(void);
void P_CheckACSStore(void);
void CheckACSPresent(int number);

extern int ACScriptCount;
extern const byte *ActionCodeBase;
extern acsInfo_t *ACSInfo;
extern int MapVars[MAX_ACS_MAP_VARS];
extern int WorldVars[MAX_ACS_WORLD_VARS];
extern acsstore_t ACSStore[MAX_ACS_STORE + 1];  // +1 for termination marker

#endif
