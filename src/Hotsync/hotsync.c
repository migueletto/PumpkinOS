#include <PalmOS.h>

#include "sys.h"
#include "pumpkin.h"
#include "xalloc.h"
#include "debug.h"
#include "resource.h"

#include "config.h"
#include "pi-debug.h"
#include "pi-socket.h"
#include "pi-file.h"
#include "pi-util.h"
#include "pi-userland.h"

#define AppID 'Hots'

#define PI_HDR_SIZE 78

typedef struct {
  int fd;
  char **localItemsText;
  char **remoteItemsText;
  UInt16 localNumItems;
  UInt16 remoteNumItems;
  UInt16 localSelected;
  UInt16 remoteSelected;
  Boolean connected, refresh, remote;
} hotsync_data_t;

static void dialog(int level, const char *format, ...) {
  va_list ap;
  char buf[256];

  va_start(ap, format);
  debugva(level, SYS_DEBUG, format, ap);
  va_end(ap);

  va_start(ap, format);
  sys_vsnprintf(buf, sizeof(buf)-1, format, ap);
  va_end(ap);
  FrmCustomAlert(level == DEBUG_ERROR ? ErrorAlert : InfoAlert, buf, "", "");
}

/*
static void paintBitmap(UInt16 id, Coord x, Coord y) {
  BitmapType *bmp;
  MemHandle h;

  if ((h = DmGet1Resource(bitmapRsc, id)) != NULL) {
    if ((bmp = MemHandleLock(h)) != NULL) {
      WinPaintBitmap(bmp, x, y);
      MemHandleUnlock(h);
    }
    DmReleaseResource(h);
  }
}
*/

static int fetch_progress(int sd, pi_progress_t *progress) {
  const char *filename = NULL;

  if (progress->type == PI_PROGRESS_RECEIVE_DB && progress->data.db.pf && progress->data.db.pf->file_name) {
    filename = progress->data.db.pf->file_name;
  } else if (progress->type == PI_PROGRESS_RECEIVE_VFS && progress->data.vfs.path && StrLen(progress->data.vfs.path)) {
    filename = progress->data.vfs.path;
  }
  if (!filename) filename = "<unnamed>";

  debug(DEBUG_INFO, SYS_DEBUG, "fetching '%s' (%d bytes)", filename, progress->transferred_bytes);

  if (!pi_socket_connected(sd)) {
    return PI_TRANSFER_STOP;
  }

  return PI_TRANSFER_CONTINUE;
}

