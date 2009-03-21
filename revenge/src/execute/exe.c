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
#include "../input/instruction_decoder/ia32/dis.h"
#include "../input/binary_file_decoder/libbfd/bfl.h"
#include "exe.h"
#include <dis-asm.h>

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

static int get_value_RTL_instruction(
	operand_t *source,
	struct memory_s *destination,
	int info_id )
{
	struct memory_s *value;
	struct memory_s *value_mem;
	struct memory_s *value_stack;
	char *info;
	if (info_id == 0) info = "srcA";
	if (info_id == 1) info = "dstA";
	printf ("get_value_RTL_instruction:%p, %p, %i\n", source, destination, info_id);
	switch (source->indirect) {
	case 0:
		/* Not indirect */
		printf("%s-direct\n", info);
		switch (source->store) {
		case STORE_IMMED:
			/* i - immediate */
			printf("%s-immediate\n", info);
			printf("index=%"PRIx64", size=%d\n",
					source->index,
					source->size);
			destination->start_address = 0;
			destination->length = source->size;
			/* known */
			destination->init_value_type = 1;
			destination->init_value = source->index;
			destination->offset_value = 0;
			/* unknown */
			destination->value_type = 0;
			/* not set yet. */
			destination->ref_memory = 0;
			/* not set yet. */
			destination->ref_log = 0;
			/* unknown */
			destination->value_scope = 0;
			/* 1 - Entry Used */
			destination->value_id = 1;
			destination->valid = 1;
			printf("value=0x%llx+0x%llx=0x%llx\n",
				destination->init_value,
				destination->offset_value,
				destination->init_value +
					 destination->offset_value);
			break;
		case STORE_REG:
			/* r - register */
			printf("%s-register\n", info);
			printf("index=%"PRIx64", size=%d\n",
					source->index,
					source->size);
			value = search_store(memory_reg,
					source->index,
					source->size);
			printf("EXE value=%p\n", value);
			/* FIXME what to do in NULL */
			if (!value) {
				value = add_new_store(memory_reg,
						source->index,
						source->size);
				value->value_id = local_counter;
				value->value_scope = 2;
				local_counter++;
			}
			if (!value)
				break;
			destination->start_address = 0;
			destination->length = value->length;
			destination->init_value_type = value->init_value_type;
			destination->init_value = value->init_value;
			destination->offset_value = value->offset_value;
			destination->value_type = value->value_type;
			destination->ref_memory =
				value->ref_memory;
			destination->ref_log =
				value->ref_log;
			destination->value_scope = value->value_scope;
			/* local counter */
			destination->value_id = value->value_id;
			/* 1 - Entry Used */
			destination->valid = 1;
			printf("value=0x%llx+0x%llx=0x%llx\n",
				destination->init_value,
				destination->offset_value,
				destination->init_value +
					destination->offset_value);
			break;
		case STORE_MEM:
			/* m - memory */
			printf("%s-memory-NOT\n", info);
			break;
		case STORE_STACK:
			/* s - stack */
			printf("%s-stack-NOT\n", info);
			break;
		default:
			/* Should not get here */
			printf("FAILED\n");
			return 1;
		}
		break;
	case 1:
		/* m - memory */
		printf("%s-indirect-NOT\n", info);
		printf("%s-memory-NOT\n", info);
		break;
	case 2:
		/* s - stack */
		printf("%s-indirect\n", info);
		printf("%s-stack\n", info);
		printf("index=%"PRIx64", size=%d\n",
				source->index,
				source->size);
		value = search_store(memory_reg,
				source->index,
				source->size);
		printf("EXE value=%p\n", value);
		/* FIXME what to do in NULL */
		if (!value) {
			value = add_new_store(memory_reg,
					source->index,
					source->size);
			value->value_id = local_counter;
			local_counter++;
		}
		if (!value)
			break;
		value_stack = search_store(memory_stack,
				value->init_value +
					value->offset_value,
					source->size);
		printf("EXE2 value_stack=%p\n", value_stack);
		if (!value_stack) {
			value_stack = add_new_store(memory_stack,
				value->init_value +
					value->offset_value,
					source->size);
			/* Only do this init on new stores */
			if ((value->init_value +
				value->offset_value) > 0x10000) {
				printf("PARAM\n");
				/* Param */
				value_stack->value_scope = 1;
				/* Param number */
				value_stack->value_id = 1;
			} else {
				printf("LOCAL\n");
				/* Local */
				value_stack->value_scope = 2;
				/* Local number */
				value_stack->value_id = local_counter;
				local_counter++;
			}
/* Section ends */
		}
		if (!value_stack)
			break;
		destination->start_address = 0;
		destination->length = value_stack->length;
		destination->init_value_type = value_stack->init_value_type;
		destination->init_value = value_stack->init_value;
		destination->offset_value = value_stack->offset_value;
		destination->indirect_init_value = value->init_value;
		destination->indirect_offset_value = value->offset_value;
		destination->value_type = value_stack->value_type;
		destination->ref_memory =
			value_stack->ref_memory;
		destination->ref_log =
			value_stack->ref_log;
		destination->value_scope = value_stack->value_scope;
		/* counter */
		destination->value_id = value_stack->value_id;
		printf("%s: scope=%d, id=%"PRIu64"\n",
			info,
			destination->value_scope,
			destination->value_id);
		/* 1 - Entry Used */
		destination->valid = 1;
		printf("value=0x%llx+0x%llx=0x%llx\n",
			destination->init_value,
			destination->offset_value,
			destination->init_value +
				destination->offset_value);
		break;
	case 3:
		/* p - ????? */
		printf("%s-indirect-NOT\n", info);
		printf("%s-ppp-NOT\n", info);
		break;
	default:
		/* Should not get here */
		printf("FAILED\n");
		return 1;
	}
	return 0;
}

