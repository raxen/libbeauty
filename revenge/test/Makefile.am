include $(top_srcdir)/misc/Makefile.common

AM_CFLAGS = -I$(top_srcdir)/test

bin_PROGRAMS = dis bf

#noinst_HEADERS = \
#	dis.h \
#	opcodes.h

dis_SOURCES = \
	dis.c

dis_LDADD = /usr/local/lib/revenge/libinput_bfd.so /usr/local/lib/revenge/libia32_decoder.so

dis_LDFLAGS = @MODULE_LDFLAGS@

bf_SOURCES = \
	bf.c

bf_LDADD = /usr/local/lib/revenge/libinput_bfd.so

bf_LDFLAGS = @MODULE_LDFLAGS@
