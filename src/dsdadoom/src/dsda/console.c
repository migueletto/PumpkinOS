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
//	DSDA Console
//

#include "d_deh.h"
#include "doomstat.h"
#include "g_game.h"
#include "hu_lib.h"
#include "hu_stuff.h"
#include "i_main.h"
#include "i_system.h"
#include "lprintf.h"
#include "m_cheat.h"
#include "m_file.h"
#include "m_menu.h"
#include "m_misc.h"
#include "p_inter.h"
#include "p_map.h"
#include "p_maputl.h"
#include "p_mobj.h"
#include "p_setup.h"
#include "p_spec.h"
#include "p_tick.h"
#include "p_user.h"
#include "s_sound.h"
#include "smooth.h"
#include "v_video.h"
#include "stricmp.h"

#include "dsda.h"
#include "dsda/build.h"
#include "dsda/brute_force.h"
#include "dsda/configuration.h"
#include "dsda/demo.h"
#include "dsda/exhud.h"
#include "dsda/features.h"
#include "dsda/font.h"
#include "dsda/global.h"
#include "dsda/map_format.h"
#include "dsda/messenger.h"
#include "dsda/mobjinfo.h"
#include "dsda/playback.h"
#include "dsda/settings.h"
#include "dsda/stretch.h"
#include "dsda/tracker.h"
#include "dsda/utility.h"

#include "console.h"

#define target_player players[consoleplayer]

#define CONSOLE_TEXT_FLAGS (VPT_ALIGN_TOP | VPT_EX_TEXT)
#define CONSOLE_ENTRY_SIZE 64

#define CF_NEVER  0x00
#define CF_DEMO   0x01
#define CF_STRICT 0x02
#define CF_ALWAYS (CF_DEMO|CF_STRICT)

typedef struct console_entry_s {
  char text[CONSOLE_ENTRY_SIZE + 1];
  struct console_entry_s* prev;
  struct console_entry_s* next;
} console_entry_t;

static console_entry_t* console_history_head;
static console_entry_t* console_entry;
static int console_entry_index;
static char console_message[CONSOLE_ENTRY_SIZE + 3] = { ' ', ' ' };
static char* console_message_entry = console_message + 2;
static hu_textline_t hu_console_prompt;
static hu_textline_t hu_console_message;

static char** dsda_console_script_lines[CONSOLE_SCRIPT_COUNT];

static int console_height;

int dsda_ConsoleHeight(void) {
  return console_height;
}

static void dsda_DrawConsole(void) {
  console_height = V_FillHeightVPT(0, 0, 16, 0, CONSOLE_TEXT_FLAGS);
  HUlib_drawTextLine(&hu_console_prompt, false);
  HUlib_drawTextLine(&hu_console_message, false);
}

menu_t dsda_ConsoleDef = {
  0,
  NULL,
  NULL,
  dsda_DrawConsole,
  0, 0,
  0, MENUF_TEXTINPUT
};

static dboolean dsda_ExecuteConsole(const char* command_line, dboolean noise);

static void dsda_UpdateConsoleDisplay(void) {
  const char* s;
  int i;

  s = console_entry->text;
  HUlib_clearTextLine(&hu_console_prompt);
  HUlib_addCharToTextLine(&hu_console_prompt, '$');
  HUlib_addCharToTextLine(&hu_console_prompt, ' ');
  for (i = 0; *s && i < console_entry_index; ++i)
    HUlib_addCharToTextLine(&hu_console_prompt, *(s++));
  HUlib_addCharToTextLine(&hu_console_prompt, '_');
  while (*s) HUlib_addCharToTextLine(&hu_console_prompt, *(s++));

  s = console_message;
  HUlib_clearTextLine(&hu_console_message);
  while (*s) HUlib_addCharToTextLine(&hu_console_message, *(s++));
}

static void dsda_ResetConsoleEntry(void) {
  console_entry_index = strlen(console_entry->text);
  dsda_UpdateConsoleDisplay();
}

dboolean dsda_OpenConsole(void) {
  static dboolean firsttime = true;

  if (gamestate != GS_LEVEL)
    return false;

  if (firsttime) {
    firsttime = false;

    HUlib_initTextLine(
      &hu_console_prompt,
      0,
      8,
      &exhud_font,
      CR_GRAY,
      CONSOLE_TEXT_FLAGS
    );

    HUlib_initTextLine(
      &hu_console_message,
      0,
      0,
      &exhud_font,
      CR_GRAY,
      CONSOLE_TEXT_FLAGS
    );

    console_history_head = Z_Calloc(sizeof(console_entry_t), 1);
    console_entry = console_history_head;
  }

  dsda_TrackFeature(uf_console);

  M_StartControlPanel();
  M_SetupNextMenu(&dsda_ConsoleDef);
  dsda_ResetConsoleEntry();

  return true;
}

static dboolean console_LevelExit(const char* command, const char* args) {
  void G_ExitLevel(int position);

  int position = 0;

  if (hexen)
    return false;

  sscanf(args, "%d", &position);

  G_ExitLevel(position);

  return true;
}

static dboolean console_LevelSecretExit(const char* command, const char* args) {
  void G_SecretExitLevel(int position);

  int position = 0;

  if (hexen)
    return false;

  sscanf(args, "%d", &position);

  G_SecretExitLevel(position);

  return true;
}

static dboolean console_ActivateLine(mobj_t* mobj, int id, dboolean bossaction) {
  if (!mobj || id < 0 || id >= numlines)
    return false;

  P_MapStart();
  P_UseSpecialLine(mobj, &lines[id], 0, bossaction);
  map_format.cross_special_line(&lines[id], 0, mobj, bossaction);
  map_format.shoot_special_line(mobj, &lines[id]);
  P_MapEnd();

  return true;
}

static dboolean console_PlayerActivateLine(const char* command, const char* args) {
  int id;

  if (sscanf(args, "%i", &id) != 1)
    return false;

  return console_ActivateLine(target_player.mo, id, false);
}

static dboolean console_PlayerSetHealth(const char* command, const char* args) {
  int health;

  if (sscanf(args, "%i", &health)) {
    target_player.mo->health = health;
    target_player.health = health;

    return true;
  }

  return false;
}

static dboolean console_PlayerSetArmor(const char* command, const char* args) {
  int arg_count;
  int armorpoints, armortype;

  arg_count = sscanf(args, "%i %i", &armorpoints, &armortype);

  if (arg_count != 2 || (armortype != 1 && armortype != 2))
    armortype = target_player.armortype;

  if (arg_count) {
    target_player.armorpoints[ARMOR_ARMOR] = armorpoints;

    if (armortype == 0) armortype = 1;
    target_player.armortype = armortype;

    return true;
  }

  return false;
}

static dboolean console_PlayerGiveWeapon(const char* command, const char* args) {
  dboolean P_GiveWeapon(player_t *player, weapontype_t weapon, dboolean dropped);
  void TryPickupWeapon(player_t * player, pclass_t weaponClass,
                       weapontype_t weaponType, mobj_t * weapon,
                       const char *message);

  int weapon;

  if (sscanf(args, "%i", &weapon)) {
    if (hexen) {
      mobj_t mo;

      if (weapon < 0 || weapon >= HEXEN_NUMWEAPONS)
        return false;

      memset(&mo, 0, sizeof(mo));

      mo.intflags |= MIF_FAKE;

      TryPickupWeapon(&target_player, target_player.pclass, weapon, &mo, "WEAPON");
    }
    else {
      if (weapon < 0 || weapon >= NUMWEAPONS)
        return false;

      P_GiveWeapon(&target_player, weapon, false);
    }

    return true;
  }

  return false;
}

static dboolean console_PlayerGiveAmmo(const char* command, const char* args) {
  int ammo;
  int amount;
  int arg_count;

  arg_count = sscanf(args, "%i %i", &ammo, &amount);

  if (arg_count == 2) {
    if (ammo < 0 || ammo >= g_numammo || amount <= 0)
      return false;

    if (hexen) {
      target_player.ammo[ammo] += amount;
      if (target_player.ammo[ammo] > MAX_MANA)
        target_player.ammo[ammo] = MAX_MANA;
    }
    else {
      target_player.ammo[ammo] += amount;
      if (target_player.ammo[ammo] > target_player.maxammo[ammo])
        target_player.ammo[ammo] = target_player.maxammo[ammo];
    }

    return true;
  }
  else if (arg_count == 1) {
    if (ammo < 0 || ammo >= g_numammo)
      return false;

    if (hexen)
      target_player.ammo[ammo] = MAX_MANA;
    else
      target_player.ammo[ammo] = target_player.maxammo[ammo];

    return true;
  }

  return false;
}

static dboolean console_PlayerSetAmmo(const char* command, const char* args) {
  int ammo;
  int amount;

  if (sscanf(args, "%i %i", &ammo, &amount) == 2) {
    if (ammo < 0 || ammo >= g_numammo || amount < 0)
      return false;

    if (hexen) {
      target_player.ammo[ammo] = amount;
      if (target_player.ammo[ammo] > MAX_MANA)
        target_player.ammo[ammo] = MAX_MANA;
    }
    else {
      target_player.ammo[ammo] = amount;
      if (target_player.ammo[ammo] > target_player.maxammo[ammo])
        target_player.ammo[ammo] = target_player.maxammo[ammo];
    }

    return true;
  }

  return false;
}

static dboolean console_PlayerGiveKey(const char* command, const char* args) {
  extern int playerkeys;

  int key;

  if (sscanf(args, "%i", &key)) {
    if (key < 0 || key >= NUMCARDS)
      return false;

    target_player.cards[key] = true;
    playerkeys |= 1 << key;

    return true;
  }

  return false;
}

