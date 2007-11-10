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
 * 10-11-2007 Updates.
 *   Copyright (C) 2007 James Courtier-Dutton James@superbug.co.uk
 */

/* Intel ia32 instruction format: -
 Instruction-Prefixes (Up to four prefixes of 1-byte each. [optional] )
 Opcode (1-, 2-, or 3-byte opcode)
 ModR/M (1 byte [if required] )
 SIB (Scale-Index-Base:1 byte [if required] )
 Displacement (Address displacement of 1, 2, or 4 bytes or none)
 Immediate (Immediate data of 1, 2, or 4 bytes or none)

 Naming convention taked from Intel Instruction set manual,
 Appendix A. 25366713.pdf
*/

#include <inttypes.h>
#include <stdlib.h>
#include <revenge/dis.h>
#include <revenge/bfl.h>
#include <dis-asm.h>

instructions_t instructions;
uint8_t *inst;
struct rev_eng *handle;
struct disassemble_info disasm_info;
char *dis_flags_table[] = { " ", "f" };
uint64_t inst_log;	/* Pointer to the current free instruction log entry. */

/* Memory and Registers are a list of accessed stores. */
/* A record is only valid when it has been accessed. */
/* Initially the search algorithm will be slow,
 * but if the method works, fast algorithms will be used. */

typedef struct memory_s memory_t;
struct memory_s {
	uint64_t start_address; /* Start address of multibyte access. */
	uint64_t length;	/* Number of bits accessed at one time */
	uint64_t init_value_type;	/* 0 - Unknown, 1 - Known */
	uint64_t init_value;	/* Initial value when first accessed */
	/* init_value + offset_value = absolute value to be used */
	uint64_t offset_value;
	/* 0 - unsigned, 1 - signed, 2 - pointer,
	 * 3 = Instruction, 4 = Instruction pointer(EIP) */
	uint64_t value_type;
	uint32_t last_accessed_from_instruction_at_memory_location;
	uint32_t last_accessed_from_instruction_log_at_location;
	/* value_scope: 0 - Param, 1 - Local, 2 - Global */
	uint64_t value_scope;
	/* Each time a new value is assigned, this value_id increases */
	uint64_t value_id;
	/* valid: 0 - Not used yet, 1 - Used */
	uint64_t valid;
} ;

memory_t memory_ram[1000];
memory_t memory_reg[100];
memory_t memory_stack[100];

typedef struct inst_log_entry_s inst_log_entry_t;
struct inst_log_entry_s {
	instruction_t instruction;	/* The instruction */
	memory_t value1;		/* First input value */
	memory_t value2;		/* Second input value */
	memory_t value3;		/* Result */
} ;
inst_log_entry_t inst_log_entry[100];

int print_inst(instruction_t *instruction, int instruction_number)
{
	printf("Instruction %d:%s%s",
		instruction_number,
		opcode_table[instruction->opcode],
		dis_flags_table[instruction->flags]);
	if (instruction->srcA.indirect) {
		printf(" %s%s[%s0x%x],",
			size_table[instruction->srcA.size],
			indirect_table[instruction->srcA.indirect],
			store_table[instruction->srcA.store],
			instruction->srcA.index);
	} else {
		printf(" %s%s0x%x,",
		size_table[instruction->srcA.size],
		store_table[instruction->srcA.store],
		instruction->srcA.index);
	}
	if (instruction->dstA.indirect) {
		printf(" %s%s[%s0x%x]\n",
			size_table[instruction->dstA.size],
			indirect_table[instruction->dstA.indirect],
			store_table[instruction->dstA.store],
			instruction->dstA.index);
	} else {
		printf(" %s%s0x%x\n",
		size_table[instruction->dstA.size],
		store_table[instruction->dstA.store],
		instruction->dstA.index);
	}
	return 1;
}

int get_value_from_index(operand_t *operand, uint64_t *index)
{
	if (operand->indirect) {
		printf(" %s%s[%s0x%x],",
			size_table[operand->size],
			indirect_table[operand->indirect],
			store_table[operand->store],
			operand->index);
	} else {
		printf(" %s%s0x%x,",
		size_table[operand->size],
		store_table[operand->store],
		operand->index);
	}
	return 1;
}

