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
//	DSDA Demo
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "doomtype.h"
#include "doomstat.h"
#include "g_game.h"
#include "m_file.h"
#include "md5.h"
#include "lprintf.h"
#include "e6y.h"
#include "p_saveg.h"

#include "dsda.h"
#include "dsda/args.h"
#include "dsda/configuration.h"
#include "dsda/data_organizer.h"
#include "dsda/excmd.h"
#include "dsda/exdemo.h"
#include "dsda/features.h"
#include "dsda/key_frame.h"
#include "dsda/map_format.h"
#include "dsda/settings.h"
#include "dsda/split_tracker.h"
#include "dsda/utility.h"

#include "demo.h"

#define INITIAL_DEMO_BUFFER_SIZE 0x20000

static char* dsda_demo_name_base;
static byte* dsda_demo_write_buffer;
static byte* dsda_demo_write_buffer_p;
static int dsda_demo_write_buffer_length;
static int dsda_extra_demo_header_data_offset;
static int largest_real_offset;
static int demo_tics;
static int compatibility_level_unspecified;

#define DSDA_DEMO_VERSION 2
#define DSDA_DEMO_HEADER_START_SIZE 8 // version + signature (6) + dsda version

const int dsda_demo_header_data_size[DSDA_DEMO_VERSION + 1] = { 0, 8, 9 };

typedef struct {
  int end_marker_location;
  int demo_tics;
  byte flags;
} dsda_demo_header_data_t;

static dsda_demo_header_data_t dsda_demo_header_data;

#define DEMOMARKER 0x80

#define DF_FROM_KEYFRAME   0x01
#define DF_CASUAL_FEATURES 0x02

static dboolean join_queued;
static int dsda_demo_version;
static int bytes_per_tic;

static dboolean use_demo_name_with_time;

int dsda_DemoTic(void) {
  return demo_tics;
}

const char* dsda_DemoNameBase(void) {
  return dsda_demo_name_base;
}

void dsda_SetDemoBaseName(const char* name) {
  size_t base_size;

  if (dsda_demo_name_base)
    Z_Free(dsda_demo_name_base);

  dsda_demo_name_base = Z_Strdup(name);

  dsda_CutExtension(dsda_demo_name_base);

  base_size = strlen(dsda_demo_name_base);
  if (base_size && dsda_demo_name_base[base_size - 1] == '$') {
    dsda_demo_name_base[base_size - 1] = '\0';
    use_demo_name_with_time = true;
  }
  else
    use_demo_name_with_time = false;
}

// from crispy - incrementing demo file names
char* dsda_GenerateDemoName(unsigned int* counter, const char* base_name) {
  char* demo_name;
  size_t demo_name_size;
  int j;

  j = *counter;
  demo_name_size = strlen(base_name) + 11; // 11 = -12345.lmp\0
  demo_name = Z_Malloc(demo_name_size);
  snprintf(demo_name, demo_name_size, "%s.lmp", base_name);

  for (; j <= 99999 && M_FileExists(demo_name); j++)
    snprintf(demo_name, demo_name_size, "%s-%05d.lmp", base_name, j);

  *counter = j;

  return demo_name;
}

#define ADD_FILE_COUNTER static unsigned int counter; \
                         DO_ONCE \
                           counter = dsda_DemoAttempts(); \
                           if (counter < 2) \
                             counter = 2; \
                         END_ONCE

char* dsda_FallbackDemoName(void) {
  ADD_FILE_COUNTER

  dsda_SetDemoBaseName("fallback");

  return dsda_GenerateDemoName(&counter, dsda_demo_name_base);
}

char* dsda_NewDemoName(void) {
  ADD_FILE_COUNTER

  if (!dsda_demo_name_base)
    dsda_SetDemoBaseName("null");

  return dsda_GenerateDemoName(&counter, dsda_demo_name_base);
}

static dboolean dsda_UseFailedDemoName(void) {
  return dsda_IntConfig(dsda_config_organize_failed_demos) &&
         !dsda_ILComplete() && !dsda_MovieComplete();
}