static dboolean console_PlayerRemoveKey(const char* command, const char* args) {
  extern int playerkeys;

  int key;

  if (sscanf(args, "%i", &key)) {
    if (key < 0 || key >= NUMCARDS)
      return false;

    target_player.cards[key] = false;
    playerkeys &= ~(1 << key);

    return true;
  }

  return false;
}

static dboolean console_PlayerGivePower(const char* command, const char* args) {
  dboolean P_GivePower(player_t *player, int power);
  void SB_Start(void);

  int power;
  int duration = -1;

  if (sscanf(args, "%i %i", &power, &duration)) {
    if (power < 0 || power >= NUMPOWERS ||
        power == pw_shield || power == pw_health2 || power == pw_minotaur)
      return false;

    target_player.powers[power] = 0;
    P_GivePower(&target_player, power);
    if (power != pw_strength)
      target_player.powers[power] = duration;

    if (raven) SB_Start();

    return true;
  }

  return false;
}

static dboolean console_PlayerRemovePower(const char* command, const char* args) {
  void SB_Start(void);

  int power;

  if (sscanf(args, "%i", &power)) {
    if (power < 0 || power >= NUMPOWERS ||
        power == pw_shield || power == pw_health2 || power == pw_minotaur)
      return false;

    target_player.powers[power] = 0;

    if (power == pw_invulnerability) {
      target_player.mo->flags2 &= ~(MF2_INVULNERABLE | MF2_REFLECTIVE);
      if (target_player.pclass == PCLASS_CLERIC)
      {
        target_player.mo->flags2 &= ~(MF2_DONTDRAW | MF2_NONSHOOTABLE);
        target_player.mo->flags &= ~(MF_SHADOW | MF_ALTSHADOW);
      }
    }
    else if (power == pw_invisibility)
      target_player.mo->flags &= ~MF_SHADOW;
    else if (power == pw_flight) {
      P_PlayerEndFlight(&target_player);
    }
    else if (power == pw_weaponlevel2 && heretic) {
      if ((target_player.readyweapon == wp_phoenixrod)
          && (target_player.psprites[ps_weapon].state
              != &states[HERETIC_S_PHOENIXREADY])
          && (target_player.psprites[ps_weapon].state
              != &states[HERETIC_S_PHOENIXUP]))
      {
        P_SetPsprite(&target_player, ps_weapon, HERETIC_S_PHOENIXREADY);
        target_player.ammo[am_phoenixrod] -= USE_PHRD_AMMO_2;
        target_player.refire = 0;
      }
      else if ((target_player.readyweapon == wp_gauntlets)
               || (target_player.readyweapon == wp_staff))
      {
        target_player.pendingweapon = target_player.readyweapon;
      }
    }

    if (raven) SB_Start();

    return true;
  }

  return false;
}

static dboolean console_PlayerSetCoordinate(const char* args, int* dest) {
  int x, x_frac = 0;

  if (sscanf(args, "%i.%i", &x, &x_frac)) {
    *dest = FRACUNIT * x;

    if (args[0] == '-')
      *dest -= x_frac;
    else
      *dest += x_frac;

    return true;
  }

  return false;
}

static dboolean console_PlayerSetX(const char* command, const char* args) {
  return console_PlayerSetCoordinate(args, &target_player.mo->x);
}

static dboolean console_PlayerSetY(const char* command, const char* args) {
  return console_PlayerSetCoordinate(args, &target_player.mo->y);
}

static dboolean console_PlayerSetZ(const char* command, const char* args) {
  return console_PlayerSetCoordinate(args, &target_player.mo->z);
}

static void console_PlayerRoundCoordinate(int* x) {
  int bits = *x & 0xffff;
  if (!bits) return;

  if (*x > 0) {
    if (bits >= 0x8000)
      *x = (*x & ~0xffff) + FRACUNIT;
    else
      *x = *x & ~0xffff;
  }
  else {
    if (bits < 0x8000)
      *x = (*x & ~0xffff) - FRACUNIT;
    else
      *x = *x & ~0xffff;
  }
}

static dboolean console_PlayerRoundX(const char* command, const char* args) {
  console_PlayerRoundCoordinate(&target_player.mo->x);

  return true;
}

static dboolean console_PlayerRoundY(const char* command, const char* args) {
  console_PlayerRoundCoordinate(&target_player.mo->y);

  return true;
}

static dboolean console_PlayerRoundXY(const char* command, const char* args) {
  console_PlayerRoundCoordinate(&target_player.mo->x);
  console_PlayerRoundCoordinate(&target_player.mo->y);

  return true;
}

static dboolean console_PlayerSetAngle(const char* command, const char* args) {
  int a, a_frac = 0;

  if (sscanf(args, "%d.%d", &a, &a_frac)) {
    target_player.mo->angle = ((angle_t) a) << 24;

    if (args[0] == '-')
      target_player.mo->angle -= (a_frac << 16);
    else
      target_player.mo->angle += (a_frac << 16);

    R_SmoothPlaying_Reset(&target_player);

    return true;
  }

  return false;
}

static dboolean console_PlayerRoundAngle(const char* command, const char* args) {
  int remainder;

  remainder = (target_player.mo->angle >> 16) & 0xff;

  target_player.mo->angle &= 0xff000000;

  if (remainder >= 0x80)
    target_player.mo->angle += 0x01000000;

  R_SmoothPlaying_Reset(&target_player);

  return true;
}

static dboolean console_DemoExport(const char* command, const char* args) {
  char name[CONSOLE_ENTRY_SIZE];

  if (sscanf(args, "%s", name) == 1) {
    dsda_ExportDemo(name);
    return true;
  }

  return false;
}

static dboolean console_DemoStart(const char* command, const char* args) {
  char name[CONSOLE_ENTRY_SIZE];

  if (sscanf(args, "%s", name) == 1)
    return dsda_StartDemoSegment(name);

  return false;
}

static dboolean console_DemoStop(const char* command, const char* args) {
  if (!demorecording)
    return false;

  G_CheckDemoStatus();
  dsda_UpdateStrictMode();

  return true;
}

static dboolean console_DemoJoin(const char* command, const char* args) {
  if (!demoplayback)
    return false;

  dsda_JoinDemo(NULL);

  return true;
}

static dboolean console_GameQuit(const char* command, const char* args) {
  I_SafeExit(0);

  return true;
}

static dboolean console_GameDescribe(const char* command, const char* args) {
  extern dsda_string_t hud_title;

  dsda_string_t str;

  dsda_StringPrintF(&str, "%s\n"
                          "Skill %d\n"
                          "%s%s%s",
                          hud_title.string, gameskill + 1,
                          nomonsters ? "-nomo " : "",
                          respawnparm ? "-respawn " : "",
                          fastparm ? "-fast" : "");

  dsda_AddAlert(str.string);

  dsda_FreeString(&str);

  return true;
}

static dboolean console_TrackerAddLine(const char* command, const char* args) {
  int id;

  if (sscanf(args, "%i", &id))
    return dsda_TrackLine(id);

  return false;
}

static dboolean console_TrackerRemoveLine(const char* command, const char* args) {
  int id;

  if (sscanf(args, "%i", &id))
    return dsda_UntrackLine(id);

  return false;
}

static dboolean console_TrackerAddLineDistance(const char* command, const char* args) {
  int id;

  if (sscanf(args, "%i", &id))
    return dsda_TrackLineDistance(id);

  return false;
}

static dboolean console_TrackerRemoveLineDistance(const char* command, const char* args) {
  int id;

  if (sscanf(args, "%i", &id))
    return dsda_UntrackLineDistance(id);

  return false;
}

static dboolean console_TrackerAddSector(const char* command, const char* args) {
  int id;

  if (sscanf(args, "%i", &id))
    return dsda_TrackSector(id);

  return false;
}

static dboolean console_TrackerRemoveSector(const char* command, const char* args) {
  int id;

  if (sscanf(args, "%i", &id))
    return dsda_UntrackSector(id);

  return false;
}

static dboolean console_TrackerAddMobj(const char* command, const char* args) {
  int id;

  if (sscanf(args, "%i", &id))
    return dsda_TrackMobj(id);

  return false;
}

static dboolean console_TrackerRemoveMobj(const char* command, const char* args) {
  int id;

  if (sscanf(args, "%i", &id))
    return dsda_UntrackMobj(id);

  return false;
}

static dboolean console_TrackerAddPlayer(const char* command, const char* args) {
  return dsda_TrackPlayer(0);
}

static dboolean console_TrackerRemovePlayer(const char* command, const char* args) {
  return dsda_UntrackPlayer(0);
}

static dboolean console_TrackerReset(const char* command, const char* args) {
  dsda_WipeTrackers();

  return true;
}

static dboolean console_JumpToTic(const char* command, const char* args) {
  int tic;

  if (sscanf(args, "%i", &tic)) {
    if (tic < 0)
      return false;

    dsda_JumpToLogicTic(tic);

    return true;
  }

  return false;
}

static dboolean console_JumpByTic(const char* command, const char* args) {
  int tic;

  if (sscanf(args, "%i", &tic)) {
    tic = true_logictic + tic;

    dsda_JumpToLogicTic(tic);

    return true;
  }

  return false;
}

static dboolean console_BuildMF(const char* command, const char* args) {
  int x;

  return sscanf(args, "%i", &x) && dsda_BuildMF(x);
}

static dboolean console_BuildMB(const char* command, const char* args) {
  int x;

  return sscanf(args, "%i", &x) && dsda_BuildMB(x);
}

