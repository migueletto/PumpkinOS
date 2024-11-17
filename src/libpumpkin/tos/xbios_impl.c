#include <PalmOS.h>

#include "plibc.h"
#include "gemdos.h"
#include "xbios_proto.h"
#include "debug.h"

void Initmouse(int16_t type, MOUSE *par, void *mousevec) {
  debug(DEBUG_ERROR, "TOS", "Initmouse not implemented");
}

void *Ssbrk(int16_t count) {
  debug(DEBUG_ERROR, "TOS", "Ssbrk not implemented");
  return 0;
}

int16_t Floprd(void *buf, int32_t filler, int16_t devno, int16_t sectno, int16_t trackno, int16_t sideno, int16_t count) {
  debug(DEBUG_ERROR, "TOS", "Floprd not implemented");
  return 0;
}

int16_t Flopwr(void *buf, int32_t filler, int16_t devno, int16_t sectno, int16_t trackno, int16_t sideno, int16_t count) {
  debug(DEBUG_ERROR, "TOS", "Flopwr not implemented");
  return 0;
}

int16_t Flopfmt(void) {
  debug(DEBUG_ERROR, "TOS", "Flopfmt not implemented");
  return 0;
}

void Dbmsg(int16_t rsrvd, int16_t msg_num, int32_t msg_arg) {
  debug(DEBUG_ERROR, "TOS", "Dbmsg not implemented");
}

void Midiws(int16_t cnt, void *ptr) {
  debug(DEBUG_ERROR, "TOS", "Midiws not implemented");
}

void Mfpint(int16_t number, void *vector) {
  // initializes the multi-function chip interrupt for the connected peripheral devices
  // this permits hardware interrupts being intercepted
  // number:
  //    0  I/O port bit 0 busy parallel port
  //    1  RS-232 DCD
  //    2  RS-232 CTS
  //    3  Blitter
  //    4  Timer D, RS-232 Baud rate generator
  //    5  Timer C, 200-Hz system clock
  //    6  Keyboard and MIDI
  //    7  FDC and DMA
  //    8  Timer B horizontal blank
  //    9  RS-232 transmit error
  //   10  RS-232 transmit buffer empty
  //   11  RS-232 receive buffer full
  //   12  RS-232 buffer full
  //   13  Timer A (DMA sound)
  //   14  RS-232 RI
  //   15  Mono monitor detect / DMA sound complete
  // vector: the interrupt service routine

  debug(DEBUG_ERROR, "TOS", "Mfpint not implemented");
}

IOREC *Iorec(int16_t dev) {
  debug(DEBUG_ERROR, "TOS", "Iorec not implemented");
  return 0;
}

int32_t Rsconf(int16_t baud, int16_t ctr, int16_t ucr, int16_t rsr, int16_t tsr, int16_t scr) {
  debug(DEBUG_ERROR, "TOS", "Rsconf not implemented");
  return 0;
}

KEYTAB *Keytbl(void *unshift, void *shift, void *capslock) {
  debug(DEBUG_ERROR, "TOS", "Keytbl not implemented");
  return 0;
}

int32_t Random(void) {
  debug(DEBUG_ERROR, "TOS", "Random not implemented");
  return 0;
}

void Protobt(void *buf, int32_t serialno, int16_t disktype, int16_t execflag) {
  debug(DEBUG_ERROR, "TOS", "Protobt not implemented");
}

int16_t Flopver(void *buf, int32_t filler, int16_t devno, int16_t sectno, int16_t trackno, int16_t sideno, int16_t count) {
  debug(DEBUG_ERROR, "TOS", "Flopver not implemented");
  return 0;
}

void Scrdmp(void) {
  debug(DEBUG_ERROR, "TOS", "Scrdmp not implemented");
}

int16_t Cursconf(int16_t func, int16_t rate) {
  debug(DEBUG_ERROR, "TOS", "Cursconf not implemented");
  return 0;
}

void Settime(uint32_t time) {
  debug(DEBUG_ERROR, "TOS", "Settime not implemented");
}

uint32_t Gettime(void) {
  debug(DEBUG_ERROR, "TOS", "Gettime not implemented");
  return 0;
}

void Bioskeys(void) {
  debug(DEBUG_ERROR, "TOS", "Bioskeys not implemented");
}

