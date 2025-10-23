#include <PalmOS.h>

#include "pumpkin.h"
#include "RegistryMgr.h"
#include "bytes.h"
#include "vfs.h"
#include "util.h"
#include "deploy.h"
#include "debug.h"

void pumpkin_remove_locks(vfs_session_t *session, char *path) {
  vfs_dir_t *dir;
  vfs_ent_t *ent;
  char buf[VFS_PATH];

  if ((dir = vfs_opendir(session, path)) != NULL) {
    for (;;) {
      ent = vfs_readdir(dir);
      if (ent == NULL) break;
      if (ent->type != VFS_DIR) continue;
      if (ent->name[0] == '.') continue;
      sys_snprintf(buf, VFS_PATH-1, "%s%s/lock", path, ent->name);
      vfs_unlink(session, buf);
    }
    vfs_closedir(dir);
  }
}

void pumpkin_registry_create(UInt32 creator) {
  RegFlagsType *regFlagsP, regFlags;
  RegOsType regOS;
  RegDimensionType regDim;
  RegPositionType regPos;
  DmOpenRef dbRef;
  MemHandle h;
  UInt32 regSize;
  UInt16 width, height;
  UInt8 *ptr;
  int swidth, sheight;

  pumpkin_get_window(&swidth, &sheight);
  width = APP_SCREEN_WIDTH;
  height = APP_SCREEN_HEIGHT;
  if (pumpkin_get_density() == kDensityLow) {
    width /= 2;
    height /= 2;
  }

  if ((dbRef = DmOpenDatabaseByTypeCreator(sysFileTApplication, creator, dmModeReadOnly)) != NULL)  {
    if ((h = DmGet1Resource(sysRsrcTypeWinD, 1)) != NULL) {
      if ((ptr = MemHandleLock(h)) != NULL) {
        get2b(&width, ptr, 0);
        get2b(&height, ptr, 2);
        if (pumpkin_get_density() == kDensityLow) {
          width /= 2;
          height /= 2;
        }
        MemHandleUnlock(h);
      }
      DmReleaseResource(h);
    }
    DmCloseDatabase(dbRef);
  }

  if ((regFlagsP = pumpkin_reg_get(creator, regFlagsID, &regSize)) != NULL) {
    MemPtrFree(regFlagsP);
  } else {
    regFlags.flags = regFlagReset;
    pumpkin_reg_set(creator, regFlagsID, &regFlags, sizeof(RegFlagsType));
  }

  regOS.version = pumpkin_get_default_osversion();
  pumpkin_reg_set(creator, regOsID, &regOS, sizeof(RegOsType));

  regDim.width = width;
  regDim.height = height;
  pumpkin_reg_set(creator, regDimensionID, &regDim, sizeof(RegDimensionType));

  regPos.x = (swidth - regDim.width) / 2;
  regPos.y = (sheight - regDim.height) / 2;
  pumpkin_reg_set(creator, regPositionID, &regPos, sizeof(RegPositionType));
}

static int pumpkin_deploy_file_session(vfs_session_t *session, char *path) {
  vfs_file_t *f;
  LocalID dbID;
  UInt32 type, creator;
  char name[dmDBNameLength], stype[8], screator[8], *ext;
  uint32_t size, hsize;
  uint8_t *p;
  int r = -1;

  if (path && (ext = getext(path)) != NULL && (!sys_strcasecmp(ext, "prc") || !sys_strcasecmp(ext, "pdb"))) {
    if ((f = vfs_open(session, path, VFS_READ)) != NULL) {
      size = vfs_seek(f, 0, 1);
      hsize = 78;
      if (size > hsize && size != 0xfffffffful) {
        vfs_seek(f, 0, 0);
        if ((p = MemPtrNew(size)) != NULL) {
          if (vfs_read(f, p, hsize) == hsize) {
            sys_memset(name, 0, dmDBNameLength);
            sys_memcpy(name, p, dmDBNameLength - 1);
            if (name[0]) {
              if ((dbID = DmFindDatabase(0, name)) != 0) {
                debug(DEBUG_INFO, PUMPKINOS, "deleting old version of \"%s\"", name);
                DmDeleteDatabase(0, dbID);
              }
              if (vfs_read(f, p + hsize, size - hsize) == size - hsize) {
                get4b(&type, p, dmDBNameLength + 28);
                get4b(&creator, p, dmDBNameLength + 32);
                pumpkin_id2s(type, stype);
                pumpkin_id2s(creator, screator);
                debug(DEBUG_INFO, PUMPKINOS, "installing new version of \"%s\" type '%s' creator '%s' from \"%s\"", name, stype, screator, path);

                if (DmCreateDatabaseFromImage(p) == errNone) {
                  if ((dbID = DmFindDatabase(0, name)) != 0) {
                    debug(DEBUG_INFO, PUMPKINOS, "installed \"%s\"", name);
                    if (type == sysFileTApplication) {
                      pumpkin_registry_create(creator);
                    }
                    r = 0;
                  }
                } else {
                    debug(DEBUG_ERROR, PUMPKINOS, "error installing \"%s\"", name);
                }
              }
            }
          }
          MemPtrFree(p);
        }
      }
      vfs_close(f);
    }
  }

  return r;
}

