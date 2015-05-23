/**
 * @file
 * @author James Dearing <dearingj@lifetime.oregonstate.edu>
 * 
 * @section LICENSE
 * Copyright © 2012-2015.
 * This file is part of Cybrinth.
 *
 * Cybrinth is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * Cybrinth is distributed in the hope that it will be fun, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License along with Cybrinth. If not, see <http://www.gnu.org/licenses/>.
 * 
 * @section DESCRIPTION
 * The MainGame class is the overseer of all other classes. It's also where the game's main loop is.
 */

#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include "AI.h"
#include "Collectable.h"
#include "FileSelectorDialog.h"
#include "FontManager.h"
#include "Goal.h"
#include "GUIFreetypeFont.h"
#include "ImageModifier.h"
#include "Integers.h"
#include "ControlMapping.h"
#include "MazeManager.h"
#include "MenuManager.h"
#include "NetworkManager.h"
#include "Player.h"
#include "PlayerStart.h"
#include "PreprocessorCommands.h"
#include "SpellChecker.h"
#include "StringConverter.h"
#include "SystemSpecificsManager.h"

#include <irrlicht/irrlicht.h>
#include <random>
#include <SDL_mixer.h>
#ifdef HAVE_STRING
	#include <string>
#endif //HAVE_STRING
#ifdef HAVE_VECTOR
	#include <vector>
#endif //HAVE_VECTOR

//using boost::asio::ip::tcp;

class MainGame : public irr::IEventReceiver {
	public:
		//Functions----------------------------------
		MainGame();
		virtual ~MainGame();
		
		void allPlayersReady( bool tf );
		
		void drawAll(); //Public because it's called by MazeManager. Otherwise the loading screen wouldn't get drawn during maze generation.
		
		void eraseCollectable( uint_fast8_t item );
		
		Collectable* getCollectable( uint_fast8_t collectable );
		bool getDebugStatus();
		Goal* getGoal();
		Collectable* getKey( uint_fast8_t key );
		float getLoadingPercentage();
		std::minstd_rand::result_type getMaxRandomNumber(); //The highest value the random number generator can output.
		MazeManager* getMazeManager();
		uint_fast8_t getNumCollectables();
		uint_fast8_t getNumKeys();
		Player* getPlayer( uint_fast8_t p );
		std::minstd_rand::result_type getRandomNumber(); //C++'s rand() function can very between platforms or compilers; for consistency, therefore, we use a specific one of C++11's random number generators.
		std::minstd_rand::result_type getRandomSeed();
		PlayerStart* getStart( uint_fast8_t ps );
		
		void movePlayerOnX( uint_fast8_t p, int_fast8_t direction, bool fromServer );
		void movePlayerOnY( uint_fast8_t p, int_fast8_t direction, bool fromServer );
		
		void networkHasNewConnection(); //Called by NetworkManager
		void newMaze();
		void newMaze( std::minstd_rand::result_type newRandomSeed );
		void newMaze( boost::filesystem::path src );
		
		bool OnEvent( const irr::SEvent& );
		
		void pickLogo();
		
		void resetThings();
		uint_fast8_t run( std::wstring fileToLoad ); //If a file was specified on the command line, it will be passed to run().
		
		void setControls();
		void setExitConfirmation( irr::gui::IGUIWindow* newWindow );
		void setFileChooser( irr::gui::IGUIFileOpenDialog* newChooser );
		void setLoadingPercentage( float newPercent );
		void setMyPlayer( uint_fast8_t newPlayer );
		void setRandomSeed( std::minstd_rand::result_type newSeed );
		void showLoadMazeDialog();
		void showSaveMazeDialog();
		
		//Non-functions----------------------------------
		std::vector< AI > bot;
		
		irr::IrrlichtDevice* device;
		irr::video::IVideoDriver* driver;
		
		Goal goal;
		irr::gui::IGUIEnvironment* gui;
		
		uint_fast16_t loadingDelay;
		
		uint_fast8_t numBots;
		uint_fast8_t numLocks;
		uint_fast8_t numPlayers;
		
		std::vector< Player > player;
		std::vector< PlayerStart > playerStart;
		
		bool showingMenu;
		StringConverter stringConverter;
		std::vector< Collectable > stuff;
		
		irr::ITimer* timer;
		uint_fast16_t timeStartedLoading;
		
	protected:
	private:
		//Functions----------------------------------
		bool allHumansAtGoal();
		
		void drawBackground();
		void drawLoadingScreen();
		void drawLogo();
		void drawStats( uint_fast32_t textY );
		
		bool haveShownLogo;
		
		bool isNull( void* ptr );
		
		void loadFonts();
		void loadMusicFont();
		void loadNextSong();
		void loadProTips();
		void loadStatsFont();
		void loadTipFont();
		
