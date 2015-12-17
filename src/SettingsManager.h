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
		
		bool backgroundAnimations;
		uint_fast16_t bitsPerPixel;
		AI::algorithm_t botAlgorithm;
		uint_fast16_t botMovementDelay;
		bool botsKnowSolution;
		
		bool debug;
		irr::video::E_DRIVER_TYPE driverType;
		
		bool fullscreen;
		
		bool getPlayMusic();
		
		bool isServer;
		
		bool markTrails;
		uint_fast32_t minHeight;
		uint_fast32_t minWidth;
		uint_fast16_t musicVolume;
		
		uint_fast16_t networkPort;
		uint_fast8_t numBots;
		uint_fast8_t numPlayers;
		
		void resetToDefaults();
		
		void setPlayMusic( bool newSetting );
		bool showBackgrounds;
		
		irr::core::dimension2d< irr::u32 > windowSize;
		
		bool vsync;
	
	private:
		std::wstring boolToWString( bool input );
		
		irr::IrrlichtDevice* device;
		std::vector< std::wstring > driverTypes = { L"opengl", L"direct3d9", L"direct3d8", L"burning's video", L"software", L"null" };
		enum driver_t : uint_fast8_t { OPENGL = 0, DIRECT3D9 = 1, DIRECT3D8 = 2, BURNINGS = 3, SOFTWARE = 4, DRIVERNULL = 5, DRIVERDO_NOT_USE = 255 };
		
		MainGame* mainGame;
		MazeManager* mazeManager;
		
		NetworkManager* network;
		
		bool playMusic;
		//possiblePrefs is used by both readPrefs() and savePrefs(). If this gets changed, be sure to update both functions!
		std::vector< std::wstring > possiblePrefs = { L"bots' solving algorithm", L"volume", L"number of bots", L"show backgrounds",
									L"fullscreen", L"mark player trails", L"debug", L"bits per pixel", L"wait for vertical sync", L"driver type", L"number of players",
									L"window size", L"play music", L"network port", L"always server", L"bots know the solution", L"bot movement delay", L"hide unseen maze areas", L"background animations" };
		//Each item in pref_t, except the last one, must match with an item in possiblePrefs.
		enum pref_t : uint_fast8_t { ALGORITHM = 0, VOLUME = 1, NUMBOTS = 2, SHOW_BACKGROUNDS = 3, FULLSCREEN = 4, MARK_TRAILS = 5, DEBUG = 6, BPP = 7, VSYNC = 8, DRIVER_TYPE = 9, NUMPLAYERS = 10,
									WINDOW_SIZE = 11, PLAY_MUSIC = 12, NETWORK_PORT = 13, ALWAYS_SERVER = 14, SOLUTION_KNOWN = 15, MOVEMENT_DELAY = 16, HIDE_UNSEEN = 17, BACKGROUND_ANIMATIONS = 18, DO_NOT_USE = 255 };
		
		SpellChecker* spellChecker;
		SystemSpecificsManager* system; // Flawfinder: ignore
		
		bool wStringToBool( std::wstring choice );
};

#endif // SETTINGSMANAGER_H
