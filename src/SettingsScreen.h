#ifndef SETTINGSSCREEN_H
#define SETTINGSSCREEN_H

#include <irrlicht/irrlicht.h>

#include "SettingsManager.h"

class MainGame;

class SettingsScreen : public irr::IEventReceiver {
	public:
		SettingsScreen();
		virtual ~SettingsScreen();
		
		irr::video::SColor backgroundColor; //Used by MainGame when it calls driver->beginScene()
		
		void changeToSettingsScreen();
		void changeFromSettingsScreen();
		
		void draw( irr::IrrlichtDevice* device );
		
		bool OnEvent( const irr::SEvent& event );
		
		void setButtonFont( irr::gui::IGUIFont* newTextFont );
		void setPointers( MainGame* newMainGame, irr::IrrlichtDevice* newDevice, irr::gui::IGUIFont* newButtonFont, irr::gui::IGUIFont* newTextFont, SettingsManager* newSettingsManager );
		void setTextFont( irr::gui::IGUIFont* newTextFont );
		void setupIconsAndStuff();
		
	protected:
	private:
		void backToMenu();
		irr::gui::IGUIFont* buttonFont;
		
		irr::gui::IGUIButton* cancelButton;
		
		irr::IrrlichtDevice* device;
		irr::video::IVideoDriver* driver;
		
		irr::gui::IGUIEnvironment* environment;
		
		enum gui_id_t : uint_fast8_t { CANCEL_ID, OK_ID, RESET_TO_DEFAULTS_ID, UNDO_CHANGES_ID, PLAY_MUSIC_CHECKBOX_ID, DO_NOT_USE_ID };
		
		MainGame* mainGame;
		
		irr::gui::IGUIButton* okButton;
		
		irr::gui::IGUICheckBox* playMusicCheckBox;
		
		void resetChangedSettings();
		irr::gui::IGUIButton* resetToDefaultsButton;
		void resetToDefaultSettings();
		irr::core::stringw restartNotice;
		
		void saveSettings();
		bool settingsChanged; //If no settings were changed, don't bother saving them.
		SettingsManager* settingsManager;
		irr::gui::IGUISkin* skin;
		
		irr::gui::IGUIFont* textFont;
		
		irr::gui::IGUIButton* undoChangesButton;
};

#endif // SETTINGSSCREEN_H