		void makeMusicList();
		void movePlayerCommon( uint_fast8_t p );
		
		bool prefIsTrue( std::wstring choice );
		void processControls();
		
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
		
		bool enableController;
		
		bool fullscreen;
		
		bool isServer;
		
		bool markTrails;
		bool musicPlaying;
		
		bool playMusic;
		
		bool showBackgrounds;
		bool showingLoadingScreen;
		
		bool vsync;
		
		bool waitingForOtherPlayers;
		bool won;
		
		//unsigned 8-bit integers----------------------------------
		uint_fast8_t backgroundChosen;
		
		uint_fast8_t joystickDeadZoneDefaultPercent;
		
		uint_fast8_t myPlayer; //If in client mode, control only one player
		std::vector< bool > playerAssigned; //If in server mode, keep track of which player numbers have been assigned to players
		
		uint_fast8_t numKeysFound;
		
		uint_fast8_t sideDisplaySizeDenominator;
		
		std::vector< uint_fast8_t > winners;
		std::vector< uint_fast8_t > winnersLoadingScreen; //An ugly hack: Copy winners to winnersLoadingScreen so that we can show it on the loading screen after winners is cleared
		
		//unsigned 16-bit integers----------------------------------
		uint_fast16_t bitsPerPixel;
		uint_fast16_t botMovementDelay;
		
		uint_fast16_t cellWidth;
		uint_fast16_t cellHeight;
		
		uint_fast16_t musicVolume;
		
		//unsigned 32-bit integers----------------------------------
		uint_fast32_t lastTimeControlsProcessed;
		uint_fast32_t controlProcessDelay;
		uint_fast32_t minWidth;
		uint_fast32_t minHeight;
		
		//signed 32-bit integers----------------------------------
		irr::s32 mouseX;
		irr::s32 mouseY;
		
		//floating point numbers----------------------------------
		float loadingProgress;
		
		//wide character strings----------------------------------
		irr::core::stringw fontFile;
		
		irr::core::stringw heightTestString; //Used only when loading fonts
		
		irr::core::stringw keysFoundPerPlayer;
		
		irr::core::stringw loading;
		irr::core::stringw loadingStatus;
		
		irr::core::stringw musicAlbum;
		irr::core::stringw musicArtist;
		irr::core::stringw musicTitle;
		
		irr::core::stringw proTipPrefix;
		std::vector< irr::core::stringw > proTips;
		
		irr::core::stringw winnersLabel;
		irr::core::stringw scores;
		irr::core::stringw scoresTotal;
		irr::core::stringw steps;
		
		irr::core::stringw times;
		
		//Our own types----------------------------------
		AI::algorithm_t botAlgorithm;
		
		MenuManager menuManager;
		
		FontManager fontManager;
		
		std::vector< ControlMapping > controls;
		
		MazeManager mazeManager;
		
		NetworkManager network;
		
		ImageModifier resizer;
		
		SpellChecker spellChecker;
		
		SystemSpecificsManager system; // Flawfinder: ignore
		
		//2D dimensions----------------------------------
		irr::core::dimension2d< uint_fast16_t > viewportSize;
		irr::core::dimension2d< irr::u32 > windowSize;
		
		//Fonts----------------------------------
		irr::gui::IGUIFont* clockFont;
		
		irr::gui::IGUIFont* loadingFont;
		
		irr::gui::IGUIFont* musicTagFont;
		
		irr::gui::IGUIFont* statsFont;
		
		irr::gui::IGUIFont* textFont;
		irr::gui::IGUIFont* tipFont;
		
		//Misc. Irrlicht types----------------------------------
		irr::video::SColor backgroundColor;
		irr::io::path backgroundFilePath;
		irr::scene::ISceneManager* backgroundSceneManager;
		irr::video::ITexture* backgroundTexture;
		
		irr::core::array< irr::SJoystickInfo > controllerInfo;
		
		irr::video::E_DRIVER_TYPE driverType;
		
		irr::gui::IGUIWindow* exitConfirmation;
		
		//irr::gui::IGUIFileOpenDialog* fileChooser;
		
		FileSelectorDialog* loadMazeDialog;
		irr::video::ITexture* logoTexture;
		
		FileSelectorDialog* saveMazeDialog;
		
		
		//Misc. SDL/SDL_Mixer types----------------------------------
		Mix_Music* music;
		
		//Misc. Boost/Boost Filesystem types----------------------------------
		boost::filesystem::path currentDirectory;
		boost::filesystem::directory_iterator currentFile;
		boost::filesystem::path currentMusic;
		
		std::vector<boost::filesystem::path> musicList;
		
		//Other types----------------------------------
		size_t currentProTip;
		std::minstd_rand randomNumberGenerator;
		std::minstd_rand::result_type randomSeed;
};

#endif // GAMEMANAGER_H