static dboolean console_BuildSR(const char* command, const char* args) {
  int x;

  return sscanf(args, "%i", &x) && dsda_BuildSR(x);
}

static dboolean console_BuildSL(const char* command, const char* args) {
  int x;

  return sscanf(args, "%i", &x) && dsda_BuildSL(x);
}

static dboolean console_BuildTR(const char* command, const char* args) {
  int x;

  return sscanf(args, "%i", &x) && dsda_BuildTR(x);
}

static dboolean console_BuildTL(const char* command, const char* args) {
  int x;

  return sscanf(args, "%i", &x) && dsda_BuildTL(x);
}

static dboolean console_BuildFU(const char* command, const char* args) {
  int x;

  return sscanf(args, "%i", &x) && dsda_BuildFU(x);
}

static dboolean console_BuildFD(const char* command, const char* args) {
  int x;

  return sscanf(args, "%i", &x) && dsda_BuildFD(x);
}

static dboolean console_BuildFC(const char* command, const char* args) {
  return dsda_BuildFC();
}

static dboolean console_BuildLU(const char* command, const char* args) {
  int x;

  return sscanf(args, "%i", &x) && dsda_BuildLU(x);
}

static dboolean console_BuildLD(const char* command, const char* args) {
  int x;

  return sscanf(args, "%i", &x) && dsda_BuildLD(x);
}

static dboolean console_BuildLC(const char* command, const char* args) {
  return dsda_BuildLC();
}

static dboolean console_BuildUA(const char* command, const char* args) {
  int x;

  return sscanf(args, "%i", &x) && dsda_BuildUA(x);
}

static dboolean console_BruteForceKeep(const char* command, const char* args) {
  int frame;

  if (!sscanf(args, "%d", &frame))
    return false;

  return dsda_KeepBruteForceFrame(frame);
}

static dboolean console_BruteForceNoMonsters(const char* command, const char* args) {
  dsda_BruteForceWithoutMonsters();

  return true;
}

static dboolean console_BruteForceMonsters(const char* command, const char* args) {
  dsda_BruteForceWithMonsters();

  return true;
}

static dboolean console_BruteForceFrame(const char* command, const char* args) {
  int frame;
  int forwardmove_min, forwardmove_max;
  int sidemove_min, sidemove_max;
  int angleturn_min, angleturn_max;
  byte buttons = 0;
  int weapon = 0;
  char button_str[4] = { 0 };
  int i, arg_count;

  arg_count = sscanf(
    args, "%i %i:%i %i:%i %i:%i %3s %d", &frame,
    &forwardmove_min, &forwardmove_max,
    &sidemove_min, &sidemove_max,
    &angleturn_min, &angleturn_max,
    button_str, &weapon
  );

  if (arg_count < 7)
    return false;

  if (arg_count > 7)
    for (i = 0; i < 3; ++i)
      switch (button_str[i]) {
        case 'a':
          buttons |= BT_ATTACK;
          break;
        case 'u':
          buttons |= BT_USE;
          break;
        case 'c':
          if (weapon > 0 && weapon < 16) {
            buttons |= BT_CHANGE;
            buttons |= (weapon << BT_WEAPONSHIFT);
          }
          else
            return false;
          break;
        case '\0':
          break;
        default:
          return false;
      }

  return dsda_AddBruteForceFrame(frame,
                                 forwardmove_min, forwardmove_max,
                                 sidemove_min, sidemove_max,
                                 angleturn_min, angleturn_max,
                                 buttons);
}

static dboolean console_BruteForceStart(const char* command, const char* args) {
  int depth;
  int forwardmove_min, forwardmove_max;
  int sidemove_min, sidemove_max;
  int angleturn_min, angleturn_max;
  char condition_args[CONSOLE_ENTRY_SIZE];
  int arg_count;

  dsda_ResetBruteForceConditions();

  arg_count = sscanf(
    args, "%i %i:%i %i:%i %i:%i %[^;]", &depth,
    &forwardmove_min, &forwardmove_max,
    &sidemove_min, &sidemove_max,
    &angleturn_min, &angleturn_max,
    condition_args
  );

  if (arg_count == 8) {
    int i;

    for (i = 0; i < depth; ++i)
      dsda_AddBruteForceFrame(i,
                              forwardmove_min, forwardmove_max,
                              sidemove_min, sidemove_max,
                              angleturn_min, angleturn_max,
                              0);
  }
  else {
    arg_count = sscanf(args, "%i %[^;]", &depth, condition_args);

    if (arg_count != 2)
      return false;
  }

  {
    int i;
    char** conditions;

    conditions = dsda_SplitString(condition_args, ",");

    if (!conditions)
      return false;

    for (i = 0; conditions[i]; ++i) {
      fixed_t value;
      char attr_s[4] = { 0 };
      char oper_s[5] = { 0 };

      if (sscanf(conditions[i], " skip %i", &value) == 1) {
        if (value >= numlines || value < 0)
          return false;

        dsda_AddMiscBruteForceCondition(dsda_bf_line_skip, value);
      }
      else if (sscanf(conditions[i], " act %i", &value) == 1) {
        if (value >= numlines || value < 0)
          return false;

        dsda_AddMiscBruteForceCondition(dsda_bf_line_activation, value);
      }
      else if (sscanf(conditions[i], " have %3[a-zA-Z]", attr_s) == 1) {
        int attr_i;

        for (attr_i = 0; attr_i < dsda_bf_item_max; ++attr_i)
          if (!strcmp(attr_s, dsda_bf_item_names[attr_i]))
            break;

        if (attr_i == dsda_bf_item_max)
          return false;

        dsda_AddMiscBruteForceCondition(dsda_bf_have_item, attr_i);
      }
      else if (sscanf(conditions[i], " %3[a-zA-Z] %4[a-zA-Z><!=] %i", attr_s, oper_s, &value) == 3) {
        int attr_i, oper_i;

        if (oper_s[0] == '=' && !oper_s[1])
          oper_s[1] = '=';

        for (attr_i = 0; attr_i < dsda_bf_attribute_max; ++attr_i)
          if (!strcmp(attr_s, dsda_bf_attribute_names[attr_i]))
            break;

        if (attr_i == dsda_bf_attribute_max)
          return false;

        for (oper_i = dsda_bf_limit_trio_zero; oper_i < dsda_bf_limit_trio_max; ++oper_i)
          if (!strcmp(oper_s, dsda_bf_limit_names[oper_i]))
            break;

        if (oper_i != dsda_bf_limit_trio_max) {
          dsda_SetBruteForceTarget(attr_i, oper_i, value, true);
          continue;
        }

        for (oper_i = 0; oper_i < dsda_bf_operator_max; ++oper_i)
          if (!strcmp(oper_s, dsda_bf_operator_names[oper_i]))
            break;

        if (oper_i == dsda_bf_operator_max)
          return false;

        dsda_AddBruteForceCondition(attr_i, oper_i, value);
      }
      else if (sscanf(conditions[i], " %3s %4s", attr_s, oper_s) == 2) {
        int attr_i, oper_i;

        for (attr_i = 0; attr_i < dsda_bf_attribute_max; ++attr_i)
          if (!strcmp(attr_s, dsda_bf_attribute_names[attr_i]))
            break;

        if (attr_i == dsda_bf_attribute_max)
          return false;

        for (oper_i = dsda_bf_limit_duo_zero; oper_i < dsda_bf_limit_duo_max; ++oper_i)
          if (!strcmp(oper_s, dsda_bf_limit_names[oper_i]))
            break;

        if (oper_i == dsda_bf_limit_duo_max)
          return false;

        dsda_SetBruteForceTarget(attr_i, oper_i, 0, false);
      }
      else {
        return false;
      }
    }

    Z_Free(conditions);
  }

  return dsda_StartBruteForce(depth);
}

static dboolean console_BuildTurbo(const char* command, const char* args) {
  dsda_ToggleBuildTurbo();

  return true;
}

static dboolean console_Exit(const char* command, const char* args) {
  extern void M_ClearMenus(void);

  M_ClearMenus();

  return true;
}

static dboolean console_BasicCheat(const char* command, const char* args) {
  return M_CheatEntered(command, args);
}

static dboolean console_IDDT(const char* command, const char* args) {
  M_CheatIDDT();

  return true;
}

static dboolean console_CheatFullClip(const char* command, const char* args) {
  target_player.cheats ^= CF_INFINITE_AMMO;
  return true;
}

static dboolean console_Freeze(const char* command, const char* args) {
  dsda_ToggleFrozenMode();
  return true;
}

static dboolean console_NoSleep(const char* command, const char* args) {
  int i;

  for (i = 0; i < numsectors; ++i)
    sectors[i].soundtarget = target_player.mo;

  return true;
}

static dboolean console_ScriptRunLine(const char* line) {
  if (strlen(line) && line[0] != '#' && line[0] != '!' && line[0] != '/') {
    if (strlen(line) >= CONSOLE_ENTRY_SIZE) {
      lprintf(LO_ERROR, "Script line too long: \"%s\" (limit %d)\n", line, CONSOLE_ENTRY_SIZE);
      return false;
    }

    if (!dsda_ExecuteConsole(line, false)) {
      lprintf(LO_ERROR, "Script line failed: \"%s\"\n", line);
      return false;
    }
  }

  return true;
}

