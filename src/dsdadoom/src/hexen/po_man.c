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
#include "p_mobj.h"
#include "r_defs.h"
#include "r_main.h"
#include "lprintf.h"
#include "w_wad.h"
#include "p_setup.h"
#include "m_bbox.h"
#include "p_tick.h"
#include "p_inter.h"
#include "p_map.h"
#include "p_maputl.h"

#include "hexen/p_acs.h"
#include "hexen/sn_sonix.h"

#include "dsda/map_format.h"
#include "dsda/preferences.h"
#include "dsda/udmf.h"
#include "dsda/utility.h"

#include "po_man.h"

#define PO_MAXPOLYSEGS 64

static polyobj_t *GetPolyobj(int polyNum);
static int GetPolyobjMirror(int poly);
static void ThrustMobj(mobj_t * mobj, seg_t * seg, polyobj_t * po);
static void UpdateSegBBox(seg_t * seg, polyobj_t * po);
static void RotatePt(int an, fixed_t * x, fixed_t * y, fixed_t startSpotX, fixed_t startSpotY);
void UnLinkPolyobj(polyobj_t * po);
void LinkPolyobj(polyobj_t * po);
static dboolean CheckMobjBlocking(seg_t * seg, polyobj_t * po);
static void InitBlockMap(void);
static void IterFindPolySegs(int x, int y, seg_t ** segList);
static void SpawnPolyobj(int index, int tag, dboolean crush, dboolean hurt);
static void TranslateToStartSpot(int tag, int originX, int originY);

polyblock_t **PolyBlockMap;
polyobj_t *polyobjs;            // list of all poly-objects on the level
int po_NumPolyobjs;

static int PolySegCount;
static fixed_t PolyStartX;
static fixed_t PolyStartY;

static void ResetSegDrawingParameters(seg_t *seg)
{
  seg->v1->px = seg->v1->x;
  seg->v1->py = seg->v1->y;
  seg->v2->px = seg->v2->x;
  seg->v2->py = seg->v2->y;
  seg->pangle = seg->angle;
}

void ResetPolySubSector(polyobj_t *po)
{
  int i;
  vertex_t avg;
  seg_t **polySeg;
  subsector_t *new_sub;

  avg.x = 0;
  avg.y = 0;
  polySeg = po->segs;

  for (i = 0; i < po->numsegs; i++, polySeg++)
  {
      ResetSegDrawingParameters(*polySeg);

      avg.x += (*polySeg)->v1->x >> FRACBITS;
      avg.y += (*polySeg)->v1->y >> FRACBITS;
  }

  avg.x /= po->numsegs;
  avg.y /= po->numsegs;

  new_sub = R_PointInSubsector(avg.x << FRACBITS, avg.y << FRACBITS);

  if (new_sub->poly)
  {
    return; // Colliding poly objects?
  }

  po->subsector->poly = NULL;
  po->subsector = new_sub;
  po->subsector->poly = po;
}

static void StopPolyEvent(polyevent_t * pe)
{
  polyobj_t *poly;

  poly = GetPolyobj(pe->polyobj);
  if (poly->specialdata == pe)
  {
      poly->specialdata = NULL;
  }
  SN_StopSequence((mobj_t *) & poly->startSpot);
  P_PolyobjFinished(poly->tag);
  P_RemoveThinker(&pe->thinker);
}

dboolean EV_StopPoly(int polyNum)
{
  polyobj_t *poly;

  poly = GetPolyobj(polyNum);

  if (poly)
  {
    if (poly->specialdata)
    {
      StopPolyEvent(poly->specialdata);
    }

    return true;
  }

  return false;
}

void T_RotatePoly(polyevent_t * pe)
{
    int absSpeed;

    if (PO_RotatePolyobj(pe->polyobj, pe->speed))
    {
        absSpeed = abs(pe->speed);

        if (pe->dist == -1)
        {                       // perpetual polyobj
            return;
        }
        pe->dist -= absSpeed;
        if (pe->dist <= 0)
        {
            StopPolyEvent(pe);
        }
        if (pe->dist < absSpeed)
        {
            pe->speed = pe->dist * (pe->speed < 0 ? -1 : 1);
        }
    }
}

dboolean EV_RotateZDoomPoly(line_t * line, int polyNum, int speed,
                            int angle, int direction, dboolean overRide)
{
    int mirror;
    polyevent_t *pe;
    polyobj_t *poly;

    poly = GetPolyobj(polyNum);
    if (poly != NULL)
    {
        if (poly->specialdata && !overRide)
        {                       // poly is already moving
            return false;
        }
    }
    else
    {
        I_Error("EV_RotatePoly:  Invalid polyobj num: %d\n", polyNum);
    }
    pe = Z_MallocLevel(sizeof(polyevent_t));
    P_AddThinker(&pe->thinker);
    pe->thinker.function = T_RotatePoly;
    pe->polyobj = polyNum;
    if (angle)
    {
        if (angle == 255)
        {
            pe->dist = -1;
        }
        else
        {
            pe->dist = angle * (ANG90 / 64);       // Angle
        }
    }
    else
    {
        pe->dist = ANGLE_MAX - 1;
    }
    pe->speed = (speed * direction * (ANG90 / 64)) >> 3;
    poly->specialdata = pe;
    SN_StartSequence((mobj_t *) & poly->startSpot, SEQ_DOOR_STONE + poly->seqType);

    while ((mirror = GetPolyobjMirror(polyNum)) != 0)
    {
        poly = GetPolyobj(mirror);
        if (poly && poly->specialdata && !overRide)
        {                       // mirroring poly is already in motion
            break;
        }
        pe = Z_MallocLevel(sizeof(polyevent_t));
        P_AddThinker(&pe->thinker);
        pe->thinker.function = T_RotatePoly;
        poly->specialdata = pe;
        pe->polyobj = mirror;
        if (angle)
        {
            if (angle == 255)
            {
                pe->dist = -1;
            }
            else
            {
                pe->dist = angle * (ANG90 / 64);   // Angle
            }
        }
        else
        {
            pe->dist = ANGLE_MAX - 1;
        }
        poly = GetPolyobj(polyNum);
        if (poly != NULL)
        {
            poly->specialdata = pe;
        }
        else
        {
            I_Error("EV_RotatePoly:  Invalid polyobj num: %d\n", polyNum);
        }
        direction = -direction;
        pe->speed = (speed * direction * (ANG90 / 64)) >> 3;
        polyNum = mirror;
        SN_StartSequence((mobj_t *) & poly->startSpot, SEQ_DOOR_STONE + poly->seqType);
    }
    return true;
}

