#include <PalmOS.h>
#include <VFSMgr.h>
#include <DLServer.h>

#ifdef ARMEMU
#include "armemu.h"
#include "armp.h"
#endif
#include "logtrap.h"
#include "m68k/m68k.h"
#include "m68k/m68kcpu.h"
#include "emupalmosinc.h"
#include "emupalmos.h"
#include "bytes.h"
#include "debug.h"

#define strArg(a) (a ? ((char *)(ram + a)) : "")

#define READ16(a) (((uint16_t *)ram)[(a)>>1])
#define READ32(a) (((uint32_t *)ram)[(a)>>2])

#undef ARG8
#undef ARG16
#undef ARG32

#define ARG8  0
#define ARG16 0
#define ARG32 0

typedef struct {
  UInt32 dbType;
  UInt32 dbCreator;
  UInt32 revision;
  UInt32 entries;
  UInt32 rsrcTypeData0;
  UInt32 rsrcTypeCode0;
  UInt32 rsrcTypeCode1;
  UInt16 rsrcIDData0;
  UInt16 rsrcIDCode0;
  UInt16 rsrcIDCode1;
  UInt16 reserved;
} SysModuleDescriptorType;

uint32_t emupalmos_arm_syscall(uint32_t group, uint32_t function, uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3, uint32_t sp) {
  uint8_t *ram = pumpkin_heap_base();
  char st[8], st2[8];
  UInt32 value, r;
  Err err;

  switch (group) {
    case 1: // DAL
      debug(DEBUG_ERROR, "ARM", "unmapped arm syscall 0x%04X in DAL", function);
      r0 = 0;
      break;
    case 2: // BOOT
      switch (function) {
        case 0x50: {
          // BitmapType *BmpCreate(Coord width, Coord height, UInt8 depth, ColorTableType *colorTableP, UInt16 *error)
          ColorTableType *colorTableP = r3 ? (ColorTableType *)(ram + r3) : NULL;
          UInt16 error;
          BitmapType *bmp = BmpCreate(r0, r1, r2, colorTableP, &error);
          r = bmp ? (uint8_t *)bmp - ram : 0;
          debug(DEBUG_TRACE, "ARM", "arm syscall BmpCreate(%d, %d, %d, 0x%08X): 0x%08X", r0, r1, r2, r3, r);
          r0 = r;
          }
          break;
        case 0x58: {
          // UInt8 BmpGetBitDepth(const BitmapType *bitmapP)
          BitmapType *bmp = r0 ? (BitmapType *)(ram + r0) : NULL;
          r = BmpGetBitDepth(bmp);
          debug(DEBUG_TRACE, "ARM", "arm syscall BmpGetBitDepth(0x%08X): %d", r0, r);
          r0 = r;
          }
          break;
        case 0x5C:
          // Err BmpCompress(BitmapType *bitmapP, BitmapCompressionType compType)
          BitmapType *bmp = r0 ? (BitmapType *)(ram + r0) : NULL;
          r1 &= 0xff;
          err = BmpCompress(bmp, r1);
          debug(DEBUG_TRACE, "ARM", "arm syscall BmpCompress(0x%08X, 0x%02X): %d", r0, r1, err);
          //r0 = err;
          // XXX apparently, in ARM syscall, BmpCompress returns not the error value, but the bitmap bits.
          // Bike or Die 2 uses the value returned by BmpCompress as the 3rd argument to BmpCreateBitmapV3, that is, the address of the bits.
          // This is super weird, but I will just do that here.
          r0 = (uint8_t *)BmpGetBits(bmp) - ram;
          break;
        case 0x54: {
          // Err BmpDelete(BitmapType *bitmapP)
          BitmapType *bmp = r0 ? (BitmapType *)(ram + r0) : NULL;
          err = BmpDelete(bmp);
          debug(DEBUG_TRACE, "ARM", "arm syscall BmpDelete(0x%08X): %d", r0, err);
          r0 = err;
          }
          break;
        case 0x64: {
          // void BmpGetDimensions(const BitmapType *bitmapP, Coord *widthP, Coord *heightP, UInt16 *rowBytesP)
          BitmapType *bmp = r0 ? (BitmapType *)(ram + r0) : NULL;
          Coord *widthP = r1 ? (Coord *)(ram + r1) : NULL;
          Coord *heightP = r2 ? (Coord *)(ram + r2) : NULL;
          UInt16 *rowBytesP = r3 ? (UInt16 *)(ram + r3) : NULL;
          BmpGetDimensions(bmp, widthP, heightP, rowBytesP);
          debug(DEBUG_TRACE, "ARM", "arm syscall BmpGetDimensions(0x%08X, 0x%08X, 0x%08X, 0x%08X)", r0, r1, r2, r3);
          }
          break;
        case 0x104: {
          // Err DlkGetSyncInfo(UInt32 *succSyncDateP, UInt32 *lastSyncDateP, DlkSyncStateType *syncStateP, Char *nameBufP, Char *logBufP, Int32 *logLenP)
          UInt32 *succSyncDateP = r0 ? (UInt32 *)(ram + r0) : NULL;
          UInt32 *lastSyncDateP = r1 ? (UInt32 *)(ram + r1) : NULL;
          DlkSyncStateType *syncStateP = r2 ? (DlkSyncStateType *)(ram + r2) : NULL;
          char *nameBufP = r3 ? (char *)(ram + r3) : NULL;
          //char *logBufP = r4 ? (char *)(ram + r4) : NULL;
          //Int32 *logLenP = r5 ? (Int32 *)(ram + r5) : NULL;
          err = DlkGetSyncInfo(succSyncDateP, lastSyncDateP, syncStateP, nameBufP, NULL, NULL);
          debug(DEBUG_TRACE, "ARM", "arm syscall DlkGetSyncInfo(0x%08X, 0x%08X, 0x%08X, 0x%08X): %d", r0, r1, r2, r3, err);
          r0 = err;
          }
          break;
        case 0x170: {
          // UInt16 DmFindResource(DmOpenRef dbP, DmResType resType, DmResID resID, MemHandle resH)
          DmOpenRef dbP = r0 ? (DmOpenRef)(ram + r0) : NULL;
          MemHandle resH = r3 ? (MemHandle)(ram + r3) : NULL;
          r = DmFindResource(dbP, r1, r2, resH);
          pumpkin_id2s(r1, st);
          debug(DEBUG_TRACE, "ARM", "arm syscall DmFindResource(0x%08X, '%s', %u, 0x%08X): %u", r0, st, r2, r3, r);
          r0 = r;
          }
          break;
        case 0x180: {
          // MemHandle DmGet1Resource(DmResType type, DmResID resID)
          MemHandle h = DmGet1Resource(r0, r1);
          r = h ? (uint8_t *)h - ram : 0;
          pumpkin_id2s(r0, st);
          debug(DEBUG_TRACE, "ARM", "arm syscall DmGet1Resource('%s', %u): 0x%08X", st, r1, r);
          r0 = r;
          }
          break;
        case 0x1A4: {
          // MemHandle DmGetResource(DmResType type, DmResID resID)
          MemHandle h = DmGetResource(r0, r1);
          r = h ? (uint8_t *)h - ram : 0;
          pumpkin_id2s(r0, st);
          debug(DEBUG_TRACE, "ARM", "arm syscall DmGetResource('%s', %u): 0x%08X", st, r1, r);
          r0 = r;
          }
          break;
        case 0x1A8: {
          // MemHandle DmGetResourceIndex(DmOpenRef dbP, UInt16 index)
          DmOpenRef dbP = r0 ? (DmOpenRef)(ram + r0) : NULL;
          MemHandle h = DmGetResourceIndex(dbP, r1);
          r = h ? (uint8_t *)h - ram : 0;
          debug(DEBUG_TRACE, "ARM", "arm syscall DmGetResourceIndex(0x%08X, %u): 0x%08X", r0, r1, r);
          r0 = r;
          }
          break;
        case 0x1E8: {
          // DmOpenRef DmOpenDatabase(LocalID dbID, UInt16 mode) XXX cardNo parameter does not exist 
          DmOpenRef dbP = DmOpenDatabase(0, r0, r1);
          r = dbP ? (uint8_t *)dbP - ram : 0;
          debug(DEBUG_TRACE, "ARM", "arm syscall DmOpenDatabase(0x%08X, 0x%04X): 0x%08X", r0, r1, r);
          r0 = r;
          }
          break;
        case 0x1F0: {
          // Err DmOpenDatabaseInfo(DmOpenRef dbP, LocalID *dbIDP, UInt16 *openCountP, UInt16 *modeP, UInt16 *cardNoP, Boolean *resDBP)
          // XXX 0x1F4 is DmOpenDatabaseInfoV40
          DmOpenRef dbP = r0 ? (DmOpenRef)(ram + r0) : NULL;
          LocalID *dbIDP = r1 ? (LocalID *)(ram + r1) : NULL;
          UInt16 *openCountP = r2 ? (UInt16 *)(ram + r2) : NULL;
          UInt16 *modeP = r3 ? (UInt16 *)(ram + r3) : NULL;
          //Boolean *resDBP = r4 ? (Boolean *)(ram + r4) : NULL;
          Boolean resDB;
          err = DmOpenDatabaseInfo(dbP, dbIDP, openCountP, modeP, NULL, &resDB);
          debug(DEBUG_TRACE, "ARM", "arm syscall DmOpenDatabaseInfo(0x%08X, 0x%08X, 0x%08X, 0x%08X): %d", r0, r1, r2, r3, err);
          r0 = err;
          }
          break;
        case 0x210: {
          debug(DEBUG_TRACE, "ARM", "arm syscall DmQueryRecord(0x%08X, %u)", r0, r1);
          MemHandle h = DmQueryRecord((DmOpenRef)(ram + r0), r1);
          r0 = h ? (uint8_t *)h - ram : 0;
          }
          break;
        case 0x220: {
          // Err DmReleaseResource(MemHandle resourceH)
          MemHandle h = r0 ? (MemHandle)(ram + r0) : NULL;
          err = DmReleaseResource(h);
          debug(DEBUG_TRACE, "ARM", "arm syscall DmReleaseResource(0x%08X): %d", r0, err);
          r0 = err;
          }
          break;
        case 0x2FC: {
          // Err ExgDBRead(ExgDBReadProcPtr readProcP, ExgDBDeleteProcPtr deleteProcP, void *userDataP, LocalID *dbIDP, Boolean *needResetP, Boolean keepDates)
          LocalID *dbID = r3 ? (LocalID *)(ram + r3) : NULL;
          uint32_t addr = *(uint32_t *)(ram + sp);
          Boolean *needReset = (Boolean *)(ram + addr);
          Boolean keepDates = *(uint32_t *)(ram + sp + 4);
          err = ExgDBReadARM(r0, r1, r2, dbID, needReset, keepDates);
          debug(DEBUG_TRACE, "ARM", "arm syscall ExgDBRead(0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, %d): %d", r0, r1, r2, r3, addr, keepDates, err);
          r0 = err;
          }
          break;
        case 0x318:
          // Err ExgGetDefaultApplication(UInt32 *creatorIDP, UInt16 id, const Char *dataTypeP)
          debug(DEBUG_TRACE, "ARM", "arm syscall ExgGetDefaultApplication(0x%08X, 0x%04X, \"%s\") not implemented", r0, r1, strArg(r2));
          r0 = 0xffff;
          break;
        case 0x348:
          // Err ExgRegisterDatatype(UInt32 creatorID, UInt16 id, const Char *dataTypesP, const Char *descriptionsP, UInt16 flags)
          pumpkin_id2s(r0, st);
          debug(DEBUG_TRACE, "ARM", "arm syscall ExgRegisterDatatype('%s', 0x%04X, %p, %p, ___) not implemented", st, r1, strArg(r2), strArg(r3));
          r0 = 0xffff;
          break;
        case 0x354:
          // Err ExgSetDefaultApplication(UInt32 creatorID, UInt16 id, const Char *dataTypeP)
          pumpkin_id2s(r0, st);
          debug(DEBUG_TRACE, "ARM", "arm syscall ExgSetDefaultApplication('%s', 0x%04X, \"%s\") not implemented", st, r1, strArg(r2));
          r0 = 0xffff;
          break;
        case 0x3E0:
          // Int16 FntBaseLine(void)
          r0 = FntBaseLine();
          debug(DEBUG_TRACE, "ARM", "arm syscall FntBaseLine(): %d", r0);
          break;
        case 0x3E4:
          // Int16 FntCharHeight(void)
          r0 = FntCharHeight();
          debug(DEBUG_TRACE, "ARM", "arm syscall FntCharHeight(): %d", r0);
          break;
        case 0x3F0:
          // Int16 FntCharWidth(Char ch)
          r = FntCharWidth(r0);
          debug(DEBUG_TRACE, "ARM", "arm syscall FntCharWidth(%d): %d", r0, r);
          r0 = r;
          break;
        case 0x410:
          // FontID FntSetFont(FontID font)
          r = FntSetFont(r0);
          debug(DEBUG_TRACE, "ARM", "arm syscall FntSetFont(%d): %d", r0, r);
          r0 = r;
          break;
        case 0x424:
          pumpkin_id2s(r0, st);
          debug(DEBUG_TRACE, "ARM", "arm syscall FtrGet('%s', %u, 0x%08X)", st, r1, r2);
          r0 = FtrGet(r0, r1, &value);
          if (r0 == 0 && r2) {
            put4l(value, ram, r2);
          }
          break;
        case 0x43C:
          // Err FtrSet(UInt32 creator, UInt16 featureNum, UInt32 newValue)
          pumpkin_id2s(r0, st);
          err = FtrSet(r0, r1, r2);
          debug(DEBUG_TRACE, "ARM", "arm syscall FtrSet('%s', %u, 0x%08X): %d", st, r1, r2, err);
          r0 = err;
          break;
        case 0x440:
          // Err FtrUnregister(UInt32 creator, UInt16 featureNum)
          pumpkin_id2s(r0, st);
          err = FtrUnregister(r0, r1);
          debug(DEBUG_TRACE, "ARM", "arm syscall FtrUnregister('%s', %u): %d", st, r1, err);
          r0 = err;
          break;
        case 0x4B8: {
          // Err MemChunkFree(MemPtr chunkDataP)
          void *p = r0 ? ram + r0 : NULL;
          err = MemChunkFree(p);
          debug(DEBUG_TRACE, "ARM", "arm syscall MemChunkFree(0x%04X): %d", r0, err);
          r0 = err;
          }
          break;
        case 0x4BC: {
          // MemPtr MemChunkNew(UInt16 heapID, UInt32 size, UInt16 attr)
          MemPtr p = MemChunkNew(r0, r1, r2);
          r = p ? (uint8_t *)p - ram : 0;
          debug(DEBUG_TRACE, "ARM", "arm syscall MemChunkNew(%u, %u, 0x%04X): 0x%08X", r0, r1, r2, r);
          r0 = r;
          break;
          }
        case 0x4DC: {
          MemHandle h = r0 ? (MemHandle)(ram + r0) : NULL;
          r = MemHandleFree(h);
          debug(DEBUG_TRACE, "ARM", "arm syscall MemHandleFree(0x%08X): %d", r0, r);
          r0 = r;
          }
          break;
        case 0x4E4: {
          MemHandle h = r0 ? (MemHandle)(ram + r0) : NULL;
          uint8_t *p = MemHandleLock(h);
          r = p ? p - ram : 0;
          debug(DEBUG_TRACE, "ARM", "arm syscall MemHandleLock(0x%08X): 0x%08X", r0, r);
          r0 = r;
          }
          break;
        case 0x500: {
          // UInt32 MemHandleSize(MemHandle h)
          MemHandle h = r0 ? (MemHandle)(ram + r0) : NULL;
          r = MemHandleSize(h);
          debug(DEBUG_TRACE, "ARM", "arm syscall MemHandleSize(0x%08X): %u", r0, r);
          r0 = r;
          }
          break;
        case 0x508:
          debug(DEBUG_TRACE, "ARM", "arm syscall MemHandleUnlock(0x%08X)", r0);
          r0 = MemHandleUnlock((MemHandle)(ram + r0));
          break;
        case 0x520: {
          // Err MemHeapFreeBytes(UInt16 heapID, UInt32 *freeP, UInt32 *maxP)
          UInt32 *freeP = r1 ? (UInt32 *)(ram + r1) : NULL;
          UInt32 *maxP = r2 ? (UInt32 *)(ram + r2) : NULL;
          err = MemHeapFreeBytes(r0, freeP, maxP);
          debug(DEBUG_TRACE, "ARM", "arm syscall MemHeapFreeBytes(%u, 0x%08X, 0x%08X): %d", r0, r1, r2, err);
          r0 = err;
          }
          break;
        case 0x558: {
          // Err MemMove(void *dstP, const void *sP, Int32 numBytes)
          void *dstP = r0 ? ram + r0 : NULL;
          void *sP = r1 ? ram + r1 : NULL;
          err = MemMove(dstP, sP, r2);
          debug(DEBUG_TRACE, "ARM", "arm syscall MemMove(0x%08X, 0x%08X, %d): %d", r0, r1, r2, err);
          r0 = err;
          }
          break;
        case 0x578: {
          // Boolean MemPtrDataStorage(MemPtr p)
          void *p = r0 ? ram + r0 : NULL;
          r = MemPtrDataStorage(p);
          debug(DEBUG_TRACE, "ARM", "arm syscall MemPtrDataStorage(0x%08X): %d", r0, r);
          r0 = r;
          }
          break;
        case 0x584: {
          // MemPtr MemPtrNew(UInt32 size)
          void *p = MemPtrNew(r0);
          r = p ? (uint8_t *)p - ram : 0;
          debug(DEBUG_TRACE, "ARM", "arm syscall MemPtrNew(%u): 0x%08X", r0, r);
          r0 = r;
          }
          break;
        case 0x58C: {
          // MemHandle MemPtrRecoverHandle(MemPtr p)
          void *p = r0 ? ram + r0 : NULL;
          MemHandle h = MemPtrRecoverHandle(p);
          r = h ? (uint8_t *)h - ram : 0;
          debug(DEBUG_TRACE, "ARM", "arm syscall MemPtrRecoverHandle(0x%08X): 0x%08X", r0, r);
          r0 = r;
          }
          break;
        case 0x5A4: {
          // Err MemPtrUnlock(MemPtr p)
          void *p = r0 ? ram + r0 : NULL;
          err = MemPtrUnlock(p);
          debug(DEBUG_TRACE, "ARM", "arm syscall MemPtrUnlock(0x%08X): %d", r0, err);
          r0 = err;
          }
          break;
        case 0x5B0: {
          // Err MemSet(void *dstP, Int32 numBytes, UInt8 value)
          void *dstP = r0 ? ram + r0 : NULL;
          err = MemSet(dstP, r1, r2);
          debug(DEBUG_TRACE, "ARM", "arm syscall MemSet(0x%08X, %d, %u): %d", r0, r1, r2, err);
          r0 = err;
          }
          break;
        case 0x780: {
          // Int16 StrCompare(const Char *s1, const Char *s2)
          char *s1 = r0 ? (char *)(ram + r0) : NULL;
          char *s2 = r1 ? (char *)(ram + r1) : NULL;
          r = StrCompare(s1, s2);
          debug(DEBUG_TRACE, "ARM", "arm syscall StrCompare(0x%08X, 0x%08X): %d", r0, r1, r);
          r0 = r;
          }
          break;
        case 0x788: {
          // Char *StrCopy(Char *dst, const Char *src)
          void *dst = r0 ? ram + r0 : NULL;
          void *src = r1 ? ram + r1 : NULL;
          char *s = StrCopy(dst, src);
          r = s ? (uint8_t *)s - ram : 0;
          debug(DEBUG_TRACE, "ARM", "arm syscall StrCopy(0x%08X, 0x%08X): 0x%08X", r0, r1, r);
          r0 = r;
          }
          break;
        case 0x798: {
          // UInt16 StrLen(const Char *src)
          void *src = r0 ? ram + r0 : NULL;
          r = StrLen(src);
          debug(DEBUG_TRACE, "ARM", "arm syscall StrLen(0x%08X): %u", r0, r);
          r0 = r;
          }
          break;
        case 0x7B4:
        case 0x7C0: {
          // Int16 StrPrintF(Char *s, const Char *formatStr, ...)
          // Int16 StrVPrintF(Char *s, const Char *formatStr, _Palm_va_list arg)
          char *s = r0 ? (char *)(ram + r0) : NULL;
          char *f = r1 ? (char *)(ram + r1) : NULL;
          int vararg = function == 0x7C0;
          Int16 res = 0;
          if (s && f) {
            int i, j = 0, k = 1, t = 0, sz, arglen = 0;
            uint32_t arg, v_arg;
            char *p, *q, fmt[16];
            if (vararg) {
              v_arg = r2;
            } else {
              v_arg = 0;
            }
            for (i = 0, p = s; f[i]; i++) {
              switch (t) {
                case 0:
                  if (f[i] == '%') {
                    j = 0;
                    fmt[j++] = f[i];
                    arglen = -1;
                    sz = 2;
                    t = 1;
                  } else {
                    *p++ = f[i];
                  }
                  break;
                case 1:
                  switch (f[i]) {
                    case 'h':
                    case 'H':
                      fmt[j++] = f[i];
                      sz = 2;
                      break;
                    case 'l':
                    case 'L':
                      fmt[j++] = f[i];
                      sz = 4;
                      break;
                    case 'd':
                    case 'i':
                    case 'u':
                    case 'x':
                    case 'X':
                      if (vararg) {
                        switch (sz) {
                          case 1:  arg = READ16(v_arg) & 0xff; v_arg += 2; break;
                          case 2:  arg = READ16(v_arg); v_arg += 2; break;
                          case 4:  arg = READ32(v_arg); v_arg += 4; break;
                          default: arg = READ16(v_arg); v_arg += 2; break;
                        }
                      } else {
                        switch (sz) {
                          case 1:  arg = ARG8;  break;
                          case 2:  arg = ARG16; break;
                          case 4:  arg = ARG32; break;
                          default: arg = ARG16; break;
                        }
                      }
                      k++;
                      fmt[j++] = f[i];
                      fmt[j] = 0;
                      sys_sprintf(p, fmt, arg);
                      p += sys_strlen(p);
                      t = 0;
                      break;
                    case 'c':
                    case 'C':
                      if (vararg) {
                        arg = READ16(v_arg) & 0xff;
                        v_arg += 2;
                      } else {
                        arg = ARG8;
                      }
                      k++;
                      fmt[j++] = f[i];
                      fmt[j] = 0;
                      sys_sprintf(p, fmt, arg);
                      p += sys_strlen(p);
                      t = 0;
                      break;
                    case 's':
                      if (vararg) {
                        arg = READ32(v_arg);
                        v_arg += 4;
                      } else {
                        arg = ARG32;
                      }
                      k++;
                      q = (char *)(ram + arg);
                      fmt[j++] = f[i];
                      fmt[j] = 0;
                      if (arglen < 0) {
                        sys_sprintf(p, fmt, q);
                      } else {
                        sys_sprintf(p, fmt, arglen, q);
                      }
                      p += sys_strlen(p);
                      t = 0;
                      break;
                    case '*':
                      if (vararg) {
                        arglen = READ16(v_arg);
                        v_arg += 2;
                      } else {
                        arglen = ARG16;
                      }
                      k++;
                      break;
                    case '%':
                      *p++ = f[i];
                      t = 0;
                      break;
                    default:
                      fmt[j++] = f[i];
                      break;
                  }
                  break;
              }
            }
            *p = 0;
            res = sys_strlen(s);
          }
          if (vararg) {
            debug(DEBUG_TRACE, "ARM", "arm syscall StrVPrintF(\"%s\", \"%s\", 0x%08X): %d", s, f, r2, res);
          } else {
            debug(DEBUG_TRACE, "ARM", "arm syscall StrPrintF(\"%s\", \"%s\", ...): %d", s, f, res);
          }
          r0 = res;
          break;
          }
        case 0x800: {
          // Err SysCurAppDatabase(LocalID *dbIDP)
          LocalID *dbIDP = r0 ? (LocalID *)(ram + r0) : NULL;
          err = SysCurAppDatabase(NULL, dbIDP);
          debug(DEBUG_TRACE, "ARM", "arm syscall SysCurAppDatabase(0x%08X): %d", r0, err);
          r0 = err;
          }
          break;
        case 0x804: {
          // Err SysCurAppDatabaseV40(UInt16 *cardNoP, LocalID *dbIDP)
          UInt16 *cardNoP = r0 ? (UInt16 *)(ram + r0) : NULL;
          LocalID *dbIDP = r1 ? (LocalID *)(ram + r1) : NULL;
          err = SysCurAppDatabase(cardNoP, dbIDP);
          debug(DEBUG_TRACE, "ARM", "arm syscall SysCurAppDatabaseV40(0x%08X, 0x%08X): %u", r0, r1, err);
          r0 = err;
          }
          break;
        case 0x824: {
          // #define SysEventGet(eventP, timeOut) EvtGetEvent((EventType *)eventP, timeOut)
          EventType event;
          EvtGetEvent(&event, r1);
          debug(DEBUG_TRACE, "ARM", "arm syscall EvtGetEvent(0x%08X [%d], %d)", r0, event.eType, r1);
          if (r0) {
            uint8_t *p8 = (uint8_t *)(ram + r0);
            uint16_t *p16 = (uint16_t *)(ram + r0);
            uint32_t *p32 = (uint32_t *)(ram + r0);
            p32[0] = event.eType;
            p8[4]  = event.penDown;
            p16[6] = event.screenX;
            p16[7] = event.screenY;
          }
          }
          break;
        case 0x834: {
          // SysAppInfoPtr SysGetAppInfo(SysAppInfoPtr *uiAppPP, SysAppInfoPtr *actionCodeAppPP)
          // XXX uiAppPP and actionCodeAppPP ignored
          emu_state_t *state = pumpkin_get_local_storage(emu_key);
          r = state->sysAppInfoStart;
          debug(DEBUG_TRACE, "ARM", "arm syscall SysGetAppInfo(0x%08X, 0x%08X): 0x%08X", r0, r1, r);
          r0 = r;
          }
          break;
        case 0x84C: {
          // Boolean SysHandleEvent(EventPtr eventP)
          if (r0) {
            EventType event;
            MemSet(&event, sizeof(EventType), 0);
            uint32_t *p32 = (uint32_t *)(ram + r0);
            event.eType = p32[0];
            r = SysHandleEvent(&event);
            debug(DEBUG_TRACE, "ARM", "arm syscall SysHandleEvent(0x%08X [%d]): %d", r0, event.eType, r);
            r0 = r;
          }
          }
          break;
        case 0x87C: {
          // https://wiki.palmoid.com/doku.php?id=syslinkerstub
          // void SysLinkerStub(const SysModuleDescriptorType *moduleP, UInt32 clientID, void **dispatchTablePP)
          debug(DEBUG_TRACE, "ARM", "arm syscall SysLinkerStub(0x%08X, 0x%08X, 0x%08X)", r0, r1, r2);
          if (r0) {
            SysModuleDescriptorType *mod = (SysModuleDescriptorType *)(ram + r0);
            pumpkin_id2s(mod->dbType, st);
            debug(DEBUG_TRACE, "ARM", "module dbType '%s'", st);
            pumpkin_id2s(mod->dbCreator, st);
            debug(DEBUG_TRACE, "ARM", "module dbCreator '%s'", st);
            debug(DEBUG_TRACE, "ARM", "module revision %u", mod->revision);
            debug(DEBUG_TRACE, "ARM", "module entries %u", mod->entries);
            pumpkin_id2s(mod->rsrcTypeData0, st);
            debug(DEBUG_TRACE, "ARM", "module rsrcTypeData0 '%s'", st);
            pumpkin_id2s(mod->rsrcTypeCode0, st);
            debug(DEBUG_TRACE, "ARM", "module rsrcTypeCode0 '%s'", st);
            pumpkin_id2s(mod->rsrcTypeCode1, st);
            debug(DEBUG_TRACE, "ARM", "module rsrcTypeCode1 '%s'", st);
            debug(DEBUG_TRACE, "ARM", "module rsrcIDData0 %u", mod->rsrcIDData0);
            debug(DEBUG_TRACE, "ARM", "module rsrcIDCode0 %u", mod->rsrcIDCode0);
            debug(DEBUG_TRACE, "ARM", "module rsrcIDCode1 %u", mod->rsrcIDCode1);
          }
          }
          break;
        case 0x880: {
          // Err SysLoadModule(UInt32 type, UInt32 creator, UInt32 ???)
          pumpkin_id2s(r0, st);
          pumpkin_id2s(r1, st2);
          UInt16 cardNo;
          LocalID dbID;
          DmSearchStateType search;
          if ((err = DmGetNextDatabaseByTypeCreator(true, &search, r0, r1, true, &cardNo, &dbID)) == errNone) {
          } else {
            debug(DEBUG_ERROR, "ARM", "module type '%s' creator '%s' not found", st, st2);
          }
          debug(DEBUG_TRACE, "ARM", "arm syscall SysLoadModule('%s', '%s', %d): %d ", st, st2, r2, err);
          r0 = err;
          }
          break;
        case 0x8B8:
          // Int16 SysRandom(Int32 newSeed)
          r = SysRandom(r0);
          debug(DEBUG_TRACE, "ARM", "arm syscall SysRandom(%d): %d", r0, r);
          r0 = r;
          break;
        case 0x8E8:
          // Err SysTaskDelay(Int32 delay)
          // in ARM syscall, SysTaskDelay() accepts delay in milliseconds, not ticks
          r = SysTaskDelayMs(r0);
          debug(DEBUG_TRACE, "ARM", "arm syscall SysTaskDelay(%d): %d", r0, r);
          r0 = r;
          break;
        case 0x924:
          // UInt32 TimGetSeconds(void)
          r0 = TimGetSeconds();
          debug(DEBUG_TRACE, "ARM", "arm syscall TimGetSecods(): %u", r0);
          break;
        case 0x928:
          // UInt32 TimGetTicks(void)
          // in ARM syscall, TimGetTicks() returns milliseconds, not ticks
          r0 = TimGetTicksMs();
          debug(DEBUG_TRACE, "ARM", "arm syscall TimGetTicks(): %u", r0);
          break;
        case 0xAF8: {
          // WinHandle WinCreateBitmapWindow(BitmapType *bitmapP, UInt16 *error)
          BitmapType *bmp = r0 ? (BitmapType *)(ram + r0) : NULL;
          UInt16 *error = r1 ? (UInt16 *)(ram + r1) : NULL;
          WinHandle wh = WinCreateBitmapWindow(bmp, error);
          r = wh ? (uint8_t *)wh - ram : 0;
          debug(DEBUG_TRACE, "ARM", "arm syscall WinCreateBitmapWindow(0x%08X, 0x%08X): 0x%08X", r0, r1, r);
          r0 = r;
          }
          break;
        case 0xB04: {
          // void WinDeleteWindow(WinHandle winHandle, Boolean eraseIt)
          WinHandle wh = r0 ? (WinHandle)(ram + r0) : NULL;
          WinDeleteWindow(wh, r1);
          debug(DEBUG_TRACE, "ARM", "arm syscall WinDeleteWindow(0x%08X, %d)", r0, r1);
          }
          break;
        case 0xB14:
          // void WinDrawChar(WChar theChar, Coord x, Coord y)
          debug(DEBUG_TRACE, "ARM", "arm syscall WinDrawChar('%c', %d, %d)", r0 & 0xff, r1, r2);
          WinDrawChar(r0, r1, r2);
          break;
        case 0xB18: {
          // void WinDrawChars(const Char *chars, Int16 len, Coord x, Coord y)
          char *s = (char *)(ram + r0);
          debug(DEBUG_TRACE, "ARM", "arm syscall WinDrawChars(\"%.*s\", %d, %d, %d)", r1, s, r1, r2, r3);
          WinDrawChars(s, r1, r2, r3);
          }
          break;
        case 0xB58:
          // void WinEraseWindow(void)
          WinEraseWindow();
          debug(DEBUG_TRACE, "ARM", "arm syscall WinEraseWindow()");
          break;
        case 0xB68: {
          // BitmapType *WinGetBitmap(WinHandle winHandle)
          WinHandle wh = r0 ? (WinHandle)(ram + r0) : NULL;
          BitmapType *bmp = WinGetBitmap(wh);
          r = bmp ? (uint8_t *)bmp - ram : 0;
          debug(DEBUG_TRACE, "ARM", "arm syscall WinGetBitmap(0x%08X): 0x%08X", r0, r);
          r0 = r;
          }
          break;
        case 0xBF0:
          // void WinPopDrawState(void)
          WinPopDrawState();
          debug(DEBUG_TRACE, "ARM", "arm syscall WinPopDrawState()");
          break;
        case 0xBF4:
          // void WinPushDrawState(void)
          WinPushDrawState();
          debug(DEBUG_TRACE, "ARM", "arm syscall WinPushDrawState()");
          break;
        case 0xC24:
          // IndexedColorType WinSetBackColor(IndexedColorType backColor)
          r = WinSetBackColor(r0);
          debug(DEBUG_TRACE, "ARM", "arm syscall WinSetBackColor(%d): %d", r0, r);
          r0 = r;
          break;
        case 0xC38: {
          // WinHandle WinSetDrawWindow(WinHandle winHandle)
          WinHandle wh = r0 ? (WinHandle)(ram + r0) : NULL;
          wh = WinSetDrawWindow(wh);
          r = wh ? (uint8_t *)wh - ram : 0;
          debug(DEBUG_TRACE, "ARM", "arm syscall WinSetDrawWindow(0x%08X): 0x%08X", r0, r);
          r0 = r;
          }
          break;
        case 0xC3C:
          // IndexedColorType WinSetForeColor(IndexedColorType backColor)
          r = WinSetForeColor(r0);
          debug(DEBUG_TRACE, "ARM", "arm syscall WinSetForeColor(%d): %d", r0, r);
          r0 = r;
          break;
        case 0xC4C:
          // IndexedColorType WinSetTextColor(IndexedColorType backColor)
          r = WinSetTextColor(r0);
          debug(DEBUG_TRACE, "ARM", "arm syscall WinSetTextColor(%d): %d", r0, r);
          r0 = r;
          break;
        case 0xC68: {
          // BitmapTypeV3 *BmpCreateBitmapV3(const BitmapType *bitmapP, UInt16 density, const void *bitsP, const ColorTableType *colorTableP)
          BitmapType *bmp = r0 ? (BitmapType *)(ram + r0) : NULL;
          void *bitsP = r2 ? ram + r2 : NULL;
          ColorTableType *colorTableP = r3 ? (ColorTableType *)(ram + r3) : NULL;
          BitmapTypeV3 *bmp1 = BmpCreateBitmapV3(bmp, r1, bitsP, colorTableP);
          r = bmp1 ? (uint8_t *)bmp1 - ram : 0;
          debug(DEBUG_TRACE, "ARM", "arm syscall BmpCreateBitmapV3(0x%08X, %u, 0x%08X, 0x%08X): 0x%08X", r0, r1, r2, r3, r);
          r0 = r;
          }
          break;
        default:
          debug(DEBUG_ERROR, "ARM", "unmapped arm syscall 0x%04X in BOOT", function);
          break;
      }
      break;
    case 3: // UI
      debug(DEBUG_ERROR, "ARM", "unmapped arm syscall 0x%04X in UI", function);
      r0 = 0;
      break;
    default:
      debug(DEBUG_ERROR, "ARM", "invalid group %u", group);
      r0 = 0;
      break;
  }

  return r0;
}
