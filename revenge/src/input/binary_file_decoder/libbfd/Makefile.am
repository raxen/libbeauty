include $(top_srcdir)/misc/Makefile.common

#AM_CPPFLAGS = -I$(top_srcdir)/src/input

#modulesdir = $(pkglibdir)/binary_file_decoder
lib_LTLIBRARIES = libinput_bfd.la

include_HEADERS = \
	bfl.h

#noinst_HEADERS = \
#	bfl.h

libinput_bfd_la_SOURCES = \
	bfl.c

libinput_bfd_la_LIBADD = \
	-lbfd

libinput_bfd_la_LDFLAGS = \
	 -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)
