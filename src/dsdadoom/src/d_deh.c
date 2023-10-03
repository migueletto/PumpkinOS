/* Emacs style mode select   -*- C -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2004 by
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
 *     Dehacked file support
 *     New for the TeamTNT "Boom" engine
 *
 * Author: Ty Halderman, TeamTNT
 *
 *--------------------------------------------------------------------*/

// killough 5/2/98: fixed headers, removed rendunant external declarations:
#include "doomdef.h"
#include "doomtype.h"
#include "doomstat.h"
#include "d_deh.h"
#include "sounds.h"
#include "info.h"
#include "m_cheat.h"
#include "p_inter.h"
#include "p_enemy.h"
#include "p_map.h"
#include "g_game.h"
#include "d_think.h"
#include "w_wad.h"
#include "m_file.h"
#include "m_misc.h"
#include "v_video.h"
#include "e6y.h"//e6y
#include "stricmp.h"

// CPhipps - modify to use logical output routine
#include "lprintf.h"

#include "dsda/args.h"
#include "dsda/mobjinfo.h"
#include "dsda/music.h"
#include "dsda/sfx.h"
#include "dsda/sprite.h"
#include "dsda/state.h"

#define TRUE 1
#define FALSE 0

static dboolean bfgcells_modified = false;

static dg_file_t *deh_log_file;

static int deh_log(const char *s, ...)
{
  int r;
  va_list v;

  if (!deh_log_file) return 0;

  va_start(v,s);
  r = DG_vprintf(deh_log_file, s, v);
  va_end(v);

  return r;
}

// e6y: for compatibility with BOOM deh parser
int deh_strcasecmp(const char *str1, const char *str2)
{
  if (prboom_comp[PC_BOOM_DEH_PARSER].state &&
      compatibility_level >= boom_compatibility_compatibility &&
      compatibility_level <= boom_202_compatibility)
  {
    return strcmp(str1, str2);
  }
  else
  {
    return strcasecmp(str1, str2);
  }
}

const char * deh_getBitsDelims(void)
{
  if (prboom_comp[PC_BOOM_DEH_PARSER].state &&
      compatibility_level >= boom_compatibility_compatibility &&
      compatibility_level <= boom_202_compatibility)
  {
    return "+";
  }
  else
  {
    return ",+| \t\f\r";
  }
}

// killough 10/98: new functions, to allow processing DEH files in-memory
// (e.g. from wads)

typedef struct {
  /* cph 2006/08/06 -
   * if lump != NULL, lump is the start of the lump,
   * inp is the current read pos. */
  const byte *inp, *lump;
  long size;
  /* else, !lump, and f is the file being read */
  dg_file_t* f;
} DEHFILE;

// killough 10/98: emulate IO whether input really comes from a file or not

static char *dehfgets(char *buf, size_t n, DEHFILE *fp)
{
  if (!fp->lump)                                     // If this is a real file,
    return DG_fgets(fp->f, buf, n);                   // return regular fgets
  if (!n || !*fp->inp || fp->size <= 0)                // If no more characters
    return NULL;
  if (n == 1)
    fp->size--, *buf = *fp->inp++;
  else
  {                                                // copy buffer
    char *p = buf;
    while (n > 1 && *fp->inp && fp->size &&
           (n--, fp->size--, *p++ = *fp->inp++) != '\n')
      ;
    *p = 0;
  }
  return buf;                                        // Return buffer pointer
}

static int dehfeof(DEHFILE *fp)
{
  return !fp->lump ? DG_eof(fp->f) : !*fp->inp || fp->size <= 0;
}

static int dehfgetc(DEHFILE *fp)
{
  //return !fp->lump ? fgetc(fp->f) : fp->size > 0 ?
  //  fp->size--, *fp->inp++ : EOF;

  char c;

  if (!fp->lump) {
    return DG_read(fp->f, &c, 1) == 1 ? c : EOF;
  } else if (fp->size > 0) {
    return fp->size--, *fp->inp++;
  } else {
    return EOF;
  }
}

static long dehftell(DEHFILE *fp)
{
  return !fp->lump ? DG_tell(fp->f) : (fp->inp - fp->lump);
}

static int dehfseek(DEHFILE *fp, long offset)
{
  if (!fp->lump)
    return DG_seek(fp->f, offset, DG_SEEK_SET);
  else
  {
    long total = (fp->inp - fp->lump) + fp->size;
    offset = BETWEEN(0, total, offset);
    fp->inp = fp->lump + offset;
    fp->size = total - offset;
    return 0;
  }
}

// haleyjd 9/22/99
int HelperThing = -1;     // in P_SpawnMapThing to substitute helper thing

// variables used in other routines
dboolean deh_pars = FALSE; // in wi_stuff to allow pars in modified games

// #include "d_deh.h" -- we don't do that here but we declare the
// variables.  This externalizes everything that there is a string
// set for in the language files.  See d_deh.h for detailed comments,
// original English values etc.  These are set to the macro values,
// which are set by D_ENGLSH.H or D_FRENCH.H(etc).  BEX files are a
// better way of changing these strings globally by language.

// ====================================================================
// Any of these can be changed using the bex extensions
#include "dstrings.h"  // to get the initial values

const char *s_PRESSKEY    = PRESSKEY;
const char *s_PRESSYN     = PRESSYN;
const char *s_QUITMSG     = QUITMSG;
const char *s_QSAVESPOT   = QSAVESPOT; // PRESSKEY;
const char *s_SAVEDEAD    = SAVEDEAD;  // PRESSKEY; // remove duplicate y/n
const char *s_NEWGAME     = NEWGAME;   // PRESSKEY;
const char *s_RESTARTLEVEL= RESTARTLEVEL; // PRESSYN;
const char *s_NIGHTMARE   = NIGHTMARE; // PRESSYN;
const char *s_SWSTRING    = SWSTRING;  // PRESSKEY;
const char *s_MSGOFF      = MSGOFF;
const char *s_MSGON       = MSGON;
const char *s_NETEND      = NETEND;    // PRESSKEY;
const char *s_ENDGAME     = ENDGAME;   // PRESSYN; // killough 4/4/98: end
const char *s_DOSY        = DOSY;
const char *s_DETAILHI    = DETAILHI;
const char *s_DETAILLO    = DETAILLO;
const char *s_GAMMALVL0   = GAMMALVL0;
const char *s_GAMMALVL1   = GAMMALVL1;
const char *s_GAMMALVL2   = GAMMALVL2;
const char *s_GAMMALVL3   = GAMMALVL3;
const char *s_GAMMALVL4   = GAMMALVL4;
const char *s_EMPTYSTRING = EMPTYSTRING;
const char *s_GOTARMOR    = GOTARMOR;
const char *s_GOTMEGA     = GOTMEGA;
const char *s_GOTHTHBONUS = GOTHTHBONUS;
const char *s_GOTARMBONUS = GOTARMBONUS;
const char *s_GOTSTIM     = GOTSTIM;
const char *s_GOTMEDINEED = GOTMEDINEED;
const char *s_GOTMEDIKIT  = GOTMEDIKIT;
const char *s_GOTSUPER    = GOTSUPER;
const char *s_GOTBLUECARD = GOTBLUECARD;
const char *s_GOTYELWCARD = GOTYELWCARD;
const char *s_GOTREDCARD  = GOTREDCARD;
const char *s_GOTBLUESKUL = GOTBLUESKUL;
const char *s_GOTYELWSKUL = GOTYELWSKUL;
const char *s_GOTREDSKULL = GOTREDSKULL;
const char *s_GOTINVUL    = GOTINVUL;
const char *s_GOTBERSERK  = GOTBERSERK;
const char *s_GOTINVIS    = GOTINVIS;
const char *s_GOTSUIT     = GOTSUIT;
const char *s_GOTMAP      = GOTMAP;
const char *s_GOTVISOR    = GOTVISOR;
const char *s_GOTMSPHERE  = GOTMSPHERE;
const char *s_GOTCLIP     = GOTCLIP;
const char *s_GOTCLIPBOX  = GOTCLIPBOX;
const char *s_GOTROCKET   = GOTROCKET;
const char *s_GOTROCKBOX  = GOTROCKBOX;
const char *s_GOTCELL     = GOTCELL;
const char *s_GOTCELLBOX  = GOTCELLBOX;
const char *s_GOTSHELLS   = GOTSHELLS;
const char *s_GOTSHELLBOX = GOTSHELLBOX;
const char *s_GOTBACKPACK = GOTBACKPACK;
const char *s_GOTBFG9000  = GOTBFG9000;
const char *s_GOTCHAINGUN = GOTCHAINGUN;
const char *s_GOTCHAINSAW = GOTCHAINSAW;
const char *s_GOTLAUNCHER = GOTLAUNCHER;
const char *s_GOTPLASMA   = GOTPLASMA;
const char *s_GOTSHOTGUN  = GOTSHOTGUN;
const char *s_GOTSHOTGUN2 = GOTSHOTGUN2;
const char *s_PD_BLUEO    = PD_BLUEO;
const char *s_PD_REDO     = PD_REDO;
const char *s_PD_YELLOWO  = PD_YELLOWO;
const char *s_PD_BLUEK    = PD_BLUEK;
const char *s_PD_REDK     = PD_REDK;
const char *s_PD_YELLOWK  = PD_YELLOWK;
const char *s_PD_BLUEC    = PD_BLUEC;
const char *s_PD_REDC     = PD_REDC;
const char *s_PD_YELLOWC  = PD_YELLOWC;
const char *s_PD_BLUES    = PD_BLUES;
const char *s_PD_REDS     = PD_REDS;
const char *s_PD_YELLOWS  = PD_YELLOWS;
const char *s_PD_ANY      = PD_ANY;
const char *s_PD_ALL3     = PD_ALL3;
const char *s_PD_ALL6     = PD_ALL6;
const char *s_GGSAVED     = GGSAVED;
const char *s_HUSTR_MSGU  = HUSTR_MSGU;
const char *s_HUSTR_E1M1  = HUSTR_E1M1;
const char *s_HUSTR_E1M2  = HUSTR_E1M2;
const char *s_HUSTR_E1M3  = HUSTR_E1M3;
const char *s_HUSTR_E1M4  = HUSTR_E1M4;
const char *s_HUSTR_E1M5  = HUSTR_E1M5;
const char *s_HUSTR_E1M6  = HUSTR_E1M6;
const char *s_HUSTR_E1M7  = HUSTR_E1M7;
const char *s_HUSTR_E1M8  = HUSTR_E1M8;
const char *s_HUSTR_E1M9  = HUSTR_E1M9;
const char *s_HUSTR_E2M1  = HUSTR_E2M1;
const char *s_HUSTR_E2M2  = HUSTR_E2M2;
const char *s_HUSTR_E2M3  = HUSTR_E2M3;
const char *s_HUSTR_E2M4  = HUSTR_E2M4;
const char *s_HUSTR_E2M5  = HUSTR_E2M5;
const char *s_HUSTR_E2M6  = HUSTR_E2M6;
const char *s_HUSTR_E2M7  = HUSTR_E2M7;
const char *s_HUSTR_E2M8  = HUSTR_E2M8;
const char *s_HUSTR_E2M9  = HUSTR_E2M9;
const char *s_HUSTR_E3M1  = HUSTR_E3M1;
const char *s_HUSTR_E3M2  = HUSTR_E3M2;
const char *s_HUSTR_E3M3  = HUSTR_E3M3;
const char *s_HUSTR_E3M4  = HUSTR_E3M4;
const char *s_HUSTR_E3M5  = HUSTR_E3M5;
const char *s_HUSTR_E3M6  = HUSTR_E3M6;
const char *s_HUSTR_E3M7  = HUSTR_E3M7;
const char *s_HUSTR_E3M8  = HUSTR_E3M8;
const char *s_HUSTR_E3M9  = HUSTR_E3M9;
const char *s_HUSTR_E4M1  = HUSTR_E4M1;
const char *s_HUSTR_E4M2  = HUSTR_E4M2;
const char *s_HUSTR_E4M3  = HUSTR_E4M3;
const char *s_HUSTR_E4M4  = HUSTR_E4M4;
const char *s_HUSTR_E4M5  = HUSTR_E4M5;
const char *s_HUSTR_E4M6  = HUSTR_E4M6;
const char *s_HUSTR_E4M7  = HUSTR_E4M7;
const char *s_HUSTR_E4M8  = HUSTR_E4M8;
const char *s_HUSTR_E4M9  = HUSTR_E4M9;
const char *s_HUSTR_1     = HUSTR_1;
const char *s_HUSTR_2     = HUSTR_2;
const char *s_HUSTR_3     = HUSTR_3;
const char *s_HUSTR_4     = HUSTR_4;
const char *s_HUSTR_5     = HUSTR_5;
const char *s_HUSTR_6     = HUSTR_6;
const char *s_HUSTR_7     = HUSTR_7;
const char *s_HUSTR_8     = HUSTR_8;
const char *s_HUSTR_9     = HUSTR_9;
const char *s_HUSTR_10    = HUSTR_10;
const char *s_HUSTR_11    = HUSTR_11;
const char *s_HUSTR_12    = HUSTR_12;
const char *s_HUSTR_13    = HUSTR_13;
const char *s_HUSTR_14    = HUSTR_14;
const char *s_HUSTR_15    = HUSTR_15;
const char *s_HUSTR_16    = HUSTR_16;
const char *s_HUSTR_17    = HUSTR_17;
const char *s_HUSTR_18    = HUSTR_18;
const char *s_HUSTR_19    = HUSTR_19;
const char *s_HUSTR_20    = HUSTR_20;
const char *s_HUSTR_21    = HUSTR_21;
const char *s_HUSTR_22    = HUSTR_22;
const char *s_HUSTR_23    = HUSTR_23;
const char *s_HUSTR_24    = HUSTR_24;
const char *s_HUSTR_25    = HUSTR_25;
const char *s_HUSTR_26    = HUSTR_26;
const char *s_HUSTR_27    = HUSTR_27;
const char *s_HUSTR_28    = HUSTR_28;
const char *s_HUSTR_29    = HUSTR_29;
const char *s_HUSTR_30    = HUSTR_30;
const char *s_HUSTR_31    = HUSTR_31;
const char *s_HUSTR_32    = HUSTR_32;
const char *s_HUSTR_33    = HUSTR_33;
const char *s_PHUSTR_1    = PHUSTR_1;
const char *s_PHUSTR_2    = PHUSTR_2;
const char *s_PHUSTR_3    = PHUSTR_3;
const char *s_PHUSTR_4    = PHUSTR_4;
const char *s_PHUSTR_5    = PHUSTR_5;
const char *s_PHUSTR_6    = PHUSTR_6;
const char *s_PHUSTR_7    = PHUSTR_7;
const char *s_PHUSTR_8    = PHUSTR_8;
const char *s_PHUSTR_9    = PHUSTR_9;
const char *s_PHUSTR_10   = PHUSTR_10;
const char *s_PHUSTR_11   = PHUSTR_11;
const char *s_PHUSTR_12   = PHUSTR_12;
const char *s_PHUSTR_13   = PHUSTR_13;
const char *s_PHUSTR_14   = PHUSTR_14;
const char *s_PHUSTR_15   = PHUSTR_15;
const char *s_PHUSTR_16   = PHUSTR_16;
const char *s_PHUSTR_17   = PHUSTR_17;
const char *s_PHUSTR_18   = PHUSTR_18;
const char *s_PHUSTR_19   = PHUSTR_19;
const char *s_PHUSTR_20   = PHUSTR_20;
const char *s_PHUSTR_21   = PHUSTR_21;
const char *s_PHUSTR_22   = PHUSTR_22;
const char *s_PHUSTR_23   = PHUSTR_23;
const char *s_PHUSTR_24   = PHUSTR_24;
const char *s_PHUSTR_25   = PHUSTR_25;
const char *s_PHUSTR_26   = PHUSTR_26;
const char *s_PHUSTR_27   = PHUSTR_27;
const char *s_PHUSTR_28   = PHUSTR_28;
const char *s_PHUSTR_29   = PHUSTR_29;
const char *s_PHUSTR_30   = PHUSTR_30;
const char *s_PHUSTR_31   = PHUSTR_31;
const char *s_PHUSTR_32   = PHUSTR_32;
const char *s_THUSTR_1    = THUSTR_1;
const char *s_THUSTR_2    = THUSTR_2;
const char *s_THUSTR_3    = THUSTR_3;
const char *s_THUSTR_4    = THUSTR_4;
const char *s_THUSTR_5    = THUSTR_5;
const char *s_THUSTR_6    = THUSTR_6;
const char *s_THUSTR_7    = THUSTR_7;
const char *s_THUSTR_8    = THUSTR_8;
const char *s_THUSTR_9    = THUSTR_9;
const char *s_THUSTR_10   = THUSTR_10;
const char *s_THUSTR_11   = THUSTR_11;
const char *s_THUSTR_12   = THUSTR_12;
const char *s_THUSTR_13   = THUSTR_13;
const char *s_THUSTR_14   = THUSTR_14;
const char *s_THUSTR_15   = THUSTR_15;
const char *s_THUSTR_16   = THUSTR_16;
const char *s_THUSTR_17   = THUSTR_17;
const char *s_THUSTR_18   = THUSTR_18;
const char *s_THUSTR_19   = THUSTR_19;
const char *s_THUSTR_20   = THUSTR_20;
const char *s_THUSTR_21   = THUSTR_21;
const char *s_THUSTR_22   = THUSTR_22;
const char *s_THUSTR_23   = THUSTR_23;
const char *s_THUSTR_24   = THUSTR_24;
const char *s_THUSTR_25   = THUSTR_25;
const char *s_THUSTR_26   = THUSTR_26;
const char *s_THUSTR_27   = THUSTR_27;
const char *s_THUSTR_28   = THUSTR_28;
const char *s_THUSTR_29   = THUSTR_29;
const char *s_THUSTR_30   = THUSTR_30;
const char *s_THUSTR_31   = THUSTR_31;
const char *s_THUSTR_32   = THUSTR_32;
const char *s_AMSTR_FOLLOWON     = AMSTR_FOLLOWON;
const char *s_AMSTR_FOLLOWOFF    = AMSTR_FOLLOWOFF;
const char *s_AMSTR_GRIDON       = AMSTR_GRIDON;
const char *s_AMSTR_GRIDOFF      = AMSTR_GRIDOFF;
const char *s_AMSTR_MARKEDSPOT   = AMSTR_MARKEDSPOT;
const char *s_AMSTR_MARKSCLEARED = AMSTR_MARKSCLEARED;
// CPhipps - automap rotate & overlay
const char* s_AMSTR_ROTATEON     = AMSTR_ROTATEON;
const char* s_AMSTR_ROTATEOFF    = AMSTR_ROTATEOFF;
const char* s_AMSTR_OVERLAYON    = AMSTR_OVERLAYON;
const char* s_AMSTR_OVERLAYOFF   = AMSTR_OVERLAYOFF;
// e6y: textured automap
const char* s_AMSTR_TEXTUREDON   = AMSTR_TEXTUREDON;
const char* s_AMSTR_TEXTUREDOFF  = AMSTR_TEXTUREDOFF;

