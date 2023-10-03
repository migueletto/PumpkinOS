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

#include "am_map.h"
#include "doomstat.h"
#include "d_event.h"
#include "s_sound.h"
#include "sounds.h"
#include "i_system.h"
#include "i_video.h"
#include "v_video.h"
#include "lprintf.h"
#include "w_wad.h"
#include "g_game.h"
#include "p_setup.h"

#include "dsda/exhud.h"
#include "dsda/mapinfo.h"

#include "heretic/mn_menu.h"
#include "heretic/sb_bar.h"

#include "hexen/sn_sonix.h"

#include "in_lude.h"

#define	TEXTSPEED 3
#define	TEXTWAIT 140

typedef enum
{
    SINGLE,
    COOPERATIVE,
    DEATHMATCH
} gametype_t;

static void WaitStop(void);
static void Stop(void);
static void LoadPics(void);
static void CheckForSkip(void);
static void InitStats(void);
static void DrDeathTally(void);
static void DrNumber(int val, int x, int y, int wrapThresh);
static void DrNumberBold(int val, int x, int y, int wrapThresh);
static void DrawHubText(void);

static dboolean intermission;
char ClusterMessage[MAX_INTRMSN_MESSAGE_SIZE];

static dboolean skipintermission;
static int interstate = 0;
static int intertime = -1;
static gametype_t gametype;
static int cnt;
static int slaughterboy;        // in DM, the player with the most kills
static int FontABaseLump;

static signed int totalFrags[MAX_MAXPLAYERS];

static int HubCount;
static char *HubText;

extern dboolean BorderNeedRefresh;

void Hexen_IN_Start(wbstartstruct_t* wbstartstruct)
{
    V_SetPalette(0);
    InitStats();
    LoadPics();
    intermission = true;
    interstate = 0;
    skipintermission = false;
    intertime = 0;
    AM_Stop(false);
    SN_StopAllSequences();
}

static void WaitStop(void)
{
    if (!--cnt)
    {
        Stop();
        gameaction = ga_leavemap;
    }
}

static void Stop(void)
{
    intermission = false;
    SB_Start();
    BorderNeedRefresh = true;
}

static const char *ClusMsgLumpNames[] = {
    "clus1msg",
    "clus2msg",
    "clus3msg",
    "clus4msg",
    "clus5msg"
};

static void InitStats(void)
{
    int i;
    int j;
    int oldCluster;
    signed int slaughterfrags;
    int slaughtercount;
    int playercount;
    const char *msgLumpName;
    int msgSize;
    int msgLump;

    if (!deathmatch)
    {
        gametype = SINGLE;
        HubCount = 0;
        oldCluster = dsda_MapCluster(gamemap);
        if (oldCluster != dsda_MapCluster(leave_data.map))
        {
            if (oldCluster >= 1 && oldCluster <= 5)
            {
                msgLumpName = ClusMsgLumpNames[oldCluster - 1];
                msgLump = W_GetNumForName(msgLumpName);
                msgSize = W_LumpLength(msgLump);
                if (msgSize >= MAX_INTRMSN_MESSAGE_SIZE)
                {
                    I_Error("Cluster message too long (%s)", msgLumpName);
                }
                memcpy(ClusterMessage, W_LumpByNum(msgLump), msgSize);
                ClusterMessage[msgSize] = '\0';    // Append terminator
                HubText = ClusterMessage;
                HubCount = strlen(HubText) * TEXTSPEED + TEXTWAIT;
                S_StartSongName("hub", true);
            }
        }
    }
    else
    {
        gametype = DEATHMATCH;
        slaughterboy = 0;
        slaughterfrags = -9999;
        playercount = 0;
        slaughtercount = 0;
        for (i = 0; i < g_maxplayers; i++)
        {
            totalFrags[i] = 0;
            if (playeringame[i])
            {
                playercount++;
                for (j = 0; j < g_maxplayers; j++)
                {
                    if (playeringame[j])
                    {
                        totalFrags[i] += players[i].frags[j];
                    }
                }
            }
            if (totalFrags[i] > slaughterfrags)
            {
                slaughterboy = 1 << i;
                slaughterfrags = totalFrags[i];
                slaughtercount = 1;
            }
            else if (totalFrags[i] == slaughterfrags)
            {
                slaughterboy |= 1 << i;
                slaughtercount++;
            }
        }
        if (playercount == slaughtercount)
        {                       // don't do the slaughter stuff if everyone is equal
            slaughterboy = 0;
        }
        S_StartSongName("hub", true);
    }
}

