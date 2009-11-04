include $(top_srcdir)/misc/Makefile.common

#AM_CPPFLAGS = -I$(top_srcdir)/src/input
AM_CFLAGS = -O0 -g3 -Wall -fPIC
#CFLAGS = -O0 -g3 -Wall

#modulesdir = $(pkglibdir)/binary_file_decoder
lib_LTLIBRARIES = libbeauty_input_bfd.la

#noinst_HEADERS = \
#	bfl.h

libbeauty_input_bfd_la_SOURCES = \
	bfl.c

libbeauty_input_bfd_la_LIBADD = -lbfd -liberty -lz
#libinput_bfd_la_LIBADD = -lbfd -liberty -lz
libbeauty_input_bfd_la_LDFLAGS = \
	 -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)
