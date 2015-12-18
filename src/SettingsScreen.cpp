#include "colors.h"
#include "CustomException.h"
#include "SettingsScreen.h"
#include "MainGame.h"
#include <iostream>

SettingsScreen::SettingsScreen() {
	restartNotice = L"Some of these settings will only take effect when the game is closed & reopened.";
	setPointers( nullptr, nullptr, nullptr, nullptr, nullptr );
	backgroundColor = LIGHTGRAY;
}

SettingsScreen::~SettingsScreen() {
	//dtor
}

void SettingsScreen::backToMenu() {
	mainGame->currentScreen = MainGame::MENUSCREEN;
	changeFromSettingsScreen();
}

void SettingsScreen::changeToSettingsScreen() {
	try {
		if( device == nullptr or device == NULL ) {
			CustomException e( L"device is null" );
			throw e;
		}
		
		driver = device->getVideoDriver();
		environment = device->getGUIEnvironment();
		skin = environment->getSkin();
		
		if( textFont != nullptr ) {
			skin->setFont( textFont );
		}
		
		{
			auto textDimensions = skin->getFont()->getDimension( restartNotice.c_str() );
			if( textDimensions.Width > settingsManager->windowSize.Width ) {
				textDimensions.Width = settingsManager->windowSize.Width;
				textDimensions.Height = textDimensions.Height * 2;
			}
			auto textRectangle = irr::core::rect< irr::s32 >( 0, 0, textDimensions.Width, textDimensions.Height );
			environment->addStaticText( restartNotice.c_str(), textRectangle );
			
			auto buttonsY = textDimensions.Height + 1;
			decltype( buttonsY ) tabsY;
			
			{
				auto buttonWidth = settingsManager->windowSize.Width / 4;
				auto buttonHeight = 30; //Not sure how tall these buttons should be; this is just a guess.
				
				auto cancelButtonRectangle = irr::core::rect< irr::s32 >( 0, buttonsY, 0 + buttonWidth, buttonsY + buttonHeight );
				cancelButton = environment->addButton( cancelButtonRectangle, 0, CANCEL_ID, L"Cancel" );
				
				auto okButtonRectangle = irr::core::rect< irr::s32 >( cancelButtonRectangle.LowerRightCorner.X , cancelButtonRectangle.UpperLeftCorner.Y, cancelButtonRectangle.LowerRightCorner.X + buttonWidth, cancelButtonRectangle.UpperLeftCorner.Y + buttonHeight );
				okButton = environment->addButton( okButtonRectangle, 0, OK_ID, L"OK" );
				
				auto resetToDefaultsButtonRectangle = irr::core::rect< irr::s32 >( okButtonRectangle.LowerRightCorner.X , okButtonRectangle.UpperLeftCorner.Y, okButtonRectangle.LowerRightCorner.X + buttonWidth, okButtonRectangle.UpperLeftCorner.Y + buttonHeight );
				resetToDefaultsButton = environment->addButton( resetToDefaultsButtonRectangle, 0, RESET_TO_DEFAULTS_ID, L"Reset to defaults" );
				
				auto undoChangesButtonRectangle = irr::core::rect< irr::s32 >( resetToDefaultsButtonRectangle.LowerRightCorner.X , resetToDefaultsButtonRectangle.UpperLeftCorner.Y, resetToDefaultsButtonRectangle.LowerRightCorner.X + buttonWidth, resetToDefaultsButtonRectangle.UpperLeftCorner.Y + buttonHeight );
				undoChangesButton = environment->addButton( undoChangesButtonRectangle, 0, UNDO_CHANGES_ID, L"Undo changes" );
				
				tabsY = buttonsY + buttonHeight + 1;
			}
			
			{
				irr::core::rect< irr::s32 > tabControlRectangle( 0, tabsY, settingsManager->windowSize.Width, settingsManager->windowSize.Height );
				tabControl = environment->addTabControl( tabControlRectangle, 0, false, true, TAB_CONTROL_ID );
				
				soundTab = tabControl->addTab( L"Sound" );
				graphicsTab = tabControl->addTab( L"Graphics" );
				multiplayerTab = tabControl->addTab( L"Multiplayer" );
				miscTab = tabControl->addTab( L"Miscellaneous" );
			}
			
			{
				decltype( settingsManager->windowSize.Height ) itemY = 0;
				
				irr::core::stringw buttonText = L"Play music";
				auto buttonTextDimensions = environment->getSkin()->getFont()->getDimension( buttonText.c_str() );
				auto MusicBoxRectangle = irr::core::rect< irr::s32 >( 0, itemY, buttonTextDimensions.Width + 30, itemY + buttonTextDimensions.Height ); //I measured the width of a checkbox as 18 pixels, plus an additional 6 pixels of space between that and the text. Then I upped it to 30 just to leave some room. Feel free to up it some more.
				playMusicCheckBox = environment->addCheckBox( settingsManager->getPlayMusic(), MusicBoxRectangle, soundTab, PLAY_MUSIC_CHECKBOX_ID, buttonText.c_str() );
				
				irr::core::stringw volumeTextString = L"Volume";
				auto volumeTextDimensions = skin->getFont()->getDimension( volumeTextString.c_str() );
				auto volumeTextRectangle = irr::core::rect< irr::s32 >( MusicBoxRectangle.LowerRightCorner.X + 1, itemY, MusicBoxRectangle.LowerRightCorner.X + 1 + volumeTextDimensions.Width, itemY + volumeTextDimensions.Height );
				volumeText = environment->addStaticText( volumeTextString.c_str(), volumeTextRectangle, false, true, soundTab );
				volumeText->setEnabled( playMusicCheckBox->isChecked() );
				
				auto volumeBarRectangle = irr::core::rect< irr::s32 >( volumeTextRectangle.LowerRightCorner.X + 1, itemY, settingsManager->windowSize.Width, volumeTextRectangle.LowerRightCorner.Y );
				volumeBar = environment->addScrollBar( true, volumeBarRectangle, soundTab, VOLUME_BAR_ID );
				volumeBar->setEnabled( playMusicCheckBox->isChecked() );
				volumeBar->setMax( 100 );
				volumeBar->setMin( 0 );
				volumeBar->setPos( settingsManager->getMusicVolume() );
				
				itemY += buttonTextDimensions.Height + 1;
			}
			
			{
				decltype( settingsManager->windowSize.Height ) itemY = 0;
				
				irr::core::stringw checkboxText = L"Fullscreen";
				auto cbTextDimensions = environment->getSkin()->getFont()->getDimension( checkboxText.c_str() );
				auto fullscreenBoxRectangle = irr::core::rect< irr::s32 >( 0, itemY, cbTextDimensions.Width + 30, itemY + cbTextDimensions.Height );
				fullscreenCheckBox = environment->addCheckBox( settingsManager->fullscreen, fullscreenBoxRectangle, graphicsTab, FULLSCREEN_CHECKBOX_ID, checkboxText.c_str() );
			}
		}
	} catch( std::exception e ) {
		std::wcerr << L"Error in SettingsScreen::changeToSettingsScreen(): " << e.what() << std::endl;
	}
}

