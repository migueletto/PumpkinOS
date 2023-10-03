// 8086tiny: a tiny, highly functional, highly portable PC emulator/VM
// Copyright 2013-14, Adrian Cable (adrian.cable@gmail.com) - http://www.megalith.co.uk/8086tiny
//
// Revision 1.25
//
// This work is licensed under the MIT License. See included LICENSE.TXT.

#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <time.h>

#include "sys.h"
#include "pwindow.h"
#include "filter.h"
#include "computer.h"
#include "i8086.h"
#include "i8086disasm.h"
#include "xalloc.h"
#include "debug.h"

// Emulator system constants
#define RAM_SIZE  0x10FFF0
#define REGS_BASE 0xF0000

// 16-bit register decodes
#define REG_AX 0
#define REG_CX 1
#define REG_DX 2
#define REG_BX 3
#define REG_SP 4
#define REG_BP 5
#define REG_SI 6
#define REG_DI 7

#define REG_ES 8
#define REG_CS 9
#define REG_SS 10
#define REG_DS 11

#define REG_ZERO 12
#define REG_SCRATCH 13

// 8-bit register decodes
#define REG_AL 0
#define REG_AH 1
#define REG_CL 2
#define REG_CH 3
#define REG_DL 4
#define REG_DH 5
#define REG_BL 6
#define REG_BH 7

// FLAGS register decodes
#define FLAG_CF 40
#define FLAG_PF 41
#define FLAG_AF 42
#define FLAG_ZF 43
#define FLAG_SF 44
#define FLAG_TF 45
#define FLAG_IF 46
#define FLAG_DF 47
#define FLAG_OF 48

// Lookup tables in the BIOS binary
#define TABLE_XLAT_OPCODE 8
#define TABLE_XLAT_SUBFUNCTION 9
#define TABLE_STD_FLAGS 10
#define TABLE_PARITY_FLAG 11
#define TABLE_BASE_INST_SIZE 12
#define TABLE_I_W_SIZE 13
#define TABLE_I_MOD_SIZE 14
#define TABLE_COND_JUMP_DECODE_A 15
#define TABLE_COND_JUMP_DECODE_B 16
#define TABLE_COND_JUMP_DECODE_C 17
#define TABLE_COND_JUMP_DECODE_D 18
#define TABLE_FLAGS_BITFIELDS 19

// Bitfields for TABLE_STD_FLAGS values
#define FLAGS_UPDATE_SZP 1
#define FLAGS_UPDATE_AO_ARITH 2
#define FLAGS_UPDATE_OC_LOGIC 4

// Helper macros

// Decode mod, r_m and reg fields in instruction
#define DECODE_RM_REG scratch2_uint = 4 * !i_mod, \
            op_to_addr = rm_addr = i_mod < 3 ? SEGREG(seg_override_en ? seg_override : bios_table_lookup[scratch2_uint + 3][i_rm], bios_table_lookup[scratch2_uint][i_rm], regs16[bios_table_lookup[scratch2_uint + 1][i_rm]] + bios_table_lookup[scratch2_uint + 2][i_rm] * i_data1+) : GET_REG_ADDR(i_rm), \
            op_from_addr = GET_REG_ADDR(i_reg), \
            i_d && (scratch_uint = op_from_addr, op_from_addr = rm_addr, op_to_addr = scratch_uint)

// Return memory-mapped register location (offset into mem array) for register #reg_id
#define GET_REG_ADDR(reg_id) (REGS_BASE + (i_w ? 2 * reg_id : (2 * reg_id + reg_id / 4) & 7))

// Returns number of top bit in operand (i.e. 8 for 8-bit operands, 16 for 16-bit operands)
#define TOP_BIT 8*(i_w + 1)

// Opcode execution unit helpers
#define OPCODE ;break; case
#define OPCODE_CHAIN ; case

// [I]MUL/[I]DIV/DAA/DAS/ADC/SBB helpers
#define MUL_MACRO(op_data_type,out_regs) (set_opcode(i8086, 0x10), \
                      out_regs[i_w + 1] = (op_result = CAST(op_data_type)mem[rm_addr] * (op_data_type)*out_regs) >> 16, \
                      regs16[REG_AX] = op_result, \
                      set_OF(i8086, set_CF(i8086, op_result - (op_data_type)op_result)))
#define DIV_MACRO(out_data_type,in_data_type,out_regs) (scratch_int = CAST(out_data_type)mem[rm_addr]) && !(scratch2_uint = (in_data_type)(scratch_uint = (out_regs[i_w+1] << 16) + regs16[REG_AX]) / scratch_int, scratch2_uint - (out_data_type)scratch2_uint) ? out_regs[i_w+1] = scratch_uint - scratch_int * (*out_regs = scratch2_uint) : pc_interrupt(i8086, 0)
#define DAA_DAS(op1,op2,mask,min) set_AF(i8086, (((scratch2_uint = regs8[REG_AL]) & 0x0F) > 9) || regs8[FLAG_AF]) && (op_result = regs8[REG_AL] op1 6, set_CF(i8086, regs8[FLAG_CF] || (regs8[REG_AL] op2 scratch2_uint))), \
                  set_CF(i8086, (((mask & 1 ? scratch2_uint : regs8[REG_AL]) & mask) > min) || regs8[FLAG_CF]) && (op_result = regs8[REG_AL] op1 0x60)
#define ADC_SBB_MACRO(a) OP(a##= regs8[FLAG_CF] +), \
             set_CF(i8086, (regs8[FLAG_CF] && (op_result == op_dest)) || (a op_result < a(int)op_dest)), \
             set_AF_OF_arith(i8086)

// Execute arithmetic/logic operations in emulator memory/registers
#define R_M_OP(dest,op,src) (i_w ? \
    (op_dest = CAST(uint16_t)dest, op_result = CAST(uint16_t)dest op (op_source = CAST(uint16_t)src)) : \
    (op_dest = dest,               op_result = dest               op (op_source = CAST(uint8_t )src)))

#define MEM_OP(dest,op,src) R_M_OP(mem[dest],op,mem[src])
#define OP(op) MEM_OP(op_to_addr,op,op_from_addr)

// Increment or decrement a register #reg_id (usually SI or DI), depending on direction flag and operand size (given by i_w)
#define INDEX_INC(reg_id) (regs16[reg_id] -= (2 * regs8[FLAG_DF] - 1)*(i_w + 1))

// Helpers for stack operations
#define R_M_PUSH(a) (i_w = 1, R_M_OP(mem[SEGREG(REG_SS, REG_SP, --)], =, a))
#define R_M_POP(a) (i_w = 1, regs16[REG_SP] += 2, R_M_OP(a, =, mem[SEGREG(REG_SS, REG_SP, -2+)]))