char* dsda_FailedDemoName(void) {
  static char* dsda_failed_demo_name_base;
  ADD_FILE_COUNTER

  if (!dsda_demo_name_base)
    dsda_SetDemoBaseName("null");

  if (!dsda_failed_demo_name_base) {
    dsda_string_t str;

    dsda_StringPrintF(&str, "%s/failed_demos", dsda_DataDir());
    M_MakeDir(str.string, false); // false: it's ok to fail here
    dsda_StringCatF(&str, "/%s", dsda_BaseName(dsda_demo_name_base));

    dsda_failed_demo_name_base = str.string;
  }

  return dsda_GenerateDemoName(&counter, dsda_failed_demo_name_base);
}

static int dsda_DemoBufferOffset(void) {
  return dsda_demo_write_buffer_p - dsda_demo_write_buffer;
}

int dsda_BytesPerTic(void) {
  return bytes_per_tic;
}

void dsda_EvaluateBytesPerTic(void) {
  bytes_per_tic = (longtics ? 5 : 4);
  if (raven) bytes_per_tic += 2;
  if (dsda_ExCmdDemo()) bytes_per_tic++;
}

static void dsda_EnsureDemoBufferSpace(size_t length) {
  int offset;

  offset = dsda_DemoBufferOffset();

  if (offset + length <= dsda_demo_write_buffer_length) return;

  while (offset + length > dsda_demo_write_buffer_length)
    dsda_demo_write_buffer_length *= 2;

  dsda_demo_write_buffer =
    (byte *)Z_Realloc(dsda_demo_write_buffer, dsda_demo_write_buffer_length);

  if (dsda_demo_write_buffer == NULL)
    I_Error("dsda_EnsureDemoBufferSpace: out of memory!");

  dsda_demo_write_buffer_p = dsda_demo_write_buffer + offset;

  lprintf(
    LO_INFO,
    "dsda_EnsureDemoBufferSpace: expanding demo buffer %d\n",
    dsda_demo_write_buffer_length
  );
}

dboolean dsda_CopyPendingCmd(ticcmd_t* cmd, int delta) {
  if (demorecording && largest_real_offset - dsda_DemoBufferOffset() >= (delta + 1) * bytes_per_tic) {
    const byte* p = dsda_demo_write_buffer_p + delta * bytes_per_tic;

    G_ReadOneTick(cmd, &p);

    return true;
  }
  else {
    memset(cmd, 0, sizeof(*cmd));

    return false;
  }
}

void dsda_CopyPriorCmd(ticcmd_t* cmd, int delta) {
  if (demorecording && dsda_DemoBufferOffset() >= delta * bytes_per_tic) {
    const byte* p = dsda_demo_write_buffer_p - delta * bytes_per_tic;

    G_ReadOneTick(cmd, &p);
  }
  else {
    memset(cmd, 0, sizeof(*cmd));
  }
}

void dsda_RestoreCommandHistory(void) {
  extern int dsda_command_history_size;

  ticcmd_t cmd = { 0 };

  // the dsda format has variable bytes_per_tic - ignoring these for now
  if (demorecording && true_logictic && dsda_command_history_size && !dsda_demo_version) {
    const byte* p;
    int count;

    count = MIN(true_logictic, dsda_command_history_size);

    p = dsda_demo_write_buffer_p - bytes_per_tic * count;

    while (p < dsda_demo_write_buffer_p) {
      G_ReadOneTick(&cmd, &p);
      dsda_AddCommandToCommandDisplay(&cmd);
    }
  }
}

void dsda_MarkCompatibilityLevelUnspecified(void) {
  compatibility_level_unspecified = true;
}

void dsda_InitDemoRecording(void) {
  static dboolean demo_key_frame_initialized;

  if (compatibility_level_unspecified)
    I_Error("You must specify a compatibility level when recording a demo!\n"
            "Example: dsda-doom -iwad DOOM -complevel 3 -record demo");

  demorecording = true;

  // Key settings revert when starting a new attempt
  dsda_RevertIntConfig(dsda_config_vertmouse);
  dsda_RevertIntConfig(dsda_config_strict_mode);

  // prboom+ has already cached its settings (with demorecording == false)
  // we need to reset things here to satisfy strict mode
  dsda_InitSettings();

  dsda_LiftInputRestrictions();
  dsda_ResetFeatures();
  dsda_TrackConfigFeatures();

  if (!demo_key_frame_initialized) {
    dsda_InitKeyFrame();
    demo_key_frame_initialized = true;
  }

  dsda_ForgetAutoKeyFrames();

  dsda_demo_write_buffer = Z_Malloc(INITIAL_DEMO_BUFFER_SIZE);
  if (dsda_demo_write_buffer == NULL)
    I_Error("dsda_InitDemo: unable to initialize demo buffer!");

  dsda_demo_write_buffer_p = dsda_demo_write_buffer;

  dsda_demo_write_buffer_length = INITIAL_DEMO_BUFFER_SIZE;

  demo_tics = 0;
}

