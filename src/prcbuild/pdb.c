#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#include "bytes.h"
#include "sys.h"
#include "pdb.h"
#include "debug.h"
#include "xalloc.h"
 
#define PDB_HEADER  78
#define PDB_RESHDR  10
#define PDB_RECHDR  8

#define dmDBNameLength 32
#define dmHdrAttrResDB 0x0001

typedef struct pdb_res_t {
  char type[8];
  uint16_t id;
  uint32_t size;
  uint8_t *data;
  struct pdb_res_t *next;
} pdb_res_t;

typedef struct pdb_t {
  char name[dmDBNameLength];
  char type[8];
  char creator[8];
  uint32_t creationDate;
  uint32_t modificationDate;
  uint32_t unique_id_seed;
  uint32_t max_unique_id;
  uint16_t numrecs;
  pdb_res_t *reslist;
  pdb_res_t *lastres;
  uint8_t header[PDB_HEADER];
} pdb_t;

// From: Friday, 1 January 1904, 00:00:00
// To: Thursday, 1 January 1970, 00:00:00
// 2,082,844,800 seconds

#define T0 2082844800ul

/*
  UInt8 name[32];   // name of the database on the PalmPilot
  UInt16 fileAttributes;  // Attributes of the pdb file.
        // 0x0002 Read-Only
        // 0x0004 Dirty AppInfoArea
        // 0x0008 Backup this database
        // 0x0010 Ok to install newer copy
        // 0x0020 Force reset after install
  UInt16 version;   // Defined by the application.
  UInt32 creationDate;    // number of seconds since January 1, 1904.
        // will not install if this value is zero
  UInt32 modificationDate;  // number of seconds since January 1, 1904.
        // will not install if this value is zero
  UInt32 lastBackupDate;    // number of seconds since January 1, 1904.
        //  can be left at zero
  UInt32 modificationNumber;  // Set to zero.
  UInt32 appInfoArea;   // The byte number in the PDB file
        // (counting from zero) at which the AppInfoArea
        // is located. This must be the first entry in
        // the Data portion of the PDB file. If this
        // database does not have an AppInfoArea, set
        // this value to zero.
  UInt32 sortInfoArea;    // The byte number in the PDB file (counting
        // from zero) at which the SortInfoArea is
        // located. This must be placed immediately
        // after the AppInfoArea, if one exists, within
        // the Data portion of the PDB file. If this
        // database does not have a SortInfoArea, set
        // this value to zero. Do not use this. See Note
        // C below for further details.
  UInt8 databaseType[4];  // Database Type used by the application 
  UInt8 creatorID[4];   // Creator ID used application.
  UInt32 uniqueIDSeed;    // used to generate the Unique ID number of
        // subsequent records. Usually set to zero.
  UInt32 nextRecordListID;  // Set this to zero.
  UInt16 numberOfRecords; // This contains the number of records


struct recheader_struct {
  UInt32 recordDataOffset;  // The byte number in the PDB file (counting
        // from zero) at which the record is located.
  UInt8 recordAttributes; // The records attributes.
        // 0x10 Secret record bit.
        // 0x20 Record in use (busy bit).
        // 0x40 Dirty record bit.
        // 0x80 Delete record on next HotSync.
        // The least significant four bits are used to
        // represent the category values.
  UInt8 uniqueID[3];    // Set this to zero and do not try to
        // second-guess what PalmOS will do
}
*/

pdb_t *pdb_new(char *name, char *type, char *creator) {
  pdb_t *pdb = NULL;

  if (name && (pdb = xcalloc(1, sizeof(pdb_t))) != NULL) {
    strncpy(pdb->name, name, dmDBNameLength);
    strncpy(pdb->type, type, 4);
    strncpy(pdb->creator, creator, 4);
    pdb->creationDate = T0 + sys_time();
    pdb->modificationDate = pdb->creationDate;
    pdb->numrecs = 0;
  }

  return pdb;
}

int pdb_destroy(pdb_t *pdb) {
  pdb_res_t *res, *auxs;
  int j, r = -1;

  if (pdb) {
    for (j = 0, res = pdb->reslist; j < pdb->numrecs && res; j++) {
      auxs = res->next;
      if (res->data) xfree(res->data);
      xfree(res);
      res = auxs;
    }
    xfree(pdb);
    r = 0;
  }

  return r;
}