dboolean EV_RotatePoly(line_t * line, byte * args, int direction, dboolean overRide)
{
    return EV_RotateZDoomPoly(line, args[0], args[1], args[2], direction, overRide);
}

void T_MovePoly(polyevent_t * pe)
{
    int absSpeed;

    if (PO_MovePolyobj(pe->polyobj, pe->xSpeed, pe->ySpeed))
    {
        absSpeed = abs(pe->speed);
        pe->dist -= absSpeed;
        if (pe->dist <= 0)
        {
            StopPolyEvent(pe);
        }
        if (pe->dist < absSpeed)
        {
            pe->speed = pe->dist * (pe->speed < 0 ? -1 : 1);
            pe->xSpeed = FixedMul(pe->speed, finecosine[pe->angle]);
            pe->ySpeed = FixedMul(pe->speed, finesine[pe->angle]);
        }
    }
}

static void EV_SpawnMovePolyEvent(int polyNum, polyobj_t *poly, fixed_t speed,
                           fixed_t dist, angle_t an, dboolean overRide)
{
    int mirror;
    polyevent_t *pe;

    pe = Z_MallocLevel(sizeof(polyevent_t));
    P_AddThinker(&pe->thinker);
    pe->thinker.function = T_MovePoly;
    pe->polyobj = polyNum;
    pe->dist = dist;
    pe->speed = speed;
    poly->specialdata = pe;
    pe->angle = an >> ANGLETOFINESHIFT;
    pe->xSpeed = FixedMul(pe->speed, finecosine[pe->angle]);
    pe->ySpeed = FixedMul(pe->speed, finesine[pe->angle]);
    SN_StartSequence((mobj_t *) & poly->startSpot, SEQ_DOOR_STONE + poly->seqType);

    while ((mirror = GetPolyobjMirror(polyNum)) != 0)
    {
        poly = GetPolyobj(mirror);
        if (poly && poly->specialdata && !overRide)
        {                       // mirroring poly is already in motion
            break;
        }
        pe = Z_MallocLevel(sizeof(polyevent_t));
        P_AddThinker(&pe->thinker);
        pe->thinker.function = T_MovePoly;
        pe->polyobj = mirror;
        poly->specialdata = pe;
        pe->dist = dist;
        pe->speed = speed;
        an = an + ANG180;    // reverse the angle
        pe->angle = an >> ANGLETOFINESHIFT;
        pe->xSpeed = FixedMul(pe->speed, finecosine[pe->angle]);
        pe->ySpeed = FixedMul(pe->speed, finesine[pe->angle]);
        polyNum = mirror;
        SN_StartSequence((mobj_t *) & poly->startSpot, SEQ_DOOR_STONE + poly->seqType);
    }
}

dboolean EV_MovePolyTo(line_t * line, int polyNum, fixed_t speed,
                       fixed_t x, fixed_t y, dboolean overRide)
{
    polyobj_t *poly;
    angle_t an;
    fixed_t dist;

    poly = GetPolyobj(polyNum);
    if (poly != NULL)
    {
        if (poly->specialdata && !overRide)
        {                       // poly is already moving
            return false;
        }
    }
    else
    {
        I_Error("EV_MovePoly:  Invalid polyobj num: %d\n", polyNum);
    }

    dist = P_AproxDistance(x - poly->startSpot.x, y - poly->startSpot.y);
    an = R_PointToAngle2(poly->startSpot.x, poly->startSpot.y, x, y);

    EV_SpawnMovePolyEvent(polyNum, poly, speed, dist, an, overRide);

    return true;
}

dboolean EV_MoveZDoomPoly(line_t * line, int polyNum, int speed,
                          int angle, int distance, dboolean timesEight, dboolean overRide)
{
    polyobj_t *poly;

    poly = GetPolyobj(polyNum);
    if (poly != NULL)
    {
        if (poly->specialdata && !overRide)
        {                       // poly is already moving
            return false;
        }
    }
    else
    {
        I_Error("EV_MovePoly:  Invalid polyobj num: %d\n", polyNum);
    }

    if (timesEight)
    {
        distance *= 8 * FRACUNIT;
    }
    else
    {
        distance *= FRACUNIT;
    }

    speed *= (FRACUNIT / 8);
    angle *= (ANG90 / 64);

    EV_SpawnMovePolyEvent(polyNum, poly, speed, distance, angle, overRide);

    return true;
}

dboolean EV_MovePoly(line_t * line, byte * args, dboolean timesEight, dboolean overRide)
{
    return EV_MoveZDoomPoly(line, args[0], args[1], args[2], args[3], timesEight, overRide);
}

