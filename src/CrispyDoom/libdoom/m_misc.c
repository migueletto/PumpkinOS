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
// DESCRIPTION:
//      Miscellaneous.
//


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <io.h>
#ifdef _MSC_VER
#include <direct.h>
#endif
#else
#include <sys/types.h>
#endif

#include "doomtype.h"

#include "deh_str.h"

#include "i_swap.h"
#include "i_system.h"
#include "i_video.h"
#include "m_misc.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"

#ifdef _WIN32
static wchar_t *ConvertMultiByteToWide(const char *str, UINT code_page)
{
    wchar_t *wstr = NULL;
    int wlen = 0;

    wlen = MultiByteToWideChar(code_page, 0, str, -1, NULL, 0);

    if (!wlen)
    {
        errno = EINVAL;
        printf("ConvertMultiByteToWide: Failed to convert path to wide encoding.\n");
        return NULL;
    }

    wstr = malloc(sizeof(wchar_t) * wlen);

    if (!wstr)
    {
        I_Error("ConvertMultiByteToWide: Failed to allocate new string.");
        return NULL;
    }

    if (MultiByteToWideChar(code_page, 0, str, -1, wstr, wlen) == 0)
    {
        errno = EINVAL;
        printf("ConvertMultiByteToWide: Failed to convert path to wide encoding.\n");
        free(wstr);
        return NULL;
    }

    return wstr;
}

static char *ConvertWideToMultiByte(const wchar_t *wstr, UINT code_page)
{
    char *str = NULL;
    int len = 0;

    len = WideCharToMultiByte(code_page, 0, wstr, -1, NULL, 0, NULL, NULL);

    if (!len)
    {
        errno = EINVAL;
        printf("ConvertWideToMultiByte: Failed to convert path to multibyte encoding.\n");
        return NULL;
    }

    str = malloc(sizeof(char) * len);

    if (!str)
    {
        I_Error("ConvertWideToMultiByte: Failed to allocate new string.");
        return NULL;
    }

    if (WideCharToMultiByte(code_page, 0, wstr, -1, str, len, NULL, NULL) == 0)
    {
        errno = EINVAL;
        printf("ConvertWideToMultiByte: Failed to convert path to multibyte encoding.\n");
        free(str);
        return NULL;
    }

    return str;
}

// Convert wide string to a UTF8 string. The result is newly allocated and must
// be freed by the caller after use.

char *M_ConvertWideToUtf8(const wchar_t *wstr)
{
    return ConvertWideToMultiByte(wstr, CP_UTF8);
}

static wchar_t *ConvertSysNativeMBToWide(const char *str)
{
    return ConvertMultiByteToWide(str, CP_ACP);
}

static char *ConvertWideToSysNativeMB(const wchar_t *wstr)
{
    return ConvertWideToMultiByte(wstr, CP_ACP);
}

// Convert UTF8 string to a wide string. The result is newly allocated and must
// be freed by the caller after use.

wchar_t *M_ConvertUtf8ToWide(const char *str)
{
    return ConvertMultiByteToWide(str, CP_UTF8);
}
#endif

// Convert multibyte string in system encoding to UTF8. The result is newly
// allocated and must be freed by the caller after use.

char *M_ConvertSysNativeMBToUtf8(const char *str)
{
#ifdef _WIN32
    char *ret = NULL;
    wchar_t *wstr = NULL;

    wstr = ConvertSysNativeMBToWide(str);

    if (!wstr)
    {
        return NULL;
    }

    ret = M_ConvertWideToUtf8(wstr);

    free(wstr);

    return ret;
#else
    return M_StringDuplicate(str);
#endif
}

// Convert UTF8 string to multibyte string in system encoding. The result is
// newly allocated and must be freed by the caller after use.

char *M_ConvertUtf8ToSysNativeMB(const char *str)
{
#ifdef _WIN32
    char *ret = NULL;
    wchar_t *wstr = NULL;

    wstr = M_ConvertUtf8ToWide(str);

    if (!wstr)
    {
        return NULL;
    }

    ret = ConvertWideToSysNativeMB(wstr);

    free(wstr);

    return ret;
#else
    return M_StringDuplicate(str);
#endif
}

dg_file_t *M_fopen(const char *filename, const char *mode)
{
    return DG_open((char *)filename, mode[0] == 'w' ? 1 : 0);
}

