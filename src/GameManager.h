/**
 * Copyright © 2013 James Dearing.
 * This file is part of Cybrinth.
 *
 * Cybrinth is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * Cybrinth is distributed in the hope that it will be fun, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License along with Cybrinth. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#define BOOST_FILESYSTEM_NO_DEPRECATED //Recommended by the Boost filesystem library documentation to prevent us from using functions which will be removed in later versions

#include "AI.h"
#include "Collectable.h"
#include "font_manager.h"
#include "Goal.h"
#include "gui_freetype_font.h"
#include "KeyMapping.h"
#include "MazeManager.h"
#include "MenuOption.h"
#include "NetworkManager.h"
#include "Player.h"
#include "PlayerStart.h"
#include "StringConverter.h"

#include <asio.hpp>
#include <irrlicht.h>
#include <SDL_mixer.h>
#include <SDL/SDL.h>
#include <queue>
#include <vector>

using namespace irr;
using namespace std;
using boost::asio::ip::tcp;

class GameManager : public IEventReceiver {
	public:
		//Functions----------------------------------
		void drawAll();

		GameManager();
		virtual ~GameManager();
		bool getDebugStatus();
		MazeManager getMazeManager();
		Goal getGoal();
		Player* getPlayer( uint8_t p );

		void movePlayerOnX( uint8_t p, int8_t direction );
		void movePlayerOnY( uint8_t p, int8_t direction );

		bool OnEvent( const SEvent& );

		void resetThings();
		int run();


		//Non-functions----------------------------------
		vector< AI > bot;

		IrrlichtDevice* device;
		video::IVideoDriver* driver;

		Goal goal;
		gui::IGUIEnvironment* gui;

		uint32_t loadingDelay;

		uint8_t numBots;
		uint8_t numLocks;
		uint8_t numPlayers;

		vector<Player> player;
		vector<PlayerStart> playerStart;

		uint32_t randomSeed;

		StringConverter stringConverter;
		vector<Collectable> stuff;

		ITimer* timer;
		uint32_t timeStartedLoading;

	protected:
	private:
		//Functions----------------------------------
		bool allHumansAtGoal();

		void drawBackground();

		void loadFonts();
		void loadMusicFont();
		void loadNextSong();
		void loadProTips();
		void loadStatsFont();
		void loadTipFont();

		void makeMusicList();

		void readPrefs();

		void setupBackground();
		void startLoadingScreen();

		void takeScreenShot();


		//Booleans----------------------------------
		bool allowSmallSize;
		bool antiAliasFonts;

		bool debug;
		bool donePlaying;

		bool enableJoystick;

		bool fullscreen;

		bool isServer;

		bool markTrails;
		bool musicPlaying;

		bool playMusic;

        bool showBackgrounds;
		bool showingLoadingScreen;
		bool showingMenu;

		bool vsync;

		bool won;


		//unsigned 8-bit integers----------------------------------
		uint8_t backgroundChosen;

		uint8_t myPlayer; //If in client mode, control only one player

		uint8_t numKeysFound;

		uint8_t sideDisplaySizeDenominator;

		vector< uint8_t > winners;


		//unsigned 16-bit integers----------------------------------
		uint16_t musicVolume;


		//unsigned 32-bit integers----------------------------------
		uint32_t bitsPerPixel;

		uint32_t cellWidth;
		uint32_t cellHeight;

		uint32_t joystickChosen;


		//wide character strings----------------------------------
		core::stringw loading;

		core::stringw musicAlbum;
		core::stringw musicArtist;
		core::stringw musicTitle;

		core::stringw proTipPrefix;
		vector< core::stringw > proTips;

		core::stringw stats;
		core::stringw steps;


		//Non-wide character strings----------------------------------
		core::stringc fontFile;


		//Our own types----------------------------------
		MenuOption backToGame;

		MenuOption exitGame;

		FontManager fm;

		vector< KeyMapping > keyMap;

		MenuOption loadMaze;

		MazeManager mazeManager;

		NetworkManager network;
		MenuOption newGame;

		MenuOption saveMaze;


		//2D dimensions----------------------------------
		core::dimension2d<uint32_t> viewportSize;
		core::dimension2d<uint32_t> windowSize;


		//Fonts----------------------------------
		gui::IGUIFont* clockFont;

		gui::IGUIFont* loadingFont;

		gui::IGUIFont* musicTagFont;

		gui::IGUIFont* statsFont;

		gui::IGUIFont* textFont;
		gui::IGUIFont* tipFont;


		//Misc. Irrlicht types----------------------------------
		scene::ISceneManager* bgscene;

		video::E_DRIVER_TYPE driverType;

		gui::IGUIFileOpenDialog* fileChooser;

		core::array<SJoystickInfo> joystickInfo;


		boost::filesystem::directory_iterator currentFile;


		//Misc. SDL/SDL_Mixer types----------------------------------
		Mix_Music* music;


		//Misc. Boost/Boost Filesystem types----------------------------------
		boost::filesystem::path currentMusic;

		vector<boost::filesystem::path> musicList;


		//Other types----------------------------------
		size_t currentProTip;
};

#endif // GAMEMANAGER_H