void T_PolyDoor(polydoor_t * pd)
{
    int absSpeed;
    polyobj_t *poly;

    if (pd->tics)
    {
        if (!--pd->tics)
        {
            poly = GetPolyobj(pd->polyobj);
            SN_StartSequence((mobj_t *) & poly->startSpot, SEQ_DOOR_STONE +
                             poly->seqType);
        }
        return;
    }
    switch (pd->type)
    {
        case PODOOR_SLIDE:
            if (PO_MovePolyobj(pd->polyobj, pd->xSpeed, pd->ySpeed))
            {
                absSpeed = abs(pd->speed);
                pd->dist -= absSpeed;
                if (pd->dist <= 0)
                {
                    poly = GetPolyobj(pd->polyobj);
                    SN_StopSequence((mobj_t *) & poly->startSpot);
                    if (!pd->close)
                    {
                        pd->dist = pd->totalDist;
                        pd->close = true;
                        pd->tics = pd->waitTics;
                        pd->direction = (ANGLE_MAX >> ANGLETOFINESHIFT) -
                            pd->direction;
                        pd->xSpeed = -pd->xSpeed;
                        pd->ySpeed = -pd->ySpeed;
                    }
                    else
                    {
                        if (poly->specialdata == pd)
                        {
                            poly->specialdata = NULL;
                        }
                        P_PolyobjFinished(poly->tag);
                        P_RemoveThinker(&pd->thinker);
                    }
                }
            }
            else
            {
                poly = GetPolyobj(pd->polyobj);
                if (poly->crush || !pd->close)
                {               // continue moving if the poly is a crusher, or is opening
                    return;
                }
                else
                {               // open back up
                    pd->dist = pd->totalDist - pd->dist;
                    pd->direction = (ANGLE_MAX >> ANGLETOFINESHIFT) -
                        pd->direction;
                    pd->xSpeed = -pd->xSpeed;
                    pd->ySpeed = -pd->ySpeed;
                    pd->close = false;
                    SN_StartSequence((mobj_t *) & poly->startSpot,
                                     SEQ_DOOR_STONE + poly->seqType);
                }
            }
            break;
        case PODOOR_SWING:
            if (PO_RotatePolyobj(pd->polyobj, pd->speed))
            {
                absSpeed = abs(pd->speed);
                if (pd->dist == -1)
                {               // perpetual polyobj
                    return;
                }
                pd->dist -= absSpeed;
                if (pd->dist <= 0)
                {
                    poly = GetPolyobj(pd->polyobj);
                    SN_StopSequence((mobj_t *) & poly->startSpot);
                    if (!pd->close)
                    {
                        pd->dist = pd->totalDist;
                        pd->close = true;
                        pd->tics = pd->waitTics;
                        pd->speed = -pd->speed;
                    }
                    else
                    {
                        if (poly->specialdata == pd)
                        {
                            poly->specialdata = NULL;
                        }
                        P_PolyobjFinished(poly->tag);
                        P_RemoveThinker(&pd->thinker);
                    }
                }
            }
            else
            {
                poly = GetPolyobj(pd->polyobj);
                if (poly->crush || !pd->close)
                {               // continue moving if the poly is a crusher, or is opening
                    return;
                }
                else
                {               // open back up and rewait
                    pd->dist = pd->totalDist - pd->dist;
                    pd->speed = -pd->speed;
                    pd->close = false;
                    SN_StartSequence((mobj_t *) & poly->startSpot,
                                     SEQ_DOOR_STONE + poly->seqType);
                }
            }
            break;
        default:
            break;
    }
}

// TODO: Split into 2 functions with accurate variable names
dboolean EV_OpenZDoomPolyDoor(line_t * line, int polyNum, int speed,
                              int angle, int distance, int delay, podoortype_t type)
{
    int mirror;
    polydoor_t *pd;
    polyobj_t *poly;
    angle_t an = 0;

    poly = GetPolyobj(polyNum);
    if (poly != NULL)
    {
        if (poly->specialdata)
        {                       // poly is already moving
            return false;
        }
    }
    else
    {
        I_Error("EV_OpenPolyDoor:  Invalid polyobj num: %d\n", polyNum);
    }
    pd = Z_MallocLevel(sizeof(polydoor_t));
    memset(pd, 0, sizeof(polydoor_t));
    P_AddThinker(&pd->thinker);
    pd->thinker.function = T_PolyDoor;
    pd->type = type;
    pd->polyobj = polyNum;
    if (type == PODOOR_SLIDE)
    {
        pd->waitTics = delay;
        pd->speed = speed * (FRACUNIT / 8);
        pd->totalDist = distance * FRACUNIT;     // Distance
        pd->dist = pd->totalDist;
        an = angle * (ANG90 / 64);
        pd->direction = an >> ANGLETOFINESHIFT;
        pd->xSpeed = FixedMul(pd->speed, finecosine[pd->direction]);
        pd->ySpeed = FixedMul(pd->speed, finesine[pd->direction]);
        SN_StartSequence((mobj_t *) & poly->startSpot, SEQ_DOOR_STONE + poly->seqType);
    }
    else if (type == PODOOR_SWING)
    {
        pd->waitTics = distance;
        pd->direction = 1;      // ADD:  PODOOR_SWINGL, PODOOR_SWINGR
        pd->speed = (speed * pd->direction * (ANG90 / 64)) >> 3;
        pd->totalDist = angle * (ANG90 / 64);
        pd->dist = pd->totalDist;
        SN_StartSequence((mobj_t *) & poly->startSpot, SEQ_DOOR_STONE + poly->seqType);
    }

    poly->specialdata = pd;

    while ((mirror = GetPolyobjMirror(polyNum)) != 0)
    {
        poly = GetPolyobj(mirror);
        if (poly && poly->specialdata)
        {                       // mirroring poly is already in motion
            break;
        }
        pd = Z_MallocLevel(sizeof(polydoor_t));
        memset(pd, 0, sizeof(polydoor_t));
        P_AddThinker(&pd->thinker);
        pd->thinker.function = T_PolyDoor;
        pd->polyobj = mirror;
        pd->type = type;
        poly->specialdata = pd;
        if (type == PODOOR_SLIDE)
        {
            pd->waitTics = delay;
            pd->speed = speed * (FRACUNIT / 8);
            pd->totalDist = distance * FRACUNIT; // Distance
            pd->dist = pd->totalDist;
            an = an + ANG180;        // reverse the angle
            pd->direction = an >> ANGLETOFINESHIFT;
            pd->xSpeed = FixedMul(pd->speed, finecosine[pd->direction]);
            pd->ySpeed = FixedMul(pd->speed, finesine[pd->direction]);
            SN_StartSequence((mobj_t *) & poly->startSpot, SEQ_DOOR_STONE + poly->seqType);
        }
        else if (type == PODOOR_SWING)
        {
            pd->waitTics = distance;
            pd->direction = -1; // ADD:  same as above
            pd->speed = (speed * pd->direction * (ANG90 / 64)) >> 3;
            pd->totalDist = angle * (ANG90 / 64);
            pd->dist = pd->totalDist;
            SN_StartSequence((mobj_t *) & poly->startSpot, SEQ_DOOR_STONE + poly->seqType);
        }
        polyNum = mirror;
    }
    return true;
}

dboolean EV_OpenPolyDoor(line_t * line, byte * args, podoortype_t type)
{
    return EV_OpenZDoomPolyDoor(line, args[0], args[1], args[2], args[3], args[4], type);
}

