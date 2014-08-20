/**
 * @file
 * @author James Dearing <dearingj@lifetime.oregonstate.edu>
 * 
 * @section LICENSE
 * Copyright © 2012-2014.
 * This file is part of Cybrinth.
 *
 * Cybrinth is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * Cybrinth is distributed in the hope that it will be fun, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License along with Cybrinth. If not, see <http://www.gnu.org/licenses/>.
 * 
 * @section DESCRIPTION
 * The MenuManager class is responsible for showing the in-game pause menu, and telling the GameManager how to proceed when a menu item is clicked.
 */

#include "MenuManager.h"
#include "GameManager.h"

MenuManager::MenuManager() {
	nextMaze.setText( L"Next maze" );
	restartMaze.setText( L"Restart maze" );
	loadMaze.setText( L"Load maze" );
	saveMaze.setText( L"Save maze" );
	exitGame.setText( L"Exit game" );
	backToGame.setText( L"Back to game" );
	freedom.setText( L"Freedom" );
}

MenuManager::~MenuManager() {
	//dtor
}

void MenuManager::draw( irr::video::IVideoDriver* driver ) {
	nextMaze.draw( driver );
	restartMaze.draw( driver );
	loadMaze.draw( driver );
	saveMaze.draw( driver );
	exitGame.draw( driver );
	backToGame.draw( driver );
	freedom.draw( driver );
}

void MenuManager::findHighlights( int_fast32_t x, int_fast32_t y ) {
	exitGame.highlighted = ( exitGame.contains( x, y ) );
	loadMaze.highlighted = ( loadMaze.contains( x, y ) );
	saveMaze.highlighted = ( saveMaze.contains( x, y ) );
	nextMaze.highlighted = ( nextMaze.contains( x, y ) );
	restartMaze.highlighted = ( restartMaze.contains( x, y ) );
	backToGame.highlighted = ( backToGame.contains( x, y ) );
	freedom.highlighted = ( freedom.contains( x, y ) );
}

/**
* Should be called only by OnEvent() or processControls().
* Arguments:
* A pointer to the GameManager.
*/
void MenuManager::processSelection( GameManager* gm ) {
	if( gm == nullptr || gm == NULL ) {
		return;
	}
	
	if( exitGame.highlighted ) {
		gm->setExitConfirmation( gm->gui->addMessageBox( L"Exit?", L"Are you sure you want to exit?", true, ( irr::gui::EMBF_YES | irr::gui::EMBF_NO ) ) );
	} else if( loadMaze.highlighted ) {
		//gm->setFileChooser( gm->gui->addFileOpenDialog( L"Select a Maze", true, 0, -1, true ) );
		gm->showLoadMazeDialog();
	} else if( saveMaze.highlighted ) {
		gm->showSaveMazeDialog();
	} else if( nextMaze.highlighted ) {
		gm->newMaze();
	} else if( restartMaze.highlighted ) {
		gm->newMaze( gm->randomSeed );
	} else if( backToGame.highlighted ) {
		gm->showingMenu = false;
	} else if( freedom.highlighted ) {
		std::wstring message = stringConverter.toStdWString( PACKAGE_NAME );
		message += L" is copyright 2012-2014 by James Dearing. Licensed under the GNU Affero General Public License, version 3.0 or (at your option) any later version, as published by the Free Software Foundation. See the file \"COPYING\" or https://www.gnu.org/licenses/agpl.html.\n\nThis means you're free to do what you want with this game: mod it, give copies to friends, sell it if you want. Whatever. It's Free software, Free as in Freedom. You should have received the program's source code with this copy; if you don't have it, you can get it from ";
		message += stringConverter.toStdWString( PACKAGE_URL );
		message += L".\n\n";
		message += stringConverter.toStdWString( PACKAGE_NAME );
		message += L" is distributed in the hope that it will be fun, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.";
		gm->gui->addMessageBox( L"Freedom", stringConverter.toStdWString( message ).c_str() ); //stringConverter.toWCharArray( message ) );
	}
}

void MenuManager::scrollHighlights( bool up ) {
	if( up ) {
		if( nextMaze.highlighted ) {
			nextMaze.highlighted = false;
			freedom.highlighted = true;
		} else if( restartMaze.highlighted ) {
			restartMaze.highlighted = false;
			nextMaze.highlighted = true;
		} else if( loadMaze.highlighted ) {
			loadMaze.highlighted = false;
			restartMaze.highlighted = true;
		} else if( saveMaze.highlighted ) {
			saveMaze.highlighted = false;
			loadMaze.highlighted = true;
		} else if( exitGame.highlighted ) {
			exitGame.highlighted = false;
			saveMaze.highlighted = true;
		} else if( backToGame.highlighted ) {
			backToGame.highlighted = false;
			exitGame.highlighted = true;
		} else if( freedom.highlighted ) {
			freedom.highlighted = false;
			backToGame.highlighted = true;
		} else { //if none are highlighted
			nextMaze.highlighted = true;
		}
	} else {
		if( nextMaze.highlighted ) {
			nextMaze.highlighted = false;
			restartMaze.highlighted = true;
		} else if( restartMaze.highlighted ) {
			restartMaze.highlighted = false;
			loadMaze.highlighted = true;
		} else if( loadMaze.highlighted ) {
			loadMaze.highlighted = false;
			saveMaze.highlighted = true;
		} else if( saveMaze.highlighted ) {
			saveMaze.highlighted = false;
			exitGame.highlighted = true;
		} else if( exitGame.highlighted ) {
			exitGame.highlighted = false;
			backToGame.highlighted = true;
		} else if( backToGame.highlighted ) {
			backToGame.highlighted = false;
			freedom.highlighted = true;
		} else if( freedom.highlighted ) {
			freedom.highlighted = false;
			nextMaze.highlighted = true;
		} else { //if none are highlighted
			nextMaze.highlighted = true;
		}
	}
}

void MenuManager::setFont( irr::gui::IGUIFont* font ) {
	nextMaze.setFont( font );
	restartMaze.setFont( font );
	loadMaze.setFont( font );
	saveMaze.setFont( font );
	exitGame.setFont( font );
	backToGame.setFont( font );
	freedom.setFont( font );
}

/**
 * Places the menu options at the correct locations on screen.
 */
void MenuManager::setPositions( uint_fast32_t windowHeight ) {
	try {
		
		uint_fast8_t numMenuOptions = 7;
		nextMaze.setY( 0 );
		restartMaze.setY( 1 * windowHeight / numMenuOptions );
		loadMaze.setY( 2 * windowHeight / numMenuOptions );
		saveMaze.setY( 3 * windowHeight / numMenuOptions );
		exitGame.setY( 4 * windowHeight / numMenuOptions );
		backToGame.setY( 5 * windowHeight / numMenuOptions );
		freedom.setY( 6 * windowHeight / numMenuOptions );
		
	} catch( std::exception &e ) {
		std::wcerr << L"Error in MazeManager::setPositions(): " << e.what() << std::endl;
	}
}
