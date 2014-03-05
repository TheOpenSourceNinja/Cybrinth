/**
 * Copyright © 2012-2014 James Dearing.
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

#include "AI.h"
#include "Collectable.h"
#include "FontManager.h"
#include "Goal.h"
#include "GUIFreetypeFont.h"
#include "ImageModifier.h"
#include "Integers.h"
#include "KeyMapping.h"
#include "MazeManager.h"
#include "MenuOption.h"
#include "NetworkManager.h"
#include "Player.h"
#include "PlayerStart.h"
#include "PreprocessorCommands.h"
#include "SpellChecker.h"
#include "StringConverter.h"
#include "SystemSpecificsManager.h"

#include <irrlicht.h>
#include <SDL_mixer.h>
#include <string>
#include <vector>

using namespace irr;

//using boost::asio::ip::tcp;

class GameManager : public IEventReceiver {
	public:
		//Functions----------------------------------
		GameManager();
		virtual ~GameManager();

		void drawAll(); //Public because it's called by MazeManager. Otherwise the loading screen wouldn't get drawn during maze generation.

		Collectable* getCollectable( uint_fast8_t collectable );
		bool getDebugStatus();
		Goal* getGoal();
		Collectable* getKey( uint_fast8_t key );
		MazeManager* getMazeManager();
		uint_fast8_t getNumKeys();
		Player* getPlayer( uint_fast8_t p );
		PlayerStart* getStart( uint_fast8_t ps );

		void movePlayerOnX( uint_fast8_t p, int_fast8_t direction );
		void movePlayerOnY( uint_fast8_t p, int_fast8_t direction );

		bool OnEvent( const SEvent& );
		
		void pickLogo();

		void resetThings();
		uint_fast8_t run();

		void setControls();


		//Non-functions----------------------------------
		std::vector< AI > bot;

		IrrlichtDevice* device;
		video::IVideoDriver* driver;

		Goal goal;
		gui::IGUIEnvironment* gui;

		uint_fast16_t loadingDelay;

		uint_fast8_t numBots;
		uint_fast8_t numLocks;
		uint_fast8_t numPlayers;

		std::vector< Player > player;
		std::vector< PlayerStart > playerStart;

		uint_fast16_t randomSeed;

		StringConverter stringConverter;
		std::vector< Collectable > stuff;

		ITimer* timer;
		uint_fast16_t timeStartedLoading;

	protected:
	private:
		//Functions----------------------------------
		void adjustMenu();
		bool allHumansAtGoal();

		bool doEventActions( std::vector< KeyMapping >::size_type k, const SEvent& event );
		void drawBackground();
		void drawLoadingScreen();
		void drawLogo();
		
		bool haveShownLogo;
		
		bool isNotNull( void* ptr );

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
		
		bool botsKnowSolution;

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
		uint_fast8_t backgroundChosen;

		uint_fast8_t myPlayer; //If in client mode, control only one player

		uint_fast8_t numKeysFound;

		uint_fast8_t sideDisplaySizeDenominator;

		std::vector< uint_fast8_t > winners;


		//unsigned 16-bit integers----------------------------------
		uint_fast16_t bitsPerPixel;

		uint_fast16_t cellWidth;
		uint_fast16_t cellHeight;

		uint_fast16_t joystickChosen;

		uint_fast16_t musicVolume;


		//unsigned 32-bit integers----------------------------------
		uint_fast32_t minWidth;
		uint_fast32_t minHeight;


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
		AI::algorithm_t botAlgorithm;
		
		MenuOption backToGame;
		
		MenuOption freedom;

		MenuOption exitGame;

		FontManager fontManager;

		std::vector< KeyMapping > keyMap;

		MenuOption loadMaze;

		MazeManager mazeManager;

		NetworkManager network;
		MenuOption newGame;

		ImageModifier resizer;

		MenuOption saveMaze;
		SpellChecker spellChecker;
		
		SystemSpecificsManager system;


		//2D dimensions----------------------------------
		core::dimension2d< uint_fast16_t > viewportSize;
		core::dimension2d< u32 > windowSize;


		//Fonts----------------------------------
		gui::IGUIFont* clockFont;

		gui::IGUIFont* loadingFont;

		gui::IGUIFont* musicTagFont;

		gui::IGUIFont* statsFont;

		gui::IGUIFont* textFont;
		gui::IGUIFont* tipFont;


		//Misc. Irrlicht types----------------------------------
		scene::ISceneManager* bgscene;
		video::ITexture* backgroundTexture;
		io::path backgroundFilePath;

		video::E_DRIVER_TYPE driverType;

		gui::IGUIWindow* exitConfirmation;

		gui::IGUIFileOpenDialog* fileChooser;

		core::array<SJoystickInfo> joystickInfo;

		video::ITexture* logoTexture;

		boost::filesystem::directory_iterator currentFile;


		//Misc. SDL/SDL_Mixer types----------------------------------
		Mix_Music* music;


		//Misc. Boost/Boost Filesystem types----------------------------------
		boost::filesystem::path currentDirectory;
		boost::filesystem::path currentMusic;

		std::vector<boost::filesystem::path> musicList;


		//Other types----------------------------------
		size_t currentProTip;
};

#endif // GAMEMANAGER_H

