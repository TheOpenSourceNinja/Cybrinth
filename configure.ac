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
PKG_CHECK_MODULES([DEPS], [freetype2 >= 14.1.8 taglib >= 1.7.2]) # Some libraries provide files for pkg-config
PKG_CHECK_EXISTS([SDL2_mixer >= 2.0.0], [PKG_CHECK_MODULES([SDLDEPS], [SDL2_mixer >= 2.0.0])], [PKG_CHECK_MODULES([SDLDEPS], [SDL_mixer >= 1.2.12])]) # Checks for both SDL2_mixer and SDL_mixer 1.2. At the time of this writing, they're the same as far as API goes.

AC_ARG_WITH([irrlicht-include-path], [AS_HELP_STRING([--with-irrlicht-include-path], [location of the Irrlicht headers, defaults to /usr/include/])], [IRRLICHT_CFLAGS="-I$withval"], [IRRLICHT_CFLAGS='-I/usr/include/'])
AC_SUBST([IRRLICHT_CFLAGS])
AC_CHECK_HEADER([irrlicht/irrlicht.h], [], [AC_MSG_ERROR(You need the Irrlicht graphics library installed)])
AC_ARG_WITH([irrlicht-lib-path], [AS_HELP_STRING([--with-irrlicht-lib-path], [location of the Irrlicht library])], [IRRLICHT_LIBS="-L$withval"], [IRRLICHT_LIBS=""])
AC_SUBST([IRRLICHT_LIBS])
AC_CHECK_LIB([Irrlicht], [createDevice])

AC_ARG_WITH([boostfilesystem-include-path], [AS_HELP_STRING([--with-boostfilesystem-include-path], [location of the Boost Filesystem headers, defaults to /usr/include/])], [BOOSTFILESYSTEM_CFLAGS="-I$withval"], [BOOSTFILESYSTEM_CFLAGS='-I/usr/include/'])
AC_SUBST([BOOSTFILESYSTEM_CFLAGS])
AC_ARG_WITH([boostfilesystem-lib-path], [AS_HELP_STRING([--with-boostfilesystem-lib-path], [location of the Boost Filesystem library])], [BOOSTFILESYSTEM_LIBS="-L$withval"], [BOOSTFILESYSTEM_LIBS=""])
AC_SUBST([BOOSTFILESYSTEM_LIBS])
AC_CHECK_HEADER([boost/filesystem.hpp], [BOOSTFILESYSTEM_LIBS_TWO='-lboost_filesystem'], [AC_MSG_ERROR(You need the Boost Filesystem library installed)]) #Since I can't figure out how to test for a linkable copy of the library, I'm just assuming that anybody who has the header files also has the library.
AC_SUBST([BOOSTFILESYSTEM_LIBS_TWO])
#AC_CHECK_LIB([boost_filesystem], [boost::filesystem::copy])

AC_ARG_WITH([boostsystem-include-path], [AS_HELP_STRING([--with-boostsystem-include-path], [location of the Boost System headers, defaults to /usr/include/])], [BOOSTSYSTEM_CFLAGS="-I$withval"], [BOOSTSYSTEM_CFLAGS='-I/usr/include/'])
AC_SUBST([BOOSTSYSTEM_CFLAGS])
AC_ARG_WITH([boostsystem-lib-path], [AS_HELP_STRING([--with-boostsystem-lib-path], [location of the Boost System library])], [BOOSTSYSTEM_LIBS="-L$withval"], [BOOSTSYSTEM_LIBS=""])
AC_SUBST([BOOSTSYSTEM_LIBS])
AC_CHECK_HEADER([boost/system/config.hpp], [BOOSTSYSTEM_LIBS_TWO='-lboost_system'], [AC_MSG_ERROR(You need the Boost System library installed)]) #Since I can't figure out how to test for a linkable copy of the library, I'm just assuming that anybody who has the header files also has the library.
AC_SUBST([BOOSTSYSTEM_LIBS_TWO])
#AC_CHECK_LIB([boost_system])

AC_CHECK_HEADER([boost/lexical_cast.hpp], [], [AC_MSG_ERROR(You need the Boost Lexical Cast library installed)])

AC_CHECK_HEADER([boost/algorithm/string.hpp], [], [AC_MSG_ERROR(You need the Boost String Algo library installed)])

# Checks for header files.
AC_CHECK_HEADERS([vector iostream string sstream map cassert])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_INTMAX_T

# Checks for library functions.
AC_CHECK_FUNCS([_wgetenv])

# End
AC_OUTPUT
