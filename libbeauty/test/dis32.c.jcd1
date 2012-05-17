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
 * 29-03-2009 Updates.
 *   Copyright (C) 2009 James Courtier-Dutton James@superbug.co.uk
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
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dis.h>
#include <bfl.h>
#include <exe.h>
#include <rev.h>
#include <dis-asm.h>
#include <assert.h>

#define EIP_START 0x40000000

struct dis_instructions_s dis_instructions;
uint8_t *inst;
size_t inst_size = 0;
uint8_t *data;
size_t data_size = 0;
struct rev_eng *handle;
struct disassemble_info disasm_info;
disassembler_ftype disassemble_fn;
char *dis_flags_table[] = { " ", "f" };
uint64_t inst_log = 1;	/* Pointer to the current free instruction log entry. */
int local_counter = 1;
struct self_s *self = NULL;


char *condition_table[] = {
	"Unknown_0",
	"OVERFLOW_1", /* Signed */
	"NOT_OVERFLOW_2", /* Signed */
	"BELOW_3",	/* Unsigned */
	"NOT_BELOW_4",	/* Unsigned */
	"EQUAL_5",	/* Signed or Unsigned */
	"NOT_EQUAL_6",	/* Signed or Unsigned */
	"ABOVE_7",	/* Unsigned */
	"NOT_ABOVE_8",	/* Unsigned */
	"SIGNED_9",	/* Signed */
	"NO_SIGNED_10",	/* Signed */
	"PARITY_11",	/* Signed or Unsigned */
	"NOT_PARITY_12",/* Signed or Unsigned */
	"LESS_13",	/* Signed */
	"GREATER_EQUAL_14", /* Signed */
	"LESS_EQUAL_15",    /* Signed */
	"GREATER_16"	/* Signed */
};

struct relocation_s {
	int type; /* 0 = invalid, 1 = external_entry_point, 2 = data */
	uint64_t index; /* Index into the external_entry_point or data */
};

struct external_entry_point_s {
	int valid;
	int type; /* 1: Internal, 2: External */
	int section_offset;
	int section_id;
	int section_index;
	uint64_t value; /* pointer to original .text entry point */
	uint64_t inst_log; /* Where the function starts in the inst_log */
	uint64_t inst_log_end; /* Where the function ends in inst_log */
	struct process_state_s process_state;
	char *name;
	/* FIXME: add function return type and param types */
};

#define RELOCATION_SIZE 100
#define EXTERNAL_ENTRY_POINTS_SIZE 100
struct relocation_s relocations[RELOCATION_SIZE];
struct external_entry_point_s external_entry_points[EXTERNAL_ENTRY_POINTS_SIZE];

/* For the .text segment. I.e. Instructions. */
#define MEMORY_TEXT_SIZE 1000
#define MEMORY_STACK_SIZE 100
#define MEMORY_REG_SIZE 100
/* For the .data segment. I.e. Static data */
#define MEMORY_DATA_SIZE 1000
#define MEMORY_USED_SIZE 1000
#define INST_LOG_ENTRY_SIZE 100
#define ENTRY_POINTS_SIZE 100

/* Used to store details of each instruction.
 * Linked by prev/next pointers
 * so that a single list can store all program flow.
 */
struct inst_log_entry_s inst_log_entry[INST_LOG_ENTRY_SIZE];
int search_back_seen[INST_LOG_ENTRY_SIZE];

/* Used to keep record of where we have been before.
 * Used to identify program flow, branches, and joins.
 */
int memory_used[MEMORY_USED_SIZE];
/* Used to keep a non bfd version of the relocation entries */
int memory_relocation[MEMORY_USED_SIZE];

struct entry_point_s {
	int used;
	/* FIXME: Is this enough, or will full register backup be required */
	uint64_t esp_init_value;
	uint64_t esp_offset_value;
	uint64_t ebp_init_value;
	uint64_t ebp_offset_value;
	uint64_t eip_init_value;
	uint64_t eip_offset_value;
	uint64_t previous_instuction;
} ;

/* This is used to hold return values from process block */
struct entry_point_s entry_point[ENTRY_POINTS_SIZE];
uint64_t entry_point_list_length = ENTRY_POINTS_SIZE;

/* redirect is used for SSA correction, when one needs to rename a variable */
/* renaming the variable within the log entries would take too long. */
/* so use log entry value_id -> redirect -> label_s */
struct label_redirect_s {
	uint64_t redirect;
} ;

struct label_s {
	/* local = 1, param = 2, data = 3, mem = 4 */
	uint64_t scope;
	/* For local or param: reg = 1, stack = 2 */
	/* For data: data = 1, &data = 2, value = 3 */
	uint64_t type;
	/* value */
	uint64_t value;
	/* human readable name */
	char *name;
} ;

int write_inst(FILE *fd, struct instruction_s *instruction, int instruction_number)
{
	int ret = 1; /* Default to failed */
	int tmp;
	tmp = fprintf(fd, "// 0x%04x:%s%s",
		instruction_number,
		opcode_table[instruction->opcode],
		dis_flags_table[instruction->flags]);
	switch (instruction->opcode) {
	case MOV:
	case ADD:
	case SUB:
	case MUL:
	case CMP:
	/* FIXME: Add DIV */
	//case DIV:
	case JMP:
		if (instruction->srcA.indirect) {
			tmp = fprintf(fd, " %s[%s0x%"PRIx64"]%s,",
				indirect_table[instruction->srcA.indirect],
				store_table[instruction->srcA.store],
				instruction->srcA.index,
				size_table[instruction->srcA.value_size]);
		} else {
			tmp = fprintf(fd, " %s0x%"PRIx64"%s,",
				store_table[instruction->srcA.store],
				instruction->srcA.index,
				size_table[instruction->srcA.value_size]);
		}
		if (instruction->dstA.indirect) {
			tmp = fprintf(fd, " %s[%s0x%"PRIx64"]%s\n",
				indirect_table[instruction->dstA.indirect],
				store_table[instruction->dstA.store],
				instruction->dstA.index,
				size_table[instruction->dstA.value_size]);
		} else {
			tmp = fprintf(fd, " %s0x%"PRIx64"%s\n",
				store_table[instruction->dstA.store],
				instruction->dstA.index,
				size_table[instruction->dstA.value_size]);
		}
		ret = 0;
		break;
	case IF:
		tmp = fprintf(fd, " cond=%"PRIu64"", instruction->srcA.index);
		tmp = fprintf(fd, " JMP-REL=0x%"PRIx64"\n", instruction->dstA.index);
		ret = 0;
		break;
	case CALL:
		if (instruction->srcA.index < 100) {
			tmp = fprintf(fd, " %"PRIu64":%s()\n",
				instruction->srcA.index,
				external_entry_points[instruction->srcA.index].name);
		} else {
			tmp = fprintf(fd, " CALL FAILED\n");
		}
		ret = 0;
		break;
	case NOP:
		tmp = fprintf(fd, "\n");
		ret = 0;
		break;
	case RET:
		tmp = fprintf(fd, "\n");
		ret = 0;
		break;
	}
	return ret;
}

int print_inst(struct instruction_s *instruction, int instruction_number)
{
	int ret;

	ret = write_inst(stdout, instruction, instruction_number);
	return ret;
}

int print_dis_instructions(void)
{
	int n;
	struct instruction_s *instruction;
	struct inst_log_entry_s *inst_log1;
	for (n = 1; n <= inst_log; n++) {
		inst_log1 =  &inst_log_entry[n];
		instruction =  &inst_log1->instruction;
		if (print_inst(instruction, n))
			return 1;
		printf("start_address:%"PRIx64", %"PRIx64" -> %"PRIx64"\n",
			inst_log1->value1.start_address,
			inst_log1->value2.start_address,
			inst_log1->value3.start_address);
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
		printf("indirect value_id:%"PRIx64", %"PRIx64" -> %"PRIx64"\n",
			inst_log1->value1.indirect_value_id,
			inst_log1->value2.indirect_value_id,
			inst_log1->value3.indirect_value_id);
		printf("value_type:0x%x, 0x%x -> 0x%x\n",
			inst_log1->value1.value_type,
			inst_log1->value2.value_type,
			inst_log1->value3.value_type);
		printf("value_scope:0x%x, 0x%x -> 0x%x\n",
			inst_log1->value1.value_scope,
			inst_log1->value2.value_scope,
			inst_log1->value3.value_scope);
		printf("value_id:0x%"PRIx64", 0x%"PRIx64" -> 0x%"PRIx64"\n",
			inst_log1->value1.value_id,
			inst_log1->value2.value_id,
			inst_log1->value3.value_id);
		if (inst_log1->prev_size > 0) {
			int n;
			for (n = 0; n < inst_log1->prev_size; n++) {
				printf("inst_prev:%d:0x%04x\n",
					n,
					inst_log1->prev[n]);
			}
		}
		if (inst_log1->next_size > 0) {
			int n;
			for (n = 0; n < inst_log1->next_size; n++) {
				printf("inst_next:%d:0x%04x\n",
					n,
					inst_log1->next[n]);
			}
		}
	}
	return 0;
}

int get_value_from_index(operand_t *operand, uint64_t *index)
{
	if (operand->indirect) {
		printf(" %s%s[%s0x%"PRIx64"],",
			size_table[operand->value_size],
			indirect_table[operand->indirect],
			store_table[operand->store],
			operand->index);
	} else {
		printf(" %s%s0x%"PRIx64",",
		size_table[operand->value_size],
		store_table[operand->store],
		operand->index);
	}
	return 1;
}

int ram_init(struct memory_s *memory_data)
{
	return 0;
}

int reg_init(struct memory_s *memory_reg)
{
	/* esp */
	memory_reg[0].start_address = REG_SP;
	/* 4 bytes */
	memory_reg[0].length = 8;
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
	/* value_scope: 0 - unknown, 1 - Param, 2 - Local, 3 - Mem */
	memory_reg[0].value_scope = 0;
	/* Each time a new value is assigned, this value_id increases */
	memory_reg[0].value_id = 0;
	/* valid: 0 - Entry Not used yet, 1 - Entry Used */
	memory_reg[0].valid = 1;

	/* ebp */
	memory_reg[1].start_address = REG_BP;
	/* 4 bytes */
	memory_reg[1].length = 8;
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
	/* value_scope: 0 - unknown, 1 - Param, 2 - Local, 3 - Mem */
	memory_reg[1].value_scope = 0;
	/* Each time a new value is assigned, this value_id increases */
	memory_reg[1].value_id = 0;
	/* valid: 0 - entry Not used yet, 1 - entry Used */
	memory_reg[1].valid = 1;

	/* eip */
	memory_reg[2].start_address = REG_IP;
	/* 4 bytes */
	memory_reg[2].length = 8;
	/* 1 - Known */
	memory_reg[2].init_value_type = 1;
	/* Initial value when first accessed */
	memory_reg[2].init_value = EIP_START;
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
	/* value_scope: 0 - unknown, 1 - Param, 2 - Local, 3 - Mem */
	memory_reg[2].value_scope = 3;
	/* Each time a new value is assigned, this value_id increases */
	memory_reg[2].value_id = 0;
	/* valid: 0 - entry Not used yet, 1 - entry Used */
	memory_reg[2].valid = 1;
	return 0;
}

int stack_init(struct memory_s *memory_stack)
{
	int n = 0;
	/* eip on the stack */
	memory_stack[n].start_address = 0x10000;
	/* 4 bytes */
	memory_stack[n].length = 8;
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
	/* value_scope: 0 - unknown, 1 - Param, 2 - Local, 3 - Mem */
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
	/* value_scope: 0 - unknown, 1 - Param, 2 - Local, 3 - Mem */
	memory_stack[n].value_scope = 0;
	/* Each time a new value is assigned, this value_id increases */
	memory_stack[n].value_id = 0;
	/* valid: 0 - Not used yet, 1 - Used */
	memory_stack[n].valid = 1;
	n++;
#endif
	for (;n < MEMORY_STACK_SIZE; n++) {
		memory_stack[n].valid = 0;
	}
	return 0;
}

int print_mem(struct memory_s *memory, int location) {
	printf("start_address:0x%"PRIx64"\n",
		memory[location].start_address);
	printf("length:0x%x\n",
		memory[location].length);
	printf("init_value_type:0x%x\n",
		memory[location].init_value_type);
	printf("init:0x%"PRIx64"\n",
		memory[location].init_value);
	printf("offset:0x%"PRIx64"\n",
		memory[location].offset_value);
	printf("indirect_init:0x%"PRIx64"\n",
		memory[location].indirect_init_value);
	printf("indirect_offset:0x%"PRIx64"\n",
		memory[location].indirect_offset_value);
	printf("value_type:0x%x\n",
		memory[location].value_type);
	printf("ref_memory:0x%"PRIx32"\n",
		memory[location].ref_memory);
	printf("ref_log:0x%"PRIx32"\n",
		memory[location].ref_log);
	printf("value_scope:0x%x\n",
		memory[location].value_scope);
	printf("value_id:0x%"PRIx64"\n",
		memory[location].value_id);
	printf("valid:0x%"PRIx64"\n",
		memory[location].valid);
	return 0;
}

int process_block(struct process_state_s *process_state, struct rev_eng *handle, uint64_t inst_log_prev, uint64_t list_length, struct entry_point_s *entry, uint64_t eip_offset_limit) {
	uint64_t offset = 0;
	int result;
	int n = 0;
	int err;
	struct inst_log_entry_s *inst_exe_prev;
	struct inst_log_entry_s *inst_exe;
	struct instruction_s *instruction;
	int instruction_offset = 0;
	int octets = 0;
	struct memory_s *memory_text;
	struct memory_s *memory_stack;
	struct memory_s *memory_reg;
	struct memory_s *memory_data;
	int *memory_used;

	memory_text = process_state->memory_text;
	memory_stack = process_state->memory_stack;
	memory_reg = process_state->memory_reg;
	memory_data = process_state->memory_data;
	memory_used = process_state->memory_used;

	printf("process_block entry\n");
	printf("inst_log=%"PRId64"\n", inst_log);
	printf("dis:Data at %p, size=0x%"PRIx64"\n", inst, inst_size);
	for (offset = 0; ;) {
	//for (offset = 0; offset < inst_size;
			//offset += dis_instructions.bytes_used) {
		/* Update EIP */
		offset = memory_reg[2].offset_value;
		if (offset >= eip_offset_limit) {
			printf("Over ran offset=0x%"PRIx64" >= eip_offset_limit=0x%"PRIx64" \n",
				offset, eip_offset_limit);
			return 1;
		}
		dis_instructions.instruction_number = 0;
		dis_instructions.bytes_used = 0;
		printf("eip=0x%"PRIx64", offset=0x%"PRIx64"\n",
			memory_reg[2].offset_value, offset);
		result = disassemble(handle, &dis_instructions, inst, offset);
		printf("bytes used = %d\n", dis_instructions.bytes_used);
		/* Memory not used yet */
		if (0 == memory_used[offset]) {
			printf("Memory not used yet\n");
			for (n = 0; n < dis_instructions.bytes_used; n++) {
				memory_used[offset + n] = -n;
				printf(" 0x%02x", inst[offset + n]);
			}
			printf("\n");
			memory_used[offset] = inst_log;
		} else {
			int inst_this = memory_used[offset];
			/* If value == maxint, then it is the destination of a jump */
			/* But I need to separate the instruction flows */
			/* A jump/branch inst should create a new instruction tree */
			printf("Memory already used\n");
			inst_exe_prev = &inst_log_entry[inst_log_prev];
			inst_exe = &inst_log_entry[inst_this];
			printf("inst_exe_prev=%p, inst_exe=%p\n",
				inst_exe_prev, inst_exe);
			inst_exe->prev_size++;
			if (inst_exe->prev_size == 1) {
				inst_exe->prev = malloc(sizeof(inst_exe->prev));
			} else {
				inst_exe->prev = realloc(inst_exe->prev, sizeof(inst_exe->prev) * inst_exe->prev_size);
			}
			inst_exe->prev[inst_exe->prev_size - 1] = inst_log_prev;
			inst_exe_prev->next_size++;
			if (inst_exe_prev->next_size == 1) {
				inst_exe_prev->next = malloc(sizeof(inst_exe_prev->next));
				inst_exe_prev->next[inst_exe_prev->next_size - 1] = inst_this;
			} else {
				inst_exe_prev->next = realloc(inst_exe_prev->next, sizeof(inst_exe_prev->next) * inst_exe_prev->next_size);
				inst_exe_prev->next[inst_exe_prev->next_size - 1] = inst_this;
			}
			break;
		}	
		//printf("disassemble_fn\n");
		//disassemble_fn = disassembler (handle->bfd);
		//printf("disassemble_fn done\n");
		printf("disassemble: ");
		octets = (*disassemble_fn) (offset, &disasm_info);
		printf("  octets=%d\n", octets);
		if (dis_instructions.bytes_used != octets) {
			printf("Unhandled instruction. Length mismatch. Got %d, expected %d, Exiting\n", dis_instructions.bytes_used, octets);
			return 1;
		}
		/* Update EIP */
		memory_reg[2].offset_value += octets;

		printf("Number of RTL dis_instructions=%d\n",
			dis_instructions.instruction_number);
		if (result == 0) {
			printf("Unhandled instruction. Exiting\n");
			return 1;
		}
		if (dis_instructions.instruction_number == 0) {
			printf("NOP instruction. Get next inst\n");
			continue;
		}
		for (n = 0; n < dis_instructions.instruction_number; n++) {
			instruction = &dis_instructions.instruction[n];
			printf( "Printing inst1111:%d, %d, %"PRId64"\n",instruction_offset, n, inst_log);
			err = print_inst(instruction, instruction_offset + n + 1);
			if (err) {
				printf("print_inst failed\n");
				return err;
			}
			inst_exe_prev = &inst_log_entry[inst_log_prev];
			inst_exe = &inst_log_entry[inst_log];
			memcpy(&(inst_exe->instruction), instruction, sizeof(struct instruction_s));
			err = execute_instruction(self, process_state, inst_exe);
			if (err) {
				printf("execute_intruction failed err=%d\n", err);
				return err;
			}
			inst_exe->prev_size++;
			if (inst_exe->prev_size == 1) {
				inst_exe->prev = malloc(sizeof(inst_exe->prev));
			} else {
				inst_exe->prev = realloc(inst_exe->prev, sizeof(inst_exe->prev) * inst_exe->prev_size);
			}
			inst_exe->prev[inst_exe->prev_size - 1] = inst_log_prev;
			inst_exe_prev->next_size++;
			if (inst_exe_prev->next_size == 1) {
				inst_exe_prev->next = malloc(sizeof(inst_exe_prev->next));
				inst_exe_prev->next[inst_exe_prev->next_size - 1] = inst_log;
			} else {
				inst_exe_prev->next = realloc(inst_exe_prev->next, sizeof(inst_exe_prev->next) * inst_exe_prev->next_size);
				inst_exe_prev->next[inst_exe_prev->next_size - 1] = inst_log;
			}
			inst_exe_prev->next[inst_exe_prev->next_size - 1] = inst_log;

			inst_log_prev = inst_log;
			inst_log++;
			if (0 == memory_reg[2].offset_value) {
				printf("Function exited\n");
				if (inst_exe_prev->instruction.opcode == NOP) {
					inst_exe_prev->instruction.opcode = RET;
				}
				break;
			}
		}
		instruction_offset += dis_instructions.instruction_number;
		if (0 == memory_reg[2].offset_value) {
			printf("Breaking\n");
			break;
		}
		if (IF == instruction->opcode) {
			printf("Breaking at IF\n");
			printf("IF: this EIP = 0x%"PRIx64"\n",
				memory_reg[2].offset_value);
			printf("IF: jump dst abs EIP = 0x%"PRIx64"\n",
				inst_exe->value3.offset_value);
			printf("IF: inst_log = %"PRId64"\n",
				inst_log);
			for (n = 0; n < list_length; n++ ) {
				if (0 == entry[n].used) {
					entry[n].esp_init_value = memory_reg[0].init_value;
					entry[n].esp_offset_value = memory_reg[0].offset_value;
					entry[n].ebp_init_value = memory_reg[1].init_value;
					entry[n].ebp_offset_value = memory_reg[1].offset_value;
					entry[n].eip_init_value = memory_reg[2].init_value;
					entry[n].eip_offset_value = memory_reg[2].offset_value;
					entry[n].previous_instuction = inst_log - 1;
					entry[n].used = 1;
					break;
				}
			}
			/* FIXME: Would starting a "n" be better here? */
			for (n = 0; n < list_length; n++ ) {
				if (0 == entry[n].used) {
					entry[n].esp_init_value = memory_reg[0].init_value;
					entry[n].esp_offset_value = memory_reg[0].offset_value;
					entry[n].ebp_init_value = memory_reg[1].init_value;
					entry[n].ebp_offset_value = memory_reg[1].offset_value;
					entry[n].eip_init_value = inst_exe->value3.init_value;
					entry[n].eip_offset_value = inst_exe->value3.offset_value;
					entry[n].previous_instuction = inst_log - 1;
					entry[n].used = 1;
					break;
				}
			}
			break;
		}
	}
	return 0;
}

int log_to_label(int store, int indirect, uint64_t index, uint64_t relocated, uint64_t value_scope, uint64_t value_id, uint64_t indirect_offset_value, uint64_t indirect_value_id, struct label_s *label) {
	int tmp;
	/* FIXME: May handle by using first switch as switch (indirect) */
	switch (store) {
	case STORE_DIRECT:
		/* FIXME: Handle the case of an immediate value being &data */
		/* but it is very difficult to know if the value is a pointer (&data) */
		/* or an offset (data[x]) */
		/* need to use the relocation table to find out */
		/* no relocation table entry == offset */
		/* relocation table entry == pointer */
		/* this info should be gathered at disassembly point */
		/* FIXME: relocation table not present in 16bit x86 mode, so another method will need to be found */
		if (indirect == IND_MEM) {
			label->scope = 3;
			label->type = 1;
			label->value = index;
		} else if (relocated) {
			label->scope = 3;
			label->type = 2;
			label->value = index;
		} else {
			label->scope = 3;
			label->type = 3;
			label->value = index;
		}
		break;
	case STORE_REG:
		switch (value_scope) {
		case 1:
			if (IND_STACK == indirect) {
				label->scope = 2;
				label->type = 2;
				label->value = index;
			} else if (0 == indirect) {
				label->scope = 2;
				label->type = 1;
				label->value = index;
			}
			break;
		case 2:
			if (IND_STACK == indirect) {
				label->scope = 1;
				label->type = 2;
				label->value = value_id;
			} else if (0 == indirect) {
				label->scope = 1;
				label->type = 1;
				label->value = value_id;
			}
			break;
		case 3: /* Data */
			/* FIXME: introduce indirect_value_id and indirect_value_scope */
			/* in order to resolve somewhere */
			/* It will always be a register, and therefore can re-use the */
			/* value_id to identify it. */
			/* It will always be a local and not a param */
			label->scope = 4;
			label->type = 1;
			label->value = indirect_value_id;
			break;
		default:
			label->scope = 0;
			label->type = value_scope;
			label->value = 0;
			printf("unknown value scope: %04"PRIx64";\n", (value_scope));
			break;
		}
		break;
	default:
		printf("Unhandled store1\n");
		break;
	}
	return 0;
}

int output_label(struct label_s *label, FILE *fd) {
	int tmp;

	switch (label->scope) {
	case 3:
		printf("%"PRIx64";\n", label->value);
		/* FIXME: Handle the case of an immediate value being &data */
		/* but it is very difficult to know if the value is a pointer (&data) */
		/* or an offset (data[x]) */
		/* need to use the relocation table to find out */
		/* no relocation table entry == offset */
		/* relocation table entry == pointer */
		/* this info should be gathered at disassembly point */
		switch (label->type) {
		case 1:
			tmp = fprintf(fd, "data%04"PRIx64,
				label->value);
			break;
		case 2:
			tmp = fprintf(fd, "&data%04"PRIx64,
				label->value);
			break;
		case 3:
			tmp = fprintf(fd, "0x%"PRIx64,
				label->value);
			break;
		default:
			printf("output_label error\n");
			return 1;
			break;
		}
		break;
	case 2:
		switch (label->type) {
		case 2:
			printf("param_stack%04"PRIx64,
				label->value);
			tmp = fprintf(fd, "param_stack%04"PRIx64,
				label->value);
			break;
		case 1:
			printf("param_reg%04"PRIx64,
				label->value);
			tmp = fprintf(fd, "param_reg%04"PRIx64,
				label->value);
			break;
		default:
			printf("output_label error\n");
			return 1;
			break;
		}
		break;
	case 1:
		switch (label->type) {
		case 2:
			printf("local_stack%04"PRIx64,
				label->value);
			tmp = fprintf(fd, "local_stack%04"PRIx64,
				label->value);
			break;
		case 1:
			printf("local_reg%04"PRIx64,
				label->value);
			tmp = fprintf(fd, "local_reg%04"PRIx64,
				label->value);
			break;
		default:
			printf("output_label error\n");
			return 1;
			break;
		}
		break;
	case 4:
		/* FIXME: introduce indirect_value_id and indirect_value_scope */
		/* in order to resolve somewhere */
		/* It will always be a register, and therefore can re-use the */
		/* value_id to identify it. */
		/* It will always be a local and not a param */
		printf("*local_reg%04"PRIx64";\n", label->value);
		tmp = fprintf(fd, "*local_reg%04"PRIx64,
			label->value);
		break;
	default:
		printf("unknown label scope: %04"PRIx64";\n", label->scope);
		tmp = fprintf(fd, "unknown%04"PRIx64,
			label->scope);
		break;
	}
	return 0;
}