int M_remove(const char *path)
{
    return DG_remove((char *)path);
}

int M_rename(const char *oldname, const char *newname)
{
    return DG_rename((char *)oldname, (char *)newname);
}

int M_stat(const char *path, struct stat *buf)
{
    return stat(path, buf);
}

char *M_getenv(const char *name)
{
    return getenv(name);
}

//
// Create a directory
//

void M_MakeDirectory(const char *path)
{
  DG_mkdir((char *)path);
}

// Check if a file exists

boolean M_FileExists(const char *filename)
{
  dg_file_t *f = DG_open((char *)filename, 0);
  boolean r = f != NULL;
  DG_close(f);
  return r;
}

// Check if a file exists by probing for common case variation of its filename.
// Returns a newly allocated string that the caller is responsible for freeing.

char *M_FileCaseExists(const char *path)
{
    char *path_dup, *filename, *ext;

    path_dup = M_StringDuplicate(path);

    // 0: actual path
    if (M_FileExists(path_dup))
    {
        return path_dup;
    }

    filename = strrchr(path_dup, DIR_SEPARATOR);
    if (filename != NULL)
    {
        filename++;
    }
    else
    {
        filename = path_dup;
    }

    // 1: lowercase filename, e.g. doom2.wad
    M_ForceLowercase(filename);

    if (M_FileExists(path_dup))
    {
        return path_dup;
    }

    // 2: uppercase filename, e.g. DOOM2.WAD
    M_ForceUppercase(filename);

    if (M_FileExists(path_dup))
    {
        return path_dup;
    }

    // 3. uppercase basename with lowercase extension, e.g. DOOM2.wad
    ext = strrchr(path_dup, '.');
    if (ext != NULL && ext > filename)
    {
        M_ForceLowercase(ext + 1);

        if (M_FileExists(path_dup))
        {
            return path_dup;
        }
    }

    // 4. lowercase filename with uppercase first letter, e.g. Doom2.wad
    if (strlen(filename) > 1)
    {
        M_ForceLowercase(filename + 1);

        if (M_FileExists(path_dup))
        {
            return path_dup;
        }
    }

    // 5. no luck
    free(path_dup);
    return NULL;
}

//
// Determine the length of an open file.
//

long M_FileLength(dg_file_t *handle)
{ 
    return DG_filesize(handle);
}

//
// M_WriteFile
//

boolean M_WriteFile(const char *name, const void *source, int length)
{
    dg_file_t *handle;
    int	count;
	
    handle = M_fopen(name, "wb");

    if (handle == NULL)
	return false;

    count = DG_write(handle, (void *)source, length);
    DG_close(handle);
	
    if (count < length)
	return false;
		
    return true;
}


//
// M_ReadFile
//

int M_ReadFile(const char *name, byte **buffer)
{
    dg_file_t *handle;
    int	count, length;
    byte *buf;
	
    handle = M_fopen(name, "rb");
    if (handle == NULL)
	I_Error ("Couldn't read file %s", name);

    // find the size of the file by seeking to the end and
    // reading the current position

    length = M_FileLength(handle);
    
    buf = Z_Malloc (length + 1, PU_STATIC, NULL);
    count = DG_read(handle, buf, length);
    DG_close(handle);
	
    if (count < length)
	I_Error ("Couldn't read file %s", name);
		
    buf[length] = '\0';
    *buffer = buf;
    return length;
}

// Returns the path to a temporary file of the given name, stored
// inside the system temporary directory.
//
// The returned value must be freed with Z_Free after use.

char *M_TempFile(const char *s)
{
    const char *tempdir;
    tempdir = "/";
    return M_StringJoin(tempdir, DIR_SEPARATOR_S, s, NULL);
}

boolean M_StrToInt(const char *str, int *result)
{
    return sscanf(str, " 0x%x", (unsigned int *) result) == 1
        || sscanf(str, " 0X%x", (unsigned int *) result) == 1
        || sscanf(str, " 0%o", (unsigned int *) result) == 1
        || sscanf(str, " %d", result) == 1;
}

// Returns the directory portion of the given path, without the trailing
// slash separator character. If no directory is described in the path,
// the string "." is returned. In either case, the result is newly allocated
// and must be freed by the caller after use.
char *M_DirName(const char *path)
{
    char *result;
    const char *pf, *pb;

    pf = strrchr(path, '/');
#ifdef _WIN32
    pb = strrchr(path, '\\');
#else
    pb = NULL;
#endif
    if (pf == NULL && pb == NULL)
    {
        return M_StringDuplicate(".");
    }
    else
    {
        const char *p = (pb > pf) ? pb : pf;
        result = M_StringDuplicate(path);
        result[p - path] = '\0';
        return result;
    }
}

