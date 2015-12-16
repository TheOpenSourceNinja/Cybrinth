#include "colors.h"
#include "SettingsScreen.h"
#include "MainGame.h"
#include <iostream>

SettingsScreen::SettingsScreen() {
	restartNotice = L"Some of these settings will only take effect when the game is closed & reopened.";
	setPointers( nullptr, nullptr, nullptr, nullptr, nullptr );
}

SettingsScreen::~SettingsScreen() {
	//dtor
}

void SettingsScreen::backToMenu() {
	mainGame->currentScreen = MainGame::MENUSCREEN;
}

void SettingsScreen::draw( irr::IrrlichtDevice* device ) {
	irr::core::dimension2d< irr::u32 > tempDimensions = textFont->getDimension( restartNotice.c_str() );
	irr::core::rect< irr::s32 > tempRectangle( 0, 0, tempDimensions.Width + 0, tempDimensions.Height + 0 );
	textFont->draw( restartNotice, tempRectangle, WHITE, true, true );
	cancel.draw( device );
	ok.draw( device );
	undoChanges.draw( device );
	resetToDefaults.draw( device );
}

void SettingsScreen::findHighlights( irr::s32 x, irr::s32 y ) {
	cancel.highlighted = cancel.contains( x, y );
	ok.highlighted = ok.contains( x, y );
	undoChanges.highlighted = undoChanges.contains( x, y );
	resetToDefaults.highlighted = resetToDefaults.contains( x, y );
}

void SettingsScreen::handleMouseEvents( const irr::SEvent& event ) {
	switch( event.MouseInput.Event ) {
		case irr::EMIE_MOUSE_MOVED: {
			findHighlights( event.MouseInput.X, event.MouseInput.Y );
			break;
		}
		case irr::EMIE_LMOUSE_PRESSED_DOWN: {
			processSelection();
			break;
		}
		default: {
			break;
		}
	}
}

void SettingsScreen::processSelection() {
	if( cancel.highlighted ) {
		std::wcout << L"Cancel button pressed" << std::endl;
		resetChangedSettings();
		backToMenu();
	} else if( ok.highlighted ) {
		std::wcout << L"OK button pressed" << std::endl;
		if( settingsChanged ) {
			saveSettings();
		}
		backToMenu();
	} else if( undoChanges.highlighted ) {
		std::wcout << L"Undo changes button pressed" << std::endl;
		resetChangedSettings();
	} else if( resetToDefaults.highlighted ) {
		std::wcout << L"Reset to defaults button pressed" << std::endl;
		resetToDefaultSettings();
	}
}

void SettingsScreen::resetChangedSettings() {
	settingsChanged = false;
	settingsManager->readPrefs();
}

void SettingsScreen::resetToDefaultSettings() {
	std::wcerr << L"SettingsScreen::resetToDefaultSettings() not implemented yet." << std::endl;
}

void SettingsScreen::saveSettings() {
	settingsManager->savePrefs();
}

void SettingsScreen::setButtonFont( irr::gui::IGUIFont* newButtonFont ) {
	buttonFont = newButtonFont;
}

void SettingsScreen::setPointers( MainGame* newMainGame, irr::IrrlichtDevice* newDevice, irr::gui::IGUIFont* newButtonFont, irr::gui::IGUIFont* newTextFont, SettingsManager* newSettingsManager ) {
	mainGame = newMainGame;
	device = newDevice;
	setButtonFont( newButtonFont );
	setTextFont( newTextFont );
	settingsManager = newSettingsManager;
}

void SettingsScreen::setTextFont( irr::gui::IGUIFont* newTextFont ) {
	textFont = newTextFont;
}

void SettingsScreen::setupIconsAndStuff() {
	cancel.setX( 0 );
	
	if( buttonFont != NULL && buttonFont != nullptr ) {
		cancel.setY( textFont->getDimension( restartNotice.c_str() ).Height + 5 );
	} else {
		cancel.setY( 5 );
	}
	cancel.setType( device, MenuOption::CANCEL );
	cancel.setFontAndResizeIcon( device, buttonFont );
	cancel.loadTexture( device );
	
	ok.setY( cancel.getY() );
	ok.setX( cancel.getX() + cancel.getWidth() + 1 );
	ok.setType( device, MenuOption::OK );
	ok.setFontAndResizeIcon( device, buttonFont );
	ok.loadTexture( device );
	
	undoChanges.setY( cancel.getY() );
	undoChanges.setX( ok.getX() + ok.getWidth() + 1 );
	undoChanges.setType( device, MenuOption::UNDO_CHANGES );
	undoChanges.setFontAndResizeIcon( device, buttonFont );
	undoChanges.loadTexture( device );
	
	resetToDefaults.setY( cancel.getY() );
	resetToDefaults.setX( undoChanges.getX() + undoChanges.getWidth() + 1 );
	resetToDefaults.setType( device, MenuOption::RESET_TO_DEFAULTS );
	resetToDefaults.setFontAndResizeIcon( device, buttonFont );
	resetToDefaults.loadTexture( device );
}