static dboolean console_ScriptRun(const char* command, const char* args) {
  char name[CONSOLE_ENTRY_SIZE];
  dboolean ret = true;

  if (sscanf(args, "%s", name)) {
    char* filename;
    char* buffer;

    filename = I_FindFile(name, "");

    if (filename) {
      if (M_ReadFileToString(filename, &buffer) != -1) {
        char* line;

        for (line = strtok(buffer, "\n;"); line; line = strtok(NULL, "\n;"))
          if (!console_ScriptRunLine(line)) {
            ret = false;
            break;
          }

        Z_Free(buffer);
      }
      else {
        lprintf(LO_ERROR, "Unable to read script file (%s)\n", filename);
        ret = false;
      }

      Z_Free(filename);
    }
    else {
      lprintf(LO_ERROR, "Cannot find script file (%s)\n", name);
      ret = false;
    }

    return ret;
  }

  return false;
}

static dboolean console_Check(const char* command, const char* args) {
  char name[CONSOLE_ENTRY_SIZE];

  if (sscanf(args, "%s", name)) {
    char* summary;

    summary = dsda_ConfigSummary(name);

    if (summary) {
      lprintf(LO_INFO, "%s\n", summary);
      Z_Free(summary);
      return true;
    }
  }

  return false;
}

static dboolean console_ChangeConfig(const char* command, const char* args, dboolean persist) {
  char name[CONSOLE_ENTRY_SIZE];
  char value_string[CONSOLE_ENTRY_SIZE];

  if (sscanf(args, "%s %s", name, value_string)) {
    int id;

    id = dsda_ConfigIDByName(name);
    if (id) {
      dsda_config_type_t config_type;

      config_type = dsda_ConfigType(id);
      if (config_type == dsda_config_int) {
        int value_int;

        if (sscanf(value_string, "%d", &value_int)) {
          dsda_UpdateIntConfig(id, value_int, persist);
          return true;
        }
      }
      else {
        dsda_UpdateStringConfig(id, value_string, persist);
        return true;
      }
    }
  }

  return false;
}

static dboolean console_Assign(const char* command, const char* args) {
  return console_ChangeConfig(command, args, false);
}

static dboolean console_Update(const char* command, const char* args) {
  return console_ChangeConfig(command, args, true);
}

static dboolean console_ToggleConfig(const char* command, const char* args, dboolean persist) {
  char name[CONSOLE_ENTRY_SIZE];

  if (sscanf(args, "%s", name)) {
    int id;

    id = dsda_ConfigIDByName(name);
    if (id) {
      dsda_config_type_t config_type;

      config_type = dsda_ConfigType(id);
      if (config_type == dsda_config_int) {
        dsda_ToggleConfig(id, persist);
        return true;
      }
    }
  }

  return false;
}

static dboolean console_ToggleAssign(const char* command, const char* args) {
  return console_ToggleConfig(command, args, false);
}

static dboolean console_ToggleUpdate(const char* command, const char* args) {
  return console_ToggleConfig(command, args, true);
}

static dboolean console_ConfigForget(const char* command, const char* args) {
  void M_ForgetCurrentConfig(void);

  M_ForgetCurrentConfig();

  return true;
}

static dboolean console_ConfigRemember(const char* command, const char* args) {
  void M_RememberCurrentConfig(void);

  M_RememberCurrentConfig();

  return true;
}

static dboolean console_WadStatsForget(const char* command, const char* args) {
  void M_ForgetWadStats(void);

  M_ForgetWadStats();

  return true;
}

static dboolean console_WadStatsRemember(const char* command, const char* args) {
  void M_RememberWadStats(void);

  M_RememberWadStats();

  return true;
}

static dboolean console_FreeTextUpdate(const char* command, const char* args) {
  dsda_UpdateStringConfig(dsda_config_free_text, args, true);

  return true;
}

static dboolean console_FreeTextClear(const char* command, const char* args) {
  dsda_UpdateStringConfig(dsda_config_free_text, "", true);

  return true;
}

static dboolean console_SetMobjState(mobj_t* mobj, statenum_t state) {
  if (!state)
    return false;

  P_MapStart();
  P_SetMobjState(mobj, state);
  P_MapEnd();

  return true;
}

static dboolean console_MoveMobj(mobj_t* mobj, fixed_t x, fixed_t y) {
  if (!mobj)
    return false;

  P_MapStart();
  P_UnqualifiedMove(mobj, x, y);
  P_MapEnd();

  return true;
}

static dboolean console_SetTarget(mobj_t* mobj, mobj_t* target) {
  if (!mobj || !target)
    return false;

  P_SetTarget(&mobj->target, target);
  mobj->threshold = BASETHRESHOLD;

  return true;
}

static void console_SetMobjFlags(mobj_t* mobj, uint64_t flags, uint64_t flags2) {
  P_MapStart();
  P_UnsetThingPosition(mobj);
  mobj->flags = flags;
  mobj->flags2 = flags2;
  P_SetThingPosition(mobj);
  P_MapEnd();
}

static dboolean console_TargetSpawn(const char* command, const char* args) {
  mobj_t* target;

  target = HU_Target();

  return target && console_SetMobjState(target, target->info->spawnstate);
}

static dboolean console_TargetSee(const char* command, const char* args) {
  mobj_t* target;

  target = HU_Target();

  return target && console_SetMobjState(target, target->info->seestate);
}

static dboolean console_TargetPain(const char* command, const char* args) {
  mobj_t* target;

  target = HU_Target();

  return target && console_SetMobjState(target, target->info->painstate);
}

static dboolean console_TargetMelee(const char* command, const char* args) {
  mobj_t* target;

  target = HU_Target();

  return target && console_SetMobjState(target, target->info->meleestate);
}

static dboolean console_TargetMissile(const char* command, const char* args) {
  mobj_t* target;

  target = HU_Target();

  return target && console_SetMobjState(target, target->info->missilestate);
}

static dboolean console_TargetDeath(const char* command, const char* args) {
  mobj_t* target;

  target = HU_Target();

  return target && console_SetMobjState(target, target->info->deathstate);
}

static dboolean console_TargetXDeath(const char* command, const char* args) {
  mobj_t* target;

  target = HU_Target();

  return target && console_SetMobjState(target, target->info->xdeathstate);
}

static dboolean console_TargetRaise(const char* command, const char* args) {
  mobj_t* target;

  target = HU_Target();

  return target && console_SetMobjState(target, target->info->raisestate);
}

static dboolean console_TargetSetState(const char* command, const char* args) {
  int state;
  mobj_t* target;

  if (sscanf(args, "%d", &state) != 1 || state < 0 || state >= num_states)
    return false;

  target = HU_Target();

  return target && console_SetMobjState(target, state);
}

static dboolean console_TargetSetHealth(const char* command, const char* args) {
  int health;
  mobj_t* target;

  if (sscanf(args, "%i", &health) != 1)
    return false;

  target = HU_Target();

  if (!target)
    return false;

  target->health = health;
  if (target->player)
    target->player->health = health;

  return true;
}

static dboolean console_TargetMove(const char* command, const char* args) {
  fixed_t x, y;
  mobj_t* target;

  if (sscanf(args, "%d %d", &x, &y) != 2)
    return false;

  x <<= FRACBITS;
  y <<= FRACBITS;

  target = HU_Target();

  return console_MoveMobj(target, x, y);
}

static dboolean console_TargetSetTarget(const char* command, const char* args) {
  mobj_t* target;
  int new_target_index;
  mobj_t* new_target;

  if (sscanf(args, "%d", &new_target_index) != 1)
    return false;

  target = HU_Target();
  new_target = dsda_FindMobj(new_target_index);

  return console_SetTarget(target, new_target);
}

static dboolean console_TargetTargetPlayer(const char* command, const char* args) {
  mobj_t* target;

  target = HU_Target();

  return console_SetTarget(target, target_player.mo);
}

static dboolean console_TargetActivateLine(const char* command, const char* args) {
  int id;
  mobj_t* target;

  if (sscanf(args, "%i", &id) != 1)
    return false;

  target = HU_Target();

  return console_ActivateLine(target, id, false);
}

static dboolean console_TargetAddFlags(const char* command, const char* args) {
  mobj_t* target;
  uint64_t flags, flags2;
  char flag_str[CONSOLE_ENTRY_SIZE];

  if (sscanf(args, "%s", flag_str) != 1)
    return false;

  target = HU_Target();

  if (!target)
    return false;

  flags = target->flags | deh_stringToMobjFlags(flag_str);
  flags2 = target->flags2 | deh_stringToMBF21MobjFlags(flag_str);

  console_SetMobjFlags(target, flags, flags2);

  return true;
}

static dboolean console_TargetRemoveFlags(const char* command, const char* args) {
  mobj_t* target;
  uint64_t flags, flags2;
  char flag_str[CONSOLE_ENTRY_SIZE];

  if (sscanf(args, "%s", flag_str) != 1)
    return false;

  target = HU_Target();

  if (!target)
    return false;

  flags = target->flags & ~deh_stringToMobjFlags(flag_str);
  flags2 = target->flags2 & ~deh_stringToMBF21MobjFlags(flag_str);

  console_SetMobjFlags(target, flags, flags2);

  return true;
}

static dboolean console_TargetSetFlags(const char* command, const char* args) {
  mobj_t* target;
  uint64_t flags, flags2;
  char flag_str[CONSOLE_ENTRY_SIZE];

  if (sscanf(args, "%s", flag_str) != 1)
    return false;

  target = HU_Target();

  if (!target)
    return false;

  flags = deh_stringToMobjFlags(flag_str);
  flags2 = deh_stringToMBF21MobjFlags(flag_str);

  console_SetMobjFlags(target, flags, flags2);

  return true;
}

