#include <PalmOS.h>
#include <VFSMgr.h>
#include <INetMgr.h>
#include <HsNavCommon.h>
#include <HsExtTraps.h>
#include <HsExt.h>

#include "sys.h"
#ifdef ARMEMU
#include "armemu.h"
#include "armp.h"
#endif
#include "pumpkin.h"
#include "logtrap.h"
#include "m68k/m68k.h"
#include "m68k/m68kcpu.h"
#include "emupalmos.h"
#include "debug.h"