int output_variable(int store, int indirect, uint64_t index, uint64_t relocated, uint64_t value_scope, uint64_t value_id, uint64_t indirect_offset_value, uint64_t indirect_value_id, FILE *fd) {
	int tmp;
	/* FIXME: May handle by using first switch as switch (indirect) */
	switch (store) {
	case STORE_DIRECT:
		printf("%"PRIx64";\n", index);
		/* FIXME: Handle the case of an immediate value being &data */
		/* but it is very difficult to know if the value is a pointer (&data) */
		/* or an offset (data[x]) */
		/* need to use the relocation table to find out */
		/* no relocation table entry == offset */
		/* relocation table entry == pointer */
		/* this info should be gathered at disassembly point */
		if (indirect == IND_MEM) {
			tmp = fprintf(fd, "data%04"PRIx64,
				index);
		} else if (relocated) {
			tmp = fprintf(fd, "&data%04"PRIx64,
				index);
		} else {
			tmp = fprintf(fd, "0x%"PRIx64,
				index);
		}
		break;
	case STORE_REG:
		switch (value_scope) {
		case 1:
			/* FIXME: Should this be param or instead param_reg, param_stack */
			if (IND_STACK == indirect) {
				printf("param_stack%04"PRIx64",%04"PRIx64",%04d",
					index, indirect_offset_value, indirect);
				tmp = fprintf(fd, "param_stack%04"PRIx64",%04"PRIx64",%04d",
					index, indirect_offset_value, indirect);
			} else if (0 == indirect) {
				printf("param_reg%04"PRIx64,
					index);
				tmp = fprintf(fd, "param_reg%04"PRIx64,
					index);
			}
			break;
		case 2:
			/* FIXME: Should this be local or instead local_reg, local_stack */
			if (IND_STACK == indirect) {
				printf("local_stack%04"PRIx64,
					value_id);
				tmp = fprintf(fd, "local_stack%04"PRIx64,
					value_id);
			} else if (0 == indirect) {
				printf("local_reg%04"PRIx64,
					value_id);
				tmp = fprintf(fd, "local_reg%04"PRIx64,
					value_id);
			}
			break;
		case 3: /* Data */
			/* FIXME: introduce indirect_value_id and indirect_value_scope */
			/* in order to resolve somewhere */
			/* It will always be a register, and therefore can re-use the */
			/* value_id to identify it. */
			/* It will always be a local and not a param */
			printf("*local_mem%04"PRIx64";\n", (indirect_value_id));
			tmp = fprintf(fd, "*local_mem%04"PRIx64,
				indirect_value_id);
			break;
		default:
			printf("unknown value scope: %04"PRIx64";\n", (value_scope));
			tmp = fprintf(fd, "unknown%04"PRIx64,
				value_scope);
			break;
		}
		break;
	default:
		printf("Unhandled store1\n");
		break;
	}
	return 0;
}

int if_expression( int condition, struct inst_log_entry_s *inst_log1_flagged,
	struct label_redirect_s *label_redirect, struct label_s *labels, FILE *fd)
{
	int opcode = inst_log1_flagged->instruction.opcode;
	int err = 0;
	int tmp;
	int store;
	int indirect;
	uint64_t index;
	uint64_t relocated;
	uint64_t value_scope;
	uint64_t value_id;
	uint64_t indirect_offset_value;
	uint64_t indirect_value_id;
	struct label_s *label;

	switch (opcode) {
	case CMP:
		switch (condition) {
		case LESS_EQUAL:
			tmp = fprintf(fd, "(");
			tmp = label_redirect[inst_log1_flagged->value2.value_id].redirect;
			label = &labels[tmp];
			tmp = output_label(label, fd);
			tmp = fprintf(fd, " <= ");
			tmp = label_redirect[inst_log1_flagged->value1.value_id].redirect;
			label = &labels[tmp];
			tmp = output_label(label, fd);
			tmp = fprintf(fd, ") ");
			break;
		default:
			err = 1;
			break;
		break;
		}
	default:
		err = 1;
		break;
	}
	return err;
}

/* If relocated_data returns 1, it means that there was a
 * relocation table entry for this data location.
 * This most likely means that this is a pointer.
 * FIXME: What to do if the relocation is to the code segment? Pointer to function?
 */
uint32_t relocated_data(struct rev_eng *handle, uint64_t offset, uint64_t size)
{
	int n;
	for (n = 0; n < handle->reloc_table_data_sz; n++) {
		if (handle->reloc_table_data[n].address == offset) {
			return 1;
		}
	}
	return 0;
}


uint32_t output_function_name(FILE *fd,
		struct external_entry_point_s *external_entry_point,
		int *param_present, int *param_size)
{
	int commas = 0;
	int tmp, n;

	printf("int %s()\n{\n", external_entry_point->name);
	printf("value = %"PRIx64"\n", external_entry_point->value);
	tmp = fprintf(fd, "int %s(", external_entry_point->name);
	for (n = 0; n < 100; n++) {
		if (param_present[n]) {
			printf("param%04x\n", n);
			if (commas) {
				tmp = fprintf(fd, ", ");
			}
			tmp = fprintf(fd, "param%04"PRIx32"", n);
			commas = 1;
			tmp = param_size[n];
			n += tmp;
		}
	}

	tmp = fprintf(fd, ")\n{\n");
	return 0;
}

