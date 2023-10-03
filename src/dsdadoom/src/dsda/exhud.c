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
//	DSDA Extended HUD
//

#include <stdio.h>

#include "am_map.h"
#include "doomstat.h"
#include "hu_stuff.h"
#include "lprintf.h"
#include "m_file.h"
#include "r_main.h"
#include "v_video.h"
#include "w_wad.h"

#include "dsda/args.h"
#include "dsda/console.h"
#include "dsda/global.h"
#include "dsda/hud_components.h"
#include "dsda/render_stats.h"
#include "dsda/settings.h"
#include "dsda/utility.h"

#include "exhud.h"

typedef struct {
  void (*init)(int x_offset, int y_offset, int vpt_flags, int* args, int arg_count, void** data);
  void (*update)(void* data);
  void (*draw)(void* data);
  const char* name;
  const int default_vpt;
  const dboolean strict;
  const dboolean off_by_default;
  const dboolean intermission;
  const dboolean not_level;
  dboolean on;
  dboolean initialized;
  void* data;
} exhud_component_t;

typedef enum {
  exhud_ammo_text,
  exhud_armor_text,
  exhud_big_ammo,
  exhud_big_armor,
  exhud_big_armor_text,
  exhud_big_artifact,
  exhud_big_health,
  exhud_big_health_text,
  exhud_composite_time,
  exhud_health_text,
  exhud_keys,
  exhud_ready_ammo_text,
  exhud_speed_text,
  exhud_stat_totals,
  exhud_tracker,
  exhud_weapon_text,
  exhud_render_stats,
  exhud_fps,
  exhud_attempts,
  exhud_local_time,
  exhud_coordinate_display,
  exhud_line_display,
  exhud_command_display,
  exhud_event_split,
  exhud_level_splits,
  exhud_color_test,
  exhud_free_text,
  exhud_message,
  exhud_secret_message,
  exhud_map_coordinates,
  exhud_map_time,
  exhud_map_title,
  exhud_map_totals,
  exhud_minimap,
  exhud_component_count,
} exhud_component_id_t;