static polyobj_t *GetPolyobj(int polyNum)
{
    int i;

    for (i = 0; i < po_NumPolyobjs; i++)
    {
        if (polyobjs[i].tag == polyNum)
        {
            return &polyobjs[i];
        }
    }
    return NULL;
}

static int GetPolyobjMirror(int poly)
{
    int i;

    for (i = 0; i < po_NumPolyobjs; i++)
    {
        if (polyobjs[i].tag == poly)
        {
            return ((*polyobjs[i].segs)->linedef->special_args[1]);
        }
    }
    return 0;
}

static void ThrustMobj(mobj_t * mobj, seg_t * seg, polyobj_t * po)
{
    int thrustAngle;
    int thrustX;
    int thrustY;
    polyevent_t *pe;

    int force;

    if (!(mobj->flags & MF_SHOOTABLE) && !mobj->player)
    {
        return;
    }
    thrustAngle = (seg->angle - ANG90) >> ANGLETOFINESHIFT;

    pe = po->specialdata;
    if (pe)
    {
        if (pe->thinker.function == T_RotatePoly)
        {
            force = pe->speed >> 8;
        }
        else
        {
            force = pe->speed >> 3;
        }
        if (force < FRACUNIT)
        {
            force = FRACUNIT;
        }
        else if (force > 4 * FRACUNIT)
        {
            force = 4 * FRACUNIT;
        }
    }
    else
    {
        force = FRACUNIT;
    }

    thrustX = FixedMul(force, finecosine[thrustAngle]);
    thrustY = FixedMul(force, finesine[thrustAngle]);
    mobj->momx += thrustX;
    mobj->momy += thrustY;
    if (po->crush)
    {
        if (po->hurt || !P_CheckPosition(mobj, mobj->x + thrustX, mobj->y + thrustY))
        {
            P_DamageMobj(mobj, NULL, NULL, 3);
        }
    }
}

static void ExpandBoxByVertex(vertex_t * v1, fixed_t * left, fixed_t * right,
                                             fixed_t * top, fixed_t * bottom)
{
    if (v1->x > *right)
    {
        *right = v1->x;
    }
    if (v1->x < *left)
    {
        *left = v1->x;
    }
    if (v1->y > *top)
    {
        *top = v1->y;
    }
    if (v1->y < *bottom)
    {
        *bottom = v1->y;
    }
}

static void UpdateSegBBox(seg_t * seg, polyobj_t * po)
{
    line_t *line;

    line = seg->linedef;

    if (map_format.zdoom)
    {
        int i;

        line->bbox[BOXRIGHT] = line->bbox[BOXLEFT] = seg->v1->x;
        line->bbox[BOXTOP] = line->bbox[BOXBOTTOM] = seg->v1->y;

        for (i = 0; i < po->numsegs; ++i)
        {
            seg = po->segs[i];
            if (seg->linedef == line)
            {
                ExpandBoxByVertex(seg->v1, &line->bbox[BOXLEFT], &line->bbox[BOXRIGHT],
                                           &line->bbox[BOXTOP], &line->bbox[BOXBOTTOM]);

                ExpandBoxByVertex(seg->v2, &line->bbox[BOXLEFT], &line->bbox[BOXRIGHT],
                                           &line->bbox[BOXTOP], &line->bbox[BOXBOTTOM]);
            }
        }
    }
    else
    {
        if (seg->v1->x < seg->v2->x)
        {
            line->bbox[BOXLEFT] = seg->v1->x;
            line->bbox[BOXRIGHT] = seg->v2->x;
        }
        else
        {
            line->bbox[BOXLEFT] = seg->v2->x;
            line->bbox[BOXRIGHT] = seg->v1->x;
        }
        if (seg->v1->y < seg->v2->y)
        {
            line->bbox[BOXBOTTOM] = seg->v1->y;
            line->bbox[BOXTOP] = seg->v2->y;
        }
        else
        {
            line->bbox[BOXBOTTOM] = seg->v2->y;
            line->bbox[BOXTOP] = seg->v1->y;
        }
    }

    // Update the line's slopetype
    line->dx = line->v2->x - line->v1->x;
    line->dy = line->v2->y - line->v1->y;
    if (!line->dx)
    {
        line->slopetype = ST_VERTICAL;
    }
    else if (!line->dy)
    {
        line->slopetype = ST_HORIZONTAL;
    }
    else
    {
        if (FixedDiv(line->dy, line->dx) > 0)
        {
            line->slopetype = ST_POSITIVE;
        }
        else
        {
            line->slopetype = ST_NEGATIVE;
        }
    }
}

dboolean PO_MovePolyobj(int num, int x, int y)
{
    int count;
    seg_t **segList;
    seg_t **veryTempSeg;
    polyobj_t *po;
    vertex_t *prevPts;
    dboolean blocked;

    if (!(po = GetPolyobj(num)))
    {
        I_Error("PO_MovePolyobj:  Invalid polyobj number: %d\n", num);
    }

    UnLinkPolyobj(po);

    segList = po->segs;
    prevPts = po->prevPts;
    blocked = false;

    validcount++;
    for (count = po->numsegs; count; count--, segList++, prevPts++)
    {
        if ((*segList)->linedef->validcount != validcount)
        {
            (*segList)->linedef->bbox[BOXTOP] += y;
            (*segList)->linedef->bbox[BOXBOTTOM] += y;
            (*segList)->linedef->bbox[BOXLEFT] += x;
            (*segList)->linedef->bbox[BOXRIGHT] += x;
            (*segList)->linedef->validcount = validcount;
        }
        for (veryTempSeg = po->segs; veryTempSeg != segList; veryTempSeg++)
        {
            if ((*veryTempSeg)->v1 == (*segList)->v1)
            {
                break;
            }
        }
        if (veryTempSeg == segList)
        {
            (*segList)->v1->x += x;
            (*segList)->v1->y += y;
        }
        (*prevPts).x += x;      // previous points are unique for each seg
        (*prevPts).y += y;
    }
    segList = po->segs;
    for (count = po->numsegs; count; count--, segList++)
    {
        if (CheckMobjBlocking(*segList, po))
        {
            blocked = true;
        }
    }
    if (blocked)
    {
        count = po->numsegs;
        segList = po->segs;
        prevPts = po->prevPts;
        validcount++;
        while (count--)
        {
            if ((*segList)->linedef->validcount != validcount)
            {
                (*segList)->linedef->bbox[BOXTOP] -= y;
                (*segList)->linedef->bbox[BOXBOTTOM] -= y;
                (*segList)->linedef->bbox[BOXLEFT] -= x;
                (*segList)->linedef->bbox[BOXRIGHT] -= x;
                (*segList)->linedef->validcount = validcount;
            }
            for (veryTempSeg = po->segs; veryTempSeg != segList;
                 veryTempSeg++)
            {
                if ((*veryTempSeg)->v1 == (*segList)->v1)
                {
                    break;
                }
            }
            if (veryTempSeg == segList)
            {
                (*segList)->v1->x -= x;
                (*segList)->v1->y -= y;
            }
            (*prevPts).x -= x;
            (*prevPts).y -= y;
            segList++;
            prevPts++;
        }
        LinkPolyobj(po);
        return false;
    }
    po->startSpot.x += x;
    po->startSpot.y += y;
    LinkPolyobj(po);
    ResetPolySubSector(po);
    return true;
}

