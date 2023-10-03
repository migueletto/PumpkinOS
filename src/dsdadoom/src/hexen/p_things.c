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

#include "doomdef.h"
#include "doomstat.h"
#include "p_mobj.h"
#include "s_sound.h"
#include "sounds.h"
#include "p_map.h"
#include "p_inter.h"
#include "p_tick.h"

#include "hexen/a_action.h"

#include "p_things.h"

static dboolean ActivateThing(mobj_t * mobj);
static dboolean DeactivateThing(mobj_t * mobj);

mobjtype_t TranslateThingType[] = {
    HEXEN_MT_MAPSPOT,                 // T_NONE
    HEXEN_MT_CENTAUR,                 // T_CENTAUR
    HEXEN_MT_CENTAURLEADER,           // T_CENTAURLEADER
    HEXEN_MT_DEMON,                   // T_DEMON
    HEXEN_MT_ETTIN,                   // T_ETTIN
    HEXEN_MT_FIREDEMON,               // T_FIREGARGOYLE
    HEXEN_MT_SERPENT,                 // T_WATERLURKER
    HEXEN_MT_SERPENTLEADER,           // T_WATERLURKERLEADER
    HEXEN_MT_WRAITH,                  // T_WRAITH
    HEXEN_MT_WRAITHB,                 // T_WRAITHBURIED
    HEXEN_MT_FIREBALL1,               // T_FIREBALL1
    HEXEN_MT_MANA1,                   // T_MANA1
    HEXEN_MT_MANA2,                   // T_MANA2
    HEXEN_MT_SPEEDBOOTS,              // T_ITEMBOOTS
    HEXEN_MT_ARTIEGG,                 // T_ITEMEGG
    HEXEN_MT_ARTIFLY,                 // T_ITEMFLIGHT
    HEXEN_MT_SUMMONMAULATOR,          // T_ITEMSUMMON
    HEXEN_MT_TELEPORTOTHER,           // T_ITEMTPORTOTHER
    HEXEN_MT_ARTITELEPORT,            // T_ITEMTELEPORT
    HEXEN_MT_BISHOP,                  // T_BISHOP
    HEXEN_MT_ICEGUY,                  // T_ICEGOLEM
    HEXEN_MT_BRIDGE,                  // T_BRIDGE
    HEXEN_MT_BOOSTARMOR,              // T_DRAGONSKINBRACERS
    HEXEN_MT_HEALINGBOTTLE,           // T_ITEMHEALTHPOTION
    HEXEN_MT_HEALTHFLASK,             // T_ITEMHEALTHFLASK
    HEXEN_MT_ARTISUPERHEAL,           // T_ITEMHEALTHFULL
    HEXEN_MT_BOOSTMANA,               // T_ITEMBOOSTMANA
    HEXEN_MT_FW_AXE,                  // T_FIGHTERAXE
    HEXEN_MT_FW_HAMMER,               // T_FIGHTERHAMMER
    HEXEN_MT_FW_SWORD1,               // T_FIGHTERSWORD1
    HEXEN_MT_FW_SWORD2,               // T_FIGHTERSWORD2
    HEXEN_MT_FW_SWORD3,               // T_FIGHTERSWORD3
    HEXEN_MT_CW_SERPSTAFF,            // T_CLERICSTAFF
    HEXEN_MT_CW_HOLY1,                // T_CLERICHOLY1
    HEXEN_MT_CW_HOLY2,                // T_CLERICHOLY2
    HEXEN_MT_CW_HOLY3,                // T_CLERICHOLY3
    HEXEN_MT_MW_CONE,                 // T_MAGESHARDS
    HEXEN_MT_MW_STAFF1,               // T_MAGESTAFF1
    HEXEN_MT_MW_STAFF2,               // T_MAGESTAFF2
    HEXEN_MT_MW_STAFF3,               // T_MAGESTAFF3
    HEXEN_MT_EGGFX,                   // T_MORPHBLAST
    HEXEN_MT_ROCK1,                   // T_ROCK1
    HEXEN_MT_ROCK2,                   // T_ROCK2
    HEXEN_MT_ROCK3,                   // T_ROCK3
    HEXEN_MT_DIRT1,                   // T_DIRT1
    HEXEN_MT_DIRT2,                   // T_DIRT2
    HEXEN_MT_DIRT3,                   // T_DIRT3
    HEXEN_MT_DIRT4,                   // T_DIRT4
    HEXEN_MT_DIRT5,                   // T_DIRT5
    HEXEN_MT_DIRT6,                   // T_DIRT6
    HEXEN_MT_ARROW,                   // T_ARROW
    HEXEN_MT_DART,                    // T_DART
    HEXEN_MT_POISONDART,              // T_POISONDART
    HEXEN_MT_RIPPERBALL,              // T_RIPPERBALL
    HEXEN_MT_SGSHARD1,                // T_STAINEDGLASS1
    HEXEN_MT_SGSHARD2,                // T_STAINEDGLASS2
    HEXEN_MT_SGSHARD3,                // T_STAINEDGLASS3
    HEXEN_MT_SGSHARD4,                // T_STAINEDGLASS4
    HEXEN_MT_SGSHARD5,                // T_STAINEDGLASS5
    HEXEN_MT_SGSHARD6,                // T_STAINEDGLASS6
    HEXEN_MT_SGSHARD7,                // T_STAINEDGLASS7
    HEXEN_MT_SGSHARD8,                // T_STAINEDGLASS8
    HEXEN_MT_SGSHARD9,                // T_STAINEDGLASS9
    HEXEN_MT_SGSHARD0,                // T_STAINEDGLASS0
    HEXEN_MT_PROJECTILE_BLADE,        // T_BLADE
    HEXEN_MT_ICESHARD,                // T_ICESHARD
    HEXEN_MT_FLAME_SMALL,             // T_FLAME_SMALL
    HEXEN_MT_FLAME_LARGE,             // T_FLAME_LARGE
    HEXEN_MT_ARMOR_1,                 // T_MESHARMOR
    HEXEN_MT_ARMOR_2,                 // T_FALCONSHIELD
    HEXEN_MT_ARMOR_3,                 // T_PLATINUMHELM
    HEXEN_MT_ARMOR_4,                 // T_AMULETOFWARDING
    HEXEN_MT_ARTIPOISONBAG,           // T_ITEMFLECHETTE
    HEXEN_MT_ARTITORCH,               // T_ITEMTORCH
    HEXEN_MT_BLASTRADIUS,             // T_ITEMREPULSION
    HEXEN_MT_MANA3,                   // T_MANA3
    HEXEN_MT_ARTIPUZZSKULL,           // T_PUZZSKULL
    HEXEN_MT_ARTIPUZZGEMBIG,          // T_PUZZGEMBIG
    HEXEN_MT_ARTIPUZZGEMRED,          // T_PUZZGEMRED
    HEXEN_MT_ARTIPUZZGEMGREEN1,       // T_PUZZGEMGREEN1
    HEXEN_MT_ARTIPUZZGEMGREEN2,       // T_PUZZGEMGREEN2
    HEXEN_MT_ARTIPUZZGEMBLUE1,        // T_PUZZGEMBLUE1
    HEXEN_MT_ARTIPUZZGEMBLUE2,        // T_PUZZGEMBLUE2
    HEXEN_MT_ARTIPUZZBOOK1,           // T_PUZZBOOK1
    HEXEN_MT_ARTIPUZZBOOK2,           // T_PUZZBOOK2
    HEXEN_MT_KEY1,                    // T_METALKEY
    HEXEN_MT_KEY2,                    // T_SMALLMETALKEY
    HEXEN_MT_KEY3,                    // T_AXEKEY
    HEXEN_MT_KEY4,                    // T_FIREKEY
    HEXEN_MT_KEY5,                    // T_GREENKEY
    HEXEN_MT_KEY6,                    // T_MACEKEY
    HEXEN_MT_KEY7,                    // T_SILVERKEY
    HEXEN_MT_KEY8,                    // T_RUSTYKEY
    HEXEN_MT_KEY9,                    // T_HORNKEY
    HEXEN_MT_KEYA,                    // T_SERPENTKEY
    HEXEN_MT_WATER_DRIP,              // T_WATERDRIP
    HEXEN_MT_FLAME_SMALL_TEMP,        // T_TEMPSMALLFLAME
    HEXEN_MT_FLAME_SMALL,             // T_PERMSMALLFLAME
    HEXEN_MT_FLAME_LARGE_TEMP,        // T_TEMPLARGEFLAME
    HEXEN_MT_FLAME_LARGE,             // T_PERMLARGEFLAME
    HEXEN_MT_DEMON_MASH,              // T_DEMON_MASH
    HEXEN_MT_DEMON2_MASH,             // T_DEMON2_MASH
    HEXEN_MT_ETTIN_MASH,              // T_ETTIN_MASH
    HEXEN_MT_CENTAUR_MASH,            // T_CENTAUR_MASH
    HEXEN_MT_THRUSTFLOOR_UP,          // T_THRUSTSPIKEUP
    HEXEN_MT_THRUSTFLOOR_DOWN,        // T_THRUSTSPIKEDOWN
    HEXEN_MT_WRAITHFX4,               // T_FLESH_DRIP1
    HEXEN_MT_WRAITHFX5,               // T_FLESH_DRIP2
    HEXEN_MT_WRAITHFX2                // T_SPARK_DRIP
};