exhud_component_t components_template[exhud_component_count] = {
  [exhud_ammo_text] = {
    dsda_InitAmmoTextHC,
    dsda_UpdateAmmoTextHC,
    dsda_DrawAmmoTextHC,
    "ammo_text",
    .default_vpt = VPT_EX_TEXT,
  },
  [exhud_armor_text] = {
    dsda_InitArmorTextHC,
    dsda_UpdateArmorTextHC,
    dsda_DrawArmorTextHC,
    "armor_text",
    .default_vpt = VPT_EX_TEXT,
  },
  [exhud_big_ammo] = {
    dsda_InitBigAmmoHC,
    dsda_UpdateBigAmmoHC,
    dsda_DrawBigAmmoHC,
    "big_ammo",
    .default_vpt = VPT_EX_TEXT,
  },
  [exhud_big_armor] = {
    dsda_InitBigArmorHC,
    dsda_UpdateBigArmorHC,
    dsda_DrawBigArmorHC,
    "big_armor",
    .default_vpt = VPT_EX_TEXT | VPT_NOOFFSET,
  },
  [exhud_big_armor_text] = {
    dsda_InitBigArmorTextHC,
    dsda_UpdateBigArmorTextHC,
    dsda_DrawBigArmorTextHC,
    "big_armor_text",
    .default_vpt = VPT_EX_TEXT,
  },
  [exhud_big_artifact] = {
    dsda_InitBigArtifactHC,
    dsda_UpdateBigArtifactHC,
    dsda_DrawBigArtifactHC,
    "big_artifact",
    .default_vpt = VPT_EX_TEXT,
  },
  [exhud_big_health] = {
    dsda_InitBigHealthHC,
    dsda_UpdateBigHealthHC,
    dsda_DrawBigHealthHC,
    "big_health",
    .default_vpt = VPT_EX_TEXT | VPT_NOOFFSET,
  },
  [exhud_big_health_text] = {
    dsda_InitBigHealthTextHC,
    dsda_UpdateBigHealthTextHC,
    dsda_DrawBigHealthTextHC,
    "big_health_text",
    .default_vpt = VPT_EX_TEXT,
  },
  [exhud_composite_time] = {
    dsda_InitCompositeTimeHC,
    dsda_UpdateCompositeTimeHC,
    dsda_DrawCompositeTimeHC,
    "composite_time",
    .default_vpt = VPT_EX_TEXT,
  },
  [exhud_health_text] = {
    dsda_InitHealthTextHC,
    dsda_UpdateHealthTextHC,
    dsda_DrawHealthTextHC,
    "health_text",
    .default_vpt = VPT_EX_TEXT,
  },
  [exhud_keys] = {
    dsda_InitKeysHC,
    dsda_UpdateKeysHC,
    dsda_DrawKeysHC,
    "keys",
    .default_vpt = VPT_EX_TEXT | VPT_NOOFFSET,
  },
  [exhud_ready_ammo_text] = {
    dsda_InitReadyAmmoTextHC,
    dsda_UpdateReadyAmmoTextHC,
    dsda_DrawReadyAmmoTextHC,
    "ready_ammo_text",
    .default_vpt = VPT_EX_TEXT,
  },
  [exhud_speed_text] = {
    dsda_InitSpeedTextHC,
    dsda_UpdateSpeedTextHC,
    dsda_DrawSpeedTextHC,
    "speed_text",
    .default_vpt = VPT_EX_TEXT,
  },
  [exhud_stat_totals] = {
    dsda_InitStatTotalsHC,
    dsda_UpdateStatTotalsHC,
    dsda_DrawStatTotalsHC,
    "stat_totals",
    .default_vpt = VPT_EX_TEXT,
  },
  [exhud_tracker] = {
    dsda_InitTrackerHC,
    dsda_UpdateTrackerHC,
    dsda_DrawTrackerHC,
    "tracker",
    .default_vpt = VPT_EX_TEXT,
    .strict = true,
  },
  [exhud_weapon_text] = {
    dsda_InitWeaponTextHC,
    dsda_UpdateWeaponTextHC,
    dsda_DrawWeaponTextHC,
    "weapon_text",
    .default_vpt = VPT_EX_TEXT,
  },
  [exhud_render_stats] = {
    dsda_InitRenderStatsHC,
    dsda_UpdateRenderStatsHC,
    dsda_DrawRenderStatsHC,
    "render_stats",
    .default_vpt = VPT_EX_TEXT,
    .strict = true,
    .off_by_default = true,
  },
  [exhud_fps] = {
    dsda_InitFPSHC,
    dsda_UpdateFPSHC,
    dsda_DrawFPSHC,
    "fps",
    .default_vpt = VPT_EX_TEXT,
    .off_by_default = true,
  },
  [exhud_attempts] = {
    dsda_InitAttemptsHC,
    dsda_UpdateAttemptsHC,
    dsda_DrawAttemptsHC,
    "attempts",
    .default_vpt = VPT_EX_TEXT,
  },
  [exhud_local_time] = {
    dsda_InitLocalTimeHC,
    dsda_UpdateLocalTimeHC,
    dsda_DrawLocalTimeHC,
    "local_time",
    .default_vpt = VPT_EX_TEXT,
  },
  [exhud_coordinate_display] = {
    dsda_InitCoordinateDisplayHC,
    dsda_UpdateCoordinateDisplayHC,
    dsda_DrawCoordinateDisplayHC,
    "coordinate_display",
    .default_vpt = VPT_EX_TEXT,
    .strict = true,
    .off_by_default = true,
  },
  [exhud_line_display] = {
    dsda_InitLineDisplayHC,
    dsda_UpdateLineDisplayHC,
    dsda_DrawLineDisplayHC,
    "line_display",
    .default_vpt = VPT_EX_TEXT,
    .strict = true,
    .off_by_default = true,
  },
  [exhud_command_display] = {
    dsda_InitCommandDisplayHC,
    dsda_UpdateCommandDisplayHC,
    dsda_DrawCommandDisplayHC,
    "command_display",
    .default_vpt = VPT_EX_TEXT,
    .strict = true,
    .off_by_default = true,
    .intermission = true,
  },
  [exhud_event_split] = {
    dsda_InitEventSplitHC,
    dsda_UpdateEventSplitHC,
    dsda_DrawEventSplitHC,
    "event_split",
    .default_vpt = VPT_EX_TEXT,
  },
  [exhud_level_splits] = {
    dsda_InitLevelSplitsHC,
    dsda_UpdateLevelSplitsHC,
    dsda_DrawLevelSplitsHC,
    "level_splits",
    .default_vpt = VPT_EX_TEXT,
    .intermission = true,
    .not_level = true,
  },
  [exhud_color_test] = {
    dsda_InitColorTestHC,
    dsda_UpdateColorTestHC,
    dsda_DrawColorTestHC,
    "color_test",
    .default_vpt = VPT_EX_TEXT,
  },
  [exhud_free_text] = {
    dsda_InitFreeTextHC,
    dsda_UpdateFreeTextHC,
    dsda_DrawFreeTextHC,
    "free_text",
    .default_vpt = VPT_EX_TEXT,
  },
  [exhud_message] = {
    dsda_InitMessageHC,
    dsda_UpdateMessageHC,
    dsda_DrawMessageHC,
    "message",
  },
  [exhud_secret_message] = {
    dsda_InitSecretMessageHC,
    dsda_UpdateSecretMessageHC,
    dsda_DrawSecretMessageHC,
    "secret_message",
  },
  [exhud_map_coordinates] = {
    dsda_InitMapCoordinatesHC,
    dsda_UpdateMapCoordinatesHC,
    dsda_DrawMapCoordinatesHC,
    "map_coordinates",
    .strict = true,
  },
  [exhud_map_time] = {
    dsda_InitMapTimeHC,
    dsda_UpdateMapTimeHC,
    dsda_DrawMapTimeHC,
    "map_time",
  },
  [exhud_map_title] = {
    dsda_InitMapTitleHC,
    dsda_UpdateMapTitleHC,
    dsda_DrawMapTitleHC,
    "map_title",
  },
  [exhud_map_totals] = {
    dsda_InitMapTotalsHC,
    dsda_UpdateMapTotalsHC,
    dsda_DrawMapTotalsHC,
    "map_totals",
  },
  [exhud_minimap] = {
    dsda_InitMinimapHC,
    dsda_UpdateMinimapHC,
    dsda_DrawMinimapHC,
    "minimap",
    .default_vpt = VPT_EX_TEXT,
    .off_by_default = true,
  },
};