const char *s_STSTR_MUS          = STSTR_MUS;
const char *s_STSTR_NOMUS        = STSTR_NOMUS;
const char *s_STSTR_DQDON        = STSTR_DQDON;
const char *s_STSTR_DQDOFF       = STSTR_DQDOFF;
const char *s_STSTR_KFAADDED     = STSTR_KFAADDED;
const char *s_STSTR_FAADDED      = STSTR_FAADDED;
const char *s_STSTR_NCON         = STSTR_NCON;
const char *s_STSTR_NCOFF        = STSTR_NCOFF;
const char *s_STSTR_BEHOLD       = STSTR_BEHOLD;
const char *s_STSTR_BEHOLDX      = STSTR_BEHOLDX;
const char *s_STSTR_CHOPPERS     = STSTR_CHOPPERS;
const char *s_STSTR_CLEV         = STSTR_CLEV;
const char *s_STSTR_COMPON       = STSTR_COMPON;
const char *s_STSTR_COMPOFF      = STSTR_COMPOFF;
const char *s_E1TEXT     = E1TEXT;
const char *s_E2TEXT     = E2TEXT;
const char *s_E3TEXT     = E3TEXT;
const char *s_E4TEXT     = E4TEXT;
const char *s_C1TEXT     = C1TEXT;
const char *s_C2TEXT     = C2TEXT;
const char *s_C3TEXT     = C3TEXT;
const char *s_C4TEXT     = C4TEXT;
const char *s_C5TEXT     = C5TEXT;
const char *s_C6TEXT     = C6TEXT;
const char *s_P1TEXT     = P1TEXT;
const char *s_P2TEXT     = P2TEXT;
const char *s_P3TEXT     = P3TEXT;
const char *s_P4TEXT     = P4TEXT;
const char *s_P5TEXT     = P5TEXT;
const char *s_P6TEXT     = P6TEXT;
const char *s_T1TEXT     = T1TEXT;
const char *s_T2TEXT     = T2TEXT;
const char *s_T3TEXT     = T3TEXT;
const char *s_T4TEXT     = T4TEXT;
const char *s_T5TEXT     = T5TEXT;
const char *s_T6TEXT     = T6TEXT;
const char *s_CC_ZOMBIE  = CC_ZOMBIE;
const char *s_CC_SHOTGUN = CC_SHOTGUN;
const char *s_CC_HEAVY   = CC_HEAVY;
const char *s_CC_IMP     = CC_IMP;
const char *s_CC_DEMON   = CC_DEMON;
const char *s_CC_LOST    = CC_LOST;
const char *s_CC_CACO    = CC_CACO;
const char *s_CC_HELL    = CC_HELL;
const char *s_CC_BARON   = CC_BARON;
const char *s_CC_ARACH   = CC_ARACH;
const char *s_CC_PAIN    = CC_PAIN;
const char *s_CC_REVEN   = CC_REVEN;
const char *s_CC_MANCU   = CC_MANCU;
const char *s_CC_ARCH    = CC_ARCH;
const char *s_CC_SPIDER  = CC_SPIDER;
const char *s_CC_CYBER   = CC_CYBER;
const char *s_CC_HERO    = CC_HERO;
// Ty 03/30/98 - new substitutions for background textures
//               during int screens
const char *bgflatE1     = "FLOOR4_8"; // end of DOOM Episode 1
const char *bgflatE2     = "SFLR6_1";  // end of DOOM Episode 2
const char *bgflatE3     = "MFLR8_4";  // end of DOOM Episode 3
const char *bgflatE4     = "MFLR8_3";  // end of DOOM Episode 4
const char *bgflat06     = "SLIME16";  // DOOM2 after MAP06
const char *bgflat11     = "RROCK14";  // DOOM2 after MAP11
const char *bgflat20     = "RROCK07";  // DOOM2 after MAP20
const char *bgflat30     = "RROCK17";  // DOOM2 after MAP30
const char *bgflat15     = "RROCK13";  // DOOM2 going MAP15 to MAP31
const char *bgflat31     = "RROCK19";  // DOOM2 going MAP31 to MAP32
const char *bgcastcall   = "BOSSBACK"; // Panel behind cast call

/* Ty 05/03/98 - externalized
 * cph - updated for prboom */
const char *savegamename = PACKAGE_TARNAME"-savegame";

// end d_deh.h variable declarations
// ====================================================================

// Do this for a lookup--the pointer (loaded above) is cross-referenced
// to a string key that is the same as the define above.  We will use
// strdups to set these new values that we read from the file, orphaning
// the original value set above.

// CPhipps - make strings pointed to const
typedef struct {
  const char **ppstr;  // doubly indirect pointer to string
  const char *lookup;  // pointer to lookup string name
  const char *orig;
} deh_strs;

/* CPhipps - const
 *         - removed redundant "Can't XXX in a netgame" strings
 */
static deh_strs deh_strlookup[] = {
  {&s_PRESSKEY,"PRESSKEY"},
  {&s_PRESSYN,"PRESSYN"},
  {&s_QUITMSG,"QUITMSG"},
  {&s_QSAVESPOT,"QSAVESPOT"},
  {&s_SAVEDEAD,"SAVEDEAD"},
  {&s_NEWGAME,"NEWGAME"},
  {&s_RESTARTLEVEL,"RESTARTLEVEL"},
  {&s_NIGHTMARE,"NIGHTMARE"},
  {&s_SWSTRING,"SWSTRING"},
  {&s_MSGOFF,"MSGOFF"},
  {&s_MSGON,"MSGON"},
  {&s_NETEND,"NETEND"},
  {&s_ENDGAME,"ENDGAME"},
  {&s_DOSY,"DOSY"},
  {&s_DETAILHI,"DETAILHI"},
  {&s_DETAILLO,"DETAILLO"},
  {&s_GAMMALVL0,"GAMMALVL0"},
  {&s_GAMMALVL1,"GAMMALVL1"},
  {&s_GAMMALVL2,"GAMMALVL2"},
  {&s_GAMMALVL3,"GAMMALVL3"},
  {&s_GAMMALVL4,"GAMMALVL4"},
  {&s_EMPTYSTRING,"EMPTYSTRING"},
  {&s_GOTARMOR,"GOTARMOR"},
  {&s_GOTMEGA,"GOTMEGA"},
  {&s_GOTHTHBONUS,"GOTHTHBONUS"},
  {&s_GOTARMBONUS,"GOTARMBONUS"},
  {&s_GOTSTIM,"GOTSTIM"},
  {&s_GOTMEDINEED,"GOTMEDINEED"},
  {&s_GOTMEDIKIT,"GOTMEDIKIT"},
  {&s_GOTSUPER,"GOTSUPER"},
  {&s_GOTBLUECARD,"GOTBLUECARD"},
  {&s_GOTYELWCARD,"GOTYELWCARD"},
  {&s_GOTREDCARD,"GOTREDCARD"},
  {&s_GOTBLUESKUL,"GOTBLUESKUL"},
  {&s_GOTYELWSKUL,"GOTYELWSKUL"},
  {&s_GOTREDSKULL,"GOTREDSKULL"},
  {&s_GOTINVUL,"GOTINVUL"},
  {&s_GOTBERSERK,"GOTBERSERK"},
  {&s_GOTINVIS,"GOTINVIS"},
  {&s_GOTSUIT,"GOTSUIT"},
  {&s_GOTMAP,"GOTMAP"},
  {&s_GOTVISOR,"GOTVISOR"},
  {&s_GOTMSPHERE,"GOTMSPHERE"},
  {&s_GOTCLIP,"GOTCLIP"},
  {&s_GOTCLIPBOX,"GOTCLIPBOX"},
  {&s_GOTROCKET,"GOTROCKET"},
  {&s_GOTROCKBOX,"GOTROCKBOX"},
  {&s_GOTCELL,"GOTCELL"},
  {&s_GOTCELLBOX,"GOTCELLBOX"},
  {&s_GOTSHELLS,"GOTSHELLS"},
  {&s_GOTSHELLBOX,"GOTSHELLBOX"},
  {&s_GOTBACKPACK,"GOTBACKPACK"},
  {&s_GOTBFG9000,"GOTBFG9000"},
  {&s_GOTCHAINGUN,"GOTCHAINGUN"},
  {&s_GOTCHAINSAW,"GOTCHAINSAW"},
  {&s_GOTLAUNCHER,"GOTLAUNCHER"},
  {&s_GOTPLASMA,"GOTPLASMA"},
  {&s_GOTSHOTGUN,"GOTSHOTGUN"},
  {&s_GOTSHOTGUN2,"GOTSHOTGUN2"},
  {&s_PD_BLUEO,"PD_BLUEO"},
  {&s_PD_REDO,"PD_REDO"},
  {&s_PD_YELLOWO,"PD_YELLOWO"},
  {&s_PD_BLUEK,"PD_BLUEK"},
  {&s_PD_REDK,"PD_REDK"},
  {&s_PD_YELLOWK,"PD_YELLOWK"},
  {&s_PD_BLUEC,"PD_BLUEC"},
  {&s_PD_REDC,"PD_REDC"},
  {&s_PD_YELLOWC,"PD_YELLOWC"},
  {&s_PD_BLUES,"PD_BLUES"},
  {&s_PD_REDS,"PD_REDS"},
  {&s_PD_YELLOWS,"PD_YELLOWS"},
  {&s_PD_ANY,"PD_ANY"},
  {&s_PD_ALL3,"PD_ALL3"},
  {&s_PD_ALL6,"PD_ALL6"},
  {&s_GGSAVED,"GGSAVED"},
  {&s_HUSTR_MSGU,"HUSTR_MSGU"},
  {&s_HUSTR_E1M1,"HUSTR_E1M1"},
  {&s_HUSTR_E1M2,"HUSTR_E1M2"},
  {&s_HUSTR_E1M3,"HUSTR_E1M3"},
  {&s_HUSTR_E1M4,"HUSTR_E1M4"},
  {&s_HUSTR_E1M5,"HUSTR_E1M5"},
  {&s_HUSTR_E1M6,"HUSTR_E1M6"},
  {&s_HUSTR_E1M7,"HUSTR_E1M7"},
  {&s_HUSTR_E1M8,"HUSTR_E1M8"},
  {&s_HUSTR_E1M9,"HUSTR_E1M9"},
  {&s_HUSTR_E2M1,"HUSTR_E2M1"},
  {&s_HUSTR_E2M2,"HUSTR_E2M2"},
  {&s_HUSTR_E2M3,"HUSTR_E2M3"},
  {&s_HUSTR_E2M4,"HUSTR_E2M4"},
  {&s_HUSTR_E2M5,"HUSTR_E2M5"},
  {&s_HUSTR_E2M6,"HUSTR_E2M6"},
  {&s_HUSTR_E2M7,"HUSTR_E2M7"},
  {&s_HUSTR_E2M8,"HUSTR_E2M8"},
  {&s_HUSTR_E2M9,"HUSTR_E2M9"},
  {&s_HUSTR_E3M1,"HUSTR_E3M1"},
  {&s_HUSTR_E3M2,"HUSTR_E3M2"},
  {&s_HUSTR_E3M3,"HUSTR_E3M3"},
  {&s_HUSTR_E3M4,"HUSTR_E3M4"},
  {&s_HUSTR_E3M5,"HUSTR_E3M5"},
  {&s_HUSTR_E3M6,"HUSTR_E3M6"},
  {&s_HUSTR_E3M7,"HUSTR_E3M7"},
  {&s_HUSTR_E3M8,"HUSTR_E3M8"},
  {&s_HUSTR_E3M9,"HUSTR_E3M9"},
  {&s_HUSTR_E4M1,"HUSTR_E4M1"},
  {&s_HUSTR_E4M2,"HUSTR_E4M2"},
  {&s_HUSTR_E4M3,"HUSTR_E4M3"},
  {&s_HUSTR_E4M4,"HUSTR_E4M4"},
  {&s_HUSTR_E4M5,"HUSTR_E4M5"},
  {&s_HUSTR_E4M6,"HUSTR_E4M6"},
  {&s_HUSTR_E4M7,"HUSTR_E4M7"},
  {&s_HUSTR_E4M8,"HUSTR_E4M8"},
  {&s_HUSTR_E4M9,"HUSTR_E4M9"},
  {&s_HUSTR_1,"HUSTR_1"},
  {&s_HUSTR_2,"HUSTR_2"},
  {&s_HUSTR_3,"HUSTR_3"},
  {&s_HUSTR_4,"HUSTR_4"},
  {&s_HUSTR_5,"HUSTR_5"},
  {&s_HUSTR_6,"HUSTR_6"},
  {&s_HUSTR_7,"HUSTR_7"},
  {&s_HUSTR_8,"HUSTR_8"},
  {&s_HUSTR_9,"HUSTR_9"},
  {&s_HUSTR_10,"HUSTR_10"},
  {&s_HUSTR_11,"HUSTR_11"},
  {&s_HUSTR_12,"HUSTR_12"},
  {&s_HUSTR_13,"HUSTR_13"},
  {&s_HUSTR_14,"HUSTR_14"},
  {&s_HUSTR_15,"HUSTR_15"},
  {&s_HUSTR_16,"HUSTR_16"},
  {&s_HUSTR_17,"HUSTR_17"},
  {&s_HUSTR_18,"HUSTR_18"},
  {&s_HUSTR_19,"HUSTR_19"},
  {&s_HUSTR_20,"HUSTR_20"},
  {&s_HUSTR_21,"HUSTR_21"},
  {&s_HUSTR_22,"HUSTR_22"},
  {&s_HUSTR_23,"HUSTR_23"},
  {&s_HUSTR_24,"HUSTR_24"},
  {&s_HUSTR_25,"HUSTR_25"},
  {&s_HUSTR_26,"HUSTR_26"},
  {&s_HUSTR_27,"HUSTR_27"},
  {&s_HUSTR_28,"HUSTR_28"},
  {&s_HUSTR_29,"HUSTR_29"},
  {&s_HUSTR_30,"HUSTR_30"},
  {&s_HUSTR_31,"HUSTR_31"},
  {&s_HUSTR_32,"HUSTR_32"},
  {&s_HUSTR_33,"HUSTR_33"},
  {&s_PHUSTR_1,"PHUSTR_1"},
  {&s_PHUSTR_2,"PHUSTR_2"},
  {&s_PHUSTR_3,"PHUSTR_3"},
  {&s_PHUSTR_4,"PHUSTR_4"},
  {&s_PHUSTR_5,"PHUSTR_5"},
  {&s_PHUSTR_6,"PHUSTR_6"},
  {&s_PHUSTR_7,"PHUSTR_7"},
  {&s_PHUSTR_8,"PHUSTR_8"},
  {&s_PHUSTR_9,"PHUSTR_9"},
  {&s_PHUSTR_10,"PHUSTR_10"},
  {&s_PHUSTR_11,"PHUSTR_11"},
  {&s_PHUSTR_12,"PHUSTR_12"},
  {&s_PHUSTR_13,"PHUSTR_13"},
  {&s_PHUSTR_14,"PHUSTR_14"},
  {&s_PHUSTR_15,"PHUSTR_15"},
  {&s_PHUSTR_16,"PHUSTR_16"},
  {&s_PHUSTR_17,"PHUSTR_17"},
  {&s_PHUSTR_18,"PHUSTR_18"},
  {&s_PHUSTR_19,"PHUSTR_19"},
  {&s_PHUSTR_20,"PHUSTR_20"},
  {&s_PHUSTR_21,"PHUSTR_21"},
  {&s_PHUSTR_22,"PHUSTR_22"},
  {&s_PHUSTR_23,"PHUSTR_23"},
  {&s_PHUSTR_24,"PHUSTR_24"},
  {&s_PHUSTR_25,"PHUSTR_25"},
  {&s_PHUSTR_26,"PHUSTR_26"},
  {&s_PHUSTR_27,"PHUSTR_27"},
  {&s_PHUSTR_28,"PHUSTR_28"},
  {&s_PHUSTR_29,"PHUSTR_29"},
  {&s_PHUSTR_30,"PHUSTR_30"},
  {&s_PHUSTR_31,"PHUSTR_31"},
  {&s_PHUSTR_32,"PHUSTR_32"},
  {&s_THUSTR_1,"THUSTR_1"},
  {&s_THUSTR_2,"THUSTR_2"},
  {&s_THUSTR_3,"THUSTR_3"},
  {&s_THUSTR_4,"THUSTR_4"},
  {&s_THUSTR_5,"THUSTR_5"},
  {&s_THUSTR_6,"THUSTR_6"},
  {&s_THUSTR_7,"THUSTR_7"},
  {&s_THUSTR_8,"THUSTR_8"},
  {&s_THUSTR_9,"THUSTR_9"},
  {&s_THUSTR_10,"THUSTR_10"},
  {&s_THUSTR_11,"THUSTR_11"},
  {&s_THUSTR_12,"THUSTR_12"},
  {&s_THUSTR_13,"THUSTR_13"},
  {&s_THUSTR_14,"THUSTR_14"},
  {&s_THUSTR_15,"THUSTR_15"},
  {&s_THUSTR_16,"THUSTR_16"},
  {&s_THUSTR_17,"THUSTR_17"},
  {&s_THUSTR_18,"THUSTR_18"},
  {&s_THUSTR_19,"THUSTR_19"},
  {&s_THUSTR_20,"THUSTR_20"},
  {&s_THUSTR_21,"THUSTR_21"},
  {&s_THUSTR_22,"THUSTR_22"},
  {&s_THUSTR_23,"THUSTR_23"},
  {&s_THUSTR_24,"THUSTR_24"},
  {&s_THUSTR_25,"THUSTR_25"},
  {&s_THUSTR_26,"THUSTR_26"},
  {&s_THUSTR_27,"THUSTR_27"},
  {&s_THUSTR_28,"THUSTR_28"},
  {&s_THUSTR_29,"THUSTR_29"},
  {&s_THUSTR_30,"THUSTR_30"},
  {&s_THUSTR_31,"THUSTR_31"},
  {&s_THUSTR_32,"THUSTR_32"},
  {&s_AMSTR_FOLLOWON,"AMSTR_FOLLOWON"},
  {&s_AMSTR_FOLLOWOFF,"AMSTR_FOLLOWOFF"},
  {&s_AMSTR_GRIDON,"AMSTR_GRIDON"},
  {&s_AMSTR_GRIDOFF,"AMSTR_GRIDOFF"},
  {&s_AMSTR_MARKEDSPOT,"AMSTR_MARKEDSPOT"},
  {&s_AMSTR_MARKSCLEARED,"AMSTR_MARKSCLEARED"},
  {&s_STSTR_MUS,"STSTR_MUS"},
  {&s_STSTR_NOMUS,"STSTR_NOMUS"},
  {&s_STSTR_DQDON,"STSTR_DQDON"},
  {&s_STSTR_DQDOFF,"STSTR_DQDOFF"},
  {&s_STSTR_KFAADDED,"STSTR_KFAADDED"},
  {&s_STSTR_FAADDED,"STSTR_FAADDED"},
  {&s_STSTR_NCON,"STSTR_NCON"},
  {&s_STSTR_NCOFF,"STSTR_NCOFF"},
  {&s_STSTR_BEHOLD,"STSTR_BEHOLD"},
  {&s_STSTR_BEHOLDX,"STSTR_BEHOLDX"},
  {&s_STSTR_CHOPPERS,"STSTR_CHOPPERS"},
  {&s_STSTR_CLEV,"STSTR_CLEV"},
  {&s_STSTR_COMPON,"STSTR_COMPON"},
  {&s_STSTR_COMPOFF,"STSTR_COMPOFF"},
  {&s_E1TEXT,"E1TEXT"},
  {&s_E2TEXT,"E2TEXT"},
  {&s_E3TEXT,"E3TEXT"},
  {&s_E4TEXT,"E4TEXT"},
  {&s_C1TEXT,"C1TEXT"},
  {&s_C2TEXT,"C2TEXT"},
  {&s_C3TEXT,"C3TEXT"},
  {&s_C4TEXT,"C4TEXT"},
  {&s_C5TEXT,"C5TEXT"},
  {&s_C6TEXT,"C6TEXT"},
  {&s_P1TEXT,"P1TEXT"},
  {&s_P2TEXT,"P2TEXT"},
  {&s_P3TEXT,"P3TEXT"},
  {&s_P4TEXT,"P4TEXT"},
  {&s_P5TEXT,"P5TEXT"},
  {&s_P6TEXT,"P6TEXT"},
  {&s_T1TEXT,"T1TEXT"},
  {&s_T2TEXT,"T2TEXT"},
  {&s_T3TEXT,"T3TEXT"},
  {&s_T4TEXT,"T4TEXT"},
  {&s_T5TEXT,"T5TEXT"},
  {&s_T6TEXT,"T6TEXT"},
  {&s_CC_ZOMBIE,"CC_ZOMBIE"},
  {&s_CC_SHOTGUN,"CC_SHOTGUN"},
  {&s_CC_HEAVY,"CC_HEAVY"},
  {&s_CC_IMP,"CC_IMP"},
  {&s_CC_DEMON,"CC_DEMON"},
  {&s_CC_LOST,"CC_LOST"},
  {&s_CC_CACO,"CC_CACO"},
  {&s_CC_HELL,"CC_HELL"},
  {&s_CC_BARON,"CC_BARON"},
  {&s_CC_ARACH,"CC_ARACH"},
  {&s_CC_PAIN,"CC_PAIN"},
  {&s_CC_REVEN,"CC_REVEN"},
  {&s_CC_MANCU,"CC_MANCU"},
  {&s_CC_ARCH,"CC_ARCH"},
  {&s_CC_SPIDER,"CC_SPIDER"},
  {&s_CC_CYBER,"CC_CYBER"},
  {&s_CC_HERO,"CC_HERO"},
  {&bgflatE1,"BGFLATE1"},
  {&bgflatE2,"BGFLATE2"},
  {&bgflatE3,"BGFLATE3"},
  {&bgflatE4,"BGFLATE4"},
  {&bgflat06,"BGFLAT06"},
  {&bgflat11,"BGFLAT11"},
  {&bgflat20,"BGFLAT20"},
  {&bgflat30,"BGFLAT30"},
  {&bgflat15,"BGFLAT15"},
  {&bgflat31,"BGFLAT31"},
  {&bgcastcall,"BGCASTCALL"},
  {&savegamename,"SAVEGAMENAME"},  // Ty 05/03/98
};

