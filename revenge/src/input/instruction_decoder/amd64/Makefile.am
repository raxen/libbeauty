include $(top_srcdir)/misc/Makefile.common

#AM_CPPFLAGS = -I$(top_srcdir)/src/input
AM_CFLAGS = -O0 -g3 -Wall
#CFLAGS = -O0 -g3 -Wall

#modulesdir = $(pkglibdir)/instruction_decoder
lib_LTLIBRARIES = libamd64_decoder.la
#bin_PROGRAMS = dis

#noinst_HEADERS = \
#	opcodes.h

libamd64_decoder_la_SOURCES = \
	opcodes.c dis.c prefix_0f.c

libamd64_decoder_la_LDFLAGS = \
	 -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)
