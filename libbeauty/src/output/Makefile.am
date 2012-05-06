include $(top_srcdir)/misc/Makefile.common

#AM_CPPFLAGS = -I$(top_srcdir)/src/input
AM_CFLAGS = -O0 -g3 -Wall
#CFLAGS = -O0 -g3 -Wall

#modulesdir = $(pkglibdir)/beauty_exe
lib_LTLIBRARIES = libbeauty_output.la
#bin_PROGRAMS = dis

#include_HEADERS = \
#	exe.h

#noinst_HEADERS = \
#	exe.h

libbeauty_output_la_SOURCES = \
	print_inst.c output_label.c

libbeauty_output_la_LDFLAGS = \
	 -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)
