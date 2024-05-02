/*  Vexed - ini.h "ini api over databases"
    Copyright (C) 2003 Scott Ludwig (scottlu@eskimo.com)
	 Copyright (C) 2006 The Vexed Source Forge Project Team
    
    March 24, 2003
          
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

typedef struct
{
	UInt16 offSecNext;
	UInt16 cprop;
	// char szSecName[];
} IniSection; // sec

typedef struct
{
   DmOpenRef pdb;
   Int16 cRecords;
   Int16 nRecordLocked;
   MemHandle hRecordLocked;
   IniSection *psecCurrent;
} Ini; // ini

Ini *OpenIni(char *pszIni);
void CloseIni(Ini *pini);
IniSection *FindFirstIniSection(Ini *pini);
IniSection *FindNextIniSection(Ini *pini);
IniSection *FindIniSection(Ini *pini, char *psz);
Boolean FindProperty(IniSection *psec, char *pszProp, char *pszValue, int cbValue);
