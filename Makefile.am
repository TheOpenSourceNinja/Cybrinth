# Cybrinth created by James Dearing, copyright 2012-2014 and licensed under the GNU AGPL (see the file COPYING)

DEB_BUILD_HARDENING = 1 #If you have hardening wrapper scripts installed (Ubuntu/Debian package "hardening-wrapper"), this should make them harden the program

AM_CPPFLAGS = -std=c++0x -Wpedantic -Wall -Wextra -Wdouble-promotion -Og $(IRRLICHT_CFLAGS) $(BOOSTFILESYSTEM_CFLAGS) $(BOOSTSYSTEM_CFLAGS) $(DEPS_CFLAGS) $(SDLDEPS_CFLAGS) -pg
AM_LDFLAGS = -pg
bin_PROGRAMS = cybrinth

#do-stuff : convert-images cybrinth
#.PHONY : do-stuff

cybrinth_SOURCES = src/AI.h src/AI.cpp src/Collectable.h src/Collectable.cpp src/colors.h src/FontManager.h src/FontManager.cpp src/GameManager.h src/GameManager.cpp src/Goal.h src/Goal.cpp src/GUIFreetypeFont.h src/GUIFreetypeFont.cpp src/ControlMapping.h src/ControlMapping.cpp src/main.cpp src/MazeCell.h src/MazeCell.cpp src/MazeManager.h src/MazeManager.cpp src/MenuOption.h src/MenuOption.cpp src/NetworkManager.h src/NetworkManager.cpp src/Object.h src/Object.cpp src/Player.h src/Player.cpp src/PlayerStart.h src/PlayerStart.cpp src/StringConverter.h src/StringConverter.cpp src/SpellChecker.h src/SpellChecker.cpp src/ImageModifier.h src/ImageModifier.cpp src/SystemSpecificsManager.h src/SystemSpecificsManager.cpp src/PreprocessorCommands.h src/MenuManager.h  src/MenuManager.cpp
# cybrinth_SOURCES = $(wildcard src/*.h src/*.cpp)
cybrinth_SOURCES += compiled-images/key.cpp compiled-images/acid.cpp compiled-images/goal.cpp compiled-images/start.cpp
# cybrinth_SOURCES += $(wildcard compiled-images/*.cpp)
cybrinth_LDADD = $(IRRLICHT_LIBS) $(BOOSTFILESYSTEM_LIBS) $(BOOSTFILESYSTEM_LIBS_TWO) $(BOOSTSYSTEM_LIBS) $(BOOSTSYSTEM_LIBS_TWO) $(DEPS_LIBS) $(SDLDEPS_LIBS)

#LIST_OF_XCFS = ` find ./images -d -iname "*.xcf" -type f -print0 `
#convert-images : $(LIST_OF_XCFS)
#	$(foreach FILE, $(LIST_OF_XCFS), $(shell xcf2png $(FILE) -o $(FILE).png))

# All lines below (in the file Makefile.am) are copied from the Autoconf documentation: https://www.gnu.org/savannah-checkouts/gnu/autoconf/manual/autoconf-2.69/html_node/Automatic-Remaking.html#Automatic-Remaking
#$(srcdir)/configure: configure.ac aclocal.m4
#	cd '$(srcdir)' && autoconf
# autoheader might not change config.h.in, so touch a stamp file.
#$(srcdir)/config.h.in: stamp-h.in
#$(srcdir)/stamp-h.in: configure.ac aclocal.m4
#	cd '$(srcdir)' && autoheader
#	date -uI > '$(srcdir)/stamp-h.in'

#config.h: stamp-h
#stamp-h: config.h.in config.status
#	./config.status

#Makefile: Makefile.in config.status
#	./config.status

#config.status: configure
#	./config.status --recheck
