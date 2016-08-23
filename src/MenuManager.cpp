/**
 * @file
 * @author James Dearing <dearingj@lifetime.oregonstate.edu>
 * 
 * @section LICENSE
 * Copyright © 2012-2016.
 * This file is part of Cybrinth.
 *
 * Cybrinth is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * Cybrinth is distributed 'as is' in the hope that it will be fun, but WITHOUT ANY WARRANTY; without even the implied warranty of TITLE, MERCHANTABILITY, COMPLETE DESTRUCTION OF EVIL MONSTERS, or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License along with Cybrinth. If not, see <http://www.gnu.org/licenses/>.
 * 
 * @section DESCRIPTION
 * The MenuManager class is responsible for showing the in-game pause menu, and telling the MainGame how to proceed when a menu item is clicked.
 */

#include "MenuManager.h"
#include "MainGame.h"

MenuManager::MenuManager() {
	mainGame = nullptr;
	
	{
		MenuOption temp;
		temp.setType( nullptr, MenuOption::NEW_MAZE );
		nextMaze = options.size();
		options.push_back( temp );
	}
	{
		MenuOption temp;
		temp.setType( nullptr, MenuOption::RESTART_MAZE );
		restartMaze = options.size();
		options.push_back( temp );
	}
	{
		MenuOption temp;
		temp.setType( nullptr, MenuOption::JOIN_SERVER );
		joinServer = options.size();
		options.push_back( temp );
	}
	{
		MenuOption temp;
		temp.setType( nullptr, MenuOption::LOAD_MAZE );
		loadMaze = options.size();
		options.push_back( temp );
	}
	{
		MenuOption temp;
		temp.setType( nullptr, MenuOption::SAVE_MAZE );
		saveMaze = options.size();
		options.push_back( temp );
	}
	{
		MenuOption temp;
		temp.setType( nullptr, MenuOption::SETTINGS );
		settings = options.size();
		options.push_back( temp );
	}
	{
		MenuOption temp;
		temp.setType( nullptr, MenuOption::EXIT_GAME );
		exitGame = options.size();
		options.push_back( temp );
	}
	{
		MenuOption temp;
		temp.setType( nullptr, MenuOption::BACK_TO_GAME );
		backToGame = options.size();
		options.push_back( temp );
	}
	{
		MenuOption temp;
		temp.setType( nullptr, MenuOption::FREEDOM );
		freedom = options.size();
		options.push_back( temp );
	}
	highlightedOption = backToGame;
}

MenuManager::~MenuManager() {
	//dtor
}

void MenuManager::draw( irr::IrrlichtDevice* device ) {
	for( decltype( options.size() ) o = 0; o < options.size(); ++o ) {
		options.at( o ).draw( device );
	}
}

/**
 * Given screen coordinates (of the mouse pointer, for example), figure out which menu item is selected.
 * Arguments:
 * x: the X coordinate of a point
 * y: the Y coordinate of a point
 **/
void MenuManager::findHighlights( int_fast32_t x, int_fast32_t y ) {
	
	decltype( options.size() ) currentHighlight = 0;
	for( decltype( options.size() ) o = 0; o < options.size(); ++o ) {
		if( options.at( o ).highlighted ) {
			currentHighlight = o;
			break;
		}
	}
	
	uint_fast64_t minDistance = UINT_FAST64_MAX;
	for( decltype( options.size() ) o = 0; o < options.size(); ++o ) {
		decltype( minDistance ) distance = sqrt( pow( abs( ( int_fast64_t ) x - ( int_fast64_t ) options.at( o ).getMiddleX() ), 2 ) + pow( abs( ( int_fast64_t ) y - ( int_fast64_t ) options.at( o ).getMiddleY() ), 2 ) );
		if( distance < minDistance ) {
			options.at( currentHighlight ).highlighted = false;
			currentHighlight = o;
			options.at( currentHighlight ).highlighted = true;
			minDistance = distance;
		}
	}
}

/**
 * Makes each menuOption load its texture.
 * Arguments:
 * device: A pointer to the Irrlicht device
 **/
void MenuManager::loadIcons( irr::IrrlichtDevice* device ) {
	for( uint_fast8_t i = 0; i < options.size(); ++i ) {
		options.at( i ).loadTexture( device );
	}
}

