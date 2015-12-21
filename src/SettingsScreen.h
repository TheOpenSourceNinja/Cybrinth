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
		irr::gui::IGUICheckBox* backgroundAnimationsCheckBox;
		void backToMenu();
		irr::gui::IGUICheckBox* bpp16CheckBox;
		irr::gui::IGUICheckBox* bpp32CheckBox;
		irr::gui::IGUIStaticText* bppText;
		irr::gui::IGUICheckBox* burningsSoftwareCheckBox;
		irr::gui::IGUIFont* buttonFont;
		
		irr::gui::IGUIButton* cancelButton;
		
		irr::IrrlichtDevice* device;
		irr::gui::IGUICheckBox* direct3D8CheckBox;
		irr::gui::IGUICheckBox* direct3D9CheckBox;
		irr::video::IVideoDriver* driver;
		irr::gui::IGUIStaticText* driverTypeText;
		
		irr::gui::IGUIEnvironment* environment;
		
		irr::gui::IGUICheckBox* fullscreenCheckBox;
		
		irr::gui::IGUITab* graphicsTab;
		enum gui_id_t : uint_fast8_t { MARK_TRAILS_CHECKBOX_ID, BACKGROUND_ANIMATIONS_CHECKBOX_ID, SHOW_BACKGROUNDS_CHECKBOX_ID, WINDOW_HEIGHT_SPINBOX_ID, WINDOW_WIDTH_SPINBOX_ID, DIRECT3D8_CHECKBOX_ID, DIRECT3D9_CHECKBOX_ID, OPENGL_CHECKBOX_ID, IRRLICHTSOFTWARE_CHECKBOX_ID, BURNINGSSOFTWARE_CHECKBOX_ID, CANCEL_ID, OK_ID, RESET_TO_DEFAULTS_ID, UNDO_CHANGES_ID, TAB_CONTROL_ID, PLAY_MUSIC_CHECKBOX_ID, VOLUME_BAR_ID, FULLSCREEN_CHECKBOX_ID, BPP16_CHECKBOX_ID, BPP32_CHECKBOX_ID, VSYNC_CHECKBOX_ID, DO_NOT_USE_ID }; //I made these an enum because I'm too lazy to manually assign values to each one, so why not make the compiler do it?
		
		irr::gui::IGUICheckBox* irrlichtSoftwareCheckBox;
		
		MainGame* mainGame;
		irr::gui::IGUICheckBox* markTrailsCheckBox;
		irr::gui::IGUITab* miscTab;
		irr::gui::IGUITab* multiplayerTab;
		
		irr::gui::IGUIButton* okButton;
		irr::gui::IGUICheckBox* openGLCheckBox;
		
		irr::gui::IGUICheckBox* playMusicCheckBox;
		
		void resetChangedSettings();
		irr::gui::IGUIButton* resetToDefaultsButton;
		void resetToDefaultSettings();
		irr::core::stringw restartNotice;
		
		void saveSettings();
		bool settingsChanged; //If no settings were changed, don't bother saving them.
		SettingsManager* settingsManager;
		irr::gui::IGUICheckBox* showBackgroundsCheckBox;
		irr::gui::IGUISkin* skin;
		irr::gui::IGUITab* soundTab;
		
		irr::gui::IGUITabControl* tabControl;
		irr::gui::IGUIFont* textFont;
		
		irr::gui::IGUIButton* undoChangesButton;
		
		irr::gui::IGUIScrollBar* volumeBar;
		irr::gui::IGUIStaticText* volumeText;
		irr::gui::IGUICheckBox* vsyncCheckBox;
		
		irr::gui::IGUIStaticText* windowSizeText;
		irr::gui::IGUISpinBox* windowHeightSpinBox;
		irr::gui::IGUISpinBox* windowWidthSpinBox;
};

#endif // SETTINGSSCREEN_H
