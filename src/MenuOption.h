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

using namespace irr;
using namespace std;

class MenuOption {
	public:
		MenuOption();
		virtual ~MenuOption();
		u32 getX();
		void setX( u32 val );
		u32 getY();
		void setY( u32 val );
		void setFont( gui::IGUIFont* newFont );
		void draw( video::IVideoDriver* driver );
		bool isWithin( core::position2d<u32> test );
		bool isWithin( u32 testX, u32 testY );
		void setText( core::stringw newText );
	protected:
	private:
		u32 x;
		u32 y;
		core::dimension2d<u32> dimension;
		gui::IGUIFont* font;
		core::stringw text;
		void setDimension();
};

#endif // MENUOPTION_H