// Convert segment:offset to linear address in emulator memory space
#define SEGREG(reg_seg,reg_ofs,op) 16 * regs16[reg_seg] + (uint16_t)(op regs16[reg_ofs])

// Returns sign bit of an 8-bit or 16-bit operand
#define SIGN_OF(a) (1 & (i_w ? CAST(int16_t)a : a) >> (TOP_BIT - 1))

// Reinterpretation cast
#define CAST(a) *(a*)&

struct i8086_t {
  uint64_t inst_counter;
  uint16_t reg_ip;
  uint16_t *regs16;
  uint8_t *regs8;
  uint8_t mem[RAM_SIZE];
  uint8_t i_rm, i_w, i_d;
  uint8_t i_reg, i_mod, i_mod_size;
  uint8_t i_reg4bit;
  uint8_t raw_opcode_id, xlat_opcode_id;
  uint8_t extra;
  uint8_t rep_mode, seg_override_en, rep_override_en;
  uint8_t trap_flag, scratch_uchar;
  uint8_t io_hi_lo, spkr_en;
  uint16_t seg_override;
  uint32_t op_source, op_dest;
  uint32_t rm_addr, op_to_addr, op_from_addr;
  uint32_t i_data0, i_data1, i_data2;
  uint32_t scratch_uint, scratch2_uint, set_flags_type;
  uint32_t ticks;
  uint64_t t0;
  int int8_asap;
  int op_result, scratch_int;
  int first, iret_debug;

  void (*callback)(void *data, uint32_t count);
  void *data;
  computer_t *computer;
};

#define inst_counter i8086->inst_counter
#define reg_ip i8086->reg_ip
#define regs16 i8086->regs16
#define regs8 i8086->regs8
#define mem i8086->mem
#define i_rm i8086->i_rm
#define i_w i8086->i_w
#define i_d i8086->i_d
#define i_reg i8086->i_reg
#define i_mod i8086->i_mod
#define i_mod_size i8086->i_mod_size
#define i_reg4bit i8086->i_reg4bit
#define raw_opcode_id i8086->raw_opcode_id
#define xlat_opcode_id i8086->xlat_opcode_id
#define extra i8086->extra
#define rep_mode i8086->rep_mode
#define seg_override_en i8086->seg_override_en
#define rep_override_en i8086->rep_override_en
#define trap_flag i8086->trap_flag
#define scratch_uchar i8086->scratch_uchar
#define io_hi_lo i8086->io_hi_lo
#define spkr_en i8086->spkr_en
#define seg_override i8086->seg_override
#define op_source i8086->op_source
#define op_dest i8086->op_dest
#define rm_addr i8086->rm_addr
#define op_to_addr i8086->op_to_addr
#define op_from_addr i8086->op_from_addr
#define i_data0 i8086->i_data0
#define i_data1 i8086->i_data1
#define i_data2 i8086->i_data2
#define scratch_uint i8086->scratch_uint
#define scratch2_uint i8086->scratch2_uint
#define set_flags_type i8086->set_flags_type
#define op_result i8086->op_result
#define scratch_int i8086->scratch_int
#define GRAPHICS_Y i8086->GRAPHICS_Y

// Global variable definitions

static const uint8_t bios_table_lookup[20][256] = {
  { 3, 3, 5, 5, 6, 7, 5, 3 },
  { 6, 7, 6, 7, 12, 12, 12, 12 },
  { 1, 1, 1, 1, 1, 1, 1, 1 },
  { 11, 11, 10, 10, 11, 11, 10, 11 },
  { 3, 3, 5, 5, 6, 7, 12, 3 },
  { 6, 7, 6, 7, 12, 12, 12, 12 },
  { 0, 0, 0, 0, 0, 0, 1, 0 },
  { 11, 11, 10, 10, 11, 11, 11, 11 },
  { 9, 9, 9, 9, 7, 7, 25, 26, 9, 9, 9, 9, 7, 7, 25, 48, 9, 9, 9, 9, 7, 7, 25, 26, 9, 9, 9, 9, 7, 7, 25, 26, 9, 9, 9, 9, 7, 7, 27, 28, 9, 9, 9, 9, 7, 7, 27, 28, 9, 9, 9, 9, 7, 7, 27, 29, 9, 9, 9, 9, 7, 7, 27, 29, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 51, 54, 52, 52, 52, 52, 52, 52, 55, 55, 55, 55, 52, 52, 52, 52, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 8, 8, 8, 15, 15, 24, 24, 9, 9, 9, 9, 10, 10, 10, 10, 16, 16, 16, 16, 16, 16, 16, 16, 30, 31, 32, 53, 33, 34, 35, 36, 11, 11, 11, 11, 17, 17, 18, 18, 47, 47, 17, 17, 17, 17, 18, 18, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 12, 12, 19, 19, 37, 37, 20, 20, 49, 50, 19, 19, 38, 39, 40, 19, 12, 12, 12, 12, 41, 42, 43, 44, 53, 53, 53, 53, 53, 53, 53, 53, 13, 13, 13, 13, 21, 21, 22, 22, 14, 14, 14, 14, 21, 21, 22, 22, 53, 0, 23, 23, 53, 45, 6, 6, 46, 46, 46, 46, 46, 46, 5, 5 },
  { 0, 0, 0, 0, 0, 0, 8, 8, 1, 1, 1, 1, 1, 1, 9, 36, 2, 2, 2, 2, 2, 2, 10, 10, 3, 3, 3, 3, 3, 3, 11, 11, 4, 4, 4, 4, 4, 4, 8, 0, 5, 5, 5, 5, 5, 5, 9, 1, 6, 6, 6, 6, 6, 6, 10, 2, 7, 7, 7, 7, 7, 7, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 21, 21, 21, 21, 21, 21, 0, 0, 0, 0, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 0, 0, 0, 0, 0, 0, 0, 0, 8, 8, 8, 8, 12, 12, 12, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 16, 22, 0, 0, 0, 0, 1, 1, 0, 255, 48, 2, 0, 0, 0, 0, 255, 255, 40, 11, 3, 3, 3, 3, 3, 3, 3, 3, 43, 43, 43, 43, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 21, 0, 0, 2, 40, 21, 21, 80, 81, 92, 93, 94, 95, 0, 0 },
  { 3, 3, 3, 3, 3, 3, 0, 0, 5, 5, 5, 5, 5, 5, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 5, 5, 5, 5, 5, 5, 0, 1, 3, 3, 3, 3, 3, 3, 0, 1, 5, 5, 5, 5, 5, 5, 0, 1, 3, 3, 3, 3, 3, 3, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 5, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1 },
  { 2, 2, 2, 2, 1, 1, 1, 1, 2, 2, 2, 2, 1, 1, 1, 2, 2, 2, 2, 2, 1, 1, 1, 1, 2, 2, 2, 2, 1, 1, 1, 1, 2, 2, 2, 2, 1, 1, 1, 1, 2, 2, 2, 2, 1, 1, 1, 1, 2, 2, 2, 2, 1, 1, 1, 1, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 3, 3, 3, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 0, 0, 2, 2, 2, 2, 4, 1, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 2, 2, 1, 1, 1, 1, 1, 1, 2, 2 },
  { 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1 },
  { 48, 40, 43, 40, 44, 41, 49, 49 },
  { 49, 49, 49, 43, 49, 49, 49, 43 },
  { 49, 49, 49, 49, 49, 49, 44, 44 },
  { 49, 49, 49, 49, 49, 49, 48, 48 },
  { 0, 2, 4, 6, 7, 8, 9, 10, 11 }
};