static void RotatePt(int an, fixed_t * x, fixed_t * y, fixed_t startSpotX,
                     fixed_t startSpotY)
{
    fixed_t trx, try;
    fixed_t gxt, gyt;

    trx = *x;
    try = *y;

    gxt = FixedMul(trx, finecosine[an]);
    gyt = FixedMul(try, finesine[an]);
    *x = (gxt - gyt) + startSpotX;

    gxt = FixedMul(trx, finesine[an]);
    gyt = FixedMul(try, finecosine[an]);
    *y = (gyt + gxt) + startSpotY;
}

dboolean PO_RotatePolyobj(int num, angle_t angle)
{
    int count;
    seg_t **segList;
    vertex_t *originalPts;
    vertex_t *prevPts;
    int an;
    polyobj_t *po;
    dboolean blocked;

    if (!(po = GetPolyobj(num)))
    {
        I_Error("PO_RotatePolyobj:  Invalid polyobj number: %d\n", num);
    }
    an = (po->angle + angle) >> ANGLETOFINESHIFT;

    UnLinkPolyobj(po);

    segList = po->segs;
    originalPts = po->originalPts;
    prevPts = po->prevPts;

    for (count = po->numsegs; count; count--, segList++, originalPts++,
         prevPts++)
    {
        prevPts->x = (*segList)->v1->x;
        prevPts->y = (*segList)->v1->y;
        (*segList)->v1->x = originalPts->x;
        (*segList)->v1->y = originalPts->y;
        RotatePt(an, &(*segList)->v1->x, &(*segList)->v1->y, po->startSpot.x,
                 po->startSpot.y);
    }
    segList = po->segs;
    blocked = false;
    validcount++;
    for (count = po->numsegs; count; count--, segList++)
    {
        if (CheckMobjBlocking(*segList, po))
        {
            blocked = true;
        }
        if ((*segList)->linedef->validcount != validcount)
        {
            UpdateSegBBox(*segList, po);
            (*segList)->linedef->validcount = validcount;
        }
        (*segList)->angle += angle;
    }
    if (blocked)
    {
        segList = po->segs;
        prevPts = po->prevPts;
        for (count = po->numsegs; count; count--, segList++, prevPts++)
        {
            (*segList)->v1->x = prevPts->x;
            (*segList)->v1->y = prevPts->y;
        }
        segList = po->segs;
        validcount++;
        for (count = po->numsegs; count; count--, segList++, prevPts++)
        {
            if ((*segList)->linedef->validcount != validcount)
            {
                UpdateSegBBox(*segList, po);
                (*segList)->linedef->validcount = validcount;
            }
            (*segList)->angle -= angle;
        }
        LinkPolyobj(po);
        return false;
    }
    po->angle += angle;
    LinkPolyobj(po);
    ResetPolySubSector(po);
    return true;
}

void UnLinkPolyobj(polyobj_t * po)
{
    polyblock_t *link;
    int i, j;
    int index;

    // remove the polyobj from each blockmap section
    for (j = po->bbox[BOXBOTTOM]; j <= po->bbox[BOXTOP]; j++)
    {
        index = j * bmapwidth;
        for (i = po->bbox[BOXLEFT]; i <= po->bbox[BOXRIGHT]; i++)
        {
            if (i >= 0 && i < bmapwidth && j >= 0 && j < bmapheight)
            {
                link = PolyBlockMap[index + i];
                while (link != NULL && link->polyobj != po)
                {
                    link = link->next;
                }
                if (link == NULL)
                {               // polyobj not located in the link cell
                    continue;
                }
                link->polyobj = NULL;
            }
        }
    }
}