// Returns the base filename described by the given path (without the
// directory name). The result points inside path and nothing new is
// allocated.
const char *M_BaseName(const char *path)
{
    const char *pf;

    pf = strrchr(path, '/');
    if (pf == NULL)
    {
        return path;
    }
    else
    {
        const char *p = pf;
        return p + 1;
    }
}

void M_ExtractFileBase(const char *path, char *dest)
{
    const char *src;
    const char *filename;
    int length;

    src = path + strlen(path) - 1;

    // back up until a \ or the start
    while (src != path && *(src - 1) != DIR_SEPARATOR)
    {
	src--;
    }

    filename = src;

    // Copy up to eight characters
    // Note: Vanilla Doom exits with an error if a filename is specified
    // with a base of more than eight characters.  To remove the 8.3
    // filename limit, instead we simply truncate the name.

    length = 0;
    memset(dest, 0, 8);

    while (*src != '\0' && *src != '.')
    {
        if (length >= 8)
        {
            printf("Warning: Truncated '%s' lump name to '%.8s'.\n",
                   filename, dest);
            break;
        }

	dest[length++] = toupper((int)*src++);
    }
}

//---------------------------------------------------------------------------
//
// PROC M_ForceUppercase
//
// Change string to uppercase.
//
//---------------------------------------------------------------------------

void M_ForceUppercase(char *text)
{
    char *p;

    for (p = text; *p != '\0'; ++p)
    {
        *p = toupper(*p);
    }
}

//---------------------------------------------------------------------------
//
// PROC M_ForceLowercase
//
// Change string to lowercase.
//
//---------------------------------------------------------------------------

void M_ForceLowercase(char *text)
{
    char *p;

    for (p = text; *p != '\0'; ++p)
    {
        *p = tolower(*p);
    }
}

//
// M_StrCaseStr
//
// Case-insensitive version of strstr()
//

const char *M_StrCaseStr(const char *haystack, const char *needle)
{
    unsigned int haystack_len;
    unsigned int needle_len;
    unsigned int len;
    unsigned int i;

    haystack_len = strlen(haystack);
    needle_len = strlen(needle);

    if (haystack_len < needle_len)
    {
        return NULL;
    }

    len = haystack_len - needle_len;

    for (i = 0; i <= len; ++i)
    {
        if (!strncasecmp(haystack + i, needle, needle_len))
        {
            return haystack + i;
        }
    }

    return NULL;
}

//
// Safe version of strdup() that checks the string was successfully
// allocated.
//

char *M_StringDuplicate(const char *orig)
{
    char *result;

    result = strdup(orig);

    if (result == NULL)
    {
        I_Error("Failed to duplicate string (length %zu)\n",
                strlen(orig));
    }

    return result;
}

//
// String replace function.
//

char *M_StringReplace(const char *haystack, const char *needle,
                      const char *replacement)
{
    char *result, *dst;
    const char *p;
    size_t needle_len = strlen(needle);
    size_t result_len, dst_len;

    // Iterate through occurrences of 'needle' and calculate the size of
    // the new string.
    result_len = strlen(haystack) + 1;
    p = haystack;

    for (;;)
    {
        p = strstr(p, needle);
        if (p == NULL)
        {
            break;
        }

        p += needle_len;
        result_len += strlen(replacement) - needle_len;
    }

    // Construct new string.

    result = malloc(result_len);
    if (result == NULL)
    {
        I_Error("M_StringReplace: Failed to allocate new string");
        return NULL;
    }

    dst = result; dst_len = result_len;
    p = haystack;

    while (*p != '\0')
    {
        if (!strncmp(p, needle, needle_len))
        {
            M_StringCopy(dst, replacement, dst_len);
            p += needle_len;
            dst += strlen(replacement);
            dst_len -= strlen(replacement);
        }
        else
        {
            *dst = *p;
            ++dst; --dst_len;
            ++p;
        }
    }

    *dst = '\0';

    return result;
}

// Safe string copy function that works like OpenBSD's strlcpy().
// Returns true if the string was not truncated.