static void write_hook(i8086_t *i8086, uint32_t addr, int w) {
  uint16_t data;

  if (addr >= 0xB8000 && addr < 0xBC000) {
    data = w ? CAST(uint16_t)mem[addr] : mem[addr];
    i8086_direct_video(i8086->computer, addr - 0xB8000, data, w);
  }
}

// Helper functions

// Set carry flag
static char set_CF(i8086_t *i8086, int new_CF) {
  return regs8[FLAG_CF] = !!new_CF;
}

// Set auxiliary flag
static char set_AF(i8086_t *i8086, int new_AF) {
  return regs8[FLAG_AF] = !!new_AF;
}

// Set overflow flag
static char set_OF(i8086_t *i8086, int new_OF) {
  return regs8[FLAG_OF] = !!new_OF;
}

// Set auxiliary and overflow flag after arithmetic operations
static char set_AF_OF_arith(i8086_t *i8086) {
  set_AF(i8086, (op_source ^= op_dest ^ op_result) & 0x10);
  if (op_result == op_dest)
    return set_OF(i8086, 0);
  else
    return set_OF(i8086, 1 & (regs8[FLAG_CF] ^ op_source >> (TOP_BIT - 1)));
}

// Assemble and return emulated CPU FLAGS register in scratch_uint
static void make_flags(i8086_t *i8086) {
  scratch_uint = 0xF002; // 8086 has reserved and unused flags set to 1
  for (int i = 9; i--;)
    scratch_uint += regs8[FLAG_CF + i] << bios_table_lookup[TABLE_FLAGS_BITFIELDS][i];
}

// Set emulated CPU FLAGS register from regs8[FLAG_xx] values
static void set_flags(i8086_t *i8086, int new_flags) {
  for (int i = 9; i--;)
    regs8[FLAG_CF + i] = !!(1 << bios_table_lookup[TABLE_FLAGS_BITFIELDS][i] & new_flags);
}

// Convert raw opcode to translated opcode index. This condenses a large number of different encodings of similar
// instructions into a much smaller number of distinct functions, which we then execute
static void set_opcode(i8086_t *i8086, uint8_t opcode) {
  xlat_opcode_id = bios_table_lookup[TABLE_XLAT_OPCODE][raw_opcode_id = opcode];
  extra = bios_table_lookup[TABLE_XLAT_SUBFUNCTION][opcode];
  i_mod_size = bios_table_lookup[TABLE_I_MOD_SIZE][opcode];
  set_flags_type = bios_table_lookup[TABLE_STD_FLAGS][opcode];
}

static int int10h(i8086_t *i8086) {
  uint16_t ax = regs16[REG_AX], bx = regs16[REG_BX], cx = regs16[REG_CX], dx = regs16[REG_DX];
  uint8_t code, color, col, row, aux;

  switch (ax >> 8) {
    case 0x00: // Set video mode
      debug(DEBUG_TRACE, "C8086", "int10h 0x%02X set video mode %d", ax >> 8, ax & 0xFF);
      return 1;
    case 0x01: // Set cursor shape
      debug(DEBUG_TRACE, "C8086", "int10h 0x%02X set cursor shape start %d end %d", ax >> 8, cx >> 8, cx & 0xFF);
      return 1;
    case 0x02: // Set cursor position
      debug(DEBUG_TRACE, "C8086", "int10h 0x%02X set cursor position page %d row %d col %d", ax >> 8, bx >> 8, dx >> 8, dx & 0xFF);
      i8086_set_cursor(i8086->computer, regs8[REG_DH], regs8[REG_DL]);
      return 1;
    case 0x03: // Get cursor position
      debug(DEBUG_TRACE, "C8086", "int10h 0x%02X get cursor position page %d", ax >> 8, bx >> 8);
      i8086_get_cursor(i8086->computer, &row, &col);
      regs16[REG_CX] = 0x0607;
      regs8[REG_DH] = row;
      regs8[REG_DL] = col;
      return 1;
    case 0x06: // Scroll up window
      aux = ax & 0xFF;
      debug(DEBUG_TRACE, "C8086", "int10h 0x%02X scroll up lines %d color 0x%02X top %d left %d bottom %d right %d", ax >> 8, aux, bx >> 8, cx >> 8, cx & 0xFF, dx >> 8, dx & 0xFF);
      if (aux == 0) {
        i8086_cls(i8086->computer);
      }
      return 1;
    case 0x07: // Scroll down window
      aux = ax & 0xFF;
      debug(DEBUG_TRACE, "C8086", "int10h 0x%02X scroll down lines %d color 0x%02X top %d left %d bottom %d right %d", ax >> 8, aux, bx >> 8, cx >> 8, cx & 0xFF, dx >> 8, dx & 0xFF);
      if (aux == 0) {
        i8086_cls(i8086->computer);
      }
      return 1;
    case 0x08: // Get character at cursor
      debug(DEBUG_TRACE, "C8086", "int10h 0x%02X get char at cursor page %d", ax >> 8, bx >> 8);
      i8086_char_cursor(i8086->computer, &code, &color);
      regs8[REG_AH] = color;
      regs8[REG_AL] = code;
      return 1;
    case 0x09: // Write char and attribute
      debug(DEBUG_TRACE, "C8086", "int10h 0x%02X write/attr page %d char %d color 0x%02X number %d", ax >> 8, bx >> 8, ax & 0xFF, bx & 0xFF, cx);
      break;
    case 0x0E: // Write character at cursor position
      code = ax & 0xFF;
      debug(DEBUG_TRACE, "C8086", "int10h 0x%02X write at cursor page %d char %3d (%c) color 0x%02X", ax >> 8, bx >> 8, code, code < 32 ? '.' : code, bx & 0xFF);
      i8086_putc(i8086->computer, code);
      return 1;
    case 0x0F: // Get video mode
      debug(DEBUG_TRACE, "C8086", "int10h 0x%02X get video mode", ax >> 8);
      regs16[REG_AX] = 0x5003;
      regs8[REG_BH] = 0;
      return 1;
    default:
      debug(DEBUG_TRACE, "C8086", "int10h other AX 0x%04X BX 0x%04X CX 0x%04X DX 0x%04X", ax, bx, cx, dx);
      return 1;
  }

  return 0;
}

