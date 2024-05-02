/*  Vexed - ini.c "ini api over databases"
    Copyright (C) 2001 Scott Ludwig (scottlu@eskimo.com)
	 Copyright (C) 2006 The Vexed Source Forge Project Team

    September 1. 2001

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

#include <PalmOS.h>
#include "ini.h"
#include "debug.h"

typedef UInt32 dword;
typedef UInt16 word;
typedef UInt8  byte;

Ini *OpenIni(char *pszIni)
{
   LocalID lid;
   Int16 cCards;
   int n;
   DmOpenRef pdb;
   Ini *pini;

   // Find the database

   lid = 0;
	cCards = MemNumCards();
   for (n = 0; n < cCards; n++) {
      lid = DmFindDatabase(n, pszIni);
      if (lid != 0)
         break;
   }
   if (lid == 0)
      return NULL;

   // Open it and take a look

   pdb = DmOpenDatabase(n, lid, dmModeReadOnly);
   if (pdb == NULL)
      return NULL;

   // Alloc Ini struct

   pini = (Ini *)MemPtrNew(sizeof(*pini));
   if (pini == NULL) {
      DmCloseDatabase(pdb);
      return NULL;
   }
   MemSet(pini, sizeof(*pini), 0);
   pini->pdb = pdb;
   pini->cRecords = DmNumRecords(pdb);
   pini->nRecordLocked = -1;

   // Done

   return pini;
}

void CloseIni(Ini *pini)
{
   if (pini->pdb == NULL)
      return;
   if (pini->nRecordLocked != -1)
      MemHandleUnlock(pini->hRecordLocked);
   DmCloseDatabase(pini->pdb);
   MemPtrFree(pini);
}

IniSection *FindFirstIniSection(Ini *pini)
{
   if (pini->nRecordLocked != -1) {
      MemHandleUnlock(pini->hRecordLocked);
      pini->nRecordLocked = -1;
   }

   pini->hRecordLocked = DmQueryRecord(pini->pdb, 0);
   if (pini->hRecordLocked == NULL)
      return NULL;
   pini->psecCurrent = (IniSection *)MemHandleLock(pini->hRecordLocked);
   if (pini->psecCurrent == NULL)
      return NULL;
   pini->nRecordLocked = 0;
   return pini->psecCurrent;
}

IniSection *FindNextIniSection(Ini *pini)
{
   int nRecord;

   if (pini->nRecordLocked == -1)
      return NULL;

   // See if we have another section in this record

   if (pini->psecCurrent->offSecNext != 0) {
      pini->psecCurrent = (IniSection *)((byte *)pini->psecCurrent + pini->psecCurrent->offSecNext);
      return pini->psecCurrent;
   }

   // We don't, unlock the current and find another

   MemHandleUnlock(pini->hRecordLocked);
   nRecord = pini->nRecordLocked + 1;
   pini->nRecordLocked = -1;
   pini->psecCurrent = NULL;

   for (; nRecord < pini->cRecords; nRecord++) {
      pini->hRecordLocked = DmQueryRecord(pini->pdb, nRecord);
      if (pini->hRecordLocked == NULL)
         continue;
      pini->psecCurrent = (IniSection *)MemHandleLock(pini->hRecordLocked);
      if (pini->psecCurrent != NULL) {
         pini->nRecordLocked = nRecord;
         break;
      }
   }

   // Return what we found

   return pini->psecCurrent;
}

IniSection *FindIniSection(Ini *pini, char *psz)
{
   IniSection *psec = FindFirstIniSection(pini);
   while (psec != NULL) {
      if (StrCompare(psz, (char *)(psec + 1)) == 0)
         return psec;
      psec = FindNextIniSection(pini);
   }

   return psec;
}

Boolean FindProperty(IniSection *psec, char *pszProp, char *pszValue, int cbValue)
{
   int n;
   char *pszT;
   int cb;

   pszT = (char *)(psec + 1) + StrLen((char *)(psec + 1)) + 1;
   for (n = 0; n < psec->cprop; n++) {
      if (StrCompare(pszT, pszProp) != 0) {
         pszT += StrLen(pszT) + 1;
         pszT += StrLen(pszT) + 1;
         continue;
      }

      pszT += StrLen(pszT) + 1;
      cb = StrLen(pszT) + 1;
      cb = cb < cbValue ? cb : cbValue;
      MemMove(pszValue, pszT, cb);
      pszValue[cb - 1] = 0;
      return true;
   }

   return false;
}