static int file_retrieve(pi_file_t *pf, int socket, int cardno, progress_func report_progress) {
  int db = -1, result, old_device = 0;
  unsigned int j;
  struct DBInfo dbi;
  struct DBSizeInfo size_info;
  pi_buffer_t *buffer = NULL;
  pi_progress_t progress;

  pi_reset_errors(socket);
  MemSet(&size_info, sizeof(size_info), 0);
  MemSet(&dbi, sizeof(dbi), 0);

  /* Try to get more info on the database to retrieve. Note that
   * with some devices like the Tungsten T3 and shadowed databases
   * like AddressDB, the size_info is -wrong-. It doesn't reflect
   * the actual contents of the database except for the number of
   * records.  Also, this call doesn't work pre-OS 3.
   */
  if ((result = dlp_FindDBByName(socket, cardno, pf->info.name, NULL, NULL, &dbi, &size_info)) < 0) {
    if (result != PI_ERR_DLP_UNSUPPORTED)
      goto fail;
    old_device = 1;
  }

  if ((result = dlp_OpenDB(socket, cardno, dlpOpenRead | dlpOpenSecret, pf->info.name, &db)) < 0)
    goto fail;

  buffer = pi_buffer_new (DLP_BUF_SIZE);
  if (buffer == NULL) {
    result = pi_set_error(socket, PI_ERR_GENERIC_MEMORY);
    goto fail;
  }

  if (old_device) {
    int num_records;
    if ((result = dlp_ReadOpenDBInfo(socket, db, &num_records)) < 0)
        goto fail;
    size_info.numRecords = num_records;
  }

  MemSet(&progress, sizeof(progress), 0);
  progress.type = PI_PROGRESS_RECEIVE_DB;
  progress.data.db.pf = pf;
  progress.data.db.size = size_info;

  if (size_info.appBlockSize
    || (dbi.miscFlags & dlpDBMiscFlagRamBased)
    || old_device) {
    /* what we're trying to do here is avoid trying to read an appBlock
     * from a ROM file, because this crashes on several devices.
     * Also, on several palmOne devices, the size info returned by the OS
     * is absolutely incorrect. This happens with some system shadow files
     * like AddressDB on T3, which actually do contain data and an appInfo
     * block but the system tells us there's no appInfo and nearly no data,
     * but still gives the accurate number of records. Seems to be bad
     * structure shadows in PACE.
     * In any case, the ultimate result is that:
     * 1. On devices pre-OS 3, we do always try to read the appInfo block
     *    because dlp_FindDBByName() is unsupported so we can't find out if
     *    there's an appInfo block
     * 2. On OS5+ devices, we're not sure that the appInfo size we have is
     *    accurate. But if we try reading an appInfo block in ROM it may
     *    crash the device
     * 3. Therefore, we only try to read the appInfo block if we are
     *    working on a RAM file or we are sure that a ROM file has appInfo.
     */
    result = dlp_ReadAppBlock(socket, db, 0, DLP_BUF_SIZE, buffer);
    if (result > 0) {
      pi_file_set_app_info(pf, buffer->data, (size_t)result);
      progress.transferred_bytes += result;
      if (report_progress && report_progress(socket,
          &progress) == PI_TRANSFER_STOP) {
        result = PI_ERR_FILE_ABORTED;
        goto fail;
      }
    }
  }

  if (pf->info.flags & dlpDBFlagResource) {
    for (j = 0; j < size_info.numRecords; j++) {
      int resource_id;
      uint32_t type;

      if ((result = dlp_ReadResourceByIndex(socket, db, j, buffer, &type, &resource_id)) < 0)
        goto fail;

      if ((result = pi_file_append_resource(pf, buffer->data, buffer->used, type, resource_id)) < 0) {
        pi_set_error(socket, result);
        goto fail;
      }

      progress.transferred_bytes += buffer->used;
      progress.data.db.transferred_records++;

      if (report_progress && report_progress(socket, &progress) == PI_TRANSFER_STOP) {
        result = pi_set_error(socket, PI_ERR_FILE_ABORTED);
        goto fail;
      }
    }
  } else for (j = 0; j < size_info.numRecords; j++) {
    int attr, category;
    uint32_t resource_id;

    if ((result = dlp_ReadRecordByIndex(socket, db, j, buffer, &resource_id, &attr, &category)) < 0)
      goto fail;

    progress.transferred_bytes += buffer->used;
    progress.data.db.transferred_records++;

    if (report_progress && report_progress(socket, &progress) == PI_TRANSFER_STOP) {
      result = pi_set_error(socket, PI_ERR_FILE_ABORTED);
      goto fail;
    }

    /* There is no way to restore records with these
       attributes, so there is no use in backing them up
     */
    if (attr & (dlpRecAttrArchived | dlpRecAttrDeleted))
      continue;

    if ((result = pi_file_append_record(pf, buffer->data, buffer->used, attr, category, resource_id)) < 0) {
      pi_set_error(socket, result);
      goto fail;
    }
  }

  pi_buffer_free(buffer);

  return dlp_CloseDB(socket, db);

fail:
  if (db != -1 && pi_socket_connected(socket)) {
    int err = pi_error(socket);      /* make sure we keep last error code */
    int palmoserr = pi_palmos_error(socket);
    
    dlp_CloseDB(socket, db);
    
    pi_set_error(socket, err);      /* then restore it afterwards */
    pi_set_palmos_error(socket, palmoserr);
  }

  if (buffer != NULL) {
    pi_buffer_free(buffer);
  }

  if (result >= 0) {
    /* one of our pi_file* calls failed */
    result = pi_set_error(socket, PI_ERR_FILE_ERROR);
  }

  return result;
}