static int deh_numstrlookup = sizeof(deh_strlookup) / sizeof(deh_strlookup[0]);

const char *deh_newlevel = "NEWLEVEL"; // CPhipps - const

// DOOM shareware/registered/retail (Ultimate) names.
// CPhipps - const**const
const char **const mapnames[] =
{
  &s_HUSTR_E1M1,
  &s_HUSTR_E1M2,
  &s_HUSTR_E1M3,
  &s_HUSTR_E1M4,
  &s_HUSTR_E1M5,
  &s_HUSTR_E1M6,
  &s_HUSTR_E1M7,
  &s_HUSTR_E1M8,
  &s_HUSTR_E1M9,

  &s_HUSTR_E2M1,
  &s_HUSTR_E2M2,
  &s_HUSTR_E2M3,
  &s_HUSTR_E2M4,
  &s_HUSTR_E2M5,
  &s_HUSTR_E2M6,
  &s_HUSTR_E2M7,
  &s_HUSTR_E2M8,
  &s_HUSTR_E2M9,

  &s_HUSTR_E3M1,
  &s_HUSTR_E3M2,
  &s_HUSTR_E3M3,
  &s_HUSTR_E3M4,
  &s_HUSTR_E3M5,
  &s_HUSTR_E3M6,
  &s_HUSTR_E3M7,
  &s_HUSTR_E3M8,
  &s_HUSTR_E3M9,

  &s_HUSTR_E4M1,
  &s_HUSTR_E4M2,
  &s_HUSTR_E4M3,
  &s_HUSTR_E4M4,
  &s_HUSTR_E4M5,
  &s_HUSTR_E4M6,
  &s_HUSTR_E4M7,
  &s_HUSTR_E4M8,
  &s_HUSTR_E4M9,

  &deh_newlevel,  // spares?  Unused.
  &deh_newlevel,
  &deh_newlevel,
  &deh_newlevel,
  &deh_newlevel,
  &deh_newlevel,
  &deh_newlevel,
  &deh_newlevel,
  &deh_newlevel,

  &deh_newlevel,  // spares?  Unused.
  &deh_newlevel,
  &deh_newlevel,
  &deh_newlevel,
  &deh_newlevel,
  &deh_newlevel,
  &deh_newlevel,
  &deh_newlevel,
  &deh_newlevel
};

// CPhipps - const**const
const char **const mapnames2[] = // DOOM 2 map names.
{
  &s_HUSTR_1,
  &s_HUSTR_2,
  &s_HUSTR_3,
  &s_HUSTR_4,
  &s_HUSTR_5,
  &s_HUSTR_6,
  &s_HUSTR_7,
  &s_HUSTR_8,
  &s_HUSTR_9,
  &s_HUSTR_10,
  &s_HUSTR_11,

  &s_HUSTR_12,
  &s_HUSTR_13,
  &s_HUSTR_14,
  &s_HUSTR_15,
  &s_HUSTR_16,
  &s_HUSTR_17,
  &s_HUSTR_18,
  &s_HUSTR_19,
  &s_HUSTR_20,

  &s_HUSTR_21,
  &s_HUSTR_22,
  &s_HUSTR_23,
  &s_HUSTR_24,
  &s_HUSTR_25,
  &s_HUSTR_26,
  &s_HUSTR_27,
  &s_HUSTR_28,
  &s_HUSTR_29,
  &s_HUSTR_30,
  &s_HUSTR_31,
  &s_HUSTR_32,
  &s_HUSTR_33,
};

// CPhipps - const**const
const char **const mapnamesp[] = // Plutonia WAD map names.
{
  &s_PHUSTR_1,
  &s_PHUSTR_2,
  &s_PHUSTR_3,
  &s_PHUSTR_4,
  &s_PHUSTR_5,
  &s_PHUSTR_6,
  &s_PHUSTR_7,
  &s_PHUSTR_8,
  &s_PHUSTR_9,
  &s_PHUSTR_10,
  &s_PHUSTR_11,

  &s_PHUSTR_12,
  &s_PHUSTR_13,
  &s_PHUSTR_14,
  &s_PHUSTR_15,
  &s_PHUSTR_16,
  &s_PHUSTR_17,
  &s_PHUSTR_18,
  &s_PHUSTR_19,
  &s_PHUSTR_20,

  &s_PHUSTR_21,
  &s_PHUSTR_22,
  &s_PHUSTR_23,
  &s_PHUSTR_24,
  &s_PHUSTR_25,
  &s_PHUSTR_26,
  &s_PHUSTR_27,
  &s_PHUSTR_28,
  &s_PHUSTR_29,
  &s_PHUSTR_30,
  &s_PHUSTR_31,
  &s_PHUSTR_32,
};

// CPhipps - const**const
const char **const mapnamest[] = // TNT WAD map names.
{
  &s_THUSTR_1,
  &s_THUSTR_2,
  &s_THUSTR_3,
  &s_THUSTR_4,
  &s_THUSTR_5,
  &s_THUSTR_6,
  &s_THUSTR_7,
  &s_THUSTR_8,
  &s_THUSTR_9,
  &s_THUSTR_10,
  &s_THUSTR_11,

  &s_THUSTR_12,
  &s_THUSTR_13,
  &s_THUSTR_14,
  &s_THUSTR_15,
  &s_THUSTR_16,
  &s_THUSTR_17,
  &s_THUSTR_18,
  &s_THUSTR_19,
  &s_THUSTR_20,

  &s_THUSTR_21,
  &s_THUSTR_22,
  &s_THUSTR_23,
  &s_THUSTR_24,
  &s_THUSTR_25,
  &s_THUSTR_26,
  &s_THUSTR_27,
  &s_THUSTR_28,
  &s_THUSTR_29,
  &s_THUSTR_30,
  &s_THUSTR_31,
  &s_THUSTR_32,
};

// Function prototypes
void    lfstrip(char *);     // strip the \r and/or \n off of a line
void    rstrip(char *);      // strip trailing whitespace
char *  ptr_lstrip(char *);  // point past leading whitespace
dboolean deh_GetData(char *, char *, uint64_t *, char **);
dboolean deh_procStringSub(char *, char *, char *);
char *  dehReformatStr(char *);

// Prototypes for block processing functions
// Pointers to these functions are used as the blocks are encountered.

static void deh_procThing(DEHFILE *fpin, char *line);
static void deh_procFrame(DEHFILE *, char *);
static void deh_procPointer(DEHFILE *, char *);
static void deh_procSounds(DEHFILE *, char *);
static void deh_procAmmo(DEHFILE *, char *);
static void deh_procWeapon(DEHFILE *, char *);
static void deh_procSprite(DEHFILE *, char *);
static void deh_procCheat(DEHFILE *, char *);
static void deh_procMisc(DEHFILE *, char *);
static void deh_procText(DEHFILE *, char *);
static void deh_procPars(DEHFILE *, char *);
static void deh_procStrings(DEHFILE *, char *);
static void deh_procError(DEHFILE *, char *);
static void deh_procBexCodePointers(DEHFILE *, char *);
static void deh_procHelperThing(DEHFILE *, char *); // haleyjd 9/22/99
// haleyjd: handlers to fully deprecate the DeHackEd text section
static void deh_procBexSounds(DEHFILE *, char *);
static void deh_procBexMusic(DEHFILE *, char *);
static void deh_procBexSprites(DEHFILE *, char *);

// Structure deh_block is used to hold the block names that can
// be encountered, and the routines to use to decipher them

typedef struct
{
  const char *key;       // a mnemonic block code name // CPhipps - const*
  void (*const fptr)(DEHFILE *, char *); // handler
} deh_block;

#define DEH_BUFFERMAX 1024 // input buffer area size, hardcodedfor now
// killough 8/9/98: make DEH_BLOCKMAX self-adjusting
#define DEH_BLOCKMAX (sizeof(deh_blocks) / sizeof(*deh_blocks))  // size of array
#define DEH_MAXKEYLEN 32 // as much of any key as we'll look at

// Put all the block header values, and the function to be called when that
// one is encountered, in this array:
static const deh_block deh_blocks[] = { // CPhipps - static const
  /* 0 */  {"Thing",deh_procThing},
  /* 1 */  {"Frame",deh_procFrame},
  /* 2 */  {"Pointer",deh_procPointer},
  /* 3 */  {"Sound",deh_procSounds},  // Ty 03/16/98 corrected from "Sounds"
  /* 4 */  {"Ammo",deh_procAmmo},
  /* 5 */  {"Weapon",deh_procWeapon},
  /* 6 */  {"Sprite",deh_procSprite},
  /* 7 */  {"Cheat",deh_procCheat},
  /* 8 */  {"Misc",deh_procMisc},
  /* 9 */  {"Text",deh_procText},  // --  end of standard "deh" entries,

  //     begin BOOM Extensions (BEX)

  /* 10 */ {"[STRINGS]",deh_procStrings}, // new string changes
  /* 11 */ {"[PARS]",deh_procPars}, // alternative block marker
  /* 12 */ {"[CODEPTR]",deh_procBexCodePointers}, // bex codepointers by mnemonic
  /* 13 */ {"[HELPER]", deh_procHelperThing}, // helper thing substitution haleyjd 9/22/99
  /* 14 */ {"[SPRITES]", deh_procBexSprites}, // bex style sprites
  /* 15 */ {"[SOUNDS]", deh_procBexSounds},   // bex style sounds
  /* 16 */ {"[MUSIC]", deh_procBexMusic},     // bex style music
  /* 17 */ {"", deh_procError} // dummy to handle anything else
};

// flag to skip included deh-style text, used with INCLUDE NOTEXT directive
static dboolean includenotext = false;

// MOBJINFO - Dehacked block name = "Thing"
// Usage: Thing nn (name)
// These are for mobjinfo_t types.  Each is an integer
// within the structure, so we can use index of the string in this
// array to offset by sizeof(int) into the mobjinfo_t array at [nn]
// * things are base zero but dehacked considers them to start at #1. ***
// CPhipps - static const

static const char *deh_mobjinfo_fields[] =
{
  "ID #",                // .doomednum
  "Initial frame",       // .spawnstate
  "Hit points",          // .spawnhealth
  "First moving frame",  // .seestate
  "Alert sound",         // .seesound
  "Reaction time",       // .reactiontime
  "Attack sound",        // .attacksound
  "Injury frame",        // .painstate
  "Pain chance",         // .painchance
  "Pain sound",          // .painsound
  "Close attack frame",  // .meleestate
  "Far attack frame",    // .missilestate
  "Death frame",         // .deathstate
  "Exploding frame",     // .xdeathstate
  "Death sound",         // .deathsound
  "Speed",               // .speed
  "Width",               // .radius
  "Height",              // .height
  "Mass",                // .mass
  "Missile damage",      // .damage
  "Action sound",        // .activesound
  "Bits",                // .flags
  "Bits2",               // .flags
  "Respawn frame",       // .raisestate
  "Dropped item",        // .droppeditem

  // mbf21
  "Infighting group",    // .infighting_group
  "Projectile group",    // .projectile_group
  "Splash group",        // .splash_group
  "MBF21 Bits",          // .flags2
  "Rip sound",           // .ripsound
  "Fast speed",          // .altspeed
  "Melee range",         // .meleerange

  // misc
  "Blood color",         // .bloodcolor

  NULL
};

// Strings that are used to indicate flags ("Bits" in mobjinfo)
// This is an array of bit masks that are related to p_mobj.h
// values, using the smae names without the MF_ in front.
// Ty 08/27/98 new code
//
// killough 10/98:
//
// Convert array to struct to allow multiple values, make array size variable

struct deh_flag_s {
  const char *name; // CPhipps - const*
  uint64_t value;
};

static uint64_t deh_translate_bits(uint64_t value, const struct deh_flag_s *flags)
{
  int i;
  uint64_t result = 0;

  for (i = 0; flags[i].name; ++i)
  {
    if (value & 1)
      result |= flags[i].value;

    value >>= 1;
  }

  return result;
}

