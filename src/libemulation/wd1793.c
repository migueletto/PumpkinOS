#include "sys.h"
#include "vfs.h"
#include "pwindow.h"
#include "filter.h"
#include "computer.h"
#include "disk.h"
#include "wd1793.h"
#include "xalloc.h"
#include "debug.h"

#define NUM_DRIVES     4
#define HEADER_LENGTH  6

struct wd1793_t {
  uint8_t status;
  uint8_t track;
  uint8_t sector;
  uint8_t data;
  uint8_t drive;
  uint8_t head;
  int command_type, busy, tr00, error, intrq, drq;
  int ready, index_pulse, head_loaded, record_type, last_direction;
  int inserted[NUM_DRIVES];
  int readOnly[NUM_DRIVES];
  uint8_t dtrack[NUM_DRIVES];
  uint16_t tracks[NUM_DRIVES];
  uint16_t heads[NUM_DRIVES];
  uint16_t sectors[NUM_DRIVES];
  uint16_t sectorlen[NUM_DRIVES];
  uint8_t header[HEADER_LENGTH];
  uint8_t buf[256];
  int ibuf, nbuf;
  void (*wdDataRequest)(int r, void *cdata);
  void (*wdInterruptRequest)(int r, void *cdata);
  void *cdata;
  disk_t *d;
};

wd1793_t *wd1793_init(disk_t *d, void *cdata) {
  wd1793_t *wd;

  if ((wd = xcalloc(1, sizeof(wd1793_t))) != NULL) {
    wd->cdata = cdata;
    wd->d = d;
  }

  return wd;
}

void wd1793_set_dr(wd1793_t *wd, void (*wdDataRequest)(int r, void *cdata)) {
  wd->wdDataRequest = wdDataRequest;
}

void wd1793_set_ir(wd1793_t *wd, void (*wdInterruptRequest)(int r, void *cdata)) {
  wd->wdInterruptRequest = wdInterruptRequest;
}

int wd1793_insert(wd1793_t *wd, int drive, int skip, int tracks, int heads, int sectors, int sectorlen, int sector0, char *name) {
  int r = -1;

  if (drive >= 0 && drive < NUM_DRIVES && name) {
    if (disk_insert(wd->d, drive, skip, tracks, heads, sectors, sectorlen, sector0, name) == 0) {
      wd->tracks[drive] = tracks;
      wd->heads[drive] = heads;
      wd->sectors[drive] = sectors;
      wd->sectorlen[drive] = sectorlen;
      wd->inserted[drive] = 1;
      debug(DEBUG_INFO, "WD1793", "inserted disk in drive %d", drive);
      r = 0;
    }
  }

  return r;
}

int wd1793_close(wd1793_t *wd) {
  int r = -1;

  if (wd) {
    xfree(wd);
    r = 0;
  }

  return r;
}

void wd1793_set_drive(wd1793_t *wd, int drive) {
  if (wd->drive != drive) {
    debug(DEBUG_INFO, "WD1793", "set drive %d", drive);
    wd->drive = drive;
  }
}

void wd1793_set_head(wd1793_t *wd, int head) {
  if (wd->head != head) {
    debug(DEBUG_INFO, "WD1793", "set head %d", head);
    wd->head = head;
  }
}

static void assertIntrq(wd1793_t *wd) {
  debug(DEBUG_TRACE, "WD1793", "assert IRQ");
  if (wd->wdInterruptRequest) wd->wdInterruptRequest(1, wd->cdata);
}

static void clearIntrq(wd1793_t *wd) {
  debug(DEBUG_TRACE, "WD1793", "clear IRQ");
  if (wd->wdInterruptRequest) wd->wdInterruptRequest(0, wd->cdata);
}

static uint8_t *read_sector(wd1793_t *wd, uint8_t drive, uint8_t track, uint8_t head, uint8_t sector) {
  disk_out(wd->d, DSKDRV, drive);
  disk_out(wd->d, DSKTKH, 0);
  disk_out(wd->d, DSKTKL, track);
  disk_out(wd->d, DSKHEAD, head);
  disk_out(wd->d, DSKSECH, 0);
  disk_out(wd->d, DSKSECL, sector);

  if (disk_read(wd->d, wd->buf, wd->sectorlen[drive]) == -1) {
    return NULL;
  }

  return wd->buf;
}

