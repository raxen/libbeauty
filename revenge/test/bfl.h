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
