#include "sys.h"
#include "dlm.h"
#include "thread.h"
#include "ptr.h"
#include "vfs.h"
#include "vfslocal.h"
#include "disk.h"
#include "script.h"
#include "debug.h"

extern int libos_app_init(int pe);
extern int libos_start(int pe);

int liblsdl2_load(void);
int liblsdl2_init(int pe, script_ref_t obj);

extern struct malloc_state dlmalloc_state;

int main(int argc, char *argv[]) {
  dlm_init(NULL);
  sys_init();
  debug_setsyslevel(NULL, DEBUG_INFO);
  //debug_setsyslevel("STOR", DEBUG_TRACE);
  debug_init(NULL);
  ptr_init();
  thread_init();

  disk_init();
  vfs_init();

  liblsdl2_load();
  liblsdl2_init(0, 0);

  vfs_local_mount("./vfs/", "/");
  libos_app_init(0);
  libos_start(0);

  vfs_finish();
  thread_close();
  debug_close();

  return 0;
}
