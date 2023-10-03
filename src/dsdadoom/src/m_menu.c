/* Emacs style mode select   -*- C -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *  DOOM selection menu, options, episode etc. (aka Big Font menus)
 *  Sliders and icons. Kinda widget stuff.
 *  Setup Menus.
 *  Extended HELP screens.
 *  Dynamic HELP screen.
 *
 *-----------------------------------------------------------------------------*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef _MSC_VER
#include <io.h>
#else
#include <unistd.h>
#endif

#include <stdio.h>

//#include "SDL.h"

#include "doomdef.h"
#include "doomstat.h"
#include "dstrings.h"
#include "d_main.h"
#include "v_video.h"
#include "w_wad.h"
#include "r_main.h"
#include "hu_stuff.h"
#include "st_stuff.h"
#include "g_game.h"
#include "s_sound.h"
#include "sounds.h"
#include "m_menu.h"
#include "d_deh.h"
#include "m_file.h"
#include "m_misc.h"
#include "lprintf.h"
#include "am_map.h"
#include "i_main.h"
#include "i_system.h"
#include "i_video.h"
#include "i_sound.h"
#include "smooth.h"
#include "r_fps.h"
#include "r_main.h"
#include "r_segs.h"
#include "f_finale.h"
#include "e6y.h"//e6y

#include "dsda/episode.h"
#include "dsda/exhud.h"
#include "dsda/features.h"
#include "dsda/font.h"
#include "dsda/game_controller.h"
#include "dsda/global.h"
#include "dsda/mapinfo.h"
#include "dsda/messenger.h"
#include "dsda/settings.h"
#include "dsda/key_frame.h"
#include "dsda/input.h"
#include "dsda/palette.h"
#include "dsda/save.h"
#include "dsda/skill_info.h"
#include "dsda/skip.h"
#include "dsda/time.h"
#include "dsda/console.h"
#include "dsda/stretch.h"
#include "dsda/text_color.h"
#include "dsda/utility.h"
#include "dsda/wad_stats.h"

#include "heretic/mn_menu.h"
#include "heretic/sb_bar.h"

/****************************
 *
 *  The following #defines are for the m_flags field of each item on every
 *  Setup Screen. They can be OR'ed together where appropriate
 */

#define S_HILITE   0x00000001 // Cursor is sitting on this item
#define S_SELECT   0x00000002 // We're changing this item
#define S_TITLE    0x00000004 // Title item
#define S_YESNO    0x00000008 // Yes or No item
#define S_CRITEM   0x00000010 // Message color
#define S_COLOR    0x00000020 // Automap color
#define S_LABEL    0x00000040
#define S_TC_SEL   0x00000080
#define S_PREV     0x00000100 // Previous menu exists
#define S_NEXT     0x00000200 // Next menu exists
#define S_INPUT    0x00000400 // Composite input binding
#define S_WEAP     0x00000800 // Weapon #
#define S_NUM      0x00001000 // Numerical item
#define S_SKIP     0x00002000 // Cursor can't land here
#define S_KEEP     0x00004000 // Don't swap key out
#define S_END      0x00008000 // Last item in list (dummy)
#define S_LEVWARN  0x00010000 // killough 8/30/98: Always warn about pending change
#define S_NOSELECT 0x00020000
#define S_CENTER   0x00040000
#define S_FILE     0x00080000 // killough 10/98: Filenames
#define S_LEFTJUST 0x00100000 // killough 10/98: items which are left-justified
#define S_CREDIT   0x00200000 // killough 10/98: credit
// #define S_      0x00400000
#define S_CHOICE   0x00800000 // this item has several values
// #define S_      0x01000000
#define S_NAME     0x02000000
#define S_RESET_Y  0x04000000
// #define S_      0x08000000
// #define S_      0x10000000
// #define S_      0x20000000
#define S_STR      0x40000000 // need to refactor things...
#define S_NOCLEAR  0x80000000

/* S_SHOWDESC  = the set of items whose description should be displayed
 * S_SHOWSET   = the set of items whose setting should be displayed
 * S_STRING    = the set of items whose settings are strings -- killough 10/98:
 * S_HASDEFPTR = the set of items whose var field points to default array
 */

#define S_SHOWDESC (S_LABEL|S_TITLE|S_YESNO|S_CRITEM|S_COLOR|S_PREV|S_NEXT|S_INPUT|S_WEAP|S_NUM|S_FILE|S_CREDIT|S_CHOICE|S_NAME)

#define S_SHOWSET  (S_YESNO|S_CRITEM|S_COLOR|S_INPUT|S_WEAP|S_NUM|S_FILE|S_CHOICE|S_NAME)

#define S_STRING (S_FILE|S_NAME)

#define S_HASDEFPTR (S_STRING|S_YESNO|S_NUM|S_WEAP|S_COLOR|S_CRITEM|S_CHOICE)

extern dboolean  message_dontfuckwithme;

extern const char* g_menu_flat;
extern int g_menu_save_page_size;
extern int g_menu_font_spacing;

//
// defaulted values
//

int hide_setup=1; // killough 5/15/98

#define QUICKSAVESLOT 999

int messageToPrint;  // 1 = message to be printed

// CPhipps - static const
static const char* messageString; // ...and here is the message string!

// message x & y
int     messx;
int     messy;
int     messageLastMenuActive;

dboolean messageNeedsInput; // timed message = no input from user

void (*messageRoutine)(int response);

static void M_DrawBackground(const char *flat, int scrn)
{
  if (dsda_IntConfig(dsda_config_menu_background))
    V_DrawBackground(flat, scrn);
}

// we are going to be entering a savegame string

int saveStringEnter;
int saveSlot;        // which slot to save in
int saveCharIndex;   // which char we're editing
// old save description before edit
char saveOldString[SAVESTRINGSIZE];

dboolean inhelpscreens; // indicates we are in or just left a help screen

dboolean BorderNeedRefresh;

menuactive_t menuactive;    // The menus are up

#define SKULLXOFF  -32
#define LINEHEIGHT  16

char savegamestrings[10][SAVESTRINGSIZE];

short itemOn;           // menu item skull is on (for Big Font menus)
short skullAnimCounter; // skull animation counter
short whichSkull;       // which skull to draw (he blinks)

// graphic name of skulls

const char skullName[2][/*8*/9] = {"M_SKULL1","M_SKULL2"};

menu_t* currentMenu; // current menudef

// phares 3/30/98
// externs added for setup menus

int mapcolor_me;    // cph

// end of externs added for setup menus

//
// PROTOTYPES
//
void M_NewGame(int choice);
void M_Episode(int choice);
void M_ChooseSkill(int choice);
void M_LoadGame(int choice);
void M_SaveGame(int choice);
void M_Options(int choice);
void M_EndGame(int choice);
void M_ReadThis(int choice);
void M_ReadThis2(int choice);
void M_QuitDOOM(int choice);

void M_ChangeSensitivity(int choice);
void M_SfxVol(int choice);
void M_MusicVol(int choice);
/* void M_ChangeDetail(int choice);  unused -- killough */
void M_SizeDisplay(int choice);
void M_StartGame(int choice);
void M_Sound(int choice);

void M_FinishReadThis(int choice);
void M_FinishHelp(int choice);            // killough 10/98
void M_LoadSelect(int choice);
void M_SaveSelect(int choice);
void M_ReadSaveStrings(void);
void M_QuickSave(void);
void M_QuickLoad(void);

void M_DrawMainMenu(void);
void M_DrawReadThis1(void);
void M_DrawReadThis2(void);
void M_DrawSkillMenu(void);
void M_DrawEpisode(void);
void M_DrawOptions(void);
void M_DrawSound(void);
void M_DrawLoad(void);
void M_DrawSave(void);
void M_DrawHelp (void);                                     // phares 5/04/98

void M_DrawSaveLoadBorder(int x,int y);
void M_DrawThermo(int x,int y,int thermWidth,int thermDot);
void M_DrawEmptyCell(menu_t *menu,int item);
void M_DrawSelCell(menu_t *menu,int item);
void M_WriteText(int x, int y, const char *string, int cm);
int  M_StringWidth(const char *string);
int  M_StringHeight(const char *string);
void M_DrawTitle(int x, int y, const char *patch, int cm,
                 const char *alttext, int altcm);
void M_StartMessage(const char *string,void *routine,dboolean input);
void M_StopMessage(void);
void M_ChangeMenu(menu_t *menu, menuactive_t mnact);
void M_ClearMenus (void);

// phares 3/30/98
// prototypes added to support Setup Menus and Extended HELP screens

int  M_GetKeyString(int,int);
void M_KeyBindings(int choice);
void M_Weapons(int);
void M_StatusBar(int);
void M_Automap(int);
void M_InitExtendedHelp(void);
void M_ExtHelpNextScreen(int);
void M_ExtHelp(int);
static int M_GetPixelWidth(const char*);
void M_DrawKeybnd(void);
void M_DrawWeapons(void);
static void M_DrawString(int cx, int cy, int color, const char* ch);
static void M_DrawMenuString(int,int,int);
static void M_DrawStringCentered(int,int,int,const char*);
void M_DrawStatusHUD(void);
void M_DrawExtHelp(void);
void M_DrawAutoMap(void);
void M_ChangeDemoSmoothTurns(void);
void M_ChangeTextureParams(void);
void M_General(int);      // killough 10/98
void M_DrawGeneral(void); // killough 10/98
void M_LevelTable(int);
void M_DrawLevelTable(void);
void M_ChangeFullScreen(void);
void M_ChangeVideoMode(void);
void M_ChangeUseGLSurface(void);
void M_ChangeApplyPalette(void);

menu_t SkillDef;                                              // phares 5/04/98

// end of prototypes added to support Setup Menus and Extended HELP screens

const char shiftxform[] =
{
  0,
  1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
  11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
  21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
  31,
  ' ', '!', '"', '#', '$', '%', '&',
  '"', // shift-'
  '(', ')', '*', '+',
  '<', // shift-,
  '_', // shift--
  '>', // shift-.
  '?', // shift-/
  ')', // shift-0
  '!', // shift-1
  '@', // shift-2
  '#', // shift-3
  '$', // shift-4
  '%', // shift-5
  '^', // shift-6
  '&', // shift-7
  '*', // shift-8
  '(', // shift-9
  ':',
  ':', // shift-;
  '<',
  '+', // shift-=
  '>', '?', '@',
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
  'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
  '[', // shift-[
  '!', // shift-backslash - OH MY GOD DOES WATCOM SUCK
  ']', // shift-]
  '"', '_',
  '\'', // shift-`
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
  'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
  '{', '|', '}', '~', 127
};

static int cr_title;
static int cr_label;
static int cr_label_highlight;
static int cr_label_edit;
static int cr_value;
static int cr_value_highlight;
static int cr_value_edit;
static int cr_info_highlight;
static int cr_info_edit;
static int cr_warning;

static void M_LoadTextColors(void)
{
  cr_title = dsda_TextCR(dsda_tc_menu_title);
  cr_label = dsda_TextCR(dsda_tc_menu_label);
  cr_label_highlight = dsda_TextCR(dsda_tc_menu_label_highlight);
  cr_label_edit = dsda_TextCR(dsda_tc_menu_label_edit);
  cr_value = dsda_TextCR(dsda_tc_menu_value);
  cr_value_highlight = dsda_TextCR(dsda_tc_menu_value_highlight);
  cr_value_edit = dsda_TextCR(dsda_tc_menu_value_edit);
  cr_info_highlight = dsda_TextCR(dsda_tc_menu_info_highlight);
  cr_info_edit = dsda_TextCR(dsda_tc_menu_info_edit);
  cr_warning = dsda_TextCR(dsda_tc_menu_warning);
}

static const dsda_font_t *menu_font;

static void M_LoadMenuFont(void)
{
  menu_font = raven ? &exhud_font : &hud_font;
}

/////////////////////////////////////////////////////////////////////////////
//
// DOOM MENUS
//

/////////////////////////////
//
// MAIN MENU
//

// main_e provides numerical values for which Big Font screen you're on

enum
{
  newgame = 0,
  loadgame,
  savegame,
  options,
  readthis,
  quitdoom,
  main_end
} main_e;

//
// MainMenu is the definition of what the main menu Screen should look
// like. Each entry shows that the cursor can land on each item (1), the
// built-in graphic lump (i.e. "M_NGAME") that should be displayed,
// the program which takes over when an item is selected, and the hotkey
// associated with the item.
//

menuitem_t MainMenu[]=
{
  {1,"M_NGAME", M_NewGame, 'n', "NEW GAME"},
  {1,"M_OPTION",M_Options, 'o', "OPTIONS"},
  {1,"M_LOADG", M_LoadGame,'l', "LOAD GAME"},
  {1,"M_SAVEG", M_SaveGame,'s', "SAVE GAME"},
  {1,"M_RDTHIS",M_ReadThis,'r', "READ THIS"},
  {1,"M_QUITG", M_QuitDOOM,'q', "QUIT GAME"}
};

menu_t MainDef =
{
  main_end,       // number of menu items
  NULL,           // previous menu screen
  MainMenu,       // table that defines menu items
  M_DrawMainMenu, // drawing routine
  97,64,          // initial cursor position
  0               // last menu item the user was on
};

//
// M_DrawMainMenu
//

void M_DrawMainMenu(void)
{
  if (raven) return MN_DrawMainMenu();

  // CPhipps - patch drawing updated
  V_DrawNamePatch(94, 2, 0, "M_DOOM", CR_DEFAULT, VPT_STRETCH);
}

/////////////////////////////
//
// Read This! MENU 1 & 2
//

// There are no menu items on the Read This! screens, so read_e just
// provides a placeholder to maintain structure.

enum
{
  rdthsempty1,
  read1_end
} read_e;

enum
{
  rdthsempty2,
  read2_end
} read_e2;

enum               // killough 10/98
{
  helpempty,
  help_end
} help_e;


// The definitions of the Read This! screens

menuitem_t ReadMenu1[] =
{
  {1,"",M_ReadThis2,0}
};

menuitem_t ReadMenu2[]=
{
  {1,"",M_FinishReadThis,0}
};

menuitem_t HelpMenu[]=    // killough 10/98
{
  {1,"",M_FinishHelp,0}
};

menu_t ReadDef1 =
{
  read1_end,
  &MainDef,
  ReadMenu1,
  M_DrawReadThis1,
  330,175,
  //280,185,              // killough 2/21/98: fix help screens
  0
};

menu_t ReadDef2 =
{
  read2_end,
  &ReadDef1,
  ReadMenu2,
  M_DrawReadThis2,
  330,175,
  0
};

menu_t HelpDef =           // killough 10/98
{
  help_end,
  &HelpDef,
  HelpMenu,
  M_DrawHelp,
  330,175,
  0
};

//
// M_ReadThis
//

void M_ReadThis(int choice)
{
  M_SetupNextMenu(&ReadDef1);
}

void M_ReadThis2(int choice)
{
  M_SetupNextMenu(&ReadDef2);
}

void M_FinishReadThis(int choice)
{
  M_SetupNextMenu(&MainDef);
}

void M_FinishHelp(int choice)        // killough 10/98
{
  M_SetupNextMenu(&MainDef);
}

//
// Read This Menus
// Had a "quick hack to fix romero bug"
//
// killough 10/98: updated with new screens

void M_DrawReadThis1(void)
{
  inhelpscreens = true;
  if (hexen)
  {
    V_DrawRawScreen("HELP2");
  }
  else if (gamemode == shareware)
  {
    // e6y: wide-res
    V_ClearBorder();
    V_DrawNamePatch(0, 0, 0, "HELP2", CR_DEFAULT, VPT_STRETCH);
  }
  else
    M_DrawCredits();
}

//
// Read This Menus - optional second page.
//
// killough 10/98: updated with new screens

void M_DrawReadThis2(void)
{
  inhelpscreens = true;
  M_DrawCredits();
}

/////////////////////////////
//
// EPISODE SELECT
//

// The definitions of the Episodes menu

menu_t EpiDef =
{
  .prevMenu = &MainDef,
  .routine = M_DrawEpisode,
  .x = 48,
  .y = 63,
};

//
//    M_Episode
//

static int chosen_episode;

void M_DrawEpisode(void)
{
  if (raven) return MN_DrawEpisode();

  // CPhipps - patch drawing updated
  V_DrawNamePatch(54, EpiDef.y - 25, 0, "M_EPISOD", CR_DEFAULT, VPT_STRETCH);
}

void M_Episode(int choice)
{
  if (gamemode == shareware && choice && !episodes[choice].vanilla) {
    M_StartMessage(s_SWSTRING, NULL, false); // Ty 03/27/98 - externalized
    M_SetupNextMenu(&ReadDef1);
    return;
  }

  chosen_episode = choice;

  if (hexen) // hack hexen class as "episode menu"
    MN_UpdateClass(chosen_episode);

  M_SetupNextMenu(&SkillDef);
}

/////////////////////////////
//
// SKILL SELECT
//

// The definitions of the Skill Select menu

menu_t SkillDef =
{
  .prevMenu = &EpiDef,
  .routine = M_DrawSkillMenu,
  .x = 48,
  .y = 63,
};

//
// M_NewGame
//

void M_DrawSkillMenu(void)
{
  if (raven) return MN_DrawSkillMenu();

  // CPhipps - patch drawing updated
  V_DrawNamePatch(96, 14, 0, "M_NEWG", CR_DEFAULT, VPT_STRETCH);
  V_DrawNamePatch(54, 38, 0, "M_SKILL",CR_DEFAULT, VPT_STRETCH);
}

void M_NewGame(int choice)
{
  if (demorecording) {  /* killough 5/26/98: exclude during demo recordings */
    M_StartMessage("you can't start a new game\n"
       "while recording a demo!\n\n"PRESSKEY,
       NULL, false); // killough 5/26/98: not externalized
    return;
  }

  // Chex Quest disabled the episode select screen, as did Doom II.
  if (num_episodes <= 1 && !hexen)
    M_SetupNextMenu(&SkillDef);
  else
    M_SetupNextMenu(&EpiDef);
}

static int chosen_skill;

static void M_FinishGameSelection(void)
{
  int episode, map;

  if (num_episodes)
  {
    episode = episodes[chosen_episode].start_episode;
    map = episodes[chosen_episode].start_map;
  }
  else
  {
    episode = 1;
    map = 1;
  }

  G_DeferedInitNew(chosen_skill, episode, map);

  if (hexen)
    SB_SetClassData();

  M_ClearMenus();
}

// CPhipps - static
static void M_VerifySkill(dboolean affirmative)
{
  if (!affirmative)
    return;

  M_FinishGameSelection();
}

void M_ChooseSkill(int choice)
{
  extern skill_info_t *skill_infos;

  chosen_skill = choice;

  if (choice < num_skills && skill_infos[choice].flags & SI_MUST_CONFIRM)
  {
    const char* message;

    if (skill_infos[choice].must_confirm)
      message = skill_infos[choice].must_confirm;
    else
      message = s_NIGHTMARE; // Ty 03/27/98 - externalized

    M_StartMessage(message, M_VerifySkill, true);

    return;
  }

  M_FinishGameSelection();
}

/////////////////////////////
//
// LOAD GAME MENU
//

// numerical values for the Load Game slots

enum
{
  load1,
  load2,
  load3,
  load4,
  load5,
  load6,
  load7,
  load_end
} load_e;

static int save_page = 0;
static const int save_page_limit = 16;

#define SAVE_PAGE_STRING_SIZE 16
char save_page_string[SAVE_PAGE_STRING_SIZE];

// The definitions of the Load Game screen

menuitem_t LoadMenue[]=
{
  {1,"", M_LoadSelect,'1'},
  {1,"", M_LoadSelect,'2'},
  {1,"", M_LoadSelect,'3'},
  {1,"", M_LoadSelect,'4'},
  {1,"", M_LoadSelect,'5'},
  {1,"", M_LoadSelect,'6'},
  {1,"", M_LoadSelect,'7'}, //jff 3/15/98 extend number of slots
  {1,"", M_LoadSelect,'8'},
};

menu_t LoadDef =
{
  load_end,
  &MainDef,
  LoadMenue,
  M_DrawLoad,
  80,34, //jff 3/15/98 move menu up
  0
};

#define LOADGRAPHIC_Y 8

// [FG] delete a savegame

dboolean delete_verify = false;

static void M_DeleteGame(int slot)
{
  char *name;

  if (dsda_LastSaveSlot() == slot)
    dsda_ResetLastSaveSlot();

  name = dsda_SaveGameName(slot + save_page * g_menu_save_page_size, false);
  remove(name);
  Z_Free(name);

  M_ReadSaveStrings();
}

//
// M_LoadGame & Cie.
//

void M_DrawLoad(void)
{
  int i;

  if (raven) return MN_DrawLoad();

  //jff 3/15/98 use symbolic load position
  // CPhipps - patch drawing updated
  V_DrawNamePatch(72 ,LOADGRAPHIC_Y, 0, "M_LOADG", CR_DEFAULT, VPT_STRETCH);
  for (i = 0 ; i < load_end ; i++) {
    M_DrawSaveLoadBorder(LoadDef.x,LoadDef.y+LINEHEIGHT*i);
    M_WriteText(LoadDef.x,LoadDef.y+LINEHEIGHT*i,savegamestrings[i], CR_DEFAULT);
  }

  M_WriteText(LoadDef.x, LoadDef.y + LINEHEIGHT * load_end, save_page_string, CR_DEFAULT);

  if (delete_verify)
    M_DrawDelVerify();
}

//
// Draw border for the savegame description
//

void M_DrawSaveLoadBorder(int x,int y)
{
  int i;

  V_DrawNamePatch(x-8, y+7, 0, "M_LSLEFT", CR_DEFAULT, VPT_STRETCH);

  for (i = 0 ; i < 24 ; i++)
    {
      V_DrawNamePatch(x, y+7, 0, "M_LSCNTR", CR_DEFAULT, VPT_STRETCH);
      x += 8;
    }

  V_DrawNamePatch(x, y+7, 0, "M_LSRGHT", CR_DEFAULT, VPT_STRETCH);
}

//
// User wants to load this game
//

void M_LoadSelect(int choice)
{
  if (!dsda_AllowMenuLoad(choice + save_page * g_menu_save_page_size))
  {
    M_StartMessage(
      "you can't load this game\n"
      "under these conditions!\n\n"PRESSKEY,
      NULL, false); // killough 5/26/98: not externalized
    return;
  }

  // CPhipps - Modified so savegame filename is worked out only internal
  //  to g_game.c, this only passes the slot.

  // killough 3/16/98, 5/15/98: add slot, cmd
  G_LoadGame(choice + save_page * g_menu_save_page_size);
  M_ClearMenus();
}

//
// killough 5/15/98: add forced loadgames
//

static char *forced_loadgame_message;

static void M_VerifyForcedLoadGame(int ch)
{
  if (ch=='y')
    G_ForcedLoadGame();
  Z_Free(forced_loadgame_message);    // free the message Z_Strdup()'ed below
  M_ClearMenus();
}

void M_ForcedLoadGame(const char *msg)
{
  forced_loadgame_message = Z_Strdup(msg); // Z_Free()'d above
  M_StartMessage(forced_loadgame_message, M_VerifyForcedLoadGame, true);
}

//
// Selected from DOOM menu
//

void M_LoadGame (int choice)
{
  delete_verify = false;

  if (!dsda_AllowAnyMenuLoad())
  {
    M_StartMessage(
      "you can't load a game\n"
      "under these conditions!\n\n"PRESSKEY,
      NULL, false); // killough 5/26/98: not externalized
    return;
  }

  M_SetupNextMenu(&LoadDef);
  M_ReadSaveStrings();
}

/////////////////////////////
//
// SAVE GAME MENU
//

// The definitions of the Save Game screen

menuitem_t SaveMenu[]=
{
  {1,"", M_SaveSelect,'1'},
  {1,"", M_SaveSelect,'2'},
  {1,"", M_SaveSelect,'3'},
  {1,"", M_SaveSelect,'4'},
  {1,"", M_SaveSelect,'5'},
  {1,"", M_SaveSelect,'6'},
  {1,"", M_SaveSelect,'7'}, //jff 3/15/98 extend number of slots
  {1,"", M_SaveSelect,'8'},
};

menu_t SaveDef =
{
  load_end, // same number of slots as the Load Game screen
  &MainDef,
  SaveMenu,
  M_DrawSave,
  80,34, //jff 3/15/98 move menu up
  0
};

//
// M_ReadSaveStrings
//  read the strings from the savegame files
//
void M_ReadSaveStrings(void)
{
  int i;

  for (i = 0 ; i < load_end ; i++) {
    char *name;               // killough 3/22/98
    dg_file_t *fp;  // killough 11/98: change to use stdio

    // killough 3/22/98
    name = dsda_SaveGameName(i + save_page * g_menu_save_page_size, false);

    fp = M_OpenFile(name,"rb");
    Z_Free(name);

    if (!fp || !DG_read(fp, &savegamestrings[i], SAVESTRINGSIZE))
    {
      strcpy(&savegamestrings[i][0],s_EMPTYSTRING); // Ty 03/27/98 - externalized
      LoadMenue[i].status = 0;
    }
    else
    {
      LoadMenue[i].status = 1;
    }

    if (fp)
    {
      DG_close(fp);
    }
  }

  snprintf(save_page_string, SAVE_PAGE_STRING_SIZE, "PAGE %d/%d", save_page + 1, save_page_limit);
}

//
//  M_SaveGame & Cie.
//
void M_DrawSave(void)
{
  int i;

  if (raven) return MN_DrawSave();

  //jff 3/15/98 use symbolic load position
  // CPhipps - patch drawing updated
  V_DrawNamePatch(72, LOADGRAPHIC_Y, 0, "M_SAVEG", CR_DEFAULT, VPT_STRETCH);
  for (i = 0 ; i < load_end ; i++)
    {
    M_DrawSaveLoadBorder(LoadDef.x,LoadDef.y+LINEHEIGHT*i);
    M_WriteText(LoadDef.x,LoadDef.y+LINEHEIGHT*i,savegamestrings[i], CR_DEFAULT);
    }

  M_WriteText(LoadDef.x, LoadDef.y + LINEHEIGHT * load_end, save_page_string, CR_DEFAULT);

  if (saveStringEnter)
    {
    i = M_StringWidth(savegamestrings[saveSlot]);
    M_WriteText(LoadDef.x + i,LoadDef.y+LINEHEIGHT*saveSlot,"_", CR_DEFAULT);
    }

  if (delete_verify)
    M_DrawDelVerify();
}

