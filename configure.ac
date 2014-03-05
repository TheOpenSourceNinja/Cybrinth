#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
# Cybrinth created by James Dearing, copyright 2012-2014 and licensed under the GNU AGPL (see the file COPYING).
# This file is based in part on the one found at https://github.com/kmcallister/autoharden/blob/master/configure.ac on 2014-02-15.

AC_PREREQ([2.69])
AC_INIT([Cybrinth], m4_esyscmd_s(date -uIhours), [dearingj@lifetime.oregonstate.edu], [cybrinth], [http://bettercodes.org/projects/cybrinth])
AC_LANG(C++)
AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_AUX_DIR([build-aux])

# Enable automake to simplify creating makefiles:
AM_INIT_AUTOMAKE([subdir-objects -Wall -Werror])
AC_CONFIG_FILES([Makefile])

# Checks for programs.
AC_PROG_CXX

# Hardening stuff. Not that I expect anybody to exploit a simple game like this.
saved_CXX="$CXX"
saved_LD="$LD"

AC_DEFUN([check_cxx_flag],
 [AX_CHECK_COMPILE_FLAG([$1], [$2], [$3], [-Werror $4])])
AC_DEFUN([check_link_flag],
 [AX_CHECK_LINK_FLAG([$1], [$2], [$3], [-Werror $4])])

AC_ARG_ENABLE([hardening],
  [AS_HELP_STRING([--enable-hardening],
    [Enable compiler and linker options to frustrate memory corruption exploits @<:@yes@:>@])],
  [hardening="$enableval"],
  [hardening="yes"])

HARDEN_CFLAGS=""
HARDEN_LDFLAGS=""
AS_IF([test x"$hardening" != x"no"], [
  check_cxx_flag([-fno-strict-overflow], [HARDEN_CFLAGS="$HARDEN_CFLAGS -fno-strict-overflow"])
  
  # This one will likely succeed, even on platforms where it does nothing.
  check_cxx_flag([-D_FORTIFY_SOURCE=2], [HARDEN_CFLAGS="$HARDEN_CFLAGS -D_FORTIFY_SOURCE=2"])
  
  check_cxx_flag([-fstack-protector-all],
   [check_link_flag([-fstack-protector-all],
     [HARDEN_CFLAGS="$HARDEN_CFLAGS -fstack-protector-all"
      check_cxx_flag([-Wstack-protector], [HARDEN_CFLAGS="$HARDEN_CFLAGS -Wstack-protector"],
        [], [-fstack-protector-all])
      check_cxx_flag([--param ssp-buffer-size=1], [HARDEN_CFLAGS="$HARDEN_CFLAGS --param ssp-buffer-size=1"],
        [], [-fstack-protector-all])])])

  # At the link step, we might want -pie (GCC) or -Wl,-pie (Clang on OS X)
  #
  # The linker checks also compile code, so we need to include -fPIE as well.
  check_cxx_flag([-fPIE],
   [check_link_flag([-fPIE -pie],
     [HARDEN_CFLAGS="$HARDEN_CFLAGS -fPIE"
      HARDEN_LDFLAGS="$HARDEN_LDFLAGS -pie"],
     [check_link_flag([-fPIE -Wl,-pie],
       [HARDEN_CFLAGS="$HARDEN_CFLAGS -fPIE"
        HARDEN_LDFLAGS="$HARDEN_LDFLAGS -Wl,-pie"])])])

  check_link_flag([-Wl,-z,relro],
   [HARDEN_LDFLAGS="$HARDEN_LDFLAGS -Wl,-z,relro"
    check_link_flag([-Wl,-z,now], [HARDEN_LDFLAGS="$HARDEN_LDFLAGS -Wl,-z,now"])])])
AC_SUBST([HARDEN_CFLAGS])
AC_SUBST([HARDEN_LDFLAGS])

# End of flag tests.
CXX="$saved_CXX"
LD="$saved_LD"
# End of hardening stuff.

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

# Checks for library functions.
# AC_CHECK_FUNCS([memset pow select socket sqrt strerror])

# End
AC_OUTPUT
