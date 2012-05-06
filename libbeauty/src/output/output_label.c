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

extern int reg_params_order[];

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
			printf("output_label error type=%"PRIx64"\n", label->type);
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
		/* FIXME: local_reg should be handled in case 1.1 above and
		 *        not be a separate label
		 */
		printf("xxxlocal_reg%04"PRIx64";\n", label->value);
		tmp = fprintf(fd, "xxxlocal_reg%04"PRIx64,
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