//
// M_Responder calls this when user is finished
//
static void M_DoSave(int slot)
{
  G_SaveGame(slot + save_page * g_menu_save_page_size, savegamestrings[slot]);
  M_ClearMenus();
}

//
// User wants to save. Start string input for M_Responder
//
static inline dboolean IsMapName(char *str)
{
    if (strlen(str) == 4 &&
        str[0] == 'E' && isdigit(str[1]) &&
        str[2] == 'M' && isdigit(str[3]))
    {
        return true;
    }

    if (strlen(str) == 5 &&
        str[0] == 'M' && str[1] == 'A' && str[2] == 'P' &&
        isdigit(str[3]) && isdigit(str[4]))
    {
        return true;
    }

    return false;
}

void M_SaveSelect(int choice)
{
  // we are going to be intercepting all chars
  saveStringEnter = 1;

  saveSlot = choice;
  strcpy(saveOldString,savegamestrings[choice]);
  if (!strcmp(savegamestrings[choice],s_EMPTYSTRING) || // Ty 03/27/98 - externalized
      IsMapName(savegamestrings[choice]))
  {
    snprintf(savegamestrings[choice], SAVESTRINGSIZE, "%s", dsda_MapLumpName(gameepisode, gamemap));
    savegamestrings[choice][SAVESTRINGSIZE - 1] = 0;
  }
  saveCharIndex = strlen(savegamestrings[choice]);
}

//
// Selected from DOOM menu
//
void M_SaveGame (int choice)
{
  delete_verify = false;

  if (gamestate != GS_LEVEL)
    return;

  M_SetupNextMenu(&SaveDef);
  M_ReadSaveStrings();
}

/////////////////////////////
//
// OPTIONS MENU
//

// numerical values for the Options menu items

enum
{
  general, // killough 10/98
  set_key_bindings,
  set_weapons,
  set_statbar,
  set_automap,
  soundvol,
  level_table,
  opt_end
} options_e;

// The definitions of the Options menu

menuitem_t OptionsMenu[]=
{
  { 1, "M_GENERL", M_General, 'g', "GENERAL" }, // killough 10/98
  { 1, "M_KEYBND", M_KeyBindings,'k', "KEY BINDINGS" },
  { 1, "M_WEAP", M_Weapons, 'w', "WEAPONS" },
  { 1, "M_STAT", M_StatusBar, 's', "STATUS BAR / HUD" },
  { 1, "M_AUTO", M_Automap, 'a', "AUTOMAP" },
  { 1, "M_SVOL", M_Sound, 's', "SOUND VOLUME" },
  { 1, "M_LVLTBL", M_LevelTable, 's', "LEVEL TABLE" },
};

menu_t OptionsDef =
{
  opt_end,
  &MainDef,
  OptionsMenu,
  M_DrawOptions,
  60,37,
  0
};

//
// M_Options
//

void M_DrawOptions(void)
{
  if (raven) return MN_DrawOptions();

  // CPhipps - patch drawing updated
  // proff/nicolas 09/20/98 -- changed for hi-res
  V_DrawNamePatch(108, 15, 0, "M_OPTTTL", CR_DEFAULT, VPT_STRETCH);
}

void M_Options(int choice)
{
  M_SetupNextMenu(&OptionsDef);
}

/////////////////////////////
//
// M_QuitDOOM
//
int quitsounds[8] =
{
  sfx_pldeth,
  sfx_dmpain,
  sfx_popain,
  sfx_slop,
  sfx_telept,
  sfx_posit1,
  sfx_posit3,
  sfx_sgtatk
};

int quitsounds2[8] =
{
  sfx_vilact,
  sfx_getpow,
  sfx_boscub,
  sfx_slop,
  sfx_skeswg,
  sfx_kntdth,
  sfx_bspact,
  sfx_sgtatk
};

static void M_QuitResponse(dboolean affirmative)
{
  if (!affirmative)
    return;

  //e6y: I_SafeExit instead of exit - prevent recursive exits
  I_SafeExit(0); // killough
}

void M_QuitDOOM(int choice)
{
  static char endstring[160];

  // We pick index 0 which is language sensitive,
  // or one at random, between 1 and maximum number.
  // Ty 03/27/98 - externalized DOSY as a string s_DOSY that's in the sprintf
  if (language != english)
    sprintf(endstring,"%s\n\n%s",s_DOSY, endmsg[0] );
  else         // killough 1/18/98: fix endgame message calculation:
    sprintf(endstring,"%s\n\n%s", endmsg[gametic%(NUM_QUITMESSAGES-1)+1], s_DOSY);

  if (dsda_SkipQuitPrompt())
    M_QuitResponse(true);
  else
    M_StartMessage(endstring,M_QuitResponse,true);
}

/////////////////////////////
//
// SOUND VOLUME MENU
//

// numerical values for the Sound Volume menu items
// The 'empty' slots are where the sliding scales appear.

enum
{
  sfx_vol,
  sfx_empty1,
  music_vol,
  sfx_empty2,
  sound_end
} sound_e;

// The definitions of the Sound Volume menu

menuitem_t SoundMenu[]=
{
  {2,"M_SFXVOL",M_SfxVol,'s'},
  {-1,"",0},
  {2,"M_MUSVOL",M_MusicVol,'m'},
  {-1,"",0}
};

menu_t SoundDef =
{
  sound_end,
  &OptionsDef,
  SoundMenu,
  M_DrawSound,
  80,64,
  0
};

//
// Change Sfx & Music volumes
//

void M_DrawSound(void)
{
  if (raven) return MN_DrawSound();

  // CPhipps - patch drawing updated
  V_DrawNamePatch(60, 38, 0, "M_SVOL", CR_DEFAULT, VPT_STRETCH);

  M_DrawThermo(SoundDef.x,SoundDef.y+LINEHEIGHT*(sfx_vol+1),16,snd_SfxVolume);

  M_DrawThermo(SoundDef.x,SoundDef.y+LINEHEIGHT*(music_vol+1),16,snd_MusicVolume);
}

void M_Sound(int choice)
{
  M_SetupNextMenu(&SoundDef);
}

void M_SfxVol(int choice)
{
  switch(choice)
  {
    case 0:
      if (dsda_IntConfig(dsda_config_sfx_volume) > 0)
        dsda_DecrementIntConfig(dsda_config_sfx_volume, true);
      break;
    case 1:
      dsda_IncrementIntConfig(dsda_config_sfx_volume, true);
      break;
  }

  // Unmute the sfx if we are adjusting the volume
  if (dsda_MuteSfx())
    dsda_ToggleConfig(dsda_config_mute_sfx, true);
}

void M_MusicVol(int choice)
{
  switch(choice)
  {
    case 0:
      if (dsda_IntConfig(dsda_config_music_volume) > 0)
        dsda_DecrementIntConfig(dsda_config_music_volume, true);
      break;
    case 1:
      dsda_IncrementIntConfig(dsda_config_music_volume, true);
      break;
  }

  // Unmute the music if we are adjusting the volume
  if (dsda_MuteMusic())
    dsda_ToggleConfig(dsda_config_mute_music, true);
}

/////////////////////////////
//
//    M_QuickSave
//

void M_QuickSave(void)
{
  if (gamestate != GS_LEVEL)
    return;

  G_SaveGame(QUICKSAVESLOT, "quicksave");
  doom_printf("quicksave");
}

/////////////////////////////
//
// M_QuickLoad
//

void M_QuickLoad(void)
{
  char *name;

  if (!dsda_AllowAnyMenuLoad())
  {
    M_StartMessage(
      "you can't load a game\n"
      "under these conditions!\n\n"PRESSKEY,
      NULL, false); // killough 5/26/98: not externalized
    return;
  }

  if (!dsda_AllowMenuLoad(QUICKSAVESLOT))
  {
    M_StartMessage(
      "you can't load this game\n"
      "under these conditions!\n\n"PRESSKEY,
      NULL, false); // killough 5/26/98: not externalized
    return;
  }

  name = dsda_SaveGameName(QUICKSAVESLOT, false);

  if (M_FileExists(name))
  {
    G_LoadGame(QUICKSAVESLOT);
    doom_printf("quickload");
  }
  else
  {
    doom_printf("no save file");
  }

  Z_Free(name);
}

/////////////////////////////
//
// M_EndGame
//

static void M_EndGameResponse(dboolean affirmative)
{
  if (!affirmative)
    return;

  // killough 5/26/98: make endgame quit if recording or playing back demo
  if (demorecording || userplayback)
    G_CheckDemoStatus();

  currentMenu->lastOn = itemOn;
  M_ClearMenus ();
  D_StartTitle ();
}

void M_EndGame(int choice)
{
  if (netgame)
    {
    M_StartMessage(s_NETEND,NULL,false); // Ty 03/27/98 - externalized
    return;
    }
  M_StartMessage(s_ENDGAME,M_EndGameResponse,true); // Ty 03/27/98 - externalized
}

void M_ChangeMessages(void)
{
  if (!dsda_ShowMessages())
    dsda_AddUnblockableMessage(s_MSGOFF);
  else
    dsda_AddMessage(s_MSGON);
}

/////////////////////////////
//
// CHANGE DISPLAY SIZE
//
// jff 2/23/98 restored to pre-HUD state
// hud_active controlled soley by F5=key_detail (key_hud)
// hud_displayed is toggled by + or = in fullscreen
// hud_displayed is cleared by -

void M_SizeDisplay(int choice)
{
  switch(choice) {
    case 0:
      if (R_FullView())
        dsda_DecrementIntConfig(dsda_config_screenblocks, true);
      break;
    case 1:
      if (R_PartialView())
        dsda_IncrementIntConfig(dsda_config_screenblocks, true);
      else
        dsda_ToggleConfig(dsda_config_hud_displayed, true);
      break;
    case 2:
      if (R_PartialView()) {
        dsda_UpdateIntConfig(dsda_config_screenblocks, 11, true);
        dsda_UpdateIntConfig(dsda_config_hud_displayed, true, true);
      }
      else {
        dsda_ToggleConfig(dsda_config_hud_displayed, true);
        if (dsda_IntConfig(dsda_config_hud_displayed))
          dsda_DecrementIntConfig(dsda_config_screenblocks, true);
      }
      break;
  }
}

//
// End of Original Menus
//
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//
// SETUP MENU (phares)
//
// We've added a set of Setup Screens from which you can configure a number
// of variables w/o having to restart the game. There are 7 screens:
//
//    Key Bindings
//    Weapons
//    Status Bar / HUD
//    Automap
//
// killough 10/98: added Compatibility and General menus
//

/////////////////////////////
//
// booleans for setup screens
// these tell you what state the setup screens are in, and whether any of
// the overlay screens (automap colors, reset button message) should be
// displayed

dboolean setup_active      = false; // in one of the setup screens
dboolean set_keybnd_active = false; // in key binding setup screens
dboolean set_weapon_active = false; // in weapons setup screen
dboolean set_status_active = false; // in status bar/hud setup screen
dboolean set_auto_active   = false; // in automap setup screen
dboolean setup_select      = false; // changing an item
dboolean setup_gather      = false; // gathering keys for value
dboolean colorbox_active   = false; // color palette being shown
dboolean set_general_active = false;
dboolean level_table_active = false;

/////////////////////////////
//
// set_menu_itemon is an index that starts at zero, and tells you which
// item on the current screen the cursor is sitting on.
//
// current_setup_menu is a pointer to the current setup menu table.

static int set_menu_itemon; // which setup item is selected?   // phares 3/98
static setup_menu_t* current_setup_menu; // points to current setup menu table

// save the setup menu's itemon value in the S_END element's x coordinate

static int M_GetSetupMenuItemOn (void)
{
  const setup_menu_t* menu = current_setup_menu;

  if (menu)
  {
    while (!(menu->m_flags & S_END))
      menu++;

    return menu->m_x;
  }

  return 0;
}

static void M_SetSetupMenuItemOn (const int x)
{
  setup_menu_t* menu = current_setup_menu;

  if (menu)
  {
    while (!(menu->m_flags & S_END))
      menu++;

    menu->m_x = x;
  }
}

static void M_UpdateSetupMenu(setup_menu_t *new_setup_menu)
{
  current_setup_menu = new_setup_menu;
  set_menu_itemon = M_GetSetupMenuItemOn();
  if (current_setup_menu[set_menu_itemon].m_flags & S_NOSELECT)
    return;
  while (current_setup_menu[set_menu_itemon++].m_flags & S_SKIP);
  current_setup_menu[--set_menu_itemon].m_flags |= S_HILITE;
}

/////////////////////////////
//
// The menu_buffer is used to construct strings for display on the screen.

#define MENU_BUFFER_SIZE 128

static char menu_buffer[MENU_BUFFER_SIZE];

/////////////////////////////
//
// M_DoNothing does just that: nothing. Just a placeholder.

static void M_DoNothing(int choice)
{
}

/////////////////////////////
//
// Items needed to satisfy the 'Big Font' menu structures:
//
// the generic_setup_e enum mimics the 'Big Font' menu structures, but
// means nothing to the Setup Menus.

enum
{
  generic_setupempty1,
  generic_setup_end
} generic_setup_e;

// Generic_Setup is a do-nothing definition that the mainstream Menu code
// can understand, while the Setup Menu code is working. Another placeholder.

menuitem_t Generic_Setup[] =
{
  {1,"",M_DoNothing,0}
};

menu_t KeybndDef =
{
  generic_setup_end,
  &OptionsDef,
  Generic_Setup,
  M_DrawKeybnd,
  34,5,      // skull drawn here
  0
};

menu_t WeaponDef =
{
  generic_setup_end,
  &OptionsDef,
  Generic_Setup,
  M_DrawWeapons,
  34,5,      // skull drawn here
  0
};

menu_t StatusHUDDef =
{
  generic_setup_end,
  &OptionsDef,
  Generic_Setup,
  M_DrawStatusHUD,
  34,5,      // skull drawn here
  0
};

menu_t AutoMapDef =
{
  generic_setup_end,
  &OptionsDef,
  Generic_Setup,
  M_DrawAutoMap,
  34,5,      // skull drawn here
  0
};

menu_t GeneralDef =                                           // killough 10/98
{
  generic_setup_end,
  &OptionsDef,
  Generic_Setup,
  M_DrawGeneral,
  34,5,      // skull drawn here
  0
};

menu_t LevelTableDef =
{
  generic_setup_end,
  &OptionsDef,
  Generic_Setup,
  M_DrawLevelTable,
  34,5,      // skull drawn here
  0
};

// Data used by the Automap color selection code

#define CHIP_SIZE 7 // size of color block for colored items

#define COLORPALXORIG ((320 - 16*(CHIP_SIZE+1))/2)
#define COLORPALYORIG ((200 - 16*(CHIP_SIZE+1))/2)

#define PAL_BLACK   0
#define PAL_WHITE   4

// Data used by the string editing code

#define ENTRY_STRING_BFR_SIZE 128

// strings must fit in this screen space
// killough 10/98: reduced, for more general uses
#define MAXENTRYWIDTH         272

static int entry_index;
static char entry_string_index[ENTRY_STRING_BFR_SIZE]; // points to new strings while editing
static int choice_value;

/////////////////////////////
//
// phares 4/18/98:
// Consolidate Item drawing code
//
// M_DrawItem draws the description of the provided item (the left-hand
// part). A different color is used for the text depending on whether the
// item is selected or not, or whether it's about to change.

// CPhipps - static, hanging else removed, const parameter
static void M_DrawItem(const setup_menu_t* s, int y)
{
  int x = s->m_x;
  int flags = s->m_flags;
  char *p, *t;
  int w = 0;
  int color =
    flags & (S_SELECT|S_TC_SEL) ? cr_label_edit :
    flags & S_HILITE ? cr_label_highlight :
    flags & (S_TITLE|S_NEXT|S_PREV) ? cr_title :
    cr_label; // killough 10/98

  /* killough 10/98:
   * Enhance to support multiline text separated by newlines.
   * This supports multiline items on horizontally-crowded menus.
   */

  for (p = t = Z_Strdup(s->m_text); (p = strtok(p,"\n")); y += 8, p = NULL)
  {      /* killough 10/98: support left-justification: */
    strcpy(menu_buffer,p);
    if (flags & S_CENTER)
      w = M_GetPixelWidth(menu_buffer) / 2;
    else if (!(flags & S_LEFTJUST))
      w = M_GetPixelWidth(menu_buffer) + 4;
    M_DrawMenuString(x - w, y ,color);
    // print a blinking "arrow" next to the currently highlighted menu item
    if (s == current_setup_menu + set_menu_itemon && whichSkull && !(flags & S_NOSELECT))
      M_DrawString(x - w - 8, y, color, ">");
  }
  Z_Free(t);
}

// If a number item is being changed, allow up to N keystrokes to 'gather'
// the value. Gather_count tells you how many you have so far. The legality
// of what is gathered is determined by the low/high settings for the item.

#define MAXGATHER 5
int  gather_count;
char gather_buffer[MAXGATHER+1];  // killough 10/98: make input character-based

/////////////////////////////
//
// phares 4/18/98:
// Consolidate Item Setting drawing code
//
// M_DrawSetting draws the setting of the provided item (the right-hand
// part. It determines the text color based on whether the item is
// selected or being changed. Then, depending on the type of item, it
// displays the appropriate setting value: yes/no, a key binding, a number,
// a paint chip, etc.

static void M_DrawSetting(const setup_menu_t* s, int y)
{
  int x = s->m_x, flags = s->m_flags, color;

  // Determine color of the text. This may or may not be used later,
  // depending on whether the item is a text string or not.

  color =
    flags & S_SELECT ? cr_value_edit :
    flags & S_HILITE ? cr_value_highlight :
    cr_value;

  // Is the item a YES/NO item?

  if (flags & S_YESNO) {
    strcpy(menu_buffer, dsda_PersistentIntConfig(s->config_id) ? "YES" : "NO");

    if (s == current_setup_menu + set_menu_itemon && whichSkull && !setup_select)
      strcat(menu_buffer, " <");
    M_DrawMenuString(x,y,color);
    return;
  }

  // Is the item a simple number?

  if (flags & (S_NUM | S_WEAP | S_CRITEM)) {
    // killough 10/98: We must draw differently for items being gathered.
    if (flags & (S_HILITE | S_SELECT) && setup_gather) {
      gather_buffer[gather_count] = 0;
      strcpy(menu_buffer, gather_buffer);
    }
    else {
      int value;

      value = dsda_PersistentIntConfig(s->config_id);

      sprintf(menu_buffer, "%d", value);

      if (flags & S_CRITEM)
        color = value;
    }
    if (s == current_setup_menu + set_menu_itemon && whichSkull && !setup_select)
      strcat(menu_buffer, " <");
    M_DrawMenuString(x, y, color);
    return;
  }

  // Is the item a key binding?

  if (flags & S_INPUT) {
    int i;
    int offset = 0;
    const char* format;
    dboolean any_input = false;
    dsda_input_t* input;
    input = dsda_Input(s->input);

    // Draw the input bound to the action
    menu_buffer[0] = '\0';

    for (i = 0; i < input->num_keys; ++i)
    {
      if (any_input)
      {
        menu_buffer[offset++] = '/';
        menu_buffer[offset] = '\0';
      }

      offset = M_GetKeyString(input->key[i], offset);
      any_input = true;
    }

    if (input->mouseb != -1)
    {
      if (any_input)
        format = "/MB%d";
      else
        format = "MB%d";

      sprintf(menu_buffer + strlen(menu_buffer), format, input->mouseb + 1);
      any_input = true;
    }

#if 0
    if (input->joyb != -1)
    {
      if (any_input)
        format = "/%s";
      else
        format = "%s";

      sprintf(menu_buffer + strlen(menu_buffer), format,
              dsda_GameControllerButtonName(input->joyb));
      any_input = true;
    }
#endif

    // "NONE"
    if (!any_input)
      M_GetKeyString(0, 0);

    if (s == current_setup_menu + set_menu_itemon && whichSkull && !setup_select)
      strcat(menu_buffer, " <");

    M_DrawMenuString(x, y, color);

    return;
  }

  // Is the item a paint chip?

  if (flags & S_COLOR) // Automap paint chip
  {
    int ch;

    ch = dsda_PersistentIntConfig(s->config_id);
    // proff 12/6/98: Drawing of colorchips completly changed for hi-res, it now uses a patch
    // draw the paint chip
    // e6y: wide-res
    {
      int xx = x, yy = y - 1, ww = 8, hh = 8;
      V_GetWideRect(&xx, &yy, &ww, &hh, VPT_STRETCH);
      V_FillRect(0, xx, yy, ww, hh, PAL_BLACK);
      xx = x + 1, yy = y, ww = 6, hh = 6;
      V_GetWideRect(&xx, &yy, &ww, &hh, VPT_STRETCH);
      V_FillRect(0, xx, yy, ww, hh, (byte)ch);
    }

    if (!ch) // don't show this item in automap mode
      V_DrawNamePatch(x+1,y,0,"M_PALNO", CR_DEFAULT, VPT_STRETCH);
    if (s == current_setup_menu + set_menu_itemon && whichSkull && !setup_select)
      M_DrawString(x + 8, y, color, " <");
    return;
  }

  // Is the item a string?
  if (flags & S_STRING) {
    static char text[ENTRY_STRING_BFR_SIZE];

    // Are we editing this string? If so, display a cursor under
    // the correct character.
    if (setup_select && (s->m_flags & (S_HILITE|S_SELECT))) {
      int cursor_start, char_width;
      char c[2];

      strcpy(text, entry_string_index);

      // If the string is too wide for the screen, trim it back,
      // one char at a time until it fits. This should only occur
      // while you're editing the string.

      while (M_GetPixelWidth(text) >= MAXENTRYWIDTH) {
        int len = strlen(text);
        text[--len] = 0;
        if (entry_index > len)
          entry_index--;
      }

      // Find the distance from the beginning of the string to
      // where the cursor should be drawn, plus the width of
      // the char the cursor is under..

      *c = text[entry_index]; // hold temporarily
      c[1] = 0;
      char_width = M_GetPixelWidth(c);
      if (char_width == 1)
        char_width = 7; // default for end of line
      text[entry_index] = 0; // NULL to get cursor position
      cursor_start = M_GetPixelWidth(text);
      text[entry_index] = *c; // replace stored char

      // Now draw the cursor
      // proff 12/6/98: Drawing of cursor changed for hi-res
      // e6y: wide-res
      if (x + cursor_start + char_width < BASE_WIDTH)
      {
        int xx = (x+cursor_start-1), yy = y, ww = char_width, hh = 9;
        V_GetWideRect(&xx, &yy, &ww, &hh, VPT_STRETCH);
        V_FillRect(0, xx, yy, ww, hh, PAL_WHITE);
      }
    }
    else {
      strncpy(text, dsda_PersistentStringConfig(s->config_id), ENTRY_STRING_BFR_SIZE - 1);
    }

    // Draw the setting for the item

    strcpy(menu_buffer, text);
    if (s == current_setup_menu + set_menu_itemon && whichSkull && !setup_select)
      strcat(menu_buffer, " <");
    M_DrawMenuString(x, y, color);
    return;
  }

  // Is the item a selection of choices?

  if (flags & S_CHOICE) {
    if (flags & S_STR)
    {
      if (setup_select && (s->m_flags & (S_HILITE | S_SELECT)))
        sprintf(menu_buffer, "%s", entry_string_index);
      else
        sprintf(menu_buffer, "%s", dsda_PersistentStringConfig(s->config_id));
    }
    else
    {
      int value;

      if (setup_select && (s->m_flags & (S_HILITE | S_SELECT)))
        value = choice_value;
      else
        value = dsda_PersistentIntConfig(s->config_id);

      if (s->selectstrings == NULL) {
        sprintf(menu_buffer, "%d", value);
      } else {
        strcpy(menu_buffer, s->selectstrings[value]);
      }
    }

    if (s == current_setup_menu + set_menu_itemon && whichSkull && !setup_select)
      strcat(menu_buffer, " <");
    M_DrawMenuString(x,y,color);
    return;
  }
}

/////////////////////////////
//
// M_DrawScreenItems takes the data for each menu item and gives it to
// the drawing routines above.

// CPhipps - static, const parameter, formatting
static void M_DrawScreenItems(const setup_menu_t* base_src, int base_y)
{
  int i = 0;
  int end_y;
  int scroll_i = 0;
  int current_i = 0;
  int max_i = 0;
  int excess_i = 0;
  int limit_i = 0;
  int buffer_i = 0;
  const setup_menu_t* src;

  i = 0;
  for (src = base_src; !(src->m_flags & S_END); src++) {
    if (src == &current_setup_menu[set_menu_itemon])
      current_i = i;

    if (src->m_flags & (S_NEXT | S_PREV)) {
      // nothing
    }
    else if (src->m_flags & S_RESET_Y) {
      i = 0;
    }
    else {
      if (i > max_i)
        max_i = i;

      ++i;
    }
  }

  end_y = base_y + (max_i + 1) * menu_font->line_height;
  if (end_y > 200)
    excess_i = (end_y - 200 + menu_font->line_height - 1) / menu_font->line_height;

  limit_i = max_i - excess_i;
  buffer_i = (max_i - current_i > 3 ? 3 : max_i - current_i);

  if (excess_i)
    while (current_i - scroll_i > limit_i - buffer_i)
      ++scroll_i;

  i = 0;
  for (src = base_src; !(src->m_flags & S_END); src++) {
    int y;
    dboolean skip_entry = false;

    if (src->m_flags & (S_NEXT | S_PREV)) {
      y = 200 - menu_font->line_height - 2;
    }
    else if (src->m_flags & S_RESET_Y) {
      skip_entry = true;
      i = 0;
    }
    else {
      y = base_y + (i - scroll_i) * menu_font->line_height;

      if (i - scroll_i < 0 || i - scroll_i > limit_i)
        skip_entry = true;

      ++i;
    }

    if (skip_entry)
      continue;

    // See if we're to draw the item description (left-hand part)
    if (src->m_flags & S_SHOWDESC)
      M_DrawItem(src, y);

    // See if we're to draw the setting (right-hand part)
    if (src->m_flags & S_SHOWSET)
      M_DrawSetting(src, y);
  }
}

