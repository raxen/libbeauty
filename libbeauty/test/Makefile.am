include $(top_srcdir)/misc/Makefile.common

AM_CFLAGS = -I$(top_srcdir)/test

bin_PROGRAMS = dis32 dis64 bf

#noinst_HEADERS = \
#	dis.h \
#	opcodes.h

dis32_SOURCES = \
	dis32.c

dis32_LDADD = -L$(libdir) -lbeauty_input_bfd -lbeauty_decoder_ia32 -lopcodes -liberty -lbeauty_exe

dis32_LDFLAGS = @MODULE_LDFLAGS@

dis64_SOURCES = \
	dis64.c

dis64_LDADD = -L$(libdir) -lbeauty_input_bfd -lbeauty_decoder_amd64 -lopcodes -liberty -lbeauty_exe

dis64_LDFLAGS = @MODULE_LDFLAGS@

bf_SOURCES = \
	bf.c

bf_LDADD = -L$(libdir) -lbeauty_input_bfd

bf_LDFLAGS = @MODULE_LDFLAGS@