static void LoadPics(void)
{
    if (HubCount || gametype == DEATHMATCH)
    {
        FontABaseLump = W_GetNumForName("FONTA_S") + 1;
    }
}

void Hexen_IN_Ticker(void)
{
    if (!intermission)
    {
        return;
    }
    if (interstate)
    {
        WaitStop();
        return;
    }
    skipintermission = false;
    CheckForSkip();
    intertime++;
    if (skipintermission || (gametype == SINGLE && !HubCount))
    {
        interstate = 1;
        cnt = 10;
        skipintermission = false;
    }
}

static void CheckForSkip(void)
{
    int i;
    player_t *player;
    static dboolean triedToSkip;

    for (i = 0, player = players; i < g_maxplayers; i++, player++)
    {
        if (playeringame[i])
        {
            if (player->cmd.buttons & BT_ATTACK)
            {
                if (!player->attackdown)
                {
                    skipintermission = 1;
                }
                player->attackdown = true;
            }
            else
            {
                player->attackdown = false;
            }
            if (player->cmd.buttons & BT_USE)
            {
                if (!player->usedown)
                {
                    skipintermission = 1;
                }
                player->usedown = true;
            }
            else
            {
                player->usedown = false;
            }
        }
    }
    if (deathmatch && intertime < 140)
    {                           // wait for 4 seconds before allowing a skip
        if (skipintermission == 1)
        {
            triedToSkip = true;
            skipintermission = 0;
        }
    }
    else
    {
        if (triedToSkip)
        {
            skipintermission = 1;
            triedToSkip = false;
        }
    }
}

void Hexen_IN_Drawer(void)
{
    if (!intermission)
    {
        return;
    }
    if (interstate)
    {
        return;
    }

    V_DrawRawScreen("INTERPIC");

    if (gametype == SINGLE)
    {
        if (HubCount)
        {
            DrawHubText();
        }
        dsda_DrawExIntermission();
    }
    else
    {
        DrDeathTally();
    }
}

#define TALLY_EFFECT_TICKS 20
#define TALLY_FINAL_X_DELTA (23*FRACUNIT)
#define TALLY_FINAL_Y_DELTA (13*FRACUNIT)
#define TALLY_START_XPOS (178*FRACUNIT)
#define TALLY_STOP_XPOS (90*FRACUNIT)
#define TALLY_START_YPOS (132*FRACUNIT)
#define TALLY_STOP_YPOS (83*FRACUNIT)
#define TALLY_TOP_X 85
#define TALLY_TOP_Y 9
#define TALLY_LEFT_X 7
#define TALLY_LEFT_Y 71
#define TALLY_TOTALS_X 291