/////////////////////////////
//
// Data used to draw the "are you sure?" dialogue box when resetting
// to defaults.

#define VERIFYBOXXORG 66
#define VERIFYBOXYORG 88

// And the routine to draw it.

// [FG] delete a savegame

void M_DrawDelVerify(void)
{
  V_DrawNamePatch(VERIFYBOXXORG,VERIFYBOXYORG,0,"M_VBOX",CR_DEFAULT,VPT_STRETCH);

  if (whichSkull) {
    strcpy(menu_buffer,"Delete savegame? (Y or N)");
    M_DrawMenuString(VERIFYBOXXORG + 8, VERIFYBOXYORG + 8, cr_warning);
  }
}

/////////////////////////////
//
// phares 4/18/98:
// M_DrawInstructions writes the instruction text just below the screen title
//
// cph 2006/08/06 - go back to the Boom version, and then clean up by using
// M_DrawStringCentered (much better than all those magic 'x' valies!)

#define INSTRUCTION_Y 20

static void M_DrawInstructionString(int cr, const char *str)
{
  M_DrawStringCentered(160, INSTRUCTION_Y, cr, str);
}

static void M_DrawInstructions(void)
{
  int flags = current_setup_menu[set_menu_itemon].m_flags;

  // There are different instruction messages depending on whether you
  // are changing an item or just sitting on it.

  if (setup_select) {
    switch (flags & (S_INPUT | S_YESNO | S_WEAP | S_NUM | S_COLOR | S_CRITEM | S_FILE | S_CHOICE | S_NAME)) {
      case S_INPUT:
        M_DrawInstructionString(cr_info_edit, "Press key or button for this action");
        break;
      case S_YESNO:
        M_DrawInstructionString(cr_info_edit, "Press ENTER key to toggle");
        break;
      case S_WEAP:
        M_DrawInstructionString(cr_info_edit, "Enter weapon number");
        break;
      case S_NUM:
        M_DrawInstructionString(cr_info_edit, "Enter value. Press ENTER when finished.");
        break;
      case S_COLOR:
        M_DrawInstructionString(cr_info_edit, "Select color and press enter");
        break;
      case S_CRITEM:
        M_DrawInstructionString(cr_info_edit, "Enter value");
        break;
      case S_FILE:
        M_DrawInstructionString(cr_info_edit, "Type/edit filename and Press ENTER");
        break;
      case S_CHOICE:
        M_DrawInstructionString(cr_info_edit, "Press left or right to choose");
        break;
      case S_NAME:
        M_DrawInstructionString(cr_info_edit, "Type / edit author and Press ENTER");
        break;
      default:
        break;
    }
  }
  else {
    if (flags & S_INPUT)
      M_DrawInstructionString(cr_info_highlight, "Press Enter to Change, Del to Clear");
    else
      M_DrawInstructionString(cr_info_highlight, "Press Enter to Change");
  }
}

#define NEXT_PAGE(page) { "->", S_SKIP | S_NEXT, m_null, 318, .menu = page }
#define PREV_PAGE(page) { "<-", S_SKIP | S_PREV | S_LEFTJUST, m_null, 2, .menu = page }
#define FINAL_ENTRY { 0, S_SKIP | S_END, m_null }
#define EMPTY_LINE { 0, S_SKIP, m_null }
#define NEW_COLUMN { 0, S_SKIP | S_RESET_Y, m_null }

#define DEFAULT_LIST_Y (INSTRUCTION_Y + 1.5 * menu_font->line_height)

static void M_EnterSetup(menu_t *menu, dboolean *setup_flag, setup_menu_t *setup_menu)
{
  M_SetupNextMenu(menu);

  setup_active = true;
  *setup_flag = true;
  setup_select = false;
  colorbox_active = false;
  setup_gather = false;

  M_UpdateSetupMenu(setup_menu);
}

/////////////////////////////
//
// The Key Binding Screen tables.

#define KB_X  160

// Definitions of the (in this case) four key binding screens.

setup_menu_t keys_settings1[];
setup_menu_t keys_settings2[];
setup_menu_t keys_settings3[];
setup_menu_t keys_settings4[];
setup_menu_t keys_settings5[];
setup_menu_t keys_settings6[];
setup_menu_t keys_settings7[];
setup_menu_t raven_keys_settings[];
setup_menu_t heretic_keys_settings[];
setup_menu_t hexen_keys_settings[];
setup_menu_t dsda_keys_settings[];
setup_menu_t script_keys_settings[];
setup_menu_t build_keys_settings1[];
setup_menu_t build_keys_settings2[];

// The table which gets you from one screen table to the next.

setup_menu_t* keys_settings[] =
{
  keys_settings1,
  keys_settings2,
  keys_settings3,
  keys_settings4,
  keys_settings5,
  keys_settings6,
  keys_settings7,
  raven_keys_settings,
  heretic_keys_settings,
  hexen_keys_settings,
  dsda_keys_settings,
  script_keys_settings,
  build_keys_settings1,
  build_keys_settings2,
  NULL
};

// The first Key Binding screen table.
// Note that the Y values are ascending. If you need to add something to
// this table, (well, this one's not a good example, because it's full)
// you need to make sure the Y values still make sense so everything gets
// displayed.
//
// Note also that the first screen of each set has a line for the reset
// button. If there is more than one screen in a set, the others don't get
// the reset button.
//
// Note also that this screen has a "->" line. This acts like an
// item, in that 'activating' it moves you along to the next screen. If
// there's a "<-" item on a screen, it behaves similarly, moving you
// to the previous screen. If you leave these off, you can't move from
// screen to screen.

setup_menu_t keys_settings1[] =  // Key Binding screen strings
{
  { "INPUT PROFILE", S_NUM, m_conf, KB_X, dsda_config_input_profile },
  EMPTY_LINE,
  {"MOVEMENT"    ,S_SKIP|S_TITLE,m_null,KB_X},
  {"FORWARD"     ,S_INPUT     ,m_scrn,KB_X,0,dsda_input_forward},
  {"BACKWARD"    ,S_INPUT     ,m_scrn,KB_X,0,dsda_input_backward},
  {"TURN LEFT"   ,S_INPUT     ,m_scrn,KB_X,0,dsda_input_turnleft},
  {"TURN RIGHT"  ,S_INPUT     ,m_scrn,KB_X,0,dsda_input_turnright},
  {"RUN"         ,S_INPUT     ,m_scrn,KB_X,0,dsda_input_speed},
  {"STRAFE LEFT" ,S_INPUT     ,m_scrn,KB_X,0,dsda_input_strafeleft},
  {"STRAFE RIGHT",S_INPUT     ,m_scrn,KB_X,0,dsda_input_straferight},
  {"STRAFE"      ,S_INPUT     ,m_scrn,KB_X,0,dsda_input_strafe},
  {"180 TURN"    ,S_INPUT     ,m_scrn,KB_X,0,dsda_input_reverse},
  {"USE"         ,S_INPUT     ,m_scrn,KB_X,0,dsda_input_use},
  EMPTY_LINE,
  {"TOGGLES"  ,S_SKIP|S_TITLE,m_null,KB_X},
  {"AUTORUN"  ,S_INPUT,m_scrn,KB_X,0,dsda_input_autorun},
  {"FREE LOOK",S_INPUT,m_scrn,KB_X,0,dsda_input_mlook},
  {"VERTMOUSE",S_INPUT,m_scrn,KB_X,0,dsda_input_novert},

  NEXT_PAGE(keys_settings2),
  FINAL_ENTRY
};

setup_menu_t keys_settings2[] =  // Key Binding screen strings
{
  {"SCREEN"      ,S_SKIP|S_TITLE,m_null,KB_X},

  // phares 4/13/98:
  // key_help and key_escape can no longer be rebound. This keeps the
  // player from getting themselves in a bind where they can't remember how
  // to get to the menus, and can't remember how to get to the help screen
  // to give them a clue as to how to get to the menus. :)

  // Also, the keys assigned to these functions cannot be bound to other
  // functions. Introduce an S_KEEP flag to show that you cannot swap this
  // key with other keys in the same 'group'. (m_scrn, etc.)

  // {"HELP"        ,S_SKIP|S_KEEP|S_INPUT ,m_scrn,0   ,0,dsda_input_help},
  // {"MENU"        ,S_SKIP|S_KEEP|S_INPUT ,m_scrn,0   ,0,dsda_input_escape},
  {"PAUSE"       ,S_INPUT     ,m_scrn,KB_X,0,dsda_input_pause},
  {"AUTOMAP"     ,S_INPUT     ,m_scrn,KB_X,0,dsda_input_map},
  {"VOLUME"      ,S_INPUT     ,m_scrn,KB_X,0,dsda_input_soundvolume},
  {"HUD"         ,S_INPUT     ,m_scrn,KB_X,0,dsda_input_hud},
  {"MESSAGES"    ,S_INPUT     ,m_scrn,KB_X,0,dsda_input_messages},
  {"GAMMA FIX"   ,S_INPUT     ,m_scrn,KB_X,0,dsda_input_gamma},
  {"SPY"         ,S_INPUT     ,m_scrn,KB_X,0,dsda_input_spy},
  {"LARGER VIEW" ,S_INPUT     ,m_scrn,KB_X,0,dsda_input_zoomin},
  {"SMALLER VIEW",S_INPUT     ,m_scrn,KB_X,0,dsda_input_zoomout},
  {"SCREENSHOT"  ,S_INPUT     ,m_scrn,KB_X,0,dsda_input_screenshot},
  EMPTY_LINE,
  {"GAME"        ,S_SKIP|S_TITLE,m_null,KB_X},
  {"SAVE"        ,S_INPUT     ,m_scrn,KB_X,0,dsda_input_savegame},
  {"LOAD"        ,S_INPUT     ,m_scrn,KB_X,0,dsda_input_loadgame},
  {"QUICKSAVE"   ,S_INPUT     ,m_scrn,KB_X,0,dsda_input_quicksave},
  {"QUICKLOAD"   ,S_INPUT     ,m_scrn,KB_X,0,dsda_input_quickload},
  {"END GAME"    ,S_INPUT     ,m_scrn,KB_X,0,dsda_input_endgame},
  {"QUIT"        ,S_INPUT     ,m_scrn,KB_X,0,dsda_input_quit},

  PREV_PAGE(keys_settings1),
  NEXT_PAGE(keys_settings3),
  FINAL_ENTRY
};

setup_menu_t keys_settings3[] =  // Key Binding screen strings
{
  {"WEAPONS" ,S_SKIP|S_TITLE,m_null,KB_X},
  {"FIST"    ,S_INPUT       ,m_scrn,KB_X,0,dsda_input_weapon1},
  {"PISTOL"  ,S_INPUT       ,m_scrn,KB_X,0,dsda_input_weapon2},
  {"SHOTGUN" ,S_INPUT       ,m_scrn,KB_X,0,dsda_input_weapon3},
  {"CHAINGUN",S_INPUT       ,m_scrn,KB_X,0,dsda_input_weapon4},
  {"ROCKET"  ,S_INPUT       ,m_scrn,KB_X,0,dsda_input_weapon5},
  {"PLASMA"  ,S_INPUT       ,m_scrn,KB_X,0,dsda_input_weapon6},
  {"BFG",     S_INPUT       ,m_scrn,KB_X,0,dsda_input_weapon7},
  {"CHAINSAW",S_INPUT       ,m_scrn,KB_X,0,dsda_input_weapon8},
  {"SSG"     ,S_INPUT       ,m_scrn,KB_X,0,dsda_input_weapon9},
  EMPTY_LINE,
  {"NEXT"    ,S_INPUT       ,m_scrn,KB_X,0,dsda_input_nextweapon},
  {"PREVIOUS",S_INPUT       ,m_scrn,KB_X,0,dsda_input_prevweapon},
  {"BEST"    ,S_INPUT       ,m_scrn,KB_X,0,dsda_input_toggleweapon},
  {"FIRE"    ,S_INPUT       ,m_scrn,KB_X,0,dsda_input_fire},

  PREV_PAGE(keys_settings2),
  NEXT_PAGE(keys_settings4),
  FINAL_ENTRY
};

setup_menu_t keys_settings4[] =  // Key Binding screen strings
{
  {"AUTOMAP"    ,S_SKIP|S_TITLE,m_null,KB_X},
  {"FOLLOW"     ,S_INPUT     ,m_map ,KB_X,0,dsda_input_map_follow},
  {"ZOOM IN"    ,S_INPUT     ,m_map ,KB_X,0,dsda_input_map_zoomin},
  {"ZOOM OUT"   ,S_INPUT     ,m_map ,KB_X,0,dsda_input_map_zoomout},
  {"SHIFT UP"   ,S_INPUT     ,m_map ,KB_X,0,dsda_input_map_up},
  {"SHIFT DOWN" ,S_INPUT     ,m_map ,KB_X,0,dsda_input_map_down},
  {"SHIFT LEFT" ,S_INPUT     ,m_map ,KB_X,0,dsda_input_map_left},
  {"SHIFT RIGHT",S_INPUT     ,m_map ,KB_X,0,dsda_input_map_right},
  {"MARK PLACE" ,S_INPUT     ,m_map ,KB_X,0,dsda_input_map_mark},
  {"CLEAR MARKS",S_INPUT     ,m_map ,KB_X,0,dsda_input_map_clear},
  {"FULL/ZOOM"  ,S_INPUT     ,m_map ,KB_X,0,dsda_input_map_gobig},
  {"GRID"       ,S_INPUT     ,m_map ,KB_X,0,dsda_input_map_grid},
  {"ROTATE"     ,S_INPUT     ,m_map ,KB_X,0,dsda_input_map_rotate},
  {"OVERLAY"    ,S_INPUT     ,m_map ,KB_X,0,dsda_input_map_overlay},
  {"TEXTURED"   ,S_INPUT     ,m_map ,KB_X,0,dsda_input_map_textured},

  PREV_PAGE(keys_settings3),
  NEXT_PAGE(keys_settings5),
  FINAL_ENTRY
};

//e6y
setup_menu_t keys_settings5[] =  // Key Binding screen strings
{
  {"GAME SPEED"           ,S_SKIP|S_TITLE,m_null,KB_X},
  {"SPEED UP"             ,S_INPUT   ,m_scrn,KB_X,0,dsda_input_speed_up},
  {"SPEED DOWN"           ,S_INPUT   ,m_scrn,KB_X,0,dsda_input_speed_down},
  {"RESET TO DEFAULT"     ,S_INPUT   ,m_scrn,KB_X,0,dsda_input_speed_default},
  EMPTY_LINE,
  {"DEMOS"                ,S_SKIP|S_TITLE,m_null,KB_X},
  {"START/STOP SKIPPING"  ,S_INPUT   ,m_scrn,KB_X,0,dsda_input_demo_skip},
  {"END LEVEL"            ,S_INPUT   ,m_scrn,KB_X,0,dsda_input_demo_endlevel},
  {"CAMERA MODE"          ,S_INPUT   ,m_scrn,KB_X,0,dsda_input_walkcamera},
  {"JOIN"                 ,S_INPUT   ,m_scrn,KB_X,0,dsda_input_join_demo},
  {"RESTART DEMO ATTEMPT" ,S_INPUT   ,m_scrn,KB_X,0,dsda_input_restart},
  EMPTY_LINE,
  {"MISC"                 ,S_SKIP|S_TITLE,m_null,KB_X},
  {"RESTART CURRENT MAP"  ,S_INPUT   ,m_scrn,KB_X,0,dsda_input_restart},
  {"NEXT LEVEL"           ,S_INPUT   ,m_scrn,KB_X,0,dsda_input_nextlevel},
  {"Show Alive Monsters"  ,S_INPUT   ,m_scrn,KB_X,0,dsda_input_showalive},

  PREV_PAGE(keys_settings4),
  NEXT_PAGE(keys_settings6),
  FINAL_ENTRY
};

setup_menu_t keys_settings6[] =
{
  {"MENUS"       ,S_SKIP|S_TITLE,m_null,KB_X},
  {"NEXT ITEM"   ,S_INPUT     ,m_menu,KB_X,0,dsda_input_menu_down},
  {"PREV ITEM"   ,S_INPUT     ,m_menu,KB_X,0,dsda_input_menu_up},
  {"LEFT"        ,S_INPUT     ,m_menu,KB_X,0,dsda_input_menu_left},
  {"RIGHT"       ,S_INPUT     ,m_menu,KB_X,0,dsda_input_menu_right},
  {"BACKSPACE"   ,S_INPUT     ,m_menu,KB_X,0,dsda_input_menu_backspace},
  { "SELECT ITEM", S_INPUT | S_NOCLEAR, m_menu, KB_X, 0, dsda_input_menu_enter },
  {"EXIT"        ,S_INPUT     ,m_menu,KB_X,0,dsda_input_menu_escape},
  {"CLEAR"       ,S_INPUT     ,m_menu,KB_X,0,dsda_input_menu_clear},
  EMPTY_LINE,
  {"MESSAGES"       ,S_SKIP|S_TITLE,m_null,KB_X},
  {"REPEAT MESSAGE" ,S_INPUT     ,m_scrn,KB_X,0,dsda_input_repeat_message},

  PREV_PAGE(keys_settings5),
  NEXT_PAGE(keys_settings7),
  FINAL_ENTRY
};

setup_menu_t keys_settings7[] =
{
  { "CHEATS", S_SKIP | S_TITLE, m_null, KB_X},
  { "God Mode", S_INPUT, m_scrn, KB_X, 0, dsda_input_iddqd },
  { "Ammo & Keys", S_INPUT, m_scrn, KB_X, 0, dsda_input_idkfa },
  { "Ammo", S_INPUT, m_scrn, KB_X, 0, dsda_input_idfa },
  { "No Clipping", S_INPUT, m_scrn, KB_X, 0, dsda_input_idclip },
  { "Health", S_INPUT, m_scrn, KB_X, 0, dsda_input_idbeholdh },
  { "Armor", S_INPUT, m_scrn, KB_X, 0, dsda_input_idbeholdm },
  { "Invulnerability", S_INPUT, m_scrn, KB_X, 0, dsda_input_idbeholdv },
  { "Berserk", S_INPUT, m_scrn, KB_X, 0, dsda_input_idbeholds },
  { "Partial Invisibility", S_INPUT, m_scrn, KB_X, 0, dsda_input_idbeholdi },
  { "Radiation Suit", S_INPUT, m_scrn, KB_X, 0, dsda_input_idbeholdr },
  { "Computer Area Map", S_INPUT, m_scrn, KB_X, 0, dsda_input_idbeholda },
  { "Light Amplification", S_INPUT, m_scrn, KB_X, 0, dsda_input_idbeholdl },
  { "Show Position", S_INPUT, m_scrn, KB_X, 0, dsda_input_idmypos },
  { "Reveal Map", S_INPUT, m_scrn, KB_X, 0, dsda_input_iddt },
  { "Reset Health", S_INPUT, m_scrn, KB_X, 0, dsda_input_ponce },
  { "Tome of Power", S_INPUT, m_scrn, KB_X, 0, dsda_input_shazam },
  { "Chicken", S_INPUT, m_scrn, KB_X, 0, dsda_input_chicken },
  { "No Target", S_INPUT, m_scrn, KB_X, 0, dsda_input_notarget },
  { "Freeze", S_INPUT, m_scrn, KB_X, 0, dsda_input_freeze },

  PREV_PAGE(keys_settings6),
  NEXT_PAGE(raven_keys_settings),
  FINAL_ENTRY
};

setup_menu_t raven_keys_settings[] = {
  { "RAVEN MOVEMENT", S_SKIP | S_TITLE, m_null, KB_X},
  { "LOOK UP", S_INPUT, m_scrn, KB_X, 0, dsda_input_lookup },
  { "LOOK DOWN", S_INPUT, m_scrn, KB_X, 0, dsda_input_lookdown },
  { "LOOK CENTER", S_INPUT, m_scrn, KB_X, 0, dsda_input_lookcenter },
  { "FLY UP", S_INPUT, m_scrn, KB_X, 0, dsda_input_flyup },
  { "FLY DOWN", S_INPUT, m_scrn, KB_X, 0, dsda_input_flydown },
  { "FLY CENTER", S_INPUT, m_scrn, KB_X, 0, dsda_input_flycenter },
  { "JUMP", S_INPUT, m_scrn, KB_X, 0, dsda_input_jump },

  PREV_PAGE(keys_settings7),
  NEXT_PAGE(heretic_keys_settings),
  FINAL_ENTRY
};

setup_menu_t heretic_keys_settings[] = {
  { "HERETIC INVENTORY", S_SKIP | S_TITLE, m_null, KB_X},
  { "USE ARTIFACT", S_INPUT, m_scrn, KB_X, 0, dsda_input_use_artifact },
  { "USE TOME OF POWER", S_INPUT, m_scrn, KB_X, 0, dsda_input_arti_tome },
  { "USE QUARTZ FLASK", S_INPUT, m_scrn, KB_X, 0, dsda_input_arti_quartz },
  { "USE MYSTIC URN", S_INPUT, m_scrn, KB_X, 0, dsda_input_arti_urn },
  { "USE TIMEBOMB", S_INPUT, m_scrn, KB_X, 0, dsda_input_arti_bomb },
  { "USE RING OF INVINCIBILITY", S_INPUT, m_scrn, KB_X, 0, dsda_input_arti_ring },
  { "USE CHAOS DEVICE", S_INPUT, m_scrn, KB_X, 0, dsda_input_arti_chaosdevice },
  { "USE SHADOWSPHERE", S_INPUT, m_scrn, KB_X, 0, dsda_input_arti_shadowsphere },
  { "USE WINGS OF WRATH", S_INPUT, m_scrn, KB_X, 0, dsda_input_arti_wings },
  { "USE TORCH", S_INPUT, m_scrn, KB_X, 0, dsda_input_arti_torch },
  { "USE MORPH OVUM", S_INPUT, m_scrn, KB_X, 0, dsda_input_arti_morph },
  { "INVENTORY LEFT", S_INPUT, m_scrn, KB_X, 0, dsda_input_invleft },
  { "INVENTORY RIGHT", S_INPUT, m_scrn, KB_X, 0, dsda_input_invright },

  PREV_PAGE(raven_keys_settings),
  NEXT_PAGE(hexen_keys_settings),
  FINAL_ENTRY
};

setup_menu_t hexen_keys_settings[] = {
  { "HEXEN INVENTORY", S_SKIP | S_TITLE, m_null, KB_X},
  { "USE ARTIFACT", S_INPUT, m_scrn, KB_X, 0, dsda_input_use_artifact },
  { "USE ICON OF THE DEFENDER", S_INPUT, m_scrn, KB_X, 0, dsda_input_arti_ring },
  { "USE QUARTZ FLASK", S_INPUT, m_scrn, KB_X, 0, dsda_input_arti_quartz },
  { "USE MYSTIC URN", S_INPUT, m_scrn, KB_X, 0, dsda_input_arti_urn },
  { "USE MYSTIC AMBIT INCANT", S_INPUT, m_scrn, KB_X, 0, dsda_input_hexen_arti_incant },
  { "USE DARK SERVANT", S_INPUT, m_scrn, KB_X, 0, dsda_input_hexen_arti_summon },
  { "USE TORCH", S_INPUT, m_scrn, KB_X, 0, dsda_input_arti_torch },
  { "USE PORKALATOR", S_INPUT, m_scrn, KB_X, 0, dsda_input_arti_morph },
  { "USE WINGS OF WRATH", S_INPUT, m_scrn, KB_X, 0, dsda_input_arti_wings },
  { "USE DISC OF REPULSION", S_INPUT, m_scrn, KB_X, 0, dsda_input_hexen_arti_disk },
  { "USE FLECHETTE", S_INPUT, m_scrn, KB_X, 0, dsda_input_hexen_arti_flechette },
  { "USE BANISHMENT DEVICE", S_INPUT, m_scrn, KB_X, 0, dsda_input_hexen_arti_banishment },
  { "USE BOOTS OF SPEED", S_INPUT, m_scrn, KB_X, 0, dsda_input_hexen_arti_boots },
  { "USE KRATER OF MIGHT", S_INPUT, m_scrn, KB_X, 0, dsda_input_hexen_arti_krater },
  { "USE DRAGONSKIN BRACERS", S_INPUT, m_scrn, KB_X, 0, dsda_input_hexen_arti_bracers },
  { "USE CHAOS DEVICE", S_INPUT, m_scrn, KB_X, 0, dsda_input_arti_chaosdevice },
  { "INVENTORY LEFT", S_INPUT, m_scrn, KB_X, 0, dsda_input_invleft },
  { "INVENTORY RIGHT", S_INPUT, m_scrn, KB_X, 0, dsda_input_invright },

  PREV_PAGE(heretic_keys_settings),
  NEXT_PAGE(dsda_keys_settings),
  FINAL_ENTRY
};

setup_menu_t dsda_keys_settings[] = {
  { "DSDA-Doom Keys", S_SKIP | S_TITLE, m_null, KB_X},
  { "Store Quick Key Frame", S_INPUT, m_scrn, KB_X, 0, dsda_input_store_quick_key_frame },
  { "Restore Quick Key Frame", S_INPUT, m_scrn, KB_X, 0, dsda_input_restore_quick_key_frame },
  { "Rewind", S_INPUT, m_scrn, KB_X, 0, dsda_input_rewind },
  { "Cycle Input Profile", S_INPUT, m_scrn, KB_X, 0, dsda_input_cycle_profile },
  { "Cycle Palette", S_INPUT, m_scrn, KB_X, 0, dsda_input_cycle_palette },
  { "Open Console", S_INPUT, m_scrn, KB_X, 0, dsda_input_console },
  { "Fake Archvile Jump", S_INPUT, m_scrn, KB_X, 0, dsda_input_avj },
  EMPTY_LINE,
  { "Toggles", S_SKIP | S_TITLE, m_null, KB_X},
  { "Command Display", S_INPUT, m_scrn, KB_X, 0, dsda_input_command_display },
  { "Strict Mode", S_INPUT, m_scrn, KB_X, 0, dsda_input_strict_mode },
  { "Coordinate Display", S_INPUT, m_scrn, KB_X, 0, dsda_input_coordinate_display },
  { "Extended HUD", S_INPUT, m_scrn, KB_X, 0, dsda_input_exhud },
  { "SFX", S_INPUT, m_scrn, KB_X, 0, dsda_input_mute_sfx },
  { "Music", S_INPUT, m_scrn, KB_X, 0, dsda_input_mute_music },
  { "Cheat Code Entry", S_INPUT, m_scrn, KB_X, 0, dsda_input_cheat_codes },
  { "Render Stats", S_INPUT, m_scrn, KB_X, 0, dsda_input_idrate },
  { "FPS", S_INPUT, m_scrn, KB_X, 0, dsda_input_fps },

  PREV_PAGE(hexen_keys_settings),
  NEXT_PAGE(script_keys_settings),
  FINAL_ENTRY
};