int pdb_add_res(pdb_t *pdb, char *type, uint16_t id, uint32_t size, uint8_t *data) {
  pdb_res_t *res;
  int r = -1;

  if (pdb && size && data) {
    res = xcalloc(1, sizeof(pdb_res_t));
    strncpy(res->type, type, 4);
    res->id = id;
    res->size = size;
    res->data = data;
    if (pdb->lastres == NULL) {
      debug(DEBUG_TRACE, "PDB", "pdb_add_res: added first resource");
      pdb->reslist = res;
      pdb->lastres = res;
    } else {
      debug(DEBUG_TRACE, "PDB", "pdb_add_res: added resource");
      pdb->lastres->next = res;
      pdb->lastres = res;
    }
    pdb->modificationDate = T0 + sys_time();
    pdb->numrecs++;
    debug(DEBUG_TRACE, "PDB", "pdb_add_res: file has %d resource(s)", pdb->numrecs);
    r = 0;
  }

  return r;
}

int pdb_save(pdb_t *pdb, int f) {
  pdb_res_t *res;
  uint32_t offset;
  uint16_t attr;
  uint8_t buf[PDB_RESHDR];
  uint8_t pad[2];
  int i, j, r = -1;

  if (pdb && f) {
    i = 0;
    strncpy((char *)&pdb->header[i], pdb->name, dmDBNameLength);
    i += dmDBNameLength;
    attr = dmHdrAttrResDB;
    offset = PDB_HEADER + pdb->numrecs * PDB_RESHDR + 2;
    i += put2b(attr, pdb->header, i);    // fileAttributes
    i += put2b(1, pdb->header, i);       // version
    i += put4b(pdb->creationDate, pdb->header, i);       // creationDate
    i += put4b(pdb->modificationDate, pdb->header, i);       // modificationDate
    i += put4b(0, pdb->header, i);       // lastBackupDate
    i += put4b(0, pdb->header, i);       // modificationNumber

    i += put4b(0, pdb->header, i);       // appInfoArea
    i += put4b(0, pdb->header, i);      // sortInfoArea

    strncpy((char *)&pdb->header[i], pdb->type, 4);
    i += 4;
    strncpy((char *)&pdb->header[i], pdb->creator, 4);
    i += 4;
    i += put4b(pdb->unique_id_seed, pdb->header, i); // uniqueIDSeed
    i += put4b(0, pdb->header, i); // nextRecordListID
    i += put2b(pdb->numrecs, pdb->header, i); // numberOfRecords

    if (sys_write(f, pdb->header, PDB_HEADER) != PDB_HEADER) {
      return -1;
    }
    debug(DEBUG_TRACE, "PDB", "pdb_save: wrote header \"%s\" (%d recs)", pdb->name, pdb->numrecs);

    for (j = 0, res = pdb->reslist; j < pdb->numrecs && res; j++, res = res->next) {
      i = 0;
      strncpy((char *)&buf[i], res->type, 4);
      i += 4;
      i += put2b(res->id, buf, i);
      i += put4b(offset, buf, i);
      if (sys_write(f, buf, PDB_RESHDR) != PDB_RESHDR) {
        return -1;
      }
      debug(DEBUG_TRACE, "PDB", "pdb_save: wrote resource index %d, offset %u", j, offset);
      offset += res->size;
    }

    pad[0] = pad[1] = 0;
    if (sys_write(f, pad, 2) != 2) {
      return -1;
    }
    debug(DEBUG_TRACE, "PDB", "pdb_save: wrote pad");

    for (j = 0, res = pdb->reslist; j < pdb->numrecs && res; j++, res = res->next) {
      if (sys_write(f, res->data, res->size) != res->size) {
        return -1;
      }
      debug(DEBUG_TRACE, "PDB", "pdb_save: wrote res (%d bytes)", res->size);
    }

    debug(DEBUG_TRACE, "PDB", "pdb_save: end");
    r = 0;
  } else {
    debug(DEBUG_ERROR, "PDB", "pdb_save: invalid parameters");
  }

  return r;
}