dboolean EV_ThingProjectile(byte * args, dboolean gravity)
{
    int tid;
    angle_t angle;
    int fineAngle;
    fixed_t speed;
    fixed_t vspeed;
    mobjtype_t moType;
    mobj_t *mobj;
    mobj_t *newMobj;
    int searcher;
    dboolean success;

    success = false;
    searcher = -1;
    tid = args[0];
    moType = TranslateThingType[args[1]];
    if (nomonsters && (mobjinfo[moType].flags & MF_COUNTKILL))
    {                           // Don't spawn monsters if -nomonsters
        return false;
    }
    angle = (int) args[2] << 24;
    fineAngle = angle >> ANGLETOFINESHIFT;
    speed = (int) args[3] << 13;
    vspeed = (int) args[4] << 13;
    while ((mobj = P_FindMobjFromTID(tid, &searcher)) != NULL)
    {
        newMobj = P_SpawnMobj(mobj->x, mobj->y, mobj->z, moType);
        if (newMobj->info->seesound)
        {
            S_StartMobjSound(newMobj, newMobj->info->seesound);
        }
        P_SetTarget(&newMobj->target, mobj); // Originator
        newMobj->angle = angle;
        newMobj->momx = FixedMul(speed, finecosine[fineAngle]);
        newMobj->momy = FixedMul(speed, finesine[fineAngle]);
        newMobj->momz = vspeed;
        newMobj->flags |= MF_DROPPED; // Don't respawn
        if (gravity == true)
        {
            newMobj->flags &= ~MF_NOGRAVITY;
            newMobj->flags2 |= MF2_LOGRAV;
        }
        if (P_CheckMissileSpawn(newMobj) == true)
        {
            success = true;
        }
    }
    return success;
}