static int int11h(i8086_t *i8086) {
  regs16[REG_AX] = CAST(uint16_t)mem[0x410];
  return 1;
}

static int int12h(i8086_t *i8086) {
  regs16[REG_AX] = 0x280; // 640K
  return 1;
}

static int int13h(i8086_t *i8086) {
  uint16_t ax = regs16[REG_AX], bx = regs16[REG_BX], cx = regs16[REG_CX], dx = regs16[REG_DX];
  uint16_t head, drive, cylinder, sector, heads_per_disk, sectors_per_track;
  uint32_t lba, num_sectors;
  int r;

  debug(DEBUG_TRACE, "C8086", "int13h AX 0x%04X BX 0x%04X CX 0x%04X DX 0x%04X", ax, bx, cx, dx);

  switch (ax >> 8) {
    case 0x00: // reset disk
      regs8[REG_AH] = 0;
      set_CF(i8086, 0);
      return 1;

    case 0x02: // read sector
      num_sectors = ax & 0xFF;
      drive = dx & 0xFF;
      head = dx >> 8;
      cylinder = ((cx & 0xFF00) >> 8) | ((cx & 0xC0) << 2);
      sector = (cx & 0x3F) - 1;

      heads_per_disk = 2;
      sectors_per_track = 18;
      lba = (cylinder * heads_per_disk + head) * sectors_per_track + sector;
      debug(DEBUG_TRACE, "C8086", "int13h 0x%02X read sector drive %d head %d track %d sector %d lba %d", ax >> 8, drive, head, cylinder, sector, lba);

      i8086_seek(i8086->computer, drive, lba);
      r = i8086_read(i8086->computer, drive, mem + SEGREG(REG_ES, REG_BX,), num_sectors * 512);
      regs8[REG_AH] = r ? 1 : 0;
      set_CF(i8086, r != 0);
      return 1;

    case 0x08: // read drive parameters
      drive = dx & 0xFF;
      if (drive == 0) {
        regs16[REG_ES] = 0x0000;
        regs16[REG_DI] = 0x001E << 2;
        regs8[REG_BL] = 4; // drive type
        regs8[REG_CH] = 0x4F;
        regs8[REG_CL] = 18; // sectors per track
        regs16[REG_DX] = 0x0101;
        regs8[REG_AH] = 0;
        set_CF(i8086, 0);
      } else {
        regs8[REG_AH] = 0x0F;
        set_CF(i8086, 1);
      }
      return 1;

    case 0x15:
      regs8[REG_AH] = 0;
      set_CF(i8086, 0);
      return 1;
  }

  return 0;
}

static int int14h(i8086_t *i8086) {
  set_CF(i8086, 1);
  return 1;
}

static int int15h(i8086_t *i8086) {
  set_CF(i8086, 1);
  return 1;
}

static uint8_t scan_code(uint8_t b) {
  return b;
}

static int int16h(i8086_t *i8086) {
  uint16_t ax = regs16[REG_AX];

  switch (ax >> 8) {
    case 0x00: // read key
      regs8[REG_AL] = i8086_getc(i8086->computer);
      regs8[REG_AH] = scan_code(regs8[REG_AL]);
      return 1;
    case 0x01: // query key
      regs8[REG_AL] = i8086_kbhit(i8086->computer);
      regs8[REG_AH] = scan_code(regs8[REG_AL]);
      regs8[FLAG_ZF] = regs8[REG_AL] ? 0 : 1;
      return 1;
    case 0x02: // query shift status
      regs8[REG_AL] = 0;
      return 1;
    case 0x12: // query extented shift status
      regs16[REG_AX] = 0;
      return 1;
  }

  return 0;
}

static int int17h(i8086_t *i8086) {
  set_CF(i8086, 1);
  return 1;
}

static uint8_t bcd(uint8_t n) {
  return ((n / 10) << 4) | (n % 10);
}

static int int1ah(i8086_t *i8086) {
  uint16_t ax = regs16[REG_AX];
  sys_tm_t tm;
  uint64_t t;
  uint16_t year;

  switch (ax >> 8) {
    case 0x00: // read system clock counter
      regs8[REG_AL] = 0; // XXX 1 if 24 hours passed since reset
      regs16[REG_DX] = CAST(uint16_t)mem[0x46C];
      regs16[REG_CX] = CAST(uint16_t)mem[0x46E];
      return 1;
    case 0x02: // read time
      t = sys_time();
      sys_localtime(&t, &tm);
      regs8[REG_CH] = bcd(tm.tm_hour);
      regs8[REG_CL] = bcd(tm.tm_min);
      regs8[REG_DH] = bcd(tm.tm_sec);
      regs8[REG_DL] = tm.tm_isdst;
      set_CF(i8086, 0);
      return 1;
    case 0x04: // read date
      t = sys_time();
      sys_localtime(&t, &tm);
      year = tm.tm_year + 1900;
      regs8[REG_CH] = bcd(year / 100);
      regs8[REG_CL] = bcd(year % 100);
      regs8[REG_DH] = bcd(tm.tm_mon + 1);
      regs8[REG_DL] = bcd(tm.tm_mday);
      set_CF(i8086, 0);
      return 1;
    case 0x0F:
      set_CF(i8086, 0);
      return 1;
  }

  set_CF(i8086, 1);
  return 1;
}

// Execute INT #interrupt_num on the emulated machine
static char pc_interrupt(i8086_t *i8086, uint8_t interrupt_num) {
  switch (interrupt_num) {
    case 0x10:
      if (int10h(i8086)) return 0;
      break;
    case 0x11:
      if (int11h(i8086)) return 0;
      break;
    case 0x12:
      if (int12h(i8086)) return 0;
      break;
    case 0x13:
      if (int13h(i8086)) return 0;
      break;
    case 0x14:
      if (int14h(i8086)) return 0;
      break;
    case 0x15:
      if (int15h(i8086)) return 0;
      break;
    case 0x16:
      if (int16h(i8086)) return 0;
      break;
    case 0x17:
      if (int17h(i8086)) return 0;
      break;
    case 0x1a:
      if (int1ah(i8086)) return 0;
      break;
  }

  set_opcode(i8086, 0xCD); // Decode like INT

  make_flags(i8086);
  R_M_PUSH(scratch_uint);
  R_M_PUSH(regs16[REG_CS]);
  R_M_PUSH(reg_ip);
  MEM_OP(REGS_BASE + 2 * REG_CS, =, 4 * interrupt_num + 2);
  R_M_OP(reg_ip, =, mem[4 * interrupt_num]);

  return regs8[FLAG_TF] = regs8[FLAG_IF] = 0;
}

