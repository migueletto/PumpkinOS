/*  ini.c "ini file reading support"
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

#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "hini.h"

// IniScanf / VIniScanf
//
// Supports:
// %s : "trimmed string", matches a string with interleaving spaces. Removes
//		leading and trailing white space, with ini-friendly delimiters
// %s+: means %s plus store index of matching string
// %+ : means store current buffer index
// '*' after % is supported and means match but don't store
// ' ', '\t', '\n' : match for optional white space (space, tab, newline)

#define IsWhiteSpace(ch) ((ch) == ' ' || (ch) == '\t' || (ch) == '\n' || (ch) == '\r')
#ifdef IsDigit
#undef IsDigit
#endif
#define IsDigit(ch) ((ch) >= '0' && (ch) <= '9')

int IniScanf(char *pszBuff, char *pszFmt, ...)
{
	va_list va;
	va_start(va, pszFmt);
	int c = VIniScanf(pszBuff, pszFmt, va);
	va_end(va);
	return c;
}

int VIniScanf(char *pszBuff, char *pszFmt, va_list va)
{
	char *pszBuffT = pszBuff;
	char *pszFmtT = pszFmt;
	int cArgs = 0;
	while (*pszFmtT != 0) {
		// Make this special check before checking for end
		// of buffer

		if (pszFmtT[0] == '%' && pszFmtT[1] == '+') {
			pszFmtT += 2;
			*va_arg(va, int *) = pszBuffT - pszBuff;
			cArgs++;
			continue;
		}

		switch (*pszFmtT) {
		case ' ':
		case '\n':
		case '\t':
			// Eat white space (space, tab, newline)

			while (IsWhiteSpace(*pszBuffT))
				pszBuffT++;
			pszFmtT++;
			continue;
		}

		// End of buffer? Bail.

		if (*pszBuffT == 0)
			break;

		int fSuppress;
		switch (*pszFmtT) {
		case '%':
			// Suppress?

			pszFmtT++;
			fSuppress = 0;
			if (*pszFmtT == '*') {
				fSuppress = 1;
				pszFmtT++;
			}

			// Format specification

			switch (*pszFmtT) {
			case 's':
				// Trimmed string.
				{
					pszFmtT++;

					// If %s+, it means save the location of substring

					int fSaveLocation = 0;
					if (*pszFmtT == '+') {
						fSaveLocation = 1;
						pszFmtT++;
					}

					// Go past initial whitespace

					int nchStart = pszBuffT - pszBuff;
					while (IsWhiteSpace(*pszBuffT)) {
						nchStart++;
						pszBuffT++;
					}

					// Copy the string into pszT. Figure out when to stop
					// and remember trimming information.

					char chStop = *pszFmtT;	
					char *pszT = va_arg(va, char *);
					char *pszWhiteLast = NULL;
					while (*pszBuffT != chStop) {
						if (*pszBuffT == 0)
							break;
						if (!IsWhiteSpace(*pszBuffT)) {
							pszWhiteLast = NULL;
						} else if (pszWhiteLast == NULL) {
							pszWhiteLast = pszT;
						}
						if (!fSuppress)
							*pszT++ = *pszBuffT++;
					}
					if (!fSuppress) {
						*pszT = 0;
						if (pszWhiteLast != NULL)
							*pszWhiteLast = 0;
						cArgs++;
					}

					// %s+ means save location string started

					if (!fSuppress && fSaveLocation) {
						*va_arg(va, int *) = nchStart;
						cArgs++;
					}
				}
				continue;

			default:
				// Non-recognized % command

				return cArgs;
			}
			break;

		default:
			// Match character

			if (*pszBuffT == *pszFmtT) {
				pszBuffT++;
				pszFmtT++;
				continue;
			}
			break;
		}

		// If we're here it means stop

		break;
	}

	return cArgs;
}

int NewSection(char *pszSec, dword nch, IniSection **ppsec, int *pcsec)
{
	//IniSection *psec = new IniSection[(*pcsec) + 1];
	IniSection *psec = calloc((*pcsec) + 1, sizeof(IniSection));
	if (psec == NULL)
		return 0;
	if (*ppsec != NULL) {
		memcpy(psec, *ppsec, sizeof(IniSection) * (*pcsec));
		free(*ppsec);
	} 
	*ppsec = psec;
	psec = &(*ppsec)[*pcsec];
	(*pcsec)++;

	psec->nchSec = nch;
	psec->cchSec = strlen(pszSec);
	psec->cprop = 0;
	psec->pprop = NULL;
	return 1;
}
	
int NewProperty(IniSection *psec, char *pszProp, dword nchProp, char *pszValue, dword nchValue)
{
	//IniProperty *pprop = new IniProperty[psec->cprop + 1];
	IniProperty *pprop = calloc(psec->cprop + 1, sizeof(IniProperty));
	if (pprop == NULL)
		return 0;
	if (psec->pprop != NULL) {
		memcpy(pprop, psec->pprop, sizeof(IniProperty) * psec->cprop);
		free(psec->pprop);
	}
	psec->pprop = pprop;
	pprop = &psec->pprop[psec->cprop];
	psec->cprop++;

	pprop->nchProp = nchProp;
	pprop->cchProp = strlen(pszProp);
	pprop->nchValue = nchValue;
	pprop->cchValue = strlen(pszValue);
	return 1;
}

IniSection *LoadIniFile(char *pszFn, int *pcSections)
{
	IniSection *psec = NULL;
	int csec = 0;

	FILE *pf = fopen(pszFn, "rb");
	if (pf == NULL)
		return 0;

	while (1) {
		// Get the next line

		dword nchLine = ftell(pf);
		char szLine[2048];
		if (fgets(szLine, sizeof(szLine) - 1, pf) == NULL)
			break;

		// Is it a section header? If so, add it

		char szT[1024];
		int nchSec;
		if (IniScanf(szLine, "\t[%s+]\t", szT, &nchSec) == 2) {
			if (!NewSection(szT, nchLine + nchSec, &psec, &csec))
				return NULL;
			continue;
		}

		// Is it just whitespace? Allow leading ; for comments

		int fAllWhiteSpace = 1;
		int cch = strlen(szLine);
		for (int n = 0; n < cch; n++) {
			if (!IsWhiteSpace(szLine[n])) {
				if (szLine[n] == ';')
					break;
				fAllWhiteSpace = 0;
				break;
			}
		}
		if (fAllWhiteSpace)
			continue;

		// See if it is property = value

		int nchProp;
		int nchValue;
		char szValue[1024];
		if (IniScanf(szLine, "%s+=%s+", szT, &nchProp, szValue, &nchValue) == 4) {
			if (!NewProperty(&psec[csec - 1], szT, nchLine + nchProp, szValue, nchLine + nchValue))
				return NULL;
			continue;
		}

		// Otherwise ignore it
	}

	*pcSections = csec;
	return psec;
}