static int put_value_RTL_instruction(
	struct inst_log_entry_s *inst)
{
	struct instruction_s *instruction;
	struct memory_s *value;
	struct memory_s *value_mem;
	struct memory_s *value_stack;
	/* Put result in dstA */
	instruction = &inst->instruction;
	switch (instruction->dstA.indirect) {
	case 0:
		/* Not indirect */
		printf("dstA-direct\n");
		switch (instruction->dstA.store) {
		case STORE_IMMED:
			/* i - immediate */
			printf("dstA-immediate-THIS SHOULD NEVER HAPPEN!\n");
			break;
		case STORE_REG:
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
			/* eip changing */
			/* Make the constant 0x24 configurable
			 * depending on CPU type.
			 */
			if (value->start_address == 0x24) {
				printf("A JUMP or RET has occured\n");
			}

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
			value->value_id = inst->value3.value_id;
			printf("Saving to reg value_id of %"PRIu64"\n", value->value_id);
			/* 1 - Entry Used */
			value->valid = 1;
			printf("value=0x%llx+0x%llx=0x%llx\n",
				value->init_value,
				value->offset_value,
				value->init_value + value->offset_value);
			break;
		case STORE_MEM:
			/* m - memory */
			printf("dstA-memory saving result - NOT\n");
			break;
		case STORE_STACK:
			/* s - stack */
			printf("dstA-stack saving result - NOT\n");
			break;
		default:
			/* Should not get here */
			printf("FAILED\n");
			return 1;
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
		value_stack->value_id = inst->value3.value_id;
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
		return 1;
	}
	return 0;
}







int execute_instruction(void *self, struct inst_log_entry_s *inst)
{
	struct instruction_s *instruction;
	struct memory_s *value;
	struct memory_s *value_mem;
	struct memory_s *value_stack;
	int ret;

	instruction = &inst->instruction;
	printf("Execute Instruction %d:%s%s\n",
		instruction->opcode,
		opcode_table[instruction->opcode],
		dis_flags_table[instruction->flags]);

	switch (instruction->opcode) {
	case NOP:
		/* Get value of srcA */
		ret = get_value_RTL_instruction( &(instruction->srcA), &(inst->value1), 0); 
		/* Get value of dstA */
		ret = get_value_RTL_instruction( &(instruction->dstA), &(inst->value2), 1); 
		/* Create result */
		printf("NOP\n");
		put_value_RTL_instruction(inst);
		break;
	case CMP:
		/* Currently, do the same as NOP */
		/* Get value of srcA */
		ret = get_value_RTL_instruction( &(instruction->srcA), &(inst->value1), 0); 
		/* Get value of dstA */
		ret = get_value_RTL_instruction( &(instruction->dstA), &(inst->value2), 1); 
		/* Create result */
		printf("CMP\n");
		put_value_RTL_instruction(inst);
		break;
	case MOV:
		/* Get value of srcA */
		ret = get_value_RTL_instruction( &(instruction->srcA), &(inst->value1), 0); 
		/* Get value of dstA */
		ret = get_value_RTL_instruction( &(instruction->dstA), &(inst->value2), 1); 
		/* Create result */
		printf("MOV\n");
		inst->value3.start_address = inst->value1.start_address;
		inst->value3.length = inst->value1.length;
		inst->value3.init_value_type = inst->value1.init_value_type;
		inst->value3.init_value = inst->value1.init_value;
		inst->value3.offset_value = inst->value1.offset_value;
		inst->value3.value_type = inst->value1.value_type;
		if (inst->instruction.dstA.indirect) {
			inst->value3.indirect_init_value =
				inst->value1.indirect_init_value;
			inst->value3.indirect_offset_value =
				inst->value1.indirect_offset_value;
		}
		inst->value3.ref_memory =
			inst->value1.ref_memory;
		inst->value3.ref_log =
			inst->value1.ref_log;
		/* Note: value_scope stays from the dst, not the src. */
		/* FIXME Maybe Exception is the MOV instruction */
		inst->value3.value_scope = inst->value2.value_scope;
		/* MOV param to local */
		/* FIXME: What about mov local -> param */
		//if (inst->value3.value_scope == 1)
		//	inst->value3.value_scope = 2;
		/* Counter */
		//if (inst->value3.value_scope == 2) {
			/* Only value_id preserves the value2 values */
		inst->value3.value_id = inst->value2.value_id;
		//}
		/* 1 - Entry Used */
		inst->value3.valid = 1;
			printf("value=0x%llx+0x%llx=0x%llx\n",
				inst->value3.init_value,
				inst->value3.offset_value,
				inst->value3.init_value +
					inst->value3.offset_value);
		put_value_RTL_instruction(inst);
		break;
	case ADD:
		/* Get value of srcA */
		ret = get_value_RTL_instruction( &(instruction->srcA), &(inst->value1), 0); 
		/* Get value of dstA */
		ret = get_value_RTL_instruction( &(instruction->dstA), &(inst->value2), 1); 
		/* Create result */
		printf("ADD\n");
		inst->value3.start_address = inst->value2.start_address;
		inst->value3.length = inst->value2.length;
		inst->value3.init_value_type = inst->value2.init_value_type;
		inst->value3.init_value = inst->value2.init_value;
		inst->value3.offset_value =
			inst->value2.offset_value + inst->value1.init_value;
		inst->value3.value_type = inst->value2.value_type;
		if (inst->instruction.dstA.indirect) {
			inst->value3.indirect_init_value =
				inst->value2.indirect_init_value;
			inst->value3.indirect_offset_value =
				inst->value2.indirect_offset_value;
		}
		inst->value3.ref_memory =
		inst->value3.ref_memory =
			inst->value2.ref_memory;
		inst->value3.ref_log =
			inst->value2.ref_log;
		inst->value3.value_scope = inst->value2.value_scope;
		/* Counter */
		inst->value3.value_id = inst->value2.value_id;
		/* 1 - Entry Used */
		inst->value3.valid = 1;
			printf("value=0x%llx+0x%llx=0x%llx\n",
				inst->value3.init_value,
				inst->value3.offset_value,
				inst->value3.init_value +
					inst->value3.offset_value);
		put_value_RTL_instruction(inst);
		break;
	case ADC:
		/* Get value of srcA */
		ret = get_value_RTL_instruction( &(instruction->srcA), &(inst->value1), 0); 
		/* Get value of dstA */
		ret = get_value_RTL_instruction( &(instruction->dstA), &(inst->value2), 1); 
		/* Create result */
		printf("ADC\n");
		put_value_RTL_instruction(inst);
		break;
	case SUB:
		/* Get value of srcA */
		ret = get_value_RTL_instruction( &(instruction->srcA), &(inst->value1), 0); 
		/* Get value of dstA */
		ret = get_value_RTL_instruction( &(instruction->dstA), &(inst->value2), 1); 
		/* Create result */
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
		/* Counter */
		inst->value3.value_id = inst->value2.value_id;
		/* 1 - Entry Used */
		inst->value3.valid = 1;
			printf("value=0x%llx+0x%llx=0x%llx\n",
				inst->value3.init_value,
				inst->value3.offset_value,
				inst->value3.init_value +
					inst->value3.offset_value);
		put_value_RTL_instruction(inst);
		break;
	case IF:
		/* Get value of srcA */
		ret = get_value_RTL_instruction( &(instruction->srcA), &(inst->value1), 0); 
		/* Get value of dstA */
		ret = get_value_RTL_instruction( &(instruction->dstA), &(inst->value2), 1); 
		/* Create result */
		printf("IF\n");
		/* Create absolute JMP value in value3 */
		value = search_store(memory_reg,
				REG_IP,
				4);
		inst->value3.start_address = value->start_address;
		inst->value3.length = value->length;
		inst->value3.init_value_type = value->init_value_type;
		inst->value3.init_value = value->init_value;
		inst->value3.offset_value = value->offset_value +
			inst->value2.init_value;
		inst->value3.value_type = value->value_type;
		inst->value3.ref_memory =
			value->ref_memory;
		inst->value3.ref_log =
			value->ref_log;
		inst->value3.value_scope = value->value_scope;
		/* Counter */
		inst->value3.value_id = value->value_id;
		/* 1 - Entry Used */
		inst->value3.valid = 1;
		/* No put_RTL_value is done for an IF */
		break;
	case JMP:
		/* Get value of srcA */
		ret = get_value_RTL_instruction( &(instruction->srcA), &(inst->value1), 0); 
		/* Get value of dstA */
		//ret = get_value_RTL_instruction( &(instruction->dstA), &(inst->value2), 1); 
		/* Create result */
		printf("JMP\n");
		/* Create absolute JMP value in value3 */
		value = search_store(memory_reg,
				REG_IP,
				4);
		printf("JMP 0x%"PRIx64"+%"PRId64"\n",
			value->offset_value, inst->value1.init_value);
		inst->value3.start_address = value->start_address;
		inst->value3.length = value->length;
		inst->value3.init_value_type = value->init_value_type;
		inst->value3.init_value = value->init_value;
		inst->value3.offset_value = value->offset_value +
			inst->value1.init_value;
		inst->value3.value_type = value->value_type;
		inst->value3.ref_memory =
			value->ref_memory;
		inst->value3.ref_log =
			value->ref_log;
		inst->value3.value_scope = value->value_scope;
		/* Counter */
		inst->value3.value_id = value->value_id;
		/* 1 - Entry Used */
		inst->value3.valid = 1;
		/* updaqe EIP */
		value->offset_value = inst->value3.offset_value;
		break;

	default:
		return 1;
		break;
	}
	return 0;
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