typedef struct {
  const char* name;
  dboolean status_bar;
  dboolean allow_offset;
  dboolean loaded;
  exhud_component_t components[exhud_component_count];
  int y_offset[VPT_ALIGN_MAX];
} dsda_hud_container_t;

typedef enum {
  hud_ex,
  hud_off,
  hud_full,
  hud_map,
  hud_null,
} dsda_hud_variant_t;

static dsda_hud_container_t containers[] = {
  [hud_ex]   = { "ex", true, true },
  [hud_off]  = { "off", true, true },
  [hud_full] = { "full", false, true },
  [hud_map] = { "map", true, false },
  [hud_null] = { NULL }
};

static dsda_hud_container_t* container;
static exhud_component_t* components;

int dsda_show_render_stats;

int dsda_ExHudVerticalOffset(void) {
  if (container && container->status_bar)
    return g_st_height;

  return 0;
}

static void dsda_TurnComponentOn(int id) {
  if (!components[id].initialized)
    return;

  components[id].on = true;
}

static void dsda_TurnComponentOff(int id) {
  components[id].on = false;
}

static void dsda_InitializeComponent(int id, int x, int y, int vpt, int* args, int arg_count) {
  components[id].initialized = true;
  components[id].init(x, y, vpt | components[id].default_vpt,
                      args, arg_count, &components[id].data);

  if (components[id].off_by_default)
    dsda_TurnComponentOff(id);
  else
    dsda_TurnComponentOn(id);
}