void SettingsScreen::changeFromSettingsScreen() {
	environment->clear();
}

void SettingsScreen::draw( irr::IrrlichtDevice* device ) {
	/*irr::core::dimension2d< irr::u32 > tempDimensions = textFont->getDimension( restartNotice.c_str() );
	irr::core::rect< irr::s32 > tempRectangle( 0, 0, tempDimensions.Width + 0, tempDimensions.Height + 0 );
	textFont->draw( restartNotice, tempRectangle, WHITE, true, true );
	cancel.draw( device );
	ok.draw( device );
	undoChanges.draw( device );
	resetToDefaults.draw( device );*/
	environment->drawAll();
}

bool SettingsScreen::OnEvent( const irr::SEvent& event ) {
	try {
		switch( event.EventType ) {
			case irr::EET_GUI_EVENT: {
				auto id = event.GUIEvent.Caller->getID();
				
				switch( event.GUIEvent.EventType ) {
					case irr::gui::EGET_BUTTON_CLICKED: {
						switch( id ) {
							case CANCEL_ID: {
								resetChangedSettings();
								backToMenu();
								return true;
							}
							case OK_ID: {
								if( settingsChanged ) {
									saveSettings();
								}
								backToMenu();
								return true;
							}
							case RESET_TO_DEFAULTS_ID: {
								resetToDefaultSettings();
								return true;
							}
							case UNDO_CHANGES_ID: {
								resetChangedSettings();
								return true;
							}
							default: {
								CustomException e( L"Unhandled button ID" );
								throw( e );
							}
						}
						
						break;
					}
					case irr::gui::EGET_CHECKBOX_CHANGED: {
						switch( id ) {
							case PLAY_MUSIC_CHECKBOX_ID: {
								settingsManager->setPlayMusic( playMusicCheckBox->isChecked() );
								volumeText->setEnabled( playMusicCheckBox->isChecked() );
								volumeBar->setEnabled( playMusicCheckBox->isChecked() );
								break;
							}
							case FULLSCREEN_CHECKBOX_ID: {
								settingsManager->fullscreen = fullscreenCheckBox->isChecked();
								break;
							}
							default: {
								CustomException e( L"Unhandled checkbox ID" );
								throw( e );
							}
						}
						
						break;
					}
					case irr::gui::EGET_SCROLL_BAR_CHANGED: {
						switch( id ) {
							case VOLUME_BAR_ID: {
								settingsManager->setMusicVolume( volumeBar->getPos() );
								break;
							}
							default: {
								CustomException e( L"Unhandled scroll bar ID" );
								throw( e );
							}
						}
						break;
					}
					default: {
						CustomException e( L"Unhandled event.GUIEvent.EventType" );
						throw( e );
					}
				}
				
				break;
			}
			default: {
				CustomException e( L"Unhandled event.EventType" );
				throw( e );
			}
		}
	} catch( CustomException e ) {
		std::wcerr << L"Error in SettingsScreen::OnEvent(): " << e.what() << std::endl;
		return false;
	}
}

/*void SettingsScreen::processSelection() {
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
}*/

void SettingsScreen::resetChangedSettings() {
	settingsChanged = false;
	settingsManager->readPrefs();
}

void SettingsScreen::resetToDefaultSettings() {
	std::wcerr << L"SettingsScreen::resetToDefaultSettings() not fully implemented yet." << std::endl;
	settingsChanged = true;
	settingsManager->resetToDefaults();
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
	/*cancel.setX( 0 );
	
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
	resetToDefaults.loadTexture( device );*/
}