static int db_install(char *dbname, int socket) {
  LocalID dbID, appInfoID;
  DmOpenRef dbRef;
  MemHandle h;
  struct CardInfo card;
  UInt16 num, index, dbAttr, dbversion, resID, recAttr;
  UInt32 totalBytes, size, type, creator, resType, uniqueID;
  Boolean resDB;
  void *p;
  int version, attr, db;

  if ((dbID = DmFindDatabase(0, dbname)) == 0) {
    dialog(DEBUG_ERROR, "Unable to locate database '%s'.", dbname);
    return -1;
  }

  if ((dbRef = DmOpenDatabase(0, dbID, dmModeReadOnly)) == NULL) {
    dialog(DEBUG_ERROR, "Error opening database '%s'.", dbname);
    return -1;
  }

  DmOpenDatabaseInfo(dbRef, NULL, NULL, NULL, NULL, &resDB);
  DmDatabaseInfo(0, dbID, NULL, &dbAttr, &dbversion, NULL, NULL, NULL, NULL, &appInfoID, NULL, &type, &creator);
  DmDatabaseSize(0, dbID, NULL, &totalBytes, NULL);
  debug(DEBUG_INFO, SYS_DEBUG, "database size is %u", totalBytes);

  version = pi_version(socket);
  debug(DEBUG_INFO, SYS_DEBUG, "dlp version 0x%04X", version);

  card.card = 0;
  if (dlp_ReadStorageInfo(socket, card.card, &card) < 0) {
    DmCloseDatabase(dbRef);
    dialog(DEBUG_ERROR, "Error reading remote storage info.");
    return -1;
  }
  debug(DEBUG_INFO, SYS_DEBUG, "available space is %u", card.ramFree);

  if (totalBytes > card.ramFree) {
    DmCloseDatabase(dbRef);
    dialog(DEBUG_ERROR, "Insufficient space to install '%s' on the device (%u > %u).", dbname, totalBytes, card.ramFree);
    return -1;
  }

  debug(DEBUG_INFO, SYS_DEBUG, "deleting remote database '%s'", dbname);
  dlp_DeleteDB(socket, 0, dbname);

  debug(DEBUG_INFO, SYS_DEBUG, "creating remote database '%s'", dbname);
  attr = 0;
  if (dbAttr & dmHdrAttrResDB) attr |= dlpDBFlagResource;
  if (dlp_CreateDB(socket, creator, type, 0, attr, dbversion, dbname, &db) < 0) {
    DmCloseDatabase(dbRef);
    dialog(DEBUG_ERROR, "Error creating remote database '%s'.", dbname);
    return -1;
  }

  if (appInfoID) {
    h = MemLocalIDToHandle(appInfoID);
    size = MemHandleSize(h);
    debug(DEBUG_INFO, SYS_DEBUG, "writing appInfo (%d bytes)", size);
    p = MemHandleLock(h);
    dlp_WriteAppBlock(socket, db, p, size);
    MemHandleUnlock(h);
  }

  if (resDB) {
    num = DmNumResources(dbRef);
    for (index = 0; index < num; index++) {
      DmResourceInfo(dbRef, index, &resType, &resID, NULL);
      h = DmGetResourceIndex(dbRef, index);
      size = MemHandleSize(h);
      p = MemHandleLockEx(h, false); // get the original resource buffer, not the decoded buffer
      debug(DEBUG_INFO, SYS_DEBUG, "writing resource %d (%d bytes)", index, size);
      dlp_WriteResource(socket, db, resType, resID, p, size);
      MemHandleUnlock(h);
      DmReleaseResource(h);
    }
  } else {
    num = DmNumRecords(dbRef);
    for (index = 0; index < num; index++) {
      DmRecordInfo(dbRef, index, &recAttr, &uniqueID, NULL);
      if ((recAttr & dmRecAttrDelete) && version < 0x0101) continue;
      h = DmGetRecord(dbRef, index);
      size = MemHandleSize(h);
      p = MemHandleLock(h);
      attr = 0;
      if (recAttr & dmRecAttrSecret) attr |= dlpRecAttrSecret;
      if (recAttr & dmRecAttrDelete) attr |= dlpRecAttrDeleted;
      debug(DEBUG_INFO, SYS_DEBUG, "writing record %d (%d bytes)", index, size);
      dlp_WriteRecord(socket, db, attr, uniqueID, recAttr & dmRecAttrCategoryMask, p, size, 0);
      MemHandleUnlock(h);
      DmReleaseRecord(h, index, false);
    }
  }

  DmCloseDatabase(dbRef);

  debug(DEBUG_INFO, SYS_DEBUG, "closing remote database '%s'", dbname);
  if (dlp_CloseDB(socket, db) < 0) {
    dialog(DEBUG_ERROR, "Error closing remote database '%s'.", dbname);
    return -1;
  }

  return 0;
}

