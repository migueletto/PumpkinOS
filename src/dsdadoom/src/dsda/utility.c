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
//	DSDA Utility
//

#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "p_maputl.h"
#include "r_main.h"
#include "z_zone.h"

#include "utility.h"
#include "sys.h"

void dsda_InitString(dsda_string_t* dest, const char* value) {
  dest->size = 1; // \0
  dest->string = NULL;

  if (value)
    dsda_StringCat(dest, value);
}

void dsda_FreeString(dsda_string_t* dest) {
  Z_Free(dest->string);
  dsda_InitString(dest, NULL);
}

static void dsda_ExpandString(dsda_string_t* dest, size_t size) {
  dest->size += size;
  if (dest->string)
    dest->string = Z_Realloc(dest->string, dest->size);
  else
    dest->string = Z_Calloc(dest->size, 1);
}

void dsda_StringCat(dsda_string_t* dest, const char* source) {
  if (!source || (!source[0] && dest->string))
    return;

  dsda_ExpandString(dest, strlen(source));
  strcat(dest->string, source);
}

void dsda_StringCatF(dsda_string_t* dest, const char* format, ...) {
  size_t length;
  va_list v;

  va_start(v, format);
  length = vsnprintf(NULL, 0, format, v);
  va_end(v);

  dsda_ExpandString(dest, length);

  va_start(v, format);
  vsnprintf(dest->string + dest->size - 1 - length, length + 1, format, v);
  va_end(v);
}

void dsda_StringPrintF(dsda_string_t* dest, const char* format, ...) {
  size_t length;
  va_list v;

  dsda_InitString(dest, NULL);

  va_start(v, format);
  length = vsnprintf(NULL, 0, format, v);
  va_end(v);

  dsda_ExpandString(dest, length);

  va_start(v, format);
  vsnprintf(dest->string + dest->size - 1 - length, length + 1, format, v);
  va_end(v);
}

void dsda_UppercaseString(char* str) {
  for (; *str; str++) *str = toupper(*str);
}

void dsda_TranslateCheckSum(dsda_cksum_t* cksum) {
  unsigned int i;

  for (i = 0; i < 16; i++)
    sprintf(&cksum->string[i * 2], "%02x", cksum->bytes[i]);
  cksum->string[32] = '\0';
}

dboolean dsda_HasFileExt(const char* file, const char* ext) {
  return strlen(file) > strlen(ext) &&
         !strcasecmp(file + strlen(file) - strlen(ext), ext);
}

char** dsda_SplitString(char* str, const char* delimiter) {
  char** result;
  int substring_count = 2;
  char* p = str;

  while (*p)
    if (*p++ == *delimiter)
      ++substring_count;

  result = Z_Calloc(substring_count, sizeof(*result));

  if (result) {
    char* token;
    int i = 0;

    token = strtok(str, delimiter);
    while (token) {
      result[i++] = token;
      token = strtok(NULL, delimiter);
    }
  }

  return result;
}

dsda_fixed_t dsda_SplitFixed(fixed_t x) {
  dsda_fixed_t result;

  result.negative = x < 0;
  result.base = x >> FRACBITS;
  result.frac = x & 0xffff;

  if (result.negative)
    if (result.frac) {
      ++result.base;
      result.frac = 0xffff - result.frac + 1;
    }

  return result;
}

void dsda_FixedToString(char* str, fixed_t x) {
  dsda_fixed_t value;

  value = dsda_SplitFixed(x);

  if (value.frac) {
    if (value.negative && !value.base)
      snprintf(str, FIXED_STRING_LENGTH, "-%i.%05i", value.base, value.frac);
    else
      snprintf(str, FIXED_STRING_LENGTH, "%i.%05i", value.base, value.frac);
  }
  else
    snprintf(str, FIXED_STRING_LENGTH, "%i", value.base);
}

dsda_angle_t dsda_SplitAngle(angle_t x) {
  dsda_angle_t result;

  result.base = x >> 24;
  result.frac = (x >> 16) & 0xff;

  return result;
}

void dsda_PrintCommandMovement(char* str, ticcmd_t* cmd) {
  str[0] = '\0';

  if (cmd->forwardmove > 0)
    str += sprintf(str, "MF%2d ", cmd->forwardmove);
  else if (cmd->forwardmove < 0)
    str += sprintf(str, "MB%2d ", -cmd->forwardmove);
  else
    str += sprintf(str, "     ");

  if (cmd->sidemove > 0)
    str += sprintf(str, "SR%2d ", cmd->sidemove);
  else if (cmd->sidemove < 0)
    str += sprintf(str, "SL%2d ", -cmd->sidemove);
  else
    str += sprintf(str, "     ");

  if (cmd->angleturn > 0)
    str += sprintf(str, "TL%2d", cmd->angleturn >> 8);
  else if (cmd->angleturn < 0)
    str += sprintf(str, "TR%2d", -(cmd->angleturn >> 8));
}

void dsda_CutExtension(char* str) {
  char* p;

  p = str + strlen(str);
  while (--p > str && *p != '/' && *p != '\\')
    if (*p == '.') {
      *p = '\0';
      break;
    }
}

