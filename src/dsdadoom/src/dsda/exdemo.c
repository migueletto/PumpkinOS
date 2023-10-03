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
//	DSDA Extended Demo
//

#include <string.h>

#include "d_main.h"
#include "doomstat.h"
#include "g_overflow.h"
#include "i_system.h"
#include "lprintf.h"
#include "m_argv.h"
#include "m_file.h"
#include "wadtbl.h"
#include "z_zone.h"
#include "e6y.h"

#include "dsda/args.h"
#include "dsda/demo.h"
#include "dsda/features.h"
#include "dsda/playback.h"
#include "dsda/utility.h"

#include "exdemo.h"

typedef struct {
  const char* name;
  byte* demo;
  byte* footer;
  size_t demo_size;
  size_t footer_size;
  uint64_t features;
  int is_signed;
} exdemo_t;

static exdemo_t exdemo;

#define DEMOEX_PORTNAME_LUMPNAME "PORTNAME"
#define DEMOEX_PARAMS_LUMPNAME "CMDLINE"
#define DEMOEX_FEATURE_LUMPNAME "FEATURES"

static void ForgetExDemo(void) {
  if (exdemo.demo)
    Z_Free(exdemo.demo);

  memset(&exdemo, 0, sizeof(exdemo));
}

static const filelump_t* DemoEx_LumpForName(const char* name, const wadinfo_t* header) {
  int i;
  const filelump_t* lump_info;
  const byte* buffer;

  buffer = (const byte*) header;
  lump_info = (const filelump_t*)(buffer + header->infotableofs);
  for (i = 0; i < header->numlumps; i++, lump_info++)
    if (!strncmp(lump_info->name, name, 8))
      return lump_info;

  return NULL;
}

static char* DemoEx_LumpAsString(const char* name, const wadinfo_t* header) {
  char* str;
  const char* lump_data;
  const byte* buffer;
  const filelump_t* lump_info;

  lump_info = DemoEx_LumpForName(name, header);
  if (!lump_info || !lump_info->size)
    return NULL;

  str = Z_Calloc(lump_info->size + 1, 1);

  buffer = (const byte*) header;
  lump_data = (const char *)buffer + lump_info->filepos;
  strncpy(str, lump_data, lump_info->size);

  return str;
}

