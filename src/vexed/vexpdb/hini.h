/*  ini.h "ini file reading support"
    Copyright (C) 2001 Scott Ludwig (scottlu@eskimo.com)
    
    This file is part of Vexed.

    Vexed is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Vexed is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Vexed; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

typedef uint32_t dword;
typedef uint8_t byte;
typedef uint16_t word;

#include <stdarg.h>

struct IniProperty // prop
{
	dword nchProp;
	int cchProp;
	word hashProp;
	dword nchValue;
	int cchValue;
};
typedef struct IniProperty IniProperty;

struct IniSection // sec
{
	dword nchSec;
	int cchSec;
	word hashSec;
	int cprop;
	struct IniProperty *pprop;
};
typedef struct IniSection IniSection;

IniSection *LoadIniFile(char *pszFn, int *pcSections);
int IniScanf(char *pszBuff, char *pszFmt, ...);
int VIniScanf(char *pszBuff, char *pszFmt, va_list va);