static dboolean console_MobjSpawn(const char* command, const char* args) {
  int index;
  mobj_t* target;

  if (sscanf(args, "%d", &index) != 1 || index < 0)
    return false;

  target = dsda_FindMobj(index);

  return target && console_SetMobjState(target, target->info->spawnstate);
}

static dboolean console_MobjSee(const char* command, const char* args) {
  int index;
  mobj_t* target;

  if (sscanf(args, "%d", &index) != 1 || index < 0)
    return false;

  target = dsda_FindMobj(index);

  return target && console_SetMobjState(target, target->info->seestate);
}

static dboolean console_MobjPain(const char* command, const char* args) {
  int index;
  mobj_t* target;

  if (sscanf(args, "%d", &index) != 1 || index < 0)
    return false;

  target = dsda_FindMobj(index);

  return target && console_SetMobjState(target, target->info->painstate);
}

static dboolean console_MobjMelee(const char* command, const char* args) {
  int index;
  mobj_t* target;

  if (sscanf(args, "%d", &index) != 1 || index < 0)
    return false;

  target = dsda_FindMobj(index);

  return target && console_SetMobjState(target, target->info->meleestate);
}

static dboolean console_MobjMissile(const char* command, const char* args) {
  int index;
  mobj_t* target;

  if (sscanf(args, "%d", &index) != 1 || index < 0)
    return false;

  target = dsda_FindMobj(index);

  return target && console_SetMobjState(target, target->info->missilestate);
}

static dboolean console_MobjDeath(const char* command, const char* args) {
  int index;
  mobj_t* target;

  if (sscanf(args, "%d", &index) != 1 || index < 0)
    return false;

  target = dsda_FindMobj(index);

  return target && console_SetMobjState(target, target->info->deathstate);
}

static dboolean console_MobjXDeath(const char* command, const char* args) {
  int index;
  mobj_t* target;

  if (sscanf(args, "%d", &index) != 1 || index < 0)
    return false;

  target = dsda_FindMobj(index);

  return target && console_SetMobjState(target, target->info->xdeathstate);
}

static dboolean console_MobjRaise(const char* command, const char* args) {
  int index;
  mobj_t* target;

  if (sscanf(args, "%d", &index) != 1 || index < 0)
    return false;

  target = dsda_FindMobj(index);

  return target && console_SetMobjState(target, target->info->raisestate);
}

static dboolean console_MobjSetState(const char* command, const char* args) {
  int state;
  int index;
  mobj_t* target;

  if (sscanf(args, "%d %d", &index, &state) != 2 || index < 0 || state < 0 || state >= num_states)
    return false;

  target = dsda_FindMobj(index);

  return target && console_SetMobjState(target, state);
}

static dboolean console_MobjSetHealth(const char* command, const char* args) {
  int health;
  int index;
  mobj_t* target;

  if (sscanf(args, "%d %i", &index, &health) != 2)
    return false;

  target = dsda_FindMobj(index);

  if (!target)
    return false;

  target->health = health;
  if (target->player)
    target->player->health = health;

  return true;
}

static dboolean console_MobjMove(const char* command, const char* args) {
  fixed_t x, y;
  int index;
  mobj_t* target;

  if (sscanf(args, "%d %d %d", &index, &x, &y) != 3)
    return false;

  x <<= FRACBITS;
  y <<= FRACBITS;

  target = dsda_FindMobj(index);

  return console_MoveMobj(target, x, y);
}

static dboolean console_MobjSetTarget(const char* command, const char* args) {
  int index;
  mobj_t* target;
  int new_target_index;
  mobj_t* new_target;

  if (sscanf(args, "%d %d", &index, &new_target_index) != 2)
    return false;

  target = dsda_FindMobj(index);
  new_target = dsda_FindMobj(new_target_index);

  return console_SetTarget(target, new_target);
}

static dboolean console_MobjTargetPlayer(const char* command, const char* args) {
  int index;
  mobj_t* target;

  if (sscanf(args, "%d", &index) != 1)
    return false;

  target = dsda_FindMobj(index);

  return console_SetTarget(target, target_player.mo);
}

static dboolean console_MobjActivateLine(const char* command, const char* args) {
  int id;
  int index;
  mobj_t* target;

  if (sscanf(args, "%d %i", &index, &id) != 2)
    return false;

  target = dsda_FindMobj(index);

  return console_ActivateLine(target, id, false);
}

static dboolean console_MobjAddFlags(const char* command, const char* args) {
  int index;
  mobj_t* target;
  uint64_t flags, flags2;
  char flag_str[CONSOLE_ENTRY_SIZE];

  if (sscanf(args, "%d %s", &index, flag_str) != 2)
    return false;

  target = dsda_FindMobj(index);

  if (!target)
    return false;

  flags = target->flags | deh_stringToMobjFlags(flag_str);
  flags2 = target->flags2 | deh_stringToMBF21MobjFlags(flag_str);

  console_SetMobjFlags(target, flags, flags2);

  return true;
}

static dboolean console_MobjRemoveFlags(const char* command, const char* args) {
  int index;
  mobj_t* target;
  uint64_t flags, flags2;
  char flag_str[CONSOLE_ENTRY_SIZE];

  if (sscanf(args, "%d %s", &index, flag_str) != 2)
    return false;

  target = dsda_FindMobj(index);

  if (!target)
    return false;

  flags = target->flags & ~deh_stringToMobjFlags(flag_str);
  flags2 = target->flags2 & ~deh_stringToMBF21MobjFlags(flag_str);

  console_SetMobjFlags(target, flags, flags2);

  return true;
}

static dboolean console_MobjSetFlags(const char* command, const char* args) {
  int index;
  mobj_t* target;
  uint64_t flags, flags2;
  char flag_str[CONSOLE_ENTRY_SIZE];

  if (sscanf(args, "%d %s", &index, flag_str) != 2)
    return false;

  target = dsda_FindMobj(index);

  if (!target)
    return false;

  flags = deh_stringToMobjFlags(flag_str);
  flags2 = deh_stringToMBF21MobjFlags(flag_str);

  console_SetMobjFlags(target, flags, flags2);

  return true;
}

static dboolean console_BossActivateLine(const char* command, const char* args) {
  int id;
  int index;
  mobj_t* target;

  if (sscanf(args, "%d %i", &index, &id) != 2)
    return false;

  target = dsda_FindMobj(index);

  return console_ActivateLine(target, id, true);
}

static dboolean console_Spawn(const char* command, const char* args) {
  fixed_t x, y, z;
  int type;

  if (sscanf(args, "%d %d %d %d", &x, &y, &z, &type) != 4 || type < 0)
    return false;

  x <<= FRACBITS;
  y <<= FRACBITS;
  z <<= FRACBITS;

  type = dsda_FindDehMobjIndex(type - 1);

  if (type == DEH_INDEX_NOT_FOUND)
    return false;

  return P_SpawnMobj(x, y, z, type) != NULL;
}

static dboolean console_StateSetTics(const char* command, const char* args) {
  int id;
  int value;

  if (sscanf(args, "%d %i", &id, &value) != 2 || id < 0 || id >= num_states)
    return false;

  states[id].tics = value;

  return true;
}

static dboolean console_StateSetMisc1(const char* command, const char* args) {
  int id;
  int value;

  if (sscanf(args, "%d %i", &id, &value) != 2 || id < 0 || id >= num_states)
    return false;

  states[id].misc1 = value;

  return true;
}

static dboolean console_StateSetMisc2(const char* command, const char* args) {
  int id;
  int value;

  if (sscanf(args, "%d %i", &id, &value) != 2 || id < 0 || id >= num_states)
    return false;

  states[id].misc2 = value;

  return true;
}

static dboolean console_StateSetArgs1(const char* command, const char* args) {
  int id;
  statearg_t value;

  if (sscanf(args, "%d %lli", &id, &value) != 2 || id < 0 || id >= num_states)
    return false;

  states[id].args[0] = value;

  return true;
}

static dboolean console_StateSetArgs2(const char* command, const char* args) {
  int id;
  statearg_t value;

  if (sscanf(args, "%d %lli", &id, &value) != 2 || id < 0 || id >= num_states)
    return false;

  states[id].args[1] = value;

  return true;
}

static dboolean console_StateSetArgs3(const char* command, const char* args) {
  int id;
  statearg_t value;

  if (sscanf(args, "%d %lli", &id, &value) != 2 || id < 0 || id >= num_states)
    return false;

  states[id].args[2] = value;

  return true;
}

static dboolean console_StateSetArgs4(const char* command, const char* args) {
  int id;
  statearg_t value;

  if (sscanf(args, "%d %lli", &id, &value) != 2 || id < 0 || id >= num_states)
    return false;

  states[id].args[3] = value;

  return true;
}

static dboolean console_StateSetArgs5(const char* command, const char* args) {
  int id;
  statearg_t value;

  if (sscanf(args, "%d %lli", &id, &value) != 2 || id < 0 || id >= num_states)
    return false;

  states[id].args[4] = value;

  return true;
}

static dboolean console_StateSetArgs6(const char* command, const char* args) {
  int id;
  statearg_t value;

  if (sscanf(args, "%d %lli", &id, &value) != 2 || id < 0 || id >= num_states)
    return false;

  states[id].args[5] = value;

  return true;
}

static dboolean console_StateSetArgs7(const char* command, const char* args) {
  int id;
  statearg_t value;

  if (sscanf(args, "%d %lli", &id, &value) != 2 || id < 0 || id >= num_states)
    return false;

  states[id].args[6] = value;

  return true;
}

