/*
 *  Copyright (C) 2004  The revenge Team
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

#include <stdio.h>
#include <inttypes.h>
#include "opcodes.h"


typedef struct operand_s operand_t;
struct operand_s {
	int store; /* 0 = value, 1 = reg, 2 = data_memory, 3 = stack_memory */
	int indirect; /* 0 = direct or immeadiate value. ( e.g. MOV AX,BX), 1 = indirect or from_memory value. (e.g. MOV AX,[BX]) */
	uint32_t value; // value depends on store
	int size; // number of bytes in value.
} ;

/* A single RTL instruction */
typedef struct instruction_s instruction_t;

struct instruction_s {
	int opcode;
	operand_t srcA;
	operand_t srcB;
	operand_t dstA;
} ;

typedef struct instructions_s instructions_t;
struct instructions_s {
	int bytes_used;
	uint8_t bytes[16];
	int instruction_number;
	instruction_t instruction[10];
} ;

/* Little endian */
uint32_t getbyte(uint8_t *address);

uint32_t getdword(uint8_t *address);

int prefix_0f(instructions_t *instructions, uint8_t *bytes_base);

int rmb(instructions_t *instructions, uint8_t *bytes_base, uint8_t *return_reg);

