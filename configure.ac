#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
# Cybrinth created by James Dearing, copyright 2012-2014 and licensed under the GNU AGPL (see the file COPYING).

AC_PREREQ([2.69])
AC_INIT([Cybrinth], m4_esyscmd_s(date -uI), [dearingj@lifetime.oregonstate.edu], [cybrinth], [http://bettercodes.org/projects/cybrinth])
AC_LANG(C++)
AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_FILES([stamp-h], [date -uI > stamp-h])

# Enable automake to simplify creating makefiles:
AM_INIT_AUTOMAKE([subdir-objects -Wall -Werror])
AC_CONFIG_FILES([Makefile])

# Checks for programs.
AC_PROG_CXX

# Checks for libraries.
PKG_CHECK_MODULES([DEPS], [freetype2 >= 14.1.8 SDL_mixer >= 1.2.12 taglib >= 1.7.2]) # Some libraries provide files for pkg-config
AC_ARG_WITH([irrlicht-include-path], [AS_HELP_STRING([--with-irrlicht-include-path], [location of the Irrlicht headers, defaults to /usr/include/irrlicht])], [IRRLICHT_CFLAGS="-I$withval"], [IRRLICHT_CFLAGS='-I/usr/include/irrlicht'])
AC_SUBST([IRRLICHT_CFLAGS])
AC_ARG_WITH([irrlicht-lib-path], [AS_HELP_STRING([--with-irrlicht-lib-path], [location of the Irrlicht library])], [IRRLICHT_LIBS="-L$withval -lIrrlicht"], [IRRLICHT_LIBS='-lIrrlicht'])
AC_SUBST([IRRLICHT_LIBS])
AC_ARG_WITH([boostfilesystem-include-path], [AS_HELP_STRING([--with-boostfilesystem-include-path], [location of the Boost Filesystem headers, defaults to /usr/include/boost])], [BOOSTFILESYSTEM_CFLAGS="-I$withval"], [BOOSTFILESYSTEM_CFLAGS='-I/usr/include/boost'])
AC_SUBST([BOOSTFILESYSTEM_CFLAGS])
AC_ARG_WITH([boostfilesystem-lib-path], [AS_HELP_STRING([--with-boostfilesystem-lib-path], [location of the Boost Filesystem library])], [BOOSTFILESYSTEM_LIBS="-L$withval -lboost_filesystem"], [BOOSTFILESYSTEM_LIBS='-lboost_filesystem'])
AC_SUBST([BOOSTFILESYSTEM_LIBS])
AC_ARG_WITH([boostsystem-include-path], [AS_HELP_STRING([--with-boostsystem-include-path], [location of the Boost System headers, defaults to /usr/include/boost])], [BOOSTSYSTEM_CFLAGS="-I$withval"], [BOOSTSYSTEM_CFLAGS='-I/usr/include/boost'])
AC_SUBST([BOOSTSYSTEM_CFLAGS])
AC_ARG_WITH([boostsystem-lib-path], [AS_HELP_STRING([--with-boostsystem-lib-path], [location of the Boost System library])], [BOOSTSYSTEM_LIBS="-L$withval -lboost_system"], [BOOSTSYSTEM_LIBS='-lboost_system'])
AC_SUBST([BOOSTSYSTEM_LIBS])

# Checks for header files.
AC_CHECK_HEADERS([vector iostream string sstream map cassert])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_INTMAX_T

# Checks for library functions.
# AC_CHECK_FUNCS([memset pow select socket sqrt strerror])

# End
AC_OUTPUT