int execute_instruction(instruction_t *instruction)
{
	printf("Execute Instruction 0x%x:%s%s",
		instruction->opcode,
		opcode_table[instruction->opcode],
		dis_flags_table[instruction->flags]);
	if (instruction->srcA.indirect) {
		printf(" %s%s[%s0x%x],",
			size_table[instruction->srcA.size],
			indirect_table[instruction->srcA.indirect],
			store_table[instruction->srcA.store],
			instruction->srcA.index);
	} else {
		printf(" %s%s0x%x,",
		size_table[instruction->srcA.size],
		store_table[instruction->srcA.store],
		instruction->srcA.index);
	}
	if (instruction->dstA.indirect) {
		printf(" %s%s[%s0x%x]\n",
			size_table[instruction->dstA.size],
			indirect_table[instruction->dstA.indirect],
			store_table[instruction->dstA.store],
			instruction->dstA.index);
	} else {
		printf(" %s%s0x%x\n",
		size_table[instruction->dstA.size],
		store_table[instruction->dstA.store],
		instruction->dstA.index);
	}
	switch (instruction->opcode) {
	case NOP:
		printf("NOP\n");
		break;
	case MOV:
		printf("MOV\n");
		break;
	case ADD:
		printf("ADD\n");
		break;
	case ADC:
		printf("ADC\n");
		break;
	case SUB:
		printf("SUB\n");
		break;
	default:
		break;
	}
	return 1;
}

int main(int argc, char *argv[])
{
	int n = 0;
	int offset = 0;
	int octets = 0;
	disassembler_ftype disassemble_fn;
	instruction_t *instruction;
	const char *file = "test.obj";
	size_t inst_size = 0;
	handle = bf_test_open_file(file);
	if (!handle) {
		printf("Failed to find or recognise file\n");
		return 1;
	}
	inst_size = bf_get_code_size(handle);
	inst = malloc(inst_size);
	bf_copy_code_section(handle, inst, inst_size);
	printf("dis:Data at %p, size=%li\n", inst, inst_size);
	for (n = 0; n < inst_size; n++) {
		printf(" 0x%02x", inst[n]);
	}
	printf("\n");

	printf("handle=%p\n", handle);
	init_disassemble_info(&disasm_info, stdout, (fprintf_ftype) fprintf);
	disasm_info.flavour = bfd_get_flavour(handle->bfd);
	disasm_info.arch = bfd_get_arch(handle->bfd);
	disasm_info.mach = bfd_get_mach(handle->bfd);
	disasm_info.disassembler_options = NULL;
	disasm_info.octets_per_byte = bfd_octets_per_byte(handle->bfd);
	disasm_info.skip_zeroes = 8;
	disasm_info.skip_zeroes_at_end = 3;
	disasm_info.disassembler_needs_relocs = 0;
	disasm_info.buffer_length = inst_size;
	disasm_info.buffer = inst;

	printf("disassemble_fn\n");
	disassemble_fn = disassembler(handle->bfd);
	printf("disassemble_fn done %p, %p\n", disassemble_fn, print_insn_i386);
	bf_test_close_file(handle);
	instructions.bytes_used = 0;
	for (offset = 0; offset < inst_size;
			offset += instructions.bytes_used) {
		instructions.instruction_number = 0;
		instructions.bytes_used = 0;
		disassemble(&instructions, inst+offset);
		printf("bytes used = %d\n", instructions.bytes_used);
		for (n = 0; n < instructions.bytes_used; n++) {
			printf(" 0x%02x", inst[n + offset]);
		}
		printf("\n");
		//printf("disassemble_fn\n");
		//disassemble_fn = disassembler (handle->bfd);
		//printf("disassemble_fn done\n");
		printf("disassemble: ");
		octets = (*disassemble_fn) (offset, &disasm_info);
		printf("  octets=%d\n", octets);

		printf("Number of RTL instructions=%d\n",
			instructions.instruction_number);
		if (instructions.instruction_number == 0) {
			printf("Unhandled instruction. Exiting\n");
			return 0;
		}
		for (n = 0; n < instructions.instruction_number; n++) {
			instruction = &instructions.instruction[n];
			if (!print_inst(instruction, n)) {
				return 1;
			}
			if (!execute_instruction(instruction)) {
				printf("execute_intruction failed\n");
				return 1;
			}
		}
	}

	printf("test1\n");
	return 0;
}

