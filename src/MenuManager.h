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
 * The MenuManager class is responsible for showing the in-game pause menu, and telling the MainGame how to proceed when a menu item is clicked.
 */

#ifndef MENUMANAGER_H
#define MENUMANAGER_H

//#include "MainGame.h"
class MainGame; //Avoids circular dependency
#include <iostream>
#include <irrlicht/irrlicht.h>
#include "MenuOption.h"
#include <string>
#include "StringConverter.h"

class MenuManager {
	public:
		MenuManager();
		virtual ~MenuManager();
		
		enum selection_t : uint_fast8_t { NEXT_MAZE, RESTART_MAZE, LOAD_MAZE, SAVE_MAZE, EXIT_GAME, BACK_TO_GAME, FREEDOM };
		
		void draw( irr::video::IVideoDriver* driver );
		
		void findHighlights( int_fast32_t x, int_fast32_t y );
		
		void processSelection( MainGame* mg );
		
		void scrollSelection( bool up );
		
		void setFont( irr::gui::IGUIFont* font );
		
		void setPositions( uint_fast32_t windowHeight );
		
		void setSelection( selection_t selection );
	protected:
	private:
		std::vector< MenuOption > options;
		
		uint_fast8_t backToGame;
		
		uint_fast8_t freedom;

		uint_fast8_t exitGame;
		
		uint_fast8_t highlightedOption;
		
		uint_fast8_t loadMaze;
		
		uint_fast8_t nextMaze;
		
		uint_fast8_t restartMaze;
		
		uint_fast8_t saveMaze;
		StringConverter stringConverter;
};

#endif // MENUMANAGER_H