// CPhipps - static const
static const struct deh_flag_s deh_mobjflags[] = {
  {"SPECIAL",      MF_SPECIAL}, // call  P_Specialthing when touched
  {"SOLID",        MF_SOLID}, // block movement
  {"SHOOTABLE",    MF_SHOOTABLE}, // can be hit
  {"NOSECTOR",     MF_NOSECTOR}, // invisible but touchable
  {"NOBLOCKMAP",   MF_NOBLOCKMAP}, // inert but displayable
  {"AMBUSH",       MF_AMBUSH}, // deaf monster
  {"JUSTHIT",      MF_JUSTHIT}, // will try to attack right back
  {"JUSTATTACKED", MF_JUSTATTACKED}, // take at least 1 step before attacking
  {"SPAWNCEILING", MF_SPAWNCEILING}, // initially hang from ceiling
  {"NOGRAVITY",    MF_NOGRAVITY}, // don't apply gravity during play
  {"DROPOFF",      MF_DROPOFF}, // can jump from high places
  {"PICKUP",       MF_PICKUP}, // will pick up items
  {"NOCLIP",       MF_NOCLIP}, // goes through walls
  {"SLIDE",        MF_SLIDE}, // keep info about sliding along walls
  {"FLOAT",        MF_FLOAT}, // allow movement to any height
  {"TELEPORT",     MF_TELEPORT}, // don't cross lines or look at heights
  {"MISSILE",      MF_MISSILE}, // don't hit same species, explode on block
  {"DROPPED",      MF_DROPPED}, // dropped, not spawned (like ammo clip)
  {"SHADOW",       MF_SHADOW}, // use fuzzy draw like spectres
  {"NOBLOOD",      MF_NOBLOOD}, // puffs instead of blood when shot
  {"CORPSE",       MF_CORPSE}, // so it will slide down steps when dead
  {"INFLOAT",      MF_INFLOAT}, // float but not to target height
  {"COUNTKILL",    MF_COUNTKILL}, // count toward the kills total
  {"COUNTITEM",    MF_COUNTITEM}, // count toward the items total
  {"SKULLFLY",     MF_SKULLFLY}, // special handling for flying skulls
  {"NOTDMATCH",    MF_NOTDMATCH}, // do not spawn in deathmatch

  // killough 10/98: TRANSLATION consists of 2 bits, not 1:

  {"TRANSLATION",  MF_TRANSLATION1}, // for Boom bug-compatibility
  {"TRANSLATION1", MF_TRANSLATION1}, // use translation table for color (players)
  {"TRANSLATION2", MF_TRANSLATION2}, // use translation table for color (players)
  {"UNUSED1",      MF_TRANSLATION2}, // unused bit # 1 -- For Boom bug-compatibility
  {"UNUSED2",      MF_UNUSED2},      // unused bit # 2 -- For Boom compatibility
  {"UNUSED3",      MF_UNUSED3},      // unused bit # 3 -- For Boom compatibility
  {"UNUSED4",      MF_TRANSLUCENT},  // unused bit # 4 -- For Boom compatibility
  {"TRANSLUCENT",  MF_TRANSLUCENT},  // apply translucency to sprite (BOOM)
  {"TOUCHY",       MF_TOUCHY},       // dies on contact with solid objects (MBF)
  {"BOUNCES",      MF_BOUNCES},      // bounces off floors, ceilings and maybe walls (MBF)
  {"FRIEND",       MF_FRIEND},       // a friend of the player(s) (MBF)
  { NULL }
};

static const struct deh_flag_s deh_mobjflags_standard[] = {
  {"SPECIAL",      MF_SPECIAL}, // call  P_Specialthing when touched
  {"SOLID",        MF_SOLID}, // block movement
  {"SHOOTABLE",    MF_SHOOTABLE}, // can be hit
  {"NOSECTOR",     MF_NOSECTOR}, // invisible but touchable
  {"NOBLOCKMAP",   MF_NOBLOCKMAP}, // inert but displayable
  {"AMBUSH",       MF_AMBUSH}, // deaf monster
  {"JUSTHIT",      MF_JUSTHIT}, // will try to attack right back
  {"JUSTATTACKED", MF_JUSTATTACKED}, // take at least 1 step before attacking
  {"SPAWNCEILING", MF_SPAWNCEILING}, // initially hang from ceiling
  {"NOGRAVITY",    MF_NOGRAVITY}, // don't apply gravity during play
  {"DROPOFF",      MF_DROPOFF}, // can jump from high places
  {"PICKUP",       MF_PICKUP}, // will pick up items
  {"NOCLIP",       MF_NOCLIP}, // goes through walls
  {"SLIDE",        MF_SLIDE}, // keep info about sliding along walls
  {"FLOAT",        MF_FLOAT}, // allow movement to any height
  {"TELEPORT",     MF_TELEPORT}, // don't cross lines or look at heights
  {"MISSILE",      MF_MISSILE}, // don't hit same species, explode on block
  {"DROPPED",      MF_DROPPED}, // dropped, not spawned (like ammo clip)
  {"SHADOW",       MF_SHADOW}, // use fuzzy draw like spectres
  {"NOBLOOD",      MF_NOBLOOD}, // puffs instead of blood when shot
  {"CORPSE",       MF_CORPSE}, // so it will slide down steps when dead
  {"INFLOAT",      MF_INFLOAT}, // float but not to target height
  {"COUNTKILL",    MF_COUNTKILL}, // count toward the kills total
  {"COUNTITEM",    MF_COUNTITEM}, // count toward the items total
  {"SKULLFLY",     MF_SKULLFLY}, // special handling for flying skulls
  {"NOTDMATCH",    MF_NOTDMATCH}, // do not spawn in deathmatch
  {"TRANSLATION1", MF_TRANSLATION1}, // use translation table for color (players)
  {"TRANSLATION2", MF_TRANSLATION2}, // use translation table for color (players)
  {"TOUCHY",       MF_TOUCHY}, // dies on contact with solid objects (MBF)
  {"BOUNCES",      MF_BOUNCES}, // bounces off floors, ceilings and maybe walls (MBF)
  {"FRIEND",       MF_FRIEND}, // a friend of the player(s) (MBF)
  {"TRANSLUCENT",  MF_TRANSLUCENT}, // apply translucency to sprite (BOOM)
  { NULL }
};

static const struct deh_flag_s deh_mobjflags_mbf21[] = {
  {"LOGRAV",         MF2_LOGRAV}, // low gravity
  {"SHORTMRANGE",    MF2_SHORTMRANGE}, // short missile range
  {"DMGIGNORED",     MF2_DMGIGNORED}, // other things ignore its attacks
  {"NORADIUSDMG",    MF2_NORADIUSDMG}, // doesn't take splash damage
  {"FORCERADIUSDMG", MF2_FORCERADIUSDMG}, // causes splash damage even if target immune
  {"HIGHERMPROB",    MF2_HIGHERMPROB}, // higher missile attack probability
  {"RANGEHALF",      MF2_RANGEHALF}, // use half distance for missile attack probability
  {"NOTHRESHOLD",    MF2_NOTHRESHOLD}, // no targeting threshold
  {"LONGMELEE",      MF2_LONGMELEE}, // long melee range
  {"BOSS",           MF2_BOSS}, // full volume see / death sound + splash immunity
  {"MAP07BOSS1",     MF2_MAP07BOSS1}, // Tag 666 "boss" on doom 2 map 7
  {"MAP07BOSS2",     MF2_MAP07BOSS2}, // Tag 667 "boss" on doom 2 map 7
  {"E1M8BOSS",       MF2_E1M8BOSS}, // E1M8 boss
  {"E2M8BOSS",       MF2_E2M8BOSS}, // E2M8 boss
  {"E3M8BOSS",       MF2_E3M8BOSS}, // E3M8 boss
  {"E4M6BOSS",       MF2_E4M6BOSS}, // E4M6 boss
  {"E4M8BOSS",       MF2_E4M8BOSS}, // E4M8 boss
  {"RIP",            MF2_RIP}, // projectile rips through targets
  {"FULLVOLSOUNDS",  MF2_FULLVOLSOUNDS}, // full volume see / death sound
  { NULL }
};

static const struct deh_flag_s deh_weaponflags_mbf21[] = {
  { "NOTHRUST",       WPF_NOTHRUST }, // doesn't thrust Mobj's
  { "SILENT",         WPF_SILENT }, // weapon is silent
  { "NOAUTOFIRE",     WPF_NOAUTOFIRE }, // weapon won't autofire in A_WeaponReady
  { "FLEEMELEE",      WPF_FLEEMELEE }, // monsters consider it a melee weapon
  { "AUTOSWITCHFROM", WPF_AUTOSWITCHFROM }, // can be switched away from when ammo is picked up
  { "NOAUTOSWITCHTO", WPF_NOAUTOSWITCHTO }, // cannot be switched to when ammo is picked up
  { NULL }
};

// STATE - Dehacked block name = "Frame" and "Pointer"
// Usage: Frame nn
// Usage: Pointer nn (Frame nn)
// These are indexed separately, for lookup to the actual
// function pointers.  Here we'll take whatever Dehacked gives
// us and go from there.  The (Frame nn) after the pointer is the
// real place to put this value.  The "Pointer" value is an xref
// that Dehacked uses and is useless to us.
// * states are base zero and have a dummy #0 (TROO)

static const char *deh_state_fields[] = // CPhipps - static const*
{
  "Sprite number",    // .sprite (spritenum_t) // an enum
  "Sprite subnumber", // .frame (long)
  "Duration",         // .tics (long)
  "Next frame",       // .nextstate (statenum_t)
  // This is set in a separate "Pointer" block from Dehacked
  "Codep Frame",      // pointer to first use of action (actionf_t)
  "Unknown 1",        // .misc1 (long)
  "Unknown 2",        // .misc2 (long)
  "Args1",            // .args[0] (statearg_t)
  "Args2",            // .args[1] (statearg_t)
  "Args3",            // .args[2] (statearg_t)
  "Args4",            // .args[3] (statearg_t)
  "Args5",            // .args[4] (statearg_t)
  "Args6",            // .args[5] (statearg_t)
  "Args7",            // .args[6] (statearg_t)
  "Args8",            // .args[7] (statearg_t)
  "MBF21 Bits",       // .flags
};

static const struct deh_flag_s deh_stateflags_mbf21[] = {
  { "SKILL5FAST", STATEF_SKILL5FAST }, // tics halve on nightmare skill
  { NULL }
};

// AMMO - Dehacked block name = "Ammo"
// usage = Ammo n (name)
// Ammo information for the few types of ammo

static const char *deh_ammo[] = // CPhipps - static const*
{
  "Max ammo",   // maxammo[]
  "Per ammo"    // clipammo[]
};

// WEAPONS - Dehacked block name = "Weapon"
// Usage: Weapon nn (name)
// Basically a list of frames and what kind of ammo (see above)it uses.

static const char *deh_weapon[] = // CPhipps - static const*
{
  "Ammo type",      // .ammo
  "Deselect frame", // .upstate
  "Select frame",   // .downstate
  "Bobbing frame",  // .readystate
  "Shooting frame", // .atkstate
  "Firing frame",   // .flashstate
  "Ammo per shot",  // .ammopershot [XA] new to mbf21
  "MBF21 Bits",     // .flags
};

// CHEATS - Dehacked block name = "Cheat"
// Usage: Cheat 0
// Always uses a zero in the dehacked file, for consistency.  No meaning.
// These are just plain funky terms compared with id's
//
// killough 4/18/98: integrated into main cheat table now (see st_stuff.c)

// MISC - Dehacked block name = "Misc"
// Usage: Misc 0
// Always uses a zero in the dehacked file, for consistency.  No meaning.

static const char *deh_misc[] = // CPhipps - static const*
{
  "Initial Health",    // initial_health
  "Initial Bullets",   // initial_bullets
  "Max Health",        // maxhealth
  "Max Armor",         // max_armor
  "Green Armor Class", // green_armor_class
  "Blue Armor Class",  // blue_armor_class
  "Max Soulsphere",    // max_soul
  "Soulsphere Health", // soul_health
  "Megasphere Health", // mega_health
  "God Mode Health",   // god_health
  "IDFA Armor",        // idfa_armor
  "IDFA Armor Class",  // idfa_armor_class
  "IDKFA Armor",       // idkfa_armor
  "IDKFA Armor Class", // idkfa_armor_class
  "BFG Cells/Shot",    // BFGCELLS
  "Monsters Infight"   // Unknown--not a specific number it seems, but
  // the logic has to be here somewhere or
  // it'd happen always
};

// TEXT - Dehacked block name = "Text"
// Usage: Text fromlen tolen
// Dehacked allows a bit of adjustment to the length (why?)

// BEX extension [CODEPTR]
// Usage: Start block, then each line is:
// FRAME nnn = PointerMnemonic

typedef struct {
  actionf_t cptr;  // actual pointer to the subroutine
  const char *lookup;  // mnemonic lookup string to be specified in BEX
  // CPhipps - const*
  short argcount;  // [XA] number of mbf21 args this action uses, if any
  long default_args[MAXSTATEARGS]; // default values for mbf21 args
  short ti_flags; // thing index on these args
} deh_bexptr;

#define TI_MISC1 0x0001
#define TI_MISC2 0x0002
#define TI_ARGSSHIFT 2
#define TI_ARGS1 0x0004
#define TI_ARGS2 0x0008
#define TI_ARGS3 0x0010
#define TI_ARGS4 0x0020
#define TI_ARGS5 0x0040
#define TI_ARGS6 0x0080
#define TI_ARGS7 0x0100
#define TI_ARGS8 0x0200

static const deh_bexptr deh_bexptrs[] = // CPhipps - static const
{
  {A_Light0,          "A_Light0"},
  {A_WeaponReady,     "A_WeaponReady"},
  {A_Lower,           "A_Lower"},
  {A_Raise,           "A_Raise"},
  {A_Punch,           "A_Punch"},
  {A_ReFire,          "A_ReFire"},
  {A_FirePistol,      "A_FirePistol"},
  {A_Light1,          "A_Light1"},
  {A_FireShotgun,     "A_FireShotgun"},
  {A_Light2,          "A_Light2"},
  {A_FireShotgun2,    "A_FireShotgun2"},
  {A_CheckReload,     "A_CheckReload"},
  {A_OpenShotgun2,    "A_OpenShotgun2"},
  {A_LoadShotgun2,    "A_LoadShotgun2"},
  {A_CloseShotgun2,   "A_CloseShotgun2"},
  {A_FireCGun,        "A_FireCGun"},
  {A_GunFlash,        "A_GunFlash"},
  {A_FireMissile,     "A_FireMissile"},
  {A_Saw,             "A_Saw"},
  {A_FirePlasma,      "A_FirePlasma"},
  {A_BFGsound,        "A_BFGsound"},
  {A_FireBFG,         "A_FireBFG"},
  {A_BFGSpray,        "A_BFGSpray"},
  {A_Explode,         "A_Explode"},
  {A_Pain,            "A_Pain"},
  {A_PlayerScream,    "A_PlayerScream"},
  {A_Fall,            "A_Fall"},
  {A_XScream,         "A_XScream"},
  {A_Look,            "A_Look"},
  {A_Chase,           "A_Chase"},
  {A_FaceTarget,      "A_FaceTarget"},
  {A_PosAttack,       "A_PosAttack"},
  {A_Scream,          "A_Scream"},
  {A_SPosAttack,      "A_SPosAttack"},
  {A_VileChase,       "A_VileChase"},
  {A_VileStart,       "A_VileStart"},
  {A_VileTarget,      "A_VileTarget"},
  {A_VileAttack,      "A_VileAttack"},
  {A_StartFire,       "A_StartFire"},
  {A_Fire,            "A_Fire"},
  {A_FireCrackle,     "A_FireCrackle"},
  {A_Tracer,          "A_Tracer"},
  {A_SkelWhoosh,      "A_SkelWhoosh"},
  {A_SkelFist,        "A_SkelFist"},
  {A_SkelMissile,     "A_SkelMissile"},
  {A_FatRaise,        "A_FatRaise"},
  {A_FatAttack1,      "A_FatAttack1"},
  {A_FatAttack2,      "A_FatAttack2"},
  {A_FatAttack3,      "A_FatAttack3"},
  {A_BossDeath,       "A_BossDeath"},
  {A_CPosAttack,      "A_CPosAttack"},
  {A_CPosRefire,      "A_CPosRefire"},
  {A_TroopAttack,     "A_TroopAttack"},
  {A_SargAttack,      "A_SargAttack"},
  {A_HeadAttack,      "A_HeadAttack"},
  {A_BruisAttack,     "A_BruisAttack"},
  {A_SkullAttack,     "A_SkullAttack"},
  {A_Metal,           "A_Metal"},
  {A_SpidRefire,      "A_SpidRefire"},
  {A_BabyMetal,       "A_BabyMetal"},
  {A_BspiAttack,      "A_BspiAttack"},
  {A_Hoof,            "A_Hoof"},
  {A_CyberAttack,     "A_CyberAttack"},
  {A_PainAttack,      "A_PainAttack"},
  {A_PainDie,         "A_PainDie"},
  {A_KeenDie,         "A_KeenDie"},
  {A_BrainPain,       "A_BrainPain"},
  {A_BrainScream,     "A_BrainScream"},
  {A_BrainDie,        "A_BrainDie"},
  {A_BrainAwake,      "A_BrainAwake"},
  {A_BrainSpit,       "A_BrainSpit"},
  {A_SpawnSound,      "A_SpawnSound"},
  {A_SpawnFly,        "A_SpawnFly"},
  {A_BrainExplode,    "A_BrainExplode"},
  {A_Detonate,        "A_Detonate"},       // killough 8/9/98
  {A_Mushroom,        "A_Mushroom"},       // killough 10/98
  {A_Die,             "A_Die"},            // killough 11/98
  {A_Spawn,           "A_Spawn", 0, {0}, TI_MISC1},          // killough 11/98
  {A_Turn,            "A_Turn"},           // killough 11/98
  {A_Face,            "A_Face"},           // killough 11/98
  {A_Scratch,         "A_Scratch"},        // killough 11/98
  {A_PlaySound,       "A_PlaySound"},      // killough 11/98
  {A_RandomJump,      "A_RandomJump"},     // killough 11/98
  {A_LineEffect,      "A_LineEffect"},     // killough 11/98

  {A_FireOldBFG,      "A_FireOldBFG"},      // killough 7/19/98: classic BFG firing function
  {A_BetaSkullAttack, "A_BetaSkullAttack"}, // killough 10/98: beta lost souls attacked different
  {A_Stop,            "A_Stop"},

  // [XA] New mbf21 codepointers
  {A_SpawnObject,         "A_SpawnObject", 8, {0}, TI_ARGS1},
  {A_MonsterProjectile,   "A_MonsterProjectile", 5, {0}, TI_ARGS1},
  {A_MonsterBulletAttack, "A_MonsterBulletAttack", 5, {0, 0, 1, 3, 5}},
  {A_MonsterMeleeAttack,  "A_MonsterMeleeAttack", 4, {3, 8, 0, 0}},
  {A_RadiusDamage,        "A_RadiusDamage", 2},
  {A_NoiseAlert,          "A_NoiseAlert", 0},
  {A_HealChase,           "A_HealChase", 2},
  {A_SeekTracer,          "A_SeekTracer", 2},
  {A_FindTracer,          "A_FindTracer", 2, {0, 10}},
  {A_ClearTracer,         "A_ClearTracer", 0},
  {A_JumpIfHealthBelow,   "A_JumpIfHealthBelow", 2},
  {A_JumpIfTargetInSight, "A_JumpIfTargetInSight", 2},
  {A_JumpIfTargetCloser,  "A_JumpIfTargetCloser", 2},
  {A_JumpIfTracerInSight, "A_JumpIfTracerInSight", 2},
  {A_JumpIfTracerCloser,  "A_JumpIfTracerCloser", 2},
  {A_JumpIfFlagsSet,      "A_JumpIfFlagsSet", 3},
  {A_AddFlags,            "A_AddFlags", 2},
  {A_RemoveFlags,         "A_RemoveFlags", 2},
  {A_WeaponProjectile,    "A_WeaponProjectile", 5, {0}, TI_ARGS1},
  {A_WeaponBulletAttack,  "A_WeaponBulletAttack", 5, {0, 0, 1, 5, 3}},
  {A_WeaponMeleeAttack,   "A_WeaponMeleeAttack", 5, {2, 10, 1 * FRACUNIT, 0, 0}},
  {A_WeaponSound,         "A_WeaponSound", 2},
  {A_WeaponAlert,         "A_WeaponAlert", 0},
  {A_WeaponJump,          "A_WeaponJump", 2},
  {A_ConsumeAmmo,         "A_ConsumeAmmo", 1},
  {A_CheckAmmo,           "A_CheckAmmo", 2},
  {A_RefireTo,            "A_RefireTo", 2},
  {A_GunFlashTo,          "A_GunFlashTo", 2},

  // This NULL entry must be the last in the list
  {NULL,              "A_NULL"},  // Ty 05/16/98
};

