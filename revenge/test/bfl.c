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
	r->section[r->section_sz++] = sect;
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

