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
 *
 *
 * 11-9-2004 Initial work.
 *   Copyright (C) 2004 James Courtier-Dutton James@superbug.co.uk
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

#include <inttypes.h>
#include <stdlib.h>
#include <revenge/dis.h>
#include <revenge/bfl.h>

instructions_t instructions;
uint8_t *inst=NULL;
struct rev_eng *handle;
char *dis_flags_table[] = { " ", "f" };

int print_inst(instructions_t *instructions)
{
	instruction_t *instruction;
	int n;
	printf("instruction_number=%d\n",instructions->instruction_number);
	if (instructions->instruction_number == 0) {
		printf("Unhandled instruction. Exiting\n");
		return 0;
	}
	for (n=0;n<instructions->instruction_number;n++) {
		instruction = &instructions->instruction[n];	
		printf("Instruction %d:%s%s",
			n,
			opcode_table[instruction->opcode],
			dis_flags_table[instruction->flags]);
		if (instruction->srcA.indirect) {
			printf(" %s%s[%s0x%x],",
				size_table[instruction->srcA.size],
				indirect_table[instruction->srcA.indirect],
				store_table[instruction->srcA.store],
				instruction->srcA.value);
		} else {
			printf(" %s%s0x%x,",
			size_table[instruction->srcA.size],
			store_table[instruction->srcA.store],
			instruction->srcA.value);
		}
		if (instruction->dstA.indirect) {
			printf(" %s%s[%s0x%x]\n",
				size_table[instruction->dstA.size],
				indirect_table[instruction->dstA.indirect],
				store_table[instruction->dstA.store],
				instruction->dstA.value);
		} else {
			printf(" %s%s0x%x\n",
			size_table[instruction->dstA.size],
			store_table[instruction->dstA.store],
			instruction->dstA.value);
		}
	}
	return 1;
}

int main(int argc, char *argv[])
{
	int n=0;
	int offset=0;
	instruction_t *instruction;
	const char *file="test.obj";
	size_t inst_size=0;
	handle = bf_test_open_file(file);
	if (!handle) {
		printf("Failed to find or recognise file\n");
		return 1;
	}
	inst_size = bf_get_code_size(handle);
	inst = malloc(inst_size);	
	bf_copy_code_section(handle, inst, inst_size);
	printf("dis:Data at %p, size=%li\n",inst, inst_size);
	for(n=0;n<inst_size;n++) {
	  printf("0x%x ",inst[n]);
	}
	printf("\n");

	printf("handle=%p\n",handle);
	bf_test_close_file(handle);

	printf("sizeof inst=%d\n",inst_size);
	for (n=0;n<inst_size;n++) {
		printf(" 0x%02x",inst[n]);
	}
	printf("\n");
	instructions.bytes_used=0;
	for(offset=0;offset<inst_size;offset+=instructions.bytes_used) {
		instructions.instruction_number=0;
		instructions.bytes_used=0;
	        disassemble(&instructions, inst+offset);
		printf("bytes used=%d\n",instructions.bytes_used);
		for (n=0;n<instructions.bytes_used;n++) {
			printf(" 0x%02x",inst[n+offset]);
		}
		printf("\n");
		if (!print_inst(&instructions)) {
			return 1;
		}
	}

	printf("test1\n");
	return 0;
}