#if 0
static int file_install(pi_file_t *pf, int socket, int cardno, progress_func report_progress) {
  int db = -1, j, reset = 0, flags, version, freeai = 0, result, err1, err2;
  size_t l, size = 0;
  void *buffer;
  pi_progress_t progress;

  version = pi_version(socket);

  xmemset(&progress, 0, sizeof(progress));
  progress.type = PI_PROGRESS_SEND_DB;
  progress.data.db.pf = pf;
  progress.data.db.size.numRecords = pf->num_entries;
  progress.data.db.size.dataBytes = pf->app_info_size;
  progress.data.db.size.appBlockSize = pf->app_info_size;
  progress.data.db.size.maxRecSize = pi_maxrecsize(socket);

  /* compute total size for progress reporting, and check that
     either records are 64k or less, or the handheld can accept
     large records. we do this prior to starting the install,
     to avoid messing the device up if we have to fail. */
  for (j = 0; j < pf->num_entries; j++) {
    result =  (pf->info.flags & dlpDBFlagResource) ?
      pi_file_read_resource(pf, j, 0, &size, 0, 0) :
      pi_file_read_record(pf, j, 0, &size, 0, 0, 0);
    if (result < 0) {
      LOG((PI_DBG_API, PI_DBG_LVL_ERR, "FILE INSTALL can't read all records/resources\n"));
      goto fail;
    }
    if (size > 65536 && version < 0x0104) {
      LOG((PI_DBG_API, PI_DBG_LVL_ERR, "FILE INSTALL Database contains record/resource over 64K!\n"));
      goto fail;
    }
    progress.data.db.size.dataBytes += size;
  }

  progress.data.db.size.totalBytes =
    progress.data.db.size.dataBytes +
    pf->ent_hdr_size * pf->num_entries +
    PI_HDR_SIZE + 2;

  /* Delete DB if it already exists */
  dlp_DeleteDB(socket, cardno, pf->info.name);

  /* Set up DB flags */
  flags = pf->info.flags;
  
  LOG((PI_DBG_API, PI_DBG_LVL_INFO, "FILE INSTALL Name: %s Flags: %8.8X\n", pf->info.name, flags));

  /* Create DB */
  if ((result = dlp_CreateDB(socket, pf->info.creator, pf->info.type, cardno, flags, pf->info.version, pf->info.name, &db)) < 0) {
    return result;
  }

  pi_file_get_app_info(pf, &buffer, &l);

  /* Compensate for bug in OS 2.x Memo */
  if (version > 0x0100 && StrCompare(pf->info.name, "MemoDB") == 0 && l > 0 && l < 282) {
    /* Justification: The appInfo structure was accidentally
       lengthend in OS 2.0, but the Memo application does not
       check that it is long enough, hence the shorter block
       from OS 1.x will cause the 2.0 Memo application to lock
       up if the sort preferences are modified. This code
       detects the installation of a short app info block on a
       2.0 machine, and lengthens it. This transformation will
       never lose information. */
    void *b2 = calloc(1, 282);
    xmemcpy(b2, buffer, (size_t)l);
    buffer = b2;
    progress.data.db.size.appBlockSize = 282;
    l = 282;
    freeai = 1;
  }

  /* All system updates seen to have the 'ptch' type, so trigger a
     reboot on those */
  if (pf->info.creator == pi_mktag('p', 't', 'c', 'h'))
    reset = 1;

  if (pf->info.flags & dlpDBFlagReset)
    reset = 1;

  /* Upload appInfo block */
  if (l > 0) {
    if ((result = dlp_WriteAppBlock(socket, db, buffer, l)) < 0) {
      if (freeai) free(buffer);
      goto fail;
    }
    if (freeai) free(buffer);
    progress.transferred_bytes = l;
    if (report_progress && report_progress(socket, &progress) == PI_TRANSFER_STOP) {
      result = pi_set_error(socket, PI_ERR_FILE_ABORTED);
      goto fail;
    }
  }

  /* Upload resources / records */
  if (pf->info.flags & dlpDBFlagResource) {
    for (j = 0; j < pf->num_entries; j++) {
      int resource_id;
      uint32_t type;

      if ((result = pi_file_read_resource(pf, j, &buffer, &size, &type,  &resource_id)) < 0)
        goto fail;

      /* Skip empty resource, it cannot be installed */
      if (size == 0)
        continue;

      if ((result = dlp_WriteResource(socket, db, type, resource_id, buffer, size)) < 0)
        goto fail;

      progress.transferred_bytes += size;
      progress.data.db.transferred_records++;

      if (report_progress && report_progress(socket, &progress) == PI_TRANSFER_STOP) {
        result = pi_set_error(socket, PI_ERR_FILE_ABORTED);
        goto fail;
      }
      
      /* If we see a 'boot' section, regardless of file
         type, require reset */
      if (type == pi_mktag('b', 'o', 'o', 't'))
        reset = 1;
    }
  } else {
    for (j = 0; j < pf->num_entries; j++) {
      int attr, category;
      uint32_t resource_id;

      if ((result = pi_file_read_record(pf, j, &buffer, &size, &attr, &category, &resource_id)) < 0)
        goto fail;

      /* Old OS version cannot install deleted records, so
         don't even try */
      if ((attr & (dlpRecAttrArchived | dlpRecAttrDeleted)) && version < 0x0101)
        continue;

      if ((result = dlp_WriteRecord(socket, db, attr, resource_id, category, buffer, size, 0)) < 0)
        goto fail;

      progress.transferred_bytes += size;
      progress.data.db.transferred_records++;

      if (report_progress && report_progress(socket, &progress) == PI_TRANSFER_STOP) {
        result = pi_set_error(socket, PI_ERR_FILE_ABORTED);
        goto fail;
      }
    }
  }

  if (reset) {
    dlp_ResetSystem(socket);
  }

  return dlp_CloseDB(socket, db);

fail:
  /* save error codes then restore them after closing/deleting the DB */
  err1 = pi_error(socket);
  err2 = pi_palmos_error(socket);

  LOG((PI_DBG_API, PI_DBG_LVL_ERR, "FILE INSTALL error: pilot-link " "0x%04x, PalmOS 0x%04x\n", err1, err2));
  if (db != -1 && pi_socket_connected(socket)) {
    dlp_CloseDB(socket, db);
  }
  if (pi_socket_connected(socket)) {
    dlp_DeleteDB(socket, cardno, pf->info.name);
  }

  pi_set_error(socket, err1);
  pi_set_palmos_error(socket, err2);

  if (result >= 0) {
    result = pi_set_error(socket, PI_ERR_FILE_ERROR);
  }

  return result;
}
#endif

