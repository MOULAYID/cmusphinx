#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([cmuclmtk], [0.99.0], [air+logios@cs.cmu.edu])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/liblmest/disc_meth.h])
AM_CONFIG_HEADER([config.h])
AM_PROG_LIBTOOL

AC_ARG_ENABLE(mingw,
	[  --enable-mingw	  Compile for native Windows execution],
	[LDFLAGS="-mno-cygwin -L/lib/mingw -L/lib/gcc/i686-pc-mingw32/3.4.4 $LDFLAGS"
	 CC='gcc -mno-cygwin'
	]
)

AC_PROG_CC
AC_PROG_INSTALL

AC_CHECK_TOOL(AR,ar)
dnl  libs/mips_swap.h has the endian-related stuff
AC_C_BIGENDIAN
AC_CHECK_LIB(m,log)

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h strings.h sys/time.h])

#AC_CONFIG_SUBDIRS(src/expat)
AC_CONFIG_FILES([Makefile src/Makefile test/Makefile src/liblmest/Makefile src/libs/Makefile src/programs/Makefile])
AC_OUTPUT
