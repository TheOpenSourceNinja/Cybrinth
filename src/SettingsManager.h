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
		
		bool prefIsTrue( std::wstring choice );
		
		void readPrefs();
		
		void savePrefs();
		void setPointers( irr::IrrlichtDevice* newDevice, MainGame* newMainGame, MazeManager* newMazeManager, NetworkManager* newNetwork, SpellChecker* newSpellChecker, SystemSpecificsManager* newSystem ); //Must be called before readPrefs!
		
		bool allowSmallSize;
		
		bool backgroundAnimations;
		uint_fast16_t bitsPerPixel;
		AI::algorithm_t botAlgorithm;
		uint_fast16_t botMovementDelay;
		bool botsKnowSolution;
		
		bool debug;
		irr::video::E_DRIVER_TYPE driverType;
		
		bool fullscreen;
		
		bool isServer;
		
		bool markTrails;
		uint_fast32_t minHeight;
		uint_fast32_t minWidth;
		uint_fast16_t musicVolume;
		
		uint_fast8_t numBots;
		uint_fast8_t numPlayers;
		
		bool playMusic;
		
		bool showBackgrounds;
		
		irr::core::dimension2d< irr::u32 > windowSize;
		
		bool vsync;
	
	private:
		irr::IrrlichtDevice* device;
		
		MainGame* mainGame;
		MazeManager* mazeManager;
		
		NetworkManager* network;
		
		SpellChecker* spellChecker;
		SystemSpecificsManager* system; // Flawfinder: ignore
};

#endif // SETTINGSMANAGER_H
