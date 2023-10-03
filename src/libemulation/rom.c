#include "sys.h"
#include "vfs.h"
#include "debug.h"

int load_rom(vfs_session_t *session, char *name, int size, uint8_t *buf) {
  vfs_file_t *file;
  int n;

  if (!session || !name || !buf) return -1;

  if ((file = vfs_open(session, name, VFS_READ)) == NULL) {
    debug(DEBUG_ERROR, "SYS", "\"%s\" not found", name);
    return -1;
  }

  if ((n = vfs_read(file, buf, size)) == -1) {
    vfs_close(file);
    return -1;
  }
  vfs_close(file);

  return 0;
}
