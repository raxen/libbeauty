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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <bfd.h>
#include <errno.h>

#include "bfl.h"

static void insert_section(struct bfd *b, asection *sect, void *obj)
{
	struct rev_eng *r = obj;
        printf("Section entered\n");
	r->section[r->section_sz++] = sect;
}

static void print_sections(struct rev_eng* ret)
{
  char *comma = "";
  unsigned int       opb = bfd_octets_per_byte (ret->bfd);
  asection          *section = ret->section[0];

  printf ("%3d %-13s %08lx  ", section->index,
          bfd_get_section_name (ret->bfd, section),
          (unsigned long) bfd_section_size (ret->bfd, section) / opb);
  bfd_printf_vma (ret->bfd, bfd_get_section_vma (ret->bfd, section));
  printf ("  ");
  bfd_printf_vma (ret->bfd, section->lma);
  printf ("  %08lx  2**%u", (unsigned long) section->filepos,
          bfd_get_section_alignment (ret->bfd, section));
  printf ("\n                ");
  printf ("  ");

#define PF(x, y) \
  if (section->flags & x) { printf ("%s%s", comma, y); comma = ", "; }

  PF (SEC_HAS_CONTENTS, "CONTENTS");
  PF (SEC_ALLOC, "ALLOC");
  PF (SEC_CONSTRUCTOR, "CONSTRUCTOR");
  PF (SEC_LOAD, "LOAD");
  PF (SEC_RELOC, "RELOC");
  PF (SEC_READONLY, "READONLY");
  PF (SEC_CODE, "CODE");
  PF (SEC_DATA, "DATA");
  PF (SEC_ROM, "ROM");
  PF (SEC_DEBUGGING, "DEBUGGING");
  PF (SEC_NEVER_LOAD, "NEVER_LOAD");
  PF (SEC_EXCLUDE, "EXCLUDE");
  PF (SEC_SORT_ENTRIES, "SORT_ENTRIES");
  PF (SEC_BLOCK, "BLOCK");
  PF (SEC_CLINK, "CLINK");
  PF (SEC_SMALL_DATA, "SMALL_DATA");
  PF (SEC_SHARED, "SHARED");
  PF (SEC_ARCH_BIT_0, "ARCH_BIT_0");
  PF (SEC_THREAD_LOCAL, "THREAD_LOCAL");

  if ((section->flags & SEC_LINK_ONCE) != 0)
    {
      const char *ls;

      switch (section->flags & SEC_LINK_DUPLICATES)
        {
        default:
          abort ();
        case SEC_LINK_DUPLICATES_DISCARD:
          ls = "LINK_ONCE_DISCARD";
          break;
        case SEC_LINK_DUPLICATES_ONE_ONLY:
          ls = "LINK_ONCE_ONE_ONLY";
          break;
        case SEC_LINK_DUPLICATES_SAME_SIZE:
          ls = "LINK_ONCE_SAME_SIZE";
          break;
        case SEC_LINK_DUPLICATES_SAME_CONTENTS:
          ls = "LINK_ONCE_SAME_CONTENTS";
          break;
        }
      printf ("%s%s", comma, ls);

      if (section->comdat != NULL)
        printf (" (COMDAT %s %ld)", section->comdat->name,
                section->comdat->symbol);

      comma = ", ";
    }

  printf ("\n");
#undef PF
}

static void print_code_section(struct rev_eng* ret)
{
  char *comma = "";
  unsigned int       opb = bfd_octets_per_byte (ret->bfd);
  asection          *section = ret->section[0];
  int                n;
  bfd_byte          *data = NULL;
  bfd_size_type      datasize = 0;

  datasize = bfd_get_section_size_before_reloc(section);
  if (datasize == 0)
    return;
  data = malloc(datasize);
  bfd_get_section_contents(ret->bfd, section, data, 0, datasize);
  for(n=0;n<datasize;n++) {
    printf("0x%x ",data[n]);
  }
  printf("\n");
}



const char *bfd_err(void)
{
	return bfd_errmsg(bfd_get_error());
}

struct rev_eng *bf_test_open_file(const char *fn)
{
	struct rev_eng *ret;
	long tmp;
	bfd *b;

        printf("Open entered\n");
	/* Open the file with libbfd */
	b = bfd_openr(fn, NULL);
	if ( b == NULL ) {
		printf("Error opening %s:\n%s",
				fn, bfd_err());
		return NULL;
	}

	/* Check it's an object file and not a core dump, or
	 * archive file or whatever else...
	 */
	if ( !bfd_check_format(b, bfd_object) ) {
		printf("Couldn't determine format of %s:%s\n",
				fn, bfd_err());
		bfd_close(b);
		return NULL;
	}

	/* Create our structure */
	ret = calloc(1, sizeof(*ret));
	if ( ret == NULL ) {
		printf("Couldn't calloc struct rev_eng\n");
		bfd_close(b);
		return NULL;
        }

	ret->bfd = b;

	tmp = bfd_count_sections(ret->bfd);
	if ( tmp <= 0 ) {
          printf("Couldn't count sections\n");
          bfd_close(b);
          return NULL;
        }
	ret->section = calloc(tmp, sizeof(*ret->section));
	if ( ret->section == NULL ) {
          printf("Couldn't calloc struct ret->section\n");
          bfd_close(b);
          return NULL;
        }
	bfd_map_over_sections(ret->bfd, insert_section, ret);
	print_sections(ret);
	print_code_section(ret);
        printf("Setup ok\n");

	return ret;
}

void bf_test_close_file(struct rev_eng *r)
{
	if (!r) return;
	if ( r->symtab )
		free(r->symtab);
	if ( r->dynsymtab )
		free(r->dynsymtab);
	if ( r->dynreloc )
		free(r->dynreloc);
	bfd_close(r->bfd);
	free(r);
}