/*
  motor_rate:
  0:  6 ms
  1: 12 ms
  2: 20 ms
  3: 30 ms
*/
static void wd1793_restore(wd1793_t *wd, int motor_rate, int verify_track_number, int head_load) {
  debug(DEBUG_INFO, "WD1793", "restore (MR=%d VF=%d HL=%d)", motor_rate, verify_track_number, head_load);
  wd->command_type = 1;
  wd->ready = wd->inserted[wd->drive];
  if (wd->ready) {
    wd->head_loaded = head_load ? 1 : 0;
    wd->index_pulse = 1;
    wd->track = 0;
    wd->tr00 = 1;
  }
  wd->intrq = 1;
}

static void wd1793_seek(wd1793_t *wd, int motor_rate, int verify_track_number, int head_load) {
  // the Track Register contains the track number of the current position and the Data Register contains the desired track number
  debug(DEBUG_INFO, "WD1793", "seek track %d (MR=%d VF=%d HL=%d)", wd->data, motor_rate, verify_track_number, head_load);
  wd->command_type = 1;
  wd->ready = wd->inserted[wd->drive];
  if (wd->ready) {
    wd->head_loaded = head_load ? 1 : 0;
    wd->index_pulse = 1;
    wd->track = wd->data;
    wd->tr00 = (wd->track == 0) ? 1 : 0;
  }
  wd->intrq = 1;
}

static void wd1793_step(wd1793_t *wd, int motor_rate, int verify_track_number, int head_load, int track_update, int step_mode) {
  int dir;

  dir = wd->last_direction;

  switch (step_mode) {
    case 1: dir = wd->last_direction; break;
    case 2: dir =  1; break;  // step-in
    case 3: dir = -1; break;  // step-out
  }
  debug(DEBUG_INFO, "WD1793", "step dir %d (MR=%d VF=%d HL=%d TU=%d)", dir, motor_rate, verify_track_number, head_load, track_update);

  wd->command_type = 1;
  wd->ready = wd->inserted[wd->drive];
  wd->last_direction = dir;
  wd->intrq = 1;
}

static void wd1793_read_sector(wd1793_t *wd, int side_compare_flag, int delay_15ms, int side_compare, int multiple_record) {
  debug(DEBUG_INFO, "WD1793", "read sector drive %d, track %d, head %d, sector %d (SCF=%d D15=%d SC=%d MR=%d)", wd->drive, wd->track, wd->head, wd->sector, side_compare_flag, delay_15ms, side_compare, multiple_record);
  wd->command_type = 2;
  wd->ibuf = 0;
  wd->ready = wd->inserted[wd->drive];
  if (wd->ready) {
    wd->head_loaded = 1;
    wd->record_type = 0;  // 1: Deleted Data Mark, 0: Data Mark
    if (read_sector(wd, wd->drive, wd->track, wd->head, wd->sector) != NULL) {
      wd->nbuf = wd->sectorlen[wd->drive];
      wd->error = 0;
      wd->drq = 1;
      debug(DEBUG_INFO, "WD1793", "read sector ok");
    } else {
      wd->drq = 0;
      wd->error = 1;
      debug(DEBUG_ERROR, "WD1793", "read sector error");
    }
  }

  wd->intrq = 0;
}

static void wd1793_write_sector(wd1793_t *wd, int side_compare_flag, int delay_15ms, int side_compare, int multiple_record) {
  debug(DEBUG_INFO, "WD1793", "write sector (SCF=%d D15=%d SC=%d MR=%d)", side_compare_flag, delay_15ms, side_compare, multiple_record);
  wd->command_type = 2;
  wd->ready = wd->inserted[wd->drive];
  if (wd->ready) {
    wd->head_loaded = 1;
  }
}

static void wd1793_read_address(wd1793_t *wd, int delay_15ms) {
  debug(DEBUG_ERROR, "WD1793", "read address not implemented (D15=%d)", delay_15ms);
  wd->command_type = 3;
  wd->ready = wd->inserted[wd->drive];
  wd->intrq = 1;
}

static void wd1793_force_interrupt(wd1793_t *wd, int ready_transition, int notready_transition, int index_pulse, int imm_int) {
  debug(DEBUG_INFO, "WD1793", "force interrupt (RT=%d NRT=%d IP=%d IMM=%d)", ready_transition, notready_transition, index_pulse, imm_int);
  wd->command_type = 1;
  wd->ibuf = 0;
  if (imm_int) wd->intrq = 1;
}

static void wd1793_read_track(wd1793_t *wd, int delay_15ms) {
  debug(DEBUG_ERROR, "WD1793", "read track not implemented (D15=%d)", delay_15ms);
  wd->command_type = 3;
  wd->ready = wd->inserted[wd->drive];
  wd->intrq = 1;
}