setup_menu_t script_keys_settings[] = {
  { "Script Keys", S_SKIP | S_TITLE, m_null, KB_X},
  { "Script 0", S_INPUT, m_scrn, KB_X, 0, dsda_input_script_0 },
  { "Script 1", S_INPUT, m_scrn, KB_X, 0, dsda_input_script_1 },
  { "Script 2", S_INPUT, m_scrn, KB_X, 0, dsda_input_script_2 },
  { "Script 3", S_INPUT, m_scrn, KB_X, 0, dsda_input_script_3 },
  { "Script 4", S_INPUT, m_scrn, KB_X, 0, dsda_input_script_4 },
  { "Script 5", S_INPUT, m_scrn, KB_X, 0, dsda_input_script_5 },
  { "Script 6", S_INPUT, m_scrn, KB_X, 0, dsda_input_script_6 },
  { "Script 7", S_INPUT, m_scrn, KB_X, 0, dsda_input_script_7 },
  { "Script 8", S_INPUT, m_scrn, KB_X, 0, dsda_input_script_8 },
  { "Script 9", S_INPUT, m_scrn, KB_X, 0, dsda_input_script_9 },

  PREV_PAGE(dsda_keys_settings),
  NEXT_PAGE(build_keys_settings1),
  FINAL_ENTRY
};

setup_menu_t build_keys_settings1[] = {
  { "Build Mode (1)", S_SKIP | S_TITLE, m_null, KB_X},
  { "Toggle Build Mode", S_INPUT, m_scrn, KB_X, 0, dsda_input_build },
  { "Advance Frame", S_INPUT, m_build, KB_X, 0, dsda_input_build_advance_frame },
  { "Reverse Frame", S_INPUT, m_build, KB_X, 0, dsda_input_build_reverse_frame },
  { "Reset Command", S_INPUT, m_build, KB_X, 0, dsda_input_build_reset_command },
  { "Toggle Source", S_INPUT, m_build, KB_X, 0, dsda_input_build_source },
  EMPTY_LINE,
  { "Controls", S_SKIP | S_TITLE, m_null, KB_X},
  { "Forward", S_INPUT, m_build, KB_X, 0, dsda_input_build_forward },
  { "Backward", S_INPUT, m_build, KB_X, 0, dsda_input_build_backward },
  { "Fine Forward", S_INPUT, m_build, KB_X, 0, dsda_input_build_fine_forward },
  { "Fine Backward", S_INPUT, m_build, KB_X, 0, dsda_input_build_fine_backward },
  { "Turn Left", S_INPUT, m_build, KB_X, 0, dsda_input_build_turn_left },
  { "Turn Right", S_INPUT, m_build, KB_X, 0, dsda_input_build_turn_right },
  { "Strafe Left", S_INPUT, m_build, KB_X, 0, dsda_input_build_strafe_left },
  { "Strafe Right", S_INPUT, m_build, KB_X, 0, dsda_input_build_strafe_right },
  { "Fine Strafe Left", S_INPUT, m_build, KB_X, 0, dsda_input_build_fine_strafe_left },
  { "Fine Strafe Right", S_INPUT, m_build, KB_X, 0, dsda_input_build_fine_strafe_right },
  { "Use", S_INPUT, m_build, KB_X, 0, dsda_input_build_use },

  PREV_PAGE(script_keys_settings),
  NEXT_PAGE(build_keys_settings2),
  FINAL_ENTRY
};

setup_menu_t build_keys_settings2[] = {
  { "Build Mode (2)", S_SKIP | S_TITLE, m_null, KB_X},
  { "Fire", S_INPUT, m_build, KB_X, 0, dsda_input_build_fire },
  { "Fist", S_INPUT, m_build, KB_X, 0, dsda_input_build_weapon1 },
  { "Pistol", S_INPUT, m_build, KB_X, 0, dsda_input_build_weapon2 },
  { "Shotgun", S_INPUT, m_build, KB_X, 0, dsda_input_build_weapon3 },
  { "Chaingun", S_INPUT, m_build, KB_X, 0, dsda_input_build_weapon4 },
  { "Rocket", S_INPUT, m_build, KB_X, 0, dsda_input_build_weapon5 },
  { "Plasma", S_INPUT, m_build, KB_X, 0, dsda_input_build_weapon6 },
  { "BFG", S_INPUT, m_build, KB_X, 0, dsda_input_build_weapon7 },
  { "Chainsaw", S_INPUT, m_build, KB_X, 0, dsda_input_build_weapon8 },
  { "SSG", S_INPUT, m_build, KB_X, 0, dsda_input_build_weapon9 },

  PREV_PAGE(build_keys_settings1),
  FINAL_ENTRY
};

// Setting up for the Key Binding screen. Turn on flags, set pointers,
// locate the first item on the screen where the cursor is allowed to
// land.

void M_KeyBindings(int choice)
{
  M_EnterSetup(&KeybndDef, &set_keybnd_active, keys_settings[0]);
}

// The drawing part of the Key Bindings Setup initialization. Draw the
// background, title, instruction line, and items.

void M_DrawKeybnd(void)
{
  M_ChangeMenu(NULL, mnact_full);

  // Set up the Key Binding screen

  M_DrawBackground(g_menu_flat, 0); // Draw background

  // proff/nicolas 09/20/98 -- changed for hi-res
  M_DrawTitle(84, 2, "M_KEYBND", CR_DEFAULT, "KEY BINDINGS", cr_title);
  M_DrawInstructions();
  M_DrawScreenItems(current_setup_menu, DEFAULT_LIST_Y);
}

/////////////////////////////
//
// The Weapon Screen tables.

#define WP_X 203

static const char *weapon_attack_alignment_strings[] = {
  "OFF", "HORIZONTAL", "CENTERED", "BOBBING", NULL
};

// There's only one weapon settings screen (for now). But since we're
// trying to fit a common description for screens, it gets a setup_menu_t,
// which only has one screen definition in it.
//
// Note that this screen has no PREV or NEXT items, since there are no
// neighboring screens.

setup_menu_t weap_settings1[];

setup_menu_t* weap_settings[] =
{
  weap_settings1,
  NULL
};

setup_menu_t weap_settings1[] =  // Weapons Settings screen
{
  { "Weapon Preferences", S_SKIP | S_TITLE, m_null, WP_X},
  { "1ST CHOICE WEAPON", S_WEAP, m_conf, WP_X, dsda_config_weapon_choice_1 },
  { "2nd CHOICE WEAPON", S_WEAP, m_conf, WP_X, dsda_config_weapon_choice_2 },
  { "3rd CHOICE WEAPON", S_WEAP, m_conf, WP_X, dsda_config_weapon_choice_3 },
  { "4th CHOICE WEAPON", S_WEAP, m_conf, WP_X, dsda_config_weapon_choice_4 },
  { "5th CHOICE WEAPON", S_WEAP, m_conf, WP_X, dsda_config_weapon_choice_5 },
  { "6th CHOICE WEAPON", S_WEAP, m_conf, WP_X, dsda_config_weapon_choice_6 },
  { "7th CHOICE WEAPON", S_WEAP, m_conf, WP_X, dsda_config_weapon_choice_7 },
  { "8th CHOICE WEAPON", S_WEAP, m_conf, WP_X, dsda_config_weapon_choice_8 },
  { "9th CHOICE WEAPON", S_WEAP, m_conf, WP_X, dsda_config_weapon_choice_9 },

  FINAL_ENTRY
};

// Setting up for the Weapons screen. Turn on flags, set pointers,
// locate the first item on the screen where the cursor is allowed to
// land.

void M_Weapons(int choice)
{
  M_EnterSetup(&WeaponDef, &set_weapon_active, weap_settings[0]);
}


// The drawing part of the Weapons Setup initialization. Draw the
// background, title, instruction line, and items.

void M_DrawWeapons(void)
{
  M_ChangeMenu(NULL, mnact_full);

  M_DrawBackground(g_menu_flat, 0); // Draw background

  // proff/nicolas 09/20/98 -- changed for hi-res
  M_DrawTitle(109, 2, "M_WEAP", CR_DEFAULT, "WEAPONS", cr_title);
  M_DrawInstructions();
  M_DrawScreenItems(current_setup_menu, DEFAULT_LIST_Y);
}

/////////////////////////////
//
// The Status Bar / HUD tables.

#define SB_X 203

// Screen table definitions

setup_menu_t stat_settings1[];
//e6y
setup_menu_t stat_settings2[];

setup_menu_t* stat_settings[] =
{
  stat_settings1,
  //e6y
  stat_settings2,
  NULL
};

setup_menu_t stat_settings1[] =  // Status Bar and HUD Settings screen
{
  { "STATUS BAR", S_SKIP | S_TITLE, m_null, SB_X},
  { "USE RED NUMBERS", S_YESNO, m_conf, SB_X, dsda_config_sts_always_red },
  { "GRAY %",S_YESNO, m_conf, SB_X, dsda_config_sts_pct_always_gray },
  { "SINGLE KEY DISPLAY", S_YESNO, m_conf, SB_X, dsda_config_sts_traditional_keys },
  EMPTY_LINE,
  { "HEADS-UP DISPLAY", S_SKIP | S_TITLE, m_null, SB_X},
  { "SHOW MESSAGES", S_YESNO, m_conf, SB_X, dsda_config_show_messages },
  { "HEALTH LOW/OK", S_NUM, m_conf, SB_X, dsda_config_hud_health_red },
  { "HEALTH OK/GOOD", S_NUM, m_conf, SB_X, dsda_config_hud_health_yellow },
  { "HEALTH GOOD/EXTRA", S_NUM, m_conf, SB_X, dsda_config_hud_health_green },
  { "AMMO LOW/OK", S_NUM, m_conf, SB_X, dsda_config_hud_ammo_red },
  { "AMMO OK/GOOD", S_NUM, m_conf, SB_X, dsda_config_hud_ammo_yellow },
  { "REPORT REVEALED SECRETS", S_YESNO, m_conf, SB_X, dsda_config_hudadd_secretarea },
  { "DEMO PLAYBACK PROGRESS BAR", S_YESNO, m_conf, SB_X, dsda_config_hudadd_demoprogressbar },

  NEXT_PAGE(stat_settings2),
  FINAL_ENTRY
};

//e6y
#define HUD_X 284

static const char *crosshair_str[] =
  { "none", "cross", "angle", "dot", "small", "slim", "tiny", "big", NULL };

setup_menu_t stat_settings2[] =
{
  { "CROSSHAIR SETTINGS", S_SKIP | S_TITLE, m_null, HUD_X},
  { "ENABLE CROSSHAIR", S_CHOICE, m_conf, HUD_X, dsda_config_hudadd_crosshair, 0, crosshair_str },
  { "SCALE CROSSHAIR", S_YESNO, m_conf, HUD_X, dsda_config_hudadd_crosshair_scale },
  { "CHANGE CROSSHAIR COLOR BY PLAYER HEALTH", S_YESNO, m_conf, HUD_X, dsda_config_hudadd_crosshair_health },
  { "CHANGE CROSSHAIR COLOR ON TARGET", S_YESNO, m_conf, HUD_X, dsda_config_hudadd_crosshair_target },
  { "LOCK CROSSHAIR ON TARGET", S_YESNO, m_conf, HUD_X, dsda_config_hudadd_crosshair_lock_target },
  { "DEFAULT CROSSHAIR COLOR", S_CRITEM, m_conf, HUD_X, dsda_config_hudadd_crosshair_color },
  { "TARGET CROSSHAIR COLOR", S_CRITEM, m_conf, HUD_X, dsda_config_hudadd_crosshair_target_color },

  PREV_PAGE(stat_settings1),
  FINAL_ENTRY
};

// Setting up for the Status Bar / HUD screen. Turn on flags, set pointers,
// locate the first item on the screen where the cursor is allowed to
// land.

void M_StatusBar(int choice)
{
  M_EnterSetup(&StatusHUDDef, &set_status_active, stat_settings[0]);
}

// The drawing part of the Status Bar / HUD Setup initialization. Draw the
// background, title, instruction line, and items.

void M_DrawStatusHUD(void)
{
  M_ChangeMenu(NULL, mnact_full);

  M_DrawBackground(g_menu_flat, 0); // Draw background

  // proff/nicolas 09/20/98 -- changed for hi-res
  M_DrawTitle(59, 2, "M_STAT", CR_DEFAULT, "STATUS BAR / HUD", cr_title);
  M_DrawInstructions();
  M_DrawScreenItems(current_setup_menu, DEFAULT_LIST_Y);
}


/////////////////////////////
//
// The Automap tables.

#define AU_X    275

setup_menu_t auto_settings1[];
setup_menu_t auto_settings2[];
setup_menu_t auto_settings3[];

setup_menu_t* auto_settings[] =
{
  auto_settings1,
  auto_settings2,
  auto_settings3,
  NULL
};

static const char *map_things_appearance_list[] =
{
  "classic",
  "scaled",
#if defined(HAVE_LIBSDL2_IMAGE)
  "icons",
#endif
  NULL
};

setup_menu_t auto_settings1[] =  // 1st AutoMap Settings screen
{
  { "Automap Components", S_SKIP | S_TITLE, m_null, AU_X},
  { "Stat Totals", S_YESNO, m_conf, AU_X, dsda_config_map_totals },
  { "Player Coordinates", S_YESNO, m_conf, AU_X, dsda_config_map_coordinates },
  { "Level / Total Time", S_YESNO, m_conf, AU_X, dsda_config_map_time },
  { "Level Title", S_YESNO, m_conf, AU_X, dsda_config_map_title },
  EMPTY_LINE,
  { "Locked doors blink", S_YESNO, m_conf, AU_X, dsda_config_map_blinking_locks },
  { "Show Secrets only after entering", S_YESNO, m_conf, AU_X, dsda_config_map_secret_after },
  { "Grid cell size 8..256, -1 for autosize", S_NUM, m_conf, AU_X, dsda_config_map_grid_size },
  { "Scroll / Zoom speed  (1..32)", S_NUM, m_conf, AU_X, dsda_config_map_scroll_speed },
  { "Use mouse wheel for zooming", S_YESNO, m_conf, AU_X, dsda_config_map_wheel_zoom },
  { "Enable textured display", S_YESNO, m_conf, AU_X, dsda_config_map_textured },
  { "Things appearance", S_CHOICE, m_conf, AU_X, dsda_config_map_things_appearance, 0, map_things_appearance_list },
  { "Show Minimap", S_YESNO, m_conf, AU_X, dsda_config_show_minimap },
  EMPTY_LINE,
  { "Translucency percentage", S_SKIP | S_TITLE, m_null, AU_X},
  //{ "Textured automap", S_NUM, m_conf, AU_X, dsda_config_map_textured_trans },
  //{ "Textured automap in overlay mode", S_NUM, m_conf, AU_X, dsda_config_map_textured_overlay_trans },
  //{ "Lines in overlay mode", S_NUM, m_conf, AU_X, dsda_config_map_lines_overlay_trans },

  NEXT_PAGE(auto_settings2),
  FINAL_ENTRY
};

setup_menu_t auto_settings2[] =  // 2st AutoMap Settings screen
{
  {"background", S_COLOR, m_conf, AU_X, dsda_config_mapcolor_back},
  {"grid lines", S_COLOR, m_conf, AU_X, dsda_config_mapcolor_grid},
  {"normal 1s wall", S_COLOR, m_conf,AU_X, dsda_config_mapcolor_wall},
  {"line at floor height change", S_COLOR, m_conf, AU_X, dsda_config_mapcolor_fchg},
  {"line at ceiling height change"      ,S_COLOR,m_conf,AU_X, dsda_config_mapcolor_cchg},
  {"line at sector with floor = ceiling",S_COLOR,m_conf,AU_X, dsda_config_mapcolor_clsd},
  {"red key"                            ,S_COLOR,m_conf,AU_X, dsda_config_mapcolor_rkey},
  {"blue key"                           ,S_COLOR,m_conf,AU_X, dsda_config_mapcolor_bkey},
  {"yellow key"                         ,S_COLOR,m_conf,AU_X, dsda_config_mapcolor_ykey},
  {"red door"                           ,S_COLOR,m_conf,AU_X, dsda_config_mapcolor_rdor},
  {"blue door"                          ,S_COLOR,m_conf,AU_X, dsda_config_mapcolor_bdor},
  {"yellow door"                        ,S_COLOR,m_conf,AU_X, dsda_config_mapcolor_ydor},

  PREV_PAGE(auto_settings1),
  NEXT_PAGE(auto_settings3),
  FINAL_ENTRY
};

setup_menu_t auto_settings3[] =  // 3nd AutoMap Settings screen
{
  {"teleporter line"                ,S_COLOR ,m_conf,AU_X, dsda_config_mapcolor_tele},
  {"secret sector boundary"         ,S_COLOR ,m_conf,AU_X, dsda_config_mapcolor_secr},
  {"revealed secret sector boundary",S_COLOR ,m_conf,AU_X, dsda_config_mapcolor_revsecr},
  //jff 4/23/98 add exit line to automap
  {"exit line"                      ,S_COLOR ,m_conf,AU_X, dsda_config_mapcolor_exit},
  {"computer map unseen line"       ,S_COLOR ,m_conf,AU_X, dsda_config_mapcolor_unsn},
  {"line w/no floor/ceiling changes",S_COLOR ,m_conf,AU_X, dsda_config_mapcolor_flat},
  {"general sprite"                 ,S_COLOR ,m_conf,AU_X, dsda_config_mapcolor_sprt},
  {"countable enemy sprite"         ,S_COLOR ,m_conf,AU_X, dsda_config_mapcolor_enemy},      // cph 2006/06/30
  {"countable item sprite"          ,S_COLOR ,m_conf,AU_X, dsda_config_mapcolor_item},       // mead 3/4/2003
  {"crosshair"                      ,S_COLOR ,m_conf,AU_X, dsda_config_mapcolor_hair},
  {"single player arrow"            ,S_COLOR ,m_conf,AU_X, dsda_config_mapcolor_sngl},
  {"your colour in multiplayer"     ,S_COLOR ,m_conf,AU_X, dsda_config_mapcolor_me},
  EMPTY_LINE,
  {"friends"                        ,S_COLOR ,m_conf,AU_X, dsda_config_mapcolor_frnd},        // killough 8/8/98

  PREV_PAGE(auto_settings2),
  FINAL_ENTRY
};


// Setting up for the Automap screen. Turn on flags, set pointers,
// locate the first item on the screen where the cursor is allowed to
// land.

void M_Automap(int choice)
{
  M_EnterSetup(&AutoMapDef, &set_auto_active, auto_settings[0]);
}

// Data used by the color palette that is displayed for the player to
// select colors.

int color_palette_x; // X position of the cursor on the color palette
int color_palette_y; // Y position of the cursor on the color palette
byte palette_background[16*(CHIP_SIZE+1)+8];

// M_DrawColPal() draws the color palette when the user needs to select a
// color.

// phares 4/1/98: now uses a single lump for the palette instead of
// building the image out of individual paint chips.

static void M_DrawColPal(void)
{
  int cpx, cpy;

  // Draw a background, border, and paint chips

  // proff/nicolas 09/20/98 -- changed for hi-res
  // CPhipps - patch drawing updated
  V_DrawNamePatch(COLORPALXORIG-5, COLORPALYORIG-5, 0, "M_COLORS", CR_DEFAULT, VPT_STRETCH);

  // Draw the cursor around the paint chip
  // (cpx,cpy) is the upper left-hand corner of the paint chip

  cpx = COLORPALXORIG+color_palette_x*(CHIP_SIZE+1)-1;
  cpy = COLORPALYORIG+color_palette_y*(CHIP_SIZE+1)-1;
  // proff 12/6/98: Drawing of colorchips completly changed for hi-res, it now uses a patch
  V_DrawNamePatch(cpx,cpy,0,"M_PALSEL",CR_DEFAULT,VPT_STRETCH); // PROFF_GL_FIX
}

// The drawing part of the Automap Setup initialization. Draw the
// background, title, instruction line, and items.

void M_DrawAutoMap(void)
{
  M_ChangeMenu(NULL, mnact_full);

  M_DrawBackground(g_menu_flat, 0); // Draw background

  // CPhipps - patch drawing updated
  M_DrawTitle(109, 2, "M_AUTO", CR_DEFAULT, "AUTOMAP", cr_title);
  M_DrawInstructions();
  M_DrawScreenItems(current_setup_menu, DEFAULT_LIST_Y);

  // If a color is being selected, need to show color paint chips

  if (colorbox_active)
    M_DrawColPal();
}

/////////////////////////////
//
// The General table.
// killough 10/10/98

setup_menu_t audiovideo_settings[], mouse_settings[], controller_settings[], misc_settings[];
setup_menu_t display_settings[], mapping_settings[], demo_settings[], tas_settings[];

setup_menu_t* gen_settings[] =
{
  audiovideo_settings,
  mouse_settings,
  controller_settings,
  misc_settings,
  display_settings,
  mapping_settings,
  demo_settings,
  tas_settings,
  NULL
};

#define G_X 226

static const char *videomodes[] = {
  "Software",
  "OpenGL",
  NULL};

static const char *gen_skillstrings[] = {
  // Dummy first option because defaultskill is 1-based
  "", "ITYTD", "HNTR", "HMP", "UV", "NM", NULL
};

static const char *gen_compstrings[] =
{
  "Default",
  "Doom v1.2",
  "Doom v1.666",
  "Doom/2 v1.9",
  "Ultimate Doom",
  "Final Doom",
  "DosDoom",
  "TASDoom",
  "Boom's vanilla",
  "Boom v2.01",
  "Boom",
  "LxDoom",
  "MBF",
  "PrBoom 2.03b",
  "PrBoom 2.1.x",
  "PrBoom 2.2.x",
  "PrBoom 2.3.x",
  "PrBoom 2.4.0",
  "Latest PrBoom+",
  "~",
  "~",
  "~",
  "MBF21",
  NULL
};

static const char *death_use_strings[] = { "default", "nothing", "reload", NULL };

static const char *render_aspects_list[] = { "auto", "16:9", "16:10", "4:3", "5:4", NULL };

static const char* render_stretch_list[] = {
  "Not Adjusted", "Doom Format", "Fit to Width", NULL
};

static const char* fake_contrast_list[] =
{
  [FAKE_CONTRAST_MODE_OFF] = "Off",
  [FAKE_CONTRAST_MODE_ON] = "Normal",
  [FAKE_CONTRAST_MODE_SMOOTH] = "Smooth",
  NULL
};

//static const char *gl_fade_mode_list[] = { "Normal", "Smooth", NULL };

setup_menu_t audiovideo_settings[] = {
  { "Video", S_SKIP | S_TITLE, m_null, G_X},
  { "Video mode", S_CHOICE | S_STR, m_conf, G_X, dsda_config_videomode, 0, videomodes },
  //{ "Screen Resolution", S_CHOICE | S_STR, m_conf, G_X, dsda_config_screen_resolution, 0, screen_resolutions_list },
  { "Aspect Ratio", S_CHOICE, m_conf, G_X, dsda_config_render_aspect, 0, render_aspects_list },
  { "Fullscreen Video mode", S_YESNO, m_conf, G_X, dsda_config_use_fullscreen },
  { "Exclusive Fullscreen", S_YESNO, m_conf, G_X, dsda_config_exclusive_fullscreen },
  { "Vertical Sync", S_YESNO, m_conf, G_X, dsda_config_render_vsync },
  { "Uncapped Framerate", S_YESNO, m_conf, G_X, dsda_config_uncapped_framerate },
  { "FPS Limit", S_NUM, m_conf, G_X, dsda_config_fps_limit },
  { "Fake Contrast", S_CHOICE, m_conf, G_X, dsda_config_fake_contrast_mode, 0, fake_contrast_list },
  //{ "GL Light Fade", S_CHOICE, m_conf, G_X, dsda_config_gl_fade_mode, 0, gl_fade_mode_list },
  EMPTY_LINE,
  { "Sound & Music", S_SKIP | S_TITLE, m_null, G_X},
  { "Number of Sound Channels", S_NUM, m_conf, G_X, dsda_config_snd_channels },
  { "Enable v1.1 Pitch Effects", S_YESNO, m_conf, G_X, dsda_config_pitched_sounds },
  { "PC Speaker emulation", S_YESNO, m_conf, G_X, dsda_config_snd_pcspeaker },
  { "Disable Sound Cutoffs", S_YESNO, m_conf, G_X, dsda_config_full_sounds },
  //{ "Preferred MIDI player", S_CHOICE | S_STR, m_conf, G_X, dsda_config_snd_midiplayer, 0, midiplayers },

  NEXT_PAGE(mouse_settings),
  FINAL_ENTRY
};

setup_menu_t mouse_settings[] = {
  { "Mouse Options", S_SKIP | S_TITLE, m_null, G_X},
  //{ "Enable Mouse", S_YESNO, m_conf, G_X, dsda_config_use_mouse },
  EMPTY_LINE,
  { "Horizontal Sensitivity", S_NUM, m_conf, G_X, dsda_config_mouse_sensitivity_horiz },
  { "Vertical Sensitivity", S_NUM, m_conf, G_X, dsda_config_mouse_sensitivity_vert },
  { "Free Look Sensitivity", S_NUM, m_conf, G_X, dsda_config_mouse_sensitivity_mlook },
  { "Acceleration", S_NUM, m_conf, G_X, dsda_config_mouse_acceleration },
  EMPTY_LINE,
  { "Enable Free Look", S_YESNO, m_conf, G_X, dsda_config_freelook },
  { "Invert Free Look", S_YESNO, m_conf, G_X, dsda_config_movement_mouseinvert },
  EMPTY_LINE,
  { "Mouse Strafe Divisor", S_NUM, m_conf, G_X, dsda_config_movement_mousestrafedivisor },
  { "Dbl-Click As Use", S_YESNO, m_conf, G_X, dsda_config_mouse_doubleclick_as_use },
  { "Vertical Mouse Movement", S_YESNO, m_conf, G_X, dsda_config_vertmouse },
  { "Carry Fractional Tics", S_YESNO, m_conf, G_X, dsda_config_mouse_carrytics },
  { "Mouse Stutter Correction", S_YESNO, m_conf, G_X, dsda_config_mouse_stutter_correction },

  PREV_PAGE(audiovideo_settings),
  NEXT_PAGE(controller_settings),
  FINAL_ENTRY
};

