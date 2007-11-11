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

struct memory_s {
	uint64_t start_address; /* Start address of multibyte access. */
	int length;	/* Number of bytes accessed at one time */
	int	 init_value_type;	/* 0 - Unknown, 1 - Known */
	uint64_t init_value;	/* Initial value when first accessed */
	/* init_value + offset_value = absolute value to be used */
	uint64_t offset_value;
	/* 0 - unknown,
	 * 1 - unsigned,
	 * 2 - signed,
	 * 3 - pointer,
	 * 4 - Instruction,
	 * 5 - Instruction pointer(EIP),
	 * 6 - Stack pointer.
	 */
	int	value_type;
	/* last_accessed_from_instruction_at_memory_location */
	uint32_t ref_memory;
	/* last_accessed_from_instruction_log_at_location */
	uint32_t ref_log;
	/* value_scope: 0 - unknown, 1 - Param, 2 - Local, 3 - Global */
	uint64_t value_scope;
	/* Each time a new value is assigned, this value_id increases */
	uint64_t value_id;
	/* valid: 0 - Entry Not used yet, 1 - Entry Used */
	uint64_t valid;
} ;

struct memory_s memory_ram[1000];
struct memory_s memory_reg[100];
struct memory_s memory_stack[100];

struct inst_log_entry_s {
	instruction_t instruction;	/* The instruction */
	struct memory_s value1;		/* First input value */
	struct memory_s value2;		/* Second input value */
	struct memory_s value3;		/* Result */
} ;

struct inst_log_entry_s inst_log_entry[100];

