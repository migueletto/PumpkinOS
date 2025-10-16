#ifndef EMUPALMOS_H
#define EMUPALMOS_H

#define READ_BYTE(BASE, ADDR) (BASE)[ADDR]
#define READ_WORD(BASE, ADDR) (((BASE)[ADDR]<<8) | (BASE)[(ADDR)+1])
#define READ_LONG(BASE, ADDR) (((BASE)[ADDR]<<24) | ((BASE)[(ADDR)+1]<<16) | ((BASE)[(ADDR)+2]<<8) | (BASE)[(ADDR)+3])

#define WRITE_BYTE(BASE, ADDR, VAL) (BASE)[ADDR] = (VAL)&0xff
#define WRITE_WORD(BASE, ADDR, VAL) (BASE)[ADDR] = ((VAL)>>8) & 0xff; (BASE)[(ADDR)+1] = (VAL)&0xff
#define WRITE_LONG(BASE, ADDR, VAL) (BASE)[ADDR] = ((VAL)>>24) & 0xff;    \
                  (BASE)[(ADDR)+1] = ((VAL)>>16)&0xff;  \
                  (BASE)[(ADDR)+2] = ((VAL)>>8)&0xff;   \
                  (BASE)[(ADDR)+3] = (VAL)&0xff

#define ARG8  m68k_read_memory_8 (sp + idx); idx += 2 // reads 1 byte but advances 2, because addresses must be aligned to 16 bits
#define ARG16 m68k_read_memory_16(sp + idx); idx += 2
#define ARG32 m68k_read_memory_32(sp + idx); idx += 4

