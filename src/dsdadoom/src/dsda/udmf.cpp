//
// Copyright(C) 2022 by Ryan Krafnick
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
//	DSDA UDMF
//

#include <cstring>
#include <vector>

extern "C" {
#include "stricmp.h"

char *Z_StrdupLevel(const char *s);
void *Z_MallocLevel(size_t size);
}

#include "scanner.h"

#include "udmf.h"

std::vector<udmf_line_t> udmf_lines;
std::vector<udmf_side_t> udmf_sides;
std::vector<udmf_vertex_t> udmf_vertices;
std::vector<udmf_sector_t> udmf_sectors;
std::vector<udmf_thing_t> udmf_things;

static void dsda_SkipValue(Scanner &scanner) {
  if (scanner.CheckToken('=')) {
    while (scanner.TokensLeft()) {
      if (scanner.CheckToken(';'))
        break;

      scanner.GetNextToken();
    }

    return;
  }

  scanner.MustGetToken('{');
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

// The scanner drops the sign when scanning, and we need it back
static char* dsda_FloatString(Scanner &scanner) {
  if (scanner.decimal >= 0)
    return Z_StrdupLevel(scanner.string);

  char* buffer = (char*) Z_MallocLevel(strlen(scanner.string) + 2);
  buffer[0] = '-';
  buffer[1] = '\0';
  strcat(buffer, scanner.string);

  return buffer;
}

#define SCAN_INT(x)  { scanner.MustGetToken('='); \
                       scanner.MustGetInteger(); \
                       x = scanner.number; \
                       scanner.MustGetToken(';'); }

#define SCAN_FLOAT(x) { scanner.MustGetToken('='); \
                        scanner.MustGetFloat(); \
                        x = scanner.decimal; \
                        scanner.MustGetToken(';'); }

#define SCAN_FLAG(x, f) { scanner.MustGetToken('='); \
                          scanner.MustGetToken(TK_BoolConst); \
                          if (scanner.boolean) \
                            x |= f; \
                          scanner.MustGetToken(';'); }

#define SCAN_STRING_N(x, n) { scanner.MustGetToken('='); \
                              scanner.MustGetToken(TK_StringConst); \
                              strncpy(x, scanner.string, n); \
                              scanner.MustGetToken(';'); }

#define SCAN_STRING(x) { scanner.MustGetToken('='); \
                         scanner.MustGetToken(TK_StringConst); \
                         x = Z_StrdupLevel(scanner.string); \
                         scanner.MustGetToken(';'); }

#define SCAN_FLOAT_STRING(x) { scanner.MustGetToken('='); \
                               scanner.MustGetFloat(); \
                               x = dsda_FloatString(scanner); \
                               scanner.MustGetToken(';'); }

