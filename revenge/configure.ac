AC_INIT
AM_INIT_AUTOMAKE("revenge",0.1.0)
AC_PREREQ(2.53)
dnl ------------------------------------
dnl Configure setup
dnl ------------------------------------

AM_CONFIG_HEADER(src/config.h)

AC_PROG_MAKE_SET
AC_PROG_INSTALL
AC_PROG_CXX
AC_DISABLE_STATIC
AM_PROG_LIBTOOL

dnl ------------------------------------
dnl Flag setup
dnl ------------------------------------

INCLUDES="-I\$(top_srcdir)/src"

CXXFLAGS="-O2 $wall"
DEBUG_CXXFLAGS="-g3 -Wall"

if test "$CXX" = "cxx"; then
   CXXFLAGS="$CXXFLAGS -D__USE_STD_IOSTREAM"
   DEBUG_CXXFLAGS="$DEBUG_CXXFLAGS -D__USE_STD_IOSTREAM"
fi

MODULE_LDFLAGS="-export-dynamic -module -avoid-version"

AC_SUBST(INCLUDES)
AC_SUBST(CXXFLAGS)
AC_SUBST(DEBUG_CXXFLAGS)
AC_SUBST(MODULE_LDFLAGS)

dnl ------------------------------------
dnl       Path setup
dnl ------------------------------------
if test "x$prefix" = xNONE; then
   AC_MSG_CHECKING(prefix)
   prefix="${ac_default_prefix}"
   AC_MSG_RESULT($prefix)
fi
if test "x$exec_prefix" = xNONE; then
   AC_MSG_CHECKING(exec_prefix)
   exec_prefix="${prefix}"
   AC_MSG_RESULT($exec_prefix)
fi

dnl ------------------------------------
dnl    Module install path
dnl ------------------------------------
AC_ARG_WITH(modules-path,[  --with-modules-path     where librevenge modules are installed],modules_path="$withval",modules_path="$libdir/revenge")
ac_modules_path=`eval echo "$modules_path"`
AC_DEFINE_UNQUOTED(MOD_PATH,"$ac_modules_path",[Define the path of the librevenge modules])
MOD_PATH="$ac_modules_path"
AC_SUBST(MOD_PATH)

dnl ------------------------------------
dnl        Makefiles
dnl ------------------------------------
AC_CONFIG_FILES([
Makefile
misc/Makefile
src/Makefile
src/input/Makefile
src/input/binary_file_decoder/Makefile
src/input/binary_file_decoder/elf/Makefile
src/input/instruction_decoder/Makefile
src/input/instruction_decoder/ia32/Makefile
test/Makefile
])
AC_OUTPUT

echo
echo -----------------
echo    RESULTS
echo -----------------