static void DemoEx_GetParams(const wadinfo_t* header) {
  char* str;
  char** params;
  int i, p, paramscount;

  str = DemoEx_LumpAsString(DEMOEX_PARAMS_LUMPNAME, header);
  if (!str)
    return;

  M_ParseCmdLine(str, NULL, NULL, &paramscount, &i);

  params = Z_Malloc(paramscount * sizeof(char*) + i * sizeof(char) + 1);
  if (params) {
    struct {
      const char* param;
      wad_source_t source;
    } files[] = {
      { "-iwad", source_iwad },
      { "-file", source_pwad },
      { "-deh", source_deh },
      { NULL }
    };

    M_ParseCmdLine(str, params, ((char*) params) + sizeof(char*) * paramscount, &paramscount, &i);

    if (!dsda_Flag(dsda_arg_iwad) && !dsda_Flag(dsda_arg_file)) {
      for (i = 0; files[i].param; ++i) {
        p = M_CheckParmEx(files[i].param, params, paramscount);
        if (p >= 0) {
          while (++p != paramscount && *params[p] != '-') {
            char* filename;

            if (files[i].source == source_deh)
              filename = I_FindDeh(params[p]);
            else
              filename = I_FindWad(params[p]);

            if (!filename)
              continue;

            if (files[i].source == source_iwad)
              AddIWAD(filename);
            else if (files[i].source == source_pwad)
              dsda_AppendStringArg(dsda_arg_file, filename);
            else if (files[i].source == source_deh)
              dsda_AppendStringArg(dsda_arg_deh, filename);

            Z_Free(filename);
          }
        }
      }
    }

    if (!dsda_Arg(dsda_arg_complevel)->found) {
      p = M_CheckParmEx("-complevel", params, paramscount);
      if (p >= 0 && p < (int) paramscount - 1)
        dsda_UpdateIntArg(dsda_arg_complevel, params[p + 1]);
    }

    //for recording or playback using "single-player coop" mode
    if (!dsda_Flag(dsda_arg_solo_net)) {
      p = M_CheckParmEx("-solo-net", params, paramscount);
      if (p >= 0)
        dsda_UpdateFlag(dsda_arg_solo_net, true);
    }

    //for recording or playback using "coop in single-player" mode
    if (!dsda_Flag(dsda_arg_coop_spawns)) {
      p = M_CheckParmEx("-coop_spawns", params, paramscount);
      if (p >= 0)
        dsda_UpdateFlag(dsda_arg_coop_spawns, true);
    }

    //for recording multiple episodes in one demo
    if (!dsda_Flag(dsda_arg_chain_episodes)) {
      p = M_CheckParmEx("-chain_episodes", params, paramscount);
      if (p >= 0)
        dsda_UpdateFlag(dsda_arg_chain_episodes, true);
    }

    if (!dsda_Flag(dsda_arg_emulate)) {
      p = M_CheckParmEx("-emulate", params, paramscount);
      if (p >= 0 && p < (int) paramscount - 1)
        dsda_UpdateStringArg(dsda_arg_emulate, params[p + 1]);
    }

    // for doom 1.2
    if (!dsda_Flag(dsda_arg_respawn)) {
      p = M_CheckParmEx("-respawn", params, paramscount);
      if (p >= 0)
        dsda_UpdateFlag(dsda_arg_respawn, true);
    }

    // for doom 1.2
    if (!dsda_Flag(dsda_arg_fast)) {
      p = M_CheckParmEx("-fast", params, paramscount);
      if (p >= 0)
        dsda_UpdateFlag(dsda_arg_fast, true);
    }

    // for doom 1.2
    if (!dsda_Flag(dsda_arg_nomonsters)) {
      p = M_CheckParmEx("-nomonsters", params, paramscount);
      if (p >= 0)
        dsda_UpdateFlag(dsda_arg_nomonsters, true);
    }

    p = M_CheckParmEx("-spechit", params, paramscount);
    if (p >= 0 && p < (int) paramscount - 1) {
      spechit_baseaddr = atoi(params[p + 1]);
    }

    //overflows
    {
      overrun_list_t overflow;
      for (overflow = 0; overflow < OVERFLOW_MAX; overflow++) {
        int value;
        char* pstr;
        char* mask;

        mask = Z_Malloc(strlen(overflow_cfgname[overflow]) + 16);
        if (mask) {
          sprintf(mask, "-set %s", overflow_cfgname[overflow]);
          pstr = strstr(str, mask);

          if (pstr) {
            strcat(mask, " = %d");
            if (sscanf(pstr, mask, &value) == 1) {
              overflows[overflow].footer = true;
              overflows[overflow].footer_emulate = value;
            }
          }
          Z_Free(mask);
        }
      }
    }

    Z_Free(params);
  }

  Z_Free(str);
}