static void dsda_ParseUDMFLineDef(Scanner &scanner) {
  udmf_line_t line = { 0 };

  line.id = -1;
  line.sideback = -1;
  line.alpha = 1.0;

  scanner.MustGetToken('{');
  while (!scanner.CheckToken('}')) {
    scanner.MustGetToken(TK_Identifier);

    if (!stricmp(scanner.string, "id")) {
      SCAN_INT(line.id);
    }
    else if (!stricmp(scanner.string, "v1")) {
      SCAN_INT(line.v1);
    }
    else if (!stricmp(scanner.string, "v2")) {
      SCAN_INT(line.v2);
    }
    else if (!stricmp(scanner.string, "special")) {
      SCAN_INT(line.special);
    }
    else if (!stricmp(scanner.string, "arg0")) {
      SCAN_INT(line.arg0);
    }
    else if (!stricmp(scanner.string, "arg1")) {
      SCAN_INT(line.arg1);
    }
    else if (!stricmp(scanner.string, "arg2")) {
      SCAN_INT(line.arg2);
    }
    else if (!stricmp(scanner.string, "arg3")) {
      SCAN_INT(line.arg3);
    }
    else if (!stricmp(scanner.string, "arg4")) {
      SCAN_INT(line.arg4);
    }
    else if (!stricmp(scanner.string, "sidefront")) {
      SCAN_INT(line.sidefront);
    }
    else if (!stricmp(scanner.string, "sideback")) {
      SCAN_INT(line.sideback);
    }
    else if (!stricmp(scanner.string, "locknumber")) {
      SCAN_INT(line.locknumber);
    }
    else if (!stricmp(scanner.string, "automapstyle")) {
      SCAN_INT(line.automapstyle);
    }
    else if (!stricmp(scanner.string, "health")) {
      SCAN_INT(line.health);
    }
    else if (!stricmp(scanner.string, "healthgroup")) {
      SCAN_INT(line.healthgroup);
    }
    else if (!stricmp(scanner.string, "alpha")) {
      SCAN_FLOAT(line.alpha);
    }
    else if (!stricmp(scanner.string, "blocking")) {
      SCAN_FLAG(line.flags, UDMF_ML_BLOCKING);
    }
    else if (!stricmp(scanner.string, "blockmonsters")) {
      SCAN_FLAG(line.flags, UDMF_ML_BLOCKMONSTERS);
    }
    else if (!stricmp(scanner.string, "twosided")) {
      SCAN_FLAG(line.flags, UDMF_ML_TWOSIDED);
    }
    else if (!stricmp(scanner.string, "dontpegtop")) {
      SCAN_FLAG(line.flags, UDMF_ML_DONTPEGTOP);
    }
    else if (!stricmp(scanner.string, "dontpegbottom")) {
      SCAN_FLAG(line.flags, UDMF_ML_DONTPEGBOTTOM);
    }
    else if (!stricmp(scanner.string, "secret")) {
      SCAN_FLAG(line.flags, UDMF_ML_SECRET);
    }
    else if (!stricmp(scanner.string, "blocksound")) {
      SCAN_FLAG(line.flags, UDMF_ML_SOUNDBLOCK);
    }
    else if (!stricmp(scanner.string, "dontdraw")) {
      SCAN_FLAG(line.flags, UDMF_ML_DONTDRAW);
    }
    else if (!stricmp(scanner.string, "mapped")) {
      SCAN_FLAG(line.flags, UDMF_ML_MAPPED);
    }
    else if (!stricmp(scanner.string, "passuse")) {
      SCAN_FLAG(line.flags, UDMF_ML_PASSUSE);
    }
    else if (!stricmp(scanner.string, "translucent")) {
      SCAN_FLAG(line.flags, UDMF_ML_TRANSLUCENT);
    }
    else if (!stricmp(scanner.string, "jumpover")) {
      SCAN_FLAG(line.flags, UDMF_ML_JUMPOVER);
    }
    else if (!stricmp(scanner.string, "blockfloaters")) {
      SCAN_FLAG(line.flags, UDMF_ML_BLOCKFLOATERS);
    }
    else if (!stricmp(scanner.string, "playercross")) {
      SCAN_FLAG(line.flags, UDMF_ML_PLAYERCROSS);
    }
    else if (!stricmp(scanner.string, "playeruse")) {
      SCAN_FLAG(line.flags, UDMF_ML_PLAYERUSE);
    }
    else if (!stricmp(scanner.string, "monstercross")) {
      SCAN_FLAG(line.flags, UDMF_ML_MONSTERCROSS);
    }
    else if (!stricmp(scanner.string, "monsteruse")) {
      SCAN_FLAG(line.flags, UDMF_ML_MONSTERUSE);
    }
    else if (!stricmp(scanner.string, "impact")) {
      SCAN_FLAG(line.flags, UDMF_ML_IMPACT);
    }
    else if (!stricmp(scanner.string, "playerpush")) {
      SCAN_FLAG(line.flags, UDMF_ML_PLAYERPUSH);
    }
    else if (!stricmp(scanner.string, "monsterpush")) {
      SCAN_FLAG(line.flags, UDMF_ML_MONSTERPUSH);
    }
    else if (!stricmp(scanner.string, "missilecross")) {
      SCAN_FLAG(line.flags, UDMF_ML_MISSILECROSS);
    }
    else if (!stricmp(scanner.string, "repeatspecial")) {
      SCAN_FLAG(line.flags, UDMF_ML_REPEATSPECIAL);
    }
    else if (!stricmp(scanner.string, "playeruseback")) {
      SCAN_FLAG(line.flags, UDMF_ML_PLAYERUSEBACK);
    }
    else if (!stricmp(scanner.string, "anycross")) {
      SCAN_FLAG(line.flags, UDMF_ML_ANYCROSS);
    }
    else if (!stricmp(scanner.string, "monsteractivate")) {
      SCAN_FLAG(line.flags, UDMF_ML_MONSTERACTIVATE);
    }
    else if (!stricmp(scanner.string, "blockplayers")) {
      SCAN_FLAG(line.flags, UDMF_ML_BLOCKPLAYERS);
    }
    else if (!stricmp(scanner.string, "blockeverything")) {
      SCAN_FLAG(line.flags, UDMF_ML_BLOCKEVERYTHING);
    }
    else if (!stricmp(scanner.string, "firstsideonly")) {
      SCAN_FLAG(line.flags, UDMF_ML_FIRSTSIDEONLY);
    }
    else if (!stricmp(scanner.string, "zoneboundary")) {
      SCAN_FLAG(line.flags, UDMF_ML_ZONEBOUNDARY);
    }
    else if (!stricmp(scanner.string, "clipmidtex")) {
      SCAN_FLAG(line.flags, UDMF_ML_CLIPMIDTEX);
    }
    else if (!stricmp(scanner.string, "wrapmidtex")) {
      SCAN_FLAG(line.flags, UDMF_ML_WRAPMIDTEX);
    }
    else if (!stricmp(scanner.string, "midtex3d")) {
      SCAN_FLAG(line.flags, UDMF_ML_MIDTEX3D);
    }
    else if (!stricmp(scanner.string, "midtex3dimpassible")) {
      SCAN_FLAG(line.flags, UDMF_ML_MIDTEX3DIMPASSIBLE);
    }
    else if (!stricmp(scanner.string, "checkswitchrange")) {
      SCAN_FLAG(line.flags, UDMF_ML_CHECKSWITCHRANGE);
    }
    else if (!stricmp(scanner.string, "blockprojectiles")) {
      SCAN_FLAG(line.flags, UDMF_ML_BLOCKPROJECTILES);
    }
    else if (!stricmp(scanner.string, "blockuse")) {
      SCAN_FLAG(line.flags, UDMF_ML_BLOCKUSE);
    }
    else if (!stricmp(scanner.string, "blocksight")) {
      SCAN_FLAG(line.flags, UDMF_ML_BLOCKSIGHT);
    }
    else if (!stricmp(scanner.string, "blockhitscan")) {
      SCAN_FLAG(line.flags, UDMF_ML_BLOCKHITSCAN);
    }
    else if (!stricmp(scanner.string, "transparent")) {
      SCAN_FLAG(line.flags, UDMF_ML_TRANSPARENT);
    }
    else if (!stricmp(scanner.string, "revealed")) {
      SCAN_FLAG(line.flags, UDMF_ML_REVEALED);
    }
    else if (!stricmp(scanner.string, "noskywalls")) {
      SCAN_FLAG(line.flags, UDMF_ML_NOSKYWALLS);
    }
    else if (!stricmp(scanner.string, "drawfullheight")) {
      SCAN_FLAG(line.flags, UDMF_ML_DRAWFULLHEIGHT);
    }
    else if (!stricmp(scanner.string, "damagespecial")) {
      SCAN_FLAG(line.flags, UDMF_ML_DAMAGESPECIAL);
    }
    else if (!stricmp(scanner.string, "deathspecial")) {
      SCAN_FLAG(line.flags, UDMF_ML_DEATHSPECIAL);
    }
    else if (!stricmp(scanner.string, "blocklandmonsters")) {
      SCAN_FLAG(line.flags, UDMF_ML_BLOCKLANDMONSTERS);
    }
    else if (!stricmp(scanner.string, "moreids")) {
      SCAN_STRING(line.moreids);
    }
    else {
      // known ignored fields:
      // comment
      // renderstyle
      // arg0str
      dsda_SkipValue(scanner);
    }
  }

  udmf_lines.push_back(line);
}

