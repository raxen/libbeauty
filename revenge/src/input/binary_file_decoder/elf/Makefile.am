include $(top_srcdir)/misc/Makefile.common

modulesdir = $(pkglibdir)/binary_file_decoder
modules_LTLIBRARIES = libElf.la

bin_PROGRAMS = 	main_test

noinst_HEADERS = \
	Elf_Decoder.hpp \
	Elf_Program_Header_T.hpp \
	Elf_Section_Header_T.hpp \
	Elf_Version_Need.hpp \
	Elf_Dynamic_Header_T.hpp \
	Elf_Reader_T.hpp \
	Elf_Symbol_Info.hpp \
	Elf_Version_Need_Aux.hpp \
	Elf_File_Header_T.hpp \
	Elf_Reader_Factory.hpp \
	Elf_Symbols_T.hpp \
	Elf_Version_Symbol.hpp \
	Elf_Header_T.hpp \
	Elf_Relocation_Add.hpp \
	Elf_Version.hpp \
	Type_Mapper.hpp \
	Elf_Note.hpp \
	Elf_Relocation_T.hpp \
	Elf_Version_Aux.hpp \
	Elf_Exceptions.hpp \
	Elf_Decoder.i \
	Elf_File.hpp \
	Elf_File.i \
	Elf_Header_Eident.hpp \
	Elf_Common.hpp \
	arm.h \
	Elf_HPPA_Common.hpp \
	ia64.hpp \
	m32r.h \
	m68k.h \
	Elf_MIPS_Common.hpp \
	pj.h \
	ppc.h \
	reloc-macros.h \
	Elf_Sparc_Common.hpp \
	vax.h \
	v850.h

EXTRA_DIST = Elf_Program_Header_T.cpp \
	Elf_Dynamic_Header_T.cpp \
	Elf_File_Header_T.cpp \
	Elf_Header_T.cpp \
	Elf_Reader_T.cpp \
	Elf_Relocation_T.cpp \
	Elf_Section_Header_T.cpp \
	Elf_Symbols_T.cpp \
	readelf.c

libElf_la_SOURCES = \
	Elf_Decoder.cpp \
	Elf_Note.cpp \
	Elf_Reader_Factory.cpp \
	Elf_Relocation_Add.cpp \
	Elf_Symbol_Info.cpp \
	Elf_Version.cpp \
	Elf_Version_Aux.cpp \
	Elf_Version_Need.cpp \
	Elf_Version_Need_Aux.cpp \
	Elf_Version_Symbol.cpp \
	Elf_Header_Eident.cpp \
	Elf_File.cpp

AM_CPPFLAGS = -I$(top_srcdir)/src/Module/Input/

libElf_la_LDFLAGS = @MODULE_LDFLAGS@

TEST_PRGS = test_Elf_Note \
	test_Elf_File_Header \
	test_Elf_Version_Symbol

check_PROGRAMS = $(TEST_PRGS)
TESTS = $(TEST_PRGS)

test_LIBS = -L$(top_builddir)/src/Repository \
	-L$(top_builddir)/src/Module/Input/Binary_File_Decoder \
	-L$(top_builddir)/src/Data_Item \
	-lElf -lBinary_File_Decoder -lRepository -lData_Item

test_Elf_Version_Symbol_SOURCES = test_Elf_Version_Symbol.cpp
test_Elf_Version_Symbol_LDADD = $(test_LIBS)

test_Elf_File_Header_SOURCES = test_Elf_File_Header.cpp
test_Elf_File_Header_LDADD = $(test_LIBS)

test_Elf_Note_SOURCES = test_Elf_Note.cpp
test_Elf_Note_LDADD = $(test_LIBS)

main_test_DEPENDENCIES = libElf.la
main_test_SOURCES = main_test.cpp
main_test_LDADD = $(test_LIBS)

clean:
	-rm -f *~