const char* dsda_BaseName(const char* str)
{
  const char* p;

  p = str + strlen(str);
  while (p > str && *(p - 1) != '/' && *(p - 1) != '\\')
    --p;

  return p;
}

const char* dsda_FileExtension(const char* str)
{
  const char* p;

  p = str + strlen(str);
  while (p > str && *(p - 1) != '.')
    --p;

  return p == str ? NULL : p;
}

static double dsda_DistanceLF(double x1, double y1, double x2, double y2) {
  return sys_sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

double dsda_DistancePointToLine(fixed_t line_x1, fixed_t line_y1,
                                fixed_t line_x2, fixed_t line_y2,
                                fixed_t point_x, fixed_t point_y) {
  double dx, dy;
  double x1, x2, y1, y2, px, py;
  double intersect, intersect_x, intersect_y;

  x1 = (double) line_x1 / FRACUNIT;
  x2 = (double) line_x2 / FRACUNIT;
  y1 = (double) line_y1 / FRACUNIT;
  y2 = (double) line_y2 / FRACUNIT;
  px = (double) point_x / FRACUNIT;
  py = (double) point_y / FRACUNIT;

  if (x1 == x2 && y1 == y2)
    return dsda_DistanceLF(x1, y1, px, py);

  dx = x2 - x1;
  dy = y2 - y1;

  intersect = ((px - x1) * dx + (py - y1) * dy) / (dx * dx + dy * dy);
  intersect = BETWEEN(0, 1, intersect);
  intersect_x = x1 + intersect * dx;
  intersect_y = y1 + intersect * dy;

  return dsda_DistanceLF(intersect_x, intersect_y, px, py);
}

angle_t dsda_IntersectionAngle(fixed_t x, fixed_t y,
                               fixed_t x1, fixed_t y1,
                               fixed_t x2, fixed_t y2) {
  angle_t angle;

  angle = R_PointToAngleEx2(x, y, x2, y2) - R_PointToAngleEx2(x, y, x1, y1);

  return (angle > ANG180) ? (ANGLE_MAX - angle + 1) : angle;
}

fixed_t dsda_FixedDistancePointToLine(fixed_t line_x1, fixed_t line_y1,
                                      fixed_t line_x2, fixed_t line_y2,
                                      fixed_t point_x, fixed_t point_y,
                                      fixed_t *closest_x, fixed_t *closest_y) {
  angle_t angle;
  fixed_t line_length;
  fixed_t distance_to_v1;
  fixed_t distance_to_v2;
  fixed_t distance_along_line;
  fixed_t distance_ratio;

  line_length = P_AproxDistance(line_x2 - line_x1, line_y2 - line_y1);
  distance_to_v1 = P_AproxDistance(point_x - line_x1, point_y - line_y1);

  angle = dsda_IntersectionAngle(line_x1, line_y1, line_x2, line_y2, point_x, point_y);

  if (angle > ANG90) {
    *closest_x = line_x1;
    *closest_y = line_y1;

    return distance_to_v1;
  }

  distance_along_line = FixedMul(distance_to_v1, finecosine[angle >> ANGLETOFINESHIFT]);

  if (distance_along_line > line_length)
  {
    distance_to_v2 = P_AproxDistance(point_x - line_x2, point_y - line_y2);

    *closest_x = line_x2;
    *closest_y = line_y2;

    return distance_to_v2;
  }

  distance_ratio = FixedDiv(distance_along_line, line_length);

  *closest_x = FixedMul(line_x2 - line_x1, distance_ratio) + line_x1;
  *closest_y = FixedMul(line_y2 - line_y1, distance_ratio) + line_y1;

  return P_AproxDistance(point_x - *closest_x, point_y - *closest_y);
}

fixed_t dsda_FloatToFixed(float x)
{
  return (fixed_t) (x * FRACUNIT);
}

// Scanning a float is a lossy process, so we must go directly from string to fixed
fixed_t dsda_StringToFixed(const char* x)
{
  int i;
  dboolean negative;
  fixed_t result;
  char frac[4] = { 0 };

  if (!x)
    return 0;

  result = 0;

  sscanf(x, "%d.%3s", &result, frac);
  negative = (x && x[0] == '-');
  result = abs(result);
  result <<= FRACBITS;

  for (i = 0; i < 3; ++i)
    if (!frac[i])
      frac[i] = '0';

  result += (fixed_t) ((int64_t) atoi(frac) * FRACUNIT / 1000);

  return negative ? -result : result;
}

byte dsda_FloatToPercent(float x)
{
  float flr;

  if (x > 1.f)
    x = 1.f;

  if (x < 0.f)
    x = 0.f;

  flr = floorf(x * 100);

  return (byte) flr;
}

int dsda_IntToFixed(int x)
{
  return (fixed_t) (x << FRACBITS);
}

// ANG1 is off by 256 / 360 due to rounding
angle_t dsda_DegreesToAngle(float x)
{
  return ANG1 * x + 256 * x / 360;
}
