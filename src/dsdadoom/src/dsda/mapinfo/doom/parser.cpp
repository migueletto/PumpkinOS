//
// Copyright(C) 2023 by Ryan Krafnick
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
//	DSDA MAPINFO Doom Parser
//

#include <cstring>
#include <cstdio>
#include <vector>
#include <string>

extern "C" {
#include "doomstat.h"
#include "z_zone.h"

#include "dsda/name.h"
#include "dsda/utility.h"
#include "stricmp.h"
}

#include "scanner.h"

#include "parser.h"

static std::vector<doom_mapinfo_map_t> doom_mapinfo_maps;
static doom_mapinfo_map_t default_map;

static std::vector<doom_mapinfo_skill_t> doom_mapinfo_skills;
static std::vector<doom_mapinfo_cluster_t> doom_mapinfo_clusters;
static std::vector<doom_mapinfo_episode_t> doom_mapinfo_episodes;

static void dsda_SkipValue(Scanner &scanner) {
  if (scanner.CheckToken('=')) {
    do {
      scanner.GetNextToken();
    } while (scanner.CheckToken(','));

    return;
  }

  if (scanner.CheckToken('{'))
  {
    int brace_count = 1;

    while (scanner.TokensLeft()) {
      if (scanner.CheckToken('}')) {
        --brace_count;
      }
      else if (scanner.CheckToken('{')) {
        ++brace_count;
      }

      if (!brace_count)
        break;

      scanner.GetNextToken();
    }

    return;
  }
}

#define STR_DUP(x) { Z_Free(x); x = Z_Strdup(scanner.string); }
#define RESET_STR(x) { Z_Free(x); x = NULL; }

// The scanner drops the sign when scanning, and we need it back
static void dsda_FloatString(Scanner &scanner, char* &str) {
  if (scanner.decimal >= 0) {
    STR_DUP(str);
  }
  else {
    str = (char*) Z_Realloc(str, strlen(scanner.string) + 2);
    str[0] = '-';
    str[1] = '\0';
    strcat(str, scanner.string);
  }
}

#define SCAN_INT(x)  { scanner.MustGetToken('='); \
                       scanner.MustGetInteger(); \
                       x = scanner.number; }

#define SCAN_FLOAT(x) { scanner.MustGetToken('='); \
                        scanner.MustGetFloat(); \
                        x = scanner.decimal; }

#define SCAN_STRING(x) { scanner.MustGetToken('='); \
                         scanner.MustGetString(); \
                         STR_DUP(x); }

#define SCAN_FLOAT_STRING(x) { scanner.MustGetToken('='); \
                               scanner.MustGetFloat(); \
                               dsda_FloatString(scanner, x); }

static void dsda_ParseDoomMapInfoMultiString(Scanner &scanner, char* &str) {
  scanner.MustGetToken('=');
  scanner.MustGetString();
  std::string multi_str(scanner.string);

  while (scanner.CheckToken(',')) {
    scanner.MustGetString();
    multi_str += "\n";
    multi_str += scanner.string;
  }

  Z_Free(str);
  str = Z_Strdup(multi_str.c_str());
}

static void dsda_ResetMap(doom_mapinfo_map_t &map) {
  Z_Free(map.lump_name);
  Z_Free(map.nice_name);
  Z_Free(map.title_patch);
  Z_Free(map.music);
  Z_Free(map.inter_music);
  Z_Free(map.exit_pic);
  Z_Free(map.enter_pic);
  Z_Free(map.border_texture);
  Z_Free(map.gravity);
  Z_Free(map.air_control);
  Z_Free(map.author);
  Z_Free(map.special_actions);

  Z_Free(map.next.map);
  Z_Free(map.next.end_pic);
  Z_Free(map.next.end_pic_b);
  Z_Free(map.next.music);
  Z_Free(map.secret_next.map);
  Z_Free(map.secret_next.end_pic);
  Z_Free(map.secret_next.end_pic_b);
  Z_Free(map.secret_next.music);

  Z_Free(map.sky1.lump);

  memset(&map, 0, sizeof(map));
}

#define REPLACE_WITH_COPY(x) { if (x) x = Z_Strdup(x); }