dboolean EV_ThingSpawn(byte * args, dboolean fog)
{
    int tid;
    angle_t angle;
    mobj_t *mobj;
    mobj_t *newMobj;
    mobj_t *fogMobj;
    mobjtype_t moType;
    int searcher;
    dboolean success;
    fixed_t z;

    success = false;
    searcher = -1;
    tid = args[0];
    moType = TranslateThingType[args[1]];
    if (nomonsters && (mobjinfo[moType].flags & MF_COUNTKILL))
    {                           // Don't spawn monsters if -nomonsters
        return false;
    }
    angle = (int) args[2] << 24;
    while ((mobj = P_FindMobjFromTID(tid, &searcher)) != NULL)
    {
        if (mobjinfo[moType].flags2 & MF2_FLOATBOB)
        {
            z = mobj->z - mobj->floorz;
        }
        else
        {
            z = mobj->z;
        }
        newMobj = P_SpawnMobj(mobj->x, mobj->y, z, moType);
        if (P_TestMobjLocation(newMobj) == false)
        {                       // Didn't fit
            P_RemoveMobj(newMobj);
        }
        else
        {
            newMobj->angle = angle;
            if (fog == true)
            {
                fogMobj = P_SpawnMobj(mobj->x, mobj->y,
                                      mobj->z + TELEFOGHEIGHT, HEXEN_MT_TFOG);
                S_StartMobjSound(fogMobj, hexen_sfx_teleport);
            }
            newMobj->flags |= MF_DROPPED;     // Don't respawn
            if (newMobj->flags2 & MF2_FLOATBOB)
            {
                newMobj->special1.i = newMobj->z - newMobj->floorz;
            }
            success = true;
        }
    }
    return success;
}