static void dsda_SetDemoBufferOffset(int offset) {
  int current_offset;

  if (dsda_demo_write_buffer == NULL) return;

  current_offset = dsda_DemoBufferOffset();

  // Cannot load forward (demo buffer would desync)
  if (offset > current_offset)
    I_Error("dsda_SetDemoBufferOffset: Impossible time traveling detected.");

  if (current_offset > largest_real_offset)
    largest_real_offset = current_offset;

  dsda_demo_write_buffer_p = dsda_demo_write_buffer + offset;
}

void dsda_WriteToDemo(const void* buffer, size_t length) {
  dsda_EnsureDemoBufferSpace(length);

  memcpy(dsda_demo_write_buffer_p, buffer, length);
  dsda_demo_write_buffer_p += length;
}

void dsda_WriteQueueToDemo(const void* buffer, size_t length) {
  int old_offset;

  old_offset = dsda_DemoBufferOffset();

  dsda_WriteToDemo(buffer, length);

  dsda_SetDemoBufferOffset(old_offset);
}

void dsda_WriteTicToDemo(const void* buffer, size_t length) {
  dsda_WriteToDemo(buffer, length);
  ++demo_tics;
}

static void dsda_WriteIntToHeader(byte** p, int value) {
  byte* header_p = *p;

  *header_p++ = (byte)((value >> 24) & 0xff);
  *header_p++ = (byte)((value >> 16) & 0xff);
  *header_p++ = (byte)((value >>  8) & 0xff);
  *header_p++ = (byte)( value        & 0xff);

  *p = header_p;
}

static int dsda_ReadIntFromHeader(const byte* p) {
  int result;

  result  = *p++ & 0xff;
  result <<= 8;
  result += *p++ & 0xff;
  result <<= 8;
  result += *p++ & 0xff;
  result <<= 8;
  result += *p++ & 0xff;

  return result;
}

static void dsda_WriteExtraDemoHeaderData(int end_marker_location) {
  byte* header_p;

  if (!dsda_demo_version) return;

  header_p = dsda_demo_write_buffer + dsda_extra_demo_header_data_offset;
  dsda_WriteIntToHeader(&header_p, end_marker_location);
  dsda_WriteIntToHeader(&header_p, demo_tics);
}

static void dsda_SetExtraDemoHeaderFlag(byte flag) {
  byte* header_p;

  header_p = dsda_demo_write_buffer + dsda_extra_demo_header_data_offset;
  header_p += 8; // skip other fields
  *header_p |= flag;
}

dboolean dsda_StartDemoSegment(const char* demo_name) {
  if (demorecording)
    return false;

  dsda_UpdateFlag(dsda_arg_dsdademo, true);
  dsda_SetDemoBaseName(demo_name);
  dsda_InitDemoRecording();
  G_BeginRecording();
  dsda_SetExtraDemoHeaderFlag(DF_FROM_KEYFRAME);

  {
    dsda_key_frame_t key_frame;

    memset(&key_frame, 0, sizeof(key_frame));
    dsda_StoreKeyFrame(&key_frame, false, false);
    dsda_WriteToDemo(&key_frame.buffer_length, sizeof(key_frame.buffer_length));
    dsda_WriteToDemo(key_frame.buffer, key_frame.buffer_length);
    Z_Free(key_frame.buffer);
  }

  dsda_UpdateStrictMode();

  return true;
}