int print_inst(instruction_t *instruction, int instruction_number)
{
	printf("Instruction %d:%s%s",
		instruction_number,
		opcode_table[instruction->opcode],
		dis_flags_table[instruction->flags]);
	if (instruction->srcA.indirect) {
		printf(" %s[%s0x%" PRIx64 "] %s,",
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

struct memory_s *search_store(
	struct memory_s *memory, uint64_t index, int size)
{
	int n = 0;
	uint64_t start = index;
	uint64_t end = index + size;
	uint64_t memory_start;
	uint64_t memory_end;
	struct memory_s *result = NULL;

	printf("memory=%p, index=%"PRIx64", size=%d\n", memory, index, size);
	while (memory[n].valid == 1) {
		printf("looping\n");
		memory_start = memory[n].start_address;
		memory_end = memory[n].start_address + memory[n].length;
		if ((start >= memory_start) &&
			(end <= memory_end)) {
			result = &memory[n];
			break;
		}
		n++;
	}
	return result;
}

struct memory_s *add_new_store(
	struct memory_s *memory, uint64_t index, int size)
{
	int n = 0;
	uint64_t start = index;
	uint64_t end = index + size;
	uint64_t memory_start;
	uint64_t memory_end;
	struct memory_s *result = NULL;

	printf("memory=%p, index=%"PRIx64", size=%d\n", memory, index, size);
	while (memory[n].valid == 1) {
		printf("looping\n");
		memory_start = memory[n].start_address;
		memory_end = memory[n].start_address + memory[n].length;
		if ((start >= memory_start) &&
			(end <= memory_end)) {
			result = NULL;
			/* Store already existed, so exit */
			goto exit_add_new_store;
		}
		n++;
	}
	result = &memory[n];
	printf("Found empty entry %d, %p\n", n, result);
	result->start_address = index;
	result->length = size;
	/* unknown */
	result->init_value_type = 0;
	result->init_value = 0;
	result->offset_value = 0;
	/* unknown */
	result->value_type = 0;
	/* not set yet. */
	result->ref_memory = 0;
	/* not set yet. */
	result->ref_log = 0;
	/* unknown */
	result->value_scope = 0;
	/* Each time a new value is assigned, this value_id increases */
	result->value_id = 1;
	/* 1 - Entry Used */
	result->valid = 1;
exit_add_new_store:
	return result;
}

int execute_instruction(instruction_t *instruction)
{
	struct inst_log_entry_s *inst;
	struct memory_s *value;
	struct memory_s *value_mem;
	struct memory_s *value_stack;

	printf("Execute Instruction 0x%x:%s%s\n",
		instruction->opcode,
		opcode_table[instruction->opcode],
		dis_flags_table[instruction->flags]);

	inst = &inst_log_entry[inst_log];
	inst_log++;
	memcpy(&(inst->instruction), instruction, sizeof(instruction_t));
	/* Get value of srcA */
	/* TODO */
	switch (instruction->srcA.indirect) {
	case 0:
		/* Not indirect */
		printf("srcA-direct\n");
		switch (instruction->srcA.store) {
		case 0:
			/* i - immediate */
			printf("srcA-immediate\n");
			printf("index=%"PRIx64", size=%d\n",
					instruction->srcA.index,
					instruction->srcA.size);
			inst->value1.start_address = 0;
			inst->value1.length = instruction->srcA.size;
			inst->value1.init_value_type = 1; /* known */
			inst->value1.init_value = instruction->srcA.index;
			inst->value1.offset_value = 0;
			inst->value1.value_type = 0; /* unknown */
			inst->value1.ref_memory = 0;  /* not set yet. */
			inst->value1.ref_log = 0; /* not set yet. */
			inst->value1.value_scope = 0; /* unknown */
			inst->value1.value_id = 1; /* 1 - Entry Used */
			inst->value1.valid = 1;
			printf("value=0x%llx+0x%llx=0x%llx\n",
				inst->value1.init_value,
				inst->value1.offset_value,
				inst->value1.init_value +
					 inst->value1.offset_value);
			break;
		case 1:
			/* r - register */
			printf("srcA-register\n");
			printf("index=%"PRIx64", size=%d\n",
					instruction->srcA.index,
					instruction->srcA.size);
			value = search_store(memory_reg,
					instruction->srcA.index,
					instruction->srcA.size);
			printf("EXE value=%p\n", value);
			/* FIXME what to do in NULL */
			if (!value) {
				value = add_new_store(memory_reg,
						instruction->srcA.index,
						instruction->srcA.size);
			}
			if (!value)
				break;
			inst->value1.start_address = 0;
			inst->value1.length = value->length;
			inst->value1.init_value_type = value->init_value_type;
			inst->value1.init_value = value->init_value;
			inst->value1.offset_value = value->offset_value;
			inst->value1.value_type = value->value_type;
			inst->value1.ref_memory =
				value->ref_memory;
			inst->value1.ref_log =
				value->ref_log;
			inst->value1.value_scope = value->value_scope;
			inst->value1.value_id = 1; /* 1 - Entry Used */
			inst->value1.valid = 1;
			printf("value=0x%llx+0x%llx=0x%llx\n",
				inst->value1.init_value,
				inst->value1.offset_value,
				inst->value1.init_value +
					inst->value1.offset_value);
			break;
		case 2:
			/* m - memory */
			break;
		case 3:
			/* s - stack */
			break;
		default:
			/* Should not get here */
			printf("FAILED\n");
			return 0;
		}
		break;
	case 1:
		/* m - memory */
		printf("srcA-indirect\n");
		break;
	case 2:
		/* s - stack */
		printf("srcA-indirect\n");
		printf("srcA-stack\n");
		printf("index=%"PRIx64", size=%d\n",
				instruction->srcA.index,
				instruction->srcA.size);
		value = search_store(memory_reg,
				instruction->srcA.index,
				instruction->srcA.size);
		printf("EXE value=%p\n", value);
		/* FIXME what to do in NULL */
		if (!value) {
			value = add_new_store(memory_reg,
					instruction->srcA.index,
					instruction->srcA.size);
		}
		if (!value)
			break;
		value_stack = search_store(memory_stack,
				value->init_value +
					value->offset_value,
					instruction->srcA.size);
		printf("EXE2 value_stack=%p\n", value_stack);
		if (!value_stack) {
			value_stack = add_new_store(memory_stack,
				value->init_value +
					value->offset_value,
					instruction->srcA.size);
		}
		if (!value_stack)
			break;
		inst->value1.start_address = 0;
		inst->value1.length = value_stack->length;
		inst->value1.init_value_type = value_stack->init_value_type;
		inst->value1.init_value = value_stack->init_value;
		inst->value1.offset_value = value_stack->offset_value;
		inst->value1.value_type = value_stack->value_type;
		inst->value1.ref_memory =
			value_stack->ref_memory;
		inst->value1.ref_log =
			value_stack->ref_log;
		inst->value1.value_scope = value_stack->value_scope;
		inst->value1.value_id = 1; /* 1 - Entry Used */
		inst->value1.valid = 1;
		printf("value=0x%llx+0x%llx=0x%llx\n",
			inst->value1.init_value,
			inst->value1.offset_value,
			inst->value1.init_value +
				inst->value1.offset_value);
		break;
	case 3:
		/* p - ????? */
		break;
	default:
		/* Should not get here */
		printf("FAILED\n");
		return 0;
	}
	/* Get value of dstA */
	/* TODO */
	switch (instruction->dstA.indirect) {
	case 0:
		/* Not indirect */
		printf("dstA-direct\n");
		switch (instruction->dstA.store) {
		case 0:
			/* i - immediate */
			printf("dstA-immediate\n");
			inst->value2.start_address = 0;
			inst->value2.length = instruction->dstA.size;
			inst->value2.init_value_type = 1; /* known */
			inst->value2.init_value = instruction->dstA.index;
			inst->value2.offset_value = 0;
			inst->value2.value_type = 0; /* unknown */
			inst->value2.ref_memory = 0;  /* not set yet. */
			inst->value2.ref_log = 0; /* not set yet. */
			inst->value2.value_scope = 0; /* unknown */
			inst->value2.value_id = 1; /* 1 - Entry Used */
			inst->value2.valid = 1;
			printf("value=0x%llx+0x%llx=0x%llx\n",
				inst->value2.init_value,
				inst->value2.offset_value,
				inst->value2.init_value +
					inst->value2.offset_value);
			break;
		case 1:
			/* r - register */
			printf("dstA-register\n");
			printf("index=%"PRIx64", size=%d\n",
					instruction->dstA.index,
					instruction->dstA.size);
			value = search_store(memory_reg,
					instruction->dstA.index,
					instruction->dstA.size);
			printf("EXE value=%p\n", value);
			/* FIXME what to do in NULL */
			if (!value) {
				value = add_new_store(memory_reg,
						instruction->dstA.index,
						instruction->dstA.size);
			}
			if (!value)
				break;
			inst->value2.start_address = 0;
			inst->value2.length = value->length;
			inst->value2.init_value_type = value->init_value_type;
			inst->value2.init_value = value->init_value;
			inst->value2.offset_value = value->offset_value;
			inst->value2.value_type = value->value_type;
			inst->value2.ref_memory =
				value->ref_memory;
			inst->value2.ref_log =
				value->ref_log;
			inst->value2.value_scope = value->value_scope;
			inst->value2.value_id = 1; /* 1 - Entry Used */
			inst->value2.valid = 1;
			printf("value=0x%llx+0x%llx=0x%llx\n",
				inst->value2.init_value,
				inst->value2.offset_value,
				inst->value2.init_value +
					inst->value2.offset_value);
			break;
		case 2:
			/* m - memory */
			printf("dstA-memory\n");
			break;
		case 3:
			/* s - stack */
			printf("dstA-stack\n");
			break;
		default:
			/* Should not get here */
			printf("FAILED\n");
			return 0;
		}
		break;
	case 1:
		/* m - memory */
		printf("dstA-indirect\n");
		printf("dstA-memory\n");
		break;
	case 2:
		/* s - stack */
		printf("dstA-indirect\n");
		printf("dstA-stack\n");
		printf("index=%"PRIx64", size=%d\n",
				instruction->dstA.index,
				instruction->dstA.size);
		value = search_store(memory_reg,
				instruction->dstA.index,
				instruction->dstA.size);
		printf("EXE value=%p\n", value);
		/* FIXME what to do in NULL */
		if (!value) {
			value = add_new_store(memory_reg,
					instruction->dstA.index,
					instruction->dstA.size);
		}
		if (!value)
			break;
		value_stack = search_store(memory_stack,
				value->init_value +
					value->offset_value,
					instruction->dstA.size);
		printf("EXE2 value_stack=%p\n", value_stack);
		if (!value_stack) {
			value_stack = add_new_store(memory_stack,
				value->init_value +
					value->offset_value,
					instruction->dstA.size);
		}
		if (!value_stack)
			break;
		inst->value2.start_address = 0;
		inst->value2.length = value_stack->length;
		inst->value2.init_value_type = value_stack->init_value_type;
		inst->value2.init_value = value_stack->init_value;
		inst->value2.offset_value = value_stack->offset_value;
		inst->value2.value_type = value_stack->value_type;
		inst->value2.ref_memory =
			value_stack->ref_memory;
		inst->value2.ref_log =
			value_stack->ref_log;
		inst->value2.value_scope = value_stack->value_scope;
		inst->value2.value_id = 1; /* 1 - Entry Used */
		inst->value2.valid = 1;
		printf("value=0x%llx+0x%llx=0x%llx\n",
			inst->value2.init_value,
			inst->value2.offset_value,
			inst->value2.init_value +
				inst->value2.offset_value);
		break;
	case 3:
		/* p - ????? */
		printf("dstA-indirect\n");
		printf("dstA-ppp\n");
		break;
	default:
		/* Should not get here */
		printf("FAILED\n");
		return 0;
	}
	/* Create result */
	/* TODO */
	switch (instruction->opcode) {
	case NOP:
		printf("NOP\n");
		break;
	case MOV:
		printf("MOV\n");
		inst->value3.start_address = inst->value1.start_address;
		inst->value3.length = inst->value1.length;
		inst->value3.init_value_type = inst->value1.init_value_type;
		inst->value3.init_value = inst->value1.init_value;
		inst->value3.offset_value = inst->value1.offset_value;
		inst->value3.value_type = inst->value1.value_type;
		inst->value3.ref_memory =
			inst->value1.ref_memory;
		inst->value3.ref_log =
			inst->value1.ref_log;
		inst->value3.value_scope = inst->value1.value_scope;
		inst->value3.value_id = 1; /* 1 - Entry Used */
		inst->value3.valid = 1;
			printf("value=0x%llx+0x%llx=0x%llx\n",
				inst->value3.init_value,
				inst->value3.offset_value,
				inst->value3.init_value +
					inst->value3.offset_value);
		break;
		break;
	case ADD:
		printf("ADD\n");
		inst->value3.start_address = inst->value2.start_address;
		inst->value3.length = inst->value2.length;
		inst->value3.init_value_type = inst->value2.init_value_type;
		inst->value3.init_value = inst->value2.init_value;
		inst->value3.offset_value =
			inst->value2.offset_value + inst->value1.init_value;
		inst->value3.value_type = inst->value2.value_type;
		inst->value3.ref_memory =
			inst->value2.ref_memory;
		inst->value3.ref_log =
			inst->value2.ref_log;
		inst->value3.value_scope = inst->value2.value_scope;
		inst->value3.value_id = 1; /* 1 - Entry Used */
		inst->value3.valid = 1;
			printf("value=0x%llx+0x%llx=0x%llx\n",
				inst->value3.init_value,
				inst->value3.offset_value,
				inst->value3.init_value +
					inst->value3.offset_value);
		break;
		break;
	case ADC:
		printf("ADC\n");
		break;
	case SUB:
		printf("SUB\n");
		inst->value3.start_address = inst->value2.start_address;
		inst->value3.length = inst->value2.length;
		inst->value3.init_value_type = inst->value2.init_value_type;
		inst->value3.init_value = inst->value2.init_value;
		inst->value3.offset_value = inst->value2.offset_value -
			inst->value1.init_value;
		inst->value3.value_type = inst->value2.value_type;
		inst->value3.ref_memory =
			inst->value2.ref_memory;
		inst->value3.ref_log =
			inst->value2.ref_log;
		inst->value3.value_scope = inst->value2.value_scope;
		inst->value3.value_id = 1;
		/* 1 - Entry Used */
		inst->value3.valid = 1;
			printf("value=0x%llx+0x%llx=0x%llx\n",
				inst->value3.init_value,
				inst->value3.offset_value,
				inst->value3.init_value +
					inst->value3.offset_value);
		break;
	default:
		break;
	}
	/* Put result in dstA */
	/* TODO */
	switch (instruction->dstA.indirect) {
	case 0:
		/* Not indirect */
		printf("dstA-direct\n");
		switch (instruction->dstA.store) {
		case 0:
			/* i - immediate */
			printf("dstA-immediate-THIS SHOULD NEVER HAPPEN!\n");
			break;
		case 1:
			/* r - register */
			printf("dstA-register saving result\n");
			value = search_store(memory_reg,
					instruction->dstA.index,
					instruction->dstA.size);
			printf("EXE value=%p\n", value);
			/* FIXME what to do in NULL */
			if (!value) {
				printf("WHY!!!!!\n");
				value = add_new_store(memory_reg,
						instruction->dstA.index,
						instruction->dstA.size);
			}
			if (!value)
				break;
			/* FIXME: these should always be the same */
			/* value->length = inst->value3.length; */
			value->init_value_type = inst->value3.init_value_type;
			value->init_value = inst->value3.init_value;
			value->offset_value = inst->value3.offset_value;
			value->value_type = inst->value3.value_type;
			value->ref_memory =
				inst->value3.ref_memory;
			value->ref_log =
				inst->value3.ref_log;
			value->value_scope = inst->value3.value_scope;
			/* 1 - Ids */
			value->value_id = 1;
			/* 1 - Entry Used */
			value->valid = 1;
			printf("value=0x%llx+0x%llx=0x%llx\n",
				value->init_value,
				value->offset_value,
				value->init_value + value->offset_value);
			break;
		case 2:
			/* m - memory */
			break;
		case 3:
			/* s - stack */
			break;
		default:
			/* Should not get here */
			printf("FAILED\n");
			return 0;
		}
		break;
	case 1:
		/* m - memory */
		break;
	case 2:
		/* s - stack */
		printf("dstA-indirect\n");
		printf("dstA-stack saving result\n");
		printf("index=%"PRIx64", size=%d\n",
				instruction->dstA.index,
				instruction->dstA.size);
		value = search_store(memory_reg,
				instruction->dstA.index,
				instruction->dstA.size);
		printf("EXE value=%p\n", value);
		/* FIXME what to do in NULL */
		if (!value) {
			value = add_new_store(memory_reg,
					instruction->dstA.index,
					instruction->dstA.size);
		}
		if (!value)
			break;
		value_stack = search_store(memory_stack,
				value->init_value +
					value->offset_value,
					instruction->dstA.size);
		printf("EXE2 value_stack=%p\n", value_stack);
		if (!value_stack) {
			value_stack = add_new_store(memory_stack,
				value->init_value +
					value->offset_value,
					instruction->dstA.size);
		}
		if (!value_stack)
			break;
		/* FIXME: these should always be the same */
		/* value_stack->length = inst->value3.length; */
		value_stack->init_value_type = inst->value3.init_value_type;
		value_stack->init_value = inst->value3.init_value;
		value_stack->offset_value = inst->value3.offset_value;
		value_stack->value_type = inst->value3.value_type;
		value_stack->ref_memory =
			inst->value3.ref_memory;
		value_stack->ref_log =
			inst->value3.ref_log;
		value_stack->value_scope = inst->value3.value_scope;
		/* 1 - Ids */
		value_stack->value_id = 1;
		/* 1 - Entry Used */
		value_stack->valid = 1;
		printf("value_stack=0x%llx+0x%llx=0x%llx\n",
			value_stack->init_value,
			value_stack->offset_value,
			value_stack->init_value + value->offset_value);
		break;
	case 3:
		/* p - ????? */
		break;
	default:
		/* Should not get here */
		printf("FAILED\n");
		return 0;
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
	/* eip on the stack */
	memory_stack[0].start_address = 0x10000;
	/* 4 bytes */
	memory_stack[0].length = 4;
	/* 1 - Known */
	memory_stack[0].init_value_type = 1;
	/* Initial value when first accessed */
	memory_stack[0].init_value = 0x0;
	/* No offset yet */
	memory_stack[0].offset_value = 0;
	/* 0 - unknown,
	 * 1 - unsigned,
	 * 2 - signed,
	 * 3 - pointer,
	 * 4 - Instruction,
	 * 5 - Instruction pointer(EIP),
	 * 6 - Stack pointer.
	 */
	memory_stack[0].value_type = 5;
	memory_stack[0].ref_memory = 0;
	memory_stack[0].ref_log = 0;
	/* value_scope: 0 - unknown, 1 - Param, 2 - Local, 3 - Global */
	memory_stack[0].value_scope = 0;
	/* Each time a new value is assigned, this value_id increases */
	memory_stack[0].value_id = 0;
	/* valid: 0 - Not used yet, 1 - Used */
	memory_stack[0].valid = 1;
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
	ram_init();
	reg_init();
	stack_init();
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
	printf("Instructions=%d\n", inst_log);
	print_instructions();
	printf("test1\n");
	return 0;
}

