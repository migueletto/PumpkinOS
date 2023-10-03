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

// MN_menu.c

#include "doomstat.h"
#include "w_wad.h"
#include "v_video.h"
#include "m_menu.h"
#include "dsda/settings.h"
#include "heretic/dstrings.h"
#include "heretic/mn_menu.h"

#define ITEM_HEIGHT 20
#define SELECTOR_XOFFSET (-28)
#define SELECTOR_YOFFSET (-1)
#define SFX_VOL_INDEX 1
#define MUS_VOL_INDEX 3

extern int g_menu_save_page_size;

static int FontABaseLump;
static int FontAYellowBaseLump;
static int FontBBaseLump;
static int SkullBaseLump;
static int MenuTime;

dboolean mn_SuicideConsole;

static void MN_InitFonts(void)
{
  FontABaseLump = W_GetNumForName("FONTA_S") + 1;
  if (hexen) FontAYellowBaseLump = W_GetNumForName("FONTAY_S") + 1;
  FontBBaseLump = W_GetNumForName("FONTB_S") + 1;
}

extern menu_t MainDef;
extern menu_t EpiDef;
extern menu_t SkillDef;
extern menu_t OptionsDef;
extern menu_t SoundDef;
extern menu_t LoadDef;
extern menu_t SaveDef;
extern menuitem_t SoundMenu[];

void M_DrawThermo(int x, int y, int thermWidth, int thermDot);

void MN_Init(void)
{
  MN_InitFonts();

  if (heretic)
  {
    SkullBaseLump = W_GetNumForName("M_SKL00");
  }
  else
  {
    SkullBaseLump = W_CheckNumForName2("FBULA0", ns_sprites);
  }

  // override doom menu parameters

  MainDef.x = 110;
  MainDef.y = 56;

  OptionsDef.x = 88;
  OptionsDef.y = 16;

  SoundDef.x = OptionsDef.x;
  SoundDef.y = OptionsDef.y;

  LoadDef.x = 70;
  LoadDef.y = 30;
  LoadDef.numitems = g_menu_save_page_size;

  SaveDef.x = 70;
  SaveDef.y = 30;
  SaveDef.numitems = g_menu_save_page_size;

  if (heretic)
  {
    EpiDef.x = 80;
    EpiDef.y = 50;

    SkillDef.x = 38;
    SkillDef.y = 30;

    if (gamemode == retail)
    {
      EpiDef.y -= ITEM_HEIGHT;
    }
  }
  else
  {
    EpiDef.x = 66;
    EpiDef.y = 66;

    SkillDef.x = 120;
    SkillDef.y = 44;
  }

  SoundMenu[0].alttext = "SFX VOLUME";
  SoundMenu[2].alttext = "MUSIC VOLUME";
}

void MN_UpdateClass(int choice)
{
  PlayerClass[consoleplayer] = choice + 1;

  switch (PlayerClass[consoleplayer])
  {
    case PCLASS_FIGHTER:
      SkillDef.x = 120;
      SkillDef.menuitems[0].alttext = "SQUIRE";
      SkillDef.menuitems[1].alttext = "KNIGHT";
      SkillDef.menuitems[2].alttext = "WARRIOR";
      SkillDef.menuitems[3].alttext = "BERSERKER";
      SkillDef.menuitems[4].alttext = "TITAN";
      break;
    case PCLASS_CLERIC:
      SkillDef.x = 116;
      SkillDef.menuitems[0].alttext = "ALTAR BOY";
      SkillDef.menuitems[1].alttext = "ACOLYTE";
      SkillDef.menuitems[2].alttext = "PRIEST";
      SkillDef.menuitems[3].alttext = "CARDINAL";
      SkillDef.menuitems[4].alttext = "POPE";
      break;
    case PCLASS_MAGE:
      SkillDef.x = 112;
      SkillDef.menuitems[0].alttext = "APPRENTICE";
      SkillDef.menuitems[1].alttext = "ENCHANTER";
      SkillDef.menuitems[2].alttext = "SORCERER";
      SkillDef.menuitems[3].alttext = "WARLOCK";
      SkillDef.menuitems[4].alttext = "ARCHIMAGE";
      break;
    default:
      break;
  }
}

