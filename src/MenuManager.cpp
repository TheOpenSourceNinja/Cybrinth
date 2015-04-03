/**
 * @file
 * @author James Dearing <dearingj@lifetime.oregonstate.edu>
 * 
 * @section LICENSE
 * Copyright © 2012-2015.
 * This file is part of Cybrinth.
 *
 * Cybrinth is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * Cybrinth is distributed in the hope that it will be fun, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License along with Cybrinth. If not, see <http://www.gnu.org/licenses/>.
 * 
 * @section DESCRIPTION
 * The MenuManager class is responsible for showing the in-game pause menu, and telling the MainGame how to proceed when a menu item is clicked.
 */

#include "MenuManager.h"
#include "MainGame.h"

MenuManager::MenuManager() {
	{
		MenuOption temp;
		temp.setText( L"Next maze" );
		nextMaze = options.size();
		options.push_back( temp );
	}
	{
		MenuOption temp;
		temp.setText( L"Restart maze" );
		restartMaze = options.size();
		options.push_back( temp );
	}
	{
		MenuOption temp;
		temp.setText( L"Load maze" );
		loadMaze = options.size();
		options.push_back( temp );
	}
	{
		MenuOption temp;
		temp.setText( L"Save maze" );
		saveMaze = options.size();
		options.push_back( temp );
	}
	{
		MenuOption temp;
		temp.setText( L"Exit game" );
		exitGame = options.size();
		options.push_back( temp );
	}
	{
		MenuOption temp;
		temp.setText( L"Back to game" );
		backToGame = options.size();
		options.push_back( temp );
	}
	{
		MenuOption temp;
		temp.setText( L"Freedom" );
		freedom = options.size();
		options.push_back( temp );
	}
	highlightedOption = backToGame;
}

MenuManager::~MenuManager() {
	//dtor
}

void MenuManager::draw( irr::video::IVideoDriver* driver ) {
	for( decltype( options.size() ) o = 0; o < options.size(); ++o ) {
		options.at( o ).draw( driver );
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
		decltype( minDistance ) distance = sqrt( pow( abs( ( decltype( minDistance ) ) x - ( decltype( minDistance ) ) options.at( o ).getMiddleX() ), 2 ) + pow( abs( ( decltype( minDistance ) ) y - ( decltype( minDistance ) ) options.at( o ).getMiddleY() ), 2 ) );
		if( distance < minDistance ) {
			options.at( currentHighlight ).highlighted = false;
			currentHighlight = o;
			options.at( currentHighlight ).highlighted = true;
			minDistance = distance;
		}
	}
	
	/*options.at( exitGame ).highlighted = ( options.at( exitGame ).contains( x, y ) );
	options.at( loadMaze ).highlighted = ( options.at( loadMaze ).contains( x, y ) );
	options.at( saveMaze ).highlighted = ( options.at( saveMaze ).contains( x, y ) );
	options.at( nextMaze ).highlighted = ( options.at( nextMaze ).contains( x, y ) );
	options.at( restartMaze ).highlighted = ( options.at( restartMaze ).contains( x, y ) );
	options.at( backToGame ).highlighted = ( options.at( backToGame ).contains( x, y ) );
	options.at( freedom ).highlighted = ( options.at( freedom ).contains( x, y ) );*/
}

