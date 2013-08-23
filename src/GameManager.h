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

#include "Integers.h"
#include <irrlicht.h>
#include <SDL_mixer.h>
#include <string>
#include <vector>

using namespace irr;

//using boost::asio::ip::tcp;

class GameManager : public IEventReceiver {
	public:
		//Functions----------------------------------
		void drawAll();
		void drawLoadingScreen();

		GameManager();
		virtual ~GameManager();
		bool getDebugStatus();
		MazeManager* getMazeManager();
		Goal* getGoal();
		Player* getPlayer( uint_least8_t p );

		void movePlayerOnX( uint_least8_t p, int_fast8_t direction );
		void movePlayerOnY( uint_least8_t p, int_fast8_t direction );

		bool OnEvent( const SEvent& );

		void resetThings();
		int run();

		void setControls();


		//Non-functions----------------------------------
		std::vector< AI > bot;

		IrrlichtDevice* device;
		video::IVideoDriver* driver;

		Goal goal;
		gui::IGUIEnvironment* gui;

		uint_least16_t loadingDelay;

		uint_least8_t numBots;
		uint_least8_t numLocks;
		uint_least8_t numPlayers;

		std::vector<Player> player;
		std::vector<PlayerStart> playerStart;

		uint_least16_t randomSeed;

		StringConverter stringConverter;
		std::vector<Collectable> stuff;

		ITimer* timer;
		uint_least16_t timeStartedLoading;

	protected:
	private:
		//Functions----------------------------------
		void adjustMenu();
		bool allHumansAtGoal();

		bool doEventActions( std::vector< KeyMapping >::size_type k, const SEvent& event );
		void drawBackground();

		void loadFonts();
		void loadMusicFont();
		void loadNextSong();
		void loadProTips();
		void loadStatsFont();
		void loadTipFont();

		void makeMusicList();

		void newMaze();
		void newMaze( boost::filesystem::path src );

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
		uint_least8_t backgroundChosen;

		uint_least8_t myPlayer; //If in client mode, control only one player

		uint_least8_t numKeysFound;

		uint_least8_t sideDisplaySizeDenominator;

		std::vector< uint_least8_t > winners;


		//unsigned 16-bit integers----------------------------------
		uint_least16_t musicVolume;


		//unsigned 16-bit integers----------------------------------
		uint_least16_t bitsPerPixel;

		uint_least16_t cellWidth;
		uint_least16_t cellHeight;

		uint_least16_t joystickChosen;


		//wide character strings----------------------------------
		core::stringw fontFile;
		core::stringw loading;

		core::stringw musicAlbum;
		core::stringw musicArtist;
		core::stringw musicTitle;

		core::stringw proTipPrefix;
		std::vector< core::stringw > proTips;

		core::stringw stats;
		core::stringw steps;


		//Our own types----------------------------------
		MenuOption backToGame;

		MenuOption exitGame;

		FontManager fm;

		std::vector< KeyMapping > keyMap;

		MenuOption loadMaze;

		MazeManager mazeManager;

		NetworkManager network;
		MenuOption newGame;

		MenuOption saveMaze;


		//2D dimensions----------------------------------
		core::dimension2d<uint_least16_t> viewportSize;
		core::dimension2d<uint_least32_t> windowSize;


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

		std::vector<boost::filesystem::path> musicList;


		//Other types----------------------------------
		size_t currentProTip;
};

#endif // GAMEMANAGER_H

