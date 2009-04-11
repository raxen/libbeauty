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
#include <revenge/rev.h>
#include "opcodes.h"

typedef struct operand_s operand_t;
struct operand_s {
	/* 0 = immeadiate value. ( e.g. MOV AX,0x0),
	 * 1 = register value. (e.g. MOV AX,BX),
	 * 2 = immeadiate pointer. (if the immeadiate value is in the relocation table) 
	 */
	int store;
	/* 0 = not relocated.
	 * 1 = relocated. (if the immeadiate value is in the relocation table)
	 */
	int relocated;
	/* 0 = direct, 1 = data_memory, 2 = stack_memory, 3 = in-out port */

	/* For IF instruction, the value "indirect" contains
         * 0 = relative
         * 1 = absolute
         */
	int indirect;
	/* value depends on store */
	/* For IF srcA, this is the condition statement */
	/* For IF dstA, this is the IP memory index. */
	uint64_t index;
	/* value depends on store */
	/* For IF dstA, this is within this group's RTL index. */
	uint64_t value;
	/* number of bytes in value. */
	/* For IF dstA, this will be a 4 bytes. */
	int size;
} ;

/* A single RTL instruction */
typedef struct instruction_s instruction_t;

struct instruction_s {
	int opcode;
	/* Set to 1 if this instruction should effect flags. */
	int flags;
	operand_t srcA; /* Used */
	operand_t srcB; /* Not currently used */
	operand_t dstA; /* Used */
} ;

typedef struct instructions_s instructions_t;
struct instructions_s {
	int bytes_used;
	uint8_t bytes[16];
	int instruction_number;
	instruction_t instruction[10];
} ;

/* Little endian */
uint32_t getbyte(uint8_t *base_address, uint64_t offset);

uint32_t getdword(uint8_t *base_address, uint64_t offset);

int prefix_0f(struct rev_eng *handle, instructions_t *instructions, uint8_t *base_address, uint64_t offset);

int rmb(struct rev_eng *handle, instructions_t *instructions, uint8_t *base_address, uint64_t offset, uint8_t *return_reg);

int disassemble(struct rev_eng *handle, instructions_t *instructions, uint8_t *base_address, uint64_t offset);