// AAA and AAS instructions - which_operation is +1 for AAA, and -1 for AAS
static int AAA_AAS(i8086_t *i8086, char which_operation) {
  return (regs16[REG_AX] += 262 * which_operation*set_AF(i8086, set_CF(i8086, ((regs8[REG_AL] & 0x0F) > 9) || regs8[FLAG_AF])), regs8[REG_AL] &= 0x0F);
}

static void int8h(i8086_t *i8086) {
  // 40:6C  dword Daily timer counter, equal to zero at midnight; incremented by INT 8; read/set by INT 1A
  // 40:70  byte  Clock rollover flag, set when 40:6C exceeds 24hrs
  CAST(uint16_t)mem[0x46C] = i8086->ticks & 0xFFFF;
  CAST(uint16_t)mem[0x46E] = i8086->ticks >> 16;

  pc_interrupt(i8086, 0x1C);
  i8086->callback(i8086->data, 0);

  i8086->ticks++;
  if (i8086->ticks == 1570909) {
    i8086->ticks = 0;
  }
}

int i8086_loop(i8086_t *i8086) {
  uint8_t *opcode_stream;
  uint32_t counter;

  if (i8086->first) {
    i8086->t0 = sys_get_clock();

    // Set CS:IP to F000:0100
    regs16[REG_CS] = 0xF000;
    reg_ip = 0x100;

    // Trap flag off
    regs8[FLAG_TF] = 0;

    // Set DL equal to the boot device: 0 for the FD, or 0x80 for the HD
    regs8[REG_DL] = 0;

    // Set CX:AX equal to the hard disk image size, if present
    regs16[REG_CX] = 0;
    regs16[REG_AX] = 0;

    i8086->first = 0;
  }

  for (counter = 0; counter < 32768; counter++) {
    opcode_stream = mem + ((regs16[REG_CS] << 4) + reg_ip);

    // Set up variables to prepare for decoding an opcode
    set_opcode(i8086, *opcode_stream);

    // Extract i_w and i_d fields from instruction
    i_w = (i_reg4bit = raw_opcode_id & 7) & 1;
    i_d = i_reg4bit / 2 & 1;

    // Extract instruction data fields
    i_data0 = CAST(int16_t)opcode_stream[1];
    i_data1 = CAST(int16_t)opcode_stream[2];
    i_data2 = CAST(int16_t)opcode_stream[3];

    // seg_override_en and rep_override_en contain number of instructions to hold segment override and REP prefix respectively
    if (seg_override_en)
      seg_override_en--;
    if (rep_override_en)
      rep_override_en--;

    // i_mod_size > 0 indicates that opcode uses i_mod/i_rm/i_reg, so decode them
    if (i_mod_size)
    {
      i_mod = (i_data0 & 0xFF) >> 6;
      i_rm = i_data0 & 7;
      i_reg = i_data0 / 8 & 7;

      if ((!i_mod && i_rm == 6) || (i_mod == 2))
        i_data2 = CAST(int16_t)opcode_stream[4];
      else if (i_mod != 1)
        i_data2 = i_data1;
      else // If i_mod is 1, operand is (usually) 8 bits rather than 16 bits
        i_data1 = (char)i_data1;

      DECODE_RM_REG;
    }

#if 0
//if (regs16[REG_CS] != 0xF000) {
char buf[256];
i8086disasm(opcode_stream, reg_ip, buf, sizeof(buf));
debug(DEBUG_INFO, "C8086", "%04X:%04X AX=%04X BX=%04X CX=%04X DX=%04X BP=%04X DS=%04X C%dZ%d %s",
  regs16[REG_CS], reg_ip, regs16[REG_AX], regs16[REG_BX], regs16[REG_CX], regs16[REG_DX], regs16[REG_BP], regs16[REG_DS], regs8[FLAG_CF], regs8[FLAG_ZF], buf);
//}
#endif

    // Instruction execution unit
    switch (xlat_opcode_id)
    {
      OPCODE_CHAIN 0: // Conditional jump (JAE, JNAE, etc.)
        // i_w is the invert flag, e.g. i_w == 1 means JNAE, whereas i_w == 0 means JAE 
        scratch_uchar = raw_opcode_id / 2 & 7;
        reg_ip += (char)i_data0 * (i_w ^ (regs8[bios_table_lookup[TABLE_COND_JUMP_DECODE_A][scratch_uchar]] || regs8[bios_table_lookup[TABLE_COND_JUMP_DECODE_B][scratch_uchar]] || regs8[bios_table_lookup[TABLE_COND_JUMP_DECODE_C][scratch_uchar]] ^ regs8[bios_table_lookup[TABLE_COND_JUMP_DECODE_D][scratch_uchar]]))
      OPCODE 1: // MOV reg, imm
        i_w = !!(raw_opcode_id & 8);
        R_M_OP(mem[GET_REG_ADDR(i_reg4bit)], =, i_data0)
      OPCODE 3: // PUSH regs16
        R_M_PUSH(regs16[i_reg4bit])
      OPCODE 4: // POP regs16
        R_M_POP(regs16[i_reg4bit])
      OPCODE 2: // INC|DEC regs16
        i_w = 1;
        i_d = 0;
        i_reg = i_reg4bit;
        DECODE_RM_REG;
        i_reg = extra
      OPCODE_CHAIN 5: // INC|DEC|JMP|CALL|PUSH
        if (i_reg < 2) // INC|DEC
          MEM_OP(op_from_addr, += 1 - 2 * i_reg +, REGS_BASE + 2 * REG_ZERO),
          op_source = 1,
          set_AF_OF_arith(i8086),
          set_OF(i8086, op_dest + 1 - i_reg == 1 << (TOP_BIT - 1)),
          (xlat_opcode_id == 5) && (set_opcode(i8086, 0x10), 0); // Decode like ADC
        else if (i_reg != 6) // JMP|CALL
          i_reg - 3 || R_M_PUSH(regs16[REG_CS]), // CALL (far)
          i_reg & 2 && R_M_PUSH(reg_ip + 2 + i_mod*(i_mod != 3) + 2*(!i_mod && i_rm == 6)), // CALL (near or far)
          i_reg & 1 && (regs16[REG_CS] = CAST(int16_t)mem[op_from_addr + 2]), // JMP|CALL (far)
          R_M_OP(reg_ip, =, mem[op_from_addr]),
          set_opcode(i8086, 0x9A); // Decode like CALL
        else // PUSH
          R_M_PUSH(mem[rm_addr])
      OPCODE 6: // TEST r/m, imm16 / NOT|NEG|MUL|IMUL|DIV|IDIV reg
        op_to_addr = op_from_addr;

        switch (i_reg)
        {
          OPCODE_CHAIN 0: // TEST
            set_opcode(i8086, 0x20); // Decode like AND
            reg_ip += i_w + 1;
            R_M_OP(mem[op_to_addr], &, i_data2)
          OPCODE 2: // NOT
            OP(=~)
          OPCODE 3: // NEG
            OP(=-);
            op_dest = 0;
            set_opcode(i8086, 0x28); // Decode like SUB
            set_CF(i8086, op_result > op_dest)
          OPCODE 4: // MUL
            i_w ? MUL_MACRO(uint16_t, regs16) : MUL_MACRO(uint8_t, regs8)
          OPCODE 5: // IMUL
            i_w ? MUL_MACRO(int16_t, regs16) : MUL_MACRO(char, regs8)
          OPCODE 6: // DIV
            i_w ? DIV_MACRO(uint16_t, unsigned, regs16) : DIV_MACRO(uint8_t, uint16_t, regs8)
          OPCODE 7: // IDIV
            i_w ? DIV_MACRO(int16_t, int, regs16) : DIV_MACRO(char, int16_t, regs8);
        }
      OPCODE 7: // ADD|OR|ADC|SBB|AND|SUB|XOR|CMP AL/AX, immed
        rm_addr = REGS_BASE;
        i_data2 = i_data0;
        i_mod = 3;
        i_reg = extra;
        reg_ip--;
      OPCODE_CHAIN 8: // ADD|OR|ADC|SBB|AND|SUB|XOR|CMP reg, immed
        op_to_addr = rm_addr;
        regs16[REG_SCRATCH] = (i_d |= !i_w) ? (char)i_data2 : i_data2;
        op_from_addr = REGS_BASE + 2 * REG_SCRATCH;
        reg_ip += !i_d + 1;
        set_opcode(i8086, 0x08 * (extra = i_reg));
      OPCODE_CHAIN 9: // ADD|OR|ADC|SBB|AND|SUB|XOR|CMP|MOV reg, r/m
        switch (extra)
        {
          OPCODE_CHAIN 0: // ADD
            OP(+=),
            set_CF(i8086, op_result < op_dest)
          OPCODE 1: // OR
            OP(|=)
          OPCODE 2: // ADC
            ADC_SBB_MACRO(+)
          OPCODE 3: // SBB
            ADC_SBB_MACRO(-)
          OPCODE 4: // AND
            OP(&=)
          OPCODE 5: // SUB
            OP(-=),
            set_CF(i8086, op_result > op_dest)
          OPCODE 6: // XOR
            OP(^=)
          OPCODE 7: // CMP
            OP(-),
            set_CF(i8086, op_result > op_dest)
          OPCODE 8: // MOV
            OP(=);
        }
      OPCODE 10: // MOV sreg, r/m | POP r/m | LEA reg, r/m
        if (!i_w) // MOV
          i_w = 1,
          i_reg += 8,
          DECODE_RM_REG,
          OP(=);
        else if (!i_d) // LEA
          seg_override_en = 1,
          seg_override = REG_ZERO,
          DECODE_RM_REG,
          R_M_OP(mem[op_from_addr], =, rm_addr);
        else // POP
          R_M_POP(mem[rm_addr])
      OPCODE 11: // MOV AL/AX, [loc]
        i_mod = i_reg = 0;
        i_rm = 6;
        i_data1 = i_data0;
        DECODE_RM_REG;
        MEM_OP(op_from_addr, =, op_to_addr)
      OPCODE 12: // ROL|ROR|RCL|RCR|SHL|SHR|???|SAR reg/mem, 1/CL/imm (80186)
        scratch2_uint = SIGN_OF(mem[rm_addr]),
        scratch_uint = extra ? // xxx reg/mem, imm
          ++reg_ip,
          (char)i_data1
        : // xxx reg/mem, CL
          i_d
            ? 31 & regs8[REG_CL]
        : // xxx reg/mem, 1
          1;
        if (scratch_uint)
        {
          if (i_reg < 4) // Rotate operations
            scratch_uint %= i_reg / 2 + TOP_BIT,
            R_M_OP(scratch2_uint, =, mem[rm_addr]);
          if (i_reg & 1) // Rotate/shift right operations
            R_M_OP(mem[rm_addr], >>=, scratch_uint);
          else // Rotate/shift left operations
            R_M_OP(mem[rm_addr], <<=, scratch_uint);
          if (i_reg > 3) // Shift operations
            set_opcode(i8086, 0x10); // Decode like ADC
          if (i_reg > 4) // SHR or SAR
            set_CF(i8086, op_dest >> (scratch_uint - 1) & 1);
        }

        switch (i_reg)
        {
          OPCODE_CHAIN 0: // ROL
            R_M_OP(mem[rm_addr], += , scratch2_uint >> (TOP_BIT - scratch_uint));
            set_OF(i8086, SIGN_OF(op_result) ^ set_CF(i8086, op_result & 1))
          OPCODE 1: // ROR
            scratch2_uint &= (1 << scratch_uint) - 1,
            R_M_OP(mem[rm_addr], += , scratch2_uint << (TOP_BIT - scratch_uint));
            set_OF(i8086, SIGN_OF(op_result * 2) ^ set_CF(i8086, SIGN_OF(op_result)))
          OPCODE 2: // RCL
            R_M_OP(mem[rm_addr], += (regs8[FLAG_CF] << (scratch_uint - 1)) + , scratch2_uint >> (1 + TOP_BIT - scratch_uint));
            set_OF(i8086, SIGN_OF(op_result) ^ set_CF(i8086, scratch2_uint & 1 << (TOP_BIT - scratch_uint)))
          OPCODE 3: // RCR
            R_M_OP(mem[rm_addr], += (regs8[FLAG_CF] << (TOP_BIT - scratch_uint)) + , scratch2_uint << (1 + TOP_BIT - scratch_uint));
            set_CF(i8086, scratch2_uint & 1 << (scratch_uint - 1));
            set_OF(i8086, SIGN_OF(op_result) ^ SIGN_OF(op_result * 2))
          OPCODE 4: // SHL
            set_OF(i8086, SIGN_OF(op_result) ^ set_CF(i8086, SIGN_OF(op_dest << (scratch_uint - 1))))
          OPCODE 5: // SHR
            set_OF(i8086, SIGN_OF(op_dest))
          OPCODE 7: // SAR
            scratch_uint < TOP_BIT || set_CF(i8086, scratch2_uint);
            set_OF(i8086, 0);
            R_M_OP(mem[rm_addr], +=, scratch2_uint *= ~(((1 << TOP_BIT) - 1) >> scratch_uint));
        }
      OPCODE 13: // LOOPxx|JCZX
        scratch_uint = !!--regs16[REG_CX];

        switch(i_reg4bit)
        {
          OPCODE_CHAIN 0: // LOOPNZ
            scratch_uint &= !regs8[FLAG_ZF]
          OPCODE 1: // LOOPZ
            scratch_uint &= regs8[FLAG_ZF]
          OPCODE 3: // JCXXZ
            scratch_uint = !++regs16[REG_CX];
        }
        reg_ip += scratch_uint*(char)i_data0
      OPCODE 14: // JMP | CALL int16_t/near
        reg_ip += 3 - i_d;
        if (!i_w)
        {
          if (i_d) // JMP far
            reg_ip = 0,
            regs16[REG_CS] = i_data2;
          else // CALL
            R_M_PUSH(reg_ip);
        }
        reg_ip += i_d && i_w ? (char)i_data0 : i_data0
      OPCODE 15: // TEST reg, r/m
        MEM_OP(op_from_addr, &, op_to_addr)
      OPCODE 16: // XCHG AX, regs16
        i_w = 1;
        op_to_addr = REGS_BASE;
        op_from_addr = GET_REG_ADDR(i_reg4bit);
      OPCODE_CHAIN 24: // NOP|XCHG reg, r/m
        if (op_to_addr != op_from_addr)
          OP(^=),
          MEM_OP(op_from_addr, ^=, op_to_addr),
          OP(^=)
      OPCODE 17: // MOVSx (extra=0)|STOSx (extra=1)|LODSx (extra=2)
        scratch2_uint = seg_override_en ? seg_override : REG_DS;

        for (scratch_uint = rep_override_en ? regs16[REG_CX] : 1; scratch_uint; scratch_uint--)
        {
          //MEM_OP(extra < 2 ? SEGREG(REG_ES, REG_DI,) : REGS_BASE, =, extra & 1 ? REGS_BASE : SEGREG(scratch2_uint, REG_SI,));
          if (extra < 2) {
            uint32_t addr = SEGREG(REG_ES, REG_DI,);
            MEM_OP(addr, =, extra & 1 ? REGS_BASE : SEGREG(scratch2_uint, REG_SI,));
            write_hook(i8086, addr, i_w);
          } else {
            MEM_OP(REGS_BASE, =, extra & 1 ? REGS_BASE : SEGREG(scratch2_uint, REG_SI,));
          }
          extra & 1 || INDEX_INC(REG_SI);
          extra & 2 || INDEX_INC(REG_DI);
        }

        if (rep_override_en)
          regs16[REG_CX] = 0
      OPCODE 18: // CMPSx (extra=0)|SCASx (extra=1)
        scratch2_uint = seg_override_en ? seg_override : REG_DS;

        if ((scratch_uint = rep_override_en ? regs16[REG_CX] : 1))
        {
          for (; scratch_uint; rep_override_en || scratch_uint--)
          {
            MEM_OP(extra ? REGS_BASE : SEGREG(scratch2_uint, REG_SI,), -, SEGREG(REG_ES, REG_DI,)),
            extra || INDEX_INC(REG_SI),
            INDEX_INC(REG_DI), rep_override_en && !(--regs16[REG_CX] && ((!op_result) == rep_mode)) && (scratch_uint = 0);
          }

          set_flags_type = FLAGS_UPDATE_SZP | FLAGS_UPDATE_AO_ARITH; // Funge to set SZP/AO flags
          set_CF(i8086, op_result > op_dest);
        }
      OPCODE 19: // RET|RETF|IRET
        i_d = i_w;
        R_M_POP(reg_ip);
        if (extra) // IRET|RETF|RETF imm16
          R_M_POP(regs16[REG_CS]);
        if (extra & 2) { // IRET
          set_flags(i8086, R_M_POP(scratch_uint));
          if (i8086->iret_debug) {
            debug(DEBUG_INFO, "C8086", "iret AX 0x%04X BX 0x%04X CX 0x%04X DX 0x%04X ES 0x%04X DI 0x%04X CF %d ZF %d", regs16[REG_AX], regs16[REG_BX], regs16[REG_CX], regs16[REG_DX], regs16[REG_ES], regs16[REG_DI], regs8[FLAG_CF], regs8[FLAG_ZF]);
            i8086->iret_debug = 0;
          }
        } else if (!i_d) { // RET|RETF imm16
          regs16[REG_SP] += i_data0;
          if (i8086->iret_debug) {
            debug(DEBUG_INFO, "C8086", "iret AX 0x%04X BX 0x%04X CX 0x%04X DX 0x%04X ES 0x%04X DI 0x%04X CF %d ZF %d", regs16[REG_AX], regs16[REG_BX], regs16[REG_CX], regs16[REG_DX], regs16[REG_ES], regs16[REG_DI], regs8[FLAG_CF], regs8[FLAG_ZF]);
            i8086->iret_debug = 0;
          }
        }
      OPCODE 20: // MOV r/m, immed
        R_M_OP(mem[op_from_addr], =, i_data2);
        write_hook(i8086, op_from_addr, i_w);
      OPCODE 21: // IN AL/AX, DX/imm8
        scratch_uint = extra ? regs16[REG_DX] : (unsigned char)i_data0;
        if (i_w) {
          regs16[REG_AX] = i8086->computer->in(i8086->computer, scratch_uint);
        } else {
          regs8[REG_AL] = i8086->computer->in(i8086->computer, scratch_uint) & 0xFF;
        }
      OPCODE 22: // OUT DX/imm8, AL/AX
        scratch_uint = extra ? regs16[REG_DX] : (unsigned char)i_data0;
        i8086->computer->out(i8086->computer, scratch_uint, i_w ? regs16[REG_AX] : regs8[REG_AL]);
      OPCODE 23: // REPxx
        rep_override_en = 2;
        rep_mode = i_w;
        seg_override_en && seg_override_en++
      OPCODE 25: // PUSH reg
        R_M_PUSH(regs16[extra])
      OPCODE 26: // POP reg
        R_M_POP(regs16[extra])
      OPCODE 27: // xS: segment overrides
        seg_override_en = 2;
        seg_override = extra;
        rep_override_en && rep_override_en++
      OPCODE 28: // DAA/DAS
        i_w = 0;
        extra ? DAA_DAS(-=, >=, 0xFF, 0x99) : DAA_DAS(+=, <, 0xF0, 0x90) // extra = 0 for DAA, 1 for DAS
      OPCODE 29: // AAA/AAS
        op_result = AAA_AAS(i8086, extra - 1)
      OPCODE 30: // CBW
        regs8[REG_AH] = -SIGN_OF(regs8[REG_AL])
      OPCODE 31: // CWD
        regs16[REG_DX] = -SIGN_OF(regs16[REG_AX])
      OPCODE 32: // CALL FAR imm16:imm16
        R_M_PUSH(regs16[REG_CS]);
        R_M_PUSH(reg_ip + 5);
        regs16[REG_CS] = i_data2;
        reg_ip = i_data0
      OPCODE 33: // PUSHF
        make_flags(i8086);
        R_M_PUSH(scratch_uint)
      OPCODE 34: // POPF
        set_flags(i8086, R_M_POP(scratch_uint))
      OPCODE 35: // SAHF
        make_flags(i8086);
        set_flags(i8086, (scratch_uint & 0xFF00) + regs8[REG_AH])
      OPCODE 36: // LAHF
        make_flags(i8086),
        regs8[REG_AH] = scratch_uint
      OPCODE 37: // LES|LDS reg, r/m
        i_w = i_d = 1;
        DECODE_RM_REG;
        OP(=);
        MEM_OP(REGS_BASE + extra, =, rm_addr + 2)
      OPCODE 38: // INT 3
        ++reg_ip;
        pc_interrupt(i8086, 3)
      OPCODE 39: // INT imm8
        reg_ip += 2;
        pc_interrupt(i8086, i_data0)
      OPCODE 40: // INTO
        ++reg_ip;
        regs8[FLAG_OF] && pc_interrupt(i8086, 4)
      OPCODE 41: // AAM
        if (i_data0 &= 0xFF)
          regs8[REG_AH] = regs8[REG_AL] / i_data0,
          op_result = regs8[REG_AL] %= i_data0;
        else // Divide by zero
          pc_interrupt(i8086, 0)
      OPCODE 42: // AAD
        i_w = 0;
        regs16[REG_AX] = op_result = 0xFF & regs8[REG_AL] + i_data0 * regs8[REG_AH]
      OPCODE 43: // SALC
        regs8[REG_AL] = -regs8[FLAG_CF]
      OPCODE 44: // XLAT
        regs8[REG_AL] = mem[SEGREG(seg_override_en ? seg_override : REG_DS, REG_BX, regs8[REG_AL] +)]
      OPCODE 45: // CMC
        regs8[FLAG_CF] ^= 1
      OPCODE 46: // CLC|STC|CLI|STI|CLD|STD
        regs8[extra / 2] = extra & 1
      OPCODE 47: // TEST AL/AX, immed
        R_M_OP(regs8[REG_AL], &, i_data0)
      ;
      OPCODE 48: // Emulator-specific 0F xx opcodes
        switch ((char)i_data0)
        {
          OPCODE_CHAIN 1: // GET_RTC
            //time(&clock_buf);
            //ftime(&ms_clock);
            //sys_memcpy(mem + SEGREG(REG_ES, REG_BX,), sys_localtime(&clock_buf), sizeof(struct sys_tm));
            //CAST(int16_t)mem[SEGREG(REG_ES, REG_BX, 36+)] = ms_clock.millitm;
            uint32_t secs = inst_counter / 1000000;
            uint32_t msec = (inst_counter % 1000000) / 1000;
            struct tm tm;
            tm.tm_hour = 8;
            tm.tm_mday = 14;
            tm.tm_mon = 9;
            tm.tm_year = 123;
            tm.tm_wday = 0;
            tm.tm_yday = 0;
            tm.tm_min = secs / 60;
            tm.tm_sec = secs % 60;
            sys_memcpy(mem + SEGREG(REG_ES, REG_BX,), &tm, sizeof(struct tm));
            CAST(short)mem[SEGREG(REG_ES, REG_BX, 36+)] = msec;
        }
    }

    // Increment instruction pointer by computed instruction length
    reg_ip += (i_mod*(i_mod != 3) + 2*(!i_mod && i_rm == 6))*i_mod_size + bios_table_lookup[TABLE_BASE_INST_SIZE][raw_opcode_id] + bios_table_lookup[TABLE_I_W_SIZE][raw_opcode_id]*(i_w + 1);

    // If instruction needs to update SF, ZF and PF, set them as appropriate
    if (set_flags_type & FLAGS_UPDATE_SZP)
    {
      regs8[FLAG_SF] = SIGN_OF(op_result);
      regs8[FLAG_ZF] = !op_result;
      regs8[FLAG_PF] = bios_table_lookup[TABLE_PARITY_FLAG][(uint8_t)op_result];

      // If instruction is an arithmetic or logic operation, also set AF/OF/CF as appropriate.
      if (set_flags_type & FLAGS_UPDATE_AO_ARITH)
        set_AF_OF_arith(i8086);
      if (set_flags_type & FLAGS_UPDATE_OC_LOGIC)
        set_CF(i8086, 0), set_OF(i8086, 0);
    }

    // Application has set trap flag, so fire INT 1
    if (trap_flag)
      pc_interrupt(i8086, 1);

    trap_flag = regs8[FLAG_TF];

    if ((counter & 0xFF) == 0) {
      uint64_t t = sys_get_clock();
      if ((t - i8086->t0) >= 55000U) {
        i8086->int8_asap = 1;
        i8086->t0 = t;
      }
    }

    // If a timer tick is pending, interrupts are enabled, and no overrides/REP are active, then process the tick
    if (i8086->int8_asap && !seg_override_en && !rep_override_en && regs8[FLAG_IF] && !regs8[FLAG_TF]) {
      int8h(i8086);
      i8086->int8_asap = 0;
    }
  }

  return 0;
}

