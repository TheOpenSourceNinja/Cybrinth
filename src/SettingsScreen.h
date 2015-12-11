#ifndef SETTINGSSCREEN_H
#define SETTINGSSCREEN_H

#include <irrlicht/irrlicht.h>

class SettingsScreen
{
	public:
		SettingsScreen();
		virtual ~SettingsScreen();
		
		void draw( irr::IrrlichtDevice* device );
		
		void setTextFont( irr::gui::IGUIFont* newTextFont );
	protected:
	private:
		irr::gui::IGUIFont* textFont;
};

#endif // SETTINGSSCREEN_H
