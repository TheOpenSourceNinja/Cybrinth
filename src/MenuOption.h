/**
 * Copyright © 2013 James Dearing.
 * This file is part of Cybrinth.
 *
 * Cybrinth is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * Cybrinth is distributed in the hope that it will be fun, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License along with Cybrinth. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef MENUOPTION_H
#define MENUOPTION_H

#include <irrlicht.h>
#include <cstdint>

using namespace std;

class MenuOption {
	public:
		MenuOption();
		virtual ~MenuOption();
		uint_least16_t getX();
		void setX( uint_least16_t val );
		uint_least16_t getY();
		void setY( uint_least16_t val );
		void setFont( irr::gui::IGUIFont* newFont );
		void draw( irr::video::IVideoDriver* driver );
		bool contains( irr::core::position2d<uint_least16_t> test );
		bool contains( uint_least16_t testX, uint_least16_t testY );
		void setText( irr::core::stringw newText );
	protected:
	private:
		uint_least16_t x;
		uint_least16_t y;
		irr::core::dimension2d<uint_least16_t> dimension;
		irr::gui::IGUIFont* font;
		irr::core::stringw text;
		void setDimension();
};

#endif // MENUOPTION_H
