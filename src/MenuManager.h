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

#ifndef MENUMANAGER_H
#define MENUMANAGER_H

//#include "MainGame.h"
class MainGame; //Avoids circular dependency
#include <iostream>
#ifdef WINDOWS
    #include <irrlicht.h>
#else
    #include <irrlicht/irrlicht.h>
#endif
#include "MenuOption.h"
#include <string>
#include "StringConverter.h"

class MenuManager {
	public:
		void draw( irr::IrrlichtDevice* device );

		void findHighlights( int_fast32_t x, int_fast32_t y );

		void loadIcons( irr::IrrlichtDevice* device );

		MenuManager();
		virtual ~MenuManager();

		void processSelection( MainGame* mg );

		void scrollSelection( bool up );
		void setFontAndResizeIcons( irr::IrrlichtDevice* device, irr::gui::IGUIFont* font );
		void setPositions( uint_fast32_t windowHeight );
		void setSelection( MenuOption::option_t selection );
	protected:
	private:
		uint_fast8_t backToGame;

		uint_fast8_t exitGame;

		uint_fast8_t freedom;

		uint_fast8_t highlightedOption;

		uint_fast8_t loadMaze;

		uint_fast8_t nextMaze;

		std::vector< MenuOption > options;

		uint_fast8_t restartMaze;

		uint_fast8_t saveMaze;
		uint_fast8_t settings;
		StringConverter stringConverter;
};

#endif // MENUMANAGER_H