int deh_maxhealth;
int deh_max_soul;
int deh_mega_health;

dboolean IsDehMaxHealth = false;
dboolean IsDehMaxSoul = false;
dboolean IsDehMegaHealth = false;

static uint64_t deh_stringToFlags(char *strval, const struct deh_flag_s *flags)
{
  uint64_t value;

  for (value = 0; (strval = strtok(strval, deh_getBitsDelims())); strval = NULL) {
    const struct deh_flag_s *flag;

    for (flag = flags; flag->name; flag++) {
      if (deh_strcasecmp(strval, flag->name)) continue;

      value |= flag->value;
      break;
    }

    if (!flag->name)
      deh_log("Could not find MBF21 bit mnemonic %s\n", strval);
  }

  return value;
}

uint64_t deh_stringToMBF21MobjFlags(char *strval)
{
  return deh_stringToFlags(strval, deh_mobjflags_mbf21);
}

uint64_t deh_stringToMobjFlags(char *strval)
{
  return deh_stringToFlags(strval, deh_mobjflags);
}

void deh_changeCompTranslucency(void)
{
  extern byte* edited_mobjinfo_bits;
  int i;
  int boom_translucent_sprites;
  int predefined_translucency[] = {
    MT_FIRE, MT_SMOKE, MT_FATSHOT, MT_BRUISERSHOT, MT_SPAWNFIRE,
    MT_TROOPSHOT, MT_HEADSHOT, MT_PLASMA, MT_BFG, MT_ARACHPLAZ, MT_PUFF,
    MT_TFOG, MT_IFOG, MT_MISC12, MT_INV, MT_INS, MT_MEGA
  };

  if (raven) return;

  boom_translucent_sprites = dsda_IntConfig(dsda_config_boom_translucent_sprites);

  for (i = 0; (size_t)i < sizeof(predefined_translucency) / sizeof(predefined_translucency[0]); i++)
  {
    if (!edited_mobjinfo_bits[predefined_translucency[i]])
    {
      if (comp[comp_translucency] || !boom_translucent_sprites)
        mobjinfo[predefined_translucency[i]].flags &= ~MF_TRANSLUCENT;
      else
        mobjinfo[predefined_translucency[i]].flags |= MF_TRANSLUCENT;
    }
  }
}

void deh_applyCompatibility(void)
{
  extern byte* edited_mobjinfo_bits;
  int comp_max = (compatibility_level == doom_12_compatibility ? 199 : 200);

  max_soul = (IsDehMaxSoul ? deh_max_soul : comp_max);
  mega_health = (IsDehMegaHealth ? deh_mega_health : comp_max);

  if (comp[comp_maxhealth])
  {
    maxhealth = 100;
    maxhealthbonus = (IsDehMaxHealth ? deh_maxhealth : comp_max);
  }
  else
  {
    maxhealth = (IsDehMaxHealth ? deh_maxhealth : 100);
    maxhealthbonus = maxhealth * 2;
  }

  if (raven) return;

  if (!edited_mobjinfo_bits[MT_SKULL])
  {
    if (compatibility_level == doom_12_compatibility)
      mobjinfo[MT_SKULL].flags |= (MF_COUNTKILL);
    else
      mobjinfo[MT_SKULL].flags &= ~(MF_COUNTKILL);
  }

  deh_changeCompTranslucency();
}

// ====================================================================
// ProcessDehFile
// Purpose: Read and process a DEH or BEX file
// Args:    filename    -- name of the DEH/BEX file
//          outfilename -- output file (DEHOUT.TXT), appended to here
// Returns: void
//
// killough 10/98:
// substantially modified to allow input from wad lumps instead of .deh files.

static int processed_dehacked;

void ProcessDehFile(const char *filename, const char *outfilename, int lumpnum)
{
  DEHFILE infile, *filein = &infile;    // killough 10/98
  char inbuffer[DEH_BUFFERMAX];  // Place to put the primary infostring
  const char *file_or_lump;
  static unsigned last_block;
  static long filepos;

  processed_dehacked = true;

  // Open output file if we're writing output
  if (outfilename && *outfilename && !deh_log_file)
  {
    static dboolean firstfile = true; // to allow append to output log
    if (!strcmp(outfilename, "-"))
      deh_log_file = NULL;
    else if (!(deh_log_file = M_OpenFile(outfilename, firstfile ? "wt" : "at")))
    {
      //lprintf(LO_WARN, "Could not open -dehout file %s\n... using stdout.\n", outfilename);
      deh_log_file = NULL;
    }
    firstfile = false;
  }

  // killough 10/98: allow DEH files to come from wad lumps

  if (filename)
  {
    if (!(infile.f = M_OpenFile(filename, "rt")))
    {
      lprintf(LO_WARN, "-deh file %s not found\n", filename);
      return;  // should be checked up front anyway
    }
    infile.lump = NULL;
    file_or_lump = "file";
  }
  else  // DEH file comes from lump indicated by third argument
  {
    infile.size = W_LumpLength(lumpnum);
    infile.inp = infile.lump = W_LumpByNum(lumpnum);
    // [FG] skip empty DEHACKED lumps
    if (!infile.inp)
    {
      lprintf(LO_WARN, "skipping empty DEHACKED (%d) lump\n", lumpnum);
      return;
    }
    filename = lumpinfo[lumpnum].wadfile->name;
    file_or_lump = "lump from";
  }

  lprintf(LO_INFO, "Loading DEH %s %s\n", file_or_lump, filename);
  deh_log("\nLoading DEH %s %s\n\n", file_or_lump, filename);

  // loop until end of file

  last_block = DEH_BLOCKMAX - 1;
  filepos = 0;
  while (dehfgets(inbuffer,sizeof(inbuffer),filein))
  {
    dboolean match;
    unsigned i;

    lfstrip(inbuffer);
    deh_log("Line='%s'\n", inbuffer);
    if (!*inbuffer || *inbuffer == '#' || *inbuffer == ' ')
      continue; /* Blank line or comment line */

    // -- If DEH_BLOCKMAX is set right, the processing is independently
    // -- handled based on data in the deh_blocks[] structure array

    // killough 10/98: INCLUDE code rewritten to allow arbitrary nesting,
    // and to greatly simplify code, fix memory leaks, other bugs

    if (!strnicmp(inbuffer, "INCLUDE", 7)) // include a file
    {
      // preserve state while including a file
      // killough 10/98: moved to here

      char *nextfile;
      dboolean oldnotext = includenotext;       // killough 10/98

      // killough 10/98: exclude if inside wads (only to discourage
      // the practice, since the code could otherwise handle it)

      if (infile.lump)
      {
        deh_log("No files may be included from wads: %s\n", inbuffer);
        continue;
      }

      // check for no-text directive, used when including a DEH
      // file but using the BEX format to handle strings

      if (!strnicmp(nextfile = ptr_lstrip(inbuffer + 7), "NOTEXT", 6))
        includenotext = true, nextfile = ptr_lstrip(nextfile + 6);

      deh_log("Branching to include file %s...\n", nextfile);

      // killough 10/98:
      // Second argument must be NULL to prevent closing deh_log_file too soon

      ProcessDehFile(nextfile, NULL, 0); // do the included file

      includenotext = oldnotext;
      deh_log("...continuing with %s\n", filename);
      continue;
    }

    for (match = 0, i = 0; i < DEH_BLOCKMAX - 1; i++)
      if (!strncasecmp(inbuffer, deh_blocks[i].key, strlen(deh_blocks[i].key)))
      { // matches one
        match = 1;
        break;  // we got one, that's enough for this block
      }

    if (match) // inbuffer matches a valid block code name
      last_block = i;
    else if (last_block >= 10 && last_block < DEH_BLOCKMAX - 1) // restrict to BEX style lumps
    { // process that same line again with the last valid block code handler
      i = last_block;
      dehfseek(filein, filepos);
    }

    deh_log("Processing function [%d] for %s\n", i, deh_blocks[i].key);
    deh_blocks[i].fptr(filein, inbuffer);  // call function

    filepos = dehftell(filein);
  }

  if (!infile.lump)
    DG_close(infile.f);                         // Close real file

  if (outfilename)   // killough 10/98: only at top recursion level
  {
    if (deh_log_file)
      DG_close(deh_log_file);
    deh_log_file = NULL;
  }

  deh_applyCompatibility();
}

// ====================================================================
// deh_procBexCodePointers
// Purpose: Handle [CODEPTR] block, BOOM Extension
// Args:    fpin  -- input file stream
//          line  -- current line in file to process
// Returns: void
//
static void deh_procBexCodePointers(DEHFILE *fpin, char *line)
{
  char key[DEH_MAXKEYLEN];
  char inbuffer[DEH_BUFFERMAX];
  int indexnum;
  char mnemonic[DEH_MAXKEYLEN];  // to hold the codepointer mnemonic
  int i; // looper
  dboolean found; // know if we found this one during lookup or not
  dsda_deh_state_t deh_state;

  // Ty 05/16/98 - initialize it to something, dummy!
  strncpy(inbuffer, line, DEH_BUFFERMAX - 1);

  // for this one, we just read 'em until we hit a blank line
  while (!dehfeof(fpin) && *inbuffer && (*inbuffer != ' '))
  {
    if (!dehfgets(inbuffer, sizeof(inbuffer), fpin)) break;
    lfstrip(inbuffer);
    if (!*inbuffer) break;   // killough 11/98: really exit on blank line

    // killough 8/98: allow hex numbers in input:
    if ((3 != sscanf(inbuffer, "%s %i = %s", key, &indexnum, mnemonic)) || (stricmp(key, "FRAME")))
    { // NOTE: different format from normal
      deh_log("Invalid BEX codepointer line - must start with 'FRAME': '%s'\n", inbuffer);
      return;  // early return
    }

    deh_log("Processing pointer at index %d: %s\n", indexnum, mnemonic);
    if (indexnum < 0)
    {
      deh_log("Pointer number must be positive (%d)\n", indexnum);
      return; // killough 10/98: fix SegViol
    }
    strcpy(key, "A_");  // reusing the key area to prefix the mnemonic
    strcat(key, ptr_lstrip(mnemonic));

    deh_state = dsda_GetDehState(indexnum);

    found = FALSE;
    i= -1; // incremented to start at zero at the top of the loop
    do  // Ty 05/16/98 - fix loop logic to look for null ending entry
    {
      ++i;
      if (!stricmp(key, deh_bexptrs[i].lookup))
      {  // Ty 06/01/98  - add  to states[].action for new djgcc version
        deh_state.state->action = deh_bexptrs[i].cptr; // assign
        deh_log(" - applied %s from codeptr[%d] to states[%d]\n",
                deh_bexptrs[i].lookup, i, indexnum);
        found = TRUE;
      }
    } while (!found && (deh_bexptrs[i].cptr != NULL));

    if (!found)
      deh_log("Invalid frame pointer mnemonic '%s' at %d\n", mnemonic, indexnum);
  }
}

//---------------------------------------------------------------------------
// To be on the safe, compatible side, we manually convert DEH bitflags
// to prboom types - POPE
//---------------------------------------------------------------------------
static uint64_t getConvertedDEHBits(uint64_t bits) {
  static const uint64_t bitMap[32] = {
    /* cf linuxdoom-1.10 p_mobj.h */
    MF_SPECIAL, // 0 Can be picked up - When touched the thing can be picked up.
    MF_SOLID, // 1 Obstacle - The thing is solid and will not let you (or others) pass through it
    MF_SHOOTABLE, // 2 Shootable - Can be shot.
    MF_NOSECTOR, // 3 Total Invisibility - Invisible, but can be touched
    MF_NOBLOCKMAP, // 4 Don't use the blocklinks (inert but displayable)
    MF_AMBUSH, // 5 Semi deaf - The thing is a deaf monster
    MF_JUSTHIT, // 6 In pain - Will try to attack right back after being hit
    MF_JUSTATTACKED, // 7 Steps before attack - Will take at least one step before attacking
    MF_SPAWNCEILING, // 8 Hangs from ceiling - When the level starts, this thing will be at ceiling height.
    MF_NOGRAVITY, // 9 No gravity - Gravity does not affect this thing
    MF_DROPOFF, // 10 Travels over cliffs - Monsters normally do not walk off ledges/steps they could not walk up. With this set they can walk off any height of cliff. Usually only used for flying monsters.
    MF_PICKUP, // 11 Pick up items - The thing can pick up gettable items.
    MF_NOCLIP, // 12 No clipping - Thing can walk through walls.
    MF_SLIDE, // 13 Slides along walls - Keep info about sliding along walls (don't really know much about this one).
    MF_FLOAT, // 14 Floating - Thing can move to any height
    MF_TELEPORT, // 15 Semi no clipping - Don't cross lines or look at teleport heights. (don't really know much about this one either).
    MF_MISSILE, // 16 Projectiles - Behaves like a projectile, explodes when hitting something that blocks movement
    MF_DROPPED, // 17 Disappearing weapon - Dropped, not spawned (like an ammo clip) I have not had much success in using this one.
    MF_SHADOW, // 18 Partial invisibility - Drawn like a spectre.
    MF_NOBLOOD, // 19 Puffs (vs. bleeds) - If hit will spawn bullet puffs instead of blood splats.
    MF_CORPSE, // 20 Sliding helpless - Will slide down steps when dead.
    MF_INFLOAT, // 21 No auto levelling - float but not to target height (?)
    MF_COUNTKILL, // 22 Affects kill % - counted as a killable enemy and affects percentage kills on level summary.
    MF_COUNTITEM, // 23 Affects item % - affects percentage items gathered on level summary.
    MF_SKULLFLY, // 24 Running - special handling for flying skulls.
    MF_NOTDMATCH, // 25 Not in deathmatch - do not spawn in deathmatch (like keys)
    MF_TRANSLATION1, // 26 Color 1 (grey / red)
    MF_TRANSLATION2, // 27 Color 2 (brown / red)
    // Convert bit 28 to MF_TOUCHY, not (MF_TRANSLATION1|MF_TRANSLATION2)
    // fixes bug #1576151 (part 1)
    MF_TOUCHY, // 28 - explodes on contact (MBF)
    MF_BOUNCES, // 29 - bounces off walls and floors (MBF)
    MF_FRIEND, // 30 - friendly monster helps players (MBF)
    MF_TRANSLUCENT // e6y: Translucency via dehacked/bex doesn't work without it
  };
  int i;
  uint64_t shiftBits = bits;
  uint64_t convertedBits = 0;
  for (i=0; i<32; i++) {
    if (shiftBits & 0x1) convertedBits |= bitMap[i];
    shiftBits >>= 1;
  }
  return convertedBits;
}

//---------------------------------------------------------------------------
// See usage below for an explanation of this function's existence - POPE
//---------------------------------------------------------------------------
static void setMobjInfoValue(int mobjInfoIndex, int keyIndex, uint64_t value) {
  mobjinfo_t *mi;
  if (mobjInfoIndex >= num_mobj_types || mobjInfoIndex < 0) return;
  mi = &mobjinfo[mobjInfoIndex];
  switch (keyIndex) {
    case 0: mi->doomednum = (int)value; return;
    case 1: mi->spawnstate = (int)value; return;
    case 2: mi->spawnhealth = (int)value; return;
    case 3: mi->seestate = (int)value; return;
    case 4: mi->seesound = (int)value; return;
    case 5: mi->reactiontime = (int)value; return;
    case 6: mi->attacksound = (int)value; return;
    case 7: mi->painstate = (int)value; return;
    case 8: mi->painchance = (int)value; return;
    case 9: mi->painsound = (int)value; return;
    case 10: mi->meleestate = (int)value; return;
    case 11: mi->missilestate = (int)value; return;
    case 12: mi->deathstate = (int)value; return;
    case 13: mi->xdeathstate = (int)value; return;
    case 14: mi->deathsound = (int)value; return;
    case 15: mi->speed = (int)value; return;
    case 16: mi->radius = (int)value; return;
    case 17: mi->height = (int)value; return;
    case 18: mi->mass = (int)value; return;
    case 19: mi->damage = (int)value; return;
    case 20: mi->activesound = (int)value; return;
    case 21: mi->flags = value; return;
    // e6y
    // Correction of wrong processing of "Respawn frame" entry.
    // There is no more synch on http://www.doomworld.com/sda/dwdemo/w303-115.zip
    // (with correction in PIT_CheckThing)
    case 22:
      if (prboom_comp[PC_FORCE_INCORRECT_PROCESSING_OF_RESPAWN_FRAME_ENTRY].state)
      {
        mi->raisestate = (int)value;
        return;
      }
      break;
    case 23:
      if (!prboom_comp[PC_FORCE_INCORRECT_PROCESSING_OF_RESPAWN_FRAME_ENTRY].state)
      {
        mi->raisestate = (int)value;
        return;
      }
      break;
    case 24: // make it base zero (deh is 1-based)
      mi->droppeditem = dsda_TranslateDehMobjIndex((int)value) - 1;
      return;

    // mbf21
    // custom groups count from the end of the vanilla list
    // -> no concern for clashes
    case 25:
      mi->infighting_group = (int)(value);
      if (mi->infighting_group < 0)
      {
        I_Error("Infighting groups must be >= 0 (check your dehacked)");
        return;
      }
      mi->infighting_group = mi->infighting_group + IG_END;
      return;
    case 26:
      mi->projectile_group = (int)(value);
      if (mi->projectile_group < 0)
        mi->projectile_group = PG_GROUPLESS;
      else
        mi->projectile_group = mi->projectile_group + PG_END;
      return;
    case 27:
      mi->splash_group = (int)(value);
      if (mi->splash_group < 0)
      {
        I_Error("Splash groups must be >= 0 (check your dehacked)");
        return;
      }
      mi->splash_group = mi->splash_group + SG_END;
      return;
    case 29: mi->ripsound = (int)value; return;
    case 30: mi->altspeed = (int)value; return;
    case 31: mi->meleerange = (int)value; return;

    // misc
    case 32: mi->bloodcolor = V_BloodColor((int)value); return;

    default: return;
  }
}