/**
* Should be called only by OnEvent() or processControls().
* Arguments:
* A pointer to the MainGame.
*/
void MenuManager::processSelection( MainGame* mg ) {
	if( mg == nullptr or mg == NULL ) {
		return;
	}
	
	if( options.at( exitGame ).highlighted ) {
		mg->setExitConfirmation( mg->gui->addMessageBox( L"Exit?", L"Are you sure you want to exit?", true, ( irr::gui::EMBF_YES bitor irr::gui::EMBF_NO ) ) );
	} else if( options.at( loadMaze ).highlighted ) {
		//mg->setFileChooser( mg->gui->addFileOpenDialog( L"Select a Maze", true, 0, -1, true ) );
		mg->showLoadMazeDialog();
	} else if( options.at( saveMaze ).highlighted ) {
		mg->showSaveMazeDialog();
	} else if( options.at( nextMaze ).highlighted ) {
		mg->newMaze();
	} else if( options.at( restartMaze ).highlighted ) {
		mg->newMaze( mg->getRandomSeed() );
	} else if( options.at( backToGame ).highlighted ) {
		mg->showingMenu = false;
	} else if( options.at( freedom ).highlighted ) {
		std::wstring message = stringConverter.toStdWString( PACKAGE_NAME );
		message += L" is copyright 2012-2015 by James Dearing. Licensed under the GNU Affero General Public License, version 3.0 or (at your option) any later version, as published by the Free Software Foundation. See the file \"COPYING\" or https://www.gnu.org/licenses/agpl.html.\n\nThis means you're free to do what you want with this game: mod it, give copies to friends, sell it if you want. Whatever. It's Free software, Free as in Freedom. You should have received the program's source code with this copy; if you don't have it, you can get it from ";
		message += stringConverter.toStdWString( PACKAGE_URL );
		message += L".\n\n";
		message += stringConverter.toStdWString( PACKAGE_NAME );
		message += L" is distributed 'as is' in the hope that it will be fun, but WITHOUT ANY WARRANTY; without even the implied warranty of TITLE, MERCHANTABILITY, COMPLETE DESTRUCTION OF IMPORTANT DATA, or FITNESS FOR A PARTICULAR PURPOSE.";
		message += L"\n\n";
		message += L"Cybrinth uses a copy of the RakNet networking library. RakNet is owned by Oculus VR, Inc. It is available under the terms of a modified 2-clause BSD license. See src/RakNet/LICENSE for the copyright license, and src/RakNet/PATENTS for patent information, and src/RakNet/ORIGIN for the URL from which this copy of RakNet was downloaded; copies of LICENSE and PATENTS can also be obtained from there. The creators of Cybrinth claim no connection to RakNet or Oculus VR, and no responsibility for any bugs or features in RakNet.";
		mg->gui->addMessageBox( L"Freedom", stringConverter.toStdWString( message ).c_str() ); //stringConverter.toWCharArray( message ) );
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
		
		/*if( nextMaze.highlighted ) {
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
			backToGame.highlighted = true;
		}*/
	} else {
		nextHighlight = ( currentHighlight + 1 ) % options.size();
	
		/*if( nextMaze.highlighted ) {
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
			backToGame.highlighted = true;
		}*/
	}
	
	options.at( currentHighlight ).highlighted = false;
	options.at( nextHighlight ).highlighted = true;
}

void MenuManager::setFont( irr::gui::IGUIFont* font ) {
	for( decltype( options.size() ) o = 0; o < options.size(); ++o ) {
		options.at( o ).setFont( font );
	}
	/*nextMaze.setFont( font );
	restartMaze.setFont( font );
	loadMaze.setFont( font );
	saveMaze.setFont( font );
	exitGame.setFont( font );
	backToGame.setFont( font );
	freedom.setFont( font );*/
}

/**
 * Places the menu options at the correct locations on screen.
 */
void MenuManager::setPositions( uint_fast32_t windowHeight ) {
	try {
		for( decltype( options.size() ) o = 0; o < options.size(); ++o ) {
			options.at( o ).setY( o * windowHeight / options.size() );
		}
		
		/*uint_fast8_t numMenuOptions = 7;
		nextMaze.setY( 0 );
		restartMaze.setY( 1 * windowHeight / numMenuOptions );
		loadMaze.setY( 2 * windowHeight / numMenuOptions );
		saveMaze.setY( 3 * windowHeight / numMenuOptions );
		exitGame.setY( 4 * windowHeight / numMenuOptions );
		backToGame.setY( 5 * windowHeight / numMenuOptions );
		freedom.setY( 6 * windowHeight / numMenuOptions );*/
		
	} catch( std::exception &e ) {
		std::wcerr << L"Error in MazeManager::setPositions(): " << e.what() << std::endl;
	}
}
