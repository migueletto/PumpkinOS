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
//	DSDA Map Format
//

#ifndef __DSDA_MAP_FORMAT__
#define __DSDA_MAP_FORMAT__

#include "doomtype.h"
#include "r_defs.h"
#include "p_maputl.h"
#include "p_spec.h"

// visibility flags - hide things that don't match
#define VF_DOOM    0x01
#define VF_HERETIC 0x02
#define VF_HEXEN   0x04
#define VF_ZDOOM   0x08

typedef struct {
  dboolean zdoom;
  dboolean hexen;
  dboolean polyobjs;
  dboolean acs;
  dboolean thing_id;
  dboolean sndseq;
  dboolean sndinfo;
  dboolean animdefs;
  dboolean doublesky;
  dboolean map99;
  short generalized_mask;
  line_activation_t switch_activation;
  void (*init_sector_special)(sector_t*, int);
  void (*player_in_special_sector)(player_t*, sector_t*);
  dboolean (*mobj_in_special_sector)(mobj_t*);
  void (*spawn_scroller)(line_t*, int);
  void (*spawn_friction)(line_t*);
  void (*spawn_pusher)(line_t*);
  void (*spawn_extra)(line_t*, int);
  void (*cross_special_line)(line_t *, int, mobj_t *, dboolean);
  void (*shoot_special_line)(mobj_t *, line_t *);
  dboolean (*test_activate_line)(line_t *, mobj_t *, int, line_activation_t);
  dboolean (*execute_line_special)(int, int *, line_t *, int, mobj_t *);
  void (*post_process_line_special)(line_t *);
  void (*post_process_sidedef_special)(side_t *, const mapsidedef_t *, sector_t *, int);
  void (*animate_surfaces)(void);
  void (*check_impact)(mobj_t *);
  void (*translate_line_flags)(unsigned int *, line_activation_t *);
  void (*apply_sector_movement_special)(mobj_t *, int);
  void (*t_vertical_door)(vldoor_t *);
  void (*t_move_floor)(floormove_t *);
  void (*t_move_ceiling)(ceiling_t *);
  void (*t_build_pillar)(pillar_t *);
  void (*t_plat_raise)(plat_t *);
  int (*ev_teleport)(short, int, line_t *, int, mobj_t *, int);
  void (*player_thrust)(player_t* player, angle_t angle, fixed_t move);
  void (*build_mobj_thing_id_list)(void);
  void (*add_mobj_thing_id)(mobj_t *, short);
  void (*remove_mobj_thing_id)(mobj_t *);
  void (*iterate_spechit)(mobj_t *, fixed_t, fixed_t);
  int (*point_on_side)(fixed_t, fixed_t, const node_t *);
  int (*point_on_seg_side)(fixed_t, fixed_t, const seg_t *);
  int (*point_on_line_side)(fixed_t, fixed_t, const line_t *);
  int (*point_on_divline_side)(fixed_t, fixed_t, const divline_t *);
  size_t mapthing_size;
  size_t maplinedef_size;
  int mt_push;
  int mt_pull;
  int dn_polyanchor;
  int dn_polyspawn_start;
  int dn_polyspawn_hurt;
  int dn_polyspawn_end;
  int visibility;
} map_format_t;

extern map_format_t map_format;

int dsda_DoorType(int index);
dboolean dsda_IsExitLine(int index);
dboolean dsda_IsTeleportLine(int index);
void dsda_ApplyZDoomMapFormat(void);
void dsda_ApplyDefaultMapFormat(void);

#endif