static int compareText(void *e1, void *e2, void *other) {
  char *s1 = *((char **)e1);
  char *s2 = *((char **)e2);

  return StrCaselessCompare(s1, s2);
}

static void freeLocalDatabases(hotsync_data_t *data) {
  int i;

  if (data->localItemsText) {
    for (i = 0; i < data->localNumItems; i++) {
      if (data->localItemsText[i]) xfree(data->localItemsText[i]);
    }
    xfree(data->localItemsText);
    data->localItemsText = NULL;
  }
  data->localNumItems = 0;
}

static void freeRemoteDatabases(hotsync_data_t *data) {
  int i;

  if (data->remoteItemsText) {
    for (i = 0; i < data->remoteNumItems; i++) {
      if (data->remoteItemsText[i]) xfree(data->remoteItemsText[i]);
    }
    xfree(data->remoteItemsText);
    data->remoteItemsText = NULL;
  }
  data->remoteNumItems = 0;
}

static void listLocalDatabases(hotsync_data_t *data) {
  LocalID dbID;
  char name[dmDBNameLength];
  int i, k;

  data->localNumItems = DmNumDatabases(0);
  data->localItemsText = xcalloc(data->localNumItems, sizeof(char *));

  for (i = 0, k = 0; i < data->localNumItems; i++) {
    if ((dbID = DmGetDatabase(0, i)) != 0) {
      if (DmDatabaseInfo(0, dbID, name, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL) == errNone) {
        data->localItemsText[k++] = xstrdup(name);
      }
    }
  }

  if (k > 0) {
    data->localItemsText = xrealloc(data->localItemsText, k * sizeof(char *));
    SysQSortP(data->localItemsText, k, sizeof(char *), compareText, NULL);
  } else {
    xfree(data->localItemsText);
    data->localItemsText = NULL;
  }
  data->localNumItems = k;
  debug(DEBUG_INFO, SYS_DEBUG, "listed %d local database(s)", data->localNumItems);
}