dboolean EV_ThingActivate(int tid)
{
    mobj_t *mobj;
    int searcher;
    dboolean success;

    success = false;
    searcher = -1;
    while ((mobj = P_FindMobjFromTID(tid, &searcher)) != NULL)
    {
        if (ActivateThing(mobj) == true)
        {
            success = true;
        }
    }
    return success;
}

dboolean EV_ThingDeactivate(int tid)
{
    mobj_t *mobj;
    int searcher;
    dboolean success;

    success = false;
    searcher = -1;
    while ((mobj = P_FindMobjFromTID(tid, &searcher)) != NULL)
    {
        if (DeactivateThing(mobj) == true)
        {
            success = true;
        }
    }
    return success;
}

dboolean EV_ThingRemove(int tid)
{
    mobj_t *mobj;
    int searcher;
    dboolean success;

    success = false;
    searcher = -1;
    while ((mobj = P_FindMobjFromTID(tid, &searcher)) != NULL)
    {
        if (mobj->type == HEXEN_MT_BRIDGE)
        {
            A_BridgeRemove(mobj);
            return true;
        }
        P_RemoveMobj(mobj);
        success = true;
    }
    return success;
}

dboolean EV_ThingDestroy(int tid)
{
    mobj_t *mobj;
    int searcher;
    dboolean success;

    success = false;
    searcher = -1;
    while ((mobj = P_FindMobjFromTID(tid, &searcher)) != NULL)
    {
        if (mobj->flags & MF_SHOOTABLE)
        {
            P_DamageMobj(mobj, NULL, NULL, 10000);
            success = true;
        }
    }
    return success;
}

