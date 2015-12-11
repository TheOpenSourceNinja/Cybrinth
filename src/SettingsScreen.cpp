#include "colors.h"
#include "SettingsScreen.h"

SettingsScreen::SettingsScreen() {
	//ctor
}

SettingsScreen::~SettingsScreen() {
	//dtor
}

void SettingsScreen::draw( irr::IrrlichtDevice* device ) {
	irr::core::stringw restartNotice( L"Some or all of these settings will only take effect when the program is closed & reopened." );
	irr::core::dimension2d< irr::u32 > tempDimensions = textFont->getDimension( restartNotice.c_str() );
	irr::core::rect< irr::s32 > tempRectangle( 0, 0, tempDimensions.Width + 0, tempDimensions.Height + 0 );
	textFont->draw( restartNotice, tempRectangle, WHITE, true, true );
}

void SettingsScreen::setTextFont( irr::gui::IGUIFont* newTextFont ) {
	textFont = newTextFont;
}