static void listRemoteDatabases(hotsync_data_t *data) {
  pi_buffer_t *buf;
  struct DBInfo info;
  int i, j, k, num;

  pi_reset_errors(data->fd);

  buf = pi_buffer_new(sizeof(struct DBInfo));
  if (buf == NULL) {
    pi_set_error(data->fd, PI_ERR_GENERIC_MEMORY);
    return;
  }

  num = 64;
  data->remoteItemsText = xcalloc(num, sizeof(char *));

  i = k = 0;
  while (dlp_ReadDBList(data->fd, 0, dlpDBListRAM | dlpDBListROM | dlpDBListMultiple, i, buf) >= 0) {
    for (j = 0; j < (int)(buf->used / sizeof(struct DBInfo)); j++) {
      xmemcpy(&info, buf->data + j * sizeof(struct DBInfo), sizeof(struct DBInfo));
      data->remoteItemsText[k++] = xstrdup(info.name);
      if (k == num) {
        num += 64;
        data->remoteItemsText = xrealloc(data->remoteItemsText, num * sizeof(char *));
      }
      i = info.index + 1;
    }
  }

  pi_buffer_free(buf);

  if (k > 0) {
    data->remoteItemsText = xrealloc(data->remoteItemsText, k * sizeof(char *));
    SysQSortP(data->remoteItemsText, k, sizeof(char *), compareText, NULL);
  } else {
    xfree(data->remoteItemsText);
    data->remoteItemsText = NULL;
  }
  data->remoteNumItems = k;
  debug(DEBUG_INFO, SYS_DEBUG, "listed %d remote database(s)", data->remoteNumItems);
}

static Boolean recvDatabase(int client, char *dbname) {
  SysNotifyParamType notify;
  struct DBInfo info;
  struct pi_file *f;
  char path[256];
  LocalID dbID;
  Boolean r = false;

  debug(DEBUG_INFO, SYS_DEBUG, "receiving database '%s'", dbname);

  if (dlp_FindDBInfo(client, 0, 0, dbname, 0, 0, &info) == 0) {
    StrCopy(path, "vfs/app_install/tmpdb.");
    if (info.flags & dlpDBFlagResource) {
      StrCat(path, "prc");
    } else {
      StrCat(path, "pdb");
    }

    info.flags &= 0x2fd;
    if ((f = pi_file_create(path, &info)) != NULL) {
      if (file_retrieve(f, client, 0, fetch_progress) == 0) {
        pi_file_close(f);
        if ((dbID = DmFindDatabase(0, dbname)) != 0) {
          DmDeleteDatabase(0, dbID);
        }
        MemSet(&notify, sizeof(notify), 0);
        notify.notifyType = sysNotifySyncFinishEvent;
        notify.broadcaster = sysNotifyBroadcasterCode;
        SysNotifyBroadcast(&notify);
        r = true;
      } else {
        pi_file_close(f);
        dialog(DEBUG_ERROR, "Error receiving '%s'.", dbname);
      }
    } else {
      dialog(DEBUG_ERROR, "Error saving '%s'.", dbname);
    }
  } else {
    dialog(DEBUG_ERROR, "Unable to locate database '%s'.", dbname);
  }

  return r;
}

static Boolean sendDatabase(int client, char *dbname) {
  debug(DEBUG_INFO, SYS_DEBUG, "sending database '%s'", dbname);

/*
  if (dlp_OpenConduit(client) < 0) {
    dialog(DEBUG_ERROR, "Error initializing conduit.");
    return false;
  }
*/

  if (db_install(dbname, client) < 0) {
    debug(DEBUG_ERROR, SYS_DEBUG, "db_install failed (%d, PalmOS error 0x%04x)", pi_error(client), pi_palmos_error(client));
    return false;
  }

  return true;
}

static void showObject(FormType *frm, UInt16 id, Boolean show) {
  UInt16 index;

  index = FrmGetObjectIndex(frm, id);
  if (show) {
    FrmShowObject(frm, index);
  } else {
    FrmHideObject(frm, index);
  }
}

static void showInitialForm(void) {
  FormType *frm;

  frm = FrmGetActiveForm();
  FrmSetTitle(frm, "Hotsync");

  showObject(frm, serialBtn, true);
  showObject(frm, networkBtn, true);
  showObject(frm, localBtn, false);
  showObject(frm, remoteBtn, false);
  showObject(frm, clientFld, false);
  showObject(frm, dbList, false);
  showObject(frm, transferBtn, false);
  showObject(frm, stopBtn, false);
}

static void showConnectedForm(hotsync_data_t *data) {
  FormType *frm;
  ControlType *ctl;
  UInt16 index;

  frm = FrmGetActiveForm();

  index = FrmGetObjectIndex(frm, transferBtn);
  ctl = (ControlType *)FrmGetObjectPtr(frm, index);
  CtlSetLabel(ctl, data->remote ? "Download" : "Upload");

  showObject(frm, serialBtn, false);
  showObject(frm, networkBtn, false);
  showObject(frm, localBtn, true);
  showObject(frm, remoteBtn, true);
  showObject(frm, clientFld, true);
  showObject(frm, dbList, true);
  showObject(frm, transferBtn, true);
  showObject(frm, stopBtn, true);
}