void Ikbdws(int16_t count, int8_t *ptr) {
  // passes a string to the intelligent keyboard controller
  debug(DEBUG_ERROR, "TOS", "Ikbdws not implemented");
}

void Jdisint(int16_t number) {
  debug(DEBUG_ERROR, "TOS", "Jdisint not implemented");
}

void Jenabint(int16_t number) {
  debug(DEBUG_ERROR, "TOS", "Jenabint not implemented");
}

int8_t Giaccess(int16_t dat, int16_t regno) {
  debug(DEBUG_ERROR, "TOS", "Giaccess not implemented");
  return 0;
}

void Offgibit(int16_t bitno) {
  debug(DEBUG_ERROR, "TOS", "Offgibit not implemented");
}

void Ongibit(void) {
  debug(DEBUG_ERROR, "TOS", "Ongibit not implemented");
}

void Xbtimer(int16_t timer, int16_t control, int16_t dat, void *vector) {
  debug(DEBUG_ERROR, "TOS", "Xbtimer not implemented");
}

void *Dosound(int8_t *buf) {
  debug(DEBUG_ERROR, "TOS", "Dosound not implemented");
  return 0;
}

int16_t Setprt(int16_t config) {
  debug(DEBUG_ERROR, "TOS", "Setprt not implemented");
  return 0;
}

int16_t Kbrate(int16_t initial, int16_t repeat) {
  // obtains or alters the current auto-repeat rate of the keyboard
  // initial: delay time before the key begins repeating (in 20ms steps). -1 leaves the value unchanged
  // repeat: time between repeats in 20ms steps. -1 leaves the value unchanged
  debug(DEBUG_ERROR, "TOS", "Kbrate not implemented");
  return 0;
}

int16_t Prtblk(PBDEF *par) {
  debug(DEBUG_ERROR, "TOS", "Prtblk not implemented");
  return 0;
}

void Vsync(void) {
}

void Puntaes(void) {
  debug(DEBUG_ERROR, "TOS", "Puntaes not implemented");
}

int16_t Floprate(int16_t devno, int16_t newrate) {
  debug(DEBUG_ERROR, "TOS", "Floprate not implemented");
  return 0;
}

int16_t DMAread(int32_t sector, int16_t count, void *buffer, int16_t devno) {
  debug(DEBUG_ERROR, "TOS", "DMAread not implemented");
  return 0;
}

int16_t DMAwrite(int32_t sector, int16_t count, void *buffer, int16_t devno) {
  debug(DEBUG_ERROR, "TOS", "DMAwrite not implemented");
  return 0;
}

int32_t Bconmap(int16_t devno) {
  debug(DEBUG_ERROR, "TOS", "Bconmap not implemented");
  return 0;
}

int16_t NVMaccess(int16_t op, int16_t start, int16_t count, int8_t *buffer) {
  debug(DEBUG_ERROR, "TOS", "NVMaccess not implemented");
  return 0;
}

void Metainit(META_INFO_1 *buffer) {
  debug(DEBUG_ERROR, "TOS", "Metainit not implemented");
}

int32_t Metaopen(int16_t drive, META_DRVINFO *buffer) {
  debug(DEBUG_ERROR, "TOS", "Metaopen not implemented");
  return 0;
}

int32_t Metaclose(int16_t drive) {
  debug(DEBUG_ERROR, "TOS", "Metaclose not implemented");
  return 0;
}

int32_t Metaread(int16_t drive, void *buffer, int32_t blockno, int16_t count) {
  debug(DEBUG_ERROR, "TOS", "Metaread not implemented");
  return 0;
}

int32_t Metawrite(int16_t drive, void *buffer, int32_t blockno, int16_t count) {
  debug(DEBUG_ERROR, "TOS", "Metawrite not implemented");
  return 0;
}

int32_t Metaseek(int16_t drive, int32_t blockno) {
  debug(DEBUG_ERROR, "TOS", "Metaseek not implemented");
  return 0;
}

int32_t Metastatus(int16_t drive, void *buffer) {
  debug(DEBUG_ERROR, "TOS", "Metastatus not implemented");
  return 0;
}

int32_t Metaioctl(int16_t drive, int32_t magic, int16_t opcode, void *buffer) {
  debug(DEBUG_ERROR, "TOS", "Metaioctl not implemented");
  return 0;
}

