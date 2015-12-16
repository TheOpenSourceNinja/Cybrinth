#ifndef SETTINGSSCREEN_H
#define SETTINGSSCREEN_H

#include <irrlicht/irrlicht.h>

#include "MenuOption.h"
#include "SettingsManager.h"

class MainGame;

class SettingsScreen {
	public:
		SettingsScreen();
		virtual ~SettingsScreen();
		
		void draw( irr::IrrlichtDevice* device );
		
		void findHighlights( irr::s32 x, irr::s32 y );
		
		void handleMouseEvents( const irr::SEvent& event );
		
		void processSelection();
		
		void setButtonFont( irr::gui::IGUIFont* newTextFont );
		void setPointers( MainGame* newMainGame, irr::IrrlichtDevice* newDevice, irr::gui::IGUIFont* newButtonFont, irr::gui::IGUIFont* newTextFont, SettingsManager* newSettingsManager );
		void setTextFont( irr::gui::IGUIFont* newTextFont );
		void setupIconsAndStuff();
		
	protected:
	private:
		void backToMenu();
		irr::gui::IGUIFont* buttonFont;
		
		MenuOption cancel;
		
		irr::IrrlichtDevice* device;
		
		MainGame* mainGame;
		
		MenuOption ok;
		
		void resetChangedSettings();
		MenuOption resetToDefaults;
		void resetToDefaultSettings();
		irr::core::stringw restartNotice;
		
		void saveSettings();
		bool settingsChanged; //If no settings were changed, don't bother saving them.
		SettingsManager* settingsManager;
		
		irr::gui::IGUIFont* textFont;
		
		MenuOption undoChanges;
};

#endif // SETTINGSSCREEN_H