#define ARG64(a,b) (((uint64_t)(a)) << 32) | (b)
#define ARG_DOUBLE(a) \
    uint32_t a ## _high = ARG32; \
    uint32_t a ## _low  = ARG32; \
    flp_double_t a; \
    a.i = ARG64(a ## _high, a ## _low)

#define RES_DOUBLE(a,p) \
    m68k_write_memory_32(p,   (uint32_t)(a.i >> 32)); \
    m68k_write_memory_32(p+4, (uint32_t)(a.i)); \
    m68k_set_reg(M68K_REG_A0, p)

#define netLibTrapOpenConfig netLibOpenConfig
#define netLibTrapConfigMakeActive netLibConfigMakeActive
#define netLibTrapConfigList netLibConfigList
#define netLibTrapConfigIndexFromName netLibConfigIndexFromName
#define netLibTrapConfigDelete netLibConfigDelete
#define netLibTrapConfigSaveAs netLibConfigSaveAs
#define netLibTrapConfigRename netLibConfigRename
#define netLibTrapConfigAliasSet netLibConfigAliasSet
#define netLibTrapConfigAliasGet netLibConfigAliasGet

#define stackSize 4096

#define sysTrapFrmGetEventHandler68K   0xA500
#define sysTrapCtlGetStyle68K          0xA501
#define sysTrapFrmGetGadgetPtr68K      0xA502
#define sysTrapSysLibNewRefNum68K      0xA503
#define sysTrapSysLibRegister68K       0xA504
#define sysTrapSysLibCancelRefNum68K   0xA505

#define sysTrapPumpkinDebug            0xA473
#define sysTrapPumpkinDebugBytes       0xA474

typedef union {
  int32_t i;
  float f;
} flp_float_t;

typedef union {
  int64_t i;
  double d;
} flp_double_t;

typedef struct {
  int finish;
  uint32_t stackStart;
  uint32_t sysAppInfoStart;
#ifdef ARMEMU
  arm_emu_t *arm;
  uint32_t *systable;
#endif
  m68k_state_t m68k_state;
  char *panic;
  uint32_t SysFormPointerArrayToStrings_addr;
  uint32_t FrmDrawForm_addr;
  uint32_t SysQSort_addr;
  uint32_t SysBinarySearch_addr;
  uint32_t SysLibLoad_addr;
  MemHandle hNative;
  uint32_t screenStart;
  uint32_t screenEnd;
  uint32_t stackp;
  uint32_t stack[256];
  uint32_t stackt[256];
  uint8_t (*read_byte)(uint32_t address);
  uint16_t (*read_word)(uint32_t address);
  uint32_t (*read_long)(uint32_t address);
  void (*write_byte)(uint32_t address, uint8_t value);
  void (*write_word)(uint32_t address, uint16_t value);
  void (*write_long)(uint32_t address, uint32_t value);
  void *extra;
  int disasm;
} emu_state_t;

emu_state_t *m68k_get_emu_state(void);

uint32_t arm_native_call(uint32_t code, uint32_t data, uint32_t userData);

uint8_t cpu_read_byte(uint32_t address);
uint16_t cpu_read_word(uint32_t address);
uint32_t cpu_read_long(uint32_t address);
void cpu_write_byte(uint32_t address, uint8_t value);
void cpu_write_word(uint32_t address, uint16_t value);
void cpu_write_long(uint32_t address, uint32_t value);
//int cpu_instr_callback(int pc);

void encode_string(uint32_t stringP, char *buf, uint32_t len);
void decode_rgb(uint32_t rgbP, RGBColorType *rgb);
void encode_rgb(uint32_t rgbP, RGBColorType *rgb);
void decode_locale(uint32_t localeP, LmLocaleType *locale);
void encode_locale(uint32_t localeP, LmLocaleType *locale);
void decode_datetime(uint32_t dateTimeP, DateTimeType *dateTime);
void encode_datetime(uint32_t dateTimeP, DateTimeType *dateTime);
void decode_event(uint32_t eventP, EventType *event);
void encode_event(uint32_t eventP, EventType *event);
void encode_notify(uint32_t notifyP, SysNotifyParamType *notify);
void decode_notify(uint32_t notifyP, SysNotifyParamType *notify);
void decode_rectangle(uint32_t rP, RectangleType *rect);
void encode_rectangle(uint32_t rP, RectangleType *rect);
void decode_point(uint32_t pP, PointType *point);
void encode_point(uint32_t pP, PointType *point);
void encode_locale(uint32_t localeP, LmLocaleType *locale);
void decode_appinfo(uint32_t appInfoP, AppInfoType *appInfo);
void encode_appinfo(uint32_t appInfoP, AppInfoType *appInfo);
void encode_gadget(uint32_t gadgetP, FormGadgetType *gadget);
void encode_deviceinfo(uint32_t deviceInfoP, DeviceInfoType *deviceInfo);
void decode_FileInfoType(uint32_t fileInfoP, FileInfoType *fileInfo);
void encode_FileInfoType(uint32_t fileInfoP, FileInfoType *fileInfo);
void encode_VolumeInfoType(uint32_t volInfoP, VolumeInfoType *volInfo);
void decode_NetConfigNameType(uint32_t nameArrayP, NetConfigNameType *nameArray);
void encode_NetConfigNameType(uint32_t nameArrayP, NetConfigNameType *nameArray);
void decode_NetSocketAddrType(uint32_t addrP, NetSocketAddrType *a);
void encode_NetSocketAddrType(uint32_t addrP, NetSocketAddrType *a);
void encode_NetHostInfoBufType(uint32_t bufP, NetHostInfoBufType *buf);
void decode_smfoptions(uint32_t selP, SndSmfOptionsType *options);

uint32_t tos_systrap(uint16_t type);
uint32_t palmos_systrap(uint16_t trap);

void palmos_serialtrap(uint32_t sp, uint16_t idx, uint32_t sel);
void palmos_highdensitytrap(uint32_t sp, uint16_t idx, uint32_t sel);
void palmos_pinstrap(uint32_t sp, uint16_t idx, uint32_t sel);
void palmos_filesystemtrap(uint32_t sp, uint16_t idx, uint32_t sel);
void palmos_intltrap(uint32_t sp, uint16_t idx, uint32_t sel);
void palmos_flptrap(uint32_t sp, uint16_t idx, uint32_t sel);
void palmos_flpemtrap(uint32_t sp, uint16_t idx, uint32_t sel);
void palmos_omtrap(uint32_t sp, uint16_t idx, uint32_t sel);
void palmos_accessortrap(uint32_t sp, uint16_t idx, uint32_t sel);
void palmos_expansiontrap(uint32_t sp, uint16_t idx, uint32_t sel);
void palmos_tsmtrap(uint32_t sp, uint16_t idx, uint32_t sel);
void palmos_lmtrap(uint32_t sp, uint16_t idx, uint32_t sel);
void palmos_netlibtrap(uint16_t trap);
void palmos_navtrap(uint32_t sp, uint16_t idx, uint32_t sel);

void *emupalmos_trap_in(uint32_t address, uint16_t trap, int arg);
void *emupalmos_trap_sel_in(uint32_t address, uint16_t trap, uint16_t sel, int arg);
uint32_t emupalmos_trap_out(void *address);

emu_state_t *emupalmos_install(void);
void emupalmos_deinstall(emu_state_t *oldState);
void emupalmos_memory_hooks(
  uint8_t (*read_byte)(uint32_t address),
  uint16_t (*read_word)(uint32_t address),
  uint32_t (*read_long)(uint32_t address),
  void (*write_byte)(uint32_t address, uint8_t value),
  void (*write_word)(uint32_t address, uint16_t value),
  void (*write_long)(uint32_t address, uint32_t value));

#endif
