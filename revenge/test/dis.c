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

#include <revenge/dis.h>

instructions_t instructions;
uint8_t inst[]={0x57,
	0x56,
	0x53,
	0x83, 0xec, 0x18,
	0x8b, 0x74, 0x24, 0x28,
	0x8d, 0x44, 0x24, 0x14,
	0x8b, 0x5c, 0x24, 0x2c,
	0x8d, 0x7e, 0x48,
	0x50,
	0x57,
	0xe8, 0xfc, 0xff, 0xff, 0xff,
	0x58,
	0x5a,
	0x0f, 0xb7, 0x44, 0x24, 0x30,
	0xc1, 0xe3, 0x10,
	0x09, 0xc3,
	0x66, 0x8b, 0x56, 0x08,
	0x89, 0xd8,
	0xef, 
	0x83, 0xc2, 0x04,
	0xed,
	0xff, 0x74, 0x24, 0x14,
	0x57,
	0x89, 0xc3,
	0xe8, 0xfc, 0xff, 0xff, 0xff,
	0x83, 0xc4, 0x20,
	0x89, 0xd8,
	0x5b,
	0x5e,
	0x5f,
	0xc3
};

int main(int argc, char *argv[]) {
//8b 74 24 28
	int n=0;
	int offset=0;
	instruction_t *instruction;
	printf("sizeof inst=%d\n",sizeof(inst));
	for (n=0;n<sizeof(inst);n++) {
		printf(" 0x%02x",inst[n]);
	}
	printf("\n");
	instructions.bytes_used=0;
	for(offset=0;offset<sizeof(inst);offset+=instructions.bytes_used) {
		instructions.instruction_number=0;
		instructions.bytes_used=0;
	        disassemble(&instructions, inst+offset);
		printf("bytes used=%d\n",instructions.bytes_used);
		for (n=0;n<instructions.bytes_used;n++) {
			printf(" 0x%02x",inst[n+offset]);
		}
		printf("\n");
		printf("instruction_number=%d\n",instructions.instruction_number);
		for (n=0;n<instructions.instruction_number;n++) {
			instruction = &instructions.instruction[n];	
			printf("Instruction %d:%s",
				n,
				opcode_table[instruction->opcode]);
			if (instruction->srcA.indirect) {
				printf(" %s[%s0x%x],",
					size_table[instruction->srcA.size],
					store_table[instruction->srcA.store],
					instruction->srcA.value);
			} else {
				printf(" %s%s0x%x,",
				size_table[instruction->srcA.size],
				store_table[instruction->srcA.store],
				instruction->srcA.value);
			}
			if (instruction->dstA.indirect) {
				printf(" %s[%s0x%x]\n",
					size_table[instruction->dstA.size],
					store_table[instruction->dstA.store],
					instruction->dstA.value);
			} else {
				printf(" %s%s0x%x\n",
				size_table[instruction->dstA.size],
				store_table[instruction->dstA.store],
				instruction->dstA.value);
			}
		}
	}

	printf("test1\n");
	return 0;
}