static void dsda_ParseUDMFSideDef(Scanner &scanner) {
  udmf_side_t side = { 0 };

  side.texturetop[0] = '-';
  side.texturebottom[0] = '-';
  side.texturemiddle[0] = '-';
  side.scalex_top = 1.f;
  side.scaley_top = 1.f;
  side.scalex_mid = 1.f;
  side.scaley_mid = 1.f;
  side.scalex_bottom = 1.f;
  side.scaley_bottom = 1.f;

  scanner.MustGetToken('{');
  while (!scanner.CheckToken('}')) {
    scanner.MustGetToken(TK_Identifier);

    if (!stricmp(scanner.string, "offsetx")) {
      SCAN_INT(side.offsetx);
    }
    else if (!stricmp(scanner.string, "offsety")) {
      SCAN_INT(side.offsety);
    }
    else if (!stricmp(scanner.string, "sector")) {
      SCAN_INT(side.sector);
    }
    else if (!stricmp(scanner.string, "light")) {
      SCAN_INT(side.light);
    }
    else if (!stricmp(scanner.string, "light_top")) {
      SCAN_INT(side.light_top);
    }
    else if (!stricmp(scanner.string, "light_mid")) {
      SCAN_INT(side.light_mid);
    }
    else if (!stricmp(scanner.string, "light_bottom")) {
      SCAN_INT(side.light_bottom);
    }
    else if (!stricmp(scanner.string, "scalex_top")) {
      SCAN_FLOAT(side.scalex_top);
    }
    else if (!stricmp(scanner.string, "scaley_top")) {
      SCAN_FLOAT(side.scaley_top);
    }
    else if (!stricmp(scanner.string, "scalex_mid")) {
      SCAN_FLOAT(side.scalex_mid);
    }
    else if (!stricmp(scanner.string, "scaley_mid")) {
      SCAN_FLOAT(side.scaley_mid);
    }
    else if (!stricmp(scanner.string, "scalex_bottom")) {
      SCAN_FLOAT(side.scalex_bottom);
    }
    else if (!stricmp(scanner.string, "scaley_bottom")) {
      SCAN_FLOAT(side.scaley_bottom);
    }
    else if (!stricmp(scanner.string, "offsetx_top")) {
      SCAN_FLOAT(side.offsetx_top);
    }
    else if (!stricmp(scanner.string, "offsety_top")) {
      SCAN_FLOAT(side.offsety_top);
    }
    else if (!stricmp(scanner.string, "offsetx_mid")) {
      SCAN_FLOAT(side.offsetx_mid);
    }
    else if (!stricmp(scanner.string, "offsety_mid")) {
      SCAN_FLOAT(side.offsety_mid);
    }
    else if (!stricmp(scanner.string, "offsetx_bottom")) {
      SCAN_FLOAT(side.offsetx_bottom);
    }
    else if (!stricmp(scanner.string, "offsety_bottom")) {
      SCAN_FLOAT(side.offsety_bottom);
    }
    else if (!stricmp(scanner.string, "lightabsolute")) {
      SCAN_FLAG(side.flags, UDMF_SF_LIGHTABSOLUTE);
    }
    else if (!stricmp(scanner.string, "lightfog")) {
      SCAN_FLAG(side.flags, UDMF_SF_LIGHTFOG);
    }
    else if (!stricmp(scanner.string, "nofakecontrast")) {
      SCAN_FLAG(side.flags, UDMF_SF_NOFAKECONTRAST);
    }
    else if (!stricmp(scanner.string, "smoothlighting")) {
      SCAN_FLAG(side.flags, UDMF_SF_SMOOTHLIGHTING);
    }
    else if (!stricmp(scanner.string, "clipmidtex")) {
      SCAN_FLAG(side.flags, UDMF_SF_CLIPMIDTEX);
    }
    else if (!stricmp(scanner.string, "wrapmidtex")) {
      SCAN_FLAG(side.flags, UDMF_SF_WRAPMIDTEX);
    }
    else if (!stricmp(scanner.string, "nodecals")) {
      SCAN_FLAG(side.flags, UDMF_SF_NODECALS);
    }
    else if (!stricmp(scanner.string, "lightabsolute_top")) {
      SCAN_FLAG(side.flags, UDMF_SF_LIGHTABSOLUTETOP);
    }
    else if (!stricmp(scanner.string, "lightabsolute_mid")) {
      SCAN_FLAG(side.flags, UDMF_SF_LIGHTABSOLUTEMID);
    }
    else if (!stricmp(scanner.string, "lightabsolute_bottom")) {
      SCAN_FLAG(side.flags, UDMF_SF_LIGHTABSOLUTEBOTTOM);
    }
    else if (!stricmp(scanner.string, "texturetop")) {
      SCAN_STRING_N(side.texturetop, 8);
    }
    else if (!stricmp(scanner.string, "texturebottom")) {
      SCAN_STRING_N(side.texturebottom, 8);
    }
    else if (!stricmp(scanner.string, "texturemiddle")) {
      SCAN_STRING_N(side.texturemiddle, 8);
    }
    else {
      // known ignored fields:
      // comment
      // nogradient_top
      // flipgradient_top
      // clampgradient_top
      // useowncolors_top
      // uppercolor_top
      // lowercolor_top
      // nogradient_mid
      // flipgradient_mid
      // clampgradient_mid
      // useowncolors_mid
      // uppercolor_mid
      // lowercolor_mid
      // nogradient_bottom
      // flipgradient_bottom
      // clampgradient_bottom
      // useowncolors_bottom
      // uppercolor_bottom
      // lowercolor_bottom
      // useowncoloradd_top
      // useowncoloradd_mid
      // useowncoloradd_bottom
      // coloradd_top
      // coloradd_mid
      // coloradd_bottom
      // colorization_top
      // colorization_mid
      // colorization_bottom
      dsda_SkipValue(scanner);
    }
  }

  udmf_sides.push_back(side);
}

