include $(top_srcdir)/misc/Makefile.common

AM_CFLAGS = -I$(top_srcdir)/test

bin_PROGRAMS = dis

#noinst_HEADERS = \
#	dis.h \
#	opcodes.h

dis_SOURCES = \
	dis.c

dis_LDADD = /usr/local/lib/revenge/libia32_decoder.so

dis_LDFLAGS = @MODULE_LDFLAGS@