setup_menu_t controller_settings[] = {
  //{ "Controller Options", S_SKIP | S_TITLE, m_null, G_X},
  //{ "Enable Controller", S_YESNO, m_conf, G_X, dsda_config_use_game_controller },
  //EMPTY_LINE,
  //{ "Left Horizontal Sensitivity", S_NUM, m_conf, G_X, dsda_config_left_analog_sensitivity_x },
  //{ "Left Vertical Sensitivity", S_NUM, m_conf, G_X, dsda_config_left_analog_sensitivity_y },
  //{ "Right Horizontal Sensitivity", S_NUM, m_conf, G_X, dsda_config_right_analog_sensitivity_x },
  //{ "Right Vertical Sensitivity", S_NUM, m_conf, G_X, dsda_config_right_analog_sensitivity_y },
  { "Acceleration", S_NUM, m_conf, G_X, dsda_config_analog_look_acceleration },
  EMPTY_LINE,
  { "Enable Free Look", S_YESNO, m_conf, G_X, dsda_config_freelook },
  { "Invert Free Look", S_YESNO, m_conf, G_X, dsda_config_invert_analog_look },
  //{ "Swap Analogs", S_YESNO, m_conf, G_X, dsda_config_swap_analogs },
  //EMPTY_LINE,
  //{ "Left Analog Deadzone", S_NUM, m_conf, G_X, dsda_config_left_analog_deadzone },
  //{ "Right Analog Deadzone", S_NUM, m_conf, G_X, dsda_config_right_analog_deadzone },
  //{ "Left Trigger Deadzone", S_NUM, m_conf, G_X, dsda_config_left_trigger_deadzone },
  //{ "Right Trigger Deadzone", S_NUM, m_conf, G_X, dsda_config_right_trigger_deadzone },

  PREV_PAGE(mouse_settings),
  NEXT_PAGE(misc_settings),
  FINAL_ENTRY
};

setup_menu_t misc_settings[] = {
  { "Miscellaneous", S_SKIP | S_TITLE, m_null, G_X},
  { "Default skill level", S_CHOICE, m_conf, G_X, dsda_config_default_skill, 0, gen_skillstrings },
  { "Default compatibility level", S_CHOICE, m_conf, G_X, dsda_config_default_complevel, 0, &gen_compstrings[1] },
  { "Enable Cheat Code Entry", S_YESNO, m_conf, G_X, dsda_config_cheat_codes },
  { "Announce Map On Entry", S_YESNO, m_conf, G_X, dsda_config_announce_map },
  EMPTY_LINE,
  { "Quality Of Life", S_SKIP | S_TITLE, m_null, G_X},
  { "Rewind Interval (s)", S_NUM, m_conf, G_X, dsda_config_auto_key_frame_interval },
  { "Rewind Depth", S_NUM, m_conf, G_X, dsda_config_auto_key_frame_depth },
  { "Rewind Timeout (ms)", S_NUM, m_conf, G_X, dsda_config_auto_key_frame_timeout },
  { "Organize My Save Files", S_YESNO, m_conf, G_X, dsda_config_organized_saves },
  { "Skip Quit Prompt", S_YESNO, m_conf, G_X, dsda_config_skip_quit_prompt },
  { "Death Use Action", S_CHOICE, m_conf, G_X, dsda_config_death_use_action, 0, death_use_strings },
  { "Boom Weapon Auto Switch", S_YESNO, m_conf, G_X, dsda_config_switch_when_ammo_runs_out },
  { "Parallel Same-Sound Limit", S_NUM, m_conf, G_X, dsda_config_parallel_sfx_limit },
  { "Parallel Same-Sound Window", S_NUM, m_conf, G_X, dsda_config_parallel_sfx_window },
  { "Play SFX For Movement Toggles", S_YESNO, m_conf, G_X, dsda_config_movement_toggle_sfx },

  PREV_PAGE(controller_settings),
  NEXT_PAGE(display_settings),
  FINAL_ENTRY
};

setup_menu_t display_settings[] = {
  { "Display Options", S_SKIP | S_TITLE, m_null, G_X},
  { "Use Extended Hud", S_YESNO, m_conf, G_X, dsda_config_exhud },
  { "Ex Hud Scale %", S_NUM, m_conf, G_X, dsda_config_ex_text_scale_x },
  { "Ex Hud Ratio %", S_NUM, m_conf, G_X, dsda_config_ex_text_ratio_y },
  { "Hide Status Bar Horns", S_YESNO, m_conf, G_X, dsda_config_hide_horns },
  { "Hide Weapon", S_YESNO, m_conf, G_X, dsda_config_hide_weapon },
  { "Wipe Screen Effect", S_YESNO,  m_conf, G_X, dsda_config_render_wipescreen },
  { "Show FPS", S_YESNO,  m_conf, G_X, dsda_config_show_fps },
  { "View Bobbing", S_YESNO, m_conf, G_X, dsda_config_viewbob },
  { "Weapon Bobbing", S_YESNO, m_conf, G_X, dsda_config_weaponbob },
  { "Quake Intensity", S_NUM, m_conf, G_X, dsda_config_quake_intensity },
  { "Weapon Attack Alignment", S_CHOICE, m_conf, G_X, dsda_config_weapon_attack_alignment, 0, weapon_attack_alignment_strings },
  EMPTY_LINE,
  { "Change Palette On Pain", S_YESNO, m_conf, G_X, dsda_config_palette_ondamage },
  { "Change Palette On Bonus", S_YESNO, m_conf, G_X, dsda_config_palette_onbonus },
  { "Change Palette On Powers", S_YESNO, m_conf, G_X, dsda_config_palette_onpowers },
  EMPTY_LINE,
  { "Status Bar and Menu Appearance", S_CHOICE, m_conf, G_X, dsda_config_render_stretch_hud, 0, render_stretch_list },
  { "Fullscreen Menu Background", S_YESNO, m_conf, G_X, dsda_config_menu_background },

  PREV_PAGE(misc_settings),
  NEXT_PAGE(mapping_settings),
  FINAL_ENTRY
};

setup_menu_t mapping_settings[] = {
  { "EMULATION", S_SKIP | S_TITLE, m_null, G_X},
  { "WARN ON SPECHITS OVERFLOW", S_YESNO, m_conf, G_X, dsda_config_overrun_spechit_warn },
  { "TRY TO EMULATE IT", S_YESNO, m_conf, G_X, dsda_config_overrun_spechit_emulate },
  { "WARN ON REJECT OVERFLOW", S_YESNO, m_conf, G_X, dsda_config_overrun_reject_warn },
  { "TRY TO EMULATE IT", S_YESNO, m_conf, G_X, dsda_config_overrun_reject_emulate },
  { "WARN ON INTERCEPTS OVERFLOW", S_YESNO, m_conf, G_X, dsda_config_overrun_intercept_warn },
  { "TRY TO EMULATE IT", S_YESNO, m_conf, G_X, dsda_config_overrun_intercept_emulate },
  { "WARN ON PLAYERINGAME OVERFLOW", S_YESNO, m_conf, G_X, dsda_config_overrun_playeringame_warn },
  { "TRY TO EMULATE IT", S_YESNO, m_conf, G_X, dsda_config_overrun_playeringame_emulate },
  EMPTY_LINE,
  { "MAPPING ERROR FIXES", S_SKIP | S_TITLE, m_conf, G_X},
  { "USE PASSES THRU ALL SPECIAL LINES", S_YESNO, m_conf, G_X, dsda_config_comperr_passuse },
  { "WALK UNDER SOLID HANGING BODIES", S_YESNO, m_conf, G_X, dsda_config_comperr_hangsolid },
  { "FIX CLIPPING IN LARGE LEVELS", S_YESNO, m_conf, G_X, dsda_config_comperr_blockmap },
  { "ALLOW VERTICAL AIMING", S_YESNO, m_conf, G_X, dsda_config_comperr_freeaim },

  PREV_PAGE(display_settings),
  NEXT_PAGE(demo_settings),
  FINAL_ENTRY
};

setup_menu_t demo_settings[] = {
  { "Demo Settings", S_SKIP | S_TITLE, m_null, G_X},
  { "Strict Mode", S_YESNO, m_conf, G_X, dsda_config_strict_mode },
  { "Cycle Ghost Colors", S_YESNO, m_conf, G_X, dsda_config_cycle_ghost_colors },
  { "Show Demo Attempts", S_YESNO, m_conf, G_X, dsda_config_show_demo_attempts },
  { "Show Split Data", S_YESNO, m_conf, G_X, dsda_config_show_split_data },
  { "Text File Author", S_NAME, m_conf, G_X, dsda_config_player_name },
  { "Quickstart Cache Tics", S_NUM, m_conf, G_X, dsda_config_quickstart_cache_tics },
  { "Smooth Demo Playback", S_YESNO, m_conf, G_X, dsda_config_demo_smoothturns },
  { "Smooth Demo Playback Factor", S_NUM, m_conf, G_X, dsda_config_demo_smoothturnsfactor },
  { "Show Precise Intermission Time", S_YESNO,  m_conf, G_X, dsda_config_show_level_splits },
  EMPTY_LINE,
  { "Organize Failed Demos", S_YESNO,  m_conf, G_X, dsda_config_organize_failed_demos },
  EMPTY_LINE,
  { "Casual Play Settings", S_SKIP | S_TITLE, m_null, G_X},
  { "Allow Jumping", S_YESNO, m_conf, G_X, dsda_config_allow_jumping },
  //{ "OpenGL Show Health Bars", S_YESNO, m_conf, G_X, dsda_config_gl_health_bar },

  PREV_PAGE(mapping_settings),
  NEXT_PAGE(tas_settings),
  FINAL_ENTRY
};

setup_menu_t tas_settings[] = {
  { "TAS Settings", S_SKIP | S_TITLE, m_null, G_X},
  { "Wipe At Full Speed", S_YESNO, m_conf, G_X, dsda_config_wipe_at_full_speed },
  { "Show Command Display", S_YESNO, m_conf, G_X, dsda_config_command_display },
  { "Command History", S_NUM, m_conf, G_X, dsda_config_command_history_size },
  { "Hide Empty Commands", S_YESNO, m_conf, G_X, dsda_config_hide_empty_commands },
  { "Show Coordinate Display", S_YESNO, m_conf, G_X, dsda_config_coordinate_display },
  { "Permanent Strafe50", S_YESNO, m_conf, G_X, dsda_config_movement_strafe50 },
  { "Strafe50 On Turns", S_YESNO, m_conf, G_X, dsda_config_movement_strafe50onturns },

  PREV_PAGE(demo_settings),
  FINAL_ENTRY
};

// To (un)set fullscreen video after menu changes
void M_ChangeFullScreen(void)
{
  I_UpdateVideoMode();

#if 0
  if (V_IsOpenGLMode())
  {
    gld_PreprocessLevel();
  }
#endif
}

void M_ChangeVideoMode(void)
{
  V_ChangeScreenResolution();
  M_ChangeMaxViewPitch();
}

void M_ChangeUseGLSurface(void)
{
  V_ChangeScreenResolution();
}

void M_ChangeDemoSmoothTurns(void)
{
  R_SmoothPlaying_Reset(NULL);
}

// Setting up for the General screen. Turn on flags, set pointers,
// locate the first item on the screen where the cursor is allowed to
// land.

void M_General(int choice)
{
  M_EnterSetup(&GeneralDef, &set_general_active, gen_settings[0]);
}

// The drawing part of the General Setup initialization. Draw the
// background, title, instruction line, and items.

void M_DrawGeneral(void)
{
  M_ChangeMenu(NULL, mnact_full);

  M_DrawBackground(g_menu_flat, 0); // Draw background

  // proff/nicolas 09/20/98 -- changed for hi-res
  M_DrawTitle(114, 2, "M_GENERL", CR_DEFAULT, "GENERAL", cr_title);
  M_DrawInstructions();
  M_DrawScreenItems(current_setup_menu, DEFAULT_LIST_Y);
}

/////////////////////////////
//
// The level table.
//

#define LEVEL_TABLE_PAGES 3

static setup_menu_t *level_table_page[LEVEL_TABLE_PAGES];
static setup_menu_t next_page_template = NEXT_PAGE(NULL);
static setup_menu_t prev_page_template = PREV_PAGE(NULL);
static setup_menu_t final_entry_template = FINAL_ENTRY;
static setup_menu_t new_column_template = NEW_COLUMN;
static setup_menu_t empty_line_template = EMPTY_LINE;

#define LOOP_LEVEL_TABLE_COLUMN { \
  for (i = 0; i < wad_stats.map_count; ++i) { \
    map = &wad_stats.maps[i]; \
    if (map->episode == -1) \
      continue; \
    entry = &level_table_page[page][base_i + i]; \

#define END_LOOP_LEVEL_TABLE_COLUMN } \
  base_i += i; \
}

#define INSERT_LEVEL_TABLE_COLUMN(heading, x) { \
  level_table_page[page][base_i] = new_column_template; \
  ++base_i; \
  level_table_page[page][base_i] = empty_line_template; \
  level_table_page[page][base_i].m_flags |= S_TITLE; \
  level_table_page[page][base_i].m_text = Z_Strdup(heading); \
  level_table_page[page][base_i].m_x = x; \
  ++base_i; \
}

#define INSERT_FINAL_LEVEL_TABLE_ENTRY { \
  level_table_page[page][base_i] = final_entry_template; \
  level_table_page[page][base_i].m_x = level_table_cursor_position[page]; \
}

#define INSERT_LEVEL_TABLE_EMPTY_LINE { \
  level_table_page[page][base_i] = empty_line_template; \
  ++base_i; \
}

#define INSERT_LEVEL_TABLE_NEXT_PAGE { \
  level_table_page[page][base_i] = next_page_template; \
  level_table_page[page][base_i].menu = level_table_page[page + 1]; \
  ++base_i; \
}

#define INSERT_LEVEL_TABLE_PREV_PAGE { \
  level_table_page[page][base_i] = prev_page_template; \
  level_table_page[page][base_i].menu = level_table_page[page - 1]; \
  ++base_i; \
}

#define START_LEVEL_TABLE_PAGE(page_number) { \
  page = page_number; \
  base_i = 0; \
  column_x = 16; \
  INSERT_LEVEL_TABLE_EMPTY_LINE \
}

static void M_FreeMText(const char *m_text)
{
  union { const char *c; char *s; } str;

  str.c = m_text;
  Z_Free(str.s);
}

typedef struct {
  int completed_count;
  int timed_count;
  int max_timed_count;
  int sk5_timed_count;
  int best_skill;
  int best_kills;
  int best_items;
  int best_secrets;
  int max_kills;
  int max_items;
  int max_secrets;
  int best_time;
  int best_max_time;
  int best_sk5_time;
} wad_stats_summary_t;

static wad_stats_summary_t wad_stats_summary;

static void M_CalculateWadStatsSummary(void)
{
  int i;
  map_stats_t *map;

  memset(&wad_stats_summary, 0, sizeof(wad_stats_summary));

  wad_stats_summary.best_skill = 6;

  for (i = 0; i < wad_stats.map_count; ++i)
  {
    map = &wad_stats.maps[i];
    if (map->episode == -1 || !map->best_skill)
      continue;

    if (map->best_skill < wad_stats_summary.best_skill)
      wad_stats_summary.best_skill = map->best_skill;

    ++wad_stats_summary.completed_count;
    wad_stats_summary.best_kills += map->best_kills;
    wad_stats_summary.best_items += map->best_items;
    wad_stats_summary.best_secrets += map->best_secrets;
    wad_stats_summary.max_kills += map->max_kills;
    wad_stats_summary.max_items += map->max_items;
    wad_stats_summary.max_secrets += map->max_secrets;

    if (map->best_time >= 0)
    {
      ++wad_stats_summary.timed_count;
      wad_stats_summary.best_time += map->best_time;
    }

    if (map->best_max_time >= 0)
    {
      ++wad_stats_summary.max_timed_count;
      wad_stats_summary.best_max_time += map->best_max_time;
    }

    if (map->best_sk5_time >= 0)
    {
      ++wad_stats_summary.sk5_timed_count;
      wad_stats_summary.best_sk5_time += map->best_sk5_time;
    }
  }
}

static int level_table_cursor_position[LEVEL_TABLE_PAGES];

static void M_ResetLevelTable(void)
{
  int i, page;
  const int page_count[LEVEL_TABLE_PAGES] = {
    wad_stats.map_count * 5 + 16,
    wad_stats.map_count * 4 + 16,
    40,
  };

  for (page = 0; page < LEVEL_TABLE_PAGES; ++page)
  {
    if (!level_table_page[page])
      level_table_page[page] = Z_Calloc(page_count[page], sizeof(*level_table_page[page]));
    else
    {
      for (i = 0; !(level_table_page[page][i].m_flags & S_END); ++i)
      {
        if (level_table_page[page][i].m_text &&
            !(level_table_page[page][i].m_flags & (S_NEXT | S_PREV)))
          M_FreeMText(level_table_page[page][i].m_text);
      }

      level_table_cursor_position[page] = level_table_page[page][i].m_x;

      memset(level_table_page[page], 0, page_count[page] * sizeof(*level_table_page[page]));
    }
  }
}

static void M_PrintTime(dsda_string_t* m_text, int tics)
{
  dsda_StringPrintF(m_text, "%d:%05.2f",
                    tics / 35 / 60,
                    (float) (tics % (60 * 35)) / 35);
}

static int wad_stats_summary_page;

static void M_BuildLevelTable(void)
{
  int i;
  int page;
  int base_i;
  int column_x;
  setup_menu_t *entry;
  map_stats_t *map;
  dsda_string_t m_text;

  M_ResetLevelTable();

  START_LEVEL_TABLE_PAGE(0)

  LOOP_LEVEL_TABLE_COLUMN
    dsda_StringPrintF(&m_text, "%s", map->lump);
    entry->m_text = m_text.string;
    entry->m_flags = S_TITLE | S_LEFTJUST;
    entry->m_x = column_x;
  END_LOOP_LEVEL_TABLE_COLUMN

  column_x += 112;
  INSERT_LEVEL_TABLE_COLUMN("SKILL", column_x)

  LOOP_LEVEL_TABLE_COLUMN
    entry->m_flags = S_LABEL | S_SKIP;
    entry->m_x = column_x;

    if (map->best_skill) {
      dsda_StringPrintF(&m_text, "%d", map->best_skill);
      entry->m_text = m_text.string;
      if (map->best_skill == 5)
        entry->m_flags |= S_TC_SEL;
    }
    else {
      entry->m_text = Z_Strdup("-");
    }
  END_LOOP_LEVEL_TABLE_COLUMN

  column_x += 64;
  INSERT_LEVEL_TABLE_COLUMN("K", column_x);

  LOOP_LEVEL_TABLE_COLUMN
    entry->m_flags = S_LABEL | S_SKIP;
    entry->m_x = column_x;

    if (map->best_skill) {
      dsda_StringPrintF(&m_text, "%d/%d", map->best_kills, map->max_kills);
      entry->m_text = m_text.string;
      if (map->best_kills == map->max_kills)
        entry->m_flags |= S_TC_SEL;
    }
    else {
      entry->m_text = Z_Strdup("-");
    }
  END_LOOP_LEVEL_TABLE_COLUMN

  column_x += 48;
  INSERT_LEVEL_TABLE_COLUMN("I", column_x);

  LOOP_LEVEL_TABLE_COLUMN
    entry->m_flags = S_LABEL | S_SKIP;
    entry->m_x = column_x;

    if (map->best_skill) {
      dsda_StringPrintF(&m_text, "%d/%d", map->best_items, map->max_items);
      entry->m_text = m_text.string;
      if (map->best_items == map->max_items)
        entry->m_flags |= S_TC_SEL;
    }
    else {
      entry->m_text = Z_Strdup("-");
    }
  END_LOOP_LEVEL_TABLE_COLUMN

  column_x += 48;
  INSERT_LEVEL_TABLE_COLUMN("S", column_x);

  LOOP_LEVEL_TABLE_COLUMN
    entry->m_flags = S_LABEL | S_SKIP;
    entry->m_x = column_x;

    if (map->best_skill) {
      dsda_StringPrintF(&m_text, "%d/%d", map->best_secrets, map->max_secrets);
      entry->m_text = m_text.string;
      if (map->best_secrets == map->max_secrets)
        entry->m_flags |= S_TC_SEL;
    }
    else {
      entry->m_text = Z_Strdup("-");
    }
  END_LOOP_LEVEL_TABLE_COLUMN

  INSERT_LEVEL_TABLE_NEXT_PAGE
  INSERT_FINAL_LEVEL_TABLE_ENTRY

  // -------- //

  START_LEVEL_TABLE_PAGE(1)

  LOOP_LEVEL_TABLE_COLUMN
    dsda_StringPrintF(&m_text, "%s", map->lump);
    entry->m_text = m_text.string;
    entry->m_flags = S_TITLE | S_LEFTJUST;
    entry->m_x = column_x;
  END_LOOP_LEVEL_TABLE_COLUMN

  column_x += 120;
  INSERT_LEVEL_TABLE_COLUMN("TIME", column_x)

  LOOP_LEVEL_TABLE_COLUMN
    entry->m_flags = S_LABEL | S_SKIP;
    entry->m_x = column_x;

    if (map->best_time >= 0) {
      M_PrintTime(&m_text, map->best_time);
      entry->m_text = m_text.string;
      entry->m_flags |= S_TC_SEL;
    }
    else {
      entry->m_text = Z_Strdup("- : --");
    }
  END_LOOP_LEVEL_TABLE_COLUMN

  column_x += 80;
  INSERT_LEVEL_TABLE_COLUMN("MAX TIME", column_x)

  LOOP_LEVEL_TABLE_COLUMN
    entry->m_flags = S_LABEL | S_SKIP;
    entry->m_x = column_x;

    if (map->best_max_time >= 0) {
      M_PrintTime(&m_text, map->best_max_time);
      entry->m_text = m_text.string;
      entry->m_flags |= S_TC_SEL;
    }
    else {
      entry->m_text = Z_Strdup("- : --");
    }
  END_LOOP_LEVEL_TABLE_COLUMN

  column_x += 80;
  INSERT_LEVEL_TABLE_COLUMN("SK 5 TIME", column_x)

  LOOP_LEVEL_TABLE_COLUMN
    entry->m_flags = S_LABEL | S_SKIP;
    entry->m_x = column_x;

    if (map->best_sk5_time >= 0) {
      M_PrintTime(&m_text, map->best_sk5_time);
      entry->m_text = m_text.string;
      entry->m_flags |= S_TC_SEL;
    }
    else {
      entry->m_text = Z_Strdup("- : --");
    }
  END_LOOP_LEVEL_TABLE_COLUMN

  INSERT_LEVEL_TABLE_PREV_PAGE
  INSERT_LEVEL_TABLE_NEXT_PAGE
  INSERT_FINAL_LEVEL_TABLE_ENTRY

  // -------- //

  M_CalculateWadStatsSummary();

  ++page;
  base_i = 0;
  wad_stats_summary_page = page;

  level_table_page[page][base_i].m_text = Z_Strdup("Summary");
  level_table_page[page][base_i].m_flags = S_TITLE | S_NOSELECT | S_CENTER;
  level_table_page[page][base_i].m_x = 160;
  ++base_i;

  INSERT_LEVEL_TABLE_EMPTY_LINE

  dsda_StringPrintF(&m_text, "Maps");
  level_table_page[page][base_i].m_text = m_text.string;
  level_table_page[page][base_i].m_flags = S_TITLE | S_SKIP;
  level_table_page[page][base_i].m_x = 162;
  ++base_i;

  dsda_StringPrintF(&m_text, "Skill");
  level_table_page[page][base_i].m_text = m_text.string;
  level_table_page[page][base_i].m_flags = S_TITLE | S_SKIP;
  level_table_page[page][base_i].m_x = 162;
  ++base_i;

  INSERT_LEVEL_TABLE_EMPTY_LINE

  dsda_StringPrintF(&m_text, "Kill Completion");
  level_table_page[page][base_i].m_text = m_text.string;
  level_table_page[page][base_i].m_flags = S_TITLE | S_SKIP;
  level_table_page[page][base_i].m_x = 162;
  ++base_i;

  INSERT_LEVEL_TABLE_EMPTY_LINE

  dsda_StringPrintF(&m_text, "Item Completion");
  level_table_page[page][base_i].m_text = m_text.string;
  level_table_page[page][base_i].m_flags = S_TITLE | S_SKIP;
  level_table_page[page][base_i].m_x = 162;
  ++base_i;

  INSERT_LEVEL_TABLE_EMPTY_LINE

  dsda_StringPrintF(&m_text, "Secret Completion");
  level_table_page[page][base_i].m_text = m_text.string;
  level_table_page[page][base_i].m_flags = S_TITLE | S_SKIP;
  level_table_page[page][base_i].m_x = 162;
  ++base_i;

  INSERT_LEVEL_TABLE_EMPTY_LINE

  dsda_StringPrintF(&m_text, "Time");
  level_table_page[page][base_i].m_text = m_text.string;
  level_table_page[page][base_i].m_flags = S_TITLE | S_SKIP;
  level_table_page[page][base_i].m_x = 162;
  ++base_i;

  INSERT_LEVEL_TABLE_EMPTY_LINE

  dsda_StringPrintF(&m_text, "Max Time");
  level_table_page[page][base_i].m_text = m_text.string;
  level_table_page[page][base_i].m_flags = S_TITLE | S_SKIP;
  level_table_page[page][base_i].m_x = 162;
  ++base_i;

  INSERT_LEVEL_TABLE_EMPTY_LINE

  dsda_StringPrintF(&m_text, "Sk 5 Time");
  level_table_page[page][base_i].m_text = m_text.string;
  level_table_page[page][base_i].m_flags = S_TITLE | S_SKIP;
  level_table_page[page][base_i].m_x = 162;
  ++base_i;

  level_table_page[page][base_i] = new_column_template;
  ++base_i;

  INSERT_LEVEL_TABLE_EMPTY_LINE
  INSERT_LEVEL_TABLE_EMPTY_LINE

  dsda_StringPrintF(&m_text, "%d / %d",
                    wad_stats_summary.completed_count, wad_stats.map_count);
  level_table_page[page][base_i].m_text = m_text.string;
  level_table_page[page][base_i].m_flags = S_LABEL | S_SKIP | S_LEFTJUST;
  level_table_page[page][base_i].m_x = 162;
  ++base_i;

  if (wad_stats_summary.completed_count == wad_stats.map_count)
    dsda_StringPrintF(&m_text, "%d", wad_stats_summary.best_skill);
  else
    dsda_StringPrintF(&m_text, "-");
  level_table_page[page][base_i].m_text = m_text.string;
  level_table_page[page][base_i].m_flags = S_LABEL | S_SKIP | S_LEFTJUST;
  level_table_page[page][base_i].m_x = 162;
  ++base_i;

  INSERT_LEVEL_TABLE_EMPTY_LINE

  dsda_StringPrintF(&m_text, "%d / ", wad_stats_summary.best_kills);
  if (wad_stats_summary.completed_count == wad_stats.map_count)
    dsda_StringCatF(&m_text, "%d", wad_stats_summary.max_kills);
  else
    dsda_StringCat(&m_text, "-");
  level_table_page[page][base_i].m_text = m_text.string;
  level_table_page[page][base_i].m_flags = S_LABEL | S_SKIP | S_LEFTJUST;
  level_table_page[page][base_i].m_x = 162;
  ++base_i;

  INSERT_LEVEL_TABLE_EMPTY_LINE

  dsda_StringPrintF(&m_text, "%d / ", wad_stats_summary.best_items);
  if (wad_stats_summary.completed_count == wad_stats.map_count)
    dsda_StringCatF(&m_text, "%d", wad_stats_summary.max_items);
  else
    dsda_StringCat(&m_text, "-");
  level_table_page[page][base_i].m_text = m_text.string;
  level_table_page[page][base_i].m_flags = S_LABEL | S_SKIP | S_LEFTJUST;
  level_table_page[page][base_i].m_x = 162;
  ++base_i;

  INSERT_LEVEL_TABLE_EMPTY_LINE

  dsda_StringPrintF(&m_text, "%d / ", wad_stats_summary.best_secrets);
  if (wad_stats_summary.completed_count == wad_stats.map_count)
    dsda_StringCatF(&m_text, "%d", wad_stats_summary.max_secrets);
  else
    dsda_StringCat(&m_text, "-");
  level_table_page[page][base_i].m_text = m_text.string;
  level_table_page[page][base_i].m_flags = S_LABEL | S_SKIP | S_LEFTJUST;
  level_table_page[page][base_i].m_x = 162;
  ++base_i;

  INSERT_LEVEL_TABLE_EMPTY_LINE

  if (wad_stats_summary.timed_count == wad_stats.map_count)
    M_PrintTime(&m_text, wad_stats_summary.best_time);
  else
    dsda_StringPrintF(&m_text, "- : --");
  level_table_page[page][base_i].m_text = m_text.string;
  level_table_page[page][base_i].m_flags = S_LABEL | S_SKIP | S_LEFTJUST;
  level_table_page[page][base_i].m_x = 162;
  ++base_i;

  INSERT_LEVEL_TABLE_EMPTY_LINE

  if (wad_stats_summary.max_timed_count == wad_stats.map_count)
    M_PrintTime(&m_text, wad_stats_summary.best_max_time);
  else
    dsda_StringPrintF(&m_text, "- : --");
  level_table_page[page][base_i].m_text = m_text.string;
  level_table_page[page][base_i].m_flags = S_LABEL | S_SKIP | S_LEFTJUST;
  level_table_page[page][base_i].m_x = 162;
  ++base_i;

  INSERT_LEVEL_TABLE_EMPTY_LINE

  if (wad_stats_summary.sk5_timed_count == wad_stats.map_count)
    M_PrintTime(&m_text, wad_stats_summary.best_sk5_time);
  else
    dsda_StringPrintF(&m_text, "- : --");
  level_table_page[page][base_i].m_text = m_text.string;
  level_table_page[page][base_i].m_flags = S_LABEL | S_SKIP | S_LEFTJUST;
  level_table_page[page][base_i].m_x = 162;
  ++base_i;

  level_table_page[page][base_i] = new_column_template;
  ++base_i;

  INSERT_LEVEL_TABLE_PREV_PAGE
  INSERT_FINAL_LEVEL_TABLE_ENTRY
}