static void FldInsertStr(FormPtr frm, UInt16 id, char *str) {
  FieldType *fld;
  UInt16 index, len;
  FieldAttrType attr;
  Boolean old;

  index = FrmGetObjectIndex(frm, id);
  fld = (FieldType *)FrmGetObjectPtr(frm, index);
  if (fld == NULL) return;

  FldGetAttributes(fld, &attr);
  old = attr.editable;
  attr.editable = true;
  FldSetAttributes(fld, &attr);

  len = FldGetTextLength(fld);
  if (len) {
    FldDelete(fld, 0, len);
  }
  if (str && str[0]) {
    FldInsert(fld, str, StrLen(str));
  } else {
    FldInsert(fld, "", 0);
  }

  attr.editable = old;
  FldSetAttributes(fld, &attr);
}

static Boolean MainFormHandleEvent(EventPtr event) {
  hotsync_data_t *data = pumpkin_get_data();
  struct PilotUser user;
  FormType *frm;
  ControlType *ctl;
  ListType *lst;
  UInt16 index;
  int fd;
  Boolean handled = false;

  switch (event->eType) {
    case frmOpenEvent:
      frm = FrmGetActiveForm();
      index = FrmGetObjectIndex(frm, localBtn);
      ctl = (ControlType *)FrmGetObjectPtr(frm, index);
      CtlSetValue(ctl, 1);
      index = FrmGetObjectIndex(frm, dbList);
      lst = (ListType *)FrmGetObjectPtr(frm, index);
      LstSetListChoices(lst, data->localItemsText, data->localNumItems);
      showInitialForm();
      FrmDrawForm(frm);
      handled = true;
      break;

    case frmUpdateEvent:
      frm = FrmGetActiveForm();
      FrmDrawForm(frm);
      handled = true;
      break;

    case menuEvent:
      switch (event->data.menu.itemID) {
        case aboutCmd:
          AbtShowAboutPumpkin(AppID);
          break;
      }
      handled = true;
      break;

    case lstSelectEvent:
      if (data->remote) {
        data->remoteSelected = event->data.lstSelect.selection;
      } else {
        data->localSelected = event->data.lstSelect.selection;
      }
      break;

    case ctlSelectEvent:
      frm = FrmGetActiveForm();
      index = FrmGetObjectIndex(frm, dbList);
      lst = (ListType *)FrmGetObjectPtr(frm, index);

      switch (event->data.ctlSelect.controlID) {
        case serialBtn:
          debug(DEBUG_INFO, SYS_DEBUG, "starting serial server");
          data->fd = plu_connect("srm:SerP");
          if (data->fd < 0) {
            data->fd = 0;
            dialog(DEBUG_ERROR, "Server did not start.");
          } else {
            FrmSetTitle(frm, "Hotsync: Serial");
            FldInsertStr(frm, clientFld, "Waiting");
            showConnectedForm(data);
            FrmUpdateForm(MainForm, 0);
          }
          break;

        case networkBtn:
          debug(DEBUG_INFO, SYS_DEBUG, "starting network server");
          data->fd = plu_connect("net:any");
          if (data->fd < 0) {
            data->fd = 0;
            dialog(DEBUG_ERROR, "Server did not start.");
          } else {
            FrmSetTitle(frm, "Hotsync: Network");
            FldInsertStr(frm, clientFld, "Waiting");
            showConnectedForm(data);
            FrmUpdateForm(MainForm, 0);
          }
          break;

        case stopBtn:
          if (data->fd) {
            debug(DEBUG_INFO, SYS_DEBUG, "stopping server");
            plu_close(data->fd);
            data->fd = 0;
            data->connected = false;
            showInitialForm();
            FrmUpdateForm(MainForm, 0);
          }
          break;

        case localBtn:
          if (data->remote) {
            data->remote = false;
            if (data->refresh) {
              pumpkin_local_refresh();
              freeLocalDatabases(data);
              listLocalDatabases(data);
              data->refresh = false;
            }
            index = FrmGetObjectIndex(frm, transferBtn);
            ctl = (ControlType *)FrmGetObjectPtr(frm, index);
            CtlSetLabel(ctl, "Upload");
            LstSetListChoices(lst, data->localItemsText, data->localNumItems);
            LstSetSelection(lst, data->localSelected);
            LstMakeItemVisible(lst, data->localSelected);
            LstDrawList(lst);
          }
          break;
        case remoteBtn:
          if (!data->remote) {
            data->remote = true;
            if (data->refresh) {
              freeRemoteDatabases(data);
              listRemoteDatabases(data);
              data->refresh = false;
            }
            index = FrmGetObjectIndex(frm, transferBtn);
            ctl = (ControlType *)FrmGetObjectPtr(frm, index);
            CtlSetLabel(ctl, "Download");
            LstSetListChoices(lst, data->remoteItemsText, data->remoteNumItems);
            LstSetSelection(lst, data->remoteSelected);
            LstMakeItemVisible(lst, data->localSelected);
            LstDrawList(lst);
          }
          break;
        case transferBtn:
          if (data->connected) {
            if (data->remote) {
              if (recvDatabase(data->fd, data->remoteItemsText[LstGetSelection(lst)])) {
                FldInsertStr(frm, clientFld, "Downloaded");
                data->refresh = true;
              } else {
                FldInsertStr(frm, clientFld, "Error");
              }
            } else {
              if (sendDatabase(data->fd, data->localItemsText[LstGetSelection(lst)])) {
                FldInsertStr(frm, clientFld, "Uploaded");
                data->refresh = true;
              } else {
                FldInsertStr(frm, clientFld, "Error");
              }
            }
          } else {
            dialog(DEBUG_INFO, "Client is not connected.");
          }
          break;
      }
      handled = true;
      break;

    case nilEvent:
      if (data->fd && !data->connected) {
        fd = plu_accept(data->fd, 0);
        if (fd > 0) {
          data->fd = fd;
          data->connected = true;
          if (dlp_ReadUserInfo(data->fd, &user) >= 0) {
            debug(DEBUG_INFO, SYS_DEBUG, "user \"%s\"", user.username);
          }
          freeRemoteDatabases(data);
          listRemoteDatabases(data);
          frm = FrmGetActiveForm();
          FldInsertStr(frm, clientFld, "Connected");
        } else if (fd < 0) {
          dialog(DEBUG_ERROR, "Client was not accepted.");
          data->fd = 0;
          data->connected = false;
          frm = FrmGetActiveForm();
          showInitialForm();
          FrmUpdateForm(MainForm, 0);
        }
      }
      break;

    default:
      break;
  }

  return handled;
}