static void dsda_CopyMap(doom_mapinfo_map_t &dest, doom_mapinfo_map_t &source) {
  dest = source;

  REPLACE_WITH_COPY(dest.lump_name);
  REPLACE_WITH_COPY(dest.nice_name);
  REPLACE_WITH_COPY(dest.title_patch);
  REPLACE_WITH_COPY(dest.music);
  REPLACE_WITH_COPY(dest.inter_music);
  REPLACE_WITH_COPY(dest.exit_pic);
  REPLACE_WITH_COPY(dest.enter_pic);
  REPLACE_WITH_COPY(dest.border_texture);
  REPLACE_WITH_COPY(dest.gravity);
  REPLACE_WITH_COPY(dest.air_control);
  REPLACE_WITH_COPY(dest.author);

  REPLACE_WITH_COPY(dest.next.map);
  REPLACE_WITH_COPY(dest.next.end_pic);
  REPLACE_WITH_COPY(dest.next.end_pic_b);
  REPLACE_WITH_COPY(dest.next.music);
  REPLACE_WITH_COPY(dest.secret_next.map);
  REPLACE_WITH_COPY(dest.secret_next.end_pic);
  REPLACE_WITH_COPY(dest.secret_next.end_pic_b);
  REPLACE_WITH_COPY(dest.secret_next.music);

  REPLACE_WITH_COPY(dest.sky1.lump);

  dest.num_special_actions = 0;
  dest.special_actions = NULL;
}

static void dsda_ParseDoomMapInfoMapNext(Scanner &scanner, doom_mapinfo_map_next_t &next) {
  scanner.MustGetToken('=');

  RESET_STR(next.map);
  RESET_STR(next.end_pic);
  RESET_STR(next.end_pic_b);
  RESET_STR(next.music);
  next.loop_music = true;
  next.end = dmi_end_null;

  scanner.MustGetString();

  if (!stricmp(scanner.string, "EndGame1")) {
    next.end_pic = Z_Strdup(gamemode == retail ? "CREDIT" : "HELP2");
  }
  else if (!stricmp(scanner.string, "EndGame2")) {
    next.end_pic = Z_Strdup("VICTORY2");
  }
  else if (!stricmp(scanner.string, "EndGame4")) {
    next.end_pic = Z_Strdup("ENDPIC");
  }
  else if (!stricmp(scanner.string, "EndGame3")) {
    next.end = dmi_end_game_scroll;
  }
  else if (!stricmp(scanner.string, "EndGameC")) {
    next.end = dmi_end_game_cast;
  }
  else if (!stricmp(scanner.string, "EndPic")) {
      scanner.MustGetToken(',');
      scanner.MustGetString();
      STR_DUP(next.end_pic);
  }
  else if (!stricmp(scanner.string, "endgame")) {
    scanner.MustGetToken('{');
    while (!scanner.CheckToken('}')) {
      scanner.MustGetToken(TK_Identifier);

      if (!stricmp(scanner.string, "pic")) {
        SCAN_STRING(next.end_pic);
      }
      else if (!stricmp(scanner.string, "hscroll")) {
        next.end = dmi_end_game_scroll;
        scanner.MustGetToken('=');
        scanner.MustGetString();
        STR_DUP(next.end_pic);
        scanner.MustGetToken(',');
        scanner.MustGetString();
        STR_DUP(next.end_pic_b);
      }
      else if (!stricmp(scanner.string, "cast")) {
        next.end = dmi_end_game_cast;
      }
      else if (!stricmp(scanner.string, "music")) {
        SCAN_STRING(next.music);
        if (scanner.CheckToken(',')) {
          scanner.MustGetInteger();
          next.loop_music = !!scanner.number;
        }
      }
      else {
        dsda_SkipValue(scanner);
      }
    }
  }
  else {
    STR_DUP(next.map);
  }
}

static void dsda_ParseDoomMapInfoMapSky(Scanner &scanner, doom_mapinfo_sky_t &sky) {
  scanner.MustGetToken('=');
  scanner.MustGetString();

  STR_DUP(sky.lump);
}

