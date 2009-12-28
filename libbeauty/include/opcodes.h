/*
 *  Copyright (C) 2004-2009 The libbeauty Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
/* Intel ia32 instruction format: -
 Instruction-Prefixes (Up to four prefixes of 1-byte each. [optional] )
 Opcode (1-, 2-, or 3-byte opcode)
 ModR/M (1 byte [if required] )
 SIB (Scale-Index-Base:1 byte [if required] )
 Displacement (Address displacement of 1, 2, or 4 bytes or none)
 Immediate (Immediate data of 1, 2, or 4 bytes or none)

 Naming convention taked from Intel Instruction set manual, Appendix A. 25366713.pdf
*/

/* enums for store_table */
enum {
	STORE_DIRECT,
	STORE_REG,
};

/* enums for indirect_table */
enum {
	IND_DIRECT,
	IND_MEM,
	IND_STACK,
	IND_IO,
};

#define NOP 0x00
#define MOV 0x01
#define ADD 0x02
#define ADC 0x03
#define SUB 0x04
#define SBB 0x05
#define OR  0x06
#define XOR 0x07
#define rAND 0x08
#define NOT 0x09
#define TEST 0x0a
#define NEG 0x0b
#define CMP 0x0c
#define MUL 0x0d
#define IMUL 0x0e
#define DIV 0x0f
#define IDIV 0x10
#define JMP 0x11 /* Relative */
#define CALL 0x12 /* non-relative */ 
#define IF  0x13
#define ROL 0x14
#define ROR 0x15
#define RCL 0x16
#define RCR 0x17
#define SHL 0x18
#define SHR 0x19
#define SAR 0x1a
#define IN  0x1b
#define OUT 0x1c
#define RET 0x1d /* Special instruction for helping to print the "return local_regNNNN;" */
#define SEX 0x1e /* Signed Extention */

#define OVERFLOW 0
#define NOT_OVERFLOW 1
#define BELOW 2
#define NOT_BELOW 3
#define EQUAL 4
#define NOT_EQUAL 5
#define BELOW_EQUAL 6
#define ABOVE 7
#define rSIGNED 8
#define NO_SIGNED 9
#define PARITY 10
#define NOT_PARITY 11
#define LESS 12
#define GREATER_EQUAL 13
#define LESS_EQUAL 14
#define GREATER 15


extern char * opcode_table[];
/* FIXME: The values are currently set to 64bit, so can handle 64bit and 32bit, but not 128bit regs. */
#define REG_AX 0x08
#define REG_CX 0x10
#define REG_DX 0x18
#define REG_BX 0x20
#define REG_SP 0x28
#define REG_BP 0x30
#define REG_SI 0x38
#define REG_DI 0x40
#define REG_IP 0x48
#define REG_08 0x50
#define REG_09 0x58
#define REG_10 0x60
#define REG_11 0x68
#define REG_12 0x70
#define REG_13 0x78
#define REG_14 0x80
#define REG_15 0x88
#define REG_TMP1 0x90
#define REG_TMP2 0x98

typedef struct reg_s reg_t;

struct reg_s {
  uint32_t offset;
  int32_t size;
} ;

extern reg_t reg_table[];

extern int immed_table[];

extern int shift2_table[];
extern int grp3_table[];

extern char *store_table[];
extern char *size_table[];
extern char *indirect_table[];

/*
ia32 registers will have their own memory space in the emulator.
All little endian.



Flags= 0x0,4 (offset, length in bytes) (not sure how to handle flags yet.)
eAX = 0x04,4
eCX = 0x08,4
eDX = 0x0c,4
eBX = 0x10,4
eSP = 0x14,4
eBP = 0x18,4
eSI = 0x1c,4
eDI = 0x20,4
eIP = 0x24,4
TMP1 = 0x28,4 (Used to convert an ia32 instruction into multiple RTL instructions.)
TMP2 = 0x2c,4 (Used to convert an ia32 instruction into multiple RTL instructions.)

AX = 0x04,2
CX = 0x08,2
DX = 0x0c,2
BX = 0x10,2
SP = 0x14,2
BP = 0x18,2
SI = 0x1c,2
DI = 0x20,2

AL = 0x04,1
CL = 0x08,1
DL = 0x0c,1
BL = 0x10,1

AH = 0x05,1
CH = 0x09,1
DH = 0x0d,1
BH = 0x11,1

*/