static int dsda_AlignmentToVPT(const char* alignment) {
  if (!strcmp(alignment, "bottom_left"))
    return VPT_ALIGN_LEFT_BOTTOM;
  else if (!strcmp(alignment, "bottom_right"))
    return VPT_ALIGN_RIGHT_BOTTOM;
  else if (!strcmp(alignment, "top_left"))
    return VPT_ALIGN_LEFT_TOP;
  else if (!strcmp(alignment, "top_right"))
    return VPT_ALIGN_RIGHT_TOP;
  else if (!strcmp(alignment, "top"))
    return VPT_ALIGN_TOP;
  else if (!strcmp(alignment, "bottom"))
    return VPT_ALIGN_BOTTOM;
  else if (!strcmp(alignment, "left"))
    return VPT_ALIGN_LEFT;
  else if (!strcmp(alignment, "right"))
    return VPT_ALIGN_RIGHT;
  else if (!strcmp(alignment, "none"))
    return VPT_STRETCH;
  else
    return -1;
}

static int dsda_ParseHUDConfig(char** hud_config, int line_i) {
  int i;
  int count;
  dboolean found;
  const char* line;
  char command[64];
  char args[64];

  for (++line_i; hud_config[line_i]; ++line_i) {
    line = hud_config[line_i];

    if (line[0] == '#' || line[0] == '/' || line[0] == '!' || !line[0])
      continue;

    count = sscanf(line, "%63s %63[^\n\r]", command, args);
    if (count != 2)
      I_Error("Invalid hud definition \"%s\"", line);

    // The start of another definition
    if (!strncmp(command, "doom", sizeof(command)) ||
        !strncmp(command, "heretic", sizeof(command)) ||
        !strncmp(command, "hexen", sizeof(command)))
      break;

    found = false;

    for (i = 0; i < exhud_component_count; ++i)
      if (!strncmp(command, components[i].name, sizeof(command))) {
        int x, y;
        int vpt;
        int component_args[4] = { 0 };
        char alignment[16];

        found = true;

        count = sscanf(args, "%d %d %15s %d %d %d %d", &x, &y, alignment,
                        &component_args[0], &component_args[1],
                        &component_args[2], &component_args[3]);
        if (count < 3)
          I_Error("Invalid hud component args \"%s\"", line);

        vpt = dsda_AlignmentToVPT(alignment);
        if (vpt < 0)
          I_Error("Invalid hud component alignment \"%s\"", line);

        dsda_InitializeComponent(i, x, y, vpt, component_args, count - 3);
      }

    if (!strncmp(command, "add_offset", sizeof(command))) {
      int offset;
      int vpt;
      char alignment[16];

      found = true;

      if (!container->allow_offset)
        I_Error("The %s config does not support add_offset", container->name);

      count = sscanf(args, "%d %15s", &offset, alignment);
      if (count != 2)
        I_Error("Invalid hud offset \"%s\"", line);

      vpt = dsda_AlignmentToVPT(alignment);
      if (vpt < 0) {
        I_Error("Invalid hud offset alignment \"%s\"", line);
        vpt = 0; // TODO: remove after I_Error marked noreturn
      }

      container->y_offset[vpt] = offset;

      if (BOTTOM_ALIGNMENT(vpt))
        container->y_offset[vpt] = -container->y_offset[vpt];
    }

    if (!found)
      I_Error("Invalid hud component \"%s\"", line);
  }

  // roll back the line that wasn't part of this config
  return line_i - 1;
}

static void dsda_ParseHUDConfigs(char** hud_config) {
  const char* line;
  int line_i;
  const char* target_format;
  char hud_variant[5];

  target_format = hexen ? "hexen %s" : heretic ? "heretic %s" : "doom %s";

  for (line_i = 0; hud_config[line_i]; ++line_i) {
    line = hud_config[line_i];

    if (sscanf(line, target_format, hud_variant)) {
      for (container = containers; container->name; container++)
        if (!strncmp(container->name, hud_variant, sizeof(hud_variant))) {
          if (container->loaded)
            break;

          container->loaded = true;
          components = container->components;
          memcpy(components, components_template, sizeof(components_template));

          line_i = dsda_ParseHUDConfig(hud_config, line_i);

          break;
        }
    }
  }
}