int32_t Metastartaudio(int16_t drive, int16_t flag, uint8_t *bytearray) {
  debug(DEBUG_ERROR, "TOS", "Metastartaudio not implemented");
  return 0;
}

int32_t Metastopaudio(int16_t drive) {
  debug(DEBUG_ERROR, "TOS", "Metastopaudio not implemented");
  return 0;
}

int32_t Metasetsongtime(int16_t drive, int16_t repeat, int32_t starttime, int32_t endtime) {
  debug(DEBUG_ERROR, "TOS", "Metasetsongtime not implemented");
  return 0;
}

int32_t Metagettoc(int16_t drive, int16_t flag, CD_TOC_ENTRY *buffer) {
  debug(DEBUG_ERROR, "TOS", "Metagettoc not implemented");
  return 0;
}

int32_t Metadiscinfo(int16_t drive, CD_DISC_INFO *p) {
  debug(DEBUG_ERROR, "TOS", "Metadiscinfo not implemented");
  return 0;
}

int16_t Blitmode(int16_t mode) {
  debug(DEBUG_ERROR, "TOS", "Blitmode not implemented");
  return 0;
}

int16_t EsetShift(int16_t shftMode) {
  debug(DEBUG_ERROR, "TOS", "EsetShift not implemented");
  return 0;
}

int16_t EgetShift(void) {
  debug(DEBUG_ERROR, "TOS", "EgetShift not implemented");
  return 0;
}

int16_t EsetBank(int16_t bankNum) {
  debug(DEBUG_ERROR, "TOS", "EsetBank not implemented");
  return 0;
}

int16_t EsetColor(int16_t colorNum, int16_t color) {
  debug(DEBUG_ERROR, "TOS", "EsetColor not implemented");
  return 0;
}

void EsetPalette(int16_t colorNum, int16_t count, int16_t *palettePtr) {
  debug(DEBUG_ERROR, "TOS", "EsetPalette not implemented");
}

void EgetPalette(int16_t colorNum, int16_t count, int16_t *palettePtr) {
  debug(DEBUG_ERROR, "TOS", "EgetPalette not implemented");
}

int16_t EsetGray(int16_t sw) {
  debug(DEBUG_ERROR, "TOS", "EsetGray not implemented");
  return 0;
}

int16_t EsetSmear(int16_t sw) {
  debug(DEBUG_ERROR, "TOS", "EsetSmear not implemented");
  return 0;
}

int16_t VsetMode(int16_t mode) {
  debug(DEBUG_ERROR, "TOS", "VsetMode not implemented");
  return 0;
}

int16_t mon_type(void) {
  debug(DEBUG_ERROR, "TOS", "mon_type not implemented");
  return 0;
}

void VsetSync(int16_t flag) {
  debug(DEBUG_ERROR, "TOS", "VsetSync not implemented");
}

int32_t VgetSize(int16_t mode) {
  debug(DEBUG_ERROR, "TOS", "VgetSize not implemented");
  return 0;
}

void VsetRGB(int16_t index, int16_t count, int32_t *array) {
  debug(DEBUG_ERROR, "TOS", "VsetRGB not implemented");
}

void VgetRGB(int16_t index, int16_t count, int32_t *array) {
  debug(DEBUG_ERROR, "TOS", "VgetRGB not implemented");
}

int16_t ValidMode(int16_t mode) {
  debug(DEBUG_ERROR, "TOS", "ValidMode not implemented");
  return 0;
}

void Dsp_DoBlock(int8_t *data_in, int32_t size_in, int8_t *data_out, int32_t size_out) {
  debug(DEBUG_ERROR, "TOS", "Dsp_DoBlock not implemented");
}

void Dsp_BlkHandShake(int8_t *data_in, int32_t size_in, int8_t *data_out, int32_t size_out) {
  debug(DEBUG_ERROR, "TOS", "Dsp_BlkHandShake not implemented");
}

void Dsp_BlkUnpacked(int32_t *data_in, int32_t size_in, int32_t *data_out, int32_t size_out) {
  debug(DEBUG_ERROR, "TOS", "Dsp_BlkUnpacked not implemented");
}