static void DemoEx_AddParams(wadtbl_t* wadtbl) {
  dsda_arg_t* arg;
  size_t i;
  char buf[200];

  const char* filename_p;

  dsda_string_t files;
  dsda_string_t iwad;
  dsda_string_t pwads;
  dsda_string_t dehs;

  dsda_InitString(&files, NULL);
  dsda_InitString(&iwad, NULL);
  dsda_InitString(&pwads, NULL);
  dsda_InitString(&dehs, NULL);

  for (i = 0; i < numwadfiles; i++) {
    const char* fileext_p;
    dsda_string_t* item = NULL;

    filename_p = PathFindFileName(wadfiles[i].name);
    fileext_p = filename_p + strlen(filename_p) - 1;

    while (fileext_p != filename_p && *(fileext_p - 1) != '.')
      fileext_p--;

    if (fileext_p == filename_p)
      continue;

    if (wadfiles[i].src == source_iwad && !iwad.string && !strcasecmp(fileext_p, "wad"))
      item = &iwad;

    if (wadfiles[i].src == source_pwad && !strcasecmp(fileext_p, "wad"))
      item = &pwads;

    if (item) {
      dsda_StringCat(item, "\"");
      dsda_StringCat(item, filename_p);
      dsda_StringCat(item, "\" ");
    }
  }

  arg = dsda_Arg(dsda_arg_deh);
  if (arg->found) {
    for (i = 0; i < arg->count; ++i) {
      char* file;

      file = I_FindDeh(arg->value.v_string_array[i]);
      if (file) {
        filename_p = PathFindFileName(file);
        dsda_StringCat(&dehs, "\"");
        dsda_StringCat(&dehs, filename_p);
        dsda_StringCat(&dehs, "\" ");
        Z_Free(file);
      }
    }
  }

  if (iwad.string) {
    dsda_StringCat(&files, "-iwad ");
    dsda_StringCat(&files, iwad.string);
  }

  if (pwads.string) {
    dsda_StringCat(&files, "-file ");
    dsda_StringCat(&files, pwads.string);
  }

  if (dehs.string) {
    dsda_StringCat(&files, "-deh ");
    dsda_StringCat(&files, dehs.string);
  }

  // add complevel for formats which do not have it in header
  if (demo_compatibility) {
    sprintf(buf, "-complevel %d ", compatibility_level);
    dsda_StringCat(&files, buf);
  }

  // for recording or playback using "single-player coop" mode
  if (dsda_Flag(dsda_arg_solo_net)) {
    sprintf(buf, "-solo-net ");
    dsda_StringCat(&files, buf);
  }

  // for recording or playback using "coop in single-player" mode
  if (dsda_Flag(dsda_arg_coop_spawns)) {
    sprintf(buf, "-coop_spawns ");
    dsda_StringCat(&files, buf);
  }

  // for recording multiple episodes in one demo
  if (dsda_Flag(dsda_arg_chain_episodes)) {
    sprintf(buf, "-chain_episodes ");
    dsda_StringCat(&files, buf);
  }

  arg = dsda_Arg(dsda_arg_emulate);
  if (arg->found) {
    sprintf(buf, "-emulate %s", arg->value.v_string);
    dsda_StringCat(&files, buf);
  }

  // doom 1.2 does not store these params in header
  if (compatibility_level == doom_12_compatibility) {
    if (dsda_Flag(dsda_arg_respawn)) {
      sprintf(buf, "-respawn ");
      dsda_StringCat(&files, buf);
    }

    if (dsda_Flag(dsda_arg_fast)) {
      sprintf(buf, "-fast ");
      dsda_StringCat(&files, buf);
    }

    if (dsda_Flag(dsda_arg_nomonsters)) {
      sprintf(buf, "-nomonsters ");
      dsda_StringCat(&files, buf);
    }
  }

  if (spechit_baseaddr != 0 && spechit_baseaddr != DEFAULT_SPECHIT_MAGIC) {
    sprintf(buf, "-spechit %d ", spechit_baseaddr);
    dsda_StringCat(&files, buf);
  }

  //overflows
  {
    overrun_list_t overflow;
    for (overflow = 0; overflow < OVERFLOW_MAX; overflow++) {
      if (overflows[overflow].happened) {
        sprintf(buf, "-set %s=%d ", overflow_cfgname[overflow], overflows[overflow].emulate);
        dsda_StringCat(&files, buf);
      }
    }
  }

  if (files.string)
    AddPWADTableLump(wadtbl, DEMOEX_PARAMS_LUMPNAME,
                     (const byte*) files.string, strlen(files.string));

  dsda_FreeString(&files);
  dsda_FreeString(&iwad);
  dsda_FreeString(&pwads);
  dsda_FreeString(&dehs);
}

int dsda_IsExDemoSigned(void) {
  return exdemo.is_signed;
}

void dsda_MergeExDemoFeatures(void) {
  if (!exdemo.is_signed)
    dsda_TrackFeature(uf_unknown);
  else {
    dsda_MergeFeatures(exdemo.features);

    if (exdemo.is_signed == -1)
      dsda_TrackFeature(uf_invalid);
  }
}

static void DemoEx_GetFeatures(const wadinfo_t* header) {
  char* str;
  char signature[33];

  exdemo.is_signed = 0;
  exdemo.features = 0;

  str = DemoEx_LumpAsString(DEMOEX_FEATURE_LUMPNAME, header);
  if (!str)
    return;

  if (sscanf(str, "%*[^\n]\n%" PRIx64 "-%32s", &exdemo.features, signature) == 2) {
    byte features[FEATURE_SIZE];
    dsda_cksum_t cksum;

    dsda_CopyFeatures2(features, exdemo.features);

    dsda_GetDemoCheckSum(&cksum, features, exdemo.demo, exdemo.demo_size);

    if (!strcmp(signature, cksum.string))
      exdemo.is_signed = 1;
    else
      exdemo.is_signed = -1;
  }
  else
    exdemo.is_signed = -1;

  Z_Free(str);
}