static void dsda_ParseDoomMapInfoTitlePatch(Scanner &scanner, doom_mapinfo_map_t &map) {
  scanner.MustGetToken('=');
  scanner.MustGetString();

  STR_DUP(map.title_patch);
  if (scanner.CheckToken(',')) {
    scanner.MustGetInteger();
    if (scanner.number)
      map.flags &= ~DMI_SHOW_AUTHOR;
  }
}

static void dsda_ParseDoomMapInfoMusic(Scanner &scanner, doom_mapinfo_map_t &map) {
  scanner.MustGetToken('=');
  scanner.MustGetString();

  if (strchr(scanner.string, ':'))
    return;

  if (scanner.CheckToken(',')) {
    scanner.MustGetInteger();
    return;
  }

  STR_DUP(map.music);
}

static void dsda_ParseDoomMapInfoIntermissionPic(Scanner &scanner, char* &pic) {
  scanner.MustGetToken('=');
  scanner.MustGetString();

  if (scanner.string[0] == '$')
    return;

  STR_DUP(pic);
}

static void dsda_ParseDoomMapInfoMapSpecialAction(Scanner &scanner,
                                               std::vector<doom_mapinfo_special_action_t> &special_actions) {
  doom_mapinfo_special_action_t special_action = { 0 };

  scanner.MustGetToken('=');

  scanner.MustGetString();
  special_action.monster_type = dsda_ActorNameToType(scanner.string);

  scanner.MustGetToken(',');

  scanner.MustGetString();
  special_action.action_special = dsda_ActionNameToNumber(scanner.string);

  for (int i = 0; i < 5; ++i) {
    if (!scanner.CheckToken(','))
      break;

    scanner.MustGetInteger();
    special_action.special_args[i] = scanner.number;
  }

  if (special_action.monster_type == NAME_NOT_FOUND ||
      special_action.action_special == NAME_NOT_FOUND)
    return;

  special_actions.push_back(special_action);
}

static void dsda_GuessLevelNum(doom_mapinfo_map_t &map) {
  int level, episode;

  dsda_UppercaseString(map.lump_name);

  if (sscanf(map.lump_name, "MAP%d", &level) == 1)
    map.level_num = level;
  else if (sscanf(map.lump_name, "E%dM%d", &episode, &level) == 2)
    map.level_num = (episode - 1) * 10 + level;
  else
    map.level_num = -1;
}