static dboolean console_StateSetArgs8(const char* command, const char* args) {
  int id;
  statearg_t value;

  if (sscanf(args, "%d %lli", &id, &value) != 2 || id < 0 || id >= num_states)
    return false;

  states[id].args[7] = value;

  return true;
}

static dboolean console_MobjInfoSetHealth(const char* command, const char* args) {
  int type;
  int value;

  if (sscanf(args, "%d %i", &type, &value) != 2 || type < 0)
    return false;

  type = dsda_FindDehMobjIndex(type - 1);

  if (type == DEH_INDEX_NOT_FOUND)
    return false;

  mobjinfo[type].spawnhealth = value;

  return true;
}

static dboolean console_MobjInfoSetRadius(const char* command, const char* args) {
  int type;
  int value;

  if (sscanf(args, "%d %i", &type, &value) != 2 || type < 0)
    return false;

  type = dsda_FindDehMobjIndex(type - 1);

  if (type == DEH_INDEX_NOT_FOUND)
    return false;

  value <<= FRACBITS;

  mobjinfo[type].radius = value;

  return true;
}

static dboolean console_MobjInfoSetHeight(const char* command, const char* args) {
  int type;
  int value;

  if (sscanf(args, "%d %i", &type, &value) != 2 || type < 0)
    return false;

  type = dsda_FindDehMobjIndex(type - 1);

  if (type == DEH_INDEX_NOT_FOUND)
    return false;

  value <<= FRACBITS;

  mobjinfo[type].height = value;

  return true;
}

static dboolean console_MobjInfoSetMass(const char* command, const char* args) {
  int type;
  int value;

  if (sscanf(args, "%d %i", &type, &value) != 2 || type < 0)
    return false;

  type = dsda_FindDehMobjIndex(type - 1);

  if (type == DEH_INDEX_NOT_FOUND)
    return false;

  mobjinfo[type].mass = value;

  return true;
}

static dboolean console_MobjInfoSetDamage(const char* command, const char* args) {
  int type;
  int value;

  if (sscanf(args, "%d %i", &type, &value) != 2 || type < 0)
    return false;

  type = dsda_FindDehMobjIndex(type - 1);

  if (type == DEH_INDEX_NOT_FOUND)
    return false;

  mobjinfo[type].damage = value;

  return true;
}

static dboolean console_MobjInfoSetSpeed(const char* command, const char* args) {
  int type;
  int value;

  if (sscanf(args, "%d %i", &type, &value) != 2 || type < 0)
    return false;

  type = dsda_FindDehMobjIndex(type - 1);

  if (type == DEH_INDEX_NOT_FOUND)
    return false;

  mobjinfo[type].speed = value;

  return true;
}

static dboolean console_MobjInfoSetFastSpeed(const char* command, const char* args) {
  int type;
  int value;

  if (sscanf(args, "%d %i", &type, &value) != 2 || type < 0)
    return false;

  type = dsda_FindDehMobjIndex(type - 1);

  if (type == DEH_INDEX_NOT_FOUND)
    return false;

  mobjinfo[type].altspeed = value;

  return true;
}

static dboolean console_MobjInfoSetMeleeRange(const char* command, const char* args) {
  int type;
  int value;

  if (sscanf(args, "%d %i", &type, &value) != 2 || type < 0)
    return false;

  type = dsda_FindDehMobjIndex(type - 1);

  if (type == DEH_INDEX_NOT_FOUND)
    return false;

  value <<= FRACBITS;

  mobjinfo[type].meleerange = value;

  return true;
}

static dboolean console_MobjInfoSetReactionTime(const char* command, const char* args) {
  int type;
  int value;

  if (sscanf(args, "%d %i", &type, &value) != 2 || type < 0)
    return false;

  type = dsda_FindDehMobjIndex(type - 1);

  if (type == DEH_INDEX_NOT_FOUND)
    return false;

  mobjinfo[type].reactiontime = value;

  return true;
}

static dboolean console_MobjInfoSetPainChance(const char* command, const char* args) {
  int type;
  int value;

  if (sscanf(args, "%d %i", &type, &value) != 2 || type < 0)
    return false;

  type = dsda_FindDehMobjIndex(type - 1);

  if (type == DEH_INDEX_NOT_FOUND)
    return false;

  mobjinfo[type].painchance = value;

  return true;
}

static dboolean console_MobjInfoSetInfightingGroup(const char* command, const char* args) {
  int type;
  int value;

  if (sscanf(args, "%d %i", &type, &value) != 2 || type < 0)
    return false;

  type = dsda_FindDehMobjIndex(type - 1);

  if (type == DEH_INDEX_NOT_FOUND)
    return false;

  if (value < 0)
    value = IG_DEFAULT;
  else
    value += IG_END;

  mobjinfo[type].infighting_group = value;

  return true;
}

static dboolean console_MobjInfoSetProjectileGroup(const char* command, const char* args) {
  int type;
  int value;

  if (sscanf(args, "%d %i", &type, &value) != 2 || type < 0)
    return false;

  type = dsda_FindDehMobjIndex(type - 1);

  if (type == DEH_INDEX_NOT_FOUND)
    return false;

  if (value < 0)
    value = PG_GROUPLESS;
  else
    value += PG_END;

  mobjinfo[type].projectile_group = value;

  return true;
}

static dboolean console_MobjInfoSetSplashGroup(const char* command, const char* args) {
  int type;
  int value;

  if (sscanf(args, "%d %i", &type, &value) != 2 || type < 0)
    return false;

  type = dsda_FindDehMobjIndex(type - 1);

  if (type == DEH_INDEX_NOT_FOUND)
    return false;

  if (value < 0)
    value = SG_DEFAULT;
  else
    value += SG_END;

  mobjinfo[type].splash_group = value;

  return true;
}

static dboolean console_MobjInfoAddFlags(const char* command, const char* args) {
  int type;
  char flag_str[CONSOLE_ENTRY_SIZE];

  if (sscanf(args, "%d %s", &type, flag_str) != 2 || type < 0)
    return false;

  type = dsda_FindDehMobjIndex(type - 1);

  if (type == DEH_INDEX_NOT_FOUND)
    return false;

  mobjinfo[type].flags |= deh_stringToMobjFlags(flag_str);
  mobjinfo[type].flags2 |= deh_stringToMBF21MobjFlags(flag_str);

  return true;
}

static dboolean console_MobjInfoRemoveFlags(const char* command, const char* args) {
  int type;
  char flag_str[CONSOLE_ENTRY_SIZE];

  if (sscanf(args, "%d %s", &type, flag_str) != 2 || type < 0)
    return false;

  type = dsda_FindDehMobjIndex(type - 1);

  if (type == DEH_INDEX_NOT_FOUND)
    return false;

  mobjinfo[type].flags &= ~deh_stringToMobjFlags(flag_str);
  mobjinfo[type].flags2 &= ~deh_stringToMBF21MobjFlags(flag_str);

  return true;
}

static dboolean console_MobjInfoSetFlags(const char* command, const char* args) {
  int type;
  char flag_str[CONSOLE_ENTRY_SIZE];

  if (sscanf(args, "%d %s", &type, flag_str) != 2 || type < 0)
    return false;

  type = dsda_FindDehMobjIndex(type - 1);

  if (type == DEH_INDEX_NOT_FOUND)
    return false;

  mobjinfo[type].flags = deh_stringToMobjFlags(flag_str);
  mobjinfo[type].flags2 = deh_stringToMBF21MobjFlags(flag_str);

  return true;
}

static dboolean console_MusicRestart(const char* command, const char* args) {
  S_StopMusic();
  S_RestartMusic();

  return true;
}

static dboolean console_AllGhosts(const char* command, const char* args) {
  if (bmapwidth)
    bmapwidth = 0;
  else
    P_RestoreOriginalBlockMap();

  return true;
}

typedef dboolean (*console_command_t)(const char*, const char*);

typedef struct {
  const char* command_name;
  console_command_t command;
  int flags;
} console_command_entry_t;