static void dsda_LoadHUDConfig(void) {
  DO_ONCE
    char* hud_config = NULL;
    char** lines;
    dsda_arg_t* arg;
    int lump;
    int length = 0;

    arg = dsda_Arg(dsda_arg_hud);
    if (arg->found)
      length = M_ReadFileToString(arg->value.v_string, &hud_config);

    lump = W_GetNumForName("DSDAHUD");

    if (lump != -1) {
      if (!hud_config)
        hud_config = W_ReadLumpToString(lump);
      else {
        hud_config = Z_Realloc(hud_config, length + W_LumpLength(lump) + 2);
        hud_config[length++] = '\n'; // in case the file didn't end in a new line
        W_ReadLump(lump, hud_config + length);
        hud_config[length + W_LumpLength(lump)] = '\0';
      }
    }

    if (hud_config) {
      lines = dsda_SplitString(hud_config, "\n\r");

      if (lines) {
        dsda_ParseHUDConfigs(lines);

        Z_Free(lines);
      }

      Z_Free(hud_config);
    }
  END_ONCE
}

static dboolean dsda_HideHUD(void) {
  return dsda_Flag(dsda_arg_nodraw) ||
         (R_FullView() && !dsda_IntConfig(dsda_config_hud_displayed));
}

static dboolean dsda_HUDActive(void) {
  return container && container->loaded;
}

static void dsda_ResetActiveHUD(void) {
  container = NULL;
  components = NULL;
}

static void dsda_UpdateActiveHUD(void) {
  container = R_FullView() ? &containers[hud_full] :
              dsda_IntConfig(dsda_config_exhud) ? &containers[hud_ex] :
              &containers[hud_off];

  if (container->loaded)
    components = container->components;
  else
    dsda_ResetActiveHUD();
}

static void dsda_ResetOffsets(void) {
  void dsda_UpdateExTextOffset(enum patch_translation_e flags, int offset);
  void dsda_ResetExTextOffsets(void);

  int i;

  dsda_ResetExTextOffsets();

  for (i = 0; i < VPT_ALIGN_MAX; ++i)
    if (container->y_offset[i])
      dsda_UpdateExTextOffset(i, container->y_offset[i]);
}

static void dsda_RefreshHUD(void) {
  if (!dsda_HUDActive())
    return;

  dsda_ResetOffsets();

  if (dsda_show_render_stats)
    dsda_TurnComponentOn(exhud_render_stats);

  dsda_RefreshExHudFPS();
  dsda_RefreshExHudMinimap();
  dsda_RefreshExHudLevelSplits();
  dsda_RefreshExHudCoordinateDisplay();
  dsda_RefreshExHudCommandDisplay();
  dsda_RefreshMapCoordinates();
  dsda_RefreshMapTotals();
  dsda_RefreshMapTime();
  dsda_RefreshMapTitle();

  if (in_game && gamestate == GS_LEVEL)
    dsda_UpdateExHud();
}

void dsda_InitExHud(void) {
  dsda_ResetActiveHUD();

  if (dsda_HideHUD())
    return;

  dsda_LoadHUDConfig();
  dsda_UpdateActiveHUD();
  dsda_RefreshHUD();
}

static void dsda_UpdateComponents(exhud_component_t* update_components) {
  int i;

  for (i = 0; i < exhud_component_count; ++i)
    if (
      update_components[i].on &&
      !update_components[i].not_level &&
      (!update_components[i].strict || !dsda_StrictMode())
    )
      update_components[i].update(update_components[i].data);
}

void dsda_UpdateExHud(void) {
  if (automap_on) {
    if (containers[hud_map].loaded)
      dsda_UpdateComponents(containers[hud_map].components);

    return;
  }

  if (!dsda_HUDActive())
    return;

  dsda_UpdateComponents(components);
}