static void dsda_ParseDoomMapInfoMapBlock(Scanner &scanner, doom_mapinfo_map_t &map,
                                       std::vector<doom_mapinfo_special_action_t> &special_actions) {
  scanner.MustGetToken('{');
  while (!scanner.CheckToken('}')) {
    scanner.MustGetToken(TK_Identifier);

    if (!stricmp(scanner.string, "LevelNum")) {
      SCAN_INT(map.level_num);
    }
    else if (!stricmp(scanner.string, "Next")) {
      dsda_ParseDoomMapInfoMapNext(scanner, map.next);
    }
    else if (!stricmp(scanner.string, "SecretNext")) {
      dsda_ParseDoomMapInfoMapNext(scanner, map.secret_next);
    }
    else if (!stricmp(scanner.string, "Cluster")) {
      SCAN_INT(map.cluster);
    }
    else if (!stricmp(scanner.string, "Sky1")) {
      dsda_ParseDoomMapInfoMapSky(scanner, map.sky1);
    }
    else if (!stricmp(scanner.string, "TitlePatch")) {
      dsda_ParseDoomMapInfoTitlePatch(scanner, map);
    }
    else if (!stricmp(scanner.string, "Par")) {
      SCAN_INT(map.par);
    }
    else if (!stricmp(scanner.string, "NoIntermission")) {
      map.flags &= ~DMI_INTERMISSION;
    }
    else if (!stricmp(scanner.string, "Intermission")) {
      map.flags |= DMI_INTERMISSION;
    }
    else if (!stricmp(scanner.string, "Music")) {
      dsda_ParseDoomMapInfoMusic(scanner, map);
    }
    else if (!stricmp(scanner.string, "ExitPic")) {
      dsda_ParseDoomMapInfoIntermissionPic(scanner, map.exit_pic);
    }
    else if (!stricmp(scanner.string, "EnterPic")) {
      dsda_ParseDoomMapInfoIntermissionPic(scanner, map.enter_pic);
    }
    else if (!stricmp(scanner.string, "InterMusic")) {
      SCAN_STRING(map.inter_music);
    }
    else if (!stricmp(scanner.string, "BorderTexture")) {
      SCAN_STRING(map.border_texture);
    }
    else if (!stricmp(scanner.string, "EvenLighting")) {
      map.flags |= DMI_EVEN_LIGHTING;
      map.flags &= ~DMI_SMOOTH_LIGHTING;
    }
    else if (!stricmp(scanner.string, "SmoothLighting")) {
      map.flags |= DMI_SMOOTH_LIGHTING;
      map.flags &= ~DMI_EVEN_LIGHTING;
    }
    else if (!stricmp(scanner.string, "Gravity")) {
      SCAN_FLOAT_STRING(map.gravity);
    }
    else if (!stricmp(scanner.string, "AirControl")) {
      SCAN_FLOAT_STRING(map.air_control);
    }
    else if (!stricmp(scanner.string, "AllowMonsterTelefrags")) {
      map.flags |= DMI_ALLOW_MONSTER_TELEFRAGS;
    }
    else if (!stricmp(scanner.string, "KillerActivatesDeathSpecials")) {
      map.flags &= ~DMI_ACTIVATE_OWN_DEATH_SPECIALS;
    }
    else if (!stricmp(scanner.string, "ActivateOwnDeathSpecials")) {
      map.flags |= DMI_ACTIVATE_OWN_DEATH_SPECIALS;
    }
    else if (!stricmp(scanner.string, "SpecialAction")) {
      dsda_ParseDoomMapInfoMapSpecialAction(scanner, special_actions);
    }
    else if (!stricmp(scanner.string, "StrictMonsterActivation")) {
      map.flags &= ~DMI_LAX_MONSTER_ACTIVATION;
    }
    else if (!stricmp(scanner.string, "LaxMonsterActivation")) {
      map.flags |= DMI_LAX_MONSTER_ACTIVATION;
    }
    else if (!stricmp(scanner.string, "MissileShootersActivateImpactLines")) {
      map.flags &= ~DMI_MISSILES_ACTIVATE_IMPACT_LINES;
    }
    else if (!stricmp(scanner.string, "MissilesActivateImpactLines")) {
      map.flags |= DMI_MISSILES_ACTIVATE_IMPACT_LINES;
    }
    else if (!stricmp(scanner.string, "FilterStarts")) {
      map.flags |= DMI_FILTER_STARTS;
    }
    else if (!stricmp(scanner.string, "AllowRespawn")) {
      map.flags |= DMI_ALLOW_RESPAWN;
    }
    else if (!stricmp(scanner.string, "NoJump")) {
      map.flags &= ~DMI_ALLOW_JUMP;
    }
    else if (!stricmp(scanner.string, "AllowJump")) {
      map.flags |= DMI_ALLOW_JUMP;
    }
    else if (!stricmp(scanner.string, "NoCheckSwitchRange")) {
      map.flags &= ~DMI_CHECK_SWITCH_RANGE;
    }
    else if (!stricmp(scanner.string, "CheckSwitchRange")) {
      map.flags |= DMI_CHECK_SWITCH_RANGE;
    }
    else if (!stricmp(scanner.string, "ResetHealth")) {
      map.flags |= DMI_RESET_HEALTH;
    }
    else if (!stricmp(scanner.string, "ResetInventory")) {
      map.flags |= DMI_RESET_INVENTORY;
    }
    else if (!stricmp(scanner.string, "NoPassover")) {
      map.flags &= ~DMI_PASSOVER;
    }
    else if (!stricmp(scanner.string, "Passover")) {
      map.flags |= DMI_PASSOVER;
    }
    else if (!stricmp(scanner.string, "UsePlayerStartZ")) {
      map.flags |= DMI_USE_PLAYER_START_Z;
    }
    else if (!stricmp(scanner.string, "Author")) {
      SCAN_STRING(map.author);
    }
    else {
      dsda_SkipValue(scanner);
    }
  }

  map.num_special_actions = special_actions.size();
  map.special_actions = (doom_mapinfo_special_action_t *) Z_Realloc(
    map.special_actions, map.num_special_actions * sizeof(*map.special_actions)
  );
  memcpy(map.special_actions, &special_actions[0],
         map.num_special_actions * sizeof(*map.special_actions));
}