void MN_Ticker(void)
{
    if (!menuactive)
    {
        return;
    }
    MenuTime++;
}

extern menu_t* currentMenu;
extern short itemOn;

void MN_DrawMessage(const char* messageString)
{
  char *msg;
  char *p;
  int y;

  msg = Z_Strdup(messageString);
  p = msg;
  y = 100 - MN_TextAHeight(msg) / 2;

  while (*p)
  {
    char *text;
    char c;

    text = p;
    while ((c = *p) && *p != '\n')
      p++;
    *p = 0;

    MN_DrTextA(text, 160 - MN_TextAWidth(text) / 2, y);
    y += R_NumPatchHeight(FontABaseLump);

    if ((*p = c))
      p++;
  }
  Z_Free(msg);
}

void MN_Drawer(void)
{
  int i;
  int x;
  int y;
  int max;
  const char* selName;

  x = currentMenu->x;
  y = currentMenu->y;
  max = currentMenu->numitems;

  for (i = 0; i < max; i++)
  {
    const char *text = currentMenu->menuitems[i].alttext;
    if (text)
      MN_DrTextB(text, x, y);
    y += ITEM_HEIGHT;
  }

  if (max)
  {
    y = currentMenu->y + (itemOn * ITEM_HEIGHT) + SELECTOR_YOFFSET;
    selName = (MenuTime & 16 ? "M_SLCTR1" : "M_SLCTR2");
    V_DrawNamePatch(x + SELECTOR_XOFFSET, y, 0, selName, CR_DEFAULT, VPT_STRETCH);
  }
  // MenuItem_t *item;
  // const char *message;
  // const char *selName;
  //
  // if (MenuActive == false)
  // {
  //   if (askforquit)
  //   {
  //     message = QuitEndMsg[typeofask - 1];
  //
  //     MN_DrTextA(message, 160 - MN_TextAWidth(message) / 2, 80);
  //     if (typeofask == 3)
  //     {
  //       MN_DrTextA(SlotText[quicksave - 1], 160 -
  //                  MN_TextAWidth(SlotText[quicksave - 1]) / 2, 90);
  //       MN_DrTextA("?", 160 +
  //                  MN_TextAWidth(SlotText[quicksave - 1]) / 2, 90);
  //     }
  //     if (typeofask == 4)
  //     {
  //       MN_DrTextA(SlotText[quickload - 1], 160 -
  //                  MN_TextAWidth(SlotText[quickload - 1]) / 2, 90);
  //       MN_DrTextA("?", 160 +
  //                  MN_TextAWidth(SlotText[quickload - 1]) / 2, 90);
  //     }
  //   }
  //   return;
  // }
  // else
  // {
  //   if (InfoType)
  //   {
  //     MN_DrawInfo();
  //     return;
  //   }
  //   if (screenblocks < 10)
  //   {
  //     BorderNeedRefresh = true;
  //   }
  //   if (CurrentMenu->drawFunc != NULL)
  //   {
  //     CurrentMenu->drawFunc();
  //   }
  //   x = CurrentMenu->x;
  //   y = CurrentMenu->y;
  //   item = CurrentMenu->items;
  //   for (i = 0; i < CurrentMenu->itemCount; i++)
  //   {
  //     if (item->type != ITT_EMPTY && item->text)
  //     {
  //       MN_DrTextB(item->text, x, y);
  //     }
  //
  //     y += ITEM_HEIGHT;
  //     item++;
  //   }
  //
  //   y = CurrentMenu->y + (CurrentItPos * ITEM_HEIGHT) + SELECTOR_YOFFSET;
  //   selName = (MenuTime & 16 ? "M_SLCTR1" : "M_SLCTR2");
  //   V_DrawPatch(x + SELECTOR_XOFFSET, y,
  //               W_LumpByName(selName));
  // }
}

static void Hexen_MN_DrawMainMenu(void);

