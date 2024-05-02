/*  main.c "ini to pdb conversion"
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

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include "hini.h"

#define _MAX_PATH  256

#define kcbRecordMax 0x8000
#define SwapWord(x) ((((x)&0xFF)<<8) | (((x)&0xFF00)>>8))
#define SwapDword(x) ((((x)&0xFF)<<24) | (((x)&0xFF00)<<8) | (((x)&0xFF0000)>>8) | (((x)&0xFF000000)>>24))

struct RecordChunk // rck
{
	byte *pb;
	byte *pbMax;
};
typedef struct RecordChunk RecordChunk;

struct SecChunk // sck
{
	word offSecNext;
	short cprop;
	// char szSecName[];
};
typedef struct SecChunk SecChunk;

struct PropChunk // pck
{
	// char szProp[];
	// char szPropValue[];
};
typedef struct PropChunk PropChunk;

int WriteDatabase(char *pszFn, RecordChunk *arck, int cChunks, long *pcbFile);
int CalcSectionSize(IniSection *psec);
int ReadString(FILE *pf, int nch, int cch, byte *pb);

void Usage()
{
	printf("\n");
	printf("Usage: vexpdb levelpack.ini\n");
	printf("\n");
	printf("This will read the level pack description held in levelpack.ini and\n");
	printf("output levelpack.pdb which is suitable for reading by Vexed.\n");
	printf("\n");
	printf("Usage: vexpdb -bin levelpack.ini\n");
	printf("\n");
	printf("This will read the level pack description held in levelpack.ini and\n");
	printf("output levelpack.bin which is the straight binary without .pdb\n");
	printf("wrapper.\n");
	printf("\n");
	exit(1);
}

int main(int argc, char **argv)
{
	char *pszFn;
	if (argc == 2) {
		pszFn = argv[1];
	} else {
		Usage();
	}

	// File exists?

	FILE *pf = fopen(pszFn, "rb");
	if (pf == NULL) {
		printf("Error opening %s for reading.\n", pszFn);
		exit(1);
	}

	// Parse .ini file

	int cSections;
	IniSection *psec = LoadIniFile(pszFn, &cSections);
	if (psec == NULL) {
		printf("Error parsing .ini file %s.\n", pszFn);
		fclose(pf);
		exit(1);
	}

	// Build in memory format

	RecordChunk arck[256];
	memset(arck, 0, sizeof(arck));
	int cChunks = 0;

	IniSection *psecT = psec;
	IniSection *psecMax = &psec[cSections];

	int fDone = 0;
	while (!fDone) {
		RecordChunk *prck = &arck[cChunks++];
		//prck->pb = new byte[kcbRecordMax];
		prck->pb = calloc(1, kcbRecordMax);
		byte *pbT = prck->pb;

		while (!fDone) {
			// If this section does not fit in this chunk, start a new chunk	

			if (&pbT[CalcSectionSize(psecT)] > &prck->pb[kcbRecordMax])
				break;

			// Add this section to this chunk

			SecChunk *psck = (SecChunk *)pbT;
			//psck->cprop = SwapWord(psecT->cprop);
			psck->cprop = psecT->cprop;

			// Copy in the section name and zero extend

			pbT = (byte *)(psck + 1);
			if (!ReadString(pf, psecT->nchSec, psecT->cchSec, pbT)) {
				printf("Error reading %s.\n", pszFn);
				fclose(pf);
				exit(1);
			}
			pbT += psecT->cchSec + 1;

			// Copy in the property / value pairs

			IniProperty *ppropT = psecT->pprop;
			for (int n = 0; n < psecT->cprop; n++) {
				if (!ReadString(pf, ppropT->nchProp, ppropT->cchProp, pbT)) {
					printf("Error reading %s.\n", pszFn);
					fclose(pf);
					exit(1);
				}
				pbT += ppropT->cchProp + 1;
				if (!ReadString(pf, ppropT->nchValue, ppropT->cchValue, pbT)) {
					printf("Error reading %s.\n", pszFn);
					fclose(pf);
					exit(1);
				}				
				pbT += ppropT->cchValue + 1;
				ppropT++;
			}

			// Even align

			if ((pbT - prck->pb) & 1)
				*pbT++ = 0;

			// Backfill the "next section" offset

			//psck->offSecNext = SwapWord(pbT - (byte *)psck);
			psck->offSecNext = pbT - (byte *)psck;

			// Next section...

			psecT++;
			if (psecT >= psecMax) {
				psck->offSecNext = 0;
				fDone = 1;
			}
		}

		// Remember how much of this chunk we wrote to

		prck->pbMax = pbT;
	}
	fclose(pf);

	// Write the chunks out to the database

	long cbFile;
	int fSuccess;
  fSuccess = WriteDatabase(pszFn, arck, cChunks, &cbFile);

	// Free chunks

	int n;
	for (n = 0; n < cChunks; n++)
		//delete arck[n].pb;
		free(arck[n].pb);

	// Free property sections

	for (n = 0; n < cSections; n++)
		//delete psec[n].pprop;
		free(psec[n].pprop);
	//delete psec;
	free(psec);

	// Stats

	if (fSuccess) {
		printf("Success.... %ld bytes written.\n", cbFile);
		return 0;
	}
	
	// Failure

	return 1;
}

int CalcSectionSize(IniSection *psec)
{
	int cb = sizeof(SecChunk);
	cb += psec->cchSec + 1;
	for (int n = 0; n < psec->cprop; n++) {
		cb += psec->pprop[n].cchProp + 1;
		cb += psec->pprop[n].cchValue + 1;
	}
	cb = (cb + 1) & ~1;
	return cb;
}

int ReadString(FILE *pf, int nch, int cch, byte *pb)
{
	*pb = 0;
	fseek(pf, nch, SEEK_SET);
	if (fread(pb, cch, 1, pf) != 1)
		return 0;
	pb[cch] = 0;
	return 1;
}

// These structs straight from Palm includes with minor editting

#define dmDBNameLength 32

typedef uint8_t UInt8;
typedef uint16_t UInt16;
typedef uint32_t UInt32;
typedef uint32_t LocalID;
typedef char Char;

struct RecordEntryType {
	LocalID localChunkID;		// local chunkID of a record
	UInt8 attributes;			// record attributes;
	UInt8 uniqueID[3];			// unique ID of record; should
								//	not be 0 for a legal record.
};
typedef struct RecordEntryType RecordEntryType;

struct RecordListType {
	LocalID nextRecordListID;	// local chunkID of next list
	UInt16 numRecords;			// number of records in this list
	UInt16 firstEntry;			// array of Record/Rsrc entries 
								// starts here
};
typedef struct RecordListType RecordListType;

struct DatabaseHdrType {
	char name[dmDBNameLength];	// name of database
	UInt16 attributes;			// database attributes
	UInt16 version;				// version of database
	UInt32 creationDate;		// creation date of database
	UInt32 modificationDate;	// latest modification date
	UInt32 lastBackupDate;		// latest backup date
	UInt32 modificationNumber;	// modification number of database
	LocalID appInfoID;			// application specific info
	LocalID sortInfoID;			// app specific sorting info
	UInt32 type;				// database type
	UInt32 creator;				// database creator 
	UInt32 uniqueIDSeed;		// used to generate unique IDs.
								//	Note that only the low order
								//	3 bytes of this is used (in
								//	RecordEntryType.uniqueID).
								//	We are keeping 4 bytes for 
								//	alignment purposes.
	RecordListType	recordList;	// first record list
};		
typedef struct DatabaseHdrType DatabaseHdrType;

dword GetCurrentTimePalmUnits() {
  return 0;
}
#if 0
#include <windows.h>
dword GetCurrentTimePalmUnits()
{
	// Palm keeps time as a count of seconds from Jan 1, 1904.

	FILETIME timCurrent;
	GetSystemTimeAsFileTime(&timCurrent);

	SYSTEMTIME timsys1904;
	timsys1904.wYear = 1904;
	timsys1904.wMonth = 1;
	timsys1904.wDayOfWeek = 6;
	timsys1904.wDay = 1;
	timsys1904.wHour = 0;
	timsys1904.wMinute = 0;
	timsys1904.wSecond = 0;
	timsys1904.wMilliseconds = 0;

	FILETIME tim1904;
	SystemTimeToFileTime(&timsys1904, &tim1904);

	// Now we have 2 64 bit unsigned ints that specifies time in terms of 100 nanosecond
	// units. Subtract the two and we have the time between now and Jan 1 1904 in 100 nanosecond
	// units. 1 s == 1e9 ns, so there are 1e7 filetime units in 1 second.

	unsigned __int64 tCurrent;
	tCurrent = ((unsigned __int64)timCurrent.dwHighDateTime << 32) | timCurrent.dwLowDateTime;
	unsigned __int64 t1904;
	t1904 = ((unsigned __int64)tim1904.dwHighDateTime << 32) | tim1904.dwLowDateTime;
	unsigned __int64 tDiff = tCurrent - t1904;

	// Divide by 1024*1024 so it fits in a dword

	dword dw = (dword)(tDiff >> 20);

	// Divide by 1e9 / (1024*1024)

	return (dword)((double)dw / (10000000.0 / (double)(1024 * 1024)));
}
#endif

int WriteDatabase(char *pszFn, RecordChunk *arck, int cChunks, long *pcbFile)
{
	// Construct the new filename, which is same as the current with an extension of ".pdb"
	
	char szFnPdb[_MAX_PATH];
	char szFname[_MAX_PATH];
  char *p, *s;
  int i;

	// Create pdb file

  strncpy(szFnPdb, pszFn, sizeof(szFnPdb)-1);
  p = strchr(szFnPdb, '.');
  p[0] = 0;
  strncpy(szFname, szFnPdb, sizeof(szFname)-1);
  strncat(szFnPdb, ".pdb", sizeof(szFnPdb)-1 - strlen(szFnPdb));
	FILE *pf = fopen(szFnPdb, "wb");
	if (pf == NULL) {
		printf("Unable to create \"%s\".\n", szFnPdb);
		return 0;
	}

	// Figure out the creator

	//dword dwCreator = (pszCreator[0] << 24) | (pszCreator[1] << 16) | (pszCreator[2] << 8) | pszCreator[3];
	dword dwCreator = 'Vexd';

	// Init the .pdb header

	DatabaseHdrType hdr;
	memset(&hdr, 0, sizeof(hdr));

	// Use the filename as the database name

  for (i = 0, s = NULL; szFname[i]; i++) {
    if (szFname[i] == '/') s = &szFname[i];
  }
  if (s) s++;
	strcpy(hdr.name, s);
	if (strlen(hdr.name) > dmDBNameLength - 1) {
		printf("Pdb filename too long.\n");
Error:
		fclose(pf);
		unlink(szFnPdb);
		return 0;
	}

	// Hardwire a bunch of stuff about this .pdb.

	hdr.attributes = SwapWord(0);
	hdr.version = SwapWord(1);
	dword dwDate = GetCurrentTimePalmUnits();
	hdr.creationDate = SwapDword(dwDate);
	hdr.modificationDate = SwapDword(dwDate);
	hdr.lastBackupDate = SwapDword(dwDate);
	hdr.modificationNumber = SwapDword(1);
	hdr.appInfoID = SwapDword(0);
	hdr.sortInfoID = SwapDword(0);
	//hdr.type = SwapDword('DATA');
	hdr.type = 'ATAD';
	hdr.creator = SwapDword(dwCreator);
	hdr.uniqueIDSeed = SwapDword(cChunks + 1);
	hdr.recordList.nextRecordListID = SwapDword(0);
	hdr.recordList.numRecords = SwapWord((word)cChunks);

	// Write the header out

	int cbHdr = sizeof(hdr) - sizeof(hdr.recordList.firstEntry);	
	if (fwrite(&hdr, cbHdr, 1, pf) != 1) {
		printf("Error writing database header.\n");
		goto Error;
	}

	// Calculate where the actual records begin, which is after the record headers

	int offNext = cbHdr + cChunks * sizeof(RecordEntryType);

	// Write out the record headers.

	int n;
	for (n = 0; n < cChunks; n++) {
		RecordEntryType entry;
		memset(&entry, 0, sizeof(entry));
		entry.localChunkID	= SwapDword(offNext);
		entry.attributes = 0;
		dword id = n + 1;
		entry.uniqueID[2] = (byte)(id & 0xff);
		entry.uniqueID[1] = (byte)((id >> 8) & 0xff);
		entry.uniqueID[0] = (byte)((id >> 16) & 0xff);
		id++;
		offNext += arck[n].pbMax - arck[n].pb;
		if (fwrite(&entry, sizeof(entry), 1, pf) != 1) {
			printf("Error writing database record header.\n");
			goto Error;
		}
	}

	// Write out the chunks

	for (n = 0; n < cChunks; n++) {
		if (fwrite(arck[n].pb, arck[n].pbMax - arck[n].pb, 1, pf) != 1) {
			printf("Error writing record data.\n");
			goto Error;
		}
	}

	*pcbFile = ftell(pf);
	fclose(pf);

	return 1;
}
