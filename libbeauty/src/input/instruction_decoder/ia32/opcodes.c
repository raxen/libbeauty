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

#include <dis.h>
#include "internal.h"

char * opcode_table[] = {
	"NOP",
	"MOV",
	"ADD",
	"ADC",
	"SUB",
	"SBB",
	"OR ",
	"XOR",
	"AND",
	"CMP",
	"MUL",
	"JMP",
	"CALL",
	"IF ",
	"ROL",  /* ROL,ROR etc. might be reduced to simpler equivalents. */
	"ROR",
	"RCL",
	"RCR",
	"SHL",
	"SHR",
	"SAR",
	"IN ",
	"OUT",
	"RET"
};

/*
#define REG_AX 0x04
#define REG_CX 0x08
#define REG_DX 0x0c
#define REG_BX 0x10
#define REG_SP 0x14
#define REG_BP 0x18
#define REG_SI 0x1c
#define REG_DI 0x20
#define REG_IP 0x24
#define REG_TMP1 0x28
#define REG_TMP2 0x2c
*/

reg_t reg_table[] = {
    { REG_AX,4 },
    { REG_CX,4 },
    { REG_DX,4 },
    { REG_BX,4 },
    { REG_SP,4 },
    { REG_BP,4 },
    { REG_SI,4 },
    { REG_DI,4 }
};
int immed_table[] = {
	ADD,
	OR,
	ADC,
	SBB,
	rAND,
	SUB,
	XOR,
	CMP
};

int shift2_table[] = {
	ROL,
	ROR,
	RCL,
	RCR,
	SHL,
	SHR,
	NOP,
	SAR
};

char *store_table[] = { "i", "r", "m", "s" };
char *size_table[] = { "/0", "/8", "/16", "/24", "/32" };
char *indirect_table[] = { "", "m", "s", "p" };


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