static console_command_entry_t console_commands[] = {
  // commands
  { "player.set_health", console_PlayerSetHealth, CF_NEVER },
  { "player.set_armor", console_PlayerSetArmor, CF_NEVER },
  { "player.give_weapon", console_PlayerGiveWeapon, CF_NEVER },
  { "player.give_ammo", console_PlayerGiveAmmo, CF_NEVER },
  { "player.set_ammo", console_PlayerSetAmmo, CF_NEVER },
  { "player.give_key", console_PlayerGiveKey, CF_NEVER },
  { "player.remove_key", console_PlayerRemoveKey, CF_NEVER },
  { "player.give_power", console_PlayerGivePower, CF_NEVER },
  { "player.remove_power", console_PlayerRemovePower, CF_NEVER },
  { "player.set_x", console_PlayerSetX, CF_NEVER },
  { "player.set_y", console_PlayerSetY, CF_NEVER },
  { "player.set_z", console_PlayerSetZ, CF_NEVER },
  { "player.round_x", console_PlayerRoundX, CF_NEVER },
  { "player.round_y", console_PlayerRoundY, CF_NEVER },
  { "player.round_xy", console_PlayerRoundXY, CF_NEVER },
  { "player.set_angle", console_PlayerSetAngle, CF_NEVER },
  { "player.round_angle", console_PlayerRoundAngle, CF_NEVER },

  { "music.restart", console_MusicRestart, CF_ALWAYS },

  { "level.exit", console_LevelExit, CF_NEVER },
  { "level.secret_exit", console_LevelSecretExit, CF_NEVER },

  { "script.run", console_ScriptRun, CF_ALWAYS },
  { "check", console_Check, CF_ALWAYS },
  { "assign", console_Assign, CF_ALWAYS },
  { "update", console_Update, CF_ALWAYS },
  { "toggle_assign", console_ToggleAssign, CF_ALWAYS },
  { "toggle_update", console_ToggleUpdate, CF_ALWAYS },
  { "config.forget", console_ConfigForget, CF_ALWAYS },
  { "config.remember", console_ConfigRemember, CF_ALWAYS },
  { "wad_stats.forget", console_WadStatsForget, CF_ALWAYS },
  { "wad_stats.remember", console_WadStatsRemember, CF_ALWAYS },
  { "free_text.update", console_FreeTextUpdate, CF_ALWAYS },
  { "free_text.clear", console_FreeTextClear, CF_ALWAYS },

  // tracking
  { "tracker.add_line", console_TrackerAddLine, CF_DEMO },
  { "t.al", console_TrackerAddLine, CF_DEMO },
  { "tracker.remove_line", console_TrackerRemoveLine, CF_DEMO },
  { "t.rl", console_TrackerRemoveLine, CF_DEMO },
  { "tracker.add_line_distance", console_TrackerAddLineDistance, CF_DEMO },
  { "t.ald", console_TrackerAddLineDistance, CF_DEMO },
  { "tracker.remove_line_distance", console_TrackerRemoveLineDistance, CF_DEMO },
  { "t.rld", console_TrackerRemoveLineDistance, CF_DEMO },
  { "tracker.add_sector", console_TrackerAddSector, CF_DEMO },
  { "t.as", console_TrackerAddSector, CF_DEMO },
  { "tracker.remove_sector", console_TrackerRemoveSector, CF_DEMO },
  { "t.rs", console_TrackerRemoveSector, CF_DEMO },
  { "tracker.add_mobj", console_TrackerAddMobj, CF_DEMO },
  { "t.am", console_TrackerAddMobj, CF_DEMO },
  { "tracker.remove_mobj", console_TrackerRemoveMobj, CF_DEMO },
  { "t.rm", console_TrackerRemoveMobj, CF_DEMO },
  { "tracker.add_player", console_TrackerAddPlayer, CF_DEMO },
  { "t.ap", console_TrackerAddPlayer, CF_DEMO },
  { "tracker.remove_player", console_TrackerRemovePlayer, CF_DEMO },
  { "t.rp", console_TrackerRemovePlayer, CF_DEMO },
  { "tracker.reset", console_TrackerReset, CF_DEMO },
  { "t.r", console_TrackerReset, CF_DEMO },

  // thing manipulation
  { "target.spawn", console_TargetSpawn, CF_NEVER },
  { "target.see", console_TargetSee, CF_NEVER },
  { "target.pain", console_TargetPain, CF_NEVER },
  { "target.melee", console_TargetMelee, CF_NEVER },
  { "target.missile", console_TargetMissile, CF_NEVER },
  { "target.death", console_TargetDeath, CF_NEVER },
  { "target.xdeath", console_TargetXDeath, CF_NEVER },
  { "target.raise", console_TargetRaise, CF_NEVER },
  { "target.set_state", console_TargetSetState, CF_NEVER },
  { "target.set_health", console_TargetSetHealth, CF_NEVER },
  { "target.move", console_TargetMove, CF_NEVER },
  { "target.set_target", console_TargetSetTarget, CF_NEVER },
  { "target.target_player", console_TargetTargetPlayer, CF_NEVER },
  { "target.add_flags", console_TargetAddFlags, CF_NEVER },
  { "target.remove_flags", console_TargetRemoveFlags, CF_NEVER },
  { "target.set_flags", console_TargetSetFlags, CF_NEVER },

  { "mobj.spawn", console_MobjSpawn, CF_NEVER },
  { "mobj.see", console_MobjSee, CF_NEVER },
  { "mobj.pain", console_MobjPain, CF_NEVER },
  { "mobj.melee", console_MobjMelee, CF_NEVER },
  { "mobj.missile", console_MobjMissile, CF_NEVER },
  { "mobj.death", console_MobjDeath, CF_NEVER },
  { "mobj.xdeath", console_MobjXDeath, CF_NEVER },
  { "mobj.raise", console_MobjRaise, CF_NEVER },
  { "mobj.set_state", console_MobjSetState, CF_NEVER },
  { "mobj.set_health", console_MobjSetHealth, CF_NEVER },
  { "mobj.move", console_MobjMove, CF_NEVER },
  { "mobj.set_target", console_MobjSetTarget, CF_NEVER },
  { "mobj.target_player", console_MobjTargetPlayer, CF_NEVER },
  { "mobj.add_flags", console_MobjAddFlags, CF_NEVER },
  { "mobj.remove_flags", console_MobjRemoveFlags, CF_NEVER },
  { "mobj.set_flags", console_MobjSetFlags, CF_NEVER },

  { "spawn", console_Spawn, CF_NEVER },

  // lines
  { "player.activate_line", console_PlayerActivateLine, CF_NEVER },
  { "target.activate_line", console_TargetActivateLine, CF_NEVER },
  { "mobj.activate_line", console_MobjActivateLine, CF_NEVER },
  { "boss.activate_line", console_BossActivateLine, CF_NEVER },

  // states
  { "state.set_tics", console_StateSetTics, CF_NEVER },
  { "state.set_misc1", console_StateSetMisc1, CF_NEVER },
  { "state.set_misc2", console_StateSetMisc2, CF_NEVER },
  { "state.set_args1", console_StateSetArgs1, CF_NEVER },
  { "state.set_args2", console_StateSetArgs2, CF_NEVER },
  { "state.set_args3", console_StateSetArgs3, CF_NEVER },
  { "state.set_args4", console_StateSetArgs4, CF_NEVER },
  { "state.set_args5", console_StateSetArgs5, CF_NEVER },
  { "state.set_args6", console_StateSetArgs6, CF_NEVER },
  { "state.set_args7", console_StateSetArgs7, CF_NEVER },
  { "state.set_args8", console_StateSetArgs8, CF_NEVER },

  // mobjinfo
  { "mobjinfo.set_health", console_MobjInfoSetHealth, CF_NEVER },
  { "mobjinfo.set_radius", console_MobjInfoSetRadius, CF_NEVER },
  { "mobjinfo.set_height", console_MobjInfoSetHeight, CF_NEVER },
  { "mobjinfo.set_mass", console_MobjInfoSetMass, CF_NEVER },
  { "mobjinfo.set_damage", console_MobjInfoSetDamage, CF_NEVER },
  { "mobjinfo.set_speed", console_MobjInfoSetSpeed, CF_NEVER },
  { "mobjinfo.set_fast_speed", console_MobjInfoSetFastSpeed, CF_NEVER },
  { "mobjinfo.set_melee_range", console_MobjInfoSetMeleeRange, CF_NEVER },
  { "mobjinfo.set_reaction_time", console_MobjInfoSetReactionTime, CF_NEVER },
  { "mobjinfo.set_pain_chance", console_MobjInfoSetPainChance, CF_NEVER },
  { "mobjinfo.set_infighting_group", console_MobjInfoSetInfightingGroup, CF_NEVER },
  { "mobjinfo.set_projectile_group", console_MobjInfoSetProjectileGroup, CF_NEVER },
  { "mobjinfo.set_splash_group", console_MobjInfoSetSplashGroup, CF_NEVER },
  { "mobjinfo.add_flags", console_MobjInfoAddFlags, CF_NEVER },
  { "mobjinfo.remove_flags", console_MobjInfoRemoveFlags, CF_NEVER },
  { "mobjinfo.set_flags", console_MobjInfoSetFlags, CF_NEVER },

  // traversing time
  { "jump.to_tic", console_JumpToTic, CF_DEMO },
  { "jump.by_tic", console_JumpByTic, CF_DEMO },

  // build mode
  { "brute_force.start", console_BruteForceStart, CF_DEMO },
  { "bf.start", console_BruteForceStart, CF_DEMO },
  { "brute_force.frame", console_BruteForceFrame, CF_DEMO },
  { "bf.frame", console_BruteForceFrame, CF_DEMO },
  { "brute_force.keep", console_BruteForceKeep, CF_DEMO },
  { "bf.keep", console_BruteForceKeep, CF_DEMO },
  { "brute_force.nomonsters", console_BruteForceNoMonsters, CF_DEMO },
  { "bf.nomo", console_BruteForceNoMonsters, CF_DEMO },
  { "brute_force.monsters", console_BruteForceMonsters, CF_DEMO },
  { "bf.mo", console_BruteForceMonsters, CF_DEMO },
  { "build.turbo", console_BuildTurbo, CF_DEMO },
  { "b.turbo", console_BuildTurbo, CF_DEMO },
  { "mf", console_BuildMF, CF_DEMO },
  { "mb", console_BuildMB, CF_DEMO },
  { "sr", console_BuildSR, CF_DEMO },
  { "sl", console_BuildSL, CF_DEMO },
  { "tr", console_BuildTR, CF_DEMO },
  { "tl", console_BuildTL, CF_DEMO },
  { "fu", console_BuildFU, CF_DEMO },
  { "fd", console_BuildFD, CF_DEMO },
  { "fc", console_BuildFC, CF_DEMO },
  { "lu", console_BuildLU, CF_DEMO },
  { "ld", console_BuildLD, CF_DEMO },
  { "lc", console_BuildLC, CF_DEMO },
  { "ua", console_BuildUA, CF_DEMO },

  // demos
  { "demo.export", console_DemoExport, CF_ALWAYS },
  { "demo.start", console_DemoStart, CF_NEVER },
  { "demo.stop", console_DemoStop, CF_ALWAYS },
  { "demo.join", console_DemoJoin, CF_ALWAYS },

  { "game.quit", console_GameQuit, CF_ALWAYS },
  { "game.describe", console_GameDescribe, CF_ALWAYS },

  // cheats
  { "idchoppers", console_BasicCheat, CF_DEMO },
  { "iddqd", console_BasicCheat, CF_DEMO },
  { "idkfa", console_BasicCheat, CF_DEMO },
  { "idfa", console_BasicCheat, CF_DEMO },
  { "idspispopd", console_BasicCheat, CF_DEMO },
  { "idclip", console_BasicCheat, CF_DEMO },
  { "idmypos", console_BasicCheat, CF_DEMO },
  { "idrate", console_BasicCheat, CF_DEMO },
  { "iddt", console_IDDT, CF_DEMO },
  { "iddst", console_BasicCheat, CF_DEMO },
  { "iddkt", console_BasicCheat, CF_DEMO },
  { "iddit", console_BasicCheat, CF_DEMO },
  { "idclev", console_BasicCheat, CF_DEMO },
  { "idmus", console_BasicCheat, CF_DEMO },

  { "tntcomp", console_BasicCheat, CF_DEMO },
  { "tntem", console_BasicCheat, CF_DEMO },
  { "tnthom", console_BasicCheat, CF_DEMO },
  { "tntka", console_BasicCheat, CF_DEMO },
  { "tntsmart", console_BasicCheat, CF_DEMO },
  { "tntpitch", console_BasicCheat, CF_DEMO },
  { "tntfast", console_BasicCheat, CF_DEMO },
  { "tntice", console_BasicCheat, CF_DEMO },
  { "tntpush", console_BasicCheat, CF_DEMO },

  { "notarget", console_BasicCheat, CF_DEMO },
  { "fly", console_BasicCheat, CF_DEMO },
  { "fullclip", console_CheatFullClip, CF_NEVER },
  { "freeze", console_Freeze, CF_NEVER },
  { "nosleep", console_NoSleep, CF_NEVER },
  { "allghosts", console_AllGhosts, CF_NEVER },

  { "quicken", console_BasicCheat, CF_DEMO },
  { "ponce", console_BasicCheat, CF_DEMO },
  { "kitty", console_BasicCheat, CF_DEMO },
  { "massacre", console_BasicCheat, CF_DEMO },
  { "rambo", console_BasicCheat, CF_DEMO },
  { "skel", console_BasicCheat, CF_DEMO },
  { "shazam", console_BasicCheat, CF_DEMO },
  { "ravmap", console_IDDT, CF_DEMO },
  { "cockadoodledoo", console_BasicCheat, CF_DEMO },
  { "gimme", console_BasicCheat, CF_DEMO },
  { "engage", console_BasicCheat, CF_DEMO },

  { "satan", console_BasicCheat, CF_DEMO },
  { "clubmed", console_BasicCheat, CF_DEMO },
  { "butcher", console_BasicCheat, CF_DEMO },
  { "nra", console_BasicCheat, CF_DEMO },
  { "indiana", console_BasicCheat, CF_DEMO },
  { "locksmith", console_BasicCheat, CF_DEMO },
  { "sherlock", console_BasicCheat, CF_DEMO },
  { "casper", console_BasicCheat, CF_DEMO },
  { "init", console_BasicCheat, CF_DEMO },
  { "mapsco", console_IDDT, CF_DEMO },
  { "deliverance", console_BasicCheat, CF_DEMO },
  { "shadowcaster", console_BasicCheat, CF_DEMO },
  { "visit", console_BasicCheat, CF_DEMO },
  { "puke", console_BasicCheat, CF_DEMO },

  // exit
  { "exit", console_Exit, CF_ALWAYS },
  { "quit", console_Exit, CF_ALWAYS },
  { NULL }
};

