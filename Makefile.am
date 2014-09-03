# Cybrinth created by James Dearing, copyright 2012-2014 and licensed under the GNU AGPL (see the file COPYING)

DEB_BUILD_HARDENING = 1 #If you have hardening wrapper scripts installed (Ubuntu/Debian package "hardening-wrapper"), this should make them harden the program

AM_CPPFLAGS = -std=c++0x -Wpedantic -Wall -Wextra $(IRRLICHT_CFLAGS) $(BOOSTFILESYSTEM_CFLAGS) $(BOOSTSYSTEM_CFLAGS) $(DEPS_CFLAGS) $(SDLDEPS_CFLAGS)
AM_LDFLAGS = -lstdc++ -lm
bin_PROGRAMS = cybrinth

#do-stuff : convert-images cybrinth
#.PHONY : do-stuff

cybrinth_SOURCES = src/AI.h src/AI.cpp src/Collectable.h src/Collectable.cpp src/colors.h src/FontManager.h src/FontManager.cpp src/GameManager.h src/GameManager.cpp src/Goal.h src/Goal.cpp src/GUIFreetypeFont.h src/GUIFreetypeFont.cpp src/ControlMapping.h src/ControlMapping.cpp src/main.cpp src/MazeCell.h src/MazeCell.cpp src/MazeManager.h src/MazeManager.cpp src/MenuOption.h src/MenuOption.cpp src/NetworkManager.h src/NetworkManager.cpp src/Object.h src/Object.cpp src/Player.h src/Player.cpp src/PlayerStart.h src/PlayerStart.cpp src/StringConverter.h src/StringConverter.cpp src/SpellChecker.h src/SpellChecker.cpp src/ImageModifier.h src/ImageModifier.cpp src/SystemSpecificsManager.h src/SystemSpecificsManager.cpp src/PreprocessorCommands.h src/MenuManager.h  src/MenuManager.cpp src/FileSelectorDialog.h src/FileSelectorDialog.cpp
# cybrinth_SOURCES = $(wildcard src/*.h src/*.cpp)
cybrinth_SOURCES += compiled-images/key.cpp compiled-images/acid.cpp compiled-images/goal.cpp compiled-images/start.cpp
# cybrinth_SOURCES += $(wildcard compiled-images/*.cpp)
cybrinth_LDADD = $(IRRLICHT_LIBS) $(BOOSTFILESYSTEM_LIBS) $(BOOSTFILESYSTEM_LIBS_TWO) $(BOOSTSYSTEM_LIBS) $(BOOSTSYSTEM_LIBS_TWO) $(DEPS_LIBS) $(SDLDEPS_LIBS)