static void wd1793_write_track(wd1793_t *wd, int delay_15ms) {
  debug(DEBUG_ERROR, "WD1793", "write track not implemented (D15=%d)", delay_15ms);
  wd->command_type = 3;
  wd->ready = wd->inserted[wd->drive];
  wd->intrq = 1;
}

/*
  Type Command                 b7 b6 b5 b4 b3 b2 b1 b0
    I  Restore                  0  0  0  0  h  V r1 r0
    I  Seek                     0  0  0  1  h  V r1 r0
    I  Step                     0  0  1  T  h  V r1 r0
    I  Step-In                  0  1  0  T  h  V r1 r0
    I  Step-Out                 0  1  1  T  h  V r1 r0
   II  Read Sector              1  0  0  m  S  E  C  0
   II  Write Sector             1  0  1  m  S  E  C a0
  III  Read Address             1  1  0  0  0  E  0  0
  III  Read Track               1  1  1  0  0  E  0  0
  III  Write Track              1  1  1  1  0  E  0  0
   IV  Force Interrupt          1  1  0  1 i3 i2 i1 i0
*/

void wd1793_write(wd1793_t *wd, int cmd, uint8_t b) {
  uint8_t command;
  int motor_rate, verify_track_number, head_load, track_update, step_mode;
  int side_compare_flag, side_compare, delay_15ms, multiple_record;
  int ready_transition, notready_transition, index_pulse, imm_int;

  switch (cmd) {
    case WD1793_COMMAND:
      debug(DEBUG_TRACE, "WD1793", "command=0x%02X", b);
      command = b & 0xF0;

      if (command == 0xD0) {  // force interrupt
        if (wd->busy) {
          debug(DEBUG_TRACE, "WD1793", "current command interrupted");
          wd->index_pulse = 0;
          wd->tr00 = 0;
          wd->head_loaded = 0;
          wd->ready = 0;
          wd->error = 0;
          wd->command_type = 0;
          wd->head_loaded = 0;
          wd->record_type = 0;
        } else {
          // status for type I command
          wd->tr00 = (wd->track == 0) ? 1 : 0;
        }
        wd->busy = 0;
        ready_transition = (b & 0x01) ? 1 : 0;
        notready_transition = (b & 0x02) ? 1 : 0;
        index_pulse = (b & 0x04) ? 1 : 0;
        imm_int = (b & 0x08) ? 1 : 0;
        wd1793_force_interrupt(wd, ready_transition, notready_transition, index_pulse, imm_int);
        break;
      }

      if (wd->busy) {
        debug(DEBUG_INFO, "WD1793", "commad received while busy");
        return;
      }
      wd->busy = 1;
      wd->index_pulse = 0;
      wd->tr00 = 0;
      wd->head_loaded = 0;
      wd->ready = 0;
      wd->error = 0;
      wd->command_type = 0;
      wd->head_loaded = 0;
      wd->record_type = 0;

      switch (command) {
        case 0x00:  // restore
          motor_rate = b & 0x03;
          verify_track_number = (b & 0x04) ? 1 : 0;
          head_load = (b & 0x08) ? 1 : 0;
          wd1793_restore(wd, motor_rate, verify_track_number, head_load);
          wd->busy = 0;
          break;
        case 0x10:  // seek
          motor_rate = b & 0x03;
          verify_track_number = (b & 0x04) ? 1 : 0;
          head_load = (b & 0x08) ? 1 : 0;
          wd1793_seek(wd, motor_rate, verify_track_number, head_load);
          wd->busy = 0;
          break;
        case 0x20:  // step
        case 0x30:
        case 0x40:  // step-in
        case 0x50:
        case 0x60:  // step-out
        case 0x70:
          motor_rate = b & 0x03;
          verify_track_number = (b & 0x04) ? 1 : 0;
          head_load = (b & 0x08) ? 1 : 0;
          track_update = (b & 0x10) ? 1 : 0;
          step_mode = (b & 0x60) >> 5;
          wd1793_step(wd, motor_rate, verify_track_number, head_load, track_update, step_mode);
          wd->busy = 0;
          break;
        case 0x80:  // read sector
        case 0x90:
          side_compare_flag = (b & 0x02) ? 1 : 0;
          delay_15ms = (b & 0x04) ? 1 : 0;
          side_compare = (b & 0x08) ? 1 : 0;
          multiple_record = (b & 0x10) ? 1 : 0;
          wd1793_read_sector(wd, side_compare_flag, delay_15ms, side_compare, multiple_record);
          break;
        case 0xA0:  // write sector
        case 0xB0:
          side_compare_flag = (b & 0x02) ? 1 : 0;
          delay_15ms = (b & 0x04) ? 1 : 0;
          side_compare = (b & 0x08) ? 1 : 0;
          multiple_record = (b & 0x10) ? 1 : 0;
          wd1793_write_sector(wd, side_compare_flag, delay_15ms, side_compare, multiple_record);
          break;
        case 0xC0:  // read address
          delay_15ms = (b & 0x04) ? 1 : 0;
          wd1793_read_address(wd, delay_15ms);
          break;
        case 0xE0:  // read track
          delay_15ms = (b & 0x04) ? 1 : 0;
          wd1793_read_track(wd, delay_15ms);
          break;
        case 0xF0:  // write track
          delay_15ms = (b & 0x04) ? 1 : 0;
          wd1793_write_track(wd, delay_15ms);
          break;
      }
      break;

    case WD1793_TRACK:
      wd->track = b;
      debug(DEBUG_INFO, "WD1793", "set track=%d", wd->track);
      break;

    case WD1793_SECTOR:
      wd->sector = b;
      debug(DEBUG_INFO, "WD1793", "set sector=%d", wd->sector);
      break;

    case WD1793_DATA:
      wd->data = b;
      debug(DEBUG_TRACE, "WD1793", "set data=0x%02X", b);
      break;

    default:
      debug(DEBUG_ERROR, "WD1793", "unknown write command %d", cmd);
      break;
  }

  if (wd->intrq) {
    assertIntrq(wd);
    wd->intrq = 0;
  } else {
    clearIntrq(wd);
  }
}

