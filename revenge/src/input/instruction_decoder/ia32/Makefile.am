include $(top_srcdir)/misc/Makefile.common

AM_CPPFLAGS = -I$(top_srcdir)/src/input

modulesdir = $(pkglibdir)/instruction_decoder
#modules_LTLIBRARIES = libIA32_Decoder.la
bin_PROGRAMS = dis

noinst_HEADERS = \
	dis.hpp \
	opcodes.hpp

dis_SOURCES = \
	opcodes.cpp dis.cpp prefix_0f.cpp

dis_LDFLAGS = @MODULE_LDFLAGS@