void M_LevelTable(int choice)
{
  M_BuildLevelTable();
  M_EnterSetup(&LevelTableDef, &level_table_active, level_table_page[0]);
}

void M_DrawLevelTable(void)
{
  M_ChangeMenu(NULL, mnact_full);

  M_DrawBackground(g_menu_flat, 0);

  M_DrawTitle(114, 2, "M_LVLTBL", CR_DEFAULT, "LEVEL TABLE", cr_title);
  if (current_setup_menu != level_table_page[wad_stats_summary_page])
    M_DrawInstructionString(cr_info_edit, "Press ENTER key to warp");
  M_DrawScreenItems(current_setup_menu, DEFAULT_LIST_Y);
}

/////////////////////////////
//
// General routines used by the Setup screens.
//

static dboolean shiftdown = false; // phares 4/10/98: SHIFT key down or not

// phares 4/17/98:
// M_SelectDone() gets called when you have finished entering your
// Setup Menu item change.

static void M_SelectDone(setup_menu_t* ptr)
{
  ptr->m_flags &= ~S_SELECT;
  ptr->m_flags |= S_HILITE;
  S_StartVoidSound(g_sfx_itemup);
  setup_select = false;
  colorbox_active = false;
}

//
// End of Setup Screens.
//
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//
// Start of Extended HELP screens               // phares 3/30/98
//
// The wad designer can define a set of extended HELP screens for their own
// information display. These screens should be 320x200 graphic lumps
// defined in a separate wad. They should be named "HELP01" through "HELP99".
// "HELP01" is shown after the regular BOOM Dynamic HELP screen, and ENTER
// and BACKSPACE keys move the player through the HELP set.
//
// Rather than define a set of menu definitions for each of the possible
// HELP screens, one definition is used, and is altered on the fly
// depending on what HELPnn lumps the game finds.

// phares 3/30/98:
// Extended Help Screen variables

int extended_help_count;   // number of user-defined help screens found
int extended_help_index;   // index of current extended help screen

menuitem_t ExtHelpMenu[] =
{
  {1,"",M_ExtHelpNextScreen,0}
};

menu_t ExtHelpDef =
{
  1,             // # of menu items
  &ReadDef1,     // previous menu
  ExtHelpMenu,   // menuitem_t ->
  M_DrawExtHelp, // drawing routine ->
  330,181,       // x,y
  0              // lastOn
};

// M_ExtHelpNextScreen establishes the number of the next HELP screen in
// the series.

void M_ExtHelpNextScreen(int choice)
{
  choice = 0;
  if (++extended_help_index > extended_help_count)
    {

      // when finished with extended help screens, return to Main Menu

      extended_help_index = 1;
      M_SetupNextMenu(&MainDef);
    }
}

// phares 3/30/98:
// Routine to look for HELPnn screens and create a menu
// definition structure that defines extended help screens.

void M_InitExtendedHelp(void)

{
  int index;
  char namebfr[] = { "HELPnn"} ;

  extended_help_count = 0;
  for (index = 1 ; index < 100 ; index++) {
    namebfr[4] = index/10 + '0';
    namebfr[5] = index%10 + '0';
    if (!W_LumpNameExists(namebfr)) {
      if (extended_help_count) {
        /* The Extended Help menu is accessed using the
         * Help hotkey (F1) or the "Read This!" menu item.
         *
         * If Extended Help screens are present, use the
         * Extended Help routine when either the F1 Help Menu
         * or the "Read This!" menu items are accessed.
         *
         * See also: https://www.doomworld.com/forum/topic/111465-boom-extended-help-screens-an-undocumented-feature/
         */
          HelpMenu[0].routine = M_ExtHelp;
        if (gamemode == commercial) {
          ExtHelpDef.prevMenu  = &ReadDef1; /* previous menu */
          ReadMenu1[0].routine = M_ExtHelp;
        } else {
          ExtHelpDef.prevMenu  = &ReadDef2; /* previous menu */
          ReadMenu2[0].routine = M_ExtHelp;
        }
      }
      return;
    }
    extended_help_count++;
  }
}

// Initialization for the extended HELP screens.

void M_ExtHelp(int choice)
{
  choice = 0;
  extended_help_index = 1; // Start with first extended help screen
  M_SetupNextMenu(&ExtHelpDef);
}

// Initialize the drawing part of the extended HELP screens.

void M_DrawExtHelp(void)
{
  char namebfr[10] = { "HELPnn" }; // CPhipps - make it local & writable

  inhelpscreens = true;              // killough 5/1/98
  namebfr[4] = extended_help_index/10 + '0';
  namebfr[5] = extended_help_index%10 + '0';
  // CPhipps - patch drawing updated
  V_DrawNamePatch(0, 0, 0, namebfr, CR_DEFAULT, VPT_STRETCH);
}

//
// End of Extended HELP screens               // phares 3/30/98
//
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
//
// Dynamic HELP screen                     // phares 3/2/98
//
// Rather than providing the static HELP screens from DOOM and its versions,
// BOOM provides the player with a dynamic HELP screen that displays the
// current settings of major key bindings.
//
// The Dynamic HELP screen is defined in a manner similar to that used for
// the Setup Screens above.
//
// M_GetKeyString finds the correct string to represent the key binding
// for the current item being drawn.

int M_GetKeyString(int c,int offset)
{
  const char* s;

  if (c >= 33 && c <= 126) {

    // The '=', ',', and '.' keys originally meant the shifted
    // versions of those keys, but w/o having to shift them in
    // the game. Any actions that are mapped to these keys will
    // still mean their shifted versions. Could be changed later
    // if someone can come up with a better way to deal with them.

    if (c == '=')      // probably means the '+' key?
      c = '+';
    else if (c == ',') // probably means the '<' key?
      c = '<';
    else if (c == '.') // probably means the '>' key?
      c = '>';
    menu_buffer[offset++] = c; // Just insert the ascii key
    menu_buffer[offset] = 0;

  } else {

    // Retrieve 4-letter (max) string representing the key

    // cph - Keypad keys, general code reorganisation to
    //  make this smaller and neater.
    if ((0x100 <= c) && (c < 0x200)) {
      if (c == KEYD_KEYPADENTER)
  s = "PADE";
      else {
  strcpy(&menu_buffer[offset], "PAD");
  offset+=4;
  menu_buffer[offset-1] = c & 0xff;
  menu_buffer[offset] = 0;
      }
    } else if ((KEYD_F1 <= c) && (c < KEYD_F10)) {
      menu_buffer[offset++] = 'F';
      menu_buffer[offset++] = '1' + c - KEYD_F1;
      menu_buffer[offset]   = 0;
    } else {
      switch(c) {
      case KEYD_TAB:      s = "TAB";  break;
      case KEYD_ENTER:      s = "ENTR"; break;
      case KEYD_ESCAPE:     s = "ESC";  break;
      case KEYD_SPACEBAR:   s = "SPAC"; break;
      case KEYD_BACKSPACE:  s = "BACK"; break;
      case KEYD_RCTRL:      s = "CTRL"; break;
      case KEYD_LEFTARROW:  s = "LARR"; break;
      case KEYD_UPARROW:    s = "UARR"; break;
      case KEYD_RIGHTARROW: s = "RARR"; break;
      case KEYD_DOWNARROW:  s = "DARR"; break;
      case KEYD_RSHIFT:     s = "SHFT"; break;
      case KEYD_RALT:       s = "ALT";  break;
      case KEYD_CAPSLOCK:   s = "CAPS"; break;
      case KEYD_SCROLLLOCK: s = "SCRL"; break;
      case KEYD_HOME:       s = "HOME"; break;
      case KEYD_PAGEUP:     s = "PGUP"; break;
      case KEYD_END:        s = "END";  break;
      case KEYD_PAGEDOWN:   s = "PGDN"; break;
      case KEYD_INSERT:     s = "INST"; break;
      case KEYD_DEL:        s = "DEL"; break;
      case KEYD_F10:        s = "F10";  break;
      case KEYD_F11:        s = "F11";  break;
      case KEYD_F12:        s = "F12";  break;
      case KEYD_PAUSE:      s = "PAUS"; break;
      case KEYD_MWHEELDOWN: s = "MWDN"; break;
      case KEYD_MWHEELUP:   s = "MWUP"; break;
      case KEYD_PRINTSC:    s = "PRSC"; break;
      case 0:               s = "NONE"; break;
      default:              s = "JUNK"; break;
      }

      if (s) { // cph - Slight code change
  strcpy(&menu_buffer[offset],s); // string to display
  offset += strlen(s);
      }
    }
  }
  return offset;
}

//
// The Dynamic HELP screen table.

#define KT_X1 283
#define KT_X2 172
#define KT_X3  87

setup_menu_t helpstrings[] =  // HELP screen strings
{
  {"SCREEN"      ,S_SKIP|S_TITLE,m_null,KT_X1},
  {"HELP"        ,S_SKIP|S_INPUT,m_null,KT_X1,0,dsda_input_help},
  {"MENU"        ,S_SKIP|S_INPUT,m_null,KT_X1,0,dsda_input_escape},
  {"PAUSE"       ,S_SKIP|S_INPUT,m_null,KT_X1,0,dsda_input_pause},
  {"AUTOMAP"     ,S_SKIP|S_INPUT,m_null,KT_X1,0,dsda_input_map},
  {"SOUND VOLUME",S_SKIP|S_INPUT,m_null,KT_X1,0,dsda_input_soundvolume},
  {"HUD"         ,S_SKIP|S_INPUT,m_null,KT_X1,0,dsda_input_hud},
  {"MESSAGES"    ,S_SKIP|S_INPUT,m_null,KT_X1,0,dsda_input_messages},
  {"GAMMA FIX"   ,S_SKIP|S_INPUT,m_null,KT_X1,0,dsda_input_gamma},
  {"SPY"         ,S_SKIP|S_INPUT,m_null,KT_X1,0,dsda_input_spy},
  {"LARGER VIEW" ,S_SKIP|S_INPUT,m_null,KT_X1,0,dsda_input_zoomin},
  {"SMALLER VIEW",S_SKIP|S_INPUT,m_null,KT_X1,0,dsda_input_zoomout},
  {"SCREENSHOT"  ,S_SKIP|S_INPUT,m_null,KT_X1,0,dsda_input_screenshot},
  EMPTY_LINE,
  {"AUTOMAP"     ,S_SKIP|S_TITLE,m_null,KT_X1},
  {"FOLLOW MODE" ,S_SKIP|S_INPUT,m_null,KT_X1,0,dsda_input_map_follow},
  {"ZOOM IN"     ,S_SKIP|S_INPUT,m_null,KT_X1,0,dsda_input_map_zoomin},
  {"ZOOM OUT"    ,S_SKIP|S_INPUT,m_null,KT_X1,0,dsda_input_map_zoomout},
  {"MARK PLACE"  ,S_SKIP|S_INPUT,m_null,KT_X1,0,dsda_input_map_mark},
  {"CLEAR MARKS" ,S_SKIP|S_INPUT,m_null,KT_X1,0,dsda_input_map_clear},
  {"FULL/ZOOM"   ,S_SKIP|S_INPUT,m_null,KT_X1,0,dsda_input_map_gobig},
  {"GRID"        ,S_SKIP|S_INPUT,m_null,KT_X1,0,dsda_input_map_grid},
  {"ROTATE"      ,S_SKIP|S_INPUT,m_null,KT_X1,0,dsda_input_map_rotate},
  {"OVERLAY"     ,S_SKIP|S_INPUT,m_null,KT_X1,0,dsda_input_map_overlay},
  NEW_COLUMN,
  {"WEAPONS"     ,S_SKIP|S_TITLE,m_null,KT_X3},
  {"FIST"        ,S_SKIP|S_INPUT,m_null,KT_X3,0,dsda_input_weapon1},
  {"PISTOL"      ,S_SKIP|S_INPUT,m_null,KT_X3,0,dsda_input_weapon2},
  {"SHOTGUN"     ,S_SKIP|S_INPUT,m_null,KT_X3,0,dsda_input_weapon3},
  {"CHAINGUN"    ,S_SKIP|S_INPUT,m_null,KT_X3,0,dsda_input_weapon4},
  {"ROCKET"      ,S_SKIP|S_INPUT,m_null,KT_X3,0,dsda_input_weapon5},
  {"PLASMA"      ,S_SKIP|S_INPUT,m_null,KT_X3,0,dsda_input_weapon6},
  {"BFG 9000"    ,S_SKIP|S_INPUT,m_null,KT_X3,0,dsda_input_weapon7},
  {"CHAINSAW"    ,S_SKIP|S_INPUT,m_null,KT_X3,0,dsda_input_weapon8},
  {"SSG"         ,S_SKIP|S_INPUT,m_null,KT_X3,0,dsda_input_weapon9},
  {"BEST"        ,S_SKIP|S_INPUT,m_null,KT_X3,0,dsda_input_toggleweapon},
  {"FIRE"        ,S_SKIP|S_INPUT,m_null,KT_X3,0,dsda_input_fire},
  EMPTY_LINE,
  {"MOVEMENT"    ,S_SKIP|S_TITLE,m_null,KT_X3},
  {"FORWARD"     ,S_SKIP|S_INPUT,m_null,KT_X3,0,dsda_input_forward},
  {"BACKWARD"    ,S_SKIP|S_INPUT,m_null,KT_X3,0,dsda_input_backward},
  {"TURN LEFT"   ,S_SKIP|S_INPUT,m_null,KT_X3,0,dsda_input_turnleft},
  {"TURN RIGHT"  ,S_SKIP|S_INPUT,m_null,KT_X3,0,dsda_input_turnright},
  {"RUN"         ,S_SKIP|S_INPUT,m_null,KT_X3,0,dsda_input_speed},
  {"STRAFE LEFT" ,S_SKIP|S_INPUT,m_null,KT_X3,0,dsda_input_strafeleft},
  {"STRAFE RIGHT",S_SKIP|S_INPUT,m_null,KT_X3,0,dsda_input_straferight},
  {"STRAFE"      ,S_SKIP|S_INPUT,m_null,KT_X3,0,dsda_input_strafe},
  {"AUTORUN"     ,S_SKIP|S_INPUT,m_null,KT_X3,0,dsda_input_autorun},
  {"180 TURN"    ,S_SKIP|S_INPUT,m_null,KT_X3,0,dsda_input_reverse},
  {"USE"         ,S_SKIP|S_INPUT,m_null,KT_X3,0,dsda_input_use},
  NEW_COLUMN,
  {"GAME"        ,S_SKIP|S_TITLE,m_null,KT_X2},
  {"SAVE"        ,S_SKIP|S_INPUT,m_null,KT_X2,0,dsda_input_savegame},
  {"LOAD"        ,S_SKIP|S_INPUT,m_null,KT_X2,0,dsda_input_loadgame},
  {"QUICKSAVE"   ,S_SKIP|S_INPUT,m_null,KT_X2,0,dsda_input_quicksave},
  {"END GAME"    ,S_SKIP|S_INPUT,m_null,KT_X2,0,dsda_input_endgame},
  {"QUICKLOAD"   ,S_SKIP|S_INPUT,m_null,KT_X2,0,dsda_input_quickload},
  {"QUIT"        ,S_SKIP|S_INPUT,m_null,KT_X2,0,dsda_input_quit},

  FINAL_ENTRY
};

/* cph 2006/08/06
 * M_DrawString() is the old M_DrawMenuString, except that it is not tied to
 * menu_buffer - no reason to force all the callers to write into one array! */

static void M_DrawString(int cx, int cy, int color, const char* ch)
{
  int   w;
  int   c;

  while (*ch) {
    c = *ch++;         // get next char
    c = toupper(c) - HU_FONTSTART;
    if (c < 0 || c> HU_FONTSIZE)
      {
      cx += menu_font->space_width;
      continue;
      }
    w = menu_font->font[c].width;
    if (cx + w > 320)
      break;

    // V_DrawpatchTranslated() will draw the string in the
    // desired color, colrngs[color]

    // CPhipps - patch drawing updated
    V_DrawNumPatch(cx, cy, 0, menu_font->font[c].lumpnum, color, VPT_STRETCH | VPT_TRANS);
    // The screen is cramped, so trim one unit from each
    // character so they butt up against each other.
    cx += w + g_menu_font_spacing;
  }
}

// M_DrawMenuString() draws the string in menu_buffer[]

static void M_DrawMenuString(int cx, int cy, int color)
{
    M_DrawString(cx, cy, color, menu_buffer);
}

// M_GetPixelWidth() returns the number of pixels in the width of
// the string, NOT the number of chars in the string.

static int M_GetPixelWidth(const char* ch)
{
  int len = 0;
  int c;

  while (*ch) {
    c = *ch++;    // pick up next char
    c = toupper(c) - HU_FONTSTART;
    if (c < 0 || c > HU_FONTSIZE)
      {
      len += menu_font->space_width;
      continue;
      }
    len += menu_font->font[c].width;
    len += g_menu_font_spacing;
  }
  len -= g_menu_font_spacing; // replace what you took away on the last char only
  return len;
}

static void M_DrawStringCentered(int cx, int cy, int color, const char* ch)
{
    M_DrawString(cx - M_GetPixelWidth(ch)/2, cy, color, ch);
}

//
// M_DrawHelp
//
// This displays the help screen

void M_DrawHelp (void)
{
  const int helplump = W_CheckNumForName("HELP");

  M_ChangeMenu(NULL, mnact_full);

  if (helplump != LUMP_NOT_FOUND && lumpinfo[helplump].source != source_iwad)
  {
    V_ClearBorder();
    V_DrawNumPatch(0, 0, 0, helplump, CR_DEFAULT, VPT_STRETCH);
  }
  else
  {
    M_DrawBackground(g_menu_flat, 0);
    M_DrawScreenItems(helpstrings, 2);
  }
}

//
// End of Dynamic HELP screen                // phares 3/2/98
//
////////////////////////////////////////////////////////////////////////////

#define CR_X 20
#define CR_X2 50

setup_menu_t cred_settings[]={
  {"Programmers",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X},
  {"Florian 'Proff' Schulze",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X2},
  {"Colin Phipps",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X2},
  {"Neil Stevens",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X2},
  {"Andrey Budko",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X2},
  EMPTY_LINE,
  {"Additional Credit To",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X},
  {"id Software for DOOM",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X2},
  {"TeamTNT for BOOM",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X2},
  {"Lee Killough for MBF",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X2},
  {"The DOSDoom-Team for DOSDOOM",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X2},
  {"Randy Heit for ZDOOM",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X2},
  {"Michael 'Kodak' Ryssen for DOOMGL",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X2},
  {"Jess Haas for lSDLDoom",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X2},
  {"all others who helped (see AUTHORS file)",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X2},

  FINAL_ENTRY
};

void M_DrawCredits(void)     // killough 10/98: credit screen
{
  const int creditlump = W_CheckNumForName("CREDIT");

  if (raven)
  {
    V_DrawRawScreen("CREDIT");
    return;
  }

  inhelpscreens = true;
  if (creditlump != LUMP_NOT_FOUND && lumpinfo[creditlump].source != source_iwad)
  {
    V_ClearBorder();
    V_DrawNumPatch(0, 0, 0, creditlump, CR_DEFAULT, VPT_STRETCH);
  }
  else
  {
    // Use V_DrawBackground here deliberately to force drawing a background
    V_DrawBackground(gamemode==shareware ? "CEIL5_1" : "MFLR8_4", 0);
    M_DrawTitle(81, 9, "PRBOOM", cr_title, PACKAGE_NAME " v" PACKAGE_VERSION, cr_title);
    M_DrawScreenItems(cred_settings, 32);
  }
}

static int M_IndexInChoices(const char *str, const char **choices) {
  int i = 0;

  while (*choices != NULL) {
    if (!strcmp(str, *choices))
      return i;
    i++;
    choices++;
  }
  return 0;
}

// [FG] support more joystick and mouse buttons

static inline int GetButtons(const unsigned int max, int data)
{
  int i;

  for (i = 0; i < max; ++i)
  {
    if (data & (1 << i))
    {
      return i;
    }
  }

  return -1;
}

typedef struct {
  int input;
  dsda_config_identifier_t config_id;
  int allowed_in_strict_mode;
  dboolean persist;
  const char* message;
  dboolean invert_message;
  dboolean play_sound;
} toggle_input_t;

static toggle_input_t toggle_inputs[] = {
  { dsda_input_strict_mode, dsda_config_strict_mode, true, false, "Strict Mode" },
  { dsda_input_novert, dsda_config_vertmouse, true, false, "Vertical Mouse Movement", .play_sound = true },
  { dsda_input_mlook, dsda_config_freelook, true, true, "Free Look", .play_sound = true },
  { dsda_input_autorun, dsda_config_autorun, true, true, "Auto Run", .play_sound = true },
  { dsda_input_messages, dsda_config_show_messages, true, true, "Messages" },
  { dsda_input_command_display, dsda_config_command_display, false, true, "Command Display" },
  { dsda_input_coordinate_display, dsda_config_coordinate_display, false, true, "Coordinate Display" },
  { dsda_input_fps, dsda_config_show_fps, true, true, "FPS" },
  { dsda_input_exhud, dsda_config_exhud, true, true, "Extended HUD" },
  { dsda_input_mute_sfx, dsda_config_mute_sfx, true, true, "SFX", true },
  { dsda_input_mute_music, dsda_config_mute_music, true, true, "Music", true },
  { dsda_input_cheat_codes, dsda_config_cheat_codes, false, true, "Cheat Codes" },
  { -1 }
};