/**
* Should be called only by OnEvent() or processControls().
* Arguments:
* A pointer to the MainGame.
*/
void MenuManager::processSelection() {
	if( mainGame == nullptr or mainGame == NULL ) {
		return;
	}
	
	if( options.at( exitGame ).highlighted ) {
		//mainGame->setExitConfirmation( mainGame->gui->addMessageBox( L"Exit?", L"Are you sure you want to exit?", true, ( irr::gui::EMBF_YES bitor irr::gui::EMBF_NO ) ) );
		mainGame->displayExitConfirmation();
	} else if( options.at( loadMaze ).highlighted ) {
		//mainGame->setFileChooser( mainGame->gui->addFileOpenDialog( L"Select a Maze", true, 0, -1, true ) );
		mainGame->showLoadMazeDialog();
	} else if( options.at( saveMaze ).highlighted ) {
		mainGame->showSaveMazeDialog();
	} else if( options.at( nextMaze ).highlighted ) {
		if( mainGame->network.getIsServer() ) {
			mainGame->newMaze();
		} else {
			if( mainGame->getDebugStatus() ) {
				std::wcout << L"Cannot create new maze: we're a client, not a server." << std::endl;
			}
		}
	} else if( options.at( restartMaze ).highlighted ) {
		mainGame->newMaze( mainGame->getRandomSeed() );
	} else if( options.at( backToGame ).highlighted ) {
		mainGame->currentScreen = MainGame::MAINSCREEN;
	} else if( options.at( freedom ).highlighted ) {
		std::wstring message = stringConverter.toStdWString( PACKAGE_NAME );
		message += L" is copyright 2012-2016 by James Dearing. Licensed under the GNU Affero General Public License, version 3.0 or (at your option) any later version, as published by the Free Software Foundation. See the file \"COPYING\" or https://www.gnu.org/licenses/agpl.html.\n\nThis means you're free to do what you want with this game: mod it, give copies to friends, sell it if you want. Whatever. It's Free software, Free as in Freedom. You should have received the program's source code with this copy; if you don't have it, you can get it from ";
		message += stringConverter.toStdWString( PACKAGE_URL );
		message += L".\n\n";
		message += stringConverter.toStdWString( PACKAGE_NAME );
		message += L" is distributed 'as is' in the hope that it will be fun, but WITHOUT ANY WARRANTY; without even the implied warranty of TITLE, MERCHANTABILITY, COMPLETE DESTRUCTION OF EVIL MONSTERS, or FITNESS FOR A PARTICULAR PURPOSE.";
		message += L"\n\n";
		message += L"Cybrinth uses a copy of the RakNet networking library. RakNet is owned by Oculus VR, Inc. It is available under the terms of a modified 2-clause BSD license. See src/RakNet/LICENSE for the copyright license, and src/RakNet/PATENTS for patent information, and src/RakNet/ORIGIN for the URL from which this copy of RakNet was downloaded; copies of LICENSE and PATENTS can also be obtained from there. The creators of Cybrinth claim no connection to RakNet or Oculus VR, and no responsibility for any bugs or features in RakNet.";
		mainGame->gui->addMessageBox( L"Freedom", message.c_str() );
	} else if( options.at( settings ).highlighted ) {
		mainGame->currentScreen = MainGame::SETTINGSSCREEN;
		mainGame->settingsScreen.changeToSettingsScreen();
	} else if( options.at( joinServer ).highlighted ) {
		//TODO: Add stuff for connecting to server here
	}
}

void MenuManager::scrollSelection( bool up ) {
	decltype( options.size() ) currentHighlight = 0;
	for( decltype( options.size() ) o = 0; o < options.size(); ++o ) {
		if( options.at( o ).highlighted ) {
			currentHighlight = o;
			break;
		}
	}
	
	decltype( options.size() ) nextHighlight;
	
	if( up ) {
		if( currentHighlight > 0 ) {
			nextHighlight = currentHighlight - 1;
		} else {
			nextHighlight = options.size() - 1;
		}
		
	} else {
		nextHighlight = ( currentHighlight + 1 ) % options.size();
		
	}
	
	options.at( currentHighlight ).highlighted = false;
	options.at( nextHighlight ).highlighted = true;
}

void MenuManager::setFontAndResizeIcons( irr::IrrlichtDevice* device, irr::gui::IGUIFont* font ) {
	for( decltype( options.size() ) o = 0; o < options.size(); ++o ) {
		options.at( o ).setFontAndResizeIcon( device, font );
	}
}


void MenuManager::setMainGame( MainGame* mg ) {
	mainGame = mg;
	for( decltype( options.size() ) o = 0; o < options.size(); ++o ) {
		options.at( o ).setMainGame( mg );
	}
}

/**
 * Places the menu options at the correct locations on screen.
 */
void MenuManager::setPositions( uint_fast32_t windowHeight ) {
	try {
		for( decltype( options.size() ) o = 0; o < options.size(); ++o ) {
			options.at( o ).setY( o * windowHeight / options.size() );
		}
	} catch( std::exception &e ) {
		std::wcerr << L"Error in MazeManager::setPositions(): " << e.what() << std::endl;
	}
}