const byte* dsda_EvaluateDemoStartPoint(const byte* demo_p) {
  if (dsda_demo_version && dsda_demo_header_data.flags & DF_FROM_KEYFRAME) {
    dsda_key_frame_t key_frame;
    union
    {
      const byte* cb;
      byte* b;
    } u = { demo_p };

    memset(&key_frame, 0, sizeof(key_frame));

    memcpy(&key_frame.buffer_length, demo_p, sizeof(key_frame.buffer_length));
    demo_p += sizeof(key_frame.buffer_length);

    key_frame.buffer = u.b;

    dsda_RestoreKeyFrame(&key_frame, false);
    demo_p += key_frame.buffer_length;
  }

  return demo_p;
}

void dsda_GetDemoCheckSum(dsda_cksum_t* cksum, byte* features, byte* demo, size_t demo_size) {
  struct MD5Context md5;

  MD5Init(&md5);

  MD5Update(&md5, demo, demo_size);

  MD5Update(&md5, features, FEATURE_SIZE);

  MD5Final(cksum->bytes, &md5);

  dsda_TranslateCheckSum(cksum);
}

void dsda_GetDemoRecordingCheckSum(dsda_cksum_t* cksum) {
  byte features[FEATURE_SIZE];

  dsda_CopyFeatures(features);

  dsda_GetDemoCheckSum(cksum, features, dsda_demo_write_buffer, dsda_DemoBufferOffset());
}

static int dsda_ExportDemoToFile(const char* demo_name) {
  int end_marker_location;
  byte end_marker = DEMOMARKER;
  int length;

  end_marker_location = dsda_demo_write_buffer_p - dsda_demo_write_buffer;

  dsda_WriteToDemo(&end_marker, 1);

  dsda_WriteExtraDemoHeaderData(end_marker_location);

  dsda_WriteExDemoFooter();

  length = dsda_DemoBufferOffset();

  if (!M_WriteFile(demo_name, dsda_demo_write_buffer, length)) {
    char* fallback_file;

    fallback_file = dsda_FallbackDemoName();

    if (!M_WriteFile(fallback_file, dsda_demo_write_buffer, length))
      I_Error("dsda_WriteDemoToFile: Failed to write demo file.");
    else
      I_Error("Bad demo file location: wrote to %s instead!", fallback_file);

    Z_Free(fallback_file);
  }

  lprintf(LO_INFO, "Wrote demo: %s\n", demo_name);

  return end_marker_location;
}

static void dsda_FreeDemoBuffer(void) {
  Z_Free(dsda_demo_write_buffer);
  dsda_demo_write_buffer = NULL;
  dsda_demo_write_buffer_p = NULL;
  dsda_demo_write_buffer_length = 0;
}

static dboolean dsda_UseDemoNameWithTime(void) {
  return use_demo_name_with_time && (dsda_ILComplete() || dsda_MovieComplete());
}

static char* dsda_DemoNameWithTime(void) {
  char* demo_name;
  char* base_name;
  int counter = 2;
  size_t length;

  length = strlen(dsda_demo_name_base) + 16 + 1;

  base_name = Z_Malloc(length);

  if (dsda_ILComplete()) {
    dsda_level_time_t level_time;

    dsda_DecomposeILTime(&level_time);

    if (level_time.m == 0)
      snprintf(base_name, length, "%s%d%02d",
               dsda_demo_name_base, level_time.s, level_time.t);
    else
      snprintf(base_name, length, "%s%d%02d%02d",
               dsda_demo_name_base, level_time.m, level_time.s, level_time.t);
  }
  else {
    dsda_movie_time_t movie_time;

    dsda_DecomposeMovieTime(&movie_time);

    if (movie_time.h)
      snprintf(base_name, length, "%s%d%02d%02d",
               dsda_demo_name_base, movie_time.h, movie_time.m, movie_time.s);
    else if (movie_time.m)
      snprintf(base_name, length, "%s%d%02d",
               dsda_demo_name_base, movie_time.m, movie_time.s);
    else
      snprintf(base_name, length, "%s%03d",
               dsda_demo_name_base, movie_time.s);
  }

  demo_name = dsda_GenerateDemoName((unsigned int *)&counter, base_name);

  Z_Free(base_name);

  return demo_name;
}