static void M_HandleToggles(void)
{
  toggle_input_t* toggle;

  for (toggle = toggle_inputs; toggle->input != -1; toggle++) {
    if (
      dsda_InputActivated(toggle->input) &&
      (toggle->allowed_in_strict_mode || !dsda_StrictMode())
    )
    {
      int value;

      value = dsda_ToggleConfig(toggle->config_id, toggle->persist);
      doom_printf("%s %s", toggle->message, value ? toggle->invert_message ? "off" : "on"
                                                  : toggle->invert_message ? "on"  : "off");

      if (toggle->play_sound && dsda_IntConfig(dsda_config_movement_toggle_sfx))
      {
        if (toggle->invert_message ? !value : value)
        {
          S_StartVoidSound(g_sfx_console);
        }
        else
        {
          S_StartVoidSound(g_sfx_oof);
        }
      }
    }
  }
}

dboolean M_ConsoleOpen(void)
{
  return menuactive && currentMenu == &dsda_ConsoleDef;
}

static void M_LeaveSetupMenu(void)
{
  M_SetSetupMenuItemOn(set_menu_itemon);
  setup_active = false;
  set_keybnd_active = false;
  set_weapon_active = false;
  set_status_active = false;
  set_auto_active = false;
  colorbox_active = false;
  set_general_active = false;
  level_table_active = false;
}

/////////////////////////////////////////////////////////////////////////////
//
// M_Responder
//
// Examines incoming keystrokes and button pushes and determines some
// action based on the state of the system.
//

static dboolean M_KeyBndResponder(int ch, int action, event_t* ev)
{
  // changing an entry
  if (setup_select)
  {
    int i;
    setup_menu_t *ptr1 = current_setup_menu + set_menu_itemon;
    setup_menu_t *ptr2 = NULL;

    int s_input = (ptr1->m_flags & S_INPUT) ? ptr1->input : 0;

    if (ev->type == ev_joystick)
    {
      setup_group group;
      dboolean search = true;

      if (!s_input)
        return true; // not a legal action here (yet)

      // see if the button is already bound elsewhere. if so, you
      // have to swap bindings so the action where it's currently
      // bound doesn't go dead. Since there is more than one
      // keybinding screen, you have to search all of them for
      // any duplicates. You're only interested in the items
      // that belong to the same group as the one you're changing.

      group  = ptr1->m_group;
      if ((ch = GetButtons(MAX_JOY_BUTTONS, ev->data1.i)) == -1)
        return true;

      for (i = 0; keys_settings[i] && search; i++)
        for (ptr2 = keys_settings[i]; !(ptr2->m_flags & S_END); ptr2++)
          if (ptr2->m_group == group && ptr1 != ptr2)
          {
            if (ptr2->m_flags & S_INPUT)
              if (dsda_InputMatchJoyB(ptr2->input, ch))
              {
                dsda_InputRemoveJoyB(ptr2->input, ch);
                search = false;
                break;
              }
          }

      dsda_InputAddJoyB(s_input, ch);
    }
    else if (ev->type == ev_mouse)
    {
      int i;
      setup_group group;
      dboolean search = true;

      if (!s_input)
        return true; // not a legal action here (yet)

      // see if the button is already bound elsewhere. if so, you
      // have to swap bindings so the action where it's currently
      // bound doesn't go dead. Since there is more than one
      // keybinding screen, you have to search all of them for
      // any duplicates. You're only interested in the items
      // that belong to the same group as the one you're changing.

      group  = ptr1->m_group;
      if ((ch = GetButtons(MAX_MOUSE_BUTTONS, ev->data1.i)) == -1)
        return true;

      for (i = 0 ; keys_settings[i] && search ; i++)
        for (ptr2 = keys_settings[i]; !(ptr2->m_flags & S_END); ptr2++)
          if (ptr2->m_group == group && ptr1 != ptr2)
          {
            if (ptr2->m_flags & S_INPUT)
              if (dsda_InputMatchMouseB(ptr2->input, ch))
              {
                dsda_InputRemoveMouseB(ptr2->input, ch);
                search = false;
                break;
              }
          }

      dsda_InputAddMouseB(s_input, ch);
    }
    else  // keyboard key
    {
      int i;
      setup_group group;
      dboolean search = true;

      // see if 'ch' is already bound elsewhere. if so, you have
      // to swap bindings so the action where it's currently
      // bound doesn't go dead. Since there is more than one
      // keybinding screen, you have to search all of them for
      // any duplicates. You're only interested in the items
      // that belong to the same group as the one you're changing.

      // if you find that you're trying to swap with an action
      // that has S_KEEP set, you can't bind ch; it's already
      // bound to that S_KEEP action, and that action has to
      // keep that key.

      group  = ptr1->m_group;
      for (i = 0; keys_settings[i] && search; i++)
        for (ptr2 = keys_settings[i]; !(ptr2->m_flags & S_END); ptr2++)
          if (ptr2->m_group == group && ptr1 != ptr2)
          {
            if (ptr2->m_flags & (S_INPUT | S_KEEP))
              if (dsda_InputMatchKey(ptr2->input, ch))
              {
                if (ptr2->m_flags & S_KEEP)
                  return true; // can't have it!

                dsda_InputRemoveKey(ptr2->input, ch);
                search = false;
                break;
              }
          }

      dsda_InputAddKey(s_input, ch);
    }

    M_SelectDone(ptr1);       // phares 4/17/98
    return true;
  }

  return false;
}

static dboolean M_WeaponResponder(int ch, int action, event_t* ev)
{
  // changing an entry
  if (setup_select)
  {
    setup_menu_t *ptr1 = current_setup_menu + set_menu_itemon;
    setup_menu_t *ptr2 = NULL;

    if (action != MENU_ENTER)
    {
      int old_value;

      ch -= '0'; // out of ascii
      if (ch < 1 || ch > 9)
        return true; // ignore

      // see if 'ch' is already assigned elsewhere. if so,
      // you have to swap assignments.
      ptr2 = weap_settings1;
      old_value = dsda_PersistentIntConfig(ptr1->config_id);
      for (; !(ptr2->m_flags & S_END); ptr2++)
        if (ptr2->m_flags & S_WEAP && ptr1 != ptr2 &&
            dsda_PersistentIntConfig(ptr2->config_id) == ch)
        {
          dsda_UpdateIntConfig(ptr2->config_id, old_value, true);
          break;
        }

      dsda_UpdateIntConfig(ptr1->config_id, ch, true);
    }

    M_SelectDone(ptr1);       // phares 4/17/98
    return true;
  }

  return false;
}

static dboolean M_AutoResponder(int ch, int action, event_t* ev)
{
  // changing an entry
  if (setup_select)
  {
    if (action == MENU_DOWN)
    {
      if (++color_palette_y == 16)
        color_palette_y = 0;
      S_StartVoidSound(g_sfx_itemup);
      return true;
    }

    if (action == MENU_UP)
    {
      if (--color_palette_y < 0)
        color_palette_y = 15;
      S_StartVoidSound(g_sfx_itemup);
      return true;
    }

    if (action == MENU_LEFT)
    {
      if (--color_palette_x < 0)
        color_palette_x = 15;
      S_StartVoidSound(g_sfx_itemup);
      return true;
    }

    if (action == MENU_RIGHT)
    {
      if (++color_palette_x == 16)
        color_palette_x = 0;
      S_StartVoidSound(g_sfx_itemup);
      return true;
    }

    if (action == MENU_ENTER)
    {
      setup_menu_t *ptr1 = current_setup_menu + set_menu_itemon;

      dsda_UpdateIntConfig(ptr1->config_id, color_palette_x + 16 * color_palette_y, true);
      M_SelectDone(ptr1);                         // phares 4/17/98
      colorbox_active = false;
      return true;
    }
  }

  return false;
}

static dboolean M_StringResponder(int ch, int action, event_t* ev)
{
  // changing an entry
  if (setup_select)
  {
    setup_menu_t *ptr1 = current_setup_menu + set_menu_itemon;

    if (ptr1->m_flags & S_STRING) // creating/editing a string?
    {
      if (action == MENU_BACKSPACE) // backspace and DEL
      {
        if (entry_string_index[entry_index] == 0)
        {
          if (entry_index > 0)
            entry_string_index[--entry_index] = 0;
        }
        // shift the remainder of the text one char left
        else
        {
          int i;

          for (i = entry_index; entry_string_index[i + 1]; ++i)
            entry_string_index[i] = entry_string_index[i + 1];
          entry_string_index[i] = '\0';
        }
      }
      else if (action == MENU_LEFT) // move cursor left
      {
        if (entry_index > 0)
          entry_index--;
      }
      else if (action == MENU_RIGHT) // move cursor right
      {
        if (entry_string_index[entry_index] != 0)
          entry_index++;
      }
      else if ((action == MENU_ENTER) || (action == MENU_ESCAPE))
      {
        dsda_UpdateStringConfig(ptr1->config_id, entry_string_index, true);
        M_SelectDone(ptr1);   // phares 4/17/98
      }

      // Adding a char to the text. Has to be a printable
      // char, and you can't overrun the buffer. If the
      // string gets larger than what the screen can hold,
      // it is dealt with when the string is drawn (above).

      else if ((ch >= 32) && (ch <= 126))
        if ((entry_index + 1) < ENTRY_STRING_BFR_SIZE)
        {
          if (shiftdown)
            ch = shiftxform[ch];
          if (entry_string_index[entry_index] == 0)
          {
            entry_string_index[entry_index++] = ch;
            entry_string_index[entry_index] = 0;
          }
          else
            entry_string_index[entry_index++] = ch;
        }

      return true;
    }

    M_SelectDone(ptr1);       // phares 4/17/98
    return true;
  }

  return false;
}

static dboolean M_LevelTableResponder(int ch, int action, event_t* ev)
{
  if (action == MENU_ENTER)
  {
    int skill;
    int map_index;
    map_stats_t *map;

    if (current_setup_menu == level_table_page[wad_stats_summary_page])
      return true;

    map_index = set_menu_itemon - 1;
    map = &wad_stats.maps[map_index];

    skill = in_game ? gameskill : startskill;

    G_DeferedInitNew(skill, map->episode, map->map);

    M_LeaveSetupMenu();
    M_ClearMenus();
    S_StartVoidSound(g_sfx_swtchx);

    return true;
  }

  return false;
}

static dboolean M_SetupCommonSelectResponder(int ch, int action, event_t* ev)
{
  // changing an entry
  if (setup_select)
  {
    setup_menu_t* ptr1 = current_setup_menu + set_menu_itemon;

    if (action == MENU_ESCAPE) // Exit key = no change
    {
      M_SelectDone(ptr1);                           // phares 4/17/98
      setup_gather = false;   // finished gathering keys, if any
      return true;
    }

    if (ptr1->m_flags & S_YESNO) // yes or no setting?
    {
      if (action == MENU_ENTER) {
        dsda_ToggleConfig(ptr1->config_id, true);
      }
      M_SelectDone(ptr1);                           // phares 4/17/98
      return true;
    }

    if (ptr1->m_flags & (S_NUM | S_CRITEM)) // number?
    {
      if (setup_gather) { // gathering keys for a value?
        /* killough 10/98: Allow negatives, and use a more
          * friendly input method (e.g. don't clear value early,
          * allow backspace, and return to original value if bad
          * value is entered).
          */
        if (action == MENU_ENTER) {
          if (gather_count) {     // Any input?
            int value;

            gather_buffer[gather_count] = 0;
            value = atoi(gather_buffer);  // Integer value

            dsda_UpdateIntConfig(ptr1->config_id, value, true);
          }
          M_SelectDone(ptr1);     // phares 4/17/98
          setup_gather = false; // finished gathering keys
          return true;
        }

        if (action == MENU_BACKSPACE && gather_count) {
          gather_count--;
          return true;
        }

        if (gather_count >= MAXGATHER)
          return true;

        if (!isdigit(ch) && ch != '-')
          return true; // ignore

        /* killough 10/98: character-based numerical input */
        gather_buffer[gather_count++] = ch;
      }
      return true;
    }

    if (ptr1->m_flags & S_CHOICE) // selection of choices?
    {
      if (action == MENU_LEFT) {
        if (ptr1->m_flags & S_STR)
        {
          int old_value, value;

          old_value = M_IndexInChoices(entry_string_index, ptr1->selectstrings);
          value = old_value - 1;
          if (value < 0)
            value = 0;
          if (old_value != value)
          {
            S_StartVoidSound(g_sfx_menu);
            strncpy(entry_string_index, ptr1->selectstrings[value], ENTRY_STRING_BFR_SIZE - 1);
          }
        }
        else
        {
          int value = choice_value;

          do {
            --value;
          } while (value > 0 && ptr1->selectstrings && ptr1->selectstrings[value][0] == '~');

          if (value >= 0 && choice_value != value) {
            S_StartVoidSound(g_sfx_menu);
            choice_value = value;
          }
        }
      }
      else if (action == MENU_RIGHT) {
        if (ptr1->m_flags & S_STR)
        {
          int old_value, value;

          old_value = M_IndexInChoices(entry_string_index, ptr1->selectstrings);
          value = old_value + 1;
          if (ptr1->selectstrings[value] == NULL)
            value = old_value;
          if (old_value != value)
          {
            S_StartVoidSound(g_sfx_menu);
            strncpy(entry_string_index, ptr1->selectstrings[value], ENTRY_STRING_BFR_SIZE - 1);
          }
        }
        else
        {
          int value = choice_value;

          do {
            ++value;
          } while (ptr1->selectstrings && ptr1->selectstrings[value] && ptr1->selectstrings[value][0] == '~');

          if (ptr1->selectstrings[value] && choice_value != value) {
            S_StartVoidSound(g_sfx_menu);
            choice_value = value;
          }
        }
      }
      else if (action == MENU_ENTER) {
        if (ptr1->m_flags & S_STR)
        {
          dsda_UpdateStringConfig(ptr1->config_id, entry_string_index, true);
        }
        else
        {
          dsda_UpdateIntConfig(ptr1->config_id, choice_value, true);
        }
        M_SelectDone(ptr1);                           // phares 4/17/98
      }
      return true;
    }
  }

  return false;
}

static dboolean M_SetupNavigationResponder(int ch, int action, event_t* ev)
{
  setup_menu_t* ptr1 = current_setup_menu + set_menu_itemon;
  setup_menu_t* ptr2 = NULL;

  if (action == MENU_DOWN)
  {
    if (ptr1->m_flags & S_NOSELECT)
      return true;

    ptr1->m_flags &= ~S_HILITE;     // phares 4/17/98
    do
      if (ptr1->m_flags & S_END)
      {
        set_menu_itemon = 0;
        ptr1 = current_setup_menu;
      }
      else
      {
        set_menu_itemon++;
        ptr1++;
      }
    while (ptr1->m_flags & S_SKIP);
    M_SelectDone(ptr1);         // phares 4/17/98
    return true;
  }

  if (action == MENU_UP)
  {
    if (ptr1->m_flags & S_NOSELECT)
      return true;

    ptr1->m_flags &= ~S_HILITE;     // phares 4/17/98
    do
    {
      if (set_menu_itemon == 0)
        do
          set_menu_itemon++;
        while(!((current_setup_menu + set_menu_itemon)->m_flags & S_END));
      set_menu_itemon--;
    }
    while((current_setup_menu + set_menu_itemon)->m_flags & S_SKIP);
    M_SelectDone(current_setup_menu + set_menu_itemon);         // phares 4/17/98
    return true;
  }

  if (action == MENU_CLEAR)
  {
    if (ptr1->m_flags & S_INPUT)
    {
      if (ptr1->m_flags & S_NOCLEAR)
      {
        S_StartVoidSound(g_sfx_oof);
      }
      else
      {
        dsda_InputReset(ptr1->input);
      }
    }

    return true;
  }

  if (action == MENU_ENTER)
  {
    int flags = ptr1->m_flags;

    // You've selected an item to change. Highlight it, post a new
    // message about what to do, and get ready to process the
    // change.
    //
    // killough 10/98: use friendlier char-based input buffer

    if (flags & (S_NUM | S_CRITEM))
    {
      setup_gather = true;
      gather_count = 0;
    }
    else if (flags & S_COLOR)
    {
      int color = dsda_PersistentIntConfig(ptr1->config_id);

      if (color < 0 || color > 255) // range check the value
        color = 0;        // 'no show' if invalid

      color_palette_x = color & 15;
      color_palette_y = color >> 4;
      colorbox_active = true;
    }
    else if (flags & S_STRING)
    {
      strncpy(entry_string_index, dsda_PersistentStringConfig(ptr1->config_id),
              ENTRY_STRING_BFR_SIZE - 1);

      entry_index = 0; // current cursor position in entry_string_index
    }
    else if (flags & S_CHOICE)
    {
      if (flags & S_STR)
      {
        strncpy(entry_string_index, dsda_PersistentStringConfig(ptr1->config_id),
                ENTRY_STRING_BFR_SIZE - 1);
      }
      else
      {
        choice_value = dsda_PersistentIntConfig(ptr1->config_id);
      }
    }

    ptr1->m_flags |= S_SELECT;
    setup_select = true;
    S_StartVoidSound(g_sfx_itemup);
    return true;
  }

  if ((action == MENU_ESCAPE) || (action == MENU_BACKSPACE))
  {
    M_LeaveSetupMenu();
    if (action == MENU_ESCAPE) // Clear all menus
      M_ClearMenus();
    else // MENU_BACKSPACE = return to Setup Menu
      if (currentMenu->prevMenu)
      {
        M_ChangeMenu(currentMenu->prevMenu, mnact_nochange);
        itemOn = currentMenu->lastOn;
        S_StartVoidSound(g_sfx_swtchn);
      }
    ptr1->m_flags &= ~(S_HILITE|S_SELECT);// phares 4/19/98
    S_StartVoidSound(g_sfx_swtchx);
    return true;
  }

  // Some setup screens may have multiple screens.
  // When there are multiple screens, m_prev and m_next items need to
  // be placed on the appropriate screen tables so the user can
  // move among the screens using the left and right arrow keys.
  // The m_var1 field contains a pointer to the appropriate screen
  // to move to.

  if (action == MENU_LEFT)
  {
    ptr2 = ptr1;
    do
    {
      ptr2++;
      if (ptr2->m_flags & S_PREV)
      {
        ptr1->m_flags &= ~S_HILITE;
        M_SetSetupMenuItemOn(set_menu_itemon);
        M_UpdateSetupMenu(ptr2->menu);
        S_StartVoidSound(g_sfx_menu);  // killough 10/98
        return true;
      }
    }
    while (!(ptr2->m_flags & S_END));
  }

  if (action == MENU_RIGHT)
  {
    ptr2 = ptr1;
    do
    {
      ptr2++;
      if (ptr2->m_flags & S_NEXT)
      {
        ptr1->m_flags &= ~S_HILITE;
        M_SetSetupMenuItemOn(set_menu_itemon);
        M_UpdateSetupMenu(ptr2->menu);
        S_StartVoidSound(g_sfx_menu);  // killough 10/98
        return true;
      }
    }
    while (!(ptr2->m_flags & S_END));
  }

  return false;
}

static dboolean M_SetupResponder(int ch, int action, event_t* ev)
{
  if (M_SetupCommonSelectResponder(ch, action, ev))
    return true;

  if (set_keybnd_active) // on a key binding setup screen
    if (M_KeyBndResponder(ch, action, ev))
      return true;

  if (set_weapon_active) // on the weapons setup screen
    if (M_WeaponResponder(ch, action, ev))
      return true;

  if (set_auto_active) // on the automap setup screen
    if (M_AutoResponder(ch, action, ev))
      return true;

  // killough 10/98: consolidate handling into one place:
  if (set_general_active || set_status_active)
    if (M_StringResponder(ch, action, ev))
      return true;

  if (level_table_active)
    if (M_LevelTableResponder(ch, action, ev))
      return true;

  // Not changing any items on the Setup screens. See if we're
  // navigating the Setup menus or selecting an item to change.
  if (M_SetupNavigationResponder(ch, action, ev))
    return true;

  return false;
}