int output_function_body(struct process_state_s *process_state,
			 FILE *fd, int start, int end, struct label_redirect_s *label_redirect, struct label_s *labels)
{
	int tmp, n;
	int err;
	struct instruction_s *instruction;
	struct instruction_s *instruction_prev;
	struct inst_log_entry_s *inst_log1;
	struct inst_log_entry_s *inst_log1_prev;
	struct memory_s *value;
	struct label_s *label;

	if (!start || !end) {
		printf("output_function_body:Invalid start or end\n");
		return 1;
	}
	printf("output_function_body:start=0x%x, end=0x%x\n", start, end);

	for (n = start; n <= end; n++) {
		inst_log1 =  &inst_log_entry[n];
		if (!inst_log1) {
			printf("output_function_body:Invalid inst_log1[0x%x]\n", n);
			return 1;
		}
		inst_log1_prev =  &inst_log_entry[inst_log1->prev[0]];
		if (!inst_log1_prev) {
			printf("output_function_body:Invalid inst_log1_prev[0x%x]\n", n);
			return 1;
		}
		instruction =  &inst_log1->instruction;
		instruction_prev =  &inst_log1_prev->instruction;

		write_inst(fd, instruction, n);
		/* Output labels when this is a join point */
		/* or when the previous instruction was some sort of jump */
		if ((inst_log1->prev_size) > 1) {
			printf("label%04"PRIx32":\n", n);
			tmp = fprintf(fd, "label%04"PRIx32":\n", n);
		} else {
			if ((inst_log1->prev[0] != (n - 1)) &&
				(inst_log1->prev[0] != 0)) {		
				printf("label%04"PRIx32":\n", n);
				tmp = fprintf(fd, "label%04"PRIx32":\n", n);
			}
		}
			
		/* Test to see if we have an instruction to output */
		printf("Inst 0x%04x: %d: value_scope = %d, %d, %d\n", n,
			instruction->opcode,
			inst_log1->value1.value_scope,
			inst_log1->value2.value_scope,
			inst_log1->value3.value_scope);
		if ((0 == inst_log1->value3.value_type) ||
			(1 == inst_log1->value3.value_type) ||
			(2 == inst_log1->value3.value_type) ||
			(3 == inst_log1->value3.value_type) ||
			(5 == inst_log1->value3.value_type)) {
			switch (instruction->opcode) {
			case MOV:
				if (inst_log1->value1.value_type == 6)
					break;
				/* FIXME: This might need adding back in when
				 *        complex EIP jumps happen
				 */
				if (inst_log1->value1.value_type == 5)
					break;
				if (print_inst(instruction, n))
					return 1;
				printf("\t");
				tmp = fprintf(fd, "\t");
				/* FIXME: Check limits */
				tmp = label_redirect[inst_log1->value3.value_id].redirect;
				label = &labels[tmp];
				tmp = output_label(label, fd);
				tmp = fprintf(fd, " = ");

				printf("\nstore=%d\n", instruction->srcA.store);
				tmp = label_redirect[inst_log1->value1.value_id].redirect;
				label = &labels[tmp];
				tmp = output_label(label, fd);
				tmp = fprintf(fd, ";\n");

				break;
			case ADD:
				if (print_inst(instruction, n))
					return 1;
				printf("\t");
				tmp = fprintf(fd, "\t");
				tmp = label_redirect[inst_log1->value3.value_id].redirect;
				label = &labels[tmp];
				tmp = output_label(label, fd);
				tmp = fprintf(fd, " += ");
				printf("\nstore=%d\n", instruction->srcA.store);
				tmp = label_redirect[inst_log1->value1.value_id].redirect;
				label = &labels[tmp];
				tmp = output_label(label, fd);
				tmp = fprintf(fd, ";\n");
				break;
			case MUL:
				if (print_inst(instruction, n))
					return 1;
				printf("\t");
				tmp = fprintf(fd, "\t");
				tmp = label_redirect[inst_log1->value3.value_id].redirect;
				label = &labels[tmp];
				tmp = output_label(label, fd);
				tmp = fprintf(fd, " *= ");
				printf("\nstore=%d\n", instruction->srcA.store);
				tmp = label_redirect[inst_log1->value1.value_id].redirect;
				label = &labels[tmp];
				tmp = output_label(label, fd);
				tmp = fprintf(fd, ";\n");
				break;
			case SUB:
				if (print_inst(instruction, n))
					return 1;
				printf("\t");
				tmp = fprintf(fd, "\t");
				tmp = label_redirect[inst_log1->value3.value_id].redirect;
				label = &labels[tmp];
				tmp = output_label(label, fd);
				tmp = fprintf(fd, " -= ");
				printf("\nstore=%d\n", instruction->srcA.store);
				tmp = label_redirect[inst_log1->value1.value_id].redirect;
				label = &labels[tmp];
				tmp = output_label(label, fd);
				tmp = fprintf(fd, ";\n");
				break;
			case JMP:
				printf("JMP reached XXXX\n");
				if (print_inst(instruction, n))
					return 1;
				tmp = fprintf(fd, "\t");
				if (instruction->srcA.relocated) {
					printf("goto rel%08"PRIx64";\n", instruction->srcA.index);
					tmp = fprintf(fd, "goto rel%08"PRIx64";\n",
						instruction->srcA.index);
				} else {
					printf("goto label%04"PRIx32";\n",
						inst_log1->next[0]);
					tmp = fprintf(fd, "goto label%04"PRIx32";\n",
						inst_log1->next[0]);
				}
				break;
			case CALL:
				/* FIXME: This does nothing at the moment. */
				if (print_inst(instruction, n))
					return 1;
				/* Search for EAX */
				printf("\t");
				tmp = fprintf(fd, "\t");
				tmp = label_redirect[inst_log1->value3.value_id].redirect;
				label = &labels[tmp];
				tmp = output_label(label, fd);
				printf(" = ");
				tmp = fprintf(fd, " = ");

				tmp = fprintf(fd, "%s();\n", 
					external_entry_points[instruction->srcA.index].name);
				printf("%s();\n",
					external_entry_points[instruction->srcA.index].name);
//				tmp = fprintf(fd, "/* call(); */\n");
//				printf("/* call(); */\n");
				break;

			case CMP:
				/* Don't do anything for this instruction. */
				/* only does anything if combined with a branch instruction */
				if (print_inst(instruction, n))
					return 1;
				tmp = fprintf(fd, "\t");
				tmp = fprintf(fd, "/* cmp; */\n");
				printf("/* cmp; */\n");
				break;

			case IF:
				/* FIXME: Never gets here, why? */
				/* Don't do anything for this instruction. */
				/* only does anything if combined with a branch instruction */
				if (print_inst(instruction, n))
					return 1;
				printf("\t");
				tmp = fprintf(fd, "\t");
				printf("if ");
				tmp = fprintf(fd, "if ");
				err = if_expression( instruction->srcA.index, inst_log1_prev, label_redirect, labels, fd);
				printf("\t prev=%d, ",inst_log1->prev[0]);
				printf("\t prev inst=%d, ",instruction_prev->opcode);
				printf("\t %s", condition_table[instruction->srcA.index]);
				printf("\t LHS=%d, ",inst_log1->prev[0]);
				printf("goto label%04"PRIx32";\n", inst_log1->next[1]);
				tmp = fprintf(fd, "goto label%04"PRIx32";\n", inst_log1->next[1]);
				break;

			case NOP:
				if (print_inst(instruction, n))
					return 1;
				break;
			case RET:
				if (print_inst(instruction, n))
					return 1;
				printf("\t");
				tmp = fprintf(fd, "\t");
				printf("return\n");
				tmp = fprintf(fd, "return ");
				tmp = label_redirect[inst_log1->value1.value_id].redirect;
				label = &labels[tmp];
				tmp = output_label(label, fd);
				tmp = fprintf(fd, ";\n");
				break;
			default:
				printf("Unhandled output instruction1\n");
				if (print_inst(instruction, n))
					return 1;
				return 1;
				break;
			}
		}
	}
	if (0 < inst_log1->next_size && inst_log1->next[0]) {		
		printf("\tgoto label%04"PRIx32";\n", inst_log1->next[0]);
		tmp = fprintf(fd, "\tgoto label%04"PRIx32";\n", inst_log1->next[0]);
	}
	tmp = fprintf(fd, "}\n\n");

	return 0;
}
/***********************************************************************************
 * This is a complex routine. It utilises dynamic lists in order to reduce 
 * memory usage.
 **********************************************************************************/
int search_back_local_stack(int start_location, uint64_t indirect_init_value, uint64_t indirect_offset_value, uint64_t *size, uint64_t **inst_list)
{
	struct instruction_s *instruction;
	struct inst_log_entry_s *inst_log1;
	uint64_t value_id;
	uint64_t mid_start_size;
	uint64_t inst_num;
	uint64_t tmp;
	int found = 0;
	int n;
	struct mid_start_s {
		uint64_t mid_start;
		uint64_t valid;
	};
	struct mid_start_s *mid_start;

	*size = 0;
	/* FIXME: This could be optimized out if the "seen" value just increased on each call */
	for (n = 0; n < INST_LOG_ENTRY_SIZE; n++) {
		search_back_seen[n] = 0;
	}
	inst_log1 =  &inst_log_entry[start_location];
	instruction =  &inst_log1->instruction;
	value_id = inst_log1->value1.value_id;
	/* FIXME: complete this */
	printf("search_back_local_stack: 0x%"PRIx64", 0x%"PRIx64"\n", indirect_init_value, indirect_offset_value);
	if (0 < inst_log1->prev_size) {
		printf("search_back:prev_size=0x%x\n", inst_log1->prev_size);
	}
	if (0 == inst_log1->prev_size) {
		printf("search_back ended\n");
		return 1;
	}
	if (0 < inst_log1->prev_size) {
		mid_start = calloc(inst_log1->prev_size, sizeof(struct mid_start_s));
		mid_start_size = inst_log1->prev_size;
		for (n = 0; n < inst_log1->prev_size; n++) {
			mid_start[n].mid_start = inst_log1->prev[n];
			mid_start[n].valid = 1;
			printf("mid_start added 0x%"PRIx64" at 0x%x\n", mid_start[n].mid_start, n);
		}
	}
	do {
		found = 0;
		for(n = 0; n < mid_start_size; n++) {
			if (1 == mid_start[n].valid) {
				inst_num = mid_start[n].mid_start;
				mid_start[n].valid = 0;
				found = 1;
				printf("mid_start removed 0x%"PRIx64" at 0x%x, size=0x%"PRIx64"\n", mid_start[n].mid_start, n, mid_start_size);
				break;
			}
		}
		if (!found) {
			printf("mid_start not found, exiting\n");
			goto search_back_exit_free;
		}
		if (search_back_seen[inst_num]) {
			continue;
		}
		search_back_seen[inst_num] = 1;
		inst_log1 =  &inst_log_entry[inst_num];
		instruction =  &inst_log1->instruction;
		value_id = inst_log1->value3.value_id;
		printf("inst_num:0x%"PRIx64"\n", inst_num);
		if ((instruction->dstA.store == STORE_REG) &&
			(inst_log1->value3.value_scope == 2) &&
			(instruction->dstA.indirect == IND_STACK) &&
			(inst_log1->value3.indirect_init_value == indirect_init_value) &&
			(inst_log1->value3.indirect_offset_value == indirect_offset_value)) {
			tmp = *size;
			tmp++;
			*size = tmp;
			if (tmp == 1) {
				*inst_list = malloc(sizeof(*inst_list));
				(*inst_list)[0] = inst_num;
			} else {
				*inst_list = realloc(*inst_list, tmp * sizeof(*inst_list));
				(*inst_list)[tmp - 1] = inst_num;
			}
		} else {
			if ((inst_log1->prev_size > 0) &&
				(inst_log1->prev[0] != 0)) {
				int prev_index;
				found = 0;
				prev_index = 0;
				for(n = 0; n < mid_start_size; n++) {
					if (0 == mid_start[n].valid) {
						mid_start[n].mid_start = inst_log1->prev[prev_index];
						prev_index++;
						mid_start[n].valid = 1;
						printf("mid_start added 0x%"PRIx64" at 0x%x\n", mid_start[n].mid_start, n);
						found = 1;
					}
					if (prev_index >= inst_log1->prev_size) {
						break;
					}
				}
				if (prev_index < inst_log1->prev_size) {
					uint64_t mid_next;
					mid_next = mid_start_size + inst_log1->prev_size - prev_index;
					mid_start = realloc(mid_start, mid_next * sizeof(struct mid_start_s));
					for(n = mid_start_size; n < mid_next; n++) {
						mid_start[n].mid_start = inst_log1->prev[prev_index];
						prev_index++;
						printf("mid_start realloc added 0x%"PRIx64" at 0x%x\n", mid_start[n].mid_start, n);
						mid_start[n].valid = 1;
					}
					mid_start_size = mid_next;
				}
				
				if (!found) {
					printf("not found\n");
					goto search_back_exit_free;
				}
			}
		}
	/* FIXME: There must be deterministic exit point */
	} while (1);
	printf("end of loop, exiting\n");

search_back_exit_free:
	free(mid_start);
	return 0;
	
}

