include $(top_srcdir)/misc/Makefile.common

AM_CPPFLAGS = -I$(top_srcdir)/src/input

modulesdir = $(pkglibdir)/instruction_decoder
#modules_LTLIBRARIES = libIA32_Decoder.la
bin_PROGRAMS = dis

noinst_HEADERS = \
	dis.h \
	opcodes.h

dis_SOURCES = \
	opcodes.c dis.c prefix_0f.c

dis_LDFLAGS = @MODULE_LDFLAGS@
