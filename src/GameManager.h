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
		GameManager();
		virtual ~GameManager();
		s32 run();
		bool OnEvent( const SEvent& );
		Player* getPlayer( u8 p );
		void movePlayerOnX( u8 p, s8 direction );
		void movePlayerOnY( u8 p, s8 direction );
		Goal goal;
		bool getDebugStatus();
		ITimer* timer;
	protected:
	private:
		bool debug;
		IrrlichtDevice* device;
		video::IVideoDriver* driver;
		MazeCell ** maze;
		u8 cols;
		u8 rows;
		core::dimension2d<u32> windowSize;
		core::dimension2d<u32> viewportSize;
		void loadFromFile();
		bool loadFromFile( boost::filesystem::path src );
		void saveToFile();
		bool saveToFile( boost::filesystem::path dest );
		void resizeMaze( u8 newCols, u8 newRows );
		vector<Player> player;
		u8 numPlayers;
		vector<PlayerStart> playerStart;
		boost::filesystem::directory_iterator currentFile;
		bool loadNextLevel();
		scene::ISceneManager* bgscene;
		void setupBackground();
		bool donePlaying;
		bool showingMenu;
		void drawAll();
		u32 cellWidth;
		u32 cellHeight;
		MenuOption newGame;
		MenuOption loadMaze;
		MenuOption saveMaze;
		MenuOption exitGame;
		MenuOption backToGame;
		gui::IGUIEnvironment* gui;
		Mix_Music *music;
		int channel;
		//void musicFinished();
		bool musicPlaying;
		u8 numKeysFound;
		u8 numLocks;
		vector<Collectable> stuff;
		void makeRandomLevel();
		void recurseRandom( u8 x, u8 y, u16 depth, u16 numSoFar);
		//u8 numKeys;
		FontManager fm;
		gui::IGUIFont* clockFont;
		gui::IGUIFont* textFont;
		gui::IGUIFont* musicTagFont;
		gui::IGUIFont* loadingFont;
		void resetThings();
		bool won;
		bool canGetTo( u8 startX, u8 startY, u8 goalX, u8 goalY );
		bool canGetToAllCollectables( u8 startX, u8 startY );
		bool existsAnythingAt( u8 x, u8 y );
		void readPrefs();
		bool fullscreen;
		u32 bitsPerPixel;
		bool vsync;
		video::E_DRIVER_TYPE driverType;
		bool playMusic;
		//boost::asio::io_service io;
		bool isServer;
		//boost::asio::io_service io_service;
		void loadNextSong();
		bool allowSmallSize;
		void *get_in_addr( struct sockaddr *sa );
		NetworkManager network;
		bool enableJoystick;
		u32 joystickChosen;
		core::array<SJoystickInfo> joystickInfo;
		u8 myPlayer; //If in client mode, control only one player
		vector<boost::filesystem::path> musicList;
		void makeMusicList();
		boost::filesystem::path currentMusic;
		void takeScreenShot();
		core::stringw musicTitle;
		core::stringw musicArtist;
		core::stringw musicAlbum;
		u32 randomSeed;
		gui::IGUIFileOpenDialog* fileChooser;
		StringConverter stringConverter;
		void drawBackground();
		u8 backgroundChosen;
		u8 numBots;
		vector< AI > bot;
		bool allHumansAtGoal();
		bool markTrails;
		bool showingLoadingScreen;
		core::stringw loading;
		core::stringw proTipStart;
		void loadFonts();
		u32 timeStartedLoading;
		void startLoadingScreen();
		vector< core::stringw > proTips;
		size_t currentProTip;
		u32 loadingDelay;
		void loadProTips();
		void loadTipFont();
		gui::IGUIFont* tipFont;
		void loadStatsFont();
		gui::IGUIFont* statsFont;
		core::stringw stats;
		core::stringc fontFile;
		core::stringw steps;
		vector< u8 > winners;
		bool antiAliasFonts;
		void loadMusicFont();
		u16 musicVolume;
};

#endif // GAMEMANAGER_H