static void dsda_AddConsoleMessage(const char* message) {
  strncpy(console_message_entry, message, CONSOLE_ENTRY_SIZE);
}

static dboolean dsda_AuthorizeCommand(console_command_entry_t* entry) {
  if (!(entry->flags & CF_DEMO) && (demorecording || demoplayback)) {
    dsda_AddConsoleMessage("command not allowed in demo mode");
    return false;
  }

  if (!(entry->flags & CF_STRICT) && dsda_StrictMode()) {
    dsda_AddConsoleMessage("command not allowed in strict mode");
    return false;
  }

  if (gamestate != GS_LEVEL) {
    dsda_AddConsoleMessage("command only allowed during levels");
    return false;
  }

  return true;
}

static dboolean dsda_ExecuteConsole(const char* command_line, dboolean noise) {
  char command[CONSOLE_ENTRY_SIZE];
  char args[CONSOLE_ENTRY_SIZE];
  int scan_count;
  dboolean ret = true;

  scan_count = sscanf(command_line, "%s %[^;]", command, args);

  if (scan_count) {
    console_command_entry_t* entry;

    if (scan_count == 1) args[0] = '\0';

    for (entry = console_commands; entry->command; entry++) {
      if (!stricmp(command, entry->command_name)) {
        if (dsda_AuthorizeCommand(entry)) {
          if (entry->command(command, args)) {
            dsda_AddConsoleMessage("command executed");

            if (noise)
              S_StartVoidSound(g_sfx_console);
          }
          else {
            dsda_AddConsoleMessage("command invalid");
            ret = false;

            if (noise)
              S_StartVoidSound(g_sfx_oof);
          }
        }
        else {
          ret = false;

          if (noise)
            S_StartVoidSound(g_sfx_oof);
        }

        break;
      }
    }

    if (!entry->command) {
      dsda_AddConsoleMessage("command unknown");
      ret = false;

      if (noise)
        S_StartVoidSound(g_sfx_oof);
    }
  }

  return ret;
}

void dsda_UpdateConsoleText(char* text) {
  int i;
  int length;

  length = strlen(text);

  for (i = 0; i < length; ++i) {
    int shift_i;

    if (text[i] < 32 || text[i] > 126)
      continue;

    for (shift_i = strlen(console_entry->text); shift_i > console_entry_index; --shift_i)
      console_entry->text[shift_i] = console_entry->text[shift_i - 1];

    console_entry->text[console_entry_index] = tolower(text[i]);
    if (console_entry_index < CONSOLE_ENTRY_SIZE)
      ++console_entry_index;
  }

  dsda_UpdateConsoleDisplay();
}

void dsda_UpdateConsoleHistory(void) {
  console_entry_t* last_command;

  if (console_entry != console_history_head)
    strcpy(console_history_head->text, console_entry->text);

  last_command = console_history_head->prev;
  if (!last_command || strcmp(last_command->text, console_entry->text)) {
    console_entry_t* new_head;

    new_head = Z_Calloc(sizeof(*new_head), 1);
    new_head->prev = console_history_head;
    console_history_head->next = new_head;
    console_history_head = new_head;
  }
  else
    memset(console_history_head->text, 0, CONSOLE_ENTRY_SIZE);

  console_entry = console_history_head;
}

void dsda_InterpretConsoleCommands(const char* str, dboolean noise, dboolean raise_errors) {
  int line;
  char* entry;
  char** lines;

  entry = Z_Strdup(str);
  lines = dsda_SplitString(entry, ";");
  for (line = 0; lines[line]; ++line)
    if (!dsda_ExecuteConsole(lines[line], noise) && raise_errors)
      I_Error("Console command failed: %s", lines[line]);

  Z_Free(lines);
  Z_Free(entry);
}

void dsda_UpdateConsole(int action) {
  if (action == MENU_BACKSPACE && console_entry_index > 0) {
    int shift_i;

    for (shift_i = console_entry_index; console_entry->text[shift_i]; ++shift_i)
      console_entry->text[shift_i - 1] = console_entry->text[shift_i];
    console_entry->text[shift_i - 1] = '\0';

    --console_entry_index;
    dsda_UpdateConsoleDisplay();
  }
  else if (action == MENU_ENTER) {
    dsda_InterpretConsoleCommands(console_entry->text, true, false);

    dsda_UpdateConsoleHistory();
    dsda_ResetConsoleEntry();
  }
  else if (action == MENU_UP) {
    if (console_entry->prev)
      console_entry = console_entry->prev;
    console_entry_index = strlen(console_entry->text);
    dsda_UpdateConsoleDisplay();
  }
  else if (action == MENU_DOWN) {
    if (console_entry->next)
      console_entry = console_entry->next;
    console_entry_index = strlen(console_entry->text);
    dsda_UpdateConsoleDisplay();
  }
  else if (action == MENU_RIGHT && console_entry->text[console_entry_index]) {
    ++console_entry_index;
    dsda_UpdateConsoleDisplay();
  }
  else if (action == MENU_LEFT && console_entry_index > 0) {
    --console_entry_index;
    dsda_UpdateConsoleDisplay();
  }
}

void dsda_ExecuteConsoleScript(int i) {
  int line;

  if (gamestate != GS_LEVEL || i < 0 || i >= CONSOLE_SCRIPT_COUNT)
    return;

  if (!dsda_console_script_lines[i]) {
    char* dup;

    dup = Z_Strdup(dsda_StringConfig(dsda_config_script_0 + i));
    dsda_console_script_lines[i] = dsda_SplitString(dup, ";");
  }

  for (line = 0; dsda_console_script_lines[i][line]; ++line)
    if (!console_ScriptRunLine(dsda_console_script_lines[i][line])) {
      doom_printf("Script %d failed", i);

      return;
    }

  doom_printf("Script %d executed", i);
}