void i8086_reset(i8086_t *i8086, uint16_t cs, uint32_t ip) {
  regs16[REG_AX] = 0;
  regs16[REG_BX] = 0;
  regs16[REG_CX] = 0;
  regs16[REG_DX] = 0;

  regs16[REG_CS] = cs;
  reg_ip = ip;

  // Trap flag off
  regs8[FLAG_TF] = 0;
}

uint8_t *i8086_mem(i8086_t *i8086) {
  return regs8 + 0x100;
}

i8086_t *i8086_open(uint32_t period, void (*callback)(void *d, uint32_t cycles), void *data, computer_t *computer) {
  i8086_t *i8086;

  if ((i8086 = xcalloc(1, sizeof(i8086_t))) != NULL) {
    i8086->callback = callback;
    i8086->data = data;
    i8086->computer = computer;
    i8086->first = 1;

    sys_memset(mem, 0, RAM_SIZE);

    // regs16 and reg8 point to F000:0, the start of memory-mapped registers.
    regs16 = (unsigned short *)(regs8 = mem + REGS_BASE);
  }

  return i8086;
}

void i8086_stop(i8086_t *i8086) {
}

int i8086_close(i8086_t *i8086) {
  int r = -1;

  if (i8086) {
    xfree(i8086);
    r = 0;
  }

  return r;
}