static void dsda_ParseDoomMapInfoMap(Scanner &scanner) {
  doom_mapinfo_map_t map;
  std::vector<doom_mapinfo_special_action_t> special_actions;

  // Create a copy of the existing special actions from the default map
  for (size_t i = 0; i < default_map.num_special_actions; ++i)
    special_actions.push_back(default_map.special_actions[i]);

  dsda_CopyMap(map, default_map);

  scanner.MustGetString();
  STR_DUP(map.lump_name);

  scanner.MustGetString();
  STR_DUP(map.nice_name);

  dsda_GuessLevelNum(map);

  dsda_ParseDoomMapInfoMapBlock(scanner, map, special_actions);

  for (auto &old_map : doom_mapinfo_maps)
    if (old_map.level_num == map.level_num)
      old_map.level_num = 0;

  for (auto &old_map : doom_mapinfo_maps)
    if (!stricmp(old_map.lump_name, map.lump_name)) {
      dsda_ResetMap(old_map);
      old_map = map;

      return;
    }

  doom_mapinfo_maps.push_back(map);
}

static void dsda_InitDefaultMap(void) {
  dsda_ResetMap(default_map);

  default_map.gravity = Z_Strdup("800");
  default_map.flags = DMI_INTERMISSION |
                      DMI_ACTIVATE_OWN_DEATH_SPECIALS |
                      DMI_LAX_MONSTER_ACTIVATION |
                      DMI_MISSILES_ACTIVATE_IMPACT_LINES |
                      DMI_SHOW_AUTHOR;
}

static void dsda_ParseDoomMapInfoDefaultMap(Scanner &scanner) {
  std::vector<doom_mapinfo_special_action_t> special_actions;

  dsda_InitDefaultMap();

  dsda_ParseDoomMapInfoMapBlock(scanner, default_map, special_actions);
}

static void dsda_ParseDoomMapInfoAddDefaultMap(Scanner &scanner) {
  std::vector<doom_mapinfo_special_action_t> special_actions;

  // Create a copy of the existing special actions from the default map
  for (size_t i = 0; i < default_map.num_special_actions; ++i)
    special_actions.push_back(default_map.special_actions[i]);

  Z_Free(default_map.special_actions);
  default_map.special_actions = NULL;
  default_map.num_special_actions = 0;

  dsda_ParseDoomMapInfoMapBlock(scanner, default_map, special_actions);
}

static void dsda_FreeSkill(doom_mapinfo_skill_t &skill) {
  Z_Free(skill.unique_id);
  Z_Free(skill.ammo_factor);
  Z_Free(skill.damage_factor);
  Z_Free(skill.armor_factor);
  Z_Free(skill.health_factor);
  Z_Free(skill.monster_health_factor);
  Z_Free(skill.friend_health_factor);
  Z_Free(skill.must_confirm);
  Z_Free(skill.name);
  Z_Free(skill.pic_name);
  Z_Free(skill.text_color);
}