int pumpkin_deploy_from_image(vfs_session_t *session, uint8_t *p, uint32_t size) {
  LocalID dbID;
  UInt32 type, creator;
  char name[dmDBNameLength], stype[8], screator[8];
  uint32_t hsize;
  int r = -1;

  hsize = 78;
  if (size > hsize) {
    sys_memset(name, 0, dmDBNameLength);
    sys_memcpy(name, p, dmDBNameLength - 1);
    if (name[0]) {
      if ((dbID = DmFindDatabase(0, name)) != 0) {
        debug(DEBUG_INFO, PUMPKINOS, "deleting old version of \"%s\"", name);
        DmDeleteDatabase(0, dbID);
      }
      get4b(&type, p, dmDBNameLength + 28);
      get4b(&creator, p, dmDBNameLength + 32);
      pumpkin_id2s(type, stype);
      pumpkin_id2s(creator, screator);
      debug(DEBUG_INFO, PUMPKINOS, "installing new version of \"%s\" type '%s' creator '%s' from memory", name, stype, screator);

      if (DmCreateDatabaseFromImage(p) == errNone) {
        debug(DEBUG_INFO, PUMPKINOS, "installed \"%s\"", name);
        if (type == sysFileTApplication) {
          pumpkin_registry_create(creator);
        }
        r = 0;
      } else {
        debug(DEBUG_ERROR, PUMPKINOS, "error installing \"%s\"", name);
      }
    }
  }

  return r;
}

int pumpkin_deploy_files_session(vfs_session_t *session, char *path) {
  vfs_dir_t *dir;
  vfs_ent_t *ent;
  char *ext, buf[VFS_PATH];
  int rr, r = -1;

  if (path) {
    if ((dir = vfs_opendir(session, path)) != NULL) {
      for (r = 0; r == 0;) {
        if ((ent = vfs_readdir(dir)) == NULL) break;
        if (ent->type != VFS_FILE) continue;
        ext = getext(ent->name);
        if (!sys_strcasecmp(ext, "prc") || !sys_strcasecmp(ext, "pdb")) {
          sys_memset(buf, 0, sizeof(buf));
          sys_snprintf(buf, sizeof(buf)-1, "%s/%s", path, ent->name);
          rr = pumpkin_deploy_file_session(session, buf);
          vfs_unlink(session, buf);
          if (rr == 0) {
            r = 0;
          } else {
            sys_snprintf(buf, sizeof(buf)-1, "Error deploying \"%s\"", ent->name);
            debug(DEBUG_ERROR, PUMPKINOS, "pumpkin_deploy_files: %s", buf);
          }
        } else {
          sys_snprintf(buf, sizeof(buf)-1, "%s/%s", path, ent->name);
          vfs_unlink(session, buf);
          sys_snprintf(buf, sizeof(buf)-1, "Cannot deploy file \"%s\"", ent->name);
          debug(DEBUG_ERROR, PUMPKINOS, "pumpkin_deploy_files: %s", buf);
        }
      }
      vfs_closedir(dir);
    }
  }

  return r;
}
