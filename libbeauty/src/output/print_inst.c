/*
 *  Copyright (C) 2012  The libbeauty Team
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
 * 06-05-2012 Initial work.
 *   Copyright (C) 2012 James Courtier-Dutton James@superbug.co.uk
 */

#include <stdio.h>
#include <rev.h>

char *dis_flags_table[] = { " ", "f" };

/* RDI, RSI, RDX, RCX, R08, R09  */
int reg_params_order[] = {
        0x40, /* RDI */
        0x38, /* RSI */
        0x18, /* RDX */
        0x10, /* RCX */
        REG_08, /* R08 */
        REG_09 /* R09 */
};


int write_inst(struct self_s *self, FILE *fd, struct instruction_s *instruction, int instruction_number, struct label_s *labels)
{
	int ret = 1; /* Default to failed */
	int tmp;
	int tmp_state = 0;
	int n, m, l;
	struct external_entry_point_s *external_entry_points = self->external_entry_points;
	tmp = fprintf(fd, "// 0x%04x:%s%s",
		instruction_number,
		opcode_table[instruction->opcode],
		dis_flags_table[instruction->flags]);

	switch (instruction->opcode) {
	case MOV:
	case ADD:
	case SUB:
	case SBB:
	case MUL:
	case IMUL:
	case rAND:
	case OR:
	case XOR:
	case SHL:
	case SHR:
	case SAL:
	case SAR:
	case CMP:
	case NOT:
	case NEG:
	case SEX:
	case TEST:
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
		if (instruction->srcA.relocated == 2) {
			for (n = 0; n < EXTERNAL_ENTRY_POINTS_MAX; n++) {
				if ((external_entry_points[n].valid != 0) &&
					(external_entry_points[n].type == 1) &&
					(external_entry_points[n].value == instruction->srcA.index)) {
					instruction->srcA.index = n;
					instruction->srcA.relocated = 1;
					break;
				}
			}
		}
		if ((instruction->srcA.indirect == IND_DIRECT) &&
			(instruction->srcA.relocated == 1)) {
			tmp = fprintf(fd, " CALL2 0x%"PRIx64":%s(",
				instruction->srcA.index,
				external_entry_points[instruction->srcA.index].name);
			tmp_state = 0;
			l = instruction->srcA.index;
			for (n = 0; n < external_entry_points[l].params_size; n++) {
				struct label_s *label;
				label = &labels[external_entry_points[l].params[n]];
				printf("reg_params_order = 0x%x, label->value = 0x%"PRIx64"\n", reg_params_order[m], label->value);
				if ((label->scope == 2) &&
					(label->type == 1)) {
					if (tmp_state > 0) {
						fprintf(fd, ", ");
					}
					fprintf(fd, "int%"PRId64"_t ",
						label->size_bits);
					tmp = output_label(label, fd);
					tmp_state++;
				}
			}
			for (n = 0; n < external_entry_points[l].params_size; n++) {
				struct label_s *label;
				label = &labels[external_entry_points[l].params[n]];
				if ((label->scope == 2) &&
					(label->type == 1)) {
					continue;
				}
				if (tmp_state > 0) {
					fprintf(fd, ", ");
				}
				fprintf(fd, "int%"PRId64"_t ",
					label->size_bits);
				tmp = output_label(label, fd);
				tmp_state++;
			}
			tmp = fprintf(fd, ");\n");
		} else if (instruction->srcA.indirect == IND_MEM) {
			tmp = fprintf(fd, "(*r0x%"PRIx64") ();\n", 
				instruction->srcA.index);
		} else {
			tmp = fprintf(fd, " CALL FAILED index=0x%"PRIx64"\n",
				instruction->srcA.index);
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

int print_inst(struct self_s *self, struct instruction_s *instruction, int instruction_number, struct label_s *labels)
{
	int ret;

	ret = write_inst(self, stdout, instruction, instruction_number, labels);
	return ret;
}

