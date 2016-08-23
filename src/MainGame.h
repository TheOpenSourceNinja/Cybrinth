/**
 * @file
 * @author James Dearing <dearingj@lifetime.oregonstate.edu>
 * 
 * @section LICENSE
 * Copyright © 2012-2016.
 * This file is part of Cybrinth.
 *
 * Cybrinth is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * Cybrinth is distributed 'as is' in the hope that it will be fun, but WITHOUT ANY WARRANTY; without even the implied warranty of TITLE, MERCHANTABILITY, COMPLETE DESTRUCTION OF EVIL MONSTERS, or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
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
#include "SettingsManager.h"
#include "SettingsScreen.h"
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
		MainGame( std::wstring fileToLoad, bool runAsScreenSaver );
		virtual ~MainGame();
		
		void adjustImageColors( irr::video::IImage* image );
		void allPlayersReady( bool tf );
		
		void displayExitConfirmation();
		void drawAll(); //Public because it's called by MazeManager. Otherwise the loading screen wouldn't get drawn during maze generation.
		
		void eraseCollectable( uint_fast8_t item );
		
		Collectable* getCollectable( uint_fast8_t collectable );
		irr::video::SColor getColorBasedOnNum( uint_fast8_t num );
		bool getDebugStatus();
		Goal* getGoal();
		Collectable* getKey( uint_fast8_t key );
		std::vector< boost::filesystem::path > getLoadableTexturesList( boost::filesystem::path searchLocation );
		float getLoadingPercentage();
		std::minstd_rand::result_type getMaxRandomNumber(); //The highest value the random number generator can output.
		MazeManager* getMazeManager();
		uint_fast8_t getNumCollectables();
		uint_fast8_t getNumKeys();
		Player* getPlayer( uint_fast8_t p );
		std::minstd_rand::result_type getRandomNumber(); //C++'s rand() function can very between platforms or compilers; for consistency, therefore, we use a specific one of C++11's random number generators.
		std::minstd_rand::result_type getRandomSeed();
		irr::core::dimension2d< irr::u32 > getScreenSize();
		PlayerStart* getStart( uint_fast8_t ps );
		
		bool isNull( void* ptr );
		
		void movePlayerOnX( uint_fast8_t p, int_fast8_t direction, bool fromServer );
		void movePlayerOnY( uint_fast8_t p, int_fast8_t direction, bool fromServer );
		void musicSettingChanged(); //Called by settingsManager to notify us the playMusic setting has been changed.
		void musicVolumeChanged();
		
		void networkHasNewConnection(); //Called by NetworkManager
		void newMaze();
		void newMaze( std::minstd_rand::result_type newRandomSeed );
		void newMaze( boost::filesystem::path src );
		
		bool OnEvent( const irr::SEvent& event );
		
		void pickLogo();
		void promptForServerIP();
		
		void resetThings();
		uint_fast8_t run(); //If a file was specified on the command line, it will be passed to run().
		
		void setControls();
		//void setExitConfirmation( irr::gui::IGUIWindow* newWindow );
		void setFileChooser( irr::gui::IGUIFileOpenDialog* newChooser );
		void setLoadingPercentage( float newPercent );
		void setMyPlayer( uint_fast8_t newPlayer );
		void setNumBots( uint_fast8_t newNumBots );
		void setNumPlayers( uint_fast8_t newNumPlayers );
		void setObjectColorBasedOnNum( Object* object, uint_fast8_t num );
		void setRandomSeed( std::minstd_rand::result_type newSeed );
		void showLoadMazeDialog();
		void showSaveMazeDialog();
		
		//Non-functions----------------------------------
		std::vector< AI > bot;
		
		enum screen_t : uint_fast8_t { MAINSCREEN, LOADINGSCREEN, MENUSCREEN, SETTINGSSCREEN, WAITINGFORPLAYERSSCREEN };
		screen_t currentScreen;
		
		irr::IrrlichtDevice* device;
		irr::video::IVideoDriver* driver;
		
		Goal goal;
		irr::gui::IGUIEnvironment* gui;
		
		uint_fast16_t loadingDelay;
		
		NetworkManager network;
		uint_fast8_t numLocks;
		
		std::vector< Player > player;
		std::vector< PlayerStart > playerStart;
		
		SettingsManager settingsManager;
		SettingsScreen settingsScreen;
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
		void drawSidebarText();
		void drawStats( uint_fast32_t textY );
		
		void initializeVariables( bool runAsScreenSaver );
		
		void loadClockFont();
		void loadExitConfirmations();
		void loadFonts();
		void loadMusicFont();
		void loadNextSong();
		void loadProTips();
		bool loadSeedFromFile( boost::filesystem::path src );
		void loadStatsFont();
		void loadTextures();
		void loadTipFont();
		
		void makeMusicList();
		void movePlayerCommon( uint_fast8_t p );
		
		void processControls();
		
		void setupBackground();
		void setupDevice();
		void setupDriver();
		void setupMusicStuff();
		void startLoadingScreen();
		
		void takeScreenShot();
		
		//Booleans----------------------------------
		bool antiAliasFonts;
		
		bool donePlaying;
		
		bool enableController;
		
		bool fillBackgroundTextureAfterLoading; //The "STARTRAILS" background animation requires this
		
		bool firstMaze; //If we're loading from a file specified on the command line, we don't want to reset the random seed when generating the first maze.
		
		bool haveFilledBackgroundTextureAfterLoading;
		
		bool haveShownLogo;
		
		bool isScreenSaver;
		
		bool musicPlaying;
		
		std::vector< bool > playerAssigned; //If in server mode, keep track of which player numbers have been assigned to players
		
		bool won;
		
		//unsigned 8-bit integers----------------------------------
		uint_fast8_t backgroundChosen;
		
		uint_fast8_t joystickDeadZoneDefaultPercent;
		
		uint_fast8_t myPlayer; //If in client mode, control only one player
		
		uint_fast8_t numKeysFound;
		
		uint_fast8_t sideDisplaySizeDenominator;
		
		std::vector< uint_fast8_t > winners;
		std::vector< uint_fast8_t > winnersLoadingScreen; //An ugly hack: Copy winners to winnersLoadingScreen so that we can show it on the loading screen after winners is cleared
		
		//unsigned 16-bit integers----------------------------------
		uint_fast16_t cellWidth;
		uint_fast16_t cellHeight;
		
		//unsigned 32-bit integers----------------------------------
		uint_fast32_t lastTimeControlsProcessed;
		uint_fast32_t controlProcessDelay;
		
		//signed 32-bit integers----------------------------------
		irr::s32 mouseX;
		irr::s32 mouseY;
		
		//floating point numbers----------------------------------
		float loadingProgress;
		
		//wide character strings----------------------------------
		std::vector< irr::core::stringw > exitConfirmations;
		
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
		MenuManager menuManager;
		
		FontManager fontManager;
		
		std::vector< ControlMapping > controls;
		
		MazeManager mazeManager;
		
		ImageModifier resizer;
		SpellChecker spellChecker;
		
		SystemSpecificsManager system; // Flawfinder: ignore
		
		//2D dimensions----------------------------------
		irr::core::dimension2d< irr::u32 > screenSize;
		
		irr::core::dimension2d< uint_fast16_t > viewportSize;
		
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
		size_t currentExitConfirmation;
		size_t currentProTip;
		std::minstd_rand randomNumberGenerator;
		std::minstd_rand::result_type randomSeed;
		enum background_t : uint_fast8_t { ORIGINAL_STARFIELD, ROTATING_STARFIELD, IMAGES, STAR_TRAILS, PLAIN_COLOR, NUMBER_OF_BACKGROUNDS };
		enum user_event_t : uint_fast8_t { USER_EVENT_WINDOW_RESIZE };
};

#endif // GAMEMANAGER_H