// ====================================================================
// deh_procThing
// Purpose: Handle DEH Thing block
// Args:    fpin  -- input file stream
//          line  -- current line in file to process
// Returns: void
//
// Ty 8/27/98 - revised to also allow mnemonics for
// bit masks for monster attributes
//

static void deh_procThing(DEHFILE *fpin, char *line)
{
  char key[DEH_MAXKEYLEN];
  char inbuffer[DEH_BUFFERMAX];
  uint64_t value;      // All deh values are ints or longs
  int indexnum;
  int internal_index;
  int ix;
  char *strval;
  dsda_deh_mobjinfo_t deh_mobjinfo;

  strncpy(inbuffer, line, DEH_BUFFERMAX - 1);
  deh_log("Thing line: '%s'\n", inbuffer);

  // killough 8/98: allow hex numbers in input:
  ix = sscanf(inbuffer, "%s %i", key, &indexnum);
  deh_log("count=%d, Thing %d\n", ix, indexnum);

  // Note that the mobjinfo[] array is base zero, but object numbers
  // in the dehacked file start with one.  Grumble.
  internal_index = dsda_TranslateDehMobjIndex(indexnum) - 1;

  deh_mobjinfo = dsda_GetDehMobjInfo(internal_index);

  // now process the stuff
  // Note that for Things we can look up the key and use its offset
  // in the array of key strings as an int offset in the structure

  // get a line until a blank or end of file--it's not
  // blank now because it has our incoming key in it
  while (!dehfeof(fpin) && *inbuffer && (*inbuffer != ' '))
  {
    // e6y: Correction of wrong processing of Bits parameter if its value is equal to zero
    // No more desync on HACX demos.
    int bGetData;

    if (!dehfgets(inbuffer, sizeof(inbuffer), fpin)) break;
    lfstrip(inbuffer);  // toss the end of line

    // killough 11/98: really bail out on blank lines (break != continue)
    if (!*inbuffer) break;  // bail out with blank line between sections

    // e6y: Correction of wrong processing of Bits parameter if its value is equal to zero
    // No more desync on HACX demos.
    bGetData = deh_GetData(inbuffer, key, &value, &strval);
    if (!bGetData)
    {
      deh_log("Bad data pair in '%s'\n", inbuffer);
      continue;
    }

    for (ix = 0; deh_mobjinfo_fields[ix]; ix++) {
      if (deh_strcasecmp(key, deh_mobjinfo_fields[ix])) continue;

      if (!deh_strcasecmp(key, "MBF21 Bits")) {
        if (bGetData == 1)
        {
          value = deh_translate_bits(value, deh_mobjflags_mbf21);
        }
        else
        {
          value = deh_stringToMBF21MobjFlags(strval);
        }

        deh_mobjinfo.info->flags2 = value;
      }
      else if (deh_strcasecmp(key, "Bits")) {
        // standard value set

        // The old code here was the cause of a DEH-related bug in prboom.
        // When the mobjinfo_t.flags member was graduated to an int64, this
        // code was caught unawares and was indexing each property of the
        // mobjinfo as if it were still an int32. This caused sets of the
        // "raisestate" member to partially overwrite the "flags" member,
        // thus screwing everything up and making most DEH patches result in
        // unshootable enemy types. Moved to a separate function above
        // and stripped of all hairy struct address indexing. - POPE
        setMobjInfoValue(internal_index, ix, value);
      }
      else if (bGetData == 1)
      { // proff
        // bit set
        // e6y: Correction of wrong processing of Bits parameter if its value is equal to zero
        // No more desync on HACX demos.

        value = getConvertedDEHBits(value);
        deh_mobjinfo.info->flags = value;
        *deh_mobjinfo.edited_bits = true; //e6y: changed by DEH
      }
      else
      {
        value = deh_stringToMobjFlags(strval);

        // Don't worry about conversion -- simply print values
        deh_log("Bits = 0x%08lX%08lX\n",
                (unsigned long)(value>>32) & 0xffffffff,
                (unsigned long)value & 0xffffffff);
        deh_mobjinfo.info->flags = value; // e6y
        *deh_mobjinfo.edited_bits = true; //e6y: changed by DEH
      }

      deh_log("Assigned 0x%08lx%08lx to %s(%d) at index %d\n",
              (unsigned long)(value>>32) & 0xffffffff,
              (unsigned long)value & 0xffffffff, key, indexnum, ix);
    }
  }
}

// ====================================================================
// deh_procFrame
// Purpose: Handle DEH Frame block
// Args:    fpin  -- input file stream
//          line  -- current line in file to process
// Returns: void
//
static void deh_procFrame(DEHFILE *fpin, char *line)
{
  char key[DEH_MAXKEYLEN];
  char inbuffer[DEH_BUFFERMAX];
  uint64_t value;      // All deh values are ints or longs
  int indexnum;
  char *strval;
  int bGetData;
  dsda_deh_state_t deh_state;

  strncpy(inbuffer, line, DEH_BUFFERMAX - 1);

  // killough 8/98: allow hex numbers in input:
  sscanf(inbuffer, "%s %i", key, &indexnum);
  deh_log("Processing Frame at index %d: %s\n", indexnum, key);
  if (indexnum < 0)
  {
    deh_log("Frame number must be positive (%d)\n", indexnum);
    return;
  }

  deh_state = dsda_GetDehState(indexnum);

  while (!dehfeof(fpin) && *inbuffer && (*inbuffer != ' '))
  {
    if (!dehfgets(inbuffer, sizeof(inbuffer), fpin)) break;
    lfstrip(inbuffer);
    if (!*inbuffer) break;         // killough 11/98
    bGetData = deh_GetData(inbuffer, key, &value, &strval);
    if (!bGetData) // returns TRUE if ok
    {
      deh_log("Bad data pair in '%s'\n", inbuffer);
      continue;
    }

    if (!deh_strcasecmp(key, deh_state_fields[0]))  // Sprite number
    {
      deh_log(" - sprite = %ld\n", (long)value);
      deh_state.state->sprite = (spritenum_t)value;
    }
    else if (!deh_strcasecmp(key, deh_state_fields[1]))  // Sprite subnumber
    {
      deh_log(" - frame = %ld\n", (long)value);
      deh_state.state->frame = (long)value; // long
    }
    else if (!deh_strcasecmp(key, deh_state_fields[2]))  // Duration
    {
      deh_log(" - tics = %ld\n", (long)value);
      deh_state.state->tics = (long)value; // long
    }
    else if (!deh_strcasecmp(key, deh_state_fields[3]))  // Next frame
    {
      deh_log(" - nextstate = %ld\n", (long)value);
      deh_state.state->nextstate = (statenum_t)value;
    }
    else if (!deh_strcasecmp(key, deh_state_fields[4]))  // Codep frame (not set in Frame deh block)
    {
      deh_log(" - codep, should not be set in Frame section!\n");
      /* nop */ ;
    }
    else if (!deh_strcasecmp(key, deh_state_fields[5]))  // Unknown 1
    {
      deh_log(" - misc1 = %ld\n", (long)value);
      deh_state.state->misc1 = (long)value; // long
    }
    else if (!deh_strcasecmp(key, deh_state_fields[6]))  // Unknown 2
    {
      deh_log(" - misc2 = %ld\n", (long)value);
      deh_state.state->misc2 = (long)value; // long
    }
    else if (!deh_strcasecmp(key, deh_state_fields[7]))  // Args1
    {
      deh_log(" - args[0] = %lld\n", (statearg_t)value);
      deh_state.state->args[0] = (statearg_t)value;
      *deh_state.defined_codeptr_args |= (1 << 0);
    }
    else if (!deh_strcasecmp(key, deh_state_fields[8]))  // Args2
    {
      deh_log(" - args[1] = %lld\n", (statearg_t)value);
      deh_state.state->args[1] = (statearg_t)value;
      *deh_state.defined_codeptr_args |= (1 << 1);
    }
    else if (!deh_strcasecmp(key, deh_state_fields[9]))  // Args3
    {
      deh_log(" - args[2] = %lld\n", (statearg_t)value);
      deh_state.state->args[2] = (statearg_t)value;
      *deh_state.defined_codeptr_args |= (1 << 2);
    }
    else if (!deh_strcasecmp(key, deh_state_fields[10]))  // Args4
    {
      deh_log(" - args[3] = %lld\n", (statearg_t)value);
      deh_state.state->args[3] = (statearg_t)value;
      *deh_state.defined_codeptr_args |= (1 << 3);
    }
    else if (!deh_strcasecmp(key, deh_state_fields[11]))  // Args5
    {
      deh_log(" - args[4] = %lld\n", (statearg_t)value);
      deh_state.state->args[4] = (statearg_t)value;
      *deh_state.defined_codeptr_args |= (1 << 4);
    }
    else if (!deh_strcasecmp(key, deh_state_fields[12]))  // Args6
    {
      deh_log(" - args[5] = %lld\n", (statearg_t)value);
      deh_state.state->args[5] = (statearg_t)value;
      *deh_state.defined_codeptr_args |= (1 << 5);
    }
    else if (!deh_strcasecmp(key, deh_state_fields[13]))  // Args7
    {
      deh_log(" - args[6] = %lld\n", (statearg_t)value);
      deh_state.state->args[6] = (statearg_t)value;
      *deh_state.defined_codeptr_args |= (1 << 6);
    }
    else if (!deh_strcasecmp(key, deh_state_fields[14]))  // Args8
    {
      deh_log(" - args[7] = %lld\n", (statearg_t)value);
      deh_state.state->args[7] = (statearg_t)value;
      *deh_state.defined_codeptr_args |= (1 << 7);
    }
    else if (!deh_strcasecmp(key, deh_state_fields[15]))  // MBF21 Bits
    {
      if (bGetData == 1)
      {
        value = deh_translate_bits(value, deh_stateflags_mbf21);
      }
      else
      {
        for (value = 0; (strval = strtok(strval, deh_getBitsDelims())); strval = NULL) {
          const struct deh_flag_s *flag;

          for (flag = deh_stateflags_mbf21; flag->name; flag++) {
            if (deh_strcasecmp(strval, flag->name)) continue;

            value |= flag->value;
            break;
          }

          if (!flag->name) {
            deh_log("Could not find MBF21 frame bit mnemonic %s\n", strval);
          }
        }
      }

      deh_state.state->flags = value;
    }
    else
      deh_log("Invalid frame string index for '%s'\n", key);
  }
}

// ====================================================================
// deh_procPointer
// Purpose: Handle DEH Code pointer block, can use BEX [CODEPTR] instead
// Args:    fpin  -- input file stream
//          line  -- current line in file to process
// Returns: void
//
static void deh_procPointer(DEHFILE *fpin, char *line) // done
{
  char key[DEH_MAXKEYLEN];
  char inbuffer[DEH_BUFFERMAX];
  uint64_t value;      // All deh values are ints or longs
  int indexnum;
  size_t i; // looper
  dsda_deh_state_t deh_state, ptr_state;

  strncpy(inbuffer, line, DEH_BUFFERMAX - 1);
  // NOTE: different format from normal

  // killough 8/98: allow hex numbers in input, fix error case:
  if (sscanf(inbuffer, "%*s %*i (%s %i)", key, &indexnum) != 2)
  {
    deh_log("Bad data pair in '%s'\n", inbuffer);
    return;
  }

  deh_log("Processing Pointer at index %d: %s\n", indexnum, key);
  if (indexnum < 0)
  {
    deh_log("Pointer number must be positive (%d)\n", indexnum);
    return;
  }

  deh_state = dsda_GetDehState(indexnum);

  while (!dehfeof(fpin) && *inbuffer && (*inbuffer != ' '))
  {
    if (!dehfgets(inbuffer, sizeof(inbuffer), fpin)) break;
    lfstrip(inbuffer);
    if (!*inbuffer) break;       // killough 11/98
    if (!deh_GetData(inbuffer, key, &value, NULL)) // returns TRUE if ok
    {
      deh_log("Bad data pair in '%s'\n", inbuffer);
      continue;
    }

    ptr_state = dsda_GetDehState(value);

    if (!deh_strcasecmp(key, deh_state_fields[4]))  // Codep frame (not set in Frame deh block)
    {
      deh_state.state->action = *ptr_state.codeptr;
      deh_log(" - applied from codeptr[%ld] to states[%d]\n", (long)value, indexnum);
      // Write BEX-oriented line to match:
      for (i = 0; i < sizeof(deh_bexptrs) / sizeof(*deh_bexptrs); i++)
      {
        if (!memcmp(&deh_bexptrs[i].cptr, ptr_state.codeptr, sizeof(actionf_t)))
        {
          deh_log("BEX [CODEPTR] -> FRAME %d = %s\n", indexnum, &deh_bexptrs[i].lookup[2]);
          break;
        }
        if (deh_bexptrs[i].cptr == NULL) // stop at null entry
          break;
      }
    }
    else
      deh_log("Invalid frame pointer index for '%s' at %ld\n", key, (long)value);
  }
}

// ====================================================================
// deh_procSounds
// Purpose: Handle DEH Sounds block
// Args:    fpin  -- input file stream
//          line  -- current line in file to process
// Returns: void
//
static void deh_procSounds(DEHFILE *fpin, char *line)
{
  char key[DEH_MAXKEYLEN];
  char inbuffer[DEH_BUFFERMAX];
  uint64_t value;      // All deh values are ints or longs
  int indexnum;
  sfxinfo_t *deh_sfx;

  strncpy(inbuffer, line, DEH_BUFFERMAX - 1);

  // killough 8/98: allow hex numbers in input:
  sscanf(inbuffer, "%s %i", key, &indexnum);
  deh_log("Processing Sounds at index %d: %s\n", indexnum, key);
  if (indexnum < 0)
    deh_log("Sound number must be positive (%d)\n", indexnum);

  deh_sfx = dsda_GetDehSFX(indexnum);

  while (!dehfeof(fpin) && *inbuffer && (*inbuffer != ' '))
  {
    if (!dehfgets(inbuffer, sizeof(inbuffer), fpin)) break;
    lfstrip(inbuffer);
    if (!*inbuffer) break;         // killough 11/98
    if (!deh_GetData(inbuffer, key, &value, NULL)) // returns TRUE if ok
    {
      deh_log("Bad data pair in '%s'\n", inbuffer);
      continue;
    }

    if (!deh_strcasecmp(key, "Offset"))
      ; // ignored
    else if (!deh_strcasecmp(key, "Zero/One"))
      ; // ignored
    else if (!deh_strcasecmp(key, "Value"))
      deh_sfx->priority = (int)value;
    else if (!deh_strcasecmp(key, "Zero 1"))
      ; // ignored
    else if (!deh_strcasecmp(key, "Zero 2"))
      deh_sfx->pitch = (int)value;
    else if (!deh_strcasecmp(key, "Zero 3"))
      deh_sfx->volume = (int)value;
    else if (!deh_strcasecmp(key, "Zero 4"))
      ; // ignored
    else if (!deh_strcasecmp(key, "Neg. One 1"))
      ; // ignored
    else if (!deh_strcasecmp(key, "Neg. One 2"))
      ; // ignored
    else
      deh_log("Invalid sound string index for '%s'\n", key);
  }
}

// ====================================================================
// deh_procAmmo
// Purpose: Handle DEH Ammo block
// Args:    fpin  -- input file stream
//          line  -- current line in file to process
// Returns: void
//
static void deh_procAmmo(DEHFILE *fpin, char *line)
{
  char key[DEH_MAXKEYLEN];
  char inbuffer[DEH_BUFFERMAX];
  uint64_t value;      // All deh values are ints or longs
  int indexnum;

  strncpy(inbuffer, line, DEH_BUFFERMAX - 1);

  // killough 8/98: allow hex numbers in input:
  sscanf(inbuffer, "%s %i", key, &indexnum);
  deh_log("Processing Ammo at index %d: %s\n", indexnum, key);
  if (indexnum < 0 || indexnum >= NUMAMMO)
    deh_log("Bad ammo number %d of %d\n", indexnum, NUMAMMO);

  while (!dehfeof(fpin) && *inbuffer && (*inbuffer != ' '))
  {
    if (!dehfgets(inbuffer, sizeof(inbuffer), fpin)) break;
    lfstrip(inbuffer);
    if (!*inbuffer) break;       // killough 11/98
    if (!deh_GetData(inbuffer, key, &value, NULL)) // returns TRUE if ok
    {
      deh_log("Bad data pair in '%s'\n", inbuffer);
      continue;
    }
    if (!deh_strcasecmp(key, deh_ammo[0]))  // Max ammo
      maxammo[indexnum] = (int)value;
    else if (!deh_strcasecmp(key, deh_ammo[1]))  // Per ammo
      clipammo[indexnum] = (int)value;
    else
      deh_log("Invalid ammo string index for '%s'\n", key);
  }
}

