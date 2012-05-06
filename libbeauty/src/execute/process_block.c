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
#include <string.h>
#if 0
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif
#include <rev.h>
#include <assert.h>

int process_block(struct self_s *self, struct process_state_s *process_state, struct rev_eng *handle, uint64_t inst_log_prev, uint64_t list_length, struct entry_point_s *entry, uint64_t eip_offset_limit) {
	uint64_t offset = 0;
	int result;
	int n, m, l;
	int err;
	int found;
	struct inst_log_entry_s *inst_exe_prev;
	struct inst_log_entry_s *inst_exe;
	struct inst_log_entry_s *inst_log_entry = self->inst_log_entry;
	struct instruction_s *instruction;
	int instruction_offset = 0;
	int octets = 0;
	struct memory_s *memory_text;
	struct memory_s *memory_stack;
	struct memory_s *memory_reg;
	struct memory_s *memory_data;
	struct dis_instructions_s dis_instructions;
	int *memory_used;
	disassembler_ftype disassemble_fn = self->disassemble_fn;

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
			if (inst_exe_prev->next_size > 0) {
				printf("JCD8a: next_size = 0x%x\n", inst_exe_prev->next_size);
			}
			if (inst_exe_prev->next_size == 0) {
				inst_exe_prev->next_size++;
				inst_exe_prev->next = malloc(sizeof(inst_exe_prev->next));
				inst_exe_prev->next[inst_exe_prev->next_size - 1] = inst_this;
			} else {
				found = 0;
				for (l = 0; l < inst_exe_prev->next_size; l++) {
					if (inst_exe_prev->next[inst_exe_prev->next_size - 1] == inst_this) {
						found = 1;
						break;
					}
				}
				if (!found) {
					inst_exe_prev->next_size++;
					inst_exe_prev->next = realloc(inst_exe_prev->next, sizeof(inst_exe_prev->next) * inst_exe_prev->next_size);
					inst_exe_prev->next[inst_exe_prev->next_size - 1] = inst_this;
				}
			}
			break;
		}	
		//printf("disassemble_fn\n");
		//disassemble_fn = disassembler (handle->bfd);
		//printf("disassemble_fn done\n");
		printf("disassemble att  : ");
	        disasm_info.disassembler_options = "att";
		octets = (*disassemble_fn) (offset, &disasm_info);
		printf("  octets=%d\n", octets);
		printf("disassemble intel: ");
	        disasm_info.disassembler_options = "intel";
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
			err = print_inst(self, instruction, instruction_offset + n + 1, NULL);
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
			if (inst_exe_prev->next_size > 1) {
				printf("JCD8b: next_size = 0x%x\n", inst_exe_prev->next_size);
			}
			if (inst_exe_prev->next_size == 1) {
				inst_exe_prev->next = malloc(sizeof(inst_exe_prev->next));
				inst_exe_prev->next[inst_exe_prev->next_size - 1] = inst_log;
			} else {
				inst_exe_prev->next = realloc(inst_exe_prev->next, sizeof(inst_exe_prev->next) * inst_exe_prev->next_size);
				inst_exe_prev->next[inst_exe_prev->next_size - 1] = inst_log;
			}
			inst_exe_prev->next[inst_exe_prev->next_size - 1] = inst_log;

			if (IF == instruction->opcode) {
				printf("IF FOUND\n");
				//printf("Breaking at IF\n");
				printf("IF: this EIP = 0x%"PRIx64"\n",
					memory_reg[2].offset_value);
				printf("IF: jump dst abs EIP = 0x%"PRIx64"\n",
					inst_exe->value3.offset_value);
				printf("IF: inst_log = %"PRId64"\n",
					inst_log);
				for (m = 0; m < list_length; m++ ) {
					if (0 == entry[m].used) {
						entry[m].esp_init_value = memory_reg[0].init_value;
						entry[m].esp_offset_value = memory_reg[0].offset_value;
						entry[m].ebp_init_value = memory_reg[1].init_value;
						entry[m].ebp_offset_value = memory_reg[1].offset_value;
						entry[m].eip_init_value = memory_reg[2].init_value;
						entry[m].eip_offset_value = memory_reg[2].offset_value;
						entry[m].previous_instuction = inst_log;
						entry[m].used = 1;
						printf("JCD:8 used 1\n");
						
						break;
					}
				}
				/* FIXME: Would starting a "m" be better here? */
				for (m = 0; m < list_length; m++ ) {
					if (0 == entry[m].used) {
						entry[m].esp_init_value = memory_reg[0].init_value;
						entry[m].esp_offset_value = memory_reg[0].offset_value;
						entry[m].ebp_init_value = memory_reg[1].init_value;
						entry[m].ebp_offset_value = memory_reg[1].offset_value;
						entry[m].eip_init_value = inst_exe->value3.init_value;
						entry[m].eip_offset_value = inst_exe->value3.offset_value;
						entry[m].previous_instuction = inst_log;
						entry[m].used = 1;
						printf("JCD:8 used 2\n");
						break;
					}
				}
			}
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
#if 0
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
#endif
	}
	return 0;
}