static void dsda_ParseUDMFVertex(Scanner &scanner) {
  udmf_vertex_t vertex = { 0 };

  scanner.MustGetToken('{');
  while (!scanner.CheckToken('}')) {
    scanner.MustGetToken(TK_Identifier);

    if (!stricmp(scanner.string, "x")) {
      SCAN_FLOAT_STRING(vertex.x);
    }
    else if (!stricmp(scanner.string, "y")) {
      SCAN_FLOAT_STRING(vertex.y);
    }
    else {
      // known ignored fields:
      // zfloor
      // zceiling
      dsda_SkipValue(scanner);
    }
  }

  udmf_vertices.push_back(vertex);
}

static void dsda_ParseUDMFSector(Scanner &scanner) {
  udmf_sector_t sector = { 0 };

  sector.lightlevel = 160;
  sector.xscalefloor = 1.f;
  sector.yscalefloor = 1.f;
  sector.xscaleceiling = 1.f;
  sector.yscaleceiling = 1.f;
  sector.gravity = "1.0";
  sector.damageinterval = 32;

  scanner.MustGetToken('{');
  while (!scanner.CheckToken('}')) {
    scanner.MustGetToken(TK_Identifier);

    if (!stricmp(scanner.string, "heightfloor")) {
      SCAN_INT(sector.heightfloor);
    }
    else if (!stricmp(scanner.string, "heightceiling")) {
      SCAN_INT(sector.heightceiling);
    }
    else if (!stricmp(scanner.string, "lightlevel")) {
      SCAN_INT(sector.lightlevel);
    }
    else if (!stricmp(scanner.string, "special")) {
      SCAN_INT(sector.special);
    }
    else if (!stricmp(scanner.string, "id")) {
      SCAN_INT(sector.id);
    }
    else if (!stricmp(scanner.string, "lightfloor")) {
      SCAN_INT(sector.lightfloor);
    }
    else if (!stricmp(scanner.string, "lightceiling")) {
      SCAN_INT(sector.lightceiling);
    }
    else if (!stricmp(scanner.string, "damageamount")) {
      SCAN_INT(sector.damageamount);
    }
    else if (!stricmp(scanner.string, "damageinterval")) {
      SCAN_INT(sector.damageinterval);
    }
    else if (!stricmp(scanner.string, "leakiness")) {
      SCAN_INT(sector.leakiness);
    }
    else if (!stricmp(scanner.string, "xpanningfloor")) {
      SCAN_FLOAT(sector.xpanningfloor);
    }
    else if (!stricmp(scanner.string, "ypanningfloor")) {
      SCAN_FLOAT(sector.ypanningfloor);
    }
    else if (!stricmp(scanner.string, "xpanningceiling")) {
      SCAN_FLOAT(sector.xpanningceiling);
    }
    else if (!stricmp(scanner.string, "ypanningceiling")) {
      SCAN_FLOAT(sector.ypanningceiling);
    }
    else if (!stricmp(scanner.string, "xscalefloor")) {
      SCAN_FLOAT(sector.xscalefloor);
    }
    else if (!stricmp(scanner.string, "yscalefloor")) {
      SCAN_FLOAT(sector.yscalefloor);
    }
    else if (!stricmp(scanner.string, "xscaleceiling")) {
      SCAN_FLOAT(sector.xscaleceiling);
    }
    else if (!stricmp(scanner.string, "yscaleceiling")) {
      SCAN_FLOAT(sector.yscaleceiling);
    }
    else if (!stricmp(scanner.string, "rotationfloor")) {
      SCAN_FLOAT(sector.rotationfloor);
    }
    else if (!stricmp(scanner.string, "rotationceiling")) {
      SCAN_FLOAT(sector.rotationceiling);
    }
    else if (!stricmp(scanner.string, "gravity")) {
      SCAN_FLOAT_STRING(sector.gravity);
    }
    else if (!stricmp(scanner.string, "lightfloorabsolute")) {
      SCAN_FLAG(sector.flags, UDMF_SECF_LIGHTFLOORABSOLUTE);
    }
    else if (!stricmp(scanner.string, "lightceilingabsolute")) {
      SCAN_FLAG(sector.flags, UDMF_SECF_LIGHTCEILINGABSOLUTE);
    }
    else if (!stricmp(scanner.string, "silent")) {
      SCAN_FLAG(sector.flags, UDMF_SECF_SILENT);
    }
    else if (!stricmp(scanner.string, "nofallingdamage")) {
      SCAN_FLAG(sector.flags, UDMF_SECF_NOFALLINGDAMAGE);
    }
    else if (!stricmp(scanner.string, "dropactors")) {
      SCAN_FLAG(sector.flags, UDMF_SECF_DROPACTORS);
    }
    else if (!stricmp(scanner.string, "norespawn")) {
      SCAN_FLAG(sector.flags, UDMF_SECF_NORESPAWN);
    }
    else if (!stricmp(scanner.string, "hidden")) {
      SCAN_FLAG(sector.flags, UDMF_SECF_HIDDEN);
    }
    else if (!stricmp(scanner.string, "waterzone")) {
      SCAN_FLAG(sector.flags, UDMF_SECF_WATERZONE);
    }
    else if (!stricmp(scanner.string, "damageterraineffect")) {
      SCAN_FLAG(sector.flags, UDMF_SECF_DAMAGETERRAINEFFECT);
    }
    else if (!stricmp(scanner.string, "damagehazard")) {
      SCAN_FLAG(sector.flags, UDMF_SECF_DAMAGEHAZARD);
    }
    else if (!stricmp(scanner.string, "noattack")) {
      SCAN_FLAG(sector.flags, UDMF_SECF_NOATTACK);
    }
    else if (!stricmp(scanner.string, "texturefloor")) {
      SCAN_STRING_N(sector.texturefloor, 8);
    }
    else if (!stricmp(scanner.string, "textureceiling")) {
      SCAN_STRING_N(sector.textureceiling, 8);
    }
    else if (!stricmp(scanner.string, "moreids")) {
      SCAN_STRING(sector.moreids);
    }
    else {
      // known ignored fields:
      // comment
      // ceilingplane_a
      // ceilingplane_b
      // ceilingplane_c
      // ceilingplane_d
      // floorplane_a
      // floorplane_b
      // floorplane_c
      // floorplane_d
      // alphafloor
      // alphaceiling
      // renderstylefloor
      // renderstyleceiling
      // lightcolor
      // fadecolor
      // desaturation
      // soundsequence
      // damagetype
      // floorterrain
      // ceilingterrain
      // portal_ceil_blocksound
      // portal_ceil_disabled
      // portal_ceil_nopass
      // portal_ceil_norender
      // portal_ceil_overlaytype
      // portal_floor_blocksound
      // portal_floor_disabled
      // portal_floor_nopass
      // portal_floor_norender
      // portal_floor_overlaytype
      // floor_reflect
      // ceiling_reflect
      // fogdensity
      // floorglowcolor
      // floorglowheight
      // ceilingglowcolor
      // ceilingglowheight
      // color_floor
      // color_ceiling
      // color_walltop
      // color_wallbottom
      // color_sprites
      // coloradd_floor
      // coloradd_ceiling
      // coloradd_sprites
      // coloradd_walls
      // colorization_floor
      // colorization_ceiling
      // noskywalls
      // healthfloor
      // healthfloorgroup
      // healthceiling
      // healthceilinggroup
      dsda_SkipValue(scanner);
    }
  }

  udmf_sectors.push_back(sector);
}