void LinkPolyobj(polyobj_t * po)
{
    int leftX, rightX;
    int topY, bottomY;
    seg_t **tempSeg;
    polyblock_t **link;
    polyblock_t *tempLink;
    int i, j;

    // calculate the polyobj bbox
    tempSeg = po->segs;
    rightX = leftX = (*tempSeg)->v1->x;
    topY = bottomY = (*tempSeg)->v1->y;

    for (i = 0; i < po->numsegs; i++, tempSeg++)
    {
        if ((*tempSeg)->v1->x > rightX)
        {
            rightX = (*tempSeg)->v1->x;
        }
        if ((*tempSeg)->v1->x < leftX)
        {
            leftX = (*tempSeg)->v1->x;
        }
        if ((*tempSeg)->v1->y > topY)
        {
            topY = (*tempSeg)->v1->y;
        }
        if ((*tempSeg)->v1->y < bottomY)
        {
            bottomY = (*tempSeg)->v1->y;
        }
    }
    po->bbox[BOXRIGHT] = (rightX - bmaporgx) >> MAPBLOCKSHIFT;
    po->bbox[BOXLEFT] = (leftX - bmaporgx) >> MAPBLOCKSHIFT;
    po->bbox[BOXTOP] = (topY - bmaporgy) >> MAPBLOCKSHIFT;
    po->bbox[BOXBOTTOM] = (bottomY - bmaporgy) >> MAPBLOCKSHIFT;
    // add the polyobj to each blockmap section
    for (j = po->bbox[BOXBOTTOM] * bmapwidth;
         j <= po->bbox[BOXTOP] * bmapwidth; j += bmapwidth)
    {
        for (i = po->bbox[BOXLEFT]; i <= po->bbox[BOXRIGHT]; i++)
        {
            if (i >= 0 && i < bmapwidth && j >= 0
                && j < bmapheight * bmapwidth)
            {
                link = &PolyBlockMap[j + i];
                if (!(*link))
                {               // Create a new link at the current block cell
                    *link = Z_MallocLevel(sizeof(polyblock_t));
                    (*link)->next = NULL;
                    (*link)->prev = NULL;
                    (*link)->polyobj = po;
                    continue;
                }
                else
                {
                    tempLink = *link;
                    while (tempLink->next != NULL
                           && tempLink->polyobj != NULL)
                    {
                        tempLink = tempLink->next;
                    }
                }
                if (tempLink->polyobj == NULL)
                {
                    tempLink->polyobj = po;
                    continue;
                }
                else
                {
                    tempLink->next = Z_MallocLevel(sizeof(polyblock_t));
                    tempLink->next->next = NULL;
                    tempLink->next->prev = tempLink;
                    tempLink->next->polyobj = po;
                }
            }
            // else, don't link the polyobj, since it's off the map
        }
    }
}

static dboolean CheckMobjBlocking(seg_t * seg, polyobj_t * po)
{
    mobj_t *mobj;
    int i, j;
    int left, right, top, bottom;
    int tmbbox[4];
    line_t *ld;
    dboolean blocked;

    ld = seg->linedef;

    top = (ld->bbox[BOXTOP] - bmaporgy + MAXRADIUS) >> MAPBLOCKSHIFT;
    bottom = (ld->bbox[BOXBOTTOM] - bmaporgy - MAXRADIUS) >> MAPBLOCKSHIFT;
    left = (ld->bbox[BOXLEFT] - bmaporgx - MAXRADIUS) >> MAPBLOCKSHIFT;
    right = (ld->bbox[BOXRIGHT] - bmaporgx + MAXRADIUS) >> MAPBLOCKSHIFT;

    blocked = false;

    bottom = bottom < 0 ? 0 : bottom;
    bottom = bottom >= bmapheight ? bmapheight - 1 : bottom;
    top = top < 0 ? 0 : top;
    top = top >= bmapheight ? bmapheight - 1 : top;
    left = left < 0 ? 0 : left;
    left = left >= bmapwidth ? bmapwidth - 1 : left;
    right = right < 0 ? 0 : right;
    right = right >= bmapwidth ? bmapwidth - 1 : right;

    for (j = bottom * bmapwidth; j <= top * bmapwidth; j += bmapwidth)
    {
        for (i = left; i <= right; i++)
        {
            for (mobj = blocklinks[j + i]; mobj; mobj = mobj->bnext)
            {
                if (mobj->flags & MF_SOLID || mobj->player)
                {
                    tmbbox[BOXTOP] = mobj->y + mobj->radius;
                    tmbbox[BOXBOTTOM] = mobj->y - mobj->radius;
                    tmbbox[BOXLEFT] = mobj->x - mobj->radius;
                    tmbbox[BOXRIGHT] = mobj->x + mobj->radius;

                    if (tmbbox[BOXRIGHT] <= ld->bbox[BOXLEFT]
                        || tmbbox[BOXLEFT] >= ld->bbox[BOXRIGHT]
                        || tmbbox[BOXTOP] <= ld->bbox[BOXBOTTOM]
                        || tmbbox[BOXBOTTOM] >= ld->bbox[BOXTOP])
                    {
                        continue;
                    }
                    if (P_BoxOnLineSide(tmbbox, ld) != -1)
                    {
                        continue;
                    }
                    ThrustMobj(mobj, seg, po);
                    blocked = true;
                }
            }
        }
    }
    return blocked;
}

void PO_ResetBlockMap(dboolean allocate)
{
  if (allocate)
    PolyBlockMap = Z_MallocLevel(bmapwidth * bmapheight * sizeof(polyblock_t *));
  memset(PolyBlockMap, 0, bmapwidth * bmapheight * sizeof(polyblock_t *));
}

static void InitBlockMap(void)
{
    int i;
    int j;
    seg_t **segList;
    int leftX, rightX;
    int topY, bottomY;

    PO_ResetBlockMap(false);

    for (i = 0; i < po_NumPolyobjs; i++)
    {
        LinkPolyobj(&polyobjs[i]);

        // calculate a rough area
        // right now, working like shit...gotta fix this...
        segList = polyobjs[i].segs;
        leftX = rightX = (*segList)->v1->x;
        topY = bottomY = (*segList)->v1->y;
        for (j = 0; j < polyobjs[i].numsegs; j++, segList++)
        {
            if ((*segList)->v1->x < leftX)
            {
                leftX = (*segList)->v1->x;
            }
            if ((*segList)->v1->x > rightX)
            {
                rightX = (*segList)->v1->x;
            }
            if ((*segList)->v1->y < bottomY)
            {
                bottomY = (*segList)->v1->y;
            }
            if ((*segList)->v1->y > topY)
            {
                topY = (*segList)->v1->y;
            }
        }
    }
}

static void IterFindPolySegs(int x, int y, seg_t ** segList)
{
    int i;

    if (x == PolyStartX && y == PolyStartY)
    {
        return;
    }
    for (i = 0; i < numsegs; i++)
    {
        if (segs[i].linedef &&
            segs[i].v1->x == x &&
            segs[i].v1->y == y)
        {
            if (!segList)
            {
                PolySegCount++;
            }
            else
            {
                *segList++ = &segs[i];
            }
            IterFindPolySegs(segs[i].v2->x, segs[i].v2->y, segList);
            return;
        }
    }
    I_Error("IterFindPolySegs:  Non-closed Polyobj located.\n");
}