static void dsda_ParseDoomMapInfoSkill(Scanner &scanner) {
  doom_mapinfo_skill_t skill = { 0 };

  scanner.MustGetString();
  STR_DUP(skill.unique_id);

  scanner.MustGetToken('{');
  while (!scanner.CheckToken('}')) {
    scanner.MustGetToken(TK_Identifier);

    if (!stricmp(scanner.string, "AmmoFactor")) {
      SCAN_FLOAT_STRING(skill.ammo_factor);
    }
    else if (!stricmp(scanner.string, "DamageFactor")) {
      SCAN_FLOAT_STRING(skill.damage_factor);
    }
    else if (!stricmp(scanner.string, "ArmorFactor")) {
      SCAN_FLOAT_STRING(skill.armor_factor);
    }
    else if (!stricmp(scanner.string, "HealthFactor")) {
      SCAN_FLOAT_STRING(skill.health_factor);
    }
    else if (!stricmp(scanner.string, "MonsterHealth")) {
      SCAN_FLOAT_STRING(skill.monster_health_factor);
    }
    else if (!stricmp(scanner.string, "FriendlyHealth")) {
      SCAN_FLOAT_STRING(skill.friend_health_factor);
    }
    else if (!stricmp(scanner.string, "RespawnTime")) {
      SCAN_INT(skill.respawn_time);
    }
    else if (!stricmp(scanner.string, "SpawnFilter")) {
      SCAN_INT(skill.spawn_filter);
    }
    else if (!stricmp(scanner.string, "Key")) {
      scanner.MustGetToken('=');
      scanner.MustGetString();
      skill.key = tolower(scanner.string[0]);
    }
    else if (!stricmp(scanner.string, "MustConfirm")) {
      if (scanner.CheckToken('=')) {
        scanner.MustGetString();
        STR_DUP(skill.must_confirm);
      }

      skill.flags |= DSI_MUST_CONFIRM;
    }
    else if (!stricmp(scanner.string, "Name")) {
      SCAN_STRING(skill.name);
    }
    else if (!stricmp(scanner.string, "PicName")) {
      SCAN_STRING(skill.pic_name);
    }
    else if (!stricmp(scanner.string, "TextColor")) {
      SCAN_STRING(skill.text_color);
    }
    else if (!stricmp(scanner.string, "SpawnMulti")) {
      skill.flags |= DSI_SPAWN_MULTI;
    }
    else if (!stricmp(scanner.string, "FastMonsters")) {
      skill.flags |= DSI_FAST_MONSTERS;
    }
    else if (!stricmp(scanner.string, "InstantReaction")) {
      skill.flags |= DSI_INSTANT_REACTION;
    }
    else if (!stricmp(scanner.string, "NoPain")) {
      skill.flags |= DSI_NO_PAIN;
    }
    else if (!stricmp(scanner.string, "DefaultSkill")) {
      skill.flags |= DSI_DEFAULT_SKILL;
    }
    else if (!stricmp(scanner.string, "PlayerRespawn")) {
      skill.flags |= DSI_PLAYER_RESPAWN;
    }
    else if (!stricmp(scanner.string, "EasyBossBrain")) {
      skill.flags |= DSI_EASY_BOSS_BRAIN;
    }
    else {
      dsda_SkipValue(scanner);
    }
  }

  for (auto &old_skill : doom_mapinfo_skills)
    if (!stricmp(old_skill.unique_id, skill.unique_id)) {
      dsda_FreeSkill(old_skill);
      old_skill = skill;

      return;
    }

  doom_mapinfo_skills.push_back(skill);
}

static void dsda_FreeCluster(doom_mapinfo_cluster_t &cluster) {
  Z_Free(cluster.enter_text);
  Z_Free(cluster.exit_text);
  Z_Free(cluster.music);
  Z_Free(cluster.flat);
  Z_Free(cluster.pic);
}

static void dsda_ParseDoomMapInfoCluster(Scanner &scanner) {
  doom_mapinfo_cluster_t cluster = { 0 };

  scanner.MustGetInteger();
  cluster.id = scanner.number;

  scanner.MustGetToken('{');
  while (!scanner.CheckToken('}')) {
    scanner.MustGetToken(TK_Identifier);

    if (!stricmp(scanner.string, "EnterText")) {
      dsda_ParseDoomMapInfoMultiString(scanner, cluster.enter_text);
    }
    else if (!stricmp(scanner.string, "ExitText")) {
      dsda_ParseDoomMapInfoMultiString(scanner, cluster.exit_text);
    }
    else if (!stricmp(scanner.string, "Music")) {
      SCAN_STRING(cluster.music);
    }
    else if (!stricmp(scanner.string, "Flat")) {
      SCAN_STRING(cluster.flat);
    }
    else if (!stricmp(scanner.string, "Pic")) {
      SCAN_STRING(cluster.pic);
    }
    else {
      dsda_SkipValue(scanner);
    }
  }

  for (auto &old_cluster : doom_mapinfo_clusters)
    if (old_cluster.id == cluster.id) {
      dsda_FreeCluster(old_cluster);
      old_cluster = cluster;
      return;
    }

  doom_mapinfo_clusters.push_back(cluster);
}