dboolean M_Responder (event_t* ev) {
  int    ch, action;
  int    i;
  static int joywait   = 0;
  static int mousewait = 0;

  ch = MENU_NULL; // will be changed to a legit char if we're going to use it here
  action = MENU_NULL; // differentiate between action caused by a binding and input ch

  // Process joystick input

  if (ev->type == ev_joystick) {
    if (ev->data1.i && joywait < dsda_GetTick())
    {
      ch = 0; // meaningless, just to get you past the check for -1
      joywait = dsda_GetTick() + 5;
    }
  }
  else if (ev->type == ev_mouse) {
    if (ev->data1.i && mousewait < dsda_GetTick())
    {
      ch = 0; // meaningless, just to get you past the check for -1
      mousewait = dsda_GetTick() + 15;
    }
  }
  else if (ev->type == ev_keydown)
  {
    ch = ev->data1.i;
                                  // phares 4/11/98:
    if (ch == KEYD_RSHIFT)        // For string processing, need
      shiftdown = true;           // to know when shift key is up or
  }                               // down so you can get at the !,#,
  else if (ev->type == ev_keyup)  // etc. keys. Keydowns are allowed
  {                               // past this point, but keyups aren't
    if (ev->data1.i == KEYD_RSHIFT) // so we need to note the difference
      shiftdown = false;          // here using the 'shiftdown' dboolean.
  }

  if (dsda_InputActivated(dsda_input_menu_left))
  {
    action = MENU_LEFT;
  }
  else if (dsda_InputActivated(dsda_input_menu_right))
  {
    action = MENU_RIGHT;
  }
  else if (dsda_InputActivated(dsda_input_menu_up))
  {
    action = MENU_UP;
  }
  else if (dsda_InputActivated(dsda_input_menu_down))
  {
    action = MENU_DOWN;
  }
  else if (dsda_InputActivated(dsda_input_menu_backspace))
  {
    action = MENU_BACKSPACE;
  }
  else if (dsda_InputActivated(dsda_input_menu_enter))
  {
    action = MENU_ENTER;
  }
  else if (dsda_InputActivated(dsda_input_menu_escape))
  {
    action = MENU_ESCAPE;
  }
  else if (dsda_InputActivated(dsda_input_menu_clear))
  {
    action = MENU_CLEAR;
  }

  if (M_ConsoleOpen() && action != MENU_ESCAPE)
  {
    if (ev->type == ev_text) {
      dsda_UpdateConsoleText(ev->text);
      return true;
    }
    else if (action != MENU_NULL)
    {
      dsda_UpdateConsole(action);
      return true;
    }
    else if (ch != MENU_NULL)
      return true;
  }

  // Save Game string input

  if (saveStringEnter && (ch != MENU_NULL || action != MENU_NULL)) {
    if (action == MENU_BACKSPACE)                            // phares 3/7/98
    {
      if (saveCharIndex > 0)
      {
        if (!strcmp(savegamestrings[saveSlot], dsda_MapLumpName(gameepisode, gamemap)))
        {
          saveCharIndex = 0;
        }
        else
        {
          saveCharIndex--;
        }
        savegamestrings[saveSlot][saveCharIndex] = 0;
      }
    }
    else if (action == MENU_ESCAPE)                    // phares 3/7/98
    {
      saveStringEnter = 0;
      strcpy(&savegamestrings[saveSlot][0],saveOldString);
    }
    else if (action == MENU_ENTER)                     // phares 3/7/98
    {
      saveStringEnter = 0;
      if (savegamestrings[saveSlot][0])
        M_DoSave(saveSlot);
    }
    else if (ch > 0)
    {
      ch = toupper(ch);
      if (ch >= 32 && ch <= 127 &&
          saveCharIndex < SAVESTRINGSIZE-1 &&
          M_StringWidth(savegamestrings[saveSlot]) < (SAVESTRINGSIZE-2)*8)
      {
        savegamestrings[saveSlot][saveCharIndex++] = ch;
        savegamestrings[saveSlot][saveCharIndex] = 0;
      }
    }
    return true;
  }

  if (
    menuactive &&
    (currentMenu == &LoadDef || currentMenu == &SaveDef) &&
    !saveStringEnter
  )
  {
    int diff = 0;

    if (action == MENU_LEFT)
      diff = -1;
    else if (action == MENU_RIGHT)
      diff = 1;

    if (diff)
    {
      save_page += diff;
      if (save_page < 0)
        save_page = save_page_limit - 1;
      else if (save_page >= save_page_limit)
        save_page = 0;

      M_ReadSaveStrings();
    }
  }

  // Take care of any messages that need input

  if (messageToPrint && ch != MENU_NULL) {
    dboolean affirmative = false;

    if (messageNeedsInput == true)
    { // phares
      if (ch == 'y' || (!ch && action == MENU_ENTER))
        affirmative = true;
      else if (ch == ' ' || ch == KEYD_ESCAPE || ch == 'n' || (!ch && action == MENU_BACKSPACE))
        affirmative = false;
      else
        return false;
    }

    M_ChangeMenu(NULL, messageLastMenuActive);
    messageToPrint = 0;
    if (messageRoutine)
      messageRoutine(affirmative);

    M_ChangeMenu(NULL, mnact_inactive);
    S_StartVoidSound(g_sfx_swtchx);
    return true;
  }

  // killough 2/22/98: add support for screenshot key:
  if (dsda_InputActivated(dsda_input_screenshot))
  {
    G_ScreenShot ();
    // Don't eat the keypress in this case. See sf bug #1843280.
  }

  if (heretic && F_BlockingInput())
    return false;

  // If there is no active menu displayed...

  if (!menuactive) {                                           // phares
    if (ch == KEYD_F1)                                         //  V
    {
      M_StartControlPanel ();
      M_ChangeMenu(&HelpDef, mnact_nochange);

      itemOn = 0;
      S_StartVoidSound(g_sfx_swtchn);
      return true;
    }

    if (dsda_InputActivated(dsda_input_savegame))
    {
      M_StartControlPanel();
      S_StartVoidSound(g_sfx_swtchn);
      M_SaveGame(0);
      return true;
    }

    if (dsda_InputActivated(dsda_input_loadgame))
    {
      M_StartControlPanel();
      S_StartVoidSound(g_sfx_swtchn);
      M_LoadGame(0);
      return true;
    }

    if (dsda_InputActivated(dsda_input_soundvolume))
    {
      M_StartControlPanel ();
      M_ChangeMenu(&SoundDef, mnact_nochange);
      itemOn = sfx_vol;
      S_StartVoidSound(g_sfx_swtchn);
      return true;
    }

    if (dsda_InputActivated(dsda_input_quicksave))
    {
      S_StartVoidSound(g_sfx_swtchn);
      M_QuickSave();
      return true;
    }

    if (dsda_InputActivated(dsda_input_endgame))
    {
      S_StartVoidSound(g_sfx_swtchn);
      M_EndGame(0);
      return true;
    }

    if (dsda_InputActivated(dsda_input_quickload))
    {
      S_StartVoidSound(g_sfx_swtchn);
      M_QuickLoad();
      return true;
    }

    if (dsda_InputActivated(dsda_input_quit))
    {
      if (!dsda_SkipQuitPrompt())
        S_StartVoidSound(g_sfx_swtchn);
      M_QuitDOOM(0);
      return true;
    }

    if (dsda_InputActivated(dsda_input_console))
    {
      if (dsda_OpenConsole())
        S_StartVoidSound(g_sfx_swtchn);
      return true;
    }

    {
      int i;

      for (i = 0; i < CONSOLE_SCRIPT_COUNT; ++i)
        if (dsda_InputActivated(dsda_input_script_0 + i)) {
          dsda_ExecuteConsoleScript(i);

          return true;
        }
    }

    // Toggle gamma
    if (dsda_InputActivated(dsda_input_gamma))
    {
//e6y
      dsda_CycleConfig(dsda_config_usegamma, true);
      dsda_AddMessage(usegamma == 0 ? s_GAMMALVL0 :
                      usegamma == 1 ? s_GAMMALVL1 :
                      usegamma == 2 ? s_GAMMALVL2 :
                      usegamma == 3 ? s_GAMMALVL3 :
                      s_GAMMALVL4);
      return true;
    }

    if (dsda_InputActivated(dsda_input_zoomout))
    {
      if (automap_active)
        return false;
      M_SizeDisplay(0);
      S_StartVoidSound(g_sfx_stnmov);
      return true;
    }

    if (dsda_InputActivated(dsda_input_zoomin))
    {                                   // jff 2/23/98
      if (automap_active)               // allow
        return false;                   // key_hud==key_zoomin
      M_SizeDisplay(1);                                             //  ^
      S_StartVoidSound(g_sfx_stnmov);                              //  |
      return true;                                                  // phares
    }

    //e6y
    if (dsda_InputActivated(dsda_input_speed_default) && !dsda_StrictMode())
    {
      int value = StepwiseSum(dsda_GameSpeed(), 0, 3, 10000, 100);
      dsda_UpdateGameSpeed(value);
      doom_printf("Game Speed %d", value);
      // Don't eat the keypress in this case.
      // return true;
    }
    if (dsda_InputActivated(dsda_input_speed_up) && !dsda_StrictMode())
    {
      int value = StepwiseSum(dsda_GameSpeed(), 1, 3, 10000, 100);
      dsda_UpdateGameSpeed(value);
      doom_printf("Game Speed %d", value);
      // Don't eat the keypress in this case.
      // return true;
    }
    if (dsda_InputActivated(dsda_input_speed_down) && !dsda_StrictMode())
    {
      int value = StepwiseSum(dsda_GameSpeed(), -1, 3, 10000, 100);
      dsda_UpdateGameSpeed(value);
      doom_printf("Game Speed %d", value);
      // Don't eat the keypress in this case.
      // return true;
    }
    if (dsda_InputActivated(dsda_input_nextlevel))
    {
      if (userplayback && !dsda_SkipMode())
      {
        dsda_SkipToNextMap();
        return true;
      }
      else
      {
        if (G_GotoNextLevel())
          return true;
      }
    }

    if (dsda_InputActivated(dsda_input_restart))
    {
      if (G_ReloadLevel())
        return true;
    }

    if (dsda_InputActivated(dsda_input_demo_endlevel))
    {
      if (userplayback && !dsda_SkipMode())
      {
        dsda_SkipToEndOfMap();
        return true;
      }
    }

    if (dsda_InputActivated(dsda_input_demo_skip))
    {
      if (userplayback)
      {
        dsda_ToggleSkipMode();
        return true;
      }
    }

    if (dsda_InputActivated(dsda_input_store_quick_key_frame))
    {
      if (
        gamestate == GS_LEVEL &&
        gameaction == ga_nothing &&
        !dsda_StrictMode()
      ) dsda_StoreQuickKeyFrame();
      return true;
    }

    if (dsda_InputActivated(dsda_input_restore_quick_key_frame))
    {
      if (!dsda_StrictMode()) dsda_RestoreQuickKeyFrame();
      return true;
    }

    if (dsda_InputActivated(dsda_input_rewind))
    {
      if (!dsda_StrictMode()) dsda_RewindAutoKeyFrame();
      return true;
    }

    if (dsda_InputActivated(dsda_input_cycle_profile))
    {
      int value = dsda_CycleConfig(dsda_config_input_profile, true);
      doom_printf("Input Profile %d", value);
      S_StartVoidSound(g_sfx_swtchn);
      return true;
    }

    if (dsda_InputActivated(dsda_input_cycle_palette))
    {
      dsda_CyclePlayPal();
      doom_printf("Palette %s", dsda_PlayPalData()->lump_name);
      S_StartVoidSound(g_sfx_swtchn);
      return true;
    }

    if (dsda_InputActivated(dsda_input_walkcamera))
    {
      if (demoplayback && gamestate == GS_LEVEL)
      {
        walkcamera.type = (walkcamera.type+1)%3;
        P_SyncWalkcam (true, (walkcamera.type!=2));
        R_ResetViewInterpolation ();
        if (walkcamera.type==0)
          R_SmoothPlaying_Reset(NULL);
        // Don't eat the keypress in this case.
        // return true;
      }
    }

#if 0
    if (V_IsOpenGLMode())
    {
      if (dsda_InputActivated(dsda_input_showalive) && !dsda_StrictMode())
      {
        const char* const show_alive_message[3] = { "off", "(mode 1) on", "(mode 2) on" };
        int show_alive = dsda_CycleConfig(dsda_config_show_alive_monsters, false);

        if (show_alive >= 0 && show_alive < 3)
          doom_printf("Show Alive Monsters %s", show_alive_message[show_alive]);
      }
    }
#endif

    M_HandleToggles();

    if (dsda_InputActivated(dsda_input_hud))   // heads-up mode
    {
      if (automap_active)              // jff 2/22/98
        return false;                  // HUD mode control
      M_SizeDisplay(2);
      return true;
    }
  }
  // Pop-up Main menu?

  if (!menuactive)
  {
    if (ch == KEYD_ESCAPE || action == MENU_ESCAPE) // phares
    {
      M_StartControlPanel ();
      S_StartVoidSound(g_sfx_swtchn);
      return true;
    }
    return false;
  }

  if (ch == MENU_NULL)
    return false; // we can't use the event here

  // [FG] delete a savegame

  if (currentMenu == &LoadDef || currentMenu == &SaveDef)
  {
    if (delete_verify)
    {
      if (toupper(ch) == 'Y')
      {
        M_DeleteGame(itemOn);
        S_StartVoidSound(g_sfx_itemup);
        delete_verify = false;
      }
      else if (toupper(ch) == 'N')
      {
        S_StartVoidSound(g_sfx_itemup);
        delete_verify = false;
      }
      return true;
    }
  }

  if (setup_active)
    if (M_SetupResponder(ch, action, ev))
      return true;

  // From here on, these navigation keys are used on the BIG FONT menus
  // like the Main Menu.

  if (action == MENU_DOWN)                             // phares 3/7/98
  {
    do
    {
      if (itemOn + 1 > currentMenu->numitems - 1)
        itemOn = 0;
      else
        itemOn++;
      S_StartVoidSound(g_sfx_menu);
    }
    while(currentMenu->menuitems[itemOn].status == -1);
    return true;
  }

  if (action == MENU_UP)                               // phares 3/7/98
  {
    do
    {
      if (!itemOn)
        itemOn = currentMenu->numitems - 1;
      else
        itemOn--;
      S_StartVoidSound(g_sfx_menu);
    }
    while(currentMenu->menuitems[itemOn].status == -1);
    return true;
  }

  if (action == MENU_LEFT)                             // phares 3/7/98
  {
    if (currentMenu->menuitems[itemOn].routine &&
        currentMenu->menuitems[itemOn].status == 2)
    {
      S_StartVoidSound(g_sfx_stnmov);
      currentMenu->menuitems[itemOn].routine(0);
    }
    return true;
  }

  if (action == MENU_RIGHT)                            // phares 3/7/98
  {
    if (currentMenu->menuitems[itemOn].routine &&
        currentMenu->menuitems[itemOn].status == 2)
    {
      S_StartVoidSound(g_sfx_stnmov);
      currentMenu->menuitems[itemOn].routine(1);
    }
    return true;
  }

  if (action == MENU_ENTER)                            // phares 3/7/98
  {
    if (currentMenu->menuitems[itemOn].routine &&
        currentMenu->menuitems[itemOn].status)
    {
      currentMenu->lastOn = itemOn;
      if (currentMenu->menuitems[itemOn].status == 2)
      {
        currentMenu->menuitems[itemOn].routine(1);   // right arrow
        S_StartVoidSound(g_sfx_stnmov);
      }
      else
      {
        currentMenu->menuitems[itemOn].routine(itemOn);
        S_StartVoidSound(g_sfx_pistol);
      }
    }
    //jff 3/24/98 remember last skill selected
    // killough 10/98 moved to skill-specific functions
    return true;
  }

  if (action == MENU_ESCAPE)                           // phares 3/7/98
  {
    currentMenu->lastOn = itemOn;
    M_ClearMenus ();
    S_StartVoidSound(g_sfx_swtchx);
    return true;
  }

  if (action == MENU_BACKSPACE)                        // phares 3/7/98
  {
    currentMenu->lastOn = itemOn;

    // phares 3/30/98:
    // add checks to see if you're in the extended help screens
    // if so, stay with the same menu definition, but bump the
    // index back one. if the index bumps back far enough ( == 0)
    // then you can return to the Read_Thisn menu definitions

    if (currentMenu->prevMenu)
    {
      if (currentMenu == &ExtHelpDef)
      {
        if (--extended_help_index == 0)
        {
          M_ChangeMenu(currentMenu->prevMenu, mnact_nochange);
          extended_help_index = 1; // reset
        }
      }
      else
        M_ChangeMenu(currentMenu->prevMenu, mnact_nochange);
      itemOn = currentMenu->lastOn;
      S_StartVoidSound(g_sfx_swtchn);
    }
    else
    {
      M_ClearMenus();
      S_StartVoidSound(g_sfx_swtchx);
    }
    return true;
  }
  else if (action == MENU_CLEAR) // [FG] delete a savegame
  {
    if (currentMenu == &LoadDef || currentMenu == &SaveDef)
    {
      if (LoadMenue[itemOn].status)
      {
        S_StartVoidSound(g_sfx_itemup);
        currentMenu->lastOn = itemOn;
        delete_verify = true;
        return true;
      }
      else
      {
        S_StartVoidSound(g_sfx_oof);
      }
    }
  }
  else
  {
    for (i = itemOn + 1; i < currentMenu->numitems; i++)
      if (currentMenu->menuitems[i].alphaKey == ch)
      {
        itemOn = i;
        S_StartVoidSound(g_sfx_menu);
        return true;
      }
    for (i = 0; i <= itemOn; i++)
      if (currentMenu->menuitems[i].alphaKey == ch)
      {
        itemOn = i;
        S_StartVoidSound(g_sfx_menu);
        return true;
      }
  }
  return false;
}

//
// End of M_Responder
//
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//
// General Routines
//
// This displays the Main menu and gets the menu screens rolling.
// Plus a variety of routines that control the Big Font menu display.
// Plus some initialization for game-dependant situations.

static void M_InitializeSkillMenu(void)
{
  extern skill_info_t *skill_infos;
  int i;

  SkillDef.lastOn = dsda_IntConfig(dsda_config_default_skill) - 1;

  SkillDef.numitems = num_skills;
  SkillDef.menuitems = Z_Calloc(num_skills, sizeof(*SkillDef.menuitems));

  for (i = 0; i < num_skills; ++i)
  {
    SkillDef.menuitems[i].status = 1;

    if (skill_infos[i].pic_name)
      strncpy(SkillDef.menuitems[i].name, skill_infos[i].pic_name, 8);

    SkillDef.menuitems[i].alttext = skill_infos[i].name;
    SkillDef.menuitems[i].color = skill_infos[i].text_color;

    SkillDef.menuitems[i].routine = M_ChooseSkill;
    SkillDef.menuitems[i].alphaKey = skill_infos[i].key;

    if (skill_infos[i].flags & SI_DEFAULT_SKILL)
      SkillDef.lastOn = i;
  }

  if (SkillDef.lastOn >= num_skills)
    SkillDef.lastOn = num_skills - 1;
}

static void M_InitializeEpisodeMenu(void)
{
  int i;

  EpiDef.numitems = num_episodes;
  EpiDef.menuitems = Z_Calloc(num_episodes, sizeof(*EpiDef.menuitems));

  for (i = 0; i < num_episodes; ++i)
  {
    EpiDef.menuitems[i].status = 1;

    if (episodes[i].pic_name)
      strncpy(EpiDef.menuitems[i].name, episodes[i].pic_name, 8);

    EpiDef.menuitems[i].alttext = episodes[i].name;

    EpiDef.menuitems[i].routine = M_Episode;
    EpiDef.menuitems[i].alphaKey = episodes[i].key;
  }

  if (!raven)
  {
    if (EpiDef.numitems <= 4)
    {
      EpiDef.y = 63;
    }
    else
    {
      EpiDef.y = 63 - (EpiDef.numitems - 4) * (LINEHEIGHT / 2);
    }
  }

  if (num_episodes > 1)
    SkillDef.prevMenu = &EpiDef;
  else
    SkillDef.prevMenu = &MainDef;
}

void M_StartControlPanel (void)
{
  // intro might call this repeatedly

  if (menuactive)
    return;

  DO_ONCE
    M_InitializeSkillMenu();
    M_InitializeEpisodeMenu();
  END_ONCE

  M_ChangeMenu(&MainDef, mnact_float);
  itemOn = currentMenu->lastOn;   // JDC
}

//
// M_Drawer
// Called after the view has been rendered,
// but before it has been blitted.
//
// killough 9/29/98: Significantly reformatted source
//

void M_Drawer (void)
{
  V_BeginUIDraw();

  inhelpscreens = false;

  // Horiz. & Vertically center string and print it.
  // killough 9/29/98: simplified code, removed 40-character width limit
  if (messageToPrint)
  {
    char* ms;
    char* p;
    int y;

    if (raven)
    {
      MN_DrawMessage(messageString);
      V_EndUIDraw();
      return;
    }

    /* cph - Z_Strdup string to writable memory */
    ms = Z_Strdup(messageString);
    p = ms;

    y = 100 - M_StringHeight(messageString)/2;
    while (*p)
    {
      char *string = p, c;
      while ((c = *p) && *p != '\n')
        p++;
      *p = 0;
      M_WriteText(160 - M_StringWidth(string)/2, y, string, CR_DEFAULT);
      y += menu_font->line_height;
      if ((*p = c))
        p++;
    }
    Z_Free(ms);
  }
  else if (menuactive)
  {
    int x, y, max, i;
    int lumps_missing;

    M_ChangeMenu(NULL, mnact_float);

    if (currentMenu->routine)
      currentMenu->routine();     // call Draw routine

    if (raven)
    {
      MN_Drawer();
      V_EndUIDraw();
      return;
    }

    // DRAW MENU

    x = currentMenu->x;
    y = currentMenu->y;
    max = currentMenu->numitems;
    lumps_missing = 0;

    for (i = 0; i < max; i++)
      if (
        currentMenu->menuitems[i].status != -1 && (
          !currentMenu->menuitems[i].name[0] || !W_LumpNameExists(currentMenu->menuitems[i].name)
        )
      )
        ++lumps_missing;

    if (!lumps_missing)
      for (i = 0; i < max; i++)
      {
        if (currentMenu->menuitems[i].name[0])
          V_DrawNamePatch(x, y, 0, currentMenu->menuitems[i].name,
                          currentMenu->menuitems[i].color, VPT_STRETCH);

        y += LINEHEIGHT;
      }
    else
      for (i = 0; i < max; i++)
      {
        const char *alttext = currentMenu->menuitems[i].alttext;

        if (alttext)
          M_WriteText(x, y + 8 - (M_StringHeight(alttext) / 2),
                      alttext, currentMenu->menuitems[i].color);

        y += LINEHEIGHT;
      }

    // DRAW SKULL
    if (max > 0)
      // CPhipps - patch drawing updated
      V_DrawNamePatch(x + SKULLXOFF, currentMenu->y - 5 + itemOn*LINEHEIGHT,0,
          skullName[whichSkull], CR_DEFAULT, VPT_STRETCH);
  }

  V_EndUIDraw();
}

void M_ChangeMenu(menu_t *menudef, menuactive_t mnact)
{
  if (menudef)
    currentMenu = menudef;

  if (mnact != mnact_nochange)
    menuactive = mnact;

  if (mnact > mnact_inactive && gamestate == GS_LEVEL)
    dsda_TrackFeature(uf_menu);

#if 0
  if (SDL_IsTextInputActive()) {
    if (!(currentMenu && currentMenu->flags & MENUF_TEXTINPUT))
      SDL_StopTextInput();
  }
  else if (currentMenu && currentMenu->flags & MENUF_TEXTINPUT)
    SDL_StartTextInput();
#endif
}

//
// M_ClearMenus
//
// Called when leaving the menu screens for the real world

void M_ClearMenus (void)
{
  M_ChangeMenu(&MainDef, mnact_inactive);

  BorderNeedRefresh = true;
}

//
// M_SetupNextMenu
//
void M_SetupNextMenu(menu_t *menudef)
{
  M_ChangeMenu(menudef, mnact_nochange);
  itemOn = currentMenu->lastOn;

  BorderNeedRefresh = true;
}

/////////////////////////////
//
// M_Ticker
//
void M_Ticker (void)
{
  // The skull counter is also used for non-skull pointers
  if (--skullAnimCounter <= 0)
  {
    whichSkull ^= 1;
    skullAnimCounter = 8;
  }

  if (raven) return MN_Ticker();
}

/////////////////////////////
//
// Message Routines
//

void M_StartMessage (const char* string,void* routine,dboolean input)
{
  messageLastMenuActive = menuactive;
  messageToPrint = 1;
  messageString = string;
  messageRoutine = routine;
  messageNeedsInput = input;
  M_ChangeMenu(NULL, mnact_float);
  return;
}

void M_StopMessage(void)
{
  M_ChangeMenu(NULL, messageLastMenuActive);
  messageToPrint = 0;
}

/////////////////////////////
//
// Thermometer Routines
//

//
// M_DrawThermo draws the thermometer graphic for Mouse Sensitivity,
// Sound Volume, etc.
//
// proff/nicolas 09/20/98 -- changed for hi-res
// CPhipps - patch drawing updated
//
void M_DrawThermo(int x,int y,int thermWidth,int thermDot )
{
  int          xx;
  int           i;
  char num[4];
  int horizScaler; //Used to allow more thermo range for mouse sensitivity.

  if (raven) return MN_DrawSlider(x, y, thermWidth, thermDot);

  /*
   * Modification By Barry Mead to allow the Thermometer to have vastly
   * larger ranges. (the thermWidth parameter can now have a value as
   * large as 200.      Modified 1-9-2000  Originally I used it to make
   * the sensitivity range for the mouse better. It could however also
   * be used to improve the dynamic range of music and sound affect
   * volume controls for example.
   */
  thermWidth = (thermWidth > 200) ? 200 : thermWidth; //Clamp to 200 max
  horizScaler = (thermWidth > 23) ? (200 / thermWidth) : 8; //Dynamic range
  xx = x;
  V_DrawNamePatch(xx, y, 0, "M_THERML", CR_DEFAULT, VPT_STRETCH);
  xx += 8;
  for (i=0;i<thermWidth;i++)
    {
    V_DrawNamePatch(xx, y, 0, "M_THERMM", CR_DEFAULT, VPT_STRETCH);
    xx += horizScaler;
    }

  xx += (8 - horizScaler);  /* make the right end look even */

  V_DrawNamePatch(xx, y, 0, "M_THERMR", CR_DEFAULT, VPT_STRETCH);

  // [crispy] print the value
  snprintf(num, sizeof(num), "%3d", thermDot);
  strcpy(menu_buffer, num);
  M_DrawMenuString(xx + 12, y + 3, cr_value_edit);

  // [crispy]
  if (thermDot >= thermWidth)
  {
    thermDot = thermWidth - 1;
  }

  V_DrawNamePatch((x+8)+thermDot*horizScaler,y,0,"M_THERMO",CR_DEFAULT,VPT_STRETCH);
}

//
// Draw an empty cell in the thermometer
//

void M_DrawEmptyCell (menu_t* menu,int item)
{
  // CPhipps - patch drawing updated
  V_DrawNamePatch(menu->x - 10, menu->y+item*LINEHEIGHT - 1, 0,
      "M_CELL1", CR_DEFAULT, VPT_STRETCH);
}

//
// Draw a full cell in the thermometer
//

void M_DrawSelCell (menu_t* menu,int item)
{
  // CPhipps - patch drawing updated
  V_DrawNamePatch(menu->x - 10, menu->y+item*LINEHEIGHT - 1, 0,
      "M_CELL2", CR_DEFAULT, VPT_STRETCH);
}

/////////////////////////////
//
// String-drawing Routines
//

//
// Find string width from menu_font chars
//

int M_StringWidth(const char* string)
{
  int i, c, w = 0;
  for (i = 0;(size_t)i < strlen(string);i++)
    w += (c = toupper(string[i]) - HU_FONTSTART) < 0 || c >= HU_FONTSIZE ?
      menu_font->space_width : menu_font->font[c].width;
  return w;
}

//
//    Find string height from menu_font chars
//

int M_StringHeight(const char* string)
{
  int i, h = menu_font->height;
  for (i = 0;string[i];i++)            // killough 1/31/98
    if (string[i] == '\n')
      h += menu_font->line_height;
  return h;
}

//
//    Write a string using the menu_font
//
void M_WriteText (int x,int y, const char* string, int cm)
{
  int   w;
  const char* ch;
  int   c;
  int   cx;
  int   cy;
  int   flags;

  ch = string;
  cx = x;
  cy = y;

  flags = VPT_STRETCH;
  if (cm != CR_DEFAULT)
    flags |= VPT_TRANS;

  while(1) {
    c = *ch++;
    if (!c)
      break;
    if (c == '\n') {
      cx = x;
      cy += 12;
      continue;
    }

    c = toupper(c) - HU_FONTSTART;
    if (c < 0 || c>= HU_FONTSIZE) {
      cx += 4;
      continue;
    }

    w = menu_font->font[c].width;
    if (cx+w > BASE_WIDTH)
      break;
    // proff/nicolas 09/20/98 -- changed for hi-res
    // CPhipps - patch drawing updated
    V_DrawNumPatch(cx, cy, 0, menu_font->font[c].lumpnum, cm, flags);
    cx+=w;
  }
}

void M_DrawTitle(int x, int y, const char *patch, int cm,
                 const char *alttext, int altcm)
{
  int lumpnum = W_CheckNumForName(patch);

  if (lumpnum != LUMP_NOT_FOUND)
  {
    int flags = VPT_STRETCH;
    if (cm != CR_DEFAULT)
      flags |= VPT_TRANS;
    V_DrawNumPatch(x, y, 0, lumpnum, cm, flags);
  }
  else
  {
    // patch doesn't exist, draw some text in place of it
    M_WriteText(160-(M_StringWidth(alttext)/2),
                y+8-(M_StringHeight(alttext)/2), // assumes patch height 16
                alttext, altcm);
  }
}

/////////////////////////////
//
// Initialization Routines to take care of one-time setup
//

// phares 4/08/98:
// M_InitHelpScreen() clears the weapons from the HELP
// screen that don't exist in this version of the game.

void M_InitHelpScreen(void)
{
  setup_menu_t* src;

  for (src = helpstrings; !(src->m_flags & S_END); src++) {
    if (!src->m_text)
      continue;

    if ((strncmp(src->m_text,"PLASMA",6) == 0) && (gamemode == shareware))
      src->m_flags = S_SKIP; // Don't show setting or item
    if ((strncmp(src->m_text,"BFG",3) == 0) && (gamemode == shareware))
      src->m_flags = S_SKIP; // Don't show setting or item
    if ((strncmp(src->m_text,"SSG",3) == 0) && (gamemode != commercial))
      src->m_flags = S_SKIP; // Don't show setting or item
  }
}

//
// M_Init
//
void M_Init(void)
{
  if (raven) MN_Init();

  M_LoadTextColors();
  M_LoadMenuFont();

  M_ChangeMenu(&MainDef, mnact_inactive);
  itemOn = currentMenu->lastOn;
  whichSkull = 0;
  skullAnimCounter = 10;
  messageToPrint = 0;
  messageString = NULL;
  messageLastMenuActive = menuactive;

  // Here we could catch other version dependencies,
  //  like HELP1/2, and four episodes.

  switch(gamemode)
  {
    case commercial:
      // This is used because DOOM 2 had only one HELP
      //  page. I use CREDIT as second page now, but
      //  kept this hack for educational purposes.
      MainMenu[readthis] = MainMenu[quitdoom];
      MainDef.numitems--;
      MainDef.y += 8;
      ReadDef1.routine = M_DrawReadThis1;
      ReadDef1.x = 330;
      ReadDef1.y = 165;
      ReadMenu1[0].routine = M_FinishReadThis;
      break;
    case registered:
      // Episode 2 and 3 are handled,
      //  branching to an ad screen.

      // killough 2/21/98: Fix registered Doom help screen
      // killough 10/98: moved to second screen, moved up to the top
      ReadDef2.y = 15;
      break;
    case shareware:
    case retail:
    default:
      break;
  }

  M_InitHelpScreen();   // init the help screen       // phares 4/08/98
  M_InitExtendedHelp(); // init extended help screens // phares 3/30/98

  //e6y
  M_ChangeSpeed();
  M_ChangeMaxViewPitch();
  M_ChangeSkyMode();
  M_ChangeFOV();

  M_ChangeDemoSmoothTurns();

  M_ChangeMapTextured();
  M_ChangeMapMultisamling();

  M_ChangeStretch();

  //M_ChangeMIDIPlayer();
}

//
// End of General Routines
//
/////////////////////////////////////////////////////////////////////////////