static void SpawnPolyobj(int index, int tag, dboolean crush, dboolean hurt)
{
    int i;
    int j;
    int psIndex;
    int psIndexOld;
    seg_t *polySegList[PO_MAXPOLYSEGS];

    for (i = 0; i < numsegs; i++)
    {
        if (segs[i].linedef &&
            segs[i].linedef->special == PO_LINE_START &&
            segs[i].linedef->special_args[0] == tag)
        {
            if (polyobjs[index].segs)
            {
                I_Error("SpawnPolyobj:  Polyobj %d already spawned.\n", tag);
            }
            segs[i].linedef->special = 0;
            segs[i].linedef->special_args[0] = 0;
            PolySegCount = 1;
            PolyStartX = segs[i].v1->x;
            PolyStartY = segs[i].v1->y;
            IterFindPolySegs(segs[i].v2->x, segs[i].v2->y, NULL);

            polyobjs[index].numsegs = PolySegCount;
            polyobjs[index].segs = Z_MallocLevel(PolySegCount * sizeof(seg_t *));
            *(polyobjs[index].segs) = &segs[i]; // insert the first seg
            IterFindPolySegs(segs[i].v2->x, segs[i].v2->y,
                             polyobjs[index].segs + 1);
            polyobjs[index].crush = crush;
            polyobjs[index].hurt = hurt;
            polyobjs[index].tag = tag;
            polyobjs[index].seqType = segs[i].linedef->special_args[2];
            if (polyobjs[index].seqType < 0
                || polyobjs[index].seqType >= SEQTYPE_NUMSEQ)
            {
                polyobjs[index].seqType = 0;
            }
            break;
        }
    }
    if (!polyobjs[index].segs)
    {                           // didn't find a polyobj through PO_LINE_START
        psIndex = 0;
        polyobjs[index].numsegs = 0;
        for (j = 1; j < PO_MAXPOLYSEGS; j++)
        {
            psIndexOld = psIndex;
            for (i = 0; i < numsegs; i++)
            {
                if (segs[i].linedef &&
                    segs[i].linedef->special == PO_LINE_EXPLICIT &&
                    segs[i].linedef->special_args[0] == tag)
                {
                    if (!segs[i].linedef->special_args[1])
                    {
                        I_Error
                            ("SpawnPolyobj:  Explicit line missing order number (probably %d) in poly %d.\n",
                             j + 1, tag);
                    }
                    if (segs[i].linedef->special_args[1] == j)
                    {
                        polySegList[psIndex] = &segs[i];
                        polyobjs[index].numsegs++;
                        psIndex++;
                        if (psIndex > PO_MAXPOLYSEGS)
                        {
                            I_Error
                                ("SpawnPolyobj:  psIndex > PO_MAXPOLYSEGS\n");
                        }
                    }
                }
            }
            // Clear out any specials for these segs...we cannot clear them out
            //      in the above loop, since we aren't guaranteed one seg per
            //              linedef.
            for (i = 0; i < numsegs; i++)
            {
                if (segs[i].linedef &&
                    segs[i].linedef->special == PO_LINE_EXPLICIT &&
                    segs[i].linedef->special_args[0] == tag
                    && segs[i].linedef->special_args[1] == j)
                {
                    segs[i].linedef->special = 0;
                    segs[i].linedef->special_args[0] = 0;
                }
            }
            if (psIndex == psIndexOld)
            {                   // Check if an explicit line order has been skipped
                // A line has been skipped if there are any more explicit
                // lines with the current tag value
                for (i = 0; i < numsegs; i++)
                {
                    if (segs[i].linedef &&
                        segs[i].linedef->special == PO_LINE_EXPLICIT &&
                        segs[i].linedef->special_args[0] == tag)
                    {
                        I_Error
                            ("SpawnPolyobj:  Missing explicit line %d for poly %d\n",
                             j, tag);
                    }
                }
            }
        }
        if (polyobjs[index].numsegs)
        {
            PolySegCount = polyobjs[index].numsegs;     // PolySegCount used globally
            polyobjs[index].crush = crush;
            polyobjs[index].hurt = hurt;
            polyobjs[index].tag = tag;
            polyobjs[index].segs = Z_MallocLevel(polyobjs[index].numsegs * sizeof(seg_t *));
            for (i = 0; i < polyobjs[index].numsegs; i++)
            {
                polyobjs[index].segs[i] = polySegList[i];
            }
            polyobjs[index].seqType = (*polyobjs[index].segs)->linedef->special_args[3];
        }

        if (!polyobjs[index].segs)
        {
            I_Error("SpawnPolyobj: Missing start / explicit line for poly %d\n", tag);
        }

        // Next, change the polyobjs first line to point to a mirror
        //              if it exists
        (*polyobjs[index].segs)->linedef->special_args[1] =
            (*polyobjs[index].segs)->linedef->special_args[2];
    }
}