/*
      TYPE I        READ        READ        READ        WRITE       WRITE
      COMMANDS      ADDRESS     SECTOR      TRACK       SECTOR      TRACK

  b7  not ready     not ready   not ready    not ready  not ready   not ready
  b6  wr. protect   0           0            0          wr. prot.   wr. prot.
  b5  head loaded   0           record type  0          wr. fault   wr. fault
  b4  seek error    RNF         RNF          0          RNF         0
  b3  CRC error     CRC error   CRC error    0          CRC error   0
  b2  track 0       lost data   lost data    lost data  lost data   lost data
  b1  index pulse   DRQ         DRQ          DRQ        DRQ         DRQ
  b0  busy          busy        busy         busy       busy        busy
*/

static uint8_t wd1793_status(wd1793_t *wd) {
  uint8_t b = 0;

  switch (wd->command_type) {
    case 1:
      if (wd->busy)        b |= 0x01;
      if (wd->index_pulse) b |= 0x02;
      if (wd->tr00)        b |= 0x04;
      if (wd->head_loaded) b |= 0x20;
      if (!wd->ready)      b |= 0x80;
      break;
    case 2:
      if (wd->busy)        b |= 0x01;
      if (wd->drq)         b |= 0x02;
      //if (wd->error)       b |= 0x10;
      if (!wd->ready)      b |= 0x80;
      break;
    case 3:
      if (wd->busy)        b |= 0x01;
      if (wd->drq)         b |= 0x02;
      if (!wd->ready)      b |= 0x80;
      break;
  }

  return b;
}

uint8_t wd1793_read(wd1793_t *wd, int cmd) {
  uint8_t b = 0xFF;

  switch (cmd) {
    case WD1793_STATUS:
      b = wd1793_status(wd);
      //clearIntrq(wd); XXX
      debug(DEBUG_TRACE, "WD1793", "get status=0x%02X", b);
      break;

    case WD1793_TRACK:
      b = wd->track;
      debug(DEBUG_INFO, "WD1793", "get track=%d", b);
      break;

    case WD1793_SECTOR:
      b = wd->sector;
      debug(DEBUG_INFO, "WD1793", "get sector=%d", b);
      break;

    case WD1793_DATA:
      if (wd->nbuf) {
        wd->data = wd->buf[wd->ibuf++];
        b = wd->data;
        debug(DEBUG_TRACE, "WD1793", "get data %d=0x%02X", wd->ibuf-1, b);

        if (wd->ibuf == wd->nbuf) {
          debug(DEBUG_TRACE, "WD1793", "get data complete");
          wd->ibuf = 0;
          wd->nbuf = 0;
          wd->busy = 0;
          wd->drq = 0;
          assertIntrq(wd);
        }
      } else {
        debug(DEBUG_TRACE, "WD1793", "get data over=0x%02X", b);
      }
      break;

    default:
      debug(DEBUG_ERROR, "WD1793", "unknown read command %d", cmd);
      break;
  }

  return b;
}