static void dsda_FreeEpisode(doom_mapinfo_episode_t &episode) {
  Z_Free(episode.map_lump);
  Z_Free(episode.name);
  Z_Free(episode.pic_name);
}

static void dsda_ParseDoomMapInfoEpisode(Scanner &scanner) {
  doom_mapinfo_episode_t episode = { 0 };

  scanner.MustGetString();
  STR_DUP(episode.map_lump);

  if (episode.map_lump[0] == '&')
    scanner.ErrorF("Invalid episode map lump %s", episode.map_lump);

  scanner.MustGetToken('{');
  while (!scanner.CheckToken('}')) {
    scanner.MustGetToken(TK_Identifier);

    if (!stricmp(scanner.string, "Name")) {
      SCAN_STRING(episode.name);
    }
    else if (!stricmp(scanner.string, "PicName")) {
      SCAN_STRING(episode.pic_name);
    }
    else if (!stricmp(scanner.string, "Key")) {
      scanner.MustGetToken('=');
      scanner.MustGetString();
      episode.key = tolower(scanner.string[0]);
    }
    else {
      dsda_SkipValue(scanner);
    }
  }

  doom_mapinfo_episodes.push_back(episode);
}

static void dsda_ParseDoomMapInfoIdentifier(Scanner &scanner) {
  scanner.MustGetToken(TK_Identifier);

  if (!stricmp(scanner.string, "map")) {
    dsda_ParseDoomMapInfoMap(scanner);
  }
  else if (!stricmp(scanner.string, "defaultmap")) {
    dsda_ParseDoomMapInfoDefaultMap(scanner);
  }
  else if (!stricmp(scanner.string, "adddefaultmap")) {
    dsda_ParseDoomMapInfoAddDefaultMap(scanner);
  }
  else if (!stricmp(scanner.string, "cluster")) {
    dsda_ParseDoomMapInfoCluster(scanner);
  }
  else if (!stricmp(scanner.string, "episode")) {
    dsda_ParseDoomMapInfoEpisode(scanner);
  }
  else if (!stricmp(scanner.string, "clearepisodes")) {
    for (auto &episode : doom_mapinfo_episodes)
      dsda_FreeEpisode(episode);
    doom_mapinfo_episodes.clear();
    doom_mapinfo.episodes_cleared = true;
  }
  else if (!stricmp(scanner.string, "skill")) {
    dsda_ParseDoomMapInfoSkill(scanner);
  }
  else if (!stricmp(scanner.string, "clearskills")) {
    for (auto &skill : doom_mapinfo_skills)
      dsda_FreeSkill(skill);
    doom_mapinfo_skills.clear();
    doom_mapinfo.skills_cleared = true;
  }
  else {
    dsda_SkipValue(scanner);
  }
}

doom_mapinfo_t doom_mapinfo;

void dsda_ParseDoomMapInfo(const unsigned char* buffer, size_t length, doom_mapinfo_errorfunc err) {
  Scanner scanner((const char*) buffer, length);

  scanner.SetErrorCallback(err);

  DO_ONCE
    dsda_InitDefaultMap();
  END_ONCE

  while (scanner.TokensLeft())
    dsda_ParseDoomMapInfoIdentifier(scanner);

  doom_mapinfo.maps = &doom_mapinfo_maps[0];
  doom_mapinfo.num_maps = doom_mapinfo_maps.size();

  doom_mapinfo.skills = &doom_mapinfo_skills[0];
  doom_mapinfo.num_skills = doom_mapinfo_skills.size();

  doom_mapinfo.clusters = &doom_mapinfo_clusters[0];
  doom_mapinfo.num_clusters = doom_mapinfo_clusters.size();

  doom_mapinfo.episodes = &doom_mapinfo_episodes[0];
  doom_mapinfo.num_episodes = doom_mapinfo_episodes.size();

  doom_mapinfo.loaded = true;
}
