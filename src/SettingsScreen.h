#ifndef SETTINGSSCREEN_H
#define SETTINGSSCREEN_H

#include <irrlicht/irrlicht.h>

#include "MenuOption.h"

class MainGame;

class SettingsScreen {
	public:
		SettingsScreen();
		virtual ~SettingsScreen();
		
		void draw( irr::IrrlichtDevice* device );
		
		void findHighlights( irr::s32 x, irr::s32 y );
		
		void handleMouseEvents( const irr::SEvent& event );
		
		void processSelection();
		
		void setPointers( MainGame* newMainGame, irr::IrrlichtDevice* newDevice, irr::gui::IGUIFont* newFont );
		void setTextFont( irr::gui::IGUIFont* newTextFont );
		void setupIconsAndStuff();
		
	protected:
	private:
		MenuOption cancel;
		
		irr::IrrlichtDevice* device;
		
		MainGame* mainGame;
		
		MenuOption ok;
		
		MenuOption resetToDefaults;
		irr::core::stringw restartNotice;
		
		irr::gui::IGUIFont* textFont;
		
		MenuOption undoChanges;
};

#endif // SETTINGSSCREEN_H
