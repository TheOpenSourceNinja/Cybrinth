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
class MainGame; //Avoids circular dependency

class MenuOption {
	public:
		enum option_t : uint_fast8_t { JOIN_SERVER, BACK_TO_GAME, CANCEL, CONTROLS, EXIT_GAME, FREEDOM, LOAD_MAZE, NEW_MAZE, NUMBOTS, OK, RESET_TO_DEFAULTS, RESTART_MAZE, SAVE_MAZE, SETTINGS, UNDO_CHANGES, DO_NOT_USE };

		MenuOption();
		virtual ~MenuOption();

		bool contains( irr::core::position2d< uint_fast32_t > test ) const;
		bool contains( uint_fast32_t testX, uint_fast32_t testY ) const;
		void createTexture( irr::IrrlichtDevice* device );

		void draw( irr::IrrlichtDevice* device );
		
		uint_fast16_t getHeight() const;
		uint_fast16_t getWidth() const;
		uint_fast16_t getMiddleX() const;
		uint_fast16_t getMiddleY() const;
		uint_fast16_t getX() const;
		uint_fast16_t getY() const;

		bool highlighted;

		void loadTexture( irr::IrrlichtDevice* device );
		
		void setFontAndResizeIcon( irr::IrrlichtDevice* device, irr::gui::IGUIFont* newFont );
		void setMainGame( MainGame* mg );
		void setType( irr::IrrlichtDevice* device, option_t newType );
		void setX( uint_fast16_t val );
		void setY( uint_fast16_t val );
	protected:
	private:
		irr::core::dimension2d< uint_fast16_t > dimension;

		irr::core::stringw fileName;  //fileName gets set equal to text by the setText() function, but will be changed by loadTexture()
		irr::gui::IGUIFont* font;

		irr::video::ITexture* iconTexture;
		
		MainGame* mainGame;
		
		void setDimension( irr::video::IVideoDriver* driver );

		irr::core::stringw text;
		option_t type;

		uint_fast16_t x;

		uint_fast16_t y;
};

#endif // MENUOPTION_H