static dboolean ActivateThing(mobj_t * mobj)
{
    if (mobj->flags & MF_COUNTKILL)
    {                           // Monster
        if (mobj->flags2 & MF2_DORMANT)
        {
            mobj->flags2 &= ~MF2_DORMANT;
            mobj->tics = 1;
            return true;
        }
        return false;
    }
    switch (mobj->type)
    {
        case HEXEN_MT_ZTWINEDTORCH:
        case HEXEN_MT_ZTWINEDTORCH_UNLIT:
            P_SetMobjState(mobj, HEXEN_S_ZTWINEDTORCH_1);
            S_StartMobjSound(mobj, hexen_sfx_ignite);
            break;
        case HEXEN_MT_ZWALLTORCH:
        case HEXEN_MT_ZWALLTORCH_UNLIT:
            P_SetMobjState(mobj, HEXEN_S_ZWALLTORCH1);
            S_StartMobjSound(mobj, hexen_sfx_ignite);
            break;
        case HEXEN_MT_ZGEMPEDESTAL:
            P_SetMobjState(mobj, HEXEN_S_ZGEMPEDESTAL2);
            break;
        case HEXEN_MT_ZWINGEDSTATUENOSKULL:
            P_SetMobjState(mobj, HEXEN_S_ZWINGEDSTATUENOSKULL2);
            break;
        case HEXEN_MT_THRUSTFLOOR_UP:
        case HEXEN_MT_THRUSTFLOOR_DOWN:
            if (mobj->special_args[0] == 0)
            {
                S_StartMobjSound(mobj, hexen_sfx_thrustspike_lower);
                mobj->flags2 &= ~MF2_DONTDRAW;
                if (mobj->special_args[1])
                    P_SetMobjState(mobj, HEXEN_S_BTHRUSTRAISE1);
                else
                    P_SetMobjState(mobj, HEXEN_S_THRUSTRAISE1);
            }
            break;
        case HEXEN_MT_ZFIREBULL:
        case HEXEN_MT_ZFIREBULL_UNLIT:
            P_SetMobjState(mobj, HEXEN_S_ZFIREBULL_BIRTH);
            S_StartMobjSound(mobj, hexen_sfx_ignite);
            break;
        case HEXEN_MT_ZBELL:
            if (mobj->health > 0)
            {
                P_DamageMobj(mobj, NULL, NULL, 10);     // 'ring' the bell
            }
            break;
        case HEXEN_MT_ZCAULDRON:
        case HEXEN_MT_ZCAULDRON_UNLIT:
            P_SetMobjState(mobj, HEXEN_S_ZCAULDRON1);
            S_StartMobjSound(mobj, hexen_sfx_ignite);
            break;
        case HEXEN_MT_FLAME_SMALL:
            S_StartMobjSound(mobj, hexen_sfx_ignite);
            P_SetMobjState(mobj, HEXEN_S_FLAME_SMALL1);
            break;
        case HEXEN_MT_FLAME_LARGE:
            S_StartMobjSound(mobj, hexen_sfx_ignite);
            P_SetMobjState(mobj, HEXEN_S_FLAME_LARGE1);
            break;
        case HEXEN_MT_BAT_SPAWNER:
            P_SetMobjState(mobj, HEXEN_S_SPAWNBATS1);
            break;
        default:
            return false;
            break;
    }
    return true;
}

static dboolean DeactivateThing(mobj_t * mobj)
{
    if (mobj->flags & MF_COUNTKILL)
    {                           // Monster
        if (!(mobj->flags2 & MF2_DORMANT))
        {
            mobj->flags2 |= MF2_DORMANT;
            mobj->tics = -1;
            return true;
        }
        return false;
    }
    switch (mobj->type)
    {
        case HEXEN_MT_ZTWINEDTORCH:
        case HEXEN_MT_ZTWINEDTORCH_UNLIT:
            P_SetMobjState(mobj, HEXEN_S_ZTWINEDTORCH_UNLIT);
            break;
        case HEXEN_MT_ZWALLTORCH:
        case HEXEN_MT_ZWALLTORCH_UNLIT:
            P_SetMobjState(mobj, HEXEN_S_ZWALLTORCH_U);
            break;
        case HEXEN_MT_THRUSTFLOOR_UP:
        case HEXEN_MT_THRUSTFLOOR_DOWN:
            if (mobj->special_args[0] == 1)
            {
                S_StartMobjSound(mobj, hexen_sfx_thrustspike_raise);
                if (mobj->special_args[1])
                    P_SetMobjState(mobj, HEXEN_S_BTHRUSTLOWER);
                else
                    P_SetMobjState(mobj, HEXEN_S_THRUSTLOWER);
            }
            break;
        case HEXEN_MT_ZFIREBULL:
        case HEXEN_MT_ZFIREBULL_UNLIT:
            P_SetMobjState(mobj, HEXEN_S_ZFIREBULL_DEATH);
            break;
        case HEXEN_MT_ZCAULDRON:
        case HEXEN_MT_ZCAULDRON_UNLIT:
            P_SetMobjState(mobj, HEXEN_S_ZCAULDRON_U);
            break;
        case HEXEN_MT_FLAME_SMALL:
            P_SetMobjState(mobj, HEXEN_S_FLAME_SDORM1);
            break;
        case HEXEN_MT_FLAME_LARGE:
            P_SetMobjState(mobj, HEXEN_S_FLAME_LDORM1);
            break;
        case HEXEN_MT_BAT_SPAWNER:
            P_SetMobjState(mobj, HEXEN_S_SPAWNBATS_OFF);
            break;
        default:
            return false;
            break;
    }
    return true;
}