void MN_DrawMainMenu(void)
{
  int frame;

  if (hexen) return Hexen_MN_DrawMainMenu();

  frame = (MenuTime / 3) % 18;
  V_DrawNamePatch(88, 0, 0, "M_HTIC", CR_DEFAULT, VPT_STRETCH);
  V_DrawNumPatch(40, 10, 0, SkullBaseLump + (17 - frame), CR_DEFAULT, VPT_STRETCH);
  V_DrawNumPatch(232, 10, 0, SkullBaseLump + frame, CR_DEFAULT, VPT_STRETCH);
}

static void Hexen_MN_DrawMainMenu(void)
{
  int frame;

  frame = (MenuTime / 5) % 7;
  V_DrawNamePatch(88, 0, 0, "M_HTIC", CR_DEFAULT, VPT_STRETCH);
  V_DrawNumPatch(37, 80, 0, SkullBaseLump + (frame + 2) % 7, CR_DEFAULT, VPT_STRETCH);
  V_DrawNumPatch(278, 80, 0, SkullBaseLump + frame, CR_DEFAULT, VPT_STRETCH);
}

// Class menu is in the episode slot for hexen
void MN_DrawEpisode(void)
{
  pclass_t class;
  static const char *boxLumpName[3] = {
    "m_fbox",
    "m_cbox",
    "m_mbox"
  };
  static const char *walkLumpName[3] = {
    "m_fwalk1",
    "m_cwalk1",
    "m_mwalk1"
  };

  if (heretic) return;

  MN_DrTextB("CHOOSE CLASS:", 34, 24);
  class = (pclass_t) itemOn;
  V_DrawNamePatch(174, 8, 0, boxLumpName[class], CR_DEFAULT, VPT_STRETCH);
  V_DrawNumPatch(174 + 24, 8 + 12, 0, W_GetNumForName(walkLumpName[class]) + ((MenuTime >> 3) & 3),
                 CR_DEFAULT, VPT_STRETCH);
}

void MN_DrawSkillMenu(void)
{
    if (heretic) return;

    MN_DrTextB("CHOOSE SKILL LEVEL:", 74, 16);
}

void MN_DrawOptions(void)
{
}

void MN_DrawSound(void)
{
  MN_DrawSlider(SoundDef.x - 8, SoundDef.y + ITEM_HEIGHT * SFX_VOL_INDEX, 16, snd_SfxVolume);

  MN_DrawSlider(SoundDef.x - 8, SoundDef.y + ITEM_HEIGHT * MUS_VOL_INDEX, 16, snd_MusicVolume);
}

extern char savegamestrings[10][SAVESTRINGSIZE];

static void MN_DrawFileSlots(int x, int y)
{
  int i;
  extern char save_page_string[];

  for (i = 0; i < g_menu_save_page_size; i++)
  {
    V_DrawNamePatch(x, y, 0, "M_FSLOT", CR_DEFAULT, VPT_STRETCH);
    MN_DrTextA(savegamestrings[i], x + 5, y + 5);
    y += ITEM_HEIGHT;
  }

  MN_DrTextA(save_page_string, x + 5, y + 5);
}

void MN_DrawLoad(void)
{
  const char *title;

  title = "LOAD GAME";

  MN_DrTextB(title, 160 - MN_TextBWidth(title) / 2, 10);
  MN_DrawFileSlots(LoadDef.x, LoadDef.y);

  if (delete_verify)
    M_DrawDelVerify();
}

extern int saveStringEnter;
extern int saveSlot;

void MN_DrawSave(void)
{
  const char *title;

  title = "SAVE GAME";

  MN_DrTextB(title, 160 - MN_TextBWidth(title) / 2, 10);
  MN_DrawFileSlots(SaveDef.x, SaveDef.y);

  if (saveStringEnter)
  {
    int i;

    i = MN_TextAWidth(savegamestrings[saveSlot]);
    MN_DrTextA("[", SaveDef.x + 5 + i, SaveDef.y + 5 + saveSlot * ITEM_HEIGHT); // [ is _ in font A
  }

  if (delete_verify)
    M_DrawDelVerify();
}

void MN_DrawPause(void)
{
  const char *title;

  title = "PAUSE";
  MN_DrTextB(title, 160 - MN_TextBWidth(title) / 2, 10);
}

