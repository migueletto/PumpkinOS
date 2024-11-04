#include <PalmOS.h>
#include <VFSMgr.h>

#ifdef ARMEMU
#include "armemu.h"
#endif
#include "m68k/m68k.h"
#include "m68k/m68kcpu.h"
#include "emupalmosinc.h"
#include "emupalmos.h"
#include "plibc.h"
#include "heap.h"
#include "tos.h"
#include "gemdos.h"
#include "gemdos_proto.h"
#include "bios_proto.h"
#include "debug.h"

#define AES_RETURN_INT(r) \
  aes_pb->intout_len = 1; \
  aes_pb->intout[0] = r

int32_t aes_call(aes_pb_t *aes_pb) {
  uint8_t *ram = pumpkin_heap_base();
  int32_t i, r = 0;

  debug(DEBUG_INFO, "TOS", "AES opcode %d", aes_pb->opcode);

  switch (aes_pb->opcode) {
    case 10: { // int16_t appl_init(void)
        // global[0]      Version number of the AES
        // global[1]      Number of applications that can run concurrently; with a value of -1 there is no limit
        // global[2]      Unique ID number of the application
        // global[3,4]    Miscellaneous information that only has meanning for the application, and can be set and read by it
        // global[5,6]    Pointer to a list of pointers to the object trees of the application (is set by rsrc_load)
        // global[7,8]    Address of the memory reserved for the resource file. Only documented by Digital Research and not by Atari.
        // global[9]      Length of the reserved memory. Only documented by Digital Research and not by Atari.
        // global[10]     Number of colour planes. Only documented by Digital Research and not by Atari.
        // global[11,12]  Reserved
        // global[13]     Maximum height of a character, which is used by the AES for the vst_height call.
        // global[14]     Minimum height of a character, which is used by the AES for the vst_height call.

        debug(DEBUG_INFO, "TOS", "AES appl_init()");

        for (i = 0; i < AES_GLOBAL_LEN; i++) {
          aes_pb->global[i] = 0;
        }
        aes_pb->global[0] = 1;
        aes_pb->global[1] = 1;
        aes_pb->global[2] = 1; // XXX Unique ID number of the application

        AES_RETURN_INT(aes_pb->global[2]);
      }
      break;
    case 11: { // int16_t appl_read(int16_t ap_rid, int16_t ap_rlength, void *ap_rpbuff)
        int16_t ap_rid = aes_pb->intin[0];
        int16_t ap_rlength = aes_pb->intin[1];
        void *ap_rpbuf = aes_pb->adrin[0] ? (void *)(ram + aes_pb->adrin[0]) : NULL;
        debug(DEBUG_INFO, "TOS", "AES appl_read(%d %d, 0x%08X)", ap_rid, ap_rlength, aes_pb->adrin[0]);
        AES_RETURN_INT(0); // 0 is returned then an error has occurred, else a positive number is returned
      }
      break;
    default:
      debug(DEBUG_ERROR, "TOS", "unmapped AES opcode %d", aes_pb->opcode);
      r = -1;
      break;
  }

  return r;
}