static void TranslateToStartSpot(int tag, int originX, int originY)
{
    seg_t **tempSeg;
    seg_t **veryTempSeg;
    vertex_t *tempPt;
    subsector_t *sub;
    polyobj_t *po;
    int deltaX;
    int deltaY;
    vertex_t avg;               // used to find a polyobj's center, and hence subsector
    int i;

    po = NULL;
    for (i = 0; i < po_NumPolyobjs; i++)
    {
        if (polyobjs[i].tag == tag)
        {
            po = &polyobjs[i];
            break;
        }
    }
    if (!po)
    {                           // didn't match the tag with a polyobj tag
        I_Error("TranslateToStartSpot:  Unable to match polyobj tag: %d\n",
                tag);
    }
    if (po->segs == NULL)
    {
        I_Error
            ("TranslateToStartSpot:  Anchor point located without a StartSpot point: %d\n",
             tag);
    }
    po->originalPts = Z_MallocLevel(po->numsegs * sizeof(vertex_t));
    po->prevPts = Z_MallocLevel(po->numsegs * sizeof(vertex_t));
    deltaX = originX - po->startSpot.x;
    deltaY = originY - po->startSpot.y;

    tempSeg = po->segs;
    tempPt = po->originalPts;
    avg.x = 0;
    avg.y = 0;

    validcount++;
    for (i = 0; i < po->numsegs; i++, tempSeg++, tempPt++)
    {
        if ((*tempSeg)->linedef->validcount != validcount)
        {
            (*tempSeg)->linedef->bbox[BOXTOP] -= deltaY;
            (*tempSeg)->linedef->bbox[BOXBOTTOM] -= deltaY;
            (*tempSeg)->linedef->bbox[BOXLEFT] -= deltaX;
            (*tempSeg)->linedef->bbox[BOXRIGHT] -= deltaX;
            (*tempSeg)->linedef->validcount = validcount;
        }
        for (veryTempSeg = po->segs; veryTempSeg != tempSeg; veryTempSeg++)
        {
            if ((*veryTempSeg)->v1 == (*tempSeg)->v1)
            {
                break;
            }
        }
        if (veryTempSeg == tempSeg)
        {                       // the point hasn't been translated, yet
            (*tempSeg)->v1->x -= deltaX;
            (*tempSeg)->v1->y -= deltaY;
        }

        ResetSegDrawingParameters(*tempSeg);

        avg.x += (*tempSeg)->v1->x >> FRACBITS;
        avg.y += (*tempSeg)->v1->y >> FRACBITS;
        // the original Pts are based off the startSpot Pt, and are
        // unique to each seg, not each linedef
        tempPt->x = (*tempSeg)->v1->x - po->startSpot.x;
        tempPt->y = (*tempSeg)->v1->y - po->startSpot.y;
    }
    avg.x /= po->numsegs;
    avg.y /= po->numsegs;
    sub = R_PointInSubsector(avg.x << FRACBITS, avg.y << FRACBITS);
    if (sub->poly != NULL)
    {
        I_Error
            ("PO_TranslateToStartSpot:  Multiple polyobjs in a single subsector.\n");
    }
    po->subsector = sub;
    sub->poly = po;
}

dboolean PO_Detect(int doomednum)
{
  if (!map_format.polyobjs) return false;

  if (doomednum == map_format.dn_polyanchor)
  {
    return true;
  }

  if (doomednum >= map_format.dn_polyspawn_start &&
      doomednum <= map_format.dn_polyspawn_end)
  {
    po_NumPolyobjs++;
    return true;
  }

  return false;
}

void PO_LoadThings(int lump)
{
    const byte *data;
    int i;
    hexen_mapthing_t spawnthing;
    const hexen_mapthing_t *mt;
    int numthings;
    int polyIndex;

    data = W_LumpByNum(lump);
    numthings = W_LumpLength(lump) / sizeof(hexen_mapthing_t);
    mt = (const hexen_mapthing_t *) data;
    polyIndex = 0;              // index polyobj number
    // Find the startSpot points, and spawn each polyobj
    for (i = 0; i < numthings; i++, mt++)
    {
        spawnthing.x = LittleShort(mt->x);
        spawnthing.y = LittleShort(mt->y);
        spawnthing.angle = LittleShort(mt->angle);
        spawnthing.type = LittleShort(mt->type);

        // 3001 = no crush, 3002 = crushing
        if (spawnthing.type >= map_format.dn_polyspawn_start &&
            spawnthing.type <= map_format.dn_polyspawn_end)
        {                       // Polyobj StartSpot Pt.
            polyobjs[polyIndex].startSpot.x = spawnthing.x << FRACBITS;
            polyobjs[polyIndex].startSpot.y = spawnthing.y << FRACBITS;
            SpawnPolyobj(polyIndex, spawnthing.angle,
                         (spawnthing.type != map_format.dn_polyspawn_start),
                         (spawnthing.type == map_format.dn_polyspawn_hurt));
            polyIndex++;
        }
    }
    mt = (const hexen_mapthing_t *) data;
    for (i = 0; i < numthings; i++, mt++)
    {
        spawnthing.x = LittleShort(mt->x);
        spawnthing.y = LittleShort(mt->y);
        spawnthing.angle = LittleShort(mt->angle);
        spawnthing.type = LittleShort(mt->type);
        if (spawnthing.type == map_format.dn_polyanchor)
        {                       // Polyobj Anchor Pt.
            TranslateToStartSpot(spawnthing.angle,
                                 spawnthing.x << FRACBITS,
                                 spawnthing.y << FRACBITS);
        }
    }
}

void PO_LoadUDMFThings(int lump)
{
    int i;
    const udmf_thing_t *mt;
    int polyIndex = 0;

    // Find the startSpot points, and spawn each polyobj
    for (i = 0, mt = &udmf.things[0]; i < udmf.num_things; i++, mt++)
    {
        // 3001 = no crush, 3002 = crushing
        if (mt->type >= map_format.dn_polyspawn_start &&
            mt->type <= map_format.dn_polyspawn_end)
        {                       // Polyobj StartSpot Pt.
            polyobjs[polyIndex].startSpot.x = dsda_StringToFixed(mt->x);
            polyobjs[polyIndex].startSpot.y = dsda_StringToFixed(mt->y);
            SpawnPolyobj(polyIndex, mt->angle,
                         (mt->type != map_format.dn_polyspawn_start),
                         (mt->type == map_format.dn_polyspawn_hurt));
            polyIndex++;
        }
    }

    for (i = 0, mt = &udmf.things[0]; i < udmf.num_things; i++, mt++)
    {
        if (mt->type == map_format.dn_polyanchor)
        {                       // Polyobj Anchor Pt.
            TranslateToStartSpot(mt->angle,
                                 dsda_StringToFixed(mt->x),
                                 dsda_StringToFixed(mt->y));
        }
    }

    if (polyIndex)
    {
        dsda_PreferOpenGL();
    }
}

void PO_Init(int lump)
{
    int i;

    polyobjs = Z_MallocLevel(po_NumPolyobjs * sizeof(polyobj_t));
    memset(polyobjs, 0, po_NumPolyobjs * sizeof(polyobj_t));

    map_loader.po_load_things(lump);

    // check for a startspot without an anchor point
    for (i = 0; i < po_NumPolyobjs; i++)
    {
        if (!polyobjs[i].originalPts)
        {
            I_Error
                ("PO_Init:  StartSpot located without an Anchor point: %d\n",
                 polyobjs[i].tag);
        }
    }
    InitBlockMap();
}

dboolean PO_Busy(int polyobj)
{
    polyobj_t *poly;

    poly = GetPolyobj(polyobj);
    if (!poly->specialdata)
    {
        return false;
    }
    else
    {
        return true;
    }
}
