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
		
		void readPrefs();
		
		void savePrefs();
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
		bool getPlayMusic();
		irr::core::dimension2d< irr::u32 > getWindowSize();
		
		bool isServer;
		bool isServerDefault;
		
		bool markTrails;
		bool markTrailsDefault;
		
		uint_fast16_t networkPort;
		uint_fast16_t networkPortDefault;
		uint_fast8_t numBots;
		uint_fast8_t numBotsDefault;
		uint_fast8_t numPlayers;
		uint_fast8_t numPlayersDefault;
		
		void resetToDefaults();
		
		void setBitsPerPixel( uint_fast8_t newBPP );
		void setFullscreenResolution( irr::core::dimension2d< irr::u32 > newResolution );
		void setMusicVolume( uint_fast8_t newVolume );
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
