/*
 * $Id: pi-sync.h,v 1.20 2006/10/17 13:24:07 desrod Exp $
 *
 * pi-sync.h: Header for generic synchronization algorithm
 *
 * Copyright (c) 2000-2001, Ximian Inc.
 *
 * Author: JP Rosevear <jpr@helixcode.com> 
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _PILOT_SYNC_H_
#define _PILOT_SYNC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "pi-macros.h"

	typedef struct _SyncHandler SyncHandler;
	typedef struct _DesktopRecord DesktopRecord;
	typedef struct _PilotRecord PilotRecord;

	struct _DesktopRecord {
		int recID;
		int catID;
		int flags;
	};

	struct _PilotRecord {
		recordid_t recID;
		int catID;
		int flags;
		void *buffer;
		size_t len;
	};

	struct _SyncHandler {
		int sd;

		char *name;
		int secret;

		void *data;

		int (*Pre) (SyncHandler *, int dbhandle, int *slow);
		int (*Post) (SyncHandler *, int dbhandle);

		int (*SetPilotID) (SyncHandler *, DesktopRecord *,
				   recordid_t);
		int (*SetStatusCleared) (SyncHandler *, DesktopRecord *);

		int (*ForEach) (SyncHandler *, DesktopRecord **);
		int (*ForEachModified) (SyncHandler *, DesktopRecord **);
		int (*Compare) (SyncHandler *, PilotRecord *,
				DesktopRecord *);

		int (*AddRecord) (SyncHandler *, PilotRecord *);
		int (*ReplaceRecord) (SyncHandler *, DesktopRecord *,
				      PilotRecord *);
		int (*DeleteRecord) (SyncHandler *, DesktopRecord *);
		int (*ArchiveRecord) (SyncHandler *, DesktopRecord *,
				      int archive);

		int (*Match) (SyncHandler *, PilotRecord *,
			      DesktopRecord **);
		int (*FreeMatch) (SyncHandler *, DesktopRecord *);

		int (*Prepare) (SyncHandler *, DesktopRecord *,
				PilotRecord *);
	};

	PilotRecord *sync_NewPilotRecord(int buf_size);
	PilotRecord *sync_CopyPilotRecord(const PilotRecord * precord);
	void sync_FreePilotRecord(PilotRecord * precord);

	DesktopRecord *sync_NewDesktopRecord(void);
	DesktopRecord *sync_CopyDesktopRecord(const DesktopRecord *
					      drecord);
	void sync_FreeDesktopRecord(DesktopRecord * drecord);

	int sync_CopyToPilot(SyncHandler * sh);
	int sync_CopyFromPilot(SyncHandler * sh);

	int sync_MergeToPilot(SyncHandler * sh);
	int sync_MergeFromPilot(SyncHandler * sh);

	int sync_Synchronize(SyncHandler * sh);

#ifdef __cplusplus
}
#endif
#endif