void Dsp_InStream(int8_t *data_in, int32_t block_size, int32_t num_blocks, int32_t *blocks_done) {
  debug(DEBUG_ERROR, "TOS", "Dsp_InStream not implemented");
}

void Dsp_OutStream(int8_t *data_out, int32_t block_size, int32_t num_blocks, int32_t *blocks_done) {
  debug(DEBUG_ERROR, "TOS", "Dsp_OutStream not implemented");
}

void Dsp_IOStream(int8_t *data_in, int8_t *data_out, int32_t block_insize, int32_t block_outsize, int32_t num_blocks, int32_t *blocks_done) {
  debug(DEBUG_ERROR, "TOS", "Dsp_IOStream not implemented");
}

void Dsp_RemoveInterrupts(int16_t mask) {
  debug(DEBUG_ERROR, "TOS", "Dsp_RemoveInterrupts not implemented");
}

int16_t Dsp_GetWordSize(void) {
  debug(DEBUG_ERROR, "TOS", "Dsp_GetWordSize not implemented");
  return 0;
}

int16_t Dsp_Lock(void) {
  debug(DEBUG_ERROR, "TOS", "Dsp_Lock not implemented");
  return 0;
}

void Dsp_Unlock(void) {
  debug(DEBUG_ERROR, "TOS", "Dsp_Unlock not implemented");
}

void Dsp_Available(int32_t *xavailable, int32_t *yavailable) {
  debug(DEBUG_ERROR, "TOS", "Dsp_Available not implemented");
}

int16_t Dsp_Reserve(int32_t xreserve, int32_t yreserve) {
  debug(DEBUG_ERROR, "TOS", "Dsp_Reserve not implemented");
  return 0;
}

int16_t Dsp_LoadProg(int8_t *file, int16_t ability, int8_t *buffer) {
  debug(DEBUG_ERROR, "TOS", "Dsp_LoadProg not implemented");
  return 0;
}

void Dsp_ExecProg(int8_t *codeptr, int32_t codesize, int16_t ability) {
  debug(DEBUG_ERROR, "TOS", "Dsp_ExecProg not implemented");
}

void Dsp_ExecBoot(int8_t *codeptr, int32_t codesize, int16_t ability) {
  debug(DEBUG_ERROR, "TOS", "Dsp_ExecBoot not implemented");
}

int32_t Dsp_LodToBinary(int8_t *file, int8_t *codeptr) {
  debug(DEBUG_ERROR, "TOS", "Dsp_LodToBinary not implemented");
  return 0;
}

void Dsp_TriggerHC(int16_t vector) {
  debug(DEBUG_ERROR, "TOS", "Dsp_TriggerHC not implemented");
}

int16_t Dsp_RequestUniqueAbility(void) {
  debug(DEBUG_ERROR, "TOS", "Dsp_RequestUniqueAbility not implemented");
  return 0;
}

int16_t Dsp_GetProgAbility(void) {
  debug(DEBUG_ERROR, "TOS", "Dsp_GetProgAbility not implemented");
  return 0;
}

void Dsp_FlushSubroutines(void) {
  debug(DEBUG_ERROR, "TOS", "Dsp_FlushSubroutines not implemented");
}

int16_t Dsp_LoadSubroutine(int8_t *codeptr, int32_t codesize, int16_t ability) {
  debug(DEBUG_ERROR, "TOS", "Dsp_LoadSubroutine not implemented");
  return 0;
}

int16_t Dsp_InqSubrAbility(int16_t ability) {
  debug(DEBUG_ERROR, "TOS", "Dsp_InqSubrAbility not implemented");
  return 0;
}

int16_t Dsp_RunSubroutine(int16_t handle) {
  debug(DEBUG_ERROR, "TOS", "Dsp_RunSubroutine not implemented");
  return 0;
}

int16_t Dsp_Hf0(int16_t flag) {
  debug(DEBUG_ERROR, "TOS", "Dsp_Hf0 not implemented");
  return 0;
}

int16_t Dsp_Hf1(int16_t flag) {
  debug(DEBUG_ERROR, "TOS", "Dsp_Hf1 not implemented");
  return 0;
}

int16_t Dsp_Hf2(void) {
  debug(DEBUG_ERROR, "TOS", "Dsp_Hf2 not implemented");
  return 0;
}