static void DrDeathTally(void)
{
    int i, j;
    fixed_t xPos, yPos;
    fixed_t xDelta, yDelta;
    fixed_t xStart, scale;
    int x, y;
    dboolean bold;
    static dboolean showTotals;
    int temp;

    V_DrawNamePatch(TALLY_TOP_X, TALLY_TOP_Y, 0, "tallytop", CR_DEFAULT, VPT_STRETCH);
    V_DrawNamePatch(TALLY_LEFT_X, TALLY_LEFT_Y, 0, "tallylft", CR_DEFAULT, VPT_STRETCH);
    if (intertime < TALLY_EFFECT_TICKS)
    {
        showTotals = false;
        scale = (intertime * FRACUNIT) / TALLY_EFFECT_TICKS;
        xDelta = FixedMul(scale, TALLY_FINAL_X_DELTA);
        yDelta = FixedMul(scale, TALLY_FINAL_Y_DELTA);
        xStart = TALLY_START_XPOS - FixedMul(scale,
                                             TALLY_START_XPOS -
                                             TALLY_STOP_XPOS);
        yPos =
            TALLY_START_YPOS - FixedMul(scale,
                                        TALLY_START_YPOS - TALLY_STOP_YPOS);
    }
    else
    {
        xDelta = TALLY_FINAL_X_DELTA;
        yDelta = TALLY_FINAL_Y_DELTA;
        xStart = TALLY_STOP_XPOS;
        yPos = TALLY_STOP_YPOS;
    }
    if (intertime >= TALLY_EFFECT_TICKS && showTotals == false)
    {
        showTotals = true;
        S_StartVoidSound(hexen_sfx_platform_stop);
    }
    y = yPos >> FRACBITS;
    for (i = 0; i < g_maxplayers; i++)
    {
        xPos = xStart;
        for (j = 0; j < g_maxplayers; j++, xPos += xDelta)
        {
            x = xPos >> FRACBITS;
            bold = (i == consoleplayer || j == consoleplayer);
            if (playeringame[i] && playeringame[j])
            {
                if (bold)
                {
                    DrNumberBold(players[i].frags[j], x, y, 100);
                }
                else
                {
                    DrNumber(players[i].frags[j], x, y, 100);
                }
            }
            else
            {
                temp = MN_TextAWidth("--") / 2;
                if (bold)
                {
                    MN_DrTextAYellow("--", x - temp, y);
                }
                else
                {
                    MN_DrTextA("--", x - temp, y);
                }
            }
        }
        if (showTotals && playeringame[i]
            && !((slaughterboy & (1 << i)) && !(intertime & 16)))
        {
            DrNumber(totalFrags[i], TALLY_TOTALS_X, y, 1000);
        }
        yPos += yDelta;
        y = yPos >> FRACBITS;
    }
}

static void DrNumber(int val, int x, int y, int wrapThresh)
{
    char buff[8] = "XX";

    if (!(val < -9 && wrapThresh < 1000))
    {
        snprintf(buff, sizeof(buff), "%d", val >= wrapThresh ? val % wrapThresh : val);
    }
    MN_DrTextA(buff, x - MN_TextAWidth(buff) / 2, y);
}

static void DrNumberBold(int val, int x, int y, int wrapThresh)
{
    char buff[8] = "XX";

    if (!(val < -9 && wrapThresh < 1000))
    {
        snprintf(buff, sizeof(buff), "%d", val >= wrapThresh ? val % wrapThresh : val);
    }
    MN_DrTextAYellow(buff, x - MN_TextAWidth(buff) / 2, y);
}

static void DrawHubText(void)
{
    int count;
    char *ch;
    int c;
    int cx, cy;
    int lump;
    int width;

    cy = 5;
    cx = 10;
    ch = HubText;
    count = (intertime - 10) / TEXTSPEED;
    if (count < 0)
    {
        count = 0;
    }
    for (; count; count--)
    {
        c = *ch++;
        if (!c)
        {
            break;
        }
        if (c == '\n')
        {
            cx = 10;
            cy += 9;
            continue;
        }
        if (c < 32)
        {
            continue;
        }
        c = toupper(c);
        if (c == 32)
        {
            cx += 5;
            continue;
        }
        lump = FontABaseLump + c - 33;
        width = R_NumPatchWidth(lump);
        if (cx + width > SCREENWIDTH)
        {
            break;
        }
        V_DrawNumPatch(cx, cy, 0, lump, CR_DEFAULT, VPT_STRETCH);
        cx += width;
    }
}
