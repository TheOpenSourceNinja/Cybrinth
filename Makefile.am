# Cybrinth created by James Dearing, copyright 2012-2013 and licensed under the GNU AGPL (see the file COPYING)

AM_CPPFLAGS = -std=c++0x $(IRRLICHT_CFLAGS) $(BOOSTFILESYSTEM_CFLAGS) $(BOOSTSYSTEM_CFLAGS) $(DEPS_CFLAGS)
bin_PROGRAMS = cybrinth
cybrinth_SOURCES = src/AI.h src/AI.cpp src/Collectable.h src/Collectable.cpp src/colors.h src/font_manager.h src/font_manager.cpp src/GameManager.h src/GameManager.cpp src/Goal.h src/Goal.cpp src/gui_freetype_font.h src/gui_freetype_font.cpp src/KeyMapping.h src/KeyMapping.cpp src/main.cpp src/MazeCell.h src/MazeCell.cpp src/MazeManager.h src/MazeManager.cpp src/MenuOption.h src/MenuOption.cpp src/NetworkManager.h src/NetworkManager.cpp src/Object.h src/Object.cpp src/Player.h src/Player.cpp src/PlayerStart.h src/PlayerStart.cpp src/StringConverter.h src/StringConverter.cpp
cybrinth_LDADD = $(IRRLICHT_LIBS) $(BOOSTFILESYSTEM_LIBS) $(BOOSTSYSTEM_LIBS) $(DEPS_LIBS)
