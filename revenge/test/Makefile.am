include $(top_srcdir)/misc/Makefile.common

AM_CFLAGS = -I$(top_srcdir)/test

bin_PROGRAMS = dis bf

#noinst_HEADERS = \
#	dis.h \
#	opcodes.h

dis_SOURCES = \
	dis.c

dis_LDADD = -L$(libdir) -linput_bfd -lia32_decoder 

dis_LDFLAGS = @MODULE_LDFLAGS@

bf_SOURCES = \
	bf.c

bf_LDADD = -L$(libdir) -linput_bfd

bf_LDFLAGS = @MODULE_LDFLAGS@