static void dsda_DrawComponents(exhud_component_t* draw_components) {
  int i;

  for (i = 0; i < exhud_component_count; ++i)
    if (
      draw_components[i].on &&
      !draw_components[i].not_level &&
      (!draw_components[i].strict || !dsda_StrictMode())
    )
      draw_components[i].draw(draw_components[i].data);
}

int global_patch_top_offset;

void dsda_DrawExHud(void) {
  global_patch_top_offset = M_ConsoleOpen() ? dsda_ConsoleHeight() : 0;

  if (automap_on) {
    if (containers[hud_map].loaded)
      dsda_DrawComponents(containers[hud_map].components);
  }
  else if (dsda_HUDActive())
    dsda_DrawComponents(components);

  global_patch_top_offset = 0;
}

void dsda_DrawExIntermission(void) {
  int i;

  if (!dsda_HUDActive())
    return;

  for (i = 0; i < exhud_component_count; ++i)
    if (
      components[i].on &&
      components[i].intermission &&
      (!components[i].strict || !dsda_StrictMode())
    )
      components[i].draw(components[i].data);
}

void dsda_ToggleRenderStats(void) {
  dsda_show_render_stats = !dsda_show_render_stats;

  if (!dsda_HUDActive())
    return;

  if (components[exhud_render_stats].on && !dsda_show_render_stats)
    dsda_TurnComponentOff(exhud_render_stats);
  else if (!components[exhud_render_stats].on && dsda_show_render_stats) {
    dsda_BeginRenderStats();
    dsda_TurnComponentOn(exhud_render_stats);
  }
}

static void dsda_BasicRefresh(dboolean (*show_component)(void), exhud_component_id_t id) {
  if (!dsda_HUDActive())
    return;

  if (show_component())
    dsda_TurnComponentOn(id);
  else
    dsda_TurnComponentOff(id);
}

static void dsda_BasicMapRefresh(dboolean (*show_component)(void), exhud_component_id_t id) {
  exhud_component_t* old_components;

  if (!dsda_HUDActive())
    return;

  old_components = components;
  components = containers[hud_map].components;

  if (show_component())
    dsda_TurnComponentOn(id);
  else
    dsda_TurnComponentOff(id);

  components = old_components;
}

void dsda_RefreshExHudFPS(void) {
  dsda_BasicRefresh(dsda_ShowFPS, exhud_fps);
}

void dsda_RefreshExHudMinimap(void) {
  if (!dsda_HUDActive())
    return;

  if (dsda_ShowMinimap()) {
    dsda_TurnComponentOn(exhud_minimap);

    // Need to update the component before calling AM_Start
    if (components[exhud_minimap].initialized)
      components[exhud_minimap].update(components[exhud_minimap].data);

    if (in_game && gamestate == GS_LEVEL)
      AM_Start(false);
  }
  else
    dsda_TurnComponentOff(exhud_minimap);
}

void dsda_RefreshExHudLevelSplits(void) {
  dsda_BasicRefresh(dsda_ShowLevelSplits, exhud_level_splits);
}

void dsda_RefreshExHudCoordinateDisplay(void) {
  if (!dsda_HUDActive())
    return;

  if (dsda_CoordinateDisplay()) {
    dsda_TurnComponentOn(exhud_coordinate_display);
    dsda_TurnComponentOn(exhud_line_display);
  }
  else {
    dsda_TurnComponentOff(exhud_coordinate_display);
    dsda_TurnComponentOff(exhud_line_display);
  }
}

void dsda_RefreshExHudCommandDisplay(void) {
  dsda_BasicRefresh(dsda_CommandDisplay, exhud_command_display);
}

void dsda_RefreshMapCoordinates(void) {
  dsda_BasicMapRefresh(dsda_MapCoordinates, exhud_map_coordinates);
}

void dsda_RefreshMapTotals(void) {
  dsda_BasicMapRefresh(dsda_MapTotals, exhud_map_totals);
}

void dsda_RefreshMapTime(void) {
  dsda_BasicMapRefresh(dsda_MapTime, exhud_map_time);
}

void dsda_RefreshMapTitle(void) {
  dsda_BasicMapRefresh(dsda_MapTitle, exhud_map_title);
}
