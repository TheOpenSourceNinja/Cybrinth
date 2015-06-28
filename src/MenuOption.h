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
 * The MenuOption class represents an item in the in-game pause menu.
 */

#ifndef MENUOPTION_H
#define MENUOPTION_H

#ifdef WINDOWS
    #include <irrlicht.h>
#else
    #include <irrlicht/irrlicht.h>
#endif
#include "Integers.h"
#include "PreprocessorCommands.h"


class MenuOption {
	public:
		enum option_t : uint_fast8_t { NEW_MAZE, RESTART_MAZE, LOAD_MAZE, SAVE_MAZE, SETTINGS, EXIT_GAME, BACK_TO_GAME, FREEDOM, DO_NOT_USE };

		MenuOption();
		virtual ~MenuOption();

		bool contains( irr::core::position2d< uint_fast32_t > test );
		bool contains( uint_fast32_t testX, uint_fast32_t testY );
		void createTexture( irr::IrrlichtDevice* device );

		void draw( irr::IrrlichtDevice* device );

		uint_fast16_t getMiddleX();
		uint_fast16_t getMiddleY();
		uint_fast16_t getX();
		uint_fast16_t getY();

		bool highlighted;

		void loadTexture( irr::IrrlichtDevice* device );

		void setFontAndResizeIcon( irr::IrrlichtDevice* device, irr::gui::IGUIFont* newFont );
		void setType( irr::IrrlichtDevice* device, option_t newType );
		void setX( uint_fast16_t val );
		void setY( uint_fast16_t val );
	protected:
	private:
		irr::core::dimension2d<uint_fast16_t> dimension;

		irr::core::stringw fileName;  //fileName gets set equal to text by the setText() function, but will be changed by loadTexture()
		irr::gui::IGUIFont* font;

		irr::video::ITexture* iconTexture;

		void setDimension();

		irr::core::stringw text;
		option_t type;

		uint_fast16_t x;

		uint_fast16_t y;
};

#endif // MENUOPTION_H