static void DemoEx_AddFeatures(wadtbl_t* wadtbl) {
  dsda_cksum_t cksum;
  char* description;
  byte* buffer;
  size_t buffer_length;
  uint64_t features;

  dsda_GetDemoRecordingCheckSum(&cksum);
  description = dsda_DescribeFeatures();
  features = dsda_UsedFeatures();

  // 18 for 64 bits in hex + \n + \0 + \- + extra space :^)
  buffer_length = strlen(cksum.string) + strlen(description) + 24;
  buffer = Z_Calloc(buffer_length, 1);

  snprintf((char *)buffer, buffer_length, "%s\n0x%016" PRIx64 "-%s", description, features, cksum.string);

  AddPWADTableLump(wadtbl, DEMOEX_FEATURE_LUMPNAME, buffer, buffer_length);

  Z_Free(buffer);
  Z_Free(description);
}

static void DemoEx_AddPort(wadtbl_t* wadtbl) {
  AddPWADTableLump(wadtbl, DEMOEX_PORTNAME_LUMPNAME,
                   (const byte*) PACKAGE_STRING, strlen(PACKAGE_STRING));
}

static void PartitionDemo(const char* filename) {
  size_t file_size;

  file_size = M_ReadFile(filename, &exdemo.demo);

  if (file_size > 0) {
    const byte* p;

    p = dsda_DemoMarkerPosition(exdemo.demo, file_size);

    if (p) {
      //skip DEMOMARKER
      p++;

      exdemo.demo_size = p - exdemo.demo;

      //seach for the "PWAD" signature after ENDDEMOMARKER
      while (p - exdemo.demo + sizeof(wadinfo_t) < file_size) {
        if (!memcmp(p, PWAD_SIGNATURE, strlen(PWAD_SIGNATURE))) {
          exdemo.footer = exdemo.demo + (p - exdemo.demo);
          exdemo.footer_size = file_size - (p - exdemo.demo);

          break;
        }
        p++;
      }
    }
    else
    {
      exdemo.demo_size = file_size;
    }
  }
  else
    ForgetExDemo();
}

static void DemoEx_NewLine(wadtbl_t* wadtbl) {
  const char* const separator = "\n";

  AddPWADTableLump(wadtbl, NULL, (const byte*) separator, strlen(separator));
}

static void DemoEx_WritePWADTable(wadtbl_t* wadtbl) {
  dsda_WriteToDemo(&wadtbl->header, sizeof(wadtbl->header));
  dsda_WriteToDemo(wadtbl->data, wadtbl->datasize);
  dsda_WriteToDemo(wadtbl->lumps, wadtbl->header.numlumps * sizeof(wadtbl->lumps[0]));
}

void dsda_WriteExDemoFooter(void) {
  wadtbl_t demoex;

  InitPWADTable(&demoex);

  DemoEx_NewLine(&demoex);
  DemoEx_NewLine(&demoex);

  DemoEx_AddFeatures(&demoex);
  DemoEx_NewLine(&demoex);

  DemoEx_AddPort(&demoex);
  DemoEx_NewLine(&demoex);

  DemoEx_AddParams(&demoex);
  DemoEx_NewLine(&demoex);

  DemoEx_WritePWADTable(&demoex);

  FreePWADTable(&demoex);
}

void dsda_LoadExDemo(const char* filename) {
  PartitionDemo(filename);

  if (exdemo.footer)
  {
    wadinfo_t* header;

    header = ReadPWADTable((char *)exdemo.footer, exdemo.footer_size);

    if (!header)
      lprintf(LO_ERROR, "LoadExDemo: demo footer is corrupted\n");
    else {
      DemoEx_GetFeatures(header);

      // get needed wads and dehs
      // restore all critical params like -spechit x
      DemoEx_GetParams(header);
    }
  }
}

int dsda_CopyExDemo(const byte** buffer, int* length) {
  if (exdemo.demo) {
    *buffer = exdemo.demo;
    *length = exdemo.demo_size;

    return true;
  }

  return false;
}