void MN_DrTextA(const char *text, int x, int y)
{
  char c;
  int lump;

  while ((c = *text++) != 0)
  {
    c = toupper(c);
    if (c < 33)
    {
      x += 5;
    }
    else
    {
      lump = FontABaseLump + c - 33;
      V_DrawNumPatch(x, y, 0, lump, CR_DEFAULT, VPT_STRETCH);
      x += R_NumPatchWidth(lump) - 1;
    }
  }
}

int MN_TextAHeight(const char *text)
{
  int i, line_height, total_height;

  line_height = R_NumPatchHeight(FontABaseLump);
  total_height = line_height;

  for (i = 0; text[i]; i++)
    if (text[i] == '\n')
      total_height += line_height;
  return total_height;
}

int MN_TextAWidth(const char *text)
{
  char c;
  int width;
  int lump;

  width = 0;
  while ((c = *text++) != 0)
  {
    c = toupper(c);
    if (c < 33)
    {
      width += 5;
    }
    else
    {
      lump = FontABaseLump + c - 33;
      width += R_NumPatchWidth(lump) - 1;
    }
  }
  return (width);
}

void MN_DrTextB(const char *text, int x, int y)
{
  char c;
  int lump;

  while ((c = *text++) != 0)
  {
    c = toupper(c);
    if (c < 33)
    {
      x += 8;
    }
    else
    {
      lump = FontBBaseLump + c - 33;
      V_DrawNumPatch(x, y, 0, lump, CR_DEFAULT, VPT_STRETCH);
      x += R_NumPatchWidth(lump) - 1;
    }
  }
}

int MN_TextBWidth(const char *text)
{
  char c;
  int width;
  int lump;

  width = 0;
  while ((c = *text++) != 0)
  {
    c = toupper(c);
    if (c < 33)
    {
      width += 5;
    }
    else
    {
      lump = FontBBaseLump + c - 33;
      width += R_NumPatchWidth(lump) - 1;
    }
  }
  return (width);
}

#define SLIDER_LIMIT 200
#define SLIDER_WIDTH (SLIDER_LIMIT - 64)
#define SLIDER_PATCH_COUNT (SLIDER_WIDTH / 8)

void MN_DrawSlider(int x, int y, int width, int slot)
{
  int x2;
  int count;
  char num[4];
  int slot_x;
  short slider_img = 0;

  width = (width > SLIDER_LIMIT) ? SLIDER_LIMIT : width;

  V_DrawNamePatch(x, y, 0, "M_SLDLT", CR_DEFAULT, VPT_STRETCH);

  for (x2 = x + 32, count = SLIDER_PATCH_COUNT; count--; x2 += 8)
  {
    const char* name;

    name = (slider_img & 1 ? "M_SLDMD1" : "M_SLDMD2");
    slider_img ^= 1;

    V_DrawNamePatch(x2, y, 0, name, CR_DEFAULT, VPT_STRETCH);
  }

  V_DrawNamePatch(x2, y, 0, "M_SLDRT", CR_DEFAULT, VPT_STRETCH);

  // [crispy] print the value
  snprintf(num, sizeof(num), "%3d", slot);
  MN_DrTextA(num, x2 + 32, y + 3);

  // [crispy] do not crash anymore if the value is out of bounds
  if (slot >= width)
  {
    slot = width - 1;
  }

  slot_x = x + 36 + (SLIDER_WIDTH - 8) * slot / (width - 1);

  V_DrawNamePatch(slot_x, y + 7, 0, "M_SLDKB", CR_DEFAULT, VPT_STRETCH);
}

// hexen

void MN_DrTextAYellow(const char *text, int x, int y)
{
    char c;
    int lump;

    while ((c = *text++) != 0)
    {
      c = toupper(c);
      if (c < 33)
      {
        x += 5;
      }
      else
      {
        lump = FontAYellowBaseLump + c - 33;
        V_DrawNumPatch(x, y, 0, lump, CR_DEFAULT, VPT_STRETCH);
        x += R_NumPatchWidth(lump) - 1;
      }
    }
}
