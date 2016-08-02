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
 * The SettingsManager class is responsible for reading game settings from prefs.cfg, storing them in prefs.cfg, and keep them in memory while the game runs. (For the graphical settings screen, see SettingsScreen.{cpp|h})
 */

#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include "AI.h"
//#include "MazeManager.h"
#include "NetworkManager.h"
#include "SpellChecker.h"
#include "SystemSpecificsManager.h"

class MazeManager; //Avoids circular dependency

class SettingsManager
{
	public:
		SettingsManager();
		~SettingsManager();
		
		void setPointers( irr::IrrlichtDevice* newDevice, MainGame* newMainGame, MazeManager* newMazeManager, NetworkManager* newNetwork, SpellChecker* newSpellChecker, SystemSpecificsManager* newSystem ); //Must be called before readPrefs!
		
		bool allowSmallSize;
		bool alwaysServer;
		bool alwaysServerDefault;
		bool autoDetectFullscreenResolution;
		bool autoDetectFullscreenResolutionDefault;
		
		bool backgroundAnimations;
		bool backgroundAnimationsDefault;
		
		AI::algorithm_t botAlgorithm;
		AI::algorithm_t botAlgorithmDefault;
		uint_fast16_t botMovementDelay;
		uint_fast16_t botMovementDelayDefault;
		bool botsKnowSolution;
		bool botsKnowSolutionDefault;
		
		std::wstring dateFormat; //for use by wcsftime()
		std::wstring dateFormatDefault;
		bool debug;
		bool debugDefault;
		irr::video::E_DRIVER_TYPE driverType;
		irr::video::E_DRIVER_TYPE driverTypeDefault;
		
		bool fullscreen;
		bool fullscreenDefault;
		irr::core::dimension2d< irr::u32 > getFullscreenResolution();
		
		uint_fast8_t getBitsPerPixel();
		irr::core::dimension2d< irr::u32 > getMinimumWindowSize();
		uint_fast8_t getMusicVolume();
		uint_fast8_t getNumBots();
		uint_fast8_t getNumPlayers();
		bool getPlayMusic();
		irr::core::dimension2d< irr::u32 > getWindowSize();
		
		bool isServer;
		bool isServerDefault;
		
		bool markTrails;
		bool markTrailsDefault;
		
		uint_fast16_t networkPort;
		
		void readPrefs();
		void resetToDefaults();
		
		void savePrefs();
		void setBitsPerPixel( uint_fast8_t newBPP );
		void setFullscreenResolution( irr::core::dimension2d< irr::u32 > newResolution );
		void setMusicVolume( uint_fast8_t newVolume );
		void setNumBots( uint_fast8_t newNumBots );
		void setNumPlayers( uint_fast8_t newNumPlayers );
		void setPlayMusic( bool newSetting );
		void setWindowSize( irr::core::dimension2d< irr::u32 > newSize );
		bool showBackgrounds;
		bool showBackgroundsDefault;
		
		std::wstring timeFormat; //for use by wcsftime()
		std::wstring timeFormatDefault;
		
		bool vsync;
		bool vsyncDefault;
	
	private:
		uint_fast8_t bitsPerPixel;
		uint_fast8_t bitsPerPixelDefault;
		
		irr::IrrlichtDevice* device;
		std::vector< std::wstring > driverTypes = { L"opengl", L"direct3d9", L"direct3d8", L"burning's video", L"software", L"null" };
		enum driver_t : uint_fast8_t { OPENGL = 0, DIRECT3D9 = 1, DIRECT3D8 = 2, BURNINGS = 3, SOFTWARE = 4, DRIVERNULL = 5, DRIVERDO_NOT_USE = 255 };
		
		irr::core::dimension2d< irr::u32 > fullscreenResolution;
		irr::core::dimension2d< irr::u32 > fullscreenResolutionDefault;
		
		bool hideUnseenDefault;
		
		MainGame* mainGame;
		MazeManager* mazeManager;
		irr::core::dimension2d< irr::u32 > minimumWindowSize; //This should be ignored if allowSmallSize is true.
		uint_fast8_t musicVolume;
		uint_fast8_t musicVolumeDefault;
		
		NetworkManager* network;
		
		uint_fast16_t networkPortDefault;
		uint_fast8_t numBots;
		uint_fast8_t numBotsDefault;
		uint_fast8_t numPlayers;
		uint_fast8_t numPlayersDefault;
		
		bool playMusic;
		bool playMusicDefault;
		//possiblePrefs is used by both readPrefs() and savePrefs(). If this gets changed, be sure to update both functions!
		std::vector< std::wstring > possiblePrefs = { L"bots' solving algorithm", L"volume", L"number of bots", L"show backgrounds",
									L"fullscreen", L"mark player trails", L"debug", L"bits per pixel", L"wait for vertical sync", L"driver type", L"number of players",
									L"window size", L"play music", L"network port", L"always server", L"bots know the solution", L"bot movement delay", L"hide unseen maze areas", L"background animations",
									L"autodetect fullscreen resolution", L"fullscreen resolution", L"time format", L"date format" };
		//Each item in pref_t must match with an item in possiblePrefs.
		enum pref_t : uint_fast8_t { ALGORITHM = 0, VOLUME = 1, NUMBOTS = 2, SHOW_BACKGROUNDS = 3, FULLSCREEN = 4, MARK_TRAILS = 5, DEBUG = 6, BPP = 7, VSYNC = 8, DRIVER_TYPE = 9, NUMPLAYERS = 10,
									WINDOW_SIZE = 11, PLAY_MUSIC = 12, NETWORK_PORT = 13, ALWAYS_SERVER = 14, SOLUTION_KNOWN = 15, MOVEMENT_DELAY = 16, HIDE_UNSEEN = 17, BACKGROUND_ANIMATIONS = 18, 
									AUTODETECT_RESOLUTION = 19, FULLSCREEN_RESOLUTION = 20, TIME_FORMAT = 21, DATE_FORMAT = 22 };
		
		SpellChecker* spellChecker;
		SystemSpecificsManager* system; // Flawfinder: ignore
		
		irr::core::dimension2d< irr::u32 > windowSize;
		irr::core::dimension2d< irr::u32 > windowSizeDefault;
		bool wStringToBool( std::wstring choice );
};

#endif // SETTINGSMANAGER_H
