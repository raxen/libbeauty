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

#ifndef __BFL__
#define __BFL__

const char *bfd_err(void);

struct rev_eng {
	bfd		*bfd;		/* libbfd structure */
	asection	**section;	/* sections */
	long		section_sz;
	asymbol		**symtab;	/* symbols (sorted) */
	long		symtab_sz;
	asymbol		**dynsymtab; 	/* dynamic symbols (sorted) */
	long		dynsymtab_sz;
	arelent		**dynreloc;	/* dynamic relocations (sorted) */
	long		dynreloc_sz;
};

struct rev_eng *bf_test_open_file(const char *fn);
void bf_test_close_file(struct rev_eng *r);

#endif /* __BFL__ */