static void dsda_ParseUDMFThing(Scanner &scanner) {
  udmf_thing_t thing = { 0 };

  thing.gravity = "1.0";
  thing.health = "1.0";
  thing.floatbobphase = -1;
  thing.alpha = 1.0;

  scanner.MustGetToken('{');
  while (!scanner.CheckToken('}')) {
    scanner.MustGetToken(TK_Identifier);

    if (!stricmp(scanner.string, "id")) {
      SCAN_INT(thing.id);
    }
    else if (!stricmp(scanner.string, "angle")) {
      SCAN_INT(thing.angle);
    }
    else if (!stricmp(scanner.string, "type")) {
      SCAN_INT(thing.type);
    }
    else if (!stricmp(scanner.string, "special")) {
      SCAN_INT(thing.special);
    }
    else if (!stricmp(scanner.string, "arg0")) {
      SCAN_INT(thing.arg0);
    }
    else if (!stricmp(scanner.string, "arg1")) {
      SCAN_INT(thing.arg1);
    }
    else if (!stricmp(scanner.string, "arg2")) {
      SCAN_INT(thing.arg2);
    }
    else if (!stricmp(scanner.string, "arg3")) {
      SCAN_INT(thing.arg3);
    }
    else if (!stricmp(scanner.string, "arg4")) {
      SCAN_INT(thing.arg4);
    }
    else if (!stricmp(scanner.string, "floatbobphase")) {
      SCAN_INT(thing.floatbobphase);
    }
    else if (!stricmp(scanner.string, "x")) {
      SCAN_FLOAT_STRING(thing.x);
    }
    else if (!stricmp(scanner.string, "y")) {
      SCAN_FLOAT_STRING(thing.y);
    }
    else if (!stricmp(scanner.string, "height")) {
      SCAN_FLOAT_STRING(thing.height);
    }
    else if (!stricmp(scanner.string, "gravity")) {
      SCAN_FLOAT_STRING(thing.gravity);
    }
    else if (!stricmp(scanner.string, "health")) {
      SCAN_FLOAT_STRING(thing.health);
    }
    else if (!stricmp(scanner.string, "scalex")) {
      SCAN_FLOAT(thing.scalex);
    }
    else if (!stricmp(scanner.string, "scaley")) {
      SCAN_FLOAT(thing.scaley);
    }
    else if (!stricmp(scanner.string, "scale")) {
      SCAN_FLOAT(thing.scale);
    }
    else if (!stricmp(scanner.string, "alpha")) {
      SCAN_FLOAT(thing.alpha);
    }
    else if (!stricmp(scanner.string, "skill1")) {
      SCAN_FLAG(thing.flags, UDMF_TF_SKILL1);
    }
    else if (!stricmp(scanner.string, "skill2")) {
      SCAN_FLAG(thing.flags, UDMF_TF_SKILL2);
    }
    else if (!stricmp(scanner.string, "skill3")) {
      SCAN_FLAG(thing.flags, UDMF_TF_SKILL3);
    }
    else if (!stricmp(scanner.string, "skill4")) {
      SCAN_FLAG(thing.flags, UDMF_TF_SKILL4);
    }
    else if (!stricmp(scanner.string, "skill5")) {
      SCAN_FLAG(thing.flags, UDMF_TF_SKILL5);
    }
    else if (!stricmp(scanner.string, "ambush")) {
      SCAN_FLAG(thing.flags, UDMF_TF_AMBUSH);
    }
    else if (!stricmp(scanner.string, "single")) {
      SCAN_FLAG(thing.flags, UDMF_TF_SINGLE);
    }
    else if (!stricmp(scanner.string, "dm")) {
      SCAN_FLAG(thing.flags, UDMF_TF_DM);
    }
    else if (!stricmp(scanner.string, "coop")) {
      SCAN_FLAG(thing.flags, UDMF_TF_COOP);
    }
    else if (!stricmp(scanner.string, "friend")) {
      SCAN_FLAG(thing.flags, UDMF_TF_FRIEND);
    }
    else if (!stricmp(scanner.string, "dormant")) {
      SCAN_FLAG(thing.flags, UDMF_TF_DORMANT);
    }
    else if (!stricmp(scanner.string, "class1")) {
      SCAN_FLAG(thing.flags, UDMF_TF_CLASS1);
    }
    else if (!stricmp(scanner.string, "class2")) {
      SCAN_FLAG(thing.flags, UDMF_TF_CLASS2);
    }
    else if (!stricmp(scanner.string, "class3")) {
      SCAN_FLAG(thing.flags, UDMF_TF_CLASS3);
    }
    else if (!stricmp(scanner.string, "standing")) {
      SCAN_FLAG(thing.flags, UDMF_TF_STANDING);
    }
    else if (!stricmp(scanner.string, "strifeally")) {
      SCAN_FLAG(thing.flags, UDMF_TF_STRIFEALLY);
    }
    else if (!stricmp(scanner.string, "translucent")) {
      SCAN_FLAG(thing.flags, UDMF_TF_TRANSLUCENT);
    }
    else if (!stricmp(scanner.string, "invisible")) {
      SCAN_FLAG(thing.flags, UDMF_TF_INVISIBLE);
    }
    else if (!stricmp(scanner.string, "countsecret")) {
      SCAN_FLAG(thing.flags, UDMF_TF_COUNTSECRET);
    }
    else {
      // known ignored fields:
      // comment
      // skill6-16
      // class4-16
      // conversation
      // arg0str
      // renderstyle
      // fillcolor
      // score
      // pitch
      // roll
      dsda_SkipValue(scanner);
    }
  }

  udmf_things.push_back(thing);
}