int main(int argc, char *argv[])
{
	int n = 0;
//	uint64_t offset = 0;
//	int instruction_offset = 0;
//	int octets = 0;
//	int result;
	char *filename;
	uint32_t arch;
	uint64_t mach;
	FILE *fd;
	int tmp;
	int err;
	const char *file = "test.obj";
	size_t inst_size = 0;
//	uint64_t reloc_size = 0;
	int l;
	struct instruction_s *instruction;
//	struct instruction_s *instruction_prev;
	struct inst_log_entry_s *inst_log1;
//	struct inst_log_entry_s *inst_log1_prev;
	struct inst_log_entry_s *inst_exe;
//	struct memory_s *value;
	uint64_t inst_log_prev = 0;
	int param_present[100];
	int param_size[100];
	char *expression;
	int not_finished;
	struct memory_s *memory_text;
	struct memory_s *memory_stack;
	struct memory_s *memory_reg;
	struct memory_s *memory_data;
	int *memory_used;
	struct label_redirect_s *label_redirect;
	struct label_s *labels;

	expression = malloc(1000); /* Buffer for if expressions */

	handle = bf_test_open_file(file);
	if (!handle) {
		printf("Failed to find or recognise file\n");
		return 1;
	}
	tmp = bf_get_arch_mach(handle, &arch, &mach);
	if ((arch != 9) ||
		(mach != 1)) {
		printf("File not the correct arch and mach\n");
		return 1;
	}

	printf("symtab_size = %ld\n", handle->symtab_sz);
	for (l = 0; l < handle->symtab_sz; l++) {
		printf("%d\n", l);
		printf("type:0x%02x\n", handle->symtab[l]->flags);
		printf("name:%s\n", handle->symtab[l]->name);
		printf("value=0x%02"PRIx64"\n", handle->symtab[l]->value);
		printf("section=%p\n", handle->symtab[l]->section);
		printf("section name=%s\n", handle->symtab[l]->section->name);
		printf("section flags=0x%02x\n", handle->symtab[l]->section->flags);
		printf("section index=0x%02"PRIx32"\n", handle->symtab[l]->section->index);
		printf("section id=0x%02"PRIx32"\n", handle->symtab[l]->section->id);
	}

	printf("sectiontab_size = %ld\n", handle->section_sz);
	for (l = 0; l < handle->section_sz; l++) {
		printf("%d\n", l);
		printf("flags:0x%02x\n", handle->section[l]->flags);
		printf("name:%s\n", handle->section[l]->name);
		printf("index=0x%02"PRIx32"\n", handle->section[l]->index);
		printf("id=0x%02"PRIx32"\n", handle->section[l]->id);
		printf("sectio=%p\n", handle->section[l]);
	}


	printf("Setup ok\n");
	inst_size = bf_get_code_size(handle);
	inst = malloc(inst_size);
	bf_copy_code_section(handle, inst, inst_size);
	printf("dis:.text Data at %p, size=0x%"PRIx64"\n", inst, inst_size);
	for (n = 0; n < inst_size; n++) {
		printf(" 0x%02x", inst[n]);
	}
	printf("\n");

	data_size = bf_get_data_size(handle);
	data = malloc(data_size);
	self = malloc(sizeof *self);
	printf("sizeof struct self_s = 0x%"PRIx64"\n", sizeof *self);
	self->data_size = data_size;
	self->data = data;
	bf_copy_data_section(handle, data, data_size);
	printf("dis:.data Data at %p, size=0x%"PRIx64"\n", data, data_size);
	for (n = 0; n < data_size; n++) {
		printf(" 0x%02x", data[n]);
	}
	printf("\n");

	bf_get_reloc_table_code_section(handle);
	for (n = 0; n < handle->reloc_table_code_sz; n++) {
		printf("reloc_table_code:addr = 0x%"PRIx64", size = 0x%"PRIx64", section_index = 0x%"PRIx64", section_name=%s, symbol_name=%s\n",
			handle->reloc_table_code[n].address,
			handle->reloc_table_code[n].size,
			handle->reloc_table_code[n].section_index,
			handle->reloc_table_code[n].section_name,
			handle->reloc_table_code[n].symbol_name);
	}

	bf_get_reloc_table_data_section(handle);
	for (n = 0; n < handle->reloc_table_data_sz; n++) {
		printf("reloc_table_data:addr = 0x%"PRIx64", size = 0x%"PRIx64", section = 0x%"PRIx64"\n",
			handle->reloc_table_data[n].address,
			handle->reloc_table_data[n].size,
			handle->reloc_table_data[n].section_index);
	}
	
	printf("handle=%p\n", handle);
	
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
	dis_instructions.bytes_used = 0;
	inst_exe = &inst_log_entry[0];
	/* Where should entry_point_list_length be initialised */
	entry_point_list_length = 100;
	/* Print the symtab */
	printf("symtab_sz = %lu\n", handle->symtab_sz);
	if (handle->symtab_sz >= 100) {
		printf("symtab too big!!! EXITING\n");
		return 1;
	}
	n = 0;
	for (l = 0; l < handle->symtab_sz; l++) {
		size_t length;
		/* FIXME: value == 0 for the first function in the .o file. */
		/*        We need to be able to handle more than
		          one function per .o file. */
		printf("section_id = %d, section_index = %d, flags = 0x%04x, value = 0x%04"PRIx64"\n",
			handle->symtab[l]->section->id,
			handle->symtab[l]->section->index,
			handle->symtab[l]->flags,
			handle->symtab[l]->value);
		if ((handle->symtab[l]->flags & 0x8) ||
			(handle->symtab[l]->flags == 0)) {
			external_entry_points[n].valid = 1;
			/* 1: Public function entry point
			 * 2: Private function entry point
			 * 3: Private label entry point
			 */
			if (handle->symtab[l]->flags & 0x8) {
				external_entry_points[n].type = 1;
			} else {
				external_entry_points[n].type = 2;
			}
			external_entry_points[n].section_offset = l;
			external_entry_points[n].section_id = 
				handle->symtab[l]->section->id;
			external_entry_points[n].section_index = 
				handle->symtab[l]->section->index;
			external_entry_points[n].value = handle->symtab[l]->value;
			length = strlen(handle->symtab[l]->name);
			external_entry_points[n].name = malloc(length+1);
			strncpy(external_entry_points[n].name, handle->symtab[l]->name, length+1);
			external_entry_points[n].process_state.memory_text =
				calloc(MEMORY_TEXT_SIZE, sizeof(struct memory_s));
			external_entry_points[n].process_state.memory_stack =
				calloc(MEMORY_STACK_SIZE, sizeof(struct memory_s));
			external_entry_points[n].process_state.memory_reg =
				calloc(MEMORY_REG_SIZE, sizeof(struct memory_s));
			external_entry_points[n].process_state.memory_data =
				calloc(MEMORY_DATA_SIZE, sizeof(struct memory_s));
			external_entry_points[n].process_state.memory_used =
				calloc(MEMORY_USED_SIZE, sizeof(int));
			memory_text = external_entry_points[n].process_state.memory_text;
			memory_stack = external_entry_points[n].process_state.memory_stack;
			memory_reg = external_entry_points[n].process_state.memory_reg;
			memory_data = external_entry_points[n].process_state.memory_data;
			memory_used = external_entry_points[n].process_state.memory_used;

			ram_init(memory_data);
			reg_init(memory_reg);
			stack_init(memory_stack);
			/* Set EIP entry point equal to symbol table entry point */
			//memory_reg[2].init_value = EIP_START;
			memory_reg[2].offset_value = external_entry_points[n].value;

			print_mem(memory_reg, 1);

			n++;
		}

	}
	printf("Number of functions = %d\n", n);
	for (n = 0; n < 100; n++) {
		if (external_entry_points[n].valid != 0) {
		printf("type = %d, sect_offset = %d, sect_id = %d, sect_index = %d, &%s() = 0x%04"PRIx64"\n",
			external_entry_points[n].type,
			external_entry_points[n].section_offset,
			external_entry_points[n].section_id,
			external_entry_points[n].section_index,
			external_entry_points[n].name,
			external_entry_points[n].value);
		}
	}
	for (n = 0; n < handle->reloc_table_code_sz; n++) {
		int len, len1;

		len = strlen(handle->reloc_table_code[n].symbol_name);
		for (l = 0; l < 100; l++) {
			if (external_entry_points[l].valid != 0) {
				len1 = strlen(external_entry_points[l].name);
				if (len != len1) {
					continue;
				}
				tmp = strncmp(external_entry_points[l].name, handle->reloc_table_code[n].symbol_name, len);
				if (0 == tmp) {
					handle->reloc_table_code[n].external_functions_index = l;
					handle->reloc_table_code[n].type =
						external_entry_points[l].type;
				}
			}
		}
	}
	for (n = 0; n < handle->reloc_table_code_sz; n++) {
		printf("reloc_table_code:addr = 0x%"PRIx64", size = 0x%"PRIx64", type = %d, function_index = 0x%"PRIx64", section_name=%s, symbol_name=%s\n",
			handle->reloc_table_code[n].address,
			handle->reloc_table_code[n].size,
			handle->reloc_table_code[n].type,
			handle->reloc_table_code[n].external_functions_index,
			handle->reloc_table_code[n].section_name,
			handle->reloc_table_code[n].symbol_name);
	}
			
	for (l = 0; l < 100; l++) {
		if ((external_entry_points[l].valid != 0) &&
			(external_entry_points[l].type == 1)) {
			struct process_state_s *process_state;
			
			process_state = &external_entry_points[l].process_state;
			memory_text = process_state->memory_text;
			memory_stack = process_state->memory_stack;
			memory_reg = process_state->memory_reg;
			memory_data = process_state->memory_data;
			memory_used = process_state->memory_used;
			external_entry_points[l].inst_log = inst_log;
			/* EIP is a parameter for process_block */
			/* Update EIP */
			//memory_reg[2].offset_value = 0;
			//inst_log_prev = 0;
			entry_point[0].used = 1;
			entry_point[0].esp_init_value = memory_reg[0].init_value;
			entry_point[0].esp_offset_value = memory_reg[0].offset_value;
			entry_point[0].ebp_init_value = memory_reg[1].init_value;
			entry_point[0].ebp_offset_value = memory_reg[1].offset_value;
			entry_point[0].eip_init_value = memory_reg[2].init_value;
			entry_point[0].eip_offset_value = memory_reg[2].offset_value;
			entry_point[0].previous_instuction = 0;
			entry_point_list_length = 100;

			print_mem(memory_reg, 1);
			printf ("LOGS: inst_log = 0x%"PRIx64"\n", inst_log);
			do {
				not_finished = 0;
				for (n = 0; n < entry_point_list_length; n++ ) {
					/* EIP is a parameter for process_block */
					/* Update EIP */
					//printf("entry:%d\n",n);
					if (entry_point[n].used) {
						memory_reg[0].init_value = entry_point[n].esp_init_value;
						memory_reg[0].offset_value = entry_point[n].esp_offset_value;
						memory_reg[1].init_value = entry_point[n].ebp_init_value;
						memory_reg[1].offset_value = entry_point[n].ebp_offset_value;
						memory_reg[2].init_value = entry_point[n].eip_init_value;
						memory_reg[2].offset_value = entry_point[n].eip_offset_value;
						inst_log_prev = entry_point[n].previous_instuction;
						not_finished = 1;
						printf ("LOGS: EIPinit = 0x%"PRIx64"\n", memory_reg[2].init_value);
						printf ("LOGS: EIPoffset = 0x%"PRIx64"\n", memory_reg[2].offset_value);
						err = process_block(process_state, handle, inst_log_prev, entry_point_list_length, entry_point, inst_size);
						/* clear the entry after calling process_block */
						entry_point[n].used = 0;
						if (err) {
							printf("process_block failed\n");
							return err;
						}
					}
				}
			} while (not_finished);	
			external_entry_points[l].inst_log_end = inst_log - 1;
			printf ("LOGS: inst_log_end = 0x%"PRIx64"\n", inst_log);
		}
	}
/*
	if (entry_point_list_length > 0) {
		for (n = 0; n < entry_point_list_length; n++ ) {
			printf("eip = 0x%"PRIx64", prev_inst = 0x%"PRIx64"\n",
				entry_point[n].eip_offset_value,
				entry_point[n].previous_instuction);
		}
	}
*/
	//inst_log--;
	printf("Instructions=%"PRId64", entry_point_list_length=%"PRId64"\n",
		inst_log,
		entry_point_list_length);

	/* Correct inst_log to identify how many dis_instructions there have been */
	inst_log--;
	print_dis_instructions();
	if (entry_point_list_length > 0) {
		for (n = 0; n < entry_point_list_length; n++ ) {
			printf("%d, eip = 0x%"PRIx64", prev_inst = 0x%"PRIx64"\n",
				entry_point[n].used,
				entry_point[n].eip_offset_value,
				entry_point[n].previous_instuction);
		}
	}
	/************************************************************
	 * This section deals with correcting SSA for branches/joins.
	 * This bit creates the labels table, ready for the next step.
	 ************************************************************/
	printf("Number of labels = 0x%x\n", local_counter);
	label_redirect = calloc(local_counter, sizeof(struct label_redirect_s));
	labels = calloc(local_counter, sizeof(struct label_s));
	/* n <= inst_log verified to be correct limit */
	for (n = 0; n <= inst_log; n++) {
		struct label_s label;
		uint64_t value_id;

		inst_log1 =  &inst_log_entry[n];
		instruction =  &inst_log1->instruction;
		value_id = inst_log1->value3.value_id;
		switch (instruction->opcode) {
		case MOV:
		case ADD:
		case ADC:
		case MUL:
		case CMP:
			if (value_id > local_counter) {
				printf("SSA Failed at inst_log 0x%x\n", n);
				return 1;
			}
			tmp = log_to_label(instruction->dstA.store,
				instruction->dstA.indirect,
				instruction->dstA.index,
				instruction->dstA.relocated,
				inst_log1->value2.value_scope,
				inst_log1->value2.value_id,
				inst_log1->value2.indirect_offset_value,
				inst_log1->value2.indirect_value_id,
				&label);
			if (value_id > 0) {
				label_redirect[value_id].redirect = value_id;
				labels[value_id].scope = label.scope;
				labels[value_id].type = label.type;
				labels[value_id].value = label.value;
			}

			value_id = inst_log1->value1.value_id;
			if (value_id > local_counter) {
				printf("SSA Failed at inst_log 0x%x\n", n);
				return 1;
			}
			tmp = log_to_label(instruction->srcA.store,
				instruction->srcA.indirect,
				instruction->srcA.index,
				instruction->srcA.relocated,
				inst_log1->value1.value_scope,
				inst_log1->value1.value_id,
				inst_log1->value1.indirect_offset_value,
				inst_log1->value1.indirect_value_id,
				&label);
			if (value_id > 0) {
				label_redirect[value_id].redirect = value_id;
				labels[value_id].scope = label.scope;
				labels[value_id].type = label.type;
				labels[value_id].value = label.value;
			}
			break;
		case CALL:
			printf("SSA CALL inst_log 0x%x\n", n);
			if (value_id > local_counter) {
				printf("SSA Failed at inst_log 0x%x\n", n);
				return 1;
			}
			tmp = log_to_label(instruction->dstA.store,
				instruction->dstA.indirect,
				instruction->dstA.index,
				instruction->dstA.relocated,
				inst_log1->value2.value_scope,
				inst_log1->value2.value_id,
				inst_log1->value2.indirect_offset_value,
				inst_log1->value2.indirect_value_id,
				&label);
			if (value_id > 0) {
				label_redirect[value_id].redirect = value_id;
				labels[value_id].scope = label.scope;
				labels[value_id].type = label.type;
				labels[value_id].value = label.value;
			}
			break;
		default:
			break;
		}
	}
	for (n = 0; n < local_counter; n++) {
		printf("labels 0x%x: redirect=0x%"PRIx64", scope=0x%"PRIx64", type=0x%"PRIx64", value=0x%"PRIx64"\n",
			n, label_redirect[n].redirect, labels[n].scope, labels[n].type, labels[n].value);
	}
	
	/************************************************************
	 * This section deals with correcting SSA for branches/joins.
	 * It build bi-directional links to instruction operands.
	 * This section does work for local_reg case.
	 ************************************************************/
	for (n = 0; n < inst_log; n++) {
		struct label_s label;
		uint64_t value_id1;
		uint64_t value_id2;

		inst_log1 =  &inst_log_entry[n];
		instruction =  &inst_log1->instruction;
		value_id1 = inst_log1->value1.value_id;
		value_id2 = inst_log1->value2.value_id;
		switch (instruction->opcode) {
		case MOV:
		case ADD:
		case ADC:
		case MUL:
		case CMP:
		default:
			break;
		/* FIXME: TODO */
		}
	}
	
	/************************************************************
	 * This section deals with correcting SSA for branches/joins.
	 * It build bi-directional links to instruction operands.
	 * This section does work for local_stack case.
	 ************************************************************/
	for (n = 0; n < inst_log; n++) {
		struct label_s label;
		uint64_t value_id;
		uint64_t value_id1;
		uint64_t value_id2;
		uint64_t size;
		uint64_t *inst_list;

		size = 0;
		inst_log1 =  &inst_log_entry[n];
		instruction =  &inst_log1->instruction;
		value_id1 = inst_log1->value1.value_id;
		value_id2 = inst_log1->value2.value_id;
		
		if (value_id1 > local_counter) {
			printf("SSA Failed at inst_log 0x%x\n", n);
			return 1;
		}
		if (value_id2 > local_counter) {
			printf("SSA Failed at inst_log 0x%x\n", n);
			return 1;
		}
		switch (instruction->opcode) {
		case MOV:
		case ADD:
		case ADC:
		case MUL:
		case CMP:
			value_id = label_redirect[value_id1].redirect;
			if ((1 == labels[value_id].scope) &&
				(2 == labels[value_id].type)) {
				printf("Found local_stack Inst:0x%x:value_id:0x%"PRIx64"\n", n, value_id1);
				tmp = search_back_local_stack(n, inst_log1->value1.indirect_init_value, inst_log1->value1.indirect_offset_value, &size, &inst_list);
				if (tmp) {
					printf("SSA search_back Failed at inst_log 0x%x\n", n);
					return 1;
				}
				printf("SSA inst:0x%x:size=0x%"PRIx64"\n", n, size);
				/* FIXME: This if statement is really doing to much */
				if (size > 0) {
					uint64_t value_id_highest = value_id;
					inst_log1->value1.prev = calloc(size, sizeof(inst_log1->value1.prev));
					inst_log1->value1.prev_size = size;
					for (l = 0; l < size; l++) {
						struct inst_log_entry_s *inst_log_l;
						inst_log_l = &inst_log_entry[inst_list[l]];
						inst_log1->value1.prev[l] = inst_list[l];
						inst_log_l->value3.next = realloc(inst_log_l->value3.next, (inst_log_l->value3.next_size + 1) * sizeof(inst_log_l->value3.next));
						inst_log_l->value3.next[inst_log_l->value3.next_size] =
							 inst_list[l];
						inst_log_l->value3.next_size++;
						if (label_redirect[inst_log_l->value3.value_id].redirect > value_id_highest) {
							value_id_highest = label_redirect[inst_log_l->value3.value_id].redirect;
						}
						printf("rel inst:0x%"PRIx64"\n", inst_list[l]);
					}
					/* Renaming is only needed if there are more than one label present */
					if (size > 0) {
						printf("Renaming label 0x%"PRIx64" to 0x%"PRIx64"\n",
							label_redirect[value_id1].redirect,
							value_id_highest);
						label_redirect[value_id1].redirect =
								value_id_highest;
						for (l = 0; l < size; l++) {
							struct inst_log_entry_s *inst_log_l;
							inst_log_l = &inst_log_entry[inst_list[l]];
							printf("Renaming label 0x%"PRIx64" to 0x%"PRIx64"\n",
								label_redirect[inst_log_l->value3.value_id].redirect,
								value_id_highest);
							label_redirect[inst_log_l->value3.value_id].redirect =
								value_id_highest;
						}
					}
				}
			}
			break;
		default:
			break;
		/* FIXME: TODO */
		}
	}
	

	/***************************************************
	 * This section deals with outputting the .c file.
	 ***************************************************/
	filename = "test.c";
	fd = fopen(filename, "w");
	if (!fd) {
		printf("Failed to open file %s, error=%p\n", filename, fd);
		return 1;
	}
	printf(".c fd=%p\n", fd);
	printf("writing out to file\n");
	/* Output function name */
	printf("\nPRINTING MEMORY_DATA\n");
	for (n = 0; n < 4; n++) {
		if (memory_data[n].valid) {
			
			tmp = relocated_data(handle, memory_data[n].start_address, 4);
			if (tmp) {
				printf("int *data%04"PRIx64" = &data%04"PRIx64"\n",
					memory_data[n].start_address,
					memory_data[n].init_value);
				tmp = fprintf(fd, "int *data%04"PRIx64" = &data%04"PRIx64";\n",
					memory_data[n].start_address,
					memory_data[n].init_value);
			} else {
				printf("int data%04"PRIx64" = 0x%04"PRIx64"\n",
					memory_data[n].start_address,
					memory_data[n].init_value);
				tmp = fprintf(fd, "int data%04"PRIx64" = 0x%"PRIx64";\n",
					memory_data[n].start_address,
					memory_data[n].init_value);
			}
		}
	}
	tmp = fprintf(fd, "\n");
	printf("\n");

	for (n = 0; n < 100; n++) {
		param_present[n] = 0;
	}
		
	for (n = 0; n < 10; n++) {
		if (memory_stack[n].start_address > 0x10000) {
			uint64_t present_index;
			present_index = memory_stack[n].start_address - 0x10000;
			if (present_index >= 100) {
				printf("param limit reached:memory_stack[%d].start_address == 0x%"PRIx64"\n",
					n, memory_stack[n].start_address);
				continue;
			}
			param_present[present_index] = 1;
			param_size[present_index] = memory_stack[n].length;
		}
	}
	for (n = 0; n < 100; n++) {
		if (param_present[n]) {
			printf("param%04x\n", n);
			tmp = param_size[n];
			n += tmp;
		}
	}

	for (l = 0; l < 100; l++) {
		/* FIXME: value == 0 for the first function in the .o file. */
		/*        We need to be able to handle more than
		          one function per .o file. */
		printf("%d:%s:start=%"PRIu64", end=%"PRIu64"\n", l,
				external_entry_points[l].name,
				external_entry_points[l].inst_log,
				external_entry_points[l].inst_log_end);
		if (external_entry_points[l].valid &&
			external_entry_points[l].type == 1) {
			struct process_state_s *process_state;
			
			process_state = &external_entry_points[l].process_state;

			output_function_name(fd, &external_entry_points[l], &param_present[0], &param_size[0]);
			output_function_body(process_state,
				fd,
				external_entry_points[l].inst_log,
				external_entry_points[l].inst_log_end,
				label_redirect,
				labels);
			for (n = external_entry_points[l].inst_log; n <= external_entry_points[l].inst_log_end; n++) {
			}			
		}
	}

	fclose(fd);
	bf_test_close_file(handle);
	print_mem(memory_reg, 1);
	for (n = 0; n < inst_size; n++) {
		printf("0x%04x: %d\n", n, memory_used[n]);
	}
	printf("\nPRINTING MEMORY_DATA\n");
	for (n = 0; n < 4; n++) {
		print_mem(memory_data, n);
		printf("\n");
	}
	printf("\nPRINTING STACK_DATA\n");
	for (n = 0; n < 10; n++) {
		print_mem(memory_stack, n);
		printf("\n");
	}
	for (n = 0; n < 100; n++) {
		param_present[n] = 0;
	}
		
	for (n = 0; n < 10; n++) {
		if (memory_stack[n].start_address >= tmp) {
			uint64_t present_index;
			present_index = memory_stack[n].start_address - 0x10000;
			if (present_index >= 100) {
				printf("param limit reached:memory_stack[%d].start_address == 0x%"PRIx64"\n",
					n, memory_stack[n].start_address);
				continue;
			}
			param_present[present_index] = 1;
			param_size[present_index] = memory_stack[n].length;
		}
	}

	for (n = 0; n < 100; n++) {
		if (param_present[n]) {
			printf("param%04x\n", n);
			tmp = param_size[n];
			n += tmp;
		}
	}
	
	return 0;
}

