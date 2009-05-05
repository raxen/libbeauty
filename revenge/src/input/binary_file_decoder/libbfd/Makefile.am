include $(top_srcdir)/misc/Makefile.common

#AM_CPPFLAGS = -I$(top_srcdir)/src/input
#AM_CFLAGS = -O0
#CFLAGS = -O0 -g

#modulesdir = $(pkglibdir)/binary_file_decoder
lib_LTLIBRARIES = libinput_bfd.la

#noinst_HEADERS = \
#	bfl.h

libinput_bfd_la_SOURCES = \
	bfl.c

libinput_bfd_la_LIBADD = -lbfd-single -liberty -lz
#libinput_bfd_la_LIBADD = -lbfd -liberty -lz
libinput_bfd_la_LDFLAGS = \
	 -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)