static void dsda_ParseUDMFIdentifier(Scanner &scanner) {
  scanner.MustGetToken(TK_Identifier);

  if (!stricmp(scanner.string, "namespace")) {
    scanner.MustGetToken('=');
    scanner.MustGetToken(TK_StringConst);

    if (stricmp(scanner.string, "zdoom") && stricmp(scanner.string, "dsda"))
      scanner.ErrorF("Unknown UDMF namespace \"%s\"", scanner.string);

    scanner.MustGetToken(';');
  }
  else if (!stricmp(scanner.string, "linedef")) {
    dsda_ParseUDMFLineDef(scanner);
  }
  else if (!stricmp(scanner.string, "sidedef")) {
    dsda_ParseUDMFSideDef(scanner);
  }
  else if (!stricmp(scanner.string, "vertex")) {
    dsda_ParseUDMFVertex(scanner);
  }
  else if (!stricmp(scanner.string, "sector")) {
    dsda_ParseUDMFSector(scanner);
  }
  else if (!stricmp(scanner.string, "thing")) {
    dsda_ParseUDMFThing(scanner);
  }
  else {
    dsda_SkipValue(scanner);
  }
}

udmf_t udmf;

void dsda_ParseUDMF(const unsigned char* buffer, size_t length, udmf_errorfunc err) {
  Scanner scanner((const char*) buffer, length);

  scanner.SetErrorCallback(err);

  udmf_lines.clear();
  udmf_sides.clear();
  udmf_vertices.clear();
  udmf_sectors.clear();
  udmf_things.clear();

  while (scanner.TokensLeft())
    dsda_ParseUDMFIdentifier(scanner);

  if (
    udmf_lines.empty() ||
    udmf_sides.empty() ||
    udmf_vertices.empty() ||
    udmf_sectors.empty() ||
    udmf_things.empty()
  )
    scanner.ErrorF("Insufficient UDMF data");

  udmf.lines = &udmf_lines[0];
  udmf.num_lines = udmf_lines.size();

  udmf.sides = &udmf_sides[0];
  udmf.num_sides = udmf_sides.size();

  udmf.vertices = &udmf_vertices[0];
  udmf.num_vertices = udmf_vertices.size();

  udmf.sectors = &udmf_sectors[0];
  udmf.num_sectors = udmf_sectors.size();

  udmf.things = &udmf_things[0];
  udmf.num_things = udmf_things.size();
}
