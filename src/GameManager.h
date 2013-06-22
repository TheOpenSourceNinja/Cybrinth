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
#include "MazeCell.h"
#include "font_manager.h"
#include "Player.h"
#include "Goal.h"
#include "Collectable.h"
#include "MenuOption.h"
#include "PlayerStart.h"
#include "gui_freetype_font.h"
#include "NetworkManager.h"
#include "StringConverter.h"
#include <irrlicht.h>
#include <vector>
#include <filesystem.hpp>
#include <asio.hpp>
#include <SDL/SDL.h>
#include <SDL_mixer.h>
#include <queue>
 
using namespace irr;
using namespace std;
using boost::asio::ip::tcp;
 
class GameManager : public IEventReceiver {
	public:
		//Functions----------------------------------
		GameManager();
		virtual ~GameManager();
		bool getDebugStatus();
		Goal getGoal();
		Player* getPlayer( u8 p );
		
		void movePlayerOnX( u8 p, s8 direction );
		void movePlayerOnY( u8 p, s8 direction );
		
		bool OnEvent( const SEvent& );
		
		s32 run();
		
		
		//Non-functions----------------------------------
		MazeCell** maze;
		ITimer* timer;
		
	protected:
	private:
		//Functions----------------------------------
		bool allHumansAtGoal();
		
		bool canGetTo( u8 startX, u8 startY, u8 goalX, u8 goalY );
		bool canGetToAllCollectables( u8 startX, u8 startY );
		
		void drawAll();
		void drawBackground();
		
		bool existsAnythingAt( u8 x, u8 y );
		
		void loadFonts();
		void loadFromFile();
		bool loadFromFile( boost::filesystem::path src );
		void loadMusicFont();
		void loadNextSong();
		void loadProTips();
		void loadStatsFont();
		void loadTipFont();
		
		void makeMusicList();
		void makeRandomLevel();
		
		void readPrefs();
		void recurseRandom( u8 x, u8 y, u16 depth, u16 numSoFar );
		void resetThings();
		void resizeMaze( u8 newCols, u8 newRows );
		
		void saveToFile();
		bool saveToFile( boost::filesystem::path dest );
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
		
		bool showingLoadingScreen;
		bool showingMenu;
		
		bool vsync;
		
		bool won;
		
		
		//unsigned 8-bit integers----------------------------------
		u8 backgroundChosen;
		
		u8 cols;
		
		u8 myPlayer; //If in client mode, control only one player
		
		u8 numBots;
		u8 numKeysFound;
		u8 numLocks;
		u8 numPlayers;
		
		u8 rows;
		
		u8 sideDisplaySizeDenominator;
		
		vector< u8 > winners;
		
		
		//unsigned 16-bit integers----------------------------------
		u16 musicVolume;
		
		
		//unsigned 32-bit integers----------------------------------
		u32 bitsPerPixel;
		
		u32 cellWidth;
		u32 cellHeight;
		
		u32 joystickChosen;
		
		u32 loadingDelay;
		
		u32 randomSeed;
		
		u32 timeStartedLoading;
		
		
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
		vector< AI > bot;
		
		MenuOption exitGame;
		
		FontManager fm;
		
		Goal goal;
		
		MenuOption loadMaze;
		
		NetworkManager network;
		MenuOption newGame;
		
		vector<Player> player;
		vector<PlayerStart> playerStart;
		
		MenuOption saveMaze;
		StringConverter stringConverter;
		vector<Collectable> stuff;
		
		
		//2D dimensions----------------------------------
		core::dimension2d<u32> viewportSize;
		core::dimension2d<u32> windowSize;
		
		
		//Fonts----------------------------------
		gui::IGUIFont* clockFont;
		
		gui::IGUIFont* loadingFont;
		
		gui::IGUIFont* musicTagFont;
		
		gui::IGUIFont* statsFont;
		
		gui::IGUIFont* textFont;
		gui::IGUIFont* tipFont;
		
		
		//Misc. Irrlicht types----------------------------------
		scene::ISceneManager* bgscene;
		
		IrrlichtDevice* device;
		video::IVideoDriver* driver;
		video::E_DRIVER_TYPE driverType;
		
		gui::IGUIFileOpenDialog* fileChooser;
		
		gui::IGUIEnvironment* gui;
		
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
 