// ====================================================================
// deh_procWeapon
// Purpose: Handle DEH Weapon block
// Args:    fpin  -- input file stream
//          line  -- current line in file to process
// Returns: void
//
static void deh_procWeapon(DEHFILE *fpin, char *line)
{
  char key[DEH_MAXKEYLEN];
  char inbuffer[DEH_BUFFERMAX];
  uint64_t value;      // All deh values are ints or longs
  int indexnum;
  char *strval;
  int bGetData;

  strncpy(inbuffer, line, DEH_BUFFERMAX - 1);

  // killough 8/98: allow hex numbers in input:
  sscanf(inbuffer, "%s %i", key, &indexnum);
  deh_log("Processing Weapon at index %d: %s\n", indexnum, key);
  if (indexnum < 0 || indexnum >= NUMWEAPONS)
    deh_log("Bad weapon number %d of %d\n", indexnum, NUMAMMO);

  while (!dehfeof(fpin) && *inbuffer && (*inbuffer != ' '))
  {
    if (!dehfgets(inbuffer, sizeof(inbuffer), fpin)) break;
    lfstrip(inbuffer);
    if (!*inbuffer) break;       // killough 11/98
    bGetData = deh_GetData(inbuffer, key, &value, &strval);
    if (!bGetData) // returns TRUE if ok
    {
      deh_log("Bad data pair in '%s'\n", inbuffer);
      continue;
    }
    if (!deh_strcasecmp(key, deh_weapon[0]))  // Ammo type
    {
      if (!raven && value == 5) value = am_noammo;
      weaponinfo[indexnum].ammo = (ammotype_t)value;
    }
    else if (!deh_strcasecmp(key, deh_weapon[1]))  // Deselect frame
      weaponinfo[indexnum].upstate = (int)value;
    else if (!deh_strcasecmp(key, deh_weapon[2]))  // Select frame
      weaponinfo[indexnum].downstate = (int)value;
    else if (!deh_strcasecmp(key, deh_weapon[3]))  // Bobbing frame
      weaponinfo[indexnum].readystate = (int)value;
    else if (!deh_strcasecmp(key, deh_weapon[4]))  // Shooting frame
      weaponinfo[indexnum].atkstate = (int)value;
    else if (!deh_strcasecmp(key, deh_weapon[5]))  // Firing frame
      weaponinfo[indexnum].flashstate = (int)value;
    else if (!deh_strcasecmp(key, deh_weapon[6]))  // Ammo per shot
    {
      weaponinfo[indexnum].ammopershot = (int)value;
      weaponinfo[indexnum].intflags |= WIF_ENABLEAPS;
    }
    else if (!deh_strcasecmp(key, deh_weapon[7]))  // MBF21 Bits
    {
      if (bGetData == 1)
      {
        value = deh_translate_bits(value, deh_weaponflags_mbf21);
      }
      else
      {
        for (value = 0; (strval = strtok(strval, deh_getBitsDelims())); strval = NULL) {
          const struct deh_flag_s *flag;

          for (flag = deh_weaponflags_mbf21; flag->name; flag++) {
            if (deh_strcasecmp(strval, flag->name)) continue;

            value |= flag->value;
            break;
          }

          if (!flag->name) {
            deh_log("Could not find MBF21 weapon bit mnemonic %s\n", strval);
          }
        }
      }

      weaponinfo[indexnum].flags = value;
    }
    else
      deh_log("Invalid weapon string index for '%s'\n",key);
  }
}

// ====================================================================
// deh_procSprite
// Purpose: Dummy - we do not support the DEH Sprite block
// Args:    fpin  -- input file stream
//          line  -- current line in file to process
// Returns: void
//
static void deh_procSprite(DEHFILE *fpin, char *line) // Not supported
{
  char key[DEH_MAXKEYLEN];
  char inbuffer[DEH_BUFFERMAX];
  int indexnum;

  // Too little is known about what this is supposed to do, and
  // there are better ways of handling sprite renaming.  Not supported.

  strncpy(inbuffer, line, DEH_BUFFERMAX - 1);

  // killough 8/98: allow hex numbers in input:
  sscanf(inbuffer, "%s %i", key, &indexnum);
  deh_log("Ignoring Sprite offset change at index %d: %s\n", indexnum, key);
  while (!dehfeof(fpin) && *inbuffer && (*inbuffer != ' '))
  {
    if (!dehfgets(inbuffer, sizeof(inbuffer), fpin)) break;
    lfstrip(inbuffer);
    if (!*inbuffer) break;      // killough 11/98
    // ignore line
    deh_log("- %s\n", inbuffer);
  }
}

// ====================================================================
// deh_procPars
// Purpose: Handle BEX extension for PAR times
// Args:    fpin  -- input file stream
//          line  -- current line in file to process
// Returns: void
//
static void deh_procPars(DEHFILE *fpin, char *line) // extension
{
  char key[DEH_MAXKEYLEN];
  char inbuffer[DEH_BUFFERMAX];
  int indexnum;
  int episode, level, partime, oldpar;

  // new item, par times
  // usage: After [PARS] Par 0 section identifier, use one or more of these
  // lines:
  //  par 3 5 120
  //  par 14 230
  // The first would make the par for E3M5 be 120 seconds, and the
  // second one makes the par for MAP14 be 230 seconds.  The number
  // of parameters on the line determines which group of par values
  // is being changed.  Error checking is done based on current fixed
  // array sizes of[4][10] and [32]

  strncpy(inbuffer, line, DEH_BUFFERMAX - 1);

  // killough 8/98: allow hex numbers in input:
  sscanf(inbuffer, "%s %i", key, &indexnum);
  deh_log("Processing Par value at index %d: %s\n", indexnum, key);
  // indexnum is a dummy entry
  while (!dehfeof(fpin) && *inbuffer && (*inbuffer != ' '))
  {
    if (!dehfgets(inbuffer, sizeof(inbuffer), fpin)) break;
    lfstrip(M_Strlwr(inbuffer)); // lowercase it
    if (!*inbuffer) break;      // killough 11/98
    if (3 != sscanf(inbuffer, "par %i %i %i", &episode, &level, &partime))
    { // not 3
      if (2 != sscanf(inbuffer, "par %i %i", &level, &partime))
      { // not 2
        deh_log("Invalid par time setting string: %s\n", inbuffer);
      }
      else
      { // is 2
        // Ty 07/11/98 - wrong range check, not zero-based
        if (level < 1 || level > 32) // base 0 array (but 1-based parm)
        {
          deh_log("Invalid MAPnn value MAP%d\n", level);
        }
        else
        {
          oldpar = cpars[level-1];
          deh_log("Changed par time for MAP%02d from %d to %d\n", level, oldpar, partime);
          cpars[level - 1] = partime;
          deh_pars = TRUE;
        }
      }
    }
    else
    { // is 3
      // note that though it's a [4][10] array, the "left" and "top" aren't used,
      // effectively making it a base 1 array.
      // Ty 07/11/98 - level was being checked against max 3 - dumb error
      // Note that episode 4 does not have par times per original design
      // in Ultimate DOOM so that is not supported here.
      if (episode < 1 || episode > 3 || level < 1 || level > 9)
      {
        deh_log("Invalid ExMx values E%dM%d\n", episode, level);
      }
      else
      {
        oldpar = pars[episode][level];
        pars[episode][level] = partime;
        deh_log("Changed par time for E%dM%d from %d to %d\n", episode, level, oldpar, partime);
        deh_pars = TRUE;
      }
    }
  }
}

// ====================================================================
// deh_procCheat
// Purpose: Handle DEH Cheat block
// Args:    fpin  -- input file stream
//          line  -- current line in file to process
// Returns: void
//
static void deh_procCheat(DEHFILE *fpin, char *line) // done
{
  char key[DEH_MAXKEYLEN];
  char inbuffer[DEH_BUFFERMAX];
  uint64_t value;      // All deh values are ints or longs
  char ch = 0; // CPhipps - `writable' null string to initialise...
  char *strval = &ch;  // pointer to the value area
  int ix, iy;   // array indices
  char *p;  // utility pointer

  deh_log("Processing Cheat: %s\n", line);

  strncpy(inbuffer, line, DEH_BUFFERMAX - 1);
  while (!dehfeof(fpin) && *inbuffer && (*inbuffer != ' '))
  {
    if (!dehfgets(inbuffer, sizeof(inbuffer), fpin)) break;
    lfstrip(inbuffer);
    if (!*inbuffer) break;       // killough 11/98
    if (!deh_GetData(inbuffer, key, &value, &strval)) // returns TRUE if ok
    {
      deh_log("Bad data pair in '%s'\n", inbuffer);
      continue;
    }
    // Otherwise we got a (perhaps valid) cheat name,
    // so look up the key in the array

    // killough 4/18/98: use main cheat code table in st_stuff.c now
    for (ix = 0; cheat[ix].cheat; ix++)
      if (cheat[ix].deh_cheat)   // killough 4/18/98: skip non-deh
      {
        if (!stricmp(key,cheat[ix].deh_cheat))  // found the cheat, ignored case
        {
          // replace it but don't overflow it.  Use current length as limit.
          // Ty 03/13/98 - add 0xff code
          // Deal with the fact that the cheats in deh files are extended
          // with character 0xFF to the original cheat length, which we don't do.
          for (iy = 0; strval[iy]; iy++)
            strval[iy] = (strval[iy] == (char)0xff) ? '\0' : strval[iy];

          iy = ix;     // killough 4/18/98

          // Ty 03/14/98 - skip leading spaces
          p = strval;
          while (*p == ' ') ++p;

          //e6y: ability to ignore cheats in dehacked files.
          if (dsda_IntConfig(dsda_config_deh_apply_cheats) && !dsda_Flag(dsda_arg_nocheats))
          {
            cheat[iy].cheat = Z_Strdup(p);
            deh_log("Assigned new cheat '%s' to cheat '%s'at index %d\n",
                    p, cheat[ix].deh_cheat, iy); // killough 4/18/98
          }
        }
      }
    deh_log("- %s\n", inbuffer);
  }
}

// ====================================================================
// deh_procMisc
// Purpose: Handle DEH Misc block
// Args:    fpin  -- input file stream
//          line  -- current line in file to process
// Returns: void
//
static void deh_procMisc(DEHFILE *fpin, char *line) // done
{
  char key[DEH_MAXKEYLEN];
  char inbuffer[DEH_BUFFERMAX];
  uint64_t value;      // All deh values are ints or longs

  strncpy(inbuffer, line, DEH_BUFFERMAX - 1);
  while (!dehfeof(fpin) && *inbuffer && (*inbuffer != ' '))
  {
    if (!dehfgets(inbuffer, sizeof(inbuffer), fpin)) break;
    lfstrip(inbuffer);
    if (!*inbuffer) break;    // killough 11/98
    if (!deh_GetData(inbuffer, key, &value, NULL)) // returns TRUE if ok
    {
      deh_log("Bad data pair in '%s'\n", inbuffer);
      continue;
    }
    // Otherwise it's ok
    deh_log("Processing Misc item '%s'\n", key);

    if (!deh_strcasecmp(key, deh_misc[0]))  // Initial Health
      initial_health = (int)value;
    else if (!deh_strcasecmp(key, deh_misc[1]))  // Initial Bullets
      initial_bullets = (int)value;
    else if (!deh_strcasecmp(key, deh_misc[2]))  // Max Health
    {
      IsDehMaxHealth = true;
      deh_maxhealth = (int)value; //e6y
    }
    else if (!deh_strcasecmp(key, deh_misc[3]))  // Max Armor
      max_armor = (int)value;
    else if (!deh_strcasecmp(key, deh_misc[4]))  // Green Armor Class
      green_armor_class = (int)value;
    else if (!deh_strcasecmp(key, deh_misc[5]))  // Blue Armor Class
      blue_armor_class = (int)value;
    else if (!deh_strcasecmp(key, deh_misc[6]))  // Max Soulsphere
    {
      IsDehMaxSoul = true;
      deh_max_soul = (int)value; //e6y
    }
    else if (!deh_strcasecmp(key, deh_misc[7]))  // Soulsphere Health
      soul_health = (int)value;
    else if (!deh_strcasecmp(key, deh_misc[8]))  // Megasphere Health
    {
      IsDehMegaHealth = true;
      deh_mega_health = (int)value; //e6y
    }
    else if (!deh_strcasecmp(key, deh_misc[9]))  // God Mode Health
      god_health = (int)value;
    else if (!deh_strcasecmp(key, deh_misc[10]))  // IDFA Armor
      idfa_armor = (int)value;
    else if (!deh_strcasecmp(key, deh_misc[11]))  // IDFA Armor Class
      idfa_armor_class = (int)value;
    else if (!deh_strcasecmp(key, deh_misc[12]))  // IDKFA Armor
      idkfa_armor = (int)value;
    else if (!deh_strcasecmp(key, deh_misc[13]))  // IDKFA Armor Class
      idkfa_armor_class = (int)value;
    else if (!deh_strcasecmp(key, deh_misc[14]))  // BFG Cells/Shot
    {
      weaponinfo[wp_bfg].ammopershot = bfgcells = (int)value;
      bfgcells_modified = true;
    }
    else if (!deh_strcasecmp(key, deh_misc[15]))  // Monsters Infight
    {
      // e6y: Dehacked support - monsters infight
      if (value == 202) monsters_infight = 0;
      else if (value == 221) monsters_infight = 1;
      else deh_log("Invalid value for 'Monsters Infight': %i", (int)value);

      /* No such switch in DOOM - nop */ //e6y ;
    }
    else
      deh_log("Invalid misc item string index for '%s'\n", key);
  }
}

// ====================================================================
// deh_procText
// Purpose: Handle DEH Text block
// Notes:   We look things up in the current information and if found
//          we replace it.  At the same time we write the new and
//          improved BEX syntax to the log file for future use.
// Args:    fpin  -- input file stream
//          line  -- current line in file to process
// Returns: void
//
static void deh_procText(DEHFILE *fpin, char *line)
{
  char key[DEH_MAXKEYLEN];
  char inbuffer[DEH_BUFFERMAX * 2];  // can't use line -- double size buffer too.
  int i; // loop variable
  int fromlen, tolen;  // as specified on the text block line
  int usedlen;  // shorter of fromlen and tolen if not matched
  dboolean found = FALSE;  // to allow early exit once found
  char* line2 = NULL;   // duplicate line for rerouting

  // Ty 04/11/98 - Included file may have NOTEXT skip flag set
  if (includenotext) // flag to skip included deh-style text
  {
    deh_log("Skipped text block because of notext directive\n");
    strcpy(inbuffer,line);
    while (!dehfeof(fpin) && *inbuffer && (*inbuffer != ' '))
      dehfgets(inbuffer, sizeof(inbuffer), fpin);  // skip block
    // Ty 05/17/98 - don't care if this fails
    return; // ************** Early return
  }

  // killough 8/98: allow hex numbers in input:
  sscanf(line, "%s %i %i", key, &fromlen, &tolen);
  deh_log("Processing Text (key=%s, from=%d, to=%d)\n", key, fromlen, tolen);

  // killough 10/98: fix incorrect usage of feof
  {
    int c, totlen = 0;
    while (totlen < fromlen + tolen && (c = dehfgetc(fpin)) != EOF)
      if (c != '\r')
        inbuffer[totlen++] = c;
    inbuffer[totlen]='\0';
  }

  // if the from and to are 4, this may be a sprite rename.  Check it
  // against the array and process it as such if it matches.  Remember
  // that the original names are (and should remain) uppercase.
  // Future: this will be from a separate [SPRITES] block.
  if (fromlen == 4 && tolen == 4)
  {
    i = dsda_GetDehSpriteIndex(inbuffer);

    if (i >= 0)
    {
      char *s;

      deh_log("Changing name of sprite at index %d from %s to %*s\n",
              i, sprnames[i], tolen, &inbuffer[fromlen]);

      // CPhipps - fix constness problem
      sprnames[i] = s = Z_Strdup(sprnames[i]);
      strncpy(s, &inbuffer[fromlen], tolen);

      found = TRUE;
    }
  }

  if (!found && fromlen < 7 && tolen < 7)  // lengths of music and sfx are 6 or shorter
  {
    usedlen = (fromlen < tolen) ? fromlen : tolen;
    if (fromlen != tolen)
      deh_log("Warning: Mismatched lengths from=%d, to=%d, used %d\n", fromlen, tolen, usedlen);

    i = dsda_GetDehSFXIndex(inbuffer, (size_t) fromlen);
    if (i >= 0)
    {
      deh_log("Changing name of sfx from %s to %*s\n",
              S_sfx[i].name, usedlen, &inbuffer[fromlen]);

      S_sfx[i].name = Z_Strdup(&inbuffer[fromlen]);

      found = TRUE;
    }
    else
    {
      i = dsda_GetDehMusicIndex(inbuffer, (size_t) fromlen);
      if (i >= 0)
      {
        deh_log("Changing name of music from %s to %*s\n",
                S_music[i].name, usedlen, &inbuffer[fromlen]);

        S_music[i].name = Z_Strdup(&inbuffer[fromlen]);

        found = TRUE;
      }
    }
  }

  if (!found) // Nothing we want to handle here--see if strings can deal with it.
  {
    deh_log("Checking text area through strings for '%.12s%s' from=%d to=%d\n",
            inbuffer, (strlen(inbuffer) > 12) ? "..." : "", fromlen, tolen);
    if ((size_t)fromlen <= strlen(inbuffer))
    {
      line2 = Z_Strdup(&inbuffer[fromlen]);
      inbuffer[fromlen] = '\0';
    }

    deh_procStringSub(NULL, inbuffer, line2);
  }
  Z_Free(line2); // may be NULL, ignored by free()
}

static void deh_procError(DEHFILE *fpin, char *line)
{
  char inbuffer[DEH_BUFFERMAX];

  strncpy(inbuffer, line, DEH_BUFFERMAX - 1);
  deh_log("Unmatched Block: '%s'\n", inbuffer);
}