void dsda_EndDemoRecording(void) {
  void ResetOverruns(void);

  char* demo_name;

  demorecording = false;

  if (dsda_UseFailedDemoName())
    demo_name = dsda_FailedDemoName();
  else if (dsda_UseDemoNameWithTime())
    demo_name = dsda_DemoNameWithTime();
  else
    demo_name = dsda_NewDemoName();

  dsda_ExportDemoToFile(demo_name);

  dsda_FreeDemoBuffer();

  Z_Free(demo_name);

  ResetOverruns();
}

void dsda_ExportDemo(const char* name) {
  char* demo_name;
  char* base_name;
  int counter = 2;
  int old_offset;

  base_name = Z_Strdup(name);

  dsda_CutExtension(base_name);

  demo_name = dsda_GenerateDemoName((unsigned int *)&counter, base_name);

  old_offset = dsda_ExportDemoToFile(demo_name);

  dsda_SetDemoBufferOffset(old_offset);

  Z_Free(base_name);
  Z_Free(demo_name);

  lprintf(LO_INFO, "Demo recording exported\n");
}

int dsda_DemoDataSize(byte complete) {
  int buffer_size;

  buffer_size = complete ? dsda_DemoBufferOffset() : 0;

  return sizeof(buffer_size) + sizeof(demo_tics) + buffer_size;
}

void dsda_StoreDemoData(byte complete) {
  int demo_write_buffer_offset;

  demo_write_buffer_offset = dsda_DemoBufferOffset();

  P_SAVE_X(demo_write_buffer_offset);
  P_SAVE_X(demo_tics);

  if (complete && demo_write_buffer_offset)
    P_SAVE_SIZE(dsda_demo_write_buffer, demo_write_buffer_offset);
}

void dsda_RestoreDemoData(byte complete) {
  int demo_write_buffer_offset;

  P_LOAD_X(demo_write_buffer_offset);
  P_LOAD_X(demo_tics);

  if (complete && demo_write_buffer_offset) {
    dsda_SetDemoBufferOffset(0);
    dsda_WriteToDemo(save_p, demo_write_buffer_offset);
    save_p += demo_write_buffer_offset;
  }
  else
    dsda_SetDemoBufferOffset(demo_write_buffer_offset);
}

void dsda_QueueJoin(void) {
  join_queued = true;
}

dboolean dsda_PendingJoin(void) {
  if (!join_queued) return false;

  join_queued = 0;
  return true;
}

void dsda_JoinDemoCmd(ticcmd_t* cmd) {
  dsda_TrackFeature(uf_join);

  // Sometimes this bit is not available
  if (
    (demo_compatibility && !prboom_comp[PC_ALLOW_SSG_DIRECT].state) ||
    (cmd->buttons & BT_CHANGE) == 0
  )
    cmd->buttons |= BT_JOIN;
  else
    dsda_QueueJoin();
}

static const byte* dsda_ReadDSDADemoHeader(const byte* demo_p, const byte* header_p, size_t size) {
  dsda_demo_version = 0;

  // 7 = 6 (signature) + 1 (dsda version)
  if (demo_p - header_p + 7 > size)
    return NULL;

  if (*demo_p++ != 0x1d)
    return NULL;

  if (strncmp((const char *) demo_p, "DSDA", 4) != 0)
    return NULL;

  demo_p += 4;

  if (*demo_p++ != 0xe6)
    return NULL;

  dsda_demo_version = *demo_p++;

  if (dsda_demo_version > DSDA_DEMO_VERSION)
    return NULL;

  if (demo_p - header_p + dsda_demo_header_data_size[dsda_demo_version] > size)
    return NULL;

  dsda_demo_header_data.end_marker_location = dsda_ReadIntFromHeader(demo_p);
  demo_p += 4;

  dsda_demo_header_data.demo_tics = dsda_ReadIntFromHeader(demo_p);
  demo_p += 4;

  if (dsda_demo_version >= 2)
    dsda_demo_header_data.flags = *demo_p++;
  else
    dsda_demo_header_data.flags = 0;

  dsda_EnableExCmd();

  if (dsda_demo_header_data.flags & DF_CASUAL_FEATURES)
    dsda_EnableCasualExCmdFeatures();

  return demo_p;
}

