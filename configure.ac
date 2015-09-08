#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([metabot], [0.1], [tim@metaverse.systems])
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_SRCDIR([include/metabot.h])
AC_CONFIG_HEADER(config.h)

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for libraries.
AC_CHECK_HEADER([winsock2.h],
    [AC_DEFINE([HAVE_LIBWS2_32], [1], [Define if libws2_32 exists.])
     LIBS="-lws2_32 -lgdi32 -lwldap32 -lz $LIBS"],
    [],
    [#include <windows.h>])

AC_CHECK_HEADER([json-c/json.h],
    [AC_DEFINE([HAVE_LIBJSON_C], [1], [Define if libjson-c exists.])
     LIBS="-ljson-c $LIBS"],
    [],
    [#include <json-c/json.h>])

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netdb.h netinet/in.h stdint.h string.h sys/socket.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_UINT64_T

# Checks for library functions.
AC_CHECK_FUNCS([gethostbyname inet_ntoa memset socket sqrt strstr])

AC_CHECK_PROGS([DOXYGEN], [doxygen])
if test -z "$DOXYGEN";
   then AC_MSG_WARN([Doxygen not found - continuing without Doxygen support])
fi

AM_CONDITIONAL([HAVE_DOXYGEN], 
[test -n "$DOXYGEN"])AM_COND_IF([HAVE_DOXYGEN], [AC_CONFIG_FILES([Doxyfile])])

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT
