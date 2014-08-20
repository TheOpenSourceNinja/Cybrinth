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

#ifndef MENUMANAGER_H
#define MENUMANAGER_H

//#include "GameManager.h"
class GameManager; //Avoids circular dependency
#include <iostream>
#include <irrlicht/irrlicht.h>
#include "MenuOption.h"
#include <string>
#include "StringConverter.h"

class MenuManager {
	public:
		MenuManager();
		virtual ~MenuManager();
		
		void draw( irr::video::IVideoDriver* driver );
		
		void findHighlights( int_fast32_t x, int_fast32_t y );
		
		void processSelection( GameManager* gm );
		
		void scrollHighlights( bool up );
		
		void setFont( irr::gui::IGUIFont* font );
		
		void setPositions( uint_fast32_t windowHeight );
	protected:
	private:
		MenuOption backToGame;
		
		MenuOption freedom;

		MenuOption exitGame;
		
		MenuOption loadMaze;
		
		MenuOption nextMaze;
		
		MenuOption restartMaze;
		
		MenuOption saveMaze;
		StringConverter stringConverter;
};

#endif // MENUMANAGER_H
