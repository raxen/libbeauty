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
 *
 *
 * 11-9-2004 Initial work.
 *   Copyright (C) 2004 James Courtier-Dutton James@superbug.co.uk
 * 10-11-2007 Updates.
 *   Copyright (C) 2007 James Courtier-Dutton James@superbug.co.uk
 * 10-10-2009 Updates.
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
#include <dis.h>
#include <bfl.h>
#include <rev.h>
#include <exe.h>
#include <dis-asm.h>


uint64_t read_data(struct self_s *self, uint64_t offset, int size) {
	uint64_t tmp, tmp2, tmp3;
	int n;

	tmp = 0;
	printf("read_data:offset = %"PRIx64"\n", offset);
	for (n = (size - 1); n >= 0; n--) {
		tmp2 = (tmp << 8);
		tmp3 = self->data[n + offset];
		printf("read_data:data = %"PRIx64"\n", tmp3);
		tmp = tmp2 | tmp3;
	}
	printf("read_data:return = %"PRIx64"\n", tmp);
	
	return tmp;
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
		/* FIXME: for now ignore size */
/*		if ((start >= memory_start) &&
			(end <= memory_end)) {
*/
		if (start == memory_start) {
			result = &memory[n];
			printf("Found entry %d in table %p, %p\n", n, memory, result);
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
		/* FIXME: for now ignore size */
/*		if ((start >= memory_start) &&
			(end <= memory_end)) {
*/
		if (start == memory_start) {
			result = NULL;
			/* Store already existed, so exit */
			goto exit_add_new_store;
		}
		n++;
	}
	result = &memory[n];
	printf("Found empty entry %d in table %p, %p\n", n, memory, result);
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
	struct self_s *self,
	struct process_state_s *process_state,
	operand_t *source,
	struct memory_s *destination,
	int info_id )
{
	struct memory_s *value = NULL;
	struct memory_s *value_data = NULL;
	struct memory_s *value_stack = NULL;
	uint64_t data_index;
	char *info;
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

	if (info_id == 0) info = "srcA";
	if (info_id == 1) info = "dstA";
	printf ("get_value_RTL_instruction:%p, %p, %i\n", source, destination, info_id);
	switch (source->indirect) {
	case IND_DIRECT:
		/* Not indirect */
		printf("%s-direct\n", info);
		switch (source->store) {
		case STORE_DIRECT:
			/* i - immediate */
			printf("%s-immediate\n", info);
			printf("%s-relocated=0x%x\n", info, source->relocated);
			printf("index=%"PRIx64", size=%d\n",
					source->index,
					source->value_size);
			destination->start_address = 0;
			destination->length = source->value_size;
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
			destination->value_id = local_counter;
			local_counter++;
			destination->valid = 1;
			printf("value=0x%"PRIx64"+0x%"PRIx64"=0x%"PRIx64"\n",
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
					source->value_size);
			value = search_store(memory_reg,
					source->index,
					source->value_size);
			printf("GET:EXE value=%p\n", value);
			/* FIXME what to do in NULL */
			if (!value) {
				value = add_new_store(memory_reg,
						source->index,
						source->value_size);
				value->value_id = local_counter;
				value->value_scope = 1;
				if (1 == info_id) {
					value->value_scope = 2;
				}
				local_counter++;
			}
			if (!value) {
				printf("GET CASE0:STORE_REG ERROR!\n");
				return 1;
				break;
			}
			destination->start_address = value->start_address;
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
			printf("value=0x%"PRIx64"+0x%"PRIx64"=0x%"PRIx64"\n",
				destination->init_value,
				destination->offset_value,
				destination->init_value +
					destination->offset_value);
			break;
		default:
			/* Should not get here */
			printf("FAILED\n");
			return 1;
		}
		break;
	case IND_MEM:
		/* m - memory */
		printf("%s-indirect\n", info);
		printf("%s-memory\n", info);
		printf("index=%"PRIx64", indirect_size=%d, value_size=%d",
				source->index,
				source->indirect_size,
				source->value_size);
		switch (source->store) {
		case STORE_DIRECT:
			data_index = source->index;
			break;
		case STORE_REG:
			value = search_store(memory_reg,
					source->index,
					source->indirect_size);
			printf("EXE value=%p\n", value);
			/* FIXME what to do in NULL */
			if (!value) {
				value = add_new_store(memory_reg,
						source->index,
						source->indirect_size);
				value->value_id = local_counter;
				local_counter++;
			}
			if (!value) {
				printf("GET CASE2:STORE_REG ERROR!\n");
				return 1;
				break;
			}
			data_index = value->init_value + value->offset_value;
			destination->indirect_value_id = value->value_id;
			break;
		default:
			/* Should not get here */
			printf("FAILED\n");
			return 1;
			break;
		}
		value_data = search_store(memory_data,
				data_index,
				source->value_size);
		printf("EXE2 value_data=%p, %p\n", value_data, &value_data);
		if (!value_data) {
			value_data = add_new_store(memory_data,
				data_index,
				source->value_size);
			value_data->init_value = read_data(self, data_index, 4); 
			printf("EXE3 value_data=%p, %p\n", value_data, &value_data);
			printf("EXE3 value_data->init_value=%"PRIx64"\n", value_data->init_value);
			/* Data */
			value_data->value_scope = 3;
			/* Param number */
			value_data->value_id =
				local_counter;
			local_counter++;
		}
		printf("variable on data:0x%"PRIx64"\n",
			data_index);
		if (!value_data) {
			printf("GET CASE2:STORE_REG2 ERROR!\n");
			return 1;
			break;
		}
		destination->start_address = value_data->start_address;
		destination->length = value_data->length;
		destination->init_value_type = value_data->init_value_type;
		destination->init_value = value_data->init_value;
		destination->offset_value = value_data->offset_value;
		destination->indirect_init_value = value->init_value;
		destination->indirect_offset_value = value->offset_value;
		destination->value_type = value_data->value_type;
		destination->ref_memory =
			value_data->ref_memory;
		destination->ref_log =
			value_data->ref_log;
		destination->value_scope = value_data->value_scope;
		/* counter */
		destination->value_id = value_data->value_id;
		printf("%s: scope=%d, id=%"PRIu64"\n",
			info,
			destination->value_scope,
			destination->value_id);
		/* 1 - Entry Used */
		destination->valid = 1;
		printf("value=0x%"PRIx64"+0x%"PRIx64"=0x%"PRIx64"\n",
			destination->init_value,
			destination->offset_value,
			destination->init_value +
				destination->offset_value);
		break;
	case IND_STACK:
		/* s - stack */
		printf("%s-indirect\n", info);
		printf("%s-stack\n", info);
		printf("index=%"PRIx64", indirect_size=%d, value_size=%d\n",
				source->index,
				source->indirect_size,
				source->value_size);
		value = search_store(memory_reg,
				source->index,
				source->indirect_size);
		printf("EXE value=%p\n", value);
		/* FIXME what to do in NULL */
		if (!value) {
			value = add_new_store(memory_reg,
					source->index,
					source->indirect_size);
			value->value_id = local_counter;
			local_counter++;
		}
		if (!value) {
			printf("GET CASE2:STORE_REG ERROR!\n");
			return 1;
			break;
		}
		value_stack = search_store(memory_stack,
				value->init_value +
					value->offset_value,
					source->value_size);
		printf("EXE2 value_stack=%p, %p\n", value_stack, &value_stack);
		if (!value_stack) {
			value_stack = add_new_store(memory_stack,
				value->init_value +
					value->offset_value,
					source->value_size);
			printf("EXE3 value_stack=%p, %p\n", value_stack, &value_stack);
			/* Only do this init on new stores */
			/* FIXME: 0x10000 should be a global variable */
			/* because it should match the ESP entry value */
			if ((value->init_value +
				value->offset_value) > 0x10000) {
				printf("PARAM\n");
				/* Param */
				value_stack->value_scope = 1;
				/* Param number */
				value_stack->value_id = local_counter;
				local_counter++;
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
		printf("variable on stack:0x%"PRIx64"\n",
			value->init_value + value->offset_value);
		if (!value_stack) {
			printf("GET CASE2:STORE_REG2 ERROR!\n");
			return 1;
			break;
		}
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
		printf("value=0x%"PRIx64"+0x%"PRIx64"=0x%"PRIx64"\n",
			destination->init_value,
			destination->offset_value,
			destination->init_value +
				destination->offset_value);
		break;
	default:
		/* Should not get here */
		printf("FAILED\n");
		return 1;
	}
	return 0;
}

static int put_value_RTL_instruction( 
	struct self_s *self,
	struct process_state_s *process_state,
	struct inst_log_entry_s *inst)
{
	struct instruction_s *instruction;
	struct memory_s *value;
//	struct memory_s *value_mem;
	struct memory_s *value_data;
	struct memory_s *value_stack;
	uint64_t data_index;
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

	/* Put result in dstA */
	instruction = &inst->instruction;
	switch (instruction->dstA.indirect) {
	case IND_DIRECT:
		/* Not indirect */
		printf("dstA-direct\n");
		switch (instruction->dstA.store) {
		case STORE_DIRECT:
			/* i - immediate */
			printf("dstA-immediate-THIS SHOULD NEVER HAPPEN!\n");
			break;
		case STORE_REG:
			/* r - register */
			printf("dstA-register saving result\n");
			value = search_store(memory_reg,
					instruction->dstA.index,
					instruction->dstA.value_size);
			printf("EXE value=%p\n", value);
			/* FIXME what to do in NULL */
			if (!value) {
				printf("WHY!!!!!\n");
				value = add_new_store(memory_reg,
						instruction->dstA.index,
						instruction->dstA.value_size);
			}
			if (!value) {
				printf("PUT CASE0:STORE_REG ERROR!\n");
				return 1;
				break;
			}
			/* eip changing */
			/* Make the constant 0x24 configurable
			 * depending on CPU type.
			 */
			printf("STORE_REG: index=0x%"PRIx64", start_address=0x%"PRIx64"\n",
				instruction->dstA.index, value->start_address);
			if (value->start_address != instruction->dstA.index) {
				printf("STORE failure\n");
				return 1;
				break;
			}
			if (value->start_address == 0x24) {
				printf("A JUMP or RET has occured\n");
			}

			/* FIXME: these should always be the same */
			/* value->length = inst->value3.length; */
			value->start_address = inst->value3.start_address;
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
			printf("value=0x%"PRIx64"+0x%"PRIx64"=0x%"PRIx64"\n",
				value->init_value,
				value->offset_value,
				value->init_value + value->offset_value);
			break;
		default:
			/* Should not get here */
			printf("FAILED\n");
			return 1;
		}
		break;
	case IND_MEM:
		/* m - memory */
		/* FIXME TODO */
		printf("dstA-indirect-NOT\n");
		printf("dstA-memory-NOT\n");
		printf("index=%"PRIx64", value_size=%d\n",
				instruction->dstA.index,
				instruction->dstA.value_size);
		switch (instruction->dstA.store) {
		case STORE_DIRECT:
			data_index = instruction->dstA.index;
			break;
		case STORE_REG:
			value = search_store(memory_reg,
					instruction->dstA.index,
					instruction->dstA.indirect_size);
			printf("EXE value=%p\n", value);
			/* FIXME what to do in NULL */
			if (!value) {
				value = add_new_store(memory_reg,
						instruction->dstA.index,
						instruction->dstA.indirect_size);
				value->value_id = local_counter;
				local_counter++;
			}
			if (!value) {
				printf("GET CASE2:STORE_REG ERROR!\n");
				return 1;
				break;
			}
			if (value->start_address != instruction->dstA.index) {
				printf("STORE failure\n");
				return 1;
				break;
			}
			data_index = value->init_value + value->offset_value;
			break;
		default:
			/* Should not get here */
			printf("FAILED\n");
			return 1;
			break;
		}
		value_data = search_store(memory_data,
				data_index,
				instruction->dstA.value_size);
		printf("EXE2 value_stack=%p\n", value_stack);
		if (!value_data) {
			value_data = add_new_store(memory_data,
				data_index,
				instruction->dstA.value_size);
		}
		if (!value_data) {
			printf("PUT CASE2:STORE_REG2 ERROR!\n");
			return 1;
			break;
		}
		if (value_data->start_address != data_index) {
			printf("STORE DATA failure\n");
			return 1;
			break;
		}
		/* FIXME: these should always be the same */
		/* value_data->length = inst->value3.length; */
		value_data->init_value_type = inst->value3.init_value_type;
		value_data->init_value = inst->value3.init_value;
		value_data->offset_value = inst->value3.offset_value;
		value_data->value_type = inst->value3.value_type;
		value_data->ref_memory =
			inst->value3.ref_memory;
		value_data->ref_log =
			inst->value3.ref_log;
		value_data->value_scope = inst->value3.value_scope;
		/* 1 - Ids */
		value_data->value_id = inst->value3.value_id;
		printf("PUT: scope=%d, id=%"PRIu64"\n",
			value_data->value_scope,
			value_data->value_id);
		/* 1 - Entry Used */
		value_data->valid = 1;
		printf("value_data=0x%"PRIx64"+0x%"PRIx64"=0x%"PRIx64"\n",
			value_data->init_value,
			value_data->offset_value,
			value_data->init_value + value_data->offset_value);
		break;
	case IND_STACK:
		/* s - stack */
		printf("dstA-indirect\n");
		printf("dstA-stack saving result\n");
		printf("index=%"PRIx64", indirect_size=%d\n",
				instruction->dstA.index,
				instruction->dstA.indirect_size);
		value = search_store(memory_reg,
				instruction->dstA.index,
				instruction->dstA.indirect_size);
		printf("EXE value=%p\n", value);
		/* FIXME what to do in NULL */
		if (!value) {
			value = add_new_store(memory_reg,
					instruction->dstA.index,
					instruction->dstA.indirect_size);
		}
		if (!value) {
			printf("PUT CASE2:STORE_REG ERROR!\n");
			return 1;
			break;
		}
		if (value->start_address != instruction->dstA.index) {
			printf("STORE failure\n");
			return 1;
			break;
		}
		value_stack = search_store(memory_stack,
				value->init_value +
					value->offset_value,
					instruction->dstA.value_size);
		printf("EXE2 value_stack=%p\n", value_stack);
		if (!value_stack) {
			value_stack = add_new_store(memory_stack,
				value->init_value +
					value->offset_value,
					instruction->dstA.value_size);
		}
		if (!value_stack) {
			printf("PUT CASE2:STORE_REG2 ERROR!\n");
			return 1;
			break;
		}
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
		printf("PUT: scope=%d, id=%"PRIu64"\n",
			value_stack->value_scope,
			value_stack->value_id);
		/* 1 - Entry Used */
		value_stack->valid = 1;
		printf("value_stack=0x%"PRIx64"+0x%"PRIx64"=0x%"PRIx64"\n",
			value_stack->init_value,
			value_stack->offset_value,
			value_stack->init_value + value_stack->offset_value);
		break;
	default:
		/* Should not get here */
		printf("FAILED\n");
		return 1;
	}
	return 0;
}







int execute_instruction(void *self, struct process_state_s *process_state, struct inst_log_entry_s *inst)
{
	struct instruction_s *instruction;
	struct memory_s *value;
	struct memory_s *memory_text;
	struct memory_s *memory_stack;
	struct memory_s *memory_reg;
	struct memory_s *memory_data;
	int *memory_used;
	struct operand_s operand;
	int16_t tmp16s;
	int32_t tmp32s;
	int64_t tmp64s;
	uint64_t tmp64u;

	memory_text = process_state->memory_text;
	memory_stack = process_state->memory_stack;
	memory_reg = process_state->memory_reg;
	memory_data = process_state->memory_data;
	memory_used = process_state->memory_used;
	int ret;

	instruction = &inst->instruction;
	printf("Execute Instruction %d:%s%s\n",
		instruction->opcode,
		opcode_table[instruction->opcode],
		dis_flags_table[instruction->flags]);

	switch (instruction->opcode) {
	case NOP:
		/* Get value of srcA */
		ret = get_value_RTL_instruction(self, process_state, &(instruction->srcA), &(inst->value1), 0); 
		/* Get value of dstA */
		ret = get_value_RTL_instruction(self, process_state, &(instruction->dstA), &(inst->value2), 0); 
		/* Create result */
		printf("NOP\n");
		//put_value_RTL_instruction(self, process_state, inst);
		break;
	case CMP:
		/* Currently, do the same as NOP */
		/* Get value of srcA */
		ret = get_value_RTL_instruction(self, process_state, &(instruction->srcA), &(inst->value1), 0); 
		/* Get value of dstA */
		ret = get_value_RTL_instruction(self, process_state, &(instruction->dstA), &(inst->value2), 0); 
		/* Create result */
		printf("CMP\n");
		/* A CMP does not save any values */
		//put_value_RTL_instruction(self, inst);
		break;
	case MOV:
		/* Get value of srcA */
		ret = get_value_RTL_instruction(self, process_state, &(instruction->srcA), &(inst->value1), 0); 
		/* Get value of dstA */
		ret = get_value_RTL_instruction(self, process_state, &(instruction->dstA), &(inst->value2), 1); 
		/* Create result */
		printf("MOV\n");
		inst->value3.start_address = inst->value2.start_address;
		inst->value3.length = inst->value1.length;
		inst->value3.init_value_type = inst->value1.init_value_type;
		inst->value3.init_value = inst->value1.init_value;
		inst->value3.offset_value = inst->value1.offset_value;
		inst->value3.value_type = inst->value1.value_type;
		if (inst->instruction.dstA.indirect) {
			inst->value3.indirect_init_value =
				inst->value2.indirect_init_value;
			inst->value3.indirect_offset_value =
				inst->value2.indirect_offset_value;
			inst->value3.indirect_value_id =
				inst->value2.indirect_value_id;
		}
		inst->value3.ref_memory =
			inst->value1.ref_memory;
		inst->value3.ref_log =
			inst->value1.ref_log;
		/* Note: value_scope stays from the dst, not the src. */
		/* FIXME Maybe Exception is the MOV instruction */
		inst->value3.value_scope = inst->value2.value_scope;
		/* MOV param to local */
		/* When the destination is a param_reg,
		 * Change it to a local_reg */
		if ((inst->value3.value_scope == 1) &&
			(STORE_REG == instruction->dstA.store) &&
			(1 == inst->value2.value_scope) &&
			(0 == instruction->dstA.indirect)) {
			inst->value3.value_scope = 2;
		}
		/* Counter */
		//if (inst->value3.value_scope == 2) {
			/* Only value_id preserves the value2 values */
		//inst->value3.value_id = inst->value2.value_id;
		inst->value3.value_id = local_counter;
		inst->value2.value_id = local_counter;
		local_counter++;
		//}
		/* 1 - Entry Used */
		inst->value3.valid = 1;
			printf("value=0x%"PRIx64"+0x%"PRIx64"=0x%"PRIx64"\n",
				inst->value3.init_value,
				inst->value3.offset_value,
				inst->value3.init_value +
					inst->value3.offset_value);
		put_value_RTL_instruction(self, process_state, inst);
		break;
	case SEX:
		/* Get value of srcA */
		ret = get_value_RTL_instruction(self, process_state, &(instruction->srcA), &(inst->value1), 0); 
		/* Get value of dstA */
		ret = get_value_RTL_instruction(self, process_state, &(instruction->dstA), &(inst->value2), 1); 
		/* Create result */
		printf("SEX\n");
		inst->value3.start_address = inst->value2.start_address;
		inst->value3.length = inst->value2.length;
		inst->value3.init_value_type = inst->value1.init_value_type;
		if (8 == inst->value3.length) {
			tmp32s = inst->value1.init_value;
			tmp64s = tmp32s;
			tmp64u = tmp64s;
		} else if (4 == inst->value3.length) {
			tmp16s = inst->value1.init_value;
			tmp32s = tmp16s;
			tmp64u = tmp32s;
		} else {
			printf("SEX length failure\n");
			return 1;
		}
		inst->value3.init_value =tmp64u;
		if (8 == inst->value3.length) {
			tmp32s = inst->value1.offset_value;
			tmp64s = tmp32s;
			tmp64u = tmp64s;
		} else if (4 == inst->value3.length) {
			tmp16s = inst->value1.offset_value;
			tmp32s = tmp16s;
			tmp64u = tmp32s;
		} else {
			printf("SEX length failure\n");
			return 1;
		}
		inst->value3.offset_value = tmp64u;
		inst->value3.value_type = inst->value1.value_type;
		if (inst->instruction.dstA.indirect) {
			inst->value3.indirect_init_value =
				inst->value2.indirect_init_value;
			inst->value3.indirect_offset_value =
				inst->value2.indirect_offset_value;
			inst->value3.indirect_value_id =
				inst->value2.indirect_value_id;
		}
		inst->value3.ref_memory =
			inst->value1.ref_memory;
		inst->value3.ref_log =
			inst->value1.ref_log;
		/* Note: value_scope stays from the dst, not the src. */
		/* FIXME Maybe Exception is the MOV instruction */
		inst->value3.value_scope = inst->value2.value_scope;
		/* MOV param to local */
		/* When the destination is a param_reg,
		 * Change it to a local_reg */
		if ((inst->value3.value_scope == 1) &&
			(STORE_REG == instruction->dstA.store) &&
			(1 == inst->value2.value_scope) &&
			(0 == instruction->dstA.indirect)) {
			inst->value3.value_scope = 2;
		}
		/* Counter */
		//if (inst->value3.value_scope == 2) {
			/* Only value_id preserves the value2 values */
		//inst->value3.value_id = inst->value2.value_id;
		inst->value3.value_id = local_counter;
		inst->value2.value_id = local_counter;
		local_counter++;
		//}
		/* 1 - Entry Used */
		inst->value3.valid = 1;
			printf("value=0x%"PRIx64"+0x%"PRIx64"=0x%"PRIx64"\n",
				inst->value3.init_value,
				inst->value3.offset_value,
				inst->value3.init_value +
					inst->value3.offset_value);
		put_value_RTL_instruction(self, process_state, inst);
		break;
	case ADD:
		/* Get value of srcA */
		ret = get_value_RTL_instruction(self, process_state, &(instruction->srcA), &(inst->value1), 0); 
		/* Get value of dstA */
		ret = get_value_RTL_instruction(self, process_state, &(instruction->dstA), &(inst->value2), 1); 
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
			inst->value3.indirect_value_id =
				inst->value2.indirect_value_id;
		}
		inst->value3.ref_memory =
			inst->value2.ref_memory;
		inst->value3.ref_log =
			inst->value2.ref_log;
		inst->value3.value_scope = inst->value2.value_scope;
		/* Counter */
		inst->value3.value_id = inst->value2.value_id;
		/* 1 - Entry Used */
		inst->value3.valid = 1;
			printf("value=0x%"PRIx64"+0x%"PRIx64"=0x%"PRIx64"\n",
				inst->value3.init_value,
				inst->value3.offset_value,
				inst->value3.init_value +
					inst->value3.offset_value);
		put_value_RTL_instruction(self, process_state, inst);
		break;
	case ADC:
		/* Get value of srcA */
		ret = get_value_RTL_instruction(self, process_state, &(instruction->srcA), &(inst->value1), 0); 
		/* Get value of dstA */
		ret = get_value_RTL_instruction(self, process_state, &(instruction->dstA), &(inst->value2), 1); 
		/* Create result */
		printf("ADC\n");
		put_value_RTL_instruction(self, process_state, inst);
		break;
	case MUL:
		/* Get value of srcA */
		ret = get_value_RTL_instruction(self, process_state, &(instruction->srcA), &(inst->value1), 0); 
		/* Get value of dstA */
		ret = get_value_RTL_instruction(self, process_state, &(instruction->dstA), &(inst->value2), 1); 
		/* Create result */
		printf("MUL\n");
		inst->value3.start_address = inst->value2.start_address;
		inst->value3.length = inst->value2.length;
		inst->value3.init_value_type = inst->value2.init_value_type;
		inst->value3.init_value = inst->value2.init_value;
		inst->value3.offset_value =
			((inst->value1.offset_value + inst->value1.init_value) 
			* (inst->value2.offset_value + inst->value2.init_value))
			 - inst->value2.init_value;
		inst->value3.value_type = inst->value2.value_type;
		if (inst->instruction.dstA.indirect) {
			inst->value3.indirect_init_value =
				inst->value2.indirect_init_value;
			inst->value3.indirect_offset_value =
				inst->value2.indirect_offset_value;
			inst->value3.indirect_value_id =
				inst->value2.indirect_value_id;
		}
		inst->value3.ref_memory =
			inst->value2.ref_memory;
		inst->value3.ref_log =
			inst->value2.ref_log;
		inst->value3.value_scope = inst->value2.value_scope;
		/* Counter */
		inst->value3.value_id = inst->value2.value_id;
		/* 1 - Entry Used */
		inst->value3.valid = 1;
			printf("value=0x%"PRIx64"+0x%"PRIx64"=0x%"PRIx64"\n",
				inst->value3.init_value,
				inst->value3.offset_value,
				inst->value3.init_value +
					inst->value3.offset_value);
		put_value_RTL_instruction(self, process_state, inst);
		break;
	case SUB:
		/* Get value of srcA */
		ret = get_value_RTL_instruction(self, process_state, &(instruction->srcA), &(inst->value1), 0); 
		/* Get value of dstA */
		ret = get_value_RTL_instruction(self, process_state, &(instruction->dstA), &(inst->value2), 1); 
		/* Create result */
		printf("SUB\n");
		inst->value3.start_address = inst->value2.start_address;
		inst->value3.length = inst->value2.length;
		inst->value3.init_value_type = inst->value2.init_value_type;
		inst->value3.init_value = inst->value2.init_value;
		inst->value3.offset_value = inst->value2.offset_value -
			inst->value1.init_value;
		inst->value3.value_type = inst->value2.value_type;
		if (inst->instruction.dstA.indirect) {
			inst->value3.indirect_init_value =
				inst->value2.indirect_init_value;
			inst->value3.indirect_offset_value =
				inst->value2.indirect_offset_value;
			inst->value3.indirect_value_id =
				inst->value2.indirect_value_id;
		}
		inst->value3.ref_memory =
			inst->value2.ref_memory;
		inst->value3.ref_log =
			inst->value2.ref_log;
		inst->value3.value_scope = inst->value2.value_scope;
		/* Counter */
		inst->value3.value_id = inst->value2.value_id;
		/* 1 - Entry Used */
		inst->value3.valid = 1;
			printf("value=0x%"PRIx64"+0x%"PRIx64"=0x%"PRIx64"\n",
				inst->value3.init_value,
				inst->value3.offset_value,
				inst->value3.init_value +
					inst->value3.offset_value);
		put_value_RTL_instruction(self, process_state, inst);
		break;
	case OR:
		/* Get value of srcA */
		ret = get_value_RTL_instruction(self, process_state, &(instruction->srcA), &(inst->value1), 0); 
		/* Get value of dstA */
		ret = get_value_RTL_instruction(self, process_state, &(instruction->dstA), &(inst->value2), 1); 
		/* Create result */
		printf("OR \n");
		inst->value3.start_address = inst->value2.start_address;
		inst->value3.length = inst->value2.length;
		inst->value3.init_value_type = inst->value2.init_value_type;
		inst->value3.init_value = 0;
		inst->value3.offset_value = (inst->value2.offset_value +
			inst->value2.init_value) |
			inst->value1.init_value;
		inst->value3.value_type = inst->value2.value_type;
		if (inst->instruction.dstA.indirect) {
			inst->value3.indirect_init_value =
				inst->value2.indirect_init_value;
			inst->value3.indirect_offset_value =
				inst->value2.indirect_offset_value;
			inst->value3.indirect_value_id =
				inst->value2.indirect_value_id;
		}
		inst->value3.ref_memory =
			inst->value2.ref_memory;
		inst->value3.ref_log =
			inst->value2.ref_log;
		inst->value3.value_scope = inst->value2.value_scope;
		/* Counter */
		inst->value3.value_id = inst->value2.value_id;
		/* 1 - Entry Used */
		inst->value3.valid = 1;
			printf("value=0x%"PRIx64"+0x%"PRIx64"=0x%"PRIx64"\n",
				inst->value3.init_value,
				inst->value3.offset_value,
				inst->value3.init_value +
					inst->value3.offset_value);
		put_value_RTL_instruction(self, process_state, inst);
		break;
	case XOR:
		/* Get value of srcA */
		ret = get_value_RTL_instruction(self, process_state, &(instruction->srcA), &(inst->value1), 0); 
		/* Get value of dstA */
		ret = get_value_RTL_instruction(self, process_state, &(instruction->dstA), &(inst->value2), 1); 
		/* Create result */
		printf("XOR\n");
		inst->value3.start_address = inst->value2.start_address;
		inst->value3.length = inst->value2.length;
		inst->value3.init_value_type = inst->value2.init_value_type;
		inst->value3.init_value = 0;
		inst->value3.offset_value = (inst->value2.offset_value +
			inst->value2.init_value) ^
			inst->value1.init_value;
		inst->value3.value_type = inst->value2.value_type;
		if (inst->instruction.dstA.indirect) {
			inst->value3.indirect_init_value =
				inst->value2.indirect_init_value;
			inst->value3.indirect_offset_value =
				inst->value2.indirect_offset_value;
			inst->value3.indirect_value_id =
				inst->value2.indirect_value_id;
		}
		inst->value3.ref_memory =
			inst->value2.ref_memory;
		inst->value3.ref_log =
			inst->value2.ref_log;
		inst->value3.value_scope = inst->value2.value_scope;
		/* Counter */
		inst->value3.value_id = inst->value2.value_id;
		/* 1 - Entry Used */
		inst->value3.valid = 1;
			printf("value=0x%"PRIx64"+0x%"PRIx64"=0x%"PRIx64"\n",
				inst->value3.init_value,
				inst->value3.offset_value,
				inst->value3.init_value +
					inst->value3.offset_value);
		put_value_RTL_instruction(self, process_state, inst);
		break;
	case NOT:
		/* Get value of srcA */
		ret = get_value_RTL_instruction(self, process_state, &(instruction->srcA), &(inst->value1), 0); 
		/* Get value of dstA */
		ret = get_value_RTL_instruction(self, process_state, &(instruction->dstA), &(inst->value2), 1); 
		/* Create result */
		printf("NOT\n");
		inst->value3.start_address = inst->value2.start_address;
		inst->value3.length = inst->value2.length;
		inst->value3.init_value_type = inst->value2.init_value_type;
		inst->value3.init_value = 0;
		inst->value3.offset_value = !(inst->value2.offset_value +
			inst->value2.init_value);
		inst->value3.value_type = inst->value2.value_type;
		if (inst->instruction.dstA.indirect) {
			inst->value3.indirect_init_value =
				inst->value2.indirect_init_value;
			inst->value3.indirect_offset_value =
				inst->value2.indirect_offset_value;
			inst->value3.indirect_value_id =
				inst->value2.indirect_value_id;
		}
		inst->value3.ref_memory =
			inst->value2.ref_memory;
		inst->value3.ref_log =
			inst->value2.ref_log;
		inst->value3.value_scope = inst->value2.value_scope;
		/* Counter */
		inst->value3.value_id = inst->value2.value_id;
		/* 1 - Entry Used */
		inst->value3.valid = 1;
			printf("value=0x%"PRIx64"+0x%"PRIx64"=0x%"PRIx64"\n",
				inst->value3.init_value,
				inst->value3.offset_value,
				inst->value3.init_value +
					inst->value3.offset_value);
		put_value_RTL_instruction(self, process_state, inst);
		break;
	case SHL:
		/* This is an UNSIGNED operation */
		/* Get value of srcA */
		ret = get_value_RTL_instruction(self, process_state, &(instruction->srcA), &(inst->value1), 0); 
		/* Get value of dstA */
		ret = get_value_RTL_instruction(self, process_state, &(instruction->dstA), &(inst->value2), 1); 
		/* Create result */
		printf("SHL\n");
		inst->value3.start_address = inst->value2.start_address;
		inst->value3.length = inst->value2.length;
		inst->value3.init_value_type = inst->value2.init_value_type;
		inst->value3.init_value = 0;
		inst->value3.offset_value = (inst->value2.offset_value +
			inst->value2.init_value) <<
			inst->value1.init_value;
		inst->value3.value_type = inst->value2.value_type;
		if (inst->instruction.dstA.indirect) {
			inst->value3.indirect_init_value =
				inst->value2.indirect_init_value;
			inst->value3.indirect_offset_value =
				inst->value2.indirect_offset_value;
			inst->value3.indirect_value_id =
				inst->value2.indirect_value_id;
		}
		inst->value3.ref_memory =
			inst->value2.ref_memory;
		inst->value3.ref_log =
			inst->value2.ref_log;
		inst->value3.value_scope = inst->value2.value_scope;
		/* Counter */
		inst->value3.value_id = inst->value2.value_id;
		/* 1 - Entry Used */
		inst->value3.valid = 1;
			printf("value=0x%"PRIx64"+0x%"PRIx64"=0x%"PRIx64"\n",
				inst->value3.init_value,
				inst->value3.offset_value,
				inst->value3.init_value +
					inst->value3.offset_value);
		put_value_RTL_instruction(self, process_state, inst);
		break;
	case SHR:
		/* This is an UNSIGNED operation */
		/* Get value of srcA */
		ret = get_value_RTL_instruction(self, process_state, &(instruction->srcA), &(inst->value1), 0); 
		/* Get value of dstA */
		ret = get_value_RTL_instruction(self, process_state, &(instruction->dstA), &(inst->value2), 1); 
		/* Create result */
		printf("SHL\n");
		inst->value3.start_address = inst->value2.start_address;
		inst->value3.length = inst->value2.length;
		inst->value3.init_value_type = inst->value2.init_value_type;
		inst->value3.init_value = 0;
		inst->value3.offset_value = (inst->value2.offset_value +
			inst->value2.init_value) >>
			inst->value1.init_value;
		inst->value3.value_type = inst->value2.value_type;
		if (inst->instruction.dstA.indirect) {
			inst->value3.indirect_init_value =
				inst->value2.indirect_init_value;
			inst->value3.indirect_offset_value =
				inst->value2.indirect_offset_value;
			inst->value3.indirect_value_id =
				inst->value2.indirect_value_id;
		}
		inst->value3.ref_memory =
			inst->value2.ref_memory;
		inst->value3.ref_log =
			inst->value2.ref_log;
		inst->value3.value_scope = inst->value2.value_scope;
		/* Counter */
		inst->value3.value_id = inst->value2.value_id;
		/* 1 - Entry Used */
		inst->value3.valid = 1;
			printf("value=0x%"PRIx64"+0x%"PRIx64"=0x%"PRIx64"\n",
				inst->value3.init_value,
				inst->value3.offset_value,
				inst->value3.init_value +
					inst->value3.offset_value);
		put_value_RTL_instruction(self, process_state, inst);
		break;
	case IF:
		/* Get value of srcA */
		ret = get_value_RTL_instruction(self, process_state, &(instruction->srcA), &(inst->value1), 0); 
		/* Get value of dstA */
		ret = get_value_RTL_instruction(self, process_state, &(instruction->dstA), &(inst->value2), 1); 
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
		ret = get_value_RTL_instruction(self, process_state, &(instruction->srcA), &(inst->value1), 0); 
		/* Get value of dstA */
		//ret = get_value_RTL_instruction(self,  &(instruction->dstA), &(inst->value2), 1); 
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
		/* update EIP */
		value->offset_value = inst->value3.offset_value;
		break;
	case CALL:
		/* Get value of dstA */
		ret = get_value_RTL_instruction(self, process_state, &(instruction->dstA), &(inst->value2), 1); 
		/* Get the current ESP value so one can convert function params to locals */
		operand.indirect = IND_DIRECT;
		operand.store = STORE_REG;
		operand.index = REG_SP;
		/* Need to find out if the reg is 32bit or 64bit. Use the REG_AX return value size */
		operand.value_size = instruction->dstA.value_size;
		ret = get_value_RTL_instruction(self, process_state, &(operand), &(inst->value1), 1); 
		printf("CALL local_counter = 0x%x\n", local_counter);
		/* FIXME: Currently this is a NOP. */
		inst->value3.start_address = inst->value2.start_address;
		inst->value3.length = inst->value2.length;
		inst->value3.init_value_type = inst->value2.init_value_type;
		inst->value3.init_value = inst->value2.init_value;
		inst->value3.offset_value = inst->value2.offset_value;
		inst->value3.value_type = inst->value2.value_type;
		inst->value3.indirect_init_value =
			inst->value2.indirect_init_value;
		inst->value3.indirect_offset_value =
			inst->value2.indirect_offset_value;
		if (inst->instruction.dstA.indirect) {
			inst->value3.indirect_init_value =
				inst->value2.indirect_init_value;
			inst->value3.indirect_offset_value =
				inst->value2.indirect_offset_value;
			inst->value3.indirect_value_id =
				inst->value2.indirect_value_id;
		}
		inst->value3.ref_memory =
			inst->value2.ref_memory;
		inst->value3.ref_log =
			inst->value2.ref_log;
		inst->value3.value_scope = inst->value2.value_scope;
		/* Counter */
		inst->value3.value_id = local_counter;
		inst->value2.value_id = local_counter;
		local_counter++;
		/* 1 - Entry Used */
		inst->value2.valid = 1;
		inst->value3.valid = 1;
			printf("value=0x%"PRIx64"+0x%"PRIx64"=0x%"PRIx64"\n",
				inst->value3.init_value,
				inst->value3.offset_value,
				inst->value3.init_value +
					inst->value3.offset_value);
		put_value_RTL_instruction(self, process_state, inst);
		break;

	default:
		printf("Unhandled EXE intruction 0x%x\n", instruction->opcode);
		return 1;
		break;
	}
	return 0;
}