// Strip off the defunct extended header (if we understand it) or abort (if we don't)
static const byte* dsda_ReadUMAPINFODemoHeader(const byte* demo_p, const byte* header_p, size_t size) {
  // 9 = 6 (signature) + 1 (version) + 2 (extension count)
  if (demo_p - header_p + 9 > size)
    return NULL;

  if (strncmp((const char *)demo_p, "PR+UM", 5) != 0)
    I_Error("G_ReadDemoHeader: Unknown demo format");

  demo_p += 6;

  // the defunct format had only version 1
  if (*demo_p++ != 1)
    I_Error("G_ReadDemoHeader: Unknown demo format");

  // the defunct format had only one extension (in two bytes)
  if (*demo_p++ != 1 || *demo_p++ != 0)
    I_Error("G_ReadDemoHeader: Unknown demo format");

  if (demo_p - header_p + 1 > size)
    return NULL;

  // the defunct extension had length 8
  if (*demo_p++ != 8)
    I_Error("G_ReadDemoHeader: Unknown demo format");

  if (demo_p - header_p + 8 > size)
    return NULL;

  if (strncmp((const char *)demo_p, "UMAPINFO", 8))
    I_Error("G_ReadDemoHeader: Unknown demo format");

  demo_p += 8;

  // the defunct extension stored the map lump (unused)
  if (demo_p - header_p + 8 > size)
    return NULL;

  demo_p += 8;

  return demo_p;
}

const byte* dsda_StripDemoVersion255(const byte* demo_p, const byte* header_p, size_t size) {
  const byte* dsda_p;

  dsda_p = dsda_ReadDSDADemoHeader(demo_p, header_p, size);

  if (dsda_p) return dsda_p;

  return dsda_ReadUMAPINFODemoHeader(demo_p, header_p, size);
}

void dsda_WriteDSDADemoHeader(byte** p) {
  byte* demo_p = *p;

  *demo_p++ = 255;

  // signature
  *demo_p++ = 0x1d;
  *demo_p++ = 'D';
  *demo_p++ = 'S';
  *demo_p++ = 'D';
  *demo_p++ = 'A';
  *demo_p++ = 0xe6;

  *demo_p++ = DSDA_DEMO_VERSION;

  dsda_demo_version = DSDA_DEMO_VERSION;
  dsda_extra_demo_header_data_offset = demo_p - *p;
  memset(demo_p, 0, dsda_demo_header_data_size[dsda_demo_version]);

  if (dsda_AllowCasualExCmdFeatures())
    demo_p[8] |= DF_CASUAL_FEATURES;

  demo_p += dsda_demo_header_data_size[dsda_demo_version];

  *p = demo_p;
}

void dsda_ApplyDSDADemoFormat(byte** demo_p) {
  dboolean use_dsda_format = false;

  if (map_format.zdoom)
  {
    if (!dsda_Flag(dsda_arg_baddemo))
      I_Error("Experimental formats require the -baddemo option to record.");

    if (!mbf21)
      I_Error("You must use complevel 21 when recording on doom-in-hexen format.");

    use_dsda_format = true;
  }

  if (dsda_Flag(dsda_arg_dsdademo))
  {
    use_dsda_format = true;
    dsda_EnableCasualExCmdFeatures();
  }

  if (use_dsda_format)
  {
    dsda_EnableExCmd();
    dsda_WriteDSDADemoHeader(demo_p);
  }
}

int dsda_DemoTicsCount(const byte* p, const byte* demobuffer, int demolength) {
  int count = 0;
  extern int demo_playerscount;

  if (dsda_demo_version)
    return dsda_demo_header_data.demo_tics;

  do {
    count++;
    p += bytes_per_tic;
  } while ((p < demobuffer + demolength) && (*p != DEMOMARKER));

  return count / demo_playerscount;
}

const byte* dsda_DemoMarkerPosition(byte* buffer, size_t file_size) {
  const byte* p;

  // read demo header
  p = G_ReadDemoHeaderEx(buffer, file_size, RDH_SKIP_HEADER);

  if (dsda_demo_version) {
    p = (const byte*) (buffer + dsda_demo_header_data.end_marker_location);

    if (*p != DEMOMARKER)
      return NULL;

    return p;
  }

  // skip demo data
  while (p < buffer + file_size && *p != DEMOMARKER)
    p += bytes_per_tic;

  if (*p != DEMOMARKER)
    return NULL;

  return p;
}