static Boolean ApplicationHandleEvent(EventPtr event) {
  FormPtr frm;
  UInt16 formID;
  Boolean handled;

  handled = false;

  switch (event->eType) {
    case frmLoadEvent:
      formID = event->data.frmLoad.formID;
      frm = FrmInitForm(formID);
      FrmSetActiveForm(frm);
      switch (formID) {
        case MainForm:
          FrmSetEventHandler(frm, MainFormHandleEvent);
          break;
      }
      handled = true;
      break;
    case frmCloseEvent:
      break;
    default:
      break;
  }

  return handled;
}

static void startApplication(void) {
  hotsync_data_t *data;

  data = xcalloc(1, sizeof(hotsync_data_t));
  pumpkin_set_data(data);
  listLocalDatabases(data);

  pi_debug_set_types(PI_DBG_ALL);
}

static void stopApplication(void) {
  hotsync_data_t *data = pumpkin_get_data();

  freeRemoteDatabases(data);
  freeLocalDatabases(data);

  if (data->fd > 0) {
    debug(DEBUG_INFO, SYS_DEBUG, "stopping server");
    plu_close(data->fd);
  }

  xfree(data);
}

static void EventLoop(void) {
  hotsync_data_t *data = pumpkin_get_data();
  EventType event;
  Err err;

  do {
    EvtGetEvent(&event, data->fd > 0 && !data->connected ? 10 : evtWaitForever);
    if (SysHandleEvent(&event)) continue;
    if (MenuHandleEvent(NULL, &event, &err)) continue;
    if (ApplicationHandleEvent(&event)) continue;
    FrmDispatchEvent(&event);

  } while (event.eType != appStopEvent);
}

UInt32 PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags) {
  switch (cmd) {
    case sysAppLaunchCmdNormalLaunch:
      break;
    default:
      return 0;
  }

  startApplication();
  FrmGotoForm(MainForm);
  EventLoop();
  FrmCloseAllForms();
  stopApplication();

  return 0;
}