// ====================================================================
// deh_procStrings
// Purpose: Handle BEX [STRINGS] extension
// Args:    fpin  -- input file stream
//          line  -- current line in file to process
// Returns: void
//
static void deh_procStrings(DEHFILE *fpin, char *line)
{
  char key[DEH_MAXKEYLEN];
  char inbuffer[DEH_BUFFERMAX];
  uint64_t value;    // All deh values are ints or longs
  char *strval;      // holds the string value of the line
  static size_t maxstrlen = 128; // maximum string length, bumped 128 at
  // a time as needed
  // holds the final result of the string after concatenation
  static char *holdstring = NULL;
  dboolean found = false;  // looking for string continuation

  deh_log("Processing extended string substitution\n");

  if (!holdstring) holdstring = Z_Malloc(maxstrlen * sizeof(*holdstring));

  *holdstring = '\0';  // empty string to start with
  strncpy(inbuffer, line, DEH_BUFFERMAX - 1);
  // Ty 04/24/98 - have to allow inbuffer to start with a blank for
  // the continuations of C1TEXT etc.
  while (!dehfeof(fpin) && *inbuffer)  /* && (*inbuffer != ' ') */
  {
    if (!dehfgets(inbuffer, sizeof(inbuffer), fpin)) break;
    if (*inbuffer == '#') continue;  // skip comment lines
    lfstrip(inbuffer);
    if (!*inbuffer && !*holdstring) break;  // killough 11/98
    if (!*holdstring) // first one--get the key
    {
      if (!deh_GetData(inbuffer, key, &value, &strval)) // returns TRUE if ok
      {
        deh_log("Bad data pair in '%s'\n", inbuffer);
        continue;
      }
    }
    while (strlen(holdstring) + strlen(inbuffer) > maxstrlen) // Ty03/29/98 - fix stupid error
    {
      // killough 11/98: allocate enough the first time
      maxstrlen = strlen(holdstring) + strlen(inbuffer);
      deh_log("* increased buffer from to %ld for buffer size %d\n",
              (long)maxstrlen, (int)strlen(inbuffer));
      holdstring = Z_Realloc(holdstring, maxstrlen * sizeof(*holdstring));
    }
    // concatenate the whole buffer if continuation or the value iffirst
    strcat(holdstring, ptr_lstrip(((*holdstring) ? inbuffer : strval)));
    rstrip(holdstring);
    // delete any trailing blanks past the backslash
    // note that blanks before the backslash will be concatenated
    // but ones at the beginning of the next line will not, allowing
    // indentation in the file to read well without affecting the
    // string itself.
    if (holdstring[strlen(holdstring) - 1] == '\\')
    {
      holdstring[strlen(holdstring) - 1] = '\0';
      continue; // ready to concatenate
    }
    if (*holdstring) // didn't have a backslash, trap above would catch that
    {
      // go process the current string
      found = deh_procStringSub(key, NULL, holdstring);  // supply keyand not search string

      if (!found)
        deh_log("Invalid string key '%s', substitution skipped.\n", key);

      *holdstring = '\0';  // empty string for the next one
    }
  }
}

// ====================================================================
// deh_procStringSub
// Purpose: Common string parsing and handling routine for DEH and BEX
// Args:    key       -- place to put the mnemonic for the string if found
//          lookfor   -- original value string to look for
//          newstring -- string to put in its place if found
// Returns: dboolean: True if string found, false if not
//
dboolean deh_procStringSub(char *key, char *lookfor, char *newstring)
{
  dboolean found; // loop exit flag
  int i;  // looper

  found = false;
  for (i = 0; i < deh_numstrlookup; i++)
  {
    if (deh_strlookup[i].orig == NULL)
    {
      deh_strlookup[i].orig = *deh_strlookup[i].ppstr;
    }
    found = lookfor ?
      !stricmp(deh_strlookup[i].orig, lookfor) :
      !stricmp(deh_strlookup[i].lookup, key);

    if (found)
    {
      char *t;
      *deh_strlookup[i].ppstr = t = Z_Strdup(newstring); // orphan originalstring
      found = true;
      // Handle embedded \n's in the incoming string, convert to 0x0a's
      {
        const char *s;
        for (s = *deh_strlookup[i].ppstr; *s; ++s, ++t)
        {
          if (*s == '\\' && (s[1] == 'n' || s[1] == 'N')) //found one
            ++s, *t = '\n';  // skip one extra for second character
          else
            *t = *s;
        }
        *t = '\0';  // cap off the target string
      }

      if (key)
        deh_log("Assigned key %s => '%s'\n", key, newstring);

      if (!key)
        deh_log("Assigned '%.12s%s' to'%.12s%s' at key %s\n",
                lookfor, (strlen(lookfor) > 12) ? "..." : "",
                newstring, (strlen(newstring) > 12) ? "..." :"",
                deh_strlookup[i].lookup);

      if (!key) // must have passed an old style string so showBEX
        deh_log("*BEX FORMAT:\n%s = %s\n*END BEX\n",
                deh_strlookup[i].lookup, dehReformatStr(newstring));

      break;
    }
  }
  if (!found)
    deh_log("Could not find '%.12s'\n", key ? key : lookfor);

  return found;
}

//========================================================================
// haleyjd 9/22/99
//
// deh_procHelperThing
//
// Allows handy substitution of any thing for helper dogs.  DEH patches
// are being made frequently for this purpose and it requires a complete
// rewiring of the DOG thing.  I feel this is a waste of effort, and so
// have added this new [HELPER] BEX block

static void deh_procHelperThing(DEHFILE *fpin, char *line)
{
  char key[DEH_MAXKEYLEN];
  char inbuffer[DEH_BUFFERMAX];
  uint64_t value;      // All deh values are ints or longs

  strncpy(inbuffer, line, DEH_BUFFERMAX - 1);
  while (!dehfeof(fpin) && *inbuffer && (*inbuffer != ' '))
  {
    if (!dehfgets(inbuffer, sizeof(inbuffer), fpin)) break;
    lfstrip(inbuffer);
    if (!*inbuffer) break;
    if (!deh_GetData(inbuffer, key, &value, NULL)) // returns TRUE if ok
    {
      deh_log("Bad data pair in '%s'\n", inbuffer);
      continue;
    }
    // Otherwise it's ok
    deh_log("Processing Helper Thing item '%s'\nvalue is %i", key, (int)value);
    if (!strncasecmp(key, "type", 4))
      HelperThing = dsda_TranslateDehMobjIndex((int)value);
  }
}

//
// deh_procBexSprites
//
// Supports sprite name substitutions without requiring use
// of the DeHackEd Text block
//
static void deh_procBexSprites(DEHFILE *fpin, char *line)
{
  char key[DEH_MAXKEYLEN];
  char inbuffer[DEH_BUFFERMAX];
  uint64_t value;    // All deh values are ints or longs
  char *strval;  // holds the string value of the line
  char candidate[5];
  int  match;

  deh_log("Processing sprite name substitution\n");

  strncpy(inbuffer, line, DEH_BUFFERMAX - 1);

  while (!dehfeof(fpin) && *inbuffer && (*inbuffer != ' '))
  {
    if (!dehfgets(inbuffer, sizeof(inbuffer), fpin))
      break;
    if (*inbuffer == '#')
      continue;  // skip comment lines
    lfstrip(inbuffer);
    if (!*inbuffer)
      break;  // killough 11/98
    if (!deh_GetData(inbuffer, key, &value, &strval)) // returns TRUE if ok
    {
      deh_log("Bad data pair in '%s'\n", inbuffer);
      continue;
    }
    // do it
    memset(candidate, 0, sizeof(candidate));
    strncpy(candidate, ptr_lstrip(strval), 4);
    if (strlen(candidate) != 4)
    {
      deh_log("Bad length for sprite name '%s'\n", candidate);
      continue;
    }

    match = dsda_GetOriginalSpriteIndex(key);
    if (match >= 0)
    {
      deh_log("Substituting '%s' for sprite '%s'\n", candidate, key);
      sprnames[match] = Z_Strdup(candidate);
    }
  }
}

// ditto for sound names
static void deh_procBexSounds(DEHFILE *fpin, char *line)
{
  char key[DEH_MAXKEYLEN];
  char inbuffer[DEH_BUFFERMAX];
  uint64_t value;    // All deh values are ints or longs
  char *strval;  // holds the string value of the line
  char candidate[7];
  int  match;
  size_t len;

  deh_log("Processing sound name substitution\n");

  strncpy(inbuffer, line, DEH_BUFFERMAX - 1);

  while (!dehfeof(fpin) && *inbuffer && (*inbuffer != ' '))
  {
    if (!dehfgets(inbuffer, sizeof(inbuffer), fpin))
      break;
    if (*inbuffer == '#')
      continue;  // skip comment lines
    lfstrip(inbuffer);
    if (!*inbuffer)
      break;  // killough 11/98
    if (!deh_GetData(inbuffer, key, &value, &strval)) // returns TRUE if ok
    {
      deh_log("Bad data pair in '%s'\n", inbuffer);
      continue;
    }
    // do it
    memset(candidate, 0, 7);
    strncpy(candidate, ptr_lstrip(strval), 6);
    len = strlen(candidate);
    if (len < 1 || len > 6)
    {
      deh_log("Bad length for sound name '%s'\n", candidate);
      continue;
    }

    match = dsda_GetOriginalSFXIndex(key);
    if (match >= 0)
    {
      deh_log("Substituting '%s' for sound '%s'\n", candidate, key);
      S_sfx[match].name = Z_Strdup(candidate);
    }
  }
}

// ditto for music names
static void deh_procBexMusic(DEHFILE *fpin, char *line)
{
  char key[DEH_MAXKEYLEN];
  char inbuffer[DEH_BUFFERMAX];
  uint64_t value;    // All deh values are ints or longs
  char *strval;  // holds the string value of the line
  char candidate[7];
  int  match;
  size_t len;

  deh_log("Processing music name substitution\n");

  strncpy(inbuffer, line, DEH_BUFFERMAX - 1);

  while (!dehfeof(fpin) && *inbuffer && (*inbuffer != ' '))
  {
    if (!dehfgets(inbuffer, sizeof(inbuffer), fpin))
      break;
    if (*inbuffer == '#')
      continue;  // skip comment lines
    lfstrip(inbuffer);
    if (!*inbuffer)
      break;  // killough 11/98
    if (!deh_GetData(inbuffer, key, &value, &strval)) // returns TRUE if ok
    {
      deh_log("Bad data pair in '%s'\n", inbuffer);
      continue;
    }
    // do it
    memset(candidate, 0, 7);
    strncpy(candidate, ptr_lstrip(strval), 6);
    len = strlen(candidate);
    if (len < 1 || len > 6)
    {
      deh_log("Bad length for music name '%s'\n", candidate);
      continue;
    }

    match = dsda_GetOriginalMusicIndex(key);
    if (match >= 0)
    {
      deh_log("Substituting '%s' for music '%s'\n", candidate, key);
      S_music[match].name = Z_Strdup(candidate);
    }
  }
}

// ====================================================================
// General utility function(s)
// ====================================================================

// ====================================================================
// dehReformatStr
// Purpose: Convert a string into a continuous string with embedded
//          linefeeds for "\n" sequences in the source string
// Args:    string -- the string to convert
// Returns: the converted string (converted in a static buffer)
//
char *dehReformatStr(char *string)
{
  static char buff[DEH_BUFFERMAX]; // only processing the changed string,
  //  don't need double buffer
  char *s, *t;

  s = string;  // source
  t = buff;    // target
  // let's play...

  while (*s)
  {
    if (*s == '\n')
      ++s, *t++ = '\\', *t++ = 'n', *t++ = '\\', *t++='\n';
    else
      *t++ = *s++;
  }
  *t = '\0';
  return buff;
}

// ====================================================================
// lfstrip
// Purpose: Strips CR/LF off the end of a string
// Args:    s -- the string to work on
// Returns: void -- the string is modified in place
//
// killough 10/98: only strip at end of line, not entire string

void lfstrip(char *s)  // strip the \r and/or \n off of a line
{
  char *p = s + strlen(s);
  while (p > s && (*--p == '\r' || *p == '\n'))
    *p = 0;
}

// ====================================================================
// rstrip
// Purpose: Strips trailing blanks off a string
// Args:    s -- the string to work on
// Returns: void -- the string is modified in place
//
void rstrip(char *s)  // strip trailing whitespace
{
  char *p = s + strlen(s);         // killough 4/4/98: same here
  while (p > s && isspace(*--p)) // break on first non-whitespace
    *p = '\0';
}

// ====================================================================
// ptr_lstrip
// Purpose: Points past leading whitespace in a string
// Args:    s -- the string to work on
// Returns: char * pointing to the first nonblank character in the
//          string.  The original string is not changed.
//
char *ptr_lstrip(char *p)  // point past leading whitespace
{
  while (isspace(*p))
    p++;
  return p;
}

// ====================================================================
// deh_GetData
// Purpose: Get a key and data pair from a passed string
// Args:    s -- the string to be examined
//          k -- a place to put the key
//          l -- pointer to a long integer to store the number
//          strval -- a pointer to the place in s where the number
//                    value comes from.  Pass NULL to not use this.
// Notes:   Expects a key phrase, optional space, equal sign,
//          optional space and a value, mostly an int but treated
//          as a long just in case.  The passed pointer to hold
//          the key must be DEH_MAXKEYLEN in size.

dboolean deh_GetData(char *s, char *k, uint64_t *l, char **strval)
{
  char *t;  // current char
  int val; // to hold value of pair
  char buffer[DEH_MAXKEYLEN];  // to hold key in progress
  // e6y: Correction of wrong processing of Bits parameter if its value is equal to zero
  // No more desync on HACX demos.
  dboolean okrc = 1;  // assume good unless we have problems
  int i;  // iterator

  *buffer = '\0';
  val = 0;  // defaults in case not otherwise set
  for (i = 0, t = s; *t && i < DEH_MAXKEYLEN; t++, i++)
  {
    if (*t == '=') break;
    buffer[i] = *t;  // copy it
  }
  buffer[--i] = '\0';  // terminate the key before the '='
  if (!*t)  // end of string with no equal sign
  {
    okrc = FALSE;
  }
  else
  {
    if (!*++t)
    {
      val = 0;  // in case "thiskey =" with no value
      okrc = FALSE;
    }
    // we've incremented t
    // e6y: Correction of wrong processing of Bits parameter if its value is equal to zero
    // No more desync on HACX demos.
    // Old code: e6y val = strtol(t,NULL,0);  // killough 8/9/98: allow hex or octal input
    if (!M_StrToInt(t, &val))
    {
      val = 0;
      okrc = 2;
    }
  }

  // go put the results in the passed pointers
  *l = val;  // may be a faked zero

  // if spaces between key and equal sign, strip them
  strcpy(k, ptr_lstrip(buffer));  // could be a zero-length string

  if (strval != NULL) // pass NULL if you don't want this back
    *strval = t;      // pointer, has to be somewhere in s,
  // even if pointing at the zero byte.

  return(okrc);
}

static deh_bexptr null_bexptr = { NULL, "(NULL)" };

void PostProcessDeh(void)
{
  int i, j;
  const deh_bexptr *bexptr_match;

  // sanity-check bfgcells and bfg ammopershot
  if (
    bfgcells_modified &&
    weaponinfo[wp_bfg].intflags & WIF_ENABLEAPS &&
    bfgcells != weaponinfo[wp_bfg].ammopershot
  )
    I_Error("Mismatch between bfgcells and bfg ammo per shot modifications! Check your dehacked.");

  if (processed_dehacked)
  {
    extern byte* defined_codeptr_args;

    for (i = 0; i < num_states; i++)
    {
      bexptr_match = &null_bexptr;

      for (j = 0; deh_bexptrs[j].cptr != NULL; ++j)
        if (states[i].action == deh_bexptrs[j].cptr)
        {
          bexptr_match = &deh_bexptrs[j];
          break;
        }

      // ensure states don't use more mbf21 args than their
      // action pointer expects, for future-proofing's sake
      for (j = MAXSTATEARGS - 1; j >= bexptr_match->argcount; j--)
        if (states[i].args[j] != 0)
          I_Error("Action %s on state %d expects no more than %d nonzero args (%d found). Check your dehacked.",
            bexptr_match->lookup, i, bexptr_match->argcount, j+1);

      // replace unset fields with default values
      for (; j >= 0; j--)
        if (!(defined_codeptr_args[i] & (1 << j)))
          states[i].args[j] = bexptr_match->default_args[j];

      // State arguments that refer to thing indices need to be translated
      if (bexptr_match->ti_flags)
      {
        short args_i;
        short ti_flags = bexptr_match->ti_flags;

        if (ti_flags & TI_MISC1)
          states[i].misc1 = dsda_TranslateDehMobjIndex(states[i].misc1);

        if (ti_flags & TI_MISC2)
          states[i].misc2 = dsda_TranslateDehMobjIndex(states[i].misc2);

        ti_flags >>= TI_ARGSSHIFT;
        for (args_i = 0; args_i < MAXSTATEARGS; ++args_i)
          if (ti_flags & (1 << args_i))
            states[i].args[args_i] = dsda_TranslateDehMobjIndex(states[i].args[args_i]);
      }

      // Flags specifications aren't cross-port consistent -> must translate / mask bits
      if (bexptr_match->cptr == A_AddFlags || bexptr_match->cptr == A_RemoveFlags)
      {
        states[i].args[0] = deh_translate_bits(states[i].args[0], deh_mobjflags_standard);
        states[i].args[1] = deh_translate_bits(states[i].args[1], deh_mobjflags_mbf21);
      }
      else if (bexptr_match->cptr == A_JumpIfFlagsSet)
      {
        states[i].args[1] = deh_translate_bits(states[i].args[1], deh_mobjflags_standard);
        states[i].args[2] = deh_translate_bits(states[i].args[2], deh_mobjflags_mbf21);
      }
    }
  }

  dsda_FreeDehStates();
  dsda_FreeDehSprites();
  dsda_FreeDehSFX();
  dsda_FreeDehMusic();
  dsda_FreeDehMobjInfo();
}