int16_t Dsp_Hf3(void) {
  debug(DEBUG_ERROR, "TOS", "Dsp_Hf3 not implemented");
  return 0;
}

void Dsp_BlkWords(void *data_in, int32_t size_in, void *data_out, int32_t size_out) {
  debug(DEBUG_ERROR, "TOS", "Dsp_BlkWords not implemented");
}

void Dsp_BlkBytes(void *data_in, int32_t size_in, void *data_out, int32_t size_out) {
  debug(DEBUG_ERROR, "TOS", "Dsp_BlkBytes not implemented");
}

int8_t Dsp_HStat(void) {
  debug(DEBUG_ERROR, "TOS", "Dsp_HStat not implemented");
  return 0;
}

void Dsp_SetVectors(void *receiver, void *transmitter) {
  debug(DEBUG_ERROR, "TOS", "Dsp_SetVectors not implemented");
}

void Dsp_MultBlocks(int32_t numsend, int32_t numreceive, DSPBLOCK *sendblocks, DSPBLOCK *receiveblocks) {
  debug(DEBUG_ERROR, "TOS", "Dsp_MultBlocks not implemented");
}

int32_t locksnd(void) {
  debug(DEBUG_ERROR, "TOS", "locksnd not implemented");
  return 0;
}

int32_t unlocksnd(void) {
  debug(DEBUG_ERROR, "TOS", "unlocksnd not implemented");
  return 0;
}

int32_t soundcmd(int16_t mode, int16_t dat) {
  debug(DEBUG_ERROR, "TOS", "soundcmd not implemented");
  return 0;
}

int32_t setbuffer(int16_t reg, void *begaddr, void *endaddr) {
  debug(DEBUG_ERROR, "TOS", "setbuffer not implemented");
  return 0;
}

int32_t setmode(int16_t mode) {
  debug(DEBUG_ERROR, "TOS", "setmode not implemented");
  return 0;
}

int32_t settracks(int16_t playtracks, int16_t rectracks) {
  debug(DEBUG_ERROR, "TOS", "settracks not implemented");
  return 0;
}

int32_t setmontracks(int16_t montrack) {
  debug(DEBUG_ERROR, "TOS", "setmontracks not implemented");
  return 0;
}

int32_t setinterrupt(int16_t src_inter, int16_t cause) {
  debug(DEBUG_ERROR, "TOS", "setinterrupt not implemented");
  return 0;
}

int32_t buffoper(int16_t mode) {
  debug(DEBUG_ERROR, "TOS", "buffoper not implemented");
  return 0;
}

int32_t dsptristate(int16_t dspxmit, int16_t dsprec) {
  debug(DEBUG_ERROR, "TOS", "dsptristate not implemented");
  return 0;
}

int32_t gpio(int16_t mode, int16_t dat) {
  debug(DEBUG_ERROR, "TOS", "gpio not implemented");
  return 0;
}

int32_t devconnect(int16_t src, int16_t dst, int16_t srcclk, int16_t prescale, int16_t protocol) {
  debug(DEBUG_ERROR, "TOS", "devconnect not implemented");
  return 0;
}

int16_t sndstatus(int16_t reset) {
  debug(DEBUG_ERROR, "TOS", "sndstatus not implemented");
  return 0;
}

int32_t buffptr(int32_t *ptr) {
  debug(DEBUG_ERROR, "TOS", "buffptr not implemented");
  return 0;
}

void VsetMask(int32_t ormask, int32_t andmask, int16_t overlay) {
  debug(DEBUG_ERROR, "TOS", "VsetMask not implemented");
}

int32_t CacheCtrl(int16_t OpCode, int16_t Param) {
  debug(DEBUG_ERROR, "TOS", "CacheCtrl not implemented");
  return 0;
}

int32_t WdgCtrl(int16_t OpCode) {
  debug(DEBUG_ERROR, "TOS", "WdgCtrl not implemented");
  return 0;
}

int32_t Xbios(int16_t command, int16_t device, int32_t param) {
  debug(DEBUG_ERROR, "TOS", "Xbios not implemented");
  return 0;
}

int32_t WavePlay(int16_t flags, int32_t rate, int32_t sptr, int32_t slen) {
  debug(DEBUG_ERROR, "TOS", "WavePlay not implemented");
  return 0;
}

