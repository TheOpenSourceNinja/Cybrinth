# Cybrinth created by James Dearing, copyright 2012-2014 and licensed under the GNU AGPL (see the file COPYING)

AM_CPPFLAGS = -std=c++0x -pg -lc $(IRRLICHT_CFLAGS) $(BOOSTFILESYSTEM_CFLAGS) $(BOOSTSYSTEM_CFLAGS) $(DEPS_CFLAGS)
AM_LDFLAGS = -pg
bin_PROGRAMS = cybrinth
cybrinth_SOURCES = src/AI.h src/AI.cpp src/Collectable.h src/Collectable.cpp src/colors.h src/FontManager.h src/FontManager.cpp src/GameManager.h src/GameManager.cpp src/Goal.h src/Goal.cpp src/GUIFreetypeFont.h src/GUIFreetypeFont.cpp src/ControlMapping.h src/ControlMapping.cpp src/main.cpp src/MazeCell.h src/MazeCell.cpp src/MazeManager.h src/MazeManager.cpp src/MenuOption.h src/MenuOption.cpp src/NetworkManager.h src/NetworkManager.cpp src/Object.h src/Object.cpp src/Player.h src/Player.cpp src/PlayerStart.h src/PlayerStart.cpp src/StringConverter.h src/StringConverter.cpp src/SpellChecker.h src/SpellChecker.cpp src/ImageModifier.h src/ImageModifier.cpp src/SystemSpecificsManager.h src/SystemSpecificsManager.cpp src/PreprocessorCommands.h compiled-images/key.cpp compiled-images/acid.cpp
cybrinth_LDADD = $(IRRLICHT_LIBS) $(BOOSTFILESYSTEM_LIBS) $(BOOSTSYSTEM_LIBS) $(DEPS_LIBS)

# All lines below (in the file Makefile.am) are copied from the Autoconf documentation: https://www.gnu.org/savannah-checkouts/gnu/autoconf/manual/autoconf-2.69/html_node/Automatic-Remaking.html#Automatic-Remaking
#$(srcdir)/configure: configure.ac aclocal.m4
#	cd '$(srcdir)' && autoconf
# autoheader might not change config.h.in, so touch a stamp file.
$(srcdir)/config.h.in: stamp-h.in
$(srcdir)/stamp-h.in: configure.ac aclocal.m4
	cd '$(srcdir)' && autoheader
	date -uI > '$(srcdir)/stamp-h.in'

config.h: stamp-h
#stamp-h: config.h.in config.status
#	./config.status

#Makefile: Makefile.in config.status
#	./config.status

#config.status: configure
#	./config.status --recheck
