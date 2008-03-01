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
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <revenge/dis.h>
#include <revenge/bfl.h>
#include <revenge/exe.h>
#include <dis-asm.h>

instructions_t instructions;
uint8_t *inst;
struct rev_eng *handle;
struct disassemble_info disasm_info;
char *dis_flags_table[] = { " ", "f" };
uint64_t inst_log;	/* Pointer to the current free instruction log entry. */
char out_buf[1024];
int local_counter = 1;
void *self = NULL;

struct memory_s memory_ram[1000];
struct memory_s memory_reg[100];
struct memory_s memory_stack[100];

struct inst_log_entry_s inst_log_entry[100];

int print_inst(struct instruction_s *instruction, int instruction_number)
{
	printf("Instruction %d:%s%s",
		instruction_number,
		opcode_table[instruction->opcode],
		dis_flags_table[instruction->flags]);
	if (instruction->srcA.indirect) {
		printf(" %s[%s0x%" PRIx64 "]%s,",
			indirect_table[instruction->srcA.indirect],
			store_table[instruction->srcA.store],
			instruction->srcA.index,
			size_table[instruction->srcA.size]);
	} else {
		printf(" %s0x%" PRIx64 "%s,",
		store_table[instruction->srcA.store],
		instruction->srcA.index,
		size_table[instruction->srcA.size]);
	}
	if (instruction->dstA.indirect) {
		printf(" %s[%s0x%" PRIx64 "]%s\n",
			indirect_table[instruction->dstA.indirect],
			store_table[instruction->dstA.store],
			instruction->dstA.index,
			size_table[instruction->dstA.size]);
	} else {
		printf(" %s0x%" PRIx64 "%s\n",
		store_table[instruction->dstA.store],
		instruction->dstA.index,
		size_table[instruction->dstA.size]);
	}
	return 1;
}

