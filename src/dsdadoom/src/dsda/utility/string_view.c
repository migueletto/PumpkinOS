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
//	DSDA Utility String View
//

#include <string.h>

#include "string_view.h"

void dsda_InitStringView(dsda_string_view_t* sv, const char* string, size_t size) {
  sv->string = string;
  sv->size = size;
}

dboolean dsda_IsStringViewEmpty(const dsda_string_view_t* sv) {
  return sv->size == 0;
}

void dsda_StringViewAtOffset(const dsda_string_view_t* sv, size_t offset,
                               dsda_string_view_t* ofs) {
  if (offset > sv->size) {
    dsda_InitStringView(ofs, NULL, 0);
    return;
  }

  ofs->string = sv->string + offset;
  ofs->size = sv->size - offset;
}

dboolean dsda_SplitStringViewAfterChar(const dsda_string_view_t* sv, char c,
                                       dsda_string_view_t* before,
                                       dsda_string_view_t* after)
{
  const char* cur;
  size_t size;

  for (cur = sv->string, size = sv->size; size; ++cur, --size)
    if (*cur == c) {
      if (before)
        dsda_InitStringView(before, sv->string, sv->size - size + 1);

      if (after)
        dsda_InitStringView(after, cur + 1, size - 1);

      return true;
    }

  if (before)
    dsda_InitStringView(before, sv->string, sv->size);

  if (after)
    dsda_InitStringView(after, NULL, 0);

  return false;
}

dboolean dsda_SplitStringViewBeforeChar(const dsda_string_view_t* sv, char c,
                                        dsda_string_view_t* before,
                                        dsda_string_view_t* after)
{
  const char* cur;
  size_t size;

  for (cur = sv->string, size = sv->size; size; ++cur, --size)
    if (*cur == c) {
      if (before)
        dsda_InitStringView(before, sv->string, sv->size - size);

      if (after)
        dsda_InitStringView(after, cur, size);

      return true;
    }

  if (before)
    dsda_InitStringView(before, sv->string, sv->size);

  if (after)
    dsda_InitStringView(after, NULL, 0);

  return false;
}

dboolean dsda_GetStringViewLine(dsda_string_view_t* sv, dsda_string_view_t* line) {
  if (dsda_IsStringViewEmpty(sv)) {
    dsda_InitStringView(line, NULL, 0);

    return false;
  }

  dsda_SplitStringViewAfterChar(sv, '\n', line, sv);

  return true;
}

dboolean dsda_StringViewStartsWith(const dsda_string_view_t* sv, const char* prefix) {
  size_t len;

  len = strlen(prefix);

  return sv->size >= len && !memcmp(sv->string, prefix, len);
}

void dsda_StringViewAfterChars(const dsda_string_view_t* sv,
                               const char* chars, dsda_string_view_t* after)
{
  const char* cur;
  size_t size;

  for (cur = sv->string, size = sv->size; size && strchr(chars, *cur); ++cur, --size);

  dsda_InitStringView(after, cur, size);
}