boolean M_StringCopy(char *dest, const char *src, size_t dest_size)
{
    size_t len;

    if (dest_size >= 1)
    {
        dest[dest_size - 1] = '\0';
        strncpy(dest, src, dest_size - 1);
    }
    else
    {
        return false;
    }

    len = strlen(dest);
    return src[len] == '\0';
}

// Safe string concat function that works like OpenBSD's strlcat().
// Returns true if string not truncated.

boolean M_StringConcat(char *dest, const char *src, size_t dest_size)
{
    size_t offset;

    offset = strlen(dest);
    if (offset > dest_size)
    {
        offset = dest_size;
    }

    return M_StringCopy(dest + offset, src, dest_size - offset);
}

// Returns true if 's' begins with the specified prefix.

boolean M_StringStartsWith(const char *s, const char *prefix)
{
    return strlen(s) >= strlen(prefix)
        && strncmp(s, prefix, strlen(prefix)) == 0;
}

// Returns true if 's' ends with the specified suffix.

boolean M_StringEndsWith(const char *s, const char *suffix)
{
    return strlen(s) >= strlen(suffix)
        && strcmp(s + strlen(s) - strlen(suffix), suffix) == 0;
}

// Return a newly-malloced string with all the strings given as arguments
// concatenated together.

char *M_StringJoin(const char *s, ...)
{
    char *result;
    const char *v;
    va_list args;
    size_t result_len;

    result_len = strlen(s) + 1;

    va_start(args, s);
    for (;;)
    {
        v = va_arg(args, const char *);
        if (v == NULL)
        {
            break;
        }

        result_len += strlen(v);
    }
    va_end(args);

    result = malloc(result_len);

    if (result == NULL)
    {
        I_Error("M_StringJoin: Failed to allocate new string.");
        return NULL;
    }

    M_StringCopy(result, s, result_len);

    va_start(args, s);
    for (;;)
    {
        v = va_arg(args, const char *);
        if (v == NULL)
        {
            break;
        }

        M_StringConcat(result, v, result_len);
    }
    va_end(args);

    return result;
}

// On Windows, vsnprintf() is _vsnprintf().
#ifdef _WIN32
#if _MSC_VER < 1400 /* not needed for Visual Studio 2008 */
#define vsnprintf _vsnprintf
#endif
#endif

// Safe, portable vsnprintf().
int M_vsnprintf(char *buf, size_t buf_len, const char *s, va_list args)
{
    int result;

    if (buf_len < 1)
    {
        return 0;
    }

    // Windows (and other OSes?) has a vsnprintf() that doesn't always
    // append a trailing \0. So we must do it, and write into a buffer
    // that is one byte shorter; otherwise this function is unsafe.
    result = vsnprintf(buf, buf_len, s, args);

    // If truncated, change the final char in the buffer to a \0.
    // A negative result indicates a truncated buffer on Windows.
    if (result < 0 || result >= buf_len)
    {
        buf[buf_len - 1] = '\0';
        result = buf_len - 1;
    }

    return result;
}

// Safe, portable snprintf().
int M_snprintf(char *buf, size_t buf_len, const char *s, ...)
{
    va_list args;
    int result;
    va_start(args, s);
    result = M_vsnprintf(buf, buf_len, s, args);
    va_end(args);
    return result;
}

//
// M_NormalizeSlashes
//
// Remove trailing slashes, translate backslashes to slashes
// The string to normalize is passed and returned in str
//
// killough 11/98: rewritten
//
// [STRIFE] - haleyjd 20110210: Borrowed from Eternity and adapted to respect
// the DIR_SEPARATOR define used by Choco Doom. This routine originated in
// BOOM.
//
void M_NormalizeSlashes(char *str)
{
    char *p;

    // Convert all slashes/backslashes to DIR_SEPARATOR
    for (p = str; *p; p++)
    {
        if ((*p == '/' || *p == '\\') && *p != DIR_SEPARATOR)
        {
            *p = DIR_SEPARATOR;
        }
    }

    // Remove trailing slashes
    while (p > str && *--p == DIR_SEPARATOR)
    {
        *p = 0;
    }

    // Collapse multiple slashes
    for (p = str; (*str++ = *p); )
    {
        if (*p++ == DIR_SEPARATOR)
        {
            while (*p == DIR_SEPARATOR)
            {
                p++;
            }
        }
    }
}