int print_instructions(void)
{
	int n;
	struct instruction_s *instruction;
	struct inst_log_entry_s *inst_log1;
	for (n = 0; n < inst_log; n++) {
		inst_log1 =  &inst_log_entry[n];
		instruction =  &inst_log1->instruction;
		if (!print_inst(instruction, n))
			return 1;
		printf("init:%"PRIx64", %"PRIx64" -> %"PRIx64"\n",
			inst_log1->value1.init_value,
			inst_log1->value2.init_value,
			inst_log1->value3.init_value);
		printf("offset:%"PRIx64", %"PRIx64" -> %"PRIx64"\n",
			inst_log1->value1.offset_value,
			inst_log1->value2.offset_value,
			inst_log1->value3.offset_value);
		printf("indirect init:%"PRIx64", %"PRIx64" -> %"PRIx64"\n",
			inst_log1->value1.indirect_init_value,
			inst_log1->value2.indirect_init_value,
			inst_log1->value3.indirect_init_value);
		printf("indirect offset:%"PRIx64", %"PRIx64" -> %"PRIx64"\n",
			inst_log1->value1.indirect_offset_value,
			inst_log1->value2.indirect_offset_value,
			inst_log1->value3.indirect_offset_value);
		printf("value_type:0x%x, 0x%x -> 0x%x\n",
			inst_log1->value1.value_type,
			inst_log1->value2.value_type,
			inst_log1->value3.value_type);
		printf("value_scope:0x%x, 0x%x -> 0x%x\n",
			inst_log1->value1.value_scope,
			inst_log1->value2.value_scope,
			inst_log1->value3.value_scope);
		printf("value_id:0x%x, 0x%x -> 0x%x\n",
			inst_log1->value1.value_id,
			inst_log1->value2.value_id,
			inst_log1->value3.value_id);
	}
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

int ram_init(void)
{

}

int reg_init(void)
{
	/* esp */
	memory_reg[0].start_address = 0x14;
	/* 4 bytes */
	memory_reg[0].length = 4;
	/* 1 - Known */
	memory_reg[0].init_value_type = 1;
	/* Initial value when first accessed */
	memory_reg[0].init_value = 0x10000;
	/* No offset yet */
	memory_reg[0].offset_value = 0;
	/* 0 - unknown,
	 * 1 - unsigned,
	 * 2 - signed,
	 * 3 - pointer,
	 * 4 - Instruction,
	 * 5 - Instruction pointer(EIP),
	 * 6 - Stack pointer.
	 */
	memory_reg[0].value_type = 6;
	memory_reg[0].ref_memory = 0;
	memory_reg[0].ref_log = 0;
	/* value_scope: 0 - unknown, 1 - Param, 2 - Local, 3 - Global */
	memory_reg[0].value_scope = 0;
	/* Each time a new value is assigned, this value_id increases */
	memory_reg[0].value_id = 0;
	/* valid: 0 - Entry Not used yet, 1 - Entry Used */
	memory_reg[0].valid = 1;

	/* ebp */
	memory_reg[1].start_address = 0x18;
	/* 4 bytes */
	memory_reg[1].length = 4;
	/* 1 - Known */
	memory_reg[1].init_value_type = 1;
	/* Initial value when first accessed */
	memory_reg[1].init_value = 0x20000;
	/* No offset yet */
	memory_reg[1].offset_value = 0;
	/* 0 - unknown,
	 * 1 - unsigned,
	 * 2 - signed,
	 * 3 - pointer,
	 * 4 - Instruction,
	 * 5 - Instruction pointer(EIP),
	 * 6 - Stack pointer.
	 */
	memory_reg[1].value_type = 6;
	memory_reg[1].ref_memory = 0;
	memory_reg[1].ref_log = 0;
	/* value_scope: 0 - unknown, 1 - Param, 2 - Local, 3 - Global */
	memory_reg[1].value_scope = 0;
	/* Each time a new value is assigned, this value_id increases */
	memory_reg[1].value_id = 0;
	/* valid: 0 - entry Not used yet, 1 - entry Used */
	memory_reg[1].valid = 1;

	/* eip */
	memory_reg[2].start_address = 0x24;
	/* 4 bytes */
	memory_reg[2].length = 4;
	/* 1 - Known */
	memory_reg[2].init_value_type = 1;
	/* Initial value when first accessed */
	memory_reg[2].init_value = 40000000;
	/* No offset yet */
	memory_reg[2].offset_value = 0;
	/* 0 - unknown,
	 * 1 - unsigned,
	 * 2 - signed,
	 * 3 - pointer,
	 * 4 - Instruction,
	 * 5 - Instruction pointer(EIP),
	 * 6 - Stack pointer.
	 */
	memory_reg[2].value_type = 5;
	memory_reg[2].ref_memory = 0;
	memory_reg[2].ref_log = 0;
	/* value_scope: 0 - unknown, 1 - Param, 2 - Local, 3 - Global */
	memory_reg[2].value_scope = 3;
	/* Each time a new value is assigned, this value_id increases */
	memory_reg[2].value_id = 0;
	/* valid: 0 - entry Not used yet, 1 - entry Used */
	memory_reg[2].valid = 1;
}

int stack_init(void)
{
	int n = 0;
	/* eip on the stack */
	memory_stack[n].start_address = 0x10000;
	/* 4 bytes */
	memory_stack[n].length = 4;
	/* 1 - Known */
	memory_stack[n].init_value_type = 1;
	/* Initial value when first accessed */
	memory_stack[n].init_value = 0x0;
	/* No offset yet */
	memory_stack[n].offset_value = 0;
	/* 0 - unknown,
	 * 1 - unsigned,
	 * 2 - signed,
	 * 3 - pointer,
	 * 4 - Instruction,
	 * 5 - Instruction pointer(EIP),
	 * 6 - Stack pointer.
	 */
	memory_stack[n].value_type = 5;
	memory_stack[n].ref_memory = 0;
	memory_stack[n].ref_log = 0;
	/* value_scope: 0 - unknown, 1 - Param, 2 - Local, 3 - Global */
	memory_stack[n].value_scope = 0;
	/* Each time a new value is assigned, this value_id increases */
	memory_stack[n].value_id = 0;
	/* valid: 0 - Not used yet, 1 - Used */
	memory_stack[n].valid = 1;
	n++;

#if 0
	/* Param1 */
	memory_stack[n].start_address = 0x10004;
	/* 4 bytes */
	memory_stack[n].length = 4;
	/* 1 - Known */
	memory_stack[n].init_value_type = 1;
	/* Initial value when first accessed */
	memory_stack[n].init_value = 0x321;
	/* No offset yet */
	memory_stack[n].offset_value = 0;
	/* 0 - unknown,
	 * 1 - unsigned,
	 * 2 - signed,
	 * 3 - pointer,
	 * 4 - Instruction,
	 * 5 - Instruction pointer(EIP),
	 * 6 - Stack pointer.
	 */
	memory_stack[n].value_type = 2;
	memory_stack[n].ref_memory = 0;
	memory_stack[n].ref_log = 0;
	/* value_scope: 0 - unknown, 1 - Param, 2 - Local, 3 - Global */
	memory_stack[n].value_scope = 0;
	/* Each time a new value is assigned, this value_id increases */
	memory_stack[n].value_id = 0;
	/* valid: 0 - Not used yet, 1 - Used */
	memory_stack[n].valid = 1;
	n++;
#endif
}

int main(int argc, char *argv[])
{
	int n = 0;
	int offset = 0;
	int octets = 0;
	int result;
	char *filename;
	int fd;
	int tmp;
	disassembler_ftype disassemble_fn;
	const char *file = "test.obj";
	size_t inst_size = 0;
	int l;
	struct instruction_s *instruction;
	struct inst_log_entry_s *inst_log1;
	struct memory_s *value;
	ram_init();
	reg_init();
	stack_init();
	handle = bf_test_open_file(file);
	if (!handle) {
		printf("Failed to find or recognise file\n");
		return 1;
	}

	printf("symtab_canon2 = %ld\n", handle->symtab_sz);
	for (l = 0; l < handle->symtab_sz; l++) {
		printf("%d\n", l);
		printf("type:0x%02x\n", handle->symtab[l]->flags);
		printf("name:%s\n", handle->symtab[l]->name);
		printf("value=0x%02x\n", handle->symtab[l]->value);
	}
	printf("Setup ok\n");
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
	instructions.bytes_used = 0;
	for (offset = 0; offset < inst_size;
			offset += instructions.bytes_used) {
		instructions.instruction_number = 0;
		instructions.bytes_used = 0;
		result = disassemble(&instructions, inst+offset);
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
		if (result == 0) {
			printf("Unhandled instruction. Exiting\n");
			return 0;
		}
		if (instructions.instruction_number == 0) {
			printf("NOP instruction. Get next inst\n");
			continue;
		}
		for (n = 0; n < instructions.instruction_number; n++) {
			struct inst_log_entry_s *inst_exe;

			instruction = &instructions.instruction[n];
			if (!print_inst(instruction, n)) {
				return 1;
			}
			inst_exe = &inst_log_entry[inst_log];
			inst_log++;
			memcpy(&(inst_exe->instruction), instruction, sizeof(struct instruction_s));

			if (!execute_instruction(self, inst_exe)) {
				printf("execute_intruction failed\n");
				return 1;
			}
		}
	}
	printf("Instructions=%d\n", inst_log);
	print_instructions();
	filename = "test.c";
	fd = open(filename, O_WRONLY | O_CREAT, S_IRWXU);
	if (fd < 0) {
		printf("Failed to open file %s, error=%d\n", filename, fd);
		return 0;
	}
	printf("fd=%d\n", fd);
	printf("writing out to file\n");
	/* Output function name */
	for (l = 0; l < handle->symtab_sz; l++) {
		if (handle->symtab[l]->flags == 0x12) {
			printf("int %s()\n{\n", handle->symtab[l]->name);
			tmp = snprintf(out_buf, 1024, "int %s()\n{\n", handle->symtab[l]->name);
			write(fd, out_buf, tmp);
		}
		//printf("type:0x%02x\n", handle->symtab[l]->flags);
		//printf("name:%s\n", handle->symtab[l]->name);
		//printf("value=0x%02x\n", handle->symtab[l]->value);
	}
	for (n = 0; n <  inst_log; n++) {
		int write_offset = 0;
		inst_log1 =  &inst_log_entry[n];
		instruction =  &inst_log1->instruction;
		/* Test to see if we have an instruction to output */
		if ((inst_log1->value1.value_scope == 1) ||
			(inst_log1->value1.value_scope == 2) ||
			(inst_log1->value2.value_scope == 1) ||
			(inst_log1->value2.value_scope == 2) ||
			(inst_log1->value3.value_scope == 1) ||
			(inst_log1->value3.value_scope == 2)) {
			if (!print_inst(instruction, n))
				return 1;
			switch (instruction->opcode) {
			case MOV:
				printf("\t");
				tmp = snprintf(out_buf + write_offset, 1024 - write_offset, "\t");
				write_offset += tmp;
				if ((inst_log1->value3.value_scope) == 2) {
					printf("local%04u = ", (inst_log1->value3.value_id));
					tmp = snprintf(out_buf + write_offset, 1024 - write_offset, "local%04u = ",
						inst_log1->value3.value_id);
					write_offset += tmp;
				} else {
					printf("param%04u = ", (inst_log1->value3.indirect_offset_value));
					tmp = snprintf(out_buf + write_offset, 1024 - write_offset, "param%04u = ",
						inst_log1->value3.indirect_offset_value);
					write_offset += tmp;
				}
				if ((inst_log1->value1.value_scope) == 2) {
					printf("local%04u;\n", (inst_log1->value1.value_id));
					tmp = snprintf(out_buf + write_offset, 1024 - write_offset, "local%04u;\n",
						inst_log1->value1.value_id);
					write_offset += tmp;
				} else {
					printf("param%04u;\n", (inst_log1->value1.indirect_offset_value));
					tmp = snprintf(out_buf + write_offset, 1024 - write_offset, "param%04u;\n",
						inst_log1->value1.indirect_offset_value);
					write_offset += tmp;
					printf("write_offset=%d\n", write_offset);
				}
				break;
			case ADD:
				printf("\t");
				tmp = snprintf(out_buf + write_offset, 1024 - write_offset, "\t");
				write_offset += tmp;
				if ((inst_log1->value3.value_scope) == 2) {
					printf("local%04u += ", (inst_log1->value3.value_id));
					tmp = snprintf(out_buf + write_offset, 1024 - write_offset, "local%04u += ",
						inst_log1->value3.value_id);
					write_offset += tmp;
				} else {
					printf("param%04u += ", (inst_log1->value3.indirect_offset_value));
					tmp = snprintf(out_buf + write_offset, 1024 - write_offset, "param%04u += ",
						inst_log1->value3.indirect_offset_value);
					write_offset += tmp;
				}
				printf("\nstore=%d\n", instruction->srcA.store);
				switch (instruction->srcA.store) {
				case STORE_IMMED:
					printf("%x;\n", instruction->srcA.index);
					tmp = snprintf(out_buf + write_offset, 1024 - write_offset, "0x%x;\n",
						instruction->srcA.index);
					write_offset += tmp;
					break;
				case STORE_REG:
					if ((inst_log1->value1.value_scope) == 2) {
						printf("local%04u;\n", (inst_log1->value1.value_id));
						tmp = snprintf(out_buf + write_offset, 1024 - write_offset, "local%04u;\n",
							inst_log1->value1.value_id);
						write_offset += tmp;
					} else {
						printf("param%04u;\n", (inst_log1->value1.indirect_offset_value));
						tmp = snprintf(out_buf + write_offset, 1024 - write_offset, "param%04u;\n",
							inst_log1->value1.indirect_offset_value);
						write_offset += tmp;
						printf("write_offset=%d\n", write_offset);
					}
					break;
				case STORE_MEM:
				case STORE_STACK:
				default:
					printf("Unhandled store1\n");
					break;
				}
				break;
			case SUB:
				printf("\t");
				tmp = snprintf(out_buf + write_offset, 1024 - write_offset, "\t");
				write_offset += tmp;
				if ((inst_log1->value3.value_scope) == 2) {
					printf("local%04u -= ", (inst_log1->value3.value_id));
					tmp = snprintf(out_buf + write_offset, 1024 - write_offset, "local%04u -= ",
						inst_log1->value3.value_id);
					write_offset += tmp;
				} else {
					printf("param%04u -= ", (inst_log1->value3.indirect_offset_value));
					tmp = snprintf(out_buf + write_offset, 1024 - write_offset, "param%04u -= ",
						inst_log1->value3.indirect_offset_value);
					write_offset += tmp;
				}
				printf("\nstore=%d\n", instruction->srcA.store);
				switch (instruction->srcA.store) {
				case STORE_IMMED:
					printf("%x;\n", instruction->srcA.index);
					tmp = snprintf(out_buf + write_offset, 1024 - write_offset, "0x%x;\n",
						instruction->srcA.index);
					write_offset += tmp;
					break;
				case STORE_REG:
					if ((inst_log1->value1.value_scope) == 2) {
						printf("local%04u;\n", (inst_log1->value1.value_id));
						tmp = snprintf(out_buf + write_offset, 1024 - write_offset, "local%04u;\n",
							inst_log1->value1.value_id);
						write_offset += tmp;
					} else {
						printf("param%04u;\n", (inst_log1->value1.indirect_offset_value));
						tmp = snprintf(out_buf + write_offset, 1024 - write_offset, "param%04u;\n",
							inst_log1->value1.indirect_offset_value);
						write_offset += tmp;
						printf("write_offset=%d\n", write_offset);
					}
					break;
				case STORE_MEM:
				case STORE_STACK:
				default:
					printf("Unhandled store1\n");
					break;
				}
				break;

			default:
				printf("Unhandled output instruction1\n");
				break;
			}
		}
		if (write_offset) {
			out_buf[write_offset] = 0;
			printf("cmd:%d:%s\n", write_offset, out_buf);
			write(fd, out_buf, write_offset);
		}
		write_offset = 0;
		if ((inst_log1->value3.value_type == 5) &&
			(!instruction->dstA.indirect) &&
			(instruction->dstA.store == STORE_REG) &&
			(instruction->dstA.index ==  REG_IP)) {
			/* FIXME: select correct return variable */
			/* Search for EAX */
			value = search_store(memory_reg,
					4, 4);
			printf("\treturn local%04d;\n}\n", value->value_id);
			tmp = snprintf(out_buf + write_offset, 1024 - write_offset, "\treturn local%04d;\n}\n", value->value_id);
			write_offset += tmp;
			write(fd, out_buf, write_offset);
			write_offset = 0;
		}
		//tmp = snprintf(out_buf, 1024, "%d\n", l);
		//write(fd, out_buf, tmp);
	}
	close(fd);
	bf_test_close_file(handle);
	return 0;
}

